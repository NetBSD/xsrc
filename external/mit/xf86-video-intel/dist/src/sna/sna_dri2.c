/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.
Copyright © 2002 by David Dawes

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors: Jeff Hartmann <jhartmann@valinux.com>
 *          David Dawes <dawes@xfree86.org>
 *          Keith Whitwell <keith@tungstengraphics.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include "sna.h"
#include "intel_options.h"

#include <xf86drm.h>
#include <i915_drm.h>
#include <dri2.h>
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,12,99,901,0) && defined(COMPOSITE)
#include <compositeext.h>
#define CHECK_FOR_COMPOSITOR
#endif

#define DBG_CAN_FLIP 1
#define DBG_CAN_XCHG 1

#define DBG_FORCE_COPY -1 /* KGEM_BLT or KGEM_3D */

#if DRI2INFOREC_VERSION < 2
#error DRI2 version supported by the Xserver is too old
#endif

static inline struct kgem_bo *ref(struct kgem_bo *bo)
{
	return kgem_bo_reference(bo);
}

struct sna_dri2_private {
	PixmapPtr pixmap;
	struct kgem_bo *bo;
	DRI2Buffer2Ptr proxy;
	bool stale;
	uint32_t size;
	int refcnt;
};

static inline struct sna_dri2_private *
get_private(void *buffer)
{
	return (struct sna_dri2_private *)((DRI2Buffer2Ptr)buffer+1);
}

pure static inline DRI2BufferPtr sna_pixmap_get_buffer(PixmapPtr pixmap)
{
	assert(pixmap->refcnt);
	return ((void **)__get_private(pixmap, sna_pixmap_key))[2];
}

static inline void sna_pixmap_set_buffer(PixmapPtr pixmap, void *ptr)
{
	assert(pixmap->refcnt);
	((void **)__get_private(pixmap, sna_pixmap_key))[2] = ptr;
}

#if DRI2INFOREC_VERSION >= 4
enum event_type {
	WAITMSC = 0,
	SWAP,
	SWAP_COMPLETE,
	FLIP,
	FLIP_THROTTLE,
	FLIP_COMPLETE,
	FLIP_ASYNC,
};

struct dri_bo {
	struct list link;
	struct kgem_bo *bo;
	uint32_t name;
	unsigned flags;
};

struct sna_dri2_event {
	struct sna *sna;
	DrawablePtr draw;
	ClientPtr client;
	enum event_type type;
	xf86CrtcPtr crtc;
	int pipe;
	bool queued;
	bool sync;
	bool chained;

	/* for swaps & flips only */
	DRI2SwapEventPtr event_complete;
	void *event_data;
	DRI2BufferPtr front;
	DRI2BufferPtr back;
	struct kgem_bo *bo;

	struct copy {
		struct kgem_bo *bo;
		unsigned flags;
		uint32_t name;
		uint32_t size;
	} pending;

	struct sna_dri2_event *chain;

	struct list link;

	int flip_continue;
	int keepalive;
	int signal;
};

#if DRI2INFOREC_VERSION < 10
#undef USE_ASYNC_SWAP
#endif

#if USE_ASYNC_SWAP
#define KEEPALIVE 8 /* wait ~100ms before discarding swap caches */
#define APPLY_DAMAGE 0
#else
#define USE_ASYNC_SWAP 0
#define KEEPALIVE 1
#define APPLY_DAMAGE 1
#endif

static void sna_dri2_flip_event(struct sna_dri2_event *flip);
inline static DRI2BufferPtr dri2_window_get_front(WindowPtr win);

static struct kgem_bo *
__sna_dri2_copy_region(struct sna *sna, DrawablePtr draw, RegionPtr region,
		      DRI2BufferPtr src, DRI2BufferPtr dst,
		      unsigned flags);

inline static void
__sna_dri2_copy_event(struct sna_dri2_event *info, unsigned flags)
{
	DBG(("%s: flags = %x\n", __FUNCTION__, flags));
	assert(info->front != info->back);
	info->bo = __sna_dri2_copy_region(info->sna, info->draw, NULL,
					  info->back, info->front,
					  flags);
	info->front->flags = info->back->flags;
}

static int front_pitch(DrawablePtr draw)
{
	DRI2BufferPtr buffer;

	buffer = NULL;
	if (draw->type != DRAWABLE_PIXMAP)
		buffer = dri2_window_get_front((WindowPtr)draw);
	if (buffer == NULL)
		buffer = sna_pixmap_get_buffer(get_drawable_pixmap(draw));

	return buffer ? buffer->pitch : 0;
}

struct dri2_window {
	DRI2BufferPtr front;
	struct sna_dri2_event *chain;
	xf86CrtcPtr crtc;
	int64_t msc_delta;
	struct list cache;
	uint32_t cache_size;
	bool cache_scanout;
};

static struct dri2_window *dri2_window(WindowPtr win)
{
	assert(win->drawable.type != DRAWABLE_PIXMAP);
	return ((void **)__get_private(win, sna_window_key))[1];
}

static bool use_scanout(struct sna *sna,
			DrawablePtr draw,
			struct dri2_window *priv)
{
	if (priv->front)
		return true;

	return (sna->flags & (SNA_LINEAR_FB | SNA_NO_WAIT | SNA_NO_FLIP)) == 0 &&
		draw->width  == sna->front->drawable.width &&
		draw->height == sna->front->drawable.height &&
		draw->bitsPerPixel == sna->front->drawable.bitsPerPixel;
}

static void
sna_dri2_get_back(struct sna *sna,
		  DrawablePtr draw,
		  DRI2BufferPtr back)
{
	struct dri2_window *priv = dri2_window((WindowPtr)draw);
	uint32_t size;
	struct kgem_bo *bo;
	struct dri_bo *c;
	uint32_t name;
	bool scanout;
	bool reuse;
	int flags;

	DBG(("%s: draw size=%dx%d, back buffer handle=%d size=%dx%d, is-scanout? %d, active?=%d, pitch=%d, front pitch=%d\n",
	     __FUNCTION__, draw->width, draw->height,
	     get_private(back)->bo->handle,
	     get_private(back)->size & 0xffff, get_private(back)->size >> 16,
	     get_private(back)->bo->scanout,
	     get_private(back)->bo->active_scanout,
	     back->pitch, front_pitch(draw)));
	assert(priv);

	scanout = use_scanout(sna, draw, priv);
	size = draw->height << 16 | draw->width;
	if (size != priv->cache_size || scanout != priv->cache_scanout) {
		while (!list_is_empty(&priv->cache)) {
			c = list_first_entry(&priv->cache, struct dri_bo, link);
			list_del(&c->link);

			DBG(("%s: releasing cached handle=%d\n", __FUNCTION__, c->bo ? c->bo->handle : 0));
			assert(c->bo);
			kgem_bo_destroy(&sna->kgem, c->bo);

			free(c);
		}
		priv->cache_size = size;
		priv->cache_scanout = scanout;
	}

	reuse = size == get_private(back)->size;
	if (reuse)
		reuse = get_private(back)->bo->scanout >= scanout;
	DBG(("%s: reuse backbuffer? %d\n", __FUNCTION__, reuse));
	if (reuse) {
		bo = get_private(back)->bo;
		assert(bo->refcnt);
		DBG(("%s: back buffer handle=%d, active?=%d, refcnt=%d\n",
		     __FUNCTION__, bo->handle, bo->active_scanout, get_private(back)->refcnt));
		if (bo->active_scanout == 0) {
			DBG(("%s: reuse unattached back\n", __FUNCTION__));
			get_private(back)->stale = false;
			return;
		}
	}

	bo = NULL;
	list_for_each_entry(c, &priv->cache, link) {
		DBG(("%s: cache: handle=%d, active=%d\n",
		     __FUNCTION__, c->bo ? c->bo->handle : 0, c->bo ? c->bo->active_scanout : -1));
		assert(c->bo);
		if (c->bo->active_scanout == 0) {
			_list_del(&c->link);
			if (c->bo == NULL) {
				free(c);
				goto out;
			}
			bo = c->bo;
			name = c->name;
			flags = c->flags;
			DBG(("%s: reuse cache handle=%d, name=%d, flags=%d\n", __FUNCTION__, bo->handle, name, flags));
			c->bo = NULL;
			break;
		}
	}
	if (bo == NULL) {
		DBG(("%s: allocating new backbuffer\n", __FUNCTION__));
		flags = CREATE_EXACT;

		if (get_private(back)->bo->scanout &&
		    use_scanout(sna, draw, priv)) {
			DBG(("%s: requesting scanout compatible back\n", __FUNCTION__));
			flags |= CREATE_SCANOUT;
		}

		bo = kgem_create_2d(&sna->kgem,
				    draw->width, draw->height, draw->bitsPerPixel,
				    get_private(back)->bo->tiling,
				    flags);
		if (bo == NULL)
			return;

		name = kgem_bo_flink(&sna->kgem, bo);
		if (name == 0) {
			kgem_bo_destroy(&sna->kgem, bo);
			return;
		}

		flags = 0;
		if (USE_ASYNC_SWAP && back->flags) {
			BoxRec box;

			box.x1 = 0;
			box.y1 = 0;
			box.x2 = draw->width;
			box.y2 = draw->height;

			DBG(("%s: filling new buffer with old back\n", __FUNCTION__));
			if (sna->render.copy_boxes(sna, GXcopy,
						   draw, get_private(back)->bo, 0, 0,
						   draw, bo, 0, 0,
						   &box, 1, COPY_LAST | COPY_DRI))
				flags = back->flags;
		}
	}
	assert(bo->active_scanout == 0);

	if (reuse && get_private(back)->bo->refcnt == 1 + get_private(back)->bo->active_scanout) {
		if (&c->link == &priv->cache)
			c = malloc(sizeof(*c));
		if (c != NULL) {
			c->bo = ref(get_private(back)->bo);
			c->name = back->name;
			c->flags = back->flags;
			list_add(&c->link, &priv->cache);
			DBG(("%s: caching handle=%d (name=%d, flags=%d, active_scanout=%d)\n", __FUNCTION__, c->bo->handle, c->name, c->flags, c->bo->active_scanout));
		}
	} else {
		if (&c->link != &priv->cache)
			free(c);
	}

	assert(bo->active_scanout == 0);
	assert(bo != get_private(back)->bo);
	kgem_bo_destroy(&sna->kgem, get_private(back)->bo);

	get_private(back)->bo = bo;
	get_private(back)->size = draw->height << 16 | draw->width;
	back->pitch = bo->pitch;
	back->name = name;
	back->flags = flags;

	assert(back->pitch);
	assert(back->name);

out:
	get_private(back)->stale = false;
}

static struct sna_dri2_event *
dri2_chain(DrawablePtr d)
{
	struct dri2_window *priv = dri2_window((WindowPtr)d);
	assert(priv != NULL);
	assert(priv->chain == NULL || priv->chain->chained);
	return priv->chain;
}
inline static DRI2BufferPtr dri2_window_get_front(WindowPtr win)
{
	struct dri2_window *priv = dri2_window(win);
	assert(priv->front == NULL || get_private(priv->front)->bo->active_scanout);
	return priv ? priv->front : NULL;
}
#else
inline static void *dri2_window_get_front(WindowPtr win) { return NULL; }
#define APPLY_DAMAGE 1
#endif

#if DRI2INFOREC_VERSION < 6

#define xorg_can_triple_buffer() 0
#define swap_limit(d, l) false
#define mark_stale(b)

#else

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,15,99,904,0)
/* Prime fixed for triple buffer support */
#define xorg_can_triple_buffer() 1
#elif XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,12,99,901,0)
/* Before numGPUScreens was introduced */
#define xorg_can_triple_buffer() 1
#else
/* Subject to crashers when combining triple buffering and Prime */
inline static bool xorg_can_triple_buffer(void)
{
	return screenInfo.numGPUScreens == 0;
}
#endif

static void
mark_stale(DRI2BufferPtr back)
{
	/* If we have reuse notifications, we can track when the
	 * client tries to present an old buffer (one that has not
	 * been updated since the last swap) and avoid showing the
	 * stale frame. (This is mostly useful for tracking down
	 * driver bugs!)
	 */
	DBG(("%s(handle=%d) => %d\n", __FUNCTION__,
	     get_private(back)->bo->handle, xorg_can_triple_buffer()));
	get_private(back)->stale = xorg_can_triple_buffer();
}

static Bool
sna_dri2_swap_limit_validate(DrawablePtr draw, int swap_limit)
{
	DBG(("%s: swap limit set to %d\n", __FUNCTION__, swap_limit));
	return swap_limit >= 1;
}

static void
sna_dri2_reuse_buffer(DrawablePtr draw, DRI2BufferPtr buffer)
{
	struct sna *sna = to_sna_from_drawable(draw);

	DBG(("%s: reusing buffer pixmap=%ld, attachment=%d, handle=%d, name=%d\n",
	     __FUNCTION__, get_drawable_pixmap(draw)->drawable.serialNumber,
	     buffer->attachment, get_private(buffer)->bo->handle, buffer->name));
	assert(get_private(buffer)->refcnt);
	assert(get_private(buffer)->bo->refcnt >= get_private(buffer)->bo->active_scanout);
	assert(kgem_bo_flink(&sna->kgem, get_private(buffer)->bo) == buffer->name);

	if (buffer->attachment == DRI2BufferBackLeft &&
	    draw->type != DRAWABLE_PIXMAP) {
		DBG(("%s: replacing back buffer on window %ld\n", __FUNCTION__, draw->id));
		sna_dri2_get_back(sna, draw, buffer);

		assert(get_private(buffer)->bo->refcnt);
		assert(get_private(buffer)->bo->active_scanout == 0);
		assert(kgem_bo_flink(&sna->kgem, get_private(buffer)->bo) == buffer->name);
		DBG(("%s: reusing back buffer handle=%d, name=%d, pitch=%d, age=%d\n",
		     __FUNCTION__, get_private(buffer)->bo->handle,
		     buffer->name, buffer->pitch, buffer->flags));
	}

	kgem_bo_submit(&sna->kgem, get_private(buffer)->bo);
}

static bool swap_limit(DrawablePtr draw, int limit)
{
	if (!xorg_can_triple_buffer())
		return false;

	DBG(("%s: draw=%ld setting swap limit to %d\n", __FUNCTION__, (long)draw->id, limit));
	DRI2SwapLimit(draw, limit);
	return true;
}
#endif

#define COLOR_PREFER_TILING_Y 0

/* Prefer to enable TILING_Y if this buffer will never be a
 * candidate for pageflipping
 */
static uint32_t color_tiling(struct sna *sna, DrawablePtr draw)
{
	uint32_t tiling;

	if (!sna->kgem.can_fence)
		return I915_TILING_NONE;

	if (COLOR_PREFER_TILING_Y &&
	    (draw->width  != sna->front->drawable.width ||
	     draw->height != sna->front->drawable.height))
		tiling = I915_TILING_Y;
	else
		tiling = I915_TILING_X;

	return kgem_choose_tiling(&sna->kgem, -tiling,
				  draw->width,
				  draw->height,
				  draw->bitsPerPixel);
}

static uint32_t other_tiling(struct sna *sna, DrawablePtr draw)
{
	/* XXX Can mix color X / depth Y? */
	return kgem_choose_tiling(&sna->kgem,
				  sna->kgem.gen >= 040 ? -I915_TILING_Y : -I915_TILING_X,
				  draw->width,
				  draw->height,
				  draw->bitsPerPixel);
}

