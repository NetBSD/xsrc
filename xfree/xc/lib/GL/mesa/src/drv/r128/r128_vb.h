/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_vb.h,v 1.3 2000/12/04 19:21:48 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
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
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
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

#ifndef _R128_VB_H_
#define _R128_VB_H_

#ifdef GLX_DIRECT_RENDERING

/* FIXME: This is endian-specific */
typedef struct {
    GLubyte	blue;
    GLubyte	green;
    GLubyte	red;
    GLubyte	alpha;
} r128_color_t;

/* The vertex structure.  The final tu1/tv1 values are only used in
 * multitexture modes, and the rhw2 value is currently never used.
 */
typedef struct {
    GLfloat x, y, z;		/* Coordinates in screen space */
    GLfloat rhw;		/* Reciprocal homogeneous w */
    r128_color_t color;		/* Diffuse color */
    r128_color_t specular;	/* Specular color (alpha is fog) */
    GLfloat tu0, tv0;		/* Texture 0 coordinates */
    GLfloat tu1, tv1;		/* Texture 1 coordinates */
    GLfloat rhw2;		/* Reciprocal homogeneous w2 */
} r128_vertex;

/* Format of vertices in r128_vertex struct */
#define R128_TEX0_VERTEX_FORMAT                                            \
    R128_CCE_VC_FRMT_RHW          |                                        \
    R128_CCE_VC_FRMT_DIFFUSE_ARGB |                                        \
    R128_CCE_VC_FRMT_SPEC_FRGB    |                                        \
    R128_CCE_VC_FRMT_S_T

#define R128_TEX1_VERTEX_FORMAT                                            \
    R128_CCE_VC_FRMT_RHW          |                                        \
    R128_CCE_VC_FRMT_DIFFUSE_ARGB |                                        \
    R128_CCE_VC_FRMT_SPEC_FRGB    |                                        \
    R128_CCE_VC_FRMT_S_T          |                                        \
    R128_CCE_VC_FRMT_S2_T2

#define R128_PROJ_TEX1_VERTEX_FORMAT                                       \
    R128_CCE_VC_FRMT_RHW          |                                        \
    R128_CCE_VC_FRMT_DIFFUSE_ARGB |                                        \
    R128_CCE_VC_FRMT_SPEC_FRGB    |                                        \
    R128_CCE_VC_FRMT_S_T          |                                        \
    R128_CCE_VC_FRMT_S2_T2        |                                        \
    R128_CCE_VC_FRMT_RHW2


/* FIXME: We currently only have assembly for 16-stride vertices */
union r128_vertex_t {
   r128_vertex	v;
   GLfloat	f[16];
   CARD32	ui[16];
};

typedef union r128_vertex_t r128Vertex;
typedef union r128_vertex_t *r128VertexPtr;

/* Vertex buffer for use when on the fast path */
typedef struct {
    GLuint         size;		/* Number of vertices in store */
    void          *vert_store;		/* Storage for vertex buffer */
    r128VertexPtr  verts;		/* Aligned start of verts in storage */
    int            last_vert;		/* Index of last vertex used */
    GLvector1ui    clipped_elements;	/* List of clipped elements */
} *r128VertexBufferPtr;

#define R128_DRIVER_DATA(vb) ((r128VertexBufferPtr)((vb)->driver_data))

#define R128_WIN_BIT		0x01
#define R128_RGBA_BIT		0x02
#define R128_FOG_BIT		0x04
#define R128_SPEC_BIT		0x08
#define R128_TEX0_BIT		0x10
#define R128_TEX1_BIT		0x20

extern void r128DDSetupInit( void );

extern void r128DDChooseRasterSetupFunc( GLcontext *ctx );
extern void r128PrintSetupFlags( char *msg, GLuint flags );

extern void r128DDCheckPartialRasterSetup( GLcontext *ctx,
					 struct gl_pipeline_stage *s );
extern void r128DDPartialRasterSetup(struct vertex_buffer *VB);
extern void r128DDDoRasterSetup(struct vertex_buffer *VB);

extern void r128DDResizeVB(struct vertex_buffer *VB, GLuint size);
extern void r128DDRegisterVB(struct vertex_buffer *VB);
extern void r128DDUnregisterVB(struct vertex_buffer *VB);

#endif
#endif /* _R128_VB_H_ */
