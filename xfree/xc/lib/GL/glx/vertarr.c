/* $XFree86: xc/lib/GL/glx/vertarr.c,v 1.2 1999/06/14 07:23:40 dawes Exp $ */
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
#include "glxclient.h"
#include <string.h>

/* macros for setting function pointers */
#define __GL_VERTEX_FUNC(NAME, let) \
    case GL_##NAME: \
      if (size == 2) \
	va->vertexCall = (void (*)(const char *))glVertex2##let##v; \
      else if (size == 3) \
	va->vertexCall = (void (*)(const char *))glVertex3##let##v; \
      else if (size == 4) \
	va->vertexCall = (void (*)(const char *))glVertex4##let##v; \
      break

#define __GL_NORMAL_FUNC(NAME, let) \
    case GL_##NAME: \
      va->normalCall = (void (*)(const char *))glNormal3##let##v; \
      break

#define __GL_COLOR_FUNC(NAME, let) \
    case GL_##NAME: \
      if (size == 3) \
	va->colorCall = (void (*)(const char *))glColor3##let##v; \
      else if (size == 4)\
	va->colorCall = (void (*)(const char *))glColor4##let##v; \
      break

#define __GL_INDEX_FUNC(NAME, let) \
    case GL_##NAME: \
      va->indexCall = (void (*)(const char *))glIndex##let##v; \
      break

#define __GL_TEXTURE_FUNC(NAME, let) \
    case GL_##NAME: \
      if (size == 1) \
	va->texCoordCall = (void (*)(const char *))glTexCoord1##let##v; \
      else if (size == 2) \
	va->texCoordCall = (void (*)(const char *))glTexCoord2##let##v; \
      else if (size == 3) \
	va->texCoordCall = (void (*)(const char *))glTexCoord3##let##v; \
      else if (size == 4) \
	va->texCoordCall = (void (*)(const char *))glTexCoord4##let##v; \
      break

static GLuint __glXTypeSize(GLenum enm)
{
  switch (enm) {
    case __GL_BOOLEAN_ARRAY:	return sizeof(GLboolean);
    case GL_BYTE: 		return sizeof(GLbyte); 	
    case GL_UNSIGNED_BYTE: 	return sizeof(GLubyte);
    case GL_SHORT: 		return sizeof(GLshort); 	
    case GL_UNSIGNED_SHORT: 	return sizeof(GLushort);
    case GL_INT: 		return sizeof(GLint); 		
    case GL_UNSIGNED_INT: 	return sizeof(GLint);
    case GL_FLOAT: 		return sizeof(GLfloat);
    case GL_DOUBLE:	 	return sizeof(GLdouble);
    default:			return 0;
  }
}

void __glXInitVertexArrayState(__GLXcontext *gc)
{
    __GLXvertArrayState *va = &gc->state.vertArray;

    va->vertexEnable = GL_FALSE;
    va->vertexCall = NULL;
    va->vertexSkip = 0;
    va->vertexPtr = 0;
    va->vertexSize = 4;
    va->vertexType = GL_FLOAT;
    va->vertexStride = 0;

    va->normalEnable = GL_FALSE;
    va->normalCall = NULL;
    va->normalSkip = 0;
    va->normalPtr = 0;
    va->normalType = GL_FLOAT;
    va->normalStride = 0;

    va->colorEnable = GL_FALSE;
    va->colorCall = NULL;
    va->colorSkip = 0;
    va->colorPtr = 0;
    va->colorSize = 4;
    va->colorType = GL_FLOAT;
    va->colorStride = 0;

    va->indexEnable = GL_FALSE;
    va->indexCall = NULL;
    va->indexSkip = 0;
    va->indexPtr = 0;
    va->indexType = GL_FLOAT;
    va->indexStride = 0;

    va->texCoordEnable = GL_FALSE;
    va->texCoordCall = NULL;
    va->texCoordSkip = 0;
    va->texCoordPtr = 0;
    va->texCoordSize = 4;
    va->texCoordType = GL_FLOAT;
    va->texCoordStride = 0;

    va->edgeFlagEnable = GL_FALSE;
    va->edgeFlagCall = NULL;
    va->edgeFlagSkip = 0;
    va->edgeFlagPtr = 0;
    va->edgeFlagStride = 0;
}

/*****************************************************************************/

