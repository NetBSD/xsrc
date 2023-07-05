/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: events.c,v 1.182 91/07/17 13:59:14 dave Exp $
 *
 * twm event handling
 *
 * 17-Nov-87 Thomas E. LaStrange                File created
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 ***********************************************************************/

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#include "add_window.h"
#include "animate.h"
#include "clicktofocus.h"
#include "colormaps.h"
#include "ctwm_atoms.h"
#include "ctwm_shutdown.h"
#include "events.h"
#include "event_handlers.h"
#include "event_internal.h"
#include "event_names.h"
#include "functions.h"
#include "functions_defs.h"
#include "gram.tab.h"
#include "iconmgr.h"
#include "icons.h"
#include "image.h"
#include "list.h"
#include "occupation.h"
#include "otp.h"
#include "parse.h"
#include "screen.h"
#include "util.h"
#include "vscreen.h"
#include "win_decorations.h"
#include "win_iconify.h"
#include "win_ops.h"
#include "win_regions.h"
#include "win_resize.h"
#include "win_ring.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "workspace_utils.h"


static void do_key_menu(MenuRoot *menu,         /* menu to pop up */
                        Window w);             /* invoking window or None */

/* Only called from HandleFocusChange() */
static void HandleFocusIn(void);
static void HandleFocusOut(void);

/*
 * This currently needs to live in the broader scope because of how it's
 * used in deferred function handling.
 */
static char *Action;

static TwmWindow *ButtonWindow; /* button press window structure */

static void SendTakeFocusMessage(TwmWindow *tmp, Time timestamp);


