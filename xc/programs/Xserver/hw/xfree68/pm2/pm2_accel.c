/* $XFree86: xc/programs/Xserver/hw/xfree68/pm2/pm2_accel.c,v 1.1.2.1 1999/06/02 07:50:13 hohndel Exp $ */
/*
 * Copyright 1996,1997 by Alan Hourihane, Wigan, England.
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
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *           Dirk Hohndel, <hohndel@suse.de>
 *           Stefan Dirsch, <sndirsch@suse.de>
 *           Mark Vojkovich, <mvojkovi@ucsd.edu>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 * 
 * Permedia 2 accelerated options.
 *
 * Adapted for XF68_FBDev with pm2fb by Michel Dänzer
 */


#include "cfb.h"
#include "xf86.h"
#include "miline.h"
#include "compiler.h"

#include "glint_regs.h"
#include "glint.h"
#include "pm2_accel.h"
#include "xf86xaa.h"
#include "xf86expblt.h"
#include "dixfontstr.h"

#include <float.h>

#if DEBUG && !defined(XFree86LOADER)
#include <stdio.h>
#endif

static char	bppand[4] = { 0x03, 0x01, 0x00, 0x00 };
static int      mode;
static int      gcolor;
static int      grop;

void            Permedia2SetupForFillRectSolid();
void		Permedia2SubsequentFillRectSolid();
void 		Permedia2SubsequentScreenToScreenCopy ();
void            Permedia2SetupForScreenToScreenCopy ();
void 		Permedia2SubsequentScreenToScreenCopy32bpp ();
void            Permedia2SetupForScreenToScreenCopy32bpp ();

static CARD32 BlitDir;

void
Permedia2SetupForScreenToScreenCopy32bpp (int xdir, int ydir, int rop,
				 unsigned planemask, int transparency_color)
{
  BlitDir = 0;
  if (xdir == 1) BlitDir |= XPositive;
  if (ydir == 1) BlitDir |= YPositive;
  
  grop = rop;

  GLINT_WAIT(5);
  DO_PLANEMASK(planemask);

  GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
  GLINT_WRITE_REG(0, RasterizerMode);

  if ((rop == GXset) || (rop == GXclear)) {
	GLINT_WRITE_REG(pprod, FBReadMode);
  } else
  if ((rop == GXcopy) || (rop == GXcopyInverted)) {
	GLINT_WRITE_REG(pprod|FBRM_SrcEnable, FBReadMode);
  } else {
	GLINT_WRITE_REG(pprod|FBRM_SrcEnable|FBRM_DstEnable, FBReadMode);
  }
  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
}

void
Permedia2SubsequentScreenToScreenCopy32bpp (int x1, int y1, int x2, int y2,
				   int w, int h)
{
  int srcaddr = y1 * pm2fbMaxX + x1;
  int dstaddr = y2 * pm2fbMaxX + x2;
  GLINT_WAIT(4);
  GLINT_WRITE_REG((y2<<16)|x2, RectangleOrigin);
  GLINT_WRITE_REG((h<<16)|w, RectangleSize);
  GLINT_WRITE_REG(srcaddr - dstaddr, FBSourceOffset);
  GLINT_WRITE_REG(PrimitiveRectangle | BlitDir, Render);
}

void
Permedia2SetupForScreenToScreenCopy (int xdir, int ydir, int rop,
				 unsigned planemask, int transparency_color)
{
  BlitDir = 0;
  if (xdir == 1) BlitDir |= XPositive;
  if (ydir == 1) BlitDir |= YPositive;
  
  grop = rop;

  GLINT_WAIT(4);
  DO_PLANEMASK(planemask);
  GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
  GLINT_WRITE_REG(0, RasterizerMode);

  if ((rop == GXset) || (rop == GXclear)) {
	mode = pprod;
  } else
  if ((rop == GXcopy) || (rop == GXcopyInverted)) {
	mode = pprod | FBRM_SrcEnable;
  } else {
	mode = pprod | FBRM_SrcEnable | FBRM_DstEnable;
  }
  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
}

