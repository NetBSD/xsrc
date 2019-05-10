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

/*
 * XXX Could support more formats.
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
#include "intel_video.h"
#include "i830_reg.h"
#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include "dixstruct.h"
#include "fourcc.h"
#include "intel_video_overlay.h"

/* overlay debugging printf function */
#if 0
#define OVERLAY_DEBUG ErrorF
#else
#define OVERLAY_DEBUG if (0) ErrorF
#endif

/* kernel modesetting overlay functions */
static Bool intel_has_overlay(intel_screen_private *intel)
{
	struct drm_i915_getparam gp;
	int has_overlay = 0;
	int ret;

	gp.param = I915_PARAM_HAS_OVERLAY;
	gp.value = &has_overlay;
	ret = drmCommandWriteRead(intel->drmSubFD, DRM_I915_GETPARAM, &gp, sizeof(gp));

	return ret == 0 && !! has_overlay;
}

static Bool intel_overlay_update_attrs(intel_screen_private *intel)
{
	intel_adaptor_private *adaptor_priv = intel_get_adaptor_private(intel);
	struct drm_intel_overlay_attrs attrs;

	attrs.flags = I915_OVERLAY_UPDATE_ATTRS;
	attrs.brightness = adaptor_priv->brightness;
	attrs.contrast = adaptor_priv->contrast;
	attrs.saturation = adaptor_priv->saturation;
	attrs.color_key = adaptor_priv->colorKey;
	attrs.gamma0 = adaptor_priv->gamma0;
	attrs.gamma1 = adaptor_priv->gamma1;
	attrs.gamma2 = adaptor_priv->gamma2;
	attrs.gamma3 = adaptor_priv->gamma3;
	attrs.gamma4 = adaptor_priv->gamma4;
	attrs.gamma5 = adaptor_priv->gamma5;

	return drmCommandWriteRead(intel->drmSubFD, DRM_I915_OVERLAY_ATTRS,
				   &attrs, sizeof(attrs)) == 0;
}

void intel_video_overlay_off(intel_screen_private *intel)
{
	struct drm_intel_overlay_put_image request;
	int ret;

	request.flags = 0;

	ret = drmCommandWrite(intel->drmSubFD, DRM_I915_OVERLAY_PUT_IMAGE,
			      &request, sizeof(request));
	(void) ret;
}
static int
intel_video_overlay_set_port_attribute(ScrnInfoPtr scrn,
                                       Atom attribute, INT32 value, pointer data)
{
	intel_adaptor_private *adaptor_priv = (intel_adaptor_private *) data;
	intel_screen_private *intel = intel_get_screen_private(scrn);

	if (attribute == intel_xv_Brightness) {
		if ((value < -128) || (value > 127))
			return BadValue;
		adaptor_priv->brightness = value;
		OVERLAY_DEBUG("BRIGHTNESS\n");
	} else if (attribute == intel_xv_Contrast) {
		if ((value < 0) || (value > 255))
			return BadValue;
		adaptor_priv->contrast = value;
		OVERLAY_DEBUG("CONTRAST\n");
	} else if (attribute == intel_xv_Saturation) {
		if ((value < 0) || (value > 1023))
			return BadValue;
		adaptor_priv->saturation = value;
	} else if (attribute == intel_xv_Pipe) {
		xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
		if ((value < -1) || (value >= xf86_config->num_crtc))
			return BadValue;
		if (value < 0)
			adaptor_priv->desired_crtc = NULL;
		else
			adaptor_priv->desired_crtc = xf86_config->crtc[value];
	} else if (attribute == intel_xv_Gamma0 && (INTEL_INFO(intel)->gen >= 030)) {
		adaptor_priv->gamma0 = value;
	} else if (attribute == intel_xv_Gamma1 && (INTEL_INFO(intel)->gen >= 030)) {
		adaptor_priv->gamma1 = value;
	} else if (attribute == intel_xv_Gamma2 && (INTEL_INFO(intel)->gen >= 030)) {
		adaptor_priv->gamma2 = value;
	} else if (attribute == intel_xv_Gamma3 && (INTEL_INFO(intel)->gen >= 030)) {
		adaptor_priv->gamma3 = value;
	} else if (attribute == intel_xv_Gamma4 && (INTEL_INFO(intel)->gen >= 030)) {
		adaptor_priv->gamma4 = value;
	} else if (attribute == intel_xv_Gamma5 && (INTEL_INFO(intel)->gen >= 030)) {
		adaptor_priv->gamma5 = value;
	} else if (attribute == intel_xv_ColorKey) {
		adaptor_priv->colorKey = value;
		OVERLAY_DEBUG("COLORKEY\n");
	} else
		return BadMatch;

	if ((attribute == intel_xv_Gamma0 ||
	     attribute == intel_xv_Gamma1 ||
	     attribute == intel_xv_Gamma2 ||
	     attribute == intel_xv_Gamma3 ||
	     attribute == intel_xv_Gamma4 ||
	     attribute == intel_xv_Gamma5) && (INTEL_INFO(intel)->gen >= 030)) {
		OVERLAY_DEBUG("GAMMA\n");
	}

	if (!intel_overlay_update_attrs(intel))
		return BadValue;

	if (attribute == intel_xv_ColorKey)
		REGION_EMPTY(scrn->pScreen, &adaptor_priv->clip);

	return Success;
}

