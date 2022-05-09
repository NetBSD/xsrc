/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * State validation for vertex/fragment shaders.
 * Note that we have to delay most vertex/fragment shader translation
 * until rendering time since the linkage between the vertex outputs and
 * fragment inputs can vary depending on the pairing of shaders.
 *
 * Authors:
 *   Brian Paul
 */


#include "main/mtypes.h"
#include "main/framebuffer.h"
#include "main/state.h"
#include "main/texobj.h"
#include "main/texstate.h"
#include "program/program.h"

#include "pipe/p_context.h"
#include "pipe/p_shader_tokens.h"
#include "util/u_simple_shaders.h"
#include "cso_cache/cso_context.h"
#include "util/u_debug.h"

#include "st_context.h"
#include "st_atom.h"
#include "st_program.h"
#include "st_texture.h"
#include "st_util.h"


static unsigned
get_texture_index(struct gl_context *ctx, const unsigned unit)
{
   struct gl_texture_object *texObj = _mesa_get_tex_unit(ctx, unit)->_Current;
   gl_texture_index index;

   if (texObj) {
      index = _mesa_tex_target_to_index(ctx, texObj->Target);
   } else {
      /* fallback for missing texture */
      index = TEXTURE_2D_INDEX;
   }

   return index;
}


static inline GLboolean
is_wrap_gl_clamp(GLint param)
{
   return param == GL_CLAMP || param == GL_MIRROR_CLAMP_EXT;
}

static void
update_gl_clamp(struct st_context *st, struct gl_program *prog, uint32_t *gl_clamp)
{
   if (!st->emulate_gl_clamp)
      return;

   gl_clamp[0] = gl_clamp[1] = gl_clamp[2] = 0;
   GLbitfield samplers_used = prog->SamplersUsed;
   unsigned unit;
   /* same as st_atom_sampler.c */
   for (unit = 0; samplers_used; unit++, samplers_used >>= 1) {
      unsigned tex_unit = prog->SamplerUnits[unit];
      if (samplers_used & 1 &&
          (st->ctx->Texture.Unit[tex_unit]._Current->Target != GL_TEXTURE_BUFFER ||
           st->texture_buffer_sampler)) {
         const struct gl_texture_object *texobj;
         struct gl_context *ctx = st->ctx;
         const struct gl_sampler_object *msamp;

         texobj = ctx->Texture.Unit[tex_unit]._Current;
         assert(texobj);

         msamp = _mesa_get_samplerobj(ctx, tex_unit);
         if (is_wrap_gl_clamp(msamp->Attrib.WrapS))
            gl_clamp[0] |= BITFIELD64_BIT(unit);
         if (is_wrap_gl_clamp(msamp->Attrib.WrapT))
            gl_clamp[1] |= BITFIELD64_BIT(unit);
         if (is_wrap_gl_clamp(msamp->Attrib.WrapR))
            gl_clamp[2] |= BITFIELD64_BIT(unit);
      }
   }
}

/**
 * Update fragment program state/atom.  This involves translating the
 * Mesa fragment program into a gallium fragment program and binding it.
 */
