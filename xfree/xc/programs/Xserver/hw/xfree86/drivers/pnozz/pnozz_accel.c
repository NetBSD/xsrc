/*
 * SBus Weitek P9100 hardware cursor support
 *
 * Copyright (C) 2005 Michael Lorenz
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
/* $NetBSD: pnozz_accel.c,v 1.2 2005/07/07 12:23:21 macallan Exp $ */

#include "pnozz.h"

static CARD32 PnozzCopyROP[] = {
	/*GXclear*/		0,
	/*GXand*/		ROP_SRC & ROP_DST,
	/*GXandReverse*/	ROP_SRC & (~ROP_DST),
	/*GXcopy*/		ROP_SRC,
	/*GXandInverted*/	(~ROP_SRC) & ROP_DST,
	/*GXnoop*/		ROP_DST,
	/*GXxor*/		ROP_SRC ^ ROP_DST,
	/*GXor*/		ROP_SRC | ROP_DST,
	/*GXnor*/		(~ROP_SRC) & (~ROP_DST),
	/*GXequiv*/		(~ROP_SRC) ^ ROP_DST,
	/*GXinvert*/		(~ROP_DST),
	/*GXorReverse*/		ROP_SRC | (~ROP_DST),
	/*GXcopyInverted*/	(~ROP_SRC),
	/*GXorInverted*/	(~ROP_SRC) | ROP_DST,
	/*GXnand*/		(~ROP_SRC) | (~ROP_DST),
	/*GXset*/		ROP_SET
};

static CARD32 PnozzDrawROP[] = {
	/*GXclear*/		0,
	/*GXand*/		ROP_PAT & ROP_DST,
	/*GXandReverse*/	ROP_PAT & (~ROP_DST),
	/*GXcopy*/		ROP_PAT,
	/*GXandInverted*/	(~ROP_PAT) & ROP_DST,
	/*GXnoop*/		ROP_DST,
	/*GXxor*/		ROP_PAT ^ ROP_DST,
	/*GXor*/		ROP_PAT | ROP_DST,
	/*GXnor*/		(~ROP_PAT) & (~ROP_DST),
	/*GXequiv*/		(~ROP_PAT) ^ ROP_DST,
	/*GXinvert*/		(~ROP_DST),
	/*GXorReverse*/		ROP_PAT | (~ROP_DST),
	/*GXcopyInverted*/	(~ROP_PAT),
	/*GXorInverted*/	(~ROP_PAT) | ROP_DST,
	/*GXnand*/		(~ROP_PAT) | (~ROP_DST),
	/*GXset*/		ROP_PAT
};

CARD32 MaxClip, junk;

static void PnozzSync(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    while((pnozz_read_4(pPnozz, ENGINE_STATUS) & 
        (ENGINE_BUSY | BLITTER_BUSY)) !=0 );
}

static void unClip(PnozzPtr pPnozz)
{
	pnozz_write_4(pPnozz, WINDOW_OFFSET, 0);
	pnozz_write_4(pPnozz, WINDOW_MIN, 0);
	pnozz_write_4(pPnozz, WINDOW_MAX, MaxClip);
	pnozz_write_4(pPnozz, BYTE_CLIP_MIN, 0);
	pnozz_write_4(pPnozz, BYTE_CLIP_MAX, MaxClip);
}

static void
PnozzInitEngine(PnozzPtr pPnozz)
{
	unClip(pPnozz);
	pnozz_write_4(pPnozz, DRAW_MODE, 0);
	pnozz_write_4(pPnozz, PLANE_MASK, 0xffffffff);	
	pnozz_write_4(pPnozz, PATTERN0, 0xffffffff);	
	pnozz_write_4(pPnozz, PATTERN1, 0xffffffff);	
	pnozz_write_4(pPnozz, PATTERN2, 0xffffffff);	
	pnozz_write_4(pPnozz, PATTERN3, 0xffffffff);	
}

static void
PnozzSetupForScreenToScreenCopy(
    ScrnInfoPtr  pScrn,
    int          xdir,
    int          ydir,
    int          rop,
    unsigned int planemask,
    int          TransparencyColour
)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    PnozzSync(pScrn);
    unClip(pPnozz);
    pnozz_write_4(pPnozz, RASTER_OP, (PnozzCopyROP[rop] & 0xff));
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
}

static void
PnozzSubsequentScreenToScreenCopy
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
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    CARD32 src, dst, srcw, dstw;
    
    src = ((xSrc & 0xffff) << 16) | (ySrc & 0xffff);
    dst = ((xDst & 0xffff) << 16) | (yDst & 0xffff);
    srcw = (((xSrc + w - 1) & 0xffff) << 16) | ((ySrc + h - 1) & 0xffff);
    dstw = (((xDst + w - 1) & 0xffff) << 16) | ((yDst + h - 1) & 0xffff);

    PnozzSync(pScrn);

    pnozz_write_4(pPnozz, ABS_XY0, src);
    pnozz_write_4(pPnozz, ABS_XY1, srcw);
    pnozz_write_4(pPnozz, ABS_XY2, dst);
    pnozz_write_4(pPnozz, ABS_XY3, dstw);
    junk = pnozz_read_4(pPnozz, COMMAND_BLIT);
}

