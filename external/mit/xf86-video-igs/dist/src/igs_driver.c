/* $OpenBSD: wsfb_driver.c,v 1.19 2003/04/27 16:42:32 matthieu Exp $ */
/* $NetBSD: igs_driver.c,v 1.11 2014/08/25 15:27:00 macallan Exp $ */
/*
 * Copyright (c) 2001 Matthieu Herrb
 *		 2009 Michael Lorenz
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

/*
 * Based on fbdev.c written by:
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *	     Michel Dänzer, <michdaen@iiic.ethz.ch>
 */
 
 /*
  * a driver for IGS CyberPro 2010 graphics controllers
  * adapted from wsfb
  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <dev/wscons/wsconsio.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"

#include "mipointer.h"
#include "mibstore.h"
#include "micmap.h"
#include "colormapst.h"
#include "xf86cmap.h"
#include "shadow.h"
#include "dgaproc.h"

/* Everything using inb/outb, etc needs "compiler.h" */
#include "compiler.h"

/* for visuals */
#include "fb.h"

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 7
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif

#ifdef XvExtension
#include "xf86xv.h"
#endif

#include "igs.h"

#include <sys/mman.h>

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) > 6
#define xf86LoaderReqSymLists(...) do {} while (0)
#define LoaderRefSymLists(...) do {} while (0)
#define xf86LoaderReqSymbols(...) do {} while (0)
#endif

#define DEBUG 0

#if DEBUG
# define TRACE_ENTER(str)       ErrorF("igs: " str " %d\n",pScrn->scrnIndex)
# define TRACE_EXIT(str)        ErrorF("igs: " str " done\n")
# define TRACE(str)             ErrorF("igs trace: " str "\n")
#else
# define TRACE_ENTER(str)
# define TRACE_EXIT(str)
# define TRACE(str)
#endif

#define IGS_DEFAULT_DEV "/dev/ttyE0"

/* Prototypes */
#ifdef XFree86LOADER
static pointer IgsSetup(pointer, pointer, int *, int *);
#endif
static Bool IgsGetRec(ScrnInfoPtr);
static void IgsFreeRec(ScrnInfoPtr);
static const OptionInfoRec * IgsAvailableOptions(int, int);
static void IgsIdentify(int);
static Bool IgsProbe(DriverPtr, int);
static Bool IgsPreInit(ScrnInfoPtr, int);
static Bool IgsScreenInit(int, ScreenPtr, int, char **);
static Bool IgsCloseScreen(int, ScreenPtr);
static void *IgsWindowLinear(ScreenPtr, CARD32, CARD32, int, CARD32 *,
			      void *);
static Bool IgsEnterVT(int, int);
static void IgsLeaveVT(int, int);
static Bool IgsSwitchMode(int, DisplayModePtr, int);
static int IgsValidMode(int, DisplayModePtr, Bool, int);
static void IgsLoadPalette(ScrnInfoPtr, int, int *, LOCO *, VisualPtr);
static Bool IgsSaveScreen(ScreenPtr, int);
static void IgsSave(ScrnInfoPtr);
static void IgsRestore(ScrnInfoPtr);

/* dga stuff */
#ifdef XFreeXDGA
static Bool IgsDGAOpenFramebuffer(ScrnInfoPtr, char **, unsigned char **,
				   int *, int *, int *);
static Bool IgsDGASetMode(ScrnInfoPtr, DGAModePtr);
static void IgsDGASetViewport(ScrnInfoPtr, int, int, int);
static Bool IgsDGAInit(ScrnInfoPtr, ScreenPtr);
#endif
static Bool IgsDriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
				pointer ptr);

/* helper functions */
static pointer igs_mmap(size_t, off_t, int);

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;

#define IGS_VERSION 		4000
#define IGS_NAME		"igs"
#define IGS_DRIVER_NAME	"igs"

_X_EXPORT DriverRec IGS = {
	IGS_VERSION,
	IGS_DRIVER_NAME,
	IgsIdentify,
	IgsProbe,
	IgsAvailableOptions,
	NULL,
	0,
	IgsDriverFunc
};

