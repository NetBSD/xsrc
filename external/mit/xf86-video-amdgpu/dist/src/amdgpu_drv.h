/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
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

/*
 * Authors:
 *   Kevin E. Martin <martin@xfree86.org>
 *   Rickard E. Faith <faith@valinux.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 */

#ifndef _AMDGPU_DRV_H_
#define _AMDGPU_DRV_H_

#include <stdlib.h>		/* For abs() */
#include <unistd.h>		/* For usleep() */
#include <sys/time.h>		/* For gettimeofday() */

#include "config.h"

#include "xf86str.h"
#include "compiler.h"

/* PCI support */
#include "xf86Pci.h"

#include "fb.h"

/* Cursor Support */
#include "xf86Cursor.h"

/* DDC support */
#include "xf86DDC.h"

/* Xv support */
#include "xf86xv.h"

#include "amdgpu_probe.h"

/* DRI support */
#include "xf86drm.h"
#include "amdgpu_drm.h"

#ifdef DAMAGE
#include "damage.h"
#include "globals.h"
#endif

#include "xf86Crtc.h"
#include "X11/Xatom.h"

#include "amdgpu_dri2.h"
#include "drmmode_display.h"
#include "amdgpu_bo_helper.h"

/* Render support */
#ifdef RENDER
#include "picturestr.h"
#endif

#include "compat-api.h"

#include "simple_list.h"
#include "amdpciids.h"

struct _SyncFence;

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#elif defined(USE_SYS_ENDIAN_H)
#include <sys/endian.h>
#else
#define bswap_16(value)  \
        ((((value) & 0xff) << 8) | ((value) >> 8))

