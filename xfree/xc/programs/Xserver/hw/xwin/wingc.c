/*
 *Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 *
 *Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 *"Software"), to deal in the Software without restriction, including
 *without limitation the rights to use, copy, modify, merge, publish,
 *distribute, sublicense, and/or sell copies of the Software, and to
 *permit persons to whom the Software is furnished to do so, subject to
 *the following conditions:
 *
 *The above copyright notice and this permission notice shall be
 *included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of the XFree86 Project
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from the XFree86 Project.
 *
 * Authors:	Harold L Hunt II
 */
/* $XFree86: xc/programs/Xserver/hw/xwin/wingc.c,v 1.2 2001/04/19 12:56:03 alanh Exp $ */

#include "win.h"

/* GC Handling Routines */
const GCFuncs winGCFuncs = {
  winValidateGCNativeGDI,
  winChangeGCNativeGDI,
  winCopyGCNativeGDI,
  winDestroyGCNativeGDI,
  winChangeClipNativeGDI,
  winDestroyClipNativeGDI,
  winCopyClipNativeGDI,
};

/* Drawing Primitives */
const GCOps winGCOps = {
  winFillSpansNativeGDI,
  winSetSpansNativeGDI,
  miPutImage,
  miCopyArea,
  miCopyPlane,
  miPolyPoint,
  winPolyLineNativeGDI,
  miPolySegment,
  miPolyRectangle,
  miPolyArc,
  miFillPolygon,
  miPolyFillRect,
  miPolyFillArc,
  miPolyText8,
  miPolyText16,
  miImageText8,
  miImageText16,
  miImageGlyphBlt,
  miPolyGlyphBlt,
  miPushPixels
#ifdef NEED_LINEHELPER
  ,NULL
#endif
};

/* See Porting Layer Definition - p. 45 */
/* See mfb/mfbgc.c - mfbCreateGC() */
/* See Strategies for Porting - pp. 15, 16 */
Bool
winCreateGCNativeGDI (GCPtr pGC)
{
  fprintf (stderr, "\nwinCreateGC () depth: %d\n\n",
	   pGC->depth);

  pGC->clientClip = NULL;
  pGC->clientClipType = CT_NONE;

  pGC->ops = (GCOps *) &winGCOps;
  pGC->funcs = (GCFuncs *) &winGCFuncs;

  pGC->miTranslate = 0;

  /*
    winGetRotatedPixmapNativeGDI(pGC) = 0;
    winGetExposeNativeGDI(pGC) = 1;
    winGetFreeCompClipNativeGDI(pGC) = 0;
    winGetCompositeClipNativeGDI(pGC) = 0;
    winGetGCPrivateNativeGDI(pGC)->bpp = BitsPerPixel (pGC->depth);
  */
  return TRUE;
}

/* See Porting Layer Definition - p. 45 */
void
winChangeGCNativeGDI (GCPtr pGC, unsigned long ulChanges)
{

}