/* Supported "chipsets" */
static SymTabRec IgsChipsets[] = {
	{ 0, "CyberPro 2010" },
	{ -1, NULL }
};

/* Supported options */
typedef enum {
	OPTION_NOACCEL,
	OPTION_HW_CURSOR,
	OPTION_SW_CURSOR
} IgsOpts;

static const OptionInfoRec IgsOptions[] = {
	{ OPTION_NOACCEL, "NoAccel", OPTV_BOOLEAN, {0}, FALSE},
	{ -1, NULL, OPTV_NONE, {0}, FALSE}
};

/* Symbols needed from other modules */
static const char *fbSymbols[] = {
	"fbPictureInit",
	"fbScreenInit",
	NULL
};
static const char *shadowSymbols[] = {
	"shadowAdd",
	"shadowSetup",
	"shadowUpdatePacked",
	"shadowUpdatePackedWeak",
	"shadowUpdateRotatePacked",
	"shadowUpdateRotatePackedWeak",
	NULL
};

static const char *ramdacSymbols[] = {
	"xf86CreateCursorInfoRec",
	"xf86DestroyCursorInfoRec",
	"xf86InitCursor",
	NULL
};

#ifdef XFree86LOADER
static XF86ModuleVersionInfo IgsVersRec = {
	"igs",
	"The NetBSD Foundation",
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	PACKAGE_VERSION_MAJOR, 
	PACKAGE_VERSION_MINOR, 
	PACKAGE_VERSION_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	NULL,
	{0, 0, 0, 0}
};

_X_EXPORT XF86ModuleData igsModuleData = { &IgsVersRec, IgsSetup, NULL };

static pointer
IgsSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;
	const char *osname;

	/* Check that we're being loaded on a OpenBSD or NetBSD system */
	LoaderGetOS(&osname, NULL, NULL, NULL);
	if (!osname || (strcmp(osname, "openbsd") != 0 &&
	                strcmp(osname, "netbsd") != 0)) {
		if (errmaj)
			*errmaj = LDR_BADOS;
		if (errmin)
			*errmin = 0;
		return NULL;
	}
	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&IGS, module, HaveDriverFuncs);
		LoaderRefSymLists(fbSymbols, shadowSymbols, ramdacSymbols,
		    NULL);
		return (pointer)1;
	} else {
		if (errmaj != NULL)
			*errmaj = LDR_ONCEONLY;
		return NULL;
	}
}
#endif /* XFree86LOADER */

static Bool
IgsGetRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate != NULL)
		return TRUE;

	pScrn->driverPrivate = xnfcalloc(sizeof(IgsRec), 1);
	return TRUE;
}

static void
IgsFreeRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate == NULL)
		return;
	xfree(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

static const OptionInfoRec *
IgsAvailableOptions(int chipid, int busid)
{
	return IgsOptions;
}

static void
IgsIdentify(int flags)
{
	xf86PrintChipsets(IGS_NAME, "driver for IGS CyberPro 2010",
			  IgsChipsets);
}

#define priv_open_device(n)	open(n,O_RDWR|O_NONBLOCK|O_EXCL)

/* Open the framebuffer device */
static int
igs_open(char *dev)
{
	int fd = -1;

	/* try argument from XF86Config first */
	if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
		/* second: environment variable */
		dev = getenv("XDEVICE");
		if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
			/* last try: default device */
			dev = IGS_DEFAULT_DEV;
			if ((fd = priv_open_device(dev)) == -1) {
				return -1;
			}
		}
	}
	return fd;
}
/* Map the framebuffer's memory */
static pointer
igs_mmap(size_t len, off_t off, int fd)
{
	int pagemask, mapsize;
	caddr_t addr;
	pointer mapaddr;

	pagemask = getpagesize() - 1;
	mapsize = ((int) len + pagemask) & ~pagemask;
	addr = 0;

	/*
	 * try and make it private first, that way once we get it, an
	 * interloper, e.g. another server, can't get this frame buffer,
	 * and if another server already has it, this one won't.
	 */
	mapaddr = (pointer) mmap(addr, mapsize,
				 PROT_READ | PROT_WRITE, MAP_SHARED,
				 fd, off);
	if (mapaddr == MAP_FAILED) {
		mapaddr = NULL;
	}
#if DEBUG
	ErrorF("mmap returns: addr %p len 0x%x\n", mapaddr, mapsize);
#endif
	return mapaddr;
}

