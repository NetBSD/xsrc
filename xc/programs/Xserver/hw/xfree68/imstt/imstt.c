/* $XFree86: xc/programs/Xserver/hw/xfree68/imstt/imstt.c,v 1.1.2.3 1999/06/02 07:50:10 hohndel Exp $ */

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "xf86.h"

#ifndef PSZ
#define PSZ 8
#endif

#include "cfb.h"
#include "cfb16.h"
#include "cfb24.h"
#include "cfb32.h"

extern void (*fbdevBitBlt)();
extern ScrnInfoRec fbdevInfoRec;
extern pointer fbdevVirtBase;
extern pointer fbdevRegBase;

extern GCOps cfb32TEOps1Rect, cfb32TEOps, cfb32NonTEOps1Rect, cfb32NonTEOps,
	     cfb24TEOps1Rect, cfb24TEOps, cfb24NonTEOps1Rect, cfb24NonTEOps,
	     cfb16TEOps1Rect, cfb16TEOps, cfb16NonTEOps1Rect, cfb16NonTEOps,
	     cfbTEOps1Rect, cfbTEOps, cfbNonTEOps1Rect, cfbNonTEOps;

static int line_pitch;
static int bytes_per_pixel;

/* IMS TwinTurbo blitter registers and macros */
#define S1SA     0x0000
#define S2SA     0x0004
#define SP       0x0008
#define DSA      0x000c
#define CNT      0x0010
#define DP_OCTL  0x0014
#define CLR      0x0018
#define BI       0x0020
#define MBC      0x0024
#define BLTCTL   0x0028
#define SSTATUS  0x0090

static inline void ttout (volatile unsigned long addr, unsigned int val)
{
	register unsigned long base_addr = (unsigned long)fbdevRegBase;

	asm("stwbrx %0,%1,%2; eieio": : "r"(val), "r"(addr), "r"(base_addr):"memory");
}

static inline unsigned int ttin (volatile unsigned long addr)
{
	register unsigned long base_addr = (unsigned long)fbdevRegBase, val;

	asm("lwbrx %0,%1,%2": "=r"(val):"r"(addr), "r"(base_addr));
	return val;
}

static inline void
ttBitBlt (int Bpp, int lpitch, int xsrc, int ysrc, int xdst, int ydst, int w, int h)
{
	unsigned long srcStart, dstStart;
	unsigned int sp, dp_octl, cnt, bltctl;

	xsrc *= Bpp;
	xdst *= Bpp;
	w *= Bpp;
	w--;
	h--;

	bltctl = 0x05;
	sp = lpitch << 16;
	cnt = h << 16;

	if (ysrc < ydst) {
		ysrc += h;
		ydst += h;
		sp |= -(lpitch) & 0xffff;
		dp_octl = -(lpitch) & 0xffff;
	} else {
		sp |= lpitch;
		dp_octl = lpitch;
	}
	if (xsrc < xdst) {
		xsrc += w;
		xdst += w;
		bltctl |= 0x80;
		cnt |= -(w) & 0xffff;
	} else {
		cnt |= w;
	}
	srcStart = ysrc * lpitch + xsrc;
	dstStart = ydst * lpitch + xdst;

	while(ttin(SSTATUS) & 0x80);
	ttout(S1SA, srcStart);
	ttout(SP, sp);
	ttout(DSA, dstStart);
	ttout(CNT, cnt);
	ttout(DP_OCTL, dp_octl);
	ttout(BLTCTL, bltctl);
	while(ttin(SSTATUS) & 0x80);
	while(ttin(SSTATUS) & 0x40);
}

/*
 * DoBitbltCopy
 *
 * Author: Keith Packard
 * Modifications: Radoslaw Kapitan
 */ 
