/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/glint_accel.c,v 1.24.2.2 1998/08/25 10:54:03 hohndel Exp $ */
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
 *	     Stefan Dirsch, <sndirsch@suse.de>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 * 
 * GLINT accelerated options.
 */

#define ACCEL_DEBUG

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

extern int pprod;
extern int coprotype;
extern GLINTWindowBase;
extern UsePCIRetry;
int ScanlineWordCount;
int savedplanemask = 0xffffffff;
int savedrop = UNIT_DISABLE;
int savedreadmode = 0;
int savedrasterizer = UNIT_DISABLE;
int savedcolorddamode = UNIT_DISABLE;
static int gcolor;
static int grop;
static int mode;
static int span;
static int gbg, gfg;
CARD32 ScratchBuffer[512];
static int blitxdir, blitydir;

void GLINTSync();
void GLINTSetupForFillRectSolid();
void GLINTSubsequentFillRectSolid();
void GLINTSubsequentFillTrapezoidSolid();
void GLINTSetupForScreenToScreenCopy();
void GLINTSubsequentScreenToScreenCopy();

void GLINTAccelInit() 
{
    xf86AccelInfoRec.Flags = 	PIXMAP_CACHE |
      				ONLY_LEFT_TO_RIGHT_BITBLT |
 				COP_FRAMEBUFFER_CONCURRENCY |
      				BACKGROUND_OPERATIONS ;

    xf86AccelInfoRec.Sync = GLINTSync;
  
    xf86GCInfoRec.PolyFillRectSolidFlags = 0;
  
    xf86AccelInfoRec.SetupForFillRectSolid = GLINTSetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = GLINTSubsequentFillRectSolid;

    xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY;

    xf86AccelInfoRec.SetupForScreenToScreenCopy =
      GLINTSetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
      GLINTSubsequentScreenToScreenCopy;

    xf86AccelInfoRec.PixmapCacheMemoryStart = glintInfoRec.displayWidth *
			glintInfoRec.virtualY * (glintInfoRec.bitsPerPixel / 8);
  
    xf86AccelInfoRec.PixmapCacheMemoryEnd = glintInfoRec.videoRam * 1024;
}

void GLINTSync() {
	while (GLINT_READ_REG(DMACount) != 0);
	GLINT_WAIT(2);
	GLINT_WRITE_REG(0xc00, FilterMode);
	GLINT_WRITE_REG(0, GlintSync);
	do {
    		while(GLINT_READ_REG(OutFIFOWords) == 0);
#define Sync_tag 0x188
	} while (GLINT_READ_REG(OutputFIFO) != Sync_tag);
}

void GLINTSetupForFillRectSolid(int color, int rop, unsigned planemask)
{
	gcolor = color;
	REPLICATE(color);
	

	grop = rop;
	GLINT_WAIT(6);
	DO_PLANEMASK(planemask);
	if (rop == GXcopy) {
		GLINT_WRITE_REG(pprod, FBReadMode);
		GLINT_WRITE_REG(UNIT_DISABLE, PatternRamMode);
		GLINT_WRITE_REG(color, FBBlockColor);
		mode = FastFillEnable;
	} else {
		GLINT_WRITE_REG(pprod | FBRM_DstEnable, FBReadMode);
		GLINT_WRITE_REG(UNIT_ENABLE, PatternRamMode);
		GLINT_WRITE_REG(color, PatternRamData0);
		mode = FastFillEnable | SpanOperation;
	}
	GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
	GLINT_WRITE_REG(1<<16,dY);
}

void GLINTSubsequentFillRectSolid(int x, int  y, int  w, int  h)
{
	GLINT_WAIT(5);
	GLINT_WRITE_REG((x+w)<<16, StartXSub);
	GLINT_WRITE_REG(x<<16,StartXDom);
	GLINT_WRITE_REG(y<<16,StartY);
	GLINT_WRITE_REG(h,GLINTCount);
	GLINT_WRITE_REG(PrimitiveTrapezoid | mode,Render);
}

void GLINTSetupForScreenToScreenCopy( int xdir, int  ydir, int  rop,
				    unsigned planemask, int transparency_color)
{
	blitydir = ydir;
	blitxdir = xdir;

	GLINT_WAIT(4);
	DO_PLANEMASK(planemask);
	GLINT_WRITE_REG(UNIT_DISABLE, PatternRamMode);

	if ((rop == GXclear) || (rop == GXset)) {
		GLINT_WRITE_REG(pprod, FBReadMode);
	} else 
	if ((rop == GXcopy) || (rop == GXcopyInverted)) {
		GLINT_WRITE_REG(pprod | FBRM_SrcEnable, FBReadMode);
	} else {
		GLINT_WRITE_REG(pprod | FBRM_SrcEnable | FBRM_DstEnable, FBReadMode);
	}

	GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);
}

void GLINTSubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2,
				     int w, int h)
{
	int srcaddr;
	int dstaddr;

	/* Glint 500TX only allows left to right bitblt's */

	GLINT_WAIT(8);
	if (blitydir < 0) {
		y1 = y1 + h - 1;
		y2 = y2 + h - 1;
		GLINT_WRITE_REG(-1<<16, dY);
	} else {
		GLINT_WRITE_REG(1<<16, dY);
	}

	srcaddr = GLINTWindowBase + y1 * glintInfoRec.displayWidth + x1;
	dstaddr = GLINTWindowBase + y2 * glintInfoRec.displayWidth + x2;

	GLINT_WRITE_REG(srcaddr - dstaddr, FBSourceOffset);
	GLINT_WRITE_REG(x2<<16, StartXDom);
	GLINT_WRITE_REG((x2+w)<<16, StartXSub);
	GLINT_WRITE_REG(y2<<16, StartY);
	GLINT_WRITE_REG(h, GLINTCount);
	GLINT_WRITE_REG(PrimitiveTrapezoid | FastFillEnable | SpanOperation, Render);
}

