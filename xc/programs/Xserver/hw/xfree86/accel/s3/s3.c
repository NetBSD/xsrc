/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3.c,v 3.147 1996/09/29 13:34:02 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Thomas Roell makes no
 * representations about the suitability of this software for any purpose. It
 * is provided "as is" without express or implied warranty.
 * 
 * THOMAS ROELL AND KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL THOMAS ROELL OR KEVIN E. MARTIN BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 * 
 * Rewritten for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)
 * 
 * Header: /home/src/xfree86/mit/server/ddx/xf86/accel/s3/RCS/s3.c,v 2.0
 * 1993/02/22 05:58:13 jon Exp
 * 
 * Modified by Amancio Hasty and Jon Tombs
 * 
 */
/* $XConsortium: s3.c /main/28 1996/01/31 10:04:33 kaleb $ */

#include "misc.h"
#include "cfb.h"
#include "pixmapstr.h"
#include "fontstruct.h"
#include "s3.h"
#include "regs3.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"
#include "s3linear.h"
#include "s3Bt485.h"
#include "Ti302X.h"
#include "IBMRGB.h"
#include "s3ELSA.h"
#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#ifdef PC98
#include "pc98_vers.h"
#include "s3pc98.h"
#endif

extern int s3MaxClock;
char s3Mbanks;
int s3Weight = RGB8_PSEUDO;
extern char *xf86VisualNames[];
char *clockchip_probed = XCONFIG_GIVEN;

extern s3VideoChipPtr s3Drivers[];

int vgaInterlaceType = VGA_DIVIDE_VERT;
void (*vgaSaveScreenFunc)() = vgaHWSaveScreen;

extern int defaultColorVisualClass;

static int s3ValidMode(
#if NeedFunctionPrototypes 
    DisplayModePtr, Bool  
#endif
);

ScrnInfoRec s3InfoRec =
{
   FALSE,			/* Bool configured */
   -1,				/* int tmpIndex */
   -1,				/* int scrnIndex */
   s3Probe,			/* Bool (* Probe)() */
   (Bool (*)())NoopDDA,		/* Bool (* Init)() */
   s3ValidMode,			/* int (* ValidMode)() */
   (void (*)())NoopDDA,		/* void (* EnterLeaveVT)() */
   (void (*)())NoopDDA,		/* void (* EnterLeaveMonitor)() */
   (void (*)())NoopDDA,		/* void (* EnterLeaveCursor)() */
   (void (*)())NoopDDA,		/* void (* AdjustFrame)() */
   (Bool (*)())NoopDDA,		/* Bool (* SwitchMode)() */
   s3PrintIdent,		/* void (* PrintIdent)() */
   8,				/* int depth */
   {5, 6, 5},			/* xrgb weight */
   8,				/* int bitsPerPixel */
   PseudoColor,			/* int defaultVisual */
   -1, -1,			/* int virtualX,virtualY */
   -1,				/* int displayWidth */
   -1, -1, -1, -1,		/* int frameX0, frameY0, frameX1, frameY1 */
   {0,},			/* OFlagSet options */
   {0,},			/* OFlagSet clockOptions */   
   {0, },              		/* OFlagSet xconfigFlag */
   NULL,			/* char *chipset */
   NULL,			/* char *ramdac */
   0,				/* int dacSpeed */
   0,				/* int clocks */
   {0,},			/* int clock[MAXCLOCKS] */
   0,				/* int maxClock */
   0,				/* int videoRam */
   0xC0000,                     /* int BIOSbase */  
   0,				/* unsigned long MemBase */
   240, 180,			/* int width, height */
   0,				/* unsigned long  speedup */
   NULL,			/* DisplayModePtr modes */   
   NULL,			/* MonPtr monitor */   
   NULL,			/* char           *clockprog */
   -1,			        /* int textclock */
   FALSE,			/* Bool           bankedMono */
   "S3",			/* char           *name */
   {0, },			/* xrgb blackColour */
   {0, },			/* xrgb whiteColour */
   s3ValidTokens,		/* int *validTokens */
   S3_PATCHLEVEL,		/* char *patchlevel */
   0,				/* int IObase */
   0,				/* int PALbase */
   0,				/* int COPbase */
   0,				/* int POSbase */
   0,				/* int instance */
   0,				/* int s3Madjust */
   0,				/* int s3Nadjust */
   0,				/* int s3MClk */
   0,				/* unsigned long VGAbase */
   0,				/* int s3RefClk */
   0,				/* int suspendTime */
   0,				/* int offTime */
   -1,				/* int s3BlankDelay */
   0,				/* int textClockFreq */
#ifdef XFreeXDGA
   0,				/* int directMode */
   s3SetVidPage,		/* Set Vid Page */
   0,				/* unsigned long physBase */
   0,				/* int physSize */
#endif
};

typedef struct S3PCIInformation {
   int DevID;
   int ChipType;
   int ChipRev;
   unsigned long MemBase;
} S3PCIInformation;

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

static unsigned S3_IOPorts[] = { DISP_STAT, H_TOTAL, H_DISP, H_SYNC_STRT,
  H_SYNC_WID, V_TOTAL, V_DISP, V_SYNC_STRT, V_SYNC_WID, DISP_CNTL,
  ADVFUNC_CNTL, SUBSYS_STAT, SUBSYS_CNTL, ROM_PAGE_SEL, CUR_Y, CUR_X,
  DESTY_AXSTP, DESTX_DIASTP, ERR_TERM, MAJ_AXIS_PCNT, GP_STAT, CMD,
  SHORT_STROKE, BKGD_COLOR, FRGD_COLOR, WRT_MASK, RD_MASK, COLOR_CMP,
  BKGD_MIX, FRGD_MIX, MULTIFUNC_CNTL, PIX_TRANS, PIX_TRANS_EXT,
};


static int Num_S3_IOPorts = (sizeof(S3_IOPorts)/sizeof(S3_IOPorts[0]));

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
   { SC1148x_M2_DAC,	"sc11482" },
   { SC1148x_M2_DAC,	"sc11483" },
   { SC1148x_M2_DAC,	"sc11484" },
   { SC1148x_M3_DAC,	"sc11485" },
   { SC1148x_M3_DAC,	"sc11487" },
   { SC1148x_M3_DAC,	"sc11489" },
   { SC15025_DAC,	"sc15025" },
   { STG1700_DAC,	"stg1700" },
   { STG1703_DAC,	"stg1703" },
   { S3_SDAC_DAC,	"s3_sdac" },
   { S3_SDAC_DAC,	"ics5342" },       /* XXXX should be checked if true */
   { S3_GENDAC_DAC,	"s3gendac" },
   { S3_GENDAC_DAC,	"ics5300" },
   { S3_TRIO32_DAC,	"s3_trio32" },
   { S3_TRIO64_DAC,	"s3_trio64" },
   { S3_TRIO64_DAC,	"s3_trio" },
   { ATT20C409_DAC,	"att20c409" },
   { SS2410_DAC,	"ss2410" },
   { -1,		"" },
};

static SymTabRec s3ChipTable[] = {
   { S3_UNKNOWN,	"unknown" },
   { S3_911,		"911" },
   { S3_924,		"924" },
   { S3_801,		"801" },
   { S3_805,		"805" },
   { S3_928,		"928" },
   { S3_TRIO_32_64,	"Trio32/64" },
   { S3_864,		"864" },
   { S3_868,		"868" },
   { S3_964,		"964" },
   { S3_968,		"968" },
   { S3_TRIO32,		"Trio32" },
   { S3_TRIO64,		"Trio64" },
   { S3_TRIO64VPLUS,	"Trio64V+" },
   { S3_ViRGE,		"ViRGE" },
   { S3_ViRGE_VX,	"ViRGE/VX" },
   { -1,		"" },
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
Bool  (*s3ClockSelectFunc) ();
static Bool LegendClockSelect();
static Bool s3ClockSelect();
static Bool icd2061ClockSelect();
static Bool s3GendacClockSelect();
static Bool ti3025ClockSelect();
static Bool ti3026ClockSelect();
static Bool IBMRGBClockSelect();
static void s3ProgramTi3025Clock(
#if NeedFunctionPrototypes
	int clk,
	unsigned char n,
	unsigned char m,
	unsigned char p
#endif
);
static Bool ch8391ClockSelect();
static Bool att409ClockSelect();
static Bool STG1703ClockSelect();
static Bool Gloria8ClockSelect();
ScreenPtr s3savepScreen;
Bool  s3Localbus = FALSE;
Bool  s3VLB = FALSE;
Bool  s3NewMmio = FALSE;
Bool  s3LinearAperture = FALSE;
Bool  s3Mmio928 = FALSE;
Bool  s3PixelMultiplexing = FALSE;
Bool  s3DAC8Bit = FALSE;
Bool  s3DACSyncOnGreen = FALSE;
Bool  s3PCIHack = FALSE;
Bool  s3PowerSaver = FALSE;
unsigned char s3LinApOpt;
unsigned char s3SAM256 = 0x00;
int s3BankSize;
int s3DisplayWidth;
pointer vgaBase = NULL;
pointer vgaBaseLow = NULL;
pointer vgaBaseHigh = NULL;
pointer s3VideoMem = NULL;
pointer s3MmioMem = NULL;
int s3Trio32FCBug = 0;
int s3_968_DashBug = 0;
unsigned long s3MemBase = 0;

extern Bool xf86Exiting, xf86Resetting, xf86ProbeFailed;
extern int  xf86Verbose;
int s3ScissR; 

int s3ScissB;
unsigned char s3SwapBits[256];
unsigned char s3Port40;
unsigned char s3Port51;
unsigned char s3Port54;
unsigned char s3Port59 = 0x00;
unsigned char s3Port5A = 0x00;
unsigned char s3Port31 = 0x8d;
void (*s3ImageReadFunc)(
#if NeedFunctionPrototypes
    int, int, int, int, char *, int, int, int, unsigned long
#endif
);
void (*s3ImageWriteFunc)(
#if NeedFunctionPrototypes
    int, int, int, int, char *, int, int, int, short, unsigned long
#endif
);
void (*s3ImageFillFunc)(
#if NeedFunctionPrototypes
    int, int, int, int, char *, int, int, int, int, int, short, unsigned long
#endif
);
int s3hotX, s3hotY;
Bool s3BlockCursor, s3ReloadCursor;
int s3CursorStartX, s3CursorStartY, s3CursorLines;
int s3RamdacType = UNKNOWN_DAC;
Bool s3UsingPixMux = FALSE;
Bool s3Bt485PixMux = FALSE;
Bool s3ATT498PixMux = FALSE;
static int maxRawClock = 0;
static Bool clockDoublingPossible = FALSE;
int s3AdjustCursorXPos = 0;
static int s3BiosVendor = UNKNOWN_BIOS;
static Bool in_s3Probe = TRUE;

#ifdef PC98
extern Bool	BoardInit();
extern int	pc98BoardType;
#endif

/*
 * s3PrintIdent -- print identification message
 */
void
s3PrintIdent()
{
  int i, j, n = 0, c = 0;
  char *id;

  ErrorF("  %s: accelerated server for S3 graphics adaptors (Patchlevel %s)\n",
	 s3InfoRec.name, s3InfoRec.patchLevel);
	 
  ErrorF("      ");
  for (i = 0; s3Drivers[i]; i++)
    for (j = 0; (id = (s3Drivers[i]->ChipIdent)(j)); j++, n++)
    {
      if (n)
      {
        ErrorF(",");
        c++;
        if (c + 1 + strlen(id) < 70)
        {
          ErrorF(" ");
          c++;
        }
        else
        {
          ErrorF("\n      ");
          c = 0;
        }
      }
      ErrorF("%s", id);
      c += strlen(id);
    }
  ErrorF("\n");
#ifdef PC98
  ErrorF("  PC98: Supported Video Boards:\n\t%s\n",PC98_S3_BOARDS);
#endif
}


static unsigned char *find_bios_string(int BIOSbase, char *match1, char *match2)
{
#define BIOS_BSIZE 1024
#define BIOS_BASE  0xc0000

   static unsigned char bios[BIOS_BSIZE];
   static int init=0;
   int i,j,l1,l2;

   if (!init) {
      init = 1;
      if (xf86ReadBIOS(BIOSbase, 0, bios, BIOS_BSIZE) != BIOS_BSIZE)
	 return NULL;
      if ((bios[0] != 0x55) || (bios[1] != 0xaa))
	 return NULL;
   }
   if (match1 == NULL)
      return NULL;

   l1 = strlen(match1);
   if (match2 != NULL) 
      l2 = strlen(match2);

   for (i=0; i<BIOS_BSIZE-l1; i++)
      if (bios[i] == match1[0] && !memcmp(&bios[i],match1,l1))
	 if (match2 == NULL) 
	    return &bios[i+l1];
	 else
	    for(j=i+l1; (j<BIOS_BSIZE-l2) && bios[j]; j++) 
	       if (bios[j] == match2[0] && !memcmp(&bios[j],match2,l2))
		  return &bios[j+l2];
   return NULL;
}




static int s3DetectMIRO_20SV_Rev(int BIOSbase)
{
   char *match1 = "miroCRYSTAL\37720SV", *match2 = "Rev.";
   unsigned char *p;

   if ((p = find_bios_string(BIOSbase,match1,match2)) != NULL) {
      if (s3BiosVendor == UNKNOWN_BIOS) 
	 s3BiosVendor = MIRO_BIOS;
      if (*p >= '0' && *p <= '9')
	 return *p - '0';
   }
   return -1;
}

static int check_SPEA_bios(int BIOSbase)
{
   char *match = " SPEA/Video";
   unsigned char *p;
   
   if ((p = find_bios_string(BIOSbase,match,NULL)) != NULL) {
      if (s3BiosVendor == UNKNOWN_BIOS) 
	 s3BiosVendor = SPEA_BIOS;
      return 1;
   }
   return 0;
}


static Bool
s3ProbeSDAC(Bool quiet)
{
   /* probe for S3 GENDAC or SDAC */
   /*
    * S3 GENDAC and SDAC have two fixed read only PLL clocks
    *     CLK0 f0: 25.255MHz   M-byte 0x28  N-byte 0x61
    *     CLK0 f1: 28.311MHz   M-byte 0x3d  N-byte 0x62
    * which can be used to detect GENDAC and SDAC since there is no chip-id
    * for the GENDAC.
    *
    * NOTE: for the GENDAC on a MIRO 10SD (805+GENDAC) reading PLL values
    * for CLK0 f0 and f1 always returns 0x7f (but is documented "read only")
    */
   
   unsigned char saveCR55, saveCR45, saveCR43, savelut[6];
   unsigned int i;		/* don't use signed int, UW2.0 compiler bug */
   long clock01, clock23;
   Bool found = FALSE;

   outb(vgaCRIndex, 0x43);
   saveCR43 = inb(vgaCRReg);
   outb(vgaCRReg, saveCR43 & ~0x02);

   outb(vgaCRIndex, 0x45);
   saveCR45 = inb(vgaCRReg);
   outb(vgaCRReg, saveCR45 & ~0x20);

   outb(vgaCRIndex, 0x55);
   saveCR55 = inb(vgaCRReg);
   outb(vgaCRReg, saveCR55 & ~1);

   outb(0x3c7,0);
   for(i=0; i<2*3; i++)		/* save first two LUT entries */
      savelut[i] = inb(0x3c9);
   outb(0x3c8,0);
   for(i=0; i<2*3; i++)		/* set first two LUT entries to zero */
      outb(0x3c9,0);

   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, saveCR55 | 1);

   outb(0x3c7,0);
   for(i=clock01=0; i<4; i++)
      clock01 = (clock01 << 8) | (inb(0x3c9) & 0xff);
   for(i=clock23=0; i<4; i++)
      clock23 = (clock23 << 8) | (inb(0x3c9) & 0xff);

   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, saveCR55 & ~1);

   outb(0x3c8,0);
   for(i=0; i<2*3; i++)		/* restore first two LUT entries */
      outb(0x3c9,savelut[i]);

   outb(vgaCRIndex, 0x55);
   outb(vgaCRReg, saveCR55);

   if ( clock01 == 0x28613d62 ||
       (clock01 == 0x7f7f7f7f && clock23 != 0x7f7f7f7f)) {
      found = TRUE;

      if (!quiet) {
	 xf86dactopel();
	 inb(0x3c6);
	 inb(0x3c6);
	 inb(0x3c6);

	 /* the fourth read will show the SDAC chip ID and revision */
	 if (((i=inb(0x3c6)) & 0xf0) == 0x70) {
	    ErrorF("%s %s: Detected an S3 SDAC 86C716 RAMDAC and programmable clock\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	    s3RamdacType = S3_SDAC_DAC;
	    saveCR43 &= ~0x02;
	    saveCR45 &= ~0x20;
	 }
	 else {
	    ErrorF("%s %s: Detected an S3 GENDAC 86C708 RAMDAC and programmable clock\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	    s3RamdacType = S3_GENDAC_DAC;
	    saveCR43 &= ~0x02;
	    saveCR45 &= ~0x20;
	 }
      }
      xf86dactopel();
   }

   outb(vgaCRIndex, 0x45);
   outb(vgaCRReg, saveCR45);

   outb(vgaCRIndex, 0x43);
   outb(vgaCRReg, saveCR43);

   return found;
}

static int
S3ProbeATT4xx(Bool quiet)
{
	/*
	 * the name might be misleading, this probes only for 409, 499, 498
	 */
	int dir, mir, olddaccomm;
	int s3RamdacType = UNKNOWN_DAC;

	xf86dactopel();
	xf86dactocomm();
	(void)inb(0x3C6);
	mir = inb(0x3C6);
	dir = inb(0x3C6);
	xf86dactopel();

	if ((mir == 0x84) && (dir == 0x98)) {
		olddaccomm = xf86getdaccomm();
		xf86setdaccomm(0x0a);
		if (xf86getdaccomm() == 0) {
			if( !quiet ) {
				ErrorF("%s %s: Detected an ATT 22C498 RAMDAC\n",
					XCONFIG_PROBED, s3InfoRec.name);
			}
			s3RamdacType = ATT22C498_DAC;
		}else{
			if( !quiet ) {
				ErrorF("%s %s: Detected an ATT 20C498/21C498 RAMDAC\n",
					XCONFIG_PROBED, s3InfoRec.name);
			}
			s3RamdacType = ATT498_DAC;
		}
		xf86setdaccomm(olddaccomm);
	} else if ((mir == 0x84) && (dir == 0x09)) {
		if( !quiet ) {
			ErrorF("%s %s: Detected an ATT 20C409 RAMDAC\n",
				XCONFIG_PROBED, s3InfoRec.name);
		}
		s3RamdacType = ATT20C409_DAC;
		if (!OFLG_ISSET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions)) {
			OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
			OFLG_SET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions);
			clockchip_probed = XCONFIG_PROBED;
		}
	} else if ((mir == 0x84) && (dir == 0x99)) {
		/*
		 * according to the 21C499 data sheet it is fully compatible
		 * with the 22C409. So we will only miss its new features
		 * this way, but in theory things might work.
		 */
		if( !quiet ) {
			ErrorF("%s %s: Detected an ATT 21C499 RAMDAC\n",
				XCONFIG_PROBED, s3InfoRec.name);
			ErrorF("%s %s:    support for this RAMDAC is untested. Please report to XFree86@XFree86.Org\n",
				XCONFIG_PROBED, s3InfoRec.name);
		}
		s3RamdacType = ATT20C409_DAC;
		if (!OFLG_ISSET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions)) {
			OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
			OFLG_SET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions);
			clockchip_probed = XCONFIG_PROBED;
		}
	}
	return(s3RamdacType);
}

/*
 * s3GetPCIInfo -- probe for PCI information
 */
S3PCIInformation *
s3GetPCIInfo()
{
   static S3PCIInformation info = {0, };
   pciConfigPtr pcrp, *pcrpp;
   Bool found = FALSE;
   int i = 0;

   pcrpp = xf86scanpci(s3InfoRec.scrnIndex);

   if (!pcrpp)
      return NULL;

   while ((pcrp = pcrpp[i])) {
      if (pcrp->_vendor == PCI_S3_VENDOR_ID) {
	 found = TRUE;
	 switch (pcrp->_device) {
	 case PCI_TRIO_32_64:
	    info.ChipType = S3_TRIO_32_64;
	    break;
	 case PCI_928:
	    info.ChipType = S3_928;
	    break;
	 case PCI_864_0:
	 case PCI_864_1:
	    info.ChipType = S3_864;
	    break;
	 case PCI_964_0:
	 case PCI_964_1:
	    info.ChipType = S3_964;
	    break;
	 case PCI_868:
	    info.ChipType = S3_868;
	    break;
	 case PCI_968:
	    info.ChipType = S3_968;
	    break;
	 case PCI_ViRGE:
	    info.ChipType = S3_ViRGE;
	    break;
	 case PCI_ViRGE_VX:
	    info.ChipType = S3_ViRGE_VX;
	    break;
	 default:
	    info.ChipType = S3_UNKNOWN;
	    info.DevID = pcrp->_device;
	    break;
	 }
	 info.ChipRev = pcrp->_rev_id;
	 info.MemBase = pcrp->_base0 & 0xFF800000;
#ifdef PC98_GA968
	 xf86writepci(s3InfoRec.scrnIndex, pcrp->_bus, pcrp->_cardnum,
			pcrp->_func,
			PCI_CMD_STAT_REG, PCI_CMD_MASK,
			PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE);
#endif
	 break;
      }
      i++;
   }

   /* for new mmio we have to ensure that the PCI base address is 
    * 64MB aligned and that there are no address collitions within 64MB.
    * S3 868/968 only pretend to need 32MB and thus fool 
    * the BIOS PCI auto configuration :-(  */
 
   if (   info.ChipType == S3_868 
       || info.ChipType == S3_968 
       || info.ChipType == S3_TRIO_32_64  /* only needed for Trio64V+ */
       /* || info.ChipType == S3_ViRGE */) {
      unsigned long base0;
      char *probed;
      char map_64m[64];
      int j;
      
      if (s3InfoRec.MemBase == 0) {
	 base0  = info.MemBase;
	 probed = XCONFIG_PROBED;
      }
      else {
	 base0  = s3InfoRec.MemBase;
	 probed = XCONFIG_GIVEN;
      }

      /* map allocated 64MB blocks */
      for (j=0; j<64; j++) map_64m[j] = 0;
      map_64m[63] = 1;  /* don't use the last 64MB area */
      for (j=0; (pcrp = pcrpp[j]); j++) {
	 if (i != j) {
	    map_64m[ (pcrp->_base0 >> 26) & 0x3f] = 1;
	    map_64m[((pcrp->_base0+0x3ffffff) >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base1 >> 26) & 0x3f] = 1;
	    map_64m[((pcrp->_base1+0x3ffffff) >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base2 >> 26) & 0x3f] = 1;
	    map_64m[((pcrp->_base2+0x3ffffff) >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base3 >> 26) & 0x3f] = 1;
	    map_64m[((pcrp->_base3+0x3ffffff) >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base4 >> 26) & 0x3f] = 1;
	    map_64m[((pcrp->_base4+0x3ffffff) >> 26) & 0x3f] = 1;
	    map_64m[ (pcrp->_base5 >> 26) & 0x3f] = 1;
	    map_64m[((pcrp->_base5+0x3ffffff) >> 26) & 0x3f] = 1;
	 }
      }

      /* check for 64MB alignment and free space */
      
      if ((base0 & 0x3ffffff) ||
	  map_64m[(base0 >> 26) & 0x3f] || 
	  map_64m[((base0+0x3ffffff) >> 26) & 0x3f]) {
	 for (j=63; j>=16 && map_64m[j]; j--);
	 info.MemBase = ((unsigned long)j) << 26;
	 ErrorF("%s %s: PCI: base address not correctly aligned or address conflict\n",
		probed, s3InfoRec.name);
	 ErrorF("\t\tbase address changed from 0x%08lx to 0x%08lx\n",
		base0, info.MemBase);
         xf86writepci(s3InfoRec.scrnIndex, pcrpp[i]->_bus, pcrpp[i]->_cardnum,
		    pcrpp[i]->_func, PCI_MAP_REG_START, ~0L,
		    info.MemBase | PCI_MAP_MEMORY | PCI_MAP_MEMORY_TYPE_32BIT);
      }
      s3Port59 = (info.MemBase >> 24) & 0xfc;
      s3Port5A = 0;
   }
   else {
      if (s3InfoRec.MemBase != 0) {
	 s3Port59 =  s3InfoRec.MemBase >> 24;
	 s3Port5A = (s3InfoRec.MemBase >> 16) & 0x08;
      }
      else {
	 s3Port59 =  info.MemBase >> 24;
	 s3Port5A = (info.MemBase >> 16) & 0x08;
      }
   }

   /* Free PCI information */
   xf86cleanpci();

   if (found && xf86Verbose) {
      if (info.ChipType != S3_UNKNOWN) {
	 ErrorF("%s %s: PCI: %s rev %x, Linear FB @ 0x%08lx\n", XCONFIG_PROBED,
		s3InfoRec.name, xf86TokenToString(s3ChipTable, info.ChipType),
		info.ChipRev, info.MemBase);
      } else {
	 ErrorF("%s %s: PCI: unknown (please report), ID 0x%04x rev %x,"
		" Linear FB @ 0x%08lx\n", XCONFIG_PROBED,
		s3InfoRec.name, info.DevID, info.ChipRev, info.MemBase);
      }
   }
   if (found)
      return &info;
   else
      return NULL;
}

