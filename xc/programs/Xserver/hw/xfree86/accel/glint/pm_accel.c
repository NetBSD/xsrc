/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/pm_accel.c,v 1.9.2.2 1998/08/25 10:54:05 hohndel Exp $ */

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
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 * 
 * Permedia accelerated options.
 */


#include "cfb.h"
#include "xf86.h"
#include "miline.h"
#include "compiler.h"

#include "glint_regs.h"
#include "glint.h"
#include "xf86xaa.h"
#include "xf86expblt.h"
#include "dixfontstr.h"

#include <float.h>

#if DEBUG && !defined(XFree86LOADER)
#include <stdio.h>
#endif

extern Bool	UsePCIRetry;
extern int	pprod;
extern int      ScanlineWordCount;
extern int	savedplanemask;
extern CARD32   ScratchBuffer[512];
static char	bppand[4] = { 0x03, 0x01, 0x00, 0x00 };
static int      blitxdir, blitydir;
static int      mode;
static int	span;
static int      gcolor;
static int      grop;
static int      gbg, gfg;
extern          GLINTWindowBase;

extern void     GLINTSync ();
extern void	xf86ImageGlyphBltTE();
extern void	xf86PolyGlyphBltTE();

void            PermediaSetupForFillRectSolid();
void		PermediaSubsequentFillRectSolid();
void 		PermediaSubsequentScreenToScreenCopy ();
void            PermediaSetupForScreenToScreenCopy ();

void
PermediaAccelInit ()
{
  xf86AccelInfoRec.Flags = PIXMAP_CACHE |
    BACKGROUND_OPERATIONS |
    ONLY_LEFT_TO_RIGHT_BITBLT |
    COP_FRAMEBUFFER_CONCURRENCY ;

  xf86AccelInfoRec.Sync = GLINTSync;

  xf86GCInfoRec.PolyFillRectSolidFlags = 0;
  xf86AccelInfoRec.SetupForFillRectSolid = PermediaSetupForFillRectSolid;
  xf86AccelInfoRec.SubsequentFillRectSolid = PermediaSubsequentFillRectSolid;

  xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY; 

  xf86AccelInfoRec.SetupForScreenToScreenCopy = PermediaSetupForScreenToScreenCopy;
  xf86AccelInfoRec.SubsequentScreenToScreenCopy = PermediaSubsequentScreenToScreenCopy;

  xf86AccelInfoRec.PixmapCacheMemoryStart = glintInfoRec.displayWidth *
	glintInfoRec.virtualY * (glintInfoRec.bitsPerPixel / 8);

  xf86AccelInfoRec.PixmapCacheMemoryEnd = glintInfoRec.videoRam * 1024;
}

void
PermediaSetupForScreenToScreenCopy (int xdir, int ydir, int rop,
				 unsigned planemask, int transparency_color)
{
  blitxdir = xdir;
  blitydir = ydir;
  
  grop = rop;

  GLINT_WAIT(5);
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
  if (ydir == 1) {
	GLINT_WRITE_REG(1<<16, dY);
  } else {
	GLINT_WRITE_REG(-1<<16, dY); 
  }
}

void
PermediaSubsequentScreenToScreenCopy (int x1, int y1, int x2, int y2,
				   int w, int h)
{
  int lowerbits = bppand[(glintInfoRec.bitsPerPixel>>3)-1];
  int srcaddr;
  int dstaddr;
  char align;

  if (blitydir < 0) {
	y1 = y1 + h - 1;
	y2 = y2 + h - 1;
  }

  /* We can only use GXcopy for Packed modes, and less than 32 width
   * gives us speed for small blits. */
  if ((w < 32) || (grop != GXcopy)) {
  	srcaddr = y1 * glintInfoRec.displayWidth + x1;
  	dstaddr = y2 * glintInfoRec.displayWidth + x2;
  	GLINT_WAIT(7);
	GLINT_WRITE_REG(mode, FBReadMode);
  	GLINT_WRITE_REG (x2<<16, StartXDom);
  	GLINT_WRITE_REG ((x2+w)<<16, StartXSub);
  	GLINT_WRITE_REG (y2 << 16, StartY);
  	GLINT_WRITE_REG (h, GLINTCount);
  } else {
  	srcaddr = y1 * glintInfoRec.displayWidth + (x1 & ~lowerbits);
  	dstaddr = y2 * glintInfoRec.displayWidth + (x2 & ~lowerbits);
  	align = (x2 & lowerbits) - (x1 & lowerbits);
  	GLINT_WAIT(8);
	GLINT_WRITE_REG(mode | FBRM_Packed | (align&7)<<20, FBReadMode);
  	GLINT_WRITE_REG(x2<<16|(x2+w), PackedDataLimits);
  	GLINT_WRITE_REG(Shiftbpp(x2)<<16, StartXDom);
  	GLINT_WRITE_REG((Shiftbpp(x2+w+7))<<16, StartXSub);
  	GLINT_WRITE_REG(y2 << 16, StartY);
  	GLINT_WRITE_REG(h, GLINTCount);
  }

  GLINT_WRITE_REG(srcaddr - dstaddr, FBSourceOffset);
  GLINT_WRITE_REG (PrimitiveTrapezoid, Render);
}

void PermediaSetupForFillRectSolid(int color, int rop, unsigned planemask)
{
  REPLICATE(color);
  gcolor = color;

  grop = rop;
  GLINT_WAIT(7);
  DO_PLANEMASK(planemask);
  GLINT_WRITE_REG(0, RasterizerMode);
  if (rop == GXcopy) {
	mode = pprod;
  	GLINT_WRITE_REG(pprod, FBReadMode);
  	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(color, FBBlockColor);
  } else {
	mode = pprod|FBRM_DstEnable;
      	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      	GLINT_WRITE_REG(color, ConstantColor);
  }
  GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
  GLINT_WRITE_REG(1<<16, dY);
}

void PermediaSubsequentFillRectSolid(int x, int y, int w, int h)
{
  int speed = 0;
  if (grop == GXcopy) {
	GLINT_WAIT(5);
  	GLINT_WRITE_REG(x<<16, StartXDom);
  	GLINT_WRITE_REG(y<<16, StartY);
  	GLINT_WRITE_REG(h, GLINTCount);
  	GLINT_WRITE_REG((x+w)<<16, StartXSub);
  	speed = FastFillEnable;
  } else {
	GLINT_WAIT(7);
      	GLINT_WRITE_REG(pprod | FBRM_Packed | FBRM_DstEnable, FBReadMode);
  	GLINT_WRITE_REG(x<<16|(x+w), PackedDataLimits);
 	GLINT_WRITE_REG(Shiftbpp(x)<<16, StartXDom);
  	GLINT_WRITE_REG(y<<16, StartY);
  	GLINT_WRITE_REG(h, GLINTCount);
  	GLINT_WRITE_REG((Shiftbpp(x+w+7))<<16, StartXSub);
  }
  GLINT_WRITE_REG(PrimitiveTrapezoid | speed, Render);
}

