/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/neo/neo_2070.c,v 1.1.2.2 1998/09/27 13:25:17 hohndel Exp $ */
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
 * This module supports acceleration for 2070.
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

/* Memory Mapped I/O for BitBlt */
#define NEO2070_BLTSTAT		0x00
#define NEO2070_BLTCNTL		0x04
#define NEO2070_XPCOLOR		0x08
#define NEO2070_FGCOLOR		0x0c
#define NEO2070_BGCOLOR		0x10
#define NEO2070_PLANEMASK	0x14
#define NEO2070_XYEXT           0x18
#define NEO2070_SRCPITCH        0x1c
#define NEO2070_SRCBITOFF       0x20
#define NEO2070_SRCSTART        0x24
#define NEO2070_DSTPITCH        0x28
#define NEO2070_DSTBITOFF       0x2c
#define NEO2070_DSTSTART        0x30

static void Neo2070SetupForScreenToScreenCopy();
static void Neo2070SubsequentScreenToScreenCopy();
static void Neo2070SetupForFillRectSolid();
static void Neo2070SubsequentFillRectSolid();

static unsigned int neo2070tmpBltCntlFlags;
static unsigned int neo2070BltCntlFlags;
static unsigned int neo2070Pitch, neo2070PixelWidth;
static unsigned int neo2070ColorShiftAmt = 0;
static unsigned int neo2070PlaneMask;
static unsigned int neo2070Rop[16] = {
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
Neo2070AccelInit()
{
    /* Initialize for 8bpp or 15/16bpp support accellerated */
    switch (vgaBitsPerPixel) {
    case 8:
	neo2070BltCntlFlags = NEO_BC1_DEPTH8;
	neo2070ColorShiftAmt = 8;
	neo2070PixelWidth = 1;
	neo2070PlaneMask = 0xff;
	break;
    case 15:
    case 16:
	neo2070BltCntlFlags = NEO_BC1_DEPTH16;
	neo2070ColorShiftAmt = 0;
	neo2070PixelWidth = 2;
	neo2070PlaneMask = 0xffff;
	break;
    case 24: /* not supported, but check anyway */
    default:
	return;
    }

    neo2070Pitch = vga256InfoRec.displayWidth * neo2070PixelWidth;

    xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS;

    /* sync */
    xf86AccelInfoRec.Sync = Neo2070Sync;

    /* screen to screen copy */
    xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY | GXCOPY_ONLY;
    xf86AccelInfoRec.SetupForScreenToScreenCopy = 
			Neo2070SetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy = 
			Neo2070SubsequentScreenToScreenCopy;

    /* solid filled rectangles */
    xf86GCInfoRec.PolyFillRectSolidFlags = GXCOPY_ONLY;
    xf86AccelInfoRec.SetupForFillRectSolid = 
			Neo2070SetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = 
			Neo2070SubsequentFillRectSolid;

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
Neo2070Sync()
{
    WAIT_ENGINE_IDLE();
}


void
Neo2070SetupForScreenToScreenCopy(xdir, ydir, rop, planemask, trans_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int trans_color;
{
    neo2070tmpBltCntlFlags = (neo2070BltCntlFlags | neo2070Rop[rop]);
    planemask &= neo2070PlaneMask;

    /* set blt control */
    WAIT_ENGINE_IDLE();
    OUTREG(NEO2070_BLTCNTL, neo2070tmpBltCntlFlags);
    OUTREG(NEO2070_PLANEMASK, planemask |= (planemask << neo2070ColorShiftAmt));
    OUTREG(NEO2070_SRCPITCH, neo2070Pitch);
    OUTREG(NEO2070_DSTPITCH, neo2070Pitch);
    OUTREG(NEO2070_SRCBITOFF, 0);
    OUTREG(NEO2070_DSTBITOFF, 0);
}


void
Neo2070SubsequentScreenToScreenCopy(srcX, srcY, dstX, dstY, w, h)
    int srcX, srcY, dstX, dstY, w, h;
{
    if ((dstY < srcY) || ((dstY == srcY) && (dstX < srcX))) {
	/* start with upper left corner */
	WAIT_ENGINE_IDLE();
	OUTREG(NEO2070_BLTCNTL, neo2070tmpBltCntlFlags);
	OUTREG(NEO2070_XYEXT, ((h-1)<<16) | ((w-1) & 0xffff));
	OUTREG(NEO2070_SRCSTART, 
	    (srcY * neo2070Pitch) + (srcX * neo2070PixelWidth));
	OUTREG(NEO2070_DSTSTART, 
	    (dstY * neo2070Pitch) + (dstX * neo2070PixelWidth));
    }
    else {
	/* start with lower right corner */
	WAIT_ENGINE_IDLE();
	OUTREG(NEO2070_BLTCNTL, (neo2070tmpBltCntlFlags | NEO_BC0_X_DEC
                                                        | NEO_BC0_DST_Y_DEC 
                                                        | NEO_BC0_SRC_Y_DEC));
	OUTREG(NEO2070_XYEXT, ((h-1)<<16) | ((w-1) & 0xffff));
	OUTREG(NEO2070_SRCSTART, 
	    ((srcY+h-1) * neo2070Pitch) + ((srcX+w-1) * neo2070PixelWidth));
	OUTREG(NEO2070_DSTSTART, 
	    ((dstY+h-1) * neo2070Pitch) + ((dstX+w-1) * neo2070PixelWidth));
    }
}


void
Neo2070SetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
    planemask &= neo2070PlaneMask;
    if (!rop) color=0;

    WAIT_ENGINE_IDLE();

    OUTREG(NEO2070_BLTCNTL, neo2070BltCntlFlags  |
                            NEO_BC0_SRC_IS_FG    | neo2070Rop[3]);
    OUTREG(NEO2070_PLANEMASK, planemask |= (planemask << neo2070ColorShiftAmt));
    if (vgaBitsPerPixel == 8) 
	OUTREG(NEO2070_FGCOLOR, color |= (color << 8));
    else
	/* swap bytes in color */
	OUTREG(NEO2070_FGCOLOR, ((color&0xff00) >> 8) | (color << 8));
    OUTREG(NEO2070_SRCPITCH, neo2070Pitch);
    OUTREG(NEO2070_DSTPITCH, neo2070Pitch);
    OUTREG(NEO2070_SRCBITOFF, 0);
    OUTREG(NEO2070_DSTBITOFF, 0);
}


void
Neo2070SubsequentFillRectSolid(x, y, w, h)
    int x, y, w, h;
{
    WAIT_ENGINE_IDLE();
    OUTREG(NEO2070_XYEXT, ((h-1)<<16) | ((w-1) & 0xffff));
    OUTREG(NEO2070_DSTSTART, (y * neo2070Pitch) + (x * neo2070PixelWidth));
}
