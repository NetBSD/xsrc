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
/* $XFree86: xc/programs/Xserver/hw/xwin/winpfbdd.c,v 1.5 2001/05/14 16:52:33 alanh Exp $ */

#include "win.h"

/*
 * Create a DirectDraw primary surface 
 */
Bool
winAllocateFBPrimaryDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;  
  HRESULT		ddrval = DD_OK;
  DDSURFACEDESC		ddsd;
  DDSURFACEDESC		*pddsdPrimary = NULL;
  DDSURFACEDESC		*pddsdOffscreen = NULL;
  RECT			rcClient;
  
  ErrorF ("winAllocateFBPrimaryDD ()\n");

  /* Get client area location in screen coords */
  GetClientRect (pScreenPriv->hwndScreen, &rcClient);
  MapWindowPoints (pScreenPriv->hwndScreen,
		   HWND_DESKTOP,
		   (LPPOINT)&rcClient, 2);

  /* Create a DirectDraw object, store the address at lpdd */
  ddrval = DirectDrawCreate (NULL, &pScreenPriv->pdd, NULL);
  if (ddrval != DD_OK)
    FatalError ("winAllocateFBPrimaryDD () - Could not start DirectDraw\n");

  ErrorF ("winAllocateFBPrimaryDD () - Created and initialized DD\n");

  /* Are we windowed or fullscreen? */
  if (pScreenInfo->fFullScreen)
    {
      /* Full screen mode */
      ddrval = IDirectDraw_SetCooperativeLevel (pScreenPriv->pdd,
						pScreenPriv->hwndScreen,
						DDSCL_FULLSCREEN
						| DDSCL_EXCLUSIVE);
      if (FAILED (ddrval))
	FatalError ("winAllocateFBPrimaryDD () - Could not set "\
		    "cooperative level\n");

      /* Change the video mode to the mode requested */
      ddrval = IDirectDraw_SetDisplayMode (pScreenPriv->pdd,
					   pScreenInfo->dwWidth,
					   pScreenInfo->dwHeight,
					   pScreenInfo->dwDepth);
       if (FAILED (ddrval))
	FatalError ("winAllocateFBPrimaryDD () - Could not set "\
		    "full screen display mode\n");
    }
  else
    {
      /* Windowed mode */
      ddrval = IDirectDraw_SetCooperativeLevel (pScreenPriv->pdd,
						pScreenPriv->hwndScreen,
						DDSCL_NORMAL);
      if (FAILED (ddrval))
	FatalError ("winAllocateFBPrimaryDD () - Could not set "\
		    "cooperative level\n");
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
       FatalError ("winAllocateFBPrimaryDD () - Could not create primary "\
		  "surface %08x\n", ddrval);

  ErrorF ("winAllocateFBPrimaryDD () - Created primary\n");

  /* Allocate a DD surface description for our screen privates */
  pddsdPrimary = pScreenPriv->pddsdPrimary
    = xalloc (sizeof (DDSURFACEDESC));
  if (pddsdPrimary == NULL)
    FatalError ("winAllocateFBPrimaryDD () - Could not allocate surface "\
		"description memory\n");
  ZeroMemory (pddsdPrimary, sizeof (*pddsdPrimary));
  pddsdPrimary->dwSize = sizeof (*pddsdPrimary);

  /* Describe the offscreen surface to be created */
  /*
   * NOTE: Do not use a DDSCAPS_VIDEOMEMORY surface,
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
				      &pScreenPriv->pddsOffscreen,
				      NULL);
  if (ddrval != DD_OK)
    FatalError ("winAllocateFBPrimaryDD () - Could not create shadow "\
		"surface\n");
  
  ErrorF ("winAllocateFBPrimaryDD () - Created offscreen\n");

  /* Allocate a DD surface description for our screen privates */
  pddsdOffscreen = pScreenPriv->pddsdOffscreen
    = xalloc (sizeof (DDSURFACEDESC));
  if (pddsdOffscreen == NULL)
    FatalError ("winAllocateFBPrimaryDD () - Could not allocate surface "\
		"description memory\n");
  ZeroMemory (pddsdOffscreen, sizeof (*pddsdOffscreen));
  pddsdOffscreen->dwSize = sizeof (*pddsdOffscreen);

  ErrorF ("winAllocateFBPrimaryDD () - Locking primary\n");

  /* Lock the primary surface */
  ddrval = IDirectDrawSurface_Lock (pScreenPriv->pddsPrimary,
				    pScreenInfo->fFullScreen ? NULL:&rcClient,
				    pddsdPrimary,
				    DDLOCK_WAIT,
				    NULL);
  if (ddrval != DD_OK || pddsdPrimary->lpSurface == NULL)
    FatalError ("winAllocateFBPrimaryDD () - Could not lock "\
		"primary surface\n");

  ErrorF ("winAllocateFBPrimaryDD () - Locked primary\n");

  /* We don't know how to deal with anything other than RGB */
  if (!(pddsdPrimary->ddpfPixelFormat.dwFlags & DDPF_RGB))
    FatalError ("winAllocateFBPrimaryDD () - Color format other than RGB\n");

  /* Grab the pitch, and memory pointer from the surface desc */
  pScreenInfo->dwStrideBytes = pddsdPrimary->u.lPitch;
  pScreenInfo->dwStride = (pScreenInfo->dwStrideBytes * 8)
    / pScreenInfo->dwDepth;
  pScreenInfo->pfb = pddsdPrimary->lpSurface;
  
  /* Grab the color depth and masks from the surface description */
  pScreenInfo->dwDepth = pddsdPrimary->ddpfPixelFormat.u.dwRGBBitCount;
  pScreenPriv->dwRedMask = pddsdPrimary->ddpfPixelFormat.u2.dwRBitMask;
  pScreenPriv->dwGreenMask = pddsdPrimary->ddpfPixelFormat.u3.dwGBitMask;
  pScreenPriv->dwBlueMask = pddsdPrimary->ddpfPixelFormat.u4.dwBBitMask;

  ErrorF ("winAllocateFBPrimaryDD () - Returning\n");

  return TRUE;
}

