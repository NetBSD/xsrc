/*
 * Occupation handling bits
 *
 * This is in fact pretty tightly tied and extremely similar to the
 * handling of the WorkSpaceManager in workmgr.c, and used to be there.
 * It makes sense to consider them together (and indeed, many of the
 * configs that affect how this works are really WorkSpaceManager* or
 * WMgr* commands.  But having them crammed together in one file is
 * unwieldy.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "add_window.h"
#include "ctwm_atoms.h"
#include "drawing.h"
#include "events.h"
#include "iconmgr.h"
#include "list.h"
#include "screen.h"
#include "occupation.h"
#include "otp.h"
#include "util.h"
#include "vscreen.h"
#include "win_iconify.h"
#include "win_regions.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "workspace_utils.h"


static int GetMaskFromResource(TwmWindow *win, char *res);
static char *mk_nullsep_string(const char *prop, int len);

static bool CanChangeOccupation(TwmWindow **twm_winp);

int fullOccupation = 0;

/*
 * The window whose occupation is currently being manipulated.
 *
 * XXX Should probably be static, but currently needed in
 * WMapRemoveWindow().  Revisit.
 */
TwmWindow *occupyWin = NULL;


/* XXX Share with captive.c? */
static XrmOptionDescRec table [] = {
	{"-xrm",            NULL,           XrmoptionResArg, (XPointer) NULL},
};




/*
 ****************************************************************
 *
 * First, funcs related to setting and changing a window's occupation.
 *
 ****************************************************************
 */


/*
 * Setup the occupation of a TwmWindow.  Called as part of the
 * AddWindow() process.
 *
 * XXX The logic flow in this is kinda weird, and it's not at all clear
 * to what extent it's really doing the right on on what should override
 * what, or which things should expand/contract on others...
 */
void
SetupOccupation(TwmWindow *twm_win, int occupation_hint)
{
	char      **cliargv = NULL;
	int       cliargc;
	WorkSpace *ws;

	/* If there aren't any config'd workspaces, there's only 0 */
	if(! Scr->workSpaceManagerActive) {
		twm_win->occupation = 1 << 0;   /* occupy workspace #0 */
		/* more?... */

		return;
	}

	/* Workspace manager window doesn't get futzed with */
	if(twm_win->iswspmgr) {
		return;
	}

	/*twm_win->occupation = twm_win->iswinbox ? fullOccupation : 0;*/
	twm_win->occupation = 0;

	/* Specified in any Occupy{} config params? */
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(LookInList(ws->clientlist, twm_win->name, &twm_win->class)) {
			twm_win->occupation |= 1 << ws->number;
		}
	}

	/* OccupyAll{} */
	if(LookInList(Scr->OccupyAll, twm_win->name, &twm_win->class)) {
		twm_win->occupation = fullOccupation;
	}

	/* See if it specified in -xrm stuff */
	if(XGetCommand(dpy, twm_win->w, &cliargv, &cliargc)) {
		Bool status;
		char *str_type;
		XrmValue value;
		XrmDatabase db = NULL;

		XrmParseCommand(&db, table, 1, "ctwm", &cliargc, cliargv);
		XFreeStringList(cliargv);
		status = XrmGetResource(db, "ctwm.workspace", "Ctwm.Workspace",
		                        &str_type, &value);
		if((status == True) && (value.size != 0)) {
			/* Copy the value.addr because it's in XRM memory not ours */
			char wrkSpcList[512];
			safe_strncpy(wrkSpcList, value.addr, MIN(value.size, 512));

			twm_win->occupation = GetMaskFromResource(twm_win, wrkSpcList);
		}
		XrmDestroyDatabase(db);
	}

	/* Does it have a property telling us */
	if(RestartPreviousState) {
		Atom actual_type;
		int actual_format;
		unsigned long nitems, bytesafter;
		unsigned char *prop;

		if(XGetWindowProperty(dpy, twm_win->w, XA_WM_OCCUPATION, 0L, 2500, False,
		                      XA_STRING, &actual_type, &actual_format, &nitems,
		                      &bytesafter, &prop) == Success) {
			if(nitems != 0) {
				twm_win->occupation = GetMaskFromProperty(prop, nitems);
				XFree(prop);
			}
		}
	}

#ifdef EWMH
	/* Maybe EWMH has something to tell us? */
	if(twm_win->occupation == 0) {
		twm_win->occupation = EwmhGetOccupation(twm_win);
	}
#endif /* EWMH */

	/* Icon Managers shouldn't get altered */
	/* XXX Should this be up near the top? */
	if(twm_win->isiconmgr) {
		return;        /* someone tried to modify occupation of icon managers */
	}


	/*
	 * Transient-ish things go with their parents unless
	 * TransientHasOccupation set in the config.
	 */
	if(! Scr->TransientHasOccupation) {
		TwmWindow *t;

		if(twm_win->istransient) {
			t = GetTwmWindow(twm_win->transientfor);
			if(t != NULL) {
				twm_win->occupation = t->occupation;
			}
		}
		else if(twm_win->group != 0) {
			t = GetTwmWindow(twm_win->group);
			if(t != NULL) {
				twm_win->occupation = t->occupation;
			}
		}
	}


	/* If we were told something specific, go with that */
	if(occupation_hint != 0) {
		twm_win->occupation = occupation_hint;
	}

	/* If it's apparently-nonsensical, put it in its vs's workspace */
	if((twm_win->occupation & fullOccupation) == 0) {
		twm_win->occupation = 1 << twm_win->vs->wsw->currentwspc->number;
	}

	/*
	 * If the occupation would not show it in the current vscreen,
	 * make it vanish.
	 *
	 * If it could be shown in one of the other vscreens, change the vscreen.
	 */
	if(!OCCUPY(twm_win, twm_win->vs->wsw->currentwspc)) {

		twm_win->vs = NULL;

#ifdef VSCREEN
		if(Scr->numVscreens > 1) {
			VirtualScreen *vs;
			for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
				if(OCCUPY(twm_win, vs->wsw->currentwspc)) {
					twm_win->vs = vs;
					twm_win->parent_vs = vs;
					break;
				}
			}
		}
