/* $XConsortium: s3gc.c,v 1.5 94/12/27 11:29:42 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3/s3gc.c,v 3.5 1995/01/28 17:02:03 dawes Exp $ */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL AND KEVIN E. MARTIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL DIGITAL OR KEVIN E. MARTIN BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

Modified for the 8514/A by Kevin E. Martin (martin@cs.unc.edu)

*/

/*
 * Modified by Amancio Hasty and Jon Tombs
 * 
 * Id: s3gc.c,v 2.3 1993/08/09 06:17:57 jon Exp jon
 */

/*
 * Modified for the CyberVision 64 by Michael Teske
 */


#include        "amiga.h"

#include        "Xmd.h"
#include        "gcstruct.h"
#include        "scrnintstr.h"
#include        "pixmapstr.h"
#include        "regionstr.h"
#include        "mistruct.h"
#include        "mifillarc.h"
#include        "fontstruct.h"
#include        "dixfontstr.h"
#include        "cfb.h"
#include        "cfbmskbits.h"
#include        "cfb8bit.h"
#include        "fastblt.h"
#include        "mergerop.h"
#include        "amigaCV.h"
#include        "migc.h"

#if PPW != 4
Please use PPW = 4!
#endif                 

void amigaCVValidateGC();
void	amiga8CVValidateGC ();
void  amigaCVDestroyGC ();  

/* externs from amigaCVblt.c */
extern void amigaCVFindOrdering ();
extern RegionPtr amigaCVCopyArea();
extern RegionPtr amigaCVCopyPlane();

/* externs from amigaCVfrect.c */  

extern void amigaCVFillSpans();
extern void amigaCVPolyFillRect();  

/* externs from amigaCVLine.c */

extern void amigaCVLine();


/* externs from amigaCVSeg.c */
extern void amigaCVSegment();

/*externs from amigaCVplypt.c */
extern void amigaCVPolyPoint();

/* externs from xf86 font cache */
extern int xf86PolyText8(), xf86PolyText16();
extern void xf86ImageText8(), xf86ImageText16();

/* externs from s3ss.c*/
extern void s3SetSpans();

GCFuncs	amiga8CVGCFuncs = {
    amiga8CVValidateGC,
    miChangeGC,
    miCopyGC,
    amigaCVDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

GCOps	amiga8CVOps = {
    amigaCVFillSpans,
    s3SetSpans,
    cfbPutImage,
    amigaCVCopyArea,
    amigaCVCopyPlane,
    amigaCVPolyPoint,
    amigaCVLine,
    amigaCVSegment,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    miFillPolygon,
    amigaCVPolyFillRect,
    miPolyFillArc,
#ifndef USE_FONTCACHE
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbImageGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8
#else
   xf86PolyText8,
   xf86PolyText16,
   xf86ImageText8,
   xf86ImageText16, 
   miImageGlyphBlt,
   miPolyGlyphBlt,
   miPushPixels
#endif
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};

/* Thes Ops are used if pWin == NULL */

GCOps	amiga8CVNonTEOps = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfbPutImage,
    amigaCVCopyArea,
    amigaCVCopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    miFillPolygon,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbImageGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};


 GCOps amiga8CVTEOps1Rect =
{
   cfbSolidSpansCopy,
   cfbSetSpans,
   cfbPutImage,
   amigaCVCopyArea,
   amigaCVCopyPlane,
   cfbPolyPoint,
   cfb8LineSS1Rect,
   cfb8SegmentSS1Rect,
   miPolyRectangle,
   cfbZeroPolyArcSS8Copy,
   cfbFillPoly1RectCopy,
   cfbPolyFillRect,
   cfbPolyFillArcSolidCopy,
   miPolyText8,
   miPolyText16,
   miImageText8,
   miImageText16,
   cfbTEGlyphBlt8,
   cfbPolyGlyphBlt8,
   cfbPushPixels8,
   NULL,
};

GCOps	amiga8CVTEOps = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfbPutImage,
    amigaCVCopyArea,
    amigaCVCopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
    cfbZeroPolyArcSS8Copy,
    miFillPolygon,
    amigaCVPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    cfbTEGlyphBlt8,
    cfbPolyGlyphBlt8,
    cfbPushPixels8
#ifdef NEED_LINEHELPER
    ,NULL
#endif
};


