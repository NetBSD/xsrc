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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_tritmp.h,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

static __inline void TAG(triangle)( GLcontext *ctx,
				    GLuint e0, GLuint e1, GLuint e2,
				    GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   tdfxVertexPtr fxverts = TDFX_DRIVER_DATA(VB)->verts;
   tdfxVertex *v[3];

#if (IND & TDFX_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[3];
#endif

#if (IND & (TDFX_TWOSIDE_BIT | TDFX_FLAT_BIT))
   GLuint c[3];
#endif

#if (IND & TDFX_CLIPRECT_BIT)
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
#endif

   v[0] = &fxverts[e0];
   v[1] = &fxverts[e1];
   v[2] = &fxverts[e2];

#if (IND & (TDFX_TWOSIDE_BIT | TDFX_FLAT_BIT))
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
#endif


#if (IND & (TDFX_TWOSIDE_BIT | TDFX_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc = ex*fy - ey*fx;

#if (IND & TDFX_TWOSIDE_BIT)
      {
	 GLuint facing = ( cc < 0.0 ) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 if (IND & TDFX_FLAT_BIT) {
	    TDFX_COLOR( (char *)&v[0]->ui[4], vbcolor[pv] );
	    v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
	 } else {
	    TDFX_COLOR( (char *)&v[0]->ui[4], vbcolor[e0] );
	    TDFX_COLOR( (char *)&v[1]->ui[4], vbcolor[e1] );
	    TDFX_COLOR( (char *)&v[2]->ui[4], vbcolor[e2] );
	 }
      }
#endif

#if (IND & TDFX_OFFSET_BIT)
      {
	 offset = ctx->Polygon.OffsetUnits;
	 z[0] = v[0]->v.z;
	 z[1] = v[1]->v.z;
	 z[2] = v[2]->v.z;
	 if (cc * cc > 1e-16) {
	    GLfloat ez	= z[0] - z[2];
	    GLfloat fz	= z[1] - z[2];
	    GLfloat a	= ey*fz - ez*fy;
	    GLfloat b	= ez*fx - ex*fz;
	    GLfloat ic	= 1.0 / cc;
	    GLfloat ac	= a * ic;
	    GLfloat bc	= b * ic;
	    if ( ac < 0.0f ) ac = -ac;
	    if ( bc < 0.0f ) bc = -bc;
	    offset += MAX2( ac, bc ) * ctx->Polygon.OffsetFactor;

	 }
	 v[0]->v.z += offset;
	 v[1]->v.z += offset;
	 v[2]->v.z += offset;
      }
#endif
   }
#elif (IND & TDFX_FLAT_BIT)
   {
      GLuint color = fxverts[pv].ui[4];
      v[0]->ui[4] = color;
      v[1]->ui[4] = color;
      v[2]->ui[4] = color;
   }
#endif

#if (IND & TDFX_CLIPRECT_BIT)
   BEGIN_CLIP_LOOP_LOCKED( fxMesa );
   grDrawTriangle( v[0], v[1], v[2] );
   END_CLIP_LOOP_LOCKED( fxMesa );
#else
   grDrawTriangle( v[0], v[1], v[2] );
#endif

#if (IND & TDFX_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
#endif

#if (IND & (TDFX_FLAT_BIT | TDFX_TWOSIDE_BIT))
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
   v[2]->ui[4] = c[2];
#endif

}



