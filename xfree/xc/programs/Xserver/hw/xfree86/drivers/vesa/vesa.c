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
 * $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vesa/vesa.c,v 1.8 2000/12/02 15:31:00 tsi Exp $
 */

#include "vesa.h"

/* All drivers initialising the SW cursor need this */
#include "mipointer.h"

/* All drivers implementing backing store need this */
#include "mibstore.h"

/* Colormap handling */
#include "micmap.h"
#include "xf86cmap.h"

/* Mandatory functions */
static OptionInfoPtr VESAAvailableOptions(int chipid, int busid);
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

/* locally used functions */
static int VESAFindIsaDevice(GDevPtr dev);
static Bool VESAMapVidMem(ScrnInfoPtr pScrn);
static void VESAUnmapVidMem(ScrnInfoPtr pScrn);
static int VESABankSwitch(ScreenPtr pScreen, unsigned int iBank);
static void VESALoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
			    LOCO *colors, VisualPtr pVisual);
static void SaveFonts(ScrnInfoPtr pScrn);
static void RestoreFonts(ScrnInfoPtr pScrn);

static void *VESAWindowPlanar(ScreenPtr pScrn, CARD32 row, CARD32 offset,
			      int mode, CARD32 *size);
static void *VESAWindowLinear(ScreenPtr pScrn, CARD32 row, CARD32 offset,
			      int mode, CARD32 *size);
static void *VESAWindowWindowed(ScreenPtr pScrn, CARD32 row, CARD32 offset,
				int mode, CARD32 *size);

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

static OptionInfoRec VESAOptions[] = {
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
static const char *fbSymbols[] = {
    "xf1bppScreenInit",
    "xf4bppScreenInit",
    "afbScreenInit",
    "fbScreenInit",
#ifdef RENDER
    "fbPictureInit",
#endif
    "cfbScreenInit",
    "mfbScreenInit",
    NULL
};

static const char *shadowSymbols[] = {
    "ShadowInit",
    NULL
};

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
 * followed by "ModuleInit".
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
	LoaderRefSymLists(fbSymbols, shadowSymbols, NULL);
	return (pointer)TRUE;
    }

    if (ErrorMajor)
	*ErrorMajor = LDR_ONCEONLY;
    return (NULL);
}

#endif

