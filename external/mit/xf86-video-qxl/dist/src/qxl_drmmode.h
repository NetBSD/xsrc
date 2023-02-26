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

#ifdef XF86DRM_MODE

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "xf86str.h"
#include "randrstr.h"
#include "xf86Crtc.h"
#ifdef HAVE_LIBUDEV
#include "libudev.h"
#endif

typedef struct {
  int fd;
  unsigned fb_id;
  drmModeResPtr mode_res;
  drmModeFBPtr mode_fb;
  int cpp;
  ScrnInfoPtr scrn;
#ifdef HAVE_LIBUDEV
  struct udev_monitor *uevent_monitor;
  InputHandlerProc uevent_handler;
#endif
} drmmode_rec, *drmmode_ptr;

typedef struct {
    drmmode_ptr drmmode;
    drmModeCrtcPtr mode_crtc;
    int hw_id;
    struct qxl_bo *cursor_bo;
    void *cursor_ptr;
  //    struct radeon_bo *rotate_bo;
    unsigned rotate_fb_id;
    int dpms_mode;
    uint16_t lut_r[256], lut_g[256], lut_b[256];
} drmmode_crtc_private_rec, *drmmode_crtc_private_ptr;


typedef struct {
    drmModePropertyPtr mode_prop;
    uint64_t value;
    int num_atoms; /* if range prop, num_atoms == 1; if enum prop, num_atoms == num_enums + 1 */
    Atom *atoms;
    int index; /* index within the kernel-size property array */
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

extern Bool drmmode_pre_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, int cpp);

extern void qxl_drmmode_uevent_init(ScrnInfoPtr scrn, drmmode_ptr drmmode);
extern void qxl_drmmode_uevent_fini(ScrnInfoPtr scrn, drmmode_ptr drmmode);
#endif

#endif
