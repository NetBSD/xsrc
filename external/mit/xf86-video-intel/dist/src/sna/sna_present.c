/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <errno.h>
#include <xf86drm.h>

#include "sna.h"

#include <xf86.h>
#include <present.h>

static present_screen_info_rec present_info;

struct sna_present_event {
	xf86CrtcPtr crtc;
	struct sna *sna;
	struct list link;
	uint64_t *event_id;
	uint64_t target_msc;
	int n_event_id;
	bool queued:1;
	bool active:1;
};

static void sna_present_unflip(ScreenPtr screen, uint64_t event_id);
static bool sna_present_queue(struct sna_present_event *info,
			      uint64_t last_msc);

static inline struct sna_present_event *
to_present_event(uintptr_t  data)
{
	return (struct sna_present_event *)(data & ~3);
}

static struct sna_present_event *info_alloc(struct sna *sna)
{
	struct sna_present_event *info;

	info = sna->present.freed_info;
	if (info) {
		sna->present.freed_info = NULL;
		return info;
	}

	return malloc(sizeof(struct sna_present_event) + sizeof(uint64_t));
}

static void info_free(struct sna_present_event *info)
{
	struct sna *sna = info->sna;

	if (sna->present.freed_info)
		free(sna->present.freed_info);

	sna->present.freed_info = info;
}

static inline bool msc_before(uint64_t msc, uint64_t target)
{
	return (int64_t)(msc - target) < 0;
}

#define MARK_PRESENT(x) ((void *)((uintptr_t)(x) | 2))

static inline xf86CrtcPtr unmask_crtc(xf86CrtcPtr crtc)
{
	return (xf86CrtcPtr)((uintptr_t)crtc & ~1);
}

static inline xf86CrtcPtr mark_crtc(xf86CrtcPtr crtc)
{
	return (xf86CrtcPtr)((uintptr_t)crtc | 1);
}

static inline bool has_vblank(xf86CrtcPtr crtc)
{
	return (uintptr_t)crtc & 1;
}

static inline int pipe_from_crtc(RRCrtcPtr crtc)
{
	return crtc ? sna_crtc_pipe(crtc->devPrivate) : -1;
}

static uint32_t pipe_select(int pipe)
{
	if (pipe > 1)
		return pipe << DRM_VBLANK_HIGH_CRTC_SHIFT;
	else if (pipe > 0)
		return DRM_VBLANK_SECONDARY;
	else
		return 0;
}

static inline int sna_wait_vblank(struct sna *sna, union drm_wait_vblank *vbl, int pipe)
{
	DBG(("%s(pipe=%d, waiting until seq=%u%s)\n",
	     __FUNCTION__, pipe, vbl->request.sequence,
	     vbl->request.type & DRM_VBLANK_RELATIVE ? " [relative]" : ""));
	vbl->request.type |= pipe_select(pipe);
	return drmIoctl(sna->kgem.fd, DRM_IOCTL_WAIT_VBLANK, vbl);
}

static uint64_t gettime_ust64(void)
{
	struct timespec tv;

	if (clock_gettime(CLOCK_MONOTONIC, &tv))
		return GetTimeInMicros();

	return ust64(tv.tv_sec, tv.tv_nsec / 1000);
}

static void vblank_complete(struct sna_present_event *info,
			    uint64_t ust, uint64_t msc)
{
	struct list * const q = sna_crtc_vblank_queue(info->crtc);
	int n;

	do {
		assert(sna_crtc_vblank_queue(info->crtc) == q);

		if (msc_before(msc, info->target_msc)) {
			DBG(("%s: event=%d too early, now %lld, expected %lld\n",
			     __FUNCTION__,
			     info->event_id[0],
			     (long long)msc, (long long)info->target_msc));
			if (sna_present_queue(info, msc))
				return;
		}

		DBG(("%s: %d events complete\n", __FUNCTION__, info->n_event_id));
		for (n = 0; n < info->n_event_id; n++) {
			DBG(("%s: pipe=%d tv=%d.%06d msc=%lld (target=%lld), event=%lld complete%s\n", __FUNCTION__,
			     sna_crtc_pipe(info->crtc),
			     (int)(ust / 1000000), (int)(ust % 1000000),
			     (long long)msc, (long long)info->target_msc,
			     (long long)info->event_id[n],
			     info->target_msc && msc == (uint32_t)info->target_msc ? "" : ": MISS"));
			present_event_notify(info->event_id[n], ust, msc);
		}
		if (info->n_event_id > 1)
			free(info->event_id);

		_list_del(&info->link);
		info_free(info);

		info = list_entry(info->link.next, typeof(*info), link);
	} while (q != &info->link && !info->queued);
}

