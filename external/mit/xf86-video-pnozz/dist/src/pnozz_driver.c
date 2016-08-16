/*
 * SBus Weitek P9100 driver
 *
 * Copyright (C) 2005, 2006 Michael Lorenz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * MICHAEL LORENZ BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $NetBSD: pnozz_driver.c,v 1.6 2016/08/16 01:27:47 mrg Exp $ */

/*
 * this driver has been tested on SPARCbook 3GX and 3TX, it supports full 
 * acceleration in 8, 16 and 24 bit colour
 */

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dev/sun/fbio.h>
#include <dev/wscons/wsconsio.h>

#include "xf86.h"
#include "xf86_OSproc.h"

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
#include "xf86Resources.h"
#endif
#include "xf86sbusBus.h"

#include "mipointer.h"
#include "micmap.h"

#define DEBUG 0

#include "fb.h"
#include "xf86cmap.h"
#include "pnozz.h"

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 6
#define xf86LoaderReqSymLists(...) do {} while (0)
#define LoaderRefSymLists(...) do {} while (0)
#endif

static const OptionInfoRec * PnozzAvailableOptions(int chipid, int busid);
static void	PnozzIdentify(int flags);
static Bool	PnozzProbe(DriverPtr drv, int flags);
static Bool	PnozzPreInit(ScrnInfoPtr pScrn, int flags);
static Bool	PnozzScreenInit(int Index, ScreenPtr pScreen, int argc,
			      char **argv);
static Bool	PnozzEnterVT(int scrnIndex, int flags);
static void	PnozzLeaveVT(int scrnIndex, int flags);
static Bool	PnozzCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool	PnozzSaveScreen(ScreenPtr pScreen, int mode);

/* Required if the driver supports mode switching */
static Bool	PnozzSwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
/* Required if the driver supports moving the viewport */
static void	PnozzAdjustFrame(int scrnIndex, int x, int y, int flags);

/* Optional functions */
static void	PnozzFreeScreen(int scrnIndex, int flags);
static ModeStatus PnozzValidMode(int scrnIndex, DisplayModePtr mode,
			       Bool verbose, int flags);

void PnozzSync(ScrnInfoPtr);
void PnozzSave(PnozzPtr);
void PnozzRestore(PnozzPtr);
int PnozzSetDepth(PnozzPtr, int);	/* return true or false */
void DumpSCR(unsigned int);

static void PnozzLoadPalette(ScrnInfoPtr, int, int *, LOCO *, VisualPtr);

#define VERSION 4000
#define PNOZZ_NAME "p9100"
#define PNOZZ_DRIVER_NAME "pnozz"
#define PNOZZ_MAJOR_VERSION 1
#define PNOZZ_MINOR_VERSION 0
#define PNOZZ_PATCHLEVEL 0

/* 
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

DriverRec PNOZZ = {
    VERSION,
    PNOZZ_DRIVER_NAME,
    PnozzIdentify,
    PnozzProbe,
    PnozzAvailableOptions,
    NULL,
    0
};

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_NOACCEL
} PnozzOpts;

static const OptionInfoRec PnozzOptions[] = {
    { OPTION_SW_CURSOR,		"SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_HW_CURSOR,		"HWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_NOACCEL,		"NoAccel",	OPTV_BOOLEAN,	{0}, FALSE },
    { -1,			NULL,		OPTV_NONE,	{0}, FALSE }
};

static const char *ramdacSymbols[] = {
    "xf86CreateCursorInfoRec",
    "xf86DestroyCursorInfoRec",
    "xf86InitCursor",
    NULL
};

static const char *fbSymbols[] = {
    "fbScreenInit",
    "fbPictureInit",
    NULL
};

static const char *xaaSymbols[] =
{
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAInit",
    NULL
};
#ifdef XFree86LOADER

static MODULESETUPPROTO(PnozzSetup);

static XF86ModuleVersionInfo PnozzVersRec =
{
	"pnozz",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	PNOZZ_MAJOR_VERSION, PNOZZ_MINOR_VERSION, PNOZZ_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0,0,0,0}
};

XF86ModuleData pnozzModuleData = { &PnozzVersRec, PnozzSetup, NULL };

pointer
PnozzSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	xf86AddDriver(&PNOZZ, module, 0);
	
	LoaderRefSymLists(xaaSymbols, ramdacSymbols, fbSymbols, NULL);
	/*
	 * Modules that this driver always requires can be loaded here
	 * by calling LoadSubModule().
	 */

	/*
	 * The return value must be non-NULL on success even though there
	 * is no TearDownProc.
	 */
	return (pointer)TRUE;
    } else {
	if (errmaj) *errmaj = LDR_ONCEONLY;
	return NULL;
    }
}

#endif /* XFree86LOADER */

static volatile unsigned int scratch32;

