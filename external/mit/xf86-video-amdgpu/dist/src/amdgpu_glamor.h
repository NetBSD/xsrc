/*
 * Copyright © 2011 Intel Corporation.
 *             2012 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including
 * the next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMDGPU_GLAMOR_H
#define AMDGPU_GLAMOR_H

#include "xf86xv.h"

#define GLAMOR_FOR_XORG  1
#include <glamor.h>

#ifndef CREATE_PIXMAP_USAGE_SHARED
#define CREATE_PIXMAP_USAGE_SHARED AMDGPU_CREATE_PIXMAP_DRI2
#endif

#define AMDGPU_CREATE_PIXMAP_SHARED(usage) \
	((usage) == AMDGPU_CREATE_PIXMAP_DRI2 || \
	 (usage) == CREATE_PIXMAP_USAGE_SHARED)

#ifndef GLAMOR_NO_DRI3
#define GLAMOR_NO_DRI3 0
#define glamor_fd_from_pixmap glamor_dri3_fd_from_pixmap
#define glamor_pixmap_from_fd glamor_egl_dri3_pixmap_from_fd
#endif

#ifndef GLAMOR_INVERTED_Y_AXIS
#define GLAMOR_INVERTED_Y_AXIS 0
#endif
#ifndef GLAMOR_USE_SCREEN
#define GLAMOR_USE_SCREEN 0
#endif
#ifndef GLAMOR_USE_PICTURE_SCREEN
#define GLAMOR_USE_PICTURE_SCREEN 0
#endif

struct amdgpu_pixmap;

Bool amdgpu_glamor_pre_init(ScrnInfoPtr scrn);
Bool amdgpu_glamor_init(ScreenPtr screen);
void amdgpu_glamor_fini(ScreenPtr screen);
void amdgpu_glamor_screen_init(ScreenPtr screen);
Bool amdgpu_glamor_create_screen_resources(ScreenPtr screen);
void amdgpu_glamor_free_screen(int scrnIndex, int flags);

void amdgpu_glamor_flush(ScrnInfoPtr pScrn);
void amdgpu_glamor_finish(ScrnInfoPtr pScrn);

Bool
amdgpu_glamor_create_textured_pixmap(PixmapPtr pixmap, struct amdgpu_buffer *bo);
void amdgpu_glamor_exchange_buffers(PixmapPtr src, PixmapPtr dst);
PixmapPtr amdgpu_glamor_set_pixmap_bo(DrawablePtr drawable, PixmapPtr pixmap);

XF86VideoAdaptorPtr amdgpu_glamor_xv_init(ScreenPtr pScreen, int num_adapt);

#endif /* AMDGPU_GLAMOR_H */
