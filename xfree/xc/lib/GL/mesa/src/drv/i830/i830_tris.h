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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_tris.h,v 1.2 2002/01/17 09:50:58 eich Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#ifndef i830_TRIS_INC
#define i830_TRIS_INC

#include "types.h"

#include "i830_drv.h"
#include "i830_ioctl.h"

extern void i830PrintRenderState( const char *msg, GLuint state );
extern void i830DDChooseRenderState(GLcontext *ctx);
extern void i830DDTrifuncInit( void );


#define I830_FLAT_BIT 	     0x1
#define I830_OFFSET_BIT	     0x2	
#define I830_TWOSIDE_BIT     0x4
#define I830_FALLBACK_BIT    0x8




static void __inline__ i830_draw_triangle( i830ContextPtr imesa,
					   i830VertexPtr v0, 
					   i830VertexPtr v1, 
					   i830VertexPtr v2 )
{
   GLuint vertsize = imesa->vertsize;
   int j;
   GLuint *vb = i830AllocDwordsInline(imesa, 3 * vertsize);

#if 0

   if(imesa->vertex_prim != PRIM3D_TRILIST) fprintf(stderr, "Not tris and rendering tris\n");

   fprintf(stderr, "\n\n\n\nDrawTriangle :\n");
   fprintf(stderr, "v0\n");
   fprintf(stderr, "u: %f\n", v0->f[6]);
   fprintf(stderr, "v: %f\n", v0->f[7]);
   fprintf(stderr, "v1\n");
   fprintf(stderr, "u: %f\n", v1->f[6]);
   fprintf(stderr, "v: %f\n", v1->f[7]);
   fprintf(stderr, "v2\n");
   fprintf(stderr, "u: %f\n", v2->f[6]);
   fprintf(stderr, "v: %f\n", v2->f[7]);
#endif

#if 0
   fprintf(stderr, "\n\n\n\nDrawTriangle :\n");
   fprintf(stderr, "v0\n");
   fprintf(stderr, "0x%x\n", v0->ui[0]);
   fprintf(stderr, "0x%x\n", v0->ui[1]);
   fprintf(stderr, "0x%x\n", v0->ui[2]);
   fprintf(stderr, "0x%x\n", v0->ui[3]);
   fprintf(stderr, "0x%x\n", v0->ui[4]);
   fprintf(stderr, "0x%x\n", v0->ui[5]);
   fprintf(stderr, "0x%x\n", v0->ui[6]);
   fprintf(stderr, "0x%x\n", v0->ui[7]);
   fprintf(stderr, "0x%x\n", v0->ui[8]);
   fprintf(stderr, "0x%x\n", v0->ui[9]);
   fprintf(stderr, "0x%x\n", v0->ui[10]);
   fprintf(stderr, "0x%x\n", v0->ui[11]);
   fprintf(stderr, "0x%x\n", v0->ui[12]);
   fprintf(stderr, "0x%x\n", v0->ui[13]);
   fprintf(stderr, "0x%x\n", v0->ui[14]);
   fprintf(stderr, "0x%x\n", v0->ui[15]);
   fprintf(stderr, "v1\n");
   fprintf(stderr, "0x%x\n", v1->ui[0]);
   fprintf(stderr, "0x%x\n", v1->ui[1]);
   fprintf(stderr, "0x%x\n", v1->ui[2]);
   fprintf(stderr, "0x%x\n", v1->ui[3]);
   fprintf(stderr, "0x%x\n", v1->ui[4]);
   fprintf(stderr, "0x%x\n", v1->ui[5]);
   fprintf(stderr, "0x%x\n", v1->ui[6]);
   fprintf(stderr, "0x%x\n", v1->ui[7]);
   fprintf(stderr, "0x%x\n", v1->ui[8]);
   fprintf(stderr, "0x%x\n", v1->ui[9]);
   fprintf(stderr, "0x%x\n", v1->ui[10]);
   fprintf(stderr, "0x%x\n", v1->ui[11]);
   fprintf(stderr, "0x%x\n", v1->ui[12]);
   fprintf(stderr, "0x%x\n", v1->ui[13]);
   fprintf(stderr, "0x%x\n", v1->ui[14]);
   fprintf(stderr, "0x%x\n", v1->ui[15]);
   fprintf(stderr, "v2\n");
   fprintf(stderr, "0x%x\n", v2->ui[0]);
   fprintf(stderr, "0x%x\n", v2->ui[1]);
   fprintf(stderr, "0x%x\n", v2->ui[2]);
   fprintf(stderr, "0x%x\n", v2->ui[3]);
   fprintf(stderr, "0x%x\n", v2->ui[4]);
   fprintf(stderr, "0x%x\n", v2->ui[5]);
   fprintf(stderr, "0x%x\n", v2->ui[6]);
   fprintf(stderr, "0x%x\n", v2->ui[7]);
   fprintf(stderr, "0x%x\n", v2->ui[8]);
   fprintf(stderr, "0x%x\n", v2->ui[9]);
   fprintf(stderr, "0x%x\n", v2->ui[10]);
   fprintf(stderr, "0x%x\n", v2->ui[11]);
   fprintf(stderr, "0x%x\n", v2->ui[12]);
   fprintf(stderr, "0x%x\n", v2->ui[13]);
   fprintf(stderr, "0x%x\n", v2->ui[14]);
   fprintf(stderr, "0x%x\n", v2->ui[15]);
#endif

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

/* We aren't supporting point params, so we are ignoring size */
static __inline__ void i830_draw_point( i830ContextPtr imesa,
					  i830VertexPtr tmp,
					  float sz )
{
   GLuint vertsize = imesa->vertsize;
   int j;
   GLuint *vb = i830AllocDwordsInline( imesa, 1 * vertsize );

#if 0
   if(imesa->vertex_prim != PRIM3D_POINTLIST) fprintf(stderr, "Not points and rendering points\n");
#endif

#if defined(USE_X86_ASM)
    __asm__ __volatile__( "rep ; movsl"
			  : "=%c" (j)
			  : "0" (vertsize), "D" ((long)vb), "S" ((long)tmp)
			  : "memory" );
#else
    for (j = 0 ; j < vertsize ; j++)
        vb[j] = tmp->ui[j];

#endif

}

static __inline__ void i830_draw_line( i830ContextPtr imesa, 
				       i830VertexPtr v0, 
				       i830VertexPtr v1 )
{
   GLuint vertsize = imesa->vertsize;
   int j;
   GLuint *vb = i830AllocDwordsInline( imesa, 2 * vertsize );

#if 0
   if(imesa->vertex_prim != PRIM3D_LINELIST) fprintf(stderr, "Not lines and rendering lines : prim %d\n", imesa->vertex_prim);
#endif

#if 0
   fprintf(stderr, "\n\n\n\nDrawLine");
   fprintf(stderr, "v0\n");
   fprintf(stderr, "0x%x\n", v0->ui[0]);
   fprintf(stderr, "0x%x\n", v0->ui[1]);
   fprintf(stderr, "0x%x\n", v0->ui[2]);
   fprintf(stderr, "0x%x\n", v0->ui[3]);
   fprintf(stderr, "0x%x\n", v0->ui[4]);
   fprintf(stderr, "0x%x\n", v0->ui[5]);
   fprintf(stderr, "0x%x\n", v0->ui[6]);
   fprintf(stderr, "0x%x\n", v0->ui[7]);
   fprintf(stderr, "0x%x\n", v0->ui[8]);
   fprintf(stderr, "0x%x\n", v0->ui[9]);
   fprintf(stderr, "0x%x\n", v0->ui[10]);
   fprintf(stderr, "0x%x\n", v0->ui[11]);
   fprintf(stderr, "0x%x\n", v0->ui[12]);
   fprintf(stderr, "0x%x\n", v0->ui[13]);
   fprintf(stderr, "0x%x\n", v0->ui[14]);
   fprintf(stderr, "0x%x\n", v0->ui[15]);
   fprintf(stderr, "v1\n");
   fprintf(stderr, "0x%x\n", v1->ui[0]);
   fprintf(stderr, "0x%x\n", v1->ui[1]);
   fprintf(stderr, "0x%x\n", v1->ui[2]);
   fprintf(stderr, "0x%x\n", v1->ui[3]);
   fprintf(stderr, "0x%x\n", v1->ui[4]);
   fprintf(stderr, "0x%x\n", v1->ui[5]);
   fprintf(stderr, "0x%x\n", v1->ui[6]);
   fprintf(stderr, "0x%x\n", v1->ui[7]);
   fprintf(stderr, "0x%x\n", v1->ui[8]);
   fprintf(stderr, "0x%x\n", v1->ui[9]);
   fprintf(stderr, "0x%x\n", v1->ui[10]);
   fprintf(stderr, "0x%x\n", v1->ui[11]);
   fprintf(stderr, "0x%x\n", v1->ui[12]);
   fprintf(stderr, "0x%x\n", v1->ui[13]);
   fprintf(stderr, "0x%x\n", v1->ui[14]);
   fprintf(stderr, "0x%x\n", v1->ui[15]);
#endif

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
