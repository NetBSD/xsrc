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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_fastpath.c,v 1.2 2001/08/18 02:51:06 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

#include "types.h"
#include "enums.h"
#include "cva.h"
#include "vertices.h"
#include "mmath.h"

#include "tdfx_context.h"
#include "tdfx_state.h"
#include "tdfx_vb.h"
#include "tdfx_tris.h"
#include "tdfx_pipeline.h"


struct tdfx_fast_tab {
   void (*build_vertices)( struct vertex_buffer *VB, GLuint do_cliptest );
   void (*interp)( GLfloat t, GLfloat *O, const GLfloat *I, const GLfloat *J );
   void (*project_vertices)( struct vertex_buffer *VB );
   void (*project_clipped_vertices)( struct vertex_buffer *VB );
};


#define POINT(x)   tdfx_draw_point( fxMesa, &vert[x], psize )
#define LINE(x,y)  tdfx_draw_line( fxMesa, &vert[x], &vert[y], lwidth )
#define TRI(x,y,z) fxMesa->Glide.grDrawTriangle( &vert[x], &vert[y], &vert[z] );


#define INDIRECT_TRI(x,y,z)						\
do {									\
   out[next_elt + 0] = &vert[x];					\
   out[next_elt + 1] = &vert[y];					\
   out[next_elt + 2] = &vert[z];					\
   next_elt += 3;							\
} while (0)



/* Direct, and no clipping required.  The clip funcs have not been
 * written yet, so this is only useful for the fast path.
 */
#define RENDER_POINTS( start, count )					\
do {									\
   GLuint e;								\
   for ( e = start ; e < count ; e++ )					\
      POINT( elt[e] );							\
} while (0)

#define RENDER_LINE( i1, i )						\
do {									\
   GLuint e1 = elt[i1], e = elt[i];					\
   LINE( e1, e );							\
} while (0)

#define RENDER_TRI( i2, i1, i, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e = elt[i];			\
   if ( parity ) {							\
      GLuint tmp = e2; e2 = e1; e1 = tmp;				\
   }									\
   TRI( e2, e1, e );							\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv )				\
do {									\
   GLuint e3 = elt[i3], e2 = elt[i2], e1 = elt[i1], e = elt[i];		\
   TRI( e3, e2, e );							\
   TRI( e2, e1, e );							\
} while (0)

#define LOCAL_VARS							\
   tdfxVertexPtr	vert = TDFX_DRIVER_DATA(VB)->verts;		\
   const GLuint		*elt = VB->EltPtr->data;			\
   GLcontext		*ctx = VB->ctx;					\
   tdfxContextPtr	fxMesa = TDFX_CONTEXT(ctx);			\
   const GLfloat	lwidth = ctx->Line.Width;			\
   const GLfloat	psize = ctx->Point.Size;			\
   (void) lwidth; (void) psize; (void) vert; (void) fxMesa;

#define TAG(x) tdfx_##x##_smooth_direct
#include "render_tmp.h"



/* Indirect, and no clipping required.  The clip funcs have not been
 * written yet, so this is only useful for the fast path.
 */
#define RENDER_POINTS( start, count )					\
do {									\
   GLuint e;								\
   for ( e = start ; e < count ; e++ )					\
      POINT( elt[e] );							\
} while (0)

#define RENDER_LINE( i1, i )						\
do {									\
   GLuint e1 = elt[i1], e = elt[i];					\
   LINE( e1, e );							\
} while (0)

#define RENDER_TRI( i2, i1, i, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e = elt[i];			\
   if ( parity ) {							\
      GLuint tmp = e2; e2 = e1; e1 = tmp;				\
   }									\
   INDIRECT_TRI( e2, e1, e );						\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv )				\
do {									\
   GLuint e3 = elt[i3], e2 = elt[i2], e1 = elt[i1], e = elt[i];		\
   TRI( e3, e2, e );							\
   TRI( e2, e1, e );							\
} while (0)

