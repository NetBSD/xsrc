/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vga/generic.c,v 1.48 2000/12/02 15:31:00 tsi Exp $ */
/*
 * Copyright (C) 1998 The XFree86 Project, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 */

/*
 * This is essentially a merge of two different drivers:  a VGA planar driver
 * (originally by David Dawes <dawes@xfree86.org>) and a 256-colour VGA driver
 * by Harm Hanemaayer <hhanemaa@cs.ruu.nl>.
 *
 * The port of this driver to XFree86 4.0 was done by:
 * David Dawes <dawes@xfree86.org>
 * Dirk H. Hohndel <hohndel@xfree86.org>
 * Marc Aurele La France <tsi@ualberta.ca>
 */

#define DEBUG(x) 
#include "xf86.h"
#include "xf86_ansic.h"
#include "compiler.h"
#include "vgaHW.h"
#include "xf86PciInfo.h"

#undef  PSZ
#define PSZ 8
#include "cfb.h"
#undef  PSZ

#include "xf4bpp.h"
#include "xf1bpp.h"

#include "shadowfb.h"

#include "mipointer.h"
#include "micmap.h"

#include "xf86RAC.h"
#include "xf86Resources.h"

/* Some systems define VGA for their own purposes */
#undef VGA

/* A few things all drivers should have */
#define VGA_NAME            "VGA"
#define VGA_DRIVER_NAME     "vga"
#define VGA_VERSION_NAME    "4.0"
#define VGA_VERSION_MAJOR   4
#define VGA_VERSION_MINOR   0
#define VGA_PATCHLEVEL	    0
#define VGA_VERSION_CURRENT ((VGA_VERSION_MAJOR << 24) | \
			     (VGA_VERSION_MINOR << 16) | VGA_PATCHLEVEL)


/* Forward definitions */
static OptionInfoPtr	  GenericAvailableOptions(int chipid, int busid);
static void       GenericIdentify(int);
static Bool       GenericProbe(DriverPtr, int);
static Bool       GenericPreInit(ScrnInfoPtr, int);
static Bool       GenericScreenInit(int, ScreenPtr, int, char **);
static Bool       GenericSwitchMode(int, DisplayModePtr, int);
static void       GenericAdjustFrame(int, int, int, int);
static Bool       GenericEnterVT(int, int);
static void       GenericLeaveVT(int, int);
static void       GenericFreeScreen(int, int);
static int        VGAFindIsaDevice(GDevPtr dev);

static int GenericValidMode(int, DisplayModePtr, Bool, int);

/* The root of all evil... */
DriverRec VGA =
{
    VGA_VERSION_CURRENT,
    VGA_DRIVER_NAME,
#if 0
    "Generic VGA driver",
#endif
    GenericIdentify,
    GenericProbe,
    GenericAvailableOptions,
    NULL,
    0
};

typedef enum {
    OPTION_SHADOW_FB,
    OPTION_VGA_CLOCKS
} GenericOpts;

static OptionInfoRec GenericOptions[] = {
    { OPTION_SHADOW_FB,         "ShadowFB",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_VGA_CLOCKS,        "VGAClocks",    OPTV_BOOLEAN,   {0}, FALSE },
    { -1,                       NULL,           OPTV_NONE,      {0}, FALSE }
};

static const char *vgahwSymbols[] = {
    "vgaHWGetHWRec",
    "vgaHWUnlock",
    "vgaHWInit",
    "vgaHWProtect",
    "vgaHWGetIOBase",
    "vgaHWMapMem",
    "vgaHWLock",
    "vgaHWFreeHWRec",
    "vgaHWSaveScreen",
    NULL
};

static const char *fbSymbols[] = {
    "xf1bppScreenInit",
    "xf4bppScreenInit",
    "cfbScreenInit",
    "mfbScreenInit",
    NULL
};

static const char *shadowfbSymbols[] = {
    "ShadowFBInit",
    NULL
};

#ifdef XFree86LOADER

/* Module loader interface */

static MODULESETUPPROTO(GenericSetup);

static XF86ModuleVersionInfo GenericVersionRec =
{
    VGA_DRIVER_NAME,
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    VGA_VERSION_MAJOR, VGA_VERSION_MINOR, VGA_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0, 0, 0, 0}
};

/*
 * This data is accessed by the loader.  The name must be the module name
 * followed by "ModuleInit".
 */
XF86ModuleData vgaModuleData = { &GenericVersionRec, GenericSetup, NULL };

static pointer
GenericSetup(pointer Module, pointer Options, int *ErrorMajor, int *ErrorMinor)
{
    static Bool Initialised = FALSE;

    if (!Initialised)
    {
        Initialised = TRUE;
        xf86AddDriver(&VGA, Module, 0);
	LoaderRefSymLists(vgahwSymbols, fbSymbols, shadowfbSymbols,
			  NULL);
        return (pointer)TRUE;
    }

    if (ErrorMajor)
        *ErrorMajor = LDR_ONCEONLY;
    return NULL;
}

#endif


enum GenericTypes
{
    CHIP_VGA_GENERIC
};

/* Supported chipsets */
static SymTabRec GenericChipsets[] =
{
    {CHIP_VGA_GENERIC, "generic"},
    {-1,               NULL}
};

