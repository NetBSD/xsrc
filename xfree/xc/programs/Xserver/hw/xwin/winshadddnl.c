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
/* $XFree86: xc/programs/Xserver/hw/xwin/winshadddnl.c,v 1.20 2001/11/21 08:51:24 alanh Exp $ */

#include "win.h"

/*
 * FIXME: Headers are broken, DEFINE_GUID doesn't work correctly,
 * so we have to redefine it here.
 */
#ifdef DEFINE_GUID
#undef DEFINE_GUID
#define DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) GUID_EXT const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#endif /* DEFINE_GUID */

/*
 * FIXME: Headers are broken, IID_IDirectDraw4 has to be defined
 * here manually.  Should be handled by ddraw.h
 */
#ifndef IID_IDirectDraw4
DEFINE_GUID( IID_IDirectDraw4, 0x9c59509a,0x39bd,0x11d1,0x8c,0x4a,0x00,0xc0,0x4f,0xd9,0x30,0xc5 );
#endif /* IID_IDirectDraw4 */

/*
 * Create a DirectDraw surface for the shadow framebuffer; also create
 * a primary surface object so we can blit to the display.
 * 
 * Install a DirectDraw clipper on our primary surface object
 * that clips our blits to the unobscured client area of our display window.
 */
Bool
winAllocateFBShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;  
  HRESULT		ddrval = DD_OK;
  DDSURFACEDESC2	ddsdPrimary;
  DDSURFACEDESC2	ddsdShadow;
  char			*lpSurface = NULL;
  DDPIXELFORMAT		ddpfPrimary;

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - w %d h %d d %d\n",
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight, pScreenInfo->dwDepth);
#endif

  /* Allocate memory for our shadow surface */
  lpSurface = xalloc (pScreenInfo->dwPaddedWidth * pScreenInfo->dwHeight);
  if (lpSurface == NULL)
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not allocate bits\n");
      return FALSE;
    }

  /*
   * Initialize the framebuffer memory so we don't get a 
   * strange display at startup
   */
  ZeroMemory (lpSurface, pScreenInfo->dwPaddedWidth * pScreenInfo->dwHeight);
  
  /* Create a clipper */
  ddrval = DirectDrawCreateClipper (0,
				    &pScreenPriv->pddcPrimary,
				    NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not attach clipper: %08x\n",
	      ddrval);
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Created a clipper\n");
#endif

  /* Attach the clipper to our display window */
  ddrval = IDirectDrawClipper_SetHWnd (pScreenPriv->pddcPrimary,
				       0,
				       pScreenPriv->hwndScreen);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Clipper not attached "\
	      "to window: %08x\n",
	      ddrval);
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Attached clipper to window\n");
#endif

  /* Create a DirectDraw object, store the address at lpdd */
  ddrval = DirectDrawCreate (NULL,
			     (LPDIRECTDRAW*) &pScreenPriv->pdd,
			     NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not start "
	      "DirectDraw: %08x\n",
	      ddrval);
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Created and initialized DD\n");
#endif

  /* Get a DirectDraw4 interface pointer */
  ddrval = IDirectDraw_QueryInterface (pScreenPriv->pdd,
				       &IID_IDirectDraw4,
				       (LPVOID*) &pScreenPriv->pdd4);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Failed DD4 query: %08x\n",
	      ddrval);
      return FALSE;
    }

  /* Are we full screen? */
  /* FIXME: If we are full screen we don't need the clipper */
  if (pScreenInfo->fFullScreen)
    {
      DDSURFACEDESC2	ddsdCurrent;
      DWORD		dwRefreshRateCurrent = 0;
      HDC		hdc = NULL;

      /* Set the cooperative level to full screen */
      ddrval = IDirectDraw4_SetCooperativeLevel (pScreenPriv->pdd4,
						 pScreenPriv->hwndScreen,
						 DDSCL_EXCLUSIVE
						 | DDSCL_FULLSCREEN);
      if (FAILED (ddrval))
	{
	  ErrorF ("winAllocateFBShadowDDNL () - Could not set "\
		  "cooperative level: %08x\n",
		  ddrval);
	  return FALSE;
	}

      /*
       * We only need to get the current refresh rate for comparison
       * if a refresh rate has been passed on the command line.
       */
      if (pScreenInfo->dwRefreshRate != 0)
	{
	  ZeroMemory (&ddsdCurrent, sizeof (ddsdCurrent));
	  ddsdCurrent.dwSize = sizeof (ddsdCurrent);

	  /* Get information about current display settings */
	  ddrval = IDirectDraw4_GetDisplayMode (pScreenPriv->pdd4,
						&ddsdCurrent);
	  if (FAILED (ddrval))
	    {
	      ErrorF ("winAllocateFBShadowDDNL () - Could not get current "
		      "refresh rate: %08x.  Continuing.\n",
		      ddrval);
	      dwRefreshRateCurrent = 0;
	    }
	  else
	    {
	      /* Grab the current refresh rate */
	      dwRefreshRateCurrent = ddsdCurrent.u2.dwRefreshRate;
	    }
	}

      /* Clean up the refresh rate */
      if (dwRefreshRateCurrent == pScreenInfo->dwRefreshRate)
	{
	  /*
	   * Refresh rate is non-specified or equal to current.
	   */
	  pScreenInfo->dwRefreshRate = 0;
	}

      /* Grab a device context for the screen */
      hdc = GetDC (NULL);
      if (hdc == NULL)
	{
	  ErrorF ("winAllocateFBShadowDDNL () - GetDC () failed\n");
	  return FALSE;
	}

      /* Only change the video mode when different than current mode */
      if (pScreenInfo->dwWidth != GetSystemMetrics (SM_CXSCREEN)
	  || pScreenInfo->dwHeight != GetSystemMetrics (SM_CYSCREEN)
	  || pScreenInfo->dwDepth != GetDeviceCaps (hdc, BITSPIXEL)
	  || pScreenInfo->dwRefreshRate != 0)
	{
	  ErrorF ("winAllocateFBShadowDDNL () - Changing video mode\n");

	  /* Change the video mode to the mode requested */
	  ddrval = IDirectDraw4_SetDisplayMode (pScreenPriv->pdd4,
						pScreenInfo->dwWidth,
						pScreenInfo->dwHeight,
						pScreenInfo->dwDepth,
						pScreenInfo->dwRefreshRate,
						0);	       
	  if (FAILED (ddrval))
	    {
	      ErrorF ("winAllocateFBShadowDDNL () - Could not set "\
		      "full screen display mode: %08x\n",
		      ddrval);
	      return FALSE;
	    }
	}
      else
	{
	  ErrorF ("winAllocateFBShadowDDNL () - Not changing video mode\n");
	}

      /* Release our DC */
      ReleaseDC (NULL, hdc);
      hdc = NULL;
    }
  else
    {
      /* Set the cooperative level for windowed mode */
      ddrval = IDirectDraw4_SetCooperativeLevel (pScreenPriv->pdd4,
						 pScreenPriv->hwndScreen,
						 DDSCL_NORMAL);
      if (FAILED (ddrval))
	{
	  ErrorF ("winAllocateFBShadowDDNL () - Could not set "\
		  "cooperative level: %08x\n",
		  ddrval);
	  return FALSE;
	}
    }

  /* Describe the primary surface */
  ZeroMemory (&ddsdPrimary, sizeof (ddsdPrimary));
  ddsdPrimary.dwSize = sizeof (ddsdPrimary);
  ddsdPrimary.dwFlags = DDSD_CAPS;
  ddsdPrimary.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  
  /* Create the primary surface */
  ddrval = IDirectDraw4_CreateSurface (pScreenPriv->pdd4,
				       &ddsdPrimary,
				       &pScreenPriv->pddsPrimary4,
				       NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not create primary "\
	      "surface: %08x\n",
	      ddrval);
      return FALSE;
    }
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Created primary\n");
#endif

  /* Get primary surface's pixel format */
  ZeroMemory (&ddpfPrimary, sizeof (ddpfPrimary));
  ddpfPrimary.dwSize = sizeof (ddpfPrimary);
  ddrval = IDirectDrawSurface4_GetPixelFormat (pScreenPriv->pddsPrimary4,
					       &ddpfPrimary);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not get primary "\
	      "pixformat: %08x\n",
	      ddrval);
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Primary masks: %08x %08x %08x "\
	  "dwRGBBitCount: %d\n",
	  ddpfPrimary.u2.dwRBitMask,
	  ddpfPrimary.u3.dwGBitMask,
	  ddpfPrimary.u4.dwBBitMask,
	  ddpfPrimary.u.dwRGBBitCount);
