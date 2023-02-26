/*
 * Copyright 2013-2014 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef XF86DRM_MODE
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include "qxl.h"

#include "qxl_surface.h"

Bool qxl_kms_check_cap(qxl_screen_t *qxl, int idx)
{
    int ret;
    struct drm_qxl_clientcap cap;

    cap.index = idx;
    ret = drmIoctl(qxl->drm_fd, DRM_IOCTL_QXL_CLIENTCAP, &cap);
    if (ret == 0)
	return TRUE;
    return FALSE;
}

#if 0
static Bool qxl_kms_getparam(qxl_screen_t *qxl, uint64_t param, uint64_t *value)
{
    int ret;
    struct drm_qxl_getparam args = {0};

    args.param = param;
    ret = drmIoctl(qxl->drm_fd, DRM_IOCTL_QXL_GETPARAM, &args);
    if (ret != 0)
	return FALSE;

    *value = args.value;
    return TRUE;
}
#endif

static Bool qxl_open_drm_master(ScrnInfoPtr pScrn)
{
    qxl_screen_t *qxl = pScrn->driverPrivate;
    struct pci_device *dev = qxl->pci;
    char *busid;
    drmSetVersion sv;
    int err;

#if defined(ODEV_ATTRIB_FD)
    if (qxl->platform_dev) {
        qxl->drm_fd = xf86_get_platform_device_int_attrib(qxl->platform_dev,
                                                          ODEV_ATTRIB_FD, -1);
        if (qxl->drm_fd != -1) {
            qxl->drmmode.fd = qxl->drm_fd;
            return TRUE;
        }
    }
#endif

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,9,99,901,0)
    XNFasprintf(&busid, "pci:%04x:%02x:%02x.%d",
                dev->domain, dev->bus, dev->dev, dev->func);
#else
    busid = XNFprintf("pci:%04x:%02x:%02x.%d",
		      dev->domain, dev->bus, dev->dev, dev->func);
#endif

    qxl->drm_fd = drmOpen("qxl", busid);
    if (qxl->drm_fd == -1) {

	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "[drm] Failed to open DRM device for %s: %s\n",
		   busid, strerror(errno));
	free(busid);
	return FALSE;
    }
    free(busid);

    /* Check that what we opened was a master or a master-capable FD,
     * by setting the version of the interface we'll use to talk to it.
     * (see DRIOpenDRMMaster() in DRI1)
     */
    sv.drm_di_major = 1;
    sv.drm_di_minor = 1;
    sv.drm_dd_major = -1;
    sv.drm_dd_minor = -1;
    err = drmSetInterfaceVersion(qxl->drm_fd, &sv);
    if (err != 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "[drm] failed to set drm interface version.\n");
	drmClose(qxl->drm_fd);
	qxl->drm_fd = -1;

	return FALSE;
    }

    qxl->drmmode.fd = qxl->drm_fd;
    return TRUE;
}

static Bool
qxl_close_screen_kms (CLOSE_SCREEN_ARGS_DECL)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn (pScreen);
    qxl_screen_t *qxl = pScrn->driverPrivate;
    Bool result;

    qxl_drmmode_uevent_fini(pScrn, &qxl->drmmode);
    pScreen->CloseScreen = qxl->close_screen;

    result = pScreen->CloseScreen (CLOSE_SCREEN_ARGS);

    return result;
}


