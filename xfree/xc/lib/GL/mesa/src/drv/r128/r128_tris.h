/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_tris.h,v 1.6 2001/04/10 16:07:53 dawes Exp $ */
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
 *   Gareth Hughes <gareth@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *
 */

#ifndef __R128_TRIS_H__
#define __R128_TRIS_H__

#ifdef GLX_DIRECT_RENDERING

#include "r128_vb.h"

extern void r128DDChooseRenderState( GLcontext *ctx );
extern void r128DDTriangleFuncsInit( void );

#define R128_FLAT_BIT		0x01
#define R128_OFFSET_BIT		0x02
#define R128_TWOSIDE_BIT	0x04
#define R128_NODRAW_BIT		0x08
#define R128_FALLBACK_BIT	0x10
#define R128_MAX_TRIFUNC	0x20


static __inline void r128_draw_triangle( r128ContextPtr rmesa,
					 r128VertexPtr v0,
					 r128VertexPtr v1,
					 r128VertexPtr v2 )
{
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = r128AllocVerticesInline( rmesa, 3 );
   int j;

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
      LE32_OUT( vb[j], v0->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v1->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v2->ui[j] );
#endif
}

static __inline void r128_draw_quad( r128ContextPtr rmesa,
				     r128VertexPtr v0,
				     r128VertexPtr v1,
				     r128VertexPtr v2,
				     r128VertexPtr v3 )
{
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = r128AllocVerticesInline( rmesa, 6 );
   int j;

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
      LE32_OUT( vb[j], v0->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v1->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v3->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v1->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v2->ui[j] );

   vb += vertsize;
   for ( j = 0 ; j < vertsize ; j++ )
      LE32_OUT( vb[j], v3->ui[j] );
#endif
}

static __inline void r128_draw_line( r128ContextPtr rmesa,
				     r128VertexPtr tmp0,
				     r128VertexPtr tmp1,
				     GLfloat width )
{
#if 1
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = r128AllocVerticesInline( rmesa, 6 );
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

   LE32_OUT_FLOAT( vb[0], x0 - ix );
   LE32_OUT_FLOAT( vb[1], y0 - iy );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp0->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x1 + ix );
   LE32_OUT_FLOAT( vb[1], y1 + iy );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp1->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x0 + ix );
   LE32_OUT_FLOAT( vb[1], y0 + iy );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp0->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x0 - ix );
   LE32_OUT_FLOAT( vb[1], y0 - iy );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp0->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x1 - ix );
   LE32_OUT_FLOAT( vb[1], y1 - iy );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp1->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x1 + ix );
   LE32_OUT_FLOAT( vb[1], y1 + iy );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp1->ui[j] );

#else

   int vertsize = rmesa->vertsize;
   CARD32 *vb = r128AllocVerticesInline( rmesa, 2 );
   int j;

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

static __inline void r128_draw_point( r128ContextPtr rmesa,
				      r128VertexPtr tmp, GLfloat sz )
{
#if 1
   GLuint vertsize = rmesa->vertsize;
   CARD32 *vb = r128AllocVerticesInline( rmesa, 6 );
   int j;
   const float x = tmp->v.x + PNT_X_OFFSET;
   const float y = tmp->v.y + PNT_Y_OFFSET;

   LE32_OUT_FLOAT( vb[0], x - sz );
   LE32_OUT_FLOAT( vb[1], y - sz );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x + sz );
   LE32_OUT_FLOAT( vb[1], y - sz );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x + sz );
   LE32_OUT_FLOAT( vb[1], y + sz );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x + sz );
   LE32_OUT_FLOAT( vb[1], y + sz );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x - sz );
   LE32_OUT_FLOAT( vb[1], y + sz );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp->ui[j] );
   vb += vertsize;

   LE32_OUT_FLOAT( vb[0], x - sz );
   LE32_OUT_FLOAT( vb[1], y - sz );
   for (j = 2 ; j < vertsize ; j++)
      LE32_OUT( vb[j], tmp->ui[j] );

#else

   int vertsize = rmesa->vertsize;
   CARD32 *vb = r128AllocVerticesInline( rmesa, 1 );
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
#endif /* __R128_TRIS_H__ */