#endif

  /* Attach our clipper to our primary surface handle */
  ddrval = IDirectDrawSurface4_SetClipper (pScreenPriv->pddsPrimary4,
					   pScreenPriv->pddcPrimary);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Primary attach clipper "\
	      "failed: %08x\n",
	      ddrval);
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Attached clipper to primary "\
	  "surface\n");
#endif

  /* Describe the shadow surface to be created */
  /*
   * NOTE: Do not use a DDSCAPS_VIDEOMEMORY surface,
   * as drawing, locking, and unlocking take forever
   * with video memory surfaces.  In addition,
   * video memory is a somewhat scarce resource,
   * so you shouldn't be allocating video memory when
   * you have the option of using system memory instead.
   */
  ZeroMemory (&ddsdShadow, sizeof (ddsdShadow));
  ddsdShadow.dwSize = sizeof (ddsdShadow);
  ddsdShadow.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH
    | DDSD_LPSURFACE | DDSD_PITCH | DDSD_PIXELFORMAT;
  ddsdShadow.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  ddsdShadow.dwHeight = pScreenInfo->dwHeight;
  ddsdShadow.dwWidth = pScreenInfo->dwWidth;
  ddsdShadow.u1.lPitch = pScreenInfo->dwPaddedWidth;
  ddsdShadow.lpSurface = lpSurface;
  ddsdShadow.u4.ddpfPixelFormat = ddpfPrimary;
  
  ErrorF ("winAllocateFBShadowDDNL () - lPitch: %d\n",
	  pScreenInfo->dwPaddedWidth);

  /* Create the shadow surface */
  ddrval = IDirectDraw4_CreateSurface (pScreenPriv->pdd4,
				       &ddsdShadow,
				       &pScreenPriv->pddsShadow4,
				       NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not create shadow "\
	      "surface: %08x\n", ddrval);
      return FALSE;
    }
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Created shadow pitch: %d\n",
	  ddsdShadow.u1.lPitch);