static struct kgem_bo *sna_pixmap_set_dri(struct sna *sna,
					  PixmapPtr pixmap)
{
	struct sna_pixmap *priv;

	DBG(("%s: attaching DRI client to pixmap=%ld\n",
	     __FUNCTION__, pixmap->drawable.serialNumber));

	priv = sna_pixmap(pixmap);
	if (priv != NULL && IS_STATIC_PTR(priv->ptr) && priv->cpu_bo) {
		DBG(("%s: SHM or unattached Pixmap, BadAlloc\n", __FUNCTION__));
		return NULL;
	}

	priv = sna_pixmap_move_to_gpu(pixmap,
				      MOVE_READ | __MOVE_FORCE | __MOVE_DRI);
	if (priv == NULL) {
		DBG(("%s: failed to move to GPU, BadAlloc\n", __FUNCTION__));
		return NULL;
	}

	assert(priv->flush == false || priv->pinned & PIN_DRI3);
	assert(priv->gpu_bo->flush == false || priv->pinned & PIN_DRI3);
	assert(priv->cpu_damage == NULL);
	assert(priv->gpu_bo);
	assert(priv->gpu_bo->proxy == NULL);

	if (!kgem_bo_is_fenced(&sna->kgem, priv->gpu_bo)) {
		if (priv->gpu_bo->tiling &&
		    !sna_pixmap_change_tiling(pixmap, I915_TILING_NONE)) {
			DBG(("%s: failed to discard tiling (%d) for DRI2 protocol\n", __FUNCTION__, priv->gpu_bo->tiling));
			return NULL;
		}
	} else {
		int tiling = color_tiling(sna, &pixmap->drawable);
		if (tiling < 0)
			tiling = -tiling;
		if (priv->gpu_bo->tiling < tiling && !priv->gpu_bo->scanout)
			sna_pixmap_change_tiling(pixmap, tiling);
	}

	priv->gpu_bo->active_scanout++;

	return priv->gpu_bo;
}

void
sna_dri2_pixmap_update_bo(struct sna *sna, PixmapPtr pixmap, struct kgem_bo *bo)
{
	DRI2BufferPtr buffer;
	struct sna_dri2_private *private;

	buffer = sna_pixmap_get_buffer(pixmap);
	if (buffer == NULL)
		return;

	DBG(("%s: pixmap=%ld, old handle=%d, new handle=%d\n", __FUNCTION__,
	     pixmap->drawable.serialNumber,
	     get_private(buffer)->bo->handle,
	     sna_pixmap(pixmap)->gpu_bo->handle));

	private = get_private(buffer);
	assert(private->pixmap == pixmap);

	assert(bo != private->bo);
	if (private->bo == bo)
		return;

	assert(private->bo->active_scanout > 0);
	private->bo->active_scanout--;

	DBG(("%s: dropping flush hint from handle=%d\n", __FUNCTION__, private->bo->handle));
	private->bo->flush = false;
	kgem_bo_destroy(&sna->kgem, private->bo);


	buffer->name = kgem_bo_flink(&sna->kgem, bo);
	buffer->pitch = bo->pitch;
	private->bo = ref(bo);
	bo->active_scanout++;

	DBG(("%s: adding flush hint to handle=%d\n", __FUNCTION__, bo->handle));
	bo->flush = true;
	if (bo->exec)
		sna->kgem.flush = 1;
	assert(sna_pixmap(pixmap)->flush);

	/* XXX DRI2InvalidateDrawable(&pixmap->drawable); */
}

static DRI2Buffer2Ptr
sna_dri2_create_buffer(DrawablePtr draw,
		       unsigned int attachment,
		       unsigned int format)
{
	struct sna *sna = to_sna_from_drawable(draw);
	DRI2Buffer2Ptr buffer;
	struct sna_dri2_private *private;
	PixmapPtr pixmap;
	struct kgem_bo *bo;
	unsigned bpp = format ? BitsPerPixel(format) : draw->bitsPerPixel;
	unsigned flags = CREATE_EXACT;
	uint32_t size;

	DBG(("%s pixmap=%ld, (attachment=%d, format=%d, drawable=%dx%d), window?=%d\n",
	     __FUNCTION__,
	     get_drawable_pixmap(draw)->drawable.serialNumber,
	     attachment, format, draw->width, draw->height,
	     draw->type != DRAWABLE_PIXMAP));

	pixmap = NULL;
	size = (uint32_t)draw->height << 16 | draw->width;
	switch (attachment) {
	case DRI2BufferFrontLeft:
		sna->needs_dri_flush = true;

		pixmap = get_drawable_pixmap(draw);
		buffer = NULL;
		if (draw->type != DRAWABLE_PIXMAP)
			buffer = dri2_window_get_front((WindowPtr)draw);
		if (buffer == NULL)
			buffer = (DRI2Buffer2Ptr)sna_pixmap_get_buffer(pixmap);
		if (buffer) {
			private = get_private(buffer);

			DBG(("%s: reusing front buffer attachment, win=%lu %dx%d, pixmap=%ld [%ld] %dx%d, handle=%d, name=%d, active_scanout=%d\n",
			     __FUNCTION__,
			     draw->type != DRAWABLE_PIXMAP ? (long)draw->id : (long)0,
			     draw->width, draw->height,
			     pixmap->drawable.serialNumber,
			     private->pixmap->drawable.serialNumber,
			     pixmap->drawable.width,
			     pixmap->drawable.height,
			     private->bo->handle, buffer->name,
			     private->bo->active_scanout));

			assert(buffer->attachment == DRI2BufferFrontLeft);
			assert(private->pixmap == pixmap);
			assert(sna_pixmap(pixmap)->flush);
			assert(sna_pixmap(pixmap)->pinned & PIN_DRI2);
			assert(kgem_bo_flink(&sna->kgem, private->bo) == buffer->name);
			assert(private->bo->pitch == buffer->pitch);
			assert(private->bo->active_scanout);

			private->refcnt++;
			return buffer;
		}

		bo = sna_pixmap_set_dri(sna, pixmap);
		if (bo == NULL)
			return NULL;

		assert(sna_pixmap(pixmap) != NULL);

		bo = ref(bo);
		if (pixmap == sna->front && !(sna->flags & SNA_LINEAR_FB))
			flags |= CREATE_SCANOUT;
		DBG(("%s: attaching to front buffer %dx%d [%p:%d], scanout? %d\n",
		     __FUNCTION__,
		     pixmap->drawable.width, pixmap->drawable.height,
		     pixmap, pixmap->refcnt, flags & CREATE_SCANOUT));
		size = (uint32_t)pixmap->drawable.height << 16 | pixmap->drawable.width;
		bpp = pixmap->drawable.bitsPerPixel;
		break;

	case DRI2BufferBackLeft:
		if (draw->type != DRAWABLE_PIXMAP) {
			if (dri2_window_get_front((WindowPtr)draw))
				flags |= CREATE_SCANOUT;
			if (draw->width  == sna->front->drawable.width &&
			    draw->height == sna->front->drawable.height &&
			    draw->bitsPerPixel == bpp &&
			    (sna->flags & (SNA_LINEAR_FB | SNA_NO_WAIT | SNA_NO_FLIP)) == 0)
				flags |= CREATE_SCANOUT;
		}
		/* fall through */
	case DRI2BufferBackRight:
	case DRI2BufferFrontRight:
	case DRI2BufferFakeFrontLeft:
	case DRI2BufferFakeFrontRight:
		DBG(("%s: creating back buffer %dx%d, suitable for scanout? %d\n",
		     __FUNCTION__,
		     draw->width, draw->height,
		     flags & CREATE_SCANOUT));

		bo = kgem_create_2d(&sna->kgem,
				    draw->width,
				    draw->height,
				    bpp,
				    color_tiling(sna, draw),
				    flags);
		break;

	case DRI2BufferStencil:
		/*
		 * The stencil buffer has quirky pitch requirements.  From Vol
		 * 2a, 11.5.6.2.1 3DSTATE_STENCIL_BUFFER, field "Surface
		 * Pitch":
		 *    The pitch must be set to 2x the value computed based on
		 *    width, as the stencil buffer is stored with two rows
		 *    interleaved.
		 * To accomplish this, we resort to the nasty hack of doubling
		 * the drm region's cpp and halving its height.
		 *
		 * If we neglect to double the pitch, then
		 * drm_intel_gem_bo_map_gtt() maps the memory incorrectly.
		 *
		 * The alignment for W-tiling is quite different to the
		 * nominal no-tiling case, so we have to account for
		 * the tiled access pattern explicitly.
		 *
		 * The stencil buffer is W tiled. However, we request from
		 * the kernel a non-tiled buffer because the kernel does
		 * not understand W tiling and the GTT is incapable of
		 * W fencing.
		 */
		bpp *= 2;
		bo = kgem_create_2d(&sna->kgem,
				    ALIGN(draw->width, 64),
				    ALIGN((draw->height + 1) / 2, 64),
				    bpp, I915_TILING_NONE, flags);
		break;

	case DRI2BufferDepth:
	case DRI2BufferDepthStencil:
	case DRI2BufferHiz:
	case DRI2BufferAccum:
		bo = kgem_create_2d(&sna->kgem,
				    draw->width, draw->height, bpp,
				    other_tiling(sna, draw),
				    flags);
		break;

	default:
		return NULL;
	}
	if (bo == NULL)
		return NULL;

	buffer = calloc(1, sizeof *buffer + sizeof *private);
	if (buffer == NULL)
		goto err;

	private = get_private(buffer);
	buffer->attachment = attachment;
	buffer->pitch = bo->pitch;
	buffer->cpp = bpp / 8;
	buffer->driverPrivate = private;
	buffer->format = format;
	buffer->flags = 0;
	buffer->name = kgem_bo_flink(&sna->kgem, bo);
	private->refcnt = 1;
	private->bo = bo;
	private->pixmap = pixmap;
	private->size = size;

	if (buffer->name == 0)
		goto err;

	if (pixmap) {
		struct sna_pixmap *priv;

		assert(attachment == DRI2BufferFrontLeft);
		assert(sna_pixmap_get_buffer(pixmap) == NULL);

		sna_pixmap_set_buffer(pixmap, buffer);
		assert(sna_pixmap_get_buffer(pixmap) == buffer);
		pixmap->refcnt++;

		priv = sna_pixmap(pixmap);
		assert(priv->flush == false || priv->pinned & PIN_DRI3);
		assert((priv->pinned & PIN_DRI2) == 0);

		/* Don't allow this named buffer to be replaced */
		priv->pinned |= PIN_DRI2;

		/* We need to submit any modifications to and reads from this
		 * buffer before we send any reply to the Client.
		 *
		 * As we don't track which Client, we flush for all.
		 */
		DBG(("%s: adding flush hint to handle=%d\n", __FUNCTION__, priv->gpu_bo->handle));
		priv->gpu_bo->flush = true;
		if (priv->gpu_bo->exec)
			sna->kgem.flush = 1;

		priv->flush |= FLUSH_READ;
		if (draw->type == DRAWABLE_PIXMAP) {
			/* DRI2 renders directly into GLXPixmaps, treat as hostile */
			kgem_bo_unclean(&sna->kgem, priv->gpu_bo);
			sna_damage_all(&priv->gpu_damage, pixmap);
			priv->clear = false;
			priv->cpu = false;
			priv->flush |= FLUSH_WRITE;
		}

		sna_watch_flush(sna, 1);
	}

	return buffer;

err:
	kgem_bo_destroy(&sna->kgem, bo);
	free(buffer);
	return NULL;
}

static void
sna_dri2_cache_bo(struct sna *sna,
		  DrawablePtr draw,
		  struct kgem_bo *bo,
		  uint32_t name,
		  uint32_t size,
		  uint32_t flags)
{
	struct dri_bo *c;

	DBG(("%s(handle=%d, name=%d)\n", __FUNCTION__, bo->handle, name));

	if (draw == NULL) {
		DBG(("%s: no draw, releasing handle=%d\n",
		     __FUNCTION__, bo->handle));
		goto err;
	}

	if (draw->type == DRAWABLE_PIXMAP) {
		DBG(("%s: not a window, releasing handle=%d\n",
		     __FUNCTION__, bo->handle));
		goto err;
	}

	if (bo->refcnt > 1 + bo->active_scanout) {
		DBG(("%s: multiple references [%d], releasing handle\n",
		     __FUNCTION__, bo->refcnt, bo->handle));
		goto err;
	}

	if ((draw->height << 16 | draw->width) != size) {
		DBG(("%s: wrong size [%dx%d], releasing handle\n",
		     __FUNCTION__,
		     size & 0xffff, size >> 16,
		     bo->handle));
		goto err;
	}

	if (bo->scanout && front_pitch(draw) != bo->pitch) {
		DBG(("%s: scanout with pitch change [%d != %d], releasing handle\n",
		     __FUNCTION__, bo->pitch, front_pitch(draw), bo->handle));
		goto err;
	}

	c = malloc(sizeof(*c));
	if (!c)
		goto err;

	DBG(("%s: caching handle=%d (name=%d, flags=%d, active_scanout=%d)\n", __FUNCTION__, bo->handle, name, flags, bo->active_scanout));

	c->bo = bo;
	c->name = name;
	c->flags = flags;
	list_add(&c->link, &dri2_window((WindowPtr)draw)->cache);
	return;

err:
	kgem_bo_destroy(&sna->kgem, bo);
}

static void _sna_dri2_destroy_buffer(struct sna *sna,
				     DrawablePtr draw,
				     DRI2Buffer2Ptr buffer)
{
	struct sna_dri2_private *private = get_private(buffer);

	if (buffer == NULL)
		return;

	DBG(("%s: %p [handle=%d] -- refcnt=%d, draw=%ld, pixmap=%ld, proxy?=%d\n",
	     __FUNCTION__, buffer, private->bo->handle, private->refcnt,
	     draw ? draw->id : 0,
	     private->pixmap ? private->pixmap->drawable.serialNumber : 0,
	     private->proxy != NULL));
	assert(private->refcnt > 0);
	if (--private->refcnt)
		return;

	assert(private->bo);

	if (private->proxy) {
		DBG(("%s: destroying proxy\n", __FUNCTION__));
		assert(private->bo->active_scanout > 0);
		private->bo->active_scanout--;

		_sna_dri2_destroy_buffer(sna, draw, private->proxy);
		private->pixmap = NULL;
	}

	if (private->pixmap) {
		PixmapPtr pixmap = private->pixmap;
		struct sna_pixmap *priv = sna_pixmap(pixmap);

		assert(sna_pixmap_get_buffer(pixmap) == buffer);
		assert(priv->gpu_bo == private->bo);
		assert(priv->gpu_bo->flush);
		assert(priv->pinned & PIN_DRI2);
		assert(priv->flush);

		DBG(("%s: removing active_scanout=%d from pixmap handle=%d\n",
		     __FUNCTION__, priv->gpu_bo->active_scanout, priv->gpu_bo->handle));
		assert(priv->gpu_bo->active_scanout > 0);
		priv->gpu_bo->active_scanout--;

		/* Undo the DRI markings on this pixmap */
		DBG(("%s: releasing last DRI pixmap=%ld, scanout?=%d\n",
		     __FUNCTION__,
		     pixmap->drawable.serialNumber,
		     pixmap == sna->front));

		list_del(&priv->flush_list);

		DBG(("%s: dropping flush hint from handle=%d\n", __FUNCTION__, private->bo->handle));
		priv->pinned &= ~PIN_DRI2;

		if ((priv->pinned & PIN_DRI3) == 0) {
			priv->gpu_bo->flush = false;
			priv->flush = false;
		}
		sna_watch_flush(sna, -1);

		sna_pixmap_set_buffer(pixmap, NULL);
		pixmap->drawable.pScreen->DestroyPixmap(pixmap);
	}

	sna_dri2_cache_bo(sna, draw,
			  private->bo,
			  buffer->name,
			  private->size,
			  buffer->flags);
	free(buffer);
}

static void sna_dri2_destroy_buffer(DrawablePtr draw, DRI2Buffer2Ptr buffer)
{
	_sna_dri2_destroy_buffer(to_sna_from_drawable(draw), draw, buffer);
}

static DRI2BufferPtr sna_dri2_reference_buffer(DRI2BufferPtr buffer)
{
	assert(get_private(buffer)->refcnt > 0);
	get_private(buffer)->refcnt++;
	return buffer;
}

static inline void damage(PixmapPtr pixmap, struct sna_pixmap *priv, RegionPtr region)
{
	assert(priv->gpu_bo);
	if (DAMAGE_IS_ALL(priv->gpu_damage))
		goto done;

	if (region == NULL) {
damage_all:
		priv->gpu_damage = _sna_damage_all(priv->gpu_damage,
						   pixmap->drawable.width,
						   pixmap->drawable.height);
		sna_damage_destroy(&priv->cpu_damage);
		list_del(&priv->flush_list);
	} else {
		sna_damage_subtract(&priv->cpu_damage, region);
		if (priv->cpu_damage == NULL)
			goto damage_all;
		sna_damage_add(&priv->gpu_damage, region);
	}
done:
	priv->cpu = false;
	priv->clear = false;
}

