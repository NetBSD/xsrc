/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3probe.c,v 1.1.2.6 1998/10/18 20:42:32 hohndel Exp $ */
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


static SymTabRec s3DacTable[] = {
   { NORMAL_DAC,	"normal" },
   { BT485_DAC,		"bt485" },
   { BT485_DAC,		"bt9485" },
   { ATT20C505_DAC,	"att20c505" },
   { TI3020_DAC,	"ti3020" },
   { ATT20C498_DAC,	"att20c498" },
   { ATT20C498_DAC,	"att21c498" },
   { ATT22C498_DAC,	"att22c498" },
   { TI3025_DAC,	"ti3025" },
   { TI3026_DAC,	"ti3026" },
   { TI3030_DAC,	"ti3030" },
   { IBMRGB525_DAC,	"ibm_rgb514" },
   { IBMRGB524_DAC,	"ibm_rgb524" },
   { IBMRGB525_DAC,	"ibm_rgb525" },
   { IBMRGB524_DAC,	"ibm_rgb526" },
   { IBMRGB528_DAC,	"ibm_rgb528" },
   { ATT20C490_DAC,	"att20c490" },
   { ATT20C490_DAC,	"att20c491" },
   { ATT20C490_DAC,	"ch8391" },
   { SC1148x_DAC,	"sc11482" },
   { SC1148x_DAC,	"sc11483" },
   { SC1148x_DAC,	"sc11484" },
   { SC1148x_DAC,	"sc11485" },
   { SC1148x_DAC,	"sc11487" },
   { SC1148x_DAC,	"sc11489" },
   { SC15025_DAC,	"sc15025" },
   { STG1700_DAC,	"stg1700" },
   { STG1703_DAC,	"stg1703" },
   { S3_SDAC_DAC,	"s3_sdac" },
   { S3_SDAC_DAC,	"ics5342" },      
   { S3_GENDAC_DAC,	"s3gendac" },
   { S3_GENDAC_DAC,	"ics5300" },
   { S3_TRIO32_DAC,	"s3_trio32" },
   { S3_TRIO64_DAC,	"s3_trio64" },
   { S3_TRIO64_DAC,	"s3_trio" },
   { ATT20C409_DAC,	"att20c409" },
   { SS2410_DAC,	"ss2410" },
   { S3_TRIO64V2_DAC,	"s3_trio64v2" },
   { S3_TRIO64V_DAC,	"s3_trio64v+" },
   { -1,		"" }
};

static unsigned S3_IOPorts[] = { DISP_STAT, H_TOTAL, H_DISP, H_SYNC_STRT,
  H_SYNC_WID, V_TOTAL, V_DISP, V_SYNC_STRT, V_SYNC_WID, DISP_CNTL,
  ADVFUNC_CNTL, SUBSYS_STAT, SUBSYS_CNTL, ROM_PAGE_SEL, CUR_Y, CUR_X,
  DESTY_AXSTP, DESTX_DIASTP, ERR_TERM, MAJ_AXIS_PCNT, GP_STAT, CMD,
  SHORT_STROKE, BKGD_COLOR, FRGD_COLOR, WRT_MASK, RD_MASK, COLOR_CMP,
  BKGD_MIX, FRGD_MIX, MULTIFUNC_CNTL, PIX_TRANS, PIX_TRANS_EXT,
};

#define Num_S3_IOPorts (sizeof(S3_IOPorts)/sizeof(S3_IOPorts[0]))
 

/*
 *  S3Probe --
 *
 *	 This is essentially what s3Probe was in the old
 *	S3 server but with some things removed or moved.  The
 *	SVGA server prints out some things for us (like videoram)
 *	so we don't duplicate the messages in here.  A major
 *	modification has been letting the SVGA server handle
 *	the modeline validation. Since modelines are not validated
 *	from within S3Probe as they were in the old server, the
 *	virtual size and display widths, etc... cannot be determined
 *	in this function. They are determined in S3FbInit. See
 *	the discussion in s3fbinit.c for more details.
 *
 *				MArk.
 *  
 */

   /*
    * These characterize a RAMDACs pixel multiplexing capabilities and
    * requirements:
    *
    *   pixMuxPossible         - pixmux is supported for the current RAMDAC
    *   allowPixMuxInterlace   - pixmux supports interlaced modes
    *   allowPixMuxSwitching   - possible to use pixmux for some modes
    *                            and non-pixmux for others
    *   pixMuxMinWidth         - smallest physical width supported in
    *                            pixmux mode
    *   nonMuxMaxClock         - highest dot clock supported without pixmux
    *   pixMuxMinClock         - lowest dot clock supported with pixmux
    *   nonMuxMaxMemory        - max video memory accessible without pixmux
    *   pixMuxLimitedWidths    - pixmux only works for logical display
    *                            widths 1024 and 2048
    *   pixMuxInterlaceOK      - FALSE if pixmux isn't possible because
    *                            there is an interlaced mode present
    *   pixMuxWidthOK          - FALSE if pixmux isn't possible because
    *                            there is mode with too small a width
    *   pixMuxClockOK	       - FALSE if pixmux isn't possible because
    *				 of a clock with too low of a value.
    */

