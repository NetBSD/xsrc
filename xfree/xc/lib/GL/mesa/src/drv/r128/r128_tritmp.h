/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_tritmp.h,v 1.5 2001/01/08 01:07:24 martin Exp $ */
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

static __inline void TAG(triangle)( GLcontext *ctx,
				    GLuint e0, GLuint e1, GLuint e2,
				    GLuint pv )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   struct vertex_buffer *VB = ctx->VB;
   r128VertexPtr verts = R128_DRIVER_DATA(VB)->verts;
   r128VertexPtr v[3];

#if (IND & R128_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[3];
#endif
#if (IND & R128_TWOSIDE_BIT)
   GLuint c[3];
#endif

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];

#if (IND & R128_TWOSIDE_BIT)
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
#endif

#if (IND & (R128_TWOSIDE_BIT | R128_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc = ex*fy - ey*fx;

#if (IND & R128_TWOSIDE_BIT)
      {
	 GLuint facing = ( cc > 0.0 ) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 if ( IND & R128_FLAT_BIT ) {
	    R128_COLOR( (char *)&v[0]->ui[4], vbcolor[pv] );
	    v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
	 } else {
	    R128_COLOR( (char *)&v[0]->ui[4], vbcolor[e0] );
	    R128_COLOR( (char *)&v[1]->ui[4], vbcolor[e1] );
	    R128_COLOR( (char *)&v[2]->ui[4], vbcolor[e2] );
	 }
      }
#endif

