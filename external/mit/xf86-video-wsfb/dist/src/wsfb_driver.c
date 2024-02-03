/*
 * Copyright © 2001-2012 Matthieu Herrb
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>
#include <dev/wscons/wsconsio.h>

/* All drivers need this. */
#include "xf86.h"
#include "xf86_OSproc.h"

#include "mipointer.h"
#include "micmap.h"
#include "colormapst.h"
#include "xf86cmap.h"
#include "shadow.h"
#include "dgaproc.h"

/* For visuals */
#include "fb.h"

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif

#ifdef XvExtension
#include "xf86xv.h"
#endif

#include "wsfb.h"

#ifdef X_PRIVSEP
extern int priv_open_device(const char *);
#else
#define priv_open_device(n)    open(n,O_RDWR|O_NONBLOCK|O_EXCL)
#endif

#if defined(__NetBSD__)
#define WSFB_DEFAULT_DEV "/dev/ttyE0"
#else
#define WSFB_DEFAULT_DEV "/dev/ttyC0"
#endif

#define DEBUG 0

#if DEBUG
# define TRACE_ENTER(str)       ErrorF("wsfb: " str " %d\n",pScrn->scrnIndex)
# define TRACE_EXIT(str)        ErrorF("wsfb: " str " done\n")
# define TRACE(str)             ErrorF("wsfb trace: " str "\n")
#else
# define TRACE_ENTER(str)
# define TRACE_EXIT(str)
# define TRACE(str)
#endif

/* Prototypes */
static pointer WsfbSetup(pointer, pointer, int *, int *);
static Bool WsfbGetRec(ScrnInfoPtr);
static void WsfbFreeRec(ScrnInfoPtr);
static const OptionInfoRec * WsfbAvailableOptions(int, int);
static void WsfbIdentify(int);
static Bool WsfbProbe(DriverPtr, int);
static Bool WsfbPreInit(ScrnInfoPtr, int);
static Bool WsfbScreenInit(SCREEN_INIT_ARGS_DECL);
static Bool WsfbCloseScreen(CLOSE_SCREEN_ARGS_DECL);
static void *WsfbWindowLinear(ScreenPtr, CARD32, CARD32, int, CARD32 *,
			      void *);
#ifdef HAVE_SHADOW_AFB
static void *WsfbWindowAfb(ScreenPtr, CARD32, CARD32, int, CARD32 *,
			      void *);
#endif
static void WsfbPointerMoved(SCRN_ARG_TYPE, int, int);
static Bool WsfbEnterVT(VT_FUNC_ARGS_DECL);
static void WsfbLeaveVT(VT_FUNC_ARGS_DECL);
static Bool WsfbSwitchMode(SWITCH_MODE_ARGS_DECL);
static int WsfbValidMode(SCRN_ARG_TYPE, DisplayModePtr, Bool, int);
static void WsfbLoadPalette(ScrnInfoPtr, int, int *, LOCO *, VisualPtr);
static Bool WsfbSaveScreen(ScreenPtr, int);
static void WsfbSave(ScrnInfoPtr);
static void WsfbRestore(ScrnInfoPtr);

/* DGA stuff */
#ifdef XFreeXDGA
static Bool WsfbDGAOpenFramebuffer(ScrnInfoPtr, char **, unsigned char **,
				   int *, int *, int *);
static Bool WsfbDGASetMode(ScrnInfoPtr, DGAModePtr);
static void WsfbDGASetViewport(ScrnInfoPtr, int, int, int);
static Bool WsfbDGAInit(ScrnInfoPtr, ScreenPtr);
#endif

static void WsfbShadowUpdateRGB16ToYUY2(ScreenPtr, shadowBufPtr);
static void WsfbShadowUpdateSwap32(ScreenPtr, shadowBufPtr);
static void WsfbShadowUpdateSplit(ScreenPtr, shadowBufPtr);

static Bool WsfbDriverFunc(ScrnInfoPtr, xorgDriverFuncOp, pointer);

/* Helper functions */
static int wsfb_open(const char *);
static pointer wsfb_mmap(size_t, off_t, int);

enum { WSFB_ROTATE_NONE = 0,
       WSFB_ROTATE_CCW = 90,
       WSFB_ROTATE_UD = 180,
       WSFB_ROTATE_CW = 270
};

/*
 * This is intentionally screen-independent.
 * It indicates the binding choice made in the first PreInit.
 */
static int pix24bpp = 0;

/*
 * Screen-independent lookup table for RGB16 to YUV conversions.
 */
static unsigned char *mapRGB16ToY = NULL;
static unsigned char *mapRGB16ToU = NULL;
static unsigned char *mapRGB16ToV = NULL;

#define WSFB_VERSION		4000
#define WSFB_NAME		"wsfb"
#define WSFB_DRIVER_NAME	"wsfb"

_X_EXPORT DriverRec WSFB = {
	WSFB_VERSION,
	(char *)WSFB_DRIVER_NAME,
	WsfbIdentify,
	WsfbProbe,
	WsfbAvailableOptions,
	NULL,
	0,
	WsfbDriverFunc
};

/* Supported "chipsets" */
static SymTabRec WsfbChipsets[] = {
	{ 0, "wsfb" },
	{ -1, NULL }
};

/* Supported options */
typedef enum {
	OPTION_SHADOW_FB,
	OPTION_ROTATE,
	OPTION_HW_CURSOR,
	OPTION_SW_CURSOR
} WsfbOpts;

static const OptionInfoRec WsfbOptions[] = {
	{ OPTION_SHADOW_FB, "ShadowFB", OPTV_BOOLEAN, {0}, FALSE},
	{ OPTION_ROTATE, "Rotate", OPTV_STRING, {0}, FALSE},
	{ OPTION_HW_CURSOR, "HWCursor", OPTV_BOOLEAN, {1}, FALSE},
	{ -1, NULL, OPTV_NONE, {0}, FALSE}
};

static XF86ModuleVersionInfo WsfbVersRec = {
	"wsfb",
	MODULEVENDORSTRING,
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

_X_EXPORT XF86ModuleData wsfbModuleData = { &WsfbVersRec, WsfbSetup, NULL };

static pointer
WsfbSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;

#if !defined(__OpenBSD__) && !defined(__NetBSD__)
	return NULL;
#endif

	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&WSFB, module, HaveDriverFuncs);
		return (pointer)1;
	} else {
		if (errmaj != NULL)
			*errmaj = LDR_ONCEONLY;
		return NULL;
	}
}

static Bool
WsfbGetRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate != NULL)
		return TRUE;

	pScrn->driverPrivate = xnfcalloc(sizeof(WsfbRec), 1);
	return TRUE;
}

