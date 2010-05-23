/*
 * Copyright 2008 Kristian Høgsberg 
 * Copyright 2008 Jérôme Glisse
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "radeon.h"
#include "radeon_dri2.h"
#include "radeon_version.h"

#ifdef RADEON_DRI2

#include "radeon_bo_gem.h"

#if DRI2INFOREC_VERSION >= 1
#define USE_DRI2_1_1_0
#endif

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,6,99,0, 0)
typedef DRI2BufferPtr BufferPtr;
#else
typedef DRI2Buffer2Ptr BufferPtr;
#endif

struct dri2_buffer_priv {
    PixmapPtr   pixmap;
    unsigned int attachment;
};


#ifndef USE_DRI2_1_1_0
static BufferPtr
radeon_dri2_create_buffers(DrawablePtr drawable,
                           unsigned int *attachments,
                           int count)
{
    ScreenPtr pScreen = drawable->pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    BufferPtr buffers;
    struct dri2_buffer_priv *privates;
    PixmapPtr pixmap, depth_pixmap;
    struct radeon_exa_pixmap_priv *driver_priv;
    int i, r;
    int flags = 0;

    buffers = xcalloc(count, sizeof *buffers);
    if (buffers == NULL) {
        return NULL;
    }
    privates = xcalloc(count, sizeof(struct dri2_buffer_priv));
    if (privates == NULL) {
        xfree(buffers);
        return NULL;
    }

    depth_pixmap = NULL;
    for (i = 0; i < count; i++) {
        if (attachments[i] == DRI2BufferFrontLeft) {
            if (drawable->type == DRAWABLE_PIXMAP) {
                pixmap = (Pixmap*)drawable;
            } else {
                pixmap = (*pScreen->GetWindowPixmap)((WindowPtr)drawable);
            }
            pixmap->refcnt++;
        } else if (attachments[i] == DRI2BufferStencil && depth_pixmap) {
            pixmap = depth_pixmap;
            pixmap->refcnt++;
        } else {
	    /* tile the back buffer */
	    switch(attachments[i]) {
	    case DRI2BufferDepth:
	    case DRI2BufferDepthStencil:
		flags = RADEON_CREATE_PIXMAP_TILING_MACRO | RADEON_CREATE_PIXMAP_TILING_MICRO;
		break;
	    case DRI2BufferBackLeft:
	    case DRI2BufferBackRight:
	    case DRI2BufferFakeFrontLeft:
	    case DRI2BufferFakeFrontRight:
		flags = RADEON_CREATE_PIXMAP_TILING_MACRO;
		break;
	    default:
		flags = 0;
	    }
	    pixmap = (*pScreen->CreatePixmap)(pScreen,
                                              drawable->width,
                                              drawable->height,
                                              drawable->depth,
                                              flags);
        }

        if (attachments[i] == DRI2BufferDepth) {
            depth_pixmap = pixmap;
        }
	info->exa_force_create = TRUE;
	exaMoveInPixmap(pixmap);
	info->exa_force_create = FALSE;
        driver_priv = exaGetPixmapDriverPrivate(pixmap);
	r = radeon_gem_get_kernel_name(driver_priv->bo, &buffers[i].name);
	if (r)
		return r;

        buffers[i].attachment = attachments[i];
        buffers[i].pitch = pixmap->devKind;
        buffers[i].cpp = pixmap->drawable.bitsPerPixel / 8;
        buffers[i].driverPrivate = &privates[i];
        buffers[i].flags = 0;
        privates[i].pixmap = pixmap;
        privates[i].attachment = attachments[i];
    }
    return buffers;
}
#else
static BufferPtr
radeon_dri2_create_buffer(DrawablePtr drawable,
                          unsigned int attachment,
                          unsigned int format)
{
    ScreenPtr pScreen = drawable->pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    BufferPtr buffers;
    struct dri2_buffer_priv *privates;
    PixmapPtr pixmap, depth_pixmap;
    struct radeon_exa_pixmap_priv *driver_priv;
    int r;
    int flags;

    buffers = xcalloc(1, sizeof *buffers);
    if (buffers == NULL) {
        return NULL;
    }
    privates = xcalloc(1, sizeof(struct dri2_buffer_priv));
    if (privates == NULL) {
        xfree(buffers);
        return NULL;
    }

    depth_pixmap = NULL;

    if (attachment == DRI2BufferFrontLeft) {
        if (drawable->type == DRAWABLE_PIXMAP) {
            pixmap = (PixmapPtr)drawable;
        } else {
            pixmap = (*pScreen->GetWindowPixmap)((WindowPtr)drawable);
        }
        pixmap->refcnt++;
    } else if (attachment == DRI2BufferStencil && depth_pixmap) {
        pixmap = depth_pixmap;
        pixmap->refcnt++;
    } else {
	/* tile the back buffer */
	switch(attachment) {
	case DRI2BufferDepth:
	case DRI2BufferDepthStencil:
	    flags = RADEON_CREATE_PIXMAP_TILING_MACRO | RADEON_CREATE_PIXMAP_TILING_MICRO;
	    break;
	case DRI2BufferBackLeft:
	case DRI2BufferBackRight:
	case DRI2BufferFakeFrontLeft:
	case DRI2BufferFakeFrontRight:
	    flags = RADEON_CREATE_PIXMAP_TILING_MACRO;
	    break;
	default:
	    flags = 0;
	}
        pixmap = (*pScreen->CreatePixmap)(pScreen,
                drawable->width,
                drawable->height,
                (format != 0)?format:drawable->depth,
                flags);
    }

    if (attachment == DRI2BufferDepth) {
        depth_pixmap = pixmap;
    }
    info->exa_force_create = TRUE;
    exaMoveInPixmap(pixmap);
    info->exa_force_create = FALSE;
    driver_priv = exaGetPixmapDriverPrivate(pixmap);
    r = radeon_gem_get_kernel_name(driver_priv->bo, &buffers->name);
    if (r)
	    return NULL;

    buffers->attachment = attachment;
    buffers->pitch = pixmap->devKind;
    buffers->cpp = pixmap->drawable.bitsPerPixel / 8;
    buffers->driverPrivate = privates;
    buffers->format = format;
    buffers->flags = 0; /* not tiled */
    privates->pixmap = pixmap;
    privates->attachment = attachment;

    return buffers;
}
#endif

