/*
 * Fujitsu AG-10e framebuffer driver.
 *
 * Copyright (C) 2007 Michael Lorenz
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
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6_driver.c,v 1.12 2005/02/18 02:55:09 dawes Exp $ */

/* need this for PRIxPTR macro */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dev/sun/fbio.h>
#include <dev/wscons/wsconsio.h>

#include <machine/int_fmtio.h>
#include "xf86.h"
#include "xf86_OSproc.h"
#include "mipointer.h"
#include "micmap.h"

#include "fb.h"
#include "xf86cmap.h"
#include "ag10e.h"
#include "xf86sbusBus.h"

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 6
#define xf86LoaderReqSymLists(...) do {} while (0)
#define LoaderRefSymLists(...) do {} while (0)
#endif

static const OptionInfoRec * AG10EAvailableOptions(int chipid, int busid);
static void	AG10EIdentify(int flags);
static Bool	AG10EProbe(DriverPtr drv, int flags);
static Bool	AG10EPreInit(ScrnInfoPtr pScrn, int flags);
static Bool	AG10EScreenInit(int Index, ScreenPtr pScreen, int argc,
			      char **argv);
static Bool	AG10EEnterVT(int scrnIndex, int flags);
static void	AG10ELeaveVT(int scrnIndex, int flags);
static Bool	AG10ECloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool	AG10ESaveScreen(ScreenPtr pScreen, int mode);

/* Required if the driver supports mode switching */
static Bool	AG10ESwitchMode(int scrnIndex, DisplayModePtr mode, int flags);
/* Required if the driver supports moving the viewport */
static void	AG10EAdjustFrame(int scrnIndex, int x, int y, int flags);

/* Optional functions */
static void	AG10EFreeScreen(int scrnIndex, int flags);
static ModeStatus AG10EValidMode(int scrnIndex, DisplayModePtr mode,
			       Bool verbose, int flags);

#define VERSION 4000
#define AG10E_NAME "AG10E"
#define AG10E_DRIVER_NAME "ag10e"
#define AG10E_MAJOR_VERSION 1
#define AG10E_MINOR_VERSION 0
#define AG10E_PATCHLEVEL 0

/*
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

DriverRec AG10E = {
    VERSION,
    AG10E_DRIVER_NAME,
    AG10EIdentify,
    AG10EProbe,
    AG10EAvailableOptions,
    NULL,
    0
};

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_NOACCEL
} AG10EOpts;

static const OptionInfoRec AG10EOptions[] = {
    { OPTION_SW_CURSOR,		"SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_HW_CURSOR,		"HWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_NOACCEL,		"NoAccel",	OPTV_BOOLEAN,	{0}, FALSE },
    { -1,			NULL,		OPTV_NONE,	{0}, FALSE }
};

static const char *xaaSymbols[] =
{
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAInit",
    NULL
};

#ifdef XFree86LOADER

static MODULESETUPPROTO(AG10ESetup);

static XF86ModuleVersionInfo AG10EVersRec =
{
	"ag10e",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	AG10E_MAJOR_VERSION, AG10E_MINOR_VERSION, AG10E_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0,0,0,0}
};

_X_EXPORT XF86ModuleData ag10eModuleData = { &AG10EVersRec, AG10ESetup, NULL };

static pointer
AG10ESetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	xf86AddDriver(&AG10E, module, 0);

	/*
	 * Modules that this driver always requires can be loaded here
	 * by calling LoadSubModule().
	 */
	LoaderRefSymLists(xaaSymbols, NULL);

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

