/*
 * NGLE - hardware acceleration.
 *
 * Copyright (C) 2024 Michael Lorenz
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
 * MICHAEL LORENZ BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* $NetBSD: ngle_accel.c,v 1.2 2024/10/22 07:42:15 macallan Exp $ */

#include <sys/types.h>
#include <dev/ic/stireg.h>


#include "ngle.h"

//#define DEBUG

#ifdef DEBUG
#define ENTER xf86Msg(X_ERROR, "%s\n", __func__)
#define LEAVE xf86Msg(X_ERROR, "%s done\n", __func__)
#define DBGMSG xf86Msg
#else
#define ENTER
#define DBGMSG if (0) xf86Msg
#define LEAVE
#endif

static inline void
NGLEWrite4(NGLEPtr fPtr, int offset, uint32_t val)
{
	volatile uint32_t *ptr = (uint32_t *)((uint8_t *)fPtr->regs + offset);
	*ptr = val;
}

static inline void
NGLEWrite1(NGLEPtr fPtr, int offset, uint8_t val)
{
	volatile uint8_t *ptr = (uint8_t *)fPtr->regs + offset;
	*ptr = val;
}

static inline uint32_t
NGLERead4(NGLEPtr fPtr, int offset)
{
	volatile uint32_t *ptr = (uint32_t *)((uint8_t *)fPtr->regs + offset);
	return *ptr;
}

static inline uint8_t
NGLERead1(NGLEPtr fPtr, int offset)
{
	volatile uint8_t *ptr = (uint8_t *)fPtr->regs + offset;
	return *ptr;
}

static void
NGLEWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	uint8_t stat;

	ENTER;
	do {
		stat = NGLERead1(fPtr, NGLE_REG_15b0);
		if (stat == 0)
			stat = NGLERead1(fPtr, NGLE_REG_15b0);
	} while (stat != 0);
	LEAVE;
}

static void
NGLEWaitFifo(NGLEPtr fPtr, int slots)
{
	uint32_t reg;

	ENTER;
	do {
		reg = NGLERead4(fPtr, NGLE_REG_34);
	} while (reg < slots);
	LEAVE;
}

static Bool
NGLEPrepareCopy
(
    PixmapPtr pSrcPixmap,
    PixmapPtr pDstPixmap,
    int       xdir,
    int       ydir,
    int       alu,
    Pixel     planemask
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int srcpitch = exaGetPixmapPitch(pSrcPixmap);
	int srcoff = exaGetPixmapOffset(pSrcPixmap);

	ENTER;

	DBGMSG(X_ERROR, "%s %d %d\n", __func__, srcoff, srcpitch);
	fPtr->offset = srcoff / srcpitch;
	NGLEWaitMarker(pDstPixmap->drawable.pScreen, 0);
	/* XXX HCRX needs ifferent values here */
	NGLEWrite4(fPtr, NGLE_REG_10,
	    BA(IndexedDcd, Otc04, Ots08, AddrLong, 0, BINapp0I, 0));
	NGLEWrite4(fPtr, NGLE_REG_14, ((alu << 8) & 0xf00) | 0x23000000);
	NGLEWrite4(fPtr, NGLE_REG_13, planemask);

	fPtr->hwmode = HW_BLIT;

	LEAVE;
	return TRUE;
}

static void
NGLECopy
(
    PixmapPtr pDstPixmap,
    int       xs,
    int       ys,
    int       xd,
    int       yd,
    int       wi,
    int       he
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int dstpitch = exaGetPixmapPitch(pDstPixmap);
	int dstoff = exaGetPixmapOffset(pDstPixmap);

	ENTER;
	NGLEWaitFifo(fPtr, 3);
	NGLEWrite4(fPtr, NGLE_REG_24, (xs << 16) | (ys + fPtr->offset));
	NGLEWrite4(fPtr, NGLE_REG_7, (wi << 16) | he);
	NGLEWrite4(fPtr, NGLE_REG_25, (xd << 16) | (yd + (dstoff / dstpitch)));

	exaMarkSync(pDstPixmap->drawable.pScreen);
	LEAVE;
}

