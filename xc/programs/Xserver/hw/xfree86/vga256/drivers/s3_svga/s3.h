/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3.h,v 1.1.2.2 1998/10/18 20:42:31 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */
#ifndef _S3_H_
#define _S3_H_

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"
#include "vgaPCI.h"

/* s3InfoRec needs to be defined as the directory everything is in. */

#define s3InfoRec S3_SVGA 

/* uncomment S3_DEBUG to get a comfortable level of debugging info (MArk) */
/* #define S3_DEBUG */

#include "s3reg.h"

/* Structure definitions */

typedef struct {
   vgaHWRec std;                /* good old IBM VGA */
   unsigned char Clock;   /* Do I need this here? (MArk) */
   unsigned char s3DacRegs[0x101];  
   unsigned char s3reg[10];     /* Video Atribute (CR30-34, CR38-3C) */
   unsigned char s3sysreg[46];  /* Video Atribute (CR40-6D)*/
   unsigned char ColorStack[8]; /* S3 hw cursor color stack CR4A/CR4B */
}
vgaS3Rec, *vgaS3Ptr;

typedef struct {
    char *DacName;
    int DacSpeeds[MAXDACSPEEDS];
    Bool (*DacProbe)();
    int (*PreInit)();
    void (*DacRestore)(vgaS3Ptr);
    void (*DacSave)(vgaS3Ptr);
    int (*DacInit)(DisplayModePtr);
} s3RamdacInfo;


/* Global variable declarations */

extern int   s3maxRawClock;
extern int   s3BppDisplayWidth;
extern int   s3CursorBytes;
extern int   s3MaxClock;
extern int   s3maxDisplayWidth;
extern int   s3maxDisplayHeight;
extern int   s3numClocks;
extern int   s3ScissB;
extern int   s3ScissR;
extern int   s3BankSize;
extern int   s3HDisplay;
extern int   s3DisplayWidth;
extern unsigned short s3ChipRev;
extern unsigned short s3ChipId;
extern short s3BiosVendor;
extern short s3RamdacType;
extern short s3Weight;
extern short s3Bpp;
extern short s3alu[16];
extern char  s3Mbanks;
extern char *s3ClockChipProbed;
extern Bool  s3Localbus;
extern Bool  s3PCIRetry;
extern Bool  s3VLB;
extern Bool  s3DAC8Bit;
extern Bool  s3DACSyncOnGreen;
extern Bool  s3PixelMultiplexing;
extern Bool  s3Bt485PixMux;
extern Bool  s3ATT498PixMux;
extern Bool  s3PowerSaver;
extern Bool  s3clockDoublingPossible;
extern Bool  s3Initialized;
extern Bool  s3newmmio;
extern unsigned char s3Port31;
extern unsigned char s3Port51;
extern unsigned char s3Port59;
extern unsigned char s3Port5A;
extern unsigned char s3LinApOpt;
extern unsigned char s3SAM256;
extern unsigned char s3DACBoarder;
extern unsigned char s3SwapBits[256];
extern ScreenPtr s3savepScreen;
extern DisplayModePtr s3CurrentMode;

extern pointer s3MmioMem;

extern int vgaCRIndex;
extern int vgaCRReg;

extern Bool  (*s3ClockSelectFunc)();

extern int   S3ValidMode(DisplayModePtr, Bool, int);
extern int   S3GetWidth(int);
extern char* S3Ident(int);
extern void* S3Save(vgaS3Ptr);
extern void  S3Restore(vgaS3Ptr);
extern void  S3RetraceWait(void);
extern void  S3BankZero(void);
extern void  S3Unlock(void);
extern void  S3FbInit();
extern void  S3SetRead();
extern void  S3CleanUp(void);
extern void  S3FillInModeInfo(DisplayModePtr);
extern void  S3EnterLeave(Bool);
extern void  S3Adjust(int,int);
extern void  S3AccelInit(void);
extern void  S3AccelInit_NewMMIO(void);
extern Bool  S3InitLevelOne(DisplayModePtr);
extern Bool  S3InitLevelTwo(DisplayModePtr);
extern Bool  S3Probe(void);
extern Bool  S3InitLevelThree(DisplayModePtr);
extern Bool  S3Init(DisplayModePtr);
extern void  S3CursorInit();
extern unsigned char S3MuxOrNot(DisplayModePtr);
extern void S3SavePalette(LUTENTRY*);
extern void S3RestorePalette(LUTENTRY*);

