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
/* $XFree86: xc/programs/Xserver/hw/xwin/winshadgdi.c,v 1.6 2001/05/14 16:52:33 alanh Exp $ */

#include "win.h"

static
Bool
winQueryScreenDIBFormat (ScreenPtr pScreen, BITMAPINFOHEADER *pbmih)
{
  winScreenPriv(pScreen);
  HBITMAP		hbmp;
#if CYGDEBUG
  LPDWORD		pdw = NULL;
#endif
  
  /* Create a memory bitmap compatible with the screen */
  hbmp = CreateCompatibleBitmap (pScreenPriv->hdcScreen, 1, 1);
  if (hbmp == NULL)
    {
      ErrorF ("winQueryScreenDIBFormat () - CreateCompatibleBitmap failed\n");
      return FALSE;
    }
  
  /* Initialize our bitmap info header */
  ZeroMemory (pbmih, sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD));
  pbmih->biSize = sizeof (BITMAPINFOHEADER);

  /* Get the biBitCount */
  if (!GetDIBits (pScreenPriv->hdcScreen,
		  hbmp,
		  0, 1,
		  NULL,
		  (BITMAPINFO*) pbmih,
		  DIB_RGB_COLORS))
    {
      ErrorF ("winQueryScreenDIBFormat () - First call to GetDIBits failed\n");
      DeleteObject (hbmp);
      return FALSE;
    }

#if CYGDEBUG
  /* Get a pointer to bitfields */
  pdw = (DWORD*) ((CARD8*)pbmih + sizeof (BITMAPINFOHEADER));

  ErrorF ("winQueryScreenDIBFormat () - First call masks: %08x %08x %08x\n",
	  pdw[0], pdw[1], pdw[2]);
#endif

  /* Get optimal color table, or the optimal bitfields */
  if (!GetDIBits (pScreenPriv->hdcScreen,
		  hbmp,
		  0, 1,
		  NULL,
		  (BITMAPINFO*)pbmih,
		  DIB_RGB_COLORS))
    {
      ErrorF ("winQueryScreenDIBFormat () - Second call to GetDIBits "\
	      "failed\n");
      DeleteObject (hbmp);
      return FALSE;
    }

  /* Free memory */
  DeleteObject (hbmp);
  
  return TRUE;
}

static
Bool
winQueryRGBBitsAndMasks (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  BITMAPINFOHEADER	*pbmih = NULL;
  Bool			fReturn = TRUE;
  LPDWORD		pdw = NULL;
  DWORD			dwRedBits, dwGreenBits, dwBlueBits;

  /* RGB BPP for 8 bit palletes is always 8 bits per pixel */
  if (GetDeviceCaps (pScreenPriv->hdcScreen, RASTERCAPS) & RC_PALETTE)
    {
      /*
       * FIXME: 8bpp doesn't work.
       */
      pScreenPriv->dwBitsPerRGB = 8;
      pScreenPriv->dwRedMask = 0x0L;
      pScreenPriv->dwGreenMask = 0x0L;
      pScreenPriv->dwBlueMask = 0x0L;
      return TRUE;
    }

  /* Color masks for 24 bpp are standardized */
  if (GetDeviceCaps (pScreenPriv->hdcScreen, PLANES)
      * GetDeviceCaps (pScreenPriv->hdcScreen, BITSPIXEL) == 24)
    {
      /* 8 bits per primary color */
      pScreenPriv->dwBitsPerRGB = 8;

      /* Set screen privates masks */
      pScreenPriv->dwRedMask = WIN_24BPP_MASK_RED;
      pScreenPriv->dwGreenMask = WIN_24BPP_MASK_GREEN;
      pScreenPriv->dwBlueMask = WIN_24BPP_MASK_BLUE;
      
      return TRUE;
    }

  /* Allocate a bitmap header and color table */
  pbmih = (BITMAPINFOHEADER*) xalloc (sizeof (BITMAPINFOHEADER)
				      + 256  * sizeof (RGBQUAD));
  if (pbmih == NULL)
    {
      ErrorF ("winQueryRGBBitsAndMasks () - xalloc failed\n");
      return FALSE;
    }

  /* Get screen description */
  if (winQueryScreenDIBFormat (pScreen, pbmih))
    {
      /* Get a pointer to bitfields */
      pdw = (DWORD*) ((CARD8*)pbmih + sizeof (BITMAPINFOHEADER));
      
#if CYGDEBUG
      ErrorF ("winQueryRGBBitsAndMasks () - Masks: %08x %08x %08x\n",
	      pdw[0], pdw[1], pdw[2]);
#endif

      /* Count the number of bits in each mask */
      dwRedBits = winCountBits (pdw[0]);
      dwGreenBits = winCountBits (pdw[1]);
      dwBlueBits = winCountBits (pdw[2]);

      /* Find maximum bits per red, green, blue */
      if (dwRedBits > dwGreenBits && dwRedBits > dwBlueBits)
	pScreenPriv->dwBitsPerRGB = dwRedBits;
      else if (dwGreenBits > dwRedBits && dwGreenBits > dwBlueBits)
	pScreenPriv->dwBitsPerRGB = dwGreenBits;
      else
	pScreenPriv->dwBitsPerRGB = dwBlueBits;

      /* Set screen privates masks */
      pScreenPriv->dwRedMask = pdw[0];
      pScreenPriv->dwGreenMask = pdw[1];
      pScreenPriv->dwBlueMask = pdw[2];
    }
  else
    {
      ErrorF ("winQueryRGBBitsAndMasks () - winQueryScreenDIBFormat failed\n");
      xfree (pbmih);
      fReturn = FALSE;
    }

  /* Free memory */
  xfree (pbmih);

  return fReturn;
}

