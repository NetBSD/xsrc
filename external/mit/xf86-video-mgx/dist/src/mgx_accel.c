/*
 * Southland Media MGX - hardware acceleration.
 *
 * Copyright (C) 2021 Michael Lorenz
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

/* $NetBSD: mgx_accel.c,v 1.1 2021/11/12 18:58:14 macallan Exp $ */

#include <sys/types.h>

#include "mgx.h"
#include <dev/sbus/mgxreg.h>

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

/* Translation from X ROP's to APM ROP's. */
static unsigned char apmROP[] = {
  0,
  0x88,
  0x44,
  0xCC,
  0x22,
  0xAA,
  0x66,
  0xEE,
  0x11,
  0x99,
  0x55,
  0xDD,
  0x33,
  0xBB,
  0x77,
  0xFF
};

static void
MgxWaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	int bail = 0x0fffffff;
	uint8_t stat;

	ENTER;
	do {
		stat = MgxRead1(pMgx, ATR_BLT_STATUS);
		if ((stat & (BLT_HOST_BUSY | BLT_ENGINE_BUSY)) == 0)
			break;
		bail--;
	} while (bail < 0);
	if (bail == 0) DBGMSG(X_ERROR, "%s timed out\n", __func__);
	LEAVE;
}

static void
MgxWait(MgxPtr pMgx)
{
	int bail = 10000;
	uint8_t stat;

	ENTER;
	do {
		stat = MgxRead1(pMgx, ATR_BLT_STATUS);
		if ((stat & (BLT_HOST_BUSY | BLT_ENGINE_BUSY)) == 0)
			break;
		bail--;
	} while (bail < 0);
	if (bail == 0) DBGMSG(X_ERROR, "%s timed out\n", __func__);
	LEAVE;
}

static void
MgxWaitFifo(MgxPtr pMgx, int depth)
{
	unsigned int i;
	uint8_t stat;

	ENTER;
	
	for (i = 100000; i != 0; i--) {
		stat = MgxRead1(pMgx, ATR_FIFO_STATUS);
		stat = (stat & FIFO_MASK) >> FIFO_SHIFT;
		DBGMSG(X_ERROR, "%s %x\n", __func__, stat);
		if (stat >= depth)
			break;
		MgxWrite1(pMgx, ATR_FIFO_STATUS, 0);
	}
	if (i == 0) xf86Msg(X_ERROR, "%s timed out\n", __func__);
	LEAVE;
}

static Bool
MgxPrepareCopy
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
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	int srcpitch = exaGetPixmapPitch(pSrcPixmap);
	int srcoff = exaGetPixmapOffset(pSrcPixmap);

	ENTER;

	DBGMSG(X_ERROR, "%s %d %d\n", __func__, srcoff, srcpitch);
	pMgx->offset = srcoff / srcpitch;

	MgxWaitFifo(pMgx, 1);
	MgxWrite1(pMgx, ATR_ROP, apmROP[alu]);
	LEAVE;
	return TRUE;
}

static void
MgxCopy
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
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	int dstpitch = exaGetPixmapPitch(pDstPixmap);
	int dstoff = exaGetPixmapOffset(pDstPixmap);
	uint32_t dec = pMgx->dec;

	ENTER;

	if (dstoff > 0 || 1) {
		DBGMSG(X_ERROR, "%s %d %d\n", __func__, dstoff, dstpitch);
		yd += dstoff / dstpitch;
	}
	ys += pMgx->offset;

        dec |= (DEC_COMMAND_BLT << DEC_COMMAND_SHIFT) |
	       (DEC_START_DIMX << DEC_START_SHIFT);

	if ((xs < xd) && (ys == yd) && ((xd - xs) < wi)) {
		xs += wi - 1;
		xd += wi - 1;
		dec |= DEC_DIR_X_REVERSE;
	}
	if (ys < yd) {
		ys += he - 1;
		yd += he - 1;
		dec |= DEC_DIR_Y_REVERSE;
	}

	DBGMSG(X_ERROR, "%s %d %d %d %d -> %d %d\n", __func__, xs, ys, wi, he, xd, yd);
	MgxWaitFifo(pMgx, 4);
	MgxWrite4(pMgx, ATR_DEC, dec);
	MgxWrite4(pMgx, ATR_SRC_XY, (ys << 16) | xs);
	MgxWrite4(pMgx, ATR_DST_XY, (yd << 16) | xd);
	MgxWrite4(pMgx, ATR_WH, (he << 16) | wi);

	exaMarkSync(pDstPixmap->drawable.pScreen);
	LEAVE;
}

static void
MgxDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
MgxPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	uint32_t dec;

	ENTER;
	dec = pMgx->dec;
	dec |= (DEC_COMMAND_RECT << DEC_COMMAND_SHIFT) |
	       (DEC_START_DIMX << DEC_START_SHIFT);
	MgxWaitFifo(pMgx, 3);
	MgxWrite1(pMgx, ATR_ROP, apmROP[alu]);
	MgxWrite4(pMgx, ATR_FG, /*bswap32*/(fg));
	MgxWrite4(pMgx, ATR_DEC, dec);
	LEAVE;
	return TRUE;
}

