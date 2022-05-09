/*
 * Copyright © 2018 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "aco_ir.h"

#include "common/sid.h"

#include <map>
#include <stack>
#include <vector>

namespace aco {

namespace {

/**
 * The general idea of this pass is:
 * The CFG is traversed in reverse postorder (forward) and loops are processed
 * several times until no progress is made.
 * Per BB two wait_ctx is maintained: an in-context and out-context.
 * The in-context is the joined out-contexts of the predecessors.
 * The context contains a map: gpr -> wait_entry
 * consisting of the information about the cnt values to be waited for.
 * Note: After merge-nodes, it might occur that for the same register
 *       multiple cnt values are to be waited for.
 *
 * The values are updated according to the encountered instructions:
 * - additional events increment the counter of waits of the same type
 * - or erase gprs with counters higher than to be waited for.
 */

// TODO: do a more clever insertion of wait_cnt (lgkm_cnt)
// when there is a load followed by a use of a previous load

/* Instructions of the same event will finish in-order except for smem
 * and maybe flat. Instructions of different events may not finish in-order. */
enum wait_event : uint16_t {
   event_smem = 1 << 0,
   event_lds = 1 << 1,
   event_gds = 1 << 2,
   event_vmem = 1 << 3,
   event_vmem_store = 1 << 4, /* GFX10+ */
   event_flat = 1 << 5,
   event_exp_pos = 1 << 6,
   event_exp_param = 1 << 7,
   event_exp_mrt_null = 1 << 8,
   event_gds_gpr_lock = 1 << 9,
   event_vmem_gpr_lock = 1 << 10,
   event_sendmsg = 1 << 11,
   num_events = 12,
};

enum counter_type : uint8_t {
   counter_exp = 1 << 0,
   counter_lgkm = 1 << 1,
   counter_vm = 1 << 2,
   counter_vs = 1 << 3,
   num_counters = 4,
};

static const uint16_t exp_events =
   event_exp_pos | event_exp_param | event_exp_mrt_null | event_gds_gpr_lock | event_vmem_gpr_lock;
static const uint16_t lgkm_events = event_smem | event_lds | event_gds | event_flat | event_sendmsg;
static const uint16_t vm_events = event_vmem | event_flat;
static const uint16_t vs_events = event_vmem_store;

uint8_t
get_counters_for_event(wait_event ev)
{
   switch (ev) {
   case event_smem:
   case event_lds:
   case event_gds:
   case event_sendmsg: return counter_lgkm;
   case event_vmem: return counter_vm;
   case event_vmem_store: return counter_vs;
   case event_flat: return counter_vm | counter_lgkm;
   case event_exp_pos:
   case event_exp_param:
   case event_exp_mrt_null:
   case event_gds_gpr_lock:
   case event_vmem_gpr_lock: return counter_exp;
   default: return 0;
   }
}

struct wait_entry {
   wait_imm imm;
   uint16_t events;  /* use wait_event notion */
   uint8_t counters; /* use counter_type notion */
   bool wait_on_read : 1;
   bool logical : 1;
   bool has_vmem_nosampler : 1;
   bool has_vmem_sampler : 1;

   wait_entry(wait_event event_, wait_imm imm_, bool logical_, bool wait_on_read_)
       : imm(imm_), events(event_), counters(get_counters_for_event(event_)),
         wait_on_read(wait_on_read_), logical(logical_), has_vmem_nosampler(false),
         has_vmem_sampler(false)
   {}

   bool join(const wait_entry& other)
   {
      bool changed = (other.events & ~events) || (other.counters & ~counters) ||
                     (other.wait_on_read && !wait_on_read) ||
                     (other.has_vmem_nosampler && !has_vmem_nosampler) ||
                     (other.has_vmem_sampler && !has_vmem_sampler);
      events |= other.events;
      counters |= other.counters;
      changed |= imm.combine(other.imm);
      wait_on_read |= other.wait_on_read;
      has_vmem_nosampler |= other.has_vmem_nosampler;
      has_vmem_sampler |= other.has_vmem_sampler;
      assert(logical == other.logical);
      return changed;
   }

