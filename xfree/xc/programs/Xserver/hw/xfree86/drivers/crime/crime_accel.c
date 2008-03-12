/* $NetBSD: crime_accel.c,v 1.2 2008/03/12 20:00:07 macallan Exp $ */
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

#define CRIMEREG(p) (volatile uint32_t *)(fPtr->engine + p)
/*#define WBFLUSH { volatile uint32_t boo = *CRIMEREG(0x4000); }*/
#define WBFLUSH __asm__ ("nop; sync;")
#define WRITE4(r, v) {*CRIMEREG(r) = v;}
#define WRITE4ST(r, v) {*CRIMEREG(r + CRIME_DE_START) = v; WBFLUSH;}
#define SYNC do {} while((*CRIMEREG(0x4000) & CRIME_DE_IDLE) == 0)

CARD32 CrimeAlphaTextureFormats[] = {PICT_a8, 0};
CARD32 CrimeTextureFormats[] = {PICT_a8b8g8r8, PICT_a8r8g8b8, 0};

void
CrimeSync(ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
#ifdef CRIME_DEBUG_LOUD
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

	SYNC;
	LOG(CRIME_DEBUG_BITBLT);
	WRITE4(CRIME_DE_XFER_STEP_X, 1);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
	fPtr->xdir = xdir;
	fPtr->ydir = ydir;
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
	int rxa, rya, rxe, rye, rxs, rys;

	LOG(CRIME_DEBUG_BITBLT);
	SYNC;
#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%s: %d, %d; %d x %d -> %d %d\n", __func__,
	    xSrc, ySrc, w, h, xDst, yDst);
#endif
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
	WRITE4(CRIME_DE_PRIMITIVE, prim);
	WRITE4(CRIME_DE_XFER_ADDR_SRC,(rxs << 16) | (rys & 0xffff));
	WRITE4(CRIME_DE_X_VERTEX_0, (rxa << 16) | (rya & 0xffff));
	WBFLUSH;
	WRITE4ST(CRIME_DE_X_VERTEX_1, (rxe << 16) | (rye & 0xffff));
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

	SYNC;
	LOG(CRIME_DEBUG_RECTFILL);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, htole32(colour));
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);
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

	LOG(CRIME_DEBUG_RECTFILL);
	SYNC;
	WRITE4(CRIME_DE_X_VERTEX_0, (x << 16) | (y & 0xffff));
	WBFLUSH;
	WRITE4ST(CRIME_DE_X_VERTEX_1,
	    ((x + w - 1) << 16) | ((y + h - 1) & 0xffff));
	DONE(CRIME_DEBUG_RECTFILL);
}

void
CrimeSetupForScanlineImageWrite(ScrnInfoPtr pScrn, int rop, 
                                unsigned int planemask, int trans_color, 
                                int bpp, int depth)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_IMAGEWRITE);
	SYNC;
#ifdef CRIME_DEBUG_LOUD
	xf86Msg(X_ERROR, "%s: %d %d \n", __func__, bpp, depth);
#endif
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_LIN_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_ABGR | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_XFER_STEP_X, 4);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	DONE(CRIME_DEBUG_IMAGEWRITE);
}

void
CrimeSubsequentImageWriteRect(ScrnInfoPtr pScrn, 
                                int x, int y, int w, int h, int skipleft)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_IMAGEWRITE);

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

void
CrimeSubsequentImageWriteScanline(ScrnInfoPtr pScrn, int bufno)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_IMAGEWRITE);
	SYNC;

	WRITE4(CRIME_DE_XFER_ADDR_SRC, bufno * 8192 + fPtr->start * 4);
	WRITE4(CRIME_DE_X_VERTEX_0, (fPtr->ux << 16) | fPtr->uy);
	WBFLUSH;
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
	SYNC;
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, htole32(fg));
	if (bg == -1) {
		/* transparent */
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_POLY_STIP);
	} else {
		WRITE4(CRIME_DE_BG, htole32(bg));
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | 
		    DE_DRAWMODE_OPAQUE_STIP | DE_DRAWMODE_POLY_STIP);
	}
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
	DONE(CRIME_DEBUG_COLOUREXPAND);
}