/*
 * s3Probe -- probe and initialize the hardware driver
 */
/* moved out of s3Probe() so s3ValidMode() can see them (MArk)*/
static Bool pixMuxPossible = FALSE;
static Bool allowPixMuxInterlace = FALSE;
static Bool allowPixMuxSwitching = FALSE;
static int nonMuxMaxClock = 0;
static int nonMuxMaxMemory = 8192;
static int maxDisplayWidth;
static int maxDisplayHeight;
static int pixMuxMinWidth = 1024;
static int pixMuxMinClock = 0;

Bool
s3Probe()
{
   DisplayModePtr pMode, pEnd;
   unsigned char config, tmp;
   int i, j, numClocks;
   int tx, ty;
   OFlagSet validOptions;
   char *card, *serno;
   int card_id, max_pix_clock, max_mem_clock, hwconf;
   int lookupFlags;

   /*
    * These characterise a RAMDACs pixel multiplexing capabilities and
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
    *                            there is mode has too small a width
    */
   Bool pixMuxNeeded = FALSE;
   Bool pixMuxLimitedWidths = TRUE;
   Bool pixMuxInterlaceOK = TRUE;
   Bool pixMuxWidthOK = TRUE;
   S3PCIInformation *pciInfo = NULL;

#if !defined(PC98) || defined(PC98_GA968)
   /* Do general PCI probe first */
   pciInfo = s3GetPCIInfo();
   if (pciInfo && pciInfo->MemBase)
      s3MemBase = pciInfo->MemBase;
#endif

   xf86ClearIOPortList(s3InfoRec.scrnIndex);
   xf86AddIOPorts(s3InfoRec.scrnIndex, Num_VGA_IOPorts, VGA_IOPorts);
   xf86AddIOPorts(s3InfoRec.scrnIndex, Num_S3_IOPorts, S3_IOPorts);

   /* Enable I/O access */
   xf86EnableIOPorts(s3InfoRec.scrnIndex);

#ifdef PC98
#ifdef PC98_PW
   pc98BoardType = PW;
#endif
#ifdef PC98_NEC
   pc98BoardType = NECWAB;
#endif
#ifdef PC98_PWLB
   pc98BoardType = PWLB;
#endif
#ifdef PC98_GA968
   pc98BoardType = GA968;
#endif

   if (OFLG_ISSET(OPTION_PCSKB, &s3InfoRec.options))
	pc98BoardType = PCSKB;
   if (OFLG_ISSET(OPTION_PCSKB4, &s3InfoRec.options))
	pc98BoardType = PCSKB4;
   if (OFLG_ISSET(OPTION_PCHKB, &s3InfoRec.options))
	pc98BoardType = PCHKB;
   if (OFLG_ISSET(OPTION_NECWAB, &s3InfoRec.options))
	pc98BoardType = NECWAB;
   if (OFLG_ISSET(OPTION_PW805I, &s3InfoRec.options))
	pc98BoardType = PW805I;
   if (OFLG_ISSET(OPTION_PWLB, &s3InfoRec.options))
	pc98BoardType = PWLB;
   if (OFLG_ISSET(OPTION_PW968, &s3InfoRec.options))
	pc98BoardType = PW968;
   ErrorF("   PC98:Board Type = %X \n",pc98BoardType);
   if(BoardInit() == FALSE)
	return(FALSE);
#endif 

   vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
   vgaCRIndex = vgaIOBase + 4;
   vgaCRReg = vgaIOBase + 5;

   outb(vgaCRIndex, 0x11);	/* for register CR11, (Vertical Retrace End) */
   outb(vgaCRReg, 0x00);		/* set to 0 */

   outb(vgaCRIndex, 0x38);		/* check if we have an S3 */
   outb(vgaCRReg, 0x00);

   /* Make sure we can't write when locked */

   if (testinx2(vgaCRIndex, 0x35, 0x0f)) {
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return(FALSE);
   }
 
   outb(vgaCRIndex, 0x38);	/* for register CR38, (REG_LOCK1) */
   outb(vgaCRReg, 0x48);	/* unlock S3 register set for read/write */

   /* Make sure we can write when unlocked */

   if (!testinx2(vgaCRIndex, 0x35, 0x0f)) {
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return(FALSE);
   }

   outb(vgaCRIndex, 0x36);		/* for register CR36 (CONFG_REG1), */
   config = inb(vgaCRReg);		/* get amount of vram installed */

   outb(vgaCRIndex, 0x30);
   s3ChipId = inb(vgaCRReg);         /* get chip id */

   s3ChipRev = s3ChipId & 0x0f;
   if (s3ChipId >= 0xe0) {
      outb(vgaCRIndex, 0x2e);
      s3ChipId |= (inb(vgaCRReg) << 8);
      outb(vgaCRIndex, 0x2f);
      s3ChipRev |= (inb(vgaCRReg) << 4);      
   }

   if (!S3_ANY_SERIES(s3ChipId)) {
      ErrorF("%s %s: Unknown S3 chipset: chip_id = 0x%02x rev. %x\n", 
	     XCONFIG_PROBED,s3InfoRec.name,s3ChipId,s3ChipRev);
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return(FALSE);
   }

   for (i = 0; s3Drivers[i]; i++) {
      if ((s3Drivers[i]->ChipProbe)()) {
	 xf86ProbeFailed = FALSE;
	 s3InfoRec.Init = s3Drivers[i]->ChipInitialize;
	 s3InfoRec.EnterLeaveVT = s3Drivers[i]->ChipEnterLeaveVT;
	 s3InfoRec.AdjustFrame = s3Drivers[i]->ChipAdjustFrame;
	 s3InfoRec.SwitchMode = s3Drivers[i]->ChipSwitchMode;
	 break;
      }
   }
   if (xf86ProbeFailed) {
      if (s3InfoRec.chipset) {
	 ErrorF("%s: '%s' is an invalid chipset", s3InfoRec.name,
		s3InfoRec.chipset);
      }
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return(FALSE);
   }

   if (s3InfoRec.ramdac) {
      s3RamdacType = xf86StringToToken(s3DacTable, s3InfoRec.ramdac);
      if (s3RamdacType < 0) {
	 ErrorF("%s %s: Unknown RAMDAC type \"%s\"\n", XCONFIG_GIVEN,
		s3InfoRec.name, s3InfoRec.ramdac);
	 xf86DisableIOPorts(s3InfoRec.scrnIndex);
	 return(FALSE);
      }
   }

   OFLG_ZERO(&validOptions);
   OFLG_SET(OPTION_LEGEND, &validOptions);
   OFLG_SET(OPTION_CLKDIV2, &validOptions);
   OFLG_SET(OPTION_NOLINEAR_MODE, &validOptions);
   if (!S3_x64_SERIES(s3ChipId))
      OFLG_SET(OPTION_NO_MEM_ACCESS, &validOptions);
   OFLG_SET(OPTION_SW_CURSOR, &validOptions);
   OFLG_SET(OPTION_BT485_CURS, &validOptions);
   OFLG_SET(OPTION_SHOWCACHE, &validOptions);
   OFLG_SET(OPTION_FB_DEBUG, &validOptions);
   OFLG_SET(OPTION_NO_FONT_CACHE, &validOptions);
   OFLG_SET(OPTION_NO_PIXMAP_CACHE, &validOptions);
   OFLG_SET(OPTION_TI3020_CURS, &validOptions);
   OFLG_SET(OPTION_NO_TI3020_CURS, &validOptions);
   OFLG_SET(OPTION_TI3026_CURS, &validOptions);
   OFLG_SET(OPTION_IBMRGB_CURS, &validOptions);
   OFLG_SET(OPTION_DAC_8_BIT, &validOptions);
   OFLG_SET(OPTION_DAC_6_BIT, &validOptions);
   OFLG_SET(OPTION_SYNC_ON_GREEN, &validOptions);
   OFLG_SET(OPTION_SPEA_MERCURY, &validOptions);
   OFLG_SET(OPTION_NUMBER_NINE, &validOptions);
   OFLG_SET(OPTION_STB_PEGASUS, &validOptions);
   OFLG_SET(OPTION_MIRO_MAGIC_S4, &validOptions);
#ifdef PC98
   OFLG_SET(OPTION_PCSKB, &validOptions);
   OFLG_SET(OPTION_PCSKB4, &validOptions);
   OFLG_SET(OPTION_PCHKB, &validOptions);
   OFLG_SET(OPTION_NECWAB, &validOptions);
   OFLG_SET(OPTION_PW805I, &validOptions);
   OFLG_SET(OPTION_PWLB, &validOptions);
   OFLG_SET(OPTION_PW968, &validOptions);
   OFLG_SET(OPTION_EPSON_MEM_WIN, &validOptions);
   OFLG_SET(OPTION_PW_MUX, &validOptions);
   OFLG_SET(OPTION_NOINIT, &validOptions);
#endif
   /* ELSA_W1000PRO isn't really required any more */
   OFLG_SET(OPTION_ELSA_W1000PRO, &validOptions);
   OFLG_SET(OPTION_ELSA_W2000PRO, &validOptions);
   OFLG_SET(OPTION_DIAMOND, &validOptions);
   OFLG_SET(OPTION_GENOA, &validOptions);
   OFLG_SET(OPTION_STB, &validOptions);
   OFLG_SET(OPTION_HERCULES, &validOptions);
   if (S3_928_P(s3ChipId))
      OFLG_SET(OPTION_PCI_HACK, &validOptions);
   OFLG_SET(OPTION_POWER_SAVER, &validOptions);
   OFLG_SET(OPTION_S3_964_BT485_VCLK, &validOptions);
   OFLG_SET(OPTION_SLOW_DRAM, &validOptions);
   OFLG_SET(OPTION_SLOW_EDODRAM, &validOptions);
   OFLG_SET(OPTION_SLOW_VRAM, &validOptions);
   OFLG_SET(OPTION_SLOW_DRAM_REFRESH, &validOptions);
   OFLG_SET(OPTION_FAST_VRAM, &validOptions);
   OFLG_SET(OPTION_TRIO32_FC_BUG, &validOptions);
   OFLG_SET(OPTION_S3_968_DASH_BUG, &validOptions);
   OFLG_SET(OPTION_TRIO64VP_BUG1, &validOptions);
   OFLG_SET(OPTION_TRIO64VP_BUG2, &validOptions);
   OFLG_SET(OPTION_TRIO64VP_BUG3, &validOptions);
   xf86VerifyOptions(&validOptions, &s3InfoRec);

#ifdef PC98
   if (OFLG_ISSET(OPTION_PW_MUX, &s3InfoRec.options)) {
      OFLG_SET(OPTION_SPEA_MERCURY, &s3InfoRec.options);
    }
#endif

   if (S3_x64_SERIES(s3ChipId))
      if (OFLG_ISSET(OPTION_NO_MEM_ACCESS, &s3InfoRec.options)) {
	 ErrorF("%s %s: Option \"nomemaccess\" is ignored for 86x/96x/TRIOxx\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_CLR(OPTION_NO_MEM_ACCESS, &s3InfoRec.options);
      }

   /* LocalBus or EISA or PCI */
   s3Localbus = ((config & 0x03) <= 2) || S3_928_P(s3ChipId);

   if (xf86Verbose) {
      if (S3_928_P(s3ChipId)) {
	 ErrorF("%s %s: card type: PCI\n", XCONFIG_PROBED, s3InfoRec.name);
      } else {
	 switch (config & 0x03) {
	 case 0:
	    ErrorF("%s %s: card type: EISA\n", XCONFIG_PROBED, s3InfoRec.name);
	    break;
	 case 1:
            ErrorF("%s %s: card type: 386/486 localbus\n",
        	   XCONFIG_PROBED, s3InfoRec.name);
	    s3VLB = TRUE;
	    break;
	 case 3:
            ErrorF("%s %s: card type: ISA\n", XCONFIG_PROBED, s3InfoRec.name);
	    break;
	 case 2:
	    ErrorF("%s %s: card type: PCI\n", XCONFIG_PROBED, s3InfoRec.name);
	 }
      }
   }

   if (OFLG_ISSET(OPTION_TRIO32_FC_BUG, &s3InfoRec.options))
      s3Trio32FCBug = TRUE;
   if (OFLG_ISSET(OPTION_S3_968_DASH_BUG, &s3InfoRec.options))
      s3_968_DashBug = TRUE;

   if (OFLG_ISSET(OPTION_GENOA, &s3InfoRec.options))
      s3BiosVendor = GENOA_BIOS;
   else if (OFLG_ISSET(OPTION_STB, &s3InfoRec.options))
      s3BiosVendor = STB_BIOS;
   else if (OFLG_ISSET(OPTION_HERCULES, &s3InfoRec.options))
      s3BiosVendor = HERCULES_BIOS;
   else if (OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options))
      s3BiosVendor = NUMBER_NINE_BIOS;

   card_id = s3DetectMIRO_20SV_Rev(s3InfoRec.BIOSbase);
   if (card_id > 1) {
      ErrorF("%s %s: MIRO 20SV Rev.2 or newer detected.\n",
             XCONFIG_PROBED, s3InfoRec.name);
      if (!OFLG_ISSET(OPTION_S3_964_BT485_VCLK, &s3InfoRec.options))
	 ErrorF("\tplease use Option \"s3_964_bt485_vclk\"\n");
   }

   if (find_bios_string(s3InfoRec.BIOSbase,"Stealth",
			"Diamond Computer Systems, Inc.") != NULL ||
       find_bios_string(s3InfoRec.BIOSbase,"Stealth",
			"Diamond Multimedia Systems, Inc.") != NULL) {
      if (s3BiosVendor == UNKNOWN_BIOS) 
	 s3BiosVendor = DIAMOND_BIOS;
      if (xf86Verbose)
	 ErrorF("%s %s: Diamond Stealth BIOS found\n",
		XCONFIG_PROBED, s3InfoRec.name);
   }

   card_id = s3DetectELSA(s3InfoRec.BIOSbase, &card, &serno, &max_pix_clock,
			  &max_mem_clock, &hwconf);
   if (card_id > 0) {
      if (s3BiosVendor == UNKNOWN_BIOS) 
	 s3BiosVendor = ELSA_BIOS;
      if (xf86Verbose)
         ErrorF("%s %s: card: %s, Ser.No. %s\n",
	        XCONFIG_PROBED, s3InfoRec.name, card, serno);
      xfree(card);
      xfree(serno);

      if (s3InfoRec.dacSpeed <= 0)
	 s3InfoRec.dacSpeed = max_pix_clock;

      do {
	 switch (card_id) {
	 case ELSA_WINNER_1000:
	 case ELSA_WINNER_1000VL:
	 case ELSA_WINNER_1000PCI:
	 case ELSA_WINNER_1000ISA:
	    if (s3ProbeSDAC(TRUE)) 
	       continue;  /* SDAC detected, don't set ICD2061A clock */
	    S3ProbeATT4xx(TRUE);
	    /* if ATT20C498/09/99 is detected, the clockchip is 
	     * already set apropriately 
	     */
	    break;
	 case ELSA_WINNER_1000AVI:
	 case ELSA_WINNER_1000PRO:
	    /* This option isn't required at the moment */
	    OFLG_SET(OPTION_ELSA_W1000PRO,  &s3InfoRec.options);
	    if (s3ProbeSDAC(TRUE)) 
	       continue;  /* SDAC detected, don't set ICD2061A clock */
            S3ProbeATT4xx(TRUE);
               /* if ATT20C498/09/99 is detected, the clockchip is 
                * already set apropriately 
                */
	    break;
	 case ELSA_WINNER_2000PRO:
	    OFLG_SET(OPTION_ELSA_W2000PRO,  &s3InfoRec.options);
	    break;
	 case ELSA_WINNER_2000:
	 case ELSA_WINNER_2000VL:
	 case ELSA_WINNER_2000PCI:
	    break;  /* set ICD2061A clock chip */
	 case ELSA_GLORIA_8:
	    if (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
	       FatalError("%s %s: for the ELSA Gloria-8 card you should not specify a clock chip!\n",
			   XCONFIG_GIVEN, s3InfoRec.name);
	    }
	    OFLG_SET(CLOCK_OPTION_GLORIA8, &s3InfoRec.clockOptions);
	    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
	    clockchip_probed = XCONFIG_PROBED;
	    /* fall through ... */

	 case ELSA_WINNER_2000AVI:
	 case ELSA_WINNER_2000PRO_X:
	 case ELSA_GLORIA_4:
	    if (OFLG_ISSET(OPTION_ELSA_W2000PRO,&s3InfoRec.options)) {
	       ErrorF("%s %s: for Ti3026/3030 RAMDACs you must not specify Option \"elsa_w2000pro\"\n",
		      XCONFIG_PROBED, s3InfoRec.name);
	       OFLG_CLR(OPTION_ELSA_W2000PRO, &s3InfoRec.options);
	    }
	    if ((card_id==ELSA_WINNER_2000PRO_X) && (hwconf & 2)) {
	       /*
	        * this version of the Winner 2000PRO/X has an external ICS9161A
	        * clockchip, so set ICD2061A flag
	        */
               if (xf86Verbose)
	          ErrorF("%s %s: Rev. G Winner 2000PRO/X with external clockchip detected\n",
		         XCONFIG_PROBED, s3InfoRec.name);
	       break;
	    } else {
	       continue;
            }
	 case ELSA_WINNER_1000PRO_TRIO32:
	 case ELSA_WINNER_1000PRO_TRIO64:
	 default: 
	    continue; /* unknown card_id, don't set ICD2061A flags */
	 }

	 /* a known ELSA card_id was returned, set ICD 2061A clock support 
	    if there is no ClockChip specified in XF86Config */

	 if (!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
	    OFLG_SET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions);
	    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
	    clockchip_probed = XCONFIG_PROBED;
	 }
      } while (0);
   }

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
#if 0
	 } else if (S3_ViRGE_SERIES(s3ChipId)) {
	    chipname = "ViRGE";
#endif
	 } else if (S3_TRIO32_SERIES(s3ChipId)) {
	    chipname = "Trio32";
	 } else if (S3_TRIO64V_SERIES(s3ChipId /* , s3ChipRev */)) {
	    chipname = "Trio64V+";
	 } else if (S3_TRIO64_SERIES(s3ChipId)) {
	    chipname = "Trio64";
	 }
	 ErrorF("%s %s: chipset:   %s rev. %x\n",
                XCONFIG_PROBED, s3InfoRec.name, chipname, s3ChipRev);
      } else if (S3_801_928_SERIES(s3ChipId)) {
	 if (S3_801_SERIES(s3ChipId)) {
            if (S3_805_I_SERIES(s3ChipId)) {
               ErrorF("%s %s: chipset:   805i",
                      XCONFIG_PROBED, s3InfoRec.name);
               if ((config & 0x03) == 3)
                  ErrorF(" (ISA)");
               else
                  ErrorF(" (VL)");
            }
	    else if (!((config & 0x03) == 3))
	       ErrorF("%s %s: chipset:   805",
                      XCONFIG_PROBED, s3InfoRec.name);
	    else
	       ErrorF("%s %s: chipset:   801",
                       XCONFIG_PROBED, s3InfoRec.name);
	    ErrorF(", ");
	    if (S3_801_REV_C(s3ChipId))
	       ErrorF("rev C or above\n");
	    else
	       ErrorF("rev A or B\n");
	 } else if (S3_928_SERIES(s3ChipId)) {
	    char *pci = S3_928_P(s3ChipId) ? "-P" : "";
	    if (S3_928_REV_E(s3ChipId))
		ErrorF("%s %s: chipset:   928%s, rev E or above\n",
                   XCONFIG_PROBED, s3InfoRec.name, pci);
	    else
	        ErrorF("%s %s: chipset:   928%s, rev D or below\n",
                   XCONFIG_PROBED, s3InfoRec.name, pci);
	 }
      } else if (S3_911_SERIES(s3ChipId)) {
	 if (S3_911_ONLY(s3ChipId)) {
	    ErrorF("%s %s: chipset:   911 \n",
                   XCONFIG_PROBED, s3InfoRec.name);
	 } else if (S3_924_ONLY(s3ChipId)) {
	    ErrorF("%s %s: chipset:   924\n",
                   XCONFIG_PROBED, s3InfoRec.name);
	 } else {
	    ErrorF("%s %s: S3 chipset type unknown, chip_id = 0x%02x\n",
		   XCONFIG_PROBED, s3InfoRec.name, s3ChipId);
	 }
      }
   }

   if (xf86Verbose) {
      ErrorF("%s %s: chipset driver: %s\n",
	     OFLG_ISSET(XCONFIG_CHIPSET, &s3InfoRec.xconfigFlag) ?
		XCONFIG_GIVEN : XCONFIG_PROBED,
	     s3InfoRec.name, s3InfoRec.chipset);
   }

   if (!s3InfoRec.videoRam) {
      if (((config & 0x20) != 0) 	/* if bit 5 is a 1, then 512k RAM */
          && (!S3_964_SERIES(s3ChipId))) {
	 s3InfoRec.videoRam = 512;
      } else {			/* must have more than 512k */
	 if (S3_911_SERIES(s3ChipId)) {
	    s3InfoRec.videoRam = 1024;
	 } else {
	    switch ((config & 0xE0) >> 5) {	/* look at bits 6 and 7 */
	       case 0:
	         s3InfoRec.videoRam = 4096;
		 break;
	       case 2:
	         s3InfoRec.videoRam = 3072;
		 break;
	       case 3:
	         s3InfoRec.videoRam = 8192;
		 break;
	       case 4:
		 s3InfoRec.videoRam = 2048;
	         break;
	       case 5:
		 s3InfoRec.videoRam = 6144;
	         break;
	       case 6:
	         s3InfoRec.videoRam = 1024;
		 break;
	    }
	 }
      }
      if (xf86Verbose) {
         ErrorF("%s %s: videoram:  %dk\n",
              XCONFIG_PROBED, s3InfoRec.name, s3InfoRec.videoRam);
      }
   } else {
      if (xf86Verbose) {
	 ErrorF("%s %s: videoram:  %dk\n", 
              XCONFIG_GIVEN, s3InfoRec.name, s3InfoRec.videoRam);
      }
   }
   if (s3InfoRec.videoRam > 1024)
      s3Mbanks = -1;
   else
      s3Mbanks = 0;

   if (xf86bpp < 0) {
      xf86bpp = s3InfoRec.depth;
   }
   if (xf86weight.red == 0 || xf86weight.green == 0 || xf86weight.blue == 0) {
      xf86weight = s3InfoRec.weight;
   }
   switch (xf86bpp) {
   case 8:
      break;
   case 15:
      s3InfoRec.depth = 15;
      xf86bpp = 16;
      s3Weight = RGB16_555;
      xf86weight.red = xf86weight.green = xf86weight.blue = 5;
      s3InfoRec.bitsPerPixel = 16;
      if (s3InfoRec.defaultVisual < 0)
	 s3InfoRec.defaultVisual = TrueColor;
      if (defaultColorVisualClass < 0)
	 defaultColorVisualClass = s3InfoRec.defaultVisual;
      break;
   case 16:
      if (xf86weight.red==5 && xf86weight.green==5 && xf86weight.blue==5) {
	 s3Weight = RGB16_555;
	 s3InfoRec.depth = 15;
      }
      else if (xf86weight.red==5 && xf86weight.green==6 && xf86weight.blue==5) {
	 s3Weight = RGB16_565;
	 s3InfoRec.depth = 16;
      }
      else {
	 ErrorF(
	   "Invalid color weighting %1d%1d%1d (only 555 and 565 are valid)\n",
	   xf86weight.red,xf86weight.green,xf86weight.blue);
	 xf86DisableIOPorts(s3InfoRec.scrnIndex);
	 return(FALSE);
      }
      s3InfoRec.bitsPerPixel = 16;
      if (s3InfoRec.defaultVisual < 0)
	 s3InfoRec.defaultVisual = TrueColor;
      if (defaultColorVisualClass < 0)
	 defaultColorVisualClass = s3InfoRec.defaultVisual;
      break;
   case 24:
#ifdef NOT_YET
      s3InfoRec.depth = 24;
      s3InfoRec.bitsPerPixel = 32; /* Use packed 24 bpp (RGB) but this 
				      should be transparant for clients */
      s3InfoRec.bitsPerPixel = 24; /* not not yet or not here ? HACK24 */ 
      s3Weight = RGB32_888;
      /* s3MaxClock = S3_MAX_32BPP_CLOCK; */
      xf86weight.red =  xf86weight.green = xf86weight.blue = 8;
      if (s3InfoRec.defaultVisual < 0)
	 s3InfoRec.defaultVisual = TrueColor;
      if (defaultColorVisualClass < 0)
	 defaultColorVisualClass = s3InfoRec.defaultVisual;
      break;
#else
      xf86bpp = 32;
      /* FALLTHROUGH */
#endif
   case 32:
      s3InfoRec.depth = 24;
      s3InfoRec.bitsPerPixel = 32; /* Use sparse 24 bpp (RGBX) */
      s3Weight = RGB32_888;
      /* s3MaxClock = S3_MAX_32BPP_CLOCK; */
      xf86weight.red =  xf86weight.green = xf86weight.blue = 8;
      if (s3InfoRec.defaultVisual < 0)
	 s3InfoRec.defaultVisual = TrueColor;
      if (defaultColorVisualClass < 0)
	 defaultColorVisualClass = s3InfoRec.defaultVisual;
      break;
   default:
      ErrorF(
	"Invalid value for bpp.  Valid values are 8, 15, 16, 24 and 32.\n");
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return(FALSE);
   }

   if (s3InfoRec.bitsPerPixel > 8 &&
       defaultColorVisualClass >= 0 && defaultColorVisualClass != TrueColor) {
      ErrorF("Invalid default visual type: %d (%s)\n", defaultColorVisualClass,
	     xf86VisualNames[defaultColorVisualClass]);
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return(FALSE);
   }

   s3Bpp = xf86bpp / 8;

   /* Make sure CR55 is unlocked for Bt485 probe */
   outb(vgaCRIndex, 0x39);
   outb(vgaCRReg, 0xA5);

   if (S3_TRIOxx_SERIES(s3ChipId)) {
      if (s3RamdacType != UNKNOWN_DAC && !DAC_IS_TRIO) {
	 ErrorF("%s %s: for Trio32/64 chips you shouldn't specify a Ramdac\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_CLR(XCONFIG_RAMDAC, &s3InfoRec.xconfigFlag);
      }
      if (!DAC_IS_TRIO) {
	 if (S3_TRIO32_SERIES(s3ChipId))
	    s3RamdacType = S3_TRIO32_DAC;
	 else 
	    s3RamdacType = S3_TRIO64_DAC;
	 s3InfoRec.ramdac = xf86TokenToString(s3DacTable, s3RamdacType);
      }
      if ( OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions) &&
	  !OFLG_ISSET(CLOCK_OPTION_S3TRIO, &s3InfoRec.clockOptions)) {
	 ErrorF("%s %s: for Trio32/64 chips you shouldn't specify a Clockchip\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 /* Clear the other clock options */
	 OFLG_ZERO(&s3InfoRec.clockOptions);
      }
      if (!OFLG_ISSET(CLOCK_OPTION_S3TRIO, &s3InfoRec.clockOptions)) {
	 OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
	 OFLG_SET(CLOCK_OPTION_S3TRIO, &s3InfoRec.clockOptions);
	 clockchip_probed = XCONFIG_PROBED;
      }
   }
   else if (S3_928_SERIES(s3ChipId) || S3_x64_SERIES(s3ChipId)
       || S3_805_I_SERIES(s3ChipId)) {
      /* First probe for Ti3026 */
      if (s3RamdacType == UNKNOWN_DAC) {
	 unsigned char saveCR55, saveCR45, saveCR43, saveID, saveTIndx;

	 outb(vgaCRIndex, 0x43);
	 saveCR43 = inb(vgaCRReg);
	 outb(vgaCRReg, saveCR43 & ~0x02);

	 outb(vgaCRIndex, 0x45);
	 saveCR45 = inb(vgaCRReg);
	 outb(vgaCRReg, saveCR45 & ~0x20);

	 outb(vgaCRIndex, 0x55);
	 saveCR55 = inb(vgaCRReg);

	 outb(vgaCRReg, (saveCR55 & 0xFC) | 0x00);
	 saveTIndx = inb(0x3c8);
	 outb(0x3c8, 0x3f);

	 outb(vgaCRReg, (saveCR55 & 0xFC) | 0x02);
	 saveID = inb(0x3c6);
	 if (saveID == 0x26 || saveID == 0x30) {
	    outb(0x3c6, ~saveID);    /* check if ID reg. is read-only */
	    if (inb(0x3c6) != saveID) {
	       outb(0x3c6, saveID);
	    }
	    else {
	       /*
		* Found TI ViewPoint 3026/3030 DAC
		*/
	       int rev;
	       outb(vgaCRReg, (saveCR55 & 0xFC) | 0x00);
	       saveTIndx = inb(0x3c8);
	       outb(0x3c8, 0x01);
	       outb(vgaCRReg, (saveCR55 & 0xFC) | 0x02);
	       rev = inb(0x3c6);
	       
	       ErrorF("%s %s: Detected a TI ViewPoint 30%02x RAMDAC rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, saveID, rev);
	       if (saveID == 0x26)
		  s3RamdacType = TI3026_DAC;
	       else  /* saveID == 0x30 */
		  s3RamdacType = TI3030_DAC;
	       saveCR43 &= ~0x02;
	       saveCR45 &= ~0x20;
	    }
	 }

	 /* restore this mess */
	 outb(vgaCRReg, (saveCR55 & 0xFC) | 0x00);
	 outb(0x3c8, saveTIndx);

	 outb(vgaCRIndex, 0x55);
	 outb(vgaCRReg, saveCR55);

	 outb(vgaCRIndex, 0x45);
	 outb(vgaCRReg, saveCR45);

	 outb(vgaCRIndex, 0x43);
	 outb(vgaCRReg, saveCR43);
      }

      /* Then probe for Ti3020 and Ti3025 */
      if (s3RamdacType == UNKNOWN_DAC) {
	 unsigned char saveCR55, saveCR5C, saveCR45, saveCR43, saveTIndx, saveTIndx2, saveTIdata;

	 outb(vgaCRIndex, 0x43);
	 saveCR43 = inb(vgaCRReg);
	 outb(vgaCRReg, saveCR43 & ~0x02);

	 outb(vgaCRIndex, 0x45);
	 saveCR45 = inb(vgaCRReg);
	 outb(vgaCRReg, saveCR45 & ~0x20);

	 outb(vgaCRIndex, 0x55);
	 saveCR55 = inb(vgaCRReg);
	 /* toggle to upper 4 direct registers */
	 outb(vgaCRReg, (saveCR55 & 0xFC) | 0x01);

	 saveTIndx = inb(TI_INDEX_REG);
	 outb(TI_INDEX_REG, TI_ID);
	 if (inb(TI_DATA_REG) == TI_VIEWPOINT20_ID) {
	    /*
	     * Found TI ViewPoint 3020 DAC
	     */
	    ErrorF("%s %s: Detected a TI ViewPoint 3020 RAMDAC\n",
	           XCONFIG_PROBED, s3InfoRec.name);
	    s3RamdacType = TI3020_DAC;
	    saveCR43 &= ~0x02;
	    saveCR45 &= ~0x20;
	 } else {
	    outb(vgaCRIndex, 0x5C);
	    saveCR5C = inb(vgaCRReg);
	    /* clear 0x20 (RS4) for 3020 mode */
	    outb(vgaCRReg, saveCR5C & 0xDF);
	    saveTIndx2 = inb(TI_INDEX_REG);
	    /* already twiddled CR55 above */
	    outb(TI_INDEX_REG, TI_CURS_CONTROL);
	    saveTIdata = inb(TI_DATA_REG);
	    /* clear TI_PLANAR_ACCESS bit */
	    outb(TI_DATA_REG, saveTIdata & 0x7F);

	    outb(TI_INDEX_REG, TI_ID);
	    if (inb(TI_DATA_REG) == TI_VIEWPOINT25_ID) {
	       /*
	        * Found TI ViewPoint 3025 DAC
	        */
	       ErrorF("%s %s: Detected a TI ViewPoint 3025 RAMDAC\n",
	              XCONFIG_PROBED, s3InfoRec.name);
	       s3RamdacType = TI3025_DAC;
	       saveCR43 &= ~0x02;
	       saveCR45 &= ~0x20;
	    }

	    /* restore this mess */
	    outb(TI_INDEX_REG, TI_CURS_CONTROL);
	    outb(TI_DATA_REG, saveTIdata);
	    outb(TI_INDEX_REG, saveTIndx2);
	    outb(vgaCRIndex, 0x5C);
	    outb(vgaCRReg, saveCR5C);
	 }
	 outb(TI_INDEX_REG, saveTIndx);
	 outb(vgaCRIndex, 0x55);
	 outb(vgaCRReg, saveCR55);

	 outb(vgaCRIndex, 0x45);
	 outb(vgaCRReg, saveCR45);

	 outb(vgaCRIndex, 0x43);
	 outb(vgaCRReg, saveCR43);
      }

      /*
       * Bt485/AT&T20C505 next
       *
       * Probe for the bloody thing.  Set 0x3C6 to a bogus value, then
       * try to get the Bt485 status register.  If it's there, then we will
       * get something else back from this port.
       */

      if (s3RamdacType == UNKNOWN_DAC) {
         unsigned char tmp2;
         tmp = inb(0x3C6);
         outb(0x3C6, 0x0F);
         if (((tmp2 = s3InBtStatReg()) & 0x80) == 0x80) {
          /*
           * Found either a BrookTree Bt485 or AT&T 20C505.
           */
          if ((tmp2 & 0xF0) == 0xD0) {
             s3RamdacType = ATT20C505_DAC;
             ErrorF("%s %s: Detected an AT&T 20C505 RAMDAC\n",
                    XCONFIG_PROBED, s3InfoRec.name);
          } else {
             s3RamdacType = BT485_DAC;
             ErrorF("%s %s: Detected a BrookTree Bt485 RAMDAC\n",
                    XCONFIG_PROBED, s3InfoRec.name);

             /* If it is a Bt485 and no clockchip is specified in the 
                XF86Config, set clockchips for SPEA Mercury / Mercury P64 */

             if (!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions))
              if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options)) {
               if (S3_964_SERIES(s3ChipId)) {
                   OFLG_SET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions);
                   OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
                   clockchip_probed = XCONFIG_PROBED;
               } else if (S3_928_ONLY(s3ChipId)) {
                   OFLG_SET(CLOCK_OPTION_SC11412, &s3InfoRec.clockOptions);
                   OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
                   clockchip_probed = XCONFIG_PROBED;
               }
              }    
          }
         }
         outb(0x3C6, tmp);
      }

      /* If it wasn't a Bt485, probe for the ATT 20C498/409/499 */
      if (s3RamdacType == UNKNOWN_DAC) {
         s3RamdacType = S3ProbeATT4xx(FALSE);
      }

      /* now, probe for the SC 15025/26 */
      if (s3RamdacType == UNKNOWN_DAC) {
	 int i;
	 unsigned char c,id[4];
	 c = xf86getdaccomm();
	 xf86setdaccomm(c | 0x10);
	 for (i=0; i<4; i++) {
	    outb(0x3C7, 0x9+i); 
	    id[i] = inb(0x3C8);
	 }
	 xf86setdaccomm(c);
	 xf86dactopel();
	 if (id[0] == 'S' &&                  /* Sierra */
	     ((id[1]<<8)|id[2]) == 15025) {   /* unique for the SC 15025/26 */
	    if (id[3] != 'A') {                     /* version number */
	       ErrorF(
		"%s %s: ==> New Sierra SC 15025/26 version (0x%x) found,\n",
		XCONFIG_PROBED, s3InfoRec.name, id[3]);
	       ErrorF("\tplease report!\n");
	    }
	    ErrorF("%s %s: Detected a Sierra SC 15025/26 RAMDAC\n",
	           XCONFIG_PROBED, s3InfoRec.name);
	    s3RamdacType = SC15025_DAC;
	 }
      }

      /* If it wasn't a Sierra, probe for the STG1700 and STG1703 */
      if (s3RamdacType == UNKNOWN_DAC) 
      {
         int cid, did, daccomm, readmask;

	 readmask = inb(0x3c6);
	 xf86dactopel();
	 daccomm = xf86getdaccomm();
	 xf86setdaccommbit(0x10);
	 xf86dactocomm();
         inb(0x3c6);
         outb(0x3c6, 0x00);
         outb(0x3c6, 0x00);
         cid = inb(0x3c6);     /* company ID */
         did = inb(0x3c6);     /* device ID */
	 xf86dactopel();
	 outb(0x3c6,readmask);

         if ((cid == 0x44) && (did == 0x00))
         {
	    ErrorF("%s %s: Detected an STG1700 RAMDAC\n",
	           XCONFIG_PROBED, s3InfoRec.name);
	    s3RamdacType = STG1700_DAC;
         }
         else if ((cid == 0x44) && (did == 0x03))
         {
	    ErrorF("%s %s: Detected an STG1703 RAMDAC\n",
	           XCONFIG_PROBED, s3InfoRec.name);
	    s3RamdacType = STG1703_DAC;
	    if (!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE,
			    &s3InfoRec.clockOptions)) {
	       OFLG_SET(CLOCK_OPTION_STG1703,    &s3InfoRec.clockOptions);
	       OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
	       clockchip_probed = XCONFIG_PROBED;
	    }
         }
	 xf86setdaccomm(daccomm);
      }

      /* probe for IBM RGB52x ramdac */
      if (s3RamdacType == UNKNOWN_DAC) {
	 int ibm_id;
	 
	 if ((ibm_id = s3IBMRGB_Probe())) {
	    s3IBMRGB_Init();
	    switch(ibm_id >> 8) {
	    case 1:
	       ErrorF("%s %s: Detected an IBM RGB525 ramdac rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, ibm_id&0xff);
	       s3RamdacType = IBMRGB525_DAC;
	       break;
	    case 2:
	       if ( (ibm_id & 0xff) == 0xf0) {
	          ErrorF("%s %s: Detected an IBM RGB528 ramdac rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, ibm_id&0xff);
	          s3RamdacType = IBMRGB528_DAC;
	       }
	       
	       else if ( (ibm_id & 0xff) == 0xe0) {
	          ErrorF("%s %s: Detected an IBM RGB528A ramdac rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, ibm_id&0xff);
	          s3RamdacType = IBMRGB528_DAC;
	       }
	       else {
	          ErrorF("%s %s: Detected an IBM RGB524 ramdac rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, ibm_id&0xff);
	          s3RamdacType = IBMRGB524_DAC;
	       }
	       break;
	    case 0x102:
	       ErrorF("%s %s: Detected an IBM RGB528 ramdac rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, ibm_id&0xff);
	       s3RamdacType = IBMRGB528_DAC;
	       break;
	    default:
	       ErrorF("%s %s: Detected an unknown IBM RGB52x ramdac rev. %x\n",
		      XCONFIG_PROBED, s3InfoRec.name, ibm_id);
	       ErrorF("\tplease report!\n");

	    }
	 }
      }
   }
   
   /* probe for S3 GENDAC or SDAC */

   if (S3_864_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId) 
       || S3_801_SERIES(s3ChipId)) {
      if (s3RamdacType == UNKNOWN_DAC) {
	 if (s3ProbeSDAC(FALSE) &&
	     !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
	    OFLG_SET(CLOCK_OPTION_S3GENDAC,    &s3InfoRec.clockOptions);
	    OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
	    clockchip_probed = XCONFIG_PROBED;
	 }
      }
   }
   
   /* probe for some of the other HiColor DACs */

   if (S3_8XX_9XX_SERIES(s3ChipId)) {
      if (s3RamdacType == UNKNOWN_DAC) {
#define Setcomm(v)        (xf86dactocomm(),outb(0x3C6,v),\
                        xf86dactocomm(),inb(0x3C6))
         unsigned char tmp, olddacpel, olddaccomm, notdaccomm;

         (void) xf86dactocomm();
         olddaccomm = inb(0x3C6);
         xf86dactopel();
         olddacpel = inb(0x3C6);

         notdaccomm = ~olddaccomm;
         outb(0x3C6, notdaccomm);
         (void) xf86dactocomm();

         if (inb(0x3C6) != notdaccomm) {
            /* Looks like a HiColor RAMDAC */
            if ((Setcomm(0xE0) & 0xE0) == 0xE0) {
               if ((Setcomm(0x60) & 0xE0) == 0x00) {
                  if ((Setcomm(0x02) & 0x02) != 0x00) {
                     s3RamdacType = ATT20C490_DAC;
                  }
               } else {
                  tmp = Setcomm(olddaccomm);
                  if (inb(0x3C6) != notdaccomm) {
                     (void) inb(0x3C6);
                     (void) inb(0x3C6);
                     (void) inb(0x3C6);
                     if (inb(0x3C6) != notdaccomm) {
                        xf86dactopel();
                        outb(0x3C6, 0xFF);
                        (void) inb(0x3C6);
                        (void) inb(0x3C6);
                        (void) inb(0x3C6);
                        switch (inb(0x3C6)) {
                        case 0x44:
                        case 0x82:
			    break;
                        case 0x8E:
			    s3RamdacType = SS2410_DAC;		
			    break;
                        default:
                            xf86dactopel();
                            outb(0x3C6, olddacpel & 0xFB);
                            xf86dactocomm();
                            outb(0x3C6, olddaccomm & 0x04);
                            tmp = inb(0x3C6);
                            outb(0x3C6, tmp & 0xFB);
                            if ((tmp & 0x04) == 0) {
                               s3RamdacType = SC1148x_M2_DAC;
                            }
                        }
                     }
                  }
               }
            }
         }
         (void) xf86dactocomm();
         outb(0x3C6, olddaccomm);
         xf86dactopel();
         outb(0x3C6, olddacpel);
#undef Setcomm
      }
   }

   /* If we still don't know the ramdac type, set it to NORMAL_DAC */
   if (s3RamdacType == UNKNOWN_DAC) {
      s3RamdacType = NORMAL_DAC;
   }
  
   if ((!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions) &&
       !OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
       (s3InfoRec.ramdac == NULL)) || S3_TRIO64_SERIES(s3ChipId))
				   { /* ensure that autodetection can be */
                                     /* overwritten 			 */	
     card_id = check_SPEA_bios(s3InfoRec.BIOSbase); 
     if (card_id > 0) {

       switch (s3RamdacType) {
       case BT485_DAC: 
       case ATT20C505_DAC: 
          if (S3_928_ONLY(s3ChipId)) {
             /* SPEA Mercury */
             ErrorF("%s %s: SPEA Mercury detected.\n",
             XCONFIG_PROBED, s3InfoRec.name);
             OFLG_SET(OPTION_SPEA_MERCURY, &s3InfoRec.options);
             OFLG_SET(CLOCK_OPTION_SC11412, &s3InfoRec.clockOptions);
             OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
             clockchip_probed = XCONFIG_PROBED; 
          } else if  (S3_964_SERIES(s3ChipId)) { 
             /* SPEA Mercury P64 */ 
             ErrorF("%s %s: SPEA Mercury P64 detected.\n",
             XCONFIG_PROBED, s3InfoRec.name);
             OFLG_SET(OPTION_SPEA_MERCURY, &s3InfoRec.options);
             OFLG_SET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions);
             OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
             clockchip_probed = XCONFIG_PROBED;
          } 
          break;
       case ATT20C498_DAC: 
          if (S3_864_SERIES(s3ChipId)) { 
            /* SPEA MirageP64 Bios 3.xx */
            ErrorF("%s %s: SPEA Mirage P64 detected.\n",
            XCONFIG_PROBED, s3InfoRec.name);
            OFLG_SET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions);
            OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
            clockchip_probed = XCONFIG_PROBED;
          }
          break;
       case S3_SDAC_DAC:
          if (S3_864_SERIES(s3ChipId)) 
            /* SPEA Mirage P64 Bios 4.xx */
            ErrorF("%s %s: SPEA Mirage P64 detected.\n",
            XCONFIG_PROBED, s3InfoRec.name);
          break;
       case S3_TRIO64_DAC: 
          if (S3_TRIO64_SERIES(s3ChipId)) 
            /* SPEA Mirage P64 Bios 5.xx */
            ErrorF("%s %s: SPEA Mirage P64 Trio64 detected.\n",
            XCONFIG_PROBED, s3InfoRec.name);
          break;
       case S3_GENDAC_DAC:
          if (S3_801_SERIES(s3ChipId))
            /* SPEA Mirage Bios 5.x */
            ErrorF("%s %s: SPEA Mirage detected.\n",
            XCONFIG_PROBED, s3InfoRec.name);
          break;
      } 
     }
    }   /* end SPEA autodetect */

   /* make sure s3InfoRec.ramdac is set correctly */
   s3InfoRec.ramdac = xf86TokenToString(s3DacTable, s3RamdacType);

   /* Check Ramdac type is supported on the current S3 chipset */
   {
      char *chips = NULL;

      switch (s3RamdacType) {
      case BT485_DAC:
      case ATT20C505_DAC:
      case TI3020_DAC:
	 if (!S3_928_ONLY(s3ChipId) && !S3_964_SERIES(s3ChipId))
	    chips = "928 and 964 chips";
	 break;
      case TI3025_DAC:
	 if (!S3_964_SERIES(s3ChipId))
	    chips = "the 964 chip";
	 break;
      case TI3026_DAC:
      case TI3030_DAC:
      case IBMRGB524_DAC:
      case IBMRGB525_DAC:
      case IBMRGB528_DAC:
	 if (!S3_964_SERIES(s3ChipId) && !S3_968_SERIES(s3ChipId))
	    chips = "964 and 968 chips";
	 break;
      case ATT20C498_DAC:
      case ATT22C498_DAC:
      case ATT20C409_DAC:
      case STG1700_DAC:
      case STG1703_DAC:
      case S3_SDAC_DAC:
	 if (!S3_86x_SERIES(s3ChipId) && !S3_805_I_SERIES(s3ChipId))
	    chips = "864, 868 and 805i chips";
	 break;
      case S3_GENDAC_DAC:
	 if (!S3_801_SERIES(s3ChipId))
	    chips = "801 and 805 chips";
	 break;
      case S3_TRIO32_DAC:
      case S3_TRIO64_DAC:
	 if (!S3_TRIOxx_SERIES(s3ChipId))
	    chips = "Trio32 and Trio64";
	 break;
      }
      if (chips) {
	 ErrorF("%s %s: Ramdac \"%s\" is only supported with %s\n",
		XCONFIG_PROBED, s3InfoRec.name, s3InfoRec.ramdac, chips);
	 OFLG_CLR(XCONFIG_RAMDAC, &s3InfoRec.xconfigFlag);
	 /* Treat the ramdac as a "normal" dac */
	 s3RamdacType = NORMAL_DAC;
	 s3InfoRec.ramdac = xf86TokenToString(s3DacTable, s3RamdacType);
      }
   }

   if (S3_TRIOxx_SERIES(s3ChipId)) {
      if (!DAC_IS_TRIO) {
	 ErrorF("%s %s: for Trio32/64 chips you shouldn't specify a Ramdac\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_CLR(XCONFIG_RAMDAC, &s3InfoRec.xconfigFlag);
	 /* Treat the ramdac as a "normal" dac */
	 if (S3_TRIO32_SERIES(s3ChipId))
	    s3RamdacType = S3_TRIO32_DAC;
	 else 
	    s3RamdacType = S3_TRIO64_DAC;
	 s3InfoRec.ramdac = xf86TokenToString(s3DacTable, s3RamdacType);
      }
   }

   if (xf86Verbose) {
      ErrorF("%s %s: Ramdac type: %s\n",
	     OFLG_ISSET(XCONFIG_RAMDAC, &s3InfoRec.xconfigFlag) ?
	     XCONFIG_GIVEN : XCONFIG_PROBED, s3InfoRec.name, s3InfoRec.ramdac);
   }

   /* Check that the depth requested is supported by the ramdac/chipset */
   {
      char *reason = NULL;

      if (S3_801_SERIES(s3ChipId)) {
	 if (s3Bpp > 2)
	    reason = "801 and 805 chips";
      }
      else if (S3_911_SERIES(s3ChipId)) {
	 if (s3Bpp > 1)
	    reason = "911 and 924 chips";
      }
      if (!S3_868_SERIES(s3ChipId) && !S3_968_SERIES(s3ChipId)) {
	 if (s3Bpp == 3)
	    reason = "non-868/968 chips";	 
      }
      {
	 switch (s3RamdacType) {
	 case NORMAL_DAC:
	    if (s3Bpp > 1)
	       reason = "a \"normal\" RAMDAC";
	    break;
	 case ATT20C490_DAC:
	    /* XXXX Is this right ( ??? ) */
	    if (s3Bpp > 2)
	       reason = "an ATT20C490 RAMDAC";
	    break;
	 case SS2410_DAC: 
	    /* ???  This ramdac should do 24 bpp... */
	    if ( s3InfoRec.depth > 24 ) 
		reason = "an Diamond SS2410 RAMDAC";
	    break;
	 case SC1148x_M2_DAC:
	    if (s3InfoRec.depth > 15)
	       reason = "a Sierra 1148{2,3,4} RAMDAC";
	    break;
	 case SC1148x_M3_DAC:
	    if (s3Bpp > 2)
	       reason = "a Sierra 1148{5,7,9} RAMDAC";
	    break;
	 case BT485_DAC:
	 case ATT20C505_DAC:
	    /*
	     * Currently: 16bpp, 32bpp for SPEA Mercury (928 + Bt485)
	     *            16bpp, 32bpp for 964 + Bt485
	     */
	    if (!OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
		!S3_964_SERIES(s3ChipId)) {
#if 0
	       if (s3Bpp > 1)
		  reason = "Bt485 and ATT20C505 RAMDACs";
#endif
	    }
	    break;
	 case ATT20C498_DAC:
	 case ATT22C498_DAC:
	 case ATT20C409_DAC:
	 case STG1700_DAC:
	 case STG1703_DAC:
	 case S3_SDAC_DAC:
	 case S3_GENDAC_DAC:
	 case S3_TRIO32_DAC:
	 case S3_TRIO64_DAC:
	    break;
	 case SC15025_DAC:
	    break;
	 case TI3020_DAC:
	    break;
	 case TI3025_DAC:
	    break;
	 case TI3026_DAC:
	 case TI3030_DAC:
	    break;
	 case IBMRGB524_DAC:
	 case IBMRGB525_DAC:
	 case IBMRGB528_DAC:
	    break;
	 default:
	    /* Should never get here */
	    if (s3Bpp > 1)
	       reason = "an unknown RAMDAC";
	    break;
	 }
      }
      if (reason) {
	 ErrorF("Depth %d is not supported with %s\n", s3InfoRec.depth, reason);
	 xf86DisableIOPorts(s3InfoRec.scrnIndex);
	 return(FALSE);
      }
   }

   /* Now check/set the DAC speed */
   /* XXXX Are these reasonable defaults? */
   if (s3InfoRec.dacSpeed <= 0) {
      switch (s3RamdacType) {
      case NORMAL_DAC:
      case ATT20C490_DAC:
      case SC1148x_M2_DAC:
      case SC1148x_M3_DAC:
      case SS2410_DAC:		/* Just guessing at this based on the 490 */
	 s3InfoRec.dacSpeed = 110000;
	 break;
      case SC15025_DAC:
      case S3_GENDAC_DAC:
	 s3InfoRec.dacSpeed = 110000;
	 break;
      case BT485_DAC:
      case ATT20C505_DAC:
      case ATT20C498_DAC:
      case ATT22C498_DAC:
      case STG1700_DAC:
      case STG1703_DAC:
      case S3_SDAC_DAC:
      case S3_TRIO32_DAC:
      case S3_TRIO64_DAC:
	 s3InfoRec.dacSpeed = 135000;
	 break;
      case TI3020_DAC:
         if (OFLG_ISSET(OPTION_ELSA_W2000PRO, &s3InfoRec.options))
	    s3InfoRec.dacSpeed = 170000;
	 else
	    s3InfoRec.dacSpeed = 135000;
	 break;
      case TI3025_DAC:
	 s3InfoRec.dacSpeed = 135000;  /* push 135MHz part to 175 ? */
	 break;
      case TI3026_DAC:
	 s3InfoRec.dacSpeed = 135000;  /* push 135MHz part to 175 ? */
	 break;
      case TI3030_DAC:
	 s3InfoRec.dacSpeed = 175000;  /* available: 175/220/250 */
	 break;
      case ATT20C409_DAC:
	 s3InfoRec.dacSpeed = 135000;  /* use DacSpeed for the 170MHz part */
	 break;
      case IBMRGB524_DAC:
      case IBMRGB525_DAC:
      case IBMRGB528_DAC:
	 s3InfoRec.dacSpeed = 170000;
	 break;
      }
   }
   
   if (xf86Verbose) {
      ErrorF("%s %s: Ramdac speed: %d\n",
	     OFLG_ISSET(XCONFIG_DACSPEED, &s3InfoRec.xconfigFlag) ?
	     XCONFIG_GIVEN : XCONFIG_PROBED, s3InfoRec.name,
	     s3InfoRec.dacSpeed / 1000);
   }

   /* Now handle the various ramdac cursor options */

   if (OFLG_ISSET(OPTION_BT485_CURS, &s3InfoRec.options)) {
      if (DAC_IS_BT485_SERIES) {
	 ErrorF("%s %s: Using hardware cursor from Bt485/20C505 RAMDAC\n",
		XCONFIG_GIVEN, s3InfoRec.name);
      } else {
	 ErrorF("%s %s: Bt485 cursor requires a Bt485 or 20C505 RAMDAC\n",
		XCONFIG_PROBED, s3InfoRec.name);
      }
   }

   if (DAC_IS_TI3020_SERIES) {
      if (OFLG_ISSET(OPTION_NO_TI3020_CURS, &s3InfoRec.options)) {
         ErrorF("%s %s: Use of Ti3020 cursor disabled in XF86Config\n",
	        XCONFIG_GIVEN, s3InfoRec.name);
	 OFLG_CLR(OPTION_TI3020_CURS, &s3InfoRec.options);
      } else {
	 /* use the ramdac cursor by default */
	 ErrorF("%s %s: Using hardware cursor from Ti3020/25 RAMDAC\n",
	        OFLG_ISSET(OPTION_TI3020_CURS, &s3InfoRec.options) ?
		XCONFIG_GIVEN : XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_SET(OPTION_TI3020_CURS, &s3InfoRec.options);
      }
   } else {
      if (OFLG_ISSET(OPTION_TI3020_CURS, &s3InfoRec.options)) {
	 ErrorF("%s %s: Ti3020 cursor requires a Ti3020/25 RAMDAC\n",
		XCONFIG_PROBED, s3InfoRec.name);
      }
   }

   if (DAC_IS_TI3026 || DAC_IS_TI3030) {
      if (OFLG_ISSET(OPTION_SW_CURSOR, &s3InfoRec.options)) {
         ErrorF("%s %s: Use of Ti3026/3030 cursor disabled in XF86Config\n",
	        XCONFIG_GIVEN, s3InfoRec.name);
	 OFLG_CLR(OPTION_TI3026_CURS, &s3InfoRec.options);
      } else {
	 /* use the ramdac cursor by default */
	 ErrorF("%s %s: Using hardware cursor from Ti3026/3030 RAMDAC\n",
	        OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options) ?
		XCONFIG_GIVEN : XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_SET(OPTION_TI3026_CURS, &s3InfoRec.options);
      }
   } else {
      if (OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options)) {
	 ErrorF("%s %s: Ti3026/3030 cursor requires a Ti3026/3030 RAMDAC\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_CLR(OPTION_TI3026_CURS, &s3InfoRec.options);
      }
   }

   if (DAC_IS_IBMRGB) {
      if (OFLG_ISSET(OPTION_SW_CURSOR, &s3InfoRec.options)) {
         ErrorF("%s %s: Use of IBM RGB52x cursor disabled in XF86Config\n",
	        XCONFIG_GIVEN, s3InfoRec.name);
	 OFLG_CLR(OPTION_IBMRGB_CURS, &s3InfoRec.options);
      } else {
	 /* use the ramdac cursor by default */
	 ErrorF("%s %s: Using hardware cursor from IBM RGB52x RAMDAC\n",
	        OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options) ?
		XCONFIG_GIVEN : XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_SET(OPTION_IBMRGB_CURS, &s3InfoRec.options);
      }
   } else {
      if (OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options)) {
	 ErrorF("%s %s: IBM RGB52x cursor requires a IBM RGB52x RAMDAC\n",
		XCONFIG_PROBED, s3InfoRec.name);
	 OFLG_CLR(OPTION_IBMRGB_CURS, &s3InfoRec.options);
      }
   }

   /* Check when pixmux is supported */

   if (DAC_IS_BT485_SERIES &&
       (
	OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
	OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options) ||
	OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options) ||
	OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) ||
	S3_964_SERIES(s3ChipId)))
      s3Bt485PixMux = TRUE;

   if (    (DAC_IS_ATT498 || DAC_IS_STG1700   || DAC_IS_SDAC || 
            DAC_IS_TRIO   || DAC_IS_ATT20C409) 
        && (S3_x64_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId)))
      if (xf86bpp <= 8) s3ATT498PixMux = TRUE;

   /* Set the pix-mux description based on the ramdac type */
   if (DAC_IS_TI3026 || DAC_IS_TI3030) {
      pixMuxPossible = TRUE;
      allowPixMuxInterlace = TRUE;
      allowPixMuxSwitching = FALSE;
      nonMuxMaxClock = 70000;
      pixMuxLimitedWidths = FALSE;
      if (S3_964_SERIES(s3ChipId)) {
         nonMuxMaxClock = 0;  /* 964 can only be in pixmux mode when */
         pixMuxMinWidth = 0;  /* working in enhanced mode */  
      }
   } else if (DAC_IS_TI3020_SERIES) {
      pixMuxPossible = TRUE;
      allowPixMuxInterlace = FALSE;
      allowPixMuxSwitching = FALSE;
      nonMuxMaxClock = 70000;
      pixMuxLimitedWidths = FALSE;
      if (S3_964_SERIES(s3ChipId)) {
         nonMuxMaxClock = 0;  /* 964 can only be in pixmux mode when */
         pixMuxMinWidth = 0;  /* working in enhanced mode */  
      }
   } else if (DAC_IS_IBMRGB) {
      pixMuxPossible = TRUE;
      allowPixMuxInterlace = TRUE;
      allowPixMuxSwitching = TRUE;
      nonMuxMaxClock = 70000;
      pixMuxLimitedWidths = FALSE;
      if (S3_964_SERIES(s3ChipId)) {
         nonMuxMaxClock = 0;  /* 964 can only be in pixmux mode when */
         pixMuxMinWidth = 0;  /* working in enhanced mode */  
      }
   } else if (s3ATT498PixMux) {
      pixMuxPossible = TRUE;
      if (DAC_IS_ATT20C498 && !DAC_IS_ATT22C498) {
	 if (S3_866_SERIES(s3ChipId) || S3_868_SERIES(s3ChipId)) {
	    nonMuxMaxClock = 100000; /* 866/868 DCLK limit */
	    pixMuxMinClock =  67500;
	 }
	 else if (S3_864_SERIES(s3ChipId)) {
	    nonMuxMaxClock =  95000; /* 864 DCLK limit */
	    pixMuxMinClock =  67500;
	 }
	 else if (S3_805_I_SERIES(s3ChipId)) {
	    nonMuxMaxClock = 80000;  /* XXXX just a guess, who has 805i docs? */
	    pixMuxMinClock = 67500;
	 }
	 else {
	    nonMuxMaxClock = 67500;
	    pixMuxMinClock = 67500;
	 }
      }
      else if (DAC_IS_TRIO) {
	 if (OFLG_ISSET(OPTION_TRIO64VP_BUG3, &s3InfoRec.options)) {
	    nonMuxMaxClock = 0;   /* need CR67=10 and DCLK/2 all the time */
	    pixMuxMinClock = 0;   /* to avoid problems with blank signal */
	 }
	 else {
	    nonMuxMaxClock = 80000;
	    pixMuxMinClock = 80000;
	 }
      }
      else {
	 nonMuxMaxClock = 67500;
	 pixMuxMinClock = 67500;
      }
      allowPixMuxInterlace = TRUE;
      allowPixMuxSwitching = TRUE;
      pixMuxLimitedWidths = FALSE;
      pixMuxMinWidth = 0;
   } else if (s3Bt485PixMux) {
      /* XXXX Are the defaults for the other parameters correct? */
      pixMuxPossible = TRUE;
      allowPixMuxInterlace = FALSE;	/* It doesn't work right (yet) */
      allowPixMuxSwitching = FALSE;	/* XXXX Is this right? */
      if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
          S3_928_ONLY(s3ChipId)) {
	 nonMuxMaxClock = 67500;	/* Doubling only works in mux mode */
	 nonMuxMaxMemory = 1024;	/* Can't access more without mux */
	 allowPixMuxSwitching = FALSE;
	 pixMuxLimitedWidths = FALSE;
	 pixMuxMinWidth = 1024;
	 if (s3Bpp == 2) {
	    nonMuxMaxMemory = 0;	/* Only 2:1MUX works (yet)!     */
	    pixMuxMinWidth = 800;
	 } else if (s3Bpp==4) {
	    nonMuxMaxMemory = 0;
	    pixMuxMinWidth = 640;
	 }
      } else if (OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options)) {
	 nonMuxMaxClock = 67500;
	 allowPixMuxSwitching = TRUE;
	 pixMuxLimitedWidths = TRUE;
	 pixMuxMinWidth = 800;
      } else if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options)) {
	allowPixMuxSwitching = TRUE;
	pixMuxLimitedWidths = TRUE;
	/* For 8bpp mode, allow PIXMUX selection based on Clock and Width. */
	if (s3Bpp == 1) {
	  nonMuxMaxClock = 85000;
	  pixMuxMinWidth = 1024;
	} else {
	  /* For 16bpp and 32bpp modes, require PIXMUX. */
	  nonMuxMaxClock = 0;
	  pixMuxMinWidth = 0;
	}
      }else if (OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) {
	allowPixMuxSwitching = FALSE;
	pixMuxLimitedWidths = TRUE;
 	/* For 8bpp mode, allow PIXMUX selection based on Clock and Width. */
 	if (s3Bpp == 1) {
 	  nonMuxMaxClock = 85000;
 	  pixMuxMinWidth = 1024;
 	} else {
	  /* For 16bpp and 32bpp modes, require PIXMUX. */
	  nonMuxMaxClock = 0;
	  pixMuxMinWidth = 0;
 	}
      } else if (S3_964_SERIES(s3ChipId)) {
         nonMuxMaxClock = 0;  /* 964 can only be in pixmux mode when */
         pixMuxMinWidth = 0;  /* working in enhanced mode */  
	 pixMuxLimitedWidths = FALSE;
      } else {
	 nonMuxMaxClock = 85000;
      }
   }

   /*
    * clock options are now done after the ramdacs because the next
    * generation ramdacs will have a built in clock (i.e. TI 3025)
    */

   /* Diamond Stealth 64 VRAM uses an ICD2061A */
   if (DAC_IS_BT485_SERIES &&
       !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions) &&
       (s3BiosVendor == DIAMOND_BIOS) && S3_964_ONLY(s3ChipId)) {
      OFLG_SET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
      clockchip_probed = XCONFIG_PROBED;
   }

   if (DAC_IS_TI3025 && 
       !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      OFLG_SET(CLOCK_OPTION_TI3025, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
      clockchip_probed = XCONFIG_PROBED;
   }

   if ((DAC_IS_TI3026 || DAC_IS_TI3030) && 
       !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      OFLG_SET(CLOCK_OPTION_TI3026, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
      clockchip_probed = XCONFIG_PROBED;
   }

   if (DAC_IS_IBMRGB && 
       !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      OFLG_SET(CLOCK_OPTION_IBMRGB, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
      clockchip_probed = XCONFIG_PROBED;
   }

   if (DAC_IS_ATT20C409 && 
       !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      OFLG_SET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
      clockchip_probed = XCONFIG_PROBED;
   }

   if (DAC_IS_TI3026 || DAC_IS_TI3030) {
      int mclk, m, n, p;
      s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x00);
      n = s3InTi3026IndReg(TI_MCLK_PLL_DATA) & 0x3f;
      s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x04);
      m = s3InTi3026IndReg(TI_MCLK_PLL_DATA) & 0x3f;
      s3OutTi3026IndReg(TI_PLL_CONTROL, 0x00, 0x08);
      p = s3InTi3026IndReg(TI_MCLK_PLL_DATA) & 0x03;
      mclk = ((1431818 * ((65-m) * 8)) / (65-n) / (1 << p) + 50) / 100;
      if (xf86Verbose)
	 ErrorF("%s %s: Using Ti3026/3030 programmable MCLK (MCLK %1.3f MHz)\n",
		clockchip_probed, s3InfoRec.name, mclk / 1000.0);
      numClocks = 3;
      if (!s3InfoRec.s3MClk)
	 s3InfoRec.s3MClk = mclk;
   }
   if (OFLG_ISSET(CLOCK_OPTION_TI3026, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = ti3026ClockSelect;
      OFLG_SET(CLOCK_OPTION_TI3026, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);
      if (xf86Verbose)
	 ErrorF("%s %s: Using Ti3026/3030 programmable DCLK\n",
		clockchip_probed, s3InfoRec.name);
   } else if (OFLG_ISSET(CLOCK_OPTION_TI3025, &s3InfoRec.clockOptions)) {
      int mclk, m, n, p, mcc, cr5c;
      s3ClockSelectFunc = ti3025ClockSelect;
      OFLG_SET(CLOCK_OPTION_TI3025, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);

      outb(vgaCRIndex, 0x5c);
      cr5c = inb(vgaCRReg);
      outb(vgaCRReg, cr5c & 0xdf);           /* clear RS4 - use 3020 mode */

      s3OutTiIndReg(TI_PLL_CONTROL, 0x00, 0x00);
      n = s3InTiIndReg(TI_MCLK_PLL_DATA) & 0x7f;
      s3OutTiIndReg(TI_PLL_CONTROL, 0x00, 0x01);
      m = s3InTiIndReg(TI_MCLK_PLL_DATA) & 0x7f;
      s3OutTiIndReg(TI_PLL_CONTROL, 0x00, 0x02);
      p = s3InTiIndReg(TI_MCLK_PLL_DATA) & 0x03;
      mcc = s3InTiIndReg(TI_MCLK_DCLK_CONTROL);
      if (mcc & 0x08) 
	 mcc = (mcc & 0x07) * 2 + 2;
      else 
	 mcc = 1;
      mclk = ((1431818 * ((m+2) * 8)) / (n+2) / (1 << p) / mcc + 50) / 100;
      if (xf86Verbose)
	 ErrorF("%s %s: Using TI 3025 programmable clock (MCLK %1.3f MHz)\n",
		clockchip_probed, s3InfoRec.name, mclk / 1000.0);
      numClocks = 3;
      if (OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options)) {
	 mclk = 55000;
	 if (xf86Verbose)
	    ErrorF("%s %s: Setting MCLK to %1.3f MHz for #9GXE64 Pro\n",
		   XCONFIG_PROBED, s3InfoRec.name, mclk / 1000.0);
	 Ti3025SetClock(2 * mclk, TI_MCLK_PLL_DATA, s3ProgramTi3025Clock);
	 mcc = s3InTiIndReg(TI_MCLK_DCLK_CONTROL);
	 s3OutTiIndReg(TI_MCLK_DCLK_CONTROL,0x00, (mcc & 0xf0) | 0x08);
      }
      if (!s3InfoRec.s3MClk)
	 s3InfoRec.s3MClk = mclk;
      outb(vgaCRIndex, 0x5c);
      outb(vgaCRReg, cr5c);
   } else if (OFLG_ISSET(CLOCK_OPTION_IBMRGB, &s3InfoRec.clockOptions)) {
      char *refclock_probed;
      int mclk=0, m, n, df;
      int m0,m1,n0,n1;
      double f0,f1,f,fdiff;
      s3ClockSelectFunc = IBMRGBClockSelect;
      OFLG_SET(CLOCK_OPTION_IBMRGB, &s3InfoRec.clockOptions);
      OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions);

      if (s3InfoRec.s3RefClk) 
	 refclock_probed = XCONFIG_GIVEN;
      else 
	 refclock_probed = XCONFIG_PROBED;

      if (s3InIBMRGBIndReg(IBMRGB_pll_ctrl1) & 1) {
	 m0 = s3InIBMRGBIndReg(IBMRGB_m0+0*2);
	 n0 = s3InIBMRGBIndReg(IBMRGB_n0+0*2) & 0x1f;
	 m1 = s3InIBMRGBIndReg(IBMRGB_m0+1*2);
	 n1 = s3InIBMRGBIndReg(IBMRGB_n0+1*2) & 0x1f;
      }
      else {
	 m0 = s3InIBMRGBIndReg(IBMRGB_f0+0);
	 m1 = s3InIBMRGBIndReg(IBMRGB_f0+1);
	 n0 = s3InIBMRGBIndReg(IBMRGB_pll_ref_div_fix) & 0x1f;
	 n1 = n0;
      }
      f0 = 25.175 / ((m0&0x3f)+65.0) * n0 * (8 >> (m0>>6));
      f1 = 28.322 / ((m1&0x3f)+65.0) * n1 * (8 >> (m1>>6));      
      if (f1>f0) fdiff = f1-f0;
      else       fdiff = f0-f1;


      /* refclock defaults should not depend on vendor options
       * but only on probed BIOS tags; it's too dangerous
       * when users play with options */

      if (find_bios_string(s3InfoRec.BIOSbase,"VideoBlitz III AV",
			   "Genoa Systems Corporation") != NULL) {
         if (s3BiosVendor == UNKNOWN_BIOS) 
	    s3BiosVendor = GENOA_BIOS;
	 if (xf86Verbose)
	    ErrorF("%s %s: Genoa VideoBlitz III AV BIOS found\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	 if (!s3InfoRec.s3RefClk) 
	    s3InfoRec.s3RefClk = 50000;
      }
      else if (find_bios_string(s3InfoRec.BIOSbase,"STB Systems, Inc.", NULL) 
	       != NULL) {
	 if (s3BiosVendor == UNKNOWN_BIOS) 
	    s3BiosVendor = STB_BIOS;
	 if (xf86Verbose)
	    ErrorF("%s %s: STB Velocity 64 BIOS found\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	 if (!s3InfoRec.s3RefClk)
	    s3InfoRec.s3RefClk = 24000;
      }
      else if (find_bios_string(s3InfoRec.BIOSbase,
				"Number Nine Visual Technology","Motion 771")
	       != NULL) {
	 if (s3BiosVendor == UNKNOWN_BIOS) 
	    s3BiosVendor = NUMBER_NINE_BIOS;
	 if (xf86Verbose)
	    ErrorF("%s %s: #9 Motion 771 BIOS found\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	 if (!s3InfoRec.s3RefClk)
	    s3InfoRec.s3RefClk = 16000;
      }
      else if (find_bios_string(s3InfoRec.BIOSbase,
				"Hercules Graphite Terminator",NULL) != NULL) {
	 if (s3BiosVendor == UNKNOWN_BIOS) 
	    s3BiosVendor = HERCULES_BIOS;
	 if (xf86Verbose)
	    ErrorF("%s %s: Hercules Graphite Terminator BIOS found\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	 if (!s3InfoRec.s3RefClk)
	    if (S3_968_SERIES(s3ChipId))
	       /* Hercules Graphite Terminator Pro(tm) BIOS. */
	       s3InfoRec.s3RefClk = 16000;
	    else		/* S3 964 */
	       /* Hercules Graphite Terminator 64(tm) BIOS. */
	       s3InfoRec.s3RefClk = 50000;
      }
      else if (!s3InfoRec.s3RefClk) {
	 if (fdiff < f0*0.02) {
	    /* f = (f0+f1)/2; */ 
	    f = f1;		/* 28.322 MHz clock seems to be more acurate then 25.175 */
	    /* try to match some known reclock values */
	    if ((int)(f*1e3/200+0.5) == 16000/200)
	       s3InfoRec.s3RefClk = 16000;
	    else if ((int)(f*1e3/200+0.5) == 50000/200)
	       s3InfoRec.s3RefClk = 50000;
	    else if ((int)(f*1e3/200+0.5) == 24000/200)
	       s3InfoRec.s3RefClk = 24000;
	    else if ((int)(f*1e3/200+0.5) == 14318/200)
	       s3InfoRec.s3RefClk = 14318;
	    else 
	       s3InfoRec.s3RefClk = (int)(f*2+0.5)*500;
	 }
      }
      else if (!s3InfoRec.s3RefClk) {
	 s3InfoRec.s3RefClk = 16000; /* default */
      }
      
      if (!DAC_IS_IBMRGB525) {
	 m = s3InIBMRGBIndReg(IBMRGB_sysclk_vco_div);
	 n = s3InIBMRGBIndReg(IBMRGB_sysclk_ref_div) & 0x1f;
	 df = m>>6;
	 m &= 0x3f;
	 if (!n) { m=0; n=1; }
	 mclk = ((s3InfoRec.s3RefClk*100 * (m+65)) / n / (8 >> df) + 50) / 100;
      }
      if (xf86Verbose) {
	 ErrorF("%s %s: Using IBM RGB52x programmable clock",
		clockchip_probed, s3InfoRec.name);
	 if (mclk)
	    ErrorF(" (MCLK %1.3f MHz)", mclk / 1000.0);
	 ErrorF("\n");	
	 ErrorF("%s %s: with refclock %1.3f MHz (probed %1.3f & %1.3f)\n",
		refclock_probed,s3InfoRec.name,s3InfoRec.s3RefClk/1e3,f0,f1);
      }
      numClocks = 3;
      if (!s3InfoRec.s3MClk)
	 s3InfoRec.s3MClk = mclk;
   } else if (OFLG_ISSET(OPTION_LEGEND, &s3InfoRec.options)) {
      s3ClockSelectFunc = LegendClockSelect;
      numClocks = 32;
   } else if (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = icd2061ClockSelect;
      if (xf86Verbose)
         ErrorF("%s %s: Using ICD2061A programmable clock\n",
		clockchip_probed, s3InfoRec.name);
      numClocks = 3;
   } else if (OFLG_ISSET(CLOCK_OPTION_GLORIA8, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = Gloria8ClockSelect;
      if (xf86Verbose)
         ErrorF("%s %s: Using ELSA Gloria-8 ICS9161/TVP3030 programmable clock\n",
		clockchip_probed, s3InfoRec.name);
      numClocks = 3;
   } else if (OFLG_ISSET(CLOCK_OPTION_SC11412, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = icd2061ClockSelect;
      if (xf86Verbose)
	 ErrorF("%s %s: Using Sierra SC11412 programmable clock\n",
		clockchip_probed, s3InfoRec.name);
      numClocks = 3;
   } else if (OFLG_ISSET(CLOCK_OPTION_S3GENDAC, &s3InfoRec.clockOptions) ||
	      OFLG_ISSET(CLOCK_OPTION_ICS5342,  &s3InfoRec.clockOptions)) {
      unsigned char saveCR55;
      int m,n,n1,n2, mclk;
      
      s3ClockSelectFunc = s3GendacClockSelect;
      
      outb(vgaCRIndex, 0x55);
      saveCR55 = inb(vgaCRReg);
      outb(vgaCRReg, saveCR55 | 1);
      
      outb(0x3C7, 10); /* read MCLK */
      m = inb(0x3C9);
      n = inb(0x3C9);
      
      outb(vgaCRIndex, 0x55);
      outb(vgaCRReg, saveCR55);	 
      
      m &= 0x7f;
      n1 = n & 0x1f;
      n2 = (n>>5) & 0x03;
      mclk = ((1431818 * (m+2)) / (n1+2) / (1 << n2) + 50) / 100;

      if (xf86Verbose)
	 ErrorF("%s %s: Using %s programmable clock (MCLK %1.3f MHz)\n"
		,clockchip_probed, s3InfoRec.name
		,OFLG_ISSET(CLOCK_OPTION_ICS5342, &s3InfoRec.clockOptions)
		? "ICS5342" : "S3 Gendac/SDAC"
		,mclk / 1000.0);
      if (s3InfoRec.s3MClk > 0) {
	 if (xf86Verbose)
	    ErrorF("%s %s: using specified MCLK value of %1.3f MHz for DRAM timings\n",
		   XCONFIG_GIVEN, s3InfoRec.name, s3InfoRec.s3MClk / 1000.0);
      }
      else {
	 s3InfoRec.s3MClk = mclk;
      }
      numClocks = 3;
   } else if (OFLG_ISSET(CLOCK_OPTION_S3TRIO, &s3InfoRec.clockOptions)) {
      unsigned char sr8;
      int m,n,n1,n2, mclk;

      s3ClockSelectFunc = s3GendacClockSelect;
      numClocks = 3;
      
      outb(0x3c4, 0x08);
      sr8 = inb(0x3c5);
      outb(0x3c5, 0x06);
      
      outb(0x3c4, 0x11);
      m = inb(0x3c5);
      outb(0x3c4, 0x10);
      n = inb(0x3c5);
      
      outb(0x3c4, 0x08);
      outb(0x3c5, sr8);
      
      m &= 0x7f;
      n1 = n & 0x1f;
      n2 = (n>>5) & 0x03;
      mclk = ((1431818 * (m+2)) / (n1+2) / (1 << n2) + 50) / 100;
      if (xf86Verbose)
	 ErrorF("%s %s: Using Trio32/64 programmable clock (MCLK %1.3f MHz)\n"
		,clockchip_probed, s3InfoRec.name
		,mclk / 1000.0);
      if (s3InfoRec.s3MClk > 0) {
	 if (xf86Verbose)
	    ErrorF("%s %s: using specified MCLK value of %1.3f MHz for DRAM timings\n",
		   XCONFIG_GIVEN, s3InfoRec.name, s3InfoRec.s3MClk / 1000.0);
      }
      else {
	 s3InfoRec.s3MClk = mclk;
      }
   } else if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = icd2061ClockSelect;
      if (xf86Verbose)
	 ErrorF("%s %s: Using ICS2595 programmable clock\n",
		XCONFIG_GIVEN, s3InfoRec.name);
      numClocks = 3;
   } else if (OFLG_ISSET(CLOCK_OPTION_CH8391, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = ch8391ClockSelect;
      if (xf86Verbose)
	 ErrorF("%s %s: Using Chrontel 8391 programmable clock\n",
		XCONFIG_GIVEN, s3InfoRec.name);
      numClocks = 3;
   } else if (OFLG_ISSET(CLOCK_OPTION_STG1703, &s3InfoRec.clockOptions)) {
      unsigned char mi, ml, mh;
      int mclk;
      
      outb(vgaCRIndex, 0x43);
      tmp = inb(vgaCRReg);
      outb(vgaCRReg, tmp & ~0x02);
      
      outb(vgaCRIndex, 0x55);
      tmp = inb(vgaCRReg) & ~0x03;
      outb(vgaCRReg, tmp | 1);  /* set RS2 */
      
      outb(0x3c7, 0x00);  /* index high */
      outb(0x3c8, 0x48);  /* index low  */
      mi = (inb(0x3c9) >> 4) & 0x03;
      
      outb(0x3c8, 0x40 + 2*mi);  /* index low  */
      ml = inb(0x3c9);
      mh = inb(0x3c9);
      
#ifdef DEBUG
      for (i=0; i<0x50; i++) {
	 if (i%16 == 0) ErrorF ("\t%04x",i);
	 if (i% 8 == 0) ErrorF (" ");
	 if (i% 4 == 0) ErrorF (" ");
	 outb(0x3c8, i);
	 ErrorF(" %02x",inb(0x3c9));
	 if (i%16 == 15) ErrorF ("\n");
      }
#endif
      
      outb(vgaCRReg, tmp);  /* reset RS2 */
      
      mclk = ((((1431818 * ((ml&0x7f) + 2)) / ((mh&0x1f) + 2)) 
	       >> ((mh>>5)&0x03)) + 50) / 100;
      
      s3ClockSelectFunc = STG1703ClockSelect;
      numClocks = 3;

      if (xf86Verbose)
	 ErrorF("%s %s: Using STG1703 programmable clock(MCLK%d %02x %02x %1.3f MHz)\n",
		XCONFIG_GIVEN, s3InfoRec.name, mi, ml,mh, mclk/1e3);
      if (s3InfoRec.s3MClk > 0) {
	 if (xf86Verbose)
	    ErrorF("%s %s: using specified MCLK value of %1.3f MHz for DRAM timings\n",
		   XCONFIG_GIVEN, s3InfoRec.name, s3InfoRec.s3MClk / 1000.0);
      }
      else
	 s3InfoRec.s3MClk = mclk;
      
   } else if (OFLG_ISSET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions)) {
      s3ClockSelectFunc = att409ClockSelect;
      if (xf86Verbose)
	 ErrorF("%s %s: Using ATT20C409/ATT20C499 programmable clock\n",
		clockchip_probed, s3InfoRec.name);
      numClocks = 3;
   } else {
      s3ClockSelectFunc = s3ClockSelect;
      numClocks = 16;
      if (!s3InfoRec.clocks) 
         vgaGetClocks(numClocks, s3ClockSelectFunc);
   }

   /*
    * Set the maximum raw clock for programmable clock chips.
    * Setting maxRawClock to 0 means no clock-chip limit imposed.
    */
   if (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      if (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
	 maxRawClock = 120000;
      } else if (OFLG_ISSET(CLOCK_OPTION_SC11412, &s3InfoRec.clockOptions)) {
	 if (!OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options)) {
	    switch (s3RamdacType) {
	    case BT485_DAC:
	       maxRawClock = 67500;
	       break;
	    case ATT20C505_DAC:
	       maxRawClock = 90000;
	       break;
	    default:
	       maxRawClock = 100000;
	       break;
	    }
	 } else {
	    maxRawClock = 100000;
	 }
      } else if (OFLG_ISSET(CLOCK_OPTION_S3GENDAC, &s3InfoRec.clockOptions)) {
	 maxRawClock = 110000;
      } else if (OFLG_ISSET(CLOCK_OPTION_ICS5342, &s3InfoRec.clockOptions)) {
	 maxRawClock = 110000;
      } else if (OFLG_ISSET(CLOCK_OPTION_TI3025, &s3InfoRec.clockOptions)) {
	 maxRawClock = s3InfoRec.dacSpeed; /* Is this right?? */
      } else if (OFLG_ISSET(CLOCK_OPTION_TI3026, &s3InfoRec.clockOptions)) {
	 maxRawClock = s3InfoRec.dacSpeed; /* Is this right?? */
      } else if (OFLG_ISSET(CLOCK_OPTION_IBMRGB, &s3InfoRec.clockOptions)) {
	 maxRawClock = s3InfoRec.dacSpeed; /* Is this right?? */
      } else if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions)) {
	 maxRawClock = 145000; /* This is what is in common_hw/ICS2595.h */
      } else if (OFLG_ISSET(CLOCK_OPTION_CH8391, &s3InfoRec.clockOptions)) {
	 maxRawClock = 135000;
      } else if (OFLG_ISSET(CLOCK_OPTION_STG1703, &s3InfoRec.clockOptions)) {
	 maxRawClock = 135000;
      } else if (OFLG_ISSET(CLOCK_OPTION_S3TRIO, &s3InfoRec.clockOptions)) {
	 maxRawClock = 135000;
      } else if (OFLG_ISSET(CLOCK_OPTION_ATT409, &s3InfoRec.clockOptions)) {
	 maxRawClock = s3InfoRec.dacSpeed; /* Is this right?? */
      } else {
	 /* Shouldn't get here */
	 maxRawClock = 0;
      }
   } else {
      maxRawClock = 0;
   }

   /*
    * Set pixel clock limit based on RAMDAC type/speed/bpp and pixmux usage.
    * Also scale maxRawClock so that it can be compared with a pixel clock,
    * and re-adjust the pixel clock limit if required.
    */

   switch (s3RamdacType) {
   case BT485_DAC:
      if (maxRawClock > 67500)
	 clockDoublingPossible = TRUE;
      /* These limits are based on the LCLK rating, and may be too high */
      if (s3Bt485PixMux && s3Bpp < 4)
	 s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      else {
	 if (s3InfoRec.dacSpeed < 150000)    /* 110 and 135 */
	    s3InfoRec.maxClock = 90000;
	 else				      /* 150 and 170 (if they exist) */
	    s3InfoRec.maxClock = 110000;
      }
      break;
   case ATT20C505_DAC:
      if (maxRawClock > 90000)
	 clockDoublingPossible = TRUE;
      /* These limits are based on the LCLK rating, and may be too high */
      if (s3Bt485PixMux && s3Bpp < 4)
	 s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      else {
	 if (s3InfoRec.dacSpeed < 110000)	  /* 85 */
	    s3InfoRec.maxClock = 85000;
	 else if (s3InfoRec.dacSpeed < 135000)	  /* 110 */
	    s3InfoRec.maxClock = 90000;
	 else					  /* 135, 150, 170 */
	    s3InfoRec.maxClock = 110000;
      }
      break;
   case ATT20C498_DAC:
   case ATT22C498_DAC:
   case ATT20C409_DAC:
   case STG1700_DAC:
   case STG1703_DAC:
   case S3_SDAC_DAC:
      if (s3ATT498PixMux) {
	 s3InfoRec.maxClock = s3InfoRec.dacSpeed;
	 if (s3Bpp == 1)	/* XXXX is this right?? */
	    clockDoublingPossible = TRUE;
      }
      else {
	 if (s3InfoRec.dacSpeed >= 135000) /* 20C498 -13, -15, -17 */
	    s3InfoRec.maxClock = 110000;
	 else				   /* 20C498 -11 */
	    s3InfoRec.maxClock = 80000;
	 /* Halve it for 32bpp */
	 if (s3Bpp == 4) {
	    s3InfoRec.maxClock /= 2;
	    maxRawClock /= 2;
	 }
      }
      break;
   case S3_TRIO32_DAC:
   case S3_TRIO64_DAC:
      if (s3ATT498PixMux)
	 s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      else if (s3Bpp < 4)
	 s3InfoRec.maxClock = 80000;
      else
	 s3InfoRec.maxClock = 50000;
      break;
   case TI3020_DAC:
      clockDoublingPossible = TRUE;
      s3InfoRec.maxClock = s3InfoRec.dacSpeed; /* looks like the same limit */
      break;                                   /* for all bpp's... */
   case TI3025_DAC:
      clockDoublingPossible = TRUE;
      s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      break;
   case TI3026_DAC:
   case TI3030_DAC:
      clockDoublingPossible = TRUE;
      s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      break;
   case IBMRGB524_DAC:
   case IBMRGB525_DAC:
   case IBMRGB528_DAC:
      clockDoublingPossible = FALSE;
      /* LCLK & SCLK limit is 100 MHz */
      if ((s3InfoRec.dacSpeed * s3Bpp) / 8 > 100000)  
	 s3InfoRec.maxClock = (100000 * 8) / s3Bpp; 
      else
	 s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      break;
      /* XXXX What happens for 16bpp and 32bpp?? */
      /* XXXX Include scaling of maxRawClock for 16bpp and 32bpp */
   case ATT20C490_DAC:
   case SS2410_DAC:	/* ?? Another GUESS! ( based on the 490)  */
   case SC1148x_M2_DAC:
   case SC1148x_M3_DAC:
      s3InfoRec.maxClock = s3InfoRec.dacSpeed;
      /* Halve it for 16bpp (32bpp not supported) */
      if (s3Bpp > 1) {
	 s3InfoRec.maxClock /= 2;
	 maxRawClock /= 2;
      }
      break;
   case S3_GENDAC_DAC:
      s3InfoRec.maxClock = s3InfoRec.dacSpeed / s3Bpp;
      break;
   case SC15025_DAC:
      {
	 int doubleEdgeLimit;
	 if (s3InfoRec.dacSpeed >= 125000)	/* -125 */
	    doubleEdgeLimit = 85000;
	 else if (s3InfoRec.dacSpeed >= 110000)	/* -110 */
	    doubleEdgeLimit = 65000;
	 else					/* -80, -66 */
	    doubleEdgeLimit = 50000;
	 switch (s3Bpp) {
	 case 1:
	    s3InfoRec.maxClock = s3InfoRec.dacSpeed;
	    break;
	 case 2:
	    s3InfoRec.maxClock = doubleEdgeLimit;
	    maxRawClock /= 2;
	    break;
	 case 4:
	    s3InfoRec.maxClock = doubleEdgeLimit / 2;
	    maxRawClock /= 4;
	    break;
	 }
      }
      break;
   default:
      /* For DACs we don't have special code for, keep this as a limit */
      s3InfoRec.maxClock = s3MaxClock;
   }
   /* Check that maxClock is not higher than dacSpeed */
   if (s3InfoRec.maxClock > s3InfoRec.dacSpeed)
      s3InfoRec.maxClock = s3InfoRec.dacSpeed;

   /* Check if this exceeds the clock chip's limit */
   if (clockDoublingPossible)
      maxRawClock *= 2;
   if (maxRawClock > 0 && s3InfoRec.maxClock > maxRawClock)
      s3InfoRec.maxClock = maxRawClock;

   /* check DCLK limit of 100MHz for 866/868 */
   if (S3_866_SERIES(s3ChipId) || S3_868_SERIES(s3ChipId)) {
      if (((s3Bpp==1 && !pixMuxPossible) || s3Bpp==2) 
	  && s3InfoRec.maxClock > 100000)
	 s3InfoRec.maxClock = 100000;
      else if (s3Bpp>2 && s3InfoRec.maxClock > 50000)
	 s3InfoRec.maxClock = 50000;  
   }
   /* check DCLK limit of 95MHz for 864 */
   else if (S3_864_SERIES(s3ChipId)) {
      if (((s3Bpp==1 && !pixMuxPossible) || s3Bpp==2) 
	  && s3InfoRec.maxClock > 95000)
	 s3InfoRec.maxClock = 95000;

      /* for 24bpp the limit should be 95/2 == 47.5MHz
	 but I set the limit to 50MHz to allow VESA 800x600@72Hz */
      else if (s3Bpp>2 && s3InfoRec.maxClock > 50000)
	 s3InfoRec.maxClock = 50000;  
   }

   if (xf86Verbose) {
      if (! OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
	 for (j = 0; j < s3InfoRec.clocks; j++) {
	    if ((j % 8) == 0) {
	       if (j != 0)
		  ErrorF("\n");
               ErrorF("%s %s: clocks:",
                OFLG_ISSET(XCONFIG_CLOCKS,&s3InfoRec.xconfigFlag) ?
                    XCONFIG_GIVEN : XCONFIG_PROBED , 
                s3InfoRec.name);
	    }
	    ErrorF(" %6.2f", (double)s3InfoRec.clock[j] / 1000.0);
         }
         ErrorF("\n");
      } 
   }

   if (pixMuxPossible && s3InfoRec.videoRam > nonMuxMaxMemory)
      pixMuxNeeded = TRUE;

   /* Adjust s3InfoRec.clock[] when not using a programable clock chip */

   if (!OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      Bool clocksChanged = FALSE;
      Bool numClocksChanged = FALSE;
      int newNumClocks = s3InfoRec.clocks;

      if (S3_864_SERIES(s3ChipId))
	 nonMuxMaxClock = 95000;
      else if (S3_805_I_SERIES(s3ChipId))
	 nonMuxMaxClock = 90000;  /* XXXX just a guess, who has 805i docs? */

      for (j = 0; j < s3InfoRec.clocks; j++) {
	 switch(s3RamdacType) {
	 case NORMAL_DAC:
	    /* only suports 8bpp -- nothing to do */
	    break;
	 case BT485_DAC:
	 case ATT20C505_DAC:
	    /* XXXX What happens here for 16bpp/32bpp ? */
	    break;
	 case TI3020_DAC:
	    switch (s3Bpp) {
	    case 1:
	       break;
	    case 2:
	       s3InfoRec.clock[j] /= 2;
	       clocksChanged = TRUE;
	       break;
	    case 4:
	       s3InfoRec.clock[j] /= 4;
	       clocksChanged = TRUE;
	       break;
	    }
	    break;
	 case ATT20C498_DAC:
	 case ATT22C498_DAC:
	 case STG1700_DAC:
	 case STG1703_DAC:	/* XXXX should this be here? */
	    switch (s3Bpp) {
	    case 1:
	       if (!numClocksChanged) {
		  newNumClocks = 32;
		  numClocksChanged = TRUE;
		  clocksChanged = TRUE;
		  for(i = s3InfoRec.clocks; i < newNumClocks; i++)
		     s3InfoRec.clock[i] = 0;  /* XXXX is clock[] initialized? */
		  if (s3InfoRec.clocks > 16) 
		     s3InfoRec.clocks = 16;
	       }
	       if (s3InfoRec.clock[j] * 2 > pixMuxMinClock &&
		   s3InfoRec.clock[j] * 2 <= s3InfoRec.dacSpeed)
		  s3InfoRec.clock[j + 16] = s3InfoRec.clock[j] * 2;
	       else
		  s3InfoRec.clock[j + 16] = 0;
	       if (s3InfoRec.clock[j] > nonMuxMaxClock)
		  s3InfoRec.clock[j] = 0;
	       break;
	    case 2:
	       /* No change for 16bpp */
	       break;
	    case 4:
	       s3InfoRec.clock[j] /= 2;
	       clocksChanged = TRUE;
	       break;
	    }
	    break;
	 case ATT20C490_DAC:
	 case SS2410_DAC:	/* GUESSING!! (based on 490)  */
	 case SC1148x_M2_DAC:
	 case SC1148x_M3_DAC:
	    if (s3Bpp > 1) {
	       s3InfoRec.clock[j] /= s3Bpp;
	       clocksChanged = TRUE;
	    }
	    break;
	 case SC15025_DAC:
	    if (s3Bpp > 1) {
	       s3InfoRec.clock[j] /= s3Bpp;
	       clocksChanged = TRUE;
	    }
	    break;
	 case TI3025_DAC:
	 case TI3026_DAC:
	 case TI3030_DAC:
	 case IBMRGB524_DAC:
	 case IBMRGB525_DAC:
	 case IBMRGB528_DAC:
	 case S3_SDAC_DAC:
	 case S3_GENDAC_DAC:
	 case S3_TRIO32_DAC:
	 case S3_TRIO64_DAC:
	    /*
	     * We should never get here since these have a programmable
	     * clock built in.
	     */
	    break;
	 default:
	    /* Do nothing */
	    break;
	 }
      }
      if (numClocksChanged)
	 s3InfoRec.clocks = newNumClocks;

      if (xf86Verbose && clocksChanged) {
	 ErrorF("%s %s: Effective pixel clocks available for depth %d:\n",
		XCONFIG_PROBED, s3InfoRec.name, s3InfoRec.depth);
	 for (j = 0; j < s3InfoRec.clocks; j++) {
	    if ((j % 8) == 0) {
	       if (j != 0)
		  ErrorF("\n");
               ErrorF("%s %s: pixel clocks:", XCONFIG_PROBED, s3InfoRec.name);
	    }
	    ErrorF(" %6.2f", (double)s3InfoRec.clock[j] / 1000.0);
         }
         ErrorF("\n");
      }
   }

   /* At this point, the s3InfoRec.clock[] values are pixel clocks */

   if (S3_911_SERIES(s3ChipId)) {
      maxDisplayWidth = 1024;
      maxDisplayHeight = 1024 - 1; /* Cursor takes exactly 1 line for 911 */
   } else if ((OFLG_ISSET(OPTION_BT485_CURS, &s3InfoRec.options) &&
	       DAC_IS_BT485_SERIES) ||
	      (OFLG_ISSET(OPTION_TI3020_CURS, &s3InfoRec.options) &&
	       DAC_IS_TI3020_SERIES) ||
	      (OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options) &&
	       (DAC_IS_TI3026 || DAC_IS_TI3030)) ||
	      (OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options) &&
	       DAC_IS_IBMRGB) ||
	      OFLG_ISSET(OPTION_SW_CURSOR, &s3InfoRec.options)) {
      maxDisplayWidth = 2048;
      maxDisplayHeight = 4096;
   } else {
      maxDisplayWidth = 2048;
      maxDisplayHeight = 4096 - 3; /* Cursor can take up to 3 lines */
   }

   if (s3InfoRec.virtualX > maxDisplayWidth) {
      ErrorF("%s: Virtual width (%d) is too large.  Maximum is %d\n",
	     s3InfoRec.name, s3InfoRec.virtualX, maxDisplayWidth);
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return (FALSE);
   }
   if (s3InfoRec.virtualY > maxDisplayHeight) {
      ErrorF("%s: Virtual height (%d) is too large.  Maximum is %d\n",
	     s3InfoRec.name, s3InfoRec.virtualY, maxDisplayHeight);
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return (FALSE);
   }
 
   lookupFlags = LOOKUP_DEFAULT;

