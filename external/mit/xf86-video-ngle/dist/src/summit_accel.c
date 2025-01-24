/*
 * hardware acceleration for Visualize FX 4
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

/* $NetBSD: summit_accel.c,v 1.5 2025/01/24 07:51:24 macallan Exp $ */

#include <sys/types.h>
#include <dev/ic/summitreg.h>


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

#define SUMMIT_READ_MODE(m) \
	if ((m) != fPtr->read_mode) { \
		SummitWait(fPtr); \
		NGLEWrite4(fPtr, VISFX_VRAM_READ_MODE, (m)); \
		fPtr->read_mode = (m); \
	}

#define SUMMIT_WRITE_MODE(m) \
	if ((m) != fPtr->write_mode) { \
		SummitWait(fPtr); \
		NGLEWrite4(fPtr, VISFX_VRAM_WRITE_MODE, (m)); \
		fPtr->write_mode = (m); \
	}

static void
SummitWait(NGLEPtr fPtr)
{
	int reg;

	ENTER;
	do {
		reg = NGLERead4(fPtr, VISFX_STATUS);
	} while ((reg & 0x01000000) != 0);
	if (reg != 0) {
		xf86Msg(X_ERROR, "%s status %08x\n", __func__, reg);
		xf86Msg(X_ERROR, "fault %08x\n", NGLERead4(fPtr, 0x641040));
	}
	LEAVE;
}
	
static void
SummitWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int reg;

	ENTER;
	do {
		reg = NGLERead4(fPtr, VISFX_STATUS);
	} while ((reg & 0x01000000) != 0);
	if (reg != 0) {
		xf86Msg(X_ERROR, "%s status %08x\n", __func__, reg);
		xf86Msg(X_ERROR, "fault %08x\n", NGLERead4(fPtr, 0x641040));
	}
	LEAVE;
}

static void
SummitWaitFifo(NGLEPtr fPtr, int count)
{
	int reg;
	do {
		reg = NGLERead4(fPtr, VISFX_FIFO);
	} while (reg < count);
#ifdef DEBUG
	if (reg != 0x800) xf86Msg(X_ERROR, "%s %x\n", __func__, reg);
#endif
}

static Bool
SummitPrepareCopy
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
	int dstoff = exaGetPixmapOffset(pDstPixmap);
	int srcoff = exaGetPixmapOffset(pSrcPixmap);
	uint32_t sm, dm;
	int y;

	ENTER;

	sm = dm = OTC01 | BIN8F | BUFFL;
	DBGMSG(X_ERROR, "%s %d %d\n", __func__, srcoff, dstoff);

	y = (srcoff >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		sm = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	fPtr->offset = y;
	SUMMIT_READ_MODE(sm);

	y = (dstoff >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		dm = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	fPtr->offsetd = y;
	SUMMIT_WRITE_MODE(dm);

	SummitWaitFifo(fPtr, 6);
	if (alu == GXcopy) {
		NGLEWrite4(fPtr, VISFX_FOE, 0);
	} else {
		NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
		NGLEWrite4(fPtr, VISFX_IBO, alu);
	}
	NGLEWrite4(fPtr, VISFX_PLANE_MASK, planemask);
	LEAVE;
	return TRUE;
}

static void
SummitCopy
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

	ENTER;
	SummitWaitFifo(fPtr, 8);
	NGLEWrite4(fPtr, VISFX_COPY_SRC, (xs << 16) | (ys + fPtr->offset));
	NGLEWrite4(fPtr, VISFX_COPY_WH, (wi << 16) | he);
	NGLEWrite4(fPtr, VISFX_COPY_DST, (xd << 16) | (yd + fPtr->offsetd));

	LEAVE;
}

static void
SummitDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
SummitPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int	ofs =  exaGetPixmapOffset(pPixmap);
	int	y;
	uint32_t wm = OTC32 | BIN8F | BUFFL | 0x8c0, rm = OTC01 | BIN8F | BUFFL;

	ENTER;
	y = (ofs >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		wm = OTC32 | BIN8F | BUFBL | 0x8c0;
		rm = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	SUMMIT_READ_MODE(rm);
	SUMMIT_WRITE_MODE(wm);
	fPtr->offset = y;
	SummitWaitFifo(fPtr, 8);		
	if (alu == GXcopy) {
		NGLEWrite4(fPtr, VISFX_FOE, 0);
	} else {
		NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
		NGLEWrite4(fPtr, VISFX_IBO, alu);
	}
	NGLEWrite4(fPtr, VISFX_FG_COLOUR, fg);
	NGLEWrite4(fPtr, VISFX_PLANE_MASK, planemask);
	LEAVE;
	return TRUE;
}

