/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_elttmp.h,v 1.1 2001/01/08 01:07:27 martin Exp $ */
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
 *   Keith Whitwell <keithw@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

/* Buffers fill from high addresses down with vertices and from low
 * addresses up with elements.
 */


/* Emit the bulk of the vertices to the first dma buffer.  Leave
 * empty slots for clipped vertices so that we can still address
 * vertices by index.
 */
static void TAG(emit_unclipped_verts)( struct vertex_buffer *VB )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(VB->ctx);
   GLfloat *dev = VB->Projected->start;
   GLubyte *color = VB->ColorPtr->start;
   GLfloat *tex0_data = VB->TexCoordPtr[0]->start;
   GLfloat *tex1_data = VB->TexCoordPtr[1]->start;
   GLuint color_stride = VB->ColorPtr->stride;
   GLuint tex0_stride = VB->TexCoordPtr[0]->stride;
   GLuint tex1_stride = VB->TexCoordPtr[1]->stride;
   GLuint buffer_stride = rmesa->vertsize;

   GLfloat *f = rmesa->next_vert;
   GLuint count = VB->Count;
   GLubyte *clipmask = VB->ClipMask;

   const GLfloat *m = rmesa->device_matrix;
   const GLfloat sx = m[0], sy = m[5], sz = m[10];
   const GLfloat tx = m[12], ty = m[13], tz = m[14];
   GLuint i;

   rmesa->retained_buf = rmesa->elt_buf;
   rmesa->first_vert_index = rmesa->next_vert_index;

   for ( i = 0 ; i < count ; f -= buffer_stride, i++ )
   {
      if ( !clipmask[i] )
      {
	 f[0] = sx * dev[0] + tx;
	 f[1] = sy * dev[1] + ty;
	 f[2] = sz * dev[2] + tz;
	 f[3] = dev[3];

	 if ( TYPE & RADEON_RGBA_BIT ) {
	    *(GLuint *)&f[4] = *(GLuint *)color;
	 }

	 if ( TYPE & RADEON_TEX0_BIT ) {
	    *(GLuint *)&f[6] = *(GLuint *)&tex0_data[0];
	    *(GLuint *)&f[7] = *(GLuint *)&tex0_data[1];
	 }

	 if ( TYPE & RADEON_TEX1_BIT ) {
	    *(GLuint *)&f[8] = *(GLuint *)&tex1_data[0];
	    *(GLuint *)&f[9] = *(GLuint *)&tex1_data[1];
	 }
      }

      STRIDE_F( dev, 16 );
      if ( TYPE & RADEON_RGBA_BIT ) color += color_stride;
      if ( TYPE & RADEON_TEX0_BIT ) STRIDE_F( tex0_data, tex0_stride );
      if ( TYPE & RADEON_TEX1_BIT ) STRIDE_F( tex1_data, tex1_stride );
   }

   rmesa->next_vert = f;
   rmesa->next_vert_index -= count;
}


/* Build three temporary clipspace vertex for clipping a triangle.
 * Recreate from the VB data rather than trying to read back from
 * uncached memory.
 */
static void TAG(build_tri_verts)( radeonContextPtr rmesa,
				  struct vertex_buffer *VB,
				  GLfloat *O,
				  GLuint *elt )
{
   GLint i;

   for ( i = 0 ; i < 3 ; i++, O += CLIP_STRIDE ) {
      GLfloat *clip = VB->Clip.start + elt[i]*4;

      O[0] = clip[0];
      O[1] = clip[1];
      O[2] = clip[2];
      O[3] = clip[3];

      if ( TYPE & RADEON_RGBA_BIT ) {
	 GLubyte *color = VEC_ELT(VB->ColorPtr, GLubyte, elt[i]);
	 *(GLuint *)&O[4] = *(GLuint *)color;
      }

      *(GLuint *)&O[5] = UNCLIPPED_VERT(elt[i]);

      if ( TYPE & RADEON_TEX0_BIT ) {
	 GLfloat *tex0_data = VEC_ELT(VB->TexCoordPtr[0], GLfloat, elt[i]);
	 *(GLuint *)&O[6] = *(GLuint *)&tex0_data[0];
	 *(GLuint *)&O[7] = *(GLuint *)&tex0_data[1];
      }

      if ( TYPE & RADEON_TEX1_BIT ) {
	 GLfloat *tex1_data = VEC_ELT(VB->TexCoordPtr[1], GLfloat, elt[i]);
	 *(GLuint *)&O[8] = *(GLuint *)&tex1_data[0];
	 *(GLuint *)&O[9] = *(GLuint *)&tex1_data[1];
      }
   }
}


