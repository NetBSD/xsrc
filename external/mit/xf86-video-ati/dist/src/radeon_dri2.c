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

    buffers = calloc(count, sizeof *buffers);
    if (buffers == NULL) {
        return NULL;
    }
    privates = calloc(count, sizeof(struct dri2_buffer_priv));
    if (privates == NULL) {
        free(buffers);
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

    buffers = calloc(1, sizeof *buffers);
    if (buffers == NULL) {
        return NULL;
    }
    privates = calloc(1, sizeof(struct dri2_buffer_priv));
    if (privates == NULL) {
        free(buffers);
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
        free(buffers[0].driverPrivate);
        free(buffers);
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

        free(buffers->driverPrivate);
        free(buffers);
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
    DrawablePtr src_drawable;
    DrawablePtr dst_drawable;
    RegionPtr copy_clip;
    GCPtr gc;
    RADEONInfoPtr info = RADEONPTR(pScrn);
    Bool vsync;

    if (src_private->attachment == DRI2BufferFrontLeft) {
        src_drawable = drawable;
    } else {
        src_drawable = &src_private->pixmap->drawable;
    }
    if (dst_private->attachment == DRI2BufferFrontLeft) {
        dst_drawable = drawable;
    } else {
        dst_drawable = &dst_private->pixmap->drawable;
    }
    gc = GetScratchGC(dst_drawable->depth, pScreen);
    copy_clip = REGION_CREATE(pScreen, NULL, 0);
    REGION_COPY(pScreen, copy_clip, region);
    (*gc->funcs->ChangeClip) (gc, CT_REGION, copy_clip, 0);
    ValidateGC(dst_drawable, gc);

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
		    exaGetPixmapDriverPrivate(dst_private->pixmap);

		if (exa_priv && exa_priv->bo)
		    radeon_bo_wait(exa_priv->bo);
	    }
	}
    }

    vsync = info->accel_state->vsync;
    info->accel_state->vsync = TRUE;

    (*gc->ops->CopyArea)(src_drawable, dst_drawable, gc,
                         0, 0, drawable->width, drawable->height, 0, 0);

    info->accel_state->vsync = vsync;

    FreeScratchGC(gc);
    radeon_cs_flush_indirect(pScrn);
}


#if DRI2INFOREC_VERSION >= 4

enum DRI2FrameEventType {
    DRI2_SWAP,
    DRI2_FLIP,
    DRI2_WAITMSC,
};

typedef struct _DRI2FrameEvent {
    XID drawable_id;
    ClientPtr client;
    enum DRI2FrameEventType type;
    int frame;

    /* for swaps & flips only */
    DRI2SwapEventPtr event_complete;
    void *event_data;
    DRI2BufferPtr front;
    DRI2BufferPtr back;
} DRI2FrameEventRec, *DRI2FrameEventPtr;

void radeon_dri2_frame_event_handler(unsigned int frame, unsigned int tv_sec,
                                     unsigned int tv_usec, void *event_data)
{
    DRI2FrameEventPtr event = event_data;
    DrawablePtr drawable;
    ScreenPtr screen;
    ScrnInfoPtr scrn;
    int status;
    int swap_type;
    BoxRec box;
    RegionRec region;

    status = dixLookupDrawable(&drawable, event->drawable_id, serverClient,
                               M_ANY, DixWriteAccess);
    if (status != Success) {
        free(event);
        return;
    }

    screen = drawable->pScreen;
    scrn = xf86Screens[screen->myNum];

    switch (event->type) {
    case DRI2_FLIP:
    case DRI2_SWAP:
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = drawable->width;
        box.y2 = drawable->height;
        REGION_INIT(pScreen, &region, &box, 0);
        radeon_dri2_copy_region(drawable, &region, event->front, event->back);
        swap_type = DRI2_BLIT_COMPLETE;

        DRI2SwapComplete(event->client, drawable, frame, tv_sec, tv_usec,
                swap_type, event->event_complete, event->event_data);
        break;
    case DRI2_WAITMSC:
        DRI2WaitMSCComplete(event->client, drawable, frame, tv_sec, tv_usec);
        break;
    default:
        /* Unknown type */
        xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                "%s: unknown vblank event received\n", __func__);
        break;
    }

    free(event);
}

