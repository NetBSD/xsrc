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
 *           Xavier Ducoin <x.ducoin@lectra.com>
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_regs2.h,v 1.3 2000/02/12 20:45:36 dawes Exp $ */

/* 3C4 */
#define BankReg 0x06
#define ExtGraphicControl 0x06
#define ClockReg 0x07
#define CPUThreshold 0x08
#define CRTThreshold 0x09
#define CRTCOff 0x0A
#define DualBanks 0x0B
#define MMIOEnable 0x0B
#define RAMSize 0x0C
#define Mode64 0x0C
#define ExtConfStatus1 0x0E
#define ExtHoznOver 0x12
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

extern int sis2Reg32MMIO[];
#define BR(x) sis2Reg32MMIO[x]

#define sisLEFT2RIGHT           0x00010000
#define sisRIGHT2LEFT           0x00000000
#define sisTOP2BOTTOM           0x00020000
#define sisBOTTOM2TOP           0x00000000

#define sisSRCSYSTEM            0x00000010
#define sisSRCVIDEO		0x00000000

#define sisNOMERGECLIP		0x04000000

#define sisCMDBLT		0x00000000
#define sisCMDCOLEXP		0x00000001
#define sisCMDLINE		0x00000004
#define sisCMDENHCOLEXP		0x00000002

#define sisTRANSPARENT		0x00100000

#define sisCLIPINTRN		0x00000000
#define sisCLIPEXTRN		0x04000000
#define sisCLIPENABL		0x00040000

#define sisPATFG		0x00000000	
#define sisPATREG		0x00000040	
#define sisPATMASK		0x00000080	

/* Macros to do useful things with the SIS BitBLT engine */

/* 
   bit 31 2D engine: 1 is idle,
   bit 30 3D engine: 1 is idle,
   bit 29 Command queue: 1 is empty
*/
#define sisBLTSync \
  while((*(volatile unsigned short *)(pSiS->IOBase + BR(16)+2) & \
	0xE000) != 0xE000){}
	
#define sisBLTWAIT \
  if (!pSiS->TurboQueue) {\
    while(!(*(volatile unsigned short *)(pSiS->IOBase + BR(16)+2) & \
	(0x8000))){}} 

#define sisSETPATMASKREG()\
   ((unsigned char *)(pSiS->IOBase + BR(11)))

#define sisSETPATREG()\
   ((unsigned char *)(pSiS->IOBase + BR(17)))

#define sisSETCMD(op) \
  *(volatile unsigned long *)(pSiS->IOBase + BR(15) ) = op

#define sisSETROP(op) \
   pSiS->ROPReg = (op<<8)

#define sisROP pSiS->ROPReg

#define sisSETSRCADDR(srcAddr) \
  *(volatile unsigned long *)(pSiS->IOBase + BR(0)) = srcAddr

#define sisSETDSTADDR(dstAddr) \
  *(volatile unsigned int *)(pSiS->IOBase + BR(4)) = dstAddr

#define sisSETPITCH(srcPitch,dstPitch) \
  *(volatile unsigned short *)(pSiS->IOBase + BR(1)) = (srcPitch); \
  *(volatile unsigned short *)(pSiS->IOBase + BR(5)) = (dstPitch) 

#define sisSETHEIGHTWIDTH(Height,Width)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(6)) = (((Height)&0xFFFF)<<16) | ((Width)&0xFFFF)

#define sisSETDSTHEIGHT(Height)\
  *(volatile unsigned short *)(pSiS->IOBase + BR(5)+2) = (Height)


#define sisSETSRCXSRCY(X,Y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(2)) = (((X)&0xFFFF)<<16)| \
      ((Y)&0xFFFF)

#define sisSETDSTXDSTY(X,Y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(3)) = (((X)&0xFFFF)<<16)| \
      ((Y)&0xFFFF)

#define sisSETCLIPTOP(x,y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(13)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETCLIPBOTTOM(x,y)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(14)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)


#define sisSETBGCOLOR(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(10)) = (bgColor)

#define sisSETFGCOLOR(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(9)) = (fgColor)


#define sisSETPATBGCOLOR(bgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(8)) = (bgColor)

#define sisSETPATFGCOLOR(fgColor)\
  *(volatile unsigned int *)(pSiS->IOBase + BR(7)) = (fgColor)


#define sisEnableCRT1HWCursor()\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8500) |= 0x40000000;
#define sisDisableCRT1HWCursor()\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8500) &= 0xBFFFFFFF;

#define sisSetCRT1CursorBGColor(color)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8504) = (color);
#define sisSetCRT1CursorFGColor(color)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8508) = (color);

#define sisSetCRT1CursorPositionX(x,preset)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x850C) = (x) | ((preset) << 16);
#define sisSetCRT1CursorPositionY(y,preset)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8510) = (y) | ((preset) << 16);

#define sisEnableCRT2HWCursor()\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8520) |= 0x40000000;
#define sisDisableCRT2HWCursor()\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8520) &= 0xBFFFFFFF;

#define sisSetCRT2CursorBGColor(color)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8524) = (color);
#define sisSetCRT2CursorFGColor(color)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8528) = (color);

#define sisSetCRT2CursorPositionX(x,preset)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x852C) = (x) | ((preset) << 16);
#define sisSetCRT2CursorPositionY(y,preset)\
  *(volatile unsigned int *)(pSiS->IOBase + 0x8530) = (y) | ((preset) << 16);

