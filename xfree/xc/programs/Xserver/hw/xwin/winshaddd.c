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
 * Authors:	Dakshinamurthy Karra
 *		Suhaib M Siddiqi
 *		Peter Busch
 *		Harold L Hunt II
 */
/* $XFree86: xc/programs/Xserver/hw/xwin/winshaddd.c,v 1.5 2001/05/14 16:52:33 alanh Exp $ */

#include "win.h"

/*
 * Create a DirectDraw surface for the shadow framebuffer; also create
 * a primary surface object so we can blit to the display.
 * 
 * Install a DirectDraw clipper on our primary surface object
 * that clips our blits to the unobscured client area of our display window.
 */
Bool
winAllocateFBShadowDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;  
  HRESULT		ddrval = DD_OK;
  DDSURFACEDESC		ddsd;
  DDSURFACEDESC		*pddsdShadow = NULL;
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD ()\n");
#endif

  /* Create a clipper */
  ddrval = DirectDrawCreateClipper (0,
				    &pScreenPriv->pddcPrimary,
				    NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDD () - Could not create clipper\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Created a clipper\n");
#endif

  /* Attach the clipper to our display window */
  ddrval = IDirectDrawClipper_SetHWnd (pScreenPriv->pddcPrimary,
				       0,
				       pScreenPriv->hwndScreen);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDD () - Clipper not attached to window\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Attached clipper to window\n");
#endif

  /* Create a DirectDraw object, store the address at lpdd */
  ddrval = DirectDrawCreate (NULL, &pScreenPriv->pdd, NULL);
  if (ddrval != DD_OK)
    {
      ErrorF ("winAllocateFBShadowDD () - Could not start DirectDraw\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Created and initialized DD\n");
#endif

  /* FIXME: If we are full screen we don't need the clipper */
  if (pScreenInfo->fFullScreen)
    {
      /* Set the cooperative level to full screen */
      ddrval = IDirectDraw_SetCooperativeLevel (pScreenPriv->pdd,
						pScreenPriv->hwndScreen,
						DDSCL_EXCLUSIVE
						| DDSCL_FULLSCREEN);
      if (FAILED (ddrval))
	{
	  ErrorF ("winAllocateFBShadowDDNL () - Could not set "\
		  "cooperative level\n");
	  return FALSE;
	}

      /* Change the video mode to the mode requested */
      ddrval = IDirectDraw_SetDisplayMode (pScreenPriv->pdd,
					   pScreenInfo->dwWidth,
					   pScreenInfo->dwHeight,
					   pScreenInfo->dwDepth);
       if (FAILED (ddrval))
	 {
	   ErrorF ("winAllocateFBShadowDDNL () - Could not set "\
		   "full screen display mode\n");
	   return FALSE;
	 }
    }
  else
    {
      /* Set the cooperative level for windowed mode */
      ddrval = IDirectDraw_SetCooperativeLevel (pScreenPriv->pdd,
						pScreenPriv->hwndScreen,
						DDSCL_NORMAL);
      if (FAILED (ddrval))
	{
	  ErrorF ("winAllocateFBShadowDDNL () - Could not set "\
		  "cooperative level\n");
	  return FALSE;
	}
    }

  /* Describe the primary surface */
  ZeroMemory (&ddsd, sizeof (ddsd));
  ddsd.dwSize = sizeof (ddsd);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  
  /* Create the primary surface */
  ddrval = IDirectDraw_CreateSurface (pScreenPriv->pdd,
				      &ddsd,
				      &pScreenPriv->pddsPrimary,
				      NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDD () - Could not create primary "\
	      "surface %08x\n", ddrval);
      return FALSE;
    }
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Created primary\n");
#endif

  /*
   * Attach a clipper to the primary surface that will clip our blits to our
   * display window.
   */
  ddrval = IDirectDrawSurface_SetClipper (pScreenPriv->pddsPrimary,
					  pScreenPriv->pddcPrimary);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDD () - Primary attach clipper failed\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Attached clipper to primary surface\n");
#endif

  /* Describe the shadow surface to be created */
  /* NOTE: Do not use a DDSCAPS_VIDEOMEMORY surface,
   * as drawing, locking, and unlocking take forever
   * with video memory surfaces.  In addition,
   * video memory is a somewhat scarce resource,
   * so you shouldn't be allocating video memory when
   * you have the option of using system memory instead.
   */
  ZeroMemory (&ddsd, sizeof (ddsd));
  ddsd.dwSize = sizeof (ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  ddsd.dwHeight = pScreenInfo->dwHeight;
  ddsd.dwWidth = pScreenInfo->dwWidth;

  /* Create the shadow surface */
  ddrval = IDirectDraw_CreateSurface (pScreenPriv->pdd,
				      &ddsd,
				      &pScreenPriv->pddsShadow,
				      NULL);
  if (ddrval != DD_OK)
    {
      ErrorF ("winAllocateFBShadowDD () - Could not create shadow "\
	      "surface: %08x\n", ddrval);
      return FALSE;
    }
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Created shadow\n");
#endif

  /* Allocate a DD surface description for our screen privates */
  pddsdShadow = pScreenPriv->pddsdShadow = xalloc (sizeof (DDSURFACEDESC));
  if (pddsdShadow == NULL)
    {
      ErrorF ("winAllocateFBShadowDD () - Could not allocate surface "\
	      "description memory\n");
      return FALSE;
    }
  ZeroMemory (pddsdShadow, sizeof (*pddsdShadow));
  pddsdShadow->dwSize = sizeof (*pddsdShadow);

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Locking shadow\n");
#endif

  /* Lock the shadow surface */
  ddrval = IDirectDrawSurface_Lock (pScreenPriv->pddsShadow,
				    NULL,
				    pddsdShadow,
				    DDLOCK_WAIT,
				    NULL);
  if (ddrval != DD_OK || pddsdShadow->lpSurface == NULL)
    {
      ErrorF ("winAllocateFBShadowDD () - Could not lock shadow "\
	      "surface: %08x\n", ddrval);
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Locked shadow\n");
#endif

  /* We don't know how to deal with anything other than RGB */
  if (!(pddsdShadow->ddpfPixelFormat.dwFlags & DDPF_RGB))
    {
      ErrorF ("winAllocateFBShadowDD () - Color format other than RGB\n");
      return FALSE;
    }

  /* Grab the pitch, and memory pointer from the surface desc */
  pScreenInfo->dwStrideBytes = pddsdShadow->u.lPitch;
  pScreenInfo->dwStride = (pScreenInfo->dwStrideBytes * 8)
    / pScreenInfo->dwDepth;
  pScreenInfo->pfb = pddsdShadow->lpSurface;
  
  /* Grab the color depth and masks from the surface description */
  pScreenInfo->dwDepth = pddsdShadow->ddpfPixelFormat.u.dwRGBBitCount;
  pScreenPriv->dwRedMask = pddsdShadow->ddpfPixelFormat.u2.dwRBitMask;
  pScreenPriv->dwGreenMask = pddsdShadow->ddpfPixelFormat.u3.dwGBitMask;
  pScreenPriv->dwBlueMask = pddsdShadow->ddpfPixelFormat.u4.dwBBitMask;
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDD () - Returning\n");
#endif

  return TRUE;
}

/*
 * Transfer the damaged regions of the shadow framebuffer to the display.
 */
void
winShadowUpdateDD (ScreenPtr pScreen, 
		   PixmapPtr pShadow,
		   RegionPtr damage)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HRESULT		ddrval = DD_OK;
  RECT			rcClient, rcDest, rcSrc;
  DWORD			dwBox = REGION_NUM_RECTS (damage);
  BoxPtr		pBox = REGION_RECTS (damage);

  /* Return immediately if the app is not active and we are fullscreen */
  if (!pScreenPriv->fActive && pScreenInfo->fFullScreen) return;

  /* Get location of display window's client area, in screen coords */
  GetClientRect (pScreenPriv->hwndScreen, &rcClient);
  MapWindowPoints (pScreenPriv->hwndScreen,
		   HWND_DESKTOP,
		   (LPPOINT)&rcClient, 2);

  /* Unlock the shadow surface, so we can blit */
  ddrval = IDirectDrawSurface_Unlock (pScreenPriv->pddsShadow, NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winShadowUpdateProcDD () - Unlock failed\n");
      return;
    }

  /* Loop through all boxes in the damaged region */
  while (dwBox--)
    {
      /* Assign damage box to source rectangle */
      rcSrc.left = pBox->x1;
      rcSrc.top = pBox->y1;
      rcSrc.right = pBox->x2;
      rcSrc.bottom = pBox->y2;
      
      /* Calculate destination rectange */
      rcDest.left = rcClient.left + rcSrc.left;
      rcDest.top = rcClient.top + rcSrc.top;
      rcDest.right = rcClient.left + rcSrc.right;
      rcDest.bottom = rcClient.top + rcSrc.bottom;

      /* Blit the damaged areas */
      ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsPrimary,
				       &rcDest,
				       pScreenPriv->pddsShadow,
				       &rcSrc,
				       DDBLT_WAIT,
				       NULL);
      
      /* Get a pointer to the next box */
      ++pBox;
    }

  /* Relock the shadow surface */
  ddrval = IDirectDrawSurface_Lock (pScreenPriv->pddsShadow,
				    NULL,
				    pScreenPriv->pddsdShadow,
				    DDLOCK_WAIT,
				    NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winShadowUpdateProcDD () - Lock failed\n");
      return;
    }

  /* Has our memory pointer changed? */
  if (pScreenInfo->pfb != pScreenPriv->pddsdShadow->lpSurface)
    {
      /* Location of shadow framebuffer has changed */
      pScreenInfo->pfb = pScreenPriv->pddsdShadow->lpSurface;
      
      /* Update the screen pixmap */
      if (!(*pScreen->ModifyPixmapHeader)(pScreen->devPrivate,
					  pScreen->width,
					  pScreen->height,
					  pScreen->rootDepth,
					  BitsPerPixel (pScreen->rootDepth),
					  PixmapBytePad (pScreenInfo->dwStride,
							 pScreenInfo->dwDepth),
					  pScreenInfo->pfb))
	{
	  ErrorF ("winShadowUpdateProcDD () - Bits changed, could not "\
		  "notify fb.\n");
	  return;
	}
    }
}

/*
 * Return a pointer to some part of the shadow framebuffer.
 * 
 * NOTE: I have not seen this function get called, yet.
 */
void *
winShadowSetWindowLinearDD (ScreenPtr	pScreen,
			    CARD32	dwRow,
			    CARD32	dwOffset,
			    int		mode,
			    CARD32	*pdwSize)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  *pdwSize = pScreenInfo->dwPaddedWidth;

  return (CARD8 *) pScreenInfo->pfb
    + dwRow * pScreenInfo->dwPaddedWidth + dwOffset;
}