static Bool
intel_overlay_put_image(intel_screen_private *intel,
                        xf86CrtcPtr crtc,
                        int id, short width, short height,
                        int dstPitch, int dstPitch2,
                        BoxPtr dstBox, short src_w, short src_h, short drw_w,
                        short drw_h)
{
	intel_adaptor_private *adaptor_priv = intel_get_adaptor_private(intel);
	struct drm_intel_overlay_put_image request;
	int ret;
	int planar = is_planar_fourcc(id);
	float scale;
	dri_bo *tmp;

	request.flags = I915_OVERLAY_ENABLE;

	request.bo_handle = adaptor_priv->buf->handle;
	if (planar) {
		request.stride_Y = dstPitch2;
		request.stride_UV = dstPitch;
	} else {
		request.stride_Y = dstPitch;
		request.stride_UV = 0;
	}
	request.offset_Y = adaptor_priv->YBufOffset;
	request.offset_U = adaptor_priv->UBufOffset;
	request.offset_V = adaptor_priv->VBufOffset;
	OVERLAY_DEBUG("off_Y: %i, off_U: %i, off_V: %i\n", request.offset_Y,
		      request.offset_U, request.offset_V);

	request.crtc_id = intel_crtc_id(crtc);
	request.dst_x = dstBox->x1;
	request.dst_y = dstBox->y1;
	request.dst_width = dstBox->x2 - dstBox->x1;
	request.dst_height = dstBox->y2 - dstBox->y1;

	request.src_width = width;
	request.src_height = height;
	/* adjust src dimensions */
	if (request.dst_height > 1) {
		scale = ((float)request.dst_height - 1) / ((float)drw_h - 1);
		request.src_scan_height = src_h * scale;
	} else
		request.src_scan_height = 1;

	if (request.dst_width > 1) {
		scale = ((float)request.dst_width - 1) / ((float)drw_w - 1);
		request.src_scan_width = src_w * scale;
	} else
		request.src_scan_width = 1;

	if (planar) {
		request.flags |= I915_OVERLAY_YUV_PLANAR | I915_OVERLAY_YUV420;
	} else {
		request.flags |= I915_OVERLAY_YUV_PACKED | I915_OVERLAY_YUV422;
		if (id == FOURCC_UYVY)
			request.flags |= I915_OVERLAY_Y_SWAP;
	}

	ret = drmCommandWrite(intel->drmSubFD, DRM_I915_OVERLAY_PUT_IMAGE,
			      &request, sizeof(request));
	if (ret)
		return FALSE;

	if (!adaptor_priv->reusable) {
		drm_intel_bo_unreference(adaptor_priv->buf);
		adaptor_priv->buf = NULL;
		adaptor_priv->reusable = TRUE;
	}

	tmp = adaptor_priv->old_buf[1];
	adaptor_priv->old_buf[1] = adaptor_priv->old_buf[0];
	adaptor_priv->old_buf[0] = adaptor_priv->buf;
	adaptor_priv->buf = tmp;

	return TRUE;
}

