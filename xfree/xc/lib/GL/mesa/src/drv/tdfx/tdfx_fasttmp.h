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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_fasttmp.h,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
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

static void TAG(tdfx_setup_full)( struct vertex_buffer *VB,
				  GLuint do_cliptest )
{
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);
   GLcontext *ctx = VB->ctx;
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   const GLfloat * const m = ctx->ModelProjectMatrix.m;
   GLuint start = VB->CopyStart;
   GLuint count = VB->Count;

   if (0) fprintf(stderr, "%s\n", __FUNCTION__);

   gl_xform_points3_v16_general( TDFX_DRIVER_DATA(VB)->verts[start].f,
				 m,
				 VB->ObjPtr->start,
				 VB->ObjPtr->stride,
				 count - start );

   if ( do_cliptest ) {
      VB->ClipAndMask = ~0;
      VB->ClipOrMask = 0;
      gl_cliptest_points4_v16( fxVB->verts[start].f,
			       fxVB->verts[count].f,
			       &(VB->ClipOrMask),
			       &(VB->ClipAndMask),
			       VB->ClipMask + start );
   }

   /* These branches are all resolved at compile time.  Hopefully all
    * the pointers are valid addresses even when not enabled.
    */
   if ( TYPE ) {
      GLubyte *color = VB->ColorPtr->start;
      GLfloat *tex0_data = VB->TexCoordPtr[fxMesa->tmu_source[0]]->start;
      GLfloat *tex1_data = VB->TexCoordPtr[fxMesa->tmu_source[1]]->start;

      const GLuint color_stride = VB->ColorPtr->stride;
      const GLuint tex0_stride = VB->TexCoordPtr[fxMesa->tmu_source[0]]->stride;
      const GLuint tex1_stride = VB->TexCoordPtr[fxMesa->tmu_source[1]]->stride;

      GLfloat *f = fxVB->verts[start].f;
      GLfloat *end = f + (16 * (count - start));

      while ( f != end ) {
	 if ( TYPE & TDFX_RGBA_BIT ) {
#if defined(USE_X86_ASM)
	    __asm__ (
	    "movl (%%edx),%%eax   \n"
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
	 if (TYPE & TDFX_TEX0_BIT) {
#if defined (USE_X86_ASM)
	    __asm__ (
	    "movl (%%ecx), %%eax    \n"
	    "movl %%eax, 24(%%edi)  \n"
	    "movl 4(%%ecx), %%eax   \n"
	    "movl %%eax, 28(%%edi)"
	    :
	    : "c" (tex0_data), "D" (f)
	    : "%eax");
#else
	    *(unsigned int *)(f+CLIP_S0) = *(unsigned int *)tex0_data;
	    *(unsigned int *)(f+CLIP_T0) = *(unsigned int *)(tex0_data+1);
#endif
	 }
	 if (TYPE & TDFX_TEX1_BIT) {
	    /* Hits a second cache line.
	     */
#if defined (USE_X86_ASM)
	    __asm__ (
	    "movl (%%esi), %%eax    \n"
	    "movl %%eax, 32(%%edi)  \n"
	    "movl 4(%%esi), %%eax   \n"
	    "movl %%eax, 36(%%edi)"
	    :
	    : "S" (tex1_data), "D" (f)
	    : "%eax");
#else
	    *(unsigned int *)(f+CLIP_S1) = *(unsigned int *)tex1_data;
	    *(unsigned int *)(f+CLIP_T1) = *(unsigned int *)(tex1_data+1);
#endif
	 }
	 if ( TYPE & TDFX_RGBA_BIT ) color += color_stride;
	 if ( TYPE & TDFX_TEX0_BIT ) STRIDE_F( tex0_data, tex0_stride );
	 if ( TYPE & TDFX_TEX1_BIT ) STRIDE_F( tex1_data, tex1_stride );
	 f += 16;
      }
   }

   fxVB->clipped_elements.count = start;
   fxVB->last_vert = count;
}


/* Changed to just put the interp func instead of the whole clip
 * routine into the header.  Less code and better chance of doing some
 * of this stuff in assembly.
 */
static void TAG(tdfx_interp_vert)( GLfloat t,
				   GLfloat *O,
				   const GLfloat *I,
				   const GLfloat *J )
{
   O[0] = LINTERP( t, I[0], J[0] );
   O[1] = LINTERP( t, I[1], J[1] );
   O[2] = LINTERP( t, I[2], J[2] );
   O[3] = LINTERP( t, I[3], J[3] );

   if ( TYPE & TDFX_RGBA_BIT ) {
      INTERP_RGBA( t,
		   ((GLubyte *)&(O[4])),
		   ((GLubyte *)&(I[4])),
		   ((GLubyte *)&(J[4])) );
   }

   if ( TYPE & TDFX_TEX0_BIT ) {
      O[6] = LINTERP( t, I[6], J[6] );
      O[7] = LINTERP( t, I[7], J[7] );
   }

   if ( TYPE & TDFX_TEX1_BIT ) {
      O[8] = LINTERP( t, I[8], J[8] );
      O[9] = LINTERP( t, I[9], J[9] );
   }
}



static void TAG(tdfx_project_vertices)( struct vertex_buffer *VB )
{
   GLcontext *ctx = VB->ctx;
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);
   GLfloat *first = fxVB->verts[VB->CopyStart].f;
   GLfloat *last = fxVB->verts[fxVB->last_vert].f;
   GLfloat m[16];
   GLfloat *f;

   m[MAT_SX] = mat->m[MAT_SX];
   m[MAT_TX] = mat->m[MAT_TX] + fxMesa->x_offset + TRI_X_OFFSET;
   m[MAT_SY] = mat->m[MAT_SY];
   m[MAT_TY] = mat->m[MAT_TY] + fxMesa->y_delta + TRI_Y_OFFSET;
   m[MAT_SZ] = mat->m[MAT_SZ];
   m[MAT_TZ] = mat->m[MAT_TZ];

   gl_project_v16( first, last, m, 16 * 4 );

   /* V3 at least requires texcoords to be multiplied by 1/w:
    */
   if ( TYPE & (TDFX_TEX0_BIT|TDFX_TEX1_BIT) ) {

      const GLfloat sScale0 = fxMesa->sScale0;
      const GLfloat tScale0 = fxMesa->tScale0;
      const GLfloat sScale1 = fxMesa->sScale1;
      const GLfloat tScale1 = fxMesa->tScale1;


      for ( f = first ; f != last ; f += 16) {
	 const GLfloat oow = f[3];

	 if (TYPE & TDFX_TEX0_BIT) {
	    f[CLIP_S0] *= sScale0 * oow;
	    f[CLIP_T0] *= tScale0 * oow;
	 }

	 if (TYPE & TDFX_TEX1_BIT) {
	    f[CLIP_S1] *= sScale1 * oow;
	    f[CLIP_T1] *= tScale1 * oow;
	 }
      }
   }
}

