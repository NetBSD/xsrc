/*	$NetBSD: pxglyph.c,v 1.2 2008/04/28 20:57:37 martin Exp $	*/

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

/*
 * Acceleration for the Leo (ZX) framebuffer - Glyph rops.
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
/* from XFree86: xc/programs/Xserver/hw/xfree86/drivers/sunleo/leo_glyph.c,v 1.1 2000/05/18 23:21:40 dawes Exp */

#include "px.h"

#include "pixmapstr.h"
#include "scrnintstr.h"
#include "fontstruct.h"
#include "dixfontstr.h"

#include "mi.h"
#include "cfb.h"

#define	DOFG(v1, v2, lw, xya, s, c)			\
	do {						\
		pb[0] = ((s)[1] << 16) | (s)[0];	\
		pb[1] = ((s)[3] << 16) | (s)[2];	\
		pb[2] = ((s)[5] << 16) | (s)[4];	\
		pb[3] = ((s)[7] << 16) | (s)[6];	\
		pb[4] = ((s)[9] << 16) | (s)[8];	\
		pb[5] = ((s)[11] << 16) | (s)[10];	\
		pb[6] = ((s)[13] << 16) | (s)[12];	\
		pb[7] = ((s)[15] << 16) | (s)[14];	\
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

void	pxSolidBox(pxScreenPrivPtr, pxPacketPtr, BoxPtr, int);

#ifdef _IMAGEGLYPH
void
pxImageGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		unsigned int nglyph, CharInfoPtr *ppci,
		pointer pGlyphBase)
#else
void
pxPolyGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		unsigned int nglyph, CharInfoPtr *ppci,
		pointer pGlyphBase)
#endif
{
	pxScreenPrivPtr sp;
	pxPrivGCPtr gcPriv;
	RegionPtr clip;
	int w, h, ex, bg, fg, x0, y0;
	BoxRec box;
	FontPtr pfont;
	u_int32_t *pb, *fb;
	pxPacket pxp;
	int v1, v2, lw, xya, psy;
	int stampw, stamphm;
	CharInfoPtr pci;

#ifdef _IMAGEGLYPH
	PX_TRACE("pxImageGlyphBlt");
#else
	PX_TRACE("pxPolyGlyphBlt");
#endif

	clip = cfbGetCompositeClip(pGC);
	pfont = pGC->font;

	/*
	 * Compte an approximate (but covering) bounding box.
	 */
	box.x1 = 0;
	if (ppci[0]->metrics.leftSideBearing < 0)
		box.x1 = ppci[0]->metrics.leftSideBearing;
	h = nglyph - 1;
	w = ppci[h]->metrics.rightSideBearing;
	while (--h >= 0)
		w += ppci[h]->metrics.characterWidth;
	box.x2 = w;
	box.y1 = -FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = FONTMAXBOUNDS(pGC->font, descent);

	box.x1 += pDrawable->x + x;
	box.x2 += pDrawable->x + x;
	box.y1 += pDrawable->y + y;
	box.y2 += pDrawable->y + y;

	switch (RECT_IN_REGION(pGC->pScreen, clip, &box)) {
	case rgnPART:
#ifndef notyet
#  ifdef _IMAGEGLYPH
		miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci,
		    pGlyphBase);
#  else
		miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci,
		    pGlyphBase);
#  endif
#else
#  ifdef _IMAGEGLYPH
		if (pGC->fillStyle == FillSolid)
			pxSlowImageGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
		else
			miImageGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
#  else
		if (pGC->fillStyle == FillSolid)
			pxSlowPolyGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
		else
			miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
#  endif
#endif
		return;
	case rgnOUT:
		return;
	default:
		break;
	}

	gcPriv = pxGetGCPrivate(pGC);
	sp = gcPriv->sp;

	pb = pxPacketStart(sp, &pxp, 6, 13);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT | STAMP_LW_PERPRIMATIVE |
	    STAMP_XY_PERPRIMATIVE | STAMP_CLIPRECT;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet | STAMP_WE_XYMASK | STAMP_WE_CLIPRECT;
	pb[4] = 0x7c00400;
	pb[5] = 0x7ffffff;

	stampw = sp->stampw;
	stamphm = sp->stamphm;
	x += pDrawable->x;
	y += pDrawable->y;
#ifdef _IMAGEGLYPH
	fg = gcPriv->fgPixel;
#else
	fg = gcPriv->fgFill;
