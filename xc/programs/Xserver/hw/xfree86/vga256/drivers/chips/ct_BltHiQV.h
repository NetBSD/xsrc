/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/chips/ct_BltHiQV.h,v 3.1 1996/09/29 13:39:13 dawes Exp $ */

/* Definitions for the Chips and Technology BitBLT engine communication. */
/* These are done using Memory Mapped IO, of the registers */
/* BitBLT modes for register 93D0. */

#define ctPATCOPY               0xF0
#define ctLEFT2RIGHT            0x000
#define ctRIGHT2LEFT            0x100
#define ctTOP2BOTTOM            0x000
#define ctBOTTOM2TOP            0x200
#define ctSRCSYSTEM             0x400
#define ctSRCMONO               0x1000
#define ctBGTRANSPARENT         0x22000
#define ctPATMONO               0x40000L
#define ctPATSOLID              0x80000L
#define ctPATSTART0             0x000000L
#define ctPATSTART1             0x100000L
#define ctPATSTART2             0x200000L
#define ctPATSTART3             0x300000L
#define ctPATSTART4             0x400000L
#define ctPATSTART5             0x500000L
#define ctPATSTART6             0x600000L
#define ctPATSTART7             0x700000L
#define ctSRCFG                 0x000000L	/* Where is this for the 65550?? */

/* The Monochrome expansion register setup */
#define ctCLIPLEFT(clip)        (clip&0x3F)
#define ctCLIPRIGHT(clip)       ((clip&0x3F) << 8)
#define ctSRCDISCARD(clip)      ((clip&0x3F) << 16)
#define ctBITALIGN              0x1000000L
#define ctBYTEALIGN             0x2000000L
#define ctWORDALIGN             0x3000000L
#define ctDWORDALIGN            0x4000000L
#define ctQWORDALIGN            0x5000000L
/* This shouldn't be used because not all chip rev's
 * have BR09 and BR0A, and I haven't even defined
 * macros to write to these registers 
 */
#define ctEXPCOLSEL             0x8000000L

/* Macros to do useful things with the C&T BitBLT engine */
#define ctBLTWAIT \
  while(*(volatile unsigned int *)(ctMMIOBase + BR(0x4)) & \
	(0x80000000)){}

#define ctSETROP(op) \
  *(unsigned int *)(ctMMIOBase + BR(0x4)) = op

#define ctSETMONOCTL(op) \
  *(unsigned int *)(ctMMIOBase + BR(0x3)) = op

#define ctSETSRCADDR(srcAddr) \
  *(unsigned int *)(ctMMIOBase + BR(0x6)) = srcAddr&0x7FFFFFL

#define ctSETDSTADDR(dstAddr) \
  *(unsigned int *)(ctMMIOBase + BR(0x7)) = dstAddr&0x7FFFFFL

#define ctSETPITCH(srcPitch,dstPitch) \
  *(unsigned int *)(ctMMIOBase + BR(0x0)) = ((dstPitch&0xFFFF)<<16)| \
      (srcPitch&0xFFFF)

#define ctSETHEIGHTWIDTHGO(Height,Width)\
  *(unsigned int *)(ctMMIOBase + BR(0x8)) = ((Height&0xFFFF)<<16)| \
      (Width&0xFFFF)

#define ctSETPATSRCADDR(srcAddr)\
  *(unsigned int *)(ctMMIOBase + BR(0x5)) = srcAddr&0x1FFFFFL

#define ctSETBGCOLOR8(bgColor)\
  *(unsigned int *)(ctMMIOBase + BR(0x1)) = (bgColor&0xFF)

#define ctSETBGCOLOR16(bgColor)\
  *(unsigned int *)(ctMMIOBase + BR(0x1)) = (bgColor&0xFFFF)

#define ctSETBGCOLOR24(bgColor)\
  *(unsigned int *)(ctMMIOBase + BR(0x1)) = (bgColor&0xFFFFFF)

#define ctSETFGCOLOR8(fgColor)\
  *(unsigned int *)(ctMMIOBase + BR(0x2)) = (fgColor&0xFF)

#define ctSETFGCOLOR16(fgColor)\
  *(unsigned int *)(ctMMIOBase + BR(0x2)) = (fgColor&0xFFFF)

#define ctSETFGCOLOR24(fgColor)\
  *(unsigned int *)(ctMMIOBase + BR(0x2)) = (fgColor&0xFFFFFF)