static
OptionInfoPtr
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
    VBEInfoBlock *vbe;
    DisplayModePtr pMode, tmp;
    ModeInfoBlock *mode;
    ModeInfoData *data = NULL;
    char *mod = NULL;
    const char *reqSym = NULL;
    Gamma gzeros = {0.0, 0.0, 0.0};
    rgb rzeros = {0, 0, 0};
    vbeInfoPtr pVbe;
    pointer pVbeModule, pDDCModule;
    int i;

    if (flags & PROBE_DETECT)
	return (FALSE);

    /* Load int10 module */
    if (!xf86LoadSubModule(pScrn, "int10"))
	return (FALSE);

    pVesa = VESAGetRec(pScrn);
    pVesa->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
    pVesa->device = xf86GetDevFromEntity(pScrn->entityList[0],
					 pScrn->entityInstanceList[0]);
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

    if (!xf86SetDepthBpp(pScrn, 8, 8, 8, Support24bppFb))
	return (FALSE);
    xf86PrintDepthBpp(pScrn);

    /* Get the depth24 pixmap format */
    if (pScrn->depth == 24 && pVesa->pix24bpp == 0)
	pVesa->pix24bpp = xf86GetBppFromDepth(pScrn, 24);

    /* color weight */
    if (pScrn->depth > 8 && !xf86SetWeight(pScrn, rzeros, rzeros))
	return (FALSE);

    /* visual init */
    if (!xf86SetDefaultVisual(pScrn, -1))
	return (FALSE);

    xf86SetGamma(pScrn, gzeros);

    if ((pVesa->pInt = xf86InitInt10(pVesa->pEnt->index)) == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Int10 initialization failed.\n");
	return (FALSE);
    }

    if ((pVesa->block = xf86Int10AllocPages(pVesa->pInt, 1, &pVesa->page)) == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Cannot allocate one scratch page in real mode memory.\n");
	return (FALSE);
    }

    vbe = VESAGetVBEInfo(pScrn);
    pVesa->major = (unsigned)(vbe->VESAVersion >> 8);
    pVesa->minor = vbe->VESAVersion & 0xff;
    pVesa->vbeInfo = vbe;
    pScrn->videoRam = vbe->TotalMemory * 64 * 1024;

    if (pVesa->major >= 2) {
	/* Load vbe module */
	if ((pVbeModule = xf86LoadSubModule(pScrn, "vbe")) == NULL)
	    return (FALSE);
	if ((pVbe = VBEInit(pVesa->pInt, pVesa->pEnt->index)) == NULL)
	    return (FALSE);

	/* Load ddc module */
	if ((pDDCModule = xf86LoadSubModule(pScrn, "ddc")) == NULL)
	    return (FALSE);

	if ((pVesa->monitor = vbeDoEDID(pVbe, pDDCModule)) != NULL) {
	    xf86PrintEDID(pVesa->monitor);
#ifdef DEBUG
	    ErrorF("Monitor data blocks:\n");
	    ErrorF("VENDOR: name %s  -	id %d  -  serial %d  -	week %d  -  year %d\n",
		   pVesa->monitor->vendor.name, pVesa->monitor->vendor.prod_id,
		   pVesa->monitor->vendor.serial, pVesa->monitor->vendor.week,
		   pVesa->monitor->vendor.year);
	    ErrorF("EDID:  Version %d  -  Revision %d\n",
		   pVesa->monitor->ver.version,
		   pVesa->monitor->ver.revision);
	    ErrorF("FEATURES:\n input: type %d	-  voltage %d  -  setup %d  -  sync %d\n"
		   " size: %d x %d\n gamma: %f\n dpms: %d\n type: %d\n"
		   " misc: %d\n redx %d  -  redy %d\n greenx %d  -  greeny %d\n"
		   " bluex: %d	-  bluey %d\n whitex %d  -  whitey\n"
		   "ESTABLISHED TIMES: %d %d %d\n"
		   "STD TIMINGS:\n",
		   pVesa->monitor->features.input_type,
		   pVesa->monitor->features.input_voltage,
		   pVesa->monitor->features.input_setup,
		   pVesa->monitor->features.input_sync,
		   pVesa->monitor->features.hsize, pVesa->monitor->features.vsize,
		   pVesa->monitor->features.gamma,
		   pVesa->monitor->features.dpms,
		   pVesa->monitor->features.display_type,
		   pVesa->monitor->features.msc, pVesa->monitor->features.redx,
		   pVesa->monitor->features.redy,
		   pVesa->monitor->features.greenx, pVesa->monitor->features.greeny,
		   pVesa->monitor->features.bluex, pVesa->monitor->features.bluey,
		   pVesa->monitor->features.whitex, pVesa->monitor->features.whitey,
		   pVesa->monitor->timings1.t1,
		   pVesa->monitor->timings1.t2,
		   pVesa->monitor->timings1.t_manu);
	    for (i = 0; i < 8; i++) {
		ErrorF(" %d %d	%d  %d\n",
		       pVesa->monitor->timings2[i].hsize,
		       pVesa->monitor->timings2[i].vsize,
		       pVesa->monitor->timings2[i].refresh,
		       pVesa->monitor->timings2[i].id);
	    }
	    ErrorF("DETAILED MONITOR SECTION:\n");
	    for (i = 0; i < 4; i++) {
		int j;

		ErrorF(" type ");
		switch (pVesa->monitor->det_mon[i].type) {
		    case DT:
			ErrorF("DT\n");
			ErrorF("  clock: %d\n"
			       "  hactive: %d\n  hblanking: %d\n"
			       "  vactive: %d\n  vblanking: %d\n"
			       "  hsyncoff: %d\n  hsyncwidth: %d\n"
			       "  vsyncoff: %d\n  vsyncwidth: %d\n"
			       "  hsize: %d\n  vsize: %d\n"
			       "  hborder: %d\n  vborder: %d\n"
			       "  interlaced: %d\n  stereo: %d\n"
			       "  sync: %d\n  misc: %d\n",
			   pVesa->monitor->det_mon[i].section.d_timings.clock,
			   pVesa->monitor->det_mon[i].section.d_timings.h_active,
			   pVesa->monitor->det_mon[i].section.d_timings.h_blanking,
			   pVesa->monitor->det_mon[i].section.d_timings.v_active,
			   pVesa->monitor->det_mon[i].section.d_timings.v_blanking,
			   pVesa->monitor->det_mon[i].section.d_timings.h_sync_off,
			   pVesa->monitor->det_mon[i].section.d_timings.h_sync_width,
			   pVesa->monitor->det_mon[i].section.d_timings.v_sync_off,
			   pVesa->monitor->det_mon[i].section.d_timings.v_sync_width,
			   pVesa->monitor->det_mon[i].section.d_timings.h_size,
			   pVesa->monitor->det_mon[i].section.d_timings.v_size,
			   pVesa->monitor->det_mon[i].section.d_timings.h_border,
			   pVesa->monitor->det_mon[i].section.d_timings.v_border,
			   pVesa->monitor->det_mon[i].section.d_timings.interlaced,
			   pVesa->monitor->det_mon[i].section.d_timings.stereo,
			   pVesa->monitor->det_mon[i].section.d_timings.sync,
			   pVesa->monitor->det_mon[i].section.d_timings.misc);
			break;
		    case DS_SERIAL:
			ErrorF("SERIAL\n");
			ErrorF("  serial: %s\n", pVesa->monitor->det_mon[i].section.serial);
			break;
		    case DS_ASCII_STR:
			ErrorF("ASCII_STR\n");
			ErrorF("  ascii_str: %s\n", pVesa->monitor->det_mon[i].section.ascii_data);
			break;
		    case DS_NAME:
			ErrorF("NAME\n");
			ErrorF("  name: %s\n", pVesa->monitor->det_mon[i].section.name);
			break;
		    case DS_RANGES:
			ErrorF("RANGES\n");
			ErrorF("  ranges: minv %d  -  maxv %d  -  minh %d  -  maxh %d  -  maxclock %d\n",
			       pVesa->monitor->det_mon[i].section.ranges.min_v,
			       pVesa->monitor->det_mon[i].section.ranges.max_v,
			       pVesa->monitor->det_mon[i].section.ranges.min_h,
			       pVesa->monitor->det_mon[i].section.ranges.min_h,
			       pVesa->monitor->det_mon[i].section.ranges.max_clock);
			break;
		    case DS_WHITE_P:
			ErrorF("WHITE_P\n");
			for (j = 0; j < 2; j++)
			    ErrorF("  index %d	-  whitex %d  -  whitey %d  -  whitegamma %d\n",
				   pVesa->monitor->det_mon[i].section.wp[j].index,
				   pVesa->monitor->det_mon[i].section.wp[j].white_x,
				   pVesa->monitor->det_mon[i].section.wp[j].white_y,
				   pVesa->monitor->det_mon[i].section.wp[j].white_gamma);
			break;
		    case DS_STD_TIMINGS:
			ErrorF("STD_TIMINGS\n");
			for (j = 0; j < 5; j++)
			    ErrorF("  %d %d  %d  %d\n",
				   pVesa->monitor->det_mon[i].section.std_t[j].hsize,
				   pVesa->monitor->det_mon[i].section.std_t[j].vsize,
				   pVesa->monitor->det_mon[i].section.std_t[j].refresh,
				   pVesa->monitor->det_mon[i].section.std_t[j].id);
			break;
		    default:
			ErrorF(" UNKNOWN\n");
			break;
		}
	    }
#endif
	}

	/* unload modules */
	xf86UnloadSubModule(pVbeModule);
	xf86UnloadSubModule(pDDCModule);
    }

