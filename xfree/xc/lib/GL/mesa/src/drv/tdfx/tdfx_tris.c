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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_tris.c,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

#include "tdfx_context.h"
#include "tdfx_tris.h"

#include "pipeline.h"
#include "vbindirect.h"

static struct {
   points_func		points;
   line_func    	line;
   triangle_func	triangle;
   quad_func		quad;
   render_func          *render_tab;
} rast_tab[TDFX_MAX_TRIFUNC];

#define TDFX_COLOR( to, from )						\
   do {									\
      (to)[0] = (from)[2];						\
      (to)[1] = (from)[1];						\
      (to)[2] = (from)[0];						\
      (to)[3] = (from)[3];						\
   } while (0)


static void tdfxPrintRenderState( const char *msg, GLuint state )
{
   fprintf( stderr, "%s: (0x%x) %s%s%s%s\n",
	    msg, state,
	    (state & TDFX_FLAT_BIT)       ? "flat, "       : "",
	    (state & TDFX_OFFSET_BIT)     ? "offset, "     : "",
	    (state & TDFX_TWOSIDE_BIT)    ? "twoside, "    : "",
	    (state & TDFX_CLIPRECT_BIT)   ? "cliprects, "  : "");
}


#define IND (0)
#define TAG(x) x
#include "tdfx_tritmp.h"

#define IND (TDFX_FLAT_BIT)
#define TAG(x) x##_flat
#include "tdfx_tritmp.h"

#define IND (TDFX_OFFSET_BIT)
#define TAG(x) x##_offset
#include "tdfx_tritmp.h"

#define IND (TDFX_OFFSET_BIT | TDFX_FLAT_BIT)
#define TAG(x) x##_offset_flat
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT)
#define TAG(x) x##_twoside
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_FLAT_BIT)
#define TAG(x) x##_twoside_flat
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_OFFSET_BIT)
#define TAG(x) x##_twoside_offset
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_OFFSET_BIT | TDFX_FLAT_BIT)
#define TAG(x) x##_twoside_offset_flat
#include "tdfx_tritmp.h"

#define IND (TDFX_CLIPRECT_BIT)
#define TAG(x) x##_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_FLAT_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_flat_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_OFFSET_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_offset_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_OFFSET_BIT | TDFX_FLAT_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_offset_flat_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_twoside_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_FLAT_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_twoside_flat_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_OFFSET_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_twoside_offset_cliprect
#include "tdfx_tritmp.h"

#define IND (TDFX_TWOSIDE_BIT | TDFX_OFFSET_BIT | TDFX_FLAT_BIT | TDFX_CLIPRECT_BIT)
#define TAG(x) x##_twoside_offset_flat_cliprect
#include "tdfx_tritmp.h"

static void tdfx_render_vb_points( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   GLcontext *ctx = VB->ctx;
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   GLint i;
   (void) parity;
   /* Adjust point coords */
   for (i = start; i < count; i++) {
      fxVB[i].v.x += PNT_X_OFFSET - TRI_X_OFFSET;
      fxVB[i].v.y += PNT_Y_OFFSET - TRI_Y_OFFSET;
   }
   grDrawVertexArrayContiguous( GR_POINTS, count-start, fxVB+start,
				sizeof(*fxVB));
   /* restore point coords */
   for (i = start; i < count; i++) {
      fxVB[i].v.x -= PNT_X_OFFSET - TRI_X_OFFSET;
      fxVB[i].v.y -= PNT_Y_OFFSET - TRI_Y_OFFSET;
   }
}

static void tdfx_render_vb_line_strip( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   GLint i;
   (void) parity;
   /* adjust line coords */
   for (i = start; i < count; i++) {
      fxVB[i].v.x += LINE_X_OFFSET - TRI_X_OFFSET;
      fxVB[i].v.y += LINE_Y_OFFSET - TRI_Y_OFFSET;
   }
   grDrawVertexArrayContiguous( GR_LINE_STRIP, count-start, fxVB+start,
				sizeof(*fxVB));
   /* restore line coords */
   for (i = start; i < count; i++) {
      fxVB[i].v.x -= LINE_X_OFFSET - TRI_X_OFFSET;
      fxVB[i].v.y -= LINE_Y_OFFSET - TRI_Y_OFFSET;
   }
}

