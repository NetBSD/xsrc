/* $XFree86: xc/programs/Xserver/GL/glx/glxfb.c,v 1.2 1999/06/14 07:31:27 dawes Exp $ */
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
** An implementation of a buffer which is part of the front buffer
*/

#include "glxserver.h"
#include "glxutil.h"
#include "glxfb.h"

#include <gcstruct.h>

/* can't include glmath.h */
extern GLuint __glFloorLog2(GLuint);

typedef struct __GLFBbufferInfoRec {
    GCPtr	pGC;
} __GLFBbufferInfo;

extern PixmapPtr __glXPrivPixGetPtr(__GLdrawableBuffer *);

/* ---------------------------------------------------------- */

static GLboolean
Resize(__GLdrawableBuffer *buf, 
       GLint x, GLint y, GLuint width, GLuint height,
       __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    buf->width = width;
    buf->height = height;
    buf->byteWidth = width * buf->elementSize;
    buf->outerWidth = width;

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

/*
** Do a swap buffer with
** a memory surface as a back buffer
** a FB surface as a front buffer
*/
GLboolean
__glXFBMemSwapBuffers(__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;
    __GLdrawableBuffer *front = &glPriv->frontBuffer;
    __GLdrawableBuffer *back = &glPriv->backBuffer;
    __GLFBbufferInfo *bufferInfo;
    GCPtr pGC;
    GLint width, height, depth, pad;
    GLubyte *buf;

    bufferInfo = (__GLFBbufferInfo *) front->other;
    pGC = bufferInfo->pGC;

    width = back->width;
    height = back->height;
    depth = back->depth;
    buf = back->base;
    pad = back->outerWidth - back->width;	/* back buffer padding */
    /* adjust buffer padding. X wants left, GL has right */
    buf -= pad;

    ValidateGC(glxPriv->pDraw, pGC);
    (*pGC->ops->PutImage)(glxPriv->pDraw, pGC,
			 depth,
			 0, 0, width, height,
			 pad, ZPixmap,
			 (char *)buf);

    return GL_TRUE;
}

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __GLFBbufferInfo *bufferInfo;

    bufferInfo = (__GLFBbufferInfo *) buf->other;

    if (bufferInfo->pGC) {
	FreeScratchGC(bufferInfo->pGC);
    }

    __glXFree(bufferInfo);
    buf->other = NULL;
}

/*
** function to return the X GC of this buffer (to be used by DDX)
*/
GCPtr __glXFBGetGC(__GLdrawableBuffer *buf)
{
    __GLFBbufferInfo *bufferInfo;

    bufferInfo = (__GLFBbufferInfo *) buf->other;

    if (bufferInfo) {
	return bufferInfo->pGC;
    } else {
	return NULL;
    }
}


void
__glXInitFB(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    __GLFBbufferInfo *bufferInfo;
    __GLXdrawablePrivate *glxPriv = (__GLXdrawablePrivate *) glPriv->other;
    GCPtr pGC;

    buf->depth = bits;
    buf->width = buf->height = 0;	/* to be filled during Update */
    buf->handle = buf->base = NULL;	/* to be filled during Update */
    buf->size = 0;
    buf->byteWidth = 0;
    buf->elementSize = ((bits-1) / 8) + 1;
    buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);

    buf->resize = Resize;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = NULL;
    buf->free = Free;

    /* allocate local information */
    bufferInfo = (__GLFBbufferInfo *) __glXMalloc(sizeof(__GLFBbufferInfo));
    buf->other = (void *) bufferInfo;

    pGC = CreateScratchGC(glxPriv->pDraw->pScreen,
			  glxPriv->pDraw->depth);
    bufferInfo->pGC = pGC;
    (*pGC->funcs->ChangeClip)(pGC, CT_NONE, NULL, 0);
}