   void remove_counter(counter_type counter)
   {
      counters &= ~counter;

      if (counter == counter_lgkm) {
         imm.lgkm = wait_imm::unset_counter;
         events &= ~(event_smem | event_lds | event_gds | event_sendmsg);
      }

      if (counter == counter_vm) {
         imm.vm = wait_imm::unset_counter;
         events &= ~event_vmem;
         has_vmem_nosampler = false;
         has_vmem_sampler = false;
      }

      if (counter == counter_exp) {
         imm.exp = wait_imm::unset_counter;
         events &= ~(event_exp_pos | event_exp_param | event_exp_mrt_null | event_gds_gpr_lock |
                     event_vmem_gpr_lock);
      }

      if (counter == counter_vs) {
         imm.vs = wait_imm::unset_counter;
         events &= ~event_vmem_store;
      }

      if (!(counters & counter_lgkm) && !(counters & counter_vm))
         events &= ~event_flat;
   }
};

struct wait_ctx {
   Program* program;
   enum chip_class chip_class;
   uint16_t max_vm_cnt;
   uint16_t max_exp_cnt;
   uint16_t max_lgkm_cnt;
   uint16_t max_vs_cnt;
   uint16_t unordered_events = event_smem | event_flat;

   uint8_t vm_cnt = 0;
   uint8_t exp_cnt = 0;
   uint8_t lgkm_cnt = 0;
   uint8_t vs_cnt = 0;
   bool pending_flat_lgkm = false;
   bool pending_flat_vm = false;
   bool pending_s_buffer_store = false; /* GFX10 workaround */

   wait_imm barrier_imm[storage_count];
   uint16_t barrier_events[storage_count] = {}; /* use wait_event notion */

   std::map<PhysReg, wait_entry> gpr_map;

   wait_ctx() {}
   wait_ctx(Program* program_)
       : program(program_), chip_class(program_->chip_class),
         max_vm_cnt(program_->chip_class >= GFX9 ? 62 : 14), max_exp_cnt(6),
         max_lgkm_cnt(program_->chip_class >= GFX10 ? 62 : 14),
         max_vs_cnt(program_->chip_class >= GFX10 ? 62 : 0),
         unordered_events(event_smem | (program_->chip_class < GFX10 ? event_flat : 0))
   {}

   bool join(const wait_ctx* other, bool logical)
   {
      bool changed = other->exp_cnt > exp_cnt || other->vm_cnt > vm_cnt ||
                     other->lgkm_cnt > lgkm_cnt || other->vs_cnt > vs_cnt ||
                     (other->pending_flat_lgkm && !pending_flat_lgkm) ||
                     (other->pending_flat_vm && !pending_flat_vm);

      exp_cnt = std::max(exp_cnt, other->exp_cnt);
      vm_cnt = std::max(vm_cnt, other->vm_cnt);
      lgkm_cnt = std::max(lgkm_cnt, other->lgkm_cnt);
      vs_cnt = std::max(vs_cnt, other->vs_cnt);
      pending_flat_lgkm |= other->pending_flat_lgkm;
      pending_flat_vm |= other->pending_flat_vm;
      pending_s_buffer_store |= other->pending_s_buffer_store;

      for (const auto& entry : other->gpr_map) {
         if (entry.second.logical != logical)
            continue;

         using iterator = std::map<PhysReg, wait_entry>::iterator;
         const std::pair<iterator, bool> insert_pair = gpr_map.insert(entry);
         if (insert_pair.second) {
            changed = true;
         } else {
            changed |= insert_pair.first->second.join(entry.second);
         }
      }

      for (unsigned i = 0; i < storage_count; i++) {
         changed |= barrier_imm[i].combine(other->barrier_imm[i]);
         changed |= (other->barrier_events[i] & ~barrier_events[i]) != 0;
         barrier_events[i] |= other->barrier_events[i];
      }

      return changed;
   }

   void wait_and_remove_from_entry(PhysReg reg, wait_entry& entry, counter_type counter)
   {
      entry.remove_counter(counter);
   }
};

