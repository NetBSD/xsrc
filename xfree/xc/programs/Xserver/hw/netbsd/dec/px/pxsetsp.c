/*	$NetBSD: pxsetsp.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

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

/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "px.h"

#include "regionstr.h"
#include "pixmapstr.h"
#include "cfb.h"

void
pxSetScanline(pxScreenPrivPtr sp, int y, int xOrigin, int xStart, int xEnd,
	      int *psrc, int umet, unsigned long planemask)
{
	int w, off;
	u_int32_t *pb, *p;
	u_int8_t *dp;
	pxPacket pxp;
	pxImgBufPtr ib;

	PX_TRACE("pxSetScanline");

	ib = pxAllocImgBuf(sp);
	w = xEnd - xStart;
	off = (xStart - xOrigin) * (sp->bpp >> 3);	
	(*sp->expandBuf)(ib->ptr, (caddr_t)psrc + off, w);

	pb = pxPacketStart(sp, &pxp, 4, 4);

	pb[0] = STAMP_CMD_WRITESPANS;
	pb[1] = (1 << 24) | planemask;
	pb[2] = 0;
	pb[3] = umet | STAMP_SPAN;
	pb[4] = ib->paddr;
	pb[5] = w << 3;
	pb[6] = (xStart << 19) | (y << 3);
	pb[7] = 0;

	pxAssociateImgBuf(sp, ib, &pxp);
	pxPacketFlush(sp, &pxp);
}

void
pxSetScanlineRaw(pxScreenPrivPtr sp, int x, int y, int w, int umet,
		 unsigned long planemask, pxImgBufPtr ib)
{
	u_int32_t *pb;
	pxPacket pxp;

	PX_TRACE("pxSetScanlineRaw");

	pb = pxPacketStart(sp, &pxp, 4, 4);

	pb[0] = STAMP_CMD_WRITESPANS;
	pb[1] = (1 << 24) | planemask;
	pb[2] = 0;
	pb[3] = umet | STAMP_SPAN;
	pb[4] = ib->paddr;
	pb[5] = w << 3;
	pb[6] = (x << 19) | (y << 3);
	pb[7] = 0;

	pxAssociateImgBuf(sp, ib, &pxp);
	pxPacketFlush(sp, &pxp);
}

/* 
 * SetSpans -- for each span copy pwidth[i] pels from psrc to pDrawable at
 * ppt[i] using the raster op from the GC.  If fSorted is TRUE, the
 * scanlines are in increasing Y order.  Source bit lines are server
 * scanline padded so that they always begin on a word boundary.
 */ 
void
pxSetSpans(DrawablePtr pDrawable, GCPtr pGC, char *charpsrc, DDXPointPtr ppt,
	   int *pwidth, int nspans, int fSorted)
{
	pxScreenPrivPtr sp;
	pxPrivGCPtr gcPriv;
	BoxPtr pbox, pboxLast, pboxTest;
	DDXPointPtr pptLast;
	RegionPtr prgnDst;
	int xStart, xEnd, yMax;
	unsigned int *psrc;

	PX_TRACE("pxSetSpans");

	psrc = (unsigned int *)charpsrc;
	prgnDst = cfbGetCompositeClip(pGC);
	gcPriv = pxGetGCPrivate(pGC);
	sp = gcPriv->sp;

	pptLast = ppt + nspans;
	yMax = pDrawable->y + pDrawable->height;

	pbox = REGION_RECTS(prgnDst);
	pboxLast = pbox + REGION_NUM_RECTS(prgnDst);

	if (fSorted) {
		/*
		 * Scan lines sorted in ascending order. Because they are
		 * sorted, we don't have to check each scanline against each
		 * clip box.  We can be sure that this scanline only has to
		 * be clipped to boxes at or after the beginning of this
		 * y-band
		 */
		pboxTest = pbox;
		while (ppt < pptLast) {
			pbox = pboxTest;
			if (ppt->y >= yMax)
				break;

			while (pbox < pboxLast) {
				if (pbox->y1 > ppt->y) {
					/* Scanline is before clip box. */
					break;
				}
				if (pbox->y2 <= ppt->y) {
					/* Clip box is before scanline. */
					pboxTest = ++pbox;
					continue;
				}
				if (pbox->x1 > ppt->x + *pwidth) {
					/* Clip box to right of scanline. */
					break;
				}
				if (pbox->x2 <= ppt->x) {
					/* Scanline to right of clip box. */
					pbox++;
					continue;
				}

				/*
				 * At least some of the scanline is in the
				 * current clip box.
				 */
				xStart = max(pbox->x1, ppt->x);
				xEnd = min(ppt->x + *pwidth, pbox->x2);
				pxSetScanline(sp, ppt->y, ppt->x, xStart,
				    xEnd, psrc, gcPriv->umet, gcPriv->pmask);

				if (ppt->x + *pwidth <= pbox->x2) {
					/* End of the line, as it were. */
					break;
				}

				pbox++;
			}
	
			/*
			 * We've tried this line against every box; it must
			 * be outside them all.  Move on to the next point.
			 */
			ppt++;
			psrc += PixmapWidthInPadUnits(*pwidth,
			    pDrawable->depth);
			pwidth++;
		}
		return;
	}

	/*
	 * Scan lines not sorted.  We must clip each line against
	 * all the boxes.
	 */
	while (ppt < pptLast) {
		if (ppt->y >= 0 && ppt->y < yMax) {
			pbox = REGION_RECTS(prgnDst);
			for (; pbox< pboxLast; pbox++) {
				if (pbox->y1 > ppt->y) {
					/*
					 * Rest of clip region is
					 * above this scanline, skip
					 * it.
					 */
					break;
				}
				if (pbox->y2 <= ppt->y) {
					/*
					 * Clip box is below
					 * scanline.
					 */
					pbox++;
					break;
				}
				if (pbox->x1 <= ppt->x + *pwidth &&
				    pbox->x2 > ppt->x) {
					xStart = max(pbox->x1, ppt->x);
					xEnd = min(pbox->x2, ppt->x + *pwidth);
					pxSetScanline(sp, ppt->y, ppt->x,
					    xStart, xEnd, psrc, gcPriv->umet,
					    gcPriv->pmask);
				}
			}
		}

		psrc += PixmapWidthInPadUnits(*pwidth, pDrawable->depth);
		ppt++;
		pwidth++;
	}
}
