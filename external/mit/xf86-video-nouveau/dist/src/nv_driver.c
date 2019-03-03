/*
 * Copyright 1996-1997 David J. McKay
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
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <fcntl.h>

#include "nv_include.h"

#include "xorg-server.h"
#include "xf86drm.h"
#include "xf86drmMode.h"
#include "nouveau_drm.h"
#ifdef DRI2
#include "dri2.h"
#endif

#include "nouveau_copy.h"
#include "nouveau_present.h"
#include "nouveau_sync.h"

#if !HAVE_XORG_LIST
#define xorg_list_is_empty              list_is_empty
#define xorg_list_for_each_entry        list_for_each_entry
#endif

/*
 * Forward definitions for the functions that make up the driver.
 */
/* Mandatory functions */
static const OptionInfoRec * NVAvailableOptions(int chipid, int busid);
static void    NVIdentify(int flags);
static Bool    NVPreInit(ScrnInfoPtr pScrn, int flags);
static Bool    NVScreenInit(SCREEN_INIT_ARGS_DECL);
static Bool    NVEnterVT(VT_FUNC_ARGS_DECL);
static void    NVLeaveVT(VT_FUNC_ARGS_DECL);
static Bool    NVCloseScreen(CLOSE_SCREEN_ARGS_DECL);
static Bool    NVSaveScreen(ScreenPtr pScreen, int mode);
static void    NVCloseDRM(ScrnInfoPtr);

/* Optional functions */
static Bool    NVDriverFunc(ScrnInfoPtr scrn, xorgDriverFuncOp op,
			    void *data);
static Bool    NVSwitchMode(SWITCH_MODE_ARGS_DECL);
static void    NVAdjustFrame(ADJUST_FRAME_ARGS_DECL);
static void    NVFreeScreen(FREE_SCREEN_ARGS_DECL);

/* Internally used functions */

static Bool	NVMapMem(ScrnInfoPtr pScrn);
static Bool	NVUnmapMem(ScrnInfoPtr pScrn);

#define NOUVEAU_PCI_DEVICE(_vendor_id, _device_id)                             \
	{ (_vendor_id), (_device_id), PCI_MATCH_ANY, PCI_MATCH_ANY,            \
	  0x00030000, 0x00ff0000, 0 }

static const struct pci_id_match nouveau_device_match[] = {
	NOUVEAU_PCI_DEVICE(0x12d2, PCI_MATCH_ANY),
	NOUVEAU_PCI_DEVICE(0x10de, PCI_MATCH_ANY),
	{ 0, 0, 0 },
};

static Bool NVPciProbe (	DriverPtr 		drv,
				int 			entity_num,
				struct pci_device	*dev,
				intptr_t		match_data	);

#ifdef XSERVER_PLATFORM_BUS
static Bool NVPlatformProbe(DriverPtr driver,
				int entity_num, int flags,
				struct xf86_platform_device *dev,
				intptr_t dev_match_data);
#endif

_X_EXPORT int NVEntityIndex = -1;

static int getNVEntityIndex(void)
{
	return NVEntityIndex;
}

/*
 * This contains the functions needed by the server after loading the
 * driver module.  It must be supplied, and gets added the driver list by
 * the Module Setup funtion in the dynamic case.  In the static case a
 * reference to this is compiled in, and this requires that the name of
 * this DriverRec be an upper-case version of the driver name.
 */

_X_EXPORT DriverRec NV = {
	NV_VERSION,
	NV_DRIVER_NAME,
	NVIdentify,
	NULL,
	NVAvailableOptions,
	NULL,
	0,
	NVDriverFunc,
	nouveau_device_match,
	NVPciProbe,
#ifdef XSERVER_PLATFORM_BUS
	NVPlatformProbe,
#endif
};

struct NvFamily
{
  char *name;
  char *chipset;
};

static struct NvFamily NVKnownFamilies[] =
{
  { "RIVA TNT",    "NV04" },
  { "RIVA TNT2",   "NV05" },
  { "GeForce 256", "NV10" },
  { "GeForce 2",   "NV11, NV15" },
  { "GeForce 4MX", "NV17, NV18" },
  { "GeForce 3",   "NV20" },
  { "GeForce 4Ti", "NV25, NV28" },
  { "GeForce FX",  "NV3x" },
  { "GeForce 6",   "NV4x" },
  { "GeForce 7",   "G7x" },
  { "GeForce 8",   "G8x" },
  { "GeForce 9",   "G9x" },
  { "GeForce GTX 2xx/3xx", "GT2xx" },
  { "GeForce GTX 4xx/5xx", "GFxxx" },
  { "GeForce GTX 6xx/7xx", "GKxxx" },
  { "GeForce GTX 9xx", "GMxxx" },
  { "GeForce GTX 10xx", "GPxxx" },
  { NULL, NULL}
};

static MODULESETUPPROTO(nouveauSetup);

static XF86ModuleVersionInfo nouveauVersRec =
{
    "nouveau",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,                     /* This is a video driver */
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0,0,0,0}
};

_X_EXPORT XF86ModuleData nouveauModuleData = { &nouveauVersRec, nouveauSetup, NULL };

static pointer
nouveauSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;

	/* This module should be loaded only once, but check to be sure. */

	if (!setupDone) {
		setupDone = TRUE;
		/* The 1 here is needed to turn off a backwards compatibility mode */
		/* Otherwise NVPciProbe() is not called */
		xf86AddDriver(&NV, module, 1);

		/*
		 * The return value must be non-NULL on success even though there
		 * is no TearDownProc.
		 */
		return (pointer)1;
	} else {
		if (errmaj) *errmaj = LDR_ONCEONLY;
		return NULL;
	}
}

static const OptionInfoRec *
NVAvailableOptions(int chipid, int busid)
{
    return NVOptions;
}

