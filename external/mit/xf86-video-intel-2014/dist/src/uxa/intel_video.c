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

#ifdef INTEL_XVMC
#define _INTEL_XVMC_SERVER_
#include "intel_xvmc.h"
#endif
#include "intel_uxa.h"
#include "intel_video_overlay.h"

Atom intel_xv_Brightness, intel_xv_Contrast, intel_xv_Saturation, intel_xv_ColorKey, intel_xv_Pipe;
Atom intel_xv_Gamma0, intel_xv_Gamma1, intel_xv_Gamma2, intel_xv_Gamma3, intel_xv_Gamma4, intel_xv_Gamma5;
Atom intel_xv_SyncToVblank;

/* client libraries expect an encoding */
const XF86VideoEncodingRec intel_xv_dummy_encoding[1] = {
	{
	 0,
	 "XV_IMAGE",
	 IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT,
	 {1, 1}
	 }
};

XF86VideoFormatRec intel_xv_formats[NUM_FORMATS] = {
	{15, TrueColor}, {16, TrueColor}, {24, TrueColor}
};

XF86AttributeRec intel_xv_attributes[NUM_ATTRIBUTES] = {
	{XvSettable | XvGettable, 0, (1 << 24) - 1, "XV_COLORKEY"},
	{XvSettable | XvGettable, -128, 127, "XV_BRIGHTNESS"},
	{XvSettable | XvGettable, 0, 255, "XV_CONTRAST"},
	{XvSettable | XvGettable, 0, 1023, "XV_SATURATION"},
	{XvSettable | XvGettable, -1, 1, "XV_PIPE"}
};

#define GAMMA_ATTRIBUTES 6
XF86AttributeRec intel_xv_gamma_attributes[GAMMA_ATTRIBUTES] = {
	{XvSettable | XvGettable, 0, 0xffffff, "XV_GAMMA0"},
	{XvSettable | XvGettable, 0, 0xffffff, "XV_GAMMA1"},
	{XvSettable | XvGettable, 0, 0xffffff, "XV_GAMMA2"},
	{XvSettable | XvGettable, 0, 0xffffff, "XV_GAMMA3"},
	{XvSettable | XvGettable, 0, 0xffffff, "XV_GAMMA4"},
	{XvSettable | XvGettable, 0, 0xffffff, "XV_GAMMA5"}
};

#ifdef INTEL_XVMC
#define NUM_IMAGES 5
#define XVMC_IMAGE 1
#else
#define NUM_IMAGES 4
#define XVMC_IMAGE 0
#endif