static PciChipsets GenericPCIchipsets[] = {
  { CHIP_VGA_GENERIC, PCI_CHIP_VGA, RES_SHARED_VGA },
  { -1,               -1,           RES_UNDEFINED },
};

static IsaChipsets GenericISAchipsets[] = {
  {CHIP_VGA_GENERIC, RES_EXCLUSIVE_VGA},
  {-1,                0 }
};

static void
GenericIdentify(int flags)
{
    xf86PrintChipsets(VGA_NAME,
        "Generic VGA driver (version " VGA_VERSION_NAME ") for chipsets",
        GenericChipsets);
}

static
OptionInfoPtr
GenericAvailableOptions(int chipid, int busid)
{
    return GenericOptions;
}

/*
 * This function is called once, at the start of the first server generation to
 * do a minimal probe for supported hardware.
 */

static Bool
GenericProbe(DriverPtr drv, int flags)
{
    Bool foundScreen = FALSE;
    int numDevSections, numUsed;
    GDevPtr *devSections;
    int *usedChips;
    int i;

    /*
     * Find the config file Device sections that match this
     * driver, and return if there are none.
     */
    if ((numDevSections = xf86MatchDevice(VGA_NAME,
					  &devSections)) <= 0) {
	return FALSE;
    }
  
    /* PCI BUS */
    if (xf86GetPciVideoInfo() ) {
	numUsed = xf86MatchPciInstances(VGA_NAME, PCI_VENDOR_GENERIC,
					GenericChipsets, GenericPCIchipsets, 
					devSections,numDevSections,
					drv, &usedChips);
	if (numUsed > 0) {
	    if (flags & PROBE_DETECT)
		foundScreen = TRUE;
	    else {
		for (i = 0; i < numUsed; i++) {
		    ScrnInfoPtr pScrn = NULL;
		    /* Allocate a ScrnInfoRec  */
		    if ((pScrn = xf86ConfigPciEntity(pScrn,0,usedChips[i],
							   GenericPCIchipsets,NULL,
							   NULL,NULL,NULL,NULL))){
			pScrn->driverVersion = VGA_VERSION_CURRENT;
			pScrn->driverName    = VGA_DRIVER_NAME;
			pScrn->name          = VGA_NAME;
			pScrn->Probe         = GenericProbe;
			pScrn->PreInit       = GenericPreInit;
			pScrn->ScreenInit    = GenericScreenInit;
			pScrn->SwitchMode    = GenericSwitchMode;
			pScrn->AdjustFrame   = GenericAdjustFrame;
			pScrn->EnterVT       = GenericEnterVT;
			pScrn->LeaveVT       = GenericLeaveVT;
			pScrn->FreeScreen    = GenericFreeScreen;
			pScrn->ValidMode     = GenericValidMode;
			foundScreen = TRUE;
		    }
		}
	    }
	    xfree(usedChips);
	}
    }
    
    /* Isa Bus */
    numUsed = xf86MatchIsaInstances(VGA_NAME,GenericChipsets,
				     GenericISAchipsets,drv,
				     VGAFindIsaDevice,devSections,
				     numDevSections,&usedChips);
    if(numUsed > 0) {
	if (flags & PROBE_DETECT)
	    foundScreen = TRUE;
	else for (i = 0; i < numUsed; i++) {
	    ScrnInfoPtr pScrn = NULL;
	    if ((pScrn = xf86ConfigIsaEntity(pScrn,0,usedChips[i],
						   GenericISAchipsets,NULL,
						   NULL,NULL,NULL,NULL))) {
	    
		pScrn->driverVersion = VGA_VERSION_CURRENT;
		pScrn->driverName    = VGA_DRIVER_NAME;
		pScrn->name          = VGA_NAME;
		pScrn->Probe         = GenericProbe;
		pScrn->PreInit       = GenericPreInit;
		pScrn->ScreenInit    = GenericScreenInit;
		pScrn->SwitchMode    = GenericSwitchMode;
		pScrn->AdjustFrame   = GenericAdjustFrame;
		pScrn->EnterVT       = GenericEnterVT;
		pScrn->LeaveVT       = GenericLeaveVT;
		pScrn->FreeScreen    = GenericFreeScreen;
		pScrn->ValidMode     = GenericValidMode;
		foundScreen = TRUE;
	    }
	}
	xfree(usedChips);
    }

    xfree(devSections);
    return foundScreen;
}

static int
VGAFindIsaDevice(GDevPtr dev)
{
#ifndef PC98_EGC
    CARD16 GenericIOBase = VGAHW_GET_IOBASE();
    CARD8 CurrentValue, TestValue;
    
    /* There's no need to unlock VGA CRTC registers here */

    /* VGA has one more read/write attribute register than EGA */
    (void) inb(GenericIOBase + 0x0AU);  /* Reset flip-flop */
    outb(0x3C0, 0x14 | 0x20);
    CurrentValue = inb(0x3C1);
    outb(0x3C0, CurrentValue ^ 0x0F);
    outb(0x3C0, 0x14 | 0x20);
    TestValue = inb(0x3C1);
    outb(0x3C0, CurrentValue);

    /* Quit now if no VGA is present */
    if ((CurrentValue ^ 0x0F) != TestValue)
      return -1;
#endif
    return (int)CHIP_VGA_GENERIC;
}

