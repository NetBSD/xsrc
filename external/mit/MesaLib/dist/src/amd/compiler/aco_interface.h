/*
 * Copyright © 2018 Google
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

#ifndef ACO_INTERFACE_H
#define ACO_INTERFACE_H

#include "nir.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ac_shader_config;

struct aco_compiler_statistic_info {
   char name[32];
   char desc[64];
};

extern const unsigned aco_num_statistics;
extern const struct aco_compiler_statistic_info* aco_statistic_infos;

void aco_compile_shader(unsigned shader_count, struct nir_shader* const* shaders,
                        struct radv_shader_binary** binary, const struct radv_shader_args* args);

void aco_compile_vs_prolog(const struct radv_vs_prolog_key* key, struct radv_prolog_binary** binary,
                           const struct radv_shader_args* args);

#ifdef __cplusplus
}
#endif

#endif
