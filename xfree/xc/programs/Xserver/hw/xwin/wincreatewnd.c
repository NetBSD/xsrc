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
/* $XFree86: xc/programs/Xserver/hw/xwin/wincreatewnd.c,v 1.1 2001/11/11 23:07:40 alanh Exp $ */

#include "win.h"
#include "shellapi.h"


/*
 * Local function prototypes
 */

static Bool
winAdjustForAutoHide (RECT *prcWorkArea);


/*
 * Create a full screen window
 */

Bool
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

  return TRUE;
}


/*
 * Create our primary Windows display window
 */

Bool
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

  /* Adjust for auto-hide taskbars */
  winAdjustForAutoHide (&rcWorkArea);

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
      iHeight = rcWorkArea.bottom - rcWorkArea.top;
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
			    | WS_MINIMIZEBOX,	/* Almost OverlappedWindow */
			    rcWorkArea.left,	/* Horizontal position */
			    rcWorkArea.top,	/* Vertical position */
			    iWidth,		/* Right edge */
			    iHeight,		/* Bottom edge */
			    (HWND) NULL,	/* No parent or owner window */
			    (HMENU) NULL,	/* No menu */
			    GetModuleHandle (NULL),/* Instance handle */
			    pScreenPriv);	/* ScreenPrivates */
  if (*phwnd == NULL)
    {
      ErrorF ("winCreateBoundingWindowWindowed () CreateWindowEx () failed\n");
      return FALSE;
    }

#if CYGDEBUG
  ErrorF ("winCreateBoundingWindowWindowed () - CreateWindowEx () returned\n");
#endif

  /* Get the client area coordinates */
  if (!GetClientRect (*phwnd, &rcClient))
    {
      ErrorF ("winCreateBoundingWindowWindowed () - GetClientRect () "
	      "failed\n");
      return FALSE;
    }
  ErrorF ("winCreateBoundingWindowWindowed () - WindowClient "\
	  "w %d h %d r %d l %d b %d t %d\n",
	  rcClient.right - rcClient.left,
	  rcClient.bottom - rcClient.top,
	  rcClient.right, rcClient.left,
	  rcClient.bottom, rcClient.top);

  /*
   * Transform the client relative coords to screen relative coords.
   * It is almost impossible to tell if the function has failed, thus
   * we do not want to check for a return value of 0, as that could
   * simply indicated that the window was positioned with the upper
   * left corner at (0,0).
   */
  MapWindowPoints (*phwnd,
		   HWND_DESKTOP,
		   (LPPOINT)&rcClient,
		   2);

  /* Show the window */
  ShowWindow (*phwnd, SW_SHOW);
  if (!UpdateWindow (*phwnd))
    {
      ErrorF ("winCreateBoundingWindowWindowed () - UpdateWindow () failed\n");
      return FALSE;
    }
  
  /* Attempt to bring our window to the top of the display */
  if (!BringWindowToTop (*phwnd))
    {
      ErrorF ("winCreateBoundingWindowWindowed () - BringWindowToTop () "
	      "failed\n");
      return FALSE;
    }

  /* Paint window background blue */
  if (pScreenInfo->dwEngine == WIN_SERVER_NATIVE_GDI)
    winPaintBackground (*phwnd, RGB (0x00, 0x00, 0xFF));

  ErrorF ("winCreateBoundingWindowWindowed () -  Returning\n");

  return TRUE;
}


/*
 * Adjust the client area so that any auto-hide toolbars
 * will work correctly.
 */

static Bool
winAdjustForAutoHide (RECT *prcWorkArea)
{
  APPBARDATA		abd;
  HWND			hwndAutoHide;

  ErrorF ("winAdjustForAutoHide - Original WorkArea: %d %d %d %d\n",
	  prcWorkArea->top, prcWorkArea->left,
	  prcWorkArea->bottom, prcWorkArea->right);

  /* Find out if the Windows taskbar is set to auto-hide */
  ZeroMemory (&abd, sizeof (abd));
  abd.cbSize = sizeof (abd);
  if (SHAppBarMessage (ABM_GETSTATE, &abd) & ABS_AUTOHIDE)
    ErrorF ("winAdjustForAutoHide - Taskbar is auto hide\n");

  /* Look for a TOP auto-hide taskbar */
  abd.uEdge = ABE_TOP;
  hwndAutoHide = (HWND) SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd);
  if (hwndAutoHide != NULL)
    {
      ErrorF ("winAdjustForAutoHide - Found TOP auto-hide taskbar\n");
      prcWorkArea->top += 1;
    }

  /* Look for a LEFT auto-hide taskbar */
  abd.uEdge = ABE_LEFT;
  hwndAutoHide = (HWND) SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd);
  if (hwndAutoHide != NULL)
    {
      ErrorF ("winAdjustForAutoHide - Found LEFT auto-hide taskbar\n");
      prcWorkArea->left += 1;
    }

  /* Look for a BOTTOM auto-hide taskbar */
  abd.uEdge = ABE_BOTTOM;
  hwndAutoHide = (HWND) SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd);
  if (hwndAutoHide != NULL)
    {
      ErrorF ("winAdjustForAutoHide - Found BOTTOM auto-hide taskbar\n");
      prcWorkArea->bottom -= 1;
    }

  /* Look for a RIGHT auto-hide taskbar */
  abd.uEdge = ABE_RIGHT;
  hwndAutoHide = (HWND) SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd);
  if (hwndAutoHide != NULL)
    {
      ErrorF ("winAdjustForAutoHide - Found RIGHT auto-hide taskbar\n");
      prcWorkArea->right -= 1;
    }

  ErrorF ("winAdjustForAutoHide - Adjusted WorkArea: %d %d %d %d\n",
	  prcWorkArea->top, prcWorkArea->left,
	  prcWorkArea->bottom, prcWorkArea->right);
  
#if 0
  /* Obtain the task bar window dimensions */
  abd.hWnd = hwndAutoHide;
  hwndAutoHide = (HWND) SHAppBarMessage (ABM_GETTASKBARPOS, &abd);
  ErrorF ("hwndAutoHide %08x abd.hWnd %08x %d %d %d %d\n",
	  hwndAutoHide, abd.hWnd,
	  abd.rc.top, abd.rc.left, abd.rc.bottom, abd.rc.right);
#endif

  return TRUE;
}
