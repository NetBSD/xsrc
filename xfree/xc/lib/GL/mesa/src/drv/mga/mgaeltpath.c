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
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgaeltpath.c,v 1.7 2001/04/10 16:07:50 dawes Exp $ */

#include <stdio.h>

#include "types.h"
#include "enums.h"
#include "cva.h"
#include "vertices.h"
#include "mmath.h"
#include "xform.h"

#include "mgacontext.h"
#include "mgapipeline.h"
#include "mgatris.h"
#include "mgastate.h"
#include "mgavb.h"


/* Always use a full-sized stride for vertices. [FIXME]
 * Stride in the buffers must be a quadword multiple.
 */
#define BUFFER_STRIDE 12
#define CLIP_STRIDE 10


static void fire_elts( mgaContextPtr mmesa )
{
   LOCK_HARDWARE( mmesa );

   /* Fire queued elements and discard that buffer if its contents
    * won't be referenced by future elements.
    */
   if (mmesa->elt_buf) {
      GLuint retain = (mmesa->elt_buf == mmesa->retained_buf);

      if (mmesa->first_elt != mmesa->next_elt) {
	 mgaFireEltsLocked( mmesa,
			    ((char *)mmesa->first_elt -
			     (char *)mmesa->elt_buf->address),
			    ((char *)mmesa->next_elt -
			     (char *)mmesa->elt_buf->address),
			    !retain );
      } else if (!retain)
	 mgaReleaseBufLocked( mmesa, mmesa->elt_buf );

      mmesa->elt_buf = 0;
   }
   else if (mmesa->vertex_dma_buffer)
   {
      mgaFlushVerticesLocked( mmesa );
   }

   mgaGetEltBufLocked( mmesa );

   UNLOCK_HARDWARE( mmesa );

   mmesa->next_vert = (GLfloat *)((char *)mmesa->elt_buf->address +
				  mmesa->elt_buf->total -
				  BUFFER_STRIDE * sizeof(GLfloat));

   mmesa->next_vert_phys = (mmesa->mgaScreen->dmaOffset +
			    mmesa->elt_buf->idx * MGA_BUFFER_SIZE +
			    mmesa->elt_buf->total -
			    BUFFER_STRIDE * sizeof(GLfloat));

   mmesa->first_elt = (GLuint *)mmesa->elt_buf->address;
   mmesa->next_elt = (GLuint *)mmesa->elt_buf->address;

}


static void release_bufs( mgaContextPtr mmesa )
{
   if (mmesa->retained_buf && mmesa->retained_buf != mmesa->elt_buf)
   {
      LOCK_HARDWARE( mmesa );
      if (mmesa->first_elt != mmesa->next_elt) {
	 mgaFireEltsLocked( mmesa,
			    ((char *)mmesa->first_elt -
			     (char *)mmesa->elt_buf->address),
			    ((char *)mmesa->next_elt -
			     (char *)mmesa->elt_buf->address),
			    0 );

	 mmesa->first_elt = mmesa->next_elt;
      }

      mgaReleaseBufLocked( mmesa, mmesa->retained_buf );
      UNLOCK_HARDWARE( mmesa );
   }

   mmesa->retained_buf = 0;
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


static void mga_tri_clip( mgaContextPtr mmesa,
			  struct vertex_buffer *VB,
			  GLuint *elt,
			  GLubyte mask )
{
   struct mga_elt_tab *tab = mmesa->elt_tab;
   mga_interp_func interp = tab->interp;
   GLuint inlist[2][VB_MAX_CLIPPED_VERTS];
   GLuint in = 0;
   GLuint n = 3, next_vert = 3;
   GLuint i;
   GLfloat verts[VB_MAX_CLIPPED_VERTS][CLIP_STRIDE];

   /* Build temporary vertices in clipspace.  This is the potential
    * downside to this path.
    */
   tab->build_tri_verts( mmesa, VB, (GLfloat *)verts, elt );

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
      GLuint space = (char *)mmesa->next_vert - (char *)mmesa->next_elt;

      if (space < n * (BUFFER_STRIDE + 3) * sizeof(GLuint))
         fire_elts(mmesa);

      /* Project the new vertices and emit to dma buffers.  Translate
       * out values to physical addresses for setup dma.
       */
      tab->project_and_emit_verts( mmesa, (GLfloat *)verts, out, n );

      /* Convert the planar polygon to a list of triangles and emit to
       * elt buffers.
       */
      for (i = 2 ; i < n ; i++) {
	 mmesa->next_elt[0] = out[0];
	 mmesa->next_elt[1] = out[i-1];
	 mmesa->next_elt[2] = out[i];
	 mmesa->next_elt += 3;
      }
   }
}




