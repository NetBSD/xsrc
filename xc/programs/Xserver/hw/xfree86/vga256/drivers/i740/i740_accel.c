/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i740/i740_accel.c,v 1.1.2.2 1999/05/14 09:00:19 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * $PI: i740_accel.c,v 1.8 1999/02/18 20:50:59 martin Exp martin $
 */

#include <math.h>

#include "compiler.h"
#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "vgaPCI.h"

#include "xf86xaa.h"

#include "i740.h"
#include "i740_reg.h"
#include "i740_macros.h"

static unsigned int i740Rop[16] = {
    0x00, /* GXclear      */
    0x88, /* GXand        */
    0x44, /* GXandReverse */
    0xCC, /* GXcopy       */
    0x22, /* GXandInvert  */
    0xAA, /* GXnoop       */
    0x66, /* GXxor        */
    0xEE, /* GXor         */
    0x11, /* GXnor        */
    0x99, /* GXequiv      */
    0x55, /* GXinvert     */
    0xDD, /* GXorReverse  */
    0x33, /* GXcopyInvert */
    0xBB, /* GXorInverted */
    0x77, /* GXnand       */
    0xFF  /* GXset        */
};

static unsigned int i740PatternRop[16] = {
    0x00, /* GXclear      */
    0xA0, /* GXand        */
    0x50, /* GXandReverse */
    0xF0, /* GXcopy       */
    0x0A, /* GXandInvert  */
    0xAA, /* GXnoop       */
    0x5A, /* GXxor        */
    0xFA, /* GXor         */
    0x05, /* GXnor        */
    0xA5, /* GXequiv      */
    0x55, /* GXinvert     */
    0xF5, /* GXorReverse  */
    0x0F, /* GXcopyInvert */
    0xAF, /* GXorInverted */
    0x5F, /* GXnand       */
    0xFF  /* GXset        */
};

static void I740Sync();
static void I740SetupForFillRectSolid();
static void I740SubsequentFillRectSolid();
static void I740SetupForScreenToScreenCopy();
static void I740SubsequentScreenToScreenCopy();
static void I740SetupFor8x8PatternColorExpand();
static void I740Subsequent8x8PatternColorExpand();
static void I740SetupForCPUToScreenColorExpand();
static void I740SubsequentCPUToScreenColorExpand();

static GFX2DOPREG_BLTER_FULL_LOAD bltcmd;

/*
 * The following function sets up the supported acceleration. Call it
 * from the FbInit() function in the SVGA driver, or before ScreenInit
 * in a monolithic server.
 */
void
I740AccelInit() {
    if (vgaBitsPerPixel == 32) {
	xf86AccelInfoRec.Flags = 0; /* Disables all acceleration */
	return;
    }

    xf86AccelInfoRec.Flags = (BACKGROUND_OPERATIONS |
			      PIXMAP_CACHE |
			      HARDWARE_PATTERN_SCREEN_ORIGIN |
			      HARDWARE_PATTERN_BIT_ORDER_MSBFIRST |
			      HARDWARE_PATTERN_MONO_TRANSPARENCY);

    /* Sync */
    xf86AccelInfoRec.Sync = I740Sync;

    /* Solid filled rectangles */
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;
    xf86AccelInfoRec.SetupForFillRectSolid =
	I740SetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid =
	I740SubsequentFillRectSolid;

    /* Screen to screen copy */
    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK;
    if (vgaBitsPerPixel == 24) {
	xf86GCInfoRec.CopyAreaFlags |= NO_TRANSPARENCY;
    }
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
	I740SetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	I740SubsequentScreenToScreenCopy;

    /* 8x8 pattern fills */
    xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
	I740SetupFor8x8PatternColorExpand;
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
	I740Subsequent8x8PatternColorExpand;

    /* CPU to screen color expansion */
#ifndef ALLOW_PCI_COLOR_EXP
    if (I740Chipset != PCI_CHIP_I740_PCI) {
#endif
	/*
	 * Currently, we are not properly able to read the bitblt engine
	 * busy bit on the PCI i740 card.  When we are able to do so, we
	 * can re-enable color expansion.
	 */
	xf86AccelInfoRec.ColorExpandFlags = (NO_PLANEMASK |
#ifdef USE_DWORD_COLOR_EXP
					     SCANLINE_PAD_DWORD |
#else
					     SCANLINE_NO_PAD |
#endif
					     CPU_TRANSFER_PAD_QWORD |
					     BIT_ORDER_IN_BYTE_MSBFIRST);
	xf86AccelInfoRec.CPUToScreenColorExpandBase =
	    (unsigned int *)(I740MMIOBase + BLTDATA);
	xf86AccelInfoRec.CPUToScreenColorExpandRange = 0x10000;
	xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
	    I740SetupForCPUToScreenColorExpand;
	xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
	    I740SubsequentCPUToScreenColorExpand;
#ifndef ALLOW_PCI_COLOR_EXP
    }
#endif

    /* Pixmap cache */
     if (I740CursorStart) {
	 /* Start pixmap cache after the end of the 4KB cursor area. */
	 xf86AccelInfoRec.PixmapCacheMemoryStart = I740CursorStart +
	     ceil((double)4096/(vga256InfoRec.displayWidth*vgaBytesPerPixel)) *
	     vga256InfoRec.displayWidth * vgaBytesPerPixel;
     } else {
	 xf86AccelInfoRec.PixmapCacheMemoryStart =
	     vga256InfoRec.virtualY * vga256InfoRec.displayWidth *
	     vgaBytesPerPixel;
     }
     xf86AccelInfoRec.PixmapCacheMemoryEnd =
	 vga256InfoRec.videoRam * 1024;
}

