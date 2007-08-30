/*
 * Fujitsu AG-10e framebuffer - defines.
 *
 * Copyright (C) 2007 Michael Lorenz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6.h,v 1.3 2001/05/04 19:05:45 dawes Exp $ */

#ifndef AG10E_H
#define AG10E_H

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86RamDac.h"
#include "Xmd.h"
#include "gcstruct.h"
#include "ag10e_regs.h"
#include "xf86sbusBus.h"
#include "xaa.h"

typedef struct {
	unsigned char	*fb;
	unsigned char   *regs;
	int		vclipmax;
	int		width;
	int		height;
	int		maxheight;
	int		vidmem;
	
	sbusDevicePtr	psdp;
	Bool		HWCursor;
	Bool		NoAccel;
	CloseScreenProcPtr CloseScreen;
	xf86CursorInfoPtr CursorInfoRec;
	struct fbcursor Cursor;
	unsigned char pal[9];

	OptionInfoPtr	Options;
	unsigned char *buffers[1];
	XAAInfoRecPtr	AccelInfoRec;

	/* Glint stuff */
	GCPtr		CurrentGC;
	DrawablePtr		CurrentDrawable;
	int		IOOffset;
	int		FIFOSize;
	int		InFifoSpace;
	int		pprod;
	CARD32		ROP;
	CARD32		FrameBufferReadMode;
	CARD32		BltScanDirection;
	CARD32		TexMapFormat;
	CARD32		PixelWidth;
	Bool		ClippingOn;
	CARD32		ForeGroundColor;
	CARD32		BackGroundColor;
	int		bppalign;
	CARD32		startxdom;
	CARD32		startxsub;
	CARD32		starty;
	CARD32		count;
	CARD32		dy;
	CARD32		x;
	CARD32		y;
	CARD32		w;
	CARD32		h;
	CARD32		dxdom;
	int		dwords;
	int		cpuheight;
	int		cpucount;
	CARD32		planemask;
	int		realWidth;

} AG10ERec, *AG10EPtr;

extern int  AG10EScreenPrivateIndex;
extern int  AG10EGCPrivateIndex;
extern int  AG10EWindowPrivateIndex;

#define GET_AG10E_FROM_SCRN(p)    ((AG10EPtr)((p)->driverPrivate))

#define AG10EGetScreenPrivate(s)						\
((AG10EPtr) (s)->devPrivates[AG10EScreenPrivateIndex].ptr)

#define AG10EGetGCPrivate(g)						\
((AG10EPrivGCPtr) (g)->devPrivates [AG10EGCPrivateIndex].ptr)

#define AG10EGetWindowPrivate(w)						\
((AG10EStipplePtr) (w)->devPrivates[AG10EWindowPrivateIndex].ptr)
                            
#define AG10ESetWindowPrivate(w,p) 					\
((w)->devPrivates[AG10EWindowPrivateIndex].ptr = (pointer) p)

int AG10EAccelInit(ScreenPtr);
Bool AG10EDGAInit(ScreenPtr);

#endif /* AG10E_H */