#endif
	}


	/* Set the property for the occupation */
	{
		long eventMask;
		char *wsstr;
		int  len;

		/* Ignore the PropertyChange we're about to do */
		if((eventMask = mask_out_event(twm_win->w, PropertyChangeMask)) < 0) {
			/* Window is horked, not much we can do */
			return;
		}

		/* Set the property for the occupation */
		len = GetPropertyFromMask(twm_win->occupation, &wsstr);
		XChangeProperty(dpy, twm_win->w, XA_WM_OCCUPATION, XA_STRING, 8,
		                PropModeReplace, (unsigned char *) wsstr, len);
		free(wsstr);

#ifdef EWMH
		EwmhSet_NET_WM_DESKTOP(twm_win);
#endif

		/* Restore event mask */
		restore_mask(twm_win->w, eventMask);
	}

	/* Set WM_STATE prop */
	{
		int state = NormalState;
		Window icon;

		if(!(RestartPreviousState
		                && GetWMState(twm_win->w, &state, &icon)
		                && (state == NormalState || state == IconicState
		                    || state == InactiveState))) {
			if(twm_win->wmhints->flags & StateHint) {
				state = twm_win->wmhints->initial_state;
			}
		}
		if(visible(twm_win)) {
			if(state == InactiveState) {
				SetMapStateProp(twm_win, NormalState);
			}
		}
		else {
			if(state == NormalState) {
				SetMapStateProp(twm_win, InactiveState);
			}
		}
	}
}


/*
 * Make sure a window is marked in a given workspace.  f.addtoworkspace.
 * Also gets called as part of the process of mapping a window; if we're
 * mapping it here, it should know that it's here.  And Xinerama magic
 * moves.
 */
void
AddToWorkSpace(char *wname, TwmWindow *twm_win)
{
	WorkSpace *ws;
	int newoccupation;

	if(!CanChangeOccupation(&twm_win)) {
		return;
	}
	ws = GetWorkspace(wname);
	if(!ws) {
		return;
	}

	if(twm_win->occupation & (1 << ws->number)) {
		return;
	}
	newoccupation = twm_win->occupation | (1 << ws->number);
	ChangeOccupation(twm_win, newoccupation);
}


/*
 * Converse of the above.  f.removefromworkspace, also called from
 * Xinerama-related magic.
 */
void
RemoveFromWorkSpace(char *wname, TwmWindow *twm_win)
{
	WorkSpace *ws;
	int newoccupation;

	if(!CanChangeOccupation(&twm_win)) {
		return;
	}
	ws = GetWorkspace(wname);
	if(!ws) {
		return;
	}

	newoccupation = twm_win->occupation & ~(1 << ws->number);
	if(!newoccupation) {
		return;
	}
	ChangeOccupation(twm_win, newoccupation);
}


/* f.toggleoccupation - flip setting for [current] workspace */
void
ToggleOccupation(char *wname, TwmWindow *twm_win)
{
	WorkSpace *ws;
	int newoccupation;

	if(!CanChangeOccupation(&twm_win)) {
		return;
	}
	ws = GetWorkspace(wname);
	if(!ws) {
		return;
	}

	newoccupation = twm_win->occupation ^ (1 << ws->number);
	if(!newoccupation) {
		/* Don't allow de-occupying _every_ ws */
		return;
	}
	ChangeOccupation(twm_win, newoccupation);
}


/* f.movetonextworkspace */
void
MoveToNextWorkSpace(VirtualScreen *vs, TwmWindow *twm_win)
{
	WorkSpace *wlist1, *wlist2;
	int newoccupation;

	if(!CanChangeOccupation(&twm_win)) {
		return;
	}

	wlist1 = vs->wsw->currentwspc;
	wlist2 = wlist1->next;
	wlist2 = wlist2 ? wlist2 : Scr->workSpaceMgr.workSpaceList;

	/* Out of (here), into (here+1) */
	newoccupation = (twm_win->occupation ^ (1 << wlist1->number))
	                | (1 << wlist2->number);
	ChangeOccupation(twm_win, newoccupation);
}


/* f.movetonextworkspaceandfollow */
void
MoveToNextWorkSpaceAndFollow(VirtualScreen *vs, TwmWindow *twm_win)
{
	if(!CanChangeOccupation(&twm_win)) {
		return;
	}

	MoveToNextWorkSpace(vs, twm_win);
	GotoNextWorkSpace(vs);
#if 0
	OtpRaise(twm_win, WinWin);  /* XXX really do this? */
#endif
}


/* f.movetoprevworkspaceand */
void
MoveToPrevWorkSpace(VirtualScreen *vs, TwmWindow *twm_win)
{
	WorkSpace *wlist1, *wlist2;
	int newoccupation;

	if(!CanChangeOccupation(&twm_win)) {
		return;
	}

	wlist1 = Scr->workSpaceMgr.workSpaceList;
	wlist2 = vs->wsw->currentwspc;
	if(wlist1 == NULL) {
		return;
	}

	while(wlist1->next != wlist2 && wlist1->next != NULL) {
		wlist1 = wlist1->next;
	}

	/* Out of (here), into (here-1) */
	newoccupation = (twm_win->occupation ^ (1 << wlist2->number))
	                | (1 << wlist1->number);
	ChangeOccupation(twm_win, newoccupation);
}


/* f.movetoprevworkspaceandfollow */
void
MoveToPrevWorkSpaceAndFollow(VirtualScreen *vs, TwmWindow *twm_win)
{
	if(!CanChangeOccupation(&twm_win)) {
		return;
	}

	MoveToPrevWorkSpace(vs, twm_win);
	GotoPrevWorkSpace(vs);
#if 0
	OtpRaise(twm_win, WinWin);          /* XXX really do this? */
#endif
}


/*
 * Set the occupation based on the window name.  This is called if
 * AutoOccupy is set, when we get a notification about a window name
 * change.
 */