GCOps amiga8CVNonTEOps1Rect =
{
   cfbSolidSpansCopy,
   cfbSetSpans,
   cfbPutImage,
   amigaCVCopyArea,
   amigaCVCopyPlane,
   cfbPolyPoint,
   cfbLineSS,
   cfbSegmentSS,
   miPolyRectangle,
#if PPW == 4
   cfbZeroPolyArcSS8Copy,
#else
   miZeroPolyArc,
#endif
   cfbFillPoly1RectCopy,
   amigaCVPolyFillRect,
   cfbPolyFillArcSolidCopy,
   miPolyText8,
   miPolyText16,
   miImageText8,
   miImageText16,
   cfbImageGlyphBlt8,
   cfbPolyGlyphBlt8,
   cfbPushPixels8,
#ifdef NEED_LINEHELPER
   NULL,
#endif
};



#if 0

Bool
s3CreateGC(pGC)
     register GCPtr pGC;
{
   cfbPrivGC *pPriv;

   switch (pGC->depth) {
     case 1:
	return (mfbCreateGC(pGC));
     case PSZ:
	break;
     default:
	ErrorF("cfbCreateGC: unsupported depth: %d\n", pGC->depth);
	return FALSE;
   }
   pGC->clientClip = NULL;
   pGC->clientClipType = CT_NONE;

 /*
  * some of the output primitives aren't really necessary, since they will be
  * filled in ValidateGC because of dix/CreateGC() setting all the change
  * bits.  Others are necessary because although they depend on being a color
  * frame buffer, they don't change
  */

   pGC->ops = &cfbNonTEOps;
   pGC->funcs = &cfbFuncs;

 /* cfb wants to translate before scan conversion */
   pGC->miTranslate = 1;

   pPriv = (cfbPrivGC *) (pGC->devPrivates[cfbGCPrivateIndex].ptr);
   pPriv->rop = pGC->alu;
   pPriv->oneRect = FALSE;
   pGC->fExpose = TRUE;
   pGC->freeCompClip = FALSE;
   pGC->pRotatedPixmap = (PixmapPtr) NULL;
   return TRUE;
}
#endif

/*
 * create a private op array for a gc
 */

static GCOps *
cfbCreateOps(prototype)
     GCOps *prototype;
{
   GCOps *ret;
   extern Bool Must_have_memory;

   /* XXX */ Must_have_memory = TRUE;
   ret = (GCOps *) xalloc(sizeof(GCOps));
   /* XXX */ Must_have_memory = FALSE;
   if (!ret)
      return 0;
   *ret = *prototype;
   ret->devPrivate.val = 1;
   return ret;
}

extern GCOps * amigaCVMatchCommon ();


/*
 * Clipping conventions if the drawable is a window CT_REGION ==>
 * pCompositeClip really is the composite CT_other ==> pCompositeClip is the
 * window clip region if the drawable is a pixmap CT_REGION ==>
 * pCompositeClip is the translated client region clipped to the pixmap
 * boundary CT_other ==> pCompositeClip is the pixmap bounding box
 */

