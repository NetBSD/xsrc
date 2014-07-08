/*
 * TCX framebuffer - hardware acceleration.
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

/* $NetBSD: tcx_accel.c,v 1.9 2014/07/08 17:05:26 macallan Exp $ */

#include <sys/types.h>

#include "tcx.h"

#ifdef DEBUG
#define ENTER xf86Msg(X_ERROR, "%s\n", __func__)
#define LEAVE xf86Msg(X_ERROR, "%s done\n", __func__)
#else
#define ENTER
#define LEAVE
#endif

static void
TcxWaitMarker(ScreenPtr pScreenInfo, int Marker)
{
    ENTER;
    /* do nothing */
}

static int
TcxMarkSync(ScreenPtr pScreenInfo)
{
    ENTER;
    return 0;
}

static Bool
TcxPrepareCopy
(
    PixmapPtr pSrcPixmap,
    PixmapPtr pDstPixmap,
    int       xdir,
    int       ydir,
    int       alu,
    Pixel     planemask
)
{
    ScrnInfoPtr pScreenInfo = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
    TcxPtr pTcx = GET_TCX_FROM_SCRN(pScreenInfo);

    ENTER;
    /* weed out the cases we can't accelerate */
#ifdef DEBUG
    xf86Msg(X_ERROR, "alu: %d mask %08x\n", alu, planemask);
#endif
    if (alu != GXcopy)
    	return FALSE;
    if ((planemask != 0xffffffff) && (planemask != 0x00ffffff))
	return FALSE;

    pTcx->xdir = xdir;
    pTcx->ydir = ydir;
    pTcx->srcoff = exaGetPixmapOffset(pSrcPixmap) >> pTcx->pitchshift;
    pTcx->srcpitch = exaGetPixmapPitch(pSrcPixmap) >> pTcx->pitchshift;
    LEAVE;
    return TRUE;
}

static void
TcxCopy
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
    ScrnInfoPtr pScreenInfo = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
    TcxPtr pTcx = GET_TCX_FROM_SCRN(pScreenInfo);
    uint64_t cmd, lcmd;
    int line, col, leftover, src, dst, xsteps, sstep, dstep, dpitch, x, xoff;
    int doff;

    ENTER;
    leftover = w & 0x1f;
    if (leftover > 0)
	    lcmd = 0x3000000000000000LL | (leftover - 1) << 24;
	    

    doff = exaGetPixmapOffset(pDstPixmap) >> pTcx->pitchshift;
    dpitch = exaGetPixmapPitch(pDstPixmap) >> pTcx->pitchshift;
    src = srcX + srcY * pTcx->srcpitch + pTcx->srcoff;
    dst = dstX + dstY * dpitch + doff;

    if (pTcx->ydir < 0) {
	src += (h - 1) * pTcx->srcpitch;
	dst += (h - 1) * dpitch;
	sstep = 0 - pTcx->srcpitch;
	dstep = 0 - dpitch;
    } else {
	sstep = pTcx->srcpitch;
	dstep = dpitch;
    }

    xsteps = w >> 5;

    if ((pTcx->xdir > 0) || (w < 33)) {
	for (line = 0; line < h; line++) {
	    x = xsteps;
	    xoff = 0;
	    while (x > 0) {
		cmd = 0x300000001f000000LL | (uint64_t)(src + xoff);
		pTcx->rblit[dst + xoff] = cmd;
		xoff += 32;
		x--;
	    }
	    if (leftover > 0) {
		cmd = lcmd | (uint64_t)(src + xoff);
		pTcx->rblit[dst + xoff] = cmd;
	    }
	    src += sstep;
	    dst += dstep;
	}
    } else {
	/* same thing but right to left */
	for (line = 0; line < h; line++) {
	    x = xsteps;
	    xoff = xsteps << 5;
	    if (leftover > 0) {
		cmd = lcmd | (uint64_t)(src + xoff);
		pTcx->rblit[dst + xoff] = cmd;
	    }
	    xoff -= 32;
	    while (x > 0) {
		cmd = 0x300000001f000000LL | (uint64_t)(src + xoff);
		pTcx->rblit[dst + xoff] = cmd;
		xoff -= 32;
		x--;
	    }
	    src += sstep;
	    dst += dstep;
	}
    }
    LEAVE;
}