void
check_instr(wait_ctx& ctx, wait_imm& wait, Instruction* instr)
{
   for (const Operand op : instr->operands) {
      if (op.isConstant() || op.isUndefined())
         continue;

      /* check consecutively read gprs */
      for (unsigned j = 0; j < op.size(); j++) {
         PhysReg reg{op.physReg() + j};
         std::map<PhysReg, wait_entry>::iterator it = ctx.gpr_map.find(reg);
         if (it == ctx.gpr_map.end() || !it->second.wait_on_read)
            continue;

         wait.combine(it->second.imm);
      }
   }

   for (const Definition& def : instr->definitions) {
      /* check consecutively written gprs */
      for (unsigned j = 0; j < def.getTemp().size(); j++) {
         PhysReg reg{def.physReg() + j};

         std::map<PhysReg, wait_entry>::iterator it = ctx.gpr_map.find(reg);
         if (it == ctx.gpr_map.end())
            continue;

         /* Vector Memory reads and writes return in the order they were issued */
         bool has_sampler = instr->isMIMG() && !instr->operands[1].isUndefined() &&
                            instr->operands[1].regClass() == s4;
         if (instr->isVMEM() && ((it->second.events & vm_events) == event_vmem) &&
             it->second.has_vmem_nosampler == !has_sampler &&
             it->second.has_vmem_sampler == has_sampler)
            continue;

         /* LDS reads and writes return in the order they were issued. same for GDS */
         if (instr->isDS() &&
             (it->second.events & lgkm_events) == (instr->ds().gds ? event_gds : event_lds))
            continue;

         wait.combine(it->second.imm);
      }
   }
}

bool
parse_wait_instr(wait_ctx& ctx, wait_imm& imm, Instruction* instr)
{
   if (instr->opcode == aco_opcode::s_waitcnt_vscnt &&
       instr->definitions[0].physReg() == sgpr_null) {
      imm.vs = std::min<uint8_t>(imm.vs, instr->sopk().imm);
      return true;
   } else if (instr->opcode == aco_opcode::s_waitcnt) {
      imm.combine(wait_imm(ctx.chip_class, instr->sopp().imm));
      return true;
   }
   return false;
}

void
perform_barrier(wait_ctx& ctx, wait_imm& imm, memory_sync_info sync, unsigned semantics)
{
   sync_scope subgroup_scope =
      ctx.program->workgroup_size <= ctx.program->wave_size ? scope_workgroup : scope_subgroup;
   if ((sync.semantics & semantics) && sync.scope > subgroup_scope) {
      unsigned storage = sync.storage;
      while (storage) {
         unsigned idx = u_bit_scan(&storage);

         /* LDS is private to the workgroup */
         sync_scope bar_scope_lds = MIN2(sync.scope, scope_workgroup);

         uint16_t events = ctx.barrier_events[idx];
         if (bar_scope_lds <= subgroup_scope)
            events &= ~event_lds;

         /* in non-WGP, the L1 (L0 on GFX10+) cache keeps all memory operations
          * in-order for the same workgroup */
         if (!ctx.program->wgp_mode && sync.scope <= scope_workgroup)
            events &= ~(event_vmem | event_vmem_store | event_smem);

         if (events)
            imm.combine(ctx.barrier_imm[idx]);
      }
   }
}

void
force_waitcnt(wait_ctx& ctx, wait_imm& imm)
{
   if (ctx.vm_cnt)
      imm.vm = 0;
   if (ctx.exp_cnt)
      imm.exp = 0;
   if (ctx.lgkm_cnt)
      imm.lgkm = 0;

   if (ctx.chip_class >= GFX10) {
      if (ctx.vs_cnt)
         imm.vs = 0;
   }
}