#define bswap_32(value) \
        (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
        (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define bswap_64(value) \
        (((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
            << 32) | \
        (uint64_t)bswap_32((uint32_t)((value) >> 32)))
#endif

#if X_BYTE_ORDER == X_BIG_ENDIAN
#define le32_to_cpu(x) bswap_32(x)
#define le16_to_cpu(x) bswap_16(x)
#define cpu_to_le32(x) bswap_32(x)
#define cpu_to_le16(x) bswap_16(x)
#else
#define le32_to_cpu(x) (x)
#define le16_to_cpu(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le16(x) (x)
#endif

/* Provide substitutes for gcc's __FUNCTION__ on other compilers */
#if !defined(__GNUC__) && !defined(__FUNCTION__)
#define __FUNCTION__ __func__	/* C99 */
#endif

typedef enum {
	OPTION_ACCEL,
	OPTION_SW_CURSOR,
	OPTION_PAGE_FLIP,
#ifdef RENDER
	OPTION_SUBPIXEL_ORDER,
#endif
	OPTION_ZAPHOD_HEADS,
	OPTION_ACCEL_METHOD,
	OPTION_DRI3,
	OPTION_DRI,
	OPTION_SHADOW_PRIMARY,
	OPTION_TEAR_FREE,
	OPTION_DELETE_DP12,
} AMDGPUOpts;

#if XF86_CRTC_VERSION >= 5
#define AMDGPU_PIXMAP_SHARING 1
#endif

#define AMDGPU_VSYNC_TIMEOUT	20000	/* Maximum wait for VSYNC (in usecs) */

/* Buffer are aligned on 4096 byte boundaries */
#define AMDGPU_GPU_PAGE_SIZE 4096
#define AMDGPU_BUFFER_ALIGN (AMDGPU_GPU_PAGE_SIZE - 1)

#define xFixedToFloat(f) (((float) (f)) / 65536)

#define AMDGPU_LOGLEVEL_DEBUG 4

/* Other macros */
#define AMDGPU_ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))
#define AMDGPU_ALIGN(x,bytes) (((x) + ((bytes) - 1)) & ~((bytes) - 1))
#define AMDGPUPTR(pScrn)      ((AMDGPUInfoPtr)(pScrn)->driverPrivate)

#define CURSOR_WIDTH	64
#define CURSOR_HEIGHT	64

#define CURSOR_WIDTH_CIK	128
#define CURSOR_HEIGHT_CIK	128

#define AMDGPU_BO_FLAGS_GBM	0x1

struct amdgpu_buffer {
	union {
		struct gbm_bo *gbm;
		amdgpu_bo_handle amdgpu;
	} bo;
	void *cpu_ptr;
	uint32_t ref_count;
	uint32_t flags;
};

typedef struct {
	EntityInfoPtr pEnt;
	pciVideoPtr PciInfo;
	int Chipset;
	AMDGPUChipFamily ChipFamily;
	struct gbm_device *gbm;

	 Bool(*CloseScreen) (CLOSE_SCREEN_ARGS_DECL);

	void (*BlockHandler) (BLOCKHANDLER_ARGS_DECL);

	void (*CreateFence) (ScreenPtr pScreen, struct _SyncFence *pFence,
			     Bool initially_triggered);

	int pix24bpp;		/* Depth of pixmap for 24bpp fb      */
	Bool dac6bits;		/* Use 6 bit DAC?                    */

	int pixel_bytes;

	Bool directRenderingEnabled;
	struct amdgpu_dri2 dri2;

	/* accel */
	PixmapPtr fbcon_pixmap;
	uint_fast32_t gpu_flushed;
	uint_fast32_t gpu_synced;
	Bool use_glamor;
	Bool force_accel;
	Bool shadow_primary;
	Bool tear_free;

	/* general */
	OptionInfoPtr Options;

	DisplayModePtr currentMode;

	CreateScreenResourcesProcPtr CreateScreenResources;
#if GET_ABI_MAJOR(ABI_VIDEODRV_VERSION) >= 10
	CreateWindowProcPtr CreateWindow;
#endif

	Bool IsSecondary;

	Bool shadow_fb;
	void *fb_shadow;
	struct amdgpu_buffer *front_buffer;
	struct amdgpu_buffer *cursor_buffer[32];

	uint64_t vram_size;
	uint64_t gart_size;
	drmmode_rec drmmode;
	Bool drmmode_inited;
	/* r6xx+ tile config */
	Bool have_tiling_info;
	int group_bytes;

	/* kms pageflipping */
	Bool allowPageFlip;

	/* cursor size */
	int cursor_w;
	int cursor_h;

	/* If bit n of this field is set, xf86_config->crtc[n] currently can't
	 * use the HW cursor
	 */
	unsigned hwcursor_disabled;

	struct {
		CreateGCProcPtr SavedCreateGC;
		RegionPtr (*SavedCopyArea)(DrawablePtr, DrawablePtr, GCPtr,
					   int, int, int, int, int, int);
		void (*SavedPolyFillRect)(DrawablePtr, GCPtr, int, xRectangle*);
		CloseScreenProcPtr SavedCloseScreen;
		GetImageProcPtr SavedGetImage;
		GetSpansProcPtr SavedGetSpans;
		CreatePixmapProcPtr SavedCreatePixmap;
		DestroyPixmapProcPtr SavedDestroyPixmap;
		CopyWindowProcPtr SavedCopyWindow;
		ChangeWindowAttributesProcPtr SavedChangeWindowAttributes;
		BitmapToRegionProcPtr SavedBitmapToRegion;
#ifdef RENDER
		CompositeProcPtr SavedComposite;
		TrianglesProcPtr SavedTriangles;
		GlyphsProcPtr SavedGlyphs;
		TrapezoidsProcPtr SavedTrapezoids;
		AddTrapsProcPtr SavedAddTraps;
		UnrealizeGlyphProcPtr SavedUnrealizeGlyph;
#endif
#ifdef AMDGPU_PIXMAP_SHARING
		SharePixmapBackingProcPtr SavedSharePixmapBacking;
		SetSharedPixmapBackingProcPtr SavedSetSharedPixmapBacking;
#endif
	} glamor;

} AMDGPUInfoRec, *AMDGPUInfoPtr;


/* amdgpu_dri3.c */
Bool amdgpu_dri3_screen_init(ScreenPtr screen);

/* amdgpu_kms.c */
void amdgpu_scanout_update_handler(xf86CrtcPtr crtc, uint32_t frame,
				   uint64_t usec, void *event_data);

/* amdgpu_present.c */
Bool amdgpu_present_screen_init(ScreenPtr screen);

/* amdgpu_sync.c */
extern Bool amdgpu_sync_init(ScreenPtr screen);
extern void amdgpu_sync_close(ScreenPtr screen);

/* amdgpu_video.c */
extern void AMDGPUInitVideo(ScreenPtr pScreen);
extern void AMDGPUResetVideo(ScrnInfoPtr pScrn);
extern xf86CrtcPtr amdgpu_pick_best_crtc(ScrnInfoPtr pScrn,
					 Bool consider_disabled,
					 int x1, int x2, int y1, int y2);

extern AMDGPUEntPtr AMDGPUEntPriv(ScrnInfoPtr pScrn);

drmVBlankSeqType amdgpu_populate_vbl_request_type(xf86CrtcPtr crtc);

#endif /* _AMDGPU_DRV_H_ */
