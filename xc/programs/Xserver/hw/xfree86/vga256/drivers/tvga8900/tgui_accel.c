/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/tvga8900/tgui_accel.c,v 3.6.2.9 1999/06/21 09:45:20 hohndel Exp $ */

/*
 * Copyright 1996 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 * 
 * Trident TGUI accelerated options.
 */

#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "miline.h"
#include "compiler.h"

#include "xf86xaa.h"

extern int TGUIRops_alu[16];
extern int TGUIRops_Pixalu[16];
extern int GE_OP;
extern int revision;
extern Bool SETUPCHIP;
extern Bool ClipOn;
#include "t89_driver.h"
#include "tgui_ger.h"
#include "tgui_mmio.h"

#ifdef TRIDENT_MMIO

#define MMIONAME(x)			x##MMIO

#define TGUISync			MMIONAME(TGUISync)
#define IMAGESync			MMIONAME(IMAGESync)
#define BLADESync			MMIONAME(BLADESync)
#define TGUIAccelInit			MMIONAME(TGUIAccelInit)
#define TGUISetupForFillRectSolid	MMIONAME(TGUISetupForFillRectSolid)
#define IMAGESetupForFillRectSolid	MMIONAME(IMAGESetupForFillRectSolid)
#define BLADESetupForFillRectSolid	MMIONAME(BLADESetupForFillRectSolid)
#define TGUI9685SetupForFillRectSolid	MMIONAME(TGUI9685SetupForFillRectSolid)
#define TGUISubsequentFillRectSolid	MMIONAME(TGUISubsequentFillRectSolid)
#define IMAGESubsequentFillRectSolid	MMIONAME(IMAGESubsequentFillRectSolid)
#define BLADESubsequentFillRectSolid	MMIONAME(BLADESubsequentFillRectSolid)
#define TGUILinearSubsequentFillRectSolid	MMIONAME(TGUILinearSubsequentFillRectSolid)
#define TGUISetClippingRectangle	MMIONAME(TGUISetClippingRectangle)
#define TGUISetupForScreenToScreenCopy	MMIONAME(TGUISetupForScreenToScreenCopy)
#define TGUISubsequentScreenToScreenCopy	MMIONAME(TGUISubsequentScreenToScreenCopy)
#define IMAGESetupForScreenToScreenCopy	MMIONAME(IMAGESetupForScreenToScreenCopy)
#define BLADESetupForScreenToScreenCopy	MMIONAME(BLADESetupForScreenToScreenCopy)
#define IMAGESubsequentScreenToScreenCopy	MMIONAME(IMAGESubsequentScreenToScreenCopy)
#define BLADESubsequentScreenToScreenCopy	MMIONAME(BLADESubsequentScreenToScreenCopy)
#define TGUISubsequentBresenhamLine		MMIONAME(TGUISubsequentBresenhamLine)
#define TGUISetupForCPUToScreenColorExpand	MMIONAME(TGUISetupForCPUToScreenColorExpand)
#define TGUISubsequentCPUToScreenColorExpand	MMIONAME(TGUISubsequentCPUToScreenColorExpand)
#define TGUISetupForScreenToScreenColorExpand	MMIONAME(TGUISetupForScreenToScreenColorExpand)
#define TGUISubsequentScreenToScreenColorExpand	MMIONAME(TGUISubsequentScreenToScreenColorExpand)
#define TGUISetupForFill8x8Pattern		MMIONAME(TGUISetupForFill8x8Pattern)
#define TGUISubsequentFill8x8Pattern		MMIONAME(TGUISubsequentFill8x8Pattern)

#endif

void TGUISync();
void IMAGESync();
void BLADESync();
void TGUISetupForFillRectSolid();
void TGUI9685SetupForFillRectSolid();
void IMAGESetupForFillRectSolid();
void BLADESetupForFillRectSolid();
void IMAGESubsequentFillRectSolid();
void BLADESubsequentFillRectSolid();
void TGUISubsequentFillRectSolid();
void TGUILinearSubsequentFillRectSolid();
void TGUISetClippingRectangle();
void TGUISetupForScreenToScreenCopy();
void TGUISubsequentScreenToScreenCopy();
void IMAGESetupForScreenToScreenCopy();
void BLADESetupForScreenToScreenCopy();
void IMAGESubsequentScreenToScreenCopy();
void BLADESubsequentScreenToScreenCopy();
void TGUISubsequentBresenhamLine();
void TGUISetupForCPUToScreenColorExpand();
void TGUISubsequentCPUToScreenColorExpand();
void TGUISetupForScreenToScreenColorExpand();
void TGUISubsequentScreenToScreenColorExpand();
void TGUISetupForFill8x8Pattern();
void TGUISubsequentFill8x8Pattern();
void TGUISetupForImageWrite();
void TGUISubsequentImageWrite();

