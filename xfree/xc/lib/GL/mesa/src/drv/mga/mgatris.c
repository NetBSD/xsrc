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
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgatris.c,v 1.8 2001/04/10 16:07:51 dawes Exp $ */

#include <stdio.h>
#include <math.h>

#include "types.h"
#include "vb.h"
#include "pipeline.h"

#include "mm.h"
#include "mgacontext.h"
#include "mgatris.h"
#include "mgavb.h"


#define MGA_COLOR(to, from) {			\
  (to)[0] = (from)[2];				\
  (to)[1] = (from)[1];				\
  (to)[2] = (from)[0];				\
  (to)[3] = (from)[3];				\
}

#define MGA_COLOR3(to, from) {			\
  (to)[0] = (from)[2];				\
  (to)[1] = (from)[1];				\
  (to)[2] = (from)[0];				\
}



static triangle_func tri_tab[0x10];
static quad_func     quad_tab[0x10];
static line_func     line_tab[0x10];
static points_func   points_tab[0x10];

#define IND (0)
#define TAG(x) x
#include "mgatritmp.h"

#define IND (MGA_FLAT_BIT)
#define TAG(x) x##_flat
#include "mgatritmp.h"

#define IND (MGA_OFFSET_BIT)
#define TAG(x) x##_offset
#include "mgatritmp.h"

#define IND (MGA_OFFSET_BIT|MGA_FLAT_BIT)
#define TAG(x) x##_offset_flat
#include "mgatritmp.h"

#define IND (MGA_TWOSIDE_BIT)
#define TAG(x) x##_twoside
#include "mgatritmp.h"

#define IND (MGA_TWOSIDE_BIT|MGA_FLAT_BIT)
#define TAG(x) x##_twoside_flat
#include "mgatritmp.h"

#define IND (MGA_TWOSIDE_BIT|MGA_OFFSET_BIT)
#define TAG(x) x##_twoside_offset
#include "mgatritmp.h"

#define IND (MGA_TWOSIDE_BIT|MGA_OFFSET_BIT|MGA_FLAT_BIT)
#define TAG(x) x##_twoside_offset_flat
#include "mgatritmp.h"


void mgaDDTrifuncInit()
{
   init();
   init_flat();
   init_offset();
   init_offset_flat();
   init_twoside();
   init_twoside_flat();
   init_twoside_offset();
   init_twoside_offset_flat();
}



#define ALL_FALLBACK (DD_SELECT | DD_FEEDBACK)
#define POINT_FALLBACK (ALL_FALLBACK | DD_POINT_SMOOTH)
#define LINE_FALLBACK (ALL_FALLBACK | DD_LINE_SMOOTH | DD_LINE_STIPPLE)
#define TRI_FALLBACK (ALL_FALLBACK | DD_TRI_SMOOTH | DD_TRI_UNFILLED)
#define ANY_FALLBACK (POINT_FALLBACK|LINE_FALLBACK|TRI_FALLBACK|DD_TRI_STIPPLE)
#define ANY_RASTER_FLAGS (DD_FLATSHADE|DD_TRI_LIGHT_TWOSIDE|DD_TRI_OFFSET)

/* Setup the Point, Line, Triangle and Quad functions based on the
   current rendering state.  Wherever possible, use the hardware to
   render the primitive.  Otherwise, fallback to software rendering. */
void mgaDDChooseRenderState(GLcontext *ctx)
{
    mgaContextPtr mmesa = MGA_CONTEXT(ctx);
    GLuint flags = ctx->TriangleCaps;
    GLuint index = 0;

    if (mmesa->Fallback) {
	mmesa->renderindex = MGA_FALLBACK_BIT;
        if (flags & DD_TRI_LIGHT_TWOSIDE) {
           mmesa->IndirectTriangles = DD_TRI_LIGHT_TWOSIDE;
        }
	return;
    }

    if (flags & ANY_RASTER_FLAGS) {
	if (flags & DD_FLATSHADE)               index |= MGA_FLAT_BIT;
	if (flags & DD_TRI_LIGHT_TWOSIDE)       index |= MGA_TWOSIDE_BIT;
	if (flags & DD_TRI_OFFSET)              index |= MGA_OFFSET_BIT;
    }

    mmesa->PointsFunc = points_tab[index];
    mmesa->LineFunc = line_tab[index];
    mmesa->TriangleFunc = tri_tab[index];
    mmesa->QuadFunc = quad_tab[index];

    mmesa->renderindex = index;
    mmesa->IndirectTriangles = 0;

    if (flags & ANY_FALLBACK) {
	if (flags & POINT_FALLBACK) {
	    mmesa->renderindex |= MGA_FALLBACK_BIT;
	    mmesa->PointsFunc = 0;
	    mmesa->IndirectTriangles |= DD_POINT_SW_RASTERIZE;
	}

	if (flags & LINE_FALLBACK) {
	    mmesa->renderindex |= MGA_FALLBACK_BIT;
	    mmesa->LineFunc = 0;
	    mmesa->IndirectTriangles |= DD_LINE_SW_RASTERIZE;
	}

	if (flags & TRI_FALLBACK) {
	    mmesa->renderindex |= MGA_FALLBACK_BIT;
	    mmesa->TriangleFunc = 0;
	    mmesa->QuadFunc = 0;
	    mmesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
					 DD_QUAD_SW_RASTERIZE);
	}
	/* Special cases:
	 */
	if ((flags & DD_TRI_STIPPLE) &&
	    (ctx->IndirectTriangles & DD_TRI_STIPPLE)) {
	    mmesa->renderindex |= MGA_FALLBACK_BIT;
	    mmesa->TriangleFunc = 0;
	    mmesa->QuadFunc = 0;
	    mmesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
					 DD_QUAD_SW_RASTERIZE);
            if (flags & DD_TRI_LIGHT_TWOSIDE) {
               mmesa->IndirectTriangles |= DD_TRI_LIGHT_TWOSIDE;
            }
	}
    }
}