static uint32_t msc_to_delay(xf86CrtcPtr crtc, uint64_t target)
{
	const DisplayModeRec *mode = &crtc->desiredMode;
	const struct ust_msc *swap = sna_crtc_last_swap(crtc);
	int64_t delay, subframe;

	assert(mode->Clock);

	delay = target - swap->msc;
	assert(delay >= 0);
	if (delay > 1) { /* try to use the hw vblank for the last frame */
		delay--;
		subframe = 0;
	} else {
		subframe = gettime_ust64() - swap_ust(swap);
		subframe += 500;
		subframe /= 1000;
	}
	delay *= mode->VTotal * mode->HTotal / mode->Clock;
	if (subframe < delay)
		delay -= subframe;
	else
		delay = 0;

	DBG(("%s: sleep %d frames, %llu ms\n", __FUNCTION__,
	     (int)(target - swap->msc), (long long)delay));
	assert(delay >= 0);
	return MIN(delay, INT32_MAX);
}

static void add_to_crtc_vblank(struct sna_present_event *info,
				int delta)
{
	info->active = true;
	if (delta == 1 && info->crtc) {
		sna_crtc_set_vblank(info->crtc);
		info->crtc = mark_crtc(info->crtc);
	}
}

static CARD32 sna_fake_vblank_handler(OsTimerPtr timer, CARD32 now, void *data)
{
	struct sna_present_event *info = data;
	union drm_wait_vblank vbl;
	uint64_t msc, ust;

	DBG(("%s(event=%lldx%d, now=%d)\n", __FUNCTION__, (long long)info->event_id[0], info->n_event_id, now));
	assert(info->queued);

	VG_CLEAR(vbl);
	vbl.request.type = DRM_VBLANK_RELATIVE;
	vbl.request.sequence = 0;
	if (sna_wait_vblank(info->sna, &vbl, sna_crtc_pipe(info->crtc)) == 0) {
		ust = ust64(vbl.reply.tval_sec, vbl.reply.tval_usec);
		msc = sna_crtc_record_vblank(info->crtc, &vbl);
		DBG(("%s: event=%lld, target msc=%lld, now %lld\n",
		     __FUNCTION__, (long long)info->event_id[0], (long long)info->target_msc, (long long)msc));
		if (msc_before(msc, info->target_msc)) {
			int delta = info->target_msc - msc;
			uint32_t delay;

			DBG(("%s: too early, requeuing delta=%d\n", __FUNCTION__, delta));
			assert(info->target_msc - msc < 1ull<<31);
			if (delta <= 2) {
				vbl.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
				vbl.request.sequence = info->target_msc;
				vbl.request.signal = (uintptr_t)MARK_PRESENT(info);
				if (sna_wait_vblank(info->sna, &vbl, sna_crtc_pipe(info->crtc)) == 0) {
					DBG(("%s: scheduled new vblank event for %lld\n", __FUNCTION__, (long long)info->target_msc));
					add_to_crtc_vblank(info, delta);
					free(timer);
					return 0;
				}
			}

			delay = msc_to_delay(info->crtc, info->target_msc);
			if (delay) {
				DBG(("%s: requeueing timer for %dms delay\n", __FUNCTION__, delay));
				return delay;
			}

			/* As a last resort use a blocking wait.
			 * Less than a millisecond for (hopefully) a rare case.
			 */
			DBG(("%s: blocking wait!\n", __FUNCTION__));
			vbl.request.type = DRM_VBLANK_ABSOLUTE;
			vbl.request.sequence = info->target_msc;
			if (sna_wait_vblank(info->sna, &vbl, sna_crtc_pipe(info->crtc)) == 0) {
				ust = ust64(vbl.reply.tval_sec, vbl.reply.tval_usec);
				msc = sna_crtc_record_vblank(info->crtc, &vbl);
			} else {
				DBG(("%s: blocking wait failed, fudging\n",
				     __FUNCTION__));
				goto fixup;
			}
		}
	} else {
fixup:
		ust = gettime_ust64();
		msc = info->target_msc;
		DBG(("%s: event=%lld, CRTC OFF, target msc=%lld, was %lld (off)\n",
		     __FUNCTION__, (long long)info->event_id[0], (long long)info->target_msc, (long long)sna_crtc_last_swap(info->crtc)->msc));
	}

	vblank_complete(info, ust, msc);
	free(timer);
	return 0;
}

