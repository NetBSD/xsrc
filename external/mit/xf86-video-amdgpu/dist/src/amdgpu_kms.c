/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Dave Airlie <airlied@redhat.com>
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <sys/ioctl.h>
/* Driver data structures */
#include "amdgpu_drv.h"
#include "amdgpu_drm_queue.h"
#include "amdgpu_glamor.h"
#include "amdgpu_probe.h"
#include "micmap.h"

#include "amdgpu_version.h"
#include "shadow.h"
#include <xf86Priv.h>

#include "amdpciids.h"

/* DPMS */
#ifdef HAVE_XEXTPROTO_71
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#endif

#include <X11/extensions/damageproto.h>

#include "amdgpu_chipinfo_gen.h"
#include "amdgpu_bo_helper.h"
#include "amdgpu_pixmap.h"

#include <gbm.h>

static DevScreenPrivateKeyRec amdgpu_client_private_key;

extern SymTabRec AMDGPUChipsets[];
static Bool amdgpu_setup_kernel_mem(ScreenPtr pScreen);

const OptionInfoRec AMDGPUOptions_KMS[] = {
	{OPTION_ACCEL, "Accel", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_SW_CURSOR, "SWcursor", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_PAGE_FLIP, "EnablePageFlip", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_SUBPIXEL_ORDER, "SubPixelOrder", OPTV_ANYSTR, {0}, FALSE},
	{OPTION_ZAPHOD_HEADS, "ZaphodHeads", OPTV_STRING, {0}, FALSE},
	{OPTION_ACCEL_METHOD, "AccelMethod", OPTV_STRING, {0}, FALSE},
	{OPTION_DRI3, "DRI3", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_DRI, "DRI", OPTV_INTEGER, {0}, FALSE},
	{OPTION_SHADOW_PRIMARY, "ShadowPrimary", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_TEAR_FREE, "TearFree", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_DELETE_DP12, "DeleteUnusedDP12Displays", OPTV_BOOLEAN, {0}, FALSE},
	{-1, NULL, OPTV_NONE, {0}, FALSE}
};

const OptionInfoRec *AMDGPUOptionsWeak(void)
{
	return AMDGPUOptions_KMS;
}

extern _X_EXPORT int gAMDGPUEntityIndex;

static int getAMDGPUEntityIndex(void)
{
	return gAMDGPUEntityIndex;
}

AMDGPUEntPtr AMDGPUEntPriv(ScrnInfoPtr pScrn)
{
	DevUnion *pPriv;
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	pPriv = xf86GetEntityPrivate(info->pEnt->index, getAMDGPUEntityIndex());
	return pPriv->ptr;
}

/* Allocate our private AMDGPUInfoRec */
static Bool AMDGPUGetRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate)
		return TRUE;

	pScrn->driverPrivate = xnfcalloc(sizeof(AMDGPUInfoRec), 1);
	return TRUE;
}

