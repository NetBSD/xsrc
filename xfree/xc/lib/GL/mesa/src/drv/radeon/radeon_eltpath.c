/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_eltpath.c,v 1.2 2001/04/01 14:00:00 tsi Exp $ */
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
 *   Gareth Hughes <gareth@valinux.com>
 *   Keith Whitwell <keithw@valinux.com>
 *
 */

#include <stdio.h>

#include "radeon_context.h"
#include "radeon_pipeline.h"
#include "radeon_ioctl.h"
#include "radeon_tris.h"
#include "radeon_state.h"
#include "radeon_vb.h"

#include "types.h"
#include "enums.h"
#include "cva.h"
#include "vertices.h"
#include "mmath.h"
#include "xform.h"

/* Always use a full-sized stride for vertices. [FIXME]
 * Stride in the buffers must be a quadword multiple.
 */
#define CLIP_STRIDE	10

static void fire_elts( radeonContextPtr rmesa )
{
   GLuint vertsize = rmesa->vertsize;

   LOCK_HARDWARE( rmesa );

   /* Fire queued elements and discard that buffer if its contents
    * won't be referenced by future elements.
    */
   if ( rmesa->elt_buf )
   {
      GLuint retain = (rmesa->elt_buf == rmesa->retained_buf);

      if ( rmesa->first_elt != rmesa->next_elt ) {
	 radeonFireEltsLocked( rmesa,
			       ((char *)rmesa->first_elt -
				(char *)rmesa->elt_buf->address),
			       ((char *)rmesa->next_elt -
				(char *)rmesa->elt_buf->address),
			       !retain );
      } else if ( !retain ) {
	 radeonReleaseBufLocked( rmesa, rmesa->elt_buf );
      }

      rmesa->elt_buf = 0;
   }
   else if ( rmesa->vert_buf )
   {
      radeonFlushVerticesLocked( rmesa );
   }

   radeonGetEltBufLocked( rmesa );

   UNLOCK_HARDWARE( rmesa );

   /* Give the compiler a chance to optimize the divisions.
    */
   switch ( vertsize ) {
   case 8:
      rmesa->next_vert_index = (GLushort)
	 (((rmesa->elt_buf->idx + 1) *
	   RADEON_BUFFER_SIZE / (8 * sizeof(GLuint))) - 1);
      rmesa->next_vert = (GLfloat *)
	 ((char *)rmesa->vert_heap +
	  rmesa->next_vert_index * 8 * sizeof(GLfloat));
      break;

   case 10:
      rmesa->next_vert_index = (GLushort)
	 (((rmesa->elt_buf->idx + 1) *
	   RADEON_BUFFER_SIZE / (10 * sizeof(GLuint))) - 1);
      rmesa->next_vert = (GLfloat *)
	 ((char *)rmesa->vert_heap +
	  rmesa->next_vert_index * 10 * sizeof(GLfloat));
      break;
   }

   rmesa->first_elt = rmesa->next_elt = (GLushort *)
      ((GLubyte *)rmesa->elt_buf->address + RADEON_INDEX_PRIM_OFFSET);

   rmesa->elt_vertsize = vertsize;
}


static void release_bufs( radeonContextPtr rmesa )
{
   if ( rmesa->retained_buf && rmesa->retained_buf != rmesa->elt_buf )
   {
      LOCK_HARDWARE( rmesa );
      if ( rmesa->first_elt != rmesa->next_elt ) {
	 radeonFireEltsLocked( rmesa,
			       ((char *)rmesa->first_elt -
				(char *)rmesa->elt_buf->address),
			       ((char *)rmesa->next_elt -
				(char *)rmesa->elt_buf->address),
			       0 );

	 ALIGN_NEXT_ELT( rmesa );
	 rmesa->first_elt = rmesa->next_elt;
      }

      radeonReleaseBufLocked( rmesa, rmesa->retained_buf );
      UNLOCK_HARDWARE( rmesa );
   }

   rmesa->retained_buf = 0;
}




#define NEGATIVE( f )		(f < 0)
#define DIFFERENT_SIGNS( a, b )	((a * b) < 0)
#define LINTERP( T, A, B )	((A) + (T) * ((B) - (A)))


