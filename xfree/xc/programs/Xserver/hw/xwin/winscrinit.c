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
/* $XFree86: xc/programs/Xserver/hw/xwin/winscrinit.c,v 1.7 2001/05/14 16:52:33 alanh Exp $ */

#include "win.h"

/*
 * Create a full screen window
 */
void
winCreateBoundingWindowFullScreen (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  int			iWidth = pScreenInfo->dwWidth;
  int			iHeight = pScreenInfo->dwHeight;
  HWND			*phwnd = &pScreenPriv->hwndScreen;
  WNDCLASS		wc;

#if CYGDEBUG
  ErrorF ("winCreateBoundingWindowFullScreen ()\n");
#endif

  /* Setup our window class */
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = winWindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle (NULL);
  wc.hIcon = 0;
  wc.hCursor = 0;
  wc.hbrBackground = 0;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = WINDOW_CLASS;
  RegisterClass (&wc);

  /* Create the window */
  *phwnd = CreateWindowExA (WS_EX_TOPMOST,	/* Extended styles */
			    WINDOW_CLASS,	/* Class name */
			    WINDOW_TITLE,	/* Window name */
			    WS_POPUP,
			    0,			/* Horizontal position */
			    0,			/* Vertical position */
			    iWidth,		/* Right edge */ 
			    iHeight,		/* Bottom edge */
			    (HWND) NULL,	/* No parent or owner window */
			    (HMENU) NULL,	/* No menu */
			    GetModuleHandle (NULL),/* Instance handle */
			    pScreenPriv);	/* ScreenPrivates */

  /* Branch on the server engine */
  switch (pScreenInfo->dwEngine)
    {
    case WIN_SERVER_SHADOW_GDI:
      /* Show the window */
      ShowWindow (*phwnd, SW_SHOWMAXIMIZED);
      break;

    default:
      /* Hide the window */
      ShowWindow (*phwnd, SW_HIDE);
      break;
    }
  
  /* Send first paint message */
  UpdateWindow (*phwnd);

  /* Attempt to bring our window to the top of the display */
  BringWindowToTop (*phwnd);
}

/*
 * Create our primary Windows display window
 */
