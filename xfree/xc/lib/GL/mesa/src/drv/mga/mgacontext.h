/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgacontext.h,v 1.4 2001/04/10 16:07:50 dawes Exp $*/
/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */

#ifndef MGALIB_INC
#define MGALIB_INC

#include <X11/Xlibint.h>
#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "dri_mesa.h"

#include "xf86drm.h"
#include "xf86drmMga.h"

#include "types.h"

#include "mm.h"
#include "mem.h"

#include "mgavb.h"
#include "mga_sarea.h"

#define MGA_SET_FIELD(reg,mask,val)  reg = ((reg) & (mask)) | ((val) & ~(mask))
#define MGA_FIELD(field,val) (((val) << (field ## _SHIFT)) & ~(field ## _MASK))
#define MGA_GET_FIELD(field, val) ((val & ~(field ## _MASK)) >> (field ## _SHIFT))

#define MGA_IS_G200(mmesa) (mmesa->mgaScreen->chipset == MGA_CARD_TYPE_G200)
#define MGA_IS_G400(mmesa) (mmesa->mgaScreen->chipset == MGA_CARD_TYPE_G400)


/* SoftwareFallback
 *    - texture env GL_BLEND -- can be fixed
 *    - 1D and 3D textures
 *    - incomplete textures
 *    - GL_DEPTH_FUNC == GL_NEVER not in h/w
 */
#define MGA_FALLBACK_TEXTURE   0x1
#define MGA_FALLBACK_BUFFER    0x2
#define MGA_FALLBACK_LOGICOP   0x4
#define MGA_FALLBACK_STENCIL   0x8
#define MGA_FALLBACK_DEPTH     0x10


/* For mgaCtx->new_state.
 */
#define MGA_NEW_DEPTH   0x1
#define MGA_NEW_ALPHA   0x2
#define MGA_NEW_FOG     0x4
#define MGA_NEW_CLIP    0x8
#define MGA_NEW_MASK    0x10
#define MGA_NEW_TEXTURE 0x20
#define MGA_NEW_CULL    0x40
#define MGA_NEW_WARP    0x80
#define MGA_NEW_STENCIL 0x100
#define MGA_NEW_CONTEXT 0x200


typedef void (*mga_interp_func)( GLfloat t,
				 GLfloat *result,
				 const GLfloat *in,
				 const GLfloat *out );






/* Reasons why the GL_BLEND fallback mightn't work:
 */
#define MGA_BLEND_ENV_COLOR 0x1
#define MGA_BLEND_MULTITEX  0x2

struct mga_elt_tab;
struct mga_texture_object_s;
struct mga_screen_private_s;

#define MGA_TEX_MAXLEVELS 5

typedef struct mga_texture_object_s
{
   struct mga_texture_object_s *next;
   struct mga_texture_object_s *prev;
   struct gl_texture_object *tObj;
   struct mga_context_t *ctx;
   PMemBlock	MemBlock;
   GLuint		offsets[MGA_TEX_MAXLEVELS];
   int             lastLevel;
   GLuint         dirty_images;
   GLuint		totalSize;
   int		texelBytes;
   GLuint 	age;
   int             bound;
   int             heap;	/* agp or card */

   mga_texture_regs_t setup;
} mgaTextureObject_t;

struct mga_context_t {

   GLcontext *glCtx;
   GLuint lastStamp;		/* fullscreen breaks dpriv->laststamp,
				 * need to shadow it here. */

   /* Bookkeeping for texturing
    */
   int lastTexHeap;
   struct mga_texture_object_s TexObjList[MGA_NR_TEX_HEAPS];
   struct mga_texture_object_s SwappedOut;
   struct mga_texture_object_s *CurrentTexObj[2];
   memHeap_t *texHeap[MGA_NR_TEX_HEAPS];
   int c_texupload;
   int c_texusage;
   int tex_thrash;


   /* Map GL texture units onto hardware.
    */
   GLuint multitex;
   GLuint tmu_source[2];
   GLuint tex_dest[2];

   GLboolean default32BitTextures;

   /* Manage fallbacks
    */
   GLuint IndirectTriangles;
   int Fallback;


   /* Support for CVA and the fastpath
    */
   unsigned int setupdone;
   unsigned int setupindex;
   unsigned int renderindex;
   unsigned int using_fast_path;
   mga_interp_func interp;


   /* Support for limited GL_BLEND fallback
    */
   unsigned int blend_flags;
   unsigned int envcolor;


   /* Shortcircuit some state changes
    */
   points_func   PointsFunc;
   line_func     LineFunc;
   triangle_func TriangleFunc;
   quad_func     QuadFunc;


   /* Manage driver and hardware state
    */
   GLuint        new_state;
   GLuint        dirty;