Bool pixMuxPossible = FALSE;
Bool allowPixMuxInterlace = FALSE;
Bool allowPixMuxSwitching = FALSE;
Bool pixMuxInterlaceOK = TRUE; 
Bool pixMuxWidthOK = TRUE;
Bool pixMuxClockOK = TRUE;
int nonMuxMaxClock = 0;
int nonMuxMaxMemory = 8192;
int pixMuxMinWidth = 1024;
int pixMuxMinClock = 0;
Bool pixMuxLimitedWidths = TRUE;


static Bool VirtualNotGiven = FALSE;


Bool S3Probe()
{
   unsigned char config;
   int i;

   if (vga256InfoRec.chipset) {
	if (StrCaseCmp(vga256InfoRec.chipset,S3Ident(0)))
	   return(FALSE);
   } 


   /* check for fake S3 chips :-(( */
   if (vgaPCIInfo && (vgaPCIInfo->Vendor == PCI_VENDOR_SIGMADESIGNS ||
       vgaPCIInfo->Vendor == PCI_VENDOR_INTERGRAPHICS)) {
     char *vendor, chip[80];
     if (vgaPCIInfo->Vendor == PCI_VENDOR_SIGMADESIGNS) {
       vendor = "Sigma Designs";
       if (vgaPCIInfo->ChipType == PCI_CHIP_SD_REALMAGIG64GX)
	 strcpy(chip,"REALmagic64/GX (SD 6425) chip");
       else
	 sprintf(chip,"unknown chip (chip_id 0x%x)",vgaPCIInfo->ChipType);
     }
     else {
       vendor = "Intergraphics";
       if (vgaPCIInfo->Vendor == PCI_CHIP_INTERG_1680)
	 strcpy(chip,"IGA-1680 chip");
       else if (vgaPCIInfo->Vendor == PCI_CHIP_INTERG_1682)
	 strcpy(chip,"IGA-1682 chip");
       else
	 sprintf(chip,"unknown chip (chip_id 0x%x)",vgaPCIInfo->ChipType);
     }
     ErrorF("\n%s %s: WARNING: %s %s detected!\n"
	"\tNote: this chip is not a product of S3, Inc., and it is not\n"
	"\tcompatible with the XFree86 S3 drivers.  We understand that\n"
	"\tsome video cards are being sold with these chips relabeled\n"
	"\tas S3 Inc. chips, including S3's logo.  They are NOT S3 chips.\n"
	"\tPlease see http://www.s3.com\n\n",
	    XCONFIG_PROBED, vga256InfoRec.name);
   }


   S3EnterLeave(ENTER);  /* this will unlock the regs. */


	/***********************************************\
	|		Is it an S3? 			|
	\***********************************************/


   /* Lock */
   outb(vgaCRIndex, 0x38); outb(vgaCRReg, 0x00);

   /* Make sure we can't write when locked */
   if (testinx2(vgaCRIndex, 0x35, 0x0f)) {
      S3EnterLeave(LEAVE);
      return(FALSE);
   }
 
   /* Unlock */
   outb(vgaCRIndex, 0x38); outb(vgaCRReg, 0x48);

   /* Make sure we can write when unlocked */
   if (!testinx2(vgaCRIndex, 0x35, 0x0f)) {
      S3EnterLeave(LEAVE);
      return(FALSE);
   }

    /* OK, it's an S3. So find out which one */

   outb(vgaCRIndex, 0x30);
   s3ChipId = inb(vgaCRReg);         

   s3ChipRev = s3ChipId & 0x0f;
   if (s3ChipId >= 0xe0) {
      outb(vgaCRIndex, 0x2d);
      s3ChipId = inb(vgaCRReg) << 8;
      outb(vgaCRIndex, 0x2e);
      s3ChipId |= inb(vgaCRReg);
      outb(vgaCRIndex, 0x2f);
      s3ChipRev = inb(vgaCRReg);
   }

   if (vga256InfoRec.chipID) {
      ErrorF("%s %s: S3 chipset override, using chip_id = 0x%02x instead of 0x%02x\n",
	     XCONFIG_GIVEN, vga256InfoRec.name, vga256InfoRec.chipID, s3ChipId);
      s3ChipId = vga256InfoRec.chipID;
   }
   if (vga256InfoRec.chipRev) {
      ErrorF("%s %s: S3 chipset override, using chip_rev = %x instead of %x\n",
	     XCONFIG_GIVEN, vga256InfoRec.name, vga256InfoRec.chipRev, s3ChipRev);
      s3ChipRev = vga256InfoRec.chipRev;
   }

   /* We complain (and bail) when we have no idea what S3 chip it is */

   if (S3_ANY_ViRGE_SERIES(s3ChipId)) {
      ErrorF("%s %s: S3 ViRGE chipset: please load \"libs3v.a\" module\n", 
	     XCONFIG_PROBED, vga256InfoRec.name);
      S3EnterLeave(LEAVE);
      return(FALSE);
   }

   if (!S3_ANY_SERIES(s3ChipId)) {
      ErrorF("%s %s: Unknown S3 chipset: chip_id = 0x%02x rev. %x\n", 
	     XCONFIG_PROBED,vga256InfoRec.name,s3ChipId,s3ChipRev);
      S3EnterLeave(LEAVE);
      return(FALSE);
   }


   /* We print out the type even though we may not have support for
	it in this driver */

   outb(vgaCRIndex, 0x36);		
   config = inb(vgaCRReg);	

   if (xf86Verbose) {
      if (S3_x64_SERIES(s3ChipId)) {
         char *chipname = "unknown";
	 if (S3_868_SERIES(s3ChipId)) {
	    chipname = "868";
	 } else if (S3_866_SERIES(s3ChipId)) {
	    chipname = "866";
	 } else if (S3_864_SERIES(s3ChipId)) {
	    chipname = "864";
	 } else if (S3_968_SERIES(s3ChipId)) {
	    chipname = "968";
	 } else if (S3_964_SERIES(s3ChipId)) {
	    chipname = "964";
	 } else if (S3_ViRGE_SERIES(s3ChipId)) {
	    chipname = "ViRGE";
	 } else if (S3_TRIO32_SERIES(s3ChipId)) {
	    chipname = "Trio32";
	 } else if (S3_TRIO64V_SERIES(s3ChipId)) {
	    chipname = "Trio64V+";
	 } else if (S3_TRIO64UVP_SERIES(s3ChipId)) {
	    chipname = "Trio64UV+";
	 } else if (S3_AURORA64VP_SERIES(s3ChipId)) {
	    chipname = "Aurora64V+ (preliminary support; please report)";
	 } else if (S3_TRIO64V2_SERIES(s3ChipId)) {
	    outb(vgaCRIndex, 0x39);
	    outb(vgaCRReg, 0xa5);
	    outb(vgaCRIndex, 0x6f);
	    if (inb(vgaCRReg) & 1)
	       chipname = "Trio64V2/GX";
	    else
	       chipname = "Trio64V2/DX";
	 } else if (S3_TRIO64_SERIES(s3ChipId)) {
	    chipname = "Trio64";
	 } else if (S3_PLATO_PX_SERIES(s3ChipId)) {
	    chipname = "PLATO/PX (preliminary support; please report)";
	 }
	 ErrorF("%s %s: chipset:   %s rev. %x\n",
                XCONFIG_PROBED, vga256InfoRec.name, chipname, s3ChipRev);
      } else if (S3_801_928_SERIES(s3ChipId)) {
	 if (S3_801_SERIES(s3ChipId)) {
            if (S3_805_I_SERIES(s3ChipId)) {
               ErrorF("%s %s: chipset:   805i",
                      XCONFIG_PROBED, vga256InfoRec.name);
               if ((config & 0x03) == 3)
                  ErrorF(" (ISA)");
               else
                  ErrorF(" (VL)");
            }
	    else if (!((config & 0x03) == 3))
	       ErrorF("%s %s: chipset:   805",
                      XCONFIG_PROBED, vga256InfoRec.name);
	    else
	       ErrorF("%s %s: chipset:   801",
                       XCONFIG_PROBED, vga256InfoRec.name);
	    ErrorF(", ");
	    if (S3_801_REV_C(s3ChipId))
	       ErrorF("rev C or above\n");
	    else
	       ErrorF("rev A or B\n");
	 } else if (S3_928_SERIES(s3ChipId)) {
	    char *pci = S3_928_P(s3ChipId) ? "-P" : "";
	    if (S3_928_REV_E(s3ChipId))
		ErrorF("%s %s: chipset:   928%s, rev E or above\n",
                   XCONFIG_PROBED, vga256InfoRec.name, pci);
	    else
	        ErrorF("%s %s: chipset:   928%s, rev D or below\n",
                   XCONFIG_PROBED, vga256InfoRec.name, pci);
	 }
      } else if (S3_911_SERIES(s3ChipId)) {
	 if (S3_911_ONLY(s3ChipId)) {
	    ErrorF("%s %s: chipset:   911 \n",
                   XCONFIG_PROBED, vga256InfoRec.name);
	 } else if (S3_924_ONLY(s3ChipId)) {
	    ErrorF("%s %s: chipset:   924\n",
                   XCONFIG_PROBED, vga256InfoRec.name);
	 } else {
	    ErrorF("%s %s: S3 chipset type unknown, chip_id = 0x%02x\n",
		   XCONFIG_PROBED, vga256InfoRec.name, s3ChipId);
	 }
      }
   }

   /* Here's where we bail if the driver doesn't have support for
	the particular chipset */

   if(S3_ViRGE_SERIES(s3ChipId)) {
      ErrorF("The ViRGE chipset is unsupported by the s3_svga driver\n"
	  	"\tPlease use the s3v driver instead\n");
      S3EnterLeave(LEAVE);
      return(FALSE);
   }

   if((!S3_TRIO64V_SERIES(s3ChipId) && !S3_x68_SERIES(s3ChipId)) 
      && !S3_TRIO64V2_SERIES(s3ChipId) && !S3_AURORA64VP_SERIES(s3ChipId) ||
	OFLG_ISSET(OPTION_NO_MMIO, &vga256InfoRec.options)){
      ErrorF("%s %s: Using Port I/O\n",
		   XCONFIG_PROBED, vga256InfoRec.name);
   } else {
	s3newmmio = TRUE;
        ErrorF("%s %s: Using new style S3 MMIO\n",
		   XCONFIG_PROBED, vga256InfoRec.name);
   }

	/***************************************\
	| 	    Set Some Options  		|
	\***************************************/

#ifdef PC98
   if (OFLG_ISSET(OPTION_PW_MUX, &vga256InfoRec.options)) 
      OFLG_SET(OPTION_SPEA_MERCURY, &vga256InfoRec.options);
#endif

   if (OFLG_ISSET(OPTION_GENOA, &vga256InfoRec.options))
      s3BiosVendor = GENOA_BIOS;
   else if (OFLG_ISSET(OPTION_STB, &vga256InfoRec.options))
      s3BiosVendor = STB_BIOS;
   else if (OFLG_ISSET(OPTION_HERCULES, &vga256InfoRec.options))
      s3BiosVendor = HERCULES_BIOS;
   else if (OFLG_ISSET(OPTION_NUMBER_NINE, &vga256InfoRec.options))
      s3BiosVendor = NUMBER_NINE_BIOS;
   else if (OFLG_ISSET(OPTION_DIAMOND, &vga256InfoRec.options))
      s3BiosVendor = DIAMOND_BIOS;


   if (OFLG_ISSET(OPTION_POWER_SAVER, &vga256InfoRec.options))
      s3PowerSaver = TRUE;

if(s3newmmio)
   if (OFLG_ISSET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options)) {
	ErrorF("Linear addressing is required for the s3_newmmio driver\n"
	   "\tPlease remove \"no_linear\" option from the XF86Config file\n");
      S3EnterLeave(LEAVE);
      return(FALSE);
   }