void
Permedia2SubsequentScreenToScreenCopy (int x1, int y1, int x2, int y2,
				   int w, int h)
{
  int lowerbits = bppand[(glintInfoRec.bitsPerPixel>>3)-1];
  int srcaddr;
  int dstaddr;
  char align;

  /* We can only use GXcopy for Packed modes */
  if (grop != GXcopy) {
  	srcaddr = y1 * pm2fbMaxX + x1;
  	dstaddr = y2 * pm2fbMaxX + x2;
	GLINT_WAIT(5);
	GLINT_WRITE_REG(mode, FBReadMode);
  	GLINT_WRITE_REG((y2<<16)|x2, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|w, RectangleSize);
  } else {
  	srcaddr = y1 * pm2fbMaxX + (x1 & ~lowerbits);
  	dstaddr = y2 * pm2fbMaxX + (x2 & ~lowerbits);
  	align = (x2 & lowerbits) - (x1 & lowerbits);
	GLINT_WAIT(6);
	GLINT_WRITE_REG(mode | FBRM_Packed, FBReadMode);
  	GLINT_WRITE_REG((y2<<16)|x2>>Bppshift, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|(w+7)>>Bppshift, RectangleSize);
  	GLINT_WRITE_REG(align<<29|x2<<16|(x2+w), PackedDataLimits);
  }
	

  GLINT_WRITE_REG(srcaddr - dstaddr, FBSourceOffset);
  GLINT_WRITE_REG(PrimitiveRectangle | BlitDir, Render);
}

/* Data should be written to FBData after this function */
void
Permedia2ImageWrite (int rop, unsigned planemask, int x, int y, int w, int h)
{
  grop = rop;

  GLINT_WAIT(4);
  DO_PLANEMASK(planemask);
  GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
  GLINT_WRITE_REG(0, RasterizerMode);

  if ((rop == GXset) || (rop == GXclear)) {
	mode = pprod;
  } else
  if ((rop == GXcopy) || (rop == GXcopyInverted)) {
	mode = pprod;
  } else {
	mode = pprod | FBRM_DstEnable;
  }

#ifdef DEBUG
	ErrorF(" P2ImageWrite: mode = %d", mode);
#endif

  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);

  /* We can only use GXcopy for Packed modes */
  /*if (grop != GXcopy) {*/
	GLINT_WAIT(5);
	GLINT_WRITE_REG(mode, FBReadMode);
  	GLINT_WRITE_REG((y<<16)|x, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|w, RectangleSize);
  /* I guess there's no packed mode when downloading?
  } else {
  	align = (x & lowerbits) /*- (x & lowerbits)*/;
	/*GLINT_WAIT(6);
	GLINT_WRITE_REG(mode | FBRM_Packed, FBReadMode);
  	GLINT_WRITE_REG((y<<16)|x>>Bppshift, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|(w+7)>>Bppshift, RectangleSize);
  	GLINT_WRITE_REG(align<<29|x<<16|(x+w), PackedDataLimits);
  }*/
	
  GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | SyncOnHostData, Render);
  /* Now the chip waits for Data to be downloaded */
}

#define STIPPLEMODE adjLeftX << 7 | adjTopY << 12 | UNIT_ENABLE
	
void Permedia2SetupForFillRectStipple(int color, int bgcolor, int adjLeftX,
        int adjTopY, int rop, unsigned planemask, int opaque)
{
  gcolor = color;
  REPLICATE(color);
  REPLICATE(bgcolor);

  GLINT_WAIT(7);
  DO_PLANEMASK(planemask);
  grop = rop;
  if (rop == GXcopy) {
	mode = pprod;
	GLINT_WRITE_REG(pprod, FBReadMode);
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, FBBlockColor);
  } else {
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      	GLINT_WRITE_REG(color, ConstantColor);
	/* We can use Packed mode for filling solid non-GXcopy rasters */
	
	mode = pprod|FBRM_DstEnable;
  }

  if (opaque)
  {
        GLINT_WRITE_REG(ASM_ForceBGColor |
                (adjLeftX & 0x7) << 7 | (adjTopY & 0x7) << 12 | UNIT_ENABLE,
                AreaStippleMode);
        GLINT_WRITE_REG(bgcolor, Texel0);
  } else	
        GLINT_WRITE_REG((adjLeftX & 0x7) << 7 | (adjTopY & 0x7) << 12 | UNIT_ENABLE,
        AreaStippleMode);

  GLINT_WRITE_REG(0, RasterizerMode);
  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
  GLINT_WRITE_REG(1<<16,dY);
}

