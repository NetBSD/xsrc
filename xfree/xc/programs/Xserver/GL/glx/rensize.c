/* $XFree86: xc/programs/Xserver/GL/glx/rensize.c,v 1.2 1999/06/14 07:31:34 dawes Exp $ */
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

#include "glxserver.h"
#include <GL/gl.h>
#include <GL/glxproto.h>
#include "unpack.h"
#include "impsize.h"

#define SWAPL(a) \
  (((a & 0xff000000U)>>24) | ((a & 0xff0000U)>>8) | \
   ((a & 0xff00U)<<8) | ((a & 0xffU)<<24))

int __glXCallListsReqSize(GLbyte *pc, Bool swap )
{
    GLsizei n = *(GLsizei *)(pc + 0);
    GLenum type = *(GLenum *)(pc + 4);

    if (swap) {
	n = SWAPL( n );
	type = SWAPL( type );
    }
    return __glCallLists_size( n, type );	/* defined in samplegl lib */
}

int __glXFogivReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 0);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glFogiv_size( pname );		/* defined in samplegl lib */
}

int __glXFogfvReqSize(GLbyte *pc, Bool swap )
{
    return __glXFogivReqSize( pc, swap );
}

int __glXLightfvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 4);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glLightfv_size( pname );	/* defined in samplegl lib */
}
	   
int __glXLightivReqSize(GLbyte *pc, Bool swap )
{
    return __glXLightfvReqSize( pc, swap );
}

int __glXLightModelfvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 0);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glLightModelfv_size( pname );	/* defined in samplegl lib */
}

int __glXLightModelivReqSize(GLbyte *pc, Bool swap )
{
    return __glXLightModelfvReqSize( pc, swap );
}

int __glXMaterialfvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 4);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glMaterialfv_size( pname );	/* defined in samplegl lib */
}

int __glXMaterialivReqSize(GLbyte *pc, Bool swap )
{
    return __glXMaterialfvReqSize( pc, swap );
}

int __glXTexGendvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 4);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 8 * __glTexGendv_size( pname );	/* defined in samplegl lib */
}

int __glXTexGenfvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 4);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glTexGenfv_size( pname );	/* defined in samplegl lib */
}

int __glXTexGenivReqSize(GLbyte *pc, Bool swap )
{
    return __glXTexGenfvReqSize( pc, swap );
}

int __glXTexParameterfvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 4);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glTexParameterfv_size( pname ); /* defined in samplegl lib */
}

int __glXTexParameterivReqSize(GLbyte *pc, Bool swap )
{
    return __glXTexParameterfvReqSize( pc, swap );
}

int __glXTexEnvfvReqSize(GLbyte *pc, Bool swap )
{
    GLenum pname = *(GLenum *)(pc + 4);
    if (swap) {
	pname = SWAPL( pname );
    }
    return 4 * __glTexEnvfv_size( pname );	/* defined in samplegl lib */
}

int __glXTexEnvivReqSize(GLbyte *pc, Bool swap )
{
    return __glXTexEnvfvReqSize( pc, swap );
}

static int Map1Size( GLint k, GLint order)
{
    if (order <= 0 || k < 0) return -1;
    return k * order;
}

int __glXMap1dReqSize(GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint order, k;
    
    target = *(GLenum*) (pc + 16);
    order = *(GLint*) (pc + 20);
    if (swap) {
	target = SWAPL( target );
	order = SWAPL( order );
    }
    k = __glEvalComputeK( target );
    return 8 * Map1Size( k, order );
}

int __glXMap1fReqSize(GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint order, k;
    
    target = *(GLenum *)(pc + 0); 
    order = *(GLint *)(pc + 12);
    if (swap) {
	target = SWAPL( target );
	order = SWAPL( order );
    }
    k = __glEvalComputeK(target);
    return 4 * Map1Size(k, order);
}

static int Map2Size(int k, int majorOrder, int minorOrder)
{
    if (majorOrder <= 0 || minorOrder <= 0 || k < 0) return -1;
    return k * majorOrder * minorOrder;
}

