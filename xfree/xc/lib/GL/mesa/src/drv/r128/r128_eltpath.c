/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_eltpath.c,v 1.2 2000/12/07 20:26:08 dawes Exp $ */
/*
 * GLX Hardware Device Driver for Matrox G400
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
 */

#include <stdio.h>

#include "r128_context.h"
#include "r128_pipeline.h"
#include "r128_ioctl.h"
#include "r128_tris.h"
#include "r128_state.h"
#include "r128_vb.h"

#include "types.h"
#include "enums.h"
#include "cva.h"
#include "vertices.h"
#include "mmath.h"
#include "xform.h"

#define DEBUG_ELTPATH	0

/* Always use a full-sized stride for vertices. [FIXME]
 * Stride in the buffers must be a quadword multiple.
 */
#define CLIP_STRIDE	10

static void fire_elts( r128ContextPtr r128ctx )
{
   int vertsize = r128ctx->vertsize;

   LOCK_HARDWARE( r128ctx );

   if ( DEBUG_ELTPATH )
      fprintf( stderr, "\n%s: elt=%p ret=%p vs=%d\n",
	       __FUNCTION__, r128ctx->elt_buf,
	       r128ctx->retained_buf, vertsize );

   /* Fire queued elements and discard that buffer if its contents
    * won't be referenced by future elements.
    */
   if ( r128ctx->elt_buf )
   {
      GLuint retain = (r128ctx->elt_buf == r128ctx->retained_buf);

      if ( r128ctx->first_elt != r128ctx->next_elt ) {
	 r128FireEltsLocked( r128ctx,
			     ((GLuint)r128ctx->first_elt -
			      (GLuint)r128ctx->elt_buf->address),
			     ((GLuint)r128ctx->next_elt -
			      (GLuint)r128ctx->elt_buf->address),
			     !retain );
      } else if ( !retain ) {
	 r128ReleaseBufLocked( r128ctx, r128ctx->elt_buf );
      }

      r128ctx->elt_buf = 0;
   }
   else if ( r128ctx->vert_buf )
   {
      r128FlushVerticesLocked( r128ctx );
   }

   r128GetEltBufLocked( r128ctx );

   UNLOCK_HARDWARE( r128ctx );

   /* Give the compiler a chance to optimize the divisions.
    */
   switch ( vertsize ) {
   case 8:
      r128ctx->next_vert_index = (GLushort)
	 (((r128ctx->elt_buf->idx + 1) *
	   R128_BUFFER_SIZE / (8 * sizeof(GLuint))) - 1);
      r128ctx->next_vert = (GLfloat *)
	 ((GLuint)r128ctx->vert_heap +
	  r128ctx->next_vert_index * 8 * sizeof(GLfloat));
      break;

   case 10:
      r128ctx->next_vert_index = (GLushort)
	 (((r128ctx->elt_buf->idx + 1) *
	   R128_BUFFER_SIZE / (10 * sizeof(GLuint))) - 1);
      r128ctx->next_vert = (GLfloat *)
	 ((GLuint)r128ctx->vert_heap +
	  r128ctx->next_vert_index * 10 * sizeof(GLfloat));
      break;
   }

   r128ctx->first_elt = r128ctx->next_elt = (GLushort *)
      ((GLubyte *)r128ctx->elt_buf->address + R128_INDEX_PRIM_OFFSET);

   r128ctx->elt_vertsize = vertsize;

   if ( DEBUG_ELTPATH )
      fprintf( stderr, "new elt=%p idx=%d tot=%d next=%p idx=%d\n\n",
	       r128ctx->elt_buf, r128ctx->elt_buf->idx,
	       r128ctx->elt_buf->total,
	       r128ctx->next_vert, r128ctx->next_vert_index );

   {
      GLint space = (GLint)((GLuint)r128ctx->next_vert -
			    (GLuint)r128ctx->next_elt);
      if ( DEBUG_ELTPATH )
	 fprintf( stderr, "   new nv=%p ne=%p space=%d\n",
		  r128ctx->next_vert, r128ctx->next_elt, space );
   }
}


