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

#ifndef _INTEL_VIDEO_H_
#define _INTEL_VIDEO_H_

#include "xf86.h"
#include "xf86_OSproc.h"

typedef struct {
	uint32_t YBufOffset;
	uint32_t UBufOffset;
	uint32_t VBufOffset;

	int brightness;
	int contrast;
	int saturation;
	xf86CrtcPtr desired_crtc;

	RegionRec clip;
	uint32_t colorKey;

	uint32_t gamma0;
	uint32_t gamma1;
	uint32_t gamma2;
	uint32_t gamma3;
	uint32_t gamma4;
	uint32_t gamma5;

	/* only used by the overlay */
	uint32_t videoStatus;
	Time offTime;
	Time freeTime;
	/** YUV data buffers */
	drm_intel_bo *buf, *old_buf[2];
	Bool reusable;

	Bool textured;
	Rotation rotation;	/* should remove intel->rotation later */

	int SyncToVblank;	/* -1: auto, 0: off, 1: on */
} intel_adaptor_private;

#define OFF_DELAY	250	/* milliseconds */

#define OFF_TIMER	0x01
#define CLIENT_VIDEO_ON	0x02

static inline intel_adaptor_private *
intel_get_adaptor_private(intel_screen_private *intel)
{
	return intel->adaptor->pPortPrivates[0].ptr;
}

int is_planar_fourcc(int id);

void intel_video_block_handler(intel_screen_private *intel);

int intel_video_query_image_attributes(ScrnInfoPtr, int, unsigned short *,
                                       unsigned short *, int *, int *);

Bool
intel_video_copy_data(ScrnInfoPtr scrn, intel_adaptor_private *adaptor_priv,
                      short width, short height, int *dstPitch, int *dstPitch2,
                      int top, int left, int npixels, int nlines,
                      int id, unsigned char *buf);

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
			RegionPtr reg, INT32 width, INT32 height);

void
intel_free_video_buffers(intel_adaptor_private *adaptor_priv);

int
intel_video_get_port_attribute(ScrnInfoPtr scrn,
                               Atom attribute, INT32 * value, pointer data);

void
intel_video_query_best_size(ScrnInfoPtr, Bool,
                            short, short, short, short, unsigned int *,
                            unsigned int *, pointer);

void
intel_setup_dst_params(ScrnInfoPtr scrn, intel_adaptor_private *adaptor_priv, short width,
		       short height, int *dstPitch, int *dstPitch2, int *size,
		       int id);

void intel_video_stop_video(ScrnInfoPtr scrn, pointer data, Bool shutdown);

extern Atom intel_xv_Brightness, intel_xv_Contrast, intel_xv_Saturation, intel_xv_ColorKey, intel_xv_Pipe;
extern Atom intel_xv_Gamma0, intel_xv_Gamma1, intel_xv_Gamma2, intel_xv_Gamma3, intel_xv_Gamma4, intel_xv_Gamma5;
extern Atom intel_xv_SyncToVblank;

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

/* Limits for the overlay/textured video source sizes.  The documented hardware
 * limits are 2048x2048 or better for overlay and both of our textured video
 * implementations.  Additionally, on the 830 and 845, larger sizes resulted in
 * the card hanging, so we keep the limits lower there.
 */
#define IMAGE_MAX_WIDTH		2048
#define IMAGE_MAX_HEIGHT	2048
#define IMAGE_MAX_WIDTH_LEGACY	1024
#define IMAGE_MAX_HEIGHT_LEGACY	1088

extern const XF86VideoEncodingRec intel_xv_dummy_encoding[1];

#define NUM_FORMATS 3

extern XF86VideoFormatRec intel_xv_formats[NUM_FORMATS];

#define NUM_ATTRIBUTES 5

extern XF86AttributeRec intel_xv_attributes[NUM_ATTRIBUTES];

#define GAMMA_ATTRIBUTES 6

extern XF86AttributeRec intel_xv_gamma_attributes[GAMMA_ATTRIBUTES];

#ifdef INTEL_XVMC
#define NUM_IMAGES 5
#define XVMC_IMAGE 1
#else
#define NUM_IMAGES 4
#define XVMC_IMAGE 0
#endif

extern XF86ImageRec intel_xv_images[NUM_IMAGES];

#endif /* _INTEL_VIDEO_H_ */
