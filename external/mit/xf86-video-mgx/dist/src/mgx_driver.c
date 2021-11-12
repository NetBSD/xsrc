/*
 * Southland Media MGX framebuffer driver.
 *
 * Copyright (C) 2021 Michael Lorenz
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
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <dev/sun/fbio.h>
#include <dev/wscons/wsconsio.h>

#include "mgx.h"

#include "mipointer.h"
#include "micmap.h"

#include "fb.h"

#include <dev/sbus/mgxreg.h>

static const OptionInfoRec * MgxAvailableOptions(int chipid, int busid);
static void	MgxIdentify(int flags);
static Bool	MgxProbe(DriverPtr drv, int flags);
static Bool	MgxPreInit(ScrnInfoPtr pScrn, int flags);
static Bool	MgxScreenInit(SCREEN_INIT_ARGS_DECL);
static Bool	MgxEnterVT(VT_FUNC_ARGS_DECL);
static void	MgxLeaveVT(VT_FUNC_ARGS_DECL);
static Bool	MgxCloseScreen(CLOSE_SCREEN_ARGS_DECL);
static Bool	MgxSaveScreen(ScreenPtr pScreen, int mode);

/* Required if the driver supports mode switching */
static Bool	MgxSwitchMode(SWITCH_MODE_ARGS_DECL);
/* Required if the driver supports moving the viewport */
static void	MgxAdjustFrame(ADJUST_FRAME_ARGS_DECL);

/* Optional functions */
static void	MgxFreeScreen(FREE_SCREEN_ARGS_DECL);
static ModeStatus MgxValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode,
			       Bool verbose, int flags);

void MgxSync(ScrnInfoPtr pScrn);

static Bool MgxDriverFunc(ScrnInfoPtr, xorgDriverFuncOp, pointer);


#define MGX_VERSION 0001
#define MGX_NAME "MGX"
#define MGX_DRIVER_NAME "mgx"
#define MGX_MAJOR_VERSION PACKAGE_VERSION_MAJOR
#define MGX_MINOR_VERSION PACKAGE_VERSION_MINOR
#define MGX_PATCHLEVEL PACKAGE_VERSION_PATCHLEVEL

/* 
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

_X_EXPORT DriverRec MGX = {
    MGX_VERSION,
    MGX_DRIVER_NAME,
    MgxIdentify,
    MgxProbe,
    MgxAvailableOptions,
    NULL,
    0,
    MgxDriverFunc
};

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_NOACCEL
} TCXOpts;

static const OptionInfoRec MgxOptions[] = {
    { OPTION_SW_CURSOR,		"SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_HW_CURSOR,		"HWcursor",	OPTV_BOOLEAN,	{0}, TRUE  },
    { OPTION_NOACCEL,		"NoAccel",	OPTV_BOOLEAN,	{0}, FALSE },
    { -1,			NULL,		OPTV_NONE,	{0}, FALSE }
};

static MODULESETUPPROTO(MgxSetup);

static XF86ModuleVersionInfo mgxVersRec =
{
	"mgx",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	MGX_MAJOR_VERSION, MGX_MINOR_VERSION, MGX_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0,0,0,0}
};

_X_EXPORT XF86ModuleData mgxModuleData = { &mgxVersRec, MgxSetup, NULL };

pointer
MgxSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	xf86AddDriver(&MGX, module, HaveDriverFuncs);

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

static Bool
MgxGetRec(ScrnInfoPtr pScrn)
{
    /*
     * Allocate an MgxRec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate != NULL)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(MgxRec), 1);
    return TRUE;
}

static void
MgxFreeRec(ScrnInfoPtr pScrn)
{
    MgxPtr pMgx;

    if (pScrn->driverPrivate == NULL)
	return;

    pMgx = GET_MGX_FROM_SCRN(pScrn);

    free(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;

    return;
}

static const OptionInfoRec *
MgxAvailableOptions(int chipid, int busid)
{
    return MgxOptions;
}

/* Mandatory */
static void
MgxIdentify(int flags)
{
    xf86Msg(X_INFO, "%s: driver for Southland Media MGX\n", MGX_NAME);
}