void pnozz_write_4(PnozzPtr p, int offset, unsigned int value)
{
	if ((offset & 0xffffff80) != p->offset_mask) {
		p->offset_mask = offset & 0xffffff80;
		scratch32 = *(volatile unsigned int *)(p->fb + offset);
	}
	*((volatile unsigned int *)(p->fbc + offset)) = value;
}

unsigned int pnozz_read_4(PnozzPtr p, int offset)
{
	if ((offset & 0xffffff80) != p->offset_mask) {
		p->offset_mask = offset & 0xffffff80;
		scratch32 = *(volatile unsigned int *)(p->fb + offset);
	}
	return *(volatile unsigned int *)(p->fbc + offset);
}

void pnozz_write_dac(PnozzPtr p, int offset, unsigned char value)
{
	CARD32 val = ((CARD32)value) << 16;

	scratch32 = pnozz_read_4(p, PWRUP_CNFG);
	if ((offset != DAC_INDX_DATA) && (offset != DAC_CMAP_DATA)) {
		do {
			pnozz_write_4(p, offset, val);
		} while (pnozz_read_4(p, offset) != val);
	} else {
		pnozz_write_4(p, offset, val);
	}
}

unsigned char pnozz_read_dac(PnozzPtr p, int offset)
{
	scratch32 = pnozz_read_4(p, PWRUP_CNFG);
	return ((pnozz_read_4(p, offset) >> 16) & 0xff);
}

void pnozz_write_dac_ctl_reg(PnozzPtr p, int offset, unsigned char val)
{

	pnozz_write_dac(p, DAC_INDX_HI, (offset & 0xff00) >> 8);
	pnozz_write_dac(p, DAC_INDX_LO, (offset & 0xff));
	pnozz_write_dac(p, DAC_INDX_DATA, val);
}

void pnozz_write_dac_ctl_reg_2(PnozzPtr p, int offset, unsigned short val)
{

	pnozz_write_dac(p, DAC_INDX_HI, (offset & 0xff00) >> 8);
	pnozz_write_dac(p, DAC_INDX_LO, (offset & 0xff));
	pnozz_write_dac(p, DAC_INDX_CTL, DAC_INDX_AUTOINCR);
	pnozz_write_dac(p, DAC_INDX_DATA, val & 0xff);
	pnozz_write_dac(p, DAC_INDX_DATA, (val & 0xff00) >> 8);
}

unsigned char pnozz_read_dac_ctl_reg(PnozzPtr p, int offset)
{
	pnozz_write_dac(p, DAC_INDX_HI, (offset & 0xff00) >> 8);
	pnozz_write_dac(p, DAC_INDX_LO, (offset & 0xff));
	return pnozz_read_dac(p, DAC_INDX_DATA);
}

void pnozz_write_dac_cmap_reg(PnozzPtr p, int offset, unsigned int val)
{
	pnozz_write_dac(p, DAC_CMAP_WRIDX,(offset & 0xff));
	pnozz_write_dac(p, DAC_CMAP_DATA,(val & 0xff));
	pnozz_write_dac(p, DAC_CMAP_DATA,(val & 0xff00) >> 8);
	pnozz_write_dac(p, DAC_CMAP_DATA,(val & 0xff0000) >> 16);
}

static void
PnozzLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices, LOCO *colors,
    VisualPtr pVisual) 
{
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    int i, index;
    
    PnozzSync(pScrn);
    pnozz_write_dac(pPnozz, DAC_INDX_CTL, DAC_INDX_AUTOINCR);

    for (i = 0; i < numColors; i++)
    {
    	index = indices[i];
	if (index >= 0) {
    	    pnozz_write_dac(pPnozz, DAC_CMAP_WRIDX, index);
	    pnozz_write_dac(pPnozz, DAC_CMAP_DATA, colors[index].red);
	    pnozz_write_dac(pPnozz, DAC_CMAP_DATA, colors[index].green);
	    pnozz_write_dac(pPnozz, DAC_CMAP_DATA, colors[index].blue);
	}
    }
    PnozzSync(pScrn);
}