void
kill(wait_imm& imm, Instruction* instr, wait_ctx& ctx, memory_sync_info sync_info)
{
   if (debug_flags & DEBUG_FORCE_WAITCNT) {
      /* Force emitting waitcnt states right after the instruction if there is
       * something to wait for.
       */
      return force_waitcnt(ctx, imm);
   }

   if (ctx.exp_cnt || ctx.vm_cnt || ctx.lgkm_cnt)
      check_instr(ctx, imm, instr);

   /* It's required to wait for scalar stores before "writing back" data.
    * It shouldn't cost anything anyways since we're about to do s_endpgm.
    */
   if (ctx.lgkm_cnt && instr->opcode == aco_opcode::s_dcache_wb) {
      assert(ctx.chip_class >= GFX8);
      imm.lgkm = 0;
   }

   if (ctx.chip_class >= GFX10 && instr->isSMEM()) {
      /* GFX10: A store followed by a load at the same address causes a problem because
       * the load doesn't load the correct values unless we wait for the store first.
       * This is NOT mitigated by an s_nop.
       *
       * TODO: Refine this when we have proper alias analysis.
       */
      if (ctx.pending_s_buffer_store && !instr->smem().definitions.empty() &&
          !instr->smem().sync.can_reorder()) {
         imm.lgkm = 0;
      }
   }

   if (ctx.program->early_rast && instr->opcode == aco_opcode::exp) {
      if (instr->exp().dest >= V_008DFC_SQ_EXP_POS && instr->exp().dest < V_008DFC_SQ_EXP_PRIM) {

         /* With early_rast, the HW will start clipping and rasterization after the 1st DONE pos
          * export. Wait for all stores (and atomics) to complete, so PS can read them.
          * TODO: This only really applies to DONE pos exports.
          *       Consider setting the DONE bit earlier.
          */
         if (ctx.vs_cnt > 0)
            imm.vs = 0;
         if (ctx.vm_cnt > 0)
            imm.vm = 0;
      }
   }

   if (instr->opcode == aco_opcode::p_barrier)
      perform_barrier(ctx, imm, instr->barrier().sync, semantic_acqrel);
   else
      perform_barrier(ctx, imm, sync_info, semantic_release);

   if (!imm.empty()) {
      if (ctx.pending_flat_vm && imm.vm != wait_imm::unset_counter)
         imm.vm = 0;
      if (ctx.pending_flat_lgkm && imm.lgkm != wait_imm::unset_counter)
         imm.lgkm = 0;

      /* reset counters */
      ctx.exp_cnt = std::min(ctx.exp_cnt, imm.exp);
      ctx.vm_cnt = std::min(ctx.vm_cnt, imm.vm);
      ctx.lgkm_cnt = std::min(ctx.lgkm_cnt, imm.lgkm);
      ctx.vs_cnt = std::min(ctx.vs_cnt, imm.vs);

      /* update barrier wait imms */
      for (unsigned i = 0; i < storage_count; i++) {
         wait_imm& bar = ctx.barrier_imm[i];
         uint16_t& bar_ev = ctx.barrier_events[i];
         if (bar.exp != wait_imm::unset_counter && imm.exp <= bar.exp) {
            bar.exp = wait_imm::unset_counter;
            bar_ev &= ~exp_events;
         }
         if (bar.vm != wait_imm::unset_counter && imm.vm <= bar.vm) {
            bar.vm = wait_imm::unset_counter;
            bar_ev &= ~(vm_events & ~event_flat);
         }
         if (bar.lgkm != wait_imm::unset_counter && imm.lgkm <= bar.lgkm) {
            bar.lgkm = wait_imm::unset_counter;
            bar_ev &= ~(lgkm_events & ~event_flat);
         }
         if (bar.vs != wait_imm::unset_counter && imm.vs <= bar.vs) {
            bar.vs = wait_imm::unset_counter;
            bar_ev &= ~vs_events;
         }
         if (bar.vm == wait_imm::unset_counter && bar.lgkm == wait_imm::unset_counter)
            bar_ev &= ~event_flat;
      }

      /* remove all gprs with higher counter from map */
      std::map<PhysReg, wait_entry>::iterator it = ctx.gpr_map.begin();
      while (it != ctx.gpr_map.end()) {
         if (imm.exp != wait_imm::unset_counter && imm.exp <= it->second.imm.exp)
            ctx.wait_and_remove_from_entry(it->first, it->second, counter_exp);
         if (imm.vm != wait_imm::unset_counter && imm.vm <= it->second.imm.vm)
            ctx.wait_and_remove_from_entry(it->first, it->second, counter_vm);
         if (imm.lgkm != wait_imm::unset_counter && imm.lgkm <= it->second.imm.lgkm)
            ctx.wait_and_remove_from_entry(it->first, it->second, counter_lgkm);
         if (imm.vs != wait_imm::unset_counter && imm.vs <= it->second.imm.vs)
            ctx.wait_and_remove_from_entry(it->first, it->second, counter_vs);
         if (!it->second.counters)
            it = ctx.gpr_map.erase(it);
         else
            it++;
      }
   }

   if (imm.vm == 0)
      ctx.pending_flat_vm = false;
   if (imm.lgkm == 0) {
      ctx.pending_flat_lgkm = false;
      ctx.pending_s_buffer_store = false;
   }
}

void
update_barrier_counter(uint8_t* ctr, unsigned max)
{
   if (*ctr != wait_imm::unset_counter && *ctr < max)
      (*ctr)++;
}

