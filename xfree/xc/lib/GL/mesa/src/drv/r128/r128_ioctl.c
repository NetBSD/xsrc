/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_ioctl.c,v 1.2 2000/12/12 17:17:07 dawes Exp $ */
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

#define R128_TIMEOUT        2000000


/* =============================================================
 * Hardware vertex buffer handling
 */

/* Get a new VB from the pool of vertex buffers in AGP space.
 */
drmBufPtr r128GetBufferLocked( r128ContextPtr r128ctx )
{
   int fd = r128ctx->r128Screen->driScreen->fd;
   int index = 0;
   int size = 0;
   drmDMAReq dma;
   drmBufPtr buf = NULL;
   int to = 0;
   int ret;

   dma.context = r128ctx->hHWContext;
   dma.send_count = 0;
   dma.send_list = NULL;
   dma.send_sizes = NULL;
   dma.flags = 0;
   dma.request_count = 1;
   dma.request_size = R128_BUFFER_SIZE;
   dma.request_list = &index;
   dma.request_sizes = &size;
   dma.granted_count = 0;

   while ( !buf && ( to++ < r128ctx->CCEtimeout ) ) {
      ret = drmDMA( fd, &dma );

      if ( ret == 0 ) {
	 buf = &r128ctx->r128Screen->buffers->list[index];
	 buf->used = 0;
#if ENABLE_PERF_BOXES
	 /* Bump the performance counter */
	 r128ctx->c_vertexBuffers++;
#endif
	 return buf;
      }
   }

   if ( !buf ) {
      drmR128EngineReset( fd );
      fprintf( stderr, "Error: Could not get new VB... exiting\n" );
      exit( -1 );
   }

   return buf;
}

void r128FlushVerticesLocked( r128ContextPtr r128ctx )
{
   XF86DRIClipRectPtr pbox = r128ctx->pClipRects;
   int nbox = r128ctx->numClipRects;
   drmBufPtr buffer = r128ctx->vert_buf;
   int count = r128ctx->num_verts;
   int prim = R128_TRIANGLES;
   int fd = r128ctx->driScreen->fd;
   int i;

   if ( 0 )
	fprintf( stderr, "%s: buf=%d count=%d\n",
		 __FUNCTION__, buffer ? buffer->idx : -1, count );

   r128ctx->num_verts = 0;
   r128ctx->vert_buf = NULL;

   if ( !buffer ) {
      return;
   }

   if ( r128ctx->dirty & ~R128_UPLOAD_CLIPRECTS ) {
      r128EmitHwStateLocked( r128ctx );
   }

   if ( !nbox ) {
      count = 0;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !count || !(r128ctx->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 r128ctx->sarea->nbox = 0;
      } else {
	 r128ctx->sarea->nbox = nbox;
      }

      drmR128FlushVertexBuffer( fd, prim, buffer->idx, count, 1 );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = r128ctx->sarea->boxes;
	 int discard = 0;

	 r128ctx->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 /* Finished with the buffer?
	  */
	 if ( nr == nbox ) {
	    discard = 1;
	 }

	 r128ctx->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128FlushVertexBuffer( fd, prim, buffer->idx, count, discard );
      }
   }

   r128ctx->dirty &= ~R128_UPLOAD_CLIPRECTS;
}



/* ================================================================
 * Indexed vertex buffer handling
 */

void r128GetEltBufLocked( r128ContextPtr r128ctx )
{
   r128ctx->elt_buf = r128GetBufferLocked( r128ctx );
}

