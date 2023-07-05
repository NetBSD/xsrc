/*
 * Functions related to moving/resizing windows.
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include "colormaps.h"
#include "events.h"
#include "event_handlers.h"
#include "functions.h"
#include "functions_defs.h"
#include "functions_internal.h"
#include "icons.h"
#include "otp.h"
#include "parse.h"
#include "r_area.h"
#include "r_layout.h"
#include "screen.h"
#include "util.h"
#include "vscreen.h"
#include "win_decorations.h"
#include "win_ops.h"
#include "win_resize.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "xparsegeometry.h"


/*
 * MoveFillDir-ectional specifiers, used in jump/pack/fill
 */
typedef enum {
	MFD_BOTTOM,
	MFD_LEFT,
	MFD_RIGHT,
	MFD_TOP,
} MoveFillDir;
static int FindConstraint(TwmWindow *tmp_win, MoveFillDir direction);

/* Internal util */
static bool belongs_to_twm_window(TwmWindow *t, Window w);


/*
 * Constrained move variables
 *
 * Used in the resize handling, but needed over in event code for
 * ButtonRelease as well.
 */
bool ConstMove = false;
CMoveDir ConstMoveDir;
int ConstMoveX;
int ConstMoveY;
int ConstMoveXL;
int ConstMoveXR;
int ConstMoveYT;
int ConstMoveYB;

/*
 * Which move-ish function is in progress.  This is _almost_ really a
 * local var in the movewindow() function, but we also reference it in
 * the HandleButtonRelease() event handler because that has to know
 * which move variant we're doing to figure out whether it has to
 * constrain the final coordinates in various ways.
 */
int MoveFunction;

/*
 * Globals used to keep track of whether the mouse has moved during a
 * resize function.
 */
int ResizeOrigX;
int ResizeOrigY;



/*
 * Now, on to the actual handlers.
 */


/*
 *********************************************************
 *
 * First, the various methods of moving windows around.
 *
 *********************************************************
 */

/*
 * Simple f.move and related
 */
static void movewindow(EF_FULLPROTO);
DFHANDLER(move)
{
	movewindow(EF_ARGS);
}
DFHANDLER(forcemove)
{
	movewindow(EF_ARGS);
}
DFHANDLER(movepack)
{
	movewindow(EF_ARGS);
}
DFHANDLER(movepush)
{
	movewindow(EF_ARGS);
}

