/* $XFree86: xc/lib/GL/glx/g_vendpriv.c,v 1.2 1999/06/14 07:23:36 dawes Exp $ */
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

#include "packvendpriv.h"

GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
	__GLX_VENDPRIV_DECLARE_VARIABLES();
	GLboolean    retval = 0;
	xGLXVendorPrivReply reply;
	__GLX_VENDPRIV_LOAD_VARIABLES();
	if (n < 0) return retval;
	cmdlen = 4+n*4;
	__GLX_VENDPRIV_BEGIN(X_GLXVendorPrivateWithReply,X_GLvop_AreTexturesResident,cmdlen);
	__GLX_VENDPRIV_PUT_LONG(0,n);
	__GLX_PUT_LONG_ARRAY(4,textures,n);
	__GLX_VENDPRIV_READ_XREPLY();
	__GLX_VENDPRIV_GET_RETVAL(retval, GLboolean);
	__GLX_VENDPRIV_GET_CHAR_ARRAY(residences,n);
	__GLX_VENDPRIV_END();
	return retval;
}

void glDeleteTextures(GLsizei n, const GLuint *textures)
{
	__GLX_VENDPRIV_DECLARE_VARIABLES();
	__GLX_VENDPRIV_LOAD_VARIABLES();
	if (n < 0) return;
	cmdlen = 4+n*4;
	__GLX_VENDPRIV_BEGIN(X_GLXVendorPrivate,X_GLvop_DeleteTextures,cmdlen);
	__GLX_VENDPRIV_PUT_LONG(0,n);
	__GLX_PUT_LONG_ARRAY(4,textures,n);
	__GLX_VENDPRIV_END();
}

void glGenTextures(GLsizei n, GLuint *textures)
{
	__GLX_VENDPRIV_DECLARE_VARIABLES();
	xGLXVendorPrivReply reply;
	__GLX_VENDPRIV_LOAD_VARIABLES();
	__GLX_VENDPRIV_BEGIN(X_GLXVendorPrivateWithReply,X_GLvop_GenTextures,4);
	__GLX_VENDPRIV_PUT_LONG(0,n);
	__GLX_VENDPRIV_READ_XREPLY();
	__GLX_VENDPRIV_GET_LONG_ARRAY(textures,n);
	__GLX_VENDPRIV_END();
}

GLboolean glIsTexture(GLuint texture)
{
	__GLX_VENDPRIV_DECLARE_VARIABLES();
	GLboolean    retval = 0;
	xGLXVendorPrivReply reply;
	__GLX_VENDPRIV_LOAD_VARIABLES();
	__GLX_VENDPRIV_BEGIN(X_GLXVendorPrivateWithReply,X_GLvop_IsTexture,4);
	__GLX_VENDPRIV_PUT_LONG(0,texture);
	__GLX_VENDPRIV_READ_XREPLY();
	__GLX_VENDPRIV_GET_RETVAL(retval, GLboolean);
	__GLX_VENDPRIV_END();
	return retval;
}

