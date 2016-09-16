/*
 * CG14 framebuffer driver.
 *
 * Copyright (C) 2000 Jakub Jelinek (jakub@redhat.com)
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/ioctl.h>
#include <string.h>
#include <sys/ioctl.h>

#include "xf86.h"
#include "xf86_OSproc.h"
#include "mipointer.h"
#include "micmap.h"

#include "fb.h"
#include "xf86cmap.h"
#include "shadow.h"
#include "cg14.h"

#if 0
#define static
#endif

#include "compat-api.h"

static const OptionInfoRec * CG14AvailableOptions(int chipid, int busid);
static void	CG14Identify(int flags);
static Bool	CG14Probe(DriverPtr drv, int flags);
static Bool	CG14PreInit(ScrnInfoPtr pScrn, int flags);
static Bool	CG14ScreenInit(SCREEN_INIT_ARGS_DECL);
static Bool	CG14EnterVT(VT_FUNC_ARGS_DECL);
static void	CG14LeaveVT(VT_FUNC_ARGS_DECL);
static Bool	CG14CloseScreen(CLOSE_SCREEN_ARGS_DECL);
static Bool	CG14SaveScreen(ScreenPtr pScreen, int mode);
static void	CG14InitCplane24(ScrnInfoPtr pScrn);
static void	CG14ExitCplane24(ScrnInfoPtr pScrn);
static void    *CG14WindowLinear(ScreenPtr, CARD32, CARD32, int, CARD32 *,
			      void *);
static Bool	CG14DriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
				pointer ptr);

/* Required if the driver supports mode switching */
static Bool	CG14SwitchMode(SWITCH_MODE_ARGS_DECL);
/* Required if the driver supports moving the viewport */
static void	CG14AdjustFrame(ADJUST_FRAME_ARGS_DECL);

/* Optional functions */
static void	CG14FreeScreen(FREE_SCREEN_ARGS_DECL);
static ModeStatus CG14ValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode,
				Bool verbose, int flags);

void CG14Sync(ScrnInfoPtr pScrn);

#define CG14_VERSION 4000
#define CG14_NAME "SUNCG14"
#define CG14_DRIVER_NAME "suncg14"
#define CG14_MAJOR_VERSION PACKAGE_VERSION_MAJOR
#define CG14_MINOR_VERSION PACKAGE_VERSION_MINOR
#define CG14_PATCHLEVEL PACKAGE_VERSION_PATCHLEVEL

/* 
 * This contains the functions needed by the server after loading the driver
 * module.  It must be supplied, and gets passed back by the SetupProc
 * function in the dynamic case.  In the static case, a reference to this
 * is compiled in, and this requires that the name of this DriverRec be
 * an upper-case version of the driver name.
 */

_X_EXPORT DriverRec SUNCG14 = {
    CG14_VERSION,
    CG14_DRIVER_NAME,
    CG14Identify,
    CG14Probe,
    CG14AvailableOptions,
    NULL,
    0,
    CG14DriverFunc
};

typedef enum {
	OPTION_SHADOW_FB,
	OPTION_HW_CURSOR,
	OPTION_SW_CURSOR,
	OPTION_ACCEL,
	OPTION_XRENDER
} CG14Opts;

static const OptionInfoRec CG14Options[] = {
    { OPTION_SHADOW_FB,	"ShadowFB", OPTV_BOOLEAN, {0}, TRUE},
    { OPTION_ACCEL, 	"Accel",    OPTV_BOOLEAN, {0}, TRUE},
    { OPTION_XRENDER,	"XRender",  OPTV_BOOLEAN, {0}, FALSE},
    { -1,			NULL,		OPTV_NONE,	{0}, FALSE }
};

static MODULESETUPPROTO(cg14Setup);

static XF86ModuleVersionInfo suncg14VersRec =
{
	"suncg14",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	CG14_MAJOR_VERSION, CG14_MINOR_VERSION, CG14_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0,0,0,0}
};

_X_EXPORT XF86ModuleData suncg14ModuleData = {
	&suncg14VersRec,
	cg14Setup,
	NULL
};