/* f.move and friends backend */
static void
movewindow(EF_FULLPROTO)
{
	int origX, origY;
	bool moving_icon;
	bool fromtitlebar;
	const Window dragroot = Scr->XineramaRoot;
	const Window rootw = eventp->xbutton.root;

	/* Better not be a menu open */
	PopDownMenu();

	/* Stash up just which f.move* variant we are */
	MoveFunction = func;

	/*
	 * Figure whether we're moving opaquely.
	 */
	if(tmp_win->OpaqueMove) {
		if(Scr->OpaqueMoveThreshold >= 200) {
			Scr->OpaqueMove = true;
		}
		else {
			const unsigned long sw = tmp_win->frame_width
			                         * tmp_win->frame_height;
			const unsigned long ss = Scr->rootw  * Scr->rooth;
			const float sf = Scr->OpaqueMoveThreshold / 100.0;

			if(sw > (ss * sf)) {
				Scr->OpaqueMove = false;
			}
			else {
				Scr->OpaqueMove = true;
			}
		}
	}
	else {
		Scr->OpaqueMove = false;
	}

#ifdef WINBOX
	/* If it's in a WindowBox, adjust coordinates as necessary */
	if(tmp_win->winbox) {
		XTranslateCoordinates(dpy, dragroot, tmp_win->winbox->window,
		                      eventp->xbutton.x_root, eventp->xbutton.y_root,
		                      &(eventp->xbutton.x_root), &(eventp->xbutton.y_root), &JunkChild);
	}
#endif

	/*
	 * XXX pulldown=true only when we're triggering from a ButtonRelease
	 * in a menu, and this warp should only be going somewhere if we hit
	 * the winbox case above and had to translate the coordinates?  But,
	 * in that case, the coordinates would be changed to be relative to
	 * the winbox window, and here we're positioning relative to Root?
	 */
	if(pulldown)
		XWarpPointer(dpy, None, Scr->Root,
		             0, 0, 0, 0, eventp->xbutton.x_root, eventp->xbutton.y_root);

	/*
	 * Stub out handlers for enter/leave notifications while we do stuff.
	 * They get reset toward the end of the ButtonRelease handler.
	 */
	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;

	if(!Scr->NoGrabServer || !Scr->OpaqueMove) {
		XGrabServer(dpy);
	}

	/*
	 * Setup size for the window showing current location as we move it.
	 * The same window is used for resize ops too, where it might be a
	 * different size.
	 */
	Scr->SizeStringOffset = SIZE_HINDENT;
	MoveResizeSizeWindow(eventp->xbutton.x_root, eventp->xbutton.y_root,
	                     Scr->SizeStringWidth + SIZE_HINDENT * 2,
	                     Scr->SizeFont.height + SIZE_VINDENT * 2);
	XMapRaised(dpy, Scr->SizeWindow);

	/*
	 * Use XGrabPointer() to configure how we get events locations
	 * reported relative to what root.
	 */
	{
#ifdef WINBOX
		const Window grabwin = (tmp_win->winbox ? tmp_win->winbox->window
		                        : Scr->XineramaRoot);
#else
		const Window grabwin = Scr->XineramaRoot;
#endif

		XGrabPointer(dpy, grabwin, True,
		             ButtonPressMask | ButtonReleaseMask |
		             ButtonMotionMask | PointerMotionMask,
		             GrabModeAsync, GrabModeAsync, grabwin, Scr->MoveCursor,
		             CurrentTime);
	}

	/*
	 * Set w to what we're actually moving.  If it's an icon, we always
	 * move it opaquely anyway.  If it's a window (that's not iconofied),
	 * we move the frame.
	 */
	moving_icon = false;
	if(context == C_ICON && tmp_win->icon && tmp_win->icon->w) {
		w = tmp_win->icon->w;
		DragX = eventp->xbutton.x;
		DragY = eventp->xbutton.y;
		moving_icon = true;
		if(tmp_win->OpaqueMove) {
			Scr->OpaqueMove = true;
		}
	}
	else if(! tmp_win->icon || w != tmp_win->icon->w) {
		XTranslateCoordinates(dpy, w, tmp_win->frame,
		                      eventp->xbutton.x,
		                      eventp->xbutton.y,
		                      &DragX, &DragY, &JunkChild);

		w = tmp_win->frame;
	}

	DragWindow = None;

	/* Get x/y relative to parent window, i.e. the virtual screen, Root.
	 * XMoveWindow() moves are relative to this.
	 * MoveOutline()s however are drawn from the XineramaRoot since they
	 * may cross virtual screens.
	 */
	XGetGeometry(dpy, w, &JunkRoot, &origDragX, &origDragY,
	             &DragWidth, &DragHeight, &DragBW,
	             &JunkDepth);

	origX = eventp->xbutton.x_root;
	origY = eventp->xbutton.y_root;
	CurrentDragX = origDragX;
	CurrentDragY = origDragY;

	/*
	 * Setup ConstrainedMove if this is a double-click.  That means
	 * setting the flags, and moving the pointer off to the middle of the
	 * window.
	 *
	 * Only do the constrained move if timer is set; need to check it
	 * in case of stupid or wicked fast servers
	 */
	if(ConstrainedMoveTime &&
	                (eventp->xbutton.time - last_time) < ConstrainedMoveTime) {
		int width, height;

		ConstMove = true;
		ConstMoveDir = MOVE_NONE;
		ConstMoveX = eventp->xbutton.x_root - DragX - DragBW;
		ConstMoveY = eventp->xbutton.y_root - DragY - DragBW;
		width = DragWidth + 2 * DragBW;
		height = DragHeight + 2 * DragBW;
		ConstMoveXL = ConstMoveX + width / 3;
		ConstMoveXR = ConstMoveX + 2 * (width / 3);
		ConstMoveYT = ConstMoveY + height / 3;
		ConstMoveYB = ConstMoveY + 2 * (height / 3);

		XWarpPointer(dpy, None, w,
		             0, 0, 0, 0, DragWidth / 2, DragHeight / 2);

		XQueryPointer(dpy, w, &JunkRoot, &JunkChild,
		              &JunkX, &JunkY, &DragX, &DragY, &JunkMask);
	}
	last_time = eventp->xbutton.time;

	/* If not moving opaquely, setup the outline bits */
	if(!Scr->OpaqueMove) {
		InstallRootColormap();
		if(!Scr->MoveDelta) {
			/*
			 * Draw initial outline.  This was previously done the
			 * first time though the outer loop by dropping out of
			 * the XCheckMaskEvent inner loop down to one of the
			 * MoveOutline's below.
			 */
			MoveOutline(dragroot,
			            origDragX - DragBW + Scr->currentvs->x,
			            origDragY - DragBW + Scr->currentvs->y,
			            DragWidth + 2 * DragBW, DragHeight + 2 * DragBW,
			            tmp_win->frame_bw,
			            moving_icon ? 0 : tmp_win->title_height + tmp_win->frame_bw3D);
			/*
			 * This next line causes HandleReleaseNotify to call
			 * XRaiseWindow().  This is solely to preserve the
			 * previous behaviour that raises a window being moved
			 * on button release even if you never actually moved
			 * any distance (unless you move less than MoveDelta or
			 * NoRaiseMove is set or OpaqueMove is set).
			 */
			DragWindow = w;
		}
	}

	/*
	 * Init whether triggered from something on the titlebar (e.g., a
	 * TitleButton bound to f.move).  We need to keep this var in a scope
	 * outside the event loop below because the resetting of it in there
	 * is supposed to have effect on future loops.
	 */
	fromtitlebar = belongs_to_twm_window(tmp_win, eventp->xbutton.window);

	if(menuFromFrameOrWindowOrTitlebar) {
		/* warp the pointer to the middle of the window */
		XWarpPointer(dpy, None, Scr->Root, 0, 0, 0, 0,
		             origDragX + DragWidth / 2,
		             origDragY + DragHeight / 2);
		XFlush(dpy);
	}

	/* Fill in the position window with where we're starting */
	DisplayPosition(tmp_win, CurrentDragX, CurrentDragY);

	/*
	 * Internal event loop for doing the moving.
	 */
	while(1) {
		const long releaseEvent = menuFromFrameOrWindowOrTitlebar ?
		                          ButtonPress : ButtonRelease;
		const long movementMask = menuFromFrameOrWindowOrTitlebar ?
		                          PointerMotionMask : ButtonMotionMask;

		/* block until there is an interesting event */
		XMaskEvent(dpy, ButtonPressMask | ButtonReleaseMask |
		           EnterWindowMask | LeaveWindowMask |
		           ExposureMask | movementMask |
		           VisibilityChangeMask, &Event);

		/* throw away enter and leave events until release */
		if(Event.xany.type == EnterNotify ||
		                Event.xany.type == LeaveNotify) {
			continue;
		}

		/* discard any extra motion events before a logical release */
		if(Event.type == MotionNotify) {
			while(XCheckMaskEvent(dpy, movementMask | releaseEvent, &Event))
				if(Event.type == releaseEvent) {
					break;
				}
		}

		/* test to see if we have a second button press to abort move */
		if(!menuFromFrameOrWindowOrTitlebar) {
			if(Event.type == ButtonPress && DragWindow != None) {
				Cursor cur;
				if(Scr->OpaqueMove) {
					XMoveWindow(dpy, DragWindow, origDragX, origDragY);
					if(moving_icon) {
						tmp_win->icon->w_x = origDragX;
						tmp_win->icon->w_y = origDragY;
					}
				}
				else {
					MoveOutline(dragroot, 0, 0, 0, 0, 0, 0);
				}
				DragWindow = None;

				XUnmapWindow(dpy, Scr->SizeWindow);
				cur = LeftButt;
				if(Event.xbutton.button == Button2) {
					cur = MiddleButt;
				}
				else if(Event.xbutton.button >= Button3) {
					cur = RightButt;
				}

				XGrabPointer(dpy, Scr->Root, True,
				             ButtonReleaseMask | ButtonPressMask,
				             GrabModeAsync, GrabModeAsync,
				             Scr->Root, cur, CurrentTime);
				func_reset_cursor = false;  // Leave cursor alone
				return;
			}
		}

		if(fromtitlebar && Event.type == ButtonPress) {
			fromtitlebar = false;
			CurrentDragX = origX = Event.xbutton.x_root;
			CurrentDragY = origY = Event.xbutton.y_root;
			XTranslateCoordinates(dpy, rootw, tmp_win->frame,
			                      origX, origY,
			                      &DragX, &DragY, &JunkChild);
			continue;
		}

		if(!DispatchEvent2()) {
			continue;
		}

		if(Cancel) {
			WindowMoved = false;
			if(!Scr->OpaqueMove) {
				UninstallRootColormap();
			}
			func_reset_cursor = false;  // Leave cursor alone
			return;
		}
		if(Event.type == releaseEvent) {
			MoveOutline(dragroot, 0, 0, 0, 0, 0, 0);
			if(moving_icon &&
			                ((CurrentDragX != origDragX ||
			                  CurrentDragY != origDragY))) {
				tmp_win->icon_moved = true;
			}
			if(!Scr->OpaqueMove && menuFromFrameOrWindowOrTitlebar) {
				int xl = Event.xbutton.x_root - (DragWidth  / 2),
				    yt = Event.xbutton.y_root - (DragHeight / 2);
				if(!moving_icon &&
				                (MoveFunction == F_MOVEPACK || MoveFunction == F_MOVEPUSH)) {
					TryToPack(tmp_win, &xl, &yt);
				}
				XMoveWindow(dpy, DragWindow, xl, yt);
			}
			if(menuFromFrameOrWindowOrTitlebar) {
				DragWindow = None;
			}
			break;
		}

		/* something left to do only if the pointer moved */
		if(Event.type != MotionNotify) {
			continue;
		}

		/* Get info about where the pointer is */
		XQueryPointer(dpy, rootw, &(eventp->xmotion.root), &JunkChild,
		              &(eventp->xmotion.x_root), &(eventp->xmotion.y_root),
		              &JunkX, &JunkY, &JunkMask);

		/*
		 * Tweak up for root.  XXX Is this even right?  There are too
		 * many Root's, and this corrects for a specific one, but I'm not
		 * sure it's the right one...
		 */
		FixRootEvent(eventp);

		/* Tweak for window box, if this is in one */
#ifdef WINBOX
		if(tmp_win->winbox) {
			XTranslateCoordinates(dpy, dragroot, tmp_win->winbox->window,
			                      eventp->xmotion.x_root, eventp->xmotion.y_root,
			                      &(eventp->xmotion.x_root), &(eventp->xmotion.y_root), &JunkChild);
		}
#endif

		/*
		 * If we haven't moved MoveDelta yet, we're not yet sure we're
		 * doing anything, so just loop back around.
		 */
		if(DragWindow == None &&
		                abs(eventp->xmotion.x_root - origX) < Scr->MoveDelta &&
		                abs(eventp->xmotion.y_root - origY) < Scr->MoveDelta) {
			continue;
		}

		/*
		 * Now we know we're moving whatever the window is.
		 */
		DragWindow = w;

		/* Raise when the move starts if we should */
		if(!Scr->NoRaiseMove && Scr->OpaqueMove && !WindowMoved) {
			TwmWindow *t;

			/*
			 * XXX In several of the error cases listed in here, it's
			 * seems almost that we should just abort the whole move
			 * process immediately if any of them are hit, because things
			 * get nonsensical.
			 */

			/* Find TwmWindow bits related to what we're dragging */
			if(XFindContext(dpy, DragWindow, TwmContext, (XPointer *) &t) == XCNOENT) {
				fprintf(stderr, "%s(): Can't find TwmWindow.\n", __func__);
				/* XXX abort? */
				t = NULL;
			}

			if(t != tmp_win) {
				fprintf(stderr, "%s(): DragWindow isn't tmp_win!\n", __func__);
				/* XXX abort? */
			}

			if(t == NULL) {
				/* Don't try doing this stuff... */
			}
			else if(DragWindow == t->frame) {
				if(moving_icon) {
					fprintf(stderr, "%s(): moving_icon is true incorrectly!\n",
					        __func__);
				}
				OtpRaise(t, WinWin);
			}
			else if(t->icon && DragWindow == t->icon->w) {
				if(!moving_icon) {
					fprintf(stderr, "%s(): moving_icon is false incorrectly!\n",
					        __func__);
				}
				OtpRaise(t, IconWin);
			}
			else {
				fprintf(stderr, "%s(): Couldn't figure what to raise.\n",
				        __func__);
			}
		}

		WindowMoved = true;

		/*
		 * Handle moving the step
		 */
		if(ConstMove) {
			/* Did we already decide it's constrained?  Do that. */
			switch(ConstMoveDir) {
				case MOVE_NONE: {
					/* Haven't figured direction yet, so do so */
					if(eventp->xmotion.x_root < ConstMoveXL ||
					                eventp->xmotion.x_root > ConstMoveXR) {
						ConstMoveDir = MOVE_HORIZ;
					}

					if(eventp->xmotion.y_root < ConstMoveYT ||
					                eventp->xmotion.y_root > ConstMoveYB) {
						ConstMoveDir = MOVE_VERT;
					}

					XQueryPointer(dpy, DragWindow, &JunkRoot, &JunkChild,
					              &JunkX, &JunkY, &DragX, &DragY, &JunkMask);
					break;
				}

				/* We know which dir it's contrained to, so figure amount */
				case MOVE_VERT:
					ConstMoveY = eventp->xmotion.y_root - DragY - DragBW;
					break;

				case MOVE_HORIZ:
					ConstMoveX = eventp->xmotion.x_root - DragX - DragBW;
					break;
			}

			/* We've got a move to do, so do it */
			if(ConstMoveDir != MOVE_NONE) {
				int xl, yt, width, height;

				xl = ConstMoveX;
				yt = ConstMoveY;
				width = DragWidth + 2 * DragBW;
				height = DragHeight + 2 * DragBW;

				if(Scr->DontMoveOff && MoveFunction != F_FORCEMOVE) {
					TryToGrid(tmp_win, &xl, &yt);
				}
				if(!moving_icon && MoveFunction == F_MOVEPUSH && Scr->OpaqueMove) {
					TryToPush(tmp_win, xl, yt);
				}

				if(!moving_icon &&
				                (MoveFunction == F_MOVEPACK || MoveFunction == F_MOVEPUSH)) {
					TryToPack(tmp_win, &xl, &yt);
				}

				if(Scr->DontMoveOff && MoveFunction != F_FORCEMOVE) {
					ConstrainByBorders(tmp_win, &xl, width, &yt, height);
				}
				CurrentDragX = xl;
				CurrentDragY = yt;
				if(Scr->OpaqueMove) {
					if(MoveFunction == F_MOVEPUSH && !moving_icon) {
						SetupWindow(tmp_win, xl, yt,
						            tmp_win->frame_width, tmp_win->frame_height, -1);
					}
					else {
						XMoveWindow(dpy, DragWindow, xl, yt);
						if(moving_icon) {
							tmp_win->icon->w_x = xl;
							tmp_win->icon->w_y = yt;
						}
					}
					WMapSetupWindow(tmp_win, xl, yt, -1, -1);
				}
				else {
					MoveOutline(dragroot, xl + Scr->currentvs->x,
					            yt + Scr->currentvs->y, width, height,
					            tmp_win->frame_bw,
					            moving_icon ? 0 : tmp_win->title_height + tmp_win->frame_bw3D);
				}
			}
		}
		else if(DragWindow != None) {
			/*
			 * There's a non-constrained move to process
			 *
			 * This is split out for virtual screens.  In that case, it's
			 * possible to drag windows from one workspace to another, and
			 * as such, these need to be adjusted to the root, rather
			 * than this virtual screen...
			 */
			const int xroot = eventp->xmotion.x_root;
			const int yroot = eventp->xmotion.y_root;
			const int width  = DragWidth + 2 * DragBW;
			const int height = DragHeight + 2 * DragBW;
			int xl, yt;

			if(!menuFromFrameOrWindowOrTitlebar) {
				xl = xroot - DragX - DragBW;
				yt = yroot - DragY - DragBW;
			}
			else {
				xl = xroot - (DragWidth / 2);
				yt = yroot - (DragHeight / 2);
			}

			if(Scr->DontMoveOff && MoveFunction != F_FORCEMOVE) {
				TryToGrid(tmp_win, &xl, &yt);
			}
			if(!moving_icon && MoveFunction == F_MOVEPUSH && Scr->OpaqueMove) {
				TryToPush(tmp_win, xl, yt);
			}

			if(!moving_icon &&
			                (MoveFunction == F_MOVEPACK || MoveFunction == F_MOVEPUSH)) {
				TryToPack(tmp_win, &xl, &yt);
			}

			if(Scr->DontMoveOff && MoveFunction != F_FORCEMOVE) {
				ConstrainByBorders(tmp_win, &xl, width, &yt, height);
			}

			CurrentDragX = xl;
			CurrentDragY = yt;
			if(Scr->OpaqueMove) {
				if(MoveFunction == F_MOVEPUSH && !moving_icon) {
					SetupWindow(tmp_win, xl, yt,
					            tmp_win->frame_width, tmp_win->frame_height, -1);
				}
				else {
					XMoveWindow(dpy, DragWindow, xl, yt);
					if(moving_icon) {
						tmp_win->icon->w_x = xl;
						tmp_win->icon->w_y = yt;
					}
				}
				if(! moving_icon) {
					WMapSetupWindow(tmp_win, xl, yt, -1, -1);
				}
			}
			else {
				MoveOutline(dragroot, xl + Scr->currentvs->x,
				            yt + Scr->currentvs->y, width, height,
				            tmp_win->frame_bw,
				            moving_icon ? 0 : tmp_win->title_height + tmp_win->frame_bw3D);
			}
		}

		/* We've moved a step, so update the displayed position */
		DisplayPosition(tmp_win, CurrentDragX, CurrentDragY);
	}

	/* Done, so hide away the position display window */
	XUnmapWindow(dpy, Scr->SizeWindow);

	/* Restore colormap if we replaced it */
	if(!Scr->OpaqueMove && DragWindow == None) {
		UninstallRootColormap();
	}

	return;
}