static void
MgxSolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	int w = x2 - x1, h = y2 - y1, dstoff, dstpitch;
	int pitch = exaGetPixmapPitch(pPixmap);
	int offset = exaGetPixmapOffset(pPixmap);

	ENTER;
	if (offset > 0) {
		DBGMSG(X_ERROR, "%s %d %d\n", __func__, offset, pitch);
		y1 += offset / pitch;
	}
	DBGMSG(X_ERROR, "%s %d %d %d %d\n", __func__, x1, y1, w, h);

	MgxWaitFifo(pMgx, 2);
	MgxWrite4(pMgx, ATR_DST_XY, (y1 << 16) | x1);
	MgxWrite4(pMgx, ATR_WH, (h << 16) | w);
	exaMarkSync(pPixmap->drawable.pScreen);
	LEAVE;
}

/*
 * Memcpy-based UTS.
 */
static Bool
MgxUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *srcc, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	uint32_t *dst = (uint32_t *)(pMgx->fb + exaGetPixmapOffset(pDst));
	uint32_t *src = (uint32_t *)srcc;	
	int i, dst_pitch = exaGetPixmapPitch(pDst) >> 2;

	ENTER;
	dst += x + (y * dst_pitch);

	MgxWait(pMgx);

	while (h--) {
		for (i = 0; i < w; i++) dst[i] = /*bswap32*/(src[i]);
		src += src_pitch >> 2;
		dst += dst_pitch;
	LEAVE;
	}
	return TRUE;
}

/*
 * Memcpy-based DFS.
 */
static Bool
MgxDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dstt, int dst_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	uint32_t *src = (uint32_t *)(pMgx->fb + exaGetPixmapOffset(pSrc));
	uint32_t *dst = (uint32_t *)dstt;
	int i, src_pitch = exaGetPixmapPitch(pSrc) >> 2;

	ENTER;
	src += x + (y * src_pitch);

	MgxWait(pMgx);

	while (h--) {
		for (i = 0; i < w; i++) dst[i] = /*bswap32*/(src[i]);
		src += src_pitch;
		dst += dst_pitch >> 2;
	}
	LEAVE;
	return TRUE;
}

Bool
MgxInitAccel(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
	ExaDriverPtr pExa;
	int lines;
	uint32_t ap;
	uint8_t reg;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	pMgx->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = pMgx->fb;
	lines = (pMgx->vramsize - 0x1000) / (pScrn->displayWidth * 4);
	DBGMSG(X_ERROR, "lines %d\n", lines);	
	pExa->memorySize = lines * pScrn->displayWidth * 4;
	pExa->offScreenBase = pScrn->displayWidth * pScrn->virtualY * 4;
	pExa->pixmapOffsetAlign = pScrn->displayWidth * 4;
	pExa->pixmapPitchAlign = pScrn->displayWidth * 4;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS | EXA_MIXED_PIXMAPS;

	pExa->maxX = 2048;
	pExa->maxY = 2048;	

	pExa->WaitMarker = MgxWaitMarker;
	pExa->PrepareSolid = MgxPrepareSolid;
	pExa->Solid = MgxSolid;
	pExa->DoneSolid = MgxDoneCopy;
	pExa->PrepareCopy = MgxPrepareCopy;
	pExa->Copy = MgxCopy;
	pExa->DoneCopy = MgxDoneCopy;

	MgxWait(pMgx);

	/* XXX support other colour depths */
	reg = MgxRead1(pMgx, ATR_PIXEL);
	DBGMSG(X_ERROR, "pixel %x\n", reg);
	reg &= ~PIXEL_DEPTH_MASK;
	reg |= PIXEL_32;
	MgxWrite1(pMgx, ATR_PIXEL, reg);
	pMgx->dec = DEC_DEPTH_32 << DEC_DEPTH_SHIFT;

	ap = MgxRead4(pMgx, ATR_APERTURE);
	MgxWrite2(pMgx, ATR_APERTURE, 0xffff);
	ap = MgxRead4(pMgx, ATR_APERTURE);

	switch (pScrn->displayWidth) {
		case 640:
			pMgx->dec |= DEC_WIDTH_640 << DEC_WIDTH_SHIFT;
			break;
		case 800:
			pMgx->dec |= DEC_WIDTH_800 << DEC_WIDTH_SHIFT;
			break;
		case 1024:
			pMgx->dec |= DEC_WIDTH_1024 << DEC_WIDTH_SHIFT;
			break;
		case 1152:
			pMgx->dec |= DEC_WIDTH_1152 << DEC_WIDTH_SHIFT;
			break;
		case 1280:
			pMgx->dec |= DEC_WIDTH_1280 << DEC_WIDTH_SHIFT;
			break;
		case 1600:
			pMgx->dec |= DEC_WIDTH_1600 << DEC_WIDTH_SHIFT;
			break;
		default:
			return FALSE; /* not supported */
	}

	/*
	 * the fb is endian-twiddled and we don't know how to turn it off, 
	 * so we convert data when copying stuff in and out 
	 */
	pExa->UploadToScreen = MgxUploadToScreen;
	pExa->DownloadFromScreen = MgxDownloadFromScreen;
	return exaDriverInit(pScreen, pExa);
}
