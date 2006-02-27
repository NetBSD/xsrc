/*
 * SBus Weitek P9100 driver - defines
 *
 * Copyright (C) 2005 Michael Lorenz
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
 * MICHAEL LORENZ BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $NetBSD: pnozz.h,v 1.4 2006/02/27 18:19:53 macallan Exp $ */

#ifndef PNOZZ_H
#define PNOZZ_H

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86RamDac.h"
#include "Xmd.h"
#include "gcstruct.h"
#include "pnozz_regs.h"
#include "xf86sbusBus.h"
#include "xaa.h"

typedef struct {
	unsigned int fg, bg;			/* FG/BG colors for stipple */
	unsigned int patalign;                  /* X/Y alignment of bits */
        unsigned int alu;			/* Transparent/Opaque + rop */
        unsigned int bits[32];                  /* The stipple bits themselves */
} PnozzStippleRec, *PnozzStipplePtr;

typedef struct {
	int type;
	PnozzStipplePtr stipple;
} PnozzPrivGCRec, *PnozzPrivGCPtr;

typedef struct {
	unsigned char	*fb;	/* 2MB framebuffer */
	unsigned char	*fbc;	/* registers, so we can just add a byte offset */
	int		vclipmax;
	int		width;
	int		height, scanlinesize, maxheight;
	int		depthshift;

	sbusDevicePtr	psdp;
	Bool		HWCursor;
	Bool		NoAccel;
	CloseScreenProcPtr CloseScreen;
	
	xf86CursorInfoPtr CursorInfoRec;
	struct fbcursor Cursor;
	unsigned char pal[9];
	
	OptionInfoPtr	Options;
	XAAInfoRecPtr	pXAA;
	unsigned char	*buffers[2];
	/*
	 * XXX this is enough for everything a SPARCbook could do on it's
	 * internal display but not necessarily for an external one
	 */
	CARD32		Buffer[6400];
	int		words, last_word;
	int		offset_mask;

	int		DidSave;
	unsigned int	SvSysConf;	/* System Configuration Register */
	unsigned int	CRTC[4];	/* CRTC values for horizontal timing */
	unsigned int	SvMemCtl;	/* memory control register */
	unsigned char	SvDAC_MCCR;	/* DAC Misc Clock Ctrl (0x02) */
	unsigned char	SvDAC_PF;	/* DAC Pixel Format (0x0a) */
	unsigned char	SvDAC_MC3;	/* DAC Misc Control 3 */
	unsigned char	SvVCO;		/* DAC System PLL VCO divider */
	unsigned char	SvPLL;		/* clock multiplier / divider */
	
} PnozzRec, *PnozzPtr;

extern int  PnozzScreenPrivateIndex;
extern int  PnozzGCPrivateIndex;
extern int  PnozzWindowPrivateIndex;

#define GET_PNOZZ_FROM_SCRN(p)    ((p->driverPrivate))

#define PnozzGetScreenPrivate(s)						\
((PnozzPtr) (s)->devPrivates[PnozzScreenPrivateIndex].ptr)

#define PnozzGetGCPrivate(g)						\
((PnozzPrivGCPtr) (g)->devPrivates [PnozzGCPrivateIndex].ptr)

#define PnozzGetWindowPrivate(w)						\
((PnozzStipplePtr) (w)->devPrivates[PnozzWindowPrivateIndex].ptr)
                            
#define PnozzSetWindowPrivate(w,p) 					\
((w)->devPrivates[PnozzWindowPrivateIndex].ptr = (pointer) p)

void pnozz_write_4(PnozzPtr, int, unsigned int);
unsigned int pnozz_read_4(PnozzPtr, int);
void pnozz_write_dac(PnozzPtr, int, unsigned char);
unsigned char pnozz_read_dac(PnozzPtr, int);
void pnozz_write_dac_ctl_reg(PnozzPtr, int, unsigned char);
void pnozz_write_dac_ctl_reg_2(PnozzPtr, int, unsigned short);
unsigned char pnozz_read_dac_ctl_reg(PnozzPtr, int);
void pnozz_write_dac_cmap_reg(PnozzPtr, int, unsigned int);

int PnozzAccelInit(ScrnInfoPtr);
void PnozzHideCursor(ScrnInfoPtr);
void PnozzShowCursor(ScrnInfoPtr);

#endif /* CG6_H */