static Bool
PnozzGetRec(ScrnInfoPtr pScrn)
{
    /*
     * Allocate an PnozzRec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate != NULL)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(PnozzRec), 1);
    return TRUE;
}

static void
PnozzFreeRec(ScrnInfoPtr pScrn)
{
    PnozzPtr pPnozz;

    if (pScrn->driverPrivate == NULL)
	return;

    pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;

    return;
}

static const OptionInfoRec *
PnozzAvailableOptions(int chipid, int busid)
{
    return PnozzOptions;
}

/* Mandatory */
static void
PnozzIdentify(int flags)
{
    xf86Msg(X_INFO, "%s: driver for Weitek P9100 found in Tadpole SPARCbook 3GX and others\n", PNOZZ_NAME);
}


/* Mandatory */
static Bool
PnozzProbe(DriverPtr drv, int flags)
{
    int i;
    GDevPtr *devSections;
    int *usedChips;
    int numDevSections;
    int numUsed;
    Bool foundScreen = FALSE;
    EntityInfoPtr pEnt;

    /*
     * The aim here is to find all cards that this driver can handle,
     * and for the ones not already claimed by another driver, claim the
     * slot, and allocate a ScrnInfoRec.
     *
     * This should be a minimal probe, and it should under no circumstances
     * change the state of the hardware.  Because a device is found, don't
     * assume that it will be used.  Don't do any initialisations other than
     * the required ScrnInfoRec initialisations.  Don't allocate any new
     * data structures.
     */

    /*
     * Next we check, if there has been a chipset override in the config file.
     * For this we must find out if there is an active device section which
     * is relevant, i.e., which has no driver specified or has THIS driver
     * specified.
     */

    if ((numDevSections = xf86MatchDevice(PNOZZ_DRIVER_NAME,
					  &devSections)) <= 0) {
	/*
	 * There's no matching device section in the config file, so quit
	 * now.
	 */
	return FALSE;
    }

    /*
     * We need to probe the hardware first.  We then need to see how this
     * fits in with what is given in the config file, and allow the config
     * file info to override any contradictions.
     */

    numUsed = xf86MatchSbusInstances(PNOZZ_NAME, SBUS_DEVICE_P9100,
		   devSections, numDevSections,
		   drv, &usedChips);
				 	
    xfree(devSections);
    if (numUsed <= 0)
	return FALSE;

    if (flags & PROBE_DETECT)
	foundScreen = TRUE;
    else
	for (i = 0; i < numUsed; i++) {
	    pEnt = xf86GetEntityInfo(usedChips[i]);
	
	    /*
	     * Check that nothing else has claimed the slots.
	     */
	    if(pEnt->active) {
		ScrnInfoPtr pScrn;
			    
		/* Allocate a ScrnInfoRec and claim the slot */
		pScrn = xf86AllocateScreen(drv, 0);
	
		/* Fill in what we can of the ScrnInfoRec */
		pScrn->driverVersion = VERSION;
		pScrn->driverName	 = PNOZZ_DRIVER_NAME;
		pScrn->name		 = PNOZZ_NAME;
		pScrn->Probe	 	 = PnozzProbe;
		pScrn->PreInit	 	 = PnozzPreInit;
		pScrn->ScreenInit	 = PnozzScreenInit;
  		pScrn->SwitchMode	 = PnozzSwitchMode;
  		pScrn->AdjustFrame	 = PnozzAdjustFrame;
		pScrn->EnterVT		 = PnozzEnterVT;
		pScrn->LeaveVT		 = PnozzLeaveVT;
		pScrn->FreeScreen	 = PnozzFreeScreen;
		pScrn->ValidMode	 = PnozzValidMode;
		xf86AddEntityToScreen(pScrn, pEnt->index);
		foundScreen = TRUE;
	    }
	    xfree(pEnt);
    	}
    xfree(usedChips);
    return foundScreen;
}

/* Mandatory */
static Bool
PnozzPreInit(ScrnInfoPtr pScrn, int flags)
{
    PnozzPtr pPnozz;
    sbusDevicePtr psdp;
    MessageType from;
    rgb defaultWeight = {0, 0, 0};
    int i;

    if (flags & PROBE_DETECT) return FALSE;

    /*
     * Note: This function is only called once at server startup, and
     * not at the start of each server generation.  This means that
     * only things that are persistent across server generations can
     * be initialised here.  xf86Screens[] is (pScrn is a pointer to one
     * of these).  Privates allocated using xf86AllocateScrnInfoPrivateIndex()  
     * are too, and should be used for data that must persist across
     * server generations.
     *
     * Per-generation data should be allocated with
     * AllocateScreenPrivateIndex() from the ScreenInit() function.
     */

    /* Allocate the PnozzRec driverPrivate */
    if (!PnozzGetRec(pScrn)) {
	return FALSE;
    }
    pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    
    /* always mismatch on first access */
    pPnozz->offset_mask = 0xffffffff;
    
    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    /* This driver doesn't expect more than one entity per screen */
    if (pScrn->numEntities > 1)
	return FALSE;
    /* This is the general case */
    for (i = 0; i < pScrn->numEntities; i++) {
	EntityInfoPtr pEnt = xf86GetEntityInfo(pScrn->entityList[i]);

	/* PNOZZ is purely SBUS */
	if (pEnt->location.type == BUS_SBUS) {
	    psdp = xf86GetSbusInfoForEntity(pEnt->index);
	    pPnozz->psdp = psdp;
	} else
	    return FALSE;
    }

    /*********************
    deal with depth
    *********************/
    if (!xf86SetDepthBpp(pScrn, 0, 0, 0, Support32bppFb)) {
	return FALSE;
    } else {
	/* Check that the returned depth is one we support */
#ifdef DEBUG
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, 
	    "Depth requested: %d\n", pScrn->depth);
#endif
	switch (pScrn->depth) {
	case 8:
	case 16:
	case 24:
	    /* OK */
	    break;
	default:
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Given depth (%d) is not supported by this driver\n",
		       pScrn->depth);
	    return FALSE;
	}
    }
    xf86PrintDepthBpp(pScrn);

    /* We use a programmable clock */
    pScrn->progClock = TRUE;

    /* Set the bits per RGB for 8bpp mode */
    if (pScrn->depth == 8)
	pScrn->rgbBits = 8;

    if (pScrn->depth > 8) {
      if (!xf86SetWeight(pScrn, defaultWeight, defaultWeight))
        return FALSE;
    }

    if (!xf86SetDefaultVisual(pScrn, -1)) {
      return FALSE;
    } else {
      /* We don't currently support DirectColor at > 8bpp */
      if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Given default visual"
		 " (%s) is not supported at depth %d\n",
		 xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
        return FALSE;
      }
    }

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);

    /* Process the options */
    if (!(pPnozz->Options = xalloc(sizeof(PnozzOptions))))
	return FALSE;

    memcpy(pPnozz->Options, PnozzOptions, sizeof(PnozzOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pPnozz->Options);
    
    /*
     * The new cmap code requires this to be initialised.
     */

    {
	Gamma zeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScrn, zeros)) {
	    return FALSE;
	}
    }

    /* Set the bits per RGB for 8bpp mode */
    from = X_DEFAULT;

    /* determine whether we use hardware or software cursor */
    pPnozz->HWCursor = TRUE;
    if (xf86GetOptValBool(pPnozz->Options, OPTION_HW_CURSOR, &pPnozz->HWCursor))
	from = X_CONFIG;
    if (xf86ReturnOptValBool(pPnozz->Options, OPTION_SW_CURSOR, FALSE)) {
	from = X_CONFIG;
	pPnozz->HWCursor = FALSE;
    }
   
    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pPnozz->HWCursor ? "HW" : "SW");

    if (xf86ReturnOptValBool(pPnozz->Options, OPTION_NOACCEL, FALSE)) {
	pPnozz->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
    }
        
    if (xf86LoadSubModule(pScrn, "fb") == NULL) {
	PnozzFreeRec(pScrn);
	return FALSE;
    }

    if (xf86LoadSubModule(pScrn, "ramdac") == NULL) {
	PnozzFreeRec(pScrn);
	return FALSE;
    }
    xf86LoaderReqSymLists(ramdacSymbols, NULL);

    if (xf86LoadSubModule(pScrn, "xaa") == NULL) {
	PnozzFreeRec(pScrn);
	return FALSE;
    }
    xf86LoaderReqSymLists(xaaSymbols, NULL);

    /*********************
    set up clock and mode stuff
    *********************/
    
    pScrn->progClock = TRUE;

    if(pScrn->display->virtualX || pScrn->display->virtualY) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Pnozz does not support a virtual desktop\n");
	pScrn->display->virtualX = 0;
	pScrn->display->virtualY = 0;
    }

    xf86SbusUseBuiltinMode(pScrn, pPnozz->psdp);
    pScrn->currentMode = pScrn->modes;
    pScrn->displayWidth = pScrn->virtualX;

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    return TRUE;
}

