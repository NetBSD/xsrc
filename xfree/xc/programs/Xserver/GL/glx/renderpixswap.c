/* $XFree86: xc/programs/Xserver/GL/glx/renderpixswap.c,v 1.4 1999/07/18 08:34:24 dawes Exp $ */
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
#include "unpack.h"
#include "g_disptab.h"

void __glXDispSwap_PolygonStipple(GLbyte *pc)
{
    __GLXpixelHeader *hdr = (__GLXpixelHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);

    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glPolygonStipple((GLubyte *)(hdr+1));
}

void __glXDispSwap_Bitmap(GLbyte *pc)
{
    __GLXdispatchBitmapHeader *hdr = (__GLXdispatchBitmapHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);
    
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->xorig);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->yorig);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->xmove);
    __GLX_SWAP_FLOAT((GLbyte *)&hdr->ymove);
    
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glBitmap((GLsizei) hdr->width, 
	     (GLsizei) hdr->height, 
	     (GLfloat) hdr->xorig, 
	     (GLfloat) hdr->yorig, 
	     (GLfloat) hdr->xmove,
	     (GLfloat) hdr->ymove, 
	     (GLubyte *)(hdr+1));
}

void __glXDispSwap_TexImage1D(GLbyte *pc)
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);
    
    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->components);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->border);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);
    
    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    glPixelStorei(GL_UNPACK_SWAP_BYTES, !hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexImage1D(hdr->target, 
		 (GLint) hdr->level, 
		 (GLint) hdr->components, 
		 (GLsizei) hdr->width,
		 (GLint) hdr->border, 
		 hdr->format, 
		 hdr->type, 
		 (GLvoid *)(hdr+1));
}

void __glXDispSwap_TexImage2D(GLbyte *pc)
{
    __GLXdispatchTexImageHeader *hdr = (__GLXdispatchTexImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);
    
    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->components);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->border);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);
    
    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    glPixelStorei(GL_UNPACK_SWAP_BYTES, !hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexImage2D(hdr->target, 
		 (GLint) hdr->level, 
		 (GLint) hdr->components, 
		 (GLsizei) hdr->width,
		 (GLsizei) hdr->height, 
		 (GLint) hdr->border, 
		 hdr->format, 
		 hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDispSwap_DrawPixels(GLbyte *pc)
{
    __GLXdispatchDrawPixelsHeader *hdr = (__GLXdispatchDrawPixelsHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);
    
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);
    
    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    glPixelStorei(GL_UNPACK_SWAP_BYTES, !hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glDrawPixels((GLsizei) hdr->width, 
		 (GLsizei) hdr->height, 
		 hdr->format, 
		 hdr->type,
		 (GLvoid *)(hdr+1));
}

void __glXDispSwap_TexSubImage1D(GLbyte *pc)
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);
    
    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->xoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);
    
    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    glPixelStorei(GL_UNPACK_SWAP_BYTES, !hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexSubImage1D(hdr->target, 
		    (GLint) hdr->level, 
		    (GLint) hdr->xoffset, 
		    (GLsizei) hdr->width,
		    hdr->format, 
		    hdr->type, 
		    (GLvoid *)(hdr+1));
}

void __glXDispSwap_TexSubImage2D(GLbyte *pc)
{
    __GLXdispatchTexSubImageHeader *hdr = (__GLXdispatchTexSubImageHeader *) pc;
    __GLX_DECLARE_SWAP_VARIABLES;

    __GLX_SWAP_INT((GLbyte *)&hdr->rowLength);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipRows);
    __GLX_SWAP_INT((GLbyte *)&hdr->skipPixels);
    __GLX_SWAP_INT((GLbyte *)&hdr->alignment);
    
    __GLX_SWAP_INT((GLbyte *)&hdr->target);
    __GLX_SWAP_INT((GLbyte *)&hdr->level);
    __GLX_SWAP_INT((GLbyte *)&hdr->xoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->yoffset);
    __GLX_SWAP_INT((GLbyte *)&hdr->width);
    __GLX_SWAP_INT((GLbyte *)&hdr->height);
    __GLX_SWAP_INT((GLbyte *)&hdr->format);
    __GLX_SWAP_INT((GLbyte *)&hdr->type);
    
    /*
    ** Just invert swapBytes flag; the GL will figure out if it needs to swap
    ** the pixel data.
    */
    glPixelStorei(GL_UNPACK_SWAP_BYTES, !hdr->swapBytes);
    glPixelStorei(GL_UNPACK_LSB_FIRST, hdr->lsbFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) hdr->rowLength);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, (GLint) hdr->skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, (GLint) hdr->skipPixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) hdr->alignment);

    glTexSubImage2D(hdr->target, 
		    (GLint) hdr->level, 
		    (GLint) hdr->xoffset, 
		    (GLint) hdr->yoffset,
		    (GLsizei) hdr->width, 
		    (GLsizei) hdr->height, 
		    hdr->format, 
		    hdr->type,
		    (GLvoid *)(hdr+1));
}