static bool sna_fake_vblank(struct sna_present_event *info)
{
	const struct ust_msc *swap = sna_crtc_last_swap(info->crtc);
	uint32_t delay;

	if (msc_before(swap->msc, info->target_msc))
		delay = msc_to_delay(info->crtc, info->target_msc);
	else
		delay = 0;

	DBG(("%s(event=%lldx%d, target_msc=%lld, msc=%lld, delay=%ums)\n",
	     __FUNCTION__, (long long)info->event_id[0], info->n_event_id,
	     (long long)info->target_msc, (long long)swap->msc, delay));
	if (delay == 0) {
		uint64_t ust, msc;

		if (msc_before(swap->msc, info->target_msc)) {
			/* Fixup and pretend it completed immediately */
			msc = info->target_msc;
			ust = gettime_ust64();
		} else {
			msc = swap->msc;
			ust = swap_ust(swap);
		}

		vblank_complete(info, ust, msc);
		return true;
	}

	return TimerSet(NULL, 0, delay, sna_fake_vblank_handler, info);
}

static bool sna_present_queue(struct sna_present_event *info,
			      uint64_t last_msc)
{
	union drm_wait_vblank vbl;
	int delta = info->target_msc - last_msc;

	DBG(("%s: target msc=%llu, seq=%u (last_msc=%llu), delta=%d\n",
	     __FUNCTION__,
	     (long long)info->target_msc,
	     (unsigned)info->target_msc,
	     (long long)last_msc,
	     delta));
	assert(info->target_msc - last_msc < 1ull<<31);
	assert(delta >= 0);

	VG_CLEAR(vbl);
	vbl.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
	vbl.request.sequence = info->target_msc;
	vbl.request.signal = (uintptr_t)MARK_PRESENT(info);
	if (delta > 2 ||
	    sna_wait_vblank(info->sna, &vbl, sna_crtc_pipe(info->crtc))) {
		DBG(("%s: vblank enqueue failed, faking delta=%d\n", __FUNCTION__, delta));
		if (!sna_fake_vblank(info))
			return false;
	} else {
		add_to_crtc_vblank(info, delta);
	}

	info->queued = true;
	return true;
}

static RRCrtcPtr
sna_present_get_crtc(WindowPtr window)
{
	struct sna *sna = to_sna_from_drawable(&window->drawable);
	BoxRec box;
	xf86CrtcPtr crtc;

	DBG(("%s: window=%ld (pixmap=%ld), box=(%d, %d)x(%d, %d)\n",
	     __FUNCTION__, window->drawable.id, get_window_pixmap(window)->drawable.serialNumber,
	     window->drawable.x, window->drawable.y,
	     window->drawable.width, window->drawable.height));

	box.x1 = window->drawable.x;
	box.y1 = window->drawable.y;
	box.x2 = box.x1 + window->drawable.width;
	box.y2 = box.y1 + window->drawable.height;

	crtc = sna_covering_crtc(sna, &box, NULL);
	if (crtc)
		return crtc->randr_crtc;

	return NULL;
}

static void add_keepalive(struct sna *sna, xf86CrtcPtr crtc, uint64_t msc)
{
	struct list *q = sna_crtc_vblank_queue(crtc);
	struct sna_present_event *info, *tmp;
	union drm_wait_vblank vbl;

	list_for_each_entry(tmp, q, link) {
		if (tmp->target_msc == msc) {
			DBG(("%s: vblank already queued for target_msc=%lld\n",
			     __FUNCTION__, (long long)msc));
			return;
		}

		if ((int64_t)(tmp->target_msc - msc) > 0)
			break;
	}

	DBG(("%s: adding keepalive for target_msc=%lld\n",
	     __FUNCTION__, (long long)msc));

	info = info_alloc(sna);
	if (!info)
		return;

	info->crtc = crtc;
	info->sna = sna;
	info->target_msc = msc;
	info->event_id = (uint64_t *)(info + 1);
	info->n_event_id = 0;

	VG_CLEAR(vbl);
	vbl.request.type = DRM_VBLANK_ABSOLUTE | DRM_VBLANK_EVENT;
	vbl.request.sequence = msc;
	vbl.request.signal = (uintptr_t)MARK_PRESENT(info);

	if (sna_wait_vblank(info->sna, &vbl, sna_crtc_pipe(crtc)) == 0) {
		list_add_tail(&info->link, &tmp->link);
		add_to_crtc_vblank(info, 1);
		info->queued = true;
	} else
		info_free(info);
}

