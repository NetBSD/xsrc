/* $XFree86: xc/lib/GL/glx/pixelstore.c,v 1.2 1999/06/14 07:23:39 dawes Exp $ */
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

#include "packsingle.h"

/*
** Specify parameters that control the storage format of pixel arrays.
*/
void glPixelStoref(GLenum pname, GLfloat param)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;
    GLuint a;

    if (!dpy) return;

    switch (pname) {
      case GL_PACK_ROW_LENGTH:
	a = (GLuint) (param + 0.5);
	if (((GLint) a) < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storePack.rowLength = a;
	break;
      case GL_PACK_SKIP_ROWS:
	a = (GLuint) (param + 0.5);
	if (((GLint) a) < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storePack.skipRows = a;
	break;
      case GL_PACK_SKIP_PIXELS:
	a = (GLuint) (param + 0.5);
	if (((GLint) a) < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storePack.skipPixels = a;
	break;
      case GL_PACK_ALIGNMENT:
	a = (GLint) (param + 0.5);
	switch (a) {
	  case 1: case 2: case 4: case 8:
	    gc->state.storePack.alignment = a;
	    break;
	  default:
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	break;
      case GL_PACK_SWAP_BYTES:
	gc->state.storePack.swapEndian = (param != 0);
	break;
      case GL_PACK_LSB_FIRST:
	gc->state.storePack.lsbFirst = (param != 0);
	break;

      case GL_UNPACK_ROW_LENGTH:
	a = (GLuint) (param + 0.5);
	if (((GLint) a) < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storeUnpack.rowLength = a;
	break;
      case GL_UNPACK_SKIP_ROWS:
	a = (GLuint) (param + 0.5);
	if (((GLint) a) < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storeUnpack.skipRows = a;
	break;
      case GL_UNPACK_SKIP_PIXELS:
	a = (GLuint) (param + 0.5);
	if (((GLint) a) < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storeUnpack.skipPixels = a;
	break;
      case GL_UNPACK_ALIGNMENT:
	a = (GLint) (param + 0.5);
	switch (a) {
	  case 1: case 2: case 4: case 8:
	    gc->state.storeUnpack.alignment = a;
	    break;
	  default:
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	break;
      case GL_UNPACK_SWAP_BYTES:
	gc->state.storeUnpack.swapEndian = (param != 0);
	break;
      case GL_UNPACK_LSB_FIRST:
	gc->state.storeUnpack.lsbFirst = (param != 0);
	break;
      default:
	/*
	** NOTE: there are currently no pixel storage commands that need to
	** be sent to the server.  This may change in future versions
	** of the API, however.
	*/
	__glXSetError(gc, GL_INVALID_ENUM);
	break;
    }
}

void glPixelStorei(GLenum pname, GLint param)
{
    __GLXcontext *gc = __glXGetCurrentContext();
    Display *dpy = gc->currentDpy;

    if (!dpy) return;

    switch (pname) {
      case GL_PACK_ROW_LENGTH:
	if (param < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storePack.rowLength = param;
	break;
      case GL_PACK_SKIP_ROWS:
	if (param < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storePack.skipRows = param;
	break;
      case GL_PACK_SKIP_PIXELS:
	if (param < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storePack.skipPixels = param;
	break;
      case GL_PACK_ALIGNMENT:
	switch (param) {
	  case 1: case 2: case 4: case 8:
	    gc->state.storePack.alignment = param;
	    break;
	  default:
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	break;
      case GL_PACK_SWAP_BYTES:
	gc->state.storePack.swapEndian = (param != 0);
	break;
      case GL_PACK_LSB_FIRST:
	gc->state.storePack.lsbFirst = (param != 0);
	break;

      case GL_UNPACK_ROW_LENGTH:
	if (param < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storeUnpack.rowLength = param;
	break;
      case GL_UNPACK_SKIP_ROWS:
	if (param < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storeUnpack.skipRows = param;
	break;
      case GL_UNPACK_SKIP_PIXELS:
	if (param < 0) {
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	gc->state.storeUnpack.skipPixels = param;
	break;
      case GL_UNPACK_ALIGNMENT:
	switch (param) {
	  case 1: case 2: case 4: case 8:
	    gc->state.storeUnpack.alignment = param;
	    break;
	  default:
	    __glXSetError(gc, GL_INVALID_VALUE);
	    return;
	}
	break;
      case GL_UNPACK_SWAP_BYTES:
	gc->state.storeUnpack.swapEndian = (param != 0);
	break;
      case GL_UNPACK_LSB_FIRST:
	gc->state.storeUnpack.lsbFirst = (param != 0);
	break;
      default:
	/*
	** NOTE: there are currently no pixel storage commands that need to
	** be sent to the server.  This may change in future versions
	** of the API, however.
	*/
	__glXSetError(gc, GL_INVALID_ENUM);
	break;
    }
}
