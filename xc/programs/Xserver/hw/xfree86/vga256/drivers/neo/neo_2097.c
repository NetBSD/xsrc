/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/neo/neo_2097.c,v 1.1.2.2 1998/09/27 13:25:18 hohndel Exp $ */
/**********************************************************************
Copyright 1998 by Precision Insight, Inc., Cedar Park, Texas.

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and
its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Precision Insight not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  Precision Insight
and its suppliers make no representations about the suitability of this
software for any purpose.  It is provided "as is" without express or 
implied warranty.

PRECISION INSIGHT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************/

/*
 * This Precision Insight driver has been sponsored by Red Hat.
 *
 * Authors:
 *   Jens Owen (jens@precisioninsight.com)
 *   Kevin E. Martin (kevin@precisioninsight.com)
 */

/*
 * This module supports acceleration for 2097 and 2160.
 */

#include "compiler.h"
#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "vgaPCI.h"

#include "miline.h"

#include "xf86xaa.h"
#include "xf86local.h"

#include "neo.h"
#include "neo_reg.h"
#include "neo_macros.h"

static void Neo2097SetupForScreenToScreenCopy();
static void Neo2097SubsequentScreenToScreenCopy();
static void Neo2097SetupForFillRectSolid();
static void Neo2097SubsequentFillRectSolid();
static void Neo2097SetupForCPUToScreenColorExpand();
static void Neo2097SubsequentCPUToScreenColorExpand();
static void Neo2097SetupFor8x8PatternColorExpand();
static void Neo2097Subsequent8x8PatternColorExpand();

static unsigned int neo2097tmpBltCntlFlags;
static unsigned int neo2097BltCntlFlags;
static unsigned int neo2097ColorShiftAmt = 0;
static unsigned int neo2097Rop[16] = {
    0x000000,    /* 0x0000 - GXclear         */
    0x080000,    /* 0x1000 - GXand           */
    0x040000,    /* 0x0100 - GXandReverse    */
    0x0c0000,    /* 0x1100 - GXcopy          */
    0x020000,    /* 0x0010 - GXandInvert     */
    0x0a0000,    /* 0x1010 - GXnoop          */
    0x060000,    /* 0x0110 - GXxor           */
    0x0e0000,    /* 0x1110 - GXor            */
    0x010000,    /* 0x0001 - GXnor           */
    0x090000,    /* 0x1001 - GXequiv         */
    0x050000,    /* 0x0101 - GXinvert        */
    0x0d0000,    /* 0x1101 - GXorReverse     */
    0x030000,    /* 0x0011 - GXcopyInvert    */
    0x0b0000,    /* 0x1011 - GXorInverted    */
    0x070000,    /* 0x0111 - GXnand          */
    0x0f0000     /* 0x1111 - GXset           */
};