static int radeon_dri2_drawable_crtc(DrawablePtr pDraw)
{
    ScreenPtr pScreen = pDraw->pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcPtr crtc;
    int crtc_id = -1;

    crtc = radeon_pick_best_crtc(pScrn,
				 pDraw->x,
				 pDraw->x + pDraw->width,
				 pDraw->y,
				 pDraw->y + pDraw->height);

    /* Make sure the CRTC is valid and this is the real front buffer */
    if (crtc != NULL && !crtc->rotatedData) {
        crtc_id = drmmode_get_crtc_id(crtc);
    }
    return crtc_id;
}

/*
 * Get current frame count and frame count timestamp, based on drawable's
 * crtc.
 */
static int radeon_dri2_get_msc(DrawablePtr draw, CARD64 *ust, CARD64 *msc)
{
    ScreenPtr screen = draw->pScreen;
    ScrnInfoPtr scrn = xf86Screens[screen->myNum];
    RADEONInfoPtr info = RADEONPTR(scrn);
    drmVBlank vbl;
    int ret;
    int crtc= radeon_dri2_drawable_crtc(draw);

    /* Drawable not displayed, make up a value */
    if (crtc == -1) {
        *ust = 0;
        *msc = 0;
        return TRUE;
    }
    vbl.request.type = DRM_VBLANK_RELATIVE;
    if (crtc > 0)
        vbl.request.type |= DRM_VBLANK_SECONDARY;
    vbl.request.sequence = 0;

    ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
    if (ret) {
        xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                "get vblank counter failed: %s\n", strerror(errno));
        return FALSE;
    }

    *ust = ((CARD64)vbl.reply.tval_sec * 1000000) + vbl.reply.tval_usec;
    *msc = vbl.reply.sequence;

    return TRUE;
}

/*
 * Request a DRM event when the requested conditions will be satisfied.
 *
 * We need to handle the event and ask the server to wake up the client when
 * we receive it.
 */
static int radeon_dri2_schedule_wait_msc(ClientPtr client, DrawablePtr draw,
                                         CARD64 target_msc, CARD64 divisor,
                                         CARD64 remainder)
{
    ScreenPtr screen = draw->pScreen;
    ScrnInfoPtr scrn = xf86Screens[screen->myNum];
    RADEONInfoPtr info = RADEONPTR(scrn);
    DRI2FrameEventPtr wait_info;
    drmVBlank vbl;
    int ret, crtc = radeon_dri2_drawable_crtc(draw);
    CARD64 current_msc;

    /* Truncate to match kernel interfaces; means occasional overflow
     * misses, but that's generally not a big deal */
    target_msc &= 0xffffffff;
    divisor &= 0xffffffff;
    remainder &= 0xffffffff;

    /* Drawable not visible, return immediately */
    if (crtc == -1)
        goto out_complete;

    wait_info = calloc(1, sizeof(DRI2FrameEventRec));
    if (!wait_info)
        goto out_complete;

    wait_info->drawable_id = draw->id;
    wait_info->client = client;
    wait_info->type = DRI2_WAITMSC;

    /* Get current count */
    vbl.request.type = DRM_VBLANK_RELATIVE;
    if (crtc > 0)
        vbl.request.type |= DRM_VBLANK_SECONDARY;
    vbl.request.sequence = 0;
    ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
    if (ret) {
        xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                "get vblank counter failed: %s\n", strerror(errno));
        goto out_complete;
    }

    current_msc = vbl.reply.sequence;

    /*
     * If divisor is zero, or current_msc is smaller than target_msc,
     * we just need to make sure target_msc passes  before waking up the
     * client.
     */
    if (divisor == 0 || current_msc < target_msc) {
        /* If target_msc already reached or passed, set it to
         * current_msc to ensure we return a reasonable value back
         * to the caller. This keeps the client from continually
         * sending us MSC targets from the past by forcibly updating
         * their count on this call.
         */
        if (current_msc >= target_msc)
            target_msc = current_msc;
        vbl.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
        if (crtc > 0)
            vbl.request.type |= DRM_VBLANK_SECONDARY;
        vbl.request.sequence = target_msc;
        vbl.request.signal = (unsigned long)wait_info;
        ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
        if (ret) {
            xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                    "get vblank counter failed: %s\n", strerror(errno));
            goto out_complete;
        }

        wait_info->frame = vbl.reply.sequence;
        DRI2BlockClient(client, draw);
        return TRUE;
    }

    /*
     * If we get here, target_msc has already passed or we don't have one,
     * so we queue an event that will satisfy the divisor/remainder equation.
     */
    vbl.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
    if (crtc > 0)
        vbl.request.type |= DRM_VBLANK_SECONDARY;

    vbl.request.sequence = current_msc - (current_msc % divisor) +
        remainder;

    /*
     * If calculated remainder is larger than requested remainder,
     * it means we've passed the last point where
     * seq % divisor == remainder, so we need to wait for the next time
     * that will happen.
     */
    if ((current_msc % divisor) >= remainder)
        vbl.request.sequence += divisor;

    vbl.request.signal = (unsigned long)wait_info;
    ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
    if (ret) {
        xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                "get vblank counter failed: %s\n", strerror(errno));
        goto out_complete;
    }

    wait_info->frame = vbl.reply.sequence;
    DRI2BlockClient(client, draw);

    return TRUE;

