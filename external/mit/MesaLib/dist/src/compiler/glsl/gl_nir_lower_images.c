/*
 * Copyright © 2019 Red Hat Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file
 *
 * Lower image operations by turning the image_deref_* into a image_* on an
 * index number or bindless_image_* intrinsic on a load_deref of the previous
 * deref source. All applicable indicies are also set so that fetching the
 * variable in the backend wouldn't be needed anymore.
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_deref.h"

#include "compiler/glsl/gl_nir.h"

static void
type_size_align_1(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   unsigned s;

   if (glsl_type_is_array(type))
      s = glsl_get_aoa_size(type);
   else
      s = 1;

   *size = s;
   *align = s;
}

static bool
lower_impl(nir_builder *b, nir_instr *instr, bool bindless_only)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(instr);

   nir_deref_instr *deref;
   nir_variable *var;

   switch (intrinsic->intrinsic) {
   case nir_intrinsic_image_deref_atomic_add:
   case nir_intrinsic_image_deref_atomic_imin:
   case nir_intrinsic_image_deref_atomic_umin:
   case nir_intrinsic_image_deref_atomic_imax:
   case nir_intrinsic_image_deref_atomic_umax:
   case nir_intrinsic_image_deref_atomic_and:
   case nir_intrinsic_image_deref_atomic_or:
   case nir_intrinsic_image_deref_atomic_xor:
   case nir_intrinsic_image_deref_atomic_exchange:
   case nir_intrinsic_image_deref_atomic_comp_swap:
   case nir_intrinsic_image_deref_atomic_fadd:
   case nir_intrinsic_image_deref_atomic_inc_wrap:
   case nir_intrinsic_image_deref_atomic_dec_wrap:
   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_samples:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_image_deref_store: {
      deref = nir_src_as_deref(intrinsic->src[0]);
      var = nir_deref_instr_get_variable(deref);
      break;
   }
   default:
      return false;
   }

   bool bindless = var->data.mode != nir_var_uniform || var->data.bindless;
   if (bindless_only && !bindless)
      return false;

   b->cursor = nir_before_instr(instr);

   nir_ssa_def *src;
   if (bindless) {
      src = nir_load_deref(b, deref);
   } else {
      src = nir_iadd_imm(b,
                         nir_build_deref_offset(b, deref, type_size_align_1),
                         var->data.driver_location);
   }
   nir_rewrite_image_intrinsic(intrinsic, src, bindless);

   return true;
}

bool
gl_nir_lower_images(nir_shader *shader, bool bindless_only)
{
   bool progress = false;

   nir_foreach_function(function, shader) {
      if (function->impl) {
         nir_builder b;
         nir_builder_init(&b, function->impl);

         bool impl_progress = false;
         nir_foreach_block(block, function->impl)
            nir_foreach_instr(instr, block)
               impl_progress |= lower_impl(&b, instr, bindless_only);

         if (impl_progress) {
            nir_metadata_preserve(function->impl,
                                  nir_metadata_block_index |
                                  nir_metadata_dominance);
            progress = true;
         } else {
            nir_metadata_preserve(function->impl, nir_metadata_all);
         }
      }
   }

   return progress;
}