static void set_bo(PixmapPtr pixmap, struct kgem_bo *bo)
{
	struct sna *sna = to_sna_from_pixmap(pixmap);
	struct sna_pixmap *priv = sna_pixmap(pixmap);

	DBG(("%s: pixmap=%ld, handle=%d (old handle=%d)\n",
	     __FUNCTION__, pixmap->drawable.serialNumber, bo->handle, priv->gpu_bo->handle));

	assert(pixmap->drawable.width * pixmap->drawable.bitsPerPixel <= 8*bo->pitch);
	assert(pixmap->drawable.height * bo->pitch <= kgem_bo_size(bo));
	assert(bo->proxy == NULL);
	assert(priv->pinned & PIN_DRI2);
	assert((priv->pinned & (PIN_PRIME | PIN_DRI3)) == 0);
	assert(priv->flush);

	if (APPLY_DAMAGE) {
		RegionRec region;

		/* Post damage on the new front buffer so that listeners, such
		 * as DisplayLink know take a copy and shove it over the USB,
		 * also for software cursors and the like.
		 */
		region.extents.x1 = region.extents.y1 = 0;
		region.extents.x2 = pixmap->drawable.width;
		region.extents.y2 = pixmap->drawable.height;
		region.data = NULL;

		/*
		 * Eeek, beware the sw cursor copying to the old bo
		 * causing recursion and mayhem.
		 */
		DBG(("%s: marking whole pixmap as damaged\n", __FUNCTION__));
		sna->ignore_copy_area = sna->flags & SNA_TEAR_FREE;
		DamageRegionAppend(&pixmap->drawable, &region);
	}

	damage(pixmap, priv, NULL);

	assert(bo->refcnt);
	if (priv->move_to_gpu) {
		DBG(("%s: applying final/discard move-to-gpu\n", __FUNCTION__));
		priv->move_to_gpu(sna, priv, 0);
	}
	if (priv->gpu_bo != bo) {
		DBG(("%s: dropping flush hint from handle=%d\n", __FUNCTION__, priv->gpu_bo->handle));
		priv->gpu_bo->flush = false;
		if (priv->cow)
			sna_pixmap_undo_cow(sna, priv, 0);
		if (priv->gpu_bo) {
			sna_pixmap_unmap(pixmap, priv);
			kgem_bo_destroy(&sna->kgem, priv->gpu_bo);
		}
		DBG(("%s: adding flush hint to handle=%d\n", __FUNCTION__, bo->handle));
		bo->flush = true;
		if (bo->exec)
			sna->kgem.flush = 1;
		priv->gpu_bo = ref(bo);
	}
	if (bo->domain != DOMAIN_GPU)
		bo->domain = DOMAIN_NONE;
	assert(bo->flush);

	if (APPLY_DAMAGE) {
		sna->ignore_copy_area = false;
		DamageRegionProcessPending(&pixmap->drawable);
	}
}

#if defined(__GNUC__)
#define popcount(x) __builtin_popcount(x)
#else
static int popcount(unsigned int x)
{
	int count = 0;

	while (x) {
		count += x&1;
		x >>= 1;
	}

	return count;
}
#endif

static void sna_dri2_select_mode(struct sna *sna, struct kgem_bo *dst, struct kgem_bo *src, bool sync)
{
	struct drm_i915_gem_busy busy;
	int mode;

	if (sna->kgem.gen < 060)
		return;

	if (sync) {
		DBG(("%s: sync, force %s ring\n", __FUNCTION__,
		     sna->kgem.gen >= 070 ? "BLT" : "RENDER"));
		kgem_set_mode(&sna->kgem,
			      sna->kgem.gen >= 070 ? KGEM_BLT : KGEM_RENDER,
			      dst);
		return;
	}

	if (DBG_FORCE_COPY != -1) {
		DBG(("%s: forcing %d\n", __FUNCTION__, DBG_FORCE_COPY));
		kgem_set_mode(&sna->kgem, DBG_FORCE_COPY, dst);
		return;
	}

	if (sna->kgem.mode != KGEM_NONE) {
		DBG(("%s: busy, not switching\n", __FUNCTION__));
		return;
	}

	if (sna->render_state.gt < 2 && sna->kgem.has_semaphores) {
		DBG(("%s: small GT [%d], not forcing selection\n",
		     __FUNCTION__, sna->render_state.gt));
		return;
	}

	VG_CLEAR(busy);
	busy.handle = src->handle;
	if (drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GEM_BUSY, &busy))
		return;

	DBG(("%s: src handle=%d busy?=%x\n", __FUNCTION__, busy.handle, busy.busy));
	if (busy.busy == 0) {
		__kgem_bo_clear_busy(src);

		busy.handle = dst->handle;
		if (drmIoctl(sna->kgem.fd, DRM_IOCTL_I915_GEM_BUSY, &busy))
			return;

		DBG(("%s: dst handle=%d busy?=%x\n", __FUNCTION__, busy.handle, busy.busy));
		if (busy.busy == 0) {
			__kgem_bo_clear_busy(dst);
			DBG(("%s: src/dst is idle, using defaults\n", __FUNCTION__));
			return;
		}
	}

	/* Sandybridge introduced a separate ring which it uses to
	 * perform blits. Switching rendering between rings incurs
	 * a stall as we wait upon the old ring to finish and
	 * flush its render cache before we can proceed on with
	 * the operation on the new ring.
	 *
	 * As this buffer, we presume, has just been written to by
	 * the DRI client using the RENDER ring, we want to perform
	 * our operation on the same ring, and ideally on the same
	 * ring as we will flip from (which should be the RENDER ring
	 * as well).
	 *
	 * The ultimate question is whether preserving the ring outweighs
	 * the cost of the query.
	 */
	mode = KGEM_RENDER;
	if ((busy.busy & 0xffff) == I915_EXEC_BLT)
		mode = KGEM_BLT;
	kgem_bo_mark_busy(&sna->kgem,
			  busy.handle == src->handle ? src : dst,
			  mode);
	_kgem_set_mode(&sna->kgem, mode);
}

static bool is_front(int attachment)
{
	return attachment == DRI2BufferFrontLeft;
}

#define DRI2_SYNC 0x1
#define DRI2_DAMAGE 0x2
#define DRI2_BO 0x4
static struct kgem_bo *
__sna_dri2_copy_region(struct sna *sna, DrawablePtr draw, RegionPtr region,
		      DRI2BufferPtr src, DRI2BufferPtr dst,
		      unsigned flags)
{
	PixmapPtr pixmap = get_drawable_pixmap(draw);
	DrawableRec scratch, *src_draw = &pixmap->drawable, *dst_draw = &pixmap->drawable;
	struct sna_dri2_private *src_priv = get_private(src);
	struct sna_dri2_private *dst_priv = get_private(dst);
	pixman_region16_t clip;
	struct kgem_bo *bo = NULL;
	struct kgem_bo *src_bo;
	struct kgem_bo *dst_bo;
	const BoxRec *boxes;
	int16_t dx, dy, sx, sy;
	unsigned hint;
	int n;

	/* To hide a stale DRI2Buffer, one may choose to substitute
	 * pixmap->gpu_bo instead of dst/src->bo, however you then run
	 * the risk of copying around invalid data. So either you may not
	 * see the results of the copy, or you may see the wrong pixels.
	 * Either way you eventually lose.
	 *
	 * We also have to be careful in case that the stale buffers are
	 * now attached to invalid (non-DRI) pixmaps.
	 */

	assert(is_front(dst->attachment) || is_front(src->attachment));
	assert(dst->attachment != src->attachment);

	clip.extents.x1 = draw->x;
	clip.extents.y1 = draw->y;
	clip.extents.x2 = draw->x + draw->width;
	clip.extents.y2 = draw->y + draw->height;
	clip.data = NULL;

	if (region) {
		pixman_region_translate(region, draw->x, draw->y);
		pixman_region_intersect(&clip, &clip, region);
		region = &clip;
	}

	if (clip.extents.x1 >= clip.extents.x2 ||
	    clip.extents.y1 >= clip.extents.y2) {
		DBG(("%s: all clipped\n", __FUNCTION__));
		return NULL;
	}

	sx = sy = dx = dy = 0;
	if (is_front(dst->attachment)) {
		sx = -draw->x;
		sy = -draw->y;
	} else {
		dx = -draw->x;
		dy = -draw->y;
	}
	if (draw->type == DRAWABLE_WINDOW) {
		WindowPtr win = (WindowPtr)draw;
		int16_t tx, ty;

		if (is_clipped(&win->clipList, draw)) {
			DBG(("%s: draw=(%d, %d), delta=(%d, %d), draw=(%d, %d),(%d, %d), clip.extents=(%d, %d), (%d, %d)\n",
			     __FUNCTION__, draw->x, draw->y,
			     get_drawable_dx(draw), get_drawable_dy(draw),
			     clip.extents.x1, clip.extents.y1,
			     clip.extents.x2, clip.extents.y2,
			     win->clipList.extents.x1, win->clipList.extents.y1,
			     win->clipList.extents.x2, win->clipList.extents.y2));

			assert(region == NULL || region == &clip);
			pixman_region_intersect(&clip, &win->clipList, &clip);
			if (!pixman_region_not_empty(&clip)) {
				DBG(("%s: all clipped\n", __FUNCTION__));
				return NULL;
			}

			region = &clip;
		}

		if (get_drawable_deltas(draw, pixmap, &tx, &ty)) {
			if (is_front(dst->attachment)) {
				pixman_region_translate(region ?: &clip, tx, ty);
				sx -= tx;
				sy -= ty;
			} else {
				sx += tx;
				sy += ty;
			}
		}
	} else
		flags &= ~DRI2_SYNC;

	scratch.pScreen = draw->pScreen;
	scratch.x = scratch.y = 0;
	scratch.width = scratch.height = 0;
	scratch.depth = draw->depth;
	scratch.bitsPerPixel = draw->bitsPerPixel;

	src_bo = src_priv->bo;
	assert(src_bo->refcnt);
	kgem_bo_unclean(&sna->kgem, src_bo);
	if (is_front(src->attachment)) {
		struct sna_pixmap *priv;

		priv = sna_pixmap_move_to_gpu(pixmap, MOVE_READ);
		if (priv)
			src_bo = priv->gpu_bo;
		DBG(("%s: updated FrontLeft src_bo from handle=%d to handle=%d\n",
		     __FUNCTION__, src_priv->bo->handle, src_bo->handle));
		assert(src_bo->refcnt);
	} else {
		RegionRec source;

		scratch.width = src_priv->size & 0xffff;
		scratch.height = src_priv->size >> 16;
		src_draw = &scratch;

		DBG(("%s: source size %dx%d, region size %dx%d, src offset %dx%d\n",
		     __FUNCTION__,
		     scratch.width, scratch.height,
		     clip.extents.x2 - clip.extents.x1,
		     clip.extents.y2 - clip.extents.y1,
		     -sx, -sy));

		source.extents.x1 = -sx;
		source.extents.y1 = -sy;
		source.extents.x2 = source.extents.x1 + scratch.width;
		source.extents.y2 = source.extents.y1 + scratch.height;
		source.data = NULL;

		assert(region == NULL || region == &clip);
		pixman_region_intersect(&clip, &clip, &source);

		if (!pixman_region_not_empty(&clip)) {
			DBG(("%s: region doesn't overlap pixmap\n", __FUNCTION__));
			return NULL;
		}
	}

	dst_bo = dst_priv->bo;
	assert(dst_bo->refcnt);
	if (is_front(dst->attachment)) {
		struct sna_pixmap *priv;
		struct list shadow;

		/* Preserve the CRTC shadow overrides */
		sna_shadow_steal_crtcs(sna, &shadow);

		hint = MOVE_WRITE | __MOVE_FORCE;
		if (clip.data)
			hint |= MOVE_READ;

		assert(region == NULL || region == &clip);
		priv = sna_pixmap_move_area_to_gpu(pixmap, &clip.extents, hint);
		if (priv) {
			damage(pixmap, priv, region ?: &clip);
			dst_bo = priv->gpu_bo;
		}
		DBG(("%s: updated FrontLeft dst_bo from handle=%d to handle=%d\n",
		     __FUNCTION__, dst_priv->bo->handle, dst_bo->handle));
		assert(dst_bo->refcnt);

		sna_shadow_unsteal_crtcs(sna, &shadow);
	} else {
		RegionRec target;

		scratch.width = dst_priv->size & 0xffff;
		scratch.height = dst_priv->size >> 16;
		dst_draw = &scratch;

		DBG(("%s: target size %dx%d, region size %dx%d\n",
		     __FUNCTION__,
		     scratch.width, scratch.height,
		     clip.extents.x2 - clip.extents.x1,
		     clip.extents.y2 - clip.extents.y1));

		target.extents.x1 = -dx;
		target.extents.y1 = -dy;
		target.extents.x2 = target.extents.x1 + scratch.width;
		target.extents.y2 = target.extents.y1 + scratch.height;
		target.data = NULL;

		assert(region == NULL || region == &clip);
		pixman_region_intersect(&clip, &clip, &target);

		flags &= ~DRI2_SYNC;
	}

	if (!wedged(sna)) {
		xf86CrtcPtr crtc;

		crtc = NULL;
		if (flags & DRI2_SYNC && sna_pixmap_is_scanout(sna, pixmap))
			crtc = sna_covering_crtc(sna, &clip.extents, NULL);
		sna_dri2_select_mode(sna, dst_bo, src_bo, crtc != NULL);

		if (crtc == NULL ||
		    !sna_wait_for_scanline(sna, pixmap, crtc, &clip.extents))
			flags &= ~DRI2_SYNC;
	}

	if (region) {
		boxes = region_rects(region);
		n = region_num_rects(region);
		assert(n);
	} else {
		region = &clip;
		boxes = &clip.extents;
		n = 1;
	}
	if (APPLY_DAMAGE || flags & DRI2_DAMAGE) {
		DBG(("%s: marking region as damaged\n", __FUNCTION__));
		sna->ignore_copy_area = sna->flags & SNA_TEAR_FREE;
		DamageRegionAppend(&pixmap->drawable, region);
	}

	DBG(("%s: copying [(%d, %d), (%d, %d)]x%d src=(%d, %d), dst=(%d, %d)\n",
	     __FUNCTION__,
	     boxes[0].x1, boxes[0].y1,
	     boxes[0].x2, boxes[0].y2,
	     n, sx, sy, dx, dy));

	hint = COPY_LAST | COPY_DRI;
	if (flags & DRI2_SYNC)
		hint |= COPY_SYNC;
	if (!sna->render.copy_boxes(sna, GXcopy,
				    src_draw, src_bo, sx, sy,
				    dst_draw, dst_bo, dx, dy,
				    boxes, n, hint))
		memcpy_copy_boxes(sna, GXcopy,
				  src_draw, src_bo, sx, sy,
				  dst_draw, dst_bo, dx, dy,
				  boxes, n, hint);

	sna->needs_dri_flush = true;
	if (flags & (DRI2_SYNC | DRI2_BO)) { /* STAT! */
		struct kgem_request *rq = RQ(dst_bo->rq);
		if (rq && rq != (void *)&sna->kgem) {
			if (rq->bo == NULL)
				kgem_submit(&sna->kgem);
			if (rq->bo) { /* Becareful in case the gpu is wedged */
				bo = ref(rq->bo);
				DBG(("%s: recording sync fence handle=%d\n",
				     __FUNCTION__, bo->handle));
			}
		}
	}

	if (APPLY_DAMAGE || flags & DRI2_DAMAGE) {
		sna->ignore_copy_area = false;
		DamageRegionProcessPending(&pixmap->drawable);
	}

	if (clip.data)
		pixman_region_fini(&clip);

	return bo;
}

