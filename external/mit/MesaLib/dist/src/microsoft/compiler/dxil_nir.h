/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef DXIL_NIR_H
#define DXIL_NIR_H

#include <stdbool.h>
#include "nir.h"
#include "nir_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

bool dxil_nir_lower_8bit_conv(nir_shader *shader);
bool dxil_nir_lower_16bit_conv(nir_shader *shader);
bool dxil_nir_lower_x2b(nir_shader *shader);
bool dxil_nir_lower_inot(nir_shader *shader);
bool dxil_nir_lower_ubo_to_temp(nir_shader *shader);
bool dxil_nir_lower_loads_stores_to_dxil(nir_shader *shader);
bool dxil_nir_lower_atomics_to_dxil(nir_shader *shader);
bool dxil_nir_lower_deref_ssbo(nir_shader *shader);
bool dxil_nir_opt_alu_deref_srcs(nir_shader *shader);
bool dxil_nir_lower_memcpy_deref(nir_shader *shader);
bool dxil_nir_lower_upcast_phis(nir_shader *shader, unsigned min_bit_size);
bool dxil_nir_lower_fp16_casts(nir_shader *shader);
bool dxil_nir_split_clip_cull_distance(nir_shader *shader);
bool dxil_nir_lower_double_math(nir_shader *shader);
bool dxil_nir_lower_system_values_to_zero(nir_shader *shader,
                                          gl_system_value* system_value,
                                          uint32_t count);
bool dxil_nir_create_bare_samplers(nir_shader *shader);
bool dxil_nir_lower_bool_input(struct nir_shader *s);

nir_ssa_def *
build_load_ubo_dxil(nir_builder *b, nir_ssa_def *buffer,
                    nir_ssa_def *offset, unsigned num_components,
                    unsigned bit_size);

uint64_t
dxil_sort_by_driver_location(nir_shader* s, nir_variable_mode modes);

void
dxil_sort_ps_outputs(nir_shader* s);

uint64_t
dxil_reassign_driver_locations(nir_shader* s, nir_variable_mode modes,
   uint64_t other_stage_mask);

#ifdef __cplusplus
}
#endif

#endif /* DXIL_NIR_H */