/* Build a table of functions to clip each primitive type.  These
 * produce a list of elements in the appropriate 'reduced' primitive,
 * ie (points, lines, triangles) containing all the clipped and
 * unclipped primitives from the original list.
 */

#define INIT(x)

#define TRI_THRESHOLD         (3 * sizeof(GLuint))

#define UNCLIPPED_VERT(x) (mmesa->first_vert_phys - x * BUFFER_STRIDE * 4)

#define TRIANGLE( e2, e1, e0 )				\
do {							\
   if (((char *)mmesa->next_vert -			\
        (char *)mmesa->next_elt) < TRI_THRESHOLD)	\
      fire_elts(mmesa);					\
   mmesa->next_elt[0] = UNCLIPPED_VERT(e2);		\
   mmesa->next_elt[1] = UNCLIPPED_VERT(e1);		\
   mmesa->next_elt[2] = UNCLIPPED_VERT(e0);		\
   mmesa->next_elt+=3;					\
} while (0)

#define CLIP_TRIANGLE( e2, e1, e0 )				\
do {								\
   GLubyte ormask = mask[e2] | mask[e1] | mask[e0];		\
   if (ormask == 0) {						\
      TRIANGLE( e2, e1, e0 );					\
   } else if ((mask[e2] & mask[e1] & mask[e0]) == 0) {		\
      out[0] = e2;						\
      out[1] = e1;						\
      out[2] = e0;						\
      mga_tri_clip( mmesa, VB, out, ormask );			\
   }								\
} while (0)

#define LOCAL_VARS						\
   mgaContextPtr mmesa = MGA_CONTEXT( VB->ctx );		\
   GLuint *elt = VB->EltPtr->data;				\
   GLuint out[VB_MAX_CLIPPED_VERTS];				\
   GLubyte *mask = VB->ClipMask; \
   (void) mask; (void) out; (void) elt; (void) mmesa;



#define RENDER_POINTS(start, count)
#define RENDER_LINE(i1, i0)
#define RENDER_TRI(i2, i1, i0, pv, parity)		\
do {							\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];	\
   if (parity) e2 = elt[i1], e1 = elt[i2];		\
   CLIP_TRIANGLE( e2, e1, e0 );				\
} while (0)

#define RENDER_QUAD(i3, i2, i1, i0, pv )		\
  CLIP_TRIANGLE(elt[i3], elt[i2], elt[i0]);     	\
  CLIP_TRIANGLE(elt[i2], elt[i1], elt[i0])

#define TAG(x) mga_##x##_elt
#include "render_tmp.h"



#define LOCAL_VARS						\
   mgaContextPtr mmesa = MGA_CONTEXT( VB->ctx );		\
   GLuint *elt = VB->EltPtr->data;				\
   (void) elt; (void) mmesa;

#define RENDER_POINTS(start, count)
#define RENDER_LINE(i1, i0)
#define RENDER_TRI(i2, i1, i0, pv, parity)		\
do {							\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];	\
   if (parity) e2 = elt[i1], e1 = elt[i2];		\
   TRIANGLE( e2, e1, e0 );				\
} while (0)

#define RENDER_QUAD(i3, i2, i1, i0, pv )	\
  TRIANGLE(elt[i3], elt[i2], elt[i0]);		\
  TRIANGLE(elt[i2], elt[i1], elt[i0])

#define TAG(x) mga_##x##_elt_unclipped
#include "render_tmp.h"




static void refresh_projection_matrix( GLcontext *ctx )
{
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);
   GLfloat *m = mmesa->device_matrix;
   GLmatrix *mat = &ctx->Viewport.WindowMap;

   REFRESH_DRAWABLE_INFO(mmesa);

   m[MAT_SX] =   mat->m[MAT_SX];
   m[MAT_TX] =   mat->m[MAT_TX] + mmesa->drawX + .5;
   m[MAT_SY] = (- mat->m[MAT_SY]);
   m[MAT_TY] = (- mat->m[MAT_TY]) + mmesa->driDrawable->h + mmesa->drawY - .5;
   m[MAT_SZ] =   mat->m[MAT_SZ] * mmesa->depth_scale;
   m[MAT_TZ] =   mat->m[MAT_TZ] * mmesa->depth_scale;
}