#ifdef DEBUG
    ErrorF("%c%c%c%c %d.%d - %s\n",
	   vbe->VESASignature[0], vbe->VESASignature[1],
	   vbe->VESASignature[2], vbe->VESASignature[3],
	   pVesa->major, pVesa->minor, vbe->OEMStringPtr);

    if (pVesa->major >= 2)
	ErrorF("Vendor: %s\nProduct: %s\nProductRev: %s\nSoftware Rev: %d.%d\n",
	       vbe->OemVendorNamePtr, vbe->OemProductNamePtr,
	       vbe->OemProductRevPtr, (unsigned)vbe->OemSoftwareRev >> 8,
	       vbe->OemSoftwareRev & 0xff);
#endif

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    if ((pScrn->monitor->DDC = pVesa->monitor) != NULL)
	xf86SetDDCproperties(pScrn, pVesa->monitor);

#ifdef DEBUG
    ErrorF("Searching for matching VESA mode(s):\n");
#endif

    i = 0;
    while (vbe->VideoModePtr[i] != 0xffff) {
	int id = vbe->VideoModePtr[i++];

	if ((mode = VESAGetModeInfo(pScrn, id)) == NULL)
	    continue;

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
	    VESAFreeModeInfo(mode);
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

#ifdef DEBUG
	ErrorF("Mode: %x (%dx%d)\n", id, mode->XResolution, mode->YResolution);
	ErrorF("	ModeAttributes: 0x%x\n", mode->ModeAttributes);
	ErrorF("	WinAAttributes: 0x%x\n", mode->WinAAttributes);
	ErrorF("	WinBAttributes: 0x%x\n", mode->WinBAttributes);
	ErrorF("	WinGranularity: %d\n", mode->WinGranularity);
	ErrorF("	WinSize: %d\n", mode->WinSize);
	ErrorF("	WinASegment: 0x%x\n", mode->WinASegment);
	ErrorF("	WinBSegment: 0x%x\n", mode->WinBSegment);
	ErrorF("	WinFuncPtr: 0x%x\n", mode->WinFuncPtr);
	ErrorF("	BytesPerScanline: %d\n", mode->BytesPerScanline);
	ErrorF("	XResolution: %d\n", mode->XResolution);
	ErrorF("	YResolution: %d\n", mode->YResolution);
	ErrorF("	XCharSize: %d\n", mode->XCharSize);
	ErrorF("	YCharSize: %d\n", mode->YCharSize);
	ErrorF("	NumberOfPlanes: %d\n", mode->NumberOfPlanes);
	ErrorF("	BitsPerPixel: %d\n", mode->BitsPerPixel);
	ErrorF("	NumberOfBanks: %d\n", mode->NumberOfBanks);
	ErrorF("	MemoryModel: %d\n", mode->MemoryModel);
	ErrorF("	BankSize: %d\n", mode->BankSize);
	ErrorF("	NumberOfImages: %d\n", mode->NumberOfImages);
	ErrorF("	RedMaskSize: %d\n", mode->RedMaskSize);
	ErrorF("	RedFieldPosition: %d\n", mode->RedFieldPosition);
	ErrorF("	GreenMaskSize: %d\n", mode->GreenMaskSize);
	ErrorF("	GreenFieldPosition: %d\n", mode->GreenFieldPosition);
	ErrorF("	BlueMaskSize: %d\n", mode->BlueMaskSize);
	ErrorF("	BlueFieldPosition: %d\n", mode->BlueFieldPosition);
	ErrorF("	RsvdMaskSize: %d\n", mode->RsvdMaskSize);
	ErrorF("	RsvdFieldPosition: %d\n", mode->RsvdFieldPosition);
	ErrorF("	DirectColorModeInfo: %d\n", mode->DirectColorModeInfo);
	if (pVesa->major >= 2) {
	    ErrorF("	PhysBasePtr: 0x%x\n", mode->PhysBasePtr);
	    if (pVesa->major >= 3) {
		ErrorF("	LinBytesPerScanLine: %d\n", mode->LinBytesPerScanLine);
		ErrorF("	BnkNumberOfImagePages: %d\n", mode->BnkNumberOfImagePages);
		ErrorF("	LinNumberOfImagePages: %d\n", mode->LinNumberOfImagePages);
		ErrorF("	LinRedMaskSize: %d\n", mode->LinRedMaskSize);
		ErrorF("	LinRedFieldPosition: %d\n", mode->LinRedFieldPosition);
		ErrorF("	LinGreenMaskSize: %d\n", mode->LinGreenMaskSize);
		ErrorF("	LinGreenFieldPosition: %d\n", mode->LinGreenFieldPosition);
		ErrorF("	LinBlueMaskSize: %d\n", mode->LinBlueMaskSize);
		ErrorF("	LinBlueFieldPosition: %d\n", mode->LinBlueFieldPosition);
		ErrorF("	LinRsvdMaskSize: %d\n", mode->LinRsvdMaskSize);
		ErrorF("	LinRsvdFieldPosition: %d\n", mode->LinRsvdFieldPosition);
		ErrorF("	MaxPixelClock: %d\n", mode->MaxPixelClock);
	    }
	}
#endif
    }