static int
sna_present_get_ust_msc(RRCrtcPtr crtc, CARD64 *ust, CARD64 *msc)
{
	struct sna *sna = to_sna_from_screen(crtc->pScreen);
	union drm_wait_vblank vbl;

	DBG(("%s(pipe=%d)\n", __FUNCTION__, sna_crtc_pipe(crtc->devPrivate)));
	if (sna_crtc_has_vblank(crtc->devPrivate)) {
		DBG(("%s: vblank active, reusing last swap msc/ust\n",
		     __FUNCTION__));
		goto last;
	}

	VG_CLEAR(vbl);
	vbl.request.type = DRM_VBLANK_RELATIVE;
	vbl.request.sequence = 0;
	if (sna_wait_vblank(sna, &vbl, sna_crtc_pipe(crtc->devPrivate)) == 0) {
		*ust = ust64(vbl.reply.tval_sec, vbl.reply.tval_usec);
		*msc = sna_crtc_record_vblank(crtc->devPrivate, &vbl);

		add_keepalive(sna, crtc->devPrivate, *msc + 1);
	} else {
		const struct ust_msc *swap;
last:
		swap = sna_crtc_last_swap(crtc->devPrivate);
		*ust = swap_ust(swap);
		*msc = swap->msc;
	}

	DBG(("%s: pipe=%d, tv=%d.%06d seq=%d msc=%lld\n", __FUNCTION__,
	     sna_crtc_pipe(crtc->devPrivate),
	     (int)(*ust / 1000000), (int)(*ust % 1000000),
	     vbl.reply.sequence, (long long)*msc));

	return Success;
}

void
sna_present_vblank_handler(struct drm_event_vblank *event)
{
	struct sna_present_event *info = to_present_event(event->user_data);
	uint64_t msc;

	if (!info->active) {
		DBG(("%s: arrived unexpectedly early (not active)\n", __FUNCTION__));
		assert(!has_vblank(info->crtc));
		return;
	}

	if (has_vblank(info->crtc)) {
		DBG(("%s: clearing immediate flag\n", __FUNCTION__));
		info->crtc = unmask_crtc(info->crtc);
		sna_crtc_clear_vblank(info->crtc);
	}

	msc = sna_crtc_record_event(info->crtc, event);

	vblank_complete(info, ust64(event->tv_sec, event->tv_usec), msc);
}

static int
sna_present_queue_vblank(RRCrtcPtr crtc, uint64_t event_id, uint64_t msc)
{
	struct sna *sna = to_sna_from_screen(crtc->pScreen);
	struct sna_present_event *info, *tmp;
	const struct ust_msc *swap;
	struct list *q;

	if (!sna_crtc_is_on(crtc->devPrivate))
		return BadAlloc;

	swap = sna_crtc_last_swap(crtc->devPrivate);
	DBG(("%s(pipe=%d, event=%lld, msc=%lld, last swap=%lld)\n",
	     __FUNCTION__, sna_crtc_pipe(crtc->devPrivate),
	     (long long)event_id, (long long)msc, (long long)swap->msc));

	if (warn_unless((int64_t)(msc - swap->msc) >= 0)) {
		DBG(("%s: pipe=%d tv=%d.%06d msc=%lld (target=%lld), event=%lld complete\n", __FUNCTION__,
		     sna_crtc_pipe(crtc->devPrivate),
		     swap->tv_sec, swap->tv_usec,
		     (long long)swap->msc, (long long)msc,
		     (long long)event_id));
		present_event_notify(event_id, swap_ust(swap), swap->msc);
		return Success;
	}
	if (warn_unless(msc - swap->msc < 1ull<<31))
		return BadValue;

	q = sna_crtc_vblank_queue(crtc->devPrivate);
	list_for_each_entry(tmp, q, link) {
		if (tmp->target_msc == msc) {
			uint64_t *events = tmp->event_id;

			if (tmp->n_event_id &&
			    is_power_of_two(tmp->n_event_id)) {
				events = malloc(2*sizeof(uint64_t)*tmp->n_event_id);
				if (events == NULL)
					return BadAlloc;

				memcpy(events,
				       tmp->event_id,
				       tmp->n_event_id*sizeof(uint64_t));
				if (tmp->n_event_id != 1)
					free(tmp->event_id);
				tmp->event_id = events;
			}

			DBG(("%s: appending event=%lld to vblank %lld x %d\n",
			     __FUNCTION__, (long long)event_id, (long long)msc, tmp->n_event_id+1));
			events[tmp->n_event_id++] = event_id;
			return Success;
		}
		if ((int64_t)(tmp->target_msc - msc) > 0) {
			DBG(("%s: previous target_msc=%lld invalid for coalescing\n",
			     __FUNCTION__, (long long)tmp->target_msc));
			break;
		}
	}

	info = info_alloc(sna);
	if (info == NULL)
		return BadAlloc;

	info->crtc = crtc->devPrivate;
	info->sna = sna;
	info->target_msc = msc;
	info->event_id = (uint64_t *)(info + 1);
	info->event_id[0] = event_id;
	info->n_event_id = 1;
	list_add_tail(&info->link, &tmp->link);
	info->queued = false;
	info->active = false;

	if (info->link.prev == q && !sna_present_queue(info, swap->msc)) {
		list_del(&info->link);
		info_free(info);
		return BadAlloc;
	}

	return Success;
}