#if (IND & R128_OFFSET_BIT)
      {
	 offset = ctx->Polygon.OffsetUnits * rmesa->depth_scale;
	 z[0] = v[0]->v.z;
	 z[1] = v[1]->v.z;
	 z[2] = v[2]->v.z;
	 if ( cc * cc > 1e-16 ) {
	    GLfloat ez = z[0] - z[2];
	    GLfloat fz = z[1] - z[2];
	    GLfloat a  = ey*fz - ez*fy;
	    GLfloat b  = ez*fx - ex*fz;
	    GLfloat ic = 1.0 / cc;
	    GLfloat ac = a * ic;
	    GLfloat bc = b * ic;
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
#endif

   r128_draw_triangle( rmesa, v[0], v[1], v[2] );

#if (IND & R128_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
#endif

#if (IND & R128_TWOSIDE_BIT)
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
   v[2]->ui[4] = c[2];
#endif
}


static void TAG(quad)( GLcontext *ctx,
		       GLuint e0, GLuint e1, GLuint e2, GLuint e3,
		       GLuint pv )
{
#if 0
   TAG(triangle)( ctx, e0, e1, e3, pv );
   TAG(triangle)( ctx, e1, e2, e3, pv );
#else
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   struct vertex_buffer *VB = ctx->VB;
   r128VertexPtr verts = R128_DRIVER_DATA(VB)->verts;
   r128VertexPtr v[4];

#if (IND & R128_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[4];
#endif
#if (IND & R128_TWOSIDE_BIT)
   int c[4];
#endif

   v[0] = &verts[e0];
   v[1] = &verts[e1];
   v[2] = &verts[e2];
   v[3] = &verts[e3];

#if (IND & R128_TWOSIDE_BIT)
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
   c[3] = v[3]->ui[4];
#endif

#if (IND & (R128_TWOSIDE_BIT | R128_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc = ex*fy - ey*fx;

#if (IND & R128_TWOSIDE_BIT)
      {
	 GLuint facing = ( cc > 0.0 ) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 if ( IND & R128_FLAT_BIT ) {
	    R128_COLOR( (char *)&v[0]->ui[4], vbcolor[pv] );
	    v[3]->ui[4] = v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
	 } else {
	    R128_COLOR( (char *)&v[0]->ui[4], vbcolor[e0] );
	    R128_COLOR( (char *)&v[1]->ui[4], vbcolor[e1] );
	    R128_COLOR( (char *)&v[2]->ui[4], vbcolor[e2] );
	    R128_COLOR( (char *)&v[3]->ui[4], vbcolor[e3] );
	 }
      }
#endif

#if (IND & R128_OFFSET_BIT)
      {
	 offset = ctx->Polygon.OffsetUnits * rmesa->depth_scale;
	 z[0] = v[0]->v.z;
	 z[1] = v[1]->v.z;
	 z[2] = v[2]->v.z;
	 z[3] = v[3]->v.z;
	 if ( cc * cc > 1e-16 ) {
	    GLfloat ez = z[0] - z[2];
	    GLfloat fz = z[1] - z[2];
	    GLfloat a  = ey*fz - ez*fy;
	    GLfloat b  = ez*fx - ex*fz;
	    GLfloat ic = 1.0 / cc;
	    GLfloat ac = a * ic;
	    GLfloat bc = b * ic;
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
#endif

   r128_draw_quad( rmesa, v[0], v[1], v[2], v[3] );

#if (IND & R128_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
   v[3]->v.z = z[3];
#endif

#if (IND & R128_TWOSIDE_BIT)
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
   v[2]->ui[4] = c[2];
   v[3]->ui[4] = c[3];
#endif
#endif
}


static void TAG(line)( GLcontext *ctx,
		       GLuint e0, GLuint e1,
		       GLuint pv )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   struct vertex_buffer *VB = ctx->VB;
   r128VertexPtr verts = R128_DRIVER_DATA(VB)->verts;
   GLfloat width = ctx->Line.Width;
   r128VertexPtr v[2];

#if (IND & R128_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[2];
#endif
#if (IND & R128_TWOSIDE_BIT)
   int c[2];
#endif

   v[0] = &verts[e0];
   v[1] = &verts[e1];

#if (IND & R128_TWOSIDE_BIT)
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
#endif

#if (IND & R128_TWOSIDE_BIT)
   {
      GLubyte (*vbcolor)[4] = ctx->VB->ColorPtr->data;
      if ( IND & R128_FLAT_BIT ) {
	 R128_COLOR( (char *)&v[0]->ui[4], vbcolor[pv] );
	 v[1]->ui[4] = v[0]->ui[4];
      } else {
	 R128_COLOR( (char *)&v[0]->ui[4], vbcolor[e0] );
	 R128_COLOR( (char *)&v[1]->ui[4], vbcolor[e1] );
      }
   }
#endif

#if (IND & R128_OFFSET_BIT)
   offset = ctx->LineZoffset * rmesa->depth_scale;
   z[0] = v[0]->v.z;
   z[1] = v[1]->v.z;
   v[0]->v.z += offset;
   v[1]->v.z += offset;
#endif

   r128_draw_line( rmesa, v[0], v[1], width );

#if (IND & R128_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
#endif

#if (IND & R128_TWOSIDE_BIT)
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
#endif
}


static void TAG(points)( GLcontext *ctx,
			 GLuint first, GLuint last )
{
   r128ContextPtr rmesa = R128_CONTEXT(ctx);
   struct vertex_buffer *VB = ctx->VB;
   r128VertexPtr verts = R128_DRIVER_DATA(VB)->verts;
   GLfloat size = ctx->Point.Size * 0.5;
   int i;

   for ( i = first ; i < last ; i++) {
      if ( VB->ClipMask[i] == 0 ) {
	 if ( IND & (R128_TWOSIDE_BIT|R128_OFFSET_BIT) ) {
	    r128Vertex tmp0 = verts[i];

	    if ( IND & R128_TWOSIDE_BIT ) {
	       GLubyte (*vbcolor)[4] = VB->ColorPtr->data;
	       R128_COLOR( (char *)&tmp0.v.color, vbcolor[i] );
	    }
	    if ( IND & R128_OFFSET_BIT ) {
	       GLfloat offset = ctx->PointZoffset * rmesa->depth_scale;
	       tmp0.v.z += offset;
	    }
	    r128_draw_point( rmesa, &tmp0, size );
	 } else {
	    r128_draw_point( rmesa, &verts[i], size );
	 }
      }
   }
}


static void TAG(init)( void )
{
   rast_tab[IND].points		= TAG(points);
   rast_tab[IND].line		= TAG(line);
   rast_tab[IND].triangle	= TAG(triangle);
   rast_tab[IND].quad		= TAG(quad);
}

#undef IND
#undef TAG
