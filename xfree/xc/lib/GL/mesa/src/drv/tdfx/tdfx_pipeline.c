/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
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
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_pipeline.c,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

#include "tdfx_context.h"
#include "tdfx_pipeline.h"
#include "tdfx_vb.h"
#include "tdfx_tris.h"

#include "vbindirect.h"

static struct gl_pipeline_stage tdfx_fast_stage = {
   "TDFX Fast Path",
   (PIPE_OP_VERT_XFORM |
    PIPE_OP_RAST_SETUP_0 |
    PIPE_OP_RAST_SETUP_1 |
    PIPE_OP_RENDER),
   PIPE_PRECALC,
   0, 0, 0, 0, 0, 0, 0, 0, 0,
   tdfxDDFastPath
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
GLboolean tdfxDDBuildPrecalcPipeline( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   struct gl_pipeline *pipe = &ctx->CVA.pre;

   if ( (fxMesa->RenderIndex & ~TDFX_CLIPRECT_BIT) == 0 &&
	(ctx->Enabled & ILLEGAL_ENABLES) == 0 &&
#ifdef VAO
	(ctx->Array.Current->Flags & (VERT_OBJ_234 |
#else
	(ctx->Array.Flags & (VERT_OBJ_234 |
#endif
			     VERT_TEX0_4  |
			     VERT_TEX1_4  |
			     VERT_ELT)) == (VERT_OBJ_23 | VERT_ELT) )
   {
      pipe->stages[0] = &tdfx_fast_stage;
      pipe->stages[1] = 0;
      pipe->new_inputs = ctx->RenderFlags & VERT_DATA;
      pipe->ops = pipe->stages[0]->ops;

      fxMesa->using_fast_path = GL_TRUE;
      return GL_TRUE;
   }

   if ( fxMesa->using_fast_path ) {
      fxMesa->using_fast_path = GL_FALSE;

      ctx->CVA.VB->ClipOrMask = 0;
      ctx->CVA.VB->ClipAndMask = CLIP_ALL_BITS;
#ifdef VAO
      ctx->Array.NewArrayState |= ctx->Array.Current->Summary;
#else
      ctx->Array.NewArrayState |= ctx->Array.Summary;
#endif
   }

   return GL_FALSE;
}


static void tdfxDDCheckRasterSetup( GLcontext *ctx,
				    struct gl_pipeline_stage *d )
{
   d->type = PIPE_IMMEDIATE | PIPE_PRECALC;
   d->inputs = ctx->RenderFlags;

   d->outputs = VERT_SETUP_FULL;

   if ( ctx->IndirectTriangles & DD_SW_SETUP )
      d->type = PIPE_IMMEDIATE;
}


static void tdfxDDRenderElements(struct vertex_buffer *VB)
{
    if (VB->ClipOrMask) {
       gl_render_elts( VB );
    } else {
       GLcontext *ctx = VB->ctx;
       tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
       fxMesa->RenderElementsRaw( VB );
    }
}


/* Register the pipeline with our stages included */
GLuint tdfxDDRegisterPipelineStages( struct gl_pipeline_stage *out,
				     const struct gl_pipeline_stage *in,
				     GLuint nr )
{
   int i, o;

   for ( i = o = 0 ; i < nr ; i++ ) {
      switch ( in[i].ops ) {

      case PIPE_OP_RAST_SETUP_0:
	 out[o] = in[i];
	 out[o].cva_state_change = (NEW_LIGHTING |
				    NEW_TEXTURING |
				    NEW_RASTER_OPS);
	 out[o].state_change = ~0;
	 out[o].check = tdfxDDCheckPartialRasterSetup;
	 out[o].run = tdfxDDPartialRasterSetup;
	 o++;
	 break;

      case PIPE_OP_RAST_SETUP_0 | PIPE_OP_RAST_SETUP_1:
	 out[o] = in[i];
	 out[o].check = tdfxDDCheckRasterSetup;
	 out[o].run = tdfxDDDoRasterSetup;
	 o++;
	 break;

      case PIPE_OP_RENDER:
	 out[o] = in[i];
	 if (in[i].type == PIPE_PRECALC)
	    out[o].run = tdfxDDRenderElements;
	 o++;
	 break;

      default:
	 out[o++] = in[i];
	 break;
      }
   }

   return o;
}