out_complete:
    DRI2WaitMSCComplete(client, draw, target_msc, 0, 0);
    return TRUE;
}

/*
 * ScheduleSwap is responsible for requesting a DRM vblank event for the
 * appropriate frame.
 *
 * In the case of a blit (e.g. for a windowed swap) or buffer exchange,
 * the vblank requested can simply be the last queued swap frame + the swap
 * interval for the drawable.
 *
 * In the case of a page flip, we request an event for the last queued swap
 * frame + swap interval - 1, since we'll need to queue the flip for the frame
 * immediately following the received event.
 *
 * The client will be blocked if it tries to perform further GL commands
 * after queueing a swap, though in the Intel case after queueing a flip, the
 * client is free to queue more commands; they'll block in the kernel if
 * they access buffers busy with the flip.
 *
 * When the swap is complete, the driver should call into the server so it
 * can send any swap complete events that have been requested.
 */
static int radeon_dri2_schedule_swap(ClientPtr client, DrawablePtr draw,
                                     DRI2BufferPtr front, DRI2BufferPtr back,
                                     CARD64 *target_msc, CARD64 divisor,
                                     CARD64 remainder, DRI2SwapEventPtr func,
                                     void *data)
{
    ScreenPtr screen = draw->pScreen;
    ScrnInfoPtr scrn = xf86Screens[screen->myNum];
    RADEONInfoPtr info = RADEONPTR(scrn);
    drmVBlank vbl;
    int ret, crtc= radeon_dri2_drawable_crtc(draw), flip = 0;
    DRI2FrameEventPtr swap_info;
    enum DRI2FrameEventType swap_type = DRI2_SWAP;
    CARD64 current_msc;
    BoxRec box;
    RegionRec region;

    /* Truncate to match kernel interfaces; means occasional overflow
     * misses, but that's generally not a big deal */
    *target_msc &= 0xffffffff;
    divisor &= 0xffffffff;
    remainder &= 0xffffffff;

    swap_info = calloc(1, sizeof(DRI2FrameEventRec));

    /* Drawable not displayed... just complete the swap */
    if (crtc == -1 || !swap_info)
        goto blit_fallback;

    swap_info->drawable_id = draw->id;
    swap_info->client = client;
    swap_info->event_complete = func;
    swap_info->event_data = data;
    swap_info->front = front;
    swap_info->back = back;

    /* Get current count */
    vbl.request.type = DRM_VBLANK_RELATIVE;
    if (crtc > 0)
        vbl.request.type |= DRM_VBLANK_SECONDARY;
    vbl.request.sequence = 0;
    ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
    if (ret) {
        xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                "first get vblank counter failed: %s\n",
                strerror(errno));
        goto blit_fallback;
    }

    current_msc = vbl.reply.sequence;
    swap_info->type = swap_type;

    /* Correct target_msc by 'flip' if swap_type == DRI2_FLIP.
     * Do it early, so handling of different timing constraints
     * for divisor, remainder and msc vs. target_msc works.
     */
    if (*target_msc > 0)
        *target_msc -= flip;

    /*
     * If divisor is zero, or current_msc is smaller than target_msc
     * we just need to make sure target_msc passes before initiating
     * the swap.
     */
    if (divisor == 0 || current_msc < *target_msc) {
        vbl.request.type =  DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
        if (crtc > 0)
            vbl.request.type |= DRM_VBLANK_SECONDARY;

        /* If non-pageflipping, but blitting/exchanging, we need to use
         * DRM_VBLANK_NEXTONMISS to avoid unreliable timestamping later
         * on.
         */
        if (flip == 0)
            vbl.request.type |= DRM_VBLANK_NEXTONMISS;
        if (crtc > 0)
            vbl.request.type |= DRM_VBLANK_SECONDARY;

        /* If target_msc already reached or passed, set it to
         * current_msc to ensure we return a reasonable value back
         * to the caller. This makes swap_interval logic more robust.
         */
        if (current_msc >= *target_msc)
            *target_msc = current_msc;

        vbl.request.sequence = *target_msc;
        vbl.request.signal = (unsigned long)swap_info;
        ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
        if (ret) {
            xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                    "divisor 0 get vblank counter failed: %s\n",
                    strerror(errno));
            goto blit_fallback;
        }

        *target_msc = vbl.reply.sequence + flip;
        swap_info->frame = *target_msc;

        return TRUE;
    }

    /*
     * If we get here, target_msc has already passed or we don't have one,
     * and we need to queue an event that will satisfy the divisor/remainder
     * equation.
     */
    vbl.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
    if (flip == 0)
        vbl.request.type |= DRM_VBLANK_NEXTONMISS;
    if (crtc > 0)
        vbl.request.type |= DRM_VBLANK_SECONDARY;

    vbl.request.sequence = current_msc - (current_msc % divisor) +
        remainder;

    /*
     * If the calculated deadline vbl.request.sequence is smaller than
     * or equal to current_msc, it means we've passed the last point
     * when effective onset frame seq could satisfy
     * seq % divisor == remainder, so we need to wait for the next time
     * this will happen.

     * This comparison takes the 1 frame swap delay in pageflipping mode
     * into account, as well as a potential DRM_VBLANK_NEXTONMISS delay
     * if we are blitting/exchanging instead of flipping.
     */
    if (vbl.request.sequence <= current_msc)
        vbl.request.sequence += divisor;

    /* Account for 1 frame extra pageflip delay if flip > 0 */
    vbl.request.sequence -= flip;

    vbl.request.signal = (unsigned long)swap_info;
    ret = drmWaitVBlank(info->dri2.drm_fd, &vbl);
    if (ret) {
        xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                "final get vblank counter failed: %s\n",
                strerror(errno));
        goto blit_fallback;
    }

    /* Adjust returned value for 1 fame pageflip offset of flip > 0 */
    *target_msc = vbl.reply.sequence + flip;
    swap_info->frame = *target_msc;

    return TRUE;

