#define DEBUG_VERB 2
/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Authors: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vesa/vesa.c,v 1.24 2001/10/01 13:44:12 eich Exp $
 */

#include "vesa.h"

/* All drivers initialising the SW cursor need this */
#include "mipointer.h"

/* All drivers implementing backing store need this */
#include "mibstore.h"

/* Colormap handling */
#include "micmap.h"
#include "xf86cmap.h"

/* DPMS */
#define DPMS_SERVER
#include "extensions/dpms.h"

/* Mandatory functions */
static const OptionInfoRec * VESAAvailableOptions(int chipid, int busid);
static void VESAIdentify(int flags);
static Bool VESAProbe(DriverPtr drv, int flags);
static Bool VESAPreInit(ScrnInfoPtr pScrn, int flags);
static Bool VESAScreenInit(int Index, ScreenPtr pScreen, int argc,
			   char **argv);
static Bool VESAEnterVT(int scrnIndex, int flags);
static void VESALeaveVT(int scrnIndex, int flags);
static Bool VESACloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool VESASaveScreen(ScreenPtr pScreen, int mode);

static Bool VESASwitchMode(int scrnIndex, DisplayModePtr pMode, int flags);
static Bool VESASetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode);
static void VESAAdjustFrame(int scrnIndex, int x, int y, int flags);
static void VESAFreeScreen(int scrnIndex, int flags);
static void VESAFreeRec(ScrnInfoPtr pScrn);

static void
VESADisplayPowerManagementSet(ScrnInfoPtr pScrn, int mode,
                int flags);

/* locally used functions */
static int VESAFindIsaDevice(GDevPtr dev);
static Bool VESAMapVidMem(ScrnInfoPtr pScrn);
static void VESAUnmapVidMem(ScrnInfoPtr pScrn);
static int VESABankSwitch(ScreenPtr pScreen, unsigned int iBank);
static void VESALoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
			    LOCO *colors, VisualPtr pVisual);
static void SaveFonts(ScrnInfoPtr pScrn);
static void RestoreFonts(ScrnInfoPtr pScrn);
static Bool 
VESASaveRestore(ScrnInfoPtr pScrn, vbeSaveRestoreFunction function);

static void *VESAWindowPlanar(ScreenPtr pScrn, CARD32 row, CARD32 offset,
			      int mode, CARD32 *size, void *closure);
static void *VESAWindowLinear(ScreenPtr pScrn, CARD32 row, CARD32 offset,
			      int mode, CARD32 *size, void *closure);
static void *VESAWindowWindowed(ScreenPtr pScrn, CARD32 row, CARD32 offset,
				int mode, CARD32 *size, void *closure);

static Bool VESADGAInit(ScrnInfoPtr pScrn, ScreenPtr pScreen);

/* 
 * This contains the functions needed by the server after loading the
 * driver module.  It must be supplied, and gets added the driver list by
 * the Module Setup funtion in the dynamic case.  In the static case a
 * reference to this is compiled in, and this requires that the name of
 * this DriverRec be an upper-case version of the driver name.
 */
DriverRec VESA = {
    VESA_VERSION,
    VESA_DRIVER_NAME,
    VESAIdentify,
    VESAProbe,
    VESAAvailableOptions,
    NULL,
    0
};

enum GenericTypes
{
    CHIP_VESA_GENERIC
};

/* Supported chipsets */
static SymTabRec VESAChipsets[] =
{
    {CHIP_VESA_GENERIC, "vesa"},
    {-1,		 NULL}
};

static PciChipsets VESAPCIchipsets[] = {
  { CHIP_VESA_GENERIC, PCI_CHIP_VGA, RES_SHARED_VGA },
  { -1,		-1,	   RES_UNDEFINED },
};

static IsaChipsets VESAISAchipsets[] = {
  {CHIP_VESA_GENERIC, RES_EXCLUSIVE_VGA},
  {-1,		0 }
};

typedef enum {
    OPTION_SHADOW_FB
} VESAOpts;

static const OptionInfoRec VESAOptions[] = {
    { OPTION_SHADOW_FB,	"ShadowFB",	OPTV_BOOLEAN,	{0},	FALSE },
    { -1,		NULL,		OPTV_NONE,	{0},	FALSE }
};

/*
 * List of symbols from other modules that this module references.  This
 * list is used to tell the loader that it is OK for symbols here to be
 * unresolved providing that it hasn't been told that they haven't been
 * told that they are essential via a call to xf86LoaderReqSymbols() or
 * xf86LoaderReqSymLists().  The purpose is this is to avoid warnings about
 * unresolved symbols that are not required.
 */
static const char *miscfbSymbols[] = {
    "xf1bppScreenInit",
    "xf4bppScreenInit",
    "afbScreenInit",
    "mfbScreenInit",
    "cfb24_32ScreenInit",
    NULL
};

static const char *fbSymbols[] = {
    "fbPictureInit",
    "fbScreenInit",
    NULL
};

static const char *shadowSymbols[] = {
    "shadowAlloc",
    "shadowInit",
    "shadowUpdatePacked",
    "shadowUpdatePlanar4",
    "shadowUpdatePlanar4x8",
    NULL
};

static const char *vbeSymbols[] = {
    "VBEBankSwitch",
    "VBEFreeModeInfo",
    "VBEGetModeInfo",
    "VBEGetVBEInfo",
    "VBEGetVBEMode",
    "VBEInit",
    "VBESaveRestore",
    "VBESetDisplayStart",
    "VBESetGetDACPaletteFormat",
    "VBESetGetLogicalScanlineLength",
    "VBESetGetPaletteData",
    "VBESetVBEMode",
    "vbeDoEDID",
    "vbeFree",
    NULL
};

static const char *ddcSymbols[] = {
    "xf86PrintEDID",
    "xf86SetDDCproperties",
    NULL
};

#if 0
static const char *vgahwSymbols[] = {
    "vgaHWDPMSSet",
    NULL
};
#endif

#ifdef XFree86LOADER

/* Module loader interface */
static MODULESETUPPROTO(vesaSetup);

static XF86ModuleVersionInfo vesaVersionRec =
{
    VESA_DRIVER_NAME,
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XF86_VERSION_CURRENT,
    VESA_MAJOR_VERSION, VESA_MINOR_VERSION, VESA_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,			/* This is a video driver */
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0, 0, 0, 0}
};

/*
 * This data is accessed by the loader.  The name must be the module name
 * followed by "ModuleData".
 */
XF86ModuleData vesaModuleData = { &vesaVersionRec, vesaSetup, NULL };

static pointer
vesaSetup(pointer Module, pointer Options, int *ErrorMajor, int *ErrorMinor)
{
    static Bool Initialised = FALSE;

    if (!Initialised)
    {
	Initialised = TRUE;
	xf86AddDriver(&VESA, Module, 0);
	LoaderRefSymLists(miscfbSymbols,
			  fbSymbols,
			  shadowSymbols,
			  vbeSymbols,
			  ddcSymbols,
			  NULL);
	return (pointer)TRUE;
    }

    if (ErrorMajor)
	*ErrorMajor = LDR_ONCEONLY;
    return (NULL);
}

#endif

static const OptionInfoRec *
VESAAvailableOptions(int chipid, int busid)
{
    return (VESAOptions);
}

static void
VESAIdentify(int flags)
{
    xf86PrintChipsets(VESA_NAME, "driver for VESA chipsets", VESAChipsets);
}

/*
 * This function is called once, at the start of the first server generation to
 * do a minimal probe for supported hardware.
 */

