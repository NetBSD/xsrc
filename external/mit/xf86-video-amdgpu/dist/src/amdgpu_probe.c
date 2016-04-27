/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>

/*
 * Authors:
 *   Kevin E. Martin <martin@xfree86.org>
 *   Rickard E. Faith <faith@valinux.com>
 * KMS support - Dave Airlie <airlied@redhat.com>
 */

#include "amdgpu_probe.h"
#include "amdgpu_version.h"
#include "amdgpu_drv.h"
#include "amdpciids.h"

#include "xf86.h"

#include "xf86drmMode.h"
#include "dri.h"

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <xf86_OSproc.h>
#endif

#ifdef XSERVER_PLATFORM_BUS
#include <xf86platformBus.h>
#endif

#include "amdgpu_chipset_gen.h"

#include "amdgpu_pci_chipset_gen.h"

#include "amdgpu_pci_device_match_gen.h"

_X_EXPORT int gAMDGPUEntityIndex = -1;

/* Return the options for supported chipset 'n'; NULL otherwise */
static const OptionInfoRec *AMDGPUAvailableOptions(int chipid, int busid)
{
	return AMDGPUOptionsWeak();
}

/* Return the string name for supported chipset 'n'; NULL otherwise. */
static void AMDGPUIdentify(int flags)
{
	xf86PrintChipsets(AMDGPU_NAME,
			  "Driver for AMD Radeon chipsets", AMDGPUChipsets);
}

static char *amdgpu_bus_id(ScrnInfoPtr pScrn, struct pci_device *dev)
{
	char *busid;

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,9,99,901,0)
	XNFasprintf(&busid, "pci:%04x:%02x:%02x.%d",
		    dev->domain, dev->bus, dev->dev, dev->func);
#else
	busid = XNFprintf("pci:%04x:%02x:%02x.%d",
			  dev->domain, dev->bus, dev->dev, dev->func);
#endif

	if (!busid)
		xf86DrvMsgVerb(pScrn->scrnIndex, X_ERROR, 0,
			       "AMDGPU: Failed to generate bus ID string\n");

	return busid;
}

static Bool amdgpu_kernel_mode_enabled(ScrnInfoPtr pScrn, char *busIdString)
{
	int ret = drmCheckModesettingSupported(busIdString);

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
	if (ret) {
		if (xf86LoadKernelModule("amdgpukms"))
			ret = drmCheckModesettingSupported(busIdString);
	}
#endif
	if (ret) {
		xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 0,
			       "[KMS] drm report modesetting isn't supported.\n");
		return FALSE;
	}

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, 0,
		       "[KMS] Kernel modesetting enabled.\n");
	return TRUE;
}

static int amdgpu_kernel_open_fd(ScrnInfoPtr pScrn, char *busid,
				 struct xf86_platform_device *platform_dev)
{
	int fd;

#ifdef XF86_PDEV_SERVER_FD
	if (platform_dev) {
		fd = xf86_get_platform_device_int_attrib(platform_dev,
							 ODEV_ATTRIB_FD, -1);
		if (fd != -1)
			return fd;
	}
#endif

	fd = drmOpen(NULL, busid);
	if (fd == -1)
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "[drm] Failed to open DRM device for %s: %s\n",
			   busid, strerror(errno));
	return fd;
}

static Bool amdgpu_open_drm_master(ScrnInfoPtr pScrn, AMDGPUEntPtr pAMDGPUEnt,
				   char *busid)
{
	drmSetVersion sv;
	int err;

	pAMDGPUEnt->fd = amdgpu_kernel_open_fd(pScrn, busid, NULL);
	if (pAMDGPUEnt->fd == -1)
		return FALSE;

	/* Check that what we opened was a master or a master-capable FD,
	 * by setting the version of the interface we'll use to talk to it.
	 * (see DRIOpenDRMMaster() in DRI1)
	 */
	sv.drm_di_major = 1;
	sv.drm_di_minor = 1;
	sv.drm_dd_major = -1;
	sv.drm_dd_minor = -1;
	err = drmSetInterfaceVersion(pAMDGPUEnt->fd, &sv);
	if (err != 0) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "[drm] failed to set drm interface version.\n");
		drmClose(pAMDGPUEnt->fd);
		return FALSE;
	}

	return TRUE;
}