#define REPLICATE(x)  				\
	if (vgaBitsPerPixel < 32) {		\
		x |= x << 16;			\
		if (vgaBitsPerPixel < 16)	\
			x |= x << 8;		\
	}

#define PLANEMASKCHECK(x)  			\
	x & ((1 << vgaBitsPerPixel)-1) !=	\
	       (1 << vgaBitsPerPixel)-1		
	

#define HAVE_CLIPPING (IsTGUI9685 || IsTGUI9682 || IsTGUI9680 || \
		       IsTGUI9660 || IsAdvCyber)
#define HAVE_TRANSPARENCY (IsTGUI9440 || IsTGUI9682 || IsTGUI9685 || IsAdvCyber)
#define HAVE_DASHEDLINES (IsTGUI9685)

#ifdef TRIDENT_MMIO
void
ImageInitializeAccelerator()
{
    int engineop;

    IMAGE_OUT(0x20, 0xF0000000);
    switch (vga256InfoRec.depth) {
	case 8:
	    engineop = 0;
	    break;
	case 15:
	    engineop = 5;
	    break;
	case 16:
	    engineop = 1;
	    break;
	case 24:
	    engineop = 2;
	    break;
    }
    IMAGE_OUT(0x20, 0x40000000 | engineop);
    IMAGE_OUT(0x20, 0x80000000);
    IMAGE_OUT(0x44, 0x00000000);
    IMAGE_OUT(0x48, 0x00000000);
    IMAGE_OUT(0x20, 0x60000000 | vga256InfoRec.displayWidth<<16 | vga256InfoRec.displayWidth);
    IMAGE_OUT(0x6C, 0x00000000);
    IMAGE_OUT(0x70, 0x00000000);
    IMAGE_OUT(0x7C, 0x00000000);
    IMAGE_OUT(0x20, 0x10000000 | 4095 << 16 | 4095);
    IMAGE_OUT(0x30, 4095 << 16 | 4095);
    SETUPCHIP = TRUE;
}

void
BladeInitializeAccelerator()
{
    CARD32 stride;

    stride = (vga256InfoRec.displayWidth >> 3) << 20;
    BLADE_OUT(0xC8, stride);
    BLADE_OUT(0xCC, stride);
    BLADE_OUT(0xD0, stride);
    BLADE_OUT(0xD4, stride);
    switch (vga256InfoRec.depth) {
	case 8:
	    stride |= 0<<29;
	    break;
	case 15:
	    stride |= 5<<29;
	    break;
	case 16:
	    stride |= 1<<29;
	    break;
	case 24:
	    stride |= 2<<29;
	    break;
    }
    BLADE_OUT(0xB8, stride);
    BLADE_OUT(0xBC, stride);
    BLADE_OUT(0xC0, stride);
    BLADE_OUT(0xC4, stride);
    SETUPCHIP = TRUE;
}
#endif

/*
 * The following function sets up the supported acceleration. Call it
 * from the FbInit() function in the SVGA driver.
 */
