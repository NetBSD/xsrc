/*
 * Copyright 2019 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef AC_SHADER_ARGS_H
#define AC_SHADER_ARGS_H

#include <stdbool.h>
#include <stdint.h>

#define AC_MAX_INLINE_PUSH_CONSTS 8

enum ac_arg_regfile
{
   AC_ARG_SGPR,
   AC_ARG_VGPR,
};

enum ac_arg_type
{
   AC_ARG_FLOAT,
   AC_ARG_INT,
   AC_ARG_CONST_PTR,       /* Pointer to i8 array */
   AC_ARG_CONST_FLOAT_PTR, /* Pointer to f32 array */
   AC_ARG_CONST_PTR_PTR,   /* Pointer to pointer to i8 array */
   AC_ARG_CONST_DESC_PTR,  /* Pointer to v4i32 array */
   AC_ARG_CONST_IMAGE_PTR, /* Pointer to v8i32 array */
};

struct ac_arg {
   uint16_t arg_index;
   bool used;
};

#define AC_MAX_ARGS 384 /* including all VS->TCS IO */

struct ac_shader_args {
   /* Info on how to declare arguments */
   struct {
      enum ac_arg_type type;
      enum ac_arg_regfile file;
      uint8_t offset;
      uint8_t size;
      bool skip;
   } args[AC_MAX_ARGS];

   uint16_t arg_count;
   uint16_t num_sgprs_used;
   uint16_t num_vgprs_used;

   uint16_t return_count;
   uint16_t num_sgprs_returned;
   uint16_t num_vgprs_returned;

   /* VS */
   struct ac_arg base_vertex;
   struct ac_arg start_instance;
   struct ac_arg draw_id;
   struct ac_arg vertex_buffers;
   struct ac_arg vertex_id;
   struct ac_arg vs_rel_patch_id;
   struct ac_arg vs_prim_id;
   struct ac_arg instance_id;

   /* Merged shaders */
   struct ac_arg tess_offchip_offset;
   struct ac_arg merged_wave_info;
   /* On gfx10:
    *  - bits 0..11: ordered_wave_id
    *  - bits 12..20: number of vertices in group
    *  - bits 22..30: number of primitives in group
    */
   struct ac_arg gs_tg_info;
   struct ac_arg scratch_offset;

   /* TCS */
   struct ac_arg tcs_factor_offset;
   struct ac_arg tcs_patch_id;
   struct ac_arg tcs_rel_ids;

   /* TES */
   struct ac_arg tes_u;
   struct ac_arg tes_v;
   struct ac_arg tes_rel_patch_id;
   struct ac_arg tes_patch_id;

   /* GS */
   struct ac_arg es2gs_offset;      /* separate legacy ES */
   struct ac_arg gs2vs_offset;      /* legacy GS */
   struct ac_arg gs_wave_id;        /* legacy GS */
   struct ac_arg gs_vtx_offset[6];  /* GFX6-8: [0-5], GFX9+: [0-2] packed */
   struct ac_arg gs_prim_id;
   struct ac_arg gs_invocation_id;

   /* Streamout */
   struct ac_arg streamout_config;
   struct ac_arg streamout_write_index;
   struct ac_arg streamout_offset[4];

   /* PS */
   struct ac_arg frag_pos[4];
   struct ac_arg front_face;
   struct ac_arg ancillary;
   struct ac_arg sample_coverage;
   struct ac_arg prim_mask;
   struct ac_arg persp_sample;
   struct ac_arg persp_center;
   struct ac_arg persp_centroid;
   struct ac_arg pull_model;
   struct ac_arg linear_sample;
   struct ac_arg linear_center;
   struct ac_arg linear_centroid;

   /* CS */
   struct ac_arg local_invocation_ids;
   struct ac_arg num_work_groups;
   struct ac_arg workgroup_ids[3];
   struct ac_arg tg_size;

   /* Vulkan only */
   struct ac_arg push_constants;
   struct ac_arg inline_push_consts[AC_MAX_INLINE_PUSH_CONSTS];
   unsigned base_inline_push_consts;
   struct ac_arg view_index;
   struct ac_arg sbt_descriptors;
   struct ac_arg ray_launch_size;
};

void ac_add_arg(struct ac_shader_args *info, enum ac_arg_regfile regfile, unsigned registers,
                enum ac_arg_type type, struct ac_arg *arg);
void ac_add_return(struct ac_shader_args *info, enum ac_arg_regfile regfile);

#endif
