/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_tris.c,v 1.2 2001/03/21 16:14:25 dawes Exp $ */
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
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#include "radeon_context.h"
#include "radeon_ioctl.h"
#include "radeon_vb.h"
#include "radeon_tris.h"
#include "radeon_state.h"

#include "pipeline.h"
#include "vbindirect.h"

static struct {
   points_func		points;
   line_func		line;
   triangle_func	triangle;
   quad_func		quad;
} rast_tab[RADEON_MAX_TRIFUNC];

#define RADEON_COLOR( to, from )					\
do {									\
   *(GLuint *)(to) = *(GLuint *)(from);					\
} while (0)

#define RADEON_COLOR3( to, from )		\
do {						\
  (to)[0] = (from)[2];				\
  (to)[1] = (from)[1];				\
  (to)[2] = (from)[0];				\
} while (0)


static void radeon_null_quad( GLcontext *ctx, GLuint v0,
			      GLuint v1, GLuint v2, GLuint v3, GLuint pv )
{
}
static void radeon_null_triangle( GLcontext *ctx, GLuint v0,
				  GLuint v1, GLuint v2, GLuint pv )
{
}
static void radeon_null_line( GLcontext *ctx,
			      GLuint v1, GLuint v2, GLuint pv )
{
}
static void radeon_null_points( GLcontext *ctx, GLuint first, GLuint last )
{
}

static void radeonPrintRenderState( const char *msg, GLuint state )
{
   fprintf( stderr, "%s: (0x%x) %s%s%s%s%s\n",
	    msg, state,
	    (state & RADEON_FLAT_BIT)       ? "flat, "       : "",
	    (state & RADEON_OFFSET_BIT)     ? "offset, "     : "",
	    (state & RADEON_TWOSIDE_BIT)    ? "twoside, "    : "",
	    (state & RADEON_NODRAW_BIT)     ? "no-draw, "    : "",
	    (state & RADEON_FALLBACK_BIT)   ? "fallback"     : "" );
}

#define IND (0)
#define TAG(x) x
#include "radeon_tritmp.h"

#define IND (RADEON_FLAT_BIT)
#define TAG(x) x##_flat
#include "radeon_tritmp.h"

#define IND (RADEON_OFFSET_BIT)
#define TAG(x) x##_offset
#include "radeon_tritmp.h"

#define IND (RADEON_OFFSET_BIT | RADEON_FLAT_BIT)
#define TAG(x) x##_offset_flat
#include "radeon_tritmp.h"

#define IND (RADEON_TWOSIDE_BIT)
#define TAG(x) x##_twoside
#include "radeon_tritmp.h"

#define IND (RADEON_TWOSIDE_BIT | RADEON_FLAT_BIT)
#define TAG(x) x##_twoside_flat
#include "radeon_tritmp.h"

#define IND (RADEON_TWOSIDE_BIT | RADEON_OFFSET_BIT)
#define TAG(x) x##_twoside_offset
#include "radeon_tritmp.h"

#define IND (RADEON_TWOSIDE_BIT | RADEON_OFFSET_BIT | RADEON_FLAT_BIT)
#define TAG(x) x##_twoside_offset_flat
#include "radeon_tritmp.h"


void radeonDDTriangleFuncsInit( void )
{
   GLint i;

   init();
   init_flat();
   init_offset();
   init_offset_flat();
   init_twoside();
   init_twoside_flat();
   init_twoside_offset();
   init_twoside_offset_flat();

   for ( i = 0 ; i < RADEON_MAX_TRIFUNC ; i++ ) {
      if ( i & RADEON_NODRAW_BIT ) {
	 rast_tab[i].points	= radeon_null_points;
	 rast_tab[i].line	= radeon_null_line;
	 rast_tab[i].triangle	= radeon_null_triangle;
	 rast_tab[i].quad	= radeon_null_quad;
      }
   }
}


/* FIXME: Only enable software fallback for stencil in 16 bpp mode after
 * we have hardware stencil support.
 */
#define ALL_FALLBACK	(DD_SELECT | DD_FEEDBACK | DD_STENCIL)
#define POINT_FALLBACK	(ALL_FALLBACK | DD_POINT_SMOOTH | DD_POINT_ATTEN)
#define LINE_FALLBACK	(ALL_FALLBACK | DD_LINE_SMOOTH | DD_LINE_STIPPLE)
#define TRI_FALLBACK	(ALL_FALLBACK | DD_TRI_SMOOTH | DD_TRI_UNFILLED)
#define ANY_FALLBACK	(POINT_FALLBACK | LINE_FALLBACK | TRI_FALLBACK)
#define ANY_RASTER_FLAGS (DD_TRI_LIGHT_TWOSIDE | DD_TRI_OFFSET | DD_Z_NEVER)

/* Setup the Point, Line, Triangle and Quad functions based on the
 * current rendering state.  Wherever possible, use the hardware to
 * render the primitive.  Otherwise, fallback to software rendering.
 */
void radeonDDChooseRenderState( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint flags = ctx->TriangleCaps;
   GLuint index = 0;

   if ( rmesa->Fallback ) {
      rmesa->RenderIndex = RADEON_FALLBACK_BIT;
      /* fixes vorder.c failure: */
      if (flags & DD_TRI_LIGHT_TWOSIDE) {
         rmesa->IndirectTriangles = DD_TRI_LIGHT_TWOSIDE;
      }
      return;
   }

   if ( flags & ANY_RASTER_FLAGS ) {
      if ( flags & DD_FLATSHADE )		index |= RADEON_FLAT_BIT;
      if ( flags & DD_TRI_LIGHT_TWOSIDE )	index |= RADEON_TWOSIDE_BIT;
      if ( flags & DD_TRI_OFFSET )		index |= RADEON_OFFSET_BIT;
      if ( flags & DD_Z_NEVER )			index |= RADEON_NODRAW_BIT;
   }

   rmesa->PointsFunc = rast_tab[index].points;
   rmesa->LineFunc = rast_tab[index].line;
   rmesa->TriangleFunc = rast_tab[index].triangle;
   rmesa->QuadFunc = rast_tab[index].quad;

   rmesa->RenderIndex = index;
   rmesa->IndirectTriangles = 0;

   if ( flags & ANY_FALLBACK ) {
      if ( flags & POINT_FALLBACK ) {
	 rmesa->RenderIndex |= RADEON_FALLBACK_BIT;
	 rmesa->PointsFunc = 0;
	 rmesa->IndirectTriangles |= DD_POINT_SW_RASTERIZE;
      }

      if ( flags & LINE_FALLBACK ) {
	 rmesa->RenderIndex |= RADEON_FALLBACK_BIT;
	 rmesa->LineFunc = 0;
	 rmesa->IndirectTriangles |= DD_LINE_SW_RASTERIZE;
      }

      if ( flags & TRI_FALLBACK ) {
	 rmesa->RenderIndex |= RADEON_FALLBACK_BIT;
	 rmesa->TriangleFunc = 0;
	 rmesa->QuadFunc = 0;
	 rmesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
				      DD_QUAD_SW_RASTERIZE);
      }

      /* fixes vorder.c failure: */
      if (flags & DD_TRI_LIGHT_TWOSIDE) {
         rmesa->IndirectTriangles |= DD_TRI_LIGHT_TWOSIDE;
      }
   }

   if ( 0 ) {
      gl_print_tri_caps( "tricaps", ctx->TriangleCaps );
      radeonPrintRenderState( "radeon render state", rmesa->RenderIndex );
   }
}