static void tdfx_render_vb_lines( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   GLint i;
   (void) parity;
   /* adjust line coords */
   for (i = start; i < count; i++) {
      fxVB[i].v.x += LINE_X_OFFSET - TRI_X_OFFSET;
      fxVB[i].v.y += LINE_Y_OFFSET - TRI_Y_OFFSET;
   }
   grDrawVertexArrayContiguous( GR_LINES, count-start, fxVB+start,
				sizeof(*fxVB));
   /* restore line coords */
   for (i = start; i < count; i++) {
      fxVB[i].v.x -= LINE_X_OFFSET - TRI_X_OFFSET;
      fxVB[i].v.y -= LINE_Y_OFFSET - TRI_Y_OFFSET;
   }
}

static void tdfx_render_vb_triangles( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   grDrawVertexArrayContiguous( GR_TRIANGLES, count-start, fxVB+start,
				sizeof(*fxVB));
   (void) parity;
}


static void tdfx_render_vb_tri_strip( struct vertex_buffer *VB,
				      GLuint start,
				      GLuint count,
				      GLuint parity )
{
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   grDrawVertexArrayContiguous( GR_TRIANGLE_STRIP, count-start, fxVB+start,
				sizeof(*fxVB));
   (void) parity;
}


static void tdfx_render_vb_tri_fan( struct vertex_buffer *VB,
				    GLuint start,
				    GLuint count,
				    GLuint parity )
{
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   grDrawVertexArrayContiguous( GR_TRIANGLE_FAN, count-start, fxVB+start,
				sizeof(*fxVB) );
   (void) parity;
}


static void tdfx_render_vb_poly( struct vertex_buffer *VB,
				 GLuint start,
				 GLuint count,
				 GLuint parity )
{
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   grDrawVertexArrayContiguous( GR_POLYGON, count-start, fxVB+start,
				sizeof(*fxVB));
   (void) parity;
}


#define RENDER_POINTS( start, count ) 			\
do {							\
   int i;						\
   for (i = start ; i < count ; i++) {			\
      v[elt[i]].v.x += PNT_X_OFFSET - TRI_X_OFFSET;	\
      v[elt[i]].v.y += PNT_Y_OFFSET - TRI_Y_OFFSET;	\
      grDrawPoint(&v[elt[i]]);				\
      v[elt[i]].v.x -= PNT_X_OFFSET - TRI_X_OFFSET;	\
      v[elt[i]].v.y -= PNT_Y_OFFSET - TRI_Y_OFFSET;	\
   }							\
} while (0)

#define RENDER_LINE( i0, i1 )				\
do {							\
   v[elt[i0]].v.x += LINE_X_OFFSET - TRI_X_OFFSET;	\
   v[elt[i0]].v.y += LINE_Y_OFFSET - TRI_Y_OFFSET;	\
   v[elt[i1]].v.x += LINE_X_OFFSET - TRI_X_OFFSET;	\
   v[elt[i1]].v.y += LINE_Y_OFFSET - TRI_Y_OFFSET;	\
   grDrawLine( &v[elt[i0]], &v[elt[i1]] );		\
   v[elt[i0]].v.x -= LINE_X_OFFSET - TRI_X_OFFSET;	\
   v[elt[i0]].v.y -= LINE_Y_OFFSET - TRI_Y_OFFSET;	\
   v[elt[i1]].v.x -= LINE_X_OFFSET - TRI_X_OFFSET;	\
   v[elt[i1]].v.y -= LINE_Y_OFFSET - TRI_Y_OFFSET;	\
} while (0)

#define RENDER_TRI( i2, i1, i, pv, parity )				\
do {									\
   if (parity) grDrawTriangle( &v[elt[i1]], &v[elt[i2]], &v[elt[i]] );	\
   else        grDrawTriangle( &v[elt[i2]], &v[elt[i1]], &v[elt[i]] );	\
} while (0)

#define RENDER_QUAD( i3, i2, i1, i, pv )			\
do {								\
   grDrawTriangle( &v[elt[i3]], &v[elt[i2]], &v[elt[i]] );	\
   grDrawTriangle( &v[elt[i2]], &v[elt[i1]], &v[elt[i]] );	\
} while (0)


#define LOCAL_VARS				\
   GLcontext *ctx = VB->ctx;			\
   const GLuint *elt = VB->EltPtr->data;        \
   tdfxVertexPtr v = TDFX_DRIVER_DATA(VB)->verts; \
   (void) v; (void) ctx;

#define TAG(x) tdfx_##x##_elts
#include "render_tmp.h"