/* Mandatory */

/* This gets called at the start of each server generation */

static Bool
PnozzScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn;
    PnozzPtr pPnozz;
    VisualPtr visual;
    int ret,len=0,i;
    unsigned int *regs, pctl, pfb, *fb;

    /* 
     * First get the ScrnInfoRec
     */
    pScrn = xf86Screens[pScreen->myNum];

    pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    /*
     * XXX
     * figure out how much video RAM we really have - 2MB is just by far the 
     * most common size
     */
    pPnozz->fb =
	xf86MapSbusMem (pPnozz->psdp, 0, 0x200000);	/* map 2MB */
    fb=(unsigned int *)pPnozz->fb;
    
    pPnozz->fbc =
	xf86MapSbusMem (pPnozz->psdp, 0x200000,0x8000);	/* map registers */

    if (! pPnozz->fbc)
	return FALSE;

    /*
     * The next step is to setup the screen's visuals, and initialise the
     * framebuffer code.  In cases where the framebuffer's default
     * choices for things like visual layouts and bits per RGB are OK,
     * this may be as simple as calling the framebuffer's ScreenInit()
     * function.  If not, the visuals will need to be setup before calling
     * a fb ScreenInit() function and fixed up after.
     */

    /*
     * Reset visual list.
     */
    miClearVisualTypes();