static void
WsfbFreeRec(ScrnInfoPtr pScrn)
{

	if (pScrn->driverPrivate == NULL)
		return;
	free(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

static const OptionInfoRec *
WsfbAvailableOptions(int chipid, int busid)
{
	return WsfbOptions;
}

static void
WsfbIdentify(int flags)
{
	xf86PrintChipsets(WSFB_NAME, "driver for wsdisplay framebuffer",
			  WsfbChipsets);
}

/* Open the framebuffer device. */
static int
wsfb_open(const char *dev)
{
	int fd = -1;

	/* Try argument from xorg.conf first. */
	if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
		/* Second: environment variable. */
		dev = getenv("XDEVICE");
		if (dev == NULL || ((fd = priv_open_device(dev)) == -1)) {
			/* Last try: default device. */
			dev = WSFB_DEFAULT_DEV;
			if ((fd = priv_open_device(dev)) == -1) {
				return -1;
			}
		}
	}
	return fd;
}

/* Map the framebuffer's memory. */
static pointer
wsfb_mmap(size_t len, off_t off, int fd)
{
	int pagemask, mapsize;
	caddr_t addr;
	pointer mapaddr;

	pagemask = getpagesize() - 1;
	mapsize = ((int) len + pagemask) & ~pagemask;
	addr = 0;

	/*
	 * Try and make it private first, that way once we get it, an
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

static Bool
WsfbProbe(DriverPtr drv, int flags)
{
	int i, fd, entity;
	GDevPtr *devSections;
	int numDevSections;
	const char *dev;
	Bool foundScreen = FALSE;

	TRACE("probe start");

	/* For now, just bail out for PROBE_DETECT. */
	if (flags & PROBE_DETECT)
		return FALSE;

	if ((numDevSections = xf86MatchDevice(WSFB_DRIVER_NAME,
					      &devSections)) <= 0)
		return FALSE;

	for (i = 0; i < numDevSections; i++) {
		ScrnInfoPtr pScrn = NULL;

		dev = xf86FindOptionValue(devSections[i]->options, "device");
		if ((fd = wsfb_open(dev)) >= 0) {
			entity = xf86ClaimFbSlot(drv, 0, devSections[i], TRUE);
			pScrn = xf86ConfigFbEntity(NULL,0,entity,
						   NULL,NULL,NULL,NULL);
			if (pScrn != NULL) {
				foundScreen = TRUE;
				pScrn->driverVersion = WSFB_VERSION;
				pScrn->driverName = (char *)WSFB_DRIVER_NAME;
				pScrn->name = (char *)WSFB_NAME;
				pScrn->Probe = WsfbProbe;
				pScrn->PreInit = WsfbPreInit;
				pScrn->ScreenInit = WsfbScreenInit;
				pScrn->SwitchMode = WsfbSwitchMode;
				pScrn->AdjustFrame = NULL;
				pScrn->EnterVT = WsfbEnterVT;
				pScrn->LeaveVT = WsfbLeaveVT;
				pScrn->ValidMode = WsfbValidMode;

				xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				    "using %s\n", dev != NULL ? dev :
				    "default device");
			}
		}
	}
	free(devSections);
	TRACE("probe done");
	return foundScreen;
}

static Bool
WsfbPreInit(ScrnInfoPtr pScrn, int flags)
{
	WsfbPtr fPtr;
	int default_depth, bitsperpixel, wstype;
	const char *dev;
	const char *s;
	Gamma zeros = {0.0, 0.0, 0.0};
	DisplayModePtr mode;
	MessageType from;

	if (flags & PROBE_DETECT) return FALSE;

	TRACE_ENTER("PreInit");

	if (pScrn->numEntities != 1) return FALSE;

	pScrn->monitor = pScrn->confScreen->monitor;

	WsfbGetRec(pScrn);
	fPtr = WSFBPTR(pScrn);

	fPtr->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 6
	pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
	pScrn->racIoFlags = pScrn->racMemFlags;
#endif

	dev = xf86FindOptionValue(fPtr->pEnt->device->options, "device");
	fPtr->fd = wsfb_open(dev);
	if (fPtr->fd == -1) {
		return FALSE;
	}

	if (ioctl(fPtr->fd, WSDISPLAYIO_GTYPE, &wstype) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "ioctl WSDISPLAY_GTYPE: %s\n",
			   strerror(errno));
		wstype = WSDISPLAY_TYPE_UNKNOWN;
	}

	if (ioctl(fPtr->fd, WSDISPLAYIO_GET_FBINFO, &fPtr->fbi) != 0) {
		struct wsdisplay_fbinfo info;
		struct wsdisplayio_fbinfo *fbi = &fPtr->fbi;
		int lb;

		xf86Msg(X_WARNING, "ioctl(WSDISPLAYIO_GET_FBINFO) failed, " \
			"falling back to old method\n");
		if (ioctl(fPtr->fd, WSDISPLAYIO_GINFO, &info) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "ioctl WSDISPLAY_GINFO: %s\n",
				   strerror(errno));
			return FALSE;
		}
		if (ioctl(fPtr->fd, WSDISPLAYIO_LINEBYTES, &lb) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "ioctl WSDISPLAYIO_LINEBYTES: %s\n",
				   strerror(errno));
			return FALSE;
		}
		/* ok, fake up a new style fbinfo */
		fbi->fbi_width = info.width;
		fbi->fbi_height = info.height;
		fbi->fbi_stride = lb;
		fbi->fbi_bitsperpixel = info.depth;
		if (info.depth > 16) {
			fbi->fbi_pixeltype = WSFB_RGB;
			if (wstype == WSDISPLAY_TYPE_SUN24 ||
			    wstype == WSDISPLAY_TYPE_SUNCG12 ||
			    wstype == WSDISPLAY_TYPE_SUNCG14 ||
			    wstype == WSDISPLAY_TYPE_SUNTCX ||
			    wstype == WSDISPLAY_TYPE_SUNFFB ||
			    wstype == WSDISPLAY_TYPE_XVR1000 ||
			    wstype == WSDISPLAY_TYPE_VC4) {
				fbi->fbi_subtype.fbi_rgbmasks.red_offset = 0;
				fbi->fbi_subtype.fbi_rgbmasks.red_size = 8;
				fbi->fbi_subtype.fbi_rgbmasks.green_offset = 8;
				fbi->fbi_subtype.fbi_rgbmasks.green_size = 8;
				fbi->fbi_subtype.fbi_rgbmasks.blue_offset = 16;
				fbi->fbi_subtype.fbi_rgbmasks.blue_size = 8;
			} else {
				fbi->fbi_subtype.fbi_rgbmasks.red_offset = 16;
				fbi->fbi_subtype.fbi_rgbmasks.red_size = 8;
				fbi->fbi_subtype.fbi_rgbmasks.green_offset = 8;
				fbi->fbi_subtype.fbi_rgbmasks.green_size = 8;
				fbi->fbi_subtype.fbi_rgbmasks.blue_offset = 0;
				fbi->fbi_subtype.fbi_rgbmasks.blue_size = 8;
			}
			fbi->fbi_subtype.fbi_rgbmasks.alpha_offset = 0;
			fbi->fbi_subtype.fbi_rgbmasks.alpha_size = 0;
		} else if (info.depth <= 8) {
			fbi->fbi_pixeltype = WSFB_CI;
			fbi->fbi_subtype.fbi_cmapinfo.cmap_entries = info.cmsize;
		}
		fbi->fbi_flags = 0;
		fbi->fbi_fbsize = lb * info.height;
#ifdef	WSDISPLAY_TYPE_LUNA
		if (wstype == WSDISPLAY_TYPE_LUNA) {
			/*
			 * XXX
			 * LUNA's FB seems to have 64 dot (8 byte) offset.
			 * This might be able to be changed in kernel
			 * lunafb driver, but current setting was pulled
			 * from 4.4BSD-Lite2/luna68k.
			 */
			fbi->fbi_fboffset = 8;
		} else
