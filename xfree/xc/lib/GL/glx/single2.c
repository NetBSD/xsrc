/* $XFree86: xc/lib/GL/glx/single2.c,v 1.2 1999/06/14 07:23:39 dawes Exp $ */
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
#include "glxclient.h"

GLenum glGetError(void)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    GLuint retval = GL_NO_ERROR;
    xGLXGetErrorReply reply;

    if (gc->error) {
	/* Use internal error first */
	retval = gc->error;
	gc->error = GL_NO_ERROR;
	return retval;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetError,0);
    __GLX_SINGLE_READ_XREPLY();
    retval = reply.error;
    __GLX_SINGLE_END();

    return retval;
}

void glGetClipPlane(GLenum plane, GLdouble *equation)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;
    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetClipPlane,4);
    __GLX_SINGLE_PUT_LONG(0,plane);
    __GLX_SINGLE_READ_XREPLY();
    if (reply.length == 8) {
	__GLX_SINGLE_GET_DOUBLE_ARRAY(equation,4);
    }
    __GLX_SINGLE_END();
}

void glGetBooleanv(GLenum val, GLboolean *b)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetBooleanv,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *b = (GLboolean)gc->state.storePack.rowLength;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *b = (GLboolean)gc->state.storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *b = (GLboolean)gc->state.storePack.skipPixels;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *b = (GLboolean)gc->state.storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *b = (GLboolean)gc->state.storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *b = (GLboolean)gc->state.storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *b = (GLboolean)gc->state.storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *b = (GLboolean)gc->state.storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *b = (GLboolean)gc->state.storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *b = (GLboolean)gc->state.storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *b = (GLboolean)gc->state.storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *b = (GLboolean)gc->state.storeUnpack.lsbFirst;
	    break;
	  case GL_VERTEX_ARRAY:
	    *b = (GLboolean)gc->state.vertArray.vertexEnable;
	    break;
	  case GL_VERTEX_ARRAY_SIZE:
	    *b = (GLboolean)gc->state.vertArray.vertexSize;
	    break;
	  case GL_VERTEX_ARRAY_TYPE:
	    *b = (GLboolean)gc->state.vertArray.vertexType;
	    break;
	  case GL_VERTEX_ARRAY_STRIDE:
	    *b = (GLboolean)gc->state.vertArray.vertexStride;
	    break;
	  case GL_NORMAL_ARRAY:
	    *b = (GLboolean)gc->state.vertArray.normalEnable;
	    break;
	  case GL_NORMAL_ARRAY_TYPE:
	    *b = (GLboolean)gc->state.vertArray.normalType;
	    break;
	  case GL_NORMAL_ARRAY_STRIDE:
	    *b = (GLboolean)gc->state.vertArray.normalStride;
	    break;
	  case GL_COLOR_ARRAY:
	    *b = (GLboolean)gc->state.vertArray.colorEnable;
	    break;
	  case GL_COLOR_ARRAY_SIZE:
	    *b = (GLboolean)gc->state.vertArray.colorSize;
	    break;
	  case GL_COLOR_ARRAY_TYPE:
	    *b = (GLboolean)gc->state.vertArray.colorType;
	    break;
	  case GL_COLOR_ARRAY_STRIDE:
	    *b = (GLboolean)gc->state.vertArray.colorStride;
	    break;
	  case GL_INDEX_ARRAY:
	    *b = (GLboolean)gc->state.vertArray.indexEnable;
	    break;
	  case GL_INDEX_ARRAY_TYPE:
	    *b = (GLboolean)gc->state.vertArray.indexType;
	    break;
	  case GL_INDEX_ARRAY_STRIDE:
	    *b = (GLboolean)gc->state.vertArray.indexStride;
	    break;
	  case GL_TEXTURE_COORD_ARRAY:
	    *b = (GLboolean)gc->state.vertArray.texCoordEnable;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *b = (GLboolean)gc->state.vertArray.texCoordSize;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *b = (GLboolean)gc->state.vertArray.texCoordType;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *b = (GLboolean)gc->state.vertArray.texCoordStride;
	    break;
	  case GL_EDGE_FLAG_ARRAY:
	    *b = (GLboolean)gc->state.vertArray.edgeFlagEnable;
	    break;
	  case GL_EDGE_FLAG_ARRAY_STRIDE:
	    *b = (GLboolean)gc->state.vertArray.edgeFlagStride;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *b = (GLboolean)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  default:
	    /*
	    ** Not a local value, so use what we got from the server.
	    */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_CHAR(b);
	    } else {
		__GLX_SINGLE_GET_CHAR_ARRAY(b,compsize);
	    }
	}
    }
    __GLX_SINGLE_END();
}

