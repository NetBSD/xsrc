/* $XFree86: xc/programs/Xserver/GL/glx/glxbuf.c,v 1.4 1999/07/18 08:34:22 dawes Exp $ */
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

#include "glxserver.h"
#include "glxutil.h"
#include "glxbuf.h"
#include "glxfb.h"
#include "glxmem.h"
#include "glxpix.h"

void
__glXFBInitDrawable(__GLXdrawablePrivate *glxPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint rgbBits;
    GLint accumBits;

    glPriv = &glxPriv->glPriv;
    rgbBits = modes->rgbBits;
    accumBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

#if defined(__GL_ALIGNED_BUFFERS)
    /* initialize pixel alignments (for more details see context.h) */
    glPriv->xAlignment = 1;
    glPriv->yAlignment = 1;
#endif

    glxPriv->swapBuffers = __glXFBMemSwapBuffers;

    glPriv->yInverted = GL_TRUE;	/* Y is upside-down */

    if (modes->doubleBufferMode) {
	if (modes->colorIndexMode) {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, modes->indexBits);
	    __glXInitMem(&glPriv->backBuffer, glPriv, modes->indexBits);
	} else {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, rgbBits);
	    __glXInitMem(&glPriv->backBuffer, glPriv, rgbBits);
	}
    } else {
	if (modes->colorIndexMode) {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, modes->indexBits);
	} else {
	    __glXInitFB(&glPriv->frontBuffer, glPriv, rgbBits);
	}
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (modes->colorIndexMode) {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, modes->indexBits);
	    } else {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, rgbBits);
	    }
	}
    }
#endif

    if (modes->haveAccumBuffer) {
	__glXInitMem(&glPriv->accumBuffer, glPriv, accumBits);
    }
    if (modes->haveDepthBuffer) {
	__glXInitMem(&glPriv->depthBuffer, glPriv, modes->depthBits);
    }
    if (modes->haveStencilBuffer) {
	__glXInitMem(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }
}

void
__glXPixInitDrawable(__GLXdrawablePrivate *glxPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint rgbBits;
    GLint accumBits;

    assert(glxPriv->pGlxPixmap);

    glPriv = &glxPriv->glPriv;
    rgbBits = modes->rgbBits;
    accumBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

#if defined(__GL_ALIGNED_BUFFERS)
    /* initialize pixel alignments (for more details see context.h) */
    glPriv->xAlignment = 1;
    glPriv->yAlignment = 1;
#endif

    glxPriv->swapBuffers = (GLboolean (*)(__GLXdrawablePrivate *))__glXNop;

    glPriv->yInverted = GL_FALSE;

    if (modes->doubleBufferMode) {
	if (modes->colorIndexMode) {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits,
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	    __glXInitMem(&glPriv->backBuffer, glPriv, modes->indexBits);
	} else {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits,
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	    __glXInitMem(&glPriv->backBuffer, glPriv, rgbBits);
	}
    } else {
	if (modes->colorIndexMode) {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits, 
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	} else {
	    __glXInitPix(&glPriv->frontBuffer, glPriv, rgbBits,
			 glxPriv->drawId, glxPriv->pGlxPixmap);
	}
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (modes->colorIndexMode) {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, modes->indexBits);
	    } else {
		__glXInitMem(&glPriv->auxBuffer[i], glPriv, rgbBits);
	    }
	}
    }
#endif

    if (modes->haveAccumBuffer) {
	__glXInitMem(&glPriv->accumBuffer, glPriv, accumBits);
    }
    if (modes->haveDepthBuffer) {
	__glXInitMem(&glPriv->depthBuffer, glPriv, modes->depthBits);
    }
    if (modes->haveStencilBuffer) {
	__glXInitMem(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }
}


#define __GLX_SET_ACCEL_BUFFER_MASK(bm) \
    if (status == GL_FALSE) return GL_FALSE; \
    if (status == GL_TRUE) accelBufferMask |= bm; \
    /* for __GL_BUFFER_FALLBACK don't do anything */

GLboolean
__glXResizeBuffers(__GLdrawablePrivate *glPriv,
		   GLint x, GLint y, GLuint width, GLuint height)
{
    __GLcontextModes *modes;
    __GLdrawableRegion *glRegion;
    GLboolean status;
    GLuint accelBufferMask;

    modes = glPriv->modes;
    accelBufferMask = 0;

    status = (*glPriv->frontBuffer.resize)(&glPriv->frontBuffer,
					   x, y, width, height, glPriv,
					   __GL_FRONT_BUFFER_MASK);
    __GLX_SET_ACCEL_BUFFER_MASK(__GL_FRONT_BUFFER_MASK);

    if (modes->doubleBufferMode) {
	status = (*glPriv->backBuffer.resize)(&glPriv->backBuffer,
					      x, y, width, height, glPriv,
					      __GL_BACK_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_BACK_BUFFER_MASK);
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    status = (*glPriv->auxBuffers[i].resize)(&glPriv->auxBuffer[i],
						     x, y, width, height, 
						     glPriv,
						     __GL_AUX_BUFFER_MASK(i));
	    __GLX_SET_ACCEL_BUFFER_MASK(__GL_AUX_BUFFER_MASK(i));
	}
    }
#endif

    if (modes->haveAccumBuffer) {
	status = (*glPriv->accumBuffer.resize)(&glPriv->accumBuffer,
					       x, y, width, height, glPriv,
					       __GL_ACCUM_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_ACCUM_BUFFER_MASK);
    }

    if (modes->haveDepthBuffer) {
	status = (*glPriv->depthBuffer.resize)(&glPriv->depthBuffer,
					       x, y, width, height, glPriv,
					       __GL_DEPTH_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_DEPTH_BUFFER_MASK);
    }

    if (modes->haveStencilBuffer) {
	status = (*glPriv->stencilBuffer.resize)(&glPriv->stencilBuffer,
						 x, y, width, height, glPriv,
						 __GL_STENCIL_BUFFER_MASK);
	__GLX_SET_ACCEL_BUFFER_MASK(__GL_STENCIL_BUFFER_MASK);
    }

    glPriv->accelBufferMask = accelBufferMask;

    /* finally, update the ownership region */
    glRegion = &glPriv->ownershipRegion;
    glRegion->numRects = 1;
    glRegion->rects[0].x0 = 0;
    glRegion->rects[0].y0 = 0;
    glRegion->rects[0].x1 = width;
    glRegion->rects[0].y1 = height;

    return GL_TRUE;
}

void
__glXFreeBuffers(__GLXdrawablePrivate *glxPriv)
{
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;
    __GLcontextModes *modes = glPriv->modes;

    if (glPriv->frontBuffer.free) {
	(*glPriv->frontBuffer.free)(&glPriv->frontBuffer, glPriv);
    }
    if (glPriv->backBuffer.free) {
	(*glPriv->backBuffer.free)(&glPriv->backBuffer, glPriv);
    }

#if defined(__GL_MAX_AUXBUFFERS) && (__GL_MAX_AUXBUFFERS > 0)
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (glPriv->auxBuffer[i].free) {
		(*glPriv->auxBuffer[i].free)(&glPriv->auxBuffer[i], glPriv);
	    }
	}
    }
#endif

    if (glPriv->accumBuffer.free) {
	(*glPriv->accumBuffer.free)(&glPriv->accumBuffer, glPriv);
    }

    if (glPriv->depthBuffer.free) {
	(*glPriv->depthBuffer.free)(&glPriv->depthBuffer, glPriv);
    }

    if (glPriv->stencilBuffer.free) {
	(*glPriv->stencilBuffer.free)(&glPriv->stencilBuffer, glPriv);
    }
}