#ifndef USE_DRI2_1_1_0
static void
radeon_dri2_destroy_buffers(DrawablePtr drawable,
                            BufferPtr buffers,
                            int count)
{
    ScreenPtr pScreen = drawable->pScreen;
    struct dri2_buffer_priv *private;
    int i;

    for (i = 0; i < count; i++) {
        private = buffers[i].driverPrivate;
        (*pScreen->DestroyPixmap)(private->pixmap);
    }
    if (buffers) {
        xfree(buffers[0].driverPrivate);
        xfree(buffers);
    }
}
#else
static void
radeon_dri2_destroy_buffer(DrawablePtr drawable, BufferPtr buffers)
{
    if(buffers)
    {
        ScreenPtr pScreen = drawable->pScreen;
        struct dri2_buffer_priv *private;

        private = buffers->driverPrivate;
        (*pScreen->DestroyPixmap)(private->pixmap);

        xfree(buffers->driverPrivate);
        xfree(buffers);
    }
}
#endif

static void
radeon_dri2_copy_region(DrawablePtr drawable,
                        RegionPtr region,
                        BufferPtr dest_buffer,
                        BufferPtr src_buffer)
{
    struct dri2_buffer_priv *src_private = src_buffer->driverPrivate;
    struct dri2_buffer_priv *dst_private = dest_buffer->driverPrivate;
    ScreenPtr pScreen = drawable->pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    PixmapPtr src_pixmap;
    PixmapPtr dst_pixmap;
    RegionPtr copy_clip;
    GCPtr gc;
    RADEONInfoPtr info = RADEONPTR(pScrn);
    Bool vsync;

    src_pixmap = src_private->pixmap;
    dst_pixmap = dst_private->pixmap;
    if (src_private->attachment == DRI2BufferFrontLeft) {
        src_pixmap = (PixmapPtr)drawable;
    }
    if (dst_private->attachment == DRI2BufferFrontLeft) {
        dst_pixmap = (PixmapPtr)drawable;
    }
    gc = GetScratchGC(drawable->depth, pScreen);
    copy_clip = REGION_CREATE(pScreen, NULL, 0);
    REGION_COPY(pScreen, copy_clip, region);
    (*gc->funcs->ChangeClip) (gc, CT_REGION, copy_clip, 0);
    ValidateGC(&dst_pixmap->drawable, gc);

    /* If this is a full buffer swap or frontbuffer flush, throttle on the
     * previous one
     */
    if (dst_private->attachment == DRI2BufferFrontLeft) {
	if (REGION_NUM_RECTS(region) == 1) {
	    BoxPtr extents = REGION_EXTENTS(pScreen, region);

	    if (extents->x1 == 0 && extents->y1 == 0 &&
		extents->x2 == drawable->width &&
		extents->y2 == drawable->height) {
		struct radeon_exa_pixmap_priv *exa_priv =
		    exaGetPixmapDriverPrivate(dst_pixmap);

		if (exa_priv && exa_priv->bo)
		    radeon_bo_wait(exa_priv->bo);
	    }
	}
    }

    vsync = info->accel_state->vsync;
    info->accel_state->vsync = TRUE;

    (*gc->ops->CopyArea)(&src_pixmap->drawable, &dst_pixmap->drawable, gc,
                         0, 0, drawable->width, drawable->height, 0, 0);

    info->accel_state->vsync = vsync;

    FreeScratchGC(gc);
    radeon_cs_flush_indirect(pScrn);
}

Bool
radeon_dri2_screen_init(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    DRI2InfoRec dri2_info = { 0 };

    if (!info->useEXA) {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "DRI2 requires EXA\n");
        return FALSE;
    }

    info->dri2.device_name = drmGetDeviceNameFromFd(info->dri2.drm_fd);

    if ( (info->ChipFamily >= CHIP_FAMILY_R600) ) {
        dri2_info.driverName = R600_DRIVER_NAME;
    } else if ( (info->ChipFamily >= CHIP_FAMILY_R300) ) {
        dri2_info.driverName = R300_DRIVER_NAME;
    } else if ( info->ChipFamily >= CHIP_FAMILY_R200 ) {
        dri2_info.driverName = R200_DRIVER_NAME;
    } else {
        dri2_info.driverName = RADEON_DRIVER_NAME;
    }
    dri2_info.fd = info->dri2.drm_fd;
    dri2_info.deviceName = info->dri2.device_name;
#ifndef USE_DRI2_1_1_0
    dri2_info.version = 1;
    dri2_info.CreateBuffers = radeon_dri2_create_buffers;
    dri2_info.DestroyBuffers = radeon_dri2_destroy_buffers;
#else
    dri2_info.version = DRI2INFOREC_VERSION;
    dri2_info.CreateBuffer = radeon_dri2_create_buffer;
    dri2_info.DestroyBuffer = radeon_dri2_destroy_buffer;
#endif
    dri2_info.CopyRegion = radeon_dri2_copy_region;
    info->dri2.enabled = DRI2ScreenInit(pScreen, &dri2_info);
    return info->dri2.enabled;
}

void radeon_dri2_close_screen(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);

    DRI2CloseScreen(pScreen);
    drmFree(info->dri2.device_name);
}

#endif
