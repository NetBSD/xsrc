/* $XFree86: xc/programs/Xserver/GL/glx/singlepix.c,v 1.4 1999/07/18 08:34:25 dawes Exp $ */
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

#define NEED_REPLIES
#include "glxserver.h"
#include "glxext.h"
#include "singlesize.h"
#include "unpack.h"
#include "g_disptab.h"

int __glXDisp_ReadPixels(__GLXclientState *cl, GLbyte *pc)
{
    GLsizei width, height;
    GLenum format, type;
    GLboolean swapBytes, lsbFirst;
    GLint compsize;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    width = *(GLsizei *)(pc + 8);
    height = *(GLsizei *)(pc + 12);
    format = *(GLenum *)(pc + 16);
    type = *(GLenum *)(pc + 20);
    swapBytes = *(GLboolean *)(pc + 24);
    lsbFirst = *(GLboolean *)(pc + 25);
    compsize = __glReadPixels_size(format,type,width,height);
    if (compsize < 0) compsize = 0;

    glPixelStorei(GL_PACK_SWAP_BYTES, swapBytes);
    glPixelStorei(GL_PACK_LSB_FIRST, lsbFirst);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glReadPixels( 
		 *(GLint    *)(pc + 0),
		 *(GLint    *)(pc + 4),
		 *(GLsizei  *)(pc + 8),
		 *(GLsizei  *)(pc + 12),
		 *(GLenum   *)(pc + 16),
		 *(GLenum   *)(pc + 20),
		 answer
		 );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }
    return Success;
}

int __glXDisp_GetTexImage(__GLXclientState *cl, GLbyte *pc)
{
    GLint level, compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0, height=0;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    level = *(GLint *)(pc + 4);
    format = *(GLenum *)(pc + 8);
    type = *(GLenum *)(pc + 12);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 16);
    
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
    /*
     * The two queries above might fail if we're in a state where queries
     * are illegal, but then width and height would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,level,format,type,width,height);
    if (compsize < 0) compsize = 0;
    
    glPixelStorei(GL_PACK_SWAP_BYTES, swapBytes);
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    glGetTexImage( 
		  *(GLenum   *)(pc + 0),
		  *(GLint    *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  *(GLenum   *)(pc + 12),
		  answer
		  );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	((xGLXGetTexImageReply *)&__glXReply)->width = width;
	((xGLXGetTexImageReply *)&__glXReply)->height = height;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }
    return Success;
}

int __glXDisp_GetPolygonStipple(__GLXclientState *cl, GLbyte *pc)
{
    GLboolean lsbFirst;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    GLubyte answerBuffer[200];
    char *answer;

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }
    pc += __GLX_SINGLE_HDR_SIZE;
    lsbFirst = *(GLboolean *)(pc + 0);

    glPixelStorei(GL_PACK_LSB_FIRST, lsbFirst);
    __GLX_GET_ANSWER_BUFFER(answer,cl,128,1);
    
    __glXClearErrorOccured();
    glGetPolygonStipple( 
			(GLubyte  *) answer
			);
    
    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(128);
	__GLX_SEND_HEADER();
	__GLX_SEND_BYTE_ARRAY(128);
    }
    return Success;
}


