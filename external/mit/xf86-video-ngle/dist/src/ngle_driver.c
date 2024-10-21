/* $NetBSD: ngle_driver.c,v 1.2 2024/10/21 13:40:53 macallan Exp $ */
/*
 * Copyright (c) 2024 Michael Lorenz
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

/* a driver for HP's NGLE family of graphics chips */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <dev/ic/stireg.h>

#include <fcntl.h>
#include <errno.h>
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
#ifdef XvExtension
#include "xf86xv.h"
#endif

/* for visuals */
#include "fb.h"

#include "ngle.h"

/* #include "wsconsio.h" */

#define NGLE_DEFAULT_DEV "/dev/ttyE0"

static pointer NGLESetup(pointer, pointer, int *, int *);
static Bool NGLEGetRec(ScrnInfoPtr);
static void NGLEFreeRec(ScrnInfoPtr);
static const OptionInfoRec * NGLEAvailableOptions(int, int);
static void NGLEIdentify(int);
static Bool NGLEProbe(DriverPtr, int);
static Bool NGLEPreInit(ScrnInfoPtr, int);
static Bool NGLEScreenInit(SCREEN_INIT_ARGS_DECL);
static Bool NGLECloseScreen(CLOSE_SCREEN_ARGS_DECL);
static Bool NGLEEnterVT(VT_FUNC_ARGS_DECL);
static void NGLELeaveVT(VT_FUNC_ARGS_DECL);
static Bool NGLESwitchMode(SWITCH_MODE_ARGS_DECL);
static int NGLEValidMode(SCRN_ARG_TYPE, DisplayModePtr, Bool, int);
static void NGLELoadPalette(ScrnInfoPtr, int, int *, LOCO *, VisualPtr);
static Bool NGLESaveScreen(ScreenPtr, int);
static void NGLESave(ScrnInfoPtr);
static void NGLERestore(ScrnInfoPtr);

/* helper functions */
static int ngle_open(const char *);
static pointer ngle_mmap(size_t, off_t, int, int);

#define VERSION			4000
#define NGLE_NAME		"ngle"
#define NGLE_DRIVER_NAME	"ngle"
#define NGLE_MAJOR_VERSION	0
#define NGLE_MINOR_VERSION	1

DriverRec NGLE = {
	VERSION,
	NGLE_DRIVER_NAME,
	NGLEIdentify,
	NGLEProbe,
	NGLEAvailableOptions,
	NULL,
	0
};

/* Supported "chipsets" */
static SymTabRec NGLEChipsets[] = {
	{ STI_DD_EG, "Visualize EG" },
	{ STI_DD_HCRX, "HCRX" },
	{ -1, NULL }
};

/* Supported options */
typedef enum {
	OPTION_HW_CURSOR,
	OPTION_SW_CURSOR
} NGLEOpts;

static const OptionInfoRec NGLEOptions[] = {
	{ OPTION_SW_CURSOR, "SWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
	{ OPTION_HW_CURSOR, "HWcursor",	OPTV_BOOLEAN,	{0}, FALSE },
	{ -1, NULL, OPTV_NONE, {0}, FALSE}
};

static XF86ModuleVersionInfo NGLEVersRec = {
	"ngle",
	"The NetBSD Foundation",
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	NGLE_MAJOR_VERSION, NGLE_MINOR_VERSION, 0,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	NULL,
	{0, 0, 0, 0}
};

_X_EXPORT XF86ModuleData ngleModuleData = { &NGLEVersRec, NGLESetup, NULL };

static pointer
NGLESetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;
	const char *osname;

	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&NGLE, module, 0);
		return (pointer)1;
	} else {
		if (errmaj != NULL)
			*errmaj = LDR_ONCEONLY;
		return NULL;
	}
}

static Bool
NGLEGetRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate != NULL)
		return TRUE;

	pScrn->driverPrivate = xnfcalloc(sizeof(NGLERec), 1);
	return TRUE;
}

