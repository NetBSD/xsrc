/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright © 2002 David Dawes

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keith@tungstengraphics.com>
 *   David Dawes <dawes@xfree86.org>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if 0
#define I830DEBUG
#endif

#include <stdint.h>

#ifndef REMAP_RESERVED
#define REMAP_RESERVED 0
#endif

#ifndef _I830_H_
#define _I830_H_

#include "xorg-server.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86Cursor.h"
#include "xf86xv.h"
#include "xf86Crtc.h"
#include "xf86RandR12.h"

#include "xorg-server.h"
#include <pciaccess.h>

#define _XF86DRI_SERVER_
#include "drm.h"
#include "dri2.h"
#include "intel_bufmgr.h"
#include "i915_drm.h"

#include "intel_driver.h"
#include "intel_options.h"
#include "intel_list.h"
#include "compat-api.h"

#if HAVE_UDEV
#include <libudev.h>
#endif

#if HAVE_DRI3
#include "misync.h"
#endif

/* remain compatible to xorg-server 1.6 */
#ifndef MONITOR_EDID_COMPLETE_RAWDATA
#define MONITOR_EDID_COMPLETE_RAWDATA EDID_COMPLETE_RAWDATA
#endif

#if XF86_CRTC_VERSION >= 5
#define INTEL_PIXMAP_SHARING 1
#endif

#define MAX_PIPES 4 /* consider making all users dynamic */

#include "common.h"

#define PITCH_NONE 0

/** enumeration of 3d consumers so some can maintain invariant state. */
enum last_3d {
	LAST_3D_OTHER,
	LAST_3D_VIDEO,
	LAST_3D_RENDER,
	LAST_3D_ROTATION
};

enum dri_type {
	DRI_DISABLED,
	DRI_NONE,
	DRI_ACTIVE
};