void
update_barrier_imm(wait_ctx& ctx, uint8_t counters, wait_event event, memory_sync_info sync)
{
   for (unsigned i = 0; i < storage_count; i++) {
      wait_imm& bar = ctx.barrier_imm[i];
      uint16_t& bar_ev = ctx.barrier_events[i];
      if (sync.storage & (1 << i) && !(sync.semantics & semantic_private)) {
         bar_ev |= event;
         if (counters & counter_lgkm)
            bar.lgkm = 0;
         if (counters & counter_vm)
            bar.vm = 0;
         if (counters & counter_exp)
            bar.exp = 0;
         if (counters & counter_vs)
            bar.vs = 0;
      } else if (!(bar_ev & ctx.unordered_events) && !(ctx.unordered_events & event)) {
         if (counters & counter_lgkm && (bar_ev & lgkm_events) == event)
            update_barrier_counter(&bar.lgkm, ctx.max_lgkm_cnt);
         if (counters & counter_vm && (bar_ev & vm_events) == event)
            update_barrier_counter(&bar.vm, ctx.max_vm_cnt);
         if (counters & counter_exp && (bar_ev & exp_events) == event)
            update_barrier_counter(&bar.exp, ctx.max_exp_cnt);
         if (counters & counter_vs && (bar_ev & vs_events) == event)
            update_barrier_counter(&bar.vs, ctx.max_vs_cnt);
      }
   }
}

void
update_counters(wait_ctx& ctx, wait_event event, memory_sync_info sync = memory_sync_info())
{
   uint8_t counters = get_counters_for_event(event);

   if (counters & counter_lgkm && ctx.lgkm_cnt <= ctx.max_lgkm_cnt)
      ctx.lgkm_cnt++;
   if (counters & counter_vm && ctx.vm_cnt <= ctx.max_vm_cnt)
      ctx.vm_cnt++;
   if (counters & counter_exp && ctx.exp_cnt <= ctx.max_exp_cnt)
      ctx.exp_cnt++;
   if (counters & counter_vs && ctx.vs_cnt <= ctx.max_vs_cnt)
      ctx.vs_cnt++;

   update_barrier_imm(ctx, counters, event, sync);

   if (ctx.unordered_events & event)
      return;

   if (ctx.pending_flat_lgkm)
      counters &= ~counter_lgkm;
   if (ctx.pending_flat_vm)
      counters &= ~counter_vm;

   for (std::pair<const PhysReg, wait_entry>& e : ctx.gpr_map) {
      wait_entry& entry = e.second;

      if (entry.events & ctx.unordered_events)
         continue;

      assert(entry.events);

      if ((counters & counter_exp) && (entry.events & exp_events) == event &&
          entry.imm.exp < ctx.max_exp_cnt)
         entry.imm.exp++;
      if ((counters & counter_lgkm) && (entry.events & lgkm_events) == event &&
          entry.imm.lgkm < ctx.max_lgkm_cnt)
         entry.imm.lgkm++;
      if ((counters & counter_vm) && (entry.events & vm_events) == event &&
          entry.imm.vm < ctx.max_vm_cnt)
         entry.imm.vm++;
      if ((counters & counter_vs) && (entry.events & vs_events) == event &&
          entry.imm.vs < ctx.max_vs_cnt)
         entry.imm.vs++;
   }
}

void
update_counters_for_flat_load(wait_ctx& ctx, memory_sync_info sync = memory_sync_info())
{
   assert(ctx.chip_class < GFX10);

   if (ctx.lgkm_cnt <= ctx.max_lgkm_cnt)
      ctx.lgkm_cnt++;
   if (ctx.vm_cnt <= ctx.max_vm_cnt)
      ctx.vm_cnt++;

   update_barrier_imm(ctx, counter_vm | counter_lgkm, event_flat, sync);

   for (std::pair<PhysReg, wait_entry> e : ctx.gpr_map) {
      if (e.second.counters & counter_vm)
         e.second.imm.vm = 0;
      if (e.second.counters & counter_lgkm)
         e.second.imm.lgkm = 0;
   }
   ctx.pending_flat_lgkm = true;
   ctx.pending_flat_vm = true;
}

void
insert_wait_entry(wait_ctx& ctx, PhysReg reg, RegClass rc, wait_event event, bool wait_on_read,
                  bool has_sampler = false)
{
   uint16_t counters = get_counters_for_event(event);
   wait_imm imm;
   if (counters & counter_lgkm)
      imm.lgkm = 0;
   if (counters & counter_vm)
      imm.vm = 0;
   if (counters & counter_exp)
      imm.exp = 0;
   if (counters & counter_vs)
      imm.vs = 0;

   wait_entry new_entry(event, imm, !rc.is_linear(), wait_on_read);
   new_entry.has_vmem_nosampler = (event & event_vmem) && !has_sampler;
   new_entry.has_vmem_sampler = (event & event_vmem) && has_sampler;

   for (unsigned i = 0; i < rc.size(); i++) {
      auto it = ctx.gpr_map.emplace(PhysReg{reg.reg() + i}, new_entry);
      if (!it.second)
         it.first->second.join(new_entry);
   }
}