#define LOCAL_VARS							\
   tdfxVertexBufferPtr	fxVB = TDFX_DRIVER_DATA(VB);			\
   tdfxVertexPtr	vert = fxVB->verts;				\
   GLuint		next_elt = fxVB->last_elt;			\
   tdfxVertexPtr	*out = fxVB->elts;				\
   const GLuint		*elt = VB->EltPtr->data;			\
   GLcontext		*ctx = VB->ctx;					\
   tdfxContextPtr	fxMesa = TDFX_CONTEXT(ctx);			\
   const GLfloat	lwidth = ctx->Line.Width;			\
   const GLfloat	psize = ctx->Point.Size;			\
   (void) lwidth; (void) psize; (void) vert; (void) out; (void) fxMesa;

#define POSTFIX								\
   fxVB->last_elt = next_elt;

#define TAG(x) tdfx_##x##_smooth_indirect
#include "render_tmp.h"



#define NEGATIVE(f)		(f < 0)
#define DIFFERENT_SIGNS(a,b)	((a*b) < 0)
#define LINTERP(T, A, B)	((A) + (T) * ((B) - (A)))

#define INTERP_RGBA( t, out, a, b )					\
do {									\
   int i;								\
   for ( i = 0 ; i < 4 ; i++ ) {					\
      GLfloat fa = UBYTE_COLOR_TO_FLOAT_COLOR( a[i] );			\
      GLfloat fb = UBYTE_COLOR_TO_FLOAT_COLOR( b[i] );			\
      GLfloat fo = LINTERP( t, fa, fb );				\
      FLOAT_COLOR_TO_UBYTE_COLOR( out[i], fo );				\
   }									\
} while (0)

#define CLIP( SGN, V, PLANE )						\
do {									\
   if ( mask & PLANE ) {						\
      GLuint *indata = inlist[in];					\
      GLuint *outdata = inlist[in ^= 1];				\
      GLuint nr = n;							\
      GLfloat *J = verts[indata[nr-1]].f;				\
      GLfloat dpJ = (SGN J[V]) + J[3];					\
									\
      inlist[0] = vlist1;						\
      for ( i = n = 0 ; i < nr ; i++ ) {				\
	 GLuint elt_i = indata[i];					\
	 GLfloat *I = verts[elt_i].f;					\
	 GLfloat dpI = (SGN I[V]) + I[3];				\
									\
	 if ( DIFFERENT_SIGNS( dpI, dpJ ) ) {				\
	    GLfloat *O = verts[next_vert].f;				\
	    GLfloat t, *in, *out;					\
									\
	    if ( NEGATIVE( dpI ) ) {					\
	       t = dpI / (dpI - dpJ);					\
	       in = I;							\
	       out = J;							\
	    } else {							\
	       t = dpJ / (dpJ - dpI);					\
	       in = J;							\
	       out = I;							\
	    }								\
									\
	    interp( t, O, in, out );					\
									\
	    clipmask[next_vert] = 0;					\
	    outdata[n++] = next_vert++;					\
	 }								\
									\
	 clipmask[elt_i] |= PLANE;      /* don't set up */		\
									\
	 if ( !NEGATIVE( dpI ) ) {					\
	    outdata[n++] = elt_i;					\
	    clipmask[elt_i] &= ~PLANE; /* set up after all */		\
	 }								\
									\
	 J = I;								\
	 dpJ = dpI;							\
      }									\
									\
      if ( n < 3 ) return;						\
   }									\
} while (0)

#define LINE_CLIP( x, y, z, w, PLANE )					\
do {									\
   if ( mask & PLANE ) {						\
      GLfloat dpI = DOT4V( I, x, y, z, w );				\
      GLfloat dpJ = DOT4V( J, x, y, z, w );				\
									\
      if ( DIFFERENT_SIGNS( dpI, dpJ ) ) {				\
	 GLfloat *O = verts[next_vert].f;				\
	 GLfloat t = dpI / (dpI - dpJ);					\
									\
	 interp( t, O, I, J );						\
									\
	 clipmask[next_vert] = 0;					\
									\
	 if ( NEGATIVE( dpI ) ) {					\
	    clipmask[elts[0]] |= PLANE;					\
	    I = O;							\
	    elts[0] = next_vert++;					\
	 } else {							\
	    clipmask[elts[1]] |= PLANE;					\
	    J = O;							\
	    elts[1] = next_vert++;					\
	 }								\
      } else if ( NEGATIVE( dpI ) ) {					\
	 return;							\
      }									\
   }									\
} while (0)


