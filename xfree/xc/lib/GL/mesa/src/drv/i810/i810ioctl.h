/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810ioctl.h,v 1.5 2000/10/24 22:45:01 dawes Exp $ */

#ifndef I810_IOCTL_H
#define I810_IOCTL_H

#include "i810context.h"


GLuint *i810AllocDwords( i810ContextPtr imesa, int dwords );

void i810GetGeneralDmaBufferLocked( i810ContextPtr mmesa ); 

void i810FlushVertices( i810ContextPtr mmesa ); 
void i810FlushVerticesLocked( i810ContextPtr mmesa );

void i810FlushGeneralLocked( i810ContextPtr imesa );
void i810WaitAgeLocked( i810ContextPtr imesa, int age );
void i810WaitAge( i810ContextPtr imesa, int age );

void i810DmaFinish( i810ContextPtr imesa );

void i810RegetLockQuiescent( i810ContextPtr imesa );

void i810DDInitIoctlFuncs( GLcontext *ctx );

void i810SwapBuffers( i810ContextPtr imesa );

int i810_check_copy(int fd);

GLbitfield i810Clear( GLcontext *ctx, GLbitfield mask, GLboolean all,
		      GLint cx, GLint cy, GLint cw, GLint ch );

#define FLUSH_BATCH(imesa) do {						\
        if (I810_DEBUG&DEBUG_VERBOSE_IOCTL)  				\
              fprintf(stderr, "FLUSH_BATCH in %s\n", __FUNCTION__);	\
	if (imesa->vertex_dma_buffer) i810FlushVertices(imesa);		\
} while (0)

extern drmBufPtr i810_get_buffer_ioctl( i810ContextPtr imesa );

static __inline
GLuint *i810AllocDwordsInline( i810ContextPtr imesa, int dwords )
{
   int bytes = dwords * 4;
   GLuint *start;

   if (!imesa->vertex_dma_buffer) 
   {
      LOCK_HARDWARE(imesa);
      imesa->vertex_dma_buffer = i810_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   } 
   else if (imesa->vertex_dma_buffer->used + bytes > 
	    imesa->vertex_dma_buffer->total) 
   {
      LOCK_HARDWARE(imesa);
      i810FlushVerticesLocked( imesa );
      imesa->vertex_dma_buffer = i810_get_buffer_ioctl( imesa );
      UNLOCK_HARDWARE(imesa);
   }

   start = (GLuint *)((char *)imesa->vertex_dma_buffer->address + 
		      imesa->vertex_dma_buffer->used);

   imesa->vertex_dma_buffer->used += bytes;
   return start;
}

#endif
