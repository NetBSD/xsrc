/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga.h,v 3.3 1996/10/16 14:43:02 dawes Exp $ */

/*
 * MGA Millennium (MGA2064W) functions
 *
 * Copyright 1996 The XFree86 Project, Inc.
 *
 * Authors
 *		Dirk Hohndel
 *			hohndel@XFree86.Org
 *		David Dawes
 *			dawes@XFree86.Org
 */

#ifndef MGA_H
#define MGA_H

#define MGAREG8(addr) *(volatile CARD8 *)(MGAMMIOBase + (addr))
#define MGAREG16(addr) *(volatile CARD16 *)(MGAMMIOBase + (addr))
#define MGAREG32(addr) *(volatile CARD32 *)(MGAMMIOBase + (addr))
#define MGAREG(addr) MGAREG32(addr)
#define OUTREG(addr, val) MGAREG(addr) = (val)

extern unsigned char *MGAMMIOBase;
extern int MGAScrnWidth;

#define MGAWAITFIFO() while(MGAREG16(MGAREG_FIFOSTATUS) & 0x100);
#define MGAWAITFREE() while(MGAREG8(MGAREG_Status + 2) & 0x01);

#define MGAWAITFIFOSLOTS(SLOTS) while ( ((MGAREG16(MGAREG_FIFOSTATUS) & 0x3f) - SLOTS) < 0 );

void
mgaLine (
#ifdef NeedFunctionPrototypes
	 DrawablePtr pDrawable, 
	 GCPtr pGC, 
	 int mode, 
	 int npt, 
	 DDXPointPtr pptInit
#endif
);

void
mgaPolyFillRect(
#ifdef NeedFunctionPrototypes
	DrawablePtr pDrawable,
	register GCPtr pGC,
	int         nrectFill,
	xRectangle  *prectInit
#endif
);                

void
mgaFillRectSolidCopy(
#ifdef NeedFunctionPrototypes
	DrawablePtr     pDrawable,
	GCPtr           pGC,
	int             nBox,
	BoxPtr          pBox
#endif
);

void
mgaPaintWindow(
#ifdef NeedFunctionPrototypes
	WindowPtr   pWin,
	RegionPtr   pRegion,
	int         what
#endif
);

void
mgaFillBoxSolid(
#ifdef NeedFunctionPrototypes
	DrawablePtr     pDrawable,
	int             nBox,
	BoxPtr          pBox,
	unsigned long   pixel
#endif
);                

int
MGAWaitForBlitter(
#ifdef NeedFunctionPrototypes
	void
#endif
);
#endif