/* Mandatory */
static Bool
MgxProbe(DriverPtr drv, int flags)
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

    if ((numDevSections = xf86MatchDevice(MGX_DRIVER_NAME,
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

    numUsed = xf86MatchSbusInstances(MGX_NAME, SBUS_DEVICE_MGX,
		   devSections, numDevSections,
		   drv, &usedChips);
				    
    free(devSections);
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
	    pScrn->driverVersion = MGX_VERSION;
	    pScrn->driverName	 = MGX_DRIVER_NAME;
	    pScrn->name		 = MGX_NAME;
	    pScrn->Probe	 = MgxProbe;
	    pScrn->PreInit	 = MgxPreInit;
	    pScrn->ScreenInit	 = MgxScreenInit;
  	    pScrn->SwitchMode	 = MgxSwitchMode;
  	    pScrn->AdjustFrame	 = MgxAdjustFrame;
	    pScrn->EnterVT	 = MgxEnterVT;
	    pScrn->LeaveVT	 = MgxLeaveVT;
	    pScrn->FreeScreen	 = MgxFreeScreen;
	    pScrn->ValidMode	 = MgxValidMode;
	    xf86AddEntityToScreen(pScrn, pEnt->index);
	    foundScreen = TRUE;
	}
	free(pEnt);
    }
    free(usedChips);
    return foundScreen;
}

/* Mandatory */
static Bool
MgxPreInit(ScrnInfoPtr pScrn, int flags)
{
    MgxPtr pMgx;
    sbusDevicePtr psdp = NULL;
    MessageType from;
    int i, prom;
    int hwCursor;

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

    /* Allocate the TcxRec driverPrivate */
    if (!MgxGetRec(pScrn)) {
	return FALSE;
    }
    pMgx = GET_MGX_FROM_SCRN(pScrn);
    
    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    /* This driver doesn't expect more than one entity per screen */
    if (pScrn->numEntities > 1)
	return FALSE;
    /* This is the general case */
    for (i = 0; i < pScrn->numEntities; i++) {
	EntityInfoPtr pEnt = xf86GetEntityInfo(pScrn->entityList[i]);

	if (pEnt->location.type == BUS_SBUS) {
	    psdp = xf86GetSbusInfoForEntity(pEnt->index);
	    pMgx->psdp = psdp;
	} else
	    return FALSE;
    }
    if (psdp == NULL)
	return FALSE;

    /**********************
    check card capabilities
    **********************/
    hwCursor = 1;

    prom = sparcPromInit();
	char *b;
	int len = 4, v = 0;

    /* see if we have more than 1MB vram */
    pMgx->vramsize = 0x400000;
    if (((b = sparcPromGetProperty(&psdp->node, "fb_size", &len)) != NULL)  &&
         (len == 4)) {
	memcpy(&v, b, 4);
	pMgx->vramsize = v;
    }
    xf86Msg(X_PROBED, "found %d KB video memory\n", v >> 10);

    if (prom)
    	sparcPromClose();

    xf86Msg(X_PROBED, "hardware cursor support %s\n",
      hwCursor ? "found" : "not found");

    /*********************
    deal with depth
    *********************/
    
    if (!xf86SetDepthBpp(pScrn, 0, 0, 0, Support32bppFb)) {
	return FALSE;
    } else {
	/* Check that the returned depth is one we support */
	switch (pScrn->depth) {
	case 8:
	case 32:
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
    if (!(pMgx->Options = malloc(sizeof(MgxOptions))))
	return FALSE;
    memcpy(pMgx->Options, MgxOptions, sizeof(MgxOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pMgx->Options);

    /*
     * This must happen after pScrn->display has been set because
     * xf86SetWeight references it.
     */
    if (pScrn->depth > 8) {
	rgb weight = {0, 0, 0};
	rgb mask = {0xff0000, 0xff00, 0xff};
                                       
	if (!xf86SetWeight(pScrn, weight, mask)) {
	    return FALSE;
	}
    }
                                                                           
    if (!xf86SetDefaultVisual(pScrn, -1))
	return FALSE;
    else if (pScrn->depth > 8) {
	/* We don't currently support DirectColor */
	if (pScrn->defaultVisual != TrueColor) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Given default visual"
		       " (%s) is not supported\n",
		       xf86GetVisualName(pScrn->defaultVisual));
	    return FALSE;
	}
    }                                                                                                  

    /*
     * The new cmap code requires this to be initialised.
     */

    {
	Gamma zeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScrn, zeros)) {
	    return FALSE;
	}
    }

    /* determine whether we use hardware or software cursor */
    
    from = X_PROBED;
    pMgx->HWCursor = FALSE;
    if (hwCursor) {
	from = X_DEFAULT;
	pMgx->HWCursor = TRUE;
	if (xf86GetOptValBool(pMgx->Options, OPTION_HW_CURSOR, &pMgx->HWCursor))
	    from = X_CONFIG;
	if (xf86ReturnOptValBool(pMgx->Options, OPTION_SW_CURSOR, FALSE)) {
	    from = X_CONFIG;
	    pMgx->HWCursor = FALSE;
	}
    }

    pMgx->NoAccel = FALSE;
    if (xf86ReturnOptValBool(pMgx->Options, OPTION_NOACCEL, FALSE)) {
	pMgx->NoAccel = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
    }

    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pMgx->HWCursor ? "HW" : "SW");

    if (xf86LoadSubModule(pScrn, "fb") == NULL) {
	MgxFreeRec(pScrn);
	return FALSE;
    }

    /*********************
    set up clock and mode stuff
    *********************/
    
    pScrn->progClock = TRUE;

    if(pScrn->display->virtualX || pScrn->display->virtualY) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "MGX does not support a virtual desktop\n");
	pScrn->display->virtualX = 0;
	pScrn->display->virtualY = 0;
    }

    xf86SbusUseBuiltinMode(pScrn, pMgx->psdp);
    pScrn->currentMode = pScrn->modes;
    pScrn->displayWidth = pScrn->virtualX;

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    return TRUE;
}