/* Mandatory */
static void
NVIdentify(int flags)
{
    struct NvFamily *family;
    size_t maxLen=0;

    xf86DrvMsg(0, X_INFO, NV_NAME " driver " NV_DRIVER_DATE "\n");
    xf86DrvMsg(0, X_INFO, NV_NAME " driver for NVIDIA chipset families :\n");

    /* maximum length for alignment */
    family = NVKnownFamilies;
    while(family->name && family->chipset)
    {
        maxLen = max(maxLen, strlen(family->name));
        family++;
    }

    /* display */
    family = NVKnownFamilies;
    while(family->name && family->chipset)
    {
        size_t len = strlen(family->name);
        xf86ErrorF("\t%s", family->name);
        while(len<maxLen+1)
        {
            xf86ErrorF(" ");
            len++;
        }
        xf86ErrorF("(%s)\n", family->chipset);
        family++;
    }
}

static Bool
NVDriverFunc(ScrnInfoPtr scrn, xorgDriverFuncOp op, void *data)
{
    xorgHWFlags *flag;

    switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
	    flag = (CARD32 *)data;
	    (*flag) = 0;
	    return TRUE;
#if XORG_VERSION_CURRENT > XORG_VERSION_NUMERIC(1,15,99,0,0)
	case SUPPORTS_SERVER_FDS:
	    return TRUE;
#endif
	default:
	    return FALSE;
    }
}

static void
NVInitScrn(ScrnInfoPtr pScrn, struct xf86_platform_device *platform_dev,
	   int entity_num)
{
	DevUnion *pPriv;
	NVEntPtr pNVEnt;

	pScrn->driverVersion    = NV_VERSION;
	pScrn->driverName       = NV_DRIVER_NAME;
	pScrn->name             = NV_NAME;

	pScrn->Probe            = NULL;
	pScrn->PreInit          = NVPreInit;
	pScrn->ScreenInit       = NVScreenInit;
	pScrn->SwitchMode       = NVSwitchMode;
	pScrn->AdjustFrame      = NVAdjustFrame;
	pScrn->EnterVT          = NVEnterVT;
	pScrn->LeaveVT          = NVLeaveVT;
	pScrn->FreeScreen       = NVFreeScreen;

	xf86SetEntitySharable(entity_num);
	if (NVEntityIndex == -1)
	    NVEntityIndex = xf86AllocateEntityPrivateIndex();

	pPriv = xf86GetEntityPrivate(entity_num,
				     NVEntityIndex);
	if (!pPriv->ptr) {
		pPriv->ptr = xnfcalloc(sizeof(NVEntRec), 1);
		pNVEnt = pPriv->ptr;
		pNVEnt->platform_dev = platform_dev;
	}
	else
		pNVEnt = pPriv->ptr;

	/* Reset settings which must not persist across server regeneration */
	if (pNVEnt->reinitGeneration != serverGeneration) {
		pNVEnt->reinitGeneration = serverGeneration;
		/* Clear mask of assigned crtc's in this generation to "none" */
		pNVEnt->assigned_crtcs = 0;
	}

	xf86SetEntityInstanceForScreen(pScrn, entity_num,
					xf86GetNumEntityInstances(entity_num) - 1);
}

static struct nouveau_device *
NVOpenNouveauDevice(struct pci_device *pci_dev,
	struct xf86_platform_device *platform_dev, int scrnIndex, Bool probe)
{
	struct nouveau_device *dev = NULL;
	char *busid;
	int ret, fd = -1;

#ifdef ODEV_ATTRIB_PATH
	if (platform_dev)
		busid = NULL;
	else
#endif
	{
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,9,99,901,0)
		XNFasprintf(&busid, "pci:%04x:%02x:%02x.%d",
			    pci_dev->domain, pci_dev->bus,
			    pci_dev->dev, pci_dev->func);
#else
		busid = XNFprintf("pci:%04x:%02x:%02x.%d",
				  pci_dev->domain, pci_dev->bus,
				  pci_dev->dev, pci_dev->func);
#endif
	}

#if defined(ODEV_ATTRIB_FD)
	if (platform_dev)
		fd = xf86_get_platform_device_int_attrib(platform_dev,
							 ODEV_ATTRIB_FD, -1);
#endif
	if (fd != -1)
		ret = nouveau_device_wrap(fd, 0, &dev);
#ifdef ODEV_ATTRIB_PATH
	else if (platform_dev) {
		const char *path;

		path = xf86_get_platform_device_attrib(platform_dev,
						       ODEV_ATTRIB_PATH);

		fd = open(path, O_RDWR | O_CLOEXEC);
		ret = nouveau_device_wrap(fd, 1, &dev);
		if (ret)
			close(fd);
	}
#endif
	else
		ret = nouveau_device_open(busid, &dev);
	if (ret)
		xf86DrvMsg(scrnIndex, X_ERROR,
			   "[drm] Failed to open DRM device for %s: %d\n",
			   busid, ret);

	free(busid);
	return dev;
}

static Bool
NVHasKMS(struct pci_device *pci_dev, struct xf86_platform_device *platform_dev)
{
	struct nouveau_device *dev = NULL;
	drmVersion *version;
	int chipset;

	dev = NVOpenNouveauDevice(pci_dev, platform_dev, -1, TRUE);
	if (!dev)
		return FALSE;

	/* Check the version reported by the kernel module.  In theory we
	 * shouldn't have to do this, as libdrm_nouveau will do its own checks.
	 * But, we're currently using the kernel patchlevel to also version
	 * the DRI interface.
	 */
	version = drmGetVersion(dev->fd);
	xf86DrvMsg(-1, X_INFO, "[drm] nouveau interface version: %d.%d.%d\n",
		   version->version_major, version->version_minor,
		   version->version_patchlevel);
	drmFree(version);

	chipset = dev->chipset;
	nouveau_device_del(&dev);


	switch (chipset & ~0xf) {
	case 0x00:
	case 0x10:
	case 0x20:
	case 0x30:
	case 0x40:
	case 0x60:
	case 0x50:
	case 0x80:
	case 0x90:
	case 0xa0:
	case 0xc0:
	case 0xd0:
	case 0xe0:
	case 0xf0:
	case 0x100:
	case 0x110:
	case 0x120:
	case 0x130:
		break;
	default:
		xf86DrvMsg(-1, X_ERROR, "Unknown chipset: NV%02X\n", chipset);
		return FALSE;
	}
	return TRUE;
}

