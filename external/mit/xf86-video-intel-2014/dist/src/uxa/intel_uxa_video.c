/***************************************************************************

 Copyright 2000 Intel Corporation.  All Rights Reserved.

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
 IN NO EVENT SHALL INTEL, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 **************************************************************************/

/*
 * i830_video.c: i830/i845 Xv driver.
 *
 * Copyright © 2002 by Alan Hourihane and David Dawes
 *
 * Authors:
 *	Alan Hourihane <alanh@tungstengraphics.com>
 *	David Dawes <dawes@xfree86.org>
 *
 * Derived from i810 Xv driver:
 *
 * Authors of i810 code:
 * 	Jonathan Bian <jonathan.bian@intel.com>
 *      Offscreen Images:
 *        Matt Sottek <matthew.j.sottek@intel.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "xorg-server.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86Pci.h"
#include "xf86fbman.h"
#include "xf86drm.h"
#include "regionstr.h"
#include "randrstr.h"
#include "windowstr.h"
#include "damage.h"
#include "intel.h"
#include "intel_uxa.h"
#include "i830_reg.h"
#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include "dixstruct.h"
#include "fourcc.h"

#ifdef INTEL_XVMC
#define _INTEL_XVMC_SERVER_
#include "intel_xvmc.h"
#endif

/* overlay debugging printf function */
#if 0
#define UXA_VIDEO_DEBUG ErrorF
#else
#define UXA_VIDEO_DEBUG if (0) ErrorF
#endif

static int intel_uxa_video_put_image_textured(ScrnInfoPtr, short, short, short, short, short, short,
			short, short, int, unsigned char *, short, short,
			Bool, RegionPtr, pointer, DrawablePtr);

static int
intel_uxa_video_set_port_attribute(ScrnInfoPtr scrn,
			     Atom attribute, INT32 value, pointer data)
{
	intel_adaptor_private *adaptor_priv = (intel_adaptor_private *) data;

	if (attribute == intel_xv_Brightness) {
		if ((value < -128) || (value > 127))
			return BadValue;
		adaptor_priv->brightness = value;
		return Success;
	} else if (attribute == intel_xv_Contrast) {
		if ((value < 0) || (value > 255))
			return BadValue;
		adaptor_priv->contrast = value;
		return Success;
	} else if (attribute == intel_xv_SyncToVblank) {
		if ((value < -1) || (value > 1))
			return BadValue;
		adaptor_priv->SyncToVblank = value;
		return Success;
	} else {
		return BadMatch;
	}
}


static int xvmc_passthrough(int id)
{
#ifdef INTEL_XVMC
	return id == FOURCC_XVMC;
#else
	return 0;
#endif
}


static void
intel_wait_for_scanline(ScrnInfoPtr scrn, PixmapPtr pixmap,
			xf86CrtcPtr crtc, RegionPtr clipBoxes)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	pixman_box16_t box, crtc_box;
	int pipe, event;
	Bool full_height;
	int y1, y2;

	pipe = -1;
	if (scrn->vtSema && pixmap_is_scanout(pixmap))
		pipe = intel_crtc_to_pipe(crtc);
	if (pipe < 0)
		return;

	box = *REGION_EXTENTS(unused, clipBoxes);

	if (crtc->transform_in_use)
		pixman_f_transform_bounds(&crtc->f_framebuffer_to_crtc, &box);

	/* We could presume the clip was correctly computed... */
	intel_crtc_box(crtc, &crtc_box);
	intel_box_intersect(&box, &crtc_box, &box);

	/*
	 * Make sure we don't wait for a scanline that will
	 * never occur
	 */
	y1 = (crtc_box.y1 <= box.y1) ? box.y1 - crtc_box.y1 : 0;
	y2 = (box.y2 <= crtc_box.y2) ?
		box.y2 - crtc_box.y1 : crtc_box.y2 - crtc_box.y1;
	if (y2 <= y1)
		return;

	full_height = FALSE;
	if (y1 == 0 && y2 == (crtc_box.y2 - crtc_box.y1))
		full_height = TRUE;

	/*
	 * Pre-965 doesn't have SVBLANK, so we need a bit
	 * of extra time for the blitter to start up and
	 * do its job for a full height blit
	 */
	if (full_height && INTEL_INFO(intel)->gen < 040)
		y2 -= 2;

	if (pipe == 0) {
		pipe = MI_LOAD_SCAN_LINES_DISPLAY_PIPEA;
		event = MI_WAIT_FOR_PIPEA_SCAN_LINE_WINDOW;
		if (full_height && INTEL_INFO(intel)->gen >= 040)
			event = MI_WAIT_FOR_PIPEA_SVBLANK;
	} else {
		pipe = MI_LOAD_SCAN_LINES_DISPLAY_PIPEB;
		event = MI_WAIT_FOR_PIPEB_SCAN_LINE_WINDOW;
		if (full_height && INTEL_INFO(intel)->gen >= 040)
			event = MI_WAIT_FOR_PIPEB_SVBLANK;
	}

	if (crtc->mode.Flags & V_INTERLACE) {
		/* DSL count field lines */
		y1 /= 2;
		y2 /= 2;
	}

	BEGIN_BATCH(5);
	/* The documentation says that the LOAD_SCAN_LINES command
	 * always comes in pairs. Don't ask me why. */
	OUT_BATCH(MI_LOAD_SCAN_LINES_INCL | pipe);
	OUT_BATCH((y1 << 16) | (y2-1));
	OUT_BATCH(MI_LOAD_SCAN_LINES_INCL | pipe);
	OUT_BATCH((y1 << 16) | (y2-1));
	OUT_BATCH(MI_WAIT_FOR_EVENT | event);
	ADVANCE_BATCH();
}

