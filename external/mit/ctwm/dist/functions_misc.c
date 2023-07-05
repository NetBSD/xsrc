/*
 * Misc function implementation
 *
 * These are things that either don't fit neatly into another category,
 * or fit into a category too small to be worth making individual files
 * for.
 */

#include "ctwm.h"

#include <stdlib.h>

#include "animate.h"
#include "ctwm_shutdown.h"
#include "functions.h"
#include "functions_defs.h"
#include "functions_internal.h"
#include "icons.h"
#include "otp.h"
#include "screen.h"
#ifdef SOUNDS
#include "sound.h"
#endif
#include "util.h"
#include "win_iconify.h"
#ifdef WINBOX
#include "windowbox.h"
#endif
#include "workspace_utils.h"

#include "ext/repl_str.h"



/*
 * Animation-related
 */
DFHANDLER(startanimation)
{
	StartAnimation();
}

DFHANDLER(stopanimation)
{
	StopAnimation();
}

DFHANDLER(speedupanimation)
{
	ModifyAnimationSpeed(1);
}

DFHANDLER(slowdownanimation)
{
	ModifyAnimationSpeed(-1);
}



/*
 * Menu-related
 */
DFHANDLER(menu)
{
	/*
	 * n.b.: The f.menu handler is all kinds of magic; it's actually
	 * completely unrelated to pulling up the menu.
	 *
	 * When a button/key binding invokes f.menu to open up a menu, that's
	 * actually handled in the KeyPress or ButtonPress handlers by
	 * calling do{_key,}_menu().  When we descend into a submenu, that's
	 * handled in KeyPress handler for keyboard navigation when we hit
	 * the Right arrow, or inside the
	 * event loop recapture in UpdateMenu() for mouse navigation when we
	 * move it to the right side of the menu entry.
	 *
	 * This handler is only used by "invoking" a menu item; releasing the
	 * mouse button on the left side without moving right to open out the
	 * submenu, or hitting the Enter key.  All it does is immediately
	 * invoke the default entry, if there is one.
	 */
	if(action && ! strncmp(action, "WGOTO : ", 8)) {
		GotoWorkSpaceByName(/* XXXXX */ Scr->currentvs,
		                                ((char *)action) + 8);
	}
	else {
		MenuItem *item;

		item = ActiveItem;
		while(item && item->sub) {
			if(!item->sub->defaultitem) {
				break;
			}
			if(item->sub->defaultitem->func != F_MENU) {
				break;
			}
			item = item->sub->defaultitem;
		}
		if(item && item->sub && item->sub->defaultitem) {
			ExecuteFunction(item->sub->defaultitem->func,
			                item->sub->defaultitem->action,
			                w, tmp_win, eventp, context, pulldown);
		}
	}
}


DFHANDLER(pin)
{
	if(! ActiveMenu) {
		return;
	}
	if(ActiveMenu->pinned) {
		XUnmapWindow(dpy, ActiveMenu->w);
		ActiveMenu->mapped = MRM_UNMAPPED;
	}
	else {
		XWindowAttributes attr;
		MenuRoot *menu;

		if(ActiveMenu->pmenu == NULL) {
			menu  = malloc(sizeof(MenuRoot));
			*menu = *ActiveMenu;
			menu->pinned = true;
			menu->mapped = MRM_NEVER;
			menu->width -= 10;
			if(menu->pull) {
				menu->width -= 16 + 10;
			}
			MakeMenu(menu);
			ActiveMenu->pmenu = menu;
		}
		else {
			menu = ActiveMenu->pmenu;
		}
		if(menu->mapped == MRM_MAPPED) {
			return;
		}
		XGetWindowAttributes(dpy, ActiveMenu->w, &attr);
		menu->x = attr.x;
		menu->y = attr.y;
		XMoveWindow(dpy, menu->w, menu->x, menu->y);
		XMapRaised(dpy, menu->w);
		menu->mapped = MRM_MAPPED;
	}
	PopDownMenu();
}



/*
 * Alternate keymaps/contexts
 */
