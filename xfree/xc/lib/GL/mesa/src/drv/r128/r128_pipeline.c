/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_pipeline.c,v 1.3 2000/12/04 19:21:46 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "r128_context.h"
#include "r128_vb.h"
#include "r128_pipeline.h"

#include "types.h"
#include "fog.h"

static struct gl_pipeline_stage r128_fast_stage = {
   "R128 Fast Path",
   (PIPE_OP_VERT_XFORM |
    PIPE_OP_RAST_SETUP_0 |
    PIPE_OP_RAST_SETUP_1 |
    PIPE_OP_RENDER),
   PIPE_PRECALC,
   0, 0, 0, 0, 0, 0, 0, 0, 0,
   r128DDFastPath
};

#define ILLEGAL_ENABLES (TEXTURE0_3D |		\
			 TEXTURE1_3D |		\
			 ENABLE_TEXMAT0 |	\
			 ENABLE_TEXMAT1 |	\
			 ENABLE_TEXGEN0 |	\
			 ENABLE_TEXGEN1 |	\
			 ENABLE_USERCLIP |	\
			 ENABLE_LIGHT |		\
			 ENABLE_FOG)

/* Build the PRECALC pipeline with our stage, if possible.  Otherwise,
 * return GL_FALSE.
 */
GLboolean r128DDBuildPrecalcPipeline( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);
   struct gl_pipeline *pipe = &ctx->CVA.pre;

   if ( r128ctx->RenderIndex == 0 &&
	(ctx->Enabled & ILLEGAL_ENABLES) == 0 &&
	(ctx->Array.Flags & (VERT_OBJ_234 |
			     VERT_TEX0_4  |
			     VERT_TEX1_4  |
			     VERT_ELT)) == (VERT_OBJ_23 | VERT_ELT) )
   {
      pipe->stages[0]  = &r128_fast_stage;
      pipe->stages[1]  = 0;
      pipe->new_inputs = ctx->RenderFlags & VERT_DATA;
      pipe->ops        = pipe->stages[0]->ops;

      r128ctx->useFastPath = GL_TRUE;
      return GL_TRUE;
   }

   if ( r128ctx->useFastPath ) {
      r128ctx->useFastPath = GL_FALSE;

      ctx->CVA.VB->ClipOrMask   = 0;
      ctx->CVA.VB->ClipAndMask  = CLIP_ALL_BITS;
      ctx->Array.NewArrayState |= ctx->Array.Summary;
   }

   return GL_FALSE;
}


/* Still do the normal fixup and copy-to-current, so this isn't so
 * bad.
 */
#define ILLEGAL_INPUTS_IMM (VERT_OBJ_4 |	\
                            VERT_TEX0_4 |	\
			    VERT_TEX1_4 |	\
			    VERT_MATERIAL)

static void r128DDCheckRasterSetup( GLcontext *ctx,
				    struct gl_pipeline_stage *d )
{
   d->type = PIPE_IMMEDIATE | PIPE_PRECALC;
   d->inputs = ctx->RenderFlags;

   /* r128 requires an extra input:
    */
   if ( ctx->FogMode == FOG_FRAGMENT )
      d->inputs |= VERT_FOG_COORD;

   d->outputs = VERT_SETUP_FULL;

   if ( ctx->IndirectTriangles & DD_SW_SETUP )
      d->type = PIPE_IMMEDIATE;
}


/* Register the pipeline with our stages included */
GLuint r128DDRegisterPipelineStages( struct gl_pipeline_stage *out,
				     const struct gl_pipeline_stage *in,
				     GLuint nr )
{
   int i, o;

   for ( i = o = 0 ; i < nr ; i++ ) {
      switch ( in[i].ops ) {
	 /* Completely replace Mesa's fog processing to generate fog
	  * coordinates instead of messing with colors.
	  */
      case PIPE_OP_FOG:
	 out[o] = gl_fog_coord_stage;
	 o++;
	 break;

      case PIPE_OP_RAST_SETUP_0:
	 out[o] = in[i];
	 out[o].cva_state_change = (NEW_LIGHTING |
				    NEW_TEXTURING |
				    NEW_RASTER_OPS);
	 out[o].state_change = ~0;
	 out[o].check = r128DDCheckPartialRasterSetup;
	 out[o].run = r128DDPartialRasterSetup;
	 o++;
	 break;

      case PIPE_OP_RAST_SETUP_0 | PIPE_OP_RAST_SETUP_1:
	 out[o] = in[i];
	 out[o].check = r128DDCheckRasterSetup;
	 out[o].run = r128DDDoRasterSetup;
	 o++;
	 break;

      default:
	 out[o++] = in[i];
	 break;
      }
   }

   return o;
}
