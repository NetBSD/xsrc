/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_drv.h,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#ifndef __I830_MESA_DRV_H__
#define __I830_MESA_DRV_H__

#include <X11/Xlibint.h>

#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "dri_mesa.h"

#include "types.h"
#include "mm.h"
#include "context.h"
#include "mmath.h"
#include "vb.h"

#include <sys/time.h>

#include "xf86drm.h"
#include "xf86drmI830.h"
#include "i830_dri.h"

#include "i830_3d_reg.h"

/* To remove all debugging, make sure I830_DEBUG is defined as a
 * preprocessor symbol, and equal to zero.  
 */
#define I830_DEBUG 0
#ifndef I830_DEBUG
#warning "Debugging enabled - expect reduced performance"
extern int I830_DEBUG;
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
#define DEBUG_VERBOSE_TRACE  0x8000
#define DEBUG_VERBOSE_STATE  0x10000

#define DV_PF_555  (1<<8)
#define DV_PF_565  (2<<8)
#define DV_PF_8888 (3<<8)

/* Reasons to fallback on all primitives.  (see also
 * imesa->IndirectTriangles).  
 */
#define I830_FALLBACK_TEXTURE        0x1
#define I830_FALLBACK_DRAW_BUFFER    0x2
#define I830_FALLBACK_READ_BUFFER    0x4
#define I830_FALLBACK_STIPPLE        0x8  
#define I830_FALLBACK_SPECULAR       0x10
#define I830_FALLBACK_STENCIL	     0x20
#define I830_FALLBACK_COLORMASK      0x40


/* for i830ctx.new_state - manage GL->driver state changes
 */
#define I830_NEW_TEXTURE 0x1

#define I830_TEX_MAXLEVELS 10


#define I830_NO_PALETTE        0x0
#define I830_USE_PALETTE       0x1
#define I830_UPDATE_PALETTE    0x2
#define I830_FALLBACK_PALETTE  0x4

#define I830_SPEC_BIT	0x1
#define I830_FOG_BIT	0x2
#define I830_ALPHA_BIT	0x4	/* GL_BLEND, not used */
#define I830_TEX0_BIT	0x8
#define I830_TEX1_BIT	0x10	
#define I830_RGBA_BIT	0x20
#define I830_WIN_BIT	0x40

/* All structures go here */
typedef struct {
  GLubyte	blue;
  GLubyte       green;
  GLubyte       red;
  GLubyte       alpha;
} i830_color;

/* A basic fixed format vertex to kick things off.  Move to dynamic
 * layouts later on.  (see also the i830_full_vertex struct in
 * i830_3d_reg.h)
 */
typedef struct {
   float x;
   float y;
   float z;
   float oow;
   /* float point_width; */
   i830_color color;
   i830_color specular;

   float tu0;
   float tv0;
   /* q only if TEXCOORDTYPE_HOMOGENEOUS (projective) or _VECTOR (cube map)*/
   /* when q is desired, need to change VRTX_TEX_SET_0_FMT(TEXCOORDFMT_2D)
      to VRTX_TEX_SET_0_FMT(TEXCOORDFMT_3D); same for other texcoords */
   /* float tq0; */

   float tu1;
   float tv1;
   /* float tq1; */

#if defined(I830_ENABLE_4_TEXTURES) /* see i830_3d_reg.h */
   float tu2;
   float tv2;
   /* float tq2; */

   float tu3;
   float tv3;
   /* float tq3; */
#endif /* I830_ENABLE_4_TEXTURES */
} i830_vertex;

/* Unfortunately only have assembly for 16-stride vertices.
 */
union i830_vertex_t {
   i830_vertex v;
   float f[16];
   GLuint ui[16];
};

typedef union i830_vertex_t i830Vertex;
typedef union i830_vertex_t *i830VertexPtr;

struct i830_vertex_buffer_t {
   GLvector1ui clipped_elements;
   i830VertexPtr verts;
   int last_vert;
   GLuint *primitive;
   GLuint *next_primitive;
   void *vert_store;
   GLuint size;
};
   
typedef struct i830_vertex_buffer_t *i830VertexBufferPtr;


/* For shared texture space managment, these texture objects may also
 * be used as proxies for regions of texture memory containing other
 * client's textures.  Such proxy textures (not to be confused with GL
 * proxy textures) are subject to the same LRU aging we use for our
 * own private textures, and thus we have a mechanism where we can
 * fairly decide between kicking out our own textures and those of
 * other clients.
 *
 * Non-local texture objects have a valid MemBlock to describe the
 * region managed by the other client, and can be identified by
 * 't->globj == 0' 
 */