static void
PnozzSetupForSolidFill
(
    ScrnInfoPtr  pScrn,
    int          colour,
    int          rop,
    unsigned int planemask
)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    CARD32 c2;

    PnozzSync(pScrn);
    unClip(pPnozz);
    switch(pPnozz->depthshift) 
    {
    	case 0:
	    c2 = (colour << 8 | colour);
	    pnozz_write_4(pPnozz, FOREGROUND_COLOR, c2 << 16 | c2);
	    break;
    	case 1:
	    c2 = (colour << 16 | colour);
	    pnozz_write_4(pPnozz, FOREGROUND_COLOR, c2);
	    break;
    	case 2:
	    pnozz_write_4(pPnozz, FOREGROUND_COLOR, colour);
	    break;
    }
    pnozz_write_4(pPnozz, RASTER_OP, PnozzDrawROP[rop] & 0xff);
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pnozz_write_4(pPnozz, COORD_INDEX, 0);
}

static void
PnozzSubsequentSolidFillRect
(
    ScrnInfoPtr pScrn,
    int         x,
    int         y,
    int         w,
    int         h
)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    PnozzSync(pScrn);
    pnozz_write_4(pPnozz, RECT_RTW_XY, ((CARD32)x << 16) | 
        ((CARD32)y & 0xFFFFL));
    pnozz_write_4(pPnozz, RECT_RTW_XY, ((CARD32)(x + w) << 16) | 
        ((CARD32)(y + h) & 0xFFFFL));
    junk=pnozz_read_4(pPnozz, COMMAND_QUAD);
}

int
PnozzAccelInit(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    XAAInfoRecPtr pXAAInfo = pPnozz->pXAA;

    pXAAInfo->Flags = LINEAR_FRAMEBUFFER | PIXMAP_CACHE | OFFSCREEN_PIXMAPS;
    pXAAInfo->maxOffPixWidth = pPnozz->width;
    pXAAInfo->maxOffPixHeight = pPnozz->maxheight;
    MaxClip = ((pPnozz->scanlinesize & 0xffff) << 16) | (pPnozz->maxheight);
    
    PnozzInitEngine(pPnozz);
    
#if 1
    {
	CARD32 src, srcw, junk;
	src = 0;
	srcw = (pPnozz->width) << 16 | (pPnozz->height);
	
	/* Blit the screen white. For aesthetic reasons. */
	
	PnozzSync(pScrn);
	pnozz_write_4(pPnozz, FOREGROUND_COLOR, 0xffffffff);
	pnozz_write_4(pPnozz, BACKGROUND_COLOR, 0xffffffff);
	pnozz_write_4(pPnozz, RASTER_OP, ROP_PAT);
	pnozz_write_4(pPnozz, COORD_INDEX, 0);
	pnozz_write_4(pPnozz, RECT_RTW_XY, src);
	pnozz_write_4(pPnozz, RECT_RTW_XY, srcw);	
	junk = pnozz_read_4(pPnozz, COMMAND_QUAD);
	PnozzSync(pScrn);
    }
#endif

#if 0
    {
    	unsigned short *sptr = pPnozz->fb;
	int i,j;
	unsigned short blah;
	/*
	for (i = 0; i < 600; i++) {
	    for (j = 0; j < 400; j++) {
	    	if (j & 4) {
		    	*sptr = 0xffff;
		} else {
			*sptr = 0;
		}
		sptr++;
	    }
	}
	*/
	for (i = 0; i < 400; i++) {
	    *sptr = 0xffff;
	    sptr++;
	    *sptr = 0x0000;
	    sptr+=800;
	 }
    }
#endif
    /* Sync */
    pXAAInfo->Sync = PnozzSync;
    /* Screen-to-screen copy */
    pXAAInfo->ScreenToScreenCopyFlags = NO_TRANSPARENCY;
    pXAAInfo->SetupForScreenToScreenCopy = PnozzSetupForScreenToScreenCopy;
    pXAAInfo->SubsequentScreenToScreenCopy =
        PnozzSubsequentScreenToScreenCopy;


    /* Solid fills */
    pXAAInfo->SetupForSolidFill = PnozzSetupForSolidFill;
    pXAAInfo->SubsequentSolidFillRect = PnozzSubsequentSolidFillRect;
#if 0
    /* 8x8 mono pattern fills */
    pXAAInfo->Mono8x8PatternFillFlags = BIT_ORDER_IN_BYTE_MSBFIRST |
	HARDWARE_PATTERN_PROGRAMMED_BITS | HARDWARE_PATTERN_SCREEN_ORIGIN;
	
    pXAAInfo->SetupForMono8x8PatternFill = PnozzSetupForMono8x8PatternFill;
    pXAAInfo->SubsequentMono8x8PatternFillRect =
        PnozzSubsequentMono8x8PatternFillRect;
#endif
    return 0;
}