typedef struct intel_screen_private {
	ScrnInfoPtr scrn;
	struct intel_device *dev;
	int cpp;

#define RENDER_BATCH			I915_EXEC_RENDER
#define BLT_BATCH			I915_EXEC_BLT
	unsigned int current_batch;

	void *modes;
	drm_intel_bo *front_buffer, *back_buffer;
	long front_pitch, front_tiling;

	dri_bufmgr *bufmgr;

#if USE_UXA
	uint32_t batch_ptr[4096];
	/** Byte offset in batch_ptr for the next dword to be emitted. */
	unsigned int batch_used;
	/** Position in batch_ptr at the start of the current BEGIN_BATCH */
	unsigned int batch_emit_start;
	/** Number of bytes to be emitted in the current BEGIN_BATCH. */
	uint32_t batch_emitting;
	dri_bo *batch_bo, *last_batch_bo[2];
	/** Whether we're in a section of code that can't tolerate flushing */
	Bool in_batch_atomic;
	/** Ending batch_used that was verified by intel_start_batch_atomic() */
	int batch_atomic_limit;
	struct list batch_pixmaps;
	drm_intel_bo *wa_scratch_bo;
	OsTimerPtr cache_expire;
#endif

	/* For Xvideo */
	Bool use_overlay;
#ifdef INTEL_XVMC
	/* For XvMC */
	Bool XvMCEnabled;
#endif

	CreateScreenResourcesProcPtr CreateScreenResources;

	Bool shadow_present;

	unsigned int tiling;
#define INTEL_TILING_FB		0x1
#define INTEL_TILING_2D		0x2
#define INTEL_TILING_3D		0x4
#define INTEL_TILING_ALL (~0)

	Bool swapbuffers_wait;
	Bool has_relaxed_fencing;

	int Chipset;
	EntityInfoPtr pEnt;
	const struct intel_device_info *info;

	unsigned int BR[20];
	unsigned int BR_tiling[2];

	CloseScreenProcPtr CloseScreen;

	void (*context_switch) (struct intel_screen_private *intel,
				int new_mode);
	void (*vertex_flush) (struct intel_screen_private *intel);
	void (*batch_flush) (struct intel_screen_private *intel);
	void (*batch_commit_notify) (struct intel_screen_private *intel);

#if USE_UXA
	struct _UxaDriver *uxa_driver;
	int uxa_flags;
#endif
	Bool need_sync;
	int accel_pixmap_offset_alignment;
	int accel_max_x;
	int accel_max_y;
	int max_bo_size;
	int max_gtt_map_size;
	int max_tiling_size;

	Bool XvDisabled;	/* Xv disabled in PreInit. */
	Bool XvEnabled;		/* Xv enabled for this generation. */
	Bool XvPreferOverlay;

	int colorKey;
	XF86VideoAdaptorPtr adaptor;
#if !HAVE_NOTIFY_FD
	ScreenBlockHandlerProcPtr BlockHandler;
#endif
	Bool overlayOn;

	struct {
		drm_intel_bo *gen4_vs_bo;
		drm_intel_bo *gen4_sf_bo;
		drm_intel_bo *gen4_wm_packed_bo;
		drm_intel_bo *gen4_wm_planar_bo;
		drm_intel_bo *gen4_cc_bo;
		drm_intel_bo *gen4_cc_vp_bo;
		drm_intel_bo *gen4_sampler_bo;
		drm_intel_bo *gen4_sip_kernel_bo;
		drm_intel_bo *wm_prog_packed_bo;
		drm_intel_bo *wm_prog_planar_bo;
		drm_intel_bo *gen6_blend_bo;
		drm_intel_bo *gen6_depth_stencil_bo;
	} video;

#if USE_UXA
	/* Render accel state */
	float scale_units[2][2];
	/** Transform pointers for src/mask, or NULL if identity */
	PictTransform *transform[2];

	PixmapPtr render_source, render_mask, render_dest;
	PicturePtr render_source_picture, render_mask_picture, render_dest_picture;
	Bool needs_3d_invariant;
	Bool needs_render_state_emit;
	Bool needs_render_vertex_emit;

	/* i830 render accel state */
	uint32_t render_dest_format;
	uint32_t cblend, ablend, s8_blendctl;

	/* i915 render accel state */
	PixmapPtr texture[2];
	uint32_t mapstate[6];
	uint32_t samplerstate[6];

	struct {
		int op;
		uint32_t dst_format;
	} i915_render_state;

	struct {
		int num_sf_outputs;
		int drawrect;
		uint32_t blend;
		dri_bo *samplers;
		dri_bo *kernel;
	} gen6_render_state;

	uint32_t prim_offset;
	void (*prim_emit)(struct intel_screen_private *intel,
			  int srcX, int srcY,
			  int maskX, int maskY,
			  int dstX, int dstY,
			  int w, int h);
	int floats_per_vertex;
	int last_floats_per_vertex;
	uint16_t vertex_offset;
	uint16_t vertex_count;
	uint16_t vertex_index;
	uint16_t vertex_used;
	uint32_t vertex_id;
	float vertex_ptr[4*1024];
	dri_bo *vertex_bo;

	uint8_t surface_data[16*1024];
	uint16_t surface_used;
	uint16_t surface_table;
	uint32_t surface_reloc;
	dri_bo *surface_bo;

	/* 965 render acceleration state */
	struct gen4_render_state *gen4_render_state;
#endif

	/* DRI enabled this generation. */
	enum dri_type dri2, dri3;
	int drmSubFD;
	char *deviceName;

	Bool use_pageflipping;
	Bool use_triple_buffer;
	Bool force_fallback;
	Bool has_kernel_flush;
	Bool needs_flush;

	/* Broken-out options. */
	OptionInfoPtr Options;

	/* Driver phase/state information */
	Bool suspended;

	enum last_3d last_3d;

	/**
	 * User option to print acceleration fallback info to the server log.
	 */
	Bool fallback_debug;
	unsigned debug_flush;
#if HAVE_UDEV
	struct udev_monitor *uevent_monitor;
	pointer uevent_handler;
#endif
	Bool has_prime_vmap_flush;

#if HAVE_DRI3
	SyncScreenFuncsRec save_sync_screen_funcs;
#endif
	void (*flush_rendering)(struct intel_screen_private *intel);
} intel_screen_private;

#define INTEL_INFO(intel) ((intel)->info)
#define IS_GENx(intel, X) (INTEL_INFO(intel)->gen >= 8*(X) && INTEL_INFO(intel)->gen < 8*((X)+1))
#define IS_GEN1(intel) IS_GENx(intel, 1)
#define IS_GEN2(intel) IS_GENx(intel, 2)
#define IS_GEN3(intel) IS_GENx(intel, 3)
#define IS_GEN4(intel) IS_GENx(intel, 4)
#define IS_GEN5(intel) IS_GENx(intel, 5)
#define IS_GEN6(intel) IS_GENx(intel, 6)
#define IS_GEN7(intel) IS_GENx(intel, 7)
#define IS_HSW(intel) (INTEL_INFO(intel)->gen == 075)

