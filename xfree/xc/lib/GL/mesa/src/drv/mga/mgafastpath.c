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
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgafastpath.c,v 1.6 2000/11/13 23:31:28 dawes Exp $ */

#include <stdio.h>

#include "types.h"
#include "enums.h"
#include "cva.h"
#include "vertices.h"
#include "mmath.h"

#include "mgacontext.h"
#include "mgapipeline.h"
#include "mgatris.h"
#include "mgastate.h"
#include "mgavb.h"


extern void mgaDDResizeVB( struct vertex_buffer *VB, GLuint size );

extern void gl_fast_copy_vb( struct vertex_buffer *VB );
  
struct mga_fast_tab {
   void (*build_vertices)( struct vertex_buffer *VB, GLuint do_cliptest );
   void (*interp)( GLfloat t, GLfloat *O, const GLfloat *I, const GLfloat *J );
};




#define POINT(x) mga_draw_point(mmesa, &ivert[x], psize)
#define LINE(x,y) mga_draw_line(mmesa, &ivert[x], &ivert[y], lwidth)
#define TRI(x,y,z) mga_draw_triangle(mmesa, &ivert[x], &ivert[y], &ivert[z])




/* Direct, and no clipping required.  I haven't written the clip funcs
 * yet, so this is only useful for the fast path.
 */
#define RENDER_POINTS( start, count )		\
do {						\
   GLuint e;					\
   for(e=start;e<count;e++)			\
     POINT(elt[e]);				\
} while (0)

#define RENDER_LINE( i1, i )			\
do {						\
   GLuint e1 = elt[i1], e = elt[i];		\
   LINE( e1, e );				\
} while (0)


#define RENDER_TRI( i2, i1, i, pv, parity)		\
do {							\
{  GLuint e2 = elt[i2], e1 = elt[i1], e = elt[i];	\
  if (parity) {GLuint tmp = e2; e2 = e1; e1 = tmp;}	\
  TRI(e2, e1, e);					\
}} while (0)


#define RENDER_QUAD( i3, i2, i1, i, pv)				\
do {								\
  GLuint e3 = elt[i3], e2 = elt[i2], e1 = elt[i1], e = elt[i];	\
  TRI(e3, e2, e);						\
  TRI(e2, e1, e);						\
} while (0)

#define LOCAL_VARS					\
   mgaVertexPtr ivert = MGA_DRIVER_DATA(VB)->verts;	\
   const GLuint *elt = VB->EltPtr->data;		\
   GLcontext *ctx = VB->ctx;				\
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);	\
   const GLfloat lwidth = ctx->Line.Width;		\
   const GLfloat psize = ctx->Point.Size;		\
   (void) lwidth; (void)psize; (void) ivert;


#define TAG(x) x##_mga_smooth_indirect
#include "render_tmp.h"