void TGUIAccelInit() {

  if (!Is3Dchip) {
    xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS | PIXMAP_CACHE |
				HARDWARE_PATTERN_MOD_64_OFFSET |
				HARDWARE_PATTERN_SCREEN_ORIGIN |
				HARDWARE_PATTERN_BIT_ORDER_MSBFIRST;

    if (!HAVE_TRANSPARENCY)
	xf86AccelInfoRec.Flags |= HARDWARE_PATTERN_TRANSPARENCY |
				HARDWARE_PATTERN_MONO_TRANSPARENCY;

    xf86AccelInfoRec.Sync = TGUISync;

    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;

    if (!HAVE_TRANSPARENCY)
	xf86GCInfoRec.PolyFillRectSolidFlags |= NO_TRANSPARENCY;

    if (IsTGUI9685) {
	xf86AccelInfoRec.SetupForFillRectSolid = TGUI9685SetupForFillRectSolid;
	xf86AccelInfoRec.SubsequentFillRectSolid = TGUISubsequentFillRectSolid;
    } else {
        xf86AccelInfoRec.SetupForFillRectSolid = TGUISetupForFillRectSolid;
        xf86AccelInfoRec.SubsequentFillRectSolid = TGUISubsequentFillRectSolid;
    }

    if (HAVE_CLIPPING) {
	xf86AccelInfoRec.SetClippingRectangle = TGUISetClippingRectangle;
	xf86AccelInfoRec.Flags |= HARDWARE_CLIP_LINE;
    }

    xf86AccelInfoRec.ErrorTermBits = 11;
    xf86AccelInfoRec.SubsequentBresenhamLine = TGUISubsequentBresenhamLine;

    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK;

    if (!HAVE_TRANSPARENCY)
	xf86GCInfoRec.CopyAreaFlags |= NO_TRANSPARENCY;
 
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
       		TGUISetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	       	TGUISubsequentScreenToScreenCopy;

    /* Fill 8x8 Pattern */
    if ((vgaBitsPerPixel != 32) && (vga256InfoRec.displayWidth <= 1024)) {
	xf86AccelInfoRec.SetupForFill8x8Pattern = TGUISetupForFill8x8Pattern;
	xf86AccelInfoRec.SubsequentFill8x8Pattern = TGUISubsequentFill8x8Pattern;
    }

    /* Color Expansion */
    xf86AccelInfoRec.ColorExpandFlags = VIDEO_SOURCE_GRANULARITY_PIXEL |
					BIT_ORDER_IN_BYTE_MSBFIRST |
					SCANLINE_PAD_DWORD |
					CPU_TRANSFER_PAD_DWORD |
					NO_PLANEMASK;

    if (!HAVE_TRANSPARENCY)
	xf86AccelInfoRec.ColorExpandFlags |= NO_TRANSPARENCY;

    xf86AccelInfoRec.SetupForCPUToScreenColorExpand = 
	TGUISetupForCPUToScreenColorExpand;
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand = 
	TGUISubsequentCPUToScreenColorExpand;
    xf86AccelInfoRec.SetupForScreenToScreenColorExpand = 
	TGUISetupForScreenToScreenColorExpand;
    xf86AccelInfoRec.SubsequentScreenToScreenColorExpand = 
	TGUISubsequentScreenToScreenColorExpand;
  } else {
    if ((TVGAchipset == BLADE3D) || (TVGAchipset == CYBERBLADE)) {
    xf86AccelInfoRec.Flags = PIXMAP_CACHE | BACKGROUND_OPERATIONS |
				ONLY_TWO_BITBLT_DIRECTIONS;

    xf86AccelInfoRec.Sync = BLADESync;

    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK | NO_TRANSPARENCY;

    xf86AccelInfoRec.SetupForFillRectSolid = BLADESetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = BLADESubsequentFillRectSolid;

    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK | NO_TRANSPARENCY;
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
       					BLADESetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	       				BLADESubsequentScreenToScreenCopy;
    } else {
    xf86AccelInfoRec.Flags = PIXMAP_CACHE | BACKGROUND_OPERATIONS |
				ONLY_TWO_BITBLT_DIRECTIONS;

    xf86AccelInfoRec.Sync = IMAGESync;

    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK | NO_TRANSPARENCY;

    xf86AccelInfoRec.SetupForFillRectSolid = IMAGESetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = IMAGESubsequentFillRectSolid;

    xf86GCInfoRec.CopyAreaFlags = NO_PLANEMASK | NO_TRANSPARENCY;
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
       					IMAGESetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
	       				IMAGESubsequentScreenToScreenCopy;
    }
  }

    xf86AccelInfoRec.ServerInfoRec = &vga256InfoRec;

    xf86AccelInfoRec.PixmapCacheMemoryStart = 
				(vga256InfoRec.virtualY *
		vga256InfoRec.displayWidth * vga256InfoRec.bitsPerPixel / 8);

    xf86AccelInfoRec.PixmapCacheMemoryEnd = 2048 * 1024 - 4096;
}

/*
 * This is the implementation of the Sync() function.
 */