#ifdef DEBUG
    ErrorF("\n");
    ErrorF("Total Memory: %d 64Kb banks (%dM)\n", vbe->TotalMemory,
	   (vbe->TotalMemory * 65536) / (1024 * 1024));
#endif

    pVesa->mapSize = vbe->TotalMemory * 65536;
    if (pScrn->modePool == NULL)
	return (FALSE);
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
		data->block = xcalloc(sizeof(CRTCInfoBlock), 1);
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

    if (pScrn->modes == NULL)
	return (FALSE);

    /* options */
    xf86CollectOptions(pScrn, NULL);
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, VESAOptions);

    /* Use shadow by default */
    if (xf86ReturnOptValBool(VESAOptions, OPTION_SHADOW_FB, TRUE))
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
		reqSym = "fbScreenInit";
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
	    mod = "fb";
	    reqSym = "fbScreenInit";

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
		    return FALSE;
	    }
	    break;
	case 0x6:	/*  Direct Color */
	    mod = "fb";
	    reqSym = "fbScreenInit";

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
	if (!xf86LoadSubModule(pScrn, "shadow"))
	    return (FALSE);
	xf86LoaderReqSymLists(shadowSymbols, NULL);
    }

    if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
	VESAFreeRec(pScrn);
	return (FALSE);
    }
    xf86LoaderReqSymbols(reqSym, NULL);
#ifdef RENDER
    xf86LoaderReqSymbols("fbPictureInit", NULL);
#endif

    return (TRUE);
}

static Bool
VESAScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    VESAPtr pVesa = VESAGetRec(pScrn);
    VisualPtr visual;
    ModeInfoBlock *mode;
    int flags;

    if (pVesa->mapPhys == 0) {
	mode = ((ModeInfoData*)(pScrn->currentMode->Private))->data;
	pScrn->videoRam = pVesa->mapSize;
	pVesa->mapPhys = mode->PhysBasePtr;
	pVesa->mapOff = 0;
    }

    if ((void*)pVesa->mapPhys == NULL) {
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
    pVesa->savedPal = VESASetGetPaletteData(pScrn, FALSE, 0, 256,
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
		if (!fbScreenInit(pScreen,
			          pVesa->shadowPtr,
				  pScrn->virtualX, pScrn->virtualY,
				  pScrn->xDpi, pScrn->yDpi,
				  pScrn->displayWidth, pScrn->bitsPerPixel))
		    return (FALSE);
	    }
	    else {
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
		    break;
		default:
		    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			       "Unsupported bpp: %d", pScrn->bitsPerPixel);
		    return FALSE;
	    }
	    break;
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
		    break;
		default:
		    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			       "Unsupported bpp: %d", pScrn->bitsPerPixel);
		    return (FALSE);
	    }
	    break;
    }

#ifdef RENDER
    fbPictureInit(pScreen, 0, 0);
#endif

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

    pScreen->CloseScreen = VESACloseScreen;
    pScreen->SaveScreen = VESASaveScreen;

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

    VESASaveRestore(xf86Screens[scrnIndex], MODE_RESTORE);
    VESASetGetPaletteData(pScrn, TRUE, 0, 256,
			  pVesa->savedPal, FALSE, TRUE);

    VESAUnmapVidMem(pScrn);
    if (pVesa->shadowPtr) {
	xfree(pVesa->shadowPtr);
	pVesa->shadowPtr = NULL;
    }
    if (pVesa->pDGAMode)
	xfree(pVesa->pDGAMode);
    pScrn->vtSema = FALSE;

    return (TRUE);
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

    if (VESASetVBEMode(pScrn, mode, data->block) == FALSE) {
	if ((data->block || (data->mode & (1 << 11))) &&
	    VESASetVBEMode(pScrn, (mode & ~(1 << 11)), NULL) == TRUE) {
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
	VESASetLogicalScanline(pScrn, pScrn->virtualX);

    if (pScrn->bitsPerPixel >= 8 && pVesa->vbeInfo->Capabilities[0] & 0x01)
	VESASetGetDACPaletteFormat(pScrn, 8);

    pScrn->vtSema = TRUE;

    return (TRUE);
}

static void
VESAAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr pScrn;

    pScrn = xf86Screens[scrnIndex];
    VESASetDisplayStart(pScrn, x, y, TRUE);
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

    pVesa->base = xf86MapVidMem(pScrn->scrnIndex, 0,
				pScrn->memPhysBase, pVesa->mapSize);
    if (pVesa->base) {
	if (pVesa->mapPhys != 0xa0000)
	    pVesa->VGAbase = xf86MapVidMem(pScrn->scrnIndex, 0,
				0xa0000, 0x10000);
	else
	    pVesa->VGAbase = pVesa->base;
    }
#ifdef DEBUG
    ErrorF("virtual address = %p  -  physical address = %p  -  size = %d\n",
	    pVesa->base, pScrn->memPhysBase, pVesa->mapSize);
#endif

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
		 CARD32 *size)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VESAPtr pVesa = VESAGetRec(pScrn);
    ModeInfoBlock *data = ((ModeInfoData*)(pScrn->currentMode->Private))->data;
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
		 CARD32 *size)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VESAPtr pVesa = VESAGetRec(pScrn);

    *size = pVesa->maxBytesPerScanline;
    return ((CARD8 *)pVesa->base + row * pVesa->maxBytesPerScanline + offset);
}

