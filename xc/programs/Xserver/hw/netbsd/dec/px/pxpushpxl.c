/*	$NetBSD: pxpushpxl.c,v 1.1 2001/09/18 20:02:54 ad Exp $	*/

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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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

#include "px.h"

#include "pixmapstr.h"
#include "regionstr.h"
#include "cfb.h"

void
pxSolidPP(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDrawable, int dx, int dy,
	  int xOrg, int yOrg)
{
	pxScreenPrivPtr sp;
	RegionRec rgnDst;
	BoxRec srcBox;
	BoxPtr pBox, pMaxBox;
	pxPacket pxp;
	u_int32_t *pb;
	pxPrivGCPtr gcPriv;

	PX_TRACE("pxSolidPP");

	srcBox.x1 = xOrg;
	srcBox.y1 = yOrg;
	srcBox.x2 = xOrg + dx;
	srcBox.y2 = yOrg + dy;
	REGION_INIT(pGC->pScreen, &rgnDst, &srcBox, 1);

	/* Clip the shape of the dst to the destination composite clip. */
	REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst,
	    cfbGetCompositeClip(pGC));

	if (!REGION_NIL(&rgnDst)) {
		gcPriv = pxGetGCPrivate(pGC);
		sp = gcPriv->sp;

		pb = pxPacketStart(sp, &pxp, 5, 12);
		pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST |
		    STAMP_LW_PERPRIMATIVE | STAMP_XY_PERPRIMATIVE;
		pb[1] = gcPriv->pmask;
		pb[2] = 0;
		pb[3] = gcPriv->umet | STAMP_WE_XYMASK;
		pb[4] = gcPriv->fgPixel;

		pBox = REGION_RECTS(&rgnDst);
		pMaxBox = pBox + REGION_NUM_RECTS(&rgnDst);

		if (pBox->y2 - pBox->y1 <= 16) {
			for (; pBox < pMaxBox; pBox++)
				pxSqueege16(sp, pBitMap, pDrawable, &pxp,
				    pBox->x1, pBox->y1, pBox->x2, pBox->y2,
				    pBox->x1 - xOrg, pBox->y1 - yOrg);
		} else {
			for (; pBox < pMaxBox; pBox++)
				pxSqueege(sp, pBitMap, pDrawable, &pxp,
				    pBox->x1, pBox->y1, pBox->x2, pBox->y2,
				    pBox->x1 - xOrg, pBox->y1 - yOrg);
		}

		pxPacketFlush(sp, &pxp);
	}

	REGION_UNINIT(pGC->pScreen, &rgnDst);
}

void
pxSqueege(pxScreenPrivPtr sp, PixmapPtr pix, DrawablePtr pDst,
	  pxPacketPtr pxp, int fx1, int fy1, int fx2, int fy2,
	  int sx, int sy)
{
	int stride, xrot, x1, x2, y, th, rh, psy, lw, tw, stampw, stamph;
	u_int16_t *base, *p, *pbs;
	u_int32_t *pb;

	PX_TRACE("pxSqueege");

	stampw = sp->stampw;
	stamph = sp->stamph;

	sx += pix->drawable.x;
	sy += pix->drawable.y;

#ifdef PX_DEBUG
	if (sx < 0 || sy < 0)
		FatalError("pxSqueege: bad co-ords (sx/sy)\n");
	if (sx + (fx2 - fx1) > pix->drawable.width)
		FatalError("pxSqueege: bad co-ords (width)\n");
	if (sy + (fy2 - fy1) > pix->drawable.height)
		FatalError("pxSqueege: bad co-ords (height)\n");
#endif

	stride = pix->devKind;
	base = (u_int16_t *)((u_int8_t *)pix->devPrivate.ptr +
	    ((sx >> 3) & ~1) + (sy * stride));

	for (xrot = sx & 15; fx1 < fx2; xrot = 0) {
		tw = min(16 - xrot, fx2 - fx1);
		x1 = fx1;
		x2 = fx1 + tw;
		fx1 += tw;

		p = base++;
		y = fy1;
		rh = fy2 - y;

		while (rh > 0) {
			th = min(rh, 16);
			lw = (th << 2) - 1;
			psy = (y << 3) + lw;

			pb = pxPacketAddPrim(sp, pxp);
			pb[8] = XYMASKADDR(stampw, stamph, x1, y, xrot, 0);
			pb[9] = (x1 << 19) | psy;
			pb[10] = (x2 << 19) | psy;
			pb[11] = lw;

			y += th;
			rh -= th;

			for (pbs = (u_int16_t *)pb; th-- != 0; pbs++) {
				*pbs = *p;
				p = (u_int16_t *)((u_int8_t *)p + stride);
			}
		}
	}
}

void
pxSqueege16(pxScreenPrivPtr sp, PixmapPtr pix, DrawablePtr pDst,
	    pxPacketPtr pxp, int fx1, int fy1, int fx2, int fy2,
	    int sx, int sy)
{
	int stride, xrot, rh, psy, lw, tw, stampw, stamph, ya;
	u_int16_t *p, *pbs, *pbsmax, *base;
	u_int32_t *pb;

	PX_TRACE("pxSqueege16");

	stampw = sp->stampw;
	stamph = sp->stamph;

	sx += pix->drawable.x;
	sy += pix->drawable.y;

#ifdef PX_DEBUG
	if (sx < 0 || sy < 0)
		FatalError("pxSqueege16: bad co-ords (sx/sy)\n");
	if (sx + (fx2 - fx1) > pix->drawable.width)
		FatalError("pxSqueege16: bad co-ords (width)\n");
	if (sy + (fy2 - fy1) > pix->drawable.height)
		FatalError("pxSqueege16: bad co-ords (height)\n");
#endif

	stride = pix->devKind;
	base = (u_int16_t *)((u_int8_t *)pix->devPrivate.ptr +
	    ((sx >> 3) & ~1) + (sy * stride));

	rh = fy2 - fy1;
	lw = (rh << 2) - 1;
	psy = (fy1 << 3) + lw;
	ya = YMASKADDR(stamph, fy1, 0);

	for (xrot = sx & 15; fx1 < fx2; xrot = 0, fx1 += tw) {
		pb = pxPacketAddPrim(sp, pxp);
		pbs = (u_int16_t *)pb;
		pbsmax = pbs + rh;

		for (p = base++; pbs < pbsmax; pbs++) {
			*pbs = *p;
			p = (u_int16_t *)((u_int8_t *)p + stride);
		}

		tw = min(16 - xrot, fx2 - fx1);
		pb[8] = ya | (XMASKADDR(stampw, fx1, xrot) << 16);
		pb[9] = (fx1 << 19) | psy;
		pb[10] = ((fx1 + tw) << 19) | psy;
		pb[11] = lw;
	}
}