static Bool
AG10EGetRec(ScrnInfoPtr pScrn)
{
    /*
     * Allocate an AG10ERec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate != NULL)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(AG10ERec), 1);
    return TRUE;
}

static void
AG10EFreeRec(ScrnInfoPtr pScrn)
{
    AG10EPtr pAG10E;

    if (pScrn->driverPrivate == NULL)
	return;

    pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    xfree(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;

    return;
}

static const OptionInfoRec *
AG10EAvailableOptions(int chipid, int busid)
{
    return AG10EOptions;
}

/* Mandatory */
static void
AG10EIdentify(int flags)
{
    xf86Msg(X_INFO, "%s: driver for Fujitsu AG-10e\n", AG10E_NAME);
}


/* Mandatory */
static Bool
AG10EProbe(DriverPtr drv, int flags)
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

    if ((numDevSections = xf86MatchDevice(AG10E_DRIVER_NAME,
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

    numUsed = xf86MatchSbusInstances(AG10E_NAME, SBUS_DEVICE_AG10E,
		   devSections, numDevSections,
		   drv, &usedChips);

    xfree(devSections);
    if (numUsed <= 0)
	return FALSE;

    if (flags & PROBE_DETECT)
	foundScreen = TRUE;
    else for (i = 0; i < numUsed; i++) {
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
	    pScrn->driverName	 = AG10E_DRIVER_NAME;
	    pScrn->name		 = AG10E_NAME;
	    pScrn->Probe	 = AG10EProbe;
	    pScrn->PreInit	 = AG10EPreInit;
	    pScrn->ScreenInit	 = AG10EScreenInit;
	    pScrn->SwitchMode	 = AG10ESwitchMode;
	    pScrn->AdjustFrame	 = AG10EAdjustFrame;
	    pScrn->EnterVT	 = AG10EEnterVT;
	    pScrn->LeaveVT	 = AG10ELeaveVT;
	    pScrn->FreeScreen	 = AG10EFreeScreen;
	    pScrn->ValidMode	 = AG10EValidMode;
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
AG10EPreInit(ScrnInfoPtr pScrn, int flags)
{
    AG10EPtr pAG10E;
    sbusDevicePtr psdp;
    rgb defaultWeight = {0, 0, 0};
    MessageType from;
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

    /* Allocate the AG10ERec driverPrivate */
    if (!AG10EGetRec(pScrn)) {
	return FALSE;
    }
    pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    /* This driver doesn't expect more than one entity per screen */
    if (pScrn->numEntities > 1)
	return FALSE;
    /* This is the general case */
    for (i = 0; i < pScrn->numEntities; i++) {
	EntityInfoPtr pEnt = xf86GetEntityInfo(pScrn->entityList[i]);

	/* AG10E is purely SBUS */
	if (pEnt->location.type == BUS_SBUS) {
	    psdp = xf86GetSbusInfoForEntity(pEnt->index);
	    pAG10E->psdp = psdp;
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
	switch (pScrn->depth) {
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

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);
    /* Process the options */
    if (!(pAG10E->Options = xalloc(sizeof(AG10EOptions))))
	return FALSE;
    memcpy(pAG10E->Options, AG10EOptions, sizeof(AG10EOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pAG10E->Options);

    /*
     * The new cmap code requires this to be initialised.
     * this card supports HW gamma correction with 10 bit resolution - maybe
     * we should figure out how to use it
     */

    {
	Gamma zeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScrn, zeros)) {
	    return FALSE;
	}
    }

    from = X_DEFAULT;
    if (!xf86SetWeight(pScrn, defaultWeight, defaultWeight))
      return FALSE;

    if (!xf86SetDefaultVisual(pScrn, -1))
	return FALSE;

    /* determine whether we use hardware or software cursor */

    pAG10E->HWCursor = TRUE;
    if (xf86GetOptValBool(pAG10E->Options, OPTION_HW_CURSOR, &pAG10E->HWCursor))
	from = X_CONFIG;
    if (xf86ReturnOptValBool(pAG10E->Options, OPTION_SW_CURSOR, FALSE)) {
	from = X_CONFIG;
	pAG10E->HWCursor = FALSE;
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pAG10E->HWCursor ? "HW" : "SW");

    if (xf86ReturnOptValBool(pAG10E->Options, OPTION_NOACCEL, FALSE)) {
	pAG10E->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
    }
        
    if (xf86LoadSubModule(pScrn, "fb") == NULL) {
	AG10EFreeRec(pScrn);
	return FALSE;
    }

    if (pAG10E->HWCursor && xf86LoadSubModule(pScrn, "ramdac") == NULL) {
	AG10EFreeRec(pScrn);
	return FALSE;
    }

    if (pAG10E->HWCursor && xf86LoadSubModule(pScrn, "xaa") == NULL) {
	AG10EFreeRec(pScrn);
	return FALSE;
    }
    xf86LoaderReqSymLists(xaaSymbols, NULL);

    /*********************
    set up clock and mode stuff
    *********************/

    pScrn->progClock = TRUE;

    if(pScrn->display->virtualX || pScrn->display->virtualY) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "AG10E does not support a virtual desktop\n");
	pScrn->display->virtualX = 0;
	pScrn->display->virtualY = 0;
    }

    xf86SbusUseBuiltinMode(pScrn, pAG10E->psdp);
    pScrn->currentMode = pScrn->modes;
    pScrn->displayWidth = pScrn->virtualX;

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    return TRUE;
}

/* Mandatory */

/* This gets called at the start of each server generation */

static Bool
AG10EScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
    ScrnInfoPtr pScrn;
    AG10EPtr pAG10E;
    struct fbtype fb;
    sbusDevicePtr psdp;
    VisualPtr visual;
    int ret;

    pScrn = xf86Screens[pScreen->myNum];
    pAG10E = GET_AG10E_FROM_SCRN(pScrn);
    psdp = pAG10E->psdp;

    /*
     * for some idiotic reason we need to check if the file descriptor is
     * really open here
     */
    if (psdp->fd == -1) {
	psdp->fd = open(psdp->device, O_RDWR);
	if (psdp->fd == -1)
	    return FALSE;
    }

    /* figure out how much VRAM we can map */
    if ((ret = ioctl(pAG10E->psdp->fd, FBIOGTYPE, &fb)) != 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	    "ioctl(FBIOGTYPE) failed with %d\n", ret);
	return FALSE;
    }
    pAG10E->vidmem = fb.fb_size;

    /* Map AG10E memory areas */
    
    pAG10E->regs = xf86MapSbusMem(psdp, pAG10E->vidmem, 0x10000);
    pAG10E->fb = xf86MapSbusMem(psdp, 0, pAG10E->vidmem);
    
    if (pAG10E->fb != NULL) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "mapped %d KB video RAM\n", 
	    pAG10E->vidmem >> 10);
    }
    
    if (!pAG10E->regs || !pAG10E->fb) {
    	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "xf86MapSbusMem failed regs:%" PRIxPTR " fb:%" PRIxPTR "\n",
		   pAG10E->regs, pAG10E->fb);
    
	if (pAG10E->fb) {
	    xf86UnmapSbusMem(psdp, pAG10E->fb, sizeof(*pAG10E->fb));
	    pAG10E->fb = NULL;
	}

	if (pAG10E->regs) {
	    xf86UnmapSbusMem(psdp, pAG10E->regs, sizeof(*pAG10E->regs));
	    pAG10E->regs = NULL;
	}

	return FALSE;
    }
    pAG10E->IOOffset = 0;
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "vram: %d\n",
        (1 << ((GLINT_READ_REG(FBMemoryCtl) & 0xE0000000)>>29)) * 1024);

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

    /* Setup the visuals we support. */
    if (!miSetVisualTypes(pScrn->depth, miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits, pScrn->defaultVisual))
	return FALSE;

    miSetPixmapDepths();

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */

    ret = fbScreenInit(pScreen, pAG10E->fb, pScrn->virtualX,
		       pScrn->virtualY, pScrn->xDpi, pScrn->yDpi,
		       pScrn->virtualX, pScrn->bitsPerPixel);
    /*if (!ret)
	return FALSE;*/

    pAG10E->width = pScrn->virtualX;
    pAG10E->height = pScrn->virtualY;
    pAG10E->maxheight = (pAG10E->vidmem / (pAG10E->width << 2)) & 0xffff;

    fbPictureInit(pScreen, 0, 0);

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

    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);

    xf86SetBlackWhitePixels(pScreen);

    if (!pAG10E->NoAccel) {
	if (!AG10EAccelInit(pScreen))
		return FALSE;
	xf86Msg(X_INFO, "%s: Using acceleration\n", pAG10E->psdp->device);
    }
    /* setup DGA */
    AG10EDGAInit(pScreen);

    /* Initialise cursor functions */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* Initialize HW cursor layer.
       Must follow software cursor initialization*/
    if (pAG10E->HWCursor) {
	if(!AG10EHWCursorInit(pScreen)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "Hardware cursor initialization failed\n");
	    return(FALSE);
	}
	xf86SbusHideOsHwCursor(psdp);
    }

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen))
	return FALSE;

    if(!xf86SbusHandleColormaps(pScreen, psdp))
	return FALSE;

    pAG10E->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = AG10ECloseScreen;
    pScreen->SaveScreen = AG10ESaveScreen;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    /* unblank the screen */
    AG10ESaveScreen(pScreen, SCREEN_SAVER_OFF);

    /* Done */
    return TRUE;
}