XF86ImageRec intel_xv_images[NUM_IMAGES] = {
	XVIMAGE_YUY2,
	XVIMAGE_YV12,
	XVIMAGE_I420,
	XVIMAGE_UYVY,
#ifdef INTEL_XVMC
	{
	 /*
	  * Below, a dummy picture type that is used in XvPutImage only to do
	  * an overlay update. Introduced for the XvMC client lib.
	  * Defined to have a zero data size.
	  */
	 FOURCC_XVMC,
	 XvYUV,
	 LSBFirst,
	 {'X', 'V', 'M', 'C',
	  0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0xAA, 0x00,
	  0x38, 0x9B, 0x71},
	 12,
	 XvPlanar,
	 3,
	 0, 0, 0, 0,
	 8, 8, 8,
	 1, 2, 2,
	 1, 2, 2,
	 {'Y', 'V', 'U',
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 XvTopToBottom},
#endif
};

void intel_video_init(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	intel_screen_private *intel = intel_get_screen_private(scrn);
	XF86VideoAdaptorPtr *adaptors = NULL, *newAdaptors = NULL;
	XF86VideoAdaptorPtr overlayAdaptor = NULL, texturedAdaptor = NULL;
	int num_adaptors = xf86XVListGenericAdaptors(scrn, &adaptors);

	/* Give our adaptor list enough space for the overlay and/or texture video
	 * adaptors.
	 */
	newAdaptors = realloc(adaptors,
			      (num_adaptors + 3) * sizeof(XF86VideoAdaptorPtr));

	if (newAdaptors == NULL) {
		free(adaptors);
		return;
	}
	adaptors = newAdaptors;

	/* Add the adaptors supported by our hardware.  First, set up the atoms
	 * that will be used by both output adaptors.
	 */
	intel_xv_Brightness = MAKE_ATOM("XV_BRIGHTNESS");
	intel_xv_Contrast = MAKE_ATOM("XV_CONTRAST");

        /* Set up textured video if we can do it at this depth and we are on
         * supported hardware.
         */
        if (!intel->force_fallback &&
            scrn->bitsPerPixel >= 16 &&
            INTEL_INFO(intel)->gen >= 030 &&
            INTEL_INFO(intel)->gen < 0100) {
                texturedAdaptor = intel_uxa_video_setup_image_textured(screen);
                if (texturedAdaptor != NULL) {
                        xf86DrvMsg(scrn->scrnIndex, X_INFO,
                                   "Set up textured video\n");
                } else {
                        xf86DrvMsg(scrn->scrnIndex, X_ERROR,
                                   "Failed to set up textured video\n");
                }
        }

        overlayAdaptor = intel_video_overlay_setup_image(screen);

        if (intel->use_overlay) {
		if (overlayAdaptor != NULL) {
			xf86DrvMsg(scrn->scrnIndex, X_INFO,
				   "Set up overlay video\n");
		} else {
			xf86DrvMsg(scrn->scrnIndex, X_ERROR,
				   "Failed to set up overlay video\n");
		}
	}

	if (overlayAdaptor && intel->XvPreferOverlay)
		adaptors[num_adaptors++] = overlayAdaptor;

	if (texturedAdaptor)
		adaptors[num_adaptors++] = texturedAdaptor;

	if (overlayAdaptor && !intel->XvPreferOverlay)
		adaptors[num_adaptors++] = overlayAdaptor;

	if (num_adaptors) {
		xf86XVScreenInit(screen, adaptors, num_adaptors);
	} else {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "Disabling Xv because no adaptors could be initialized.\n");
		intel->XvEnabled = FALSE;
	}

#ifdef INTEL_XVMC
        if (texturedAdaptor)
                intel_xvmc_adaptor_init(screen);
#endif

	free(adaptors);
}

void intel_free_video_buffers(intel_adaptor_private *adaptor_priv)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (adaptor_priv->old_buf[i]) {
			drm_intel_bo_disable_reuse(adaptor_priv->old_buf[i]);
			drm_intel_bo_unreference(adaptor_priv->old_buf[i]);
			adaptor_priv->old_buf[i] = NULL;
		}
	}

	if (adaptor_priv->buf) {
		drm_intel_bo_unreference(adaptor_priv->buf);
		adaptor_priv->buf = NULL;
	}
}

int
intel_video_get_port_attribute(ScrnInfoPtr scrn,
                               Atom attribute, INT32 * value, pointer data)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	intel_adaptor_private *adaptor_priv = (intel_adaptor_private *) data;

	if (attribute == intel_xv_Brightness) {
		*value = adaptor_priv->brightness;
	} else if (attribute == intel_xv_Contrast) {
		*value = adaptor_priv->contrast;
	} else if (attribute == intel_xv_Saturation) {
		*value = adaptor_priv->saturation;
	} else if (attribute == intel_xv_Pipe) {
		int c;
		xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
		for (c = 0; c < xf86_config->num_crtc; c++)
			if (xf86_config->crtc[c] == adaptor_priv->desired_crtc)
				break;
		if (c == xf86_config->num_crtc)
			c = -1;
		*value = c;
	} else if (attribute == intel_xv_Gamma0 && (INTEL_INFO(intel)->gen >= 030)) {
		*value = adaptor_priv->gamma0;
	} else if (attribute == intel_xv_Gamma1 && (INTEL_INFO(intel)->gen >= 030)) {
		*value = adaptor_priv->gamma1;
	} else if (attribute == intel_xv_Gamma2 && (INTEL_INFO(intel)->gen >= 030)) {
		*value = adaptor_priv->gamma2;
	} else if (attribute == intel_xv_Gamma3 && (INTEL_INFO(intel)->gen >= 030)) {
		*value = adaptor_priv->gamma3;
	} else if (attribute == intel_xv_Gamma4 && (INTEL_INFO(intel)->gen >= 030)) {
		*value = adaptor_priv->gamma4;
	} else if (attribute == intel_xv_Gamma5 && (INTEL_INFO(intel)->gen >= 030)) {
		*value = adaptor_priv->gamma5;
	} else if (attribute == intel_xv_ColorKey) {
		*value = adaptor_priv->colorKey;
	} else if (attribute == intel_xv_SyncToVblank) {
		*value = adaptor_priv->SyncToVblank;
	} else
		return BadMatch;

	return Success;
}