static void mga_render_elements_direct( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   GLenum prim = ctx->CVA.elt_mode;
   GLuint nr = VB->EltPtr->count;
   render_func func = render_tab_mga_smooth_indirect[prim];
   GLuint p = 0;
   
   if (mmesa->new_state)
      mgaDDUpdateHwState( ctx );

   do {
      func( VB, 0, nr, 0 );
   } while (ctx->Driver.MultipassFunc &&
	    ctx->Driver.MultipassFunc( VB, ++p ));
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


#define CLIP(SGN,V,PLANE)					\
if (mask & PLANE) {						\
   GLuint *indata = inlist[in];					\
   GLuint *outdata = inlist[in ^= 1];				\
   GLuint nr = n;						\
   GLfloat *J = verts[indata[nr-1]].f;			\
   GLfloat dpJ = (SGN J[V]) + J[3];				\
								\
   inlist[0] = vlist1;						\
   for (i = n = 0 ; i < nr ; i++) {				\
      GLuint elt_i = indata[i];					\
      GLfloat *I = verts[elt_i].f;				\
      GLfloat dpI = (SGN I[V]) + I[3];				\
								\
      if (DIFFERENT_SIGNS(dpI, dpJ)) {				\
								\
	 GLfloat *O = verts[next_vert].f;			\
	 GLfloat t, *in, *out;					\
								\
	 if (NEGATIVE(dpI)) {					\
	     t = dpI / (dpI - dpJ);				\
	     in = I;						\
	     out = J;						\
	 }							\
         else							\
	 {							\
	    t = dpJ / (dpJ - dpI);				\
	    in = J;						\
	    out = I;						\
	 }							\
								\
	 interp(t, O, in, out);					\
								\
	 clipmask[next_vert] = 0;				\
	 outdata[n++] = next_vert++;				\
      }								\
								\
      clipmask[elt_i] |= PLANE;		/* don't set up */	\
								\
      if (!NEGATIVE(dpI)) {					\
	 outdata[n++] = elt_i;					\
	 clipmask[elt_i] &= ~PLANE;	/* set up after all */	\
      }								\
								\
      J = I;							\
      dpJ = dpI;						\
   }								\
								\
   if (n < 3) return;						\
}

#define LINE_CLIP(x,y,z,w,PLANE)		\
if (mask & PLANE) {				\
   GLfloat dpI = DOT4V(I,x,y,z,w);		\
   GLfloat dpJ = DOT4V(J,x,y,z,w);		\
						\
   if (DIFFERENT_SIGNS(dpI, dpJ)) {		\
      GLfloat *O = verts[next_vert].f;	\
      GLfloat t = dpI / (dpI - dpJ);		\
						\
      interp(t, O, I, J);			\
						\
      clipmask[next_vert] = 0;			\
						\
      if (NEGATIVE(dpI)) {			\
  	 clipmask[elts[0]] |= PLANE;		\
	 I = O; elts[0] = next_vert++;		\
      } else {					\
  	 clipmask[elts[1]] |= PLANE;		\
	 J = O;	elts[1] = next_vert++;		\
      }						\
   }						\
   else if (NEGATIVE(dpI))			\
      return;					\
}


static __inline void mga_tri_clip( GLuint **p_elts,
				    mgaVertex *verts,
				    GLubyte *clipmask,
				    GLuint *p_next_vert,
				    GLubyte mask,
				    mga_interp_func interp )
{
   GLuint *elts = *p_elts;
   GLuint next_vert = *p_next_vert;
   GLuint vlist1[VB_MAX_CLIPPED_VERTS];
   GLuint vlist2[VB_MAX_CLIPPED_VERTS];
   GLuint *inlist[2];
   GLuint *out;
   GLuint in = 0;
   GLuint n = 3;
   GLuint i;

   inlist[0] = elts;
   inlist[1] = vlist2;

   CLIP(-,0,CLIP_RIGHT_BIT);
   CLIP(+,0,CLIP_LEFT_BIT);
   CLIP(-,1,CLIP_TOP_BIT);
   CLIP(+,1,CLIP_BOTTOM_BIT);
   CLIP(-,2,CLIP_FAR_BIT);
   CLIP(+,2,CLIP_NEAR_BIT);

   /* Convert the planar polygon to a list of triangles.
    */
   out = inlist[in];
   
   for (i = 2 ; i < n ; i++) {
      elts[0] = out[0];
      elts[1] = out[i-1];
      elts[2] = out[i];
      elts += 3;
   }

   *p_next_vert = next_vert;
   *p_elts = elts;
}


static __inline void mga_line_clip( GLuint **p_elts,
				    mgaVertex *verts,
				    GLubyte *clipmask,
				    GLuint *p_next_vert,
				    GLubyte mask,
				    mga_interp_func interp )
{
   GLuint *elts = *p_elts;
   GLfloat *I = verts[elts[0]].f;
   GLfloat *J = verts[elts[1]].f;
   GLuint next_vert = *p_next_vert;

   LINE_CLIP(1,0,0,-1,CLIP_LEFT_BIT);
   LINE_CLIP(-1,0,0,1,CLIP_RIGHT_BIT);
   LINE_CLIP(0,1,0,-1,CLIP_TOP_BIT);
   LINE_CLIP(0,-1,0,1,CLIP_BOTTOM_BIT);
   LINE_CLIP(0,0,1,-1,CLIP_FAR_BIT);
   LINE_CLIP(0,0,-1,1,CLIP_NEAR_BIT);

   *p_next_vert = next_vert;
   *p_elts += 2;
}



#define CLIP_POINT( e )				\
   if (mask[e])					\
      *out++ = e 

#define CLIP_LINE( e1, e0 )						\
do {									\
   GLubyte ormask = mask[e0] | mask[e1];				\
   out[0] = e1;								\
   out[1] = e0;								\
   out+=2;								\
   if (ormask) {							\
      out-=2;								\
      if (!(mask[e0] & mask[e1])) {					\
	 mga_line_clip( &out, verts, mask, &next_vert, ormask, interp); \
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
   if (ormask) {							\
      out -= 3;								\
      if ( !(mask[e2] & mask[e1] & mask[e0])) {				\
	 mga_tri_clip( &out, verts, mask, &next_vert, ormask, interp ); \
      }									\
   }									\
} while (0)






/* Build a table of functions to clip each primitive type.  These
 * produce a list of elements in the appropriate 'reduced' primitive,
 * ie (points, lines, triangles) containing all the clipped and
 * unclipped primitives from the original list.
 */
#define LOCAL_VARS						\
   mgaContextPtr mmesa = MGA_CONTEXT( VB->ctx );                \
   GLuint *elt = VB->EltPtr->data;				\
   mgaVertex *verts = MGA_DRIVER_DATA(VB)->verts;		\
   GLuint next_vert = MGA_DRIVER_DATA(VB)->last_vert;		\
   GLuint *out = MGA_DRIVER_DATA(VB)->clipped_elements.data;	\
   GLubyte *mask = VB->ClipMask;				\
   mga_interp_func interp = mmesa->interp;                      \
   (void) interp; (void) verts;

#define POSTFIX							\
   MGA_DRIVER_DATA(VB)->clipped_elements.count =		\
          out - MGA_DRIVER_DATA(VB)->clipped_elements.data;	\
   MGA_DRIVER_DATA(VB)->last_vert = next_vert;		


#define INIT(x)

#define RENDER_POINTS(start, count)			\
do {							\
   GLuint i;						\
   for (i = start ; i < count ; i++ )			\
      CLIP_POINT( elt[i] );				\
} while (0)

#define RENDER_LINE(i1, i0)      			\
   CLIP_LINE(elt[i1], elt[i0])

#define RENDER_TRI(i2, i1, i0, pv, parity)		\
do {							\
   GLuint e2 = elt[i2], e1 = elt[i1], e0 = elt[i0];	\
   if (parity) e2 = elt[i1], e1 = elt[i2];		\
   CLIP_TRIANGLE( e2, e1, e0 );				\
} while (0)

#define RENDER_QUAD(i3, i2, i1, i0, pv )		\
  CLIP_TRIANGLE(elt[i3], elt[i2], elt[i0]);     	\
  CLIP_TRIANGLE(elt[i2], elt[i1], elt[i0])

#define TAG(x) mga_clip_##x##_elt
#include "render_tmp.h"



static void mga_project_vertices( struct vertex_buffer *VB )
{
   mgaVertexBufferPtr mgaVB = MGA_DRIVER_DATA(VB);
   GLcontext *ctx = VB->ctx;
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   GLfloat m[16];

   REFRESH_DRAWABLE_INFO(mmesa);

   m[MAT_SX] =   mat->m[MAT_SX];
   m[MAT_TX] =   mat->m[MAT_TX] + mmesa->drawX + SUBPIXEL_X;
   m[MAT_SY] = (- mat->m[MAT_SY]);
   m[MAT_TY] = (- mat->m[MAT_TY]) + mmesa->driDrawable->h + mmesa->drawY + SUBPIXEL_Y;
   m[MAT_SZ] =   mat->m[MAT_SZ] * mmesa->depth_scale;
   m[MAT_TZ] =   mat->m[MAT_TZ] * mmesa->depth_scale;

   gl_project_v16( mgaVB->verts[VB->CopyStart].f,
		   mgaVB->verts[mgaVB->last_vert].f,
		   m,
		   16 * 4 );
}

static void mga_project_clipped_vertices( struct vertex_buffer *VB )
{
   mgaVertexBufferPtr mgaVB = MGA_DRIVER_DATA(VB);
   GLcontext *ctx = VB->ctx;
   mgaContextPtr mmesa = MGA_CONTEXT(ctx);
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   GLfloat m[16];

   REFRESH_DRAWABLE_INFO(mmesa);

   m[MAT_SX] =   mat->m[MAT_SX];
   m[MAT_TX] =   mat->m[MAT_TX] + mmesa->drawX + SUBPIXEL_X;
   m[MAT_SY] = (- mat->m[MAT_SY]);
   m[MAT_TY] = (- mat->m[MAT_TY]) + mmesa->driDrawable->h + mmesa->drawY - SUBPIXEL_Y;
   m[MAT_SZ] =   mat->m[MAT_SZ] * mmesa->depth_scale;
   m[MAT_TZ] =   mat->m[MAT_TZ] * mmesa->depth_scale;

   gl_project_clipped_v16( mgaVB->verts[VB->CopyStart].f,
			   mgaVB->verts[mgaVB->last_vert].f,
			   m,
			   16 * 4,
			   VB->ClipMask + VB->CopyStart );
}


/* Pack rgba and/or texture into the remaining half of a 32 byte vertex.
 */
#define CLIP_UBYTE_COLOR  4
#define CLIP_UBYTE_B 0   
#define CLIP_UBYTE_G 1  
#define CLIP_UBYTE_R 2
#define CLIP_UBYTE_A 3   
#define CLIP_S0 6
#define CLIP_T0 7
#define CLIP_S1 8
#define CLIP_T1 9

#define TYPE (0)
#define TAG(x) x
#include "mgafasttmp.h"

#define TYPE (MGA_RGBA_BIT)
#define TAG(x) x##_RGBA
#include "mgafasttmp.h"

#define TYPE (MGA_TEX0_BIT)
#define TAG(x) x##_TEX0
#include "mgafasttmp.h"

#define TYPE (MGA_RGBA_BIT|MGA_TEX0_BIT)
#define TAG(x) x##_RGBA_TEX0
#include "mgafasttmp.h"

#define TYPE (MGA_RGBA_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT)
#define TAG(x) x##_RGBA_TEX0_TEX1
#include "mgafasttmp.h"

#define TYPE (MGA_TEX0_BIT|MGA_TEX1_BIT)
#define TAG(x) x##_TEX0_TEX1
#include "mgafasttmp.h"


/* Very sparsely popluated array - fix the indices.
 */
static struct mga_fast_tab mgaFastTab[0x80];

void mgaDDFastPathInit( void )
{
   mga_clip_render_init_elt();
   render_init_mga_smooth_indirect();

   mga_init_fastpath( &mgaFastTab[0] );
   mga_init_fastpath_RGBA( &mgaFastTab[MGA_RGBA_BIT] );
   mga_init_fastpath_TEX0( &mgaFastTab[MGA_TEX0_BIT] );
   mga_init_fastpath_RGBA_TEX0( &mgaFastTab[MGA_RGBA_BIT|MGA_TEX0_BIT] );
   mga_init_fastpath_TEX0_TEX1( &mgaFastTab[MGA_TEX0_BIT|MGA_TEX1_BIT] );
   mga_init_fastpath_RGBA_TEX0_TEX1( &mgaFastTab[MGA_RGBA_BIT|MGA_TEX0_BIT|
						  MGA_TEX1_BIT] );
}

#define VALID_SETUP (MGA_RGBA_BIT|MGA_TEX0_BIT|MGA_TEX1_BIT)


void mgaDDFastPath( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLenum prim = ctx->CVA.elt_mode;
   mgaContextPtr mmesa = MGA_CONTEXT( ctx ); 
   struct mga_fast_tab *tab = &mgaFastTab[mmesa->setupindex & VALID_SETUP];
   GLuint do_cliptest = 1;


   gl_prepare_arrays_cva( VB );	                 /* still need this */

#if 1
   if (gl_reduce_prim[prim] == GL_TRIANGLES && 
       VB->Count < (MGA_DMA_BUF_SZ / 48) &&
       (ctx->ModelProjectMatrix.flags & (MAT_FLAG_GENERAL|
					 MAT_FLAG_PERSPECTIVE)) &&
       mmesa->mgaScreen->chipset == MGA_CARD_TYPE_G400) 
   {
      mgaDDEltPath( VB );
      return;
   }
#endif

   /* Reserve enough space for the pathological case.
    */
   if (VB->EltPtr->count * 12 > MGA_DRIVER_DATA(VB)->size) {
      mgaDDResizeVB( VB, VB->EltPtr->count * 12 );
      do_cliptest = 1;
   }

   tab->build_vertices( VB, do_cliptest );       /* object->clip space */

   if (mmesa->new_state)
      mgaDDUpdateHwState( ctx );

   if (VB->ClipOrMask) {
      if (!VB->ClipAndMask) {
	 render_func *clip = mga_clip_render_tab_elt;
      
	 mmesa->interp = tab->interp;
      
	 clip[prim]( VB, 0, VB->EltPtr->count, 0 ); /* build new elts */

	 ctx->CVA.elt_mode = gl_reduce_prim[prim];
	 VB->EltPtr = &(MGA_DRIVER_DATA(VB)->clipped_elements);

	 mga_project_clipped_vertices( VB );    /* clip->device space */
	 mga_render_elements_direct( VB );        /* render using new list */
      }
   } else {
      mga_project_vertices( VB );               /* clip->device space  */
      mga_render_elements_direct( VB );           /* render using orig list */
   }

   /* This indicates that there is no cached data to reuse.  
    */
   VB->pipeline->data_valid = 0;
   VB->pipeline->new_state = 0;
}

