/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3driver.c,v 1.1.2.2 1998/02/15 16:09:37 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */
/*

   Written mostly by Mark Vojkovich (mvojkovi@ucsd.edu)
   With pieces stolen from XF86_S3

*/
	

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "s3reg.h"
#include "s3.h"
#include "mipointer.h"
#include "xf86cursor.h"

/* Not all of these used, I'll weed out the unecessary ones later (MArk) */

int   s3maxRawClock = 0;
int   s3BppDisplayWidth;
int   s3CursorBytes;
int   s3maxDisplayWidth;
int   s3maxDisplayHeight;
int   s3numClocks;
int   s3ScissB;
int   s3ScissR;
int   s3HDisplay;
int   s3DisplayWidth = 0;
unsigned short s3ChipRev;
unsigned short s3ChipId;
short s3BiosVendor = UNKNOWN_BIOS;
short s3RamdacType = UNKNOWN_DAC;
short s3Bpp = 1;
char  s3Mbanks;
char *s3ClockChipProbed = XCONFIG_GIVEN;
Bool  s3PCIRetry = FALSE;
Bool  s3Localbus = FALSE;
Bool  s3VLB = FALSE;
Bool  s3DAC8Bit = FALSE;
Bool  s3DACSyncOnGreen = FALSE;
Bool  s3PixelMultiplexing = FALSE;
Bool  s3Bt485PixMux = FALSE;
Bool  s3ATT498PixMux = FALSE;
Bool  s3PowerSaver = FALSE;
Bool  s3Initialized = FALSE;
Bool  s3clockDoublingPossible = FALSE;
Bool  s3newmmio = FALSE;
unsigned char 	s3Port31;
unsigned char   s3Port51;
unsigned char 	s3Port59 = 0x00;
unsigned char 	s3Port5A = 0x00;
unsigned char 	s3LinApOpt;	/* bottom of CR58 */
unsigned char   s3SAM256;	/* top of CR58 */
unsigned char 	s3SwapBits[256];
unsigned char 	s3DACBoarder = 0xff;
ScreenPtr 	s3savepScreen;
DisplayModePtr	s3CurrentMode = NULL;

static LUTENTRY s3SavedPalette[256];

pointer s3MmioMem = NULL;

Bool  (*s3ClockSelectFunc) ();

int vgaCRIndex;
int vgaCRReg;

vgaVideoChipRec s3InfoRec = {
  S3Probe,		/* Bool (* ChipProbe)() */
  S3Ident,		/* char * (* ChipIdent)() */
  S3EnterLeave,		/* void (* ChipEnterLeave)() */
  S3Init,		/* Bool (* ChipInit)() */
  S3ValidMode,  	/* int (* ChipValidMode)() */
  S3Save,		/* void * (* ChipSave)() */
  S3Restore,		/* void (* ChipRestore)() */
  S3Adjust,		/* void (* ChipAdjust)() */
  vgaHWSaveScreen,	/* void (* ChipSaveScreen)() */
  (void(*)())NoopDDA,  	/* void (* ChipGetMode)() */
  S3FbInit,		/* void (* ChipFbInit)() */
  S3SetRead,		/* void (* ChipSetRead)() */
  S3SetRead,		/* void (* ChipSetWrite)() */
  S3SetRead,		/* void (* ChipSetReadWrite)() */
  0x10000,		/* int ChipMapSize */
  0x10000,		/* int ChipSegmentSize */
  16,			/* int ChipSegmentShift */
  0xFFFF,		/* int ChipSegmentMask */
  0x00000, 0x10000,      /* int ChipReadBottom, int ChipReadTop */
  0x00000, 0x10000,	/* int ChipWriteBottom, int ChipWriteTop */
  FALSE,		/* Bool ChipUse2Banks */  
  VGA_DIVIDE_VERT,	/* int ChipInterlaceType */   
  {0,},			/* OFlagSet ChipOptionFlags */    
  8,			/* int ChipRounding */
  FALSE,		/* Bool ChipUseLinearAddressing */  			
  0,			/* int ChipLinearBase */
  0,			/* int ChipLinearSize */
  /*
   * This is TRUE if the driver has support for the given depth for 
   * the detected configuration. It must be set in the Probe function.
   * It most cases it should be FALSE.
   */
  TRUE,		/* 16bpp */
  TRUE,		/* 24bpp */
  TRUE,		/* 32bpp */
  NULL,			/* DisplayModePtr ChipBuiltinModes */
  1,			/* int ChipClockMulFactor */
  1			/* int ChipClockDivFactor */
};

short s3alu[16] =
{
   MIX_0,
   MIX_AND,
   MIX_SRC_AND_NOT_DST,
   MIX_SRC,
   MIX_NOT_SRC_AND_DST,
   MIX_DST,
   MIX_XOR,
   MIX_OR,
   MIX_NOR,
   MIX_XNOR,
   MIX_NOT_DST,
   MIX_SRC_OR_NOT_DST,
   MIX_NOT_SRC,
   MIX_NOT_SRC_OR_DST,
   MIX_NAND,
   MIX_1
};

/*
 *  s3Probe --
 *
 */
 
/* moved to s3probe.c */

/*
 *  s3Ident --
 *
 */

char * S3Ident(int n)
{
	static char *chipsets[] = {"s3_svga" };

	if (n + 1 > sizeof(chipsets) / sizeof(char *))
		return(NULL);
	else
		return(chipsets[n]);
}