void glGetDoublev(GLenum val, GLdouble *d)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetDoublev,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *d = (GLdouble)gc->state.storePack.rowLength;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *d = (GLdouble)gc->state.storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *d = (GLdouble)gc->state.storePack.skipPixels;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *d = (GLdouble)gc->state.storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *d = (GLdouble)gc->state.storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *d = (GLdouble)gc->state.storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *d = (GLdouble)gc->state.storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *d = (GLdouble)gc->state.storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *d = (GLdouble)gc->state.storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *d = (GLdouble)gc->state.storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *d = (GLdouble)gc->state.storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *d = (GLdouble)gc->state.storeUnpack.lsbFirst;
	    break;
	  case GL_VERTEX_ARRAY:
	    *d = (GLdouble)gc->state.vertArray.vertexEnable;
	    break;
	  case GL_VERTEX_ARRAY_SIZE:
	    *d = (GLdouble)gc->state.vertArray.vertexSize;
	    break;
	  case GL_VERTEX_ARRAY_TYPE:
	    *d = (GLdouble)gc->state.vertArray.vertexType;
	    break;
	  case GL_VERTEX_ARRAY_STRIDE:
	    *d = (GLdouble)gc->state.vertArray.vertexStride;
	    break;
	  case GL_NORMAL_ARRAY:
	    *d = (GLdouble)gc->state.vertArray.normalEnable;
	    break;
	  case GL_NORMAL_ARRAY_TYPE:
	    *d = (GLdouble)gc->state.vertArray.normalType;
	    break;
	  case GL_NORMAL_ARRAY_STRIDE:
	    *d = (GLdouble)gc->state.vertArray.normalStride;
	    break;
	  case GL_COLOR_ARRAY:
	    *d = (GLdouble)gc->state.vertArray.colorEnable;
	    break;
	  case GL_COLOR_ARRAY_SIZE:
	    *d = (GLdouble)gc->state.vertArray.colorSize;
	    break;
	  case GL_COLOR_ARRAY_TYPE:
	    *d = (GLdouble)gc->state.vertArray.colorType;
	    break;
	  case GL_COLOR_ARRAY_STRIDE:
	    *d = (GLdouble)gc->state.vertArray.colorStride;
	    break;
	  case GL_INDEX_ARRAY:
	    *d = (GLdouble)gc->state.vertArray.indexEnable;
	    break;
	  case GL_INDEX_ARRAY_TYPE:
	    *d = (GLdouble)gc->state.vertArray.indexType;
	    break;
	  case GL_INDEX_ARRAY_STRIDE:
	    *d = (GLdouble)gc->state.vertArray.indexStride;
	    break;
	  case GL_TEXTURE_COORD_ARRAY:
	    *d = (GLdouble)gc->state.vertArray.texCoordEnable;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *d = (GLdouble)gc->state.vertArray.texCoordSize;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *d = (GLdouble)gc->state.vertArray.texCoordType;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *d = (GLdouble)gc->state.vertArray.texCoordStride;
	    break;
	  case GL_EDGE_FLAG_ARRAY:
	    *d = (GLdouble)gc->state.vertArray.edgeFlagEnable;
	    break;
	  case GL_EDGE_FLAG_ARRAY_STRIDE:
	    *d = (GLdouble)gc->state.vertArray.edgeFlagStride;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *d = (GLdouble)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  default:
	    /*
	     ** Not a local value, so use what we got from the server.
	     */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_DOUBLE(d);
	    } else {
		__GLX_SINGLE_GET_DOUBLE_ARRAY(d,compsize);
	    }
	}
    }
    __GLX_SINGLE_END();
}

