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
/* $XFree86: xc/programs/Xserver/hw/xwin/winwndproc.c,v 1.19 2001/11/21 08:51:24 alanh Exp $ */

#include "win.h"


/*
 * Called by winWakeupHandler
 * Processes current Windows message
 */

LRESULT CALLBACK
winWindowProc (HWND hwnd, UINT message, 
	       WPARAM wParam, LPARAM lParam)
{
  static winPrivScreenPtr	pScreenPriv = NULL;
  static winScreenInfo		*pScreenInfo = NULL;
  static ScreenPtr		pScreen = NULL;
  static HWND			hwndLastMouse = NULL;
  static unsigned long		ulServerGeneration = 0;
  winPrivScreenPtr		pScreenPrivLast;
  int				iScanCode;
  int				i;
  static HWND			hwndLastPrivates = NULL;

  /* Watch for server regeneration */
  if (g_ulServerGeneration != ulServerGeneration)
    {
      /*
       * Might as well declare that this window received 
       * the last mouse message
       */
      hwndLastMouse = hwnd;
      
      /* Store new server generation */
      ulServerGeneration = g_ulServerGeneration;
    }

  /* Only retrieve new privates pointers if window handle is null or changed */
  if ((pScreenPriv == NULL || hwnd != hwndLastPrivates)
      && (pScreenPriv = GetProp (hwnd, WIN_SCR_PROP)) != NULL)
    {
#if CYGDEGUG
      ErrorF ("winWindowProc () - Setting privates handle\n");
#endif
      pScreenInfo = pScreenPriv->pScreenInfo;
      pScreen = pScreenInfo->pScreen;
      hwndLastPrivates = hwnd;
    }
  else if (pScreenPriv == NULL)
    {
      /* For safety, handle case that should never happen */
      pScreenInfo = NULL;
      pScreen = NULL;
      hwndLastPrivates = NULL;
    }

  /* Branch on message type */
  switch (message)
    {
    case WM_CREATE:
#if CYGDEBUG
      ErrorF ("winWindowProc () - WM_CREATE\n");
#endif
      
      /*
       * Add a property to our display window that references
       * this screens' privates.
       *
       * This allows the window procedure to refer to the
       * appropriate window DC and shadow DC for the window that
       * it is processing.  We use this to repaint exposed
       * areas of our display window.
       */
      pScreenPriv = ((LPCREATESTRUCT) lParam)->lpCreateParams;
      pScreenInfo = pScreenPriv->pScreenInfo;
      pScreen = pScreenInfo->pScreen;
      hwndLastPrivates = hwnd;
      SetProp (hwnd, WIN_SCR_PROP, pScreenPriv);

      /* Store the mode key states so restore doesn't try to restore them */
      winStoreModeKeyStates (pScreen);
      return 0;

    case WM_PAINT:
#if CYGDEBUG
      ErrorF ("winWindowProc () - WM_PAINT\n");
#endif
      /* Only paint if we have privates and the server is enabled */
      if (pScreenPriv == NULL
	  || !pScreenPriv->fEnabled
	  || (pScreenInfo->fFullScreen && !pScreenPriv->fActive))
	{
	  /* We don't want to paint */
	  break;
	}

      /* Break out here if we don't have a valid paint routine */
      if (pScreenPriv->pwinBltExposedRegions == NULL)
	break;
      
      /* Call the engine dependent repainter */
      (*pScreenPriv->pwinBltExposedRegions) (pScreen);
      return 0;

    case WM_PALETTECHANGED:
      {
#if CYGDEBUG
	ErrorF ("winWindowProc () WM_PALETTECHANGED\n");
#endif
	/* Don't process if we don't have privates or a colormap */
	if (pScreenPriv == NULL || pScreenPriv->pcmapInstalled == NULL)
	  break;

	/* Return if we caused the palette to change */
	if ((HWND) wParam == hwnd)
	  {
	    /* Redraw the screen */
	    (*pScreenPriv->pwinRedrawScreen) (pScreen);
	    return 0;
	  }
	
	/* Reinstall the windows palette */
	(*pScreenPriv->pwinRealizeInstalledPalette) (pScreen);
	
	/* Redraw the screen */
	(*pScreenPriv->pwinRedrawScreen) (pScreen);
	return 0;
      }

    case WM_MOUSEMOVE:
      /* We can't do anything without privates */
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      
      /* Has the mouse pointer crossed screens? */
      if (pScreen != miPointerCurrentScreen ())
	miPointerSetNewScreen (pScreenInfo->dwScreen, 0, 0);

      /* Sometimes we hide, sometimes we show */
      if (hwndLastMouse != NULL && hwndLastMouse != hwnd)
	{
	  /* Cursor is now over NC area of another screen */
	  pScreenPrivLast = GetProp (hwndLastMouse, WIN_SCR_PROP);
	  if (pScreenPrivLast == NULL)
	    {
	      ErrorF ("winWindowProc () - WM_MOUSEMOVE - Couldn't obtain "
		      "last screen privates\n");
	      return 0;
	    }

	  /* Show cursor if last screen is still hiding it */
	  if (!pScreenPrivLast->fCursor)
	    {
	      pScreenPrivLast->fCursor = TRUE;
	      ShowCursor (TRUE);
	    }

	  /* Hide cursor for our screen if we are not hiding it */
	  if (pScreenPriv->fCursor)
	    {
	      pScreenPriv->fCursor = FALSE;
	      ShowCursor (FALSE);
	    }
	}
      else if (pScreenPriv->fActive
	  && pScreenPriv->fCursor)
	{
	  /* Hide Windows cursor */
	  pScreenPriv->fCursor = FALSE;
	  ShowCursor (FALSE);
	}
      else if (!pScreenPriv->fActive
	       && !pScreenPriv->fCursor)
	{
	  /* Show Windows cursor */
	  pScreenPriv->fCursor = TRUE;
	  ShowCursor (TRUE);
	}

      /* Deliver absolute cursor position to X Server */
      miPointerAbsoluteCursor (GET_X_LPARAM(lParam),
			       GET_Y_LPARAM(lParam),
			       g_c32LastInputEventTime = GetTickCount ());

      /* Store pointer to last window handle */
      hwndLastMouse = hwnd;
      return 0;

    case WM_NCMOUSEMOVE:
      /* We can't do anything without privates */
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

      /* Non-client mouse movement, show Windows cursor */
      if (hwndLastMouse != NULL && hwndLastMouse != hwnd)
	{
	  /* Cursor is now over NC area of another screen */
	  pScreenPrivLast = GetProp (hwndLastMouse, WIN_SCR_PROP);
	  if (pScreenPrivLast == NULL)
	    {
	      ErrorF ("winWindowProc () - WM_NCMOUSEMOVE - Couldn't obtain "
		      "last screen privates\n");
	      return 0;
	    }

	  /* Show cursor if last screen is still hiding it */
	  if (!pScreenPrivLast->fCursor)
	    {
	      pScreenPrivLast->fCursor = TRUE;
	      ShowCursor (TRUE);
	    }

	  /* Hide cursor for our screen if we are not hiding it */
	  if (pScreenPriv->fCursor)
	    {
	      pScreenPriv->fCursor = FALSE;
	      ShowCursor (FALSE);
	    }
	}
      else if (!pScreenPriv->fCursor)
	{
	  pScreenPriv->fCursor = TRUE;
	  ShowCursor (TRUE);
	}

      /* Store pointer to last window handle */
      hwndLastMouse = hwnd;
      return 0;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (pScreen, ButtonPress, Button1, wParam);
      
    case WM_LBUTTONUP:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (pScreen, ButtonRelease, Button1, wParam);

    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (pScreen, ButtonPress, Button2, wParam);
      
    case WM_MBUTTONUP:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (pScreen, ButtonRelease, Button2, wParam);
      
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (pScreen, ButtonPress, Button3, wParam);
      
    case WM_RBUTTONUP:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (pScreen, ButtonRelease, Button3, wParam);

    case WM_TIMER:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

      /* Branch on the timer id */
      switch (wParam)
	{
	case WIN_E3B_TIMER_ID:
	  /* Send delayed button press */
	  winMouseButtonsSendEvent (ButtonPress,
				    pScreenPriv->iE3BCachedPress);

	  /* Kill this timer */
	  KillTimer (pScreenPriv->hwndScreen, WIN_E3B_TIMER_ID);

	  /* Clear screen privates flags */
	  pScreenPriv->iE3BCachedPress = 0;
	  break;
	}
      return 0;

    case WM_MOUSEWHEEL:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;
      return winMouseWheel (pScreen, GET_WHEEL_DELTA_WPARAM(wParam));

    case WM_SETFOCUS:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

      /* Restore the state of all mode keys */
      winRestoreModeKeyStates (pScreen);
      return 0;

    case WM_KILLFOCUS:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

      /* Store the state of all mode keys */
      winStoreModeKeyStates (pScreen);

      /* Release any pressed modifiers */
      winKeybdReleaseModifierKeys ();
      return 0;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

      /*
       * FIXME: Catching Alt-F4 like this is really terrible.  This should
       * be generalized to handle other Windows keyboard signals.  Actually,
       * the list keys to catch and the actions to perform when caught should
       * be configurable; that way user's can customize the keys that they
       * need to have passed through to their window manager or apps, or they
       * can remap certain actions to new key codes that do not conflict
       * with the X apps that they are using.  Yeah, that'll take awhile.
       */
      if ((pScreenInfo->fUseWinKillKey && wParam == VK_F4
	   && (GetKeyState (VK_MENU) & 0x8000))
	  || (pScreenInfo->fUseUnixKillKey && wParam == VK_BACK
	      && (GetKeyState (VK_MENU) & 0x8000)
	      && (GetKeyState (VK_CONTROL) & 0x8000))) 
	{
	  /*
	   * Better leave this message here, just in case some unsuspecting
	   * user enters Alt + F4 and is surprised when the application
	   * quits.
	   */
	  ErrorF ("winWindowProc () - Closekey hit, quitting\n");
	  
	  /* Tell our message queue to give up */
	  PostMessage (hwnd, WM_CLOSE, 0, 0);
	  return 0;
	}
      
      /*
       * Don't do anything for the Windows keys, as focus will soon
       * be returned to Windows.  We may be able to trap the Windows keys,
       * but we should determine if that is desirable before doing so.
       */
      if (wParam == VK_LWIN || wParam == VK_RWIN)
	break;

      /* Discard fake Ctrl_L presses that precede AltGR on non-US keyboards */
      if (winIsFakeCtrl_L (message, wParam, lParam))
	return 0;
      
      /* Send the key event(s) */
      winTranslateKey (wParam, lParam, &iScanCode);
      for (i = 0; i < LOWORD(lParam); ++i)
	winSendKeyEvent (iScanCode, TRUE);
      return 0;

    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

      /*
       * Don't do anything for the Windows keys, as focus will soon
       * be returned to Windows.  We may be able to trap the Windows keys,
       * but we should determine if that is desirable before doing so.
       */
      if (wParam == VK_LWIN || wParam == VK_RWIN)
	break;

      /* Ignore the fake Ctrl_L that follows an AltGr release */
      if (winIsFakeCtrl_L (message, wParam, lParam))
	return 0;

      /* Enqueue a keyup event */
      winTranslateKey (wParam, lParam, &iScanCode);
      winSendKeyEvent (iScanCode, FALSE);
      return 0;

    case WM_HOTKEY:
      if (pScreenPriv == NULL)
	break;

      /* Call the engine-specific hot key handler */
      (*pScreenPriv->pwinHotKeyAltTab) (pScreen);
      return 0;

    case WM_ACTIVATE:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

#if CYGDEBUG
      ErrorF ("winWindowProc () - WM_ACTIVATE\n");
#endif
      /*
       * Focus is being changed to another window.
       * The other window may or may not belong to
       * our process.
       */

      /* Clear any lingering wheel delta */
      pScreenPriv->iDeltaZ = 0;

      /* Have we changed X screens? */
      if ((LOWORD (wParam) == WA_ACTIVE || LOWORD (wParam) == WA_CLICKACTIVE)
	  && pScreenPriv->fEnabled && pScreen != miPointerCurrentScreen ())
	miPointerSetNewScreen (pScreenInfo->dwScreen, 0, 0);

      /* Handle showing or hiding the mouse */
      if (hwndLastMouse != NULL && hwndLastMouse != hwnd)
	{
	  /*
	   * Activation has transferred between screens.
	   * This section is processed by the screen receiving
	   * focus, as it is the only one that notices the difference
	   * between hwndLastMouse and hwnd.
	   */
	  pScreenPrivLast = GetProp (hwndLastMouse, WIN_SCR_PROP);
	  if (pScreenPrivLast == NULL)
	    {
	      ErrorF ("winWindowProc () - WM_ACTIVATE - Couldn't obtain last "
		      "screen privates\n");
	      return 0;
	    }

	  /* Show cursor if last screen is still hiding it */
	  if (!pScreenPrivLast->fCursor)
	    {
	      pScreenPrivLast->fCursor = TRUE;
	      ShowCursor (TRUE);
	    }

	  /* Hide cursor for our screen if we are not hiding it */
	  if (pScreenPriv->fCursor)
	    {
	      pScreenPriv->fCursor = FALSE;
	      ShowCursor (FALSE);
	    }
	}
      else if ((LOWORD(wParam) == WA_ACTIVE
		|| LOWORD(wParam) == WA_CLICKACTIVE)
	       && pScreenPriv->fCursor)
	{
	  pScreenPriv->fCursor = FALSE;
	  ShowCursor (FALSE);
	}
      else if (LOWORD(wParam) == WA_INACTIVE
	       && !pScreenPriv->fCursor)
	{
	  pScreenPriv->fCursor = TRUE;
	  ShowCursor (TRUE);
	}

      /* Store last active window handle */
      hwndLastMouse = hwnd;
      return 0;

    case WM_ACTIVATEAPP:
      if (pScreenPriv == NULL || pScreenInfo->fIgnoreInput)
	break;

#if CYGDEBUG
      ErrorF ("winWindowProc () - WM_ACTIVATEAPP\n");
#endif
      /* Activate or deactivate */
      pScreenPriv->fActive = wParam;

      /* Are we activating or deactivating? */
      if (pScreenPriv->fActive
	  && pScreenPriv->fCursor)
	{
	  pScreenPriv->fCursor = FALSE;
	  ShowCursor (FALSE);
	}
      else if (!pScreenPriv->fActive
	       && !pScreenPriv->fCursor)
	{
	  pScreenPriv->fCursor = TRUE;
	  ShowCursor (TRUE);
	}

      /* Call engine specific screen activation/deactivation function */
      (*pScreenPriv->pwinActivateApp) (pScreen);
      return 0;

    case WM_CLOSE:
      /* Tell X that we are giving up */
      GiveUp (0);
      return 0;
    }

  return DefWindowProc (hwnd, message, wParam, lParam);
}
