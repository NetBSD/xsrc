/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_elttmp.h,v 1.1 2000/12/04 19:21:46 dawes Exp $ */
/*
 * DRI Hardware Device Driver for G200/G400
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

/* Buffers fill from high addresses down with vertices and from low
 * addresses up with elements.
 */


/* Emit the bulk of the vertices to the first dma buffer.  Leave
 * empty slots for clipped vertices so that we can still address
 * vertices by index.
 */
static void TAG(emit_unclipped_verts)( struct vertex_buffer *VB )
{
   GLuint i;
   r128ContextPtr r128ctx = R128_CONTEXT(VB->ctx);
   GLfloat *dev = VB->Projected->start;
   GLubyte *color = VB->ColorPtr->start;
   GLfloat *tex0_data = VB->TexCoordPtr[0]->start;
   GLfloat *tex1_data = VB->TexCoordPtr[1]->start;
   GLuint color_stride = VB->ColorPtr->stride;
   GLuint tex0_stride = VB->TexCoordPtr[0]->stride;
   GLuint tex1_stride = VB->TexCoordPtr[1]->stride;
   GLuint buffer_stride = r128ctx->vertsize;

   GLfloat *f = r128ctx->next_vert;
   GLuint count = VB->Count;
   GLubyte *clipmask = VB->ClipMask;

   const GLfloat *m = r128ctx->device_matrix;
   const GLfloat sx = m[0], sy = m[5], sz = m[10];
   const GLfloat tx = m[12], ty = m[13], tz = m[14];

   if ( 0 )
      fprintf( stderr, "%s: stride=%d\n", __FUNCTION__, buffer_stride );

   r128ctx->retained_buf = r128ctx->elt_buf;
   r128ctx->first_vert_index = r128ctx->next_vert_index;

   for ( i = 0 ; i < count ; f -= buffer_stride, i++ )
   {
      if ( !clipmask[i] )
      {
	 if ( 0 )
	    fprintf( stderr, "vert=%d addr=%p space=0x%x\n",
		     i, f, (GLuint)f - (GLuint)r128ctx->elt_buf->address );

	 f[0] = sx * dev[0] + tx;
	 f[1] = sy * dev[1] + ty;
	 f[2] = sz * dev[2] + tz;
	 f[3] = dev[3];

	 if (TYPE & R128_RGBA_BIT) {
#if 0 /*defined(USE_X86_ASM)*/
	    __asm__ (
	       "movl (%%edx),%%eax   \n"
	       "bswap %%eax          \n"
	       "rorl $8,%%eax        \n"
	       "movl %%eax,16(%%edi) \n"
	       :
	       : "d" (color), "D" (f)
	       : "%eax" );
#else
	    GLubyte *b = (GLubyte *)&f[4];
	    b[CLIP_UBYTE_B] = color[2];
	    b[CLIP_UBYTE_G] = color[1];
	    b[CLIP_UBYTE_R] = color[0];
	    b[CLIP_UBYTE_A] = color[3];
#endif
	 }

	 if (TYPE & R128_TEX0_BIT) {
	    *(int*)&f[6] = *(int*)&tex0_data[0];
	    *(int*)&f[7] = *(int*)&tex0_data[1];
	 }

	 if (TYPE & R128_TEX1_BIT) {
	    *(int*)&f[8] = *(int*)&tex1_data[0];
	    *(int*)&f[9] = *(int*)&tex1_data[1];
	 }
      }

      STRIDE_F(dev, 16);
      if (TYPE & R128_RGBA_BIT) color += color_stride;
      if (TYPE & R128_TEX0_BIT) STRIDE_F(tex0_data, tex0_stride);
      if (TYPE & R128_TEX1_BIT) STRIDE_F(tex1_data, tex1_stride);
   }

   r128ctx->next_vert = f;
   r128ctx->next_vert_index -= count;
}


/* Build three temporary clipspace vertex for clipping a triangle.
 * Recreate from the VB data rather than trying to read back from
 * uncached memory.
 */
