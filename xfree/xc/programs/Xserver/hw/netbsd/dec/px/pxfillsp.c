/*	$NetBSD: pxfillsp.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

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

#include "px.h"

#include "pixmapstr.h"
#include "scrnintstr.h"

#include "cfb.h"
#include "mi.h"
#include "mispans.h"

void
pxFillSpans(DrawablePtr pDrawable, GCPtr pGC, int n, DDXPointPtr ppt,
	    int *pwidth, int fSorted)
{
	pxPrivGCPtr gcPriv;
	int *pwidthFree, nTmp;
	DDXPointPtr pptFree;
	RegionPtr clip;
	BoxPtr pOneBox;

	PX_TRACE("pxFillSpans");

	clip = cfbGetCompositeClip(pGC);

	switch (REGION_NUM_RECTS(clip)) {
	case 0:
		return;

	case 1:
		pOneBox = &clip->extents;
		break;

	default:
		nTmp = n * miFindMaxBand(clip);

		pwidthFree = (int *)ALLOCATE_LOCAL(nTmp * sizeof(int));
		pptFree =
		    (DDXPointRec *)ALLOCATE_LOCAL(nTmp * sizeof(DDXPointRec));
		if (!pptFree || !pwidthFree) {
			if (pptFree)
				DEALLOCATE_LOCAL(pptFree);
			if (pwidthFree)
				DEALLOCATE_LOCAL(pwidthFree);
			return;
		}
		n = miClipSpans(clip, ppt, pwidth, n, pptFree, pwidthFree,
		    fSorted);
		pwidth = pwidthFree;
		ppt = pptFree;
		pOneBox = NULL;
		break;
	}

	if (n != 0) {
		gcPriv = pxGetGCPrivate(pGC);
		(*gcPriv->doFillSpans)(pDrawable, pGC, gcPriv, pOneBox, n,
		    ppt, pwidth);
	}

	if (pOneBox == NULL) {
		DEALLOCATE_LOCAL(pptFree);
		DEALLOCATE_LOCAL(pwidthFree);
	}
}

void
pxDoFillSpans(DrawablePtr pDrawable, GCPtr pGC, pxPrivGCPtr gcPriv,
	      BoxPtr pOneBox, int n, DDXPointPtr ppt, int *pwidth)
{
	pxScreenPrivPtr sp;
	DDXPointPtr pptmax;
	int psy, x, y, w, h;
	u_int32_t *pb;
	pxPacket pxp;

	PX_TRACE("pxDoFillSpans");

	sp = gcPriv->sp;

	pb = pxPacketStart(sp, &pxp, 5, 3);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_CONST | STAMP_LW_PERPRIMATIVE;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet;
	pb[4] = gcPriv->fgFill;

	for (pptmax = ppt + n; ppt < pptmax; ppt++, pwidth++) {
		w = pwidth[0];
		x = ppt[0].x;
		y = ppt[0].y;

		if (pOneBox != NULL) {
			if (y < pOneBox->y1 || y >= pOneBox->y2)
				continue;
			if (x < pOneBox->x1) {
				w -= (pOneBox->x1 - x);
				if (w <= 0)
					continue;
				x = pOneBox->x1;
			}
			if (x + w > pOneBox->x2) {
				if (x >= pOneBox->x2)
					continue;
				w = pOneBox->x2 - x;
			}
		}

		for (h = 1; h < 17; h++, ppt++, pwidth++) {
			if (ppt + 1 == pptmax)
				break;
			if (ppt[1].x != x || pwidth[1] != w ||
			    ppt[1].y != y + 1)
				break;
		}

		h = (h << 2) - 1;
		psy = (y << 3) + h;

		pb = pxPacketAddPrim(sp, &pxp);
		pb[0] = (x << 19) | psy;
		pb[1] = ((x + w) << 19) | psy;
		pb[2] = h;
	}

	pxPacketFlush(sp, &pxp);
}

void
pxDoFillSpansS(DrawablePtr pDrawable, GCPtr pGC, pxPrivGCPtr gcPriv,
	       BoxPtr pOneBox, int n, DDXPointPtr ppt, int *pwidth)
{
	int v1, v2, psy, x, y, w, xya, stampw, stamphm, xorg, yorg, h;
	DDXPointPtr pptmax;
	pxScreenPrivPtr sp;
	u_int32_t *pb, *mask;
	pxPacket pxp;

	PX_TRACE("pxDoFillSpansS");

	mask = (u_int32_t *)gcPriv->mask.data;
	sp = gcPriv->sp;
	stampw = sp->stampw;
	stamphm = sp->stamphm;

	pb = pxPacketStart(sp, &pxp, 4, 13);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT | STAMP_LW_PERPRIMATIVE |
	    STAMP_XY_PERPRIMATIVE;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet | STAMP_WE_XYMASK;

	for (pptmax = ppt + n; ppt < pptmax; ppt++, pwidth++) {
		w = pwidth[0];
		x = ppt[0].x;
		y = ppt[0].y;

		if (pOneBox != NULL) {
			if (y < pOneBox->y1 || y >= pOneBox->y2)
				continue;
			if (x < pOneBox->x1) {
				w -= (pOneBox->x1 - x);
				if (w <= 0)
					continue;
				x = pOneBox->x1;
			}
			if (x + w > pOneBox->x2) {
				if (x >= pOneBox->x2)
					continue;
				w = pOneBox->x2 - x;
			}
		}

		for (h = 1; h < 17; h++, ppt++, pwidth++) {
			if (ppt + 1 == pptmax)
				break;
			if (ppt[1].x != x || pwidth[1] != w ||
			    ppt[1].y != y + 1)
				break;
		}

		xorg = (x - pGC->patOrg.x - pDrawable->x) & 15;
		yorg = (y - pGC->patOrg.y - pDrawable->y) & 15;

		xya = XYMASKADDR(stampw, stamphm, x, y, xorg, yorg);
		h = (h << 2) - 1;
		psy = (y << 3) + h;
		v1 = (x << 19) | psy;
		v2 = ((x + w) << 19) | psy;

		if (gcPriv->fillStyle != FillStippled) {
			pb = pxPacketAddPrim(sp, &pxp);
			pb[0] = ~mask[0];
			pb[1] = ~mask[1];
			pb[2] = ~mask[2];
			pb[3] = ~mask[3];
			pb[4] = ~mask[4];
			pb[5] = ~mask[5];
			pb[6] = ~mask[6];
			pb[7] = ~mask[7];
			pb[8] = xya;
			pb[9] = v1;
			pb[10] = v2;
			pb[11] = h;
			pb[12] = gcPriv->bgPixel;
		}

		pb = pxPacketAddPrim(sp, &pxp);
		pb[0] = mask[0];
		pb[1] = mask[1];
		pb[2] = mask[2];
		pb[3] = mask[3];
		pb[4] = mask[4];
		pb[5] = mask[5];
		pb[6] = mask[6];
		pb[7] = mask[7];
		pb[8] = xya;
		pb[9] = v1;
		pb[10] = v2;
		pb[11] = h;
		pb[12] = gcPriv->fgFill;
	}

	pxPacketFlush(sp, &pxp);
}

void
pxDoFillSpansT(DrawablePtr pDrawable, GCPtr pGC, pxPrivGCPtr gcPriv,
	       BoxPtr pOneBox, int n, DDXPointPtr ppt, int *pwidth)
{
	pxScreenPrivPtr sp;
	DDXPointPtr pptmax;
	int x, y, w, xorg, yorg, tbp, tw;
	pxImgBufPtr ib;
	PixmapPtr pix;
	void *p;

	PX_TRACE("pxDoFillSpansT");

	sp = gcPriv->sp;
	pix = pGC->tile.pixmap;

	for (pptmax = ppt + n; ppt < pptmax; ppt++) {
		w = *pwidth++;
		x = ppt->x;
		y = ppt->y;

		if (pOneBox != NULL) {
			if (y < pOneBox->y1 || y >= pOneBox->y2)
				continue;
			if (x < pOneBox->x1) {
				w -= (pOneBox->x1 - x);
				if (w <= 0)
					continue;
				x = pOneBox->x1;
			}
			if (x + w > pOneBox->x2) {
				if (x >= pOneBox->x2)
					continue;
				w = pOneBox->x2 - x;
			}
		}

		ib = pxAllocImgBuf(sp);

		xorg = x - pGC->patOrg.x - pDrawable->x;
		yorg = y - pGC->patOrg.y - pDrawable->y;

		tbp = pix->drawable.bitsPerPixel >> 3;
		tw = pix->drawable.width;
		p = (caddr_t)pix->devPrivate.ptr +
		    (yorg % pix->drawable.height + pix->drawable.y) * pix->devKind +
		    (pix->drawable.x * tbp);

		(*sp->tileBuf)(ib->ptr, p, w, tw, xorg % tw);
		pxSetScanlineRaw(sp, x, y, w, gcPriv->umet, gcPriv->pmask, ib);
	}
}

void
pxDoFillSpansUS(DrawablePtr pDrawable, GCPtr pGC, pxPrivGCPtr gcPriv,
	        BoxPtr pOneBox, int n, DDXPointPtr ppt, int *pwidth)
{
	pxScreenPrivPtr sp;
	DDXPointPtr pptmax;
	int x, y, w, xorg, yorg;
	pxImgBufPtr ib;

	PX_TRACE("pxDoFillSpansUS");

	sp = gcPriv->sp;

	for (pptmax = ppt + n; ppt < pptmax; ppt++) {
		w = *pwidth++;
		x = ppt->x;
		y = ppt->y;

		if (pOneBox != NULL) {
			if (y < pOneBox->y1 || y >= pOneBox->y2)
				continue;
			if (x < pOneBox->x1) {
				w -= (pOneBox->x1 - x);
				if (w <= 0)
					continue;
				x = pOneBox->x1;
			}
			if (x + w > pOneBox->x2) {
				if (x >= pOneBox->x2)
					continue;
				w = pOneBox->x2 - x;
			}
		}

		ib = pxAllocImgBuf(sp);

		xorg = x - pGC->patOrg.x - pDrawable->x;
		yorg = y - pGC->patOrg.y - pDrawable->y;

		if (gcPriv->fillStyle == FillStippled) {
			pxGetScanlineRaw(sp, x, y, w, ib);
			pxStippleBuf(pGC->stipple, ib->ptr, w, pGC->fillStyle,
			    gcPriv->fgFill, gcPriv->bgPixel, xorg, yorg);
		} else {
			pxStippleBufOpaque(pGC->stipple, ib->ptr, w,
			    pGC->fillStyle, gcPriv->fgFill, gcPriv->bgPixel,
			    xorg, yorg);
		}
		pxSetScanlineRaw(sp, x, y, w, gcPriv->umet, gcPriv->pmask,
		    ib);
	}
}