void S3EnterLeave(Bool enter)
{
   unsigned char tmp;
   static Bool s3VTSwitchBack = FALSE;

#ifdef S3_DEBUG
   ErrorF("In S3EnterLeave(%s)\n", enter ? "ENTER" : "LEAVE");
#endif

#ifdef XFreeXDGA
   if ((vga256InfoRec.directMode & XF86DGADirectGraphics) && !enter) {
	S3SavePalette(s3SavedPalette);
	s3VTSwitchBack = TRUE;
    	if(XAACursorInfoRec.Flags & USE_HARDWARE_CURSOR) 
 		XAACursorInfoRec.HideCursor();
	return;
   }
#endif 

   if (enter) {
      xf86EnableIOPorts(vga256InfoRec.scrnIndex);

      vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
      vgaCRIndex = vgaIOBase + 4;
      vgaCRReg = vgaIOBase + 5;

      S3Unlock();

      /* Unprotect CRTC[0-7] */
      outb(vgaCRIndex, 0x11); 
      tmp = inb(vgaCRReg) & 0x7F;
      outb(vgaCRReg, tmp);

      /* needed for virtual console switchback since SVGA server
	doesn't reinit but restores */
      if(s3VTSwitchBack) {
         if(s3CurrentMode) {
		s3Initialized = FALSE;
		if(!S3Init(s3CurrentMode)) {
	  	/* what *should* we do here?  MArk */
	   	    FatalError("Whoops! Oversight in EnterLeave(ENTER)\n");
		} 
      	  }

	  S3RestorePalette(s3SavedPalette);
	  s3VTSwitchBack = FALSE;
      }

   } else { 

	if(s3Initialized) { 
	    S3SavePalette(s3SavedPalette);
	    s3VTSwitchBack = TRUE;
	    S3CleanUp(); 
	    s3Initialized = FALSE;
       	}	


	/* Protect CRTC[0-7] */
	outb(vgaIOBase + 4, 0x11); 
	tmp = (inb(vgaIOBase + 5) & 0x7F) | 0x80;
	outb(vgaIOBase + 5, tmp);

        /* I suppose I should probably lock the S3 here (MArk) */
	
	xf86DisableIOPorts(vga256InfoRec.scrnIndex);
   }

}


/*
 *   S3Init --
 *
 */

/* moved to s3init.c */

/*
 *  S3ValidMode --
 *
 */

/* moved to s3probe.c */


/*
 *   S3Save --
 *
 *   S3Restore --
 *
 */

/* moved to s3save.c */
 


void 	S3Adjust(int x, int y)
{
   int   Base, origBase;
   unsigned char tmp;
      
   if (OFLG_ISSET(OPTION_SHOWCACHE, &vga256InfoRec.options)) {
         if(y) y += 128;
   }

   if (x > s3DisplayWidth - s3HDisplay)
      x  = s3DisplayWidth - s3HDisplay;

   /* so may S3 cards have problems with some odd base addresses, 
    * to catch them all only even base values will be used.
    */

   origBase = (y * s3DisplayWidth + x) * s3Bpp;
   Base = (origBase >> 2) & ~1;

   if (S3_964_SERIES(s3ChipId)) {
      switch(s3RamdacType) {
	case BT485_DAC:
	case ATT20C505_DAC:
      	    if ((Base & 0x3f) >= 0x3c) 
	 	Base = (Base & ~0x3f) | 0x3b;
            else if (s3Bpp>1 && (Base & 0x3f) == 0x3a) 
	 	Base = (Base & ~0x3f) | 0x39;
      	    else if (s3Bpp>2 && (Base & 0x1f) == 0x1a) 
	 	Base = (Base & ~0x1f) | 0x19;
            break;
       	case TI3025_DAC:
	case TI3026_DAC:
	case TI3030_DAC:
	case IBMRGB524_DAC:
  	case IBMRGB525_DAC:
	case IBMRGB528_DAC:
	  {
      	    int px, py, a;
      	    miPointerPosition(&px, &py);
      	    if (s3Bpp == 3) {
	 	if (DAC_IS_TI3030 || DAC_IS_IBMRGB528)
	   	    a = 12;
	 	else 
	    	    a = 6;
	 	if (px-x > s3HDisplay/2)
	    	    Base = ((origBase + (a-1)*4) >> 2) & ~1;
	 	Base -= Base % a;
      	    } else {
	 	if (s3Bpp==1 && !DAC_IS_TI3030 && !DAC_IS_IBMRGB528)
	    	    a = 4-1;
	 	else 
	    	    a = 8-1;
	 	if (px-x > s3HDisplay/2)
	    	    Base = ((origBase + a*4) >> 2) & ~1;
	 	Base &= ~a;
      	    }
	  }
	default:
	     break;
      }
   }
   outb(vgaCRIndex, 0x31);
   outb(vgaCRReg, ((Base & 0x030000) >> 12) | s3Port31);
   s3Port51 &= ~0x03;
   s3Port51 |= ((Base & 0x0c0000) >> 18);
   outb(vgaCRIndex, 0x51);
   /* Don't override current bank selection */
   tmp = (inb(vgaCRReg) & ~0x03) | (s3Port51 & 0x03);
   outb(vgaCRReg, tmp);

   outw(vgaCRIndex, (Base & 0x00FF00) | 0x0C);
   outw(vgaCRIndex, ((Base & 0x00FF) << 8) | 0x0D);

#ifdef XFreeXDGA
   if (vga256InfoRec.directMode & XF86DGADirectGraphics) {
      /* Wait until vertical retrace is in progress. */
      S3RetraceWait();
   }
#endif

}

/*
 * S3FbInit --
 *
 */

/* moved to s3fbinit.c */

/*
 *  S3SetRead --
 *
 */

/* moved to s3bank.s */


