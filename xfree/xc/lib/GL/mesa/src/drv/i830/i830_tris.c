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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_tris.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#include <stdio.h>
#include <math.h>

#include "types.h"
#include "vb.h"
#include "pipeline.h"

#include "mm.h"

#include "i830_drv.h"
#include "i830_tris.h"

/* Used in i830tritmp.h
 */
#define I830_COLOR(to, from) {			\
  (to)[0] = (from)[2];				\
  (to)[1] = (from)[1];				\
  (to)[2] = (from)[0];				\
  (to)[3] = (from)[3];				\
}

#define I830_COLOR3(to, from) {                  \
  (to)[0] = (from)[2];                           \
  (to)[1] = (from)[1];                           \
  (to)[2] = (from)[0];                           \
}

static triangle_func tri_tab[0x10];   
static quad_func     quad_tab[0x10];  
static line_func     line_tab[0x10];  
static points_func   points_tab[0x10];

#define IND (0)
#define TAG(x) x
#include "i830_tritmp.h"

#define IND (I830_FLAT_BIT)
#define TAG(x) x##_flat
#include "i830_tritmp.h"

#define IND (I830_OFFSET_BIT)	
#define TAG(x) x##_offset
#include "i830_tritmp.h"

#define IND (I830_OFFSET_BIT|I830_FLAT_BIT) 
#define TAG(x) x##_offset_flat
#include "i830_tritmp.h"

#define IND (I830_TWOSIDE_BIT)	
#define TAG(x) x##_twoside
#include "i830_tritmp.h"

#define IND (I830_TWOSIDE_BIT|I830_FLAT_BIT) 
#define TAG(x) x##_twoside_flat
#include "i830_tritmp.h"

#define IND (I830_TWOSIDE_BIT|I830_OFFSET_BIT) 
#define TAG(x) x##_twoside_offset
#include "i830_tritmp.h"

#define IND (I830_TWOSIDE_BIT|I830_OFFSET_BIT|I830_FLAT_BIT) 
#define TAG(x) x##_twoside_offset_flat
#include "i830_tritmp.h"



void i830DDTrifuncInit()
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
#define POINT_FALLBACK (ALL_FALLBACK | DD_POINT_SMOOTH | DD_POINT_ATTEN)
#define LINE_FALLBACK (ALL_FALLBACK | DD_LINE_STIPPLE)
#define TRI_FALLBACK (ALL_FALLBACK | DD_TRI_UNFILLED | DD_TRI_STIPPLE | DD_TRI_SMOOTH)
#define ANY_FALLBACK (POINT_FALLBACK|LINE_FALLBACK|TRI_FALLBACK|DD_TRI_STIPPLE)
#define ANY_RASTER_FLAGS (DD_FLATSHADE|DD_TRI_LIGHT_TWOSIDE|DD_TRI_OFFSET)

void i830DDChooseRenderState(GLcontext *ctx)
{
    i830ContextPtr imesa = I830_CONTEXT(ctx);
    GLuint         flags   = ctx->TriangleCaps;
    CARD32 index    = 0;

    if (imesa->Fallback) {
	imesa->renderindex = I830_FALLBACK_BIT;
	return;
    }
    
    if (flags & ANY_RASTER_FLAGS) {
	if (flags & DD_FLATSHADE)               index |= I830_FLAT_BIT;
	if (flags & DD_TRI_LIGHT_TWOSIDE)       index |= I830_TWOSIDE_BIT;
	if (flags & DD_TRI_OFFSET)              index |= I830_OFFSET_BIT; 
    }
	
    imesa->PointsFunc = points_tab[index];
    imesa->LineFunc = line_tab[index];
    imesa->TriangleFunc = tri_tab[index];
    imesa->QuadFunc = quad_tab[index];
    imesa->renderindex = index;
    imesa->IndirectTriangles = 0;

    if (flags & ANY_FALLBACK) {
	if (flags & POINT_FALLBACK) {
	    imesa->renderindex |= I830_FALLBACK_BIT;
	    imesa->PointsFunc = 0;
	    imesa->IndirectTriangles |= DD_POINT_SW_RASTERIZE;
	}
	    
	if (flags & LINE_FALLBACK) {
	    imesa->renderindex |= I830_FALLBACK_BIT;
	    imesa->LineFunc = 0;
	    imesa->IndirectTriangles |= DD_LINE_SW_RASTERIZE;
	}

	if (flags & TRI_FALLBACK) {
	    imesa->renderindex |= I830_FALLBACK_BIT;
	    imesa->TriangleFunc = 0;
	    imesa->QuadFunc = 0;
	    imesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
					 DD_QUAD_SW_RASTERIZE);
	}
	/* Special cases:
	 */
	if ((flags & DD_TRI_STIPPLE) &&
	    (ctx->IndirectTriangles & DD_TRI_STIPPLE)) {
	    imesa->renderindex |= I830_FALLBACK_BIT;
	    imesa->TriangleFunc = 0;
	    imesa->QuadFunc = 0;
	    imesa->IndirectTriangles |= (DD_TRI_SW_RASTERIZE |
					 DD_QUAD_SW_RASTERIZE);
	}
    }
}



