/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgatritmp.h,v 1.5 2000/12/05 21:18:34 dawes Exp $ */

static __inline void TAG(triangle)(GLcontext *ctx,
				   GLuint e0, GLuint e1, GLuint e2,
				   GLuint pv)
{
   mgaContextPtr        mmesa   = MGA_CONTEXT(ctx);
   struct vertex_buffer *VB        = ctx->VB;
   mgaVertexPtr         mgaverts = MGA_DRIVER_DATA(VB)->verts;
   mgaVertex     *v[3];

#if (IND & MGA_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[3];
#endif

#if (IND & (MGA_TWOSIDE_BIT | MGA_FLAT_BIT))
   GLuint c[3];
#endif

   v[0] = &mgaverts[e0];
   v[1] = &mgaverts[e1];
   v[2] = &mgaverts[e2];

#if (IND & (MGA_TWOSIDE_BIT | MGA_FLAT_BIT))
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
#endif


#if (IND & (MGA_TWOSIDE_BIT | MGA_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc  = ex*fy - ey*fx;

#if (IND & MGA_TWOSIDE_BIT)
      {
	 GLuint  facing        = (cc > 0.0) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 if (IND & MGA_FLAT_BIT) {
	    MGA_COLOR((char *)&v[0]->ui[4], vbcolor[pv]);
	    v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
	 } else {
	    MGA_COLOR((char *)&v[0]->ui[4], vbcolor[e0]);
	    MGA_COLOR((char *)&v[1]->ui[4], vbcolor[e1]);
	    MGA_COLOR((char *)&v[2]->ui[4], vbcolor[e2]);
	 }
      }
#endif

#if (IND & MGA_OFFSET_BIT)
      {
	 offset = ctx->Polygon.OffsetUnits * mmesa->depth_scale;
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
#elif (IND & MGA_FLAT_BIT)
   {
      GLuint color = mgaverts[pv].ui[4];
      v[0]->ui[4] = color;
      v[1]->ui[4] = color;
      v[2]->ui[4] = color;
   }
#endif

   mga_draw_triangle( mmesa, v[0], v[1], v[2] );

#if (IND & MGA_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
#endif

#if (IND & (MGA_FLAT_BIT | MGA_TWOSIDE_BIT))
   v[0]->ui[4] = c[0];
   v[1]->ui[4] = c[1];
   v[2]->ui[4] = c[2];    
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
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   mgaVertexPtr mgaVB = MGA_DRIVER_DATA(ctx->VB)->verts;
   float width = ctx->Line.Width;
   GLfloat z0, z1;
   GLuint c0, c1;
   mgaVertex *vert0 = &mgaVB[v0];
   mgaVertex *vert1 = &mgaVB[v1];


   if (IND & MGA_TWOSIDE_BIT) {
      GLubyte (*vbcolor)[4] = ctx->VB->ColorPtr->data;

      if (IND & MGA_FLAT_BIT) {
	 MGA_COLOR((char *)&vert0->v.color,vbcolor[pv]);
	 *(int *)&vert1->v.color = *(int *)&vert0->v.color;
      } else {
	 MGA_COLOR((char *)&vert0->v.color,vbcolor[v0]);
	 MGA_COLOR((char *)&vert1->v.color,vbcolor[v1]);
      }
   } else if (IND & MGA_FLAT_BIT) {
      c0 = *(GLuint *) &(vert0->v.color);
      c1 = *(GLuint *) &(vert1->v.color);
      *(int *)&vert0->v.color = 
	 *(int *)&vert1->v.color = *(int *)&mgaVB[pv].v.color;
   }

   if (IND & MGA_OFFSET_BIT) {
      GLfloat offset = ctx->LineZoffset * mmesa->depth_scale; 
      z0 = vert0->v.z;
      z1 = vert1->v.z;
      vert0->v.z += offset;
      vert1->v.z += offset;
   }

   mga_draw_line( mmesa, &mgaVB[v0], &mgaVB[v1], width );

   if (IND & MGA_OFFSET_BIT) {
      vert0->v.z = z0;
      vert1->v.z = z1;
   }
   
   if ((IND & MGA_FLAT_BIT) && !(IND & MGA_TWOSIDE_BIT)) {
      *(GLuint *) &(vert0->v.color) = c0;
      *(GLuint *) &(vert1->v.color) = c1;
   }
}


static void TAG(points)( GLcontext *ctx, GLuint first, GLuint last )
{
   mgaContextPtr mmesa = MGA_CONTEXT( ctx );
   struct vertex_buffer *VB = ctx->VB;
   mgaVertexPtr mgaVB = MGA_DRIVER_DATA(VB)->verts;
   GLfloat sz = ctx->Point.Size * .5;
   int i;
   
   for(i=first;i<last;i++) 
      if(VB->ClipMask[i]==0) {
	 if (IND & (MGA_TWOSIDE_BIT|MGA_OFFSET_BIT)) {	
	    mgaVertex tmp0 = mgaVB[i];
	    if (IND & MGA_TWOSIDE_BIT) {
	       GLubyte (*vbcolor)[4] = VB->ColorPtr->data;
	       MGA_COLOR((char *)&tmp0.v.color, vbcolor[i]);
	    }
	    if (IND & MGA_OFFSET_BIT) {
	       GLfloat offset = ctx->PointZoffset * mmesa->depth_scale;
	       tmp0.v.z += offset;
	    }
	    mga_draw_point( mmesa, &tmp0, sz );
	 } else
	    mga_draw_point( mmesa, &mgaVB[i], sz );
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