/* Allocate a DIB for the shadow framebuffer GDI server */
Bool
winAllocateFBShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  BITMAPINFOHEADER	*pbmih = NULL;
  DIBSECTION		dibsection;
  Bool			fReturn = TRUE;

  /* Get device contexts for the screen and shadow bitmap */
  pScreenPriv->hdcScreen = GetDC (pScreenPriv->hwndScreen);
  pScreenPriv->hdcShadow = CreateCompatibleDC (pScreenPriv->hdcScreen);

  /* Allocate bitmap info header */
  pbmih = (BITMAPINFOHEADER*) xalloc (sizeof (BITMAPINFOHEADER)
				      + 256 * sizeof (RGBQUAD));
  if (pbmih == NULL)
    {
      ErrorF ("winAllocateFBShadowGDI - xalloc () failed\n");
      return FALSE;
    }

  /* Query the screen format */
  fReturn = winQueryScreenDIBFormat (pScreen, pbmih);

  /* Describe shadow bitmap to be created */
  pbmih->biWidth = pScreenInfo->dwWidth;
  pbmih->biHeight = -pScreenInfo->dwHeight;
  
  /* Create a DI shadow bitmap with a bit pointer */
  pScreenPriv->hbmpShadow = CreateDIBSection (pScreenPriv->hdcScreen,
					      (BITMAPINFO *) pbmih,
					      0,
					      (VOID**) &pScreenInfo->pfb,
					      NULL,
					      0);
  if (pScreenPriv->hbmpShadow == NULL || pScreenInfo->pfb == NULL)
    {
      ErrorF ("winAllocateFBShadowGDI () - CreateDIBSection failed\n");
      return FALSE;
    }
  else
    {
#if CYGDEBUG
      ErrorF ("winAllocateFBShadowGDI () - Shadow buffer allocated\n");
#endif
    }

  /* Get information about the bitmap that was allocated */
  GetObject (pScreenPriv->hbmpShadow, sizeof (dibsection),
	     &dibsection);

#if CYGDEBUG
  /* Print information about bitmap allocated */
  ErrorF ("winAllocateFBShadowGDI () - Dibsection width: %d height: %d\n",
	  dibsection.dsBmih.biWidth, dibsection.dsBmih.biHeight);
#endif

  /* Select the shadow bitmap into the shadow DC */
  SelectObject (pScreenPriv->hdcShadow,
		pScreenPriv->hbmpShadow);

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowGDI () - Attempting a shadow blit\n");
#endif

  fReturn = BitBlt (pScreenPriv->hdcScreen,
		    0, 0,
		    pScreenInfo->dwWidth, pScreenInfo->dwHeight,
		    pScreenPriv->hdcShadow,
		    0, 0,
		    SRCCOPY);
  if (fReturn)
    {
#if CYGDEBUG
      ErrorF ("winAllocateFBShadowGDI () - Shadow blit success\n");
#endif
    }
  else
    {
      ErrorF ("winAllocateFBShadowGDI () - Shadow blit failure\n");
      return FALSE;
    }

  /* Set screeninfo stride */
  pScreenInfo->dwStrideBytes = pScreenInfo->dwPaddedWidth;
  pScreenInfo->dwStride = (pScreenInfo->dwStrideBytes * 8)
    / pScreenInfo->dwDepth;
  
  return fReturn;
}