redo_mode_lookup:

   tx = s3InfoRec.virtualX;
   ty = s3InfoRec.virtualY;
   pMode = s3InfoRec.modes;
   if (pMode == NULL) {
      ErrorF("No modes supplied in XF86Config\n");
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return (FALSE);
   }
   pEnd = NULL;
   do {
      DisplayModePtr pModeSv;

      pModeSv = pMode->next;
      /*
       * xf86LookupMode returns FALSE if it ran into an invalid
       * parameter 
       */
      if (!xf86LookupMode(pMode, &s3InfoRec, lookupFlags)) {
	 xf86DeleteMode(&s3InfoRec, pMode);
      } else if (pMode->HDisplay > maxDisplayWidth) {
	 ErrorF("%s %s: Width of mode \"%s\" is too large (max is %d)\n",
		XCONFIG_PROBED, s3InfoRec.name, pMode->name, maxDisplayWidth);
	 xf86DeleteMode(&s3InfoRec, pMode);
      } else if (pMode->VDisplay > maxDisplayHeight) {
	 ErrorF("%s %s: Height of mode \"%s\" is too large (max is %d)\n",
		XCONFIG_PROBED, s3InfoRec.name, pMode->name, maxDisplayHeight);
	 xf86DeleteMode(&s3InfoRec, pMode);
      } else if ((pMode->HDisplay * (1 + pMode->VDisplay) * s3Bpp) >
		 s3InfoRec.videoRam * 1024) {
	 ErrorF("%s %s: Too little memory for mode \"%s\"\n", XCONFIG_PROBED,
		s3InfoRec.name, pMode->name);
	 if (!OFLG_ISSET(OPTION_BT485_CURS,  &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_TI3020_CURS, &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_SW_CURSOR,   &s3InfoRec.options))
	 ErrorF("%s %s: NB. 1 scan line is required for the hardware cursor\n",
	        XCONFIG_PROBED, s3InfoRec.name);
	 xf86DeleteMode(&s3InfoRec, pMode);
      } else if (((tx > 0) && (pMode->HDisplay > tx)) ||
		 ((ty > 0) && (pMode->VDisplay > ty))) {
	 ErrorF("%s %s: Resolution %dx%d too large for virtual %dx%d\n",
		XCONFIG_PROBED, s3InfoRec.name,
		pMode->HDisplay, pMode->VDisplay, tx, ty);
	 xf86DeleteMode(&s3InfoRec, pMode);
      } else {
	 /*
	  * Successfully looked up this mode.  If pEnd isn't
	  * initialized, set it to this mode.
	  */
	 if (pEnd == (DisplayModePtr) NULL)
	    pEnd = pMode;

	 s3InfoRec.virtualX = max(s3InfoRec.virtualX, pMode->HDisplay);
	 s3InfoRec.virtualY = max(s3InfoRec.virtualY, pMode->VDisplay);

	 /*
	  * Check what impact each mode has on pixel multiplexing,
	  * and mark those modes for which pixmux must be used.
	  */
	 if (pixMuxPossible) {
	    if (s3Bpp == 1 && s3ATT498PixMux && !DAC_IS_SDAC &&
		(S3_864_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId))
		&& !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE,
			       &s3InfoRec.clockOptions)) {
	       if (pMode->Clock > 15) {
		  pMode->Flags |= V_PIXMUX;
		  pixMuxNeeded = TRUE;
	       }
	    }
	    else if (s3InfoRec.clock[pMode->Clock] > nonMuxMaxClock) {
	       pMode->Flags |= V_PIXMUX;
	       pixMuxNeeded = TRUE;
	    }
	    if (s3InfoRec.videoRam > nonMuxMaxMemory)
	       pMode->Flags |= V_PIXMUX;
	    /* XXXX this needs some changes */
	    if ((OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
		 OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) &&
		s3InfoRec.virtualX * s3InfoRec.virtualY > 2*1024*1024) {
	      /* PIXMUX must be used to access more than 2mb memory. */
	      pMode->Flags |= V_PIXMUX;
	      pixMuxNeeded = TRUE;
	    }

	    /*
	     * Check if pixmux can't be used.  There are two cases:
	     *
	     *   1. No switching between mux and non-mux modes.  In this case
	     *      the presence of any mode which can't be used in pixmux
	     *      mode is flagged.
	     *   2. Switching allowed.  In this cases the presence of modes
	     *      which require mux for one feature, but can't use it
	     *      because of another is flagged.
	     */
	    if (!allowPixMuxSwitching || (pMode->Flags & V_PIXMUX)) {
	       if (pMode->HDisplay < pixMuxMinWidth)
		  pixMuxWidthOK = FALSE;
	       if ((pMode->Flags & V_INTERLACE) && !allowPixMuxInterlace)
		  pixMuxInterlaceOK = FALSE;
	    }
	 }
      }
      pMode = pModeSv;
   } while (pMode != pEnd);

   if ((tx != s3InfoRec.virtualX) || (ty != s3InfoRec.virtualY))
      OFLG_CLR(XCONFIG_VIRTUAL,&s3InfoRec.xconfigFlag);

   /*
    * Are we using pixel multiplexing, or does the mode combination mean
    * we can't continue.  Note, this is a case we can't really deal with
    * by deleting modes -- there is no unique choice of modes to delete,
    * so let the user deal with it.
    */
   if (pixMuxPossible && pixMuxNeeded) {
      if (!pixMuxWidthOK) {
	 pMode = s3InfoRec.modes;
	 pEnd = NULL;
	 do {
	    DisplayModePtr pModeSv;

	    pModeSv = pMode->next;

	    if (pMode->HDisplay < pixMuxMinWidth) {
	       xf86DeleteMode(&s3InfoRec, pMode);
	    } else {
	       if (pEnd == (DisplayModePtr) NULL)
		  pEnd = pMode;
	    }

	    pMode = pModeSv;
	 } while (pMode != pEnd);
	 if (s3InfoRec.videoRam > nonMuxMaxMemory) {
	    ErrorF("%s %s: To access more than %dkB video memory the RAMDAC "
		   "must\n", XCONFIG_PROBED, s3InfoRec.name, nonMuxMaxMemory);
	    ErrorF("\toperate in pixel multiplex mode, but pixel "
		   "multiplexing\n");
	    ErrorF("\tcannot be used for modes of width less than %d.\n",
		   pixMuxMinWidth);
	    ErrorF("\tAdjust the Modes and/or VideoRam and Virtual lines in\n");
	    ErrorF("\tyour XF86Config to meet these requirements\n");
	 } else {
	    if (nonMuxMaxClock > 0) {
	       ErrorF("%s %s: Modes with a dot-clock above %dMHz require the "
		      "RAMDAC to\n", XCONFIG_PROBED, s3InfoRec.name,
		      nonMuxMaxClock / 1000);
	       ErrorF("\toperate in pixel multiplex mode, but pixel "
		      "multiplexing\n");
	       ErrorF("\tcannot be used for modes with width less than %d.\n",
		      pixMuxMinWidth);
	    } else {
	       ErrorF("%s %s: The RAMDAC must operate in pixel multiplex "
		      "mode,\n", XCONFIG_PROBED, s3InfoRec.name);
	       ErrorF("\tbut pixel multiplexing cannot be used for modes\n");
	       ErrorF("\twith width less than %d.\n", pixMuxMinWidth);
	    }
	    ErrorF("\tAdjust the Modes line in your XF86Config to meet these ");
	    ErrorF("\trequirements.\n");
	 }
      }
      if (!pixMuxInterlaceOK) {
	 if (s3InfoRec.videoRam > nonMuxMaxMemory) {
	    ErrorF("%s %s: To access more than %dkB video memory the RAMDAC "
		   "must\n", XCONFIG_PROBED, s3InfoRec.name, nonMuxMaxMemory);
	    ErrorF("\toperate in pixel multiplex mode, but pixel "
		   "multiplexing\n");
	    ErrorF("\tcannot be used for interlaced modes.\n");
#if 1
	    ErrorF("\tRescanning modes with interlaced modes elimintated.\n");
#else
	    ErrorF("Adjust the Modes and/or VideoRam and Virtual lines in\n");
	    ErrorF("your XF86Config to meet these requirements\n");
#endif
	 } else {
	    if (nonMuxMaxClock > 0) {
	       ErrorF("%s %s: Modes with a dot-clock above %dMHz require the "
		      "RAMDAC to\n", XCONFIG_PROBED, s3InfoRec.name,
		      nonMuxMaxClock / 1000);
	       ErrorF("\toperate in pixel multiplex mode, but pixel "
		      "multiplexing\n");
	       ErrorF("\tcannot be used for interlaced modes.\n");
	    } else {
	       ErrorF("%s %s: The RAMDAC must operate in pixel multiplex "
		      "mode,\n", XCONFIG_PROBED, s3InfoRec.name);
	       ErrorF("\tbut pixel multiplexing cannot be used for interlaced "
		      "modes.\n");
	    }
#if 1
	    ErrorF("\tRescanning modes with interlaced modes elimintated.\n");
#else
	    ErrorF("Adjust the Modes line in your XF86Config to meet these ");
	    ErrorF("requirements.\n");
#endif
	 }
	 pixMuxInterlaceOK = TRUE;
	 lookupFlags = LOOKUP_NO_INTERLACED;
	 goto redo_mode_lookup;
      }
#if 0
      if (!pixMuxWidthOK || !pixMuxInterlaceOK) {
	 xf86DisableIOPorts(s3InfoRec.scrnIndex);
	 return(FALSE);
      } else
#endif
      {
	 if (xf86Verbose)
	    ErrorF("%s %s: Operating RAMDAC in pixel multiplex mode\n",
		   XCONFIG_PROBED, s3InfoRec.name);
	 s3UsingPixMux = TRUE;
      }
   }

   /* pixmux on Bt485 requires use of Bt's cursor */
   if (((s3Bt485PixMux && s3UsingPixMux) ||
	OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
	OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) &&
       !OFLG_ISSET(OPTION_BT485_CURS, &s3InfoRec.options)) {
      OFLG_SET(OPTION_BT485_CURS, &s3InfoRec.options);
      ErrorF("%s %s: Using hardware cursor from Bt485/20C505 RAMDAC\n",
	     XCONFIG_PROBED, s3InfoRec.name);
   }

   if (s3UsingPixMux && !allowPixMuxSwitching) {
      /* Mark all modes as V_PIXMUX */
      pEnd = pMode = s3InfoRec.modes;
      do {
	 pMode->Flags |= V_PIXMUX;
         pMode = pMode->next;
      } while (pMode != pEnd);
   }

   /*
    * For programmable clocks, fill in the SynthClock value
    * and set V_DBLCLK as required for each mode
    */

   if (OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE, &s3InfoRec.clockOptions)) {
      /* First just copy the pixel values */
      pEnd = pMode = s3InfoRec.modes;
      do {
	 pMode->SynthClock = s3InfoRec.clock[pMode->Clock];
	 pMode = pMode->next;
      } while (pMode != pEnd);
      /* Now make adjustments */
      pEnd = pMode = s3InfoRec.modes;
      do {
	 switch(s3RamdacType) {
	 case NORMAL_DAC:
	    /* only suports 8bpp -- nothing to do */
	    break;
	 case BT485_DAC:
	    {
	       int c;

	       if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
		   OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options))
		  c = 85000;
	       else if (S3_964_SERIES(s3ChipId) && s3Bpp == 4)
		  c = 90000;
	       else
		  c = 67500;
	       if (pMode->SynthClock > c) {
		  pMode->SynthClock /= 2;
		  pMode->Flags |= V_DBLCLK;
	       }
	    }
	    break;
	 case ATT20C505_DAC:
	    if (pMode->SynthClock > 90000) {
	       pMode->SynthClock /= 2;
	       pMode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3020_DAC:
	    if (pMode->SynthClock > 100000) {
	       pMode->SynthClock /= 2;
	       pMode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3025_DAC:
	    if (pMode->SynthClock > 80000) {
               /* the SynthClock will be divided and clock doubled by the PLL */
	       pMode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3026_DAC:  /* IBMRGB??? */
	 case TI3030_DAC:
            if (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
               /*
                * for the mixed Ti3026/3030 + ICD2061A cases we need to split
                * at 120MHz; Since the ICD2061A clock code dislikes 120MHz
                * we already double for that
                */
	       if (pMode->SynthClock >= 120000) {
	          pMode->Flags |= V_DBLCLK;
	          pMode->SynthClock /= 2;
	       }
	    } else {
	       /*
	        * use the Ti3026/3030 clock
	        */
	       if (pMode->SynthClock > 80000) {
                  /* 
                   * the SynthClock will be divided and clock doubled 
                   * by the PLL 
                   */
	          pMode->Flags |= V_DBLCLK;
	       }
	    }
	    break;
	 case IBMRGB52x_DAC:
	 case IBMRGB524_DAC:
	 case IBMRGB525_DAC:
	 case IBMRGB528_DAC:
	    if (pMode->SynthClock > 80000 || S3_968_SERIES(s3ChipId)) {
	       pMode->Flags |= V_DBLCLK;
	    }
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
	        * This one depend on pixel multiplexing for 8bpp.
	        * Although existing code implies it depends on ramdac
	        * clock doubling instead (are the two tied together?)
	        * We'll act based on clock doubling changeover at 67500
	        */
	       if (( DAC_IS_ATT20C498 && pMode->SynthClock > nonMuxMaxClock) ||
		   (!DAC_IS_ATT20C498 && pMode->SynthClock > 67500)) {
		  if (!(DAC_IS_SDAC)) {
		     pMode->SynthClock /= 2;
		     pMode->Flags |= V_DBLCLK;
		  }
	       }
	       break;
	    case 2:
	       /* No change for 16bpp */
	       break;
	    case 4:
	       pMode->SynthClock *= 2;
	       break;
	    }
	    break;
	 case S3_TRIO32_DAC:
	 case S3_TRIO64_DAC:
	    switch (s3Bpp) {
	    case 1:
#if 0  
	       /* XXXX pMode->SynthClock /= 2 might be better with sr15 &= ~0x40
		  in s3init.c if screen wouldn't completely blank... */
	       if (pMode->SynthClock > nonMuxMaxClock) {
		  pMode->SynthClock /= 2;
		  pMode->Flags |= V_DBLCLK;
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
	 case SC1148x_M2_DAC:
	 case SC1148x_M3_DAC:
	 case SC15025_DAC:
	 case S3_GENDAC_DAC:
	    if (s3Bpp > 1) {
	       pMode->SynthClock *= s3Bpp;
	    }
	    break;
	 default:
	    /* Do nothing */
	    break;
	 }
	 pMode = pMode->next;
      } while (pMode != pEnd);
   }

   pEnd = pMode = s3InfoRec.modes;
   do {
      /* Setup the Mode.Private if required */
      if (S3_964_SERIES(s3ChipId) || S3_968_SERIES(s3ChipId)) {
	 if (!pMode->PrivSize || !pMode->Private) {
	    pMode->PrivSize = S3_MODEPRIV_SIZE;
	    pMode->Private = (INT32 *)xcalloc(sizeof(INT32), S3_MODEPRIV_SIZE);
	    pMode->Private[0] = 0;
	 }

	 /* Set default for invert_vclk */
	 if (!(pMode->Private[0] & (1 << S3_INVERT_VCLK))) {
	    if (DAC_IS_TI3026 && (s3BiosVendor == DIAMOND_BIOS ||
				  OFLG_ISSET(OPTION_DIAMOND, &s3InfoRec.options)))
	       pMode->Private[S3_INVERT_VCLK] = 1;
	    else if (DAC_IS_TI3030) 
	       if ((s3Bpp == 2 && (pMode->Flags & V_DBLCLK)) || s3Bpp == 4)
		  pMode->Private[S3_INVERT_VCLK] = 1;
	       else 
		  pMode->Private[S3_INVERT_VCLK] = 0;
	    else if (DAC_IS_IBMRGB)
	       if (s3Bpp == 4) 
		  pMode->Private[S3_INVERT_VCLK] = 0;
	       else if (s3BiosVendor == STB_BIOS && s3Bpp == 2 
			&& s3InfoRec.clock[pMode->Clock] > 125000 
			&& s3InfoRec.clock[pMode->Clock] < 175000)
		  pMode->Private[S3_INVERT_VCLK] = 0;
	       else if ((s3BiosVendor == NUMBER_NINE_BIOS ||
			 s3BiosVendor == HERCULES_BIOS) &&
			S3_968_SERIES(s3ChipId))
		  pMode->Private[S3_INVERT_VCLK] = 0;
	       else
		  pMode->Private[S3_INVERT_VCLK] = 1;
	    else 
	       pMode->Private[S3_INVERT_VCLK] = 0;
	    pMode->Private[0] |= 1 << S3_INVERT_VCLK;
	 }

	 /* Set default for blank_delay */
	 if (!(pMode->Private[0] & (1 << S3_BLANK_DELAY))) {
	    pMode->Private[0] |= (1 << S3_BLANK_DELAY);
	    if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES) {
	       if ((pMode->Flags & V_DBLCLK) || s3Bpp > 1)
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	       else
		  pMode->Private[S3_BLANK_DELAY] = 0x01;
	    } else if (DAC_IS_TI3025) {
	       if (s3Bpp == 1)
		  if (pMode->Flags & V_DBLCLK)
		     pMode->Private[S3_BLANK_DELAY] = 0x02;
		  else
		     pMode->Private[S3_BLANK_DELAY] = 0x03;
	       else if (s3Bpp == 2)
		  if (pMode->Flags & V_DBLCLK)
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
		  else
		     pMode->Private[S3_BLANK_DELAY] = 0x01;
	       else /* (s3Bpp == 4) */
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	    } else if (DAC_IS_TI3026) {
	       if (s3BiosVendor == DIAMOND_BIOS 
                   || OFLG_ISSET(OPTION_DIAMOND, &s3InfoRec.options)) {
	          if (s3Bpp == 1) 
		     pMode->Private[S3_BLANK_DELAY] = 0x72;
	          else if (s3Bpp == 2) 
		     pMode->Private[S3_BLANK_DELAY] = 0x73;
	          else /*if (s3Bpp == 4)*/ 
		     pMode->Private[S3_BLANK_DELAY] = 0x75;
	       } else {
	          if (s3Bpp == 1) 
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
	          else if (s3Bpp == 2) 
		     pMode->Private[S3_BLANK_DELAY] = 0x01;
	          else /*if (s3Bpp == 4)*/ 
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	    } else if (DAC_IS_TI3030){
	       if (s3Bpp == 1 || (s3Bpp == 2 && !(pMode->Flags & V_DBLCLK)))
		  pMode->Private[S3_BLANK_DELAY] = 0x01;
	       else
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	    } else if (DAC_IS_IBMRGB) {
	       if (s3BiosVendor == GENOA_BIOS) {
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	       else if (s3BiosVendor == STB_BIOS) {
		  if (s3Bpp == 1 && s3InfoRec.clock[pMode->Clock] > 50000)
		     pMode->Private[S3_BLANK_DELAY] = 0x55;
		  else
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	       else if (s3BiosVendor == HERCULES_BIOS) {
		 if (S3_968_SERIES(s3ChipId)) {
		   pMode->Private[S3_BLANK_DELAY] = 0x00;
		 }
		 else {
		   pMode->Private[S3_BLANK_DELAY] = (4/s3Bpp) - 1;
		   if (pMode->Flags & V_DBLCLK) 
		     pMode->Private[S3_BLANK_DELAY] >>= 1; 
		 }
	       }
	       else
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	    } else {
	       pMode->Private[S3_BLANK_DELAY] = 0x00;
	    }
	 }
	 
	 /* Set default for early_sc */      
	 if (!(pMode->Private[0] & (1 << S3_EARLY_SC))) {
	    pMode->Private[0] |= 1 << S3_EARLY_SC;
	    if (DAC_IS_TI3025) {
	       if (OFLG_ISSET(OPTION_NUMBER_NINE,&s3InfoRec.options))
		  pMode->Private[S3_EARLY_SC] = 1;
	       else
		  pMode->Private[S3_EARLY_SC] = 0;
	    } else if ((DAC_IS_TI3026 || DAC_IS_TI3030)
		       && OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
	       if (s3Bpp == 2 && (pMode->Flags & V_DBLCLK))
		  pMode->Private[S3_EARLY_SC] = 1;
	       else
		  pMode->Private[S3_EARLY_SC] = 0;
	    } else if (DAC_IS_TI3026 
		       && OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
	       if (s3Bpp == 2 && (pMode->Flags & V_DBLCLK))
		  pMode->Private[S3_EARLY_SC] = 1;
	       else
		  pMode->Private[S3_EARLY_SC] = 0;
	    } else if (DAC_IS_IBMRGB) {
	       if (s3BiosVendor == GENOA_BIOS) {
	          pMode->Private[S3_EARLY_SC] = 0;
	       }
	       else if (s3BiosVendor == STB_BIOS) {
		  if (s3Bpp == 2 && s3InfoRec.clock[pMode->Clock] > 125000)
		     pMode->Private[S3_EARLY_SC] = 0;
		  else if (s3Bpp == 4)
		     pMode->Private[S3_EARLY_SC] = 0;
		  else 
		     pMode->Private[S3_EARLY_SC] = 1;
	       }
	       else if (s3BiosVendor == HERCULES_BIOS) {
		 if (S3_968_SERIES(s3ChipId))
		   pMode->Private[S3_EARLY_SC] = 0;
		 else
		   pMode->Private[S3_EARLY_SC] = 0;
	       }
	       else
	          pMode->Private[S3_EARLY_SC] = 0;
	    } else {
	       pMode->Private[S3_EARLY_SC] = 0;
	    }
	 }
      }
      pMode = pMode->next;
   } while (pMode != pEnd);

   if (DAC_IS_BT485_SERIES || DAC_IS_TI3020_SERIES || DAC_IS_TI3026 
        || DAC_IS_TI3030 || DAC_IS_IBMRGB) {
      if (!OFLG_ISSET(OPTION_DAC_6_BIT, &s3InfoRec.options) || s3Bpp > 1)
	 s3DAC8Bit = TRUE;
      if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &s3InfoRec.options)) {
	 s3DACSyncOnGreen = TRUE;
	 if (xf86Verbose)
	    ErrorF("%s %s: Putting RAMDAC into sync-on-green mode\n",
		   XCONFIG_GIVEN, s3InfoRec.name);
      }
   }

   if (DAC_IS_SC15025 || DAC_IS_ATT498 || DAC_IS_STG1700 || DAC_IS_ATT20C409) {
      if (!OFLG_ISSET(OPTION_DAC_6_BIT, &s3InfoRec.options) || s3Bpp > 1)
         s3DAC8Bit = TRUE;
   }

   if (DAC_IS_ATT490) {
      if (OFLG_ISSET(OPTION_DAC_8_BIT, &s3InfoRec.options) || s3Bpp > 1)
         s3DAC8Bit = TRUE;
   }

   if (OFLG_ISSET(OPTION_DAC_8_BIT, &s3InfoRec.options) && !s3DAC8Bit) {
      ErrorF("%s %s: Option \"dac_8_bit\" not recognised for RAMDAC \"%s\"\n",
	     XCONFIG_PROBED, s3InfoRec.name, s3InfoRec.ramdac);
   }

   if (xf86Verbose) {
      if (s3InfoRec.bitsPerPixel == 8)
	 ErrorF("%s %s: Using %d bits per RGB value\n",
		OFLG_ISSET(OPTION_DAC_8_BIT, &s3InfoRec.options) ||
		OFLG_ISSET(OPTION_DAC_6_BIT, &s3InfoRec.options) ?
		XCONFIG_GIVEN : XCONFIG_PROBED, s3InfoRec.name,
		s3DAC8Bit ?  8 : 6);
      else if (s3InfoRec.bitsPerPixel == 16)
	 ErrorF("%s %s: Using 16 bpp.  Color weight: %1d%1d%1d\n",
		XCONFIG_GIVEN, s3InfoRec.name, xf86weight.red,
		xf86weight.green, xf86weight.blue);
      else if (xf86bpp == 24)
	 ErrorF("%s %s: Using packed 24 bpp.  Color weight: %1d%1d%1d\n",
		XCONFIG_GIVEN, s3InfoRec.name, xf86weight.red,
		xf86weight.green, xf86weight.blue);
      else if (s3InfoRec.bitsPerPixel == 32)
	 ErrorF("%s %s: Using sparse 32 bpp.  Color weight: %1d%1d%1d\n",
		XCONFIG_GIVEN, s3InfoRec.name, xf86weight.red,
		xf86weight.green, xf86weight.blue);
   }

   /* Select the appropriate logical line width */
   if (s3UsingPixMux && pixMuxLimitedWidths) {
      if (s3InfoRec.virtualX <= 1024) {
	 s3DisplayWidth = 1024;
      } else if (s3InfoRec.virtualX <= 2048) {
	 s3DisplayWidth = 2048;
      } else { /* should never get here */
	 ErrorF("Internal error in DisplayWidth check, virtual width = %d\n",
	        s3InfoRec.virtualX);
	 xf86DisableIOPorts(s3InfoRec.scrnIndex);
	 return (FALSE);
      }
   } else if (S3_911_SERIES(s3ChipId)) {
      s3DisplayWidth = 1024;
   } else {
      if (s3InfoRec.virtualX <= 640) {
	 s3DisplayWidth = 640;
      } else if (s3InfoRec.virtualX <= 800) {
	 s3DisplayWidth = 800;
      } else if (s3InfoRec.virtualX <= 1024) {
	 s3DisplayWidth = 1024;
      } else if ((s3InfoRec.virtualX <= 1152) &&
		 (   S3_801_REV_C(s3ChipId) 
                  || S3_805_I_SERIES(s3ChipId)
		  || S3_928_REV_E(s3ChipId)
		  || S3_x64_SERIES(s3ChipId))) {
	 s3DisplayWidth = 1152;
      } else if (s3InfoRec.virtualX <= 1280) {
	 s3DisplayWidth = 1280;
      } else if ((s3InfoRec.virtualX <= 1600) && 
		 ((S3_928_REV_E(s3ChipId) &&
		   !(OFLG_ISSET(OPTION_NUMBER_NINE, &s3InfoRec.options) &&
		     (s3Bpp == 1)))
		  || S3_x64_SERIES(s3ChipId))) {
	 s3DisplayWidth = 1600;
      } else if (s3InfoRec.virtualX <= 2048) {
	 s3DisplayWidth = 2048;
      } else { /* should never get here */
	 ErrorF("Internal error in DisplayWidth check, virtual width = %d\n",
	        s3InfoRec.virtualX);
	 xf86DisableIOPorts(s3InfoRec.scrnIndex);
	 return (FALSE);
      }
   }
   s3BppDisplayWidth = s3Bpp * s3DisplayWidth;
   /*
    * Work out where to locate S3's HW cursor storage.  It must be on a
    * 1k boundary.  When using a RAMDAC cursor, set s3CursorStartY
    * and s3CursorLines appropriately for the memory usage calculation below
    */

   if (OFLG_ISSET(OPTION_BT485_CURS, &s3InfoRec.options) ||
       OFLG_ISSET(OPTION_TI3020_CURS, &s3InfoRec.options) ||
       OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options) ||
       OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options) ||
       OFLG_ISSET(OPTION_SW_CURSOR, &s3InfoRec.options)) {
      s3CursorStartY = s3InfoRec.virtualY;
      s3CursorLines = 0;
   } else {
      int st_addr = (s3InfoRec.virtualY * s3BppDisplayWidth + 1023) & ~1023;
      s3CursorStartX = st_addr % s3BppDisplayWidth;
      s3CursorStartY = st_addr / s3BppDisplayWidth;
      s3CursorLines = ((s3CursorStartX + 1023) / s3BppDisplayWidth) + 1;
   }

   /*
    * Reduce the videoRam value if necessary to prevent Y coords exceeding
    * the 12-bit (4096) limit when small display widths are used on cards
    * with a lot of memory
    */
   if (s3InfoRec.videoRam * 1024 / s3BppDisplayWidth > 4096) {
      s3InfoRec.videoRam = s3BppDisplayWidth * 4096 / 1024;
      ErrorF("%s %s: videoram usage reduced to %dk to avoid co-ord overflow\n",
	     XCONFIG_PROBED, s3InfoRec.name, s3InfoRec.videoRam);
   }
  
   /*
    * This one is difficult to deal with by deleting modes.  Do you delete
    * modes to reduce the vertical size or to reduce the displayWidth?  Either
    * way it will require the recalculation of everything above.  This one
    * is in the too-hard basket.
    */
   if ((s3BppDisplayWidth * (s3CursorStartY + s3CursorLines)) >
       s3InfoRec.videoRam * 1024) { /* XXXX improve this message */
      ErrorF("%s %s: Display size %dx%d is too large: ", 
             OFLG_ISSET(XCONFIG_VIRTUAL,&s3InfoRec.xconfigFlag) ?
                 XCONFIG_GIVEN : XCONFIG_PROBED,
             s3InfoRec.name,
	     s3DisplayWidth, s3InfoRec.virtualY);
      xf86DisableIOPorts(s3InfoRec.scrnIndex);
      return (FALSE);
   }
   if (xf86Verbose) {
      ErrorF("%s %s: Virtual resolution set to %dx%d\n", 
             OFLG_ISSET(XCONFIG_VIRTUAL,&s3InfoRec.xconfigFlag) ?
                 XCONFIG_GIVEN : XCONFIG_PROBED,
             s3InfoRec.name,
	     s3InfoRec.virtualX, s3InfoRec.virtualY);
   }

   if (OFLG_ISSET(OPTION_PCI_HACK, &s3InfoRec.options))
      s3PCIHack = TRUE;
   if (OFLG_ISSET(OPTION_POWER_SAVER, &s3InfoRec.options))
      s3PowerSaver = TRUE;

   if (! (s3Port59 | s3Port5A)) { /* s3Port59/s3Port5A not yet initialized */
      if (s3InfoRec.MemBase != 0) {
	 s3Port59 =  s3InfoRec.MemBase >> 24;
	 s3Port5A = (s3InfoRec.MemBase >> 16) & 0x08;
      }
      else {
	 outb(vgaCRIndex, 0x59);
	 s3Port59 = inb(vgaCRReg);
	 outb(vgaCRIndex, 0x5a);
	 s3Port5A = inb(vgaCRReg);
      }
   }