/* Mandatory */

/* This gets called at the start of each server generation */

static Bool
MgxScreenInit(SCREEN_INIT_ARGS_DECL)
{
    ScrnInfoPtr pScrn;
    MgxPtr pMgx;
    VisualPtr visual;
    int ret;

    /* 
     * First get the ScrnInfoRec
     */
    pScrn = xf86ScreenToScrn(pScreen);

    pMgx = GET_MGX_FROM_SCRN(pScrn);

    /* Map the fb */
    pMgx->fb = xf86MapSbusMem (pMgx->psdp, MGX_FLIPOFFSET, pMgx->vramsize);
    pMgx->blt = xf86MapSbusMem (pMgx->psdp, MGX_BLTOFFSET, 0x1000);

    if (!pMgx->blt) {
    	xf86Msg(X_ERROR, "failed to map blitter space\n");
	return FALSE;
    }

    if (!pMgx->fb) {
    	xf86Msg(X_ERROR, "failed to map framebuffer\n");
	return FALSE;
    }

    /* Darken the screen for aesthetic reasons and set the viewport */
    MgxSaveScreen(pScreen, SCREEN_SAVER_ON);

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

    if (pScrn->depth == 8)
	/* Set the bits per RGB for 8bpp mode */
	pScrn->rgbBits = 8;

    /* Setup the visuals we support. */

    if (!miSetVisualTypes(pScrn->depth,
			  pScrn->depth != 8 ? TrueColorMask :
				miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits, pScrn->defaultVisual))
	return FALSE;

    miSetPixmapDepths ();

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */

    ret = fbScreenInit(pScreen, pMgx->fb, pScrn->virtualX,
		       pScrn->virtualY, pScrn->xDpi, pScrn->yDpi,
		       pScrn->virtualX, pScrn->bitsPerPixel);

    if (!ret)
	return FALSE;

    xf86SetBlackWhitePixels(pScreen);

    if (pScrn->bitsPerPixel > 8) {
	/* Fixup RGB ordering */
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

#ifdef RENDER
    /* must be after RGB ordering fixed */
    fbPictureInit (pScreen, 0, 0);
#endif

    if (!pMgx->NoAccel) {
        XF86ModReqInfo req;
        int errmaj, errmin;

        memset(&req, 0, sizeof(XF86ModReqInfo));
        req.majorversion = 2;
        req.minorversion = 0;
        if (!LoadSubModule(pScrn->module, "exa", NULL, NULL, NULL, &req,
            &errmaj, &errmin))
        {
            LoaderErrorMsg(NULL, "exa", errmaj, errmin);
            return FALSE;
        }
	if (!MgxInitAccel(pScreen))
	    return FALSE;
    }

    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);

    /* Initialise cursor functions */
    miDCInitialize (pScreen, xf86GetPointerScreenFuncs());

    /* Initialize HW cursor layer. 
       Must follow software cursor initialization*/
    if (pMgx->HWCursor) { 

	if(!MgxSetupCursor(pScreen)) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
		       "Hardware cursor initialization failed\n");
	    return(FALSE);
	}
    }

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen))
	return FALSE;

    if(pScrn->depth == 8 && !xf86SbusHandleColormaps(pScreen, pMgx->psdp))
	return FALSE;

    pMgx->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = MgxCloseScreen;
    pScreen->SaveScreen = MgxSaveScreen;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    /* unblank the screen */
    MgxSaveScreen(pScreen, SCREEN_SAVER_OFF);

    /* Done */
    return TRUE;
}