void
winCreateBoundingWindowWindowed (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  int			iWidth = pScreenInfo->dwWidth;
  int			iHeight = pScreenInfo->dwHeight;
  HWND			*phwnd = &pScreenPriv->hwndScreen;
  WNDCLASS		wc;
  RECT			rcClient, rcWorkArea;

  /* Setup our window class */
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = winWindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle (NULL);
  wc.hIcon = 0;
  wc.hCursor = 0;
  wc.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = WINDOW_CLASS;
  RegisterClass (&wc);

  /* Get size of work area */
  SystemParametersInfo (SPI_GETWORKAREA, 0, &rcWorkArea, 0);

#if CYGDEBUG
  ErrorF ("winCreateBoundingWindowWindowed () - WorkArea width %d height %d\n",
	  rcWorkArea.right - rcWorkArea.left,
	  rcWorkArea.bottom - rcWorkArea.top);
#endif

  /* Adjust the window width and height for border and title bars */
  iWidth += 2 * GetSystemMetrics (SM_CXFIXEDFRAME);
  iHeight += 2 * GetSystemMetrics (SM_CYFIXEDFRAME) 
    + GetSystemMetrics (SM_CYCAPTION);
  
  /* Trim window width to fit work area */
  if (iWidth > (rcWorkArea.right - rcWorkArea.left))
    {
      iWidth = rcWorkArea.right - rcWorkArea.left;
      pScreenInfo->dwWidth = iWidth -
	2 * GetSystemMetrics (SM_CXFIXEDFRAME);
    }
  
  /* Trim window height to fit work area */
  if (iHeight >= (rcWorkArea.bottom - rcWorkArea.top))
    {
      /*
       * FIXME: Currently chopping 1 off the maximum height
       * to allow hidden start bars to pop up when the mouse
       * reaches the bottom of the screen.
       * 
       * This only works if the start menu is at the bottom
       * of the screen.
       */
      iHeight = rcWorkArea.bottom - rcWorkArea.top - 1;
      pScreenInfo->dwHeight = iHeight
	- 2 * GetSystemMetrics (SM_CYFIXEDFRAME)
	- GetSystemMetrics (SM_CYCAPTION);
    }
  
#if CYGDEBUG
  ErrorF ("winCreateBoundingWindowWindowed () - Adjusted width: %d "\
	  "height: %d\n",
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight);
#endif
    
  /* Create the window */
  *phwnd = CreateWindowExA (0,			/* Extended styles */
			    WINDOW_CLASS,	/* Class name */
			    WINDOW_TITLE,	/* Window name */
			    WS_OVERLAPPED
			    | WS_CAPTION
			    | WS_SYSMENU
			    | WS_MINIMIZEBOX,	/* Almost an OverlappedWindow */
			    0,			/* Horizontal position */
			    0,			/* Vertical position */
			    iWidth,		/* Right edge */
			    iHeight,		/* Bottom edge */
			    (HWND) NULL,	/* No parent or owner window */
			    (HMENU) NULL,	/* No menu */
			    GetModuleHandle (NULL),/* Instance handle */
			    pScreenPriv);	/* ScreenPrivates */

  /* Get the client area coordinates */
  GetClientRect (*phwnd, &rcClient);
  ErrorF ("winCreateBoundingWindowWindowed () - WindowClient "\
	  "width %d height %d\n",
	  rcClient.right - rcClient.left,
	  rcClient.bottom - rcClient.top);
  MapWindowPoints (*phwnd, HWND_DESKTOP, (LPPOINT)&rcClient, 2);

  /* Show the window */
  ShowWindow (*phwnd, SW_SHOW);
  UpdateWindow (*phwnd);
  
  /* Attempt to bring our window to the top of the display */
  BringWindowToTop (*phwnd);
}

/*
 * Determine what type of screen we are initializing
 * and call the appropriate procedure to intiailize
 * that type of screen.
 */
Bool
winScreenInit (int index,
	       ScreenPtr pScreen,
	       int argc, char **argv)
{
  winScreenInfoPtr      pScreenInfo = &g_winScreens[index];
  winPrivScreenPtr	pScreenPriv;

  /* Allocate privates for this screen */
  winAllocatePrivates (pScreen);

  /* Get a pointer to the privates structure that was allocated */
  pScreenPriv = winGetScreenPriv (pScreen);

  /* Save a pointer to this screen in the screen info structure */
  pScreenInfo->pScreen = pScreen;

  /* Save a pointer to the screen info in the screen privates structure */
  /* This allows us to get back to the screen info from a sceen pointer */
  pScreenPriv->pScreenInfo = pScreenInfo;

  /* Detect which engines are supported */
  winDetectSupportedEngines (pScreen);

  /* Determine which engine to use */
  if (!winSetEngine (pScreen))
    {
      ErrorF ("winScreenInit () - winSetEngine () failed\n");
      return FALSE;
    }

  /* Adjust the video mode for our engine type */
  if (!(*pScreenPriv->pwinAdjustVideoMode) (pScreen))
    {
      ErrorF ("winScreenInit () - winAdjustVideoMode () failed\n");
      return FALSE;
    }

  /* Call the engine dependent screen initialization procedure */
  if (!((*pScreenPriv->pwinFinishScreenInit) (index, pScreen, argc, argv)))
    {
      ErrorF ("winScreenInit () - winFinishScreenInit () failed\n");
      return FALSE;
    }

  return TRUE;
}

/* See Porting Layer Definition - p. 20 */
Bool
winFinishScreenInitFB (int index,
		       ScreenPtr pScreen,
		       int argc, char **argv)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  Bool			fReturn = TRUE;
  char			*pbits = NULL;

  /* Initial display parameters */
  pScreenInfo->dwBPP = winBitsPerPixel (pScreenInfo->dwDepth);

