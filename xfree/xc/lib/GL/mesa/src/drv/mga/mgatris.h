/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgatris.h,v 1.8 2001/04/10 16:07:51 dawes Exp $ */

#ifndef MGATRIS_INC
#define MGATRIS_INC

#include "types.h"
#include "mgaioctl.h"

extern void mgaDDChooseRenderState(GLcontext *ctx);
extern void mgaDDTrifuncInit( void );


#define MGA_FLAT_BIT	    0x1
#define MGA_OFFSET_BIT	    0x2
#define MGA_TWOSIDE_BIT	    0x4
#define MGA_FALLBACK_BIT    0x8

static __inline void mga_draw_triangle( mgaContextPtr mmesa,
				      mgaVertex *v0,
				      mgaVertex *v1,
				      mgaVertex *v2 )
{
   GLuint vertsize = mmesa->vertsize;
   GLuint *wv = mgaAllocVertexDwordsInline( mmesa, 3 * vertsize );
   int j;

#if defined (USE_X86_ASM)
   /* GTH: We can safely assume the vertex stride is some number of
    * dwords, and thus a "rep movsd" is okay.  The vb pointer is
    * automagically updated with this instruction, so we don't have
    * to manually take care of incrementing it.
    */
      __asm__ __volatile__( "rep ; movsl"
			    : "=%c" (j)
			    : "0" (vertsize), "D" ((long)wv), "S" ((long)v0)
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
   {
      for (j = 0 ; j < vertsize ; j++)
	 wv[j] = v0->ui[j];

      wv += vertsize;
      for (j = 0 ; j < vertsize ; j++)
	 wv[j] = v1->ui[j];

      wv += vertsize;
      for (j = 0 ; j < vertsize ; j++)
	 wv[j] = v2->ui[j];
   }
#endif
}


static __inline void mga_draw_point( mgaContextPtr mmesa,
				   mgaVertex *tmp, float sz )
{
   GLuint vertsize = mmesa->vertsize;
   GLuint *wv = mgaAllocVertexDwords( mmesa, 6*vertsize);
   GLuint j;
   const GLfloat x = tmp->v.x + 0.125;
   const GLfloat y = tmp->v.y - 0.125;

   *(float *)&wv[0] = x - sz;
   *(float *)&wv[1] = y - sz;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x + sz;
   *(float *)&wv[1] = y - sz;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x + sz;
   *(float *)&wv[1] = y + sz;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x + sz;
   *(float *)&wv[1] = y + sz;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x - sz;
   *(float *)&wv[1] = y + sz;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x - sz;
   *(float *)&wv[1] = y - sz;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp->ui[j];
}


static __inline void mga_draw_line( mgaContextPtr mmesa,
				  const mgaVertex *tmp0,
				  const mgaVertex *tmp1,
				  float width )
{
   const GLuint vertsize = mmesa->vertsize;
   GLuint *wv = mgaAllocVertexDwords( mmesa, 6 * vertsize );
   GLuint j;
   GLfloat x0 = tmp0->v.x;
   GLfloat y0 = tmp0->v.y;
   GLfloat x1 = tmp1->v.x;
   GLfloat y1 = tmp1->v.y;
   GLfloat dx, dy, ix, iy;
   GLfloat hw;

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

   *(float *)&wv[0] = x0 - ix;
   *(float *)&wv[1] = y0 - iy;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp0->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x1 + ix;
   *(float *)&wv[1] = y1 + iy;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp1->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x0 + ix;
   *(float *)&wv[1] = y0 + iy;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp0->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x0 - ix;
   *(float *)&wv[1] = y0 - iy;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp0->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x1 - ix;
   *(float *)&wv[1] = y1 - iy;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp1->ui[j];
   wv += vertsize;

   *(float *)&wv[0] = x1 + ix;
   *(float *)&wv[1] = y1 + iy;
   for (j = 2 ; j < vertsize ; j++)
      wv[j] = tmp1->ui[j];
   wv += vertsize;
}




#endif
