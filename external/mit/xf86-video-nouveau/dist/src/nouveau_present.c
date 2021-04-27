/*
 * Copyright 2013 Red Hat Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs <bskeggs@redhat.com>
 */

#include "nouveau_present.h"
#if defined(DRI3)
#include "nv_include.h"
#include "xf86drmMode.h"

struct nouveau_present {
	struct present_screen_info info;
};

static RRCrtcPtr
nouveau_present_crtc(WindowPtr window)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(window->drawable.pScreen);
	xf86CrtcPtr crtc;

	crtc = nouveau_pick_best_crtc(scrn, FALSE,
                                  window->drawable.x,
                                  window->drawable.y,
                                  window->drawable.width,
                                  window->drawable.height);

	if (!crtc)
		return NULL;

	return crtc->randr_crtc;
}

static int
nouveau_present_ust_msc(RRCrtcPtr rrcrtc, uint64_t *ust, uint64_t *msc)
{
	xf86CrtcPtr crtc = rrcrtc->devPrivate;
	NVPtr pNv = NVPTR(crtc->scrn);
	drmVBlank args;
	int ret;

	args.request.type = DRM_VBLANK_RELATIVE;
	args.request.type |= drmmode_head(crtc) << DRM_VBLANK_HIGH_CRTC_SHIFT;
	args.request.sequence = 0,
	args.request.signal = 0,

	ret = drmWaitVBlank(pNv->dev->fd, &args);
	if (ret) {
		*ust = *msc = 0;
		return BadMatch;
	}

	*ust = (CARD64)args.reply.tval_sec * 1000000 + args.reply.tval_usec;
	*msc = args.reply.sequence;
	return Success;
}

struct nouveau_present_vblank {
	uint64_t msc;
};

static void
nouveau_present_vblank(void *priv, uint64_t name, uint64_t ust, uint32_t msc_lo)
{
	struct nouveau_present_vblank *event = priv;
	uint64_t msc;

	msc = (event->msc & 0xffffffff00000000ULL) | msc_lo;
	if (msc < event->msc)
		event->msc += 1ULL << 32;

	present_event_notify(name, ust, msc);
}

static int
nouveau_present_vblank_queue(RRCrtcPtr rrcrtc, uint64_t event_id, uint64_t msc)
{
	xf86CrtcPtr crtc = rrcrtc->devPrivate;
	NVPtr pNv = NVPTR(crtc->scrn);
	drmVBlank args;
	struct nouveau_present_vblank *event;
	void *token;
	int ret;

	event = drmmode_event_queue(crtc->scrn, event_id, sizeof(*event),
				    nouveau_present_vblank, &token);
	if (!event)
		return BadAlloc;

	event->msc = msc;

	args.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
	args.request.type |= drmmode_head(crtc) << DRM_VBLANK_HIGH_CRTC_SHIFT;
	args.request.sequence = msc;
	args.request.signal = (unsigned long)token;

	while ((ret = drmWaitVBlank(pNv->dev->fd, &args)) != 0) {
		if (errno != EBUSY) {
			xf86DrvMsgVerb(crtc->scrn->scrnIndex, X_WARNING, 4,
				   "PRESENT: Wait for VBlank failed: %s\n", strerror(errno));
			drmmode_event_abort(crtc->scrn, event_id, false);
			return BadAlloc;
		}
		ret = drmmode_event_flush(crtc->scrn);
		if (ret < 0) {
			xf86DrvMsgVerb(crtc->scrn->scrnIndex, X_WARNING, 4,
				   "PRESENT: Event flush failed\n");
			drmmode_event_abort(crtc->scrn, event_id, false);
			return BadAlloc;
		}
	}

	return Success;
}

static void
nouveau_present_vblank_abort(RRCrtcPtr rrcrtc, uint64_t event_id, uint64_t msc)
{
	xf86CrtcPtr crtc = rrcrtc->devPrivate;
	drmmode_event_abort(crtc->scrn, event_id, true);
}

static void
nouveau_present_flush(WindowPtr window)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(window->drawable.pScreen);
	NVPtr pNv = NVPTR(scrn);
	if (pNv->Flush)
		pNv->Flush(scrn);
}

struct nouveau_present_flip {
	uint64_t msc;
	uint32_t old;
	int fd;
};

static Bool
nouveau_present_flip_check(RRCrtcPtr rrcrtc, WindowPtr window,
			   PixmapPtr pixmap, Bool sync_flip)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(window->drawable.pScreen);
	NVPtr pNv = NVPTR(scrn);
	xf86CrtcPtr crtc = rrcrtc->devPrivate;
	struct nouveau_pixmap *priv = nouveau_pixmap(pixmap);

	if (!scrn->vtSema || !drmmode_crtc_on(crtc) || crtc->rotatedData)
		return FALSE;

	if (!priv) {
		/* The pixmap may not have had backing for low-memory GPUs, or
		 * if we ran out of VRAM. Make sure it's properly backed for
		 * flipping.
		 */
		pNv->exa_force_cp = TRUE;
		exaMoveInPixmap(pixmap);
		pNv->exa_force_cp = FALSE;
		priv = nouveau_pixmap(pixmap);
	}

	return priv ? TRUE : FALSE;
}

