/* xf86drmMga.c -- User-level interface to MGA DRM device
 * Created: Sun Apr  9 18:13:54 2000 by gareth@valinux.com
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
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/drm/xf86drmMga.c,v 1.6 2001/08/27 17:40:59 dawes Exp $ */

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
#  include <X11/Xlibint.h>
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
#include "xf86drmMga.h"
#include "drm.h"

#define MGA_IDLE_RETRY		2048


int drmMGAInitDMA( int fd, drmMGAInit *info )
{
   drm_mga_init_t init;

   memset( &init, 0, sizeof(drm_mga_init_t) );

   init.func			= MGA_INIT_DMA;

   init.sarea_priv_offset	= info->sarea_priv_offset;
   init.sgram			= info->sgram;
   init.chipset			= info->chipset;
   init.maccess			= info->maccess;

   init.fb_cpp			= info->fb_cpp;
   init.front_offset		= info->front_offset;
   init.front_pitch		= info->front_pitch;
   init.back_offset		= info->back_offset;
   init.back_pitch		= info->back_pitch;

   init.depth_cpp		= info->depth_cpp;
   init.depth_offset		= info->depth_offset;
   init.depth_pitch		= info->depth_pitch;

   init.texture_offset[0]	= info->texture_offset[0];
   init.texture_size[0]		= info->texture_size[0];
   init.texture_offset[1]	= info->texture_offset[1];
   init.texture_size[1]		= info->texture_size[1];

   init.fb_offset		= info->fb_offset;
   init.mmio_offset		= info->mmio_offset;
   init.status_offset		= info->status_offset;
   init.warp_offset		= info->warp_offset;
   init.primary_offset		= info->primary_offset;
   init.buffers_offset		= info->buffers_offset;

   if ( ioctl( fd, DRM_IOCTL_MGA_INIT, &init ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmMGACleanupDMA( int fd )
{
   drm_mga_init_t init;

   memset( &init, 0, sizeof(drm_mga_init_t) );

   init.func = MGA_CLEANUP_DMA;

   if ( ioctl( fd, DRM_IOCTL_MGA_INIT, &init ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmMGAFlushDMA( int fd, drmLockFlags flags )
{
   drm_lock_t lock;
   int ret, i = 0;

   memset( &lock, 0, sizeof(drm_lock_t) );

   if ( flags & DRM_LOCK_QUIESCENT )	lock.flags |= _DRM_LOCK_QUIESCENT;
   if ( flags & DRM_LOCK_FLUSH )	lock.flags |= _DRM_LOCK_FLUSH;
   if ( flags & DRM_LOCK_FLUSH_ALL )	lock.flags |= _DRM_LOCK_FLUSH_ALL;

   do {
      ret = ioctl( fd, DRM_IOCTL_MGA_FLUSH, &lock );
   } while ( ret && errno == EBUSY && i++ < MGA_IDLE_RETRY );

   if ( ret == 0 )
      return 0;
   if ( errno != EBUSY )
      return -errno;

   if ( lock.flags & _DRM_LOCK_QUIESCENT ) {
      /* Only keep trying if we need quiescence.
       */
      lock.flags &= ~(_DRM_LOCK_FLUSH | _DRM_LOCK_FLUSH_ALL);

      do {
	 ret = ioctl( fd, DRM_IOCTL_MGA_FLUSH, &lock );
      } while ( ret && errno == EBUSY && i++ < MGA_IDLE_RETRY );
   }

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmMGAEngineReset( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_MGA_RESET, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmMGAFullScreen( int fd, int enable )
{
   return -EINVAL;
}

int drmMGASwapBuffers( int fd )
{
   int ret, i = 0;

   do {
      ret = ioctl( fd, DRM_IOCTL_MGA_SWAP, NULL );
   } while ( ret && errno == EBUSY && i++ < MGA_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmMGAClear( int fd, unsigned int flags,
		 unsigned int clear_color, unsigned int clear_depth,
		 unsigned int color_mask, unsigned int depth_mask )
{
   drm_mga_clear_t clear;
   int ret, i = 0;

   clear.flags = flags;
   clear.clear_color = clear_color;
   clear.clear_depth = clear_depth;
   clear.color_mask = color_mask;
   clear.depth_mask = depth_mask;

   do {
      ret = ioctl( fd, DRM_IOCTL_MGA_CLEAR, &clear );
   } while ( ret && errno == EBUSY && i++ < MGA_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmMGAFlushVertexBuffer( int fd, int index, int used, int discard )
{
   drm_mga_vertex_t vertex;

   vertex.idx = index;
   vertex.used = used;
   vertex.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_MGA_VERTEX, &vertex ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmMGAFlushIndices( int fd, int index, int start, int end, int discard )
{
   drm_mga_indices_t indices;

   indices.idx = index;
   indices.start = start;
   indices.end = end;
   indices.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_MGA_INDICES, &indices ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmMGATextureLoad( int fd, int index,
		       unsigned int dstorg, unsigned int length )
{
   drm_mga_iload_t iload;
   int ret, i = 0;

   iload.idx = index;
   iload.dstorg = dstorg;
   iload.length = length;

   do {
      ret = ioctl( fd, DRM_IOCTL_MGA_ILOAD, &iload );
   } while ( ret && errno == EBUSY && i++ < MGA_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmMGAAgpBlit( int fd, unsigned int planemask,
		   unsigned int src_offset, int src_pitch,
		   unsigned int dst_offset, int dst_pitch,
		   int delta_sx, int delta_sy,
		   int delta_dx, int delta_dy,
		   int height, int ydir )
{
   drm_mga_blit_t blit;
   int ret, i = 0;

   blit.planemask = planemask;
   blit.srcorg = src_offset;
   blit.dstorg = dst_offset;
   blit.src_pitch = src_pitch;
   blit.dst_pitch = dst_pitch;
   blit.delta_sx = delta_sx;
   blit.delta_sy = delta_sy;
   blit.delta_dx = delta_dx;
   blit.delta_dx = delta_dx;
   blit.height = height;
   blit.ydir = ydir;

   do {
      ret = ioctl( fd, DRM_IOCTL_MGA_BLIT, &blit );
   } while ( ret && errno == EBUSY && i++ < MGA_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}