static Bool
VESAProbe(DriverPtr drv, int flags)
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
    if ((numDevSections = xf86MatchDevice(VESA_NAME,
					  &devSections)) <= 0)
	return (FALSE);

    /* PCI BUS */
    if (xf86GetPciVideoInfo()) {
	numUsed = xf86MatchPciInstances(VESA_NAME, PCI_VENDOR_GENERIC,
					VESAChipsets, VESAPCIchipsets, 
					devSections, numDevSections,
					drv, &usedChips);
	if (numUsed > 0) {
	    if (flags & PROBE_DETECT)
		foundScreen = TRUE;
	    else {
		for (i = 0; i < numUsed; i++) {
		    ScrnInfoPtr pScrn = NULL;
		    /* Allocate a ScrnInfoRec  */
		    if ((pScrn = xf86ConfigPciEntity(pScrn,0,usedChips[i],
						     VESAPCIchipsets,NULL,
						     NULL,NULL,NULL,NULL))) {
			pScrn->driverVersion = VESA_VERSION;
			pScrn->driverName    = VESA_DRIVER_NAME;
			pScrn->name	     = VESA_NAME;
			pScrn->Probe	     = VESAProbe;
			pScrn->PreInit       = VESAPreInit;
			pScrn->ScreenInit    = VESAScreenInit;
			pScrn->SwitchMode    = VESASwitchMode;
			pScrn->AdjustFrame   = VESAAdjustFrame;
			pScrn->EnterVT       = VESAEnterVT;
			pScrn->LeaveVT       = VESALeaveVT;
			pScrn->FreeScreen    = VESAFreeScreen;
			foundScreen = TRUE;
		    }
		}
	    }
	    xfree(usedChips);
	}
    }

    /* Isa Bus */
    numUsed = xf86MatchIsaInstances(VESA_NAME,VESAChipsets,
				    VESAISAchipsets, drv,
				    VESAFindIsaDevice, devSections,
				    numDevSections, &usedChips);
    if(numUsed > 0) {
	if (flags & PROBE_DETECT)
	    foundScreen = TRUE;
	else for (i = 0; i < numUsed; i++) {
	    ScrnInfoPtr pScrn = NULL;
	    if ((pScrn = xf86ConfigIsaEntity(pScrn, 0,usedChips[i],
					     VESAISAchipsets, NULL,
					     NULL, NULL, NULL, NULL))) {
	    
		pScrn->driverVersion = VESA_VERSION;
		pScrn->driverName    = VESA_DRIVER_NAME;
		pScrn->name	     = VESA_NAME;
		pScrn->Probe	     = VESAProbe;
		pScrn->PreInit       = VESAPreInit;
		pScrn->ScreenInit    = VESAScreenInit;
		pScrn->SwitchMode    = VESASwitchMode;
		pScrn->AdjustFrame   = VESAAdjustFrame;
		pScrn->EnterVT       = VESAEnterVT;
		pScrn->LeaveVT       = VESALeaveVT;
		pScrn->FreeScreen    = VESAFreeScreen;
		foundScreen = TRUE;
	    }
	}
	xfree(usedChips);
    }

    xfree(devSections);

    return (foundScreen);
}

static int
VESAFindIsaDevice(GDevPtr dev)
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
    return (int)CHIP_VESA_GENERIC;
}

static VESAPtr
VESAGetRec(ScrnInfoPtr pScrn)
{
    if (!pScrn->driverPrivate)
	pScrn->driverPrivate = xcalloc(sizeof(VESARec), 1);

    return ((VESAPtr)pScrn->driverPrivate);
}

static void
VESAFreeRec(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa = VESAGetRec(pScrn);
#if 0
    DisplayModePtr mode = pScrn->modes;
    /* I am not sure if the modes will ever get freed.
     * Anyway, the data unknown to other modules is being freed here.
     */
    if (mode) {
	do {
	    if (mode->Private) {
		ModeInfoData *data = (ModeInfoData*)mode->Private;

		if (data->block)
		    xfree(data->block);

		xfree(data);

		mode->Private = NULL;
	    }
	    mode = mode->next;
	} while (mode && mode != pScrn->modes);
    }
#endif
    xfree(pVesa->monitor);
    xfree(pVesa->vbeInfo);
    xfree(pVesa->pal);
    xfree(pVesa->savedPal);
    xfree(pVesa->fonts);
    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;
}

/*
 * This function is called once for each screen at the start of the first
 * server generation to initialise the screen for all server generations.
 */