static Bool
NVPciProbe(DriverPtr drv, int entity_num, struct pci_device *pci_dev,
	   intptr_t match_data)
{
	PciChipsets NVChipsets[] = {
		{ pci_dev->device_id,
		  (pci_dev->vendor_id << 16) | pci_dev->device_id, NULL },
		{ -1, -1, NULL }
	};
	ScrnInfoPtr pScrn = NULL;

	if (!NVHasKMS(pci_dev, NULL))
		return FALSE;

	pScrn = xf86ConfigPciEntity(pScrn, 0, entity_num, NVChipsets,
				    NULL, NULL, NULL, NULL, NULL);
	if (!pScrn)
		return FALSE;

	NVInitScrn(pScrn, NULL, entity_num);

	return TRUE;
}

#ifdef XSERVER_PLATFORM_BUS
static Bool
NVPlatformProbe(DriverPtr driver,
            int entity_num, int flags, struct xf86_platform_device *dev, intptr_t dev_match_data)
{
	ScrnInfoPtr scrn = NULL;
	uint32_t scr_flags = 0;

	if (!NVHasKMS(dev->pdev, dev))
		return FALSE;

	if (flags & PLATFORM_PROBE_GPU_SCREEN)
		scr_flags = XF86_ALLOCATE_GPU_SCREEN;

	scrn = xf86AllocateScreen(driver, scr_flags);
	if (!scrn)
		return FALSE;

	if (xf86IsEntitySharable(entity_num))
		xf86SetEntityShared(entity_num);
	xf86AddEntityToScreen(scrn, entity_num);

	NVInitScrn(scrn, dev, entity_num);

	return TRUE;
}
#endif

#define MAX_CHIPS MAXSCREENS

Bool
NVSwitchMode(SWITCH_MODE_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);

	return xf86SetSingleMode(pScrn, mode, RR_Rotate_0);
}

/*
 * This function is used to initialize the Start Address - the first
 * displayed location in the video memory.
 */
/* Usually mandatory */
void 
NVAdjustFrame(ADJUST_FRAME_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	drmmode_adjust_frame(pScrn, x, y);
}

/*
 * This is called when VT switching back to the X server.  Its job is
 * to reinitialise the video mode.
 */

/* Mandatory */
static Bool
NVEnterVT(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	NVPtr pNv = NVPTR(pScrn);
#ifdef XF86_PDEV_SERVER_FD
	NVEntPtr pNVEnt = NVEntPriv(pScrn);
#endif
	int ret;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVEnterVT is called.\n");

#ifdef XF86_PDEV_SERVER_FD
	if (!(pNVEnt->platform_dev &&
	      (pNVEnt->platform_dev->flags & XF86_PDEV_SERVER_FD)))
#endif
	{
		ret = drmSetMaster(pNv->dev->fd);
		if (ret)
			ErrorF("Unable to get master: %s\n", strerror(errno));
	}

	if (XF86_CRTC_CONFIG_PTR(pScrn)->num_crtc && !xf86SetDesiredModes(pScrn))
		return FALSE;

	if (pNv->overlayAdaptor && pNv->Architecture != NV_ARCH_04)
		NV10WriteOverlayParameters(pScrn);

	return TRUE;
}

/*
 * This is called when VT switching away from the X server.  Its job is
 * to restore the previous (text) mode.
 */

/* Mandatory */
static void
NVLeaveVT(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	NVPtr pNv = NVPTR(pScrn);
#ifdef XF86_PDEV_SERVER_FD
	NVEntPtr pNVEnt = NVEntPriv(pScrn);
#endif
	int ret;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "NVLeaveVT is called.\n");

#ifdef XF86_PDEV_SERVER_FD
	if (pNVEnt->platform_dev &&
	    (pNVEnt->platform_dev->flags & XF86_PDEV_SERVER_FD))
		return;
#endif

	ret = drmDropMaster(pNv->dev->fd);
	if (ret && errno != EIO && errno != ENODEV)
		ErrorF("Error dropping master: %i(%m)\n", -errno);
}

static void
NVFlushCallback(CallbackListPtr *list, pointer user_data, pointer call_data)
{
	ScrnInfoPtr pScrn = user_data;
	NVPtr pNv = NVPTR(pScrn);
	if (pScrn->vtSema && pNv->Flush)
		pNv->Flush(pScrn);
}

#ifdef NOUVEAU_PIXMAP_SHARING
static void
redisplay_dirty(ScreenPtr screen, PixmapDirtyUpdatePtr dirty)
{
	RegionRec pixregion;

	PixmapRegionInit(&pixregion, dirty->slave_dst);

	DamageRegionAppend(&dirty->slave_dst->drawable, &pixregion);
#ifdef HAS_DIRTYTRACKING_ROTATION
	PixmapSyncDirtyHelper(dirty);
#else
	PixmapSyncDirtyHelper(dirty, &pixregion);
#endif

	DamageRegionProcessPending(&dirty->slave_dst->drawable);
	RegionUninit(&pixregion);
}

static void
nouveau_dirty_update(ScreenPtr screen)
{
	RegionPtr region;
	PixmapDirtyUpdatePtr ent;

	if (xorg_list_is_empty(&screen->pixmap_dirty_list))
		return;

	xorg_list_for_each_entry(ent, &screen->pixmap_dirty_list, ent) {
		region = DamageRegion(ent->damage);
		if (RegionNotEmpty(region)) {
			redisplay_dirty(screen, ent);
			DamageEmpty(ent->damage);
		}
	}
}
#endif

static void 
NVBlockHandler (BLOCKHANDLER_ARGS_DECL)
{
	SCREEN_PTR(arg);
	ScrnInfoPtr pScrn   = xf86ScreenToScrn(pScreen);
	NVPtr pNv = NVPTR(pScrn);

	pScreen->BlockHandler = pNv->BlockHandler;
	(*pScreen->BlockHandler) (BLOCKHANDLER_ARGS);
	pScreen->BlockHandler = NVBlockHandler;

#ifdef NOUVEAU_PIXMAP_SHARING
	nouveau_dirty_update(pScreen);
#endif

	NVFlushCallback(NULL, pScrn, NULL);

	if (pNv->VideoTimerCallback) 
		(*pNv->VideoTimerCallback)(pScrn, currentTime.milliseconds);
}

