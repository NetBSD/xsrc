/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_ioctl.c,v 1.6 2001/04/10 16:07:52 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "r128_context.h"
#include "r128_state.h"
#include "r128_ioctl.h"

#include "mem.h"

#define R128_TIMEOUT        2048


/* =============================================================
 * Hardware vertex buffer handling
 */

/* Get a new VB from the pool of vertex buffers in AGP space.
 */
drmBufPtr r128GetBufferLocked( r128ContextPtr rmesa )
{
   int fd = rmesa->r128Screen->driScreen->fd;
   int index = 0;
   int size = 0;
   drmDMAReq dma;
   drmBufPtr buf = NULL;
   int to = 0;
   int ret;

   dma.context = rmesa->hHWContext;
   dma.send_count = 0;
   dma.send_list = NULL;
   dma.send_sizes = NULL;
   dma.flags = 0;
   dma.request_count = 1;
   dma.request_size = R128_BUFFER_SIZE;
   dma.request_list = &index;
   dma.request_sizes = &size;
   dma.granted_count = 0;

   while ( !buf && ( to++ < R128_TIMEOUT ) ) {
      ret = drmDMA( fd, &dma );

      if ( ret == 0 ) {
	 buf = &rmesa->r128Screen->buffers->list[index];
	 buf->used = 0;
#if ENABLE_PERF_BOXES
	 /* Bump the performance counter */
	 rmesa->c_vertexBuffers++;
#endif
	 return buf;
      }
   }

   if ( !buf ) {
      drmR128EngineReset( fd );
      UNLOCK_HARDWARE( rmesa );
      fprintf( stderr, "Error: Could not get new VB... exiting\n" );
      exit( -1 );
   }

   return buf;
}

void r128FlushVerticesLocked( r128ContextPtr rmesa )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   drmBufPtr buffer = rmesa->vert_buf;
   int count = rmesa->num_verts;
   int prim = R128_TRIANGLES;
   int fd = rmesa->driScreen->fd;
   int i;

   rmesa->num_verts = 0;
   rmesa->vert_buf = NULL;

   if ( !buffer )
      return;

   if ( rmesa->dirty & ~R128_UPLOAD_CLIPRECTS )
      r128EmitHwStateLocked( rmesa );

   if ( !nbox )
      count = 0;

   if ( nbox >= R128_NR_SAREA_CLIPRECTS )
      rmesa->dirty |= R128_UPLOAD_CLIPRECTS;

   if ( !count || !(rmesa->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmR128FlushVertexBuffer( fd, prim, buffer->idx, count, 1 );
   }
   else
   {
      for ( i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;
	 int discard = 0;

	 rmesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++ ) {
	    *b++ = pbox[i];
	 }

	 /* Finished with the buffer?
	  */
	 if ( nr == nbox ) {
	    discard = 1;
	 }

	 rmesa->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128FlushVertexBuffer( fd, prim, buffer->idx, count, discard );
      }
   }

   rmesa->dirty &= ~R128_UPLOAD_CLIPRECTS;
}



/* ================================================================
 * Indexed vertex buffer handling
 */

void r128GetEltBufLocked( r128ContextPtr rmesa )
{
   rmesa->elt_buf = r128GetBufferLocked( rmesa );
}

void r128FireEltsLocked( r128ContextPtr rmesa,
			 GLuint start, GLuint end,
			 GLuint discard )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   drmBufPtr buffer = rmesa->elt_buf;
   int prim = R128_TRIANGLES;
   int fd = rmesa->driScreen->fd;
   int i;

   if ( !buffer )
      return;

   if ( rmesa->dirty & ~R128_UPLOAD_CLIPRECTS )
      r128EmitHwStateLocked( rmesa );

   if ( !nbox )
      end = start;

   if ( nbox >= R128_NR_SAREA_CLIPRECTS )
      rmesa->dirty |= R128_UPLOAD_CLIPRECTS;

   if ( start == end || !(rmesa->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmR128FlushIndices( fd, prim, buffer->idx, start, end, discard );
   }
   else
   {
      for ( i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;
	 int d = 0;

	 rmesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++ ) {
	    *b++ = pbox[i];
	 }

	 /* Finished with the buffer?
	  */
	 if ( nr == nbox ) {
	    d = discard;
	 }

	 rmesa->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128FlushIndices( fd, prim, buffer->idx, start, end, d );
      }
   }

   if ( R128_DEBUG & DEBUG_ALWAYS_SYNC ) {
      drmR128WaitForIdleCCE( rmesa->driFd );
   }

   rmesa->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128FlushEltsLocked( r128ContextPtr rmesa )
{
   if ( rmesa->first_elt != rmesa->next_elt ) {
      r128FireEltsLocked( rmesa,
			  ((char *)rmesa->first_elt -
			   (char *)rmesa->elt_buf->address),
			  ((char *)rmesa->next_elt -
			   (char *)rmesa->elt_buf->address),
			  0 );

      ALIGN_NEXT_ELT( rmesa );
      rmesa->first_elt = rmesa->next_elt;
   }
}