static Bool
VESAPreInit(ScrnInfoPtr pScrn, int flags)
{
    VESAPtr pVesa;
    VbeInfoBlock *vbe;
    DisplayModePtr pMode, tmp;
    VbeModeInfoBlock *mode;
    ModeInfoData *data = NULL;
    char *mod = NULL;
    const char *reqSym = NULL;
    Gamma gzeros = {0.0, 0.0, 0.0};
    rgb rzeros = {0, 0, 0};
    pointer pVbeModule, pDDCModule;
    int i;

    if (flags & PROBE_DETECT)
	return (FALSE);

    pVesa = VESAGetRec(pScrn);
    pVesa->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
    pVesa->device = xf86GetDevFromEntity(pScrn->entityList[0],
					 pScrn->entityInstanceList[0]);

#if 0
    /* Load vgahw module */
    if (!xf86LoadSubModule(pScrn, "vgahw"))
    	return (FALSE);

    xf86LoaderReqSymLists(vgahwSymbols, NULL);
#endif

    /* Load vbe module */
    if ((pVbeModule = xf86LoadSubModule(pScrn, "vbe")) == NULL)
        return (FALSE);

    xf86LoaderReqSymLists(vbeSymbols, NULL);

    if ((pVesa->pVbe = VBEInit(NULL, pVesa->pEnt->index)) == NULL)
        return (FALSE);

    if (pVesa->pEnt->location.type == BUS_PCI) {
	pVesa->pciInfo = xf86GetPciInfoForEntity(pVesa->pEnt->index);
	pVesa->pciTag = pciTag(pVesa->pciInfo->bus, pVesa->pciInfo->device,
			       pVesa->pciInfo->func);
	pVesa->primary = xf86IsPrimaryPci(pVesa->pciInfo);
    }
    else
	pVesa->primary = TRUE;

    pScrn->chipset = "vesa";
    pScrn->monitor = pScrn->confScreen->monitor;
    pScrn->progClock = TRUE;
    pScrn->rgbBits = 8;

    if (!xf86SetDepthBpp(pScrn, 8, 8, 8, Support24bppFb)) {
        vbeFree(pVesa->pVbe);
	return (FALSE);
    }
    xf86PrintDepthBpp(pScrn);

    /* Get the depth24 pixmap format */
    if (pScrn->depth == 24 && pVesa->pix24bpp == 0)
	pVesa->pix24bpp = xf86GetBppFromDepth(pScrn, 24);

    /* color weight */
    if (pScrn->depth > 8 && !xf86SetWeight(pScrn, rzeros, rzeros)) {
        vbeFree(pVesa->pVbe);
	return (FALSE);
    }
    /* visual init */
    if (!xf86SetDefaultVisual(pScrn, -1)) {
        vbeFree(pVesa->pVbe);
	return (FALSE);
    }

    xf86SetGamma(pScrn, gzeros);

    vbe = VBEGetVBEInfo(pVesa->pVbe);
    pVesa->major = (unsigned)(vbe->VESAVersion >> 8);
    pVesa->minor = vbe->VESAVersion & 0xff;
    pVesa->vbeInfo = vbe;
    pScrn->videoRam = vbe->TotalMemory * 64 * 1024;

    if (pVesa->major >= 2) {
	/* Load ddc module */
      if ((pDDCModule = xf86LoadSubModule(pScrn, "ddc")) == NULL) {
            vbeFree(pVesa->pVbe);
	    return (FALSE);
      }

	if ((pVesa->monitor = vbeDoEDID(pVesa->pVbe, pDDCModule)) != NULL) {
	    xf86PrintEDID(pVesa->monitor);
	}

	xf86UnloadSubModule(pDDCModule);
    }

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    if ((pScrn->monitor->DDC = pVesa->monitor) != NULL)
	xf86SetDDCproperties(pScrn, pVesa->monitor);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, DEBUG_VERB,
			"Searching for matching VESA mode(s):\n");

    i = 0;
    while (vbe->VideoModePtr[i] != 0xffff) {
	int id = vbe->VideoModePtr[i++];

	if ((mode = VBEGetModeInfo(pVesa->pVbe, id)) == NULL)
	    continue;
	xf86ErrorFVerb(DEBUG_VERB,
	    "Mode: %x (%dx%d)\n", id, mode->XResolution, mode->YResolution);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	ModeAttributes: 0x%x\n", mode->ModeAttributes);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinAAttributes: 0x%x\n", mode->WinAAttributes);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinBAttributes: 0x%x\n", mode->WinBAttributes);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinGranularity: %d\n", mode->WinGranularity);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinSize: %d\n", mode->WinSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinASegment: 0x%x\n", mode->WinASegment);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinBSegment: 0x%x\n", mode->WinBSegment);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	WinFuncPtr: 0x%x\n", mode->WinFuncPtr);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	BytesPerScanline: %d\n", mode->BytesPerScanline);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	XResolution: %d\n", mode->XResolution);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	YResolution: %d\n", mode->YResolution);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	XCharSize: %d\n", mode->XCharSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	YCharSize: %d\n", mode->YCharSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	NumberOfPlanes: %d\n", mode->NumberOfPlanes);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	BitsPerPixel: %d\n", mode->BitsPerPixel);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	NumberOfBanks: %d\n", mode->NumberOfBanks);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	MemoryModel: %d\n", mode->MemoryModel);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	BankSize: %d\n", mode->BankSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	NumberOfImages: %d\n", mode->NumberOfImages);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	RedMaskSize: %d\n", mode->RedMaskSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	RedFieldPosition: %d\n", mode->RedFieldPosition);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	GreenMaskSize: %d\n", mode->GreenMaskSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	GreenFieldPosition: %d\n", mode->GreenFieldPosition);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	BlueMaskSize: %d\n", mode->BlueMaskSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	BlueFieldPosition: %d\n", mode->BlueFieldPosition);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	RsvdMaskSize: %d\n", mode->RsvdMaskSize);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	RsvdFieldPosition: %d\n", mode->RsvdFieldPosition);
	xf86ErrorFVerb(DEBUG_VERB,
	    "	DirectColorModeInfo: %d\n", mode->DirectColorModeInfo);
	if (pVesa->major >= 2) {
	    xf86ErrorFVerb(DEBUG_VERB,
		"	PhysBasePtr: 0x%x\n", mode->PhysBasePtr);
	    if (pVesa->major >= 3) {
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinBytesPerScanLine: %d\n", mode->LinBytesPerScanLine);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	BnkNumberOfImagePages: %d\n", mode->BnkNumberOfImagePages);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinNumberOfImagePages: %d\n", mode->LinNumberOfImagePages);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinRedMaskSize: %d\n", mode->LinRedMaskSize);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinRedFieldPosition: %d\n", mode->LinRedFieldPosition);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinGreenMaskSize: %d\n", mode->LinGreenMaskSize);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinGreenFieldPosition: %d\n", mode->LinGreenFieldPosition);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinBlueMaskSize: %d\n", mode->LinBlueMaskSize);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinBlueFieldPosition: %d\n", mode->LinBlueFieldPosition);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinRsvdMaskSize: %d\n", mode->LinRsvdMaskSize);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	LinRsvdFieldPosition: %d\n", mode->LinRsvdFieldPosition);
		xf86ErrorFVerb(DEBUG_VERB,
		    "	MaxPixelClock: %d\n", mode->MaxPixelClock);
	    }
	}

	if (!(mode->ModeAttributes & (1 << 0)) ||	/* supported in the configured hardware */
	    !(mode->ModeAttributes & (1 << 4)) ||	/* text mode */
	    (pScrn->bitsPerPixel != 1 && !(mode->ModeAttributes & (1 << 3))) || /* monochrome */
	    (mode->BitsPerPixel > 8 &&
		(mode->RedMaskSize + mode->GreenMaskSize +
		 mode->BlueMaskSize != pScrn->depth)) ||
	    /* only linear mode, but no PhysBasePtr */
	    ((mode->ModeAttributes & (1 << 6)) &&
	     (mode->ModeAttributes & (1 << 7)) && !mode->PhysBasePtr) ||
	    ((mode->ModeAttributes & (1 << 6)) &&
	     !(mode->ModeAttributes & (1 << 7))) ||
	    mode->BitsPerPixel != pScrn->bitsPerPixel) {
	    VBEFreeModeInfo(mode);
	    continue;
	}

	pMode = xcalloc(sizeof(DisplayModeRec), 1);
	pMode->prev = pMode->next = NULL;

	pMode->status = MODE_OK;
	pMode->type = M_T_DEFAULT;/*M_T_BUILTIN;*/

	/* for adjust frame */
	pMode->HDisplay = mode->XResolution;
	pMode->VDisplay = mode->YResolution;

	data = xcalloc(sizeof(ModeInfoData), 1);
	data->mode = id;
	data->data = mode;
	pMode->PrivSize = sizeof(ModeInfoData);
	pMode->Private = (INT32*)data;

	if (pScrn->modePool == NULL) {
	    pScrn->modePool = pMode;
	    pMode->next = pMode->prev = pMode;
	}
	else {
	    tmp = pScrn->modePool;

	    tmp->prev = pMode;
	    while (tmp->next != pScrn->modePool)
		tmp = tmp->next;
	    tmp->next = pMode;
	    pMode->prev = tmp;
	    pMode->next = pScrn->modePool;
	}

    }

    xf86ErrorFVerb(DEBUG_VERB, "\n");
    xf86ErrorFVerb(DEBUG_VERB,
	"Total Memory: %d 64Kb banks (%dM)\n", vbe->TotalMemory,
	   (vbe->TotalMemory * 65536) / (1024 * 1024));

    pVesa->mapSize = vbe->TotalMemory * 65536;
    if (pScrn->modePool == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No matching modes\n");
        vbeFree(pVesa->pVbe);
	return (FALSE);
    }
    for (i = 0; pScrn->modePool != NULL && pScrn->display->modes[i] != NULL; i++) {
	pMode = pScrn->modePool;

	do {
	    DisplayModePtr next = pMode->next;
	    int width, height;

	    if (sscanf(pScrn->display->modes[i], "%dx%d", &width, &height) == 2 &&
		width == pMode->HDisplay && height == pMode->VDisplay) {
		pMode->name = strdup(pScrn->display->modes[i]);

		pMode->prev->next = pMode->next;
		pMode->next->prev = pMode->prev;

		if (pScrn->modes == NULL) {
		    pScrn->modes = pMode;
		    pMode->next = pMode->prev = pMode;
		}
		else {
		    tmp = pScrn->modes;

		    tmp->prev = pMode;
		    while (tmp->next != pScrn->modes)
			tmp = tmp->next;
		    pMode->prev = tmp;
		    tmp->next = pMode;
		    pMode->next = pScrn->modes;
		}
		if (pMode == pScrn->modePool)
		    pScrn->modePool = (next == pMode) ? NULL : next;
		break;
	    }
	    pMode = next;
	} while (pMode != pScrn->modePool && pScrn->modePool != NULL);
    }

    if (pScrn->modes == NULL)
	pScrn->modes = pScrn->modePool;
    tmp = pScrn->modes;
    do {
	mode = ((ModeInfoData*)tmp->Private)->data;
	if (mode->XResolution > pScrn->virtualX) {
	    pScrn->virtualX = mode->XResolution;
	    pVesa->maxBytesPerScanline = mode->BytesPerScanline;
	}
	if (mode->YResolution > pScrn->virtualY)
	    pScrn->virtualY = mode->YResolution;
    } while ((tmp = tmp->next) != pScrn->modes);

    if (pVesa->monitor != NULL) {
	pMode = pScrn->modes;

	do {
	    int maxClock = 0;
	    DisplayModePtr last = pScrn->monitor->Modes;

	    for (i = 0; i < 4; i++)
		if (pVesa->monitor->det_mon[i].type == DT &&
		    pVesa->monitor->det_mon[i].section.d_timings.h_active ==
		    	pMode->HDisplay &&
		    pVesa->monitor->det_mon[i].section.d_timings.v_active ==
		        pMode->VDisplay) {
		    maxClock = pVesa->monitor->
			det_mon[i].section.d_timings.clock / 1000;
		    break;
		}

	    tmp = NULL;
	    if (maxClock) {
		for (; last != NULL; last = last->next) {
		    if (pMode->name != NULL &&
			strcmp(pMode->name, last->name) == 0 &&
			last->Clock <= maxClock) {
			tmp = last;
			/* keep looping to find the best refresh */
		    }
		}
	    }

	    if (tmp != NULL) {
		int from = (int)(&((DisplayModePtr)0)->Clock);
		int to = (int)(&((DisplayModePtr)0)->ClockIndex);

		data->mode |= (1 << 11);

		/* copy the "interesting" information */
		memcpy((char*)pMode + from, (char*)tmp + from, to - from);
		data = (ModeInfoData*)pMode->Private;
		data->block = xcalloc(sizeof(VbeCRTCInfoBlock), 1);
		data->block->HorizontalTotal = pMode->HTotal;
		data->block->HorizontalSyncStart = pMode->HSyncStart;
		data->block->HorizontalSyncEnd = pMode->HSyncEnd;
		data->block->VerticalTotal = pMode->VTotal;
		data->block->VerticalSyncStart = pMode->VSyncStart;
		data->block->VerticalSyncEnd = pMode->VSyncEnd;
		data->block->Flags = ((pMode->Flags & V_NHSYNC) ? CRTC_NHSYNC : 0) |
		     ((pMode->Flags & V_NVSYNC) ? CRTC_NVSYNC : 0);
		data->block->PixelClock = pMode->Clock * 1000;
		data->block->RefreshRate = ((double)(pMode->Clock * 1000) /
			(double)(pMode->HTotal * pMode->VTotal)) * 100;
	    }
	    pMode = pMode->next;
	} while (pMode != pScrn->modes);
    }

    pScrn->currentMode = pScrn->modes;
    pScrn->displayWidth = pScrn->virtualX;

    xf86PrintModes(pScrn);

    if (pScrn->modes == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No modes\n");
        vbeFree(pVesa->pVbe);
	return (FALSE);
    }

    /* options */
    xf86CollectOptions(pScrn, NULL);
    if (!(pVesa->Options = xalloc(sizeof(VESAOptions)))) {
        vbeFree(pVesa->pVbe);
	return FALSE;
    }
    memcpy(pVesa->Options, VESAOptions, sizeof(VESAOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pVesa->Options);

    /* Use shadow by default */
    if (xf86ReturnOptValBool(pVesa->Options, OPTION_SHADOW_FB, TRUE)) 
	pVesa->shadowFB = TRUE;

    mode = ((ModeInfoData*)pScrn->modes->Private)->data;
    switch (mode->MemoryModel) {
	case 0x0:	/* Text mode */
	case 0x1:	/* CGA graphics */
	case 0x2:	/* Hercules graphics */
	case 0x5:	/* Non-chain 4, 256 color */
	case 0x7:	/* YUV */
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Unsupported Memory Model: %d", mode->MemoryModel);
	    break;
	case 0x3:	/* Planar */
	    if (pVesa->shadowFB) {
		mod = "fb";
		pScrn->bitmapBitOrder = BITMAP_BIT_ORDER; 

		xf86LoaderReqSymbols("fbPictureInit", NULL);
	    }
	    else {
		switch (pScrn->bitsPerPixel) {
		    case 1:
			mod = "xf1bpp";
			reqSym = "xf1bppScreenInit";
			break;
		    case 4:
			mod = "xf4bpp";
			reqSym = "xf4bppScreenInit";
			break;
		    default:
			mod = "afb";
			reqSym = "afbScreenInit";
			break;
		}
	    }
	    break;
	case 0x4:	/* Packed pixel */
	case 0x6:	/*  Direct Color */
	    mod = "fb";
	    pScrn->bitmapBitOrder = BITMAP_BIT_ORDER; 

	    switch (pScrn->bitsPerPixel) {
		case 8:
		case 16:
		case 32:
		    break;
		case 24:
		  if (pVesa->pix24bpp == 32) {
			mod = "xf24_32bpp";
			reqSym = "cfb24_32ScreenInit";
		    }
		    break;
		default:
		    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			       "Unsupported bpp: %d", pScrn->bitsPerPixel);
		    vbeFree(pVesa->pVbe);
		    return FALSE;
	    }
	    break;
    }

    if (pVesa->shadowFB) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Using \"Shadow Framebuffer\"\n");
	if (pScrn->depth == 1) {
            mod = "mfb";
	    reqSym = "mfbScreenInit";
	}
	if (!xf86LoadSubModule(pScrn, "shadow")) {
	    vbeFree(pVesa->pVbe);
	    return (FALSE);
	}
	xf86LoaderReqSymLists(shadowSymbols, NULL);
    }

    if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
	VESAFreeRec(pScrn);
        vbeFree(pVesa->pVbe);
	return (FALSE);
    }

    if (mod) {
	if (reqSym) {
	    xf86LoaderReqSymbols(reqSym, NULL);
	} else {
	    xf86LoaderReqSymLists(fbSymbols, NULL);
	}
    }

    vbeFree(pVesa->pVbe);

    return (TRUE);
}

