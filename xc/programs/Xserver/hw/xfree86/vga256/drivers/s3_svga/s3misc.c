/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3misc.c,v 1.1.2.3 1999/12/11 15:31:01 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
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


extern int nonMuxMaxClock;
extern int nonMuxMaxMemory;
extern int pixMuxMinClock;
extern int pixMuxMinWidth;
extern Bool allowPixMuxInterlace;
extern Bool pixMuxLimitedWidths;
extern Bool pixMuxPossible;



/*
 *  S3PixMuxShift --
 *
 *    	Determine pixmux shift.
 * 	Clocks need to be setup by here and V_DBLCLK needs to be
 *	determined beforehand (ie. call S3SetSynthClock() first).
 *	I think this is a nice example of something that shouldn't
 *	be stuck in S3Init like it was in the original S3 server.
 * 	Modularity. Modularity. Modularity
 *	
 *			MArk
 */	

static short S3PixMuxShift(DisplayModePtr mode)
{
   short pixMuxShift;

#ifdef S3_DEBUG 
    ErrorF("In S3PixMuxShift()\n");
#endif

   
   if (OFLG_ISSET(OPTION_ELSA_W2000PRO, &vga256InfoRec.options))
      pixMuxShift = (vga256InfoRec.clock[mode->Clock] > 120000) ? 2 : 
		      ((vga256InfoRec.clock[mode->Clock] > 60000) ? 1 : 0);
   else if (DAC_IS_IBMRGB528)
      pixMuxShift = (vga256InfoRec.clock[mode->Clock] > 220000 && s3Bpp <= 2) ? 
		2 : ((vga256InfoRec.clock[mode->Clock] > 110000) ? 1 : 0 );
   else if ((mode->Flags & V_DBLCLK) && (DAC_IS_TI3026) 
	    && (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &vga256InfoRec.clockOptions)))
      pixMuxShift =  (s3Bpp <= 2) ? 2 : 1;
   else if ((mode->Flags & V_DBLCLK) && DAC_IS_TI3030)
      pixMuxShift =  1;
   else if (S3_964_SERIES(s3ChipId) && DAC_IS_IBMRGB) {
      pixMuxShift = (mode->Flags & V_DBLCLK) ? 1 : 0;
      if (s3Bpp == 4)
	  pixMuxShift = 0; /* cf CR67 */
   } else if (S3_964_SERIES(s3ChipId) && DAC_IS_TI3025)
      pixMuxShift =  (mode->Flags & V_DBLCLK) ? 1 : 0;
   else if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES)
      pixMuxShift =  (mode->Flags & V_DBLCLK) ? 1 : 0;
   else if (S3_801_928_SERIES(s3ChipId) && DAC_IS_SC15025)
      pixMuxShift = -(s3Bpp>>1);  /* for 16/32 bpp */
   else if (S3_864_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId))
      pixMuxShift = -(s3Bpp>>1);  /* for 16/32 bpp */
   else if (S3_AURORA64VP_SERIES(s3ChipId))
      pixMuxShift = 0;
   else if (S3_TRIOxx_SERIES(s3ChipId))
      pixMuxShift = -(s3Bpp == 2);
   else if (S3_x64_SERIES(s3ChipId)) 
      pixMuxShift = 0;
   else if ((S3_928_SERIES(s3ChipId) && 
	(DAC_IS_TI3020 || DAC_IS_BT485_SERIES)) && s3PixelMultiplexing) {
      if (s3Bpp == 4)      pixMuxShift = 0;  /* 32 bit */
      else if (s3Bpp == 2) pixMuxShift = 1;  /* 16 bit */
      else                 pixMuxShift = 2;  /*  8 bit */
   } else if (s3PixelMultiplexing)  
      pixMuxShift = 2; 
   else 
      pixMuxShift = 0;

   return pixMuxShift;
}