#ifdef XFreeXDGA
      s3InfoRec.displayWidth = s3DisplayWidth;
      s3InfoRec.directMode = XF86DGADirectPresent;
#endif
   in_s3Probe = FALSE;
   return TRUE;
}

#ifdef PC98
void
#if NeedFunctionPrototypes
s3ConnectPCI(CARD16 vendor, CARD16 device)
#else
s3ConnectPCI(vendor, device)
    CARD16 vendor;
    CARD16 device;
#endif
{
    pciConfigPtr pcrp, *pcrpp;
    unsigned int dev;

    pcrpp = xf86scanpci(s3InfoRec.scrnIndex);

    if (!pcrpp)
	return;

    for (dev = 0; (pcrp = pcrpp[dev]) != NULL; dev ++)
    {
	if (pcrp->_vendor == vendor && pcrp->_device == device)
	{
	    xf86writepci(s3InfoRec.scrnIndex, pcrp->_bus, pcrp->_cardnum,
		pcrp->_func,
		PCI_CMD_STAT_REG, PCI_CMD_MASK,
		PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE);
	    break;
	}
    }

    xf86cleanpci();
    return;
}

void
#if NeedFunctionPrototypes
s3DisconnectPCI(CARD16 vendor, CARD16 device)
#else
s3DisconnectPCI(vendor, device)
    CARD16 vendor;
    CARD16 device;