void TGUISync() {
	int count = 0, timeout = 0;
	int busy;

	for (;;) {
		BLTBUSY(busy);
		if (busy != GE_BUSY) return;
		count++;
		if (count == 10000000) {
			ErrorF("Trident: BitBLT engine time-out.\n");
			count = 9990000;
			timeout++;
			if (timeout == 8) {
				/* Reset BitBLT Engine */
				TGUI_STATUS(0x00);
				return;
			}
		}
	}
}

/*
 * This is the implementation of the SetupForFillRectSolid function
 * that sets up the coprocessor for a subsequent batch for solid
 * rectangle fills.
 */

void TGUISetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
	REPLICATE(color);

	TGUI_FCOLOUR(color);
	TGUI_BCOLOUR(color);
	TGUI_FMIX(TGUIRops_Pixalu[rop]);
}

void TGUI9685SetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
	REPLICATE(color);

	TGUI_FPATCOL(color);
	TGUI_BPATCOL(color);
	TGUI_FMIX(TGUIRops_Pixalu[rop]);
}
/*
 * This is the implementation of the SubsequentForFillRectSolid function
 * that sends commands to the coprocessor to fill a solid rectangle of
 * the specified location and size, with the parameters from the SetUp
 * call.
 */
void TGUISubsequentFillRectSolid(x, y, w, h)
    int x, y, w, h;
{
	int direction = 0;
	TGUI_DRAWFLAG(SOLIDFILL | PATMONO | direction);
	TGUI_DIM_XY(w,h);
	TGUI_DEST_XY(x,y);
	TGUI_COMMAND(GE_BLT);
}

void TGUILinearSubsequentFillRectSolid(x, y, w, h)
	int x, y, w, h;
{
	int dstaddr = y * vga256InfoRec.displayWidth + x;

	TGUI_DRAWFLAG(SOLIDFILL | PATMONO | DSTLINEAR);
#if 0
	TGUI_BBDSTSIZE(h,w);
	TGUI_BBDST(dstaddr);
#else
	TGUI_DIM_XY(w,h);
	TGUI_DEST_LINEAR(dstaddr);
	TGUI_DEST_PITCH(vga256InfoRec.virtualX);
#endif
	TGUI_COMMAND(GE_BLT);
}

void TGUISetClippingRectangle(x1, y1, x2, y2)
{
	TGUI_SRCCLIP_XY(x1, y1);
	TGUI_DSTCLIP_XY(x2, y2);
	ClipOn = TRUE;
}

static int blitxdir, blitydir;
 
void TGUISetupForScreenToScreenCopy(xdir, ydir, rop, planemask,
transparency_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int transparency_color;
{
    int direction = 0;

    if (xdir < 0) direction |= XNEG;
    if (ydir < 0) direction |= YNEG;
    if ((HAVE_TRANSPARENCY) && (transparency_color != -1)) {
	direction |= TRANS_ENABLE;
	REPLICATE(transparency_color);
	TGUI_BCOLOUR(transparency_color);
    }
    TGUI_DRAWFLAG(direction | SCR2SCR);
    TGUI_FMIX(TGUIRops_alu[rop]);
    blitxdir = xdir;
    blitydir = ydir;
}

void TGUISubsequentScreenToScreenCopy(x1, y1, x2, y2, w, h)
    int x1, y1, x2, y2, w, h;
{
    if (blitydir < 0) {
        y1 = y1 + h - 1;
	y2 = y2 + h - 1;
    }
    if (blitxdir < 0) {
	x1 = x1 + w - 1;
	x2 = x2 + w - 1;
    }
    TGUI_SRC_XY(x1,y1);
    TGUI_DEST_XY(x2,y2);
    TGUI_DIM_XY(w,h);
    TGUI_COMMAND(GE_BLT);
}

void TGUISubsequentBresenhamLine(x1, y1, octant, err, e1, e2, length)
    int x1, y1, octant, err, e1, e2, length;
{
	int direction = 0;

	if (octant & YMAJOR)      direction |= YMAJ;
	if (octant & XDECREASING) direction |= XNEG;
	if (octant & YDECREASING) direction |= YNEG;
	TGUI_SRC_XY(e2,e1);
	TGUI_DEST_XY(x1,y1);
	TGUI_DIM_XY(err,length);
	if ((HAVE_CLIPPING) && (ClipOn)) {
		if ((IsTGUI9682) || (IsAdvCyber)) {
			GE_OP &= 0xFEFF; /* Enable Clipping */
		} else 
		if (IsTGUI9685) {
			direction |= CLIPENABLE;
		}
	}
	TGUI_DRAWFLAG(SOLIDFILL | STENCIL | direction);
	TGUI_COMMAND(GE_BRESLINE);
	if ((IsTGUI9682) || (IsAdvCyber))
		GE_OP |= 0x100; /* Disable Clipping */
	ClipOn = FALSE;
}

void TGUISetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
	int drawflag = 0;

	REPLICATE(fg);

	TGUI_FCOLOUR(fg);
	if ((HAVE_TRANSPARENCY) && (bg == -1)) {
		drawflag |= TRANS_ENABLE;
		TGUI_BCOLOUR(~fg);
	} else {
		REPLICATE(bg);
		TGUI_BCOLOUR(bg);
	}
	TGUI_DRAWFLAG(SRCMONO | drawflag);
	TGUI_FMIX(TGUIRops_alu[rop]);
}

