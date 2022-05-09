/*
 * Copyright © 2020 Google LLC
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

/**
 * @file
 *
 * Trims off the unused trailing components of SSA defs.
 *
 * Due to various optimization passes (or frontend implementations,
 * particularly prog_to_nir), we may have instructions generating vectors
 * whose components don't get read by any instruction. As it can be tricky
 * to eliminate unused low components or channels in the middle of a writemask
 * (you might need to increment some offset from a load_uniform, for example),
 * it is trivial to just drop the trailing components. For vector ALU only used
 * by ALU, this pass eliminates arbitrary channels and reswizzles the uses.
 *
 * This pass is probably only of use to vector backends -- scalar backends
 * typically get unused def channel trimming by scalarizing and dead code
 * elimination.
 */

#include "nir.h"
#include "nir_builder.h"

static bool
shrink_dest_to_read_mask(nir_ssa_def *def)
{
   /* early out if there's nothing to do. */
   if (def->num_components == 1)
      return false;

   /* don't remove any channels if used by an intrinsic */
   nir_foreach_use(use_src, def) {
      if (use_src->parent_instr->type == nir_instr_type_intrinsic)
         return false;
   }

   unsigned mask = nir_ssa_def_components_read(def);
   int last_bit = util_last_bit(mask);

   /* If nothing was read, leave it up to DCE. */
   if (!mask)
      return false;

   if (def->num_components > last_bit) {
      def->num_components = last_bit;
      return true;
   }

   return false;
}

static bool
opt_shrink_vectors_alu(nir_builder *b, nir_alu_instr *instr)
{
   nir_ssa_def *def = &instr->dest.dest.ssa;

   /* Nothing to shrink */
   if (def->num_components == 1)
      return false;

   bool is_vec = false;
   switch (instr->op) {
      /* don't use nir_op_is_vec() as not all vector sizes are supported. */
      case nir_op_vec4:
      case nir_op_vec3:
      case nir_op_vec2:
         is_vec = true;
         break;
      default:
         if (nir_op_infos[instr->op].output_size != 0)
            return false;
         break;
   }

   /* don't remove any channels if used by an intrinsic */
   nir_foreach_use(use_src, def) {
      if (use_src->parent_instr->type == nir_instr_type_intrinsic)
         return false;
   }

   unsigned mask = nir_ssa_def_components_read(def);
   unsigned last_bit = util_last_bit(mask);
   unsigned num_components = util_bitcount(mask);

   /* return, if there is nothing to do */
   if (mask == 0 || num_components == def->num_components)
      return false;

   const bool is_bitfield_mask = last_bit == num_components;

   if (is_vec) {
      /* replace vecN with smaller version */
      nir_ssa_def *srcs[NIR_MAX_VEC_COMPONENTS] = { 0 };
      unsigned index = 0;
      for (int i = 0; i < last_bit; i++) {
         if ((mask >> i) & 0x1)
            srcs[index++] = nir_ssa_for_alu_src(b, instr, i);
      }
      assert(index == num_components);
      nir_ssa_def *new_vec = nir_vec(b, srcs, num_components);
      nir_ssa_def_rewrite_uses(def, new_vec);
      def = new_vec;
   }

   if (is_bitfield_mask) {
      /* just reduce the number of components and return */
      def->num_components = num_components;
      instr->dest.write_mask = mask;
      return true;
   }

   if (!is_vec) {
      /* update sources */
      for (int i = 0; i < nir_op_infos[instr->op].num_inputs; i++) {
         unsigned index = 0;
         for (int j = 0; j < last_bit; j++) {
            if ((mask >> j) & 0x1)
               instr->src[i].swizzle[index++] = instr->src[i].swizzle[j];
         }
         assert(index == num_components);
      }

      /* update dest */
      def->num_components = num_components;
      instr->dest.write_mask = BITFIELD_MASK(num_components);
   }

   /* compute new dest swizzles */
   uint8_t reswizzle[NIR_MAX_VEC_COMPONENTS] = { 0 };
   unsigned index = 0;
   for (int i = 0; i < last_bit; i++) {
      if ((mask >> i) & 0x1)
         reswizzle[i] = index++;
   }
   assert(index == num_components);

   /* update uses */
   nir_foreach_use(use_src, def) {
      assert(use_src->parent_instr->type == nir_instr_type_alu);
      nir_alu_src *alu_src = (nir_alu_src*)use_src;
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         alu_src->swizzle[i] = reswizzle[alu_src->swizzle[i]];
   }

   return true;
}