static void
intel_update_dst_box_to_crtc_coords(ScrnInfoPtr scrn, xf86CrtcPtr crtc,
				    BoxPtr dstBox)
{
	int tmp;

	/* for overlay, we should take it from crtc's screen
	 * coordinate to current crtc's display mode.
	 * yeah, a bit confusing.
	 */
	switch (crtc->rotation & 0xf) {
	case RR_Rotate_0:
		dstBox->x1 -= crtc->x;
		dstBox->x2 -= crtc->x;
		dstBox->y1 -= crtc->y;
		dstBox->y2 -= crtc->y;
		break;
	case RR_Rotate_90:
		tmp = dstBox->x1;
		dstBox->x1 = dstBox->y1 - crtc->x;
		dstBox->y1 = scrn->virtualX - tmp - crtc->y;
		tmp = dstBox->x2;
		dstBox->x2 = dstBox->y2 - crtc->x;
		dstBox->y2 = scrn->virtualX - tmp - crtc->y;
		tmp = dstBox->y1;
		dstBox->y1 = dstBox->y2;
		dstBox->y2 = tmp;
		break;
	case RR_Rotate_180:
		tmp = dstBox->x1;
		dstBox->x1 = scrn->virtualX - dstBox->x2 - crtc->x;
		dstBox->x2 = scrn->virtualX - tmp - crtc->x;
		tmp = dstBox->y1;
		dstBox->y1 = scrn->virtualY - dstBox->y2 - crtc->y;
		dstBox->y2 = scrn->virtualY - tmp - crtc->y;
		break;
	case RR_Rotate_270:
		tmp = dstBox->x1;
		dstBox->x1 = scrn->virtualY - dstBox->y1 - crtc->x;
		dstBox->y1 = tmp - crtc->y;
		tmp = dstBox->x2;
		dstBox->x2 = scrn->virtualY - dstBox->y2 - crtc->x;
		dstBox->y2 = tmp - crtc->y;
		tmp = dstBox->x1;
		dstBox->x1 = dstBox->x2;
		dstBox->x2 = tmp;
		break;
	}

	return;
}

static Bool
intel_video_overlay_display(ScrnInfoPtr scrn, xf86CrtcPtr crtc,
                            int id, short width, short height,
                            int dstPitch, int dstPitch2,
                            BoxPtr dstBox, short src_w, short src_h, short drw_w,
                            short drw_h)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	int tmp;

	OVERLAY_DEBUG("I830DisplayVideo: %dx%d (pitch %d)\n", width, height,
		      dstPitch);

	/*
	 * If the video isn't visible on any CRTC, turn it off
	 */
	if (!crtc) {
		intel_video_overlay_off(intel);
		return TRUE;
	}

	intel_update_dst_box_to_crtc_coords(scrn, crtc, dstBox);

	if (crtc->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
		tmp = width;
		width = height;
		height = tmp;
		tmp = drw_w;
		drw_w = drw_h;
		drw_h = tmp;
		tmp = src_w;
		src_w = src_h;
		src_h = tmp;
	}

	return intel_overlay_put_image(intel, crtc, id,
					 width, height,
					 dstPitch, dstPitch2, dstBox,
					 src_w, src_h, drw_w, drw_h);
}