#endif
{
    pciConfigPtr pcrp, *pcrpp;
    unsigned int dev;

    pcrpp = xf86scanpci(s3InfoRec.scrnIndex);

    if (!pcrpp)
	return;

    for (dev = 0; (pcrp = pcrpp[dev]) != NULL; dev ++)
    {
	if (pcrp->_vendor == vendor && pcrp->_device == device)
	{
	    xf86writepci(s3InfoRec.scrnIndex, pcrp->_bus, pcrp->_cardnum,
		pcrp->_func,
		PCI_CMD_STAT_REG, PCI_CMD_MASK,
		0);
	    break;
	}
    }

    xf86cleanpci();
    return;
}
#endif

static Bool
s3ClockSelect(no)
     int   no;

{
   unsigned char temp;
   static unsigned char save1, save2;
 
   UNLOCK_SYS_REGS;
   
   switch(no)
   {
   case CLK_REG_SAVE:
      save1 = inb(0x3CC);
      outb(vgaCRIndex, 0x42);
      save2 = inb(vgaCRReg);
      break;
   case CLK_REG_RESTORE:
      outb(0x3C2, save1);
      outb(vgaCRIndex, 0x42);
      outb(vgaCRReg, save2);
      break;
   default:
      no &= 0xF;
      if (no == 0x03)
      {
	 /*
	  * Clock index 3 is a 0Hz clock on all the S3-recommended 
	  * synthesizers (except the Chrontel).  A 0Hz clock will lock up 
	  * the chip but good (requiring power to be cycled).  Nuke that.
	  */
         LOCK_SYS_REGS;
	 return(FALSE);
      }
      temp = inb(0x3CC);
      outb(0x3C2, temp | 0x0C);
      outb(vgaCRIndex, 0x42);
      temp = inb(vgaCRReg) & 0xf0;
      outb(vgaCRReg, temp | no);
      usleep(150000);
   }
   LOCK_SYS_REGS;
   return(TRUE);
}


