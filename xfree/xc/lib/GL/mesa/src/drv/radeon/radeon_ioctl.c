/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_ioctl.c,v 1.4 2001/04/10 16:07:53 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

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
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "radeon_context.h"
#include "radeon_state.h"
#include "radeon_ioctl.h"

#include "mem.h"

#define RADEON_TIMEOUT             2048
#define USE_IN_MEMORY_SCRATCH_REGS 0


/* =============================================================
 * Hardware vertex buffer handling
 */

/* Get a new VB from the pool of vertex buffers in AGP space.
 */
drmBufPtr radeonGetBufferLocked( radeonContextPtr rmesa )
{
   int fd = rmesa->radeonScreen->driScreen->fd;
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
   dma.request_size = RADEON_BUFFER_SIZE;
   dma.request_list = &index;
   dma.request_sizes = &size;
   dma.granted_count = 0;

   while ( !buf && ( to++ < RADEON_TIMEOUT ) ) {
      ret = drmDMA( fd, &dma );

      if ( ret == 0 ) {
	 buf = &rmesa->radeonScreen->buffers->list[index];
	 buf->used = 0;
#if ENABLE_PERF_BOXES
	 /* Bump the performance counter */
	 rmesa->c_vertexBuffers++;
#endif
	 return buf;
      }
   }

   if ( !buf ) {
      drmRadeonEngineReset( fd );
      UNLOCK_HARDWARE( rmesa );
      fprintf( stderr, "Error: Could not get new VB... exiting\n" );
      exit( -1 );
   }

   return buf;
}

static GLboolean intersect_rect( XF86DRIClipRectPtr out,
				 XF86DRIClipRectPtr a,
				 XF86DRIClipRectPtr b )
{
   *out = *a;
   if ( b->x1 > out->x1 ) out->x1 = b->x1;
   if ( b->y1 > out->y1 ) out->y1 = b->y1;
   if ( b->x2 < out->x2 ) out->x2 = b->x2;
   if ( b->y2 < out->y2 ) out->y2 = b->y2;
   if ( out->x1 >= out->x2 ) return GL_FALSE;
   if ( out->y1 >= out->y2 ) return GL_FALSE;
   return GL_TRUE;
}

void radeonFlushVerticesLocked( radeonContextPtr rmesa )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   drmBufPtr buffer = rmesa->vert_buf;
   int count = rmesa->num_verts;
   int prim = RADEON_TRIANGLES;
   int fd = rmesa->driScreen->fd;
   int i;

   rmesa->vert_buf = NULL;
   rmesa->num_verts = 0;

   if ( !buffer )
      return;

   if ( rmesa->dirty & ~RADEON_UPLOAD_CLIPRECTS )
      radeonEmitHwStateLocked( rmesa );

   if ( !nbox )
      count = 0;

   if ( nbox >= RADEON_NR_SAREA_CLIPRECTS )
      rmesa->dirty |= RADEON_UPLOAD_CLIPRECTS;

   if ( !count || !(rmesa->dirty & RADEON_UPLOAD_CLIPRECTS) )
   {
      if ( nbox == 1 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmRadeonFlushVertexBuffer( fd, prim, buffer->idx, count, 1 );
   }
   else
   {
      for ( i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + RADEON_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;
	 int discard = 0;

	 if ( rmesa->scissor ) {
	    rmesa->sarea->nbox = 0;

	    for ( ; i < nr ; i++ ) {
	       *b = pbox[i];
	       if ( intersect_rect( b, b, &rmesa->scissor_rect ) ) {
		  rmesa->sarea->nbox++;
		  b++;
	       }
	    }

	    /* Culled?
	     */
	    if ( !rmesa->sarea->nbox ) {
	       if ( nr < nbox ) continue;
	       count = 0;
	    }
	 } else {
	    rmesa->sarea->nbox = nr - i;
	    for ( ; i < nr ; i++) {
	       *b++ = pbox[i];
	    }
	 }

	 /* Finished with the buffer?
	  */
	 if ( nr == nbox ) {
	    discard = 1;
	 }

	 rmesa->sarea->dirty |= RADEON_UPLOAD_CLIPRECTS;
	 drmRadeonFlushVertexBuffer( fd, prim, buffer->idx, count, discard );
      }
   }

   rmesa->dirty &= ~RADEON_UPLOAD_CLIPRECTS;
}



