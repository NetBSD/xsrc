/*	$NetBSD: pxpushpxl.c,v 1.3 2008/04/28 20:57:37 martin Exp $	*/

/*-
 * Copyright (c) 2001, 2002 The NetBSD Foundation, Inc.
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

#include "px.h"

#include "pixmapstr.h"
#include "scrnintstr.h"
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

		pb = pxPacketStart(sp, &pxp, 4, 13);
		pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT |
		    STAMP_LW_PERPRIMATIVE | STAMP_XY_PERPRIMATIVE;
		pb[1] = gcPriv->pmask;
		pb[2] = 0;
		pb[3] = gcPriv->umet | STAMP_WE_XYMASK;

		pBox = REGION_RECTS(&rgnDst);
		pMaxBox = pBox + REGION_NUM_RECTS(&rgnDst);

		if (pBox->y2 - pBox->y1 <= 16) {
			for (; pBox < pMaxBox; pBox++)
				pxSqueege(sp, &pxp,
				    (u_int8_t *)pBitMap->devPrivate.ptr,
				    pBitMap->devKind,
				    pBox->x1, pBox->y1,
				    pBox->x2, pBox->y2,
				    pBox->x1 - xOrg + pBitMap->drawable.x,
				    pBox->y1 - yOrg + pBitMap->drawable.y,
				    gcPriv->fgFill);
		} else {
			for (; pBox < pMaxBox; pBox++)
				pxSqueege(sp, &pxp,
				    (u_int8_t *)pBitMap->devPrivate.ptr,
				    pBitMap->devKind,
				    pBox->x1, pBox->y1,
				    pBox->x2, pBox->y2,
				    pBox->x1 - xOrg + pBitMap->drawable.x,
				    pBox->y1 - yOrg + pBitMap->drawable.y,
				    gcPriv->fgFill);
		}

		pxPacketFlush(sp, &pxp);
	}

	REGION_UNINIT(pGC->pScreen, &rgnDst);
}

void
pxSqueege(pxScreenPrivPtr sp, pxPacketPtr pxp, u_int8_t *bits, int stride,
	  int fx1, int fy1, int fx2, int fy2,
	  int sx, int sy, int fgFill)
{
	int xrot, x1, x2, y, th, rh, psy, lw, tw, stampw, stamphm, xa, latch;
	u_int16_t *pbs, *p, *base;
	u_int32_t *pb, tbuf[8];

	PX_TRACE("pxSqueege");

	stampw = sp->stampw;
	stamphm = sp->stamphm;
	base = (u_int16_t *)(bits + ((sx >> 3) & ~1) + (sy * stride));

	for (xrot = sx & 15; fx1 < fx2; xrot = 0) {
		tw = min(16 - xrot, fx2 - fx1);
		x1 = fx1;
		x2 = fx1 + tw;
		fx1 += tw;

		p = base++;
		y = fy1;
		rh = fy2 - y;
		xa = XMASKADDR(stampw, x1, xrot) << 16;

		while (rh > 0) {
			th = min(rh, 16);
			lw = (th << 2) - 1;
			psy = (y << 3) + lw;

			pb = pxPacketAddPrim(sp, pxp);
			pb[8] = YMASKADDR(stamphm, y, 0) | xa;
			pb[9] = (x1 << 19) | psy;
			pb[10] = (x2 << 19) | psy;
			pb[11] = lw;
			pb[12] = fgFill;

			y += th;
			rh -= th;

			for (pbs = (u_int16_t *)tbuf; th-- != 0; pbs++) {
				*pbs = *p;
				p = (u_int16_t *)((u_int8_t *)p + stride);
			}

			/*
			 * Avoid non-32-bit writes across the TURBOchannel
			 * bus.  They're slow, and cause corruption on mips.
			 */
			pb[0] = tbuf[0];
			pb[1] = tbuf[1];
			pb[2] = tbuf[2];
			pb[3] = tbuf[3];
			pb[4] = tbuf[4];
			pb[5] = tbuf[5];
			pb[6] = tbuf[6];
			pb[7] = tbuf[7];
		}
	}
}

void
pxSqueege16(pxScreenPrivPtr sp, pxPacketPtr pxp, u_int8_t *bits, int stride,
	  int fx1, int fy1, int fx2, int fy2,
	  int sx, int sy, int fgFill)
{
	int xrot, rh, psy, lw, tw, stampw, stamphm, ya;
	u_int16_t *p, *pbs, *pbsmax, *base;
	u_int32_t *pb, tbuf[8];

	PX_TRACE("pxSqueege16");

	stampw = sp->stampw;
	stamphm = sp->stamphm;
	base = (u_int16_t *)(bits + ((sx >> 3) & ~1) + (sy * stride));

	rh = fy2 - fy1;
	lw = (rh << 2) - 1;
	psy = (fy1 << 3) + lw;
	ya = YMASKADDR(stamphm, fy1, 0);

	for (xrot = sx & 15; fx1 < fx2; xrot = 0, fx1 += tw) {
		pb = pxPacketAddPrim(sp, pxp);
		pbs = (u_int16_t *)tbuf;
		pbsmax = pbs + rh;

		for (p = base++; pbs < pbsmax; pbs++) {
			*pbs = *p;
			p = (u_int16_t *)((u_int8_t *)p + stride);
		}

		/*
		 * Avoid non-32-bit writes across the TURBOchannel bus. 
		 * They're slow, and cause corruption on mips.
		 */
		pb[0] = tbuf[0];
		pb[1] = tbuf[1];
		pb[2] = tbuf[2];
		pb[3] = tbuf[3];
		pb[4] = tbuf[4];
		pb[5] = tbuf[5];
		pb[6] = tbuf[6];
		pb[7] = tbuf[7];

		tw = min(16 - xrot, fx2 - fx1);
		pb[8] = ya | (XMASKADDR(stampw, fx1, xrot) << 16);
		pb[9] = (fx1 << 19) | psy;
		pb[10] = ((fx1 + tw) << 19) | psy;
		pb[11] = lw;
		pb[12] = fgFill;
	}
}
