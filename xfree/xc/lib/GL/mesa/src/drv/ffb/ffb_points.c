/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_points.c,v 1.1 2000/06/20 05:08:39 dawes Exp $
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
#include "ffb_points.h"
#include "ffb_lock.h"
#include "extensions.h"
#include "vb.h"
#include "dd.h"
#include "pipeline.h"

#define FFB_POINT_OFFSET_BIT	0x01
#define FFB_POINT_AA_BIT	0x02
#define FFB_POINT_BIG_BIT	0x04
#define FFB_POINT_ALPHA_BIT	0x08
#define FFB_POINT_FALLBACK_BIT	0x10

static points_func ffb_points_tab[0x20];

#define IND (0)
#define TAG(x) x
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_OFFSET_BIT)
#define TAG(x) x##_offset
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_AA_BIT)
#define TAG(x) x##_aa
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_OFFSET_BIT|FFB_POINT_AA_BIT)
#define TAG(x) x##_offset_aa
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_BIG_BIT)
#define TAG(x) x##_big
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_BIG_BIT|FFB_POINT_OFFSET_BIT)
#define TAG(x) x##_big_offset
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_ALPHA_BIT)
#define TAG(x) x##_alpha
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_ALPHA_BIT|FFB_POINT_OFFSET_BIT)
#define TAG(x) x##_alpha_offset
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_ALPHA_BIT|FFB_POINT_AA_BIT)
#define TAG(x) x##_alpha_aa
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_ALPHA_BIT|FFB_POINT_OFFSET_BIT|FFB_POINT_AA_BIT)
#define TAG(x) x##_alpha_offset_aa
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_ALPHA_BIT|FFB_POINT_BIG_BIT)
#define TAG(x) x##_alpha_big
#include "ffb_pointtmp.h"

#define IND (FFB_POINT_ALPHA_BIT|FFB_POINT_BIG_BIT|FFB_POINT_OFFSET_BIT)
#define TAG(x) x##_alpha_big_offset
#include "ffb_pointtmp.h"

void ffbDDPointfuncInit(void)
{
	init();
	init_offset();
	init_aa();
	init_offset_aa();
	init_big();
	init_big_offset();
	init_alpha();
	init_alpha_offset();
	init_alpha_aa();
	init_alpha_offset_aa();
	init_alpha_big();
	init_alpha_big_offset();
}

/* Our caller makes sure TriangleCaps is non-zero and that
 * we are not doing feedback/selection (Mesa handles those
 * cases).
 */
void ffbDDChoosePointRenderState(GLcontext *ctx)
{
	ffbContextPtr fmesa = FFB_CONTEXT(ctx);
	GLuint flags = ctx->TriangleCaps;
	GLuint ind = 0;

	if (!flags) {
		fmesa->PointsFunc = ffb_points_tab[0];
		return;
	}

	if (flags & DD_TRI_OFFSET)
		ind |= FFB_POINT_OFFSET_BIT;
	if (flags & DD_POINT_SIZE) {
		if (ctx->Point.SmoothFlag)
			ind |= FFB_POINT_FALLBACK_BIT;
		else
			ind |= FFB_POINT_BIG_BIT;
	} else if (ctx->Point.SmoothFlag) {
		ind |= FFB_POINT_AA_BIT;
	}

	/* If blending or the alpha test is enabled we need to
	 * provide alpha components to the chip, else we can
	 * do without it and thus feed vertex data to the chip
	 * more efficiently.
	 */
	if (ctx->Color.BlendEnabled || ctx->Color.AlphaEnabled)
		ind |= FFB_POINT_ALPHA_BIT;

	fmesa->PointsFunc = ffb_points_tab[ind];
}