static void
NGLEFreeRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate == NULL)
		return;
	free(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

static const OptionInfoRec *
NGLEAvailableOptions(int chipid, int busid)
{
	return NGLEOptions;
}

static void
NGLEIdentify(int flags)
{
	xf86PrintChipsets(NGLE_NAME, "driver for NGLE framebuffers",
			  NGLEChipsets);
}


#define priv_open_device(n)	open(n,O_RDWR|O_NONBLOCK|O_EXCL)

/* Open the framebuffer device */
static int
ngle_open(const char *dev)
{
	int fd = -1;

	/* try argument from XF86Config first */
	if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
		/* second: environment variable */
		dev = getenv("XDEVICE");
		if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
			/* last try: default device */
			dev = NGLE_DEFAULT_DEV;
			if ((fd = priv_open_device(dev)) == -1) {
				return -1;
			}
		}
	}
	return fd;
}

/* Map the framebuffer's memory */
static pointer
ngle_mmap(size_t len, off_t off, int fd, int ro)
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
#ifdef NGLE_DEBUG
	ErrorF("mmap returns: addr %p len 0x%x\n", mapaddr, len);
#endif
	return mapaddr;
}

static Bool
NGLEProbe(DriverPtr drv, int flags)
{
	ScrnInfoPtr pScrn = NULL;
	int i, fd, entity, wstype;
       	GDevPtr *devSections;
	int numDevSections;
	char *dev;
	const char *name;
	uint32_t gid;
	Bool foundScreen = FALSE;

	if ((numDevSections = xf86MatchDevice(NGLE_DRIVER_NAME,
					      &devSections)) <= 0)
		return FALSE;


	if ((fd = ngle_open(NGLE_DEFAULT_DEV)) == 0)
		return FALSE;

	if (ioctl(fd, WSDISPLAYIO_GTYPE, &wstype) == -1)
		return FALSE;

	if (wstype != WSDISPLAY_TYPE_STI)
		return FALSE;

	if (ioctl(fd, GCID, &gid) == -1)
		return FALSE;

	/* reject GIDs not in the table */
	if ((name = xf86TokenToString(NGLEChipsets, gid)) == NULL)
		return FALSE;

	xf86Msg(X_INFO, "%s: found %s ( GID %08x )\n", __func__, name, gid);

	if ( xf86DoConfigure && xf86DoConfigurePass1 ) {
		GDevPtr pGDev;

		pGDev = xf86AddBusDeviceToConfigure(NGLE_DRIVER_NAME, BUS_NONE,
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
	/* ok, at this point we know we've got a NGLE */
	for (i = 0; i < numDevSections; i++) {
	
		entity = xf86ClaimFbSlot(drv, 0, devSections[i], TRUE);
		pScrn = xf86ConfigFbEntity(NULL, 0, entity,
		    NULL, NULL, NULL, NULL);
		if (pScrn != NULL) {
			foundScreen = TRUE;
			pScrn->driverVersion = VERSION;
			pScrn->driverName = NGLE_DRIVER_NAME;
			pScrn->name = NGLE_NAME;
			pScrn->Probe = NGLEProbe;
			pScrn->PreInit = NGLEPreInit;
			pScrn->ScreenInit = NGLEScreenInit;
			pScrn->SwitchMode = NGLESwitchMode;
			pScrn->AdjustFrame = NULL;
			pScrn->EnterVT = NGLEEnterVT;
			pScrn->LeaveVT = NGLELeaveVT;
			pScrn->ValidMode = NGLEValidMode;

		}
	}
	free(devSections);
	return foundScreen;
}

static Bool
NGLEPreInit(ScrnInfoPtr pScrn, int flags)
{
	NGLEPtr fPtr;
	int default_depth, bitsperpixel, gid;
	const char *dev;
	char *mod = NULL;
	const char *reqSym = NULL;
	Gamma zeros = {0.0, 0.0, 0.0};
	DisplayModePtr mode;
	MessageType from;
	rgb rgbzeros = { 0, 0, 0 }, masks;

	if (flags & PROBE_DETECT) return FALSE;

	if (pScrn->numEntities != 1) return FALSE;

	pScrn->monitor = pScrn->confScreen->monitor;

	NGLEGetRec(pScrn);
	fPtr = NGLEPTR(pScrn);

	fPtr->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

	dev = xf86FindOptionValue(fPtr->pEnt->device->options, "device");
	fPtr->fd = ngle_open(dev);
	if (fPtr->fd == -1) {
		return FALSE;
	}

	if (ioctl(fPtr->fd, WSDISPLAYIO_GET_FBINFO, &fPtr->fbi) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAY_GINFO: %s\n",
			   strerror(errno));
		return FALSE;
	}

	if (ioctl(fPtr->fd, GCID, &gid) == -1)
		return FALSE;

	fPtr->gid = gid;
	fPtr->fbacc = 0;
	
	switch (gid) {
		case STI_DD_EG:
			fPtr->buf = BINapp1I;
			fPtr->fbacc = BA(IndexedDcd, Otc04, Ots08, AddrByte, 0, fPtr->buf, 0);
			break;
		case STI_DD_HCRX:
			/* XXX BINovly if in 8 bit */
			fPtr->buf = BINapp0F8;
			fPtr->fbacc = BA(IndexedDcd, Otc04, Ots08, AddrLong, 0, fPtr->buf, 0);
	break;
	}
	xf86Msg(X_ERROR, "gid %08x fb access %08x\n", fPtr->gid, fPtr->fbacc);		

	/* Handle depth */
	default_depth = fPtr->fbi.fbi_bitsperpixel <= 24 ? fPtr->fbi.fbi_bitsperpixel : 24;
	bitsperpixel = fPtr->fbi.fbi_bitsperpixel == 15 ? 16 : fPtr->fbi.fbi_bitsperpixel;
	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth,
		bitsperpixel,
		bitsperpixel >= 24 ? Support24bppFb|Support32bppFb : 0))
		return FALSE;

	xf86PrintDepthBpp(pScrn);

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
	pScrn->chipset   = "NGLE";
	fPtr->fbmem_len = pScrn->videoRam  = fPtr->fbi.fbi_fbsize;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vidmem: %dk\n",
		   pScrn->videoRam/1024);

	/* handle options */
	xf86CollectOptions(pScrn, NULL);
	if (!(fPtr->Options = malloc(sizeof(NGLEOptions))))
		return FALSE;
	memcpy(fPtr->Options, NGLEOptions, sizeof(NGLEOptions));
	xf86ProcessOptions(pScrn->scrnIndex, fPtr->pEnt->device->options,
			   fPtr->Options);
	
	/* fake video mode struct */
	mode = (DisplayModePtr)malloc(sizeof(DisplayModeRec));
	mode->prev = mode;
	mode->next = mode;
	mode->name = "NGLE current mode";
	mode->status = MODE_OK;
	mode->type = M_T_BUILTIN;
	mode->Clock = 0;
	mode->HDisplay = fPtr->fbi.fbi_width;
	mode->HSyncStart = 0;
	mode->HSyncEnd = 0;
	mode->HTotal = 0;
	mode->HSkew = 0;
	mode->VDisplay = fPtr->fbi.fbi_height;
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
	pScrn->virtualX = fPtr->fbi.fbi_width;
	pScrn->virtualY = fPtr->fbi.fbi_height;
	pScrn->displayWidth = 2048;

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
		NGLEFreeRec(pScrn);
		return FALSE;
	}

	if (xf86LoadSubModule(pScrn, "exa") == NULL) {
		NGLEFreeRec(pScrn);
		return FALSE;
	}

	if (xf86LoadSubModule(pScrn, "ramdac") == NULL) {
		NGLEFreeRec(pScrn);
		return FALSE;
	}
	
	return TRUE;
}