Bool qxl_pre_init_kms(ScrnInfoPtr pScrn, int flags)
{
    int           scrnIndex = pScrn->scrnIndex;
    qxl_screen_t *qxl = NULL;

    if (!pScrn->confScreen)
	return FALSE;

    /* zaphod mode is for suckers and i choose not to implement it */
    if (xf86IsEntityShared (pScrn->entityList[0]))
    {
	xf86DrvMsg (scrnIndex, X_ERROR, "No Zaphod mode for you\n");
	return FALSE;
    }
    
    if (!pScrn->driverPrivate)
	pScrn->driverPrivate = xnfcalloc (sizeof (qxl_screen_t), 1);

    qxl = pScrn->driverPrivate;
    qxl->device_primary = QXL_DEVICE_PRIMARY_UNDEFINED;
    qxl->pScrn = pScrn;
    qxl->x_modes = NULL;
    qxl->entity = xf86GetEntityInfo (pScrn->entityList[0]);
    qxl->kms_enabled = TRUE;
    xorg_list_init(&qxl->ums_bos);

    qxl_kms_setup_funcs(qxl);
    qxl->pci = xf86GetPciInfoForEntity (qxl->entity->index);

    pScrn->monitor = pScrn->confScreen->monitor;

    if (qxl_open_drm_master(pScrn) == FALSE) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Kernel modesetting setup failed\n");
	goto out;
    }

    if (!qxl_pre_init_common(pScrn))
	goto out;

    xf86SetDpi (pScrn, 0, 0);

    if (!xf86LoadSubModule (pScrn, "fb"))
	goto out;

    if (!xf86LoadSubModule (pScrn, "ramdac"))
	goto out;

    if (drmmode_pre_init(pScrn, &qxl->drmmode, pScrn->bitsPerPixel / 8) == FALSE)
      goto out;

    qxl->virtual_x = pScrn->virtualX;
    qxl->virtual_y = pScrn->virtualY;
    
    pScrn->display->virtualX = qxl->virtual_x;
    pScrn->display->virtualY = qxl->virtual_y;

    xf86DrvMsg (scrnIndex, X_INFO, "PreInit complete\n");
#ifdef GIT_VERSION
    xf86DrvMsg (scrnIndex, X_INFO, "git commit %s\n", GIT_VERSION);
#endif

    return TRUE;

 out:
    if (qxl)
      free(qxl);
    return FALSE;
}

static Bool
qxl_create_screen_resources_kms(ScreenPtr pScreen)
{
    ScrnInfoPtr    pScrn = xf86ScreenToScrn (pScreen);
    qxl_screen_t * qxl = pScrn->driverPrivate;
    Bool           ret;
    PixmapPtr      pPixmap;
    qxl_surface_t *surf;
    
    pScreen->CreateScreenResources = qxl->create_screen_resources;
    ret = pScreen->CreateScreenResources (pScreen);
    pScreen->CreateScreenResources = qxl_create_screen_resources_kms;
    
    if (!ret)
	return FALSE;
    
    pPixmap = pScreen->GetScreenPixmap (pScreen);
    
    qxl_set_screen_pixmap_header (pScreen);
    
    if ((surf = get_surface (pPixmap)))
        qxl->bo_funcs->destroy_surface(surf);
    
    set_surface (pPixmap, qxl->primary);

    qxl_drmmode_uevent_init(pScrn, &qxl->drmmode);

    if (!uxa_resources_init (pScreen))
	return FALSE;
    
    qxl->screen_resources_created = TRUE;
    return TRUE;
}

static Bool
qxl_blank_screen (ScreenPtr pScreen, int mode)
{
    return TRUE;
}

Bool
qxl_enter_vt_kms (VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR (arg);
    qxl_screen_t *qxl = pScrn->driverPrivate;
    int ret;

#ifdef XF86_PDEV_SERVER_FD
    if (!(qxl->platform_dev &&
            (qxl->platform_dev->flags & XF86_PDEV_SERVER_FD)))
#endif
    {
        ret = drmSetMaster(qxl->drm_fd);
        if (ret) {
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
                       "drmSetMaster failed: %s\n",
                       strerror(errno));
        }
    }

    if (!xf86SetDesiredModes(pScrn))
	return FALSE;

    //    pScrn->EnableDisableFBAccess (XF86_SCRN_ARG (pScrn), TRUE);
    return TRUE;
}

void
qxl_leave_vt_kms (VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR (arg); 
    int ret;
    qxl_screen_t *qxl = pScrn->driverPrivate;
    xf86_hide_cursors (pScrn);
    //    pScrn->EnableDisableFBAccess (XF86_SCRN_ARG (pScrn), FALSE);

#ifdef XF86_PDEV_SERVER_FD
    if (qxl->platform_dev && (qxl->platform_dev->flags & XF86_PDEV_SERVER_FD))
        return;
#endif

    ret = drmDropMaster(qxl->drm_fd);
    if (ret) {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "drmDropMaster failed: %s\n",
		   strerror(errno));
    }
}


