/*	$NetBSD: sunZX.c,v 1.1 2003/01/27 22:46:14 ad Exp $	*/

/*
 * Acceleration for the Leo (ZX) framebuffer
 *
 * Copyright (C) 1999, 2000 Jakub Jelinek (jakub@redhat.com)
 *
 * Modified for Xsun by Andrew Doran (ad@netbsd.org)
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

#ifdef __NetBSD__

#define PSZ 32

#include "sun.h"
#include "sunZX.h"

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "cfb.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "gcstruct.h"
#include "window.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mi.h"
#include "mispans.h"
#include "mistruct.h"
#include "mibstore.h"
#include "migc.h"

#include "cfbmskbits.h"
#include "cfb8bit.h"

void LeoValidateGC(GCPtr, unsigned long, DrawablePtr);
void LeoDestroyGC(GCPtr);

RegionPtr LeoCopyArea(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable,
				 GCPtr pGC, int srcx, int srcy, int width, int height,
				 int dstx, int dsty);

RegionPtr LeoCopyPlane(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable,
				  GCPtr pGC, int srcx, int srcy, int width, int height,
				  int dstx, int dsty, unsigned long bitPlane);

void LeoFillBoxSolid (DrawablePtr pDrawable, int nBox,
				 BoxPtr pBox, unsigned long pixel);

void LeoPolyFillRect(DrawablePtr pDrawable, GCPtr pGC,
				int nrectFill, xRectangle *prectInit);

void LeoPolyFillRect1Rect(DrawablePtr pDrawable, register GCPtr pGC,
				     int nrectFill, xRectangle *prectInit);

void LeoPolyFillStippledRect(DrawablePtr pDrawable, GCPtr pGC,
				    int nrectFill, xRectangle *prectInit);

void LeoFillSpansSolid (DrawablePtr pDrawable, GCPtr pGC,
			       int n, DDXPointPtr ppt,
			       int *pwidth, int fSorted);

void LeoFillSpansStippled (DrawablePtr pDrawable, GCPtr pGC,
			          int n, DDXPointPtr ppt,
			          int *pwidth, int fSorted);

void LeoPolyGlyphBlt (DrawablePtr pDrawable, GCPtr pGC, int x, int y,
				 unsigned int nglyph, CharInfoPtr *ppci, pointer pGlyphBase);

void LeoTEGlyphBlt (DrawablePtr pDrawable, GCPtr pGC, int x, int y,
			       unsigned int nglyph, CharInfoPtr *ppci, pointer pGlyphBase);

void LeoPolyTEGlyphBlt (DrawablePtr pDrawable, GCPtr pGC, int x, int y,
				   unsigned int nglyph, CharInfoPtr *ppci, pointer pGlyphBase);

void LeoFillPoly1RectGeneral(DrawablePtr pDrawable, GCPtr pGC, int shape, 
			int mode, int count, DDXPointPtr ptsIn);

void LeoZeroPolyArcSS8General(DrawablePtr pDraw, GCPtr pGC, int narcs, xArc *parcs);

void LeoTile32FSGeneral(DrawablePtr pDrawable, GCPtr pGC, int nInit,
			       DDXPointPtr pptInit, int *pwidthInit, int fSorted);

void LeoPolyFillArcSolidGeneral(DrawablePtr pDrawable, GCPtr pGC, 
				       int narcs, xArc *parcs);

int LeoCheckFill (GCPtr pGC, DrawablePtr pDrawable);

void LeoDoBitblt (DrawablePtr pSrc, DrawablePtr pDst, int alu, RegionPtr prgnDst,
			 DDXPointPtr pptSrc, unsigned long planemask);

int	LeoScreenPrivateIndex;
int	LeoGCPrivateIndex;
int	LeoWindowPrivateIndex;
int	LeoGeneration;

const int	leoRopTable[16] = {
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_ZERO,		/* GXclear */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW_AND_OLD,	/* GXand */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW_AND_NOLD,	/* GXandReverse */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW,		/* GXcopy */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NNEW_AND_OLD,	/* GXandInverted */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_OLD,		/* GXnoop */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW_XOR_OLD,	/* GXxor */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW_OR_OLD,	/* GXor */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NNEW_AND_NOLD,	/* GXnor */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NNEW_XOR_NOLD,	/* GXequiv */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NOLD,		/* GXinvert */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW_OR_NOLD,	/* GXorReverse */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NNEW,		/* GXcopyInverted */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NNEW_OR_OLD,	/* GXorInverted */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_NNEW_OR_NOLD,	/* GXnand */
	LEO_ATTR_RGBE_ENABLE|LEO_ROP_ONES		/* GXset */
};

GCFuncs LeoGCFuncs = {
	LeoValidateGC,
	miChangeGC,
	miCopyGC,
	LeoDestroyGC,
	miChangeClip,
	miDestroyClip,
	miCopyClip,
};

GCOps	LeoTEOps1Rect = {
	cfbSolidSpansCopy,
	cfbSetSpans,
	cfbPutImage,
	LeoCopyArea,
	cfbCopyPlane,
	cfbPolyPoint,
	cfb8LineSS1Rect,
	cfb8SegmentSS1Rect,
	miPolyRectangle,
	cfbZeroPolyArcSS8Copy,
	cfbFillPoly1RectCopy,
	LeoPolyFillRect1Rect,
	cfbPolyFillArcSolidCopy,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	LeoTEGlyphBlt,
	LeoPolyTEGlyphBlt,
	mfbPushPixels
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

GCOps	LeoNonTEOps1Rect = {
	cfbSolidSpansCopy,
	cfbSetSpans,
	cfbPutImage,
	LeoCopyArea,
	cfbCopyPlane,
	cfbPolyPoint,
	cfb8LineSS1Rect,
	cfb8SegmentSS1Rect,
	miPolyRectangle,
	cfbZeroPolyArcSS8Copy,
	cfbFillPoly1RectCopy,
	LeoPolyFillRect1Rect,
	cfbPolyFillArcSolidCopy,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	cfbImageGlyphBlt8,
	LeoPolyGlyphBlt,
	mfbPushPixels
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

GCOps	LeoTEOps = {
	cfbSolidSpansCopy,
	cfbSetSpans,
	cfbPutImage,
	LeoCopyArea,
	cfbCopyPlane,
	cfbPolyPoint,
	cfbLineSS,
	cfbSegmentSS,
	miPolyRectangle,
	cfbZeroPolyArcSS8Copy,
	miFillPolygon,
	LeoPolyFillRect,
	cfbPolyFillArcSolidCopy,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	LeoTEGlyphBlt,
	LeoPolyTEGlyphBlt,
	mfbPushPixels
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

GCOps	LeoNonTEOps = {
	cfbSolidSpansCopy,
	cfbSetSpans,
	cfbPutImage,
	LeoCopyArea,
	cfbCopyPlane,
	cfbPolyPoint,
	cfbLineSS,
	cfbSegmentSS,
	miPolyRectangle,
	cfbZeroPolyArcSS8Copy,
	miFillPolygon,
	LeoPolyFillRect,
	cfbPolyFillArcSolidCopy,
	miPolyText8,
	miPolyText16,
	miImageText8,
	miImageText16,
	cfbImageGlyphBlt8,
	LeoPolyGlyphBlt,
	mfbPushPixels
#ifdef NEED_LINEHELPER
	,NULL
#endif
};

void
LeoPolyGlyphBlt (DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		     unsigned int nglyph, CharInfoPtr *ppci, pointer pGlyphBase)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pGC->pScreen);
	LeoCommand0 *lc0 = pLeo->lc0;
	LeoDraw *ld0 = pLeo->ld0;
	RegionPtr clip;
	CharInfoPtr pci;
	int w, h, x0, y0, i;
	unsigned int *bits;
	BoxRec box;
	int curw = -1;
	unsigned int *fbf;
	unsigned char *fb;
	int height, width;

	clip = cfbGetCompositeClip(pGC);
	/* compute an approximate (but covering) bounding box */
	box.x1 = 0;
	if (ppci[0]->metrics.leftSideBearing < 0)
		box.x1 = ppci[0]->metrics.leftSideBearing;
	h = nglyph - 1;
	w = ppci[h]->metrics.rightSideBearing;
	while (--h >= 0)
		w += ppci[h]->metrics.characterWidth;
	box.x2 = w;
	box.y1 = -FONTMAXBOUNDS(pGC->font,ascent);
	box.y2 = FONTMAXBOUNDS(pGC->font,descent);
		
	box.x1 += pDrawable->x + x;
	box.x2 += pDrawable->x + x;
	box.y1 += pDrawable->y + y;
	box.y2 += pDrawable->y + y;
	
	switch (RECT_IN_REGION(pGC->pScreen, clip, &box)) {
	case rgnPART:
		if (REGION_NUM_RECTS(clip) == 1) {
			SETREG(ld0->vclipmin, (clip->extents.y1 << 16) | clip->extents.x1);
			SETREG(ld0->vclipmax, ((clip->extents.y2 - 1) << 16) | (clip->extents.x2 - 1));
			break;
		}
		cfbPolyGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, pGlyphBase);
	case rgnOUT:
		return;
	default:
		clip = NULL;
		break;
	}
	
	x += pDrawable->x;
	y += pDrawable->y;
	
	SETREG(lc0->fontt, 1);
	SETREG(lc0->addrspace, LEO_ADDRSPC_FONT_OBGR);
	SETREG(ld0->fg, pGC->fgPixel);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	height = pLeo->height;
	width = pLeo->width;
	
	fb = (unsigned char *)pLeo->fb;

	while (nglyph--) {
		pci = *ppci++;

		w = GLYPHWIDTHPIXELS (pci);
		h = GLYPHHEIGHTPIXELS (pci);
		if (!w || !h)
			goto next_glyph;

		x0 = x + pci->metrics.leftSideBearing;
		y0 = y - pci->metrics.ascent;

		/* We're off the screen to the left, making our way
		 * back onto the screen.
		 */
		if((x0 >> 31) == -1)
			goto next_glyph;

		/* We walked off the screen (to the right or downwards)
		 * or we started there, we're never going to work our
		 * way back so stop now.
		 */
		if(x0 >= width || y0 >= height)
			break;
		
		bits = (unsigned int *) pci->bits;

		if (w != curw) {
			curw = w;
			if (w)
				SETREG(lc0->fontmsk, 0xffffffff << (32 - w));
			else
				SETREG(lc0->fontmsk, 0);
		}

		fbf = (unsigned *)(fb + (y0 << 13) + (x0 << 2));
		if (y0 + h <= height)
			for (i = 0; i < h; i++) {
				*fbf = *bits++;
				fbf += 2048;
			}
		else
			for (i = 0; i < h && y0 + i < height; i++) {
				*fbf = *bits++;
				fbf += 2048;
			}
	next_glyph:
		x += pci->metrics.characterWidth;
	}
	
	SETREG(lc0->addrspace, LEO_ADDRSPC_OBGR);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
	if (clip) {
		SETREG(ld0->vclipmin, 0);
		SETREG(ld0->vclipmax, pLeo->vclipmax);
	}
}

