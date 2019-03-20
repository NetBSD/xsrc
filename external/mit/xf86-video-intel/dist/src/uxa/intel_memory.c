/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright © 2002 by David Dawes.

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
IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keith@tungstengraphics.com>
 *   David Dawes <dawes@xfree86.org>
 *
 * Updated for Dual Head capabilities:
 *   Alan Hourihane <alanh@tungstengraphics.com>
 */

/**
 * @file intel_memory.c
 *
 * This is the video memory allocator.  Our memory allocation is different from
 * other graphics chips, where you have a fixed amount of graphics memory
 * available that you want to put to the best use.  Instead, we have almost no
 * memory pre-allocated, and we have to choose an appropriate amount of system
 * memory to use.
 *
 * The allocations we might do:
 *
 * - Ring buffer
 * - HW cursor block (either one block or four)
 * - Overlay registers
 * - Front buffer (screen 1)
 * - Front buffer (screen 2, only in zaphod mode)
 * - Back/depth buffer (3D only)
 * - Compatibility texture pool (optional, more is always better)
 * - New texture pool (optional, more is always better.  aperture allocation
 *     only)
 *
 * The user may request a specific amount of memory to be used
 * (intel->pEnt->videoRam != 0), in which case allocations have to fit within
 * that much aperture.  If not, the individual allocations will be
 * automatically sized, and will be fit within the maximum aperture size.
 * Only the actual memory used (not alignment padding) will get actual AGP
 * memory allocated.
 *
 * Given that the allocations listed are generally a page or more than a page,
 * our allocator will only return page-aligned offsets, simplifying the memory
 * binding process.  For smaller allocations, the acceleration architecture's
 * linear allocator is preferred.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "xorg-server.h"
#include "xf86.h"
#include "xf86_OSproc.h"

#include "intel.h"
#include "i915_drm.h"

/**
 * Returns the fence size for a tiled area of the given size.
 */
unsigned long intel_get_fence_size(intel_screen_private *intel, unsigned long size)
{
	unsigned long i;
	unsigned long start;

	if (INTEL_INFO(intel)->gen >= 040 || intel->has_relaxed_fencing) {
		/* The 965 can have fences at any page boundary. */
		return ALIGN(size, 4096);
	} else {
		/* Align the size to a power of two greater than the smallest fence
		 * size.
		 */
		if (IS_GEN3(intel))
			start = MB(1);
		else
			start = KB(512);

		for (i = start; i < size; i <<= 1) ;

		return i;
	}
}

/**
 * On some chips, pitch width has to be a power of two tile width, so
 * calculate that here.
 */
unsigned long
intel_get_fence_pitch(intel_screen_private *intel, unsigned long pitch,
		     uint32_t tiling_mode)
{
	unsigned long i;
	unsigned long tile_width = (tiling_mode == I915_TILING_Y) ? 128 : 512;

	if (tiling_mode == I915_TILING_NONE)
		return pitch;

	/* 965+ is flexible */
	if (INTEL_INFO(intel)->gen >= 040)
		return ALIGN(pitch, tile_width);

	/* Pre-965 needs power of two tile width */
	for (i = tile_width; i < pitch; i <<= 1) ;

	return i;
}

Bool intel_check_display_stride(ScrnInfoPtr scrn, int stride, Bool tiling)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	int limit;

	/* 8xx spec has always 8K limit, but tests show larger limit in
	   non-tiling mode, which makes large monitor work. */
	if (tiling) {
		if (IS_GEN2(intel))
			limit = KB(8);
		else if (IS_GEN3(intel))
			limit = KB(8);
		else if (IS_GEN4(intel))
			limit = KB(16);
		else
			limit = KB(32);
	} else
		limit = KB(32);

	return stride <= limit;
}

static size_t
agp_aperture_size(struct pci_device *dev, int gen)
{
	return dev->regions[gen < 030 ? 0 : 2].size;
}

void intel_set_gem_max_sizes(ScrnInfoPtr scrn)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	size_t agp_size = agp_aperture_size(xf86GetPciInfoForEntity(intel->pEnt->index),
					    INTEL_INFO(intel)->gen);

	/* The chances of being able to mmap an object larger than
	 * agp_size/2 are slim. Moreover, we may be forced to fallback
	 * using a gtt mapping as both the source and a mask, as well
	 * as a destination and all need to fit into the aperture.
	 */
	intel->max_gtt_map_size = agp_size / 4;

	/* Let objects be tiled up to the size where only 4 would fit in
	 * the aperture, presuming best case alignment. Also if we
	 * cannot mmap it using the GTT we will be stuck. */
	intel->max_tiling_size = intel->max_gtt_map_size;

	/* Large BOs will tend to hit SW fallbacks frequently, and also will
	 * tend to fail to successfully map when doing SW fallbacks because we
	 * overcommit address space for BO access, or worse cause aperture
	 * thrashing.
	 */
	intel->max_bo_size = intel->max_gtt_map_size;
}

