/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/cyrix/cyrix_accel.c,v 1.1.2.6 1999/06/23 12:37:21 hohndel Exp $ */

/*
 * Copyright 1999 by Brian Falardeau.
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

/* The following ROPs only use pattern and destination data. */
/* They are used when the planemask specifies all planes (no mask). */

static const int windowsROPpat[16] = { 
	0x00, /* GXclear = 0 */ 
	0xA0, /* GXand = pat AND dst */ 
	0x50, /* GXandReverse = pat AND NOT dst */
	0xF0, /* GXcopy = pat */ 
	0x0A, /* GXandInverted = NOT pat AND dst */ 
	0xAA, /* GXnoop = dst */ 
	0x5A, /* GXxor = pat XOR dst */  
	0xFA, /* GXor = pat OR dst */
	0x05, /* GXnor = NOT pat AND NOT dst */ 
	0xA5, /* GXequiv = NOT pat XOR dst */ 
	0x55, /* GXinvert = NOT dst */ 
	0xF5, /* GXorReverse = pat OR NOT dst */ 
	0x0F, /* GXcopyInverted = NOT pat */ 
	0xAF, /* GXorInverted = NOT pat OR dst */ 
	0x5F, /* GXnand = NOT pat OR NOT dst */ 
	0xFF, /* GXset = 1 */ 
};

/* The following ROPs use source data to specify a planemask. */
/* If the planemask (src) is one, then the result is the appropriate */
/* combination of pattern and destination data.  If the planemask (src) */
/* is zero, then the result is always just destination data. */

static const int windowsROPsrcMask[16] = { 
	0x22, /* GXclear => 0 if src = 1, dst if src = 0 */ 
	0xA2, /* GXand = pat AND dst if src = 1, dst if src = 0 */
	0x62, /* GXandReverse = pat AND NOT dst if src = 1, dst if src = 0 */
	0xE2, /* GXcopy = pat if src = 1, dst if src = 0 */ 
 	0x2A, /* GXandInverted = NOT pat AND dst if src = 1, dst if src = 0 */ 
	0xAA, /* GXnoop = dst if src = 1, dst if src = 0 */
	0x6A, /* GXxor = pat XOR dst if src = 1, dst if src = 0 */
	0xEA, /* GXor = pat OR dst if src = 1, dst if src = 0 */
    0x26, /* GXnor = NOT pat AND NOT dst if src = 1, dst if src = 0 */
	0xA6, /* GXequiv = NOT pat XOR dst if src = 1, dst if src = 0 */
	0x66, /* GXinvert = NOT dst if src = 1, dst if src = 0 */
	0xE6, /* GXorReverse = pat OR NOT dst if src = 1, dst if src = 0 */
    0x2E, /* GXcopyInverted = NOT pat if src = 1, dst if src = 0 */
	0xAE, /* GXorInverted = NOT pat OR dst if src = 1, dst if src = 0 */
	0x6E, /* GXnand = NOT pat OR NOT dst if src = 1, dst if src = 0 */
	0xEE, /* GXset = 1 if src = 1, dst if src = 0 */
};

/* The following ROPs use pattern data to specify a planemask. */
/* If the planemask (pat) is one, then the result is the appropriate */
/* combination of source and destination data.  If the planemask (pat) */
/* is zero, then the result is always just destination data. */

static const int windowsROPpatMask[16] = { 0x0A, 0x8A, 0x4A, 0xCA,
                                           0x2A, 0xAA, 0x6A, 0xEA,
                                           0x1A, 0x9A, 0x5A, 0xDA,
                                           0x3A, 0xBA, 0x7A, 0xFA };



static int blitMode;
static int vectorMode;
static int transMode;
static int copyXdir;
static int setBlitModeOnSync = 0;

/* STATIC VARIABLES FOR THIS FILE */
/* Used to maintain state between setup and rendering calls. */

static int CYRIXsavedROP;
static int CYRIXtransparent;
static int CYRIXtransColor;
static int CYRIXstartMonoExpand = 0;

static unsigned short CYRIXsaveX, CYRIXsaveY, CYRIXsaveW, CYRIXsaveH;

/* Forward declaration of functions used in the driver */

void CYRIXAccelSync();
void CYRIXAccelInit();
void CYRIXSetupForFillRectSolid();
void CYRIXSubsequentFillRectSolid();
void CYRIXSetupFor8x8PatternColorExpand();
void CYRIXSubsequent8x8PatternColorExpand();
void CYRIXSetupForScreenToScreenCopy();
void CYRIXSubsequentScreenToScreenCopy();

void CYRIXSubsequentBresenhamLine();
void CYRIXSetupForCPUToScreenColorExpand();
void CYRIXSubsequentCPUToScreenColorExpand();

