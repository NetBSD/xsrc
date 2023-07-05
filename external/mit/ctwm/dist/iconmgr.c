/*
 * Copyright 1989 Massachusetts Institute of Technology
 * Copyright 1992 Claude Lecommandeur.
 */

/***********************************************************************
 *
 * $XConsortium: iconmgr.c,v 1.48 91/09/10 15:27:07 dave Exp $
 *
 * Icon Manager routines
 *
 * 09-Mar-89 Tom LaStrange              File Created
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
#include <string.h>
#include <strings.h>

#include <X11/Xatom.h>

#include "util.h"
#include "iconmgr.h"
#include "icons_builtin.h"
#include "screen.h"
#include "drawing.h"
#include "functions_defs.h"
#include "list.h"
#include "occupation.h"
#include "otp.h"
#include "add_window.h"
#include "gram.tab.h"
#include "vscreen.h"
#include "win_decorations.h"
#include "win_resize.h"
#include "win_utils.h"
#include "xparsegeometry.h"


/* Where we start drawing the name in the icon manager */
static int iconmgr_textx;

static WList *Active = NULL;
static WList *Current = NULL;
WList *DownIconManager = NULL;

/***********************************************************************
 *
 *  Procedure:
 *      CreateIconManagers - creat all the icon manager windows
 *              for this screen.
 *
 *  Returned Value:
 *      none
 *
 *  Inputs:
 *      none
 *
 ***********************************************************************
 */

void CreateIconManagers(void)
{
	WorkSpace    *ws;

	if(Scr->NoIconManagers) {
		return;
	}

	/*
	 * Move past the iconified icon to start the text.
	 * XXX Semi-arbitrary magic add'l padding, to deal with various inner
	 * positioning of the icon subwindow.  Be smarter (or at least
	 * clearer) about this...
	 */
	iconmgr_textx = im_iconified_icon_width + 11;
	if(Scr->use3Diconmanagers) {
		iconmgr_textx += Scr->IconManagerShadowDepth;
	}

	if(Scr->siconifyPm == None) {
		Scr->siconifyPm = Create2DIconManagerIcon();
	}

	// This loop is confusing.  The inner for() loops p over the ->next
	// elements in the list, which is all the iconmgr's in the workspace.
	// The outer for() loops q over the ->nextv (<-- extra 'v' on the
	// end), which is a link to the head of the iconmgr list for the
	// _next_ workspace.
	ws = Scr->workSpaceMgr.workSpaceList;
	for(IconMgr *q = Scr->iconmgr; q != NULL; q = q->nextv) {
		for(IconMgr *p = q; p != NULL; p = p->next) {
			int gx, gy;
			char imname[100];
			int mask;
			int gravity;
			int bw;
			Pixel background;

			snprintf(imname, sizeof(imname), "%s Icon Manager", p->name);

			if(!p->geometry || !strlen(p->geometry)) {
				p->geometry = "+0+0";
			}
			mask = RLayoutXParseGeometry(Scr->Layout, p->geometry,
			                             &gx, &gy,
			                             (unsigned int *) &p->width, (unsigned int *)&p->height);

			bw = LookInList(Scr->NoBorder, imname, NULL) ? 0 :
			     (Scr->ThreeDBorderWidth ? Scr->ThreeDBorderWidth : Scr->BorderWidth);

			if(mask & XNegative) {
				gx += Scr->rootw - p->width - 2 * bw;
				gravity = (mask & YNegative) ? SouthEastGravity : NorthEastGravity;
			}
			else {
				gravity = (mask & YNegative) ? SouthWestGravity : NorthWestGravity;
			}
			if(mask & YNegative) {
				gy += Scr->rooth - p->height - 2 * bw;
			}

			background = Scr->IconManagerC.back;
			GetColorFromList(Scr->IconManagerBL, p->name, NULL,
			                 &background);

			if(p->width  < 1) {
				p->width  = 1;
			}
			if(p->height < 1) {
				p->height = 1;
			}
			p->w = XCreateSimpleWindow(dpy, Scr->Root,
			                           gx, gy, p->width, p->height, 1,
			                           Scr->Black, background);


			/* Scr->workSpaceMgr.activeWSPC = ws; */

			/* Setup various WM properties on the iconmgr's window */
			{
				char *icon_name;
				XWMHints wmhints;
				XClassHint clhints;

				if(p->icon_name) {
					icon_name = strdup(p->icon_name);
				}
				else {
					asprintf(&icon_name, "%s Icons", p->name);
				}

				wmhints.initial_state = NormalState;
				wmhints.input         = True;
				wmhints.flags         = InputHint | StateHint;

				clhints.res_name  = icon_name;
				clhints.res_class = "TwmIconManager";

				XmbSetWMProperties(dpy, p->w, imname, icon_name, NULL, 0, NULL,
				                   &wmhints, &clhints);
				free(icon_name);
			}


			p->twm_win = AddWindow(p->w, AWT_ICON_MANAGER, p, Scr->currentvs);

			// SetupOccupation() called from AddWindow() doesn't setup
			// occupation for icon managers, nor clear vs if occupation
			// lacks.  So make it occupy the one we're setting up, or the
			// 1st if we ran out somehow...
			if(ws) {
				p->twm_win->occupation = 1 << ws->number;

				// ConfigureWorkSpaceManager() ran before us, so we can
				// tell whether we're in the ws to reveal this IM.
				if(ws->number != Scr->currentvs->wsw->currentwspc->number) {
					p->twm_win->vs = NULL;
				}
			}
			else {
				p->twm_win->occupation = 1;
			}

#ifdef DEBUG_ICONMGR
			fprintf(stderr,
			        "CreateIconManagers: IconMgr %p: twm_win=%p win=0x%lx "
			        "name='%s' x=%d y=%d w=%d h=%d occupation=%x\n",
			        p, p->twm_win, p->twm_win->w, p->name,
			        gx, gy,  p->width, p->height, p->twm_win->occupation);
#endif

			{
				XSizeHints sizehints;

				sizehints.flags       = PWinGravity;
				sizehints.win_gravity = gravity;
				XSetWMSizeHints(dpy, p->w, &sizehints, XA_WM_NORMAL_HINTS);
			}

			p->twm_win->mapped = false;
			SetMapStateProp(p->twm_win, WithdrawnState);
			if(p->twm_win && (p->twm_win->wmhints->initial_state == IconicState)) {
				p->twm_win->isicon = true;
			}
			else if(!Scr->NoIconManagers && Scr->ShowIconManager) {
				p->twm_win->isicon = false;
			}
			else {
				p->twm_win->isicon = true;
			}
		}
		if(ws != NULL) {
			ws = ws->next;
		}
	}

	if(Scr->workSpaceManagerActive) {
		Scr->workSpaceMgr.workSpaceList->iconmgr = Scr->iconmgr;
	}


	/*
	 * Grab buttons/keystrokes for icon managers appropriately.
	 * Normally, this is done in AddWindow(), but it explicitly skips it
	 * for icon managers.  It's not at all clear why GrabButtons() would
	 * do so; I don't think it needs to.  GrabKeys() does do some looping
	 * over the Scr->iconmgr list at the end though, so it's possible we
	 * need to delay calling it until now when the list is all filled up.
	 * This needs further investigation; it may be that the special case
	 * and this code can be removed.  X-ref comments in add_window.c
	 * about it.
	 */
	for(IconMgr *q = Scr->iconmgr; q != NULL; q = q->nextv) {
		for(IconMgr *p = q; p != NULL; p = p->next) {
			GrabButtons(p->twm_win);
			GrabKeys(p->twm_win);
		}
	}

}