#endif

  /* Grab the pitch, and memory pointer from the surface desc */
  pScreenInfo->dwStrideBytes = pScreenInfo->dwPaddedWidth;
  pScreenInfo->dwStride = (pScreenInfo->dwStrideBytes * 8)
    / pScreenInfo->dwDepth;
  pScreenInfo->pfb = lpSurface;
  
  /* Grab the color depth and masks from the surface description */
  pScreenInfo->dwDepth = ddsdShadow.u4.ddpfPixelFormat.u1.dwRGBBitCount;
  pScreenPriv->dwRedMask = ddsdShadow.u4.ddpfPixelFormat.u2.dwRBitMask;
  pScreenPriv->dwGreenMask = ddsdShadow.u4.ddpfPixelFormat.u3.dwGBitMask;
  pScreenPriv->dwBlueMask = ddsdShadow.u4.ddpfPixelFormat.u4.dwBBitMask;
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Returning\n");
#endif

  return TRUE;
}

/*
 * Transfer the damaged regions of the shadow framebuffer to the display.
 */
void
winShadowUpdateDDNL (ScreenPtr pScreen, 
		     shadowBufPtr pBuf)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  RegionPtr		damage = &pBuf->damage;
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

  /* Loop through all boxes in the damaged region */
  while (dwBox--)
    {
      /* Assign damage box to source rectangle */
      rcSrc.left = pBox->x1;
      rcSrc.top = pBox->y1;
      rcSrc.right = pBox->x2;
      rcSrc.bottom = pBox->y2;
      
      /* Calculate destination rectangle */
      rcDest.left = rcClient.left + rcSrc.left;
      rcDest.top = rcClient.top + rcSrc.top;
      rcDest.right = rcClient.left + rcSrc.right;
      rcDest.bottom = rcClient.top + rcSrc.bottom;

      /* Blit the damaged areas */
      ddrval = IDirectDrawSurface4_Blt (pScreenPriv->pddsPrimary4,
					&rcDest,
					pScreenPriv->pddsShadow4,
					&rcSrc,
					DDBLT_WAIT,
					NULL);
      if (FAILED (ddrval))
	{
	  ErrorF ("winShadowUpdateDDNL () - IDirectDrawSurface4_Blt () "
		  "failed: %08x\n",
		  ddrval);
	}

      /* Get a pointer to the next box */
      ++pBox;
    }
}