static void
sna_present_abort_vblank(RRCrtcPtr crtc, uint64_t event_id, uint64_t msc)
{
	DBG(("%s(pipe=%d, event=%lld, msc=%lld)\n",
	     __FUNCTION__, pipe_from_crtc(crtc),
	     (long long)event_id, (long long)msc));
}

static void
sna_present_flush(WindowPtr window)
{
}

static bool
check_flip__crtc(struct sna *sna,
		 RRCrtcPtr crtc)
{
	if (!sna_crtc_is_on(crtc->devPrivate)) {
		DBG(("%s: CRTC off\n", __FUNCTION__));
		return false;
	}

	assert(sna->scrn->vtSema);

	if (!sna->mode.front_active) {
		DBG(("%s: DPMS off, no flips\n", __FUNCTION__));
		return FALSE;
	}

	if (sna->mode.rr_active) {
		DBG(("%s: RandR transformation active\n", __FUNCTION__));
		return false;
	}

	return true;
}

static Bool
sna_present_check_flip(RRCrtcPtr crtc,
		       WindowPtr window,
		       PixmapPtr pixmap,
		       Bool sync_flip)
{
	struct sna *sna = to_sna_from_pixmap(pixmap);
	struct sna_pixmap *flip;

	DBG(("%s(pipe=%d, pixmap=%ld, sync_flip=%d)\n",
	     __FUNCTION__,
	     pipe_from_crtc(crtc),
	     pixmap->drawable.serialNumber,
	     sync_flip));

	if (!sna->scrn->vtSema) {
		DBG(("%s: VT switched away, no flips\n", __FUNCTION__));
		return FALSE;
	}

	if (sna->flags & SNA_NO_FLIP) {
		DBG(("%s: flips not suported\n", __FUNCTION__));
		return FALSE;
	}

	if (sync_flip) {
		if ((sna->flags & SNA_HAS_FLIP) == 0) {
			DBG(("%s: sync flips not suported\n", __FUNCTION__));
			return FALSE;
		}
	} else {
		if ((sna->flags & SNA_HAS_ASYNC_FLIP) == 0) {
			DBG(("%s: async flips not suported\n", __FUNCTION__));
			return FALSE;
		}
	}

	if (!check_flip__crtc(sna, crtc)) {
		DBG(("%s: flip invalid for CRTC\n", __FUNCTION__));
		return FALSE;
	}

	flip = sna_pixmap(pixmap);
	if (flip == NULL) {
		DBG(("%s: unattached pixmap\n", __FUNCTION__));
		return FALSE;
	}

	if (flip->cpu_bo && IS_STATIC_PTR(flip->ptr)) {
		DBG(("%s: SHM pixmap\n", __FUNCTION__));
		return FALSE;
	}

	if (flip->pinned) {
		assert(flip->gpu_bo);
		if (sna->flags & SNA_LINEAR_FB) {
			if (flip->gpu_bo->tiling != I915_TILING_NONE) {
				DBG(("%s: pined bo, tilng=%d needs NONE\n",
				     __FUNCTION__, flip->gpu_bo->tiling));
				return FALSE;
			}
		} else {
			if (!sna->kgem.can_scanout_y &&
			    flip->gpu_bo->tiling == I915_TILING_Y) {
				DBG(("%s: pined bo, tilng=%d and can't scanout Y\n",
				     __FUNCTION__, flip->gpu_bo->tiling));
				return FALSE;
			}
		}

		if (flip->gpu_bo->pitch & 63) {
			DBG(("%s: pined bo, bad pitch=%d\n",
			     __FUNCTION__, flip->gpu_bo->pitch));
			return FALSE;
		}
	}

	return TRUE;
}