static void
I740Sync() {
    WAIT_ENGINE_IDLE();
}

static void
I740SetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
    bltcmd.BR00 = (((vga256InfoRec.displayWidth * vgaBytesPerPixel) << 16) |
		   (vga256InfoRec.displayWidth * vgaBytesPerPixel));
    bltcmd.BR01 = color;
    bltcmd.BR04 = SOLID_PAT_SELECT | PAT_IS_MONO | i740PatternRop[rop];
}

static void
I740SubsequentFillRectSolid(x, y, w, h)
    int x, y, w, h;
{
    WAIT_LP_FIFO(12);
    OUTREG(LP_FIFO, 0x6000000A);
    OUTREG(LP_FIFO, bltcmd.BR00);
    OUTREG(LP_FIFO, bltcmd.BR01);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, bltcmd.BR04);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, (y * vga256InfoRec.displayWidth + x) * vgaBytesPerPixel);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, (h << 16) | (w * vgaBytesPerPixel));
}

static void
I740SetupForScreenToScreenCopy(xdir, ydir, rop, planemask, transparency_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int transparency_color;
{
    bltcmd.BR00 = (((vga256InfoRec.displayWidth * vgaBytesPerPixel) << 16) |
		 (vga256InfoRec.displayWidth * vgaBytesPerPixel));

    bltcmd.BR04 = SRC_IS_IN_COLOR | SRC_USE_SRC_ADDR | i740Rop[rop];
    if (xdir == -1)
	bltcmd.BR04 |= BLT_RIGHT_TO_LEFT;
    else
	bltcmd.BR04 |= BLT_LEFT_TO_RIGHT;

    if (ydir == -1)
	bltcmd.BR04 |= BLT_BOT_TO_TOP;
    else
	bltcmd.BR04 |= BLT_TOP_TO_BOT;

    if (transparency_color != -1) {
	bltcmd.BR01 = transparency_color;
	bltcmd.BR04 |= (COLOR_TRANSP_ENABLE |
			COLOR_TRANSP_ROP |
			COLOR_TRANSP_EQ);
    } else {
	bltcmd.BR01 = 0x00000000;
    }
}

static void
I740SubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
    int x1, y1, x2, y2, w, h;
{
    if (bltcmd.BR04 & BLT_BOT_TO_TOP) {
	bltcmd.BR06 = (y1 + h - 1) *
	    vga256InfoRec.displayWidth * vgaBytesPerPixel;
	bltcmd.BR07 = (y2 + h - 1) *
	    vga256InfoRec.displayWidth * vgaBytesPerPixel;
    } else {
	bltcmd.BR06 = y1 * vga256InfoRec.displayWidth * vgaBytesPerPixel;
	bltcmd.BR07 = y2 * vga256InfoRec.displayWidth * vgaBytesPerPixel;
    }

    if (bltcmd.BR04 & BLT_RIGHT_TO_LEFT) {
	bltcmd.BR06 += (x1 + w - 1) * vgaBytesPerPixel + vgaBytesPerPixel - 1;
	bltcmd.BR07 += (x2 + w - 1) * vgaBytesPerPixel + vgaBytesPerPixel - 1;
    } else {
	bltcmd.BR06 += x1 * vgaBytesPerPixel;
	bltcmd.BR07 += x2 * vgaBytesPerPixel;
    }

    WAIT_LP_FIFO(12);
    OUTREG(LP_FIFO, 0x6000000A);
    OUTREG(LP_FIFO, bltcmd.BR00);
    OUTREG(LP_FIFO, bltcmd.BR01);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, bltcmd.BR04);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, bltcmd.BR06);
    OUTREG(LP_FIFO, bltcmd.BR07);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, (h << 16) | (w * vgaBytesPerPixel));
}