void
st_update_fp( struct st_context *st )
{
   struct st_program *stfp;

   assert(st->ctx->FragmentProgram._Current);
   stfp = st_program(st->ctx->FragmentProgram._Current);
   assert(stfp->Base.Target == GL_FRAGMENT_PROGRAM_ARB);

   void *shader;

   if (st->shader_has_one_variant[MESA_SHADER_FRAGMENT] &&
       !stfp->ati_fs && /* ATI_fragment_shader always has multiple variants */
       !stfp->Base.ExternalSamplersUsed /* external samplers need variants */) {
      shader = stfp->variants->driver_shader;
   } else {
      struct st_fp_variant_key key;

      /* use memset, not an initializer to be sure all memory is zeroed */
      memset(&key, 0, sizeof(key));

      key.st = st->has_shareable_shaders ? NULL : st;

      key.lower_flatshade = st->lower_flatshade &&
                            st->ctx->Light.ShadeModel == GL_FLAT;

      /* _NEW_COLOR */
      key.lower_alpha_func = COMPARE_FUNC_ALWAYS;
      if (st->lower_alpha_test && _mesa_is_alpha_test_enabled(st->ctx))
         key.lower_alpha_func = st->ctx->Color.AlphaFunc;

      /* _NEW_LIGHT_STATE | _NEW_PROGRAM */
      key.lower_two_sided_color = st->lower_two_sided_color &&
         _mesa_vertex_program_two_side_enabled(st->ctx);

      /* _NEW_POINT | _NEW_PROGRAM */
      if (st->lower_texcoord_replace && st->ctx->Point.PointSprite &&
          st->ctx->Point.CoordReplace)
         key.lower_texcoord_replace = st->ctx->Point.CoordReplace;

      /* gl_driver_flags::NewFragClamp */
      key.clamp_color = st->clamp_frag_color_in_shader &&
                        st->ctx->Color._ClampFragmentColor;

      /* _NEW_MULTISAMPLE | _NEW_BUFFERS */
      key.persample_shading =
         st->force_persample_in_shader &&
         _mesa_is_multisample_enabled(st->ctx) &&
         st->ctx->Multisample.SampleShading &&
         st->ctx->Multisample.MinSampleShadingValue *
         _mesa_geometric_samples(st->ctx->DrawBuffer) > 1;

      key.lower_depth_clamp =
         st->clamp_frag_depth_in_shader &&
         (st->ctx->Transform.DepthClampNear ||
          st->ctx->Transform.DepthClampFar);

      if (stfp->ati_fs) {
         key.fog = st->ctx->Fog._PackedEnabledMode;

         for (unsigned u = 0; u < MAX_NUM_FRAGMENT_REGISTERS_ATI; u++) {
            key.texture_index[u] = get_texture_index(st->ctx, u);
         }
      }

      key.external = st_get_external_sampler_key(st, &stfp->Base);
      update_gl_clamp(st, st->ctx->FragmentProgram._Current, key.gl_clamp);

      simple_mtx_lock(&st->ctx->Shared->Mutex);
      shader = st_get_fp_variant(st, stfp, &key)->base.driver_shader;
      simple_mtx_unlock(&st->ctx->Shared->Mutex);
   }

   st_reference_prog(st, &st->fp, stfp);

   cso_set_fragment_shader_handle(st->cso_context, shader);
}


/**
 * Update vertex program state/atom.  This involves translating the
 * Mesa vertex program into a gallium fragment program and binding it.
 */
void
st_update_vp( struct st_context *st )
{
   struct st_program *stvp;

   /* find active shader and params -- Should be covered by
    * ST_NEW_VERTEX_PROGRAM
    */
   assert(st->ctx->VertexProgram._Current);
   stvp = st_program(st->ctx->VertexProgram._Current);
   assert(stvp->Base.Target == GL_VERTEX_PROGRAM_ARB);

   if (st->shader_has_one_variant[MESA_SHADER_VERTEX] &&
       !st->vertdata_edgeflags) {
      st->vp_variant = st_common_variant(stvp->variants);
   } else {
      struct st_common_variant_key key;

      memset(&key, 0, sizeof(key));

      key.st = st->has_shareable_shaders ? NULL : st;

      /* When this is true, we will add an extra input to the vertex
       * shader translation (for edgeflags), an extra output with
       * edgeflag semantics, and extend the vertex shader to pass through
       * the input to the output.  We'll need to use similar logic to set
       * up the extra vertex_element input for edgeflags.
       */
      key.passthrough_edgeflags = st->vertdata_edgeflags;

      key.clamp_color = st->clamp_vert_color_in_shader &&
                        st->ctx->Light._ClampVertexColor &&
                        (stvp->Base.info.outputs_written &
                         (VARYING_SLOT_COL0 |
                          VARYING_SLOT_COL1 |
                          VARYING_SLOT_BFC0 |
                          VARYING_SLOT_BFC1));

      key.lower_depth_clamp =
            !st->gp && !st->tep &&
            st->clamp_frag_depth_in_shader &&
            (st->ctx->Transform.DepthClampNear ||
             st->ctx->Transform.DepthClampFar);

      if (key.lower_depth_clamp)
         key.clip_negative_one_to_one =
               st->ctx->Transform.ClipDepthMode == GL_NEGATIVE_ONE_TO_ONE;

      if (!st->ctx->GeometryProgram._Current &&
          !st->ctx->TessEvalProgram._Current) {
         /* _NEW_POINT */
         key.lower_point_size = st->lower_point_size &&
                                !st_point_size_per_vertex(st->ctx);

         /* _NEW_TRANSFORM */
         if (st->lower_ucp && st_user_clip_planes_enabled(st->ctx))
            key.lower_ucp = st->ctx->Transform.ClipPlanesEnabled;
      }

      update_gl_clamp(st, st->ctx->VertexProgram._Current, key.gl_clamp);

      simple_mtx_lock(&st->ctx->Shared->Mutex);
      st->vp_variant = st_get_common_variant(st, stvp, &key);
      simple_mtx_unlock(&st->ctx->Shared->Mutex);
   }

   st_reference_prog(st, &st->vp, stvp);

   cso_set_vertex_shader_handle(st->cso_context,
                                st->vp_variant->base.driver_shader);
}


