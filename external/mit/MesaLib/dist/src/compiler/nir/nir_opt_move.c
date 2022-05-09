/*
 * Copyright © 2016 Intel Corporation
 * Copyright © 2019 Valve Corporation
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
 */

#include "nir.h"

/**
 * \file nir_opt_move.c
 *
 * This pass can move various operations just before their first use inside the
 * same basic block. Usually this is to reduce register usage. It's probably
 * not a good idea to use this in an optimization loop.
 *
 * Moving comparisons is useful because many GPUs generate condition codes
 * for comparisons, and use predication for conditional selects and control
 * flow.  In a sequence such as:
 *
 *     vec1 32 ssa_1 = flt a b
 *     <some other operations>
 *     vec1 32 ssa_2 = bcsel ssa_1 c d
 *
 * the backend would likely do the comparison, producing condition codes,
 * then save those to a boolean value.  The intervening operations might
 * trash the condition codes.  Then, in order to do the bcsel, it would
 * need to re-populate the condition code register based on the boolean.
 *
 * By moving the comparison just before the bcsel, the condition codes could
 * be used directly.  This eliminates the need to reload them from the boolean
 * (generally eliminating an instruction).  It may also eliminate the need to
 * create a boolean value altogether (unless it's used elsewhere), which could
 * lower register pressure.
 */

static bool
move_source(nir_src *src, nir_block *block, nir_instr *before, nir_move_options options)
{
   if (!src->is_ssa)
      return false;

   nir_instr *src_instr = src->ssa->parent_instr;

   if (src_instr->block == block && nir_can_move_instr(src_instr, options)) {
      exec_node_remove(&src_instr->node);

      if (before)
         exec_node_insert_node_before(&before->node, &src_instr->node);
      else
         exec_list_push_tail(&block->instr_list, &src_instr->node);

      return true;
   }
   return false;
}

struct source_cb_data {
   bool *progress;
   nir_move_options options;
};

static bool
move_source_cb(nir_src *src, void *data_ptr)
{
   struct source_cb_data data = *(struct source_cb_data*)data_ptr;

   nir_instr *instr = src->parent_instr;
   if (move_source(src, instr->block, instr, data.options))
      *data.progress = true;

   return true; /* nir_foreach_src should keep going */
}

static bool
move(nir_block *block, nir_move_options options)
{
   bool progress = false;

   /* We use a simple approach: walk instructions backwards.
    *
    * If the instruction's source is a comparison from the same block,
    * simply move it here.  This may break SSA if it's used earlier in
    * the block as well.  However, as we walk backwards, we'll find the
    * earlier use and move it again, further up.  It eventually ends up
    * dominating all uses again, restoring SSA form.
    *
    * Before walking instructions, we consider the if-condition at the
    * end of the block, if one exists.  It's effectively a use at the
    * bottom of the block.
    */
   nir_if *iff = nir_block_get_following_if(block);
   if (iff) {
      progress |= move_source(&iff->condition, block, NULL, options);
   }

   nir_foreach_instr_reverse(instr, block) {
      /* The sources of phi instructions happen after the predecessor block
       * but before this block.  (Yes, that's between blocks).  This means
       * that we don't need to move them in order for them to be correct.
       * We could move them to encourage comparisons that are used in a phi to
       * the end of the block, doing so correctly would make the pass
       * substantially more complicated and wouldn't gain us anything since
       * the phi can't use a flag value anyway.
       */

      if (instr->type == nir_instr_type_phi) {
         /* We're going backwards so everything else is a phi too */
         break;
      } else if (instr->type == nir_instr_type_alu) {
         /* Walk ALU instruction sources backwards so that bcsel's boolean
          * condition is processed last for when comparisons are being moved.
          */
         nir_alu_instr *alu = nir_instr_as_alu(instr);
         for (int i = nir_op_infos[alu->op].num_inputs - 1; i >= 0; i--) {
            progress |= move_source(&alu->src[i].src, block, instr, options);
         }
      } else {
         struct source_cb_data data;
         data.progress = &progress;
         data.options = options;
         nir_foreach_src(instr, move_source_cb, &data);
      }
   }

   return progress;
}

bool
nir_opt_move(nir_shader *shader, nir_move_options options)
{
   bool progress = false;

   nir_foreach_function(func, shader) {
      if (!func->impl)
         continue;

      bool impl_progress = false;
      nir_foreach_block(block, func->impl) {
         if (move(block, options))
            impl_progress = true;
      }

      if (impl_progress) {
         nir_metadata_preserve(func->impl, nir_metadata_block_index |
                                           nir_metadata_dominance |
                                           nir_metadata_live_ssa_defs);
         progress = true;
      } else {
         nir_metadata_preserve(func->impl, nir_metadata_all);
      }
   }

   return progress;
}
