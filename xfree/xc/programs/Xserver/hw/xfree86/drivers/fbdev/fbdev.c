/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/fbdev/fbdev.c,v 1.27 2000/12/06 15:35:18 eich Exp $ */

/*
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *	     Michel Dänzer, <michdaen@iiic.ethz.ch>
 */

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "mipointer.h"
#include "mibstore.h"
#include "micmap.h"
#include "colormapst.h"
#include "xf86cmap.h"
#include "shadowfb.h"

/* for visuals */
#include "fb.h"
#include "cfb24_32.h"
#ifdef USE_AFB
#include "afb.h"
#endif

#include "xf86Resources.h"
#include "xf86RAC.h"

#include "fbdevhw.h"

#ifdef XvExtension
#include "xf86xv.h"
#endif

#define DEBUG 0

#if DEBUG
# define TRACE_ENTER(str)       ErrorF("fbdev: " str " %d\n",pScrn->scrnIndex)
# define TRACE_EXIT(str)        ErrorF("fbdev: " str " done\n")
# define TRACE(str)             ErrorF("fbdev trace: " str "\n")
#else
# define TRACE_ENTER(str)
# define TRACE_EXIT(str)
# define TRACE(str)
#endif

/* -------------------------------------------------------------------- */
/* prototypes                                                           */

static OptionInfoPtr FBDevAvailableOptions(int chipid, int busid);
static void	FBDevIdentify(int flags);
static Bool	FBDevProbe(DriverPtr drv, int flags);
static Bool	FBDevPreInit(ScrnInfoPtr pScrn, int flags);
static Bool	FBDevScreenInit(int Index, ScreenPtr pScreen, int argc,
				char **argv);
static Bool	FBDevCloseScreen(int scrnIndex, ScreenPtr pScreen);
static Bool	FBDevSaveScreen(ScreenPtr pScreen, int mode);
static void	FBDevDPMSSet(ScrnInfoPtr pScrn, int mode, int flags);

/* -------------------------------------------------------------------- */

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;

#define VERSION			4000
#define FBDEV_NAME		"FBDev"
#define FBDEV_DRIVER_NAME	"fbdev"
#define FBDEV_MAJOR_VERSION	0
#define FBDEV_MINOR_VERSION	1

DriverRec FBDEV = {
	VERSION,
	FBDEV_DRIVER_NAME,
#if 0
	"driver for linux framebuffer devices",
#endif
	FBDevIdentify,
	FBDevProbe,
	FBDevAvailableOptions,
	NULL,
	0
};

/* Supported "chipsets" */
static SymTabRec FBDevChipsets[] = {
    { 0, "fbdev" },
#ifdef USE_AFB
    { 0, "afb" },
#endif
#if 0
    { 0, "cfb8" },
    { 0, "cfb16" },
    { 0, "cfb24" },
    { 0, "cfb32" },
#endif
    {-1, NULL }
};

/* Supported options */
typedef enum {
	OPTION_SHADOW_FB,
	OPTION_FBDEV
} FBDevOpts;

static OptionInfoRec FBDevOptions[] = {
	{ OPTION_SHADOW_FB,	"ShadowFB",	OPTV_BOOLEAN,	{0},	FALSE },
	{ OPTION_FBDEV,		"fbdev",	OPTV_STRING,	{0},	FALSE },
	{ -1,			NULL,		OPTV_NONE,	{0},	FALSE }
};

/* -------------------------------------------------------------------- */

static const char *afbSymbols[] = {
	"afbScreenInit",
	"afbCreateDefColormap",
	NULL
};

static const char *cfbSymbols[] = {
	"fbScreenInit",
	"cfb24_32ScreenInit",
	NULL
};

static const char *shadowSymbols[] = {
	"ShadowFBInit",
	NULL
};

static const char *fbdevHWSymbols[] = {
	"fbdevHWProbe",
	"fbdevHWInit",
	"fbdevHWSetVideoModes",
	"fbdevHWUseBuildinMode",

	"fbdevHWGetName",
	"fbdevHWGetDepth",
	"fbdevHWGetLineLength",
	"fbdevHWGetVidmem",

	/* colormap */
	"fbdevHWLoadpalette",

	/* ScrnInfo hooks */
	"fbdevHWSwitchMode",
	"fbdevHWAdjustFrame",
	"fbdevHWEnterVT",
	"fbdevHWLeaveVT",
	"fbdevHWValidMode",
	NULL
};