static Bool
NVCreateScreenResources(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	NVPtr pNv = NVPTR(pScrn);

	pScreen->CreateScreenResources = pNv->CreateScreenResources;
	if (!(*pScreen->CreateScreenResources)(pScreen))
		return FALSE;
	pScreen->CreateScreenResources = NVCreateScreenResources;

	drmmode_fbcon_copy(pScreen);
	if (!NVEnterVT(VT_FUNC_ARGS(0)))
		return FALSE;

	if (pNv->AccelMethod == EXA) {
		PixmapPtr ppix = pScreen->GetScreenPixmap(pScreen);
		nouveau_bo_ref(pNv->scanout, &nouveau_pixmap(ppix)->bo);
	}

	return TRUE;
}

/*
 * This is called at the end of each server generation.  It restores the
 * original (text) mode.  It should also unmap the video memory, and free
 * any per-generation data allocated by the driver.  It should finish
 * by unwrapping and calling the saved CloseScreen function.
 */

/* Mandatory */
static Bool
NVCloseScreen(CLOSE_SCREEN_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	NVPtr pNv = NVPTR(pScrn);

	if (XF86_CRTC_CONFIG_PTR(pScrn)->num_crtc)
		drmmode_screen_fini(pScreen);

	nouveau_present_fini(pScreen);
	nouveau_dri2_fini(pScreen);
	nouveau_sync_fini(pScreen);
	nouveau_copy_fini(pScreen);

	if (pScrn->vtSema) {
		NVLeaveVT(VT_FUNC_ARGS(0));
		pScrn->vtSema = FALSE;
	}

	NVTakedownVideo(pScrn);
	NVAccelCommonFini(pScrn);
	NVUnmapMem(pScrn);

	xf86_cursors_fini(pScreen);

	DeleteCallback(&FlushCallback, NVFlushCallback, pScrn);

	if (pNv->ShadowPtr) {
		free(pNv->ShadowPtr);
		pNv->ShadowPtr = NULL;
	}
	if (pNv->overlayAdaptor) {
		free(pNv->overlayAdaptor);
		pNv->overlayAdaptor = NULL;
	}
	if (pNv->blitAdaptor) {
		free(pNv->blitAdaptor);
		pNv->blitAdaptor = NULL;
	}
	if (pNv->textureAdaptor[0]) {
		free(pNv->textureAdaptor[0]);
		pNv->textureAdaptor[0] = NULL;
	}
	if (pNv->textureAdaptor[1]) {
		free(pNv->textureAdaptor[1]);
		pNv->textureAdaptor[1] = NULL;
	}
	if (pNv->EXADriverPtr) {
		exaDriverFini(pScreen);
		free(pNv->EXADriverPtr);
		pNv->EXADriverPtr = NULL;
	}

	pScrn->vtSema = FALSE;
	pScreen->CloseScreen = pNv->CloseScreen;
	pScreen->BlockHandler = pNv->BlockHandler;
	return (*pScreen->CloseScreen)(CLOSE_SCREEN_ARGS);
}

/* Free up any persistent data structures */

/* Optional */
static void
NVFreeScreen(FREE_SCREEN_ARGS_DECL)
{
	/*
	 * This only gets called when a screen is being deleted.  It does not
	 * get called routinely at the end of a server generation.
	 */
	SCRN_INFO_PTR(arg);
	NVPtr pNv = NVPTR(pScrn);

	if (!pNv)
		return;

	NVCloseDRM(pScrn);

	free(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

#define NVPreInitFail(fmt, args...) do {                                    \
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "%d: "fmt, __LINE__, ##args); \
	NVFreeScreen(FREE_SCREEN_ARGS(pScrn));			\
	return FALSE;                                                       \
} while(0)

static void
NVCloseDRM(ScrnInfoPtr pScrn)
{
	NVPtr pNv = NVPTR(pScrn);

	drmFree(pNv->drm_device_name);
	nouveau_client_del(&pNv->client);
	nouveau_device_del(&pNv->dev);
	free(pNv->render_node);
}

static void
nouveau_setup_capabilities(ScrnInfoPtr pScrn)
{
#ifdef NOUVEAU_PIXMAP_SHARING
	NVPtr pNv = NVPTR(pScrn);
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	uint64_t value;
	int ret;

	pScrn->capabilities = 0;
	ret = drmGetCap(pNv->dev->fd, DRM_CAP_PRIME, &value);
	if (ret == 0) {
		if (value & DRM_PRIME_CAP_EXPORT)
			pScrn->capabilities |= RR_Capability_SourceOutput;
		if (value & DRM_PRIME_CAP_IMPORT) {
			pScrn->capabilities |= RR_Capability_SourceOffload;
			if (xf86_config->num_crtc)
				pScrn->capabilities |= RR_Capability_SinkOutput;
		}
	}
#endif
}

NVEntPtr NVEntPriv(ScrnInfoPtr pScrn)
{
	DevUnion     *pPriv;
	NVPtr  pNv   = NVPTR(pScrn);
	pPriv = xf86GetEntityPrivate(pNv->pEnt->index,
				     getNVEntityIndex());
	return pPriv->ptr;
}

static Bool NVOpenDRMMaster(ScrnInfoPtr pScrn)
{
	NVPtr pNv = NVPTR(pScrn);
	NVEntPtr pNVEnt = NVEntPriv(pScrn);
	drmSetVersion sv;
	int err;
	int ret;

	if (pNVEnt->fd) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   " reusing fd for second head\n");
		ret = nouveau_device_wrap(pNVEnt->fd, 0, &pNv->dev);
		if (ret) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				"[drm] error creating device\n");
			return FALSE;
		}
		return TRUE;
	}

	pNv->dev = NVOpenNouveauDevice(pNv->PciInfo, pNVEnt->platform_dev,
				       pScrn->scrnIndex, FALSE);
	if (!pNv->dev)
		return FALSE;

	sv.drm_di_major = 1;
	sv.drm_di_minor = 1;
	sv.drm_dd_major = -1;
	sv.drm_dd_minor = -1;
	err = drmSetInterfaceVersion(pNv->dev->fd, &sv);
	if (err != 0) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "[drm] failed to set drm interface version.\n");
		nouveau_device_del(&pNv->dev);
		return FALSE;
	}
	pNVEnt->fd = pNv->dev->fd;
	return TRUE;
}