static Bool
LegendClockSelect(no)
     int   no;
{

 /*
  * Sigma Legend special handling
  * 
  * The Legend uses an ICS 1394-046 clock generator.  This can generate 32
  * different frequencies.  The Legend can use all 32.  Here's how:
  * 
  * There are two flip/flops used to latch two inputs into the ICS clock
  * generator.  The five inputs to the ICS are then
  * 
  * ICS     ET-4000 ---     --- FS0     CS0 FS1     CS1 FS2     ff0 flip/flop 0
  * outbut FS3     CS2 FS4     ff1     flip/flop 1 outbut
  * 
  * The flip/flops are loaded from CS0 and CS1.  The flip/flops are latched by
  * CS2, on the rising edge. After CS2 is set low, and then high, it is then
  * set to its final value.
  * 
  */
   static unsigned char save1, save2;
   unsigned char temp = inb(0x3CC);

   switch(no)
   {
   case CLK_REG_SAVE:
      save1 = inb(0x3CC);
      outb(vgaCRIndex, 0x34);
      save2 = inb(vgaCRReg);
      break;
   case CLK_REG_RESTORE:
      outb(0x3C2, save1);
      outw(vgaCRIndex, 0x34 | (save2 << 8));
      break;
   default:
      outb(0x3C2, (temp & 0xF3) | ((no & 0x10) >> 1) | (no & 0x04));
      outw(vgaCRIndex, 0x0034);
      outw(vgaCRIndex, 0x0234);
      outw(vgaCRIndex, ((no & 0x08) << 6) | 0x34);
      outb(0x3C2, (temp & 0xF3) | ((no << 2) & 0x0C));
   }
   return(TRUE);
}