/* Blit the damaged regions of the shadow fb to the screen */
void
winShadowUpdateGDI (ScreenPtr pScreen, 
		    PixmapPtr pShadow,
		    RegionPtr damage)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  DWORD			dwBox = REGION_NUM_RECTS (damage);
  BoxPtr		pBox = REGION_RECTS (damage);
  int			x, y, w, h;

  /* Return immediately if the app is not active and we are fullscreen */
  if (!pScreenPriv->fActive && pScreenInfo->fFullScreen) return;

  /* Loop through all boxes in the damaged region */
  while (dwBox--)
    {
      /* Calculate x offset, y offset, width, and height for
	 current damage box
      */
      x = pBox->x1;
      y = pBox->y1;
      w = pBox->x2 - pBox->x1;
      h = pBox->y2 - pBox->y1;

      BitBlt (pScreenPriv->hdcScreen,
	      x, y,
	      w, h,
	      pScreenPriv->hdcShadow,
	      x, y,
	      SRCCOPY);

      /* Get a pointer to the next box */
      ++pBox;
    }
}

void *
winShadowSetWindowLinearGDI (ScreenPtr	pScreen,
			     CARD32	dwRow,
			     CARD32	dwOffset,
			     int	mode,
			     CARD32	*pdwSize)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  *pdwSize = pScreenInfo->dwPaddedWidth;
  return (CARD8 *) pScreenInfo->pfb 
    + dwRow * pScreenInfo->dwPaddedWidth + dwOffset;
}

void *
winShadowWindowGDI (ScreenPtr	pScreen,
		    CARD32	dwRow,
		    CARD32	dwOffset,
		    int		mode,
		    CARD32	*pdwSize)
{
  return winShadowSetWindowLinearGDI (pScreen, dwRow, dwOffset, mode, pdwSize);
}

/* See Porting Layer Definition - p. 33 */
/* We wrap whatever CloseScreen procedure was specified by fb;
   a pointer to said procedure is stored in our devPrivate
*/
Bool
winCloseScreenShadowGDI (int nIndex, ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  Bool			fReturn;

#if CYGDEBUG
  ErrorF ("winCloseScreenShadowGDI () - Freeing screen resources\n");
#endif

  /* Flag that the screen is closed */
  pScreenPriv->fClosed = TRUE;
  pScreenPriv->fActive = FALSE;

  /* Call the wrapped CloseScreen procedure */
  pScreen->CloseScreen = pScreenPriv->CloseScreen;
  fReturn = (*pScreen->CloseScreen) (nIndex, pScreen);

  /* Delete the window property */
  RemoveProp (pScreenPriv->hwndScreen, WIN_SCR_PROP);

  /* Free the shadow DC; which allows the bitmap to be freed */
  DeleteDC (pScreenPriv->hdcShadow);
  
  /* Free the shadow bitmap */
  DeleteObject (pScreenPriv->hbmpShadow);

  /* Free the screen DC */
  ReleaseDC (pScreenPriv->hwndScreen, pScreenPriv->hdcScreen);

  /* Redisplay the Windows cursor */
  if (!pScreenPriv->fCursor)
      ShowCursor (TRUE);

  /* Kill our window */
  if (pScreenPriv->hwndScreen)
    {
      DestroyWindow (pScreenPriv->hwndScreen);
      pScreenPriv->hwndScreen = NULL;
    }

  /* Kill our screeninfo's pointer to the screen */
  pScreenInfo->pScreen = NULL;

  /* Invalidate the ScreenInfo's fb pointer */
  pScreenInfo->pfb = NULL;

  /* Free the screen privates for this screen */
  xfree ((pointer) pScreenPriv);
 
  return fReturn;
}

