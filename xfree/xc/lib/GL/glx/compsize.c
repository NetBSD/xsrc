/* $XFree86: xc/lib/GL/glx/compsize.c,v 1.2 1999/06/14 07:23:34 dawes Exp $ */
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

#include <GL/gl.h>

#include "size.h"

GLint __glFogiv_size(GLenum pname)
{
    switch (pname) {
      case GL_FOG_COLOR:	return 4;
      case GL_FOG_DENSITY:	return 1;
      case GL_FOG_END:		return 1;
      case GL_FOG_MODE:		return 1;
      case GL_FOG_INDEX:	return 1;
      case GL_FOG_START:	return 1;
      default:
	return 0;
    }
}

GLint __glFogfv_size(GLenum pname)
{
    return __glFogiv_size(pname);
}

GLint __glCallLists_size(GLsizei n, GLenum type)
{
    GLint size;

    if (n < 0) return 0;
    switch (type) {
      case GL_BYTE:		size = 1; break;
      case GL_UNSIGNED_BYTE:	size = 1; break;
      case GL_SHORT:		size = 2; break;
      case GL_UNSIGNED_SHORT:	size = 2; break;
      case GL_INT:		size = 4; break;
      case GL_UNSIGNED_INT:	size = 4; break;
      case GL_FLOAT:		size = 4; break;
      case GL_2_BYTES:		size = 2; break;
      case GL_3_BYTES:		size = 3; break;
      case GL_4_BYTES:		size = 4; break;
      default:
	return 0;
    }
    return n * size;
}

GLint __glDrawPixels_size(GLenum format, GLenum type, GLsizei w, GLsizei h)
{
    GLint elements, esize;
    
    switch (format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
      case GL_DEPTH_COMPONENT:
	elements = 1;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
	elements = 1;
	break;
      case GL_LUMINANCE_ALPHA:
	elements = 2;
	break;
      case GL_RGB:
	elements = 3;
	break;
      case GL_RGBA:
      case GL_ABGR_EXT:
	elements = 4;
	break;
      default:
	return 0;
    }
    switch (type) {
      case GL_BITMAP:
	if (format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX) {
	    return (h * ((w+7)/8));
	} else {
	    return 0;
	}
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	esize = 1;
	break;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	esize = 2;
	break;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	esize = 4;
	break;
      default:
	return 0;
    }
    return (elements * esize * w * h);
}

GLint __glBitmap_size(GLsizei w, GLsizei h)
{
    return __glDrawPixels_size(GL_COLOR_INDEX, GL_BITMAP, w, h);
}

GLint __glTexGendv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_GEN_MODE:
	return 1;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
	return 4;
      default:
	return 0;
    }
}

GLint __glTexGenfv_size(GLenum e)
{
    return __glTexGendv_size(e);
}

GLint __glTexGeniv_size(GLenum e)
{
    return __glTexGendv_size(e);
}

GLint __glTexParameterfv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
	return 1;
      case GL_TEXTURE_BORDER_COLOR:
	return 4;
      default:
	return 0;
    }
}

GLint __glTexParameteriv_size(GLenum e)
{
    return __glTexParameterfv_size(e);
}

GLint __glTexEnvfv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_ENV_MODE:
	return 1;
      case GL_TEXTURE_ENV_COLOR:
	return 4;
      default:
	return 0;
    }
}

GLint __glTexEnviv_size(GLenum e)
{
    return __glTexEnvfv_size(e);
}

GLint __glTexImage1D_size(GLenum format, GLenum type, GLsizei w)
{
    GLint elements, esize;

    if (w < 0) return 0;
    switch (format) {
      case GL_COLOR_INDEX:
	elements = 1;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
	elements = 1;
	break;
      case GL_LUMINANCE_ALPHA:
	elements = 2;
	break;
      case GL_RGB:
	elements = 3;
	break;
      case GL_RGBA:
      case GL_ABGR_EXT:
	elements = 4;
	break;
      default:
	return 0;
    }
    switch (type) {
      case GL_BITMAP:
	if (format == GL_COLOR_INDEX) {
	    return (w+7)/8;
	} else {
	    return 0;
	}
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	esize = 1;
	break;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	esize = 2;
	break;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	esize = 4;
	break;
      default:
	return 0;
    }
    return (elements * esize * w);
}

GLint __glTexImage2D_size(GLenum format, GLenum type, GLsizei w, GLsizei h)
{
    GLint elements, esize;

    if (w < 0) return 0;
    if (h < 0) return 0;
    switch (format) {
      case GL_COLOR_INDEX:
	elements = 1;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
	elements = 1;
	break;
      case GL_LUMINANCE_ALPHA:
	elements = 2;
	break;
      case GL_RGB:
	elements = 3;
	break;
      case GL_RGBA:
      case GL_ABGR_EXT:
	elements = 4;
	break;
      default:
	return 0;
    }
    switch (type) {
      case GL_BITMAP:
	if (format == GL_COLOR_INDEX) {
	    return (h * ((w+7)/8));
	} else {
	    return 0;
	}
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	esize = 1;
	break;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	esize = 2;
	break;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	esize = 4;
	break;
      default:
	return 0;
    }
    return (elements * esize * w * h);
}

GLint __glLightfv_size(GLenum pname)
{
    switch (pname) {
      case GL_SPOT_EXPONENT:		return 1;
      case GL_SPOT_CUTOFF:		return 1;
      case GL_AMBIENT:			return 4;
      case GL_DIFFUSE:			return 4;
      case GL_SPECULAR:			return 4;
      case GL_POSITION:			return 4;
      case GL_SPOT_DIRECTION:		return 3;
      case GL_CONSTANT_ATTENUATION:	return 1;
      case GL_LINEAR_ATTENUATION:	return 1;
      case GL_QUADRATIC_ATTENUATION:	return 1;
      default:
	return 0;
    }
}

GLint __glLightiv_size(GLenum pname)
{
    return __glLightfv_size(pname);
}

GLint __glLightModelfv_size(GLenum pname)
{
    switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:		return 4;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:		return 1;
      case GL_LIGHT_MODEL_TWO_SIDE:		return 1;
      default:
	return 0;
    }
}

GLint __glLightModeliv_size(GLenum pname)
{
    return __glLightModelfv_size(pname);
}

GLint __glMaterialfv_size(GLenum pname)
{
    switch (pname) {
      case GL_SHININESS:		return 1;
      case GL_EMISSION:			return 4;
      case GL_AMBIENT:			return 4;
      case GL_DIFFUSE:			return 4;
      case GL_SPECULAR:			return 4;
      case GL_AMBIENT_AND_DIFFUSE:	return 4;
      case GL_COLOR_INDEXES:		return 3;
      default:
	return 0;
    }
}

GLint __glMaterialiv_size(GLenum pname)
{
    return __glMaterialfv_size(pname);
}