/* Some chips have specific errata (or limits) that we need to workaround. */
#define IS_I830(intel) (intel_get_device_id((intel)->dev) == PCI_CHIP_I830_M)
#define IS_845G(intel) (intel_get_device_id((intel)->dev) == PCI_CHIP_845_G)
#define IS_I865G(intel) (intel_get_device_id((intel)->dev) == PCI_CHIP_I865_G)

#define IS_I915G(pI810) (intel_get_device_id((intel)->dev) == PCI_CHIP_I915_G || intel_get_device_id((intel)->dev) == PCI_CHIP_E7221_G)
#define IS_I915GM(pI810) (intel_get_device_id((intel)->dev) == PCI_CHIP_I915_GM)

#define IS_965_Q(pI810) (intel_get_device_id((intel)->dev) == PCI_CHIP_I965_Q)

/* supports Y tiled surfaces (pre-965 Mesa isn't ready yet) */
#define SUPPORTS_YTILING(pI810) (INTEL_INFO(intel)->gen >= 040)
#define HAS_BLT(pI810) (INTEL_INFO(intel)->gen >= 060)

#ifndef I915_PARAM_HAS_PRIME_VMAP_FLUSH
#define I915_PARAM_HAS_PRIME_VMAP_FLUSH 21
#endif

enum {
	DEBUG_FLUSH_BATCHES = 0x1,
	DEBUG_FLUSH_CACHES = 0x2,
	DEBUG_FLUSH_WAIT = 0x4,
};

extern Bool intel_mode_pre_init(ScrnInfoPtr pScrn, int fd, int cpp);
extern void intel_mode_init(struct intel_screen_private *intel);
extern void intel_mode_disable_unused_functions(ScrnInfoPtr scrn);
extern void intel_mode_remove_fb(intel_screen_private *intel);
extern void intel_mode_close(intel_screen_private *intel);
extern void intel_mode_fini(intel_screen_private *intel);
extern int intel_mode_read_drm_events(intel_screen_private *intel);
extern void intel_mode_hotplug(intel_screen_private *intel);

typedef void (*intel_drm_handler_proc)(ScrnInfoPtr scrn,
                                       xf86CrtcPtr crtc,
                                       uint64_t seq,
                                       uint64_t usec,
                                       void *data);

typedef void (*intel_drm_abort_proc)(ScrnInfoPtr scrn,
                                     xf86CrtcPtr crtc,
                                     void *data);

extern uint32_t intel_drm_queue_alloc(ScrnInfoPtr scrn, xf86CrtcPtr crtc, void *data, intel_drm_handler_proc handler, intel_drm_abort_proc abort);
extern void intel_drm_abort(ScrnInfoPtr scrn, Bool (*match)(void *data, void *match_data), void *match_data);
extern void intel_drm_abort_seq(ScrnInfoPtr scrn, uint32_t seq);

extern int intel_get_pipe_from_crtc_id(drm_intel_bufmgr *bufmgr, xf86CrtcPtr crtc);
extern int intel_crtc_id(xf86CrtcPtr crtc);
extern int intel_output_dpms_status(xf86OutputPtr output);
extern void intel_copy_fb(ScrnInfoPtr scrn);

int
intel_get_crtc_msc_ust(ScrnInfoPtr scrn, xf86CrtcPtr crtc, uint64_t *msc, uint64_t *ust);

uint32_t
intel_crtc_msc_to_sequence(ScrnInfoPtr scrn, xf86CrtcPtr crtc, uint64_t expect);

uint64_t
intel_sequence_to_crtc_msc(xf86CrtcPtr crtc, uint32_t sequence);

enum DRI2FrameEventType {
	DRI2_SWAP,
	DRI2_SWAP_CHAIN,
	DRI2_FLIP,
	DRI2_WAITMSC,
};

#if XORG_VERSION_CURRENT <= XORG_VERSION_NUMERIC(1,7,99,3,0)
typedef void (*DRI2SwapEventPtr)(ClientPtr client, void *data, int type,
				 CARD64 ust, CARD64 msc, CARD64 sbc);
#endif

typedef void (*intel_pageflip_handler_proc) (uint64_t frame,
                                             uint64_t usec,
                                             void *data);

typedef void (*intel_pageflip_abort_proc) (void *data);

