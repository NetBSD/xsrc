#ifndef __GLX_packrender_h__
#define __GLX_packrender_h__

/* $XFree86: xc/lib/GL/glx/packrender.h,v 1.5 2000/02/18 16:23:10 dawes Exp $ */
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

#define NEED_GL_FUNCS_WRAPPED
#include "glxclient.h"

/*
** The macros in this header convert the client machine's native data types to
** wire protocol data types.  The header is part of the porting layer of the
** client library, and it is intended that hardware vendors will rewrite this
** header to suit their own machines.
*/

/*
** Pad a count of bytes to the nearest multiple of 4.  The X protocol
** transfers data in 4 byte quantities, so this macro is used to
** insure the right amount of data being sent.
*/
#define __GLX_PAD(a) (((a)+3) & ~3)

/*
 ** Network size parameters
 */
#define sz_double 8

/* Setup for all commands */
#define __GLX_DECLARE_VARIABLES() \
    __GLXcontext *gc;		  \
    GLubyte *pc, *pixelHeaderPC;  \
    GLuint compsize, cmdlen

#define __GLX_LOAD_VARIABLES()	   \
    gc = __glXGetCurrentContext(); \
    pc = gc->pc;                   \
    (void) pixelHeaderPC;          \
    (void) compsize;               \
    (void) cmdlen

/*
** Variable sized command support macro.  This macro is used by calls
** that are potentially larger than __GLX_SMALL_RENDER_CMD_SIZE.
** Because of their size, they may not automatically fit in the buffer.
** If the buffer can't hold the command then it is flushed so that
** the command will fit in the next buffer.
*/
#define __GLX_BEGIN_VARIABLE(opcode,size)	\
    if (pc + (size) > gc->bufEnd) {		\
	pc = __glXFlushRenderBuffer(gc, pc);	\
    }						\
    __GLX_PUT_SHORT(0,size);			\
    __GLX_PUT_SHORT(2,opcode)

#define __GLX_BEGIN_VARIABLE_LARGE(opcode,size)	\
    pc = __glXFlushRenderBuffer(gc, pc);	\
    __GLX_PUT_LONG(0,size);			\
    __GLX_PUT_LONG(4,opcode)

#define __GLX_BEGIN_VARIABLE_WITH_PIXEL(opcode,size) \
    if (pc + (size) > gc->bufEnd) {		     \
	pc = __glXFlushRenderBuffer(gc, pc);	     \
    }						     \
    __GLX_PUT_SHORT(0,size);			     \
    __GLX_PUT_SHORT(2,opcode);			     \
    pc += __GLX_RENDER_HDR_SIZE;		     \
    pixelHeaderPC = pc;				     \
    pc += __GLX_PIXEL_HDR_SIZE

#define __GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(opcode,size) \
    pc = __glXFlushRenderBuffer(gc, pc);		   \
    __GLX_PUT_LONG(0,size);				   \
    __GLX_PUT_LONG(4,opcode);				   \
    pc += __GLX_RENDER_LARGE_HDR_SIZE;			   \
    pixelHeaderPC = pc;					   \
    pc += __GLX_PIXEL_HDR_SIZE

/*
** Fixed size command support macro.  This macro is used by calls that
** are never larger than __GLX_SMALL_RENDER_CMD_SIZE.  Because they
** always fit in the buffer, and because the buffer promises to
** maintain enough room for them, we don't need to check for space
** before doing the storage work.
*/
#define __GLX_BEGIN(opcode,size) \
    __GLX_PUT_SHORT(0,size);     \
    __GLX_PUT_SHORT(2,opcode)

/*
** Finish a rendering command by advancing the pc.  If the pc is now past
** the limit pointer then there is no longer room for a
** __GLX_SMALL_RENDER_CMD_SIZE sized command, which will break the
** assumptions present in the __GLX_BEGIN macro.  In this case the
** rendering buffer is flushed out into the X protocol stream (which may
** or may not do I/O).
*/
#define __GLX_END(size)			       \
    pc += size;				       \
    if (pc > gc->limit) {		       \
	(void) __glXFlushRenderBuffer(gc, pc); \
    } else {				       \
	gc->pc = pc;			       \
    }