#ifdef DEBUG
    xf86Msg(X_ERROR, "depth: %d, bpp: %d\n", pScrn->depth, pScrn->bitsPerPixel);
#endif    
    switch (pScrn->bitsPerPixel) {
    	case 8:
	    pPnozz->depthshift = 0;
	    break;    	
    	case 16:
	    pPnozz->depthshift = 1;
	    break;    	
    	case 32:
	    pPnozz->depthshift = 2;
	    break;    	
	default:
	    return FALSE;
    }
    pPnozz->width = pScrn->virtualX;
    pPnozz->height = pScrn->virtualY;
    pPnozz->scanlinesize = pScrn->virtualX << pPnozz->depthshift;
    
    PnozzSave(pPnozz);

    /* 
     * ok, let's switch to whatever depth That Guy Out There wants.
     * We won't switch video mode, only colour depth - 
     */
    if(!PnozzSetDepth(pPnozz, pScrn->bitsPerPixel)) 
    	return FALSE;

    /* Setup the visuals we support. */

    if (!miSetVisualTypes(pScrn->depth, miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits, pScrn->defaultVisual))
	return FALSE;
	
    miSetPixmapDepths();

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */
#if DEBUG
    xf86Msg(X_ERROR, "sls: %d\n", pPnozz->scanlinesize);
#endif
    ret = fbScreenInit(pScreen, pPnozz->fb, pScrn->virtualX,
		       pScrn->virtualY, pScrn->xDpi, pScrn->yDpi,
		       pScrn->displayWidth, pScrn->bitsPerPixel);

    /* should be set by PnozzSetDepth() */
    pPnozz->maxheight = (0x200000 / pPnozz->scanlinesize) & 0xffff;
#if DEBUG
    xf86Msg(X_ERROR, "max scanlines: %d\n", pPnozz->maxheight);
#endif
    if (!ret)
	return FALSE;

    if (pScrn->bitsPerPixel > 8) {
      visual = pScreen->visuals + pScreen->numVisuals;
      while (--visual >= pScreen->visuals) {
        if ((visual->class | DynamicClass) == DirectColor) {
	  visual->offsetRed = pScrn->offset.red;
	  visual->offsetGreen = pScrn->offset.green;
	  visual->offsetBlue = pScrn->offset.blue;
	  visual->redMask = pScrn->mask.red;
	  visual->greenMask = pScrn->mask.green;
	  visual->blueMask = pScrn->mask.blue;
        }
      }
    }

    fbPictureInit(pScreen, 0, 0);

    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);
    xf86SetBlackWhitePixels(pScreen);

    if (!pPnozz->NoAccel) {
    	BoxRec bx;
	pPnozz->pXAA = XAACreateInfoRec();
	PnozzAccelInit(pScrn);
	bx.x1 = bx.y1 = 0;
	bx.x2 = pPnozz->width;
	bx.y2 = pPnozz->maxheight;
	xf86InitFBManager(pScreen, &bx);
	if(!XAAInit(pScreen, pPnozz->pXAA))
	    return FALSE;
	xf86Msg(X_INFO, "%s: Using acceleration\n", pPnozz->psdp->device);
    }

    /* Initialise cursor functions */
    miDCInitialize (pScreen, xf86GetPointerScreenFuncs());

    /*
     * Initialize HW cursor layer. 
     * Must follow software cursor initialization
     */
    xf86SbusHideOsHwCursor(pPnozz->psdp);
    if (pPnozz->HWCursor) { 
	extern Bool PnozzHWCursorInit(ScreenPtr pScreen);

	if(!PnozzHWCursorInit(pScreen)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
		       "Hardware cursor initialization failed\n");
	    return(FALSE);
	}
    }

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen))
	return FALSE;
#if 1
    if(!xf86SbusHandleColormaps(pScreen, pPnozz->psdp))
#else
    if(!xf86HandleColormaps(pScreen, 256, pScrn->rgbBits, PnozzLoadPalette, NULL, 
        /*CMAP_PALETTED_TRUECOLOR|*/CMAP_RELOAD_ON_MODE_SWITCH))
#endif
	return FALSE;
    pPnozz->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = PnozzCloseScreen;
    pScreen->SaveScreen = PnozzSaveScreen;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    /* unblank the screen */
    PnozzSaveScreen(pScreen, SCREEN_SAVER_OFF);

    /* Done */
    return TRUE;
}


/* Usually mandatory */
static Bool
PnozzSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    xf86Msg(X_ERROR, "SwitchMode: %d %d %d %d\n", mode->CrtcHTotal, 
        mode->CrtcHSyncStart, mode->CrtcHSyncEnd, mode->CrtcHDisplay);
    return TRUE;
}


/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
static void 
PnozzAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    /* we don't support virtual desktops for now */
    return;
}