static void
SummitSolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int wi = x2 - x1, he = y2 - y1;

	ENTER;
	
	y1 += fPtr->offset;
	
	SummitWaitFifo(fPtr, 6);		
	NGLEWrite4(fPtr, VISFX_START, (x1 << 16) | y1);
	NGLEWrite4(fPtr, VISFX_SIZE, (wi << 16) | he);

	LEAVE;
}

static Bool
SummitUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int	ofs =  exaGetPixmapOffset(pDst);
	int i;
	uint32_t *line, mode = OTC01 | BIN8F | BUFFL;

	ENTER;

	y += (ofs >> 13);	/* pitch is 8192 bytes in 24 bit */
	if (y >= fPtr->fbi.fbi_height) {
		mode = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	SUMMIT_WRITE_MODE(mode);
	NGLEWrite4(fPtr, VISFX_PLANE_MASK, 0xffffffff);
	NGLEWrite4(fPtr, VISFX_FOE, 0);
	
	while (h--) {
		SummitWaitFifo(fPtr, w + 1);
		NGLEWrite4(fPtr, VISFX_VRAM_WRITE_DEST, (y << 16) | x);
		line = (uint32_t *)src;

		for (i = 0; i < w; i++)
			NGLEWrite4(fPtr, VISFX_VRAM_WRITE_DATA_INCRX, line[i]);
		src += src_pitch;
		y++;
	}
	return TRUE;
}

static Bool
SummitDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	uint8_t *src;
	int	ofs =  exaGetPixmapOffset(pSrc);
	uint32_t mode = OTC01 | BIN8F | BUFFL;

	ENTER;
	y += (ofs >> 13);
	if (y >= fPtr->fbi.fbi_height) {
		mode = OTC01 | BIN8F | BUFBL;
		y -= fPtr->fbi.fbi_height;
	}
	SUMMIT_READ_MODE(mode);
	SummitWait(fPtr);
	NGLEWrite4(fPtr, VISFX_RPH, VISFX_RPH_LTR);
	SummitWait(fPtr);

	src = fPtr->fbmem;
	src += (y << 13) + (x << 2);

	while (h--) {
		memcpy(dst, src, w << 2);
		src += 8192;
		dst += dst_pitch;
	}

	return TRUE;
}

Bool
SummitPrepareAccess(PixmapPtr pPixmap, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int	ofs =  exaGetPixmapOffset(pPixmap);
	int	y;

	ENTER;
	if (ofs == 0) {
		/* accessing the visible framebuffer */
		SUMMIT_READ_MODE(OTC01 | BIN8F | BUFFL);
		SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFFL);
	} else {
		SUMMIT_READ_MODE(OTC01 | BIN8F | BUFBL);
		SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFBL);
		y = ofs >> 13;
		y -= fPtr->fbi.fbi_height;
		pPixmap->devPrivate.ptr = fPtr->fbmem + (y << 13);
	}
	NGLEWrite4(fPtr, VISFX_FOE, 0);
	//NGLEWrite4(fPtr, VISFX_CONTROL, 0x200);
	SummitWait(fPtr);
	LEAVE;
	return TRUE;
}

void
SummitFinishAccess(PixmapPtr pPixmap, int index)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);

	ENTER;
	//NGLEWrite4(fPtr, VISFX_CONTROL, 0);
	LEAVE;
}

Bool
SummitInitAccel(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	ExaDriverPtr pExa;
	int bpp = pScrn->bitsPerPixel >> 3;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	fPtr->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = fPtr->fbmem;
	pExa->memorySize = fPtr->fbi.fbi_stride * (fPtr->fbi.fbi_height * 2);
	pExa->offScreenBase = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
	pExa->pixmapOffsetAlign = fPtr->fbi.fbi_stride;
	pExa->pixmapPitchAlign = fPtr->fbi.fbi_stride;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS | EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;	

	pExa->WaitMarker = SummitWaitMarker;
	pExa->Solid = SummitSolid;
	pExa->DoneSolid = SummitDoneCopy;
	pExa->Copy = SummitCopy;
	pExa->DoneCopy = SummitDoneCopy;
	pExa->PrepareCopy = SummitPrepareCopy;
	pExa->PrepareSolid = SummitPrepareSolid;
	pExa->UploadToScreen = SummitUploadToScreen;
	pExa->DownloadFromScreen = SummitDownloadFromScreen;
	pExa->PrepareAccess = SummitPrepareAccess;
	pExa->FinishAccess = SummitFinishAccess;

	fPtr->read_mode = -1;
	fPtr->write_mode = -1;
	SUMMIT_READ_MODE(OTC01 | BIN8F | BUFFL);
	SUMMIT_WRITE_MODE(OTC01 | BIN8F | BUFFL);
	NGLEWrite4(fPtr, VISFX_FOE, FOE_BLEND_ROP);
	NGLEWrite4(fPtr, VISFX_IBO, GXcopy);
	NGLEWrite4(fPtr, VISFX_CONTROL, 0);

	return exaDriverInit(pScreen, pExa);
}