/* Usually mandatory */
static Bool
AG10ESwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
    return TRUE;
}


/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
static void
AG10EAdjustFrame(int scrnIndex, int x, int y, int flags)
{
    /* we don't support virtual desktops */
    return;
}

/*
 * This is called when VT switching back to the X server.  Its job is
 * to reinitialise the video mode.
 */

/* Mandatory */
static Bool
AG10EEnterVT(int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);

    if (pAG10E->HWCursor) {
	xf86SbusHideOsHwCursor(pAG10E->psdp);
    }
    return TRUE;
}


/*
 * This is called when VT switching away from the X server.
 */

/* Mandatory */
static void
AG10ELeaveVT(int scrnIndex, int flags)
{
    return;
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 */

/* Mandatory */
static Bool
AG10ECloseScreen(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);
    sbusDevicePtr psdp = pAG10E->psdp;

    pScrn->vtSema = FALSE;

    xf86UnmapSbusMem(psdp, pAG10E->regs, 0x10000);
    xf86UnmapSbusMem(psdp, pAG10E->fb, pAG10E->vidmem);

    if (pAG10E->HWCursor)
	xf86SbusHideOsHwCursor(psdp);

    pScreen->CloseScreen = pAG10E->CloseScreen;
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
    return FALSE;
}


/* Free up any per-generation data structures */

/* Optional */
static void
AG10EFreeScreen(int scrnIndex, int flags)
{
    AG10EFreeRec(xf86Screens[scrnIndex]);
}


/* Checks if a mode is suitable for the selected chipset. */

/* Optional */
static ModeStatus
AG10EValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
    if (mode->Flags & V_INTERLACE)
	return(MODE_NO_INTERLACE);

    return(MODE_OK);
}

/* Do screen blanking */

/* Mandatory */
static Bool
AG10ESaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    AG10EPtr pAG10E = GET_AG10E_FROM_SCRN(pScrn);
    int flag;

    switch (mode)
    {
	case SCREEN_SAVER_ON:
	case SCREEN_SAVER_CYCLE:
	    flag = 0;
	    ioctl(pAG10E->psdp->fd, FBIOSVIDEO, &flag);
	    break;
	case SCREEN_SAVER_OFF:
	case SCREEN_SAVER_FORCER:
	    flag = 1;
	    ioctl(pAG10E->psdp->fd, FBIOSVIDEO, &flag);
	    break;
	default:
	    return FALSE;
    }

    return TRUE;
}