void
WmgrRedoOccupation(TwmWindow *win)
{
	WorkSpace *ws;
	int       newoccupation;

	if(LookInList(Scr->OccupyAll, win->name, &win->class)) {
		newoccupation = fullOccupation;
	}
	else {
		newoccupation = 0;
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			if(LookInList(ws->clientlist, win->name, &win->class)) {
				newoccupation |= 1 << ws->number;
			}
		}
	}
	if(newoccupation != 0) {
		ChangeOccupation(win, newoccupation);
	}
}


/* f.vanish */
void
WMgrRemoveFromCurrentWorkSpace(VirtualScreen *vs, TwmWindow *win)
{
	WorkSpace *ws;
	int       newoccupation;

	ws = vs->wsw->currentwspc;
	if(!ws) {
		/* Impossible? */
		return;
	}
	if(! OCCUPY(win, ws)) {
		return;
	}

	newoccupation = win->occupation & ~(1 << ws->number);
	if(newoccupation == 0) {
		return;
	}

	ChangeOccupation(win, newoccupation);
}


/* f.warphere */
void
WMgrAddToCurrentWorkSpaceAndWarp(VirtualScreen *vs, char *winname)
{
	TwmWindow *tw;
	int       newoccupation;

	/* Find named window on this screen */
	for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
		if(match(winname, tw->name)) {
			break;
		}
	}

	/* Didn't find it by name?  Try by class */
	if(!tw) {
		for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
			if(match(winname, tw->class.res_name)) {
				break;
			}
		}
	}
	if(!tw) {
		for(tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
			if(match(winname, tw->class.res_class)) {
				break;
			}
		}
	}

	/* Still didn't find?  Beep at the user and bail. */
	if(!tw) {
		XBell(dpy, 0);
		return;
	}

	/* If WarpUnmapped isn't set and this isn't mapped, beep and bail */
	if((! Scr->WarpUnmapped) && (! tw->mapped)) {
		XBell(dpy, 0);
		return;
	}

	/* Move it here if it's not */
	if(! OCCUPY(tw, vs->wsw->currentwspc)) {
		newoccupation = tw->occupation | (1 << vs->wsw->currentwspc->number);
		ChangeOccupation(tw, newoccupation);
	}

	/* If we get here, WarpUnmapped is set, so map it if we need to */
	if(! tw->mapped) {
		DeIconify(tw);
	}

	/* And go */
	WarpToWindow(tw, Scr->RaiseOnWarp);
}


/* f.occupyall backend */
void
OccupyAll(TwmWindow *twm_win)
{
	IconMgr *save;

	if(!CanChangeOccupation(&twm_win)) {
		return;
	}

	/*
	 * Temporarily alter Scr->iconmgr because stuff down in
	 * ChangeOccupation winds up adding/removing bits, and that doesn't
	 * work right when we're setting all?  XXX Investigate further.
	 */
	save = Scr->iconmgr;
	Scr->iconmgr = Scr->workSpaceMgr.workSpaceList->iconmgr;
	ChangeOccupation(twm_win, fullOccupation);
	Scr->iconmgr = save;
}



/*
 ****************************************************************
 *
 * Pieces related to the Occupy window
 *
 ****************************************************************
 */

static ColorPair occupyButtoncp;

static char *ok_string         = "OK",
             *cancel_string     = "Cancel",
              *everywhere_string = "All";

/*
 * Create the Occupy window.  Part of startup process.
 *
 * Do not do the layout of the parts, only calculate the initial total
 * size. For the layout, call ResizeOccupyWindow() at the end.
 *
 * There is only one Occupy window (per Screen), it is reparented to each
 * virtual screen as needed.
 */