/*
 * f.pack -- moving until collision
 *
 * XXX Collapse this down; no need for an extra level of indirection on
 * the function calling.
 */
static void packwindow(TwmWindow *tmp_win, const char *direction);
DFHANDLER(pack)
{
	if(tmp_win->squeezed) {
		XBell(dpy, 0);
		return;
	}
	packwindow(tmp_win, action);
}

static void
packwindow(TwmWindow *tmp_win, const char *direction)
{
	int          cons, newx, newy;
	int          x, y, px, py, junkX, junkY;
	unsigned int junkK;
	Window       junkW;

	if(!strcmp(direction,   "left")) {
		cons  = FindConstraint(tmp_win, MFD_LEFT);
		if(cons == -1) {
			return;
		}
		newx  = cons;
		newy  = tmp_win->frame_y;
	}
	else if(!strcmp(direction,  "right")) {
		cons  = FindConstraint(tmp_win, MFD_RIGHT);
		if(cons == -1) {
			return;
		}
		newx  = cons;
		newx -= tmp_win->frame_width + 2 * tmp_win->frame_bw;
		newy  = tmp_win->frame_y;
	}
	else if(!strcmp(direction,    "top")) {
		cons  = FindConstraint(tmp_win, MFD_TOP);
		if(cons == -1) {
			return;
		}
		newx  = tmp_win->frame_x;
		newy  = cons;
	}
	else if(!strcmp(direction, "bottom")) {
		cons  = FindConstraint(tmp_win, MFD_BOTTOM);
		if(cons == -1) {
			return;
		}
		newx  = tmp_win->frame_x;
		newy  = cons;
		newy -= tmp_win->frame_height + 2 * tmp_win->frame_bw;
	}
	else {
		return;
	}

	XQueryPointer(dpy, Scr->Root, &junkW, &junkW, &junkX, &junkY, &x, &y, &junkK);
	px = x - tmp_win->frame_x + newx;
	py = y - tmp_win->frame_y + newy;
	XWarpPointer(dpy, Scr->Root, Scr->Root, 0, 0, 0, 0, px, py);
	OtpRaise(tmp_win, WinWin);
	XMoveWindow(dpy, tmp_win->frame, newx, newy);
	SetupWindow(tmp_win, newx, newy, tmp_win->frame_width,
	            tmp_win->frame_height, -1);
}


