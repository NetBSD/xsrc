/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_fastpath.c,v 1.1 2001/01/08 01:07:27 martin Exp $ */
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
 *   Keith Whitwell <keithw@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "radeon_state.h"
#include "radeon_vb.h"
#include "radeon_pipeline.h"
#include "radeon_ioctl.h"
#include "radeon_tris.h"

#include "mmath.h"
#include "cva.h"
#include "vertices.h"


struct radeon_fast_tab {
   void (*build_vertices)( struct vertex_buffer *VB, GLuint do_cliptest );
   void (*interp)( GLfloat t, GLfloat *O, const GLfloat *I, const GLfloat *J );
};

#define POINT(x)   radeon_draw_point( rmesa, &vert[x], psize )
#define LINE(x,y)  radeon_draw_line( rmesa, &vert[x], &vert[y], lwidth )
#define TRI(x,y,z) radeon_draw_triangle( rmesa, &vert[x], &vert[y], &vert[z] )


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
      GLuint tmp = e2;							\
      e2 = e1;								\
      e1 = tmp;								\
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
   radeonVertexPtr vert = RADEON_DRIVER_DATA(VB)->verts;		\
   const GLuint *elt = VB->EltPtr->data;				\
   GLcontext *ctx = VB->ctx;						\
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);			\
   const GLfloat lwidth = ctx->Line.Width;				\
   const GLfloat psize = ctx->Point.Size;				\
   (void) lwidth; (void) psize; (void) vert;

#define TAG(x) radeon_##x##_smooth_indirect
#include "render_tmp.h"



#define NEGATIVE( f )		(f < 0)
#define DIFFERENT_SIGNS( a, b )	((a * b) < 0)
#define LINTERP( T, A, B )	((A) + (T) * ((B) - (A)))


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
      GLfloat dpI = DOT4V( I, x, y, z, w);				\
      GLfloat dpJ = DOT4V( J, x, y, z, w);				\
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
      } else if ( NEGATIVE( dpI ) ) return;				\
   }									\
} while (0)


static __inline void radeon_tri_clip( GLuint **p_elts,
				      radeonVertexPtr verts,
				      GLubyte *clipmask,
				      GLuint *p_next_vert,
				      GLubyte mask,
				      radeon_interp_func interp )
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


