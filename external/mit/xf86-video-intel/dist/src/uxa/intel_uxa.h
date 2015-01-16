/*
 * Copyright © 2014 Keith Packard
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

#ifndef _INTEL_UXA_H_
#define _INTEL_UXA_H_

#include "intel_video.h"
#include "uxa.h"

struct intel_uxa_pixmap {
	dri_bo *bo;

	struct list batch;

	uint8_t tiling;
	int8_t busy :2;
	uint8_t dirty :1;
	uint8_t offscreen :1;
	uint8_t pinned :5;
#define PIN_SCANOUT 0x1
#define PIN_DRI2 0x2
#define PIN_DRI3 0x4
#define PIN_PRIME 0x8
};

#if HAS_DEVPRIVATEKEYREC
extern DevPrivateKeyRec uxa_pixmap_index;
#else
extern int uxa_pixmap_index;
#endif

static inline struct intel_uxa_pixmap *intel_uxa_get_pixmap_private(PixmapPtr pixmap)
{
#if HAS_DEVPRIVATEKEYREC
	return dixGetPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#else
	return dixLookupPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#endif
}

static inline Bool intel_uxa_pixmap_is_busy(struct intel_uxa_pixmap *priv)
{
	if (priv->busy == -1)
		priv->busy = drm_intel_bo_busy(priv->bo);
	return priv->busy;
}

static inline void intel_uxa_set_pixmap_private(PixmapPtr pixmap, struct intel_uxa_pixmap *intel)
{
	dixSetPrivate(&pixmap->devPrivates, &uxa_pixmap_index, intel);
}

static inline Bool intel_uxa_pixmap_is_dirty(PixmapPtr pixmap)
{
	return pixmap && intel_uxa_get_pixmap_private(pixmap)->dirty;
}

static inline Bool intel_uxa_pixmap_tiled(PixmapPtr pixmap)
{
	return intel_uxa_get_pixmap_private(pixmap)->tiling != I915_TILING_NONE;
}

dri_bo *intel_uxa_get_pixmap_bo(PixmapPtr pixmap);
void intel_uxa_set_pixmap_bo(PixmapPtr pixmap, dri_bo * bo);

Bool intel_uxa_init(ScreenPtr pScreen);
Bool intel_uxa_create_screen_resources(ScreenPtr pScreen);
void intel_uxa_block_handler(intel_screen_private *intel);

static inline Bool intel_uxa_pixmap_is_offscreen(PixmapPtr pixmap)
{
	struct intel_uxa_pixmap *priv = intel_uxa_get_pixmap_private(pixmap);
	return priv && priv->offscreen;
}

/* Batchbuffer support macros and functions */
#include "intel_batchbuffer.h"

/* I830 specific functions */
extern void IntelEmitInvarientState(ScrnInfoPtr scrn);
extern void I830EmitInvarientState(ScrnInfoPtr scrn);
extern void I915EmitInvarientState(ScrnInfoPtr scrn);

extern void I830EmitFlush(ScrnInfoPtr scrn);

/* i830_render.c */
Bool i830_check_composite(int op,
			  PicturePtr sourcec, PicturePtr mask, PicturePtr dest,
			  int width, int height);
Bool i830_check_composite_target(PixmapPtr pixmap);
Bool i830_check_composite_texture(ScreenPtr screen, PicturePtr picture);
Bool i830_prepare_composite(int op, PicturePtr sourcec, PicturePtr mask,
			    PicturePtr dest, PixmapPtr sourcecPixmap,
			    PixmapPtr maskPixmap, PixmapPtr destPixmap);
void i830_composite(PixmapPtr dest, int srcX, int srcY,
		    int maskX, int maskY, int dstX, int dstY, int w, int h);
void i830_vertex_flush(intel_screen_private *intel);

/* i915_render.c */
Bool i915_check_composite(int op,
			  PicturePtr sourcec, PicturePtr mask, PicturePtr dest,
			  int width, int height);
