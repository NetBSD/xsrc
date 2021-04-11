/*
 * Functions related to captive-mode ctwm.
 */

#include "ctwm.h"

#include <stdlib.h>

#include "captive.h"
#include "cursor.h"
#include "events.h"
#include "functions_internal.h"
#include "screen.h"


/*
 * Pulling a window into the ctwm it was invoked from.
 */
DFHANDLER(adoptwindow)
{
	AdoptWindow();
}


/*
 * Interactively moving a window between ctwm's.
 */
DFHANDLER(hypermove)
{
	bool    cont = true;
	Window  root = RootWindow(dpy, Scr->screen);
	Cursor  cursor;
	Window captive_root;

	if(tmp_win->iswinbox || tmp_win->iswspmgr) {
		XBell(dpy, 0);
		return;
	}

	{
		CaptiveCTWM cctwm = GetCaptiveCTWMUnderPointer();
		cursor = MakeStringCursor(cctwm.name);
		free(cctwm.name);
		captive_root = cctwm.root;
	}

	XGrabPointer(dpy, root, True,
	             ButtonPressMask | ButtonMotionMask | ButtonReleaseMask,
	             GrabModeAsync, GrabModeAsync, root, cursor, CurrentTime);
	while(cont) {
		XMaskEvent(dpy, ButtonPressMask | ButtonMotionMask |
		           ButtonReleaseMask, &Event);
		switch(Event.xany.type) {
			case ButtonPress:
				cont = false;
				break;

			case ButtonRelease: {
				CaptiveCTWM cctwm = GetCaptiveCTWMUnderPointer();
				cont = false;
				free(cctwm.name);
				if(cctwm.root == Scr->Root) {
					break;
				}
				if(cctwm.root == Scr->XineramaRoot) {
					break;
				}
				SetNoRedirect(tmp_win->w);
				XUngrabButton(dpy, AnyButton, AnyModifier, tmp_win->w);
				XReparentWindow(dpy, tmp_win->w, cctwm.root, 0, 0);
				XMapWindow(dpy, tmp_win->w);
				break;
			}

			case MotionNotify: {
				CaptiveCTWM cctwm = GetCaptiveCTWMUnderPointer();
				if(cctwm.root != captive_root) {
					unsigned int chmask;

					XFreeCursor(dpy, cursor);
					cursor = MakeStringCursor(cctwm.name);
					captive_root = cctwm.root;

					chmask = (ButtonPressMask | ButtonMotionMask
					          | ButtonReleaseMask);
					XChangeActivePointerGrab(dpy, chmask,
					                         cursor, CurrentTime);
				}
				free(cctwm.name);
				break;
			}
		}
	}

	ButtonPressed = -1;
	XUngrabPointer(dpy, CurrentTime);
	XFreeCursor(dpy, cursor);

	return;
}