#endif
			fbi->fbi_fboffset = 0;
	}
	xf86Msg(X_INFO, "fboffset %x\n", (int)fPtr->fbi.fbi_fboffset);
	/*
	 * Allocate room for saving the colormap.
	 */
	if (fPtr->fbi.fbi_pixeltype == WSFB_CI &&
	    fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries > 0) {
		fPtr->saved_cmap.red =
		    (unsigned char *)malloc(fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries);
		if (fPtr->saved_cmap.red == NULL) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			    "Cannot malloc %d bytes\n",
			    fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries);
			return FALSE;
		}
		fPtr->saved_cmap.green =
		    (unsigned char *)malloc(fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries);
		if (fPtr->saved_cmap.green == NULL) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			    "Cannot malloc %d bytes\n",
			    fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries);
			free(fPtr->saved_cmap.red);
			return FALSE;
		}
		fPtr->saved_cmap.blue =
		    (unsigned char *)malloc(fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries);
		if (fPtr->saved_cmap.blue == NULL) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			    "Cannot malloc %d bytes\n",
			    fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries);
			free(fPtr->saved_cmap.red);
			free(fPtr->saved_cmap.green);
			return FALSE;
		}
	}

	/* Handle depth */
	default_depth = fPtr->fbi.fbi_bitsperpixel <= 24 ? fPtr->fbi.fbi_bitsperpixel : 24;
	bitsperpixel = fPtr->fbi.fbi_bitsperpixel == 15 ? 16 : fPtr->fbi.fbi_bitsperpixel;
#if defined(__NetBSD__) && defined(WSDISPLAY_TYPE_LUNA)
	if (wstype == WSDISPLAY_TYPE_LUNA) {
		/*
		 * LUNA's color framebuffers support 4bpp or 8bpp
		 * but they have multiple 1bpp VRAM planes like ancient VGA.
		 */
#ifdef HAVE_SHADOW_AFB
		if (bitsperpixel == 8) {
			/*
			 * For 8bpp one, we can use the bitplane ops with
			 * shadow update proc as amiga.
			 */
			fPtr->planarAfb = TRUE;
		} else
#endif
		{
			/*
			 * For 4bpp one (or there is no planar support),
			 * just use only the first one plane
			 * as 1bpp monochrome server.
			 *
			 * Note OpenBSD/luna88k workarounds this by
			 * switching depth and palette settings by
			 * WSDISPLAYIO_SETGFXMODE ioctl.
			 */
			default_depth = 1;
			bitsperpixel = 1;
		}
	}
#endif
#ifdef WSDISPLAY_TYPE_AMIGACC
	if (wstype == WSDISPLAY_TYPE_AMIGACC) {
		/*
		 * Video memory is organized in bitplanes.
		 * 8bpp or 1bpp supported in this driver.
		 * With 8bpp conversion to bitplane format
		 * is done in shadow update proc.
		 * With 1bpp no conversion needed.
		 */
#ifdef HAVE_SHADOW_AFB
		if (bitsperpixel == 8) {
			fPtr->planarAfb = TRUE;
		} else
#endif
		{
			default_depth = 1;
			bitsperpixel = 1;
		}
	}
#endif

	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth,
		bitsperpixel,
		bitsperpixel >= 24 ? Support24bppFb|Support32bppFb : 0))
		return FALSE;

	/* Check consistency. */
	if (pScrn->bitsPerPixel != bitsperpixel) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		    "specified depth (%d) or bpp (%d) doesn't match "
		    "framebuffer depth (%d)\n", pScrn->depth,
		    pScrn->bitsPerPixel, bitsperpixel);
		return FALSE;
	}
	xf86PrintDepthBpp(pScrn);

	/* Get the depth24 pixmap format. */
	if (pScrn->depth == 24 && pix24bpp == 0)
		pix24bpp = xf86GetBppFromDepth(pScrn, 24);

	/* Handle options. */
	xf86CollectOptions(pScrn, NULL);
	fPtr->Options = (OptionInfoRec *)malloc(sizeof(WsfbOptions));
	if (fPtr->Options == NULL)
		return FALSE;
	memcpy(fPtr->Options, WsfbOptions, sizeof(WsfbOptions));
	xf86ProcessOptions(pScrn->scrnIndex, fPtr->pEnt->device->options,
			   fPtr->Options);

	/* Use shadow framebuffer by default, on depth >= 8 */
	xf86Msg(X_INFO, "fbi_flags: %x\n", fPtr->fbi.fbi_flags);
	if ((pScrn->depth >= 8) &&
	   ((fPtr->fbi.fbi_flags & WSFB_VRAM_IS_RAM) == 0)) {
		fPtr->shadowFB = xf86ReturnOptValBool(fPtr->Options,
						      OPTION_SHADOW_FB, TRUE);
	} else
		if (xf86ReturnOptValBool(fPtr->Options,
					 OPTION_SHADOW_FB, FALSE)) {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
				   "Shadow FB option ignored on depth < 8\n");
		}
	if (fPtr->fbi.fbi_flags & WSFB_VRAM_IS_SPLIT) {
		if (!fPtr->shadowFB) {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
				   "Shadow FB forced on for split framebuffer\n");
			fPtr->shadowFB = TRUE;
		}
	}

	fPtr->useRGB16ToYUY2 = FALSE;
#ifdef WSDISPLAY_TYPE_HOLLYWOOD
	if (wstype == WSDISPLAY_TYPE_HOLLYWOOD) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "Enabling RGB16->YUY2 conversion for Hollywood\n");
		fPtr->useRGB16ToYUY2 = TRUE;
		if (!fPtr->shadowFB) {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
				   "Shadow FB forced on for RGB16->YUY2 conversion\n");
			fPtr->shadowFB = TRUE;
		}
		/*
		 * Hollywood has a YUY2 framebuffer, but we treat it as
		 * RGB565 and convert with a custom shadowproc.
		 */
		fPtr->fbi.fbi_pixeltype = WSFB_RGB;
	}