static int
igsFindIsaDevice(GDevPtr dev)
{
	int found = -1;
	uint8_t id0, id1, rev;

	/* read chip ID from extended VGA registers */
	id0 = igs_ext_read(IGS_EXT_CHIP_ID0);
	id1 = igs_ext_read(IGS_EXT_CHIP_ID1);
	rev = igs_ext_read(IGS_EXT_CHIP_REV);
	xf86Msg(X_ERROR, "%s: %x %x %x\n", __func__, id0, id1, rev);
	if ((id0 == 0xa4) && (id1 == 8))
		found = 0;
	return found;
}

static Bool
IgsProbe(DriverPtr drv, int flags)
{
    ScrnInfoPtr pScrn = NULL;
    IgsPtr cPtr;
    Bool foundScreen = FALSE;
    int numDevSections, numUsed;
    GDevPtr *devSections;
    int *usedChips;
    int i, chipset, entity;
    
    /*
     * Find the config file Device sections that match this
     * driver, and return if there are none.
     */
    if ((numDevSections = xf86MatchDevice(IGS_DRIVER_NAME,
					  &devSections)) <= 0) {
	return FALSE;
    }

    /* Isa Bus */
    if ((numDevSections =
      xf86MatchDevice(IGS_DRIVER_NAME, &devSections)) > 0) {
	for (i = 0; i < numDevSections; i++) {
	    if ((chipset = igsFindIsaDevice(devSections[i])) > -1) {
		if ( xf86DoConfigure && xf86DoConfigurePass1 ) {
		    xf86AddBusDeviceToConfigure(IGS_DRIVER_NAME, BUS_ISA, 
			  NULL, chipset);
		}
		if (flags & PROBE_DETECT) {
		    return TRUE;
		}
		if (!xf86CheckStrOption(devSections[i]->options, "BusID", 
		  "ISA")) {
		    continue;
		}

		pScrn = NULL;
		entity = xf86ClaimFbSlot(drv, 0, devSections[i], TRUE);
	    	pScrn = xf86ConfigFbEntity(NULL, 0, entity, NULL, NULL,
		  NULL, NULL);
		pScrn->driverVersion = IGS_VERSION;
		pScrn->driverName    = IGS_DRIVER_NAME;
		pScrn->name          = IGS_NAME;
		pScrn->Probe         = IgsProbe;
		pScrn->PreInit       = IgsPreInit;
		pScrn->ScreenInit    = IgsScreenInit;
		pScrn->SwitchMode    = IgsSwitchMode;
		pScrn->AdjustFrame   = NULL;
		pScrn->EnterVT       = IgsEnterVT;
		pScrn->LeaveVT       = IgsLeaveVT;
		pScrn->ValidMode     = IgsValidMode;
		if (!IgsGetRec(pScrn)) {
		    return FALSE;
		}
		cPtr = IGSPTR(pScrn);
		cPtr->Chipset = chipset;
		cPtr->fb_paddr =
		    ((uint32_t)igs_ext_read(IGS_EXT_LINA_HI)) << 24;
		xf86Msg(X_ERROR, "Aperture at %08lx\n", cPtr->fb_paddr);
	    }
	}
    }
    
    xfree(devSections);
    return foundScreen;
}