void r128FireEltsLocked( r128ContextPtr r128ctx,
			 GLuint start, GLuint end,
			 GLuint discard )
{
   XF86DRIClipRectPtr pbox = r128ctx->pClipRects;
   int nbox = r128ctx->numClipRects;
   drmBufPtr buffer = r128ctx->elt_buf;
   int prim = R128_TRIANGLES;
   int fd = r128ctx->driScreen->fd;
   int i;

   if ( 0 )
	fprintf( stderr, "%s: start=%d end=%d discard=%d\n",
		 __FUNCTION__, start, end, discard );

   if ( !buffer ) {
      return;
   }

   if ( r128ctx->dirty & ~R128_UPLOAD_CLIPRECTS ) {
      r128EmitHwStateLocked( r128ctx );
   }

   if ( !nbox ) {
      end = start;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( start == end || !(r128ctx->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 r128ctx->sarea->nbox = 0;
      } else {
	 r128ctx->sarea->nbox = nbox;
      }

      drmR128FlushIndices( fd, prim, buffer->idx, start, end, discard );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = r128ctx->sarea->boxes;
	 int d = 0;

	 r128ctx->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 /* Finished with the buffer?
	  */
	 if ( nr == nbox ) {
	    d = discard;
	 }

	 r128ctx->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128FlushIndices( fd, prim, buffer->idx, start, end, discard );
      }
   }

   if ( R128_DEBUG & DEBUG_ALWAYS_SYNC ) {
      drmR128WaitForIdleCCE( r128ctx->driFd );
   }

   r128ctx->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128FlushEltsLocked( r128ContextPtr r128ctx )
{
   if ( r128ctx->first_elt != r128ctx->next_elt ) {
      r128FireEltsLocked( r128ctx,
			  ((GLuint)r128ctx->first_elt -
			   (GLuint)r128ctx->elt_buf->address),
			  ((GLuint)r128ctx->next_elt -
			   (GLuint)r128ctx->elt_buf->address),
			  0 );

      ALIGN_NEXT_ELT( r128ctx );
      r128ctx->first_elt = r128ctx->next_elt;
   }
}

void r128ReleaseBufLocked( r128ContextPtr r128ctx, drmBufPtr buffer )
{
   int fd = r128ctx->driScreen->fd;

   if ( 0 )
	fprintf( stderr, "%s: buffer=%p\n",
		 __FUNCTION__, buffer );

   if ( !buffer ) {
      return;
   }
   drmR128FlushVertexBuffer( fd, R128_TRIANGLES, buffer->idx, 0, 1 );
}


/* Allocate some space in the current vertex buffer.  If the current
 * buffer is full, flush it and grab another one.
 */
CARD32 *r128AllocVertices( r128ContextPtr r128ctx, int count )
{
   return r128AllocVerticesInline( r128ctx, count );
}



/* ================================================================
 * Texture uploads
 */

void r128FireBlitLocked( r128ContextPtr r128ctx, drmBufPtr buffer,
			 GLint offset, GLint pitch, GLint format,
			 GLint x, GLint y, GLint width, GLint height )
{
   GLint ret;

   ret = drmR128TextureBlit( r128ctx->driFd, buffer->idx,
			     offset, pitch, format,
			     x, y, width, height );

   if ( ret ) {
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
static int r128WaitForFrameCompletion( r128ContextPtr r128ctx )
{
   unsigned char *R128MMIO = r128ctx->r128Screen->mmio;
   CARD32 frame;
   int i;
   int wait = 0;

   while ( 1 ) {
      frame = INREG( R128_LAST_FRAME_REG );
      if ( r128ctx->sarea->last_frame - frame <= R128_MAX_OUTSTANDING ) {
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

/* Copy the back color buffer to the front color buffer */
void r128SwapBuffers( r128ContextPtr r128ctx )
{
   GLint nbox = r128ctx->numClipRects;
   GLint i;
   GLint ret;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "\n********************************\n" );
      fprintf( stderr, "\n%s( %p )\n\n",
	       __FUNCTION__, r128ctx->glCtx );
      fflush( stderr );
   }

   /* Flush any outstanding vertex buffers */
   FLUSH_BATCH( r128ctx );

   LOCK_HARDWARE( r128ctx );

   /* Throttle the frame rate -- only allow one pending swap buffers
    * request at a time.
    */
   if ( !r128WaitForFrameCompletion( r128ctx ) ) {
      r128ctx->hardwareWentIdle = 1;
   } else {
      r128ctx->hardwareWentIdle = 0;
   }

   for ( i = 0 ; i < nbox ; ) {
      GLint nr = MIN2( i + R128_NR_SAREA_CLIPRECTS , nbox );
      XF86DRIClipRectPtr box = r128ctx->pClipRects;
      XF86DRIClipRectPtr b = r128ctx->sarea->boxes;
      GLint n = 0;

      for ( ; i < nr ; i++ ) {
	 *b++ = *(XF86DRIClipRectRec *)&box[i];
	 n++;
      }
      r128ctx->sarea->nbox = n;

      ret = drmR128SwapBuffers( r128ctx->driFd );

      if ( ret ) {
	 fprintf( stderr, "drmR128SwapBuffers: return = %d\n", ret );
	 exit( 1 );
      }
   }

   if ( R128_DEBUG & DEBUG_ALWAYS_SYNC ) {
      drmR128WaitForIdleCCE( r128ctx->driFd );
   }

   UNLOCK_HARDWARE( r128ctx );

   r128ctx->new_state |= R128_NEW_CONTEXT;
   r128ctx->dirty |= (R128_UPLOAD_CONTEXT |
		      R128_UPLOAD_MASKS |
		      R128_UPLOAD_CLIPRECTS);

#if ENABLE_PERF_BOXES
   /* Log the performance counters if necessary */
   r128PerformanceCounters( r128ctx );
#endif
}


/* ================================================================
 * Buffer clear
 */

static GLbitfield r128DDClear( GLcontext *ctx, GLbitfield mask, GLboolean all,
			       GLint cx, GLint cy, GLint cw, GLint ch )
{
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);
   __DRIdrawablePrivate *dPriv = r128ctx->driDrawable;
   GLuint flags = 0;
   GLint i;
   GLint ret;

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s:\n", __FUNCTION__ );
   }

   FLUSH_BATCH( r128ctx );

   /* Update and emit any new state.  We need to do this here to catch
    * changes to the masks.
    * FIXME: Just update the masks?
    */
   if ( r128ctx->new_state )
      r128DDUpdateHWState( ctx );

   if ( mask & DD_FRONT_LEFT_BIT ) {
      flags |= DRM_R128_FRONT;
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if ( mask & DD_BACK_LEFT_BIT ) {
      flags |= DRM_R128_BACK;
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if ( ( mask & DD_DEPTH_BIT ) && ctx->Depth.Mask ) {
      flags |= DRM_R128_DEPTH;
      mask &= ~DD_DEPTH_BIT;
   }
#if 0
   /* FIXME: Add stencil support */
   if ( mask & DD_STENCIL_BIT ) {
      flags |= DRM_R128_DEPTH;
      mask &= ~DD_STENCIL_BIT;
   }
#endif

   if ( !flags ) {
      return mask;
   }

   /* Flip top to bottom */
   cx += dPriv->x;
   cy  = dPriv->y + dPriv->h - cy - ch;

   LOCK_HARDWARE( r128ctx );

   if ( r128ctx->dirty & ~R128_UPLOAD_CLIPRECTS ) {
      r128EmitHwStateLocked( r128ctx );
   }

   for ( i = 0 ; i < r128ctx->numClipRects ; ) {
      GLint nr = MIN2( i + R128_NR_SAREA_CLIPRECTS , r128ctx->numClipRects );
      XF86DRIClipRectPtr box = r128ctx->pClipRects;
      XF86DRIClipRectPtr b = r128ctx->sarea->boxes;
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

      r128ctx->sarea->nbox = n;

      if ( R128_DEBUG & DEBUG_VERBOSE_IOCTL ) {
	 fprintf( stderr,
		  "drmR128Clear: flag 0x%x color %x depth %x nbox %d\n",
		  flags,
		  (GLuint)r128ctx->ClearColor,
		  (GLuint)r128ctx->ClearDepth,
		  r128ctx->sarea->nbox );
      }

      ret = drmR128Clear( r128ctx->driFd, flags,
			  cx, cy, cw, ch,
			  r128ctx->ClearColor, r128ctx->ClearDepth );

      if ( ret ) {
	 fprintf( stderr, "drmR128Clear: return = %d\n", ret );
	 exit( 1 );
      }
   }

   UNLOCK_HARDWARE( r128ctx );

   r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;

   return mask;
}


/* ================================================================
 * Depth spans, pixels
 */

void r128WriteDepthSpanLocked( r128ContextPtr r128ctx,
			       GLuint n, GLint x, GLint y,
			       const GLdepth depth[],
			       const GLubyte mask[] )
{
   XF86DRIClipRectPtr pbox = r128ctx->pClipRects;
   int nbox = r128ctx->numClipRects;
   int fd = r128ctx->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(r128ctx->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 r128ctx->sarea->nbox = 0;
      } else {
	 r128ctx->sarea->nbox = nbox;
      }

      drmR128WriteDepthSpan( fd, n, x, y, depth, mask );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = r128ctx->sarea->boxes;

	 r128ctx->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 r128ctx->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128WriteDepthSpan( fd, n, x, y, depth, mask );
      }
   }

   r128ctx->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128WriteDepthPixelsLocked( r128ContextPtr r128ctx, GLuint n,
				 const GLint x[], const GLint y[],
				 const GLdepth depth[],
				 const GLubyte mask[] )
{
   XF86DRIClipRectPtr pbox = r128ctx->pClipRects;
   int nbox = r128ctx->numClipRects;
   int fd = r128ctx->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(r128ctx->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 r128ctx->sarea->nbox = 0;
      } else {
	 r128ctx->sarea->nbox = nbox;
      }

      drmR128WriteDepthPixels( fd, n, x, y, depth, mask );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = r128ctx->sarea->boxes;

	 r128ctx->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 r128ctx->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128WriteDepthPixels( fd, n, x, y, depth, mask );
      }
   }

   r128ctx->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128ReadDepthSpanLocked( r128ContextPtr r128ctx,
			      GLuint n, GLint x, GLint y )
{
   XF86DRIClipRectPtr pbox = r128ctx->pClipRects;
   int nbox = r128ctx->numClipRects;
   int fd = r128ctx->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(r128ctx->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 r128ctx->sarea->nbox = 0;
      } else {
	 r128ctx->sarea->nbox = nbox;
      }

      drmR128ReadDepthSpan( fd, n, x, y );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = r128ctx->sarea->boxes;

	 r128ctx->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 r128ctx->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128ReadDepthSpan( fd, n, x, y );
      }
   }

   r128ctx->dirty &= ~R128_UPLOAD_CLIPRECTS;
}