void 
Neo2097AccelInit()
{
    /* Initialize for 8bpp or 15/16bpp support accellerated */
    switch (vgaBitsPerPixel) {
    case 8:
	neo2097BltCntlFlags = NEO_BC1_DEPTH8;
	neo2097ColorShiftAmt = 8;
	break;
    case 15:
    case 16:
	neo2097BltCntlFlags = NEO_BC1_DEPTH16;
	neo2097ColorShiftAmt = 0;
	break;
    case 24:
    default:
	return;
    }

    /* Initialize for widths */
    switch (vga256InfoRec.displayWidth) {
    case 640:
	neo2097BltCntlFlags |= NEO_BC1_X_640;
	break;
    case 800:
	neo2097BltCntlFlags |= NEO_BC1_X_800;
	break;
    case 1024:
	neo2097BltCntlFlags |= NEO_BC1_X_1024;
	break;
    default:
	return;
    }

    xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS |
                             HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
                             HARDWARE_PATTERN_BIT_ORDER_MSBFIRST |
                             HARDWARE_PATTERN_MONO_TRANSPARENCY;

    /* sync */
    xf86AccelInfoRec.Sync = Neo2097Sync;

    /* screen to screen copy */
    xf86GCInfoRec.CopyAreaFlags = (NO_TRANSPARENCY | NO_PLANEMASK);
    xf86AccelInfoRec.SetupForScreenToScreenCopy = 
			Neo2097SetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy = 
			Neo2097SubsequentScreenToScreenCopy;

    /* solid filled rectangles */
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
    xf86AccelInfoRec.SetupForFillRectSolid = 
			Neo2097SetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = 
			Neo2097SubsequentFillRectSolid;

    /* cpu to screen color expansion */
    xf86AccelInfoRec.ColorExpandFlags = ( NO_PLANEMASK |
					  SCANLINE_PAD_BYTE |
					  CPU_TRANSFER_PAD_DWORD |
					  BIT_ORDER_IN_BYTE_MSBFIRST );
    xf86AccelInfoRec.CPUToScreenColorExpandBase = 
			(unsigned*)(NeoMMIOBase + 0x100000);
    xf86AccelInfoRec.CPUToScreenColorExpandRange = 0x100000;

    xf86AccelInfoRec.SetupForCPUToScreenColorExpand = 
			Neo2097SetupForCPUToScreenColorExpand;
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand = 
			Neo2097SubsequentCPUToScreenColorExpand;

    /* 8x8 pattern fills */
    xf86AccelInfoRec.SetupFor8x8PatternColorExpand = 
			Neo2097SetupFor8x8PatternColorExpand;
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand = 
			Neo2097Subsequent8x8PatternColorExpand;

    /* pixmap cache */
    {
        int cacheStart, cacheEnd;

        cacheStart = vga256InfoRec.virtualY * 
                       vga256InfoRec.displayWidth * vgaBitsPerPixel / 8;

        /* reserve:
         *  space for hardware cursor
         *  ScratchBufferSize bytes for scanlineColorExpand  
         */
	if (NeoCursorMemSegment)
	    cacheEnd = NeoCursorMemSegment * 1024 
			 - xf86AccelInfoRec.ScratchBufferSize;
	else 
	    cacheEnd = vga256InfoRec.videoRam * 1024 
			 - xf86AccelInfoRec.ScratchBufferSize;

        /* NOT_DONE: do we need some kind of minimum threshold for 
         * available memory before turning on the pixmap cache?
         *
         * i.e. if(cacheEnd > cacheStart+THRESHOLD)
         */
        if(cacheEnd > cacheStart) {
	    xf86AccelInfoRec.PixmapCacheMemoryStart = cacheStart;
	    xf86AccelInfoRec.PixmapCacheMemoryEnd = cacheEnd;
	    xf86AccelInfoRec.Flags |= PIXMAP_CACHE;
	}
    }
}


void
Neo2097Sync()
{
    WAIT_ENGINE_IDLE();
}


void
Neo2097SetupForScreenToScreenCopy(xdir, ydir, rop, planemask, trans_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int trans_color;
{
    neo2097tmpBltCntlFlags = (neo2097BltCntlFlags  |
		              NEO_BC3_SKIP_MAPPING |
		              NEO_BC3_DST_XY_ADDR  |
		              NEO_BC3_SRC_XY_ADDR  | neo2097Rop[rop]);

    /* set blt control */
    WAIT_ENGINE_IDLE();
    OUTREG(NEOREG_BLTCNTL, neo2097tmpBltCntlFlags);
}


void
Neo2097SubsequentScreenToScreenCopy(srcX, srcY, dstX, dstY, w, h)
    int srcX, srcY, dstX, dstY, w, h;
{
    if ((dstY < srcY) || ((dstY == srcY) && (dstX < srcX))) {
	/* start with upper left corner */
	WAIT_ENGINE_IDLE();
	OUTREG(NEOREG_BLTCNTL, neo2097tmpBltCntlFlags);
	OUTREG(NEOREG_SRCSTARTOFF, (srcY<<16) | (srcX & 0xffff));
	OUTREG(NEOREG_DSTSTARTOFF, (dstY<<16) | (dstX & 0xffff));
	OUTREG(NEOREG_XYEXT, (h<<16) | (w & 0xffff));
    }
    else {
	/* start with lower right corner */
	WAIT_ENGINE_IDLE();
	OUTREG(NEOREG_BLTCNTL, (neo2097tmpBltCntlFlags | NEO_BC0_X_DEC
                                                       | NEO_BC0_DST_Y_DEC 
                                                       | NEO_BC0_SRC_Y_DEC));
	OUTREG(NEOREG_SRCSTARTOFF, ((srcY+h-1)<<16) | ((srcX+w-1) & 0xffff));
	OUTREG(NEOREG_DSTSTARTOFF, ((dstY+h-1)<<16) | ((dstX+w-1) & 0xffff));
	OUTREG(NEOREG_XYEXT, (h<<16) | (w & 0xffff));
    }
}


void
Neo2097SetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
    WAIT_ENGINE_IDLE();

    /* set blt control */
    OUTREG(NEOREG_BLTCNTL, neo2097BltCntlFlags  |
                           NEO_BC0_SRC_IS_FG    |
                           NEO_BC3_SKIP_MAPPING |
                           NEO_BC3_DST_XY_ADDR  |
                           NEO_BC3_SRC_XY_ADDR  | neo2097Rop[rop]);

    /* set foreground color */
    OUTREG(NEOREG_FGCOLOR, color |= (color << neo2097ColorShiftAmt));
}