static Bool
GenericClockSelect(ScrnInfoPtr pScreenInfo, int ClockNumber)
{
#   ifndef PC98_EGC
        static CARD8 save_misc;

        switch (ClockNumber)
        {
            case CLK_REG_SAVE:
                save_misc = inb(0x3CC);
                break;

            case CLK_REG_RESTORE:
                outb(0x3C2, save_misc);
                break;

            default:
                outb(0x3C2, (save_misc & 0xF3) | ((ClockNumber << 2) & 0x0C));
                break;
        }
#   endif

    return TRUE;
}


/*
 * This structure is used to wrap the screen's CloseScreen vector.
 */
typedef struct _GenericRec
{
    Bool ShadowFB;
    CARD8 * ShadowPtr;
    CARD32 ShadowPitch;
    CloseScreenProcPtr CloseScreen;
} GenericRec, *GenericPtr;


static GenericPtr
GenericGetRec(ScrnInfoPtr pScreenInfo)
{
    if (!pScreenInfo->driverPrivate)
        pScreenInfo->driverPrivate = xcalloc(sizeof(GenericRec), 1);

    return (GenericPtr)pScreenInfo->driverPrivate;
}


static void
GenericFreeRec(ScrnInfoPtr pScreenInfo)
{
    if (xf86LoaderCheckSymbol("vgaHWFreeHWRec"))
	vgaHWFreeHWRec(pScreenInfo);
    xfree(pScreenInfo->driverPrivate);
    pScreenInfo->driverPrivate = NULL;
}


static void
GenericProtect(ScrnInfoPtr pScreenInfo, Bool On)
{
    vgaHWProtect(pScreenInfo, On);
}


static Bool
GenericSaveScreen(ScreenPtr pScreen, int mode)
{
    return vgaHWSaveScreen(pScreen, mode);
}

static void
GenericBlankScreen(ScrnInfoPtr pScreenInfo, Bool Unblank)
{
    vgaHWBlankScreen(pScreenInfo, Unblank);
}


/* The default mode */
static DisplayModeRec GenericDefaultMode =
{
    NULL, NULL,                         /* prev & next */
    "Generic 320x200 default mode",
    MODE_OK,                            /* Mode status */
    M_T_CRTC_C,                         /* Mode type   */
    12588,                              /* Pixel clock */
    320, 336, 384, 400,                 /* HTiming */
    0,                                  /* HSkew */
    200, 206, 207, 224,                 /* VTiming */
    2,                                  /* VScan */
    V_CLKDIV2 | V_NHSYNC | V_PVSYNC,    /* Flags */
    0, 25176,                           /* ClockIndex & SynthClock */
    0, 0, 0, 0, 0, 0,                   /* Crtc timings set by ... */
    0,                                  /* ... xf86SetCrtcForModes() */
    0, 0, 0, 0, 0, 0,
    FALSE, FALSE,                       /* These are unadjusted timings */
    0, NULL,                            /* PrivSize & Private */
    0.0, 0.0                            /* HSync & VRefresh */
};


/*
 * This function is called once for each screen at the start of the first
 * server generation to initialise the screen for all server generations.
 */
