/*
 * SBus Weitek P9100 hardware acceleration support
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
/* $NetBSD: pnozz_accel.c,v 1.2 2011/05/25 14:15:26 christos Exp $ */

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dev/sun/fbio.h>
#include <dev/wscons/wsconsio.h>

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

void
PnozzSync(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    while((pnozz_read_4(pPnozz, ENGINE_STATUS) & 
        (ENGINE_BUSY | BLITTER_BUSY)) !=0 );
}

/*
 * Both the framebuffer and the colour registers are apparently little endian.
 * For framebuffer accesses we can just turn on byte swapping, for the colour
 * registers we need to juggle bytes ourselves.
 */

static void
pnozz_write_colour(PnozzPtr pPnozz, int reg, CARD32 colour)
{
    CARD32 c2;
    
    switch(pPnozz->depthshift) 
    {
    	case 0:
	    c2 = (colour << 8 | colour);
	    pnozz_write_4(pPnozz, reg, c2 << 16 | c2);
	    break;
    	case 1:
    	    c2 = ((colour & 0xff) << 8) | ((colour & 0xff00) >> 8);
	    c2 |= c2 << 16;
	    pnozz_write_4(pPnozz, reg, c2);
	    break;
    	case 2:
    	    c2 = ((colour & 0x00ff00ff) << 8) | ((colour & 0xff00ff00) >> 8);
    	    c2 = (( c2 & 0xffff0000) >> 16) | ((c2 & 0x0000ffff) << 16);
	    pnozz_write_4(pPnozz, reg, c2);
	    break;
    }
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

    pnozz_write_4(pPnozz, RASTER_OP, (PnozzCopyROP[rop] & 0xff));
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
}

/*
 * the drawing engine is weird. Even though BLIT and QUAD commands use the
 * same registers to program coordinates there's an important difference -
 * horizontal coordinates for QUAD commands are in pixels, for BLIT commands
 * and the byte clipping registers they're IN BYTES.
 */
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
    
    src = (((xSrc << pPnozz->depthshift) & 0x1fff) << 16) | (ySrc & 0x1fff);
    dst = (((xDst << pPnozz->depthshift) & 0x1fff) << 16) | (yDst & 0x1fff);
    srcw = ((((xSrc + w) << pPnozz->depthshift) - 1) << 16) | 
        ((ySrc + h - 1) & 0x1fff);
    dstw = ((((xDst + w) << pPnozz->depthshift) - 1) << 16) |
        ((yDst + h - 1) & 0x1fff);

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

    pnozz_write_colour(pPnozz, FOREGROUND_COLOR, colour);
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
    pnozz_write_4(pPnozz, RECT_RTW_XY, ((x & 0x1fff) << 16) | 
        (y & 0x1fff));
    pnozz_write_4(pPnozz, RECT_RTP_XY, (((w & 0x1fff) << 16) | 
        (h & 0x1fff)));
    junk = pnozz_read_4(pPnozz, COMMAND_QUAD);
}

static void 
PnozzSetupForCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
        		int fg, int bg,
			int rop,
			unsigned int planemask)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    PnozzSync(pScrn);
    
    if (bg == -1) {
    	/* transparent */
	pnozz_write_colour(pPnozz, FOREGROUND_COLOR, fg);    
	pnozz_write_4(pPnozz, RASTER_OP, PnozzDrawROP[rop] | ROP_PIX1_TRANS);
    } else {
	/* 
	 * this doesn't make any sense to me either, but for some reason the
	 * chip applies the foreground colour to 0 pixels and background to 1
	 * when set to this sort of ROP. The old XF 3.3 driver source claimed
	 * that the chip doesn't support opaque colour expansion at all.
	 */
	pnozz_write_colour(pPnozz, FOREGROUND_COLOR, bg);
	pnozz_write_colour(pPnozz, BACKGROUND_COLOR, fg);
    
	pnozz_write_4(pPnozz, RASTER_OP, PnozzCopyROP[rop] & 0xff);
    }
    
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pnozz_write_4(pPnozz, COORD_INDEX, 0);
}

static void 
PnozzSubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
			int x, int y, int w, int h,
			int skipleft )
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    CARD32 rest = w & 0x1f;
    
    pnozz_write_4(pPnozz, ABS_X0, x);
    pnozz_write_4(pPnozz, ABS_XY1, (x << 16) | (y & 0xFFFFL));
    pnozz_write_4(pPnozz, ABS_X2, (x + w));
    pnozz_write_4(pPnozz, ABS_Y3, 1);

    pPnozz->words = (w >> 5);	/* whole words to write */
    
    if (rest > 0) {
    	pPnozz->last_word = (rest - 1) << 2;
    } else
    	pPnozz->last_word = -1;
}