/*
 * S3SetSynthClock --
 *
 * 	For programmable clocks, fill in the SynthClock value
 * 	and set V_DBLCLK as required for each mode
 *
 */

static void S3SetSynthClock(DisplayModePtr mode)
{
#ifdef S3_DEBUG 
    ErrorF("In S3SetSynthClock()\n");
#endif

   mode->SynthClock = vga256InfoRec.clock[mode->Clock];
	
   switch(s3RamdacType) {
	 case NORMAL_DAC:
	    /* only suports 8bpp -- nothing to do */
	    break;
	 case BT485_DAC:
	    {
	       int c;
	       if (OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options) ||
		   OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &vga256InfoRec.options))
		  c = 85000;
	       else if (S3_964_SERIES(s3ChipId) && s3Bpp == 4)
		  c = 90000;
	       else
		  c = 67500;
	       if (mode->SynthClock > c) {
		  mode->SynthClock /= 2;
		  mode->Flags |= V_DBLCLK;
	       }
	    }
	    break;
	 case ATT20C505_DAC:
	    if (mode->SynthClock > 90000) {
	       mode->SynthClock /= 2;
	       mode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3020_DAC:
	    if (mode->SynthClock > 100000) {
	       mode->SynthClock /= 2;
	       mode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3025_DAC:
	    if (mode->SynthClock > 80000) {
               /* the SynthClock will be divided and clock doubled by the PLL */
	       mode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3026_DAC:  /* IBMRGB??? */
	 case TI3030_DAC:
            if (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &vga256InfoRec.clockOptions))
	    {
               /*
                * for the mixed Ti3026/3030 + ICD2061A cases we need to split
                * at 120MHz; Since the ICD2061A clock code dislikes 120MHz
                * we already double for that
                */
	       if (mode->SynthClock >= 120000) {
	          mode->Flags |= V_DBLCLK;
	          mode->SynthClock /= 2;
	       }

	    } else {
	       /*
	        * use the Ti3026/3030 clock
	        */
	       if (mode->SynthClock > 80000) {
                  /* 
                   * the SynthClock will be divided and clock doubled 
                   * by the PLL 
                   */
	          mode->Flags |= V_DBLCLK;
	       }
	    }
	    break;
	 case IBMRGB524_DAC:
	 case IBMRGB525_DAC:
	 case IBMRGB528_DAC:
	    if (mode->SynthClock > 80000 || S3_968_SERIES(s3ChipId)) 
	       mode->Flags |= V_DBLCLK;
	    break;
	 case ATT20C498_DAC:
	 case ATT22C498_DAC:
	 case ATT20C409_DAC:
	 case STG1700_DAC:
	 case STG1703_DAC:
	 case S3_SDAC_DAC:
	    switch (s3Bpp) {
	    case 1:
	       /*
	        * This one depends on pixel multiplexing for 8bpp.
	        * Although existing code implies it depends on ramdac
	        * clock doubling instead (are the two tied together?).
		* CEG: Yes, if the AT&T dac is in pixmux mode, the clock
		* must be halved (and the DBLCLK flag set, go figure).
	        */
	       if (( DAC_IS_ATT20C498 && mode->SynthClock > nonMuxMaxClock)
		   || (!DAC_IS_ATT20C498 && mode->SynthClock > 67500)
		   || (mode->Flags & V_PIXMUX)) {
		  if (!(DAC_IS_SDAC)) {
		     mode->SynthClock /= 2;
		     mode->Flags |= V_DBLCLK;
		  }
	       }
	       break;
	    case 2:
	       /* No change for 16bpp */
	       break;
	    case 4:
	       mode->SynthClock *= 2;
	       break;
	    }
	    break;
	 case S3_TRIO32_DAC:
	 case S3_TRIO64_DAC:
	    switch (s3Bpp) {
	    case 1:
#if 0  

	       /* XXXX mode->SynthClock /= 2 might be better with sr15 &= ~0x40
		  in s3init.c if screen wouldn't completely blank... */
	       if (mode->SynthClock > nonMuxMaxClock) {
		  mode->SynthClock /= 2;
		  mode->Flags |= V_DBLCLK;
	       }
#endif
	       break;
	    case 2:
	    case 4:
	       /* No change for 16bpp and 24bpp */
	       break;
	    }
	    break;
	 case ATT20C490_DAC:
 	 case SS2410_DAC:    /* just guessing ( based on 490 ) */
	 case SC1148x_DAC:
	 case SC15025_DAC:
	 case S3_GENDAC_DAC:
	    if (s3Bpp > 1) 
	       mode->SynthClock *= s3Bpp;
	    break;
	 default:
	    /* Do nothing */
	    break;
   }

}