void glGetFloatv(GLenum val, GLfloat *f)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetFloatv,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);

    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *f = (GLfloat)gc->state.storePack.rowLength;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *f = (GLfloat)gc->state.storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *f = (GLfloat)gc->state.storePack.skipPixels;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *f = (GLfloat)gc->state.storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *f = (GLfloat)gc->state.storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *f = (GLfloat)gc->state.storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *f = (GLfloat)gc->state.storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *f = (GLfloat)gc->state.storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *f = (GLfloat)gc->state.storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *f = (GLfloat)gc->state.storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *f = (GLfloat)gc->state.storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *f = (GLfloat)gc->state.storeUnpack.lsbFirst;
	    break;
	  case GL_VERTEX_ARRAY:
	    *f = (GLfloat)gc->state.vertArray.vertexEnable;
	    break;
	  case GL_VERTEX_ARRAY_SIZE:
	    *f = (GLfloat)gc->state.vertArray.vertexSize;
	    break;
	  case GL_VERTEX_ARRAY_TYPE:
	    *f = (GLfloat)gc->state.vertArray.vertexType;
	    break;
	  case GL_VERTEX_ARRAY_STRIDE:
	    *f = (GLfloat)gc->state.vertArray.vertexStride;
	    break;
	  case GL_NORMAL_ARRAY:
	    *f = (GLfloat)gc->state.vertArray.normalEnable;
	    break;
	  case GL_NORMAL_ARRAY_TYPE:
	    *f = (GLfloat)gc->state.vertArray.normalType;
	    break;
	  case GL_NORMAL_ARRAY_STRIDE:
	    *f = (GLfloat)gc->state.vertArray.normalStride;
	    break;
	  case GL_COLOR_ARRAY:
	    *f = (GLfloat)gc->state.vertArray.colorEnable;
	    break;
	  case GL_COLOR_ARRAY_SIZE:
	    *f = (GLfloat)gc->state.vertArray.colorSize;
	    break;
	  case GL_COLOR_ARRAY_TYPE:
	    *f = (GLfloat)gc->state.vertArray.colorType;
	    break;
	  case GL_COLOR_ARRAY_STRIDE:
	    *f = (GLfloat)gc->state.vertArray.colorStride;
	    break;
	  case GL_INDEX_ARRAY:
	    *f = (GLfloat)gc->state.vertArray.indexEnable;
	    break;
	  case GL_INDEX_ARRAY_TYPE:
	    *f = (GLfloat)gc->state.vertArray.indexType;
	    break;
	  case GL_INDEX_ARRAY_STRIDE:
	    *f = (GLfloat)gc->state.vertArray.indexStride;
	    break;
	  case GL_TEXTURE_COORD_ARRAY:
	    *f = (GLfloat)gc->state.vertArray.texCoordEnable;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *f = (GLfloat)gc->state.vertArray.texCoordSize;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *f = (GLfloat)gc->state.vertArray.texCoordType;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *f = (GLfloat)gc->state.vertArray.texCoordStride;
	    break;
	  case GL_EDGE_FLAG_ARRAY:
	    *f = (GLfloat)gc->state.vertArray.edgeFlagEnable;
	    break;
	  case GL_EDGE_FLAG_ARRAY_STRIDE:
	    *f = (GLfloat)gc->state.vertArray.edgeFlagStride;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *f = (GLfloat)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  default:
	    /*
	    ** Not a local value, so use what we got from the server.
	    */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_FLOAT(f);
	    } else {
		__GLX_SINGLE_GET_FLOAT_ARRAY(f,compsize);
	    }
	}
    }
    __GLX_SINGLE_END();
}