static void *
st_update_common_program(struct st_context *st, struct gl_program *prog,
                         unsigned pipe_shader, struct st_program **dst)
{
   struct st_program *stp;

   if (!prog) {
      st_reference_prog(st, dst, NULL);
      return NULL;
   }

   stp = st_program(prog);
   st_reference_prog(st, dst, stp);

   if (st->shader_has_one_variant[prog->info.stage])
      return stp->variants->driver_shader;

   struct st_common_variant_key key;

   /* use memset, not an initializer to be sure all memory is zeroed */
   memset(&key, 0, sizeof(key));

   key.st = st->has_shareable_shaders ? NULL : st;

   if (pipe_shader == PIPE_SHADER_GEOMETRY ||
       pipe_shader == PIPE_SHADER_TESS_EVAL) {
      key.clamp_color = st->clamp_vert_color_in_shader &&
                        st->ctx->Light._ClampVertexColor &&
                        (stp->Base.info.outputs_written &
                         (VARYING_SLOT_COL0 |
                          VARYING_SLOT_COL1 |
                          VARYING_SLOT_BFC0 |
                          VARYING_SLOT_BFC1));

      key.lower_depth_clamp =
            (pipe_shader == PIPE_SHADER_GEOMETRY || !st->gp) &&
            st->clamp_frag_depth_in_shader &&
            (st->ctx->Transform.DepthClampNear ||
             st->ctx->Transform.DepthClampFar);

      if (key.lower_depth_clamp)
         key.clip_negative_one_to_one =
               st->ctx->Transform.ClipDepthMode == GL_NEGATIVE_ONE_TO_ONE;

      if (st->lower_ucp && st_user_clip_planes_enabled(st->ctx) &&
          pipe_shader == PIPE_SHADER_GEOMETRY)
         key.lower_ucp = st->ctx->Transform.ClipPlanesEnabled;

      key.lower_point_size = st->lower_point_size &&
                             !st_point_size_per_vertex(st->ctx);

   }

   update_gl_clamp(st, prog, key.gl_clamp);

   simple_mtx_lock(&st->ctx->Shared->Mutex);
   void *result = st_get_common_variant(st, stp, &key)->base.driver_shader;
   simple_mtx_unlock(&st->ctx->Shared->Mutex);

   return result;
}


void
st_update_gp(struct st_context *st)
{
   void *shader = st_update_common_program(st,
                                           st->ctx->GeometryProgram._Current,
                                           PIPE_SHADER_GEOMETRY, &st->gp);
   cso_set_geometry_shader_handle(st->cso_context, shader);
}


void
st_update_tcp(struct st_context *st)
{
   void *shader = st_update_common_program(st,
                                           st->ctx->TessCtrlProgram._Current,
                                           PIPE_SHADER_TESS_CTRL, &st->tcp);
   cso_set_tessctrl_shader_handle(st->cso_context, shader);
}


void
st_update_tep(struct st_context *st)
{
   void *shader = st_update_common_program(st,
                                           st->ctx->TessEvalProgram._Current,
                                           PIPE_SHADER_TESS_EVAL, &st->tep);
   cso_set_tesseval_shader_handle(st->cso_context, shader);
}


void
st_update_cp(struct st_context *st)
{
   void *shader = st_update_common_program(st,
                                           st->ctx->ComputeProgram._Current,
                                           PIPE_SHADER_COMPUTE, &st->cp);
   cso_set_compute_shader_handle(st->cso_context, shader);
}
