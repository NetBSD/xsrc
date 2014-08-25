/*
 * IGS CyberPro - hardware acceleration.
 *
 * Copyright (C) 2009 Michael Lorenz
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

/* $NetBSD: igs_accel.c,v 1.9 2014/08/25 15:27:00 macallan Exp $ */

#include <sys/types.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"

#include "igs.h"

/*#define DEBUG*/

#ifdef DEBUG
#define ENTER xf86Msg(X_ERROR, "%s\n", __func__)
#define LEAVE xf86Msg(X_ERROR, "%s done\n", __func__)
#else
#define ENTER
#define LEAVE
#endif

static inline void IgsWrite1(IgsPtr fPtr, int offset, uint8_t val)
{
	*(fPtr->reg + offset) = val;
}


static inline void IgsWrite2(IgsPtr fPtr, int offset, uint16_t val)
{
	*(volatile uint16_t *)(fPtr->reg + offset) = val;
}

static inline void IgsWrite4(IgsPtr fPtr, int offset, uint32_t val)
{
	*(volatile uint32_t *)(fPtr->reg + offset) = val;
}

static inline uint8_t IgsRead1(IgsPtr fPtr, int offset)
{
	return *(fPtr->reg + offset);
}

static inline uint16_t IgsRead2(IgsPtr fPtr, int offset)
{
	return *(volatile uint16_t *)(fPtr->reg + offset);
}

static void
IgsWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	int bail = 0x0fffffff;

	ENTER;
	IgsWrite1(fPtr, IGS_COP_MAP_FMT_REG, fPtr->mapfmt);
	while (((IgsRead1(fPtr,
	    IGS_COP_CTL_REG) & (IGS_COP_CTL_BUSY | IGS_COP_CTL_HFEMPTZ)) != 0)
	    && (bail > 0)) {
		bail--;
		IgsWrite1(fPtr, IGS_COP_MAP_FMT_REG, fPtr->mapfmt);
	}

	/* reset the coprocessor if we run into a timeout */
	if (bail == 0) {
		xf86Msg(X_ERROR, "%s: timeout\n", __func__);
		IgsWrite1(fPtr, IGS_COP_CTL_REG, 0);
	}
	LEAVE;
}

static void
IgsWaitReady(IgsPtr fPtr)
{
	int bail = 0x0fffffff;

	ENTER;
	IgsWrite1(fPtr, IGS_COP_MAP_FMT_REG, fPtr->mapfmt);
	while (((IgsRead1(fPtr,
	    IGS_COP_CTL_REG) & (IGS_COP_CTL_BUSY | IGS_COP_CTL_HFEMPTZ)) != 0)
	    && (bail > 0)) {
		bail--;
		IgsWrite1(fPtr, IGS_COP_MAP_FMT_REG, fPtr->mapfmt);
	}

	/* reset the coprocessor if we run into a timeout */
	if (bail == 0) {
		xf86Msg(X_ERROR, "%s: timeout\n", __func__);
		IgsWrite1(fPtr, IGS_COP_CTL_REG, 0);
	}
	LEAVE;
}

static Bool
IgsPrepareCopy
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
	IgsPtr fPtr = IGSPTR(pScrn);

	ENTER;
	fPtr->cmd = IGS_COP_OP_PXBLT | IGS_COP_OP_FG_FROM_SRC | 
	    IGS_COP_PPM_FIXED_FG;
	if (xdir < 0)
		fPtr->cmd |= IGS_COP_OCTANT_X_NEG;
	if (ydir < 0)
		fPtr->cmd |= IGS_COP_OCTANT_Y_NEG;

	IgsWaitReady(fPtr);
	IgsWrite1(fPtr, IGS_COP_CTL_REG, 0);
	fPtr->srcoff = exaGetPixmapOffset(pSrcPixmap) >> fPtr->shift;
	fPtr->srcpitch = exaGetPixmapPitch(pSrcPixmap) >> fPtr->shift;

	IgsWrite2(fPtr, IGS_COP_SRC_MAP_WIDTH_REG, fPtr->srcpitch - 1);
	IgsWrite2(fPtr, IGS_COP_DST_MAP_WIDTH_REG, 
	    (exaGetPixmapPitch(pDstPixmap) >> fPtr->shift) - 1);
	IgsWrite1(fPtr, IGS_COP_FG_MIX_REG, alu);
	IgsWrite4(fPtr, IGS_PLANE_MASK_REG, planemask);
	LEAVE;
	return TRUE;
}

static void
IgsCopy
(
    PixmapPtr pDstPixmap,
    int       srcX,
    int       srcY,
    int       dstX,
    int       dstY,
    int       w,
    int       h
)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	int dstpitch, dstoff;

	if (fPtr->cmd & IGS_COP_OCTANT_X_NEG) {
		srcX += (w - 1);
		dstX += (w - 1);
	}

	if (fPtr->cmd & IGS_COP_OCTANT_Y_NEG) {
		srcY += (h - 1);
		dstY += (h - 1);
	}
	IgsWaitReady(fPtr);
	IgsWrite4(fPtr, IGS_COP_SRC_START_REG, fPtr->srcoff + srcX + 
	    fPtr->srcpitch * srcY);
	dstpitch = exaGetPixmapPitch(pDstPixmap) >> fPtr->shift;
	dstoff = exaGetPixmapOffset(pDstPixmap) >> fPtr->shift;
	IgsWrite4(fPtr, IGS_COP_DST_START_REG, dstoff + dstX + 
	    dstpitch * dstY);
	IgsWrite2(fPtr, IGS_COP_WIDTH_REG, w - 1);
	IgsWrite2(fPtr, IGS_COP_HEIGHT_REG, h - 1);
	IgsWrite2(fPtr, IGS_COP_PIXEL_OP_REG, fPtr->cmd & 0xffff);
	IgsWrite2(fPtr, IGS_COP_PIXEL_OP_REG + 2, (fPtr->cmd >> 16) & 0xffff);
	exaMarkSync(pDstPixmap->drawable.pScreen);
	LEAVE;
}

