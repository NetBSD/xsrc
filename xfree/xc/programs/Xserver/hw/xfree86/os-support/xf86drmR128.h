/* xf86drm.h -- OS-independent header for DRM user-level library interface
 * Created: Sun Apr  9 18:16:28 2000 by kevin@precisioninsight.com
 *
 * Copyright 1999, 2000 Precision Insight, Inc., Cedar Park, Texas.
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
 * $XFree86: xc/programs/Xserver/hw/xfree86/os-support/xf86drmR128.h,v 3.11 2001/04/16 15:02:13 tsi Exp $
 *
 */

#ifndef _XF86DRI_R128_H_
#define _XF86DRI_R128_H_

#include "X11/Xmd.h"

/*
 * WARNING: If you change any of these defines, make sure to change
 * the kernel include file as well (r128_drm.h)
 */

#define DRM_R128_FRONT		0x1
#define DRM_R128_BACK		0x2
#define DRM_R128_DEPTH		0x4

typedef struct {
   unsigned long sarea_priv_offset;
   int is_pci;
   int cce_mode;
   int cce_secure;		/* FIXME: Deprecated, we should remove this */
   int ring_size;
   int usec_timeout;

   unsigned int fb_bpp;
   unsigned int front_offset, front_pitch;
   unsigned int back_offset, back_pitch;
   unsigned int depth_bpp;
   unsigned int depth_offset, depth_pitch;
   unsigned int span_offset;

   unsigned long fb_offset;
   unsigned long mmio_offset;
   unsigned long ring_offset;
   unsigned long ring_rptr_offset;
   unsigned long buffers_offset;
   unsigned long agp_textures_offset;
} drmR128Init;

extern int drmR128InitCCE( int fd, drmR128Init *info );
extern int drmR128CleanupCCE( int fd );

extern int drmR128StartCCE( int fd );
extern int drmR128StopCCE( int fd );
extern int drmR128ResetCCE( int fd );
extern int drmR128WaitForIdleCCE( int fd );

extern int drmR128EngineReset( int fd );

extern int drmR128FullScreen( int fd, int enable );

extern int drmR128SwapBuffers( int fd );
extern int drmR128Clear( int fd, unsigned int flags,
			 unsigned int clear_color, unsigned int clear_depth,
			 unsigned int color_mask, unsigned int depth_mask );

extern int drmR128FlushVertexBuffer( int fd, int prim, int indx,
				     int count, int discard );
extern int drmR128FlushIndices( int fd, int prim, int indx,
				int start, int end, int discard );

extern int drmR128TextureBlit( int fd, int indx,
			       int offset, int pitch, int format,
			       int x, int y, int width, int height );

extern int drmR128WriteDepthSpan( int fd, int n, int x, int y,
				  const unsigned int depth[],
				  const unsigned char mask[] );
extern int drmR128WriteDepthPixels( int fd, int n,
				    const int x[], const int y[],
				    const unsigned int depth[],
				    const unsigned char mask[] );
extern int drmR128ReadDepthSpan( int fd, int n, int x, int y );
extern int drmR128ReadDepthPixels( int fd, int n,
				   const int x[], const int y[] );

extern int drmR128PolygonStipple( int fd, unsigned int *mask );

extern int drmR128FlushIndirectBuffer( int fd, int indx,
				       int start, int end, int discard );

#endif