#endif
	bg = gcPriv->bgPixel;

	while (nglyph--) {
		pci = *ppci++;
		w = GLYPHWIDTHPIXELS(pci);
		h = GLYPHHEIGHTPIXELS(pci);

		if ((w | h) == 0)
			continue;

		fb = (u_int32_t *)pci->bits;
		x0 = x + pci->metrics.leftSideBearing;
		y0 = y - pci->metrics.ascent;
		ex = x0 + w;
		x += pci->metrics.characterWidth;
		lw = (h << 2) - 1;
		psy = (y0 << 3) + lw;
		v1 = (x0 << 19) | psy;
		v2 = (ex << 19) | psy;
		xya = XYMASKADDR(stampw, stamphm, x0, y, 0, 0);
#ifdef _IMAGEGLYPH
		pb = pxPacketAddPrim(sp, &pxp);
		DOBG(v1, v2, lw, xya, bg);
#endif
		pb = pxPacketAddPrim(sp, &pxp);
		DOFG(v1, v2, lw, xya, fb, fg);
	}

	pxPacketFlush(sp, &pxp);
}

#ifdef _IMAGEGLYPH
void
pxImageTEGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		  unsigned int nglyph, CharInfoPtr *ppci,
		  pointer pGlyphBase)
#else
void
pxPolyTEGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		 unsigned int nglyph, CharInfoPtr *ppci,
		 pointer pGlyphBase)
#endif
{
	pxScreenPrivPtr sp;
	pxPrivGCPtr gcPriv;
	RegionPtr clip;
	u_int ex, fg, widthGlyph, heightGlyph;
	BoxRec box;
	FontPtr pfont;
	u_int32_t *pb, *fb;
	pxPacket pxp;
	int v1, v2, lw, xya, psy, stampw, stamphm, ya;

#ifdef _IMAGEGLYPH
	PX_TRACE("pxImageTEGlyphBlt");
#else
	PX_TRACE("pxPolyTEGlyphBlt");
#endif

	pfont = pGC->font;
	widthGlyph = FONTMAXBOUNDS(pfont,characterWidth);
	heightGlyph = FONTASCENT(pfont) + FONTDESCENT(pfont);

	if ((widthGlyph | heightGlyph) == 0)
		return;

	clip = cfbGetCompositeClip(pGC);

	box.x1 = x + pDrawable->x;
	box.x2 = box.x1 + (widthGlyph * nglyph);
	box.y1 = y + pDrawable->y - FONTASCENT(pfont);
	box.y2 = box.y1 + heightGlyph;

	switch (RECT_IN_REGION(pGC->pScreen, clip, &box)) {
	case rgnPART:
#ifndef notyet
#  ifdef _IMAGEGLYPH
		miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci,
		    pGlyphBase);
#  else
		miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci,
		    pGlyphBase);
#  endif
#else
#  ifdef _IMAGEGLYPH
		if (pGC->fillStyle == FillSolid)
			pxSlowImageGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
		else
			miImageGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
#  else
		if (pGC->fillStyle == FillSolid)
			pxSlowPolyGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
		else
			miPolyGlyphBlt(pDrawable, pGC, x, y, nglyph,
			    ppci, pGlyphBase);
#  endif
#endif		
		return;
	case rgnOUT:
		return;
	default:
		break;
	}

	gcPriv = pxGetGCPrivate(pGC);
	sp = gcPriv->sp;

	pb = pxPacketStart(sp, &pxp, 6, 13);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT | STAMP_LW_PERPRIMATIVE |
	    STAMP_XY_PERPRIMATIVE | STAMP_CLIPRECT;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet | STAMP_WE_XYMASK | STAMP_WE_CLIPRECT;
	pb[4] = 0x7c00400;
	pb[5] = 0x7ffffff;

	x += pDrawable->x;
	y = y + pDrawable->y - FONTASCENT(pfont);

	stampw = sp->stampw;
	stamphm = sp->stamphm;
	ya = YMASKADDR(stamphm, y, 0);

#ifdef _IMAGEGLYPH
	ex = x + (widthGlyph * nglyph);
	lw = (heightGlyph << 2) - 1;
	psy = (y << 3) + lw;
	v1 = (x << 19) | psy;
	v2 = (ex << 19) | psy;
	xya = ya | (XMASKADDR(stampw, x, 0) << 16);
	pb = pxPacketAddPrim(sp, &pxp);
	DOBG(v1, v2, lw, xya, gcPriv->bgPixel);
#endif

#ifdef _IMAGEGLYPH
	fg = gcPriv->fgPixel;
#else
	fg = gcPriv->fgFill;
#endif

	while (nglyph--) {
		fb = (u_int32_t *)FONTGLYPHBITS(pglyphBase, *ppci++);
		ex = x + widthGlyph;
		lw = (heightGlyph << 2) - 1;
		psy = (y << 3) + lw;
		v1 = (x << 19) | psy;
		v2 = (ex << 19) | psy;
		xya = ya | (XMASKADDR(stampw, x, 0) << 16);
		pb = pxPacketAddPrim(sp, &pxp);
		DOFG(v1, v2, lw, xya, fb, fg);
		x += widthGlyph;
	}

	pxPacketFlush(sp, &pxp);
}

#ifdef notyet
/*
 * "Slow" glyph routines.  These are used when glyphs are larger than 16x16,
 * or when the glyph is clipped - cases that our basic routines don't
 * handle.  The advantages over using the mi layer are that fewer packets
 * will be used, and that no intermediate copy to a scratch pixmap is
 * performed.
 */
