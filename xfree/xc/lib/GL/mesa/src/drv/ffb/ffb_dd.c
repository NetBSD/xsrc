/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_dd.c,v 1.2 2001/04/10 16:07:50 dawes Exp $
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

#include "types.h"
#include "vbrender.h"

#include <stdio.h>
#include <stdlib.h>

#include "mm.h"
#include "ffb_dd.h"
#include "ffb_span.h"
#include "ffb_depth.h"
#include "ffb_context.h"
#include "ffb_vb.h"
#include "ffb_tris.h"
#include "ffb_clear.h"
#include "ffb_pipeline.h"
#include "ffb_lock.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"

/* Mesa's Driver Functions */

static const GLubyte *ffbDDGetString(GLcontext *ctx, GLenum name)
{
	switch (name) {
	case GL_VENDOR:
		return (GLubyte *) "David S. Miller";
	case GL_RENDERER:
		return (GLubyte *) "Mesa DRI FFB 20010321";
	default:
		return NULL;
	};
}

static GLint ffbGetParameteri(const GLcontext *ctx, GLint param)
{
	switch (param) {
	case DD_HAVE_HARDWARE_FOG:
		/* XXX We have per-fragment fog, once fog code is done,
		 * XXX set this.
		 */
		return 0;
	default:
#if 0
		ffbError("ffbGetParameteri(): unknown parameter!\n");
#endif
		return 0;
	};
}

static void ffbBufferSize(GLcontext *ctx, GLuint *width, GLuint *height)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);

/*      LOCK_HARDWARE(fmesa); */
	*width = fmesa->driDrawable->w;
	*height = fmesa->driDrawable->h;
/*      UNLOCK_HARDWARE(fmesa); */
}

void ffbDDExtensionsInit(GLcontext *ctx)
{
	/* Nothing for now until we start to add
	 * real acceleration. -DaveM
	 */

	/* XXX Need to turn off GL_EXT_blend_func_separate for one.
	 * XXX Also BlendEquation should be turned off too, what
	 * XXX EXT is that assosciated with?
	 */
}

static void ffbDDFinish(GLcontext *ctx)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);

	LOCK_HARDWARE(fmesa);
	FFBWait(fmesa, fmesa->regs);
	UNLOCK_HARDWARE(fmesa);
}

void ffbDDInitDriverFuncs(GLcontext *ctx)
{
	ctx->Driver.GetBufferSize	 = ffbBufferSize;
	ctx->Driver.GetString		 = ffbDDGetString;
	ctx->Driver.GetParameteri	 = ffbGetParameteri;
	ctx->Driver.Clear		 = ffbDDClear;

	ctx->Driver.Finish		 = ffbDDFinish;

	/* VB hooks */
	ctx->Driver.RegisterVB		 = ffbDDRegisterVB;
	ctx->Driver.UnregisterVB	 = ffbDDUnregisterVB;

	ctx->Driver.BuildPrecalcPipeline = ffbDDBuildPrecalcPipeline;
}