static Bool
GenericPreInit(ScrnInfoPtr pScreenInfo, int flags)
{
    static rgb        defaultWeight = {0, 0, 0};
    static ClockRange GenericClockRange = {NULL, 0, 80000, 0, FALSE, TRUE, 1, 1, 0};
    MessageType       From;
    int               i, videoRam, Rounding, nModes = 0;
    char             *Module;
    const char	     *Sym;
    vgaHWPtr          pvgaHW;
    GenericPtr        pGenericPriv;
    EntityInfoPtr     pEnt;

    if (flags & PROBE_DETECT) return FALSE;

    /* Set the monitor */
    pScreenInfo->monitor = pScreenInfo->confScreen->monitor;

    if (pScreenInfo->numEntities > 1)
	return FALSE;
    pEnt = xf86GetEntityInfo(*pScreenInfo->entityList);
    if (pEnt->resources)
	return FALSE;

    {
	resRange unusedmem[] =	{ {ResShrMemBlock,0xB0000,0xB7FFF},
				  {ResShrMemBlock,0xB8000,0xBFFFF},
				  _END };
	
	/* XXX Should this be "disabled" or "unused"? */
	xf86SetOperatingState(unusedmem, pEnt->index, ResUnusedOpr);
    }

    /* Determine depth, bpp, etc. */
    if (!xf86SetDepthBpp(pScreenInfo, 4, 0, 4, NoDepth24Support))
        return FALSE;
    pScreenInfo->chipset = (char *)xf86TokenToString(GenericChipsets,
						   pEnt->chipset);

    switch (pScreenInfo->depth)
    {
        case 1:  Module = "xf1bpp"; Sym = "xf1bppScreenInit";  break;
        case 4:  Module = "xf4bpp"; Sym = "xf4bppScreenInit";  break;
        case 8:  Module = "cfb";    Sym = "cfbScreenInit";     break;

        default:
            xf86DrvMsg(pScreenInfo->scrnIndex, X_ERROR,
                "Given depth (%d) is not supported by this driver.\n",
                pScreenInfo->depth);
            return FALSE;
    }
    xf86PrintDepthBpp(pScreenInfo);

    /* Determine colour weights */
    pScreenInfo->rgbBits = 6;
    if (!xf86SetWeight(pScreenInfo, defaultWeight, defaultWeight))
        return FALSE;

    /* XXX:  Check that returned weight is supported */

    /* Determine default visual */
    if (!xf86SetDefaultVisual(pScreenInfo, -1))
        return FALSE;

    /* The gamma fields must be initialised when using the new cmap code */
    if (pScreenInfo->depth > 1) {
	Gamma zeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScreenInfo, zeros))
	    return FALSE;
    }

    /*
     * Determine videoRam.  For mode validation purposes, this needs to be
     * limited to VGA specifications.
     */
    if ((videoRam = pEnt->device->videoRam))
    {
        pScreenInfo->videoRam = videoRam;
        if (pScreenInfo->depth == 8)
        {
            if (videoRam > 64)
                pScreenInfo->videoRam = 64;
        }
        else
        {
            if (videoRam > 256)
                pScreenInfo->videoRam = 256;
        }
        From = X_CONFIG;
    }
    else
    {
        if (pScreenInfo->depth == 8)
            videoRam = 64;
        else
            videoRam = 256;
        pScreenInfo->videoRam = videoRam;
        From = X_DEFAULT;       /* Instead of X_PROBED */
    }
    if (pScreenInfo->depth == 1)
        pScreenInfo->videoRam >>= 2;
    xf86DrvMsg(pScreenInfo->scrnIndex, From, "videoRam: %d kBytes", videoRam);
    if (videoRam != pScreenInfo->videoRam)
        xf86ErrorF(" (using %d kBytes)", pScreenInfo->videoRam);
    xf86ErrorF(".\n");

    if (xf86RegisterResources(pEnt->index,NULL,ResNone))
	return FALSE;
	
    /* Ensure vgahw entry points are available for the clock probe */
    if (!xf86LoadSubModule(pScreenInfo, "vgahw"))
        return FALSE;

    xf86LoaderReqSymLists(vgahwSymbols, NULL);

    /* Allocate driver private structure */
    if (!(pGenericPriv = GenericGetRec(pScreenInfo)))
        return FALSE;

    /* Ensure vgahw private structure is allocated */
    if (!vgaHWGetHWRec(pScreenInfo))
        return FALSE;


    pvgaHW = VGAHWPTR(pScreenInfo);
    pvgaHW->MapSize = 0x00010000;       /* Standard 64kB VGA window */
    vgaHWGetIOBase(pvgaHW);             /* Get VGA I/O base */

#ifndef __NOT_YET__
    if (pScreenInfo->depth == 8)
    {
        pScreenInfo->numClocks = 1;
        pScreenInfo->clock[0] = 25175;
        goto SetDefaultMode;
    }
#endif

    /*
     * Determine clocks.  Limit them to the first four because that's all that
     * can be addressed.
     */
    if ((pScreenInfo->numClocks = pEnt->device->numclocks))
    {
        if (pScreenInfo->numClocks > 4)
            pScreenInfo->numClocks = 4;
        for (i = 0;  i < pScreenInfo->numClocks;  i++)
            pScreenInfo->clock[i] = pEnt->device->clock[i];
        From = X_CONFIG;
    } else  if (xf86ReturnOptValBool(GenericOptions,OPTION_VGA_CLOCKS,FALSE)) {
        pScreenInfo->numClocks = 2;
        pScreenInfo->clock[0] = 25175;
        pScreenInfo->clock[1] = 28322;
    } else
    {
        xf86GetClocks(pScreenInfo, 4, GenericClockSelect, GenericProtect,
            GenericBlankScreen, VGAHW_GET_IOBASE() + 0x0A, 0x08, 1, 28322);
        From = X_PROBED;
    }
    xf86ShowClocks(pScreenInfo, From);

    if (pScreenInfo->display->modes && pScreenInfo->display->modes[0])
    {
        /* Set the virtual X rounding (in bits) */
        if (pScreenInfo->depth == 8)
            Rounding = 16 * 8;
        else
            Rounding = 16;

        /*
         * Validate the modes.  Note that the limits passed to
         * xf86ValidateModes() are VGA CRTC architectural limits.
         */
        pScreenInfo->maxHValue = 2080;
        pScreenInfo->maxVValue = 1025;
        nModes = xf86ValidateModes(pScreenInfo,
            pScreenInfo->monitor->Modes, pScreenInfo->display->modes,
            &GenericClockRange, NULL, 8, 2040, Rounding, 1, 1024,
            pScreenInfo->display->virtualX, pScreenInfo->display->virtualY,
            0x10000, LOOKUP_CLOSEST_CLOCK | LOOKUP_CLKDIV2);

        if (nModes < 0)
            return FALSE;

        /* Remove invalid modes */
        xf86PruneDriverModes(pScreenInfo);
    }

    if (!nModes || !pScreenInfo->modes)
    {
#ifndef __NOT_YET__
  SetDefaultMode:
#endif
        /* Set a default mode, overridding any virtual settings */
        pScreenInfo->virtualX = pScreenInfo->displayWidth = 320;
        pScreenInfo->virtualY = 200;
        pScreenInfo->modes = xalloc(sizeof(DisplayModeRec));
        if (!pScreenInfo->modes)
            return FALSE;
        *pScreenInfo->modes = GenericDefaultMode;
        pScreenInfo->modes->prev = pScreenInfo->modes;
        pScreenInfo->modes->next = pScreenInfo->modes;

        pScreenInfo->virtualFrom = X_DEFAULT;
    }

    /* Set CRTC values for the modes */
    xf86SetCrtcForModes(pScreenInfo, 0);

    /* Set current mode to the first in list */
    pScreenInfo->currentMode = pScreenInfo->modes;

    /* Print mode list */
    xf86PrintModes(pScreenInfo);

    /* Set display resolution */
    xf86SetDpi(pScreenInfo, 0, 0);

    /* Deal with options */
    xf86CollectOptions(pScreenInfo, NULL);
    xf86ProcessOptions(pScreenInfo->scrnIndex, pScreenInfo->options, 
		       GenericOptions);

    if (xf86ReturnOptValBool(GenericOptions,OPTION_SHADOW_FB,FALSE)) {
        pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
        pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	pGenericPriv->ShadowFB = TRUE;
        xf86DrvMsg(pScreenInfo->scrnIndex, X_CONFIG,
		   "Using \"Shadow Framebuffer\"\n");
	switch (pScreenInfo->depth)
	{
            case 1:  Module = "mfb"; Sym = "mfbScreenInit";  break;
            case 4:  Module = "cfb"; Sym = "cfbScreenInit";  break;
	}
	if (!xf86LoadSubModule(pScreenInfo, "shadowfb"))
	    return FALSE;
	xf86LoaderReqSymLists(shadowfbSymbols, NULL);
    }

    /* Ensure depth-specific entry points are available */
    if (!xf86LoadSubModule(pScreenInfo, Module))
	return FALSE;

    xf86LoaderReqSymbols(Sym, NULL);

    /* Only one chipset here */
    if (!pScreenInfo->chipset)
        pScreenInfo->chipset = (char *)GenericChipsets[0].name;
   
    return TRUE;        /* Tada! */
}


