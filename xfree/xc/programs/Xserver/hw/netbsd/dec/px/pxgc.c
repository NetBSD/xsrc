/*	$NetBSD: pxgc.c,v 1.3 2011/05/24 23:12:36 jakllsch Exp $	*/

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

#include "Xmd.h"
#include "Xproto.h"
#include "cfb.h"
#include "cfbmap.h"
#include "cfbmskbits.h"
#include "fontstruct.h"
#include "dixfontstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "mibstore.h"
#include "migc.h"

#if 0
/* XXX */
#undef cfbNonTEOps
#undef cfbCreateGC
#undef cfbValidateGC
#endif

void	pxValidateGC(GCPtr, u_long, DrawablePtr);

const int pxRopTable[16] = {
	STAMP_UPDATE_ENABLE | STAMP_METHOD_CLEAR,	/* GXclear */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_AND,		/* GXand */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_ANDREV,	/* GXandReverse */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_COPY,	/* GXcopy */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_ANDINV,	/* GXandInverted */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_NOOP,	/* GXnoop */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_XOR,		/* GXxor */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_OR,		/* GXor */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_NOR,		/* GXnor */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_EQUIV,	/* GXequiv */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_INV,		/* GXinvert */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_ORREV,	/* GXorReverse */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_COPYINV,	/* GXcopyInverted */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_ORINV,	/* GXorInverted */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_NAND,	/* GXnand */
	STAMP_UPDATE_ENABLE | STAMP_METHOD_SET,		/* GXset */
};

GCFuncs pxGCFuncs = {
	pxValidateGC,
	miChangeGC,
	miCopyGC,
	miDestroyGC,
	miChangeClip,
	miDestroyClip,
	miCopyClip,
};

GCOps	pxGCOps = {
	pxFillSpans,		/* px  -   -   */
	pxSetSpans,		/* px  -   -   */
	cfbPutImage,		/* -   -   cfb */
	pxCopyArea,		/* px  -   -   */
	miCopyPlane,		/* -   mi  -   */
	pxPolyPoint,		/* px  -   -   */
	miWideLine,		/* -   mi  -   */
	miPolySegment,		/* px  mi  -   */
	miPolyRectangle,	/* -   mi  -   */
	miPolyArc,		/* -   mi  -   */
	miFillPolygon,		/* -   mi  -   */
	miPolyFillRect,		/* px  mi  -   */
	miPolyFillArc,		/* -   mi  -   */
	miPolyText8,		/* -   mi  -   */
	miPolyText16,		/* -   mi  -   */
	miImageText8,		/* px  mi  -   */
	miImageText16,		/* px  mi  -   */
	miPolyGlyphBlt,		/* px  mi  -   */
	miImageGlyphBlt,	/* px  mi  -   */
	miPushPixels,		/* px  mi  -   */
	{ NULL }
};

Bool
pxCreateGC(GCPtr pGC)
{
	pxPrivGCPtr gcPriv;

	PX_TRACE("pxCreateGC");

	switch (pGC->depth) {
	case 1:
		return mfbCreateGC(pGC);
	case 8:
		if (!cfbCreateGC(pGC))
			return FALSE;
		break;
	case 24:
	case 32:
		if (!cfb32CreateGC(pGC))
			return FALSE;
		break;
	default:
		FatalError("pxCreateGC: invalid depth\n");
		break;
	}

	pGC->ops = &pxGCOps;
	pGC->funcs = &pxGCFuncs;
	gcPriv = pxGetGCPrivate(pGC);
	gcPriv->umet = STAMP_METHOD_COPY | STAMP_UPDATE_ENABLE;
	gcPriv->type = 0xbabed;
	gcPriv->sp = pGC->pScreen->devPrivates[pxScreenPrivateIndex].ptr;

	return TRUE;
}

