/* xf86drmRadeon.c -- User-level interface to Radeon DRM device
 *
 * Copyright 2000 VA Linx Systems, Inc., Fremont, California.
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
 * Authors:
 *   Gareth Hughes <gareth@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/drm/xf86drmRadeon.c,v 1.3 2001/04/10 16:08:04 dawes Exp $ */

#ifdef XFree86Server
# include "xf86.h"
# include "xf86_OSproc.h"
# include "xf86_ansic.h"
# define _DRM_MALLOC xalloc
# define _DRM_FREE   xfree
# ifndef XFree86LOADER
#  include <sys/mman.h>
# endif
#else
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <ctype.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/mman.h>
# include <sys/time.h>
# ifdef DRM_USE_MALLOC
#  define _DRM_MALLOC malloc
#  define _DRM_FREE   free
extern int xf86InstallSIGIOHandler(int fd, void (*f)(int, void *), void *);
extern int xf86RemoveSIGIOHandler(int fd);
# else
#  include <Xlibint.h>
#  define _DRM_MALLOC Xmalloc
#  define _DRM_FREE   Xfree
# endif
#endif

/* Not all systems have MAP_FAILED defined */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#ifdef __linux__
#include <sys/sysmacros.h>	/* for makedev() */
#endif
#include "xf86drm.h"
#include "xf86drmRadeon.h"
#include "drm.h"

#define RADEON_BUFFER_RETRY	32
#define RADEON_IDLE_RETRY	16