/* See Porting Layer Definition - pp. 45-46 */
/* See mfb/mfbgc.c - mfbValidateGC() */
/* See Strategies for Porting - pp. 15, 16 */
void
winValidateGCNativeGDI (GCPtr pGC,
			unsigned long dwChanges,
			DrawablePtr pDrawable)
{
#if 0
  PixmapPtr		pPixmap;
  int			nIndex, iResult;
  unsigned long		dwMask = dwChanges;
  HPEN			hPen;
  HBRUSH		hBrush;
  HBITMAP		hBitmap;
  DEBUG_FN_NAME("winValidateGC");
  DEBUGVARS;
  //DEBUGPROC_MSG;

  fprintf (stderr, "winValidateGC - pDrawable: %08x, pGC: %08x\n",
	   pDrawable, pGC);

  switch (pDrawable->type)
    {
    case DRAWABLE_PIXMAP:
      /* I can handle drawable pixmaps, no problem */
      pPixmap = (PixmapPtr) pDrawable;

      fprintf (stderr, "winValidateGC - pPixmap->devPrivate.ptr: %08x\n",
	       pPixmap->devPrivate.ptr);

      /* Select the bitmap into the memory device context.
	 NOTE: A bitmap can only be selected into a single
	 memory device context at a time.
      */
      SelectObject (g_hdcMem, pPixmap->devPrivate.ptr);

      /* Sync the DC settings with the GC settings */
      switch (pGC->fillStyle)
	{
	case FillSolid:
	  /* Select a stock pen */
	  if (pDrawable->depth == 1 && pGC->fgPixel)
	    {
	      SelectObject (g_hdcMem, GetStockObject (WHITE_PEN));
	    }
	  else if (pDrawable->depth == 1 && !pGC->fgPixel)
	    {
	      SelectObject (g_hdcMem, GetStockObject (BLACK_PEN));
	    }
	  else if (pGC->fgPixel)
	    {
	      SelectObject (g_hdcMem, CreatePen (PS_SOLID, 0, pGC->fgPixel));
	    }
	  else
	    {
	      SelectObject (g_hdcMem, GetStockObject (BLACK_PEN));
	    }
	  break;
	case FillStippled:
	  break;
	default:
	  break;
	}
      
      break;
    case DRAWABLE_WINDOW:
      switch (pGC->fillStyle)
	{
	case FillTiled:
	  /* Need to select the tile into the memory DC */
	  SelectObject (g_hdcMem, pGC->tile.pixmap->devPrivate.ptr);

	  /* Blit the tile to a remote area of the screen */
	  BitBlt (g_hdc, 64, 64,
		  pGC->tile.pixmap->drawable.width,
		  pGC->tile.pixmap->drawable.height,
		  g_hdcMem,
		  0, 0, SRCCOPY);
	  DEBUG_MSG("Blitted the tile to a remote area of the screen");
	  break;
	case FillStippled:
	case FillSolid:
	default:
	  break;
	}
      break;
    case UNDRAWABLE_WINDOW:
      break;
    case DRAWABLE_BUFFER:
      break;
    default:
      break;
    }

#if 0
  /* Inspect changes to the GC */
  while (dwMask)
    {
      /* This peels off one change at a time */
      nIndex = lowbit (dwMask);
      dwMask &= ~nIndex;
      
      switch (nIndex)
	{
	case GCFunction:
	  /* mfb falls through to GCForeground */
	  fprintf (stderr, "winValidateGC - GCFunction\n");
	  break;
	case GCForeground:
	  fprintf (stderr, "winValidateGC - GCForeground\n");
	  break;
	case GCPlaneMask:
	  fprintf (stderr, "winValidateGC - GCPlaneMask\n");
	  break;
	case GCBackground:
	  fprintf (stderr, "winValidateGC - GCBackground\n");
	  break;
	case GCLineStyle:
	case GCLineWidth:
	case GCJoinStyle:
	  fprintf (stderr, "winValidateGC - GCLineStyle, etc.\n");
	  break;
	case GCCapStyle:
	  fprintf (stderr, "winValidateGC - GCCapStyle\n");
	  break;
	case GCFillStyle:
	  fprintf (stderr, "winValidateGC - GCFillStyle\n");
	  break;
	case GCFillRule:
	  fprintf (stderr, "winValidateGC - GCFillRule\n");
	  break;
	case GCTile:
	  fprintf (stderr, "winValidateGC - GCTile\n");
	  break;
	case GCStipple:
	  fprintf (stderr, "winValidateGC - GCStipple\n");
	  break;
	case GCTileStipXOrigin:
	  fprintf (stderr, "winValidateGC - GCTileStipXOrigin\n");
	  break;
	case GCTileStipYOrigin:
	  fprintf (stderr, "winValidateGC - GCTileStipYOrigin\n");
	  break;
	case GCFont:
	  fprintf (stderr, "winValidateGC - GCFont\n");
	  break;
	case GCSubwindowMode:
	  fprintf (stderr, "winValidateGC - GCSubwindowMode\n");
	  break;
	case GCGraphicsExposures:
	  fprintf (stderr, "winValidateGC - GCGraphicsExposures\n");
	  break;
	case GCClipXOrigin:
	  fprintf (stderr, "winValidateGC - GCClipXOrigin\n");
	  break;
	case GCClipYOrigin:
	  fprintf (stderr, "winValidateGC - GCClipYOrigin\n");
	  break;
	case GCClipMask:
	  fprintf (stderr, "winValidateGC - GCClipMask\n");
	  break;
	case GCDashOffset:
	  fprintf (stderr, "winValidateGC - GCDashOffset\n");
	  break;
	case GCDashList:
	  fprintf (stderr, "winValidateGC - GCDashList\n");
	  break;
	case GCArcMode:
	  fprintf (stderr, "winValidateGC - GCArcMode\n");
	  break;
	default:
	  fprintf (stderr, "winValidateGC - default\n");
	  break;
	}
    }
#endif
#endif
}

/* See Porting Layer Definition - p. 46 */
void
winCopyGCNativeGDI (GCPtr pGCsrc, unsigned long ulMask, GCPtr pGCdst)
{

}

/* See Porting Layer Definition - p. 46 */
void
winDestroyGCNativeGDI (GCPtr pGC)
{

}

/* See Porting Layer Definition - p. 46 */
void
winChangeClipNativeGDI (GCPtr pGC, int nType, pointer pValue, int nRects)
{

}

/* See Porting Layer Definition - p. 47 */
void
winDestroyClipNativeGDI (GCPtr pGC)
{

}

/* See Porting Layer Definition - p. 47 */
void
winCopyClipNativeGDI (GCPtr pGCdst, GCPtr pGCsrc)
{

}