/* ================================================================
 * Indexed vertex buffer handling
 */

void radeonGetEltBufLocked( radeonContextPtr rmesa )
{
   rmesa->elt_buf = radeonGetBufferLocked( rmesa );
}

void radeonFireEltsLocked( radeonContextPtr rmesa,
			   GLuint start, GLuint end,
			   GLuint discard )
{
   XF86DRIClipRectPtr pbox = rmesa->pClipRects;
   int nbox = rmesa->numClipRects;
   drmBufPtr buffer = rmesa->elt_buf;
   int prim = RADEON_TRIANGLES;
   int fd = rmesa->driScreen->fd;
   int i;

   if ( !buffer )
      return;

   if ( rmesa->dirty & ~RADEON_UPLOAD_CLIPRECTS )
      radeonEmitHwStateLocked( rmesa );

   if ( !nbox )
      end = start;

   if ( nbox >= RADEON_NR_SAREA_CLIPRECTS )
      rmesa->dirty |= RADEON_UPLOAD_CLIPRECTS;

   if ( start == end || !(rmesa->dirty & RADEON_UPLOAD_CLIPRECTS) )
   {
      if ( nbox == 1 ) {
	 rmesa->sarea->nbox = 0;
      } else {
	 rmesa->sarea->nbox = nbox;
      }

      drmRadeonFlushIndices( fd, prim, buffer->idx, start, end, discard );
   }
   else
   {
      for ( i = 0 ; i < nbox ; ) {
	 int nr = MIN2( i + RADEON_NR_SAREA_CLIPRECTS, nbox );
	 XF86DRIClipRectPtr b = rmesa->sarea->boxes;
	 int d = 0;

	 if ( rmesa->scissor ) {
	    rmesa->sarea->nbox = 0;

	    for ( ; i < nr ; i++ ) {
	       *b = pbox[i];
	       if ( intersect_rect( b, b, &rmesa->scissor_rect ) ) {
		  rmesa->sarea->nbox++;
		  b++;
	       }
	    }

	    /* Culled?
	     */
	    if ( !rmesa->sarea->nbox ) {
	       if ( nr < nbox ) continue;
	       end = start;
	    }
	 } else {
	    rmesa->sarea->nbox = nr - i;
	    for ( ; i < nr ; i++) {
	       *b++ = pbox[i];
	    }
	 }

	 /* Finished with the buffer?
	  */
	 if ( nr == nbox ) {
	    d = discard;
	 }

	 rmesa->sarea->dirty |= RADEON_UPLOAD_CLIPRECTS;
	 drmRadeonFlushIndices( fd, prim, buffer->idx, start, end, d );
      }
   }

   rmesa->dirty &= ~RADEON_UPLOAD_CLIPRECTS;
}

void radeonFlushEltsLocked( radeonContextPtr rmesa )
{
   if ( rmesa->first_elt != rmesa->next_elt ) {
      radeonFireEltsLocked( rmesa,
			    ((char *)rmesa->first_elt -
			     (char *)rmesa->elt_buf->address),
			    ((char *)rmesa->next_elt -
			     (char *)rmesa->elt_buf->address),
			    0 );

      ALIGN_NEXT_ELT( rmesa );
      rmesa->first_elt = rmesa->next_elt;
   }
}

void radeonReleaseBufLocked( radeonContextPtr rmesa, drmBufPtr buffer )
{
   int fd = rmesa->driScreen->fd;

   if ( !buffer )
      return;

   drmRadeonFlushVertexBuffer( fd, RADEON_TRIANGLES, buffer->idx, 0, 1 );
}


/* Allocate some space in the current vertex buffer.  If the current
 * buffer is full, flush it and grab another one.
 */
CARD32 *radeonAllocVertices( radeonContextPtr rmesa, GLuint count )
{
   return radeonAllocVerticesInline( rmesa, count );
}