static int
intel_video_overlay_put_image(ScrnInfoPtr scrn,
                              short src_x, short src_y,
                              short drw_x, short drw_y,
                              short src_w, short src_h,
                              short drw_w, short drw_h,
                              int id, unsigned char *buf,
                              short width, short height,
                              Bool sync, RegionPtr clipBoxes, pointer data,
                              DrawablePtr drawable)
{
	intel_adaptor_private *adaptor_priv = (intel_adaptor_private *) data;
	int dstPitch, dstPitch2;
	BoxRec dstBox;
	xf86CrtcPtr crtc;
	int top, left, npixels, nlines;

#if 0
	ErrorF("I830PutImage: src: (%d,%d)(%d,%d), dst: (%d,%d)(%d,%d)\n"
	       "width %d, height %d\n", src_x, src_y, src_w, src_h, drw_x,
	       drw_y, drw_w, drw_h, width, height);
#endif

	/* If dst width and height are less than 1/8th the src size, the
	 * src/dst scale factor becomes larger than 8 and doesn't fit in
	 * the scale register. */
	if (src_w >= (drw_w * 8))
		drw_w = src_w / 7;

	if (src_h >= (drw_h * 8))
		drw_h = src_h / 7;

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

	/* overlay can't handle rotation natively, store it for the copy func */
	if (crtc)
		adaptor_priv->rotation = crtc->rotation;
	else {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "Fail to clip video to any crtc!\n");
		return Success;
	}

	if (!intel_video_copy_data(scrn, adaptor_priv, width, height,
				  &dstPitch, &dstPitch2,
				  top, left, npixels, nlines, id, buf))
		return BadAlloc;

	if (!intel_video_overlay_display
	    (scrn, crtc, id, width, height, dstPitch, dstPitch2,
	     &dstBox, src_w, src_h, drw_w, drw_h))
		return BadAlloc;

	/* update cliplist */
	if (!REGION_EQUAL(scrn->pScreen, &adaptor_priv->clip, clipBoxes)) {
		REGION_COPY(scrn->pScreen, &adaptor_priv->clip, clipBoxes);
		xf86XVFillKeyHelperDrawable(drawable,
					    adaptor_priv->colorKey,
					    clipBoxes);
	}

	adaptor_priv->videoStatus = CLIENT_VIDEO_ON;

	return Success;
}

