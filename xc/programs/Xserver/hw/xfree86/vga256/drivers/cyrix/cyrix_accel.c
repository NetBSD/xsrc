/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_accel.c,v 1.1.2.5 1998/12/22 07:49:58 hohndel Exp $ */

/*
 * Copyright 1998 by Annius Groenink, Amsterdam.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Annius Groenink not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Annius Groenink makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ANNIUS GROENINK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <math.h>
#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "xf86xaa.h"
#include "xf86_Config.h"
#include "xf86_OSlib.h"
#include "cyrix.h"
#include "miline.h"


/* size of color expand source area (embedded in frame buffer) */
#define CYRIXexpandSize       32768

/* Raster operations are converted in such a way that we can use them to
   do planemask operations: lower nybble is NOP (pattern=planemask),
   upper nybble inverted X raster operation (bits 0 - 3 correspond to
   bits 3 - 0 and 7 - 4 in Windows style ROP).  In some routines,
   the role of source and pattern is inverted. */
static const int windowsROPpatMask[16] = { 0x0A, 0x8A, 0x4A, 0xCA,
                                           0x2A, 0xAA, 0x6A, 0xEA,
                                           0x1A, 0x9A, 0x5A, 0xDA,
                                           0x3A, 0xBA, 0x7A, 0xFA };

static const int windowsROPsrcMask[16] = { 0x22, 0xA2, 0x62, 0xE2,
                                           0x2A, 0xAA, 0x6A, 0xEA,
                                           0x26, 0xA6, 0x66, 0xE6,
                                           0x2E, 0xAE, 0x6E, 0xEE };

static int bltBufWidth;

static int blitMode;
static int vectorMode;
static int transMode;
static int copyXdir;
static int setBlitModeOnSync = 0;


/* Forward declaration of functions used in the driver */
void CYRIXAccelSync();
void CYRIXAccelInit();
void CYRIXSetupForFillRectSolid();
void CYRIXSubsequentFillRectSolid();
void CYRIXSetupForScreenToScreenCopy();
void CYRIXSubsequentScreenToScreenCopy();
void CYRIXSubsequentBresenhamLine();
void CYRIXSetupFor8x8PatternColorExpand();
void CYRIXSubsequent8x8PatternColorExpand();
void CYRIXSetupForCPUToScreenColorExpand();
void CYRIXSubsequentCPUToScreenColorExpand();


/* Acceleration init function, sets up pointers to our accelerated functions */
void 
CYRIXAccelInit() 
{	/* General acceleration flags */
	xf86AccelInfoRec.Flags = PIXMAP_CACHE
	                       | BACKGROUND_OPERATIONS
#if 0
	                       | HARDWARE_PATTERN_MONO_TRANSPARENCY
#endif
	                       | HARDWARE_PATTERN_SCREEN_ORIGIN
	                       | HARDWARE_PATTERN_BIT_ORDER_MSBFIRST
	                       | HARDWARE_PATTERN_PROGRAMMED_BITS;

	/* Sync */
	xf86AccelInfoRec.Sync = CYRIXAccelSync;

	/* Filled rectangles */
	xf86AccelInfoRec.SetupForFillRectSolid = 
	    CYRIXSetupForFillRectSolid;
	xf86AccelInfoRec.SubsequentFillRectSolid = 
	    CYRIXSubsequentFillRectSolid;
	xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;

	/* ScreenToScreen copies */
	xf86AccelInfoRec.SetupForScreenToScreenCopy =
	    CYRIXSetupForScreenToScreenCopy;
	xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	    CYRIXSubsequentScreenToScreenCopy;

	xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK | GXCOPY_ONLY;

#if 0
	/* Bresenham lines - disable because of minor display errors */
	xf86AccelInfoRec.SubsequentBresenhamLine =
	    CYRIXSubsequentBresenhamLine;
	xf86AccelInfoRec.ErrorTermBits = 15;
#endif

	/* 8x8 color-expanded patterns */
	xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
	    CYRIXSetupFor8x8PatternColorExpand;
	xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
	    CYRIXSubsequent8x8PatternColorExpand;

	/* Color expansion */
	xf86AccelInfoRec.ColorExpandFlags = BIT_ORDER_IN_BYTE_MSBFIRST |
	                                    NO_PLANEMASK |
	                                    TRANSPARENCY_GXCOPY |
	                                    SCANLINE_PAD_BYTE;

	/* Use two blit buffers in a row for text expansion
	   (this is an undefendable fix to a text display distortion
	   bug if we don't give XAA enough room, but the only thing that
	   seems to make it work properly) */
	xf86AccelInfoRec.CPUToScreenColorExpandBase =
	    (unsigned int*)(GXregisters + CYRIXbltBuf0Address);
	xf86AccelInfoRec.CPUToScreenColorExpandRange =
	    CYRIXbltBufSize * 2;

	xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
	    CYRIXSetupForCPUToScreenColorExpand;
	xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
	    CYRIXSubsequentCPUToScreenColorExpand;

	/* set up the video memory space available to the pixmap cache */
	xf86InitPixmapCache(&vga256InfoRec,
	    CYRIXoffscreenAddress,
	    CYRIXoffscreenAddress + CYRIXoffscreenSize);

	/* calculate the pixel width of a blit buffer for convenience */
	bltBufWidth = CYRIXbltBufSize / (vgaBitsPerPixel / 8);
} 