pointer
cg14Setup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	xf86AddDriver(&SUNCG14, module, HaveDriverFuncs);

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
CG14GetRec(ScrnInfoPtr pScrn)
{
    /*
     * Allocate an Cg14Rec, and hook it into pScrn->driverPrivate.
     * pScrn->driverPrivate is initialised to NULL, so we can check if
     * the allocation has already been done.
     */
    if (pScrn->driverPrivate != NULL)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(Cg14Rec), 1);
    return TRUE;
}

static void
CG14FreeRec(ScrnInfoPtr pScrn)
{
    Cg14Ptr pCg14;

    if (pScrn->driverPrivate == NULL)
	return;

    pCg14 = GET_CG14_FROM_SCRN(pScrn);

    free(pScrn->driverPrivate);
    pScrn->driverPrivate = NULL;

    return;
}

static const OptionInfoRec *
CG14AvailableOptions(int chipid, int busid)
{
    return CG14Options;
}

/* Mandatory */
static void
CG14Identify(int flags)
{
    xf86Msg(X_INFO, "%s: driver for CG14\n", CG14_NAME);
}


/* Mandatory */
static Bool
CG14Probe(DriverPtr drv, int flags)
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

    if ((numDevSections = xf86MatchDevice(CG14_DRIVER_NAME,
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

    numUsed = xf86MatchSbusInstances(CG14_NAME, SBUS_DEVICE_CG14,
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
	    pScrn->driverVersion = CG14_VERSION;
	    pScrn->driverName	 = CG14_DRIVER_NAME;
	    pScrn->name		 = CG14_NAME;
	    pScrn->Probe	 = CG14Probe;
	    pScrn->PreInit	 = CG14PreInit;
	    pScrn->ScreenInit	 = CG14ScreenInit;
  	    pScrn->SwitchMode	 = CG14SwitchMode;
  	    pScrn->AdjustFrame	 = CG14AdjustFrame;
	    pScrn->EnterVT	 = CG14EnterVT;
	    pScrn->LeaveVT	 = CG14LeaveVT;
	    pScrn->FreeScreen	 = CG14FreeScreen;
	    pScrn->ValidMode	 = CG14ValidMode;
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
CG14PreInit(ScrnInfoPtr pScrn, int flags)
{
    Cg14Ptr pCg14;
    sbusDevicePtr psdp = NULL;
    int i, from, size, len, reg[6], prom;
    char *ptr;

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

    /* Allocate the Cg14Rec driverPrivate */
    if (!CG14GetRec(pScrn)) {
	return FALSE;
    }
    pCg14 = GET_CG14_FROM_SCRN(pScrn);
    
    /* Set pScrn->monitor */
    pScrn->monitor = pScrn->confScreen->monitor;

    /* This driver doesn't expect more than one entity per screen */
    if (pScrn->numEntities > 1)
	return FALSE;
    /* This is the general case */
    for (i = 0; i < pScrn->numEntities; i++) {
	EntityInfoPtr pEnt = xf86GetEntityInfo(pScrn->entityList[i]);

	/* CG14 is purely AFX, but we handle it like SBUS */
	if (pEnt->location.type == BUS_SBUS) {
	    psdp = xf86GetSbusInfoForEntity(pEnt->index);
	    pCg14->psdp = psdp;
	} else
	    return FALSE;
    }
    if (psdp == NULL)
	return FALSE;

    pCg14->memsize = 4 * 1024 * 1024;	/* always safe */
    if ((psdp->height * psdp->width * 4) > 0x00400000)
    	 pCg14->memsize = 0x00800000;
    len = 24;
    prom = sparcPromInit();
    if (ptr = sparcPromGetProperty(&psdp->node, "reg", &len)) {
    	if (len >= 24) {
    	    memcpy(reg, ptr, 24);
    	    size = reg[5];
    	    xf86Msg(X_DEBUG, "memsize from reg: %d MB\n", size >> 20);
	    if (size > pCg14->memsize)
    		pCg14->memsize = size;
    	}
    }
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "found %d MB video memory\n",
      pCg14->memsize >> 20);
    /*********************
    deal with depth
    *********************/
    
    if (!xf86SetDepthBpp(pScrn, 0, 0, 0, Support24bppFb|Support32bppFb))
		return FALSE;
    /* Check that the returned depth is one we support */
    switch (pScrn->depth) {
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

    /* Collect all of the relevant option flags (fill in pScrn->options) */
    xf86CollectOptions(pScrn, NULL);
    /* Process the options */
    if (!(pCg14->Options = malloc(sizeof(CG14Options))))
	return FALSE;
    memcpy(pCg14->Options, CG14Options, sizeof(CG14Options));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pCg14->Options);
    pCg14->use_shadow = xf86ReturnOptValBool(pCg14->Options, OPTION_SHADOW_FB,
        TRUE);
    pCg14->use_accel = xf86ReturnOptValBool(pCg14->Options, OPTION_ACCEL,
        TRUE);
    pCg14->use_xrender = xf86ReturnOptValBool(pCg14->Options, OPTION_XRENDER,
        FALSE);

    /*
     * This must happen after pScrn->display has been set because
     * xf86SetWeight references it.
     */
    if (pScrn->depth > 8) {
	rgb weight = {0, 0, 0};
	rgb mask = {0xff, 0xff00, 0xff0000};
                                       
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

    if (xf86LoadSubModule(pScrn, "fb") == NULL) {
	CG14FreeRec(pScrn);
	return FALSE;
    }

    if (pCg14->use_shadow) {
	xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Using shadow framebuffer\n");
	if (xf86LoadSubModule(pScrn, "shadow") == NULL) {
	    CG14FreeRec(pScrn);
	    return FALSE;
	}
    }

    from = X_DEFAULT;
    pCg14->HWCursor = TRUE;
    if (xf86GetOptValBool(pCg14->Options, OPTION_HW_CURSOR, &pCg14->HWCursor))
	from = X_CONFIG;
    if (xf86ReturnOptValBool(pCg14->Options, OPTION_SW_CURSOR, FALSE)) {
	from = X_CONFIG;
	pCg14->HWCursor = FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pCg14->HWCursor ? "HW" : "SW");

    /*********************
    set up clock and mode stuff
    *********************/
    
    pScrn->progClock = TRUE;

    if(pScrn->display->virtualX || pScrn->display->virtualY) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "CG14 does not support a virtual desktop\n");
	pScrn->display->virtualX = 0;
	pScrn->display->virtualY = 0;
    }

    xf86SbusUseBuiltinMode(pScrn, pCg14->psdp);
    pScrn->currentMode = pScrn->modes;
    pScrn->displayWidth = pScrn->virtualX;

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    return TRUE;
}

static Bool
CG14CreateScreenResources(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);
    PixmapPtr pPixmap;
    Bool ret;

    pScreen->CreateScreenResources = pCg14->CreateScreenResources;
    ret = pScreen->CreateScreenResources(pScreen);
    pScreen->CreateScreenResources = CG14CreateScreenResources;

    if (!ret)
	return FALSE;

    pPixmap = pScreen->GetScreenPixmap(pScreen);

    if (!shadowAdd(pScreen, pPixmap, shadowUpdatePackedWeak(),
	CG14WindowLinear, 0, NULL)) {
	return FALSE;
    }
    return TRUE;
}


static Bool
CG14ShadowInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);

    if (!shadowSetup(pScreen)) {
	return FALSE;
    }

    pCg14->CreateScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = CG14CreateScreenResources;

    return TRUE;
}
/* Mandatory */

