/* $XFree86: xc/lib/GL/glx/singlepix.c,v 1.2 1999/06/14 07:23:39 dawes Exp $ */
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

#include "packsingle.h"

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
		  GLenum format, GLenum type, GLvoid *pixels)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXReadPixelsReply reply;
    GLubyte *buf;

    if (!dpy) return;
    __GLX_SINGLE_LOAD_VARIABLES();

    /* Send request */
    __GLX_SINGLE_BEGIN(X_GLsop_ReadPixels,__GLX_PAD(26));
    __GLX_SINGLE_PUT_LONG(0,x);
    __GLX_SINGLE_PUT_LONG(4,y);
    __GLX_SINGLE_PUT_LONG(8,width);
    __GLX_SINGLE_PUT_LONG(12,height);
    __GLX_SINGLE_PUT_LONG(16,format);
    __GLX_SINGLE_PUT_LONG(20,type);
    __GLX_SINGLE_PUT_CHAR(24,gc->state.storePack.swapEndian);
    __GLX_SINGLE_PUT_CHAR(25,GL_FALSE);
    __GLX_SINGLE_READ_XREPLY();
    compsize = reply.length << 2;

    if (compsize != 0) {
	/* Allocate a holding buffer to transform the data from */
	buf = (GLubyte*) Xmalloc(compsize);
	if (!buf) {
	    /* Throw data away */
	    _XEatData(dpy, compsize);
	    __glXSetError(gc, GL_OUT_OF_MEMORY);
	} else {
	    /*
	    ** Fetch data into holding buffer.  Apply pixel store pack modes
	    ** to put data back into client memory
	    */
	    __GLX_SINGLE_GET_CHAR_ARRAY(buf,compsize);
	    __glEmptyImage(gc, width, height, format, type, buf, pixels);
	    Xfree((char*) buf);
	}
    } else {
	/*
	** GL error occurred; don't modify user's buffer.
	*/
    }
    __GLX_SINGLE_END();
}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type,
		   GLvoid *texels)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXGetTexImageReply reply;
    GLubyte *buf;

    if (!dpy) return;
    __GLX_SINGLE_LOAD_VARIABLES();

    /* Send request */
    __GLX_SINGLE_BEGIN(X_GLsop_GetTexImage,__GLX_PAD(17));
    __GLX_SINGLE_PUT_LONG(0,target);
    __GLX_SINGLE_PUT_LONG(4,level);
    __GLX_SINGLE_PUT_LONG(8,format);
    __GLX_SINGLE_PUT_LONG(12,type);
    __GLX_SINGLE_PUT_CHAR(16,gc->state.storePack.swapEndian);
    __GLX_SINGLE_READ_XREPLY();
    compsize = reply.length << 2;

    if (compsize != 0) {
	/* Allocate a holding buffer to transform the data from */
	buf = (GLubyte*) Xmalloc(compsize);
	if (!buf) {
	    /* Throw data away */
	    _XEatData(dpy, compsize);
	    __glXSetError(gc, GL_OUT_OF_MEMORY);
	} else {
	    GLint width, height;

	    /*
	    ** Fetch data into holding buffer.  Apply pixel store pack modes
	    ** to put data back into client memory
	    */
	    width = reply.width;
	    height = reply.height;
	    __GLX_SINGLE_GET_CHAR_ARRAY(buf,compsize);
	    __glEmptyImage(gc, width, height, format, type, buf, texels);
	    Xfree((char*) buf);
	}
    } else {
	/*
	** GL error occured, don't modify user's buffer.
	*/
    }
    __GLX_SINGLE_END();
}

void glGetPolygonStipple(GLubyte *mask)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;
    GLubyte buf[128];

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetPolygonStipple,__GLX_PAD(1));
    __GLX_SINGLE_PUT_CHAR(0,GL_FALSE);
    __GLX_SINGLE_READ_XREPLY();
    if (reply.length == 32) {
	__GLX_SINGLE_GET_CHAR_ARRAY(buf,128);
	__glEmptyImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP, buf, mask);
    }
    __GLX_SINGLE_END();
}