void
CreateOccupyWindow(void)
{
	int           width; // Caculated and altered, unlike most others
	int           Dummy = 1;
	TwmWindow     *tmp_win;
	/* Shorthands for the Occupy window */
	OccupyWindow  *occwin = Scr->workSpaceMgr.occupyWindow;
	Window        w; // occwin->w
	/* Misc other shorthands */
	const int lines   = Scr->workSpaceMgr.lines;
	const int columns = Scr->workSpaceMgr.columns;
	const int bwidth  = Scr->vScreenList->wsw->bwidth;
	const int bheight = Scr->vScreenList->wsw->bheight;
	const int vspace = occwin->vspace;
	const int hspace = occwin->hspace;
	const int height = ((bheight + vspace) * lines) + bheight + (2 * vspace);

	/* Struct embedded in [struct embedded in] Scr, so memory's waiting */

	/* There isn't anything we should do without workspaces... */
	if(!Scr->workSpaceManagerActive) {
		return;
	}

	/* Initialize font and colorpair bits */
	occwin->font     = Scr->IconManagerFont;
	occwin->cp       = Scr->IconManagerC;
#ifdef COLOR_BLIND_USER
	occwin->cp.shadc = Scr->White;
	occwin->cp.shadd = Scr->Black;
#else
	if(!Scr->BeNiceToColormap) {
		GetShadeColors(&occwin->cp);
	}
#endif

	/* We already know that these should be too */
	occwin->lines   = lines;
	occwin->columns = columns;


	/*
	 * Work out the necessary size of the OK/Cancel/All buttons at the
	 * bottom.
	 */
	{
		XRectangle inc_rect;
		XRectangle logical_rect;
		MyFont font = occwin->font;
		int bbwidth;  // Bottom button width
		/* Window min width based on bottom vs. workspace btns */
		int bb_width, ws_width;

		/* Buttons gotta be as wide as the biggest of the three strings */
		XmbTextExtents(font.font_set, ok_string, strlen(ok_string),
		               &inc_rect, &logical_rect);
		bbwidth = logical_rect.width;

		XmbTextExtents(font.font_set, cancel_string, strlen(cancel_string),
		               &inc_rect, &logical_rect);
		bbwidth = MAX(bbwidth, logical_rect.width);

		XmbTextExtents(font.font_set, everywhere_string,
		               strlen(everywhere_string),
		               &inc_rect, &logical_rect);
		bbwidth = MAX(bbwidth, logical_rect.width);

		/* Plus the padding width */
		bbwidth += hspace;

		/*
		 * So, the final width of those bottom buttons is that, plus the
		 * 3d button look extra on both sides, plus a little extra.  I
		 * guess that extra + 2 is similar to TitlePadding or
		 * ButtonIndent on titlebars, but we don't have a config param
		 * for it on the workspace manager (which is the config used for
		 * the occupy window), so leave it as a magic constant for now.
		 */
		occwin->owidth = bbwidth + 2 * Scr->WMgrButtonShadowDepth + 2;

		/*
		 * The whole thing has to be at least triple the min width of
		 * those bottom buttons, since there are three of them.  The
		 * layout is "hspace button hspace button [...] hspace", to pad
		 * between and on both sides.
		 */
		bb_width = 3 * (bbwidth + hspace) + hspace;

		/*
		 * It also has to be the width of our per-WS buttons.  Per-ws
		 * buttons are sized the same as in the button-state WSM, and
		 * then we add the padding to them as above.
		 */
		ws_width = columns * (bwidth + hspace) + hspace;

		/* So the window has to be as wide as the wider of those */
		width = MAX(bb_width, ws_width);
	}


	/* Now we know the size, so make the window */
	w = occwin->w = XCreateSimpleWindow(dpy, Scr->Root, 0, 0, width, height,
	                                    1, Scr->Black, occwin->cp.back);

	/* Take those base sizes as a minimum */
	occwin->minwidth  = width;
	occwin->minheight = height;


	/*
	 * Make subwindows as buttons for the workspaces.  They're laid out
	 * in a grid mirroring the workspace manager's.
	 */
	{
		int i = 0, j = 0;
		WorkSpace *ws;

		occwin->obuttonw = calloc(Scr->workSpaceMgr.count, sizeof(Window));

		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			int idx = (j * columns) + i;

			/*
			 * Make and map.  Note that we're not setting the size or
			 * location at all here; ResizeOccupyWindow() does all that.
			 * We just make 'em.
			 */
			occwin->obuttonw[idx] = XCreateSimpleWindow(dpy, w,
			                        Dummy /* x */,
			                        Dummy /* y */,
			                        Dummy /* width */,
			                        Dummy /* height */,
			                        0, Scr->Black, ws->cp.back);
			XMapWindow(dpy, occwin->obuttonw[idx]);

			/* Inc around to the next location */
			i++;
			if(i == columns) {
				i = 0;
				j++;
			}
		}
	}


	/*
	 * Now start putting together the OK/Cancel/All buttons
	 */

	/* Background for them is hardcoded */
	GetColor(Scr->Monochrome, &(occupyButtoncp.back), "gray50");

	/* Foreground (not used here) is too */
	occupyButtoncp.fore = Scr->White;

	/* Override (probably historical */
	if(!Scr->BeNiceToColormap) {
		GetShadeColors(&occupyButtoncp);
	}

	/* Make 'em */
	{
		Window tw;

		tw = XCreateSimpleWindow(dpy, w, Dummy, Dummy, Dummy, Dummy, 0,
		                         Scr->Black, occupyButtoncp.back);
		XMapWindow(dpy, tw);
		occwin->OK = tw;

		tw = XCreateSimpleWindow(dpy, w, Dummy, Dummy, Dummy, Dummy, 0,
		                         Scr->Black, occupyButtoncp.back);
		XMapWindow(dpy, tw);
		occwin->cancel = tw;

		tw = XCreateSimpleWindow(dpy, w, Dummy, Dummy, Dummy, Dummy, 0,
		                         Scr->Black, occupyButtoncp.back);
		XMapWindow(dpy, tw);
		occwin->allworkspc = tw;
	}


	/* Setup various window properties */
	{
		XSizeHints sizehints;
		XWMHints wmhints;

		sizehints.flags       = PBaseSize | PMinSize | PMaxSize;
		sizehints.min_width   = width;
		sizehints.min_height  = height;
		sizehints.base_width  = width;
		sizehints.base_height = height;
		sizehints.max_width   = width;
		sizehints.max_height  = height;

		wmhints.flags         = InputHint | StateHint;
		wmhints.input         = True;
		wmhints.initial_state = NormalState;

		XmbSetWMProperties(dpy, w, occwin->name, occwin->icon_name,
		                   NULL, 0, &sizehints, &wmhints, NULL);
	}


	/*
	 * Create the TwmWindow wrapping around it, with decorations etc.  We
	 * do this so early in startup that we're not listening for window
	 * creation events yet.
	 */
	tmp_win = AddWindow(w, AWT_OCCUPY, Scr->iconmgr, Scr->currentvs);
	if(! tmp_win) {
		fprintf(stderr, "cannot create occupy window, exiting...\n");
		exit(1);
	}
	tmp_win->vs = NULL;
	tmp_win->occupation = 0;

	/* tmp_win is more convenient the rest of the func, but put in place */
	occwin->twm_win = tmp_win;


	/*
	 * Setup the window to have a button-pushing cursor and listen for
	 * clicks.
	 */
	{
		unsigned long attrmask;
		XSetWindowAttributes attr;
		XWindowAttributes wattr;

		attr.cursor = Scr->ButtonCursor;
		attrmask = CWCursor;
		XChangeWindowAttributes(dpy, w, attrmask, &attr);

		XGetWindowAttributes(dpy, w, &wattr);
		attrmask = wattr.your_event_mask | KeyPressMask | KeyReleaseMask
		           | ExposureMask;
		XSelectInput(dpy, w, attrmask);
	}


	/*
	 * Now for each of the buttons (workspaces + OK/Cancel/All), we mark
	 * them as listening to click and exposure events.  We also stash
	 * away the screen and wrapping TwmWindow in contexts so other code
	 * can dredge them up.
	 */