void glGetIntegerv(GLenum val, GLint *i)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetIntegerv,4);
    __GLX_SINGLE_PUT_LONG(0,val);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);
    
    if (compsize == 0) {
	/*
	** Error occured; don't modify user's buffer.
	*/
    } else {
	/*
	** For all the queries listed here, we use the locally stored
	** values rather than the one returned by the server.  Note that
	** we still needed to send the request to the server in order to
	** find out whether it was legal to make a query (it's illegal,
	** for example, to call a query between glBegin() and glEnd()).
	*/
	switch (val) {
	  case GL_PACK_ROW_LENGTH:
	    *i = (GLint)gc->state.storePack.rowLength;
	    break;
	  case GL_PACK_SKIP_ROWS:
	    *i = (GLint)gc->state.storePack.skipRows;
	    break;
	  case GL_PACK_SKIP_PIXELS:
	    *i = (GLint)gc->state.storePack.skipPixels;
	    break;
	  case GL_PACK_ALIGNMENT:
	    *i = (GLint)gc->state.storePack.alignment;
	    break;
	  case GL_PACK_SWAP_BYTES:
	    *i = (GLint)gc->state.storePack.swapEndian;
	    break;
	  case GL_PACK_LSB_FIRST:
	    *i = (GLint)gc->state.storePack.lsbFirst;
	    break;
	  case GL_UNPACK_ROW_LENGTH:
	    *i = (GLint)gc->state.storeUnpack.rowLength;
	    break;
	  case GL_UNPACK_SKIP_ROWS:
	    *i = (GLint)gc->state.storeUnpack.skipRows;
	    break;
	  case GL_UNPACK_SKIP_PIXELS:
	    *i = (GLint)gc->state.storeUnpack.skipPixels;
	    break;
	  case GL_UNPACK_ALIGNMENT:
	    *i = (GLint)gc->state.storeUnpack.alignment;
	    break;
	  case GL_UNPACK_SWAP_BYTES:
	    *i = (GLint)gc->state.storeUnpack.swapEndian;
	    break;
	  case GL_UNPACK_LSB_FIRST:
	    *i = (GLint)gc->state.storeUnpack.lsbFirst;
	    break;
	  case GL_VERTEX_ARRAY:
	    *i = (GLint)gc->state.vertArray.vertexEnable;
	    break;
	  case GL_VERTEX_ARRAY_SIZE:
	    *i = (GLint)gc->state.vertArray.vertexSize;
	    break;
	  case GL_VERTEX_ARRAY_TYPE:
	    *i = (GLint)gc->state.vertArray.vertexType;
	    break;
	  case GL_VERTEX_ARRAY_STRIDE:
	    *i = (GLint)gc->state.vertArray.vertexStride;
	    break;
	  case GL_NORMAL_ARRAY:
	    *i = (GLint)gc->state.vertArray.normalEnable;
	    break;
	  case GL_NORMAL_ARRAY_TYPE:
	    *i = (GLint)gc->state.vertArray.normalType;
	    break;
	  case GL_NORMAL_ARRAY_STRIDE:
	    *i = (GLint)gc->state.vertArray.normalStride;
	    break;
	  case GL_COLOR_ARRAY:
	    *i = (GLint)gc->state.vertArray.colorEnable;
	    break;
	  case GL_COLOR_ARRAY_SIZE:
	    *i = (GLint)gc->state.vertArray.colorSize;
	    break;
	  case GL_COLOR_ARRAY_TYPE:
	    *i = (GLint)gc->state.vertArray.colorType;
	    break;
	  case GL_COLOR_ARRAY_STRIDE:
	    *i = (GLint)gc->state.vertArray.colorStride;
	    break;
	  case GL_INDEX_ARRAY:
	    *i = (GLint)gc->state.vertArray.indexEnable;
	    break;
	  case GL_INDEX_ARRAY_TYPE:
	    *i = (GLint)gc->state.vertArray.indexType;
	    break;
	  case GL_INDEX_ARRAY_STRIDE:
	    *i = (GLint)gc->state.vertArray.indexStride;
	    break;
	  case GL_TEXTURE_COORD_ARRAY:
	    *i = (GLint)gc->state.vertArray.texCoordEnable;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_SIZE:
	    *i = (GLint)gc->state.vertArray.texCoordSize;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_TYPE:
	    *i = (GLint)gc->state.vertArray.texCoordType;
	    break;
	  case GL_TEXTURE_COORD_ARRAY_STRIDE:
	    *i = (GLint)gc->state.vertArray.texCoordStride;
	    break;
	  case GL_EDGE_FLAG_ARRAY:
	    *i = (GLint)gc->state.vertArray.edgeFlagEnable;
	    break;
	  case GL_EDGE_FLAG_ARRAY_STRIDE:
	    *i = (GLint)gc->state.vertArray.edgeFlagStride;
	    break;
	  case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
	    *i = (GLint)__GL_CLIENT_ATTRIB_STACK_DEPTH;
	    break;
	  default:
	    /*
	    ** Not a local value, so use what we got from the server.
	    */
	    if (compsize == 1) {
		__GLX_SINGLE_GET_LONG(i);
	    } else {
		__GLX_SINGLE_GET_LONG_ARRAY(i,compsize);
	    }
	}
    }
    __GLX_SINGLE_END();
}