else
   if (S3_911_SERIES(s3ChipId)) {
	OFLG_SET(OPTION_NOLINEAR_MODE, &vga256InfoRec.options);
   }


	/***************************************\
   	|	 LocalBus, EISA or PCI ?	|
	\***************************************/


   s3Localbus = ((config & 0x03) <= 2) || S3_928_P(s3ChipId);

   {
      char* bustype = "PCI";
 
      if (!S3_928_P(s3ChipId)) {
	 switch (config & 0x03) {
	 case 0:
	    bustype = "EISA\n";
	    break;
	 case 1:
            bustype = "386/486 localbus",
	    s3VLB = TRUE;
	    break;
	 case 3:
            bustype = "ISA";
	    break;
	 default:	/* anything else is PCI */
	    break;
	 }
      }
      if(xf86Verbose) 
       	    ErrorF("%s %s: bus type: %s\n", XCONFIG_PROBED,
				vga256InfoRec.name, bustype);
   }

if(s3newmmio) {
   if(OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options)) {
	if(config & 0x02) {
       	    ErrorF("%s %s: Using PCI retry\n", XCONFIG_GIVEN, 
					vga256InfoRec.name);
	    s3PCIRetry = TRUE;
	} else {
       	    ErrorF("%s %s: PCI retry only available on PCI bus\n"
		"\t\tOption disabled\n", XCONFIG_GIVEN, vga256InfoRec.name);
	    OFLG_CLR(OPTION_PCI_RETRY, &vga256InfoRec.options);
	}
   }
}
	
	/***************************************\
	|	Detect VideoRam amount 		|
	\***************************************/


   if (!vga256InfoRec.videoRam) {
      if (((config & 0x20) != 0) 	/* if bit 5 is a 1, then 512k RAM */
          && (!S3_964_SERIES(s3ChipId))) {
	 vga256InfoRec.videoRam = 512;
      } else {			/* must have more than 512k */
	 if (S3_911_SERIES(s3ChipId)) {
	    vga256InfoRec.videoRam = 1024;
	 } else {
	    if (S3_PLATO_PX_SERIES(s3ChipId))
	       vga256InfoRec.videoRam = (8-((config & 0xE0) >> 5)) * 512;
	    else switch ((config & 0xE0) >> 5) {	/* look at bits 6 and 7 */
	       case 0:
	         vga256InfoRec.videoRam = 4096;
		 break;
	       case 2:
	         vga256InfoRec.videoRam = 3072;
		 break;
	       case 3:
	         vga256InfoRec.videoRam = 8192;
		 break;
	       case 4:
		 vga256InfoRec.videoRam = 2048;
	         break;
	       case 5:
		 vga256InfoRec.videoRam = 6144;
	         break;
	       case 6:
	         vga256InfoRec.videoRam = 1024;
		 break;
	    }
	 }
      }
   } /* Gets printed out by svga server later */

   if (vga256InfoRec.videoRam > 1024)
      s3Mbanks = -1;
   else
      s3Mbanks = 0;

	/*****************************************************\
	| Determine card vendor to aid in clockchip detection |
	\*****************************************************/
	

