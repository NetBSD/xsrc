/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgapipeline.c,v 1.4 2001/04/10 16:07:50 dawes Exp $ */
/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */

#include <stdio.h>
#include "mgavb.h"
#include "mgadd.h"
#include "mgacontext.h"
#include "mgatris.h"
#include "mgapipeline.h"
#include "fog.h"

static struct gl_pipeline_stage mga_fast_stage = {
   "MGA fast path",
   (PIPE_OP_VERT_XFORM|PIPE_OP_RAST_SETUP_0|
    PIPE_OP_RAST_SETUP_1|PIPE_OP_RENDER),
   PIPE_PRECALC,
   0, 0, 0, 0, 0, 0, 0, 0, 0,
   mgaDDFastPath
};


#define ILLEGAL_ENABLES (TEXTURE0_3D|		\
			 TEXTURE1_3D|		\
			 ENABLE_TEXMAT0 |	\
			 ENABLE_TEXMAT1 |	\
			 ENABLE_TEXGEN0 |	\
			 ENABLE_TEXGEN1 |	\
			 ENABLE_USERCLIP |	\
			 ENABLE_LIGHT |		\
			 ENABLE_FOG)


/* The driver gets first shot at building the pipeline - make some
 * quick tests to see if we can use the fast path.
 */
GLboolean mgaDDBuildPrecalcPipeline( GLcontext *ctx )
{
   struct gl_pipeline *pipe = &ctx->CVA.pre;
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );

   if (mmesa->renderindex == 0 &&
       (ctx->Enabled & ILLEGAL_ENABLES) == 0 &&
       (ctx->Array.Flags & (VERT_OBJ_234|
			    VERT_TEX0_4|
			    VERT_TEX1_4|
			    VERT_ELT)) == (VERT_OBJ_23|VERT_ELT))
   {
      pipe->stages[0] = &mga_fast_stage;
      pipe->stages[1] = 0;
      pipe->new_inputs = ctx->RenderFlags & VERT_DATA;
      pipe->ops = pipe->stages[0]->ops;
      mmesa->using_fast_path = 1;
      return 1;
   }

   if (mmesa->using_fast_path)
   {
      mmesa->using_fast_path = 0;
      ctx->CVA.VB->ClipOrMask = 0;
      ctx->CVA.VB->ClipAndMask = CLIP_ALL_BITS;
      ctx->Array.NewArrayState |= ctx->Array.Summary;
      return 0;
   }

   return 0;
}




/* Still do the normal fixup and copy-to-current, so this isn't so
 * bad.
 */
#define ILLEGAL_INPUTS_IMM (VERT_OBJ_4|		\
                            VERT_TEX0_4|	\
			    VERT_TEX1_4|	\
			    VERT_MATERIAL)


static void mgaDDCheckRasterSetup( GLcontext *ctx, struct gl_pipeline_stage *d )
{
   d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
   d->inputs = ctx->RenderFlags;

   /* MGA requires an extra input:
    */
   if (ctx->FogMode == FOG_FRAGMENT)
      d->inputs |= VERT_FOG_COORD;

   d->outputs = VERT_SETUP_FULL;

   if (ctx->IndirectTriangles & DD_SW_SETUP)
      d->type = PIPE_IMMEDIATE;
}


GLuint mgaDDRegisterPipelineStages( struct gl_pipeline_stage *out,
				    const struct gl_pipeline_stage *in,
				    GLuint nr )
{
   GLuint i, o;

   for (i = o = 0 ; i < nr ; i++) {
      switch (in[i].ops) {

	 /* Completely replace Mesa's fog processing to generate fog
	  * coordinates instead of messing with colors.
	  */
      case PIPE_OP_FOG:
	 out[o] = gl_fog_coord_stage;
	 o++;
	 break;

      case PIPE_OP_RAST_SETUP_0:
	 out[o] = in[i];
	 out[o].cva_state_change = NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS;
	 out[o].state_change = ~0;
	 out[o].check = mgaDDCheckPartialRasterSetup;
	 out[o].run = mgaDDPartialRasterSetup;
	 o++;
	 break;

      case PIPE_OP_RAST_SETUP_0|PIPE_OP_RAST_SETUP_1:
	 out[o] = in[i];
	 out[o].check = mgaDDCheckRasterSetup;
	 out[o].run = mgaDDDoRasterSetup;
	 o++;
	 break;

      default:
	 out[o++] = in[i];
	 break;
      }
   }

   return o;
}
