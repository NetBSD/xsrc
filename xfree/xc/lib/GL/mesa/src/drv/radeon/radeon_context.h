/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_context.h,v 1.2 2001/03/21 16:14:24 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

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

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef __RADEON_CONTEXT_H__
#define __RADEON_CONTEXT_H__

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>

#include "dri_mesaint.h"
#include "dri_tmm.h"

#include "xf86drm.h"
#include "xf86drmRadeon.h"

#include "types.h"

#include "radeon_sarea.h"
#include "radeon_reg.h"

struct radeon_context;
typedef struct radeon_context radeonContextRec;
typedef struct radeon_context *radeonContextPtr;

#include "radeon_lock.h"
#include "radeon_texobj.h"
#include "radeon_screen.h"

/* Flags for what context state needs to be updated */
#define RADEON_NEW_ALPHA		0x0001
#define RADEON_NEW_DEPTH		0x0002
#define RADEON_NEW_FOG			0x0004
#define RADEON_NEW_CLIP			0x0008
#define RADEON_NEW_CULL			0x0010
#define RADEON_NEW_MASKS		0x0020
#define RADEON_NEW_WINDOW		0x0040
#define RADEON_NEW_TEXTURE		0x0080
#define RADEON_NEW_CONTEXT		0x0100
#define RADEON_NEW_ALL			0x01ff

/* Flags for software fallback cases */
#define RADEON_FALLBACK_TEXTURE		0x0001
#define RADEON_FALLBACK_DRAW_BUFFER	0x0002
#define RADEON_FALLBACK_READ_BUFFER	0x0004
#define RADEON_FALLBACK_STENCIL		0x0008
#define RADEON_FALLBACK_RENDER_MODE	0x0010
#define RADEON_FALLBACK_MULTIDRAW	0x0020
#define RADEON_FALLBACK_LOGICOP		0x0040

/* Subpixel offsets for window coordinates (triangles):
 */
#define SUBPIXEL_X  (0.0625)
#define SUBPIXEL_Y  (0.125)

/* Offset for points:
 */
#define PNT_X_OFFSET  ( 0.125F)
#define PNT_Y_OFFSET  (-0.125F)

typedef void (*radeon_interp_func)( GLfloat t,
				    GLfloat *result,
				    const GLfloat *in,
				    const GLfloat *out );

struct radeon_elt_tab {
   void (*emit_unclipped_verts)( struct vertex_buffer *VB );

   void (*build_tri_verts)( radeonContextPtr rmesa,
			    struct vertex_buffer *VB,
			    GLfloat *O, GLuint *elt );

   void (*interp)( GLfloat t, GLfloat *O,
		   const GLfloat *I, const GLfloat *J );

   void (*project_and_emit_verts)( radeonContextPtr rmesa,
				   const GLfloat *verts,
				   GLuint *elts,
				   GLuint nr );
};

struct radeon_context {
   GLcontext *glCtx;			/* Mesa context */

   /* Driver and hardware state management
    */
   GLuint new_state;
   GLuint dirty;			/* Hardware state to be updated */
   radeon_context_regs_t setup;

   GLuint vertsize;
   GLuint vc_format;
   GLfloat depth_scale;

   GLuint Color;			/* Current draw color */
   GLuint ClearColor;			/* Color used to clear color buffer */
   GLuint ClearDepth;			/* Value used to clear depth buffer */
   GLuint ClearStencil;			/* Value used to clear stencil */
   GLuint DepthMask;
   GLuint StencilMask;

   /* Map GL texture units onto hardware
    */
   GLint multitex;
   GLint tmu_source[RADEON_MAX_TEXTURE_UNITS];
   GLint tex_dest[RADEON_MAX_TEXTURE_UNITS];
   GLuint color_combine[RADEON_MAX_TEXTURE_UNITS];
   GLuint alpha_combine[RADEON_MAX_TEXTURE_UNITS];
   GLuint env_color[RADEON_MAX_TEXTURE_UNITS];
   GLuint lod_bias[RADEON_MAX_TEXTURE_UNITS];

