/* $NetBSD: crime_accel.c,v 1.16 2011/05/23 19:49:27 christos Exp $ */
/*
 * Copyright (c) 2008 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* a driver for the CRIME rendering engine foundin SGI O2 workstations */

#include "crime.h"
#include "picturestr.h"
#include "xaalocal.h"
#include "xaa.h"
#include "fb.h"
#include "fbpict.h"
#include "mi.h"
#include "mipict.h"
#include "mizerarc.h"

uint32_t regcache[0x1000];

#define CRIMEREG(p) (volatile uint32_t *)(fPtr->engine + p)
#define WBFLUSH __asm("sync")
#if 0
#define WRITE4(r, v) {if (regcache[r >> 2] != v) { \
			*CRIMEREG(r) = v; \
			regcache[r >> 2] = v; } }
#else
#define WRITE4(r, v) { *CRIMEREG(r) = v; }
#endif
#define WRITE4ST(r, v) {WBFLUSH; *CRIMEREG(r + CRIME_DE_START) = v; WBFLUSH;}
#ifdef CRIME_DEBUG
#define SYNC { int bail = 0; do {bail++; } \
          while(((*CRIMEREG(0x4000) & CRIME_DE_IDLE) == 0) && (bail < 10000000)); \
          if (bail == 10000000) { \
          	xf86Msg(X_ERROR, "sync timeout\n"); \
          	WRITE4ST(CRIME_MTE_FLUSH, 0); \
          	WRITE4ST(CRIME_DE_FLUSH, 0); \
          } \
        }
#else
#define SYNC do {} while ((*CRIMEREG(0x4000) & CRIME_DE_IDLE) == 0)
#endif
#define SYNCMTE do {} while ((*CRIMEREG(0x4000) & CRIME_DE_MTE_IDLE) == 0)
#define MAKE_ROOM(x) do {} while ((16 - \
				   CRIME_PIPE_LEVEL(*CRIMEREG(0x4000))) < x);

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

CARD32 CrimeAlphaTextureFormats[] = {PICT_a8, 0};
CARD32 CrimeTextureFormats[] = {PICT_a8b8g8r8, PICT_a8r8g8b8, 0};

void
CrimeSync(ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
#if defined(CRIME_DEBUG_LOUD) && (CRIME_DEBUG_MASK & CRIME_DEBUG_SYNC)
	volatile uint32_t *status = CRIMEREG(CRIME_DE_STATUS);

	xf86Msg(X_ERROR, "%s: %08x\n", __func__, *status);
#endif
	LOG(CRIME_DEBUG_SYNC);
	SYNC;
}

static void
CrimeSetupForScreenToScreenCopy(
	ScrnInfoPtr  pScrn,
	int          xdir,
	int          ydir,
	int          rop,
	unsigned int planemask,
	int          TransparencyColour
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_BITBLT);
	MAKE_ROOM(9);
	if ((rop == GXcopy) && (planemask == 0xffffffff) && (xdir > 0)) {
		/* use the MTE */
		WRITE4(CRIME_MTE_MODE, MTE_MODE_DST_ECC |
		    MTE_TLB_A << MTE_DST_TLB_SHIFT |
		    MTE_TLB_A << MTE_SRC_TLB_SHIFT |
		    MTE_DEPTH_32 << MTE_DEPTH_SHIFT |
		    MTE_MODE_COPY);
		fPtr->use_mte = 1;
		if (ydir > 0) {
			WRITE4(CRIME_MTE_DST_Y_STEP, 1);
			WRITE4(CRIME_MTE_SRC_Y_STEP, 1);
		} else {
			WRITE4(CRIME_MTE_DST_Y_STEP, -1);
			WRITE4(CRIME_MTE_SRC_Y_STEP, -1);
		}		
	} else
		fPtr->use_mte = 0;

	WRITE4(CRIME_DE_XFER_STEP_X, 1);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | 
	    DE_DRAWMODE_ROP | DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
		    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	fPtr->xdir = xdir;
	fPtr->ydir = ydir;
	SYNC;
	DONE(CRIME_DEBUG_BITBLT);
}

static void
CrimeSubsequentScreenToScreenCopy
(
	ScrnInfoPtr pScrn,
	int         xSrc,
	int         ySrc,
	int         xDst,
	int         yDst,
	int         w,
	int         h
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t prim = DE_PRIM_RECTANGLE;
	uint32_t rxa, rya, rxe, rye, rxs, rys, rxd, ryd, rxde, ryde;

	LOG(CRIME_DEBUG_BITBLT);
#if defined(CRIME_DEBUG_LOUD) && (CRIME_DEBUG_MASK & CRIME_DEBUG_BITBLT)
	xf86Msg(X_ERROR, "%s: %d, %d; %d x %d -> %d %d\n", __func__,
	    xSrc, ySrc, w, h, xDst, yDst);
#endif
	if ((fPtr->use_mte) && (w > 64) && (abs(ySrc - yDst) > 4) &&
	   ((xSrc & 15) == (xDst & 15))) {
		if (fPtr->ydir == -1) {
			/* bottom to top */
			rye = ySrc;
			rya = ySrc + h - 1;
			ryd = yDst + h - 1;
			ryde = yDst;
		} else {
			/* top to bottom */
			rye = ySrc + h - 1;
			rya = ySrc;
			ryd = yDst;
			ryde = yDst + h - 1;
		}
		rxa = xSrc << 2;
		rxe = ((xSrc + w) << 2) - 1;
		rxd = xDst << 2;
		rxde = ((xDst + w) << 2) - 1;
		MAKE_ROOM(4);
		WRITE4(CRIME_MTE_SRC0, (rxa << 16) | rya);
		WRITE4(CRIME_MTE_SRC1, (rxe << 16) | rye);
		WRITE4(CRIME_MTE_DST0, (rxd << 16) | ryd);
		WRITE4ST(CRIME_MTE_DST1, (rxde << 16) | ryde);

	} else {
		if (fPtr->xdir == -1) {
			prim |= DE_PRIM_RL;
			rxe = xDst;
			rxa = xDst + w - 1;
			rxs = xSrc + w - 1;
		} else {
			prim |= DE_PRIM_LR;
			rxe = xDst + w - 1;
			rxa = xDst;
			rxs = xSrc;
		}
		if (fPtr->ydir == -1) {
			prim |= DE_PRIM_BT;
			rye = yDst;
			rya = yDst + h - 1;
			rys = ySrc + h - 1;
		} else {
			prim |= DE_PRIM_TB;
			rye = yDst + h - 1;
			rya = yDst;
			rys = ySrc;
		}

		MAKE_ROOM(4);
		WRITE4(CRIME_DE_PRIMITIVE, prim);
		WRITE4(CRIME_DE_XFER_ADDR_SRC,(rxs << 16) | (rys & 0xffff));
		WRITE4(CRIME_DE_X_VERTEX_0, (rxa << 16) | (rya & 0xffff));
		WRITE4ST(CRIME_DE_X_VERTEX_1, (rxe << 16) | (rye & 0xffff));
	}
	DONE(CRIME_DEBUG_BITBLT);
}

static void
CrimeSetupForSolidFill
(
    ScrnInfoPtr  pScrn,
    int          colour,
    int          rop,
    unsigned int planemask
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	int i;

	LOG(CRIME_DEBUG_RECTFILL);
#ifdef MTE_DRAW_RECT
	if (rop == GXcopy) {
		fPtr->use_mte = 1;
		MAKE_ROOM(3);
		WRITE4(CRIME_MTE_MODE, MTE_MODE_DST_ECC |
		    MTE_TLB_A << MTE_DST_TLB_SHIFT |
		    MTE_TLB_A << MTE_SRC_TLB_SHIFT |
		    MTE_DEPTH_32 << MTE_DEPTH_SHIFT);
		WRITE4(CRIME_MTE_DST_Y_STEP, 1);
		WRITE4(CRIME_MTE_BG, colour << 8);
		SYNCMTE;
	} else
#endif
	{
		fPtr->use_mte = 0;
		MAKE_ROOM(7);
		WRITE4(CRIME_DE_PLANEMASK, planemask);
		WRITE4(CRIME_DE_ROP, rop);
		WRITE4(CRIME_DE_FG, colour << 8);
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | 	
		    DE_DRAWMODE_ROP |\
		    DE_DRAWMODE_SCISSOR_EN);
		WRITE4(CRIME_DE_PRIMITIVE,
		    DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
		WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
		    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
		SYNC;
	}
	DONE(CRIME_DEBUG_RECTFILL);
}