void r128ReleaseBufLocked( r128ContextPtr rmesa, drmBufPtr buffer )
{
   int fd = rmesa->driScreen->fd;

   if ( !buffer )
      return;

   drmR128FlushVertexBuffer( fd, R128_TRIANGLES, buffer->idx, 0, 1 );
}


/* Allocate some space in the current vertex buffer.  If the current
 * buffer is full, flush it and grab another one.
 */
CARD32 *r128AllocVertices( r128ContextPtr rmesa, int count )
{
   return r128AllocVerticesInline( rmesa, count );
}


/* ================================================================
 * Texture uploads
 */

void r128FireBlitLocked( r128ContextPtr rmesa, drmBufPtr buffer,
			 GLint offset, GLint pitch, GLint format,
			 GLint x, GLint y, GLint width, GLint height )
{
   GLint ret;

   ret = drmR128TextureBlit( rmesa->driFd, buffer->idx,
			     offset, pitch, format,
			     x, y, width, height );

   if ( ret ) {
      UNLOCK_HARDWARE( rmesa );
      fprintf( stderr, "drmR128TextureBlit: return = %d\n", ret );
      exit( 1 );
   }
}


/* ================================================================
 * SwapBuffers with client-side throttling
 */

static void delay( void ) {
/* Prevent an optimizing compiler from removing a spin loop */
}

#define R128_MAX_OUTSTANDING	2

/* Throttle the frame rate -- only allow one pending swap buffers
 * request at a time.
 * GH: We probably don't want a timeout here, as we can wait as
 * long as we want for a frame to complete.  If it never does, then
 * the card has locked.
 */
static int r128WaitForFrameCompletion( r128ContextPtr rmesa )
{
   unsigned char *R128MMIO = rmesa->r128Screen->mmio.map;
   CARD32 frame;
   int i;
   int wait = 0;

   while ( 1 ) {
#if defined(__alpha__)
       /* necessary to preserve the Alpha paradigm */
       /* NOTE: this will not work on SPARSE machines */
       mem_barrier();
       frame = *(volatile CARD32 *)(void *)(R128MMIO + R128_LAST_FRAME_REG);
#else
      frame = INREG( R128_LAST_FRAME_REG );
#endif

      if ( 0 )
	 fprintf( stderr, " last=0x%08x frame=0x%08x\n",
		  rmesa->sarea->last_frame, frame );

      if ( rmesa->sarea->last_frame - frame <= R128_MAX_OUTSTANDING ) {
	 break;
      }

      /* Spin in place a bit so we aren't hammering the register */
      wait++;
      for ( i = 0 ; i < 1024 ; i++ ) {
	 delay();
      }
   }

   return wait;
}

/* Copy the back color buffer to the front color buffer.
 */
void r128SwapBuffers( r128ContextPtr rmesa )
{
   GLint nbox;
   GLint i;
   GLint ret;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "\n********************************\n" );
      fprintf( stderr, "\n%s( %p )\n\n",
	       __FUNCTION__, rmesa->glCtx );
      fflush( stderr );
   }

   FLUSH_BATCH( rmesa );

   LOCK_HARDWARE( rmesa );

   nbox = rmesa->numClipRects;	/* must be in locked region */

   /* Throttle the frame rate -- only allow one pending swap buffers
    * request at a time.
    */
   if ( !r128WaitForFrameCompletion( rmesa ) ) {
      rmesa->hardwareWentIdle = 1;
   } else {
      rmesa->hardwareWentIdle = 0;
   }

   for ( i = 0 ; i < nbox ; ) {
      GLint nr = MIN2( i + R128_NR_SAREA_CLIPRECTS , nbox );
      XF86DRIClipRectPtr box = rmesa->pClipRects;
      XF86DRIClipRectPtr b = rmesa->sarea->boxes;
      GLint n = 0;

      for ( ; i < nr ; i++ ) {
	 *b++ = *(XF86DRIClipRectRec *)&box[i];
	 n++;
      }
      rmesa->sarea->nbox = n;

      ret = drmR128SwapBuffers( rmesa->driFd );

      if ( ret ) {
	 UNLOCK_HARDWARE( rmesa );
	 fprintf( stderr, "drmR128SwapBuffers: return = %d\n", ret );
	 exit( 1 );
      }
   }

   if ( R128_DEBUG & DEBUG_ALWAYS_SYNC ) {
      drmR128WaitForIdleCCE( rmesa->driFd );
   }

   UNLOCK_HARDWARE( rmesa );

   rmesa->new_state |= R128_NEW_CONTEXT;
   rmesa->dirty |= (R128_UPLOAD_CONTEXT |
		    R128_UPLOAD_MASKS |
		    R128_UPLOAD_CLIPRECTS);

