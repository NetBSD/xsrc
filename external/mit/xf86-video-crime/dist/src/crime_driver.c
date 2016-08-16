/* $NetBSD: crime_driver.c,v 1.12 2016/08/16 01:27:46 mrg Exp $ */
/*
 * Copyright (c) 2008 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* a driver for the CRIME rendering engine foundin SGI O2 workstations */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dev/wscons/wsconsio.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"

#include "mipointer.h"
#include "micmap.h"
#include "colormapst.h"
#include "xf86cmap.h"

/* for visuals */
#include "fb.h"

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif

#ifdef XvExtension
#include "xf86xv.h"
#endif

#include "crime.h"

/* #include "wsconsio.h" */

#ifndef XFree86LOADER
#include <sys/mman.h>
#endif

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 6
#define xf86LoaderReqSymLists(...) do {} while (0)
#define LoaderRefSymLists(...) do {} while (0)
#endif

#define CRIME_DEFAULT_DEV "/dev/ttyE0"

/* Prototypes */
#ifdef XFree86LOADER
static pointer CrimeSetup(pointer, pointer, int *, int *);
#endif
static Bool CrimeGetRec(ScrnInfoPtr);
static void CrimeFreeRec(ScrnInfoPtr);
static const OptionInfoRec * CrimeAvailableOptions(int, int);
static void CrimeIdentify(int);
static Bool CrimeProbe(DriverPtr, int);
static Bool CrimePreInit(ScrnInfoPtr, int);
static Bool CrimeScreenInit(int, ScreenPtr, int, char **);
static Bool CrimeCloseScreen(int, ScreenPtr);
static Bool CrimeEnterVT(int, int);
static void CrimeLeaveVT(int, int);
static Bool CrimeSwitchMode(int, DisplayModePtr, int);
static int CrimeValidMode(int, DisplayModePtr, Bool, int);
static void CrimeLoadPalette(ScrnInfoPtr, int, int *, LOCO *, VisualPtr);
static Bool CrimeSaveScreen(ScreenPtr, int);
static void CrimeSave(ScrnInfoPtr);
static void CrimeRestore(ScrnInfoPtr);

/* helper functions */
static int crime_open(char *);
static pointer crime_mmap(size_t, off_t, int, int);

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;

#define VERSION			4000
#define CRIME_NAME		"crime"
#define CRIME_DRIVER_NAME	"crime"
#define CRIME_MAJOR_VERSION	0
#define CRIME_MINOR_VERSION	1

DriverRec CRIME = {
	VERSION,
	CRIME_DRIVER_NAME,
	CrimeIdentify,
	CrimeProbe,
	CrimeAvailableOptions,
	NULL,
	0
};

/* Supported "chipsets" */
static SymTabRec CrimeChipsets[] = {
	{ 0, "crime" },
	{ -1, NULL }
};

/* Supported options */
typedef enum {
	OPTION_HW_CURSOR,
	OPTION_SW_CURSOR
} CrimeOpts;