void TGUISubsequentCPUToScreenColorExpand(x, y, w, h, skipleft)
    int x, y, w, h, skipleft;
{
	TGUI_DEST_XY(x,y);
	TGUI_DIM_XY(w,h);
	TGUI_COMMAND(GE_BLT);
}

void TGUISetupForFill8x8Pattern(patternx, patterny, rop, planemask,
transparency_color)
    int patternx, patterny, rop;
    unsigned planemask;
    int transparency_color;
{
	int direction = 0;
    	if ((HAVE_TRANSPARENCY) && (transparency_color != -1)) {
		direction |= TRANS_ENABLE;
		REPLICATE(transparency_color);
		TGUI_BCOLOUR(transparency_color);
    	}
	TGUI_FMIX(TGUIRops_Pixalu[rop]); /* ROP */
	TGUI_DRAWFLAG(PAT2SCR | direction);
	TGUI_PATLOC(((patterny * vga256InfoRec.displayWidth *
			vga256InfoRec.bitsPerPixel / 8) + 
			(patternx * vga256InfoRec.bitsPerPixel / 8)) >> 6);
}

void TGUISubsequentFill8x8Pattern(patternx, patterny, x, y, w, h)
    int patternx, patterny;
    int x, y, w, h;
{
	TGUI_DEST_XY(x,y);
	TGUI_DIM_XY(w,h);
	TGUI_COMMAND(GE_BLT);
}

void TGUISetupForScreenToScreenColorExpand(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
	int drawflag = 0;

	REPLICATE(fg);

	TGUI_FCOLOUR(fg);
	if ((HAVE_TRANSPARENCY) && (bg == -1)) {
		drawflag |= TRANS_ENABLE;
		TGUI_BCOLOUR(~fg);
	} else {
		REPLICATE(bg);
		TGUI_BCOLOUR(bg);
	}
	TGUI_DRAWFLAG(SCR2SCR | SRCMONO | drawflag);
	TGUI_FMIX(TGUIRops_alu[rop]);
}

void TGUISubsequentScreenToScreenColorExpand(srcx, srcy, x, y, w, h)
    int srcx, srcy, x, y, w, h;
{
	TGUI_SRC_XY(srcx,srcy);
	TGUI_DEST_XY(x,y);
	TGUI_DIM_XY(w,h);
	TGUI_COMMAND(GE_BLT);
}

void
IMAGESync()
{
    int count = 0, timeout = 0;
    int busy;

    for (;;) {
	IMAGEBUSY(busy);
	if (busy == 0) {
	    return;
	}
	count++;
	if (count == 10000000) {
	    ErrorF("Trident: BitBLT engine time-out.\n");
	    count = 9990000;
	    timeout++;
	    if (timeout == 8) {
		/* Reset BitBLT Engine */
		IMAGE_STATUS(0x00);
		return;
	    }
	}
    }
}

void
IMAGESetupForScreenToScreenCopy( 
				int xdir, int ydir, int rop,
				unsigned int planemask, int transparency_color)
{
    blitxdir = 0;
    if ((xdir < 0) || (ydir < 0)) blitxdir |= 1<<2;

    if (!SETUPCHIP) ImageInitializeAccelerator();
    IMAGE_OUT(0x20, 0x40000000 | GE_OP);
    IMAGE_OUT(0x20, 0x90000000 | TGUIRops_alu[rop]);
}

