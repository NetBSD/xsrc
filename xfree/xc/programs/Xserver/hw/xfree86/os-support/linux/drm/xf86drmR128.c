/* xf86drmR128.c -- User-level interface to Rage 128 DRM device
 * Created: Sun Apr  9 18:13:54 2000 by kevin@precisioninsight.com
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
 * Author: Kevin E. Martin <martin@valinux.com>
 *
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/linux/drm/xf86drmR128.c,v 1.11 2001/08/27 17:40:59 dawes Exp $ */

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
#include "xf86drmR128.h"
#include "drm.h"

#define R128_BUFFER_RETRY	32
#define R128_IDLE_RETRY		32


int drmR128InitCCE( int fd, drmR128Init *info )
{
   drm_r128_init_t init;

   memset( &init, 0, sizeof(drm_r128_init_t) );

   init.func			= R128_INIT_CCE;
   init.sarea_priv_offset	= info->sarea_priv_offset;
   init.is_pci			= info->is_pci;
   init.cce_mode		= info->cce_mode;
   init.cce_secure		= info->cce_secure;
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
   init.span_offset		= info->span_offset;

   init.fb_offset		= info->fb_offset;
   init.mmio_offset		= info->mmio_offset;
   init.ring_offset		= info->ring_offset;
   init.ring_rptr_offset	= info->ring_rptr_offset;
   init.buffers_offset		= info->buffers_offset;
   init.agp_textures_offset	= info->agp_textures_offset;

   if ( ioctl( fd, DRM_IOCTL_R128_INIT, &init ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128CleanupCCE( int fd )
{
   drm_r128_init_t init;

   memset( &init, 0, sizeof(drm_r128_init_t) );

   init.func = R128_CLEANUP_CCE;

   if ( ioctl( fd, DRM_IOCTL_R128_INIT, &init ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128StartCCE( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_R128_CCE_START, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128StopCCE( int fd )
{
   drm_r128_cce_stop_t stop;
   int ret, i = 0;

   stop.flush = 1;
   stop.idle = 1;

   ret = ioctl( fd, DRM_IOCTL_R128_CCE_STOP, &stop );

   if ( ret == 0 ) {
      return 0;
   } else if ( errno != EBUSY ) {
      return -errno;
   }

   stop.flush = 0;

   do {
      ret = ioctl( fd, DRM_IOCTL_R128_CCE_STOP, &stop );
   } while ( ret && errno == EBUSY && i++ < R128_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else if ( errno != EBUSY ) {
      return -errno;
   }

   stop.idle = 0;

   if ( ioctl( fd, DRM_IOCTL_R128_CCE_STOP, &stop ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128ResetCCE( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_R128_CCE_RESET, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128WaitForIdleCCE( int fd )
{
   int ret, i = 0;

   do {
      ret = ioctl( fd, DRM_IOCTL_R128_CCE_IDLE, NULL );
   } while ( ret && errno == EBUSY && i++ < R128_IDLE_RETRY );

   if ( ret == 0 ) {
      return 0;
   } else {
      return -errno;
   }
}

int drmR128EngineReset( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_R128_RESET, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128FullScreen( int fd, int enable )
{
   drm_r128_fullscreen_t fs;

   if ( enable ) {
      fs.func = R128_INIT_FULLSCREEN;
   } else {
      fs.func = R128_CLEANUP_FULLSCREEN;
   }

   if ( ioctl( fd, DRM_IOCTL_R128_FULLSCREEN, &fs ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128SwapBuffers( int fd )
{
   if ( ioctl( fd, DRM_IOCTL_R128_SWAP, NULL ) ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128Clear( int fd, unsigned int flags,
		  unsigned int clear_color, unsigned int clear_depth,
		  unsigned int color_mask, unsigned int depth_mask )
{
   drm_r128_clear_t clear;

   clear.flags = flags;
   clear.clear_color = clear_color;
   clear.clear_depth = clear_depth;
   clear.color_mask = color_mask;
   clear.depth_mask = depth_mask;

   if ( ioctl( fd, DRM_IOCTL_R128_CLEAR, &clear ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128FlushVertexBuffer( int fd, int prim, int index,
			      int count, int discard )
{
   drm_r128_vertex_t v;

   v.prim = prim;
   v.idx = index;
   v.count = count;
   v.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_R128_VERTEX, &v ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128FlushIndices( int fd, int prim, int index,
			 int start, int end, int discard )
{
   drm_r128_indices_t elts;

   elts.prim = prim;
   elts.idx = index;
   elts.start = start;
   elts.end = end;
   elts.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_R128_INDICES, &elts ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128TextureBlit( int fd, int index,
			int offset, int pitch, int format,
			int x, int y, int width, int height )
{
   drm_r128_blit_t blit;

   blit.idx = index;
   blit.offset = offset;
   blit.pitch = pitch;
   blit.format = format;
   blit.x = x;
   blit.y = y;
   blit.width = width;
   blit.height = height;

   if ( ioctl( fd, DRM_IOCTL_R128_BLIT, &blit ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128WriteDepthSpan( int fd, int n, int x, int y,
			   const unsigned int depth[],
			   const unsigned char mask[] )
{
   drm_r128_depth_t d;

   d.func = R128_WRITE_SPAN;
   d.n = n;
   d.x = &x;
   d.y = &y;
   d.buffer = (unsigned int *)depth;
   d.mask = (unsigned char *)mask;

   if ( ioctl( fd, DRM_IOCTL_R128_DEPTH, &d ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128WriteDepthPixels( int fd, int n,
			     const int x[], const int y[],
			     const unsigned int depth[],
			     const unsigned char mask[] )
{
   drm_r128_depth_t d;

   d.func = R128_WRITE_PIXELS;
   d.n = n;
   d.x = (int *)x;
   d.y = (int *)y;
   d.buffer = (unsigned int *)depth;
   d.mask = (unsigned char *)mask;

   if ( ioctl( fd, DRM_IOCTL_R128_DEPTH, &d ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128ReadDepthSpan( int fd, int n, int x, int y )
{
   drm_r128_depth_t d;

   d.func = R128_READ_SPAN;
   d.n = n;
   d.x = &x;
   d.y = &y;
   d.buffer = NULL;
   d.mask = NULL;

   if ( ioctl( fd, DRM_IOCTL_R128_DEPTH, &d ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128ReadDepthPixels( int fd, int n,
			    const int x[], const int y[] )
{
   drm_r128_depth_t d;

   d.func = R128_READ_PIXELS;
   d.n = n;
   d.x = (int *)x;
   d.y = (int *)y;
   d.buffer = NULL;
   d.mask = NULL;

   if ( ioctl( fd, DRM_IOCTL_R128_DEPTH, &d ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128PolygonStipple( int fd, unsigned int *mask )
{
   drm_r128_stipple_t stipple;

   stipple.mask = mask;

   if ( ioctl( fd, DRM_IOCTL_R128_STIPPLE, &stipple ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}

int drmR128FlushIndirectBuffer( int fd, int index,
				int start, int end, int discard )
{
   drm_r128_indirect_t ind;

   ind.idx = index;
   ind.start = start;
   ind.end = end;
   ind.discard = discard;

   if ( ioctl( fd, DRM_IOCTL_R128_INDIRECT, &ind ) < 0 ) {
      return -errno;
   } else {
      return 0;
   }
}