static Bool
VESAScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VESAPtr pVesa = VESAGetRec(pScrn);
    VisualPtr visual;
    VbeModeInfoBlock *mode;
    int flags;
    int init_picture = 0;

    if ((pVesa->pVbe = VBEInit(NULL, pVesa->pEnt->index)) == NULL)
        return (FALSE);

    if (pVesa->mapPhys == 0) {
	mode = ((ModeInfoData*)(pScrn->currentMode->Private))->data;
	pScrn->videoRam = pVesa->mapSize;
	pVesa->mapPhys = mode->PhysBasePtr;
	pVesa->mapOff = 0;
    }

    if (pVesa->mapPhys == 0) {
	pVesa->mapPhys = 0xa0000;
	pVesa->mapSize = 0x10000;
    }

    if (!VESAMapVidMem(pScrn)) {
	if (pVesa->mapPhys != 0xa0000) {
	    pVesa->mapPhys = 0xa0000;
	    pVesa->mapSize = 0x10000;
	    if (!VESAMapVidMem(pScrn))
		return (FALSE);
	}
	else
	    return (FALSE);
    }

    if (pVesa->shadowFB && (pVesa->shadowPtr =
	shadowAlloc(pScrn->virtualX, pScrn->virtualY,
		    pScrn->bitsPerPixel)) == NULL)
	return (FALSE);

    /* save current video state */
    VESASaveRestore(pScrn, MODE_SAVE);
    pVesa->savedPal = VBESetGetPaletteData(pVesa->pVbe, FALSE, 0, 256,
					    NULL, FALSE, FALSE);

    /* set first video mode */
    if (!VESASetMode(pScrn, pScrn->currentMode))
	return (FALSE);

    /* mi layer */
    miClearVisualTypes();
    if (!xf86SetDefaultVisual(pScrn, -1))
	return (FALSE);
    if (pScrn->bitsPerPixel > 8) {
	if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
			      pScrn->rgbBits, TrueColor))
	    return (FALSE);
    }
    else {
	if (!miSetVisualTypes(pScrn->depth,
			      miGetDefaultVisualMask(pScrn->depth),
			      pScrn->rgbBits, pScrn->defaultVisual))
	    return (FALSE);
    }
    if (!miSetPixmapDepths())
	return (FALSE);

    mode = ((ModeInfoData*)pScrn->modes->Private)->data;
    switch (mode->MemoryModel) {
	case 0x0:	/* Text mode */
	case 0x1:	/* CGA graphics */
	case 0x2:	/* Hercules graphics */
	case 0x5:	/* Non-chain 4, 256 color */
	case 0x7:	/* YUV */
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Unsupported Memory Model: %d", mode->MemoryModel);
	    return (FALSE);
	case 0x3:	/* Planar */
	    if (pVesa->shadowFB) {
	        if (pScrn->depth == 1) {
		    if (!mfbScreenInit(pScreen,
				      pVesa->shadowPtr,
				      pScrn->virtualX, pScrn->virtualY,
				      pScrn->xDpi, pScrn->yDpi,
				      pScrn->displayWidth))
		        return FALSE;
		} else {
		    if (!fbScreenInit(pScreen,
				      pVesa->shadowPtr,
				      pScrn->virtualX, pScrn->virtualY,
				      pScrn->xDpi, pScrn->yDpi,
				      pScrn->displayWidth, pScrn->bitsPerPixel))
		    return (FALSE);
		    init_picture = 1;
		}
	    } else {
		switch (pScrn->bitsPerPixel) {
		    case 1:
			if (!xf1bppScreenInit(pScreen, pVesa->base,
					      pScrn->virtualX, pScrn->virtualY,
					      pScrn->xDpi, pScrn->yDpi,
					      pScrn->displayWidth))
			    return (FALSE);
			break;
		    case 4:
			if (!xf4bppScreenInit(pScreen, pVesa->base,
					      pScrn->virtualX, pScrn->virtualY,
					      pScrn->xDpi, pScrn->yDpi,
					      pScrn->displayWidth))
			    return (FALSE);
			break;
		    default:
			if (!afbScreenInit(pScreen, pVesa->base,
					   pScrn->virtualX, pScrn->virtualY,
					   pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth))
			    return (FALSE);
			break;
		}
	    }
	    break;
	case 0x4:	/* Packed pixel */
	case 0x6:	/*  Direct Color */
	    switch (pScrn->bitsPerPixel) {
		case 24:
		    if (pVesa->pix24bpp == 32) {
		        if (!cfb24_32ScreenInit(pScreen,
			    pVesa->shadowFB ? pVesa->shadowPtr : pVesa->base,
						pScrn->virtualX, pScrn->virtualY,
					 	pScrn->xDpi, pScrn->yDpi,
						pScrn->displayWidth))
			    return (FALSE);
			break;
		    }
		case 8:
		case 16:
		case 32:
		    if (!fbScreenInit(pScreen,
			    pVesa->shadowFB ? pVesa->shadowPtr : pVesa->base,
				       pScrn->virtualX, pScrn->virtualY,
				       pScrn->xDpi, pScrn->yDpi,
				       pScrn->displayWidth, pScrn->bitsPerPixel))
			return (FALSE);
		    init_picture = 1;
		    break;
		default:
		    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			       "Unsupported bpp: %d", pScrn->bitsPerPixel);
		    return (FALSE);
	    }
	    break;
    }


    if (pScrn->bitsPerPixel > 8) {
	/* Fixup RGB ordering */
	visual = pScreen->visuals + pScreen->numVisuals;
	while (--visual >= pScreen->visuals) {
	    if ((visual->class | DynamicClass) == DirectColor) {
		visual->offsetRed   = pScrn->offset.red;
		visual->offsetGreen = pScrn->offset.green;
		visual->offsetBlue  = pScrn->offset.blue;
		visual->redMask     = pScrn->mask.red;
		visual->greenMask   = pScrn->mask.green;
		visual->blueMask    = pScrn->mask.blue;
	    }
	}
    }

    /* must be after RGB ordering fixed */
    if (init_picture)
	fbPictureInit(pScreen, 0, 0);

    if (pVesa->shadowFB) {
	ShadowUpdateProc update;
	ShadowWindowProc window;

	if (mode->MemoryModel == 3) {	/* Planar */
	  if (pScrn->bitsPerPixel == 8)
		update = shadowUpdatePlanar4x8;
	    else
		update = shadowUpdatePlanar4;
	    window = VESAWindowPlanar;
	}
	else if (pVesa->mapPhys == 0xa0000) {	/* Windowed */
	    update = shadowUpdatePacked;
	    window = VESAWindowWindowed;
	}
	else {	/* Linear */
	    update = shadowUpdatePacked;
	    window = VESAWindowLinear;
	}

	if (!shadowInit(pScreen, update, window))
	    return (FALSE);
    }
    else if (pVesa->mapPhys == 0xa0000 && mode->MemoryModel != 0x3) {
	unsigned int bankShift = 0;
	while ((unsigned)(64 >> bankShift) != mode->WinGranularity)
	    bankShift++;
	pVesa->curBank = -1;
	pVesa->bank.SetSourceBank =
	pVesa->bank.SetDestinationBank =
	pVesa->bank.SetSourceAndDestinationBanks = VESABankSwitch;
	pVesa->bank.pBankA = pVesa->bank.pBankB = pVesa->base;
	pVesa->bank.BankSize = (mode->WinSize * 1024) >> bankShift;
	pVesa->bank.nBankDepth = pScrn->depth;
	if (!miInitializeBanking(pScreen, pScrn->virtualX, pScrn->virtualY,
				 pScrn->virtualX, &pVesa->bank)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Bank switch initialization failed!\n");
	    return (FALSE);
	}
    }

    VESADGAInit(pScrn, pScreen);

    xf86SetBlackWhitePixels(pScreen);
    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);

    /* software cursor */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* colormap */
    if (!miCreateDefColormap(pScreen))
	return (FALSE);

    flags = CMAP_RELOAD_ON_MODE_SWITCH;

    if(!xf86HandleColormaps(pScreen, 256,
	pVesa->vbeInfo->Capabilities[0] & 0x01 ? 8 : 6,
	VESALoadPalette, NULL, flags))
	return (FALSE);

    pVesa->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = VESACloseScreen;
    pScreen->SaveScreen = VESASaveScreen;

    xf86DPMSInit(pScreen, VESADisplayPowerManagementSet, 0);

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1)
        xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

    return (TRUE);
}