static const OptionInfoRec CrimeOptions[] = {
	{ OPTION_SW_CURSOR, "SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
	{ OPTION_HW_CURSOR, "HWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
	{ -1, NULL, OPTV_NONE, {0}, FALSE}
};

/* Symbols needed from other modules */
static const char *fbSymbols[] = {
	"fbPictureInit",
	"fbScreenInit",
	NULL
};

static const char *xaaSymbols[] =
{
    "XAACreateInfoRec",
    "XAADestroyInfoRec",
    "XAAInit",
    "XAAScreenIndex",
    NULL
};

static const char *ramdacSymbols[] = {
	"xf86CreateCursorInfoRec",
	"xf86DestroyCursorInfoRec",
	"xf86InitCursor",
	NULL
};

#ifdef XFree86LOADER
static XF86ModuleVersionInfo CrimeVersRec = {
	"crime",
	"The NetBSD Foundation",
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	CRIME_MAJOR_VERSION, CRIME_MINOR_VERSION, 0,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	NULL,
	{0, 0, 0, 0}
};

_X_EXPORT XF86ModuleData crimeModuleData = { &CrimeVersRec, CrimeSetup, NULL };

static pointer
CrimeSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;
	const char *osname;

	/* Check that we're being loaded on a OpenBSD or NetBSD system */
	LoaderGetOS(&osname, NULL, NULL, NULL);
	if (!osname 
	    && (strcmp(osname, "netbsd") != 0)) {
		if (errmaj)
			*errmaj = LDR_BADOS;
		if (errmin)
			*errmin = 0;
		return NULL;
	}
	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&CRIME, module, 0);
		LoaderRefSymLists(xaaSymbols, fbSymbols, NULL);
		return (pointer)1;
	} else {
		if (errmaj != NULL)
			*errmaj = LDR_ONCEONLY;
		return NULL;
	}
}
#endif /* XFree86LOADER */

static Bool
CrimeGetRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate != NULL)
		return TRUE;

	pScrn->driverPrivate = xnfcalloc(sizeof(CrimeRec), 1);
	return TRUE;
}

static void
CrimeFreeRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate == NULL)
		return;
	xfree(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

static const OptionInfoRec *
CrimeAvailableOptions(int chipid, int busid)
{
	return CrimeOptions;
}

static void
CrimeIdentify(int flags)
{
	xf86PrintChipsets(CRIME_NAME, "driver for CRIME framebuffer",
			  CrimeChipsets);
}


#define priv_open_device(n)	open(n,O_RDWR|O_NONBLOCK|O_EXCL)

/* Open the framebuffer device */
static int
crime_open(char *dev)
{
	int fd = -1;

	/* try argument from XF86Config first */
	if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
		/* second: environment variable */
		dev = getenv("XDEVICE");
		if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
			/* last try: default device */
			dev = CRIME_DEFAULT_DEV;
			if ((fd = priv_open_device(dev)) == -1) {
				return -1;
			}
		}
	}
	return fd;
}

/* Map the framebuffer's memory */
static pointer
crime_mmap(size_t len, off_t off, int fd, int ro)
{
	pointer mapaddr;

	/*
	 * try and make it private first, that way once we get it, an
	 * interloper, e.g. another server, can't get this frame buffer,
	 * and if another server already has it, this one won't.
	 */
	if (ro) {
		mapaddr = (pointer) mmap(NULL, len,
					 PROT_READ, MAP_SHARED,
					 fd, off);
		xf86Msg(X_ERROR, "mapping %08x read only\n", off);
	} else {
		mapaddr = (pointer) mmap(NULL, len,
					 PROT_READ | PROT_WRITE, MAP_SHARED,
					 fd, off);
		xf86Msg(X_ERROR, "mapping %08x read/write\n", off);
	}
	if (mapaddr == (pointer) -1) {
		mapaddr = NULL;
	}
#ifdef CRIME_DEBUG
	ErrorF("mmap returns: addr %p len 0x%x\n", mapaddr, len);
#endif
	return mapaddr;
}