static void 
CrimeSubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
			int x, int y, int w, int h,
			int skipleft )
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_COLOUREXPAND);
	SYNC;

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
	SYNC;

	WRITE4(CRIME_DE_STIPPLE_MODE, 0x001f0000 | (fPtr->start << 24));
	WRITE4(CRIME_DE_STIPPLE_PAT, *boo);
	boo++;
	WRITE4(CRIME_DE_X_VERTEX_0, (x + fPtr->start << 16) | fPtr->uy);
	WBFLUSH;
	WRITE4ST(CRIME_DE_X_VERTEX_1,
		((x + min(idx, 32) - 1) << 16) | (fPtr->uy));
	idx -= 32;
	x += 32;
	WRITE4(CRIME_DE_STIPPLE_MODE, 0x001f0000);
	
	while (idx > 0) {
		WRITE4(CRIME_DE_STIPPLE_PAT, *boo);
		boo++;
		WRITE4(CRIME_DE_X_VERTEX_0, (x << 16) | fPtr->uy);
		WBFLUSH;
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

	SYNC;
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, htole32(color));
	WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP);
	DONE(CRIME_DEBUG_LINES);
}

static void
CrimeSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn, int x1, int y1, int x2,
    int y2, int flags)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_LINES);
	SYNC;
	if (flags & OMIT_LAST) {
		WRITE4(CRIME_DE_PRIMITIVE,
			DE_PRIM_LINE | DE_PRIM_LINE_SKIP_END | 2);
	} else {
		WRITE4(CRIME_DE_PRIMITIVE,
			DE_PRIM_LINE | 2);
	}
	WRITE4(CRIME_DE_X_VERTEX_0, (x1 << 16) | y1);
	WBFLUSH;
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
	SYNC;

	fPtr->uw = length;
	WRITE4(CRIME_DE_PLANEMASK, planemask);
	WRITE4(CRIME_DE_ROP, rop);
	WRITE4(CRIME_DE_FG, htole32(fg));
	if (bg == -1) {
		/* transparent */
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | DE_DRAWMODE_LINE_STIP);
	} else {
		WRITE4(CRIME_DE_BG, htole32(bg));
		WRITE4(CRIME_DE_DRAWMODE,
		    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK |
		    DE_DRAWMODE_ROP | 
		    DE_DRAWMODE_OPAQUE_STIP | DE_DRAWMODE_LINE_STIP);
	}
	/*
	 * can we trust the Xserver to always hand us a 32bit aligned 
	 * pattern buffer?
	 */
	memcpy(&pat, pattern, 4);
	WRITE4(CRIME_DE_STIPPLE_PAT, pat);
	DONE(CRIME_DEBUG_LINES);
}

void
CrimeSubsequentDashedTwoPointLine( ScrnInfoPtr pScrn,
        int x1, int y1, int x2, int y2, int flags, int phase)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	uint32_t stipmode;

	LOG(CRIME_DEBUG_LINES);
	SYNC;

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
	WBFLUSH;
	WRITE4ST(CRIME_DE_X_VERTEX_1, (x2 << 16) | y2);
	DONE(CRIME_DEBUG_LINES);
}

void
CrimeSetClippingRectangle ( ScrnInfoPtr pScrn,
                        int left, int top, int right, int bottom)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_CLIPPING);
	/* nothing so far */
	DONE(CRIME_DEBUG_CLIPPING);
}