static Bool
NGLEScreenInit(SCREEN_INIT_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
	VisualPtr visual;
	int ret, flags, width, height, i, j;
	int wsmode = WSDISPLAYIO_MODE_MAPPED;
	size_t len;

#ifdef NGLE_DEBUG
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
	fPtr->regs = ngle_mmap(0x400000, 0x80000000, fPtr->fd, 0);

	if (fPtr->regs == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ngle_mmap registers: %s\n", strerror(errno));
		return FALSE;
	}

	fPtr->fbmem = ngle_mmap(fPtr->fbmem_len, 0, fPtr->fd, 0);
	if (fPtr->fbmem == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ngle_mmap fb: %s\n", strerror(errno));
		return FALSE;
	}

	NGLESave(pScrn);
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
			   fPtr->fbmem,
			   width, height,
			   pScrn->xDpi, pScrn->yDpi,
			   pScrn->displayWidth,
			   pScrn->bitsPerPixel);

	if (!ret)
		return FALSE;

	if (pScrn->bitsPerPixel > 8) {
		/* Fixup RGB ordering. */
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

	if (!fbPictureInit(pScreen, NULL, 0))
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "RENDER extension initialisation failed.");

	xf86SetBlackWhitePixels(pScreen);
	xf86SetBackingStore(pScreen);

	if (fPtr) {
		NGLEInitAccel(pScreen);
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Using acceleration\n");
	}

	/* software cursor */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	/* check for hardware cursor support */
	NGLESetupCursor(pScreen);
	
	/* colormap */
	if (!miCreateDefColormap(pScreen))
		return FALSE;

	flags = CMAP_RELOAD_ON_MODE_SWITCH;
	if(!xf86HandleColormaps(pScreen, 256, 8, NGLELoadPalette,
				NULL, flags))
		return FALSE;

	pScreen->SaveScreen = NGLESaveScreen;

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
	pScreen->CloseScreen = NGLECloseScreen;

	return TRUE;
}