void
LeoTEGlyphBlt (DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		   unsigned int nglyph, CharInfoPtr *ppci, pointer pGlyphBase)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pGC->pScreen);
	LeoCommand0 *lc0 = pLeo->lc0;
	LeoDraw *ld0 = pLeo->ld0;
	RegionPtr clip;
	int h, hTmp;
	int widthGlyph, widthGlyphs;
	BoxRec bbox;
	FontPtr pfont = pGC->font;
	int curw = -1;
	unsigned int *fbf;
	unsigned char *fb;
	int height, width;

	widthGlyph = FONTMAXBOUNDS(pfont,characterWidth);
	h = FONTASCENT(pfont) + FONTDESCENT(pfont);
	clip = cfbGetCompositeClip(pGC);
	bbox.x1 = x + pDrawable->x;
	bbox.x2 = bbox.x1 + (widthGlyph * nglyph);
	bbox.y1 = y + pDrawable->y - FONTASCENT(pfont);
	bbox.y2 = bbox.y1 + h;

	/* If fully out of range, and we have no chance of getting back
	 * in range, no work to do.
	 */
	y = y + pDrawable->y - FONTASCENT(pfont);
	x += pDrawable->x;
	height = pLeo->height;
	width = pLeo->width;
	
	if (x >= width)
	   	return;

	switch (RECT_IN_REGION(pGC->pScreen, clip, &bbox)) {
	case rgnPART: 
		if (REGION_NUM_RECTS(clip) == 1) {
			SETREG(ld0->vclipmin, (clip->extents.y1 << 16) | clip->extents.x1);
			SETREG(ld0->vclipmax, ((clip->extents.y2 - 1) << 16) | (clip->extents.x2 - 1));
			break;
		}
		x -= pDrawable->x;
		y = y - pDrawable->y + FONTASCENT(pfont);
		if (pGlyphBase)
			cfbPolyGlyphBlt8 (pDrawable, pGC, x, y, nglyph, ppci, NULL);
		else
			miImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pGlyphBase);
	case rgnOUT:
		return;
	default:
		clip = NULL;
		break;
	}

	SETREG(lc0->addrspace, LEO_ADDRSPC_FONT_OBGR);
	SETREG(ld0->fg, pGC->fgPixel);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
		
	fb = (unsigned char *)pLeo->fb;

	if(pGlyphBase)
		SETREG(lc0->fontt, 1);
	else {
		SETREG(lc0->fontt, 0);
		SETREG(ld0->bg, pGC->bgPixel);
	}

#define LoopIt(count, w, loadup, fetch) \
	if (w != curw) { \
		curw = w; \
		SETREG(lc0->fontmsk, 0xffffffff << (32 - w)); \
	} \
	while (nglyph >= count) { \
		loadup \
		nglyph -= count; \
		fbf = (unsigned *)(fb + (y << 13) + (x << 2)); \
		hTmp = h; \
		if (y + h <= height) \
			while (hTmp--) { \
				*fbf = fetch; \
				fbf += 2048; \
			} \
		else \
			for (hTmp = 0; hTmp < h && y + hTmp < height; hTmp++) { \
				*fbf = fetch; \
				fbf += 2048; \
			} \
		x += w; \
		if(x >= width) \
			goto out; \
	}

	if (widthGlyph <= 8) {
		widthGlyphs = widthGlyph << 2;
		LoopIt(4, widthGlyphs,
		       unsigned int *char1 = (unsigned int *) (*ppci++)->bits;
		       unsigned int *char2 = (unsigned int *) (*ppci++)->bits;
		       unsigned int *char3 = (unsigned int *) (*ppci++)->bits;
		       unsigned int *char4 = (unsigned int *) (*ppci++)->bits;,
		       (*char1++ | ((*char2++ | ((*char3++ | (*char4++
							      >> widthGlyph))
						 >> widthGlyph))
				    >> widthGlyph)))
	} else if (widthGlyph <= 10) {
		widthGlyphs = (widthGlyph << 1) + widthGlyph;
		LoopIt(3, widthGlyphs,
		       unsigned int *char1 = (unsigned int *) (*ppci++)->bits;
		       unsigned int *char2 = (unsigned int *) (*ppci++)->bits;
		       unsigned int *char3 = (unsigned int *) (*ppci++)->bits;,
		       (*char1++ | ((*char2++ | (*char3++ >> widthGlyph)) >> widthGlyph)))
	} else if (widthGlyph <= 16) {
		widthGlyphs = widthGlyph << 1;
		LoopIt(2, widthGlyphs,
		       unsigned int *char1 = (unsigned int *) (*ppci++)->bits;
		       unsigned int *char2 = (unsigned int *) (*ppci++)->bits;,
		       (*char1++ | (*char2++ >> widthGlyph)))
	}
	if(nglyph != 0) {
		if (widthGlyph != curw) {
			curw = widthGlyph;
			SETREG(lc0->fontmsk, 0xffffffff << (32 - widthGlyph));
		}
		while (nglyph--) {
			unsigned int *char1 = (unsigned int *) (*ppci++)->bits;
			fbf = (unsigned *)(fb + (y << 13) + (x << 2));
			hTmp = h;
			if (y + h <= height)
				while (hTmp--) {
					*fbf = *char1++;
					fbf += 2048;
				}
			else
				for (hTmp = 0; hTmp < h && y + hTmp < height; hTmp++) {
					*fbf = *char1++;
					fbf += 2048;
				}
			x += widthGlyph;
			if (x >= width)
				goto out;
		}
	}
	
out:	SETREG(lc0->addrspace, LEO_ADDRSPC_OBGR);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
	if (clip) {
		SETREG(ld0->vclipmin, 0);
		SETREG(ld0->vclipmax, pLeo->vclipmax);
	}
}

void
LeoPolyTEGlyphBlt (DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		       unsigned int nglyph, CharInfoPtr *ppci, pointer pGlyphBase)
{
	LeoTEGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, (char *) 1);
}