/* Save mode on server entry */
static void
GenericSave(ScrnInfoPtr pScreenInfo)
{
    vgaHWSave(pScreenInfo, &VGAHWPTR(pScreenInfo)->SavedReg, VGA_SR_ALL);
}


/* Restore the mode that was saved on server entry */
static void
GenericRestore(ScrnInfoPtr pScreenInfo)
{
    vgaHWPtr pvgaHW = VGAHWPTR(pScreenInfo);

    vgaHWProtect(pScreenInfo, TRUE);
    vgaHWRestore(pScreenInfo, &pvgaHW->SavedReg, VGA_SR_ALL);
    vgaHWProtect(pScreenInfo, FALSE);
}


/* Set a graphics mode */
static Bool
GenericSetMode(ScrnInfoPtr pScreenInfo, DisplayModePtr pMode)
{
    vgaHWPtr pvgaHW = VGAHWPTR(pScreenInfo);

    if (!vgaHWInit(pScreenInfo, pMode))
        return FALSE;
    pScreenInfo->vtSema = TRUE;

#ifndef __NOT_YET__
    if (pScreenInfo->depth == 8)
    {
        int i;

        static const CARD8 CRTC[24] =
        {
#ifndef DEBUGOVERSCAN
            0x5F, 0x4F, 0x4F, 0x80, 0x54, 0x00, 0xBE, 0x1F,
            0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x8F, 0xBF, 0xA3
#else
	    /* These values make some of the overscan area visible */
            0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 
            0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3
#endif
        };

        /* Override vgaHW's CRTC timings */
        for (i = 0;  i < 24;  i++)
            pvgaHW->ModeReg.CRTC[i] = CRTC[i];

        /* Clobber any CLKDIV2 */
        pvgaHW->ModeReg.Sequencer[1] = 0x01;
    }
#endif

    /* Programme the registers */
    vgaHWProtect(pScreenInfo, TRUE);
    vgaHWRestore(pScreenInfo, &pvgaHW->ModeReg, VGA_SR_MODE | VGA_SR_CMAP);
    vgaHWProtect(pScreenInfo, FALSE);

    return TRUE;
}


static Bool
GenericEnterGraphics(ScreenPtr pScreen, ScrnInfoPtr pScreenInfo)
{
    vgaHWPtr pvgaHW = VGAHWPTR(pScreenInfo);

    /* Unlock VGA registers */
    vgaHWUnlock(pvgaHW);

    /* Save the current state and setup the current mode */
    GenericSave(pScreenInfo);
    if (!GenericSetMode(pScreenInfo, pScreenInfo->currentMode))
        return FALSE;

    /* Possibly blank the screen */
    if (pScreen)
        GenericSaveScreen(pScreen, SCREEN_SAVER_ON);

    (*pScreenInfo->AdjustFrame)(pScreenInfo->scrnIndex,
        pScreenInfo->frameX0, pScreenInfo->frameY0, 0);

    return TRUE;
}


static void
GenericLeaveGraphics(ScrnInfoPtr pScreenInfo)
{
    GenericRestore(pScreenInfo);
    vgaHWLock(VGAHWPTR(pScreenInfo));
}


