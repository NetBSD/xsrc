/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_tris.h,v 1.2 2001/03/21 16:14:25 dawes Exp $ */
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
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef __RADEON_TRIS_H__
#define __RADEON_TRIS_H__

#ifdef GLX_DIRECT_RENDERING

#include "radeon_vb.h"

extern void radeonDDChooseRenderState( GLcontext *ctx );
extern void radeonDDTriangleFuncsInit( void );

#define RADEON_ANTIALIAS_BIT	0x00	/* GH: Do we need this? */
#define RADEON_FLAT_BIT		0x01
#define RADEON_OFFSET_BIT	0x02
#define RADEON_TWOSIDE_BIT	0x04
#define RADEON_NODRAW_BIT	0x08
#define RADEON_FALLBACK_BIT	0x10
#define RADEON_MAX_TRIFUNC	0x20


static __inline void radeon_draw_triangle( radeonContextPtr rmesa,
					   radeonVertexPtr v0,
					   radeonVertexPtr v1,
					   radeonVertexPtr v2 )
{
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = radeonAllocVerticesInline( rmesa, 3 );
   GLuint j;

#if defined (USE_X86_ASM)
   /* GH: We can safely assume the vertex stride is some number of
    * dwords, and thus a "rep movsd" is okay.  The vb pointer is
    * automagically updated with this instruction, so we don't have
    * to manually take care of incrementing it.
    */
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
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v0->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v1->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v2->ui[j];
#endif
}

static __inline void radeon_draw_quad( radeonContextPtr rmesa,
				       radeonVertexPtr v0,
				       radeonVertexPtr v1,
				       radeonVertexPtr v2,
				       radeonVertexPtr v3 )
{
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = radeonAllocVerticesInline( rmesa, 6 );
   GLuint j;

#if defined (USE_X86_ASM)
   /* GH: We can safely assume the vertex stride is some number of
    * dwords, and thus a "rep movsd" is okay.  The vb pointer is
    * automagically updated with this instruction, so we don't have
    * to manually take care of incrementing it.
    */
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
			 : "0" (vertsize), "S" ((long)v3)
			 : "memory" );
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "S" ((long)v1)
			 : "memory" );
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "S" ((long)v2)
			 : "memory" );
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "S" ((long)v3)
			 : "memory" );
#else
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v0->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v1->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v3->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v1->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v2->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = v3->ui[j];
#endif
}

static __inline void radeon_draw_line( radeonContextPtr rmesa,
				       radeonVertexPtr tmp0,
				       radeonVertexPtr tmp1,
				       GLfloat width )
{
#if 1
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = radeonAllocVerticesInline( rmesa, 6 );
   GLfloat hw, dx, dy, ix, iy;
   GLuint j;
   GLfloat x0 = tmp0->v.x;
   GLfloat y0 = tmp0->v.y;
   GLfloat x1 = tmp1->v.x;
   GLfloat y1 = tmp1->v.y;

   hw = 0.5F * width;
   if (hw > 0.1F && hw < 0.5F) {
      hw = 0.5F;
   }

   /* adjust vertices depending on line direction */
   dx = tmp0->v.x - tmp1->v.x;
   dy = tmp0->v.y - tmp1->v.y;
   if (dx * dx > dy * dy) {
      /* X-major line */
      ix = 0.0F;
      iy = hw;
      if (x1 < x0) {
         x0 += 0.5F;
         x1 += 0.5F;
      }
      y0 -= 0.5F;
      y1 -= 0.5F;
   }
   else {
      /* Y-major line */
      ix = hw;
      iy = 0.0F;
      if (y1 > y0) {
         y0 -= 0.5F;
         y1 -= 0.5F;
      }
      x0 += 0.5F;
      x1 += 0.5F;
   }

   *(float *)&vb[0] = x0 - ix;
   *(float *)&vb[1] = y0 - iy;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp0->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x1 + ix;
   *(float *)&vb[1] = y1 + iy;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp1->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x0 + ix;
   *(float *)&vb[1] = y0 + iy;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp0->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x0 - ix;
   *(float *)&vb[1] = y0 - iy;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp0->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x1 - ix;
   *(float *)&vb[1] = y1 - iy;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp1->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x1 + ix;
   *(float *)&vb[1] = y1 + iy;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp1->ui[j];
#else
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = radeonAllocVerticesInline( rmesa, RADEON_LINES, 2 );
   GLuint j;

#if defined (USE_X86_ASM)
   /* GH: We can safely assume the vertex stride is some number of
    * dwords, and thus a "rep movsd" is okay.  The vb pointer is
    * automagically updated with this instruction, so we don't have
    * to manually take care of incrementing it.
    */
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "D" ((long)vb), "S" ((long)tmp0)
			 : "memory" );
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "S" ((long)tmp1)
			 : "memory" );
#else
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = tmp0->ui[j];

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = tmp1->ui[j];
#endif
#endif
}

static __inline void radeon_draw_point( radeonContextPtr rmesa,
					radeonVertexPtr tmp, GLfloat sz )
{
#if 1
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = radeonAllocVerticesInline( rmesa, 6 );
   GLuint j;
   const float x = tmp->v.x + PNT_X_OFFSET;
   const float y = tmp->v.y + PNT_Y_OFFSET;

   *(float *)&vb[0] = x - sz;
   *(float *)&vb[1] = y - sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x + sz;
   *(float *)&vb[1] = y - sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x + sz;
   *(float *)&vb[1] = y + sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x + sz;
   *(float *)&vb[1] = y + sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x - sz;
   *(float *)&vb[1] = y + sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];
   vb += vertsize;

   *(float *)&vb[0] = x - sz;
   *(float *)&vb[1] = y - sz;
   for (j = 2 ; j < vertsize ; j++)
      vb[j] = tmp->ui[j];

#else
   int vertsize = rmesa->vertsize;
   CARD32 *vb = radeonAllocVerticesInline( rmesa, RADEON_3_VERTEX_POINTS, 1 );
   int j;

#if defined (USE_X86_ASM)
   /* GH: We can safely assume the vertex stride is some number of
    * dwords, and thus a "rep movsd" is okay.  The vb pointer is
    * automagically updated with this instruction, so we don't have
    * to manually take care of incrementing it.
    */
   __asm__ __volatile__( "rep ; movsl"
			 : "=%c" (j)
			 : "0" (vertsize), "D" ((long)vb), "S" ((long)tmp)
			 : "memory" );
#else
   for ( j = 0 ; j < vertsize ; j++ )
      vb[j] = tmp->ui[j];
#endif
#endif
}

#endif
#endif /* __RADEON_TRIS_H__ */
