/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3fbinit.c,v 1.1.2.1 1998/02/07 10:05:41 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */
#define USE_VGAHWINIT

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "site.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "vga.h"

#include "s3.h"
#include "s3reg.h"
#include "s3Bt485.h"
#define S3_SERVER
#include "Ti302X.h"
#include "IBMRGB.h"
#define XCONFIG_FLAGS_ONLY 
#include "xf86_Config.h"


extern int nonMuxMaxClock;
extern int pixMuxMinClock;


/*
 *   S3FbInit --
 *
 *	  This function is different in that some stuff seemingly more
 *	suitable for the Probe function is in here, but that is only 
 *	because it had to wait until after the modelines were validated
 *	or the width was determined etc...
 *
 *				MArk.
 *
 */


void S3FbInit(void)
{
   int i;

#ifdef S3_DEBUG
    ErrorF("In S3FbInit()\n");
#endif
    
      /**********************************************************************\
      | Adjust vga256InfoRec.clock[] when not using a programable clock chip |
      \**********************************************************************/
	/* This had to wait until after the clocks were setup (MArk). */

   if (!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &vga256InfoRec.clockOptions)) {
      Bool clocksChanged = FALSE;
      Bool numClocksChanged = FALSE;
      int newNumClocks = vga256InfoRec.clocks;


      /* New rules if we don't have a programmable clock ?*/
      if (S3_864_SERIES(s3ChipId))
	 nonMuxMaxClock = 95000;
      else if (S3_805_I_SERIES(s3ChipId))
	 nonMuxMaxClock = 90000;  

      /* Clock Modifications for some RAMDACs */
      for (i = 0; i < vga256InfoRec.clocks; i++) {
	 switch(s3RamdacType) {
	  case ATT20C498_DAC:
	  case ATT22C498_DAC:
	  case STG1700_DAC:
	  case STG1703_DAC:
	    switch (s3Bpp) {
	     case 1:
	        if (!numClocksChanged) {
		  newNumClocks = 32;
		  numClocksChanged = TRUE;
		  clocksChanged = TRUE;
		  for(i = vga256InfoRec.clocks; i < newNumClocks; i++)
		     vga256InfoRec.clock[i] = 0;  
		  if (vga256InfoRec.clocks > 16) 
		     vga256InfoRec.clocks = 16;
	        }
	        if (vga256InfoRec.clock[i] * 2 > pixMuxMinClock &&
		   vga256InfoRec.clock[i] * 2 <= vga256InfoRec.dacSpeeds[0])
		  vga256InfoRec.clock[i + 16] = vga256InfoRec.clock[i] * 2;
	        else
		  vga256InfoRec.clock[i + 16] = 0;
	        if (vga256InfoRec.clock[i] > nonMuxMaxClock)
		  vga256InfoRec.clock[i] = 0;
	        break;
	     case 4:
	        vga256InfoRec.clock[i] /= 2;
	        clocksChanged = TRUE;
	        break;
	    }
	    break;
	  case ATT20C490_DAC:
	  case SS2410_DAC:
	  case SC1148x_DAC:
	  case TI3020_DAC:
	  case SC15025_DAC:
	    if (s3Bpp > 1) {
	       vga256InfoRec.clock[i] /= s3Bpp;
	       clocksChanged = TRUE;
	    }
 	    break;
 	  default:
	    break;
	 }
      }
      if (numClocksChanged)
	 vga256InfoRec.clocks = newNumClocks;

      if (xf86Verbose && clocksChanged) {
	 ErrorF("%s %s: Effective pixel clocks available for depth %d:\n",
		XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.depth);
	 for (i = 0; i < vga256InfoRec.clocks; i++) {
	    if ((i % 8) == 0) {
	       if (i != 0)
		  ErrorF("\n");
               ErrorF("%s %s: pixel clocks:", XCONFIG_PROBED, 
						vga256InfoRec.name);
	    }
	    ErrorF(" %6.2f", (double)vga256InfoRec.clock[i] / 1000.0);
         }
         ErrorF("\n");
      }
   }

   /* At this point, the vga256InfoRec.clock[] values are pixel clocks */


  /*
    * Reduce the videoRam value if necessary to prevent Y coords exceeding
    * the 12-bit (4096) limit when small display widths are used on cards
    * with a lot of memory
    */