void
CrimeDisableClipping (ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);

	LOG(CRIME_DEBUG_CLIPPING);
	SYNC;

	WRITE4(CRIME_DE_CLIPMODE, 0);
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
	SYNC;
	/* XXX this register is not where it's supposed to be */
	WRITE4(CRIME_DE_ALPHA_COLOR, fPtr->alpha_color);
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
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
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
		dptr = fPtr->buffers[bufnum];
		if (fPtr->alpha_color == 0) {
			memcpy(dptr, aptr, width);
		} else {
			for (j = 0; j < width; j++) {
				aval = aptr[j];
				*dptr = aval | fPtr->alpha_color;
				dptr++;
			}
		}
		SYNC;
		WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
		WRITE4(CRIME_DE_X_VERTEX_0, dstx << 16 | (dsty + i));
		WBFLUSH;
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
	SYNC;
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
	    (DE_ALPHA_OP_SRC_ALPHA << DE_ALPHA_OP_SRC_SHIFT) |
	    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);
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
	aptr = fPtr->alpha_texture + (fPtr->us * srcy) + (srcx << 2);
	lptr = aptr;
	if ((fPtr->uw < 128) && (fPtr->uw < width)) {
		rep = 128 / fPtr->uw;
		period = rep * fPtr->uw;
	}

	if (fPtr->format == PICT_a8b8g8r8) {
#ifdef CRIME_DEBUG_LOUD
		xf86Msg(X_ERROR, "ABGR\n");
#endif
		for (i = 0; i < height; i++) {
			dptr = fPtr->buffers[bufnum];
			memcpy(dptr, aptr, fPtr->us);
			SYNC;
			WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
			WRITE4(CRIME_DE_X_VERTEX_0, dstx << 16 | (dsty + i));
			WBFLUSH;
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
		lcnt = fPtr->uh;
		for (i = 0; i < height; i++) {
			dptr = fPtr->buffers[bufnum];
			for (k = 0; k < rep; k++) {
				sptr = (uint32_t)aptr;
				for (j = 0; j < fPtr->uw; j++) {
					pixel = *sptr;
					*dptr = (pixel << 8) | (pixel >> 24);
					dptr++;
					sptr++;
				}
			}			
			xoff = 0;
			SYNC;
			WRITE4(CRIME_DE_XFER_ADDR_SRC, bufnum * 8192);
			while (xoff < width) {
				xa = dstx + xoff;
				xe = dstx + min(xoff + period, width) - 1;
				SYNC;
				WRITE4(CRIME_DE_X_VERTEX_0,
				    xa << 16 | (dsty + i));
				WBFLUSH;
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
		xf86Msg(X_ERROR, "%s: mask?!\n", __func__);
		return;
	}

	if (pDst->alphaMap || pSrc->alphaMap || (pMask && pMask->alphaMap)) {
		xf86Msg(X_ERROR, "%s: alpha-map\n", __func__);
		return;
	}

	xDst += pDst->pDrawable->x;
	yDst += pDst->pDrawable->y;
	xSrc += pSrc->pDrawable->x;
	ySrc += pSrc->pDrawable->y;

	if(pMask) {
		if(pMask->componentAlpha) {
			xf86Msg(X_ERROR, "%s: alpha component mask\n", 
			    __func__);
	    		return;
		}
 
		/* for now we only do it if there is a 1x1 (solid) source */

		if((pSrc->pDrawable->width == 1) &&
		   (pSrc->pDrawable->height == 1)) {
			CARD16 red, green, blue, alpha;
			CARD32 pixel =
		*((CARD32*)(((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr));

			if(!XAAGetRGBAFromPixel(pixel, &red, &green, &blue, 
			    &alpha, pSrc->format)) {
				xf86Msg(X_ERROR, "%s: can't read pixel\n", 
				    __func__);
				return;
			}
			xMask += pMask->pDrawable->x;
			yMask += pMask->pDrawable->y;	

			/* pull out color expandable operations here */
			if((pMask->format == PICT_a1) && (alpha == 0xffff) &&
			    (op == PictOpOver) && !pMask->repeat) {
				PixmapPtr pPix = (PixmapPtr)(pMask->pDrawable);
				int skipleft;

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

					 (*infoRec->WriteBitmap)(infoRec->pScrn, 					    pbox->x1, pbox->y1,
					    pbox->x2 - pbox->x1, 
					    pbox->y2 - pbox->y1,
					(unsigned char*)(pPix->devPrivate.ptr) + 					    (pPix->devKind *
					    (pbox->y1 + yMask)) + 
					    ((skipleft >> 3) & ~3), 
					    pPix->devKind, 
					    skipleft & 31, pixel, -1, GXcopy, 
					    ~0);
					pbox++;
				}

				/* WriteBitmap sets the Sync flag */		 
				REGION_UNINIT(pScreen, &region);
				DONE(CRIME_DEBUG_XRENDER);
				return;
			}

			formats = infoRec->CPUToScreenAlphaTextureFormats;

			w = pMask->pDrawable->width;
			h = pMask->pDrawable->height;

			if(pMask->repeat) {
				flags |= XAA_RENDER_REPEAT;
			}

			while(*formats != pMask->format) {
				if(!(*formats)) {
					xf86Msg(X_ERROR,
					    "no alpha format %d\n", 
					    pMask->format);
					return;
				}
				formats++;
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

			CrimeSetupForCPUToScreenAlphaTexture(infoRec->pScrn,
			    op, red, green, blue, alpha, pMask->format, 
			    ((PixmapPtr)(pMask->pDrawable))->devPrivate.ptr,
			    ((PixmapPtr)(pMask->pDrawable))->devKind, 
			    w, h, flags);

			xMask -= xDst;
			yMask -= yDst;

			while(nbox--) {
				CrimeSubsequentCPUToScreenAlphaTexture(
				    infoRec->pScrn,
				    pbox->x1, pbox->y1, pbox->x1 + xMask,
				    pbox->y1 + yMask, pbox->x2 - pbox->x1,
				    pbox->y2 - pbox->y1);
				pbox++;
			}

			SET_SYNC_FLAG(infoRec);
			REGION_UNINIT(pScreen, &region);
			DONE(CRIME_DEBUG_XRENDER);
			return;
		} else {
			/* RGB & separate transparency info */
			if (pMask->format == PICT_a1) {
				xf86Msg(X_ERROR, "bitmask\n");
			} else if (pMask->format == PICT_a8) {
				xf86Msg(X_ERROR, "alpha mask\n");
			} else
				xf86Msg(X_ERROR, "unknown mask %d\n", 
				    pMask->format);
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

			/* XXX deal with the mask */
			CrimeSetupForCPUToScreenTexture(infoRec->pScrn,
				op, pSrc->format, 
				((PixmapPtr)(pSrc->pDrawable))->devPrivate.ptr,
				((PixmapPtr)(pSrc->pDrawable))->devKind, 
				w, h, flags);

			xSrc -= xDst;
			ySrc -= yDst;

			while(nbox--) {
				CrimeSubsequentCPUToScreenTexture(
				    infoRec->pScrn,
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
			
	} else {
		formats = infoRec->CPUToScreenTextureFormats;

		w = pSrc->pDrawable->width;
		h = pSrc->pDrawable->height;

		if(pSrc->repeat)
			flags |= XAA_RENDER_REPEAT;

		while(*formats != pSrc->format) {
			if(!(*formats)) {
				xf86Msg(X_ERROR,
				    "%s: format %d not found\n",
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

	LOG(CRIME_DEBUG_XRENDER);
	if (pSrc->transform || (pMask && pMask->transform)) {
		xf86Msg(X_ERROR, "%s: mask?!\n", __func__);
		return;
	}

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

	SYNC;
	WRITE4(CRIME_DE_MODE_SRC, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_ABGR | DE_MODE_PIXDEPTH_32);
	WRITE4(CRIME_DE_XFER_STEP_X, 1);
	WRITE4(CRIME_DE_ALPHA_FUNC, 
	    DE_ALPHA_ADD |
	    (DE_ALPHA_OP_ONE << DE_ALPHA_OP_SRC_SHIFT) |
	    (DE_ALPHA_OP_1_MINUS_SRC_ALPHA << DE_ALPHA_OP_DST_SHIFT));
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ALPHA_BLEND |
	    DE_DRAWMODE_XFER_EN);
	WRITE4(CRIME_DE_PRIMITIVE,
		DE_PRIM_RECTANGLE | DE_PRIM_LR | DE_PRIM_TB);

	xSrc -= xDst;
	ySrc -= yDst;

	/* assume no overlap - might bite us in the arse at some point */
	while(nbox--) {
			CrimeSubsequentCPUToScreenTexture(infoRec->pScrn,
			    pbox->x1, pbox->y1, pbox->x1 + xSrc,
			    pbox->y1 + ySrc, pbox->x2 - pbox->x1,
			    pbox->y2 - pbox->y1);
		xs = pbox->x1 + xSrc;
		ys = pbox->y1 + ySrc;
		xd = pbox->x1;
		yd = pbox->y1;
		w = pbox->x2 - pbox->x1;
		h = pbox->y2 - pbox->y1;
		SYNC;
		WRITE4(CRIME_DE_XFER_ADDR_SRC,(xs << 16) | (ys & 0xffff));
		WRITE4(CRIME_DE_X_VERTEX_0, (xd << 16) | (yd & 0xffff));
		WBFLUSH;
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

	if(!REGION_NUM_RECTS(pDst->pCompositeClip))
		return TRUE;

	if(!infoRec->pScrn->vtSema)
		return FALSE;

	if((pDst->pDrawable->type == DRAWABLE_WINDOW) ||
	    IS_OFFSCREEN_PIXMAP(pDst->pDrawable)) {
		if ((pSrc->pDrawable->type == DRAWABLE_WINDOW) ||
		    IS_OFFSCREEN_PIXMAP(pSrc->pDrawable)) {
			/* screen-to-screen */
			CrimeDoScreenToScreenComposite(op, pSrc, pMask, pDst,
			    xSrc, ySrc, xMask, yMask,
			    xDst, yDst, width, height);
			return TRUE;
		} else {
			/* CPU-to-screen composite */
			CrimeDoCPUToScreenComposite(op, pSrc, pMask, pDst,
			    xSrc, ySrc, xMask, yMask,
			    xDst, yDst, width, height);
			return TRUE;
		}
	} else {
		if ((pSrc->pDrawable->type == DRAWABLE_WINDOW) ||
		    IS_OFFSCREEN_PIXMAP(pSrc->pDrawable)) {
			/* screen-to-RAM */
			xf86Msg(X_ERROR, "%s: screen-to-RAM composite\n", 
			   __func__);
			return TRUE;
		} else {
			/* RAM-to-RAM */
			return FALSE;
		}
	}
	xf86Msg(X_ERROR, "composite fucked\n");
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
	SYNC;
	WRITE4(CRIME_DE_FG, pGC->fgPixel);
	WRITE4(CRIME_DE_ROP, pGC->alu);
	WRITE4(CRIME_DE_PLANEMASK, pGC->planemask);
	WRITE4(CRIME_DE_DRAWMODE,
	    DE_DRAWMODE_PLANEMASK | DE_DRAWMODE_BYTEMASK | DE_DRAWMODE_ROP);
	WRITE4(CRIME_DE_PRIMITIVE, DE_PRIM_POINT);
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

		 		SYNC;
				WBFLUSH;
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

int
CrimeAccelInit(ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	XAAInfoRecPtr pXAAInfo = fPtr->pXAA;
	int i;

	LOG(CRIME_DEBUG_ALL);
	SYNC;
	WRITE4(CRIME_DE_MODE_DST, DE_MODE_TLB_A | DE_MODE_BUFDEPTH_32 |
			    DE_MODE_TYPE_RGBA | DE_MODE_PIXDEPTH_32);

	WRITE4(CRIME_DE_XFER_STEP_Y, 1);
	WRITE4(CRIME_DE_XFER_STRD_DST, 0);
	WRITE4(CRIME_DE_XFER_STRD_SRC, 1);

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
	
	pXAAInfo->Flags = LINEAR_FRAMEBUFFER | PIXMAP_CACHE | OFFSCREEN_PIXMAPS;
	pXAAInfo->maxOffPixWidth = fPtr->info.width;
	pXAAInfo->maxOffPixHeight = 2048;
	
	/* Sync */
	pXAAInfo->Sync = CrimeSync;

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

	/* colour expansion */
	pXAAInfo->ScanlineCPUToScreenColorExpandFillFlags = 
		LEFT_EDGE_CLIPPING;
	pXAAInfo->NumScanlineColorExpandBuffers = 1;
	fPtr->expandbuffers[0] = fPtr->expand;
	pXAAInfo->ScanlineColorExpandBuffers = &fPtr->expandbuffers;
	pXAAInfo->SetupForScanlineCPUToScreenColorExpandFill = 
		CrimeSetupForCPUToScreenColorExpandFill;
	pXAAInfo->SubsequentScanlineCPUToScreenColorExpandFill =
		CrimeSubsequentScanlineCPUToScreenColorExpandFill;
	pXAAInfo->SubsequentColorExpandScanline =
		CrimeSubsequentColorExpandScanline;

	/* clipping */
	pXAAInfo->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY |
		HARDWARE_CLIP_SOLID_FILL | HARDWARE_CLIP_SOLID_LINE;
	pXAAInfo->SetClippingRectangle = CrimeSetClippingRectangle;
	pXAAInfo->DisableClipping = CrimeDisableClipping;

	/* solid line drawing */
	pXAAInfo->SetupForSolidLine = CrimeSetupForSolidLine;
	pXAAInfo->SubsequentSolidTwoPointLine = 
	    CrimeSubsequentSolidTwoPointLine;
	pXAAInfo->SolidLineFlags = BIT_ORDER_IN_BYTE_MSBFIRST;

	/* dashed line drawing */
	pXAAInfo->SetupForDashedLine = CrimeSetupForDashedLine;
	pXAAInfo->SubsequentDashedTwoPointLine = 
	    CrimeSubsequentDashedTwoPointLine;
	pXAAInfo->DashedLineFlags = LINE_PATTERN_MSBFIRST_MSBJUSTIFIED;
	pXAAInfo->DashPatternMaxLength = 32;

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
#endif
	pXAAInfo->ValidatePolyPoint = CrimeValidatePolyPoint;
	pXAAInfo->PolyPointMask = GCFunction;

	return -1;
}