static __inline void tdfx_tri_clip( GLuint **p_elts,
				    tdfxVertex *verts,
				    GLubyte *clipmask,
				    GLuint *p_next_vert,
				    GLubyte mask,
				    tdfx_interp_func interp )
{
   GLuint *elts = *p_elts;
   GLuint next_vert = *p_next_vert;
   GLuint in = 0;
   GLuint n = 3;
   GLuint vlist1[VB_MAX_CLIPPED_VERTS];
   GLuint vlist2[VB_MAX_CLIPPED_VERTS];
   GLuint *inlist[2];
   GLuint *out;
   GLuint i;

   inlist[0] = elts;
   inlist[1] = vlist2;

   CLIP( -, 0, CLIP_RIGHT_BIT );
   CLIP( +, 0, CLIP_LEFT_BIT );
   CLIP( -, 1, CLIP_TOP_BIT );
   CLIP( +, 1, CLIP_BOTTOM_BIT );
   CLIP( -, 2, CLIP_FAR_BIT );
   CLIP( +, 2, CLIP_NEAR_BIT );

   /* Convert the planar polygon to a list of triangles */
   out = inlist[in];

   for ( i = 2 ; i < n ; i++ ) {
      elts[0] = out[0];
      elts[1] = out[i-1];
      elts[2] = out[i];
      elts += 3;
   }

   *p_next_vert = next_vert;
   *p_elts = elts;
}


static __inline void tdfx_line_clip( GLuint **p_elts,
				     tdfxVertex *verts,
				     GLubyte *clipmask,
				     GLuint *p_next_vert,
				     GLubyte mask,
				     tdfx_interp_func interp )
{
   GLuint *elts = *p_elts;
   GLfloat *I = verts[elts[0]].f;
   GLfloat *J = verts[elts[1]].f;
   GLuint next_vert = *p_next_vert;

   LINE_CLIP( 1, 0, 0, -1, CLIP_LEFT_BIT );
   LINE_CLIP( -1, 0, 0, 1, CLIP_RIGHT_BIT );
   LINE_CLIP( 0, 1, 0, -1, CLIP_TOP_BIT );
   LINE_CLIP( 0, -1, 0, 1, CLIP_BOTTOM_BIT );
   LINE_CLIP( 0, 0, 1, -1, CLIP_FAR_BIT );
   LINE_CLIP( 0, 0, -1, 1, CLIP_NEAR_BIT );

   *p_next_vert = next_vert;
   *p_elts += 2;
}


#define CLIP_POINT( e )							\
do {									\
   if ( mask[e] ) *out++ = e;						\
} while (0)

#define CLIP_LINE( e1, e0 )						\
do {									\
   GLubyte ormask = mask[e0] | mask[e1];				\
   out[0] = e1;								\
   out[1] = e0;								\
   out += 2;								\
   if ( ormask ) {							\
      out-=2;								\
      if ( !(mask[e0] & mask[e1]) ) {					\
	 tdfx_line_clip( &out, verts, mask,				\
			 &next_vert, ormask, interp );			\
      }									\
   }									\
} while (0)

#define CLIP_TRIANGLE( e2, e1, e0 )					\
do {									\
   GLubyte ormask;							\
   out[0] = e2;								\
   out[1] = e1;								\
   out[2] = e0;								\
   out += 3;								\
   ormask = mask[e2] | mask[e1] | mask[e0];				\
   if ( ormask ) {							\
      out -= 3;								\
      if ( !(mask[e2] & mask[e1] & mask[e0]) ) {			\
	 tdfx_tri_clip( &out, verts, mask,				\
			&next_vert, ormask, interp );			\
      }									\
   }									\
} while (0)



/* Build a table of functions to clip each primitive type.  These
 * produce a list of elements in the appropriate 'reduced' primitive,
 * ie (points, lines, triangles) containing all the clipped and
 * unclipped primitives from the original list.
 */