/*
 * Return a pointer to some part of the shadow framebuffer.
 * 
 * NOTE: I have not seen this function get called, yet.
 *  
 * We call winShadowSetWindowLinearDD because there could,
 * theoretically, be other framebuffer styles that
 * required a different calculation.
 */
void *
winShadowWindowDD (ScreenPtr	pScreen,
		   CARD32	row,
		   CARD32	offset,
		   int		mode,
		   CARD32	*size)
{
  return winShadowSetWindowLinearDD (pScreen, row, offset, mode, size);
}

/*
 * Call the wrapped CloseScreen function.
 * 
 * Free our resources and private structures.
 */
Bool
winCloseScreenShadowDD (int nIndex, ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  Bool			fReturn;
  
#if CYGDEBUG
  ErrorF ("winCloseScreenShadowDD () - Freeing screen resources\n");
#endif

  /* Flag that the screen is closed */
  pScreenPriv->fClosed = TRUE;
  pScreenPriv->fActive = FALSE;

  /* Call the wrapped CloseScreen procedure */
  pScreen->CloseScreen = pScreenPriv->CloseScreen;
  fReturn = (*pScreen->CloseScreen) (nIndex, pScreen);

  /* Delete the window property */
  RemoveProp (pScreenPriv->hwndScreen, WIN_SCR_PROP);

  /* Free the shadow surface, if there is one */
  if (pScreenPriv->pddsShadow)
    {
      IDirectDrawSurface_Unlock (pScreenPriv->pddsShadow, NULL);
      IDirectDrawSurface_Release (pScreenPriv->pddsShadow);
      pScreenPriv->pddsShadow = NULL;
    }

  /* Release the primary surface, if there is one */
  if (pScreenPriv->pddsPrimary)
    {
      IDirectDrawSurface_Release (pScreenPriv->pddsPrimary);
      pScreenPriv->pddsPrimary = NULL;
    }

  /* Free the DirectDraw object, if there is one */
  if (pScreenPriv->pdd)
    {
      IDirectDraw_RestoreDisplayMode (pScreenPriv->pdd);
      IDirectDraw_Release (pScreenPriv->pdd);
      pScreenPriv->pdd = NULL;
    }

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

/*
 * Tell mi what sort of visuals we need.
 * 
 * Generally we only need one visual, as our screen can only
 * handle one format at a time, I believe.  You may want
 * to verify that last sentence.
 */
Bool
winInitVisualsShadowDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  DWORD			dwRedBits, dwGreenBits, dwBlueBits;

  /* Count the number of ones in each color mask */
  dwRedBits = winCountBits (pScreenPriv->dwRedMask);
  dwGreenBits = winCountBits (pScreenPriv->dwGreenMask);
  dwBlueBits = winCountBits (pScreenPriv->dwBlueMask);
  
  /* Store the maximum number of ones in a color mask as the bitsPerRGB */
  if (dwRedBits > dwGreenBits && dwRedBits > dwBlueBits)
    pScreenPriv->dwBitsPerRGB = dwRedBits;
  else if (dwGreenBits > dwRedBits && dwGreenBits > dwBlueBits)
    pScreenPriv->dwBitsPerRGB = dwGreenBits;
  else
    pScreenPriv->dwBitsPerRGB = dwBlueBits;
  
  ErrorF ("winInitVisualsShadowDD () - Masks: %08x %08x %08x bpRGB: %d\n",
	  pScreenPriv->dwRedMask,
	  pScreenPriv->dwGreenMask,
	  pScreenPriv->dwBlueMask,
	  pScreenPriv->dwBitsPerRGB);

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
	  ErrorF ("winInitVisualsShadowDD () - miSetVisualTypesAndMasks "\
		  "failed\n");
	  return FALSE;
	}
      break;

    case 8:
#if CYGDEBUG
      ErrorF ("winInitVisualsShadowDD () - Calling "\
	      "miSetVisualTypesAndMasks\n");
#endif /* CYGDEBUG */
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     PseudoColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     PseudoColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowDD () - miSetVisualTypesAndMasks "\
		  "failed\n");
	  return FALSE;
	}
#if CYGDEBUG
      ErrorF ("winInitVisualsShadowDD () - Returned from "\
	      "miSetVisualTypesAndMasks\n");
#endif /* CYGDEBUG */
      break;

    default:
      ErrorF ("winInitVisualsDD () - Unknown screen depth\n");
      return FALSE;
    }

  /* Set DPI info */
  pScreenInfo->dwDPIx = 100;
  pScreenInfo->dwDPIy = 100;

  ErrorF ("winInitVisualsShadowDD () - Returning\n");

  return TRUE;
}

Bool
winAdjustVideoModeShadowDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HDC			hdc = NULL;
  DWORD			dwDepth;

  /* Are we fullscreen? */
  if (pScreenInfo->fFullScreen)
    {
      /* We don't need to adjust the video mode for fullscreen */
      return TRUE;
    }

  /* We're in serious trouble if we can't get a DC */
  hdc = GetDC (NULL);
  if (hdc == NULL)
    {
      ErrorF ("winAdjustVideoModeShadowDD () - GetDC () failed\n");
      return FALSE;
    }

  /* Query GDI for current display depth */
  dwDepth = GetDeviceCaps (hdc, BITSPIXEL);

  /* Is GDI using a depth different than command line parameter? */
  if (dwDepth != pScreenInfo->dwDepth)
    {
      /* Warn user if GDI depth is different than depth specified */
      ErrorF ("winAdjustVideoModeShadowDD () - Command line depth: %d, "\
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
winBltExposedRegionsShadowDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  RECT			rcClient, rcSrc;
  HDC			hdcUpdate = NULL;
  PAINTSTRUCT		ps;
  HRESULT		ddrval = DD_OK;
  Bool			fReturn = TRUE;

  /* BeginPaint gives us an hdc that clips to the invalidated region */
  hdcUpdate = BeginPaint (pScreenPriv->hwndScreen, &ps);

  /* Unlock the shadow surface, so we can blit */
  ddrval = IDirectDrawSurface_Unlock (pScreenPriv->pddsShadow, NULL);
  if (FAILED (ddrval))
    {
      fReturn = FALSE;
      goto winBltExposedRegionsShadowDD_Exit;
    }
	  
  /* Get client area in screen coords */
  GetClientRect (pScreenPriv->hwndScreen, &rcClient);
  MapWindowPoints (pScreenPriv->hwndScreen,
		   HWND_DESKTOP,
		   (LPPOINT)&rcClient, 2);
	  
  /* Source can be enter shadow surface, as Blt should clip */
  rcSrc.left = 0;
  rcSrc.top = 0;
  rcSrc.right = pScreenInfo->dwWidth;
  rcSrc.bottom = pScreenInfo->dwHeight;

  /* Our Blt should be clipped to the invalidated region */
  ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsPrimary,
				   &rcClient,
				   pScreenPriv->pddsShadow,
				   &rcSrc,
				   DDBLT_WAIT,
				   NULL);

  /* Relock the shadow surface */
  ddrval = IDirectDrawSurface_Lock (pScreenPriv->pddsShadow,
				    NULL,
				    pScreenPriv->pddsdShadow,
				    DDLOCK_WAIT,
				    NULL);
  if (FAILED (ddrval))
    {
      fReturn = FALSE;
      goto winBltExposedRegionsShadowDD_Exit;
    }

  /* Has our memory pointer changed? */
  if (pScreenInfo->pfb != pScreenPriv->pddsdShadow->lpSurface)
    winUpdateFBPointer (pScreen,
			pScreenPriv->pddsdShadow->lpSurface);

 winBltExposedRegionsShadowDD_Exit:
  /* EndPaint frees the DC */
  if (hdcUpdate != NULL)
    EndPaint (pScreenPriv->hwndScreen, &ps);
  return fReturn;
}