#define INTERP_RGBA( t, out, a, b ) {					\
   GLuint i;								\
   for ( i = 0 ; i < 4 ; i++ ) {					\
      GLfloat fa = UBYTE_COLOR_TO_FLOAT_COLOR( a[i] );			\
      GLfloat fb = UBYTE_COLOR_TO_FLOAT_COLOR( b[i] );			\
      GLfloat fo = LINTERP( t, fa, fb );				\
      FLOAT_COLOR_TO_UBYTE_COLOR( out[i], fo );				\
   }									\
}


#define CLIP( SGN, V, PLANE )						\
do {									\
   if ( mask & PLANE ) {						\
      GLuint *indata = inlist[in];					\
      GLuint *outdata = inlist[in ^= 1];				\
      GLuint nr = n;							\
      GLfloat *J = verts[indata[nr-1]];					\
      GLfloat dpJ = (SGN J[V]) + J[3];					\
									\
      for ( i = n = 0 ; i < nr ; i++ ) {				\
	 GLuint elt_i = indata[i];					\
	 GLfloat *I = verts[elt_i];					\
	 GLfloat dpI = (SGN I[V]) + I[3];				\
									\
	 if ( DIFFERENT_SIGNS( dpI, dpJ ) ) {				\
	    GLfloat *O = verts[next_vert];				\
	    outdata[n++] = next_vert++;					\
									\
	    if ( NEGATIVE( dpI ) ) {					\
	       GLfloat t = dpI / (dpI - dpJ);				\
	       interp( t, O, I, J );					\
	    }								\
	    else							\
	    {								\
	       GLfloat t = dpJ / (dpJ - dpI);				\
	       interp( t, O, J, I );					\
	    }								\
	 }								\
									\
	 if ( !NEGATIVE( dpI ) )					\
	    outdata[n++] = elt_i;					\
									\
	 J = I;								\
	 dpJ = dpI;							\
      }									\
									\
      if ( n < 3 ) return;						\
   }									\
} while (0)


static void radeon_tri_clip( radeonContextPtr rmesa,
			     struct vertex_buffer *VB,
			     GLuint *elt,
			     GLubyte mask )
{
   struct radeon_elt_tab *tab = rmesa->elt_tab;
   radeon_interp_func interp = tab->interp;
   GLuint vertsize = rmesa->vertsize;
   GLuint inlist[2][VB_MAX_CLIPPED_VERTS];
   GLuint in = 0;
   GLuint n = 3, next_vert = 3;
   GLuint i;
   GLfloat verts[VB_MAX_CLIPPED_VERTS][CLIP_STRIDE];

   /* Build temporary vertices in clipspace.  This is the potential
    * downside to this path.
    */
   tab->build_tri_verts( rmesa, VB, (GLfloat *)verts, elt );

   inlist[0][0] = 0;
   inlist[0][1] = 1;
   inlist[0][2] = 2;

   CLIP( -, 0, CLIP_RIGHT_BIT );
   CLIP( +, 0, CLIP_LEFT_BIT );
   CLIP( -, 1, CLIP_TOP_BIT );
   CLIP( +, 1, CLIP_BOTTOM_BIT );
   CLIP( -, 2, CLIP_FAR_BIT );
   CLIP( +, 2, CLIP_NEAR_BIT );


   {
      GLuint *out = inlist[in];
      GLint space = (GLint)((char *)rmesa->next_vert -
			    (char *)rmesa->next_elt);

      if ( space < (GLint)(n * (vertsize + 2) * sizeof(GLuint)) ) {
	 fire_elts( rmesa );
      }

      /* Project the new vertices and emit to dma buffers.  Translate
       * out values to physical addresses for setup dma.
       */
      tab->project_and_emit_verts( rmesa, (GLfloat *)verts, out, n );

      /* Convert the planar polygon to a list of triangles and emit to
       * elt buffers.
       */
      for ( i = 2 ; i < n ; i++ ) {
	 rmesa->next_elt[0] = (GLushort) out[0];
	 rmesa->next_elt[1] = (GLushort) out[i-1];
	 rmesa->next_elt[2] = (GLushort) out[i];
	 rmesa->next_elt += 3;
      }
   }
}




/* Build a table of functions to clip each primitive type.  These
 * produce a list of elements in the appropriate 'reduced' primitive,
 * ie (points, lines, triangles) containing all the clipped and
 * unclipped primitives from the original list.
 */