void glVertexPointer(GLint size, GLenum type, GLsizei stride,
		     const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    /* Check arguments */
    if (size < 2 || size > 4 || stride < 0) {
        __glXSetError(gc, GL_INVALID_VALUE);
        return;
    }

    /* Choose appropriate api call */
    switch(type) {
        __GL_VERTEX_FUNC(SHORT, s);
        __GL_VERTEX_FUNC(INT, i);
        __GL_VERTEX_FUNC(FLOAT, f);
        __GL_VERTEX_FUNC(DOUBLE, d);
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    va->vertexSize = size;
    va->vertexType = type;
    va->vertexStride = stride;
    va->vertexPtr = pointer;

    /* Set internal state */
    if (stride == 0) {
	va->vertexSkip = __glXTypeSize(type) * size;
    } else {
	va->vertexSkip = stride;
    }
}

void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    /* Check arguments */
    if (stride < 0) {
	__glXSetError(gc, GL_INVALID_VALUE);
	return;
    } 

    /* Choose appropriate api call */
    switch(type) {
	__GL_NORMAL_FUNC(BYTE, b);
	__GL_NORMAL_FUNC(SHORT, s);
	__GL_NORMAL_FUNC(INT, i);
	__GL_NORMAL_FUNC(FLOAT, f);
	__GL_NORMAL_FUNC(DOUBLE, d);
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    va->normalType = type;
    va->normalStride = stride;
    va->normalPtr = pointer;

    /* Set internal state */
    if (stride == 0) {
	va->normalSkip = 3 * __glXTypeSize(type);
    } else {
	va->normalSkip = stride;
    }
}

void glColorPointer(GLint size, GLenum type, GLsizei stride,
		    const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    /* Check arguments */
    if (stride < 0) {
	__glXSetError(gc, GL_INVALID_VALUE);
	return;
    } 

    /* Choose appropriate api call */
    switch(type) {
	__GL_COLOR_FUNC(BYTE, b);
	__GL_COLOR_FUNC(UNSIGNED_BYTE, ub);
	__GL_COLOR_FUNC(SHORT, s);
	__GL_COLOR_FUNC(UNSIGNED_SHORT, us);
	__GL_COLOR_FUNC(INT, i);
	__GL_COLOR_FUNC(UNSIGNED_INT, ui);
	__GL_COLOR_FUNC(FLOAT, f);
	__GL_COLOR_FUNC(DOUBLE, d);
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }
    va->colorSize = size;
    va->colorType = type;
    va->colorStride = stride;
    va->colorPtr = pointer;

    /* Set internal state */
    if (stride == 0) {
        va->colorSkip = size * __glXTypeSize(type);
    } else {
        va->colorSkip = stride;
    }
}

void glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    /* Check arguments */
    if (stride < 0) {
        __glXSetError(gc, GL_INVALID_VALUE);
        return;
    }

    /* Choose appropriate api call */
    switch(type) {
	__GL_INDEX_FUNC(UNSIGNED_BYTE, ub);
        __GL_INDEX_FUNC(SHORT, s);
        __GL_INDEX_FUNC(INT, i);
        __GL_INDEX_FUNC(FLOAT, f);
        __GL_INDEX_FUNC(DOUBLE, d);
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    va->indexType = type;
    va->indexStride = stride;
    va->indexPtr = pointer;

    /* Set internal state */
    if (stride == 0) {
	va->indexSkip = __glXTypeSize(type);
    } else {
	va->indexSkip = stride;
    }
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
		       const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    /* Check arguments */
    if (size < 1 || size > 4 || stride < 0) {
	__glXSetError(gc, GL_INVALID_VALUE);
	return;
    } 

    /* Choose appropriate api call */
    switch(type) {
	__GL_TEXTURE_FUNC(SHORT, s);
	__GL_TEXTURE_FUNC(INT, i);
	__GL_TEXTURE_FUNC(FLOAT, f);
	__GL_TEXTURE_FUNC(DOUBLE,  d);
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    va->texCoordSize = size;
    va->texCoordType = type;
    va->texCoordStride = stride;
    va->texCoordPtr = pointer;

    /* Set internal state */
    if (stride == 0) {
	va->texCoordSkip = __glXTypeSize(type) * size;
    } else {
	va->texCoordSkip = stride;
    }
}

void glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    /* Check arguments */
    if (stride < 0) {
	__glXSetError(gc, GL_INVALID_VALUE);
	return;
    } 

    va->edgeFlagStride = stride;
    va->edgeFlagPtr = pointer;

    /* Set internal state */
    if (stride == 0) {
	va->edgeFlagSkip = sizeof(GLboolean);
    } else {
	va->edgeFlagSkip = stride;
    }

    /* Choose appropriate api call */
    va->edgeFlagCall = glEdgeFlagv;

}

void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    GLboolean tEnable = GL_FALSE, cEnable = GL_FALSE, nEnable = GL_FALSE;
    GLenum tType = GL_FLOAT, nType = GL_FLOAT, vType = GL_FLOAT;
    GLenum cType = GL_FALSE;
    GLint tSize = 0, cSize = 0, nSize = 3, vSize;
    int cOffset = 0, nOffset = 0, vOffset = 0;
    GLint trueStride, size;

    switch (format) {
      case GL_V2F:
	vSize = 2;
	size = __glXTypeSize(vType) * vSize;
	break;
      case GL_V3F:
	vSize = 3;
	size = __glXTypeSize(vType) * vSize;
	break;
      case GL_C4UB_V2F:
	cEnable = GL_TRUE;
	cSize = 4;
	cType = GL_UNSIGNED_BYTE;
	vSize = 2;
	vOffset = __glXTypeSize(cType) * cSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_C4UB_V3F:
	cEnable = GL_TRUE;
	cSize = 4;
	cType = GL_UNSIGNED_BYTE;
	vSize = 3;
	vOffset = __glXTypeSize(vType) * cSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_C3F_V3F:
	cEnable = GL_TRUE;
	cSize = 3;
	cType = GL_FLOAT;
	vSize = 3;
	vOffset = __glXTypeSize(cType) * cSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_N3F_V3F:
	nEnable = GL_TRUE;
	vSize = 3;
	vOffset = __glXTypeSize(nType) * nSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_C4F_N3F_V3F:
	cEnable = GL_TRUE;
	cSize = 4;
	cType = GL_FLOAT;
	nEnable = GL_TRUE;
	nOffset = __glXTypeSize(cType) * cSize;
	vSize = 3;
	vOffset = nOffset + __glXTypeSize(nType) * nSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T2F_V3F:
	tEnable = GL_TRUE;
	tSize = 2;
	vSize = 3;
	vOffset = __glXTypeSize(tType) * tSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T4F_V4F:
	tEnable = GL_TRUE;
	tSize = 4;
	vSize = 4;
	vOffset = __glXTypeSize(tType) * tSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T2F_C4UB_V3F:
	tEnable = GL_TRUE;
	tSize = 2;
	cEnable = GL_TRUE;
	cSize = 4;
	cType = GL_UNSIGNED_BYTE;
	cOffset = __glXTypeSize(tType) * tSize;
	vSize = 3;
	vOffset = cOffset + __glXTypeSize(cType) * cSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T2F_C3F_V3F:
	tEnable = GL_TRUE;
	tSize = 2;
	cEnable = GL_TRUE;
	cSize = 3;
	cType = GL_FLOAT;
	cOffset = __glXTypeSize(tType) * tSize;
	vSize = 3;
	vOffset = cOffset + __glXTypeSize(cType) * cSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T2F_N3F_V3F:
	tEnable = GL_TRUE;
	tSize = 2;
	nEnable = GL_TRUE;
	nOffset = __glXTypeSize(tType) * tSize;
	vSize = 3;
	vOffset = nOffset + __glXTypeSize(nType) * nSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T2F_C4F_N3F_V3F:
	tEnable = GL_TRUE;
	tSize = 2;
	cEnable = GL_TRUE;
	cSize = 4;
	cType = GL_FLOAT;
	cOffset = __glXTypeSize(tType) * tSize;
	nEnable = GL_TRUE;
	nOffset = cOffset + __glXTypeSize(cType) * cSize;
	vSize = 3;
	vOffset = nOffset + __glXTypeSize(nType) * nSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      case GL_T4F_C4F_N3F_V4F:
	tEnable = GL_TRUE;
	tSize = 4;
	cEnable = GL_TRUE;
	cSize = 4;
	cType = GL_FLOAT;
	cOffset = __glXTypeSize(tType) * tSize;
	nEnable = GL_TRUE;
	nOffset = cOffset + __glXTypeSize(cType) * cSize;
	vSize = 4;
	vOffset = nOffset + __glXTypeSize(nType) * nSize;
	size = vOffset + __glXTypeSize(vType) * vSize;
	break;
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    trueStride = (stride == 0) ? size : stride;

    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    if (tEnable) {
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(tSize, tType, trueStride, (const char *)pointer);
    } else {
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (cEnable) {
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(cSize, cType, trueStride, (const char *)pointer+cOffset);
    } else {
	glDisableClientState(GL_COLOR_ARRAY);
    }
    if (nEnable) {
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(nType, trueStride, (const char *)pointer+nOffset);
    } else {
	glDisableClientState(GL_NORMAL_ARRAY);
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(vSize, vType, trueStride, (const char *)pointer+vOffset);
}

/*****************************************************************************/

void glArrayElement(GLint i)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;

    if (va->edgeFlagEnable == GL_TRUE) {
	(*va->edgeFlagCall)(va->edgeFlagPtr+i*va->edgeFlagSkip);
    }

    if (va->texCoordEnable == GL_TRUE) {
	(*va->texCoordCall)(va->texCoordPtr+i*va->texCoordSkip);
    }

    if (va->colorEnable == GL_TRUE) {
	(*va->colorCall)(va->colorPtr+i*va->colorSkip);
    }

    if (va->indexEnable == GL_TRUE) {
	(*va->indexCall)(va->indexPtr+i*va->indexSkip);
    }

    if (va->normalEnable == GL_TRUE) {
	(*va->normalCall)(va->normalPtr+i*va->normalSkip);
    }

    if (va->vertexEnable == GL_TRUE) {
	(*va->vertexCall)(va->vertexPtr+i*va->vertexSkip);
    }
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;
    const char *vaPtr = NULL, *naPtr = NULL, *caPtr = NULL, 
               *iaPtr = NULL, *tcaPtr = NULL;
    const GLboolean *efaPtr = NULL;
    GLint i;

    switch(mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_QUAD_STRIP:
      case GL_QUADS:
      case GL_POLYGON:
	break;
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    if (count < 0) {
	__glXSetError(gc, GL_INVALID_VALUE);
	return;
    } 

    /*
    ** Set up pointers for quick array traversal.
    */
    if (va->normalEnable == GL_TRUE) 
	naPtr = va->normalPtr + first * va->normalSkip;
    if (va->colorEnable == GL_TRUE) 
	caPtr = va->colorPtr + first * va->colorSkip;
    if (va->indexEnable == GL_TRUE) 
	iaPtr = va->indexPtr + first * va->indexSkip;
    if (va->texCoordEnable == GL_TRUE) 
	tcaPtr = va->texCoordPtr + first * va->texCoordSkip;
    if (va->edgeFlagEnable == GL_TRUE) 
	efaPtr = va->edgeFlagPtr + first * va->edgeFlagSkip;
    if (va->vertexEnable == GL_TRUE)
	vaPtr = va->vertexPtr + first * va->vertexSkip;

    glBegin(mode);
        for (i = 0; i < count; i++) {
            if (va->edgeFlagEnable == GL_TRUE) {
                (*va->edgeFlagCall)(efaPtr);
                efaPtr += va->edgeFlagSkip;
            }
            if (va->texCoordEnable == GL_TRUE) {
                (*va->texCoordCall)(tcaPtr);
                tcaPtr += va->texCoordSkip;
            }
            if (va->colorEnable == GL_TRUE) {
                (*va->colorCall)(caPtr);
                caPtr += va->colorSkip;
            }
            if (va->indexEnable == GL_TRUE) {
                (*va->indexCall)(iaPtr);
                iaPtr += va->indexSkip;
            }
            if (va->normalEnable == GL_TRUE) {
                (*va->normalCall)(naPtr);
                naPtr += va->normalSkip;
            }
            if (va->vertexEnable == GL_TRUE) {
                (*va->vertexCall)(vaPtr);
                vaPtr += va->vertexSkip;
        }
    }
    glEnd();
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type,
		    const GLvoid *indices)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    __GLXvertArrayState *va = &gc->state.vertArray;
    const GLubyte *iPtr1 = NULL;
    const GLushort *iPtr2 = NULL;
    const GLuint *iPtr3 = NULL;
    GLint i, offset = 0;

    switch (mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_QUAD_STRIP:
      case GL_QUADS:
      case GL_POLYGON:
	break;
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    if (count < 0) {
	__glXSetError(gc, GL_INVALID_VALUE);
	return;
    } 

    switch (type) {
      case GL_UNSIGNED_BYTE:
	iPtr1 = (const GLubyte *)indices;
	break;
      case GL_UNSIGNED_SHORT:
	iPtr2 = (const GLushort *)indices;
	break;
      case GL_UNSIGNED_INT:
	iPtr3 = (const GLuint *)indices;
	break;
      default:
        __glXSetError(gc, GL_INVALID_ENUM);
        return;
    }

    glBegin(mode);
        for (i = 0; i < count; i++) {
	    switch (type) {
	      case GL_UNSIGNED_BYTE:
		offset = (GLint)(*iPtr1++);
		break;
	      case GL_UNSIGNED_SHORT:
		offset = (GLint)(*iPtr2++);
		break;
	      case GL_UNSIGNED_INT:
		offset = (GLint)(*iPtr3++);
		break;
	    }
            if (va->edgeFlagEnable == GL_TRUE) {
                (*va->edgeFlagCall)(va->edgeFlagPtr+(offset*va->edgeFlagSkip));
            }
            if (va->texCoordEnable == GL_TRUE) {
                (*va->texCoordCall)(va->texCoordPtr+(offset*va->texCoordSkip));
            }
            if (va->colorEnable == GL_TRUE) {
                (*va->colorCall)(va->colorPtr+(offset*va->colorSkip));
            }
            if (va->indexEnable == GL_TRUE) {
                (*va->indexCall)(va->indexPtr+(offset*va->indexSkip));
            }
            if (va->normalEnable == GL_TRUE) {
                (*va->normalCall)(va->normalPtr+(offset*va->normalSkip));
            }
            if (va->vertexEnable == GL_TRUE) {
                (*va->vertexCall)(va->vertexPtr+(offset*va->vertexSkip));
        }
    }
    glEnd();
}