static Bool
NVPreInitDRM(ScrnInfoPtr pScrn)
{
	NVPtr pNv = NVPTR(pScrn);
	int ret;

	if (!xf86LoadSubModule(pScrn, "dri2"))
		return FALSE;

	/* Load the kernel module, and open the DRM */
	ret = NVOpenDRMMaster(pScrn);
	if (!ret) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "[drm] error opening the drm\n");
		return FALSE;
	}

	ret = nouveau_client_new(pNv->dev, &pNv->client);
	if (ret)
		return FALSE;

	pNv->drm_device_name = drmGetDeviceNameFromFd(pNv->dev->fd);

	return TRUE;
}

/* Mandatory */
Bool
NVPreInit(ScrnInfoPtr pScrn, int flags)
{
	struct nouveau_device *dev;
	NVPtr pNv;
	MessageType from;
	const char *reason, *string;
	uint64_t v;
	int ret;
	int defaultDepth = 0;

	if (flags & PROBE_DETECT) {
		EntityInfoPtr pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

		if (!pEnt)
			return FALSE;

		free(pEnt);

		return TRUE;
	}

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

	/* Check the number of entities, and fail if it isn't one. */
	if (pScrn->numEntities != 1)
		return FALSE;

	/* Allocate the NVRec driverPrivate */
	if (!(pScrn->driverPrivate = xnfcalloc(1, sizeof(NVRec))))
		return FALSE;
	pNv = NVPTR(pScrn);

	/* Get the entity, and make sure it is PCI. */
	pNv->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
	if (pNv->pEnt->location.type != BUS_PCI
#ifdef XSERVER_PLATFORM_BUS
		&& pNv->pEnt->location.type != BUS_PLATFORM
#endif
		)
		return FALSE;

	if (xf86IsEntityShared(pScrn->entityList[0])) {
		if(!xf86IsPrimInitDone(pScrn->entityList[0])) {
			pNv->Primary = TRUE;
			xf86SetPrimInitDone(pScrn->entityList[0]);
		} else {
			pNv->Secondary = TRUE;
		}
        }

	/* Find the PCI info for this screen */
	pNv->PciInfo = xf86GetPciInfoForEntity(pNv->pEnt->index);

	/* Initialise the kernel module */
	if (!NVPreInitDRM(pScrn))
		NVPreInitFail("\n");
	dev = pNv->dev;

	pScrn->chipset = malloc(sizeof(char) * 25);
	sprintf((char *)pScrn->chipset, "NVIDIA NV%02X", dev->chipset);
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "Chipset: \"%s\"\n", pScrn->chipset);

	switch (dev->chipset & ~0xf) {
	case 0x00:
		pNv->Architecture = NV_ARCH_04;
		break;
	case 0x10:
		pNv->Architecture = NV_ARCH_10;
		break;
	case 0x20:
		pNv->Architecture = NV_ARCH_20;
		break;
	case 0x30:
		pNv->Architecture = NV_ARCH_30;
		break;
	case 0x40:
	case 0x60:
		pNv->Architecture = NV_ARCH_40;
		break;
	case 0x50:
	case 0x80:
	case 0x90:
	case 0xa0:
		pNv->Architecture = NV_TESLA;
		break;
	case 0xc0:
	case 0xd0:
		pNv->Architecture = NV_FERMI;
		break;
	case 0xe0:
	case 0xf0:
	case 0x100:
		pNv->Architecture = NV_KEPLER;
		break;
	case 0x110:
	case 0x120:
		pNv->Architecture = NV_MAXWELL;
		break;
	case 0x130:
		pNv->Architecture = NV_PASCAL;
		break;
	default:
		return FALSE;
	}

	/* Set pScrn->monitor */
	pScrn->monitor = pScrn->confScreen->monitor;

	/*
	 * The first thing we should figure out is the depth, bpp, etc.
	 */

	if (dev->vram_size <= 16 * 1024 * 1024)
		defaultDepth = 16;
	if (!xf86SetDepthBpp(pScrn, defaultDepth, 0, 0, Support32bppFb)) {
		NVPreInitFail("\n");
	} else {
		/* Check that the returned depth is one we support */
		switch (pScrn->depth) {
		case 16:
		case 24:
			/* OK */
			break;
		case 30:
			/* OK on NV50 KMS */
			if (pNv->Architecture < NV_TESLA)
				NVPreInitFail("Depth 30 supported on G80+ only\n");
			break;
		case 15: /* 15 may get done one day, so leave any code for it in place */
		default:
			NVPreInitFail("Given depth (%d) is not supported by this driver\n",
				pScrn->depth);
		}
	}
	xf86PrintDepthBpp(pScrn);

	/*
	 * This must happen after pScrn->display has been set because
	 * xf86SetWeight references it.
	 */
	rgb rgbzeros = {0, 0, 0};

	if (pScrn->depth == 30) {
		rgb rgbmask;

		rgbmask.red   = 0x000003ff;
		rgbmask.green = 0x000ffc00;
		rgbmask.blue  = 0x3ff00000;
		if (!xf86SetWeight(pScrn, rgbzeros, rgbmask))
			NVPreInitFail("\n");

		/* xf86SetWeight() seems to think ffs(1) == 0... */
		pScrn->offset.red--;
		pScrn->offset.green--;
		pScrn->offset.blue--;
	} else {
		if (!xf86SetWeight(pScrn, rgbzeros, rgbzeros))
			NVPreInitFail("\n");
	}

	if (!xf86SetDefaultVisual(pScrn, -1))
		NVPreInitFail("\n");

	/* We don't support DirectColor */
	if (pScrn->defaultVisual != TrueColor) {
		NVPreInitFail("Given default visual (%s) is not supported at depth %d\n",
			      xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
	}

	/* We use a programmable clock */
	pScrn->progClock = TRUE;

	/* Collect all of the relevant option flags (fill in pScrn->options) */
	xf86CollectOptions(pScrn, NULL);

	/* Process the options */
	if (!(pNv->Options = malloc(sizeof(NVOptions))))
		return FALSE;
	memcpy(pNv->Options, NVOptions, sizeof(NVOptions));
	xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, pNv->Options);

	from = X_DEFAULT;

	pNv->HWCursor = TRUE;
	/*
	 * The preferred method is to use the "hw cursor" option as a tri-state
	 * option, with the default set above.
	 */
	if (xf86GetOptValBool(pNv->Options, OPTION_HW_CURSOR, &pNv->HWCursor)) {
		from = X_CONFIG;
	}
	/* For compatibility, accept this too (as an override) */
	if (xf86ReturnOptValBool(pNv->Options, OPTION_SW_CURSOR, FALSE)) {
		from = X_CONFIG;
		pNv->HWCursor = FALSE;
	}
	xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pNv->HWCursor ? "HW" : "SW");

	string = xf86GetOptValString(pNv->Options, OPTION_ACCELMETHOD);
	if (string) {
		if      (!strcmp(string,   "none")) pNv->AccelMethod = NONE;
		else if (!strcmp(string,    "exa")) pNv->AccelMethod = EXA;
		else {
			xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				   "Invalid AccelMethod specified\n");
		}
	}

	if (pNv->AccelMethod == UNKNOWN) {
		pNv->AccelMethod = EXA;
	}

	if (xf86ReturnOptValBool(pNv->Options, OPTION_NOACCEL, FALSE)) {
		pNv->AccelMethod = NONE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
	}

	if (xf86ReturnOptValBool(pNv->Options, OPTION_SHADOW_FB, FALSE)) {
		pNv->ShadowFB = TRUE;
		pNv->AccelMethod = NONE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, 
			"Using \"Shadow Framebuffer\" - acceleration disabled\n");
	}

	if (pNv->AccelMethod > NONE) {
		if (pNv->Architecture >= NV_TESLA)
			pNv->wfb_enabled = xf86ReturnOptValBool(
				pNv->Options, OPTION_WFB, FALSE);

		pNv->tiled_scanout = TRUE;
	}

	pNv->ce_enabled =
		xf86ReturnOptValBool(pNv->Options, OPTION_ASYNC_COPY, FALSE);

	/* Define maximum allowed level of DRI implementation to use.
	 * We default to DRI2 on EXA for now, as DRI3 still has some
	 * problems.
	 */
	pNv->max_dri_level = 2;
	from = X_DEFAULT;

	if (xf86GetOptValInteger(pNv->Options, OPTION_DRI,
				 &pNv->max_dri_level)) {
		from = X_CONFIG;
		if (pNv->max_dri_level < 2)
			pNv->max_dri_level = 2;
		if (pNv->max_dri_level > 3)
			pNv->max_dri_level = 3;
	}
	xf86DrvMsg(pScrn->scrnIndex, from, "Allowed maximum DRI level %i.\n",
		   pNv->max_dri_level);

	if (pNv->AccelMethod > NONE && pNv->dev->chipset >= 0x11) {
		from = X_DEFAULT;
		pNv->glx_vblank = TRUE;
		if (xf86GetOptValBool(pNv->Options, OPTION_GLX_VBLANK,
				      &pNv->glx_vblank))
			from = X_CONFIG;

		xf86DrvMsg(pScrn->scrnIndex, from, "GLX sync to VBlank %s.\n",
			   pNv->glx_vblank ? "enabled" : "disabled");
	}