/* Interpolate between two of the vertices constructed above.
 */
static void TAG(interp)( GLfloat t,
			 GLfloat *O,
			 const GLfloat *I,
			 const GLfloat *J )
{
   O[0] = LINTERP( t, I[0], J[0] );
   O[1] = LINTERP( t, I[1], J[1] );
   O[2] = LINTERP( t, I[2], J[2] );
   O[3] = LINTERP( t, I[3], J[3] );

   if ( TYPE & RADEON_RGBA_BIT ) {
      INTERP_RGBA( t,
		   ((GLubyte *)&(O[4])),
		   ((GLubyte *)&(I[4])),
		   ((GLubyte *)&(J[4])) );
   }

   *(GLuint *)&O[5] = ~0;	/* note that this is a new vertex */

   if ( TYPE & RADEON_TEX0_BIT ) {
      O[6] = LINTERP( t, I[6], J[6] );
      O[7] = LINTERP( t, I[7], J[7] );
   }

   if ( TYPE & RADEON_TEX1_BIT ) {
      O[8] = LINTERP( t, I[8], J[8] );
      O[9] = LINTERP( t, I[9], J[9] );
   }
}



/* When clipping is complete, scan the final vertex list and emit any
 * new ones to dma buffers.  Update the element list to a format
 * suitable for sending to hardware.
 */
static void TAG(project_and_emit_verts)( radeonContextPtr rmesa,
					 const GLfloat *verts,
					 GLuint *elt,
					 GLuint nr)
{
   GLfloat *O = rmesa->next_vert;
   GLushort index = rmesa->next_vert_index;
   GLuint buffer_stride = rmesa->vertsize;

   const GLfloat *m = rmesa->device_matrix;
   const GLfloat sx = m[0], sy = m[5], sz = m[10];
   const GLfloat tx = m[12], ty = m[13], tz = m[14];
   GLuint i;

   for ( i = 0 ; i < nr ; i++ ) {
      const GLfloat *I = &verts[elt[i] * CLIP_STRIDE];
      GLuint tmp = *(GLuint *)&I[5];

      if ( (elt[i] = tmp) == ~0 ) {
	 GLfloat oow = 1.0 / I[3];

	 elt[i] = index--;

	 O[0] = sx * I[0] * oow + tx;
	 O[1] = sy * I[1] * oow + ty;
	 O[2] = sz * I[2] * oow + tz;
	 O[3] = oow;

	 if ( TYPE & RADEON_RGBA_BIT ) {
	    *(GLuint *)&O[4] = *(GLuint *)&I[4];
	 }

	 if ( TYPE & RADEON_TEX0_BIT ) {
	    *(GLuint *)&O[6] = *(GLuint *)&I[6];
	    *(GLuint *)&O[7] = *(GLuint *)&I[7];
	 }

	 if ( TYPE & RADEON_TEX1_BIT ) {
	    *(GLuint *)&O[8] = *(GLuint *)&I[8];
	    *(GLuint *)&O[9] = *(GLuint *)&I[9];
	 }

	 O -= buffer_stride;
      }
   }

   rmesa->next_vert = O;
   rmesa->next_vert_index = index;
}



static void TAG(radeon_init_eltpath)( struct radeon_elt_tab *tab )
{
   tab->emit_unclipped_verts	= TAG(emit_unclipped_verts);
   tab->build_tri_verts		= TAG(build_tri_verts);
   tab->interp			= TAG(interp);
   tab->project_and_emit_verts	= TAG(project_and_emit_verts);
}

#undef TYPE
#undef TAG
#undef STRIDE