void
amiga8CVValidateGC(pGC, changes, pDrawable)
     register GCPtr pGC;
     Mask  changes;
     DrawablePtr pDrawable;
{
   WindowPtr pWin;
   int   mask;			/* stateChanges */
   int   index;			/* used for stepping through bitfields */
   int   new_rrop;
   int   new_line, new_text, new_fillspans, new_fillarea;
   int   new_rotate;
   int   xrot, yrot;

 /* flags for changing the proc vector */
   cfbPrivGCPtr devPriv;
   int   oneRect;

   new_rotate = pGC->lastWinOrg.x != pDrawable->x ||
      pGC->lastWinOrg.y != pDrawable->y;

   pGC->lastWinOrg.x = pDrawable->x;
   pGC->lastWinOrg.y = pDrawable->y;
   if (pDrawable->type == DRAWABLE_WINDOW) {
      pWin = (WindowPtr) pDrawable;
   } else {
      pWin = (WindowPtr) NULL;
   }

   devPriv = ((cfbPrivGCPtr) (pGC->devPrivates[cfbGCPrivateIndex].ptr));

   new_rrop = FALSE;
   new_line = FALSE;
   new_text = FALSE;
   new_fillspans = FALSE;
   new_fillarea = FALSE;

 /*
  * if the client clip is different or moved OR the subwindowMode has changed
  * OR the window's clip has changed since the last validation we need to
  * recompute the composite clip
  */

   if ((changes & (GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode)) ||
     (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
      ) {
      ScreenPtr pScreen = pGC->pScreen;

      if (pWin) {
	 RegionPtr pregWin;
	 Bool  freeTmpClip, freeCompClip;

	 if (pGC->subWindowMode == IncludeInferiors) {
	    pregWin = NotClippedByChildren(pWin);
	    freeTmpClip = TRUE;
	 } else {
	    pregWin = &pWin->clipList;
	    freeTmpClip = FALSE;
	 }
	 freeCompClip = pGC->freeCompClip;

       /*
        * if there is no client clip, we can get by with just keeping the
        * pointer we got, and remembering whether or not should destroy (or
        * maybe re-use) it later.  this way, we avoid unnecessary copying of
        * regions.  (this wins especially if many clients clip by children
        * and have no client clip.)
        */
	 if (pGC->clientClipType == CT_NONE) {
	    if (freeCompClip)
	       REGION_DESTROY(pScreen, cfbGetCompositeClip(pGC));
	    pGC->pCompositeClip = pregWin;
	    pGC->freeCompClip = freeTmpClip;
	 } else {

	  /*
	   * we need one 'real' region to put into the composite clip. if
	   * pregWin the current composite clip are real, we can get rid of
	   * one. if pregWin is real and the current composite clip isn't,
	   * use pregWin for the composite clip. if the current composite
	   * clip is real and pregWin isn't, use the current composite clip.
	   * if neither is real, create a new region.
	   */

	    REGION_TRANSLATE(pScreen, pGC->clientClip,
					 pDrawable->x + pGC->clipOrg.x,
					 pDrawable->y + pGC->clipOrg.y);

	    if (freeCompClip) {
	       REGION_INTERSECT(pGC->pScreen, cfbGetCompositeClip(pGC),
					   pregWin, pGC->clientClip);
	       if (freeTmpClip)
		  REGION_DESTROY(pScreen, pregWin);
	    } else if (freeTmpClip) {
	       REGION_INTERSECT(pScreen, pregWin, pregWin, pGC->clientClip);
	       pGC->pCompositeClip = pregWin;
	    } else {
	       pGC->pCompositeClip = REGION_CREATE(pScreen, NullBox,
								   0);
	       REGION_INTERSECT(pScreen, cfbGetCompositeClip(pGC),
				      pregWin, pGC->clientClip);
	    }
	    pGC->freeCompClip = TRUE;
	    REGION_TRANSLATE(pScreen, pGC->clientClip,
					 -(pDrawable->x + pGC->clipOrg.x),
					 -(pDrawable->y + pGC->clipOrg.y));

	 }
      }
    /* end of composite clip for a window */
      else {
	 BoxRec pixbounds;

       /* XXX should we translate by drawable.x/y here ? */
	 pixbounds.x1 = 0;
	 pixbounds.y1 = 0;
	 pixbounds.x2 = pDrawable->width;
	 pixbounds.y2 = pDrawable->height;

	 if (pGC->freeCompClip) {
	    REGION_RESET(pScreen, cfbGetCompositeClip(pGC), &pixbounds);
	 } else {
	    pGC->freeCompClip = TRUE;
	    pGC->pCompositeClip = REGION_CREATE(pScreen, &pixbounds, 1);
	 }

	 if (pGC->clientClipType == CT_REGION) {
	    REGION_TRANSLATE(pScreen, cfbGetCompositeClip(pGC),
					 -pGC->clipOrg.x, -pGC->clipOrg.y);
	    REGION_INTERSECT(pScreen, cfbGetCompositeClip(pGC),
				   cfbGetCompositeClip(pGC),
				   pGC->clientClip);
	    REGION_TRANSLATE(pScreen, cfbGetCompositeClip(pGC),
					 pGC->clipOrg.x, pGC->clipOrg.y);
	 }
      }				/* end of composute clip for pixmap */
      oneRect = REGION_NUM_RECTS(cfbGetCompositeClip(pGC)) == 1;
      if (oneRect != devPriv->oneRect)
	 new_line = TRUE;
      devPriv->oneRect = oneRect;
   }
   mask = changes;
   while (mask) {
      index = lowbit(mask);
      mask &= ~index;

    /*
     * this switch acculmulates a list of which procedures might have to
     * change due to changes in the GC.  in some cases (e.g. changing one 16
     * bit tile for another) we might not really need a change, but the code
     * is being paranoid. this sort of batching wins if, for example, the alu
     * and the font have been changed, or any other pair of items that both
     * change the same thing.
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
	      int   width = pGC->stipple->drawable.width;
	      PixmapPtr nstipple;

	      if ((width <= 32) && !(width & (width - 1)) &&
		  (nstipple = cfbCopyPixmap(pGC->stipple))) {
		 cfbPadPixmap(nstipple);
		 cfbDestroyPixmap(pGC->stipple);
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
  * If the drawable has changed,  check its depth & ensure suitable entries
  * are in the proc vector.
  */
   if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS))) {
      new_fillspans = TRUE;	/* deal with FillSpans later */
   }
   if (new_rotate || new_fillspans) {
      Bool  new_pix = FALSE;

      xrot = pGC->patOrg.x + pDrawable->x;
      yrot = pGC->patOrg.y + pDrawable->y;

      switch (pGC->fillStyle) {
	case FillTiled:
	   if (!pGC->tileIsPixel) {
	      int   width = pGC->tile.pixmap->drawable.width * PSZ;

	      if ((width <= 32) && !(width & (width - 1))) {
		 cfbCopyRotatePixmap(pGC->tile.pixmap,
				     &pGC->pRotatedPixmap,
				     xrot, yrot);
		 new_pix = TRUE;
	      }
	   }
	   break;
#if (PPW == 4)
	case FillStippled:
	case FillOpaqueStippled:
	   {
	      int   width = pGC->stipple->drawable.width;

	      if ((width <= 32) && !(width & (width - 1))) {
		 mfbCopyRotatePixmap(pGC->stipple,
				     &pGC->pRotatedPixmap, xrot, yrot);
		 new_pix = TRUE;
	      }
	   }
	   break;
#endif
      }
      if (!new_pix && pGC->pRotatedPixmap) {
	 cfbDestroyPixmap(pGC->pRotatedPixmap);
	 pGC->pRotatedPixmap = (PixmapPtr) NULL;
      }
   }
   if (new_rrop) {
      int   old_rrop;

      old_rrop = devPriv->rop;
      devPriv->rop = cfbReduceRasterOp(pGC->alu, pGC->fgPixel,
				       pGC->planemask,
				       &devPriv->and, &devPriv->xor);
      if (old_rrop == devPriv->rop)
	 new_rrop = FALSE;
      else {
#if PPW ==  4
	 new_line = TRUE;
	 new_text = TRUE;
#endif
	 new_fillspans = TRUE;
	 new_fillarea = TRUE;
      }
   }
   if (pWin && pGC->ops->devPrivate.val != 2) {
      if (pGC->ops->devPrivate.val == 1)
	 miDestroyGCOps(pGC->ops);

      /* XXX */
      pGC->ops = cfbCreateOps(&amiga8CVOps);
      pGC->ops->devPrivate.val = 2;

    /*
     * Make sure that everything is properly initialized the first time
     * through
     */
      new_rrop = new_line = new_text = new_fillspans = new_fillarea = TRUE;
   } else if (!pWin && (new_rrop || new_fillspans || new_text || new_fillarea || new_line)) {
      GCOps *newops;

      if ((newops = amigaCVMatchCommon(pGC, devPriv, 8))) {
	 if (pGC->ops->devPrivate.val)
	    miDestroyGCOps(pGC->ops);
	 pGC->ops = newops;
	 new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
      } else {
	 if (!pGC->ops->devPrivate.val) {
	    pGC->ops = cfbCreateOps(pGC->ops);
	    pGC->ops->devPrivate.val = 1;
	 } else if (pGC->ops->devPrivate.val != 1) {
	    miDestroyGCOps(pGC->ops);
	    pGC->ops = cfbCreateOps(&amiga8CVNonTEOps);
	    pGC->ops->devPrivate.val = 1;
	    new_rrop = new_line = new_text = new_fillspans = new_fillarea = TRUE;
	 }
      }
   }
 /* deal with the changes we've collected */
   if (new_line) {
#if 0
      if (pWin) {
	 if (pGC->lineWidth == 0)
	    pGC->ops->PolyArc = miZeroPolyArc;
	 else
	    pGC->ops->PolyArc = miPolyArc;
	 pGC->ops->PolySegment = miPolySegment;
	 switch (pGC->lineStyle) {
	   case LineSolid:
	      if (pGC->lineWidth == 0) {
		 if (pGC->fillStyle == FillSolid) {
		    pGC->ops->Polylines = s3Line;
		    pGC->ops->PolySegment = s3Segment;
		 } else
		    pGC->ops->Polylines = miZeroLine;
	      } else
		 pGC->ops->Polylines = miWideLine;
	      break;
	   case LineOnOffDash:
	   case LineDoubleDash:
#define USES3DLINE	   
#if defined(USES3DLINE)
	      if (pGC->lineWidth == 0) {
		 if (pGC->fillStyle == FillSolid) {
		    pGC->ops->Polylines = s3Dline;
		    pGC->ops->PolySegment = s3Dsegment;
		 } else
		    pGC->ops->Polylines = miWideDash;
	      } else
		 pGC->ops->Polylines = miWideDash;
#else
	      pGC->ops->Polylines = miWideDash;
#endif
	      break;
	 }

      } else 
#endif /* if pwin, #if 0 */
	{
	 pGC->ops->FillPolygon = miFillPolygon;
	 if (devPriv->oneRect && pGC->fillStyle == FillSolid) {
	    switch (devPriv->rop) {
	      case GXcopy:
		 pGC->ops->FillPolygon = cfbFillPoly1RectCopy;
		 break;
	      default:
		 pGC->ops->FillPolygon = cfbFillPoly1RectGeneral;
		 break;
	    }
	 }
	 if (pGC->lineWidth == 0) {
#if PPW == 4
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid)) {
	       switch (devPriv->rop) {
		 case GXxor:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Xor;
		    break;
		 case GXcopy:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Copy;
		    break;
		 default:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8General;
		    break;
	       }
	    } else
#endif
	       pGC->ops->PolyArc = miZeroPolyArc;
	 } else
	    pGC->ops->PolyArc = miPolyArc;
	 pGC->ops->PolySegment = miPolySegment;
	 switch (pGC->lineStyle) {
	   case LineSolid:
	      if (pGC->lineWidth == 0) {
		 if (pGC->fillStyle == FillSolid) {
		    if (pWin) {
		       pGC->ops->Polylines = amigaCVLine;
		       pGC->ops->PolySegment = amigaCVSegment;
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
   }
   if (new_text && (pGC->font)) {
#if 0
      if (pWin) {
	 pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
	 pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
      } else 
#endif
	{
	 if (FONTMAXBOUNDS(pGC->font, rightSideBearing) -
	     FONTMINBOUNDS(pGC->font, leftSideBearing) > 32 ||
	     FONTMINBOUNDS(pGC->font, characterWidth) < 0) {
	    pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
	    pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
	 } else {
#if PPW == 4
	    if (pGC->fillStyle == FillSolid) {
	       if (devPriv->rop == GXcopy)
		  pGC->ops->PolyGlyphBlt = cfbPolyGlyphBlt8;
	       else
		  pGC->ops->PolyGlyphBlt = cfbPolyGlyphRop8;
	    } else
#endif
	       pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
	  /* special case ImageGlyphBlt for terminal emulator fonts */
	    if (TERMINALFONT(pGC->font) &&
		(pGC->planemask & PMSK) == PMSK
#if PPW == 4
		&& FONTMAXBOUNDS(pGC->font, characterWidth) >= 4
#endif
	       ) {
#if PPW == 4
	       pGC->ops->ImageGlyphBlt = cfbTEGlyphBlt8;
#else
	       pGC->ops->ImageGlyphBlt = cfbTEGlyphBlt;
#endif
	    } else {
#if PPW == 4
	       pGC->ops->ImageGlyphBlt = cfbImageGlyphBlt8;
#else
	       pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
#endif
	    }
	 }
      }
   }
   if (new_fillspans) {
      if (pWin) {
	 switch (pGC->fillStyle) {
	   case FillSolid:
	      pGC->ops->FillSpans = amigaCVFillSpans/*s3SolidFSpans*/;
	      break;
#if 0
	   case FillTiled:
	      pGC->ops->FillSpans = s3TiledFSpans;
	      break;
	   case FillStippled:
	      pGC->ops->FillSpans = s3StipFSpans;
	      break;
	   case FillOpaqueStippled:
	      pGC->ops->FillSpans = s3OStipFSpans;
	      break;
#else
	   case FillTiled:
	      if (pGC->pRotatedPixmap)
	      {
		   if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		       pGC->ops->FillSpans = cfbTile32FSCopy;
		   else
		       pGC->ops->FillSpans = cfbTile32FSGeneral;
	      }
	      else
		  pGC->ops->FillSpans = cfbUnnaturalTileFS;
	      break;
	   case FillStippled:
	      if (pGC->pRotatedPixmap)
		  pGC->ops->FillSpans = cfb8Stipple32FS;
	      else
		  pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	      break;
	   case FillOpaqueStippled:
	       if (pGC->pRotatedPixmap)
		   pGC->ops->FillSpans = cfb8OpaqueStipple32FS;
	       else
		   pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	       break;
#endif
	   default:
	      FatalError("amigaCVValidateGC: illegal fillStyle\n");
	 }
      } else {
	 switch (pGC->fillStyle) {
	   case FillSolid:
	      switch (devPriv->rop) {
		case GXcopy:
		   pGC->ops->FillSpans = cfbSolidSpansCopy;
		   break;
		case GXxor:
		   pGC->ops->FillSpans = cfbSolidSpansXor;
		   break;
		default:
		   pGC->ops->FillSpans = cfbSolidSpansGeneral;
		   break;
	      }
	      break;

	   case FillTiled:
	      if (pGC->pRotatedPixmap) {
		 if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->FillSpans = cfbTile32FSCopy;
		 else
		    pGC->ops->FillSpans = cfbTile32FSGeneral;
	      } else
		 pGC->ops->FillSpans = cfbUnnaturalTileFS;
	      break;
	   case FillStippled:

#if PPW == 4
	      if (pGC->pRotatedPixmap)
		 pGC->ops->FillSpans = cfb8Stipple32FS;
	      else
#endif
		 pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	      break;
	   case FillOpaqueStippled:
#if PPW == 4
	      if (pGC->pRotatedPixmap)
		 pGC->ops->FillSpans = cfb8OpaqueStipple32FS;
	      else
#endif
		 pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	      break;
	   default:
	      FatalError("s3ValidateGC: illegal fillStyle\n");
	 }
      }
   }				/* end of new_fillspans */
   if (new_fillarea) {
      if (pWin) {
	 pGC->ops->PolyFillRect = miPolyFillRect;
#if 0
	 if (pGC->fillStyle == FillSolid)
#endif
	     pGC->ops->PolyFillRect = amigaCVPolyFillRect/*s3PolyFillRect*/;

#if 0
	 else if (pGC->fillStyle == FillTiled) {
	    pGC->ops->PolyFillRect = cfbPolyFillRect;
	 }
#endif
	 pGC->ops->PolyFillArc = miPolyFillArc;
	 pGC->ops->PushPixels = miPushPixels;
	 if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = cfbPushPixels8;
	 if (pGC->fillStyle == FillSolid) {
	    switch (devPriv->rop) {
	      case GXcopy:
		 pGC->ops->PolyFillArc = cfbPolyFillArcSolidCopy;
		 break;
	      default:
		 pGC->ops->PolyFillArc = cfbPolyFillArcSolidGeneral;
		 break;
	    }
	 }

      } else {
#if PPW != 4
	 pGC->ops->PolyFillRect = miPolyFillRect;
	 if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled) {
	    pGC->ops->PolyFillRect = cfbPolyFillRect;
	 }
#endif
#if PPW == 4
	 pGC->ops->PushPixels = mfbPushPixels;
	 if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = cfbPushPixels8;
#endif
	 pGC->ops->PolyFillArc = miPolyFillArc;
	 if (pGC->fillStyle == FillSolid) {
	    switch (devPriv->rop) {
	      case GXcopy:
		 pGC->ops->PolyFillArc = cfbPolyFillArcSolidCopy;
		 break;
	      default:
		 pGC->ops->PolyFillArc = cfbPolyFillArcSolidGeneral;
		 break;
	    }
	 }
      }
   }
}


