/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_ioctl.h,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#ifndef I830_IOCTL_H
#define I830_IOCTL_H

#include "i830_drv.h"


GLuint *i830AllocDwords( i830ContextPtr imesa, int dwords );

void i830GetGeneralDmaBufferLocked( i830ContextPtr mmesa ); 

void i830FlushVertices( i830ContextPtr mmesa ); 
void i830FlushVerticesLocked( i830ContextPtr mmesa );

void i830FlushGeneralLocked( i830ContextPtr imesa );
void i830WaitAgeLocked( i830ContextPtr imesa, int age );
void i830WaitAge( i830ContextPtr imesa, int age );

void i830DmaFinish( i830ContextPtr imesa );

void i830RegetLockQuiescent( i830ContextPtr imesa );

void i830DDInitIoctlFuncs( GLcontext *ctx );

void i830SwapBuffers( i830ContextPtr imesa );

int i830_check_copy(int fd);

GLbitfield i830Clear( GLcontext *ctx, GLbitfield mask, GLboolean all,
		      GLint cx, GLint cy, GLint cw, GLint ch );

#define FLUSH_BATCH(imesa) do {						\
        if (I830_DEBUG&DEBUG_VERBOSE_IOCTL)  				\
              fprintf(stderr, "FLUSH_BATCH in %s\n", __FUNCTION__);	\
	if (imesa->vertex_dma_buffer) i830FlushVertices(imesa);		\
} while (0)

extern drmBufPtr i830_get_buffer_ioctl( i830ContextPtr imesa );

static __inline
GLuint *i830AllocDwordsInline( i830ContextPtr imesa, int dwords )
{
   int bytes = dwords * 4;
   GLuint *start;

   if (!imesa->vertex_dma_buffer) 
   {
      LOCK_HARDWARE(imesa);
      imesa->vertex_dma_buffer = i830_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   } 
   else if (imesa->vertex_dma_buffer->used + bytes > 
	    imesa->vertex_dma_buffer->total) 
   {
      LOCK_HARDWARE(imesa);
      i830FlushVerticesLocked( imesa );
      imesa->vertex_dma_buffer = i830_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   }

   start = (GLuint *)((char *)imesa->vertex_dma_buffer->address + 
		      imesa->vertex_dma_buffer->used);

   imesa->vertex_dma_buffer->used += bytes;
   return start;
}

static __inline
GLuint *i830AllocDwordsInlineLocked( i830ContextPtr imesa, int dwords )
{
   int bytes = dwords * 4;
   GLuint *start;

   if (!imesa->vertex_dma_buffer) 
   {
      imesa->vertex_dma_buffer = i830_get_buffer_ioctl( imesa );
   } 
   else if (imesa->vertex_dma_buffer->used + bytes > 
	    imesa->vertex_dma_buffer->total) 
   {
      i830FlushVerticesLocked( imesa );
      imesa->vertex_dma_buffer = i830_get_buffer_ioctl( imesa );
   }

   start = (GLuint *)((char *)imesa->vertex_dma_buffer->address + 
		      imesa->vertex_dma_buffer->used);

   imesa->vertex_dma_buffer->used += bytes;
   return start;
}

#endif
