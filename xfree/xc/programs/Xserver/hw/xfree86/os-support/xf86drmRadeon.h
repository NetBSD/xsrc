/* xf86drmRadeon.h -- OS-independent header for Radeon DRM user-level
 *                    library interface
 *
 * Copyright 2000 VA Linux Systems, Inc., Fremont, California.
 * All Rights Reserved.
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
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *   Gareth Hughes <gareth@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/os-support/xf86drmRadeon.h,v 1.6 2001/04/16 15:02:13 tsi Exp $
 *
 */

#ifndef _XF86DRI_RADEON_H_
#define _XF86DRI_RADEON_H_

/* WARNING: If you change any of these defines, make sure to change
 * the kernel include file as well (radeon_drm.h)
 */

#define DRM_RADEON_FRONT	0x1
#define DRM_RADEON_BACK		0x2
#define DRM_RADEON_DEPTH	0x4

typedef struct {
   unsigned long sarea_priv_offset;
   int is_pci;
   int cp_mode;
   int agp_size;
   int ring_size;
   int usec_timeout;

   unsigned int fb_bpp;
   unsigned int front_offset, front_pitch;
   unsigned int back_offset, back_pitch;
   unsigned int depth_bpp;
   unsigned int depth_offset, depth_pitch;

   unsigned long fb_offset;
   unsigned long mmio_offset;
   unsigned long ring_offset;
   unsigned long ring_rptr_offset;
   unsigned long buffers_offset;
   unsigned long agp_textures_offset;
} drmRadeonInit;

typedef struct {
   unsigned int x;
   unsigned int y;
   unsigned int width;
   unsigned int height;
   void *data;
} drmRadeonTexImage;

extern int drmRadeonInitCP( int fd, drmRadeonInit *info );
extern int drmRadeonCleanupCP( int fd );

extern int drmRadeonStartCP( int fd );
extern int drmRadeonStopCP( int fd );
extern int drmRadeonResetCP( int fd );
extern int drmRadeonWaitForIdleCP( int fd );

extern int drmRadeonEngineReset( int fd );

extern int drmRadeonFullScreen( int fd, int enable );

extern int drmRadeonSwapBuffers( int fd );
extern int drmRadeonClear( int fd, unsigned int flags,
			   unsigned int clear_color, unsigned int clear_depth,
			   unsigned int color_mask, unsigned int depth_mask,
			   void *boxes, int nbox );

extern int drmRadeonFlushVertexBuffer( int fd, int prim, int indx,
				       int count, int discard );
extern int drmRadeonFlushIndices( int fd, int prim, int indx,
				  int start, int end, int discard );

extern int drmRadeonLoadTexture( int fd, int offset, int pitch, int format,
				 int width, int height,
				 drmRadeonTexImage *image );

extern int drmRadeonPolygonStipple( int fd, unsigned int *mask );

extern int drmRadeonFlushIndirectBuffer( int fd, int indx,
					 int start, int end, int discard );

#endif