static Bool
icd2061ClockSelect(freq)
     int   freq;
{
   Bool result = TRUE;

   UNLOCK_SYS_REGS;
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Convert freq to Hz */
	 freq *= 1000;
	 /* Use the "Alt" version always since it is more reliable */
	 if (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
	    /* setting exactly 120 MHz doesn't work all the time */
	    if (freq > 119900000) freq = 119900000;
#if defined(PC98_PW) || defined(PC98_PWLB)
	    AltICD2061SetClock(freq, 0);
#else
	    AltICD2061SetClock(freq, 2);
	    AltICD2061SetClock(freq, 2);
	    AltICD2061SetClock(freq, 2);
#endif
	    if (DAC_IS_TI3026 || DAC_IS_TI3030) {
	       /* 
	        * then we need to setup the loop clock
	        */
	       Ti3026SetClock(freq/1000, 2, s3Bpp, TI_LOOP_CLOCK);
	       s3OutTi3026IndReg(TI_MCLK_LCLK_CONTROL, ~0x20, 0x20);
            }
	 } else if (OFLG_ISSET(CLOCK_OPTION_SC11412, &s3InfoRec.clockOptions)) {
	    result = SC11412SetClock((long)freq/1000);
	 } else if (OFLG_ISSET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions)) {
	    result = ICS2595SetClock((long)freq/1000);
	    result = ICS2595SetClock((long)freq/1000);
	 } else { /* Should never get here */
	    result = FALSE;
	    break;
	 }

	 if (!OFLG_ISSET(CLOCK_OPTION_ICS2595, &s3InfoRec.clockOptions)) {
	    unsigned char tmp;
	    outb(vgaCRIndex, 0x42);/* select the clock */
	    tmp = inb(vgaCRReg) & 0xf0;
	    if (OFLG_ISSET(OPTION_SPEA_MERCURY, &s3InfoRec.options) &&
                S3_964_SERIES(s3ChipId)) /* SPEA Mercury P64 uses bit2/3  */
                 outb(vgaCRReg, tmp | 0x06);   /* for synchronizing reasons (?) */
            else outb(vgaCRReg, tmp | 0x02); 
            usleep(150000);
	 }
	 /* Do the clock doubler selection in s3Init() */
      }
   }
   LOCK_SYS_REGS;
   return(result);
}


/* the ELSA Gloria-8 uses a TVP3030 with ICS9161 as refclock */

static Bool
Gloria8ClockSelect(freq)
     int   freq;
{
   Bool result = TRUE;
   unsigned char tmp;
   int p;

   UNLOCK_SYS_REGS;
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      for(p=0; p<4; p++)
	 if ((freq << p) >= 120000) break;

      AltICD2061SetClock((freq * 1000) >> (3-p), 2);

      Ti3030SetClock(14318 << (3-p), 2, s3Bpp, TI_BOTH_CLOCKS);
      Ti3030SetClock(freq, 2, s3Bpp, TI_LOOP_CLOCK);
      s3OutTi3026IndReg(TI_MCLK_LCLK_CONTROL, ~0x38, 0x30);
      s3OutTi3026IndReg(TI_MCLK_LCLK_CONTROL, ~0x38, 0x38);

      outb(vgaCRIndex, 0x42);/* select the clock */
      tmp = inb(vgaCRReg) & 0xf0;
      outb(vgaCRReg, tmp | 0x06);
   }
   LOCK_SYS_REGS;
   return(result);
}


/* The GENDAC code also works for the SDAC, used for Trio32/Trio64 too */

static Bool
s3GendacClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
   unsigned char tmp;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
#if defined(PC98_PW)
	(void) S3gendacSetClock(freq, 7);  /* PW805i can't use reg 2 */
#else

	 if (S3_TRIOxx_SERIES(s3ChipId)) {
	    (void) S3TrioSetClock(freq, 2); /* can't fail */
	 }
	 else {
	    if (OFLG_ISSET(CLOCK_OPTION_ICS5342, &s3InfoRec.clockOptions))
	       (void) ICS5342SetClock(freq, 2); /* can't fail */
	    else
	       (void) S3gendacSetClock(freq, 2); /* can't fail */
#endif
	    outb(vgaCRIndex, 0x42);/* select the clock */
#if defined(PC98_PW)
	    tmp = inb(vgaCRReg) & 0xf0;
	    outb(vgaCRReg, tmp | 0x07);
#else
	    tmp = inb(vgaCRReg) & 0xf0;
	    outb(vgaCRReg, tmp | 0x02);
#endif
	    usleep(150000);
#if !defined(PC98_PW)
	 }