/*
 * The source rectangle of the video is defined by (src_x, src_y, src_w, src_h).
 * The dest rectangle of the video is defined by (drw_x, drw_y, drw_w, drw_h).
 * id is a fourcc code for the format of the video.
 * buf is the pointer to the source data in system memory.
 * width and height are the w/h of the source data.
 * If "sync" is TRUE, then we must be finished with *buf at the point of return
 * (which we always are).
 * clipBoxes is the clipping region in screen space.
 * data is a pointer to our port private.
 * drawable is some Drawable, which might not be the screen in the case of
 * compositing.  It's a new argument to the function in the 1.1 server.
 */
static int
intel_uxa_video_put_image_textured(ScrnInfoPtr scrn,
                                   short src_x, short src_y,
                                   short drw_x, short drw_y,
                                   short src_w, short src_h,
                                   short drw_w, short drw_h,
                                   int id, unsigned char *buf,
                                   short width, short height,
                                   Bool sync, RegionPtr clipBoxes, pointer data,
                                   DrawablePtr drawable)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	intel_adaptor_private *adaptor_priv = (intel_adaptor_private *) data;
	PixmapPtr pixmap = get_drawable_pixmap(drawable);
	int dstPitch, dstPitch2;
	BoxRec dstBox;
	xf86CrtcPtr crtc;
	int top, left, npixels, nlines;

	if (!intel_uxa_pixmap_is_offscreen(pixmap))
		return BadAlloc;

#if 0
	ErrorF("I830PutImage: src: (%d,%d)(%d,%d), dst: (%d,%d)(%d,%d)\n"
	       "width %d, height %d\n", src_x, src_y, src_w, src_h, drw_x,
	       drw_y, drw_w, drw_h, width, height);
#endif

	if (!intel_clip_video_helper(scrn,
				    adaptor_priv,
				    &crtc,
				    &dstBox,
				    src_x, src_y, drw_x, drw_y,
				    src_w, src_h, drw_w, drw_h,
				    id,
				    &top, &left, &npixels, &nlines, clipBoxes,
				    width, height))
		return Success;

	if (xvmc_passthrough(id)) {
		uint32_t *gem_handle = (uint32_t *)buf;
		int size;

		intel_setup_dst_params(scrn, adaptor_priv, width, height,
				&dstPitch, &dstPitch2, &size, id);

		if (IS_I915G(intel) || IS_I915GM(intel)) {
			/* XXX: i915 is not support and needs some
			 * serious care.  grep for KMS in i915_hwmc.c */
			return BadAlloc;
		}

		if (adaptor_priv->buf)
			drm_intel_bo_unreference(adaptor_priv->buf);

		adaptor_priv->buf =
			drm_intel_bo_gem_create_from_name(intel->bufmgr,
							  "xvmc surface",
							  *gem_handle);
		if (adaptor_priv->buf == NULL)
			return BadAlloc;

		adaptor_priv->reusable = FALSE;
	} else {
		if (!intel_video_copy_data(scrn, adaptor_priv, width, height,
					  &dstPitch, &dstPitch2,
					  top, left, npixels, nlines, id, buf))
			return BadAlloc;
	}

	if (crtc && adaptor_priv->SyncToVblank != 0 && INTEL_INFO(intel)->gen < 060) {
		intel_wait_for_scanline(scrn, pixmap, crtc, clipBoxes);
	}

	if (INTEL_INFO(intel)->gen >= 060) {
		Gen6DisplayVideoTextured(scrn, adaptor_priv, id, clipBoxes,
					 width, height, dstPitch, dstPitch2,
					 src_w, src_h,
					 drw_w, drw_h, pixmap);
	} else if (INTEL_INFO(intel)->gen >= 040) {
		I965DisplayVideoTextured(scrn, adaptor_priv, id, clipBoxes,
					 width, height, dstPitch, dstPitch2,
					 src_w, src_h,
					 drw_w, drw_h, pixmap);
	} else {
		I915DisplayVideoTextured(scrn, adaptor_priv, id, clipBoxes,
					 width, height, dstPitch, dstPitch2,
					 src_w, src_h, drw_w, drw_h,
					 pixmap);
	}

	intel_get_screen_private(scrn)->needs_flush = TRUE;
	DamageDamageRegion(drawable, clipBoxes);

	/* And make sure the WAIT_FOR_EVENT is queued before any
	 * modesetting/dpms operations on the pipe.
	 */
	intel_batch_submit(scrn);

	return Success;
}

