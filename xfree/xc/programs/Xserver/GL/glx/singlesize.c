/* $XFree86: xc/programs/Xserver/GL/glx/singlesize.c,v 1.2 1999/06/14 07:31:35 dawes Exp $ */
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
#include "singlesize.h"

/*
** These routines compute the size of variable-size returned parameters.
** Unlike the similar routines that do the same thing for variable-size
** incoming parameters, the samplegl library itself doesn't use these routines.
** Hence, they are located here, in the GLX extension library.
*/

GLint __glReadPixels_size(GLenum format, GLenum type, GLint w, GLint h)
{
    GLint elements, esize;
    GLint rowsize, padding;
    
    if (w < 0 || h < 0) { 
	return -1;
    }
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
	return -1;
    }
    /*
    ** According to the GLX protocol, each row must be padded to a multiple of
    ** 4 bytes.  4 bytes also happens to be the default alignment in the pixel
    ** store modes of the GL.
    */
    switch (type) {
      case GL_BITMAP:
        if (format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX) {
	   rowsize = ((w * elements)+7)/8;
	   padding = rowsize % 4;
	   if (padding) {
	      rowsize += 4 - padding;
	   }
	   return (rowsize * h);
	} else {
	   return -1;
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
	return -1;
    }
    rowsize = w * elements * esize;
    padding = rowsize % 4;
    if (padding) {
	rowsize += 4 - padding;
    }
    return (rowsize * h);
}

GLint __glGetTexEnvfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	return 1;
      case GL_TEXTURE_ENV_COLOR:
	return 4;
      default:
	return -1;
    }
}


GLint __glGetTexEnviv_size(GLenum pname)
{
    return __glGetTexEnvfv_size(pname);
}

GLint __glGetTexGenfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	return 1;
      case GL_OBJECT_PLANE:
	return 4;
      case GL_EYE_PLANE:
	return 4;
      default:
	return -1;
    }
}

GLint __glGetTexGendv_size(GLenum pname)
{
    return __glGetTexGenfv_size(pname);
}

GLint __glGetTexGeniv_size(GLenum pname)
{
    return __glGetTexGenfv_size(pname);
}

GLint __glGetTexParameterfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	return 1;
      case GL_TEXTURE_WRAP_T:
	return 1;
      case GL_TEXTURE_MIN_FILTER:
	return 1;
      case GL_TEXTURE_MAG_FILTER:
	return 1;
      case GL_TEXTURE_BORDER_COLOR:
	return 4;
      case GL_TEXTURE_PRIORITY:
	return 1;
      case GL_TEXTURE_RESIDENT:
        return 1;

      default:
	return -1;
    }
}

GLint __glGetTexParameteriv_size(GLenum pname)
{
    return __glGetTexParameterfv_size(pname);
}

GLint __glGetLightfv_size(GLenum pname)
{
    switch (pname) {
      case GL_AMBIENT:
	return 4;
      case GL_DIFFUSE:
	return 4;
      case GL_SPECULAR:
	return 4;
      case GL_POSITION:	    
	return 4;
      case GL_SPOT_DIRECTION:
	return 3;
      case GL_SPOT_EXPONENT:
	return 1;
      case GL_SPOT_CUTOFF:
	return 1;
      case GL_CONSTANT_ATTENUATION:
	return 1;
      case GL_LINEAR_ATTENUATION:
	return 1;
      case GL_QUADRATIC_ATTENUATION:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetLightiv_size(GLenum pname)
{
    return __glGetLightfv_size(pname);
}

static GLint EvalComputeK(GLenum target)
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
    }
    return 0;
}
  
GLint __glGetMap_size(GLenum target, GLenum query)
{
    GLint k, order=0, majorMinor[2];

    /*
    ** Assume target and query are both valid.
    */
    switch (target) {
      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
	switch (query) {
	  case GL_COEFF:
	    k = EvalComputeK(target);
	    glGetMapiv(target, GL_ORDER, &order);
	    /*
	    ** The query above might fail, but then order will be zero anyway.
	    */
	    return (order * k);
	  case GL_DOMAIN:
	    return 2;
	  case GL_ORDER:
	    return 1;
	}
	break;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
	switch (query) {
	  case GL_COEFF:
	    k = EvalComputeK(target);
	    majorMinor[0] = majorMinor[1] = 0;
	    glGetMapiv(target, GL_ORDER, majorMinor);
	    /*
	    ** The query above might fail, but then majorMinor will be zeroes
	    */
	    return (majorMinor[0] * majorMinor[1] * k);
	  case GL_DOMAIN:
	    return 4;
	  case GL_ORDER:
	    return 2;
	}
	break;
    }
    return -1;
}