   mga_context_regs_t setup;

   GLuint        warp_pipe;
   GLuint        vertsize;
   GLuint        MonoColor;
   GLuint        ClearColor;
   GLuint        ClearDepth;
   GLuint        poly_stipple;
   GLfloat       depth_scale;

   GLuint        depth_clear_mask;
   GLuint        stencil_clear_mask;
   GLuint        hw_stencil;
   GLboolean     canDoStipple;

   /* Dma buffers
    */
   drmBufPtr  vertex_dma_buffer;
   drmBufPtr  iload_buffer;


   /* Drawable, cliprect and scissor information
    */
   int dirty_cliprects;		/* which sets of cliprects are uptodate? */
   int draw_buffer;		/* which buffer are we rendering to */
   unsigned int drawOffset;		/* draw buffer address in  space */
   int read_buffer;
   int readOffset;
   int drawX, drawY;		/* origin of drawable in draw buffer */
   int lastX, lastY;		/* detect DSTORG bug */
   GLuint numClipRects;		/* cliprects for the draw buffer */
   XF86DRIClipRectPtr pClipRects;
   XF86DRIClipRectRec draw_rect;
   XF86DRIClipRectRec scissor_rect;
   int scissor;

   XF86DRIClipRectRec tmp_boxes[2][MGA_NR_SAREA_CLIPRECTS];


   /* Texture aging and DMA based aging.
    */
   unsigned int texAge[MGA_NR_TEX_HEAPS];/* texture LRU age  */
   unsigned int dirtyAge;		/* buffer age for synchronization */

   GLuint primary_offset;

   /* Mirrors of some DRI state.
    */
   GLframebuffer *glBuffer;
   drmContext hHWContext;
   drmLock *driHwLock;
   int driFd;
   Display *display;
   __DRIdrawablePrivate *driDrawable;
   __DRIscreenPrivate *driScreen;
   struct mga_screen_private_s *mgaScreen;
   MGASAREAPrivPtr sarea;


   /* New setupdma path
    */
   drmBufPtr elt_buf, retained_buf;
   GLuint *first_elt, *next_elt;
   GLfloat *next_vert;
   GLuint next_vert_phys;
   GLuint first_vert_phys;
   struct mga_elt_tab *elt_tab;
   GLfloat device_matrix[16];
};



#define MGAPACKCOLOR555(r,g,b,a) \
  ((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3) | \
    ((a) ? 0x8000 : 0))

#define MGAPACKCOLOR565(r,g,b) \
  ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

#define MGAPACKCOLOR88(l, a) \
  (((l) << 8) | (a))

#define MGAPACKCOLOR888(r,g,b) \
  (((r) << 16) | ((g) << 8) | (b))

#define MGAPACKCOLOR8888(r,g,b,a) \
  (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define MGAPACKCOLOR4444(r,g,b,a) \
  ((((a) & 0xf0) << 8) | (((r) & 0xf0) << 4) | ((g) & 0xf0) | ((b) >> 4))


#define MGA_DEBUG 0
#ifndef MGA_DEBUG
extern int MGA_DEBUG;
#endif

#define DEBUG_ALWAYS_SYNC	0x1
#define DEBUG_VERBOSE_MSG	0x2
#define DEBUG_VERBOSE_LRU	0x4
#define DEBUG_VERBOSE_DRI	0x8
#define DEBUG_VERBOSE_IOCTL	0x10
#define DEBUG_VERBOSE_2D	0x20
#define DEBUG_VERBOSE_FALLBACK	0x40

static __inline__ GLuint mgaPackColor(GLuint cpp,
				      GLubyte r, GLubyte g,
				      GLubyte b, GLubyte a)
{
  switch (cpp) {
  case 2:
    return MGAPACKCOLOR565(r,g,b);
  case 4:
    return MGAPACKCOLOR8888(r,g,b,a);
  default:
    return 0;
  }
}


/*
 * Subpixel offsets for window coordinates:
 */
#define SUBPIXEL_X (-0.5F)
#define SUBPIXEL_Y (-0.5F + 0.125)


typedef struct mga_context_t mgaContext;
typedef struct mga_context_t *mgaContextPtr;

struct mga_elt_tab {
   void (*emit_unclipped_verts)( struct vertex_buffer *VB );

   void (*build_tri_verts)( mgaContextPtr mmesa,
			    struct vertex_buffer *VB,
			    GLfloat *O, GLuint *elt );

   void (*interp)( GLfloat t, GLfloat *O,
		   const GLfloat *I, const GLfloat *J );

   void (*project_and_emit_verts)( mgaContextPtr mmesa,
				   const GLfloat *verts,
				   GLuint *elts,
				   int nr );
};

#endif