static void
I740SetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, 
				      rop, planemask)
    unsigned int patternx, patterny;
    int bg, fg, rop;
    unsigned int planemask;
{
    bltcmd.BR00 = (((vga256InfoRec.displayWidth * vgaBytesPerPixel) << 16) |
		 (vga256InfoRec.displayWidth * vgaBytesPerPixel));

    bltcmd.BR01 = bg;
    bltcmd.BR02 = fg;

    bltcmd.BR04 = PAT_IS_MONO | i740PatternRop[rop];
    if (bg == -1) bltcmd.BR04 |= MONO_PAT_TRANSP;

    bltcmd.BR05 = ((patternx >> 3) +
		 patterny * vga256InfoRec.displayWidth * vgaBytesPerPixel);
}

static void
I740Subsequent8x8PatternColorExpand(patternx, patterny, x, y, w, h)
    unsigned int patternx, patterny;
    int x, y, w, h;
{
    WAIT_LP_FIFO(12);
    OUTREG(LP_FIFO, 0x6000000A);
    OUTREG(LP_FIFO, bltcmd.BR00);
    OUTREG(LP_FIFO, bltcmd.BR01);
    OUTREG(LP_FIFO, bltcmd.BR02);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, bltcmd.BR04 | ((y << 20) & PAT_VERT_ALIGN));
    OUTREG(LP_FIFO, bltcmd.BR05);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, (y * vga256InfoRec.displayWidth + x) * vgaBytesPerPixel);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, (h << 16) | (w * vgaBytesPerPixel));
}

static void
I740SetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned int planemask;
{
    bltcmd.BR00 = (vga256InfoRec.displayWidth * vgaBytesPerPixel) << 16;
    bltcmd.BR01 = bg;
    bltcmd.BR02 = fg;
#ifdef USE_DWORD_COLOR_EXP
    bltcmd.BR03 = MONO_DWORD_ALIGN | MONO_USE_COLEXP;
#else
    bltcmd.BR03 = MONO_BIT_ALIGN | MONO_USE_COLEXP;
#endif
    bltcmd.BR04 = SRC_IS_MONO | SRC_USE_BLTDATA | i740Rop[rop];
    if (bg == -1) bltcmd.BR04 |= MONO_SRC_TRANSP;
}

static void
I740SubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
    int x, y, w, h, skipleft;
{
    WAIT_ENGINE_IDLE();
    OUTREG(LP_FIFO, 0x6000000A);
    OUTREG(LP_FIFO, bltcmd.BR00);
    OUTREG(LP_FIFO, bltcmd.BR01);
    OUTREG(LP_FIFO, bltcmd.BR02);
    OUTREG(LP_FIFO, bltcmd.BR03 | (skipleft & MONO_SRC_LEFT_CLIP));
    OUTREG(LP_FIFO, bltcmd.BR04);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, (y * vga256InfoRec.displayWidth + x) * vgaBytesPerPixel);
    OUTREG(LP_FIFO, 0x00000000);
    OUTREG(LP_FIFO, 0x00000000);
#ifdef USE_DWORD_COLOR_EXP
    /*
     * This extra wait is necessary to keep the bitblt engine from
     * locking up, but I am not sure why it is needed.  If we take it
     * out, "x11perf -copyplane10" will lock the bitblt engine.  When
     * the bitblt engine is locked, it is waiting for mono data to be
     * written to the BLTDATA region, which seems to imply that some of
     * the data that was written was lost.  This might be fixed by
     * BLT_SKEW changes.  Update: The engine still locks up with this
     * extra wait.  More investigation (and time) is needed.
     */
    WAIT_BLT_IDLE();
#endif
    OUTREG(LP_FIFO, (h << 16) | (w * vgaBytesPerPixel));
}