static void tdfxDDRenderEltsRaw( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   struct vertex_buffer *OldVB = ctx->VB;
   GLenum prim = ctx->CVA.elt_mode;
   GLuint nr = VB->EltPtr->count;
   render_func func = tdfx_render_tab_elts[prim];
   GLuint p = 0;

   ctx->VB = VB;
   ctx->Driver.RenderStart( ctx );

   BEGIN_CLIP_LOOP_LOCKED( fxMesa );
   do {
      func( VB, 0, nr, 0 );
   } while ( ctx->Driver.MultipassFunc &&
	     ctx->Driver.MultipassFunc( VB, ++p ) );
   END_CLIP_LOOP_LOCKED( fxMesa );

   ctx->Driver.RenderFinish( ctx );
   ctx->VB = OldVB;
}


void tdfxDDTriangleFuncsInit( void )
{
   init();
   init_flat();
   init_offset();
   init_offset_flat();
   init_twoside();
   init_twoside_flat();
   init_twoside_offset();
   init_twoside_offset_flat();

   init_cliprect();
   init_flat_cliprect();
   init_offset_cliprect();
   init_offset_flat_cliprect();
   init_twoside_cliprect();
   init_twoside_flat_cliprect();
   init_twoside_offset_cliprect();
   init_twoside_offset_flat_cliprect();

   rast_tab[0].render_tab[GL_POINTS] = tdfx_render_vb_points;
   rast_tab[0].render_tab[GL_LINE_STRIP] = tdfx_render_vb_line_strip;
   rast_tab[0].render_tab[GL_LINES] = tdfx_render_vb_lines;
   rast_tab[0].render_tab[GL_TRIANGLES] = tdfx_render_vb_triangles;
   rast_tab[0].render_tab[GL_TRIANGLE_STRIP] = tdfx_render_vb_tri_strip;
   rast_tab[0].render_tab[GL_TRIANGLE_FAN] = tdfx_render_vb_tri_fan;
   rast_tab[0].render_tab[GL_POLYGON] = tdfx_render_vb_poly;

   tdfx_render_init_elts();
}


#define ALL_FALLBACK	(DD_SELECT | DD_FEEDBACK)
#define POINT_FALLBACK	(ALL_FALLBACK | DD_POINT_SMOOTH | DD_POINT_ATTEN)
#define LINE_FALLBACK	(ALL_FALLBACK | DD_LINE_STIPPLE)
#define TRI_FALLBACK	(ALL_FALLBACK | DD_TRI_SMOOTH | DD_TRI_UNFILLED)
#define ANY_FALLBACK	(POINT_FALLBACK | LINE_FALLBACK | TRI_FALLBACK | DD_TRI_STIPPLE | DD_LINE_SMOOTH | DD_LINE_WIDTH | DD_POINT_SIZE )
#define ANY_RENDER_FLAGS (DD_FLATSHADE | DD_TRI_LIGHT_TWOSIDE | DD_TRI_OFFSET)

/* Setup the Point, Line, Triangle and Quad functions based on the
 * current rendering state.  Wherever possible, use the hardware to
 * render the primitive.  Otherwise, fallback to software rendering.
 */