/* ================================================================
 * Texture uploads
 */

void radeonFireBlitLocked( radeonContextPtr rmesa, drmBufPtr buffer,
			   GLint offset, GLint pitch, GLint format,
			   GLint x, GLint y, GLint width, GLint height )
{
#if 0
   GLint ret;

   ret = drmRadeonTextureBlit( rmesa->driFd, buffer->idx,
			       offset, pitch, format,
			       x, y, width, height );

   if ( ret ) {
      UNLOCK_HARDWARE( rmesa );
      fprintf( stderr, "drmRadeonTextureBlit: return = %d\n", ret );
      exit( 1 );
   }
#endif
}


/* ================================================================
 * SwapBuffers with client-side throttling
 */

#define RADEON_MAX_OUTSTANDING	2

/* Throttle the frame rate -- only allow one pending swap buffers
 * request at a time.
 * GH: We probably don't want a timeout here, as we can wait as
 * long as we want for a frame to complete.  If it never does, then
 * the card has locked.
 */
#if USE_IN_MEMORY_SCRATCH_REGS
static int radeonWaitForFrameCompletion( radeonContextPtr rmesa )
{
   RADEONSAREAPrivPtr sarea = rmesa->sarea;
   __volatile__ CARD32 *scratch = rmesa->radeonScreen->scratch;
   CARD32 frame;
   int wait = 0;

   while ( 1 ) {
      /* Read the frame counter from the in-memory copy of the scratch
       * register.  Nifty, eh?  Should significantly reduce the bus
       * traffic for SwapBuffers-limited apps (generally pretty trivial
       * ones, but anyway).
       */
      frame = scratch[0];
      if ( sarea->last_frame - frame <= RADEON_MAX_OUTSTANDING ) {
	 break;
      }
      wait++;
   }

   return wait;
}
#else
static void delay( void ) {
/* Prevent an optimizing compiler from removing a spin loop */
}

static int radeonWaitForFrameCompletion( radeonContextPtr rmesa )
{
   unsigned char *RADEONMMIO = rmesa->radeonScreen->mmio.map;
   RADEONSAREAPrivPtr sarea = rmesa->sarea;
   CARD32 frame;
   int wait = 0;
   int i;

   while ( 1 ) {
#if defined(__alpha__)
      /* necessary to preserve the Alpha paradigm */
      /* NOTE: this will not work on SPARSE machines */
      mem_barrier();
      frame = *(volatile CARD32 *)(void *)(RADEONMMIO + RADEON_LAST_FRAME_REG);
#else
      frame = INREG( RADEON_LAST_FRAME_REG );
#endif
      if ( sarea->last_frame - frame <= RADEON_MAX_OUTSTANDING ) {
	 break;
      }
      wait++;
      /* Spin in place a bit so we aren't hammering the bus */
      for ( i = 0 ; i < 1024 ; i++ ) {
	 delay();
      }
   }

   return wait;
}
#endif

/* Copy the back color buffer to the front color buffer.
 */
void radeonSwapBuffers( radeonContextPtr rmesa )
{
   GLint nbox;
   GLint i;
   GLint ret;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "\n%s( %p )\n\n", __FUNCTION__, rmesa->glCtx );
   }

   FLUSH_BATCH( rmesa );

   LOCK_HARDWARE( rmesa );

   nbox = rmesa->numClipRects;	/* must be in locked region */

   /* Throttle the frame rate -- only allow one pending swap buffers
    * request at a time.
    */
   if ( !radeonWaitForFrameCompletion( rmesa ) ) {
      rmesa->hardwareWentIdle = 1;
   } else {
      rmesa->hardwareWentIdle = 0;
   }

   for ( i = 0 ; i < nbox ; ) {
      GLint nr = MIN2( i + RADEON_NR_SAREA_CLIPRECTS , nbox );
      XF86DRIClipRectPtr box = rmesa->pClipRects;
      XF86DRIClipRectPtr b = rmesa->sarea->boxes;
      GLint n = 0;

      for ( ; i < nr ; i++ ) {
	 *b++ = *(XF86DRIClipRectPtr)&box[i];
	 n++;
      }
      rmesa->sarea->nbox = n;

      ret = drmRadeonSwapBuffers( rmesa->driFd );

      if ( ret ) {
	 fprintf( stderr, "drmRadeonSwapBuffers: return = %d\n", ret );
	 UNLOCK_HARDWARE( rmesa );
	 exit( 1 );
      }
   }

   UNLOCK_HARDWARE( rmesa );

   rmesa->new_state |= RADEON_NEW_CONTEXT;
   rmesa->dirty |= (RADEON_UPLOAD_CONTEXT |
		    RADEON_UPLOAD_MASKS |
		    RADEON_UPLOAD_CLIPRECTS);