/*
 * This is called when VT switching back to the X server.  Its job is
 * to reinitialise the video mode.
 */

/* Mandatory */
static Bool
PnozzEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);

    xf86SbusHideOsHwCursor (pPnozz->psdp);
    return TRUE;
}


/*
 * This is called when VT switching away from the X server.
 */

/* Mandatory */
static void
PnozzLeaveVT(int scrnIndex, int flags)
{
    return;
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 */

/* Mandatory */
static Bool
PnozzCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    int state = 1;

    pScrn->vtSema = FALSE;
    
    if (pPnozz->HWCursor)
    	PnozzHideCursor(pScrn);

    PnozzRestore(pPnozz);	/* restore colour depth */
    
    xf86UnmapSbusMem(pPnozz->psdp, pPnozz->fb,0x200000);
    xf86UnmapSbusMem(pPnozz->psdp, pPnozz->fbc,0x8000);

    /* make sure video is turned on */
    ioctl(pPnozz->psdp->fd, FBIOSVIDEO, &state);
    
    pScreen->CloseScreen = pPnozz->CloseScreen;
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
    return FALSE;
}


/* Free up any per-generation data structures */

/* Optional */
static void
PnozzFreeScreen(int scrnIndex, int flags)
{
    PnozzFreeRec(xf86Screens[scrnIndex]);
}


/* Checks if a mode is suitable for the selected chipset. */

/* Optional */
static ModeStatus
PnozzValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
    if (mode->Flags & V_INTERLACE)
	return(MODE_BAD);

    return(MODE_OK);
}

/* Do screen blanking */

/* Mandatory */
static Bool
PnozzSaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    PnozzPtr pPnozz = GET_PNOZZ_FROM_SCRN(pScrn);
    int fd = pPnozz->psdp->fd, state;
    
    /* 
     * we're using ioctl() instead of just whacking the DAC because the 
     * underlying driver will also turn off the backlight which we couldn't do 
     * from here without adding lots more hardware dependencies 
     */
    switch(mode)
    {
	case SCREEN_SAVER_ON:
	case SCREEN_SAVER_CYCLE:
    		state = 0;
		if(ioctl(fd, FBIOSVIDEO, &state) == -1)
		{
			/* complain */
		}
		break;
	case SCREEN_SAVER_OFF:
	case SCREEN_SAVER_FORCER:
    		state = 1;
		if(ioctl(fd, FBIOSVIDEO, &state) == -1)
		{
			/* complain */
		}
		break;
	default:
		return FALSE;
    }
    return TRUE;
}

int shift_1(int b)
{
    if (b > 0)
	return (16 << b);
    return 0;
}

int shift_2(int b)
{
    if (b > 0)
	return (512 << b);
    return 0;
}

void
PnozzSave(PnozzPtr pPnozz)
{
	int i;
	pPnozz->SvSysConf = pnozz_read_4(pPnozz, SYS_CONF);
	pPnozz->SvDAC_MC3 = pnozz_read_dac_ctl_reg(pPnozz, DAC_MISC_3);
	pPnozz->SvDAC_MCCR = pnozz_read_dac_ctl_reg(pPnozz, DAC_MISC_CLK);
	pPnozz->SvDAC_PF = pnozz_read_dac_ctl_reg(pPnozz, DAC_PIXEL_FMT);
	pPnozz->SvPLL = pnozz_read_dac_ctl_reg(pPnozz, DAC_PLL0);
	pPnozz->SvVCO = pnozz_read_dac_ctl_reg(pPnozz, DAC_VCO_DIV);
	pPnozz->SvMemCtl = pnozz_read_4(pPnozz, VID_MEM_CONFIG);
	for (i = 0; i < 4; i++)
		pPnozz->CRTC[i] = pnozz_read_4(pPnozz, VID_HTOTAL + (i << 2));
	pPnozz->DidSave = 1;
#if DEBUG
	xf86Msg(X_ERROR, "Saved: %x %x %x %x\n", pPnozz->SvSysConf, 
	    pPnozz->SvDAC_MCCR, pPnozz->SvDAC_PF, pPnozz->SvDAC_MC3);
	DumpSCR(pPnozz->SvSysConf);
#endif
} 

void DumpSCR(unsigned int scr)
{
#if DEBUG
	int s0, s1, s2, s3, ps;
	int width;
	ps = (scr >> PIXEL_SHIFT) & 7;
	s0 = (scr >> SHIFT_0) & 7;
	s1 = (scr >> SHIFT_1) & 7;
	s2 = (scr >> SHIFT_2) & 7;
	s3 = (scr >> SHIFT_3) & 3;
	width = shift_1(s0) + shift_1(s1) + shift_1(s2) + shift_2(s3);
	xf86Msg(X_ERROR, "ps: %d wi: %d\n", ps, width);
#endif
}