static void
sna_dri2_copy_region(DrawablePtr draw,
		     RegionPtr region,
		     DRI2BufferPtr dst,
		     DRI2BufferPtr src)
{
	PixmapPtr pixmap = get_drawable_pixmap(draw);
	struct sna *sna = to_sna_from_pixmap(pixmap);

	DBG(("%s: pixmap=%ld, src=%u (attach=%d, refs=%d/%d, flush=%d, stale=%d) , dst=%u (attach=%d, refs=%d/%d, flush=%d, stale=%d)\n",
	     __FUNCTION__,
	     pixmap->drawable.serialNumber,
	     get_private(src)->bo->handle,
	     src->attachment,
	     get_private(src)->refcnt,
	     get_private(src)->bo->refcnt,
	     get_private(src)->bo->flush,
	     get_private(src)->stale,
	     get_private(dst)->bo->handle,
	     dst->attachment,
	     get_private(dst)->refcnt,
	     get_private(dst)->bo->refcnt,
	     get_private(dst)->bo->flush,
	     get_private(dst)->stale));

	assert(src != dst);

	assert(get_private(src)->refcnt);
	assert(get_private(dst)->refcnt);

	if (get_private(src)->stale)
		return;

	assert(get_private(src)->bo != get_private(dst)->bo);

	assert(get_private(src)->bo->refcnt);
	assert(get_private(dst)->bo->refcnt);

	DBG(("%s: region (%d, %d), (%d, %d) x %d\n",
	     __FUNCTION__,
	     region->extents.x1, region->extents.y1,
	     region->extents.x2, region->extents.y2,
	     region_num_rects(region)));

	__sna_dri2_copy_region(sna, draw, region, src, dst, DRI2_DAMAGE);
}

inline static uint32_t pipe_select(int pipe)
{
	/* The third pipe was introduced with IvyBridge long after
	 * multiple pipe support was added to the kernel, hence
	 * we can safely ignore the capability check - if we have more
	 * than two pipes, we can assume that they are fully supported.
	 */
	assert(pipe < _DRM_VBLANK_HIGH_CRTC_MASK);
	if (pipe > 1)
		return pipe << DRM_VBLANK_HIGH_CRTC_SHIFT;
	else if (pipe > 0)
		return DRM_VBLANK_SECONDARY;
	else
		return 0;
}

static inline bool sna_next_vblank(struct sna_dri2_event *info)
{
	union drm_wait_vblank vbl;

	DBG(("%s(pipe=%d, waiting until next vblank)\n",
	     __FUNCTION__, info->pipe));
	assert(info->pipe != -1);

	VG_CLEAR(vbl);
	vbl.request.type =
		DRM_VBLANK_RELATIVE |
		DRM_VBLANK_EVENT |
		pipe_select(info->pipe);
	vbl.request.sequence = 1;
	vbl.request.signal = (uintptr_t)info;

	assert(!info->queued);
	if (drmIoctl(info->sna->kgem.fd, DRM_IOCTL_WAIT_VBLANK, &vbl))
		return false;

	info->queued = true;
	return true;
}

static inline bool sna_wait_vblank(struct sna_dri2_event *info,
				   unsigned seq)
{
	union drm_wait_vblank vbl;

	DBG(("%s(pipe=%d, waiting until vblank %u)\n",
	     __FUNCTION__, info->pipe, seq));
	assert(info->pipe != -1);

	VG_CLEAR(vbl);
	vbl.request.type =
		DRM_VBLANK_ABSOLUTE |
		DRM_VBLANK_EVENT |
		pipe_select(info->pipe);
	vbl.request.sequence = seq;
	vbl.request.signal = (uintptr_t)info;

	assert(!info->queued);
	if (drmIoctl(info->sna->kgem.fd, DRM_IOCTL_WAIT_VBLANK, &vbl))
		return false;

	info->queued = true;
	return true;
}

#if DRI2INFOREC_VERSION >= 4

static void dri2_window_attach(WindowPtr win, struct dri2_window *priv)
{
	assert(win->drawable.type == DRAWABLE_WINDOW);
	assert(dri2_window(win) == NULL);
	((void **)__get_private(win, sna_window_key))[1] = priv;
	assert(dri2_window(win) == priv);
}

static uint64_t
draw_current_msc(DrawablePtr draw, xf86CrtcPtr crtc, uint64_t msc)
{
	struct dri2_window *priv;

	assert(draw);
	if (draw->type != DRAWABLE_WINDOW)
		return msc;

	priv = dri2_window((WindowPtr)draw);
	if (priv == NULL) {
		priv = malloc(sizeof(*priv));
		if (priv != NULL) {
			priv->front = NULL;
			priv->crtc = crtc;
			priv->msc_delta = 0;
			priv->chain = NULL;
			priv->cache_size = 0;
			list_init(&priv->cache);
			dri2_window_attach((WindowPtr)draw, priv);
		}
	} else {
		if (priv->crtc != crtc) {
			const struct ust_msc *last = sna_crtc_last_swap(priv->crtc);
			const struct ust_msc *this = sna_crtc_last_swap(crtc);
			DBG(("%s: Window transferring from pipe=%d [msc=%llu] to pipe=%d [msc=%llu], delta now %lld\n",
			     __FUNCTION__,
			     sna_crtc_pipe(priv->crtc), (long long)last->msc,
			     sna_crtc_pipe(crtc), (long long)this->msc,
			     (long long)(priv->msc_delta + this->msc - last->msc)));
			priv->msc_delta += this->msc - last->msc;
			priv->crtc = crtc;
		}
		msc -= priv->msc_delta;
	}
	return  msc;
}

static uint32_t
draw_target_seq(DrawablePtr draw, uint64_t msc)
{
	struct dri2_window *priv = dri2_window((WindowPtr)draw);
	if (priv == NULL)
		return msc;
	DBG(("%s: converting target_msc=%llu to seq %u\n",
	     __FUNCTION__, (long long)msc, (unsigned)(msc + priv->msc_delta)));
	return msc + priv->msc_delta;
}

static xf86CrtcPtr
sna_dri2_get_crtc(DrawablePtr draw)
{
	if (draw->type == DRAWABLE_PIXMAP)
		return NULL;

	/* Make sure the CRTC is valid and this is the real front buffer */
	return sna_covering_crtc(to_sna_from_drawable(draw),
				 &((WindowPtr)draw)->clipList.extents,
				 NULL);
}

static void frame_swap_complete(struct sna_dri2_event *frame, int type)
{
	const struct ust_msc *swap;

	assert(frame->signal);
	frame->signal = false;

	if (frame->client == NULL) {
		DBG(("%s: client already gone\n", __FUNCTION__));
		return;
	}

	assert(frame->draw);

	swap = sna_crtc_last_swap(frame->crtc);
	DBG(("%s(type=%d): draw=%ld, pipe=%d, frame=%lld [msc=%lld], tv=%d.%06d\n",
	     __FUNCTION__, type, (long)frame->draw->id, frame->pipe,
	     (long long)swap->msc,
	     (long long)draw_current_msc(frame->draw, frame->crtc, swap->msc),
	     swap->tv_sec, swap->tv_usec));

	DRI2SwapComplete(frame->client, frame->draw,
			 draw_current_msc(frame->draw, frame->crtc, swap->msc),
			 swap->tv_sec, swap->tv_usec,
			 type, frame->event_complete, frame->event_data);
}

static void fake_swap_complete(struct sna *sna, ClientPtr client,
			       DrawablePtr draw, xf86CrtcPtr crtc,
			       int type, DRI2SwapEventPtr func, void *data)
{
	const struct ust_msc *swap;

	assert(draw);

	if (crtc == NULL)
		crtc = sna_primary_crtc(sna);

	swap = sna_crtc_last_swap(crtc);
	DBG(("%s(type=%d): draw=%ld, pipe=%d, frame=%lld [msc %lld], tv=%d.%06d\n",
	     __FUNCTION__, type, (long)draw->id, crtc ? sna_crtc_pipe(crtc) : -1,
	     (long long)swap->msc,
	     (long long)draw_current_msc(draw, crtc, swap->msc),
	     swap->tv_sec, swap->tv_usec));

	DRI2SwapComplete(client, draw,
			 draw_current_msc(draw, crtc, swap->msc),
			 swap->tv_sec, swap->tv_usec,
			 type, func, data);
}

static void
sna_dri2_remove_event(struct sna_dri2_event *info)
{
	WindowPtr win = (WindowPtr)info->draw;
	struct dri2_window *priv;

	assert(win->drawable.type == DRAWABLE_WINDOW);
	DBG(("%s: remove[%p] from window %ld, active? %d\n",
	     __FUNCTION__, info, (long)win->drawable.id, info->draw != NULL));
	assert(!info->signal);

	priv = dri2_window(win);
	assert(priv);
	assert(priv->chain != NULL);
	assert(info->chained);
	info->chained = false;

	if (priv->chain != info) {
		struct sna_dri2_event *chain = priv->chain;
		while (chain->chain != info) {
			assert(chain->chained);
			chain = chain->chain;
		}
		assert(chain != info);
		assert(info->chain != chain);
		chain->chain = info->chain;
		return;
	}

	priv->chain = info->chain;
	if (priv->chain == NULL) {
		struct dri_bo *c, *tmp;

		c = list_entry(priv->cache.next->next, struct dri_bo, link);
		list_for_each_entry_safe_from(c, tmp, &priv->cache, link) {
			list_del(&c->link);

			DBG(("%s: releasing cached handle=%d\n", __FUNCTION__, c->bo ? c->bo->handle : 0));
			assert(c->bo);
			kgem_bo_destroy(&info->sna->kgem, c->bo);
			free(c);
		}
	}
}

static void
sna_dri2_event_free(struct sna_dri2_event *info)
{
	DBG(("%s(draw?=%d)\n", __FUNCTION__, info->draw != NULL));
	assert(!info->queued);
	assert(!info->signal);
	assert(info->pending.bo == NULL);

	if (info->sna->dri2.flip_pending == info)
		info->sna->dri2.flip_pending = NULL;
	assert(info->sna->dri2.flip_pending != info);
	if (info->chained)
		sna_dri2_remove_event(info);

	assert((info->front == NULL && info->back == NULL) || info->front != info->back);
	_sna_dri2_destroy_buffer(info->sna, info->draw, info->front);
	_sna_dri2_destroy_buffer(info->sna, info->draw, info->back);

	if (info->bo) {
		DBG(("%s: releasing batch handle=%d\n", __FUNCTION__, info->bo->handle));
		kgem_bo_destroy(&info->sna->kgem, info->bo);
	}

	_list_del(&info->link);
	free(info);
}

static void
sna_dri2_client_gone(CallbackListPtr *list, void *closure, void *data)
{
	NewClientInfoRec *clientinfo = data;
	ClientPtr client = clientinfo->client;
	struct sna_client *priv = sna_client(client);
	struct sna *sna = closure;

	if (priv->events.next == NULL)
		return;

	if (client->clientState != ClientStateGone)
		return;

	DBG(("%s(active?=%d)\n", __FUNCTION__,
	     !list_is_empty(&priv->events)));

	while (!list_is_empty(&priv->events)) {
		struct sna_dri2_event *event;

		event = list_first_entry(&priv->events, struct sna_dri2_event, link);
		assert(event->client == client);
		list_del(&event->link);
		event->signal = false;

		if (event->pending.bo) {
			assert(event->pending.bo->active_scanout > 0);
			event->pending.bo->active_scanout--;

			kgem_bo_destroy(&sna->kgem, event->pending.bo);
			event->pending.bo = NULL;
		}

		if (event->chained)
			sna_dri2_remove_event(event);

		event->client = NULL;
		event->draw = NULL;
		event->keepalive = 1;
		assert(!event->signal);

		if (!event->queued)
			sna_dri2_event_free(event);
	}

	if (--sna->dri2.client_count == 0)
		DeleteCallback(&ClientStateCallback, sna_dri2_client_gone, sna);
}

static bool add_event_to_client(struct sna_dri2_event *info, struct sna *sna, ClientPtr client)
{
	struct sna_client *priv = sna_client(client);

	if (priv->events.next == NULL) {
		if (sna->dri2.client_count++ == 0 &&
		    !AddCallback(&ClientStateCallback, sna_dri2_client_gone, sna))
			return false;

		list_init(&priv->events);
	}

	list_add(&info->link, &priv->events);
	info->client = client;
	return true;
}

static struct sna_dri2_event *
sna_dri2_add_event(struct sna *sna,
		   DrawablePtr draw,
		   ClientPtr client,
		   xf86CrtcPtr crtc)
{
	struct dri2_window *priv;
	struct sna_dri2_event *info, *chain;

	assert(draw != NULL);
	assert(draw->type == DRAWABLE_WINDOW);
	DBG(("%s: adding event to window %ld)\n",
	     __FUNCTION__, (long)draw->id));

	priv = dri2_window((WindowPtr)draw);
	if (priv == NULL)
		return NULL;

	info = calloc(1, sizeof(struct sna_dri2_event));
	if (info == NULL)
		return NULL;

	info->sna = sna;
	info->draw = draw;
	info->crtc = crtc;
	info->pipe = sna_crtc_pipe(crtc);
	info->keepalive = 1;

	if (!add_event_to_client(info, sna, client)) {
		free(info);
		return NULL;
	}

	assert(priv->chain != info);
	info->chained = true;

	if (priv->chain == NULL) {
		priv->chain = info;
		return info;
	}

	chain = priv->chain;
	while (chain->chain != NULL)
		chain = chain->chain;

	assert(chain != info);
	chain->chain = info;
	return info;
}

static void decouple_window(WindowPtr win,
			    struct dri2_window *priv,
			    struct sna *sna,
			    bool signal)
{
	if (priv->front) {
		DBG(("%s: decouple private front\n", __FUNCTION__));
		assert(priv->crtc);
		sna_shadow_unset_crtc(sna, priv->crtc);

		_sna_dri2_destroy_buffer(sna, NULL, priv->front);
		priv->front = NULL;
	}

	if (priv->chain) {
		struct sna_dri2_event *info, *chain;

		DBG(("%s: freeing chain\n", __FUNCTION__));

		chain = priv->chain;
		while ((info = chain)) {
			DBG(("%s: freeing event, pending signal? %d, pending swap? handle=%d\n",
			     __FUNCTION__, info->signal,
			     info->pending.bo ? info->pending.bo->handle : 0));
			assert(info->draw == &win->drawable);

			if (info->pending.bo) {
				if (signal) {
					bool was_signalling = info->signal;
					info->signal = true;
					frame_swap_complete(info, DRI2_EXCHANGE_COMPLETE);
					info->signal = was_signalling;
				}
				assert(info->pending.bo->active_scanout > 0);
				info->pending.bo->active_scanout--;

				kgem_bo_destroy(&sna->kgem, info->pending.bo);
				info->pending.bo = NULL;
			}

			if (info->signal && signal)
				frame_swap_complete(info, DRI2_EXCHANGE_COMPLETE);
			info->signal = false;
			info->draw = NULL;
			info->keepalive = 1;
			assert(!info->signal);
			list_del(&info->link);

			chain = info->chain;
			info->chain = NULL;
			info->chained = false;

			if (!info->queued)
				sna_dri2_event_free(info);
		}

		priv->chain = NULL;
	}
}

void sna_dri2_decouple_window(WindowPtr win)
{
	struct dri2_window *priv;

	priv = dri2_window(win);
	if (priv == NULL)
		return;

	DBG(("%s: window=%ld\n", __FUNCTION__, win->drawable.id));
	decouple_window(win, priv, to_sna_from_drawable(&win->drawable), true);
}

void sna_dri2_destroy_window(WindowPtr win)
{
	struct dri2_window *priv;
	struct sna *sna;

	priv = dri2_window(win);
	if (priv == NULL)
		return;

	DBG(("%s: window=%ld\n", __FUNCTION__, win->drawable.id));
	sna = to_sna_from_drawable(&win->drawable);
	decouple_window(win, priv, sna, false);

	while (!list_is_empty(&priv->cache)) {
		struct dri_bo *c;

		c = list_first_entry(&priv->cache, struct dri_bo, link);
		list_del(&c->link);

		DBG(("%s: releasing cached handle=%d\n", __FUNCTION__, c->bo ? c->bo->handle : 0));
		assert(c->bo);
		kgem_bo_destroy(&sna->kgem, c->bo);
		free(c);
	}

	free(priv);
}

static void
sna_dri2_flip_handler(struct drm_event_vblank *event, void *data)
{
	DBG(("%s: sequence=%d\n", __FUNCTION__, event->sequence));
	sna_dri2_flip_event(data);
}

