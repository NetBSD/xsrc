/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_vb.c,v 1.11 2001/01/08 01:07:24 martin Exp $ */
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
 *   Gareth Hughes <gareth@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *
 */

#include "r128_context.h"
#include "r128_ioctl.h"
#include "r128_state.h"
#include "r128_vb.h"

#include "mem.h"
#include "stages.h"

#define TEX0								\
do {									\
   v->v.tu0 = tc0[i][0];						\
   v->v.tv0 = tc0[i][1];						\
} while (0)

#define TEX1								\
do {									\
   v->v.tu1 = tc1[i][0];						\
   v->v.tv1 = tc1[i][1];						\
} while (0)

#define SPC								\
do {									\
   GLubyte *spec = &(VB->Spec[0][i][0]);				\
   v->v.specular.blue  = spec[2];					\
   v->v.specular.green = spec[1];					\
   v->v.specular.red   = spec[0];					\
} while (0)

#define FOG								\
do {									\
   GLubyte *spec = &(VB->Spec[0][i][0]);				\
   v->v.specular.alpha = spec[3];					\
} while (0)

#define COL								\
do {									\
   GLubyte *col = &(VB->Color[0]->data[i][0]);				\
   v->v.color.blue  = col[2];						\
   v->v.color.green = col[1];						\
   v->v.color.red   = col[0];						\
   v->v.color.alpha = col[3];						\
} while (0)

#define TEX0_4								\
do {									\
   if ( VB->TexCoordPtr[0]->size == 4 ) {				\
      GLfloat (*tc)[4] = VB->TexCoordPtr[0]->data;			\
      v = &(R128_DRIVER_DATA(VB)->verts[start]);			\
      for ( i = start ; i < end ; i++, v++ ) {				\
	 float oow = 1.0 / tc[i][3];					\
	 v->v.rhw *= tc[i][3];						\
	 v->v.tu0 *= oow;						\
	 v->v.tv0 *= oow;						\
      }									\
   }									\
} while (0)

#ifdef USE_RHW2

#define TEX1_4								\
do {									\
   if ( VB->TexCoordPtr[1]->size == 4 ) {				\
      GLfloat (*tc)[4] = VB->TexCoordPtr[1]->data;			\
      v = &(R128_DRIVER_DATA(VB)->verts[start]);			\
      for ( i = start ; i < end ; i++, v++ ) {				\
	 float oow = 1.0 / tc[i][3];					\
	 v->v.rhw2 *= tc[i][3];						\
	 v->v.tu1 *= oow;						\
	 v->v.tv1 *= oow;						\
      }									\
   }									\
} while (0)

#define COORD								\
do {									\
   GLfloat *win = VB->Win.data[i];					\
   v->v.x =               win[0] + xoffset;				\
   v->v.y =             - win[1] + yoffset;				\
   v->v.z = depth_scale * win[2];					\
   v->v.rhw = v->v.rhw2 = win[3];					\
} while (0)

#else /* USE_RHW2 */

#define TEX1_4

#define COORD								\
do {									\
   GLfloat *win = VB->Win.data[i];					\
   v->v.x =               win[0] + xoffset;				\
   v->v.y =             - win[1] + yoffset;				\
   v->v.z = depth_scale * win[2];					\
   v->v.rhw =             win[3];					\
} while (0)

#endif /* USE_RHW2 */

#define NOP


#define SETUPFUNC(name,win,col,tex0,tex1,tex0_4,tex1_4,spec,fog)	\
static void name( struct vertex_buffer *VB, GLuint start, GLuint end )	\
{									\
   r128ContextPtr rmesa = R128_CONTEXT(VB->ctx);			\
   __DRIdrawablePrivate *dPriv = rmesa->driDrawable;			\
   r128VertexPtr v;							\
   GLfloat (*tc0)[4];							\
   GLfloat (*tc1)[4];							\
   const GLfloat depth_scale = rmesa->depth_scale;			\
   const GLfloat xoffset = SUBPIXEL_X;					\
   const GLfloat yoffset = dPriv->h + SUBPIXEL_Y;			\
   GLint i;								\
									\
   (void) xoffset; (void) yoffset; (void) depth_scale;			\
									\
   gl_import_client_data( VB, VB->ctx->RenderFlags,			\
			  (VB->ClipOrMask				\
			   ? VEC_WRITABLE | VEC_GOOD_STRIDE		\
			   : VEC_GOOD_STRIDE) );			\
									\
   tc0 = VB->TexCoordPtr[rmesa->tmu_source[0]]->data;			\
   tc1 = VB->TexCoordPtr[rmesa->tmu_source[1]]->data;			\
									\
   v = &(R128_DRIVER_DATA(VB)->verts[start]);				\
									\
   if ( VB->ClipOrMask == 0 ) {						\
      for ( i = start ; i < end ; i++, v++ ) {				\
	 win;								\
	 col;								\
	 spec;								\
	 fog;								\
	 tex0;								\
	 tex1;								\
      }									\
   } else {								\
      for ( i = start ; i < end ; i++, v++ ) {				\
	 if ( VB->ClipMask[i] == 0 ) {					\
	    win;							\
	    spec;							\
	    fog;							\
	    tex0;							\
	    tex1;							\
	 }								\
	 col;								\
      }									\
   }									\
   tex0_4;								\
   tex1_4;								\
}