#ifdef XFree86LOADER

MODULESETUPPROTO(FBDevSetup);

static XF86ModuleVersionInfo FBDevVersRec =
{
	"fbdev",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	FBDEV_MAJOR_VERSION, FBDEV_MINOR_VERSION, 0,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	NULL,
	{0,0,0,0}
};

XF86ModuleData fbdevModuleData = { &FBDevVersRec, FBDevSetup, NULL };

pointer
FBDevSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;

	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&FBDEV, module, 0);
		LoaderRefSymLists(afbSymbols, cfbSymbols, shadowSymbols, NULL);
		return (pointer)1;
	} else {
		if (errmaj) *errmaj = LDR_ONCEONLY;
		return NULL;
	}
}

#endif /* XFree86LOADER */

/* -------------------------------------------------------------------- */
/* our private data, and two functions to allocate/free this            */

typedef struct {
	unsigned char*			fbstart;
	unsigned char*			fbmem;
	int				fboff;
	unsigned char*			shadowmem;
	int				shadowPitch;
	Bool				shadowFB;
	CloseScreenProcPtr		CloseScreen;
	EntityInfoPtr			pEnt;
} FBDevRec, *FBDevPtr;

#define FBDEVPTR(p) ((FBDevPtr)((p)->driverPrivate))

static Bool
FBDevGetRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate != NULL)
		return TRUE;
	
	pScrn->driverPrivate = xnfcalloc(sizeof(FBDevRec), 1);
	return TRUE;
}

static void
FBDevFreeRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate == NULL)
		return;
	xfree(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

/* -------------------------------------------------------------------- */

static OptionInfoPtr
FBDevAvailableOptions(int chipid, int busid)
{
	return FBDevOptions;
}

static void
FBDevIdentify(int flags)
{
	xf86PrintChipsets(FBDEV_NAME, "driver for framebuffer", FBDevChipsets);
}

static Bool
FBDevProbe(DriverPtr drv, int flags)
{
	int i;
	ScrnInfoPtr pScrn;
       	GDevPtr *devSections;
	int numDevSections;
	int bus,device,func;
	char *dev;
	Bool foundScreen = FALSE;

	TRACE("probe start");

	/* For now, just bail out for PROBE_DETECT. */
	if (flags & PROBE_DETECT)
		return FALSE;

	if ((numDevSections = xf86MatchDevice(FBDEV_DRIVER_NAME, &devSections)) <= 0) 
	    return FALSE;
	
	if (!xf86LoadDrvSubModule(drv, "fbdevhw"))
	    return FALSE;
	    
	xf86LoaderReqSymLists(fbdevHWSymbols, NULL);
	
	for (i = 0; i < numDevSections; i++) {
	    Bool isIsa = FALSE;
	    Bool isPci = FALSE;

	    dev = xf86FindOptionValue(devSections[i]->options,"fbdev");
	    if (devSections[i]->busID) {
	        if (xf86ParsePciBusString(devSections[i]->busID,&bus,&device,
					  &func)) {
		    if (!xf86CheckPciSlot(bus,device,func))
		        continue;
		    isPci = TRUE;
		} else if (xf86ParseIsaBusString(devSections[i]->busID))
		    isIsa = TRUE;
		  
	    }
	    if (fbdevHWProbe(NULL,dev,NULL)) {
		pScrn = NULL;
		if (isPci) {
		    /* XXX what about when there's no busID set? */
		    int entity;
		    
		    entity = xf86ClaimPciSlot(bus,device,func,drv,
					      0,devSections[i],
					      TRUE);
		    pScrn = xf86ConfigPciEntity(pScrn,0,entity,
						      NULL,RES_SHARED_VGA,
						      NULL,NULL,NULL,NULL);
		    /* xf86DrvMsg() can't be called without setting these */
		    pScrn->driverName    = FBDEV_DRIVER_NAME;
		    pScrn->name          = FBDEV_NAME;
		    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
			       "claimed PCI slot %d:%d:%d\n",bus,device,func);

		} else if (isIsa) {
		    int entity;
		    
		    entity = xf86ClaimIsaSlot(drv, 0,
					      devSections[i], TRUE);
		    pScrn = xf86ConfigIsaEntity(pScrn,0,entity,
						      NULL,RES_SHARED_VGA,
						      NULL,NULL,NULL,NULL);
		} else {
		   int entity;

		    entity = xf86ClaimFbSlot(drv, 0,
					      devSections[i], TRUE);
		    pScrn = xf86ConfigFbEntity(pScrn,0,entity,
					       NULL,NULL,NULL,NULL);
		   
		}
		if (pScrn) {
		    foundScreen = TRUE;
		    
		    pScrn->driverVersion = VERSION;
		    pScrn->driverName    = FBDEV_DRIVER_NAME;
		    pScrn->name          = FBDEV_NAME;
		    pScrn->Probe         = FBDevProbe;
		    pScrn->PreInit       = FBDevPreInit;
		    pScrn->ScreenInit    = FBDevScreenInit;
		    pScrn->SwitchMode    = fbdevHWSwitchMode;
		    pScrn->AdjustFrame   = fbdevHWAdjustFrame;
		    pScrn->EnterVT       = fbdevHWEnterVT;
		    pScrn->LeaveVT       = fbdevHWLeaveVT;
		    pScrn->ValidMode     = fbdevHWValidMode;
		    
		    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			       "using %s\n", dev ? dev : "default device");
		}
	    }
	}
	xfree(devSections);
	TRACE("probe done");
	return foundScreen;
}