static void
CrimeSubsequentSolidFillRect
(
    ScrnInfoPtr pScrn,
    int         x,
    int         y,
    int         w,
    int         h
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	int xa, xe, ya, ye;

	LOG(CRIME_DEBUG_RECTFILL);
#ifdef MTE_DRAW_RECT
	if (fPtr->use_mte) {
		
		/*
		 * the MTE doesn't support clipping so we have to do it
		 * ourselves - luckily it's trivial with rectangles
		 */
		xa = MAX(fPtr->cxa, x);
		ya = MAX(fPtr->cya, y);
		xe = MIN(fPtr->cxe, x + w);
		ye = MIN(fPtr->cye, y + h);
		if ((xa < xe) && (ya < ye)) {
			MAKE_ROOM(2);
			WRITE4(CRIME_MTE_DST0, (xa << 18) | (ya & 0xffff));
			WRITE4ST(CRIME_MTE_DST1,
		 	   (((xe << 2) - 1 ) << 16) | ((ye - 1) & 0xffff));
		}
	} else 
#endif
	{
		MAKE_ROOM(2);
		WRITE4(CRIME_DE_X_VERTEX_0, (x << 16) | (y & 0xffff));
		WRITE4ST(CRIME_DE_X_VERTEX_1,
		    ((x + w - 1) << 16) | ((y + h - 1) & 0xffff));
	}
	DONE(CRIME_DEBUG_RECTFILL);
}

static void
CrimeSetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int patx, int paty,
        int fg, int bg, int rop, unsigned int planemask)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t pat;

	LOG(CRIME_DEBUG_RECTFILL);
	MAKE_ROOM(7);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, fg << 8);
	if (bg == -1) {
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_POLY_STIP |
		    DE_DRAWMODE_SCISSOR_EN);
	} else {
		WRITE4(CRIME_DE_BG, bg << 8);
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | 
		    DE_DRAWMODE_ROP | DE_DRAWMODE_POLY_STIP | 
		    DE_DRAWMODE_OPAQUE_STIP | DE_DRAWMODE_SCISSOR_EN);
	}
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);

	/*
	 * we need to store the pattern so we can just hammer it into the
	 * stipple register later on
	 */
	pat = patx & 0xff000000;
	pat |= pat >> 8;
	fPtr->pattern[0] = pat | (pat >> 16);
	pat = patx & 0x00ff0000;
	pat |= pat << 8;
	fPtr->pattern[1] = pat | (pat >> 16);
	pat = patx & 0x0000ff00;
	pat |= pat >> 8;
	fPtr->pattern[2] = pat | (pat << 16);
	pat = patx & 0x000000ff;
	pat |= pat << 8;
	fPtr->pattern[3] = pat | (pat << 16);

	pat = paty & 0xff000000;
	pat |= pat >> 8;
	fPtr->pattern[4] = pat | (pat >> 16);
	pat = paty & 0x00ff0000;
	pat |= pat << 8;
	fPtr->pattern[5] = pat | (pat >> 16);
	pat = paty & 0x0000ff00;
	pat |= pat >> 8;
	fPtr->pattern[6] = pat | (pat << 16);
	pat = paty & 0x000000ff;
	pat |= pat << 8;
	fPtr->pattern[7] = pat | (pat << 16);
	SYNC;
	DONE(CRIME_DEBUG_RECTFILL);
}

static void
CrimeSubsequentMono8x8PatternFillRect( ScrnInfoPtr pScrn,
        	int patx, int paty, int x, int y, int w, int h)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	int i, pat;

	LOG(CRIME_DEBUG_RECTFILL);

	/* first setup the stipple stuff */
	
	MAKE_ROOM(1);
	WRITE4(CRIME_DE_STIPPLE_MODE, 0x001f0000 | (patx << 24));
	pat = paty;
	
	for (i = 0; i < h; i++) {
		MAKE_ROOM(3);
		WRITE4(CRIME_DE_STIPPLE_PAT, fPtr->pattern[pat]);
		WRITE4(CRIME_DE_X_VERTEX_0, (x << 16) | ((y + i) & 0xffff));
		WRITE4ST(CRIME_DE_X_VERTEX_1,
		    ((x + w - 1) << 16) | ((y + i) & 0xffff));
		pat = (pat + 1) & 7;
	}
	DONE(CRIME_DEBUG_RECTFILL);
}

static void
CrimeSetupForScanlineImageWrite(ScrnInfoPtr pScrn, int rop, 
                                unsigned int planemask, int trans_color, 
                                int bpp, int depth)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_IMAGEWRITE);
#if defined(CRIME_DEBUG_LOUD) && (CRIME_DEBUG_MASK & CRIME_DEBUG_IMAGEWRITE)
	if ((bpp == 24) || (depth == 24))
	xf86Msg(X_ERROR, "%s: %d %d \n", __func__, bpp, depth);
#endif
	MAKE_ROOM(7);
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGB | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_XFER_STEP_X, 4);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	SYNC;
	DONE(CRIME_DEBUG_IMAGEWRITE);
}

static void
CrimeSubsequentImageWriteRect(ScrnInfoPtr pScrn, 
                                int x, int y, int w, int h, int skipleft)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_IMAGEWRITE);

#if defined(CRIME_DEBUG_LOUD) && (CRIME_DEBUG_MASK & CRIME_DEBUG_IMAGEWRITE)
	xf86Msg(X_ERROR, "%s: %d %d %d %d\n", __func__, x, y, w, h);
#endif

	fPtr->start = skipleft;
	x += skipleft;
	w -= skipleft;
	if (x < 0) {
		fPtr->ux = 0;
		w += x;
		fPtr->start -= x;
	} else {
		fPtr->ux = x;
	}
	fPtr->uy = y;
	fPtr->uw = w;
	fPtr->uh = h;
	DONE(CRIME_DEBUG_IMAGEWRITE);
}

static void
CrimeSubsequentImageWriteScanline(ScrnInfoPtr pScrn, int bufno)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_IMAGEWRITE);
	/*
	 * we need to sync here, otherwise we might queue up more copy
	 * commands than we have buffers
	 */
	SYNC;
	WRITE4(CRIME_DE_XFER_ADDR_SRC, (bufno << 13)  + (fPtr->start << 2));
	WRITE4(CRIME_DE_X_VERTEX_0, (fPtr->ux << 16) | fPtr->uy);
	WRITE4ST(CRIME_DE_X_VERTEX_1,
		((fPtr->ux + fPtr->uw - 1) << 16) | (fPtr->uy));
	fPtr->uy++;
	DONE(CRIME_DEBUG_IMAGEWRITE);
}

static void 
CrimeSetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
        		int fg, int bg,
			int rop,
			unsigned int planemask)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_COLOUREXPAND);
	MAKE_ROOM(7);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, fg << 8);
	if (bg == -1) {
		/* transparent */
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_POLY_STIP);
	} else {
		WRITE4(CRIME_DE_BG, bg << 8);
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | 
		    DE_DRAWMODE_OPAQUE_STIP | DE_DRAWMODE_POLY_STIP);
	}
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	SYNC;
	DONE(CRIME_DEBUG_COLOUREXPAND);
}

static void 
CrimeSubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
			int x, int y, int w, int h,
			int skipleft )
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_COLOUREXPAND);

	fPtr->start = skipleft;
	fPtr->ux = x;
	fPtr->uy = y;
	fPtr->uw = w;
	fPtr->uh = h;
	DONE(CRIME_DEBUG_COLOUREXPAND);
}

static void
CrimeSubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t *boo = (uint32_t *)fPtr->expandbuffers[bufno];
	int idx = fPtr->uw, x = fPtr->ux;
	
	LOG(CRIME_DEBUG_COLOUREXPAND);

	MAKE_ROOM(5);
	WRITE4(CRIME_DE_STIPPLE_MODE, 0x001f0000 | (fPtr->start << 24));
	WRITE4(CRIME_DE_STIPPLE_PAT, *boo);
	boo++;
	WRITE4(CRIME_DE_X_VERTEX_0, (x + fPtr->start << 16) | fPtr->uy);
	WRITE4ST(CRIME_DE_X_VERTEX_1,
		((x + min(idx, 32) - 1) << 16) | (fPtr->uy));
	idx -= 32;
	x += 32;
	WRITE4(CRIME_DE_STIPPLE_MODE, 0x001f0000);
	
	while (idx > 0) {
		MAKE_ROOM(3);
		WRITE4(CRIME_DE_STIPPLE_PAT, *boo);
		boo++;
		WRITE4(CRIME_DE_X_VERTEX_0, (x << 16) | fPtr->uy);
		WRITE4ST(CRIME_DE_X_VERTEX_1,
			((x + min(idx, 32) - 1) << 16) | (fPtr->uy));
		idx -= 32;
		x += 32;
	}
	fPtr->uy++;
	DONE(CRIME_DEBUG_COLOUREXPAND);
}

static void 
CrimeSetupForSolidLine(ScrnInfoPtr pScrn, int color, int rop, 
    unsigned int planemask)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_LINES);

	MAKE_ROOM(5);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, color << 8);
	WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_SCISSOR_EN);
	SYNC;
	DONE(CRIME_DEBUG_LINES);
}