/* Free our private AMDGPUInfoRec */
static void AMDGPUFreeRec(ScrnInfoPtr pScrn)
{
	DevUnion *pPriv;
	AMDGPUEntPtr pAMDGPUEnt;
	AMDGPUInfoPtr info;

	if (!pScrn)
		return;

	info = AMDGPUPTR(pScrn);
	if (info && info->fbcon_pixmap)
		pScrn->pScreen->DestroyPixmap(info->fbcon_pixmap);

	pPriv = xf86GetEntityPrivate(xf86GetEntityInfo(pScrn->entityList[pScrn->numEntities - 1])->index,
				     gAMDGPUEntityIndex);
	pAMDGPUEnt = pPriv->ptr;
	if (pAMDGPUEnt->fd > 0) {
		DevUnion *pPriv;
		AMDGPUEntPtr pAMDGPUEnt;
		pPriv = xf86GetEntityPrivate(pScrn->entityList[0],
					     getAMDGPUEntityIndex());

		pAMDGPUEnt = pPriv->ptr;
		pAMDGPUEnt->fd_ref--;
		if (!pAMDGPUEnt->fd_ref) {
			amdgpu_device_deinitialize(pAMDGPUEnt->pDev);
#ifdef XF86_PDEV_SERVER_FD
			if (!(pAMDGPUEnt->platform_dev &&
			      pAMDGPUEnt->platform_dev->flags & XF86_PDEV_SERVER_FD))
#endif
				drmClose(pAMDGPUEnt->fd);
			pAMDGPUEnt->fd = 0;
		}
	}

	free(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

static void *amdgpuShadowWindow(ScreenPtr screen, CARD32 row, CARD32 offset,
				int mode, CARD32 * size, void *closure)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(screen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	int stride;

	stride = (pScrn->displayWidth * pScrn->bitsPerPixel) / 8;
	*size = stride;

	return ((uint8_t *) info->front_buffer->cpu_ptr + row * stride + offset);
}

static void
amdgpuUpdatePacked(ScreenPtr pScreen, shadowBufPtr pBuf)
{
	shadowUpdatePacked(pScreen, pBuf);
}

static Bool
callback_needs_flush(AMDGPUInfoPtr info, struct amdgpu_client_priv *client_priv)
{
	return (int)(client_priv->needs_flush - info->gpu_flushed) > 0;
}

static void
amdgpu_event_callback(CallbackListPtr *list,
		      pointer user_data, pointer call_data)
{
	EventInfoRec *eventinfo = call_data;
	ScrnInfoPtr pScrn = user_data;
	ScreenPtr pScreen = pScrn->pScreen;
	struct amdgpu_client_priv *client_priv =
		dixLookupScreenPrivate(&eventinfo->client->devPrivates,
				       &amdgpu_client_private_key, pScreen);
	struct amdgpu_client_priv *server_priv =
		dixLookupScreenPrivate(&serverClient->devPrivates,
				       &amdgpu_client_private_key, pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	int i;

	if (callback_needs_flush(info, client_priv) ||
	    callback_needs_flush(info, server_priv))
		return;

	/* Don't let gpu_flushed get too far ahead of needs_flush, in order
	 * to prevent false positives in callback_needs_flush()
	 */
	client_priv->needs_flush = info->gpu_flushed;
	server_priv->needs_flush = info->gpu_flushed;
	
	for (i = 0; i < eventinfo->count; i++) {
		if (eventinfo->events[i].u.u.type == info->callback_event_type) {
			client_priv->needs_flush++;
			server_priv->needs_flush++;
			return;
		}
	}
}

static void
amdgpu_flush_callback(CallbackListPtr *list,
		      pointer user_data, pointer call_data)
{
	ScrnInfoPtr pScrn = user_data;
	ScreenPtr pScreen = pScrn->pScreen;
	ClientPtr client = call_data ? call_data : serverClient;
	struct amdgpu_client_priv *client_priv =
		dixLookupScreenPrivate(&client->devPrivates,
				       &amdgpu_client_private_key, pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	if (pScrn->vtSema && callback_needs_flush(info, client_priv))
		amdgpu_glamor_flush(pScrn);
}

static Bool AMDGPUCreateScreenResources_KMS(ScreenPtr pScreen)
{
	ExtensionEntry *damage_ext = CheckExtension("DAMAGE");
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	PixmapPtr pixmap;

	pScreen->CreateScreenResources = info->CreateScreenResources;
	if (!(*pScreen->CreateScreenResources) (pScreen))
		return FALSE;
	pScreen->CreateScreenResources = AMDGPUCreateScreenResources_KMS;

	/* Set the RandR primary output if Xorg hasn't */
	if (dixPrivateKeyRegistered(rrPrivKey)) {
		rrScrPrivPtr rrScrPriv = rrGetScrPriv(pScreen);

		if (
#ifdef AMDGPU_PIXMAP_SHARING
		    !pScreen->isGPU &&
#endif
		    !rrScrPriv->primaryOutput)
		{
			xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);

			rrScrPriv->primaryOutput = xf86_config->output[0]->randr_output;
			RROutputChanged(rrScrPriv->primaryOutput, FALSE);
			rrScrPriv->layoutChanged = TRUE;
		}
	}

	if (!drmmode_set_desired_modes(pScrn, &info->drmmode, pScrn->is_gpu))
		return FALSE;

	drmmode_uevent_init(pScrn, &info->drmmode);

	if (info->shadow_fb) {
		pixmap = pScreen->GetScreenPixmap(pScreen);

		if (!shadowAdd(pScreen, pixmap, amdgpuUpdatePacked,
			       amdgpuShadowWindow, 0, NULL))
			return FALSE;
	}

	if (info->dri2.enabled || info->use_glamor) {
		if (info->front_buffer) {
			PixmapPtr pPix = pScreen->GetScreenPixmap(pScreen);

			if (!amdgpu_set_pixmap_bo(pPix, info->front_buffer))
				return FALSE;
		}
	}

	if (info->use_glamor)
		amdgpu_glamor_create_screen_resources(pScreen);

	info->callback_event_type = -1;
	if (damage_ext) {
		info->callback_event_type = damage_ext->eventBase + XDamageNotify;

		if (!AddCallback(&FlushCallback, amdgpu_flush_callback, pScrn))
			return FALSE;

		if (!AddCallback(&EventCallback, amdgpu_event_callback, pScrn)) {
			DeleteCallback(&FlushCallback, amdgpu_flush_callback, pScrn);
			return FALSE;
		}

		if (!dixRegisterScreenPrivateKey(&amdgpu_client_private_key, pScreen,
						 PRIVATE_CLIENT, sizeof(struct amdgpu_client_priv))) {
			DeleteCallback(&FlushCallback, amdgpu_flush_callback, pScrn);
			DeleteCallback(&EventCallback, amdgpu_event_callback, pScrn);
			return FALSE;
		}
	}

	return TRUE;
}

static Bool
amdgpu_scanout_extents_intersect(xf86CrtcPtr xf86_crtc, BoxPtr extents)
{
	extents->x1 -= xf86_crtc->filter_width >> 1;
	extents->x2 += xf86_crtc->filter_width >> 1;
	extents->y1 -= xf86_crtc->filter_height >> 1;
	extents->y2 += xf86_crtc->filter_height >> 1;
	pixman_f_transform_bounds(&xf86_crtc->f_framebuffer_to_crtc, extents);

	extents->x1 = max(extents->x1, 0);
	extents->y1 = max(extents->y1, 0);
	extents->x2 = min(extents->x2, xf86_crtc->mode.HDisplay);
	extents->y2 = min(extents->y2, xf86_crtc->mode.VDisplay);

	return (extents->x1 < extents->x2 && extents->y1 < extents->y2);
}

static RegionPtr
transform_region(RegionPtr region, struct pict_f_transform *transform,
		 int w, int h)
{
	BoxPtr boxes = RegionRects(region);
	int nboxes = RegionNumRects(region);
	xRectanglePtr rects = malloc(nboxes * sizeof(*rects));
	RegionPtr transformed;
	int nrects = 0;
	BoxRec box;
	int i;

	for (i = 0; i < nboxes; i++) {
		box.x1 = boxes[i].x1;
		box.x2 = boxes[i].x2;
		box.y1 = boxes[i].y1;
		box.y2 = boxes[i].y2;
		pixman_f_transform_bounds(transform, &box);

		box.x1 = max(box.x1, 0);
		box.y1 = max(box.y1, 0);
		box.x2 = min(box.x2, w);
		box.y2 = min(box.y2, h);
		if (box.x1 >= box.x2 || box.y1 >= box.y2)
			continue;

		rects[nrects].x = box.x1;
		rects[nrects].y = box.y1;
		rects[nrects].width = box.x2 - box.x1;
		rects[nrects].height = box.y2 - box.y1;
		nrects++;
	}

	transformed = RegionFromRects(nrects, rects, CT_UNSORTED);
	free(rects);
	return transformed;
}

static void
amdgpu_sync_scanout_pixmaps(xf86CrtcPtr xf86_crtc, RegionPtr new_region,
							int scanout_id)
{
	drmmode_crtc_private_ptr drmmode_crtc = xf86_crtc->driver_private;
	DrawablePtr dst = &drmmode_crtc->scanout[scanout_id].pixmap->drawable;
	DrawablePtr src = &drmmode_crtc->scanout[scanout_id ^ 1].pixmap->drawable;
	RegionPtr last_region = &drmmode_crtc->scanout_last_region;
	ScrnInfoPtr scrn = xf86_crtc->scrn;
	ScreenPtr pScreen = scrn->pScreen;
	RegionRec remaining;
	RegionPtr sync_region = NULL;
	BoxRec extents;
	GCPtr gc;

	if (RegionNil(last_region))
		return;

	RegionNull(&remaining);
	RegionSubtract(&remaining, last_region, new_region);
	if (RegionNil(&remaining))
		goto uninit;

	extents = *RegionExtents(&remaining);
	if (!amdgpu_scanout_extents_intersect(xf86_crtc, &extents))
		goto uninit;

#if XF86_CRTC_VERSION >= 4
	if (xf86_crtc->driverIsPerformingTransform) {
		sync_region = transform_region(&remaining,
					       &xf86_crtc->f_framebuffer_to_crtc,
					       dst->width, dst->height);
	} else
#endif /* XF86_CRTC_VERSION >= 4 */
	{
		sync_region = RegionDuplicate(&remaining);
		RegionTranslate(sync_region, -xf86_crtc->x, -xf86_crtc->y);
	}

	gc = GetScratchGC(dst->depth, pScreen);
	if (gc) {
		ValidateGC(dst, gc);
		gc->funcs->ChangeClip(gc, CT_REGION, sync_region, 0);
		sync_region = NULL;
		gc->ops->CopyArea(src, dst, gc, 0, 0, dst->width, dst->height, 0, 0);
		FreeScratchGC(gc);
	}

 uninit:
	if (sync_region)
		RegionDestroy(sync_region);
	RegionUninit(&remaining);
}

#ifdef AMDGPU_PIXMAP_SHARING

static RegionPtr
dirty_region(PixmapDirtyUpdatePtr dirty)
{
	RegionPtr damageregion = DamageRegion(dirty->damage);
	RegionPtr dstregion;

#ifdef HAS_DIRTYTRACKING_ROTATION
	if (dirty->rotation != RR_Rotate_0) {
		dstregion = transform_region(damageregion,
					     &dirty->f_inverse,
					     dirty->slave_dst->drawable.width,
					     dirty->slave_dst->drawable.height);
	} else
#endif
	{
		RegionRec pixregion;

		dstregion = RegionDuplicate(damageregion);
		RegionTranslate(dstregion, -dirty->x, -dirty->y);
		PixmapRegionInit(&pixregion, dirty->slave_dst);
		RegionIntersect(dstregion, dstregion, &pixregion);
		RegionUninit(&pixregion);
	}

	return dstregion;
}

static void
redisplay_dirty(PixmapDirtyUpdatePtr dirty, RegionPtr region)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(dirty->src->drawable.pScreen);

	if (RegionNil(region))
		goto out;

	if (dirty->slave_dst->master_pixmap)
		DamageRegionAppend(&dirty->slave_dst->drawable, region);

#ifdef HAS_DIRTYTRACKING_ROTATION
	PixmapSyncDirtyHelper(dirty);
#else
	PixmapSyncDirtyHelper(dirty, region);
#endif

	amdgpu_glamor_flush(scrn);
	if (dirty->slave_dst->master_pixmap)
		DamageRegionProcessPending(&dirty->slave_dst->drawable);

out:
	DamageEmpty(dirty->damage);
}

static void
amdgpu_prime_scanout_update_abort(xf86CrtcPtr crtc, void *event_data)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;

	drmmode_crtc->scanout_update_pending = FALSE;
}

void
amdgpu_sync_shared_pixmap(PixmapDirtyUpdatePtr dirty)
{
	ScreenPtr master_screen = dirty->src->master_pixmap->drawable.pScreen;
	PixmapDirtyUpdatePtr ent;
	RegionPtr region;

	xorg_list_for_each_entry(ent, &master_screen->pixmap_dirty_list, ent) {
		if (ent->slave_dst != dirty->src)
			continue;

		region = dirty_region(ent);
		redisplay_dirty(ent, region);
		RegionDestroy(region);
	}
}


#if HAS_SYNC_SHARED_PIXMAP

static Bool
master_has_sync_shared_pixmap(ScrnInfoPtr scrn, PixmapDirtyUpdatePtr dirty)
{
	ScreenPtr master_screen = dirty->src->master_pixmap->drawable.pScreen;

	return master_screen->SyncSharedPixmap != NULL;
}

static Bool
slave_has_sync_shared_pixmap(ScrnInfoPtr scrn, PixmapDirtyUpdatePtr dirty)
{
	ScreenPtr slave_screen = dirty->slave_dst->drawable.pScreen;

	return slave_screen->SyncSharedPixmap != NULL;
}

static void
call_sync_shared_pixmap(PixmapDirtyUpdatePtr dirty)
{
	ScreenPtr master_screen = dirty->src->master_pixmap->drawable.pScreen;

	master_screen->SyncSharedPixmap(dirty);
}

#else /* !HAS_SYNC_SHARED_PIXMAP */

static Bool
master_has_sync_shared_pixmap(ScrnInfoPtr scrn, PixmapDirtyUpdatePtr dirty)
{
	ScrnInfoPtr master_scrn = xf86ScreenToScrn(dirty->src->master_pixmap->drawable.pScreen);

	return master_scrn->driverName == scrn->driverName;
}

static Bool
slave_has_sync_shared_pixmap(ScrnInfoPtr scrn, PixmapDirtyUpdatePtr dirty)
{
	ScrnInfoPtr slave_scrn = xf86ScreenToScrn(dirty->slave_dst->drawable.pScreen);

	return slave_scrn->driverName == scrn->driverName;
}

static void
call_sync_shared_pixmap(PixmapDirtyUpdatePtr dirty)
{
	amdgpu_sync_shared_pixmap(dirty);
}

#endif /* HAS_SYNC_SHARED_PIXMAPS */


static Bool
amdgpu_prime_scanout_do_update(xf86CrtcPtr crtc, unsigned scanout_id)
{
	ScrnInfoPtr scrn = crtc->scrn;
	ScreenPtr screen = scrn->pScreen;
	AMDGPUInfoPtr info = AMDGPUPTR(scrn);
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	PixmapPtr scanoutpix = crtc->randr_crtc->scanout_pixmap;
	PixmapDirtyUpdatePtr dirty;
	Bool ret = FALSE;

	xorg_list_for_each_entry(dirty, &screen->pixmap_dirty_list, ent) {
		if (dirty->src == scanoutpix && dirty->slave_dst ==
		    drmmode_crtc->scanout[scanout_id ^ info->tear_free].pixmap) {
			RegionPtr region;

			if (master_has_sync_shared_pixmap(scrn, dirty))
				call_sync_shared_pixmap(dirty);

			region = dirty_region(dirty);
			if (RegionNil(region))
				goto destroy;

			if (info->tear_free) {
				RegionTranslate(region, crtc->x, crtc->y);
				amdgpu_sync_scanout_pixmaps(crtc, region, scanout_id);
				amdgpu_glamor_flush(scrn);
				RegionCopy(&drmmode_crtc->scanout_last_region, region);
				RegionTranslate(region, -crtc->x, -crtc->y);
				dirty->slave_dst = drmmode_crtc->scanout[scanout_id].pixmap;
			}

			redisplay_dirty(dirty, region);
			ret = TRUE;
		destroy:
			RegionDestroy(region);
			break;
		}
	}

	return ret;
}

void
amdgpu_prime_scanout_update_handler(xf86CrtcPtr crtc, uint32_t frame, uint64_t usec,
				     void *event_data)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;

	amdgpu_prime_scanout_do_update(crtc, 0);
	drmmode_crtc->scanout_update_pending = FALSE;
}