/* This gets called at the start of each server generation */

static Bool
CG14ScreenInit(SCREEN_INIT_ARGS_DECL)
{
    ScrnInfoPtr pScrn;
    Cg14Ptr pCg14;
    VisualPtr visual;
    int ret, have_accel = 0;

    /* 
     * First get the ScrnInfoRec
     */
    pScrn = xf86ScreenToScrn(pScreen);

    pCg14 = GET_CG14_FROM_SCRN(pScrn);

    /* Map the CG14 memory */
    pCg14->fb = xf86MapSbusMem (pCg14->psdp, CG14_DIRECT_VOFF, pCg14->memsize);
    pCg14->x32 = xf86MapSbusMem (pCg14->psdp, CG14_X32_VOFF,
				 (pCg14->psdp->width * pCg14->psdp->height));
    pCg14->xlut = xf86MapSbusMem (pCg14->psdp, CG14_XLUT_VOFF, 4096);
    pCg14->curs = xf86MapSbusMem (pCg14->psdp, CG14_CURSOR_VOFF, 4096);

    pCg14->sxreg = xf86MapSbusMem (pCg14->psdp, CG14_SXREG_VOFF, 4096);
    pCg14->sxio = xf86MapSbusMem (pCg14->psdp, CG14_SXIO_VOFF, 0x04000000);
    have_accel = (pCg14->sxreg != NULL) && (pCg14->sxio != NULL);

    if (have_accel) {
    	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
    	  "found kernel support for SX acceleration\n");
    }
    have_accel = have_accel & pCg14->use_accel;
    if (have_accel) {
    	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "using acceleration\n");
    	if (pCg14->use_shadow)
    	    xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "disabling shadow\n");
    	pCg14->use_shadow = FALSE;
    }
    	
    pCg14->width = pCg14->psdp->width;
    pCg14->height = pCg14->psdp->height;

    if (! pCg14->fb || !pCg14->x32 || !pCg14->xlut || !pCg14->curs) {
    	xf86Msg(X_ERROR,
	    "can't mmap something: fd %08x  x32 %08x xlut %08x cursor %08x\n",
	    (uint32_t)pCg14->fb, (uint32_t)pCg14->x32, (uint32_t)pCg14->xlut,
	    (uint32_t)pCg14->curs);
	return FALSE;
    }

    /* Darken the screen for aesthetic reasons and set the viewport */
    CG14SaveScreen(pScreen, SCREEN_SAVER_ON);

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

    if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
			  pScrn->rgbBits, pScrn->defaultVisual))
	return FALSE;

    miSetPixmapDepths ();

    if (pCg14->use_shadow) {
	pCg14->shadow = malloc(pScrn->virtualX * pScrn->virtualY * 4);
		
	if (!pCg14->shadow) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	        "Failed to allocate shadow framebuffer\n");
	    return FALSE;
	}
    }

    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */

    CG14InitCplane24(pScrn);
    ret = fbScreenInit(pScreen, pCg14->use_shadow ? pCg14->shadow : pCg14->fb,
    		       pScrn->virtualX,
		       pScrn->virtualY, pScrn->xDpi, pScrn->yDpi,
		       pScrn->virtualX, pScrn->bitsPerPixel);

    if (!ret)
	return FALSE;

    /* must be after RGB ordering fixed */
    fbPictureInit (pScreen, 0, 0);

    if (pCg14->use_shadow && !CG14ShadowInit(pScreen)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		    "shadow framebuffer initialization failed\n");
	return FALSE;
    }

    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);

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

    /* setup acceleration */
    if (have_accel) {
	XF86ModReqInfo req;
	int errmaj, errmin;

	memset(&req, 0, sizeof(XF86ModReqInfo));
	req.majorversion = 2;
	req.minorversion = 0;
	if (!LoadSubModule(pScrn->module, "exa", NULL, NULL, NULL, &req,
	    &errmaj, &errmin)) {
		LoaderErrorMsg(NULL, "exa", errmaj, errmin);
		return FALSE;
	}
	if (!CG14InitAccel(pScreen))
	    have_accel = FALSE;
    }


    /* Initialise cursor functions */
    miDCInitialize (pScreen, xf86GetPointerScreenFuncs());

    /* check for hardware cursor support */
    if (pCg14->HWCursor)
	CG14SetupCursor(pScreen);

    /* Initialise default colourmap */
    if (!miCreateDefColormap(pScreen))
	return FALSE;

    pCg14->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = CG14CloseScreen;
    pScreen->SaveScreen = CG14SaveScreen;

    /* Report any unused options (only for the first generation) */
    if (serverGeneration == 1) {
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
    }

    /* unblank the screen */
    CG14SaveScreen(pScreen, SCREEN_SAVER_OFF);

    /* Done */
    return TRUE;
}


