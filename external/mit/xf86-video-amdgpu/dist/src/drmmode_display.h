/*
 * Copyright © 2007 Red Hat, Inc.
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
 *     Dave Airlie <airlied@redhat.com>
 *
 */
#ifndef DRMMODE_DISPLAY_H
#define DRMMODE_DISPLAY_H

#include "xf86drmMode.h"
#ifdef HAVE_LIBUDEV
#include "libudev.h"
#endif

#include "amdgpu_drm_queue.h"
#include "amdgpu_probe.h"
#include "amdgpu.h"

#ifndef DRM_CAP_TIMESTAMP_MONOTONIC
#define DRM_CAP_TIMESTAMP_MONOTONIC 0x6
#endif

typedef struct {
	unsigned fb_id;
	drmModeFBPtr mode_fb;
	int cpp;
	ScrnInfoPtr scrn;
#ifdef HAVE_LIBUDEV
	struct udev_monitor *uevent_monitor;
	InputHandlerProc uevent_handler;
#endif
	drmEventContext event_context;
	int count_crtcs;

	Bool delete_dp_12_displays;

	Bool dri2_flipping;
	Bool present_flipping;
} drmmode_rec, *drmmode_ptr;

typedef struct {
	int fd;
	unsigned old_fb_id;
	int flip_count;
	void *event_data;
	unsigned int fe_frame;
	uint64_t fe_usec;
	xf86CrtcPtr fe_crtc;
	amdgpu_drm_handler_proc handler;
	amdgpu_drm_abort_proc abort;
} drmmode_flipdata_rec, *drmmode_flipdata_ptr;

struct drmmode_scanout {
	struct amdgpu_buffer *bo;
	PixmapPtr pixmap;
	unsigned fb_id;
	int width, height;
};

typedef struct {
	drmmode_ptr drmmode;
	drmModeCrtcPtr mode_crtc;
	int hw_id;
	struct amdgpu_buffer *cursor_buffer;
	struct drmmode_scanout rotate;
	struct drmmode_scanout scanout[2];
	struct drmmode_scanout scanout_destroy[2];
	DamagePtr scanout_damage;
	RegionRec scanout_last_region;
	unsigned scanout_id;
	Bool scanout_update_pending;
	int dpms_mode;
	/* For when a flip is pending when DPMS off requested */
	int pending_dpms_mode;
	CARD64 dpms_last_ust;
	uint32_t dpms_last_seq;
	int dpms_last_fps;
	uint32_t interpolated_vblanks;

	/* Modeset needed for DPMS on */
	Bool need_modeset;
	/* A flip is pending for this CRTC */
	Bool flip_pending;
} drmmode_crtc_private_rec, *drmmode_crtc_private_ptr;

typedef struct {
	drmModePropertyPtr mode_prop;
	uint64_t value;
	int num_atoms;		/* if range prop, num_atoms == 1; if enum prop, num_atoms == num_enums + 1 */
	Atom *atoms;
} drmmode_prop_rec, *drmmode_prop_ptr;

typedef struct {
	drmmode_ptr drmmode;
	int output_id;
	drmModeConnectorPtr mode_output;
	drmModeEncoderPtr *mode_encoders;
	drmModePropertyBlobPtr edid_blob;
	int dpms_enum_id;
	int num_props;
	drmmode_prop_ptr props;
	int enc_mask;
	int enc_clone_mask;
} drmmode_output_private_rec, *drmmode_output_private_ptr;


enum drmmode_flip_sync {
    FLIP_VSYNC,
    FLIP_ASYNC,
};


extern Bool drmmode_pre_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int cpp);
extern void drmmode_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
extern void drmmode_fini(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
extern void drmmode_set_cursor(ScrnInfoPtr scrn, drmmode_ptr drmmode, int id,
			       struct amdgpu_buffer *bo);
void drmmode_adjust_frame(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int x, int y);
extern Bool drmmode_set_desired_modes(ScrnInfoPtr pScrn, drmmode_ptr drmmode,
				      Bool set_hw);
extern void drmmode_copy_fb(ScrnInfoPtr pScrn, drmmode_ptr drmmode);
extern Bool drmmode_setup_colormap(ScreenPtr pScreen, ScrnInfoPtr pScrn);

extern void drmmode_scanout_free(ScrnInfoPtr scrn);

extern void drmmode_uevent_init(ScrnInfoPtr scrn, drmmode_ptr drmmode);
extern void drmmode_uevent_fini(ScrnInfoPtr scrn, drmmode_ptr drmmode);

extern int drmmode_get_crtc_id(xf86CrtcPtr crtc);
extern int drmmode_get_pitch_align(ScrnInfoPtr scrn, int bpe);
extern void drmmode_clear_pending_flip(xf86CrtcPtr crtc);
Bool amdgpu_do_pageflip(ScrnInfoPtr scrn, ClientPtr client,
			PixmapPtr new_front, uint64_t id, void *data,
			int ref_crtc_hw_id, amdgpu_drm_handler_proc handler,
			amdgpu_drm_abort_proc abort,
			enum drmmode_flip_sync flip_sync);
int drmmode_crtc_get_ust_msc(xf86CrtcPtr crtc, CARD64 *ust, CARD64 *msc);
int drmmode_get_current_ust(int drm_fd, CARD64 * ust);

#endif