static Bool
FBDevPreInit(ScrnInfoPtr pScrn, int flags)
{
	FBDevPtr fPtr;
	int default_depth;
	char *mod = NULL;
	const char *reqSym = NULL;
	Gamma zeros = {0.0, 0.0, 0.0};

	if (flags & PROBE_DETECT) return FALSE;

	TRACE_ENTER("PreInit");

	/* Check the number of entities, and fail if it isn't one. */
	if (pScrn->numEntities != 1)
		return FALSE;

	pScrn->monitor = pScrn->confScreen->monitor;

	FBDevGetRec(pScrn);
	fPtr = FBDEVPTR(pScrn);

	fPtr->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

	pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
	/* XXX Is this right?  Can probably remove RAC_FB */
	pScrn->racIoFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;

	if (fPtr->pEnt->location.type == BUS_PCI &&
	    xf86RegisterResources(fPtr->pEnt->index,NULL,ResExclusive)) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "xf86RegisterResources() found resource conflicts\n");
		return FALSE;
	}

	/* open device */
	if (!fbdevHWInit(pScrn,NULL,xf86FindOptionValue(fPtr->pEnt->device->options,"fbdev")))
		return FALSE;
	default_depth = fbdevHWGetDepth(pScrn);
	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth, default_depth,
			     Support24bppFb | Support32bppFb))
		return FALSE;
	xf86PrintDepthBpp(pScrn);

	/* Get the depth24 pixmap format */
	if (pScrn->depth == 24 && pix24bpp == 0)
		pix24bpp = xf86GetBppFromDepth(pScrn, 24);

	/* color weight */
	if (pScrn->depth > 8) {
		rgb zeros = { 0, 0, 0 };
		if (!xf86SetWeight(pScrn, zeros, zeros))
			return FALSE;
	}

	/* visual init */
	if (!xf86SetDefaultVisual(pScrn, -1))
		return FALSE;

	/* We don't currently support DirectColor at > 8bpp */
	if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Given default visual"
			   " (%s) is not supported at depth %d\n",
			   xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
		return FALSE;
	}

	xf86SetGamma(pScrn,zeros);

	pScrn->progClock = TRUE;
	pScrn->rgbBits   = 8;
	pScrn->chipset   = "fbdev";
	pScrn->videoRam  = fbdevHWGetVidmem(pScrn);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Hardware: %s (vidmem: %dk)\n",
		   fbdevHWGetName(pScrn),pScrn->videoRam/1024);

	/* handle options */
	xf86CollectOptions(pScrn, NULL);
	xf86ProcessOptions(pScrn->scrnIndex, fPtr->pEnt->device->options, FBDevOptions);
	fPtr->shadowFB = xf86ReturnOptValBool(FBDevOptions, OPTION_SHADOW_FB, TRUE);
	xf86DrvMsg(pScrn->scrnIndex,
		   xf86IsOptionSet(FBDevOptions, OPTION_SHADOW_FB) ? X_CONFIG : X_DEFAULT,
		   "Option ShadowFB is %s\n",fPtr->shadowFB ? "on" : "off");

	/* select video modes */

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Checking Modes against framebuffer device...\n");
	fbdevHWSetVideoModes(pScrn);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Checking Modes against monitor...\n");
	{
		DisplayModePtr mode, first = mode = pScrn->modes;
		
		if (mode != NULL) do {
			mode->status = xf86CheckModeForMonitor(mode, pScrn->monitor);
			mode = mode->next;
		} while (mode != NULL && mode != first);

		xf86PruneDriverModes(pScrn);
	}

	if (NULL == pScrn->modes)
		fbdevHWUseBuildinMode(pScrn);
	pScrn->currentMode = pScrn->modes;

	if (fPtr->shadowFB)
		pScrn->displayWidth = pScrn->virtualX;	/* ShadowFB handles this correctly */
	else
		/* FIXME: this doesn't work for all cases, e.g. when each scanline
			has a padding which is independent from the depth (controlfb) */
		pScrn->displayWidth = fbdevHWGetLineLength(pScrn)/(fbdevHWGetDepth(pScrn) >> 3);

	xf86PrintModes(pScrn);

	/* Set display resolution */
	xf86SetDpi(pScrn, 0, 0);

	/* Load bpp-specific modules */
	switch (fbdevHWGetType(pScrn))
	{
	case FBDEVHW_PLANES:
		mod = "afb";
		reqSym = "afbScreenInit";
		break;
	case FBDEVHW_PACKED_PIXELS:
		mod = "fb";
		reqSym = "fbScreenInit";

		switch (pScrn->bitsPerPixel)
		{
		case 8:
		case 16:
		case 32:
			break;
		case 24:
			if (pix24bpp == 32)
			{
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
	case FBDEVHW_INTERLEAVED_PLANES:
               /* Not supported yet, don't know what to do with this */
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "Interleaved Planes are not supported yet by drivers/fbdev.");
		return FALSE;
	case FBDEVHW_TEXT:
               /* This should never happen ...
                * we should check for this much much earlier ... */
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "Text mode is not supprted by drivers/fbdev.\n"
               "Why do you want to run the X in TEXT mode anyway ?");
		return FALSE;
       case FBDEVHW_VGA_PLANES:
               /* Not supported yet */
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "EGA/VGA Planes are not supprted yet by drivers/fbdev.");
               return FALSE;
       default:
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
               "Fbdev type (%d) not supported yet.");
               return FALSE;
	}
	if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
		FBDevFreeRec(pScrn);
		return FALSE;
	}
	xf86LoaderReqSymbols(reqSym, NULL);

	/* Load shadowFB if needed */
	if (fPtr->shadowFB) {
		if (!xf86LoadSubModule(pScrn, "shadowfb")) {
			FBDevFreeRec(pScrn);
			return FALSE;
		}
		xf86LoaderReqSymbols("ShadowFBInit", NULL);
	}

	TRACE_EXIT("PreInit");
	return TRUE;
}