Bool
winActivateAppShadowDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);

  /*
   * Do we have a surface?
   * Are we active?
   * Are we fullscreen?
   */
  if (pScreenPriv != NULL
      && pScreenPriv->pddsPrimary != NULL
      && pScreenPriv->fActive
      )
    {
      /* Primary surface was lost, restore it */
      IDirectDrawSurface_Restore (pScreenPriv->pddsPrimary);
    }

  return TRUE;
}

/* Set engine specific functions */
Bool
winSetEngineFunctionsShadowDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  
  /* Set our pointers */
  pScreenPriv->pwinAllocateFB = winAllocateFBShadowDD;
  pScreenPriv->pwinShadowUpdate = winShadowUpdateDD;
  pScreenPriv->pwinShadowWindow = winShadowWindowDD;
  pScreenPriv->pwinCloseScreen = winCloseScreenShadowDD;
  pScreenPriv->pwinInitVisuals = winInitVisualsShadowDD;
  pScreenPriv->pwinAdjustVideoMode = winAdjustVideoModeShadowDD;
  if (pScreenInfo->fFullScreen)
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowFullScreen;
  else
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowWindowed;
  pScreenPriv->pwinFinishScreenInit = winFinishScreenInitFB;
  pScreenPriv->pwinBltExposedRegions = winBltExposedRegionsShadowDD;
  pScreenPriv->pwinActivateApp = winActivateAppShadowDD;

  return TRUE;
}