#if ENABLE_PERF_BOXES
   /* Log the performance counters if necessary */
   r128PerformanceCounters( rmesa );
#endif
}

void r128PageFlip( r128ContextPtr rmesa )
{
   GLint ret;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "\n%s( %p ): page=%d\n\n",
	       __FUNCTION__, rmesa->glCtx, rmesa->currentPage );
   }

   FLUSH_BATCH( rmesa );

   LOCK_HARDWARE( rmesa );

   /* Throttle the frame rate -- only allow one pending swap buffers
    * request at a time.
    */
   if ( !r128WaitForFrameCompletion( rmesa ) ) {
      rmesa->hardwareWentIdle = 1;
   } else {
      rmesa->hardwareWentIdle = 0;
   }

   /* The kernel will have been initialized to perform page flipping
    * on a swapbuffers ioctl.
    */
   ret = drmR128SwapBuffers( rmesa->driFd );

   UNLOCK_HARDWARE( rmesa );

   if ( ret ) {
      fprintf( stderr, "drmR128SwapBuffers: return = %d\n", ret );
      exit( 1 );
   }

   if ( rmesa->currentPage == 0 ) {
	 rmesa->drawOffset = rmesa->r128Screen->frontOffset;
	 rmesa->drawPitch  = rmesa->r128Screen->frontPitch;
	 rmesa->currentPage = 1;
   } else {
	 rmesa->drawOffset = rmesa->r128Screen->backOffset;
	 rmesa->drawPitch  = rmesa->r128Screen->backPitch;
	 rmesa->currentPage = 0;
   }

   rmesa->setup.dst_pitch_offset_c = (((rmesa->drawPitch/8) << 21) |
				      (rmesa->drawOffset >> 5));
   rmesa->new_state |= R128_NEW_WINDOW;

   /* FIXME: Do we need this anymore? */
   rmesa->new_state |= R128_NEW_CONTEXT;
   rmesa->dirty |= (R128_UPLOAD_CONTEXT |
		    R128_UPLOAD_MASKS |
		    R128_UPLOAD_CLIPRECTS);

#if ENABLE_PERF_BOXES
   /* Log the performance counters if necessary */
   r128PerformanceCounters( rmesa );
#endif
}


/* ================================================================
 * Buffer clear
 */