static void
PnozzSubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
#define PIXEL_1_FULL (PIXEL_1 + (31 << 2))
    CARD32 *buf;
    volatile CARD32 *pix = ((volatile CARD32 *)(pPnozz->fbc + PIXEL_1_FULL));
    int i = 0;

    PnozzSync(pScrn);
    buf = (CARD32 *)pPnozz->buffers[bufno];
    junk = *(volatile CARD32 *)(pPnozz->fb + PIXEL_1_FULL);
    for (i = 0; i < pPnozz->words; i++)
	*pix = buf[i];
    if (pPnozz->last_word >= 0)
    	*(volatile CARD32 *)(pPnozz->fbc + PIXEL_1 + pPnozz->last_word) =
    	    buf[i];
}

static void 
PnozzSetupForSolidLine(ScrnInfoPtr pScrn, int color, int rop, 
    unsigned int planemask)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    PnozzSync(pScrn);
    
    pnozz_write_colour(pPnozz, FOREGROUND_COLOR, color);    
    pnozz_write_4(pPnozz, RASTER_OP, (PnozzDrawROP[rop] & 0xff) | ROP_OVERSIZE);
    
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pnozz_write_4(pPnozz, COORD_INDEX, 0);

}

static void
PnozzSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn, int x1, int y1, int x2,
    int y2, int flags)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    PnozzSync(pScrn);
    
    /* 
     * XXX we're blatantly ignoring the flags parameter which could tell us not 
     * to draw the last point. Xsun simply reads it from the framebuffer and 
     * puts it back after drawing the line but that would mean we have to wait 
     * until the line is actually drawn. On the other hand - line drawing is 
     * pretty fast so we won't lose too much speed
     */
    pnozz_write_4(pPnozz, LINE_RTW_XY, (x1 << 16) | y1);
    pnozz_write_4(pPnozz, LINE_RTW_XY, (x2 << 16) | y2);
    junk = pnozz_read_4(pPnozz, COMMAND_QUAD);
}      

static void
PnozzSetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int pat0, int pat1,
        int fg, int bg, int rop, unsigned int planemask)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    CARD32 pat;
    
    PnozzSync(pScrn);

    if (bg == -1) {
	pnozz_write_4(pPnozz, RASTER_OP, 
	    (PnozzDrawROP[rop] & 0xff) | ROP_NO_SOLID | ROP_TRANS);
    } else {
        pnozz_write_colour(pPnozz, COLOR_0, bg);
        pnozz_write_4(pPnozz, RASTER_OP,
            (PnozzDrawROP[rop] & 0xff) | ROP_NO_SOLID);
    }
    pnozz_write_colour(pPnozz, COLOR_1, fg);
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pat = (pat0 & 0xff000000) | ((pat0 >> 8) & 0x00ffff00) | 
        ((pat0 >> 16) & 0x000000ff);
    pnozz_write_4(pPnozz, PATTERN0, pat);
    pat = ((pat0 << 8) & 0x00ffff00) | ((pat0 << 16) & 0xff000000) | 
        (pat0 & 0x000000ff);
    pnozz_write_4(pPnozz, PATTERN1, pat);
    pat = (pat1 & 0xff000000) | ((pat1 >> 8) & 0x00ffff00) | 
        ((pat1 >> 16) & 0x000000ff);
    pnozz_write_4(pPnozz, PATTERN2, pat);
    pat = ((pat1 << 8) & 0x00ffff00) | ((pat1 << 16) & 0xff000000) | 
        (pat1 & 0x000000ff);
    pnozz_write_4(pPnozz, PATTERN3, pat);
    pnozz_write_4(pPnozz, COORD_INDEX, 0);
}

static void
PnozzSubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn,
        	int patx, int paty, int x, int y, int w, int h)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    PnozzSync(pScrn);
    pnozz_write_4(pPnozz, PATTERN_ORIGIN_X, patx);
    pnozz_write_4(pPnozz, PATTERN_ORIGIN_Y, paty);
    pnozz_write_4(pPnozz, RECT_RTW_XY, ((x & 0x1fff) << 16) | 
        (y & 0x1fff));
    pnozz_write_4(pPnozz, RECT_RTP_XY, (((w & 0x1fff) << 16) | 
        (h & 0x1fff)));
    junk = pnozz_read_4(pPnozz, COMMAND_QUAD);

}

static void
PnozzSetClippingRectangle(ScrnInfoPtr pScrn, int left, int top, int right, 
			 int bottom)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    CARD32 cmin, cmax;

    cmin = (left << 16) | top;
    cmax = (right << 16) | bottom;
    
    pnozz_write_4(pPnozz, WINDOW_MIN, cmin);
    pnozz_write_4(pPnozz, WINDOW_MAX, cmax);

    cmin = ((left << pPnozz->depthshift) << 16) | top;
    cmax = ((right << pPnozz->depthshift) << 16) | bottom;
    
    pnozz_write_4(pPnozz, BYTE_CLIP_MIN, cmin);
    pnozz_write_4(pPnozz, BYTE_CLIP_MAX, cmax);
}

static void
PnozzDisableClipping(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    pnozz_write_4(pPnozz, WINDOW_MIN, 0);
    pnozz_write_4(pPnozz, WINDOW_MAX, MaxClip);
    pnozz_write_4(pPnozz, BYTE_CLIP_MIN, 0);
    pnozz_write_4(pPnozz, BYTE_CLIP_MAX, MaxClip);
}