static void TAG(build_tri_verts)( r128ContextPtr r128ctx,
				  struct vertex_buffer *VB,
				  GLfloat *O,
				  GLuint *elt )
{
   int i;

   for ( i = 0 ; i < 3 ; i++, O += CLIP_STRIDE ) {
      GLfloat *clip = VB->Clip.start + elt[i]*4;

      O[0] = clip[0];
      O[1] = clip[1];
      O[2] = clip[2];
      O[3] = clip[3];

      if (TYPE & R128_RGBA_BIT) {
	 GLubyte *col = VEC_ELT(VB->ColorPtr, GLubyte, elt[i]);
	 GLubyte *b = (GLubyte *)&O[4];
	 b[CLIP_UBYTE_R] = col[0];
	 b[CLIP_UBYTE_G] = col[1];
	 b[CLIP_UBYTE_B] = col[2];
	 b[CLIP_UBYTE_A] = col[3];
      }

      if ( 0 )
	 fprintf(stderr,
		 "build_tri_vert elt[%d]=%d index=0x%x (first_index=0x%x)\n",
		 i, elt[i], (GLuint)UNCLIPPED_VERT(elt[i]),
		 (GLuint)r128ctx->first_vert_index);

      *(GLuint *)&O[5] = UNCLIPPED_VERT(elt[i]);

      if (TYPE & R128_TEX0_BIT) {
	 GLfloat *tex0_data = VEC_ELT(VB->TexCoordPtr[0], GLfloat, elt[i]);
	 *(int*)&O[6] = *(int*)&tex0_data[0];
	 *(int*)&O[7] = *(int*)&tex0_data[1];
      }

      if (TYPE & R128_TEX1_BIT) {
	 GLfloat *tex1_data = VEC_ELT(VB->TexCoordPtr[1], GLfloat, elt[i]);
	 *(int*)&O[8] = *(int*)&tex1_data[0];
	 *(int*)&O[9] = *(int*)&tex1_data[1];
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
   O[0] = LINTERP(t, I[0], J[0]);
   O[1] = LINTERP(t, I[1], J[1]);
   O[2] = LINTERP(t, I[2], J[2]);
   O[3] = LINTERP(t, I[3], J[3]);

   if (TYPE & R128_RGBA_BIT) {
      INTERP_RGBA(t,
		  ((GLubyte *)&(O[4])),
		  ((GLubyte *)&(I[4])),
		  ((GLubyte *)&(J[4])));
   }

   if (0) fprintf(stderr, "setting 0x%x to ~0\n", (GLuint)&O[5]);

   *(GLuint *)&O[5] = ~0;	/* note that this is a new vertex */

   if (TYPE & R128_TEX0_BIT) {
      O[6] = LINTERP(t, I[6], J[6]);
      O[7] = LINTERP(t, I[7], J[7]);
   }

   if (TYPE & R128_TEX1_BIT) {
      O[8] = LINTERP(t, I[8], J[8]);
      O[9] = LINTERP(t, I[9], J[9]);
   }
}



/* When clipping is complete, scan the final vertex list and emit any
 * new ones to dma buffers.  Update the element list to a format
 * suitable for sending to hardware.
 */
static void TAG(project_and_emit_verts)( r128ContextPtr r128ctx,
					 const GLfloat *verts,
					 GLuint *elt,
					 int nr)
{
   GLfloat *O = r128ctx->next_vert;
   GLushort index = r128ctx->next_vert_index;
   GLuint buffer_stride = r128ctx->vertsize;

   const GLfloat *m = r128ctx->device_matrix;
   const GLfloat sx = m[0], sy = m[5], sz = m[10];
   const GLfloat tx = m[12], ty = m[13], tz = m[14];
   GLuint i;

   for (i = 0 ; i < nr ; i++) {
      const GLfloat *I = &verts[elt[i] * CLIP_STRIDE];
      GLuint tmp = *(GLuint *)&I[5];

      if (0) fprintf(stderr, "elt[%d] (tmp 0x%x %d) %d --> ",
		     i, (GLuint)&I[5], tmp, elt[i]);

      if ((elt[i] = tmp) == ~0)
      {
	 GLfloat oow = 1.0/I[3];

	 elt[i] = index--;

	 O[0] = sx * I[0] * oow + tx;
	 O[1] = sy * I[1] * oow + ty;
	 O[2] = sz * I[2] * oow + tz;
	 O[3] = oow;

	 if (TYPE & R128_RGBA_BIT) {
	    *(int*)&O[4] = *(int*)&I[4];
	 }

	 if (TYPE & R128_TEX0_BIT) {
	    *(int*)&O[6] = *(int*)&I[6];
	    *(int*)&O[7] = *(int*)&I[7];
	 }

	 if (TYPE & R128_TEX1_BIT) {
	    *(int*)&O[8] = *(int*)&I[8];
	    *(int*)&O[9] = *(int*)&I[9];
	 }

	 O -= buffer_stride;
      }
      if (0) fprintf(stderr, "0x%x\n", elt[i]);
   }

   r128ctx->next_vert = O;
   r128ctx->next_vert_index = index;
}



static void TAG(r128_init_eltpath)( struct r128_elt_tab *tab )
{
   tab->emit_unclipped_verts = TAG(emit_unclipped_verts);
   tab->build_tri_verts = TAG(build_tri_verts);
   tab->interp = TAG(interp);
   tab->project_and_emit_verts = TAG(project_and_emit_verts);
}

#undef TYPE
#undef TAG
#undef STRIDE