#define TEX_0	1
#define TEX_1	2

struct i830_texture_object_t {
   struct i830_texture_object_t *next, *prev;

   GLuint age;   
   struct gl_texture_object *globj;
     
   int Pitch;
   int Height;
   int texelBytes;
   int totalSize;
   int bound;

   PMemBlock MemBlock;   
   char *BufAddr;
   
   GLuint min_level;
   GLuint max_level;
   GLuint dirty_images;

   GLenum palette_format;
   GLuint palette[256];

   struct { 
      const struct gl_texture_image *image;
      int offset;		/* into BufAddr */
      int height;
      int internalFormat;
   } image[I830_TEX_MAXLEVELS];

   /* Support for multitexture.
    */
   GLuint current_unit;   
   GLuint Setup[I830_TEX_SETUP_SIZE];
};		

typedef struct {
   drmHandle handle;
   drmSize size;
   char *map;
} i830Region, *i830RegionPtr;

typedef struct {
   i830Region front;
   i830Region back;
   i830Region depth;
   i830Region tex;

   int deviceID;
   int width;
   int height;
   int mem;

   int cpp;			/* for front and back buffers */
   int bitsPerPixel;

   int fbFormat;
   int fbOffset;
   int fbStride;

   int backOffset;
   int depthOffset;

   int backPitch;
   int backPitchBits;

   int textureOffset;
   int textureSize;
   int logTextureGranularity;

   __DRIscreenPrivate *driScrnPriv;
   drmBufMapPtr  bufs;
   int use_copy_buf;
   unsigned int sarea_priv_offset;
} i830ScreenPrivate;

typedef struct i830_context_t i830Context;
typedef struct i830_context_t *i830ContextPtr;
typedef struct i830_texture_object_t *i830TextureObjectPtr;

typedef void (*i830_interp_func)( GLfloat t, 
				  GLfloat *result,
				  const GLfloat *in,
				  const GLfloat *out );



struct i830_context_t {
   GLint refcount;

   GLcontext *glCtx;

   i830TextureObjectPtr CurrentTexObj[2];

   struct i830_texture_object_t TexObjList;
   struct i830_texture_object_t SwappedOut; 

   int TextureMode;

   /* Hardware state
    */
   GLuint Setup[I830_CTX_SETUP_SIZE];
   GLuint BufferSetup[I830_DEST_SETUP_SIZE];
   GLuint TexBlend[I830_TEXBLEND_COUNT][I830_TEXBLEND_SIZE];
   GLuint TexBlendWordsUsed[I830_TEXBLEND_COUNT];
   GLuint TexBlendColorPipeNum[I830_TEXBLEND_COUNT];
   GLuint LastTexEnabled;
   GLuint TexEnabledMask;
   int vertsize;

   /* Initial GL state, stored here so we can easily access it to draw
    * quads to do clears.
    */
   GLuint Init_Setup[I830_CTX_SETUP_SIZE];
   GLuint Init_BufferSetup[I830_DEST_SETUP_SIZE];
   GLuint Init_TexBlend[I830_TEXBLEND_COUNT][I830_TEXBLEND_SIZE];
   GLuint Init_TexBlendWordsUsed[I830_TEXBLEND_COUNT];
   GLuint Init_TexBlendColorPipeNum[I830_TEXBLEND_COUNT];

   /* Support for CVA and the fast paths.
    */
   GLuint setupdone;
   GLuint setupindex;
   GLuint renderindex;
   GLuint using_fast_path;
   i830_interp_func interp;

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
   GLuint MonoColor;
   GLuint ClearColor;

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

   XF86DRIClipRectRec draw_rect;

   drmContext hHWContext;
   drmLock *driHwLock;
   int driFd;
   Display *display;
   
   int hw_stencil;

   GLfloat depth_scale;
   int depth_clear_mask;
   int stencil_clear_mask;
   int ClearDepth;

   GLboolean mask_red;
   GLboolean mask_green;
   GLboolean mask_blue;
   GLboolean mask_alpha;

   GLubyte clear_red;
   GLubyte clear_green;
   GLubyte clear_blue;
   GLubyte clear_alpha;

   GLenum palette_format;
   GLuint palette[256];

   __DRIdrawablePrivate *driDrawable;
   __DRIscreenPrivate *driScreen;
   i830ScreenPrivate *i830Screen;
   I830SAREAPtr sarea;
};