/* stick that in here (MArk) */

    	/*******************************\
   	|	 Determine RAMDAC 	|
	\*******************************/

   /* Is one given in the XF86Config? */
   if (vga256InfoRec.ramdac) {
      s3RamdacType = xf86StringToToken(s3DacTable, vga256InfoRec.ramdac);
      if (s3RamdacType < 0) {
	 ErrorF("%s %s: Unknown RAMDAC type \"%s\"\n", XCONFIG_GIVEN,
		vga256InfoRec.name, vga256InfoRec.ramdac);
	 S3EnterLeave(LEAVE);
	 return(FALSE);
      }
   }

   /* if not given in XF86Config, probe for one */
   if(s3RamdacType == UNKNOWN_DAC) {
     for (i = 1; s3Ramdacs[i].DacName; i++) {
      if ((s3Ramdacs[i].DacProbe)()) {
	 s3RamdacType = i;
	 break;
       }
     }
   } else {  /* if given in XF86Config, verify it */
    if(!(s3Ramdacs[s3RamdacType].DacProbe)()) {
      ErrorF("WARNING: Did not detect a ramdac of type \"%s\" as specified!\n",
		s3Ramdacs[s3RamdacType].DacName);
	/* but we accept the user's assertion */
    }
   }

   /* If we still don't know the ramdac type, set it to NORMAL_DAC */
   if (s3RamdacType == UNKNOWN_DAC) {
      if (xf86Verbose) 
         ErrorF("%s %s: Unknown ramdac. Setting type to \"normal_dac\".\n",
              XCONFIG_PROBED, vga256InfoRec.name);
      s3RamdacType = NORMAL_DAC;
   }

   vga256InfoRec.ramdac = s3Ramdacs[s3RamdacType].DacName;

   if (xf86Verbose) {
      ErrorF("%s %s: Ramdac type: %s or compatible.\n",
	OFLG_ISSET(XCONFIG_RAMDAC, &vga256InfoRec.xconfigFlag) ?
   	XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name, 
					vga256InfoRec.ramdac);
   }


	/*******************************************************\
	| Set some SPEA specific options that had to wait until |
	| after we knew what the ramdac was.			|
	\*******************************************************/ 