static void release_bufs( r128ContextPtr r128ctx )
{
   if ( DEBUG_ELTPATH )
      fprintf( stderr, "%s: %d idx=%d\n",
	       __FUNCTION__,
	       r128ctx->retained_buf &&
	       r128ctx->retained_buf != r128ctx->elt_buf,
	       r128ctx->retained_buf ? r128ctx->retained_buf->idx : -1 );

   if ( r128ctx->retained_buf && r128ctx->retained_buf != r128ctx->elt_buf )
   {
      LOCK_HARDWARE( r128ctx );
      if ( r128ctx->first_elt != r128ctx->next_elt ) {
	 r128FireEltsLocked( r128ctx,
			     ((GLuint)r128ctx->first_elt -
			      (GLuint)r128ctx->elt_buf->address),
			     ((GLuint)r128ctx->next_elt -
			      (GLuint)r128ctx->elt_buf->address),
			     0 );

	 ALIGN_NEXT_ELT( r128ctx );
	 r128ctx->first_elt = r128ctx->next_elt;
      }

      r128ReleaseBufLocked( r128ctx, r128ctx->retained_buf );
      UNLOCK_HARDWARE( r128ctx );
   }

   r128ctx->retained_buf = 0;
}




#define NEGATIVE(f)          (f < 0)
#define DIFFERENT_SIGNS(a,b) ((a*b) < 0)
#define LINTERP( T, A, B )   ( (A) + (T) * ( (B) - (A) ) )


#define INTERP_RGBA(t, out, a, b) {			\
   int i;						\
   for (i = 0; i < 4; i++) {				\
      GLfloat fa = UBYTE_COLOR_TO_FLOAT_COLOR(a[i]);	\
      GLfloat fb = UBYTE_COLOR_TO_FLOAT_COLOR(b[i]);	\
      GLfloat fo = LINTERP(t, fa, fb);			\
      FLOAT_COLOR_TO_UBYTE_COLOR(out[i], fo);		\
   }							\
}


#define CLIP(SGN,V,PLANE)			\
if (mask & PLANE) {				\
   GLuint *indata = inlist[in];			\
   GLuint *outdata = inlist[in ^= 1];		\
   GLuint nr = n;				\
   GLfloat *J = verts[indata[nr-1]];		\
   GLfloat dpJ = (SGN J[V]) + J[3];		\
						\
   for (i = n = 0 ; i < nr ; i++) {		\
      GLuint elt_i = indata[i];			\
      GLfloat *I = verts[elt_i];		\
      GLfloat dpI = (SGN I[V]) + I[3];		\
						\
      if (DIFFERENT_SIGNS(dpI, dpJ)) {		\
	 GLfloat *O = verts[next_vert];	\
	 outdata[n++] = next_vert++;		\
						\
	 if (NEGATIVE(dpI)) {			\
	    GLfloat t = dpI / (dpI - dpJ);	\
	    interp(t, O, I, J);			\
	 }					\
         else					\
	 {					\
	    GLfloat t = dpJ / (dpJ - dpI);	\
	    interp(t, O, J, I);			\
	 }					\
      }						\
						\
      if (!NEGATIVE(dpI))			\
	 outdata[n++] = elt_i;			\
						\
      J = I;					\
      dpJ = dpI;				\
   }						\
						\
   if (n < 3) return;				\
}