static Bool
flip__async(struct sna *sna,
	    RRCrtcPtr crtc,
	    uint64_t event_id,
	    uint64_t target_msc,
	    struct kgem_bo *bo)
{
	DBG(("%s(pipe=%d, event=%lld, handle=%d)\n",
	     __FUNCTION__,
	     pipe_from_crtc(crtc),
	     (long long)event_id,
	     bo->handle));

	if (!sna_page_flip(sna, bo, NULL, NULL)) {
		DBG(("%s: async pageflip failed\n", __FUNCTION__));
		present_info.capabilities &= ~PresentCapabilityAsync;
		return FALSE;
	}

	DBG(("%s: pipe=%d tv=%ld.%06d msc=%lld (target=%lld), event=%lld complete\n", __FUNCTION__,
	     pipe_from_crtc(crtc),
	     (long)(gettime_ust64() / 1000000), (int)(gettime_ust64() % 1000000),
	     crtc ? (long long)sna_crtc_last_swap(crtc->devPrivate)->msc : 0LL,
	     (long long)target_msc, (long long)event_id));
	present_event_notify(event_id, gettime_ust64(), target_msc);
	return TRUE;
}

static void
present_flip_handler(struct drm_event_vblank *event, void *data)
{
	struct sna_present_event *info = data;
	struct ust_msc swap;

	DBG(("%s(sequence=%d): event=%lld\n", __FUNCTION__, event->sequence, (long long)info->event_id[0]));
	assert(info->n_event_id == 1);
	if (!info->active) {
		DBG(("%s: arrived unexpectedly early (not active)\n", __FUNCTION__));
		return;
	}

	if (info->crtc == NULL) {
		swap.tv_sec = event->tv_sec;
		swap.tv_usec = event->tv_usec;
		swap.msc = event->sequence;
	} else {
		info->crtc = unmask_crtc(info->crtc);
		swap = *sna_crtc_last_swap(info->crtc);
	}

	DBG(("%s: pipe=%d, tv=%d.%06d msc=%lld (target %lld), event=%lld complete%s\n", __FUNCTION__,
	     info->crtc ? sna_crtc_pipe(info->crtc) : -1,
	     swap.tv_sec, swap.tv_usec, (long long)swap.msc,
	     (long long)info->target_msc,
	     (long long)info->event_id[0],
	     info->target_msc && info->target_msc == swap.msc ? "" : ": MISS"));
	present_event_notify(info->event_id[0], swap_ust(&swap), swap.msc);
	if (info->crtc) {
		sna_crtc_clear_vblank(info->crtc);
		if (!sna_crtc_has_vblank(info->crtc))
			add_keepalive(info->sna, info->crtc, swap.msc + 1);
	}

	if (info->sna->present.unflip) {
		DBG(("%s: executing queued unflip (event=%lld)\n", __FUNCTION__, (long long)info->sna->present.unflip));
		sna_present_unflip(xf86ScrnToScreen(info->sna->scrn),
				   info->sna->present.unflip);
		info->sna->present.unflip = 0;
	}
	info_free(info);
}

static Bool
flip(struct sna *sna,
     RRCrtcPtr crtc,
     uint64_t event_id,
     uint64_t target_msc,
     struct kgem_bo *bo)
{
	struct sna_present_event *info;

	DBG(("%s(pipe=%d, event=%lld, handle=%d)\n",
	     __FUNCTION__,
	     pipe_from_crtc(crtc),
	     (long long)event_id,
	     bo->handle));

	info = info_alloc(sna);
	if (info == NULL)
		return FALSE;

	info->crtc = crtc ? crtc->devPrivate : NULL;
	info->sna = sna;
	info->event_id = (uint64_t *)(info + 1);
	info->event_id[0] = event_id;
	info->n_event_id = 1;
	info->target_msc = target_msc;
	info->active = false;

	if (!sna_page_flip(sna, bo, present_flip_handler, info)) {
		DBG(("%s: pageflip failed\n", __FUNCTION__));
		info_free(info);
		return FALSE;
	}

