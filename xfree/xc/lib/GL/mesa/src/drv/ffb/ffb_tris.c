/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_tris.c,v 1.1 2000/06/20 05:08:40 dawes Exp $
 *
 * GLX Hardware Device Driver for Sun Creator/Creator3D
 * Copyright (C) 2000 David S. Miller
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
 * DAVID MILLER, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    David S. Miller <davem@redhat.com>
 */

#include "types.h"
#include "vbrender.h"

#include <stdio.h>
#include <stdlib.h>

#include "mm.h"
#include "ffb_dd.h"
#include "ffb_span.h"
#include "ffb_depth.h"
#include "ffb_context.h"
#include "ffb_vb.h"
#include "ffb_tris.h"
#include "ffb_lock.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"

#undef TRI_DEBUG

#ifdef TRI_DEBUG
#define FFB_DUMP_PRIM(name)	\
		fprintf(stderr, "FFB: Begin primitive %s%s%s" #name ".\n", \
				(IND & FFB_TRI_OFFSET_BIT) ? "OFFSET " : "", \
				(IND & FFB_TRI_FLAT_BIT) ? "FLAT " : "", \
				(IND & FFB_TRI_TWOSIDE_BIT) ? "TWOSIDE " : "")
#define FFB_DUMP_VERTEX(V)	\
		fprintf(stderr, "FFB: VERTEX ARGB(%c%x.%08x:%c%x.%08x:%c%x.%08x:%c%x.%08x) " \
			"XYZ(%04x.%04x:%04x.%04x:%c%x.%08x)\n", \
			(((V)->color[which_color].alpha & (1 << 31)) ? '-' : ' '), \
			(((V)->color[which_color].alpha & (1 << 30)) >> 30), \
			(((V)->color[which_color].alpha & ((1 << 30) - 1))), \
			(((V)->color[which_color].red & (1 << 31)) ? '-' : ' '), \
			(((V)->color[which_color].red & (1 << 30)) >> 30), \
			(((V)->color[which_color].red & ((1 << 30) - 1))), \
			(((V)->color[which_color].green & (1 << 31)) ? '-' : ' '), \
			(((V)->color[which_color].green & (1 << 30)) >> 30), \
			(((V)->color[which_color].green & ((1 << 30) - 1))), \
			(((V)->color[which_color].blue & (1 << 31)) ? '-' : ' '), \
			(((V)->color[which_color].blue & (1 << 30)) >> 30), \
			(((V)->color[which_color].blue & ((1 << 30) - 1))), \
			(((V)->x >> 16)), (((V)->x & ((1 << 16) - 1))), \
			(((V)->y >> 16)), (((V)->y & ((1 << 16) - 1))), \
			(((V)->z & (1 << 31)) ? '-' : ' '), \
			(((V)->z & (1 << 30)) >> 30), \
			(((V)->z & ((1 << 30) - 1))))
#else
#define FFB_DUMP_PRIM(name)	do { } while(0)
#define FFB_DUMP_VERTEX(V)	do { } while(0)
#endif

#define FFB_TRI_FLAT_BIT	0x01
#define FFB_TRI_TWOSIDE_BIT	0x02
#define FFB_TRI_OFFSET_BIT	0x04
#define FFB_TRI_CULL_BIT	0x08
#define FFB_TRI_ALPHA_BIT	0x10
#define FFB_TRI_FALLBACK_BIT	0x20

static triangle_func ffb_tri_tab[0x40];
static quad_func ffb_quad_tab[0x40];

#define IND (0)
#define TAG(x) x
#include "ffb_tritmp.h"

#define IND (FFB_TRI_FLAT_BIT)
#define TAG(x) x##_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_TWOSIDE_BIT)
#define TAG(x) x##_twoside
#include "ffb_tritmp.h"

#define IND (FFB_TRI_TWOSIDE_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_twoside_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_twoside_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_twoside_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT)
#define TAG(x) x##_cull
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_cull_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_cull_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_cull_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT)
#define TAG(x) x##_cull_twoside
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_cull_twoside_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_cull_twoside_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_cull_twoside_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT)
#define TAG(x) x##_alpha
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_alpha_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_TWOSIDE_BIT)
#define TAG(x) x##_alpha_twoside
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_twoside_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_alpha_twoside_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_twoside_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT)
#define TAG(x) x##_alpha_cull
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_cull_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_alpha_cull_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_cull_offset_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT)
#define TAG(x) x##_alpha_cull_twoside
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_cull_twoside_flat
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT)
#define TAG(x) x##_alpha_cull_twoside_offset
#include "ffb_tritmp.h"

#define IND (FFB_TRI_ALPHA_BIT|FFB_TRI_CULL_BIT|FFB_TRI_TWOSIDE_BIT|FFB_TRI_OFFSET_BIT|FFB_TRI_FLAT_BIT)
#define TAG(x) x##_alpha_cull_twoside_offset_flat
#include "ffb_tritmp.h"

void ffbDDTrifuncInit()
{
	init();
	init_flat();
	init_offset();
	init_offset_flat();
	init_twoside();
	init_twoside_flat();
	init_twoside_offset();
	init_twoside_offset_flat();
	init_cull();
	init_cull_flat();
	init_cull_offset();
	init_cull_offset_flat();
	init_cull_twoside();
	init_cull_twoside_flat();
	init_cull_twoside_offset();
	init_cull_twoside_offset_flat();
	init_alpha();
	init_alpha_flat();
	init_alpha_offset();
	init_alpha_offset_flat();
	init_alpha_twoside();
	init_alpha_twoside_flat();
	init_alpha_twoside_offset();
	init_alpha_twoside_offset_flat();
	init_alpha_cull();
	init_alpha_cull_flat();
	init_alpha_cull_offset();
	init_alpha_cull_offset_flat();
	init_alpha_cull_twoside();
	init_alpha_cull_twoside_flat();
	init_alpha_cull_twoside_offset();
	init_alpha_cull_twoside_offset_flat();
}

void ffbDDChooseTriRenderState(GLcontext *ctx)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	GLuint flags = ctx->TriangleCaps;
	GLuint ind = 0;

	if (!flags) {
		fmesa->TriangleFunc = ffb_tri_tab[0];
		fmesa->QuadFunc = ffb_quad_tab[0];
		return;
	}

	if (ctx->Texture.ReallyEnabled) {
		fmesa->TriangleFunc = NULL;
		fmesa->QuadFunc = NULL;
		return;
	}

	if (flags & DD_FLATSHADE)
		ind |= FFB_TRI_FLAT_BIT;
	if (flags & DD_TRI_CULL)
		ind |= FFB_TRI_CULL_BIT;
	if (ctx->Polygon.SmoothFlag)
		ind |= FFB_TRI_FALLBACK_BIT;
	if (flags & DD_TRI_OFFSET)
		ind |= FFB_TRI_OFFSET_BIT;
	if (flags & DD_TRI_LIGHT_TWOSIDE)
		ind |= FFB_TRI_TWOSIDE_BIT;

	/* If blending or the alpha test is enabled we need to
	 * provide alpha components to the chip, else we can
	 * do without it and thus feed vertex data to the chip
	 * more efficiently.
	 */
	if (ctx->Color.BlendEnabled || ctx->Color.AlphaEnabled)
		ind |= FFB_TRI_ALPHA_BIT;

	fmesa->TriangleFunc = ffb_tri_tab[ind];
	fmesa->QuadFunc = ffb_quad_tab[ind];
}
