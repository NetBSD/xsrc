/*
 * Acceleration for the Creator and Creator3D framebuffer - DGA support.
 *
 * Copyright (C) 2000 David S. Miller (davem@redhat.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * DAVID MILLER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sunffb/ffb_dga.c,v 1.4 2004/12/07 15:59:20 tsi Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "dgaproc.h"

#include "ffb.h"
#include "ffb_regs.h"
#include "ffb_rcache.h"
#include "ffb_fifo.h"
#include "ffb_stip.h"
#include "ffb_loops.h"
#include "ffb_gc.h"

static Bool FFB_OpenFramebuffer(ScrnInfoPtr pScrn, char **name,
				unsigned int *mem,
				unsigned int *size, unsigned int *offset,
				unsigned int *extra);
static Bool FFB_SetMode(ScrnInfoPtr pScrn, DGAModePtr pMode);
static void FFB_SetViewport(ScrnInfoPtr pScrn, int x, int y, int flags);
static int FFB_GetViewport(ScrnInfoPtr pScrn);
static void FFB_Flush(ScrnInfoPtr pScrn);

/*
 * Have to disable all this stuff for now until I figure out where
 * we should get the WID values from... ho hum... -DaveM
 */
#if 0
static void FFB_FillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h,
			 unsigned long color);

#ifdef USE_VIS
static void FFB_BlitRect(ScrnInfoPtr pScrn, int srcx, int srcy, int w, int h,
			 int dstx, int dsty);
#else
#define FFB_BlitRect NULL
#endif
#else
#define FFB_FillRect NULL
#define FFB_BlitRect NULL
#endif

static DGAFunctionRec FFB_DGAFuncs = {
	FFB_OpenFramebuffer,
	NULL,
	FFB_SetMode,
	FFB_SetViewport,
	FFB_GetViewport,
	FFB_Flush,
	FFB_FillRect,
	FFB_BlitRect,
	NULL
};

void FFB_InitDGA(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	DGAModePtr mode;
	Bool result;

	mode = xnfcalloc(sizeof(DGAModeRec), 1);
	if (!mode) {
		xf86Msg(X_WARNING, "%s: DGA init failed, cannot alloc DGAMode.\n",
			pFfb->psdp->device);
		return;
	}

	mode->mode = pScrn->modes;

	mode->flags = DGA_PIXMAP_AVAILABLE;
#if 0
	mode->flags |= DGA_FILL_RECT;
#ifdef USE_VIS
	mode->flags |= DGA_BLIT_RECT;
#endif
#else
	mode->flags |= DGA_CONCURRENT_ACCESS;
#endif

	mode->imageWidth = mode->pixmapWidth = mode->viewportWidth =
		pScrn->virtualX;
	mode->imageHeight = mode->pixmapHeight = mode->viewportHeight =
		pScrn->virtualY;

	mode->bytesPerScanline = (2048 * 4);

	mode->byteOrder = pScrn->imageByteOrder;
	mode->depth = 24;
	mode->bitsPerPixel = 32;
	mode->red_mask = 0xff;
	mode->green_mask = 0xff00;
	mode->blue_mask = 0xff0000;
	mode->visualClass = TrueColor;
	mode->address = (pointer)pFfb->fb;

	result = DGAInit(pScreen, &FFB_DGAFuncs, mode, 1);
	if (result == FALSE) {
		xf86Msg(X_WARNING,
			"%s: DGA init failed, DGAInit returns FALSE.\n",
			pFfb->psdp->device);
	} else {
		xf86Msg(X_INFO, "%s: DGA support initialized.\n",
			pFfb->psdp->device);
	}
}

static Bool FFB_OpenFramebuffer(ScrnInfoPtr pScrn, char **name,
				unsigned int *mem,
				unsigned int *size, unsigned int *offset,
				unsigned int *extra)
{
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);

	*name = pFfb->psdp->device;

	/* We give the user the dumb frame buffer. */
	*mem = FFB_DFB24_VOFF;
	*size = 0x1000000;
	*offset = 0;
	*extra = 0;

	return TRUE;
}

static Bool FFB_SetMode(ScrnInfoPtr pScrn, DGAModePtr pMode)
{
	/*
	 * Nothing to do, we currently only support one mode
	 * and we are always in it.
	 */
	return TRUE;
}

static void FFB_SetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
	/* We don't support viewports, so... */
}

static int FFB_GetViewport(ScrnInfoPtr pScrn)
{
	/* No viewports, none pending... */
	return 0;
}

static void FFB_Flush(ScrnInfoPtr pScrn)
{
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;

	FFBWait(pFfb, ffb);
}

#if 0

static void FFB_FillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h, unsigned long color)
{
	DrawableRec draw;
	BoxRec box;

	draw.pScreen = pScrn->pScreen;
	box.x1 = x;
	box.y1 = y;
	box.x2 = x + w;
	box.y2 = y + h;

	CreatorFillBoxSolid(&draw, 1, &box, color);
}

#ifdef USE_VIS
static void FFB_BlitRect(ScrnInfoPtr pScrn, int srcx, int srcy,
			 int w, int h, int dstx, int dsty)
{
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	ffb_fbcPtr ffb = pFfb->regs;

	if (!pFfb->disable_vscroll &&
	    dstx == srcx &&
	    dsty != dsty) {
		FFB_WRITE_ATTRIBUTES_VSCROLL(pFfb, 0x00ffffff);
		FFBFifo(pFfb, 7);
		ffb->drawop = FFB_DRAWOP_VSCROLL;
		FFB_WRITE64(&ffb->by, srcy, srcx);
		FFB_WRITE64_2(&ffb->dy, dsty, dstx);
		FFB_WRITE64_3(&ffb->bh, h, w);
		pFfb->rp_active = 1;
	} else {
		unsigned char *base = (unsigned char *)pFfb->fb;
		int use_prefetch = pFfb->use_blkread_prefetch;

		FFB_WRITE_ATTRIBUTES_SFB_VAR(pFfb, 0x00ffffff, GXcopy);
		FFBWait(pFfb, ffb);
		if (use_prefetch) {
			FFBFifo(pFfb, 1);
			ffb->mer = FFB_MER_EIRA;
			pFfb->rp_active = 1;
			FFBWait(pFfb, ffb);
		}
		if (srcx < dstx) {
			VISmoveImageRL((base +
					((srcy + h - 1) * (2048 * 4)) +
					(srcx * (32 / 8))),
				       (base +
					((dsty + h - 1) * (2048 * 4)) +
					(dstx * (32 / 8))),
				       (w * (32 / 8)),
				       h,
				       -(2048 * 4), - (2048 * 4));
		} else {
			VISmoveImageLR((base +
					((srcy + h - 1) * (2048 * 4)) +
					(srcx * (32 / 8))),
				       (base +
					((dsty + h - 1) * (2048 * 4)) +
					(dstx * (32 / 8))),
				       (w * (32 / 8)),
				       h,
				       -(2048 * 4), - (2048 * 4));
		}
		if (use_prefetch) {
			FFBFifo(pFfb, 1);
			ffb->mer = FFB_MER_DRA;
			pFfb->rp_active = 1;
			FFBWait(pFfb, pFfb->regs);
		}
	}
}
#endif

#endif