#ifdef NOUVEAU_GETPARAM_HAS_PAGEFLIP
	reason = ": no kernel support";
	from = X_DEFAULT;

	ret = nouveau_getparam(pNv->dev, NOUVEAU_GETPARAM_HAS_PAGEFLIP, &v);
	if (ret == 0 && v == 1) {
		pNv->has_pageflip = TRUE;
		if (xf86GetOptValBool(pNv->Options, OPTION_PAGE_FLIP, &pNv->has_pageflip))
			from = X_CONFIG;
		reason = "";
	}
#else
	reason = ": not available at build time";
#endif

	xf86DrvMsg(pScrn->scrnIndex, from, "Page flipping %sabled%s\n",
		   pNv->has_pageflip ? "en" : "dis", reason);

	if(xf86GetOptValInteger(pNv->Options, OPTION_VIDEO_KEY, &(pNv->videoKey))) {
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "video key set to 0x%x\n",
					pNv->videoKey);
	} else {
		pNv->videoKey =  (1 << pScrn->offset.red) | 
					(1 << pScrn->offset.green) |
		(((pScrn->mask.blue >> pScrn->offset.blue) - 1) << pScrn->offset.blue);
	}

	/* Limit to max 2 pending swaps - we can't handle more than triple-buffering: */
	pNv->max_swap_limit = 2;

	if(xf86GetOptValInteger(pNv->Options, OPTION_SWAP_LIMIT, &(pNv->swap_limit))) {
		if (pNv->swap_limit < 1)
			pNv->swap_limit = 1;

		if (pNv->swap_limit > pNv->max_swap_limit)
			pNv->swap_limit = pNv->max_swap_limit;

		reason = "";
		from = X_CONFIG;

		if ((DRI2INFOREC_VERSION < 6) && (pNv->swap_limit > 1)) {
			/* No swap limit api in server. A value > 1 requires use
			 * of problematic hacks.
			 */
			from = X_WARNING;
			reason = ": Caution: Use of this swap limit > 1 violates OML_sync_control spec on this X-Server!\n";
		}
	} else {
		/* Always default to double-buffering, because it avoids artifacts like
		 * unthrottled rendering of non-fullscreen clients under desktop composition.
		 */
		pNv->swap_limit = 1;
		reason = "";
		from = X_DEFAULT;
	}

	xf86DrvMsg(pScrn->scrnIndex, from, "Swap limit set to %d [Max allowed %d]%s\n",
		   pNv->swap_limit, pNv->max_swap_limit, reason);

	/* Does kernel do the sync of pageflips to vblank? */
	pNv->has_async_pageflip = FALSE;
#ifdef DRM_CAP_ASYNC_PAGE_FLIP
	ret = drmGetCap(pNv->dev->fd, DRM_CAP_ASYNC_PAGE_FLIP, &v);
	if (ret == 0 && v == 1) {
		pNv->has_async_pageflip = TRUE;
	}
	xf86DrvMsg(pScrn->scrnIndex, X_DEFAULT, "Page flipping synced to vblank by %s.\n",
			   pNv->has_async_pageflip ? "kernel" : "ddx");