static void r128_tri_clip( r128ContextPtr r128ctx,
			   struct vertex_buffer *VB,
			   GLuint *elt,
			   GLubyte mask )
{
   struct r128_elt_tab *tab = r128ctx->elt_tab;
   r128_interp_func interp = tab->interp;
   GLint vertsize = r128ctx->vertsize;
   GLuint inlist[2][VB_MAX_CLIPPED_VERTS];
   GLuint in = 0;
   GLuint n = 3, next_vert = 3;
   GLuint i;
   GLfloat verts[VB_MAX_CLIPPED_VERTS][CLIP_STRIDE];

   /* Build temporary vertices in clipspace.  This is the potential
    * downside to this path.
    */
   tab->build_tri_verts( r128ctx, VB, (GLfloat *)verts, elt );

   inlist[0][0] = 0;
   inlist[0][1] = 1;
   inlist[0][2] = 2;

   CLIP(-,0,CLIP_RIGHT_BIT);
   CLIP(+,0,CLIP_LEFT_BIT);
   CLIP(-,1,CLIP_TOP_BIT);
   CLIP(+,1,CLIP_BOTTOM_BIT);
   CLIP(-,2,CLIP_FAR_BIT);
   CLIP(+,2,CLIP_NEAR_BIT);


   {
      GLuint *out = inlist[in];
      GLint space = (GLint)((GLuint)r128ctx->next_vert -
			    (GLuint)r128ctx->next_elt);

      if ( DEBUG_ELTPATH )
	 fprintf( stderr, "  clip nv=%p ne=%p space=%d thresh=%d %d\n",
		  r128ctx->next_vert, r128ctx->next_elt,
		  space, (GLint)n * (vertsize + 2) * (GLint)sizeof(GLuint),
		  space < (GLint)n * (vertsize + 2) * (GLint)sizeof(GLuint) );

      /* GH: Why the hell do we explicitly have to test the sign of the
       * available space here?
       */
      if ( space < (GLint)n * (vertsize + 2) * (GLint)sizeof(GLuint) ) {
	 fire_elts( r128ctx );
      }

      /* Project the new vertices and emit to dma buffers.  Translate
       * out values to physical addresses for setup dma.
       */
      tab->project_and_emit_verts( r128ctx, (GLfloat *)verts, out, n );

      /* Convert the planar polygon to a list of triangles and emit to
       * elt buffers.
       */
      for (i = 2 ; i < n ; i++) {
	 r128ctx->next_elt[0] = (GLushort) out[0];
	 r128ctx->next_elt[1] = (GLushort) out[i-1];
	 r128ctx->next_elt[2] = (GLushort) out[i];
	 r128ctx->next_elt += 3;
      }
   }
   if ( DEBUG_ELTPATH )
      fflush( stderr );
}




/* Build a table of functions to clip each primitive type.  These
 * produce a list of elements in the appropriate 'reduced' primitive,
 * ie (points, lines, triangles) containing all the clipped and
 * unclipped primitives from the original list.
 */

#define INIT(x)

#define TRI_THRESHOLD		(GLint)(2 * sizeof(GLuint))

#define UNCLIPPED_VERT(x)	(GLushort)(r128ctx->first_vert_index - x)

#define TRIANGLE( e2, e1, e0 )						\
do {									\
   if ( DEBUG_ELTPATH )							\
      fprintf( stderr, "   tri nv=%p ne=%p space=%d thresh=%d %d\n",	\
	       r128ctx->next_vert, r128ctx->next_elt,			\
	       (GLint)((GLuint)r128ctx->next_vert -			\
		       (GLuint)r128ctx->next_elt), TRI_THRESHOLD,	\
	       ( (GLint)((GLuint)r128ctx->next_vert -			\
			 (GLuint)r128ctx->next_elt) < TRI_THRESHOLD ) );\
   if ( (GLint)((GLuint)r128ctx->next_vert -				\
		(GLuint)r128ctx->next_elt) < TRI_THRESHOLD ) {		\
      if ( DEBUG_ELTPATH )						\
	 fprintf( stderr, "      firing elts...\n" );			\
      fire_elts( r128ctx );						\
   }									\
   r128ctx->next_elt[0] = UNCLIPPED_VERT( e2 );				\
   r128ctx->next_elt[1] = UNCLIPPED_VERT( e1 );				\
   r128ctx->next_elt[2] = UNCLIPPED_VERT( e0 );				\
   if ( 0 )								\
      fprintf( stderr, "    tri %d,%d,%d -> %hd,%hd,%hd\n",		\
	       e2, e1, e0, r128ctx->next_elt[0],			\
	       r128ctx->next_elt[1], r128ctx->next_elt[2]);		\
   r128ctx->next_elt += 3;						\
   if ( DEBUG_ELTPATH )							\
      fflush( stderr );							\
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
      r128_tri_clip( r128ctx, VB, out, ormask );			\
   }									\
} while (0)

