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
 *
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810vb.h,v 1.3 2000/08/28 02:43:12 tsi Exp $ */

#ifndef I810VB_INC
#define I810VB_INC

#include "vb.h"
#include "types.h"

/* 
 *  color type for the vertex data
 *  we probably want to use an internal datatype here?
 */
typedef struct {
  GLubyte	blue;
  GLubyte       green;
  GLubyte       red;
  GLubyte       alpha;
} i810_color;


/* A basic fixed format vertex to kick things off.  Move to dynamic
 * layouts later on.  (see also the i810_full_vertex struct in
 * i810_3d_reg.h)
 */
typedef struct {
   GLfloat x,y,z;	        /* coordinates in screen space*/
   GLfloat oow;			/* reciprocal homogeneous w */
   i810_color color;		/* vertex color */
   i810_color specular;		/* specular color, alpha is fog */
   GLfloat tu0,tv0;		/* texture 0 */
   GLfloat tu1,tv1;		/* texture 1 */
} i810_vertex;


/* Unfortunately only have assembly for 16-stride vertices.
 */
union i810_vertex_t {
   i810_vertex v;
   float f[16];
   GLuint ui[16];
};

typedef union i810_vertex_t i810Vertex;
typedef union i810_vertex_t *i810VertexPtr;

struct i810_vertex_buffer_t {
   GLvector1ui clipped_elements;
   i810VertexPtr verts;
   int last_vert;
   GLuint *primitive;
   GLuint *next_primitive;
   void *vert_store;
   GLuint size;
};
   
typedef struct i810_vertex_buffer_t *i810VertexBufferPtr;


#define I810_CONTEXT(ctx)    ((i810ContextPtr)(ctx->DriverCtx))
#define I810_DRIVER_DATA(vb) ((i810VertexBufferPtr)((vb)->driver_data))


#define I810_SPEC_BIT       0x1
#define I810_FOG_BIT	    0x2
#define I810_ALPHA_BIT      0x4	/* GL_BLEND, not used */
#define I810_TEX1_BIT       0x8
#define I810_TEX0_BIT       0x10	
#define I810_RGBA_BIT       0x20
#define I810_WIN_BIT        0x40


extern void i810ChooseRasterSetupFunc(GLcontext *ctx);
extern void i810PrintSetupFlags(char *msg, GLuint flags );
extern void i810DDDoRasterSetup( struct vertex_buffer *VB );
extern void i810DDPartialRasterSetup( struct vertex_buffer *VB );
extern void i810DDCheckPartialRasterSetup( GLcontext *ctx, 
					   struct gl_pipeline_stage *d );

extern void i810DDViewport( GLcontext *ctx, 
			   GLint x, GLint y, 
			   GLsizei width, GLsizei height );

extern void i810DDDepthRange( GLcontext *ctx, 
			     GLclampd nearval, GLclampd farval );


extern void i810DDUnregisterVB( struct vertex_buffer *VB );
extern void i810DDRegisterVB( struct vertex_buffer *VB );
extern void i810DDResizeVB( struct vertex_buffer *VB, GLuint size );

extern void i810DDSetupInit( void );


#endif
