/*
 * Shutdown (/restart) bits
 */

#include "ctwm.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "animate.h"
#ifdef CAPTIVE
#include "captive.h"
#endif
#include "colormaps.h"
#include "ctwm_atoms.h"
#include "ctwm_shutdown.h"
#include "screen.h"
#ifdef SESSION
#include "session.h"
#endif
#ifdef SOUNDS
# include "sound.h"
#endif
#include "otp.h"
#include "win_ops.h"
#include "win_utils.h"


static void RestoreForShutdown(Time mytime);


/**
 * Put a window back where it should be if we don't (any longer) control
 * it and reparent it back up to the root.  This leaves it where it was
 * before we started (well, adjusted by any moves we've made to it
 * since), and placed so that if we restart and take it back over, it'll
 * wind up right where it is now, so restarting doesn't shift windows all
 * over the place.
 */
void
RestoreWinConfig(TwmWindow *tmp)
{
	int gravx, gravy;
	int newx, newy;

	// Things adjusting by the border have to move our border size, but
	// subtract away from that the old border we're restoring.
	const int borders = tmp->frame_bw + tmp->frame_bw3D - tmp->old_bw;

	// If this window is "unmapped" by moving it way offscreen, and is in
	// that state, move it back onto the window.
	if(tmp->UnmapByMovingFarAway && !visible(tmp)) {
		XMoveWindow(dpy, tmp->frame, tmp->frame_x, tmp->frame_y);
	}

	// If it's squeezed, unsqueeze it.
	if(tmp->squeezed) {
		Squeeze(tmp);
	}

	// This is apparently our standard "is this window still around?"
	// idiom.
	if(XGetGeometry(dpy, tmp->w, &JunkRoot, &JunkX, &JunkY,
	                &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0) {
		// Well, give up...
		return;
	}

	// Get gravity bits to know how to move stuff around when we take
	// away the decorations.
	GetGravityOffsets(tmp, &gravx, &gravy);

	// We want to move the window to where it should be "outside" of our
	// frame.  This varies depending on the window gravity detail, and we
	// have to account for that, since on re-startup we'll be doing it to
	// resposition it after we re-wrap it.
	//
	// e.g., in simple "NorthWest" gravity, we just made the frame start
	// where the window did, and shifted the app window right (by the
	// border width) and down (by the border width + titlebar).  However,
	// "SouthEast" gravity means the bottom right of the frame is where
	// the windows' was, and the window itself shifted left/up by the
	// border.  Compare e.g. an xterm with -geometry "+0+0" with one
	// using "-0-0" as an easy trick to make windows with different
	// geoms.
	newx = tmp->frame_x;
	newy = tmp->frame_y;


	// So, first consider the north/south gravity.  If gravity is North,
	// we put the top of the frame where the window was and shifted
	// everything inside down, so the top of the frame now is where the
	// window should be put.  With South-y gravity, the window should
	// wind up at the bottom of the frame, which means we need to shift
	// it down by the titlebar height, plus twice the border width.  But
	// if the vertical gravity is neutral, then the window needs to wind
	// up staying right where it is, because we built the frame around it
	// without moving it.
	//
	// Previous code here (and code elsewhere) expressed this calculation
	// by the rather confusing idiom ((gravy + 1) * border_width), which
	// gives the right answer, but is confusing as hell...
	if(gravy < 0) {
		// North; where the frame starts (already set)
	}
	else if(gravy > 0) {
		// South; shift down title + 2*border
		newy += tmp->title_height + 2 * borders;
	}
	else {
		// Neutral; down by the titlebar + border.
		newy += tmp->title_height + borders;
	}


	// Now east/west.  West means align with the frame start, east means
	// align with the frame right edge, neutral means where it already
	// is.
	if(gravx < 0) {
		// West; it's already correct
	}
	else if(gravx > 0) {
		// East; shift over by 2*border
		newx += 2 * borders;
	}
	else {
		// Neutral; over by the left border
		newx += borders;
	}


#ifdef WINBOX
	// If it's in a WindowBox, reparent the frame back up to our real root
	if(tmp->winbox && tmp->winbox->twmwin && tmp->frame) {
		int xbox, ybox;
		unsigned int j_bw;

		// XXX This isn't right, right?  This will give coords relative
		// to the window box, but we're using them relative to the real
		// screen root?
		if(XGetGeometry(dpy, tmp->frame, &JunkRoot, &xbox, &ybox,
		                &JunkWidth, &JunkHeight, &j_bw, &JunkDepth)) {
			ReparentWindow(dpy, tmp, WinWin, Scr->Root, xbox, ybox);
		}
	}
#endif


	// Restore the original window border if there were one
	if(tmp->old_bw) {
		XSetWindowBorderWidth(dpy, tmp->w, tmp->old_bw);
	}

	// Reparent and move back to where it otter be
	XReparentWindow(dpy, tmp->w, Scr->Root, newx, newy);

	// If it came with a pre-made icon window, hide it
	if(tmp->wmhints->flags & IconWindowHint) {
		XUnmapWindow(dpy, tmp->wmhints->icon_window);
	}

	// Done
	return;
}


/**
 * Restore some window positions/etc in preparation for going away.
 */
static void
RestoreForShutdown(Time mytime)
{
	ScreenInfo *savedScr = Scr;  // We need Scr flipped around...

	XGrabServer(dpy);

	for(int scrnum = 0; scrnum < NumScreens; scrnum++) {
		if((Scr = ScreenList[scrnum]) == NULL) {
			continue;
		}

		// Force reinstalling any colormaps
		InstallColormaps(0, &Scr->RootColormaps);

		// Pull all the windows out of their frames and reposition them
		// where the frame was, with approprate adjustments for gravity.
		// This will preserve their positions when we restart, or put
		// them back where they were before we started.  Do it from the
		// bottom up of our stacking order to preserve the stacking.
		for(TwmWindow *tw = OtpBottomWin() ; tw != NULL
		                ; tw = OtpNextWinUp(tw)) {
			if(tw->isiconmgr || tw->iswspmgr || tw->isoccupy) {
				// Don't bother with internals...
				continue;
			}
			RestoreWinConfig(tw);
		}
	}

	XUngrabServer(dpy);

	// Restore
	Scr = savedScr;

	// Focus away from any windows
	SetFocus(NULL, mytime);
}


/**
 * Cleanup and exit ctwm
 */
void
DoShutdown(void)
{

#ifdef SOUNDS
	// Denounce ourselves
	play_exit_sound();
#endif

	// Restore windows/colormaps for our absence.
	RestoreForShutdown(CurrentTime);

#ifdef EWMH
	// Clean up EWMH properties
	EwmhTerminate();
#endif

	// Clean up our list of workspaces
	XDeleteProperty(dpy, Scr->Root, XA_WM_WORKSPACESLIST);

#ifdef CAPTIVE
	// Shut down captive stuff
	if(CLarg.is_captive) {
		RemoveFromCaptiveList(Scr->captivename);
	}
#endif

	// Close up shop
	XCloseDisplay(dpy);
	exit(0);
}


/**
 * exec() ourself to restart.
 */
void
DoRestart(Time t)
{
	// Don't try to do any further animating
	StopAnimation();
	XSync(dpy, 0);

	// Replace all the windows/colormaps as if we were going away.  'cuz
	// we are.
	RestoreForShutdown(t);
	XSync(dpy, 0);

#ifdef SESSION
	// Shut down session management connection cleanly.
	shutdown_session();
#endif

	// Re-run ourself
	fprintf(stderr, "%s:  restarting:  %s\n", ProgramName, *Argv);
	execvp(*Argv, Argv);

	// Uh oh, we shouldn't get here...
	fprintf(stderr, "%s:  unable to restart:  %s\n", ProgramName, *Argv);

	// We should probably un-RestoreForShutdown() etc.  If that exec
	// fails, we're in a really weird state...
	XBell(dpy, 0);
}