/*
 * Call the wrapped CloseScreen function.
 * 
 * Free our resources and private structures.
 */
Bool
winCloseScreenPrimaryDD (int nIndex, ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  Bool			fReturn;
  
  ErrorF ("winCloseScreenPrimaryDD () - Freeing screen resources\n");

  /* Flag that the screen is closed */
  pScreenPriv->fClosed = TRUE;
  pScreenPriv->fActive = FALSE;

  /* Call the wrapped CloseScreen procedure */
  pScreen->CloseScreen = pScreenPriv->CloseScreen;
  fReturn = (*pScreen->CloseScreen) (nIndex, pScreen);

  /* Delete the window property */
  RemoveProp (pScreenPriv->hwndScreen, WIN_SCR_PROP);

  /* Free the offscreen surface, if there is one */
  if (pScreenPriv->pddsOffscreen)
    {
      IDirectDrawSurface_Unlock (pScreenPriv->pddsOffscreen, NULL);
      IDirectDrawSurface_Release (pScreenPriv->pddsOffscreen);
      pScreenPriv->pddsOffscreen = NULL;
    }

  /* Release the primary surface, if there is one */
  if (pScreenPriv->pddsPrimary)
    {
      IDirectDrawSurface_Unlock (pScreenPriv->pddsPrimary, NULL);
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
winInitVisualsPrimaryDD (ScreenPtr pScreen)
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
  
  ErrorF ("winInitVisualsPrimaryDD () - Masks: %08x %08x %08x bpRGB: %d\n",
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
	  ErrorF ("winInitVisualsPrimaryDD () - " \
		  "miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
      break;

    case 8:
#if CYGDEBUG
      ErrorF ("winInitVisuals () - Calling miSetVisualTypesAndMasks\n");
#endif /* CYGDEBUG */
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     PseudoColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     PseudoColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsPrimaryDD () - "\
		  "miSetVisualTypesAndMasks failed\n");
	  return FALSE;
	}
#if CYGDEBUG
      ErrorF ("winInitVisualsPrimaryDD () - Returned from "\
	      "miSetVisualTypesAndMasks\n");
#endif /* CYGDEBUG */
      break;

    default:
      ErrorF ("winInitVisualsPrimaryDD () - Unknown screen depth\n");
      return FALSE;
    }

  /* Set DPI info */
  pScreenInfo->dwDPIx = 100;
  pScreenInfo->dwDPIy = 100;

  ErrorF ("winInitVisualsPrimaryDD () - Returning\n");

  return TRUE;
}

Bool
winAdjustVideoModePrimaryDD (ScreenPtr pScreen)
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
      ErrorF ("winAdjustVideoModePrimaryDD () - GetDC failed\n");
      return FALSE;
    }

  /* Query GDI for current display depth */
  dwDepth = GetDeviceCaps (hdc, BITSPIXEL);

  /* Is GDI using a depth different than command line parameter? */
  if (dwDepth != pScreenInfo->dwDepth)
    {
      /* Warn user if GDI depth is different than depth specified */
      ErrorF ("winAdjustVideoModePrimaryDD () - Command line depth: %d, "\
	      "using depth: %d\n", pScreenInfo->dwDepth, dwDepth);

      /* We'll use GDI's depth */
      pScreenInfo->dwDepth = dwDepth;
    }
  
  /* Release our DC */
  ReleaseDC (NULL, hdc);

  return TRUE;
}