/*
** Send all pending commands to server.
*/
void glFlush(void)
{
    __GLX_SINGLE_DECLARE_VARIABLES();

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_Flush,0);
    __GLX_SINGLE_END();

    /* And finally flush the X protocol data */
    XFlush(dpy);
}

void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    __GLX_SINGLE_DECLARE_VARIABLES();

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_FeedbackBuffer,8);
    __GLX_SINGLE_PUT_LONG(0,size);
    __GLX_SINGLE_PUT_LONG(4,type);
    __GLX_SINGLE_END();

    gc->feedbackBuf = buffer;
}

void glSelectBuffer(GLsizei numnames, GLuint *buffer)
{
    __GLX_SINGLE_DECLARE_VARIABLES();

    if (!dpy) return;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_SelectBuffer,4);
    __GLX_SINGLE_PUT_LONG(0,numnames);
    __GLX_SINGLE_END();

    gc->selectBuf = buffer;
}

GLint glRenderMode(GLenum mode)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    GLint retval;
    xGLXRenderModeReply reply;

    if (!dpy) return -1;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_RenderMode,4);
    __GLX_SINGLE_PUT_LONG(0,mode);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_RETVAL(retval,GLint);

    if (reply.newMode != mode) {
	/*
	** Switch to new mode did not take effect, therefore an error
	** occured.  When an error happens the server won't send us any
	** other data.
	*/
    } else {
	/* Read the feedback or selection data */
	if (gc->renderMode == GL_FEEDBACK) {
	    __GLX_SINGLE_GET_SIZE(compsize);
	    __GLX_SINGLE_GET_FLOAT_ARRAY(gc->feedbackBuf, compsize);
	} else
	if (gc->renderMode == GL_SELECT) {
	    __GLX_SINGLE_GET_SIZE(compsize);
	    __GLX_SINGLE_GET_LONG_ARRAY(gc->selectBuf, compsize);
	}
	gc->renderMode = mode;
    }
    __GLX_SINGLE_END();

    return retval;
}

void glFinish(void)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_Finish,0);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_END();
}