GLint __glGetMapdv_size(GLenum target, GLenum query)
{
    return __glGetMap_size(target, query);
}

GLint __glGetMapfv_size(GLenum target, GLenum query)
{
    return __glGetMap_size(target, query);
}

GLint __glGetMapiv_size(GLenum target, GLenum query)
{
    return __glGetMap_size(target, query);
}

GLint __glGetMaterialfv_size(GLenum pname)
{
    switch (pname) {
      case GL_SHININESS:
	return 1;
      case GL_COLOR_INDEXES:
	return 3;
      case GL_EMISSION:
	return 4;
      case GL_AMBIENT:
	return 4;
      case GL_DIFFUSE:
	return 4;
      case GL_SPECULAR:
	return 4;
      case GL_AMBIENT_AND_DIFFUSE:
	return 4;
      default:
	return -1;
    }
}

GLint __glGetMaterialiv_size(GLenum pname)
{
    return __glGetMaterialfv_size(pname);
}

GLint __glGetPixelMap_size(GLenum map)
{
    GLint size;
    GLenum query;
    
    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
	query = GL_PIXEL_MAP_I_TO_I_SIZE;
	break;
      case GL_PIXEL_MAP_S_TO_S:
	query = GL_PIXEL_MAP_S_TO_S_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_R:
	query = GL_PIXEL_MAP_I_TO_R_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_G:
	query = GL_PIXEL_MAP_I_TO_G_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_B:
	query = GL_PIXEL_MAP_I_TO_B_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_A:
	query = GL_PIXEL_MAP_I_TO_A_SIZE;
	break;
      case GL_PIXEL_MAP_R_TO_R:
	query = GL_PIXEL_MAP_R_TO_R_SIZE;
	break;
      case GL_PIXEL_MAP_G_TO_G:
	query = GL_PIXEL_MAP_G_TO_G_SIZE;
	break;
      case GL_PIXEL_MAP_B_TO_B:
	query = GL_PIXEL_MAP_B_TO_B_SIZE;
	break;
      case GL_PIXEL_MAP_A_TO_A:
	query = GL_PIXEL_MAP_A_TO_A_SIZE;
	break;
      default:
	return -1;
    }
    glGetIntegerv(query, &size);
    return size;
}

GLint __glGetPixelMapfv_size(GLenum map)
{
    return __glGetPixelMap_size(map);
}

GLint __glGetPixelMapuiv_size(GLenum map)
{
    return __glGetPixelMap_size(map);
}

GLint __glGetPixelMapusv_size(GLenum map)
{
    return __glGetPixelMap_size(map);
}