static Bool
CrimeProbe(DriverPtr drv, int flags)
{
	ScrnInfoPtr pScrn = NULL;
	int i, fd, entity, wstype;
       	GDevPtr *devSections;
	int numDevSections;
	char *dev;
	Bool foundScreen = FALSE;

	if ((numDevSections = xf86MatchDevice(CRIME_DRIVER_NAME,
					      &devSections)) <= 0)
		return FALSE;


	if ((fd = crime_open(CRIME_DEFAULT_DEV)) == 0)
		return FALSE;

	if (ioctl(fd, WSDISPLAYIO_GTYPE, &wstype) == -1)
		return FALSE;
	if (wstype != WSDISPLAY_TYPE_CRIME)
		return FALSE;

	xf86Msg(X_INFO, "%s: CRIME found\n", __func__);

	if ( xf86DoConfigure && xf86DoConfigurePass1 ) {
		GDevPtr pGDev;

		pGDev = xf86AddBusDeviceToConfigure(CRIME_DRIVER_NAME, BUS_NONE,
			NULL, 0);
		if (pGDev) {
			/*
			 * XF86Match???Instances() treat chipID and chipRev as
			 * overrides, so clobber them here.
			 */
			pGDev->chipID = pGDev->chipRev = -1;
	    	}
	}

	if (flags & PROBE_DETECT) {
		return TRUE;
	}

	if (numDevSections > 1) {
		xf86Msg(X_ERROR, "Ignoring additional device sections\n");
		numDevSections = 1;
	}
	/* ok, at this point we know we've got a CRIME */
	for (i = 0; i < numDevSections; i++) {
	
		entity = xf86ClaimFbSlot(drv, 0, devSections[i], TRUE);
		pScrn = xf86ConfigFbEntity(NULL, 0, entity,
		    NULL, NULL, NULL, NULL);
		if (pScrn != NULL) {
			foundScreen = TRUE;
			pScrn->driverVersion = VERSION;
			pScrn->driverName = CRIME_DRIVER_NAME;
			pScrn->name = CRIME_NAME;
			pScrn->Probe = CrimeProbe;
			pScrn->PreInit = CrimePreInit;
			pScrn->ScreenInit = CrimeScreenInit;
			pScrn->SwitchMode = CrimeSwitchMode;
			pScrn->AdjustFrame = NULL;
			pScrn->EnterVT = CrimeEnterVT;
			pScrn->LeaveVT = CrimeLeaveVT;
			pScrn->ValidMode = CrimeValidMode;

		}
	}
	xfree(devSections);
	return foundScreen;
}

