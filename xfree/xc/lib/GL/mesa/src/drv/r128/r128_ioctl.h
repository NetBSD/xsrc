/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_ioctl.h,v 1.1 2000/12/04 19:21:46 dawes Exp $ */
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

#ifndef __R128_IOCTL_H__
#define __R128_IOCTL_H__

#ifdef GLX_DIRECT_RENDERING

#include "r128_dri.h"
#include "r128_reg.h"
#include "r128_lock.h"

#include "xf86drm.h"
#include "xf86drmR128.h"

#define R128_DEFAULT_TOTAL_CCE_TIMEOUT 1000000 /* usecs */

#define R128_BUFFER_MAX_DWORDS	(R128_BUFFER_SIZE / sizeof(CARD32))


#define FLUSH_BATCH( r128ctx )						\
do {									\
   if ( R128_DEBUG & DEBUG_VERBOSE_IOCTL )				\
      fprintf( stderr, "FLUSH_BATCH in %s\n", __FUNCTION__ );		\
   if ( r128ctx->vert_buf ) {						\
      r128FlushVertices( r128ctx );					\
   } else if ( r128ctx->next_elt != r128ctx->first_elt ) {		\
      r128FlushElts( r128ctx );						\
   }									\
} while (0)

#define r128FlushVertices( r128ctx )		\
do {						\
   LOCK_HARDWARE( r128ctx );			\
   r128FlushVerticesLocked( r128ctx );		\
   UNLOCK_HARDWARE( r128ctx );			\
} while (0)


extern drmBufPtr r128GetBufferLocked( r128ContextPtr r128ctx );
extern void r128FlushVerticesLocked( r128ContextPtr r128ctx );


#define r128FlushElts( r128ctx )		\
do {						\
   LOCK_HARDWARE( r128ctx );			\
   r128FlushEltsLocked( r128ctx );		\
   UNLOCK_HARDWARE( r128ctx );			\
} while (0)

extern void r128GetEltBufLocked( r128ContextPtr r128ctx );
extern void r128FlushEltsLocked( r128ContextPtr r128ctx );
extern void r128FireEltsLocked( r128ContextPtr r128ctx,
				GLuint start, GLuint end,
				GLuint discard );
extern void r128ReleaseBufLocked( r128ContextPtr r128ctx, drmBufPtr buffer );


/* 64-bit align the next element address, and then make room for the
 * next indexed prim packet header.
 */
#define ALIGN_NEXT_ELT( r128ctx )					\
do {									\
   r128ctx->next_elt = (GLushort *)					\
      (((GLuint)r128ctx->next_elt + 7) & ~0x7);				\
   r128ctx->next_elt = (GLushort *)					\
      ((GLubyte *)r128ctx->next_elt + R128_INDEX_PRIM_OFFSET);		\
} while (0)


/* Make this available as both a regular and an inline function.
 */
extern CARD32 *r128AllocVertices( r128ContextPtr r128ctx, int count );

static __inline CARD32 *r128AllocVerticesInline( r128ContextPtr r128ctx,
						 int count )
{
    int bytes = count * r128ctx->vertsize * sizeof(CARD32);
    CARD32 *head;

    if ( !r128ctx->vert_buf ) {
	LOCK_HARDWARE( r128ctx );

	if ( r128ctx->first_elt != r128ctx->next_elt ) {
	    r128FlushEltsLocked( r128ctx );
	}

	r128ctx->vert_buf = r128GetBufferLocked( r128ctx );

	UNLOCK_HARDWARE( r128ctx );
    } else if ( r128ctx->vert_buf->used + bytes > r128ctx->vert_buf->total ) {
	LOCK_HARDWARE( r128ctx );

	r128FlushVerticesLocked( r128ctx );
	r128ctx->vert_buf = r128GetBufferLocked( r128ctx );

	UNLOCK_HARDWARE( r128ctx );
    }

    head = (CARD32 *)((char *)r128ctx->vert_buf->address +
		      r128ctx->vert_buf->used);

    r128ctx->num_verts += count;
    r128ctx->vert_buf->used += bytes;
    return head;
}


extern void r128FireBlitLocked( r128ContextPtr r128ctx, drmBufPtr buffer,
				GLint offset, GLint pitch, GLint format,
				GLint x, GLint y, GLint width, GLint height );


extern void r128WriteDepthSpanLocked( r128ContextPtr r128ctx,
				      GLuint n, GLint x, GLint y,
				      const GLdepth depth[],
				      const GLubyte mask[] );
extern void r128WriteDepthPixelsLocked( r128ContextPtr r128ctx, GLuint n,
					const GLint x[], const GLint y[],
					const GLdepth depth[],
					const GLubyte mask[] );
extern void r128ReadDepthSpanLocked( r128ContextPtr r128ctx,
				     GLuint n, GLint x, GLint y );
extern void r128ReadDepthPixelsLocked( r128ContextPtr r128ctx, GLuint n,
				       const GLint x[], const GLint y[] );


extern void r128SwapBuffers( r128ContextPtr r128ctx );


#define r128WaitForIdle( r128ctx )		\
do {						\
   LOCK_HARDWARE( r128ctx );			\
   r128WaitForIdleLocked( r128ctx );		\
   UNLOCK_HARDWARE( r128ctx );			\
} while (0)

extern void r128WaitForIdleLocked( r128ContextPtr r128ctx );


extern void r128DDInitIoctlFuncs( GLcontext *ctx );



/* ================================================================
 * Deprecated functions:
 */

typedef union {
    float f;
    int   i;
} floatTOint;

/* Insert an integer value into the CCE ring buffer. */
#define R128CCE(v)				\
do {						\
   r128ctx->CCEbuf[r128ctx->CCEcount] = (v);	\
   r128ctx->CCEcount++;				\
} while (0)

/* Insert an floating point value into the CCE ring buffer. */
#define R128CCEF(v)				\
do {						\
   floatTOint fTi;				\
   fTi.f = (v);					\
   r128ctx->CCEbuf[r128ctx->CCEcount] = fTi.i;	\
   r128ctx->CCEcount++;				\
} while (0)

/* Insert a type-[0123] packet header into the ring buffer */
#define R128CCE0(p,r,n)   R128CCE((p) | ((n) << 16) | ((r) >> 2))
#define R128CCE1(p,r1,r2) R128CCE((p) | (((r2) >> 2) << 11) | ((r1) >> 2))
#define R128CCE2(p)       R128CCE((p))
#define R128CCE3(p,n)     R128CCE((p) | ((n) << 16))

#define R128CCE_SUBMIT_PACKET()						  \
do {									  \
   r128SubmitPacketLocked( r128ctx, r128ctx->CCEbuf, r128ctx->CCEcount ); \
   r128ctx->CCEcount = 0;						  \
} while (0)

extern void r128SubmitPacketLocked( r128ContextPtr r128ctx,
				    CARD32 *buf, GLuint count );

#endif
#endif /* __R128_IOCTL_H__ */