#define EVT (ButtonPressMask | ButtonReleaseMask | ExposureMask)
#define BTN_IPT_CTX(win) \
        XSelectInput(dpy, (win), EVT); \
        XSaveContext(dpy, (win), TwmContext, (XPointer) tmp_win); \
        XSaveContext(dpy, (win), ScreenContext, (XPointer) Scr);

	for(WorkSpace *ws = Scr->workSpaceMgr.workSpaceList
	                    ; ws != NULL ; ws = ws->next) {
		BTN_IPT_CTX(occwin->obuttonw[ws->number]);
	}

	BTN_IPT_CTX(occwin->OK);
	BTN_IPT_CTX(occwin->cancel);
	BTN_IPT_CTX(occwin->allworkspc);

#undef BTN_IPT_CTX
#undef EVT


	/* Mark that we're not mapped */
	SetMapStateProp(tmp_win, WithdrawnState);

	/* Now call that func that sizes all the buttons */
	ResizeOccupyWindow(tmp_win);
}


/*
 * Slightly misleading name: layout the internals of the Occupy window
 * based on its current size.  That does happen when it's resized, but
 * also when it's initially created.  I guess you could call "creation" a
 * resize of a sort...
 */
void
ResizeOccupyWindow(TwmWindow *win)
{
	int        bwidth, bheight, owidth, oheight;
	int        hspace, vspace;
	int        lines, columns;
	int        neww, newh;
	WorkSpace  *ws;
	int        i, j, x, y;
	OccupyWindow *occwin = Scr->workSpaceMgr.occupyWindow;

	/* Floor at the original size */
	neww = MAX(win->attr.width,  occwin->minwidth);
	newh = MAX(win->attr.height, occwin->minheight);
	if(occwin->width == neww && occwin->height == newh) {
		return;
	}

	/* Space between WS buttons.  From WMgr{Horiz,Vert}ButtonIndent. */
	hspace  = occwin->hspace;
	vspace  = occwin->vspace;

	/* Lines/cols in the layout.  Same as WorkspaceManager's */
	lines   = Scr->workSpaceMgr.lines;
	columns = Scr->workSpaceMgr.columns;

	/* Width/height of each button, based on the above and window size */
	bwidth  = (neww -  columns    * hspace) / columns;
	bheight = (newh - (lines + 2) * vspace) / (lines + 1);

	/* Width/height of the OK/Cancel/All buttons */
	owidth  = occwin->owidth;
	oheight = bheight;


	/*
	 * Lay out the workspace buttons
	 */
	i = 0;
	j = 0;
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		XMoveResizeWindow(dpy, occwin->obuttonw [j * columns + i],
		                  i * (bwidth  + hspace) + (hspace / 2),
		                  j * (bheight + vspace) + (vspace / 2),
		                  bwidth, bheight);
		i++;
		if(i == columns) {
			i = 0;
			j++;
		}
	}


	/*
	 * Now the action buttons
	 */
	hspace = (neww - 3 * owidth) / 4;  // Padding between
	x = hspace;
	y = ((bheight + vspace) * lines) + ((3 * vspace) / 2);
	XMoveResizeWindow(dpy, occwin->OK, x, y, owidth, oheight);
	x += owidth + hspace;
	XMoveResizeWindow(dpy, occwin->cancel, x, y, owidth, oheight);
	x += owidth + hspace;
	XMoveResizeWindow(dpy, occwin->allworkspc, x, y, owidth, oheight);


	/* Save all those dimensions we figured */
	occwin->width   = neww;
	occwin->height  = newh;
	occwin->bwidth  = bwidth;
	occwin->bheight = bheight;
	occwin->owidth  = owidth;

	/* Don't need to repaint it; it'll get expose events */
}


/*
 * Draw the window when we need to (e.g., on expose)
 */
void
PaintOccupyWindow(void)
{
	WorkSpace    *ws;
	OccupyWindow *occwin;
	int          width, height;

	occwin = Scr->workSpaceMgr.occupyWindow;
	width  = occwin->width;
	height = occwin->height;

	Draw3DBorder(occwin->w, 0, 0, width, height, 2, occwin->cp, off, true, false);

	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		Window bw = occwin->obuttonw [ws->number];
		ButtonState bs = (occwin->tmpOccupation & (1 << ws->number)) ? on : off;

		PaintWsButton(OCCUPYWINDOW, NULL, bw, ws->label, ws->cp, bs);
	}
	PaintWsButton(OCCUPYBUTTON, NULL, occwin->OK,         ok_string,
	              occupyButtoncp, off);
	PaintWsButton(OCCUPYBUTTON, NULL, occwin->cancel,     cancel_string,
	              occupyButtoncp, off);
	PaintWsButton(OCCUPYBUTTON, NULL, occwin->allworkspc, everywhere_string,
	              occupyButtoncp, off);
}


/*
 * Somebody clicked in the Occupy window
 */
void
OccupyHandleButtonEvent(XEvent *event)
{
	WorkSpace    *ws;
	OccupyWindow *occupyW;
	Window       buttonW;

	/*
	 * Doesn't make sense that this can even happen if there are no
	 * workspaces...
	 */
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	/* ... or if there's no Occupy window up for anything */
	if(occupyWin == NULL) {
		return;
	}

	/* Which sub-window (button) was clicked */
	buttonW = event->xbutton.window;
	if(buttonW == 0) {
		return;        /* icon */
	}

	/* Grab onto the pointer for the duration of our action */
	XGrabPointer(dpy, Scr->Root, True,
	             ButtonPressMask | ButtonReleaseMask,
	             GrabModeAsync, GrabModeAsync,
	             Scr->Root, None, CurrentTime);

	/* Find the workspace button that was clicked */
	occupyW = Scr->workSpaceMgr.occupyWindow;
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(occupyW->obuttonw [ws->number] == buttonW) {
			break;
		}
	}

	if(ws != NULL) {
		/* If one was, toggle it */
		int mask = 1 << ws->number;
		ButtonState bs = (occupyW->tmpOccupation & mask) ? off : on;

		PaintWsButton(OCCUPYWINDOW, NULL, occupyW->obuttonw [ws->number],
		              ws->label, ws->cp, bs);
		occupyW->tmpOccupation ^= mask;
	}
	else if(buttonW == occupyW->OK) {
		/* Else if we clicked OK, set things and close the window */
		if(occupyW->tmpOccupation == 0) {
			return;
		}
		ChangeOccupation(occupyWin, occupyW->tmpOccupation);
		XUnmapWindow(dpy, occupyW->twm_win->frame);
		occupyW->twm_win->mapped = false;
		occupyW->twm_win->occupation = 0;
		occupyWin = NULL;
		XSync(dpy, 0);
	}
	else if(buttonW == occupyW->cancel) {
		/* Or cancel, do nothing and close the window */
		XUnmapWindow(dpy, occupyW->twm_win->frame);
		occupyW->twm_win->mapped = false;
		occupyW->twm_win->occupation = 0;
		occupyWin = NULL;
		XSync(dpy, 0);
	}
	else if(buttonW == occupyW->allworkspc) {
		/* Or All, set 'em all */
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			PaintWsButton(OCCUPYWINDOW, NULL, occupyW->obuttonw [ws->number],
			              ws->label, ws->cp, on);
		}
		occupyW->tmpOccupation = fullOccupation;
	}

	/* Release the pointer, if ??? */
	if(ButtonPressed == -1) {
		XUngrabPointer(dpy, CurrentTime);
	}
}