static void *
VESAWindowWindowed(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		   CARD32 *size)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    VESAPtr pVesa = VESAGetRec(pScrn);
    ModeInfoBlock *data = ((ModeInfoData*)(pScrn->currentMode->Private))->data;
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
	    VESASetGetPaletteData(pScrn, TRUE, base, idx - base,
				  pVesa->pal + base, FALSE, TRUE);
	    idx = base = j;
	}
    }

    if (idx - 1 == indices[i - 1])
	VESASetGetPaletteData(pScrn, TRUE, base, idx - base,
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
    CARD8 tmp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

    index |= 0x20;
    outb(VGA_ATTR_INDEX, index);
    outb(VGA_ATTR_DATA_W, value);
}

static int
ReadAttr(int index)
{
    CARD8 tmp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

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

VBEInfoBlock *
VESAGetVBEInfo(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa;
    VBEInfoBlock *block = NULL;
    int i, pStr, pModes;
    char *str;
    CARD16 major, minor, *modes;

    pVesa = VESAGetRec(pScrn);
    bzero(pVesa->block, sizeof(VBEInfoBlock));

    /*
    Input:
	AH    := 4Fh	Super VGA support
	AL    := 00h	Return Super VGA information
	ES:DI := Pointer to buffer

    Output:
	AX    := status
	(All other registers are preserved)
     */

    pVesa->block[0] = 'V';
    pVesa->block[1] = 'B';
    pVesa->block[2] = 'E';
    pVesa->block[3] = '2';

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f00;
    pVesa->pInt->es = SEG_ADDR(pVesa->page);
    pVesa->pInt->di = SEG_OFF(pVesa->page);
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (NULL);

    block = xcalloc(sizeof(VBEInfoBlock), 1);
    block->VESASignature[0] = pVesa->block[0];
    block->VESASignature[1] = pVesa->block[1];
    block->VESASignature[2] = pVesa->block[2];
    block->VESASignature[3] = pVesa->block[3];

    block->VESAVersion = *(CARD16*)(pVesa->block + 4);
    major = (unsigned)block->VESAVersion >> 8;
    minor = block->VESAVersion & 0xff;

    pStr = *(CARD32*)(pVesa->block + 6);
    str = xf86int10Addr(pVesa->pInt, FARP(pStr));
    block->OEMStringPtr = strdup(str);

    block->Capabilities[0] = pVesa->block[10];
    block->Capabilities[1] = pVesa->block[11];
    block->Capabilities[2] = pVesa->block[12];
    block->Capabilities[3] = pVesa->block[13];

    pModes = *(CARD32*)(pVesa->block + 14);
    modes = xf86int10Addr(pVesa->pInt, FARP(pModes));
    i = 0;
    while (modes[i] != 0xffff)
	i++;
    block->VideoModePtr = xalloc(sizeof(CARD16) * i + 1);
    memcpy(block->VideoModePtr, modes, sizeof(CARD16) * i);
    block->VideoModePtr[i] = 0xffff;

    block->TotalMemory = *(CARD16*)(pVesa->block + 18);

    if (major < 2)
	memcpy(&block->OemSoftwareRev, pVesa->block + 20, 236);
    else {
	block->OemSoftwareRev = *(CARD16*)(pVesa->block + 20);
	pStr = *(CARD32*)(pVesa->block + 22);
	str = xf86int10Addr(pVesa->pInt, FARP(pStr));
	block->OemVendorNamePtr = strdup(str);
	pStr = *(CARD32*)(pVesa->block + 26);
	str = xf86int10Addr(pVesa->pInt, FARP(pStr));
	block->OemProductNamePtr = strdup(str);
	pStr = *(CARD32*)(pVesa->block + 30);
	str = xf86int10Addr(pVesa->pInt, FARP(pStr));
	block->OemProductRevPtr = strdup(str);
	memcpy(&block->Reserved, pVesa->block + 34, 222);
	memcpy(&block->OemData, pVesa->block + 256, 256);
    }

    return (block);
}

void
VESAFreeVBEInfo(VBEInfoBlock *block)
{
    xfree(block->OEMStringPtr);
    xfree(block->VideoModePtr);
    if (((unsigned)block->VESAVersion >> 8) >= 2) {
	xfree(block->OemVendorNamePtr);
	xfree(block->OemProductNamePtr);
	xfree(block->OemProductRevPtr);
    }
    xfree(block);
}

Bool
VESASetVBEMode(ScrnInfoPtr pScrn, int mode, CRTCInfoBlock *block)
{
    VESAPtr pVesa;

    pVesa = VESAGetRec(pScrn);
    /*
    Input:
	AH    := 4Fh	Super VGA support
	AL    := 02h	Set Super VGA video mode
	BX    := Video mode
	    D0-D8  := Mode number
	    D9-D10 := Reserved (must be 0)
	    D11    := 0 Use current default refresh rate
		   := 1 Use user specified CRTC values for refresh rate
	    D12-13	Reserved for VBE/AF (must be 0)
	    D14    := 0 Use windowed frame buffer model
		   := 1 Use linear/flat frame buffer model
	    D15    := 0 Clear video memory
		   := 1 Don't clear video memory
	ES:DI := Pointer to CRTCInfoBlock structure

    Output: AX = Status
	(All other registers are preserved)
    */
    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f02;
    pVesa->pInt->bx = mode;
    if (block) {
	pVesa->pInt->bx |= 1 << 11;
	memcpy(pVesa->block, block, sizeof(CRTCInfoBlock));
	pVesa->pInt->es = SEG_ADDR(pVesa->page);
	pVesa->pInt->di = SEG_OFF(pVesa->page);
    }

    xf86ExecX86int10(pVesa->pInt);

    return (pVesa->pInt->ax == 0x4f);
}

Bool
VESAGetVBEMode(ScrnInfoPtr pScrn, int *mode)
{
    VESAPtr pVesa;

    pVesa = VESAGetRec(pScrn);
    /*
    Input:
	AH := 4Fh	Super VGA support
	AL := 03h	Return current video mode

    Output:
	AX := Status
	BX := Current video mode
	(All other registers are preserved)
    */
    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f03;

    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax == 0x4f) {
	*mode = pVesa->pInt->bx;

	return (TRUE);
    }

    return (FALSE);
}