static Bool
VESAEnterVT(int scrnIndex, int flags)
{
    return (VESASetMode(xf86Screens[scrnIndex],
			xf86Screens[scrnIndex]->currentMode));
}

static void
VESALeaveVT(int scrnIndex, int flags)
{
    VESASaveRestore(xf86Screens[scrnIndex], MODE_RESTORE);
}

static Bool
VESACloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VESAPtr pVesa = VESAGetRec(pScrn);

    if (pScrn->vtSema) {
	VESASaveRestore(xf86Screens[scrnIndex], MODE_RESTORE);
	VBESetGetPaletteData(pVesa->pVbe, TRUE, 0, 256,
			     pVesa->savedPal, FALSE, TRUE);
	VESAUnmapVidMem(pScrn);
    }
    if (pVesa->shadowPtr) {
	xfree(pVesa->shadowPtr);
	pVesa->shadowPtr = NULL;
    }
    if (pVesa->pDGAMode) {
	xfree(pVesa->pDGAMode);
	pVesa->pDGAMode = NULL;
	pVesa->nDGAMode = 0;
    }
    pScrn->vtSema = FALSE;

    pScreen->CloseScreen = pVesa->CloseScreen;
    return pScreen->CloseScreen(scrnIndex, pScreen);
}

static Bool
VESASwitchMode(int scrnIndex, DisplayModePtr pMode, int flags)
{
    return VESASetMode(xf86Screens[scrnIndex], pMode);
}