static void
amdgpu_prime_scanout_update(PixmapDirtyUpdatePtr dirty)
{
	ScreenPtr screen = dirty->slave_dst->drawable.pScreen;
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(scrn);
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	xf86CrtcPtr xf86_crtc = NULL;
	drmmode_crtc_private_ptr drmmode_crtc = NULL;
	uintptr_t drm_queue_seq;
	drmVBlank vbl;
	int c;

	/* Find the CRTC which is scanning out from this slave pixmap */
	for (c = 0; c < xf86_config->num_crtc; c++) {
		xf86_crtc = xf86_config->crtc[c];
		drmmode_crtc = xf86_crtc->driver_private;
		if (drmmode_crtc->scanout[0].pixmap == dirty->slave_dst)
			break;
	}

	if (c == xf86_config->num_crtc ||
	    !xf86_crtc->enabled ||
	    drmmode_crtc->scanout_update_pending ||
	    !drmmode_crtc->scanout[0].pixmap ||
	    drmmode_crtc->pending_dpms_mode != DPMSModeOn)
		return;

	drm_queue_seq = amdgpu_drm_queue_alloc(xf86_crtc,
					       AMDGPU_DRM_QUEUE_CLIENT_DEFAULT,
					       AMDGPU_DRM_QUEUE_ID_DEFAULT, NULL,
					       amdgpu_prime_scanout_update_handler,
					       amdgpu_prime_scanout_update_abort);
	if (drm_queue_seq == AMDGPU_DRM_QUEUE_ERROR) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "amdgpu_drm_queue_alloc failed for PRIME update\n");
		return;
	}

	vbl.request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT;
	vbl.request.type |= amdgpu_populate_vbl_request_type(xf86_crtc);
	vbl.request.sequence = 1;
	vbl.request.signal = drm_queue_seq;
	if (drmWaitVBlank(pAMDGPUEnt->fd, &vbl)) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "drmWaitVBlank failed for PRIME update: %s\n",
			   strerror(errno));
		amdgpu_drm_abort_entry(drm_queue_seq);
		return;
	}

	drmmode_crtc->scanout_update_pending = TRUE;
}

static void
amdgpu_prime_scanout_flip_abort(xf86CrtcPtr crtc, void *event_data)
{
	drmmode_crtc_private_ptr drmmode_crtc = event_data;

	drmmode_crtc->scanout_update_pending = FALSE;
	drmmode_clear_pending_flip(crtc);
}

static void
amdgpu_prime_scanout_flip(PixmapDirtyUpdatePtr ent)
{
	ScreenPtr screen = ent->slave_dst->drawable.pScreen;
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(scrn);
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	xf86CrtcPtr crtc = NULL;
	drmmode_crtc_private_ptr drmmode_crtc = NULL;
	uintptr_t drm_queue_seq;
	unsigned scanout_id;
	int c;

	/* Find the CRTC which is scanning out from this slave pixmap */
	for (c = 0; c < xf86_config->num_crtc; c++) {
		crtc = xf86_config->crtc[c];
		drmmode_crtc = crtc->driver_private;
		scanout_id = drmmode_crtc->scanout_id;
		if (drmmode_crtc->scanout[scanout_id].pixmap == ent->slave_dst)
			break;
	}

	if (c == xf86_config->num_crtc ||
	    !crtc->enabled ||
	    drmmode_crtc->scanout_update_pending ||
	    !drmmode_crtc->scanout[drmmode_crtc->scanout_id].pixmap ||
	    drmmode_crtc->pending_dpms_mode != DPMSModeOn)
		return;

	scanout_id = drmmode_crtc->scanout_id ^ 1;
	if (!amdgpu_prime_scanout_do_update(crtc, scanout_id))
		return;

	drm_queue_seq = amdgpu_drm_queue_alloc(crtc,
					       AMDGPU_DRM_QUEUE_CLIENT_DEFAULT,
					       AMDGPU_DRM_QUEUE_ID_DEFAULT,
					       drmmode_crtc, NULL,
					       amdgpu_prime_scanout_flip_abort);
	if (drm_queue_seq == AMDGPU_DRM_QUEUE_ERROR) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "Allocating DRM event queue entry failed for PRIME flip.\n");
		return;
	}

	if (drmModePageFlip(pAMDGPUEnt->fd, drmmode_crtc->mode_crtc->crtc_id,
			    drmmode_crtc->scanout[scanout_id].fb_id,
			    DRM_MODE_PAGE_FLIP_EVENT, (void*)drm_queue_seq)) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING, "flip queue failed in %s: %s\n",
			   __func__, strerror(errno));
		return;
	}

	drmmode_crtc->scanout_id = scanout_id;
	drmmode_crtc->scanout_update_pending = TRUE;
	drmmode_crtc->flip_pending = TRUE;
}

