#ifndef _singlesize_h_
#define _singlesize_h_

/* $XFree86: xc/programs/Xserver/GL/glx/singlesize.h,v 1.2 1999/06/14 07:31:36 dawes Exp $ */
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

extern GLint __glReadPixels_size(GLenum format, GLenum type,
				 GLint width, GLint height);
extern GLint __glGetTexEnvfv_size(GLenum pname);
extern GLint __glGetTexEnviv_size(GLenum pname);
extern GLint __glGetTexGenfv_size(GLenum pname);
extern GLint __glGetTexGendv_size(GLenum pname);
extern GLint __glGetTexGeniv_size(GLenum pname);
extern GLint __glGetTexParameterfv_size(GLenum pname);
extern GLint __glGetTexParameteriv_size(GLenum pname);
extern GLint __glGetLightfv_size(GLenum pname);
extern GLint __glGetLightiv_size(GLenum pname);
extern GLint __glGetMap_size(GLenum pname, GLenum query);
extern GLint __glGetMapdv_size(GLenum target, GLenum query);
extern GLint __glGetMapfv_size(GLenum target, GLenum query);
extern GLint __glGetMapiv_size(GLenum target, GLenum query);
extern GLint __glGetMaterialfv_size(GLenum pname);
extern GLint __glGetMaterialiv_size(GLenum pname);
extern GLint __glGetPixelMap_size(GLenum map);
extern GLint __glGetPixelMapfv_size(GLenum map);
extern GLint __glGetPixelMapuiv_size(GLenum map);
extern GLint __glGetPixelMapusv_size(GLenum map);
extern GLint __glGet_size(GLenum sq);
extern GLint __glGetDoublev_size(GLenum sq);
extern GLint __glGetFloatv_size(GLenum sq);
extern GLint __glGetIntegerv_size(GLenum sq);
extern GLint __glGetBooleanv_size(GLenum sq);
extern GLint __glGetTexLevelParameterfv_size(GLenum pname);
extern GLint __glGetTexLevelParameteriv_size(GLenum pname);
extern GLint __glGetTexImage_size(GLenum target, GLint level, GLenum format,
				  GLenum type, GLint width, GLint height);

#endif /* _singlesize_h_ */