void
IMAGESubsequentScreenToScreenCopy(int x1, int y1,
					int x2, int y2, int w, int h)
{
    if (blitxdir) {
	IMAGE_OUT(0x00, (y1+h-1)<<16 | (x1+w-1));
	IMAGE_OUT(0x04, y1<<16 | x1);
	IMAGE_OUT(0x08, (y2+h-1)<<16 | (x2+w-1));
	IMAGE_OUT(0x0C, y2<<16 | x2);
    } else {
	IMAGE_OUT(0x00, y1<<16 | x1);
	IMAGE_OUT(0x04, (y1+h-1)<<16 | (x1+w-1));
	IMAGE_OUT(0x08, y2<<16 | x2);
	IMAGE_OUT(0x0C, (y2+h-1)<<16 | (x2+w-1));
    }

    IMAGE_OUT(0x24, 0x80000000 | 1<<22 | 1<<7 | 1<<10 | blitxdir);
    IMAGESync();
}

void
IMAGESetupForFillRectSolid(int color, 
				    int rop, unsigned int planemask)
{
    REPLICATE(color);
    if (!SETUPCHIP) ImageInitializeAccelerator();
    IMAGE_OUT(0x20, 0x40000000 | GE_OP);
    IMAGE_OUT(0x44, color);
    IMAGE_OUT(0x48, color);
    IMAGE_OUT(0x20, 0x90000000 | TGUIRops_alu[rop]);
}

void
IMAGESubsequentFillRectSolid(int x, int y, int w, int h)
{
    IMAGE_OUT(0x08, y<<16 | x);
    IMAGE_OUT(0x0C, (y+h-1)<<16 | x+w-1);
    IMAGE_OUT(0x24, 0x80000000 | 1<<22 | 1<<10 | 1<<9);
    IMAGESync();
}

void
BLADESync()
{
    int count = 0, timeout = 0;
    int busy;

    for (;;) {
	BLADEBUSY(busy);
	if (busy == 0) {
	    return;
	}
	count++;
	if (count == 10000000) {
	    ErrorF("Trident: BitBLT engine time-out.\n");
	    count = 9990000;
	    timeout++;
	    if (timeout == 8) {
		/* Reset BitBLT Engine */
		BLADE_STATUS(0x00);
		return;
	    }
	}
    }
}

void
BLADESetupForScreenToScreenCopy( 
				int xdir, int ydir, int rop,
				unsigned int planemask, int transparency_color)
{
    blitxdir = 0;
    if ((xdir < 0) || (ydir < 0)) blitxdir |= 1<<1;

    if (!SETUPCHIP) BladeInitializeAccelerator();
    BLADE_OUT(0x48, TGUIRops_alu[rop]);
}

void
BLADESubsequentScreenToScreenCopy(int x1, int y1,
					int x2, int y2, int w, int h)
{
    BLADE_OUT(0x44, 0xE0000000 | 1<<19 | 1<<4 | 1<<2 | blitxdir);

    if (blitxdir) {
	BLADE_OUT(0x00, (y1+h-1)<<16 | (x1+w-1));
	BLADE_OUT(0x04, y1<<16 | x1);
	BLADE_OUT(0x08, (y2+h-1)<<16 | (x2+w-1));
	BLADE_OUT(0x0C, y2<<16 | x2);
    } else {
	BLADE_OUT(0x00, y1<<16 | x1);
	BLADE_OUT(0x04, (y1+h-1)<<16 | (x1+w-1));
	BLADE_OUT(0x08, y2<<16 | x2);
	BLADE_OUT(0x0C, (y2+h-1)<<16 | (x2+w-1));
    }
    BLADESync();
}

void
BLADESetupForFillRectSolid(int color, 
				    int rop, unsigned int planemask)
{
    REPLICATE(color);
    if (!SETUPCHIP) BladeInitializeAccelerator();
    BLADE_OUT(0x60, color);
    BLADE_OUT(0x48, TGUIRops_alu[rop]);
}

void
BLADESubsequentFillRectSolid(int x, int y, int w, int h)
{
    BLADE_OUT(0x44, 0x20000000 | 1<<19 | 1<<4 | 2<<2);
    BLADE_OUT(0x08, y<<16 | x);
    BLADE_OUT(0x0C, (y+h-1)<<16 | x+w-1);
    BLADESync();
}