int __glXMap2dReqSize(GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint uorder, vorder, k;
    
    target = *(GLenum *)(pc + 32);
    uorder = *(GLint *)(pc + 36);
    vorder = *(GLint *)(pc + 40);
    if (swap) {
	target = SWAPL( target );
	uorder = SWAPL( uorder );
	vorder = SWAPL( vorder );
    }
    k = __glEvalComputeK( target );
    return 8 * Map2Size( k, uorder, vorder );
}

int __glXMap2fReqSize(GLbyte *pc, Bool swap )
{
    GLenum target;
    GLint uorder, vorder, k;
    
    target = *(GLenum *)(pc + 0); 
    uorder = *(GLint *)(pc + 12);
    vorder = *(GLint *)(pc + 24);
    if (swap) {
	target = SWAPL( target );
	uorder = SWAPL( uorder );
	vorder = SWAPL( vorder );
    }
    k = __glEvalComputeK( target );
    return 4 * Map2Size( k, uorder, vorder );
}

int __glXPixelMapfvReqSize(GLbyte *pc, Bool swap )
{
    GLint mapsize;
    mapsize = *(GLint *)(pc + 4);
    if (swap) {
	mapsize = SWAPL( mapsize );
    }
    return 4 * mapsize;
}

int __glXPixelMapuivReqSize(GLbyte *pc, Bool swap )
{
    return __glXPixelMapfvReqSize( pc, swap );
}

int __glXPixelMapusvReqSize(GLbyte *pc, Bool swap )
{
    GLint mapsize;
    mapsize = *(GLint *)(pc + 4);
    if (swap) {
	mapsize = SWAPL( mapsize );
    }
    return 2 * mapsize;
}

static int ImageSize( GLenum format, GLenum type, GLsizei w, GLsizei h,
		      GLint rowLength, GLint skipRows, GLint alignment )
{
    GLint bytesPerElement, elementsPerGroup, groupsPerRow;
    GLint groupSize, rowSize, padding;
    
    if (w < 0 || h < 0 ||
	(type == GL_BITMAP &&
	 (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX))) {
	return -1;
    }
    if (w==0 || h==0) return 0;
    
    if (type == GL_BITMAP) {
	if (rowLength > 0) {
	    groupsPerRow = rowLength;
	} else {
	    groupsPerRow = w;
	}
	rowSize = (groupsPerRow + 7) >> 3;
	padding = (rowSize % alignment);
	if (padding) {
	    rowSize += alignment - padding;
	}
	return ((h + skipRows) * rowSize);
    } else {
	switch(type) {
	  case GL_UNSIGNED_BYTE:
	  case GL_BYTE:
	    bytesPerElement = 1;
	    break;
	  case GL_UNSIGNED_SHORT:
	  case GL_SHORT:
	    bytesPerElement = 2;
	    break;
	  case GL_INT:
	  case GL_UNSIGNED_INT:
	  case GL_FLOAT:
	    bytesPerElement = 4;
	    break;
	  default:
	    return -1;
	}
	switch(format) {
	  case GL_COLOR_INDEX:
	  case GL_STENCIL_INDEX:
	  case GL_DEPTH_COMPONENT:
	    elementsPerGroup = 1;
	    break;
	  case GL_RED:
	  case GL_GREEN:
	  case GL_BLUE:
	  case GL_ALPHA:
	  case GL_LUMINANCE:
	    elementsPerGroup = 1;
	    break;
	  case GL_LUMINANCE_ALPHA:
	    elementsPerGroup = 2;
	    break;
	  case GL_RGB:
	    elementsPerGroup = 3;
	    break;
	  case GL_RGBA:
	  case GL_ABGR_EXT:
	    elementsPerGroup = 4;
	    break;
	  default:
	    return -1;
	}
	groupSize = bytesPerElement * elementsPerGroup;
	if (rowLength > 0) {
	    groupsPerRow = rowLength;
	} else {
	    groupsPerRow = w;
	}
	rowSize = groupsPerRow * groupSize;
	padding = (rowSize % alignment);
	if (padding) {
	    rowSize += alignment - padding;
	}
	return ((h + skipRows) * rowSize);
    }
}