static void
TcxDoneCopy(PixmapPtr pDstPixmap)
{
    ENTER;
    LEAVE;
}

static Bool
TcxPrepareSolid(
    PixmapPtr pPixmap,
    int alu,
    Pixel planemask,
    Pixel fg)
{
    ScrnInfoPtr pScreenInfo = xf86Screens[pPixmap->drawable.pScreen->myNum];
    TcxPtr pTcx = GET_TCX_FROM_SCRN(pScreenInfo);
    uint32_t hwfg;

    ENTER;

    /* weed out the cases we can't accelerate */
    if (pTcx->HasStipROP) {
    	hwfg = alu << 28;
    } else if (alu == GXcopy) {
        hwfg = 0x30000000;
    } else 
    	return FALSE;

    if ((planemask != 0xffffffff) && (planemask != 0x00ffffff))
	return FALSE;
    if (exaGetPixmapOffset(pPixmap) != 0)
	return FALSE;
    pTcx->fg = (fg & 0x00ffffff);
    /* set colour space ID if we're in 24bit mode */
    if (pTcx->pitchshift != 0)
    	hwfg |= 0x03000000;
    pTcx->fg |= hwfg;
#ifdef DEBUG
    xf86Msg(X_ERROR, "fg: %08x\n", hwfg);
#endif
    LEAVE;
    return TRUE;
}

static void
TcxSolid(
    PixmapPtr pPixmap,
    int x1,
    int y1,
    int x2,
    int y2)
{
    ScrnInfoPtr pScreenInfo = xf86Screens[pPixmap->drawable.pScreen->myNum];
    TcxPtr pTcx = GET_TCX_FROM_SCRN(pScreenInfo);
    int dpitch, dst, line, fullsteps, i;
    uint64_t cmd, rcmd, lcmd, tmpl;
    uint32_t pmask;

    dpitch = exaGetPixmapPitch(pPixmap) >> pTcx->pitchshift;
    dst = x1 + y1 * dpitch;

    tmpl = ((uint64_t)pTcx->fg) << 32;

    /*
     * thanks to the funky architecture of the tcx's stipple 'engine' we have
     * to deal with two different cases:
     * - the whole width of the rectangle fits into a single 32 pixel aligned
     *   unit of 32 pixels
     * - the first and the last 32bit unit may or may not contain less than
     *   32 pixels
     */
    x2 -= 1;
    if ((x1 & 0xffe0) == (x2 & 0xffe0)) {
	/* the whole width fits in one 32 pixel write */

	/* first zero out pixels on the right */
	pmask = 0xffffffff << (31 - (x2 & 0x1f));
	/* then mask out pixels on the left */
	pmask &= (0xffffffff >> (x1 & 0x1f));
#ifdef DEBUG
	xf86Msg(X_ERROR, "%d %d %08x %d %d\n", x1, x2, pmask, y1, y2);
#endif
	cmd = tmpl | (uint64_t)pmask;
	dst &= 0xffffffe0;
	for (line = y1; line < y2; line++) {
	    pTcx->rstip[dst] = cmd;
	    dst += dpitch;
	}
    } else {
	/* at least two writes per line */
	pmask = 0xffffffff << (31 - (x2 & 0x1f));
	rcmd = tmpl | (uint64_t)pmask;
	pmask = 0xffffffff >> (x1 & 0x1f);
	lcmd = tmpl | (uint64_t)pmask;
	cmd = tmpl | 0xffffffffLL;
	dst &= 0xffffffe0;
	fullsteps = ((x2 >> 5) - (x1 >> 5));
#ifdef DEBUG
	xf86Msg(X_ERROR, "%d %d %08x %d %d\n", x1, x2, pmask, y1, y2);
	xf86Msg(X_ERROR, "fullsteps: %d\n", fullsteps);
#endif
	fullsteps = fullsteps << 5;
	for (line = y1; line < y2; line++) {
	    pTcx->rstip[dst] = lcmd;
	    for (i = 32; i < fullsteps; i+= 32)
		pTcx->rstip[dst + i] = cmd;
	    pTcx->rstip[dst + i] = rcmd;
	    dst += dpitch;
	}
    }
}