#define LOCAL_VARS							\
   tdfxContextPtr	fxMesa = TDFX_CONTEXT(VB->ctx);			\
   tdfxVertexBufferPtr	fxVB = TDFX_DRIVER_DATA(VB);			\
   GLuint		*elt = VB->EltPtr->data;			\
   tdfxVertexPtr	verts = fxVB->verts;				\
   GLuint		next_vert = fxVB->last_vert;			\
   GLuint		*out = fxVB->clipped_elements.data;		\
   GLubyte		*mask = VB->ClipMask;				\
   tdfx_interp_func	interp = fxMesa->interp;			\
   (void) interp; (void) verts;

#define POSTFIX								\
   fxVB->clipped_elements.count = out - fxVB->clipped_elements.data;	\
   fxVB->last_vert = next_vert;


#define INIT( x )

#define RENDER_POINTS( start, count )					\
do {									\
   GLuint i;								\
   for ( i = start ; i < count ; i++ )					\
      CLIP_POINT( elt[i] );						\
} while (0)

#define RENDER_LINE( i1, i0 )						\
do {									\
   CLIP_LINE( elt[i1], elt[i0] );					\
} while (0)

#define RENDER_TRI( i2, i1, i0, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];			\
   if ( parity ) e2 = elt[i1], e1 = elt[i2];				\
   CLIP_TRIANGLE( e2, e1, e0 );						\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i0, pv )				\
do {									\
   CLIP_TRIANGLE( elt[i3], elt[i2], elt[i0] );				\
   CLIP_TRIANGLE( elt[i2], elt[i1], elt[i0] );				\
} while (0)

#define TAG(x) tdfx_##x##_clip_elt
#include "render_tmp.h"




/* Pack rgba and/or texture into the remaining half of a 32 byte vertex.
 */
#define CLIP_UBYTE_COLOR	4
#define CLIP_UBYTE_B		0
#define CLIP_UBYTE_G		1
#define CLIP_UBYTE_R		2
#define CLIP_UBYTE_A		3
#define CLIP_S0			6
#define CLIP_T0			7
#define CLIP_S1			8
#define CLIP_T1			9

#define TYPE (0)
#define TAG(x) x
#include "tdfx_fasttmp.h"

#define TYPE (TDFX_RGBA_BIT)
#define TAG(x) x##_RGBA
#include "tdfx_fasttmp.h"

#define TYPE (TDFX_TEX0_BIT)
#define TAG(x) x##_TEX0
#include "tdfx_fasttmp.h"

#define TYPE (TDFX_RGBA_BIT | TDFX_TEX0_BIT)
#define TAG(x) x##_RGBA_TEX0
#include "tdfx_fasttmp.h"

#define TYPE (TDFX_RGBA_BIT | TDFX_TEX0_BIT | TDFX_TEX1_BIT)
#define TAG(x) x##_RGBA_TEX0_TEX1
#include "tdfx_fasttmp.h"

/* This one *could* get away with sneaking TEX1 into the color and
 * specular slots, thus fitting inside a cache line.  Would be even
 * better to switch to a smaller vertex.
 */
#define TYPE (TDFX_TEX0_BIT | TDFX_TEX1_BIT)
#define TAG(x) x##_TEX0_TEX1
#include "tdfx_fasttmp.h"



/* Render elements directly from original list of vertices.
 */
static void tdfx_render_elements_direct( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GLenum prim = ctx->CVA.elt_mode;
   GLuint nr = VB->EltPtr->count;
   render_func func = tdfx_render_tab_smooth_direct[prim];
   GLuint p = 0;

   if ( fxMesa->new_state )
      tdfxDDUpdateHwState( ctx );

   BEGIN_CLIP_LOOP( fxMesa );
   do {
      func( VB, 0, nr, 0 );
   } while ( ctx->Driver.MultipassFunc &&
	     ctx->Driver.MultipassFunc( VB, ++p ) );
   END_CLIP_LOOP( fxMesa );
}

/* Render elements indirectly from original list of vertices.
 */