static bool
sna_dri2_flip(struct sna_dri2_event *info)
{
	struct kgem_bo *bo = get_private(info->back)->bo;
	struct kgem_bo *tmp_bo;
	uint32_t tmp_name, tmp_flags;
	int tmp_pitch;

	DBG(("%s(type=%d)\n", __FUNCTION__, info->type));

	assert(sna_pixmap_get_buffer(info->sna->front) == info->front);
	assert(get_drawable_pixmap(info->draw)->drawable.height * bo->pitch <= kgem_bo_size(bo));
	assert(get_private(info->front)->size == get_private(info->back)->size);
	assert(bo->refcnt);

	if (info->sna->mode.flip_active) {
		DBG(("%s: %d flips still active, aborting\n",
		     __FUNCTION__, info->sna->mode.flip_active));
		return false;
	}

	assert(!info->queued);
	if (!sna_page_flip(info->sna, bo, sna_dri2_flip_handler,
			   info->type == FLIP_ASYNC ? NULL : info))
		return false;

	DBG(("%s: queued flip=%p\n", __FUNCTION__, info->type == FLIP_ASYNC ? NULL : info));
	assert(info->signal || info->type != FLIP_THROTTLE);

	assert(info->sna->dri2.flip_pending == NULL ||
	       info->sna->dri2.flip_pending == info);
	if (info->type != FLIP_ASYNC)
		info->sna->dri2.flip_pending = info;

	DBG(("%s: marked handle=%d as scanout, swap front (handle=%d, name=%d) and back (handle=%d, name=%d)\n",
	     __FUNCTION__, bo->handle,
	     get_private(info->front)->bo->handle, info->front->name,
	     get_private(info->back)->bo->handle, info->back->name));

	tmp_bo = get_private(info->front)->bo;
	tmp_name = info->front->name;
	tmp_pitch = info->front->pitch;
	tmp_flags = info->front->flags;

	assert(tmp_bo->active_scanout > 0);
	tmp_bo->active_scanout--;

	set_bo(info->sna->front, bo);

	info->front->flags = info->back->flags;
	info->front->name = info->back->name;
	info->front->pitch = info->back->pitch;
	get_private(info->front)->bo = bo;
	bo->active_scanout++;
	assert(bo->active_scanout <= bo->refcnt);

	info->back->flags = tmp_flags;
	info->back->name = tmp_name;
	info->back->pitch = tmp_pitch;
	get_private(info->back)->bo = tmp_bo;
	mark_stale(info->back);

	assert(get_private(info->front)->bo->refcnt);
	assert(get_private(info->back)->bo->refcnt);
	assert(get_private(info->front)->bo != get_private(info->back)->bo);

	info->keepalive = KEEPALIVE;
	info->queued = true;
	return true;
}

static bool
can_flip(struct sna * sna,
	 DrawablePtr draw,
	 DRI2BufferPtr front,
	 DRI2BufferPtr back,
	 xf86CrtcPtr crtc)
{
	WindowPtr win = (WindowPtr)draw;
	PixmapPtr pixmap;

	assert((sna->flags & SNA_NO_WAIT) == 0);

	if (!DBG_CAN_FLIP)
		return false;

	if (draw->type == DRAWABLE_PIXMAP)
		return false;

	if (!sna->mode.front_active) {
		DBG(("%s: no, active CRTC\n", __FUNCTION__));
		return false;
	}

	assert(sna->scrn->vtSema);
	assert(!sna->mode.hidden);

	if ((sna->flags & (SNA_HAS_FLIP | SNA_HAS_ASYNC_FLIP)) == 0) {
		DBG(("%s: no, pageflips disabled\n", __FUNCTION__));
		return false;
	}

	if (front->cpp != back->cpp) {
		DBG(("%s: no, format mismatch, front = %d, back = %d\n",
		     __FUNCTION__, front->cpp, back->cpp));
		return false;
	}

	if (sna->mode.shadow_active) {
		DBG(("%s: no, shadow enabled\n", __FUNCTION__));
		return false;
	}

	if (!sna_crtc_is_on(crtc)) {
		DBG(("%s: ref-pipe=%d is disabled\n", __FUNCTION__, sna_crtc_pipe(crtc)));
		return false;
	}

	pixmap = get_window_pixmap(win);
	if (pixmap != sna->front) {
		DBG(("%s: no, window (pixmap=%ld) is not attached to the front buffer (pixmap=%ld)\n",
		     __FUNCTION__, pixmap->drawable.serialNumber, sna->front->drawable.serialNumber));
		return false;
	}

	if (sna_pixmap_get_buffer(pixmap) != front) {
		DBG(("%s: no, DRI2 drawable is no longer attached (old name=%d, new name=%d) to pixmap=%ld\n",
		     __FUNCTION__, front->name,
		     sna_pixmap_get_buffer(pixmap) ? sna_pixmap_get_buffer(pixmap)->name : 0,
		     pixmap->drawable.serialNumber));
		return false;
	}

	assert(get_private(front)->pixmap == sna->front);
	assert(sna_pixmap(sna->front)->gpu_bo == get_private(front)->bo);

	if (!get_private(back)->bo->scanout) {
		DBG(("%s: no, DRI2 drawable was too small at time of creation)\n",
		     __FUNCTION__));
		return false;
	}

	if (get_private(back)->size != get_private(front)->size) {
		DBG(("%s: no, DRI2 drawable does not fit into scanout\n",
		     __FUNCTION__));
		return false;
	}

	DBG(("%s: window size: %dx%d, clip=(%d, %d), (%d, %d) x %d\n",
	     __FUNCTION__,
	     win->drawable.width, win->drawable.height,
	     win->clipList.extents.x1, win->clipList.extents.y1,
	     win->clipList.extents.x2, win->clipList.extents.y2,
	     region_num_rects(&win->clipList)));
	if (!RegionEqual(&win->clipList, &draw->pScreen->root->winSize)) {
		DBG(("%s: no, window is clipped: clip region=(%d, %d), (%d, %d), root size=(%d, %d), (%d, %d)\n",
		     __FUNCTION__,
		     win->clipList.extents.x1,
		     win->clipList.extents.y1,
		     win->clipList.extents.x2,
		     win->clipList.extents.y2,
		     draw->pScreen->root->winSize.extents.x1,
		     draw->pScreen->root->winSize.extents.y1,
		     draw->pScreen->root->winSize.extents.x2,
		     draw->pScreen->root->winSize.extents.y2));
		return false;
	}

	if (draw->x != 0 || draw->y != 0 ||
#ifdef COMPOSITE
	    draw->x != pixmap->screen_x ||
	    draw->y != pixmap->screen_y ||
#endif
	    draw->width != pixmap->drawable.width ||
	    draw->height != pixmap->drawable.height) {
		DBG(("%s: no, window is not full size (%dx%d)!=(%dx%d)\n",
		     __FUNCTION__,
		     draw->width, draw->height,
		     pixmap->drawable.width,
		     pixmap->drawable.height));
		return false;
	}

	/* prevent an implicit tiling mode change */
	if (get_private(back)->bo->tiling > I915_TILING_X) {
		DBG(("%s -- no, tiling mismatch: front %d, back=%d, want-tiled?=%d\n",
		     __FUNCTION__,
		     get_private(front)->bo->tiling,
		     get_private(back)->bo->tiling,
		     !!(sna->flags & SNA_LINEAR_FB)));
		return false;
	}

	if (get_private(front)->bo->pitch != get_private(back)->bo->pitch) {
		DBG(("%s -- no, pitch mismatch: front %d, back=%d\n",
		     __FUNCTION__,
		     get_private(front)->bo->pitch,
		     get_private(back)->bo->pitch));
		return false;
	}

	if (sna_pixmap(pixmap)->pinned & ~(PIN_DRI2 | PIN_SCANOUT)) {
		DBG(("%s -- no, pinned: front %x\n",
		     __FUNCTION__, sna_pixmap(pixmap)->pinned));
		return false;
	}

	DBG(("%s: yes, pixmap=%ld\n", __FUNCTION__, pixmap->drawable.serialNumber));
	return true;
}

static bool
can_xchg(struct sna *sna,
	 DrawablePtr draw,
	 DRI2BufferPtr front,
	 DRI2BufferPtr back)
{
	WindowPtr win = (WindowPtr)draw;
	PixmapPtr pixmap;

	if (!DBG_CAN_XCHG)
		return false;

	if (draw->type == DRAWABLE_PIXMAP)
		return false;

	if (front->cpp != back->cpp) {
		DBG(("%s: no, format mismatch, front = %d, back = %d\n",
		     __FUNCTION__, front->cpp, back->cpp));
		return false;
	}

	pixmap = get_window_pixmap(win);
	if (get_private(front)->pixmap != pixmap) {
		DBG(("%s: no, DRI2 drawable is no longer attached, old pixmap=%ld, now pixmap=%ld\n",
		     __FUNCTION__,
		     get_private(front)->pixmap->drawable.serialNumber,
		     pixmap->drawable.serialNumber));
		return false;
	}

	DBG(("%s: window size: %dx%d, clip=(%d, %d), (%d, %d) x %d, pixmap size=%dx%d\n",
	     __FUNCTION__,
	     win->drawable.width, win->drawable.height,
	     win->clipList.extents.x1, win->clipList.extents.y1,
	     win->clipList.extents.x2, win->clipList.extents.y2,
	     region_num_rects(&win->clipList),
	     pixmap->drawable.width,
	     pixmap->drawable.height));
	if (is_clipped(&win->clipList, &pixmap->drawable)) {
		DBG(("%s: no, %dx%d window is clipped: clip region=(%d, %d), (%d, %d)\n",
		     __FUNCTION__,
		     draw->width, draw->height,
		     win->clipList.extents.x1,
		     win->clipList.extents.y1,
		     win->clipList.extents.x2,
		     win->clipList.extents.y2));
		return false;
	}

	DBG(("%s: back size=%x, front size=%x\n",
	     __FUNCTION__, get_private(back)->size, get_private(front)->size));
	if (get_private(back)->size != get_private(front)->size) {
		DBG(("%s: no, back buffer %dx%d does not match front buffer %dx%d\n",
		     __FUNCTION__,
		     get_private(back)->size & 0x7fff, (get_private(back)->size >> 16) & 0x7fff,
		     get_private(front)->size & 0x7fff, (get_private(front)->size >> 16) & 0x7fff));
		return false;
	}

	if (pixmap == sna->front && !(sna->flags & SNA_TEAR_FREE) && sna->mode.front_active) {
		DBG(("%s: no, front buffer, requires flipping\n",
		     __FUNCTION__));
		return false;
	}

	if (sna_pixmap(pixmap)->pinned & ~(PIN_DRI2 | PIN_SCANOUT)) {
		DBG(("%s: no, pinned: %x\n",
		     __FUNCTION__, sna_pixmap(pixmap)->pinned));
		return false;
	}

	DBG(("%s: yes, pixmap=%ld\n", __FUNCTION__, pixmap->drawable.serialNumber));
	return true;
}

static bool
overlaps_other_crtc(struct sna *sna, xf86CrtcPtr desired)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(sna->scrn);
	int c;

	for (c = 0; c < sna->mode.num_real_crtc; c++) {
		xf86CrtcPtr crtc = config->crtc[c];

		if (crtc == desired)
			continue;

		if (!crtc->enabled)
			continue;

		if (desired->bounds.x1 < crtc->bounds.x2 &&
		    desired->bounds.x2 > crtc->bounds.x1 &&
		    desired->bounds.y1 < crtc->bounds.y2 &&
		    desired->bounds.y2 > crtc->bounds.y1)
			return true;
	}

	return false;
}

static bool
can_xchg_crtc(struct sna *sna,
	      DrawablePtr draw,
	      xf86CrtcPtr crtc,
	      DRI2BufferPtr front,
	      DRI2BufferPtr back)
{
	WindowPtr win = (WindowPtr)draw;
	PixmapPtr pixmap;

	if (!DBG_CAN_XCHG)
		return false;

	if ((sna->flags & SNA_TEAR_FREE) == 0) {
		DBG(("%s: no, requires TearFree\n",
		     __FUNCTION__));
		return false;
	}

	if (draw->type == DRAWABLE_PIXMAP)
		return false;

	if (front->cpp != back->cpp) {
		DBG(("%s: no, format mismatch, front = %d, back = %d\n",
		     __FUNCTION__, front->cpp, back->cpp));
		return false;
	}

	if (memcmp(&win->clipList.extents, &crtc->bounds, sizeof(crtc->bounds))) {
		DBG(("%s: no, window [(%d, %d), (%d, %d)] does not cover CRTC [(%d, %d), (%d, %d)]\n",
		     __FUNCTION__,
		     win->clipList.extents.x1, win->clipList.extents.y1,
		     win->clipList.extents.x2, win->clipList.extents.y2,
		     crtc->bounds.x1, crtc->bounds.y1,
		     crtc->bounds.x2, crtc->bounds.y2));
		return false;
	}

	if (sna_crtc_is_transformed(crtc)) {
		DBG(("%s: no, CRTC is rotated\n", __FUNCTION__));
		return false;
	}

	pixmap = get_window_pixmap(win);
	if (pixmap != sna->front) {
		DBG(("%s: no, not attached to front buffer\n", __FUNCTION__));
		return false;
	}

	if (get_private(front)->pixmap != pixmap) {
		DBG(("%s: no, DRI2 drawable is no longer attached, old pixmap=%ld, now pixmap=%ld\n",
		     __FUNCTION__,
		     get_private(front)->pixmap->drawable.serialNumber,
		     pixmap->drawable.serialNumber));
		return false;
	}

	DBG(("%s: window size: %dx%d, clip=(%d, %d), (%d, %d) x %d\n",
	     __FUNCTION__,
	     win->drawable.width, win->drawable.height,
	     win->clipList.extents.x1, win->clipList.extents.y1,
	     win->clipList.extents.x2, win->clipList.extents.y2,
	     region_num_rects(&win->clipList)));
	if (is_clipped(&win->clipList, &win->drawable)) {
		DBG(("%s: no, %dx%d window is clipped: clip region=(%d, %d), (%d, %d)\n",
		     __FUNCTION__,
		     draw->width, draw->height,
		     win->clipList.extents.x1,
		     win->clipList.extents.y1,
		     win->clipList.extents.x2,
		     win->clipList.extents.y2));
		return false;
	}

	if (overlaps_other_crtc(sna, crtc)) {
		DBG(("%s: no, overlaps other CRTC\n", __FUNCTION__));
		return false;
	}

	if (get_private(back)->size != (draw->height << 16 | draw->width)) {
		DBG(("%s: no, DRI2 buffers does not fit window\n",
		     __FUNCTION__));
		return false;
	}

	assert(win != win->drawable.pScreen->root);
	DBG(("%s: yes, pixmap=%ld\n", __FUNCTION__, pixmap->drawable.serialNumber));
	return true;
}

static void
sna_dri2_xchg(DrawablePtr draw, DRI2BufferPtr front, DRI2BufferPtr back)
{
	WindowPtr win = (WindowPtr)draw;
	struct kgem_bo *back_bo, *front_bo;
	PixmapPtr pixmap;
	int tmp;

	assert(draw->type != DRAWABLE_PIXMAP);
	pixmap = get_window_pixmap(win);

	back_bo = get_private(back)->bo;
	front_bo = get_private(front)->bo;

	DBG(("%s: win=%ld, exchange front=%d/%d,ref=%d and back=%d/%d,ref=%d, pixmap=%ld %dx%d\n",
	     __FUNCTION__, win->drawable.id,
	     front_bo->handle, front->name, get_private(front)->refcnt,
	     back_bo->handle, back->name, get_private(back)->refcnt,
	     pixmap->drawable.serialNumber,
	     pixmap->drawable.width,
	     pixmap->drawable.height));

	DBG(("%s: back_bo handle=%d, pitch=%d, size=%d, ref=%d, active_scanout?=%d\n",
	     __FUNCTION__, back_bo->handle, back_bo->pitch, kgem_bo_size(back_bo), back_bo->refcnt, back_bo->active_scanout));
	DBG(("%s: front_bo handle=%d, pitch=%d, size=%d, ref=%d, active_scanout?=%d\n",
	     __FUNCTION__, front_bo->handle, front_bo->pitch, kgem_bo_size(front_bo), front_bo->refcnt, front_bo->active_scanout));

	assert(front_bo != back_bo);
	assert(front_bo->refcnt);
	assert(back_bo->refcnt);

	assert(sna_pixmap_get_buffer(pixmap) == front);

	assert(pixmap->drawable.height * back_bo->pitch <= kgem_bo_size(back_bo));
	assert(pixmap->drawable.height * front_bo->pitch <= kgem_bo_size(front_bo));

	set_bo(pixmap, back_bo);

	get_private(front)->bo = back_bo;
	get_private(back)->bo = front_bo;
	mark_stale(back);

	assert(front_bo->active_scanout > 0);
	front_bo->active_scanout--;
	back_bo->active_scanout++;
	assert(back_bo->active_scanout <= back_bo->refcnt);

	tmp = front->name;
	front->name = back->name;
	back->name = tmp;

	tmp = front->pitch;
	front->pitch = back->pitch;
	back->pitch = tmp;

	tmp = front->flags;
	front->flags = back->flags;
	back->flags = tmp;

	assert(front_bo->refcnt);
	assert(back_bo->refcnt);

	assert(front_bo->pitch == get_private(front)->bo->pitch);
	assert(back_bo->pitch == get_private(back)->bo->pitch);

	assert(get_private(front)->bo == sna_pixmap(pixmap)->gpu_bo);
}

