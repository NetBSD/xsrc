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
/* $XFree86: xc/programs/Xserver/hw/xwin/winwndproc.c,v 1.4 2001/05/08 08:14:09 alanh Exp $ */

#include "win.h"

/*
 * Called by winWakeupHandler
 * Processes current Windows message
 */
LRESULT CALLBACK
winWindowProc (HWND hWnd, UINT message, 
	       WPARAM wParam, LPARAM lParam)
{
  winPrivScreenPtr      pScreenPriv = NULL;
  winScreenInfo		*pScreenInfo = NULL;
  ScreenPtr		pScreen = NULL;
  xEvent		xCurrentEvent;
  LPCREATESTRUCT	pcs;
  HRESULT		ddrval;
  RECT			rcClient, rcSrc;
  int			iScanCode;

  /* Initialize our event structure */
  ZeroMemory (&xCurrentEvent, sizeof (xCurrentEvent));

  /* Retrieve screen privates pointers for this window */
  pScreenPriv = GetProp (hWnd, WIN_SCR_PROP);
  if (pScreenPriv != NULL)
    {
      pScreenInfo = pScreenPriv->pScreenInfo;
      pScreen = pScreenInfo->pScreen;
    }

  /* Branch on message type */
  switch (message)
    {
    case WM_CREATE:
      /*
       * Add a property to our display window that references
       * this screens' privates.
       *
       * This allows the window procedure to refer to the
       * appropriate window DC and shadow DC for the window that
       * it is processing.  We use this to repaint exposed
       * areas of our display window.
       */
      pcs = (LPCREATESTRUCT) lParam;
      pScreenPriv = pcs->lpCreateParams;
      pScreen = pScreenPriv->pScreenInfo->pScreen;
      SetProp (hWnd,
	       WIN_SCR_PROP,
	       pScreenPriv);

      /* Store the mode key states so restore doesn't try to restore them */
      winStoreModeKeyStates (pScreen);
      return 0;

    case WM_PAINT:
      /* Only paint if we have privates and the server is enabled */
      if (pScreenPriv == NULL
	  || !pScreenPriv->fEnabled
	  || (pScreenInfo->fFullScreen && !pScreenPriv->fActive))
	{
	  /* We don't want to paint */
	  break;
	}
      
      /* Call the engine dependent repainter */
      (*pScreenPriv->pwinBltExposedRegions) (pScreen);
      return 0;

    case WM_MOUSEMOVE:
      /* We can't do anything without privates */
      if (pScreenPriv == NULL)
	break;

      /* Sometimes we hide, sometimes we show */
      if (pScreenPriv->fActive
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
      return 0;

    case WM_NCMOUSEMOVE:
      /* Non-client mouse movement, show Windows cursor */
      if (!pScreenPriv->fCursor)
	{
	  pScreenPriv->fCursor = TRUE;
	  ShowCursor (TRUE);
	}
      return 0;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
      return winMouseButtonsHandle (pScreen, ButtonPress, Button1, wParam);
      
    case WM_LBUTTONUP:
      return winMouseButtonsHandle (pScreen, ButtonRelease, Button1, wParam);

    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
      return winMouseButtonsHandle (pScreen, ButtonPress, Button2, wParam);
      
    case WM_MBUTTONUP:
      return winMouseButtonsHandle (pScreen, ButtonRelease, Button2, wParam);
      
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
      return winMouseButtonsHandle (pScreen, ButtonPress, Button3, wParam);
      
    case WM_RBUTTONUP:
      return winMouseButtonsHandle (pScreen, ButtonRelease, Button3, wParam);

    case WM_TIMER:
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
      return winMouseWheel (pScreen, GET_WHEEL_DELTA_WPARAM(wParam));

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (winIsFakeCtrl_L (message, wParam, lParam))
	return 0;
      winTranslateKey (wParam, lParam, &iScanCode);
      xCurrentEvent.u.u.type = KeyPress;
      xCurrentEvent.u.u.detail = iScanCode;
      xCurrentEvent.u.keyButtonPointer.time
	= g_c32LastInputEventTime = GetTickCount ();
      mieqEnqueue (&xCurrentEvent);
      return 0;

    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (winIsFakeCtrl_L (message, wParam, lParam))
	return 0;
      winTranslateKey (wParam, lParam, &iScanCode);
      xCurrentEvent.u.u.type = KeyRelease;
      xCurrentEvent.u.u.detail = iScanCode;
      xCurrentEvent.u.keyButtonPointer.time
	= g_c32LastInputEventTime = GetTickCount ();
      mieqEnqueue (&xCurrentEvent);
      return 0;

    case WM_HOTKEY:
      if (pScreenPriv == NULL)
	break;

      /* Handle each engine type */
      switch (pScreenInfo->dwEngine)
	{
	case WIN_SERVER_PRIMARY_DD:
	  /* Alt+Tab was pressed, we will lose focus very soon */
	  pScreenPriv->fActive = FALSE;

	  /*
	   * We need to save the primary fb to an offscreen fb when
	   * we get deactivated, and point the fb code at the offscreen
	   * fb for the duration of the deactivation.
	   */
	  if (pScreenPriv != NULL
	      && pScreenPriv->pddsPrimary != NULL
	      && pScreenPriv->pddsPrimary != NULL)
	    {
	      /* We are deactivating */
		      
	      /* Get client area in screen coords */
	      GetClientRect (pScreenPriv->hwndScreen, &rcClient);
	      MapWindowPoints (pScreenPriv->hwndScreen,
			       HWND_DESKTOP,
			       (LPPOINT)&rcClient, 2);

	      /* Did we loose the primary surface? */
	      ddrval = IDirectDrawSurface_IsLost (pScreenPriv->pddsPrimary);
	      if (ddrval == DD_OK)
		{
		  ddrval = IDirectDrawSurface_Unlock (pScreenPriv->pddsPrimary,
						      NULL);
		  if (FAILED (ddrval))
		    FatalError ("winWindowProc () - Failed unlocking primary "\
				"surface\n");
		}
	      
	      /* Setup a source rectangle */
	      rcSrc.left = 0;
	      rcSrc.top = 0;
	      rcSrc.right = pScreenInfo->dwWidth;
	      rcSrc.bottom = pScreenInfo->dwHeight;

	      /* Blit the primary surface to the offscreen surface */
	      ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsOffscreen,
					       NULL, /* should be rcDest */
					       pScreenPriv->pddsPrimary,
					       NULL,
					       DDBLT_WAIT,
					       NULL);
	      if (ddrval == DDERR_SURFACELOST)
		{
		  IDirectDrawSurface_Restore (pScreenPriv->pddsOffscreen);  
		  IDirectDrawSurface_Restore (pScreenPriv->pddsPrimary);
		  		  
		  /* Blit the primary surface to the offscreen surface */
		  ddrval = IDirectDrawSurface_Blt (pScreenPriv->pddsOffscreen,
						   NULL, /* should be rcDest */
						   pScreenPriv->pddsPrimary,
						   NULL,
						   DDBLT_WAIT,
						   NULL);
		  if (FAILED (ddrval))
		    FatalError ("winWindowProc () - Failed blitting primary "\
				"surface to offscreen surface: %08x\n",
				ddrval);
		}
	      else
		{
		  FatalError ("winWindowProc() - Unknown error from "\
			      "Blt: %08dx\n", ddrval);
		}

	      /* Lock the offscreen surface */
	      ddrval = IDirectDrawSurface_Lock (pScreenPriv->pddsOffscreen,
						NULL,
						pScreenPriv->pddsdOffscreen,
						DDLOCK_WAIT,
						NULL);
	      if (ddrval != DD_OK
		  || pScreenPriv->pddsdPrimary->lpSurface == NULL)
		FatalError ("winWindowProc () - Could not lock "\
			    "offscreen surface\n");

	      /* Notify FB of the new memory pointer */
	      winUpdateFBPointer (pScreen,
				  pScreenPriv->pddsdOffscreen->lpSurface);

	      /* Unregister our hotkey */
	      UnregisterHotKey (hWnd, 1);
	      return 0;
	    }
	}
      break;

    case WM_ACTIVATE:
      /*
       * Focus is being changed to another window.
       * The other window may or may not belong to
       * our process.
       */
      
      /* We can't do anything if we don't have screen privates */
      if (pScreenPriv == NULL)
	break;

      /* Clear any lingering wheel delta */
      pScreenPriv->iDeltaZ = 0;

      /* Activating or deactivating? */
      if (LOWORD (wParam) == WA_ACTIVE || LOWORD (wParam) == WA_CLICKACTIVE)
	{
	  /* Restore the state of all mode keys */
	  winRestoreModeKeyStates (pScreen);

	  /* Have we changed input screens? */
	  if (pScreenPriv->fEnabled
	      && pScreen != miPointerCurrentScreen ())
	    {
	      /*
	       * Tell mi that we are changing the screen that receives
	       * mouse input events.
	       */
	      miPointerSetNewScreen (pScreenInfo->dwScreen,
				     0, 0);
	    }
	}
      else
	{
	  /* Store the state of all mode keys */
	  winStoreModeKeyStates (pScreen);
	}

      /* Are we activating or deactivating? */
      if ((LOWORD(wParam) == WA_ACTIVE
	  || LOWORD(wParam) == WA_CLICKACTIVE)
	  && pScreenPriv->fCursor)
	{
	  /* Hide Windows cursor */
	  pScreenPriv->fCursor = FALSE;
	  ShowCursor (FALSE);
	}
      else if (LOWORD(wParam) == WA_INACTIVE
	       && !pScreenPriv->fCursor)
	{
	  /* Show Windows cursor */
	  pScreenPriv->fCursor = TRUE;
	  ShowCursor (TRUE);
	}
      return 0;

    case WM_ACTIVATEAPP:
      /* We can't do anything if we don't have screen privates */
      if (pScreenPriv == NULL)
	break;

      /* Activate or deactivate */
      pScreenPriv->fActive = wParam;

      /* Are we activating or deactivating? */
      if (pScreenPriv->fActive
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

      /* Call engine specific screen activation/deactivation function */
      (*pScreenPriv->pwinActivateApp) (pScreen);
      return 0;

    case WM_CLOSE:
      /* Tell X that we are giving up */
      GiveUp (0);
      return 0;
    }

  return DefWindowProc (hWnd, message, wParam, lParam);
}