static bool
opt_shrink_vectors_image_store(nir_builder *b, nir_intrinsic_instr *instr)
{
   enum pipe_format format;
   if (instr->intrinsic == nir_intrinsic_image_deref_store) {
      nir_deref_instr *deref = nir_src_as_deref(instr->src[0]);
      format = nir_deref_instr_get_variable(deref)->data.image.format;
   } else {
      format = nir_intrinsic_format(instr);
   }
   if (format == PIPE_FORMAT_NONE)
      return false;

   unsigned components = util_format_get_nr_components(format);
   if (components >= instr->num_components)
      return false;

   nir_ssa_def *data = nir_channels(b, instr->src[3].ssa, BITSET_MASK(components));
   nir_instr_rewrite_src(&instr->instr, &instr->src[3], nir_src_for_ssa(data));
   instr->num_components = components;

   return true;
}

static bool
opt_shrink_vectors_intrinsic(nir_builder *b, nir_intrinsic_instr *instr, bool shrink_image_store)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_uniform:
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_input_vertex:
   case nir_intrinsic_load_per_vertex_input:
   case nir_intrinsic_load_interpolated_input:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_push_constant:
   case nir_intrinsic_load_constant:
   case nir_intrinsic_load_shared:
   case nir_intrinsic_load_global:
   case nir_intrinsic_load_global_constant:
   case nir_intrinsic_load_kernel_input:
   case nir_intrinsic_load_scratch:
   case nir_intrinsic_store_output:
   case nir_intrinsic_store_per_vertex_output:
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_store_shared:
   case nir_intrinsic_store_global:
   case nir_intrinsic_store_scratch:
      break;
   case nir_intrinsic_bindless_image_store:
   case nir_intrinsic_image_deref_store:
   case nir_intrinsic_image_store:
      return shrink_image_store && opt_shrink_vectors_image_store(b, instr);
   default:
      return false;
   }

   /* Must be a vectorized intrinsic that we can resize. */
   assert(instr->num_components != 0);

   if (nir_intrinsic_infos[instr->intrinsic].has_dest) {
      /* loads: Trim the dest to the used channels */

      if (shrink_dest_to_read_mask(&instr->dest.ssa)) {
         instr->num_components = instr->dest.ssa.num_components;
         return true;
      }
   } else {
      /* Stores: trim the num_components stored according to the write
       * mask.
       */
      unsigned write_mask = nir_intrinsic_write_mask(instr);
      unsigned last_bit = util_last_bit(write_mask);
      if (last_bit < instr->num_components && instr->src[0].is_ssa) {
         nir_ssa_def *def = nir_channels(b, instr->src[0].ssa,
                                         BITSET_MASK(last_bit));
         nir_instr_rewrite_src(&instr->instr,
                               &instr->src[0],
                               nir_src_for_ssa(def));
         instr->num_components = last_bit;

         return true;
      }
   }

   return false;
}

static bool
opt_shrink_vectors_load_const(nir_load_const_instr *instr)
{
   return shrink_dest_to_read_mask(&instr->def);
}

static bool
opt_shrink_vectors_ssa_undef(nir_ssa_undef_instr *instr)
{
   return shrink_dest_to_read_mask(&instr->def);
}

static bool
opt_shrink_vectors_instr(nir_builder *b, nir_instr *instr, bool shrink_image_store)
{
   b->cursor = nir_before_instr(instr);

   switch (instr->type) {
   case nir_instr_type_alu:
      return opt_shrink_vectors_alu(b, nir_instr_as_alu(instr));

   case nir_instr_type_intrinsic:
      return opt_shrink_vectors_intrinsic(b, nir_instr_as_intrinsic(instr), shrink_image_store);

   case nir_instr_type_load_const:
      return opt_shrink_vectors_load_const(nir_instr_as_load_const(instr));

   case nir_instr_type_ssa_undef:
      return opt_shrink_vectors_ssa_undef(nir_instr_as_ssa_undef(instr));

   default:
      return false;
   }

   return true;
}

bool
nir_opt_shrink_vectors(nir_shader *shader, bool shrink_image_store)
{
   bool progress = false;

   nir_foreach_function(function, shader) {
      if (!function->impl)
         continue;

      nir_builder b;
      nir_builder_init(&b, function->impl);

      nir_foreach_block_reverse(block, function->impl) {
         nir_foreach_instr_reverse(instr, block) {
            progress |= opt_shrink_vectors_instr(&b, instr, shrink_image_store);
         }
      }

      if (progress) {
         nir_metadata_preserve(function->impl,
                               nir_metadata_block_index |
                               nir_metadata_dominance);
      } else {
         nir_metadata_preserve(function->impl, nir_metadata_all);
      }
   }

   return progress;
}