int __glXDrawPixelsReqSize(GLbyte *pc, Bool swap )
{
    __GLXdispatchDrawPixelsHeader *hdr = (__GLXdispatchDrawPixelsHeader *) pc;
    GLenum format = hdr->format;
    GLenum type = hdr->type;
    GLint w = hdr->width;
    GLint h = hdr->height;
    GLint rowLength = hdr->rowLength;
    GLint skipRows = hdr->skipRows;
    GLint alignment = hdr->alignment;

    if (swap) {
	format = SWAPL( format );
	type = SWAPL( type );
	w = SWAPL( w );
	h = SWAPL( h );
 	rowLength = SWAPL( rowLength );
	skipRows = SWAPL( skipRows );
	alignment = SWAPL( alignment );
    }
    return ImageSize( format, type, w, h, rowLength, skipRows, alignment );
}

int __glXBitmapReqSize(GLbyte *pc, Bool swap )
{
    __GLXdispatchBitmapHeader *hdr = (__GLXdispatchBitmapHeader *) pc;
    GLint w = hdr->width;
    GLint h = hdr->height;
    GLint rowLength = hdr->rowLength;
    GLint skipRows = hdr->skipRows;
    GLint alignment = hdr->alignment;

    if (swap) {
	w = SWAPL( w );
	h = SWAPL( h );
 	rowLength = SWAPL( rowLength );
	skipRows = SWAPL( skipRows );
	alignment = SWAPL( alignment );
    }
    return ImageSize( GL_COLOR_INDEX, GL_BITMAP, w, h,
		      rowLength, skipRows, alignment );
}

int __glXTexImage1DReqSize(GLbyte *pc, Bool swap )
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;
    GLenum target = hdr->target;
    GLenum format = hdr->format;
    GLenum type = hdr->type;
    GLint w = hdr->width;
    GLint rowLength = hdr->rowLength;
    GLint skipRows = hdr->skipRows;
    GLint alignment = hdr->alignment;

    if (swap) {
	target = SWAPL( target );
	format = SWAPL( format );
	type = SWAPL( type );
	w = SWAPL( w );
	rowLength = SWAPL( rowLength );
	skipRows = SWAPL( skipRows );
	alignment = SWAPL( alignment );
    }
    if (target == GL_PROXY_TEXTURE_1D) {
	return 0;
    } else if (format == GL_STENCIL_INDEX || format == GL_DEPTH_COMPONENT) {
	return -1;
    }
    return ImageSize( format, type, w, 1, rowLength, skipRows, alignment );
}

int __glXTexImage2DReqSize(GLbyte *pc, Bool swap )
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;
    GLenum target = hdr->target;
    GLenum format = hdr->format;
    GLenum type = hdr->type;
    GLint w = hdr->width;
    GLint h = hdr->height;
    GLint rowLength = hdr->rowLength;
    GLint skipRows = hdr->skipRows;
    GLint alignment = hdr->alignment;

    if (swap) {
	target = SWAPL( target );
	format = SWAPL( format );
	type = SWAPL( type );
	w = SWAPL( w );
	h = SWAPL( h );
	rowLength = SWAPL( rowLength );
	skipRows = SWAPL( skipRows );
	alignment = SWAPL( alignment );
    }
    if (target == GL_PROXY_TEXTURE_2D) {
	return 0;
    } else if (format == GL_STENCIL_INDEX || format == GL_DEPTH_COMPONENT) {
	return -1;
    }
    return ImageSize( format, type, w, h, rowLength, skipRows, alignment );
}

int __glXTypeSize(GLenum enm)
{
  switch(enm) {
    case GL_BYTE: 		return sizeof(GLbyte); 	
    case GL_UNSIGNED_BYTE: 	return sizeof(GLubyte);
    case GL_SHORT: 		return sizeof(GLshort); 	
    case GL_UNSIGNED_SHORT: 	return sizeof(GLushort);
    case GL_INT: 		return sizeof(GLint); 		
    case GL_UNSIGNED_INT: 	return sizeof(GLint);
    case GL_FLOAT: 		return sizeof(GLfloat);
    case GL_DOUBLE:	 	return sizeof(GLdouble);
    default:			return -1;
  }
}