/***********************************************************************
 *
 *  Procedure:
 *      AllocateIconManager - allocate a new icon manager
 *
 *  Inputs:
 *      name    - the name of this icon manager
 *      icon_name - the name of the associated icon
 *      geom    - a geometry string to eventually parse
 *      columns - the number of columns this icon manager has
 *
 ***********************************************************************
 */

IconMgr *AllocateIconManager(char *name, char *icon_name, char *geom,
                             int columns)
{
	IconMgr *p;

#ifdef DEBUG_ICONMGR
	fprintf(stderr, "AllocateIconManager\n");
	fprintf(stderr, "  name=\"%s\" icon_name=\"%s\", geom=\"%s\", col=%d\n",
	        name, icon_name, geom, columns);
#endif

	if(Scr->NoIconManagers) {
		return NULL;
	}

	if(columns < 1) {
		columns = 1;
	}
	p = calloc(1, sizeof(IconMgr));
	p->name      = name;
	p->icon_name = icon_name;
	p->geometry  = geom;
	p->columns   = columns;
	p->scr       = Scr;
	p->width     = 150;
	p->height    = 10;

	if(Scr->iconmgr == NULL) {
		Scr->iconmgr = p;
		Scr->iconmgr->lasti = p;
	}
	else {
		Scr->iconmgr->lasti->next = p;
		p->prev = Scr->iconmgr->lasti;
		Scr->iconmgr->lasti = p;
	}
	return(p);
}


/*
 * Each workspace has its own [set of] icon manager[s].  The initial main
 * one was setup via AllocateIconManager() early in startup.  The others
 * were setup during parsing the config file.  Then this gets called late
 * in startup, after all the workspaces are setup, to copy them all into
 * each one.
 *
 * Note this is distinct from CreateIconManagers(); that creates and
 * draws the windows, this creates and connects up the data structures.
 */