static void
amdgpu_dirty_update(ScrnInfoPtr scrn)
{
	AMDGPUInfoPtr info = AMDGPUPTR(scrn);
	ScreenPtr screen = scrn->pScreen;
	PixmapDirtyUpdatePtr ent;
	RegionPtr region;

	xorg_list_for_each_entry(ent, &screen->pixmap_dirty_list, ent) {
		if (screen->isGPU) {
			PixmapDirtyUpdatePtr region_ent = ent;

			if (master_has_sync_shared_pixmap(scrn, ent)) {
				ScreenPtr master_screen = ent->src->master_pixmap->drawable.pScreen;

				xorg_list_for_each_entry(region_ent, &master_screen->pixmap_dirty_list, ent) {
					if (region_ent->slave_dst == ent->src)
						break;
				}
			}

			region = dirty_region(region_ent);

			if (RegionNotEmpty(region)) {
				if (info->tear_free)
					amdgpu_prime_scanout_flip(ent);
				else
					amdgpu_prime_scanout_update(ent);
			} else {
				DamageEmpty(region_ent->damage);
			}

			RegionDestroy(region);
		} else {
			if (slave_has_sync_shared_pixmap(scrn, ent))
				continue;

			region = dirty_region(ent);
			redisplay_dirty(ent, region);
			RegionDestroy(region);
		}
	}
}
#endif

static Bool
amdgpu_scanout_do_update(xf86CrtcPtr xf86_crtc, int scanout_id)
{
	drmmode_crtc_private_ptr drmmode_crtc = xf86_crtc->driver_private;
	RegionPtr pRegion = DamageRegion(drmmode_crtc->scanout_damage);
	ScrnInfoPtr scrn = xf86_crtc->scrn;
	ScreenPtr pScreen = scrn->pScreen;
	AMDGPUInfoPtr info = AMDGPUPTR(scrn);
	DrawablePtr pDraw;
	BoxRec extents;

	if (!xf86_crtc->enabled ||
	    drmmode_crtc->pending_dpms_mode != DPMSModeOn ||
	    !drmmode_crtc->scanout[scanout_id].pixmap)
		return FALSE;

	if (!RegionNotEmpty(pRegion))
		return FALSE;

	pDraw = &drmmode_crtc->scanout[scanout_id].pixmap->drawable;
	extents = *RegionExtents(pRegion);
	if (!amdgpu_scanout_extents_intersect(xf86_crtc, &extents))
		return FALSE;

	if (info->tear_free) {
		amdgpu_sync_scanout_pixmaps(xf86_crtc, pRegion, scanout_id);
		RegionCopy(&drmmode_crtc->scanout_last_region, pRegion);
	}
	RegionEmpty(pRegion);

#if XF86_CRTC_VERSION >= 4
	if (xf86_crtc->driverIsPerformingTransform) {
		SourceValidateProcPtr SourceValidate = pScreen->SourceValidate;
		PictFormatPtr format = PictureWindowFormat(pScreen->root);
		int error;
		PicturePtr src, dst;
		XID include_inferiors = IncludeInferiors;

		src = CreatePicture(None,
				    &pScreen->root->drawable,
				    format,
				    CPSubwindowMode,
				    &include_inferiors, serverClient, &error);
		if (!src) {
			ErrorF("Failed to create source picture for transformed scanout "
			       "update\n");
			goto out;
		}

		dst = CreatePicture(None, pDraw, format, 0L, NULL, serverClient, &error);
		if (!dst) {
			ErrorF("Failed to create destination picture for transformed scanout "
			       "update\n");
			goto free_src;
		}
		error = SetPictureTransform(src, &xf86_crtc->crtc_to_framebuffer);
		if (error) {
			ErrorF("SetPictureTransform failed for transformed scanout "
			       "update\n");
			goto free_dst;
		}

		if (xf86_crtc->filter)
			SetPicturePictFilter(src, xf86_crtc->filter, xf86_crtc->params,
					     xf86_crtc->nparams);

		pScreen->SourceValidate = NULL;
		CompositePicture(PictOpSrc,
				 src, NULL, dst,
				 extents.x1, extents.y1, 0, 0, extents.x1,
				 extents.y1, extents.x2 - extents.x1,
				 extents.y2 - extents.y1);
		pScreen->SourceValidate = SourceValidate;

 free_dst:
		FreePicture(dst, None);
 free_src:
		FreePicture(src, None);
	} else
 out:
#endif /* XF86_CRTC_VERSION >= 4 */
	{
		GCPtr gc = GetScratchGC(pDraw->depth, pScreen);

		ValidateGC(pDraw, gc);
		(*gc->ops->CopyArea)(&pScreen->GetScreenPixmap(pScreen)->drawable,
				     pDraw, gc,
				     xf86_crtc->x + extents.x1, xf86_crtc->y + extents.y1,
				     extents.x2 - extents.x1, extents.y2 - extents.y1,
				     extents.x1, extents.y1);
		FreeScratchGC(gc);
	}

	amdgpu_glamor_flush(xf86_crtc->scrn);

	return TRUE;
}

static void
amdgpu_scanout_update_abort(xf86CrtcPtr crtc, void *event_data)
{
	drmmode_crtc_private_ptr drmmode_crtc = event_data;

	drmmode_crtc->scanout_update_pending = FALSE;
}

void
amdgpu_scanout_update_handler(xf86CrtcPtr crtc, uint32_t frame, uint64_t usec,
							  void *event_data)
{
	amdgpu_scanout_do_update(crtc, 0);

	amdgpu_scanout_update_abort(crtc, event_data);
}

static void
amdgpu_scanout_update(xf86CrtcPtr xf86_crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = xf86_crtc->driver_private;
	uintptr_t drm_queue_seq;
	ScrnInfoPtr scrn;
	AMDGPUEntPtr pAMDGPUEnt;
	drmVBlank vbl;
	DamagePtr pDamage;
	RegionPtr pRegion;
	BoxRec extents;

	if (!xf86_crtc->enabled ||
	    drmmode_crtc->scanout_update_pending ||
	    !drmmode_crtc->scanout[0].pixmap ||
	    drmmode_crtc->pending_dpms_mode != DPMSModeOn)
		return;

	pDamage = drmmode_crtc->scanout_damage;
	if (!pDamage)
		return;

	pRegion = DamageRegion(pDamage);
	if (!RegionNotEmpty(pRegion))
		return;

	extents = *RegionExtents(pRegion);
	if (!amdgpu_scanout_extents_intersect(xf86_crtc, &extents)) {
		RegionEmpty(pRegion);
		return;
	}

	scrn = xf86_crtc->scrn;
	drm_queue_seq = amdgpu_drm_queue_alloc(xf86_crtc,
					       AMDGPU_DRM_QUEUE_CLIENT_DEFAULT,
					       AMDGPU_DRM_QUEUE_ID_DEFAULT,
					       drmmode_crtc,
					       amdgpu_scanout_update_handler,
					       amdgpu_scanout_update_abort);
	if (drm_queue_seq == AMDGPU_DRM_QUEUE_ERROR) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "amdgpu_drm_queue_alloc failed for scanout update\n");
		return;
	}

	pAMDGPUEnt = AMDGPUEntPriv(scrn);
	vbl.request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT;
	vbl.request.type |= amdgpu_populate_vbl_request_type(xf86_crtc);
	vbl.request.sequence = 1;
	vbl.request.signal = drm_queue_seq;
	if (drmWaitVBlank(pAMDGPUEnt->fd, &vbl)) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "drmWaitVBlank failed for scanout update: %s\n",
			   strerror(errno));
		amdgpu_drm_abort_entry(drm_queue_seq);
		return;
	}

	drmmode_crtc->scanout_update_pending = TRUE;
}

static void
amdgpu_scanout_flip_abort(xf86CrtcPtr crtc, void *event_data)
{
	drmmode_crtc_private_ptr drmmode_crtc = event_data;

	drmmode_crtc->scanout_update_pending = FALSE;
	drmmode_clear_pending_flip(crtc);
}