#if CYGDEBUG
  ErrorF ("winFinishScreenInitFB () - dwBPP: %d\n", pScreenInfo->dwBPP);
#endif

  /* Create display window */
  (*pScreenPriv->pwinCreateBoundingWindow) (pScreen);

  /* Set the padded screen width */
  pScreenInfo->dwPaddedWidth = PixmapBytePad (pScreenInfo->dwWidth,
					      pScreenInfo->dwDepth);

  /* Clear the visuals list */
  winClearVisualTypes ();

  /* Create framebuffer */
  fReturn = (*pScreenPriv->pwinAllocateFB) (pScreen);
  if (!fReturn)
    {
      ErrorF ("winFinishScreenInitFB () - Could not allocate framebuffer\n");
      return FALSE;
    }

  /* Init visuals */
  fReturn = (*pScreenPriv->pwinInitVisuals) (pScreen);
  if (!fReturn)
    {
      ErrorF ("winFinishScreenInitFB () - winInitVisuals failed\n");
      return FALSE;
    }

  /* Setup a local variable to point to the framebuffer */
  pbits = pScreenInfo->pfb;
  
  /* Apparently we need this for the render extension */
  miSetPixmapDepths ();

  /* Initialize the fb code */
  if (!fbScreenInit (pScreen,
		     pScreenInfo->pfb,
		     pScreenInfo->dwWidth, pScreenInfo->dwHeight,
		     pScreenInfo->dwDPIx, pScreenInfo->dwDPIy,
		     pScreenInfo->dwStride,
		     pScreenInfo->dwBPP))
    {
      ErrorF ("winFinishScreenInitFB () - fbScreenInit failed\n");
      return FALSE;
    }

  pScreen->GetWindowPixmap = winGetWindowPixmap;
  pScreen->SetWindowPixmap = winSetWindowPixmap;

#ifdef RENDER
  /* Render extension initialization, calls miPictureInit */
  fbPictureInit (pScreen, NULL, 0);
#endif

  /* Setup the cursor routines */
  miDCInitialize (pScreen, &g_winPointerCursorFuncs);

  /* Create a default colormap */
  fReturn = fbCreateDefColormap (pScreen);
  if (!fReturn)
    {
      ErrorF ("winFinishScreenInitFB () - Could not create colormap\n");
      return FALSE;
    }

  /* Initialize the shadow framebuffer layer */
  if (pScreenInfo->dwEngine == WIN_SERVER_SHADOW_GDI
      || pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DD
      || pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DDNL)
    {
      shadowInit (pScreen,
		  pScreenPriv->pwinShadowUpdate,
		  pScreenPriv->pwinShadowWindow);
    }

  /*
   * Register our block and wakeup handlers; these procedures
   * process messages in our Windows message queue; specifically,
   * they process mouse and keyboard input.
   */
  RegisterBlockAndWakeupHandlers (winBlockHandler,
				  winWakeupHandler,
				  pScreen);

  /* Wrap either fb's or shadow's CloseScreen with our CloseScreen */
  pScreenPriv->CloseScreen = pScreen->CloseScreen;
  pScreen->CloseScreen = pScreenPriv->pwinCloseScreen;

  /* See Porting Layer Definition - p. 33 */
  /* SaveScreen () has something to do with screen savers */
  /* Our SaveScreen () does nothing */
  pScreen->SaveScreen = winSaveScreen;

  /* Tell the server that we are enabled */
  pScreenPriv->fEnabled = TRUE;

  return fReturn;
}

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
	  /* FIXME: Assuming we have DirectDraw3+ */
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

/* See Porting Layer Definition - p. 33 */
Bool
winSaveScreen (ScreenPtr pScreen, int on)
{
  return TRUE;
}