#define CLIP_UBYTE_B 0
#define CLIP_UBYTE_G 1
#define CLIP_UBYTE_R 2
#define CLIP_UBYTE_A 3


#define TYPE (0)
#define TAG(x) x
#include "mgaelttmp.h"

#define TYPE (MGA_RGBA_BIT)
#define TAG(x) x##_RGBA
#include "mgaelttmp.h"

#define TYPE (MGA_TEX0_BIT)
#define TAG(x) x##_TEX0
#include "mgaelttmp.h"

#define TYPE (MGA_RGBA_BIT|MGA_TEX0_BIT)
#define TAG(x) x##_RGBA_TEX0
#include "mgaelttmp.h"

#define TYPE (MGA_RGBA_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT)
#define TAG(x) x##_RGBA_TEX0_TEX1
#include "mgaelttmp.h"

#define TYPE (MGA_TEX0_BIT|MGA_TEX1_BIT)
#define TAG(x) x##_TEX0_TEX1
#include "mgaelttmp.h"


/* Very sparsely popluated array - fix the indices.
 */
static struct mga_elt_tab mgaEltTab[0x80];

void mgaDDEltPathInit( void )
{
   mga_render_init_elt();
   mga_render_init_elt_unclipped();

   mga_init_eltpath( &mgaEltTab[0] );
   mga_init_eltpath_RGBA( &mgaEltTab[MGA_RGBA_BIT] );
   mga_init_eltpath_TEX0( &mgaEltTab[MGA_TEX0_BIT] );
   mga_init_eltpath_RGBA_TEX0( &mgaEltTab[MGA_RGBA_BIT|MGA_TEX0_BIT] );
   mga_init_eltpath_TEX0_TEX1( &mgaEltTab[MGA_TEX0_BIT|MGA_TEX1_BIT] );
   mga_init_eltpath_RGBA_TEX0_TEX1( &mgaEltTab[MGA_RGBA_BIT|MGA_TEX0_BIT|
						  MGA_TEX1_BIT] );
}

#define VALID_SETUP (MGA_RGBA_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT)



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
void mgaDDEltPath( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   struct mga_elt_tab *tab = &mgaEltTab[mmesa->setupindex & VALID_SETUP];

   VB->ClipPtr = TransformRaw(&VB->Clip, &ctx->ModelProjectMatrix, VB->ObjPtr );

   refresh_projection_matrix( ctx );

   VB->ClipAndMask = ~0;
   VB->ClipOrMask = 0;
   VB->Projected = gl_clip_tab[VB->ClipPtr->size]( VB->ClipPtr,
						   &VB->Win,
						   VB->ClipMask,
						   &VB->ClipOrMask,
						   &VB->ClipAndMask );

   if (VB->ClipAndMask)
      return;

   if (mmesa->vertex_dma_buffer)
      mgaFlushVertices( mmesa );

   if (mmesa->new_state)
      mgaDDUpdateHwState( ctx );

   /* Allocate a single buffer to hold unclipped vertices.  All
    * unclipped vertices must be contiguous.
    */
   if ((char *)mmesa->next_vert - (char *)mmesa->next_elt <
       VB->Count * BUFFER_STRIDE * sizeof(GLuint))
      fire_elts( mmesa );

   mmesa->retained_buf = mmesa->elt_buf;

   /* Emit unclipped vertices to the buffer.
    */
   tab->emit_unclipped_verts( VB );

   /* Emit indices and clipped vertices to one or more buffers.
    */
   if (VB->ClipOrMask) {
      mmesa->elt_tab = tab;
      mga_render_tab_elt[prim]( VB, 0, VB->EltPtr->count, 0 );
   } else
      mga_render_tab_elt_unclipped[prim]( VB, 0, VB->EltPtr->count, 0 );

   /* Send to hardware and release any retained buffers.
    */
   release_bufs( mmesa );

   /* This indicates that there is no cached data to reuse.
    */
   VB->pipeline->data_valid = 0;
   VB->pipeline->new_state = 0;
}