#endif
	/* Rotation */
	fPtr->rotate = WSFB_ROTATE_NONE;
	if ((s = xf86GetOptValString(fPtr->Options, OPTION_ROTATE))) {
		if (pScrn->depth >= 8) {
			if (!xf86NameCmp(s, "CW")) {
				fPtr->shadowFB = TRUE;
				fPtr->rotate = WSFB_ROTATE_CW;
				xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				    "Rotating screen clockwise\n");
			} else if (!xf86NameCmp(s, "CCW")) {
				fPtr->shadowFB = TRUE;
				fPtr->rotate = WSFB_ROTATE_CCW;
				xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				    "Rotating screen counter clockwise\n");
			} else if (!xf86NameCmp(s, "UD")) {
				fPtr->shadowFB = TRUE;
				fPtr->rotate = WSFB_ROTATE_UD;
				xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				    "Rotating screen upside down\n");
			} else {
				xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				    "\"%s\" is not a valid value for Option "
				    "\"Rotate\"\n", s);
				xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				    "Valid options are \"CW\", \"CCW\","
				    " or \"UD\"\n");
			}
		} else {
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			    "Option \"Rotate\" ignored on depth < 8\n");
		}
	}


	fPtr->useSwap32 = FALSE;
	/* Color weight */
	if (fPtr->fbi.fbi_pixeltype == WSFB_RGB) {
		rgb zeros = { 0, 0, 0 }, masks;

		if (fPtr->fbi.fbi_subtype.fbi_rgbmasks.red_size > 0) {
			uint32_t msk;

			/*
			 * see if we need to byte-swap pixels
			 * XXX this requires a shadow FB and is incompatible
			 * (for now ) with rotation
			 */
			if ((fPtr->fbi.fbi_bitsperpixel == 32) &&
			    (fPtr->fbi.fbi_subtype.fbi_rgbmasks.blue_offset == 24) &&
			    (fPtr->rotate == WSFB_ROTATE_NONE) &&
			    (fPtr->shadowFB == TRUE)) {
			    	/*
			    	 * looks like BGRA - set the swap flag and flip
			    	 * the offsets
			    	 */
			    	xf86Msg(X_INFO, "endian-flipped RGB framebuffer "
			    			"detected, using WsfbShadowUpdateSwap32()\n");
			    	fPtr->fbi.fbi_subtype.fbi_rgbmasks.blue_offset = 0;
			    	fPtr->fbi.fbi_subtype.fbi_rgbmasks.green_offset = 8;
			    	fPtr->fbi.fbi_subtype.fbi_rgbmasks.red_offset = 16;
			    	fPtr->fbi.fbi_subtype.fbi_rgbmasks.alpha_offset = 24;
				fPtr->useSwap32 = TRUE;
			}

			msk = 0xffffffff;
			msk = msk << fPtr->fbi.fbi_subtype.fbi_rgbmasks.red_size;
			msk = ~msk;
			masks.red = msk << fPtr->fbi.fbi_subtype.fbi_rgbmasks.red_offset; 

			msk = 0xffffffff;
			msk = msk << fPtr->fbi.fbi_subtype.fbi_rgbmasks.green_size;
			msk = ~msk;
			masks.green = msk << fPtr->fbi.fbi_subtype.fbi_rgbmasks.green_offset; 

			msk = 0xffffffff;
			msk = msk << fPtr->fbi.fbi_subtype.fbi_rgbmasks.blue_size;
			msk = ~msk;
			masks.blue = msk << fPtr->fbi.fbi_subtype.fbi_rgbmasks.blue_offset; 
			xf86Msg(X_INFO, "masks generated: %08lx %08lx %08lx\n",
			    (unsigned long)masks.red,
			    (unsigned long)masks.green,
			    (unsigned long)masks.blue);
		} else {
			masks.red = 0;
			masks.green = 0;
			masks.blue = 0;
		}

		if (!xf86SetWeight(pScrn, zeros, masks))
			return FALSE;
	}

	/* Visual init */
	if (!xf86SetDefaultVisual(pScrn, -1))
		return FALSE;

	/* We don't currently support DirectColor at > 8bpp . */
	if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Given default visual"
			   " (%s) is not supported at depth %d\n",
			   xf86GetVisualName(pScrn->defaultVisual),
			   pScrn->depth);
		return FALSE;
	}

	xf86SetGamma(pScrn,zeros);

	pScrn->progClock = TRUE;
	pScrn->rgbBits   = (pScrn->depth >= 8) ? 8 : pScrn->depth;
	pScrn->chipset   = (char *)"wsfb";
	pScrn->videoRam  = fPtr->fbi.fbi_fbsize;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Vidmem: %dk\n",
		   pScrn->videoRam/1024);

	/* Fake video mode struct. */
	mode = (DisplayModePtr)malloc(sizeof(DisplayModeRec));
	mode->prev = mode;
	mode->next = mode;
	mode->name = (char *)"wsfb current mode";
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
	pScrn->displayWidth = pScrn->virtualX;

	/* Set the display resolution. */
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


	/* Load shadow if needed. */
	if (fPtr->shadowFB) {
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
			   "Using \"Shadow Framebuffer\"\n");
		if (xf86LoadSubModule(pScrn, "shadow") == NULL) {
			WsfbFreeRec(pScrn);
			return FALSE;
		}
	}
	if (xf86LoadSubModule(pScrn, "fb") == NULL) {
		WsfbFreeRec(pScrn);
		return FALSE;
	}

	if (xf86LoadSubModule(pScrn, "ramdac") == NULL) {
		WsfbFreeRec(pScrn);
		return FALSE;
        }

	TRACE_EXIT("PreInit");
	return TRUE;
}

static void
wsfbUpdateRotatePacked(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    shadowUpdateRotatePacked(pScreen, pBuf);
}

static void
wsfbUpdatePacked(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    shadowUpdatePacked(pScreen, pBuf);
}

static Bool
WsfbCreateScreenResources(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	WsfbPtr fPtr = WSFBPTR(pScrn);
	PixmapPtr pPixmap;
	Bool ret;
	void (*shadowproc)(ScreenPtr, shadowBufPtr);
	ShadowWindowProc windowproc = WsfbWindowLinear;

	pScreen->CreateScreenResources = fPtr->CreateScreenResources;
	ret = pScreen->CreateScreenResources(pScreen);
	pScreen->CreateScreenResources = WsfbCreateScreenResources;

	if (!ret)
		return FALSE;

	pPixmap = pScreen->GetScreenPixmap(pScreen);
	if (fPtr->fbi.fbi_flags & WSFB_VRAM_IS_SPLIT) {
		shadowproc = WsfbShadowUpdateSplit;
	} else if (fPtr->useRGB16ToYUY2) {
		/* Build RGB16 to Y, U, and V lookup tables */
		if (mapRGB16ToY == NULL) {
			mapRGB16ToY = malloc(0x30000);
			if (mapRGB16ToY == NULL) {
				xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				    "Cannot malloc %d bytes for RGB16->YUY2\n",
				    0x30000);
				return FALSE;
			}
			mapRGB16ToU = mapRGB16ToY + 0x10000;
			mapRGB16ToV = mapRGB16ToY + 0x20000;
        		for (unsigned int n = 0; n < 0x10000; n++) {
                		/* RGB565 values, scaled to 8 bits */
                		const double R = (((n >> 11) & 0x1f) * 255) / 31;
                		const double G = (((n >> 5) & 0x3f) * 255) / 63;
                		const double B = (((n >> 0) & 0x1f) * 255) / 31;

				/* Convert to YUV */
	                	mapRGB16ToY[n] =
				    0.257 * R + 0.504 * G + 0.098 * B +  16;
	                	mapRGB16ToU[n] =
				   -0.148 * R - 0.291 * G + 0.439 * B + 128;
	                	mapRGB16ToV[n] =
				    0.439 * R - 0.368 * G - 0.071 * B + 128;
			}
		}
        
		shadowproc = WsfbShadowUpdateRGB16ToYUY2;
	} else if (fPtr->useSwap32) {
		shadowproc = WsfbShadowUpdateSwap32;
	} else if (fPtr->rotate) {
		shadowproc = wsfbUpdateRotatePacked;
	} else