#if ENABLE_PERF_BOXES
   /* Log the performance counters if necessary */
   radeonPerformanceCounters( rmesa );
#endif
}

void radeonPageFlip( radeonContextPtr rmesa )
{
   GLint ret;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "\n%s( %p ): page=%d\n\n",
	       __FUNCTION__, rmesa->glCtx, rmesa->currentPage );
   }

   FLUSH_BATCH( rmesa );

   LOCK_HARDWARE( rmesa );

   /* Throttle the frame rate -- only allow one pending swap buffers
    * request at a time.
    */
   if ( !radeonWaitForFrameCompletion( rmesa ) ) {
      rmesa->hardwareWentIdle = 1;
   } else {
      rmesa->hardwareWentIdle = 0;
   }

   /* The kernel will have been initialized to perform page flipping
    * on a swapbuffers ioctl.
    */
   ret = drmRadeonSwapBuffers( rmesa->driFd );

   UNLOCK_HARDWARE( rmesa );

   if ( ret ) {
      fprintf( stderr, "drmRadeonSwapBuffers: return = %d\n", ret );
      exit( 1 );
   }

   if ( rmesa->currentPage == 0 ) {
	 rmesa->drawOffset = rmesa->radeonScreen->frontOffset;
	 rmesa->drawPitch  = rmesa->radeonScreen->frontPitch;
	 rmesa->currentPage = 1;
   } else {
	 rmesa->drawOffset = rmesa->radeonScreen->backOffset;
	 rmesa->drawPitch  = rmesa->radeonScreen->backPitch;
	 rmesa->currentPage = 0;
   }

   rmesa->setup.rb3d_coloroffset = rmesa->drawOffset;
   rmesa->setup.rb3d_colorpitch = rmesa->drawPitch;

   rmesa->new_state |= RADEON_NEW_WINDOW;

   /* FIXME: Do we need this anymore? */
   rmesa->new_state |= RADEON_NEW_CONTEXT;
   rmesa->dirty |= (RADEON_UPLOAD_CONTEXT |
		    RADEON_UPLOAD_MASKS |
		    RADEON_UPLOAD_CLIPRECTS);

#if ENABLE_PERF_BOXES
   /* Log the performance counters if necessary */
   radeonPerformanceCounters( rmesa );
#endif
}


/* ================================================================
 * Buffer clear
 */
#define RADEON_MAX_CLEARS	256

static GLbitfield radeonDDClear( GLcontext *ctx, GLbitfield mask,
				 GLboolean all,
				 GLint cx, GLint cy, GLint cw, GLint ch )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   __DRIdrawablePrivate *dPriv = rmesa->driDrawable;
   RADEONSAREAPrivPtr sarea = rmesa->sarea;
#if USE_IN_MEMORY_SCRATCH_REGS
   __volatile__ CARD32 *scratch = rmesa->radeonScreen->scratch;
#else
   unsigned char *RADEONMMIO = rmesa->radeonScreen->mmio.map;