/* Unravel the screen */
static Bool
GenericCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScreenInfo = xf86Screens[scrnIndex];
    GenericPtr pGenericPriv = GenericGetRec(pScreenInfo);
    Bool Closed = TRUE;

    if(pGenericPriv->ShadowPtr)
	xfree(pGenericPriv->ShadowPtr);

    if (pGenericPriv && (pScreen->CloseScreen = pGenericPriv->CloseScreen))
    {
        pGenericPriv->CloseScreen = NULL;
        Closed = (*pScreen->CloseScreen)(scrnIndex, pScreen);
    }

    if (pScreenInfo->vtSema) {
	GenericLeaveGraphics(pScreenInfo);
	pScreenInfo->vtSema = FALSE;
    }
    vgaHWUnmapMem(pScreenInfo);

    return Closed;
}


#ifdef DPMSExtension
static void
GenericDPMSSet(ScrnInfoPtr pScreen, int mode, int flags)
{
    vgaHWDPMSSet(pScreen, mode, flags);
}
#endif


static void
GenericRefreshArea1bpp(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
    GenericPtr pPriv = GenericGetRec(pScrn);
    vgaHWPtr pvgaHW = VGAHWPTR(pScrn);
    int width, height, FBPitch, left, i, j, phase;
    CARD8  *dst, *dstPtr, *src, *srcPtr;
   
    FBPitch = pScrn->displayWidth >> 3;

    while(num--) {
	left = pbox->x1 & ~7;
        width = ((pbox->x2 - left) + 7) >> 3;
        height = pbox->y2 - pbox->y1;
        src = pPriv->ShadowPtr + (pbox->y1 * pPriv->ShadowPitch) + (left >> 3); 
        dst = (CARD8*)pvgaHW->Base + (pbox->y1 * FBPitch) + (left >> 3);

	if((phase = (long)dst & 3L)) {
	    phase = 4 - phase;
	    if(phase > width) phase = width;
	    width -= phase;
	}

        while(height--) {
	    dstPtr = dst;
	    srcPtr = src;
	    i = width;
	    j = phase;
	    while(j--) 
 		*dstPtr++ = byte_reversed[*srcPtr++];
	    while(i >= 4) {
		*((CARD32*)dstPtr) = byte_reversed[srcPtr[0]] |
				    (byte_reversed[srcPtr[1]] << 8) |
				    (byte_reversed[srcPtr[2]] << 16) |
				    (byte_reversed[srcPtr[3]] << 24);
		srcPtr += 4;
		dstPtr += 4;
		i -= 4;
	    }
	    while(i--)
 		*dstPtr++ = byte_reversed[*srcPtr++];
            dst += FBPitch;
            src += pPriv->ShadowPitch;
        }
        
        pbox++;
    }

} 

