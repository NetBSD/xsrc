#ifndef _size_h_
#define _size_h_

/* $XFree86: xc/lib/GL/glx/size.h,v 1.2 1999/06/14 07:23:40 dawes Exp $ */
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
** These are _size functions that are needed to pack the arguments
** into the protocol
*/
extern GLint __glBitmap_size(GLsizei w, GLsizei h);
extern GLint __glCallLists_size(GLsizei n, GLenum type);
extern GLint __glDrawPixels_size(GLenum format, GLenum type, GLsizei w,GLsizei h);
extern GLint __glFogfv_size(GLenum pname);
extern GLint __glFogiv_size(GLenum pname);
extern GLint __glLightModelfv_size(GLenum pname);
extern GLint __glLightModeliv_size(GLenum pname);
extern GLint __glLightfv_size(GLenum pname);
extern GLint __glLightiv_size(GLenum pname);
extern GLint __glMaterialfv_size(GLenum pname);
extern GLint __glMaterialiv_size(GLenum pname);
extern GLint __glTexEnvfv_size(GLenum e);
extern GLint __glTexEnviv_size(GLenum e);
extern GLint __glTexGendv_size(GLenum e);
extern GLint __glTexGenfv_size(GLenum e);
extern GLint __glTexGeniv_size(GLenum pname);
extern GLint __glTexImage1D_size(GLenum format, GLenum type, GLsizei w);
extern GLint __glTexImage2D_size(GLenum format, GLenum type,
				 GLsizei w, GLsizei h);
extern GLint __glTexParameterfv_size(GLenum e);
extern GLint __glTexParameteriv_size(GLenum e);

#endif /* _size_h_ */