static Bool
CrimePreInit(ScrnInfoPtr pScrn, int flags)
{
	CrimePtr fPtr;
	int default_depth, wstype;
	char *dev;
	char *mod = NULL;
	const char *reqSym = NULL;
	Gamma zeros = {0.0, 0.0, 0.0};
	DisplayModePtr mode;
	MessageType from;
	rgb rgbzeros = { 0, 0, 0 }, masks;

	if (flags & PROBE_DETECT) return FALSE;

	if (pScrn->numEntities != 1) return FALSE;

	pScrn->monitor = pScrn->confScreen->monitor;

	CrimeGetRec(pScrn);
	fPtr = CRIMEPTR(pScrn);

	fPtr->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
	pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
	pScrn->racIoFlags = pScrn->racMemFlags;
#endif

	dev = xf86FindOptionValue(fPtr->pEnt->device->options, "device");
	fPtr->fd = crime_open(dev);
	if (fPtr->fd == -1) {
		return FALSE;
	}

	if (ioctl(fPtr->fd, WSDISPLAYIO_GINFO, &fPtr->info) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAY_GINFO: %s\n",
			   strerror(errno));
		return FALSE;
	}
	if (ioctl(fPtr->fd, WSDISPLAYIO_GTYPE, &wstype) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAY_GTYPE: %s\n",
			   strerror(errno));
		return FALSE;
	}
        
	/* Handle depth */
	default_depth = fPtr->info.depth <= 24 ? fPtr->info.depth : 24;
	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth,
			     fPtr->info.depth, Support24bppFb|Support32bppFb))
		return FALSE;
	xf86PrintDepthBpp(pScrn);

	/* Get the depth24 pixmap format */
	if (pScrn->depth == 24 && pix24bpp == 0)
		pix24bpp = xf86GetBppFromDepth(pScrn, 24);

	/* color weight */
	masks.red =   0x00ff0000;
	masks.green = 0x0000ff00;
	masks.blue =  0x000000ff;
	if (!xf86SetWeight(pScrn, rgbzeros, masks))
		return FALSE;

	/* visual init */
	if (!xf86SetDefaultVisual(pScrn, -1))
		return FALSE;

	/* We don't currently support DirectColor at > 8bpp */
	if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Given default visual"
			   " (%s) is not supported at depth %d\n",
			   xf86GetVisualName(pScrn->defaultVisual),
			   pScrn->depth);
		return FALSE;
	}

	xf86SetGamma(pScrn,zeros);

	pScrn->progClock = TRUE;
	pScrn->rgbBits   = 8;
	pScrn->chipset   = "crime";
	pScrn->videoRam  = fPtr->info.width * 4 * 2048;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vidmem: %dk\n",
		   pScrn->videoRam/1024);

	/* handle options */
	xf86CollectOptions(pScrn, NULL);
	if (!(fPtr->Options = xalloc(sizeof(CrimeOptions))))
		return FALSE;
	memcpy(fPtr->Options, CrimeOptions, sizeof(CrimeOptions));
	xf86ProcessOptions(pScrn->scrnIndex, fPtr->pEnt->device->options,
			   fPtr->Options);
	
	/* fake video mode struct */
	mode = (DisplayModePtr)xalloc(sizeof(DisplayModeRec));
	mode->prev = mode;
	mode->next = mode;
	mode->name = "crime current mode";
	mode->status = MODE_OK;
	mode->type = M_T_BUILTIN;
	mode->Clock = 0;
	mode->HDisplay = fPtr->info.width;
	mode->HSyncStart = 0;
	mode->HSyncEnd = 0;
	mode->HTotal = 0;
	mode->HSkew = 0;
	mode->VDisplay = fPtr->info.height;
	mode->VSyncStart = 0;
	mode->VSyncEnd = 0;
	mode->VTotal = 0;
	mode->VScan = 0;
	mode->Flags = 0;
	if (pScrn->modes != NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Ignoring mode specification from screen section\n");
	}
	pScrn->currentMode = pScrn->modes = mode;
	pScrn->virtualX = fPtr->info.width;
	pScrn->virtualY = fPtr->info.height;
	pScrn->displayWidth = pScrn->virtualX;

	/* Set the display resolution */
	xf86SetDpi(pScrn, 0, 0);

	from = X_DEFAULT;
	fPtr->HWCursor = TRUE;
	if (xf86GetOptValBool(fPtr->Options, OPTION_HW_CURSOR, &fPtr->HWCursor))
		from = X_CONFIG;
	if (xf86ReturnOptValBool(fPtr->Options, OPTION_SW_CURSOR, FALSE)) {
		from = X_CONFIG;
		fPtr->HWCursor = FALSE;
	}
	xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		fPtr->HWCursor ? "HW" : "SW");

	if (xf86LoadSubModule(pScrn, "fb") == NULL) {
		CrimeFreeRec(pScrn);
		return FALSE;
	}
	xf86LoaderReqSymLists(fbSymbols, NULL);

	if (xf86LoadSubModule(pScrn, "xaa") == NULL) {
		CrimeFreeRec(pScrn);
		return FALSE;
	}
	xf86LoaderReqSymLists(xaaSymbols, NULL);

	if (xf86LoadSubModule(pScrn, "ramdac") == NULL) {
		CrimeFreeRec(pScrn);
		return FALSE;
	}
	xf86LoaderReqSymLists(ramdacSymbols, NULL);
	
	return TRUE;
}

static Bool
CrimeScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	CrimePtr fPtr = CRIMEPTR(pScrn);
	VisualPtr visual;
	int ret, flags, width, height, i, j;
	int wsmode = WSDISPLAYIO_MODE_MAPPED;
	size_t len;

#ifdef CRIME_DEBUG
	ErrorF("\tbitsPerPixel=%d, depth=%d, defaultVisual=%s\n"
	       "\tmask: %x,%x,%x, offset: %d,%d,%d\n",
	       pScrn->bitsPerPixel,
	       pScrn->depth,
	       xf86GetVisualName(pScrn->defaultVisual),
	       pScrn->mask.red,pScrn->mask.green,pScrn->mask.blue,
	       pScrn->offset.red,pScrn->offset.green,pScrn->offset.blue);
