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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_tris.h,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *	Keith Whitwell <keithw@valinux.com>
 *
 */

#ifndef __TDFX_TRIS_H__
#define __TDFX_TRIS_H__

#ifdef GLX_DIRECT_RENDERING

#include "tdfx_vb.h"
#include "tdfx_render.h"

extern void tdfxDDChooseRenderState( GLcontext *ctx );
extern void tdfxDDTriangleFuncsInit( void );


#define TDFX_FLAT_BIT		0x01
#define TDFX_OFFSET_BIT		0x02
#define TDFX_TWOSIDE_BIT	0x04
#define TDFX_CLIPRECT_BIT	0x10
#define TDFX_FALLBACK_BIT	0x20
#define TDFX_MAX_TRIFUNC        0x40


static __inline void tdfx_draw_triangle( tdfxContextPtr fxMesa,
					 tdfxVertex *v0,
					 tdfxVertex *v1,
					 tdfxVertex *v2 )
{
   grDrawTriangle( v0, v1, v2 );
}


static __inline void tdfx_draw_point( tdfxContextPtr fxMesa,
				      tdfxVertex *tmp, float sz )
{
   if ( sz <= 1.0 ) {
      /* Save and restore original x,y rather than copying whole
       * vertex.
       */
      GLfloat x = tmp->v.x, y = tmp->v.y;
      tmp->v.x += PNT_X_OFFSET - TRI_X_OFFSET;
      tmp->v.y += PNT_Y_OFFSET - TRI_Y_OFFSET;
      grDrawPoint( tmp );
      tmp->v.x = x;
      tmp->v.y = y;
   }
   else {
      const GLfloat xLeft  = tmp->v.x - 0.5 * sz - TRI_X_OFFSET + PNT_X_OFFSET;
      const GLfloat xRight = tmp->v.x + 0.5 * sz - TRI_X_OFFSET + PNT_X_OFFSET;
      const GLfloat yBot   = tmp->v.y - 0.5 * sz - TRI_Y_OFFSET + PNT_Y_OFFSET;
      const GLfloat yTop   = tmp->v.y + 0.5 * sz - TRI_Y_OFFSET + PNT_Y_OFFSET;
      tdfxVertex verts[4];

      verts[0] = *tmp;
      verts[1] = *tmp;
      verts[2] = *tmp;
      verts[3] = *tmp;

      verts[0].v.x = xLeft;
      verts[0].v.y = yBot;

      verts[1].v.x = xRight;
      verts[1].v.y = yBot;

      verts[2].v.x = xRight;
      verts[2].v.y = yTop;

      verts[3].v.x = xLeft;
      verts[3].v.y = yTop;

      grDrawVertexArrayContiguous( GR_TRIANGLE_FAN, 4, verts,
				   sizeof(tdfxVertex) );
   }
}


static __inline void tdfx_draw_line( tdfxContextPtr fxMesa,
				     tdfxVertex *tmp0,
				     tdfxVertex *tmp1,
				     float width )
{
   if ( width <= 1.0 )
   {
      /* Faster to save and restore 4 dwords than to copy 32 dwords.
       */
      GLfloat x0 = tmp0->v.x, y0 = tmp0->v.y;
      GLfloat x1 = tmp1->v.x, y1 = tmp1->v.y;
      tmp0->v.x += LINE_X_OFFSET - TRI_X_OFFSET;
      tmp0->v.y += LINE_Y_OFFSET - TRI_Y_OFFSET;
      tmp1->v.x += LINE_X_OFFSET - TRI_X_OFFSET;
      tmp1->v.y += LINE_Y_OFFSET - TRI_Y_OFFSET;
      grDrawLine(tmp0, tmp1);
      tmp0->v.x = x0;
      tmp0->v.y = y0;
      tmp1->v.x = x1;
      tmp1->v.y = y1;
   }
   else
   {
      tdfxVertex verts[4];
      float dx, dy, ix, iy;

      dx = tmp0->v.x - tmp1->v.x;
      dy = tmp0->v.y - tmp1->v.y;

      if (dx * dx > dy * dy) {
	 iy = width * .5;
	 ix = 0;
      } else {
	 iy = 0;
	 ix = width * .5;
      }

      verts[0] = *tmp0;
      verts[1] = *tmp0;
      verts[2] = *tmp1;
      verts[3] = *tmp1;

      verts[0].v.x = tmp0->v.x - ix;
      verts[0].v.y = tmp0->v.y - iy;

      verts[1].v.x = tmp0->v.x + ix;
      verts[1].v.y = tmp0->v.y + iy;

      verts[2].v.x = tmp1->v.x + ix;
      verts[2].v.y = tmp1->v.y + iy;

      verts[3].v.x = tmp1->v.x - ix;
      verts[3].v.y = tmp1->v.y - iy;

      grDrawVertexArrayContiguous( GR_TRIANGLE_FAN, 4, verts,
				   sizeof(tdfxVertex) );
   }
}

void tdfxDDToggleTriCliprects( GLcontext *ctx );


#endif /* GLX_DIRECT_RENDERING */

#endif /* __TDFX_TRIS_H__ */