void DumpDAC(PnozzPtr pPnozz)
{
#if DEBUG
    int addr, i, val;
    char line[256], buffer[16];
    pnozz_write_dac(pPnozz, DAC_INDX_LO, 0);
    pnozz_write_dac(pPnozz, DAC_INDX_HI, 0);
    for (addr = 0; addr < 0x100; addr += 16) {
    	snprintf(line, 16, "%02x:", addr);
	for (i=0;i<16;i++) {
	    val = pnozz_read_dac(pPnozz, DAC_INDX_DATA);
	    snprintf(buffer, 16, " %02x", val);
	    strcat(line, buffer);
	}
	xf86Msg(X_ERROR, "%s\n", line);
    }
#endif
}
   
void DumpCRTC(PnozzPtr pPnozz)
{
#if DEBUG
    int i;
    unsigned int reg;
    for (i = 0x108; i<0x140; i += 4) {
        reg = pnozz_read_4(pPnozz, i);
	xf86Msg(X_ERROR, "%x / %d ", reg, reg);
    }
    reg = pnozz_read_4(pPnozz, VID_MEM_CONFIG);
    xf86Msg(X_ERROR, "memcfg: %08x\n", reg);
    xf86Msg(X_ERROR, "shiftclk:  %x\n", (reg >> 10) & 7);
    xf86Msg(X_ERROR, "shiftmode: %x\n", (reg >> 22) & 3);
    xf86Msg(X_ERROR, "crtc_clk:  %x\n", (reg >> 13) & 7);
#endif
}

void
PnozzRestore(PnozzPtr pPnozz)
{
    int i;
    if(pPnozz->DidSave == 1) {
	pnozz_write_4(pPnozz, SYS_CONF, pPnozz->SvSysConf);
	pnozz_write_4(pPnozz, VID_MEM_CONFIG, pPnozz->SvMemCtl);
	for (i = 0; i < 4; i++)
	    pnozz_write_4(pPnozz, VID_HTOTAL + (i << 2), pPnozz->CRTC[i]);

        pnozz_write_dac_ctl_reg(pPnozz, DAC_PLL0, pPnozz->SvPLL);
        pnozz_write_dac_ctl_reg(pPnozz, DAC_MISC_3, pPnozz->SvDAC_MC3);
        pnozz_write_dac_ctl_reg(pPnozz, DAC_MISC_CLK, pPnozz->SvDAC_MCCR);
	pnozz_write_dac_ctl_reg(pPnozz, DAC_PIXEL_FMT, pPnozz->SvDAC_PF);
	pnozz_write_dac_ctl_reg(pPnozz, DAC_VCO_DIV, pPnozz->SvVCO);
    }
} 

unsigned int upper_bit(unsigned int b)
{
	unsigned int mask=0x80000000;
	int cnt = 31;
	if (b == 0)
		return -1;
	while ((mask != 0) && ((b & mask) == 0)) {
		mask = mask >> 1;
		cnt--;
	}
	return cnt;
}

/*
 * To switch colour depth we need to:
 * - double or quadruple both crtc and shift clock ( for 16 or 32 bit )
 * - double or quadruple scanline length
 * - switch the DAC to the appropriate pixel format
 * - tell the drawing engine about new line length / pixel size
 */

