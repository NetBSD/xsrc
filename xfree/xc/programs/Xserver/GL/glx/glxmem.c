/* $XFree86: xc/programs/Xserver/GL/glx/glxmem.c,v 1.4 1999/07/18 08:34:23 dawes Exp $ */
/*
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
**
** $SGI$
*/

/*
** Implementation of a buffer in main memory
*/

#include "glxserver.h"
#include "glxmem.h"
#include "glxext.h"
#include "GL/internal/glcore.h"

/* don't want to include glmath.h */
extern GLuint __glFloorLog2(GLuint);

/* ---------------------------------------------------------- */

#define	BUF_ALIGN 32	/* x86 cache alignment (used for assembly paths) */
#define	BUF_ALIGN_MASK	(BUF_ALIGN-1)

static GLboolean
Resize(__GLdrawableBuffer *buf, 
       GLint x, GLint y, GLuint width, GLuint height,
       __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __GLXdrawablePrivate *glxPriv = (__GLXdrawablePrivate *) glPriv->other;
    GLuint newSize;
    void *ubase;
    GLint pixelWidth;
    GLint alignedWidth;

    /*
    ** Note: 
    **	buf->handle : unaligned base
    **  buf->base   : aligned base
    */

    pixelWidth = BUF_ALIGN / buf->elementSize;
    alignedWidth = (width & ~(pixelWidth-1)) + pixelWidth;

    newSize = alignedWidth * height * buf->elementSize;

    /*
    ** Only allocate buffer space for the SGI core.
    ** Mesa handles its own buffer allocations.
    */
#if defined(__GL_BUFFER_SIZE_TRACKS_WINDOW)
    if (__glXCoreType() == GL_CORE_SGI) {
#else
    if (newSize > buf->size && __glXCoreType() == GL_CORE_SGI) {
#endif
	if (buf->handle) {
	    ubase = (*glPriv->realloc)(buf->handle, newSize + BUF_ALIGN_MASK);
	    if (ubase == NULL) {
		return GL_FALSE;
	    }
	} else {
	    ubase = (*glPriv->malloc)(newSize + BUF_ALIGN_MASK);
	    if (ubase == NULL) {
		return GL_FALSE;
	    }
	}
	buf->size = newSize;

	buf->handle = ubase;
	buf->base = (void *)(((size_t)ubase + BUF_ALIGN_MASK) &
			     (unsigned int) ~BUF_ALIGN_MASK);
	assert(((size_t)buf->base % BUF_ALIGN) == 0);
    }

    buf->width = width;
    buf->height = height;
    buf->byteWidth = alignedWidth * buf->elementSize;
    buf->outerWidth = alignedWidth;

    return GL_TRUE;
}

static void
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    if (buf->handle) {
	(*glPriv->free)(buf->handle);
	buf->handle = NULL;
    }
}


void
__glXInitMem(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    buf->width = buf->height = 0;	/* to be filled during Update */
    buf->depth = bits;
    buf->size = 0;
    buf->handle = buf->base = NULL;	/* to be filled during Update */
    buf->byteWidth = 0;
    buf->elementSize = ((bits - 1) / 8) + 1;
    buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);

    buf->resize = Resize;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = NULL;
    buf->free = Free;
}