#define INIT( x )

#define TRI_THRESHOLD		(GLint)(2 * sizeof(GLuint))

#define UNCLIPPED_VERT( x )	(GLushort)(rmesa->first_vert_index - x)

#define TRIANGLE( e2, e1, e0 )						\
do {									\
   if ( (GLint)((char *)rmesa->next_vert -				\
		(char *)rmesa->next_elt) < TRI_THRESHOLD ) {		\
      fire_elts( rmesa );						\
   }									\
   rmesa->next_elt[0] = UNCLIPPED_VERT( e2 );				\
   rmesa->next_elt[1] = UNCLIPPED_VERT( e1 );				\
   rmesa->next_elt[2] = UNCLIPPED_VERT( e0 );				\
   rmesa->next_elt += 3;						\
} while (0)

#define CLIP_TRIANGLE( e2, e1, e0 )					\
do {									\
   GLubyte ormask = mask[e2] | mask[e1] | mask[e0];			\
   if ( ormask == 0 ) {							\
      TRIANGLE( e2, e1, e0 );						\
   } else if ( (mask[e2] & mask[e1] & mask[e0]) == 0 ) {		\
      out[0] = e2;							\
      out[1] = e1;							\
      out[2] = e0;							\
      radeon_tri_clip( rmesa, VB, out, ormask );			\
   }									\
} while (0)

#define LOCAL_VARS							\
   radeonContextPtr rmesa = RADEON_CONTEXT(VB->ctx);			\
   GLuint *elt = VB->EltPtr->data;					\
   GLuint out[VB_MAX_CLIPPED_VERTS];					\
   GLubyte *mask = VB->ClipMask; 					\
   (void) mask; (void) out; (void) elt; (void) rmesa;



#define RENDER_POINTS( start, count )
#define RENDER_LINE( i1, i0 )
#define RENDER_TRI( i2, i1, i0, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];			\
   if ( parity ) e2 = elt[i1], e1 = elt[i2];				\
   CLIP_TRIANGLE( e2, e1, e0 );						\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i0, pv )				\
  CLIP_TRIANGLE( elt[i3], elt[i2], elt[i0] );     			\
  CLIP_TRIANGLE( elt[i2], elt[i1], elt[i0] )

#define TAG(x) radeon_##x##_elt
#include "render_tmp.h"



#define LOCAL_VARS							\
   radeonContextPtr rmesa = RADEON_CONTEXT(VB->ctx);			\
   GLuint *elt = VB->EltPtr->data;					\
   (void) elt; (void) rmesa;

#define RENDER_POINTS( start, count )
#define RENDER_LINE( i1, i0 )
#define RENDER_TRI( i2, i1, i0, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];			\
   if ( parity ) e2 = elt[i1], e1 = elt[i2];				\
   TRIANGLE( e2, e1, e0 );						\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i0, pv )				\
  TRIANGLE( elt[i3], elt[i2], elt[i0] );				\
  TRIANGLE( elt[i2], elt[i1], elt[i0] )

#define TAG(x) radeon_##x##_elt_unclipped
#include "render_tmp.h"




static void refresh_projection_matrix( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   GLfloat *m = rmesa->device_matrix;

   m[MAT_SX] =  mat->m[MAT_SX];
   m[MAT_TX] =  mat->m[MAT_TX];
   m[MAT_SY] = -mat->m[MAT_SY];
   m[MAT_TY] = -mat->m[MAT_TY];
   m[MAT_SZ] =  mat->m[MAT_SZ];
   m[MAT_TZ] =  mat->m[MAT_TZ];
}

#define CLIP_UBYTE_R 0
#define CLIP_UBYTE_G 1
#define CLIP_UBYTE_B 2
#define CLIP_UBYTE_A 3


#define TYPE (0)
#define TAG(x) x
#include "radeon_elttmp.h"

#define TYPE (RADEON_RGBA_BIT)
#define TAG(x) x##_RGBA
#include "radeon_elttmp.h"

#define TYPE (RADEON_TEX0_BIT)
#define TAG(x) x##_TEX0
#include "radeon_elttmp.h"

#define TYPE (RADEON_RGBA_BIT|RADEON_TEX0_BIT)
#define TAG(x) x##_RGBA_TEX0
#include "radeon_elttmp.h"