ModeInfoBlock *
VESAGetModeInfo(ScrnInfoPtr pScrn, int mode)
{
    VESAPtr pVesa;
    ModeInfoBlock *block = NULL;

    pVesa = VESAGetRec(pScrn);
    bzero(pVesa->block, sizeof(ModeInfoBlock));

    /*
    Input:
	AH    := 4Fh	Super VGA support
	AL    := 01h	Return Super VGA mode information
	CX    := 	Super VGA video mode
			(mode number must be one of those returned by Function 0)
	ES:DI := Pointer to buffer

    Output:
	AX    := status
	(All other registers are preserved)
     */
    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f01;
    pVesa->pInt->cx = mode;
    pVesa->pInt->es = SEG_ADDR(pVesa->page);
    pVesa->pInt->di = SEG_OFF(pVesa->page);
    xf86ExecX86int10(pVesa->pInt);
    if (pVesa->pInt->ax != 0x4f)
	return (NULL);

    block = xcalloc(sizeof(ModeInfoBlock), 1);

    block->ModeAttributes = *(CARD16*)pVesa->block;
    block->WinAAttributes = pVesa->block[2];
    block->WinBAttributes = pVesa->block[3];
    block->WinGranularity = *(CARD16*)(pVesa->block + 4);
    block->WinSize = *(CARD16*)(pVesa->block + 6);
    block->WinASegment = *(CARD16*)(pVesa->block + 8);
    block->WinBSegment = *(CARD16*)(pVesa->block + 10);
    block->WinFuncPtr = *(CARD32*)(pVesa->block + 12);
    block->BytesPerScanline = *(CARD16*)(pVesa->block + 16);

    /* mandatory information for VBE 1.2 and above */
    block->XResolution = *(CARD16*)(pVesa->block + 18);
    block->YResolution = *(CARD16*)(pVesa->block + 20);
    block->XCharSize = pVesa->block[22];
    block->YCharSize = pVesa->block[23];
    block->NumberOfPlanes = pVesa->block[24];
    block->BitsPerPixel = pVesa->block[25];
    block->NumberOfBanks = pVesa->block[26];
    block->MemoryModel = pVesa->block[27];
    block->BankSize = pVesa->block[28];
    block->NumberOfImages = pVesa->block[29];
    block->Reserved = pVesa->block[30];

    /* Direct color fields (required for direct/6 and YUV/7 memory models) */
    block->RedMaskSize = pVesa->block[31];
    block->RedFieldPosition = pVesa->block[32];
    block->GreenMaskSize = pVesa->block[33];
    block->GreenFieldPosition = pVesa->block[34];
    block->BlueMaskSize = pVesa->block[35];
    block->BlueFieldPosition = pVesa->block[36];
    block->RsvdMaskSize = pVesa->block[37];
    block->RsvdFieldPosition = pVesa->block[38];
    block->DirectColorModeInfo = pVesa->block[39];

    /* Mandatory information for VBE 2.0 and above */
    if (pVesa->major >= 2) {
	block->PhysBasePtr = *(CARD32*)(pVesa->block + 40);
	block->Reserved32 = *(CARD32*)(pVesa->block + 44);
	block->Reserved16 = *(CARD16*)(pVesa->block + 48);

	/* Mandatory information for VBE 3.0 and above */
	if (pVesa->major >= 3) {
	    block->LinBytesPerScanLine = *(CARD16*)(pVesa->block + 50);
	    block->BnkNumberOfImagePages = pVesa->block[52];
	    block->LinNumberOfImagePages = pVesa->block[53];
	    block->LinRedMaskSize = pVesa->block[54];
	    block->LinRedFieldPosition = pVesa->block[55];
	    block->LinGreenMaskSize = pVesa->block[56];
	    block->LinGreenFieldPosition = pVesa->block[57];
	    block->LinBlueMaskSize = pVesa->block[58];
	    block->LinBlueFieldPosition = pVesa->block[59];
	    block->LinRsvdMaskSize = pVesa->block[60];
	    block->LinRsvdFieldPosition = pVesa->block[61];
	    block->MaxPixelClock = *(CARD32*)(pVesa->block + 62);
	    memcpy(&block->Reserved2, pVesa->block + 66, 188);
	}
	else
	    memcpy(&block->LinBytesPerScanLine, pVesa->block + 50, 206);
    }
    else
	memcpy(&block->PhysBasePtr, pVesa->block + 40, 216);

    return (block);
}