static Bool
IgsPreInit(ScrnInfoPtr pScrn, int flags)
{
	IgsPtr fPtr;
	int default_depth, vram_size = 2 * 1024 * 1024;
	char *dev, *s;
	char *mod = NULL;
	const char *reqSym = NULL;
	Gamma zeros = {0.0, 0.0, 0.0};
	DisplayModePtr mode;
	MessageType from;

	if (flags & PROBE_DETECT) return FALSE;

	TRACE_ENTER("PreInit");

	if (pScrn->numEntities != 1) return FALSE;

	pScrn->monitor = pScrn->confScreen->monitor;

	IgsGetRec(pScrn);
	fPtr = IGSPTR(pScrn);

	fPtr->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
	pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
	pScrn->racIoFlags = pScrn->racMemFlags;
#endif

	dev = xf86FindOptionValue(fPtr->pEnt->device->options, "device");
	fPtr->fd = igs_open(dev);
	if (fPtr->fd == -1) {
		return FALSE;
	}

	if (ioctl(fPtr->fd, WSDISPLAYIO_GINFO, &fPtr->info) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAY_GINFO: %s\n",
			   strerror(errno));
		return FALSE;
	}

	/* Handle depth */
	default_depth = fPtr->info.depth <= 24 ? fPtr->info.depth : 24;
	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth,
		fPtr->info.depth,
		fPtr->info.depth >= 24 ? Support24bppFb|Support32bppFb : 0))
		return FALSE;

	/* Check consistency */
	if (pScrn->bitsPerPixel != fPtr->info.depth) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		    "specified depth (%d) or bpp (%d) doesn't match "
		    "framebuffer depth (%d)\n", pScrn->depth, 
		    pScrn->bitsPerPixel, fPtr->info.depth);
		return FALSE;
	}
	xf86PrintDepthBpp(pScrn);

	/* Get the depth24 pixmap format */
	if (pScrn->depth == 24 && pix24bpp == 0)
		pix24bpp = xf86GetBppFromDepth(pScrn, 24);

	/* color weight */
	if (pScrn->depth > 8) {
		rgb zeros = { 0, 0, 0 }, masks;

		masks.red = 0;
		masks.green = 0;
		masks.blue = 0;

		if (!xf86SetWeight(pScrn, zeros, masks))
			return FALSE;
	}

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
	pScrn->chipset   = "igs";
	pScrn->videoRam  = vram_size;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vidmem: %dk\n",
		   pScrn->videoRam/1024);

	/* handle options */
	xf86CollectOptions(pScrn, NULL);
	if (!(fPtr->Options = xalloc(sizeof(IgsOptions))))
		return FALSE;
	memcpy(fPtr->Options, IgsOptions, sizeof(IgsOptions));
	xf86ProcessOptions(pScrn->scrnIndex, fPtr->pEnt->device->options,
			   fPtr->Options);

	/* fake video mode struct */
	mode = (DisplayModePtr)xalloc(sizeof(DisplayModeRec));
	mode->prev = mode;
	mode->next = mode;
	mode->name = "igs current mode";
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

	if (xf86GetOptValBool(fPtr->Options, OPTION_NOACCEL, &fPtr->no_accel))
		from = X_CONFIG;

	xf86DrvMsg(pScrn->scrnIndex, from, "%s acceleration\n",
		fPtr->no_accel ? "disabling" : "enabling");

	/* Load bpp-specific modules */
	switch(pScrn->bitsPerPixel) {
	default:
		mod = "fb";
		break;
	}

	if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
		IgsFreeRec(pScrn);
		return FALSE;
	}

	if (xf86LoadSubModule(pScrn, "ramdac") == NULL) {
		IgsFreeRec(pScrn);
		return FALSE;
        }

	if (mod) {
		if (reqSym) {
			xf86LoaderReqSymbols(reqSym, NULL);
		} else {
			xf86LoaderReqSymLists(fbSymbols, NULL);
		}
	}
	TRACE_EXIT("PreInit");
	return TRUE;
}

static Bool
IgsCreateScreenResources(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	PixmapPtr pPixmap;
	Bool ret;

	pScreen->CreateScreenResources = fPtr->CreateScreenResources;
	ret = pScreen->CreateScreenResources(pScreen);
	pScreen->CreateScreenResources = IgsCreateScreenResources;

	if (!ret)
		return FALSE;

	pPixmap = pScreen->GetScreenPixmap(pScreen);

	if (!shadowAdd(pScreen, pPixmap, shadowUpdatePackedWeak(),
		IgsWindowLinear, FALSE, NULL)) {
		return FALSE;
	}
	return TRUE;
}