	add_to_crtc_vblank(info, 1);
	return TRUE;
}

static struct kgem_bo *
get_flip_bo(PixmapPtr pixmap)
{
	struct sna *sna = to_sna_from_pixmap(pixmap);
	struct sna_pixmap *priv;

	DBG(("%s(pixmap=%ld)\n", __FUNCTION__, pixmap->drawable.serialNumber));

	priv = sna_pixmap_move_to_gpu(pixmap, MOVE_READ | __MOVE_SCANOUT | __MOVE_FORCE);
	if (priv == NULL) {
		DBG(("%s: cannot force pixmap to the GPU\n", __FUNCTION__));
		return NULL;
	}

	if (priv->gpu_bo->scanout)
		return priv->gpu_bo;

	if (sna->kgem.has_llc && !wedged(sna) && !priv->pinned) {
		struct kgem_bo *bo;
		uint32_t tiling;

		tiling = I915_TILING_NONE;
		if ((sna->flags & SNA_LINEAR_FB) == 0)
			tiling = I915_TILING_X;

		bo = kgem_create_2d(&sna->kgem,
				    pixmap->drawable.width,
				    pixmap->drawable.height,
				    pixmap->drawable.bitsPerPixel,
				    tiling, CREATE_SCANOUT | CREATE_CACHED);
		if (bo) {
			BoxRec box;

			box.x1 = box.y1 = 0;
			box.x2 = pixmap->drawable.width;
			box.y2 = pixmap->drawable.height;

			if (sna->render.copy_boxes(sna, GXcopy,
						   &pixmap->drawable, priv->gpu_bo, 0, 0,
						   &pixmap->drawable, bo, 0, 0,
						   &box, 1, 0)) {
				sna_pixmap_unmap(pixmap, priv);
				kgem_bo_destroy(&sna->kgem, priv->gpu_bo);

				priv->gpu_bo = bo;
			} else
				kgem_bo_destroy(&sna->kgem, bo);
		}
	}

	if (sna->flags & SNA_LINEAR_FB &&
	    priv->gpu_bo->tiling &&
	    !sna_pixmap_change_tiling(pixmap, I915_TILING_NONE)) {
		DBG(("%s: invalid tiling for scanout, user requires linear\n", __FUNCTION__));
		return NULL;
	}

	if (priv->gpu_bo->tiling == I915_TILING_Y &&
	    !sna->kgem.can_scanout_y &&
	    !sna_pixmap_change_tiling(pixmap, I915_TILING_X)) {
		DBG(("%s: invalid Y-tiling, cannot convert\n", __FUNCTION__));
		return NULL;
	}

	if (priv->gpu_bo->pitch & 63) {
		DBG(("%s: invalid pitch, no conversion\n", __FUNCTION__));
		return NULL;
	}

	return priv->gpu_bo;
}

static Bool
sna_present_flip(RRCrtcPtr crtc,
		 uint64_t event_id,
		 uint64_t target_msc,
		 PixmapPtr pixmap,
		 Bool sync_flip)
{
	struct sna *sna = to_sna_from_pixmap(pixmap);
	struct kgem_bo *bo;

	DBG(("%s(pipe=%d, event=%lld, msc=%lld, pixmap=%ld, sync?=%d)\n",
	     __FUNCTION__,
	     pipe_from_crtc(crtc),
	     (long long)event_id,
	     (long long)target_msc,
	     pixmap->drawable.serialNumber, sync_flip));

	if (!check_flip__crtc(sna, crtc)) {
		DBG(("%s: flip invalid for CRTC\n", __FUNCTION__));
		return FALSE;
	}

	assert(sna->present.unflip == 0);

	if (sna->flags & SNA_TEAR_FREE) {
		DBG(("%s: disabling TearFree (was %s) in favour of Present flips\n",
		     __FUNCTION__, sna->mode.shadow_enabled ? "enabled" : "disabled"));
		sna->mode.shadow_enabled = false;
	}
	assert(!sna->mode.shadow_enabled);

	if (sna->mode.flip_active) {
		struct pollfd pfd;

		DBG(("%s: flips still pending, stalling\n", __FUNCTION__));
		pfd.fd = sna->kgem.fd;
		pfd.events = POLLIN;
		while (poll(&pfd, 1, 0) == 1)
			sna_mode_wakeup(sna);
		if (sna->mode.flip_active)
			return FALSE;
	}

	bo = get_flip_bo(pixmap);
	if (bo == NULL) {
		DBG(("%s: flip invalid bo\n", __FUNCTION__));
		return FALSE;
	}

	if (sync_flip)
		return flip(sna, crtc, event_id, target_msc, bo);
	else
		return flip__async(sna, crtc, event_id, target_msc, bo);
}

