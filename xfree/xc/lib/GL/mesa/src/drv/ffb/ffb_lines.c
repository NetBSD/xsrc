/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_lines.c,v 1.1 2000/06/20 05:08:38 dawes Exp $
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
#include "ffb_lines.h"
#include "ffb_lock.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"
#include "pipeline.h"

#define FFB_LINE_FLAT_BIT	0x01
#define FFB_LINE_TWOSIDE_BIT	0x02
#define FFB_LINE_OFFSET_BIT	0x04
#define FFB_LINE_WIDE_BIT	0x08
#define FFB_LINE_ALPHA_BIT	0x10
#define FFB_LINE_FALLBACK_BIT	0x20

static line_func ffb_line_tab[0x40];

/* If the line is not wide, we can support all of the line
 * patterning and smooth shading features of OpenGL fully.
 * If it is wide we use triangles to render them and as such
 * we lose the capability to do the patterning+shading in HW.
 *
 * XXX Actually, with the triangle method we can probably do
 * XXX the shading/antialiasing too if we are careful.  It might
 * XXX not work, need to investigate this. -DaveM
 */

#define IND (0)
#define TAG(x) x
#include "ffb_linetmp.h"

#define IND (FFB_LINE_FLAT_BIT)
#define TAG(x) x##_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_TWOSIDE_BIT)
#define TAG(x) x##_twoside
#include "ffb_linetmp.h"

#define IND (FFB_LINE_TWOSIDE_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_twoside_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_twoside_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_twoside_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT)
#define TAG(x) x##_wide
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_wide_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_wide_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_wide_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT)
#define TAG(x) x##_wide_twoside
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_wide_twoside_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_wide_twoside_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_wide_twoside_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT)
#define TAG(x) x##_alpha
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_alpha_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_TWOSIDE_BIT)
#define TAG(x) x##_alpha_twoside
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_twoside_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_alpha_twoside_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_twoside_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT)
#define TAG(x) x##_alpha_wide
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_wide_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_alpha_wide_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_wide_offset_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT)
#define TAG(x) x##_alpha_wide_twoside
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_wide_twoside_flat
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT)
#define TAG(x) x##_alpha_wide_twoside_offset
#include "ffb_linetmp.h"

#define IND (FFB_LINE_ALPHA_BIT|FFB_LINE_WIDE_BIT|FFB_LINE_TWOSIDE_BIT|FFB_LINE_OFFSET_BIT|FFB_LINE_FLAT_BIT)
#define TAG(x) x##_alpha_wide_twoside_offset_flat
#include "ffb_linetmp.h"

void ffbDDLinefuncInit(void)
{
	init();
	init_flat();
	init_offset();
	init_offset_flat();
	init_twoside();
	init_twoside_flat();
	init_twoside_offset();
	init_twoside_offset_flat();
	init_wide();
	init_wide_flat();
	init_wide_offset();
	init_wide_offset_flat();
	init_wide_twoside();
	init_wide_twoside_flat();
	init_wide_twoside_offset();
	init_wide_twoside_offset_flat();
	init_alpha();
	init_alpha_flat();
	init_alpha_offset();
	init_alpha_offset_flat();
	init_alpha_twoside();
	init_alpha_twoside_flat();
	init_alpha_twoside_offset();
	init_alpha_twoside_offset_flat();
	init_alpha_wide();
	init_alpha_wide_flat();
	init_alpha_wide_offset();
	init_alpha_wide_offset_flat();
	init_alpha_wide_twoside();
	init_alpha_wide_twoside_flat();
	init_alpha_wide_twoside_offset();
	init_alpha_wide_twoside_offset_flat();
}

/* Our caller makes sure TriangleCaps is non-zero and that
 * we are not doing feedback/selection (Mesa handles those
 * cases).
 */
void ffbDDChooseLineRenderState(GLcontext *ctx)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	GLuint flags = ctx->TriangleCaps;
	GLuint ind = 0;

	if (!flags) {
		fmesa->LineFunc = ffb_line_tab[0];
		return;
	}

	if (flags & DD_FLATSHADE)
		ind |= FFB_LINE_FLAT_BIT;
	if (flags & DD_TRI_OFFSET)
		ind |= FFB_LINE_OFFSET_BIT;
	if (flags & DD_TRI_LIGHT_TWOSIDE)
		ind |= FFB_LINE_TWOSIDE_BIT;
	if (flags & DD_LINE_WIDTH) {
		/* We cannot currently do wide lines with stipples
		 * or antialiased in HW.
		 */
		if ((flags & DD_LINE_STIPPLE) != 0 ||
		    ctx->Line.SmoothFlag)
			ind |= FFB_LINE_FALLBACK_BIT;
		else
			ind |= FFB_LINE_WIDE_BIT;
	} else {
		if ((flags & DD_LINE_STIPPLE) != 0 &&
		    fmesa->lpat == FFB_LPAT_BAD)
			ind |= FFB_LINE_FALLBACK_BIT;
	}

	/* If blending or the alpha test is enabled we need to
	 * provide alpha components to the chip, else we can
	 * do without it and thus feed vertex data to the chip
	 * more efficiently.
	 */
	if (ctx->Color.BlendEnabled || ctx->Color.AlphaEnabled)
		ind |= FFB_LINE_ALPHA_BIT;

	fmesa->LineFunc = ffb_line_tab[ind];
}