#endif
      }
   }
   LOCK_SYS_REGS;
   return(result);
}


static void
#if NeedFunctionPrototypes
s3ProgramTi3025Clock(
int clk,
unsigned char n,
unsigned char m,
unsigned char p)
#else
s3ProgramTi3025Clock(clk, n, m, p)
int clk;
unsigned char n;
unsigned char m;
unsigned char p;
#endif
{
   /*
    * Reset the clock data index
    */
   s3OutTiIndReg(TI_PLL_CONTROL, 0x00, 0x00);

   if (clk != TI_MCLK_PLL_DATA) {
      /*
       * Now output the clock frequency
       */
      s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, n);
      s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, m);
      s3OutTiIndReg(TI_PIXEL_CLOCK_PLL_DATA, 0x00, p | TI_PLL_ENABLE);

      /*
       * And now set up the loop clock for RCLK
       */
      s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, 0x01);
      s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, 0x01);
      s3OutTiIndReg(TI_LOOP_CLOCK_PLL_DATA, 0x00, p>0 ? p : 1);
      s3OutTiIndReg(TI_MISC_CONTROL, 0x00,
		    TI_MC_LOOP_PLL_RCLK | TI_MC_LCLK_LATCH | TI_MC_INT_6_8_CONTROL);

      /*
       * And finally enable the clock
       */
      s3OutTiIndReg(TI_INPUT_CLOCK_SELECT, 0x00, TI_ICLK_PLL);
   } else {
      /*
       * Set MCLK
       */
      s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, n);
      s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, m);
      s3OutTiIndReg(TI_MCLK_PLL_DATA, 0x00, p | 0x80);
   }
}

static Bool
ti3025ClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
   unsigned char tmp;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Check if clock frequency is within range */
	 /* XXXX Check this elsewhere */
	 if (freq < 20000) {
	    ErrorF("%s %s: Specified dot clock (%.3f) too low for TI 3025",
		   XCONFIG_PROBED, s3InfoRec.name, freq / 1000.0);
	    result = FALSE;
	    break;
	 }
	 (void) Ti3025SetClock(freq, 2, s3ProgramTi3025Clock); /* can't fail */
	 outb(vgaCRIndex, 0x42);/* select the clock */
	 tmp = inb(vgaCRReg) & 0xf0;
	 outb(vgaCRReg, tmp | 0x02);
	 usleep(150000);
      }
   }
   LOCK_SYS_REGS;
   return(result);
}


static Bool
ti3026ClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
   unsigned char tmp;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Check if clock frequency is within range */
	 /* XXXX Check this elsewhere */
	 if (freq < 13750) {
	    ErrorF("%s %s: Specified dot clock (%.3f) too low for Ti3026/3030",
		   XCONFIG_PROBED, s3InfoRec.name, freq / 1000.0);
	    result = FALSE;
	    break;
	 }
	 (void) Ti3026SetClock(freq, 2, s3Bpp, TI_BOTH_CLOCKS); /* can't fail */
	 outb(vgaCRIndex, 0x42);/* select the clock */
	 tmp = inb(vgaCRReg) & 0xf0;
	 outb(vgaCRReg, tmp | 0x02);
      }
   }
   LOCK_SYS_REGS;
   return(result);
}

static Bool
IBMRGBClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Check if clock frequency is within range */
	 /* XXXX Check this elsewhere */
	 if (freq < 16250) {
	    ErrorF("%s %s: Specified dot clock (%.3f) too low for IBM RGB52x",
		   XCONFIG_PROBED, s3InfoRec.name, freq / 1000.0);
	    result = FALSE;
	    break;
	 }
	 (void)IBMRGBSetClock(freq, 2, s3InfoRec.dacSpeed, s3InfoRec.s3RefClk);
      }
   }
   LOCK_SYS_REGS;
   return(result);
}

static Bool
ch8391ClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
   unsigned char tmp;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Check if clock frequency is within range */
	 /* XXXX Check this elsewhere */
	 if (freq < 8500 || freq > 135000) {
	    ErrorF("%s %s: Specified dot clock (%.3f) out of range for Chrontel 8391",
		   XCONFIG_PROBED, s3InfoRec.name, freq / 1000.0);
	    result = FALSE;
	    break;
	 }
	 (void) Chrontel8391SetClock(freq, 2); /* can't fail */
	 outb(vgaCRIndex, 0x42);/* select the clock */
	 tmp = inb(vgaCRReg) & 0xf0;
	 outb(vgaCRReg, tmp | 0x02);
	 usleep(150000);
      }
   }
   LOCK_SYS_REGS;
   return(result);
}

static Bool
att409ClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
   unsigned char tmp;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Check if clock frequency is within range */
	 /* XXXX Check this elsewhere */
	 if (freq < 15000 || freq > 240000) {
	    ErrorF("%s %s: Specified dot clock (%.3f) out of range for ATT20C409",
		   XCONFIG_PROBED, s3InfoRec.name, freq / 1000.0);
	    result = FALSE;
	    break;
	 }
	 (void) Att409SetClock(freq, 2); /* can't fail */
	 outb(vgaCRIndex, 0x42);/* select the clock */
	 tmp = inb(vgaCRReg) & 0xf0;
	 outb(vgaCRReg, tmp | 0x02);
	 usleep(150000);
      }
   }
   LOCK_SYS_REGS;
   return(result);
}

static Bool
STG1703ClockSelect(freq)
     int   freq;

{
   Bool result = TRUE;
   unsigned char tmp;
 
   UNLOCK_SYS_REGS;
   
   switch(freq)
   {
   case CLK_REG_SAVE:
   case CLK_REG_RESTORE:
      result = s3ClockSelect(freq);
      break;
   default:
      {
	 /* Check if clock frequency is within range */
	 /* XXXX Check this elsewhere */
	 if (freq < 8500 || freq > 135000) {
	    ErrorF("%s %s: Specified dot clock (%.3f) out of range for STG1703",
		   XCONFIG_PROBED, s3InfoRec.name, freq / 1000.0);
	    result = FALSE;
	    break;
	 }
	 (void) STG1703SetClock(freq, 2); /* can't fail */
	 outb(vgaCRIndex, 0x42);/* select the clock */
	 tmp = inb(vgaCRReg) & 0xf0;
	 outb(vgaCRReg, tmp | 0x02);
	 usleep(150000);
      }
   }
   LOCK_SYS_REGS;
   return(result);
}

/*
 * Quick way to get s3ValidMode actually doing something
 * so the new vidmode extension functions can use it.
 * s3Probe() should be altered to use this... eventually.
 *			MArk (mvojkovi@ucsd.edu)
 */
static int
s3ValidMode(DisplayModePtr pMode, Bool verbose)
{
    Bool ModeCantPixmux = FALSE;

    if(in_s3Probe)
	return MODE_OK;

    /* Trivial size tests */
    if(pMode->HDisplay > maxDisplayWidth) {
	if(verbose)
	   ErrorF("%s %s: Width of mode \"%s\" is too large (max is %d)\n",
		XCONFIG_PROBED, s3InfoRec.name, pMode->name, maxDisplayWidth);
	return MODE_BAD;
    } else if(pMode->VDisplay > maxDisplayHeight) {
	if(verbose)
	   ErrorF("%s %s: Height of mode \"%s\" is too large (max is %d)\n",
		XCONFIG_PROBED, s3InfoRec.name, pMode->name, maxDisplayHeight);
	return MODE_BAD;
    } else if((pMode->HDisplay * (1 + pMode->VDisplay) * s3Bpp) >
		 s3InfoRec.videoRam * 1024) {
	if(verbose)
	   ErrorF("%s %s: Too little memory for mode \"%s\"\n", XCONFIG_PROBED,
		s3InfoRec.name, pMode->name);
	if (!OFLG_ISSET(OPTION_BT485_CURS,  &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_TI3020_CURS, &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_TI3026_CURS, &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_IBMRGB_CURS, &s3InfoRec.options) &&
	     !OFLG_ISSET(OPTION_SW_CURSOR,   &s3InfoRec.options))
	if(verbose)
	   ErrorF("%s %s: NB. 1 scan line is required for the hardware "
		"cursor\n", XCONFIG_PROBED, s3InfoRec.name);
	return MODE_BAD;
    } else if(((s3InfoRec.virtualX > 0) && 
	       (pMode->HDisplay > s3InfoRec.virtualX)) ||
	       ((s3InfoRec.virtualY > 0) && 
	       (pMode->VDisplay > s3InfoRec.virtualY))) {
	if(verbose)
	  ErrorF("%s %s: Resolution %dx%d too large for virtual %dx%d\n",
		XCONFIG_PROBED, s3InfoRec.name, pMode->HDisplay,
		pMode->VDisplay, s3InfoRec.virtualX, s3InfoRec.virtualY);
	return MODE_BAD;
    } 

    if (pixMuxPossible) {

	/* Find out if the mode requires pixmux */

 	if (s3Bpp == 1 && s3ATT498PixMux && !DAC_IS_SDAC &&
		(S3_864_SERIES(s3ChipId) || S3_805_I_SERIES(s3ChipId))
		&& !OFLG_ISSET(CLOCK_OPTION_PROGRAMABLE,
			       &s3InfoRec.clockOptions)) {
	       if (pMode->Clock > 15) {
		  pMode->Flags |= V_PIXMUX;
	       }
	}
	else if (s3InfoRec.clock[pMode->Clock] > nonMuxMaxClock) {
	        pMode->Flags |= V_PIXMUX;
	}
	if (s3InfoRec.videoRam > nonMuxMaxMemory) {
	       pMode->Flags |= V_PIXMUX;
	}
	if ((OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
		 OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options)) &&
		s3InfoRec.virtualX * s3InfoRec.virtualY > 2*1024*1024) {
	      /* PIXMUX must be used to access more than 2mb memory. */
	      pMode->Flags |= V_PIXMUX;
	}
	
	/* Find out if the mode can't be used with pixmux */

	if (pMode->HDisplay < pixMuxMinWidth)
	    ModeCantPixmux = TRUE;

	if ((pMode->Flags & V_INTERLACE) && !allowPixMuxInterlace)
	    ModeCantPixmux = TRUE;

	if (s3InfoRec.clock[pMode->Clock] < pixMuxMinClock)
	    ModeCantPixmux = TRUE;

	if (!allowPixMuxSwitching ) {

	    /* set V_PIXMUX if s3UsingPixMux and the mode can do pimux */

	    if (s3UsingPixMux && !ModeCantPixmux)
		pMode->Flags |= V_PIXMUX;

	    /*
	     * If switching between pixmux and non-pixmux isn't allowed, the
	     * mode is rejected when:
	     *
	     *   1. The initial mode set contains no modes requiring pixmux,
	     *      but this one needs it.
	     *
	     *   2. The initial mode set contains modes requiring pixmux,
	     *      and this one can't use pixmux.
	     */

	    if (s3UsingPixMux && ModeCantPixmux) {
		if (verbose) {
		   ErrorF("%s %s: Mode \"%s\" can't work with pixel "
			"multiplexing and is\n",
			XCONFIG_PROBED, s3InfoRec.name, pMode->name);
		   ErrorF("\tincompatible with the current modes.\n");
		}
		return MODE_BAD;
	    }

	    if (!s3UsingPixMux && (pMode->Flags & V_PIXMUX)) {
		if (verbose) {
		   ErrorF("%s %s: Mode \"%s\" requires pixel multiplexing "
			"and is\n", XCONFIG_PROBED, s3InfoRec.name,
			pMode->name);
		   ErrorF("\tincompatible with in the current mode.\n");
		}
		return MODE_BAD;
	    }
	}

    }  /* pixMuxPossible */	 


   /*
    * For programmable clocks, fill in the SynthClock value
    * and set V_DBLCLK as required for each mode
    */

   pMode->SynthClock = s3InfoRec.clock[pMode->Clock];
	
   switch(s3RamdacType) {
	 case NORMAL_DAC:
	    /* only suports 8bpp -- nothing to do */
	    break;
	 case BT485_DAC:
	    {
	       int c;

	       if (OFLG_ISSET(OPTION_STB_PEGASUS, &s3InfoRec.options) ||
		   OFLG_ISSET(OPTION_MIRO_MAGIC_S4, &s3InfoRec.options))
		  c = 85000;
	       else if (S3_964_SERIES(s3ChipId) && s3Bpp == 4)
		  c = 90000;
	       else
		  c = 67500;
	       if (pMode->SynthClock > c) {
		  pMode->SynthClock /= 2;
		  pMode->Flags |= V_DBLCLK;
	       }
	    }
	    break;
	 case ATT20C505_DAC:
	    if (pMode->SynthClock > 90000) {
	       pMode->SynthClock /= 2;
	       pMode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3020_DAC:
	    if (pMode->SynthClock > 100000) {
	       pMode->SynthClock /= 2;
	       pMode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3025_DAC:
	    if (pMode->SynthClock > 80000) {
               /* the SynthClock will be divided and clock doubled by the PLL */
	       pMode->Flags |= V_DBLCLK;
	    }
	    break;
	 case TI3026_DAC:  /* IBMRGB??? */
	 case TI3030_DAC:
            if (OFLG_ISSET(CLOCK_OPTION_ICD2061A, &s3InfoRec.clockOptions)) {
               /*
                * for the mixed Ti3026/3030 + ICD2061A cases we need to split
                * at 120MHz; Since the ICD2061A clock code dislikes 120MHz
                * we already double for that
                */
	       if (pMode->SynthClock >= 120000) {
	          pMode->Flags |= V_DBLCLK;
	          pMode->SynthClock /= 2;
	       }
	    } else {
	       /*
	        * use the Ti3026/3030 clock
	        */
	       if (pMode->SynthClock > 80000) {
                  /* 
                   * the SynthClock will be divided and clock doubled 
                   * by the PLL 
                   */
	          pMode->Flags |= V_DBLCLK;
	       }
	    }
	    break;
	 case IBMRGB52x_DAC:
	 case IBMRGB524_DAC:
	 case IBMRGB525_DAC:
	 case IBMRGB528_DAC:
	    if (pMode->SynthClock > 80000 || S3_968_SERIES(s3ChipId)) {
	       pMode->Flags |= V_DBLCLK;
	    }
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
	        * This one depend on pixel multiplexing for 8bpp.
	        * Although existing code implies it depends on ramdac
	        * clock doubling instead (are the two tied together?)
	        * We'll act based on clock doubling changeover at 67500
	        */
	       if (( DAC_IS_ATT20C498 && pMode->SynthClock > nonMuxMaxClock) ||
		   (!DAC_IS_ATT20C498 && pMode->SynthClock > 67500)) {
		  if (!(DAC_IS_SDAC)) {
		     pMode->SynthClock /= 2;
		     pMode->Flags |= V_DBLCLK;
		  }
	       }
	       break;
	    case 2:
	       /* No change for 16bpp */
	       break;
	    case 4:
	       pMode->SynthClock *= 2;
	       break;
	    }
	    break;
	 case S3_TRIO32_DAC:
	 case S3_TRIO64_DAC:
	    switch (s3Bpp) {
	    case 1:
#if 0  
	       /* XXXX pMode->SynthClock /= 2 might be better with sr15 &= ~0x40
		  in s3init.c if screen wouldn't completely blank... */
	       if (pMode->SynthClock > nonMuxMaxClock) {
		  pMode->SynthClock /= 2;
		  pMode->Flags |= V_DBLCLK;
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
	 case SC1148x_M2_DAC:
	 case SC1148x_M3_DAC:
	 case SC15025_DAC:
	 case S3_GENDAC_DAC:
	    if (s3Bpp > 1) {
	       pMode->SynthClock *= s3Bpp;
	    }
	    break;
	 default:
	    /* Do nothing */
	    break;
   }
         
   /* Setup the Mode.Private if required */
   if (S3_964_SERIES(s3ChipId) || S3_968_SERIES(s3ChipId)) {
	 if (!pMode->PrivSize || !pMode->Private) {
	    pMode->PrivSize = S3_MODEPRIV_SIZE;
	    pMode->Private = (INT32 *)xcalloc(sizeof(INT32), S3_MODEPRIV_SIZE);
	    pMode->Private[0] = 0;
	 }

     	/* Set default for invert_vclk */
   	if (!(pMode->Private[0] & (1 << S3_INVERT_VCLK))) {
	    if (DAC_IS_TI3026 && (s3BiosVendor == DIAMOND_BIOS ||
				  OFLG_ISSET(OPTION_DIAMOND,
				  &s3InfoRec.options)))
	       pMode->Private[S3_INVERT_VCLK] = 1;
	    else if (DAC_IS_TI3030) 
	       if ((s3Bpp == 2 && (pMode->Flags & V_DBLCLK)) || s3Bpp == 4)
		  pMode->Private[S3_INVERT_VCLK] = 1;
	       else 
		  pMode->Private[S3_INVERT_VCLK] = 0;
	    else if (DAC_IS_IBMRGB)
	       if (s3Bpp == 4) 
		  pMode->Private[S3_INVERT_VCLK] = 0;
	       else if (s3BiosVendor == STB_BIOS && s3Bpp == 2 
			&& s3InfoRec.clock[pMode->Clock] > 125000 
			&& s3InfoRec.clock[pMode->Clock] < 175000)
		  pMode->Private[S3_INVERT_VCLK] = 0;
	       else if ((s3BiosVendor == NUMBER_NINE_BIOS ||
			 s3BiosVendor == HERCULES_BIOS) &&
			S3_968_SERIES(s3ChipId))
		  pMode->Private[S3_INVERT_VCLK] = 0;
	       else
		  pMode->Private[S3_INVERT_VCLK] = 1;
	    else 
	       pMode->Private[S3_INVERT_VCLK] = 0;
	    pMode->Private[0] |= 1 << S3_INVERT_VCLK;
    	}

    	/* Set default for blank_delay */
    	if (!(pMode->Private[0] & (1 << S3_BLANK_DELAY))) {
	    pMode->Private[0] |= (1 << S3_BLANK_DELAY);
	    if (S3_964_SERIES(s3ChipId) && DAC_IS_BT485_SERIES) {
	       if ((pMode->Flags & V_DBLCLK) || s3Bpp > 1)
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	       else
		  pMode->Private[S3_BLANK_DELAY] = 0x01;
	    } else if (DAC_IS_TI3025) {
	       if (s3Bpp == 1)
		  if (pMode->Flags & V_DBLCLK)
		     pMode->Private[S3_BLANK_DELAY] = 0x02;
		  else
		     pMode->Private[S3_BLANK_DELAY] = 0x03;
	       else if (s3Bpp == 2)
		  if (pMode->Flags & V_DBLCLK)
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
		  else
		     pMode->Private[S3_BLANK_DELAY] = 0x01;
	       else /* (s3Bpp == 4) */
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	    } else if (DAC_IS_TI3026) {
	       if (s3BiosVendor == DIAMOND_BIOS 
                   || OFLG_ISSET(OPTION_DIAMOND, &s3InfoRec.options)) {
	          if (s3Bpp == 1) 
		     pMode->Private[S3_BLANK_DELAY] = 0x72;
	          else if (s3Bpp == 2) 
		     pMode->Private[S3_BLANK_DELAY] = 0x73;
	          else /*if (s3Bpp == 4)*/ 
		     pMode->Private[S3_BLANK_DELAY] = 0x75;
	       } else {
	          if (s3Bpp == 1) 
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
	          else if (s3Bpp == 2) 
		     pMode->Private[S3_BLANK_DELAY] = 0x01;
	          else /*if (s3Bpp == 4)*/ 
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	    } else if (DAC_IS_TI3030){
	       if (s3Bpp == 1 || (s3Bpp == 2 && !(pMode->Flags & V_DBLCLK)))
		  pMode->Private[S3_BLANK_DELAY] = 0x01;
	       else
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	    } else if (DAC_IS_IBMRGB) {
	       if (s3BiosVendor == GENOA_BIOS) {
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	       else if (s3BiosVendor == STB_BIOS) {
		  if (s3Bpp == 1 && s3InfoRec.clock[pMode->Clock] > 50000)
		     pMode->Private[S3_BLANK_DELAY] = 0x55;
		  else
		     pMode->Private[S3_BLANK_DELAY] = 0x00;
	       }
	       else if (s3BiosVendor == HERCULES_BIOS) {
		 if (S3_968_SERIES(s3ChipId)) {
		   pMode->Private[S3_BLANK_DELAY] = 0x00;
		 }
		 else {
		   pMode->Private[S3_BLANK_DELAY] = (4/s3Bpp) - 1;
		   if (pMode->Flags & V_DBLCLK) 
		     pMode->Private[S3_BLANK_DELAY] >>= 1; 
		 }
	       }
	       else
		  pMode->Private[S3_BLANK_DELAY] = 0x00;
	    } else {
	       pMode->Private[S3_BLANK_DELAY] = 0x00;
	    }
    	}
	 
    	/* Set default for early_sc */      
    	if (!(pMode->Private[0] & (1 << S3_EARLY_SC))) {
	    pMode->Private[0] |= 1 << S3_EARLY_SC;
	    if (DAC_IS_TI3025) {
	       if (OFLG_ISSET(OPTION_NUMBER_NINE,&s3InfoRec.options))
		  pMode->Private[S3_EARLY_SC] = 1;
	       else
		  pMode->Private[S3_EARLY_SC] = 0;
	    } else if ((DAC_IS_TI3026 || DAC_IS_TI3030)
		       && OFLG_ISSET(CLOCK_OPTION_ICD2061A,
				     &s3InfoRec.clockOptions)) {
	       if (s3Bpp == 2 && (pMode->Flags & V_DBLCLK))
		  pMode->Private[S3_EARLY_SC] = 1;
	       else
		  pMode->Private[S3_EARLY_SC] = 0;
	    } else if (DAC_IS_TI3026 
		       && OFLG_ISSET(CLOCK_OPTION_ICD2061A,
				     &s3InfoRec.clockOptions)) {
	       if (s3Bpp == 2 && (pMode->Flags & V_DBLCLK))
		  pMode->Private[S3_EARLY_SC] = 1;
	       else
		  pMode->Private[S3_EARLY_SC] = 0;
	    } else if (DAC_IS_IBMRGB) {
	       if (s3BiosVendor == GENOA_BIOS) {
	          pMode->Private[S3_EARLY_SC] = 0;
	       }
	       else if (s3BiosVendor == STB_BIOS) {
		  if (s3Bpp == 2 && s3InfoRec.clock[pMode->Clock] > 125000)
		     pMode->Private[S3_EARLY_SC] = 0;
		  else if (s3Bpp == 4)
		     pMode->Private[S3_EARLY_SC] = 0;
		  else 
		     pMode->Private[S3_EARLY_SC] = 1;
	       }
	       else if (s3BiosVendor == HERCULES_BIOS) {
		 if (S3_968_SERIES(s3ChipId))
		   pMode->Private[S3_EARLY_SC] = 0;
		 else
		   pMode->Private[S3_EARLY_SC] = 0;
	       }
	       else
	          pMode->Private[S3_EARLY_SC] = 0;
	    } else {
	       pMode->Private[S3_EARLY_SC] = 0;
	    }
    	}
    }

   return MODE_OK;
}