const GLubyte *glGetString(GLenum name)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;
    GLubyte *s = NULL;

    if (!dpy) return 0;

    /*
    ** Return the cached copy if the string has already been fetched
    */
    switch(name) {
      case GL_VENDOR:
	if (gc->vendor) return gc->vendor;
	break;
      case GL_RENDERER:
	if (gc->renderer) return gc->renderer;
	break;
      case GL_VERSION:
	if (gc->version) return gc->version;
	break;
      case GL_EXTENSIONS:
	if (gc->extensions) return gc->extensions;
	break;
      default:
	__glXSetError(gc, GL_INVALID_ENUM);
	return 0;
    }

    /*
    ** Get requested string from server
    */
    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_GetString,4);
    __GLX_SINGLE_PUT_LONG(0,name);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_SIZE(compsize);
    s = (GLubyte*) Xmalloc(compsize);
    if (!s) {
	/* Throw data on the floor */
	_XEatData(dpy, compsize);
	__glXSetError(gc, GL_OUT_OF_MEMORY);
    } else {
	__GLX_SINGLE_GET_CHAR_ARRAY(s,compsize);

	/*
	** Update local cache
	*/
	switch(name) {
	  case GL_VENDOR:
	    gc->vendor = s;
	    break;
	  case GL_RENDERER:
	    gc->renderer = s;
	    break;
	  case GL_VERSION:
	    gc->version = s;
	    break;
	  case GL_EXTENSIONS:
	    gc->extensions = s;
	    break;
	}
    }
    __GLX_SINGLE_END();
    return s;
}

GLboolean glIsEnabled(GLenum cap)
{
    __GLX_SINGLE_DECLARE_VARIABLES();
    xGLXSingleReply reply;
    GLboolean retval = 0;

    if (!dpy) return 0;

    switch(cap) {
      case GL_VERTEX_ARRAY:
	  return gc->state.vertArray.vertexEnable;
      case GL_NORMAL_ARRAY:
	  return gc->state.vertArray.normalEnable;
      case GL_COLOR_ARRAY:
	  return gc->state.vertArray.colorEnable;
      case GL_INDEX_ARRAY:
	  return gc->state.vertArray.indexEnable;
      case GL_TEXTURE_COORD_ARRAY:
	  return gc->state.vertArray.texCoordEnable;
      case GL_EDGE_FLAG_ARRAY:
	  return gc->state.vertArray.edgeFlagEnable;
    }

    __GLX_SINGLE_LOAD_VARIABLES();
    __GLX_SINGLE_BEGIN(X_GLsop_IsEnabled,4);
    __GLX_SINGLE_PUT_LONG(0,cap);
    __GLX_SINGLE_READ_XREPLY();
    __GLX_SINGLE_GET_RETVAL(retval, GLboolean);
    __GLX_SINGLE_END();
    return retval;
}

void glGetPointerv(GLenum pname, void **params)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;

    if (!dpy) return;

    switch(pname) {
      case GL_VERTEX_ARRAY_POINTER:
	  *params = (void *)gc->state.vertArray.vertexPtr;
	  return;
      case GL_NORMAL_ARRAY_POINTER:
	  *params = (void *)gc->state.vertArray.normalPtr;
	  return;
      case GL_COLOR_ARRAY_POINTER:
	  *params = (void *)gc->state.vertArray.colorPtr;
	  return;
      case GL_INDEX_ARRAY_POINTER:
	  *params = (void *)gc->state.vertArray.indexPtr;
	  return;
      case GL_TEXTURE_COORD_ARRAY_POINTER:
	  *params = (void *)gc->state.vertArray.texCoordPtr;
	  return;
      case GL_EDGE_FLAG_ARRAY_POINTER:
	  *params = (void *)gc->state.vertArray.edgeFlagPtr;
        return;
      case GL_FEEDBACK_BUFFER_POINTER:
        *params = (void *)gc->feedbackBuf;
        return;
      case GL_SELECTION_BUFFER_POINTER:
        *params = (void *)gc->selectBuf;
        return;
      default:
	__glXSetError(gc, GL_INVALID_ENUM);
        return;
    }
}