/*
 * Call the wrapped CloseScreen function.
 *
 * Free our resources and private structures.
 */
Bool
winCloseScreenShadowDDNL (int nIndex, ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  Bool			fReturn;

#if 1
  ErrorF ("winCloseScreenShadowDDNL () - Freeing screen resources\n");
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
  if (pScreenPriv->pddsShadow4)
    {
      IDirectDrawSurface4_Release (pScreenPriv->pddsShadow4);
      xfree (pScreenInfo->pfb);
      pScreenInfo->pfb = NULL;
      pScreenPriv->pddsShadow4 = NULL;
    }

  /* Release the primary surface, if there is one */
  if (pScreenPriv->pddsPrimary4)
    {
      IDirectDrawSurface4_Release (pScreenPriv->pddsPrimary4);
      pScreenPriv->pddsPrimary4 = NULL;
    }

  /* Free the DirectDraw4 object, if there is one */
  if (pScreenPriv->pdd4)
    {
      IDirectDraw4_RestoreDisplayMode (pScreenPriv->pdd4);
      IDirectDraw4_Release (pScreenPriv->pdd4);
      pScreenPriv->pdd4 = NULL;
    }

  /* Free the DirectDraw object, if there is one */
  if (pScreenPriv->pdd)
    {
      IDirectDraw_Release (pScreenPriv->pdd);
      pScreenPriv->pdd = NULL;
    }

  /* Kill our window */
  if (pScreenPriv->hwndScreen)
    {
      DestroyWindow (pScreenPriv->hwndScreen);
      pScreenPriv->hwndScreen = NULL;
    }

  /* Kill our screeninfo's pointer to the screen */
  pScreenInfo->pScreen = NULL;

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
winInitVisualsShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  DWORD			dwRedBits, dwGreenBits, dwBlueBits;

  /* Count the number of ones in each color mask */
  dwRedBits = winCountBits (pScreenPriv->dwRedMask);
  dwGreenBits = winCountBits (pScreenPriv->dwGreenMask);
  dwBlueBits = winCountBits (pScreenPriv->dwBlueMask);
  
  /* Store the maximum number of ones in a color mask as the bitsPerRGB */
  if (dwRedBits == 0 || dwGreenBits == 0 || dwBlueBits == 0)
    pScreenPriv->dwBitsPerRGB = 8;
  else if (dwRedBits > dwGreenBits && dwRedBits > dwBlueBits)
    pScreenPriv->dwBitsPerRGB = dwRedBits;
  else if (dwGreenBits > dwRedBits && dwGreenBits > dwBlueBits)
    pScreenPriv->dwBitsPerRGB = dwGreenBits;
  else
    pScreenPriv->dwBitsPerRGB = dwBlueBits;

  ErrorF ("winInitVisualsShadowDDNL () - Masks %08x %08x %08x BPRGB %d d %d\n",
	  pScreenPriv->dwRedMask,
	  pScreenPriv->dwGreenMask,
	  pScreenPriv->dwBlueMask,
	  pScreenPriv->dwBitsPerRGB,
	  pScreenInfo->dwDepth);

  /* Create a single visual according to the Windows screen depth */
  switch (pScreenInfo->dwDepth)
    {
    case 32:
    case 24:
    case 16:
    case 15:
      /* Setup the real visual */
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     TrueColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     TrueColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowDDNL () - "\
		  "miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;

    case 8:
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     pScreenInfo->fFullScreen 
				     ? PseudoColorMask : StaticColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     pScreenInfo->fFullScreen 
				     ? PseudoColor : StaticColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowDDNL () - "\
		  "miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;

    default:
      ErrorF ("winInitVisualsDDNL () - Unknown screen depth\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winInitVisualsShadowDDNL () - Returning\n");
#endif

  return TRUE;
}

Bool
winAdjustVideoModeShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HDC			hdc = NULL;
  DWORD			dwDepth;

  /* We're in serious trouble if we can't get a DC */
  hdc = GetDC (NULL);
  if (hdc == NULL)
    {
      ErrorF ("winAdjustVideoModeShadowDDNL () - GetDC () failed\n");
      return FALSE;
    }

  /* Query GDI for current display depth */
  dwDepth = GetDeviceCaps (hdc, BITSPIXEL);

  /* DirectDraw can only change the depth in fullscreen mode */
  if (pScreenInfo->dwDepth == WIN_DEFAULT_DEPTH)
    {
      /* No -depth parameter passed, let the user know the depth being used */
      ErrorF ("winAdjustVideoModeShadowDDNL () - Using Windows display "
	      "depth of %d bits per pixel\n", dwDepth);

      /* Use GDI's depth */
      pScreenInfo->dwDepth = dwDepth;
    }
  else if (pScreenInfo->fFullScreen
	   && pScreenInfo->dwDepth != dwDepth)
    {
      /* FullScreen, and GDI depth differs from -depth parameter */
      ErrorF ("winAdjustVideoModeShadowDDNL () - FullScreen, using command "
	      "line depth: %d\n", pScreenInfo->dwDepth);
    }
  else if (dwDepth != pScreenInfo->dwDepth)
    {
      /* Windowed, and GDI depth differs from -depth parameter */
      ErrorF ("winAdjustVideoModeShadowDDNL () - Windowed, command line "
	      "depth: %d, using depth: %d\n",
	      pScreenInfo->dwDepth, dwDepth);

      /* We'll use GDI's depth */
      pScreenInfo->dwDepth = dwDepth;
    }

  /* See if the shadow bitmap will be larger than the DIB size limit */
  if (pScreenInfo->dwWidth * pScreenInfo->dwHeight * pScreenInfo->dwDepth
      >= WIN_DIB_MAXIMUM_SIZE)
    {
      ErrorF ("winAdjustVideoModeShadowDDNL () - Requested DirectDraw surface "
	      "will be larger than %d MB.  The surface may fail to be "
	      "allocated on Windows 95, 98, or Me, due to a %d MB limit in "
	      "DIB size.  This limit does not apply to Windows NT/2000, and "
	      "this message may be ignored on those platforms.\n",
	      WIN_DIB_MAXIMUM_SIZE_MB, WIN_DIB_MAXIMUM_SIZE_MB);
    }
  
  /* Release our DC */
  ReleaseDC (NULL, hdc);

  return TRUE;
}