/* Routines in GXRENDER.C */

void gxr_initialize(unsigned char *regptr, unsigned short bpp, 
	unsigned short BB0base, unsigned short BB1base, 
	unsigned short BBwidthPixels);

void gxr_wait_until_idle(void);

void gxr_load_solid_source(unsigned short color);

void gxr_load_mono_source(unsigned short bgcolor, unsigned short fgcolor,
	unsigned short transparent);

void gxr_load_solid_pattern(unsigned short color);

void gxr_load_mono_pattern(unsigned short bgcolor, unsigned short fgcolor, 
	unsigned long data0, unsigned long data1, unsigned char transparency);

void gxr_load_raster_operation(unsigned char rop);

void gxr_pattern_fill(unsigned short x, unsigned short y, 
	unsigned short width, unsigned short height);

void gxr_screen_to_screen_blt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height);

void gxr_screen_to_screen_xblt(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned short color);

void gxr_text_glyph(unsigned short srcx, unsigned short srcy,
	unsigned short dstx, unsigned short dsty, unsigned short width, 
	unsigned short height, unsigned char *data, unsigned short pitch); 

void gxr_bresenham_line(unsigned short x, unsigned short y, 
	unsigned short length, unsigned short initerr, 
	unsigned short axialerr, unsigned short diagerr, 
	unsigned short flags);

/*
//---------------------------------------------------------------------------
// CYRIXAccelInit
//
// This routine hooks the acceleration routines and sets appropriate flags.
//---------------------------------------------------------------------------
*/

void 
CYRIXAccelInit() 
{
	int bltBufWidth;

	/* General acceleration flags */
	xf86AccelInfoRec.Flags = PIXMAP_CACHE
	                       | BACKGROUND_OPERATIONS
	                       | HARDWARE_PATTERN_SCREEN_ORIGIN
	                       | HARDWARE_PATTERN_BIT_ORDER_MSBFIRST
	                       | HARDWARE_PATTERN_PROGRAMMED_BITS
	                       | HARDWARE_PATTERN_MONO_TRANSPARENCY;

	/* Sync */
	xf86AccelInfoRec.Sync = CYRIXAccelSync;

	/* Filled rectangles */
	xf86AccelInfoRec.SetupForFillRectSolid = 
	    CYRIXSetupForFillRectSolid;
	xf86AccelInfoRec.SubsequentFillRectSolid = 
	    CYRIXSubsequentFillRectSolid;
	xf86GCInfoRec.PolyFillRectSolidFlags = 0;

	/* ScreenToScreen copies */
	xf86AccelInfoRec.SetupForScreenToScreenCopy =
	    CYRIXSetupForScreenToScreenCopy;
	xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	    CYRIXSubsequentScreenToScreenCopy;
	xf86GCInfoRec.CopyAreaFlags = TRANSPARENCY_GXCOPY;

	/* Bresenham lines */
	xf86AccelInfoRec.SubsequentBresenhamLine =
	    CYRIXSubsequentBresenhamLine;
	xf86AccelInfoRec.ErrorTermBits = 15;

	/* 8x8 color-expanded patterns */
	xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
	    CYRIXSetupFor8x8PatternColorExpand;
	xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
	    CYRIXSubsequent8x8PatternColorExpand;

	/* Color expansion */
	xf86AccelInfoRec.ColorExpandFlags = BIT_ORDER_IN_BYTE_MSBFIRST |
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

	/* pass parameters to GXRENDER.C file */

	gxr_initialize((unsigned char *) GXregisters, 
		(unsigned short) vgaBitsPerPixel,
		(unsigned short) CYRIXbltBuf0Address, 
		(unsigned short) CYRIXbltBuf1Address, 
		(unsigned short) bltBufWidth);
} 

/*
//---------------------------------------------------------------------------
// CYRIXAccelSync
//
// This routine is called before accessing the frame buffer directly to 
// make sure that the graphics pipeline is idle.  It is also called after
// loading the monochrome data into BB0 for bitmap to screen BLTs.
//---------------------------------------------------------------------------
*/

void
CYRIXAccelSync()
{	
	/* CHECK IF END TO CPU TO SCREEN EXPAND BLT */

	if (CYRIXstartMonoExpand)
	{	
		/* START CPU TO SCREEN EXPAND BLT */
		/* Data has already been loaded into BB0, so use NULL pointer. */

		/* this is formally incorrect: XAA may use both BB0 and BB1
		   for the text source bitmap, so READ_DST_FB1 should not be
	       used.  So far, this problem has not manifested itself in
	       practice. */

		CYRIXstartMonoExpand = 0;
		gxr_text_glyph(0, 0, CYRIXsaveX, CYRIXsaveY, CYRIXsaveW, 
			CYRIXsaveH, 0, 0);
	}
	
	/* WAIT UNTIL IDLE */

	gxr_wait_until_idle();
}