void Permedia2SubsequentFillRectStipple(int x, int y, int w, int h)
{
  int speed = 0;

  if (grop == GXcopy) {
	GLINT_WAIT(3);
  	GLINT_WRITE_REG((y<<16)|x, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|w, RectangleSize);
  	/*speed = FastFillEnable;*/
  } else {
	GLINT_WAIT(5);
	GLINT_WRITE_REG(mode|FBRM_Packed, FBReadMode);
  	GLINT_WRITE_REG((y<<16)|x>>Bppshift, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|(w+7)>>Bppshift, RectangleSize);
  	GLINT_WRITE_REG(x<<16|(x+w), PackedDataLimits);
  	speed = 0;
  }
  GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive /*| speed*/
  	| AreaStippleEnable, Render);
}

void Permedia2SetupForFillRectSolid(int color, int rop, unsigned planemask)
{
  gcolor = color;
  REPLICATE(color);

  GLINT_WAIT(7);
  DO_PLANEMASK(planemask);
  grop = rop;
  if (rop == GXcopy) {
	mode = pprod;
	GLINT_WRITE_REG(pprod, FBReadMode);
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, FBBlockColor);
  } else {
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      	GLINT_WRITE_REG(color, ConstantColor);
	/* We can use Packed mode for filling solid non-GXcopy rasters */
	
	mode = pprod|FBRM_DstEnable;
  }
  GLINT_WRITE_REG(0, RasterizerMode);
  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
  GLINT_WRITE_REG(1<<16,dY);
}

void Permedia2SubsequentFillRectSolid(int x, int y, int w, int h)
{
  int speed = 0;

  if (grop == GXcopy) {
	GLINT_WAIT(3);
  	GLINT_WRITE_REG((y<<16)|x, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|w, RectangleSize);
  	speed = FastFillEnable;
  } else {
	GLINT_WAIT(5);
	GLINT_WRITE_REG(mode|FBRM_Packed, FBReadMode);
  	GLINT_WRITE_REG((y<<16)|x>>Bppshift, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|(w+7)>>Bppshift, RectangleSize);
  	GLINT_WRITE_REG(x<<16|(x+w), PackedDataLimits);
  	speed = 0;
  }
  GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | speed, Render);
}

void Permedia2SetupForFillRectSolidBitmask(int color, int bgcolor, int px, int rop,
        unsigned planemask, int opaque)
{
  gcolor = color;
  REPLICATE(color);
  REPLICATE(bgcolor);

  GLINT_WAIT(7);
  DO_PLANEMASK(planemask);
  grop = rop;
  if (rop == GXcopy) {
	mode = pprod;
	GLINT_WRITE_REG(pprod, FBReadMode);
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, FBBlockColor);
  } else {
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      	GLINT_WRITE_REG(color, ConstantColor);
	/* We can use Packed mode for filling solid non-GXcopy rasters */
	
	mode = pprod|FBRM_DstEnable;
  }

  if (opaque){
	GLINT_WRITE_REG(ForceBackgroundColor | BitMaskPackingEachScanline | BitMaskRelative |
	        (px & 0x1f) << 10, RasterizerMode);
	GLINT_WRITE_REG(bgcolor, Texel0);
  } else
  	GLINT_WRITE_REG(BitMaskPackingEachScanline | BitMaskRelative | (px & 0x1f) << 10, RasterizerMode);
  	
  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
  GLINT_WRITE_REG(1<<16,dY);
}

void Permedia2SubsequentFillRectSolidBitmask(int x, int y, int w, int h)
{
  int speed = 0;

  if (grop == GXcopy) {
	GLINT_WAIT(3);
  	GLINT_WRITE_REG((y<<16)|x, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|w, RectangleSize);
  	/*speed = FastFillEnable;*/
  } else {
	GLINT_WAIT(5);
	GLINT_WRITE_REG(mode|FBRM_Packed, FBReadMode);
  	GLINT_WRITE_REG((y<<16)|x>>Bppshift, RectangleOrigin);
  	GLINT_WRITE_REG((h<<16)|(w+7)>>Bppshift, RectangleSize);
  	GLINT_WRITE_REG(x<<16|(x+w), PackedDataLimits);
  	speed = 0;
  }
  GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive /*| speed*/ | SyncOnBitMask |
  	SyncOnHostData, Render);
}
