/* $XFree86: xc/programs/Xserver/GL/glx/glxpix.c,v 1.3 2000/09/26 15:57:02 tsi Exp $ */
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
** An implementation of a glx pixmap buffer 
*/

#include "glxserver.h"
#include "glxutil.h"
#include "glxpix.h"

#include <gcstruct.h>

/* don't want to include glmath.h */
extern GLuint __glFloorLog2(GLuint);

typedef struct __GLPixBufferInfoRec {
    GCPtr pGC;
} __GLPixBufferInfo;

/* ---------------------------------------------------------- */

static GLboolean
Resize(__GLdrawableBuffer *buf, 
       GLint x, GLint y, GLuint width, GLuint height,
       __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    buf->width = width;
    buf->height = width;
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

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __GLPixBufferInfo *bufferInfo;

    if (LookupIDByType((XID)(long)buf->handle, __glXPixmapRes)) {
	FreeResource((XID)(long)buf->handle, FALSE);
	buf->handle = NULL;
    }

    bufferInfo = (__GLPixBufferInfo *) buf->other;

    if (bufferInfo->pGC) {
	FreeScratchGC(bufferInfo->pGC);
    }

    __glXFree(bufferInfo);
    buf->other = NULL;
}

void
__glXInitPix(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, 
	     GLint bits, XID glxpixmapId, __GLXpixmap *pGlxPixmap)
{
    __GLPixBufferInfo *bufferInfo;

    buf->width = buf->height = 0;	/* to be filled during Update */
    buf->depth = bits;
    buf->size = 0;
    buf->base = NULL;
    buf->byteWidth = 0;
    buf->elementSize = ((bits-1) / 8) + 1;
    buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);

    buf->handle = (void *)(long) glxpixmapId;
    pGlxPixmap->refcnt++;

    buf->resize = Resize;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = NULL;
    buf->free = Free;

    /* allocate local information */
    bufferInfo = (__GLPixBufferInfo *) __glXMalloc(sizeof(__GLPixBufferInfo));
    buf->other = (void *) bufferInfo;

    bufferInfo->pGC = CreateScratchGC(pGlxPixmap->pDraw->pScreen,
				      pGlxPixmap->pDraw->depth);
}