static unsigned int set_mask_ignore(unsigned int modifier)
{
	modifier &= ~Scr->IgnoreModifier;

	return modifier;
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleColormapNotify - colormap notify event handler
 *
 * This procedure handles both a client changing its own colormap, and
 * a client explicitly installing its colormap itself (only the window
 * manager should do that, so we must set it correctly).
 *
 ***********************************************************************
 */

void HandleColormapNotify(void)
{
	XColormapEvent *cevent = (XColormapEvent *) &Event;
	ColormapWindow *cwin, **cwins;
	TwmColormap *cmap;
	int lost, won, n, number_cwins;

	/*    if (! Tmp_win) return; */
	if(XFindContext(dpy, cevent->window, ColormapContext,
	                (XPointer *)&cwin) == XCNOENT) {
		return;
	}
	cmap = cwin->colormap;

	if(cevent->new) {
		if(XFindContext(dpy, cevent->colormap, ColormapContext,
		                (XPointer *)&cwin->colormap) == XCNOENT) {
			cwin->colormap = CreateTwmColormap(cevent->colormap);
		}
		else {
			cwin->colormap->refcnt++;
		}

		cmap->refcnt--;

		if(cevent->state == ColormapUninstalled) {
			cmap->state &= ~CM_INSTALLED;
		}
		else {
			cmap->state |= CM_INSTALLED;
		}

		if(cmap->state & CM_INSTALLABLE) {
			InstallColormaps(ColormapNotify, NULL);
		}

		if(cmap->refcnt == 0) {
			XDeleteContext(dpy, cmap->c, ColormapContext);
			free(cmap);
		}

		return;
	}

	if(cevent->state == ColormapUninstalled &&
	                (cmap->state & CM_INSTALLABLE)) {
		if(!(cmap->state & CM_INSTALLED)) {
			return;
		}
		cmap->state &= ~CM_INSTALLED;

		if(!ColortableThrashing) {
			ColortableThrashing = true;
			XSync(dpy, 0);
		}

		if(cevent->serial >= Scr->cmapInfo.first_req) {
			number_cwins = Scr->cmapInfo.cmaps->number_cwins;

			/*
			 * Find out which colortables collided.
			 */

			cwins = Scr->cmapInfo.cmaps->cwins;
			for(lost = won = -1, n = 0;
			                (lost == -1 || won == -1) && n < number_cwins;
			                n++) {
				if(lost == -1 && cwins[n] == cwin) {
					lost = n;   /* This is the window which lost its colormap */
					continue;
				}

				if(won == -1 &&
				                cwins[n]->colormap->install_req == cevent->serial) {
					won = n;    /* This is the window whose colormap caused */
					continue;   /* the de-install of the previous colormap */
				}
			}

			/*
			** Cases are:
			** Both the request and the window were found:
			**          One of the installs made honoring the WM_COLORMAP
			**          property caused another of the colormaps to be
			**          de-installed, just mark the scoreboard.
			**
			** Only the request was found:
			**          One of the installs made honoring the WM_COLORMAP
			**          property caused a window not in the WM_COLORMAP
			**          list to lose its map.  This happens when the map
			**          it is losing is one which is trying to be installed,
			**          but is getting getting de-installed by another map
			**          in this case, we'll get a scoreable event later,
			**          this one is meaningless.
			**
			** Neither the request nor the window was found:
			**          Somebody called installcolormap, but it doesn't
			**          affect the WM_COLORMAP windows.  This case will
			**          probably never occur.
			**
			** Only the window was found:
			**          One of the WM_COLORMAP windows lost its colormap
			**          but it wasn't one of the requests known.  This is
			**          probably because someone did an "InstallColormap".
			**          The colormap policy is "enforced" by re-installing
			**          the colormaps which are believed to be correct.
			*/

			if(won != -1) {
				if(lost != -1) {
					/* lower diagonal index calculation */
					if(lost > won) {
						n = lost * (lost - 1) / 2 + won;
					}
					else {
						n = won * (won - 1) / 2 + lost;
					}
					Scr->cmapInfo.cmaps->scoreboard[n] = 1;
				}
				else {
					/*
					** One of the cwin installs caused one of the cwin
					** colormaps to be de-installed, so I'm sure to get an
					** UninstallNotify for the cwin I know about later.
					** I haven't got it yet, or the test of CM_INSTALLED
					** above would have failed.  Turning the CM_INSTALLED
					** bit back on makes sure we get back here to score
					** the collision.
					*/
					cmap->state |= CM_INSTALLED;
				}
			}
			else if(lost != -1) {
				InstallColormaps(ColormapNotify, NULL);
			}
			else {
				ColortableThrashing = false; /* Gross Hack for HP WABI. CL. */
			}
		}
	}

	else if(cevent->state == ColormapUninstalled) {
		cmap->state &= ~CM_INSTALLED;
	}

	else if(cevent->state == ColormapInstalled) {
		cmap->state |= CM_INSTALLED;
	}
}


/*
 * LastFocusEvent -- skip over focus in/out events for this
 *              window.
 */

static XEvent *LastFocusEvent(Window w, XEvent *first)
{
	static XEvent current;
	XEvent *last, new;

	new = *first;
	last = NULL;

	do {
		if((new.type == FocusIn || new.type == FocusOut)
		                && new.xfocus.mode == NotifyNormal
		                && (new.xfocus.detail == NotifyNonlinear
		                    || new.xfocus.detail == NotifyPointer
		                    || new.xfocus.detail == NotifyAncestor
		                    || (new.xfocus.detail == NotifyNonlinearVirtual)
		                   )) {
			current = new;
			last = &current;
		}

#ifdef TRACE_FOCUS
		fprintf(stderr, "%s(): Focus%s 0x%x mode=%d, detail=%d\n",
		        __func__, new.xfocus.type == FocusIn ? "In" : "Out",
		        Tmp_win, new.xfocus.mode, new.xfocus.detail);
#endif

	}
	while(XCheckWindowEvent(dpy, w, FocusChangeMask, &new));
	return last;
}


/*
 * Focus change handlers.
 *
 * Depending on how events get called, these are sometimes redundant, as
 * the Enter event handler does practically all of this anyway.  But
 * there are presumably ways we can wind up Focus'ing a window without
 * Enter'ing it as well.
 *
 * It's also a little convoluted how these wind up getting called.  With
 * most events, we call a handler, then handle that event.  However, with
 * focus, we troll through our list of pending Focus-related events for
 * the window and just handle the last one, since some could pile up
 * fast.  That means that, even if we get called for a FocusIn event,
 * there might be a FocusOut later in the queue, and _that_'s the one we
 * pick up and handle, and we discard the rest [for that window].  So,
 * the event handling code calls a single entry point for both types, and
 * then it figures out which backend handler to actually fire.
 */
void
HandleFocusChange(void)
{
	XEvent *event;

	/* If there's no event window, nothing to do */
	if(!Tmp_win) {
		return;
	}

	/*
	 * Consume all the focus events for the window we're called about and
	 * grab the last one to process.
	 *
	 * XXX It should be guaranteed that the window in the X event in our
	 * global Event is the same as Tmp_win->w as the event dispatcher
	 * sets it so.  Maybe we should do both checks on the same var for
	 * consistency though?
	 *
	 * It's not immediately clear how this can wind up returning nothing,
	 * but if it does, we don't have anything to do either.
	 */
	event = LastFocusEvent(Event.xany.window, &Event);
	if(event == NULL) {
		return;
	}

	/*
	 * Icon managers don't do anything with focus events on themselves,
	 * so just skip back if this is one.  Done after LastFocusEvent()
	 * call for efficiency, so we don't fall into this func multiple
	 * times if multiple events are queued for it.
	 */
	if(Tmp_win->isiconmgr) {
		return;
	}

#ifdef TRACE_FOCUS
	fprintf(stderr, "HandleFocus%s(): 0x%x (0x%x, 0x%x), mode=%d, "
	        "detail=%d\n",
	        (event->type == FocusIn ? "In" : "Out"),
	        Tmp_win, Tmp_win->w, event->window, event->mode,
	        event->detail);
#endif

	/* And call actual handler */
	if(event->type == FocusIn) {
		HandleFocusIn();
	}
	else {
		HandleFocusOut();
	}
}


static void
HandleFocusIn(void)
{
	if(! Tmp_win->wmhints->input) {
		return;
	}
	if(Scr->Focus == Tmp_win) {
		return;
	}

#ifdef EWMH
	// Handle focus-dependent re-stacking of what we're moving out of.
	if(Scr->Focus && OtpIsFocusDependent(Scr->Focus)) {
		OtpUnfocusWindow(Scr->Focus);
		// NULL's Scr->Focus
	}
#endif

	if(Tmp_win->AutoSqueeze && Tmp_win->squeezed) {
		AutoSqueeze(Tmp_win);
	}
	SetFocusVisualAttributes(Tmp_win, true);

#ifdef EWMH
	// Handle focus-dependent re-stacking of what we're moving in to.
	if(Tmp_win && OtpIsFocusDependent(Tmp_win)) {
		OtpFocusWindow(Tmp_win);
		// Sets Scr->Focus
	}
#endif

	// Redundant in EWMH case
	Scr->Focus = Tmp_win;
}


static void
HandleFocusOut(void)
{
	if(Scr->Focus != Tmp_win) {
		return;
	}
	if(Scr->SloppyFocus) {
		return;
	}
	if(Tmp_win->AutoSqueeze && !Tmp_win->squeezed) {
		AutoSqueeze(Tmp_win);
	}
	SetFocusVisualAttributes(Tmp_win, false);

#ifdef EWMH
	/*
	 * X-ref HandleFocusIn() comment.  FocusOut is only leaving a window,
	 * not entering a new one, so there's only one we may need to
	 * restack.
	 */
	if(Scr->Focus && OtpIsFocusDependent(Scr->Focus)) {
		OtpUnfocusWindow(Scr->Focus);
		// NULL's Scr->Focus
	}
#endif

	// Redundant in EWMH case
	Scr->Focus = NULL;
}



/*
 * Only sent if SubstructureNotifyMask is selected on the (root) window.
 */
void HandleCirculateNotify(void)
{
	VirtualScreen *vs;
#ifdef DEBUG_EVENTS
	fprintf(stderr, "HandleCirculateNotify\n");
	fprintf(stderr, "event=%x window=%x place=%d\n",
	        (unsigned)Event.xcirculate.event,
	        (unsigned)Event.xcirculate.window,
	        Event.xcirculate.place);
#endif

	for(vs = Scr->vScreenList; vs; vs = vs->next) {
		if(Event.xcirculate.event == vs->window) {
			TwmWindow *twm_win = GetTwmWindow(Event.xcirculate.window);

			if(twm_win) {
				WinType wt;

				if(Event.xcirculate.window == twm_win->frame) {
					wt = WinWin;
				}
				else if(twm_win->icon &&
				                Event.xcirculate.window == twm_win->icon->w) {
					wt = IconWin;
				}
				else {
					return;
				}

				OtpHandleCirculateNotify(vs,
				                         twm_win, wt,
				                         Event.xcirculate.place);
			}
		}
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      HandleVisibilityNotify - visibility notify event handler
 *
 * This routine keeps track of visibility events so that colormap
 * installation can keep the maximum number of useful colormaps
 * installed at one time.
 *
 ***********************************************************************
 */

void HandleVisibilityNotify(void)
{
	XVisibilityEvent *vevent = (XVisibilityEvent *) &Event;
	ColormapWindow *cwin;
	TwmColormap *cmap;

	if(XFindContext(dpy, vevent->window, ColormapContext,
	                (XPointer *)&cwin) == XCNOENT) {
		return;
	}

	/*
	 * when Saber complains about retreiving an <int> from an <unsigned int>
	 * just type "touch vevent->state" and "cont"
	 */
	cmap = cwin->colormap;
	if((cmap->state & CM_INSTALLABLE) &&
	                vevent->state != cwin->visibility &&
	                (vevent->state == VisibilityFullyObscured ||
	                 cwin->visibility == VisibilityFullyObscured) &&
	                cmap->w == cwin->w) {
		cwin->visibility = vevent->state;
		InstallWindowColormaps(VisibilityNotify, NULL);
	}
	else {
		cwin->visibility = vevent->state;
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleKeyRelease - key release event handler
 *
 ***********************************************************************
 */

void HandleKeyRelease(void)
{
	if(Tmp_win == Scr->currentvs->wsw->twm_win) {
		WMgrHandleKeyReleaseEvent(Scr->currentvs, &Event);
	}
}



/*
 * HandleKeyPress - key press event handler
 *
 * When a key is pressed, we may do various things with it.  If we're in
 * a menu, various keybindings move around in it, others get silently
 * ignored.  Else, we look through the various bindings set in the config
 * file and invoke whatever should be.  If none of that matches, and it
 * seems like some window should have focus, pass the event down to that
 * window.
 */
void HandleKeyPress(void)
{
	/*
	 * If the Info window (f.identify/f.version) is currently up, any key
	 * press will drop it away.
	 */
	if(Scr->InfoWindow.mapped) {
		XUnmapWindow(dpy, Scr->InfoWindow.win);
		Scr->InfoWindow.mapped = false;
	}


	/*
	 * If a menu is up, we interpret various keys as moving around or
	 * doing things in the menu.  No other key bindings or usages are
	 * considered.
	 */
	if(ActiveMenu != NULL) {
		MenuItem *item;
		char *keynam;
		KeySym keysym;
		Window junkW;

		item = NULL;

		/* What key was pressed? */
		keysym = XLookupKeysym((XKeyEvent *) &Event, 0);
		if(! keysym) {
			return;
		}
		keynam = XKeysymToString(keysym);
		if(! keynam) {
			return;
		}


		/*
		 * Initial handling of the various keystrokes.  Most keys are
		 * completely handled here; we only descend out into later for
		 * for Return/Right keys that do invoke-y stuff on menu entries.
		 */
		if(keysym == XK_Down || keysym == XK_space) {
			/*
			 * Down or Spacebar moves us to the next entry in the menu,
			 * looping back around to the top when it falls off the
			 * bottom.
			 *
			 * Start with our X and (current+height)Y, then wrap around
			 * to the top (Y)/into the menu (X) as necessary.
			 */
			int xx = Event.xkey.x;
			int yy = Event.xkey.y + Scr->EntryHeight;
			int wx, wy;
			XTranslateCoordinates(dpy, Scr->Root, ActiveMenu->w, xx, yy, &wx, &wy, &junkW);
			if((wy < 0) || (wy > ActiveMenu->height)) {
				yy -= (wy - (Scr->EntryHeight / 2) - 2);
			}
			if((wx < 0) || (wx > ActiveMenu->width)) {
				xx -= (wx - (ActiveMenu->width / 2));
			}

			/*
			 * Move the pointer there.  We'll get a Motion notify from
			 * the X server as a result, which will fall into the loop in
			 * UpdateMenu() and handle re-highlighting etc.
			 */
			XWarpPointer(dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			             ActiveMenu->width, ActiveMenu->height, xx, yy);
			return;
		}
		else if(keysym == XK_Up || keysym == XK_BackSpace) {
			/*
			 * Up/Backspace move up an entry, with details similar in
			 * reverse to the above.
			 */
			int xx = Event.xkey.x;
			int yy = Event.xkey.y - Scr->EntryHeight;
			int wx, wy;
			XTranslateCoordinates(dpy, Scr->Root, ActiveMenu->w, xx, yy, &wx, &wy, &junkW);
			if((wy < 0) || (wy > ActiveMenu->height)) {
				yy -= (wy - ActiveMenu->height + (Scr->EntryHeight / 2) + 2);
			}
			if((wx < 0) || (wx > ActiveMenu->width)) {
				xx -= (wx - (ActiveMenu->width / 2));
			}
			XWarpPointer(dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			             ActiveMenu->width, ActiveMenu->height, xx, yy);
			return;
		}
		else if(keysym == XK_Right || keysym == XK_Return) {
			/*
			 * Right/Return mean we're invoking some entry item, so we
			 * take note of where we are for activating at the end of
			 * this set of conditionals.
			 *
			 * Follow this down into the following if(item) block for
			 * details, particularly in the subtle differences between
			 * Right and Return on f.menu entries.
			 */
			item = ActiveItem;
		}
		else if(keysym == XK_Left || keysym == XK_Escape) {
			/*
			 * Left/Escape back up to a higher menu level, or out totally
			 * from the top.
			 */
			MenuRoot *menu;

			/* Leave pinned menus alone though */
			if(ActiveMenu->pinned) {
				return;
			}

			/* Top-level?  Clear out and leave menu mode totally. */
			if(!ActiveMenu->prev || MenuDepth == 1) {
				PopDownMenu();
				XUngrabPointer(dpy, CurrentTime);
				return;
			}

			/*
			 * We're in a sub level.  Figure out various stuff for where
			 * we are and where we should be in the up-level, clear out
			 * the windows for this level, and warp us up there.
			 */
			int xx = Event.xkey.x;
			int yy = Event.xkey.y;
			int wx, wy;
			menu = ActiveMenu->prev;
			XTranslateCoordinates(dpy, Scr->Root, menu->w, xx, yy, &wx, &wy, &junkW);
			xx -= (wx - (menu->width / 2));
			if(menu->lastactive)
				yy -= (wy - menu->lastactive->item_num * Scr->EntryHeight -
				       (Scr->EntryHeight / 2) - 2);
			else {
				yy -= (wy - (Scr->EntryHeight / 2) - 2);
			}
			XUnmapWindow(dpy, ActiveMenu->w);
			if(Scr->Shadow) {
				XUnmapWindow(dpy, ActiveMenu->shadow);
			}
			XWarpPointer(dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			             menu->width, menu->height, xx, yy);
			return;
		}
		else if(strlen(keynam) == 1) {
			/*
			 * This would mean pressing a more normal (e.g., letter/num)
			 * key.  These find the first entry starting with a matching
			 * character and jump to it.
			 */
			MenuItem *startitem;
			int xx = Event.xkey.x;
			int yy = Event.xkey.y;
			int wx, wy;

			startitem = ActiveItem ? ActiveItem : ActiveMenu->first;
			item = startitem->next;
			if(item == NULL) {
				item = ActiveMenu->first;
			}
			unsigned int keymod = (Event.xkey.state & mods_used);
			keymod = set_mask_ignore(keymod);

			while(item != startitem) {
				bool matched = false;
				size_t offset = 0;
				switch(item->item [0]) {
					case '^' :
						if((keymod & ControlMask) &&
						                (keynam [0] == Tolower(item->item [1]))) {
							matched = true;
						}
						break;
					case '~' :
						if((keymod & Mod1Mask) &&
						                (keynam [0] == Tolower(item->item [1]))) {
							matched = true;
						}
						break;
					case ' ' :
						offset = 1;
					default :
						if(((Scr->IgnoreCaseInMenuSelection) &&
						                (keynam [0] == Tolower(item->item [offset]))) ||

						                ((keymod & ShiftMask) && Isupper(item->item [offset]) &&
						                 (keynam [0] == Tolower(item->item [offset]))) ||

						                (!(keymod & ShiftMask) && Islower(item->item [offset]) &&
						                 (keynam [0] == item->item [offset]))) {
							matched = true;
						}
						break;
				}
				if(matched) {
					break;
				}
				item = item->next;
				if(item == NULL) {
					item = ActiveMenu->first;
				}
			}
			if(item == startitem) {
				return;
			}
			wx = ActiveMenu->width / 2;
			wy = (item->item_num * Scr->EntryHeight) + (Scr->EntryHeight / 2) + 2;
			XTranslateCoordinates(dpy, ActiveMenu->w, Scr->Root, wx, wy, &xx, &yy, &junkW);
			XWarpPointer(dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			             ActiveMenu->width, ActiveMenu->height, xx, yy);
			return;
		}
		else {
			/* Other keys get ignored */
			return;
		}


		/*
		 * So if we get here, the key pressed was a Right/Return on an
		 * entry to select it (chosen entry now in item).  Every other
		 * case is have been completely handled in the block above and
		 * would have already returned.
		 *
		 * So item should always be the entry we just tried to invoke.
		 * I'm not sure how it could be empty, but if it is, we just hop
		 * ourselves out of the menu.  Otherwise, we do whatever we want
		 * to do with the entry type we're on.
		 */
		if(item) {
			switch(item->func) {
				/* f.nop and f.title, we just silently let pass */
				case 0 :
				case F_TITLE :
					break;

				/* If it's a f.menu, there's more magic to do */
				case F_MENU: {
					/*
					 * Return is treated separately from Right.  It
					 * "invokes" the menu item, which immediately calls
					 * whatever the default menu entry is (which may be
					 * nothing).
					 */
					if(!strcmp(keynam, "Return")) {
						if(ActiveMenu == Scr->Workspaces) {
							/*
							 * f.menu "TwmWorkspaces".  The "invocation"
							 * of this jumps to the workspace in
							 * question, as if it were a default entry of
							 * f.gotoworkspace.
							 *
							 * XXX Grody magic.  Maybe this should be
							 * unwound to a default entry...
							 */
							PopDownMenu();
							XUngrabPointer(dpy, CurrentTime);
							GotoWorkSpaceByName(Scr->currentvs, item->action + 8);
						}
						else {
							/*
							 * Calling the f.menu handler invokes the
							 * default action.  We handle popping out of
							 * menus ourselves.
							 */
							ExecuteFunction(item->func, item->action,
							                ButtonWindow ? ButtonWindow->frame : None,
							                ButtonWindow, &Event, Context, false);
							PopDownMenu();
						}

						/*
						 * Whatever invocation Return does is done, so we
						 * are too.
						 */
						return;
					}

					/*
					 * Right arrow causes opening up a sub-f.menu.  Open
					 * it up in the appropriate place, [re-]set
					 * highlights, and call do_key_menu() to do a lot of
					 * the internals of it.
					 */
					int xx = Event.xkey.x;
					int yy = Event.xkey.y;
					int wx, wy;
					XTranslateCoordinates(dpy, Scr->Root, ActiveMenu->w, xx, yy,
					                      &wx, &wy, &junkW);
					if(ActiveItem) {
						ActiveItem->state = 0;
						PaintEntry(ActiveMenu, ActiveItem, false);
						ActiveItem = NULL;
					}
					xx -= (wx - ActiveMenu->width);
					yy -= (wy - item->item_num * Scr->EntryHeight - (Scr->EntryHeight / 2) - 2);
					Event.xkey.x_root = xx;
					Event.xkey.y_root = yy;
					XWarpPointer(dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
					             ActiveMenu->width, ActiveMenu->height, xx, yy);
					if(ActiveMenu == Scr->Workspaces) {
						CurrentSelectedWorkspace = item->item;
					}
					do_key_menu(item->sub, None);
					CurrentSelectedWorkspace = NULL;
					break;
				}

				/*
				 * Any other f.something.  Pop down the menu (unless
				 * we're trying to pin it up), and invoke the function.
				 */
				default :
					if(item->func != F_PIN) {
						PopDownMenu();
					}
					ExecuteFunction(item->func, item->action,
					                ButtonWindow ? ButtonWindow->frame : None,
					                ButtonWindow, &Event, Context, false);
			}

			/* Done whatever invocation of the entry we need */
		}
		else {
			/* Was no item; pop out of the menu */
			PopDownMenu();
			XUngrabPointer(dpy, CurrentTime);
		}

		/*
		 * We're done handling the keypress in a menu, so there's nothing
		 * else to do.
		 */
		return;
	}


	/*
	 * Not in a menu, so we loop through our various bindings.  First,
	 * figure out what context we're in.  This goes in a global var,
	 * presumably because stuff way down the chain of invoking some item
	 * may need to refer up to it.
	 */
	Context = C_NO_CONTEXT;
	if(Event.xany.window == Scr->Root) {
		if(AlternateContext) {
			XUngrabPointer(dpy, CurrentTime);
			XUngrabKeyboard(dpy, CurrentTime);
			AlternateContext = false;
			Context = C_ALTERNATE;
		}
		else if(AlternateKeymap && Event.xkey.subwindow) {
			Tmp_win = GetTwmWindow(Event.xkey.subwindow);
			if(Tmp_win) {
				Event.xany.window = Tmp_win->w;
			}
		}
		else {
			Context = C_ROOT;
		}
	}
	if(Tmp_win) {
		if(0) {
			/* Dummy to simplify constructions of else if's */
		}
#ifdef EWMH_DESKTOP_ROOT
		else if(Tmp_win->ewmhWindowType == wt_Desktop) {
			fprintf(stderr, "HandleKeyPress: wt_Desktop -> C_ROOT\n");
			Context = C_ROOT;
		}
#endif
		else if(Event.xany.window == Tmp_win->title_w) {
			Context = C_TITLE;
		}
		else if(Event.xany.window == Tmp_win->w) {
			Context = C_WINDOW;
		}
		else if(Tmp_win->icon && (Event.xany.window == Tmp_win->icon->w)) {
			Context = C_ICON;
		}
		else if(Event.xany.window == Tmp_win->frame) {
			Context = C_FRAME;
		}
		else if(Tmp_win->iconmanagerlist) {
			if(Event.xany.window == Tmp_win->iconmanagerlist->w ||
			                Event.xany.window == Tmp_win->iconmanagerlist->icon) {
				Context = C_ICONMGR;
			}
		}
		if(Tmp_win->iswspmgr) {
			Context = C_WORKSPACE;
		}
	}

	/*
	 * We've figured out the Context.  Now see what modifiers we might
	 * have set...
	 */
	unsigned int modifier = (Event.xkey.state | AlternateKeymap) & mods_used;
	modifier = set_mask_ignore(modifier);
	if(AlternateKeymap) {
		XUngrabPointer(dpy, CurrentTime);
		XUngrabKeyboard(dpy, CurrentTime);
		AlternateKeymap = 0;
	}


	/*
	 * Loop over our key bindings and do its thing if we find a matching
	 * one.
	 */
	for(FuncKey *key = Scr->FuncKeyRoot.next; key != NULL; key = key->next) {
		/*
		 * Is this what we're trying to invoke?  Gotta be the right key,
		 * and right modifier; those are easy.
		 *
		 * Context is tougher; that has to match what we're expecting as
		 * well, except in the case of C_NAME, which we always have to
		 * check to see if it'll match any windows.  So if we have the
		 * right key and modifier, and it's a C_NAME context, it's a
		 * "maybe" match and we have to go through the checks.
		 */
		if(key->keycode != Event.xkey.keycode ||
		                key->mods != modifier ||
		                (key->cont != Context && key->cont != C_NAME)) {
			/* Nope, not yet */
			continue;
		}

		/* 'k, it's a match (or potential match, in C_NAME case) */

		/*
		 * Weed out the functions that don't make sense to execute from a
		 * key press
		 *
		 * TODO: add keyboard moving/resizing of windows.
		 */
		if(key->func == F_MOVE || key->func == F_RESIZE) {
			return;
		}

		if(key->cont != C_NAME) {
			/* Normal context binding; do what it wants */
			if(key->func == F_MENU) {
				/*
				 * f.menu doesn't call the f.menu handler; we directly
				 * do_key_menu() to pull it up.
				 *
				 * Note this is "we called f.menu from a keybinding", not
				 * "we hit f.menu inside a menu we had up"; that's above.
				 */
				ButtonWindow = Tmp_win;
				do_key_menu(key->menu, (Window) None);
			}
			else {
#ifdef EWMH_DESKTOP_ROOT
				if(Context == C_ROOT && Tmp_win != NULL) {
					Context = C_WINDOW;
					fprintf(stderr, "HandleKeyPress: wt_Desktop -> C_WINDOW\n");
				}
#endif /* EWMH */
				ExecuteFunction(key->func, key->action, Event.xany.window,
				                Tmp_win, &Event, Context, false);
				if(!AlternateKeymap && !AlternateContext) {
					XUngrabPointer(dpy, CurrentTime);
				}
			}
			return;
		}
		else {
			/*
			 * By-name binding (i.e., quoted string for the context
			 * argument in config; see the manual).  Find windows
			 * matching that name and invoke on them, if any.
			 *
			 * This is the 'maybe' case above; we don't know whether this
			 * does something until we try it.  If we don't get a match,
			 * we loop back around and keep going through our functions
			 * until we do.
			 */
			bool matched = false;
			const size_t len = strlen(key->win_name);

			/* try and match the name first */
			for(Tmp_win = Scr->FirstWindow; Tmp_win != NULL;
			                Tmp_win = Tmp_win->next) {
				if(!strncmp(key->win_name, Tmp_win->name, len)) {
					matched = true;
					ExecuteFunction(key->func, key->action, Tmp_win->frame,
					                Tmp_win, &Event, C_FRAME, false);
					if(!AlternateKeymap && !AlternateContext) {
						XUngrabPointer(dpy, CurrentTime);
					}
				}
			}

			/* now try the res_name */
			if(!matched) {
				for(Tmp_win = Scr->FirstWindow; Tmp_win != NULL;
				                Tmp_win = Tmp_win->next) {
					if(!strncmp(key->win_name, Tmp_win->class.res_name, len)) {
						matched = true;
						ExecuteFunction(key->func, key->action, Tmp_win->frame,
						                Tmp_win, &Event, C_FRAME, false);
						if(!AlternateKeymap && !AlternateContext) {
							XUngrabPointer(dpy, CurrentTime);
						}
					}
				}
			}

			/* now try the res_class */
			if(!matched) {
				for(Tmp_win = Scr->FirstWindow; Tmp_win != NULL;
				                Tmp_win = Tmp_win->next) {
					if(!strncmp(key->win_name, Tmp_win->class.res_class, len)) {
						matched = true;
						ExecuteFunction(key->func, key->action, Tmp_win->frame,
						                Tmp_win, &Event, C_FRAME, false);
						if(!AlternateKeymap && !AlternateContext) {
							XUngrabPointer(dpy, CurrentTime);
						}
					}
				}
			}

			/*
			 * If we wound up invoking something, we're done, so return.
			 * If we didn't, we fall through to the next loop through our
			 * defined bindings.
			 *
			 * By-name bindings are unique in this; normal contexts
			 * couldn't have multiple matches, so that side of things
			 * finishes when it deals with its found match.  But with
			 * by-name we could have multiple bindings of a given
			 * button/modifier with different names, so we have to go
			 * back around to the next run through the for() loop.
			 */
			if(matched) {
				return;
			}
		} // regular context or by-name?
	} // Loop over all bindings


	/*
	 * If we get here, no function key was bound to the key.  Send it to
	 * the client if it was in a window we know about.  Mostly this
	 * doesn't happen; clients with focus get their events more directly,
	 * but special cases may cause this.
	 */
	if(Tmp_win) {
		if(Context == C_WORKSPACE) {
			WMgrHandleKeyPressEvent(Scr->currentvs, &Event);
			return;
		}
		if(Context == C_ICON ||
		                Context == C_FRAME ||
		                Context == C_TITLE ||
		                Context == C_ICONMGR) {
			Event.xkey.window = Tmp_win->w;
			XSendEvent(dpy, Tmp_win->w, False, KeyPressMask, &Event);
		}
	}


	/* And done */
}



/***********************************************************************
 *
 *  Procedure:
 *      HandlePropertyNotify - property notify event handler
 *
 ***********************************************************************
 */

void HandlePropertyNotify(void)
{
	Atom actual = None;
	int actual_format;
	unsigned long nitems, bytesafter;
	unsigned long valuemask;            /* mask for create windows */
	XSetWindowAttributes attributes;    /* attributes for create windows */
	Pixmap pm;
	Icon *icon;


	/* watch for standard colormap changes */
	if(Event.xproperty.window == Scr->Root) {

		if(Event.xproperty.atom == XA_WM_CURRENTWORKSPACE) {
			unsigned char *prop;
			switch(Event.xproperty.state) {
				case PropertyNewValue:
					if(XGetWindowProperty(dpy, Scr->Root, XA_WM_CURRENTWORKSPACE,
					                      0L, 200L, False, XA_STRING, &actual, &actual_format,
					                      &nitems, &bytesafter, &prop) == Success) {
						if(nitems == 0) {
							return;
						}
						GotoWorkSpaceByName(Scr->vScreenList, (char *)prop);
						XFree(prop);
					}
					return;

				default:
					return;
			}
		}
		switch(Event.xproperty.state) {
			case PropertyNewValue: {
				XStandardColormap *maps = NULL;
				int nmaps;

				if(XGetRGBColormaps(dpy, Scr->Root, &maps, &nmaps,
				                    Event.xproperty.atom)) {
					/* if got one, then replace any existing entry */
					InsertRGBColormap(Event.xproperty.atom, maps, nmaps, true);
				}
				return;
			}

			case PropertyDelete:
				RemoveRGBColormap(Event.xproperty.atom);
				return;
		}
	}

	if(!Tmp_win) {
		return;        /* unknown window */
	}

#define MAX_NAME_LEN 200L               /* truncate to this many */

	switch(Event.xproperty.atom) {
		case XA_WM_NAME: {
			char *prop = GetWMPropertyString(Tmp_win->w, XA_WM_NAME);
			if(prop == NULL) {
				// Clear
				FreeWMPropertyString(Tmp_win->names.wm_name);
				Tmp_win->names.wm_name = NULL;
				apply_window_name(Tmp_win);
				return;
			}

			if(Tmp_win->names.wm_name != NULL
			                && strcmp(Tmp_win->names.wm_name, prop) == 0) {
				/* No change, just free and skip out */
				free(prop);
				return;
			}

			/* It's changing, free the old and bring in the new */
			FreeWMPropertyString(Tmp_win->names.wm_name);
			Tmp_win->names.wm_name = prop;

			/* Kick the reset process */
			apply_window_name(Tmp_win);

			break;
		}

		case XA_WM_ICON_NAME: {
			char *prop = GetWMPropertyString(Tmp_win->w, XA_WM_ICON_NAME);
			if(prop == NULL) {
				// Clear
				FreeWMPropertyString(Tmp_win->names.wm_icon_name);
				Tmp_win->names.wm_icon_name = NULL;
				apply_window_icon_name(Tmp_win);
				return;
			}

			/* No change?  Nothing to do. */
			if(Tmp_win->names.wm_icon_name != NULL
			                && strcmp(Tmp_win->names.wm_icon_name, prop) == 0) {
				free(prop);
				return;
			}

			/* It's changing, free the old and bring in the new */
			FreeWMPropertyString(Tmp_win->names.wm_icon_name);
			Tmp_win->names.wm_icon_name = prop;

			/* And show the new */
			apply_window_icon_name(Tmp_win);

			break;
		}

		case XA_WM_HINTS: {
			{
				XWMHints *nhints = XGetWMHints(dpy, Event.xany.window);
				if(!nhints) {
					/*
					 * I guess this means that window removed the
					 * property completely.  Just keep using what we
					 * already have for it though; we gotta have
					 * something, and whatever it last said is probably
					 * more reasonable than getting completely new
					 * synthetic hints anyway.
					 */
					break;
				}

				XFree(Tmp_win->wmhints);
				Tmp_win->wmhints = munge_wmhints(Tmp_win, nhints);
			}

			icon = Tmp_win->icon;

			/*
			 * If there already is an icon found in a way that has priority
			 * over these hints, disable the flags and remove them from
			 * consideration, now and in the future.
			 */
			if(Tmp_win->forced ||
			                (icon && icon->match == match_net_wm_icon)) {
				Tmp_win->wmhints->flags &= ~(IconWindowHint | IconPixmapHint | IconMaskHint);
			}

			if(Tmp_win->wmhints->flags & IconWindowHint) {
				if(icon && icon->w) {
					int icon_x, icon_y;

					/*
					 * There's already an icon window.
					 * Try to find out where it is; if we succeed, move the new
					 * window to where the old one is.
					 */
					if(XGetGeometry(dpy, icon->w, &JunkRoot, &icon_x,
					                &icon_y, &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth)) {
						/*
						 * Move the new icon window to where the old one was.
						 */
						XMoveWindow(dpy, Tmp_win->wmhints->icon_window, icon_x,
						            icon_y);
					}

					/*
					 * If the window is iconic, map the new icon window.
					 */
					if(Tmp_win->isicon) {
						XMapWindow(dpy, Tmp_win->wmhints->icon_window);
					}

					/*
					 * Now, if the old window isn't ours, unmap it, otherwise
					 * just get rid of it completely.
					 */
					if(icon->w_not_ours) {
						if(icon->w != Tmp_win->wmhints->icon_window) {
							XUnmapWindow(dpy, icon->w);
						}
					}
					else {
						XDestroyWindow(dpy, icon->w);
					}

					/*
					 * The new icon window isn't our window, so note that fact
					 * so that we don't treat it as ours.
					 */
					icon->w_not_ours = true;

					/*
					 * Now make the new window the icon window for this window,
					 * and set it up to work as such (select for key presses
					 * and button presses/releases, set up the contexts for it,
					 * and define the cursor for it).
					 */
					icon->w = Tmp_win->wmhints->icon_window;
					XSelectInput(dpy, icon->w,
					             KeyPressMask | ButtonPressMask | ButtonReleaseMask);
					XSaveContext(dpy, icon->w, TwmContext, (XPointer)Tmp_win);
					XSaveContext(dpy, icon->w, ScreenContext, (XPointer)Scr);
					XDefineCursor(dpy, icon->w, Scr->IconCursor);
				}
			}

			if(icon && icon->w &&
			                (Tmp_win->wmhints->flags & IconPixmapHint)) {
				int x;
				unsigned int IconDepth;

				if(!XGetGeometry(dpy, Tmp_win->wmhints->icon_pixmap, &JunkRoot,
				                 &JunkX, &JunkY, (unsigned int *)&icon->width,
				                 (unsigned int *)&icon->height, &JunkBW,
				                 &IconDepth)) {
					return;
				}

				pm = XCreatePixmap(dpy, Scr->Root, icon->width,
				                   icon->height, Scr->d_depth);

				FB(icon->iconc.fore, icon->iconc.back);

				if(IconDepth == Scr->d_depth)
					XCopyArea(dpy, Tmp_win->wmhints->icon_pixmap, pm, Scr->NormalGC,
					          0, 0, icon->width, icon->height, 0, 0);
				else
					XCopyPlane(dpy, Tmp_win->wmhints->icon_pixmap, pm, Scr->NormalGC,
					           0, 0, icon->width, icon->height, 0, 0, 1);

				if(icon->image) {
					/* Release the existing Image: it may be a shared one (UnknownIcon) */
					ReleaseIconImage(icon);
					/* conjure up a new Image */
					Image *image = AllocImage();
					image->pixmap = pm;
					image->width  = icon->width;
					image->height = icon->height;
					icon->image = image;
					icon->match = match_icon_pixmap_hint;
				}

				valuemask = CWBackPixmap;
				attributes.background_pixmap = pm;

				if(icon->bm_w) {
					XDestroyWindow(dpy, icon->bm_w);
				}

				x = GetIconOffset(icon);
				icon->bm_w =
				        XCreateWindow(dpy, icon->w, x, 0,
				                      icon->width,
				                      icon->height,
				                      0, Scr->d_depth,
				                      CopyFromParent, Scr->d_visual,
				                      valuemask, &attributes);

				if(!(Tmp_win->wmhints->flags & IconMaskHint)) {
					XRectangle rect;

					rect.x      = x;
					rect.y      = 0;
					rect.width  = icon->width;
					rect.height = icon->height;
					XShapeCombineRectangles(dpy, icon->w, ShapeBounding, 0,
					                        0, &rect, 1, ShapeUnion, 0);
				}
				XMapSubwindows(dpy, icon->w);
				RedoIconName(Tmp_win);
			}
			if(icon && icon->w &&
			                (Tmp_win->wmhints->flags & IconMaskHint) &&
			                icon->match == match_icon_pixmap_hint) {
				/* Only set the mask if the pixmap came from a WM_HINTS too,
				 * for easier resource management.
				 */
				int x;
				Pixmap mask;
				GC gc;
				unsigned int IconWidth, IconHeight, IconDepth;

				if(!XGetGeometry(dpy, Tmp_win->wmhints->icon_mask, &JunkRoot,
				                 &JunkX, &JunkY, &IconWidth, &IconHeight, &JunkBW,
				                 &IconDepth)) {
					return;
				}
				if(IconDepth != 1) {
					return;
				}

				mask = XCreatePixmap(dpy, Scr->Root, IconWidth, IconHeight, 1);
				if(!mask) {
					return;
				}
				gc = XCreateGC(dpy, mask, 0, NULL);
				if(!gc) {
					return;
				}
				XCopyArea(dpy, Tmp_win->wmhints->icon_mask, mask, gc,
				          0, 0, IconWidth, IconHeight, 0, 0);
				XFreeGC(dpy, gc);
				x = GetIconOffset(icon);
				XShapeCombineMask(dpy, icon->bm_w, ShapeBounding, 0, 0, mask,
				                  ShapeSet);
				XShapeCombineMask(dpy, icon->w,    ShapeBounding, x, 0, mask,
				                  ShapeSet);
				if(icon->image) {
					if(icon->image->mask) {
						XFreePixmap(dpy, icon->image->mask);
					}
					icon->image->mask = mask;
					RedoIconName(Tmp_win);
				}
			}
			if(Tmp_win->wmhints->flags & IconPixmapHint) {
				AutoPopupMaybe(Tmp_win);
			}

			break;
		}

		case XA_WM_NORMAL_HINTS: {
			GetWindowSizeHints(Tmp_win);
			break;
		}
		default: {
			if(Event.xproperty.atom == XA_WM_COLORMAP_WINDOWS) {
				FetchWmColormapWindows(Tmp_win);    /* frees old data */
				break;
			}
			else if(Event.xproperty.atom == XA_WM_PROTOCOLS) {
				FetchWmProtocols(Tmp_win);
				break;
			}
			else if(Event.xproperty.atom == XA_WM_OCCUPATION) {
				unsigned char *prop;
				if(XGetWindowProperty(dpy, Tmp_win->w, Event.xproperty.atom, 0L, MAX_NAME_LEN,
				                      False,
				                      XA_STRING, &actual, &actual_format, &nitems,
				                      &bytesafter, &prop) != Success ||
				                actual == None) {
					return;
				}
				ChangeOccupation(Tmp_win, GetMaskFromProperty(prop, nitems));
				XFree(prop);
			}
			else if(Event.xproperty.atom == XA_CTWM_WM_NAME) {
				char *prop = GetWMPropertyString(Tmp_win->w, XA_CTWM_WM_NAME);
				if(prop == NULL) {
					// Clearing
					FreeWMPropertyString(Tmp_win->names.ctwm_wm_name);
					Tmp_win->names.ctwm_wm_name = NULL;
					apply_window_name(Tmp_win);
					return;
				}

				if(Tmp_win->names.ctwm_wm_name != NULL
				                && strcmp(Tmp_win->names.ctwm_wm_name,
				                          prop) == 0) {
					/* No change, just free and skip out */
					free(prop);
					return;
				}

				/* It's changing, free the old and bring in the new */
				FreeWMPropertyString(Tmp_win->names.ctwm_wm_name);
				Tmp_win->names.ctwm_wm_name = prop;

				/* Kick the reset process */
				apply_window_name(Tmp_win);

				return;
			}
			else if(Event.xproperty.atom == XA_CTWM_WM_ICON_NAME) {
				char *prop = GetWMPropertyString(Tmp_win->w,
				                                 XA_CTWM_WM_ICON_NAME);
				if(prop == NULL) {
					// Clearing
					FreeWMPropertyString(Tmp_win->names.ctwm_wm_icon_name);
					Tmp_win->names.ctwm_wm_icon_name = NULL;
					apply_window_icon_name(Tmp_win);
					return;
				}

				if(Tmp_win->names.ctwm_wm_icon_name != NULL
				                && strcmp(Tmp_win->names.ctwm_wm_icon_name,
				                          prop) == 0) {
					/* No change, just free and skip out */
					free(prop);
					return;
				}

				/* It's changing, free the old and bring in the new */
				FreeWMPropertyString(Tmp_win->names.ctwm_wm_icon_name);
				Tmp_win->names.ctwm_wm_icon_name = prop;

				/* Kick the reset process */
				apply_window_icon_name(Tmp_win);

				break;
			}
#ifdef EWMH
			else if(EwmhHandlePropertyNotify(&Event.xproperty, Tmp_win)) {
				/* event handled */
			}
#endif /* EWMH */
			break;
		}
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleClientMessage - client message event handler
 *
 ***********************************************************************
 */

void HandleClientMessage(void)
{

	if(Event.xclient.message_type == XA_WM_CHANGE_STATE) {
		if(Tmp_win != NULL) {
			if(Event.xclient.data.l[0] == IconicState && !Tmp_win->isicon) {
				XEvent button;
				XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
				              &(button.xmotion.x_root),
				              &(button.xmotion.y_root),
				              &JunkX, &JunkY, &JunkMask);

				ExecuteFunction(F_ICONIFY, NULL, Event.xany.window,
				                Tmp_win, &button, FRAME, false);
				XUngrabPointer(dpy, CurrentTime);
			}
		}
		return;
	}

#ifdef EWMH
	if(EwmhClientMessage(&Event.xclient)) {
		return;
	}
#endif

	else if((Event.xclient.message_type == XA_WM_PROTOCOLS) &&
	                (Event.xclient.data.l[0] == XA_WM_END_OF_ANIMATION)) {
		if(Animating > 0) {
			Animating--;
		}
		else {
			fprintf(stderr, "!! end of unknown animation !!\n");
		}
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleExpose - expose event handler
 *
 ***********************************************************************
 */

static void flush_expose(Window w);

void HandleExpose(void)
{
	MenuRoot *tmp;
	VirtualScreen *vs;

	if(XFindContext(dpy, Event.xany.window, MenuContext, (XPointer *)&tmp) == 0) {
		PaintMenu(tmp, &Event);
		return;
	}

	if(Event.xexpose.count != 0) {
		return;
	}

	if(Event.xany.window == Scr->InfoWindow.win && Scr->InfoWindow.mapped) {
		draw_info_window();
		flush_expose(Event.xany.window);
	}
	else if(Tmp_win != NULL) {
		if(Scr->use3Dborders && (Event.xany.window == Tmp_win->frame)) {
			PaintBorders(Tmp_win, ((Tmp_win == Scr->Focus) ? true : false));
			flush_expose(Event.xany.window);
			return;
		}
		else if(Event.xany.window == Tmp_win->title_w) {
			PaintTitle(Tmp_win);
			flush_expose(Event.xany.window);
			return;
		}
		else if(Tmp_win->icon && (Event.xany.window == Tmp_win->icon->w) &&
		                ! Scr->NoIconTitlebar &&
		                ! LookInList(Scr->NoIconTitle, Tmp_win->name, &Tmp_win->class)) {
			PaintIcon(Tmp_win);
			flush_expose(Event.xany.window);
			return;
		}
		else if(Tmp_win->titlebuttons) {
			int i;
			TBWindow *tbw;
			Window w = Event.xany.window;
			int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

			/*
			 * This looks an awful lot like a manual reimplementation of
			 * PaintTitleButtons().  It's not quite though, it's just
			 * looking up one button to paint it.  And it would be a
			 * little grody trying to shoehorn it in.
			 */
			for(i = 0, tbw = Tmp_win->titlebuttons; i < nb; i++, tbw++) {
				if(w == tbw->window) {
					PaintTitleButton(Tmp_win, tbw);
					flush_expose(tbw->window);
					return;
				}
			}
		}
		for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
			if(Tmp_win == vs->wsw->twm_win) {
				WMgrHandleExposeEvent(vs, &Event);
				flush_expose(Event.xany.window);
				return;
			}
		}
		if(Tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win) {
			/*
			 * The occupyWindow has a bunch of sub-windows for each
			 * button in it, and each of them wind up getting Expose
			 * events kicked for them.  The upshot is that we re-paint
			 * the occupy window once for itself, and then once for each
			 * button inside it.  We'll always get one for the window
			 * itself, so just paint it for that one, and ignore the rest
			 * of the events.
			 *
			 * XXX Maybe a better solution is just to mask off Expose
			 * events for the other windows...
			 */
			if(Event.xany.window == Scr->workSpaceMgr.occupyWindow->w) {
				PaintOccupyWindow();
				flush_expose(Event.xany.window);
			}
			return;
		}
		else if(Tmp_win->iconmanagerlist) {
			WList *iconmanagerlist = Tmp_win->iconmanagerlist;

			if(Event.xany.window == iconmanagerlist->w) {
				DrawIconManagerIconName(Tmp_win);
				flush_expose(Event.xany.window);
				return;
			}
			else if(Event.xany.window == iconmanagerlist->icon) {
				ShowIconifiedIcon(Tmp_win);
				flush_expose(Event.xany.window);
				return;
			}
		}
	}
}


static void remove_window_from_ring(TwmWindow *tmp)
{
	if(enter_win == tmp) {
		enter_flag = false;
		enter_win = NULL;
	}
	if(raise_win == Tmp_win) {
		raise_win = NULL;
	}
	if(leave_win == tmp) {
		leave_flag = false;
		leave_win = NULL;
	}
	if(lower_win == Tmp_win) {
		lower_win = NULL;
	}

	UnlinkWindowFromRing(tmp);
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleDestroyNotify - DestroyNotify event handler
 *
 ***********************************************************************
 */

void HandleDestroyNotify(void)
{
	/*
	 * Warning, this is also called by HandleUnmapNotify; if it ever needs to
	 * look at the event, HandleUnmapNotify will have to mash the UnmapNotify
	 * into a DestroyNotify.
	 */

	if(Tmp_win == NULL) {
		return;
	}

	RemoveWindowFromRegion(Tmp_win);

	if(Tmp_win->icon != NULL) {
		OtpRemove(Tmp_win, IconWin);
	}
	OtpRemove(Tmp_win, WinWin);

#ifdef EWMH
	/* Remove the old window from the EWMH client list */
	EwmhDeleteClientWindow(Tmp_win);
	EwmhSet_NET_CLIENT_LIST_STACKING();
#endif /* EWMH */
	if(Tmp_win == Scr->Focus) {
		Scr->Focus = NULL;
		FocusOnRoot();
	}
	if(Scr->SaveWorkspaceFocus) {
		struct WorkSpace *ws;
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			if(ws->save_focus == Tmp_win) {
				ws->save_focus = NULL;
			}
		}
	}
	XDeleteContext(dpy, Tmp_win->w, TwmContext);
	XDeleteContext(dpy, Tmp_win->w, ScreenContext);
	XDeleteContext(dpy, Tmp_win->frame, TwmContext);
	XDeleteContext(dpy, Tmp_win->frame, ScreenContext);
	if(Tmp_win->icon && Tmp_win->icon->w) {
		XDeleteContext(dpy, Tmp_win->icon->w, TwmContext);
		XDeleteContext(dpy, Tmp_win->icon->w, ScreenContext);
	}
	if(Tmp_win->title_height) {
		int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

		XDeleteContext(dpy, Tmp_win->title_w, TwmContext);
		XDeleteContext(dpy, Tmp_win->title_w, ScreenContext);
		if(Tmp_win->hilite_wl) {
			XDeleteContext(dpy, Tmp_win->hilite_wl, TwmContext);
			XDeleteContext(dpy, Tmp_win->hilite_wl, ScreenContext);
		}
		if(Tmp_win->hilite_wr) {
			XDeleteContext(dpy, Tmp_win->hilite_wr, TwmContext);
			XDeleteContext(dpy, Tmp_win->hilite_wr, ScreenContext);
		}
		if(Tmp_win->lolite_wr) {
			XDeleteContext(dpy, Tmp_win->lolite_wr, TwmContext);
			XDeleteContext(dpy, Tmp_win->lolite_wr, ScreenContext);
		}
		if(Tmp_win->lolite_wl) {
			XDeleteContext(dpy, Tmp_win->lolite_wl, TwmContext);
			XDeleteContext(dpy, Tmp_win->lolite_wl, ScreenContext);
		}
		if(Tmp_win->titlebuttons) {
			int i;

			for(i = 0; i < nb; i++) {
				XDeleteContext(dpy, Tmp_win->titlebuttons[i].window,
				               TwmContext);
				XDeleteContext(dpy, Tmp_win->titlebuttons[i].window,
				               ScreenContext);
			}
		}
		/*
		 * The hilite_wl etc windows don't need to be XDestroyWindow()ed
		 * since that will happen when the parent is destroyed (??)
		 */
	}

	if(Scr->cmapInfo.cmaps == &Tmp_win->cmaps) {
		InstallColormaps(DestroyNotify, &Scr->RootColormaps);
	}

	/*
	 * TwmWindows contain the following pointers
	 *
	 *     1.  (obsolete)
	 *     2.  name
	 *     3.  icon_name
	 *     4.  wmhints
	 *     5.  class.res_name
	 *     6.  class.res_class
	 *     7.  list
	 *     8.  iconmgrp
	 *     9.  cwins
	 *     10. titlebuttons
	 *     11. window ring
	 *     12. squeeze_info (delete if squeeze_info_copied)
	 *     13. HiliteImage
	 *     14. iconslist
	 */
	WMapRemoveWindow(Tmp_win);
	if(Tmp_win->gray) {
		XFreePixmap(dpy, Tmp_win->gray);
	}

	/*
	 * According to the manual page, the following destroys all child windows
	 * of the frame too, which is most of the windows we're concerned with, so
	 * anything related to them must be done before here.
	 * Icons are not child windows.
	 */
	XDestroyWindow(dpy, Tmp_win->frame);
	DeleteIconsList(Tmp_win);                                   /* 14 */
	if(Tmp_win->icon) {
		Icon *icon = Tmp_win->icon;
		if(icon->w && !icon->w_not_ours) {
			IconDown(Tmp_win);
		}
		DeleteIcon(icon);
		Tmp_win->icon = NULL;
	}
	Tmp_win->occupation = 0;
	RemoveIconManager(Tmp_win);                                 /* 7 */
	if(Scr->FirstWindow == Tmp_win) {
		Scr->FirstWindow = Tmp_win->next;
	}
	if(Tmp_win->prev != NULL) {
		Tmp_win->prev->next = Tmp_win->next;
	}
	if(Tmp_win->next != NULL) {
		Tmp_win->next->prev = Tmp_win->prev;
	}
	if(Tmp_win->auto_raise) {
		Scr->NumAutoRaises--;
	}
	if(Tmp_win->auto_lower) {
		Scr->NumAutoLowers--;
	}

	FreeWMPropertyString(Tmp_win->names.ctwm_wm_name); // 2
	FreeWMPropertyString(Tmp_win->names.wm_name);      // 2
	FreeWMPropertyString(Tmp_win->names.ctwm_wm_icon_name); // 3
	FreeWMPropertyString(Tmp_win->names.wm_icon_name); // 3
#ifdef EWMH
	FreeWMPropertyString(Tmp_win->names.net_wm_name);      // 2
	FreeWMPropertyString(Tmp_win->names.net_wm_icon_name); // 3
#endif

	XFree(Tmp_win->wmhints);                                    /* 4 */
	if(Tmp_win->class.res_name && Tmp_win->class.res_name != NoName) { /* 5 */
		XFree(Tmp_win->class.res_name);
	}
	if(Tmp_win->class.res_class && Tmp_win->class.res_class != NoName) { /* 6 */
		XFree(Tmp_win->class.res_class);
	}
	free_cwins(Tmp_win);                                        /* 9 */
	if(Tmp_win->titlebuttons) {                                 /* 10 */
		free(Tmp_win->titlebuttons);
		Tmp_win->titlebuttons = NULL;
	}

	remove_window_from_ring(Tmp_win);                           /* 11 */
	if(Tmp_win->squeeze_info_copied) {                          /* 12 */
		free(Tmp_win->squeeze_info);
		Tmp_win->squeeze_info = NULL;
	}
	DeleteHighlightWindows(Tmp_win);                            /* 13 */

	free(Tmp_win);
	Tmp_win = NULL;

	if(Scr->ClickToFocus || Scr->SloppyFocus) {
		set_last_window(Scr->currentvs->wsw->currentwspc);
	}
}


void
HandleCreateNotify(void)
{
#ifdef DEBUG_EVENTS
	fprintf(stderr, "CreateNotify w = 0x%x\n",
	        (unsigned)Event.xcreatewindow.window);
	fflush(stderr);
	XBell(dpy, 0);
	XSync(dpy, 0);
#endif
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleMapRequest - MapRequest event handler
 *
 ***********************************************************************
 */

void HandleMapRequest(void)
{
	int zoom_save;

	Event.xany.window = Event.xmaprequest.window;
	Tmp_win = GetTwmWindow(Event.xany.window);

	/* If the window has never been mapped before ... */
	if(Tmp_win == NULL) {
		/* Add decorations. */
		VirtualScreen *vs = Scr->currentvs;

		Tmp_win = AddWindow(Event.xany.window,
		                    AWT_NORMAL,
		                    NULL,
		                    vs);
		if(Tmp_win == NULL) {
			return;
		}
#ifdef EWMH
		/* add the new window to the EWMH client list */
		EwmhAddClientWindow(Tmp_win);
		EwmhSet_NET_CLIENT_LIST_STACKING();

		/* Tell it whatever we think of it */
		EwmhSet_NET_WM_STATE(Tmp_win, EWMH_STATE_ALL);
#endif /* EWMH */
	}
	else {
		/*
		 * If the window has been unmapped by the client, it won't be listed
		 * in the icon manager.  Add it again, if requested.
		 */
		if(Tmp_win->iconmanagerlist == NULL) {
			AddIconManager(Tmp_win);
		}
	}

	if(Tmp_win->isiconmgr) {
		return;
	}
	if(Tmp_win->squeezed) {
		return;
	}

	if(Scr->WindowMask) {
		XRaiseWindow(dpy, Scr->WindowMask);
	}

	/* If it's not merely iconified, and we have hints, use them. */
	if(! Tmp_win->isicon) {
		int state;
		Window icon;

		state = NormalState;
		/* use WM_STATE if enabled */
		if(!(RestartPreviousState && GetWMState(Tmp_win->w, &state, &icon) &&
		                (state == NormalState || state == IconicState || state == InactiveState))) {
			if(Tmp_win->wmhints->flags & StateHint) {
				state = Tmp_win->wmhints->initial_state;
			}
		}
		switch(state) {
			case DontCareState:
			case NormalState:
			case ZoomState:
				if(Tmp_win->StartSqueezed) {
					Squeeze(Tmp_win);
				}
				else {
					XMapWindow(dpy, Tmp_win->w);
				}
				XMapWindow(dpy, Tmp_win->frame);
				SetMapStateProp(Tmp_win, NormalState);
				SetRaiseWindow(Tmp_win);
				Tmp_win->mapped = true;
				if(Scr->ClickToFocus && Tmp_win->wmhints->input) {
					SetFocus(Tmp_win, CurrentTime);
				}
				/* kai */
				if(Scr->AutoFocusToTransients &&
				                Tmp_win->istransient &&
				                Tmp_win->wmhints->input) {
					SetFocus(Tmp_win, CurrentTime);
				}
				break;

			case InactiveState:
				if(!OCCUPY(Tmp_win, Scr->currentvs->wsw->currentwspc) &&
				                HandlingEvents && /* to avoid warping during startup */
				                LookInList(Scr->WarpOnDeIconify, Tmp_win->name, &Tmp_win->class)) {
					if(!Scr->NoRaiseDeicon) {
						OtpRaise(Tmp_win, WinWin);
					}
					AddToWorkSpace(Scr->currentvs->wsw->currentwspc->name, Tmp_win);
				}
				Tmp_win->mapped = true;
				if(Tmp_win->UnmapByMovingFarAway) {
					XMoveWindow(dpy, Tmp_win->frame, Scr->rootw + 1, Scr->rooth + 1);
					XMapWindow(dpy, Tmp_win->w);
					XMapWindow(dpy, Tmp_win->frame);
				}
				if(Tmp_win->StartSqueezed) {
					Squeeze(Tmp_win);
				}
				break;

			case IconicState:
				zoom_save = Scr->DoZoom;
				Scr->DoZoom = false;
				Iconify(Tmp_win, -100, -100);
				Scr->DoZoom = zoom_save;
				break;
		}
	}
	/* If no hints, or currently an icon, just "deiconify" */
	else {
		if(!OCCUPY(Tmp_win, Scr->currentvs->wsw->currentwspc) &&
		                LookInList(Scr->WarpOnDeIconify, Tmp_win->name, &Tmp_win->class)) {
			AddToWorkSpace(Scr->currentvs->wsw->currentwspc->name, Tmp_win);
		}
		if(1/*OCCUPY (Tmp_win, Scr->workSpaceMgr.activeWSPC)*/) {
			if(Tmp_win->StartSqueezed) {
				Squeeze(Tmp_win);
			}
			DeIconify(Tmp_win);
			SetRaiseWindow(Tmp_win);
		}
		else {
			Tmp_win->mapped = true;
		}
	}
	if(Tmp_win->mapped) {
		WMapMapWindow(Tmp_win);
	}
	MaybeAnimate = true;
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleMapNotify - MapNotify event handler
 *
 ***********************************************************************
 */

void HandleMapNotify(void)
{
	if(Tmp_win == NULL) {
		return;
	}

	/*
	 * Need to do the grab to avoid race condition of having server send
	 * MapNotify to client before the frame gets mapped; this is bad because
	 * the client would think that the window has a chance of being viewable
	 * when it really isn't.
	 */
	XGrabServer(dpy);

	// Mapping the window, hide its icon
	if(Tmp_win->icon && Tmp_win->icon->w) {
		XUnmapWindow(dpy, Tmp_win->icon->w);
	}

	// Map up everything inside the frame (not the frame itself, so if it
	// wasn't already up, nothing will show yet)
	XMapSubwindows(dpy, Tmp_win->frame);

	// Choose which of the hi/lolite's should be left up, based on the
	// focus
	if(Scr->Focus != Tmp_win) {
		if(Tmp_win->hilite_wl) {
			XUnmapWindow(dpy, Tmp_win->hilite_wl);
		}
		if(Tmp_win->hilite_wr) {
			XUnmapWindow(dpy, Tmp_win->hilite_wr);
		}
	}
	else {
		if(Tmp_win->lolite_wl) {
			XUnmapWindow(dpy, Tmp_win->lolite_wl);
		}
		if(Tmp_win->lolite_wr) {
			XUnmapWindow(dpy, Tmp_win->lolite_wr);
		}
	}

	// Now map the frame itself (which brings all the rest into view)
	XMapWindow(dpy, Tmp_win->frame);

	XUngrabServer(dpy);
	XFlush(dpy);

	// Set the flags
	Tmp_win->mapped = true;
	Tmp_win->isicon = false;
	Tmp_win->icon_on = false;
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleUnmapNotify - UnmapNotify event handler
 *
 ***********************************************************************
 */

void HandleUnmapNotify(void)
{
	int dstx, dsty;
	Window dumwin;

	/*
	 * The July 27, 1988 ICCCM spec states that a client wishing to switch
	 * to WithdrawnState should send a synthetic UnmapNotify with the
	 * event field set to (pseudo-)root, in case the window is already
	 * unmapped (which is the case for twm for IconicState).  Unfortunately,
	 * we looked for the TwmContext using that field, so try the window
	 * field also.
	 */
	if(Tmp_win == NULL) {
		Event.xany.window = Event.xunmap.window;
		Tmp_win = GetTwmWindow(Event.xany.window);
	}

	if(Tmp_win == NULL || Event.xunmap.window == Tmp_win->frame ||
	                (Tmp_win->icon && Event.xunmap.window == Tmp_win->icon->w) ||
	                (!Tmp_win->mapped && !Tmp_win->isicon)) {
		return;
	}
	/*
	    if (Tmp_win == NULL || (!Tmp_win->mapped && !Tmp_win->isicon))
	        return;
	*/
	/*
	 * The program may have unmapped the client window, from either
	 * NormalState or IconicState.  Handle the transition to WithdrawnState.
	 *
	 * We need to reparent the window back to the root (so that twm exiting
	 * won't cause it to get mapped) and then throw away all state (pretend
	 * that we've received a DestroyNotify).
	 */
	/* Is it the correct behaviour ???
	    XDeleteProperty (dpy, Tmp_win->w, XA_WM_OCCUPATION);
	*/
#ifdef EWMH
	EwmhUnmapNotify(Tmp_win);
#endif /* EWMH */
	XGrabServer(dpy);
	if(XTranslateCoordinates(dpy, Event.xunmap.window, Tmp_win->attr.root,
	                         0, 0, &dstx, &dsty, &dumwin)) {
		XEvent ev;
		Bool reparented = XCheckTypedWindowEvent(dpy, Event.xunmap.window,
		                  ReparentNotify, &ev);
		SetMapStateProp(Tmp_win, WithdrawnState);
		if(reparented) {
			// It got reparented, get rid of our alterations.
			if(Tmp_win->old_bw) {
				XSetWindowBorderWidth(dpy,
				                      Event.xunmap.window,
				                      Tmp_win->old_bw);
			}
			if(Tmp_win->wmhints->flags & IconWindowHint) {
				XUnmapWindow(dpy, Tmp_win->wmhints->icon_window);
			}
		}
		else {
			// Didn't get reparented, so we should do it ourselves
			XReparentWindow(dpy, Event.xunmap.window, Tmp_win->attr.root,
			                dstx, dsty);
			// XXX Need to think more about just what the roots and
			// coords are here...
			RestoreWinConfig(Tmp_win);
		}
		XRemoveFromSaveSet(dpy, Event.xunmap.window);
		XSelectInput(dpy, Event.xunmap.window, NoEventMask);
		HandleDestroyNotify();          /* do not need to mash event before */
	} /* else window no longer exists and we'll get a destroy notify */
	XUngrabServer(dpy);
	XFlush(dpy);
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleMotionNotify - MotionNotify event handler
 *
 ***********************************************************************
 */

void HandleMotionNotify(void)
{
	if(ResizeWindow != (Window) 0) {
		XQueryPointer(dpy, Event.xany.window,
		              &(Event.xmotion.root), &JunkChild,
		              &(Event.xmotion.x_root), &(Event.xmotion.y_root),
		              &(Event.xmotion.x), &(Event.xmotion.y),
		              &JunkMask);

		FixRootEvent(&Event);
		/* Set WindowMoved appropriately so that f.deltastop will
		   work with resize as well as move. */
		if(abs(Event.xmotion.x - ResizeOrigX) >= Scr->MoveDelta
		                || abs(Event.xmotion.y - ResizeOrigY) >= Scr->MoveDelta) {
			WindowMoved = true;
		}

		Tmp_win = GetTwmWindow(ResizeWindow);
#ifdef WINBOX
		if(Tmp_win && Tmp_win->winbox) {
			XTranslateCoordinates(dpy, Scr->Root, Tmp_win->winbox->window,
			                      Event.xmotion.x_root, Event.xmotion.y_root,
			                      &(Event.xmotion.x_root), &(Event.xmotion.y_root), &JunkChild);
		}
#endif
		DoResize(Event.xmotion.x_root, Event.xmotion.y_root, Tmp_win);
	}
	else if(Scr->BorderCursors && Tmp_win && Event.xany.window == Tmp_win->frame) {
		SetBorderCursor(Tmp_win, Event.xmotion.x, Event.xmotion.y);
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleButtonRelease - ButtonRelease event handler
 *
 ***********************************************************************
 */
void HandleButtonRelease(void)
{
	int xl, yt, w, h;
	unsigned mask;

	if(Scr->InfoWindow.mapped) {  /* delete info box on 2nd button release */
		if(Context == C_IDENTIFY) {
			XUnmapWindow(dpy, Scr->InfoWindow.win);
			Scr->InfoWindow.mapped = false;
			Context = C_NO_CONTEXT;
		}
	}

	if(DragWindow != None) {
		MoveOutline(Scr->XineramaRoot, 0, 0, 0, 0, 0, 0);

		Tmp_win = GetTwmWindow(DragWindow);
#ifdef WINBOX
		if(Tmp_win->winbox) {
			XTranslateCoordinates(dpy, Scr->Root, Tmp_win->winbox->window,
			                      Event.xbutton.x_root, Event.xbutton.y_root,
			                      &(Event.xbutton.x_root), &(Event.xbutton.y_root), &JunkChild);
		}
#endif
		if(DragWindow == Tmp_win->frame) {
			xl = Event.xbutton.x_root - DragX - Tmp_win->frame_bw;
			yt = Event.xbutton.y_root - DragY - Tmp_win->frame_bw;
			w = DragWidth + 2 * Tmp_win->frame_bw;
			h = DragHeight + 2 * Tmp_win->frame_bw;
		}
		else {
			xl = Event.xbutton.x_root - DragX - DragBW;
			yt = Event.xbutton.y_root - DragY - DragBW;
			w = DragWidth + 2 * DragBW;
			h = DragHeight + 2 * DragBW;
		}

		if(ConstMove) {
			if(ConstMoveDir == MOVE_HORIZ) {
				yt = ConstMoveY;
			}

			if(ConstMoveDir == MOVE_VERT) {
				xl = ConstMoveX;
			}

			if(ConstMoveDir == MOVE_NONE) {
				yt = ConstMoveY;
				xl = ConstMoveX;
			}
		}

		if(Scr->DontMoveOff && MoveFunction != F_FORCEMOVE) {
			TryToGrid(Tmp_win, &xl, &yt);
		}
		if(MoveFunction == F_MOVEPUSH &&
		                Scr->OpaqueMove &&
		                DragWindow == Tmp_win->frame) {
			TryToPush(Tmp_win, xl, yt);
		}
		if(MoveFunction == F_MOVEPACK ||
		                (MoveFunction == F_MOVEPUSH &&
		                 DragWindow == Tmp_win->frame)) {
			TryToPack(Tmp_win, &xl, &yt);
		}
		if(Scr->DontMoveOff && MoveFunction != F_FORCEMOVE) {
			ConstrainByBorders(Tmp_win, &xl, w, &yt, h);
		}

		CurrentDragX = xl;
		CurrentDragY = yt;
#ifdef VSCREEN
		/*
		 * sometimes getScreenOf() replies with the wrong window when moving
		 * y to a negative number.  Need to figure out why... [XXX]
		 * It seems to be because the first XTranslateCoordinates() doesn't
		 * translate the coordinates to be relative to XineramaRoot.
		 * That in turn is probably because a window is visible in a ws or
		 * vs where it shouldn't (inconsistent with its occupation).
		 * A problem of this kind was fixed with f.adoptwindow but remains
		 * with f.hypermove.
		 * As a result, the window remains in the original vs/ws and
		 * is irretrievably moved out of view. [Rhialto]
		 */
		if(xl < 0 || yt < 0 || xl > Scr->rootw || yt > Scr->rooth) {
			int odestx, odesty;
			int destx, desty;
			Window cr;
			VirtualScreen *newvs;

			XTranslateCoordinates(dpy, Tmp_win->vs->window,
			                      Scr->XineramaRoot, xl, yt, &odestx, &odesty, &cr);

			newvs = findIfVScreenOf(odestx, odesty);

			if(newvs && newvs->wsw && newvs->wsw->currentwspc) {
				XTranslateCoordinates(dpy, Scr->XineramaRoot,
				                      newvs->window, odestx, odesty,
				                      &destx, &desty, &cr);
				AddToWorkSpace(newvs->wsw->currentwspc->name, Tmp_win);
				RemoveFromWorkSpace(Tmp_win->vs->wsw->currentwspc->name, Tmp_win);
				xl = destx;
				yt = desty;
			}
		}
#endif
		if(DragWindow == Tmp_win->frame) {
			SetupWindow(Tmp_win, xl, yt,
			            Tmp_win->frame_width, Tmp_win->frame_height, -1);
		}
		else {
			XMoveWindow(dpy, DragWindow, xl, yt);
			if(DragWindow == Tmp_win->icon->w) {
				Tmp_win->icon->w_x = xl;
				Tmp_win->icon->w_y = yt;
			}
		}

		if(!Scr->NoRaiseMove) {  /* && !Scr->OpaqueMove)    opaque already did */
			if(DragWindow == Tmp_win->frame) {
				OtpRaise(Tmp_win, WinWin);
			}
			else if(Tmp_win->icon && DragWindow == Tmp_win->icon->w) {
				OtpRaise(Tmp_win, IconWin);
			}
			else {
				fprintf(stderr, "ERROR -- events.c:2815\n");
			}
		}

		if(!Scr->OpaqueMove) {
			UninstallRootColormap();
		}
		else {
			XSync(dpy, 0);
		}

		if(Scr->NumAutoRaises) {
			enter_flag = true;
			enter_win = NULL;
			raise_win = ((DragWindow == Tmp_win->frame && !Scr->NoRaiseMove)
			             ? Tmp_win : NULL);
		}

		/* CCC equivalent code for auto lower not needed? */

#if 0
		if(Scr->NumAutoLowers) {
			leave_flag = true;
			leave_win = NULL;
			lower_win = ((DragWindow == Tmp_win->frame)
			             ? Tmp_win : NULL);
		}
#endif

		DragWindow = (Window) 0;
		ConstMove = false;
	}

	if(ResizeWindow != (Window) 0) {
		EndResize();
	}

	if(ActiveMenu != NULL && RootFunction == 0) {
		if(ActiveItem) {
			int func = ActiveItem->func;
			Action = ActiveItem->action;
			switch(func) {
				case F_TITLE:
					if(Scr->StayUpMenus)   {
						ButtonPressed = -1;
						if(Scr->WarpToDefaultMenuEntry && ActiveMenu->defaultitem) {
							WarpCursorToDefaultEntry(ActiveMenu);
						}
						return;
					}
					break;
				case F_MOVE:
				case F_FORCEMOVE:
				case F_DESTROY:
				case F_DELETE:
				case F_DELETEORDESTROY:
					ButtonPressed = -1;
					break;
				case F_CIRCLEUP:
				case F_CIRCLEDOWN:
				case F_REFRESH:
				case F_WARPTOSCREEN:
					PopDownMenu();
					break;
				default:
					break;
			}
			if(func != F_PIN && func != F_MENU) {
				PopDownMenu();
			}
			ExecuteFunction(func, Action,
			                ButtonWindow ? ButtonWindow->frame : None,
			                ButtonWindow, &Event, Context, true);
			Context = C_NO_CONTEXT;
			ButtonWindow = NULL;

			/* if we are not executing a defered command, then take down the
			 * menu
			 */
			if(ActiveMenu) {
				PopDownMenu();
			}
		}
		else if(Scr->StayUpMenus && !ActiveMenu->entered) {
			ButtonPressed = -1;
			if(Scr->WarpToDefaultMenuEntry && ActiveMenu->defaultitem) {
				WarpCursorToDefaultEntry(ActiveMenu);
			}
			return;
		}
		else {
			PopDownMenu();
		}
	}

	mask = (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask);
	switch(Event.xbutton.button) {
		case Button1:
			mask &= ~Button1Mask;
			break;
		case Button2:
			mask &= ~Button2Mask;
			break;
		case Button3:
			mask &= ~Button3Mask;
			break;
		case Button4:
			mask &= ~Button4Mask;
			break;
		case Button5:
			mask &= ~Button5Mask;
			break;
	}

	if(RootFunction != 0 ||
	                ResizeWindow != None ||
	                DragWindow != None) {
		ButtonPressed = -1;
	}

	if(AlternateKeymap || AlternateContext) {
		ButtonPressed = -1;
		return;
	}

	if(RootFunction == 0 &&
	                (Event.xbutton.state & mask) == 0 &&
	                DragWindow == None &&
	                ResizeWindow == None) {
		XUngrabPointer(dpy, CurrentTime);
		XUngrabServer(dpy);
		XFlush(dpy);
		EventHandler[EnterNotify] = HandleEnterNotify;
		EventHandler[LeaveNotify] = HandleLeaveNotify;
		ButtonPressed = -1;
		if(DownIconManager) {
			DownIconManager->down = false;
			if(Scr->Highlight) {
				DrawIconManagerBorder(DownIconManager, false);
			}
			DownIconManager = NULL;
		}
		Cancel = false;
	}
}


/*
 * Pop up a submenu as a result of moving the mouse right on its entry.
 */
static void do_menu(MenuRoot *menu,     /* menu to pop up */
                    Window w)          /* invoking window or None */
{
	int x = Event.xbutton.x_root;
	int y = Event.xbutton.y_root;
	bool center;

	if(!Scr->NoGrabServer) {
		XGrabServer(dpy);
	}
	if(w) {
		int h = Scr->TBInfo.width - Scr->TBInfo.border;
		Window child;

		XTranslateCoordinates(dpy, w, Scr->Root, 0, h, &x, &y, &child);
		center = false;
	}
	else {
		center = true;
	}
	if(PopUpMenu(menu, x, y, center)) {
		UpdateMenu();
	}
	else {
		XBell(dpy, 0);
	}
}


/*
 * Pop up a submenu as a result of hitting the Right arrow key while on
 * its entry.  We should try folding these two together a bit more.
 */
static void do_key_menu(MenuRoot *menu,         /* menu to pop up */
                        Window w)              /* invoking window or None */
{
	int x = Event.xkey.x_root;
	int y = Event.xkey.y_root;
	bool center;

	/* I don't think this is necessary.
	    if (!Scr->NoGrabServer) XGrabServer(dpy);
	*/
	if(w) {
		int h = Scr->TBInfo.width - Scr->TBInfo.border;
		Window child;

		XTranslateCoordinates(dpy, w, Scr->Root, 0, h, &x, &y, &child);
		center = false;
	}
	else {
		center = true;
	}
	if(PopUpMenu(menu, x, y, center)) {
		/*
		 * Note: UpdateMenu() has the internal re-capture of the event
		 * loop to handle in-menu stuff, so this won't actually return
		 * until we somehow exit out of that [sub]menu.
		 */
		UpdateMenu();
	}
	else {
		XBell(dpy, 0);
	}

}


/***********************************************************************
 *
 *  Procedure:
 *      HandleButtonPress - ButtonPress event handler
 *
 ***********************************************************************
 */
void HandleButtonPress(void)
{
	unsigned int modifier;
	Cursor cur;
	MenuRoot *mr;
	FuncButton *tmp = 0;
	int func = 0;
	Window w;


	/* pop down the menu, if any */

	if(XFindContext(dpy, Event.xbutton.window, MenuContext,
	                (XPointer *) &mr) != XCSUCCESS) {
		mr = NULL;
	}
	if(ActiveMenu && (! ActiveMenu->pinned) &&
	                (Event.xbutton.subwindow != ActiveMenu->w)) {
		PopDownMenu();
		return;
	}
	if((ActiveMenu != NULL) && (RootFunction != 0) && (mr != ActiveMenu)) {
		PopDownMenu();
	}

	XSync(dpy, 0);
	/* XXX - remove? */

	/* want menus if we have info box */
	if(ButtonPressed != -1 && !Scr->InfoWindow.mapped) {
		/* we got another butt press in addition to one still held
		 * down, we need to cancel the operation we were doing
		 */
		Cancel = true;
		CurrentDragX = origDragX;
		CurrentDragY = origDragY;
		if(!menuFromFrameOrWindowOrTitlebar) {
			if(Scr->OpaqueMove && DragWindow != None) {
				XMoveWindow(dpy, DragWindow, origDragX, origDragY);
			}
			else {
				MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
			}
		}
		XUnmapWindow(dpy, Scr->SizeWindow);
		if(!Scr->OpaqueMove) {
			UninstallRootColormap();
		}
		ResizeWindow = None;
		DragWindow = None;
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

		return;
	}
	else {
		ButtonPressed = Event.xbutton.button;
	}

	if((ActiveMenu != NULL) && (ActiveMenu->pinned)) {
		if(Event.xbutton.window == ActiveMenu->w) {
			modifier = (Event.xbutton.state & mods_used);
			modifier = set_mask_ignore(modifier);
			if((ActiveItem && (ActiveItem->func == F_TITLE)) || (modifier == Mod1Mask)) {
				MoveMenu(&Event);
				/*ButtonPressed = -1;*/
			}
		}
		Context = C_ROOT;
		return;
	}

	if(ResizeWindow != None ||
	                DragWindow != None  ||
	                ActiveMenu != NULL) {
		return;
	}

	/* check the title bar buttons */
	if(Tmp_win && Tmp_win->title_height && Tmp_win->titlebuttons) {
		int i;
		TBWindow *tbw;
		TitleButtonFunc *tbf;
		int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

		modifier = Event.xbutton.state & mods_used;
		modifier = set_mask_ignore(modifier);

		for(i = 0, tbw = Tmp_win->titlebuttons; i < nb; i++, tbw++) {
			if(Event.xany.window == tbw->window) {
				for(tbf = tbw->info->funs; tbf; tbf = tbf->next) {
					if(tbf->num == ButtonPressed
					                && tbf->mods == modifier) {
						switch(tbf->func) {
							/*
							 * Opening up a menu doesn't use the f.menu
							 * handler, we use our do_menu(); x-ref
							 * comments in the handler for details.
							 */
							case F_MENU :
								Context = C_TITLE;
								ButtonWindow = Tmp_win;
								do_menu(tbf->menuroot, tbw->window);
								break;

							default :
								ExecuteFunction(tbf->func, tbf->action,
								                Event.xany.window, Tmp_win,
								                &Event, C_TITLE, false);
						}
						return;
					}
				}
			}
		}
	}

	Context = C_NO_CONTEXT;

	if(Event.xany.window == Scr->InfoWindow.win) {
		Context = C_IDENTIFY;
	}

	if(Event.xany.window == Scr->Root) {
		if(AlternateContext) {
			XUngrabPointer(dpy, CurrentTime);
			XUngrabKeyboard(dpy, CurrentTime);
			AlternateContext = false;
			Context = C_ALTERNATE;
		}
		else if(AlternateKeymap && Event.xbutton.subwindow) {
			int dx, dy;
			Window child;

			w = Event.xbutton.subwindow;
			Tmp_win = GetTwmWindow(w);
			if(Tmp_win) {
				Event.xany.window    = Tmp_win->frame;
				XTranslateCoordinates(dpy, Scr->Root, Tmp_win->frame,
				                      Event.xbutton.x, Event.xbutton.y, &dx, &dy, &child);
				Event.xbutton.x = dx;
				Event.xbutton.x = dy;
				Event.xbutton.subwindow = child;
			}
		}
		else {
			Context = C_ROOT;
		}
	}
	if(Tmp_win) {
		if(Tmp_win->iconmanagerlist && (RootFunction != 0) &&
		                ((Event.xany.window == Tmp_win->iconmanagerlist->icon) ||
		                 (Event.xany.window == Tmp_win->iconmanagerlist->w))) {
			int x, y;

			Tmp_win = Tmp_win->iconmanagerlist->iconmgr->twm_win;
			XTranslateCoordinates(dpy, Event.xany.window, Tmp_win->w,
			                      Event.xbutton.x, Event.xbutton.y,
			                      &x, &y, &JunkChild);

			Event.xbutton.x = x - Tmp_win->frame_bw3D;
			Event.xbutton.y = y - Tmp_win->title_height - Tmp_win->frame_bw3D;
			Event.xany.window = Tmp_win->w;
			Context = C_WINDOW;
		}
#ifdef EWMH_DESKTOP_ROOT
		else if(Tmp_win->ewmhWindowType == wt_Desktop) {
			fprintf(stderr, "HandleButtonPress: wt_Desktop -> C_ROOT\n");
			Context = C_ROOT;
		}
#endif
		else if(Event.xany.window == Tmp_win->title_w) {
			if(Scr->ClickToFocus && Tmp_win->wmhints->input) {
				SetFocus(Tmp_win, CurrentTime);
			}
			Context = C_TITLE;
		}
		else if(Event.xany.window == Tmp_win->w) {
			if(Scr->ClickToFocus || Scr->RaiseOnClick) {
				if(Scr->ClickToFocus && Tmp_win->wmhints->input) {
					SetFocus(Tmp_win, CurrentTime);
				}
				if(Scr->RaiseOnClick) {
					OtpRaise(Tmp_win, WinWin);
					WMapRaise(Tmp_win);
				}
				XSync(dpy, 0);
				XAllowEvents(dpy, ReplayPointer, CurrentTime);
				XSync(dpy, 0);
				ButtonPressed = -1;
				return;
			}
			else {
				printf("ERROR! ERROR! ERROR! YOU SHOULD NOT BE HERE!!!\n");
				Context = C_WINDOW;
			}
		}
		else if(Tmp_win->icon && (Event.xany.window == Tmp_win->icon->w)) {
			Context = C_ICON;
		}
		else if(Event.xany.window == Tmp_win->frame) {
			Window chwin;

			/* since we now place a button grab on the frame instead
			 * of the window, (see GrabButtons() in add_window.c), we
			 * need to figure out where the pointer exactly is before
			 * assigning Context.  If the pointer is on the application
			 * window we will change the event structure to look as if
			 * it came from the application window.
			 */
			if(Event.xbutton.subwindow == Tmp_win->w) {
				XTranslateCoordinates(dpy, Event.xany.window, Tmp_win->w,
				                      Event.xbutton.x, Event.xbutton.y,
				                      &Event.xbutton.x, &Event.xbutton.y,
				                      &chwin);
				Event.xbutton.window = Tmp_win->w;

#ifdef WINBOX
				if(Tmp_win->iswinbox && chwin) {
					int x, y;
					TwmWindow *wtmp;
					XTranslateCoordinates(dpy, Tmp_win->w, chwin,
					                      Event.xbutton.x, Event.xbutton.y,
					                      &x, &y, &chwin);
					if(chwin && (wtmp = GetTwmWindow(chwin))) {
						Event.xany.window = chwin;
						Event.xbutton.x   = x;
						Event.xbutton.y   = y;
						Tmp_win = wtmp;
					}
				}
#endif
				Context = C_WINDOW;
			}
			else if(Event.xbutton.subwindow
			                && (Event.xbutton.subwindow == Tmp_win->title_w)) {
				Context = C_TITLE;
			}
			else {
				Context = C_FRAME;
			}

			if(Scr->ClickToFocus && Tmp_win->wmhints->input) {
				SetFocus(Tmp_win, CurrentTime);
			}
		}
		else if(Tmp_win->iswspmgr ||
		                (Tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win)) {
			/*Context = C_WINDOW; probably a typo */
			Context = C_WORKSPACE;
		}
		else if(Tmp_win->iconmanagerlist) {
			if((Event.xany.window == Tmp_win->iconmanagerlist->icon) ||
			                (Event.xany.window == Tmp_win->iconmanagerlist->w)) {
				Tmp_win->iconmanagerlist->down = true;
				if(Scr->Highlight) {
					DrawIconManagerBorder(Tmp_win->iconmanagerlist, false);
				}
				DownIconManager = Tmp_win->iconmanagerlist;
				Context = C_ICONMGR;
			}
		}
	}

	/* this section of code checks to see if we were in the middle of
	 * a command executed from a menu
	 */
	if(RootFunction != 0) {
		if(Event.xany.window == Scr->Root) {
			int x, y;

			/* if the window was the Root, we don't know for sure it
			 * it was the root.  We must check to see if it happened to be
			 * inside of a client that was getting button press events.
			 */
			XTranslateCoordinates(dpy, Scr->Root, Scr->Root,
			                      Event.xbutton.x,
			                      Event.xbutton.y,
			                      &x, &y, &Event.xany.window);

			if(Event.xany.window != 0 &&
			                (Tmp_win = GetTwmWindow(Event.xany.window))) {
#ifdef WINBOX
				if(Tmp_win->iswinbox) {
					Window win;
					XTranslateCoordinates(dpy, Scr->Root, Event.xany.window,
					                      x, y, &x, &y, &win);
					XTranslateCoordinates(dpy, Event.xany.window, win,
					                      x, y, &x, &y, &win);
					if(win != 0) {
						Event.xany.window = win;
					}
				}
#endif
			}
			if(Event.xany.window == 0 ||
			                !(Tmp_win = GetTwmWindow(Event.xany.window))) {
				RootFunction = 0;
				XBell(dpy, 0);
				return;
			}
			XTranslateCoordinates(dpy, Scr->Root, Event.xany.window,
			                      Event.xbutton.x,
			                      Event.xbutton.y,
			                      &x, &y, &JunkChild);

			Event.xbutton.x = x;
			Event.xbutton.y = y;
			Context = C_WINDOW;
		}
		else if(mr != NULL) {
			RootFunction = 0;
			XBell(dpy, 0);
			return;
		}

		/* make sure we are not trying to move an identify window */
		if(Event.xany.window != Scr->InfoWindow.win) {
			/*
			 * X-ref comment at top of file about Action; this is where
			 * we need to use its broader lifespan.
			 */
			ExecuteFunction(RootFunction, Action, Event.xany.window,
			                Tmp_win, &Event, Context, false);
		}

		RootFunction = 0;
		return;
	}

	ButtonWindow = Tmp_win;

	/* if we get to here, we have to execute a function or pop up a
	 * menu
	 */
	modifier = (Event.xbutton.state | AlternateKeymap) & mods_used;
	modifier = set_mask_ignore(modifier);
	if(AlternateKeymap) {
		XUngrabPointer(dpy, CurrentTime);
		XUngrabKeyboard(dpy, CurrentTime);
		AlternateKeymap = 0;
	}
	if((Context == C_NO_CONTEXT) || (Context == C_IDENTIFY)) {
		return;
	}

	RootFunction = 0;

	/* see if there already is a key defined for this context */
	for(tmp = Scr->FuncButtonRoot.next; tmp != NULL; tmp = tmp->next) {
		if((tmp->num  == Event.xbutton.button) &&
		                (tmp->cont == Context) && (tmp->mods == modifier)) {
			break;
		}
	}
	if(tmp) {
		func = tmp->func;
		switch(func) {
			/*
			 * f.menu isn't invoked, it's handle magically.  Other funcs
			 * we just invoke.  X-ref the f.menu handler for details.
			 */
			case F_MENU :
				do_menu(tmp->menu, (Window) None);
				break;

			default :
				if(func != 0) {
					Action = tmp->item ? tmp->item->action : NULL;
#ifdef EWMH_DESKTOP_ROOT
					if(Context == C_ROOT && Tmp_win != NULL) {
						Context = C_WINDOW;
						fprintf(stderr, "HandleButtonPress: wt_Desktop -> C_WINDOW\n");
					}
#endif /* EWMH */
					ExecuteFunction(func,
					                Action, Event.xany.window, Tmp_win, &Event, Context, false);
				}
		}
	}
	else {
		if(Tmp_win == Scr->currentvs->wsw->twm_win) {
			WMgrHandleButtonEvent(Scr->currentvs, &Event);
			return;
		}
	}
	if(Tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win) {
		OccupyHandleButtonEvent(&Event);
	}
	else if(func == 0 && Scr->DefaultFunction.func != 0) {
		if(Scr->DefaultFunction.func == F_MENU) {
			do_menu(Scr->DefaultFunction.menu, (Window) None);
		}
		else {
			Action = Scr->DefaultFunction.item ?
			         Scr->DefaultFunction.item->action : NULL;
			ExecuteFunction(Scr->DefaultFunction.func, Action,
			                Event.xany.window, Tmp_win, &Event, Context, false);
		}
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      HENQueueScanner - EnterNotify event q scanner
 *
 *      Looks at the queued events and determines if any matching
 *      LeaveNotify events or EnterEvents deriving from the
 *      termination of a grab are behind this event to allow
 *      skipping of unnecessary processing.
 *
 ***********************************************************************
 */

typedef struct HENScanArgs {
	Window w;           /* Window we are currently entering */
	Bool leaves;        /* Any LeaveNotifies found for this window */
	Bool inferior;      /* Was NotifyInferior the mode for LeaveNotify */
	Bool enters;        /* Any EnterNotify events with NotifyUngrab */
} HENScanArgs;

/* ARGSUSED*/
static Bool HENQueueScanner(Display *display, XEvent *ev, char *_args)
{
	HENScanArgs *args = (void *)_args;

	if(ev->type == LeaveNotify) {
		if(ev->xcrossing.window == args->w &&
		                ev->xcrossing.mode == NotifyNormal) {
			args->leaves = True;
			/*
			 * Only the last event found matters for the Inferior field.
			 */
			args->inferior =
			        (ev->xcrossing.detail == NotifyInferior);
		}
	}
	else if(ev->type == EnterNotify) {
		if(ev->xcrossing.mode == NotifyUngrab) {
			args->enters = True;
		}
	}

	return (False);
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleEnterNotify - EnterNotify event handler
 *
 ***********************************************************************
 */

void HandleEnterNotify(void)
{
	MenuRoot *mr, *tmp;
	XEnterWindowEvent *ewp = &Event.xcrossing;
	HENScanArgs scanArgs;
	XEvent dummy;
	VirtualScreen *vs;

	/*
	 * if we aren't in the middle of menu processing
	 */
	if(!ActiveMenu) {
		/*
		 * We're not interested in pseudo Enter/Leave events generated
		 * from grab initiations.
		 */
		if(ewp->mode == NotifyGrab) {
			return;
		}

		/*
		 * Scan for Leave and Enter Notify events to see if we can avoid some
		 * unnecessary processing.
		 */
		scanArgs.w = ewp->window;
		scanArgs.leaves = scanArgs.enters = False;
		XCheckIfEvent(dpy, &dummy, HENQueueScanner, (void *) &scanArgs);

		/*
		 * if entering root window, restore twm default colormap so that
		 * titlebars are legible
		 */
		if(ewp->window == Scr->Root) {
			Window forus_ret;
			int focus_rev;

			if(!scanArgs.leaves && !scanArgs.enters) {
				InstallColormaps(EnterNotify, &Scr->RootColormaps);
			}
			if(! Scr->FocusRoot) {
				return;
			}
			XGetInputFocus(dpy, &forus_ret, &focus_rev);
			if((forus_ret != PointerRoot) && (forus_ret != None)) {
				SetFocus(NULL, Event.xcrossing.time);
			}
			return;
		}
		for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
			if(ewp->window == vs->window) {
				Scr->Root  = vs->window;
#ifdef CAPTIVE
				Scr->rootx = Scr->crootx + vs->x;
				Scr->rooty = Scr->crooty + vs->y;
#else
				Scr->rootx = vs->x;
				Scr->rooty = vs->y;
#endif
				Scr->rootw = vs->w;
				Scr->rooth = vs->h;
				Scr->currentvs = vs;
#if 0
				fprintf(stderr, "entering new vs : %p, 0x%lx, %d, %d, %d, %d\n",
				        vs, Scr->Root, vs->x, vs->y, vs->w, vs->h);
#endif
				return;
			}
		}

		/* Handle RaiseDelay, if any.....
		 */
		if(RaiseDelay > 0) {
			if(Tmp_win && Tmp_win->auto_raise &&
			                (!Tmp_win->iconmanagerlist ||
			                 Tmp_win->iconmanagerlist->w != ewp->window)) {
				ColormapWindow *cwin;
				static struct timeval tout, timeout = {0, 12500};

				if(XFindContext(dpy, Tmp_win->w, ColormapContext,
				                (XPointer *)&cwin) == XCNOENT) {
					cwin = NULL;
				}

				if((ewp->detail != NotifyInferior
				                || Tmp_win->frame == ewp->window)
				                && (!cwin || cwin->visibility != VisibilityUnobscured)) {
					int x, y, px, py, d, i;
					Window w;

					XQueryPointer(dpy, Scr->Root, &w, &w, &px, &py,
					              &d, &d, (unsigned int *)&d);

					/* The granularity of RaiseDelay is about 25 ms.
					 * The timeout variable is set to 12.5 ms since we
					 * pass this way twice each time a twm window is
					 * entered.
					 */
					for(i = 25; i < RaiseDelay; i += 25) {
						tout = timeout;
						select(0, NULL, NULL, NULL, &tout);
						/* Did we leave this window already? */
						scanArgs.w = ewp->window;
						scanArgs.leaves = scanArgs.enters = False;
						XCheckIfEvent(dpy, &dummy, HENQueueScanner,
						              (void *) &scanArgs);
						if(scanArgs.leaves && !scanArgs.inferior) {
							return;
						}

						XQueryPointer(dpy, Scr->Root, &w, &w, &x, &y,
						              &d, &d, (unsigned int *)&d);

						/* Has the pointer moved?  If so reset the loop cnt.
						 * We want the pointer to be still for RaiseDelay
						 * milliseconds before terminating the loop
						 */
						if(x != px || y != py) {
							i = 0;
							px = x;
							py = y;
						}
					}
				}
			}

			/*
			 * Scan for Leave and Enter Notify events to see if we can avoid some
			 * unnecessary processing.
			 */
			scanArgs.w = ewp->window;
			scanArgs.leaves = scanArgs.enters = False;
			XCheckIfEvent(dpy, &dummy, HENQueueScanner, (void *) &scanArgs);

			/*
			 * if entering root window, restore twm default colormap so that
			 * titlebars are legible
			 */
			if(ewp->window == Scr->Root) {
				if(!scanArgs.leaves && !scanArgs.enters) {
					InstallColormaps(EnterNotify, &Scr->RootColormaps);
				}
				return;
			}
		}
		/* End of RaiseDelay modification. */

		/*
		 * if we have an event for a specific one of our windows
		 */
		if(Tmp_win) {
			/*
			 * If currently in PointerRoot mode (indicated by FocusRoot), then
			 * focus on this window
			 */
			if(Scr->FocusRoot && (!scanArgs.leaves || scanArgs.inferior)) {
				bool accinput;

				if(Scr->ShrinkIconTitles &&
				                Tmp_win->icon &&
				                ewp->window == Tmp_win->icon->w &&
				                ewp->detail != NotifyInferior) {
					if(Scr->AutoRaiseIcons) {
						OtpRaise(Tmp_win, IconWin);
					}
					ExpandIconTitle(Tmp_win);
					return;
				}

				if(Tmp_win->iconmanagerlist) {
					CurrentIconManagerEntry(Tmp_win->iconmanagerlist);
				}

				accinput = Tmp_win->mapped && Tmp_win->wmhints->input;
				if(Tmp_win->iconmanagerlist &&
				                ewp->window == Tmp_win->iconmanagerlist->w &&
				                !accinput &&
				                Tmp_win->iconmanagerlist->iconmgr &&
				                Tmp_win->iconmanagerlist->iconmgr->twm_win) {
					SetFocus(Tmp_win->iconmanagerlist->iconmgr->twm_win,
					         CurrentTime);
					return;
				}

				if(Tmp_win->mapped) {
					/*
					 * unhighlight old focus window
					 */

					/*
					 * If entering the frame or the icon manager, then do
					 * "window activation things":
					 *
					 *     1.  <highlighting is not done here any more>
					 *     2.  install frame colormap
					 *     3.  <frame and highlight border not set here>
					 *     4.  focus on client window to forward typing
					 *     4a. same as 4 but for icon mgr w/with NoTitleFocus
					 *     5.  send WM_TAKE_FOCUS if requested
					 */
					if(Scr->BorderCursors && ewp->window == Tmp_win->frame) {
						SetBorderCursor(Tmp_win, ewp->x, ewp->y);
					}
					if(ewp->window == Tmp_win->frame ||
					                (Scr->IconManagerFocus &&
					                 Tmp_win->iconmanagerlist &&
					                 ewp->window == Tmp_win->iconmanagerlist->w)) {

						if(!scanArgs.leaves && !scanArgs.enters) {
							InstallColormaps(EnterNotify,       /* 2 */
							                 &Scr->RootColormaps);
						}

						/*
						 * Event is in the frame or the icon mgr:
						 *
						 * "4" -- TitleFocus is set: windows should get
						 *        focus as long as they accept input.
						 *
						 * "4a" - If TitleFocus is not set, windows should get
						 *        the focus if the event was in the icon mgr
						 *        (as long as they accept input).
						 *
						 */

						/* If the window takes input... */
						if(Tmp_win->wmhints->input) {

							/* if 4 or 4a, focus on the window */
							if(Scr->TitleFocus ||
							                (Tmp_win->iconmanagerlist &&
							                 (Tmp_win->iconmanagerlist->w == ewp->window))) {
								SetFocus(Tmp_win, ewp->time);
							}
						}

						if(Scr->TitleFocus &&
						                (Tmp_win->protocols & DoesWmTakeFocus)) {   /* 5 */

							/* for both locally or globally active */
							SendTakeFocusMessage(Tmp_win, ewp->time);
						}
						else if(!Scr->TitleFocus
						                && Tmp_win->wmhints->input
						                && Event.xcrossing.focus) {
							SynthesiseFocusIn(Tmp_win->w);
						}

					}
					else if(ewp->window == Tmp_win->w) {
						/*
						 * If we are entering the application window, install
						 * its colormap(s).
						 */
						if(Scr->BorderCursors) {
							SetBorderCursor(Tmp_win, -1000, -1000);
						}
						if(!scanArgs.leaves || scanArgs.inferior) {
							InstallWindowColormaps(EnterNotify, Tmp_win);
						}

						if(Event.xcrossing.focus) {
							SynthesiseFocusIn(Tmp_win->w);
						}

						/* must deal with WM_TAKE_FOCUS clients now, if
						   we're not in TitleFocus mode */

						if(!(Scr->TitleFocus) &&
						                (Tmp_win->protocols & DoesWmTakeFocus)) {

							/* locally active clients need help from WM
							   to get the input focus */

							if(Tmp_win->wmhints->input) {
								SetFocus(Tmp_win, ewp->time);
							}

							/* for both locally & globally active clnts */

							SendTakeFocusMessage(Tmp_win, ewp->time);
						}
					}
				}                       /* end if Tmp_win->mapped */
				if(ewp->window == Tmp_win->wmhints->icon_window &&
				                (!scanArgs.leaves || scanArgs.inferior)) {
					InstallWindowColormaps(EnterNotify, Tmp_win);
				}
			}                           /* end if FocusRoot */
			else if(Scr->BorderCursors && (ewp->window == Tmp_win->w)) {
				SetBorderCursor(Tmp_win, -1000, -1000);
			}
			/*
			 * If this window is to be autoraised, mark it so
			 */
			if(Tmp_win->auto_raise) {
				enter_win = Tmp_win;
				if(enter_flag == false) {
					AutoRaiseWindow(Tmp_win);
				}
			}
			else if(enter_flag && raise_win == Tmp_win) {
				enter_win = Tmp_win;
			}
			/*
			 * set ring leader
			 */
			if(WindowIsOnRing(Tmp_win) && (!enter_flag || raise_win == enter_win)) {
				Scr->RingLeader = Tmp_win;
			}
			XSync(dpy, 0);
			return;
		}                               /* end if Tmp_win */
	}                                   /* end if !ActiveMenu */

	/*
	 * Find the menu that we are dealing with now; punt if unknown
	 */
	if(XFindContext(dpy, ewp->window, MenuContext, (XPointer *)&mr) != XCSUCCESS) {
		return;
	}

	if(! ActiveMenu && mr->pinned && (RootFunction == 0)) {
		PopUpMenu(mr, 0, 0, false);
		Context = C_ROOT;
		UpdateMenu();
		return;
	}
	mr->entered = true;
	if(RootFunction == 0) {
		for(tmp = ActiveMenu; tmp; tmp = tmp->prev) {
			if(tmp == mr) {
				break;
			}
		}
		if(! tmp) {
			return;
		}

		for(tmp = ActiveMenu; tmp != mr; tmp = tmp->prev) {
			if(tmp->pinned) {
				break;
			}
			HideMenu(tmp);
			MenuDepth--;
		}
		UninstallRootColormap();

		if(ActiveItem) {
			ActiveItem->state = 0;
			PaintEntry(ActiveMenu, ActiveItem, false);
		}
		ActiveItem = NULL;
		ActiveMenu = mr;
		if(1/*Scr->StayUpMenus*/) {
			int i, x, y, x_root, y_root, entry;
			MenuItem *mi;

			XQueryPointer(dpy, ActiveMenu->w, &JunkRoot, &JunkChild, &x_root, &y_root,
			              &x, &y, &JunkMask);
			if((x > 0) && (y > 0) && (x < ActiveMenu->width) && (y < ActiveMenu->height)) {
				entry = y / Scr->EntryHeight;
				for(i = 0, mi = ActiveMenu->first; mi != NULL; i++, mi = mi->next) {
					if(i == entry) {
						break;
					}
				}
				if(mi) {
					ActiveItem = mi;
					ActiveItem->state = 1;
					PaintEntry(ActiveMenu, ActiveItem, false);
				}
			}
		}
		if(ActiveMenu->pinned) {
			XUngrabPointer(dpy, CurrentTime);
		}
	}
	return;
}


/***********************************************************************
 *
 *  Procedure:
 *      HLNQueueScanner - LeaveNotify event q scanner
 *
 *      Looks at the queued events and determines if any
 *      EnterNotify events are behind this event to allow
 *      skipping of unnecessary processing.
 *
 ***********************************************************************
 */

typedef struct HLNScanArgs {
	Window w;           /* The window getting the LeaveNotify */
	Bool enters;        /* Any EnterNotify event at all */
	Bool matches;       /* Any matching EnterNotify events */
} HLNScanArgs;

/* ARGSUSED*/
static Bool HLNQueueScanner(Display *display, XEvent *ev, char *_args)
{
	HLNScanArgs *args = (void *)_args;

	if(ev->type == EnterNotify && ev->xcrossing.mode != NotifyGrab) {
		args->enters = True;
		if(ev->xcrossing.window == args->w) {
			args->matches = True;
		}
	}

	return (False);
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleLeaveNotify - LeaveNotify event handler
 *
 ***********************************************************************
 */

void HandleLeaveNotify(void)
{
	bool inicon;

	if(ActiveMenu && ActiveMenu->pinned
	                && (Event.xcrossing.window == ActiveMenu->w)) {
		PopDownMenu();
	}

	if(Tmp_win == NULL) {
		/* No window to be Leave'ing, so nothing much to do... */
		return;
	}

	/*
	 * We're not interested in pseudo Enter/Leave events generated
	 * from grab initiations and terminations.
	 */
	if(Event.xcrossing.mode != NotifyNormal) {
		return;
	}

	if(Scr->ShrinkIconTitles &&
	                Tmp_win->icon &&
	                Event.xcrossing.window == Tmp_win->icon->w &&
	                Event.xcrossing.detail != NotifyInferior) {
		ShrinkIconTitle(Tmp_win);
		return;
	}

	// Are we Leave'ing the icon manager entry for the Tmp_win in
	// question, or some other part of the window itself?
	inicon = (Tmp_win->iconmanagerlist &&
	          Tmp_win->iconmanagerlist->w == Event.xcrossing.window);

	if(Scr->RingLeader && Scr->RingLeader == Tmp_win &&
	                (Event.xcrossing.detail != NotifyInferior &&
	                 Event.xcrossing.window != Tmp_win->w)) {
#ifdef DEBUG
		fprintf(stderr,
		        "HandleLeaveNotify: Event.xcrossing.window %x != Tmp_win->w %x\n",
		        Event.xcrossing.window, Tmp_win->w);
#endif
		if(!inicon) {
			if(Event.xcrossing.window != Tmp_win->frame /*was: Tmp_win->mapped*/) {
				Tmp_win->ring.cursor_valid = false;
#ifdef DEBUG
				fprintf(stderr, "HandleLeaveNotify: cursor_valid = false\n");
#endif
			}
			else {          /* Event.xcrossing.window == Tmp_win->frame */
				Tmp_win->ring.cursor_valid = true;
				Tmp_win->ring.curs_x = (Event.xcrossing.x_root -
				                        Tmp_win->frame_x);
				Tmp_win->ring.curs_y = (Event.xcrossing.y_root -
				                        Tmp_win->frame_y);
#ifdef DEBUG
				fprintf(stderr,
				        "HandleLeaveNotify: cursor_valid = true; x = %d (%d-%d), y = %d (%d-%d)\n",
				        Tmp_win->ring.curs_x, Event.xcrossing.x_root, Tmp_win->frame_x,
				        Tmp_win->ring.curs_y, Event.xcrossing.y_root, Tmp_win->frame_y);
#endif
			}
		}
		Scr->RingLeader = NULL;
	}


	/*
	 * Are we moving focus based on the leave?  There are 2 steps to
	 * this:
	 * - Scr->FocusRoot is our "are we automatically changing focus
	 *   based on the leave" flag.  This gets unset when ClickToFocus
	 *   is config'd or a window is f.focus'd.
	 * - Then we check the detail for the focus leaving.  We're only
	 *   getting here for normal entry/exits.  Most cases outside of
	 *   the icon manager peek ahead in the event queue for any
	 *   Enter's, and don't do anything if there are; if there were,
	 *   we'd be moving the focus when we get them, so no point doing
	 *   it twice.  So the remainder here assumes there aren't any
	 *   Enters waiting.
	 *
	 *   See
	 *   <https://www.x.org/releases/X11R7.7/doc/libX11/libX11/libX11.html#Normal_EntryExit_Events>
	 *   for details of the cases.  So, when do we want to un-set the
	 *   focus?  Let's look at each doc'd value...
	 *   - NotifyAncestor means we're leaving a subwindow for its
	 *     parent.  That means the root, so, yep, we want to yield
	 *     focus up to it.
	 *   - NotifyNonLinear means we're leaving a window for something
	 *     that isn't a parent or child.  So we should probably yield
	 *     focus in this case too.
	 *   - NotifyInferior means we're leaving a window for a
	 *     subwindow of it.  From the WM perspective, that means
	 *     we're leaving focus where it was.
	 *   - NotifyVirtual means we're in the middle of an ascending
	 *     sequence.  Nothing to do; the Ancestor handling already
	 *     did the job.
	 *   - NotifyNonLinearVirtual is another "in the middle" case, so
	 *     we skip handling there too; the endpoints will do
	 *     whatever's necessary.
	 */
	if(Scr->FocusRoot
	                && Event.xcrossing.detail != NotifyInferior
	                && Event.xcrossing.detail != NotifyVirtual
	                && Event.xcrossing.detail != NotifyNonlinearVirtual
	  ) {
		HLNScanArgs scanArgs;
		XEvent dummy;

		/*
		 * Scan for EnterNotify events to see if we can avoid some
		 * unnecessary processing.
		 */
		scanArgs.w = Event.xcrossing.window;
		scanArgs.enters = scanArgs.matches = False;
		XCheckIfEvent(dpy, &dummy, HLNQueueScanner,
		              (char *) &scanArgs);

		if((inicon && Scr->IconManagerFocus)
		                || (Event.xcrossing.window == Tmp_win->frame
		                    && !scanArgs.matches)
		  ) {
			// Defocusing window because we moved out of its entry in an
			// icon manager, or because we moved out of its frame.

			// Nothing to do if we were in the icon manager, and the
			// window's either unmapped or doesn't accept input.  XXX Is
			// the inicon flag needed here?  If it's not mapped, we
			// presumably couldn't have gotten a Leave on its frame
			// anyway, and if it's not accepting input, we probably don't
			// need to focus out anyway?  Left conditional because this
			// matches historical behavior prior to some rework here, but
			// revisit.
			if(inicon && (!Tmp_win->mapped || !Tmp_win->wmhints->input)) {
				return;
			}

			// Shift away focus
			if(Scr->TitleFocus || Tmp_win->protocols & DoesWmTakeFocus) {
				SetFocus(NULL, Event.xcrossing.time);
			}

			// If we're in the icon manager, we need to take a FocusOut
			// event for the window, since it wouldn't have gotten one.
			// If we're in the frame, we fake one anyway as historical
			// code says "pretend there was a focus out as sometimes we
			// don't get one".
			if(Event.xcrossing.focus) {
				SynthesiseFocusOut(Tmp_win->w);
			}
		}
		else if(Event.xcrossing.window == Tmp_win->w && !scanArgs.enters) {
			// Flipping colormaps around because we moved out of the
			// window.
			InstallColormaps(LeaveNotify, &Scr->RootColormaps);
		}
	}

	/* Autolower modification. */
	if(Tmp_win->auto_lower) {
		leave_win = Tmp_win;
		if(leave_flag == false) {
			AutoLowerWindow(Tmp_win);
		}
	}
	else if(leave_flag && lower_win == Tmp_win) {
		leave_win = Tmp_win;
	}

	XSync(dpy, 0);
	return;
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleConfigureRequest - ConfigureRequest event handler
 *
 ***********************************************************************
 */

void HandleConfigureRequest(void)
{
	XWindowChanges xwc;
	unsigned long xwcm;
	int x, y, width, height, bw;
	int gravx, gravy;
	XConfigureRequestEvent *cre = &Event.xconfigurerequest;
	bool sendEvent;

#ifdef DEBUG_EVENTS
	fprintf(stderr, "ConfigureRequest\n");
	if(cre->value_mask & CWX) {
		fprintf(stderr, "  x = %d\n", cre->x);
	}
	if(cre->value_mask & CWY) {
		fprintf(stderr, "  y = %d\n", cre->y);
	}
	if(cre->value_mask & CWWidth) {
		fprintf(stderr, "  width = %d\n", cre->width);
	}
	if(cre->value_mask & CWHeight) {
		fprintf(stderr, "  height = %d\n", cre->height);
	}
	if(cre->value_mask & CWSibling) {
		fprintf(stderr, "  above = 0x%x\n", (unsigned)cre->above);
	}
	if(cre->value_mask & CWStackMode) {
		fprintf(stderr, "  stack = %d\n", cre->detail);
	}
#endif

	/*
	 * Event.xany.window is Event.xconfigurerequest.parent, so Tmp_win will
	 * be wrong
	 */
	Event.xany.window = cre->window;    /* mash parent field */
	Tmp_win = GetTwmWindow(cre->window);

	/*
	 * According to the July 27, 1988 ICCCM draft, we should ignore size and
	 * position fields in the WM_NORMAL_HINTS property when we map a window.
	 * Instead, we'll read the current geometry.  Therefore, we should respond
	 * to configuration requests for windows which have never been mapped.
	 */
	if(!Tmp_win || (Tmp_win->icon && (Tmp_win->icon->w == cre->window))) {
		xwcm = cre->value_mask &
		       (CWX | CWY | CWWidth | CWHeight | CWBorderWidth);
		xwc.x = cre->x;
		xwc.y = cre->y;
		xwc.width = cre->width;
		xwc.height = cre->height;
		xwc.border_width = cre->border_width;
		XConfigureWindow(dpy, Event.xany.window, xwcm, &xwc);
		return;
	}

	sendEvent = false;
	if((cre->value_mask & CWStackMode) && Tmp_win->stackmode) {
		TwmWindow *otherwin;

		if(cre->value_mask & CWSibling) {
			otherwin = GetTwmWindow(cre->above);
			if(otherwin) {
				OtpForcePlacement(Tmp_win, cre->detail, otherwin);
			}
			else {
				fprintf(stderr, "XConfigureRequest: unkown otherwin\n");
			}
		}
		else {
			switch(cre->detail) {
				case TopIf:
				case Above:
					OtpRaise(Tmp_win, WinWin);
					break;
				case BottomIf:
				case Below:
					OtpLower(Tmp_win, WinWin);
					break;
				case Opposite:
					OtpRaiseLower(Tmp_win, WinWin);
					break;
				default:
					;
			}
		}
		sendEvent = true;
	}


	/* Don't modify frame_XXX fields before calling SetupWindow! */
	x = Tmp_win->frame_x;
	y = Tmp_win->frame_y;
	width = Tmp_win->frame_width;
	height = Tmp_win->frame_height;
	bw = Tmp_win->frame_bw;

	/*
	 * Section 4.1.5 of the ICCCM states that the (x,y) coordinates in the
	 * configure request are for the upper-left outer corner of the window.
	 * This means that we need to adjust for the additional title height as
	 * well as for any border width changes that we decide to allow.  The
	 * current window gravity is to be used in computing the adjustments, just
	 * as when initially locating the window.  Note that if we do decide to
	 * allow border width changes, we will need to send the synthetic
	 * ConfigureNotify event.
	 */
	GetGravityOffsets(Tmp_win, &gravx, &gravy);

	if(cre->value_mask & CWBorderWidth) {
		int bwdelta = cre->border_width - Tmp_win->old_bw;  /* posit growth */
		if(bwdelta && Scr->ClientBorderWidth) {   /* if change allowed */
			x += gravx * bwdelta;       /* change default values only */
			y += gravy * bwdelta;       /* ditto */
			bw = cre->border_width;
			if(Tmp_win->title_height) {
				height += bwdelta;
			}
			x += (gravx < 0) ? bwdelta : -bwdelta;
			y += (gravy < 0) ? bwdelta : -bwdelta;
		}
		Tmp_win->old_bw = cre->border_width;  /* for restoring */
	}

	if((cre->value_mask & CWX)) {       /* override even if border change */
		x = cre->x - bw;
		x -= ((gravx < 0) ? 0 : Tmp_win->frame_bw3D);
	}
	if((cre->value_mask & CWY)) {
		y = cre->y - ((gravy < 0) ? 0 : Tmp_win->title_height) - bw;
		y -= ((gravy < 0) ? 0 : Tmp_win->frame_bw3D);
	}

	if(cre->value_mask & CWWidth) {
		width = cre->width + 2 * Tmp_win->frame_bw3D;
	}
	if(cre->value_mask & CWHeight) {
		height = cre->height + Tmp_win->title_height + 2 * Tmp_win->frame_bw3D;
	}

	if(width != Tmp_win->frame_width || height != Tmp_win->frame_height) {
		unzoom(Tmp_win);
	}

	/* Workaround for Java 1.4 bug that freezes the application whenever
	 * a new window is displayed. (When UsePPosition is on and either
	 * UseThreeDBorders or BorderWidth 0 is set.)
	 */
	if(!bw) {
		sendEvent = true;
	}

	/*
	 * SetupWindow (x,y) are the location of the upper-left outer corner and
	 * are passed directly to XMoveResizeWindow (frame).  The (width,height)
	 * are the inner size of the frame.  The inner width is the same as the
	 * requested client window width; the inner height is the same as the
	 * requested client window height plus any title bar slop.
	 */
#ifdef DEBUG_EVENTS
	fprintf(stderr, "SetupFrame(x=%d, y=%d, width=%d, height=%d, bw=%d)\n",
	        x, y, width, height, bw);
#endif
	SetupFrame(Tmp_win, x, y, width, height, bw, sendEvent);
}


/***********************************************************************
 *
 *  Procedure:
 *      HandleShapeNotify - shape notification event handler
 *
 ***********************************************************************
 */
void
HandleShapeNotify(void)
{
	XShapeEvent     *sev = (XShapeEvent *) &Event;

	if(Tmp_win == NULL) {
		return;
	}
	if(sev->kind != ShapeBounding) {
		return;
	}
	if(!Tmp_win->wShaped && sev->shaped) {
		XShapeCombineMask(dpy, Tmp_win->frame, ShapeClip, 0, 0, None,
		                  ShapeSet);
	}
	Tmp_win->wShaped = sev->shaped;
	SetFrameShape(Tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *      HandleSelectionClear - selection lost event handler
 *
 ***********************************************************************
 */
#ifdef EWMH
void
HandleSelectionClear(void)
{
	XSelectionClearEvent    *sev = (XSelectionClearEvent *) &Event;

	if(sev->window == Scr->icccm_Window) {
		EwmhSelectionClear(sev);
	}
}
#endif


/***********************************************************************
 *
 *  Procedure:
 *      HandleUnknown - unknown event handler
 *
 ***********************************************************************
 */

void HandleUnknown(void)
{
#ifdef DEBUG_EVENTS
	fprintf(stderr, "HandleUnknown: Event.type = %d\n", Event.type);
#endif
}


static void flush_expose(Window w)
{
	XEvent dummy;

	while(XCheckTypedWindowEvent(dpy, w, Expose, &dummy)) {
		/* nada */;
	}
}


/* Util func used a few times above */
static void
SendTakeFocusMessage(TwmWindow *tmp, Time timestamp)
{
	send_clientmessage(tmp->w, XA_WM_TAKE_FOCUS, timestamp);
}