static void
GenericRefreshArea4bpp(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
    GenericPtr pPriv = GenericGetRec(pScrn);
    vgaHWPtr pvgaHW = VGAHWPTR(pScrn);
    int width, height, FBPitch, left, i, j, SRCPitch, phase;
    register CARD32 m;
    CARD8  s1, s2, s3, s4;
    CARD32 *src, *srcPtr;
    CARD8  *dst, *dstPtr;
   
    FBPitch = pScrn->displayWidth >> 3;
    SRCPitch = pPriv->ShadowPitch >> 2;

    pvgaHW->writeGr(pvgaHW, 0x05, 0x00);
    pvgaHW->writeGr(pvgaHW, 0x01, 0x00);
    pvgaHW->writeGr(pvgaHW, 0x08, 0xFF);

    while(num--) {
	left = pbox->x1 & ~7;
        width = ((pbox->x2 - left) + 7) >> 3;
        height = pbox->y2 - pbox->y1;
        src = (CARD32*)pPriv->ShadowPtr + (pbox->y1 * SRCPitch) + (left >> 2); 
        dst = (CARD8*)pvgaHW->Base + (pbox->y1 * FBPitch) + (left >> 3);

	if((phase = (long)dst & 3L)) {
	    phase = 4 - phase;
	    if(phase > width) phase = width;
	    width -= phase;
	}

        while(height--) {
	    pvgaHW->writeSeq(pvgaHW, 0x02, 1);
	    dstPtr = dst;
	    srcPtr = src;
	    i = width;
	    j = phase;
	    while(j--) {
		m = (srcPtr[1] & 0x01010101) | ((srcPtr[0] & 0x01010101) << 4);
 		*dstPtr++ = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
		srcPtr += 2;
	    }
	    while(i >= 4) {
		m = (srcPtr[1] & 0x01010101) | ((srcPtr[0] & 0x01010101) << 4);
 		s1 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
		m = (srcPtr[3] & 0x01010101) | ((srcPtr[2] & 0x01010101) << 4);
 		s2 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
		m = (srcPtr[5] & 0x01010101) | ((srcPtr[4] & 0x01010101) << 4);
 		s3 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
		m = (srcPtr[7] & 0x01010101) | ((srcPtr[6] & 0x01010101) << 4);
 		s4 = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
		*((CARD32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
		srcPtr += 8;
		dstPtr += 4;
		i -= 4;
	    }
	    while(i--) {
		m = (srcPtr[1] & 0x01010101) | ((srcPtr[0] & 0x01010101) << 4);
 		*dstPtr++ = (m >> 24) | (m >> 15) | (m >> 6) | (m << 3);
		srcPtr += 2;
	    }

	    pvgaHW->writeSeq(pvgaHW, 0x02, 1 << 1);
	    dstPtr = dst;
	    srcPtr = src;
	    i = width;
	    j = phase;
	    while(j--) {
		m = (srcPtr[1] & 0x02020202) | ((srcPtr[0] & 0x02020202) << 4);
 		*dstPtr++ = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
		srcPtr += 2;
	    }
	    while(i >= 4) {
		m = (srcPtr[1] & 0x02020202) | ((srcPtr[0] & 0x02020202) << 4);
 		s1 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
		m = (srcPtr[3] & 0x02020202) | ((srcPtr[2] & 0x02020202) << 4);
 		s2 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
		m = (srcPtr[5] & 0x02020202) | ((srcPtr[4] & 0x02020202) << 4);
 		s3 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
		m = (srcPtr[7] & 0x02020202) | ((srcPtr[6] & 0x02020202) << 4);
 		s4 = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
		*((CARD32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
		srcPtr += 8;
		dstPtr += 4;
		i -= 4;
	    }
	    while(i--) {
		m = (srcPtr[1] & 0x02020202) | ((srcPtr[0] & 0x02020202) << 4);
 		*dstPtr++ = (m >> 25) | (m >> 16) | (m >> 7) | (m << 2);
		srcPtr += 2;
	    }

	    pvgaHW->writeSeq(pvgaHW, 0x02, 1 << 2);
	    dstPtr = dst;
	    srcPtr = src;
	    i = width;
	    j = phase;
	    while(j--) {
		m = (srcPtr[1] & 0x04040404) | ((srcPtr[0] & 0x04040404) << 4);
 		*dstPtr++ = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
		srcPtr += 2;
	    }
	    while(i >= 4) {
		m = (srcPtr[1] & 0x04040404) | ((srcPtr[0] & 0x04040404) << 4);
 		s1 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
		m = (srcPtr[3] & 0x04040404) | ((srcPtr[2] & 0x04040404) << 4);
 		s2 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
		m = (srcPtr[5] & 0x04040404) | ((srcPtr[4] & 0x04040404) << 4);
 		s3 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
		m = (srcPtr[7] & 0x04040404) | ((srcPtr[6] & 0x04040404) << 4);
 		s4 = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
		*((CARD32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
		srcPtr += 8;
		dstPtr += 4;
		i -= 4;
	    }
	    while(i--) {
		m = (srcPtr[1] & 0x04040404) | ((srcPtr[0] & 0x04040404) << 4);
 		*dstPtr++ = (m >> 26) | (m >> 17) | (m >> 8) | (m << 1);
		srcPtr += 2;
	    }
	    
	    pvgaHW->writeSeq(pvgaHW, 0x02, 1 << 3);
	    dstPtr = dst;
	    srcPtr = src;
	    i = width;
	    j = phase;
	    while(j--) {
		m = (srcPtr[1] & 0x08080808) | ((srcPtr[0] & 0x08080808) << 4);
 		*dstPtr++ = (m >> 27) | (m >> 18) | (m >> 9) | m;
		srcPtr += 2;
	    }
	    while(i >= 4) {
		m = (srcPtr[1] & 0x08080808) | ((srcPtr[0] & 0x08080808) << 4);
 		s1 = (m >> 27) | (m >> 18) | (m >> 9) | m;
		m = (srcPtr[3] & 0x08080808) | ((srcPtr[2] & 0x08080808) << 4);
 		s2 = (m >> 27) | (m >> 18) | (m >> 9) | m;
		m = (srcPtr[5] & 0x08080808) | ((srcPtr[4] & 0x08080808) << 4);
 		s3 = (m >> 27) | (m >> 18) | (m >> 9) | m;
		m = (srcPtr[7] & 0x08080808) | ((srcPtr[6] & 0x08080808) << 4);
 		s4 = (m >> 27) | (m >> 18) | (m >> 9) | m;
		*((CARD32*)dstPtr) = s1 | (s2 << 8) | (s3 << 16) | (s4 << 24);
		srcPtr += 8;
		dstPtr += 4;
		i -= 4;
	    }
	    while(i--) {
		m = (srcPtr[1] & 0x08080808) | ((srcPtr[0] & 0x08080808) << 4);
 		*dstPtr++ = (m >> 27) | (m >> 18) | (m >> 9) | m;
		srcPtr += 2;
	    }

            dst += FBPitch;
            src += SRCPitch;
        }
        
        pbox++;
    }

} 

static Bool
GenericScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScreenInfo = xf86Screens[scrnIndex];

    vgaHWPtr pvgaHW;
    GenericPtr pGenericPriv;
    Bool Inited = FALSE;

    /* Get driver private */
    pGenericPriv = GenericGetRec(pScreenInfo);

    /* Map VGA aperture */
    if (!vgaHWMapMem(pScreenInfo))
        return FALSE;

    /* Initialise graphics mode */
    if (!GenericEnterGraphics(pScreen, pScreenInfo))
        return FALSE;

    /* Get vgahw private */
    pvgaHW = VGAHWPTR(pScreenInfo);

    miClearVisualTypes();

    if (!miSetVisualTypes(pScreenInfo->depth,
			  miGetDefaultVisualMask(pScreenInfo->depth),
			  pScreenInfo->rgbBits, pScreenInfo->defaultVisual))
	return FALSE;

    /* Initialise the framebuffer */
    switch (pScreenInfo->depth)
    {
        case 1:
	    if (pGenericPriv->ShadowFB) {
		pGenericPriv->ShadowPitch = 
				((pScreenInfo->virtualX + 31) >> 3) & ~3L;
		pGenericPriv->ShadowPtr = xalloc(pGenericPriv->ShadowPitch *
						 pScreenInfo->virtualY);
		if(pGenericPriv->ShadowPtr == NULL)
		    return FALSE;
		Inited = mfbScreenInit(pScreen, pGenericPriv->ShadowPtr,
				       pScreenInfo->virtualX, 
				       pScreenInfo->virtualY,
				       pScreenInfo->xDpi, pScreenInfo->yDpi,
				       pScreenInfo->displayWidth);
		ShadowFBInit(pScreen, GenericRefreshArea1bpp);
	    } else {
		Inited = xf1bppScreenInit(pScreen, pvgaHW->Base,
					  pScreenInfo->virtualX, 
					  pScreenInfo->virtualY,
					  pScreenInfo->xDpi, pScreenInfo->yDpi,
					  pScreenInfo->displayWidth);
	    }
            break;
        case 4:
	    if (pGenericPriv->ShadowFB) {
		/* in order to use ShadowFB we do depth 4 / bpp 8 */
		pScreenInfo->bitsPerPixel = 8;
		pGenericPriv->ShadowPitch = (pScreenInfo->virtualX + 3) & ~3L;
		pGenericPriv->ShadowPtr = xalloc(pGenericPriv->ShadowPitch *
						 pScreenInfo->virtualY);
		if(pGenericPriv->ShadowPtr == NULL)
		    return FALSE;
		Inited = cfbScreenInit(pScreen, pGenericPriv->ShadowPtr,
				       pScreenInfo->virtualX, 
				       pScreenInfo->virtualY,
				       pScreenInfo->xDpi, pScreenInfo->yDpi,
				       pScreenInfo->displayWidth);
		ShadowFBInit(pScreen, GenericRefreshArea4bpp);
	    } else {
		Inited = xf4bppScreenInit(pScreen, pvgaHW->Base,
					  pScreenInfo->virtualX, 
					  pScreenInfo->virtualY,
					  pScreenInfo->xDpi, pScreenInfo->yDpi,
					  pScreenInfo->displayWidth);
	    }
            break;
        case 8:
            Inited = cfbScreenInit(pScreen, pvgaHW->Base,
                pScreenInfo->virtualX, pScreenInfo->virtualY,
                pScreenInfo->xDpi, pScreenInfo->yDpi,
                pScreenInfo->displayWidth);
            break;
    }

    if (!Inited)
        return FALSE;

    miInitializeBackingStore(pScreen);

    xf86SetBlackWhitePixels(pScreen);

    /* Initialise cursor */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* Setup default colourmap */
    Inited = miCreateDefColormap(pScreen);

    /* Try the new code based on the new colormap layer */
    if (pScreenInfo->depth > 1)
        vgaHWHandleColormaps(pScreen);

#ifdef DPMSExtension
    xf86DPMSInit(pScreen, GenericDPMSSet, 0);
#endif

    /* Wrap the screen's CloseScreen vector and set its SaveScreen vector */
    pGenericPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = GenericCloseScreen;
    pScreen->SaveScreen = GenericSaveScreen;

    if (!Inited)
        GenericCloseScreen(scrnIndex, pScreen);

    pScreenInfo->racIoFlags = RAC_COLORMAP | RAC_VIEWPORT;
    if (pScreenInfo->depth < 8)
	pScreenInfo->racIoFlags |= RAC_FB;
    pScreenInfo->racMemFlags = RAC_FB;
    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScreenInfo->scrnIndex, pScreenInfo->options);

    return Inited;
}


static Bool
GenericSwitchMode(int scrnIndex, DisplayModePtr pMode, int flags)
{
    return GenericSetMode(xf86Screens[scrnIndex], pMode);
}


static void
GenericAdjustFrame(int scrnIndex, int x, int y, int flags)
{
#   ifndef PC98_EGC
        ScrnInfoPtr pScreenInfo = xf86Screens[scrnIndex];
        vgaHWPtr pvgaHW = VGAHWPTR(pScreenInfo);
        int Base = (y * pScreenInfo->displayWidth + x) >> 3;

        outw(pvgaHW->IOBase + 4, (Base & 0x00FF00) | 0x0C);
        outw(pvgaHW->IOBase + 4, ((Base & 0x0000FF) << 8) | 0x0D);
#   endif
}


static Bool
GenericEnterVT(int scrnIndex, int flags)
{
     return GenericEnterGraphics(NULL, xf86Screens[scrnIndex]);
}


static void
GenericLeaveVT(int scrnIndex, int flags)
{
    GenericLeaveGraphics(xf86Screens[scrnIndex]);
}


static void
GenericFreeScreen(int scrnIndex, int flags)
{
    GenericFreeRec(xf86Screens[scrnIndex]);
}


static int
GenericValidMode(int scrnIndex, DisplayModePtr pMode, Bool Verbose, int flags)
{
    if (pMode->Flags & V_INTERLACE)
        return MODE_NO_INTERLACE;

    return MODE_OK;
}