/*
 * f.jump* -- moving incrementally in various directions
 */
static void jump(TwmWindow *tmp_win, MoveFillDir direction, const char *action);
DFHANDLER(jumpleft)
{
	jump(tmp_win, MFD_LEFT, action);
}
DFHANDLER(jumpright)
{
	jump(tmp_win, MFD_RIGHT, action);
}
DFHANDLER(jumpdown)
{
	jump(tmp_win, MFD_BOTTOM, action);
}
DFHANDLER(jumpup)
{
	jump(tmp_win, MFD_TOP, action);
}

static void
jump(TwmWindow *tmp_win, MoveFillDir direction, const char *action)
{
	int          fx, fy, px, py, step, status, cons;
	int          fwidth, fheight;
	int          junkX, junkY;
	unsigned int junkK;
	Window       junkW;

	if(tmp_win->squeezed) {
		XBell(dpy, 0);
		return;
	}

	if(! action) {
		return;
	}
	status = sscanf(action, "%d", &step);
	if(status != 1) {
		return;
	}
	if(step < 1) {
		return;
	}

	fx = tmp_win->frame_x;
	fy = tmp_win->frame_y;
	XQueryPointer(dpy, Scr->Root, &junkW, &junkW, &junkX, &junkY, &px, &py, &junkK);
	px -= fx;
	py -= fy;

	fwidth  = tmp_win->frame_width  + 2 * tmp_win->frame_bw;
	fheight = tmp_win->frame_height + 2 * tmp_win->frame_bw;
	switch(direction) {
		case MFD_LEFT:
			cons  = FindConstraint(tmp_win, MFD_LEFT);
			if(cons == -1) {
				return;
			}
			fx -= step * Scr->XMoveGrid;
			if(fx < cons) {
				fx = cons;
			}
			break;
		case MFD_RIGHT:
			cons  = FindConstraint(tmp_win, MFD_RIGHT);
			if(cons == -1) {
				return;
			}
			fx += step * Scr->XMoveGrid;
			if(fx + fwidth > cons) {
				fx = cons - fwidth;
			}
			break;
		case MFD_TOP:
			cons  = FindConstraint(tmp_win, MFD_TOP);
			if(cons == -1) {
				return;
			}
			fy -= step * Scr->YMoveGrid;
			if(fy < cons) {
				fy = cons;
			}
			break;
		case MFD_BOTTOM:
			cons  = FindConstraint(tmp_win, MFD_BOTTOM);
			if(cons == -1) {
				return;
			}
			fy += step * Scr->YMoveGrid;
			if(fy + fheight > cons) {
				fy = cons - fheight;
			}
			break;
	}
	/* Pebl Fixme: don't warp if jump happens through iconmgr */
	XWarpPointer(dpy, Scr->Root, Scr->Root, 0, 0, 0, 0, fx + px, fy + py);
	if(!Scr->NoRaiseMove) {
		OtpRaise(tmp_win, WinWin);
	}
	SetupWindow(tmp_win, fx, fy, tmp_win->frame_width, tmp_win->frame_height, -1);
}



