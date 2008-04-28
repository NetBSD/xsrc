/*	$NetBSD: pxbitblt.c,v 1.3 2008/04/28 20:57:37 martin Exp $	*/

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
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Keith Packard

*/

#include "px.h"

#include "scrnintstr.h"
#include "pixmapstr.h"
#include "regionstr.h"

#include "cfb.h"
#include "mi.h"

/* XXX */
#ifdef cfbCopyArea
#undef cfbCopyArea
#endif

extern RegionPtr cfbCopyArea(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr/*pGC*/,
    int /*srcx*/,
    int /*srcy*/,
    int /*width*/,
    int /*height*/,
    int /*dstx*/,
    int /*dsty*/
#endif
);

extern RegionPtr cfb32CopyArea(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr/*pGC*/,
    int /*srcx*/,
    int /*srcy*/,
    int /*width*/,
    int /*height*/,
    int /*dstx*/,
    int /*dsty*/
#endif
);

RegionPtr
pxCopyArea(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GCPtr pGC,
	   int srcx, int srcy, int width, int height, int dstx, int dsty)
{

	if (pDstDrawable->type == DRAWABLE_WINDOW &&
	    (pSrcDrawable->type == DRAWABLE_WINDOW ||
	    pSrcDrawable->type == DRAWABLE_PIXMAP))
		return (cfbBitBlt(pSrcDrawable, pDstDrawable, pGC, srcx,
		    srcy, width, height, dstx, dsty, pxDoBitblt, 0L));

	if (pDstDrawable->type == DRAWABLE_WINDOW ||
	    pSrcDrawable->type == DRAWABLE_WINDOW)
		return (miCopyArea(pSrcDrawable, pDstDrawable, pGC,
		    srcx, srcy, width, height, dstx, dsty));

	if (pSrcDrawable->bitsPerPixel == 8)
		return (cfbCopyArea(pSrcDrawable, pDstDrawable, pGC, srcx,
		    srcy, width, height, dstx, dsty));

	return (cfb32CopyArea(pSrcDrawable, pDstDrawable, pGC, srcx, srcy,
	    width, height, dstx, dsty));
}