Bool qxl_screen_init_kms(SCREEN_INIT_ARGS_DECL)
{
    ScrnInfoPtr    pScrn = xf86ScreenToScrn (pScreen);
    qxl_screen_t * qxl = pScrn->driverPrivate;
    VisualPtr      visual;

    miClearVisualTypes ();
    if (!miSetVisualTypes (pScrn->depth, miGetDefaultVisualMask (pScrn->depth),
                           pScrn->rgbBits, pScrn->defaultVisual))
	goto out;
    if (!miSetPixmapDepths ())
	goto out;
    pScrn->displayWidth = pScrn->virtualX;

    if (!qxl_fb_init (qxl, pScreen))
	goto out;
    
    visual = pScreen->visuals + pScreen->numVisuals;
    while (--visual >= pScreen->visuals)
    {
	if ((visual->class | DynamicClass) == DirectColor)
	{
	    visual->offsetRed = pScrn->offset.red;
	    visual->offsetGreen = pScrn->offset.green;
	    visual->offsetBlue = pScrn->offset.blue;
	    visual->redMask = pScrn->mask.red;
	    visual->greenMask = pScrn->mask.green;
	    visual->blueMask = pScrn->mask.blue;
	}
    }
    
    qxl->uxa = uxa_driver_alloc ();

// GETPARAM
    /* no surface cache for kms surfaces for now */
#if 0
    if (!qxl_kms_getparam(qxl, QXL_PARAM_NUM_SURFACES, &n_surf))
	n_surf = 1024;
    qxl->surface_cache = qxl_surface_cache_create (qxl, n_surf);
#endif
    pScreen->SaveScreen = qxl_blank_screen;

    qxl_uxa_init (qxl, pScreen);

    DamageSetup (pScreen);

    miDCInitialize (pScreen, xf86GetPointerScreenFuncs());

    xf86_cursors_init (pScreen, 64, 64,
		       (HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
			HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
			HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1 |
			HARDWARE_CURSOR_UPDATE_UNHIDDEN |
			HARDWARE_CURSOR_ARGB));

    if (!miCreateDefColormap (pScreen))
        goto out;

    if (!xf86CrtcScreenInit (pScreen))
	return FALSE;

    if (!qxl_resize_primary_to_virtual (qxl))
	return FALSE;

    qxl->create_screen_resources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = qxl_create_screen_resources_kms;

    qxl->close_screen = pScreen->CloseScreen;
    pScreen->CloseScreen = qxl_close_screen_kms;

    return qxl_enter_vt_kms(VT_FUNC_ARGS);
 out:
    return FALSE;

}

#define QXL_BO_DATA 1
#define QXL_BO_SURF 2
#define QXL_BO_CMD 4
#define QXL_BO_SURF_PRIMARY 8

struct qxl_kms_bo {
    uint32_t handle;
    const char *name;
    uint32_t size;
    int type;
    xorg_list_t bos;
    void *mapping;
    qxl_screen_t *qxl;
    int refcnt;
};

static struct qxl_bo *qxl_bo_alloc(qxl_screen_t *qxl,
				   unsigned long size, const char *name)
{
    struct qxl_kms_bo *bo;
    struct drm_qxl_alloc alloc;
    int ret;

    bo = calloc(1, sizeof(struct qxl_kms_bo));
    if (!bo)
	return NULL;

    alloc.size = size;
    alloc.handle = 0;

    ret = drmIoctl(qxl->drm_fd, DRM_IOCTL_QXL_ALLOC, &alloc);
    if (ret) {
        xf86DrvMsg(qxl->pScrn->scrnIndex, X_ERROR,
                   "error doing QXL_ALLOC\n");
	free(bo);
        return NULL; // an invalid handle
    }

    bo->name = name;
    bo->size = size;
    bo->type = QXL_BO_DATA;
    bo->handle = alloc.handle;
    bo->qxl = qxl;
    bo->refcnt = 1;
    return (struct qxl_bo *)bo;
}

