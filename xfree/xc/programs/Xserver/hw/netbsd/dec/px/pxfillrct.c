/*	$NetBSD: pxfillrct.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Acceleration for the Leo (ZX) framebuffer - Rectangle filling.
 *
 * Copyright (C) 1999, 2000 Jakub Jelinek (jakub@redhat.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "px.h"

#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"

#if defined(_STIPPLE)
#  if defined(_OPAQUE)
#    define PX_OP(x)		x##SO
#  else
#    define PX_OP(x)		x##S
#  endif
#else
#  define PX_OP(x)		x
#endif

#ifdef _STIPPLE
#define	DOFG(v1, v2, lw, xya, s, c)			\
	do {						\
		pb[0] = (s)[0];				\
		pb[1] = (s)[1];				\
		pb[2] = (s)[2];				\
		pb[3] = (s)[3];				\
		pb[4] = (s)[4];				\
		pb[5] = (s)[5];				\
		pb[6] = (s)[6];				\
		pb[7] = (s)[7];				\
		pb[8] = xya;				\
		pb[9] = v1;				\
		pb[10] = v2;				\
		pb[11] = lw;				\
		pb[12] = c;				\
	} while (0)

#define	DOBG(v1, v2, lw, xya, c)			\
	do {						\
		const int all1 = 0xffffffff;		\
							\
		pb[0] = all1;				\
		pb[1] = all1;				\
		pb[2] = all1;				\
		pb[3] = all1;				\
		pb[4] = all1;				\
		pb[5] = all1;				\
		pb[6] = all1;				\
		pb[7] = all1;				\
		pb[8] = xya;				\
		pb[9] = v1;				\
		pb[10] = v2;				\
		pb[11] = lw;				\
		pb[12] = c;				\
	} while (0)
#else
#define	DOFG(v1, v2, lw, xya, s, c)			\
	do {						\
		pb[0] = v1;				\
		pb[1] = v2;				\
		pb[2] = lw;				\
	} while (0)
#endif

void
PX_OP(pxPolyFillRect)(DrawablePtr pDrawable, GCPtr pGC,
		      int nrectFill, xRectangle *prectInit)
{
	pxPrivGCPtr gcPriv;
	pxScreenPrivPtr sp;
	xRectangle *prect, *prectmax;
	RegionPtr prgnClip;
	BoxPtr pbox, pboxmax, pextent;
	int xorg, yorg, v1, v2, psy, lw, x, y;
	int x1, y1, x2, y2, bx1, by1, bx2, by2, xx, yy;
	u_int32_t *pb;
	pxPacket pxp;
#ifdef _STIPPLE
	int xya, stampw, stamphm;
#endif

	PX_TRACE("##PX_OP(pxPolyFillRct)##");

	gcPriv = pxGetGCPrivate(pGC);
	sp = gcPriv->sp;
	prgnClip = cfbGetCompositeClip(pGC);

	xorg = pDrawable->x;
	yorg = pDrawable->y;
	prectmax = prectInit + nrectFill;

	if (xorg != 0 || yorg != 0) {
		prect = prectInit;
		while (prect < prectmax) {
			prect->x += xorg;
			prect->y += yorg;
			prect++;
		}
	}

	prect = prectInit;

#ifdef _STIPPLE
	pb = pxPacketStart(sp, &pxp, 4, 13);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT | STAMP_LW_PERPRIMATIVE |
	    STAMP_XY_PERPRIMATIVE;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet | STAMP_WE_XYMASK;

	stampw = sp->stampw;
	stamphm = sp->stamphm;
#else
	pb = pxPacketStart(sp, &pxp, 5, 3);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST | STAMP_LW_PERPRIMATIVE;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet;
	pb[4] = gcPriv->fgPixel;
#endif

	if (REGION_NUM_RECTS(prgnClip) == 1) {
		pextent = REGION_RECTS(prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;

		for (; prect < prectmax; prect++) {
			x = prect->x;
			y = prect->y;
			xx = x + prect->width;
			yy = y + prect->height;
			if (x < x1)
				x = x1;
			if (y < y1)
				y = y1;
			if (xx > x2)
				xx = x2;
			if (yy > y2)
				yy = y2;
			if (x >= xx)
				continue;
			if (y >= yy)
				continue;

			lw = ((yy - y) << 2) - 1;
			psy = (y << 3) + lw;
			v1 = (x << 19) | psy;
			v2 = (xx << 19) | psy;
#ifdef _STIPPLE
			xorg = (x - pGC->patOrg.x - pDrawable->x) & 15;
			yorg = (y - pGC->patOrg.y - pDrawable->y) & 15;
			xya = XYMASKADDR(stampw, stamphm, x, y, xorg, yorg);
#ifdef _OPAQUE
			pb = pxPacketAddPrim(sp, &pxp);
			DOBG(v1, v2, lw, xya, gcPriv->mask.bg);
#endif
#endif
			pb = pxPacketAddPrim(sp, &pxp);
			DOFG(v1, v2, lw, xya, (u_int32_t *)gcPriv->mask.data,
			    gcPriv->mask.fg);
		}
	} else {
		pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;

		for (; prect < prectmax; prect++) {
			if ((bx1 = prect->x) < x1)
				bx1 = x1;
    			if ((by1 = prect->y) < y1)
				by1 = y1;

			bx2 = (int) prect->x + (int) prect->width;
			if (bx2 > x2)
				bx2 = x2;
    			by2 = (int) prect->y + (int) prect->height;
			if (by2 > y2)
				by2 = y2;

			if (bx1 >= bx2 || by1 >= by2)
				continue;

			pbox = REGION_RECTS(prgnClip);
			pboxmax = pbox + REGION_NUM_RECTS(prgnClip);

			/*
			 * Clip the rectangle to each box in the clip region
			 * this is logically equivalent to calling Intersect().
			 */
			for (; pbox < pboxmax; pbox++) {
				x = max(bx1, pbox->x1);
				y = max(by1, pbox->y1);
				xx = min(bx2, pbox->x2);
				yy = min(by2, pbox->y2);

				/* see if clipping left anything */
				if (x >= xx || y >= yy)
					continue;

				lw = ((yy - y) << 2) - 1;
				psy = (y << 3) + lw;
				v1 = (x << 19) | psy;
				v2 = (xx << 19) | psy;
#ifdef _STIPPLE
				xorg = (x - pGC->patOrg.x - pDrawable->x) & 15;
				yorg = (y - pGC->patOrg.y - pDrawable->y) & 15;
				xya = XYMASKADDR(stampw, stamphm, x, y, xorg,
				    yorg);
#ifdef _OPAQUE
				pb = pxPacketAddPrim(sp, &pxp);
				DOBG(v1, v2, lw, xya, gcPriv->mask.bg);
#endif
#endif
				pb = pxPacketAddPrim(sp, &pxp);
				DOFG(v1, v2, lw, xya,
				    (u_int32_t *)gcPriv->mask.data,
				    gcPriv->mask.fg);
			}
		}
	}

	pxPacketFlush(sp, &pxp);
}