/* 
 *  S3SetupModePrivate --
 *
 *	Need to know if you are using V_DBLCLK first.
 */

static void  S3SetupModePrivate(DisplayModePtr mode)
{
#ifdef S3_DEBUG 
    ErrorF("In S3SetupModePrivate()\n");
#endif

/* couldn't we set this up in the modeline validation stage and add
	one more element to it - the flag that I talked about in
	S3Init() ? MArk */
       if(!mode->PrivSize || !mode->Private) {
       	   mode->PrivSize = S3_MODEPRIV_SIZE;
       	   mode->Private = (INT32 *)xcalloc(sizeof(INT32), S3_MODEPRIV_SIZE);
        }

	mode->Private[0] = 0;
	
	/***********************************************\
     	|	 Set default for S3_INVERT_VCLK 	|
	\***********************************************/

	
   	if (!(mode->Private[0] & (1 << S3_INVERT_VCLK))) {
	    if (DAC_IS_TI3026 && (s3BiosVendor == DIAMOND_BIOS ||
				  OFLG_ISSET(OPTION_DIAMOND,
				  &vga256InfoRec.options)))
	       mode->Private[S3_INVERT_VCLK] = 1;
	    else if (DAC_IS_TI3030) 
	       if ((s3Bpp == 2 && (mode->Flags & V_DBLCLK)) || s3Bpp == 4)
		  mode->Private[S3_INVERT_VCLK] = 1;
	       else 
		  mode->Private[S3_INVERT_VCLK] = 0;
	    else if (DAC_IS_IBMRGB)
	       if (s3Bpp == 4) 
		  mode->Private[S3_INVERT_VCLK] = 0;
	       else if (s3BiosVendor == STB_BIOS && s3Bpp == 2 
			&& vga256InfoRec.clock[mode->Clock] > 125000 
			&& vga256InfoRec.clock[mode->Clock] < 175000)
		  mode->Private[S3_INVERT_VCLK] = 0;
	       else if ((s3BiosVendor == NUMBER_NINE_BIOS ||
			 s3BiosVendor == HERCULES_BIOS) &&
			S3_968_SERIES(s3ChipId))
		  mode->Private[S3_INVERT_VCLK] = 0;
	       else
		  mode->Private[S3_INVERT_VCLK] = 1;
	    else 
	       mode->Private[S3_INVERT_VCLK] = 0;
	    mode->Private[0] |= 1 << S3_INVERT_VCLK;
    	}

	/***********************************************\
    	|	 Set default for S3_BLANK_DELAY 	|
	\***********************************************/

    	if (!(mode->Private[0] & (1 << S3_BLANK_DELAY))) {
	    mode->Private[0] |= (1 << S3_BLANK_DELAY);
	    if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES) {
	       if ((mode->Flags & V_DBLCLK) || s3Bpp > 1)
		  mode->Private[S3_BLANK_DELAY] = 0x00;
	       else
		  mode->Private[S3_BLANK_DELAY] = 0x01;
	    } else if (DAC_IS_TI3025) {
	       if (s3Bpp == 1)
		  if (mode->Flags & V_DBLCLK)
		     mode->Private[S3_BLANK_DELAY] = 0x02;
		  else
		     mode->Private[S3_BLANK_DELAY] = 0x03;
	       else if (s3Bpp == 2)
		  if (mode->Flags & V_DBLCLK)
		     mode->Private[S3_BLANK_DELAY] = 0x00;
		  else
		     mode->Private[S3_BLANK_DELAY] = 0x01;
	       else /* (s3Bpp == 4) */
		  mode->Private[S3_BLANK_DELAY] = 0x00;
	    } else if (DAC_IS_TI3026) {
	       if (s3BiosVendor == DIAMOND_BIOS 
                   || OFLG_ISSET(OPTION_DIAMOND, &vga256InfoRec.options)) {
	          if (s3Bpp == 1) 
		     mode->Private[S3_BLANK_DELAY] = 0x72;
	          else if (s3Bpp == 2) 
		     mode->Private[S3_BLANK_DELAY] = 0x73;
	          else /*if (s3Bpp == 4)*/ 
		     mode->Private[S3_BLANK_DELAY] = 0x75;
	       } else {
	          if (s3Bpp == 1) 
		     mode->Private[S3_BLANK_DELAY] = 0x00;
	          else if (s3Bpp == 2) 
		     mode->Private[S3_BLANK_DELAY] = 0x01;
	          else /*if (s3Bpp == 4)*/ 
		     mode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	    } else if (DAC_IS_TI3030){
	       if (s3Bpp == 1 || (s3Bpp == 2 && !(mode->Flags & V_DBLCLK)))
		  mode->Private[S3_BLANK_DELAY] = 0x01;
	       else
		  mode->Private[S3_BLANK_DELAY] = 0x00;
	    } else if (DAC_IS_IBMRGB) {
	       if (s3BiosVendor == GENOA_BIOS) {
		  mode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	       else if (s3BiosVendor == STB_BIOS) {
		  if (s3Bpp == 1 && vga256InfoRec.clock[mode->Clock] > 50000)
		     mode->Private[S3_BLANK_DELAY] = 0x55;
		  else
		     mode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	       else if (s3BiosVendor == HERCULES_BIOS) {
		 if (S3_968_SERIES(s3ChipId)) {
		   mode->Private[S3_BLANK_DELAY] = 0x00;
		 }
		 else {
		   mode->Private[S3_BLANK_DELAY] = (4/s3Bpp) - 1;
		   if (mode->Flags & V_DBLCLK) 
		     mode->Private[S3_BLANK_DELAY] >>= 1; 
		 }
	       }
	       else
		  mode->Private[S3_BLANK_DELAY] = 0x00;
	    } else {
	       mode->Private[S3_BLANK_DELAY] = 0x00;
	    }
    	}

	/***************************************\
    	| 	Set default for S3_EARLY_SC 	|
	\***************************************/
      
    	if (!(mode->Private[0] & (1 << S3_EARLY_SC))) {
	    mode->Private[0] |= 1 << S3_EARLY_SC;
	    if (DAC_IS_TI3025) {
	       if (OFLG_ISSET(OPTION_NUMBER_NINE,&vga256InfoRec.options))
		  mode->Private[S3_EARLY_SC] = 1;
	       else
		  mode->Private[S3_EARLY_SC] = 0;
	    } else if ((DAC_IS_TI3026 || DAC_IS_TI3030)
		       && OFLG_ISSET(CLOCK_OPTION_ICD2061A,
				     &vga256InfoRec.clockOptions)) {
	       if (s3Bpp == 2 && (mode->Flags & V_DBLCLK))
		  mode->Private[S3_EARLY_SC] = 1;
	       else
		  mode->Private[S3_EARLY_SC] = 0;
	    } else if (DAC_IS_TI3026 
		       && OFLG_ISSET(CLOCK_OPTION_ICD2061A,
				     &vga256InfoRec.clockOptions)) {
	       if (s3Bpp == 2 && (mode->Flags & V_DBLCLK))
		  mode->Private[S3_EARLY_SC] = 1;
	       else
		  mode->Private[S3_EARLY_SC] = 0;
	    } else if (DAC_IS_IBMRGB) {
	       if (s3BiosVendor == GENOA_BIOS) {
	          mode->Private[S3_EARLY_SC] = 0;
	       }
	       else if (s3BiosVendor == STB_BIOS) {
		  if (s3Bpp == 2 && vga256InfoRec.clock[mode->Clock] > 125000)
		     mode->Private[S3_EARLY_SC] = 0;
		  else if (s3Bpp == 4)
		     mode->Private[S3_EARLY_SC] = 0;
		  else 
		     mode->Private[S3_EARLY_SC] = 1;
	       }
	       else if (s3BiosVendor == HERCULES_BIOS) {
		 if (S3_968_SERIES(s3ChipId))
		   mode->Private[S3_EARLY_SC] = 0;
		 else
		   mode->Private[S3_EARLY_SC] = 0;
	       }
	       else
	          mode->Private[S3_EARLY_SC] = 0;
	    } else {
	       mode->Private[S3_EARLY_SC] = 0;
	    }
 	}
}

/*
 *  S3MuxOrNot --
 *
 */

unsigned char S3MuxOrNot(DisplayModePtr mode)
{
    	unsigned char verdict = 0x00;

#ifdef S3_DEBUG 
    ErrorF("In S3MuxOrNot()\n");
#endif

      /* Find out if the mode requires pixmux */

 	if ((s3Bpp == 1) && s3ATT498PixMux && !DAC_IS_SDAC &&
		(S3_864_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId))
		&& !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE,
			       &vga256InfoRec.clockOptions)) {
	       if (mode->Clock > 15) 
		  verdict |= MUSTMUX;
	}

	if (vga256InfoRec.clock[mode->Clock] > nonMuxMaxClock) {
		  verdict |= MUSTMUX;
	}

	if (vga256InfoRec.videoRam > nonMuxMaxMemory) 
		  verdict |= MUSTMUX;

        /* Really? virtual size is not exactly memory usage (MArk) */
	if ((OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options) ||
	     OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &vga256InfoRec.options)) &&
		vga256InfoRec.virtualX * vga256InfoRec.virtualY > 2*1024*1024) {
		  verdict |= MUSTMUX;
	}


    /* Find out if the mode can't be used with pixmux */

	if (mode->HDisplay < pixMuxMinWidth)
		  verdict |= CANTMUX;

	if ((mode->Flags & V_INTERLACE) && !allowPixMuxInterlace)
		  verdict |= CANTMUX;

	if (vga256InfoRec.clock[mode->Clock] < pixMuxMinClock)
		  verdict |= CANTMUX;

        if((verdict == (MUSTMUX | CANTMUX)))
	  ErrorF("%s: %s: Error! Conflicting multiplexing requirements for "
		"mode \"%s\"\n", vga256InfoRec.name, vga256InfoRec.chipset,
		mode->name); 
    
     	return verdict;
}