static void
CrimeSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn, int x1, int y1, int x2,
    int y2, int flags)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_LINES);
	MAKE_ROOM(3);
	if (flags & OMIT_LAST) {
		WRITE4(CRIME_DE_PRIMITIVE,
			DE_PRIM_LINE | DE_PRIM_LINE_SKIP_END | 2);
	} else {
		WRITE4(CRIME_DE_PRIMITIVE,
			DE_PRIM_LINE | 2);
	}
	WRITE4(CRIME_DE_X_VERTEX_0, (x1 << 16) | y1);
	WRITE4ST(CRIME_DE_X_VERTEX_1, (x2 << 16) | y2);
	DONE(CRIME_DEBUG_LINES);
}

void
CrimeSetupForDashedLine(ScrnInfoPtr pScrn,
		int fg, int bg, int rop, unsigned int planemask,
        	int length, unsigned char *pattern)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t pat;

	LOG(CRIME_DEBUG_LINES);

	fPtr->uw = length;
	MAKE_ROOM(7);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, fg << 8);
	if (bg == -1) {
		/* transparent */
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_LINE_STIP |
		    DE_DRAWMODE_SCISSOR_EN);
	} else {
		WRITE4(CRIME_DE_BG, bg << 8);
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_SCISSOR_EN |
		    DE_DRAWMODE_OPAQUE_STIP | DE_DRAWMODE_LINE_STIP);
	}
	/*
	 * can we trust the Xserver to always hand us a 32bit aligned 
	 * pattern buffer?
	 */
	memcpy(&pat, pattern, 4);
	WRITE4(CRIME_DE_STIPPLE_PAT, pat);
	SYNC;
	DONE(CRIME_DEBUG_LINES);
}

void
CrimeSubsequentDashedTwoPointLine( ScrnInfoPtr pScrn,
        int x1, int y1, int x2, int y2, int flags, int phase)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t stipmode;

	LOG(CRIME_DEBUG_LINES);
	MAKE_ROOM(4);

	if (flags & OMIT_LAST) {
		WRITE4(CRIME_DE_PRIMITIVE,
			DE_PRIM_LINE | DE_PRIM_LINE_SKIP_END | 2);
	} else {
		WRITE4(CRIME_DE_PRIMITIVE,
			DE_PRIM_LINE | 2);
	}

	stipmode = ((fPtr->uw - 1) << 16) | (phase << 24);
	WRITE4(CRIME_DE_STIPPLE_MODE, stipmode);
	WRITE4(CRIME_DE_X_VERTEX_0, (x1 << 16) | y1);
	WRITE4ST(CRIME_DE_X_VERTEX_1, (x2 << 16) | y2);
	DONE(CRIME_DEBUG_LINES);
}

void
CrimeSetClippingRectangle ( ScrnInfoPtr pScrn,
                        int left, int top, int right, int bottom)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_CLIPPING);
	MAKE_ROOM(2);
	WRITE4(CRIME_DE_SCISSOR, (left << 16) | top);
	WRITE4(CRIME_DE_SCISSOR + 4, ((right + 1) << 16) | (bottom + 1));
	fPtr->cxa = left;
	fPtr->cxe = right;
	fPtr->cya = top;
	fPtr->cye = bottom;
	SYNC;
	DONE(CRIME_DEBUG_CLIPPING);
}

void
CrimeDisableClipping (ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_CLIPPING);
	MAKE_ROOM(2);
	WRITE4(CRIME_DE_SCISSOR, 0);
	WRITE4(CRIME_DE_SCISSOR + 4, 0x3fff3fff);
	fPtr->cxa = 0;
	fPtr->cxe = 2047;
	fPtr->cya = 0;
	fPtr->cye = 2047;
	SYNC;
	DONE(CRIME_DEBUG_CLIPPING);
}