Bool i915_check_composite_target(PixmapPtr pixmap);
Bool i915_check_composite_texture(ScreenPtr screen, PicturePtr picture);
Bool i915_prepare_composite(int op, PicturePtr sourcec, PicturePtr mask,
			    PicturePtr dest, PixmapPtr sourcecPixmap,
			    PixmapPtr maskPixmap, PixmapPtr destPixmap);
void i915_composite(PixmapPtr dest, int srcX, int srcY,
		    int maskX, int maskY, int dstX, int dstY, int w, int h);
void i915_vertex_flush(intel_screen_private *intel);
void i915_batch_commit_notify(intel_screen_private *intel);
void i830_batch_commit_notify(intel_screen_private *intel);
/* i965_render.c */
unsigned int gen4_render_state_size(ScrnInfoPtr scrn);
void gen4_render_state_init(ScrnInfoPtr scrn);
void gen4_render_state_cleanup(ScrnInfoPtr scrn);
Bool i965_check_composite(int op,
			  PicturePtr sourcec, PicturePtr mask, PicturePtr dest,
			  int width, int height);
Bool i965_check_composite_texture(ScreenPtr screen, PicturePtr picture);
Bool i965_prepare_composite(int op, PicturePtr sourcec, PicturePtr mask,
			    PicturePtr dest, PixmapPtr sourcecPixmap,
			    PixmapPtr maskPixmap, PixmapPtr destPixmap);
void i965_composite(PixmapPtr dest, int srcX, int srcY,
		    int maskX, int maskY, int dstX, int dstY, int w, int h);

void i965_vertex_flush(intel_screen_private *intel);
void i965_batch_flush(intel_screen_private *intel);
void i965_batch_commit_notify(intel_screen_private *intel);

/* i965_3d.c */
void gen6_upload_invariant_states(intel_screen_private *intel);
void gen6_upload_viewport_state_pointers(intel_screen_private *intel,
					 drm_intel_bo *cc_vp_bo);
void gen7_upload_viewport_state_pointers(intel_screen_private *intel,
					 drm_intel_bo *cc_vp_bo);
void gen6_upload_urb(intel_screen_private *intel);
void gen7_upload_urb(intel_screen_private *intel);
void gen6_upload_cc_state_pointers(intel_screen_private *intel,
				   drm_intel_bo *blend_bo, drm_intel_bo *cc_bo,
				   drm_intel_bo *depth_stencil_bo,
				   uint32_t blend_offset);
void gen7_upload_cc_state_pointers(intel_screen_private *intel,
				   drm_intel_bo *blend_bo, drm_intel_bo *cc_bo,
				   drm_intel_bo *depth_stencil_bo,
				   uint32_t blend_offset);
void gen6_upload_sampler_state_pointers(intel_screen_private *intel,
					drm_intel_bo *sampler_bo);
void gen7_upload_sampler_state_pointers(intel_screen_private *intel,
					drm_intel_bo *sampler_bo);
void gen7_upload_bypass_states(intel_screen_private *intel);
void gen6_upload_gs_state(intel_screen_private *intel);
void gen6_upload_vs_state(intel_screen_private *intel);
void gen6_upload_clip_state(intel_screen_private *intel);
void gen6_upload_sf_state(intel_screen_private *intel, int num_sf_outputs, int read_offset);
void gen7_upload_sf_state(intel_screen_private *intel, int num_sf_outputs, int read_offset);
void gen6_upload_binding_table(intel_screen_private *intel, uint32_t ps_binding_table_offset);
void gen7_upload_binding_table(intel_screen_private *intel, uint32_t ps_binding_table_offset);
void gen6_upload_depth_buffer_state(intel_screen_private *intel);
void gen7_upload_depth_buffer_state(intel_screen_private *intel);

Bool intel_uxa_transform_is_affine(PictTransformPtr t);

Bool
intel_uxa_get_transformed_coordinates(int x, int y, PictTransformPtr transform,
				 float *x_out, float *y_out);

Bool
intel_uxa_get_transformed_coordinates_3d(int x, int y, PictTransformPtr transform,
				    float *x_out, float *y_out, float *z_out);

