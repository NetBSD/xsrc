/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_fasttmp.h,v 1.3 2001/01/08 01:07:20 martin Exp $ */
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
 *   Keith Whitwell <keithw@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

/* The first part of setup is applied to all vertices, clipped or
 * unclipped.  This data will be used for clipping, and then all
 * vertices with a zero clipmask will be projected to device space.
 *
 * This could be split into several loops, but - it seems that the
 * large stride of the fxVertices makes cache issues the big
 * performance factor, and that multiple loops mean multiple cache
 * misses....
 */
static void TAG(r128_setup_full)( struct vertex_buffer *VB,
				  GLuint do_cliptest )
{
   GLcontext *ctx = VB->ctx;
   r128VertexBufferPtr rvb = R128_DRIVER_DATA(VB);
   const GLfloat *m = ctx->ModelProjectMatrix.m;
   GLuint start  = VB->CopyStart;
   GLuint count  = VB->Count;

   gl_xform_points3_v16_general( rvb->verts[start].f,
				 m,
				 VB->ObjPtr->start,
				 VB->ObjPtr->stride,
				 count - start );

   if ( do_cliptest ) {
      VB->ClipAndMask = ~0;
      VB->ClipOrMask = 0;
      gl_cliptest_points4_v16( rvb->verts[start].f,
			       rvb->verts[count].f,
			       &(VB->ClipOrMask),
			       &(VB->ClipAndMask),
			       VB->ClipMask + start );
   }

   /* These branches are all resolved at compile time.  Hopefully all
    * the pointers are valid addresses even when not enabled.
    */
   if ( TYPE ) {
      GLubyte *color = VB->ColorPtr->start;
      GLfloat *tex0_data = VB->TexCoordPtr[0]->start;
      GLfloat *tex1_data = VB->TexCoordPtr[1]->start;

      GLuint color_stride = VB->ColorPtr->stride;
      GLuint tex0_stride = VB->TexCoordPtr[0]->stride;
      GLuint tex1_stride = VB->TexCoordPtr[1]->stride;

      GLfloat *f = rvb->verts[start].f;
      GLfloat *end = f + (16 * (count - start));

      while ( f != end ) {
	 if ( TYPE & R128_RGBA_BIT ) {
#if defined(USE_X86_ASM)
	    /* GH: I'd like to get some accurate timing data on the
	     * bswap instruction, but it gives a nice speedup anyway.
	     */
	    __asm__ ( "movl (%%edx),%%eax   \n"
		      "bswap %%eax          \n"
		      "rorl $8,%%eax        \n"
		      "movl %%eax,16(%%edi) \n"
		      :
		      : "d" (color), "D" (f)
		      : "%eax" );
#else
	    GLubyte *col = color;
	    GLubyte *b = (GLubyte *)&f[CLIP_UBYTE_COLOR];
	    b[CLIP_UBYTE_B] = col[2];
	    b[CLIP_UBYTE_G] = col[1];
	    b[CLIP_UBYTE_R] = col[0];
	    b[CLIP_UBYTE_A] = col[3];
#endif
	 }
	 if ( TYPE & R128_TEX0_BIT ) {
#if defined (USE_X86_ASM)
	    __asm__ ( "movl (%%ecx), %%eax    \n"
		      "movl %%eax, 24(%%edi)  \n"
		      "movl 4(%%ecx), %%eax   \n"
		      "movl %%eax, 28(%%edi)"
		      :
		      : "c" (tex0_data), "D" (f)
		      : "%eax" );
#else
	    *(GLuint *)(f+CLIP_S0) = *(GLuint *)tex0_data;
	    *(GLuint *)(f+CLIP_T0) = *(GLuint *)(tex0_data+1);
#endif
	 }
	 if ( TYPE & R128_TEX1_BIT ) {
	    /* Hits a second cache line.
	     */
#if defined (USE_X86_ASM)
	    __asm__ ( "movl (%%esi), %%eax    \n"
		      "movl %%eax, 32(%%edi)  \n"
		      "movl 4(%%esi), %%eax   \n"
		      "movl %%eax, 36(%%edi)"
		      :
		      : "S" (tex1_data), "D" (f)
		      : "%eax" );
#else
	    *(GLuint *)(f+CLIP_S1) = *(GLuint *)tex1_data;
	    *(GLuint *)(f+CLIP_T1) = *(GLuint *)(tex1_data+1);
#endif
	 }
	 if ( TYPE & R128_RGBA_BIT ) color += color_stride;
	 if ( TYPE & R128_TEX0_BIT ) STRIDE_F( tex0_data, tex0_stride );
	 if ( TYPE & R128_TEX1_BIT ) STRIDE_F( tex1_data, tex1_stride );
	 f += 16;
      }
   }

   rvb->clipped_elements.count = start;
   rvb->last_vert = count;
}


/* Changed to just put the interp func instead of the whole clip
 * routine into the header.  Less code and better chance of doing some
 * of this stuff in assembly.
 */
static void TAG(r128_interp_vert)( GLfloat t,
				   GLfloat *O,
				   const GLfloat *I,
				   const GLfloat *J )
{
   O[0] = LINTERP( t, I[0], J[0] );
   O[1] = LINTERP( t, I[1], J[1] );
   O[2] = LINTERP( t, I[2], J[2] );
   O[3] = LINTERP( t, I[3], J[3] );

   if ( TYPE & R128_RGBA_BIT ) {
      INTERP_RGBA( t,
		   ((GLubyte *)&(O[4])),
		   ((GLubyte *)&(I[4])),
		   ((GLubyte *)&(J[4])) );
   }

   if ( TYPE & R128_TEX0_BIT ) {
      O[6] = LINTERP( t, I[6], J[6] );
      O[7] = LINTERP( t, I[7], J[7] );
   }

   if ( TYPE & R128_TEX1_BIT ) {
      O[8] = LINTERP( t, I[8], J[8] );
      O[9] = LINTERP( t, I[9], J[9] );
   }
}


static void TAG(r128_init_fastpath)(struct r128_fast_tab *tab)
{
   tab->build_vertices	= TAG(r128_setup_full);
   tab->interp		= TAG(r128_interp_vert);
}

#undef TYPE
#undef TAG
#undef SIZE
