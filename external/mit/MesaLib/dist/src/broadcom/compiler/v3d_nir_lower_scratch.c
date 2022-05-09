/*
 * Copyright © 2018 Intel Corporation
 * Copyright © 2018 Broadcom
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

#include "v3d_compiler.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"

/** @file v3d_nir_lower_scratch.c
 *
 * Swizzles around the addresses of
 * nir_intrinsic_load_scratch/nir_intrinsic_store_scratch so that a QPU stores
 * a cacheline at a time per dword of scratch access, scalarizing and removing
 * writemasks in the process.
 */

static nir_ssa_def *
v3d_nir_scratch_offset(nir_builder *b, nir_intrinsic_instr *instr)
{
        bool is_store = instr->intrinsic == nir_intrinsic_store_scratch;
        nir_ssa_def *offset = nir_ssa_for_src(b, instr->src[is_store ? 1 : 0], 1);

        assert(nir_intrinsic_align_mul(instr) >= 4);
        assert(nir_intrinsic_align_offset(instr) == 0);

        /* The spill_offset register will already have the subgroup ID (EIDX)
         * shifted and ORed in at bit 2, so all we need to do is to move the
         * dword index up above V3D_CHANNELS.
         */
        return nir_imul_imm(b, offset, V3D_CHANNELS);
}

static void
v3d_nir_lower_load_scratch(nir_builder *b, nir_intrinsic_instr *instr)
{
        b->cursor = nir_before_instr(&instr->instr);

        nir_ssa_def *offset = v3d_nir_scratch_offset(b,instr);

        nir_ssa_def *chans[NIR_MAX_VEC_COMPONENTS];
        for (int i = 0; i < instr->num_components; i++) {
                nir_ssa_def *chan_offset =
                        nir_iadd_imm(b, offset, V3D_CHANNELS * i * 4);

                nir_intrinsic_instr *chan_instr =
                        nir_intrinsic_instr_create(b->shader, instr->intrinsic);
                chan_instr->num_components = 1;
                nir_ssa_dest_init(&chan_instr->instr, &chan_instr->dest, 1,
                                  instr->dest.ssa.bit_size, NULL);

                chan_instr->src[0] = nir_src_for_ssa(chan_offset);

                nir_intrinsic_set_align(chan_instr, 4, 0);

                nir_builder_instr_insert(b, &chan_instr->instr);

                chans[i] = &chan_instr->dest.ssa;
        }

        nir_ssa_def *result = nir_vec(b, chans, instr->num_components);
        nir_ssa_def_rewrite_uses(&instr->dest.ssa, result);
        nir_instr_remove(&instr->instr);
}

static void
v3d_nir_lower_store_scratch(nir_builder *b, nir_intrinsic_instr *instr)
{
        b->cursor = nir_before_instr(&instr->instr);

        nir_ssa_def *offset = v3d_nir_scratch_offset(b, instr);
        nir_ssa_def *value = nir_ssa_for_src(b, instr->src[0],
                                             instr->num_components);

        for (int i = 0; i < instr->num_components; i++) {
                if (!(nir_intrinsic_write_mask(instr) & (1 << i)))
                        continue;

                nir_ssa_def *chan_offset =
                        nir_iadd_imm(b, offset, V3D_CHANNELS * i * 4);

                nir_intrinsic_instr *chan_instr =
                        nir_intrinsic_instr_create(b->shader, instr->intrinsic);
                chan_instr->num_components = 1;

                chan_instr->src[0] = nir_src_for_ssa(nir_channel(b,
                                                                 value,
                                                                 i));
                chan_instr->src[1] = nir_src_for_ssa(chan_offset);
                nir_intrinsic_set_write_mask(chan_instr, 0x1);
                nir_intrinsic_set_align(chan_instr, 4, 0);

                nir_builder_instr_insert(b, &chan_instr->instr);
        }

        nir_instr_remove(&instr->instr);
}

void
v3d_nir_lower_scratch(nir_shader *s)
{
        nir_foreach_function(function, s) {
                if (!function->impl)
                        continue;

                nir_builder b;
                nir_builder_init(&b, function->impl);

                nir_foreach_block(block, function->impl) {
                        nir_foreach_instr_safe(instr, block) {
                                if (instr->type != nir_instr_type_intrinsic)
                                        continue;

                                nir_intrinsic_instr *intr =
                                        nir_instr_as_intrinsic(instr);

                                switch (intr->intrinsic) {
                                case nir_intrinsic_load_scratch:
                                        v3d_nir_lower_load_scratch(&b, intr);
                                        break;
                                case nir_intrinsic_store_scratch:
                                        v3d_nir_lower_store_scratch(&b, intr);
                                        break;
                                default:
                                        break;
                                }
                        }
                }

                nir_metadata_preserve(function->impl,
                                      nir_metadata_block_index |
                                      nir_metadata_dominance);
        }
}
