/* $XFree86: xc/lib/GL/glx/renderpix.c,v 1.2 1999/06/14 07:23:39 dawes Exp $ */
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
** This file contains routines that deal with unpacking data from client
** memory using the pixel store unpack modes and then shipping it to
** the server.  For all of these routines (except glPolygonStipple) there
** are two forms of the transport - small and large.  Small commands are
** the commands that fit into the "rendering" transport buffer.  Large
** commands are sent to the server in chunks by __glXSendLargeCommand.
**
** All of the commands send over a pixel header (see glxproto.h) which
** describes the pixel store modes that the server must use to properly
** handle the data.  Any pixel store modes not done by the __glFillImage
** routine are passed on to the server.
*/

/*
** Send a large image to the server.  If necessary, a buffer is allocated
** to hold the unpacked data that is copied from the clients memory.
*/
static void SendLargeImage(__GLXcontext *gc,
			   GLint compsize, GLint width, GLint height,
			   GLenum format, GLenum type, const GLvoid *src,
			   GLubyte *pc, GLubyte *modes)
{
    if (!gc->fastImageUnpack) {
	/* Allocate a temporary holding buffer */
	GLubyte *buf = (GLubyte *) Xmalloc(compsize);
	if (!buf) {
	    __glXSetError(gc, GL_OUT_OF_MEMORY);
	    return;
	}

	/* Apply pixel store unpack modes to copy data into buf */
	(*gc->fillImage)(gc, width, height, format, type, src, buf, modes);

	/* Send large command */
	__glXSendLargeCommand(gc, gc->pc, pc - gc->pc, buf, compsize);

	/* Free buffer */
	Xfree((char*) buf);
    } else {
	/* Just send the data straight as is */
	__glXSendLargeCommand(gc, gc->pc, pc - gc->pc, pc, compsize);
    }
}

/*
** Send a large null image to the server.  To be backwards compatible,
** data must be sent to the server even when the application has passed
** a null pointer into glTexImage1D or glTexImage2D.
*/
static void SendLargeNULLImage(__GLXcontext *gc,
			       GLint compsize, GLint width, GLint height,
			       GLenum format, GLenum type, const GLvoid *src,
			       GLubyte *pc, GLubyte *modes)
{
    GLubyte *buf = (GLubyte *) Xmalloc(compsize);

    /* Allocate a temporary holding buffer */
    if (!buf) {
	__glXSetError(gc, GL_OUT_OF_MEMORY);
	return;
    }

    /* Send large command */
    __glXSendLargeCommand(gc, gc->pc, pc - gc->pc, buf, compsize);

    /* Free buffer */
    Xfree((char*) buf);
}

/************************************************************************/

