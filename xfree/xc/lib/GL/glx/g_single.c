/* $XFree86: xc/lib/GL/glx/g_single.c,v 1.2 1999/06/14 07:23:36 dawes Exp $ */
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
*/

#include "packsingle.h"

void glNewList(GLuint list, GLenum mode)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_NewList,8);
	__GLX_SINGLE_PUT_LONG(0,list);
	__GLX_SINGLE_PUT_LONG(4,mode);
	__GLX_SINGLE_END();
}

void glEndList(void)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_EndList,0);
	__GLX_SINGLE_END();
}

void glDeleteLists(GLuint list, GLsizei range)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_DeleteLists,8);
	__GLX_SINGLE_PUT_LONG(0,list);
	__GLX_SINGLE_PUT_LONG(4,range);
	__GLX_SINGLE_END();
}

GLuint glGenLists(GLsizei range)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	GLuint    retval = 0;
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GenLists,4);
	__GLX_SINGLE_PUT_LONG(0,range);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_RETVAL(retval, GLuint);
	__GLX_SINGLE_END();
	return retval;
}

void glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetLightfv,8);
	__GLX_SINGLE_PUT_LONG(0,light);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(params);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetLightiv,8);
	__GLX_SINGLE_PUT_LONG(0,light);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(params);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetMapdv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,query);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_DOUBLE(v);
	} else {
	    __GLX_SINGLE_GET_DOUBLE_ARRAY(v,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetMapfv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,query);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(v);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(v,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetMapiv(GLenum target, GLenum query, GLint *v)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetMapiv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,query);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(v);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(v,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetMaterialfv,8);
	__GLX_SINGLE_PUT_LONG(0,face);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(params);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetMaterialiv,8);
	__GLX_SINGLE_PUT_LONG(0,face);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(params);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetPixelMapfv(GLenum map, GLfloat *values)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetPixelMapfv,4);
	__GLX_SINGLE_PUT_LONG(0,map);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(values);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(values,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetPixelMapuiv(GLenum map, GLuint *values)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetPixelMapuiv,4);
	__GLX_SINGLE_PUT_LONG(0,map);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(values);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(values,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetPixelMapusv(GLenum map, GLushort *values)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetPixelMapusv,4);
	__GLX_SINGLE_PUT_LONG(0,map);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_SHORT(values);
	} else {
	    __GLX_SINGLE_GET_SHORT_ARRAY(values,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexEnvfv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(params);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexEnviv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(params);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexGendv,8);
	__GLX_SINGLE_PUT_LONG(0,coord);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_DOUBLE(params);
	} else {
	    __GLX_SINGLE_GET_DOUBLE_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexGenfv,8);
	__GLX_SINGLE_PUT_LONG(0,coord);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(params);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexGeniv,8);
	__GLX_SINGLE_PUT_LONG(0,coord);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(params);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexParameterfv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(params);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexParameteriv,8);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(params);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexLevelParameterfv,12);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,level);
	__GLX_SINGLE_PUT_LONG(8,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_FLOAT(params);
	} else {
	    __GLX_SINGLE_GET_FLOAT_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_GetTexLevelParameteriv,12);
	__GLX_SINGLE_PUT_LONG(0,target);
	__GLX_SINGLE_PUT_LONG(4,level);
	__GLX_SINGLE_PUT_LONG(8,pname);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_SIZE(compsize);
	if (compsize == 1) {
	    __GLX_SINGLE_GET_LONG(params);
	} else {
	    __GLX_SINGLE_GET_LONG_ARRAY(params,compsize);
	}
	__GLX_SINGLE_END();
}

GLboolean glIsList(GLuint list)
{
	__GLX_SINGLE_DECLARE_VARIABLES();
	GLboolean    retval = 0;
	xGLXSingleReply reply;
	__GLX_SINGLE_LOAD_VARIABLES();
	__GLX_SINGLE_BEGIN(X_GLsop_IsList,4);
	__GLX_SINGLE_PUT_LONG(0,list);
	__GLX_SINGLE_READ_XREPLY();
	__GLX_SINGLE_GET_RETVAL(retval, GLboolean);
	__GLX_SINGLE_END();
	return retval;
}