/* Set a graphics mode */
static Bool
VESASetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode)
{
    VESAPtr pVesa;
    ModeInfoData *data;
    int mode;

    pVesa = VESAGetRec(pScrn);

    data = (ModeInfoData*)pMode->Private;

    mode = data->mode | (1 << 15);

    /* enable linear addressing */
    if (pVesa->mapPhys != 0xa0000)
	mode |= 1 << 14;

    if (VBESetVBEMode(pVesa->pVbe, mode, data->block) == FALSE) {
	if ((data->block || (data->mode & (1 << 11))) &&
	    VBESetVBEMode(pVesa->pVbe, (mode & ~(1 << 11)), NULL) == TRUE) {
	    /* Some cards do not like setting the clock.
	     * Free it as it will not be any longer useful
	     */
	    xfree(data->block);
	    data->block = NULL;
	    data->mode &= ~(1 << 11);
	}
	else {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Set VBE Mode failed!\n");
	    return (FALSE);
	}
    }

    pVesa->bankSwitchWindowB =
	!((data->data->WinBSegment == 0) && (data->data->WinBAttributes == 0));

    if (data->data->XResolution != pScrn->virtualX)
	VBESetLogicalScanline(pVesa->pVbe, pScrn->virtualX);

    if (pScrn->bitsPerPixel >= 8 && pVesa->vbeInfo->Capabilities[0] & 0x01)
	VBESetGetDACPaletteFormat(pVesa->pVbe, 8);

    pScrn->vtSema = TRUE;

    return (TRUE);
}

static void
VESAAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    VESAPtr pVesa = VESAGetRec(xf86Screens[scrnIndex]);

    VBESetDisplayStart(pVesa->pVbe, x, y, TRUE);
}

static void
VESAFreeScreen(int scrnIndex, int flags)
{
    VESAFreeRec(xf86Screens[scrnIndex]);
}

static Bool
VESAMapVidMem(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa = VESAGetRec(pScrn);

    if (pVesa->base != NULL)
	return (TRUE);

    pScrn->memPhysBase = pVesa->mapPhys;
    pScrn->fbOffset = pVesa->mapOff;

    if (pVesa->mapPhys != 0xa0000 && pVesa->pEnt->location.type == BUS_PCI)
	pVesa->base = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER,
				    pVesa->pciTag, pScrn->memPhysBase,
				    pVesa->mapSize);
    else
	pVesa->base = xf86MapVidMem(pScrn->scrnIndex, 0,
				    pScrn->memPhysBase, pVesa->mapSize);

    if (pVesa->base) {
	if (pVesa->mapPhys != 0xa0000)
	    pVesa->VGAbase = xf86MapVidMem(pScrn->scrnIndex, 0,
				0xa0000, 0x10000);
	else
	    pVesa->VGAbase = pVesa->base;
    }
    xf86ErrorFVerb(DEBUG_VERB,
	"virtual address = %p  -  physical address = %p  -  size = %d\n",
	    pVesa->base, pScrn->memPhysBase, pVesa->mapSize);

    return (pVesa->base != NULL);
}

static void
VESAUnmapVidMem(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa = VESAGetRec(pScrn);

    if (pVesa->base == NULL)
	return;

    xf86UnMapVidMem(pScrn->scrnIndex, pVesa->base, pVesa->mapSize);
    if (pVesa->mapPhys != 0xa0000)
	xf86UnMapVidMem(pScrn->scrnIndex, pVesa->VGAbase, 0x10000);
    pVesa->base = NULL;
}

void *
VESAWindowPlanar(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		 CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VESAPtr pVesa = VESAGetRec(pScrn);
    VbeModeInfoBlock *data = ((ModeInfoData*)(pScrn->currentMode->Private))->data;
    int window;
    int mask = 1 << (offset & 3);

    outb(0x3c4, 2);
    outb(0x3c5, mask);
    offset = (offset >> 2) + pVesa->maxBytesPerScanline * row;
    window = offset / (data->WinGranularity * 1024);
    pVesa->windowAoffset = window * data->WinGranularity * 1024;
    VESABankSwitch(pScreen, window);
    *size = data->WinSize * 1024 - (offset - pVesa->windowAoffset);

    return (void *)((unsigned long)pVesa->base +
		   (offset - pVesa->windowAoffset));
}

static void *
VESAWindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		 CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VESAPtr pVesa = VESAGetRec(pScrn);

    *size = pVesa->maxBytesPerScanline;
    return ((CARD8 *)pVesa->base + row * pVesa->maxBytesPerScanline + offset);
}

static void *
VESAWindowWindowed(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		   CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VESAPtr pVesa = VESAGetRec(pScrn);
    VbeModeInfoBlock *data = ((ModeInfoData*)(pScrn->currentMode->Private))->data;
    int window;

    offset += pVesa->maxBytesPerScanline * row;
    window = offset / (data->WinGranularity * 1024);
    pVesa->windowAoffset = window * data->WinGranularity * 1024;
    VESABankSwitch(pScreen, window);
    *size = data->WinSize * 1024 - (offset - pVesa->windowAoffset);

    return (void *)((unsigned long)pVesa->base +
		    (offset - pVesa->windowAoffset));
}

static void
VESALoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
		LOCO *colors, VisualPtr pVisual)
{
#if 0
    /* This code works, but is very slow for programs that use it intensively */
    VESAPtr pVesa = VESAGetRec(pScrn);
    int i, idx, base;

    if (pVesa->pal == NULL)
	pVesa->pal = xcalloc(1, sizeof(CARD32) * 256);

    for (i = 0, base = idx = indices[i]; i < numColors; i++, idx++) {
	int j = indices[i];

	if (j < 0 || j >= 256)
	    continue;
	pVesa->pal[j] = colors[j].blue |
			(colors[j].green << 8) |
			(colors[j].red << 16);
	if (j != idx) {
	    VBESetGetPaletteData(pVesa->pVbe, TRUE, base, idx - base,
				  pVesa->pal + base, FALSE, TRUE);
	    idx = base = j;
	}
    }

    if (idx - 1 == indices[i - 1])
	VBESetGetPaletteData(pVesa->pVbe, TRUE, base, idx - base,
			      pVesa->pal + base, FALSE, TRUE);
#else
#define WriteDacWriteAddr(value)	outb(VGA_DAC_WRITE_ADDR, value)
#define WriteDacData(value)		outb(VGA_DAC_DATA, value);
#undef DACDelay
#define DACDelay()								\
	do {									\
	    unsigned char temp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);	\
	    temp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);		\
	} while (0)
    int i, idx;

    for (i = 0; i < numColors; i++) {
	idx = indices[i];
	WriteDacWriteAddr(idx);
	DACDelay();
	WriteDacData(colors[idx].red);
	DACDelay();
	WriteDacData(colors[idx].green);
	DACDelay();
	WriteDacData(colors[idx].blue);
	DACDelay();
    }
#endif
}

/*
 * Just adapted from the std* functions in vgaHW.c
 */
static void
WriteAttr(int index, int value)
{
    CARD8 tmp;

    tmp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

    index |= 0x20;
    outb(VGA_ATTR_INDEX, index);
    outb(VGA_ATTR_DATA_W, value);
}

static int
ReadAttr(int index)
{
    CARD8 tmp;

    tmp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

    index |= 0x20;
    outb(VGA_ATTR_INDEX, index);
    return (inb(VGA_ATTR_DATA_R));
}

#define WriteMiscOut(value)	outb(VGA_MISC_OUT_W, value)
#define ReadMiscOut()		inb(VGA_MISC_OUT_R)
#define WriteSeq(index, value)	outb(VGA_SEQ_INDEX, index);\
				outb(VGA_SEQ_DATA, value)