void AllocateOtherIconManagers(void)
{
	IconMgr   *imfirst; // First IM on each workspace
	WorkSpace *ws;

	/* No defined workspaces?  Nothing to do. */
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	/* The first workspace just gets the ones we already have */
	ws = Scr->workSpaceMgr.workSpaceList;
	ws->iconmgr = Scr->iconmgr;

	/* From the second on, we start copying */
	imfirst = ws->iconmgr;
	for(ws = ws->next; ws != NULL; ws = ws->next) {
		IconMgr *ip, *previ, *p = NULL;

		/* Copy in the first iconmgr */
		ws->iconmgr  = malloc(sizeof(IconMgr));
		*ws->iconmgr = *imfirst;

		/*
		 * This first is now the nextv to the first in the previous WS,
		 * and we don't [yet] have a nextv of our own.
		 * */
		imfirst->nextv = ws->iconmgr;
		ws->iconmgr->nextv = NULL;

		/*
		 * Start from the second, and copy them each from the prior
		 * workspace we just went through.
		 * */
		previ = ws->iconmgr;
		for(ip = imfirst->next; ip != NULL; ip = ip->next) {
			/* Copy the base bits */
			p  = malloc(sizeof(IconMgr));
			*p = *ip;

			/* Link up the double-links, and there's no nextv [yet] */
			previ->next = p;
			p->prev     = previ;
			p->next     = NULL;
			p->nextv    = NULL;

			/* We're now the nextv to that one in the old workspace */
			ip->nextv  = p;

			/* And back around to the next one to copy into this WS */
			previ = p;
		}

		/* Each one has a pointer to the last IM in this WS, so save those */
		for(ip = ws->iconmgr; ip != NULL; ip = ip->next) {
			ip->lasti = p;
		}

		/*
		 * And back around to the next workspace, which works from those
		 * we made for this WS.  We go from imfirst rather than
		 * Scr->iconmgr so the ip->nextv rewrites are correct above; we
		 * have to fill them in on the next loop.
		 */
		imfirst = ws->iconmgr;
	}
}


/***********************************************************************
 *
 *  Procedure:
 *      MoveIconManager - move the pointer around in an icon manager
 *
 *  Inputs:
 *      dir     - one of the following:
 *                      F_FORWICONMGR   - forward in the window list
 *                      F_BACKICONMGR   - backward in the window list
 *                      F_UPICONMGR     - up one row
 *                      F_DOWNICONMGR   - down one row
 *                      F_LEFTICONMGR   - left one column
 *                      F_RIGHTICONMGR  - right one column
 *
 *  Special Considerations:
 *      none
 *
 ***********************************************************************
 */