void
insert_wait_entry(wait_ctx& ctx, Operand op, wait_event event, bool has_sampler = false)
{
   if (!op.isConstant() && !op.isUndefined())
      insert_wait_entry(ctx, op.physReg(), op.regClass(), event, false, has_sampler);
}

void
insert_wait_entry(wait_ctx& ctx, Definition def, wait_event event, bool has_sampler = false)
{
   insert_wait_entry(ctx, def.physReg(), def.regClass(), event, true, has_sampler);
}

void
gen(Instruction* instr, wait_ctx& ctx)
{
   switch (instr->format) {
   case Format::EXP: {
      Export_instruction& exp_instr = instr->exp();

      wait_event ev;
      if (exp_instr.dest <= 9)
         ev = event_exp_mrt_null;
      else if (exp_instr.dest <= 15)
         ev = event_exp_pos;
      else
         ev = event_exp_param;
      update_counters(ctx, ev);

      /* insert new entries for exported vgprs */
      for (unsigned i = 0; i < 4; i++) {
         if (exp_instr.enabled_mask & (1 << i)) {
            unsigned idx = exp_instr.compressed ? i >> 1 : i;
            assert(idx < exp_instr.operands.size());
            insert_wait_entry(ctx, exp_instr.operands[idx], ev);
         }
      }
      insert_wait_entry(ctx, exec, s2, ev, false);
      break;
   }
   case Format::FLAT: {
      FLAT_instruction& flat = instr->flat();
      if (ctx.chip_class < GFX10 && !instr->definitions.empty())
         update_counters_for_flat_load(ctx, flat.sync);
      else
         update_counters(ctx, event_flat, flat.sync);

      if (!instr->definitions.empty())
         insert_wait_entry(ctx, instr->definitions[0], event_flat);
      break;
   }
   case Format::SMEM: {
      SMEM_instruction& smem = instr->smem();
      update_counters(ctx, event_smem, smem.sync);

      if (!instr->definitions.empty())
         insert_wait_entry(ctx, instr->definitions[0], event_smem);
      else if (ctx.chip_class >= GFX10 && !smem.sync.can_reorder())
         ctx.pending_s_buffer_store = true;

      break;
   }
   case Format::DS: {
      DS_instruction& ds = instr->ds();
      update_counters(ctx, ds.gds ? event_gds : event_lds, ds.sync);
      if (ds.gds)
         update_counters(ctx, event_gds_gpr_lock);

      if (!instr->definitions.empty())
         insert_wait_entry(ctx, instr->definitions[0], ds.gds ? event_gds : event_lds);

      if (ds.gds) {
         for (const Operand& op : instr->operands)
            insert_wait_entry(ctx, op, event_gds_gpr_lock);
         insert_wait_entry(ctx, exec, s2, event_gds_gpr_lock, false);
      }
      break;
   }
   case Format::MUBUF:
   case Format::MTBUF:
   case Format::MIMG:
   case Format::GLOBAL: {
      wait_event ev =
         !instr->definitions.empty() || ctx.chip_class < GFX10 ? event_vmem : event_vmem_store;
      update_counters(ctx, ev, get_sync_info(instr));

      bool has_sampler = instr->isMIMG() && !instr->operands[1].isUndefined() &&
                         instr->operands[1].regClass() == s4;

      if (!instr->definitions.empty())
         insert_wait_entry(ctx, instr->definitions[0], ev, has_sampler);

      if (ctx.chip_class == GFX6 && instr->format != Format::MIMG && instr->operands.size() == 4) {
         ctx.exp_cnt++;
         update_counters(ctx, event_vmem_gpr_lock);
         insert_wait_entry(ctx, instr->operands[3], event_vmem_gpr_lock);
      } else if (ctx.chip_class == GFX6 && instr->isMIMG() && !instr->operands[2].isUndefined()) {
         ctx.exp_cnt++;
         update_counters(ctx, event_vmem_gpr_lock);
         insert_wait_entry(ctx, instr->operands[2], event_vmem_gpr_lock);
      }

      break;
   }
   case Format::SOPP: {
      if (instr->opcode == aco_opcode::s_sendmsg || instr->opcode == aco_opcode::s_sendmsghalt)
         update_counters(ctx, event_sendmsg);
      break;
   }
   default: break;
   }
}