static void
amdgpu_scanout_flip(ScreenPtr pScreen, AMDGPUInfoPtr info,
					xf86CrtcPtr xf86_crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = xf86_crtc->driver_private;
	ScrnInfoPtr scrn;
	AMDGPUEntPtr pAMDGPUEnt;
	uintptr_t drm_queue_seq;
	unsigned scanout_id;

	if (drmmode_crtc->scanout_update_pending)
		return;

	scanout_id = drmmode_crtc->scanout_id ^ 1;
	if (!amdgpu_scanout_do_update(xf86_crtc, scanout_id))
		return;

	scrn = xf86_crtc->scrn;
	drm_queue_seq = amdgpu_drm_queue_alloc(xf86_crtc,
					       AMDGPU_DRM_QUEUE_CLIENT_DEFAULT,
					       AMDGPU_DRM_QUEUE_ID_DEFAULT,
					       drmmode_crtc, NULL,
					       amdgpu_scanout_flip_abort);
	if (drm_queue_seq == AMDGPU_DRM_QUEUE_ERROR) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "Allocating DRM event queue entry failed.\n");
		return;
	}

	pAMDGPUEnt = AMDGPUEntPriv(scrn);
	if (drmModePageFlip(pAMDGPUEnt->fd, drmmode_crtc->mode_crtc->crtc_id,
			    drmmode_crtc->scanout[scanout_id].fb_id,
			    DRM_MODE_PAGE_FLIP_EVENT, (void*)drm_queue_seq)) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING, "flip queue failed in %s: %s\n",
			   __func__, strerror(errno));
		return;
	}

	drmmode_crtc->scanout_id = scanout_id;
	drmmode_crtc->scanout_update_pending = TRUE;
	drmmode_crtc->flip_pending = TRUE;
}

static void AMDGPUBlockHandler_KMS(BLOCKHANDLER_ARGS_DECL)
{
	SCREEN_PTR(arg);
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	int c;

	pScreen->BlockHandler = info->BlockHandler;
	(*pScreen->BlockHandler) (BLOCKHANDLER_ARGS);
	pScreen->BlockHandler = AMDGPUBlockHandler_KMS;

#ifdef AMDGPU_PIXMAP_SHARING
	if (!pScreen->isGPU)
#endif
	{
		for (c = 0; c < xf86_config->num_crtc; c++) {
			if (info->tear_free)
				amdgpu_scanout_flip(pScreen, info, xf86_config->crtc[c]);
			else if (info->shadow_primary
#if XF86_CRTC_VERSION >= 4
					 || xf86_config->crtc[c]->driverIsPerformingTransform
#endif
				)
				amdgpu_scanout_update(xf86_config->crtc[c]);
		}
	}

	if (info->use_glamor)
		amdgpu_glamor_flush(pScrn);

#ifdef AMDGPU_PIXMAP_SHARING
	amdgpu_dirty_update(pScrn);
#endif
}

static void AMDGPUBlockHandler_oneshot(BLOCKHANDLER_ARGS_DECL)
{
	SCREEN_PTR(arg);
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	AMDGPUBlockHandler_KMS(BLOCKHANDLER_ARGS);

	drmmode_set_desired_modes(pScrn, &info->drmmode, TRUE);
}

/* This is called by AMDGPUPreInit to set up the default visual */
static Bool AMDGPUPreInitVisual(ScrnInfoPtr pScrn)
{
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	if (!xf86SetDepthBpp(pScrn, 0, 0, 0, Support32bppFb))
		return FALSE;

	switch (pScrn->depth) {
	case 8:
	case 15:
	case 16:
	case 24:
		break;

	default:
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Given depth (%d) is not supported by %s driver\n",
			   pScrn->depth, AMDGPU_DRIVER_NAME);
		return FALSE;
	}

	xf86PrintDepthBpp(pScrn);

	info->pix24bpp = xf86GetBppFromDepth(pScrn, pScrn->depth);
	info->pixel_bytes = pScrn->bitsPerPixel / 8;

	if (info->pix24bpp == 24) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Amdgpu does NOT support 24bpp\n");
		return FALSE;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Pixel depth = %d bits stored in %d byte%s (%d bpp pixmaps)\n",
		   pScrn->depth,
		   info->pixel_bytes,
		   info->pixel_bytes > 1 ? "s" : "", info->pix24bpp);

	if (!xf86SetDefaultVisual(pScrn, -1))
		return FALSE;

	if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Default visual (%s) is not supported at depth %d\n",
			   xf86GetVisualName(pScrn->defaultVisual),
			   pScrn->depth);
		return FALSE;
	}
	return TRUE;
}

/* This is called by AMDGPUPreInit to handle all color weight issues */
static Bool AMDGPUPreInitWeight(ScrnInfoPtr pScrn)
{
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	/* Save flag for 6 bit DAC to use for
	   setting CRTC registers.  Otherwise use
	   an 8 bit DAC, even if xf86SetWeight sets
	   pScrn->rgbBits to some value other than
	   8. */
	info->dac6bits = FALSE;

	if (pScrn->depth > 8) {
		rgb defaultWeight = { 0, 0, 0 };

		if (!xf86SetWeight(pScrn, defaultWeight, defaultWeight))
			return FALSE;
	} else {
		pScrn->rgbBits = 8;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Using %d bits per RGB (%d bit DAC)\n",
		   pScrn->rgbBits, info->dac6bits ? 6 : 8);

	return TRUE;
}

static Bool AMDGPUPreInitAccel_KMS(ScrnInfoPtr pScrn)
{
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	if (xf86ReturnOptValBool(info->Options, OPTION_ACCEL, TRUE)) {
		AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);
		Bool use_glamor = TRUE;
#ifdef HAVE_GBM_BO_USE_LINEAR
		const char *accel_method;

		accel_method = xf86GetOptValString(info->Options, OPTION_ACCEL_METHOD);
		if ((accel_method && !strcmp(accel_method, "none")))
			use_glamor = FALSE;
#endif

#ifdef DRI2
		info->dri2.available = ! !xf86LoadSubModule(pScrn, "dri2");
#endif

		if (info->dri2.available)
			info->gbm = gbm_create_device(pAMDGPUEnt->fd);
		if (info->gbm == NULL)
			info->dri2.available = FALSE;

		if (use_glamor &&
			amdgpu_glamor_pre_init(pScrn))
			return TRUE;

		if (info->dri2.available)
			return TRUE;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "GPU accel disabled or not working, using shadowfb for KMS\n");
	info->shadow_fb = TRUE;
	if (!xf86LoadSubModule(pScrn, "shadow"))
		info->shadow_fb = FALSE;

	return TRUE;
}

static Bool AMDGPUPreInitChipType_KMS(ScrnInfoPtr pScrn)
{
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	int i;

	info->Chipset = PCI_DEV_DEVICE_ID(info->PciInfo);
	pScrn->chipset =
	    (char *)xf86TokenToString(AMDGPUChipsets, info->Chipset);
	if (!pScrn->chipset) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "ChipID 0x%04x is not recognized\n", info->Chipset);
		return FALSE;
	}

	if (info->Chipset < 0) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Chipset \"%s\" is not recognized\n",
			   pScrn->chipset);
		return FALSE;
	}
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
		   "Chipset: \"%s\" (ChipID = 0x%04x)\n",
		   pScrn->chipset, info->Chipset);

	for (i = 0; i < sizeof(AMDGPUCards) / sizeof(AMDGPUCardInfo); i++) {
		if (info->Chipset == AMDGPUCards[i].pci_device_id) {
			AMDGPUCardInfo *card = &AMDGPUCards[i];
			info->ChipFamily = card->chip_family;
			break;
		}
	}

	return TRUE;
}

static Bool amdgpu_get_tile_config(ScrnInfoPtr pScrn)
{
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);
	struct amdgpu_gpu_info gpu_info;

	memset(&gpu_info, 0, sizeof(gpu_info));
	amdgpu_query_gpu_info(pAMDGPUEnt->pDev, &gpu_info);

	switch ((gpu_info.gb_addr_cfg & 0x70) >> 4) {
	case 0:
		info->group_bytes = 256;
		break;
	case 1:
		info->group_bytes = 512;
		break;
	default:
		return FALSE;
	}

	info->have_tiling_info = TRUE;
	return TRUE;
}