/*
 *  S3GetWidth --
 *
 */

int S3GetWidth(int width)
{
    if (pixMuxPossible && pixMuxLimitedWidths) {
       /* NOTE: this happens if pixmux is merely possible. 
		A casualty of my method (MArk). */
      if (width <= 1024) 
	 return 1024;
    } else if (S3_911_SERIES(s3ChipId)) {
      return 1024;
    } else {
      if (width <= 640) 
	 return 640;
      else if (width <= 800)
	 return 800;
      else if (width <= 1024)
	 return 1024;
      else if ((width <= 1152) &&
		 (   S3_801_REV_C(s3ChipId) 
                  || S3_805_I_SERIES(s3ChipId)
		  || S3_928_REV_E(s3ChipId)
		  || S3_x64_SERIES(s3ChipId)))
	 return 1152;
      else if (width <= 1280)
	 return 1280;
      else if ((width <= 1600) && 
		((S3_928_REV_E(s3ChipId) &&
		!(OFLG_ISSET(OPTION_NUMBER_NINE, &vga256InfoRec.options) &&
		     (s3Bpp == 1))) || S3_x64_SERIES(s3ChipId)))
	 return 1600;
    }

    return 2048;
}


/*
 *   S3RetraceWait --
 *
 */

void S3RetraceWait(void)
{
    outb(vgaCRIndex, 0x17);  /* is that better ? MArk */
    if(inb(vgaCRReg) & 0x80) {
    	while (inb(vgaIOBase + 0x0A) & 0x08);
   	while (!(inb(vgaIOBase + 0x0A) & 0x08));
    }
}


