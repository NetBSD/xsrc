/*
 * Taking over the screen
 */

#include "ctwm.h"

#include <stdio.h>
#include <X11/Xproto.h>
#include <X11/Xmu/Error.h>

#include "ctwm_takeover.h"
#include "screen.h"


/// Flag for "we got an error trying to take over".  Set in temporary
/// error handler.
static bool RedirectError;

// Our special during-takeover and normal operating error handlers.
static int CatchRedirectError(Display *display, XErrorEvent *event);
static int TwmErrorHandler(Display *display, XErrorEvent *event);



/**
 * Take over as WM for a screen
 */
bool
takeover_screen(ScreenInfo *scr)
{
	unsigned long attrmask;

#ifdef EWMH
	// Early EWMH setup.  This tries to do the EWMH display takeover.
	EwmhInitScreenEarly(scr);
#endif


	/*
	 * Subscribe to various events on the root window.  Because X
	 * only allows a single client to subscribe to
	 * SubstructureRedirect and ButtonPress bits, this also serves to
	 * mutex who is The WM for the root window, and thus (aside from
	 * captive) the Screen.
	 *
	 * To catch whether that failed, we set a special one-shot error
	 * handler to flip a var that we test to find out whether the
	 * redirect failed.
	 */
	// Flush out any previous errors
	XSync(dpy, 0);

	// Set our event listening mask
	RedirectError = false;
	XSetErrorHandler(CatchRedirectError);
	attrmask = ColormapChangeMask | EnterWindowMask |
	           PropertyChangeMask | SubstructureRedirectMask |
	           KeyPressMask | ButtonPressMask | ButtonReleaseMask;
#ifdef EWMH
	attrmask |= StructureNotifyMask;
#endif
#ifdef CAPTIVE
	if(CLarg.is_captive) {
		attrmask |= StructureNotifyMask;
	}
#endif
	XSelectInput(dpy, scr->Root, attrmask);

	// Make sure we flush out any errors that may have caused.  This
	// ensures our RedirectError flag will be set if the server sent us
	// one.
	XSync(dpy, 0);

	// Go ahead and set our normal-operation error handler.
	XSetErrorHandler(TwmErrorHandler);

	// So, did we fail?
	if(RedirectError) {
		fprintf(stderr, "%s: another window manager is already running",
		        ProgramName);
		if(CLarg.MultiScreen && NumScreens > 0) {
			fprintf(stderr, " on screen %d?\n", scr->screen);
		}
		else {
			fprintf(stderr, "?\n");
		}

		// XSetErrorHandler() isn't local to the Screen; it's for the
		// whole connection.  We wind up in a slightly weird state
		// once we've set it up, but decided we aren't taking over
		// this screen, but resetting it would be a little weird too,
		// because maybe we have taken over some other screen.  So,
		// just throw up our hands.
		return false;
	}

	// Nope, it's ours!
	return true;
}



/**
 * Temporary error handler used during startup.  We expect an
 * error if we fail to take over some of the XSelectInput() events
 * we're trying to (which only 1 thing at a time is allowed to).
 * Probably that would be a BadAccess error type?  But really, any error
 * means we're in trouble and should skip over the display, so we don't
 * check any more deeply...
 */
static int
CatchRedirectError(Display *display, XErrorEvent *event)
{
	// Set the global flag
	RedirectError = true;
	return 0;
}


/**
 * Error handler used in normal operation.  Or, perhaps, error ignorer
 * used in normal operation.  If run with `-v`, we'll print out a lot of
 * the errors we might get, though we always skip several.
 */
static int
TwmErrorHandler(Display *display, XErrorEvent *event)
{
	if(!CLarg.PrintErrorMessages) {
		// Not `-v`?  Always be silent.
		return 0;
	}

	// If a client dies and we try to touch it before we notice, we get a
	// BadWindow error for most operations, except a few (GetGeometry
	// being the big one?) where we'll get a BadDrawable.  This isn't
	// really an "error", just a harmless race.
	if(event->error_code == BadWindow
	                || (event->request_code == X_GetGeometry && event->error_code != BadDrawable)) {
		return 0;
	}

	// Else, we're `-v`'d, and it wasn't one of our 'expected' bits, so
	// talk about it.
	XmuPrintDefaultErrorMessage(display, event, stderr);
	return 0;
}