SETUPFUNC(rs_wt0,	COORD, NOP, TEX0, NOP,  TEX0_4, NOP,    NOP, NOP)
SETUPFUNC(rs_wt0t1,	COORD, NOP, TEX0, TEX1, TEX0_4, TEX1_4, NOP, NOP)
SETUPFUNC(rs_wft0,	COORD, NOP, TEX0, NOP,  TEX0_4, NOP,    NOP, FOG)
SETUPFUNC(rs_wft0t1,	COORD, NOP, TEX0, TEX1, TEX0_4, TEX1_4, NOP, FOG)
SETUPFUNC(rs_wg,	COORD, COL, NOP,  NOP,  NOP,    NOP,    NOP, NOP)
SETUPFUNC(rs_wgs,	COORD, COL, NOP,  NOP,  NOP,    NOP,    SPC, NOP)
SETUPFUNC(rs_wgt0,	COORD, COL, TEX0, NOP,  TEX0_4, NOP,    NOP, NOP)
SETUPFUNC(rs_wgt0t1,	COORD, COL, TEX0, TEX1, TEX0_4, TEX1_4, NOP, NOP)
SETUPFUNC(rs_wgst0,	COORD, COL, TEX0, NOP,  TEX0_4, NOP,    SPC, NOP)
SETUPFUNC(rs_wgst0t1,	COORD, COL, TEX0, TEX1, TEX0_4, TEX1_4, SPC, NOP)
SETUPFUNC(rs_wgf,	COORD, COL, NOP,  NOP,  NOP,    NOP,    NOP, FOG)
SETUPFUNC(rs_wgfs,	COORD, COL, NOP,  NOP,  NOP,    NOP,    SPC, FOG)
SETUPFUNC(rs_wgft0,	COORD, COL, TEX0, NOP,  TEX0_4, NOP,    NOP, FOG)
SETUPFUNC(rs_wgft0t1,	COORD, COL, TEX0, TEX1, TEX0_4, TEX1_4, NOP, FOG)
SETUPFUNC(rs_wgfst0,	COORD, COL, TEX0, NOP,  TEX0_4, NOP,    SPC, FOG)
SETUPFUNC(rs_wgfst0t1,	COORD, COL, TEX0, TEX1, TEX0_4, TEX1_4, SPC, FOG)

SETUPFUNC(rs_t0,	NOP,   NOP, TEX0, NOP,  TEX0_4, NOP,    NOP, NOP)
SETUPFUNC(rs_t0t1,	NOP,   NOP, TEX0, TEX1, TEX0_4, TEX1_4, NOP, NOP)
SETUPFUNC(rs_f,		NOP,   NOP, NOP,  NOP,  NOP,    NOP,    NOP, FOG)
SETUPFUNC(rs_ft0,	NOP,   NOP, TEX0, NOP,  TEX0_4, NOP,    NOP, FOG)
SETUPFUNC(rs_ft0t1,	NOP,   NOP, TEX0, TEX1, TEX0_4, TEX1_4, NOP, FOG)
SETUPFUNC(rs_g,		NOP,   COL, NOP,  NOP,  NOP,    NOP,    NOP, NOP)
SETUPFUNC(rs_gs,	NOP,   COL, NOP,  NOP,  NOP,    NOP,    SPC, NOP)
SETUPFUNC(rs_gt0,	NOP,   COL, TEX0, NOP,  TEX0_4, NOP,    NOP, NOP)
SETUPFUNC(rs_gt0t1,	NOP,   COL, TEX0, TEX1, TEX0_4, TEX1_4, NOP, NOP)
SETUPFUNC(rs_gst0,	NOP,   COL, TEX0, NOP,  TEX0_4, NOP,    SPC, NOP)
SETUPFUNC(rs_gst0t1,	NOP,   COL, TEX0, TEX1, TEX0_4, TEX1_4, SPC, NOP)
SETUPFUNC(rs_gf,	NOP,   COL, NOP,  NOP,  NOP,    NOP,    NOP, FOG)
SETUPFUNC(rs_gfs,	NOP,   COL, NOP,  NOP,  NOP,    NOP,    SPC, FOG)
SETUPFUNC(rs_gft0,	NOP,   COL, TEX0, NOP,  TEX0_4, NOP,    NOP, FOG)
SETUPFUNC(rs_gft0t1,	NOP,   COL, TEX0, TEX1, TEX0_4, TEX1_4, NOP, FOG)
SETUPFUNC(rs_gfst0,	NOP,   COL, TEX0, NOP,  TEX0_4, NOP,    SPC, FOG)
SETUPFUNC(rs_gfst0t1,	NOP,   COL, TEX0, TEX1, TEX0_4, TEX1_4, SPC, FOG)