void
#ifdef _IMAGEGLYPH
pxSlowPolyGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		   unsigned int nglyph, CharInfoPtr *ppci,
		   pointer pGlyphBase)
#else
pxSlowImageGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		   unsigned int nglyph, CharInfoPtr *ppci,
		   pointer pGlyphBase)
#endif
{
	pxScreenPrivPtr sp;
	FontPtr pfont;
	pxPacket pxp;
	CharInfoPtr pci;
	RegionRec rgnDst;
	RegionPtr clip;
	BoxRec srcBox;
	BoxPtr pBox, pMaxBox;
	pxPrivGCPtr gcPriv;
	int w, h, x0, y0;
	u_int32_t *pb;

#ifdef _IMAGEGLYPH
	PX_TRACE("pxSlowImageGlyphBlt");
#else
	PX_TRACE("pxSlowPolyGlyphBlt");
#endif

	pfont = pGC->font;
	x += pDrawable->x;
	y += pDrawable->y;
	gcPriv = pxGetGCPrivate(pGC);
	sp = gcPriv->sp;
	clip = cfbGetCompositeClip(pGC);

	pb = pxPacketStart(sp, &pxp, 4, 13);
	pb[0] = STAMP_CMD_LINES | STAMP_RGB_FLAT |
	    STAMP_LW_PERPRIMATIVE | STAMP_XY_PERPRIMATIVE;
	pb[1] = gcPriv->pmask;
	pb[2] = 0;
	pb[3] = gcPriv->umet | STAMP_WE_XYMASK;

	while (nglyph-- > 0) {
		pci = *ppci++;
		w = GLYPHWIDTHPIXELS(pci);
		h = GLYPHHEIGHTPIXELS(pci);

		if ((w | h) == 0)
			continue;

		x0 = x + pci->metrics.leftSideBearing;
		y0 = y - pci->metrics.ascent;
		srcBox.x1 = x0;
		srcBox.y1 = y0;
		srcBox.x2 = x0 + w;
		srcBox.y2 = y0 + h;
		REGION_INIT(pGC->pScreen, &rgnDst, &srcBox, 1);

		/*
		 * Clip the shape of the dst to the destination composite
		 * clip.
		 */
		REGION_INTERSECT(pGC->pScreen, &rgnDst, &rgnDst, clip);

		if (!REGION_NIL(&rgnDst)) {
			pBox = REGION_RECTS(&rgnDst);
			pMaxBox = pBox + REGION_NUM_RECTS(&rgnDst);

			if (pBox->y2 - pBox->y1 <= 16) {
				for (; pBox < pMaxBox; pBox++) {
#ifdef _IMAGEGLYPH
					pxSolidBox(sp, &pxp, pBox,
					    gcPriv->bgPixel);
#endif
					pxSqueege16(sp, &pxp, pci->bits,
					    PADGLYPHWIDTHBYTES(w),
					    pBox->x1, pBox->y1,
					    pBox->x2, pBox->y2,
					    pBox->x1 - x0,
					    pBox->y1 - y0,
					    gcPriv->fgFill);
				}
			} else {
				for (; pBox < pMaxBox; pBox++) {
#ifdef _IMAGEGLYPH
					pxSolidBox(sp, &pxp, pBox,
					    gcPriv->bgPixel);
#endif
					pxSqueege16(sp, &pxp, pci->bits,
					    PADGLYPHWIDTHBYTES(w),
					    pBox->x1, pBox->y1,
					    pBox->x2, pBox->y2,
					    pBox->x1 - x0,
					    pBox->y1 - y0,
					    gcPriv->fgFill);
				}
			}
		}

		x += pci->metrics.characterWidth;
		REGION_UNINIT(pGC->pScreen, &rgnDst);
	}

	pxPacketFlush(sp, &pxp);
}

#ifdef _IMAGEGLYPH
void
pxSolidBox(pxScreenPrivPtr sp, pxPacketPtr pxp, BoxPtr pBox, int fill)
{
	static const u_int allones = 0xffffffff;
	u_int32_t *pb;
	int lw, psy;

	pb = pxPacketAddPrim(sp, pxp);
	pb[0] = allones;
	pb[1] = allones;
	pb[2] = allones;
	pb[3] = allones;
	pb[4] = allones;
	pb[5] = allones;
	pb[6] = allones;
	pb[7] = allones;

	lw = ((pBox->y2 - pBox->y1) << 2) - 1;
	psy = (pBox->y1 << 3) + lw;

	pb[8] = XYMASKADDR(sp->stampw, sp->stamphm, pBox->x1, pBox->y1, 0, 0);
	pb[9] = (pBox->x1 << 19) | psy;
	pb[10] = (pBox->x2 << 19) | psy;
	pb[11] = lw;
	pb[12] = fill;
}
#endif
#endif