/* for ShadowFB */
static void
FBDevRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	FBDevPtr fPtr = FBDEVPTR(pScrn);
	int width, height, Bpp, FBPitch;
	unsigned char *src, *dst;

	Bpp = pScrn->bitsPerPixel >> 3;
	FBPitch = fbdevHWGetLineLength(pScrn);

	while(num--) {
		width = (pbox->x2 - pbox->x1) * Bpp;
		height = pbox->y2 - pbox->y1;
		src = fPtr->shadowmem + (pbox->y1 * fPtr->shadowPitch) +
			(pbox->x1 * Bpp);
		dst = fPtr->fbmem + fPtr->fboff + (pbox->y1 * FBPitch) + (pbox->x1 * Bpp);

		while(height--) {
			memcpy(dst, src, width);
			dst += FBPitch;
			src += fPtr->shadowPitch;
		}
		pbox++;
	}
}

static Bool
FBDevSaveScreen(ScreenPtr pScreen, int mode)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	FBDevPtr fPtr = FBDEVPTR(pScrn);
	BoxRec box;
	Bool unblank;

	TRACE_ENTER("FBDevSaveScreen");
	if (!(fPtr->shadowFB))
		/* Not implemented yet - alloc huge memory block and copy ? */
		return TRUE;

	unblank = xf86IsUnblank(mode);

	if (unblank) {
		box.x1 = 0;
		box.x2 = pScrn->virtualX;
		box.y1 = 0;
		box.y2 = pScrn->virtualY;
		FBDevRefreshArea(pScrn, 1, &box);
	} else {
		memset(fPtr->fbmem + fPtr->fboff, 0,
		       pScrn->virtualX * pScrn->virtualY * ((pScrn->bitsPerPixel+7)/8));
	}
	return TRUE;
}

