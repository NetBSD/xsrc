/* $XConsortium: tga.c /main/11 1996/10/28 04:23:31 kaleb $ */
/*
 * Copyright 1995,96 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/tga/tga.c,v 3.17.2.10 1999/06/18 13:08:21 hohndel Exp $ */

#include "X.h"
#include "input.h"
#include "screenint.h"
#include "dix.h"
#include "cfb.h"
#include "cfb32.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "tga.h"
#include "tga_presets.h"

#include "xf86xaa.h"
#include "xf86scrin.h"

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#ifdef DPMSExtension
#include "opaque.h"
#include "extensions/dpms.h"
#endif

extern int defaultColorVisualClass;

static int tgaValidMode(
#if NeedFunctionPrototypes
    DisplayModePtr,
    Bool,
    int
#endif
);

ScrnInfoRec tgaInfoRec = {
    FALSE,		/* Bool configured */
    -1,			/* int tmpIndex */
    -1,			/* int scrnIndex */
    tgaProbe,      	/* Bool (* Probe)() */
    tgaInitialize,	/* Bool (* Init)() */
    tgaValidMode,	/* Bool (* ValidMode)() */
    tgaEnterLeaveVT,	/* void (* EnterLeaveVT)() */
    (void (*)())NoopDDA,/* void (* EnterLeaveMonitor)() */
    (void (*)())NoopDDA,/* void (* EnterLeaveCursor)() */
    tgaAdjustFrame,	/* void (* AdjustFrame)() */
    tgaSwitchMode,	/* Bool (* SwitchMode)() */
    tgaDPMSSet,		/* void (* DPMSSet)() */
    tgaPrintIdent,	/* void (* PrintIdent)() */
    8,			/* int depth */
    {5, 6, 5},          /* xrgb weight */
    8,			/* int bitsPerPixel */
    PseudoColor,       	/* int defaultVisual */
    -1, -1,		/* int virtualX,virtualY */
    -1,                 /* int displayWidth */
    -1, -1, -1, -1,	/* int frameX0, frameY0, frameX1, frameY1 */
    {0, },	       	/* OFlagSet options */
    {0, },	       	/* OFlagSet clockOptions */
    {0, },	       	/* OFlagSet xconfigFlag */
    NULL,	       	/* char *chipset */
    NULL,	       	/* char *ramdac */
    {0, 0, 0, 0},	/* int dacSpeeds[MAXDACSPEEDS] */
    0,			/* int dacSpeedBpp */
    0,			/* int clocks */
    {0, },		/* int clock[MAXCLOCKS] */
    0,			/* int maxClock */
    0,			/* int videoRam */
    0, 		        /* int BIOSbase */   
    0,			/* unsigned long MemBase */
    240, 180,		/* int width, height */
    0,                  /* unsigned long  speedup */
    NULL,	       	/* DisplayModePtr modes */
    NULL,	       	/* MonPtr monitor */
    NULL,               /* char *clockprog */
    -1,                 /* int textclock */   
    FALSE,              /* Bool bankedMono */
    "DEC_TGA",          /* char *name */
    {0, },		/* xrgb blackColour */
    {0, },		/* xrgb whiteColour */
    tgaValidTokens,	/* int *validTokens */
    TGA_PATCHLEVEL,	/* char *patchlevel */
    0,			/* int IObase */
    0,			/* int PALbase */
    0,			/* int COPbase */
    0,			/* int POSbase */
    0,			/* int instance */
    0,			/* int s3Madjust */
    0,			/* int s3Nadjust */
    0,			/* int s3MClk */
    0,			/* int chipID */
    0,			/* int chipRev */
    0,			/* unsigned long VGAbase */
    0,			/* int s3RefClk */
    -1,			/* int s3BlankDelay */
    0,			/* int textClockFreq */
    NULL,               /* char* DCConfig */
    NULL,               /* char* DCOptions */
    0,			/* int MemClk */
    0			/* int LCDClk */

#ifdef XFreeXDGA
    ,0,			/* int directMode */
    NULL,		/* Set Vid Page */
    0,			/* unsigned long physBase */
    0			/* int physSize */
#endif
};