void
pxValidateGC(GCPtr pGC, u_long changes, DrawablePtr pDrawable)
{
	pxScreenPrivPtr sp;
	int mask, index, maxw, maxh, new_line, new_text, new_fill;
	pxPrivGCPtr gcPriv;
        cfbPrivGCPtr devPriv;
	int oneRect;

	PX_TRACE("pxValidateGC");

	gcPriv = pxGetGCPrivate(pGC);
	sp = gcPriv->sp;

	if (pDrawable->type != DRAWABLE_WINDOW) {
		if (gcPriv->type == DRAWABLE_WINDOW) {
			extern GCOps cfbNonTEOps;
			extern GCOps cfb32NonTEOps;

			miDestroyGCOps(pGC->ops);

			if (sp->bpp == 8)
				pGC->ops = &cfbNonTEOps;
			else
				pGC->ops = &cfb32NonTEOps;

			changes = (1 << (GCLastBit+1)) - 1;
			pGC->stateChanges = changes;
		}

		gcPriv->type = pDrawable->type;

		if (sp->bpp == 8)
			cfbValidateGC(pGC, changes, pDrawable);
		else
			cfb32ValidateGC(pGC, changes, pDrawable);

		/*
		 * CopyArea() may have a WINDOW drawable as the source, so
		 * we always override it.
		 */
		pGC->ops->CopyArea = pxCopyArea;
		return;
	}

	if (gcPriv->type != DRAWABLE_WINDOW) {
		changes = (1 << (GCLastBit+1)) - 1;
		gcPriv->type = DRAWABLE_WINDOW;
		pGC->ops = miCreateGCOps(&pxGCOps);
	}

	pGC->lastWinOrg.x = pDrawable->x;
	pGC->lastWinOrg.y = pDrawable->y;
	devPriv = cfbGetGCPrivate(pGC);

	new_line = FALSE;
	new_text = FALSE;
	new_fill = FALSE;

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
		case GCForeground:
			if (sp->bpp == 8)
				gcPriv->fgPixel = PX_DUPBYTE(pGC->fgPixel);
			else
				gcPriv->fgPixel = pGC->fgPixel;
			new_fill = TRUE;
			break;
		case GCBackground:
			if (sp->bpp == 8)
				gcPriv->bgPixel = PX_DUPBYTE(pGC->bgPixel);
			else
				gcPriv->bgPixel = pGC->bgPixel;
			break;
		case GCFunction:
			gcPriv->umet = pxRopTable[pGC->alu];
			break;
		case GCPlaneMask:
			if (sp->bpp == 8)
				gcPriv->pmask = PX_DUPBYTE(pGC->planemask);
			else
				gcPriv->pmask = pGC->planemask & 0xffffff;
			break;
		case GCLineStyle:
		case GCLineWidth:
			new_line = TRUE;
			break;
		case GCFillStyle:
			new_text = TRUE;
			new_fill = TRUE;
			new_line = TRUE;
			break;
		case GCTile:
			new_fill = TRUE;
			break;
		case GCStipple:
			new_fill = TRUE;
			break;
		case GCFont:
			new_text = TRUE;
			break;
		default:
			break;
		}
	}

	/*
	 * Has the drawable changed?
	 */
	if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
		new_fill = TRUE;
		new_line = TRUE;
		new_text = TRUE;
	}

	/*
	 * Solid tiles become solid fills.  (This isn't an assumption that
	 * we should be making, but since the speed difference is so great,
	 * we do.)
	 */
	if (new_fill) {
		if (pGC->fillStyle == FillTiled && pGC->tileIsPixel) {
			gcPriv->fillStyle = FillSolid;
			if (sp->bpp == 8)
				gcPriv->fgFill = PX_DUPBYTE(pGC->tile.pixel);
			else
				gcPriv->fgFill = pGC->tile.pixel;
		} else {
			gcPriv->fillStyle = pGC->fillStyle;
			gcPriv->fgFill = gcPriv->fgPixel;
		}
	}

	/*
	 * Deal with the changes we've collected.
	 */
	if (new_line) {

		switch (pGC->lineStyle) {
		case LineSolid:
			if(pGC->lineWidth == 0) {
				if (gcPriv->fillStyle == FillSolid) {
					pGC->ops->Polylines = pxPolylines;
					pGC->ops->PolySegment = pxPolySegment;
					pGC->ops->PolyArc = pxZeroPolyArc;
				} else {
					pGC->ops->Polylines = miZeroLine;
					pGC->ops->PolySegment = miPolySegment;
					pGC->ops->PolyArc = miZeroPolyArc;
				}
			} else {
				pGC->ops->Polylines = miWideLine;
				pGC->ops->PolySegment = miPolySegment;
				pGC->ops->PolyArc = miPolyArc;
			}
			break;

		case LineOnOffDash:
		case LineDoubleDash:
			if (pGC->lineWidth == 0) { 
				if (gcPriv->fillStyle == FillSolid) {
					pGC->ops->Polylines = pxPolylinesD;
					pGC->ops->PolySegment = pxPolySegmentD;
				} else {
					pGC->ops->Polylines = miZeroDashLine;
					pGC->ops->PolySegment = miPolySegment;
				}
			} else {
				pGC->ops->Polylines = miWideDash;
				pGC->ops->PolySegment = miPolySegment;
			}
			pGC->ops->PolyArc = miPolyArc;
			break;
		}
	}

	if (new_text && pGC->font) {
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
		pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
		maxw = FONTMAXBOUNDS(pGC->font, rightSideBearing) -
		    FONTMINBOUNDS(pGC->font, leftSideBearing);
		maxh = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);

		if (FONTMINBOUNDS(pGC->font, characterWidth) >= 0) {
			if (maxw <= 16 && maxh <= 16) {
				if (TERMINALFONT(pGC->font)) {
					if (gcPriv->fillStyle == FillSolid)
						pGC->ops->PolyGlyphBlt =
						    pxPolyTEGlyphBlt;
					pGC->ops->ImageGlyphBlt =
					    pxImageTEGlyphBlt;
				} else {
					if (gcPriv->fillStyle == FillSolid)
						pGC->ops->PolyGlyphBlt =
						    pxPolyGlyphBlt;
					pGC->ops->ImageGlyphBlt =
					    pxImageGlyphBlt;
				}
			}
#ifdef notyet
			else {
				if (gcPriv->fillStyle == FillSolid)
					pGC->ops->PolyGlyphBlt =
					    pxSlowPolyGlyphBlt;
				pGC->ops->ImageGlyphBlt =
				    pxSlowImageGlyphBlt;
			}
#endif
		}
	}

	if (new_fill) {
		switch (gcPriv->fillStyle) {
		case FillSolid:
			gcPriv->doFillSpans = pxDoFillSpans;
 			pGC->ops->PolyFillRect = pxPolyFillRect;
			pGC->ops->PushPixels = pxSolidPP;
			pGC->ops->PolyFillArc = pxPolyFillArc;
			break;

		case FillTiled:
			if (pxMaskFromTile(sp, pGC->tile.pixmap, &gcPriv->mask)) {
				pGC->ops->PolyFillRect = pxPolyFillRectSO;
				gcPriv->doFillSpans = pxDoFillSpansS;
			} else {
				pGC->ops->PolyFillRect = miPolyFillRect;
				gcPriv->doFillSpans = pxDoFillSpansT;
			}
			pGC->ops->PushPixels = miPushPixels;
			pGC->ops->PolyFillArc = miPolyFillArc;
			break;

		case FillStippled:
			if (pxMaskFromStipple(sp, pGC->stipple, &gcPriv->mask,
			    pGC->fgPixel, pGC->bgPixel)) {
				pGC->ops->PolyFillRect = pxPolyFillRectS;
				gcPriv->doFillSpans = pxDoFillSpansS;
#ifdef notyet
				pGC->ops->PushPixels = pxPushPixelsS;
#endif
			} else {
				pGC->ops->PolyFillRect = miPolyFillRect;
				gcPriv->doFillSpans = pxDoFillSpansUS;
#ifdef notyet
				pGC->ops->PushPixels = miPushPixels;
#endif
			}
#ifndef notyet
			pGC->ops->PushPixels = miPushPixels;
#endif
			pGC->ops->PolyFillArc = miPolyFillArc;
			break;

		case FillOpaqueStippled:
			if (pxMaskFromStipple(sp, pGC->stipple, &gcPriv->mask,
			    pGC->fgPixel, pGC->bgPixel)) {
				pGC->ops->PolyFillRect = pxPolyFillRectSO;
				gcPriv->doFillSpans = pxDoFillSpansS;
			} else {
				pGC->ops->PolyFillRect = miPolyFillRect;
				gcPriv->doFillSpans = pxDoFillSpansUS;
			}
			pGC->ops->PushPixels = miPushPixels;
			pGC->ops->PolyFillArc = miPolyFillArc;
			break;

		default:
			FatalError("pxValidateGC: illegal fillStyle\n");
		}
	}
}