static void sna_dri2_xchg_crtc(struct sna *sna, DrawablePtr draw, xf86CrtcPtr crtc, DRI2BufferPtr front, DRI2BufferPtr back)
{
	WindowPtr win = (WindowPtr)draw;
	struct dri2_window *priv = dri2_window(win);

	DBG(("%s: exchange front=%d/%d and back=%d/%d, win id=%lu, pixmap=%ld %dx%d\n",
	     __FUNCTION__,
	     get_private(front)->bo->handle, front->name,
	     get_private(back)->bo->handle, back->name,
	     win->drawable.id,
	     get_window_pixmap(win)->drawable.serialNumber,
	     get_window_pixmap(win)->drawable.width,
	     get_window_pixmap(win)->drawable.height));
	assert(can_xchg_crtc(sna, draw, crtc, front, back));

	if (APPLY_DAMAGE) {
		DBG(("%s: marking drawable as damaged\n", __FUNCTION__));
		sna->ignore_copy_area = sna->flags & SNA_TEAR_FREE;
		DamageRegionAppend(&win->drawable, &win->clipList);
	}
	sna_shadow_set_crtc(sna, crtc, get_private(back)->bo);
	if (APPLY_DAMAGE) {
		sna->ignore_copy_area = false;
		DamageRegionProcessPending(&win->drawable);
	}

	if (priv->front == NULL) {
		DRI2Buffer2Ptr tmp;

		tmp = calloc(1, sizeof(*tmp) + sizeof(struct sna_dri2_private));
		if (tmp == NULL) {
			sna_shadow_unset_crtc(sna, crtc);
			return;
		}

		tmp->attachment = DRI2BufferFrontLeft;
		tmp->driverPrivate = tmp + 1;
		tmp->cpp = back->cpp;
		tmp->format = back->format;

		get_private(tmp)->refcnt = 1;
		get_private(tmp)->bo = kgem_create_2d(&sna->kgem,
						      draw->width, draw->height, draw->bitsPerPixel,
						      get_private(back)->bo->tiling,
						      CREATE_SCANOUT | CREATE_EXACT);
		if (get_private(tmp)->bo != NULL) {
			tmp->pitch = get_private(tmp)->bo->pitch;
			tmp->name = kgem_bo_flink(&sna->kgem, get_private(tmp)->bo);
		}
		if (tmp->name == 0) {
			if (get_private(tmp)->bo != NULL)
				kgem_bo_destroy(&sna->kgem, get_private(tmp)->bo);
			sna_shadow_unset_crtc(sna, crtc);
			return;
		}
		get_private(tmp)->size = get_private(back)->size;
		get_private(tmp)->pixmap = get_private(front)->pixmap;
		get_private(tmp)->proxy = sna_dri2_reference_buffer(front);
		get_private(tmp)->bo->active_scanout++;

		priv->front = front = tmp;
	}
	assert(front == priv->front);

	{
		struct kgem_bo *front_bo = get_private(front)->bo;
		struct kgem_bo *back_bo = get_private(back)->bo;
		unsigned tmp;

		assert(front_bo->refcnt);
		assert(back_bo->refcnt);

		get_private(back)->bo = front_bo;
		get_private(front)->bo = back_bo;
		mark_stale(back);

		assert(front_bo->active_scanout > 0);
		front_bo->active_scanout--;
		back_bo->active_scanout++;
		assert(back_bo->active_scanout <= back_bo->refcnt);

		tmp = front->name;
		front->name = back->name;
		back->name = tmp;

		tmp = front->pitch;
		front->pitch = back->pitch;
		back->pitch = tmp;

		tmp = front->flags;
		front->flags = back->flags;
		back->flags = tmp;
	}
}

static void chain_swap(struct sna_dri2_event *chain)
{
	DBG(("%s: draw=%ld, queued?=%d, type=%d\n",
	     __FUNCTION__, (long)chain->draw->id, chain->queued, chain->type));

	if (chain->queued) /* too early! */
		return;

	if (chain->draw == NULL) {
		sna_dri2_event_free(chain);
		return;
	}

	assert(chain == dri2_chain(chain->draw));
	assert(chain->signal);

	switch (chain->type) {
	case SWAP_COMPLETE:
		DBG(("%s: emitting chained vsync'ed blit\n", __FUNCTION__));
		if (can_xchg(chain->sna, chain->draw, chain->front, chain->back)) {
			sna_dri2_xchg(chain->draw, chain->front, chain->back);
		} else if (can_xchg_crtc(chain->sna, chain->draw, chain->crtc,
					 chain->front, chain->back)) {
			sna_dri2_xchg_crtc(chain->sna, chain->draw, chain->crtc,
					   chain->front, chain->back);
		} else {
			__sna_dri2_copy_event(chain, chain->sync | DRI2_BO);
		}
		assert(get_private(chain->back)->bo != get_private(chain->front)->bo);
	case SWAP:
		break;
	default:
		return;
	}

	if ((chain->type == SWAP_COMPLETE &&
	     !swap_limit(chain->draw, 2 + !chain->sync) &&
	     !chain->sync) ||
	    !sna_next_vblank(chain)) {
		DBG(("%s: vblank wait failed, unblocking client\n", __FUNCTION__));
		frame_swap_complete(chain, DRI2_BLIT_COMPLETE);
		sna_dri2_event_free(chain);
	}
}

static inline bool rq_is_busy(struct kgem *kgem, struct kgem_bo *bo)
{
	if (bo == NULL)
		return false;

	return __kgem_bo_is_busy(kgem, bo);
}

static bool sna_dri2_blit_complete(struct sna_dri2_event *info)
{
	if (!info->bo)
		return true;

	if (__kgem_bo_is_busy(&info->sna->kgem, info->bo)) {
		DBG(("%s: vsync'ed blit is still busy, postponing\n",
		     __FUNCTION__));
		if (sna_next_vblank(info))
			return false;

		kgem_bo_sync__gtt(&info->sna->kgem, info->bo);
	}

	DBG(("%s: blit finished\n", __FUNCTION__));
	kgem_bo_destroy(&info->sna->kgem, info->bo);
	info->bo = NULL;

	return true;
}

void sna_dri2_vblank_handler(struct drm_event_vblank *event)
{
	struct sna_dri2_event *info = (void *)(uintptr_t)event->user_data;
	struct sna *sna = info->sna;
	DrawablePtr draw;
	uint64_t msc;

	DBG(("%s(type=%d, sequence=%d, draw=%ld)\n", __FUNCTION__, info->type, event->sequence, info->draw ? info->draw->serialNumber : 0));
	assert(info->queued);
	info->queued = false;

	msc = sna_crtc_record_event(info->crtc, event);

	draw = info->draw;
	if (draw == NULL) {
		DBG(("%s -- drawable gone\n", __FUNCTION__));
		goto done;
	}

	assert((info->front == NULL && info->back == NULL) || info->front != info->back);
	switch (info->type) {
	case FLIP:
		/* If we can still flip... */
		assert(info->signal);
		if (can_flip(sna, draw, info->front, info->back, info->crtc) &&
		    sna_dri2_flip(info))
			return;

		/* fall through */
	case SWAP:
		assert(info->signal);
		if (can_xchg(info->sna, draw, info->front, info->back)) {
			sna_dri2_xchg(draw, info->front, info->back);
			info->type = SWAP_COMPLETE;
		} else if (can_xchg_crtc(sna, draw, info->crtc,
					 info->front, info->back)) {
			sna_dri2_xchg_crtc(sna, draw, info->crtc,
					   info->front, info->back);
			info->type = SWAP_COMPLETE;
		}  else {
			__sna_dri2_copy_event(info, DRI2_BO | DRI2_SYNC);
			info->type = SWAP_COMPLETE;
		}

		if (sna_next_vblank(info))
			return;

		DBG(("%s -- requeue failed, errno=%d\n", __FUNCTION__, errno));
		assert(info->pending.bo == NULL);
		assert(info->keepalive == 1);
		/* fall through */
	case SWAP_COMPLETE:
		DBG(("%s: %d complete, frame=%d tv=%d.%06d\n",
		     __FUNCTION__, info->type,
		     event->sequence, event->tv_sec, event->tv_usec));

		if (info->signal) {
			if (!sna_dri2_blit_complete(info))
				return;

			DBG(("%s: triple buffer swap complete, unblocking client (frame=%d, tv=%d.%06d)\n", __FUNCTION__,
			     event->sequence, event->tv_sec, event->tv_usec));
			frame_swap_complete(info, DRI2_BLIT_COMPLETE);
		}

		if (info->pending.bo) {
			struct copy current_back;

			DBG(("%s: swapping back handle=%d [name=%d, active=%d] for pending handle=%d [name=%d, active=%d], front handle=%d [name=%d, active=%d]\n",
			     __FUNCTION__,
			     get_private(info->back)->bo->handle, info->back->name, get_private(info->back)->bo->active_scanout,
			     info->pending.bo->handle, info->pending.name, info->pending.bo->active_scanout,
			     get_private(info->front)->bo->handle, info->front->name, get_private(info->front)->bo->active_scanout));

			assert(info->pending.bo->active_scanout > 0);
			info->pending.bo->active_scanout--;

			current_back.bo = get_private(info->back)->bo;
			current_back.size = get_private(info->back)->size;
			current_back.name = info->back->name;
			current_back.flags = info->back->flags;

			get_private(info->back)->bo = info->pending.bo;
			get_private(info->back)->size = info->pending.size;
			info->back->name = info->pending.name;
			info->back->pitch = info->pending.bo->pitch;
			info->back->flags = info->pending.flags;
			info->pending.bo = NULL;

			assert(get_private(info->back)->bo != get_private(info->front)->bo);

			if (can_xchg(info->sna, info->draw, info->front, info->back))
				sna_dri2_xchg(info->draw, info->front, info->back);
			else if (can_xchg_crtc(info->sna, info->draw, info->crtc,
						 info->front, info->back))
				sna_dri2_xchg_crtc(info->sna, info->draw, info->crtc,
						   info->front, info->back);
			else
				__sna_dri2_copy_event(info, info->sync | DRI2_BO);

			sna_dri2_cache_bo(info->sna, info->draw,
					  get_private(info->back)->bo,
					  info->back->name,
					  get_private(info->back)->size,
					  info->back->flags);

			get_private(info->back)->bo = current_back.bo;
			get_private(info->back)->size = current_back.size;
			info->back->name = current_back.name;
			info->back->pitch = current_back.bo->pitch;
			info->back->flags = current_back.flags;

			DBG(("%s: restored current back handle=%d [name=%d, active=%d], active=%d], front handle=%d [name=%d, active=%d]\n",
			     __FUNCTION__,
			     get_private(info->back)->bo->handle, info->back->name, get_private(info->back)->bo->active_scanout,
			     get_private(info->front)->bo->handle, info->front->name, get_private(info->front)->bo->active_scanout));

			assert(info->draw);
			assert(!info->signal);
			info->keepalive++;
			info->signal = true;
		}

		if (--info->keepalive) {
			if (sna_next_vblank(info))
				return;

			if (info->signal) {
				DBG(("%s: triple buffer swap complete, unblocking client (frame=%d, tv=%d.%06d)\n", __FUNCTION__,
				     event->sequence, event->tv_sec, event->tv_usec));
				frame_swap_complete(info, DRI2_BLIT_COMPLETE);
			}
		}
		break;

	case WAITMSC:
		assert(info->client);
		DRI2WaitMSCComplete(info->client, draw, msc,
				    event->tv_sec, event->tv_usec);
		break;
	default:
		xf86DrvMsg(sna->scrn->scrnIndex, X_WARNING,
			   "%s: unknown vblank event received\n", __func__);
		/* Unknown type */
		break;
	}

	if (info->chain) {
		DBG(("%s: continuing chain\n", __FUNCTION__));
		assert(info->chain != info);
		assert(info->draw == draw);
		sna_dri2_remove_event(info);
		chain_swap(info->chain);
	}

done:
	sna_dri2_event_free(info);
	DBG(("%s complete\n", __FUNCTION__));
}

static void
sna_dri2_immediate_blit(struct sna *sna,
			struct sna_dri2_event *info,
			bool sync)
{
	struct sna_dri2_event *chain = dri2_chain(info->draw);

	if (sna->flags & SNA_NO_WAIT)
		sync = false;

	DBG(("%s: emitting immediate blit, throttling client, synced? %d, chained? %d, pipe %d\n",
	     __FUNCTION__, sync, chain != info, info->pipe));
	assert(chain);

	info->type = SWAP_COMPLETE;
	info->sync = sync;
	info->keepalive = KEEPALIVE;

	if (chain == info) {
		DBG(("%s: no pending blit, starting chain\n", __FUNCTION__));

		assert(info->front != info->back);
		if (can_xchg(info->sna, info->draw, info->front, info->back)) {
			sna_dri2_xchg(info->draw, info->front, info->back);
		} else if (can_xchg_crtc(info->sna, info->draw, info->crtc,
					 info->front, info->back)) {
			sna_dri2_xchg_crtc(info->sna, info->draw, info->crtc,
					   info->front, info->back);
		} else
			__sna_dri2_copy_event(info, sync | DRI2_BO);

		assert(info->signal);

		if ((!swap_limit(info->draw, 2 + !sync) && !sync) ||
		    !sna_next_vblank(info)) {
			DBG(("%s: fake triple buffering, unblocking client\n", __FUNCTION__));
			frame_swap_complete(info, DRI2_BLIT_COMPLETE);
			sna_dri2_event_free(info);
		}
		return;
	}

	DBG(("%s: current event front=%d [name=%d, active?=%d], back=%d [name=%d, active?=%d]\n", __FUNCTION__,
	     get_private(chain->front)->bo->handle, chain->front->name, get_private(chain->front)->bo->active_scanout,
	     get_private(chain->back)->bo->handle, chain->back->name, get_private(chain->back)->bo->active_scanout));

	if (chain->type == SWAP_COMPLETE && chain->front == info->front) {
		assert(chain->draw == info->draw);
		assert(chain->client == info->client);
		assert(chain->event_complete == info->event_complete);
		assert(chain->event_data == info->event_data);
		assert(chain->queued);

		if ((!sync || !chain->sync) && chain->pending.bo) {
			bool signal = chain->signal;

			DBG(("%s: swap elision, unblocking client\n", __FUNCTION__));
			assert(chain->draw);
			chain->signal = true;
			frame_swap_complete(chain, DRI2_EXCHANGE_COMPLETE);
			chain->signal = signal;

			assert(chain->pending.bo->active_scanout > 0);
			chain->pending.bo->active_scanout--;

			sna_dri2_cache_bo(chain->sna, chain->draw,
					  chain->pending.bo,
					  chain->pending.name,
					  chain->pending.size,
					  chain->pending.flags);
			chain->pending.bo = NULL;
		}

		if (chain->pending.bo == NULL && swap_limit(info->draw, 2 + !sync)) {
			DBG(("%s: setting handle=%d as pending blit (current event front=%d, back=%d)\n", __FUNCTION__,
			     get_private(info->back)->bo->handle,
			     get_private(chain->front)->bo->handle,
			     get_private(chain->back)->bo->handle));
			chain->pending.bo = ref(get_private(info->back)->bo);
			chain->pending.size = get_private(info->back)->size;
			chain->pending.name = info->back->name;
			chain->pending.flags = info->back->flags;
			chain->sync = sync;
			info->signal = false; /* transfer signal to pending */

			/* Prevent us from handing it back on next GetBuffers */
			chain->pending.bo->active_scanout++;

			sna_dri2_event_free(info);
			return;
		}
	}

	DBG(("%s: pending blit, chained\n", __FUNCTION__));
}