static int
ReadSeq(int index)
{
    outb(VGA_SEQ_INDEX, index);

    return (inb(VGA_SEQ_DATA));
}

#define WriteGr(index, value)	outb(VGA_GRAPH_INDEX, index);\
				outb(VGA_GRAPH_DATA, value)
static int
ReadGr(int index)
{
    outb(VGA_GRAPH_INDEX, index);

    return (inb(VGA_GRAPH_DATA));
}

#define WriteCrtc(index, value)	outb(VGA_CRTC_INDEX_OFFSET, index);\
				outb(VGA_CRTC_DATA_OFFSET, value)

static int
ReadCrtc(int index)
{
    outb(VGA_CRTC_INDEX_OFFSET, index);
    return inb(VGA_CRTC_DATA_OFFSET);
}

static void
SeqReset(Bool start)
{
    if (start) {
	WriteSeq(0x00, 0x01);		/* Synchronous Reset */
    }
    else {
	WriteSeq(0x00, 0x03);		/* End Reset */
    }
}

static void
SaveFonts(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa = VESAGetRec(pScrn);
    unsigned char miscOut, attr10, gr4, gr5, gr6, seq2, seq4, scrn;

    if (pVesa->fonts != NULL)
	return;

    /* If in graphics mode, don't save anything */
    attr10 = ReadAttr(0x10);
    if (attr10 & 0x01)
	return;

    pVesa->fonts = xalloc(16384);

    /* save the registers that are needed here */
    miscOut = ReadMiscOut();
    gr4 = ReadGr(0x04);
    gr5 = ReadGr(0x05);
    gr6 = ReadGr(0x06);
    seq2 = ReadSeq(0x02);
    seq4 = ReadSeq(0x04);

    /* Force into colour mode */
    WriteMiscOut(miscOut | 0x01);

    scrn = ReadSeq(0x01) | 0x20;
    SeqReset(TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(FALSE);

    WriteAttr(0x10, 0x01);	/* graphics mode */

    /*font1 */
    WriteSeq(0x02, 0x04);	/* write to plane 2 */
    WriteSeq(0x04, 0x06);	/* enable plane graphics */
    WriteGr(0x04, 0x02);	/* read plane 2 */
    WriteGr(0x05, 0x00);	/* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);	/* set graphics */
    slowbcopy_frombus(pVesa->VGAbase, pVesa->fonts, 8192);

    /* font2 */
    WriteSeq(0x02, 0x08);	/* write to plane 3 */
    WriteSeq(0x04, 0x06);	/* enable plane graphics */
    WriteGr(0x04, 0x03);	/* read plane 3 */
    WriteGr(0x05, 0x00);	/* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);	/* set graphics */
    slowbcopy_frombus(pVesa->VGAbase, pVesa->fonts + 8192, 8192);

    scrn = ReadSeq(0x01) & ~0x20;
    SeqReset(TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(FALSE);

    /* Restore clobbered registers */
    WriteAttr(0x10, attr10);
    WriteSeq(0x02, seq2);
    WriteSeq(0x04, seq4);
    WriteGr(0x04, gr4);
    WriteGr(0x05, gr5);
    WriteGr(0x06, gr6);
    WriteMiscOut(miscOut);
}

static void
RestoreFonts(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa = VESAGetRec(pScrn);
    unsigned char miscOut, attr10, gr1, gr3, gr4, gr5, gr6, gr8, seq2, seq4, scrn;

    if (pVesa->fonts == NULL)
	return;

    if (pVesa->mapPhys == 0xa0000 && pVesa->curBank != 0)
	VESABankSwitch(pScrn->pScreen, 0);

    /* save the registers that are needed here */
    miscOut = ReadMiscOut();
    attr10 = ReadAttr(0x10);
    gr1 = ReadGr(0x01);
    gr3 = ReadGr(0x03);
    gr4 = ReadGr(0x04);
    gr5 = ReadGr(0x05);
    gr6 = ReadGr(0x06);
    gr8 = ReadGr(0x08);
    seq2 = ReadSeq(0x02);
    seq4 = ReadSeq(0x04);

    /* Force into colour mode */
    WriteMiscOut(miscOut | 0x01);

    scrn = ReadSeq(0x01) | 0x20;
    SeqReset(TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(FALSE);

    WriteAttr(0x10, 0x01);	/* graphics mode */
    if (pScrn->depth == 4) {
	/* GJA */
	WriteGr(0x03, 0x00);	/* don't rotate, write unmodified */
	WriteGr(0x08, 0xFF);	/* write all bits in a byte */
	WriteGr(0x01, 0x00);	/* all planes come from CPU */
    }

    WriteSeq(0x02, 0x04);   /* write to plane 2 */
    WriteSeq(0x04, 0x06);   /* enable plane graphics */
    WriteGr(0x04, 0x02);    /* read plane 2 */
    WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);    /* set graphics */
    slowbcopy_tobus(pVesa->fonts, pVesa->VGAbase, 8192);

    WriteSeq(0x02, 0x08);   /* write to plane 3 */
    WriteSeq(0x04, 0x06);   /* enable plane graphics */
    WriteGr(0x04, 0x03);    /* read plane 3 */
    WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
    WriteGr(0x06, 0x05);    /* set graphics */
    slowbcopy_tobus(pVesa->fonts + 8192, pVesa->VGAbase, 8192);

    scrn = ReadSeq(0x01) & ~0x20;
    SeqReset(TRUE);
    WriteSeq(0x01, scrn);
    SeqReset(FALSE);

    /* restore the registers that were changed */
    WriteMiscOut(miscOut);
    WriteAttr(0x10, attr10);
    WriteGr(0x01, gr1);
    WriteGr(0x03, gr3);
    WriteGr(0x04, gr4);
    WriteGr(0x05, gr5);
    WriteGr(0x06, gr6);
    WriteGr(0x08, gr8);
    WriteSeq(0x02, seq2);
    WriteSeq(0x04, seq4);
}

static Bool
VESASaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Bool on = xf86IsUnblank(mode);

    if (on)
	SetTimeSinceLastInputEvent();

    if (pScrn->vtSema) {
	unsigned char scrn = ReadSeq(0x01);

	if (on)
	    scrn &= ~0x20;
	else
	    scrn |= 0x20;
	SeqReset(TRUE);
	WriteSeq(0x01, scrn);
	SeqReset(FALSE);
    }

    return (TRUE);
}

static int 
VESABankSwitch(ScreenPtr pScreen, unsigned int iBank)
{
    VESAPtr pVesa;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    pVesa = VESAGetRec(pScrn);
    if (pVesa->curBank == iBank)
	return (0);
    if (!VBEBankSwitch(pVesa->pVbe, iBank, 0))
        return (1);
    if (pVesa->bankSwitchWindowB) {
        if (!VBEBankSwitch(pVesa->pVbe, iBank, 1))
	   return (1);
    }
    pVesa->curBank = iBank;

    return (0);
}

Bool
VESASaveRestore(ScrnInfoPtr pScrn, vbeSaveRestoreFunction function)
{
    VESAPtr pVesa;

    if (MODE_QUERY < 0 || function > MODE_RESTORE)
	return (FALSE);

    pVesa = VESAGetRec(pScrn);


    /* Query amount of memory to save state */
    if (function == MODE_QUERY ||
	(function == MODE_SAVE && pVesa->state == NULL)) {

	/* Make sure we save at least this information in case of failure */
	(void)VBEGetVBEMode(pVesa->pVbe, &pVesa->stateMode);
	SaveFonts(pScrn);

	if (pVesa->major > 1) {
	    if (!VBESaveRestore(pVesa->pVbe,function,(pointer)&pVesa->state,
				&pVesa->stateSize,&pVesa->statePage))
	        return FALSE;

	}
    }

    /* Save/Restore Super VGA state */
    if (function != MODE_QUERY) {
        Bool retval = TRUE;

	if (pVesa->major > 1) {
	    if (function == MODE_RESTORE)
		memcpy(pVesa->state, pVesa->pstate, pVesa->stateSize);

	    if ((retval = VBESaveRestore(pVesa->pVbe,function,
					 (pointer)&pVesa->state,
					 &pVesa->stateSize,&pVesa->statePage))
		&& function == MODE_SAVE) {
	        /* don't rely on the memory not being touched */
	        if (pVesa->pstate == NULL)
		    pVesa->pstate = xalloc(pVesa->stateSize);
		memcpy(pVesa->pstate, pVesa->state, pVesa->stateSize);
	    }
	}

	if (function == MODE_RESTORE) {
	    VBESetVBEMode(pVesa->pVbe, pVesa->stateMode, NULL);
	    RestoreFonts(pScrn);
	}

	if (!retval)
	    return (FALSE);

    }

    return (TRUE);
}

static void
VESADisplayPowerManagementSet(ScrnInfoPtr pScrn, int mode,
                int flags)
{
#if 0
   /* XXX How can this work without the vgahw module being initialized? */
   vgaHWDPMSSet(pScrn, mode, flags);
#else
    unsigned char seq1 = 0, crtc17 = 0;

    if (!pScrn->vtSema)
	return;

    switch (mode) {
	case DPMSModeOn:
	    /* Screen: On; HSync: On, VSync: On */
	    seq1 = 0x00;
	    crtc17 = 0x80;
	    break;
	case DPMSModeStandby:
	    /* Screen: Off; HSync: Off, VSync: On -- Not Supported */
	    seq1 = 0x20;
	    crtc17 = 0x80;
	    break;
	case DPMSModeSuspend:
	    /* Screen: Off; HSync: On, VSync: Off -- Not Supported */
	    seq1 = 0x20;
	    crtc17 = 0x80;
	    break;
	case DPMSModeOff:
	    /* Screen: Off; HSync: Off, VSync: Off */
	    seq1 = 0x20;
	    crtc17 = 0x00;
	    break;
    }
    WriteSeq(0x00, 0x01);		  /* Synchronous Reset */
    seq1 |= ReadSeq(0x01) & ~0x20;
    WriteSeq(0x01, seq1);
    crtc17 |= ReadCrtc(0x17) & ~0x80;
    usleep(10000);
    WriteCrtc(0x17, crtc17);
    WriteSeq(0x00, 0x03);		  /* End Reset */
#endif
}




/***********************************************************************
 * DGA stuff
 ***********************************************************************/
static Bool VESADGAOpenFramebuffer(ScrnInfoPtr pScrn, char **DeviceName,
				   unsigned char **ApertureBase,
				   int *ApertureSize, int *ApertureOffset,
				   int *flags);
static Bool VESADGASetMode(ScrnInfoPtr pScrn, DGAModePtr pDGAMode);
static void VESADGASetViewport(ScrnInfoPtr pScrn, int x, int y, int flags);

static Bool
VESADGAOpenFramebuffer(ScrnInfoPtr pScrn, char **DeviceName,
		       unsigned char **ApertureBase, int *ApertureSize,
		       int *ApertureOffset, int *flags)
{
    VESAPtr pVesa = VESAGetRec(pScrn);

    *DeviceName = NULL;		/* No special device */
    *ApertureBase = (unsigned char *)(long)(pVesa->mapPhys);
    *ApertureSize = pVesa->mapSize;
    *ApertureOffset = pVesa->mapOff;
    *flags = DGA_NEED_ROOT;

    return (TRUE);
}

static Bool
VESADGASetMode(ScrnInfoPtr pScrn, DGAModePtr pDGAMode)
{
    DisplayModePtr pMode;
    int scrnIdx = pScrn->pScreen->myNum;
    int frameX0, frameY0;

    if (pDGAMode) {
	pMode = pDGAMode->mode;
	frameX0 = frameY0 = 0;
    }
    else {
	if (!(pMode = pScrn->currentMode))
	    return (TRUE);

	frameX0 = pScrn->frameX0;
	frameY0 = pScrn->frameY0;
    }

    if (!(*pScrn->SwitchMode)(scrnIdx, pMode, 0))
	return (FALSE);
    (*pScrn->AdjustFrame)(scrnIdx, frameX0, frameY0, 0);

    return (TRUE);
}

static void
VESADGASetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
    (*pScrn->AdjustFrame)(pScrn->pScreen->myNum, x, y, flags);
}