int drmRadeonInitCP( int fd, drmRadeonInit *info )
{
   drm_radeon_init_t init;

   memset( &init, 0, sizeof(drm_radeon_init_t) );

   init.func			= RADEON_INIT_CP;
   init.sarea_priv_offset	= info->sarea_priv_offset;
   init.is_pci			= info->is_pci;
   init.cp_mode			= info->cp_mode;
   init.agp_size		= info->agp_size;
   init.ring_size		= info->ring_size;
   init.usec_timeout		= info->usec_timeout;

   init.fb_bpp			= info->fb_bpp;
   init.front_offset		= info->front_offset;
   init.front_pitch		= info->front_pitch;
   init.back_offset		= info->back_offset;
   init.back_pitch		= info->back_pitch;

   init.depth_bpp		= info->depth_bpp;
   init.depth_offset		= info->depth_offset;
   init.depth_pitch		= info->depth_pitch;

   init.fb_offset		= info->fb_offset;
   init.mmio_offset		= info->mmio_offset;
   init.ring_offset		= info->ring_offset;
   init.ring_rptr_offset	= info->ring_rptr_offset;
   init.buffers_offset		= info->buffers_offset;
   init.agp_textures_offset	= info->agp_textures_offset;

   if ( ioctl( fd, DRM_IOCTL_RADEON_CP_INIT, &init ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonCleanupCP( int fd )
{
   drm_radeon_init_t init;

   memset( &init, 0, sizeof(drm_radeon_init_t) );

   init.func = RADEON_CLEANUP_CP;

   if ( ioctl( fd, DRM_IOCTL_RADEON_CP_INIT, &init ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonStartCP( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_RADEON_CP_START, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonStopCP( int fd )
{
   drm_radeon_cp_stop_t stop;
   int ret, i = 0;

   stop.flush = 1;
   stop.idle = 1;

   ret = ioctl( fd, DRM_IOCTL_RADEON_CP_STOP, &stop );

   if ( ret == 0 ) {
      return 0;
   } else if ( errno != EBUSY ) {
      return -errno;
   }

   stop.flush = 0;

   do {
      ret = ioctl( fd, DRM_IOCTL_RADEON_CP_STOP, &stop );
   } while ( ret && errno == EBUSY && i++ < RADEON_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else if ( errno != EBUSY ) {
      return -errno;
   }

   stop.idle = 0;

   if ( ioctl( fd, DRM_IOCTL_RADEON_CP_STOP, &stop ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonResetCP( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_RADEON_CP_RESET, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonWaitForIdleCP( int fd )
{
   int ret, i = 0;

   do {
      ret = ioctl( fd, DRM_IOCTL_RADEON_CP_IDLE, NULL );
   } while ( ret && errno == EBUSY && i++ < RADEON_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmRadeonEngineReset( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_RADEON_RESET, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonFullScreen( int fd, int enable )
{
   drm_radeon_fullscreen_t fs;

   if ( enable ) {
      fs.func = RADEON_INIT_FULLSCREEN;
   } else {
      fs.func = RADEON_CLEANUP_FULLSCREEN;
   }

   if ( ioctl( fd, DRM_IOCTL_RADEON_FULLSCREEN, &fs ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonSwapBuffers( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_RADEON_SWAP, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonClear( int fd, unsigned int flags,
		    unsigned int clear_color, unsigned int clear_depth,
		    unsigned int color_mask, unsigned int depth_mask,
		    void *b, int nbox )
{
   drm_radeon_clear_t clear;
   drm_radeon_clear_rect_t depth_boxes[RADEON_NR_SAREA_CLIPRECTS];
   drm_clip_rect_t *boxes = (drm_clip_rect_t *)b;
   int i;

   clear.flags = flags;
   clear.clear_color = clear_color;
   clear.clear_depth = clear_depth;
   clear.color_mask = color_mask;
   clear.depth_mask = depth_mask;
   clear.depth_boxes = depth_boxes;

   /* We can remove this when we do real depth clears, instead of
    * rendering a rectangle into the depth buffer.  This prevents
    * floating point calculations being done in the kernel.
    */
   for ( i = 0 ; i < nbox ; i++ ) {
      depth_boxes[i].f[CLEAR_X1] = (float)boxes[i].x1;
      depth_boxes[i].f[CLEAR_Y1] = (float)boxes[i].y1;
      depth_boxes[i].f[CLEAR_X2] = (float)boxes[i].x2;
      depth_boxes[i].f[CLEAR_Y2] = (float)boxes[i].y2;
      depth_boxes[i].f[CLEAR_DEPTH] = (float)clear_depth;
   }

   if ( ioctl( fd, DRM_IOCTL_RADEON_CLEAR, &clear ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonFlushVertexBuffer( int fd, int prim, int index,
				int count, int discard )
{
   drm_radeon_vertex_t v;

   v.prim = prim;
   v.idx = index;
   v.count = count;
   v.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_RADEON_VERTEX, &v ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonFlushIndices( int fd, int prim, int index,
			   int start, int end, int discard )
{
   drm_radeon_indices_t elts;

   elts.prim = prim;
   elts.idx = index;
   elts.start = start;
   elts.end = end;
   elts.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_RADEON_INDICES, &elts ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonLoadTexture( int fd, int offset, int pitch, int format,
			  int width, int height, drmRadeonTexImage *image )
{
   drm_radeon_texture_t tex;
   drm_radeon_tex_image_t tmp;
   int ret;

   tex.offset = offset;
   tex.pitch = pitch;
   tex.format = format;
   tex.width = width;
   tex.height = height;
   tex.image = &tmp;

   /* This gets updated by the kernel when a multipass blit is needed.
    */
   memcpy( &tmp, image, sizeof(drm_radeon_tex_image_t) );

   do {
      ret = ioctl( fd, DRM_IOCTL_RADEON_TEXTURE, &tex );
   } while ( ret && errno == EAGAIN );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmRadeonPolygonStipple( int fd, unsigned int *mask )
{
   drm_radeon_stipple_t stipple;

   stipple.mask = mask;

   if ( ioctl( fd, DRM_IOCTL_RADEON_STIPPLE, &stipple ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmRadeonFlushIndirectBuffer( int fd, int index,
				  int start, int end, int discard )
{
   drm_radeon_indirect_t ind;

   ind.idx = index;
   ind.start = start;
   ind.end = end;
   ind.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_RADEON_INDIRECT, &ind ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}