/* Usually mandatory */
static Bool
MgxSwitchMode(SWITCH_MODE_ARGS_DECL)
{
    return TRUE;
}


/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
static void 
MgxAdjustFrame(ADJUST_FRAME_ARGS_DECL)
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
MgxEnterVT(VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);

    if (pMgx->HWCursor) {
	xf86SbusHideOsHwCursor (pMgx->psdp);
    }
    return TRUE;
}


/*
 * This is called when VT switching away from the X server.
 */

/* Mandatory */
static void
MgxLeaveVT(VT_FUNC_ARGS_DECL)
{
    return;
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 */

/* Mandatory */
static Bool
MgxCloseScreen(CLOSE_SCREEN_ARGS_DECL)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);

    pScrn->vtSema = FALSE;
    xf86UnmapSbusMem(pMgx->psdp, pMgx->fb, pMgx->vramsize);
    if (pMgx->blt)
	xf86UnmapSbusMem(pMgx->psdp, pMgx->blt, 0x1000);
    
    if (pMgx->HWCursor)
	xf86SbusHideOsHwCursor (pMgx->psdp);

    pScreen->CloseScreen = pMgx->CloseScreen;
    return (*pScreen->CloseScreen)(CLOSE_SCREEN_ARGS);
}


/* Free up any per-generation data structures */

/* Optional */
static void
MgxFreeScreen(FREE_SCREEN_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    MgxFreeRec(pScrn);
}


/* Checks if a mode is suitable for the selected chipset. */

/* Optional */
static ModeStatus
MgxValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode, Bool verbose, int flags)
{
    if (mode->Flags & V_INTERLACE)
	return(MODE_BAD);

    return(MODE_OK);
}

/* Do screen blanking */

/* Mandatory */
static Bool
MgxSaveScreen(ScreenPtr pScreen, int mode)
    /* this function should blank the screen when unblank is FALSE and
       unblank it when unblank is TRUE -- it doesn't actually seem to be
       used for much though */
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    MgxPtr pMgx = GET_MGX_FROM_SCRN(pScrn);
    int fd = pMgx->psdp->fd, state;
    
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

/*
 * This is the implementation of the Sync() function.
 */
void
MgxSync(ScrnInfoPtr pScrn)
{
    return;
}

static Bool
MgxDriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
    pointer ptr)
{
	xorgHWFlags *flag;
	
	switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
		flag = (CARD32*)ptr;
		(*flag) = HW_MMIO;
		return TRUE;
	default:
		return FALSE;
	}
}