XF86VideoAdaptorPtr intel_uxa_video_setup_image_textured(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	intel_screen_private *intel = intel_get_screen_private(scrn);
	XF86VideoAdaptorPtr adapt;
	intel_adaptor_private *adaptor_privs;
	DevUnion *devUnions;
	int nports = 16, i;

	UXA_VIDEO_DEBUG("intel_video_overlay_setup_image\n");

	adapt = calloc(1, sizeof(XF86VideoAdaptorRec));
	adaptor_privs = calloc(nports, sizeof(intel_adaptor_private));
	devUnions = calloc(nports, sizeof(DevUnion));
	if (adapt == NULL || adaptor_privs == NULL || devUnions == NULL) {
		free(adapt);
		free(adaptor_privs);
		free(devUnions);
		return NULL;
	}

	adapt->type = XvWindowMask | XvInputMask | XvImageMask;
	adapt->flags = 0;
	adapt->name = "Intel(R) Textured Video";
	adapt->nEncodings = 1;
	adapt->pEncodings = xnfalloc(sizeof(intel_xv_dummy_encoding));
	memcpy(adapt->pEncodings, intel_xv_dummy_encoding, sizeof(intel_xv_dummy_encoding));
	adapt->nFormats = NUM_FORMATS;
	adapt->pFormats = intel_xv_formats;
	adapt->nPorts = nports;
	adapt->pPortPrivates = devUnions;
	adapt->nAttributes = 0;
	adapt->pAttributes = NULL;
	if (IS_I915G(intel) || IS_I915GM(intel))
		adapt->nImages = NUM_IMAGES - XVMC_IMAGE;
	else
		adapt->nImages = NUM_IMAGES;

	adapt->pImages = intel_xv_images;
	adapt->PutVideo = NULL;
	adapt->PutStill = NULL;
	adapt->GetVideo = NULL;
	adapt->GetStill = NULL;
	adapt->StopVideo = intel_video_stop_video;
	adapt->SetPortAttribute = intel_uxa_video_set_port_attribute;
	adapt->GetPortAttribute = intel_video_get_port_attribute;
	adapt->QueryBestSize = intel_video_query_best_size;
	adapt->PutImage = intel_uxa_video_put_image_textured;
	adapt->QueryImageAttributes = intel_video_query_image_attributes;

	for (i = 0; i < nports; i++) {
		intel_adaptor_private *adaptor_priv = &adaptor_privs[i];

		adaptor_priv->textured = TRUE;
		adaptor_priv->videoStatus = 0;
		adaptor_priv->buf = NULL;
		adaptor_priv->old_buf[0] = NULL;
		adaptor_priv->old_buf[1] = NULL;

		adaptor_priv->rotation = RR_Rotate_0;
		adaptor_priv->SyncToVblank = 1;

		/* gotta uninit this someplace, XXX: shouldn't be necessary for textured */
		REGION_NULL(screen, &adaptor_priv->clip);

		adapt->pPortPrivates[i].ptr = (pointer) (adaptor_priv);
	}

	intel_xv_SyncToVblank = MAKE_ATOM("XV_SYNC_TO_VBLANK");

	return adapt;
}