static Bool
IgsShadowInit(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);

	if (!shadowSetup(pScreen))
		return FALSE;
	fPtr->CreateScreenResources = pScreen->CreateScreenResources;
	pScreen->CreateScreenResources = IgsCreateScreenResources;

	return TRUE;
}

static Bool
IgsScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	VisualPtr visual;
	int ret, flags, ncolors;
	int wsmode = WSDISPLAYIO_MODE_MAPPED;
	size_t len;

	TRACE_ENTER("IgsScreenInit");
#if DEBUG
	ErrorF("\tbitsPerPixel=%d, depth=%d, defaultVisual=%s\n"
	       "\tmask: %x,%x,%x, offset: %u,%u,%u\n",
	       pScrn->bitsPerPixel,
	       pScrn->depth,
	       xf86GetVisualName(pScrn->defaultVisual),
	       pScrn->mask.red,pScrn->mask.green,pScrn->mask.blue,
	       pScrn->offset.red,pScrn->offset.green,pScrn->offset.blue);
#endif
	fPtr->linebytes = fPtr->info.width * (fPtr->info.depth >> 3);

	/* Switch to graphics mode - required before mmap */
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &wsmode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAYIO_SMODE: %s\n",
			   strerror(errno));
		return FALSE;
	}

	/* find our aperture */
	
	/* assume 2MB for now, until I add actual RAM size probing */
	len = 2 * 1024 * 1024;
	fPtr->fbmem = igs_mmap(len, fPtr->fb_paddr, fPtr->fd);

	if (fPtr->fbmem == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "igs_mmap(fb): %s\n", strerror(errno));
		return FALSE;
	}
	fPtr->fbmem_len = len - 1024; /* leave room for the hw cursor */

	fPtr->reg = igs_mmap(4096,
	    fPtr->fb_paddr + IGS_MEM_MMIO_SELECT + IGS_COP_BASE_B, fPtr->fd);
	if (fPtr->reg == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "igs_mmap(registers): %s\n", strerror(errno));
		return FALSE;
	}
	xf86Msg(X_ERROR, "0x10: %08x\n", *(uint32_t *)(fPtr->reg + 0x10));

	IgsSave(pScrn);
	pScrn->vtSema = TRUE;

	/* mi layer */
	miClearVisualTypes();
	if (pScrn->bitsPerPixel > 8) {
		if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
				      pScrn->rgbBits, TrueColor))
			return FALSE;
	} else {
		if (!miSetVisualTypes(pScrn->depth,
				      miGetDefaultVisualMask(pScrn->depth),
				      pScrn->rgbBits, pScrn->defaultVisual))
			return FALSE;
	}
	if (!miSetPixmapDepths())
		return FALSE;

	fPtr->fbstart = fPtr->fbmem;

	switch (pScrn->bitsPerPixel) {
	case 8:
	case 16:
	case 24:
	case 32:
		ret = fbScreenInit(pScreen,
		    fPtr->fbstart,
		    pScrn->virtualX, pScrn->virtualY,
		    pScrn->xDpi, pScrn->yDpi,
		    pScrn->displayWidth, pScrn->bitsPerPixel);
		break;
	default:
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Unsupported bpp: %d", pScrn->bitsPerPixel);
		return FALSE;
	} /* case */

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

	if (pScrn->bitsPerPixel >= 8) {
		if (!fbPictureInit(pScreen, NULL, 0))
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
				   "RENDER extension initialisation failed.");
	}

#ifdef XFreeXDGA
	IgsDGAInit(pScrn, pScreen);