static __inline void TAG(quad)( GLcontext *ctx, GLuint e0,
				GLuint e1, GLuint e2, GLuint e3,
				GLuint pv )
{
   struct vertex_buffer *VB = ctx->VB;
   tdfxVertexPtr fxverts = TDFX_DRIVER_DATA(VB)->verts;
   tdfxVertex *v[4];

#if (IND & TDFX_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[4];
#endif

#if (IND & (TDFX_TWOSIDE_BIT | TDFX_FLAT_BIT))
   GLuint c[4];
#endif

#if (IND & TDFX_CLIPRECT_BIT)
    tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
#endif


   v[0] = &fxverts[e0];
   v[1] = &fxverts[e1];
   v[2] = &fxverts[e2];
   v[3] = &fxverts[e3];


/*     fprintf(stderr, "%s\n", __FUNCTION__); */

#if (IND & (TDFX_TWOSIDE_BIT | TDFX_FLAT_BIT))
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
   c[3] = v[3]->ui[4];
#endif


#if (IND & (TDFX_TWOSIDE_BIT | TDFX_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc = ex*fy - ey*fx;

#if (IND & TDFX_TWOSIDE_BIT)
      {
	 GLuint facing = ( cc < 0.0 ) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 if (IND & TDFX_FLAT_BIT) {
	    TDFX_COLOR( (char *)&v[0]->ui[4], vbcolor[pv] );
	    v[3]->ui[4] = v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
	 } else {
	    TDFX_COLOR( (char *)&v[0]->ui[4], vbcolor[e0] );
	    TDFX_COLOR( (char *)&v[1]->ui[4], vbcolor[e1] );
	    TDFX_COLOR( (char *)&v[2]->ui[4], vbcolor[e2] );
	    TDFX_COLOR( (char *)&v[3]->ui[4], vbcolor[e3] );
	 }
      }
#endif

#if (IND & TDFX_OFFSET_BIT)
      {
	 offset = ctx->Polygon.OffsetUnits;
	 z[0] = v[0]->v.z;
	 z[1] = v[1]->v.z;
	 z[2] = v[2]->v.z;
	 z[3] = v[3]->v.z;
	 if (cc * cc > 1e-16) {
	    GLfloat ez	= z[0] - z[2];
	    GLfloat fz	= z[1] - z[2];
	    GLfloat a	= ey*fz - ez*fy;
	    GLfloat b	= ez*fx - ex*fz;
	    GLfloat ic	= 1.0 / cc;
	    GLfloat ac	= a * ic;
	    GLfloat bc	= b * ic;
	    if ( ac < 0.0f ) ac = -ac;
	    if ( bc < 0.0f ) bc = -bc;
	    offset += MAX2( ac, bc ) * ctx->Polygon.OffsetFactor;

	 }
	 v[0]->v.z += offset;
	 v[1]->v.z += offset;
	 v[2]->v.z += offset;
	 v[3]->v.z += offset;
      }
#endif
   }
#elif (IND & TDFX_FLAT_BIT)
   {
      GLuint color = fxverts[pv].ui[4];
      v[0]->ui[4] = color;
      v[1]->ui[4] = color;
      v[2]->ui[4] = color;
      v[3]->ui[4] = color;
   }
#endif

/* Marginally faster to call grDrawTriangle twice
 * than calling grDrawVertexArray.
 */
#if (IND & TDFX_CLIPRECT_BIT)
   BEGIN_CLIP_LOOP_LOCKED( fxMesa );
/*     grDrawVertexArray( GR_TRIANGLE_FAN, 4, v );  */
   grDrawTriangle( v[0], v[1], v[3] );
   grDrawTriangle( v[1], v[2], v[3] );
   END_CLIP_LOOP_LOCKED( fxMesa );
#else
/*     grDrawVertexArray( GR_TRIANGLE_FAN, 4, v );  */
   grDrawTriangle( v[0], v[1], v[3] );
   grDrawTriangle( v[1], v[2], v[3] );
#endif

#if (IND & TDFX_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
   v[3]->v.z = z[3];
#endif

#if (IND & (TDFX_FLAT_BIT | TDFX_TWOSIDE_BIT))
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
   v[2]->ui[4] = c[2];
   v[3]->ui[4] = c[3];
#endif
}