/* do something different here MArk */

	/*******************************************************\
   	|	 Now set the DAC speed if not already set 	|
	\*******************************************************/

   if(vga256InfoRec.bitsPerPixel > 8) 
	s3Bpp = vga256InfoRec.bitsPerPixel >> 3;

   i = s3Bpp - 1;

   if (vga256InfoRec.dacSpeeds[i] <= 0) {
	vga256InfoRec.dacSpeeds[i] = s3Ramdacs[s3RamdacType].DacSpeeds[i];
   } else {
	if(vga256InfoRec.dacSpeeds[i] > s3Ramdacs[s3RamdacType].DacSpeeds[i])
	   ErrorF("WARNING!!! default dacSpeed limit for %i bpp has been "
 		"overridden\n\t %d MHz changed to %d MHz\n",s3Bpp << 3,
		s3Ramdacs[s3RamdacType].DacSpeeds[i] / 1000,
		vga256InfoRec.dacSpeeds[i] / 1000);
	s3Ramdacs[s3RamdacType].DacSpeeds[i] = vga256InfoRec.dacSpeeds[i];
   }
  
   if (xf86Verbose) {
      ErrorF("%s %s: Max RAMDAC speed for this depth: %d MHz\n",
	     OFLG_ISSET(XCONFIG_DACSPEED, &vga256InfoRec.xconfigFlag) ?
	     XCONFIG_GIVEN : XCONFIG_PROBED, vga256InfoRec.name,
	     vga256InfoRec.dacSpeeds[i] / 1000);
   }

   /*******************************************************************\
   | Check that the depth requested is supported by the ramdac/chipset |
   \*******************************************************************/

 
    if (S3_801_SERIES(s3ChipId)) {
      if (s3Bpp > 2) {
         ErrorF("Depths greater than 16bpp are not supported for 801/805 "
		"chips.\n");
         S3EnterLeave(LEAVE);
	 return(FALSE);
      }
    }
    else if (S3_911_SERIES(s3ChipId)) {
      if (s3Bpp > 1) {
         ErrorF("Depths greater than 8bpp are not supported for 911/924 "
		"chips.\n");
         S3EnterLeave(LEAVE);
	 return(FALSE);
      }
    }

    if (!S3_868_SERIES(s3ChipId) && !S3_968_SERIES(s3ChipId)) {
      if (s3Bpp == 3) {
         ErrorF("Packed-pixel 24bpp depths are only supported for 868/968 "
		"chips\n");
         S3EnterLeave(LEAVE);
	 return(FALSE);
      }	 
    }

    /* I guess these should return a status flag. Just 0 or 1 for now (MArk)*/
    if ((s3Ramdacs[s3RamdacType].PreInit)() <= 0) {
         S3EnterLeave(LEAVE);
	 return(FALSE);
    }

   if (xf86Verbose && pixMuxPossible)  /* Best we can do (MArk) */
	ErrorF("%s %s: Operating RAMDAC in pixel multiplex mode\n",
		   XCONFIG_PROBED, vga256InfoRec.name);

    
	/***************************************\
	|	Last minute Clock Checks 	|
	\***************************************/

   /* Check that maxClock is not higher than dacSpeed */
   if (vga256InfoRec.maxClock > vga256InfoRec.dacSpeeds[s3Bpp - 1])
      vga256InfoRec.maxClock = vga256InfoRec.dacSpeeds[s3Bpp - 1];

   /* Modify vga256InfoRec.maxClock if necessary */
   if(s3clockDoublingPossible) 
      s3maxRawClock *= 2;
   if ((s3maxRawClock > 0) && (vga256InfoRec.maxClock > s3maxRawClock))
      vga256InfoRec.maxClock = s3maxRawClock;

   /* check DCLK limit of 100MHz for 866/868 */
   if (S3_866_SERIES(s3ChipId) || S3_868_SERIES(s3ChipId)) {
      if (((s3Bpp==1 && !pixMuxPossible) || s3Bpp==2) 
	  && vga256InfoRec.maxClock > 100000)
	 vga256InfoRec.maxClock = 100000;
      else if (s3Bpp>2 && vga256InfoRec.maxClock > 50000)
	 vga256InfoRec.maxClock = 50000;  
   }
   /* check DCLK limit of 95MHz for 864 */
   else if (S3_864_SERIES(s3ChipId)) {
      if (((s3Bpp==1 && !pixMuxPossible) || s3Bpp==2) 
	  && vga256InfoRec.maxClock > 95000)
	 vga256InfoRec.maxClock = 95000;

      /* for 24bpp the limit should be 95/2 == 47.5MHz
	 but I set the limit to 50MHz to allow VESA 800x600@72Hz */
      else if (s3Bpp>2 && vga256InfoRec.maxClock > 50000)
	 vga256InfoRec.maxClock = 50000;  
   }

	/***********************************************\
	|	Set s3maxDisplayHeight and Width 	|
	\***********************************************/

   if (S3_911_SERIES(s3ChipId)) {
      s3maxDisplayWidth = 1024;
      s3maxDisplayHeight = 1024;
   } else {
      s3maxDisplayWidth = 2048;
      s3maxDisplayHeight = 4096;
   } 

   if((vga256InfoRec.virtualX <= 0) || (vga256InfoRec.virtualY <= 0))
	VirtualNotGiven = TRUE;
   else {   /* Virtual size was given */
     if (vga256InfoRec.virtualX > s3maxDisplayWidth) {
        ErrorF("%s: Virtual width (%d) is too large.  Maximum is %d\n",
	     vga256InfoRec.name, vga256InfoRec.virtualX, s3maxDisplayWidth);
       	S3EnterLeave(LEAVE);
        return (FALSE);
     }
     if (vga256InfoRec.virtualY > s3maxDisplayHeight) {
        ErrorF("%s: Virtual height (%d) is too large.  Maximum is %d\n",
	     vga256InfoRec.name, vga256InfoRec.virtualY, s3maxDisplayHeight);
       	S3EnterLeave(LEAVE);
        return (FALSE);
     }

     s3DisplayWidth = S3GetWidth(vga256InfoRec.virtualX);
     s3BppDisplayWidth = s3DisplayWidth * s3Bpp;
   }
 


		/***********************\
		|  	8 bit DAC	|
		\***********************/



	/* Is this correct?  (MArk) */
   if((DAC_IS_SC15025 || DAC_IS_ATT498 || DAC_IS_STG1700 || DAC_IS_ATT20C409 ||
	DAC_IS_ATT490 || DAC_IS_BT485_SERIES || DAC_IS_TI3020_SERIES || 
	DAC_IS_TI3026 || DAC_IS_TI3030 || DAC_IS_IBMRGB) && 
	!OFLG_ISSET(OPTION_DAC_6_BIT, &vga256InfoRec.options)) {
	s3DAC8Bit = TRUE;
   	OFLG_SET(OPTION_DAC_8_BIT, &vga256InfoRec.options);
    }
   

   if (OFLG_ISSET(OPTION_DAC_8_BIT, &vga256InfoRec.options) && !s3DAC8Bit) {
      OFLG_CLR(OPTION_DAC_8_BIT, &vga256InfoRec.options);
      ErrorF("%s %s: Option \"dac_8_bit\" not recognized for RAMDAC \"%s\"\n",
	     XCONFIG_PROBED, vga256InfoRec.name, vga256InfoRec.ramdac);
   }


	/*******************************\
	|	Sync-On-Green 		|
	\*******************************/


   if (DAC_IS_BT485_SERIES || DAC_IS_TI3020_SERIES || DAC_IS_TI3026 ||
        			 DAC_IS_TI3030 || DAC_IS_IBMRGB) {
     if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &vga256InfoRec.options)) {
	 s3DACSyncOnGreen = TRUE;
	 if (xf86Verbose)
	    ErrorF("%s %s: Putting RAMDAC into sync-on-green mode\n",
		   XCONFIG_GIVEN, vga256InfoRec.name);
      }
   }