Bool
winInitVisualsShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  /* Determine our color masks */
  if (!winQueryRGBBitsAndMasks (pScreen))
    {
      ErrorF ("winInitVisualsShadowGDI () - winQueryRGBBitsAndMasks failed\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winInitVisualsGDI () - Masks: %08x %08x %08x bpRGB: %d\n",
	  pScreenPriv->dwRedMask,
	  pScreenPriv->dwGreenMask,
	  pScreenPriv->dwBlueMask,
	  pScreenPriv->dwBitsPerRGB);
#endif

  /* Create a single visual according to the Windows screen depth */
  switch (pScreenInfo->dwDepth)
    {
    case 32:
    case 24:
    case 16:
    case 15:
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     TrueColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     TrueColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsGDI () - miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;

    case 8:
#if CYGDEBUG
      ErrorF ("winInitVisualsGDI () - Calling miSetVisualTypesAndMasks\n");
#endif /* CYGDEBUG */
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     PseudoColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     PseudoColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsGDI () - miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;
    default:
      ErrorF ("winInitVisualsGDI () - Unknown screen depth\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winInitVisualsShadowGDI () - Returned from "\
	  "miSetVisualTypesAndMasks\n");
#endif

  /* Set DPI info */
  pScreenInfo->dwDPIx = GetDeviceCaps (pScreenPriv->hdcScreen, LOGPIXELSX);
  pScreenInfo->dwDPIy = GetDeviceCaps (pScreenPriv->hdcScreen, LOGPIXELSY);

#if CYGDEBUG
  ErrorF ("winInitVisualsGDI () - Returning\n");
#endif

  return TRUE;
}

/* Adjust the video mode */
Bool
winAdjustVideoModeShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HDC			hdc;
  DWORD			dwDepth;
  
  hdc = GetDC (NULL);

  /* We're in serious trouble if we can't get a DC */
  if (hdc == NULL)
    {
      ErrorF ("winAdjustVideoModeShadowGDI () - GetDC () failed\n");
      return FALSE;
    }

  /* Query GDI for current display depth */
  dwDepth = GetDeviceCaps (hdc, BITSPIXEL);

  /* Is GDI using a depth different than command line parameter? */
  if (dwDepth != pScreenInfo->dwDepth)
    {
      /* Warn user if GDI depth is different than depth specified */
      ErrorF ("winAdjustVideoModeShadowGDI () - Command line depth: %d, "\
	      "using depth: %d\n", pScreenInfo->dwDepth, dwDepth);

      /* We'll use GDI's depth */
      pScreenInfo->dwDepth = dwDepth;
    }
  
  /* Release our DC */
  ReleaseDC (NULL, hdc);

  return TRUE;
}

/* Blt exposed regions to the screen */
Bool
winBltExposedRegionsShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HDC			hdcUpdate;
  PAINTSTRUCT		ps;

  /* BeginPaint gives us an hdc that clips to the invalidated region */
  hdcUpdate = BeginPaint (pScreenPriv->hwndScreen, &ps);

  /* Our BitBlt will be clipped to the invalidated region */
  BitBlt (hdcUpdate,
	  0, 0,
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight,
	  pScreenPriv->hdcShadow,
	  0, 0,
	  SRCCOPY);

  /* EndPaint frees the DC */
  EndPaint (pScreenPriv->hwndScreen, &ps);

  return TRUE;
}

Bool
winActivateAppShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  /*
   * Are we active?
   * Are we fullscreen?
   */
  if (pScreenPriv != NULL
      && pScreenPriv->fActive
      && pScreenInfo->fFullScreen)
    {
      /*
       * Activating, attempt to bring our window 
       * to the top of the display
       */
      ShowWindow (pScreenPriv->hwndScreen, SW_RESTORE);
    }
	  
  /*
   * Are we inactive?
   * Are we fullscreen?
   */
  if (pScreenPriv != NULL
      && !pScreenPriv->fActive
      && pScreenInfo->fFullScreen)
    {
      /*
       * Deactivating, stuff our window onto the
       * task bar.
       */
      ShowWindow (pScreenPriv->hwndScreen, SW_MINIMIZE);
    }

  return TRUE;
}

/* Set engine specific funtions */
Bool
winSetEngineFunctionsShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  
  /* Set our pointers */
  pScreenPriv->pwinAllocateFB = winAllocateFBShadowGDI;
  pScreenPriv->pwinShadowUpdate = winShadowUpdateGDI;
  pScreenPriv->pwinShadowWindow = winShadowWindowGDI;
  pScreenPriv->pwinCloseScreen = winCloseScreenShadowGDI;
  pScreenPriv->pwinInitVisuals = winInitVisualsShadowGDI;
  pScreenPriv->pwinAdjustVideoMode = winAdjustVideoModeShadowGDI;
  if (pScreenInfo->fFullScreen)
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowFullScreen;
  else
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowWindowed;
  pScreenPriv->pwinFinishScreenInit = winFinishScreenInitFB;
  pScreenPriv->pwinBltExposedRegions = winBltExposedRegionsShadowGDI;
  pScreenPriv->pwinActivateApp = winActivateAppShadowGDI;

  return TRUE;
}