static __inline void TAG(line)( GLcontext *ctx, GLuint v0, GLuint v1,
				GLuint pv )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(ctx->VB)->verts;
   float width = ctx->Line.Width;
   GLfloat z0, z1;
   GLuint c0, c1;
   tdfxVertex *vert0 = &fxVB[v0];
   tdfxVertex *vert1 = &fxVB[v1];

   if ( IND & TDFX_TWOSIDE_BIT ) {
      GLubyte (*vbcolor)[4] = ctx->VB->ColorPtr->data;

      if ( IND & TDFX_FLAT_BIT ) {
	 TDFX_COLOR( (char *)&vert0->v.color,vbcolor[pv] );
	 *(int *)&vert1->v.color = *(int *)&vert0->v.color;
      } else {
	 TDFX_COLOR( (char *)&vert0->v.color,vbcolor[v0] );
	 TDFX_COLOR( (char *)&vert1->v.color,vbcolor[v1] );
      }
   } else if ( IND & TDFX_FLAT_BIT ) {
      c0 = *(GLuint *) &(vert0->v.color);
      c1 = *(GLuint *) &(vert1->v.color);
      *(int *)&vert0->v.color =
	 *(int *)&vert1->v.color = *(int *)&fxVB[pv].v.color;
   }

   if ( IND & TDFX_OFFSET_BIT ) {
      GLfloat offset = ctx->LineZoffset;
      z0 = vert0->v.z;
      z1 = vert1->v.z;
      vert0->v.z += offset;
      vert1->v.z += offset;
   }

   if (IND & TDFX_CLIPRECT_BIT) {
      BEGIN_CLIP_LOOP_LOCKED( fxMesa );
      tdfx_draw_line( fxMesa, &fxVB[v0], &fxVB[v1], width );
      END_CLIP_LOOP_LOCKED( fxMesa );
   } else
      tdfx_draw_line( fxMesa, &fxVB[v0], &fxVB[v1], width );

   if ( IND & TDFX_OFFSET_BIT ) {
      vert0->v.z = z0;
      vert1->v.z = z1;
   }

   if ( (IND & TDFX_FLAT_BIT) && !(IND & TDFX_TWOSIDE_BIT) ) {
      *(GLuint *) &(vert0->v.color) = c0;
      *(GLuint *) &(vert1->v.color) = c1;
   }
}


static __inline void TAG(points)( GLcontext *ctx, GLuint first, GLuint last )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   struct vertex_buffer *VB = ctx->VB;
   tdfxVertexPtr fxVB = TDFX_DRIVER_DATA(VB)->verts;
   GLfloat sz = ctx->Point.Size;
   int i;

   for ( i = first ; i < last ; i++ ) {
      if ( VB->ClipMask[i] == 0 ) {
	 if ( IND & (TDFX_TWOSIDE_BIT|TDFX_OFFSET_BIT) )
	 {
	    tdfxVertex tmp0 = fxVB[i];

	    if ( IND & TDFX_TWOSIDE_BIT ) {
	       GLubyte (*vbcolor)[4] = VB->ColorPtr->data;
	       TDFX_COLOR( (char *)&tmp0.v.color, vbcolor[i] );
	    }

	    if ( IND & TDFX_OFFSET_BIT ) {
	       GLfloat offset = ctx->PointZoffset;
	       tmp0.v.z += offset;
	    }

	    if (IND & TDFX_CLIPRECT_BIT) {
	       BEGIN_CLIP_LOOP_LOCKED( fxMesa );
	       tdfx_draw_point( fxMesa, &tmp0, sz );
	       END_CLIP_LOOP_LOCKED( fxMesa );
	    } else
	       tdfx_draw_point( fxMesa, &tmp0, sz );
	 }
	 else if (IND & TDFX_CLIPRECT_BIT)
	 {
	    BEGIN_CLIP_LOOP_LOCKED( fxMesa );
	    tdfx_draw_point( fxMesa, &fxVB[i], sz );
	    END_CLIP_LOOP_LOCKED( fxMesa );
	 }
	 else
	    tdfx_draw_point( fxMesa, &fxVB[i], sz );
      }
   }
}




/* Accelerate unclipped VB rendering when fxMesa->renderIndex != 0
 *
 * The versions for renderIndex == 0 are further optimized and appear
 * in tdfx_tris.c
 */
#if (TYPE == 0)
#define RENDER_POINTS( start, count ) TAG(points)( ctx, start, count )
#define RENDER_LINE( i1, i ) TAG(line)( ctx, i1, i, i )
#define RENDER_TRI( i2, i1, i, pv, parity )			\
  do {								\
    if (parity) TAG(triangle)( ctx, i1, i2, i, pv );	\
    else        TAG(triangle)( ctx, i2, i1, i, pv );	\
  } while (0)
#define RENDER_QUAD( i3, i2, i1, i, pv ) TAG(quad)( ctx, i3, i2, i1, i, pv )
#define LOCAL_VARS GLcontext *ctx = VB->ctx;
#define PRESERVE_TAG
#include "render_tmp.h"
#endif



static void TAG(init)( void )
{
   rast_tab[IND].triangle	= TAG(triangle);
   rast_tab[IND].quad		= TAG(quad);
   rast_tab[IND].line		= TAG(line);
   rast_tab[IND].points		= TAG(points);
#if (TYPE == 0)
   rast_tab[IND].render_tab     = TAG(render_tab);
#else
   rast_tab[IND].render_tab     = 0;
#endif
   TAG(render_init)();
}


#undef IND
#undef TAG