static Bool
CrimeSetupForCPUToScreenAlphaTexture (
   ScrnInfoPtr	pScrn,
   int		op,
   CARD16	red,
   CARD16	green,
   CARD16	blue,
   CARD16	alpha,
   int		alphaType,
   CARD8	*alphaPtr,
   int		alphaPitch,
   int		width,
   int		height,
   int		flags
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	if (op != PictOpOver) {
		xf86Msg(X_ERROR, "%s: op %d\n", __func__, op);
		op = PictOpOver;
	}

	LOG(CRIME_DEBUG_XRENDER);

	fPtr->alpha_color = ((red & 0xff00) << 16) |
			    ((green & 0xff00) << 8) |
			    (blue & 0xff00);
	fPtr->uw = width;
	fPtr->uh = height;
	fPtr->us = alphaPitch;
	fPtr->alpha_texture = alphaPtr;
	fPtr->format = alphaType;
#ifdef CRIME_DEBUG_LOUD
	if (alphaType != PICT_a8) {
		xf86Msg(X_ERROR, "ARGB mask %08x %d\n", (uint32_t)alphaPtr,
		    alphaPitch);
	}
	xf86Msg(X_ERROR, "%s: %x %x %x\n", __func__, red, green, blue);
#endif
	MAKE_ROOM(7);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	/* XXX this register is not where it's supposed to be */
	WRITE4(CRIME_DE_ALPHA_COLOR, fPtr->alpha_color);
	if (alphaType == PICT_a8) {
		if (fPtr->alpha_color == 0) {
			WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_8 |
				    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
			WRITE4(CRIME_DE_XFER_STEP_X, 1);
			WRITE4(CRIME_DE_ALPHA_FUNC, 
			    DE_ALPHA_ADD |
			    (DE_ALPHA_OP_ZERO << DE_ALPHA_OP_SRC_SHIFT) |
			    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
		} else {
			WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
				    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
			WRITE4(CRIME_DE_XFER_STEP_X, 4);
			WRITE4(CRIME_DE_ALPHA_FUNC, 
			    DE_ALPHA_ADD |
			    (DE_ALPHA_OP_SRC_ALPHA << DE_ALPHA_OP_SRC_SHIFT) |
			    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
		}
	} else {
		WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
				    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
		WRITE4(CRIME_DE_XFER_STEP_X, 4);
		WRITE4(CRIME_DE_ALPHA_FUNC, 
		    DE_ALPHA_ADD |
		    (DE_ALPHA_OP_SRC_ALPHA << DE_ALPHA_OP_SRC_SHIFT) |
		    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
	}	
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	SYNC;
	DONE(CRIME_DEBUG_XRENDER);
	return TRUE;
}

void
CrimeSubsequentCPUToScreenAlphaTexture (
    ScrnInfoPtr	pScrn,
    int		dstx,
    int		dsty,
    int		srcx,
    int		srcy,
    int		width,
    int		height
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	unsigned char *aptr;
	uint32_t *dptr, aval;
	int i, j;
	int bufnum = 0;

	LOG(CRIME_DEBUG_XRENDER);
#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%d %d %d %d %d %d\n",srcx, srcy, dstx, dsty, width, 
	    height); 
#endif
	aptr = fPtr->alpha_texture + (fPtr->us * srcy) + srcx;
	for (i = 0; i < height; i++) {
		dptr = (uint32_t *)fPtr->buffers[bufnum];
		if (fPtr->alpha_color == 0) {
			memcpy(dptr, aptr, width);
		} else {
			for (j = 0; j < width; j++) {
				aval = aptr[j];
				*dptr = aval | fPtr->alpha_color;
				dptr++;
			}
		}
		MAKE_ROOM(3);
		WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
		WRITE4(CRIME_DE_X_VERTEX_0, dstx << 16 | (dsty + i));
		WRITE4ST(CRIME_DE_X_VERTEX_1,
			((dstx + width - 1) << 16) | (dsty + i));
		bufnum++;
		if (bufnum == 8) bufnum = 0;
		aptr += fPtr->us;
	}
	DONE(CRIME_DEBUG_XRENDER);
}

void
CrimeSubsequentCPUToScreenAlphaTexture32 (
    ScrnInfoPtr	pScrn,
    int		dstx,
    int		dsty,
    int		srcx,
    int		srcy,
    int		width,
    int		height
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint8_t *aptr;
	uint32_t *dptr, *sptr;
	int i, j;
	int bufnum = 0;

	LOG(CRIME_DEBUG_XRENDER);
#ifdef CRIME_DEBUG_LOUD_
	xf86Msg(X_ERROR, "%d %d %d %d %d %d\n",srcx, srcy, dstx, dsty, width, 
	    height); 
#endif
	aptr = fPtr->alpha_texture + (fPtr->us * srcy) + (srcx << 2);
	for (i = 0; i < height; i++) {
		dptr = (uint32_t *)fPtr->buffers[bufnum];
		sptr = (uint32_t *)aptr;
		for (j = 0; j < width; j++) {
			*dptr = (*sptr >> 24) | fPtr->alpha_color;
#ifdef CRIME_DEBUG_LOUD_
			xf86Msg(X_ERROR, "%08x %08x\n", *sptr, *dptr);
#endif
			sptr++;
			dptr++;
		}
		MAKE_ROOM(3);
		WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
		WRITE4(CRIME_DE_X_VERTEX_0, dstx << 16 | (dsty + i));
		WRITE4ST(CRIME_DE_X_VERTEX_1,
			((dstx + width - 1) << 16) | (dsty + i));
		bufnum++;
		if (bufnum == 8) bufnum = 0;
		aptr += fPtr->us;
	}
	DONE(CRIME_DEBUG_XRENDER);
}

static Bool
CrimeSetupForCPUToScreenTexture (
   ScrnInfoPtr	pScrn,
   int		op,
   int		texType,
   CARD8	*texPtr,
   int		texPitch,
   int		width,
   int		height,
   int		flags
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	if (op != PictOpOver) {
		xf86Msg(X_ERROR, "%s: op %d\n", __func__, op);
		op = PictOpOver;
	}

	LOG(CRIME_DEBUG_XRENDER);

	fPtr->uw = width;
	fPtr->uh = height;
	fPtr->us = texPitch;
	fPtr->alpha_texture = texPtr;
	MAKE_ROOM(6);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	if (texType == PICT_a8b8g8r8) {
		WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
				    DE_MODE_TYPE_ABGR | DE_MODE_PIXDEPTH_32);
	} else {
		WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
				    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	}
	fPtr->format = texType;
	WRITE4(CRIME_DE_XFER_STEP_X, 4);
	WRITE4(CRIME_DE_ALPHA_FUNC, 
	    DE_ALPHA_ADD |
	    (DE_ALPHA_OP_ONE/*SRC_ALPHA*/ << DE_ALPHA_OP_SRC_SHIFT) |
	    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	SYNC;
	DONE(CRIME_DEBUG_XRENDER);
	return TRUE;
}

void
CrimeSubsequentCPUToScreenTexture (
    ScrnInfoPtr	pScrn,
    int		dstx,
    int		dsty,
    int		srcx,
    int		srcy,
    int		width,
    int		height
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	unsigned char *aptr, *lptr;
	uint32_t *dptr, *sptr, aval, pixel;
	int i, j, k, l, rep = 1, period = fPtr->uw, xoff, lcnt, xa, xe;
	int bufnum = 0;

	LOG(CRIME_DEBUG_XRENDER);
#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%s: %d %d %d %d %d %d\n", __func__,
	    srcx, srcy, dstx, dsty, width, height); 
#endif
	if ((width == 1) || (fPtr->format != PICT_a8r8g8b8)) {
		xf86Msg(X_ERROR, "%s: %d %d %d %d %d %d %d %d\n", __func__,
		    srcx, srcy, dstx, dsty, width, height, fPtr->uw, fPtr->us); 
		return;
	}

	aptr = fPtr->alpha_texture + (srcx << 2);
	lptr = aptr + (fPtr->us * srcy);
	if ((fPtr->uw < 128) && (fPtr->uw < width)) {
		rep = 128 / fPtr->uw;
		period = rep * fPtr->uw;
	}

	if (fPtr->format == PICT_a8b8g8r8) {
#ifdef CRIME_DEBUG_LOUD
		xf86Msg(X_ERROR, "ABGR\n");
#endif
		for (i = 0; i < height; i++) {
			dptr = (uint32_t *)fPtr->buffers[bufnum];
			memcpy(dptr, aptr, fPtr->us);
			MAKE_ROOM(3);
			WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
			WRITE4(CRIME_DE_X_VERTEX_0, dstx << 16 | (dsty + i));
			WRITE4ST(CRIME_DE_X_VERTEX_1,
				((dstx + width - 1) << 16) | (dsty + i));
			bufnum++;
			if (bufnum == 8) bufnum = 0;
			aptr += fPtr->us;
		}
	} else {
#ifdef CRIME_DEBUG_LOUD
		xf86Msg(X_ERROR, "ARGB %08x %d\n", (uint32_t)aptr, fPtr->uw);
#endif
		lcnt = fPtr->uh - srcy;
		for (i = 0; i < height; i++) {
			dptr = (uint32_t *)fPtr->buffers[bufnum];
			for (k = 0; k < rep; k++) {
				sptr = (uint32_t *)aptr;
				for (j = 0; j < fPtr->uw; j++) {
					pixel = *sptr;
					*dptr = (pixel << 8) | (pixel >> 24);
					dptr++;
					sptr++;
				}
			}			
			xoff = 0;
			MAKE_ROOM(1);
			WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
			while (xoff < width) {
				xa = dstx + xoff;
				xe = dstx + min(xoff + period, width) - 1;
				MAKE_ROOM(2);
				WRITE4(CRIME_DE_X_VERTEX_0,
				    xa << 16 | (dsty + i));
				WRITE4ST(CRIME_DE_X_VERTEX_1,
					(xe << 16) | (dsty + i));
				xoff += period;
			}
			bufnum++;
			if (bufnum == 8) bufnum = 0;
			lcnt--;
			if (lcnt == 0) {
				aptr = lptr;
				lcnt = fPtr->uh;
			} else
				aptr += fPtr->us;
		}
	}	
	DONE(CRIME_DEBUG_XRENDER);
}

static Bool
CrimeSetupForCPUToScreenTextureMask(
   ScrnInfoPtr	pScrn,
   int		op,
   int		texType,
   CARD8	*srcPtr,
   int		srcPitch,
   CARD8	*mskPtr,
   int		mskPitch,
   int		width,
   int		height,
   int		flags
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	if (op != PictOpOver) {
		xf86Msg(X_ERROR, "%s: op %d\n", __func__, op);
		op = PictOpOver;
	}

	LOG(CRIME_DEBUG_XRENDER);

	fPtr->uw = width;
	fPtr->uh = height;
	fPtr->us = srcPitch >> 2;
	if (PICT_FORMAT_BPP(texType) == 32) {
		fPtr->um = mskPitch >> 2;
	} else
		fPtr->um = mskPitch;
	fPtr->msk = (uint8_t *)mskPtr;
	fPtr->src = (uint8_t *)srcPtr;
	fPtr->texture_depth = PICT_FORMAT_BPP(texType);

	MAKE_ROOM(6);
	/* always expect ARGB for now */
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	fPtr->format = texType;
	WRITE4(CRIME_DE_ALPHA_FUNC, 
	    DE_ALPHA_ADD |
	    (DE_ALPHA_OP_SRC_ALPHA << DE_ALPHA_OP_SRC_SHIFT) |
	    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
	WRITE4(CRIME_DE_XFER_STEP_X, 4);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	SYNC;
	DONE(CRIME_DEBUG_XRENDER);
	return TRUE;
}

void
CrimeSubsequentCPUToScreenTextureMask32(
    ScrnInfoPtr	pScrn,
    int		dstx,
    int		dsty,
    int		srcx,
    int		srcy,
    int		maskx,
    int		masky,
    int		width,
    int		height
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t *lsptr, *lmptr, *asptr, *amptr;
	uint32_t *dptr, *sptr, *mptr, aval, pixel, mask;
	int i, j, k, l, rep = 1, period = fPtr->uw, xoff, lcnt, xa, xe;
	int bufnum = 0;
	int sr, sg, sb, sa, mr, mg, mb, ma, rr, gg, bb, aa;

	LOG(CRIME_DEBUG_XRENDER);
#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%s: %d %d %d %d %d %d; %d %d %d\n", __func__,
	    srcx, srcy, dstx, dsty, width, height, maskx, masky, fPtr->um); 
#endif
	sptr = fPtr->src + (srcx << 2);
	mptr = fPtr->msk + (srcx << 2);
	lsptr = sptr + (fPtr->us * srcy);
	lmptr = mptr + (fPtr->um * srcy);
	if ((fPtr->uw < 128) && (fPtr->uw < width)) {
		rep = 128 / fPtr->uw;
		period = rep * fPtr->uw;
	}

#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "ARGB %08x %d\n", (uint32_t)asptr, fPtr->uw);
#endif
	lcnt = fPtr->uh - srcy;
	for (i = 0; i < height; i++) {
		dptr = (uint32_t *)fPtr->buffers[bufnum];
		for (k = 0; k < rep; k++) {
			asptr = lsptr;
			amptr = lmptr;
			for (j = 0; j < fPtr->uw; j++) {
				pixel = *asptr;
				mask = *amptr;
				if (mask == 0xffffffff) {
					*dptr = (pixel >> 24) | (pixel << 8);
				} else if (mask == 0x00000000) {
					*dptr = 0;
				} else {
					/* input is ARGB */
					sb = pixel & 0xff;
					sg = (pixel >> 8) & 0xff;
					sr = (pixel >> 16) & 0xff;
					sa = (pixel >> 24) & 0xff;
					mb = mask & 0xff;
					mg = (mask >> 8) & 0xff;
					mr = (mask >> 16) & 0xff;
					ma = (mask >> 24) & 0xff;

					/* and here we need an RGBA pixel */
					bb = (((sb * mb) + 0x80) & 0xff00);
					gg = (((sg * mg) + 0x80) & 0xff00) << 8;
					rr = (((sr * mr) + 0x80) & 0xff00) << 16;
					aa = (((sa * ma) + 0x80) & 0xff00) >> 8;
					/*
					 * actually we could let the HW do this stuff
					 */
					*dptr = aa | rr | gg | bb;
				}
				dptr++;
				asptr++;
				amptr++;
			}
		}			
		xoff = 0;
		MAKE_ROOM(1);
		WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
		while (xoff < width) {
			xa = dstx + xoff;
			xe = dstx + min(xoff + period, width) - 1;
			MAKE_ROOM(2);
			WRITE4(CRIME_DE_X_VERTEX_0,
			    xa << 16 | (dsty + i));
			WRITE4ST(CRIME_DE_X_VERTEX_1,
				(xe << 16) | (dsty + i));
			xoff += period;
		}
		bufnum++;
		if (bufnum == 8) bufnum = 0;
		lcnt--;
		if (lcnt == 0) {
			/* back to the beginning */
			lsptr = sptr;
			lmptr = mptr;
			lcnt = fPtr->uh;
		} else
			/* next line */
			lsptr += fPtr->us;
			lmptr += fPtr->um;
	}
	DONE(CRIME_DEBUG_XRENDER);
}

void
CrimeSubsequentCPUToScreenTextureMask8(
    ScrnInfoPtr	pScrn,
    int		dstx,
    int		dsty,
    int		srcx,
    int		srcy,
    int		maskx,
    int		masky,
    int		width,
    int		height
)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t *lsptr, *asptr;
	uint32_t *dptr, *sptr, aval, pixel, mask;
	uint8_t  *lmptr, *amptr, *mptr;
	int i, j, k, l, rep = 1, period = fPtr->uw, xoff, lcnt, xa, xe;
	int bufnum = 0;
	int sr, sg, sb, sa, rr, gg, bb, aa;

	LOG(CRIME_DEBUG_XRENDER);
#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%s: %d %d %d %d %d %d\n", __func__,
	    srcx, srcy, dstx, dsty, width, height); 