static struct qxl_bo *qxl_cmd_alloc(qxl_screen_t *qxl,
				    unsigned long size, const char *name)
{
    struct qxl_kms_bo *bo;

    bo = calloc(1, sizeof(struct qxl_kms_bo));
    if (!bo)
	return NULL;
    bo->mapping = malloc(size);
    if (!bo->mapping) {
	free(bo);
	return NULL;
    }
    bo->name = name;
    bo->size = size;
    bo->type = QXL_BO_CMD;
    bo->handle = 0;
    bo->qxl = qxl;
    bo->refcnt = 1;
    return (struct qxl_bo *)bo;
}

static void *qxl_bo_map(struct qxl_bo *_bo)
{
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)_bo;
    void *map;
    struct drm_qxl_map qxl_map;
    qxl_screen_t *qxl;

    if (!bo)
	return NULL;

    qxl = bo->qxl;
    if (bo->mapping)
	return bo->mapping;

    memset(&qxl_map, 0, sizeof(qxl_map));

    qxl_map.handle = bo->handle;
    
    if (drmIoctl(qxl->drm_fd, DRM_IOCTL_QXL_MAP, &qxl_map)) {
	xf86DrvMsg(qxl->pScrn->scrnIndex, X_ERROR,
                   "error doing QXL_MAP: %s\n", strerror(errno));
        return NULL;
    }

    map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, qxl->drm_fd,
               qxl_map.offset);
    if (map == MAP_FAILED) {
        xf86DrvMsg(qxl->pScrn->scrnIndex, X_ERROR,
                   "mmap failure: %s\n", strerror(errno));
        return NULL;
    }

    bo->mapping = map;
    return bo->mapping;
}

static void qxl_bo_unmap(struct qxl_bo *_bo)
{
}

static void qxl_bo_incref(qxl_screen_t *qxl, struct qxl_bo *_bo)
{
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)_bo;
    bo->refcnt++;
}

static void qxl_bo_decref(qxl_screen_t *qxl, struct qxl_bo *_bo)
{
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)_bo;
    struct drm_gem_close args;
    int ret;

    bo->refcnt--;
    if (bo->refcnt > 0)
	return;

    if (bo->type == QXL_BO_CMD) {
	free(bo->mapping);
	goto out;
    } else if (bo->mapping)
	munmap(bo->mapping, bo->size);
	
    /* just close the handle */
    args.handle = bo->handle;
    ret = drmIoctl(qxl->drm_fd, DRM_IOCTL_GEM_CLOSE, &args);
    if (ret) {
        xf86DrvMsg(qxl->pScrn->scrnIndex, X_ERROR,
                   "error doing QXL_DECREF\n");
    }
 out:
    free(bo);
}

static void qxl_bo_output_bo_reloc(qxl_screen_t *qxl, uint32_t dst_offset,
				struct qxl_bo *_dst_bo,
				struct qxl_bo *_src_bo)
{
    struct qxl_kms_bo *dst_bo = (struct qxl_kms_bo *)_dst_bo;
    struct qxl_kms_bo *src_bo = (struct qxl_kms_bo *)_src_bo;
    struct drm_qxl_reloc *r = &qxl->cmds.relocs[qxl->cmds.n_relocs];
    
    if (qxl->cmds.n_reloc_bos >= MAX_RELOCS || qxl->cmds.n_relocs >= MAX_RELOCS)
      assert(0);

    qxl->cmds.reloc_bo[qxl->cmds.n_reloc_bos] = _src_bo;
    qxl->cmds.n_reloc_bos++;
    src_bo->refcnt++;
      
    /* fix the kernel names */
    r->reloc_type = QXL_RELOC_TYPE_BO;
    r->dst_handle = dst_bo->handle;
    r->src_handle = src_bo->handle;
    r->dst_offset = dst_offset;
    r->src_offset = 0;
    qxl->cmds.n_relocs++;
}