/*
 *
 *
 *
 *
 * TEST CODE BELOW - NOT USED IN NORMAL COMPILATION
 *
 *
 *
 *
 *
 */

/* See Porting Layer Definition - p. 20 */
Bool
winFinishScreenInitNativeGDI (int index,
			      ScreenPtr pScreen,
			      int argc, char **argv)
{
  winScreenInfoPtr      pScreenInfo = &g_winScreens[index];
  PictFormatPtr         formats = NULL;
  int                   nformats = 0;
  Bool                  fReturn = FALSE;
  int			xsize, ysize;
  int			dpix = 75, dpiy = 75;
  char                  *pbits = NULL;
  VisualPtr		pVisuals = NULL;
  DepthPtr		pDepths = NULL;
  VisualID		rootVisual = 0;
  int			nVisuals = 0, nDepths = 0, nRootDepth = 0;
  winPrivScreenPtr	pScreenPriv = NULL;
  
  ErrorF ("winScreenInit ()\n");

  if (!winAllocatePrivates (pScreen))
    {
      ErrorF ("winFinishScreenInitNativeGDI () winAllocatePrivates failed\n");
      return FALSE;
    }

  /* Get a pointer to the privates structure that was allocated */
  pScreenPriv = winGetScreenPriv (pScreen);

  /* Save a pointer to this screen in the screen info structure */
  pScreenInfo->pScreen = pScreen;

  /* Save a pointer to the screen info in the sceen privates structure */
  /* This allows us to get back to the screen info from a sceen pointer */
  pScreenPriv->pScreenInfo = pScreenInfo;

  /* Initial display parameters */
  pScreenInfo->dwPaddedWidth = PixmapBytePad (pScreenInfo->dwWidth,
					      pScreenInfo->dwDepth);
  pScreenInfo->dwBPP = winBitsPerPixel (pScreenInfo->dwDepth);
  ErrorF ("winScreenInit () - screen (%dx%dx%d)\n",
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight, pScreenInfo->dwDepth);

  /* Copy the width and height into local variables */
  xsize = pScreenInfo->dwWidth;
  ysize = pScreenInfo->dwHeight;

  /* Create primary display window */
  winCreateBoundingWindowWindowed (pScreen);
  
  /* Simple screen information */
  pScreen->width = xsize;
  pScreen->height = ysize;
  pScreen->mmWidth = (xsize * 254 + dpix * 5) / (dpix * 10);
  pScreen->mmHeight = (ysize * 254 + dpiy * 5) / (dpiy * 10);
  pScreen->defColormap = FakeClientID (0);
  pScreen->minInstalledCmaps = 1;
  pScreen->maxInstalledCmaps = 1;
  pScreen->backingStoreSupport = NotUseful;
  pScreen->saveUnderSupport = NotUseful;

  pScreen->GetScreenPixmap = miGetScreenPixmap;
  pScreen->SetScreenPixmap = miSetScreenPixmap;
  
  /* Region Routines */
#ifdef NEED_SCREEN_REGIONS
  pScreen->RegionCreate = miRegionCreate;
  pScreen->RegionInit = miRegionInit;
  pScreen->RegionCopy = miRegionCopy;
  pScreen->RegionDestroy = miRegionDestroy;
  pScreen->RegionUninit = miRegionUninit;
  pScreen->Intersect = miIntersect;
  pScreen->Union = miUnion;
  pScreen->Subtract = miSubtract;
  pScreen->Inverse = miInverse;
  pScreen->RegionReset = miRegionReset;
  pScreen->TranslateRegion = miTranslateRegion;
  pScreen->RectIn = miRectIn;
  pScreen->PointInRegion = miPointInRegion;
  pScreen->RegionNotEmpty = miRegionNotEmpty;
  pScreen->RegionBroken = miRegionBroken;
  pScreen->RegionBreak = miRegionBreak;
  pScreen->RegionEmpty = miRegionEmpty;
  pScreen->RegionExtents = miRegionExtents;
  pScreen->RegionAppend = miRegionAppend;
  pScreen->RegionValidate = miRegionValidate;
#endif /* NEED_SCREEN_REGIONS */
  pScreen->BitmapToRegion = winPixmapToRegionNativeGDI;
#ifdef NEED_SCREEN_REGIONS
  pScreen->RectsToRegion = miRectsToRegion;
#endif /* NEED_SCREEN_REGIONS */
  
  /* Cursor Routines for a Screen */
  /* See mi/midispcur.c - miDCInitialize() */
  /* See Porting Layer Definition - pp. 25-26 */
  pScreen->PointerNonInterestBox = (PointerNonInterestBoxProcPtr) 0;

  /* Colormap Routines */
  pScreen->CreateColormap = winInitializeColormapNativeGDI;
  pScreen->DestroyColormap = (void (*)())NoopDDA;
  pScreen->InstallColormap = winInstallColormapNativeGDI;
  pScreen->UninstallColormap = winUninstallColormapNativeGDI;
  pScreen->ListInstalledColormaps = winListInstalledColormapsNativeGDI;
  pScreen->StoreColors = (void (*)())NoopDDA;
  pScreen->ResolveColor = winResolveColorNativeGDI;

  /* Fonts */
  pScreen->RealizeFont = winRealizeFontNativeGDI;
  pScreen->UnrealizeFont = winUnrealizeFontNativeGDI;

  /* Other Screen Routines */
  pScreen->GetImage = miGetImage;
  pScreen->GetSpans = winGetSpansNativeGDI;
  pScreen->QueryBestSize = winQueryBestSizeNativeGDI;
  pScreen->SourceValidate = (SourceValidateProcPtr) 0;
  pScreen->SaveScreen = winSaveScreen;  
  pScreen->CloseScreen = miCloseScreen;
  pScreen->CreateScreenResources = miCreateScreenResources;

  /* Pixmaps */
  pScreen->CreatePixmap = winCreatePixmapNativeGDI;
  pScreen->DestroyPixmap = winDestroyPixmapNativeGDI;
  pScreen->ModifyPixmapHeader = miModifyPixmapHeader;

  /* Window Procedures */
  pScreen->CreateWindow = winCreateWindowNativeGDI;
  pScreen->DestroyWindow = winDestroyWindowNativeGDI;
  pScreen->PositionWindow = winPositionWindowNativeGDI;
  pScreen->ChangeWindowAttributes = winChangeWindowAttributesNativeGDI;
  pScreen->RealizeWindow = winMapWindowNativeGDI;
  pScreen->UnrealizeWindow = winUnmapWindowNativeGDI;
  pScreen->ValidateTree = miValidateTree;
  pScreen->PostValidateTree = (PostValidateTreeProcPtr) 0;
  pScreen->WindowExposures = miWindowExposures;
  pScreen->ClipNotify = (ClipNotifyProcPtr) 0;

  /* Window Painting Procedures */
  pScreen->ClearToBackground = miClearToBackground;
  pScreen->CopyWindow = winCopyWindowNativeGDI;
  pScreen->PaintWindowBackground = miPaintWindow;
  pScreen->PaintWindowBorder = miPaintWindow;
  
  /* Screen Operation for Backing Store */
  pScreen->SaveDoomedAreas = 0;
  pScreen->RestoreAreas = 0;
  pScreen->TranslateBackingStore = 0;
  pScreen->ExposeCopy = 0;
  pScreen->ClearBackingStore = 0;
  pScreen->DrawGuarantee = 0;
  
  /* Screen Operations for Multi-Layered Framebuffers */
  pScreen->MarkWindow = miMarkWindow;
  pScreen->MarkOverlappedWindows = miMarkOverlappedWindows;
  pScreen->ChangeSaveUnder = miChangeSaveUnder;
  pScreen->PostChangeSaveUnder = miPostChangeSaveUnder;
  pScreen->MoveWindow = miMoveWindow;
  pScreen->ResizeWindow = miSlideAndSizeWindow;
  pScreen->GetLayerWindow = miGetLayerWindow;
  pScreen->HandleExposures = miHandleValidateExposures;
  pScreen->ReparentWindow = (ReparentWindowProcPtr) 0;
#ifdef SHAPE
  pScreen->SetShape = miSetShape;
#endif
  pScreen->ChangeBorderWidth = miChangeBorderWidth;
  pScreen->MarkUnrealizedWindow = miMarkUnrealizedWindow;

  /* GC Handling Routines */
  /*
   * All other GC handling routines are pointed to through
   * pScreen->gcfuncs
   */
  /* See Porting Layer Definition pp. 43-46 */
  pScreen->CreateGC = winCreateGCNativeGDI;

  pScreen->RestackWindow = (RestackWindowProcPtr) 0;

  pScreen->SendGraphicsExpose = miSendGraphicsExpose;

  /* Block and Wakeup Handlers */
  pScreen->BlockHandler = (ScreenBlockHandlerProcPtr) NoopDDA;
  pScreen->WakeupHandler = (ScreenWakeupHandlerProcPtr) NoopDDA;
  pScreen->blockData = (pointer) 0;
  pScreen->wakeupData = (pointer) 0;

  fprintf (stderr, "winScreenInit () - calling miInitVisuals()\n");
  if (!winInitVisualsNativeGDI (pScreen))
    {
      ErrorF ("winScreenInit () - winInitVisuals returned FALSE\n");
      return FALSE;
    }
  else
    {
      ErrorF ("winScreenInit () - winInitVisuals returned TRUE\n");
    }

  /* Visuals */
  pScreen->numDepths = nDepths;
  pScreen->rootDepth = nRootDepth;
  pScreen->allowedDepths = pDepths;
  pScreen->rootVisual = rootVisual;
  pScreen->numVisuals = nVisuals;
  pScreen->visuals = pVisuals;

  ErrorF ("winScreenInit () - nDepths: %d, nRootDepth: %d, nVisuals: %d\n",
	  nDepths, nRootDepth, nVisuals);
  
  fprintf (stderr, "winScreenInit () - calling miSetZeroLineBias()\n");
  miSetZeroLineBias (pScreen, pScreenInfo->dwLineBias);

  miPointerSetNewScreen (pScreenInfo->dwScreen, 0, 0);

  ErrorF ("winScreenInit () - calling miDCInitialize()\n");
  if (!miDCInitialize (pScreen, &g_winPointerCursorFuncs))
    {
      ErrorF ("winScreenInit () - miDCInitialize failed\n");
      return FALSE;
    }
  else
    {
      ErrorF ("winScreenInit () - miDCInitialize succeeded\n");
    }

  ErrorF ("winScreenInit () - calling winCreateDefColormap()\n");
  fReturn = winCreateDefColormapNativeGDI (pScreen);

#ifdef RENDER
  ErrorF ("winScreenInit () - calling miPictureInit()\n");
  miPictureInit (pScreen, formats, nformats);
#endif
  
  if (fReturn)
    {
      RegisterBlockAndWakeupHandlers (winBlockHandler,
				      winWakeupHandler,
				      NULL);
    }
  pScreenInfo->pScreen = pScreen;

  miScreenDevPrivateInit (pScreen, xsize, pbits);

  ErrorF ("winScreenInit () - Successful addition of Screen %p %p\n",
	  pScreen->devPrivate,
	  pScreen);

  return fReturn;
}

PixmapPtr
winGetWindowPixmap (WindowPtr pwin)
{
  ErrorF ("winGetWindowPixmap ()\n");
  return NULL;
}

void
winSetWindowPixmap (WindowPtr pwin, PixmapPtr pPix)
{
  ErrorF ("winSetWindowPixmap ()\n");
}
