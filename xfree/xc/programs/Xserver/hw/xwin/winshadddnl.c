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
/* $XFree86: xc/programs/Xserver/hw/xwin/winshadddnl.c,v 1.5 2001/05/14 16:52:33 alanh Exp $ */

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
      ErrorF ("winAllocateFBShadowDDNL () - Could not attach clipper\n");
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
	      "to window\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Attached clipper to window\n");
#endif

  /* Create a DirectDraw object, store the address at lpdd */
  ddrval = DirectDrawCreate (NULL,
			     (LPDIRECTDRAW*) &pScreenPriv->pdd,
			     NULL);
  if (ddrval != DD_OK)
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not start DirectDraw\n");
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
      /* Set the cooperative level to full screen */
      ddrval = IDirectDraw_SetCooperativeLevel (pScreenPriv->pdd4,
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
      ddrval = IDirectDraw_SetCooperativeLevel (pScreenPriv->pdd4,
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
  ZeroMemory (&ddsdPrimary, sizeof (ddsdPrimary));
  ddsdPrimary.dwSize = sizeof (ddsdPrimary);
  ddsdPrimary.dwFlags = DDSD_CAPS;
  ddsdPrimary.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  
  /* Create the primary surface */
  ddrval = IDirectDraw_CreateSurface (pScreenPriv->pdd4,
				      (LPDDSURFACEDESC)&ddsdPrimary,
				      (LPDIRECTDRAWSURFACE*)
				      &pScreenPriv->pddsPrimary4,
				      NULL);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not create primary "\
	      "surface %08x\n", ddrval);
      return FALSE;
    }
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Created primary\n");
#endif

  /* Get primary surface's pixel format */
  ZeroMemory (&ddpfPrimary, sizeof (ddpfPrimary));
  ddpfPrimary.dwSize = sizeof (ddpfPrimary);
  ddrval = IDirectDrawSurface_GetPixelFormat (pScreenPriv->pddsPrimary4,
					       &ddpfPrimary);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not get primary "\
	      "pixformat\n");
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
  ddrval = IDirectDrawSurface_SetClipper (pScreenPriv->pddsPrimary4,
					   pScreenPriv->pddcPrimary);
  if (FAILED (ddrval))
    {
      ErrorF ("winAllocateFBShadowDDNL () - Primary attach clipper "\
	      "failed\n");
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
  ddsdShadow.u.lPitch = pScreenInfo->dwPaddedWidth;
  ddsdShadow.lpSurface = lpSurface;
  ddsdShadow.ddpfPixelFormat = ddpfPrimary;
  
  /* Create the shadow surface */
  ddrval = IDirectDraw_CreateSurface (pScreenPriv->pdd4,
				      (LPDDSURFACEDESC)&ddsdShadow,
				      (LPDIRECTDRAWSURFACE*)
				      &pScreenPriv->pddsShadow4,
				      NULL);
  if (ddrval != DD_OK)
    {
      ErrorF ("winAllocateFBShadowDDNL () - Could not create shadow "\
	      "surface %08x\n", ddrval);
      return FALSE;
    }
  
#if CYGDEBUG
  ErrorF ("winAllocateFBShadowDDNL () - Created shadow pitch: %d\n",
	  ddsdShadow.u.lPitch);
#endif

  /* Grab the pitch, and memory pointer from the surface desc */
  pScreenInfo->dwStrideBytes = pScreenInfo->dwPaddedWidth;
  pScreenInfo->dwStride = (pScreenInfo->dwStrideBytes * 8)
    / pScreenInfo->dwDepth;
  pScreenInfo->pfb = lpSurface;
  
  /* Grab the color depth and masks from the surface description */
  pScreenInfo->dwDepth = ddsdShadow.ddpfPixelFormat.u.dwRGBBitCount;
  pScreenPriv->dwRedMask = ddsdShadow.ddpfPixelFormat.u2.dwRBitMask;
  pScreenPriv->dwGreenMask = ddsdShadow.ddpfPixelFormat.u3.dwGBitMask;
  pScreenPriv->dwBlueMask = ddsdShadow.ddpfPixelFormat.u4.dwBBitMask;
  
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
      ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsPrimary4,
				       &rcDest,
				       pScreenPriv->pddsShadow4,
				       &rcSrc,
				       DDBLT_WAIT,
				       NULL);
      
      /* Get a pointer to the next box */
      ++pBox;
    }
}