#endif

	xf86SetBlackWhitePixels(pScreen);
	miInitializeBackingStore(pScreen);
	xf86SetBackingStore(pScreen);

	/* setup acceleration */
	if (!fPtr->no_accel) {
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
		if (!IgsInitAccel(pScreen))
			fPtr->no_accel = 1;
	}

	/* software cursor */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	/* check for hardware cursor support */
	if (fPtr->HWCursor)
		IgsSetupCursor(pScreen);

	/* colormap */
	if (!miCreateDefColormap(pScreen))
		return FALSE;
	flags = CMAP_RELOAD_ON_MODE_SWITCH;
	ncolors = fPtr->info.cmsize;
	/* on StaticGray visuals, fake a 256 entries colormap */
	if (ncolors == 0)
		ncolors = 256;
	if(!xf86HandleColormaps(pScreen, ncolors, 8, IgsLoadPalette,
				NULL, flags))
		return FALSE;

	pScreen->SaveScreen = IgsSaveScreen;

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
	pScreen->CloseScreen = IgsCloseScreen;

	TRACE_EXIT("IgsScreenInit");
	return TRUE;
}

static Bool
IgsCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	IgsPtr fPtr = IGSPTR(pScrn);

	TRACE_ENTER("IgsCloseScreen");

	if (pScrn->vtSema) {
		IgsRestore(pScrn);
		if (munmap(fPtr->fbmem, fPtr->fbmem_len) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap: %s\n", strerror(errno));
		}

		fPtr->fbmem = NULL;
	}
#ifdef XFreeXDGA
	if (fPtr->pDGAMode) {
		xfree(fPtr->pDGAMode);
		fPtr->pDGAMode = NULL;
		fPtr->nDGAMode = 0;
	}
#endif
	pScrn->vtSema = FALSE;

	/* unwrap CloseScreen */
	pScreen->CloseScreen = fPtr->CloseScreen;
	return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}

static void *
IgsWindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		CARD32 *size, void *closure)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);

	if (fPtr->linebytes)
		*size = fPtr->linebytes;
	else {
		if (ioctl(fPtr->fd, WSDISPLAYIO_LINEBYTES, size) == -1)
			return NULL;
		fPtr->linebytes = *size;
	}
	return ((CARD8 *)fPtr->fbmem + row *fPtr->linebytes + offset);
}

static Bool
IgsEnterVT(int scrnIndex, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];

	TRACE_ENTER("EnterVT");
	pScrn->vtSema = TRUE;
	return TRUE;
}

static void
IgsLeaveVT(int scrnIndex, int flags)
{
#if DEBUG
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
#endif

	TRACE_ENTER("LeaveVT");
}

static Bool
IgsSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
#if DEBUG
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
#endif

	TRACE_ENTER("SwitchMode");
	/* Nothing else to do */
	return TRUE;
}

static int
IgsValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
#if DEBUG
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
#endif

	TRACE_ENTER("ValidMode");
	return MODE_OK;
}

static void
IgsLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
	       LOCO *colors, VisualPtr pVisual)
{
	IgsPtr fPtr = IGSPTR(pScrn);
	struct wsdisplay_cmap cmap;
	unsigned char red[256],green[256],blue[256];
	int i, indexMin=256, indexMax=0;

	TRACE_ENTER("LoadPalette");

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
	TRACE_EXIT("LoadPalette");
}

static Bool
IgsSaveScreen(ScreenPtr pScreen, int mode)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	IgsPtr fPtr = IGSPTR(pScrn);
	int state;

	TRACE_ENTER("SaveScreen");

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
IgsSave(ScrnInfoPtr pScrn)
{
	IgsPtr fPtr = IGSPTR(pScrn);

	TRACE_ENTER("IgsSave");

	if (fPtr->info.cmsize == 0)
		return;

}

static void
IgsRestore(ScrnInfoPtr pScrn)
{
	IgsPtr fPtr = IGSPTR(pScrn);
	int mode;

	TRACE_ENTER("IgsRestore");

	/* Clear the screen */
	memset(fPtr->fbmem, 0, fPtr->fbmem_len);

	/* Restore the text mode */
	mode = WSDISPLAYIO_MODE_EMUL;
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error setting text mode %s\n", strerror(errno));
	}
	TRACE_EXIT("IgsRestore");
}

#ifdef XFreeXDGA
/***********************************************************************
 * DGA stuff
 ***********************************************************************/