static void
PnozzSetupForImageWrite(ScrnInfoPtr pScrn, int rop, unsigned int planemask,
  int trans_color, int depth, int bpp)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    pnozz_write_4(pPnozz, RASTER_OP, PnozzCopyROP[rop] & 0xff);
    pnozz_write_4(pPnozz, PLANE_MASK, planemask);
    pnozz_write_4(pPnozz, COORD_INDEX, 0);
    
    xf86Msg(X_ERROR, "setup for image write\n");
}

static void
PnozzImageWriteRect(ScrnInfoPtr pScrn, int x, int y, int wi, int he, int skip)
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    volatile CARD32 junk;
    
    pnozz_write_4(pPnozz, ABS_X0, x);
    pnozz_write_4(pPnozz, ABS_XY1, (x << 16) | y);
    pnozz_write_4(pPnozz, ABS_X2, x + wi);
    pnozz_write_4(pPnozz, ABS_Y3, 1);
    junk = *(volatile CARD32 *)(pPnozz->fb + PIXEL_8);
}

/*
 * TODO:
 * - CPU to VRAM colour blits
 * - DGA support
 */

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
	srcw = pPnozz->width << 16 | pPnozz->height;
	
	/* Blit the screen black. For aesthetic reasons. */
	
	PnozzSync(pScrn);
	pnozz_write_4(pPnozz, FOREGROUND_COLOR, 0x00000000);
	pnozz_write_4(pPnozz, BACKGROUND_COLOR, 0xffffffff);
	pnozz_write_4(pPnozz, RASTER_OP, ROP_PAT);
	pnozz_write_4(pPnozz, COORD_INDEX, 0);
	pnozz_write_4(pPnozz, RECT_RTW_XY, src);
	pnozz_write_4(pPnozz, RECT_RTW_XY, srcw);	
	junk = pnozz_read_4(pPnozz, COMMAND_QUAD);
	PnozzSync(pScrn);
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

    /* colour expansion */
    pXAAInfo->ScanlineCPUToScreenColorExpandFillFlags = 
	/*LEFT_EDGE_CLIPPING|*/SCANLINE_PAD_DWORD;
    pXAAInfo->NumScanlineColorExpandBuffers = 2;
    pPnozz->buffers[0] = (unsigned char *)pPnozz->Buffer;
    pPnozz->buffers[1] = (unsigned char *)&pPnozz->Buffer[pPnozz->scanlinesize];
    pXAAInfo->ScanlineColorExpandBuffers = pPnozz->buffers;
    pXAAInfo->SetupForScanlineCPUToScreenColorExpandFill = 
	PnozzSetupForCPUToScreenColorExpandFill;
    pXAAInfo->SubsequentScanlineCPUToScreenColorExpandFill =
	PnozzSubsequentScanlineCPUToScreenColorExpandFill;
    pXAAInfo->SubsequentColorExpandScanline =
	PnozzSubsequentColorExpandScanline;

    /* line drawing */
    pXAAInfo->SetupForSolidLine = PnozzSetupForSolidLine;
    pXAAInfo->SubsequentSolidTwoPointLine = PnozzSubsequentSolidTwoPointLine;
    pXAAInfo->SolidLineFlags = BIT_ORDER_IN_BYTE_MSBFIRST;

    /* clipping */
    pXAAInfo->SetClippingRectangle = PnozzSetClippingRectangle;
    pXAAInfo->DisableClipping = PnozzDisableClipping;
    pXAAInfo->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY |
        HARDWARE_CLIP_SOLID_FILL |
        HARDWARE_CLIP_MONO_8x8_FILL |
        /*HARDWARE_CLIP_COLOR_8x8_FILL |*/
        HARDWARE_CLIP_SOLID_LINE;

    /* 8x8 mono pattern fills */
    pXAAInfo->Mono8x8PatternFillFlags = HARDWARE_PATTERN_PROGRAMMED_BITS |
        HARDWARE_PATTERN_SCREEN_ORIGIN | HARDWARE_PATTERN_PROGRAMMED_ORIGIN;
    pXAAInfo->SetupForMono8x8PatternFill = PnozzSetupForMono8x8PatternFill;
    pXAAInfo->SubsequentMono8x8PatternFillRect =
        PnozzSubsequentMono8x8PatternFillRect;

    /* image uploads */
    pXAAInfo->ImageWriteBase = pPnozz->fbc + PIXEL_8;
    pXAAInfo->ImageWriteRange = 4;
    pXAAInfo->ImageWriteFlags = /*CPU_TRANSFER_BASE_FIXED |*/ CPU_TRANSFER_PAD_DWORD |
        NO_TRANSPARENCY;
    pXAAInfo->SetupForImageWrite = PnozzSetupForImageWrite;
    pXAAInfo->SubsequentImageWriteRect = PnozzImageWriteRect;
    
    return 0;
}