/* Usually mandatory */
static Bool
CG14SwitchMode(SWITCH_MODE_ARGS_DECL)
{
    xf86Msg(X_ERROR, "CG14SwitchMode\n");
    return TRUE;
}


/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
static void 
CG14AdjustFrame(ADJUST_FRAME_ARGS_DECL)
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
CG14EnterVT(VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);

    CG14InitCplane24 (pScrn);
    return TRUE;
}


/*
 * This is called when VT switching away from the X server.
 */

/* Mandatory */
static void
CG14LeaveVT(VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);

    CG14ExitCplane24 (pScrn);
    return;
}


/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should really also unmap the video memory too.
 */

/* Mandatory */
static Bool
CG14CloseScreen(CLOSE_SCREEN_ARGS_DECL)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);
    PixmapPtr pPixmap;

    if (pCg14->use_shadow) {

	pPixmap = pScreen->GetScreenPixmap(pScreen);
	shadowRemove(pScreen, pPixmap);
	pCg14->use_shadow = FALSE;
    }

    pScrn->vtSema = FALSE;
    CG14ExitCplane24 (pScrn);
    xf86UnmapSbusMem(pCg14->psdp, pCg14->fb,
		     (pCg14->psdp->width * pCg14->psdp->height * 4));
    xf86UnmapSbusMem(pCg14->psdp, pCg14->x32,
		     (pCg14->psdp->width * pCg14->psdp->height));
    xf86UnmapSbusMem(pCg14->psdp, pCg14->xlut, 4096);
    xf86UnmapSbusMem(pCg14->psdp, pCg14->curs, 4096);
    
    pScreen->CloseScreen = pCg14->CloseScreen;
    return (*pScreen->CloseScreen)(CLOSE_SCREEN_ARGS);
}