void
VESAFreeModeInfo(ModeInfoBlock *block)
{
    xfree(block);
}

Bool
VESASaveRestore(ScrnInfoPtr pScrn, int function)
{
    VESAPtr pVesa;

    if (MODE_QUERY < 0 || function > MODE_RESTORE)
	return (FALSE);

    pVesa = VESAGetRec(pScrn);

    /*
    Input:
	AH    := 4Fh	Super VGA support
	AL    := 04h	Save/restore Super VGA video state
	DL    := 00h	Return save/restore state buffer size
	CX    := Requested states
		D0 = Save/restore video hardware state
		D1 = Save/restore video BIOS data state
		D2 = Save/restore video DAC state
		D3 = Save/restore Super VGA state

    Output:
	AX = Status
	BX = Number of 64-byte blocks to hold the state buffer
	(All other registers are preserved)


    Input:
	AH    := 4Fh	Super VGA support
	AL    := 04h	Save/restore Super VGA video state
	DL    := 01h	Save Super VGA video state
	CX    := Requested states (see above)
	ES:BX := Pointer to buffer

    Output:
	AX    := Status
	(All other registers are preserved)


    Input:
	AH    := 4Fh	Super VGA support
	AL    := 04h	Save/restore Super VGA video state
	DL    := 02h	Restore Super VGA video state
	CX    := Requested states (see above)
	ES:BX := Pointer to buffer

    Output:
	AX     := Status
	(All other registers are preserved)
     */

    /* Query amount of memory to save state */
    if (function == MODE_QUERY ||
	(function == MODE_SAVE && pVesa->state == NULL)) {
	int npages;

	/* Make sure we save at least this information in case of failure */
	(void)VESAGetVBEMode(pScrn, &pVesa->stateMode);
	SaveFonts(pScrn);

	if (pVesa->major > 1) {
	    pVesa->pInt->num = 0x10;
	    pVesa->pInt->ax = 0x4f04;
	    pVesa->pInt->dx = 0;
	    pVesa->pInt->cx = 0x000f;
	    xf86ExecX86int10(pVesa->pInt);
	    if (pVesa->pInt->ax != 0x4f)
		return (FALSE);

	    npages = (pVesa->pInt->bx * 64) / 4096 + 1;
	    if ((pVesa->state = xf86Int10AllocPages(pVesa->pInt, npages,
						    &pVesa->statePage)) == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Cannot allocate memory to save SVGA state.\n");
		return (FALSE);
	    }
	}
    }

    /* Save/Restore Super VGA state */
    if (function != MODE_QUERY) {
	int ax_reg = 0;

	if (pVesa->major > 1) {
	    pVesa->pInt->num = 0x10;
	    pVesa->pInt->ax = 0x4f04;
	    pVesa->pInt->dx = function;
	    pVesa->pInt->cx = 0x000f;

	    if (function == MODE_RESTORE)
		memcpy(pVesa->state, pVesa->pstate, pVesa->stateSize);

	    pVesa->pInt->es = SEG_ADDR(pVesa->statePage);
	    pVesa->pInt->bx = SEG_OFF(pVesa->statePage);
	    xf86ExecX86int10(pVesa->pInt);
	    ax_reg = pVesa->pInt->ax;
	}

	if (function == MODE_RESTORE) {
	    VESASetVBEMode(pScrn, pVesa->stateMode, NULL);
	    RestoreFonts(pScrn);
	}

	if (pVesa->major > 1) {
	    if (ax_reg != 0x4f)
		return (FALSE);

	    if (function == MODE_SAVE && pVesa->pstate == NULL) {
		/* don't rely on the memory not being touched */
		pVesa->stateSize = pVesa->pInt->bx * 64;
		pVesa->pstate = xalloc(pVesa->stateSize);
		memcpy(pVesa->pstate, pVesa->state, pVesa->stateSize);
	    }
	}
    }

    return (TRUE);
}

int
VESABankSwitch(ScreenPtr pScreen, unsigned int iBank)
{
    VESAPtr pVesa;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    pVesa = VESAGetRec(pScrn);
    if (pVesa->curBank == iBank)
	return (0);
    pVesa->curBank = iBank;

    /*
    Input:
	AH    := 4Fh	Super VGA support
	AL    := 05h

    Output:
     */
    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f05;
    pVesa->pInt->bx = 0;
    pVesa->pInt->dx = iBank;
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (1);

    if (pVesa->bankSwitchWindowB) {
	pVesa->pInt->num = 0x10;
	pVesa->pInt->ax = 0x4f05;
	pVesa->pInt->bx = 1;
	pVesa->pInt->dx = iBank;
	xf86ExecX86int10(pVesa->pInt);

	if (pVesa->pInt->ax != 0x4f)
	    return (1);
    }

    return (0);
}