#ifdef HAVE_SHADOW_AFB
	if (fPtr->planarAfb) {
		shadowproc = shadowUpdateAfb8;
		windowproc = WsfbWindowAfb;
	} else
#endif
	{
		shadowproc = wsfbUpdatePacked;
	}
	
	if (!shadowAdd(pScreen, pPixmap, shadowproc,
		windowproc, fPtr->rotate, NULL)) {
		return FALSE;
	}
	return TRUE;
}


static Bool
WsfbShadowInit(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	WsfbPtr fPtr = WSFBPTR(pScrn);

	if (!shadowSetup(pScreen))
		return FALSE;
	fPtr->CreateScreenResources = pScreen->CreateScreenResources;
	pScreen->CreateScreenResources = WsfbCreateScreenResources;

	return TRUE;
}

static Bool
WsfbScreenInit(SCREEN_INIT_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	WsfbPtr fPtr = WSFBPTR(pScrn);
	VisualPtr visual;
	int ret, flags, ncolors;
	int wsmode = WSDISPLAYIO_MODE_DUMBFB;
	int wstype;
	int width;
	size_t len;

	TRACE_ENTER("WsfbScreenInit");
#if DEBUG
	ErrorF("\tbitsPerPixel=%d, depth=%d, defaultVisual=%s\n"
	       "\tmask: %x,%x,%x, offset: %u,%u,%u\n",
	       pScrn->bitsPerPixel,
	       pScrn->depth,
	       xf86GetVisualName(pScrn->defaultVisual),
	       pScrn->mask.red,pScrn->mask.green,pScrn->mask.blue,
	       pScrn->offset.red,pScrn->offset.green,pScrn->offset.blue);
#endif
	switch (fPtr->fbi.fbi_bitsperpixel) {
	case 1:
	case 4:
	case 8:
		len = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
#ifdef HAVE_SHADOW_AFB
		if (fPtr->planarAfb) {
			/*
			 * stride is "bytes per line" for each plane so
			 * we need a number of planes to mmap in planar case.
			 */
			len *= fPtr->fbi.fbi_bitsperpixel;
		}
#endif

		break;
	case 15:
	case 16:
		if (fPtr->fbi.fbi_stride == fPtr->fbi.fbi_width) {
			len = fPtr->fbi.fbi_width * fPtr->fbi.fbi_height * sizeof(short);
		} else {
			len = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
		}
		break;
	case 24:
		if (fPtr->fbi.fbi_stride == fPtr->fbi.fbi_width) {
			len = fPtr->fbi.fbi_width * fPtr->fbi.fbi_height * 3;
		} else {
			len = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
		}
		break;
	case 32:
		if (fPtr->fbi.fbi_stride == fPtr->fbi.fbi_width) {
			len = fPtr->fbi.fbi_width * fPtr->fbi.fbi_height * sizeof(int);
		} else {
			len = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
		}
		break;
	default:
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "unsupported depth %d\n", fPtr->fbi.fbi_bitsperpixel);
		return FALSE;
	}
	/* Switch to graphics mode - required before mmap. */
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &wsmode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAYIO_SMODE: %s\n",
			   strerror(errno));
		return FALSE;
	}
	/* Get wsdisplay type to handle quirks */
	if (ioctl(fPtr->fd, WSDISPLAYIO_GTYPE, &wstype) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ioctl WSDISPLAY_GTYPE: %s\n",
			   strerror(errno));
		return FALSE;
	}
	len = max(len, fPtr->fbi.fbi_fbsize);
	fPtr->fbmem = wsfb_mmap(len + fPtr->fbi.fbi_fboffset, 0, fPtr->fd);

	if (fPtr->fbmem == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "wsfb_mmap: %s\n", strerror(errno));
		return FALSE;
	}
	fPtr->fbmem_len = len;

	WsfbSave(pScrn);
	pScrn->vtSema = TRUE;

	/* MI layer */
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

	if (fPtr->rotate == WSFB_ROTATE_CW
	    || fPtr->rotate == WSFB_ROTATE_CCW) {
		int tmp = pScrn->virtualX;
		pScrn->virtualX = pScrn->displayWidth = pScrn->virtualY;
		pScrn->virtualY = tmp;
	}
	if (fPtr->rotate && !fPtr->PointerMoved) {
		fPtr->PointerMoved = pScrn->PointerMoved;
		pScrn->PointerMoved = WsfbPointerMoved;
	}

	fPtr->fbstart = fPtr->fbmem + fPtr->fbi.fbi_fboffset;

	if (fPtr->shadowFB) {
		if (fPtr->rotate) {
			/*
			 * Note Rotate and Shadow FB options are valid
			 * only on depth >= 8.
			 */
			len = pScrn->virtualX * pScrn->virtualY *
			    (pScrn->bitsPerPixel >> 3);
		} else
#ifdef HAVE_SHADOW_AFB
		if (fPtr->planarAfb) {
			/* always 8bpp */
			len = pScrn->virtualX * pScrn->virtualY;
		} else
#endif
		{
			len = fPtr->fbi.fbi_stride * pScrn->virtualY;
		}
		fPtr->shadow = calloc(1, len);

		if (!fPtr->shadow) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			    "Failed to allocate shadow framebuffer\n");
			return FALSE;
		}
	}

	/*
	 * fbScreenInit() seems to require "pixel width of frame buffer"
	 * but it is actually "stride in pixel" of frame buffer,
	 * per xorg/xserver/tree/fb/fbscreen.c.
	 */
	if (fPtr->rotate) {
		width = pScrn->displayWidth;
	} else
#ifdef HAVE_SHADOW_AFB
	if (fPtr->planarAfb) {
		width = pScrn->displayWidth;
	} else
#endif
	{
		if (pScrn->bitsPerPixel > 8) {
			width =
			    fPtr->fbi.fbi_stride / (pScrn->bitsPerPixel >> 3);
		} else {
			width =
			    fPtr->fbi.fbi_stride * (8 / pScrn->bitsPerPixel);
		}
	}
	switch (pScrn->bitsPerPixel) {
	case 1:
		ret = fbScreenInit(pScreen,
		    fPtr->fbstart,
		    pScrn->virtualX, pScrn->virtualY,
		    pScrn->xDpi, pScrn->yDpi,
		    width, pScrn->bitsPerPixel);
		break;
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
		ret = fbScreenInit(pScreen,
		    fPtr->shadowFB ? fPtr->shadow : fPtr->fbstart,
		    pScrn->virtualX, pScrn->virtualY,
		    pScrn->xDpi, pScrn->yDpi,
		    width,
		    pScrn->bitsPerPixel);
		break;
	default:
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Unsupported bpp: %d\n", pScrn->bitsPerPixel);
		return FALSE;
	} /* case */

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

	if (pScrn->bitsPerPixel >= 8) {
		if (!fbPictureInit(pScreen, NULL, 0))
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
				   "RENDER extension initialisation failed.\n");
	}
	if (fPtr->shadowFB && !WsfbShadowInit(pScreen)) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		    "shadow framebuffer initialization failed\n");
		return FALSE;
	}

#ifdef XFreeXDGA
	if (!fPtr->rotate)
		WsfbDGAInit(pScrn, pScreen);
	else
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Rotated display, "
		    "disabling DGA\n");