/* should this go somewhere else? (MArk) */

   if (vga256InfoRec.videoRam * 1024 / s3BppDisplayWidth > 4096) {
      vga256InfoRec.videoRam = s3BppDisplayWidth * 4096 / 1024;
      ErrorF("%s %s: videoram usage reduced to %dk to avoid co-ord overflow\n",
	     XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.videoRam);
   }
  

   if (S3_964_SERIES(s3ChipId) &&
	  !OFLG_ISSET(OPTION_NUMBER_NINE, &vga256InfoRec.options))
         s3SAM256 = 0x40;
   else if ((OFLG_ISSET(OPTION_SPEA_MERCURY, &vga256InfoRec.options) &&
               S3_928_ONLY(s3ChipId)) ||
	       OFLG_ISSET(OPTION_STB_PEGASUS, &vga256InfoRec.options))
         s3SAM256 = 0x80; /* set 6 MCLK cycles for R/W time on Mercury */
   else
         s3SAM256 = 0x00;


    if(xf86Verbose)
      ErrorF("%s %s: Framebuffer aligned on %i pixel width\n",
	 	XCONFIG_PROBED, vga256InfoRec.name, s3DisplayWidth);


	/* I should probably stick this somewhere else ? */
   s3ScissB = ((vga256InfoRec.videoRam * 1024) / s3BppDisplayWidth) - 1;
   s3ScissR = s3DisplayWidth - 1;



	/*******************************\
	|	Set Linear Base	 	|
	\*******************************/


      /* determine if we are linear addressing */
 
   if (s3newmmio || (S3_801_928_SERIES(s3ChipId) && s3Localbus 
	  && !OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)
	  && !OFLG_ISSET(OPTION_NO_MEM_ACCESS, &vga256InfoRec.options))) 
   {

         s3InfoRec.ChipUseLinearAddressing = TRUE;   

	  
	 if (vga256InfoRec.MemBase) 
	    s3InfoRec.ChipLinearBase = vga256InfoRec.MemBase;
         else if (vgaPCIInfo && (vgaPCIInfo->Vendor == PCI_S3_VENDOR_ID) &&
		(vgaPCIInfo->MemBase & 0xFF800000))
	    s3InfoRec.ChipLinearBase = vgaPCIInfo->MemBase & 0xFF800000;
	 else if (S3_x64_SERIES(s3ChipId)) 
	    s3InfoRec.ChipLinearBase = 0xf3000000; 
	 else 
	    s3InfoRec.ChipLinearBase = 0x03000000;
	 

    	s3Port59 = s3InfoRec.ChipLinearBase >> 24;
   	s3Port5A = s3InfoRec.ChipLinearBase >> 16;

       	s3InfoRec.ChipLinearSize =  vga256InfoRec.videoRam * 1024;

	/* Map the registers */
   if(s3newmmio) {
   	if(!(s3MmioMem = xf86MapVidMem(vga256InfoRec.scrnIndex, MMIO_REGION, 
      		(pointer)(s3InfoRec.ChipLinearBase + S3_NEWMMIO_REGBASE), 	
		S3_NEWMMIO_REGSIZE)))
      	FatalError("Unable to memory map registers!\n");  
   } 
			
	if (vga256InfoRec.videoRam <= 1024) 
	    s3LinApOpt = 0x15;
	else if (vga256InfoRec.videoRam <= 2048) 
	    s3LinApOpt = 0x16;
	else 
	    s3LinApOpt = 0x17;

        if(s3newmmio)
	    s3LinApOpt &= ~0x04;	
	     
   } 

   if(xf86Verbose) {
  	if(s3InfoRec.ChipUseLinearAddressing) 
	    ErrorF("%s %s: Linear mapped framebuffer at 0x%08lx\n",
		 XCONFIG_PROBED, vga256InfoRec.name, s3InfoRec.ChipLinearBase);
	else
	    ErrorF("%s %s: Bank switching... Bank size %i\n",
			 XCONFIG_PROBED, vga256InfoRec.name, 0x10000);
    }



	/*******************************\
      	|	Hardware Cursor		|
	\*******************************/

    if(!OFLG_ISSET(OPTION_SW_CURSOR, &vga256InfoRec.options)) 
	S3CursorInit();
    else {
      	ErrorF("%s %s: Hardware cursor disabled in XF86Config\n",
			XCONFIG_GIVEN, vga256InfoRec.name);
	s3CursorBytes = 0;
    }


	/*******************************\
	|	Setup XAA	 	|
	\*******************************/

    if(!OFLG_ISSET(OPTION_NOACCEL, &vga256InfoRec.options)) {
	if(s3newmmio)
	    S3AccelInit_NewMMIO();
	else
	    S3AccelInit();
    }

}