void MoveIconManager(int dir)
{
	IconMgr *ip;
	WList *tmp = NULL;
	int cur_row, cur_col, new_row, new_col;
	int row_inc, col_inc;
	bool got_it;

	if(!Current) {
		return;
	}

	cur_row = Current->row;
	cur_col = Current->col;
	ip = Current->iconmgr;

	row_inc = 0;
	col_inc = 0;
	got_it = false;

	switch(dir) {
		case F_FORWICONMGR:
			if((tmp = Current->next) == NULL) {
				tmp = ip->first;
			}
			got_it = true;
			break;

		case F_BACKICONMGR:
			if((tmp = Current->prev) == NULL) {
				tmp = ip->last;
			}
			got_it = true;
			break;

		case F_UPICONMGR:
			row_inc = -1;
			break;

		case F_DOWNICONMGR:
			row_inc = 1;
			break;

		case F_LEFTICONMGR:
			col_inc = -1;
			break;

		case F_RIGHTICONMGR:
			col_inc = 1;
			break;
	}

	/* If got_it is false ast this point then we got a left, right,
	 * up, or down, command.  We will enter this loop until we find
	 * a window to warp to.
	 */
	new_row = cur_row;
	new_col = cur_col;

	while(!got_it) {
		new_row += row_inc;
		new_col += col_inc;
		if(new_row < 0) {
			new_row = ip->cur_rows - 1;
		}
		if(new_col < 0) {
			new_col = ip->cur_columns - 1;
		}
		if(new_row >= ip->cur_rows) {
			new_row = 0;
		}
		if(new_col >= ip->cur_columns) {
			new_col = 0;
		}

		/* Now let's go through the list to see if there is an entry with this
		 * new position
		 */
		for(tmp = ip->first; tmp != NULL; tmp = tmp->next) {
			if(tmp->row == new_row && tmp->col == new_col) {
				got_it = true;
				break;
			}
		}
	}

	if(!got_it) {
		fprintf(stderr,
		        "%s:  unable to find window (%d, %d) in icon manager\n",
		        ProgramName, new_row, new_col);
		return;
	}

	if(tmp == NULL) {
		return;
	}

	/* raise the frame so the icon manager is visible */
	if(ip->twm_win->mapped) {
		OtpRaise(ip->twm_win, WinWin);
		XWarpPointer(dpy, None, tmp->icon, 0, 0, 0, 0, 5, 5);
	}
	else {
		if(tmp->twm->title_height) {
			int tbx = Scr->TBInfo.titlex;
			int x = tmp->twm->highlightxr;
			XWarpPointer(dpy, None, tmp->twm->title_w, 0, 0, 0, 0,
			             tbx + (x - tbx) / 2,
			             Scr->TitleHeight / 4);
		}
		else {
			XWarpPointer(dpy, None, tmp->twm->w, 0, 0, 0, 0, 5, 5);
		}
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      MoveMappedIconManager - move the pointer around in an icon manager
 *
 *  Inputs:
 *      dir     - one of the following:
 *                      F_FORWMAPICONMGR        - forward in the window list
 *                      F_BACKMAPICONMGR        - backward in the window list
 *
 *  Special Considerations:
 *      none
 *
 ***********************************************************************
 */

void MoveMappedIconManager(int dir)
{
	IconMgr *ip;
	WList *tmp = NULL;
	WList *orig = NULL;
	bool got_it;

	if(!Current) {
		Current = Active;
	}
	if(!Current) {
		return;
	}

	ip = Current->iconmgr;

	got_it = false;
	tmp = Current;
	orig = Current;

	while(!got_it) {
		switch(dir) {
			case F_FORWMAPICONMGR:
				if((tmp = tmp->next) == NULL) {
					tmp = ip->first;
				}
				break;

			case F_BACKMAPICONMGR:
				if((tmp = tmp->prev) == NULL) {
					tmp = ip->last;
				}
				break;
		}
		if(tmp->twm->mapped) {
			got_it = true;
			break;
		}
		if(tmp == orig) {
			break;
		}
	}

	if(!got_it) {
		fprintf(stderr, "%s:  unable to find open window in icon manager\n",
		        ProgramName);
		return;
	}

	if(tmp == NULL) {
		return;
	}

	/* raise the frame so the icon manager is visible */
	if(ip->twm_win->mapped) {
		OtpRaise(ip->twm_win, WinWin);
		XWarpPointer(dpy, None, tmp->icon, 0, 0, 0, 0, 5, 5);
	}
	else {
		if(tmp->twm->title_height) {
			XWarpPointer(dpy, None, tmp->twm->title_w, 0, 0, 0, 0,
			             tmp->twm->title_width / 2,
			             Scr->TitleHeight / 4);
		}
		else {
			XWarpPointer(dpy, None, tmp->twm->w, 0, 0, 0, 0, 5, 5);
		}
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      JumpIconManager - jump from one icon manager to another,
 *              possibly even on another screen
 *
 *  Inputs:
 *      dir     - one of the following:
 *                      F_NEXTICONMGR   - go to the next icon manager
 *                      F_PREVICONMGR   - go to the previous one
 *
 ***********************************************************************
 */

void JumpIconManager(int dir)
{
	IconMgr *ip, *tmp_ip = NULL;
	bool got_it = false;
	ScreenInfo *sp;
	int screen;

	if(!Current) {
		return;
	}


#define ITER(i) (dir == F_NEXTICONMGR ? (i)->next : (i)->prev)
#define IPOFSP(sp) (dir == F_NEXTICONMGR ? sp->iconmgr : sp->iconmgr->lasti)
#define TEST(ip) if ((ip)->count != 0 && (ip)->twm_win->mapped) \
                 { got_it = true; break; }

	ip = Current->iconmgr;
	for(tmp_ip = ITER(ip); tmp_ip; tmp_ip = ITER(tmp_ip)) {
		TEST(tmp_ip);
	}

	if(!got_it) {
		int origscreen = ip->scr->screen;
		int inc = (dir == F_NEXTICONMGR ? 1 : -1);

		for(screen = origscreen + inc; ; screen += inc) {
			if(screen >= NumScreens) {
				screen = 0;
			}
			else if(screen < 0) {
				screen = NumScreens - 1;
			}

			sp = ScreenList[screen];
			if(sp) {
				for(tmp_ip = IPOFSP(sp); tmp_ip; tmp_ip = ITER(tmp_ip)) {
					TEST(tmp_ip);
				}
			}
			if(got_it || screen == origscreen) {
				break;
			}
		}
	}

#undef ITER
#undef IPOFSP
#undef TEST

	if(!got_it) {
		XBell(dpy, 0);
		return;
	}

	/* raise the frame so it is visible */
	OtpRaise(tmp_ip->twm_win, WinWin);
	if(tmp_ip->active) {
		XWarpPointer(dpy, None, tmp_ip->active->icon, 0, 0, 0, 0, 5, 5);
	}
	else {
		XWarpPointer(dpy, None, tmp_ip->w, 0, 0, 0, 0, 5, 5);
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      AddIconManager - add a window to an icon manager
 *
 *  Inputs:
 *      tmp_win - the TwmWindow structure
 *
 ***********************************************************************
 */

WList *AddIconManager(TwmWindow *tmp_win)
{
	WList *tmp, *old;
	IconMgr *ip;

	/* Some window types don't wind up in icon managers ever */
	if(tmp_win->isiconmgr || tmp_win->istransient || tmp_win->iswspmgr
	                || tmp_win->w == Scr->workSpaceMgr.occupyWindow->w) {
		return NULL;
	}

	/* Icon managers can be shut off wholesale in the config */
	if(Scr->NoIconManagers) {
		return NULL;
	}

	/* Config could declare not to IMify this type of window in two ways */
	if(LookInList(Scr->IconMgrNoShow, tmp_win->name, &tmp_win->class)) {
		return NULL;
	}
	if(Scr->IconManagerDontShow
	                && !LookInList(Scr->IconMgrShow, tmp_win->name, &tmp_win->class)) {
		return NULL;
	}

	/* Dredge up the appropriate IM */
	if((ip = (IconMgr *)LookInList(Scr->IconMgrs, tmp_win->name,
	                               &tmp_win->class)) == NULL) {
		if(Scr->workSpaceManagerActive) {
			ip = Scr->workSpaceMgr.workSpaceList->iconmgr;
		}
		else {
			ip = Scr->iconmgr;
		}
	}

	/* IM's exist in all workspaces, so loop through WSen */
	tmp = NULL;
	old = tmp_win->iconmanagerlist;
	while(ip != NULL) {
		int h;
		unsigned long valuemask;         /* mask for create windows */
		XSetWindowAttributes attributes; /* attributes for create windows */

		/* Is the window in this workspace? */
		if((tmp_win->occupation & ip->twm_win->occupation) == 0) {
			/* Nope, skip onward */
			ip = ip->nextv;
			continue;
		}

		/* Yep, create entry and stick it in */
		tmp = calloc(1, sizeof(WList));
		tmp->iconmgr = ip;
		tmp->twm = tmp_win;

		InsertInIconManager(ip, tmp, tmp_win);

		/* IM color settings, shared worldwide */
		tmp->cp.fore   = Scr->IconManagerC.fore;
		tmp->cp.back   = Scr->IconManagerC.back;
		tmp->highlight = Scr->IconManagerHighlight;

		GetColorFromList(Scr->IconManagerFL, tmp_win->name,
		                 &tmp_win->class, &tmp->cp.fore);
		GetColorFromList(Scr->IconManagerBL, tmp_win->name,
		                 &tmp_win->class, &tmp->cp.back);
		GetColorFromList(Scr->IconManagerHighlightL, tmp_win->name,
		                 &tmp_win->class, &tmp->highlight);

		/*
		 * If we're using 3d icon managers, each line item has its own
		 * icon; see comment on creation function for details.  With 2d
		 * icon managers, it's the same for all of them, so it's stored
		 * screen-wide.
		 */
		if(Scr->use3Diconmanagers) {
			if(!Scr->BeNiceToColormap) {
				GetShadeColors(&tmp->cp);
			}
			tmp->iconifypm = Create3DIconManagerIcon(tmp->cp);
		}

		/* Refigure the height of the whole IM */
		h = Scr->IconManagerFont.avg_height
		    + 2 * (ICON_MGR_OBORDER + ICON_MGR_OBORDER);
		if(h < (im_iconified_icon_height + 4)) {
			h = im_iconified_icon_height + 4;
		}

		ip->height = h * ip->count;
		tmp->me = ip->count;
		tmp->x = -1;
		tmp->y = -1;
		tmp->height = -1;
		tmp->width = -1;


		/* Make a window for this row in the IM */
		valuemask = (CWBackPixel | CWBorderPixel | CWEventMask | CWCursor);
		attributes.background_pixel = tmp->cp.back;
		attributes.border_pixel = tmp->cp.back;
		attributes.event_mask = (KeyPressMask | ButtonPressMask |
		                         ButtonReleaseMask | ExposureMask);
		if(Scr->IconManagerFocus) {
			attributes.event_mask |= (EnterWindowMask | LeaveWindowMask);
		}
		attributes.cursor = Scr->IconMgrCursor;
		tmp->w = XCreateWindow(dpy, ip->w, 0, 0, 1,
		                       h, 0,
		                       CopyFromParent, CopyFromParent,
		                       CopyFromParent,
		                       valuemask, &attributes);


		/* Setup the icon for it too */
		valuemask = (CWBackPixel | CWBorderPixel | CWEventMask | CWCursor);
		attributes.background_pixel = tmp->cp.back;
		attributes.border_pixel = Scr->Black;
		attributes.event_mask = (ButtonReleaseMask | ButtonPressMask
		                         | ExposureMask);
		attributes.cursor = Scr->ButtonCursor;
		/* The precise location will be set it in PackIconManager.  */
		tmp->icon = XCreateWindow(dpy, tmp->w, 0, 0,
		                          im_iconified_icon_width,
		                          im_iconified_icon_height,
		                          0, CopyFromParent,
		                          CopyFromParent,
		                          CopyFromParent,
		                          valuemask, &attributes);


		/* Bump housekeeping for the IM */
		ip->count += 1;
		PackIconManager(ip);
		if(Scr->WindowMask) {
			XRaiseWindow(dpy, Scr->WindowMask);
		}
		XMapWindow(dpy, tmp->w);

		XSaveContext(dpy, tmp->w, TwmContext, (XPointer) tmp_win);
		XSaveContext(dpy, tmp->w, ScreenContext, (XPointer) Scr);
		XSaveContext(dpy, tmp->icon, TwmContext, (XPointer) tmp_win);
		XSaveContext(dpy, tmp->icon, ScreenContext, (XPointer) Scr);

		if(!ip->twm_win->isicon) {
			if(visible(ip->twm_win)) {
				SetMapStateProp(ip->twm_win, NormalState);
				XMapWindow(dpy, ip->w);
				XMapWindow(dpy, ip->twm_win->frame);
			}
			ip->twm_win->mapped = true;
		}


		/*
		 * Stick this entry on the head of our list of "IM entries we
		 * created", and loop around to the next WS for this IM.
		 */
		tmp->nextv = old;
		old = tmp;
		ip = ip->nextv;
	}

	/* If we didn't create at least one thing, we're done here */
	if(tmp == NULL) {
		return NULL;
	}

	/* Stash where the window is IM-listed */
	tmp_win->iconmanagerlist = tmp;

	/* ??? */
	if(! visible(tmp->iconmgr->twm_win)) {
		old = tmp;
		tmp = tmp->nextv;
		while(tmp != NULL) {
			if(visible(tmp->iconmgr->twm_win)) {
				break;
			}
			old = tmp;
			tmp = tmp->nextv;
		}
		if(tmp != NULL) {
			old->nextv = tmp->nextv;
			tmp->nextv = tmp_win->iconmanagerlist;
			tmp_win->iconmanagerlist = tmp;
		}
	}

	/* Hand back the list places we added */
	return tmp_win->iconmanagerlist;
}

/***********************************************************************
 *
 *  Procedure:
 *      InsertInIconManager - put an allocated entry into an icon
 *              manager
 *
 *  Inputs:
 *      ip      - the icon manager pointer
 *      tmp     - the entry to insert
 *
 ***********************************************************************
 */

void InsertInIconManager(IconMgr *ip, WList *tmp, TwmWindow *tmp_win)
{
	WList *tmp1;
	bool added;

	added = false;
	if(ip->first == NULL) {
		ip->first = tmp;
		tmp->prev = NULL;
		ip->last = tmp;
		added = true;
	}
	else if(Scr->SortIconMgr) {
		for(tmp1 = ip->first; tmp1 != NULL; tmp1 = tmp1->next) {
			int compresult;
			if(Scr->CaseSensitive) {
				compresult = strcmp(tmp_win->icon_name, tmp1->twm->icon_name);
			}
			else {
				compresult = strcasecmp(tmp_win->icon_name, tmp1->twm->icon_name);
			}
			if(compresult < 0) {
				tmp->next = tmp1;
				tmp->prev = tmp1->prev;
				tmp1->prev = tmp;
				if(tmp->prev == NULL) {
					ip->first = tmp;
				}
				else {
					tmp->prev->next = tmp;
				}
				added = true;
				break;
			}
		}
	}

	if(!added) {
		ip->last->next = tmp;
		tmp->prev = ip->last;
		ip->last = tmp;
	}
}

void RemoveFromIconManager(IconMgr *ip, WList *tmp)
{
	if(tmp->prev == NULL) {
		ip->first = tmp->next;
	}
	else {
		tmp->prev->next = tmp->next;
	}

	if(tmp->next == NULL) {
		ip->last = tmp->prev;
	}
	else {
		tmp->next->prev = tmp->prev;
	}

	/* pebl: If the list was the current and tmp was the last in the list
	   reset current list */
	if(Current == tmp) {
		Current = ip->first;
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      RemoveIconManager - remove a window from the icon manager
 *
 *  Inputs:
 *      tmp_win - the TwmWindow structure
 *
 ***********************************************************************
 */

void RemoveIconManager(TwmWindow *tmp_win)
{
	IconMgr *ip;
	WList *tmp, *tmp1, *save;

	if(tmp_win->iconmanagerlist == NULL) {
		return;
	}

	tmp  = tmp_win->iconmanagerlist;
	tmp1 = NULL;

	while(tmp != NULL) {
		ip = tmp->iconmgr;
		if((tmp_win->occupation & ip->twm_win->occupation) != 0) {
			tmp1 = tmp;
			tmp  = tmp->nextv;
			continue;
		}
		RemoveFromIconManager(ip, tmp);

		XDeleteContext(dpy, tmp->icon, TwmContext);
		XDeleteContext(dpy, tmp->icon, ScreenContext);
		XDestroyWindow(dpy, tmp->icon);
		XDeleteContext(dpy, tmp->w, TwmContext);
		XDeleteContext(dpy, tmp->w, ScreenContext);
		XDestroyWindow(dpy, tmp->w);
		ip->count -= 1;

		PackIconManager(ip);

		if(ip->count == 0) {
			XUnmapWindow(dpy, ip->twm_win->frame);
			ip->twm_win->mapped = false;
		}
		if(tmp1 == NULL) {
			tmp_win->iconmanagerlist = tmp_win->iconmanagerlist->nextv;
		}
		else {
			tmp1->nextv = tmp->nextv;
		}

		save = tmp;
		tmp = tmp->nextv;
		free(save);
	}
}

void CurrentIconManagerEntry(WList *current)
{
	Current = current;
}

void ActiveIconManager(WList *active)
{
	active->active = true;
	Active = active;
	Active->iconmgr->active = active;
	Current = Active;
	DrawIconManagerBorder(active, false);
}

void NotActiveIconManager(WList *active)
{
	active->active = false;
	DrawIconManagerBorder(active, false);
}

void DrawIconManagerBorder(WList *tmp, bool fill)
{
	if(Scr->use3Diconmanagers) {
		Draw3DBorder(tmp->w, 0, 0, tmp->width, tmp->height,
		             Scr->IconManagerShadowDepth, tmp->cp,
		             (tmp->active && Scr->Highlight ? on : off),
		             fill, false);
	}
	else {
		XSetForeground(dpy, Scr->NormalGC, tmp->cp.fore);
		XDrawRectangle(dpy, tmp->w, Scr->NormalGC, 2, 2, tmp->width - 5,
		               tmp->height - 5);

		XSetForeground(dpy, Scr->NormalGC,
		               (tmp->active && Scr->Highlight
		                ? tmp->highlight : tmp->cp.back));

		XDrawRectangle(dpy, tmp->w, Scr->NormalGC, 0, 0, tmp->width - 1,
		               tmp->height - 1);
		XDrawRectangle(dpy, tmp->w, Scr->NormalGC, 1, 1, tmp->width - 3,
		               tmp->height - 3);
	}
}

/***********************************************************************
 *
 *  Procedure:
 *      SortIconManager - sort the dude
 *
 *  Inputs:
 *      ip      - a pointer to the icon manager struture
 *
 ***********************************************************************
 */

void SortIconManager(IconMgr *ip)
{
	WList *tmp1, *tmp2;
	int done;

	if(ip == NULL) {
		ip = Active->iconmgr;
	}

	done = false;
	do {
		for(tmp1 = ip->first; tmp1 != NULL; tmp1 = tmp1->next) {
			int compresult;
			if((tmp2 = tmp1->next) == NULL) {
				done = true;
				break;
			}
			if(Scr->CaseSensitive) {
				compresult = strcmp(tmp1->twm->icon_name, tmp2->twm->icon_name);
			}
			else {
				compresult = strcasecmp(tmp1->twm->icon_name, tmp2->twm->icon_name);
			}
			if(compresult > 0) {
				/* take it out and put it back in */
				RemoveFromIconManager(ip, tmp2);
				InsertInIconManager(ip, tmp2, tmp2->twm);
				break;
			}
		}
	}
	while(!done);
	PackIconManager(ip);
}

/***********************************************************************
 *
 *  Procedure:
 *      PackIconManager - pack the icon manager windows following
 *              an addition or deletion
 *
 *  Inputs:
 *      ip      - a pointer to the icon manager struture
 *
 ***********************************************************************
 */

void PackIconManagers(void)
{
	TwmWindow *twm_win;

	for(twm_win = Scr->FirstWindow; twm_win != NULL; twm_win = twm_win->next) {
		if(twm_win->iconmgrp) {
			PackIconManager(twm_win->iconmgrp);
		}
	}
}

void PackIconManager(IconMgr *ip)
{
	int newwidth, i, row, col, maxcol,  colinc, rowinc, wheight, wwidth;
	int new_x, new_y;
	int savewidth;
	WList *tmp;
	int mask;

	wheight = Scr->IconManagerFont.avg_height
	          + 2 * (ICON_MGR_OBORDER + ICON_MGR_IBORDER);
	if(wheight < (im_iconified_icon_height + 4)) {
		wheight = im_iconified_icon_height + 4;
	}

	wwidth = ip->width / ip->columns;

	rowinc = wheight;
	colinc = wwidth;

	row = 0;
	col = ip->columns;
	maxcol = 0;
	for(i = 0, tmp = ip->first; tmp != NULL; i++, tmp = tmp->next) {
		tmp->me = i;
		if(++col >= ip->columns) {
			col = 0;
			row += 1;
		}
		if(col > maxcol) {
			maxcol = col;
		}

		new_x = col * colinc;
		new_y = (row - 1) * rowinc;

		/* if the position or size has not changed, don't touch it */
		if(tmp->x != new_x || tmp->y != new_y ||
		                tmp->width != wwidth || tmp->height != wheight) {
			XMoveResizeWindow(dpy, tmp->w, new_x, new_y, wwidth, wheight);
			if(tmp->height != wheight)
				XMoveWindow(dpy, tmp->icon, ICON_MGR_OBORDER + ICON_MGR_IBORDER,
				            (wheight - im_iconified_icon_height) / 2);

			tmp->row = row - 1;
			tmp->col = col;
			tmp->x = new_x;
			tmp->y = new_y;
			tmp->width = wwidth;
			tmp->height = wheight;
		}
	}
	maxcol += 1;

	ip->cur_rows = row;
	ip->cur_columns = maxcol;
	ip->height = row * rowinc;
	if(ip->height == 0) {
		ip->height = rowinc;
	}
	newwidth = maxcol * colinc;
	if(newwidth == 0) {
		newwidth = colinc;
	}

	XResizeWindow(dpy, ip->w, newwidth, ip->height);

	mask = RLayoutXParseGeometry(Scr->Layout, ip->geometry, &JunkX, &JunkY,
	                             &JunkWidth, &JunkHeight);
	if(mask & XNegative) {
		ip->twm_win->frame_x += ip->twm_win->frame_width - newwidth -
		                        2 * ip->twm_win->frame_bw3D;
	}
	if(mask & YNegative) {
		ip->twm_win->frame_y += ip->twm_win->frame_height - ip->height -
		                        2 * ip->twm_win->frame_bw3D - ip->twm_win->title_height;
	}
	savewidth = ip->width;
	if(ip->twm_win)
		SetupWindow(ip->twm_win,
		            ip->twm_win->frame_x, ip->twm_win->frame_y,
		            newwidth + 2 * ip->twm_win->frame_bw3D,
		            ip->height + ip->twm_win->title_height + 2 * ip->twm_win->frame_bw3D, -1);
	ip->width = savewidth;
}

void dump_iconmanager(IconMgr *mgr, char *label)
{
	fprintf(stderr, "IconMgr %s %p name='%s' geom='%s'\n",
	        label,
	        mgr,
	        mgr->name,
	        mgr->geometry);
	fprintf(stderr, "next = %p, prev = %p, lasti = %p, nextv = %p\n",
	        mgr->next,
	        mgr->prev,
	        mgr->lasti,
	        mgr->nextv);
}


/*
 * Draw the window name into the icon manager line
 */
void
DrawIconManagerIconName(TwmWindow *tmp_win)
{
	WList *iconmanagerlist = tmp_win->iconmanagerlist;
	XRectangle ink_rect, logical_rect;

	XmbTextExtents(Scr->IconManagerFont.font_set,
	               tmp_win->icon_name, strlen(tmp_win->icon_name),
	               &ink_rect, &logical_rect);

	if(UpdateFont(&Scr->IconManagerFont, logical_rect.height)) {
		PackIconManagers();
	}

	// Write in the title
	FB(iconmanagerlist->cp.fore, iconmanagerlist->cp.back);

	/* XXX This is a completely absurd way of writing this */
	((Scr->use3Diconmanagers && (Scr->Monochrome != COLOR)) ?
	 XmbDrawImageString : XmbDrawString)
	(dpy,
	 iconmanagerlist->w,
	 Scr->IconManagerFont.font_set,
	 Scr->NormalGC,
	 iconmgr_textx,
	 (Scr->IconManagerFont.avg_height - logical_rect.height) / 2
	 + (- logical_rect.y)
	 + ICON_MGR_OBORDER
	 + ICON_MGR_IBORDER,
	 tmp_win->icon_name,
	 strlen(tmp_win->icon_name));

	// Draw the border around it.  Our "border" isn't an X border, it's
	// just our own drawing inside the X window.  Since XmbDrawString()
	// believes it has all the space in the window to fill, it might
	// scribble into the space where we're drawing the border, so draw
	// the border after the text to cover it up.
	DrawIconManagerBorder(iconmanagerlist, false);
}


/*
 * Copy the icon into the icon manager for a window that's iconified.
 * This is slightly different for the 3d vs 2d case, since the 3d is just
 * copying a pixmap in, while the 2d is drawing a bitmap in with the
 * fg/bg colors appropriate to the line.
 */
void
ShowIconifiedIcon(TwmWindow *tmp_win)
{
	WList *iconmanagerlist = tmp_win->iconmanagerlist;

	if(Scr->use3Diconmanagers && iconmanagerlist->iconifypm) {
		XCopyArea(dpy, iconmanagerlist->iconifypm,
		          iconmanagerlist->icon,
		          Scr->NormalGC, 0, 0,
		          im_iconified_icon_width, im_iconified_icon_height, 0, 0);
	}
	else {
		FB(iconmanagerlist->cp.fore, iconmanagerlist->cp.back);
		XCopyPlane(dpy, Scr->siconifyPm, iconmanagerlist->icon,
		           Scr->NormalGC, 0, 0,
		           im_iconified_icon_width, im_iconified_icon_height, 0, 0, 1);
	}
}