extern miPointerScreenFuncRec xf86PointerScreenFuncs;
extern Bool xf86Exiting, xf86Resetting, xf86ProbeFailed;
ScreenPtr savepScreen = NULL;
Bool tgaDAC8Bit = FALSE;
Bool tgaDACSyncOnGreen = FALSE;
Bool tgaBt485PixMux = FALSE;
Bool tgaReloadCursor, tgaBlockCursor;
unsigned char tgaSwapBits[256];
pointer tga_reg_base;
int tgahotX, tgahotY;
static PixmapPtr ppix = NULL;
int tga_type;
int tgaDisplayWidth;
pointer tgaVideoMem = NULL;
pointer tgaCursorMem = NULL;
extern unsigned char *tgaVideoMemSave;
tgaCRTCRegRec tgaCRTCRegs;
volatile unsigned long *VidBase;
#define tgaReorderSwapBits(a,b)		b = \
		(a & 0x80) >> 7 | \
		(a & 0x40) >> 5 | \
		(a & 0x20) >> 3 | \
		(a & 0x10) >> 1 | \
		(a & 0x08) << 1 | \
		(a & 0x04) << 3 | \
		(a & 0x02) << 5 | \
		(a & 0x01) << 7;

/*
 * tgaPrintIdent
 */

void
tgaPrintIdent()
{
	ErrorF("  %s: non-accelerated server for DEC 21030 TGA graphics adapters\n",
			tgaInfoRec.name);
	ErrorF("(Patchlevel %s)\n", tgaInfoRec.patchLevel);
}


/*
 * ICS1562ClockSelect --
 *      select one of the possible clocks ...
 */

Bool
ICS1562ClockSelect(freq)
     int freq;
{
  unsigned char pll_bits[7];
  unsigned long temp;
  int i, j;

  /*
   * For the DEC 21030 TGA, There lies an ICS1562 Clock Generator.
   * This requires the 55 clock bits be written in a serial manner to
   * bit 0 of the CLOCK register and on the 56th bit set the hold flag.
   */
  switch(freq)
  {
    case CLK_REG_SAVE:
      /* The clock register is a write only register */
      break;
    case CLK_REG_RESTORE:
      /* Therefore we don't know what the value is to read and restore */
      break;
    default:
      ICS1562_CalcClockBits(freq, pll_bits);
      for (i = 0;i <= 6; i++) {
	for (j = 0; j <= 7; j++) {
	  temp = (pll_bits[i] >> (7-j)) & 1;
	  if (i == 6 && j == 7)
	    temp |= 2;
	  TGA_WRITE_REG(temp, TGA_CLOCK_REG);
	}
      }
  }
  return(TRUE);
}

/*
 * tgaProbe --
 *      check up whether a TGA based board is installed
 */

