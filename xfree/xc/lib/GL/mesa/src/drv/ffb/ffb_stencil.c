/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_stencil.c,v 1.1 2000/06/20 05:08:39 dawes Exp $
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
#include "ffb_dd.h"
#include "ffb_span.h"
#include "ffb_context.h"
#include "ffb_stencil.h"
#include "ffb_lock.h"

static void 
FFBWriteStencilSpan(GLcontext *ctx, GLuint n, GLint x, GLint y,
		    const GLstencil stencil[], const GLubyte mask[])
{
	if (ctx->Depth.Mask) {
		ffbContextPtr fmesa = FFB_CONTEXT(ctx);
		__DRIdrawablePrivate *dPriv = fmesa->driDrawable;
		GLuint *zptr;
		GLuint i;

		if (!fmesa->hw_locked)
			LOCK_HARDWARE(fmesa);
		FFBFifo(fmesa, 2);
		fmesa->regs->fbc = (FFB_FBC_WB_C | FFB_FBC_ZE_OFF |
				    FFB_FBC_YE_ON | FFB_FBC_RGBE_OFF);
		fmesa->regs->ppc = FFB_PPC_YS_VAR;
		FFBWait(fmesa, fmesa->regs);

		y = (dPriv->h - y);
		zptr = (GLuint *)
			((char *)fmesa->sfb32 +
			 ((dPriv->x + x) << 2) +
			 ((dPriv->y + y) << 13));
		
		for (i = 0; i < n; i++) {
			if (mask[i])
				*zptr = (stencil[i] & 0xf) << 28;
			zptr++;
		}

		FFBFifo(fmesa, 2);
		fmesa->regs->fbc = fmesa->fbc;
		fmesa->regs->ppc = fmesa->ppc;
		fmesa->ffbScreen->rp_active = 1;
		if (!fmesa->hw_locked)
			UNLOCK_HARDWARE(fmesa);
	}
}

static void 
FFBWriteStencilPixels(GLcontext *ctx, GLuint n, const GLint x[], const GLint y[],
		      const GLstencil stencil[], const GLubyte mask[])
{
	if (ctx->Depth.Mask) {
		ffbContextPtr fmesa = FFB_CONTEXT(ctx);
		__DRIdrawablePrivate *dPriv = fmesa->driDrawable;
		char *zbase;
		GLuint i;

		if (!fmesa->hw_locked)
			LOCK_HARDWARE(fmesa);
		FFBFifo(fmesa, 2);
		fmesa->regs->fbc = (FFB_FBC_WB_C | FFB_FBC_ZE_OFF |
				    FFB_FBC_YE_ON | FFB_FBC_RGBE_OFF);
		fmesa->regs->ppc = FFB_PPC_YS_VAR;
		fmesa->ffbScreen->rp_active = 1;
		FFBWait(fmesa, fmesa->regs);

		zbase = ((char *)fmesa->sfb32 +
			 (dPriv->x << 2) + (dPriv->y << 13));
		
		for (i = 0; i < n; i++) {
			GLint y1 = (dPriv->h - y[i]);
			GLint x1 = x[i];
			GLuint *zptr;

			zptr = (GLuint *)
				(zbase + (x1 << 2) + (y1 << 13));
			if (mask[i])
				*zptr = (stencil[i] & 0xf) << 28;
		}

		FFBFifo(fmesa, 2);
		fmesa->regs->fbc = fmesa->fbc;
		fmesa->regs->ppc = fmesa->ppc;
		fmesa->ffbScreen->rp_active = 1;
		if (!fmesa->hw_locked)
			UNLOCK_HARDWARE(fmesa);
	}
}

static void 
FFBReadStencilSpan(GLcontext *ctx, GLuint n, GLint x, GLint y, GLstencil stencil[])
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	__DRIdrawablePrivate *dPriv = fmesa->driDrawable;
	GLuint *zptr;
	GLuint i;

	if (!fmesa->hw_locked)
		LOCK_HARDWARE(fmesa);
	FFBFifo(fmesa, 1);
	fmesa->regs->fbc = FFB_FBC_RB_C;
	fmesa->ffbScreen->rp_active = 1;
	FFBWait(fmesa, fmesa->regs);

	y = (dPriv->h - y);
	zptr = (GLuint *)
		((char *)fmesa->sfb32 +
		 ((dPriv->x + x) << 2) +
		 ((dPriv->y + y) << 13));
		
	for (i = 0; i < n; i++) {
		stencil[i] = (*zptr >> 28) & 0xf;
		zptr++;
	}

	FFBFifo(fmesa, 1);
	fmesa->regs->fbc = fmesa->fbc;
	fmesa->ffbScreen->rp_active = 1;
	if (!fmesa->hw_locked)
		UNLOCK_HARDWARE(fmesa);
}

static void 
FFBReadStencilPixels(GLcontext *ctx, GLuint n, const GLint x[], const GLint y[],
		     GLstencil stencil[])
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	__DRIdrawablePrivate *dPriv = fmesa->driDrawable;
	char *zbase;
	GLuint i;

	if (!fmesa->hw_locked)
		LOCK_HARDWARE(fmesa);
	FFBFifo(fmesa, 1);
	fmesa->regs->fbc = FFB_FBC_RB_C;
	fmesa->ffbScreen->rp_active = 1;
	FFBWait(fmesa, fmesa->regs);

	zbase = ((char *)fmesa->sfb32 +
		 (dPriv->x << 2) + (dPriv->y << 13));
		
	for (i = 0; i < n; i++) {
		GLint y1 = (dPriv->h - y[i]);
		GLint x1 = x[i];
		GLuint *zptr;

		zptr = (GLuint *)
			(zbase + (x1 << 2) + (y1 << 13));
		stencil[i] = (*zptr >> 28) & 0xf;
	}

	FFBFifo(fmesa, 1);
	fmesa->regs->fbc = fmesa->fbc;
	fmesa->ffbScreen->rp_active = 1;
	if (!fmesa->hw_locked)
		UNLOCK_HARDWARE(fmesa);
}

void ffbDDInitStencilFuncs(GLcontext *ctx)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);

	if (fmesa->ffb_sarea->flags & FFB_DRI_FFB2PLUS) {
		ctx->Driver.WriteStencilSpan	= FFBWriteStencilSpan;
		ctx->Driver.ReadStencilSpan	= FFBReadStencilSpan;
		ctx->Driver.WriteStencilPixels	= FFBWriteStencilPixels;
		ctx->Driver.ReadStencilPixels	= FFBReadStencilPixels;
	} else {
		ctx->Driver.WriteStencilSpan	= NULL;
		ctx->Driver.ReadStencilSpan	= NULL;
		ctx->Driver.WriteStencilPixels	= NULL;
		ctx->Driver.ReadStencilPixels	= NULL;
	}
}