#endif

	ret = drmmode_pre_init(pScrn, pNv->dev->fd, pScrn->bitsPerPixel >> 3);
	if (ret == FALSE)
		NVPreInitFail("Kernel modesetting failed to initialize\n");

	/*
	 * If the driver can do gamma correction, it should call xf86SetGamma()
	 * here.
	 */
	Gamma gammazeros = {0.0, 0.0, 0.0};

	if (!xf86SetGamma(pScrn, gammazeros))
		NVPreInitFail("\n");

#ifdef NOUVEAU_PIXMAP_SHARING
	/*
	 * The driver will not work as gpu screen without acceleration enabled.
	 * To support this usecase modesetting ddx can be used instead.
	 */
	if (pNv->AccelMethod <= NONE || pNv->ShadowFB) {
		/*
		 * Optimus mode requires acceleration enabled.
		 * So if no mode is found, or the screen is created
		 * as a gpu screen the pre init should fail.
		 */
		if (pScrn->is_gpu || !pScrn->modes)
			return FALSE;
	}

#else
	/* No usable mode, no optimus config possible */
	if (!pScrn->modes)
		return FALSE;
#endif

	nouveau_setup_capabilities(pScrn);

	if (!pScrn->modes) {
		pScrn->modes = xf86ModesAdd(pScrn->modes,
			xf86CVTMode(pScrn->display->virtualX,
				    pScrn->display->virtualY,
				    60, 0, 0));
	}

	/* Set the current mode to the first in the list */
	pScrn->currentMode = pScrn->modes;

	/* Print the list of modes being used */
	xf86PrintModes(pScrn);

	/* Set display resolution */
	xf86SetDpi(pScrn, 0, 0);

	if (pNv->wfb_enabled) {
		if (xf86LoadSubModule(pScrn, "wfb") == NULL)
			NVPreInitFail("\n");
	}

	if (xf86LoadSubModule(pScrn, "fb") == NULL)
		NVPreInitFail("\n");

	/* Load shadowfb */
	if (!xf86LoadSubModule(pScrn, "shadowfb"))
		NVPreInitFail("\n");

	return TRUE;
}


static Bool
NVMapMem(ScrnInfoPtr pScrn)
{
	NVPtr pNv = NVPTR(pScrn);
	int ret, pitch;

	ret = nouveau_allocate_surface(pScrn, pScrn->virtualX, pScrn->virtualY,
				       pScrn->bitsPerPixel,
				       NOUVEAU_CREATE_PIXMAP_SCANOUT,
				       &pitch, &pNv->scanout);
	if (!ret) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Error allocating scanout buffer: %d\n", ret);
		return FALSE;
	}

	pScrn->displayWidth = pitch / (pScrn->bitsPerPixel / 8);
	return TRUE;
}

/*
 * Unmap the framebuffer and offscreen memory.
 */

static Bool
NVUnmapMem(ScrnInfoPtr pScrn)
{
	NVPtr pNv = NVPTR(pScrn);

	drmmode_remove_fb(pScrn);

	nouveau_bo_ref(NULL, &pNv->transfer);
	nouveau_bo_ref(NULL, &pNv->scanout);
	return TRUE;
}

static void
NVLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices,
	      LOCO * colors, VisualPtr pVisual)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	int c;
	int i, j, index;
	CARD16 lut_r[256], lut_g[256], lut_b[256];

	for (c = 0; c < xf86_config->num_crtc; c++) {
		xf86CrtcPtr crtc = xf86_config->crtc[c];

		/* code borrowed from intel driver */
		switch (pScrn->depth) {
		case 15:
			for (i = 0; i < numColors; i++) {
				index = indices[i];
				for (j = 0; j < 8; j++) {
					lut_r[index * 8 + j] = colors[index].red << 8;
					lut_g[index * 8 + j] = colors[index].green << 8;
					lut_b[index * 8 + j] = colors[index].blue << 8;
				}
			}
			break;

		case 16:
			for (i = 0; i < numColors; i++) {
				index = indices[i];

				if (i <= 31) {
					for (j = 0; j < 8; j++) {
						lut_r[index * 8 + j] = colors[index].red << 8;
						lut_b[index * 8 + j] = colors[index].blue << 8;
					}
				}

				for (j = 0; j < 4; j++) {
					lut_g[index * 4 + j] = colors[index].green << 8;
				}
			}
			break;

		default:
			for (i = 0; i < numColors; i++) {
				index = indices[i];
				lut_r[index] = colors[index].red << 8;
				lut_g[index] = colors[index].green << 8;
				lut_b[index] = colors[index].blue << 8;
			}
			break;
		}

		if (crtc->randr_crtc)
			/* Make the change through RandR */
			RRCrtcGammaSet(crtc->randr_crtc, lut_r, lut_g, lut_b);
	}
}

/* Mandatory */