/*
 * f.occupy backend - pop up Occupy control for some window
 */
void
Occupy(TwmWindow *twm_win)
{
	int          x, y;
	unsigned int width, height;
	Window       w;
	struct OccupyWindow    *occupyWindow;
	TwmWindow *occupy_twm;

	/* Don't pop up on stuff we can't change */
	if(!CanChangeOccupation(&twm_win)) {
		return;
	}

	/* Grab our one screen-wide f.occupy window */
	occupyWindow = Scr->workSpaceMgr.occupyWindow;
	occupyWindow->tmpOccupation = twm_win->occupation;
	w = occupyWindow->w;

	/* Figure where to put it so it's centered on the cursor */
	XGetGeometry(dpy, w, &JunkRoot, &JunkX, &JunkY, &width, &height,
	             &JunkBW, &JunkDepth);
	XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkRoot, &JunkX, &JunkY,
	              &x, &y, &JunkMask);

	occupy_twm = occupyWindow->twm_win;
	occupy_twm->occupation = twm_win->occupation;

	width += 2 * (occupy_twm->frame_bw3D + occupy_twm->frame_bw);
	height += 2 * (occupy_twm->frame_bw3D + occupy_twm->frame_bw);
	x -= (width  / 2);
	y -= (height / 2);

	/* Clip to screen */
	ConstrainByLayout(Scr->BorderedLayout, -1, &x, width, &y, height);

	/* Move the occupy window to where it should be */
	if(occupy_twm->parent_vs != twm_win->parent_vs) {
		occupy_twm->vs = twm_win->parent_vs;
		occupy_twm->frame_x = x;
		occupy_twm->frame_y = y;
		/*
		 * XXX Should this be using DisplayWin() like everything else,
		 * rather than manually grubbing beneath it?
		 */
		ReparentFrameAndIcon(occupy_twm);
	}
	else {
		XMoveWindow(dpy, occupyWindow->twm_win->frame, x, y);
	}

	/* And show it */
	SetMapStateProp(occupy_twm, NormalState);
	XMapWindow(dpy, occupyWindow->w);
	XMapWindow(dpy, occupy_twm->frame);

	/* XXX Must be a better way to express "all the way on top" */
	OtpSetPriority(occupy_twm, WinWin, 0, Above);

	/* Mark it shown, and stash what window we're showing it for */
	occupyWindow->twm_win->mapped = true;
	occupyWin = twm_win;
}




/*
 ****************************************************************
 *
 * Backend and misc
 *
 ****************************************************************
 */


/*
 * The actual meat of occupation-changing; [re-]set the occupation for
 * the window.  This is the func that actually sets and saves the new
 * occupation, moves the window where it should be, etc.  Should maybe be
 * called something more like "SetOccupation()".
 */