static void *
CG14WindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);

    *size = pCg14->width << 2;
    return (CARD8 *)pCg14->fb + row * (pCg14->width << 2) + offset;
}

/* Free up any per-generation data structures */

/* Optional */
static void
CG14FreeScreen(FREE_SCREEN_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    CG14FreeRec(pScrn);
}


/* Checks if a mode is suitable for the selected chipset. */

/* Optional */
static ModeStatus
CG14ValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode, Bool verbose, int flags)
{
    if (mode->Flags & V_INTERLACE)
	return(MODE_BAD);

    return(MODE_OK);
}

/* Do screen blanking */

/* Mandatory */
static Bool
CG14SaveScreen(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);
    int state;
    switch(mode) {
	case SCREEN_SAVER_ON:
	case SCREEN_SAVER_CYCLE:
		state = FBVIDEO_OFF;
		ioctl(pCg14->psdp->fd, FBIOSVIDEO, &state);
		break;
	case SCREEN_SAVER_OFF:
	case SCREEN_SAVER_FORCER:
		state = FBVIDEO_ON;
		ioctl(pCg14->psdp->fd, FBIOSVIDEO, &state);
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
CG14Sync(ScrnInfoPtr pScrn)
{
    return;
}

/*
 * This initializes the card for 24 bit mode.
 */
static void
CG14InitCplane24(ScrnInfoPtr pScrn)
{
  Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);
  int size, bpp;
              
  size = pScrn->virtualX * pScrn->virtualY;
  bpp = 32;
  ioctl (pCg14->psdp->fd, CG14_SET_PIXELMODE, &bpp);
  memset (pCg14->fb, 0, size * 4);
  memset (pCg14->x32, 0, size);
  memset (pCg14->xlut, 0, 0x200);
}                                                  

/*
 * This initializes the card for 8 bit mode.
 */
static void
CG14ExitCplane24(ScrnInfoPtr pScrn)
{
  Cg14Ptr pCg14 = GET_CG14_FROM_SCRN(pScrn);
  int bpp = 8;
              
  ioctl (pCg14->psdp->fd, CG14_SET_PIXELMODE, &bpp);
}


static Bool
CG14DriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
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