/* Blt exposed regions to the screen */
Bool
winBltExposedRegionsShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  RECT			rcClient, rcSrc;
  HDC			hdcUpdate;
  PAINTSTRUCT		ps;
  HRESULT		ddrval = DD_OK;
  Bool			fReturn = TRUE;

  /* BeginPaint gives us an hdc that clips to the invalidated region */
  hdcUpdate = BeginPaint (pScreenPriv->hwndScreen, &ps);
  if (hdcUpdate == NULL)
    {
      fReturn = FALSE;
      ErrorF ("winBltExposedRegionsShadowDDNL () - BeginPaint () returned "
	      "a NULL device context handle.  Aborting blit attempt.\n");
      goto winBltExposedRegionsShadowDDNL_Exit;
    }

  /* Get client area in screen coords */
  fReturn = GetClientRect (pScreenPriv->hwndScreen, &rcClient);
  if (!fReturn)
    {
      fReturn = FALSE;
      ErrorF ("winBltExposedRegionsShadowDDNL () - GetClientRect () failed\n");
      goto winBltExposedRegionsShadowDDNL_Exit;
    }

  /* Map the client coords to screen coords */
  MapWindowPoints (pScreenPriv->hwndScreen,
		   HWND_DESKTOP,
		   (LPPOINT)&rcClient,
		   2);
	  
  /* Source can be entire shadow surface, as Blt should clip for us */
  rcSrc.left = 0;
  rcSrc.top = 0;
  rcSrc.right = pScreenInfo->dwWidth;
  rcSrc.bottom = pScreenInfo->dwHeight;

  /* Our Blt should be clipped to the invalidated region */
  ddrval = IDirectDrawSurface4_Blt (pScreenPriv->pddsPrimary4,
				    &rcClient,
				    pScreenPriv->pddsShadow4,
				    &rcSrc,
				    DDBLT_WAIT,
				    NULL);
  if (FAILED (ddrval))
    {
      fReturn = FALSE;
      ErrorF ("winBltExposedRegionsShadowDDNL () - IDirectDrawSurface_Blt "
	      "failed: %08x %d\n", ddrval, ddrval);
      goto winBltExposedRegionsShadowDDNL_Exit;
    }

 winBltExposedRegionsShadowDDNL_Exit:
  /* EndPaint frees the DC */
  if (hdcUpdate != NULL)
    EndPaint (pScreenPriv->hwndScreen, &ps);
  return fReturn;
}