void
ChangeOccupation(TwmWindow *tmp_win, int newoccupation)
{
	TwmWindow *t;
	WorkSpace *ws;
	int oldoccupation;
	int changedoccupation;

	if((newoccupation == 0)
	                || (newoccupation == tmp_win->occupation)) {
		/*
		 * occupation=0 we interpret as "leave it alone".  == current,
		 * ditto.  Go ahead and re-set the WM_OCCUPATION property though,
		 * in case it's been broken by another client.
		 */
		char *namelist;
		int  len;
		long eventMask;

		/* Mask out the PropertyChange events while we change the prop */
		eventMask = mask_out_event(tmp_win->w, PropertyChangeMask);

		len = GetPropertyFromMask(tmp_win->occupation, &namelist);
		XChangeProperty(dpy, tmp_win->w, XA_WM_OCCUPATION, XA_STRING, 8,
		                PropModeReplace, (unsigned char *) namelist, len);
		free(namelist);
#ifdef EWMH
		EwmhSet_NET_WM_DESKTOP(tmp_win);
#endif

		/* Reset event mask */
		restore_mask(tmp_win->w, eventMask);
		return;
	}

	/*
	 * OK, there's something to change.  Stash the current state.
	 */
	oldoccupation = tmp_win->occupation;

	/*
	 * Add it to IconManager in the new WS[en], remove from old.  We have
	 * to do the rather odd dance because AddIconManager() loops through
	 * workspaces, and will add it to any workspaces it occupies (even if
	 * it's already there).  RemoveIconManager() goes over the window's
	 * list of what icon managers it's on and removes it from any that
	 * don't match the current occupation, so it can just be told "here's
	 * where I should be".
	 */
	tmp_win->occupation = newoccupation & ~oldoccupation;
	AddIconManager(tmp_win);
	tmp_win->occupation = newoccupation;
	RemoveIconManager(tmp_win);

	/* If it shouldn't be "here", vanish it */
	if(tmp_win->vs && !OCCUPY(tmp_win, tmp_win->vs->wsw->currentwspc)) {
		Vanish(tmp_win->vs, tmp_win);
	}

	/*
	 * Try to find an(other) virtual screen which shows a workspace
	 * where the window has occupation, so that the window can be shown
	 * there now.
	 */
	if(!tmp_win->vs) {
		VirtualScreen *vs;
		for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
			if(OCCUPY(tmp_win, vs->wsw->currentwspc)) {
				DisplayWin(vs, tmp_win);
				break;
			}
		}
	}

	/*
	 * Loop over workspaces.  Find the first one that it used to be in.
	 * If it's not there anymore, take it out of the WindowRegion there
	 * (RWFR() silently returns if we're not using WindowRegion's), and
	 * add it the WindowRegion in the first WS it now occupies.
	 *
	 * XXX I'm not sure this is entirely sensible; it seems like just
	 * unconditionally Remove/Place'ing would have the same effect?
	 */
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		int mask = 1 << ws->number;
		if(oldoccupation & mask) {
			if(!(newoccupation & mask)) {
				int final_x, final_y;
				RemoveWindowFromRegion(tmp_win);
				if(PlaceWindowInRegion(tmp_win, &final_x, &final_y)) {
					XMoveWindow(dpy, tmp_win->frame, final_x, final_y);
				}
			}
			break;
		}
	}

	/* Now set the WM_OCCUPATION property */
	{
		char *namelist;
		int  len;
		long eventMask;

		eventMask = mask_out_event(tmp_win->w, PropertyChangeMask);

		len = GetPropertyFromMask(newoccupation, &namelist);
		XChangeProperty(dpy, tmp_win->w, XA_WM_OCCUPATION, XA_STRING, 8,
		                PropModeReplace, (unsigned char *) namelist, len);
		free(namelist);
#ifdef EWMH
		EwmhSet_NET_WM_DESKTOP(tmp_win);
#endif

		restore_mask(tmp_win->w, eventMask);
	}


	/*
	 * Handle showing it up in the workspace map in the appropriate
	 * places.
	 *
	 * Note that this whole block messes with the {new,old}occupation
	 * vars.  That's "safe" because they're no longer used for their
	 * original purposes, only for the WSmap changes, but it's still
	 * kinda fugly.  Change to local vars at the drop of a hat with later
	 * changes...
	 */
	if(!WMapWindowMayBeAdded(tmp_win)) {
		/* Not showing in the map, so pretend it's nowhere */
		newoccupation = 0;
	}
	if(Scr->workSpaceMgr.noshowoccupyall) {
		/*
		 * Don't show OccupyAll.  Note that this means both OccupyAll
		 * window, AND windows manually set to occupy everything.  We
		 * don't have to adjust newoccupation, because the above
		 * conditional would have caught it, so we only need to edit old.
		 */
		if(oldoccupation == fullOccupation) {
			oldoccupation = 0;
		}
	}

	/* Flip the ones that need flipping */
	changedoccupation = oldoccupation ^ newoccupation;
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		int mask = 1 << ws->number;
		if(changedoccupation & mask) {
			if(newoccupation & mask) {
				/* Add to WS */
				WMapAddWindowToWorkspace(tmp_win, ws);
			}
			else {
				/*
				 * Remove from WS.  We have to take it out of saved focus
				 * if it were there.  Maybe there are other places we
				 * might need to remove it from (warpring?)?
				 */
				WMapRemoveWindowFromWorkspace(tmp_win, ws);
				if(Scr->SaveWorkspaceFocus && ws->save_focus == tmp_win) {
					ws->save_focus = NULL;
				}
			}
		}
	}


	/*
	 * If transients don't have their own occupation, find any transients
	 * of this window and move them with it.
	 */
	if(! Scr->TransientHasOccupation) {
		for(t = Scr->FirstWindow; t != NULL; t = t->next) {
			if(t != tmp_win &&
			                ((t->istransient && t->transientfor == tmp_win->w) ||
			                 t->group == tmp_win->w)) {
				ChangeOccupation(t, tmp_win->occupation);
			}
		}
	}

	/* All done */
	return;
}


/*
 * There are various reasons you might not be able to change the
 * occupation of a window (either due to attributes of it, or the state
 * of your session/WM), so provide a function to check them all when we
 * try a change.
 *
 * Note that this is _not_ called from ChangeOccupation(); only from
 * other things that wrap it.  Since CO() gets called from states where
 * this would [falsely] fail, it would be a bad idea to put it there.
 */
static bool
CanChangeOccupation(TwmWindow **twm_winp)
{
	TwmWindow *twm_win;

	/* No workspaces config'd?  Changing is nonsensical. */
	if(!Scr->workSpaceManagerActive) {
		return false;
	}

	/*
	 * f.occupy window up?  Can't change in the middle of changing.
	 * Though if it's not mapped, still pull it up, else iconifying the
	 * occupy window breaks it forever.
	 */
	if(occupyWin != NULL && Scr->workSpaceMgr.occupyWindow->twm_win->mapped) {
		return false;
	}

	/* XXX Can we jut do this in the init?  Check all callers. */
	twm_win = *twm_winp;

	/* Don't change occupation of icon managers */
	if(twm_win->isiconmgr) {
		return false;
	}

	/* XXX Should check iswspmgr here too? */

	/*
	 * If transients don't have their own occupation, check
	 * transient/group bits.
	 */
	if(!Scr->TransientHasOccupation) {
		if(twm_win->istransient) {
			return false;
		}
		if(twm_win->group != (Window) 0 && twm_win->group != twm_win->w) {
			/*
			 * When trying to modify a group member window,
			 * operate on the group leader instead
			 * (and thereby on all group member windows as well).
			 * If we can't find the group leader, pretend it isn't set.
			 */
			twm_win = GetTwmWindow(twm_win->group);
			if(!twm_win) {
				return true;
			}
			*twm_winp = twm_win;
		}
	}

	/* Sure, go ahead, change it */
	return true;
}


/*
 * Add a client name to a list determining which workspaces it will
 * occupy.  Used in handling the Occupy { } block in config file.
 */