#define LOCAL_VARS							\
   r128ContextPtr r128ctx = R128_CONTEXT( VB->ctx );			\
   GLuint *elt = VB->EltPtr->data;					\
   GLuint out[VB_MAX_CLIPPED_VERTS];					\
   GLubyte *mask = VB->ClipMask; 					\
   (void) mask; (void) out; (void) elt; (void) r128ctx;



#define RENDER_POINTS( start, count )
#define RENDER_LINE( i1, i0 )
#define RENDER_TRI( i2, i1, i0, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];			\
   if (parity) e2 = elt[i1], e1 = elt[i2];				\
   CLIP_TRIANGLE( e2, e1, e0 );						\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i0, pv )				\
  CLIP_TRIANGLE( elt[i3], elt[i2], elt[i0] );     			\
  CLIP_TRIANGLE( elt[i2], elt[i1], elt[i0] )

#define TAG(x) r128_##x##_elt
#include "render_tmp.h"



#define LOCAL_VARS							\
   r128ContextPtr r128ctx = R128_CONTEXT( VB->ctx );			\
   GLuint *elt = VB->EltPtr->data;					\
   (void) elt; (void) r128ctx;

#define RENDER_POINTS( start, count )
#define RENDER_LINE( i1, i0 )
#define RENDER_TRI( i2, i1, i0, pv, parity )				\
do {									\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];			\
   if (parity) e2 = elt[i1], e1 = elt[i2];				\
   TRIANGLE( e2, e1, e0 );						\
} while (0)

#define RENDER_QUAD(i3, i2, i1, i0, pv )				\
  TRIANGLE( elt[i3], elt[i2], elt[i0] );				\
  TRIANGLE( elt[i2], elt[i1], elt[i0] )

#define TAG(x) r128_##x##_elt_unclipped
#include "render_tmp.h"




static void refresh_projection_matrix( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT(ctx);
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   GLfloat *m = r128ctx->device_matrix;

   m[MAT_SX] =  mat->m[MAT_SX];
   m[MAT_TX] =  mat->m[MAT_TX];
   m[MAT_SY] = -mat->m[MAT_SY];
   m[MAT_TY] = -mat->m[MAT_TY] + r128ctx->driDrawable->h;
   m[MAT_SZ] =  mat->m[MAT_SZ] * r128ctx->depth_scale;
   m[MAT_TZ] =  mat->m[MAT_TZ] * r128ctx->depth_scale;
}

#define CLIP_UBYTE_B 0
#define CLIP_UBYTE_G 1
#define CLIP_UBYTE_R 2
#define CLIP_UBYTE_A 3


#define TYPE (0)
#define TAG(x) x
#include "r128_elttmp.h"

#define TYPE (R128_RGBA_BIT)
#define TAG(x) x##_RGBA
#include "r128_elttmp.h"

#define TYPE (R128_TEX0_BIT)
#define TAG(x) x##_TEX0
#include "r128_elttmp.h"

#define TYPE (R128_RGBA_BIT|R128_TEX0_BIT)
#define TAG(x) x##_RGBA_TEX0
#include "r128_elttmp.h"

#define TYPE (R128_RGBA_BIT|R128_TEX0_BIT|R128_TEX1_BIT)
#define TAG(x) x##_RGBA_TEX0_TEX1
#include "r128_elttmp.h"

#define TYPE (R128_TEX0_BIT|R128_TEX1_BIT)
#define TAG(x) x##_TEX0_TEX1
#include "r128_elttmp.h"


/* Very sparsely popluated array - fix the indices.
 */