void
intel_video_query_best_size(ScrnInfoPtr scrn,
		  Bool motion,
		  short vid_w, short vid_h,
		  short drw_w, short drw_h,
		  unsigned int *p_w, unsigned int *p_h, pointer data)
{
	if (vid_w > (drw_w << 1))
		drw_w = vid_w >> 1;
	if (vid_h > (drw_h << 1))
		drw_h = vid_h >> 1;

	*p_w = drw_w;
	*p_h = drw_h;
}

static Bool
intel_video_copy_packed_data(intel_adaptor_private *adaptor_priv,
                                 unsigned char *buf,
                                 int srcPitch, int dstPitch, int top, int left, int h, int w)
{
	unsigned char *src, *dst, *dst_base;
	int i, j;
	unsigned char *s;

#if 0
	ErrorF("intel_video_copy_packed_data: (%d,%d) (%d,%d)\n"
	       "srcPitch: %d, dstPitch: %d\n", top, left, h, w,
	       srcPitch, dstPitch);
#endif

	src = buf + (top * srcPitch) + (left << 1);

	if (drm_intel_gem_bo_map_gtt(adaptor_priv->buf))
		return FALSE;

	dst_base = adaptor_priv->buf->virtual;

	dst = dst_base + adaptor_priv->YBufOffset;

	switch (adaptor_priv->rotation) {
	case RR_Rotate_0:
		w <<= 1;
		for (i = 0; i < h; i++) {
			memcpy(dst, src, w);
			src += srcPitch;
			dst += dstPitch;
		}
		break;
	case RR_Rotate_90:
		h <<= 1;
		for (i = 0; i < h; i += 2) {
			s = src;
			for (j = 0; j < w; j++) {
				/* Copy Y */
				dst[(i + 0) + ((w - j - 1) * dstPitch)] = *s++;
				(void)*s++;
			}
			src += srcPitch;
		}
		h >>= 1;
		src = buf + (top * srcPitch) + (left << 1);
		for (i = 0; i < h; i += 2) {
			for (j = 0; j < w; j += 2) {
				/* Copy U */
				dst[((i * 2) + 1) + ((w - j - 1) * dstPitch)] =
				    src[(j * 2) + 1 + (i * srcPitch)];
				dst[((i * 2) + 1) + ((w - j - 2) * dstPitch)] =
				    src[(j * 2) + 1 + ((i + 1) * srcPitch)];
				/* Copy V */
				dst[((i * 2) + 3) + ((w - j - 1) * dstPitch)] =
				    src[(j * 2) + 3 + (i * srcPitch)];
				dst[((i * 2) + 3) + ((w - j - 2) * dstPitch)] =
				    src[(j * 2) + 3 + ((i + 1) * srcPitch)];
			}
		}
		break;
	case RR_Rotate_180:
		w <<= 1;
		for (i = 0; i < h; i++) {
			s = src;
			for (j = 0; j < w; j += 4) {
				dst[(w - j - 4) + ((h - i - 1) * dstPitch)] =
				    *s++;
				dst[(w - j - 3) + ((h - i - 1) * dstPitch)] =
				    *s++;
				dst[(w - j - 2) + ((h - i - 1) * dstPitch)] =
				    *s++;
				dst[(w - j - 1) + ((h - i - 1) * dstPitch)] =
				    *s++;
			}
			src += srcPitch;
		}
		break;
	case RR_Rotate_270:
		h <<= 1;
		for (i = 0; i < h; i += 2) {
			s = src;
			for (j = 0; j < w; j++) {
				/* Copy Y */
				dst[(h - i - 2) + (j * dstPitch)] = *s++;
				(void)*s++;
			}
			src += srcPitch;
		}
		h >>= 1;
		src = buf + (top * srcPitch) + (left << 1);
		for (i = 0; i < h; i += 2) {
			for (j = 0; j < w; j += 2) {
				/* Copy U */
				dst[(((h - i) * 2) - 3) + (j * dstPitch)] =
				    src[(j * 2) + 1 + (i * srcPitch)];
				dst[(((h - i) * 2) - 3) +
				    ((j + 1) * dstPitch)] =
				    src[(j * 2) + 1 + ((i + 1) * srcPitch)];
				/* Copy V */
				dst[(((h - i) * 2) - 1) + (j * dstPitch)] =
				    src[(j * 2) + 3 + (i * srcPitch)];
				dst[(((h - i) * 2) - 1) +
				    ((j + 1) * dstPitch)] =
				    src[(j * 2) + 3 + ((i + 1) * srcPitch)];
			}
		}
		break;
	}

	drm_intel_gem_bo_unmap_gtt(adaptor_priv->buf);
	return TRUE;
}

