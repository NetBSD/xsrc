/*
 *Copyright (C) 1994-2001 The XFree86 Project, Inc. All Rights Reserved.
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
/* $XFree86: xc/programs/Xserver/hw/xwin/winengine.c,v 1.1 2001/11/11 23:07:40 alanh Exp $ */

#include "win.h"


/*
 * Detect engines supported by current Windows version
 * DirectDraw version and hardware
 */

Bool
winDetectSupportedEngines (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  OSVERSIONINFO		osvi;
  HMODULE		hmodDirectDraw = NULL;

  /* Initialize the engine support flags */
  pScreenInfo->dwEnginesSupported = WIN_SERVER_SHADOW_GDI;

#if WIN_NATIVE_GDI_SUPPORT
  pScreenInfo->dwEnginesSupported |= WIN_SERVER_NATIVE_GDI;
#endif

  /* Get operating system version information */
  ZeroMemory (&osvi, sizeof (osvi));
  osvi.dwOSVersionInfoSize = sizeof (osvi);
  GetVersionEx (&osvi);

  /* Branch on platform ID */
  switch (osvi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
      /* Engine 4 is supported on NT only */
      ErrorF ("winDetectSupportedEngines () - Windows NT/2000\n");
      break;

    case VER_PLATFORM_WIN32_WINDOWS:
      /* Engine 4 is supported on NT only */
      ErrorF ("winDetectSupportedEngines () - Windows 95/98/Me\n");
      break;
    }

  /* Determine if DirectDraw is installed */
  hmodDirectDraw = LoadLibraryEx ("ddraw.dll", NULL, 0);

  /* Do we have DirectDraw? */
  if (hmodDirectDraw != NULL)
    {
      FARPROC		fpDirectDrawCreate = NULL;
      LPDIRECTDRAW	lpdd = NULL;
      LPDIRECTDRAW4	lpdd4 = NULL;
      HRESULT		ddrval;

      /* Try to get the DirectDrawCreate address */
      fpDirectDrawCreate = GetProcAddress (hmodDirectDraw,
					   "DirectDrawCreate");
      
      /* Did the proc name exist? */
      if (fpDirectDrawCreate == NULL)
	{
	  /* No DirectDraw support */
	  return TRUE;
	}

      /* DirectDrawCreate exists, try to call it */
      /* Create a DirectDraw object, store the address at lpdd */
      ddrval = (*fpDirectDrawCreate) (NULL,
				      (void**) &lpdd,
				      NULL);
      if (FAILED (ddrval))
	{
	  /* No DirectDraw support */
	  ErrorF ("winDetectSupportedEngines () - DirectDraw not installed\n");
	  return TRUE;
	}
      else
	{
	  /* We have DirectDraw */
	  ErrorF ("winDetectSupportedEngines () - DirectDraw installed\n");
	  pScreenInfo->dwEnginesSupported |= WIN_SERVER_SHADOW_DD;

	  /* Allow PrimaryDD engine if NT */
	  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	    {
	      pScreenInfo->dwEnginesSupported |= WIN_SERVER_PRIMARY_DD;
	      ErrorF ("winDetectSupportedEngines () - Allowing PrimaryDD\n");
	    }
	}
      
      /* Try to query for DirectDraw4 interface */
      ddrval = IDirectDraw_QueryInterface (lpdd,
					   &IID_IDirectDraw4,
					   (LPVOID*) &lpdd4);
      if (SUCCEEDED (ddrval))
	{
	  /* We have DirectDraw4 */
	  ErrorF ("winDetectSupportedEngines () - DirectDraw4 installed\n");
	  pScreenInfo->dwEnginesSupported |= WIN_SERVER_SHADOW_DDNL;
	}

      /* Cleanup DirectDraw interfaces */
      if (lpdd4 != NULL)
	IDirectDraw_Release (lpdd4);
      if (lpdd != NULL)
	IDirectDraw_Release (lpdd);

      /* Unload the DirectDraw library */
      FreeLibrary (hmodDirectDraw);
      hmodDirectDraw = NULL;
    }

  ErrorF ("winDetectSupportedEngines () - Returning, supported engines %08x\n",
	  pScreenInfo->dwEnginesSupported);

  return TRUE;
}