/*
 * Return a pointer to some part of the shadow framebuffer.
 * 
 * NOTE: I have not seen this function get called, yet.
 */
void *
winShadowSetWindowLinearDDNL (ScreenPtr	pScreen,
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
winShadowWindowDDNL (ScreenPtr	pScreen,
		     CARD32	row,
		     CARD32	offset,
		     int	mode,
		     CARD32	*size)
{
  FatalError ("winShadowWindowProcDDNL () - Hmm... this function has never "\
	      "been called before.  Please send a message to "\
	      "cygwin-xfree@cygwin.com if you get this message.\n");
  return winShadowSetWindowLinearDDNL (pScreen, row, offset, mode, size);
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

#if CYGDEBUG
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
      IDirectDrawSurface_Release (pScreenPriv->pddsShadow4);
      xfree (pScreenInfo->pfb);
      pScreenInfo->pfb = NULL;
      pScreenPriv->pddsShadow4 = NULL;
    }

  /* Release the primary surface, if there is one */
  if (pScreenPriv->pddsPrimary4)
    {
      IDirectDrawSurface_Release (pScreenPriv->pddsPrimary4);
      pScreenPriv->pddsPrimary4 = NULL;
    }

  /* Free the DirectDraw object, if there is one */
  if (pScreenPriv->pdd)
    {
      IDirectDraw_RestoreDisplayMode (pScreenPriv->pdd);
      IDirectDraw_Release (pScreenPriv->pdd);
      pScreenPriv->pdd = NULL;
    }

  /* Free the DirectDraw4 object, if there is one */
  if (pScreenPriv->pdd4)
    {
      IDirectDraw_Release (pScreenPriv->pdd4);
      pScreenPriv->pdd4 = NULL;
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
  if (dwRedBits > dwGreenBits && dwRedBits > dwBlueBits)
    pScreenPriv->dwBitsPerRGB = dwRedBits;
  else if (dwGreenBits > dwRedBits && dwGreenBits > dwBlueBits)
    pScreenPriv->dwBitsPerRGB = dwGreenBits;
  else
    pScreenPriv->dwBitsPerRGB = dwBlueBits;

  ErrorF ("winInitVisualsShadowDDNL () - Masks %08x %08x %08x RGB %d d %d\n",
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
#if CYGDEBUG
      ErrorF ("winInitVisualsShadowDDNL () - Calling "\
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
	  ErrorF ("winInitVisualsShadowDDNL () - "\
		  "miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;

    default:
      ErrorF ("winInitVisualsDDNL () - Unknown screen depth\n");
      return FALSE;
    }
  
  /* Set DPI info */
  pScreenInfo->dwDPIx = 100;
  pScreenInfo->dwDPIy = 100;

  ErrorF ("winInitVisualsShadowDDNL () - Returning\n");

  return TRUE;
}

Bool
winAdjustVideoModeShadowDDNL (ScreenPtr pScreen)
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
      ErrorF ("winAdjustVideoModeShadowDDNL () - GetDC () failed\n");
      return FALSE;
    }

  /* Query GDI for current display depth */
  dwDepth = GetDeviceCaps (hdc, BITSPIXEL);

  /* Is GDI using a depth different than command line parameter? */
  if (dwDepth != pScreenInfo->dwDepth)
    {
      /* Warn user if GDI depth is different than depth specified */
      ErrorF ("winAdjustVideoModeShadowDDNL () - Command line depth: %d, "\
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
winBltExposedRegionsShadowDDNL (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  RECT			rcClient, rcSrc;
  HDC			hdcUpdate;
  PAINTSTRUCT		ps;
  HRESULT		ddrval = DD_OK;

  /* BeginPaint gives us an hdc that clips to the invalidated region */
  hdcUpdate = BeginPaint (pScreenPriv->hwndScreen, &ps);

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
  ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsPrimary4,
				   &rcClient,
				   pScreenPriv->pddsShadow4,
				   &rcSrc,
				   DDBLT_WAIT,
				   NULL);

  /* EndPaint frees the DC */
  EndPaint (pScreenPriv->hwndScreen, &ps);

  return TRUE;
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
      IDirectDrawSurface_Restore (pScreenPriv->pddsPrimary4);
    }

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
  pScreenPriv->pwinShadowWindow = winShadowWindowDDNL;
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

  return TRUE;
}