void
Neo2097SubsequentFillRectSolid(x, y, w, h)
    int x, y, w, h;
{
    WAIT_ENGINE_IDLE();
    OUTREG(NEOREG_DSTSTARTOFF, (y<<16) | (x & 0xffff));
    OUTREG(NEOREG_XYEXT, (h<<16) | (w & 0xffff));
}


void Neo2097SetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
    if (bg == -1) {
	/* transparent setup */
	WAIT_ENGINE_IDLE();
	OUTREG(NEOREG_BLTCNTL, neo2097BltCntlFlags  |
			       NEO_BC0_SYS_TO_VID   |
			       NEO_BC0_SRC_MONO     |
			       NEO_BC0_SRC_TRANS    |
			       NEO_BC3_SKIP_MAPPING |
			       NEO_BC3_DST_XY_ADDR  | neo2097Rop[rop]);

	OUTREG(NEOREG_FGCOLOR, fg |= (fg << neo2097ColorShiftAmt));
    }
    else {
	/* opaque setup */
	WAIT_ENGINE_IDLE();
	OUTREG(NEOREG_BLTCNTL, neo2097BltCntlFlags  |
			       NEO_BC0_SYS_TO_VID   |
			       NEO_BC0_SRC_MONO     |
			       NEO_BC3_SKIP_MAPPING |
			       NEO_BC3_DST_XY_ADDR  | neo2097Rop[rop]);

	OUTREG(NEOREG_FGCOLOR, fg |= (fg << neo2097ColorShiftAmt));
	OUTREG(NEOREG_BGCOLOR, bg |= (bg << neo2097ColorShiftAmt));
    }
}


static void Neo2097SubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
    int x, y, w, h, skipleft;
{
    WAIT_ENGINE_IDLE();
    OUTREG(NEOREG_SRCBITOFF, skipleft);
    OUTREG(NEOREG_SRCSTARTOFF, 0);
    OUTREG(NEOREG_DSTSTARTOFF, (y<<16) | (x & 0xffff));
    OUTREG(NEOREG_XYEXT, (h<<16) | (w & 0xffff));
}


void Neo2097SetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, 
                                                        rop, planemask)
    unsigned patternx, patterny;
    int bg, fg, rop;
    unsigned planemask;
{
    if (bg == -1) {
	/* transparent setup */
	neo2097tmpBltCntlFlags = ( neo2097BltCntlFlags  |
		                   NEO_BC0_SRC_MONO     |
		                   NEO_BC0_FILL_PAT     |
		                   NEO_BC0_SRC_TRANS    |
		                   NEO_BC3_SKIP_MAPPING |
		                   NEO_BC3_DST_XY_ADDR  | neo2097Rop[rop]);

	WAIT_ENGINE_IDLE();
	OUTREG(NEOREG_FGCOLOR, fg |= (fg << neo2097ColorShiftAmt));
	OUTREG(NEOREG_SRCSTARTOFF, 
	    (patterny*vga256InfoRec.displayWidth*vgaBitsPerPixel + patternx)
              >> 3);
    }
    else {
	/* opaque setup */
	neo2097tmpBltCntlFlags = ( neo2097BltCntlFlags  |
		                   NEO_BC0_SRC_MONO     |
		                   NEO_BC0_FILL_PAT     |
		                   NEO_BC3_SKIP_MAPPING |
		                   NEO_BC3_DST_XY_ADDR  | neo2097Rop[rop]);

	WAIT_ENGINE_IDLE();
	OUTREG(NEOREG_FGCOLOR, fg |= (fg << neo2097ColorShiftAmt));
	OUTREG(NEOREG_BGCOLOR, bg |= (bg << neo2097ColorShiftAmt));
	OUTREG(NEOREG_SRCSTARTOFF, 
	    (patterny*vga256InfoRec.displayWidth*vgaBitsPerPixel + patternx)
              >> 3);
    }
}


static void Neo2097Subsequent8x8PatternColorExpand(patternx, patterny, 
                                                               x, y, w, h)
    unsigned patternx, patterny;
    int x, y, w, h;
{
    patterny &= 0x7;

    WAIT_ENGINE_IDLE();
    OUTREG(NEOREG_BLTCNTL, neo2097tmpBltCntlFlags | (patterny << 20));
    OUTREG(NEOREG_SRCBITOFF, patternx);
    OUTREG(NEOREG_DSTSTARTOFF, (y<<16) | (x & 0xffff));
    OUTREG(NEOREG_XYEXT, (h<<16) | (w & 0xffff));
}