static struct r128_elt_tab r128EltTab[0x80];

void r128DDEltPathInit( void )
{
   r128_render_init_elt();
   r128_render_init_elt_unclipped();

   r128_init_eltpath( &r128EltTab[0] );
   r128_init_eltpath_RGBA( &r128EltTab[R128_RGBA_BIT] );
   r128_init_eltpath_TEX0( &r128EltTab[R128_TEX0_BIT] );
   r128_init_eltpath_RGBA_TEX0( &r128EltTab[R128_RGBA_BIT|R128_TEX0_BIT] );
   r128_init_eltpath_TEX0_TEX1( &r128EltTab[R128_TEX0_BIT|R128_TEX1_BIT] );
   r128_init_eltpath_RGBA_TEX0_TEX1( &r128EltTab[R128_RGBA_BIT|R128_TEX0_BIT|R128_TEX1_BIT] );
}

#define VALID_SETUP (R128_RGBA_BIT|R128_TEX0_BIT|R128_TEX1_BIT)



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
void r128DDEltPath( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   struct r128_elt_tab *tab = &r128EltTab[r128ctx->SetupIndex & VALID_SETUP];
   GLint vertsize = r128ctx->vertsize;
   GLint space;

   if ( DEBUG_ELTPATH )
      fprintf( stderr, "\n\n\n%s: count=%d space=%d\n",
	       __FUNCTION__, VB->Count,
	       (GLint)((GLuint)r128ctx->next_vert -
		       (GLuint)r128ctx->next_elt) );

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

   if ( VB->ClipAndMask ) {
      if ( DEBUG_ELTPATH )
	 fprintf( stderr, "    clipped, returning...\n" );
      return;
   }
   if ( r128ctx->vert_buf ) {
      r128FlushVertices( r128ctx );
   }
   if ( r128ctx->new_state ) {
      r128DDUpdateHWState( ctx );
   }

   space = (GLint)((GLuint)r128ctx->next_vert -
		   (GLuint)r128ctx->next_elt);

   /* Allocate a single buffer to hold unclipped vertices.  All
    * unclipped vertices must be contiguous.
    */
   if ( DEBUG_ELTPATH )
      fprintf( stderr, "   top nv=%p ne=%p space=%d reqd=%d count=%d   clip=0x%x\n\n",
	       r128ctx->next_vert, r128ctx->next_elt, space,
	       (GLint)VB->Count * vertsize * sizeof(GLuint),
	       VB->Count, VB->ClipOrMask );

   /* Because we need to adjust the next_elt pointer to accomodate the
    * CCE packet header, we can sometimes go past the next_vert pointer
    * and thus have negative space.
    */
   if ( space < (GLint)VB->Count * vertsize * (GLint)sizeof(GLuint) ||
	r128ctx->vertsize != r128ctx->elt_vertsize ) {
      fire_elts( r128ctx );
   }

   r128ctx->retained_buf = r128ctx->elt_buf;

   /* Emit unclipped vertices to the buffer.
    */
   tab->emit_unclipped_verts( VB );

   /* Emit indices and clipped vertices to one or more buffers.
    */
   if ( VB->ClipOrMask ) {
      r128ctx->elt_tab = tab;
      r128_render_tab_elt[prim]( VB, 0, VB->EltPtr->count, 0 );
   } else {
      r128_render_tab_elt_unclipped[prim]( VB, 0, VB->EltPtr->count, 0 );
   }

   /* Send to hardware and release the elt buffer.
    */
   release_bufs( r128ctx );

   /* This indicates that there is no cached data to reuse.
    */
   VB->pipeline->data_valid = 0;
   VB->pipeline->new_state = 0;

   if ( DEBUG_ELTPATH ) {
      FLUSH_BATCH( r128ctx );

      LOCK_HARDWARE( r128ctx );
      drmR128WaitForIdleCCE( r128ctx->driFd );
      UNLOCK_HARDWARE( r128ctx );

      __asm__ __volatile__ ( "int $3" );
   }
}