/*
 *********************************************************
 *
 * Next up, straightforward resizing operations
 *
 *********************************************************
 */

/*
 * Standard f.resize
 */
DFHANDLER(resize)
{
	PopDownMenu();
	if(tmp_win->squeezed) {
		XBell(dpy, 0);
		return;
	}
	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;

	OpaqueResizeSize(tmp_win);

	if(pulldown)
		XWarpPointer(dpy, None, Scr->Root,
		             0, 0, 0, 0, eventp->xbutton.x_root, eventp->xbutton.y_root);

	if(!tmp_win->icon || (w != tmp_win->icon->w)) {         /* can't resize icons */

		/*        fromMenu = False;  ????? */
		if((Context == C_FRAME || Context == C_WINDOW || Context == C_TITLE
		                || Context == C_ROOT)
		                && cur_fromMenu()) {
			resizeFromCenter(w, tmp_win);
		}
		else {
			/*
			 * see if this is being done from the titlebar
			 */
			bool from3dborder = (eventp->xbutton.window == tmp_win->frame);
			bool fromtitlebar = !from3dborder &&
			                    belongs_to_twm_window(tmp_win, eventp->xbutton.window);

			/* Save pointer position so we can tell if it was moved or
			   not during the resize. */
			ResizeOrigX = eventp->xbutton.x_root;
			ResizeOrigY = eventp->xbutton.y_root;

			StartResize(eventp, tmp_win, fromtitlebar, from3dborder);
			func_reset_cursor = false;  // Leave special cursor alone

			do {
				XMaskEvent(dpy,
				           ButtonPressMask | ButtonReleaseMask |
				           EnterWindowMask | LeaveWindowMask |
				           ButtonMotionMask | VisibilityChangeMask | ExposureMask, &Event);

				if(fromtitlebar && Event.type == ButtonPress) {
					fromtitlebar = false;
					continue;
				}

				if(Event.type == MotionNotify) {
					/* discard any extra motion events before a release */
					while
					(XCheckMaskEvent
					                (dpy, ButtonMotionMask | ButtonReleaseMask, &Event))
						if(Event.type == ButtonRelease) {
							break;
						}
				}

				if(!DispatchEvent2()) {
					continue;
				}

			}
			while(!(Event.type == ButtonRelease || Cancel));
		}
	}
	return;
}


