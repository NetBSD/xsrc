/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/sis/sis_Blitter.h,v 1.2.2.5 1999/05/15 13:53:32 dawes Exp $ */


/* Definitions for the SIS engine communication. */
/* These are done using Memory Mapped IO, of the registers */
/* 
 * Modified for Sis by Xavier Ducoin (xavier@rd.lectra.fr) 
 */

#define BR(x) sisReg32MMIO[x]

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

#define sisCLIPINTRN		0x00
#define sisCLIPEXTRN		0x80

#define sisCLIPENABL		0x40

#define sisPATREG		0x08
#define sisPATFG		0x04
#define sisPATBG		0x00


/* Macros to do useful things with the SIS BitBLT engine */

#define sisBLTSync \
  while(*(volatile unsigned short *)(sisMMIOBase + BR(10)+2) & \
	(0x4000)){}
	
/* According to SiS 6326 2D programming guide, 16 bits position at   */
/* 0x82A8 returns queue free. But this don't work, so don't wait     */
/* anything when turbo-queue is enabled. If there are frequent syncs */
/* (as XAA does) this should work. But not for xaa_benchmark :-(     */

#define sisBLTWAIT \
  if (!sisTurboQueue) {\
    while(*(volatile unsigned short *)(sisMMIOBase + BR(10)+2) & \
	(0x4000)){}} /* \
    else {while(*(volatile unsigned short *)(sisMMIOBase + BR(10)) < \
	63){}} */

#define sisSETPATREG()\
   ((volatile unsigned char *)(sisMMIOBase + BR(11)))

#define sisSETPATREGL()\
   ((volatile unsigned long *)(sisMMIOBase + BR(11)))

#define sisSETCMD(op) \
  *(volatile unsigned short *)(sisMMIOBase + BR(10) +2 ) = op

#define sisSETROPFG(op) \
  *(volatile unsigned int *)(sisMMIOBase + BR(4)) = ((*(volatile unsigned int *)(sisMMIOBase + BR(4)))&0xffffff) | (op<<24)

#define sisSETROPBG(op) \
  *(volatile unsigned int *)(sisMMIOBase + BR(5)) = ((*(volatile unsigned int *)(sisMMIOBase + BR(5)))&0xffffff) | (op<<24)

#define sisSETROP(op) \
   sisSETROPFG(op);sisSETROPBG(op);


#define sisSETSRCADDR(srcAddr) \
  *(volatile unsigned int *)(sisMMIOBase + BR(0)) = srcAddr&0x3FFFFFL

#define sisSETDSTADDR(dstAddr) \
  *(volatile unsigned int *)(sisMMIOBase + BR(1)) = dstAddr&0x3FFFFFL

#define sisSETPITCH(srcPitch,dstPitch) \
  *(volatile unsigned int *)(sisMMIOBase + BR(2)) = ((dstPitch&0xFFFF)<<16)| \
      (srcPitch&0xFFFF)

/* according to SIS 2D Engine Programming Guide 
 * width -1 independant of Bpp
 */ 
#define sisSETHEIGHTWIDTH(Height,Width)\
  *(volatile unsigned int *)(sisMMIOBase + BR(3)) = (((Height)&0xFFFF)<<16)| \
      ((Width)&0xFFFF)

#define sisSETCLIPTOP(x,y)\
  *(volatile unsigned int *)(sisMMIOBase + BR(8)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETCLIPBOTTOM(x,y)\
  *(volatile unsigned int *)(sisMMIOBase + BR(9)) = (((y)&0xFFFF)<<16)| \
      ((x)&0xFFFF)

#define sisSETBGCOLOR(bgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(5)) = (bgColor)

#define sisSETBGCOLOR8(bgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(5)) = (bgColor&0xFF)

#define sisSETBGCOLOR16(bgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(5)) = (bgColor&0xFFFF)

#define sisSETBGCOLOR24(bgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(5)) = (bgColor&0xFFFFFF)


#define sisSETFGCOLOR(fgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(4)) = (fgColor)

#define sisSETFGCOLOR8(fgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(4)) = (fgColor&0xFF)

#define sisSETFGCOLOR16(fgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(4)) = (fgColor&0xFFFF)

#define sisSETFGCOLOR24(fgColor)\
  *(volatile unsigned int *)(sisMMIOBase + BR(4)) = (fgColor&0xFFFFFF)