/*
 *  S3Unlock --
 *
 */

void S3Unlock(void)
{
   outb(vgaCRIndex, 0x38); outb(vgaCRReg, 0x48);
   outb(vgaCRIndex, 0x39); outb(vgaCRReg, 0xa5);
}

/*
 *  S3Lock --
 *
 *	I don't actually use this one (MArk)	
 */

void S3Lock(void)
{
   outb(vgaCRIndex, 0x38); outb(vgaCRReg, 0x00);
   outb(vgaCRIndex, 0x39); outb(vgaCRReg, 0x5A);
}


/*
 *  S3BankZero --
 *
 */

void S3BankZero()
{
   unsigned char tmp;

   outb(vgaCRIndex, 0x35);		
   tmp = inb(vgaCRReg) & 0xf0;
   outb(vgaCRReg, tmp);
   /* but wait, there's more! */
   if(S3_801_928_SERIES(s3ChipId)) {
	outb(vgaCRIndex, 0x51);
	tmp = inb(vgaCRReg) & 0xf3;
	outb(vgaCRReg, tmp);
   }
}



/*
 *  S3FillInModeInfo --
 *
 */


void S3FillInModeInfo(DisplayModePtr mode)
{
    short pixMuxShift = 0;
    Bool changed=FALSE;
    int oldCrtcHSyncStart, oldCrtcHSyncEnd, oldCrtcHTotal;

#ifdef S3_DEBUG 
    ErrorF("In S3FillInModeInfo()\n");
#endif

    /* Do we need this if we aren't using a programmable clock? (MArk) */
    S3SetSynthClock(mode);

    if (S3_964_SERIES(s3ChipId)) 
	S3SetupModePrivate(mode);

    pixMuxShift = S3PixMuxShift(mode);

    if (!mode->CrtcHAdjusted) {
      if (s3Bpp == 3 && S3_968_SERIES(s3ChipId) && 
			(DAC_IS_TI3026 || DAC_IS_TI3030)) {
	 mode->CrtcHTotal     = (mode->CrtcHTotal     * 3) / 4;
	 mode->CrtcHDisplay   = (mode->CrtcHDisplay   * 3) / 4;
	 mode->CrtcHSyncStart = (mode->CrtcHSyncStart * 3) / 4;
	 mode->CrtcHSyncEnd   = (mode->CrtcHSyncEnd   * 3) / 4;
	 mode->CrtcHSkew      = (mode->CrtcHSkew      * 3) / 4;
      }
      if (pixMuxShift > 0) {
	 /* now divide the horizontal timing parameters as required */
	 mode->CrtcHTotal     >>= pixMuxShift;
	 mode->CrtcHDisplay   >>= pixMuxShift;
	 mode->CrtcHSyncStart >>= pixMuxShift;
	 mode->CrtcHSyncEnd   >>= pixMuxShift;
	 mode->CrtcHSkew      >>= pixMuxShift;
      }
      else if (pixMuxShift < 0) {
	 /* now multiply the horizontal timing parameters as required */
	 mode->CrtcHTotal     <<= -pixMuxShift;
	 mode->CrtcHDisplay   <<= -pixMuxShift;
	 mode->CrtcHSyncStart <<= -pixMuxShift;
	 mode->CrtcHSyncEnd   <<= -pixMuxShift;
	 mode->CrtcHSkew      <<= -pixMuxShift;
      }
      mode->CrtcHAdjusted = TRUE;
    }

    /*************************************************************\
    | 	Do some sanity checks on the horizontal timing parameters |
    \*************************************************************/

      oldCrtcHSyncStart = mode->CrtcHSyncStart;
      oldCrtcHSyncEnd   = mode->CrtcHSyncEnd;
      oldCrtcHTotal     = mode->CrtcHTotal;
      if (mode->CrtcHTotal > 4096) {  /*  CrtcHTotal/8  is a 9 bit value */
	 mode->CrtcHTotal = 4096;
	 changed = TRUE;
      }
      if (mode->CrtcHSyncEnd >= mode->CrtcHTotal) {
	 mode->CrtcHSyncEnd = mode->CrtcHTotal - 1;
	          changed = TRUE;
      }
      if (mode->CrtcHSyncStart >= mode->CrtcHSyncEnd) {
	 mode->CrtcHSyncStart = mode->CrtcHSyncEnd - 1;
         changed = TRUE;
      }
      if ((DAC_IS_TI3030 || DAC_IS_IBMRGB528) && s3Bpp==1) {
	 /* for 128bit bus we need multiple of 16 8bpp pixels... */
	 if (mode->CrtcHTotal & 0x0f) {
	    mode->CrtcHTotal = (mode->CrtcHTotal + 0x0f) & ~0x0f;
	    changed = TRUE;
	 }
      }
      if (s3Bpp == 3) {
	 /* for packed 24bpp CrtcHTotal must be multiple of 3*8... */
	 if ((mode->CrtcHTotal >> 3) % 3 != 0) {
	    mode->CrtcHTotal >>= 3;
	    mode->CrtcHTotal += 3 - mode->CrtcHTotal % 3;
	    mode->CrtcHTotal <<= 3;
	    changed = TRUE;
	 }
      }
      if (changed) {
	 ErrorF("%s %s: mode line has to be modified ...\n",
		XCONFIG_PROBED, vga256InfoRec.name);
	 ErrorF("\t\tfrom   %4d %4d %4d %4d   %4d %4d %4d %4d\n"
		,mode->HDisplay, mode->HSyncStart, mode->HSyncEnd, mode->HTotal
		,mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal
		);
	 if(pixMuxShift < 0)
	    ErrorF("\t\tto     %4d %4d %4d %4d   %4d %4d %4d %4d\n",
		mode->CrtcHDisplay >> -pixMuxShift,
		mode->CrtcHSyncStart >> -pixMuxShift,
		mode->CrtcHSyncEnd  >> -pixMuxShift,
		mode->CrtcHTotal >> -pixMuxShift,
		mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal
		);
	  else
	    ErrorF("\t\tto     %4d %4d %4d %4d   %4d %4d %4d %4d\n",
		mode->CrtcHDisplay << pixMuxShift,
		mode->CrtcHSyncStart << pixMuxShift,
		mode->CrtcHSyncEnd  << pixMuxShift,
		mode->CrtcHTotal << pixMuxShift,
		mode->VDisplay, mode->VSyncStart, mode->VSyncEnd, mode->VTotal
		);
      }


}


void S3SavePalette(LUTENTRY* pal)
{
    register short i;

    outb(DAC_R_INDEX, 0);
    for (i=0; i < 256; i++) {
	 pal[i].r = inb(DAC_DATA);
	 pal[i].g = inb(DAC_DATA);
	 pal[i].b = inb(DAC_DATA);
    }
}


void S3RestorePalette(LUTENTRY* pal)
{
    register short i;

    outb(DAC_W_INDEX, 0);
    for (i=0; i < 256; i++) {
	 outb(DAC_DATA, pal[i].r);
	 outb(DAC_DATA, pal[i].g);
	 outb(DAC_DATA, pal[i].b);
    }
}