int __glXDrawArraysSize( GLbyte *pc, Bool swap )
{
    __GLXdispatchDrawArraysHeader *hdr = (__GLXdispatchDrawArraysHeader *) pc;
    __GLXdispatchDrawArraysComponentHeader *compHeader;
    GLint numVertexes = hdr->numVertexes;
    GLint numComponents = hdr->numComponents;
    GLint arrayElementSize = 0;
    int i;

    if (swap) {
	numVertexes = SWAPL( numVertexes );
	numComponents = SWAPL( numComponents );
    }

    pc += sizeof(__GLXdispatchDrawArraysHeader);
    compHeader = (__GLXdispatchDrawArraysComponentHeader *) pc;

    for (i=0; i<numComponents; i++) {
	GLenum datatype = compHeader[i].datatype;
	GLint numVals = compHeader[i].numVals;
	GLint component = compHeader[i].component;

	if (swap) {
	    datatype = SWAPL( datatype );
	    numVals = SWAPL( numVals );
	    component = SWAPL( component );
	}

        switch (component) {
          case GL_VERTEX_ARRAY:
          case GL_COLOR_ARRAY:
          case GL_TEXTURE_COORD_ARRAY:
            break;
          case GL_NORMAL_ARRAY:
            if (numVals != 3) {
		/* bad size */
                return -1;
            }
	    break;
          case GL_INDEX_ARRAY:
            if (numVals != 1) {
		/* bad size */
                return -1;
            }
            break;
          case GL_EDGE_FLAG_ARRAY:
            if ((numVals != 1) && (datatype != GL_UNSIGNED_BYTE)) {
		/* bad size or bad type */
		return -1;
            }
            break;
          default:
            /* unknown component type */
            return -1;
	}

	arrayElementSize += __GLX_PAD(numVals * __glXTypeSize(datatype));

	pc += sizeof(__GLXdispatchDrawArraysComponentHeader);
    }

    return ((numComponents * sizeof(__GLXdispatchDrawArraysComponentHeader)) +
            (numVertexes * arrayElementSize));
}

int __glXPrioritizeTexturesReqSize(GLbyte *pc, Bool swap )
{
    GLint n = *(GLsizei *)(pc + 0);
    if (swap) n = SWAPL(n);
    return(8*n); /* 4*n for textures, 4*n for priorities */
}

int __glXTexSubImage1DReqSize(GLbyte *pc, Bool swap )
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;
    GLenum target = hdr->target;
    GLenum format = hdr->format;
    GLenum type = hdr->type;
    GLint w = hdr->width;
    GLint rowLength = hdr->rowLength;
    GLint skipRows = hdr->skipRows;
    GLint alignment = hdr->alignment;

    if (swap) {
	format = SWAPL( format );
	type = SWAPL( type );
	w = SWAPL( w );
	rowLength = SWAPL( rowLength );
	skipRows = SWAPL( skipRows );
	alignment = SWAPL( alignment );
    }
    return ImageSize( format, type, w, 1, rowLength, skipRows, alignment );
}

int __glXTexSubImage2DReqSize(GLbyte *pc, Bool swap )
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;
    GLenum target = hdr->target;
    GLenum format = hdr->format;
    GLenum type = hdr->type;
    GLint w = hdr->width;
    GLint h = hdr->height;
    GLint rowLength = hdr->rowLength;
    GLint skipRows = hdr->skipRows;
    GLint alignment = hdr->alignment;

    if (swap) {
	format = SWAPL( format );
	type = SWAPL( type );
	w = SWAPL( w );
	h = SWAPL( h );
	rowLength = SWAPL( rowLength );
	skipRows = SWAPL( skipRows );
	alignment = SWAPL( alignment );
    }
    return ImageSize( format, type, w, h, rowLength, skipRows, alignment );
}