int
PnozzSetDepth(PnozzPtr pPnozz, int depth)
{
    int new_sls;
    unsigned int bits, scr, sscr, memctl, mem;
    int s0, s1, s2, s3, ps, crtcline;
    unsigned char pf, mc3, es;

#if DEBUG
    DumpDAC(pPnozz);
    DumpCRTC(pPnozz);
#endif

    switch (depth) {
	case 8:
	    pPnozz->depthshift = 0;
	    ps = 2;
	    pf = 3;
	    mc3 = 0;
	    es = 0;	/* no swapping */
	    memctl = 3;
	    break;
	case 16:
	    pPnozz->depthshift = 1;
	    ps = 3;
	    pf = 4;
	    mc3 = 0;
	    es = 2;	/* swap bytes in 16bit words */
	    memctl = 2;
	    break;
	case 24:
	    /* boo */
	    xf86Msg(X_ERROR, "We don't DO 24bit pixels dammit!\n");
	    return 0;
	case 32:
	    pPnozz->depthshift = 2;
	    ps = 5;
	    pf = 6;
	    mc3 = 0;
	    es = 6;	/* swap both half-words and bytes */
	    memctl = 1;	/* 0 */
	    break;
    }
    /*
     * this could be done a lot shorter and faster but then nobody would 
     * understand what the hell we're doing here without getting a major 
     * headache. Scanline size is encoded as 4 shift values, 3 of them 3 bits 
     * wide, 16 << n for n>0, one 2 bits, 512 << n for n>0. n==0 means 0
     */
    new_sls = pPnozz->width << pPnozz->depthshift;
    pPnozz->scanlinesize = new_sls;
    bits = new_sls;
    s3 = upper_bit(bits);
    if (s3 > 9) {
	bits &= ~(1 << s3);
	s3 -= 9;
    } else s3 = 0;
    s2 = upper_bit(bits);
    if (s2 > 0) {
	bits &= ~(1 << s2);
	s2 -= 4;
    } else s2 = 0;
    s1 = upper_bit(bits);
    if (s1 > 0) {
        bits &= ~(1 << s1);
        s1 -= 4;
    } else s1 = 0;
    s0 = upper_bit(bits);
    if (s0 > 0) {
        bits &= ~(1 << s0);
        s0 -= 4;
    } else s0 = 0;

#if DEBUG
    xf86Msg(X_ERROR, "sls: %x sh: %d %d %d %d leftover: %x\n", new_sls, s0, s1, 
        s2, s3, bits);
#endif

    /* 
     * now let's put these values into the System Config Register. No need to 
     * read it here since we (hopefully) just saved the content 
     */
    scr = pnozz_read_4(pPnozz, SYS_CONF);
    scr = (s0 << SHIFT_0) | (s1 << SHIFT_1) | (s2 << SHIFT_2) | 
        (s3 << SHIFT_3) | (ps << PIXEL_SHIFT) | (es << SWAP_SHIFT);
#if DEBUG
    xf86Msg(X_ERROR, "new scr: %x DAC %x %x\n", scr, pf, mc3);
    DumpSCR(scr);
#endif
    
    mem = pnozz_read_4(pPnozz, VID_MEM_CONFIG);
#if DEBUG
    xf86Msg(X_ERROR, "old memctl: %08x\n", mem);
#endif
    /* set shift and crtc clock */
    mem &= ~(0x0000fc00);
    mem |= (memctl << 10) | (memctl << 13);
    pnozz_write_4(pPnozz, VID_MEM_CONFIG, mem);
#if DEBUG
    xf86Msg(X_ERROR, "new memctl: %08x\n", mem);
#endif    
    /* whack the engine... */
    pnozz_write_4(pPnozz, SYS_CONF, scr);
    
    /* ok, whack the DAC */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_MISC_1, 0x11);
    pnozz_write_dac_ctl_reg(pPnozz, DAC_MISC_2, 0x45);
    pnozz_write_dac_ctl_reg(pPnozz, DAC_MISC_3, mc3);
    /* 
     * despite the 3GX manual saying otherwise we don't need to mess with any
     * clock dividers here
     */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_MISC_CLK, 1);
    pnozz_write_dac_ctl_reg(pPnozz, 3, 0);
    pnozz_write_dac_ctl_reg(pPnozz, 4, 0);
    
    pnozz_write_dac_ctl_reg(pPnozz, DAC_POWER_MGT, 0);
    pnozz_write_dac_ctl_reg(pPnozz, DAC_OPERATION, 0);
    pnozz_write_dac_ctl_reg(pPnozz, DAC_PALETTE_CTRL, 0);

    pnozz_write_dac_ctl_reg(pPnozz, DAC_PIXEL_FMT, pf);
    
    /* TODO: distinguish between 15 and 16 bit */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_8BIT_CTRL, 0);
    /* direct colour, linear, 565 */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_16BIT_CTRL, 0xc6);
    /* direct colour */
    pnozz_write_dac_ctl_reg(pPnozz, DAC_32BIT_CTRL, 3);
    
    pnozz_write_dac_ctl_reg(pPnozz, 0x10, 2);
    pnozz_write_dac_ctl_reg(pPnozz, 0x11, 0);
    pnozz_write_dac_ctl_reg(pPnozz, 0x14, 5);
    pnozz_write_dac_ctl_reg(pPnozz, 0x08, 1);
    pnozz_write_dac_ctl_reg(pPnozz, 0x15, 5);
    pnozz_write_dac_ctl_reg(pPnozz, 0x16, 0x63);
   
    /* whack the CRTC */
    /* we always transfer 64bit in one go */
    crtcline = pPnozz->scanlinesize >> 3;
#if DEBUG
    xf86Msg(X_ERROR, "crtcline: %d\n", crtcline);
#endif
    pnozz_write_4(pPnozz, VID_HTOTAL, (24 << pPnozz->depthshift) + crtcline);
    pnozz_write_4(pPnozz, VID_HSRE, 8 << pPnozz->depthshift);
    pnozz_write_4(pPnozz, VID_HBRE, 18 << pPnozz->depthshift);
    pnozz_write_4(pPnozz, VID_HBFE, (18 << pPnozz->depthshift) + crtcline);

#if DEBUG
    sscr = pnozz_read_4(pPnozz, SYS_CONF);
    xf86Msg(X_ERROR, "scr: %x\n", sscr);
#endif
    return TRUE;
}
