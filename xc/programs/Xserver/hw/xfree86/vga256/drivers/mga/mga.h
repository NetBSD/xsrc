/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga.h,v 3.8.2.4 1998/02/01 16:05:10 robin Exp $ */
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

#if defined(__alpha__)
#define mb() __asm__ __volatile__("mb": : :"memory")
#define INREG8(addr) xf86ReadSparse8(MGAMMIOBase, (addr))
#define INREG16(addr) xf86ReadSparse16(MGAMMIOBase, (addr))
#define INREG(addr) xf86ReadSparse32(MGAMMIOBase, (addr))
#define OUTREG8(addr,val) do { xf86WriteSparse8((val),MGAMMIOBase,(addr)); \
				mb();} while(0)
#define OUTREG16(addr,val) do { xf86WriteSparse16((val),MGAMMIOBase,(addr)); \
				mb();} while(0)
#define OUTREG(addr, val) do { xf86WriteSparse32((val),MGAMMIOBase,(addr)); \
				mb();} while(0)
#else /* __alpha__ */
#define INREG8(addr) *(volatile CARD8 *)(MGAMMIOBase + (addr))
#define INREG16(addr) *(volatile CARD16 *)(MGAMMIOBase + (addr))
#define INREG(addr) *(volatile CARD32 *)(MGAMMIOBase + (addr))
#define OUTREG8(addr, val) *(volatile CARD8 *)(MGAMMIOBase + (addr)) = (val)
#define OUTREG16(addr, val) *(volatile CARD16 *)(MGAMMIOBase + (addr)) = (val)
#define OUTREG(addr, val) *(volatile CARD32 *)(MGAMMIOBase + (addr)) = (val)
#endif /* __alpha__ */

#define MGAISBUSY() (INREG8(MGAREG_Status + 2) & 0x01)
#define MGAWAITFIFO() while(INREG16(MGAREG_FIFOSTATUS) & 0x100);
#define MGAWAITFREE() while(MGAISBUSY());
#define MGAWAITFIFOSLOTS(slots) while ( ((INREG16(MGAREG_FIFOSTATUS) & 0x3f) - (slots)) < 0 );

typedef struct {
    Bool	isHwCursor;
    int		CursorMaxWidth;
    int 	CursorMaxHeight;
    int		CursorFlags;
    void	(*LoadCursorImage)();
    void	(*ShowCursor)();
    void	(*HideCursor)();
    void	(*SetCursorPosition)();
    void	(*SetCursorColors)();
    long	maxPixelClock;
    long	MemoryClock;
} MGARamdacRec;

extern MGARamdacRec MGAdac;
extern pciTagRec MGAPciTag;
extern int MGAchipset;
extern int MGArev;
extern int MGAinterleave;
extern int MGABppShft;
extern int MGAusefbitblt;
extern int MGAydstorg;
extern unsigned char *MGAMMIOBase;

extern void Mga8AccelInit();
extern void Mga16AccelInit();
extern void Mga24AccelInit();
extern void Mga32AccelInit();
extern void MGAStormAccelInit();
extern void MGAStormSync();
extern void MGAStormEngineInit();
extern void MGA3026RamdacInit();
extern void MGA1064RamdacInit();

#ifdef __alpha__
extern unsigned char *MGAMMIOBaseDENSE;
#endif

/*
 * ROPs
 *
 * for some silly reason, the bits in the ROPs are just the other way round
 */

/*
 * definitions for the new acceleration interface
 */
#define WAITUNTILFINISHED()	MGAWAITFREE()
#define SETBACKGROUNDCOLOR(col)	OUTREG(MGAREG_BCOL, (col))
#define SETFOREGROUNDCOLOR(col)	OUTREG(MGAREG_FCOL, (col))
#define SETRASTEROP(rop)	mga_cmd |= MGARop[rop]
#define SETWRITEPLANEMASK(pm)	OUTREG(MGAREG_PLNWT, (pm))
#define SETBLTXYDIR(x,y)	OUTREG(MGAREG_SGN, ((-x+1)>>1)+4*((-y+1)>>1))

#endif
