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



/* The first part of setup is applied to all vertices, clipped or
 * unclipped.  This data w!ill be used for clipping, and then all
 * vertices with a zero clipmask will be projected to device space.
 *
 * This could be split into several loops, but - it seems that the
 * large stride of the fxVertices makes cache issues the big
 * performance factor, and that multiple loops mean multiple cache
 * misses....
 */
static void TAG(i810_setup_full)( struct vertex_buffer *VB, GLuint do_cliptest )
{
   GLcontext *ctx = VB->ctx;
   const GLfloat * const m = ctx->ModelProjectMatrix.m;
   GLuint start = VB->CopyStart;
   GLuint count = VB->Count;
   GLuint i;

   gl_xform_points3_v16_general(I810_DRIVER_DATA(VB)->verts[start].f,
				m, 
				VB->ObjPtr->start,
				VB->ObjPtr->stride,
				count - start);

   if (do_cliptest)
   {
      VB->ClipAndMask = ~0; 
      VB->ClipOrMask = 0;
      gl_cliptest_points4_v16(I810_DRIVER_DATA(VB)->verts[start].f,
			      I810_DRIVER_DATA(VB)->verts[count].f,
			      &(VB->ClipOrMask),
			      &(VB->ClipAndMask),
			      VB->ClipMask + start);
   }

   /* These branches are all resolved at compile time.  Hopefully all
    * the pointers are valid addresses even when not enabled.
    */
   if (TYPE) {
      GLubyte *color = VB->ColorPtr->start;
      GLfloat *tex0_data = VB->TexCoordPtr[0]->start;
      GLfloat *tex1_data = VB->TexCoordPtr[1]->start;

      GLuint color_stride = VB->ColorPtr->stride;
      GLuint tex0_stride = VB->TexCoordPtr[0]->stride;
      GLuint tex1_stride = VB->TexCoordPtr[1]->stride;

      GLfloat *f = I810_DRIVER_DATA(VB)->verts[start].f;

      for (i = start ; i < count ; i++, f += 16) {
	 if (TYPE & I810_RGBA_BIT) {
	    GLubyte *b = (GLubyte *)&f[CLIP_UBYTE_COLOR];
	    GLubyte *col = color; color += color_stride;
	    b[CLIP_UBYTE_R] = col[0];
	    b[CLIP_UBYTE_G] = col[1];
	    b[CLIP_UBYTE_B] = col[2];
	    b[CLIP_UBYTE_A] = col[3];
	 }
	 if (TYPE & I810_TEX0_BIT) {
	    f[CLIP_S0] = tex0_data[0];
	    f[CLIP_T0] = tex0_data[1];
	    STRIDE_F(tex0_data, tex0_stride);
	 }
	 if (TYPE & I810_TEX1_BIT) {
	    f[CLIP_S1] = tex1_data[0];
	    f[CLIP_T1] = tex1_data[1];
	    STRIDE_F(tex1_data, tex1_stride);
	 }
      }
   }

   I810_DRIVER_DATA(VB)->clipped_elements.count = start;
   I810_DRIVER_DATA(VB)->last_vert = count;
}


/* Changed to just put the interp func instead of the whole clip
 * routine into the header.  Less code and better chance of doing some
 * of this stuff in assembly.
 */
static void TAG(i810_interp_vert)( GLfloat t, 
				  GLfloat *O,
				  const GLfloat *I,
				  const GLfloat *J )
{
   O[0] = LINTERP(t, I[0], J[0]);
   O[1] = LINTERP(t, I[1], J[1]);
   O[2] = LINTERP(t, I[2], J[2]);
   O[3] = LINTERP(t, I[3], J[3]);

   if (TYPE & I810_RGBA_BIT) {
      INTERP_RGBA(t,
		  ((GLubyte *)&(O[4])),
		  ((GLubyte *)&(I[4])),
		  ((GLubyte *)&(J[4])));
   }

   if (TYPE & I810_TEX0_BIT) {
      O[6] = LINTERP(t, I[6], J[6]);
      O[7] = LINTERP(t, I[7], J[7]);
   }

   if (TYPE & I810_TEX1_BIT) {
      O[8] = LINTERP(t, I[8], J[8]);
      O[9] = LINTERP(t, I[9], J[9]);
   }
}


static void TAG(i810_init_fastpath)( struct i810_fast_tab *tab )
{
   tab->interp = TAG(i810_interp_vert);
   tab->build_vertices = TAG(i810_setup_full);
}

#undef TYPE
#undef TAG
#undef SIZE