static inline void
intel_uxa_debug_fallback(ScrnInfoPtr scrn, const char *format, ...) _X_ATTRIBUTE_PRINTF(2, 3);

static inline void
intel_uxa_debug_fallback(ScrnInfoPtr scrn, const char *format, ...)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	va_list ap;

	va_start(ap, format);
	if (intel->fallback_debug) {
		xf86DrvMsg(scrn->scrnIndex, X_INFO, "fallback: ");
		LogVMessageVerb(X_INFO, 1, format, ap);
	}
	va_end(ap);
}

static inline Bool
intel_uxa_check_pitch_2d(PixmapPtr pixmap)
{
	uint32_t pitch = intel_pixmap_pitch(pixmap);
	if (pitch > KB(32)) {
		ScrnInfoPtr scrn = xf86ScreenToScrn(pixmap->drawable.pScreen);
		intel_uxa_debug_fallback(scrn, "pitch exceeds 2d limit 32K\n");
		return FALSE;
	}
	return TRUE;
}

/* For pre-965 chip only, as they have 8KB limit for 3D */
static inline Bool
intel_uxa_check_pitch_3d(PixmapPtr pixmap)
{
	uint32_t pitch = intel_pixmap_pitch(pixmap);
	if (pitch > KB(8)) {
		ScrnInfoPtr scrn = xf86ScreenToScrn(pixmap->drawable.pScreen);
		intel_uxa_debug_fallback(scrn, "pitch exceeds 3d limit 8K\n");
		return FALSE;
	}
	return TRUE;
}

/**
 * Little wrapper around drm_intel_bo_reloc to return the initial value you
 * should stuff into the relocation entry.
 *
 * If only we'd done this before settling on the library API.
 */
static inline uint32_t
intel_uxa_emit_reloc(drm_intel_bo * bo, uint32_t offset,
		 drm_intel_bo * target_bo, uint32_t target_offset,
		 uint32_t read_domains, uint32_t write_domain)
{
	drm_intel_bo_emit_reloc(bo, offset, target_bo, target_offset,
				read_domains, write_domain);

	return target_bo->offset + target_offset;
}

static inline drm_intel_bo *intel_uxa_bo_alloc_for_data(intel_screen_private *intel,
						    const void *data,
						    unsigned int size,
						    const char *name)
{
	drm_intel_bo *bo;
	int ret;

	bo = drm_intel_bo_alloc(intel->bufmgr, name, size, 4096);
	assert(bo);

	ret = drm_intel_bo_subdata(bo, 0, size, data);
	assert(ret == 0);

	return bo;
	(void)ret;
}

void intel_uxa_debug_flush(ScrnInfoPtr scrn);


Bool intel_uxa_get_aperture_space(ScrnInfoPtr scrn, drm_intel_bo ** bo_table,
                                  int num_bos);

XF86VideoAdaptorPtr intel_uxa_video_setup_image_textured(ScreenPtr screen);

void I915DisplayVideoTextured(ScrnInfoPtr scrn,
			      intel_adaptor_private *adaptor_priv,
			      int id, RegionPtr dstRegion, short width,
			      short height, int video_pitch, int video_pitch2,
			      short src_w, short src_h,
			      short drw_w, short drw_h, PixmapPtr pixmap);

void I965DisplayVideoTextured(ScrnInfoPtr scrn,
			      intel_adaptor_private *adaptor_priv,
			      int id, RegionPtr dstRegion, short width,
			      short height, int video_pitch, int video_pitch2,
			      short src_w, short src_h,
			      short drw_w, short drw_h, PixmapPtr pixmap);

void Gen6DisplayVideoTextured(ScrnInfoPtr scrn,
			      intel_adaptor_private *adaptor_priv,
			      int id, RegionPtr dstRegion, short width,
			      short height, int video_pitch, int video_pitch2,
			      short src_w, short src_h,
			      short drw_w, short drw_h, PixmapPtr pixmap);

void i965_free_video(ScrnInfoPtr scrn);

#endif /* _INTEL_UXA_H_ */