static void
NGLEDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
NGLEPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);

	ENTER;
	NGLEWaitFifo(fPtr, 4);
	/* plane mask */
	NGLEWrite4(fPtr, NGLE_REG_13, planemask);
	/* bitmap op */
	NGLEWrite4(fPtr, NGLE_REG_14, 
	    IBOvals(alu, 0, BitmapExtent08, 0, DataDynamic, MaskOtc, 1, 0));

	/* XXX HCRX needs different values here */
	/* dst bitmap access */
	NGLEWrite4(fPtr, NGLE_REG_11,
	    BA(IndexedDcd, Otc32, OtsIndirect, AddrLong, 0, BINapp0I, 0));
    	NGLEWrite4(fPtr, NGLE_REG_35, fg);
	fPtr->hwmode = HW_FILL;

	LEAVE;
	return TRUE;
}

static void
NGLESolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int w = x2 - x1, h = y2 - y1;
	int pitch = exaGetPixmapPitch(pPixmap);
	int offset = exaGetPixmapOffset(pPixmap);
	uint32_t mask;
	int wi, rest;

	ENTER;
	
	y1 += offset >> 11;

	/*
	 * XXX
	 * Turns out this thing always fills rectangles to the next 32 pixel
	 * boundary on te right. To get around this we split the rectangle
	 * into a multiples-of-32 part and the rest, so we can mask off the
	 * excess pixels.
	 */
	rest = w & 0x1f;
	wi = w & 0xffffe0;
	if (wi > 0) {
		NGLEWaitFifo(fPtr, 3);
		/* transfer data */
		NGLEWrite4(fPtr, NGLE_REG_8, 0xffffffff);
		/* dst XY */
		NGLEWrite4(fPtr, NGLE_REG_6, (x1 << 16) | y1);
		/* len XY start */
		NGLEWrite4(fPtr, NGLE_REG_9, (wi << 16) | h);
	}
	if (rest > 0) {
		mask = 0xffffffff << (32 - w);
		/* transfer data */
		NGLEWaitFifo(fPtr, 3);
		NGLEWrite4(fPtr, NGLE_REG_8, mask);
		/* dst XY */
		NGLEWrite4(fPtr, NGLE_REG_6, ((x1 + wi) << 16) | y1);
		/* len XY start */
		NGLEWrite4(fPtr, NGLE_REG_9, (rest << 16) | h);
	}
	exaMarkSync(pPixmap->drawable.pScreen);
	LEAVE;
}

Bool
NGLEPrepareAccess(PixmapPtr pPixmap, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);

	if (fPtr->hwmode == HW_FB) return TRUE;

	NGLEWaitMarker(pPixmap->drawable.pScreen, 0);
	NGLEWrite4(fPtr, NGLE_REG_10, fPtr->fbacc);
	NGLEWrite4(fPtr, NGLE_REG_14, 0x83000300);
	NGLEWrite4(fPtr, NGLE_REG_13, 0xff);
	NGLEWaitMarker(pPixmap->drawable.pScreen, 0);
	NGLEWrite1(fPtr, NGLE_REG_16b1, 1);
	fPtr->hwmode = HW_FB;
	return TRUE;
}

Bool
NGLEInitAccel(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	ExaDriverPtr pExa;
	int lines, bpp = pScrn->bitsPerPixel >> 3;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	fPtr->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = fPtr->fbmem;
	lines = fPtr->fbmem_len / fPtr->fbi.fbi_stride;
	DBGMSG(X_ERROR, "lines %d\n", lines);	
	pExa->memorySize = fPtr->fbmem_len;
	pExa->offScreenBase = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
	pExa->pixmapOffsetAlign = fPtr->fbi.fbi_stride;
	pExa->pixmapPitchAlign = fPtr->fbi.fbi_stride;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS | EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;	

	fPtr->hwmode = -1;

	pExa->WaitMarker = NGLEWaitMarker;
	pExa->PrepareSolid = NGLEPrepareSolid;
	pExa->Solid = NGLESolid;
	pExa->DoneSolid = NGLEDoneCopy;
	pExa->PrepareCopy = NGLEPrepareCopy;
	pExa->Copy = NGLECopy;
	pExa->DoneCopy = NGLEDoneCopy;
	pExa->PrepareAccess = NGLEPrepareAccess;
	NGLEWaitMarker(pScreen, 0);

	return exaDriverInit(pScreen, pExa);
}