static bool
sna_dri2_flip_continue(struct sna_dri2_event *info)
{
	struct kgem_bo *bo = get_private(info->front)->bo;

	DBG(("%s(mode=%d)\n", __FUNCTION__, info->flip_continue));
	assert(info->flip_continue > 0);
	info->type = info->flip_continue;
	info->flip_continue = 0;

	assert(!info->signal);
	info->signal = info->type == FLIP_THROTTLE && info->draw;

	if (info->sna->mode.front_active == 0)
		return false;

	if (bo != sna_pixmap(info->sna->front)->gpu_bo)
		return false;

	assert(!info->queued);
	if (!sna_page_flip(info->sna, bo, sna_dri2_flip_handler, info))
		return false;

	DBG(("%s: queued flip=%p\n", __FUNCTION__, info));
	assert(info->sna->dri2.flip_pending == NULL ||
	       info->sna->dri2.flip_pending == info);
	info->sna->dri2.flip_pending = info;
	info->queued = true;

	return true;
}

static bool
sna_dri2_flip_keepalive(struct sna_dri2_event *info)
{
	DBG(("%s(keepalive?=%d)\n", __FUNCTION__, info->keepalive-1));
	assert(info->keepalive > 0);
	if (!--info->keepalive)
		return false;

	if (info->draw == NULL)
		return false;

	DBG(("%s: marking next flip as complete\n", __FUNCTION__));
	info->flip_continue = FLIP_COMPLETE;
	return sna_dri2_flip_continue(info);
}

static void chain_flip(struct sna *sna)
{
	struct sna_dri2_event *chain = sna->dri2.flip_pending;

	assert(chain->type == FLIP);
	DBG(("%s: chaining type=%d, cancelled?=%d window=%ld\n",
	     __FUNCTION__, chain->type, chain->draw == NULL, chain->draw ? chain->draw->id : 0));

	sna->dri2.flip_pending = NULL;
	if (chain->draw == NULL) {
		sna_dri2_event_free(chain);
		return;
	}

	assert(chain == dri2_chain(chain->draw));
	assert(!chain->queued);

	if (can_flip(sna, chain->draw, chain->front, chain->back, chain->crtc) &&
	    sna_dri2_flip(chain)) {
		DBG(("%s: performing chained flip\n", __FUNCTION__));
	} else {
		DBG(("%s: emitting chained vsync'ed blit\n", __FUNCTION__));
		__sna_dri2_copy_event(chain, DRI2_SYNC);

		if (xorg_can_triple_buffer()) {
			chain->type = SWAP_COMPLETE;
			assert(chain->signal);
			if (sna_next_vblank(chain))
				return;
		}

		DBG(("%s: fake triple buffering (or vblank wait failed), unblocking client\n", __FUNCTION__));
		frame_swap_complete(chain, DRI2_BLIT_COMPLETE);
		sna_dri2_event_free(chain);
	}
}

static void sna_dri2_flip_event(struct sna_dri2_event *flip)
{
	struct sna *sna = flip->sna;

	DBG(("%s flip=%p (pipe=%d, event=%d, queued?=%d)\n", __FUNCTION__, flip, flip->pipe, flip->type, flip->queued));
	if (!flip->queued) /* pageflip died whilst being queued */
		return;
	flip->queued = false;

	if (sna->dri2.flip_pending == flip)
		sna->dri2.flip_pending = NULL;

	/* We assume our flips arrive in order, so we don't check the frame */
	switch (flip->type) {
	case FLIP:
		if (flip->signal) {
			DBG(("%s: swap complete, unblocking client\n", __FUNCTION__));
			frame_swap_complete(flip, DRI2_FLIP_COMPLETE);
		}
		sna_dri2_event_free(flip);

		if (sna->dri2.flip_pending)
			chain_flip(sna);
		break;

	case FLIP_THROTTLE:
		if (flip->signal) {
			DBG(("%s: triple buffer swap complete, unblocking client\n", __FUNCTION__));
			frame_swap_complete(flip, DRI2_FLIP_COMPLETE);
		}
		/* fall through */
	case FLIP_COMPLETE:
		assert(!flip->signal);
		if (sna->dri2.flip_pending) {
			DBG(("%s: pending flip\n", __FUNCTION__));
			sna_dri2_event_free(flip);
			chain_flip(sna);
		} else if (!flip->flip_continue) {
			DBG(("%s: flip chain complete\n", __FUNCTION__));
			if (!sna_dri2_flip_keepalive(flip)) {
				if (flip->chain) {
					sna_dri2_remove_event(flip);
					chain_swap(flip->chain);
				}

				sna_dri2_event_free(flip);
			}
		} else if (!sna_dri2_flip_continue(flip)) {
			DBG(("%s: no longer able to flip\n", __FUNCTION__));
			if (flip->draw != NULL)
				__sna_dri2_copy_event(flip, 0);
			if (flip->signal) {
				DBG(("%s: fake triple buffering, unblocking client\n", __FUNCTION__));
				frame_swap_complete(flip, DRI2_BLIT_COMPLETE);
			}
			sna_dri2_event_free(flip);
		}
		break;

	default: /* Unknown type */
		xf86DrvMsg(sna->scrn->scrnIndex, X_WARNING,
			   "%s: unknown vblank event received\n", __func__);
		sna_dri2_event_free(flip);
		if (sna->dri2.flip_pending)
			chain_flip(sna);
		break;
	}
}

static int
sna_query_vblank(struct sna *sna, xf86CrtcPtr crtc, union drm_wait_vblank *vbl)
{
	VG_CLEAR(*vbl);
	vbl->request.type =
		_DRM_VBLANK_RELATIVE | pipe_select(sna_crtc_pipe(crtc));
	vbl->request.sequence = 0;

	return drmIoctl(sna->kgem.fd, DRM_IOCTL_WAIT_VBLANK, vbl);
}

static uint64_t
get_current_msc(struct sna *sna, DrawablePtr draw, xf86CrtcPtr crtc)
{
	union drm_wait_vblank vbl;
	uint64_t ret;

	if (sna_query_vblank(sna, crtc, &vbl) == 0)
		ret = sna_crtc_record_vblank(crtc, &vbl);
	else
		ret = sna_crtc_last_swap(crtc)->msc;

	return draw_current_msc(draw, crtc, ret);
}

#if defined(CHECK_FOR_COMPOSITOR)
static Bool find(pointer value, XID id, pointer cdata)
{
	return TRUE;
}
#endif

static int use_triple_buffer(struct sna *sna, ClientPtr client, bool async)
{
	if ((sna->flags & SNA_TRIPLE_BUFFER) == 0) {
		DBG(("%s: triple buffer disabled, using FLIP\n", __FUNCTION__));
		return FLIP;
	}

	if (async) {
		DBG(("%s: running async, using %s\n", __FUNCTION__,
		     sna->flags & SNA_HAS_ASYNC_FLIP ? "FLIP_ASYNC" : "FLIP_COMPLETE"));
		return sna->flags & SNA_HAS_ASYNC_FLIP ? FLIP_ASYNC : FLIP_COMPLETE;
	}

	if (xorg_can_triple_buffer()) {
		DBG(("%s: triple buffer enabled, using FLIP_THROTTLE\n", __FUNCTION__));
		return FLIP_THROTTLE;
	}

#if defined(CHECK_FOR_COMPOSITOR)
	/* Hack: Disable triple buffering for compositors */
	{
		struct sna_client *priv = sna_client(client);
		if (priv->is_compositor == 0)
			priv->is_compositor =
				LookupClientResourceComplex(client,
							    CompositeClientWindowType+1,
							    find, NULL) ? FLIP : FLIP_COMPLETE;

		DBG(("%s: fake triple buffer enabled?=%d using %s\n", __FUNCTION__,
		     priv->is_compositor != FLIP, priv->is_compositor == FLIP ? "FLIP" : "FLIP_COMPLETE"));
		return priv->is_compositor;
	}
#else
	DBG(("%s: fake triple buffer enabled, using FLIP_COMPLETE\n", __FUNCTION__));
	return FLIP_COMPLETE;
#endif
}

static bool immediate_swap(struct sna *sna,
			   DrawablePtr draw,
			   xf86CrtcPtr crtc,
			   uint64_t *target_msc,
			   uint64_t divisor,
			   uint64_t remainder,
			   uint64_t *current_msc)
{
	/*
	 * If divisor is zero, or current_msc is smaller than target_msc
	 * we just need to make sure target_msc passes before initiating
	 * the swap.
	 */
	if (divisor == 0) {
		*current_msc = -1;

		if (sna->flags & SNA_NO_WAIT) {
			DBG(("%s: yes, waits are disabled\n", __FUNCTION__));
			return true;
		}

		if (*target_msc)
			*current_msc = get_current_msc(sna, draw, crtc);

		DBG(("%s: current_msc=%ld, target_msc=%ld -- %s\n",
		     __FUNCTION__, (long)*current_msc, (long)*target_msc,
		     (*current_msc >= *target_msc - 1) ? "yes" : "no"));
		return *current_msc >= *target_msc - 1;
	}

	DBG(("%s: explicit waits requests, divisor=%ld\n",
	     __FUNCTION__, (long)divisor));
	*current_msc = get_current_msc(sna, draw, crtc);
	if (*current_msc >= *target_msc) {
		DBG(("%s: missed target, queueing event for next: current=%lld, target=%lld, divisor=%lld, remainder=%lld\n",
		     __FUNCTION__,
		     (long long)*current_msc,
		     (long long)*target_msc,
		     (long long)divisor,
		     (long long)remainder));

		*target_msc = *current_msc + remainder - *current_msc % divisor;
		if (*target_msc <= *current_msc)
			*target_msc += divisor;
	}

	DBG(("%s: target_msc=%lld, current_msc=%lld, immediate?=%d\n",
	     __FUNCTION__, (long long)*target_msc, (long long)*current_msc,
	     *current_msc >= *target_msc - 1));
	return *current_msc >= *target_msc - 1;
}

static bool
sna_dri2_schedule_flip(ClientPtr client, DrawablePtr draw, xf86CrtcPtr crtc,
		       DRI2BufferPtr front, DRI2BufferPtr back,
		       bool immediate, CARD64 *target_msc, CARD64 current_msc,
		       DRI2SwapEventPtr func, void *data)
{
	struct sna *sna = to_sna_from_drawable(draw);
	struct sna_dri2_event *info;

	if (immediate) {
		bool signal = false;
		info = sna->dri2.flip_pending;
		DBG(("%s: performing immediate swap on pipe %d, pending? %d, mode: %d, continuation? %d\n",
		     __FUNCTION__, sna_crtc_pipe(crtc),
		     info != NULL, info ? info->flip_continue : 0,
		     info && info->draw == draw));

		if (info && info->draw == draw) {
			assert(info->type != FLIP);
			assert(info->queued);
			assert(info->front != info->back);
			if (info->front != front) {
				assert(info->front != NULL);
				_sna_dri2_destroy_buffer(sna, draw, info->front);
				info->front = sna_dri2_reference_buffer(front);
			}
			if (info->back != back) {
				assert(info->back != NULL);
				_sna_dri2_destroy_buffer(sna, draw, info->back);
				info->back = sna_dri2_reference_buffer(back);
			}
			assert(info->front != info->back);
			DBG(("%s: executing xchg of pending flip: flip_continue=%d, keepalive=%d, chain?=%d\n", __FUNCTION__, info->flip_continue, info->keepalive, current_msc < *target_msc));
			sna_dri2_xchg(draw, front, back);
			info->keepalive = KEEPALIVE;
			if (xorg_can_triple_buffer() &&
			    current_msc < *target_msc) {
				DBG(("%s: chaining flip\n", __FUNCTION__));
				info->flip_continue = FLIP_THROTTLE;
				goto out;
			} else {
				info->flip_continue = FLIP_COMPLETE;
				signal = info->signal;
				assert(info->draw);
				info->signal = true;
				goto new_back;
			}
		}

		info = sna_dri2_add_event(sna, draw, client, crtc);
		if (info == NULL)
			return false;

		assert(info->crtc == crtc);
		info->event_complete = func;
		info->event_data = data;
		assert(info->draw);
		info->signal = true;

		assert(front != back);
		info->front = sna_dri2_reference_buffer(front);
		info->back = sna_dri2_reference_buffer(back);

		if (sna->dri2.flip_pending) {
			/* We need to first wait (one vblank) for the
			 * async flips to complete before this client
			 * can take over.
			 */
			DBG(("%s: queueing flip after pending completion\n",
			     __FUNCTION__));
			info->type = FLIP;
			sna->dri2.flip_pending = info;
			current_msc++;
		} else if (sna->mode.flip_active) {
			DBG(("%s: %d outstanding flips from old client, queueing\n",
			     __FUNCTION__, sna->mode.flip_active));
			goto queue;
		} else {
			info->type = use_triple_buffer(sna, client, *target_msc == 0);
			if (!sna_dri2_flip(info)) {
				DBG(("%s: flip failed, falling back\n", __FUNCTION__));
				info->signal = false;
				sna_dri2_event_free(info);
				return false;
			}
			assert(get_private(info->front)->bo->active_scanout);
		}

		swap_limit(draw, 1 + (info->type == FLIP_THROTTLE));
		if (info->type >= FLIP_COMPLETE) {
new_back:
			if (!xorg_can_triple_buffer())
				sna_dri2_get_back(sna, draw, back);
			DBG(("%s: fake triple buffering, unblocking client\n", __FUNCTION__));
			frame_swap_complete(info, DRI2_EXCHANGE_COMPLETE);
			assert(info->draw);
			info->signal = signal;
			if (info->type == FLIP_ASYNC)
				sna_dri2_event_free(info);
		}
out:
		DBG(("%s: target_msc=%llu\n", __FUNCTION__, current_msc + 1));
		*target_msc = current_msc + 1;
		return true;
	}

queue:
	if (KEEPALIVE > 1 && sna->dri2.flip_pending) {
		info = sna->dri2.flip_pending;
		info->keepalive = 1;
	}

	info = sna_dri2_add_event(sna, draw, client, crtc);
	if (info == NULL)
		return false;

	assert(info->crtc == crtc);
	info->event_complete = func;
	info->event_data = data;
	assert(info->draw);
	info->signal = true;
	info->type = FLIP;

	assert(front != back);
	info->front = sna_dri2_reference_buffer(front);
	info->back = sna_dri2_reference_buffer(back);

	if (*target_msc <= current_msc + 1 && sna_dri2_flip(info)) {
		*target_msc = current_msc + 1;
	} else {
		/* Account for 1 frame extra pageflip delay */
		if (!sna_wait_vblank(info,
				     draw_target_seq(draw, *target_msc - 1))) {
			info->signal = false;
			sna_dri2_event_free(info);
			return false;
		}
	}

	DBG(("%s: reported target_msc=%llu\n", __FUNCTION__, *target_msc));
	swap_limit(draw, 1);
	return true;
}