/* This gets called at the start of each server generation */
static Bool
NVScreenInit(SCREEN_INIT_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	NVPtr pNv = NVPTR(pScrn);
	int ret;
	VisualPtr visual;
	unsigned char *FBStart;
	int displayWidth;

	if (pNv->AccelMethod == EXA) {
		if (!NVAccelCommonInit(pScrn)) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "Error initialising acceleration.  "
				   "Falling back to NoAccel\n");
			pNv->AccelMethod = NONE;
			pNv->ShadowFB = TRUE;
			pNv->wfb_enabled = FALSE;
			pNv->tiled_scanout = FALSE;
			pScrn->displayWidth = nv_pitch_align(pNv,
							     pScrn->virtualX,
							     pScrn->depth);
		}
	}

	nouveau_copy_init(pScreen);

	/* Allocate and map memory areas we need */
	if (!NVMapMem(pScrn))
		return FALSE;

	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	int i;

	/* need to point to new screen on server regeneration */
	for (i = 0; i < xf86_config->num_crtc; i++)
		xf86_config->crtc[i]->scrn = pScrn;
	for (i = 0; i < xf86_config->num_output; i++)
		xf86_config->output[i]->scrn = pScrn;

	/*
	 * The next step is to setup the screen's visuals, and initialise the
	 * framebuffer code.  In cases where the framebuffer's default
	 * choices for things like visual layouts and bits per RGB are OK,
	 * this may be as simple as calling the framebuffer's ScreenInit()
	 * function.  If not, the visuals will need to be setup before calling
	 * a fb ScreenInit() function and fixed up after.
	 *
	 * For most PC hardware at depths >= 8, the defaults that fb uses
	 * are not appropriate.  In this driver, we fixup the visuals after.
	 */

	/*
	 * Reset the visual list.
	 */
	miClearVisualTypes();

	/* Setup the visuals we support. */
	if (!miSetVisualTypes(pScrn->depth, 
			      miGetDefaultVisualMask(pScrn->depth),
			      pScrn->rgbBits, pScrn->defaultVisual))
		return FALSE;

	if (!miSetPixmapDepths ())
		return FALSE;

	/*
	 * Call the framebuffer layer's ScreenInit function, and fill in other
	 * pScreen fields.
	 */

	if (pNv->ShadowFB) {
		pNv->ShadowPitch = BitmapBytePad(pScrn->bitsPerPixel * pScrn->virtualX);
		pNv->ShadowPtr = malloc(pNv->ShadowPitch * pScrn->virtualY);
		displayWidth = pNv->ShadowPitch / (pScrn->bitsPerPixel >> 3);
		FBStart = pNv->ShadowPtr;
	} else
	if (pNv->AccelMethod <= NONE) {
		pNv->ShadowPtr = NULL;
		displayWidth = pScrn->displayWidth;
		nouveau_bo_map(pNv->scanout, NOUVEAU_BO_RDWR, pNv->client);
		FBStart = pNv->scanout->map;
	} else {
		pNv->ShadowPtr = NULL;
		displayWidth = pScrn->displayWidth;
		FBStart = NULL;
	}

	switch (pScrn->bitsPerPixel) {
	case 16:
	case 32:
	if (pNv->wfb_enabled) {
		ret = wfbScreenInit(pScreen, FBStart, pScrn->virtualX,
				    pScrn->virtualY, pScrn->xDpi, pScrn->yDpi,
				    displayWidth, pScrn->bitsPerPixel,
				    nouveau_wfb_setup_wrap,
				    nouveau_wfb_finish_wrap);
	} else {
		ret = fbScreenInit(pScreen, FBStart, pScrn->virtualX,
				   pScrn->virtualY, pScrn->xDpi, pScrn->yDpi,
				   displayWidth, pScrn->bitsPerPixel);
	}
		break;
	default:
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Internal error: invalid bpp (%d) in NVScreenInit\n",
			   pScrn->bitsPerPixel);
		ret = FALSE;
		break;
	}
	if (!ret)
		return FALSE;

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

	if (pNv->wfb_enabled)
		wfbPictureInit (pScreen, 0, 0);
	else
		fbPictureInit (pScreen, 0, 0);

	xf86SetBlackWhitePixels(pScreen);

	if (nouveau_present_init(pScreen))
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "Hardware support for Present enabled\n");
	else
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "Hardware support for Present disabled\n");

	nouveau_sync_init(pScreen);
	nouveau_dri2_init(pScreen);
	if (pNv->AccelMethod == EXA) {
		if (pNv->max_dri_level >= 3 &&
		    !nouveau_dri3_screen_init(pScreen))
			return FALSE;

		if (!nouveau_exa_init(pScreen))
			return FALSE;
	}

	xf86SetBackingStore(pScreen);
	xf86SetSilkenMouse(pScreen);

	/* 
	 * Initialize software cursor.
	 * Must precede creation of the default colormap.
	 */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	/*
	 * Initialize HW cursor layer. 
	 * Must follow software cursor initialization.
	 */
	if (xf86_config->num_crtc && pNv->HWCursor) {
		ret = drmmode_cursor_init(pScreen);
		if (ret != TRUE) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "Hardware cursor initialization failed\n");
			pNv->HWCursor = FALSE;
		}
	}

	if (pNv->ShadowFB)
		ShadowFBInit(pScreen, NVRefreshArea);

	pScrn->fbOffset = 0;

	NVInitVideo(pScreen);

	/* Wrap the block handler here, if we do it after the EnterVT we
	 * can end up in the unfortunate case where we've wrapped the
	 * xf86RotateBlockHandler which sometimes is not expecting to
	 * be in the wrap chain and calls a NULL pointer...
	 */
	pNv->BlockHandler = pScreen->BlockHandler;
	pScreen->BlockHandler = NVBlockHandler;

	if (!AddCallback(&FlushCallback, NVFlushCallback, pScrn))
		return FALSE;

	pScrn->vtSema = TRUE;
	pScrn->pScreen = pScreen;

	xf86DPMSInit(pScreen, xf86DPMSSet, 0);

	/* Wrap the current CloseScreen function */
	pScreen->SaveScreen = NVSaveScreen;
	pNv->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = NVCloseScreen;
	pNv->CreateScreenResources = pScreen->CreateScreenResources;
	pScreen->CreateScreenResources = NVCreateScreenResources;

#ifdef NOUVEAU_PIXMAP_SHARING
	pScreen->StartPixmapTracking = PixmapStartDirtyTracking;
	pScreen->StopPixmapTracking = PixmapStopDirtyTracking;
#endif

	if (!xf86CrtcScreenInit(pScreen))
		return FALSE;

	/* Initialise default colourmap */
	if (!miCreateDefColormap(pScreen))
		return FALSE;

	/*
	 * Initialize colormap layer.
	 * Must follow initialization of the default colormap.
	 * X-Server < 1.20 mishandles > 256 slots / > 8 bpc color maps, so skip
	 * color map setup on old servers at > 8 bpc. Gamma luts still work.
	 */
	if (xf86_config->num_crtc && (pScrn->rgbBits <= 8 ||
	    XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,20,0,0,0)) &&
	    !xf86HandleColormaps(pScreen, 1 << pScrn->rgbBits, pScrn->rgbBits,
				 NVLoadPalette, NULL, CMAP_PALETTED_TRUECOLOR))
		return FALSE;

	/* Report any unused options (only for the first generation) */
	if (serverGeneration == 1)
		xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

	if (xf86_config->num_crtc)
		drmmode_screen_init(pScreen);
	else
		pNv->glx_vblank = FALSE;
	return TRUE;
}

static Bool
NVSaveScreen(ScreenPtr pScreen, int mode)
{
	return TRUE;
}