static void TAG(tdfx_project_clipped_vertices)( struct vertex_buffer *VB )
{
   GLfloat *f;

   GLcontext *ctx = VB->ctx;
   GLmatrix *mat = &ctx->Viewport.WindowMap;
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   tdfxVertexBufferPtr fxVB = TDFX_DRIVER_DATA(VB);
   GLfloat *first = fxVB->verts[VB->CopyStart].f;
   GLfloat *last = fxVB->verts[fxVB->last_vert].f;
   const GLubyte *mask = VB->ClipMask + VB->CopyStart;
   GLfloat m[16];

   m[MAT_SX] = mat->m[MAT_SX];
   m[MAT_TX] = mat->m[MAT_TX] + fxMesa->x_offset + TRI_X_OFFSET;
   m[MAT_SY] = mat->m[MAT_SY];
   m[MAT_TY] = mat->m[MAT_TY] + fxMesa->y_delta + TRI_Y_OFFSET;
   m[MAT_SZ] = mat->m[MAT_SZ];
   m[MAT_TZ] = mat->m[MAT_TZ];

   gl_project_clipped_v16( first, last, m, 16 * 4, mask );

   /* V3 at least requires texcoords to be multiplied by 1/w:
    */
   if ( TYPE & (TDFX_TEX0_BIT|TDFX_TEX1_BIT) ) {

      const GLfloat sScale0 = fxMesa->sScale0;
      const GLfloat tScale0 = fxMesa->tScale0;
      const GLfloat sScale1 = fxMesa->sScale1;
      const GLfloat tScale1 = fxMesa->tScale1;

      for ( f = first ; f != last ; f += 16, mask++) {
	 if (!*mask) {
	    const GLfloat oow = f[3];
	    if (TYPE & TDFX_TEX0_BIT) {
	       f[CLIP_S0] *= sScale0 * oow;
	       f[CLIP_T0] *= tScale0 * oow;
	    }

	    if (TYPE & TDFX_TEX1_BIT) {
	       f[CLIP_S1] *= sScale1 * oow;
	       f[CLIP_T1] *= tScale1 * oow;
	    }
	 }
      }
   }
}

static void TAG(tdfx_init_fastpath)( struct tdfx_fast_tab *tab )
{
   tab->build_vertices	= TAG(tdfx_setup_full);
   tab->interp		= TAG(tdfx_interp_vert);
   tab->project_vertices = TAG(tdfx_project_vertices);
   tab->project_clipped_vertices = TAG(tdfx_project_clipped_vertices);
}

#undef TYPE
#undef TAG
#undef SIZE