   /* Texture object bookkeeping
    */
   radeonTexObjPtr CurrentTexObj[RADEON_MAX_TEXTURE_UNITS];
   radeonTexObj TexObjList[RADEON_NR_TEX_HEAPS];
   radeonTexObj SwappedOut;
   memHeap_t *texHeap[RADEON_NR_TEX_HEAPS];
   GLint lastTexAge[RADEON_NR_TEX_HEAPS];
   GLint lastTexHeap;

   /* Current rendering state, fallbacks
    */
   points_func   PointsFunc;
   line_func     LineFunc;
   triangle_func TriangleFunc;
   quad_func     QuadFunc;

   GLuint IndirectTriangles;
   GLuint Fallback;

   /* Fast path
    */
   GLuint SetupIndex;
   GLuint SetupDone;
   GLuint RenderIndex;
   GLuint OnFastPath;
   radeon_interp_func interp;
   GLfloat *tmp_matrix;

   /* Vertex buffers
    */
   drmBufPtr vert_buf;
   GLuint vert_prim;
   GLuint num_verts;

   /* Elt path
    */
   drmBufPtr elt_buf, retained_buf;
   GLushort *first_elt, *next_elt;
   GLfloat *next_vert, *vert_heap;
   GLushort next_vert_index;
   GLushort first_vert_index;
   GLuint elt_vertsize;
   struct radeon_elt_tab *elt_tab;
   GLfloat device_matrix[16];

   /* Page flipping
    */
   GLuint doPageFlip;
   GLuint currentPage;

   /* Drawable, cliprect and scissor information
    */
   GLenum DrawBuffer;			/* Optimize draw buffer update */
   GLint drawOffset, drawPitch;
   GLint readOffset, readPitch;

   GLuint numClipRects;			/* Cliprects for the draw buffer */
   XF86DRIClipRectPtr pClipRects;

   GLuint scissor;
   XF86DRIClipRectRec scissor_rect;	/* Current software scissor */

   /* Mirrors of some DRI state
    */
   Display *display;			/* X server display */

   __DRIcontextPrivate	*driContext;	/* DRI context */
   __DRIscreenPrivate	*driScreen;	/* DRI screen */
   __DRIdrawablePrivate	*driDrawable;	/* DRI drawable bound to this ctx */

   int lastStamp;		        /* mirror driDrawable->lastStamp */

   drmContext hHWContext;
   drmLock *driHwLock;
   int driFd;

   radeonScreenPtr radeonScreen;	/* Screen private DRI data */
   RADEONSAREAPrivPtr sarea;		/* Private SAREA data */

#ifdef PER_CONTEXT_SAREA
   char *private_sarea;			/* Per-context private SAREA */
#endif

   /* Performance counters
    */
   GLuint boxes;			/* Draw performance boxes */
   GLuint hardwareWentIdle;
   GLuint c_clears;
   GLuint c_drawWaits;
   GLuint c_textureSwaps;
   GLuint c_textureBytes;
   GLuint c_vertexBuffers;
};

#define RADEON_CONTEXT(ctx)		((radeonContextPtr)(ctx->DriverCtx))


extern GLboolean radeonCreateContext( Display *dpy, GLvisual *glVisual,
				      __DRIcontextPrivate *driContextPriv );
extern void radeonDestroyContext( radeonContextPtr rmesa );
extern radeonContextPtr radeonMakeCurrent( radeonContextPtr oldCtx,
					   radeonContextPtr newCtx,
					   __DRIdrawablePrivate *dPriv );


/* ================================================================
 * Debugging:
 */
#define DO_DEBUG		0
#define ENABLE_PERF_BOXES	0

#if DO_DEBUG
extern int RADEON_DEBUG;
#else
#define RADEON_DEBUG		0
#endif

#define DEBUG_ALWAYS_SYNC	0x01
#define DEBUG_VERBOSE_API	0x02
#define DEBUG_VERBOSE_MSG	0x04
#define DEBUG_VERBOSE_LRU	0x08
#define DEBUG_VERBOSE_DRI	0x10
#define DEBUG_VERBOSE_IOCTL	0x20
#define DEBUG_VERBOSE_2D	0x40
#define DEBUG_VERBOSE_TEXTURE	0x80

#endif
#endif /* __RADEON_CONTEXT_H__ */