static bool has_pending_events(struct sna *sna)
{
	struct pollfd pfd;
	pfd.fd = sna->kgem.fd;
	pfd.events = POLLIN;
	return poll(&pfd, 1, 0) == 1;
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
static int
sna_dri2_schedule_swap(ClientPtr client, DrawablePtr draw, DRI2BufferPtr front,
		       DRI2BufferPtr back, CARD64 *target_msc, CARD64 divisor,
		       CARD64 remainder, DRI2SwapEventPtr func, void *data)
{
	struct sna *sna = to_sna_from_drawable(draw);
	xf86CrtcPtr crtc = NULL;
	struct sna_dri2_event *info = NULL;
	int type = DRI2_EXCHANGE_COMPLETE;
	CARD64 current_msc;
	bool immediate;

	DBG(("%s: draw=%lu %dx%d, pixmap=%ld %dx%d, back=%u (refs=%d/%d, flush=%d, active=%d, stale=%d) , front=%u (refs=%d/%d, flush=%d, active=%d)\n",
	     __FUNCTION__,
	     (long)draw->id, draw->width, draw->height,
	     get_drawable_pixmap(draw)->drawable.serialNumber,
	     get_drawable_pixmap(draw)->drawable.width,
	     get_drawable_pixmap(draw)->drawable.height,
	     get_private(back)->bo->handle,
	     get_private(back)->refcnt,
	     get_private(back)->bo->refcnt,
	     get_private(back)->bo->flush,
	     get_private(back)->bo->active_scanout,
	     get_private(back)->stale,
	     get_private(front)->bo->handle,
	     get_private(front)->refcnt,
	     get_private(front)->bo->refcnt,
	     get_private(front)->bo->flush,
	     get_private(front)->bo->active_scanout));

	DBG(("%s(target_msc=%llu, divisor=%llu, remainder=%llu)\n",
	     __FUNCTION__,
	     (long long)*target_msc,
	     (long long)divisor,
	     (long long)remainder));

	assert(front != back);
	assert(get_private(front) != get_private(back));

	assert(get_private(front)->refcnt);
	assert(get_private(back)->refcnt);

	if (get_private(front)->pixmap != get_drawable_pixmap(draw)) {
		DBG(("%s: decoupled DRI2 front pixmap=%ld, actual pixmap=%ld\n",
		     __FUNCTION__,
		     get_private(front)->pixmap->drawable.serialNumber,
		     get_drawable_pixmap(draw)->drawable.serialNumber));
		goto skip;
	}

	if (get_private(back)->stale) {
		DBG(("%s: stale back buffer\n", __FUNCTION__));
		goto skip;
	}

	assert(get_private(back)->bo != get_private(front)->bo);
	assert(get_private(front)->bo->refcnt);
	assert(get_private(back)->bo->refcnt);

	assert(get_private(front)->bo->active_scanout);
	assert(!get_private(back)->bo->active_scanout);

	if (draw->type != DRAWABLE_PIXMAP) {
		WindowPtr win = (WindowPtr)draw;
		struct dri2_window *priv = dri2_window(win);

		if (priv->front) {
			front = priv->front;
			assert(front->attachment == DRI2BufferFrontLeft);
			assert(get_private(front)->refcnt);
			assert(get_private(front)->pixmap == get_drawable_pixmap(draw));
		}

		if (win->clipList.extents.x2 <= win->clipList.extents.x1 ||
		    win->clipList.extents.y2 <= win->clipList.extents.y1) {
			DBG(("%s: window clipped (%d, %d), (%d, %d)\n",
			     __FUNCTION__,
			     win->clipList.extents.x1,
			     win->clipList.extents.y1,
			     win->clipList.extents.x2,
			     win->clipList.extents.y2));
			goto skip;
		}
	}

	DBG(("%s: using front handle=%d, active_scanout?=%d, flush?=%d\n", __FUNCTION__, get_private(front)->bo->handle, get_private(front)->bo->active_scanout, sna_pixmap_from_drawable(draw)->flush));
	assert(get_private(front)->bo->active_scanout);
	assert(sna_pixmap_from_drawable(draw)->flush);

	/* Drawable not displayed... just complete the swap */
	if ((sna->flags & SNA_NO_WAIT) == 0)
		crtc = sna_dri2_get_crtc(draw);
	if (crtc == NULL) {
		DBG(("%s: off-screen, immediate update\n", __FUNCTION__));
		goto blit;
	}

	assert(draw->type != DRAWABLE_PIXMAP);

	while (dri2_chain(draw) && has_pending_events(sna)) {
		DBG(("%s: flushing pending events\n", __FUNCTION__));
		sna_mode_wakeup(sna);
	}

	immediate = immediate_swap(sna, draw, crtc,
				   target_msc, divisor, remainder,
				   &current_msc);

	if (can_flip(sna, draw, front, back, crtc) &&
	    sna_dri2_schedule_flip(client, draw, crtc, front, back,
				  immediate, target_msc, current_msc,
				  func, data))
		return TRUE;

	info = sna_dri2_add_event(sna, draw, client, crtc);
	if (!info)
		goto blit;

	assert(info->crtc == crtc);
	info->event_complete = func;
	info->event_data = data;
	assert(info->draw);
	info->signal = true;

	assert(front != back);
	info->front = sna_dri2_reference_buffer(front);
	info->back = sna_dri2_reference_buffer(back);

	if (immediate) {
		bool sync = current_msc < *target_msc;
		sna_dri2_immediate_blit(sna, info, sync);
		*target_msc = current_msc + sync;
		DBG(("%s: reported target_msc=%llu\n",
		     __FUNCTION__, *target_msc));
		return TRUE;
	}

	info->type = SWAP;
	if (*target_msc <= current_msc + 1) {
		DBG(("%s: performing blit before queueing\n", __FUNCTION__));
		__sna_dri2_copy_event(info, DRI2_SYNC);
		info->type = SWAP_COMPLETE;
		if (!sna_next_vblank(info))
			goto fake;

		DBG(("%s: reported target_msc=%llu\n",
		     __FUNCTION__, *target_msc));
		*target_msc = current_msc + 1;
		swap_limit(draw, 2);
	} else {
		if (!sna_wait_vblank(info,
				     draw_target_seq(draw, *target_msc - 1)))
			goto blit;

		DBG(("%s: reported target_msc=%llu (in)\n",
		     __FUNCTION__, *target_msc));
		swap_limit(draw, 1);
	}

	return TRUE;

blit:
	DBG(("%s -- blit\n", __FUNCTION__));
	if (can_xchg(sna, draw, front, back)) {
		sna_dri2_xchg(draw, front, back);
	} else {
		__sna_dri2_copy_region(sna, draw, NULL, back, front, 0);
		front->flags = back->flags;
		type = DRI2_BLIT_COMPLETE;
	}
	if (draw->type == DRAWABLE_PIXMAP)
		goto fake;
skip:
	DBG(("%s: unable to show frame, unblocking client\n", __FUNCTION__));
	if (crtc == NULL && (sna->flags & SNA_NO_WAIT) == 0)
		crtc = sna_primary_crtc(sna);
	if (crtc && sna_crtc_is_on(crtc)) {
		if (info == NULL)
			info = sna_dri2_add_event(sna, draw, client, crtc);
		if (info != dri2_chain(draw))
			goto fake;

		assert(info->crtc == crtc);

		info->type = SWAP_COMPLETE;
		info->event_complete = func;
		info->event_data = data;
		assert(info->draw);
		info->signal = true;

		if (info->front == NULL)
			info->front = sna_dri2_reference_buffer(front);
		if (info->back == NULL)
			info->back = sna_dri2_reference_buffer(back);

		if (!sna_next_vblank(info))
			goto fake;

		swap_limit(draw, 1);
	} else {
fake:
		/* XXX Use a Timer to throttle the client? */
		fake_swap_complete(sna, client, draw, crtc, type, func, data);
		if (info) {
			assert(info->draw);
			info->signal = false;
			sna_dri2_event_free(info);
		}
	}
	DBG(("%s: reported target_msc=%llu (in)\n", __FUNCTION__, *target_msc));
	return TRUE;
}

/*
 * Get current frame count and frame count timestamp, based on drawable's
 * crtc.
 */
static int
sna_dri2_get_msc(DrawablePtr draw, CARD64 *ust, CARD64 *msc)
{
	struct sna *sna = to_sna_from_drawable(draw);
	xf86CrtcPtr crtc = sna_dri2_get_crtc(draw);
	const struct ust_msc *swap;
	union drm_wait_vblank vbl;

	DBG(("%s(draw=%ld, pipe=%d)\n", __FUNCTION__, draw->id,
	     crtc ? sna_crtc_pipe(crtc) : -1));

	/* Drawable not displayed, make up a *monotonic* value */
	if (crtc == NULL)
		crtc = sna_primary_crtc(sna);
	if (crtc == NULL)
		return FALSE;

	if (sna_query_vblank(sna, crtc, &vbl) == 0)
		sna_crtc_record_vblank(crtc, &vbl);

	swap = sna_crtc_last_swap(crtc);
	*msc = draw_current_msc(draw, crtc, swap->msc);
	*ust = ust64(swap->tv_sec, swap->tv_usec);
	DBG(("%s: msc=%llu [raw=%llu], ust=%llu\n", __FUNCTION__,
	     (long long)*msc, swap->msc, (long long)*ust));
	return TRUE;
}

/*
 * Request a DRM event when the requested conditions will be satisfied.
 *
 * We need to handle the event and ask the server to wake up the client when
 * we receive it.
 */
static int
sna_dri2_schedule_wait_msc(ClientPtr client, DrawablePtr draw, CARD64 target_msc,
			   CARD64 divisor, CARD64 remainder)
{
	struct sna *sna = to_sna_from_drawable(draw);
	struct sna_dri2_event *info = NULL;
	xf86CrtcPtr crtc;
	CARD64 current_msc;
	const struct ust_msc *swap;

	crtc = sna_dri2_get_crtc(draw);
	DBG(("%s(pipe=%d, target_msc=%llu, divisor=%llu, rem=%llu)\n",
	     __FUNCTION__, crtc ? sna_crtc_pipe(crtc) : -1,
	     (long long)target_msc,
	     (long long)divisor,
	     (long long)remainder));

	/* Drawable not visible, return immediately */
	if (crtc == NULL)
		crtc = sna_primary_crtc(sna);
	if (crtc == NULL)
		return FALSE;

	current_msc = get_current_msc(sna, draw, crtc);

	/* If target_msc already reached or passed, set it to
	 * current_msc to ensure we return a reasonable value back
	 * to the caller. This keeps the client from continually
	 * sending us MSC targets from the past by forcibly updating
	 * their count on this call.
	 */
	if (divisor == 0 && current_msc >= target_msc)
		goto out_complete;

	info = sna_dri2_add_event(sna, draw, client, crtc);
	if (!info)
		goto out_complete;

	assert(info->crtc == crtc);
	info->type = WAITMSC;

	/*
	 * If divisor is zero, or current_msc is smaller than target_msc,
	 * we just need to make sure target_msc passes before waking up the
	 * client. Otherwise, compute the next msc to match divisor/remainder.
	 */
	if (divisor && current_msc >= target_msc) {
		DBG(("%s: missed target, queueing event for next: current=%lld, target=%lld, divisor=%lld, remainder=%lld\n",
		     __FUNCTION__,
		     (long long)current_msc,
		     (long long)target_msc,
		     (long long)divisor,
		     (long long)remainder));
		target_msc = current_msc + remainder - current_msc % divisor;
		if (target_msc <= current_msc)
			target_msc += divisor;
	}

	if (!sna_wait_vblank(info, draw_target_seq(draw, target_msc)))
		goto out_free_info;

	DRI2BlockClient(client, draw);
	return TRUE;

out_free_info:
	sna_dri2_event_free(info);
out_complete:
	swap = sna_crtc_last_swap(crtc);
	DRI2WaitMSCComplete(client, draw,
			    draw_current_msc(draw, crtc, swap->msc),
			    swap->tv_sec, swap->tv_usec);
	return TRUE;
}
#else
void sna_dri2_destroy_window(WindowPtr win) { }
void sna_dri2_decouple_window(WindowPtr win) { }
#endif

static bool has_i830_dri(void)
{
	return access(DRI_DRIVER_PATH "/i830_dri.so", R_OK) == 0;
}

static int
namecmp(const char *s1, const char *s2)
{
	char c1, c2;

	if (!s1 || *s1 == 0) {
		if (!s2 || *s2 == 0)
			return 0;
		else
			return 1;
	}

	while (*s1 == '_' || *s1 == ' ' || *s1 == '\t')
		s1++;

	while (*s2 == '_' || *s2 == ' ' || *s2 == '\t')
		s2++;

	c1 = isupper(*s1) ? tolower(*s1) : *s1;
	c2 = isupper(*s2) ? tolower(*s2) : *s2;
	while (c1 == c2) {
		if (c1 == '\0')
			return 0;

		s1++;
		while (*s1 == '_' || *s1 == ' ' || *s1 == '\t')
			s1++;

		s2++;
		while (*s2 == '_' || *s2 == ' ' || *s2 == '\t')
			s2++;

		c1 = isupper(*s1) ? tolower(*s1) : *s1;
		c2 = isupper(*s2) ? tolower(*s2) : *s2;
	}

	return c1 - c2;
}

static bool is_level(const char **str)
{
	const char *s = *str;
	char *end;
	unsigned val;

	if (s == NULL || *s == '\0')
		return true;

	if (namecmp(s, "on") == 0)
		return true;
	if (namecmp(s, "true") == 0)
		return true;
	if (namecmp(s, "yes") == 0)
		return true;

	if (namecmp(s, "0") == 0)
		return true;
	if (namecmp(s, "off") == 0)
		return true;
	if (namecmp(s, "false") == 0)
		return true;
	if (namecmp(s, "no") == 0)
		return true;

	val = strtoul(s, &end, 0);
	if (val && *end == '\0')
		return true;
	if (val && *end == ':')
		*str = end + 1;
	return false;
}

static const char *options_get_dri(struct sna *sna)
{
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,99,901,0)
	return xf86GetOptValString(sna->Options, OPTION_DRI);
#else
	return NULL;
#endif
}

static const char *dri_driver_name(struct sna *sna)
{
	const char *s = options_get_dri(sna);

	if (is_level(&s)) {
		if (sna->kgem.gen < 030)
			return has_i830_dri() ? "i830" : "i915";
		else if (sna->kgem.gen < 040)
			return "i915";
		else
			return "i965";
	}

	return s;
}

bool sna_dri2_open(struct sna *sna, ScreenPtr screen)
{
	DRI2InfoRec info;
	int major = 1, minor = 0;
#if DRI2INFOREC_VERSION >= 4
	const char *driverNames[2];
#endif

	DBG(("%s()\n", __FUNCTION__));

	if (wedged(sna)) {
		xf86DrvMsg(sna->scrn->scrnIndex, X_WARNING,
			   "loading DRI2 whilst acceleration is disabled.\n");
	}

	if (xf86LoaderCheckSymbol("DRI2Version"))
		DRI2Version(&major, &minor);

	if (minor < 1) {
		xf86DrvMsg(sna->scrn->scrnIndex, X_WARNING,
			   "DRI2 requires DRI2 module version 1.1.0 or later\n");
		return false;
	}

	memset(&info, '\0', sizeof(info));
	info.fd = sna->kgem.fd;
	info.driverName = dri_driver_name(sna);
	info.deviceName = intel_get_master_name(sna->dev);

	DBG(("%s: loading dri driver '%s' [gen=%d] for device '%s'\n",
	     __FUNCTION__, info.driverName, sna->kgem.gen, info.deviceName));

#if DRI2INFOREC_VERSION == 2
	/* The ABI between 2 and 3 was broken so we could get rid of
	 * the multi-buffer alloc functions.  Make sure we indicate the
	 * right version so DRI2 can reject us if it's version 3 or above. */
	info.version = 2;
#else
	info.version = 3;
#endif
	info.CreateBuffer = sna_dri2_create_buffer;
	info.DestroyBuffer = sna_dri2_destroy_buffer;

	info.CopyRegion = sna_dri2_copy_region;
#if DRI2INFOREC_VERSION >= 4
	info.version = 4;
	info.ScheduleSwap = sna_dri2_schedule_swap;
	info.GetMSC = sna_dri2_get_msc;
	info.ScheduleWaitMSC = sna_dri2_schedule_wait_msc;
	info.numDrivers = 2;
	info.driverNames = driverNames;
	driverNames[0] = info.driverName;
	driverNames[1] = "va_gl";
#endif

#if DRI2INFOREC_VERSION >= 6
	if (xorg_can_triple_buffer()) {
		DBG(("%s: enabling Xorg triple buffering\n", __FUNCTION__));
		info.version = 6;
		info.SwapLimitValidate = sna_dri2_swap_limit_validate;
		info.ReuseBufferNotify = sna_dri2_reuse_buffer;
	}
#endif

#if USE_ASYNC_SWAP
	DBG(("%s: enabled async swap and buffer age\n", __FUNCTION__));
	info.version = 10;
	info.scheduleSwap0 = 1;
	info.bufferAge = 1;
#endif

	return DRI2ScreenInit(screen, &info);
}

void sna_dri2_close(struct sna *sna, ScreenPtr screen)
{
	DBG(("%s()\n", __FUNCTION__));
	DRI2CloseScreen(screen);
}
