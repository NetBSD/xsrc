/*
 * Copyright 1998,1999 by Alan Hourihane, Wigan, England.
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
 * Authors:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *           Mike Chapman <mike@paranoia.com>, 
 *           Juanjo Santamarta <santamarta@ctv.es>, 
 *           Mitani Hiroshi <hmitani@drl.mei.co.jp> 
 *           David Thomas <davtom@dream.org.uk>. 
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_regs.h,v 1.14 2000/12/02 01:16:18 dawes Exp $ */

#include "vgaHW.h"

#define	inSISREG(base)			inb(base)
#define	outSISREG(base,val)		outb(base,val)
#define orSISREG(base,val)		do { \
					  unsigned char temp = inb(base); \
					  outSISREG(base, temp | (val)); \
					} while (0)
#define andSISREG(base,val)		do { \
					  unsigned char temp = inb(base); \
					  outSISREG(base, temp & (val)); \
					} while (0)

#define	inSISIDXREG(base,idx,var)	do { \
					  outb(base,idx); var=inb((base)+1); \
					} while (0)
#if 0
#define	outSISIDXREG(base,idx,val)	outw(base, (val)<<8 | (idx));
#endif
#define	outSISIDXREG(base,idx,val)	do { \
					  outb(base,idx); outb((base)+1,val); \
					} while (0)
#define orSISIDXREG(base,idx,val)	do { \
					  unsigned char temp; \
					  outb(base,idx);	\
					  temp = inb((base)+1)|(val); \
					  outSISIDXREG(base,idx,temp); \
					} while (0)
#define andSISIDXREG(base,idx,and)	do { \
					  unsigned char temp; \
					  outb(base,idx);	\
					  temp = inb((base)+1)&(and); \
					  outSISIDXREG(base,idx,temp); \
					} while (0)
#define	setSISIDXREG(base,idx,and,or)	do { \
					  unsigned char temp; \
					  outb(base,idx);	\
					  temp = (inb((base)+1)&(and))|(or); \
					  outSISIDXREG(base,idx,temp); \
					} while (0)

#define	BITMASK(h,l)	(((unsigned)(1U << ((h)-(l)+1))-1)<<(l))
#define	GENMASK(mask)	BITMASK(1?mask,0?mask)

#define	GETBITS(var,mask)	(((var) & GENMASK(mask)) >> (0?mask))
#define	SETBITS(val,mask)	((val) << (0?mask))
#define	SETBIT(n)		(1<<(n))

#define	GETBITSTR(val,from,to)	((GETBITS(val,from)) << (0?to))
#define	SETVARBITS(var,val,from,to)	(((var)&(~(GENMASK(to)))) | \
					GETBITSTR(val,from,to))
#define	GETVAR8(var)		((var)&0xFF)
#define	SETVAR8(var,val)	(var) =  GETVAR8(val)

#define	VGA_RELIO_BASE	0x380

#define	AROFFSET	VGA_ATTR_INDEX - VGA_RELIO_BASE
#define	ARROFFSET	VGA_ATTR_DATA_R - VGA_RELIO_BASE
#define	GROFFSET	VGA_GRAPH_INDEX - VGA_RELIO_BASE
#define	SROFFSET	VGA_SEQ_INDEX - VGA_RELIO_BASE
#define	CROFFSET	VGA_CRTC_INDEX_OFFSET + VGA_IOBASE_COLOR-VGA_RELIO_BASE
#define	MISCROFFSET	VGA_MISC_OUT_R - VGA_RELIO_BASE
#define	MISCWOFFSET	VGA_MISC_OUT_W - VGA_RELIO_BASE

#define	SISAR		pSiS->RElIO+AROFFSET
#define	SISARR		pSiS->RELIO+ARROFFSET
#define	SISGR		pSiS->RELIO+GROFFSET
#define SISSR		pSiS->RelIO+SROFFSET
#define	SISCR		pSiS->RelIO+CROFFSET
#define	SISMISCR	pSiS->RelIO+MISCROFFSET
#define	SISMISCW	pSiS->RelIO+MISCWOFFSET
#define SISPART1        pSiS->RelIO+0x04
#define SISPART2        pSiS->RelIO+0x10
#define SISPART3        pSiS->RelIO+0x12
#define SISPART4        pSiS->RelIO+0x14 
#define SISPART5        pSiS->RelIO+0x16 


/* 3C4 */
#define BankReg 0x06
#define ClockReg 0x07
#define CPUThreshold 0x08
#define CRTThreshold 0x09
#define CRTCOff 0x0A
#define DualBanks 0x0B
#define MMIOEnable 0x0B
#define RAMSize 0x0C
#define Mode64 0x0C
#define ExtConfStatus1 0x0E
#define ClockBase 0x13
#define LinearAdd0 0x20
#define LinearAdd1 0x21
#define GraphEng 0x27
#define MemClock0 0x28
#define MemClock1 0x29
#define XR2A 0x2A
#define XR2B 0x2B
#define TurboQueueBase 0x2C
#define FBSize 0x2F
#define ExtMiscCont5 0x34
#define ExtMiscCont9 0x3C