#endif
	sptr = (uint32_t *)fPtr->src + (fPtr->us * srcy) + (srcx << 2);
	mptr = (uint8_t *)fPtr->msk + (fPtr->um * srcy) + srcx;
	lsptr = sptr;
	lmptr = mptr;
	if ((fPtr->uw < 128) && (fPtr->uw < width)) {
		rep = 128 / fPtr->uw;
		period = rep * fPtr->uw;
	}

#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "ARGB %08x %d\n", (uint32_t)asptr, fPtr->uw);
#endif
	lcnt = fPtr->uh;
	for (i = 0; i < height; i++) {
		dptr = (uint32_t *)fPtr->buffers[bufnum];
		for (k = 0; k < rep; k++) {
			asptr = lsptr;
			amptr = lmptr;
			for (j = 0; j < fPtr->uw; j++) {
				pixel = *asptr;
				mask = *amptr;
				if (mask == 0xff) {
					*dptr = (pixel >> 24) | (pixel << 8);
				} else if (mask == 0x00) {
					*dptr = 0;
				} else {
					/* input is ARGB */
					sb = pixel & 0xff;
					sg = (pixel >> 8) & 0xff;
					sr = (pixel >> 16) & 0xff;
					sa = (pixel >> 24) & 0xff;

					/* and here we need an RGBA pixel */
					bb = (((sb * mask) + 0x80) & 0xff00);
					gg = (((sg * mask) + 0x80) & 0xff00)
					     << 8;
					rr = (((sr * mask) + 0x80) & 0xff00)
					     << 16;
					aa = (((sa * mask) + 0x80) & 0xff00)
					     >> 8;
					/*
					 * actually we could let the HW do this 
					 * stuff
					 */
					*dptr = aa | rr | gg | bb;
				}
				dptr++;
				asptr++;
				amptr++;
			}
		}			
		xoff = 0;
		MAKE_ROOM(1);
		WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
		while (xoff < width) {
			xa = dstx + xoff;
			xe = dstx + min(xoff + period, width) - 1;
			MAKE_ROOM(2);
			WRITE4(CRIME_DE_X_VERTEX_0,
			    xa << 16 | (dsty + i));
			WRITE4ST(CRIME_DE_X_VERTEX_1,
				(xe << 16) | (dsty + i));
			xoff += period;
		}
		bufnum++;
		if (bufnum == 8) bufnum = 0;
		lcnt--;
		if (lcnt == 0) {
			/* back to the beginning */
			lsptr = sptr;
			lmptr = mptr;
			lcnt = fPtr->uh;
		} else
			/* next line */
			lsptr += fPtr->us;
			lmptr += fPtr->um;
	}
	DONE(CRIME_DEBUG_XRENDER);
}