XF86VideoAdaptorPtr intel_video_overlay_setup_image(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	intel_screen_private *intel = intel_get_screen_private(scrn);
	XF86VideoAdaptorPtr adapt;
	intel_adaptor_private *adaptor_priv;
	XF86AttributePtr att;

	/* Set up overlay video if it is available */
	intel->use_overlay = intel_has_overlay(intel);
	if (!intel->use_overlay)
                return NULL;

	OVERLAY_DEBUG("intel_video_overlay_setup_image\n");

	if (!(adapt = calloc(1,
			     sizeof(XF86VideoAdaptorRec) +
			     sizeof(intel_adaptor_private) +
			     sizeof(DevUnion))))
		return NULL;

	adapt->type = XvWindowMask | XvInputMask | XvImageMask;
	adapt->flags = VIDEO_OVERLAID_IMAGES /*| VIDEO_CLIP_TO_VIEWPORT */ ;
	adapt->name = "Intel(R) Video Overlay";
	adapt->nEncodings = 1;
	adapt->pEncodings = xnfalloc(sizeof(intel_xv_dummy_encoding));
	memcpy(adapt->pEncodings, intel_xv_dummy_encoding, sizeof(intel_xv_dummy_encoding));
	if (IS_845G(intel) || IS_I830(intel)) {
		adapt->pEncodings->width = IMAGE_MAX_WIDTH_LEGACY;
		adapt->pEncodings->height = IMAGE_MAX_HEIGHT_LEGACY;
	}
	adapt->nFormats = NUM_FORMATS;
	adapt->pFormats = intel_xv_formats;
	adapt->nPorts = 1;
	adapt->pPortPrivates = (DevUnion *) (&adapt[1]);

	adaptor_priv = (intel_adaptor_private *)&adapt->pPortPrivates[1];

	adapt->pPortPrivates[0].ptr = (pointer) (adaptor_priv);
	adapt->nAttributes = NUM_ATTRIBUTES;
	if (INTEL_INFO(intel)->gen >= 030)
		adapt->nAttributes += GAMMA_ATTRIBUTES;	/* has gamma */
	adapt->pAttributes =
	    xnfalloc(sizeof(XF86AttributeRec) * adapt->nAttributes);
	/* Now copy the attributes */
	att = adapt->pAttributes;
	memcpy((char *)att, (char *)intel_xv_attributes,
	       sizeof(XF86AttributeRec) * NUM_ATTRIBUTES);
	att += NUM_ATTRIBUTES;
	if (INTEL_INFO(intel)->gen >= 030) {
		memcpy((char *)att, (char *)intel_xv_gamma_attributes,
		       sizeof(XF86AttributeRec) * GAMMA_ATTRIBUTES);
	}
	adapt->nImages = NUM_IMAGES - XVMC_IMAGE;

	adapt->pImages = intel_xv_images;
	adapt->PutVideo = NULL;
	adapt->PutStill = NULL;
	adapt->GetVideo = NULL;
	adapt->GetStill = NULL;
	adapt->StopVideo = intel_video_stop_video;
	adapt->SetPortAttribute = intel_video_overlay_set_port_attribute;
	adapt->GetPortAttribute = intel_video_get_port_attribute;
	adapt->QueryBestSize = intel_video_query_best_size;
	adapt->PutImage = intel_video_overlay_put_image;
	adapt->QueryImageAttributes = intel_video_query_image_attributes;

	adaptor_priv->textured = FALSE;
	adaptor_priv->colorKey = intel->colorKey & ((1 << scrn->depth) - 1);
	adaptor_priv->videoStatus = 0;
	adaptor_priv->brightness = -19;	/* (255/219) * -16 */
	adaptor_priv->contrast = 75;	/* 255/219 * 64 */
	adaptor_priv->saturation = 146;	/* 128/112 * 128 */
	adaptor_priv->desired_crtc = NULL;
	adaptor_priv->buf = NULL;
	adaptor_priv->old_buf[0] = NULL;
	adaptor_priv->old_buf[1] = NULL;
	adaptor_priv->gamma5 = 0xc0c0c0;
	adaptor_priv->gamma4 = 0x808080;
	adaptor_priv->gamma3 = 0x404040;
	adaptor_priv->gamma2 = 0x202020;
	adaptor_priv->gamma1 = 0x101010;
	adaptor_priv->gamma0 = 0x080808;

	adaptor_priv->rotation = RR_Rotate_0;

	/* gotta uninit this someplace */
	REGION_NULL(screen, &adaptor_priv->clip);

	intel->adaptor = adapt;

	intel_xv_ColorKey = MAKE_ATOM("XV_COLORKEY");
	intel_xv_Brightness = MAKE_ATOM("XV_BRIGHTNESS");
	intel_xv_Contrast = MAKE_ATOM("XV_CONTRAST");
	intel_xv_Saturation = MAKE_ATOM("XV_SATURATION");

	/* Allow the pipe to be switched from pipe A to B when in clone mode */
	intel_xv_Pipe = MAKE_ATOM("XV_PIPE");

	if (INTEL_INFO(intel)->gen >= 030) {
		intel_xv_Gamma0 = MAKE_ATOM("XV_GAMMA0");
		intel_xv_Gamma1 = MAKE_ATOM("XV_GAMMA1");
		intel_xv_Gamma2 = MAKE_ATOM("XV_GAMMA2");
		intel_xv_Gamma3 = MAKE_ATOM("XV_GAMMA3");
		intel_xv_Gamma4 = MAKE_ATOM("XV_GAMMA4");
		intel_xv_Gamma5 = MAKE_ATOM("XV_GAMMA5");
	}

	intel_overlay_update_attrs(intel);

	return adapt;
}