/* 3x4 */
#define Offset 0x13

#define read_xr(num,var) do {outb(0x3c4, num);var=inb(0x3c5);} while (0)

/* Definitions for the SIS engine communication. */

extern int sisReg32MMIO[];
#define BR(x) sisReg32MMIO[x]

/* These are done using Memory Mapped IO, of the registers */
/* 
 * Modified for Sis by Xavier Ducoin (xavier@rd.lectra.fr) 
 */


#define sisLEFT2RIGHT           0x10
#define sisRIGHT2LEFT           0x00
#define sisTOP2BOTTOM           0x20
#define sisBOTTOM2TOP           0x00

#define sisSRCSYSTEM            0x03
#define sisSRCVIDEO		0x02
#define sisSRCFG		0x01
#define sisSRCBG		0x00

#define sisCMDBLT		0x0000
#define sisCMDBLTMSK		0x0100
#define sisCMDCOLEXP		0x0200
#define sisCMDLINE		0x0300

#define sisCMDENHCOLEXP		0x2000

#define sisXINCREASE		0x10
#define sisYINCREASE		0x20
#define sisCLIPENABL		0x40
#define sisCLIPINTRN		0x80 
#define sisCLIPEXTRN		0x00


#define sisPATREG		0x08
#define sisPATFG		0x04
#define sisPATBG		0x00

#define sisLASTPIX		0x0800
#define sisXMAJOR		0x0400


/* Macros to do useful things with the SIS BitBLT engine */

#define sisBLTSync \
  while(*(volatile unsigned short *)(pSiS->IOBase + BR(10)+2) & \
	(0x4000)){}

/* According to SiS 6326 2D programming guide, 16 bits position at   */
/* 0x82A8 returns queue free. But this don't work, so don't wait     */
/* anything when turbo-queue is enabled. If there are frequent syncs */
/* this should work. But not for xaa_benchmark :-(     */

#define sisBLTWAIT \
  if (!pSiS->TurboQueue) {\
    while(*(volatile unsigned short *)(pSiS->IOBase + BR(10)+2) & \
	(0x4000)){}} /* \
    else {while(*(volatile unsigned short *)(pSiS->IOBase + BR(10)) < \
	63){}} */

#define sisSETPATREG()\
   ((unsigned char *)(pSiS->IOBase + BR(11)))

#define sisSETPATREGL()\
   ((unsigned long *)(pSiS->IOBase + BR(11)))

#define sisSETCMD(op) \
  *(volatile unsigned short *)(pSiS->IOBase + BR(10) +2 ) = op

#define sisSETROPFG(op) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = ((*(volatile unsigned int *)(pSiS->IOBase + BR(4)))&0xffffff) | (op<<24)

#define sisSETROPBG(op) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = ((*(volatile unsigned int *)(pSiS->IOBase + BR(5)))&0xffffff) | (op<<24)

#define sisSETROP(op) \
   sisSETROPFG(op);sisSETROPBG(op);


#define sisSETSRCADDR(srcAddr) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(0)) = srcAddr&0x3FFFFFL

#define sisSETDSTADDR(dstAddr) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(1)) = dstAddr&0x3FFFFFL

#define sisSETPITCH(srcPitch,dstPitch) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(2)) = ((dstPitch&0xFFFF)<<16)| \
      (srcPitch&0xFFFF)

/* according to SIS 2D Engine Programming Guide 
 * width -1 independant of Bpp
 */ 
#define sisSETHEIGHTWIDTH(Height,Width)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(3)) = (((Height)&0xFFFF)<<16)| \
      ((Width)&0xFFFF)

#define sisSETCLIPTOP(x,y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(8)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETCLIPBOTTOM(x,y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(9)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETBGCOLOR(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor)

#define sisSETBGCOLOR8(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor&0xFF)

#define sisSETBGCOLOR16(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor&0xFFFF)

#define sisSETBGCOLOR24(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(5)) = (bgColor&0xFFFFFF)


#define sisSETFGCOLOR(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor)

#define sisSETFGCOLOR8(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor&0xFF)

#define sisSETFGCOLOR16(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor&0xFFFF)

#define sisSETFGCOLOR24(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = (fgColor&0xFFFFFF)

/* Line drawing */

#define sisSETXStart(XStart) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(0)) = XStart&0xFFFF

#define sisSETYStart(YStart) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(1)) = YStart&0xFFFF

#define sisSETLineMajorCount(MajorAxisCount) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(3)) = MajorAxisCount&0xFFFF

#define sisSETLineSteps(K1,K2) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(6)) = (((K1)&0xFFFF)<<16)| \
      ((K2)&0xFFFF)

#define sisSETLineErrorTerm(ErrorTerm) \
  *(volatile unsigned short *)(pSiS->IOBase + BR(7)) = ErrorTerm