#endif

	/* Switch to graphics mode - required before mmap */
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &wsmode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAYIO_SMODE: %s\n",
			   strerror(errno));
		return FALSE;
	}
	fPtr->engine = crime_mmap(0x5000, 0x15000000, fPtr->fd, 0);

	if (fPtr->engine == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "crime_mmap engine: %s\n", strerror(errno));
		return FALSE;
	}

	fPtr->linear = crime_mmap(0x10000, 0x15010000, fPtr->fd, 0);
	if (fPtr->linear == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "crime_mmap linear: %s\n", strerror(errno));
		return FALSE;
	}

	memset(fPtr->linear, 0, 0x10000);
#ifdef CRIME_DEBUG
	fPtr->fb = crime_mmap(8192 * fPtr->info.height, 0, fPtr->fd, 1);
	if (fPtr->fb == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "crime_mmap fb: %s\n", strerror(errno));
		return FALSE;
	}
#else
	fPtr->fb = malloc(8192 * fPtr->info.height);
	if (fPtr->fb == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Cannot allocate fake fb: %s\n", strerror(errno));
		return FALSE;
	}
#endif

	CrimeSave(pScrn);
	pScrn->vtSema = TRUE;

	/* mi layer */
	miClearVisualTypes();
	if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
			      pScrn->rgbBits, TrueColor))
		return FALSE;

	if (!miSetPixmapDepths())
		return FALSE;

	height = pScrn->virtualY;
	width = pScrn->virtualX;

	ret = fbScreenInit(pScreen,
			   fPtr->fb,
			   width, height,
			   pScrn->xDpi, pScrn->yDpi,
			   pScrn->displayWidth,
			   pScrn->bitsPerPixel);

	if (!ret)
		return FALSE;

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
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			    "%d %d %d %08x %08x %08x\n",
			    visual->offsetRed, visual->offsetGreen,
			    visual->offsetBlue, visual->redMask,
			    visual->greenMask, visual->blueMask);
		}
	}

	if (!fbPictureInit(pScreen, NULL, 0))
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "RENDER extension initialisation failed.");

	xf86SetBlackWhitePixels(pScreen);
	xf86SetBackingStore(pScreen);

	if (fPtr) {
		BoxRec bx;
		fPtr->pXAA = XAACreateInfoRec();
		CrimeAccelInit(pScrn);
		bx.x1 = bx.y1 = 0;
		bx.x2 = fPtr->info.width;
		bx.y2 = 2048;
		xf86InitFBManager(pScreen, &bx);
		if(!XAAInit(pScreen, fPtr->pXAA))
			return FALSE;
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Using acceleration\n");
	}

	/* software cursor */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	/* check for hardware cursor support */
	CrimeSetupCursor(pScreen);
	
	/* colormap */
	if (!miCreateDefColormap(pScreen))
		return FALSE;
		flags = CMAP_RELOAD_ON_MODE_SWITCH;
	if(!xf86HandleColormaps(pScreen, 256, 8, CrimeLoadPalette,
				NULL, flags))
		return FALSE;

	pScreen->SaveScreen = CrimeSaveScreen;

#ifdef XvExtension
	{
		XF86VideoAdaptorPtr *ptr;

		int n = xf86XVListGenericAdaptors(pScrn,&ptr);
		if (n) {
			xf86XVScreenInit(pScreen,ptr,n);
		}
	}
#endif

	/* Wrap the current CloseScreen function */
	fPtr->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = CrimeCloseScreen;

	return TRUE;
}