static Bool
FBDevScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	FBDevPtr fPtr = FBDEVPTR(pScrn);
	VisualPtr visual;
	int ret,flags;

	TRACE_ENTER("FBDevScreenInit");
#if DEBUG
	ErrorF("\tbitsPerPixel=%d, depth=%d, defaultVisual=%s\n"
	       "\tmask: %x,%x,%x, offset: %d,%d,%d\n",
	       pScrn->bitsPerPixel,
	       pScrn->depth,
	       xf86GetVisualName(pScrn->defaultVisual),
	       pScrn->mask.red,pScrn->mask.green,pScrn->mask.blue,
	       pScrn->offset.red,pScrn->offset.green,pScrn->offset.blue);
#endif

	if (NULL == (fPtr->fbmem = fbdevHWMapVidmem(pScrn)))
		return FALSE;
	fPtr->fboff = fbdevHWLinearOffset(pScrn);

	fbdevHWSave(pScrn);

	if (!fbdevHWModeInit(pScrn, pScrn->currentMode))
		return FALSE;
	fbdevHWAdjustFrame(scrnIndex,0,0,0);

	/* mi layer */
	miClearVisualTypes();
	if (pScrn->bitsPerPixel > 8) {
		if (!miSetVisualTypes(pScrn->depth, TrueColorMask, pScrn->rgbBits, TrueColor))
			return FALSE;
	} else {
		if (!miSetVisualTypes(pScrn->depth,
				      miGetDefaultVisualMask(pScrn->depth),
				      pScrn->rgbBits, pScrn->defaultVisual))
			return FALSE;
	}

	/* shadowfb */
	if (fPtr->shadowFB) {
		fPtr->shadowPitch =
			((pScrn->virtualX * pScrn->bitsPerPixel >> 3) + 3) & ~3L;
		fPtr->shadowmem = xalloc(fPtr->shadowPitch * pScrn->virtualY);
		fPtr->fbstart   = fPtr->shadowmem;
	} else {
		fPtr->shadowmem = NULL;
		fPtr->fbstart   = fPtr->fbmem + fPtr->fboff;
	}

	switch (fbdevHWGetType(pScrn))
	{
#ifdef USE_AFB
	case FBDEVHW_PLANES:
		ret = afbScreenInit
			(pScreen, fPtr->fbstart, pScrn->virtualX, pScrn->virtualY,
			 pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth);
		break;
#endif
	case FBDEVHW_PACKED_PIXELS:
		switch (pScrn->bitsPerPixel) {
		case 24:
			if (pix24bpp == 32)
			{
				ret = cfb24_32ScreenInit
					(pScreen, fPtr->fbstart, pScrn->virtualX, pScrn->virtualY,
				 	pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth);
				break;
			}
		case 8:
		case 16:
		case 32:
			ret = fbScreenInit
				(pScreen, fPtr->fbstart, pScrn->virtualX, pScrn->virtualY,
				 pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth, pScrn->bitsPerPixel);
			break;
	 	default:
			xf86DrvMsg(scrnIndex, X_ERROR,
				   "Internal error: invalid bpp (%d) in FBDevScreenInit\n",
				   pScrn->bitsPerPixel);
			ret = FALSE;
			break;
		}
		break;
	case FBDEVHW_INTERLEAVED_PLANES:
		/* This should never happen ...
		* we should check for this much much earlier ... */
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: Text mode is not supprted by drivers/fbdev.\n"
		"Comment: Why do you want to run the X in TEXT mode anyway ?");
		ret = FALSE;
		break;
	case FBDEVHW_TEXT:
		/* This should never happen ...
		* we should check for this much much earlier ... */
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: Text mode is not supprted by drivers/fbdev.\n"
		"Comment: Why do you want to run the X in TEXT mode anyway ?");
		ret = FALSE;
		break;
	case FBDEVHW_VGA_PLANES:
		/* Not supported yet */
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: EGA/VGA Planes are not supprted"
		" yet by drivers/fbdev.");
		ret = FALSE;
		break;
	default:
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: fbdev type (%d) unsupported in"
		" FBDevScreenInit\n");
		ret = FALSE;
		break;
	}
	if (!ret)
		return FALSE;

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

	xf86SetBlackWhitePixels(pScreen);
	miInitializeBackingStore(pScreen);
	xf86SetBackingStore(pScreen);

	/* software cursor */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	if(fPtr->shadowFB)
		ShadowFBInit(pScreen, FBDevRefreshArea);

	/* colormap */
	switch (fbdevHWGetType(pScrn))
	{
	/* XXX It would be simpler to use miCreateDefColormap() in all cases. */
#ifdef USE_AFB
	case FBDEVHW_PLANES:
		if (!afbCreateDefColormap(pScreen))
			return FALSE;
		break;
#endif
	case FBDEVHW_PACKED_PIXELS:
		if (!miCreateDefColormap(pScreen))
			return FALSE;
		break;
	case FBDEVHW_INTERLEAVED_PLANES:
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: invalid fbdev type (interleaved planes)"
		" in FBDevScreenInit\n");
		return FALSE;
	case FBDEVHW_TEXT:
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: invalid fbdev type (text)"
		" in FBDevScreenInit\n");
		return FALSE;
	case FBDEVHW_VGA_PLANES:
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: invalid fbdev type (ega/vga planes)"
		" in FBDevScreenInit\n");
		return FALSE;
	default:
		xf86DrvMsg(scrnIndex, X_ERROR,
		"Internal error: invalid fbdev type (%d) in FBDevScreenInit\n");
		return FALSE;
	}
	flags = CMAP_PALETTED_TRUECOLOR;
	if(!xf86HandleColormaps(pScreen, 256, 8, fbdevHWLoadPalette, NULL, flags))
		return FALSE;

