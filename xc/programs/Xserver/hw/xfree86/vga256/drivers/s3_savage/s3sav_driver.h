/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_driver.h,v 1.1.2.2 1999/12/01 12:49:34 hohndel Exp $ */

/* Header file for ViRGE server */

/* Declared in s3v_driver.c */
extern vgaVideoChipRec S3V;
extern vgaCRIndex, vgaCRReg;

/* Driver variables */

#if defined(linux) && defined(__i386__)
#define	USEBIOS
#endif

/* Driver data structure; this should contain all neeeded info for a mode */
typedef struct {     
   vgaHWRec std;
   unsigned int mode;
   unsigned int refresh;

   unsigned char SR8, SR10, SR11, SR12, SR13, SR15, SR18, SR29; /* SR9-SR1C, ext seq. */
   /*unsigned char SR54, SR55, SR56, SR57;*/
   unsigned char Clock;
   unsigned char s3DacRegs[0x101];
   unsigned char CR31, CR33, CR34, CR36, CR3A, CR3B, CR3C;
   unsigned char CR40, CR42, CR43, CR45;
   unsigned char CR50, CR51, CR53, CR58, CR5B, CR5D, CR5E;
   unsigned char CR65, CR66, CR67, CR68, CR69, CR6F; /* Video attrib. */
   unsigned char CR86, CR88;
   unsigned char CR90, CR91, CRB0;
   unsigned char ColorStack[8]; /* S3 hw cursor color stack CR4A/CR4B */
   unsigned int  STREAMS[22];   /* Streams regs */
   unsigned int  MMPR0, MMPR1, MMPR2, MMPR3;   /* MIU regs */
} vgaS3VRec, *vgaS3VPtr;

/*
 * This is a convenience macro, so that entries in the driver structure
 * can simply be dereferenced with 'new->xxx'.
 */

#define new ((vgaS3VPtr)vgaNewVideoState)


/* PCI info structure */

typedef struct S3PCIInformation {
   int DevID;
   int ChipType;
   int ChipRev;
   unsigned long MemBase;
   unsigned long MemBase1;
} S3PCIInformation;

/* Private data structure used for storing all variables needed in driver */
/* This is not exported outside of here, so no need to worry about packing */

typedef struct {
   int chip;
   unsigned short ChipId;
   unsigned int MmioBase;
   unsigned int FrameBufferBase;
   pointer MmioMem;
   pointer BciMem;
   int MemOffScreen;
   int HorizScaleFactor;
   Bool STREAMSRunning;
   Bool NeedSTREAMS;
   int Width, Bpp,Bpl, ScissB;
   unsigned PlaneMask;
   int bltbug_width1, bltbug_width2;
   int MCLK;
   Bool NoPCIRetry;

   int (*WaitQueue)(int);
   int (*WaitIdle)(void);
   int (*WaitIdleEmpty)(void);
   int (*WaitCommandEmpty)(void);

} S3VPRIV;


/* VESA BIOS Mode information. */

#ifdef USEBIOS
typedef struct _S3VMODETABLE {
   unsigned short Width;
   unsigned short Height;
   unsigned short VesaMode;
   unsigned char RefreshCount;
   unsigned char RefreshRate[8];
} S3VMODETABLE, * PS3VMODETABLE;
#endif

/* Function prototypes */

S3PCIInformation * S3SAVGetPCIInfo();
extern Bool S3SAVCursorInit();
extern void S3SAVRestoreCursor();
extern void S3SAVWarpCursor();
extern void S3SAVQueryBestSize();
#ifdef USEBIOS
extern unsigned short S3SAVGetBIOSModeCount( int );
extern unsigned short S3SAVGetBIOSModeTable( int, S3VMODETABLE* );
extern void S3SAVSetVESAMode( int, int );
extern void S3SAVSetTextMode( );
#endif

/* Constants for CR69. */

#define CRT_ACTIVE	0x01
#define LCD_ACTIVE	0x02
#define TV_ACTIVE	0x04
#define CRT_ATTACHED	0x10
#define LCD_ATTACHED	0x20
#define TV_ATTACHED	0x40

/* Various defines which are used to pass flags between the Setup and 
 * Subsequent functions. 
 */

#define NO_MONO_FILL      0x00
#define NEED_MONO_FILL    0x01
#define MONO_TRANSPARENCY 0x02


/* This function was taken from accel/s3v.h. It adjusts the width
 * of transfers for mono images to works around some bugs.
 */

extern S3VPRIV s3vPriv;

static __inline__ int S3SAVCheckLSPN(int w, int dir)
{
   int lspn = (w * s3vPriv.Bpp) & 63;  /* scanline width in bytes modulo 64*/

   if (s3vPriv.Bpp == 1) {
      if (lspn <= 8*1)
	 w += 16;
      else if (lspn <= 16*1)
	 w += 8;
   } else if (s3vPriv.Bpp == 2) {
      if (lspn <= 4*2)
	 w += 8;
      else if (lspn <= 8*2)
	 w += 4;
   } else {  /* s3vPriv.Bpp == 3 */
      if (lspn <= 3*3) 
	 w += 6;
      else if (lspn <= 6*3)
	 w += 3;
   }
   if (dir && w >= s3vPriv.bltbug_width1 && w <= s3vPriv.bltbug_width2) {
      w = s3vPriv.bltbug_width2 + 1;
   }

   return w;
}

/* And this adjusts color bitblts widths to work around GE bugs */

static __inline__ int S3SAVCheckBltWidth(int w)
{
   if (w >= s3vPriv.bltbug_width1 && w <= s3vPriv.bltbug_width2) {
      w = s3vPriv.bltbug_width2 + 1;
   }
   return w;
}

/* This next function determines if the Source operand is present in the
 * given ROP. The rule is that both the lower and upper nibble of the rop
 * have to be neither 0x00, 0x05, 0x0a or 0x0f. If a CPU-Screen blit is done
 * with a ROP which does not contain the source, the virge will hang when
 * data is written to the image transfer area. 
 */

static __inline__ Bool S3SAVROPHasSrc(shifted_rop)
int shifted_rop;
{
    int rop = (shifted_rop & (0xff << 17)) >> 17;

    if ((((rop & 0x0f) == 0x0a) | ((rop & 0x0f) == 0x0f) 
        | ((rop & 0x0f) == 0x05) | ((rop & 0x0f) == 0x00)) &
       (((rop & 0xf0) == 0xa0) | ((rop & 0xf0) == 0xf0) 
        | ((rop & 0xf0) == 0x50) | ((rop & 0xf0) == 0x00)))
            return FALSE;
    else 
            return TRUE;
}

/* This next function determines if the Destination operand is present in the
 * given ROP. The rule is that both the lower and upper nibble of the rop
 * have to be neither 0x00, 0x03, 0x0c or 0x0f. 
 */

static __inline__ Bool S3SAVROPHasDst(shifted_rop)
int shifted_rop;
{
    int rop = (shifted_rop & (0xff << 17)) >> 17;

    if ((((rop & 0x0f) == 0x0c) | ((rop & 0x0f) == 0x0f) 
        | ((rop & 0x0f) == 0x03) | ((rop & 0x0f) == 0x00)) &
       (((rop & 0xf0) == 0xc0) | ((rop & 0xf0) == 0xf0) 
        | ((rop & 0xf0) == 0x30) | ((rop & 0xf0) == 0x00)))
            return FALSE;
    else 
            return TRUE;
}