void
LeoFillPoly1RectGeneral(DrawablePtr pDrawable, GCPtr pGC, int shape, 
			int mode, int count, DDXPointPtr ptsIn)
{
        LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoDraw		*ld0 = pLeo->ld0;

	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	cfbFillPoly1RectCopy(pDrawable, pGC, shape, mode, count, ptsIn);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoZeroPolyArcSS8General(DrawablePtr pDrawable, GCPtr pGC, int narcs, xArc *parcs)
{
        LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoDraw		*ld0 = pLeo->ld0;

	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	cfbZeroPolyArcSS8Copy(pDrawable, pGC, narcs, parcs);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoTile32FSGeneral(DrawablePtr pDrawable, GCPtr pGC, int nInit,
		   DDXPointPtr pptInit, int *pwidthInit, int fSorted)
{
        LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoDraw		*ld0 = pLeo->ld0;

	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	cfbTile32FSCopy(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoPolyFillArcSolidGeneral(DrawablePtr pDrawable, GCPtr pGC,
			   int narcs, xArc *parcs)
{
        LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoDraw		*ld0 = pLeo->ld0;

	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	cfbPolyFillArcSolidCopy(pDrawable, pGC, narcs, parcs);
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoDoBitblt(DrawablePtr pSrc, DrawablePtr pDst, int alu, RegionPtr prgnDst,
		DDXPointPtr pptSrc, unsigned long planemask)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pDst->pScreen);
	LeoCommand0 *lc0 = pLeo->lc0;
	LeoDraw *ld0 = pLeo->ld0;
	BoxPtr pboxTmp;
	DDXPointPtr pptTmp;
	int nbox;
	BoxPtr pboxNext, pboxBase, pbox;

	pbox = REGION_RECTS(prgnDst);
	nbox = REGION_NUM_RECTS(prgnDst);

	pptTmp = pptSrc;
	pboxTmp = pbox;

	SETREG(ld0->rop, leoRopTable[alu]);

	if (pptSrc->y < pbox->y1) {
		if (pptSrc->x < pbox->x1) {
			/* reverse order of bands and rects in each band */
			pboxTmp=pbox+nbox;
                        pptTmp=pptSrc+nbox;

			while (nbox--){
				pboxTmp--;
				pptTmp--;
				if (pptTmp->y <= pboxTmp->y2) {
					SETREG(lc0->extent, 0x80000000 | (pboxTmp->x2 - pboxTmp->x1 - 1) |
						      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
					SETREG(lc0->src, (pptTmp->x + pboxTmp->x2 - pboxTmp->x1 - 1) |
						   ((pptTmp->y + pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
					SETREG(lc0->copy, (pboxTmp->x2 - 1) | ((pboxTmp->y2 - 1) << 11));
				} else {
					SETREG(lc0->extent, (pboxTmp->x2 - pboxTmp->x1 - 1) |
						      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
					SETREG(lc0->src, pptTmp->x | (pptTmp->y << 11));
					SETREG(lc0->copy, pboxTmp->x1 | (pboxTmp->y1 << 11));
				}
				while (lc0->csr & LEO_CSR_BLT_BUSY);
			}
		} else {
			/* keep ordering in each band, reverse order of bands */
			pboxBase = pboxNext = pbox+nbox-1;

			while (pboxBase >= pbox) {			/* for each band */

				/* find first box in band */
				while (pboxNext >= pbox &&
				       pboxBase->y1 == pboxNext->y1)
					pboxNext--;
		
				pboxTmp = pboxNext+1;			/* first box in band */
				pptTmp = pptSrc + (pboxTmp - pbox);	/* first point in band */
		
				while (pboxTmp <= pboxBase) {		/* for each box in band */
					if (pptTmp->y <= pboxTmp->y2) {
						SETREG(lc0->extent, 0x80000000 | (pboxTmp->x2 - pboxTmp->x1 - 1) |
							      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
						SETREG(lc0->src, (pptTmp->x + pboxTmp->x2 - pboxTmp->x1 - 1) |
							   ((pptTmp->y + pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
						SETREG(lc0->copy, (pboxTmp->x2 - 1) | ((pboxTmp->y2 - 1) << 11));
					} else {
						SETREG(lc0->extent, (pboxTmp->x2 - pboxTmp->x1 - 1) |
							      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
						SETREG(lc0->src, pptTmp->x | (pptTmp->y << 11));
						SETREG(lc0->copy, pboxTmp->x1 | (pboxTmp->y1 << 11));
					}
					while (lc0->csr & LEO_CSR_BLT_BUSY);
					++pboxTmp;
					++pptTmp;	
				}
				pboxBase = pboxNext;
			
			}
		}
	} else {
		if (pptSrc->x < pbox->x1) {
			/* reverse order of rects in each band */

			pboxBase = pboxNext = pbox;

			while (pboxBase < pbox+nbox) { /* for each band */

				/* find last box in band */
				while (pboxNext < pbox+nbox &&
				       pboxNext->y1 == pboxBase->y1)
					pboxNext++;

				pboxTmp = pboxNext;			/* last box in band */
				pptTmp = pptSrc + (pboxTmp - pbox);	/* last point in band */

				if (pptSrc->y == pbox->y1) {
					while (pboxTmp != pboxBase) {		/* for each box in band */
						--pboxTmp;
						--pptTmp;
						SETREG(lc0->extent, 0x80000000 | (pboxTmp->x2 - pboxTmp->x1 - 1) |
							      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
						SETREG(lc0->src, (pptTmp->x + pboxTmp->x2 - pboxTmp->x1 - 1) |
							   ((pptTmp->y + pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
						SETREG(lc0->copy, (pboxTmp->x2 - 1) | ((pboxTmp->y2 - 1) << 11));
						while (lc0->csr & LEO_CSR_BLT_BUSY);
					}
				} else {
					while (pboxTmp != pboxBase) {		/* for each box in band */
						--pboxTmp;
						--pptTmp;
						SETREG(lc0->extent, (pboxTmp->x2 - pboxTmp->x1 - 1) |
							      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
						SETREG(lc0->src, pptTmp->x | (pptTmp->y << 11));
						SETREG(lc0->copy, pboxTmp->x1 | (pboxTmp->y1 << 11));
						while (lc0->csr & LEO_CSR_BLT_BUSY);
					}
				}
				pboxBase = pboxNext;
			}
		} else {
			while (nbox--) {
				SETREG(lc0->extent, (pboxTmp->x2 - pboxTmp->x1 - 1) |
					      ((pboxTmp->y2 - pboxTmp->y1 - 1) << 11));
				SETREG(lc0->src, pptTmp->x | (pptTmp->y << 11));
				SETREG(lc0->copy, pboxTmp->x1 | (pboxTmp->y1 << 11));
				while (lc0->csr & LEO_CSR_BLT_BUSY);
				pboxTmp++;
				pptTmp++;
			}
		}
	}

	SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
}

RegionPtr
LeoCopyArea(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable,
		GCPtr pGC, int srcx, int srcy, int width, int height, int dstx, int dsty)
{
	if (pSrcDrawable->type != DRAWABLE_WINDOW)
		return cfbCopyArea (pSrcDrawable, pDstDrawable,
				    pGC, srcx, srcy, width, height, dstx, dsty);
	return cfbBitBlt (pSrcDrawable, pDstDrawable,
			  pGC, srcx, srcy, width, height, dstx, dsty, (void (*)())LeoDoBitblt, 0);
}

void
LeoFillSpansSolid (DrawablePtr pDrawable, GCPtr pGC,
		   int n, DDXPointPtr ppt,
		   int *pwidth, int fSorted)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pGC->pScreen);
	LeoCommand0 *lc0 = pLeo->lc0;
	LeoDraw *ld0 = pLeo->ld0;
	int numRects, *pwidthFree;
	DDXPointPtr pptFree;
	RegionPtr clip;
	unsigned char *fb;
	int fg;
	int cx1 = 0, cy1 = 0, cx2 = 0, cy2 = 0;

	clip = cfbGetCompositeClip(pGC);
	numRects = REGION_NUM_RECTS(clip);
	
	if (!numRects)
		return;
		
	if (numRects == 1) {
		cx1 = clip->extents.x1;
		cx2 = clip->extents.x2;
		cy1 = clip->extents.y1;
		cy2 = clip->extents.y2;
	} else {
		int nTmp = n * miFindMaxBand(clip);

		pwidthFree = (int *)ALLOCATE_LOCAL(nTmp * sizeof(int));
		pptFree = (DDXPointRec *)ALLOCATE_LOCAL(nTmp * sizeof(DDXPointRec));
		if (!pptFree || !pwidthFree) {
			if (pptFree) DEALLOCATE_LOCAL(pptFree);
			if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
			return;
		}
		n = miClipSpans(clip,
				ppt, pwidth, n,
				pptFree, pwidthFree, fSorted);
		pwidth = pwidthFree;
		ppt = pptFree;
	}
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	SETREG(ld0->fg, fg = pGC->fgPixel);
	fb = (unsigned char *)pLeo->fb;
	
	while (n--) {
		int x, y, w;
		unsigned int *fbf;
		
		w = *pwidth++;
		x = ppt->x;
		y = ppt->y;
		ppt++;

		if (numRects == 1) {
			if (y < cy1 || y >= cy2) continue;
			if (x < cx1) {
				w -= (cx1 - x);
				if (w <= 0) continue;
				x = cx1;
			}
			if (x + w > cx2) {
				if (x >= cx2) continue;
				w = cx2 - x;
			}
		}
		
		if (w > 12) {
			SETREG(lc0->extent, w - 1);
			SETREG(lc0->fill, (y << 11) | x);
			while (lc0->csr & LEO_CSR_BLT_BUSY);
		} else {
			fbf = (unsigned int *)(fb + (y << 13) + (x << 2));
			while (w--)
				*fbf++ = fg;
		}
	}
	
	if (numRects != 1) {
		DEALLOCATE_LOCAL(pptFree);
		DEALLOCATE_LOCAL(pwidthFree);
	}
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoFillSpansStippled (DrawablePtr pDrawable, GCPtr pGC,
		      int n, DDXPointPtr ppt,
		      int *pwidth, int fSorted)
{
	LeoPrivGCPtr gcPriv = LeoGetGCPrivate (pGC);
	LeoPtr pLeo = LeoGetScreenPrivate (pGC->pScreen);
	LeoCommand0 *lc0 = pLeo->lc0;
	LeoDraw *ld0 = pLeo->ld0;
	int numRects, *pwidthFree;
	DDXPointPtr pptFree;
	RegionPtr clip;
	unsigned char *fb;
	unsigned int *bits, msk;
	int cx1 = 0, cy1 = 0, cx2 = 0, cy2 = 0;

	clip = cfbGetCompositeClip(pGC);
	numRects = REGION_NUM_RECTS(clip);
	
	if (!numRects)
		return;
		
	if (numRects == 1) {
		cx1 = clip->extents.x1;
		cx2 = clip->extents.x2;
		cy1 = clip->extents.y1;
		cy2 = clip->extents.y2;
	} else {
		int nTmp = n * miFindMaxBand(clip);

		pwidthFree = (int *)ALLOCATE_LOCAL(nTmp * sizeof(int));
		pptFree = (DDXPointRec *)ALLOCATE_LOCAL(nTmp * sizeof(DDXPointRec));
		if (!pptFree || !pwidthFree) {
			if (pptFree) DEALLOCATE_LOCAL(pptFree);
			if (pwidthFree) DEALLOCATE_LOCAL(pwidthFree);
			return;
		}
		n = miClipSpans(clip,
				ppt, pwidth, n,
				pptFree, pwidthFree, fSorted);
		pwidth = pwidthFree;
		ppt = pptFree;
	}
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	SETREG(ld0->fg, gcPriv->stipple->fg);
	fb = (unsigned char *)pLeo->fb;
	SETREG(lc0->addrspace, LEO_ADDRSPC_FONT_OBGR);
	if (gcPriv->stipple->alu & 0x80) {
		SETREG(lc0->fontt, 1);
	} else {
		SETREG(lc0->fontt, 0);
		SETREG(ld0->bg, gcPriv->stipple->bg);
	}
	SETREG(lc0->fontmsk, 0xffffffff);
	msk = 0xffffffff;
	bits = &gcPriv->stipple->bits[0];
	while (n--) {
		int x, y, w;
		unsigned int *dst, s, i, sw, sm;
		
		w = *pwidth++;
		x = ppt->x;
		y = ppt->y;
		ppt++;
		if (numRects == 1) {
			if (y < cy1 || y >= cy2) continue;
			if (x < cx1) {
				w -= (cx1 - x);
				if (w <= 0) continue;
				x = cx1;
			}
			if (x + w > cx2) {
				if (x >= cx2) continue;
				w = cx2 - x;
			}
		}
		s = bits[y & 31];
		dst = (unsigned int *)(fb + (y << 13) + ((x & ~31) << 2));
		if (x & 31) {
			sw = 32 - (x & 31);
			sm = 0xffffffff >> (x & 31);
			w -= sw;
			if (w <= 0) {
				if (w) sm &= 0xffffffff << (32 - (w & 31));
				if (msk != sm) {
					msk = sm;
					SETREG(lc0->fontmsk, sm);
				}
				*dst = s;
				continue;
			}
			if (msk != sm) {
				msk = sm;
				SETREG(lc0->fontmsk, sm);
			}
			*dst = s;
			dst += 32;
		} else {
			sw = 0;
		}
		sw = w & 31;
		w &= ~31;
		if (w && msk != 0xffffffff) {
			msk = 0xffffffff;
			SETREG(lc0->fontmsk, 0xffffffff);
			for (i = 0; i < w; i += 32) {
				*dst = s;
				dst += 32;
			}
		}
		if (sw) {
			msk = 0xffffffff << (32 - sw);
			SETREG(lc0->fontmsk, msk);
			*dst = s;
		}
	}
	
	if (numRects != 1) {
		DEALLOCATE_LOCAL(pptFree);
		DEALLOCATE_LOCAL(pwidthFree);
	}
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
	lc0->addrspace = LEO_ADDRSPC_OBGR;
}

void
LeoPolyFillRect(DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle *prectInit)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoCommand0 	*lc0 = pLeo->lc0;
	LeoDraw		*ld0 = pLeo->ld0;
	xRectangle	*prect;
	RegionPtr	prgnClip;
	register BoxPtr	pbox;
	BoxPtr		pextent;
	int		n;
	int		xorg, yorg;
    
	/* No garbage please. */
	if(nrectFill <= 0)
		return;

	prgnClip = cfbGetCompositeClip(pGC);

	prect = prectInit;
	xorg = pDrawable->x;
	yorg = pDrawable->y;
	if (xorg || yorg) {
		prect = prectInit;
		n = nrectFill;
		while(n--) {
			prect->x += xorg;
			prect->y += yorg;
			prect++;
		}
	}

	prect = prectInit;
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	SETREG(ld0->fg, pGC->fgPixel);

	if (REGION_NUM_RECTS(prgnClip) == 1) {
		int x1, y1, x2, y2;
		int x, y, xx, yy;

		pextent = REGION_RECTS(prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;
		while (nrectFill--) {
			x = prect->x;
			y = prect->y;
			xx = x + prect->width;
			yy = y + prect->height;
			if (x < x1)
				x = x1;
			if (y < y1)
				y = y1;
			prect++;
			if (xx > x2) xx = x2;
			if (yy > y2) yy = y2;
			if (x >= xx) continue;
			if (y >= yy) continue;

			SETREG(lc0->extent, (xx - x - 1) | ((yy - y - 1) << 11));
			SETREG(lc0->fill, x | (y << 11));
			while (lc0->csr & LEO_CSR_BLT_BUSY);
		}
	} else {
		int x1, y1, x2, y2, bx1, by1, bx2, by2;
		int x, y, w, h;

		pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;
		while (nrectFill--) {
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

			prect++;
    
			if (bx1 >= bx2 || by1 >= by2)
				continue;
    
			n = REGION_NUM_RECTS (prgnClip);
			pbox = REGION_RECTS(prgnClip);
    
			/* clip the rectangle to each box in the clip region
			   this is logically equivalent to calling Intersect()
			 */
			while(n--) {
				x = max(bx1, pbox->x1);
				y = max(by1, pbox->y1);
				w = min(bx2, pbox->x2) - x;
				h = min(by2, pbox->y2) - y;
				pbox++;

				/* see if clipping left anything */
				if (w > 0 && h > 0) {
					SETREG(lc0->extent, (w - 1) | ((h - 1) << 11));
					SETREG(lc0->fill, x | (y << 11));
			
					while (lc0->csr & LEO_CSR_BLT_BUSY);
				}
			}
		}
	}
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoPolyFillRect1Rect(DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle *prectInit)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoCommand0 	*lc0 = pLeo->lc0;
	LeoDraw		*ld0 = pLeo->ld0;
	xRectangle	*prect;
	RegionPtr	prgnClip;
	BoxPtr		pextent;
	int		n;
	int		xorg, yorg;
	int		x1, y1, x2, y2;
	int		x, y, xx, yy;
    
	/* No garbage please. */
	if(nrectFill <= 0)
		return;

	prgnClip = cfbGetCompositeClip(pGC);

	prect = prectInit;
	xorg = pDrawable->x;
	yorg = pDrawable->y;
	if (xorg || yorg) {
		prect = prectInit;
		n = nrectFill;
		while(n--) {
			prect->x += xorg;
			prect->y += yorg;
			prect++;
		}
	}

	prect = prectInit;
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, leoRopTable[pGC->alu]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	SETREG(ld0->fg, pGC->fgPixel);

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
	while (nrectFill--) {
		x = prect->x;
		y = prect->y;
		xx = x + prect->width;
		yy = y + prect->height;
		if (x < x1)
			x = x1;
		if (y < y1)
			y = y1;
		prect++;
		if (xx > x2) xx = x2;
		if (x >= xx) continue;
		if (yy > y2) yy = y2;
		if (y >= yy) continue;

		SETREG(lc0->extent, (xx - x - 1) | ((yy - y - 1) << 11));
		SETREG(lc0->fill, x | (y << 11));
		while (lc0->csr & LEO_CSR_BLT_BUSY);
	}
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
}

void
LeoPolyFillStippledRect(DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle *prectInit)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	LeoPrivGCPtr	gcPriv = LeoGetGCPrivate (pGC);
	LeoCommand0 	*lc0 = pLeo->lc0;
	LeoDraw		*ld0 = pLeo->ld0;
	xRectangle	*prect;
	RegionPtr	prgnClip;
	register BoxPtr	pbox;
	BoxPtr		pextent;
	int		n;
	int		xorg, yorg;
	unsigned char	*fb;
	unsigned int	*dst, *dline, *src, *srcstart, *srcend;
    
	/* No garbage please. */
	if(nrectFill <= 0)
		return;

	prgnClip = cfbGetCompositeClip(pGC);

	prect = prectInit;
	xorg = pDrawable->x;
	yorg = pDrawable->y;
	if (xorg || yorg) {
		prect = prectInit;
		n = nrectFill;
		while(n--) {
			prect->x += xorg;
			prect->y += yorg;
			prect++;
		}
	}

	prect = prectInit;
	
	if ((gcPriv->stipple->alu & 0xf) != GXcopy)
		SETREG(ld0->rop, leoRopTable[gcPriv->stipple->alu & 0xf]);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, pGC->planemask);
	SETREG(ld0->fg, gcPriv->stipple->fg);
	SETREG(lc0->addrspace, LEO_ADDRSPC_FONT_OBGR);
	if (gcPriv->stipple->alu & 0x80) {
		SETREG(lc0->fontt, 1);
	} else {
		SETREG(lc0->fontt, 0);
		SETREG(ld0->bg, gcPriv->stipple->bg);
	}
	fb = (unsigned char *)pLeo->fb;
	srcstart = &gcPriv->stipple->bits[0];
	srcend = &gcPriv->stipple->bits[32];

	if (REGION_NUM_RECTS(prgnClip) == 1) {
		int x1, y1, x2, y2;
		int x, y, xx, yy, w, h;
		int i, j, sw, sm, ew, em, s;

		pextent = REGION_RECTS(prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;
		while (nrectFill--) {
			x = prect->x;
			y = prect->y;
			xx = x + prect->width;
			yy = y + prect->height;
			if (x < x1) x = x1;
			if (y < y1) y = y1;
			if (xx > x2) xx = x2;
			if (yy > y2) yy = y2;
			prect++;
			if (x >= xx) continue;
			if (y >= yy) continue;
			prect++;
			w = xx - x;
			h = yy - y;
			if (x & 31) {
				sw = 32 - (x & 31);
				sm = 0xffffffff >> (x & 31);
			} else {
				sw = 0;
				sm = 0xffffffff;
			}
			dline = (unsigned int *)(fb + (y << 13) + ((x & ~31) << 2));
			src = srcstart + (y & 31);
			w -= sw;
			if (w <= 0) {
				if (w)
					sm &= 0xffffffff << (32 - (w & 31));
				SETREG(lc0->fontmsk, sm);
				
				for (i = 0; i < h; i++) {
					s = *src++;
					*dline = s;
					if (src == srcend)
						src = srcstart;
					dline += 2048;
				}
			} else {
				ew = w & 31;
				em = 0xffffffff << (32 - ew);
				w &= ~31;
				
				if (!sw && !ew)
					SETREG(lc0->fontmsk, 0xffffffff);
				for (i = 0; i < h; i++) {
					s = *src++;
					dst = dline;
					if (sw) {
						SETREG(lc0->fontmsk, sm);
						*dst = s;
						dst += 32;
						SETREG(lc0->fontmsk, 0xffffffff);
					} else if (ew)
						SETREG(lc0->fontmsk, 0xffffffff);
					for (j = 0; j < w; j += 32) {
						*dst = s;
						dst += 32;
					}
					if (ew) {
						SETREG(lc0->fontmsk, em);
						*dst = s;
					}
					if (src == srcend)
						src = srcstart;
					dline += 2048;
				}
			}
		}
	} else {
		int x1, y1, x2, y2, bx1, by1, bx2, by2;
		int x, y, w, h;
		int i, j, sw, sm, ew, em, s;

		pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;
		while (nrectFill--) {
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

			prect++;
    
			if (bx1 >= bx2 || by1 >= by2)
				continue;
    
			n = REGION_NUM_RECTS (prgnClip);
			pbox = REGION_RECTS(prgnClip);
    
			/* clip the rectangle to each box in the clip region
			   this is logically equivalent to calling Intersect()
			 */
			while(n--) {
				x = max(bx1, pbox->x1);
				y = max(by1, pbox->y1);
				w = min(bx2, pbox->x2) - x;
				h = min(by2, pbox->y2) - y;
				pbox++;

				/* see if clipping left anything */
				if (w > 0 && h > 0) {
					if (x & 31) {
						sw = 32 - (x & 31);
						sm = 0xffffffff >> (x & 31);
					} else {
						sw = 0;
						sm = 0xffffffff;
					}
					dline = (unsigned int *)(fb + (y << 13) + ((x & ~31) << 2));
					src = srcstart + (y & 31);
					w -= sw;
					if (w <= 0) {
						w += 32;
						if (w != 32)
							sm &= 0xffffffff << (32 - (w & 31));
						SETREG(lc0->fontmsk, sm);
				
						for (i = 0; i < h; i++) {
							s = *src++;
							*dline = s;
							if (src == srcend)
								src = srcstart;
							dline += 2048;
						}
					} else {
						ew = w & 31;
						em = 0xffffffff << (32 - ew);
						w &= ~31;
				
						if (!sw && !ew)
							SETREG(lc0->fontmsk, 0xffffffff);
		
						for (i = 0; i < h; i++) {
							s = *src++;
							dst = dline;
							if (sw) {
								SETREG(lc0->fontmsk, sm);
								*dst = s;
								dst += 32;
								SETREG(lc0->fontmsk, 0xffffffff);
							} else if (ew)
								SETREG(lc0->fontmsk, 0xffffffff);
							for (j = 0; j < w; j += 32) {
								*dst = s;
								dst += 32;
							}
							if (ew) {
								SETREG(lc0->fontmsk, em);
								*dst = s;
							}
							if (src == srcend)
								src = srcstart;
							dline += 2048;
						}
					}
				}
			}
		}
	}
	
	if (pGC->alu != GXcopy)
		SETREG(ld0->rop, LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW);
	if (pGC->planemask != 0xffffff)
		SETREG(ld0->planemask, 0xffffff);
	SETREG(lc0->addrspace, LEO_ADDRSPC_OBGR);
}

int
LeoCheckTile (PixmapPtr pPixmap, LeoStipplePtr stipple, int ox, int oy)
{
	unsigned int *sbits;
	unsigned int fg = 0, bg = 0;
	int fgset = 0, bgset = 0;
	unsigned int *tilebitsLine, *tilebits, tilebit;
	unsigned int sbit, mask;
	int nbwidth;
	int h, w;
	int x, y;
	int s_y, s_x;

	h = pPixmap->drawable.height;
	if (h > 32 || (h & (h - 1)))
		return FALSE;
	w = pPixmap->drawable.width;
	if (w > 32 || (w & (w - 1)))
		return FALSE;
	stipple->patalign = (oy << 16) | ox;
	sbits = stipple->bits;
	tilebitsLine = (unsigned int *) pPixmap->devPrivate.ptr;
	nbwidth = pPixmap->devKind / sizeof(unsigned int);

	for (y = 0; y < h; y++) {
		tilebits = tilebitsLine;
		tilebitsLine += nbwidth;
		sbit = 0;
		mask = 1 << 31;
		for (x = 0; x < w; x++) {
			tilebit = *tilebits++;
			if (fgset && tilebit == fg)
				sbit |=  mask;
			else if (!bgset || tilebit != bg) {
				if (!fgset) {
					fgset = 1;
					fg = tilebit;
					sbit |= mask;
				} else if (!bgset) {
					bgset = 1;
					bg = tilebit;
				} else
					return FALSE;
			}
			mask >>= 1;
		}
		for (s_x = w; s_x < 32; s_x <<= 1)
			sbit = sbit | (sbit >> s_x);
		sbit = (sbit >> ox) | (sbit << (32 - ox));
		for (s_y = y; s_y < 32; s_y += h)
			sbits[(s_y + oy) & 31] = sbit;
	}
	stipple->fg = fg;
	stipple->bg = bg;
	return TRUE;
}

int
LeoCheckStipple (PixmapPtr pPixmap, LeoStipplePtr stipple, int ox, int oy)
{
	unsigned int *sbits;
	unsigned int *stippleBits;
	unsigned int sbit, mask, nbwidth;
	int h, w;
	int y;
	int s_y, s_x;

	h = pPixmap->drawable.height;
	if (h > 32 || (h & (h - 1)))
		return FALSE;
	w = pPixmap->drawable.width;
	if (w > 32 || (w & (w - 1)))
		return FALSE;
	stipple->patalign = (oy << 16) | ox;
	sbits = stipple->bits;
	stippleBits = (unsigned int *) pPixmap->devPrivate.ptr;
	nbwidth = pPixmap->devKind / sizeof(unsigned int);
	mask = ~0 << (32 - w);
	for (y = 0; y < h; y++) {
		sbit = (*stippleBits) & mask;
		stippleBits += nbwidth;
		for (s_x = w; s_x < 32; s_x <<= 1)
			sbit = sbit | (sbit >> s_x);
		sbit = (sbit >> ox) | (sbit << (32 - ox));
		for (s_y = y; s_y < 32; s_y += h)
			sbits[(s_y + oy) & 31] = sbit;
	}
	return TRUE;
}

int
LeoCheckFill (GCPtr pGC, DrawablePtr pDrawable)
{
	LeoPrivGCPtr gcPriv = LeoGetGCPrivate (pGC);
	LeoPtr pLeo = LeoGetScreenPrivate(pDrawable->pScreen);
	LeoStipplePtr stipple;
	unsigned int alu;
	int xrot, yrot;

	if (pGC->fillStyle == FillSolid) {
		if (gcPriv->stipple) {
			xfree (gcPriv->stipple);
			gcPriv->stipple = 0;
		}
		return TRUE;
	}
	if (!(stipple = gcPriv->stipple)) {
		if (!pLeo->tmpStipple) {
			pLeo->tmpStipple = (LeoStipplePtr) xalloc (sizeof *pLeo->tmpStipple);
			if (!pLeo->tmpStipple)
				return FALSE;
		}
		stipple = pLeo->tmpStipple;
	}
	xrot = (pGC->patOrg.x + pDrawable->x) & 31;
	yrot = (pGC->patOrg.y + pDrawable->y) & 31;
	alu = pGC->alu;
	switch (pGC->fillStyle) {
	case FillTiled:
		if (!LeoCheckTile (pGC->tile.pixmap, stipple, xrot, yrot)) {
			if (gcPriv->stipple) {
				xfree (gcPriv->stipple);
				gcPriv->stipple = 0;
			}
			return FALSE;
		}
		break;
	case FillStippled:
		alu |= 0x80;
	case FillOpaqueStippled:
		if (!LeoCheckStipple (pGC->stipple, stipple, xrot, yrot)) {
			if (gcPriv->stipple) {
				xfree (gcPriv->stipple);
				gcPriv->stipple = 0;
			}
			return FALSE;
		}
		stipple->fg = pGC->fgPixel;
		stipple->bg = pGC->bgPixel;
		break;
	}
	stipple->alu = alu;
	gcPriv->stipple = stipple;
	if (stipple == pLeo->tmpStipple)
		pLeo->tmpStipple = 0;
	return TRUE;
}


GCOps *
LeoMatchCommon (GCPtr pGC, cfbPrivGCPtr devPriv)
{
	if (pGC->lineWidth != 0)
		return 0;
	if (pGC->lineStyle != LineSolid)
		return 0;
	if (pGC->fillStyle != FillSolid)
		return 0;
	if (devPriv->rop != GXcopy)
		return 0;
	if (pGC->font &&
		FONTMAXBOUNDS(pGC->font,rightSideBearing) -
		FONTMINBOUNDS(pGC->font,leftSideBearing) <= 32 &&
		FONTMINBOUNDS(pGC->font,characterWidth) >= 0) {
		if (TERMINALFONT(pGC->font))
			if (devPriv->oneRect)
				return &LeoTEOps1Rect;
			else
				return &LeoTEOps;
		else
			if (devPriv->oneRect)
				return &LeoNonTEOps1Rect;
			else
				return &LeoNonTEOps;
	}
	return 0;
}

Bool
LeoCreateGC(GCPtr pGC)
{
	LeoPrivGCPtr gcPriv;

	if (pGC->depth == 1)
		return mfbCreateGC (pGC);
	if (!cfbCreateGC (pGC))
		return FALSE;

	pGC->ops = & LeoNonTEOps;
	pGC->funcs = & LeoGCFuncs;
	gcPriv = LeoGetGCPrivate (pGC);
	gcPriv->type = DRAWABLE_WINDOW;
	gcPriv->stipple = 0;
	return TRUE;
}

void
LeoDestroyGC (GCPtr pGC)
{
	LeoPrivGCPtr gcPriv = LeoGetGCPrivate (pGC);
        
	if (gcPriv->stipple)
		xfree (gcPriv->stipple);
	miDestroyGC (pGC);
}

void
LeoValidateGC(GCPtr pGC, unsigned long changes, DrawablePtr pDrawable)
{
	int mask;
	int index;
	int new_rrop;
	int new_line, new_text, new_fillspans, new_fillarea;
	int new_rotate;
	int xrot, yrot;
	/* flags for changing the proc vector */
	LeoPrivGCPtr gcPriv;
        cfbPrivGCPtr devPriv;
	int oneRect, type;
	LeoPtr pLeo = LeoGetScreenPrivate (pDrawable->pScreen);
	
	gcPriv = LeoGetGCPrivate (pGC);
	type = pDrawable->type;
	if (type != DRAWABLE_WINDOW) {
		if (gcPriv->type == DRAWABLE_WINDOW) {
			extern GCOps cfbNonTEOps;

			miDestroyGCOps (pGC->ops);
			pGC->ops = &cfbNonTEOps;
			changes = (1 << (GCLastBit+1)) - 1;
			pGC->stateChanges = changes;
			gcPriv->type = type;
		}
		cfbValidateGC (pGC, changes, pDrawable);
		return;
	}

	if (gcPriv->type != DRAWABLE_WINDOW) {
		changes = (1 << (GCLastBit+1)) - 1;
		gcPriv->type = DRAWABLE_WINDOW;
	}

	new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
		 pGC->lastWinOrg.y != pDrawable->y;

	pGC->lastWinOrg.x = pDrawable->x;
	pGC->lastWinOrg.y = pDrawable->y;
	devPriv = cfbGetGCPrivate(pGC);

	new_rrop = FALSE;
	new_line = FALSE;
	new_text = FALSE;
	new_fillspans = FALSE;
	new_fillarea = FALSE;

	/*
	 * if the client clip is different or moved OR the subwindowMode has
	 * changed OR the window's clip has changed since the last validation
	 * we need to recompute the composite clip 
	 */

	if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	    (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))) {
		miComputeCompositeClip (pGC, pDrawable);
		oneRect = REGION_NUM_RECTS(cfbGetCompositeClip(pGC)) == 1;
		if (oneRect != devPriv->oneRect)
			new_line = TRUE;
		devPriv->oneRect = oneRect;
	}

	mask = changes;
	while (mask) {
		index = lowbit (mask);
		mask &= ~index;

		/*
		 * this switch acculmulates a list of which procedures might have
		 * to change due to changes in the GC.  in some cases (e.g.
		 * changing one 16 bit tile for another) we might not really need
		 * a change, but the code is being paranoid. this sort of batching
		 * wins if, for example, the alu and the font have been changed,
		 * or any other pair of items that both change the same thing. 
		 */
		switch (index) {
		case GCFunction:
		case GCForeground:
			new_rrop = TRUE;
			break;
		case GCPlaneMask:
			new_rrop = TRUE;
			new_text = TRUE;
			break;
		case GCBackground:
			break;
		case GCLineStyle:
		case GCLineWidth:
			new_line = TRUE;
			break;
		case GCJoinStyle:
		case GCCapStyle:
			break;
		case GCFillStyle:
			new_text = TRUE;
			new_fillspans = TRUE;
			new_line = TRUE;
			new_fillarea = TRUE;
			break;
		case GCFillRule:
			break;
		case GCTile:
			new_fillspans = TRUE;
			new_fillarea = TRUE;
			break;

		case GCStipple:
			if (pGC->stipple) {
			int width = pGC->stipple->drawable.width;
				PixmapPtr nstipple;

				if ((width <= PGSZ) && !(width & (width - 1)) &&
				    (nstipple = cfbCopyPixmap(pGC->stipple))) {
					cfbPadPixmap(nstipple);
					(*pGC->pScreen->DestroyPixmap)(pGC->stipple);
					pGC->stipple = nstipple;
				}
			}
			new_fillspans = TRUE;
			new_fillarea = TRUE;
			break;

		case GCTileStipXOrigin:
			new_rotate = TRUE;
			break;

		case GCTileStipYOrigin:
			new_rotate = TRUE;
			break;

		case GCFont:
			new_text = TRUE;
			break;
		case GCSubwindowMode:
			break;
		case GCGraphicsExposures:
			break;
		case GCClipXOrigin:
			break;
		case GCClipYOrigin:
			break;
		case GCClipMask:
			break;
		case GCDashOffset:
			break;
		case GCDashList:
			break;
		case GCArcMode:
			break;
		default:
			break;
		}
	}

	/*
	 * If the drawable has changed,  ensure suitable
	 * entries are in the proc vector. 
	 */
	if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
		new_fillspans = TRUE;	/* deal with FillSpans later */
	}

	if (new_rotate || new_fillspans) {
		Bool new_pix = FALSE;

		xrot = pGC->patOrg.x + pDrawable->x;
		yrot = pGC->patOrg.y + pDrawable->y;

		LeoCheckFill (pGC, pDrawable);
		
		switch (pGC->fillStyle) {
		case FillTiled:
			if (!pGC->tileIsPixel) {
				int width = pGC->tile.pixmap->drawable.width * PSZ;

				if ((width <= 32) && !(width & (width - 1))) {
					cfbCopyRotatePixmap(pGC->tile.pixmap,
							    &devPriv->pRotatedPixmap,
							    xrot, yrot);
					new_pix = TRUE;
				}
			}
			break;
		}

		if (!new_pix && devPriv->pRotatedPixmap) {
			(*pGC->pScreen->DestroyPixmap)(devPriv->pRotatedPixmap);
			devPriv->pRotatedPixmap = (PixmapPtr) NULL;
		}
	}

	if (new_rrop) {
		int old_rrop;
		
		if (gcPriv->stipple) {
			if (pGC->fillStyle == FillStippled)
				gcPriv->stipple->alu = pGC->alu | 0x80;
			else
				gcPriv->stipple->alu = pGC->alu;
			if (pGC->fillStyle != FillTiled) {
				gcPriv->stipple->fg = pGC->fgPixel;
				gcPriv->stipple->bg = pGC->bgPixel;
			}
                }

		old_rrop = devPriv->rop;
		devPriv->rop = cfbReduceRasterOp (pGC->alu, pGC->fgPixel,
						  pGC->planemask,
						  &devPriv->and, &devPriv->xor);
		if (old_rrop == devPriv->rop)
			new_rrop = FALSE;
		else {
			new_line = TRUE;
			new_text = TRUE;
			new_fillspans = TRUE;
			new_fillarea = TRUE;
		}
	}

	if (new_rrop || new_fillspans || new_text || new_fillarea || new_line) {
		GCOps	*newops;

		if ((newops = LeoMatchCommon (pGC, devPriv)) != NULL) {
			if (pGC->ops->devPrivate.val)
			miDestroyGCOps (pGC->ops);
			pGC->ops = newops;
			new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
		} else {
			if (!pGC->ops->devPrivate.val) {
				pGC->ops = miCreateGCOps (pGC->ops);
				pGC->ops->devPrivate.val = 1;
			}
			pGC->ops->CopyArea = LeoCopyArea;
		}
	}

	/* deal with the changes we've collected */
	if (new_line) {
		pGC->ops->FillPolygon = miFillPolygon;
		if (devPriv->oneRect && pGC->fillStyle == FillSolid) {
			switch (devPriv->rop) {
			case GXcopy:
				pGC->ops->FillPolygon = cfbFillPoly1RectCopy;
				break;
			default:
				pGC->ops->FillPolygon = LeoFillPoly1RectGeneral;
				break;
			}
		}
		if (pGC->lineWidth == 0) {
			if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid)) {
				switch (devPriv->rop) {
				case GXcopy:
					pGC->ops->PolyArc = cfbZeroPolyArcSS8Copy;
					break;
				default:
					pGC->ops->PolyArc = LeoZeroPolyArcSS8General;
					break;
				}
			} else
				pGC->ops->PolyArc = miZeroPolyArc;
		} else
			pGC->ops->PolyArc = miPolyArc;
		pGC->ops->PolySegment = miPolySegment;
		switch (pGC->lineStyle) {
		case LineSolid:
			if(pGC->lineWidth == 0) {
				if (pGC->fillStyle == FillSolid) {
					if (devPriv->oneRect &&
					    ((pDrawable->x >= pGC->pScreen->width - 32768) &&
					    (pDrawable->y >= pGC->pScreen->height - 32768))) {
						pGC->ops->Polylines = cfb8LineSS1Rect;
						pGC->ops->PolySegment = cfb8SegmentSS1Rect;
					} else {
						pGC->ops->Polylines = cfbLineSS;
						pGC->ops->PolySegment = cfbSegmentSS;
					}
				} else
					pGC->ops->Polylines = miZeroLine;
			} else
				pGC->ops->Polylines = miWideLine;
			break;
		case LineOnOffDash:
		case LineDoubleDash:
			if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid) {
				pGC->ops->Polylines = cfbLineSD;
				pGC->ops->PolySegment = cfbSegmentSD;
			} else
				pGC->ops->Polylines = miWideDash;
			break;
		}
	}

	if (new_text && pGC->font) {
		if (FONTMAXBOUNDS(pGC->font,rightSideBearing) -
		    FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
		    FONTMINBOUNDS(pGC->font,characterWidth) < 0) {
			pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
			pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
		} else {
			if (pGC->fillStyle == FillSolid) {
				if (TERMINALFONT (pGC->font))
					pGC->ops->PolyGlyphBlt = LeoPolyTEGlyphBlt;
				else
					pGC->ops->PolyGlyphBlt = LeoPolyGlyphBlt;
			} else
				pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
				
			/* special case ImageGlyphBlt for terminal emulator fonts */
			if (TERMINALFONT (pGC->font))
				pGC->ops->ImageGlyphBlt = LeoTEGlyphBlt;
			else
				pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
		}
	}	

	if (new_fillspans) {
		switch (pGC->fillStyle) {
		case FillSolid:
			pGC->ops->FillSpans = LeoFillSpansSolid;
			break;
		case FillTiled:
			if (devPriv->pRotatedPixmap) {
				if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
					pGC->ops->FillSpans = cfbTile32FSCopy;
				else
					pGC->ops->FillSpans = LeoTile32FSGeneral;
			} else
				pGC->ops->FillSpans = cfbUnnaturalTileFS;
			break;
		case FillStippled:
			pGC->ops->FillSpans = cfbUnnaturalStippleFS;
			break;
		case FillOpaqueStippled:
			pGC->ops->FillSpans = cfbUnnaturalStippleFS;
			break;
		default:
			FatalError("LeoValidateGC: illegal fillStyle\n");
		}
		if (gcPriv->stipple)
			pGC->ops->FillSpans = LeoFillSpansStippled;
	} /* end of new_fillspans */

	if (new_fillarea) {
		pGC->ops->PolyFillRect = miPolyFillRect;
		if (pGC->fillStyle == FillSolid) {
			if (devPriv->oneRect)
				pGC->ops->PolyFillRect = LeoPolyFillRect1Rect;
			else
				pGC->ops->PolyFillRect = LeoPolyFillRect;
		} else if (gcPriv->stipple)
			pGC->ops->PolyFillRect = LeoPolyFillStippledRect;
		else if (pGC->fillStyle == FillTiled)
			pGC->ops->PolyFillRect = cfbPolyFillRect;
		pGC->ops->PolyFillArc = miPolyFillArc;
		if (pGC->fillStyle == FillSolid) {
			switch (devPriv->rop) {
			case GXcopy:
				pGC->ops->PolyFillArc = cfbPolyFillArcSolidCopy;
				break;
			default:
				pGC->ops->PolyFillArc = LeoPolyFillArcSolidGeneral;
				break;
			}
		}
	}
}

void
LeoCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
	ScreenPtr pScreen = pWin->drawable.pScreen;
	LeoPtr pLeo = LeoGetScreenPrivate (pScreen);
	DDXPointPtr pptSrc;
	DDXPointPtr ppt;
	RegionPtr prgnDst;
	BoxPtr pbox;
	int dx, dy;
	int i, nbox;
	WindowPtr pwinRoot;
	extern WindowPtr *WindowTable;

	dx = ptOldOrg.x - pWin->drawable.x;
	dy = ptOldOrg.y - pWin->drawable.y;

	pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

	prgnDst = REGION_CREATE(pWin->drawable.pScreen, NULL, 1);

	REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
	REGION_INTERSECT(pWin->drawable.pScreen, prgnDst, &pWin->borderClip, prgnSrc);

	pbox = REGION_RECTS(prgnDst);
	nbox = REGION_NUM_RECTS(prgnDst);
	if(!(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
		return;
	ppt = pptSrc;

	for (i = nbox; --i >= 0; ppt++, pbox++) {
		ppt->x = pbox->x1 + dx;
		ppt->y = pbox->y1 + dy;
	}

	LeoDoBitblt ((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
		     GXcopy, prgnDst, pptSrc, ~0L);
	DEALLOCATE_LOCAL(pptSrc);
	REGION_DESTROY(pWin->drawable.pScreen, prgnDst);
}

#ifdef notdef
void LeoVtChange (ScreenPtr pScreen, int enter)
{
	LeoPtr pLeo = LeoGetScreenPrivate (pScreen); 
	LeoCommand0 *lc0 = pLeo->lc0;
	LeoDraw *ld0 = pLeo->ld0;

	ld0->wid = 1;
	ld0->widclip = 0;
	ld0->wmask = 0xffff;
	ld0->planemask = 0xffffff;
	ld0->rop = LEO_ATTR_WE_ENABLE|LEO_ATTR_RGBE_ENABLE|LEO_ATTR_FORCE_WID;
	ld0->fg = 0;
	ld0->vclipmin = 0;
	ld0->vclipmax = (pLeo->psdp->width - 1) | ((pLeo->psdp->height - 1) << 16);
	
	while (lc0->csr & LEO_CSR_BLT_BUSY);
	
	lc0->extent = (pLeo->psdp->width - 1) | ((pLeo->psdp->height - 1) << 11);
	lc0->fill = 0;
	
	while (lc0->csr & LEO_CSR_BLT_BUSY);
	
	lc0->addrspace = LEO_ADDRSPC_OBGR;
	ld0->rop = LEO_ATTR_RGBE_ENABLE|LEO_ROP_NEW;
}
#endif

int
sunZXInit (pScreen, fb)
	ScreenPtr   pScreen;
        fbFd        *fb;
{
	LeoCommand0 *lc0;
	LeoDraw *ld0;
	LeoRec *pLeo;

	lc0 = (LeoCommand0 *) ((char *)fb->fb + LEO_LC0_VOFF);
	ld0 = (LeoDraw *) ((char *)fb->fb + LEO_LD0_VOFF);

	if (serverGeneration != LeoGeneration) {
		LeoScreenPrivateIndex = AllocateScreenPrivateIndex ();
		if (LeoScreenPrivateIndex == -1) return FALSE;
		LeoGCPrivateIndex = AllocateGCPrivateIndex ();
		LeoWindowPrivateIndex = AllocateWindowPrivateIndex ();
		LeoGeneration = serverGeneration;
	}

	if (!AllocateGCPrivate(pScreen, LeoGCPrivateIndex, sizeof(LeoPrivGCRec))) return FALSE;
	if (!AllocateWindowPrivate(pScreen, LeoWindowPrivateIndex, 0)) return FALSE;

	pLeo = (LeoRec *)xalloc(sizeof(LeoRec));
	pScreen->devPrivates[LeoScreenPrivateIndex].ptr = pLeo;

	pLeo->fb = (unsigned *)fb->fb;
	pLeo->lc0 = lc0;
	pLeo->ld0 = ld0;
	pLeo->width = fb->info.fb_width;
	pLeo->height = fb->info.fb_height;
	pLeo->vclipmax = (fb->info.fb_width - 1) | ((fb->info.fb_height - 1) << 16);

	pScreen->CreateGC = LeoCreateGC;
	pScreen->CopyWindow = LeoCopyWindow;

	while (lc0->csr & LEO_CSR_BLT_BUSY);

	SETREG(ld0->wid, 1);
	SETREG(ld0->widclip, 0);
	SETREG(ld0->wmask, 0xffff);
	SETREG(ld0->planemask, 0xffffff);
	SETREG(ld0->rop, LEO_ROP_ZERO|LEO_ATTR_WE_ENABLE|LEO_ATTR_RGBE_ENABLE|LEO_ATTR_FORCE_WID);
	SETREG(ld0->fg, 0);
	SETREG(ld0->vclipmin, 0);
	SETREG(ld0->vclipmax, (fb->info.fb_width - 1) | ((fb->info.fb_height - 1) << 16));

	SETREG(lc0->addrspace, LEO_ADDRSPC_OBGR);
	SETREG(lc0->extent, (fb->info.fb_width - 1) | ((fb->info.fb_height - 1) << 11));
	SETREG(lc0->fill, 0);

	while (lc0->csr & LEO_CSR_BLT_BUSY);

	return TRUE;
}

static void LeoUpdateColormap(pScreen, index, count, rmap, gmap, bmap)
    ScreenPtr	pScreen;
    int		index, count;
    u_char	*rmap, *gmap, *bmap;
{
    struct fbcmap sunCmap;

    sunCmap.index = index;
    sunCmap.count = count;
    sunCmap.red = &rmap[index];
    sunCmap.green = &gmap[index];
    sunCmap.blue = &bmap[index];

    if (ioctl(sunFbs[pScreen->myNum].fd, FBIOPUTCMAP, &sunCmap) == -1)
	FatalError( "LeoUpdateColormap: FBIOPUTCMAP failed\n");
}

static void LeoStoreColors (pmap, ndef, pdefs)
    ColormapPtr pmap;
    int ndef;
    xColorItem* pdefs;
{
  struct fbcmap cmap;
  u_char rmap[256], gmap[256], bmap[256];
  SetupScreen (pmap->pScreen);
  VisualPtr pVisual = pmap->pVisual;
  int i;

  if (pPrivate->installedMap != NULL && pPrivate->installedMap != pmap)
    return;
  for (i = 0; i < 256; i++) {
    rmap[i] = pmap->red[i].co.local.red >> 8;
    gmap[i] = pmap->green[i].co.local.green >> 8;
    bmap[i] = pmap->blue[i].co.local.blue >> 8;
  }
  while (ndef--) {
    i = pdefs->pixel;
    if (pdefs->flags & DoRed)
      rmap[(i & pVisual->redMask) >> pVisual->offsetRed] = (pdefs->red >> 8);
    if (pdefs->flags & DoGreen)
      gmap[(i & pVisual->greenMask) >> pVisual->offsetGreen] = (pdefs->green >> 8);
    if (pdefs->flags & DoBlue)
      bmap[(i & pVisual->blueMask) >> pVisual->offsetBlue] = (pdefs->blue >> 8);
    pdefs++;
  }
  LeoUpdateColormap (pmap->pScreen, 0, 256, rmap, gmap, bmap);
}

static void LeoScreenInit (pScreen)
    ScreenPtr pScreen;
{
#ifndef STATIC_COLOR
    SetupScreen (pScreen);
#endif

#ifndef STATIC_COLOR
    pScreen->InstallColormap = sunInstallColormap;
    pScreen->UninstallColormap = sunUninstallColormap;
    pScreen->ListInstalledColormaps = sunListInstalledColormaps;
    pScreen->StoreColors = LeoStoreColors;
    pPrivate->UpdateColormap = LeoUpdateColormap;
    if (sunFlipPixels) {
	Pixel pixel = pScreen->whitePixel;
	pScreen->whitePixel = pScreen->blackPixel;
	pScreen->blackPixel = pixel;
    }
#endif
}

Bool sunLEOInit (screen, pScreen, argc, argv)
    int		    screen;    	/* what screen am I going to be */
    ScreenPtr	    pScreen;  	/* The Screen to initialize */
    int		    argc;    	/* The number of the Server's arguments. */
    char	    **argv;   	/* The arguments themselves. Don't change! */
{
    pointer	fb;

    sunFbs[screen].EnterLeave = (void (*)())NoopDDA;

    if (!sunScreenAllocate (pScreen))
	return FALSE;

    if (!sunFbs[screen].fb) {
	if ((fb = sunMemoryMap (0x803000, 0, sunFbs[screen].fd)) == NULL)
	    return FALSE;
	sunFbs[screen].fb = fb;
    }

    if (!sunZXInit (pScreen, &sunFbs[screen]))
        return FALSE;

    if (!cfb32ScreenInit(pScreen, fb,
	    sunFbs[screen].info.fb_width,
	    sunFbs[screen].info.fb_height,
	    monitorResolution, monitorResolution,
	    2048))
	    return FALSE;

    LeoScreenInit(pScreen);
    if (!sunScreenInit(pScreen))
	return FALSE;
    sunSaveScreen(pScreen, SCREEN_SAVER_OFF);
    return (cfbCreateDefColormap(pScreen));
}

#endif	/* __NetBSD__ */