static GLbitfield r128DDClear( GLcontext *ctx, GLbitfield mask, GLboolean all,
			       GLint cx, GLint cy, GLint cw, GLint ch )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   __DRIdrawablePrivate *dPriv = rmesa->driDrawable;
   GLuint flags = 0;
   GLuint color_mask = 0;
   GLuint depth_mask = 0;
   GLint i;
   GLint ret;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s:\n", __FUNCTION__ );
   }

   FLUSH_BATCH( rmesa );

   /* Update and emit any new state.  We need to do this here to catch
    * changes to the masks.
    * FIXME: Just update the masks?
    */
   if ( rmesa->new_state )
      r128DDUpdateHWState( ctx );

   if ( mask & DD_FRONT_LEFT_BIT ) {
      flags |= DRM_R128_FRONT;
      color_mask = rmesa->setup.plane_3d_mask_c;
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if ( mask & DD_BACK_LEFT_BIT ) {
      flags |= DRM_R128_BACK;
      color_mask = rmesa->setup.plane_3d_mask_c;
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if ( ( mask & DD_DEPTH_BIT ) && ctx->Depth.Mask ) {
      flags |= DRM_R128_DEPTH;
      depth_mask |= rmesa->DepthMask;
      mask &= ~DD_DEPTH_BIT;
   }
#if 0
   /* FIXME: Add stencil support */
   if ( mask & DD_STENCIL_BIT ) {
      flags |= DRM_R128_DEPTH;
      depth_mask |= rmesa->StencilMask;
      mask &= ~DD_STENCIL_BIT;
   }
#endif

   if ( !flags )
      return mask;

   /* Flip top to bottom */
   cx += dPriv->x;
   cy  = dPriv->y + dPriv->h - cy - ch;

   LOCK_HARDWARE( rmesa );

   for ( i = 0 ; i < rmesa->numClipRects ; ) {
      GLint nr = MIN2( i + R128_NR_SAREA_CLIPRECTS , rmesa->numClipRects );
      XF86DRIClipRectPtr box = rmesa->pClipRects;
      XF86DRIClipRectPtr b = rmesa->sarea->boxes;
      GLint n = 0;

      if ( !all ) {
	 for ( ; i < nr ; i++ ) {
	    GLint x = box[i].x1;
	    GLint y = box[i].y1;
	    GLint w = box[i].x2 - x;
	    GLint h = box[i].y2 - y;

	    if ( x < cx ) w -= cx - x, x = cx;
	    if ( y < cy ) h -= cy - y, y = cy;
	    if ( x + w > cx + cw ) w = cx + cw - x;
	    if ( y + h > cy + ch ) h = cy + ch - y;
	    if ( w <= 0 ) continue;
	    if ( h <= 0 ) continue;

	    b->x1 = x;
	    b->y1 = y;
	    b->x2 = x + w;
	    b->y2 = y + h;
	    b++;
	    n++;
	 }
      } else {
	 for ( ; i < nr ; i++ ) {
	    *b++ = *(XF86DRIClipRectRec *)&box[i];
	    n++;
	 }
      }

      rmesa->sarea->nbox = n;

      if ( R128_DEBUG & DEBUG_VERBOSE_IOCTL ) {
	 fprintf( stderr,
		  "drmR128Clear: flag 0x%x color %x depth %x nbox %d\n",
		  flags,
		  (GLuint)rmesa->ClearColor,
		  (GLuint)rmesa->ClearDepth,
		  rmesa->sarea->nbox );
      }

      ret = drmR128Clear( rmesa->driFd, flags,
			  rmesa->ClearColor, rmesa->ClearDepth,
			  color_mask, depth_mask );

      if ( ret ) {
	 UNLOCK_HARDWARE( rmesa );
	 fprintf( stderr, "drmR128Clear: return = %d\n", ret );
	 exit( 1 );
      }
   }

   UNLOCK_HARDWARE( rmesa );

   rmesa->dirty |= R128_UPLOAD_CLIPRECTS;

   return mask;
}


/* ================================================================
 * Depth spans, pixels
 */

void r128WriteDepthSpanLocked( r128ContextPtr rmesa,
			       GLuint n, GLint x, GLint y,
			       const GLdepth depth[],
			       const GLubyte mask[] )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   int fd = rmesa->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      rmesa->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(rmesa->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmR128WriteDepthSpan( fd, n, x, y, depth, mask );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;

	 rmesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 rmesa->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128WriteDepthSpan( fd, n, x, y, depth, mask );
      }
   }

   rmesa->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128WriteDepthPixelsLocked( r128ContextPtr rmesa, GLuint n,
				 const GLint x[], const GLint y[],
				 const GLdepth depth[],
				 const GLubyte mask[] )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   int fd = rmesa->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      rmesa->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(rmesa->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmR128WriteDepthPixels( fd, n, x, y, depth, mask );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;

	 rmesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 rmesa->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128WriteDepthPixels( fd, n, x, y, depth, mask );
      }
   }

   rmesa->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128ReadDepthSpanLocked( r128ContextPtr rmesa,
			      GLuint n, GLint x, GLint y )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   int fd = rmesa->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      rmesa->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(rmesa->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmR128ReadDepthSpan( fd, n, x, y );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;

	 rmesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 rmesa->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128ReadDepthSpan( fd, n, x, y );
      }
   }

   rmesa->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128ReadDepthPixelsLocked( r128ContextPtr rmesa, GLuint n,
				const GLint x[], const GLint y[] )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   int fd = rmesa->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      rmesa->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(rmesa->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmR128ReadDepthPixels( fd, n, x, y );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;

	 rmesa->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 rmesa->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128ReadDepthPixels( fd, n, x, y );
      }
   }

   rmesa->dirty &= ~R128_UPLOAD_CLIPRECTS;
}


void r128WaitForIdleLocked( r128ContextPtr rmesa )
{
    int fd = rmesa->r128Screen->driScreen->fd;
    int to = 0;
    int ret;

    do {
	ret = drmR128WaitForIdleCCE( fd );
    } while ( ( ret == -EBUSY ) && ( to++ < R128_TIMEOUT ) );

    if ( ret < 0 ) {
	drmR128EngineReset( fd );
	UNLOCK_HARDWARE( rmesa );
	fprintf( stderr, "Error: Rage 128 timed out... exiting\n" );
	exit( -1 );
    }
}


void r128DDInitIoctlFuncs( GLcontext *ctx )
{
    ctx->Driver.Clear = r128DDClear;
}