static Bool amdgpu_get_scrninfo(int entity_num, struct pci_device *pci_dev)
{
	ScrnInfoPtr pScrn = NULL;
	char *busid;
	EntityInfoPtr pEnt;
	DevUnion *pPriv;
	AMDGPUEntPtr pAMDGPUEnt;

	pScrn = xf86ConfigPciEntity(pScrn, 0, entity_num, AMDGPUPciChipsets,
				    NULL, NULL, NULL, NULL, NULL);

	if (!pScrn)
		return FALSE;

	busid = amdgpu_bus_id(pScrn, pci_dev);
	if (!amdgpu_kernel_mode_enabled(pScrn, busid))
		goto error;

	pScrn->driverVersion = AMDGPU_VERSION_CURRENT;
	pScrn->driverName = AMDGPU_DRIVER_NAME;
	pScrn->name = AMDGPU_NAME;
	pScrn->Probe = NULL;

	pScrn->PreInit = AMDGPUPreInit_KMS;
	pScrn->ScreenInit = AMDGPUScreenInit_KMS;
	pScrn->SwitchMode = AMDGPUSwitchMode_KMS;
	pScrn->AdjustFrame = AMDGPUAdjustFrame_KMS;
	pScrn->EnterVT = AMDGPUEnterVT_KMS;
	pScrn->LeaveVT = AMDGPULeaveVT_KMS;
	pScrn->FreeScreen = AMDGPUFreeScreen_KMS;
	pScrn->ValidMode = AMDGPUValidMode;

	pEnt = xf86GetEntityInfo(entity_num);

	/* Create a AMDGPUEntity for all chips, even with old single head
	 * Radeon, need to use pAMDGPUEnt for new monitor detection routines.
	 */
	xf86SetEntitySharable(entity_num);

	if (gAMDGPUEntityIndex == -1)
		gAMDGPUEntityIndex = xf86AllocateEntityPrivateIndex();

	pPriv = xf86GetEntityPrivate(pEnt->index, gAMDGPUEntityIndex);

	if (!pPriv->ptr) {
		uint32_t major_version;
		uint32_t minor_version;

		pPriv->ptr = xnfcalloc(sizeof(AMDGPUEntRec), 1);
		if (!pPriv->ptr)
			goto error;

		pAMDGPUEnt = pPriv->ptr;
		if (!amdgpu_open_drm_master(pScrn, pAMDGPUEnt, busid))
			goto error_fd;

		pAMDGPUEnt->fd_ref = 1;

		if (amdgpu_device_initialize(pAMDGPUEnt->fd,
					     &major_version,
					     &minor_version,
					     &pAMDGPUEnt->pDev)) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "amdgpu_device_initialize failed\n");
			goto error_amdgpu;
		}
	} else {
		pAMDGPUEnt = pPriv->ptr;
		pAMDGPUEnt->fd_ref++;
	}

	xf86SetEntityInstanceForScreen(pScrn, pEnt->index,
				       xf86GetNumEntityInstances(pEnt->
								 index)
				       - 1);
	free(pEnt);
	free(busid);

	return TRUE;

error_amdgpu:
	drmClose(pAMDGPUEnt->fd);
error_fd:
	free(pPriv->ptr);
error:
	free(busid);
	return FALSE;
}

static Bool
amdgpu_pci_probe(DriverPtr pDriver,
		 int entity_num, struct pci_device *device, intptr_t match_data)
{
	return amdgpu_get_scrninfo(entity_num, device);
}