void glPolygonStipple(const GLubyte *mask)
{
    __GLX_DECLARE_VARIABLES();

    compsize = __glImageSize(32, 32, GL_COLOR_INDEX, GL_BITMAP);
    cmdlen = __GLX_PAD(__GLX_POLYGONSTIPPLE_CMD_HDR_SIZE + compsize);
    __GLX_LOAD_VARIABLES();
    if (!gc->currentDpy) return;

    __GLX_BEGIN(X_GLrop_PolygonStipple,cmdlen);
    pc += __GLX_RENDER_HDR_SIZE;
    pixelHeaderPC = pc;
    pc += __GLX_PIXEL_HDR_SIZE;
    (*gc->fillImage)(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP,
		     mask, pc, pixelHeaderPC);
    __GLX_END(__GLX_PAD(compsize));
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
	      GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    __GLX_DECLARE_VARIABLES();

    compsize = __glImageSize(width, height, GL_COLOR_INDEX, GL_BITMAP);
    cmdlen = __GLX_PAD(__GLX_BITMAP_CMD_HDR_SIZE + compsize);
    __GLX_LOAD_VARIABLES();
    if (!gc->currentDpy) return;

    if (cmdlen <= gc->maxSmallRenderCommandSize) {
	/* Use GLXRender protocol to send small command */
	__GLX_BEGIN_VARIABLE_WITH_PIXEL(X_GLrop_Bitmap,cmdlen);
	__GLX_PUT_LONG(0,width);
	__GLX_PUT_LONG(4,height);
	__GLX_PUT_FLOAT(8,xorig);
	__GLX_PUT_FLOAT(12,yorig);
	__GLX_PUT_FLOAT(16,xmove);
	__GLX_PUT_FLOAT(20,ymove);
	pc += __GLX_BITMAP_HDR_SIZE;
	if (compsize > 0) {
	    (*gc->fillImage)(gc, width, height, GL_COLOR_INDEX, GL_BITMAP,
			     bitmap, pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    GLubyte *pc = pixelHeaderPC;
	    __GLX_PUT_CHAR(0,GL_FALSE);
	    __GLX_PUT_CHAR(1,GL_FALSE);
	    __GLX_PUT_CHAR(2,0);
	    __GLX_PUT_CHAR(3,0);
	    __GLX_PUT_LONG(4,0);
	    __GLX_PUT_LONG(8,0);
	    __GLX_PUT_LONG(12,0);
	    __GLX_PUT_LONG(16,1);
	}
	__GLX_END(__GLX_PAD(compsize));
    } else {
	/* Use GLXRenderLarge protocol to send command */
	__GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(X_GLrop_Bitmap,cmdlen+4);
	__GLX_PUT_LONG(0,width);
	__GLX_PUT_LONG(4,height);
	__GLX_PUT_FLOAT(8,xorig);
	__GLX_PUT_FLOAT(12,yorig);
	__GLX_PUT_FLOAT(16,xmove);
	__GLX_PUT_FLOAT(20,ymove);
	pc += __GLX_BITMAP_HDR_SIZE;
	SendLargeImage(gc, compsize, width, height, GL_COLOR_INDEX,
		       GL_BITMAP, bitmap, pc, pixelHeaderPC);
    }
}

void glTexImage1D(GLenum target, GLint level, GLint components,
		  GLsizei width, GLint border, GLenum format, GLenum type,
		  const GLvoid *image)
{
    __GLX_DECLARE_VARIABLES();

    if (target == GL_PROXY_TEXTURE_1D) {
	compsize = 0;
    } else {
	compsize = __glImageSize(width, 1, format, type);
    }
    cmdlen = __GLX_PAD(__GLX_TEXIMAGE_CMD_HDR_SIZE + compsize);
    __GLX_LOAD_VARIABLES();
    if (!gc->currentDpy) return;

    if (cmdlen <= gc->maxSmallRenderCommandSize) {
	/* Use GLXRender protocol to send small command */
	__GLX_BEGIN_VARIABLE_WITH_PIXEL(X_GLrop_TexImage1D,cmdlen);
	__GLX_PUT_LONG(0,target);
	__GLX_PUT_LONG(4,level);
	__GLX_PUT_LONG(8,components);
	__GLX_PUT_LONG(12,width);
	__GLX_PUT_LONG(20,border);
	__GLX_PUT_LONG(24,format);
	__GLX_PUT_LONG(28,type);
	pc += __GLX_TEXIMAGE_HDR_SIZE;
	if (compsize > 0 && image != NULL) {
	    (*gc->fillImage)(gc, width, 1, format, type,
			     image, pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    GLubyte *pc = pixelHeaderPC;
	    __GLX_PUT_CHAR(0,GL_FALSE);
	    __GLX_PUT_CHAR(1,GL_FALSE);
	    __GLX_PUT_CHAR(2,0);
	    __GLX_PUT_CHAR(3,0);
	    __GLX_PUT_LONG(4,0);
	    __GLX_PUT_LONG(8,0);
	    __GLX_PUT_LONG(12,0);
	    __GLX_PUT_LONG(16,1);
	}
	__GLX_END(__GLX_PAD(compsize));
    } else {
	/* Use GLXRenderLarge protocol to send command */
	__GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(X_GLrop_TexImage1D,cmdlen+4);
	__GLX_PUT_LONG(0,target);
	__GLX_PUT_LONG(4,level);
	__GLX_PUT_LONG(8,components);
	__GLX_PUT_LONG(12,width);
	__GLX_PUT_LONG(16,1);
	__GLX_PUT_LONG(20,border);
	__GLX_PUT_LONG(24,format);
	__GLX_PUT_LONG(28,type);
	pc += __GLX_TEXIMAGE_HDR_SIZE;
	if (image != NULL) {
	    SendLargeImage(gc, compsize, width, 1, format,
			   type, image, pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    {
		GLubyte *pc = pixelHeaderPC;
		__GLX_PUT_CHAR(0,GL_FALSE);
		__GLX_PUT_CHAR(1,GL_FALSE);
		__GLX_PUT_CHAR(2,0);
		__GLX_PUT_CHAR(3,0);
		__GLX_PUT_LONG(4,0);
		__GLX_PUT_LONG(8,0);
		__GLX_PUT_LONG(12,0);
		__GLX_PUT_LONG(16,1);
	    }
	    SendLargeNULLImage(gc, compsize, width, 1, format,
			       type, image, pc, pixelHeaderPC);
	}
    }
}

void glTexImage2D(GLenum target, GLint level, GLint components,
		  GLsizei width, GLsizei height, GLint border, GLenum format,
		  GLenum type, const GLvoid *image)
{
    __GLX_DECLARE_VARIABLES();

    if (target == GL_PROXY_TEXTURE_2D) {
	compsize = 0;
    } else {
	compsize = __glImageSize(width, height, format, type);
    }
    cmdlen = __GLX_PAD(__GLX_TEXIMAGE_CMD_HDR_SIZE + compsize);
    __GLX_LOAD_VARIABLES();
    if (!gc->currentDpy) return;

    if (cmdlen <= gc->maxSmallRenderCommandSize) {
	/* Use GLXRender protocol to send small command */
	__GLX_BEGIN_VARIABLE_WITH_PIXEL(X_GLrop_TexImage2D,cmdlen);
	__GLX_PUT_LONG(0,target);
	__GLX_PUT_LONG(4,level);
	__GLX_PUT_LONG(8,components);
	__GLX_PUT_LONG(12,width);
	__GLX_PUT_LONG(16,height);
	__GLX_PUT_LONG(20,border);
	__GLX_PUT_LONG(24,format);
	__GLX_PUT_LONG(28,type);
	pc += __GLX_TEXIMAGE_HDR_SIZE;
	if (compsize > 0 && image != NULL) {
	    (*gc->fillImage)(gc, width, height, format, type,
			     image, pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    GLubyte *pc = pixelHeaderPC;
	    __GLX_PUT_CHAR(0,GL_FALSE);
	    __GLX_PUT_CHAR(1,GL_FALSE);
	    __GLX_PUT_CHAR(2,0);
	    __GLX_PUT_CHAR(3,0);
	    __GLX_PUT_LONG(4,0);
	    __GLX_PUT_LONG(8,0);
	    __GLX_PUT_LONG(12,0);
	    __GLX_PUT_LONG(16,1);
	}
	__GLX_END(__GLX_PAD(compsize));
    } else {
	/* Use GLXRenderLarge protocol to send command */
	__GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(X_GLrop_TexImage2D,cmdlen+4);
	__GLX_PUT_LONG(0,target);
	__GLX_PUT_LONG(4,level);
	__GLX_PUT_LONG(8,components);
	__GLX_PUT_LONG(12,width);
	__GLX_PUT_LONG(16,height);
	__GLX_PUT_LONG(20,border);
	__GLX_PUT_LONG(24,format);
	__GLX_PUT_LONG(28,type);
	pc += __GLX_TEXIMAGE_HDR_SIZE;
	if (image != NULL) {
	    SendLargeImage(gc, compsize, width, height, format,
			   type, image, pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    {
		GLubyte *pc = pixelHeaderPC;
		__GLX_PUT_CHAR(0,GL_FALSE);
		__GLX_PUT_CHAR(1,GL_FALSE);
		__GLX_PUT_CHAR(2,0);
		__GLX_PUT_CHAR(3,0);
		__GLX_PUT_LONG(4,0);
		__GLX_PUT_LONG(8,0);
		__GLX_PUT_LONG(12,0);
		__GLX_PUT_LONG(16,1);
	    }
	    SendLargeNULLImage(gc, compsize, width, height, format,
			       type, image, pc, pixelHeaderPC);
	}
    }
}

void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
		  const GLvoid *image)
{
    __GLX_DECLARE_VARIABLES();

    compsize = __glImageSize(width, height, format, type);
    cmdlen = __GLX_PAD(__GLX_DRAWPIXELS_CMD_HDR_SIZE + compsize);
    __GLX_LOAD_VARIABLES();
    if (!gc->currentDpy) return;

    if (cmdlen <= gc->maxSmallRenderCommandSize) {
	/* Use GLXRender protocol to send small command */
	__GLX_BEGIN_VARIABLE_WITH_PIXEL(X_GLrop_DrawPixels,cmdlen);
	__GLX_PUT_LONG(0,width);
	__GLX_PUT_LONG(4,height);
	__GLX_PUT_LONG(8,format);
	__GLX_PUT_LONG(12,type);
	pc += __GLX_DRAWPIXELS_HDR_SIZE;
	if (compsize > 0) {
	    (*gc->fillImage)(gc, width, height, format, type,
			     image, pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    GLubyte *pc = pixelHeaderPC;
	    __GLX_PUT_CHAR(0,GL_FALSE);
	    __GLX_PUT_CHAR(1,GL_FALSE);
	    __GLX_PUT_CHAR(2,0);
	    __GLX_PUT_CHAR(3,0);
	    __GLX_PUT_LONG(4,0);
	    __GLX_PUT_LONG(8,0);
	    __GLX_PUT_LONG(12,0);
	    __GLX_PUT_LONG(16,1);
	}
	__GLX_END(__GLX_PAD(compsize));
    } else {
	/* Use GLXRenderLarge protocol to send command */
	__GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(X_GLrop_DrawPixels,cmdlen+4);
	__GLX_PUT_LONG(0,width);
	__GLX_PUT_LONG(4,height);
	__GLX_PUT_LONG(8,format);
	__GLX_PUT_LONG(12,type);
	pc += __GLX_DRAWPIXELS_HDR_SIZE;
	SendLargeImage(gc, compsize, width, height, format,
		       type, image, pc, pixelHeaderPC);
    }
}

static void __glx_TexSubImage1D2D(GLshort opcode, GLenum target, GLint level,
			GLint xoffset, GLint yoffset, GLsizei width, 	
			GLsizei height, GLenum format, GLenum type, 	
			const GLvoid *image)
{
    __GLX_DECLARE_VARIABLES();

    if (image == NULL) {
	compsize = 0;
    } else {
        compsize = __glImageSize(width, height, format, type);
    }

    cmdlen = __GLX_PAD(__GLX_TEXSUBIMAGE_CMD_HDR_SIZE + compsize);
    __GLX_LOAD_VARIABLES();
    if (!gc->currentDpy) return;

    if (cmdlen <= gc->maxSmallRenderCommandSize) {
	/* Use GLXRender protocol to send small command */
	__GLX_BEGIN_VARIABLE_WITH_PIXEL(opcode, cmdlen);
	__GLX_PUT_LONG(0,target);
	__GLX_PUT_LONG(4,level);
	__GLX_PUT_LONG(8,xoffset);
	__GLX_PUT_LONG(12,yoffset);
	__GLX_PUT_LONG(16,width);
	__GLX_PUT_LONG(20,height);
	__GLX_PUT_LONG(24,format);
	__GLX_PUT_LONG(28,type);
	if (image == NULL) {
	    __GLX_PUT_LONG(32,GL_TRUE);
	} else {
	    __GLX_PUT_LONG(32,GL_FALSE);
	}
	pc += __GLX_TEXSUBIMAGE_HDR_SIZE;
	if (compsize > 0) {
	    (*gc->fillImage)(gc, width, height, format, type, image, 
					pc, pixelHeaderPC);
	} else {
	    /* Setup default store modes */
	    GLubyte *pc = pixelHeaderPC;
	    __GLX_PUT_CHAR(0,GL_FALSE);
	    __GLX_PUT_CHAR(1,GL_FALSE);
	    __GLX_PUT_CHAR(2,0);
	    __GLX_PUT_CHAR(3,0);
	    __GLX_PUT_LONG(4,0);
	    __GLX_PUT_LONG(8,0);
	    __GLX_PUT_LONG(12,0);
	    __GLX_PUT_LONG(16,1);
	}
	__GLX_END(__GLX_PAD(compsize));
    } else {
	/* Use GLXRenderLarge protocol to send command */
	__GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(opcode,cmdlen+4);
	__GLX_PUT_LONG(0,target);
	__GLX_PUT_LONG(4,level);
	__GLX_PUT_LONG(8,xoffset);
	__GLX_PUT_LONG(12,yoffset);
	__GLX_PUT_LONG(16,width);
	__GLX_PUT_LONG(20,height);
	__GLX_PUT_LONG(24,format);
	__GLX_PUT_LONG(28,type);
	if (image == NULL) {
	    __GLX_PUT_LONG(32,GL_TRUE);
	} else {
	    __GLX_PUT_LONG(32,GL_FALSE);
	}
	pc += __GLX_TEXSUBIMAGE_HDR_SIZE;
	SendLargeImage(gc, compsize, width, height, 
		       format, type, image, pc, pixelHeaderPC);
    }
}
	
void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, 
			GLsizei width, GLenum format, GLenum type,
		      	const GLvoid *image)
{
    __glx_TexSubImage1D2D(X_GLrop_TexSubImage1D, target, level, xoffset, 
			 0, width, 1, format, type, image);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, 
			GLint yoffset, GLsizei width, GLsizei height, 
			GLenum format, GLenum type, const GLvoid *image)
{
    __glx_TexSubImage1D2D(X_GLrop_TexSubImage2D, target, level, xoffset, 
			 yoffset, width, height, format, type, image);
}