Bool
tgaProbe()
{
  int i;
  Bool pModeInited = FALSE;
  pointer Base;
  DisplayModePtr pMode, pEnd;
  OFlagSet validOptions;
  pciConfigPtr pcrp, *pcrpp;

#define TGA_DEVICE_ID1 0x00041011

  pcrpp = xf86scanpci(tgaInfoRec.scrnIndex);

  if (!pcrpp)
	return(FALSE);

  i = 0;
  while ((pcrp = pcrpp[i]) != (pciConfigPtr)NULL) {
	if ((pcrp->_device_vendor == TGA_DEVICE_ID1) &&
	    (pcrp->_command & PCI_CMD_IO_ENABLE) &&
	    (pcrp->_command & PCI_CMD_MEM_ENABLE))
		break;
	i++;
  }

  if (!pcrp)
	return(FALSE);

  if (tgaInfoRec.MemBase == 0)
      tgaInfoRec.MemBase = pcrp->_base0 & 0xFFFFFF00;

  Base = xf86MapVidMem(0,EXTENDED_REGION,(pointer)tgaInfoRec.MemBase,2097152);

  tga_reg_base = (pointer *)((char*)(Base) + TGA_REGS_OFFSET);
  tga_type = (*(unsigned int *)Base >> 12) & 0xf;

  /* Let's find out what kind of TGA chip we've got ! */
  /* We only support the 8 plane TGA with BT485 Ramdac - so there ! */
  switch (tga_type)
  {
	case TYPE_TGA_8PLANE:
		ErrorF("%s %s: DEC 21030 TGA 8 Plane Chip Found.\n",
			XCONFIG_PROBED, tgaInfoRec.name);
		break;
	case TYPE_TGA_24PLANE:
		ErrorF("%s %s: DEC 21030 TGA 24 Plane Chip Found.\n",
			XCONFIG_PROBED, tgaInfoRec.name);
		break;
	case TYPE_TGA_24PLUSZ:
		ErrorF("%s %s: DEC 21030 TGA 24 Plane 3D Chip Found.\n",
			XCONFIG_PROBED, tgaInfoRec.name);
		break;
	default:
		ErrorF("%s %s: OUCH ! Found an unknown TGA Chip. Aborting..\n",
			XCONFIG_PROBED, tgaInfoRec.name);
		return(FALSE);
		break;
  }

  if (tgaInfoRec.videoRam == 0)
  {
	switch (tga_type) {
	case TYPE_TGA_8PLANE:
		tgaInfoRec.videoRam = 2*1024;
		break;
	case TYPE_TGA_24PLANE:
		tgaInfoRec.videoRam = 8*1024;
		break;
	case TYPE_TGA_24PLUSZ:
		tgaInfoRec.videoRam = 16*1024;
		break;
	}
  	ErrorF("%s %s: videoram : %dk\n", XCONFIG_PROBED, tgaInfoRec.name,
		tgaInfoRec.videoRam);
  }
  else
  {
  	ErrorF("%s %s: videoram : %dk\n", XCONFIG_GIVEN, tgaInfoRec.name,
		tgaInfoRec.videoRam);
  }

  /* There is an algorithm for the ICS 1562 */
  OFLG_SET(CLOCK_OPTION_ICS1562, &tgaInfoRec.clockOptions);
  OFLG_SET(CLOCK_OPTION_PROGRAMABLE, &tgaInfoRec.clockOptions);
  ErrorF("%s %s: Using ICS1562 programmable clock\n", 
	 XCONFIG_PROBED, tgaInfoRec.name);


  /* Initialize options that reflect the TGA */
  OFLG_ZERO(&validOptions);

#ifdef NOTYET
  /* According to the 21030 manual - The Cursor of the 21030 is latched
   * through to the RAMDAC's own cursor, so it may be that both of these
   * are the same, but obviously we've got different methods of accessing
   * them. I guess that the UDB(Multia) has these latches.....
   */
  if (tga_type != TYPE_TGA_8PLANE)
	/* Use BT463 Ramdac cursor, utilizing 24Plane/24Plane3d chip */
  	OFLG_SET(OPTION_HW_CURSOR, &validOptions);
  else
  	/* Or - If we have an 8plane use BT485 HW cursor directly */
  	OFLG_SET(OPTION_BT485_CURS, &validOptions);
#endif

  OFLG_SET(OPTION_DAC_8_BIT, &validOptions);
  OFLG_SET(OPTION_DAC_8_BIT, &tgaInfoRec.options); /* Set 8bit by default */
  OFLG_SET(OPTION_DAC_6_BIT, &validOptions);
/*  OFLG_SET(OPTION_SYNC_ON_GREEN, &validOptions); */
  OFLG_SET(OPTION_POWER_SAVER, &validOptions);

  xf86VerifyOptions(&validOptions, &tgaInfoRec);
  tgaInfoRec.chipset = "tga";
  xf86ProbeFailed = FALSE;

  tgaInfoRec.dacSpeeds[0] = 135000; 	/* 135MHz for the Bt485 */
  tgaInfoRec.maxClock = 135000;		/* 135MHz for the Bt485 */

  /* Let's grab the basic mode lines */
#ifdef NOTYET
  tx = tgaInfoRec.virtualX;
  ty = tgaInfoRec.virtualY;
#else
  if (tgaInfoRec.virtualX > 0) {
	ErrorF("%s %s: Virtual coordinates - Not yet supported. "
		   "Ignoring.\n", XCONFIG_GIVEN, tgaInfoRec.name);
  }
#endif
  pMode = tgaInfoRec.modes;
  if (pMode == NULL)
  { 
	ErrorF("No modes specified in the XF86Config file.\n");
	return (FALSE);
  }
  pEnd = (DisplayModePtr)NULL;
  do
  {
	DisplayModePtr pModeSv;

	pModeSv = pMode->next;
	
	/* Delete any invalid ones */
	if (xf86LookupMode(pMode, &tgaInfoRec, LOOKUP_DEFAULT) == FALSE) {
		pModeSv = pMode->next;
		xf86DeleteMode(&tgaInfoRec, pMode);
		pMode = pModeSv;
#ifdef NOTYET
	} else if (((tx > 0) && (pMode->HDisplay > tx)) ||
		   ((ty > 0) && (pMode->VDisplay > ty))) {
		pModeSv = pMode->next;
		ErrorF("%s %s: Resolution %dx%d too large for virtual %dx%d\n",
			XCONFIG_PROBED, tgaInfoRec.name,
			pMode->HDisplay, pMode->VDisplay, tx, ty);
		xf86DeleteMode(&tgaInfoRec, pMode);
		pMode = pModeSv;
#endif
	} else {
		if (pEnd == (DisplayModePtr) NULL)
			pEnd = pMode;

#ifdef NOTYET
		tgaInfoRec.virtualX = max(tgaInfoRec.virtualX,
						pMode->HDisplay);
		tgaInfoRec.virtualY = max(tgaInfoRec.virtualY,
						pMode->VDisplay);
#else
		if (pMode->HDisplay % 4)
		{
			pModeSv = pMode->next;
			ErrorF("%s %s: Horizontal Resolution %d not divisible"
				" by a factor of 4, removing modeline.\n",
				XCONFIG_GIVEN, tgaInfoRec.name,
				pMode->HDisplay);
			xf86DeleteMode(&tgaInfoRec, pMode);
			pMode = pModeSv;
		}
		else
		{
			tgaInfoRec.virtualX = pMode->HDisplay;
			tgaInfoRec.virtualY = pMode->VDisplay;
			pModeInited = TRUE; /* We have a mode - only 1 supported */
		}
#endif
		pMode = pMode->next;
	}
  } while (pModeInited == FALSE); /* (pMode != pEnd); */

  ErrorF("%s %s: TGA chipset currently only supports one modeline.\n",
		XCONFIG_PROBED, tgaInfoRec.name);

  tgaInfoRec.displayWidth = tgaInfoRec.virtualX;

  if (OFLG_ISSET(OPTION_DAC_8_BIT, &tgaInfoRec.options))
	tgaDAC8Bit = TRUE;

  if (OFLG_ISSET(OPTION_DAC_6_BIT, &tgaInfoRec.options))
	tgaDAC8Bit = FALSE;

#ifdef NOTYET
  if (OFLG_ISSET(OPTION_SYNC_ON_GREEN, &tgaInfoRec.options)) {
	tgaDACSyncOnGreen = TRUE;
	ErrorF("%s %s: Putting RAMDAC into sync-on-green mode\n",
		XCONFIG_GIVEN, tgaInfoRec.name);
  }
#endif

#ifdef DPMSExtension
  if (DPMSEnabledSwitch ||
      (OFLG_ISSET(OPTION_POWER_SAVER, &tgaInfoRec.options) &&
       !DPMSDisabledSwitch))
	defaultDPMSEnabled = DPMSEnabled = TRUE;
#endif

  if (xf86bpp < 0)
	xf86bpp = tgaInfoRec.depth;
  if (xf86weight.red == 0 || xf86weight.green == 0 || xf86weight.blue == 0)
	xf86weight = tgaInfoRec.weight;

  if ((tga_type == 0) && (xf86bpp>8)) {
    ErrorF("Invalid value for bpp. Only 8bpp is supported on 8-plane TGA.\n");
    return(FALSE);
  }
  if ((tga_type != 0) && ((xf86bpp==8) || (xf86bpp==16))) {
    ErrorF("Invalid value for bpp. Only 24bpp is supported on 24-plane TGA.\n");
    return(FALSE);
  }
  switch (xf86bpp) {
  case 8:
    /* XAA uses this */
    xf86weight.green = (tgaDAC8Bit ? 8 : 6);
    break;
#ifdef NOTYET
  case 16:
    if (xf86weight.red==5 && xf86weight.green==5 && xf86weight.blue==5) {
      tgaInfoRec.depth = 15;
    } else if (xf86weight.red==5 &&
	       xf86weight.green==6 && xf86weight.blue==5) {
      tgaInfoRec.depth = 16;
    } else {
      ErrorF(
	"Invalid color weighting %1d%1d%1d (only 555 and 565 are valid)\n",
	xf86weight.red,xf86weight.green,xf86weight.blue);
      return(FALSE);
    }
    tgaInfoRec.bitsPerPixel = 16;
    if (tgaInfoRec.defaultVisual < 0)
      tgaInfoRec.defaultVisual = TrueColor;
    if (defaultColorVisualClass < 0)
      defaultColorVisualClass = tgaInfoRec.defaultVisual;
    break;
#endif
  case 24:
    xf86weight.red =  xf86weight.green = xf86weight.blue = 8;
    tgaInfoRec.depth = 24;
    tgaInfoRec.bitsPerPixel = 32;
    if (tgaInfoRec.defaultVisual < 0)
      tgaInfoRec.defaultVisual = TrueColor;
    if (defaultColorVisualClass < 0)
      defaultColorVisualClass = tgaInfoRec.defaultVisual;
    break;
  default:
         ErrorF("Invalid value for bpp.  Valid values are 8, 16, 24 and 32.\n");
         return(FALSE);
  }

#if 0
  if (tga_type == 0) { /* 8-plane */
    switch (xf86bpp) {
    case 8:
      /* XAA uses this */
      xf86weight.green = (tgaDAC8Bit ? 8 : 6);
      break;
    default:
      ErrorF("Invalid value for bpp. 8bpp is only supported.\n");
      return(FALSE);
    }
  } else {
    switch (xf86bpp) {
    case 32:
      /* XAA uses this */
      xf86weight.red = xf86weight.green = xf86weight.blue = 8;
      break;
    default:
      ErrorF("Invalid value for bpp. 32bpp is only supported.\n");
      return(FALSE);
    }
  }
#endif

#ifdef XFreeXDGA
#ifdef NOTYET
  tgaInfoRec.directMode = XF86DGADirectPresent;
#endif
#endif

  return(TRUE);
}