DFHANDLER(altkeymap)
{
	int alt, stat_;

	if(! action) {
		return;
	}
	stat_ = sscanf(action, "%d", &alt);
	if(stat_ != 1) {
		return;
	}
	if((alt < 1) || (alt > 5)) {
		return;
	}
	AlternateKeymap = Alt1Mask << (alt - 1);
	XGrabPointer(dpy, Scr->Root, True, ButtonPressMask | ButtonReleaseMask,
	             GrabModeAsync, GrabModeAsync,
	             Scr->Root, Scr->AlterCursor, CurrentTime);
	func_reset_cursor = false;  // Leave special cursor alone
	XGrabKeyboard(dpy, Scr->Root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	return;
}

DFHANDLER(altcontext)
{
	AlternateContext = true;
	XGrabPointer(dpy, Scr->Root, False, ButtonPressMask | ButtonReleaseMask,
	             GrabModeAsync, GrabModeAsync,
	             Scr->Root, Scr->AlterCursor, CurrentTime);
	func_reset_cursor = false;  // Leave special cursor alone
	XGrabKeyboard(dpy, Scr->Root, False, GrabModeAsync, GrabModeAsync, CurrentTime);
	return;
}



/*
 * A few trivial ctwm-control-ish meta-functions
 */
DFHANDLER(quit)
{
	DoShutdown();
}

DFHANDLER(restart)
{
	DoRestart(eventp->xbutton.time);
}

DFHANDLER(beep)
{
	XBell(dpy, 0);
}

DFHANDLER(trace)
{
	DebugTrace(action);
}



#ifdef WINBOX
/*
 * Special windowbox-related
 */
DFHANDLER(fittocontent)
{
	if(!tmp_win->iswinbox) {
		XBell(dpy, 0);
		return;
	}
	fittocontent(tmp_win);
}
#endif



/*
 * A few things that are sorta windows/icons related, but don't really
 * fit with the window-targetted things in functions_win.
 */
DFHANDLER(showbackground)
{
	ShowBackground(Scr->currentvs, -1);
}

DFHANDLER(raiseicons)
{
	for(TwmWindow *t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t->icon && t->icon->w) {
			OtpRaise(t, IconWin);
		}
	}
}

DFHANDLER(rescuewindows)
{
	RescueWindows();
}



/*
 * Despite the name, this is more like 'gotoworkspace' than the other
 * 'warpto*' funcs, as it's just about switching your view, not anything
 * going to a window.
 */
static void
WarpToScreen(int n, int inc)
{
	Window dumwin;
	int x, y, dumint;
	unsigned int dummask;
	ScreenInfo *newscr = NULL;

	while(!newscr) {
		/* wrap around */
		if(n < 0) {
			n = NumScreens - 1;
		}
		else if(n >= NumScreens) {
			n = 0;
		}

		newscr = ScreenList[n];
		if(!newscr) {                   /* make sure screen is managed */
			if(inc) {                   /* walk around the list */
				n += inc;
				continue;
			}
			fprintf(stderr, "%s:  unable to warp to unmanaged screen %d\n",
			        ProgramName, n);
			XBell(dpy, 0);
			return;
		}
	}

	if(Scr->screen == n) {
		return;        /* already on that screen */
	}

	PreviousScreen = Scr->screen;
	XQueryPointer(dpy, Scr->Root, &dumwin, &dumwin, &x, &y,
	              &dumint, &dumint, &dummask);

	XWarpPointer(dpy, None, newscr->Root, 0, 0, 0, 0, x, y);
	Scr = newscr;
	return;
}

DFHANDLER(warptoscreen)
{
	if(strcmp(action, WARPSCREEN_NEXT) == 0) {
		WarpToScreen(Scr->screen + 1, 1);
	}
	else if(strcmp(action, WARPSCREEN_PREV) == 0) {
		WarpToScreen(Scr->screen - 1, -1);
	}
	else if(strcmp(action, WARPSCREEN_BACK) == 0) {
		WarpToScreen(PreviousScreen, 0);
	}
	else {
		WarpToScreen(atoi(action), 0);
	}
}



/*
 * Sound-related
 */