/*
//---------------------------------------------------------------------------
// CYRIXSetupForFillRectSolid
//
// This routine is called to setup the solid pattern color for future
// rectangular fills or vectors.
//---------------------------------------------------------------------------
*/

void 
CYRIXSetupForFillRectSolid(color, rop, planemask)
int color, rop;
unsigned int planemask;
{
	/* LOAD THE SOLID PATTERN COLOR */

	gxr_load_solid_pattern((unsigned short) color);

	/* CHECK IF PLANEMASK IS NOT USED (ALL PLANES ENABLED) */

	if (planemask == (unsigned int) -1)
	{
		/* use normal pattern ROPs if all planes are enabled */

		gxr_load_raster_operation(windowsROPpat[rop & 0x0F]);
	}
	else
	{
		/* select ROP that uses planemask in src data */

		gxr_load_solid_source((unsigned short) planemask);
		gxr_load_raster_operation(windowsROPsrcMask[rop & 0x0F]);
	}
}
    
/*
//---------------------------------------------------------------------------
// CYRIXSubsequentFillRectSolid
//
// This routine is called to fill a rectangular region using the previously
// specified solid pattern and raster operation.  
//
// Sample application uses:
//   - Window backgrounds. 
//   - x11perf: rectangle tests (-rect500).
//   - x11perf: fill trapezoid tests (-trap100).
//   - x11perf: horizontal line segments (-hseg500).
//---------------------------------------------------------------------------
*/
    
void 
CYRIXSubsequentFillRectSolid(x, y, w, h)
int x, y, w, h;
{
	/* call routine to fill rectangular region */

	gxr_pattern_fill((unsigned short) x, (unsigned short) y, 
		(unsigned short) w, (unsigned short) h);
}

/*
//---------------------------------------------------------------------------
// CYRIXSetupFor8x8PatternColorExpand
//
// This routine is called to setup the monochrome pattern (8x8) and raster 
// operation for future rectangular fills.
//---------------------------------------------------------------------------
*/

void CYRIXSetupFor8x8PatternColorExpand(patternx, patterny, bg, fg, rop, planemask)
int patternx, patterny;
int bg, fg, rop;
unsigned int planemask;
{	int trans = (bg == -1);

	/* LOAD PATTERN COLORS AND DATA */

	gxr_load_mono_pattern((unsigned short) bg, (unsigned short) fg,
		(unsigned long) patternx, (unsigned long) patterny, 
		(unsigned char) trans);

	/* CHECK IF PLANEMASK IS NOT USED (ALL PLANES ENABLED) */

	if (planemask == (unsigned int) -1)
	{
		/* use normal pattern ROPs if all planes are enabled */

		gxr_load_raster_operation(windowsROPpat[rop & 0x0F]);
	}
	else
	{
		/* select ROP that uses planemask in src data */

		gxr_load_solid_source((unsigned short) planemask);
		gxr_load_raster_operation(windowsROPsrcMask[rop & 0x0F]);
	}
}

/*
//---------------------------------------------------------------------------
// CYRIXSubsequent8x8PatternColorExpand
//
// This routine is called to fill a rectangular region using the previously
// specified monochrome pattern (8x8) and raster operation.
//
// Sample application uses:
//   - Patterned desktops
//   - x11perf: stippled rectangle tests (-srect500).
//   - x11perf: opaque stippled rectangle tests (-osrect500).
//---------------------------------------------------------------------------
*/

void CYRIXSubsequent8x8PatternColorExpand(patternx, patterny, x, y, w, h)
int patternx, patterny;
int x, y, w, h;
{
	/* call routine to fill rectangular region */

	gxr_pattern_fill((unsigned short) x, (unsigned short) y, 
		(unsigned short) w, (unsigned short) h);
}

/*
//---------------------------------------------------------------------------
// CYRIXSetupForScreenToScreenCopy
//
// This routine is called to setup the planemask and raster operation 
// for future screen to screen BLTs.
//---------------------------------------------------------------------------
*/

void 
CYRIXSetupForScreenToScreenCopy(xdir, ydir, rop, planemask, transparency_color)
int xdir, ydir;
int rop;
unsigned int planemask;
int transparency_color;
{
	/* LOAD PLANEMASK INTO PATTERN DATA */

	gxr_load_solid_pattern((unsigned short) planemask);
	
	/* SET RASTER OPERATION FOR USING PATTERN AS PLANE MASK */

	gxr_load_raster_operation(windowsROPpatMask[rop & 0x0F]);

	/* SAVE TRANSPARENCY FLAG */

	CYRIXtransparent = (transparency_color == -1) ? 0 : 1;
	CYRIXtransColor = transparency_color;
}