Bool
VESASetGetLogicalScanlineLength(ScrnInfoPtr pScrn, int command, int width,
				int *pixels, int *bytes, int *max)
{
    VESAPtr pVesa;

    if (command < SCANWID_SET || command > SCANWID_GET_MAX)
	return (FALSE);

    pVesa = VESAGetRec(pScrn);
    /*
    Input:
	AX := 4F06h VBE Set/Get Logical Scan Line Length
	BL := 00h Set Scan Line Length in Pixels
	   := 01h Get Scan Line Length
	   := 02h Set Scan Line Length in Bytes
	   := 03h Get Maximum Scan Line Length
	CX := If BL=00h Desired Width in Pixels
	      If BL=02h Desired Width in Bytes
	      (Ignored for Get Functions)

    Output:
	AX := VBE Return Status
	BX := Bytes Per Scan Line
	CX := Actual Pixels Per Scan Line
	      (truncated to nearest complete pixel)
	DX := Maximum Number of Scan Lines
     */

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f06;
    pVesa->pInt->bx = command;
    if (command == SCANWID_SET || command == SCANWID_SET_BYTES)
	pVesa->pInt->cx = width;
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (FALSE);

    if (command == SCANWID_GET || command == SCANWID_GET_MAX) {
	if (pixels)
	    *pixels = pVesa->pInt->cx;
	if (bytes)
	    *bytes = pVesa->pInt->bx;
	if (max)
	    *max = pVesa->pInt->dx;
    }

    return (TRUE);
}

Bool
VESASetDisplayStart(ScrnInfoPtr pScrn, int x, int y, Bool wait_retrace)
{
    VESAPtr pVesa;

    pVesa = VESAGetRec(pScrn);

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f07;
    pVesa->pInt->bx = wait_retrace ? 0x80 : 0x00;
    pVesa->pInt->cx = x;
    pVesa->pInt->dx = y;
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (FALSE);

    return (TRUE);
}

Bool
VESAGetDisplayStart(ScrnInfoPtr pScrn, int *x, int *y)
{
    VESAPtr pVesa;

    pVesa = VESAGetRec(pScrn);

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f07;
    pVesa->pInt->bx = 0x01;
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (FALSE);

    *x = pVesa->pInt->cx;
    *y = pVesa->pInt->dx;

    return (TRUE);
}

int
VESASetGetDACPaletteFormat(ScrnInfoPtr pScrn, int bits)
{
    VESAPtr pVesa = VESAGetRec(pScrn);

    /*
    Input:
	AX := 4F08h VBE Set/Get Palette Format
	BL := 00h Set DAC Palette Format
	   := 01h Get DAC Palette Format
	BH := Desired bits of color per primary
	      (Set DAC Palette Format only)

    Output:
	AX := VBE Return Status
	BH := Current number of bits of color per primary
     */

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f08;
    if (!bits)
	pVesa->pInt->bx = 0x01;
    else 
	pVesa->pInt->bx = (bits & 0x00ff) << 8;
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (0);

    return (bits != 0 ? bits : (pVesa->pInt->bx >> 8) & 0x00ff);
}

CARD32 *
VESASetGetPaletteData(ScrnInfoPtr pScrn, Bool set, int first, int num,
		      CARD32 *data, Bool secondary, Bool wait_retrace)
{
    VESAPtr pVesa = VESAGetRec(pScrn);

    /*
    Input:
    (16-bit)
	AX    := 4F09h VBE Load/Unload Palette Data
	BL    := 00h Set Palette Data
	      := 01h Get Palette Data
	      := 02h Set Secondary Palette Data
	      := 03h Get Secondary Palette Data
	      := 80h Set Palette Data during Vertical Retrace
	CX    := Number of palette registers to update (to a maximum of 256)
	DX    := First of the palette registers to update (start)
	ES:DI := Table of palette values (see below for format)

    Output:
	AX    := VBE Return Status


    Input:
    (32-bit)
	BL     := 00h Set Palette Data
	       := 80h Set Palette Data during Vertical Retrace
	CX     := Number of palette registers to update (to a maximum of 256)
	DX     := First of the palette registers to update (start)
	ES:EDI := Table of palette values (see below for format)
	DS     := Selector for memory mapped registers
     */

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f09;
    if (!secondary)
	pVesa->pInt->bx = set && wait_retrace ? 0x80 : set ? 0 : 1;
    else
	pVesa->pInt->bx = set ? 2 : 3;
    pVesa->pInt->cx = num;
    pVesa->pInt->dx = first;
    pVesa->pInt->es = SEG_ADDR(pVesa->page);
    pVesa->pInt->di = SEG_OFF(pVesa->page);
    if (set)
	memcpy(pVesa->block, data, num * sizeof(CARD32));
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (NULL);

    if (set)
	return (data);

    data = xalloc(num * sizeof(CARD32));
    memcpy(data, pVesa->block, num * sizeof(CARD32));

    return (data);
}

VESApmi *
VESAGetVBEpmi(ScrnInfoPtr pScrn)
{
    VESAPtr pVesa;
    VESApmi *pmi;

    pVesa = VESAGetRec(pScrn);
    /*
    Input:
	AH    := 4Fh	Super VGA support
	AL    := 0Ah	Protected Mode Interface
	BL    := 00h	Return Protected Mode Table

    Output:
	AX    := Status
	ES    := Real Mode Segment of Table
	DI    := Offset of Table
	CX    := Lenght of Table including protected mode code in bytes (for copying purposes)
	(All other registers are preserved)
     */

    pVesa->pInt->num = 0x10;
    pVesa->pInt->ax = 0x4f0a;
    pVesa->pInt->bx = 0;
    pVesa->pInt->di = 0;
    xf86ExecX86int10(pVesa->pInt);

    if (pVesa->pInt->ax != 0x4f)
	return (NULL);

    pmi = xalloc(sizeof(VESApmi));
    pmi->seg_tbl = pVesa->pInt->es;
    pmi->tbl_off = pVesa->pInt->di;
    pmi->tbl_len = pVesa->pInt->cx;

    return (pmi);
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
    *ApertureBase = (unsigned char *)(pVesa->mapPhys);
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