Bool
winActivateAppShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);

  /*
   * Do we have a surface?
   * Are we active?
   * Are we full screen?
   */
  if (pScreenPriv != NULL
      && pScreenPriv->pddsPrimary4 != NULL
      && pScreenPriv->fActive
      )
    {
      /* Primary surface was lost, restore it */
      IDirectDrawSurface4_Restore (pScreenPriv->pddsPrimary4);
    }

  return TRUE;
}


/*
 * Reblit the shadow framebuffer to the screen.
 */

Bool
winRedrawScreenShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HRESULT		ddrval = DD_OK;
  RECT			rcClient, rcSrc;

  /* Get location of display window's client area, in screen coords */
  GetClientRect (pScreenPriv->hwndScreen, &rcClient);
  MapWindowPoints (pScreenPriv->hwndScreen,
		   HWND_DESKTOP,
		   (LPPOINT)&rcClient, 2);

  /* Source can be entire shadow surface, as Blt should clip for us */
  rcSrc.left = 0;
  rcSrc.top = 0;
  rcSrc.right = pScreenInfo->dwWidth;
  rcSrc.bottom = pScreenInfo->dwHeight;

  /* Redraw the whole window, to take account for the new colors */
  ddrval = IDirectDrawSurface4_Blt (pScreenPriv->pddsPrimary4,
				    &rcClient,
				    pScreenPriv->pddsShadow4,
				    &rcSrc,
				    DDBLT_WAIT,
				    NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winRedrawScreenShadowDDNL () - IDirectDrawSurface4_Blt () "
	      "failed: %08x\n",
	      ddrval);
    }

  return TRUE;
}


/* Realize the currently installed colormap */
Bool
winRealizeInstalledPaletteShadowDDNL (ScreenPtr pScreen)
{
  return TRUE;
}

/* Install the specified colormap */
Bool
winInstallColormapShadowDDNL (ColormapPtr pColormap)
{
  ScreenPtr		pScreen = pColormap->pScreen;
  winScreenPriv(pScreen);
  winCmapPriv(pColormap);
  HRESULT		ddrval = DD_OK;

  /* Install the DirectDraw palette on the primary surface */
  ddrval = IDirectDrawSurface4_SetPalette (pScreenPriv->pddsPrimary4,
					   pCmapPriv->lpDDPalette);
  if (FAILED (ddrval))
    {
      ErrorF ("winInstallColormapShadowDDNL () - Failed installing the "
	      "DirectDraw palette.\n");
      return FALSE;
    }

  /* Save a pointer to the newly installed colormap */
  pScreenPriv->pcmapInstalled = pColormap;

  return TRUE;
}


/* Store the specified colors in the specified colormap */
Bool
winStoreColorsShadowDDNL (ColormapPtr pColormap, 
			  int ndef,
			  xColorItem *pdefs)
{
  ScreenPtr		pScreen = pColormap->pScreen;
  winScreenPriv(pScreen);
  winCmapPriv(pColormap);
  ColormapPtr curpmap = pScreenPriv->pcmapInstalled;
  HRESULT		ddrval = DD_OK;
  
  /* Put the X colormap entries into the Windows logical palette */
  ddrval = IDirectDrawPalette_SetEntries (pCmapPriv->lpDDPalette,
					  0,
					  pdefs[0].pixel,
					  ndef,
					  pCmapPriv->peColors 
					  + pdefs[0].pixel);
  if (FAILED (ddrval))
    {
      ErrorF ("winStoreColorsShadowDDNL () - SetEntries () failed\n");
      return FALSE;
    }

  /* Don't install the DirectDraw palette if the colormap is not installed */
  if (pColormap != curpmap)
    {
      return TRUE;
    }

  if (!winInstallColormapShadowDDNL (pColormap))
    {
      ErrorF ("winStoreColorsShadowDDNL () - Failed installing colormap\n");
      return FALSE;
    }

  return TRUE;
}


