/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_vb.h,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

#ifndef __TDFX_VB_H__
#define __TDFX_VB_H__

#ifdef GLX_DIRECT_RENDERING

#include "types.h"
#include "vb.h"

/*
 * Color type for the vertex data
 */
typedef struct {
   GLubyte	blue;
   GLubyte	green;
   GLubyte	red;
   GLubyte	alpha;
} tdfx_color_t;


/* The vertex structure.  The final tu1/tv1 values only used in
 * multitexture modes, and tq0/tq1 in projected texture modes.
 */
typedef struct {
   GLfloat		x, y, z;	/* Coordinates in screen space */
   GLfloat 		rhw;		/* Reciprocal homogeneous w */
   tdfx_color_t		color;		/* Diffuse color */
   GLuint		padding;	/* ... */
   GLfloat		tu0, tv0;	/* Texture 0 coordinates */
   GLfloat		tu1, tv1;	/* Texture 1 coordinates */
   GLfloat		tq0, tq1;	/* Projected texture coordinates */
} tdfx_vertex;


/* The fastpath code still expects a 16-float stride vertex.
 */
union tdfx_vertex_t {
   tdfx_vertex	v;
   GLfloat	f[16];
   GLuint	ui[16];
};

typedef union tdfx_vertex_t tdfxVertex;
typedef union tdfx_vertex_t *tdfxVertexPtr;

/* Vertex buffer for use when on the fast path */
struct tdfx_vertex_buffer {
   tdfxVertexPtr verts;
   GLvector1ui clipped_elements;
   GLuint size;
   int last_vert;
   void *vert_store;

   tdfxVertexPtr *elts;
   GLuint elt_size;
   GLuint last_elt;
};

typedef struct tdfx_vertex_buffer *tdfxVertexBufferPtr;

#define TDFX_DRIVER_DATA(vb)	((tdfxVertexBufferPtr)((vb)->driver_data))


#define TDFX_WIN_BIT		0x01
#define TDFX_RGBA_BIT		0x02
#define TDFX_FOG_BIT		0x04
#define TDFX_SPEC_BIT		0x08
#define TDFX_TEX0_BIT		0x10
#define TDFX_TEX1_BIT		0x20


extern void tdfxDDSetupInit( void );

extern void tdfxDDChooseRasterSetupFunc( GLcontext *ctx );
extern void tdfxPrintSetupFlags( char *msg, GLuint flags );

extern void tdfxDDCheckPartialRasterSetup( GLcontext *ctx,
					   struct gl_pipeline_stage *s );
extern void tdfxDDPartialRasterSetup( struct vertex_buffer *VB );
extern void tdfxDDDoRasterSetup( struct vertex_buffer *VB );

extern void tdfxDDResizeVB( struct vertex_buffer *VB, GLuint size );
extern void tdfxDDResizeElts( struct vertex_buffer *VB, GLuint size );
extern void tdfxDDRegisterVB( struct vertex_buffer *VB );
extern void tdfxDDUnregisterVB( struct vertex_buffer *VB );


#endif /* GLX_DIRECT_RENDERING */

#endif /* __TDFX_VB_H__ */