static void intel_memcpy_plane(unsigned char *dst, unsigned char *src,
			       int height, int width,
			       int dstPitch, int srcPitch, Rotation rotation)
{
	int i, j = 0;
	unsigned char *s;

	switch (rotation) {
	case RR_Rotate_0:
		/* optimise for the case of no clipping */
		if (srcPitch == dstPitch && srcPitch == width)
			memcpy(dst, src, srcPitch * height);
		else
			for (i = 0; i < height; i++) {
				memcpy(dst, src, width);
				src += srcPitch;
				dst += dstPitch;
			}
		break;
	case RR_Rotate_90:
		for (i = 0; i < height; i++) {
			s = src;
			for (j = 0; j < width; j++) {
				dst[(i) + ((width - j - 1) * dstPitch)] = *s++;
			}
			src += srcPitch;
		}
		break;
	case RR_Rotate_180:
		for (i = 0; i < height; i++) {
			s = src;
			for (j = 0; j < width; j++) {
				dst[(width - j - 1) +
				    ((height - i - 1) * dstPitch)] = *s++;
			}
			src += srcPitch;
		}
		break;
	case RR_Rotate_270:
		for (i = 0; i < height; i++) {
			s = src;
			for (j = 0; j < width; j++) {
				dst[(height - i - 1) + (j * dstPitch)] = *s++;
			}
			src += srcPitch;
		}
		break;
	}
}

static Bool
intel_video_copy_planar_data(intel_adaptor_private *adaptor_priv,
		   unsigned char *buf, int srcPitch, int srcPitch2,
		   int dstPitch, int dstPitch2,
		   int srcH, int top, int left,
		   int h, int w, int id)
{
	unsigned char *src1, *src2, *src3, *dst_base, *dst1, *dst2, *dst3;

#if 0
	ErrorF("intel_video_copy_planar_data: srcPitch %d, srcPitch %d, dstPitch %d\n"
	       "nlines %d, npixels %d, top %d, left %d\n",
	       srcPitch, srcPitch2, dstPitch, h, w, top, left);
#endif

	/* Copy Y data */
	src1 = buf + (top * srcPitch) + left;
#if 0
	ErrorF("src1 is %p, offset is %ld\n", src1,
	       (unsigned long)src1 - (unsigned long)buf);
#endif

	if (drm_intel_gem_bo_map_gtt(adaptor_priv->buf))
		return FALSE;

	dst_base = adaptor_priv->buf->virtual;

	dst1 = dst_base + adaptor_priv->YBufOffset;

	intel_memcpy_plane(dst1, src1, h, w, dstPitch2, srcPitch,
			  adaptor_priv->rotation);

	/* Copy V data for YV12, or U data for I420 */
	src2 = buf +		/* start of YUV data */
	    (srcH * srcPitch) +	/* move over Luma plane */
	    ((top >> 1) * srcPitch2) +	/* move down from by top lines */
	    (left >> 1);	/* move left by left pixels */

#if 0
	ErrorF("src2 is %p, offset is %ld\n", src2,
	       (unsigned long)src2 - (unsigned long)buf);
#endif
	if (id == FOURCC_I420)
		dst2 = dst_base + adaptor_priv->UBufOffset;
	else
		dst2 = dst_base + adaptor_priv->VBufOffset;

	intel_memcpy_plane(dst2, src2, h / 2, w / 2,
			  dstPitch, srcPitch2, adaptor_priv->rotation);

	/* Copy U data for YV12, or V data for I420 */
	src3 = buf +		/* start of YUV data */
	    (srcH * srcPitch) +	/* move over Luma plane */
	    ((srcH >> 1) * srcPitch2) +	/* move over Chroma plane */
	    ((top >> 1) * srcPitch2) +	/* move down from by top lines */
	    (left >> 1);	/* move left by left pixels */
#if 0
	ErrorF("src3 is %p, offset is %ld\n", src3,
	       (unsigned long)src3 - (unsigned long)buf);
#endif
	if (id == FOURCC_I420)
		dst3 = dst_base + adaptor_priv->VBufOffset;
	else
		dst3 = dst_base + adaptor_priv->UBufOffset;

	intel_memcpy_plane(dst3, src3, h / 2, w / 2,
			  dstPitch, srcPitch2, adaptor_priv->rotation);

	drm_intel_gem_bo_unmap_gtt(adaptor_priv->buf);
	return TRUE;
}