/*
 * The various zoom resizes
 */
DFHANDLER(zoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(horizoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(fullzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(fullscreenzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(leftzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(rightzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(topzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(bottomzoom)
{
	fullzoom(tmp_win, func);
}

DFHANDLER(xzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xhorizoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xfullzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xfullscreenzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xleftzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xrightzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xtopzoom)
{
	fullzoom(tmp_win, func);
}
DFHANDLER(xbottomzoom)
{
	fullzoom(tmp_win, func);
}


/*
 * f.fill - resizing until collision
 *
 * XXX Similar to f.pack's, collapse away this extra level of function.
 */
static void fillwindow(TwmWindow *tmp_win, const char *direction);
DFHANDLER(fill)
{
	if(tmp_win->squeezed) {
		XBell(dpy, 0);
		return;
	}
	fillwindow(tmp_win, action);
}

static void
fillwindow(TwmWindow *tmp_win, const char *direction)
{
	int cons, newx, newy, save;
	unsigned int neww, newh;
	int i;
	const int winx = tmp_win->frame_x;
	const int winy = tmp_win->frame_y;
	const int winw = tmp_win->frame_width  + 2 * tmp_win->frame_bw;
	const int winh = tmp_win->frame_height + 2 * tmp_win->frame_bw;

	if(!strcmp(direction, "left")) {
		cons = FindConstraint(tmp_win, MFD_LEFT);
		if(cons == -1) {
			return;
		}
		newx = cons;
		newy = tmp_win->frame_y;
		neww = winw + winx - newx;
		newh = winh;
		neww -= 2 * tmp_win->frame_bw;
		newh -= 2 * tmp_win->frame_bw;
		ConstrainSize(tmp_win, &neww, &newh);
	}
	else if(!strcmp(direction, "right")) {
		for(i = 0; i < 2; i++) {
			cons = FindConstraint(tmp_win, MFD_RIGHT);
			if(cons == -1) {
				return;
			}
			newx = tmp_win->frame_x;
			newy = tmp_win->frame_y;
			neww = cons - winx;
			newh = winh;
			save = neww;
			neww -= 2 * tmp_win->frame_bw;
			newh -= 2 * tmp_win->frame_bw;
			ConstrainSize(tmp_win, &neww, &newh);
			if((neww != winw) || (newh != winh) ||
			                (cons == Scr->rootw - Scr->BorderRight)) {
				break;
			}
			neww = save;
			SetupWindow(tmp_win, newx, newy, neww, newh, -1);
		}
	}
	else if(!strcmp(direction, "top")) {
		cons = FindConstraint(tmp_win, MFD_TOP);
		if(cons == -1) {
			return;
		}
		newx = tmp_win->frame_x;
		newy = cons;
		neww = winw;
		newh = winh + winy - newy;
		neww -= 2 * tmp_win->frame_bw;
		newh -= 2 * tmp_win->frame_bw;
		ConstrainSize(tmp_win, &neww, &newh);
	}
	else if(!strcmp(direction, "bottom")) {
		for(i = 0; i < 2; i++) {
			cons = FindConstraint(tmp_win, MFD_BOTTOM);
			if(cons == -1) {
				return;
			}
			newx = tmp_win->frame_x;
			newy = tmp_win->frame_y;
			neww = winw;
			newh = cons - winy;
			save = newh;
			neww -= 2 * tmp_win->frame_bw;
			newh -= 2 * tmp_win->frame_bw;
			ConstrainSize(tmp_win, &neww, &newh);
			if((neww != winw) || (newh != winh) ||
			                (cons == Scr->rooth - Scr->BorderBottom)) {
				break;
			}
			newh = save;
			SetupWindow(tmp_win, newx, newy, neww, newh, -1);
		}
	}
	else if(!strcmp(direction, "vertical")) {
		if(tmp_win->zoomed == ZOOM_NONE) {
			tmp_win->save_frame_height = tmp_win->frame_height;
			tmp_win->save_frame_width = tmp_win->frame_width;
			tmp_win->save_frame_y = tmp_win->frame_y;
			tmp_win->save_frame_x = tmp_win->frame_x;

			tmp_win->frame_y++;
			newy = FindConstraint(tmp_win, MFD_TOP);
			tmp_win->frame_y--;
			newh = FindConstraint(tmp_win, MFD_BOTTOM) - newy;
			newh -= 2 * tmp_win->frame_bw;

			newx = tmp_win->frame_x;
			neww = tmp_win->frame_width;

			ConstrainSize(tmp_win, &neww, &newh);

			/* if the bottom of the window has moved up
			 * it will be pushed down */
			if(newy + newh < tmp_win->save_frame_y + tmp_win->save_frame_height) {
				newy = tmp_win->save_frame_y +
				       tmp_win->save_frame_height - newh;
			}
			tmp_win->zoomed = F_ZOOM;
			SetupWindow(tmp_win, newx, newy, neww, newh, -1);
		}
		else {
			fullzoom(tmp_win, tmp_win->zoomed);
		}
		return;
	}
	else {
		return;
	}
	SetupWindow(tmp_win, newx, newy, neww, newh, -1);
}



/*
 *********************************************************
 *
 * Resizing/moving to specified geometries
 *
 *********************************************************
 */

/*
 * Resizing to a window's idea of its "normal" size, from WM_NORMAL_HINTS
 * property.
 */
DFHANDLER(initsize)
{
	int grav, x, y;
	unsigned int width, height, swidth, sheight;

	grav = ((tmp_win->hints.flags & PWinGravity)
	        ? tmp_win->hints.win_gravity : NorthWestGravity);

	if(!(tmp_win->hints.flags & USSize) && !(tmp_win->hints.flags & PSize)) {
		return;
	}

	width  = tmp_win->hints.width  + 2 * tmp_win->frame_bw3D;
	height  = tmp_win->hints.height + 2 * tmp_win->frame_bw3D +
	          tmp_win->title_height;
	ConstrainSize(tmp_win, &width, &height);

	x  = tmp_win->frame_x;
	y  = tmp_win->frame_y;
	swidth = tmp_win->frame_width;
	sheight = tmp_win->frame_height;

	switch(grav) {
		case ForgetGravity:
		case StaticGravity:
		case NorthWestGravity:
		case NorthGravity:
		case WestGravity:
		case CenterGravity:
			break;

		case NorthEastGravity:
		case EastGravity:
			x += swidth - width;
			break;

		case SouthWestGravity:
		case SouthGravity:
			y += sheight - height;
			break;

		case SouthEastGravity:
			x += swidth - width;
			y += sheight - height;
			break;
	}

	SetupWindow(tmp_win, x, y, width, height, -1);
	return;
}


/*
 * Setting a window to a specific specified geometry string.
 */
DFHANDLER(moveresize)
{
	int x, y, mask;
	unsigned int width, height;
	int px = 20, py = 30;

	mask = RLayoutXParseGeometry(Scr->Layout, action, &x, &y, &width, &height);
	if(!(mask &  WidthValue)) {
		width = tmp_win->frame_width;
	}
	else {
		width += 2 * tmp_win->frame_bw3D;
	}
	if(!(mask & HeightValue)) {
		height = tmp_win->frame_height;
	}
	else {
		height += 2 * tmp_win->frame_bw3D + tmp_win->title_height;
	}
	ConstrainSize(tmp_win, &width, &height);
	if(mask & XValue) {
		if(mask & XNegative) {
			x += Scr->rootw  - width;
		}
	}
	else {
		x = tmp_win->frame_x;
	}
	if(mask & YValue) {
		if(mask & YNegative) {
			y += Scr->rooth - height;
		}
	}
	else {
		y = tmp_win->frame_y;
	}

	{
		int          junkX, junkY;
		unsigned int junkK;
		Window       junkW;
		XQueryPointer(dpy, Scr->Root, &junkW, &junkW, &junkX, &junkY, &px, &py, &junkK);
	}
	px -= tmp_win->frame_x;
	if(px > width) {
		px = width / 2;
	}
	py -= tmp_win->frame_y;
	if(py > height) {
		px = height / 2;
	}

	SetupWindow(tmp_win, x, y, width, height, -1);
	XWarpPointer(dpy, Scr->Root, Scr->Root, 0, 0, 0, 0, x + px, y + py);
	return;
}


/*
 * Making a specified alteration to a window's size
 */
DFHANDLER(changesize)
{
	/* XXX Only use of this func; should we collapse? */
	ChangeSize(action, tmp_win);
}


/*
 * Stashing and flipping back to a geometry
 */
DFHANDLER(savegeometry)
{
	savegeometry(tmp_win);
}

DFHANDLER(restoregeometry)
{
	restoregeometry(tmp_win);
}




/*
 *********************************************************
 *
 * Misc utils used in the above
 *
 *********************************************************
 */

/*
 * Used in the various move/fill/pack/etc bits
 */
static int
FindConstraint(TwmWindow *tmp_win, MoveFillDir direction)
{
	TwmWindow  *t;
	int ret, limit;
	const int winx = tmp_win->frame_x;
	const int winy = tmp_win->frame_y;
	const int winw = tmp_win->frame_width  + 2 * tmp_win->frame_bw;
	const int winh = tmp_win->frame_height + 2 * tmp_win->frame_bw;

	RArea area = RAreaNew(winx, winy, winw, winh);

	switch(direction) {
		case MFD_LEFT:
			limit = RLayoutFindMonitorLeftEdge(Scr->BorderedLayout, &area);
			if(winx < limit) {
				return -1;
			}
			ret = limit;
			break;
		case MFD_RIGHT:
			limit = RLayoutFindMonitorRightEdge(Scr->BorderedLayout, &area);
			if(winx + winw > limit) {
				return -1;
			}
			ret = limit + 1;
			break;
		case MFD_TOP:
			limit = RLayoutFindMonitorTopEdge(Scr->BorderedLayout, &area);
			if(winy < limit) {
				return -1;
			}
			ret = limit;
			break;
		case MFD_BOTTOM:
			limit = RLayoutFindMonitorBottomEdge(Scr->BorderedLayout, &area);
			if(winy + winh > limit) {
				return -1;
			}
			ret = limit + 1;
			break;
		default:
			return -1;
	}
	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		const int w = t->frame_width  + 2 * t->frame_bw;
		const int h = t->frame_height + 2 * t->frame_bw;

		if(t == tmp_win) {
			continue;
		}
		if(!visible(t)) {
			continue;
		}
		if(!t->mapped) {
			continue;
		}

		switch(direction) {
			case MFD_LEFT:
				if(winx        <= t->frame_x + w) {
					continue;
				}
				if(winy        >= t->frame_y + h) {
					continue;
				}
				if(winy + winh <= t->frame_y) {
					continue;
				}
				ret = MAX(ret, t->frame_x + w);
				break;
			case MFD_RIGHT:
				if(winx + winw >= t->frame_x) {
					continue;
				}
				if(winy        >= t->frame_y + h) {
					continue;
				}
				if(winy + winh <= t->frame_y) {
					continue;
				}
				ret = MIN(ret, t->frame_x);
				break;
			case MFD_TOP:
				if(winy        <= t->frame_y + h) {
					continue;
				}
				if(winx        >= t->frame_x + w) {
					continue;
				}
				if(winx + winw <= t->frame_x) {
					continue;
				}
				ret = MAX(ret, t->frame_y + h);
				break;
			case MFD_BOTTOM:
				if(winy + winh >= t->frame_y) {
					continue;
				}
				if(winx        >= t->frame_x + w) {
					continue;
				}
				if(winx + winw <= t->frame_x) {
					continue;
				}
				ret = MIN(ret, t->frame_y);
				break;
		}
	}
	return ret;
}


/*
 * Is Window w part of the conglomerate of metawindows we put around the
 * real window for TwmWindow t?  Note that this does _not_ check if w is
 * the actual window we built the TwmWindow t around.
 */
static bool
belongs_to_twm_window(TwmWindow *t, Window w)
{
	/* Safety */
	if(!t) {
		return false;
	}

	/* Part of the framing we put around the window? */
	if(w == t->frame || w == t->title_w
	                || w == t->hilite_wl || w == t->hilite_wr) {
		return true;
	}

	/* Part of the icon bits? */
	if(t->icon && (w == t->icon->w || w == t->icon->bm_w)) {
		return true;
	}

	/* One of the title button windows? */
	if(t->titlebuttons) {
		TBWindow *tbw;
		int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;
		for(tbw = t->titlebuttons ; nb > 0 ; tbw++, nb--) {
			if(tbw->window == w) {
				return true;
			}
		}
	}

	/* Then no */
	return false;
}
