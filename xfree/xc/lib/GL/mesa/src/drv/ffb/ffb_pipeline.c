/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_pipeline.c,v 1.1 2000/06/20 05:08:39 dawes Exp $
 *
 * GLX Hardware Device Driver for Sun Creator/Creator3D
 * Copyright (C) 2000 David S. Miller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID MILLER, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    David S. Miller <davem@redhat.com>
 */

#include "ffb_xmesa.h"
#include "ffb_context.h"
#include "ffb_vb.h"
#include "ffb_pipeline.h"

#include "stages.h"
#include "pipeline.h"

GLboolean ffbDDBuildPrecalcPipeline(GLcontext *ctx)
{   
	/* This doesn't do anything interesting yet. */
	return 0;
}

static void ffbDDCheckRasterSetup(GLcontext *ctx, struct gl_pipeline_stage *d)
{
	d->type = PIPE_IMMEDIATE|PIPE_PRECALC;
	d->inputs = ctx->RenderFlags;
	d->outputs = VERT_SETUP_FULL;

	if (ctx->IndirectTriangles & DD_SW_SETUP)
		d->type = PIPE_IMMEDIATE;
}

GLuint ffbDDRegisterPipelineStages(struct gl_pipeline_stage *out,
				   const struct gl_pipeline_stage *in,
				   GLuint nr)
{
	GLuint i, o;

	for (i = o = 0 ; i < nr ; i++) {
		switch (in[i].ops) {
		case PIPE_OP_RAST_SETUP_0:
			out[o] = in[i];
			out[o].cva_state_change =
				NEW_LIGHTING|NEW_TEXTURING|NEW_RASTER_OPS;
			out[o].state_change = ~0;
			out[o].check = ffbDDCheckPartialRasterSetup;
			out[o].run = ffbDDPartialRasterSetup;
			o++;
			break;

		case PIPE_OP_RAST_SETUP_0|PIPE_OP_RAST_SETUP_1:
			out[o] = in[i];
			out[o].check = ffbDDCheckRasterSetup;
			out[o].run = ffbDDDoRasterSetup;
			o++;
			break;

		default:
			out[o++] = in[i];
			break;
		};
	}

	return o;
}