void
emit_waitcnt(wait_ctx& ctx, std::vector<aco_ptr<Instruction>>& instructions, wait_imm& imm)
{
   if (imm.vs != wait_imm::unset_counter) {
      assert(ctx.chip_class >= GFX10);
      SOPK_instruction* waitcnt_vs =
         create_instruction<SOPK_instruction>(aco_opcode::s_waitcnt_vscnt, Format::SOPK, 0, 1);
      waitcnt_vs->definitions[0] = Definition(sgpr_null, s1);
      waitcnt_vs->imm = imm.vs;
      instructions.emplace_back(waitcnt_vs);
      imm.vs = wait_imm::unset_counter;
   }
   if (!imm.empty()) {
      SOPP_instruction* waitcnt =
         create_instruction<SOPP_instruction>(aco_opcode::s_waitcnt, Format::SOPP, 0, 0);
      waitcnt->imm = imm.pack(ctx.chip_class);
      waitcnt->block = -1;
      instructions.emplace_back(waitcnt);
   }
   imm = wait_imm();
}

void
handle_block(Program* program, Block& block, wait_ctx& ctx)
{
   std::vector<aco_ptr<Instruction>> new_instructions;

   wait_imm queued_imm;

   for (aco_ptr<Instruction>& instr : block.instructions) {
      bool is_wait = parse_wait_instr(ctx, queued_imm, instr.get());

      memory_sync_info sync_info = get_sync_info(instr.get());
      kill(queued_imm, instr.get(), ctx, sync_info);

      gen(instr.get(), ctx);

      if (instr->format != Format::PSEUDO_BARRIER && !is_wait) {
         if (!queued_imm.empty())
            emit_waitcnt(ctx, new_instructions, queued_imm);

         new_instructions.emplace_back(std::move(instr));
         perform_barrier(ctx, queued_imm, sync_info, semantic_acquire);
      }
   }

   if (!queued_imm.empty())
      emit_waitcnt(ctx, new_instructions, queued_imm);

   block.instructions.swap(new_instructions);
}

} /* end namespace */

void
insert_wait_states(Program* program)
{
   /* per BB ctx */
   std::vector<bool> done(program->blocks.size());
   std::vector<wait_ctx> in_ctx(program->blocks.size(), wait_ctx(program));
   std::vector<wait_ctx> out_ctx(program->blocks.size(), wait_ctx(program));

   std::stack<unsigned, std::vector<unsigned>> loop_header_indices;
   unsigned loop_progress = 0;

   if (program->stage.has(SWStage::VS) && program->info->vs.dynamic_inputs) {
      for (Definition def : program->vs_inputs) {
         update_counters(in_ctx[0], event_vmem);
         insert_wait_entry(in_ctx[0], def, event_vmem);
      }
   }

   for (unsigned i = 0; i < program->blocks.size();) {
      Block& current = program->blocks[i++];
      wait_ctx ctx = in_ctx[current.index];

      if (current.kind & block_kind_loop_header) {
         loop_header_indices.push(current.index);
      } else if (current.kind & block_kind_loop_exit) {
         bool repeat = false;
         if (loop_progress == loop_header_indices.size()) {
            i = loop_header_indices.top();
            repeat = true;
         }
         loop_header_indices.pop();
         loop_progress = std::min<unsigned>(loop_progress, loop_header_indices.size());
         if (repeat)
            continue;
      }

      bool changed = false;
      for (unsigned b : current.linear_preds)
         changed |= ctx.join(&out_ctx[b], false);
      for (unsigned b : current.logical_preds)
         changed |= ctx.join(&out_ctx[b], true);

      if (done[current.index] && !changed) {
         in_ctx[current.index] = std::move(ctx);
         continue;
      } else {
         in_ctx[current.index] = ctx;
      }

      if (current.instructions.empty()) {
         out_ctx[current.index] = std::move(ctx);
         continue;
      }

      loop_progress = std::max<unsigned>(loop_progress, current.loop_nest_depth);
      done[current.index] = true;

      handle_block(program, current, ctx);

      out_ctx[current.index] = std::move(ctx);
   }
}

} // namespace aco