static Bool AMDGPUDriverFunc(ScrnInfoPtr scrn, xorgDriverFuncOp op, void *data)
{
	xorgHWFlags *flag;

	switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
		flag = (CARD32 *) data;
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

#ifdef XSERVER_PLATFORM_BUS
static Bool
amdgpu_platform_probe(DriverPtr pDriver,
		      int entity_num, int flags,
		      struct xf86_platform_device *dev, intptr_t match_data)
{
	ScrnInfoPtr pScrn;
	int scr_flags = 0;
	char *busid;
	EntityInfoPtr pEnt;
	DevUnion *pPriv;
	AMDGPUEntPtr pAMDGPUEnt;

	if (!dev->pdev)
		return FALSE;

	if (flags & PLATFORM_PROBE_GPU_SCREEN)
		scr_flags = XF86_ALLOCATE_GPU_SCREEN;

	pScrn = xf86AllocateScreen(pDriver, scr_flags);
	if (xf86IsEntitySharable(entity_num))
		xf86SetEntityShared(entity_num);
	xf86AddEntityToScreen(pScrn, entity_num);

	busid = amdgpu_bus_id(pScrn, dev->pdev);
	if (!busid)
		return FALSE;

	if (!amdgpu_kernel_mode_enabled(pScrn, busid))
		goto error;

	pScrn->driverVersion = AMDGPU_VERSION_CURRENT;
	pScrn->driverName = AMDGPU_DRIVER_NAME;
	pScrn->name = AMDGPU_NAME;
	pScrn->Probe = NULL;
	pScrn->PreInit = AMDGPUPreInit_KMS;
	pScrn->ScreenInit = AMDGPUScreenInit_KMS;
	pScrn->SwitchMode = AMDGPUSwitchMode_KMS;
	pScrn->AdjustFrame = AMDGPUAdjustFrame_KMS;
	pScrn->EnterVT = AMDGPUEnterVT_KMS;
	pScrn->LeaveVT = AMDGPULeaveVT_KMS;
	pScrn->FreeScreen = AMDGPUFreeScreen_KMS;
	pScrn->ValidMode = AMDGPUValidMode;

	pEnt = xf86GetEntityInfo(entity_num);

	/* Create a AMDGPUEntity for all chips, even with old single head
	 * Radeon, need to use pAMDGPUEnt for new monitor detection routines.
	 */
	xf86SetEntitySharable(entity_num);

	if (gAMDGPUEntityIndex == -1)
		gAMDGPUEntityIndex = xf86AllocateEntityPrivateIndex();

	pPriv = xf86GetEntityPrivate(pEnt->index, gAMDGPUEntityIndex);

	if (!pPriv->ptr) {
		uint32_t major_version;
		uint32_t minor_version;

		pPriv->ptr = xnfcalloc(sizeof(AMDGPUEntRec), 1);
		pAMDGPUEnt = pPriv->ptr;
		pAMDGPUEnt->fd = amdgpu_kernel_open_fd(pScrn, busid, dev);
		if (pAMDGPUEnt->fd < 0)
			goto error_fd;

		pAMDGPUEnt->fd_ref = 1;

		if (amdgpu_device_initialize(pAMDGPUEnt->fd,
					     &major_version,
					     &minor_version,
					     &pAMDGPUEnt->pDev)) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "amdgpu_device_initialize failed\n");
			goto error_amdgpu;
		}
	} else {
		pAMDGPUEnt = pPriv->ptr;
		pAMDGPUEnt->fd_ref++;
	}
	pAMDGPUEnt->platform_dev = dev;

	xf86SetEntityInstanceForScreen(pScrn, pEnt->index,
				       xf86GetNumEntityInstances(pEnt->
								 index)
				       - 1);
	free(pEnt);
	free(busid);

	return TRUE;

error_amdgpu:
	drmClose(pAMDGPUEnt->fd);
error_fd:
	free(pPriv->ptr);
error:
	free(busid);
	return FALSE;
}
#endif

_X_EXPORT DriverRec AMDGPU = {
	AMDGPU_VERSION_CURRENT,
	AMDGPU_DRIVER_NAME,
	AMDGPUIdentify,
	NULL,
	AMDGPUAvailableOptions,
	NULL,
	0,
	AMDGPUDriverFunc,
	amdgpu_device_match,
	amdgpu_pci_probe,
#ifdef XSERVER_PLATFORM_BUS
	amdgpu_platform_probe
#endif
};