static void
nouveau_present_flip(void *priv, uint64_t name, uint64_t ust, uint32_t msc_lo)
{
	struct nouveau_present_flip *flip = priv;
	uint64_t msc;

	msc = (flip->msc & ~0xffffffffULL) | msc_lo;
	if (msc < flip->msc)
		msc += 1ULL << 32;

	present_event_notify(name, ust, msc);
	drmModeRmFB(flip->fd, flip->old);
}

static Bool
nouveau_present_flip_exec(ScrnInfoPtr scrn, uint64_t event_id, int sync,
			  uint64_t target_msc, PixmapPtr pixmap, Bool vsync)
{
	struct nouveau_pixmap *priv = nouveau_pixmap(pixmap);
	NVPtr pNv = NVPTR(scrn);
	uint32_t next_fb;
	void *token;
	int ret;

	ret = drmModeAddFB(pNv->dev->fd, pixmap->drawable.width,
			   pixmap->drawable.height, pixmap->drawable.depth,
			   pixmap->drawable.bitsPerPixel, pixmap->devKind,
			   priv->bo->handle, &next_fb);
	if (ret == 0) {
		struct nouveau_present_flip *flip =
			drmmode_event_queue(scrn, event_id, sizeof(*flip),
					    nouveau_present_flip, &token);
		if (flip) {
			xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);
			int last = 0, i;

			drmmode_swap(scrn, next_fb, &flip->old);
			flip->fd = pNv->dev->fd;
			flip->msc = target_msc;

			for (i = 0; i < config->num_crtc; i++) {
				if (drmmode_crtc_on(config->crtc[i]))
					last = i;
			}

			for (i = 0; i < config->num_crtc; i++) {
				int type = vsync ? 0 : DRM_MODE_PAGE_FLIP_ASYNC;
				int crtc = drmmode_crtc(config->crtc[i]);
				void *user = NULL;

				if (!drmmode_crtc_on(config->crtc[i]))
					continue;

				if (token && ((crtc == sync) || (i == last))) {
					type |= DRM_MODE_PAGE_FLIP_EVENT;
					user  = token;
				}

				ret = drmModePageFlip(pNv->dev->fd, crtc,
						      next_fb, type, user);
				if (ret == 0 && user) {
					token = NULL;
				}
			}

			if (token == NULL) {
				return TRUE;
			}

			drmmode_swap(scrn, flip->old, &next_fb);
			drmmode_event_abort(scrn, event_id, false);
		}

		drmModeRmFB(pNv->dev->fd, next_fb);
	}

	return FALSE;
}

static Bool
nouveau_present_flip_next(RRCrtcPtr rrcrtc, uint64_t event_id,
			  uint64_t target_msc, PixmapPtr pixmap, Bool vsync)
{
	xf86CrtcPtr crtc = rrcrtc->devPrivate;
	ScrnInfoPtr scrn = crtc->scrn;
	return nouveau_present_flip_exec(scrn, event_id, drmmode_crtc(crtc),
					 target_msc, pixmap, vsync);
}

static void
nouveau_present_flip_stop(ScreenPtr screen, uint64_t event_id)
{
	PixmapPtr pixmap = screen->GetScreenPixmap(screen);
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	nouveau_present_flip_exec(scrn, event_id, 0, 0, pixmap, TRUE);
}

void
nouveau_present_fini(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);
	if (pNv->present) {
		free(pNv->present);
		pNv->present = NULL;
	}
}

Bool
nouveau_present_init(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);
	struct nouveau_present *present;
	uint64_t value;
	int ret;

	present = pNv->present = calloc(1, sizeof(*present));
	if (!present)
		return FALSE;

	present->info.version = PRESENT_SCREEN_INFO_VERSION;
	present->info.get_crtc = nouveau_present_crtc;
	present->info.get_ust_msc = nouveau_present_ust_msc;
	present->info.queue_vblank = nouveau_present_vblank_queue;
	present->info.abort_vblank = nouveau_present_vblank_abort;
	present->info.flush = nouveau_present_flush;

	if (pNv->has_pageflip) {
#ifdef DRM_CAP_ASYNC_PAGE_FLIP
		ret = drmGetCap(pNv->dev->fd, DRM_CAP_ASYNC_PAGE_FLIP, &value);
		if (ret == 0 && value == 1)
			present->info.capabilities |= PresentCapabilityAsync;
#endif
		present->info.check_flip = nouveau_present_flip_check;
		present->info.flip = nouveau_present_flip_next;
		present->info.unflip = nouveau_present_flip_stop;
	}

	return present_screen_init(screen, &present->info);
}
#endif
