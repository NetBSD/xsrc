#ifndef _impsize_h_
#define _impsize_h_

/* $XFree86: xc/programs/Xserver/GL/glx/impsize.h,v 1.2 1999/06/14 07:31:33 dawes Exp $ */
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
** These are defined in libsampleGL.a. They are not technically part of
** the defined interface between libdixGL.a and libsampleGL.a (that interface
** being the functions in the __glXScreenInfo structure, plus the OpenGL API
** itself), but we thought it was better to call these routines than to
** replicate the code in here.
*/
extern int __glCallLists_size(GLsizei n, GLenum type);
extern int __glDrawPixels_size(GLenum format, GLenum type, GLsizei w,GLsizei h);
extern int __glFogfv_size(GLenum pname);
extern int __glFogiv_size(GLenum pname);
extern int __glLightModelfv_size(GLenum pname);
extern int __glLightModeliv_size(GLenum pname);
extern int __glLightfv_size(GLenum pname);
extern int __glLightiv_size(GLenum pname);
extern int __glMaterialfv_size(GLenum pname);
extern int __glMaterialiv_size(GLenum pname);
extern int __glTexEnvfv_size(GLenum e);
extern int __glTexEnviv_size(GLenum e);
extern int __glTexGendv_size(GLenum e);
extern int __glTexGenfv_size(GLenum e);
extern int __glTexGeniv_size(GLenum pname);
extern int __glTexParameterfv_size(GLenum e);
extern int __glTexParameteriv_size(GLenum e);

extern int __glEvalComputeK(GLenum target);

#endif /* _impsize_h_ */