#ifdef DPMSExtension
	xf86DPMSInit(pScreen, FBDevDPMSSet, 0);
#endif

	pScreen->SaveScreen = FBDevSaveScreen;

	/* Wrap the current CloseScreen function */
	fPtr->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = FBDevCloseScreen;

#ifdef XvExtension
	{
	    XF86VideoAdaptorPtr *ptr;

	    int n = xf86XVListGenericAdaptors(pScrn,&ptr);
	    if (n) {
		xf86XVScreenInit(pScreen,ptr,n);
	    }
	}
#endif

#if DEBUG
	ErrorF("FBDevScreenInit done\n",pScrn->scrnIndex);
#endif
	return TRUE;
}

static Bool
FBDevCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	FBDevPtr fPtr = FBDEVPTR(pScrn);
	
	fbdevHWRestore(pScrn);
	fbdevHWUnmapVidmem(pScrn);
	if (fPtr->shadowmem)
		xfree(fPtr->shadowmem);
	pScrn->vtSema = FALSE;

	pScreen->CloseScreen = fPtr->CloseScreen;
	return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}


#ifdef DPMSExtension
static void
FBDevDPMSSet(ScrnInfoPtr pScrn, int mode, int flags)
{
	fbdevHWDPMSSet(pScrn, mode, flags);
}
#endif
