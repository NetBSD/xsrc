/*
 * Copyright 1997,1998 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/spc8110/spc_accel.c,v 1.1.2.1 1998/10/18 20:42:38 hohndel Exp $ */

#include "vga256.h"
#include "compiler.h"
#include "xf86.h"
#include "vga.h"
#include "xf86xaa.h"
#include "spc_driver.h"

static void SPCSync();
static void SPCSetupForScreenToScreenCopy();
static void SPCSubsequentScreenToScreenCopy();
static void SPCSetupForFillRectSolid();
static void SPCSubsequentFillRectSolid();

/* #define XAA_DEBUG	/* */

void
SPCAccelInit()
{
    xf86AccelInfoRec.Flags = 
	PIXMAP_CACHE | GXCOPY_ONLY | NO_PLANEMASK |
	ONLY_TWO_BITBLT_DIRECTIONS;

    /* Wait for Accelerator to finish the current operation */
    xf86AccelInfoRec.Sync = SPCSync;

    /* Accelerated filled rectangles */
    xf86GCInfoRec.PolyFillRectSolidFlags = GXCOPY_ONLY | NO_PLANEMASK;
    xf86AccelInfoRec.SetupForFillRectSolid = SPCSetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = SPCSubsequentFillRectSolid;

    /* Accelerated screen-screen bitblts */
    xf86GCInfoRec.CopyAreaFlags = 
    	GXCOPY_ONLY | NO_PLANEMASK | NO_TRANSPARENCY;
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
	SPCSetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	SPCSubsequentScreenToScreenCopy;

    /* Pixmap cache setup */
    xf86AccelInfoRec.PixmapCacheMemoryStart =
	vga256InfoRec.virtualY * vga256InfoRec.displayWidth
	* vga256InfoRec.bitsPerPixel / 8;
    xf86AccelInfoRec.PixmapCacheMemoryEnd =
	vga256InfoRec.videoRam * 1024 - 1024;
}

#define SYNC()    while (inb(0x3d0) & 0x80)

static void
SPCSync()
{
    while (inb(0x3d0) & 0x80)
    	;
}

static int blitdir;
static int foreground;
static int background;
static int transparency;

void SPCSetupForScreenToScreenCopy(int xdir, int ydir, int rop,
				unsigned int planemask,
				int transparency_color)
{
    if (xdir == 1 && ydir == 1) {
	blitdir = DOWN_RIGHT;
    } else if (xdir == -1 && ydir == -1) {
	blitdir = UP_LEFT;
    }

    SYNC();
    outl(0x3d0, blitdir | 4);
    outl(0x3d0, 0x1005);	/* auto-start */
}

static void
SPCSubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h)
{
    unsigned long srcaddr;
    unsigned long destaddr;

    --w;
    --h;
    if (blitdir == DOWN_RIGHT) {
	srcaddr = y1 * vga256InfoRec.displayWidth + x1;
	destaddr = y2 * vga256InfoRec.displayWidth + x2;
    } else {
	srcaddr = (y1 + h) * vga256InfoRec.displayWidth + x1 + w;
	destaddr = (y2 + h) * vga256InfoRec.displayWidth + x2 + w;
    }

    SYNC();
    outl(0x3d0, (srcaddr << 8) | 0);
    outl(0x3d0, (destaddr << 8) | 1);
    outl(0x3d0, (h << 16) | 2);
    /* writing width starts the blit */
    outl(0x3d0, (w << 16) | 3);
}

static void
SPCSetupForFillRectSolid(int color, int rop, unsigned int planemask)
{
    foreground = color << 8;

    SYNC();
    outl(0x3d0, 0x204);
    outl(0x3d0, 0x1005);	/* auto-start */
}

static void
SPCSubsequentFillRectSolid(int x, int y, int w, int h)
{
    unsigned long pdst = (y * vga256InfoRec.displayWidth) + x;

    --w;
    --h;
    SYNC();
    outl(0x3d0, (pdst << 8) | 1);
    outl(0x3d0, (h << 16) | 2);
    /* writing width starts the blit */
    outl(0x3d0, (w << 16) | foreground | 3);
}


