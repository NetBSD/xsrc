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

static void FFB_FillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h,
			 unsigned long color);

static void FFB_BlitRect(ScrnInfoPtr pScrn, int srcx, int srcy, int w, int h,
			 int dstx, int dsty);

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

	mode->flags = DGA_PIXMAP_AVAILABLE | DGA_FILL_RECT | DGA_BLIT_RECT | 
	    DGA_CONCURRENT_ACCESS;

	mode->imageWidth = mode->pixmapWidth = 2048;
	
	mode->viewportWidth = pScrn->modes->HDisplay;
		//pScrn->virtualX;
	mode->imageHeight = mode->pixmapHeight = 2048;
	
	mode->viewportHeight = pScrn->modes->VDisplay;
		//pScrn->virtualY;

	mode->bytesPerScanline = (2048 * 4);

	mode->byteOrder = pScrn->imageByteOrder;
	mode->depth = 32;
	mode->bitsPerPixel = 32;
	mode->red_mask = pScrn->mask.red;
	mode->green_mask = pScrn->mask.green;
	mode->blue_mask = pScrn->mask.blue;
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

extern void FFB_SetupForSolidFill(ScrnInfoPtr, int, int, unsigned int);
extern void FFB_SubsequentSolidFillRect(ScrnInfoPtr, int, int, int, int);

static void FFB_FillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h, unsigned long color)
{
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);

	FFB_SetupForSolidFill(pScrn, color, GXcopy, ~0);
	FFB_SubsequentSolidFillRect(pScrn, x, y, w, h);
	SET_SYNC_FLAG(pFfb->pXAAInfo);
}

extern void FFB_SetupForScreenToScreenCopy(ScrnInfoPtr, int, int, int,
						unsigned int, int);
extern void FFB_SubsequentScreenToScreenCopy(ScrnInfoPtr, int, int, int, int, 
						int, int);
						
static void FFB_BlitRect(ScrnInfoPtr pScrn, int srcx, int srcy,
			 int w, int h, int dstx, int dsty)
{
	FFBPtr pFfb = GET_FFB_FROM_SCRN(pScrn);
	int xdir = ((srcx < dstx) && (srcy == dsty)) ? -1 : 1;
	int ydir = (srcy < dsty) ? -1 : 1;

	FFB_SetupForScreenToScreenCopy(pScrn, xdir, ydir, GXcopy, ~0, -1);
	FFB_SubsequentScreenToScreenCopy(pScrn, srcx, srcy, dstx,dsty, w, h);
	SET_SYNC_FLAG(pFfb->pXAAInfo);
}