void tdfxDDChooseRenderState( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   GLuint flags = ctx->TriangleCaps;
   CARD32 index = 0;
   fxMesa->RenderElementsRaw = tdfxDDRenderEltsRaw;

   if ( fxMesa->Fallback ) {
      fxMesa->RenderElementsRaw = gl_render_elts;
      fxMesa->RenderIndex = TDFX_FALLBACK_BIT;
      return;
   }

   if ( flags & ANY_RENDER_FLAGS ) {
      if ( flags & DD_FLATSHADE )		index |= TDFX_FLAT_BIT;
      if ( flags & DD_TRI_LIGHT_TWOSIDE )	index |= TDFX_TWOSIDE_BIT;
      if ( flags & DD_TRI_OFFSET )		index |= TDFX_OFFSET_BIT;
      fxMesa->RenderElementsRaw = gl_render_elts;
   }

   if ( fxMesa->numClipRects > 1 )
      index |= TDFX_CLIPRECT_BIT;

   fxMesa->PointsFunc	= rast_tab[index].points;
   fxMesa->LineFunc	= rast_tab[index].line;
   fxMesa->TriangleFunc	= rast_tab[index].triangle;
   fxMesa->QuadFunc	= rast_tab[index].quad;
   fxMesa->RenderVBRawTab = rast_tab[index].render_tab;
   fxMesa->RenderIndex = index;
   fxMesa->IndirectTriangles = 0;

   if ( flags & ANY_FALLBACK ) {
      if ( flags & POINT_FALLBACK ) {
	 fxMesa->PointsFunc = 0;
	 fxMesa->RenderVBRawTab = 0;
	 fxMesa->IndirectTriangles |= DD_POINT_SW_RASTERIZE;
	 fxMesa->RenderIndex |= TDFX_FALLBACK_BIT;
      }

      if ( flags & LINE_FALLBACK ) {
	 fxMesa->LineFunc = 0;
	 fxMesa->RenderVBRawTab = 0;
	 fxMesa->IndirectTriangles |= DD_LINE_SW_RASTERIZE;
	 fxMesa->RenderIndex |= TDFX_FALLBACK_BIT;
      }

      if ( flags & TRI_FALLBACK ) {
	 fxMesa->TriangleFunc = 0;
	 fxMesa->QuadFunc = 0;
	 fxMesa->RenderVBRawTab = 0;
	 fxMesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
				       DD_QUAD_SW_RASTERIZE);
	 fxMesa->RenderIndex |= TDFX_FALLBACK_BIT;
      }

      /* Special case:  wide, AA lines must be done in software */
      if (flags & DD_LINE_SMOOTH) {
         if (ctx->Line.Width != 1.0) {
	    fxMesa->RenderVBRawTab = 0;
            fxMesa->LineFunc = 0;
            fxMesa->IndirectTriangles |= DD_LINE_SW_RASTERIZE;
            fxMesa->RenderIndex |= TDFX_FALLBACK_BIT;
         }
      }

      /* Special case:  we can do polygon stipples, but otherwise */
      if ((flags & DD_TRI_STIPPLE) &&
	 (ctx->IndirectTriangles & DD_TRI_STIPPLE)) {
	 fxMesa->TriangleFunc = 0;
	 fxMesa->QuadFunc = 0;
	 fxMesa->RenderVBRawTab = 0;
	 fxMesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
				       DD_QUAD_SW_RASTERIZE);
	 fxMesa->RenderIndex |= TDFX_FALLBACK_BIT;
      }

      if (flags & (DD_LINE_WIDTH | DD_POINT_SIZE))
	 fxMesa->RenderVBRawTab = 0;

      fxMesa->RenderElementsRaw = gl_render_elts;
   }


   if ( 0 ) {
      gl_print_tri_caps( "tricaps", ctx->TriangleCaps );
      tdfxPrintRenderState( "tdfx render state", fxMesa->RenderIndex );
   }
}


void tdfxDDToggleTriCliprects( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   int oldidx = fxMesa->RenderIndex;
   int newidx;

   if (fxMesa->Fallback)
      return;

   if (fxMesa->numClipRects > 1)
      newidx = (fxMesa->RenderIndex |= TDFX_CLIPRECT_BIT);
   else
      newidx = (fxMesa->RenderIndex &= ~TDFX_CLIPRECT_BIT);

   if (ctx->Driver.TriangleFunc == rast_tab[oldidx].triangle)
      ctx->Driver.TriangleFunc = rast_tab[newidx].triangle;

   if (ctx->Driver.QuadFunc == rast_tab[oldidx].quad)
      ctx->Driver.QuadFunc = rast_tab[newidx].quad;

   if (ctx->Driver.LineFunc == rast_tab[oldidx].line)
      ctx->Driver.LineFunc = rast_tab[newidx].line;

   if (ctx->Driver.PointsFunc == rast_tab[oldidx].points)
      ctx->Driver.PointsFunc = rast_tab[newidx].points;

   if (ctx->Driver.RenderVBRawTab == rast_tab[oldidx].render_tab)
      ctx->Driver.RenderVBRawTab = rast_tab[newidx].render_tab;

   if (ctx->TriangleFunc == rast_tab[oldidx].triangle)
      ctx->TriangleFunc = rast_tab[newidx].triangle;

   if (ctx->QuadFunc == rast_tab[oldidx].quad)
      ctx->QuadFunc = rast_tab[newidx].quad;

   fxMesa->PointsFunc	= rast_tab[newidx].points;
   fxMesa->LineFunc	= rast_tab[newidx].line;
   fxMesa->TriangleFunc	= rast_tab[newidx].triangle;
   fxMesa->QuadFunc	= rast_tab[newidx].quad;
   fxMesa->RenderVBRawTab = rast_tab[newidx].render_tab;

   if (newidx == 0 &&
       (ctx->IndirectTriangles & (DD_LINE_WIDTH|DD_POINT_SIZE)) == 0)
      fxMesa->RenderElementsRaw = tdfxDDRenderEltsRaw;
   else
      fxMesa->RenderElementsRaw = gl_render_elts;

   if (0)
      tdfxPrintRenderState( "toggle tdfx render state", fxMesa->RenderIndex );
}