/*
 * tgaInitialize --
 *
 */

Bool
tgaInitialize (scr_index, pScreen, argc, argv)
	int		scr_index;
	ScreenPtr	pScreen;
	int		argc;
	char		**argv;
{
	int displayResolution = 75; 	/* default to 75dpi */
	int i;
	extern int monitorResolution;
	Bool (*ScreenInitFunc)(register ScreenPtr, pointer, int, int, int, int, int);

	/* Init the screen */
	
	tgaInitAperture(scr_index);
	tgaInit(tgaInfoRec.modes);
	tgaCalcCRTCRegs(&tgaCRTCRegs, tgaInfoRec.modes);
	tgaSetCRTCRegs(&tgaCRTCRegs);
	tgaInitEnvironment();
	for (i = 0; i < 256; i++)
	{ 
		tgaReorderSwapBits(i, tgaSwapBits[i]);
	}

	/*
	 * Take display resolution from the -dpi flag 
	 */
	if (monitorResolution)
		displayResolution = monitorResolution;
	
#if 1
	/* Let's use the new XAA Architecture.....*/
 	TGAAccelInit();

	switch (tgaInfoRec.bitsPerPixel) {
	case 8:
	  ScreenInitFunc = &xf86XAAScreenInit8bpp;
	  break;
	case 16:
	  ScreenInitFunc = &xf86XAAScreenInit16bpp;
	  break;
	case 32:
	  ScreenInitFunc = &xf86XAAScreenInit32bpp;
	  break;
	}

	if (!ScreenInitFunc(pScreen,
#else
	if (!cfbScreenInit(pScreen,
#endif
			(pointer) tgaVideoMem,
			tgaInfoRec.virtualX, tgaInfoRec.virtualY,
			displayResolution, displayResolution,
			tgaInfoRec.displayWidth))
		return(FALSE);

	pScreen->whitePixel = (Pixel) 1;
	pScreen->blackPixel = (Pixel) 0;
	XF86FLIP_PIXELS();
	pScreen->CloseScreen = tgaCloseScreen;
	pScreen->SaveScreen = tgaSaveScreen;

	switch(tgaInfoRec.bitsPerPixel) {
		case 8:
			pScreen->InstallColormap = tgaInstallColormap;
			pScreen->UninstallColormap = tgaUninstallColormap;
			pScreen->ListInstalledColormaps = 
						tgaListInstalledColormaps;
			pScreen->StoreColors = tgaStoreColors;
			break;
		case 16:
		case 32:
			pScreen->InstallColormap = cfbInstallColormap;
			pScreen->UninstallColormap = cfbUninstallColormap;
			pScreen->ListInstalledColormaps = 
						cfbListInstalledColormaps;
			pScreen->StoreColors = (void (*)())NoopDDA;
			break;
	}

	if ( /* (OFLG_ISSET(OPTION_HW_CURSOR, &tgaInfoRec.options)) || */
	     (OFLG_ISSET(OPTION_BT485_CURS, &tgaInfoRec.options)) ) {
		pScreen->QueryBestSize = tgaQueryBestSize;
		xf86PointerScreenFuncs.WarpCursor = tgaWarpCursor;
		(void)tgaCursorInit(0, pScreen);
	} 
	else
	{
		miDCInitialize (pScreen, &xf86PointerScreenFuncs);
	}

	savepScreen = pScreen;
	return (cfbCreateDefColormap(pScreen));
}

/*
 *      Assign a new serial number to the window.
 *      Used to force GC validation on VT switch.
 */

/*ARGSUSED*/
static int
tgaNewSerialNumber(pWin, data)
    WindowPtr pWin;
    pointer data;
{
    pWin->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    return WT_WALKCHILDREN;
}


/*
 * tgaEnterLeaveVT -- 
 *      grab/ungrab the current VT completely.
 */

void
tgaEnterLeaveVT(enter, screen_idx)
     Bool enter;
     int screen_idx;
{
    PixmapPtr pspix;
    ScreenPtr pScreen = savepScreen;

    if (!xf86Exiting && !xf86Resetting) {
        switch (tgaInfoRec.bitsPerPixel) {
        case 8:
            pspix = (PixmapPtr)pScreen->devPrivate;
            break;
        case 32:
	    pspix = (PixmapPtr)pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr;
            break;
        }
    }

    if (pScreen && !xf86Exiting && !xf86Resetting)
        WalkTree(pScreen, tgaNewSerialNumber, 0);

    if (enter) {
	xf86MapDisplay(screen_idx, LINEAR_REGION);
	if (!xf86Resetting) {
	    ScrnInfoPtr pScr = (ScrnInfoPtr)XF86SCRNINFO(pScreen);

	    tgaCalcCRTCRegs(&tgaCRTCRegs, tgaInfoRec.modes);
	    tgaSetCRTCRegs(&tgaCRTCRegs);
	    tgaInit(tgaInfoRec.modes);
	    tgaInitEnvironment();
	    tgaRestoreDACvalues();
	    tgaAdjustFrame(pScr->frameX0, pScr->frameY0);
	    tgaRestoreCursor(pScreen);

	    if (pspix->devPrivate.ptr != tgaVideoMem && ppix) {
		pspix->devPrivate.ptr = tgaVideoMem;
#if 1
		ppix->devPrivate.ptr = (pointer)tgaVideoMem;
#else
		(tgaImageWriteFunc)(0, 0, pScreen->width, pScreen->height,
				 ppix->devPrivate.ptr,
				 PixmapBytePad(pScreen->width,
					       pScreen->rootDepth),
				 0, 0, MIX_SRC, ~0);
#endif
	    }
	}
	if (ppix) {
	    (pScreen->DestroyPixmap)(ppix);
	    ppix = NULL;
	}
    } else {
	xf86MapDisplay(screen_idx, LINEAR_REGION);
	if (!xf86Exiting) {
	    ppix = (pScreen->CreatePixmap)(pScreen, tgaInfoRec.displayWidth,
					    pScreen->height,
					    pScreen->rootDepth);

	    if (ppix) {
#if 1
		ppix->devPrivate.ptr = (pointer)tgaVideoMemSave;
#else
		(tgaImageReadFunc)(0, 0, pScreen->width, pScreen->height,
				ppix->devPrivate.ptr,
				PixmapBytePad(pScreen->width,
					      pScreen->rootDepth),
				0, 0, ~0);
#endif
		pspix->devPrivate.ptr = ppix->devPrivate.ptr;
	    }
	}

	xf86InvalidatePixmapCache();

	if (!xf86Resetting) {
#ifdef XFreeXDGA
	    if (!(tgaInfoRec.directMode & XF86DGADirectGraphics))
#endif
		tgaCleanUp();
	}
	xf86UnMapDisplay(screen_idx, LINEAR_REGION);
    }
}

/*
 * tgaCloseScreen --
 *      called to ensure video is enabled when server exits.
 */

/*ARGSUSED*/
Bool
tgaCloseScreen(screen_idx, pScreen)
    int        screen_idx;
    ScreenPtr  pScreen;
{
    extern void tgaClearSavedCursor();

    /*
     * Hmm... The server may shut down even if it is not running on the
     * current vt. Let's catch this case here.
     */
    xf86Exiting = TRUE;
    if (xf86VTSema)
	tgaEnterLeaveVT(LEAVE, screen_idx);
    else if (ppix) {
    /* 7-Jan-94 CEG: The server is not running on the current vt.
     * Free the screen snapshot taken when the server vt was left.
     */
	    (savepScreen->DestroyPixmap)(ppix);
	    ppix = NULL;
    }

#ifdef NOTYET
    tgaClearSavedCursor(screen_idx);
#endif

    switch (tgaInfoRec.bitsPerPixel) {
    case 8:
        cfbCloseScreen(screen_idx, savepScreen);
	break;
    case 32:
        cfb32CloseScreen(screen_idx, savepScreen);
	break;
    }

    savepScreen = NULL;
    return(TRUE);
}

/*
 * tgaSaveScreen --
 *      blank the screen.
 */
Bool
tgaSaveScreen (pScreen, on)
     ScreenPtr     pScreen;
     Bool          on;
{
    if (on)
	SetTimeSinceLastInputEvent();

    if (xf86VTSema) {
        if (on) {
           TGA_WRITE_REG(0x01, TGA_VALID_REG); /* SCANNING */
        } else {
           TGA_WRITE_REG(0x03, TGA_VALID_REG); /* SCANNING and BLANK */    
        }
    }

    return(TRUE);
}

/*
 * tgaDPMSSet -- Sets VESA Display Power Management Signaling (DPMS) Mode
 *
 * Only the Off and On modes are currently supported.
 */

void
tgaDPMSSet(PowerManagementMode)
    int PowerManagementMode;
{
#ifdef DPMSExtension
    int crtcGenCntl;
    if (!xf86VTSema) return;
    crtcGenCntl = TGA_READ_REG(TGA_VALID_REG);
    switch (PowerManagementMode)
    {
    case DPMSModeOn:
	/* HSync: On, VSync: On */
	crtcGenCntl |= 0x0001;
	break;
    case DPMSModeStandby:
	/* HSync: Off, VSync: On -- Not Supported */
	break;
    case DPMSModeSuspend:
	/* HSync: On, VSync: Off -- Not Supported */
	break;
    case DPMSModeOff:
	/* HSync: Off, VSync: Off */
	crtcGenCntl &= 0xFFFE;
	break;
    }
    usleep(10000);
    TGA_WRITE_REG(crtcGenCntl, TGA_VALID_REG);
#endif
}

/*
 * tgaAdjustFrame --
 *      Modify the CRT_OFFSET for panning the display.
 */
void
tgaAdjustFrame(x, y)
    int x, y;
{
	/* TGA needs much work for this to happen */
}

/*
 * tgaSwitchMode --
 *      Reinitialize the CRTC registers for the new mode.
 */
Bool
tgaSwitchMode(mode)
    DisplayModePtr mode;
{
    tgaCalcCRTCRegs(&tgaCRTCRegs, mode);
    tgaSetCRTCRegs(&tgaCRTCRegs);

    return(TRUE);
}

/*
 * tgaValidMode --
 *
 */
static int
tgaValidMode(mode, verbose, flag)
    DisplayModePtr mode;
    Bool verbose;
    int flag;
{
    if (mode->Flags & V_INTERLACE)
    {
	ErrorF("%s %s: Cannot support interlaced modes, deleting.\n",
			XCONFIG_GIVEN, tgaInfoRec.name);
	return MODE_BAD;
    }
    return MODE_OK;
}