void
intel_setup_dst_params(ScrnInfoPtr scrn, intel_adaptor_private *adaptor_priv, short width,
		       short height, int *dstPitch, int *dstPitch2, int *size,
		       int id)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	int pitchAlign;

	/* Only needs to be DWORD-aligned for textured on i915, but overlay has
	 * stricter requirements.
	 */
	if (adaptor_priv->textured) {
		pitchAlign = 4;
	} else {
		if (INTEL_INFO(intel)->gen >= 040)
			/* Actually the alignment is 64 bytes, too. But the
			 * stride must be at least 512 bytes. Take the easy fix
			 * and align on 512 bytes unconditionally. */
			pitchAlign = 512;
		else if (IS_I830(intel) || IS_845G(intel))
			/* Harsh, errata on these chipsets limit the stride to be
			 * a multiple of 256 bytes.
			 */
			pitchAlign = 256;
		else
			pitchAlign = 64;
	}

#if INTEL_XVMC
	/* for i915 xvmc, hw requires 1kb aligned surfaces */
	if ((id == FOURCC_XVMC) && IS_GEN3(intel))
		pitchAlign = 1024;
#endif

	/* Determine the desired destination pitch (representing the chroma's pitch,
	 * in the planar case.
	 */
	if (is_planar_fourcc(id)) {
		if (adaptor_priv->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
			*dstPitch = ALIGN((height / 2), pitchAlign);
			*dstPitch2 = ALIGN(height, pitchAlign);
			*size = *dstPitch * width * 3;
		} else {
			*dstPitch = ALIGN((width / 2), pitchAlign);
			*dstPitch2 = ALIGN(width, pitchAlign);
			*size = *dstPitch * height * 3;
		}
	} else {
		if (adaptor_priv->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
			*dstPitch = ALIGN((height << 1), pitchAlign);
			*size = *dstPitch * width;
		} else {
			*dstPitch = ALIGN((width << 1), pitchAlign);
			*size = *dstPitch * height;
		}
		*dstPitch2 = 0;
	}
#if 0
	ErrorF("srcPitch: %d, dstPitch: %d, size: %d\n", srcPitch, *dstPitch,
	       size);
#endif

	adaptor_priv->YBufOffset = 0;

	if (adaptor_priv->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
		adaptor_priv->UBufOffset =
		    adaptor_priv->YBufOffset + (*dstPitch2 * width);
		adaptor_priv->VBufOffset =
		    adaptor_priv->UBufOffset + (*dstPitch * width / 2);
	} else {
		adaptor_priv->UBufOffset =
		    adaptor_priv->YBufOffset + (*dstPitch2 * height);
		adaptor_priv->VBufOffset =
		    adaptor_priv->UBufOffset + (*dstPitch * height / 2);
	}
}