static void
IgsDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
IgsPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);

	ENTER;
	fPtr->cmd = IGS_COP_OP_PXBLT | IGS_COP_PPM_FIXED_FG;

	IgsWaitReady(fPtr);

	IgsWrite1(fPtr, IGS_COP_CTL_REG, 0);

	IgsWrite2(fPtr, IGS_COP_DST_MAP_WIDTH_REG, 
	    (exaGetPixmapPitch(pPixmap) >> fPtr->shift) - 1);
	IgsWrite1(fPtr, IGS_COP_FG_MIX_REG, alu);
	IgsWrite4(fPtr, IGS_PLANE_MASK_REG, planemask);
	IgsWrite4(fPtr, IGS_COP_FG_REG, fg);
	LEAVE;
	return TRUE;
}

static void
IgsSolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	int w = x2 - x1, h = y2 - y1, dstoff, dstpitch;

	ENTER;
	IgsWaitReady(fPtr);
	dstpitch = exaGetPixmapPitch(pPixmap) >> fPtr->shift;
	dstoff = exaGetPixmapOffset(pPixmap) >> fPtr->shift;
	IgsWrite4(fPtr, IGS_COP_DST_START_REG, dstoff + x1 + 
	    dstpitch * y1);
	IgsWrite2(fPtr, IGS_COP_WIDTH_REG, w - 1);
	IgsWrite2(fPtr, IGS_COP_HEIGHT_REG, h - 1);
	IgsWrite2(fPtr, IGS_COP_PIXEL_OP_REG, fPtr->cmd & 0xffff);
	IgsWrite2(fPtr, IGS_COP_PIXEL_OP_REG + 2, (fPtr->cmd >> 16) & 0xffff);
	exaMarkSync(pPixmap->drawable.pScreen);
}

/*
 * Memcpy-based UTS.
 */
static Bool
IgsUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	unsigned char *dst = fPtr->fbmem + exaGetPixmapOffset(pDst);
	int dst_pitch = exaGetPixmapPitch(pDst);

	int bpp    = pDst->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	ENTER;
	dst += (x * cpp) + (y * dst_pitch);

	IgsWaitReady(fPtr);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}
	LEAVE;
	return TRUE;
}

/*
 * Memcpy-based DFS.
 */
static Bool
IgsDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	unsigned char *src = fPtr->fbmem + exaGetPixmapOffset(pSrc);
	int src_pitch = exaGetPixmapPitch(pSrc);

	int bpp    = pSrc->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	ENTER;
	src += (x * cpp) + (y * src_pitch);

	IgsWaitReady(fPtr);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}
	LEAVE;
	return TRUE;
}

#ifdef __arm__
static Bool
IgsPrepareAccess(PixmapPtr pPix, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPix->drawable.pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);

	IgsWaitReady(fPtr);
	return TRUE;	
}

static void
IgsFinishAccess(PixmapPtr pPix, int index)
{
}
#endif

Bool
IgsInitAccel(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	ExaDriverPtr pExa;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	fPtr->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = fPtr->fbmem;
	pExa->memorySize = fPtr->fbmem_len;
	pExa->offScreenBase = fPtr->linebytes * fPtr->info.height;
	pExa->pixmapOffsetAlign = 4;
	pExa->pixmapPitchAlign = 4;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS |
		      EXA_SUPPORTS_OFFSCREEN_OVERLAPS |
		      EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;	

	pExa->WaitMarker = IgsWaitMarker;
	pExa->PrepareSolid = IgsPrepareSolid;
	pExa->Solid = IgsSolid;
	pExa->DoneSolid = IgsDoneCopy;
	pExa->PrepareCopy = IgsPrepareCopy;
	pExa->Copy = IgsCopy;
	pExa->DoneCopy = IgsDoneCopy;

	switch(fPtr->info.depth) {
		case 8:
			fPtr->shift = 0;
			fPtr->mapfmt = IGS_COP_MAP_8BPP;
			break;
		case 16:
			fPtr->shift = 1;
			fPtr->mapfmt = IGS_COP_MAP_16BPP;
			break;
		case 24:
		case 32:
			fPtr->shift = 2;
			fPtr->mapfmt = IGS_COP_MAP_32BPP;
			break;
		default:	
			ErrorF("Unsupported depth: %d\n", fPtr->info.depth);
	}
	IgsWrite1(fPtr, IGS_COP_MAP_FMT_REG, fPtr->mapfmt);

	/* EXA hits more optimized paths when it does not have to fallback 
	 * because of missing UTS/DFS, hook memcpy-based UTS/DFS.
	 */
	pExa->UploadToScreen = IgsUploadToScreen;
	pExa->DownloadFromScreen = IgsDownloadFromScreen;
#ifdef __arm__
	pExa->PrepareAccess = IgsPrepareAccess;
	pExa->FinishAccess = IgsFinishAccess;
#endif
	return exaDriverInit(pScreen, pExa);
}
