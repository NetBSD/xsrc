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
 *
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810tris.h,v 1.8 2000/12/04 19:21:43 dawes Exp $ */

#ifndef I810TRIS_INC
#define I810TRIS_INC

#include "types.h"
#include "i810ioctl.h"
#include "i810vb.h"

extern void i810PrintRenderState( const char *msg, GLuint state );
extern void i810DDChooseRenderState(GLcontext *ctx);
extern void i810DDTrifuncInit( void );


#define I810_FLAT_BIT 	     0x1
#define I810_OFFSET_BIT	     0x2
#define I810_TWOSIDE_BIT     0x4
#define I810_FALLBACK_BIT    0x8




static void __inline__ i810_draw_triangle( i810ContextPtr imesa,
					   i810VertexPtr v0,
					   i810VertexPtr v1,
					   i810VertexPtr v2 )
{
   GLuint vertsize = imesa->vertsize;
   GLuint *vb = i810AllocDwordsInline( imesa, 3 * vertsize );
   int j;

#if defined(USE_X86_ASM)
    __asm__ __volatile__( "rep ; movsl"
			  : "=%c" (j)
			  : "0" (vertsize), "D" ((long)vb), "S" ((long)v0)
			  : "memory" );
    __asm__ __volatile__( "rep ; movsl"
			  : "=%c" (j)
			  : "0" (vertsize), "S" ((long)v1)
			  : "memory" );
    __asm__ __volatile__( "rep ; movsl"
			  : "=%c" (j)
			  : "0" (vertsize), "S" ((long)v2)
			  : "memory" );
#else
   for (j = 0 ; j < vertsize ; j++)
      vb[j] = v0->ui[j];

   vb += vertsize;
   for (j = 0 ; j < vertsize ; j++)
      vb[j] = v1->ui[j];

   vb += vertsize;
   for (j = 0 ; j < vertsize ; j++)
      vb[j] = v2->ui[j];
#endif
}


static __inline__ void i810_draw_point( i810ContextPtr imesa,
					i810VertexPtr tmp,
					float sz )
{
   int vertsize = imesa->vertsize;
   GLuint *vb = i810AllocDwordsInline( imesa, 6 * vertsize );
   int j;

   *(float *)&vb[0] = tmp->v.x - sz;
   *(float *)&vb[1] = tmp->v.y - sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = tmp->v.x + sz;
   *(float *)&vb[1] = tmp->v.y - sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = tmp->v.x + sz;
   *(float *)&vb[1] = tmp->v.y + sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = tmp->v.x + sz;
   *(float *)&vb[1] = tmp->v.y + sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = tmp->v.x - sz;
   *(float *)&vb[1] = tmp->v.y + sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = tmp->v.x - sz;
   *(float *)&vb[1] = tmp->v.y - sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
}


static __inline__ void i810_draw_line( i810ContextPtr imesa,
				       i810VertexPtr v0,
				       i810VertexPtr v1 )
{
   GLuint vertsize = imesa->vertsize;
   GLuint *vb = i810AllocDwordsInline( imesa, 2 * vertsize );
   int j;

#if defined(USE_X86_ASM)
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "D" ((long)vb), "S" ((long)v0)
			 : "memory" );
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "S" ((long)v1)
			 : "memory" );
#else
   for (j = 0 ; j < vertsize ; j++)
      vb[j] = v0->ui[j];

   vb += vertsize;
   for (j = 0 ; j < vertsize ; j++)
      vb[j] = v1->ui[j];
#endif
}


#endif