/*
 * Set the engine type, depending on the engines
 * supported for this screen, and whether the user
 * suggested an engine type
 */

Bool
winSetEngine (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HDC			hdc;
  DWORD			dwDepth;

  /* Get a DC */
  hdc = GetDC (NULL);
  if (hdc == NULL)
    {
      ErrorF ("winSetEngine () - Couldn't get an HDC\n");
      return FALSE;
    }

  /*
   * pScreenInfo->dwDepth may be 0 to indicate that the current screen
   * depth is to be used.  Thus, we must query for the current display
   * depth here.
   */
  dwDepth = GetDeviceCaps (hdc, BITSPIXEL);

  /* Release the DC */
  ReleaseDC (NULL, hdc);
  hdc = NULL;

  /* ShadowGDI is the only engine that supports windowed PseudoColor */
  if (dwDepth == 8 && !pScreenInfo->fFullScreen)
    {
      ErrorF ("winSetEngine () - Windowed && PseudoColor => ShadowGDI\n");
      pScreenInfo->dwEngine = WIN_SERVER_SHADOW_GDI;

      /* Set engine function pointers */
      winSetEngineFunctionsShadowGDI (pScreen);
      return TRUE;
    }

  /* If the user's choice is supported, we'll use that */
  if (pScreenInfo->dwEnginesSupported & pScreenInfo->dwEnginePreferred)
    {
      ErrorF ("winSetEngine () - Using user's preference: %d\n",
	      pScreenInfo->dwEnginePreferred);
      pScreenInfo->dwEngine = pScreenInfo->dwEnginePreferred;

      /* Setup engine function pointers */
      switch (pScreenInfo->dwEngine)
	{
	case WIN_SERVER_SHADOW_GDI:
	  winSetEngineFunctionsShadowGDI (pScreen);
	  break;
	case WIN_SERVER_SHADOW_DD:
	  winSetEngineFunctionsShadowDD (pScreen);
	  break;
	case WIN_SERVER_SHADOW_DDNL:
	  winSetEngineFunctionsShadowDDNL (pScreen);
	  break;
	case WIN_SERVER_PRIMARY_DD:
	  winSetEngineFunctionsPrimaryDD (pScreen);
	  break;
	case WIN_SERVER_NATIVE_GDI:
	  winSetEngineFunctionsNativeGDI (pScreen);
	  break;
	default:
	  FatalError ("winSetEngine () - Invalid engine type\n");
	}
      return TRUE;
    }

  /* ShadowDDNL has good performance, so why not */
  if (pScreenInfo->dwEnginesSupported & WIN_SERVER_SHADOW_DDNL)
    {
      ErrorF ("winSetEngine () - Using Shadow DirectDraw NonLocking\n");
      pScreenInfo->dwEngine = WIN_SERVER_SHADOW_DDNL;

      /* Set engine function pointers */
      winSetEngineFunctionsShadowDDNL (pScreen);
      return TRUE;
    }

  /* ShadowDD is next in line */
  if (pScreenInfo->dwEnginesSupported & WIN_SERVER_SHADOW_DD)
    {
      ErrorF ("winSetEngine () - Using Shadow DirectDraw\n");
      pScreenInfo->dwEngine = WIN_SERVER_SHADOW_DD;

      /* Set engine function pointers */
      winSetEngineFunctionsShadowDD (pScreen);
      return TRUE;
    }

  /* ShadowGDI is next in line */
  if (pScreenInfo->dwEnginesSupported & WIN_SERVER_SHADOW_GDI)
    {
      ErrorF ("winSetEngine () - Using Shadow GDI DIB\n");
      pScreenInfo->dwEngine = WIN_SERVER_SHADOW_GDI;

      /* Set engine function pointers */
      winSetEngineFunctionsShadowGDI (pScreen);
      return TRUE;
    }

  return TRUE;
}