static Bool
intel_setup_video_buffer(ScrnInfoPtr scrn, intel_adaptor_private *adaptor_priv,
			 int alloc_size, int id, unsigned char *buf)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);

	/* Free the current buffer if we're going to have to reallocate */
	if (adaptor_priv->buf && adaptor_priv->buf->size < alloc_size)
		intel_free_video_buffers(adaptor_priv);

	if (adaptor_priv->buf == NULL) {
		adaptor_priv->buf = drm_intel_bo_alloc(intel->bufmgr, "xv buffer",
						       alloc_size, 4096);
		if (adaptor_priv->buf == NULL)
			return FALSE;

		adaptor_priv->reusable = TRUE;
	}

	return TRUE;
}

Bool
intel_video_copy_data(ScrnInfoPtr scrn, intel_adaptor_private *adaptor_priv,
                      short width, short height, int *dstPitch, int *dstPitch2,
                      int top, int left, int npixels, int nlines,
                      int id, unsigned char *buf)
{
	int srcPitch = 0, srcPitch2 = 0;
	int size;

	if (is_planar_fourcc(id)) {
		srcPitch = ALIGN(width, 0x4);
		srcPitch2 = ALIGN((width >> 1), 0x4);
	} else {
		srcPitch = width << 1;
	}

	intel_setup_dst_params(scrn, adaptor_priv, width, height, dstPitch,
				dstPitch2, &size, id);

	if (!intel_setup_video_buffer(scrn, adaptor_priv, size, id, buf))
		return FALSE;

	/* copy data */
	if (is_planar_fourcc(id)) {
		return intel_video_copy_planar_data(adaptor_priv, buf, srcPitch, srcPitch2,
					  *dstPitch, *dstPitch2,
					  height, top, left, nlines,
					  npixels, id);
	} else {
		return intel_video_copy_packed_data(adaptor_priv, buf, srcPitch, *dstPitch, top, left,
					  nlines, npixels);
	}
}

int is_planar_fourcc(int id)
{
	switch (id) {
	case FOURCC_YV12:
	case FOURCC_I420:
#ifdef INTEL_XVMC
	case FOURCC_XVMC:
#endif
		return 1;
	case FOURCC_UYVY:
	case FOURCC_YUY2:
		return 0;
	default:
		ErrorF("Unknown format 0x%x\n", id);
		return 0;
	}
}

Bool
intel_clip_video_helper(ScrnInfoPtr scrn,
			intel_adaptor_private *adaptor_priv,
			xf86CrtcPtr * crtc_ret,
			BoxPtr dst,
			short src_x, short src_y,
			short drw_x, short drw_y,
			short src_w, short src_h,
			short drw_w, short drw_h,
			int id,
			int *top, int* left, int* npixels, int *nlines,
			RegionPtr reg, INT32 width, INT32 height)
{
	Bool ret;
	RegionRec crtc_region_local;
	RegionPtr crtc_region = reg;
	BoxRec crtc_box;
	INT32 x1, x2, y1, y2;
	xf86CrtcPtr crtc;

	x1 = src_x;
	x2 = src_x + src_w;
	y1 = src_y;
	y2 = src_y + src_h;

	dst->x1 = drw_x;
	dst->x2 = drw_x + drw_w;
	dst->y1 = drw_y;
	dst->y2 = drw_y + drw_h;

	/*
	 * For overlay video, compute the relevant CRTC and
	 * clip video to that
	 */
	crtc = intel_covering_crtc(scrn, dst, adaptor_priv->desired_crtc,
				   &crtc_box);

	/* For textured video, we don't actually want to clip at all. */
	if (crtc && !adaptor_priv->textured) {
		REGION_INIT(screen, &crtc_region_local, &crtc_box, 1);
		crtc_region = &crtc_region_local;
		REGION_INTERSECT(screen, crtc_region, crtc_region,
				 reg);
	}
	*crtc_ret = crtc;

	ret = xf86XVClipVideoHelper(dst, &x1, &x2, &y1, &y2,
				    crtc_region, width, height);
	if (crtc_region != reg)
		REGION_UNINIT(screen, &crtc_region_local);

	*top = y1 >> 16;
	*left = (x1 >> 16) & ~1;
	*npixels = ALIGN(((x2 + 0xffff) >> 16), 2) - *left;
	if (is_planar_fourcc(id)) {
		*top &= ~1;
		*nlines = ALIGN(((y2 + 0xffff) >> 16), 2) - *top;
	} else
		*nlines = ((y2 + 0xffff) >> 16) - *top;

	return ret;
}