static void AMDGPUSetupCapabilities(ScrnInfoPtr pScrn)
{
#ifdef AMDGPU_PIXMAP_SHARING
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);
	uint64_t value;
	int ret;

	pScrn->capabilities = 0;

	/* PRIME offloading requires acceleration */
	if (!info->use_glamor)
		return;

	ret = drmGetCap(pAMDGPUEnt->fd, DRM_CAP_PRIME, &value);
	if (ret == 0) {
		if (value & DRM_PRIME_CAP_EXPORT)
			pScrn->capabilities |= RR_Capability_SourceOutput | RR_Capability_SourceOffload;
		if (value & DRM_PRIME_CAP_IMPORT) {
			pScrn->capabilities |= RR_Capability_SinkOffload;
			if (info->drmmode.count_crtcs)
				pScrn->capabilities |= RR_Capability_SinkOutput;
		}
	}
#endif
}

/* When the root window is created, initialize the screen contents from
 * console if -background none was specified on the command line
 */
static Bool AMDGPUCreateWindow_oneshot(WindowPtr pWin)
{
	ScreenPtr pScreen = pWin->drawable.pScreen;
	ScrnInfoPtr pScrn;
	AMDGPUInfoPtr info;
	Bool ret;

	if (pWin != pScreen->root)
		ErrorF("%s called for non-root window %p\n", __func__, pWin);

	pScrn = xf86ScreenToScrn(pScreen);
	info = AMDGPUPTR(pScrn);
	pScreen->CreateWindow = info->CreateWindow;
	ret = pScreen->CreateWindow(pWin);

	if (ret)
		drmmode_copy_fb(pScrn, &info->drmmode);

	return ret;
}

Bool AMDGPUPreInit_KMS(ScrnInfoPtr pScrn, int flags)
{
	AMDGPUInfoPtr info;
	AMDGPUEntPtr pAMDGPUEnt;
	DevUnion *pPriv;
	Gamma zeros = { 0.0, 0.0, 0.0 };
	int cpp;
	uint64_t heap_size = 0;
	uint64_t max_allocation = 0;
	Bool sw_cursor;

	if (flags & PROBE_DETECT)
		return TRUE;

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPUPreInit_KMS\n");
	if (pScrn->numEntities != 1)
		return FALSE;
	if (!AMDGPUGetRec(pScrn))
		return FALSE;

	info = AMDGPUPTR(pScrn);
	info->IsSecondary = FALSE;
	info->pEnt =
	    xf86GetEntityInfo(pScrn->entityList[pScrn->numEntities - 1]);
	if (info->pEnt->location.type != BUS_PCI
#ifdef XSERVER_PLATFORM_BUS
	    && info->pEnt->location.type != BUS_PLATFORM
#endif
	    )
		goto fail;

	pPriv = xf86GetEntityPrivate(pScrn->entityList[0],
				     getAMDGPUEntityIndex());
	pAMDGPUEnt = pPriv->ptr;

	if (xf86IsEntityShared(pScrn->entityList[0])) {
		if (xf86IsPrimInitDone(pScrn->entityList[0])) {
			info->IsSecondary = TRUE;
		} else {
			xf86SetPrimInitDone(pScrn->entityList[0]);
		}
	}

	if (info->IsSecondary)
		pAMDGPUEnt->secondary_scrn = pScrn;
	else
		pAMDGPUEnt->primary_scrn = pScrn;

	info->PciInfo = xf86GetPciInfoForEntity(info->pEnt->index);
	pScrn->monitor = pScrn->confScreen->monitor;

	if (!AMDGPUPreInitVisual(pScrn))
		goto fail;

	xf86CollectOptions(pScrn, NULL);
	if (!(info->Options = malloc(sizeof(AMDGPUOptions_KMS))))
		goto fail;

	memcpy(info->Options, AMDGPUOptions_KMS, sizeof(AMDGPUOptions_KMS));
	xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, info->Options);

	if (!AMDGPUPreInitWeight(pScrn))
		goto fail;

	if (!AMDGPUPreInitChipType_KMS(pScrn))
		goto fail;

	info->dri2.available = FALSE;
	info->dri2.enabled = FALSE;
	info->dri2.pKernelDRMVersion = drmGetVersion(pAMDGPUEnt->fd);
	if (info->dri2.pKernelDRMVersion == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "AMDGPUDRIGetVersion failed to get the DRM version\n");
		goto fail;
	}

	/* Get ScreenInit function */
	if (!xf86LoadSubModule(pScrn, "fb"))
		return FALSE;

	if (!AMDGPUPreInitAccel_KMS(pScrn))
		goto fail;

	amdgpu_drm_queue_init();

	/* don't enable tiling if accel is not enabled */
	if (info->use_glamor) {
		/* set default group bytes, overridden by kernel info below */
		info->group_bytes = 256;
		info->have_tiling_info = FALSE;
		amdgpu_get_tile_config(pScrn);
	}

	if (info->use_glamor) {
		info->tear_free = xf86ReturnOptValBool(info->Options,
						       OPTION_TEAR_FREE, FALSE);

		if (info->tear_free)
			xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
				   "TearFree enabled\n");

		info->shadow_primary =
			xf86ReturnOptValBool(info->Options, OPTION_SHADOW_PRIMARY, FALSE);

		if (info->shadow_primary)
			xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "ShadowPrimary enabled\n");
	}

	sw_cursor = xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE);

	info->allowPageFlip = xf86ReturnOptValBool(info->Options,
						   OPTION_PAGE_FLIP,
						   TRUE);
	if (sw_cursor || info->tear_free || info->shadow_primary) {
		    xf86DrvMsg(pScrn->scrnIndex,
			       info->allowPageFlip ? X_WARNING : X_DEFAULT,
			       "KMS Pageflipping: disabled%s\n",
			       info->allowPageFlip ?
			       (sw_cursor ? " because of SWcursor" :
				" because of ShadowPrimary/TearFree") : "");
		    info->allowPageFlip = FALSE;
	} else {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "KMS Pageflipping: %sabled\n",
			   info->allowPageFlip ? "en" : "dis");
	}

	if (xf86ReturnOptValBool(info->Options, OPTION_DELETE_DP12, FALSE)) {
		info->drmmode.delete_dp_12_displays = TRUE;
	}

	if (drmmode_pre_init(pScrn, &info->drmmode, pScrn->bitsPerPixel / 8) ==
	    FALSE) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Kernel modesetting setup failed\n");
		goto fail;
	}

	AMDGPUSetupCapabilities(pScrn);

	if (info->drmmode.count_crtcs == 1)
		pAMDGPUEnt->HasCRTC2 = FALSE;
	else
		pAMDGPUEnt->HasCRTC2 = TRUE;

	if (info->ChipFamily >= CHIP_FAMILY_TAHITI &&
	    info->ChipFamily <= CHIP_FAMILY_HAINAN) {
		info->cursor_w = CURSOR_WIDTH;
		info->cursor_h = CURSOR_HEIGHT;
	} else {
		info->cursor_w = CURSOR_WIDTH_CIK;
		info->cursor_h = CURSOR_HEIGHT_CIK;
	}

	amdgpu_query_heap_size(pAMDGPUEnt->pDev, AMDGPU_GEM_DOMAIN_GTT,
				&heap_size, &max_allocation);
	info->gart_size = heap_size;
	amdgpu_query_heap_size(pAMDGPUEnt->pDev, AMDGPU_GEM_DOMAIN_VRAM,
				&heap_size, &max_allocation);
	info->vram_size = max_allocation;

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "mem size init: gart size :%llx vram size: s:%llx visible:%llx\n",
		   (unsigned long long)info->gart_size,
		   (unsigned long long)heap_size,
		   (unsigned long long)max_allocation);

	cpp = pScrn->bitsPerPixel / 8;
	pScrn->displayWidth =
	    AMDGPU_ALIGN(pScrn->virtualX, drmmode_get_pitch_align(pScrn, cpp));

	/* Set display resolution */
	xf86SetDpi(pScrn, 0, 0);

	if (!xf86SetGamma(pScrn, zeros))
		return FALSE;

	if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
		if (!xf86LoadSubModule(pScrn, "ramdac"))
			return FALSE;
	}

	if (pScrn->modes == NULL
#ifdef XSERVER_PLATFORM_BUS
	    && !pScrn->is_gpu
#endif
	    ) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No modes.\n");
		goto fail;
	}

	return TRUE;
fail:
	AMDGPUFreeRec(pScrn);
	return FALSE;

}