static Bool
IgsDGAOpenFramebuffer(ScrnInfoPtr pScrn, char **DeviceName,
		       unsigned char **ApertureBase, int *ApertureSize,
		       int *ApertureOffset, int *flags)
{
	*DeviceName = NULL;		/* No special device */
	*ApertureBase = (unsigned char *)(pScrn->memPhysBase);
	*ApertureSize = pScrn->videoRam;
	*ApertureOffset = pScrn->fbOffset;
	*flags = 0;

	return TRUE;
}

static Bool
IgsDGASetMode(ScrnInfoPtr pScrn, DGAModePtr pDGAMode)
{
	DisplayModePtr pMode;
	int scrnIdx = pScrn->pScreen->myNum;
	int frameX0, frameY0;

	if (pDGAMode) {
		pMode = pDGAMode->mode;
		frameX0 = frameY0 = 0;
	} else {
		if (!(pMode = pScrn->currentMode))
			return TRUE;

		frameX0 = pScrn->frameX0;
		frameY0 = pScrn->frameY0;
	}

	if (!(*pScrn->SwitchMode)(scrnIdx, pMode, 0))
		return FALSE;
	(*pScrn->AdjustFrame)(scrnIdx, frameX0, frameY0, 0);

	return TRUE;
}

static void
IgsDGASetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
	(*pScrn->AdjustFrame)(pScrn->pScreen->myNum, x, y, flags);
}

static int
IgsDGAGetViewport(ScrnInfoPtr pScrn)
{
	return (0);
}

static DGAFunctionRec IgsDGAFunctions =
{
	IgsDGAOpenFramebuffer,
	NULL,       /* CloseFramebuffer */
	IgsDGASetMode,
	IgsDGASetViewport,
	IgsDGAGetViewport,
	NULL,       /* Sync */
	NULL,       /* FillRect */
	NULL,       /* BlitRect */
	NULL,       /* BlitTransRect */
};

static void
IgsDGAAddModes(ScrnInfoPtr pScrn)
{
	IgsPtr fPtr = IGSPTR(pScrn);
	DisplayModePtr pMode = pScrn->modes;
	DGAModePtr pDGAMode;

	do {
		pDGAMode = xrealloc(fPtr->pDGAMode,
				    (fPtr->nDGAMode + 1) * sizeof(DGAModeRec));
		if (!pDGAMode)
			break;

		fPtr->pDGAMode = pDGAMode;
		pDGAMode += fPtr->nDGAMode;
		(void)memset(pDGAMode, 0, sizeof(DGAModeRec));

		++fPtr->nDGAMode;
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

		if (fPtr->linebytes)
			pDGAMode->bytesPerScanline = fPtr->linebytes;
		else {
			ioctl(fPtr->fd, WSDISPLAYIO_LINEBYTES,
			      &fPtr->linebytes);
			pDGAMode->bytesPerScanline = fPtr->linebytes;
		}

		pDGAMode->imageWidth = pMode->HDisplay;
		pDGAMode->imageHeight =  pMode->VDisplay;
		pDGAMode->pixmapWidth = pDGAMode->imageWidth;
		pDGAMode->pixmapHeight = pDGAMode->imageHeight;
		pDGAMode->maxViewportX = pScrn->virtualX -
			pDGAMode->viewportWidth;
		pDGAMode->maxViewportY = pScrn->virtualY -
			pDGAMode->viewportHeight;

		pDGAMode->address = fPtr->fbstart;

		pMode = pMode->next;
	} while (pMode != pScrn->modes);
}

static Bool
IgsDGAInit(ScrnInfoPtr pScrn, ScreenPtr pScreen)
{
	IgsPtr fPtr = IGSPTR(pScrn);

	if (pScrn->depth < 8)
		return FALSE;

	if (!fPtr->nDGAMode)
		IgsDGAAddModes(pScrn);

	return (DGAInit(pScreen, &IgsDGAFunctions,
			fPtr->pDGAMode, fPtr->nDGAMode));
}
#endif

static Bool
IgsDriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
    pointer ptr)
{
	xorgHWFlags *flag;
	
	switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
		flag = (CARD32*)ptr;
		(*flag) = HW_IO | HW_MMIO;
		return TRUE;
	default:
		return FALSE;
	}
}