extern void i830GetLock(i830ContextPtr imesa, GLuint flags);
extern void i830EmitHwStateLocked(i830ContextPtr imesa);
extern void i830EmitScissorValues(i830ContextPtr imesa, int box_nr,
				     int emit);
extern void i830EmitDrawingRectangle(i830ContextPtr imesa);
extern void i830XMesaSetBackClipRects(i830ContextPtr imesa);
extern void i830XMesaSetFrontClipRects(i830ContextPtr imesa);
extern void i830DestroyTexObj(i830ContextPtr imesa,
				 i830TextureObjectPtr t);
extern int i830UploadTexImages(i830ContextPtr imesa,
				  i830TextureObjectPtr t);
extern void i830ResetGlobalLRU(i830ContextPtr imesa);
extern void i830TexturesGone(i830ContextPtr imesa,
				GLuint start, GLuint end, GLuint in_use);
extern void i830PrintLocalLRU(i830ContextPtr imesa);
extern void i830PrintGlobalLRU(i830ContextPtr imesa);
extern void i830ChooseRasterSetupFunc(GLcontext *ctx);
extern void i830PrintSetupFlags(char *msg, GLuint flags);
extern void i830UpdateTextureState(GLcontext *ctx);



extern void i830DDFastPath(struct vertex_buffer *VB);
extern void i830DDFastPathInit(void);
extern void i830DDExtensionsInit(GLcontext *ctx);
extern void i830DDInitDriverFuncs(GLcontext *ctx);
extern void i830DDInitSpanFuncs(GLcontext *ctx);
extern void i830DDUpdateHwState(GLcontext *ctx);
extern void i830DDUpdateState(GLcontext *ctx);
extern void i830DDInitState(i830ContextPtr imesa);
extern void i830DDInitStateFuncs(GLcontext *ctx);
extern void i830DDInitTextureFuncs(GLcontext *ctx);
extern void i830DDDoRasterSetup(struct vertex_buffer *VB);
extern void i830DDPartialRasterSetup(struct vertex_buffer *VB);
extern void i830DDCheckPartialRasterSetup(GLcontext *ctx,
					     struct gl_pipeline_stage *d);
extern void i830DDViewport(GLcontext *ctx, GLint x, GLint y,
			      GLsizei width, GLsizei height);
extern void i830DDDepthRange(GLcontext *ctx, GLclampd nearval,
				GLclampd farval);
extern void i830DDUnregisterVB(struct vertex_buffer *VB);
extern void i830DDRegisterVB(struct vertex_buffer *VB);
extern void i830DDResizeVB(struct vertex_buffer *VB, GLuint size);
extern void i830DDSetupInit(void);
extern GLboolean i830DDBuildPrecalcPipeline(GLcontext *ctx);
extern GLuint i830DDRegisterPipelineStages(struct gl_pipeline_stage *out,
					const struct gl_pipeline_stage *in,
					GLuint nr);

#define I830_TEX_UNIT_ENABLED(unit)		(1<<unit)
#define VALID_I830_TEXTURE_OBJECT(tobj)	(tobj)

#define I830_CONTEXT(ctx)	((i830ContextPtr)(ctx->DriverCtx))
#define I830_DRIVER_DATA(vb) ((i830VertexBufferPtr)((vb)->driver_data))
#define GET_DISPATCH_AGE(imesa) imesa->sarea->last_dispatch
#define GET_ENQUEUE_AGE(imesa)	imesa->sarea->last_enqueue

/* Lock the hardware and validate our state.  
 */
#define LOCK_HARDWARE( imesa )				\
  do {							\
    char __ret=0;					\
    DRM_CAS(imesa->driHwLock, imesa->hHWContext,	\
	    (DRM_LOCK_HELD|imesa->hHWContext), __ret);	\
    if (__ret)						\
        i830GetLock( imesa, 0 );			\
  } while (0)



/* Unlock the hardware using the global current context 
 */
#define UNLOCK_HARDWARE(imesa)					\
    DRM_UNLOCK(imesa->driFd, imesa->driHwLock, imesa->hHWContext);	


/* This is the wrong way to do it, I'm sure.  Otherwise the drm
 * bitches that I've already got the heavyweight lock.  At worst,
 * this is 3 ioctls.  The best solution probably only gets me down 
 * to 2 ioctls in the worst case.
 */
#define LOCK_HARDWARE_QUIESCENT( imesa ) do {	\
   LOCK_HARDWARE( imesa );			\
   i830RegetLockQuiescent( imesa );		\
} while(0)

#endif /* __I830_MESA_DRV_H__ */