static __inline void radeon_line_clip( GLuint **p_elts,
				       radeonVertexPtr verts,
				       GLubyte *clipmask,
				       GLuint *p_next_vert,
				       GLubyte mask,
				       radeon_interp_func interp )
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
	 radeon_line_clip( &out, verts, mask,				\
			   &next_vert, ormask, interp );		\
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
	 radeon_tri_clip( &out, verts, mask,				\
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
   radeonContextPtr rmesa = RADEON_CONTEXT(VB->ctx);			\
   radeonVertexBufferPtr rvb = RADEON_DRIVER_DATA(VB);			\
   GLuint *elt = VB->EltPtr->data;					\
   radeonVertexPtr verts = rvb->verts;					\
   GLuint next_vert = rvb->last_vert;					\
   GLuint *out = rvb->clipped_elements.data;				\
   GLubyte *mask = VB->ClipMask;					\
   radeon_interp_func interp = rmesa->interp;				\
   (void) interp; (void) verts;

#define POSTFIX								\
   rvb->clipped_elements.count = out - rvb->clipped_elements.data;	\
   rvb->last_vert = next_vert;


#define INIT( x )

#define RENDER_POINTS( start, count )					\
do {									\
   GLuint i;								\
   for ( i = start; i < count ; i++ )					\
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

#define TAG(x) radeon_##x##_clip_elt
#include "render_tmp.h"



/* Pack rgba and/or texture into the remaining half of a 32 byte vertex.
 */
#define CLIP_UBYTE_COLOR	4
#define CLIP_UBYTE_R		0
#define CLIP_UBYTE_G		1
#define CLIP_UBYTE_B		2
#define CLIP_UBYTE_A		3
#define CLIP_S0			6
#define CLIP_T0			7
#define CLIP_S1			8
#define CLIP_T1			9

#define TYPE (0)
#define TAG(x) x
#include "radeon_fasttmp.h"

#define TYPE (RADEON_RGBA_BIT)
#define TAG(x) x##_RGBA
#include "radeon_fasttmp.h"

#define TYPE (RADEON_TEX0_BIT)
#define TAG(x) x##_TEX0
#include "radeon_fasttmp.h"

#define TYPE (RADEON_RGBA_BIT | RADEON_TEX0_BIT)
#define TAG(x) x##_RGBA_TEX0
#include "radeon_fasttmp.h"

#define TYPE (RADEON_RGBA_BIT | RADEON_TEX0_BIT | RADEON_TEX1_BIT)
#define TAG(x) x##_RGBA_TEX0_TEX1
#include "radeon_fasttmp.h"

/* This one *could* get away with sneaking TEX1 into the color and
 * specular slots, thus fitting inside a cache line.  Would be even
 * better to switch to a smaller vertex.
 */
#define TYPE (RADEON_TEX0_BIT | RADEON_TEX1_BIT)
#define TAG(x) x##_TEX0_TEX1
#include "radeon_fasttmp.h"



static void radeon_render_elements_direct( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLenum prim = ctx->CVA.elt_mode;
   GLuint nr = VB->EltPtr->count;
   render_func func = radeon_render_tab_smooth_indirect[prim];
   GLuint p = 0;

   if ( rmesa->new_state )
      radeonDDUpdateHWState( ctx );

   do {
      func( VB, 0, nr, 0 );
   } while ( ctx->Driver.MultipassFunc &&
	     ctx->Driver.MultipassFunc( VB, ++p ) );
}

/* GH: These should go away altogether on the Radeon.  We should disable
 * the viewport mapping entirely in Mesa and let the hardware do it in
 * all cases.
 */
static void radeon_project_vertices( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonVertexBufferPtr rvb = RADEON_DRIVER_DATA(VB);
   GLfloat *m = rmesa->tmp_matrix;

   m[MAT_SX] =  mat->m[MAT_SX];
   m[MAT_TX] =  mat->m[MAT_TX];
   m[MAT_SY] = -mat->m[MAT_SY];
   m[MAT_TY] = -mat->m[MAT_TY];
   m[MAT_SZ] =  mat->m[MAT_SZ];
   m[MAT_TZ] =  mat->m[MAT_TZ];

   gl_project_v16( rvb->verts[VB->CopyStart].f,
		   rvb->verts[rvb->last_vert].f,
		   m,
		   16 * 4 );
}

static void radeon_project_clipped_vertices( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   radeonVertexBufferPtr rvb = RADEON_DRIVER_DATA(VB);
   GLfloat *m = rmesa->tmp_matrix;

   m[MAT_SX] =  mat->m[MAT_SX];
   m[MAT_TX] =  mat->m[MAT_TX];
   m[MAT_SY] = -mat->m[MAT_SY];
   m[MAT_TY] = -mat->m[MAT_TY];
   m[MAT_SZ] =  mat->m[MAT_SZ];
   m[MAT_TZ] =  mat->m[MAT_TZ];

   gl_project_clipped_v16( rvb->verts[VB->CopyStart].f,
			   rvb->verts[rvb->last_vert].f,
			   m,
			   16 * 4,
			   VB->ClipMask + VB->CopyStart );
}

static struct radeon_fast_tab radeonFastTab[RADEON_MAX_SETUPFUNC];

void radeonDDFastPathInit( void )
{
   radeon_render_init_clip_elt();
   radeon_render_init_smooth_indirect();

   radeon_init_fastpath( &radeonFastTab[0] );
   radeon_init_fastpath_RGBA( &radeonFastTab[RADEON_RGBA_BIT] );
   radeon_init_fastpath_TEX0( &radeonFastTab[RADEON_TEX0_BIT] );
   radeon_init_fastpath_RGBA_TEX0( &radeonFastTab[(RADEON_RGBA_BIT |
						   RADEON_TEX0_BIT)] );
   radeon_init_fastpath_TEX0_TEX1( &radeonFastTab[(RADEON_TEX0_BIT |
						   RADEON_TEX1_BIT)] );
   radeon_init_fastpath_RGBA_TEX0_TEX1( &radeonFastTab[(RADEON_RGBA_BIT |
							RADEON_TEX0_BIT |
							RADEON_TEX1_BIT)] );
}

#define VALID_SETUP (RADEON_RGBA_BIT | RADEON_TEX0_BIT | RADEON_TEX1_BIT)

void radeonDDFastPath( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   radeonContextPtr rmesa = RADEON_CONTEXT( ctx );
   struct radeon_fast_tab *tab =
      &radeonFastTab[rmesa->SetupIndex & VALID_SETUP];
   GLuint do_cliptest = 1;

   gl_prepare_arrays_cva( VB );   /* still need this */

   if ( ( gl_reduce_prim[prim] == GL_TRIANGLES ) &&
	( VB->Count < (RADEON_BUFFER_SIZE / (10 * sizeof(GLuint))) ) &&
	( ctx->ModelProjectMatrix.flags & (MAT_FLAG_GENERAL |
					   MAT_FLAG_PERSPECTIVE) ) )
   {
      radeonDDEltPath( VB );
      return;
   }

   /* Reserve enough space for the pathological case */
   if ( VB->EltPtr->count * 12 > RADEON_DRIVER_DATA(VB)->size ) {
      radeonDDResizeVB( VB, VB->EltPtr->count * 12 );
      do_cliptest = 1;
   }

   tab->build_vertices( VB, do_cliptest );	/* object->clip space */

   if ( rmesa->new_state )
      radeonDDUpdateHWState( ctx );

   if ( VB->ClipOrMask ) {
      if ( !VB->ClipAndMask ) {
	 render_func *clip = radeon_render_tab_clip_elt;

	 rmesa->interp = tab->interp;

	 clip[prim]( VB, 0, VB->EltPtr->count, 0 ); /* build new elts */

	 ctx->CVA.elt_mode = gl_reduce_prim[prim];
	 VB->EltPtr = &(RADEON_DRIVER_DATA(VB)->clipped_elements);

	 radeon_project_clipped_vertices( VB );	/* clip->device space */
	 radeon_render_elements_direct( VB );	/* render using new list */
      }
   } else {
      radeon_project_vertices( VB );		/* clip->device space  */
      radeon_render_elements_direct( VB );	/* render using orig list */
   }

   /* This indicates that there is no cached data to reuse */
   VB->pipeline->data_valid = 0;
   VB->pipeline->new_state = 0;
}