#ifdef XFreeXDGA
   vga256InfoRec.directMode = XF86DGADirectPresent;
#endif

   vga256InfoRec.bankedMono = FALSE;
   vga256InfoRec.chipset = S3Ident(0);

	/*******************************\
 	|	Set the Valid Options 	|
	\*******************************/

/* if you see one that isn't used/needed anymore, please remove it (MArk) */

   OFLG_SET(OPTION_LEGEND, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_CLKDIV2, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NOACCEL, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NOLINEAR_MODE, &s3InfoRec.ChipOptionFlags);
   if (!S3_x64_SERIES(s3ChipId))
      OFLG_SET(OPTION_NO_MEM_ACCESS, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SW_CURSOR, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_BT485_CURS, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SHOWCACHE, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_FB_DEBUG, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NO_FONT_CACHE, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NO_PIXMAP_CACHE, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_TI3020_CURS, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_TI3026_CURS, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_IBMRGB_CURS, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_DAC_8_BIT, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_DAC_6_BIT, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SYNC_ON_GREEN, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SPEA_MERCURY, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NUMBER_NINE, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_STB_PEGASUS, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.ChipOptionFlags);
#ifdef PC98
   OFLG_SET(OPTION_PCSKB, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PCSKB4, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PCHKB, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NECWAB, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PW805I, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PWLB, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PW968, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_EPSON_MEM_WIN, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PW_MUX, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NOINIT, &s3InfoRec.ChipOptionFlags);