static Bool
NGLECloseScreen(CLOSE_SCREEN_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	NGLEPtr fPtr = NGLEPTR(pScrn);

	if (pScrn->vtSema) {
		NGLERestore(pScrn);
		if (munmap(fPtr->regs, 0x40000) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap engine: %s\n", strerror(errno));
		}

		if (munmap(fPtr->fbmem, fPtr->fbmem_len) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap fb: %s\n", strerror(errno));
		}

		fPtr->regs = NULL;
		fPtr->fbmem = NULL;
	}
	pScrn->vtSema = FALSE;

	/* unwrap CloseScreen */
	pScreen->CloseScreen = fPtr->CloseScreen;
	return (*pScreen->CloseScreen)(pScreen);
}

static Bool
NGLEEnterVT(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);

	pScrn->vtSema = TRUE;
	return TRUE;
}

static void
NGLELeaveVT(VT_FUNC_ARGS_DECL)
{
}

static Bool
NGLESwitchMode(SWITCH_MODE_ARGS_DECL)
{

	/* Nothing else to do */
	return TRUE;
}

static int
NGLEValidMode(SCRN_ARG_TYPE, DisplayModePtr mode, Bool verbose, int flags)
{

	return MODE_OK;
}

static void
NGLELoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
	       LOCO *colors, VisualPtr pVisual)
{
	NGLEPtr fPtr = NGLEPTR(pScrn);
	struct wsdisplay_cmap cmap;
	unsigned char red[256],green[256],blue[256];
	int i, indexMin=256, indexMax=0;

	/* nothing to do if there is no color palette support */
	if (fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries == 0)
		return;

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
		/*
		 * Change all colors in 2 ioctls
		 * and limit the data to be transferred.
		 */
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
		/* Get current map. */
		if (ioctl(fPtr->fd, WSDISPLAYIO_GETCMAP, &cmap) == -1)
			ErrorF("ioctl FBIOGETCMAP: %s\n", strerror(errno));
		/* Change the colors that require updating. */
		for (i = 0; i < numColors; i++) {
			red[indices[i]]   = colors[indices[i]].red;
			green[indices[i]] = colors[indices[i]].green;
			blue[indices[i]]  = colors[indices[i]].blue;
		}
		/* Write the colormap back. */
		if (ioctl(fPtr->fd,WSDISPLAYIO_PUTCMAP, &cmap) == -1)
			ErrorF("ioctl FBIOPUTCMAP: %s\n", strerror(errno));
	}
}

static Bool
NGLESaveScreen(ScreenPtr pScreen, int mode)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	NGLEPtr fPtr = NGLEPTR(pScrn);
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
NGLESave(ScrnInfoPtr pScrn)
{
}

static void
NGLERestore(ScrnInfoPtr pScrn)
{
	NGLEPtr fPtr = NGLEPTR(pScrn);
	int mode;

	/* Restore the text mode */
	mode = WSDISPLAYIO_MODE_EMUL;
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error setting text mode %s\n", strerror(errno));
	}
}