void r128ReadDepthPixelsLocked( r128ContextPtr r128ctx, GLuint n,
				const GLint x[], const GLint y[] )
{
   XF86DRIClipRectPtr pbox = r128ctx->pClipRects;
   int nbox = r128ctx->numClipRects;
   int fd = r128ctx->driScreen->fd;
   int i;

   if ( !nbox || !n ) {
      return;
   }
   if ( nbox >= R128_NR_SAREA_CLIPRECTS ) {
      r128ctx->dirty |= R128_UPLOAD_CLIPRECTS;
   }

   if ( !(r128ctx->dirty & R128_UPLOAD_CLIPRECTS) )
   {
      if ( nbox < 3 ) {
	 r128ctx->sarea->nbox = 0;
      } else {
	 r128ctx->sarea->nbox = nbox;
      }

      drmR128ReadDepthPixels( fd, n, x, y );
   }
   else
   {
      for (i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + R128_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = r128ctx->sarea->boxes;

	 r128ctx->sarea->nbox = nr - i;
	 for ( ; i < nr ; i++) {
	    *b++ = pbox[i];
	 }

	 r128ctx->sarea->dirty |= R128_UPLOAD_CLIPRECTS;
	 drmR128ReadDepthPixels( fd, n, x, y );
      }
   }

   r128ctx->dirty &= ~R128_UPLOAD_CLIPRECTS;
}