/*
 * Memcpy-based UTS.
 */
static Bool
TcxUploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
    ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
    TcxPtr pTcx       = GET_TCX_FROM_SCRN(pScrn);
    char  *dst        = pTcx->fb + exaGetPixmapOffset(pDst);
    int    dst_pitch  = exaGetPixmapPitch(pDst);

    int bpp    = pDst->drawable.bitsPerPixel;
    int cpp    = (bpp + 7) / 8;
    int wBytes = w * cpp;

    ENTER;
    dst += (x * cpp) + (y * dst_pitch);

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
TcxDownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
    ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
    TcxPtr pTcx       = GET_TCX_FROM_SCRN(pScrn);
    char  *src        = pTcx->fb + exaGetPixmapOffset(pSrc);
    int    src_pitch  = exaGetPixmapPitch(pSrc);

    int bpp    = pSrc->drawable.bitsPerPixel;
    int cpp    = (bpp + 7) / 8;
    int wBytes = w * cpp;

    ENTER;
    src += (x * cpp) + (y * src_pitch);

    while (h--) {
        memcpy(dst, src, wBytes);
        src += src_pitch;
        dst += dst_pitch;
    }
    LEAVE;
    return TRUE;
}

Bool
TcxInitAccel(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    TcxPtr pTcx = GET_TCX_FROM_SCRN(pScrn);
    ExaDriverPtr pExa;

    pExa = exaDriverAlloc();
    if (!pExa)
        return FALSE;

    pTcx->pExa = pExa;

    pExa->exa_major = EXA_VERSION_MAJOR;
    pExa->exa_minor = EXA_VERSION_MINOR;

    /*
     * The S24 can display both 8 and 24bit data at the same time, and in
     * 24bit we can choose between gamma corrected and direct. No idea how that
     * would map to EXA - we'd have to pick the right framebuffer to draw into
     * and Solid() would need to know what kind of pixels to write
     */
    pExa->memoryBase = pTcx->fb;
    if (pScrn->depth == 8) {
	pExa->memorySize = pTcx->vramsize;
	pExa->offScreenBase = pTcx->psdp->width * pTcx->psdp->height;
	pExa->pixmapOffsetAlign = 1;
	pExa->pixmapPitchAlign = 1;
    } else {
	pExa->memorySize = 1024 * 1024 * 4;
	pExa->offScreenBase = pTcx->psdp->width * pTcx->psdp->height * 4;
	pExa->pixmapOffsetAlign = 4;
	pExa->pixmapPitchAlign = 4;
    }

    pExa->flags = EXA_OFFSCREEN_PIXMAPS;

    pExa->maxX = 2048;
    pExa->maxY = 2048;	/* dummy, available VRAM is the limit */

    pExa->MarkSync = TcxMarkSync;
    pExa->WaitMarker = TcxWaitMarker;

    pExa->PrepareSolid = TcxPrepareSolid;
    pExa->Solid = TcxSolid;
    pExa->DoneSolid = TcxDoneCopy;

    pExa->PrepareCopy = TcxPrepareCopy;
    pExa->Copy = TcxCopy;
    pExa->DoneCopy = TcxDoneCopy;

    /* EXA hits more optimized paths when it does not have to fallback because
     * of missing UTS/DFS, hook memcpy-based UTS/DFS.
     */
    pExa->UploadToScreen = TcxUploadToScreen;
    pExa->DownloadFromScreen = TcxDownloadFromScreen;

    return exaDriverInit(pScreen, pExa);
}