extern void (* dacOutTi3026IndReg)(unsigned char,unsigned char,unsigned char);
extern unsigned char (* dacInTi3026IndReg)(unsigned char);
extern void s3OutTi3026IndReg(unsigned char, unsigned char, unsigned char);
extern unsigned char s3InTi3026IndReg(unsigned char);
extern void s3OutTiIndReg(unsigned char, unsigned char, unsigned char);
extern unsigned char s3InTiIndReg(unsigned char);
  


extern vgaVideoChipRec s3InfoRec;
extern s3RamdacInfo s3Ramdacs[];

/* DAC Numbers */

#define UNKNOWN_DAC       -1
#define NORMAL_DAC         0
#define S3_TRIO32_DAC      1
#define S3_TRIO64_DAC      2
#define TI3026_DAC         3
#define TI3030_DAC         4
#define TI3020_DAC         5
#define TI3025_DAC         6
#define BT485_DAC          7
#define ATT20C505_DAC      8
#define ATT22C498_DAC      9
#define ATT498_DAC        10
#define ATT20C498_DAC      ATT498_DAC
#define ATT20C409_DAC     11
#define SC15025_DAC       12
#define STG1700_DAC       13
#define STG1703_DAC       14
#define IBMRGB524_DAC     15
#define IBMRGB525_DAC     16
#define IBMRGB528_DAC     17
#define S3_SDAC_DAC       18
#define S3_GENDAC_DAC     19
#define ATT20C490_DAC     20
#define	SS2410_DAC	  21
#define SC1148x_DAC 	  22
#define S3_TRIO64V2_DAC   23
#define S3_TRIO64V_DAC    24

/* DAC Macros */

#define DAC_IS_BT485_SERIES	(s3RamdacType == BT485_DAC || \
				 s3RamdacType == ATT20C505_DAC)
#define DAC_IS_TI3020_SERIES	(s3RamdacType == TI3020_DAC || \
				 s3RamdacType == TI3025_DAC)
#define DAC_IS_TI3020		(s3RamdacType == TI3020_DAC)
#define DAC_IS_TI3025		(s3RamdacType == TI3025_DAC)
#define DAC_IS_TI3026		(s3RamdacType == TI3026_DAC)
#define DAC_IS_TI3030		(s3RamdacType == TI3030_DAC)
#define DAC_IS_ATT20C498	(s3RamdacType == ATT20C498_DAC)
#define DAC_IS_ATT22C498	(s3RamdacType == ATT22C498_DAC)
#define DAC_IS_ATT498		(DAC_IS_ATT20C498 || DAC_IS_ATT22C498)
#define DAC_IS_ATT490		(s3RamdacType == ATT20C490_DAC)
#define DAC_IS_SC15025		(s3RamdacType == SC15025_DAC)
#define DAC_IS_STG1703          (s3RamdacType == STG1703_DAC)
#define DAC_IS_STG1700          (s3RamdacType == STG1700_DAC || DAC_IS_STG1703)
#define DAC_IS_SDAC             (s3RamdacType == S3_SDAC_DAC)
#define DAC_IS_GENDAC           (s3RamdacType == S3_GENDAC_DAC)
#define DAC_IS_TRIO32           (s3RamdacType == S3_TRIO32_DAC)
#define DAC_IS_TRIO64           (s3RamdacType == S3_TRIO64_DAC || s3RamdacType == S3_TRIO64V_DAC)
#define DAC_IS_TRIO64V2         (s3RamdacType == S3_TRIO64V2_DAC)
#define DAC_IS_TRIO             (DAC_IS_TRIO32 || DAC_IS_TRIO64 || DAC_IS_TRIO64V2)
#define DAC_IS_IBMRGB524        (s3RamdacType == IBMRGB524_DAC)
#define DAC_IS_IBMRGB525        (s3RamdacType == IBMRGB525_DAC)
#define DAC_IS_IBMRGB528        (s3RamdacType == IBMRGB528_DAC)
#define DAC_IS_IBMRGB           (DAC_IS_IBMRGB524 || DAC_IS_IBMRGB525 || \
				 DAC_IS_IBMRGB528)
#define DAC_IS_SC1148x		(s3RamdacType == SC1148x_DAC)
#define DAC_IS_ATT20C409	(s3RamdacType == ATT20C409_DAC)
#define DAC_IS_SS2410		(s3RamdacType == SS2410_DAC )

/* Vendor BIOS types */

#define UNKNOWN_BIOS		-1
#define ELSA_BIOS		 1
#define MIRO_BIOS		 2
#define SPEA_BIOS		 3
#define GENOA_BIOS		 4
#define STB_BIOS		 5
#define NUMBER_NINE_BIOS	 6
#define HERCULES_BIOS		 7
#define DIAMOND_BIOS		 8

#define MUSTMUX			1
#define	CANTMUX			2



#endif /* _S3_H_ */