static Bool
CrimeCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	CrimePtr fPtr = CRIMEPTR(pScrn);

	if (pScrn->vtSema) {
		CrimeRestore(pScrn);
		if (munmap(fPtr->engine, 0x5000) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap engine: %s\n", strerror(errno));
		}

		if (munmap(fPtr->linear, 0x10000) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap linear: %s\n", strerror(errno));
		}
#ifdef CRIME_DEBUG
		if (munmap(fPtr->fb, 8192 * fPtr->info.height) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap fb: %s\n", strerror(errno));
		}
#else
		free(fPtr->fb);
#endif

		fPtr->engine = NULL;
		fPtr->linear = NULL;
	}
	pScrn->vtSema = FALSE;

	/* unwrap CloseScreen */
	pScreen->CloseScreen = fPtr->CloseScreen;
	return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}

static Bool
CrimeEnterVT(int scrnIndex, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];

	pScrn->vtSema = TRUE;
	return TRUE;
}

static void
CrimeLeaveVT(int scrnIndex, int flags)
{
}

static Bool
CrimeSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{

	/* Nothing else to do */
	return TRUE;
}

static int
CrimeValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{

	return MODE_OK;
}

static void
CrimeLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
	       LOCO *colors, VisualPtr pVisual)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	struct wsdisplay_cmap cmap;
	unsigned char red[256],green[256],blue[256];
	int i, indexMin=256, indexMax=0;

	cmap.count   = 1;
	cmap.red   = red;
	cmap.green = green;
	cmap.blue  = blue;

	if (numColors == 1) {
		/* Optimisation */
		cmap.index = indices[0];
		red[0]   = colors[indices[0]].red;
		green[0] = colors[indices[0]].green;
		blue[0]  = colors[indices[0]].blue;
		if (ioctl(fPtr->fd,WSDISPLAYIO_PUTCMAP, &cmap) == -1)
			ErrorF("ioctl FBIOPUTCMAP: %s\n", strerror(errno));
	} else {
		/* Change all colors in 2 syscalls */
		/* and limit the data to be transfered */
		for (i = 0; i < numColors; i++) {
			if (indices[i] < indexMin)
				indexMin = indices[i];
			if (indices[i] > indexMax)
				indexMax = indices[i];
		}
		cmap.index = indexMin;
		cmap.count = indexMax - indexMin + 1;
		cmap.red = &red[indexMin];
		cmap.green = &green[indexMin];
		cmap.blue = &blue[indexMin];
		/* Get current map */
		if (ioctl(fPtr->fd, WSDISPLAYIO_GETCMAP, &cmap) == -1)
			ErrorF("ioctl FBIOGETCMAP: %s\n", strerror(errno));
		/* Change the colors that require updating */
		for (i = 0; i < numColors; i++) {
			red[indices[i]]   = colors[indices[i]].red;
			green[indices[i]] = colors[indices[i]].green;
			blue[indices[i]]  = colors[indices[i]].blue;
		}
		/* Write the colormap back */
		if (ioctl(fPtr->fd,WSDISPLAYIO_PUTCMAP, &cmap) == -1)
			ErrorF("ioctl FBIOPUTCMAP: %s\n", strerror(errno));
	}
}

static Bool
CrimeSaveScreen(ScreenPtr pScreen, int mode)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	CrimePtr fPtr = CRIMEPTR(pScrn);
	int state;

	if (!pScrn->vtSema)
		return TRUE;

	if (mode != SCREEN_SAVER_FORCER) {
		state = xf86IsUnblank(mode)?WSDISPLAYIO_VIDEO_ON:
		                            WSDISPLAYIO_VIDEO_OFF;
		ioctl(fPtr->fd,
		      WSDISPLAYIO_SVIDEO, &state);
	}
	return TRUE;
}


static void
CrimeSave(ScrnInfoPtr pScrn)
{
}

static void
CrimeRestore(ScrnInfoPtr pScrn)
{
	CrimePtr fPtr = CRIMEPTR(pScrn);
	int mode;

	/* Restore the text mode */
	mode = WSDISPLAYIO_MODE_EMUL;
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error setting text mode %s\n", strerror(errno));
	}
}