typedef struct _DRI2FrameEvent {
	struct intel_screen_private *intel;

	XID drawable_id;
	ClientPtr client;
	enum DRI2FrameEventType type;
	int frame;

	struct list drawable_resource, client_resource;

	/* for swaps & flips only */
	DRI2SwapEventPtr event_complete;
	void *event_data;
	DRI2BufferPtr front;
	DRI2BufferPtr back;

	/* current scanout for triple buffer */
	int old_width;
	int old_height;
	int old_pitch;
	int old_tiling;
	dri_bo *old_buffer;
} DRI2FrameEventRec, *DRI2FrameEventPtr;

extern Bool intel_do_pageflip(intel_screen_private *intel,
			      dri_bo *new_front,
			      int ref_crtc_hw_id,
			      Bool async,
			      void *pageflip_data,
			      intel_pageflip_handler_proc pageflip_handler,
			      intel_pageflip_abort_proc pageflip_abort);

static inline intel_screen_private *
intel_get_screen_private(ScrnInfoPtr scrn)
{
	return (intel_screen_private *)(scrn->driverPrivate);
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ALIGN
#define ALIGN(i,m)	(((i) + (m) - 1) & ~((m) - 1))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

extern void intel_video_init(ScreenPtr pScreen);
extern void intel_box_intersect(BoxPtr dest, BoxPtr a, BoxPtr b);
extern void intel_crtc_box(xf86CrtcPtr crtc, BoxPtr crtc_box);

extern xf86CrtcPtr intel_covering_crtc(ScrnInfoPtr scrn, BoxPtr box,
				      xf86CrtcPtr desired, BoxPtr crtc_box_ret);

Bool I830DRI2ScreenInit(ScreenPtr pScreen);
void I830DRI2CloseScreen(ScreenPtr pScreen);

/* intel_dri3.c */
Bool intel_dri3_screen_init(ScreenPtr screen);

extern Bool intel_crtc_on(xf86CrtcPtr crtc);
int intel_crtc_to_pipe(xf86CrtcPtr crtc);

/* intel_memory.c */
unsigned long intel_get_fence_size(intel_screen_private *intel, unsigned long size);
unsigned long intel_get_fence_pitch(intel_screen_private *intel, unsigned long pitch,
				   uint32_t tiling_mode);
Bool intel_check_display_stride(ScrnInfoPtr scrn, int stride, Bool tiling);
void intel_set_gem_max_sizes(ScrnInfoPtr scrn);

unsigned int
intel_compute_size(struct intel_screen_private *intel,
                   int w, int h, int bpp, unsigned usage,
                   uint32_t *tiling, int *stride);

drm_intel_bo *intel_allocate_framebuffer(ScrnInfoPtr scrn,
					 int width, int height, int cpp,
					 int *out_stride,
					 uint32_t *out_tiling);

static inline PixmapPtr get_drawable_pixmap(DrawablePtr drawable)
{
	ScreenPtr screen = drawable->pScreen;

	if (drawable->type == DRAWABLE_PIXMAP)
		return (PixmapPtr) drawable;
	else
		return screen->GetWindowPixmap((WindowPtr) drawable);
}

static inline Bool pixmap_is_scanout(PixmapPtr pixmap)
{
	ScreenPtr screen = pixmap->drawable.pScreen;

	return pixmap == screen->GetScreenPixmap(screen);
}

static inline int
intel_pixmap_pitch(PixmapPtr pixmap)
{
	return (unsigned long)pixmap->devKind;
}

/*
 * intel_sync.c
 */

#if HAVE_DRI3
Bool intel_sync_init(ScreenPtr screen);
void intel_sync_close(ScreenPtr screen);
#else
static inline Bool intel_sync_init(ScreenPtr screen) { return 0; }
static inline void intel_sync_close(ScreenPtr screen) { }
#endif

/*
 * intel_present.c
 */

#if 0
#define DebugPresent(x) ErrorF x
#else
#define DebugPresent(x)
#endif

#if HAVE_PRESENT
Bool intel_present_screen_init(ScreenPtr screen);
#else
static inline Bool intel_present_screen_init(ScreenPtr screen) { return 0; }
#endif

dri_bo *
intel_get_pixmap_bo(PixmapPtr pixmap);

void
intel_set_pixmap_bo(PixmapPtr pixmap, dri_bo *bo);

void
intel_flush(intel_screen_private *intel);

#endif /* _I830_H_ */
