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
/* $XFree86: xc/programs/Xserver/hw/xwin/winfillsp.c,v 1.1 2001/04/05 20:13:49 dawes Exp $ */

#include "win.h"

/* See Porting Layer Definition - p. 54 */
void
winFillSpansNativeGDI (DrawablePtr	pDrawable,
		       GCPtr		pGC,
		       int		nSpans,
		       DDXPointPtr	pPoints,
		       int		*pWidths,
		       int		fSorted)
{
#if 0
  int		iIdx = 0, i, iX;
  DDXPointPtr	pPoint = NULL;
  int		*pnWidth = 0;
  PixmapPtr	pPixmap = NULL;
  HDC		hdcStipple;
  HBITMAP	hbmpFilledStipple;
  HBITMAP	hbmpMaskedForeground;
  HBITMAP	hbmpDrawable;
  HBRUSH	hbrushStipple;
  HBRUSH	hBrush;
  DEBUG_FN_NAME("winFillSpans");
  DEBUGVARS;
  //DEBUGPROC_MSG;
  
  fprintf (stderr, "winFillSpans () - pDrawable: %08x\n",
	   pDrawable);

  /* Branch on the fill type */
  switch (pGC->fillStyle)
    {
    case FillSolid:
      BitBlt (g_hdc, pDrawable->width, pDrawable->height,
	      pDrawable->width, pDrawable->height,
	      g_hdcMem, 0, 0, SRCCOPY);
      DEBUG_MSG("Solid fill - original drawable");

      /* Enumerate spans */
      for (iIdx = 0; iIdx < nSpans; ++iIdx)
	{
	  /* Get pointers to the current span location and width */
	  pPoint = pPoints + iIdx;
	  pnWidth = pWidths + iIdx;
	  
	  /* Draw the requested line */
	  MoveToEx (g_hdcMem, pPoint->x, pPoint->y, NULL);
	  LineTo (g_hdcMem, pPoint->x + *pnWidth, pPoint->y);

	  fprintf (stderr,
		   "(%dx%dx%d) from: (%d,%d) to: (%d,%d), color: %08x\n",
		   pDrawable->width, pDrawable->height, pDrawable->depth,
		   pPoint->x, pPoint->y, pPoint->x + *pnWidth, pPoint->y,
		   pGC->fgPixel);
	}

      BitBlt (g_hdc, pDrawable->width * 2, pDrawable->height,
	      pDrawable->width, pDrawable->height,
	      g_hdcMem, 0, 0, SRCCOPY);
      DEBUG_MSG("Solid Fill - Filled");
      break;
    case FillStippled:
      /* TODO: Construct the correct stipple, store it in hdcStipple */
      fprintf (stderr, "winFillSpans () - Stipple bitmap: %08x (%dx%d)\n",
	       pGC->stipple->devPrivate.ptr,
	       pGC->stipple->drawable.width,
	       pGC->stipple->drawable.height);
      SelectObject (hdcStipple, (HBITMAP)pGC->stipple->devPrivate.ptr);
      BitBlt (g_hdc, 0, 0,
	      pGC->stipple->drawable.width,
	      pGC->stipple->drawable.height,
	      hdcStipple,
	      0, 0,
	      SRCCOPY);
      DEBUG_MSG("Blitted original stipple to screen");

      /* Create a memory DC to hold the stipple */
      hdcStipple = CreateCompatibleDC (g_hdc);

      /* Create a destination sized compatible bitmap */
      hbmpFilledStipple = CreateCompatibleBitmap (g_hdc,
						  pDrawable->width,
						  pDrawable->height);
      
      /* Select the stipple bitmap into the stipple DC */
      SelectObject (hdcStipple, hbmpFilledStipple);
      
      /* Set foreground and background to white and black */
      SetTextColor (hdcStipple, RGB(0xFF, 0xFF, 0xFF));
      SetBkColor (hdcStipple, RGB (0x00, 0x00, 0x00));
      
      /* Create a pattern brush from the original stipple */
      hbrushStipple = CreatePatternBrush (pGC->stipple->devPrivate.ptr);
      
      /* Select the original stipple brush into the stipple DC */
      SelectObject (hdcStipple, hbrushStipple);

      /* PatBlt the original stipple to the filled stipple */
      PatBlt (hdcStipple, 0, 0, pDrawable->width, pDrawable->height, PATCOPY);
      BitBlt (g_hdc, pDrawable->width, 0,
	      pDrawable->width, pDrawable->height,
	      hdcStipple, 0, 0, SRCCOPY);
      DEBUG_MSG("Filled a drawable-sized stipple");

      /* Mask out the bits from the drawable that are being preserved;
	 hbmpFilledStipple now contains the preserved original bits */
      BitBlt (hdcStipple, 0, 0, pDrawable->width, pDrawable->height,
	      g_hdcMem, 0, 0, SRCERASE);
      BitBlt (g_hdc, pDrawable->width * 2, 0,
	      pDrawable->width, pDrawable->height,
	      hdcStipple, 0, 0, SRCCOPY);
      DEBUG_MSG("Preserved original bits");

      /* Create a destination sized compatible bitmap to hold
	 the masked foreground color */
      hbmpMaskedForeground = CreateCompatibleBitmap (g_hdc,
						     pDrawable->width,
						     pDrawable->height);

      /* TODO: This code chunk can move to winValidateGC () */
      /* Set the foreground color for the stipple fill */
      if (pGC->fgPixel == 0x1)
	{
	  SetTextColor (g_hdcMem, RGB(0x00, 0x00, 0x00));
	  DEBUG_MSG("Grey fill");
	}
      else if (pGC->fgPixel == 0xFFFF)
	{
	  SetTextColor (g_hdcMem, RGB(0xFF, 0xFF, 0xFF));
	  DEBUG_MSG("White fill");
	}
      else
	{
	  SetTextColor (g_hdcMem, RGB(0x00, 0x00, 0x00));
	  DEBUG_MSG("Black fill");
	}
      SetBkColor (g_hdcMem, RGB(0x00, 0x00, 0x00));

      /* Select the masked foreground bitmap into the default memory DC;
	 this should pop the drawable bitmap out of the default DC */
      hbmpDrawable = SelectObject (g_hdcMem, hbmpMaskedForeground);

      /* Free the original drawable */
      DeleteObject (hbmpDrawable);
      hbmpDrawable = NULL;
  
      /* Select the stipple brush into the default memory DC */
      SelectObject (g_hdcMem, hbrushStipple);

      /* Create the masked foreground bitmap using the original stipple */
      PatBlt (g_hdcMem, 0, 0, pDrawable->width, pDrawable->height, PATCOPY);
      BitBlt (g_hdc, pDrawable->width * 3, 0,
	      pDrawable->width, pDrawable->height,
	      g_hdcMem, 0, 0, SRCCOPY);
      DEBUG_MSG("Masked foreground bitmap");
      
      /* Combine the masked foreground with the masked drawable;
       hbmpFilledStipple will contain the drawable stipple filled
       with the current foreground color */
      BitBlt (hdcStipple, 0, 0, pDrawable->width, pDrawable->height,
	      g_hdcMem, 0, 0, SRCPAINT);
      BitBlt (g_hdc, pDrawable->width * 4, 0,
	      pDrawable->width, pDrawable->height,
	      hdcStipple, 0, 0, SRCCOPY);
      DEBUG_MSG("Completed stipple");

      /* Release the stipple DC */
      DeleteDC (hdcStipple);
      
      /* Pop the stipple pattern brush out of the default mem DC */
      SelectObject (g_hdcMem, GetStockObject (WHITE_BRUSH));

      /* Destroy the original stipple pattern brush */
      DeleteObject (hbrushStipple);

      /* Select the result into the default memory DC */
      SelectObject (g_hdcMem, hbmpFilledStipple);

      /* Free the masked foreground bitmap */
      DeleteObject (hbmpMaskedForeground);
      
      /* Point the drawable to the new bitmap */
      ((PixmapPtr)pDrawable)->devPrivate.ptr = hbmpFilledStipple;
      break;
    case FillOpaqueStippled:
      fprintf (stderr, "\n\nwinFillSpans () - OpaqueStippled\n\n");
      break;
    case FillTiled:
      /* Assumes that the drawable is the screen and the tile has been
	 selected into the default memory DC */
#if 0
      hBrush = CreatePatternBrush (pGC->tile.pixmap->devPrivate.ptr);
      hBrush = SelectObject (g_hdc, hBrush);

      PatBlt (g_hdc, 0, 0, pDrawable->width, pDrawable->height, PATCOPY);

      hBrush = SelectObject (g_hdc, hBrush);
      
      DeleteObject (hBrush);
#endif

      /* Enumerate spans */
      for (iIdx = 0; iIdx < nSpans; ++iIdx)
	{
	  /* Get pointers to the current span location and width */
	  pPoint = pPoints + iIdx;
	  pnWidth = pWidths + iIdx;
	  
	  for (iX = 0; iX < *pnWidth; iX += pGC->tile.pixmap->drawable.width)
	    {
	      /* Blit the tile to the screen,
		 one line at a time */
	      BitBlt (g_hdc,
		      pPoint->x + iX,
		      pPoint->y,
		      pGC->tile.pixmap->drawable.width,
		      1,
		      g_hdcMem,
		      0,
		      pPoint->y % pGC->tile.pixmap->drawable.height,
		      SRCCOPY);
	    }
	}
      break;
    default:
      fprintf (stderr, "winFillSpans () - Unknown fillStyle\n");
      exit (1);
      break;
    }
#endif
}