static void
CrimeDoCPUToScreenComposite(
   	CARD8      op,
        PicturePtr pSrc,
        PicturePtr pMask,
        PicturePtr pDst,
        INT16      xSrc,
        INT16      ySrc,
        INT16      xMask,
        INT16      yMask,
        INT16      xDst,
        INT16      yDst,
        CARD16     width,
        CARD16     height
)
{
	ScreenPtr pScreen = pDst->pDrawable->pScreen;
	XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCREEN(pScreen);
	RegionRec region;
	CARD32 *formats;
	int flags = 0;
	BoxPtr pbox;
	int nbox, w, h;

	LOG(CRIME_DEBUG_XRENDER);
	if (pSrc->transform || (pMask && pMask->transform)) {
		xf86Msg(X_ERROR, "%s: transform?!\n", __func__);
		return;
	}

	if (pDst->alphaMap || pSrc->alphaMap || (pMask && pMask->alphaMap)) {
		xf86Msg(X_ERROR, "%s: alpha-map?!\n", __func__);
		return;
	}

	xDst += pDst->pDrawable->x;
	yDst += pDst->pDrawable->y;
	xSrc += pSrc->pDrawable->x;
	ySrc += pSrc->pDrawable->y;

	if(pMask) {
		CARD16 red, green, blue, alpha;
		CARD32 pixel =
		   *((CARD32*)(((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr));
#ifdef CRIME_DEBUG_LOUD
		if(pMask->componentAlpha) {
			xf86Msg(X_ERROR, "%s: alpha component mask\n", 
			    __func__);
			xf86Msg(X_ERROR, "src: %d x %d\n",
			    pSrc->pDrawable->width,
			    pSrc->pDrawable->height);
		}
#endif 
		if ((pSrc->pDrawable->width == 1) &&
		    (pSrc->pDrawable->height == 1)) {

			if(!XAAGetRGBAFromPixel(pixel, &red, &green, &blue, 
			    &alpha, pSrc->format)) {
				xf86Msg(X_ERROR, "%s: can't read pixel\n", 
				    __func__);
				return;
			}
#ifdef CRIME_DEBUG_LOUD
			xf86Msg(X_ERROR, "RGBA: %d %d %d %d\n", red, green, 
			    blue, alpha);
#endif
			xMask += pMask->pDrawable->x;
			yMask += pMask->pDrawable->y;	

			/* pull out color expandable operations here */
			if(pMask->format == PICT_a1) {
				PixmapPtr pPix = (PixmapPtr)(pMask->pDrawable);
				int skipleft;

				if (op != PictOpOver) {
					xf86Msg(X_ERROR, "!over\n");
					return;
				}
				if (pMask->repeat) {
					xf86Msg(X_ERROR, "mono repeat\n");
					return;
				}
				if (!miComputeCompositeRegion (&region, pSrc, 
				    pMask, pDst, xSrc, ySrc, xMask, yMask, xDst,
				    yDst, width, height)) {
					return;
				}

				nbox = REGION_NUM_RECTS(&region);
				pbox = REGION_RECTS(&region);   

				if(!nbox) {
					return;	
				}

				XAAGetPixelFromRGBA(&pixel, red, green, blue, 0,
				    pDst->format);

				xMask -= xDst;
				yMask -= yDst;

				while(nbox--) {
					skipleft = pbox->x1 + xMask;

					(*infoRec->WriteBitmap)(infoRec->pScrn, 					    	    pbox->x1, pbox->y1,
					    pbox->x2 - pbox->x1, 
					    pbox->y2 - pbox->y1,
					    (unsigned char*) 
					        (pPix->devPrivate.ptr) +
						(pPix->devKind *
					        (pbox->y1 + yMask)) + 
						((skipleft >> 3) & ~3), 
					    pPix->devKind, skipleft & 31, pixel,
					    -1, GXcopy, ~0);
					pbox++;
				}

				/* WriteBitmap sets the Sync flag */		 
				REGION_UNINIT(pScreen, &region);
				DONE(CRIME_DEBUG_XRENDER);
				return;
			}
			if((pMask->format == PICT_a8) ||
			   (pMask->format == PICT_a8r8g8b8)) {

				w = pMask->pDrawable->width;
				h = pMask->pDrawable->height;

				if(pMask->repeat) {
					flags |= XAA_RENDER_REPEAT;
				}

				if (!miComputeCompositeRegion (&region, pSrc, pMask, 
				    pDst, xSrc, ySrc, xMask, yMask, xDst, yDst,
				    width, height)) {
					return;
				}

				nbox = REGION_NUM_RECTS(&region);
				pbox = REGION_RECTS(&region);   

				if(!nbox) {
					REGION_UNINIT(pScreen, &region);
					return;
				}

				CrimeSetupForCPUToScreenAlphaTexture( 
				    infoRec->pScrn, op, red, green, blue, alpha,
				    pMask->format,
				    ((PixmapPtr)(pMask->pDrawable))->devPrivate.ptr,
				    ((PixmapPtr)(pMask->pDrawable))->devKind, 
				    w, h, flags);

				xMask -= xDst;
				yMask -= yDst;

				if (pMask->format != PICT_a8) {
					while(nbox--) {
						CrimeSubsequentCPUToScreenAlphaTexture32(
						    infoRec->pScrn,
						    pbox->x1, pbox->y1, pbox->x1 + xMask,
						    pbox->y1 + yMask, pbox->x2 - pbox->x1,
						    pbox->y2 - pbox->y1);
						pbox++;
					}
				} else {
					while(nbox--) {
						CrimeSubsequentCPUToScreenAlphaTexture(
						    infoRec->pScrn,
						    pbox->x1, pbox->y1,
						    pbox->x1 + xMask,
						    pbox->y1 + yMask,
						    pbox->x2 - pbox->x1,
						    pbox->y2 - pbox->y1);
						pbox++;
					}
				}
				SET_SYNC_FLAG(infoRec);
				REGION_UNINIT(pScreen, &region);
				DONE(CRIME_DEBUG_XRENDER);
				return;
			} else {
				xf86Msg(X_ERROR, "unknown mask %x\n", 
				    pMask->format);
			}
			REGION_UNINIT(pScreen, &region);
			DONE(CRIME_DEBUG_XRENDER);
			return;
		} else {
			/* source isn't solid */

			w = pSrc->pDrawable->width;
			h = pSrc->pDrawable->height;

			if(pSrc->repeat)
				flags |= XAA_RENDER_REPEAT;

			if (!miComputeCompositeRegion (&region, pSrc, pMask, 
			    pDst, xSrc, ySrc, xMask, yMask, xDst, yDst,
			    width, height)) {
				return;
			}

			nbox = REGION_NUM_RECTS(&region);
			pbox = REGION_RECTS(&region);   

			if(!nbox) {
				REGION_UNINIT(pScreen, &region);
				return;
			}
			CrimeSetupForCPUToScreenTextureMask(
			    infoRec->pScrn, op, pMask->format,
			    ((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr,
			    ((PixmapPtr)(pSrc->pDrawable))->devKind,
			    ((PixmapPtr)(pMask->pDrawable))->devPrivate.ptr,
			    ((PixmapPtr)(pMask->pDrawable))->devKind,
			    w, h, flags);

			xSrc -= xDst;
			ySrc -= yDst;

			if (PICT_FORMAT_BPP(pMask->format) == 32) {
				while(nbox--) {
					CrimeSubsequentCPUToScreenTextureMask32(
					    infoRec->pScrn,
					    pbox->x1, pbox->y1,
					    pbox->x1 + xSrc, pbox->y1 + ySrc,
					    xMask, yMask,
					    pbox->x2 - pbox->x1,
					    pbox->y2 - pbox->y1);
					pbox++;
				}
			} else {
				while(nbox--) {
					CrimeSubsequentCPUToScreenTextureMask8(
					    infoRec->pScrn,
					    pbox->x1, pbox->y1, pbox->x1 + xSrc,
					    pbox->y1 + ySrc,
					    xMask, yMask,
					    pbox->x2 - pbox->x1,
					    pbox->y2 - pbox->y1);
					pbox++;
				}
			}

			SET_SYNC_FLAG(infoRec);
			REGION_UNINIT(pScreen, &region);
			DONE(CRIME_DEBUG_XRENDER);
			return;
		}		
	} else {	/* no mask */
		formats = infoRec->CPUToScreenTextureFormats;

		w = pSrc->pDrawable->width;
		h = pSrc->pDrawable->height;

		if(pSrc->repeat)
			flags |= XAA_RENDER_REPEAT;

		while(*formats != pSrc->format) {
			if(!(*formats)) {
				xf86Msg(X_ERROR,
				    "%s: format %x not found\n",
				    __func__, pSrc->format);
				return;
			}
			formats++;
		}

		if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
                                   xSrc, ySrc, xMask, yMask, xDst, yDst,
                                   width, height)) {
			return;
		}

		nbox = REGION_NUM_RECTS(&region);
		pbox = REGION_RECTS(&region);   

		if(!nbox) {
			REGION_UNINIT(pScreen, &region);
			return;
		}

		CrimeSetupForCPUToScreenTexture(infoRec->pScrn,
			op, pSrc->format, 
			((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr,
			((PixmapPtr)(pSrc->pDrawable))->devKind, 
			w, h, flags);

		xSrc -= xDst;
		ySrc -= yDst;

		while(nbox--) {
			CrimeSubsequentCPUToScreenTexture(infoRec->pScrn,
			    pbox->x1, pbox->y1, pbox->x1 + xSrc,
			    pbox->y1 + ySrc, pbox->x2 - pbox->x1,
			    pbox->y2 - pbox->y1);
			pbox++;
		}

		SET_SYNC_FLAG(infoRec);
		REGION_UNINIT(pScreen, &region);
		DONE(CRIME_DEBUG_XRENDER);
		return;
	}
	xf86Msg(X_ERROR, "%s: shouldn't be here\n", __func__);
}

static void
CrimeDoScreenToScreenComposite(
   	CARD8      op,
        PicturePtr pSrc,
        PicturePtr pMask,
        PicturePtr pDst,
        INT16      xSrc,
        INT16      ySrc,
        INT16      xMask,
        INT16      yMask,
        INT16      xDst,
        INT16      yDst,
        CARD16     width,
        CARD16     height
)
{
	ScreenPtr pScreen = pDst->pDrawable->pScreen;
	XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCREEN(pScreen);
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	CrimePtr fPtr = CRIMEPTR(pScrn);
	RegionRec region;
	CARD32 *formats;
	int flags = 0;
	BoxPtr pbox;
	int nbox;
	int xs, ys, xd, yd, w, h;
xf86Msg(X_ERROR, "CRIME screen-to-screen composite\n");
	LOG(CRIME_DEBUG_XRENDER);
	if (pSrc->transform || (pMask && pMask->transform)) {
		xf86Msg(X_ERROR, "%s: mask?!\n", __func__);
		return;
	}
	xf86Msg(X_ERROR, "%s: %d\n", __func__, op);
	if (pDst->alphaMap || pSrc->alphaMap || (pMask && pMask->alphaMap)) {
		xf86Msg(X_ERROR, "%s: alpha-map\n", __func__);
		return;
	}

	xDst += pDst->pDrawable->x;
	yDst += pDst->pDrawable->y;
	xSrc += pSrc->pDrawable->x;
	ySrc += pSrc->pDrawable->y;

	formats = infoRec->CPUToScreenTextureFormats;

	w = pSrc->pDrawable->width;
	h = pSrc->pDrawable->height;

	if(pSrc->repeat)
		flags |= XAA_RENDER_REPEAT;

	while(*formats != pSrc->format) {
		if(!(*formats)) return;
		formats++;
	}

	if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
	    xSrc, ySrc, xMask, yMask, xDst, yDst, width, height))
		return;

	nbox = REGION_NUM_RECTS(&region);
	pbox = REGION_RECTS(&region);   

	if(!nbox) {
		REGION_UNINIT(pScreen, &region);
		return;
	}

	MAKE_ROOM(6);
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_XFER_STEP_X, 1);
	WRITE4(CRIME_DE_ALPHA_FUNC, 
	    DE_ALPHA_ADD |
	    (DE_ALPHA_OP_SRC_ALPHA << DE_ALPHA_OP_SRC_SHIFT) |
	    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	SYNC;
	xSrc -= xDst;
	ySrc -= yDst;

	/* assume no overlap - might bite us in the arse at some point */
	while(nbox--) {
		xs = pbox->x1 + xSrc;
		ys = pbox->y1 + ySrc;
		xd = pbox->x1;
		yd = pbox->y1;
		w = pbox->x2 - pbox->x1;
		h = pbox->y2 - pbox->y1;
		MAKE_ROOM(3);
		WRITE4(CRIME_DE_XFER_ADDR_SRC,(xs << 16) | (ys & 0xffff));
		WRITE4(CRIME_DE_X_VERTEX_0, (xd << 16) | (yd & 0xffff));
		WRITE4ST(CRIME_DE_X_VERTEX_1,
		    ((xd + w - 1) << 16) | ((yd + h - 1) & 0xffff));
		pbox++;
	}

	SET_SYNC_FLAG(infoRec);
	REGION_UNINIT(pScreen, &region);
	return;
}

static Bool
CrimeComposite(
   	CARD8      op,
        PicturePtr pSrc,
        PicturePtr pMask,
        PicturePtr pDst,                
        INT16      xSrc,
        INT16      ySrc,
        INT16      xMask,
        INT16      yMask,
        INT16      xDst,
        INT16      yDst,
        CARD16     width,
        CARD16     height
)
{
	ScreenPtr pScreen = pDst->pDrawable->pScreen;
	XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_SCREEN(pScreen);

	LOG(CRIME_DEBUG_XRENDER);

	if(!REGION_NUM_RECTS(pDst->pCompositeClip)) {
#ifdef CRIME_DEBUG_LOUD
		xf86Msg(X_ERROR, "%s: empty clip\n", __func__);
#endif
		return TRUE;
	}

	if(!infoRec->pScrn->vtSema) {
#ifdef CRIME_DEBUG_LOUD
		xf86Msg(X_ERROR, "%s: semaphore\n", __func__);
#endif
		return FALSE;
	}

	if((pDst->pDrawable->type == DRAWABLE_WINDOW) ||
	    IS_OFFSCREEN_PIXMAP(pDst->pDrawable) ||
	    PIXMAP_IS_SCREEN(pDst->pDrawable)) {
		if ((pSrc->pDrawable->type == DRAWABLE_WINDOW) ||
		    IS_OFFSCREEN_PIXMAP(pSrc->pDrawable) ||
		    PIXMAP_IS_SCREEN(pSrc->pDrawable)) {
			/* screen-to-screen */
			CrimeDoScreenToScreenComposite(op, pSrc, pMask, pDst,
			    xSrc, ySrc, xMask, yMask,
			    xDst, yDst, width, height);
			return TRUE;
		} else {
			/* CPU-to-screen composite */
			if (op != PictOpOver)
				xf86Msg(X_ERROR, "%s: op %d\n", __func__, op);
			CrimeDoCPUToScreenComposite(op, pSrc, pMask, pDst,
			    xSrc, ySrc, xMask, yMask,
			    xDst, yDst, width, height);
			return TRUE;
		}
	} else {
		if ((pSrc->pDrawable->type == DRAWABLE_WINDOW) ||
		    IS_OFFSCREEN_PIXMAP(pSrc->pDrawable) ||
		    PIXMAP_IS_SCREEN(pSrc->pDrawable)) {
			/* 
			 * screen-to-RAM
			 * Download from screen, then composite
			 */
			xf86Msg(X_ERROR, "%s: screen-to-RAM composite\n", 
			   __func__);
			return TRUE;
		} else {
			/* RAM-to-RAM */
#ifdef CRIME_DEBUG_LOUD
			xf86Msg(X_ERROR, "%s: fallback %d, %08x %08x %08x\n", 
			    __func__, op, (uint32_t)pSrc->format, 
			    (uint32_t)pDst->format,
			    pMask == NULL ? 0 : (uint32_t)pMask->format);
#endif
#if 0
			return FALSE;
#else
			fbComposite(op, pSrc, pMask, pDst, xSrc, ySrc, 
			    xMask, yMask, xDst, yDst, width, height);
			return TRUE;
#endif
		}
	}
	xf86Msg(X_ERROR, "composite fucked\n");
}

static Bool
CrimeGlyphs (CARD8	op,
	  PicturePtr	pSrc,
	  PicturePtr	pDst,
	  PictFormatPtr	maskFormat,
	  INT16		xSrc,
	  INT16		ySrc,
	  int		nlist,
	  GlyphListPtr	list,
	  GlyphPtr	*glyphs)
{
	ScreenPtr	pScreen = pDst->pDrawable->pScreen;
	PicturePtr	pPicture;
	GlyphPtr	glyph;
	int		xDst = list->xOff, yDst = list->yOff;
	int		x = 0, y = 0, i, n;

	while (nlist--)     {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		while (n--) {
			glyph = *glyphs++;
			pPicture = GlyphPicture (glyph)[pScreen->myNum];

			CrimeComposite (op,
				  pSrc,
				  pPicture,
				  pDst,
				  xSrc + (x - glyph->info.x) - xDst,
				  ySrc + (y - glyph->info.y) - yDst,
				  0, 0,
				  x - glyph->info.x,
				  y - glyph->info.y,
				  glyph->info.width,
				  glyph->info.height);

			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
		list++;
	}
	return TRUE;
}

static void
CrimePolyPoint(
    DrawablePtr pDraw,
    GCPtr pGC,
    int mode,
    int npt,
    xPoint *pptInit 
)
{
	ScreenPtr pScreen = pDraw->pScreen;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	CrimePtr fPtr = CRIMEPTR(pScrn);
	BoxPtr pBox;
	xPoint *ppt, *pts;
	int x1, x2, y1, y2, x, y, i, nBox;

	/* make pointlist origin relative */
	ppt = pptInit;
	if (mode == CoordModePrevious) {
		for (i = 0; i < npt; i++) {
			ppt++;
			ppt->x += (ppt-1)->x;
			ppt->y += (ppt-1)->y;
		}
	}
	MAKE_ROOM(6);
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_FG, pGC->fgPixel << 8);
	WRITE4(CRIME_DE_ROP, pGC->alu);
	WRITE4(CRIME_DE_PLANEMASK, pGC->planemask);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP);
	WRITE4(CRIME_DE_PRIMITIVE, DE_PRIM_POINT);
	SYNC;
	for (nBox = REGION_NUM_RECTS (pGC->pCompositeClip),
	    pBox = REGION_RECTS (pGC->pCompositeClip);
	    nBox--; pBox++) {

		pts = pptInit;
		for (i = 0; i < npt; i++) {
			x1 = pBox->x1;
			y1 = pBox->y1;
			x2 = pBox->x2;
			y2 = pBox->y2;
			x = pts->x + pDraw->x;
			y = pts->y + pDraw->y;
			if (x1 <= x && x < x2 && y1 <= y && y < y2) {

		 		MAKE_ROOM(1);
				WRITE4ST(CRIME_DE_X_VERTEX_0,
					(x << 16) | y);
			}
			pts++;
		}
	}
}

static void
CrimeValidatePolyPoint(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw )
{

	if ((pDraw->type == DRAWABLE_WINDOW) ||
	    IS_OFFSCREEN_PIXMAP(pDraw)) {
		pGC->ops->PolyPoint = CrimePolyPoint;
	} else
		xf86Msg(X_ERROR, "boo\n");	
}
static void
CrimePolyArc(DrawablePtr pDraw,
               GCPtr pGC,
               int narcs,
               xArc *parcs)
{
    xArc *arc;
    BoxRec box;
    int i, x2, y2;
    RegionPtr cclip;

    cclip = pGC->pCompositeClip;

    if(!REGION_NUM_RECTS(cclip))
	return;

    for (arc = parcs, i = narcs; --i >= 0; arc++) {
	if (miCanZeroArc(arc)) {
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
 	    x2 = box.x1 + (int)arc->width + 1;
 	    box.x2 = x2;
 	    y2 = box.y1 + (int)arc->height + 1;
 	    box.y2 = y2;
 	    if ( (x2 <= SHRT_MAX) && (y2 <= SHRT_MAX) &&
 		    (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN) )
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}

static void
CrimeValidatePolyArc(GCPtr pGC,
                       unsigned long changes,
                       DrawablePtr pDraw)
{
	if ((pDraw->type == DRAWABLE_WINDOW) ||
	    IS_OFFSCREEN_PIXMAP(pDraw)) {
		pGC->ops->PolyPoint = CrimePolyPoint;
		/*pGC->ops->PolyArc = miPolyArc;*/
		pGC->ops->PolyArc = CrimePolyArc;
	} else
	{
		pGC->ops->PolyPoint = XAAGetFallbackOps()->PolyPoint;
		pGC->ops->PolyArc = XAAGetFallbackOps()->PolyArc;
	}
}

static void copyRGBAtoARGB(uint32_t *dest, uint32_t *src, int len)
{
	while (len > 0) {
		*dest = *src >> 8;
		dest++;
		src++;
		len--;
	}
}

static void 
CrimeReadPixmap(ScrnInfoPtr pScrn, 
                     int x, 
		     int y, 
		     int w, 
		     int h,
		     unsigned char *dst, 
		     int dstwidth, 
		     int bpp, 
		     int depth)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	int bufno = 0;
	int nextbuf, i, len = w << 2;
	int mx = x << 2, offset;

	offset = mx & 0x3f;
	mx &= ~0x3f;
	len = (len + offset + 0x3f) & ~0x3f;

	LOG(CRIME_DEBUG_IMAGEREAD);

#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%s: %d %d %d %d\n", __func__, x, y, w, h);
#endif

	MAKE_ROOM(3);

	/*
	 * apparently all MTE coordinates are in bytes, not pixels
	 * also, the MTE has some crazy alignment requirements - if
	 * we don't do as above the thing will deadlock sooner or later
	 * We use the MTE here because I couldn't get the rendering engine
	 * to actually transfer anything into a linear buffer. The other
	 * way around works just fine though. Shouldn't make much of a
	 * difference, transfer times should be dominated by copying
	 * data in and out of the DMA buffer anyway
	 */
	WRITE4(CRIME_MTE_MODE, (MTE_TLB_LIN_A << MTE_DST_TLB_SHIFT) |
			 (MTE_TLB_A << MTE_SRC_TLB_SHIFT) |
			 (MTE_DEPTH_8 << MTE_DEPTH_SHIFT) |
			 MTE_MODE_DST_ECC | MTE_MODE_COPY);
	WRITE4(CRIME_MTE_SRC_Y_STEP, 1);
	WRITE4(CRIME_MTE_DST_Y_STEP, 1);
	SYNCMTE;
	WRITE4(CRIME_MTE_SRC0, (mx << 16) | y);
	WRITE4(CRIME_MTE_SRC1, ((mx + len) << 16) | y);
	WRITE4(CRIME_MTE_DST0, (bufno << 13));
	WRITE4ST(CRIME_MTE_DST1, (bufno << 13) + len);
	for (i = y + 1; i < y + h; i++) {
		nextbuf = (bufno + 1) & 7;
		SYNCMTE;
		WRITE4(CRIME_MTE_SRC0, (mx << 16) | i);
		WRITE4(CRIME_MTE_SRC1, ((mx + len) << 16) | i);
		WRITE4(CRIME_MTE_DST0, (nextbuf << 13));
		WRITE4ST(CRIME_MTE_DST1, (nextbuf << 13) + len);
		copyRGBAtoARGB((uint32_t *)dst,
			       (uint32_t *)(fPtr->buffers[bufno] + offset), w);
		dst += dstwidth;
		bufno = nextbuf;
	}
	SYNCMTE;
	copyRGBAtoARGB((uint32_t *)dst,
		       (uint32_t *)(fPtr->buffers[bufno] + offset), w);
	DONE(CRIME_DEBUG_IMAGEREAD);

}

int
CrimeAccelInit(ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	XAAInfoRecPtr pXAAInfo = fPtr->pXAA;
	int i;

	for (i = 0; i < 0x1000; i++) regcache[i] = 0x12345678;
	LOG(CRIME_DEBUG_ALL);
	SYNC;
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);

	WRITE4(CRIME_DE_XFER_STEP_Y, 1);
	WRITE4(CRIME_DE_XFER_STRD_DST, 4);
	WRITE4(CRIME_DE_XFER_STRD_SRC, 1);

	WRITE4(CRIME_MTE_BYTEMASK, 0xffffffff);
	WRITE4(CRIME_MTE_SRC_Y_STEP, 4);
	WRITE4(CRIME_MTE_DST_Y_STEP, 4);
	SYNC;

	/* blit the screen black */
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP);
	WRITE4(CRIME_DE_ROP, 3);
	WRITE4(CRIME_DE_PRIMITIVE, DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	WRITE4(CRIME_DE_FG, 0);
	WRITE4(CRIME_DE_X_VERTEX_0, 0);
	WRITE4ST(CRIME_DE_X_VERTEX_1,
	    fPtr->info.width << 16 | fPtr->info.height);
	SYNC;
	
	pXAAInfo->Flags = /*LINEAR_FRAMEBUFFER |*/ PIXMAP_CACHE | OFFSCREEN_PIXMAPS;
	pXAAInfo->maxOffPixWidth = fPtr->info.width;
	pXAAInfo->maxOffPixHeight = 2048;
	
	/* Sync */
	pXAAInfo->Sync = CrimeSync;

	CrimeDisableClipping(pScrn);

	/* Screen-to-screen copy */
	pXAAInfo->ScreenToScreenCopyFlags = NO_TRANSPARENCY;
	pXAAInfo->SetupForScreenToScreenCopy = CrimeSetupForScreenToScreenCopy;
	pXAAInfo->SubsequentScreenToScreenCopy =
		CrimeSubsequentScreenToScreenCopy;

	/* rectangle fills */
	pXAAInfo->SetupForSolidFill = CrimeSetupForSolidFill;
	pXAAInfo->SubsequentSolidFillRect = CrimeSubsequentSolidFillRect;
	
	/* image writes */
	pXAAInfo->ScanlineImageWriteFlags =
	    NO_TRANSPARENCY | LEFT_EDGE_CLIPPING |
	    LEFT_EDGE_CLIPPING_NEGATIVE_X |
	    CPU_TRANSFER_PAD_DWORD | SCANLINE_PAD_DWORD;
	pXAAInfo->NumScanlineImageWriteBuffers = 8;
	for (i = 0; i < 8; i++)
		fPtr->buffers[i] = fPtr->linear + (i * 8192);
	pXAAInfo->ScanlineImageWriteBuffers = fPtr->buffers;
	pXAAInfo->SetupForScanlineImageWrite = 
		CrimeSetupForScanlineImageWrite;
	pXAAInfo->SubsequentScanlineImageWriteRect =
		CrimeSubsequentImageWriteRect;
	pXAAInfo->SubsequentImageWriteScanline =
		CrimeSubsequentImageWriteScanline;

	/* read pixmap */
	pXAAInfo->ReadPixmapFlags = 0
	                             | CPU_TRANSFER_PAD_DWORD
				     ;
	pXAAInfo->ReadPixmap = CrimeReadPixmap;

	/* colour expansion */
	pXAAInfo->ScanlineCPUToScreenColorExpandFillFlags = 
		LEFT_EDGE_CLIPPING;
	pXAAInfo->NumScanlineColorExpandBuffers = 1;
	fPtr->expandbuffers[0] = (uint8_t *)fPtr->expand;
	pXAAInfo->ScanlineColorExpandBuffers = (void *)&fPtr->expandbuffers;
	pXAAInfo->SetupForScanlineCPUToScreenColorExpandFill = 
		CrimeSetupForCPUToScreenColorExpandFill;
	pXAAInfo->SubsequentScanlineCPUToScreenColorExpandFill =
		CrimeSubsequentScanlineCPUToScreenColorExpandFill;
	pXAAInfo->SubsequentColorExpandScanline =
		CrimeSubsequentColorExpandScanline;

	/* clipping */
	pXAAInfo->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY |
		HARDWARE_CLIP_SOLID_FILL | 
		HARDWARE_CLIP_SOLID_LINE |
		HARDWARE_CLIP_MONO_8x8_FILL | HARDWARE_CLIP_DASHED_LINE;
	pXAAInfo->SetClippingRectangle = CrimeSetClippingRectangle;
	pXAAInfo->DisableClipping = CrimeDisableClipping;

	/* solid line drawing */
	pXAAInfo->SetupForSolidLine = CrimeSetupForSolidLine;
	pXAAInfo->SubsequentSolidTwoPointLine = 
	    CrimeSubsequentSolidTwoPointLine;
	pXAAInfo->SolidLineFlags = 0;

	/* dashed line drawing */
	pXAAInfo->SetupForDashedLine = CrimeSetupForDashedLine;
	pXAAInfo->SubsequentDashedTwoPointLine = 
	    CrimeSubsequentDashedTwoPointLine;
	pXAAInfo->DashedLineFlags = LINE_PATTERN_MSBFIRST_MSBJUSTIFIED;
	pXAAInfo->DashPatternMaxLength = 32;

	/* mono pattern fills */
	pXAAInfo->Mono8x8PatternFillFlags = HARDWARE_PATTERN_PROGRAMMED_BITS |
	    HARDWARE_PATTERN_PROGRAMMED_ORIGIN | BIT_ORDER_IN_BYTE_MSBFIRST;
	pXAAInfo->SetupForMono8x8PatternFill = CrimeSetupForMono8x8PatternFill;
	pXAAInfo->SubsequentMono8x8PatternFillRect =
	    CrimeSubsequentMono8x8PatternFillRect;

	/* XRender acceleration */
#ifdef RENDER
	pXAAInfo->CPUToScreenAlphaTextureFlags = 0;
	pXAAInfo->SetupForCPUToScreenAlphaTexture =
	    CrimeSetupForCPUToScreenAlphaTexture;
	pXAAInfo->SubsequentCPUToScreenAlphaTexture =
	    CrimeSubsequentCPUToScreenAlphaTexture;
	pXAAInfo->CPUToScreenAlphaTextureFormats = CrimeAlphaTextureFormats;

	pXAAInfo->SetupForCPUToScreenTexture = CrimeSetupForCPUToScreenTexture;
	pXAAInfo->SubsequentCPUToScreenTexture = 
	    CrimeSubsequentCPUToScreenTexture;
	pXAAInfo->CPUToScreenTextureFlags = 0;
	pXAAInfo->CPUToScreenTextureFormats = CrimeTextureFormats;
	pXAAInfo->Composite = CrimeComposite;
	pXAAInfo->Glyphs = CrimeGlyphs;
#endif
	pXAAInfo->ValidatePolyPoint = CrimeValidatePolyPoint;
	pXAAInfo->PolyPointMask = GCFunction;
	pXAAInfo->ValidatePolyArc = CrimeValidatePolyArc;
	pXAAInfo->PolyArcMask = GCFunction | GCLineWidth;

	return -1;
}