/* set colors - called through access macros in cyrix.h */
static __inline__ void CYRIXsetColors01(reg, col0, col1)
int reg;
int col0;
int col1;
{	if (vgaBitsPerPixel == 16)
		GX_REG(reg) = ((col1 & 0xFFFF) << 16) | (col0 & 0xFFFF);
	else
	{	col0 &= 0xFF;
		col1 &= 0xFF;
		GX_REG(reg) = (col1 << 24) | (col1 << 16) | (col0 << 8) | col0;
}	}


/* The generic Sync() function that waits for everything to
   be completed (e.g. before writing to the frame buffer
   directly). */
void
CYRIXAccelSync()
{	if (setBlitModeOnSync)
	{	setBlitModeOnSync = 0;
		CYRIXsetupSync();
		CYRIXsetBlitMode();
	}
	while (GX_REG(GP_BLIT_STATUS) &
	       (BS_BLIT_BUSY|BS_PIPELINE_BUSY|BS_BLIT_PENDING));
}


/* Solid rectangles */
void 
CYRIXSetupForFillRectSolid(color, rop, planemask)
int color, rop;
unsigned int planemask;
{	if (xf86GCInfoRec.PolyFillRectSolidFlags & NO_PLANEMASK)
		planemask = 0xFFFF;
	if (xf86GCInfoRec.PolyFillRectSolidFlags & GXCOPY_ONLY)
		rop = GXcopy;

	CYRIXsetupSync();
	CYRIXsetSourceColors01(color, color);
	CYRIXsetPatColors01(planemask, 0);
	CYRIXsetPatMode(rop, RM_PAT_DISABLE);
	blitMode = BM_READ_SRC_NONE | BM_WRITE_FB | BM_SOURCE_EXPAND
	         | IfDest(rop, planemask, BM_READ_DST_FB0);
	vectorMode = IfDest(rop, planemask, VM_READ_DST_FB);
}
    
    
void 
CYRIXSubsequentFillRectSolid(x, y, w, h)
int x, y, w, h;
{	/* divide the operation into columns if required; use twice the
           blit buffer width because buffer 0 will overflow into buffer 1 */
	while (w > 2 * bltBufWidth)
	{	CYRIXSubsequentFillRectSolid(x, y, 2 * bltBufWidth, h);
		x += 2 * bltBufWidth;
		w -= 2 * bltBufWidth;
	}
	CYRIXsetupSync();
	CYRIXsetDstXY(x, y);
	CYRIXsetWH(w, h);
	CYRIXsetBlitMode();
}


/* Screen to screen copies */
void 
CYRIXSetupForScreenToScreenCopy(xdir, ydir, rop, planemask, transparency_color)
int xdir, ydir;
int rop;
unsigned int planemask;
int transparency_color;
{	if (xf86GCInfoRec.CopyAreaFlags & NO_PLANEMASK)
		planemask = 0xFFFF;
	if (xf86GCInfoRec.CopyAreaFlags & GXCOPY_ONLY)
		rop = GXcopy;
	if (xf86GCInfoRec.CopyAreaFlags & NO_TRANSPARENCY)
		transparency_color = -1;

	CYRIXsetupSync();
	CYRIXsetPatColors01(planemask, 0);

	if (transparency_color == -1)
	{	CYRIXsetPatMode(rop, RM_PAT_DISABLE);
		transMode = 0;
	}
	else
	{	CYRIXsetPatModeTrans(RM_PAT_DISABLE);
		transMode = 1;

		if (xf86GCInfoRec.CopyAreaFlags & TRANSPARENCY_GXCOPY)
			rop = GXcopy;

		/* fill blit buffer 1 with the transparency color */
		if (vgaBitsPerPixel == 16)
		{	int              k   = CYRIXbltBufSize / 4;
			CARD32           val = (transparency_color << 16) |
			                       transparency_color;
			volatile CARD32* buf = &(GX_REG(CYRIXbltBuf1Address));

			while (--k >= 0) buf[k] = val;
		}
		else
			memset(GXregisters + CYRIXbltBuf1Address,
			       transparency_color, CYRIXbltBufSize);
	}

	blitMode = BM_READ_SRC_FB | BM_WRITE_FB | BM_SOURCE_COLOR
	         | (transMode ? BM_READ_DST_NONE : IfDest(rop, planemask, BM_READ_DST_FB1))
	         | (ydir < 0 ? BM_REVERSE_Y : 0);

	copyXdir = xdir;
}