static void qxl_bo_write_command(qxl_screen_t *qxl, uint32_t cmd_type, struct qxl_bo *_bo)
{
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)_bo;
    struct drm_qxl_execbuffer eb;
    struct drm_qxl_command c;
    int ret;
    int i;

    c.type = cmd_type;
    c.command_size = bo->size - sizeof(union QXLReleaseInfo);
    c.command = pointer_to_u64(((uint8_t *)bo->mapping + sizeof(union QXLReleaseInfo)));
    if (qxl->cmds.n_relocs) {
	c.relocs_num = qxl->cmds.n_relocs;
	c.relocs = pointer_to_u64(qxl->cmds.relocs);
    } else {
	c.relocs_num = 0;
	c.relocs = 0;
    }
    eb.flags = 0;
    eb.commands_num = 1;
    eb.commands = pointer_to_u64(&c);
    ret = drmIoctl(qxl->drm_fd, DRM_IOCTL_QXL_EXECBUFFER, &eb);
    if (ret) {
        xf86DrvMsg(qxl->pScrn->scrnIndex, X_ERROR,
                   "EXECBUFFER failed\n");
    }
    qxl->cmds.n_relocs = 0;
    qxl->bo_funcs->bo_decref(qxl, _bo);

    for (i = 0; i < qxl->cmds.n_reloc_bos; i++)
      qxl->bo_funcs->bo_decref(qxl, qxl->cmds.reloc_bo[i]);
    qxl->cmds.n_reloc_bos = 0;
}

static void qxl_bo_update_area(qxl_surface_t *surf, int x1, int y1, int x2, int y2)
{
    int ret;
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)surf->bo;
    struct drm_qxl_update_area update_area = {
        .handle = bo->handle,
        .left = x1,
        .top = y1,
        .right = x2,
        .bottom = y2
    };

    ret = drmIoctl(surf->qxl->drm_fd,
                   DRM_IOCTL_QXL_UPDATE_AREA, &update_area);
    if (ret) {
        fprintf(stderr, "error doing QXL_UPDATE_AREA %d %d %d\n", ret, errno, surf->id);
    }
}

static struct qxl_bo *qxl_bo_create_primary(qxl_screen_t *qxl, uint32_t width, uint32_t height, int32_t stride, uint32_t format)
{
    struct qxl_kms_bo *bo;
    struct drm_qxl_alloc_surf param;
    int ret;

    bo = calloc(1, sizeof(struct qxl_kms_bo));
    if (!bo)
	return NULL;

    param.format = SPICE_SURFACE_FMT_32_xRGB;
    param.width = width;
    param.height = height;
    param.stride = stride;
    param.handle = 0;
    ret = drmIoctl(qxl->drm_fd,
		   DRM_IOCTL_QXL_ALLOC_SURF, &param);
    if (ret)
	return NULL;

    bo->name = "surface memory";
    bo->size = stride * param.height;
    bo->type = QXL_BO_SURF_PRIMARY;
    bo->handle = param.handle;
    bo->qxl = qxl;
    bo->refcnt = 1;

    qxl->primary_bo = (struct qxl_bo *)bo;
    qxl->device_primary = QXL_DEVICE_PRIMARY_CREATED;
    return (struct qxl_bo *)bo;
}

static void qxl_bo_destroy_primary(qxl_screen_t *qxl, struct qxl_bo *bo)
{
    qxl_bo_decref(qxl, bo);

    qxl->primary_bo = NULL;
    qxl->device_primary = QXL_DEVICE_PRIMARY_NONE;
}