/* ================================================================
 * Deprecated function...
 */
void r128SubmitPacketLocked( r128ContextPtr r128ctx,
			     CARD32 *buf, GLuint count )
{
   CARD32 *b;
   int c = count;
   int fd = r128ctx->r128Screen->driScreen->fd;
   int to = 0;
   int ret;

   do {
      b = buf + (count - c);
      ret = drmR128SubmitPacket( fd, b, &c, 0 );
   } while ( ( ret == -EBUSY ) && ( to++ < r128ctx->CCEtimeout ) );

   if ( ret < 0 ) {
      drmR128EngineReset( fd );
      fprintf( stderr, "Error: Could not submit packet... exiting\n" );
      exit( -1 );
   }
}



void r128WaitForIdleLocked( r128ContextPtr r128ctx )
{
    int fd = r128ctx->r128Screen->driScreen->fd;
    int to = 0;
    int ret;

    do {
	ret = drmR128WaitForIdleCCE( fd );
    } while ( ( ret == -EBUSY ) && ( to++ < r128ctx->CCEtimeout ) );

    if ( ret < 0 ) {
	drmR128EngineReset( fd );
	fprintf( stderr, "Error: Rage 128 timed out... exiting\n" );
	exit( -1 );
    }
}


void r128DDInitIoctlFuncs( GLcontext *ctx )
{
    ctx->Driver.Clear	= r128DDClear;
}
