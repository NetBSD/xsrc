/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_tritmp.h,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

static __inline void TAG(triangle)(GLcontext *ctx,
				   GLuint e0, GLuint e1, GLuint e2,
				   GLuint pv)
{
   i830ContextPtr        imesa   = I830_CONTEXT(ctx);
   struct vertex_buffer *VB        = ctx->VB;
   i830VertexPtr         i830verts = I830_DRIVER_DATA(VB)->verts;
   i830Vertex     *v[3];

#if (IND & I830_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[3];
#endif

#if (IND & (I830_TWOSIDE_BIT | I830_FLAT_BIT))
   GLuint c[3];
   GLuint s[3];
#endif

   v[0] = &i830verts[e0];
   v[1] = &i830verts[e1];
   v[2] = &i830verts[e2];

#if (IND & (I830_TWOSIDE_BIT | I830_FLAT_BIT))
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
   s[0] = v[0]->ui[5];
   s[1] = v[1]->ui[5];
   s[2] = v[2]->ui[5];
#endif


#if (IND & (I830_TWOSIDE_BIT | I830_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc  = ex*fy - ey*fx;

#if (IND & I830_TWOSIDE_BIT)
      {
	 GLuint  facing        = (cc > 0.0) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 GLubyte (*vbspec)[4] = VB->Spec[facing];
	 if (IND & I830_FLAT_BIT) {
	    I830_COLOR((char *)&v[0]->ui[4], vbcolor[pv]);
	    v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
            I830_COLOR3((char *)&v[0]->ui[5], vbspec[pv]);
	    v[2]->ui[5] = v[1]->ui[5] = v[0]->ui[5];
	 } else {
	    I830_COLOR((char *)&v[0]->ui[4], vbcolor[e0]);
	    I830_COLOR((char *)&v[1]->ui[4], vbcolor[e1]);
	    I830_COLOR((char *)&v[2]->ui[4], vbcolor[e2]);
	    I830_COLOR3((char *)&v[0]->ui[5], vbspec[e0]);
	    I830_COLOR3((char *)&v[1]->ui[5], vbspec[e1]);
	    I830_COLOR3((char *)&v[2]->ui[5], vbspec[e2]);
	 }
      }
#endif

#if (IND & I830_OFFSET_BIT)
      {
	 /*offset = ctx->Polygon.OffsetUnits * 1.0/0x10000;*/
	 offset = ctx->Polygon.OffsetUnits * imesa->depth_scale;
	 z[0] = v[0]->v.z;
	 z[1] = v[1]->v.z;
	 z[2] = v[2]->v.z;
	 if (cc * cc > 1e-16) {
	    GLfloat ez     = z[0] - z[2];
	    GLfloat fz     = z[1] - z[2];
	    GLfloat a      = ey*fz - ez*fy;
	    GLfloat b      = ez*fx - ex*fz;
	    GLfloat ic     = 1.0 / cc;
	    GLfloat ac     = a * ic;
	    GLfloat bc     = b * ic;
	    if (ac < 0.0f) ac = -ac;
	    if (bc < 0.0f) bc = -bc;
	    offset += MAX2(ac, bc) * ctx->Polygon.OffsetFactor;
	 }
	 v[0]->v.z += offset;
	 v[1]->v.z += offset;
	 v[2]->v.z += offset;
      }
#endif
   }
#elif (IND & I830_FLAT_BIT)
   {
      GLuint color = i830verts[pv].ui[4];
      GLuint spec = i830verts[pv].ui[5];
      v[0]->ui[4] = color;
      v[1]->ui[4] = color;
      v[2]->ui[4] = color;
      v[0]->ui[5] = spec;
      v[1]->ui[5] = spec;
      v[2]->ui[5] = spec;
   }
#endif

   i830_draw_triangle( imesa, v[0], v[1], v[2] );

#if (IND & I830_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
#endif

#if (IND & (I830_FLAT_BIT | I830_TWOSIDE_BIT))
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
   v[2]->ui[4] = c[2];    
   v[0]->ui[5] = s[0];
   v[1]->ui[5] = s[1];
   v[2]->ui[5] = s[2];    
#endif

}




static void TAG(quad)( GLcontext *ctx, GLuint v0,
		       GLuint v1, GLuint v2, GLuint v3, 
		       GLuint pv )
{
   TAG(triangle)( ctx, v0, v1, v3, pv );
   TAG(triangle)( ctx, v1, v2, v3, pv );
}


