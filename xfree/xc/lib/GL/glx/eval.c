/* $XFree86: xc/lib/GL/glx/eval.c,v 1.2 1999/06/14 07:23:35 dawes Exp $ */
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

#include "packrender.h"

/*
** Routines to pack evaluator maps into the transport buffer.  Maps are
** allowed to have extra arbitrary data, so these routines extract just
** the information that the GL needs.
*/

void __glFillMap1f(GLint k, GLint order, GLint stride, 
		   const GLfloat *points, GLubyte *pc)
{
    if (stride == k) {
	/* Just copy the data */
	__GLX_PUT_FLOAT_ARRAY(0, points, order * k);
    } else {
	GLint i;

	for (i = 0; i < order; i++) {
	    __GLX_PUT_FLOAT_ARRAY(0, points, k);
	    points += stride;
	    pc += k * __GLX_SIZE_FLOAT32;
	}
    }
}

void __glFillMap1d(GLint k, GLint order, GLint stride, 
		   const GLdouble *points, GLubyte *pc)
{
    if (stride == k) {
	/* Just copy the data */
	__GLX_PUT_DOUBLE_ARRAY(0, points, order * k);
    } else {
	GLint i;
	for (i = 0; i < order; i++) {
            __GLX_PUT_DOUBLE_ARRAY(0, points, k);
	    points += stride;
	    pc += k * __GLX_SIZE_FLOAT64;
	}
    }
}

void __glFillMap2f(GLint k, GLint majorOrder, GLint minorOrder, 
		   GLint majorStride, GLint minorStride,
		   const GLfloat *points, GLfloat *data)
{
    GLint i, j, x;

    if ((minorStride == k) && (majorStride == minorOrder*k)) {
	/* Just copy the data */
	__GLX_MEM_COPY(data, points, majorOrder * majorStride *
		       __GLX_SIZE_FLOAT32);
	return;
    }
    for (i = 0; i < majorOrder; i++) {
	for (j = 0; j < minorOrder; j++) {
	    for (x = 0; x < k; x++) {
		data[x] = points[x];
	    }
	    points += minorStride;
	    data += k;
	}
	points += majorStride - minorStride * minorOrder;
    }
}

void __glFillMap2d(GLint k, GLint majorOrder, GLint minorOrder, 
		   GLint majorStride, GLint minorStride,
		   const GLdouble *points, GLdouble *data)
{
    int i,j,x;

    if ((minorStride == k) && (majorStride == minorOrder*k)) {
	/* Just copy the data */
	__GLX_MEM_COPY(data, points, majorOrder * majorStride *
		       __GLX_SIZE_FLOAT64);
	return;
    }

#ifdef __GLX_ALIGN64
    x = k * __GLX_SIZE_FLOAT64;
#endif
    for (i = 0; i<majorOrder; i++) {
	for (j = 0; j<minorOrder; j++) {
#ifdef __GLX_ALIGN64
	    __GLX_MEM_COPY(data, points, x);
#else
	    for (x = 0; x<k; x++) {
		data[x] = points[x];
	    }
#endif
	    points += minorStride;
	    data += k;
	}
	points += majorStride - minorStride * minorOrder;
    }
}

GLint __glEvalComputeK(GLenum target)
{
    switch(target) {
      case GL_MAP1_VERTEX_4:
      case GL_MAP1_COLOR_4:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_4:
      case GL_MAP2_COLOR_4:
      case GL_MAP2_TEXTURE_COORD_4:
	return 4;
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_NORMAL:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_NORMAL:
	return 3;
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_2:
	return 2;
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP1_INDEX:
      case GL_MAP2_INDEX:
	return 1;
      default:
	return 0;
    }
}