blit_fallback:
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = draw->width;
    box.y2 = draw->height;
    REGION_INIT(pScreen, &region, &box, 0);

    radeon_dri2_copy_region(draw, &region, front, back);

    DRI2SwapComplete(client, draw, 0, 0, 0, DRI2_BLIT_COMPLETE, func, data);
    if (swap_info)
        free(swap_info);
    *target_msc = 0; /* offscreen, so zero out target vblank count */
    return TRUE;
}

#endif /* DRI2INFOREC_VERSION >= 4 */


Bool
radeon_dri2_screen_init(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    DRI2InfoRec dri2_info = { 0 };
#if DRI2INFOREC_VERSION >= 4
    const char *driverNames[1];
#endif

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

#if DRI2INFOREC_VERSION >= 4
    if (info->dri->pKernelDRMVersion->version_minor >= 4) {
        dri2_info.version = 4;
        dri2_info.ScheduleSwap = radeon_dri2_schedule_swap;
        dri2_info.GetMSC = radeon_dri2_get_msc;
        dri2_info.ScheduleWaitMSC = radeon_dri2_schedule_wait_msc;
        dri2_info.numDrivers = 1;
        dri2_info.driverNames = driverNames;
        driverNames[0] = dri2_info.driverName;
    } else {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "You need a newer kernel for sync extension\n");
    }
#endif

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