#if 0
static void tdfx_render_elements_indirect( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);
   GLenum prim = ctx->CVA.elt_mode;
   GLuint nr = VB->EltPtr->count;
   render_func func = tdfx_render_tab_smooth_indirect[prim];
   GLuint p = 0;

   if ( fxMesa->new_state )
      tdfxDDUpdateHwState( ctx );

   do {
      func( VB, 0, nr, 0 );
   } while ( ctx->Driver.MultipassFunc &&
	     ctx->Driver.MultipassFunc( VB, ++p ) );

   BEGIN_CLIP_LOOP( fxMesa );
   fxMesa->Glide.grDrawVertexArray( GR_TRIANGLES, fxVB->last_elt, fxVB->elts );
   END_CLIP_LOOP( fxMesa );

   fxVB->last_elt = 0;
}
#endif

/* Very sparsely popluated array - fix the indices.
 */
static struct tdfx_fast_tab fxFastTab[0x80];

void tdfxDDFastPathInit( void )
{
   tdfx_render_init_clip_elt();
   tdfx_render_init_smooth_direct();
   tdfx_render_init_smooth_indirect();

   tdfx_init_fastpath( &fxFastTab[0] );
   tdfx_init_fastpath_RGBA( &fxFastTab[TDFX_RGBA_BIT] );
   tdfx_init_fastpath_TEX0( &fxFastTab[TDFX_TEX0_BIT] );
   tdfx_init_fastpath_RGBA_TEX0( &fxFastTab[TDFX_RGBA_BIT|TDFX_TEX0_BIT] );
   tdfx_init_fastpath_TEX0_TEX1( &fxFastTab[TDFX_TEX0_BIT|TDFX_TEX1_BIT] );
   tdfx_init_fastpath_RGBA_TEX0_TEX1( &fxFastTab[TDFX_RGBA_BIT|TDFX_TEX0_BIT|
						TDFX_TEX1_BIT] );
}


#define VALID_SETUP (TDFX_RGBA_BIT | TDFX_TEX0_BIT | TDFX_TEX1_BIT)


void tdfxDDFastPath( struct vertex_buffer *VB )
{
   GLcontext *ctx  = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);
   struct tdfx_fast_tab *tab = &fxFastTab[fxMesa->SetupIndex & VALID_SETUP];

   if ( fxMesa->new_state ) {
      tdfxDDUpdateHwState( ctx );
   }
   else if ( fxMesa->dirty & TDFX_UPLOAD_VERTEX_LAYOUT ) {
      /* After extensive debugging I discovered that the vertex layout
       * may need to be updated at this point.  Not sure how this works
       * in the other drivers. -BP
       */
      LOCK_HARDWARE( fxMesa );
      fxMesa->Glide.grGlideSetVertexLayout( fxMesa->layout[fxMesa->vertexFormat] );
      fxMesa->dirty &= ~TDFX_UPLOAD_VERTEX_LAYOUT;
      UNLOCK_HARDWARE( fxMesa );
   }

   gl_prepare_arrays_cva( VB );   /* still need this */

   /* Reserve enough space for the pathological case */
   if ( VB->EltPtr->count * 12 > fxVB->size )
      tdfxDDResizeVB( VB, VB->EltPtr->count * 12 );

   tab->build_vertices( VB, 1 );	/* object->clip space */

   if ( VB->ClipOrMask ) {
      if ( !VB->ClipAndMask ) {
	 render_func *clip = tdfx_render_tab_clip_elt;

	 fxMesa->interp = tab->interp;

	 clip[prim]( VB, 0, VB->EltPtr->count, 0 ); /* build new elts */

	 ctx->CVA.elt_mode = gl_reduce_prim[prim];
	 VB->EltPtr = &(fxVB->clipped_elements);

	 tab->project_clipped_vertices( VB );	/* clip->device space */
	 tdfx_render_elements_direct( VB );	/* render using new list */
      }
   } else {
      tab->project_vertices( VB );		/* clip->device space  */
      tdfx_render_elements_direct( VB );	/* render using orig list */
   }

   /* This indicates that there is no cached data to reuse */
   VB->pipeline->data_valid = 0;
   VB->pipeline->new_state = 0;
}