bool
AddToClientsList(char *workspace, char *client)
{
	WorkSpace *ws;

	/* "all" is a magic workspace value which makes it occupy anywhere */
	if(strcmp(workspace, "all") == 0) {
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			AddToList(&ws->clientlist, client, "");
		}
		return true;
	}

	/* If prefixed with "ws:", strip the prefix and lookup by WS name */
	if(strncmp(workspace, "ws:", 3) == 0) {
		if((ws = GetWorkspace(workspace + 3)) != NULL) {
			AddToList(&ws->clientlist, client, "");
			return true;
		}
	}

	/* Else find that named workspace and all this to it */
	if((ws = GetWorkspace(workspace)) != NULL) {
		AddToList(&ws->clientlist, client, "");
		return true;
	}

	/* Couldn't figure where to put it */
	return false;
}


/*
 * Turn a ctwm.workspace resource string into an occupation mask.  n.b.;
 * this changes the 'res' arg in-place.
 */
static int
GetMaskFromResource(TwmWindow *win, char *res)
{
	WorkSpace *ws;
	int       mask;
	enum { O_SET, O_ADD, O_REM } mode;
	char *wrkSpcName, *tokst;

	/*
	 * This can set the occupation to a specific set of workspaces ("ws1
	 * ws3"), add to the set it woudl have otherwise ("+ws1 ws3"), or
	 * remove from the set it would otherwise ("-ws1 ws3").  The +/-
	 * apply to the whole expression, not to the individual entries in
	 * it.  So first, figure out what we're doing.
	 */
	mode = O_SET;
	if(*res == '+') {
		mode = O_ADD;
		res++;
	}
	else if(*res == '-') {
		mode = O_REM;
		res++;
	}

	/*
	 * Walk through the string adding the workspaces specified into the
	 * mask of what we're doing.
	 */
	mask = 0;
	for(wrkSpcName = strtok_r(res, " ", &tokst) ; wrkSpcName
	                ; wrkSpcName = strtok_r(NULL, " ", &tokst)) {
		if(strcmp(wrkSpcName, "all") == 0) {
			mask = fullOccupation;
			break;
		}
		if(strcmp(wrkSpcName, "current") == 0) {
			VirtualScreen *vs = Scr->currentvs;
			if(vs) {
				mask |= (1 << vs->wsw->currentwspc->number);
			}
			continue;
		}

		ws = GetWorkspace(wrkSpcName);
		if(ws != NULL) {
			mask |= (1 << ws->number);
		}
		else {
			fprintf(stderr, "unknown workspace : %s\n", wrkSpcName);
		}
	}

	/*
	 * And return that mask, with necessary alterations depending on +/-
	 * specified.
	 */
	switch(mode) {
		case O_SET:
			return (mask);
		case O_ADD:
			return (mask | win->occupation);
		case O_REM:
			return (win->occupation & ~mask);
	}

	/* Can't get here */
	fprintf(stderr, "%s(): Unreachable.\n", __func__);
	return 0;
}


/*
 * Turns a \0-separated buffer of workspace names into an occupation
 * bitmask.
 */
unsigned int
GetMaskFromProperty(unsigned char *_prop, unsigned long len)
{
	char         wrkSpcName[256];
	WorkSpace    *ws;
	unsigned int mask;
	int          l;
	char         *prop;

	mask = 0;
	l = 0;
	prop = (char *) _prop;
	while(l < len) {
		/* If you have WS names longer than 256 chars, that's just Too Bad */
		safe_strncpy(wrkSpcName, prop, 256);
		l    += strlen(prop) + 1;
		prop += strlen(prop) + 1;
		if(strcmp(wrkSpcName, "all") == 0) {
			mask = fullOccupation;
			break;
		}

		ws = GetWorkspace(wrkSpcName);
		if(ws != NULL) {
			mask |= (1 << ws->number);
		}
		else {
			fprintf(stderr, "unknown workspace : %s\n", wrkSpcName);
		}
	}

#if 0
	{
		char *dbs = mk_nullsep_string((char *)_prop, len);
		fprintf(stderr, "%s('%s') -> 0x%x\n", __func__, dbs, mask);
		free(dbs);
	}
#endif

	return (mask);
}


/*
 * Turns an occupation mask into a \0-separated buffer (not really a
 * string) of the workspace names.
 */
int
GetPropertyFromMask(unsigned int mask, char **prop)
{
	WorkSpace *ws;
	int       len;
	char      *wss[MAXWORKSPACE];
	int       i;

	/* If it's everything, just say 'all' */
	if(mask == fullOccupation) {
		*prop = strdup("all");
		return 3;
	}

	/* Stash up pointers to all the labels for WSen it's in */
	memset(wss, 0, sizeof(wss));
	i = 0;
	len = 0;
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(mask & (1 << ws->number)) {
			wss[i++] = ws->label;
			len += strlen(ws->label) + 1;
		}
	}

	/* Assemble them into \0-separated string */
	*prop = malloc(len);
	len = 0;
	for(i = 0 ; wss[i] != NULL ; i++) {
		strcpy((*prop + len), wss[i]);
		len += strlen(wss[i]) + 1; // Skip past \0
	}

#if 0
	{
		char *dbs = mk_nullsep_string(*prop, len);
		fprintf(stderr, "%s(0x%x) -> %d:'%s'\n", __func__, mask, len, dbs);
		free(dbs);
	}
#endif

	return len;
}


/*
 * Generate a printable variant of the null-separated strings we use for
 * stashing in XA_WM_OCCUPATION.  Used for debugging
 * Get{Property,Mask}From{Mask,Property}().
 */
#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
#endif
static char *
mk_nullsep_string(const char *prop, int len)
{
	char *dbs;
	int i, j;

	/*
	 * '\0' => "\\0" means we need longer than input; *2 is overkill,
	 * but always sufficient, and it's cheap.
	 */
	dbs = malloc(len * 2);
	i = j = 0;
	while(i < len) {
		size_t slen = strlen(prop + i);

		strcpy(dbs + j, (prop + i));
		i += slen + 1;
		strcpy(dbs + j + slen, "\\0");
		j += slen + 2;
	}

	return dbs;
}
#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
