/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810tritmp.h,v 1.5 2000/12/05 21:18:33 dawes Exp $ */

static __inline void TAG(triangle)(GLcontext *ctx,
				   GLuint e0, GLuint e1, GLuint e2,
				   GLuint pv)
{
   i810ContextPtr        i810ctx   = I810_CONTEXT(ctx);
   struct vertex_buffer *VB        = ctx->VB;
   i810VertexPtr         i810verts = I810_DRIVER_DATA(VB)->verts;
   i810Vertex     *v[3];

#if (IND & I810_OFFSET_BIT)
   GLfloat offset;
   GLfloat z[3];
#endif

#if (IND & (I810_TWOSIDE_BIT | I810_FLAT_BIT))
   GLuint c[3];
#endif

   v[0] = &i810verts[e0];
   v[1] = &i810verts[e1];
   v[2] = &i810verts[e2];

#if (IND & (I810_TWOSIDE_BIT | I810_FLAT_BIT))
   c[0] = v[0]->ui[4];
   c[1] = v[1]->ui[4];
   c[2] = v[2]->ui[4];
#endif


#if (IND & (I810_TWOSIDE_BIT | I810_OFFSET_BIT))
   {
      GLfloat ex = v[0]->v.x - v[2]->v.x;
      GLfloat ey = v[0]->v.y - v[2]->v.y;
      GLfloat fx = v[1]->v.x - v[2]->v.x;
      GLfloat fy = v[1]->v.y - v[2]->v.y;
      GLfloat cc  = ex*fy - ey*fx;

#if (IND & I810_TWOSIDE_BIT)
      {
	 GLuint  facing        = (cc > 0.0) ^ ctx->Polygon.FrontBit;
	 GLubyte (*vbcolor)[4] = VB->Color[facing]->data;
	 if (IND & I810_FLAT_BIT) {
	    I810_COLOR((char *)&v[0]->ui[4], vbcolor[pv]);
	    v[2]->ui[4] = v[1]->ui[4] = v[0]->ui[4];
	 } else {
	    I810_COLOR((char *)&v[0]->ui[4], vbcolor[e0]);
	    I810_COLOR((char *)&v[1]->ui[4], vbcolor[e1]);
	    I810_COLOR((char *)&v[2]->ui[4], vbcolor[e2]);
	 }
      }
#endif

#if (IND & I810_OFFSET_BIT)
      {
	 offset = ctx->Polygon.OffsetUnits * 1.0/0x10000;
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
#elif (IND & I810_FLAT_BIT)
   {
      GLuint color = i810verts[pv].ui[4];
      v[0]->ui[4] = color;
      v[1]->ui[4] = color;
      v[2]->ui[4] = color;
   }
#endif

   i810_draw_triangle( i810ctx, v[0], v[1], v[2] );

#if (IND & I810_OFFSET_BIT)
   v[0]->v.z = z[0];
   v[1]->v.z = z[1];
   v[2]->v.z = z[2];
#endif

#if (IND & (I810_FLAT_BIT | I810_TWOSIDE_BIT))
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
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   i810VertexPtr i810VB = I810_DRIVER_DATA(ctx->VB)->verts;

#if (IND & (I810_TWOSIDE_BIT|I810_FLAT_BIT|I810_OFFSET_BIT)) 
   i810Vertex tmp0 = i810VB[v0];
   i810Vertex tmp1 = i810VB[v1];

   if (IND & I810_TWOSIDE_BIT) {
      GLubyte (*vbcolor)[4] = ctx->VB->ColorPtr->data;

      if (IND & I810_FLAT_BIT) {
	 I810_COLOR((char *)&tmp0.v.color,vbcolor[pv]);
	 *(int *)&tmp1.v.color = *(int *)&tmp0.v.color;
      } else {
	 I810_COLOR((char *)&tmp0.v.color,vbcolor[v0]);
	 I810_COLOR((char *)&tmp1.v.color,vbcolor[v1]);
      }

   } else if (IND & I810_FLAT_BIT) {
      *(int *)&tmp0.v.color = *(int *)&i810VB[pv].v.color;
      *(int *)&tmp1.v.color = *(int *)&i810VB[pv].v.color;
   }

   /* Relies on precomputed LineZoffset from vbrender.c
    */
   if (IND & I810_OFFSET_BIT) {
      GLfloat offset = ctx->LineZoffset * (1.0 / 0x10000); 
      tmp0.v.z += offset;
      tmp1.v.z += offset;
   }

   i810_draw_line( imesa, &tmp0, &tmp1 );
#else
   i810_draw_line( imesa, &i810VB[v0], &i810VB[v1] );
#endif
}


static void TAG(points)( GLcontext *ctx, GLuint first, GLuint last )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   struct vertex_buffer *VB = ctx->VB;
   i810VertexPtr i810VB = I810_DRIVER_DATA(VB)->verts;
   GLfloat sz = ctx->Point.Size * .5;
   int i;

   /* Culling is disabled automatically via. the
    * ctx->Driver.ReducedPrimitiveChange() callback.  
    */
   
   for(i=first;i<last;i++) {
      if(VB->ClipMask[i]==0) {
	 if (IND & I810_TWOSIDE_BIT) {	
	    i810Vertex tmp0 = i810VB[i];
	    if (IND & I810_TWOSIDE_BIT) {
	       GLubyte (*vbcolor)[4] = VB->ColorPtr->data;
	       I810_COLOR((char *)&tmp0.v.color, vbcolor[i]);
	    }
	    if (IND & I810_OFFSET_BIT) {
	       GLfloat offset = ctx->PointZoffset * (1.0 / 0x10000);
	       tmp0.v.z += offset;
	    }
	    i810_draw_point( imesa, &tmp0, sz );
	 } else
	    i810_draw_point( imesa, &i810VB[i], sz );
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