Bool
winActivateAppPrimaryDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  RECT			rcSrc, rcClient;
  HRESULT		ddrval = DD_OK;

  /*
   * We need to blit our offscreen fb to
   * the screen when we are activated, and we need to point
   * the fb code back to the primary surface memory.
   */
  if (pScreenPriv != NULL
      && pScreenPriv->pddsPrimary != NULL
      && pScreenPriv->pddsOffscreen != NULL
      && pScreenPriv->fActive)
    {
      /* We are activating */
      ddrval = IDirectDrawSurface_IsLost (pScreenPriv->pddsOffscreen);
      if (ddrval == DD_OK)
	{
	  IDirectDrawSurface_Unlock (pScreenPriv->pddsOffscreen,
				     NULL);
	  /*
	   * We don't check for an error from Unlock, because it
	   * doesn't matter if the Unlock failed.
	   */
	}
	      
      /* Restore both surfaces, just cause I like it that way */
      IDirectDrawSurface_Restore (pScreenPriv->pddsOffscreen);
      IDirectDrawSurface_Restore (pScreenPriv->pddsPrimary);
			      
      /* Get client area in screen coords */
      GetClientRect (pScreenPriv->hwndScreen, &rcClient);
      MapWindowPoints (pScreenPriv->hwndScreen,
		       HWND_DESKTOP,
		       (LPPOINT)&rcClient, 2);
	      
      /* Setup a source rectangle */
      rcSrc.left = 0;
      rcSrc.top = 0;
      rcSrc.right = pScreenInfo->dwWidth;
      rcSrc.bottom = pScreenInfo->dwHeight;

      ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsPrimary,
				       &rcClient,
				       pScreenPriv->pddsOffscreen,
				       &rcSrc,
				       DDBLT_WAIT,
				       NULL);
      if (FAILED (ddrval))
	FatalError ("winWindowProc () - Failed blitting offscreen "\
		    "surface to primary surface %08x\n", ddrval);

      /* Lock the primary surface */
      ddrval = IDirectDrawSurface_Lock (pScreenPriv->pddsPrimary,
					&rcClient,
					pScreenPriv->pddsdPrimary,
					DDLOCK_WAIT,
					NULL);
      if (ddrval != DD_OK
	  || pScreenPriv->pddsdPrimary->lpSurface == NULL)
	FatalError ("winWindowProc () - Could not lock "\
		    "primary surface\n");

      /* Notify FB of the new memory pointer */
      winUpdateFBPointer (pScreen,
			  pScreenPriv->pddsdPrimary->lpSurface);

      /*
       * Register the Alt-Tab combo as a hotkey so we can copy
       * the primary framebuffer before the display mode changes
       */
      RegisterHotKey (pScreenPriv->hwndScreen, 1, MOD_ALT, 9);
    }

  return TRUE;
}

/* Set engine specific functions */
Bool
winSetEngineFunctionsPrimaryDD (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  
  /* Set our pointers */
  pScreenPriv->pwinAllocateFB = winAllocateFBPrimaryDD;
  pScreenPriv->pwinShadowUpdate
    = (winShadowUpdateProcPtr) (void (*)())NoopDDA;
  pScreenPriv->pwinShadowWindow
    = (winShadowWindowProcPtr) (void (*)())NoopDDA;
  pScreenPriv->pwinCloseScreen = winCloseScreenPrimaryDD;
  pScreenPriv->pwinInitVisuals = winInitVisualsPrimaryDD;
  pScreenPriv->pwinAdjustVideoMode = winAdjustVideoModePrimaryDD;
  if (pScreenInfo->fFullScreen)
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowFullScreen;
  else
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowWindowed;
  pScreenPriv->pwinFinishScreenInit = winFinishScreenInitFB;
  pScreenPriv->pwinBltExposedRegions
    = (winBltExposedRegionsProcPtr) (void (*)())NoopDDA;
  pScreenPriv->pwinActivateApp = winActivateAppPrimaryDD;

  return TRUE;
}