static void rs_invalid( struct vertex_buffer *VB, GLuint start, GLuint end )
{
   fprintf( stderr, "r128RasterSetup(): invalid setup function\n" );
}

typedef void (*setupFunc)( struct vertex_buffer *, GLuint, GLuint );
static setupFunc setup_func[R128_MAX_SETUPFUNC];


void r128DDSetupInit( void )
{
   GLint i;

   for ( i = 0 ; i < R128_MAX_SETUPFUNC ; i++ ) {
      setup_func[i] = rs_invalid;
   }

   /* Functions to build vertices from scratch
    */
   setup_func[R128_WIN_BIT|R128_TEX0_BIT] = rs_wt0;
   setup_func[R128_WIN_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_wt0t1;
   setup_func[R128_WIN_BIT|R128_FOG_BIT|R128_TEX0_BIT] = rs_wft0;
   setup_func[R128_WIN_BIT|R128_FOG_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_wft0t1;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT] = rs_wg;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_SPEC_BIT] = rs_wgs;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_TEX0_BIT] = rs_wgt0;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_wgt0t1;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_SPEC_BIT|R128_TEX0_BIT] = rs_wgst0;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_SPEC_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_wgst0t1;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_FOG_BIT] = rs_wgf;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_FOG_BIT|R128_SPEC_BIT] = rs_wgfs;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_FOG_BIT|R128_TEX0_BIT] = rs_wgft0;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_FOG_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_wgft0t1;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_FOG_BIT|R128_SPEC_BIT|R128_TEX0_BIT] = rs_wgfst0;
   setup_func[R128_WIN_BIT|R128_RGBA_BIT|R128_FOG_BIT|R128_SPEC_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_wgfst0t1;

   /* Repair functions
    */
   setup_func[R128_TEX0_BIT] = rs_t0;
   setup_func[R128_TEX0_BIT|R128_TEX1_BIT] = rs_t0t1;
   setup_func[R128_FOG_BIT] = rs_f;
   setup_func[R128_FOG_BIT|R128_TEX0_BIT] = rs_ft0;
   setup_func[R128_FOG_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_ft0t1;
   setup_func[R128_RGBA_BIT] = rs_g;
   setup_func[R128_RGBA_BIT|R128_SPEC_BIT] = rs_gs;
   setup_func[R128_RGBA_BIT|R128_TEX0_BIT] = rs_gt0;
   setup_func[R128_RGBA_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_gt0t1;
   setup_func[R128_RGBA_BIT|R128_SPEC_BIT|R128_TEX0_BIT] = rs_gst0;
   setup_func[R128_RGBA_BIT|R128_SPEC_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_gst0t1;
   setup_func[R128_RGBA_BIT|R128_FOG_BIT] = rs_gf;
   setup_func[R128_RGBA_BIT|R128_FOG_BIT|R128_SPEC_BIT] = rs_gfs;
   setup_func[R128_RGBA_BIT|R128_FOG_BIT|R128_TEX0_BIT] = rs_gft0;
   setup_func[R128_RGBA_BIT|R128_FOG_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_gft0t1;
   setup_func[R128_RGBA_BIT|R128_FOG_BIT|R128_SPEC_BIT|R128_TEX0_BIT] = rs_gfst0;
   setup_func[R128_RGBA_BIT|R128_FOG_BIT|R128_SPEC_BIT|R128_TEX0_BIT|R128_TEX1_BIT] = rs_gfst0t1;
}


void r128PrintSetupFlags( char *msg, GLuint flags )
{
   fprintf( stderr, "%s: %d %s%s%s%s%s%s\n",
	    msg,
	    (int)flags,
	    (flags & R128_WIN_BIT)	? " xyzw," : "",
	    (flags & R128_RGBA_BIT)	? " rgba," : "",
	    (flags & R128_SPEC_BIT)	? " spec," : "",
	    (flags & R128_FOG_BIT)	? " fog," : "",
	    (flags & R128_TEX0_BIT)	? " tex-0," : "",
	    (flags & R128_TEX1_BIT)	? " tex-1," : "" );
}


/* Initialize the vertex buffer setup functions based on the current
 * rendering state.
 */
void r128DDChooseRasterSetupFunc( GLcontext *ctx )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   GLint index = R128_WIN_BIT | R128_RGBA_BIT;

   rmesa->multitex = 0;
   rmesa->vertsize = 8;
   rmesa->vc_format = R128_TEX0_VERTEX_FORMAT;
   rmesa->tmu_source[0] = 0;
   rmesa->tmu_source[1] = 1;
   rmesa->tex_dest[0] = R128_TEX0_BIT;
   rmesa->tex_dest[1] = R128_TEX1_BIT;
   rmesa->blend_flags &= ~R128_BLEND_MULTITEX;

   if ( ctx->Texture.ReallyEnabled & ENABLE_TEX0 ) {
      if ( R128_IS_PLAIN( rmesa ) &&		/* Pro/M3 support GL_BLEND */
	   ctx->Texture.Unit[0].EnvMode == GL_BLEND && rmesa->env_color ) {
	 rmesa->multitex = 1;
	 rmesa->vertsize = 10;
	 rmesa->vc_format = R128_TEX1_VERTEX_FORMAT;
	 rmesa->tmu_source[1] = 0;
	 index |= R128_TEX1_BIT;
      }

      index |= R128_TEX0_BIT;
   }

   if ( ctx->Texture.ReallyEnabled & ENABLE_TEX1 ) {
      if ( ctx->Texture.ReallyEnabled & ENABLE_TEX0 ) {
	 rmesa->multitex = 1;
	 rmesa->vertsize = 10;
	 rmesa->vc_format = R128_TEX1_VERTEX_FORMAT;
	 if ( R128_IS_PLAIN( rmesa ) )		/* Pro/M3 support GL_BLEND */
	    rmesa->blend_flags |= R128_BLEND_MULTITEX;
	 index |= R128_TEX1_BIT;
      } else {
	 /* Just a funny way of doing single texturing.
	  */
	 rmesa->tmu_source[0] = 1;
	 rmesa->tex_dest[1] = R128_TEX0_BIT;

	 if ( R128_IS_PLAIN( rmesa ) &&		/* Pro/M3 support GL_BLEND */
	      ctx->Texture.Unit[1].EnvMode == GL_BLEND && rmesa->env_color ) {
	    rmesa->multitex = 1;
	    rmesa->vertsize = 10;
	    rmesa->vc_format = R128_TEX1_VERTEX_FORMAT;
	    rmesa->tmu_source[1] = 1;
	    index |= R128_TEX1_BIT;
	 }

	 index |= R128_TEX0_BIT;
      }
   }

   if ( ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR )
      index |= R128_SPEC_BIT;

   if ( ctx->Fog.Enabled )
      index |= R128_FOG_BIT;

   if ( R128_DEBUG & DEBUG_VERBOSE_MSG ) {
      fprintf( stderr, "\n" );
      r128PrintSetupFlags( "full setup function", index );
   }

   rmesa->new_state |= R128_NEW_TEXTURE;
   rmesa->SetupIndex = index;

   ctx->Driver.RasterSetup = setup_func[index];
}

/* Check to see if any updates of the vertex buffer entries are needed.
 */
void r128DDCheckPartialRasterSetup( GLcontext *ctx,
				    struct gl_pipeline_stage *s )
{
   r128ContextPtr rmesa = R128_CONTEXT( ctx );
   GLint tmp = rmesa->SetupDone;

   s->type = 0;
   rmesa->SetupDone = 0;

   if ( (ctx->Array.Summary & VERT_OBJ_ANY) == 0 )
      return;

   if ( ctx->IndirectTriangles )
      return;

   rmesa->SetupDone = tmp;
}

/* Repair existing precalculated vertices with new data.
 */
void r128DDPartialRasterSetup( struct vertex_buffer *VB )
{
   r128ContextPtr rmesa = R128_CONTEXT(VB->ctx);
   GLuint new = VB->pipeline->new_outputs;
   GLuint available = VB->pipeline->outputs;
   GLuint index = 0;

   if ( new & VERT_WIN ) {
      new = available;
      index |= R128_WIN_BIT | R128_FOG_BIT;
   }

   if ( new & VERT_RGBA )
      index |= R128_RGBA_BIT | R128_SPEC_BIT;

   if ( new & VERT_TEX0_ANY )
      index |= R128_TEX0_BIT;

   if ( new & VERT_TEX1_ANY )
      index |= rmesa->tex_dest[1];

   if ( new & VERT_FOG_COORD )
      index |= R128_FOG_BIT;

   rmesa->SetupDone &= ~index;
   index &= rmesa->SetupIndex;
   rmesa->SetupDone |= index;

   if ( R128_DEBUG & DEBUG_VERBOSE_MSG )
      r128PrintSetupFlags( "partial setup function", index );

   if ( index )
      setup_func[index]( VB, VB->Start, VB->Count );
}

void r128DDDoRasterSetup( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;

   if ( VB->Type == VB_CVA_PRECALC ) {
      r128DDPartialRasterSetup( VB );
   } else if ( ctx->Driver.RasterSetup ) {
      ctx->Driver.RasterSetup( VB, VB->CopyStart, VB->Count );
   }
}


/* ================================================================
 * Hardware-format vertex buffers
 */

void r128DDResizeVB( struct vertex_buffer *VB, GLuint size )
{
   r128VertexBufferPtr rvb = R128_DRIVER_DATA(VB);

   while ( rvb->size < size )
      rvb->size *= 2;

   ALIGN_FREE( rvb->vert_store );
   rvb->vert_store = ALIGN_MALLOC( sizeof(r128Vertex) * rvb->size, 32 );
   if ( !rvb->vert_store ) {
      fprintf( stderr, "Cannot allocate vertex store!  Exiting...\n" );
      exit( 1 );
   }

   rvb->verts = (r128VertexPtr)rvb->vert_store;

   gl_vector1ui_free( &rvb->clipped_elements );
   gl_vector1ui_alloc( &rvb->clipped_elements, VEC_WRITABLE, rvb->size, 32 );
   if ( !rvb->clipped_elements.start ) {
      fprintf( stderr, "Cannot allocate clipped elements!  Exiting...\n" );
      exit( 1 );
   }

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *)ALIGN_MALLOC( sizeof(GLubyte) * rvb->size, 32 );
   if ( !VB->ClipMask ) {
      fprintf( stderr, "Cannot allocate clipmask!  Exiting...\n" );
      exit( 1 );
   }
}

void r128DDRegisterVB( struct vertex_buffer *VB )
{
   r128VertexBufferPtr rvb;

   rvb = (r128VertexBufferPtr)CALLOC( sizeof(*rvb) );

   rvb->size = VB->Size * 2;
   rvb->vert_store = ALIGN_MALLOC( sizeof(r128Vertex) * rvb->size, 32 );
   if ( !rvb->vert_store ) {
      fprintf( stderr, "Cannot allocate vertex store!  Exiting...\n" );
      exit( 1 );
   }

   rvb->verts = (r128VertexPtr)rvb->vert_store;

   gl_vector1ui_alloc( &rvb->clipped_elements, VEC_WRITABLE, rvb->size, 32 );
   if ( !rvb->clipped_elements.start ) {
      fprintf( stderr, "Cannot allocate clipped elements!  Exiting...\n" );
      exit( 1 );
   }

   ALIGN_FREE( VB->ClipMask );
   VB->ClipMask = (GLubyte *)ALIGN_MALLOC( sizeof(GLubyte) * rvb->size, 32 );
   if ( !VB->ClipMask ) {
      fprintf( stderr, "Cannot allocate clipmask!  Exiting...\n" );
      exit( 1 );
   }

   VB->driver_data = rvb;
}

void r128DDUnregisterVB( struct vertex_buffer *VB )
{
   r128VertexBufferPtr rvb = R128_DRIVER_DATA(VB);

   if ( rvb ) {
      if ( rvb->vert_store ) ALIGN_FREE( rvb->vert_store );
      gl_vector1ui_free( &rvb->clipped_elements );
      FREE( rvb );
      VB->driver_data = 0;
   }
}