/*
** Return the number of words needed to pass back the requested
** value.
*/
GLint __glGet_size(GLenum sq)
{
    switch (sq) {
      case GL_MAX_TEXTURE_SIZE:
	return 1;
      case GL_SUBPIXEL_BITS:
	return 1;
      case GL_MAX_LIST_NESTING:
	return 1;
      case GL_MAP1_COLOR_4:
      case GL_MAP1_INDEX:
      case GL_MAP1_NORMAL:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
	return 1;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_INDEX:
      case GL_MAP2_NORMAL:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
	return 1;
      case GL_AUTO_NORMAL:
	return 1;
      case GL_CURRENT_COLOR:
	return 4;
      case GL_CURRENT_INDEX:
	return 1;
      case GL_CURRENT_NORMAL:
	return 3;
      case GL_CURRENT_TEXTURE_COORDS:
	return 4;
      case GL_CURRENT_RASTER_INDEX:
	return 1;
      case GL_CURRENT_RASTER_COLOR:
	return 4;
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
	return 4;
      case GL_CURRENT_RASTER_POSITION:
	return 4;
      case GL_CURRENT_RASTER_POSITION_VALID:
	return 1;
      case GL_CURRENT_RASTER_DISTANCE:
	return 1;
      case GL_POINT_SIZE:
	return 1;
      case GL_POINT_SIZE_RANGE:
	return 2;
      case GL_POINT_SIZE_GRANULARITY:
	return 1;
      case GL_POINT_SMOOTH:
	return 1;
      case GL_LINE_SMOOTH:
	return 1;
      case GL_LINE_WIDTH:
	return 1;
      case GL_LINE_WIDTH_RANGE:
	return 2;
      case GL_LINE_WIDTH_GRANULARITY:
	return 1;
      case GL_LINE_STIPPLE_PATTERN:
	return 1;
      case GL_LINE_STIPPLE_REPEAT:
	return 1;
      case GL_LINE_STIPPLE:
	return 1;
      case GL_POLYGON_MODE:
	return 2;
      case GL_POLYGON_SMOOTH:
	return 1;
      case GL_POLYGON_STIPPLE:
	return 1;
      case GL_EDGE_FLAG:
	return 1;
      case GL_CULL_FACE:
	return 1;
      case GL_CULL_FACE_MODE:
	return 1;
      case GL_FRONT_FACE:
	return 1;
      case GL_LIGHTING:
	return 1;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
	return 1;
      case GL_LIGHT_MODEL_TWO_SIDE:
	return 1;
      case GL_LIGHT_MODEL_AMBIENT:
	return 4;
      case GL_COLOR_MATERIAL:
	return 1;
      case GL_COLOR_MATERIAL_FACE:
	return 1;
      case GL_COLOR_MATERIAL_PARAMETER:
	return 1;
      case GL_SHADE_MODEL:
	return 1;
      case GL_FOG:
	return 1;
      case GL_FOG_INDEX:
	return 1;
      case GL_FOG_DENSITY:
	return 1;
      case GL_FOG_START:
	return 1;
      case GL_FOG_END:
	return 1;
      case GL_FOG_MODE:
	return 1;
      case GL_FOG_COLOR:
	return 4;
      case GL_DEPTH_RANGE:
	return 2;
      case GL_DEPTH_TEST:
	return 1;
      case GL_DEPTH_WRITEMASK:
	return 1;
      case GL_DEPTH_CLEAR_VALUE:
	return 1;
      case GL_DEPTH_FUNC:
	return 1;
      case GL_ACCUM_CLEAR_VALUE:
	return 4;
      case GL_STENCIL_TEST:
	return 1;
      case GL_STENCIL_CLEAR_VALUE:
	return 1;
      case GL_STENCIL_FUNC:
	return 1;
      case GL_STENCIL_VALUE_MASK:
	return 1;
      case GL_STENCIL_FAIL:
	return 1;
      case GL_STENCIL_PASS_DEPTH_FAIL:
	return 1;
      case GL_STENCIL_PASS_DEPTH_PASS:
	return 1;
      case GL_STENCIL_REF:
	return 1;
      case GL_STENCIL_WRITEMASK:
	return 1;
      case GL_MATRIX_MODE:
	return 1;
      case GL_NORMALIZE:
	return 1;
      case GL_VIEWPORT:
	return 4;
      case GL_ATTRIB_STACK_DEPTH:
	return 1;
      case GL_MODELVIEW_STACK_DEPTH:
	return 1;
      case GL_PROJECTION_STACK_DEPTH:
	return 1;
      case GL_TEXTURE_STACK_DEPTH:
	return 1;
      case GL_MODELVIEW_MATRIX:
	return 16;
      case GL_PROJECTION_MATRIX:
	return 16;
      case GL_TEXTURE_MATRIX:
	return 16;
      case GL_ALPHA_TEST:
	return 1;
      case GL_ALPHA_TEST_FUNC:
	return 1;
      case GL_ALPHA_TEST_REF:
	return 1;
      case GL_DITHER:
	return 1;
      case GL_BLEND_DST:
	return 1;
      case GL_BLEND_SRC:
	return 1;
      case GL_BLEND:
	return 1;
      case GL_LOGIC_OP_MODE:
	return 1;
      case GL_LOGIC_OP:
	return 1;
      case GL_DRAW_BUFFER:
	return 1;
      case GL_READ_BUFFER:
	return 1;
      case GL_SCISSOR_TEST:
	return 1;
      case GL_SCISSOR_BOX:
	return 4;
      case GL_INDEX_CLEAR_VALUE:
	return 1;
      case GL_INDEX_MODE:
	return 1;
      case GL_INDEX_WRITEMASK:
	return 1;
      case GL_COLOR_CLEAR_VALUE:
	return 4;
      case GL_RGBA_MODE:
	return 1;
      case GL_COLOR_WRITEMASK:
	return 4;
      case GL_RENDER_MODE:
	return 1;
      case GL_PERSPECTIVE_CORRECTION_HINT:
	return 1;
      case GL_POINT_SMOOTH_HINT:
	return 1;
      case GL_LINE_SMOOTH_HINT:
	return 1;
      case GL_POLYGON_SMOOTH_HINT:
	return 1;
      case GL_FOG_HINT:
	return 1;
      case GL_LIST_BASE:
	return 1;
      case GL_LIST_INDEX:
	return 1;
      case GL_LIST_MODE:
	return 1;
      case GL_TEXTURE_GEN_S:
	return 1;
      case GL_TEXTURE_GEN_T:
	return 1;
      case GL_TEXTURE_GEN_R:
	return 1;
      case GL_TEXTURE_GEN_Q:
	return 1;
      case GL_PACK_SWAP_BYTES:
	return 1;
      case GL_PACK_LSB_FIRST:
	return 1;
      case GL_PACK_ROW_LENGTH:
	return 1;
      case GL_PACK_SKIP_ROWS:
	return 1;
      case GL_PACK_SKIP_PIXELS:
	return 1;
      case GL_PACK_ALIGNMENT:
	return 1;
      case GL_UNPACK_SWAP_BYTES:
	return 1;
      case GL_UNPACK_LSB_FIRST:
	return 1;
      case GL_UNPACK_ROW_LENGTH:
	return 1;
      case GL_UNPACK_SKIP_ROWS:
	return 1;
      case GL_UNPACK_SKIP_PIXELS:
	return 1;
      case GL_UNPACK_ALIGNMENT:
	return 1;
      case GL_MAP_COLOR:
	return 1;
      case GL_MAP_STENCIL:
	return 1;
      case GL_INDEX_SHIFT:
	return 1;
      case GL_INDEX_OFFSET:
	return 1;
      case GL_RED_SCALE:
      case GL_GREEN_SCALE:
      case GL_BLUE_SCALE:
      case GL_ALPHA_SCALE:
      case GL_DEPTH_SCALE:
	return 1;
      case GL_RED_BIAS:
      case GL_GREEN_BIAS:
      case GL_BLUE_BIAS:
      case GL_ALPHA_BIAS:
      case GL_DEPTH_BIAS:
	return 1;
      case GL_ZOOM_X:
      case GL_ZOOM_Y:
	return 1;
      case GL_PIXEL_MAP_I_TO_I_SIZE:      case GL_PIXEL_MAP_S_TO_S_SIZE:
      case GL_PIXEL_MAP_I_TO_R_SIZE:      case GL_PIXEL_MAP_I_TO_G_SIZE:
      case GL_PIXEL_MAP_I_TO_B_SIZE:      case GL_PIXEL_MAP_I_TO_A_SIZE:
      case GL_PIXEL_MAP_R_TO_R_SIZE:      case GL_PIXEL_MAP_G_TO_G_SIZE:
      case GL_PIXEL_MAP_B_TO_B_SIZE:      case GL_PIXEL_MAP_A_TO_A_SIZE:
	return 1;
      case GL_MAX_EVAL_ORDER:
	return 1;
      case GL_MAX_LIGHTS:
	return 1;
      case GL_MAX_CLIP_PLANES:
	return 1;
      case GL_MAX_PIXEL_MAP_TABLE:
	return 1;
      case GL_MAX_ATTRIB_STACK_DEPTH:
	return 1;
      case GL_MAX_MODELVIEW_STACK_DEPTH:
	return 1;
      case GL_MAX_NAME_STACK_DEPTH:
	return 1;
      case GL_MAX_PROJECTION_STACK_DEPTH:
	return 1;
      case GL_MAX_TEXTURE_STACK_DEPTH:
	return 1;
      case GL_INDEX_BITS:
	return 1;
      case GL_RED_BITS:
	return 1;
      case GL_GREEN_BITS:
	return 1;
      case GL_BLUE_BITS:
	return 1;
      case GL_ALPHA_BITS:
	return 1;
      case GL_DEPTH_BITS:
	return 1;
      case GL_STENCIL_BITS:
	return 1;
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
	return 1;
      case GL_MAP1_GRID_DOMAIN:
	return 2;
      case GL_MAP1_GRID_SEGMENTS:
	return 1;
      case GL_MAP2_GRID_DOMAIN:
	return 4;
      case GL_MAP2_GRID_SEGMENTS:
	return 2;
      case GL_TEXTURE_1D:
      case GL_TEXTURE_2D:
	return 1;
      case GL_NAME_STACK_DEPTH:
	return 1;
      case GL_MAX_VIEWPORT_DIMS:
        return 2;
      case GL_DOUBLEBUFFER:
        return 1;
      case GL_AUX_BUFFERS:
        return 1;
      case GL_STEREO:
        return 1;
      case GL_CLIP_PLANE0:	case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:	case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:	case GL_CLIP_PLANE5:
	return 1;
      case GL_LIGHT0:	case GL_LIGHT1:
      case GL_LIGHT2:	case GL_LIGHT3:
      case GL_LIGHT4:	case GL_LIGHT5:
      case GL_LIGHT6:	case GL_LIGHT7:
	return 1;
      case GL_VERTEX_ARRAY:
      case GL_VERTEX_ARRAY_SIZE:
      case GL_VERTEX_ARRAY_TYPE:
      case GL_VERTEX_ARRAY_STRIDE:
      case GL_NORMAL_ARRAY:
      case GL_NORMAL_ARRAY_TYPE:
      case GL_NORMAL_ARRAY_STRIDE:
      case GL_COLOR_ARRAY:
      case GL_COLOR_ARRAY_SIZE:
      case GL_COLOR_ARRAY_TYPE:
      case GL_COLOR_ARRAY_STRIDE:
      case GL_INDEX_ARRAY:
      case GL_INDEX_ARRAY_TYPE:
      case GL_INDEX_ARRAY_STRIDE:
      case GL_TEXTURE_COORD_ARRAY:
      case GL_TEXTURE_COORD_ARRAY_SIZE:
      case GL_TEXTURE_COORD_ARRAY_TYPE:
      case GL_TEXTURE_COORD_ARRAY_STRIDE:
      case GL_EDGE_FLAG_ARRAY:
      case GL_EDGE_FLAG_ARRAY_STRIDE:
	return 1;
      case GL_TEXTURE_BINDING_1D:
      case GL_TEXTURE_BINDING_2D:
	return 1;
      case GL_BLEND_COLOR_EXT:
	return 4;
      case GL_BLEND_EQUATION_EXT:
	return 1;
      case GL_POLYGON_OFFSET_FACTOR:
      case GL_POLYGON_OFFSET_UNITS:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetDoublev_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetFloatv_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetIntegerv_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetBooleanv_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetTexLevelParameterfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_WIDTH:
      case GL_TEXTURE_HEIGHT:
      case GL_TEXTURE_COMPONENTS:
      case GL_TEXTURE_BORDER:
      case GL_TEXTURE_RED_SIZE:
      case GL_TEXTURE_GREEN_SIZE:
      case GL_TEXTURE_BLUE_SIZE:
      case GL_TEXTURE_ALPHA_SIZE:
      case GL_TEXTURE_LUMINANCE_SIZE:
      case GL_TEXTURE_INTENSITY_SIZE:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetTexLevelParameteriv_size(GLenum pname)
{
    return __glGetTexLevelParameterfv_size(pname);
}

GLint __glGetTexImage_size(GLenum target, GLint level, GLenum format,
			   GLenum type, GLint width, GLint height)
{
    GLint elements, esize;
    GLint padding, rowsize;

    switch (format) {
      case GL_RGBA:
      case GL_ABGR_EXT:
	elements = 4;
	break;
      case GL_RGB:
	elements = 3;
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
      default:
	return -1;
    }
    switch (type) {
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
	return -1;
    }
    /*
    ** According to the GLX protocol, each row must be padded to a multiple of
    ** 4 bytes.  4 bytes also happens to be the default alignment in the pixel
    ** store modes of the GL.
    */
    rowsize = width * elements * esize;
    padding = rowsize % 4;
    if (padding) {
	rowsize += 4 - padding;
    }
    return (rowsize * height);
}