/* Array copy macros */
#define __GLX_MEM_COPY(dest,src,bytes) \
    memcpy(dest, src, bytes)

/* Single item copy macros */
#define __GLX_PUT_CHAR(offset,a) \
    *((INT8 *) (pc + offset)) = a

#ifndef _CRAY
#define __GLX_PUT_SHORT(offset,a) \
    *((INT16 *) (pc + offset)) = a

#define __GLX_PUT_LONG(offset,a) \
    *((INT32 *) (pc + offset)) = a

#define __GLX_PUT_FLOAT(offset,a) \
    *((FLOAT32 *) (pc + offset)) = a

#else
#define __GLX_PUT_SHORT(offset,a) \
 { GLubyte *cp = (pc+offset); \
   int shift = (64-16) - ((int)(cp) >> (64-6)); \
   *(int *)cp = (*(int *)cp & ~(0xffff << shift)) | ((a & 0xffff) << shift); }

#define __GLX_PUT_LONG(offset,a) \
 { GLubyte *cp = (pc+offset); \
   int shift = (64-32) - ((int)(cp) >> (64-6)); \
   *(int *)cp = (*(int *)cp & ~(0xffffffff << shift)) | ((a & 0xffffffff) << shift); }

#define __GLX_PUT_FLOAT(offset,a) \
    gl_put_float((pc + offset),a)

#define __GLX_PUT_DOUBLE(offset,a) \
    gl_put_double(pc + offset, a)

extern void gl_put_float(/*GLubyte *, struct cray_single*/);
extern void gl_put_double(/*GLubyte *, struct cray_double*/);
#endif

#ifndef _CRAY

#ifdef __GLX_ALIGN64
/*
** This can certainly be done better for a particular machine
** architecture!
*/
#define __GLX_PUT_DOUBLE(offset,a) \
    __GLX_MEM_COPY(pc + offset, &a, 8)
#else
#define __GLX_PUT_DOUBLE(offset,a) \
    *((FLOAT64 *) (pc + offset)) = a
#endif

#endif

#define __GLX_PUT_CHAR_ARRAY(offset,a,alen) \
    __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_INT8)

#ifndef _CRAY
#define __GLX_PUT_SHORT_ARRAY(offset,a,alen) \
    __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_INT16)

#define __GLX_PUT_LONG_ARRAY(offset,a,alen) \
    __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_INT32)

#define __GLX_PUT_FLOAT_ARRAY(offset,a,alen) \
    __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_FLOAT32)

#define __GLX_PUT_DOUBLE_ARRAY(offset,a,alen) \
    __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_FLOAT64)

#else
#define __GLX_PUT_SHORT_ARRAY(offset,a,alen) \
    gl_put_short_array((GLubyte *)(pc + offset), a, alen * __GLX_SIZE_INT16)

#define __GLX_PUT_LONG_ARRAY(offset,a,alen) \
    gl_put_long_array((GLubyte *)(pc + offset), (long *)a, alen * __GLX_SIZE_INT32)

#define __GLX_PUT_FLOAT_ARRAY(offset,a,alen) \
    gl_put_float_array((GLubyte *)(pc + offset), (float *)a, alen * __GLX_SIZE_FLOAT32)

#define __GLX_PUT_DOUBLE_ARRAY(offset,a,alen) \
    gl_put_double_array((GLubyte *)(pc + offset), (double *)a, alen * __GLX_SIZE_FLOAT64)

extern gl_put_short_array (GLubyte *, short *, int);
extern gl_put_long_array (GLubyte *, long *, int);
extern gl_put_float_array (GLubyte *, float *, int);
extern gl_put_double_array (GLubyte *, double *, int);

#endif /* _CRAY */

#endif /* !__GLX_packrender_h__ */