void 
CYRIXSubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
int x1, y1, x2, y2, w, h;
{	int up       = (blitMode & BM_REVERSE_Y);

	/* divide the operation into columns when necessary */
	if (copyXdir < 0)
	{	int x_offset = w - bltBufWidth;

		while (x_offset > 0)
		{	CYRIXSubsequentScreenToScreenCopy(x1 + x_offset, y1,
			                                  x2 + x_offset, y2,
			                                  bltBufWidth, h);
			x_offset -= bltBufWidth;
			w -= bltBufWidth;
	}	}
	else while (w > bltBufWidth)
	{	CYRIXSubsequentScreenToScreenCopy(x1, y1, x2, y2,
		                                  bltBufWidth, h);
		x1 += bltBufWidth;
		x2 += bltBufWidth;
		w -= bltBufWidth;
	}

	CYRIXsetupSync();
	CYRIXsetSrcXY(x1, (up ? (y1 + h - 1) : y1));
	CYRIXsetDstXY(x2, (up ? (y2 + h - 1) : y2));

	/* in transparent mode, one line reads the transparency color
	   into a processor-internal register, and the remaining lines
	   can be done in a single second pass */
	if (transMode)
	{	blitMode |= BM_READ_DST_BB1;
		CYRIXsetWH(w, 1);
		CYRIXsetBlitMode();
		h--;
		if (!h) return;
		if (up) { y1--; y2--; }
		else { y1++; y2++; }
		CYRIXsetupSync();
		blitMode &= ~(BM_READ_DST_BB1);
	}
	CYRIXsetWH(w, h);
	CYRIXsetBlitMode();
}


/* Bresenham lines */
void
CYRIXSubsequentBresenhamLine(x1, y1, octant, err, e1, e2, length)
int x1, y1, octant, err, e1, e2, length;
{	if (octant & YMAJOR)
	{	vectorMode = (vectorMode & VM_READ_DST_FB) | VM_Y_MAJOR;
		if (!(octant & XDECREASING)) vectorMode |= VM_MINOR_INC;
		if (!(octant & YDECREASING)) vectorMode |= VM_MAJOR_INC;
	}
	else
	{	vectorMode = (vectorMode & VM_READ_DST_FB) | VM_X_MAJOR;
		if (!(octant & XDECREASING)) vectorMode |= VM_MAJOR_INC;
		if (!(octant & YDECREASING)) vectorMode |= VM_MINOR_INC;
	}

	CYRIXsetupSync();
	CYRIXsetDstXY(x1, y1);
	CYRIXsetWH(length, (err & 0xFFFF));
	CYRIXsetSrcXY((e1 & 0xFFFF), (e2 & 0xFFFF));
	CYRIXsetVectorMode();
}


/* 8x8 pattern color expand */
void CYRIXSetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, rop, planemask)
int patternx, patterny;
int bg, fg, rop;
unsigned int planemask;
{	int trans = (bg == -1);

	if (xf86AccelInfoRec.ColorExpandFlags & NO_PLANEMASK)
		planemask = 0xFFFF;
	if (trans && (xf86AccelInfoRec.ColorExpandFlags & TRANSPARENCY_GXCOPY))
		rop = GXcopy;

	CYRIXsetupSync();
	CYRIXsetSourceColors01(planemask, planemask);
	CYRIXsetPatColors01(trans ? 0 : bg, fg);
	CYRIXsetPatData(patternx, patterny);
	CYRIXsetPatModeX(rop, RM_PAT_MONO | (trans ? RM_PAT_TRANSPARENT : 0));

	blitMode = BM_READ_SRC_NONE | BM_WRITE_FB | BM_SOURCE_EXPAND
	         | (trans ? IfDest(rop, planemask, BM_READ_DST_FB0) : BM_READ_DST_NONE);
}

void CYRIXSubsequent8x8PatternColorExpand(patternx, patterny, x, y, w, h)
int patternx, patterny;
int x, y, w, h;
{	CYRIXSubsequentFillRectSolid(x, y, w, h);
}


/* CPU-to-screen color expansion */
void CYRIXSetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
int bg, fg, rop;
unsigned int planemask;
{	int trans = (bg == -1);

	if (trans && (xf86AccelInfoRec.ColorExpandFlags & TRANSPARENCY_GXCOPY))
		rop = GXcopy;

	CYRIXsetupSync();
	CYRIXsetSourceColors01(trans ? 0 : bg, fg);
	CYRIXsetPatColors01(planemask, 0);

	CYRIXsetPatMode(rop, RM_PAT_DISABLE | (trans ? RM_SRC_TRANSPARENT : 0));

	/* this is formally incorrect: XAA may use both BB0 and BB1
	   for the text source bitmap, so READ_DST_FB1 should not be
	   used.  So far, this problem has not manifested itself in
	   practice. */
	blitMode = BM_READ_SRC_BB0 | BM_WRITE_FB | BM_SOURCE_EXPAND
	         | (trans ? IfDest(rop, planemask, BM_READ_DST_FB1) : BM_READ_DST_NONE);
}

void CYRIXSubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
int x, y, w, h;
int skipleft;
{	CYRIXsetupSync();
	CYRIXsetSrcXY(0, 0);
	CYRIXsetDstXY(x, y);
	CYRIXsetWH(w, h);

	CYRIXAccelSync();
	setBlitModeOnSync = 1;
}