static qxl_surface_t *
qxl_kms_surface_create(qxl_screen_t *qxl,
		       int width,
		       int height,
		       int bpp)
{
    SpiceSurfaceFmt format;
    qxl_surface_t *surface;
    int stride;
    struct qxl_kms_bo *bo;
    pixman_format_code_t pformat;
    void *dev_ptr;
    int ret;
    uint32_t *dev_addr;

    struct drm_qxl_alloc_surf param;
    if (!qxl->enable_surfaces)
	return NULL;

    if ((bpp & 3) != 0)
    {
	ErrorF ("%s: Bad bpp: %d (%d)\n", __FUNCTION__, bpp, bpp & 7);
	return NULL;
    }

    if (bpp != 8 && bpp != 16 && bpp != 32 && bpp != 24)
    {
	ErrorF ("%s: Unknown bpp\n", __FUNCTION__);
	return NULL;
    }

    if (width == 0 || height == 0)
    {
	ErrorF ("%s: Zero width or height\n", __FUNCTION__);
	return NULL;
    }

    qxl_get_formats (bpp, &format, &pformat);
    stride = width * PIXMAN_FORMAT_BPP (pformat) / 8;
    stride = (stride + 3) & ~3;

    bo = calloc(1, sizeof(struct qxl_kms_bo));
    if (!bo)
	return NULL;

    param.format = format;
    param.width = width;
    param.height = height;
    param.stride = -stride;
    param.handle = 0;
    ret = drmIoctl(qxl->drm_fd,
		   DRM_IOCTL_QXL_ALLOC_SURF, &param);
    if (ret)
	return NULL;

    bo->name = "surface memory";
    bo->size = stride * height + stride;
    bo->type = QXL_BO_SURF;
    bo->handle = param.handle;
    bo->qxl = qxl;
    bo->refcnt = 1;

    /* then fill out the driver surface */
    surface = calloc(1, sizeof *surface);
    surface->bo = (struct qxl_bo *)bo;
    surface->qxl = qxl;
    surface->id = bo->handle;
    surface->image_bo = NULL;
    dev_ptr = qxl->bo_funcs->bo_map(surface->bo);
    dev_addr
	= (uint32_t *)((uint8_t *)dev_ptr + stride * (height - 1));
    surface->dev_image = pixman_image_create_bits (
		   pformat, width, height, dev_addr, - stride);

    surface->host_image = pixman_image_create_bits (
	pformat, width, height, NULL, -1);
    REGION_INIT (NULL, &(surface->access_region), (BoxPtr)NULL, 0);
    qxl->bo_funcs->bo_unmap(surface->bo);
    surface->access_type = UXA_ACCESS_RO;
    surface->bpp = bpp;

    return surface;
}

static void qxl_kms_surface_destroy(qxl_surface_t *surf)
{
    qxl_screen_t *qxl = surf->qxl;

    if (surf->dev_image)
	pixman_image_unref (surf->dev_image);
    if (surf->host_image)
	pixman_image_unref (surf->host_image);

    if (surf->image_bo)
      qxl->bo_funcs->bo_decref(qxl, surf->image_bo);
    qxl->bo_funcs->bo_decref(qxl, surf->bo);
    free(surf);
}

static void qxl_bo_output_surf_reloc(qxl_screen_t *qxl, uint32_t dst_offset,
				     struct qxl_bo *_dst_bo, qxl_surface_t *surf)
{
    struct qxl_kms_bo *dst_bo = (struct qxl_kms_bo *)_dst_bo;
    struct drm_qxl_reloc *r = &qxl->cmds.relocs[qxl->cmds.n_relocs];
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)surf->bo;
    if (qxl->cmds.n_reloc_bos >= MAX_RELOCS || qxl->cmds.n_relocs >= MAX_RELOCS)
	assert(0);

    qxl->cmds.reloc_bo[qxl->cmds.n_reloc_bos] = surf->bo;
    qxl->cmds.n_reloc_bos++;
    bo->refcnt++;

    /* fix the kernel names */
    r->reloc_type = QXL_RELOC_TYPE_SURF;
    r->dst_handle = dst_bo->handle;
    r->src_handle = bo->handle;
    r->dst_offset = dst_offset;
    r->src_offset = 0;
    qxl->cmds.n_relocs++;
}

static struct qxl_bo_funcs qxl_kms_bo_funcs = {
    qxl_bo_alloc,
    qxl_cmd_alloc,
    qxl_bo_map,
    qxl_bo_unmap,
    qxl_bo_decref,
    qxl_bo_incref,
    qxl_bo_output_bo_reloc,
    qxl_bo_write_command,
    qxl_bo_update_area,
    qxl_bo_create_primary,
    qxl_bo_destroy_primary,
    qxl_kms_surface_create,
    qxl_kms_surface_destroy,
    qxl_bo_output_surf_reloc,
};

void qxl_kms_setup_funcs(qxl_screen_t *qxl)
{
    qxl->bo_funcs = &qxl_kms_bo_funcs;
}

uint32_t qxl_kms_bo_get_handle(struct qxl_bo *_bo)
{
    struct qxl_kms_bo *bo = (struct qxl_kms_bo *)_bo;
    
    return bo->handle;
}
#endif