static void TAG(line)( GLcontext *ctx, GLuint v0, GLuint v1, GLuint pv )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   i830VertexPtr i830VB = I830_DRIVER_DATA(ctx->VB)->verts;
   GLfloat z0, z1;
   GLuint c0, c1;
   GLuint s0, s1;
   i830Vertex *vert0 = &i830VB[v0];
   i830Vertex *vert1 = &i830VB[v1];

   if (IND & I830_TWOSIDE_BIT) {
      GLubyte (*vbcolor)[4] = ctx->VB->ColorPtr->data;
      GLubyte (*vbspec)[4] = ctx->VB->Specular;

      if (IND & I830_FLAT_BIT) {
	 I830_COLOR((char *)&vert0->v.color,vbcolor[pv]);
	 *(int *)&vert1->v.color = *(int *)&vert0->v.color;
         I830_COLOR3((char *)&vert0->v.specular, vbspec[pv]);
         *(int *)&vert1->v.specular = *(int *)&vert0->v.specular;
      } else {
	 I830_COLOR((char *)&vert0->v.color,vbcolor[v0]);
	 I830_COLOR((char *)&vert1->v.color,vbcolor[v1]);
         I830_COLOR3((char *)&vert0->v.specular, vbspec[v0]);
         I830_COLOR3((char *)&vert1->v.specular, vbspec[v1]);
      }
   } else if (IND & I830_FLAT_BIT) {
      c0 = *(GLuint *) &(vert0->v.color);
      c1 = *(GLuint *) &(vert1->v.color);
      *(int *)&vert0->v.color = 
	 *(int *)&vert1->v.color = *(int *)&i830VB[pv].v.color;
      s0 = *(GLuint *) &(vert0->v.specular);
      s1 = *(GLuint *) &(vert1->v.specular);
      *(int *)&vert0->v.specular = 
         *(int *)&vert1->v.specular = *(int *)&i830VB[pv].v.specular;
   }

   if (IND & I830_OFFSET_BIT) {
      /*GLfloat offset = ctx->LineZoffset * (1.0 / 0x10000);*/
      GLfloat offset = ctx->LineZoffset * imesa->depth_scale;
      z0 = vert0->v.z;
      z1 = vert1->v.z;
      vert0->v.z += offset;
      vert1->v.z += offset;
   }

   i830_draw_line( imesa, &i830VB[v0], &i830VB[v1]);

   if (IND & I830_OFFSET_BIT) {
      vert0->v.z = z0;
      vert1->v.z = z1;
   }

   if ((IND & I830_FLAT_BIT) && !(IND & I830_TWOSIDE_BIT)) {
      *(GLuint *) &(vert0->v.color) = c0;
      *(GLuint *) &(vert1->v.color) = c1;
      *(GLuint *) &(vert0->v.specular) = s0;
      *(GLuint *) &(vert1->v.specular) = s1;
   }
}

static void TAG(points)( GLcontext *ctx, GLuint first, GLuint last )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   struct vertex_buffer *VB = ctx->VB;
   i830VertexPtr i830VB = I830_DRIVER_DATA(VB)->verts;
   GLfloat sz = ctx->Point.Size * .5;
   int i;

   for(i=first;i<last;i++) 
      if(VB->ClipMask[i]==0) {
	 if (IND & (I830_TWOSIDE_BIT|I830_OFFSET_BIT)) {	
	    i830Vertex tmp0 = i830VB[i];
	    if (IND & I830_TWOSIDE_BIT) {
	       GLubyte (*vbcolor)[4] = VB->ColorPtr->data;
	       I830_COLOR((char *)&tmp0.v.color, vbcolor[i]);
	    }
	    if (IND & I830_OFFSET_BIT) {
	       /*GLfloat offset = ctx->PointZoffset * (1.0 / 0x10000);*/
	       GLfloat offset = ctx->PointZoffset * imesa->depth_scale;
	       tmp0.v.z += offset;
	    }
	    tmp0.f[0] -= 0.125;
	    tmp0.f[1] -= 0.125;
	    i830_draw_point( imesa, &tmp0, sz );
	 } else {
	    i830VB[i].f[0] -= 0.125;
	    i830VB[i].f[1] -= 0.125;
	    i830_draw_point( imesa, &i830VB[i], sz );
	    i830VB[i].f[0] += 0.125;
	    i830VB[i].f[1] += 0.125;
	 }
      }
}




static void TAG(init)( void )
{
   tri_tab[IND] = TAG(triangle);
   quad_tab[IND] = TAG(quad);
   line_tab[IND] = TAG(line);
   points_tab[IND] = TAG(points);
}


#undef IND
#undef TAG
