/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_ioctl.h,v 1.2 2001/04/01 14:00:00 tsi Exp $ */
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

#ifndef __RADEON_IOCTL_H__
#define __RADEON_IOCTL_H__

#ifdef GLX_DIRECT_RENDERING

#include "radeon_dri.h"
#include "radeon_lock.h"

#include "xf86drm.h"
#include "xf86drmRadeon.h"

#define RADEON_BUFFER_MAX_DWORDS	(RADEON_BUFFER_SIZE / sizeof(CARD32))


extern drmBufPtr radeonGetBufferLocked( radeonContextPtr rmesa );
extern void radeonFlushVerticesLocked( radeonContextPtr rmesa );

extern void radeonGetEltBufLocked( radeonContextPtr rmesa );
extern void radeonFlushEltsLocked( radeonContextPtr rmesa );
extern void radeonFireEltsLocked( radeonContextPtr rmesa,
				  GLuint start, GLuint end,
				  GLuint discard );
extern void radeonReleaseBufLocked( radeonContextPtr rmesa, drmBufPtr buffer );

/* Make this available as both a regular and an inline function.
 */
extern CARD32 *radeonAllocVertices( radeonContextPtr rmesa, GLuint count );

static __inline CARD32 *radeonAllocVerticesInline( radeonContextPtr rmesa,
						   GLuint count )
{
   int bytes = count * rmesa->vertsize * 4;
   CARD32 *head;

   if ( !rmesa->vert_buf ) {
      LOCK_HARDWARE( rmesa );

      if ( rmesa->first_elt != rmesa->next_elt ) {
	 radeonFlushEltsLocked( rmesa );
      }

      rmesa->vert_buf = radeonGetBufferLocked( rmesa );

      UNLOCK_HARDWARE( rmesa );
   } else if ( rmesa->vert_buf->used + bytes > rmesa->vert_buf->total ) {
      LOCK_HARDWARE( rmesa );

      radeonFlushVerticesLocked( rmesa );
      rmesa->vert_buf = radeonGetBufferLocked( rmesa );

      UNLOCK_HARDWARE( rmesa );
   }

   head = (CARD32 *)((char *)rmesa->vert_buf->address +
		     rmesa->vert_buf->used);

   rmesa->vert_buf->used += bytes;
   rmesa->num_verts += count;
   return head;
}

extern void radeonFireBlitLocked( radeonContextPtr rmesa,
				  drmBufPtr buffer,
				  GLint offset, GLint pitch, GLint format,
				  GLint x, GLint y,
				  GLint width, GLint height );

extern void radeonSwapBuffers( radeonContextPtr rmesa );
extern void radeonPageFlip( radeonContextPtr rmesa );

extern void radeonWaitForIdleLocked( radeonContextPtr rmesa );


extern void radeonDDInitIoctlFuncs( GLcontext *ctx );


/* ================================================================
 * Helper macros:
 */

#define FLUSH_BATCH( rmesa )						\
do {									\
   if ( RADEON_DEBUG & DEBUG_VERBOSE_IOCTL )				\
      fprintf( stderr, "FLUSH_BATCH in %s\n", __FUNCTION__ );		\
   if ( rmesa->vert_buf ) {						\
      radeonFlushVertices( rmesa );					\
   } else if ( rmesa->next_elt != rmesa->first_elt ) {			\
      radeonFlushElts( rmesa );						\
   }									\
} while (0)

/* 64-bit align the next element address, and then make room for the
 * next indexed prim packet header.
 */
#define ALIGN_NEXT_ELT( rmesa )						\
do {									\
   rmesa->next_elt = (GLushort *)					\
      (((unsigned long)rmesa->next_elt + 7) & ~0x7);			\
   rmesa->next_elt = (GLushort *)					\
      ((GLubyte *)rmesa->next_elt + RADEON_INDEX_PRIM_OFFSET);		\
} while (0)

#define radeonFlushVertices( rmesa )					\
do {									\
   LOCK_HARDWARE( rmesa );						\
   radeonFlushVerticesLocked( rmesa );					\
   UNLOCK_HARDWARE( rmesa );						\
} while (0)

#define radeonFlushElts( rmesa )					\
do {									\
   LOCK_HARDWARE( rmesa );						\
   radeonFlushEltsLocked( rmesa );					\
   UNLOCK_HARDWARE( rmesa );						\
} while (0)

#define radeonWaitForIdle( rmesa )					\
do {									\
   LOCK_HARDWARE( rmesa );						\
   radeonWaitForIdleLocked( rmesa );					\
   UNLOCK_HARDWARE( rmesa );						\
} while (0)

#endif
#endif /* __RADEON_IOCTL_H__ */