/* Colormap initialization procedure */
Bool
winCreateColormapShadowDDNL (ColormapPtr pColormap)
{
  HRESULT		ddrval = DD_OK;
  ScreenPtr		pScreen = pColormap->pScreen;
  winScreenPriv(pScreen);
  winCmapPriv(pColormap);
  
  /* Create a DirectDraw palette */
  ddrval = IDirectDraw4_CreatePalette (pScreenPriv->pdd4,
				       DDPCAPS_8BIT | DDPCAPS_ALLOW256,
				       pCmapPriv->peColors,
				       &pCmapPriv->lpDDPalette,
				       NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winCreateColormapShadowDDNL () - CreatePalette failed\n");
      return FALSE;
    }

  return TRUE;
}


/* Colormap destruction procedure */
Bool
winDestroyColormapShadowDDNL (ColormapPtr pColormap)
{
  winScreenPriv(pColormap->pScreen);
  winCmapPriv(pColormap);
  HRESULT		ddrval = DD_OK;

  /*
   * Is colormap to be destroyed the default?
   *
   * Non-default colormaps should have had winUninstallColormap
   * called on them before we get here.  The default colormap
   * will not have had winUninstallColormap called on it.  Thus,
   * we need to handle the default colormap in a special way.
   */
  if (pColormap->flags & IsDefault)
    {
#if CYGDEBUG
      ErrorF ("winDestroyColormapShadowDDNL () - Destroying default "
	      "colormap\n");
#endif
      
      /*
       * FIXME: Walk the list of all screens, popping the default
       * palette out of each screen device context.
       */
      
      /* Pop the palette out of the primary surface */
      ddrval = IDirectDrawSurface4_SetPalette (pScreenPriv->pddsPrimary4,
					       NULL);
      if (FAILED (ddrval))
	{
	  ErrorF ("winDestroyColormapShadowDDNL () - Failed freeing the "
		  "default colormap DirectDraw palette.\n");
	  return FALSE;
	}

      /* Clear our private installed colormap pointer */
      pScreenPriv->pcmapInstalled = NULL;
    }
  
  /* Release the palette */
  IDirectDrawPalette_Release (pCmapPriv->lpDDPalette);
 
  /* Invalidate the colormap privates */
  pCmapPriv->lpDDPalette = NULL;

  return TRUE;
}


/* Set pointers to our engine specific functions */
Bool
winSetEngineFunctionsShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  
  /* Set our pointers */
  pScreenPriv->pwinAllocateFB = winAllocateFBShadowDDNL;
  pScreenPriv->pwinShadowUpdate = winShadowUpdateDDNL;
  pScreenPriv->pwinCloseScreen = winCloseScreenShadowDDNL;
  pScreenPriv->pwinInitVisuals = winInitVisualsShadowDDNL;
  pScreenPriv->pwinAdjustVideoMode = winAdjustVideoModeShadowDDNL;
  if (pScreenInfo->fFullScreen)
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowFullScreen;
  else
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowWindowed;
  pScreenPriv->pwinFinishScreenInit = winFinishScreenInitFB;
  pScreenPriv->pwinBltExposedRegions = winBltExposedRegionsShadowDDNL;
  pScreenPriv->pwinActivateApp = winActivateAppShadowDDNL;
  pScreenPriv->pwinRedrawScreen = winRedrawScreenShadowDDNL;
  pScreenPriv->pwinRealizeInstalledPalette
    = winRealizeInstalledPaletteShadowDDNL;
  pScreenPriv->pwinInstallColormap = winInstallColormapShadowDDNL;
  pScreenPriv->pwinStoreColors = winStoreColorsShadowDDNL;
  pScreenPriv->pwinCreateColormap = winCreateColormapShadowDDNL;
  pScreenPriv->pwinDestroyColormap = winDestroyColormapShadowDDNL;
  pScreenPriv->pwinHotKeyAltTab = (winHotKeyAltTabPtr) (void (*)())NoopDDA;

  return TRUE;
}