static void
ttDoBitbltCopy (DrawablePtr pSrc, DrawablePtr pDst, int alu, RegionPtr prgnDst, DDXPointPtr pptSrc, unsigned long planemask)
{
	BoxPtr pbox;
	int nbox;
	BoxPtr pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
	/* temporaries for shuffling rectangles */
	DDXPointPtr pptTmp, pptNew1, pptNew2;
	/* shuffling boxes entails shuffling the
	   source points too */
	int careful;
	register int Bpp, lpitch;

	if (!(pSrc->type == DRAWABLE_WINDOW && pDst->type == DRAWABLE_WINDOW) || !xf86VTSema) {
		switch (pSrc->bitsPerPixel) {
			case 8:
				cfbDoBitbltCopy(pSrc, pDst, alu, prgnDst, pptSrc, planemask);
				break;
			case 16:
				cfb16DoBitbltCopy(pSrc, pDst, alu, prgnDst, pptSrc, planemask);
				break;
#ifdef CONFIG_CFB24
			case 24:
				cfb24DoBitbltCopy(pSrc, pDst, alu, prgnDst, pptSrc, planemask);
				break;
#endif
			case 32:
				cfb32DoBitbltCopy(pSrc, pDst, alu, prgnDst, pptSrc, planemask);
				break;
		}
		return;
	}

	pbox = REGION_RECTS(prgnDst);
	nbox = REGION_NUM_RECTS(prgnDst);

	pboxNew1 = NULL;
	pptNew1 = NULL;
	pboxNew2 = NULL;
	pptNew2 = NULL;

	/* XXX we have to err on the side of safety when both are windows,
	 * because we don't know if IncludeInferiors is being used.
	 */
	careful = ((pSrc == pDst) ||
		  ((pSrc->type == DRAWABLE_WINDOW) &&
		  (pDst->type == DRAWABLE_WINDOW)));

	if (careful && (pptSrc->y < pbox->y1)) {
		if (nbox > 1) {
			/* keep ordering in each band, reverse order of bands */
			pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
			if (!pboxNew1)
				return;
			pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
			if (!pptNew1) {
				DEALLOCATE_LOCAL(pboxNew1);
				return;
			}
			pboxBase = pboxNext = pbox+nbox-1;
			while (pboxBase >= pbox) {
				while ((pboxNext >= pbox)
				    && (pboxBase->y1 == pboxNext->y1))
					pboxNext--;
				pboxTmp = pboxNext+1;
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
	}
 
	if (careful && (pptSrc->x < pbox->x1)) {
		if (nbox > 1) {
			/* reverse order of rects in each band */
			pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
			pptNew2 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
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
				while ((pboxNext < pbox+nbox)
				    && (pboxNext->y1 == pboxBase->y1))
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

	Bpp = bytes_per_pixel;
	lpitch = line_pitch;
	while (nbox--) {
		ttBitBlt(Bpp, lpitch,
			 pptSrc->x, pptSrc->y, pbox->x1, pbox->y1,
			 pbox->x2 - pbox->x1, pbox->y2 - pbox->y1);
		pbox++;
		pptSrc++;
	}

	/* free up stuff */
	if (pboxNew2) {
		DEALLOCATE_LOCAL(pptNew2);
		DEALLOCATE_LOCAL(pboxNew2);
	}
	if (pboxNew1) {
		DEALLOCATE_LOCAL(pptNew1);
		DEALLOCATE_LOCAL(pboxNew1);
	}
}

static RegionPtr
tt8CopyArea (DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GC *pGC,
	      int srcx, int srcy, int width, int height, int dstx, int dsty)
{
	void (*doBitBlt)();

	doBitBlt = ttDoBitbltCopy;
	if (pGC->alu != GXcopy || (pGC->planemask & 0xFF) != 0xFF) {
		doBitBlt = cfbDoBitbltGeneral;
		if ((pGC->planemask & 0xFF) == 0xFF) {
			switch (pGC->alu) {
				case GXxor:
					doBitBlt = cfbDoBitbltXor;
					break;
				case GXor:
					doBitBlt = cfbDoBitbltOr;
					break;
			}
		}
	}

	return cfbBitBlt(pSrcDrawable, pDstDrawable, pGC, 
			 srcx, srcy, width, height, dstx, dsty, doBitBlt, 0L);
}

static RegionPtr
tt16CopyArea (DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GC *pGC,
	      int srcx, int srcy, int width, int height, int dstx, int dsty)
{
	void (*doBitBlt)();

	doBitBlt = ttDoBitbltCopy;
	if (pGC->alu != GXcopy || (pGC->planemask & 0xFFFF) != 0xFFFF) {
		doBitBlt = cfb16DoBitbltGeneral;
		if ((pGC->planemask & 0xFFFF) == 0xFFFF) {
			switch (pGC->alu) {
				case GXxor:
					doBitBlt = cfb16DoBitbltXor;
					break;
				case GXor:
					doBitBlt = cfb16DoBitbltOr;
					break;
			}
		}
	}

	return cfb16BitBlt(pSrcDrawable, pDstDrawable, pGC, 
			   srcx, srcy, width, height, dstx, dsty, doBitBlt, 0L);
}

#ifdef CONFIG_CFB24
static RegionPtr
tt24CopyArea (DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GC *pGC,
	      int srcx, int srcy, int width, int height, int dstx, int dsty)
{
	void (*doBitBlt)();

	doBitBlt = ttDoBitbltCopy;
	if (pGC->alu != GXcopy || (pGC->planemask & 0xFFFFFF) != 0xFFFFFF) {
		doBitBlt = cfb24DoBitbltGeneral;
		if ((pGC->planemask & 0xFFFFFF) == 0xFFFFFF) {
			switch (pGC->alu) {
				case GXxor:
					doBitBlt = cfb24DoBitbltXor;
					break;
				case GXor:
					doBitBlt = cfb24DoBitbltOr;
					break;
			}
		}
	}

	return cfb24BitBlt(pSrcDrawable, pDstDrawable, pGC, 
			   srcx, srcy, width, height, dstx, dsty, doBitBlt, 0L);
}
#endif

static RegionPtr
tt32CopyArea (DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GC *pGC,
	       int srcx, int srcy, int width, int height, int dstx, int dsty)
{
	void (*doBitBlt)();

	doBitBlt = ttDoBitbltCopy;
	if (pGC->alu != GXcopy || (pGC->planemask & 0xFFFFFFFF) != 0xFFFFFFFF) {
		doBitBlt = cfb32DoBitbltGeneral;
		if ((pGC->planemask & 0xFFFFFFFF) == 0xFFFFFFFF) {
			switch (pGC->alu) {
				case GXxor:
					doBitBlt = cfb32DoBitbltXor;
					break;
				case GXor:
					doBitBlt = cfb32DoBitbltOr;
					break;
			}
		}
	}

	return cfb32BitBlt(pSrcDrawable, pDstDrawable, pGC, 
			   srcx, srcy, width, height, dstx, dsty, doBitBlt, 0L);
}

extern WindowPtr *WindowTable;

static void
ttCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
	DDXPointPtr pptSrc;
	register DDXPointPtr ppt;
	RegionRec rgnDst;
	register BoxPtr pbox;
	register int dx, dy;
	register int i, nbox;
	WindowPtr pwinRoot;

	pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

	REGION_INIT(pWin->drawable.pScreen, &rgnDst, NullBox, 0);

	dx = ptOldOrg.x - pWin->drawable.x;
	dy = ptOldOrg.y - pWin->drawable.y;
	REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
	REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst, &pWin->borderClip, prgnSrc);

	pbox = REGION_RECTS(&rgnDst);
	nbox = REGION_NUM_RECTS(&rgnDst);
	if (!nbox || !(pptSrc = (DDXPointPtr) ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec)))) {
		REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
		return;
	}
	ppt = pptSrc;

	for (i = nbox; --i >= 0; ppt++, pbox++) {
		ppt->x = pbox->x1 + dx;
		ppt->y = pbox->y1 + dy;
	}

	ttDoBitbltCopy((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot, GXcopy, &rgnDst, pptSrc, ~0L);
	DEALLOCATE_LOCAL(pptSrc);
	REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
}

static void
ttTileFillRect (DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle *prectInit)
{
	switch (pDrawable->bitsPerPixel) {
		case 8:
			cfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			break;
		case 16:
			if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
				cfb16PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			else
				miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			break;
#ifdef CONFIG_CFB24
		case 24:
			if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
				cfb24PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			else
				miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			break;
#endif
		case 32:
			if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
				cfb32PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			else
				miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
			break;
	}
}

static void
ttPolyFillRect (DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle *prectInit)
{
	if (pDrawable->type != DRAWABLE_WINDOW || !xf86VTSema) {
		switch (pDrawable->bitsPerPixel) {
			case 8:
				cfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				break;
			case 16:
				if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
					cfb16PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				else
					miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				break;
#ifdef CONFIG_CFB24
			case 24:
				if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
					cfb24PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				else
					miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				break;
#endif
			case 32:
				if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
					cfb32PolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				else
					miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
				break;
		}
		return;
	}

	/*
	 * Optimize for solid fills
	 */
	if ((pGC->fillStyle == FillSolid)
	    || ((pGC->fillStyle == FillOpaqueStippled)
		&& (pGC->fgPixel == pGC->bgPixel))) {
		int    nClipRects;     /* Number of clipping rectangles */
		BoxPtr pClipRects;     /* Current clipping box */
		int    clipXMin;       /* Upper left corner of clip rect */
		int    clipYMin;       /* Upper left corner of clip rect */
		int    clipXMax;       /* Lower right corner of clip rect */
		int    clipYMax;       /* Lower right corner of clip rect */
		int    drawableXOrg;   /* Drawables x origin */
		int    drawableYOrg;   /* Drawables y origin */
		cfbPrivGC *priv;
		int lpitch, Bpp, clr;

		drawableXOrg = pDrawable->x;
		drawableYOrg = pDrawable->y;

		priv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
		nClipRects = REGION_NUM_RECTS(priv->pCompositeClip);
		pClipRects = REGION_RECTS(priv->pCompositeClip);

		Bpp = bytes_per_pixel;
		lpitch = line_pitch;
		clr = pGC->fgPixel;
		switch(Bpp) {
			case 1:
				clr = clr | (clr << 8) | (clr << 16) | (clr << 24);
				break;
			case 2:
				clr = clr | (clr << 16);
				break;
			/* case 3: case 4: */
			default:
				clr = clr;
				break;
		}
		*((unsigned int *)((unsigned long)fbdevRegBase + CLR)) = clr;
		while(ttin(SSTATUS) & 0x80);
		ttout(DP_OCTL, lpitch);
		ttout(SP, lpitch);
		ttout(BI, 0xffffffff);
		ttout(MBC, 0xffffffff);
		while(ttin(SSTATUS) & 0x80);
		while(ttin(SSTATUS) & 0x40);
#if 0
		regw(DP_MIX, (mach64alu[pGC->alu] << 16) | MIX_DST);
        	regw(DP_WRITE_MASK, pGC->planemask);
#endif

		for (; nClipRects > 0; nClipRects--, pClipRects++) {
			xRectangle *pRect;   /* current rectangle to fill */
			int         nRects;  /* Number of rectangles to fill */

			clipYMin = pClipRects->y1;
			clipYMax = pClipRects->y2;
			clipXMin = pClipRects->x1;
			clipXMax = pClipRects->x2;

			for (nRects = nrectFill, pRect = prectInit;
			     nRects > 0;
			     nRects--, pRect++) {
				int rectX1;     /* points used to find the width */
				int rectX2;     /* points used to find the width */
				int rectY1;     /* points used to find the height */
				int rectY2;     /* points used to find the height */
				int rectWidth;  /* Width of the rect to be drawn */
				int rectHeight; /* Height of the rect to be drawn */

				/*
				 * Clip and translate each rect
				 */
				rectX1 = max((pRect->x + drawableXOrg), clipXMin);
				rectY1 = max((pRect->y + drawableYOrg), clipYMin);
				rectX2 = min((pRect->x + pRect->width + drawableXOrg), clipXMax);
				rectY2 = min((pRect->y + pRect->height + drawableYOrg), clipYMax);

				rectWidth = rectX2 - rectX1;
				rectHeight = rectY2 - rectY1;

				if ((rectWidth > 0) && (rectHeight > 0)) {
					while(ttin(SSTATUS) & 0x80);
					ttout(DSA, rectY1 * lpitch + rectX1 * Bpp);
					ttout(S1SA, rectY1 * lpitch + rectX1 * Bpp);
					ttout(CNT, ((rectHeight - 1) << 16) | (rectWidth * Bpp - 1));
					ttout(BLTCTL, 0x840); /* 0x200000 */
					while(ttin(SSTATUS) & 0x80);
					while(ttin(SSTATUS) & 0x40);
				}
			} /* end for loop through each rectangle to draw */
		} /* end for loop through each clip rectangle */

	} else { /* end section to draw solid patterns */
		ttTileFillRect(pDrawable, pGC, nrectFill, prectInit);
	}
}

void
imstt_init (ScreenPtr scr)
{
	int width, Bpp;

	width = scr->width;
	Bpp = scr->rootDepth >> 3;
	switch (Bpp) {
		case 1:
			cfbTEOps.CopyArea = tt8CopyArea;
			cfbNonTEOps.CopyArea = tt8CopyArea;
			cfbTEOps1Rect.CopyArea = tt8CopyArea;
			cfbNonTEOps1Rect.CopyArea = tt8CopyArea;
			cfbTEOps.PolyFillRect = ttPolyFillRect;
			cfbNonTEOps.PolyFillRect = ttPolyFillRect;
			cfbTEOps1Rect.PolyFillRect = ttPolyFillRect;
			cfbNonTEOps1Rect.PolyFillRect = ttPolyFillRect;
			break;
		case 2:
			cfb16TEOps.CopyArea = tt16CopyArea;
			cfb16NonTEOps.CopyArea = tt16CopyArea;
			cfb16TEOps1Rect.CopyArea = tt16CopyArea;
			cfb16NonTEOps1Rect.CopyArea = tt16CopyArea;
			cfb16TEOps.PolyFillRect = ttPolyFillRect;
			cfb16NonTEOps.PolyFillRect = ttPolyFillRect;
			cfb16TEOps1Rect.PolyFillRect = ttPolyFillRect;
			cfb16NonTEOps1Rect.PolyFillRect = ttPolyFillRect;
			break;
#ifdef CONFIG_CFB24
		case 3:
			cfb24TEOps.CopyArea = tt24CopyArea;
			cfb24NonTEOps.CopyArea = tt24CopyArea;
			cfb24TEOps1Rect.CopyArea = tt24CopyArea;
			cfb24NonTEOps1Rect.CopyArea = tt24CopyArea;
			cfb24TEOps.PolyFillRect = ttPolyFillRect;
			cfb24NonTEOps.PolyFillRect = ttPolyFillRect;
			cfb24TEOps1Rect.PolyFillRect = ttPolyFillRect;
			cfb24NonTEOps1Rect.PolyFillRect = ttPolyFillRect;
			break;
#endif
		case 4:
			cfb32TEOps.CopyArea = tt32CopyArea;
			cfb32NonTEOps.CopyArea = tt32CopyArea;
			cfb32TEOps1Rect.CopyArea = tt32CopyArea;
			cfb32NonTEOps1Rect.CopyArea = tt32CopyArea;
			cfb32TEOps.PolyFillRect = ttPolyFillRect;
			cfb32NonTEOps.PolyFillRect = ttPolyFillRect;
			cfb32TEOps1Rect.PolyFillRect = ttPolyFillRect;
			cfb32NonTEOps1Rect.PolyFillRect = ttPolyFillRect;
			break;
	}

	bytes_per_pixel = Bpp;
	line_pitch = width * Bpp;

	fbdevBitBlt = ttDoBitbltCopy;
	scr->CopyWindow = ttCopyWindow;
}