#endif
	if (fPtr->rotate) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		    "Enabling Driver Rotation, " "disabling RandR\n");
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) < 24
		xf86DisableRandR();
#endif
		if (pScrn->bitsPerPixel == 24)
			xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			    "Rotation might be broken in 24 bpp\n");
	}

	xf86SetBlackWhitePixels(pScreen);
	xf86SetBackingStore(pScreen);

	/* Software cursor. */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	/* check for hardware cursor support */
	if (fPtr->HWCursor)
		WsfbSetupCursor(pScreen);

	/*
	 * Colormap
	 *
	 * Note that, even on less than 8 bit depth frame buffers, we
	 * expect the colormap to be programmable with 8 bit values.
	 * As of now, this is indeed the case on all OpenBSD supported
	 * graphics hardware.
	 */
	if (!miCreateDefColormap(pScreen))
		return FALSE;
	flags = CMAP_RELOAD_ON_MODE_SWITCH;

	ncolors = 0;
	if (fPtr->fbi.fbi_pixeltype == WSFB_CI) {
		ncolors = fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries;
	}

	/* On StaticGray visuals, fake a 256 entries colormap. */
	if (ncolors == 0)
		ncolors = 256;
	if(!xf86HandleColormaps(pScreen, ncolors, 8, WsfbLoadPalette,
				NULL, flags))
		return FALSE;

#if defined(__NetBSD__) && defined(WSDISPLAY_TYPE_LUNA)
	if (wstype == WSDISPLAY_TYPE_LUNA) {
		ncolors = fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries;
		if (ncolors > 0
#ifdef HAVE_SHADOW_AFB
		    && !fPtr->planarAfb
#endif
		) {
			/*
			 * Override palette to use 4bpp/8bpp framebuffers as
			 * monochrome server by using only the first plane.
			 * See also comment in WsfbPreInit().
			 */
			struct wsdisplay_cmap cmap;
			uint8_t r[256], g[256], b[256];
			int p;

			for (p = 0; p < ncolors; p++)
				r[p] = g[p] = b[p] = (p & 1) ? 0xff : 0;
			cmap.index = 0;
			cmap.count = ncolors;
			cmap.red   = r;
			cmap.green = g;
			cmap.blue  = b;
			if (ioctl(fPtr->fd, WSDISPLAYIO_PUTCMAP, &cmap) == -1) {
				xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "ioctl WSDISPLAYIO_PUTCMAP: %s\n",
				   strerror(errno));
			}
		}
	}
#endif

	pScreen->SaveScreen = WsfbSaveScreen;

#ifdef XvExtension
	{
		XF86VideoAdaptorPtr *ptr;

		int n = xf86XVListGenericAdaptors(pScrn,&ptr);
		if (n) {
			xf86XVScreenInit(pScreen,ptr,n);
		}
	}
#endif

	/* Wrap the current CloseScreen function. */
	fPtr->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = WsfbCloseScreen;

	TRACE_EXIT("WsfbScreenInit");
	return TRUE;
}

static Bool
WsfbCloseScreen(CLOSE_SCREEN_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	PixmapPtr pPixmap;
	WsfbPtr fPtr = WSFBPTR(pScrn);


	TRACE_ENTER("WsfbCloseScreen");

	pPixmap = pScreen->GetScreenPixmap(pScreen);
	if (fPtr->shadowFB)
		shadowRemove(pScreen, pPixmap);

	if (pScrn->vtSema) {
		WsfbRestore(pScrn);
		if (munmap(fPtr->fbmem, fPtr->fbmem_len + fPtr->fbi.fbi_fboffset) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "munmap: %s\n", strerror(errno));
		}

		fPtr->fbmem = NULL;
	}
#ifdef XFreeXDGA
	if (fPtr->pDGAMode) {
		free(fPtr->pDGAMode);
		fPtr->pDGAMode = NULL;
		fPtr->nDGAMode = 0;
	}
#endif
	pScrn->vtSema = FALSE;

	/* Unwrap CloseScreen. */
	pScreen->CloseScreen = fPtr->CloseScreen;
	TRACE_EXIT("WsfbCloseScreen");
	return (*pScreen->CloseScreen)(CLOSE_SCREEN_ARGS);
}

static void *
WsfbWindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		CARD32 *size, void *closure)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	WsfbPtr fPtr = WSFBPTR(pScrn);

	if (fPtr->fbi.fbi_stride)
		*size = fPtr->fbi.fbi_stride;
	else {
		if (ioctl(fPtr->fd, WSDISPLAYIO_LINEBYTES, size) == -1)
			return NULL;
		fPtr->fbi.fbi_stride = *size;
	}
	return ((CARD8 *)fPtr->fbstart + row * fPtr->fbi.fbi_stride + offset);
}

#ifdef HAVE_SHADOW_AFB
/*
 * For use with shadowUpdateAfb8
 *
 * For video memory layout with non-interleaved bitplanes.
 */
static void *
WsfbWindowAfb(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		CARD32 *size, void *closure)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	WsfbPtr fPtr = WSFBPTR(pScrn);

	/* size is offset from start of bitplane to next bitplane */
	*size = fPtr->fbi.fbi_stride * fPtr->fbi.fbi_height;
	return ((CARD8 *)fPtr->fbstart + row * fPtr->fbi.fbi_stride + offset);
}
#endif

static void
WsfbPointerMoved(SCRN_ARG_TYPE arg, int x, int y)
{
    SCRN_INFO_PTR(arg);
    WsfbPtr fPtr = WSFBPTR(pScrn);
    int newX, newY;

    switch (fPtr->rotate)
    {
    case WSFB_ROTATE_CW:
	/* 90 degrees CW rotation. */
	newX = pScrn->pScreen->height - y - 1;
	newY = x;
	break;

    case WSFB_ROTATE_CCW:
	/* 90 degrees CCW rotation. */
	newX = y;
	newY = pScrn->pScreen->width - x - 1;
	break;

    case WSFB_ROTATE_UD:
	/* 180 degrees UD rotation. */
	newX = pScrn->pScreen->width - x - 1;
	newY = pScrn->pScreen->height - y - 1;
	break;

    default:
	/* No rotation. */
	newX = x;
	newY = y;
	break;
    }

    /* Pass adjusted pointer coordinates to wrapped PointerMoved function. */
    (*fPtr->PointerMoved)(arg, newX, newY);
}

static Bool
WsfbEnterVT(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	WsfbPtr fPtr = WSFBPTR(pScrn);
	int mode;

	TRACE_ENTER("EnterVT");
	pScrn->vtSema = TRUE;

	/* Restore the graphics mode. */
	mode = WSDISPLAYIO_MODE_DUMBFB;
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error setting graphics mode %s\n", strerror(errno));
	}

	TRACE_EXIT("EnterVT");
	return TRUE;
}