static Bool AMDGPUCursorInit_KMS(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	return xf86_cursors_init(pScreen, info->cursor_w, info->cursor_h,
				 (HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
				  HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
				  HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1 |
				  HARDWARE_CURSOR_UPDATE_UNHIDDEN |
				  HARDWARE_CURSOR_ARGB));
}

void AMDGPUBlank(ScrnInfoPtr pScrn)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	xf86OutputPtr output;
	xf86CrtcPtr crtc;
	int o, c;

	for (c = 0; c < xf86_config->num_crtc; c++) {
		crtc = xf86_config->crtc[c];
		for (o = 0; o < xf86_config->num_output; o++) {
			output = xf86_config->output[o];
			if (output->crtc != crtc)
				continue;

			output->funcs->dpms(output, DPMSModeOff);
		}
		crtc->funcs->dpms(crtc, DPMSModeOff);
	}
}

void AMDGPUUnblank(ScrnInfoPtr pScrn)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	xf86OutputPtr output;
	xf86CrtcPtr crtc;
	int o, c;
	for (c = 0; c < xf86_config->num_crtc; c++) {
		crtc = xf86_config->crtc[c];
		if (!crtc->enabled)
			continue;
		crtc->funcs->dpms(crtc, DPMSModeOn);
		for (o = 0; o < xf86_config->num_output; o++) {
			output = xf86_config->output[o];
			if (output->crtc != crtc)
				continue;
			output->funcs->dpms(output, DPMSModeOn);
		}
	}
}

static Bool amdgpu_set_drm_master(ScrnInfoPtr pScrn)
{
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);
	int err;

#ifdef XF86_PDEV_SERVER_FD
	if (pAMDGPUEnt->platform_dev &&
	    (pAMDGPUEnt->platform_dev->flags & XF86_PDEV_SERVER_FD))
		return TRUE;
#endif

	err = drmSetMaster(pAMDGPUEnt->fd);
	if (err)
		ErrorF("Unable to retrieve master\n");

	return err == 0;
}

static void amdgpu_drop_drm_master(ScrnInfoPtr pScrn)
{
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);

#ifdef XF86_PDEV_SERVER_FD
	if (pAMDGPUEnt->platform_dev &&
	    (pAMDGPUEnt->platform_dev->flags & XF86_PDEV_SERVER_FD))
		return;
#endif

	drmDropMaster(pAMDGPUEnt->fd);
}



static Bool AMDGPUSaveScreen_KMS(ScreenPtr pScreen, int mode)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	Bool unblank;

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPUSaveScreen(%d)\n", mode);

	unblank = xf86IsUnblank(mode);
	if (unblank)
		SetTimeSinceLastInputEvent();

	if ((pScrn != NULL) && pScrn->vtSema) {
		if (unblank)
			AMDGPUUnblank(pScrn);
		else
			AMDGPUBlank(pScrn);
	}
	return TRUE;
}

/* Called at the end of each server generation.  Restore the original
 * text mode, unmap video memory, and unwrap and call the saved
 * CloseScreen function.
 */
static Bool AMDGPUCloseScreen_KMS(CLOSE_SCREEN_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPUCloseScreen\n");

	/* Clear mask of assigned crtc's in this generation */
	pAMDGPUEnt->assigned_crtcs = 0;

	drmmode_uevent_fini(pScrn, &info->drmmode);
	amdgpu_drm_queue_close(pScrn);

	if (info->callback_event_type != -1) {
		DeleteCallback(&EventCallback, amdgpu_event_callback, pScrn);
		DeleteCallback(&FlushCallback, amdgpu_flush_callback, pScrn);
	}

	amdgpu_sync_close(pScreen);
	amdgpu_drop_drm_master(pScrn);

	drmmode_fini(pScrn, &info->drmmode);
	if (info->dri2.enabled) {
		amdgpu_dri2_close_screen(pScreen);
	}
	amdgpu_glamor_fini(pScreen);
	pScrn->vtSema = FALSE;
	xf86ClearPrimInitDone(info->pEnt->index);
	pScreen->BlockHandler = info->BlockHandler;
	pScreen->CloseScreen = info->CloseScreen;
	return (*pScreen->CloseScreen) (CLOSE_SCREEN_ARGS);
}

void AMDGPUFreeScreen_KMS(FREE_SCREEN_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPUFreeScreen\n");

	AMDGPUFreeRec(pScrn);
}

Bool AMDGPUScreenInit_KMS(SCREEN_INIT_ARGS_DECL)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	int subPixelOrder = SubPixelUnknown;
	MessageType from;
	Bool value;
	int driLevel;
	const char *s;
	void *front_ptr;

	pScrn->fbOffset = 0;

	miClearVisualTypes();
	if (!miSetVisualTypes(pScrn->depth,
			      miGetDefaultVisualMask(pScrn->depth),
			      pScrn->rgbBits, pScrn->defaultVisual))
		return FALSE;
	miSetPixmapDepths();

	if (!amdgpu_set_drm_master(pScrn))
		return FALSE;

	info->directRenderingEnabled = FALSE;
	if (info->shadow_fb == FALSE)
		info->directRenderingEnabled = amdgpu_dri2_screen_init(pScreen);

	if (!amdgpu_setup_kernel_mem(pScreen)) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "amdgpu_setup_kernel_mem failed\n");
		return FALSE;
	}
	front_ptr = info->front_buffer->cpu_ptr;

	if (info->shadow_fb) {
		info->fb_shadow = calloc(1,
					 pScrn->displayWidth * pScrn->virtualY *
					 ((pScrn->bitsPerPixel + 7) >> 3));
		if (info->fb_shadow == NULL) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "Failed to allocate shadow framebuffer\n");
			info->shadow_fb = FALSE;
		} else {
			if (!fbScreenInit(pScreen, info->fb_shadow,
					  pScrn->virtualX, pScrn->virtualY,
					  pScrn->xDpi, pScrn->yDpi,
					  pScrn->displayWidth,
					  pScrn->bitsPerPixel))
				return FALSE;
		}
	}

	if (info->shadow_fb == FALSE) {
		/* Init fb layer */
		if (!fbScreenInit(pScreen, front_ptr,
				  pScrn->virtualX, pScrn->virtualY,
				  pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
				  pScrn->bitsPerPixel))
			return FALSE;
	}

	xf86SetBlackWhitePixels(pScreen);

	if (pScrn->bitsPerPixel > 8) {
		VisualPtr visual;

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
	}

	/* Must be after RGB order fixed */
	fbPictureInit(pScreen, 0, 0);

#ifdef RENDER
	if ((s = xf86GetOptValString(info->Options, OPTION_SUBPIXEL_ORDER))) {
		if (strcmp(s, "RGB") == 0)
			subPixelOrder = SubPixelHorizontalRGB;
		else if (strcmp(s, "BGR") == 0)
			subPixelOrder = SubPixelHorizontalBGR;
		else if (strcmp(s, "NONE") == 0)
			subPixelOrder = SubPixelNone;
		PictureSetSubpixelOrder(pScreen, subPixelOrder);
	}
#endif

	value = xorgGetVersion() >= XORG_VERSION_NUMERIC(1,18,3,0,0);
	from = X_DEFAULT;

	if (info->use_glamor) {
		if (xf86GetOptValBool(info->Options, OPTION_DRI3, &value))
			from = X_CONFIG;

		if (xf86GetOptValInteger(info->Options, OPTION_DRI, &driLevel) &&
			(driLevel == 2 || driLevel == 3)) {
			from = X_CONFIG;
			value = driLevel == 3;
		}
	}

	if (value) {
		value = amdgpu_sync_init(pScreen) &&
			amdgpu_present_screen_init(pScreen) &&
			amdgpu_dri3_screen_init(pScreen);

		if (!value)
			from = X_WARNING;
	}

	xf86DrvMsg(pScrn->scrnIndex, from, "DRI3 %sabled\n", value ? "en" : "dis");

	pScrn->vtSema = TRUE;
	xf86SetBackingStore(pScreen);

	if (info->directRenderingEnabled) {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "Direct rendering enabled\n");
	} else {
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "Direct rendering disabled\n");
	}

	if (info->use_glamor && info->directRenderingEnabled) {
		xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
			       "Initializing Acceleration\n");
		if (amdgpu_glamor_init(pScreen)) {
			xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				   "Acceleration enabled\n");
		} else {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "Acceleration initialization failed\n");
			xf86DrvMsg(pScrn->scrnIndex, X_INFO,
				   "2D and 3D acceleration disabled\n");
			info->use_glamor = FALSE;
		}
	} else if (info->directRenderingEnabled) {
		if (!amdgpu_pixmap_init(pScreen))
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "3D acceleration disabled\n");
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "2D acceleration disabled\n");
	} else {
		xf86DrvMsg(pScrn->scrnIndex, X_INFO, "2D and 3D cceleration disabled\n");
	}

	/* Init DPMS */
	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "Initializing DPMS\n");
	xf86DPMSInit(pScreen, xf86DPMSSet, 0);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "Initializing Cursor\n");

	/* Set Silken Mouse */
	xf86SetSilkenMouse(pScreen);

	/* Cursor setup */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
		if (AMDGPUCursorInit_KMS(pScreen)) {
		}
	}

	/* DGA setup */