unsigned int
intel_compute_size(struct intel_screen_private *intel,
                   int w, int h, int bpp, unsigned usage,
                   uint32_t *tiling, int *stride)
{
	int pitch, size;

	if (*tiling != I915_TILING_NONE) {
		/* First check whether tiling is necessary. */
		pitch = (w * bpp  + 7) / 8;
		pitch = ALIGN(pitch, 64);
		size = pitch * ALIGN (h, 2);
		if (INTEL_INFO(intel)->gen < 040) {
			/* Gen 2/3 has a maximum stride for tiling of
			 * 8192 bytes.
			 */
			if (pitch > KB(8))
				*tiling = I915_TILING_NONE;

			/* Narrower than half a tile? */
			if (pitch < 256)
				*tiling = I915_TILING_NONE;

			/* Older hardware requires fences to be pot size
			 * aligned with a minimum of 1 MiB, so causes
			 * massive overallocation for small textures.
			 */
			if (size < 1024*1024/2 && !intel->has_relaxed_fencing)
				*tiling = I915_TILING_NONE;
		} else if (!(usage & INTEL_CREATE_PIXMAP_DRI2) && size <= 4096) {
			/* Disable tiling beneath a page size, we will not see
			 * any benefit from reducing TLB misses and instead
			 * just incur extra cost when we require a fence.
			 */
			*tiling = I915_TILING_NONE;
		}
	}

	pitch = (w * bpp + 7) / 8;
	if (!(usage & INTEL_CREATE_PIXMAP_DRI2) && pitch <= 256)
		*tiling = I915_TILING_NONE;

	if (*tiling != I915_TILING_NONE) {
		int aligned_h, tile_height;

		if (IS_GEN2(intel))
			tile_height = 16;
		else if (*tiling == I915_TILING_X)
			tile_height = 8;
		else
			tile_height = 32;
		aligned_h = ALIGN(h, tile_height);

		*stride = intel_get_fence_pitch(intel,
						ALIGN(pitch, 512),
						*tiling);

		/* Round the object up to the size of the fence it will live in
		 * if necessary.  We could potentially make the kernel allocate
		 * a larger aperture space and just bind the subset of pages in,
		 * but this is easier and also keeps us out of trouble (as much)
		 * with drm_intel_bufmgr_check_aperture().
		 */
		size = intel_get_fence_size(intel, *stride * aligned_h);

		if (size > intel->max_tiling_size)
			*tiling = I915_TILING_NONE;
	}

	if (*tiling == I915_TILING_NONE) {
		/* We only require a 64 byte alignment for scanouts, but
		 * a 256 byte alignment for sharing with PRIME.
		 */
		*stride = ALIGN(pitch, 256);
		/* Round the height up so that the GPU's access to a 2x2 aligned
		 * subspan doesn't address an invalid page offset beyond the
		 * end of the GTT.
		 */
		size = *stride * ALIGN(h, 2);
	}

	return size;
}

drm_intel_bo *intel_allocate_framebuffer(ScrnInfoPtr scrn,
					 int width, int height, int cpp,
					 int *out_stride,
					 uint32_t *out_tiling)
{
	intel_screen_private *intel = intel_get_screen_private(scrn);
	uint32_t tiling;
	int stride, size;
	drm_intel_bo *bo;

	intel_set_gem_max_sizes(scrn);

	if (intel->tiling & INTEL_TILING_FB)
		tiling = I915_TILING_X;
	else
		tiling = I915_TILING_NONE;

retry:
	size = intel_compute_size(intel,
                                  width, height,
                                  intel->cpp*8, 0,
                                  &tiling, &stride);
	if (!intel_check_display_stride(scrn, stride, tiling)) {
		if (tiling != I915_TILING_NONE) {
			tiling = I915_TILING_NONE;
			goto retry;
		}

		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "Front buffer stride %d kB "
			   "exceeds display limit\n", stride / 1024);
		return NULL;
	}

	bo = drm_intel_bo_alloc(intel->bufmgr, "front buffer", size, 0);
	if (bo == NULL)
		return FALSE;

	if (tiling != I915_TILING_NONE)
		drm_intel_bo_set_tiling(bo, &tiling, stride);

	xf86DrvMsg(scrn->scrnIndex, X_INFO,
		   "Allocated new frame buffer %dx%d stride %d, %s\n",
		   width, height, stride,
		   tiling == I915_TILING_NONE ? "untiled" : "tiled");

	drm_intel_bo_disable_reuse(bo);

	*out_stride = stride;
	*out_tiling = tiling;
	return bo;
}