#endif
   /* ELSA_W1000PRO isn't really required any more */
   OFLG_SET(OPTION_ELSA_W1000PRO, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_ELSA_W2000PRO, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_DIAMOND, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_GENOA, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_STB, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_HERCULES, &s3InfoRec.ChipOptionFlags);
   if (S3_928_P(s3ChipId))
      OFLG_SET(OPTION_PCI_HACK, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_S3_964_BT485_VCLK, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SLOW_DRAM, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SLOW_EDODRAM, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SLOW_VRAM, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_SLOW_DRAM_REFRESH, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_FAST_VRAM, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_TRIO32_FC_BUG, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_S3_968_DASH_BUG, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_TRIO64VP_BUG1, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_TRIO64VP_BUG2, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_TRIO64VP_BUG3, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_PCI_RETRY, &s3InfoRec.ChipOptionFlags);
   OFLG_SET(OPTION_NO_MMIO, &s3InfoRec.ChipOptionFlags);

   return TRUE;
}








/*
 * S3ValidMode --
 *
 *	Validates modelines. Can be used by the VidMode extension (I hope).
 *	Determines virtual size if one wasn't given in the XF86Config.
 *	Also determines s3DisplayWidth & s3BppDisplayWidth.  
 *
 *			MArk.
 */

int S3ValidMode(DisplayModePtr pMode, Bool verbose, int flag)
{
    int ProposedVirtualX;
    int ProposedVirtualY;
    int ProposedDisplayWidth;
    int ProposedBppDisplayWidth;
    int ProposedMemUsage;

    ProposedVirtualX = pMode->HDisplay;
    ProposedVirtualY = pMode->VDisplay;
    ProposedDisplayWidth =  max(S3GetWidth(ProposedVirtualX),s3DisplayWidth);
    ProposedBppDisplayWidth = ProposedDisplayWidth * s3Bpp;
    ProposedMemUsage = ProposedBppDisplayWidth * ProposedVirtualY;


		/***********************\
		|  Trivial Size Tests 	|
		\***********************/

    if(ProposedVirtualX > s3maxDisplayWidth) {
	if(verbose)
	  ErrorF("%s %s: Width of mode \"%s\" is too large (max is %d)\n",
	  XCONFIG_PROBED, vga256InfoRec.name, pMode->name, s3maxDisplayWidth);
	return MODE_BAD;
    } else
   
    if(ProposedVirtualY > s3maxDisplayHeight) {
	if(verbose)
	  ErrorF("%s %s: Height of mode \"%s\" is too large (max is %d)\n",
	  XCONFIG_PROBED, vga256InfoRec.name, pMode->name, s3maxDisplayHeight);
	return MODE_BAD;
    } else

    if(ProposedMemUsage > (vga256InfoRec.videoRam * 1024)) {
	if(verbose)
	   ErrorF("%s %s: Too little video memory for mode \"%s\" "
		"(%d bytes required)\n", XCONFIG_PROBED, vga256InfoRec.name, 
		pMode->name, ProposedMemUsage);
	return MODE_BAD;
    } 


    /* sizes are within the capabilities of the hardware by here */
       /* we want it to return here on the first pass */

    if(VirtualNotGiven && !s3Initialized) {
     	/* Need a better test to keep the VidMode extesion out than 
					!s3Initialized  (Mark). */
	/* Well, we should add a special flag value for that (dhh) */
      if( flag == MODE_USED ) {
        vga256InfoRec.virtualX = max(ProposedVirtualX,vga256InfoRec.virtualX);
        vga256InfoRec.virtualY = max(ProposedVirtualY,vga256InfoRec.virtualY);
      }
      
    } else {     /* fixed size */
	if((ProposedVirtualX > vga256InfoRec.virtualX) ||
	   (ProposedVirtualY > vga256InfoRec.virtualY)) {
	  if(verbose)
	    ErrorF("%s %s: Resolution %dx%d too large for virtual %dx%d\n",
	      XCONFIG_PROBED, vga256InfoRec.name, ProposedVirtualX,
	      ProposedVirtualY, vga256InfoRec.virtualX, vga256InfoRec.virtualY);
	  return MODE_BAD;
	}

    }

    if( flag == MODE_USED ) {
      s3DisplayWidth = max(ProposedDisplayWidth, s3DisplayWidth);
      s3BppDisplayWidth = s3DisplayWidth * s3Bpp;
    }
    
    return MODE_OK;
}