void
pxDoBitblt(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, int alu,
	   RegionPtr prgnDst, DDXPointPtr pptSrc, unsigned long pmask)
{
	BoxPtr pbox, pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
	DDXPointPtr pptTmp, pptNew1, pptNew2;
	pxScreenPrivPtr sp;
	int w, h, ydir, nbox, src, dst, umet, yStrideSrc, srcDir, careful;
	int xStrideSrc;
	u_int8_t *psrc, *psrcBase;
	u_int32_t *pb;
	PixmapPtr pPix;
	pxPacket pxp;

	PX_TRACE("pxDoBitblt");

	sp = pSrcDrawable->pScreen->devPrivates[pxScreenPrivateIndex].ptr;

	pmask &= 0x00ffffff;
	pbox = REGION_RECTS(prgnDst);
	nbox = REGION_NUM_RECTS(prgnDst);

	pboxNew1 = NULL;
	pptNew1 = NULL;
	pboxNew2 = NULL;
	pptNew2 = NULL;

	careful = (pSrcDrawable == pDstDrawable ||
	    (pSrcDrawable->type == DRAWABLE_WINDOW &&
	    pDstDrawable->type == DRAWABLE_WINDOW));

	if (careful && pptSrc->y < pbox->y1) {
		/*
		 * Walk source bottom to top.
		 */
		ydir = -1;

		if (nbox > 1) {
			/*
			 * Keep ordering in each band, reverse order of
			 * bands.
			 */
			pboxNew1 = (BoxPtr)
			    ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
			if (!pboxNew1)
				return;
			pptNew1 = (DDXPointPtr)
			    ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
			if (!pptNew1) {
				DEALLOCATE_LOCAL(pboxNew1);
				return;
			}

			pboxBase = pboxNext = pbox + nbox - 1;
			while (pboxBase >= pbox) {
				while (pboxNext >= pbox &&
				   pboxBase->y1 == pboxNext->y1)
					pboxNext--;
				pboxTmp = pboxNext + 1;
				pptTmp = pptSrc + (pboxTmp - pbox);
				while (pboxTmp <= pboxBase) {
					*pboxNew1++ = *pboxTmp++;
					*pptNew1++ = *pptTmp++;
				}
				pboxBase = pboxNext;
			}

			pboxNew1 -= nbox;
			pbox = pboxNew1;
			pptNew1 -= nbox;
			pptSrc = pptNew1;
		}
	} else {
		/*
		 * Walk source top to bottom.
		 */
		ydir = 1;
	}

	if (careful && pptSrc->x < pbox->x1) {
		/*
		 * Walk source right to left.
		 */
		if (nbox > 1) {
			/*
			 * Reverse order of rects in each band.
			 */
			pboxNew2 =
			    (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
			pptNew2 =
			    (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
			if (!pboxNew2 || !pptNew2) {
				if (pptNew2)
					DEALLOCATE_LOCAL(pptNew2);
				if (pboxNew2)
					DEALLOCATE_LOCAL(pboxNew2);
				if (pboxNew1) {
					DEALLOCATE_LOCAL(pptNew1);
					DEALLOCATE_LOCAL(pboxNew1);
				}
				return;
			}

			pboxBase = pboxNext = pbox;
			while (pboxBase < pbox+nbox) {
				while ((pboxNext < pbox+nbox) &&
				   (pboxNext->y1 == pboxBase->y1))
				pboxNext++;
				pboxTmp = pboxNext;
				pptTmp = pptSrc + (pboxTmp - pbox);
				while (pboxTmp != pboxBase) {
					*pboxNew2++ = *--pboxTmp;
					*pptNew2++ = *--pptTmp;
				}
				pboxBase = pboxNext;
			}

			pboxNew2 -= nbox;
			pbox = pboxNew2;
			pptNew2 -= nbox;
			pptSrc = pptNew2;
		}
	}

	if (pSrcDrawable->type == DRAWABLE_WINDOW) {
		umet = pxRopTable[alu] | STAMP_SPAN;
		ydir *= 8;

		/*
		 * The stamp goes bananas if you mix span widths, or don't
		 * properly specify COPYSPAN_ALIGNED.  I'm not sure which it
		 * is, but to make it happy we emit a seperate packet for
		 * each box.
		 */
		while (nbox--) {
			pb = pxPacketStart(sp, &pxp, 5, 3);
			pb[0] = STAMP_CMD_COPYSPANS | STAMP_LW_PERPACKET;
			pb[1] = pmask;
			pb[2] = 0x0;
			pb[3] = umet;
			pb[4] = 1;

			w = pbox->x2 - pbox->x1;
			h = pbox->y2 - pbox->y1;

			/*
			 * XXX This is necessary when copying right -> left,
			 * as the PixelStamp seems to interpret the length
			 * incorrectly; most probably something to do with
			 * the sub-pixel coordinate system.  More testing
			 * required.
			 */
			if (pptSrc->x <= pbox->x1)
				w--;

			if (pptSrc->x == pbox->x1)
				pb[3] |= STAMP_COPYSPAN_ALIGNED;
#ifdef notdef
			/*
			 * XXX This also makes the PixelStamp go bananas,
			 * but works just fine in the kernel.  Why?
			 */
			else if (pptSrc->y == pbox->y1 &&
			    pptSrc->x <= pbox->x1 && pptSrc->x + w > pbox->x1)
				pb[3] |= STAMP_HALF_BUFF;
#endif

			if (ydir < 0) {
				src = (pptSrc->x << 19) |
				    ((pptSrc->y + h - 1) << 3);
				dst = (pbox->x1 << 19) |
				    ((pbox->y1 + h - 1) << 3);
			} else {
				src = (pptSrc->x << 19) | (pptSrc->y << 3);
				dst = (pbox->x1 << 19) | (pbox->y1 << 3);
			}

			for (w <<= 3; h-- != 0; src += ydir, dst += ydir) {
				pb = pxPacketAddPrim(sp, &pxp);
				pb[0] = w;
				pb[1] = src;
				pb[2] = dst;
			}

			pxPacketFlush(sp, &pxp);

			pbox++;
			pptSrc++;
		}
	} else /* if (pSrcDrawable->type == DRAWABLE_PIXMAP) */ {
		umet = pxRopTable[alu];
		pPix = (PixmapPtr)pSrcDrawable;
		xStrideSrc = sp->bpp >> 3;
		yStrideSrc = pPix->devKind;
		psrcBase = (u_int8_t *)pPix->devPrivate.ptr;
		srcDir = ydir * yStrideSrc;

		while (nbox--) {
			h = pbox->y2 - pbox->y1;

			if (ydir < 0) {
				src = pptSrc->y + h - 1;
				dst = pbox->y2 - 1;
			} else {
				src = pptSrc->y;
				dst = pbox->y1;
			}
			psrc = psrcBase + pptSrc->x * xStrideSrc +
			    src * yStrideSrc;

			for (; h != 0; h--) {
				pxSetScanline(sp, dst, pbox->x1, pbox->x1,
				    pbox->x2, (u_int32_t *)psrc, umet, pmask);
				dst += ydir;
				psrc += srcDir;
			}

			pbox++;
			pptSrc++;
		}
	}

	if (pboxNew1) {
		DEALLOCATE_LOCAL(pptNew1);
		DEALLOCATE_LOCAL(pboxNew1);
	}

	if (pboxNew2) {
		DEALLOCATE_LOCAL(pptNew2);
		DEALLOCATE_LOCAL(pboxNew2);
	}
}
