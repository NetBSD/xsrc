/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/sis/sis_Blitter2.h,v 1.1.2.1 1999/04/23 17:41:59 hohndel Exp $ */


/* Definitions for the SIS engine communication. */
/* These are done using Memory Mapped IO, of the registers */
/* 
 * Modified for Sis by Xavier Ducoin (xavier@rd.lectra.fr) 
 */

/*
 * Definitions for the Second Generation Graphics Engine.
 * 02/23/99.
 *
 */

#define BR(x) sis2Reg32MMIO[x]

#define sisLEFT2RIGHT           0x00010000
#define sisRIGHT2LEFT           0x00000000
#define sisTOP2BOTTOM           0x00020000
#define sisBOTTOM2TOP           0x00000000

#define sisSRCSYSTEM            0x00000010
#define sisSRCVIDEO		0x00000000



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

/* bit 31 2D engine: 1 is idle,
   bit 30 3D engine: 1 is idle,
   bit 29 Command queue: 1 is empty
   */
#define sisBLTSync \
  while(!(*(volatile unsigned short *)(sisMMIOBase + BR(16)+2) & \
	(0x8000))){}
	
/* According to SiS 6326 2D programming guide, 16 bits position at   */
/* 0x82A8 returns queue free. But this don't work, so don't wait     */
/* anything when turbo-queue is enabled. If there are frequent syncs */
/* (as XAA does) this should work. But not for xaa_benchmark :-(     */

#define sisBLTWAIT \
  if (!sisTurboQueue) {\
    while(!(*(volatile unsigned short *)(sisMMIOBase + BR(16)+2) & \
	(0x8000))){}} /* \
    else {while(*(volatile unsigned short *)(sisMMIOBase + BR(10)) < \
	63){}} */

#define sisSETPATMASKREG()\
   ((unsigned char *)(sisMMIOBase + BR(11)))

#define sisSETPATREG()\
   ((unsigned char *)(sisMMIOBase + BR(17)))

#define sisSETCMD(op) \
  *(unsigned long *)(sisMMIOBase + BR(15) ) = op

#define sisSETROP(op) \
   sisROP = (op<<8)

#define sisSETSRCADDR(srcAddr) \
  *(unsigned long *)(sisMMIOBase + BR(0)) = srcAddr

#define sisSETDSTADDR(dstAddr) \
  *(unsigned int *)(sisMMIOBase + BR(4)) = dstAddr

#define sisSETPITCH(srcPitch,dstPitch) \
  *(unsigned short *)(sisMMIOBase + BR(1)) = (srcPitch); \
  *(unsigned short *)(sisMMIOBase + BR(5)) = (dstPitch) 

#define sisSETHEIGHTWIDTH(Height,Width)\
  *(unsigned int *)(sisMMIOBase + BR(6)) = (((Height)&0xFFFF)<<16) | ((Width)&0xFFFF)

#define sisSETDSTHEIGHT(Height)\
  *(unsigned short *)(sisMMIOBase + BR(5)+2) = (Height)


#define sisSETSRCXSRCY(X,Y)\
  *(unsigned int *)(sisMMIOBase + BR(2)) = (((X)&0xFFFF)<<16)| \
      ((Y)&0xFFFF)

#define sisSETDSTXDSTY(X,Y)\
  *(unsigned int *)(sisMMIOBase + BR(3)) = (((X)&0xFFFF)<<16)| \
      ((Y)&0xFFFF)

#define sisSETCLIPTOP(x,y)\
  *(unsigned int *)(sisMMIOBase + BR(13)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETCLIPBOTTOM(x,y)\
  *(unsigned int *)(sisMMIOBase + BR(14)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)


#define sisSETBGCOLOR(bgColor)\
  *(unsigned int *)(sisMMIOBase + BR(10)) = (bgColor)

#define sisSETFGCOLOR(fgColor)\
  *(unsigned int *)(sisMMIOBase + BR(9)) = (fgColor)


#define sisSETPATBGCOLOR(bgColor)\
  *(unsigned int *)(sisMMIOBase + BR(8)) = (bgColor)

#define sisSETPATFGCOLOR(fgColor)\
  *(unsigned int *)(sisMMIOBase + BR(7)) = (fgColor)


