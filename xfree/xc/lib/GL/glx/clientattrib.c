/* $XFree86: xc/lib/GL/glx/clientattrib.c,v 1.4 2000/02/18 16:23:09 dawes Exp $ */
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

#include <assert.h>
#define NEED_GL_FUNCS_WRAPPED
#include "glxclient.h"

/*****************************************************************************/

void glEnableClientState(GLenum array)
{
    __GLXcontext *gc = __glXGetCurrentContext();

    switch (array) {
	case GL_COLOR_ARRAY:
	    gc->state.vertArray.colorEnable = GL_TRUE;
	    break;
	case GL_EDGE_FLAG_ARRAY:
	    gc->state.vertArray.edgeFlagEnable = GL_TRUE;
	    break;
	case GL_INDEX_ARRAY:
	    gc->state.vertArray.indexEnable = GL_TRUE;
	    break;
	case GL_NORMAL_ARRAY:
	    gc->state.vertArray.normalEnable = GL_TRUE;
	    break;
	case GL_TEXTURE_COORD_ARRAY:
	    gc->state.vertArray.texCoordEnable = GL_TRUE;
	    break;
	case GL_VERTEX_ARRAY:
	    gc->state.vertArray.vertexEnable = GL_TRUE;
	    break;
	default:
	    __glXSetError(gc, GL_INVALID_ENUM);
    }
}

void glDisableClientState(GLenum array)
{
    __GLXcontext *gc = __glXGetCurrentContext();

    switch (array) {
	case GL_COLOR_ARRAY:
	    gc->state.vertArray.colorEnable = GL_FALSE;
	    break;
	case GL_EDGE_FLAG_ARRAY:
	    gc->state.vertArray.edgeFlagEnable = GL_FALSE;
	    break;
	case GL_INDEX_ARRAY:
	    gc->state.vertArray.indexEnable = GL_FALSE;
	    break;
	case GL_NORMAL_ARRAY:
	    gc->state.vertArray.normalEnable = GL_FALSE;
	    break;
	case GL_TEXTURE_COORD_ARRAY:
	    gc->state.vertArray.texCoordEnable = GL_FALSE;
	    break;
	case GL_VERTEX_ARRAY:
	    gc->state.vertArray.vertexEnable = GL_FALSE;
	    break;
	default:
	    __glXSetError(gc, GL_INVALID_ENUM);
    }
}

/************************************************************************/

void glPushClientAttrib(GLuint mask)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXattribute **spp = gc->attributes.stackPointer, *sp;

    if (spp < &gc->attributes.stack[__GL_CLIENT_ATTRIB_STACK_DEPTH]) {
	if (!(sp = *spp)) {
	    sp = (__GLXattribute *)Xmalloc(sizeof(__GLXattribute));
	    *spp = sp;
	}
	sp->mask = mask;
	gc->attributes.stackPointer = spp + 1;
	if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
	    sp->storePack = gc->state.storePack;
	    sp->storeUnpack = gc->state.storeUnpack;
	}
	if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
	    sp->vertArray = gc->state.vertArray;
	}
    } else {
	__glXSetError(gc, GL_STACK_OVERFLOW);
	return;
    }
}

void glPopClientAttrib(void)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXattribute **spp = gc->attributes.stackPointer, *sp;
    GLuint mask;

    if (spp > &gc->attributes.stack[0]) {
	--spp;
	sp = *spp;
	assert(sp != 0);
	mask = sp->mask;
	gc->attributes.stackPointer = spp;

	if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
	    gc->state.storePack = sp->storePack;
	    gc->state.storeUnpack = sp->storeUnpack;
	}
	if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
	    gc->state.vertArray = sp->vertArray;
	}

	sp->mask = 0;
    } else {
	__glXSetError(gc, GL_STACK_UNDERFLOW);
	return;
    }
}

void __glFreeAttributeState(__GLXcontext *gc)
{
    __GLXattribute *sp, **spp;

    for (spp = &gc->attributes.stack[0];
         spp < &gc->attributes.stack[__GL_CLIENT_ATTRIB_STACK_DEPTH];
	 spp++) {
	sp = *spp;
        if (sp) {
            XFree((char *)sp);
        } else {
            break;
	}
    }
}