#endif
   CARD32 clear;
   GLuint flags = 0;
   GLuint color_mask = 0;
   GLuint depth_mask = 0;
   GLint ret, i;

   if ( RADEON_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s:  all=%d cx=%d cy=%d cw=%d ch=%d\n",
	       __FUNCTION__, all, cx, cy, cw, ch );
   }

   FLUSH_BATCH( rmesa );

   /* Update and emit any new state.  We need to do this here to catch
    * changes to the masks.
    * FIXME: Just update the masks?
    */
   if ( rmesa->new_state )
      radeonDDUpdateHWState( ctx );

   if ( mask & DD_FRONT_LEFT_BIT ) {
      flags |= DRM_RADEON_FRONT;
      color_mask = rmesa->setup.rb3d_planemask;
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if ( mask & DD_BACK_LEFT_BIT ) {
      flags |= DRM_RADEON_BACK;
      color_mask = rmesa->setup.rb3d_planemask;
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if ( mask & DD_DEPTH_BIT ) {
      if ( ctx->Depth.Mask ) {
	 flags |= DRM_RADEON_DEPTH;
	 depth_mask |= rmesa->DepthMask;
      }
      mask &= ~DD_DEPTH_BIT;
   }
#if 0
   /* FIXME: Add stencil support */
   if ( mask & DD_STENCIL_BIT ) {
      flags |= DRM_RADEON_DEPTH;
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

   /* Throttle the number of clear ioctls we do.
    */
#if USE_IN_MEMORY_SCRATCH_REGS
   while ( 1 ) {
      clear = scratch[2];
      if ( sarea->last_clear - clear <= RADEON_MAX_CLEARS ) {
	 break;
      }
   }
#else
   while ( 1 ) {
#if defined(__alpha__)
     /* necessary to preserve the Alpha paradigm */
     /* NOTE: this will not work on SPARSE machines */
     mem_barrier();
     clear = *(volatile CARD32 *)(void *)(RADEONMMIO + RADEON_LAST_CLEAR_REG);
#else
      clear = INREG( RADEON_LAST_CLEAR_REG );
#endif
      if ( sarea->last_clear - clear <= RADEON_MAX_CLEARS ) {
	 break;
      }
      /* Spin in place a bit so we aren't hammering the bus */
      for ( i = 0 ; i < 1024 ; i++ ) {
	 delay();
      }
   }
#endif

   for ( i = 0 ; i < rmesa->numClipRects ; ) {
      GLint nr = MIN2( i + RADEON_NR_SAREA_CLIPRECTS, rmesa->numClipRects );
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
	    *b++ = *(XF86DRIClipRectPtr)&box[i];
	    n++;
	 }
      }

      rmesa->sarea->nbox = n;

      if ( RADEON_DEBUG & DEBUG_VERBOSE_IOCTL ) {
	 fprintf( stderr,
		  "drmRadeonClear: flag 0x%x color %x depth %x nbox %d\n",
		  flags,
		  (GLuint)rmesa->ClearColor,
		  (GLuint)rmesa->ClearDepth,
		  rmesa->sarea->nbox );
      }

      ret = drmRadeonClear( rmesa->driFd, flags,
			    rmesa->ClearColor, rmesa->ClearDepth,
			    color_mask, depth_mask,
			    rmesa->sarea->boxes, rmesa->sarea->nbox );

      if ( ret ) {
	 UNLOCK_HARDWARE( rmesa );
	 fprintf( stderr, "drmRadeonClear: return = %d\n", ret );
	 exit( 1 );
      }
   }

   UNLOCK_HARDWARE( rmesa );

   rmesa->dirty |= RADEON_UPLOAD_CLIPRECTS;

   return mask;
}


void radeonWaitForIdleLocked( radeonContextPtr rmesa )
{
    int fd = rmesa->radeonScreen->driScreen->fd;
    int to = 0;
    int ret;

    do {
	ret = drmRadeonWaitForIdleCP( fd );
    } while ( ( ret == -EBUSY ) && ( to++ < RADEON_TIMEOUT ) );

    if ( ret < 0 ) {
	drmRadeonEngineReset( fd );
	UNLOCK_HARDWARE( rmesa );
	fprintf( stderr, "Error: Radeon timed out... exiting\n" );
	exit( -1 );
    }
}


void radeonDDInitIoctlFuncs( GLcontext *ctx )
{
    ctx->Driver.Clear = radeonDDClear;
}