#ifdef XFreeXDGA
	/* DGA is dangerous on kms as the base and framebuffer location may change:
	 * http://lists.freedesktop.org/archives/xorg-devel/2009-September/002113.html
	 */
	/* xf86DiDGAInit(pScreen, info->LinearAddr + pScrn->fbOffset); */
#endif
	if (info->shadow_fb == FALSE) {
		/* Init Xv */
		xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
			       "Initializing Xv\n");
		AMDGPUInitVideo(pScreen);
	}

	if (info->shadow_fb == TRUE) {
		if (!shadowSetup(pScreen)) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "Shadowfb initialization failed\n");
			return FALSE;
		}
	}
	pScrn->pScreen = pScreen;

	if (serverGeneration == 1 && bgNoneRoot && info->use_glamor) {
		info->CreateWindow = pScreen->CreateWindow;
		pScreen->CreateWindow = AMDGPUCreateWindow_oneshot;
	}

	/* Provide SaveScreen & wrap BlockHandler and CloseScreen */
	/* Wrap CloseScreen */
	info->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = AMDGPUCloseScreen_KMS;
	pScreen->SaveScreen = AMDGPUSaveScreen_KMS;
	info->BlockHandler = pScreen->BlockHandler;
	pScreen->BlockHandler = AMDGPUBlockHandler_oneshot;

	info->CreateScreenResources = pScreen->CreateScreenResources;
	pScreen->CreateScreenResources = AMDGPUCreateScreenResources_KMS;

#ifdef AMDGPU_PIXMAP_SHARING
	pScreen->StartPixmapTracking = PixmapStartDirtyTracking;
	pScreen->StopPixmapTracking = PixmapStopDirtyTracking;
#if HAS_SYNC_SHARED_PIXMAP
	pScreen->SyncSharedPixmap = amdgpu_sync_shared_pixmap;
#endif
#endif

	if (!xf86CrtcScreenInit(pScreen))
		return FALSE;

	/* Wrap pointer motion to flip touch screen around */
//    info->PointerMoved = pScrn->PointerMoved;
//    pScrn->PointerMoved = AMDGPUPointerMoved;

	if (!drmmode_setup_colormap(pScreen, pScrn))
		return FALSE;

	/* Note unused options */
	if (serverGeneration == 1)
		xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

	drmmode_init(pScrn, &info->drmmode);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPUScreenInit finished\n");

	return TRUE;
}

Bool AMDGPUEnterVT_KMS(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPUEnterVT_KMS\n");

	amdgpu_set_drm_master(pScrn);

	pScrn->vtSema = TRUE;

	if (!drmmode_set_desired_modes(pScrn, &info->drmmode, TRUE))
		return FALSE;

	return TRUE;
}

void AMDGPULeaveVT_KMS(VT_FUNC_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "AMDGPULeaveVT_KMS\n");

	amdgpu_drop_drm_master(pScrn);

	xf86RotateFreeShadow(pScrn);
	drmmode_scanout_free(pScrn);

	xf86_hide_cursors(pScrn);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, AMDGPU_LOGLEVEL_DEBUG,
		       "Ok, leaving now...\n");
}

Bool AMDGPUSwitchMode_KMS(SWITCH_MODE_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	Bool ret;
	ret = xf86SetSingleMode(pScrn, mode, RR_Rotate_0);
	return ret;

}

void AMDGPUAdjustFrame_KMS(ADJUST_FRAME_ARGS_DECL)
{
	SCRN_INFO_PTR(arg);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	drmmode_adjust_frame(pScrn, &info->drmmode, x, y);
	return;
}

static Bool amdgpu_setup_kernel_mem(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	AMDGPUInfoPtr info = AMDGPUPTR(pScrn);
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	int cpp = info->pixel_bytes;
	int cursor_size;
	int c;

	cursor_size = info->cursor_w * info->cursor_h * 4;
	cursor_size = AMDGPU_ALIGN(cursor_size, AMDGPU_GPU_PAGE_SIZE);
	for (c = 0; c < xf86_config->num_crtc; c++) {
		/* cursor objects */
		if (info->cursor_buffer[c] == NULL) {
			if (info->gbm) {
				info->cursor_buffer[c] = (struct amdgpu_buffer *)calloc(1, sizeof(struct amdgpu_buffer));
				if (!info->cursor_buffer[c]) {
					return FALSE;
				}
				info->cursor_buffer[c]->ref_count = 1;
				info->cursor_buffer[c]->flags = AMDGPU_BO_FLAGS_GBM;

				info->cursor_buffer[c]->bo.gbm = gbm_bo_create(info->gbm,
									       info->cursor_w,
									       info->cursor_h,
									       GBM_FORMAT_ARGB8888,
									       GBM_BO_USE_CURSOR | GBM_BO_USE_WRITE);
				if (!info->cursor_buffer[c]->bo.gbm) {
					xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
						   "Failed to allocate cursor buffer memory\n");
					free(info->cursor_buffer[c]);
					return FALSE;
				}
			} else {
				AMDGPUEntPtr pAMDGPUEnt = AMDGPUEntPriv(pScrn);
				info->cursor_buffer[c] = amdgpu_bo_open(pAMDGPUEnt->pDev,
									cursor_size,
									0,
									AMDGPU_GEM_DOMAIN_VRAM);
				if (!(info->cursor_buffer[c])) {
					ErrorF("Failed to allocate cursor buffer memory\n");
					return FALSE;
				}

				if (amdgpu_bo_cpu_map(info->cursor_buffer[c]->bo.amdgpu,
							&info->cursor_buffer[c]->cpu_ptr)) {
					ErrorF("Failed to map cursor buffer memory\n");
				}
			}

			drmmode_set_cursor(pScrn, &info->drmmode, c,
					   info->cursor_buffer[c]);
		}
	}

	if (info->front_buffer == NULL) {
		int pitch;
		int hint = 0;

		if (info->shadow_primary)
			hint = AMDGPU_CREATE_PIXMAP_LINEAR | AMDGPU_CREATE_PIXMAP_GTT;
		else if (!info->use_glamor)
			hint = AMDGPU_CREATE_PIXMAP_LINEAR;

		info->front_buffer =
			amdgpu_alloc_pixmap_bo(pScrn, pScrn->virtualX,
					       pScrn->virtualY, pScrn->depth,
					       hint, pScrn->bitsPerPixel,
					       &pitch);
		if (!(info->front_buffer)) {
			ErrorF("Failed to allocate front buffer memory\n");
			return FALSE;
		}

		if (!info->use_glamor &&
		    amdgpu_bo_map(pScrn, info->front_buffer) != 0) {
			ErrorF("Failed to map front buffer memory\n");
			return FALSE;
		}

		pScrn->displayWidth = pitch / cpp;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Front buffer pitch: %d bytes\n",
		   pScrn->displayWidth * cpp);
	return TRUE;
}

/* Used to disallow modes that are not supported by the hardware */
ModeStatus AMDGPUValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode,
			   Bool verbose, int flag)
{
	/* There are problems with double scan mode at high clocks
	 * They're likely related PLL and display buffer settings.
	 * Disable these modes for now.
	 */
	if (mode->Flags & V_DBLSCAN) {
		if ((mode->CrtcHDisplay >= 1024) || (mode->CrtcVDisplay >= 768))
			return MODE_CLOCK_RANGE;
	}
	return MODE_OK;
}