static void
WsfbLeaveVT(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	WsfbPtr fPtr = WSFBPTR(pScrn);
	int mode;

	TRACE_ENTER("LeaveVT");

	/*
	 * stuff to do:
	 * - turn off hw cursor
	 * - restore colour map if WSFB_CI
	 * - ioctl(WSDISPLAYIO_MODE_EMUL) to notify the kernel driver that
	 *   we're backing off
	 */

	if (fPtr->fbi.fbi_pixeltype == WSFB_CI &&
	    fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries > 0) {
		/* reset colormap for text mode */
		if (ioctl(fPtr->fd, WSDISPLAYIO_PUTCMAP,
			  &(fPtr->saved_cmap)) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "error restoring colormap %s\n",
				   strerror(errno));
		}
	}

	/* Restore the text mode. */
	mode = WSDISPLAYIO_MODE_EMUL;
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error setting text mode %s\n", strerror(errno));
	}

	pScrn->vtSema = FALSE;
	TRACE_EXIT("LeaveVT");
}

static Bool
WsfbSwitchMode(SWITCH_MODE_ARGS_DECL)
{
#if DEBUG
	SCRN_INFO_PTR(arg);
#endif

	TRACE_ENTER("SwitchMode");
	/* Nothing else to do. */
	return TRUE;
}

static int
WsfbValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode, Bool verbose, int flags)
{
#if DEBUG
	SCRN_INFO_PTR(arg);
#endif

	TRACE_ENTER("ValidMode");
	return MODE_OK;
}

static void
WsfbLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
	       LOCO *colors, VisualPtr pVisual)
{
	WsfbPtr fPtr = WSFBPTR(pScrn);
	struct wsdisplay_cmap cmap;
	unsigned char red[256],green[256],blue[256];
	int i, indexMin=256, indexMax=0;

	TRACE_ENTER("LoadPalette");

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
	TRACE_EXIT("LoadPalette");
}

static Bool
WsfbSaveScreen(ScreenPtr pScreen, int mode)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	WsfbPtr fPtr = WSFBPTR(pScrn);
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
	TRACE_EXIT("SaveScreen");
	return TRUE;
}


static void
WsfbSave(ScrnInfoPtr pScrn)
{
	WsfbPtr fPtr = WSFBPTR(pScrn);

	TRACE_ENTER("WsfbSave");

	/* nothing to save if we don't run in colour-indexed mode */
	if (fPtr->fbi.fbi_pixeltype != WSFB_CI)
		return;

	/* nothing to do if no color palette support */
	if (fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries == 0)
		return;

	fPtr->saved_cmap.index = 0;
	fPtr->saved_cmap.count = fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries;
	if (ioctl(fPtr->fd, WSDISPLAYIO_GETCMAP,
		  &(fPtr->saved_cmap)) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error saving colormap %s\n", strerror(errno));
	}
	TRACE_EXIT("WsfbSave");

}

static void
WsfbRestore(ScrnInfoPtr pScrn)
{
	WsfbPtr fPtr = WSFBPTR(pScrn);
	int mode;

	TRACE_ENTER("WsfbRestore");

	if (fPtr->fbi.fbi_pixeltype == WSFB_CI &&
	    fPtr->fbi.fbi_subtype.fbi_cmapinfo.cmap_entries > 0) {
		/* reset colormap for text mode */
		if (ioctl(fPtr->fd, WSDISPLAYIO_PUTCMAP,
			  &(fPtr->saved_cmap)) == -1) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "error restoring colormap %s\n",
				   strerror(errno));
		}
	}

	/* Clear the screen. */
	memset(fPtr->fbstart, 0, fPtr->fbmem_len);

	/* Restore the text mode. */
	mode = WSDISPLAYIO_MODE_EMUL;
	if (ioctl(fPtr->fd, WSDISPLAYIO_SMODE, &mode) == -1) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "error setting text mode %s\n", strerror(errno));
	}
	TRACE_EXIT("WsfbRestore");
}

#ifdef XFreeXDGA
/***********************************************************************
 * DGA stuff
 ***********************************************************************/

static Bool
WsfbDGAOpenFramebuffer(ScrnInfoPtr pScrn, char **DeviceName,
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
WsfbDGASetMode(ScrnInfoPtr pScrn, DGAModePtr pDGAMode)
{
	DisplayModePtr pMode;
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

	if (!(*pScrn->SwitchMode)(SWITCH_MODE_ARGS(pScrn, pMode)))
		return FALSE;
	(*pScrn->AdjustFrame)(ADJUST_FRAME_ARGS(pScrn, frameX0, frameY0));

	return TRUE;
}

static void
WsfbDGASetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
	(*pScrn->AdjustFrame)(ADJUST_FRAME_ARGS(pScrn, x, y));
}

static int
WsfbDGAGetViewport(ScrnInfoPtr pScrn)
{
	return (0);
}

static DGAFunctionRec WsfbDGAFunctions =
{
	WsfbDGAOpenFramebuffer,
	NULL,       /* CloseFramebuffer */
	WsfbDGASetMode,
	WsfbDGASetViewport,
	WsfbDGAGetViewport,
	NULL,       /* Sync */
	NULL,       /* FillRect */
	NULL,       /* BlitRect */
	NULL,       /* BlitTransRect */
};

