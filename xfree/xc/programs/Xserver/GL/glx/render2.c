/* $XFree86: xc/programs/Xserver/GL/glx/render2.c,v 1.4 1999/07/18 08:34:24 dawes Exp $ */
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

/* #define NEED_REPLIES */
#include "glxserver.h"
#include "unpack.h"
#include "g_disptab.h"

extern GLint __glEvalComputeK(GLenum target);

void __glXDisp_Map1f(GLbyte *pc)
{
    GLint order, k;
    GLfloat u1, u2, *points;
    GLenum target;

    target = *(GLenum *)(pc + 0); 
    order = *(GLint *)(pc + 12);
    u1 = *(GLfloat *)(pc + 4);
    u2 = *(GLfloat *)(pc + 8);
    points = (GLfloat *)(pc + 16);
    k = __glEvalComputeK(target);

    glMap1f(target, u1, u2, k, order, points);
}

void __glXDisp_Map2f(GLbyte *pc)
{
    GLint uorder, vorder, ustride, vstride, k;
    GLfloat u1, u2, v1, v2, *points;
    GLenum target;
    
    target = *(GLenum *)(pc + 0); 
    uorder = *(GLint *)(pc + 12);
    vorder = *(GLint *)(pc + 24);
    u1 = *(GLfloat *)(pc + 4);
    u2 = *(GLfloat *)(pc + 8);
    v1 = *(GLfloat *)(pc + 16);
    v2 = *(GLfloat *)(pc + 20);
    points = (GLfloat *)(pc + 28);

    k = __glEvalComputeK(target);
    ustride = vorder * k;
    vstride = k;

    glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

void __glXDisp_Map1d(GLbyte *pc)
{
    GLint order, k, compsize;
    GLenum target;
    GLdouble u1, u2, *points;

    target = *(GLenum*) (pc + 16);
    order = *(GLint*) (pc + 20);
    k = __glEvalComputeK(target);
    if (order < 0 || k < 0) {
	compsize = 0;
    } else {
	compsize = order * k;
    }
    __GLX_GET_DOUBLE(u1,pc);
    __GLX_GET_DOUBLE(u2,pc+8);
    pc += 24;

#ifdef __GLX_ALIGN64
    if (((unsigned long)pc) & 7) {
	/*
	** Copy the doubles up 4 bytes, trashing the command but aligning
	** the data in the process
	*/
	__GLX_MEM_COPY(pc-4, pc, compsize*8);
	points = (GLdouble*) (pc - 4);
    } else {
	points = (GLdouble*) pc;
    }
#else
    points = (GLdouble*) pc;
#endif
    glMap1d(target, u1, u2, k, order, points);
}

void __glXDisp_Map2d(GLbyte *pc)
{
    GLdouble u1, u2, v1, v2, *points;
    GLint uorder, vorder, ustride, vstride, k, compsize;
    GLenum target;

    target = *(GLenum *)(pc + 32);
    uorder = *(GLint *)(pc + 36);
    vorder = *(GLint *)(pc + 40);
    k = __glEvalComputeK(target);
    if (vorder < 0 || uorder < 0 || k < 0) {
	compsize = 0;
    } else {
	compsize = uorder * vorder * k;
    }
    __GLX_GET_DOUBLE(u1,pc);
    __GLX_GET_DOUBLE(u2,pc+8);
    __GLX_GET_DOUBLE(v1,pc+16);
    __GLX_GET_DOUBLE(v2,pc+24);
    pc += 44;

    ustride = vorder * k;
    vstride = k;

#ifdef __GLX_ALIGN64
    if (((unsigned long)pc) & 7) {
	/*
	** Copy the doubles up 4 bytes, trashing the command but aligning
	** the data in the process
	*/
	__GLX_MEM_COPY(pc-4, pc, compsize*8);
	points = (GLdouble*) (pc - 4);
    } else {
	points = (GLdouble*) pc;
    }
#else
    points = (GLdouble*) pc;
#endif
    glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

void __glXDisp_CallLists(GLbyte *pc)
{
    GLenum type;
    GLsizei n;

    type = *(GLenum *)(pc + 4);
    n = *(GLsizei *)(pc + 0);
    glCallLists(n, type, pc + 8);
}

void __glXDisp_DrawArrays(GLbyte *pc)
{
    __GLXdispatchDrawArraysHeader *hdr = (__GLXdispatchDrawArraysHeader *)pc;
    __GLXdispatchDrawArraysComponentHeader *compHeader;
    GLint numVertexes = hdr->numVertexes;
    GLint numComponents = hdr->numComponents;
    GLenum primType = hdr->primType;
    GLint stride = 0;
    int i;

    pc += sizeof(__GLXdispatchDrawArraysHeader);
    compHeader = (__GLXdispatchDrawArraysComponentHeader *)pc;

    /* compute stride (same for all component arrays) */
    for (i = 0; i < numComponents; i++) {
	GLenum datatype = compHeader[i].datatype;
	GLint numVals = compHeader[i].numVals;

        stride += __GLX_PAD(numVals * __glXTypeSize(datatype));
    }

    pc += numComponents * sizeof(__GLXdispatchDrawArraysComponentHeader);

    /* set up component arrays */
    for (i = 0; i < numComponents; i++) {
	GLenum datatype = compHeader[i].datatype;
	GLint numVals = compHeader[i].numVals;
	GLenum component = compHeader[i].component;

        switch (component) {
          case GL_VERTEX_ARRAY:
	    glEnableClientState(GL_VERTEX_ARRAY);
	    glVertexPointer(numVals, datatype, stride, pc);
            break;
          case GL_NORMAL_ARRAY:
	    glEnableClientState(GL_NORMAL_ARRAY);
	    glNormalPointer(datatype, stride, pc);
            break;
          case GL_COLOR_ARRAY:
	    glEnableClientState(GL_COLOR_ARRAY);
	    glColorPointer(numVals, datatype, stride, pc);
            break;
          case GL_INDEX_ARRAY:
	    glEnableClientState(GL_INDEX_ARRAY);
	    glIndexPointer(datatype, stride, pc);
            break;
          case GL_TEXTURE_COORD_ARRAY:
	    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	    glTexCoordPointer(numVals, datatype, stride, pc);
            break;
          case GL_EDGE_FLAG_ARRAY:
	    glEnableClientState(GL_EDGE_FLAG_ARRAY);
	    glEdgeFlagPointer(stride, (const GLboolean *)pc);
            break;
          default:
            break;
	}

        pc += __GLX_PAD(numVals * __glXTypeSize(datatype));
    }

    glDrawArrays(primType, 0, numVertexes);

    /* turn off anything we might have turned on */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
}