/*
//---------------------------------------------------------------------------
// CYRIXSubsequentScreenToScreenCopy
//
// This routine is called to perform a screen to screen BLT using the 
// previously specified planemask, raster operation, and transparency flag.
//
// Sample application uses (non-transparent):
//   - Moving windows.
//   - x11perf: scroll tests (-scroll500).
//   - x11perf: copy from window to window (-copywinwin500).
//
// No application found using transparency.
//---------------------------------------------------------------------------
*/

void 
CYRIXSubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
int x1, y1, x2, y2, w, h;
{
	if (CYRIXtransparent)
	{
		/* CALL ROUTINE FOR TRANSPARENT SCREEN TO SCREEN BLT */
		/* Should only be called for the "copy" raster operation. */

		gxr_screen_to_screen_xblt(
			(unsigned short) x1, (unsigned short) y1, 
			(unsigned short) x2, (unsigned short) y2, 
			(unsigned short) w, (unsigned short) h, 
			(unsigned short) CYRIXtransColor);
	}
	else
	{
		/* CALL ROUTINE FOR NORMAL SCREEN TO SCREEN BLT */

		gxr_screen_to_screen_blt(
			(unsigned short) x1, (unsigned short) y1, 
			(unsigned short) x2, (unsigned short) y2, 
			(unsigned short) w, (unsigned short) h);
	}
}

/*
//---------------------------------------------------------------------------
// CYRIXSubsequentBresenhamLine
//
// This routine is called to render a vector using the specified Bresenham
// parameters.  
//
// Sample application uses:
//   - Window outlines on window move.
//   - x11perf: line segments (-seg500).
//---------------------------------------------------------------------------
*/

void
CYRIXSubsequentBresenhamLine(x1, y1, octant, err, e1, e2, length)
int x1, y1, octant, err, e1, e2, length;
{	
	unsigned short flags;

	/* DETERMINE YMAJOR AND DIRECTION FLAGS */

	if (octant & YMAJOR)
	{	flags = VM_Y_MAJOR;
		if (!(octant & XDECREASING)) flags |= VM_MINOR_INC;
		if (!(octant & YDECREASING)) flags |= VM_MAJOR_INC;
	}
	else
	{	flags = VM_X_MAJOR;
		if (!(octant & XDECREASING)) flags |= VM_MAJOR_INC;
		if (!(octant & YDECREASING)) flags |= VM_MINOR_INC;
	}

	/* CALL ROUTINE TO DRAW VECTOR */

	gxr_bresenham_line((unsigned short) x1, (unsigned short) y1, 
		(unsigned short) length, (unsigned short) err, 
		(unsigned short) e1, (unsigned short) e2, (unsigned short) flags);	
}

/*
//---------------------------------------------------------------------------
// CYRIXSetupForCPUToScreenColorExpand
//
// This routine is called to setup the planemask, colors, and raster 
// operation for future monocrome bitmap to screen BLTs.
//---------------------------------------------------------------------------
*/

void CYRIXSetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
int bg, fg, rop;
unsigned int planemask;
{	int trans = (bg == -1);

	/* LOAD SOURCE COLORS */

	gxr_load_mono_source((unsigned short) bg, (unsigned short) fg, 
		(unsigned short) trans);

	/* LOAD PLANEMASK INTO PATTERN DATA */

	gxr_load_solid_pattern((unsigned short) planemask);
	
	/* SET RASTER OPERATION FOR USING PATTERN AS PLANE MASK */

	gxr_load_raster_operation(windowsROPpatMask[rop & 0x0F]);
}

/*
//---------------------------------------------------------------------------
// CYRIXSubsequentCPUToScreenColorExpand
//
// This routine is called to render expanded monocrome bitmap data to the
// screen using the previously specified colors and raster operation.  Since
// the server loads the monochrome data into BB0, not the driver, this 
// routine just sets a flag and saves the parameters to use when the server
// is done loading the data and calls the CYRIXAccelSync function. 
//
// Sample application uses:
//   - Text in windows.
//   - x11perf: text (-ftext, -f8text, -f9text, ...).
//---------------------------------------------------------------------------
*/

void CYRIXSubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
int x, y, w, h;
int skipleft;
{
	CYRIXstartMonoExpand = 1;
	CYRIXsaveX = x;
	CYRIXsaveY = y;
	CYRIXsaveW = w;
	CYRIXsaveH = h;

	/* WAIT UNTIL IDLE BEFORE ALLOWING WRITES TO BLT BUFFERS */
	/* Server will load the monochrome data into BB0 after this. */

	gxr_wait_until_idle();
}

/* END OF FILE */

