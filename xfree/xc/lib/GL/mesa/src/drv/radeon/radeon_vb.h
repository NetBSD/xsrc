/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_vb.h,v 1.1 2001/01/08 01:07:29 martin Exp $ */
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

#ifndef __RADEON_VB_H__
#define __RADEON_VB_H__

#ifdef GLX_DIRECT_RENDERING

/* FIXME: This is endian-specific */
typedef struct {
   GLubyte	red;
   GLubyte	green;
   GLubyte	blue;
   GLubyte	alpha;
} radeon_color_t;

/* The vertex structure.  The final tu1/tv1 values are only used in
 * multitexture modes, and the rhw2 value is currently never used.
 */
typedef struct {
   GLfloat x, y, z;			/* Coordinates in screen space */
   GLfloat rhw;				/* Reciprocal homogeneous w */
   radeon_color_t color;		/* Diffuse color */
   radeon_color_t specular;		/* Specular color (alpha is fog) */
   GLfloat tu0, tv0;			/* Texture 0 coordinates */
   GLfloat tu1, tv1;			/* Texture 1 coordinates */
   GLfloat rhw2;			/* Reciprocal homogeneous w2 */
} radeon_vertex;

/* Format of vertices in radeon_vertex struct:
 */
#define RADEON_TEX0_VERTEX_FORMAT	(RADEON_CP_VC_FRMT_XY |		\
					 RADEON_CP_VC_FRMT_Z |		\
					 RADEON_CP_VC_FRMT_W0 |		\
					 RADEON_CP_VC_FRMT_PKCOLOR |	\
					 RADEON_CP_VC_FRMT_PKSPEC |	\
					 RADEON_CP_VC_FRMT_ST0)

#define RADEON_TEX1_VERTEX_FORMAT	(RADEON_CP_VC_FRMT_XY |		\
					 RADEON_CP_VC_FRMT_Z |		\
					 RADEON_CP_VC_FRMT_W0 |		\
					 RADEON_CP_VC_FRMT_PKCOLOR |	\
					 RADEON_CP_VC_FRMT_PKSPEC |	\
					 RADEON_CP_VC_FRMT_ST0 |	\
					 RADEON_CP_VC_FRMT_ST1)

#if 0
#define RADEON_PROJ_TEX1_VERTEX_FORMAT	(RADEON_CP_VC_FRMT_XY |		\
					 RADEON_CP_VC_FRMT_Z |		\
					 RADEON_CP_VC_FRMT_W0 |		\
					 RADEON_CP_VC_FRMT_PKCOLOR |	\
					 RADEON_CP_VC_FRMT_PKSPEC |	\
					 RADEON_CP_VC_FRMT_ST0 |	\
					 RADEON_CP_VC_FRMT_ST1 |	\
					 RADEON_CP_VC_FRMT_Q1)
#endif


/* The fastpath code still expects a 16-float stride vertex.
 */
union radeon_vertex_t {
   radeon_vertex v;
   GLfloat f[16];
   GLuint ui[16];
};

typedef union radeon_vertex_t radeonVertex;
typedef union radeon_vertex_t *radeonVertexPtr;

typedef struct {
   radeonVertexPtr verts;
   GLvector1ui clipped_elements;
   GLint last_vert;
   void *vert_store;
   GLuint size;
} *radeonVertexBufferPtr;

#define RADEON_DRIVER_DATA(vb)	((radeonVertexBufferPtr)((vb)->driver_data))

#define RADEON_WIN_BIT		0x01
#define RADEON_RGBA_BIT		0x02
#define RADEON_FOG_BIT		0x04
#define RADEON_SPEC_BIT		0x08
#define RADEON_TEX0_BIT		0x10
#define RADEON_TEX1_BIT		0x20
#define RADEON_MAX_SETUPFUNC	0x40

extern void radeonDDChooseRasterSetupFunc( GLcontext *ctx );
extern void radeonPrintSetupFlags( char *msg, GLuint flags );

extern void radeonDDCheckPartialRasterSetup( GLcontext *ctx,
					     struct gl_pipeline_stage *s );
extern void radeonDDPartialRasterSetup( struct vertex_buffer *VB );
extern void radeonDDDoRasterSetup( struct vertex_buffer *VB );

extern void radeonDDResizeVB( struct vertex_buffer *VB, GLuint size );
extern void radeonDDRegisterVB( struct vertex_buffer *VB );
extern void radeonDDUnregisterVB( struct vertex_buffer *VB );

extern void radeonDDSetupInit( void );

#endif
#endif /* __RADEON_VB_H__ */
