/*
 * GLX Hardware Device Driver for Intel i810
 * Copyright (C) 1999 Keith Whitwell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * KEITH WHITWELL, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810context.h,v 1.6 2001/08/27 21:12:19 dawes Exp $ */

#ifndef I810CONTEXT_INC
#define I810CONTEXT_INC

typedef struct i810_context_t i810Context;
typedef struct i810_context_t *i810ContextPtr;
typedef struct i810_texture_object_t *i810TextureObjectPtr;

#include <X11/Xlibint.h>
#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "dri_mesa.h"

#include "types.h"
#include "i810_init.h"
#include "drm.h"
#include "mm.h"
#include "i810log.h"

#include "i810tex.h"
#include "i810vb.h"


/* Reasons to fallback on all primitives.  (see also
 * imesa->IndirectTriangles).  
 */
#define I810_FALLBACK_TEXTURE        0x1
#define I810_FALLBACK_DRAW_BUFFER    0x2
#define I810_FALLBACK_READ_BUFFER    0x4
#define I810_FALLBACK_COLORMASK      0x8  
#define I810_FALLBACK_STIPPLE        0x10  
#define I810_FALLBACK_SPECULAR       0x20 
#define I810_FALLBACK_LOGICOP        0x40



/* for i810ctx.new_state - manage GL->driver state changes
 */
#define I810_NEW_TEXTURE 0x1


typedef void (*i810_interp_func)( GLfloat t, 
				  GLfloat *result,
				  const GLfloat *in,
				  const GLfloat *out );

#ifndef PCI_CHIP_I810				 
#define PCI_CHIP_I810              0x7121
#define PCI_CHIP_I810_DC100        0x7123
#define PCI_CHIP_I810_E            0x7125 
#define PCI_CHIP_I815              0x1132 
#endif

#define IS_I810(imesa) (imesa->i810Screen->deviceID == PCI_CHIP_I810 ||	\
			imesa->i810Screen->deviceID == PCI_CHIP_I810_DC100 || \
			imesa->i810Screen->deviceID == PCI_CHIP_I810_E)
#define IS_I815(imesa) (imesa->i810Screen->deviceID == PCI_CHIP_I815)


struct i810_context_t {
   GLint refcount;

   GLcontext *glCtx;

   i810TextureObjectPtr CurrentTexObj[2];

   struct i810_texture_object_t TexObjList;
   struct i810_texture_object_t SwappedOut; 

   int TextureMode;

   /* Hardware state
    */
   GLuint Setup[I810_CTX_SETUP_SIZE];
   GLuint BufferSetup[I810_DEST_SETUP_SIZE];
   int vertsize;
   

   /* Support for CVA and the fast paths.
    */
   GLuint setupdone;
   GLuint setupindex;
   GLuint renderindex;
   GLuint using_fast_path;
   i810_interp_func interp;

   /* Shortcircuit some state changes.
    */
   points_func PointsFunc;
   line_func LineFunc;
   triangle_func TriangleFunc;
   quad_func QuadFunc;

   /* Manage our own state */
   GLuint new_state; 

   /* Manage hardware state */
   GLuint dirty;
   memHeap_t *texHeap;

   /* One of the few bits of hardware state that can't be calculated
    * completely on the fly:
    */
   GLuint LcsCullMode;

   /* Funny mesa mirrors
    */
   GLushort MonoColor;
   GLushort ClearColor;

   /* DRI stuff
    */
   drmBufPtr  vertex_dma_buffer;
   GLuint vertex_prim;

   GLframebuffer *glBuffer;
   
   /* Two flags to keep track of fallbacks.
    */
   GLuint IndirectTriangles;
   GLuint Fallback;


   GLuint needClip;

   /* These refer to the current draw (front vs. back) buffer:
    */
   char *drawMap;		/* draw buffer address in virtual mem */
   char *readMap;	
   int drawX;			/* origin of drawable in draw buffer */
   int drawY;
   GLuint numClipRects;		/* cliprects for that buffer */
   XF86DRIClipRectPtr pClipRects;

   int lastSwap;
   int secondLastSwap;
   int texAge;
   int ctxAge;
   int dirtyAge;
   int any_contend;		/* throttle me harder */

   int scissor;
   drm_clip_rect_t draw_rect;
   drm_clip_rect_t scissor_rect;

   drmContext hHWContext;
   drmLock *driHwLock;
   int driFd;
   Display *display;

   __DRIdrawablePrivate *driDrawable;
   __DRIscreenPrivate *driScreen;
   i810ScreenPrivate *i810Screen; 
   drm_i810_sarea_t *sarea;
};


/* To remove all debugging, make sure I810_DEBUG is defined as a
 * preprocessor symbol, and equal to zero.  
 */
#define I810_DEBUG 0   
#ifndef I810_DEBUG
#warning "Debugging enabled - expect reduced performance"
extern int I810_DEBUG;
#endif

#define DEBUG_VERBOSE_2D     0x1
#define DEBUG_VERBOSE_RING   0x8
#define DEBUG_VERBOSE_OUTREG 0x10
#define DEBUG_ALWAYS_SYNC    0x40
#define DEBUG_VERBOSE_MSG    0x80
#define DEBUG_NO_OUTRING     0x100
#define DEBUG_NO_OUTREG      0x200
#define DEBUG_VERBOSE_API    0x400
#define DEBUG_VALIDATE_RING  0x800
#define DEBUG_VERBOSE_LRU    0x1000
#define DEBUG_VERBOSE_DRI    0x2000
#define DEBUG_VERBOSE_IOCTL  0x4000


#endif