static int
VESADGAGetViewport(ScrnInfoPtr pScrn)
{
    return (0);
}

static DGAFunctionRec VESADGAFunctions =
{
    VESADGAOpenFramebuffer,
    NULL,       /* CloseFramebuffer */
    VESADGASetMode,
    VESADGASetViewport,
    VESADGAGetViewport,
    NULL,       /* Sync */
    NULL,       /* FillRect */
    NULL,       /* BlitRect */
    NULL,       /* BlitTransRect */
};

static void
VESADGAAddModes(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa = VESAGetRec(pScrn);
    DisplayModePtr pMode = pScrn->modes;
    DGAModePtr pDGAMode;

    do {
	pDGAMode = xrealloc(pVesa->pDGAMode,
			    (pVesa->nDGAMode + 1) * sizeof(DGAModeRec));
	if (!pDGAMode)
	    break;

	pVesa->pDGAMode = pDGAMode;
	pDGAMode += pVesa->nDGAMode;
	(void)memset(pDGAMode, 0, sizeof(DGAModeRec));

	++pVesa->nDGAMode;
	pDGAMode->mode = pMode;
	pDGAMode->flags = DGA_CONCURRENT_ACCESS | DGA_PIXMAP_AVAILABLE;
	pDGAMode->byteOrder = pScrn->imageByteOrder;
	pDGAMode->depth = pScrn->depth;
	pDGAMode->bitsPerPixel = pScrn->bitsPerPixel;
	pDGAMode->red_mask = pScrn->mask.red;
	pDGAMode->green_mask = pScrn->mask.green;
	pDGAMode->blue_mask = pScrn->mask.blue;
	pDGAMode->visualClass = pScrn->bitsPerPixel > 8 ?
	    TrueColor : PseudoColor;
	pDGAMode->xViewportStep = 1;
	pDGAMode->yViewportStep = 1;
	pDGAMode->viewportWidth = pMode->HDisplay;
	pDGAMode->viewportHeight = pMode->VDisplay;

	pDGAMode->bytesPerScanline = pVesa->maxBytesPerScanline;
	pDGAMode->imageWidth = pMode->HDisplay;
	pDGAMode->imageHeight =  pMode->VDisplay;
	pDGAMode->pixmapWidth = pDGAMode->imageWidth;
	pDGAMode->pixmapHeight = pDGAMode->imageHeight;
	pDGAMode->maxViewportX = pScrn->virtualX -
				    pDGAMode->viewportWidth;
	pDGAMode->maxViewportY = pScrn->virtualY -
				    pDGAMode->viewportHeight;

	pDGAMode->address = pVesa->base;

	pMode = pMode->next;
    } while (pMode != pScrn->modes);
}

static Bool
VESADGAInit(ScrnInfoPtr pScrn, ScreenPtr pScreen)
{
    VESAPtr pVesa = VESAGetRec(pScrn);

    if (pScrn->depth < 8 || pVesa->mapPhys == 0xa0000L)
	return (FALSE);

    if (!pVesa->nDGAMode)
	VESADGAAddModes(pScrn);

    return (DGAInit(pScreen, &VESADGAFunctions,
	    pVesa->pDGAMode, pVesa->nDGAMode));
}