#ifdef SOUNDS
DFHANDLER(togglesound)
{
	toggle_sound();
}

DFHANDLER(rereadsounds)
{
	reread_sounds();
}
#endif



/*
 * And executing an external program
 */
static void Execute(const char *_s);

DFHANDLER(exec)
{
	PopDownMenu();
	if(!Scr->NoGrabServer) {
		XUngrabServer(dpy);
		XSync(dpy, 0);
	}
	XUngrabPointer(dpy, CurrentTime);
	XSync(dpy, 0);
	Execute(action);
}


static void
Execute(const char *_s)
{
	char *s;
	char *_ds;
	char *orig_display;
	int restorevar = 0;
	char *subs;

	/* Seatbelt */
	if(!_s) {
		return;
	}

	/* Work on a local copy since we're mutating it */
	s = strdup(_s);
	if(!s) {
		return;
	}

	/* Stash up current $DISPLAY value for resetting */
	orig_display = getenv("DISPLAY");


	/*
	 * Build a display string using the current screen number, so that
	 * X programs which get fired up from a menu come up on the screen
	 * that they were invoked from, unless specifically overridden on
	 * their command line.
	 *
	 * Which is to say, given that we're on display "foo.bar:1.2", we
	 * want to translate that into "foo.bar:1.{Scr->screen}".
	 *
	 * We strdup() because DisplayString() is a macro returning into the
	 * dpy structure, and we're going to mutate the value we get from it.
	 */
	_ds = DisplayString(dpy);
	if(_ds) {
		char *ds;
		char *colon;

		ds = strdup(_ds);
		if(!ds) {
			goto end_execute;
		}

		/* If it's not host:dpy, we don't have anything to do here */
		colon = strrchr(ds, ':');
		if(colon) {
			char *dot, *new_display;

			/* Find the . in display.screen and chop it off */
			dot = strchr(colon, '.');
			if(dot) {
				*dot = '\0';
			}

			/* Build a new string with our correct screen info */
			asprintf(&new_display, "%s.%d", ds, Scr->screen);
			if(!new_display) {
				free(ds);
				goto end_execute;
			}

			/* And set */
			setenv("DISPLAY", new_display, 1);
			free(new_display);
			restorevar = 1;
		}
		free(ds);
	}


	/*
	 * We replace a couple placeholders in the string.  $currentworkspace
	 * is documented in the manual; $redirect is not.
	 */
	subs = strstr(s, "$currentworkspace");
	if(subs) {
		char *tmp;
		char *wsname;

		wsname = GetCurrentWorkSpaceName(Scr->currentvs);
		if(!wsname) {
			wsname = "";
		}

		tmp = replace_substr(s, "$currentworkspace", wsname);
		if(!tmp) {
			goto end_execute;
		}
		free(s);
		s = tmp;
	}

#ifdef CAPTIVE
	subs = strstr(s, "$redirect");
	if(subs) {
		char *tmp;
		char *redir;

		if(CLarg.is_captive) {
			asprintf(&redir, "-xrm 'ctwm.redirect:%s'", Scr->captivename);
			if(!redir) {
				goto end_execute;
			}
		}
		else {
			redir = malloc(1);
			*redir = '\0';
		}

		tmp = replace_substr(s, "$redirect", redir);
		free(s);
		s = tmp;

		free(redir);
	}
#endif


	/*
	 * Call it.  Return value doesn't really matter, since whatever
	 * happened we're done.  Maybe someday if we develop a "show user
	 * message" generalized func, we can tell the user if executing
	 * failed somehow.
	 */
	system(s);


	/*
	 * Restore $DISPLAY if we changed it.  It's probably only necessary
	 * in edge cases (it might be used by ctwm restarting itself, for
	 * instance) and it's not quite clear whether the DisplayString()
	 * result would even be wrong for that, but what the heck, setenv()
	 * is cheap.
	 */
	if(restorevar) {
		if(orig_display) {
			setenv("DISPLAY", orig_display, 1);
		}
		else {
			unsetenv("DISPLAY");
		}
	}


	/* Clean up */
end_execute:
	free(s);
	return;
}