int
intel_video_query_image_attributes(ScrnInfoPtr scrn,
                                   int id,
                                   unsigned short *w, unsigned short *h,
                                   int *pitches, int *offsets)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	int size, tmp;

#if 0
	ErrorF("intel_video_query_image_attributes: w is %d, h is %d\n", *w, *h);
#endif

	if (IS_845G(intel) || IS_I830(intel)) {
		if (*w > IMAGE_MAX_WIDTH_LEGACY)
			*w = IMAGE_MAX_WIDTH_LEGACY;
		if (*h > IMAGE_MAX_HEIGHT_LEGACY)
			*h = IMAGE_MAX_HEIGHT_LEGACY;
	} else {
		if (*w > IMAGE_MAX_WIDTH)
			*w = IMAGE_MAX_WIDTH;
		if (*h > IMAGE_MAX_HEIGHT)
			*h = IMAGE_MAX_HEIGHT;
	}

	*w = (*w + 1) & ~1;
	if (offsets)
		offsets[0] = 0;

	switch (id) {
		/* IA44 is for XvMC only */
	case FOURCC_IA44:
	case FOURCC_AI44:
		if (pitches)
			pitches[0] = *w;
		size = *w * *h;
		break;
	case FOURCC_YV12:
	case FOURCC_I420:
		*h = (*h + 1) & ~1;
		size = (*w + 3) & ~3;
		if (pitches)
			pitches[0] = size;
		size *= *h;
		if (offsets)
			offsets[1] = size;
		tmp = ((*w >> 1) + 3) & ~3;
		if (pitches)
			pitches[1] = pitches[2] = tmp;
		tmp *= (*h >> 1);
		size += tmp;
		if (offsets)
			offsets[2] = size;
		size += tmp;
#if 0
		if (pitches)
			ErrorF("pitch 0 is %d, pitch 1 is %d, pitch 2 is %d\n",
			       pitches[0], pitches[1], pitches[2]);
		if (offsets)
			ErrorF("offset 1 is %d, offset 2 is %d\n", offsets[1],
			       offsets[2]);
		if (offsets)
			ErrorF("size is %d\n", size);
#endif
		break;
#ifdef INTEL_XVMC
	case FOURCC_XVMC:
		*h = (*h + 1) & ~1;
		size = sizeof(struct intel_xvmc_command);
		if (pitches)
			pitches[0] = size;
		break;
#endif
	case FOURCC_UYVY:
	case FOURCC_YUY2:
	default:
		size = *w << 1;
		if (pitches)
			pitches[0] = size;
		size *= *h;
		break;
	}

	return size;
}

void intel_video_stop_video(ScrnInfoPtr scrn, pointer data, Bool shutdown)
{
	intel_adaptor_private *adaptor_priv = (intel_adaptor_private *) data;

	if (adaptor_priv->textured)
		return;

	REGION_EMPTY(scrn->pScreen, &adaptor_priv->clip);

	if (shutdown) {
		if (adaptor_priv->videoStatus & CLIENT_VIDEO_ON)
			intel_video_overlay_off(intel_get_screen_private(scrn));

		intel_free_video_buffers(adaptor_priv);
		adaptor_priv->videoStatus = 0;
	} else {
		if (adaptor_priv->videoStatus & CLIENT_VIDEO_ON) {
			adaptor_priv->videoStatus |= OFF_TIMER;
			adaptor_priv->offTime = currentTime.milliseconds + OFF_DELAY;
		}
	}

}

void
intel_video_block_handler(intel_screen_private *intel)
{
	intel_adaptor_private *adaptor_priv;

	/* no overlay */
	if (intel->adaptor == NULL)
		return;

	adaptor_priv = intel_get_adaptor_private(intel);
	if (adaptor_priv->videoStatus & OFF_TIMER) {
		Time now = currentTime.milliseconds;
		if (adaptor_priv->offTime < now) {
			/* Turn off the overlay */
			intel_video_overlay_off(intel);
			intel_free_video_buffers(adaptor_priv);
			adaptor_priv->videoStatus = 0;
		}
	}
}
