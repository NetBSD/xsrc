/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgavb.h,v 1.6 2001/04/10 16:07:51 dawes Exp $ */
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

#ifndef MGAVB_INC
#define MGAVB_INC

#include "types.h"
#include "vb.h"


/* common datatypes for the mga warp engines */

/*
 *  color type for the vertex data
 *  we probably want to use an internal datatype here?
 */
typedef struct mga_warp_color_t {
  GLubyte	blue;
  GLubyte       green;
  GLubyte       red;
  GLubyte       alpha;
} mga_warp_color;



/*
 * The vertex structure.  The final tu1/tv1 values only used in multitexture
 * modes.
 */
typedef struct mga_warp_vertex_t {
  GLfloat		x,y,z;	        /* coordinates in screen space*/
  GLfloat 		rhw;	        /* reciprocal homogeneous w */
  mga_warp_color	color;		/* vertex color */
  mga_warp_color	specular;	/* specular color, alpha is fog */
  GLfloat		tu0,tv0;	/* texture coordinates */
  GLfloat		tu1,tv1;	/* same for second stage */
} mga_warp_vertex;


/* The fastpath code still expects a 16-float stride vertex.
 */
union mga_vertex_t {
   mga_warp_vertex v;
   float f[16];
   GLuint ui[16];
};

typedef union mga_vertex_t mgaVertex;
typedef union mga_vertex_t *mgaVertexPtr;

struct mga_vertex_buffer_t {
   GLvector1ui clipped_elements;
   mgaVertexPtr verts;
   int last_vert;
   GLuint *primitive;
   GLuint *next_primitive;
   void *vert_store;
   GLuint size;

   GLuint *vert_buf;
   GLuint *elt_buf;
   GLuint vert_phys_start;
};

typedef struct mga_vertex_buffer_t *mgaVertexBufferPtr;

#define MGA_CONTEXT(ctx)    ((mgaContextPtr)((ctx)->DriverCtx))
#define MGA_DRIVER_DATA(vb) ((mgaVertexBufferPtr)((vb)->driver_data))


#define MGA_FOG_BIT	   MGA_F
#define MGA_ALPHA_BIT      MGA_A
#define MGA_SPEC_BIT       MGA_S
#define MGA_TEX1_BIT       MGA_T2
#define MGA_TEX0_BIT       0x10	    /* non-warp parameters */
#define MGA_RGBA_BIT       0x20
#define MGA_WIN_BIT        0x40

struct gl_pipeline_stage;

extern void mgaChooseRasterSetupFunc(GLcontext *ctx);
extern void mgaPrintSetupFlags(char *msg, GLuint flags );
extern void mgaDDDoRasterSetup( struct vertex_buffer *VB );
extern void mgaDDPartialRasterSetup( struct vertex_buffer *VB );
extern void mgaDDCheckPartialRasterSetup( GLcontext *ctx,
					  struct gl_pipeline_stage *d );


extern void mgaDDUnregisterVB( struct vertex_buffer *VB );
extern void mgaDDRegisterVB( struct vertex_buffer *VB );
extern void mgaDDResizeVB( struct vertex_buffer *VB, GLuint size );

extern void mgaDDSetupInit( void );


#endif