#define TYPE (RADEON_RGBA_BIT|RADEON_TEX0_BIT|RADEON_TEX1_BIT)
#define TAG(x) x##_RGBA_TEX0_TEX1
#include "radeon_elttmp.h"

#define TYPE (RADEON_TEX0_BIT|RADEON_TEX1_BIT)
#define TAG(x) x##_TEX0_TEX1
#include "radeon_elttmp.h"


/* Very sparsely popluated array - fix the indices.
 */
static struct radeon_elt_tab radeonEltTab[RADEON_MAX_SETUPFUNC];

void radeonDDEltPathInit( void )
{
   radeon_render_init_elt();
   radeon_render_init_elt_unclipped();

   radeon_init_eltpath( &radeonEltTab[0] );
   radeon_init_eltpath_RGBA( &radeonEltTab[RADEON_RGBA_BIT] );
   radeon_init_eltpath_TEX0( &radeonEltTab[RADEON_TEX0_BIT] );
   radeon_init_eltpath_RGBA_TEX0( &radeonEltTab[(RADEON_RGBA_BIT |
						 RADEON_TEX0_BIT)] );
   radeon_init_eltpath_TEX0_TEX1( &radeonEltTab[(RADEON_TEX0_BIT |
						 RADEON_TEX1_BIT)] );
   radeon_init_eltpath_RGBA_TEX0_TEX1( &radeonEltTab[(RADEON_RGBA_BIT |
						      RADEON_TEX0_BIT |
						      RADEON_TEX1_BIT)] );
}

#define VALID_SETUP (RADEON_RGBA_BIT|RADEON_TEX0_BIT|RADEON_TEX1_BIT)



/* Use a temporary array for device coordinates, so that we can easily
 * tap into existing mesa assembly.  Otherwise consider emitting
 * device coordinates to dma buffers directly from the project/cliptest
 * routine.  (requires output stride, potential loss of writecombining
 * efficiency?)
 *
 * This path is a lot closer to the standard vertex path in the
 * initial stages than the original fastpath.  A slightly more optimal
 * path could be constructed, but would require us to write new
 * assembly.
 */
void radeonDDEltPath( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   struct radeon_elt_tab *tab =
       &radeonEltTab[rmesa->SetupIndex & VALID_SETUP];
   GLint vertsize = rmesa->vertsize;
   GLint space;

   VB->ClipPtr = TransformRaw( &VB->Clip,
			       &ctx->ModelProjectMatrix,
			       VB->ObjPtr );

   refresh_projection_matrix( ctx );

   VB->ClipAndMask = ~0;
   VB->ClipOrMask = 0;
   VB->Projected = gl_clip_tab[VB->ClipPtr->size]( VB->ClipPtr,
						   &VB->Win,
						   VB->ClipMask,
						   &VB->ClipOrMask,
						   &VB->ClipAndMask );

   if ( VB->ClipAndMask )
      return;

   if ( rmesa->vert_buf )
      radeonFlushVertices( rmesa );

   if ( rmesa->new_state )
      radeonDDUpdateHWState( ctx );

   space = (GLint)((char *)rmesa->next_vert -
		   (char *)rmesa->next_elt);

   /* Allocate a single buffer to hold unclipped vertices.  All
    * unclipped vertices must be contiguous.
    */
   if ( space < (GLint)(VB->Count * vertsize * sizeof(GLuint)) ||
	rmesa->vertsize != rmesa->elt_vertsize ) {
      fire_elts( rmesa );
   }

   rmesa->retained_buf = rmesa->elt_buf;

   /* Emit unclipped vertices to the buffer.
    */
   tab->emit_unclipped_verts( VB );

   /* Emit indices and clipped vertices to one or more buffers.
    */
   if ( VB->ClipOrMask ) {
      rmesa->elt_tab = tab;
      radeon_render_tab_elt[prim]( VB, 0, VB->EltPtr->count, 0 );
   } else {
      radeon_render_tab_elt_unclipped[prim]( VB, 0, VB->EltPtr->count, 0 );
   }

   /* Send to hardware and release the elt buffer.
    */
   release_bufs( rmesa );

   /* This indicates that there is no cached data to reuse.
    */
   VB->pipeline->data_valid = 0;
   VB->pipeline->new_state = 0;

   FLUSH_BATCH( rmesa );
}