static void
sna_present_unflip(ScreenPtr screen, uint64_t event_id)
{
	struct sna *sna = to_sna_from_screen(screen);
	struct kgem_bo *bo;

	DBG(("%s(event=%lld)\n", __FUNCTION__, (long long)event_id));
	if (sna->mode.front_active == 0 || sna->mode.rr_active) {
		const struct ust_msc *swap;

		DBG(("%s: no CRTC active, perform no-op flip\n", __FUNCTION__));

notify:
		swap = sna_crtc_last_swap(sna_primary_crtc(sna));
		DBG(("%s: pipe=%d, tv=%d.%06d msc=%lld, event=%lld complete\n", __FUNCTION__,
		     -1,
		     swap->tv_sec, swap->tv_usec, (long long)swap->msc,
		     (long long)event_id));
		present_event_notify(event_id, swap_ust(swap), swap->msc);
		return;
	}

	if (sna->mode.flip_active) {
		DBG(("%s: %d outstanding flips, queueing unflip\n", __FUNCTION__, sna->mode.flip_active));
		assert(sna->present.unflip == 0);
		sna->present.unflip = event_id;
		return;
	}

	bo = get_flip_bo(screen->GetScreenPixmap(screen));

	/* Are we unflipping after a failure that left our ScreenP in place? */
	if (!sna_needs_page_flip(sna, bo))
		goto notify;

	assert(!sna->mode.shadow_enabled);
	if (sna->flags & SNA_TEAR_FREE) {
		DBG(("%s: %s TearFree after Present flips\n",
		     __FUNCTION__, sna->mode.shadow_damage != NULL ? "enabling" : "disabling"));
		sna->mode.shadow_enabled = sna->mode.shadow_damage != NULL;
	}

	if (bo == NULL) {
reset_mode:
		DBG(("%s: failed, trying to restore original mode\n", __FUNCTION__));
		xf86SetDesiredModes(sna->scrn);
		goto notify;
	}

	assert(sna_pixmap(screen->GetScreenPixmap(screen))->pinned & PIN_SCANOUT);

	if (sna->flags & SNA_HAS_ASYNC_FLIP) {
		DBG(("%s: trying async flip restore\n", __FUNCTION__));
		if (flip__async(sna, NULL, event_id, 0, bo))
			return;
	}

	if (!flip(sna, NULL, event_id, 0, bo))
		goto reset_mode;
}

void sna_present_cancel_flip(struct sna *sna)
{
	if (sna->present.unflip) {
		const struct ust_msc *swap;

		swap = sna_crtc_last_swap(sna_primary_crtc(sna));
		present_event_notify(sna->present.unflip,
				     swap_ust(swap), swap->msc);

		sna->present.unflip = 0;
	}
}

static present_screen_info_rec present_info = {
	.version = PRESENT_SCREEN_INFO_VERSION,

	.get_crtc = sna_present_get_crtc,
	.get_ust_msc = sna_present_get_ust_msc,
	.queue_vblank = sna_present_queue_vblank,
	.abort_vblank = sna_present_abort_vblank,
	.flush = sna_present_flush,

	.capabilities = PresentCapabilityNone,
	.check_flip = sna_present_check_flip,
	.flip = sna_present_flip,
	.unflip = sna_present_unflip,
};

bool sna_present_open(struct sna *sna, ScreenPtr screen)
{
	DBG(("%s(num_crtc=%d)\n", __FUNCTION__, sna->mode.num_real_crtc));

	if (sna->mode.num_real_crtc == 0)
		return false;

	sna_present_update(sna);
	list_init(&sna->present.vblank_queue);

	return present_screen_init(screen, &present_info);
}

void sna_present_update(struct sna *sna)
{
	if (sna->flags & SNA_HAS_ASYNC_FLIP)
		present_info.capabilities |= PresentCapabilityAsync;
	else
		present_info.capabilities &= ~PresentCapabilityAsync;

	DBG(("%s: has_async_flip? %d\n", __FUNCTION__,
	     !!(present_info.capabilities & PresentCapabilityAsync)));
}

void sna_present_close(struct sna *sna, ScreenPtr screen)
{
	DBG(("%s()\n", __FUNCTION__));
}