static void
WsfbDGAAddModes(ScrnInfoPtr pScrn)
{
	WsfbPtr fPtr = WSFBPTR(pScrn);
	DisplayModePtr pMode = pScrn->modes;
	DGAModePtr pDGAMode;

	do {
		pDGAMode = realloc(fPtr->pDGAMode,
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

		if (fPtr->fbi.fbi_stride)
			pDGAMode->bytesPerScanline = fPtr->fbi.fbi_stride;
		else {
			ioctl(fPtr->fd, WSDISPLAYIO_LINEBYTES,
			      &fPtr->fbi.fbi_stride);
			pDGAMode->bytesPerScanline = fPtr->fbi.fbi_stride;
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
WsfbDGAInit(ScrnInfoPtr pScrn, ScreenPtr pScreen)
{
	WsfbPtr fPtr = WSFBPTR(pScrn);

	if (pScrn->depth < 8)
		return FALSE;

	if (!fPtr->nDGAMode)
		WsfbDGAAddModes(pScrn);

	return (DGAInit(pScreen, &WsfbDGAFunctions,
			fPtr->pDGAMode, fPtr->nDGAMode));
}
#endif

static Bool
WsfbDriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
    pointer ptr)
{
	xorgHWFlags *flag;

	switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
		flag = (CARD32*)ptr;
		(*flag) = 0;
		return TRUE;
	default:
		return FALSE;
	}
}

static inline void
WsfbCopyRGB16ToYUY2(void *dest, void *src, int len)
{
	uint16_t *src16 = src;
	uint32_t *dest32 = dest;

	while (len > 0) {
		const uint16_t rgb0 = src16[0];
		const uint16_t rgb1 = src16[1];
		const uint16_t rgb = ((rgb0 >> 1) & ~0x8410) +
				     ((rgb1 >> 1) & ~0x8410) +
				     ((rgb0 & rgb1) & 0x0841);
		const uint32_t y0 = mapRGB16ToY[rgb0];
		const uint32_t y1 = mapRGB16ToY[rgb1];
		const uint32_t u = mapRGB16ToU[rgb];
		const uint32_t v = mapRGB16ToV[rgb];

		*dest32 = (y0 << 24) | (u << 16) | (y1 << 8) | v;

		dest32++;
		src16 += 2;
		len -= 4;
	}
}

void
WsfbShadowUpdateRGB16ToYUY2(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    RegionPtr	damage = DamageRegion (pBuf->pDamage);
    PixmapPtr	pShadow = pBuf->pPixmap;
    int		nbox = RegionNumRects (damage);
    BoxPtr	pbox = RegionRects (damage);
    FbBits	*shaBase, *shaLine, *sha;
    FbStride	shaStride;
    int		scrBase, scrLine, scr;
    int		shaBpp;
    int		shaXoff, shaYoff; /* XXX assumed to be zero */
    int		x, y, w, h, width;
    int         i;
    FbBits	*winBase = NULL, *win;
    CARD32      winSize;

    fbGetDrawable (&pShadow->drawable, shaBase, shaStride, shaBpp, shaXoff, shaYoff);
    while (nbox--)
    {
	x = pbox->x1 * shaBpp;
	y = pbox->y1;
	w = (pbox->x2 - pbox->x1) * shaBpp;
	h = pbox->y2 - pbox->y1;

	scrLine = (x >> FB_SHIFT);
	shaLine = shaBase + y * shaStride + (x >> FB_SHIFT);
				   
	x &= FB_MASK;
	w = (w + x + FB_MASK) >> FB_SHIFT;
	
	while (h--)
	{
	    winSize = 0;
	    scrBase = 0;
	    width = w;
	    scr = scrLine;
	    sha = shaLine;
	    while (width) {
		/* how much remains in this window */
		i = scrBase + winSize - scr;
		if (i <= 0 || scr < scrBase)
		{
		    winBase = (FbBits *) (*pBuf->window) (pScreen,
							  y,
							  scr * sizeof (FbBits),
							  SHADOW_WINDOW_WRITE,
							  &winSize,
							  pBuf->closure);
		    if(!winBase)
			return;
		    scrBase = scr;
		    winSize /= sizeof (FbBits);
		    i = winSize;
		}
		win = winBase + (scr - scrBase);
		if (i > width)
		    i = width;
		width -= i;
		scr += i;
		WsfbCopyRGB16ToYUY2(win, sha, i * sizeof(FbBits));
		sha += i;
	    }
	    shaLine += shaStride;
	    y++;
	}
	pbox++;
    }
}

static inline void
memcpy32sw(void *dest, void *src, int len)
{
	uint32_t *d = dest, *s = src;

#if DEBUG
	if ((((long)dest & 3) + ((long)src & 3) + (len & 3)) != 0) {
		xf86Msg(X_ERROR, "unaligned %s\n", __func__);
		return;
	}
#endif
	while (len > 0) {
		*d = bswap32(*s);
		d++;
		s++;
		len -= 4;
	}
}

/* adapted from miext/shadow/shpacked.c::shadowUpdatePacked() */
void
WsfbShadowUpdateSwap32(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    RegionPtr	damage = DamageRegion (pBuf->pDamage);
    PixmapPtr	pShadow = pBuf->pPixmap;
    int		nbox = RegionNumRects (damage);
    BoxPtr	pbox = RegionRects (damage);
    FbBits	*shaBase, *shaLine, *sha;
    FbStride	shaStride;
    int		scrBase, scrLine, scr;
    int		shaBpp;
    int		shaXoff, shaYoff; /* XXX assumed to be zero */
    int		x, y, w, h, width;
    int         i;
    FbBits	*winBase = NULL, *win;
    CARD32      winSize;

    fbGetDrawable (&pShadow->drawable, shaBase, shaStride, shaBpp, shaXoff, shaYoff);
    while (nbox--)
    {
	x = pbox->x1 * shaBpp;
	y = pbox->y1;
	w = (pbox->x2 - pbox->x1) * shaBpp;
	h = pbox->y2 - pbox->y1;

	scrLine = (x >> FB_SHIFT);
	shaLine = shaBase + y * shaStride + (x >> FB_SHIFT);
				   
	x &= FB_MASK;
	w = (w + x + FB_MASK) >> FB_SHIFT;
	
	while (h--)
	{
	    winSize = 0;
	    scrBase = 0;
	    width = w;
	    scr = scrLine;
	    sha = shaLine;
	    while (width) {
		/* how much remains in this window */
		i = scrBase + winSize - scr;
		if (i <= 0 || scr < scrBase)
		{
		    winBase = (FbBits *) (*pBuf->window) (pScreen,
							  y,
							  scr * sizeof (FbBits),
							  SHADOW_WINDOW_WRITE,
							  &winSize,
							  pBuf->closure);
		    if(!winBase)
			return;
		    scrBase = scr;
		    winSize /= sizeof (FbBits);
		    i = winSize;
		}
		win = winBase + (scr - scrBase);
		if (i > width)
		    i = width;
		width -= i;
		scr += i;
		memcpy32sw(win, sha, i * sizeof(FbBits));
		sha += i;
	    }
	    shaLine += shaStride;
	    y++;
	}
	pbox++;
    }
}

void
WsfbShadowUpdateSplit(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    WsfbPtr 	fPtr = WSFBPTR(pScrn);
    RegionPtr	damage = DamageRegion (pBuf->pDamage);
    PixmapPtr	pShadow = pBuf->pPixmap;
    int		nbox = RegionNumRects (damage);
    BoxPtr	pbox = RegionRects (damage);
    FbBits	*shaBase, *shaLine, *sha;
    FbStride	shaStride;
    int		scrBase, scrLine, scr;
    int		shaBpp;
    int		shaXoff, shaYoff; /* XXX assumed to be zero */
    int		x, y, w, h, width;
    int         i;
    FbBits	*winBase = NULL, *win, *win2;
    unsigned long split = fPtr->fbi.fbi_fbsize / 2; 
    CARD32      winSize;

    fbGetDrawable (&pShadow->drawable, shaBase, shaStride, shaBpp, shaXoff, shaYoff);
    while (nbox--)
    {
	x = pbox->x1 * shaBpp;
	y = pbox->y1;
	w = (pbox->x2 - pbox->x1) * shaBpp;
	h = pbox->y2 - pbox->y1;

	scrLine = (x >> FB_SHIFT);
	shaLine = shaBase + y * shaStride + (x >> FB_SHIFT);
				   
	x &= FB_MASK;
	w = (w + x + FB_MASK) >> FB_SHIFT;
	
	while (h--)
	{
	    winSize = 0;
	    scrBase = 0;
	    width = w;
	    scr = scrLine;
	    sha = shaLine;
	    while (width) {
		/* how much remains in this window */
		i = scrBase + winSize - scr;
		if (i <= 0 || scr < scrBase)
		{
		    winBase = (FbBits *) (*pBuf->window) (pScreen,
							  y,
							  scr * sizeof (FbBits),
							  SHADOW_WINDOW_WRITE,
							  &winSize,
							  pBuf->closure);
		    if(!winBase)
			return;
		    scrBase = scr;
		    winSize /= sizeof (FbBits);
		    i = winSize;
		}
		win = winBase + (scr - scrBase);
		win2 = (FbBits *)(split + (unsigned long)win);
		if (i > width)
		    i = width;
		width -= i;
		scr += i;
		memcpy(win, sha, i * sizeof(FbBits));
		memcpy(win2, sha, i * sizeof(FbBits));
		sha += i;
	    }
	    shaLine += shaStride;
	    y++;
	}
	pbox++;
    }
}
