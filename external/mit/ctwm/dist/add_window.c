/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/

/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */

/**********************************************************************
 *
 * $XConsortium: add_window.c,v 1.153 91/07/10 13:17:26 dave Exp $
 *
 * Add a new window, put the titlbar and other stuff around
 * the window
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 **********************************************************************/

#include <stdio.h>
#include <string.h>
#ifndef VMS
#include <sys/time.h>
#else
#include <time.h>
#endif
#if defined(AIXV3) || defined(_SYSTYPE_SVR4) || defined(ibm) || defined __QNX__
#include <sys/select.h>
#endif
#include "twm.h"
#ifdef VMS
#include <decw$include/Xatom.h>
#else
#include <X11/Xatom.h>
#endif
#include "add_window.h"
#include "windowbox.h"
#include "util.h"
#include "resize.h"
#include "parse.h"
#include "list.h"
#include "events.h"
#include "menus.h"
#include "screen.h"
#include "icons.h"
#include "iconmgr.h"
#include "session.h"
#include "mwmhints.h"

#define gray_width 2
#define gray_height 2
static unsigned char gray_bits[] = {
   0x02, 0x01};
static unsigned char black_bits[] = {
   0xFF, 0xFF};

int AddingX;
int AddingY;
unsigned int AddingW;
unsigned int AddingH;

static int PlaceX = -1;
static int PlaceY = -1;
static void CreateWindowTitlebarButtons(TwmWindow *tmp_win);
void DealWithNonSensicalGeometries(Display *dpy, Window vroot, TwmWindow *tmp_win);

static void		splitWindowRegionEntry (WindowEntry	*we,
						int grav1, int grav2,
						int w, int h);
static WindowEntry	*findWindowEntry (WorkSpace    *wl,
					  TwmWindow    *tmp_win,
					  WindowRegion **wrp);
static WindowEntry	*prevWindowEntry (WindowEntry	*we,
					  WindowRegion	*wr);
static void		mergeWindowEntries (WindowEntry	*old, WindowEntry *we);

char NoName[] = "Untitled"; /* name if no name is specified */
int  resizeWhenAdd;

extern Atom _OL_WIN_ATTR;

#if defined(__hpux) && !defined(_XPG4_EXTENDED)
#   define FDSET int*
#else
#   define FDSET fd_set*
#endif
/************************************************************************
 *
 *  Procedure:
 *	GetGravityOffsets - map gravity to (x,y) offset signs for adding
 *		to x and y when window is mapped to get proper placement.
 * 
 ************************************************************************
 */

void GetGravityOffsets (TwmWindow *tmp,	/* window from which to get gravity */
			int *xp, int *yp)	/* return values */
{
    static struct _gravity_offset {
	int x, y;
    } gravity_offsets[11] = {
	{  0,  0 },			/* ForgetGravity */
	{ -1, -1 },			/* NorthWestGravity */
	{  0, -1 },			/* NorthGravity */
	{  1, -1 },			/* NorthEastGravity */
	{ -1,  0 },			/* WestGravity */
	{  0,  0 },			/* CenterGravity */
	{  1,  0 },			/* EastGravity */
	{ -1,  1 },			/* SouthWestGravity */
	{  0,  1 },			/* SouthGravity */
	{  1,  1 },			/* SouthEastGravity */
	{  0,  0 },			/* StaticGravity */
    };
    register int g = ((tmp->hints.flags & PWinGravity) 
		      ? tmp->hints.win_gravity : NorthWestGravity);

    if (g < ForgetGravity || g > StaticGravity) {
	*xp = *yp = 0;
    } else {
	*xp = gravity_offsets[g].x;
	*yp = gravity_offsets[g].y;
    }
}




/***********************************************************************
 *
 *  Procedure:
 *	AddWindow - add a new window to the twm list
 *
 *  Returned Value:
 *	(TwmWindow *) - pointer to the TwmWindow structure
 *
 *  Inputs:
 *	w	- the window id of the window to add
 *	iconm	- flag to tell if this is an icon manager window
 *		0 --> normal window.
 *		1 --> icon manager.
 *		2 --> window box;
 *		else --> iconmgr;
 *
 *	iconp	- pointer to icon manager struct
 *
 ***********************************************************************
 */

TwmWindow *AddWindow(Window w, int iconm, IconMgr *iconp)
{
    virtualScreen *vs;
    TwmWindow *tmp_win;			/* new twm window structure */
    int stat;
    XEvent event;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int width, height;			/* tmp variable */
    int ask_user;		/* don't know where to put the window */
    int gravx, gravy;			/* gravity signs for positioning */
    int namelen;
    int bw2;
    short saved_x, saved_y, restore_icon_x, restore_icon_y;
    unsigned short saved_width, saved_height;
    Bool restore_iconified = 0;
    Bool restore_icon_info_present = 0;
    int restoredFromPrevSession;
    Bool width_ever_changed_by_user;
    Bool height_ever_changed_by_user;
    int saved_occupation; /* <== [ Matthew McNeill Feb 1997 ] == */
    Bool        random_placed = False;
    int		found = 0;
#ifndef VMS
    fd_set	mask;
    int		fd;
    struct timeval timeout;
#endif
    XRectangle ink_rect;
    XRectangle logical_rect;
    WindowBox *winbox;
    int iswinbox = 0;
    int iswman = 0;
    Window vroot;

#ifdef DEBUG
    fprintf(stderr, "AddWindow: w = 0x%x\n", w);
#endif

    if (!captive && RedirectToCaptive (w)) return (NULL);

    /* allocate space for the twm window */
    tmp_win = (TwmWindow *)calloc(1, sizeof(TwmWindow));
    if (tmp_win == 0)
    {
	fprintf (stderr, "%s: Unable to allocate memory to manage window ID %lx.\n",
		 ProgramName, w);
	return NULL;
    }
    switch (iconm) {
	case  0 : iswinbox = 0; break;
	case  1 : iswinbox = 0; break;
	case  2 : iswinbox = 1; iconm  = 0; break;
	case  3 : iswman   = 1; iconm  = 0; break;
	default : iswinbox = 0; iswman = 0; iconm = 1; break;
    }
    tmp_win->w = w;
    tmp_win->zoomed = ZOOM_NONE;
    tmp_win->iconmgr = iconm;
    tmp_win->iconmgrp = iconp;
    tmp_win->wspmgr = iswman;
    tmp_win->iswinbox = iswinbox;
    tmp_win->vs = NULL;
    tmp_win->old_parent_vs = NULL;
    tmp_win->savevs = NULL;
    tmp_win->cmaps.number_cwins = 0;
    tmp_win->savegeometry.width = -1;

    XSelectInput(dpy, tmp_win->w, PropertyChangeMask);
    XGetWindowAttributes(dpy, tmp_win->w, &tmp_win->attr);
    tmp_win->name = (char*) GetWMPropertyString(tmp_win->w, XA_WM_NAME);
    tmp_win->class = NoClass;
    XGetClassHint(dpy, tmp_win->w, &tmp_win->class);
    FetchWmProtocols (tmp_win);
    FetchWmColormapWindows (tmp_win);

    if (GetWindowConfig (tmp_win,
	&saved_x, &saved_y, &saved_width, &saved_height,
	&restore_iconified, &restore_icon_info_present,
	&restore_icon_x, &restore_icon_y,
	&width_ever_changed_by_user, &height_ever_changed_by_user,
	&saved_occupation)) /* <== [ Matthew McNeill Feb 1997 ] == */
    {
	tmp_win->attr.x = saved_x;
	tmp_win->attr.y = saved_y;

	tmp_win->widthEverChangedByUser = width_ever_changed_by_user;
	tmp_win->heightEverChangedByUser = height_ever_changed_by_user;
	
	if (width_ever_changed_by_user)
	    tmp_win->attr.width = saved_width;

	if (height_ever_changed_by_user)
	    tmp_win->attr.height = saved_height;

	restoredFromPrevSession = 1;
    }
    else
    {
	tmp_win->widthEverChangedByUser = False;
	tmp_win->heightEverChangedByUser = False;

	restoredFromPrevSession = 0;
    }

    /*
     * do initial clip; should look at window gravity
     */
    if (tmp_win->attr.width > Scr->MaxWindowWidth)
      tmp_win->attr.width = Scr->MaxWindowWidth;
    if (tmp_win->attr.height > Scr->MaxWindowHeight)
      tmp_win->attr.height = Scr->MaxWindowHeight;

    tmp_win->wmhints = XGetWMHints(dpy, tmp_win->w);

    if (tmp_win->wmhints)
    {
	if (restore_iconified)
	{
	    tmp_win->wmhints->initial_state = IconicState;
	    tmp_win->wmhints->flags |= StateHint;
	}

	if (restore_icon_info_present)
	{
	    tmp_win->wmhints->icon_x = restore_icon_x;
	    tmp_win->wmhints->icon_y = restore_icon_y;
	    tmp_win->wmhints->flags |= IconPositionHint;
	}
    }

    if (tmp_win->wmhints) tmp_win->wmhints->input = True;
				/* CL: Having with not willing focus
				cause problems with AutoSqueeze and a few others
				things. So I suppress it. And the whole focus thing
				is buggy anyway */
    if (tmp_win->wmhints && !(tmp_win->wmhints->flags & InputHint))
	tmp_win->wmhints->input = True;
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & WindowGroupHint)) {
	tmp_win->group = tmp_win->wmhints->window_group;
	if (tmp_win->group) {
	    /*
	     * GTK windows often have a spurious "group leader" window which is
	     * never reported to us and therefore does not really exist.  This
	     * is annoying because we treat group members a lot like transient
	     * windows.  Look for that here. It is in fact a duplicate of the
	     * WM_CLIENT_LEADER property.
	     */
	    if (tmp_win->group != w && !GetTwmWindow(tmp_win->group)) {
		tmp_win->group = 0;
	    }
	} 
    } else
	tmp_win->group = 0;

    /*
     * The July 27, 1988 draft of the ICCCM ignores the size and position
     * fields in the WM_NORMAL_HINTS property.
     */

    tmp_win->transient = Transient(tmp_win->w, &tmp_win->transientfor);

    tmp_win->nameChanged = 0;
    if (tmp_win->name == NULL)
	tmp_win->name = NoName;
    if (tmp_win->class.res_name == NULL)
    	tmp_win->class.res_name = NoName;
    if (tmp_win->class.res_class == NULL)
    	tmp_win->class.res_class = NoName;

    /*
     * full_name seems to exist only because in the conditional code below,
     * name is sometimes changed. In all other cases, name and full_name
     * seem to be identical. Is that worth it?
     */
    tmp_win->full_name = tmp_win->name;
#ifdef CLAUDE
    if (strstr (tmp_win->name, " - Mozilla")) {
      char *moz = strstr (tmp_win->name, " - Mozilla");
      *moz = '\0';
    }
#endif
    namelen = strlen (tmp_win->name);

    if (LookInList(Scr->IgnoreTransientL, tmp_win->full_name, &tmp_win->class))
      tmp_win->transient = 0;

    tmp_win->highlight = Scr->Highlight && 
	(!LookInList(Scr->NoHighlight, tmp_win->full_name, 
	    &tmp_win->class));

    tmp_win->stackmode = Scr->StackMode &&
	(!LookInList(Scr->NoStackModeL, tmp_win->full_name, 
	    &tmp_win->class));

    tmp_win->ontoppriority = (LookInList(Scr->AlwaysOnTopL,
	tmp_win->full_name, &tmp_win->class)) ? ONTOP_MAX : ONTOP_DEFAULT;

    tmp_win->titlehighlight = Scr->TitleHighlight && 
	(!LookInList(Scr->NoTitleHighlight, tmp_win->full_name, 
	    &tmp_win->class));

    tmp_win->auto_raise = Scr->AutoRaiseDefault ||
	LookInList(Scr->AutoRaise, tmp_win->full_name,
			      &tmp_win->class);
    if (tmp_win->auto_raise) Scr->NumAutoRaises++;

    tmp_win->auto_lower = Scr->AutoLowerDefault ||
	LookInList(Scr->AutoLower, tmp_win->full_name,
			      &tmp_win->class);
    if (tmp_win->auto_lower) Scr->NumAutoLowers++;

    tmp_win->iconify_by_unmapping = Scr->IconifyByUnmapping;
    if (Scr->IconifyByUnmapping)
    {
	tmp_win->iconify_by_unmapping = iconm ? FALSE :
	    !LookInList(Scr->DontIconify, tmp_win->full_name,
		&tmp_win->class);
    }
    tmp_win->iconify_by_unmapping = tmp_win->iconify_by_unmapping ||
	LookInList(Scr->IconifyByUn, tmp_win->full_name, &tmp_win->class);

    if (LookInList (Scr->UnmapByMovingFarAway, tmp_win->full_name, &tmp_win->class))
	tmp_win->UnmapByMovingFarAway = True;
    else
	tmp_win->UnmapByMovingFarAway = False;

    if (LookInList (Scr->DontSetInactive, tmp_win->full_name, &tmp_win->class))
	tmp_win->DontSetInactive = True;
    else
	tmp_win->DontSetInactive = False;

    if (LookInList (Scr->AutoSqueeze, tmp_win->full_name, &tmp_win->class))
	tmp_win->AutoSqueeze = True;
    else
	tmp_win->AutoSqueeze = False;

    if (LookInList (Scr->StartSqueezed, tmp_win->full_name, &tmp_win->class))
	tmp_win->StartSqueezed = True;
    else
	tmp_win->StartSqueezed = False;

    if (Scr->AlwaysSqueezeToGravity
	|| LookInList (Scr->AlwaysSqueezeToGravityL, tmp_win->full_name, &tmp_win->class))
	tmp_win->AlwaysSqueezeToGravity = True;
    else
	tmp_win->AlwaysSqueezeToGravity = False;

    if (tmp_win->transient || tmp_win->group) {
	TwmWindow *t = NULL;
	if (tmp_win->transient) t = GetTwmWindow(tmp_win->transientfor);
	if (!t && tmp_win->group) t = GetTwmWindow(tmp_win->group);
	if (t) tmp_win->UnmapByMovingFarAway = t->UnmapByMovingFarAway;
    }
    if ((Scr->WindowRingAll && !iswman &&
	!LookInList(Scr->WindowRingExcludeL, tmp_win->full_name, &tmp_win->class)) ||
	LookInList(Scr->WindowRingL, tmp_win->full_name, &tmp_win->class)) {
	if (Scr->Ring) {
	    tmp_win->ring.next = Scr->Ring->ring.next;
	    if (Scr->Ring->ring.next->ring.prev)
	      Scr->Ring->ring.next->ring.prev = tmp_win;
	    Scr->Ring->ring.next = tmp_win;
	    tmp_win->ring.prev = Scr->Ring;
	} else {
	    tmp_win->ring.next = tmp_win->ring.prev = Scr->Ring = tmp_win;
	}
    } else
      tmp_win->ring.next = tmp_win->ring.prev = NULL;
    tmp_win->ring.cursor_valid = False;

    tmp_win->squeeze_info = NULL;
    tmp_win->squeeze_info_copied = 0;
    /*
     * get the squeeze information; note that this does not have to be freed
     * since it is coming from the screen list
     */
    if (HasShape) {
	if (!LookInList (Scr->DontSqueezeTitleL, tmp_win->full_name, 
			 &tmp_win->class)) {
	    tmp_win->squeeze_info = (SqueezeInfo *)
	      LookInList (Scr->SqueezeTitleL, tmp_win->full_name,
			  &tmp_win->class);
	    if (!tmp_win->squeeze_info) {
		static SqueezeInfo default_squeeze = { J_LEFT, 0, 0 };
		if (Scr->SqueezeTitle)
		  tmp_win->squeeze_info = &default_squeeze;
	    }
	}
      }

    tmp_win->old_bw = tmp_win->attr.border_width;

    { 
	MotifWmHints mwmHints;
	Boolean have_title;

	GetMWMHints(tmp_win->w, &mwmHints);

	tmp_win->frame_bw3D = Scr->ThreeDBorderWidth;
	if (((mwmHints.flags & MWM_HINTS_DECORATIONS) &&	
		 (mwmHints.decorations & MWM_DECOR_BORDER) == 0)
	    || LookInList(Scr->NoBorder, tmp_win->full_name, &tmp_win->class)) {
	    tmp_win->frame_bw = 0;
	    tmp_win->frame_bw3D = 0;
	} else if (tmp_win->frame_bw3D != 0) {
	    tmp_win->frame_bw = 0;
	    Scr->ClientBorderWidth = FALSE;
	} else if (Scr->ClientBorderWidth) {
	    tmp_win->frame_bw = tmp_win->old_bw;
	} else {
	    tmp_win->frame_bw = Scr->BorderWidth;
	}
	bw2 = tmp_win->frame_bw * 2;

	
	have_title = True;
	if (mwmHints.flags & MWM_HINTS_DECORATIONS)
	    have_title = (mwmHints.decorations & MWM_DECOR_TITLE) != 0;
	if (Scr->NoTitlebar)
	    have_title = False;
	if (LookInList(Scr->MakeTitle, tmp_win->full_name, &tmp_win->class))
	    have_title = True;
	if (LookInList(Scr->NoTitle, tmp_win->full_name, &tmp_win->class))
	    have_title = False;

	if (have_title) {
	    tmp_win->title_height = Scr->TitleHeight + tmp_win->frame_bw;
	} else {
	    tmp_win->title_height = 0;
	}
    }

    tmp_win->OpaqueMove = Scr->DoOpaqueMove;
    if (LookInList(Scr->OpaqueMoveList, tmp_win->full_name, &tmp_win->class))
	tmp_win->OpaqueMove = TRUE;
    else
    if (LookInList(Scr->NoOpaqueMoveList, tmp_win->full_name, &tmp_win->class))
	tmp_win->OpaqueMove = FALSE;

    tmp_win->OpaqueResize = Scr->DoOpaqueResize;
    if (LookInList(Scr->OpaqueResizeList, tmp_win->full_name, &tmp_win->class))
	tmp_win->OpaqueResize = TRUE;
    else
    if (LookInList(Scr->NoOpaqueResizeList, tmp_win->full_name, &tmp_win->class))
	tmp_win->OpaqueResize = FALSE;

    /* if it is a transient window, don't put a title on it */
    if (tmp_win->transient && !Scr->DecorateTransients)
	tmp_win->title_height = 0;

    if (LookInList(Scr->StartIconified, tmp_win->full_name, &tmp_win->class))
    {
	if (!tmp_win->wmhints)
	{
	    tmp_win->wmhints = (XWMHints *)malloc(sizeof(XWMHints));
	    tmp_win->wmhints->flags = 0;
	}
	tmp_win->wmhints->initial_state = IconicState;
	tmp_win->wmhints->flags |= StateHint;
    }

    GetWindowSizeHints (tmp_win);

    if (restoredFromPrevSession)
    {
	/*
	 * When restoring window positions from the previous session,
	 * we always use NorthWest gravity.
	 */

	gravx = gravy = -1;
    }
    else
    {
	GetGravityOffsets (tmp_win, &gravx, &gravy);
    }

    /*
     * Don't bother user if:
     * 
     *     o  the window is a transient, or
     * 
     *     o  a USPosition was requested, or
     * 
     *     o  a PPosition was requested and UsePPosition is ON or
     *        NON_ZERO if the window is at other than (0,0)
     */
    ask_user = TRUE;
    if (tmp_win->transient || 
	(tmp_win->hints.flags & USPosition) ||
        ((tmp_win->hints.flags & PPosition) && Scr->UsePPosition &&
	 (Scr->UsePPosition == PPOS_ON || 
	  tmp_win->attr.x != 0 || tmp_win->attr.y != 0)))
      ask_user = FALSE;

    /*===============[ Matthew McNeill 1997 ]==========================*
     * added the occupation parameter to this function call so that the 
     * occupation can be set up in a specific state if desired 
     * (restore session for example)
     */

    /* note, this is where tmp_win->vs get setup, among other things */

    if (restoredFromPrevSession) {
      SetupOccupation (tmp_win, saved_occupation);
    } else
      SetupOccupation (tmp_win, 0);
    /*=================================================================*/

    tmp_win->frame_width  = tmp_win->attr.width  + 2 * tmp_win->frame_bw3D;
    tmp_win->frame_height = tmp_win->attr.height + 2 * tmp_win->frame_bw3D +
			    tmp_win->title_height;
    ConstrainSize (tmp_win, &tmp_win->frame_width, &tmp_win->frame_height);
    winbox = findWindowBox (tmp_win);
    if (PlaceWindowInRegion (tmp_win, &(tmp_win->attr.x), &(tmp_win->attr.y))) {
	ask_user = False;
    }
    if (LookInList (Scr->WindowGeometries, tmp_win->full_name, &tmp_win->class)) {
        char *geom;
	int mask_;
	geom = LookInList (Scr->WindowGeometries, tmp_win->full_name, &tmp_win->class);
	mask_ = XParseGeometry (geom, &tmp_win->attr.x, &tmp_win->attr.y,
				(unsigned int*) &tmp_win->attr.width,
				(unsigned int*) &tmp_win->attr.height);

	if (mask_ & XNegative) tmp_win->attr.x += Scr->rootw - tmp_win->attr.width;
	if (mask_ & YNegative) tmp_win->attr.y += Scr->rooth - tmp_win->attr.height;
	ask_user = False;
    }

    if (XFindContext (dpy, w, VirtScreenContext, (XPointer *)&vs) == XCSUCCESS)
      vroot = vs->window;
    else
    if (tmp_win->vs)
      vroot = tmp_win->vs->window;
    else
      vroot = Scr->Root;
    if (winbox) vroot = winbox->window;

    /*
     * do any prompting for position
     */

    if (HandlingEvents && ask_user && !restoredFromPrevSession) {
      if ((Scr->RandomPlacement == RP_ALL) ||
          ((Scr->RandomPlacement == RP_UNMAPPED) &&
	   ((tmp_win->wmhints && (tmp_win->wmhints->initial_state == IconicState)) ||
	    (! visible (tmp_win))))) { /* just stick it somewhere */

#ifdef DEBUG
	fprintf(stderr,
		"DEBUG[RandomPlacement]: win: %dx%d+%d+%d, screen: %dx%d, title height: %d, random: +%d+%d\n",
		tmp_win->attr.width, tmp_win->attr.height,
		tmp_win->attr.x, tmp_win->attr.y,
		Scr->rootw, Scr->rooth,
		tmp_win->title_height,
		PlaceX, PlaceY);
#endif

	/* Initiallise PlaceX and PlaceY */
	if (PlaceX < 0 && PlaceY < 0) {
	  if (Scr->RandomDisplacementX >= 0)
	    PlaceX = Scr->BorderLeft + 5;
	  else
	    PlaceX = Scr->rootw - tmp_win->attr.width - Scr->BorderRight - 5;
	  if (Scr->RandomDisplacementY >= 0)
	    PlaceY = Scr->BorderTop + 5;
	  else
	    PlaceY = Scr->rooth - tmp_win->attr.height - tmp_win->title_height
	      - Scr->BorderBottom - 5;
	}

	/* For a positive horizontal displacement, if the right edge
	   of the window would fall outside of the screen, start over
	   by placing the left edge of the window 5 pixels inside
	   the left edge of the screen.*/
	if (Scr->RandomDisplacementX >= 0
	    && (PlaceX + tmp_win->attr.width
		> Scr->rootw - Scr->BorderRight - 5))
	  PlaceX = Scr->BorderLeft + 5;

	/* For a negative horizontal displacement, if the left edge
	   of the window would fall outside of the screen, start over
	   by placing the right edge of the window 5 pixels inside
	   the right edge of the screen.*/
	if (Scr->RandomDisplacementX < 0 && PlaceX < Scr->BorderLeft + 5)
	  PlaceX = Scr->rootw - tmp_win->attr.width - Scr->BorderRight - 5;

	/* For a positive vertical displacement, if the bottom edge
	   of the window would fall outside of the screen, start over
	   by placing the top edge of the window 5 pixels inside the
	   top edge of the screen.  Because we add the title height
	   further down, we need to count with it here as well.  */
	if (Scr->RandomDisplacementY >= 0
	    && (PlaceY + tmp_win->attr.height + tmp_win->title_height
		> Scr->rooth - Scr->BorderBottom - 5))
	  PlaceY = Scr->BorderTop + 5;

	/* For a negative vertical displacement, if the top edge of
	   the window would fall outside of the screen, start over by
	   placing the bottom edge of the window 5 pixels inside the
	   bottom edge of the screen.  Because we add the title height
	   further down, we need to count with it here as well.  */
	if (Scr->RandomDisplacementY < 0 && PlaceY < Scr->BorderTop + 5)
	  PlaceY = Scr->rooth - tmp_win->attr.height - tmp_win->title_height
	    - Scr->BorderBottom - 5;

	/* Assign the current random placement to the new window, as
	   a preliminary measure.  Add the title height so things will
	   look right.  */
	tmp_win->attr.x = PlaceX;
	tmp_win->attr.y = PlaceY + tmp_win->title_height;

	/* If the window is not supposed to move off the screen, check
	   that it's still within the screen, and if not, attempt to
	   correct the situation. */
	if (Scr->DontMoveOff) {
	    int available;

#ifdef DEBUG
	    fprintf(stderr,
		    "DEBUG[DontMoveOff]: win: %dx%d+%d+%d, screen: %dx%d, bw2: %d, bw3D: %d\n",
		    tmp_win->attr.width, tmp_win->attr.height,
		    tmp_win->attr.x, tmp_win->attr.y,
		    Scr->rootw, Scr->rooth,
		    bw2, tmp_win->frame_bw3D);
#endif

	    /* If the right edge of the window is outside the right edge
	       of the screen, we need to move the window left.  Note that
	       this can only happen with windows that are less than 50
	       pixels less wide than the screen. */
	    if ((tmp_win->attr.x + tmp_win->attr.width)  > Scr->rootw) {
	      available = Scr->rootw - tmp_win->attr.width
		- 2 * (bw2 + tmp_win->frame_bw3D);

#ifdef DEBUG
	      fprintf(stderr, "DEBUG[DontMoveOff]: availableX: %d\n",
		      available);
#endif

	      /* If the window is wider than the screen or exactly the width
		 of the screen, the availability is exactly 0.  The result
		 will be to have the window placed as much to the left as
		 possible. */
	      if (available <= 0) available = 0;

	      /* Place the window exactly between the left and right edge of
		 the screen when possible.  If available was originally less
		 than zero, it means the window's left edge will be against
		 the screen's left edge, and the window's right edge will be
		 outside the screen.  */
	      tmp_win->attr.x = available / 2;
	    }

	    /* If the bottom edge of the window is outside the bottom edge
	       of the screen, we need to move the window up.  Note that
	       this can only happen with windows that are less than 50
	       pixels less tall than the screen.  Don't forget to count
	       with the title height and the frame widths.  */
	    if ((tmp_win->attr.y + tmp_win->attr.height)  > Scr->rooth) {
	      available = Scr->rooth - tmp_win->attr.height
		- tmp_win->title_height - 2 * (bw2 + tmp_win->frame_bw3D);

#ifdef DEBUG
	      fprintf(stderr, "DEBUG[DontMoveOff]: availableY: %d\n",
		      available);
#endif

	      /* If the window is taller than the screen or exactly the
		 height of the screen, the availability is exactly 0.
		 The result will be to have the window placed as much to
		 the top as possible. */
	      if (available <= 0) available = 0;

	      /* Place the window exactly between the top and bottom edge of
		 the screen when possible.  If available was originally less
		 than zero, it means the window's top edge will be against
		 the screen's top edge, and the window's bottom edge will be
		 outside the screen.  Again, don't forget to add the title
		 height.  */
	      tmp_win->attr.y = available / 2 + tmp_win->title_height;
	    }

#ifdef DEBUG
	    fprintf(stderr,
		    "DEBUG[DontMoveOff]: win: %dx%d+%d+%d, screen: %dx%d\n",
		    tmp_win->attr.width, tmp_win->attr.height,
		    tmp_win->attr.x, tmp_win->attr.y,
		    Scr->rootw, Scr->rooth);
#endif
	}

	/* We know that if the window's left edge has moved compared to
	   PlaceX, it will have moved to the left.  If it was moved less
	   than 15 pixel either way, change the next "random position"
	   30 pixels down and right. */
	if (PlaceX - tmp_win->attr.x < 15
	    || PlaceY - (tmp_win->attr.y - tmp_win->title_height) < 15) {
	  PlaceX += Scr->RandomDisplacementX;
	  PlaceY += Scr->RandomDisplacementY;
	}

	random_placed = True;
      } else {				/* else prompt */
	if (!(tmp_win->wmhints && tmp_win->wmhints->flags & StateHint &&
	      tmp_win->wmhints->initial_state == IconicState))
	{
	    Bool firsttime = True;

	    /* better wait until all the mouse buttons have been 
	     * released.
	     */
	    while (TRUE)
	    {
		XUngrabServer(dpy);
		XSync(dpy, 0);
		XGrabServer(dpy);

		JunkMask = 0;
		if (!XQueryPointer (dpy, Scr->Root, &JunkRoot, 
				    &JunkChild, &JunkX, &JunkY,
				    &AddingX, &AddingY, &JunkMask))
		  JunkMask = 0;

		JunkMask &= (Button1Mask | Button2Mask | Button3Mask |
			     Button4Mask | Button5Mask);

		/*
		 * watch out for changing screens
		 */
		if (firsttime) {
		    if (JunkRoot != Scr->Root) {
			register int scrnum;
			for (scrnum = 0; scrnum < NumScreens; scrnum++) {
			    if (JunkRoot == RootWindow (dpy, scrnum)) break;
			}
			if (scrnum != NumScreens) PreviousScreen = scrnum;
		    }
		    if (Scr->currentvs) {
			vroot = Scr->currentvs->window;
		    }
		    firsttime = False;
		}
		if (winbox) vroot = winbox->window;

		/*
		 * wait for buttons to come up; yuck
		 */
		if (JunkMask != 0) continue;

		/* 
		 * this will cause a warp to the indicated root
		 */
		stat = XGrabPointer(dpy, vroot, False,
		    ButtonPressMask | ButtonReleaseMask |
		    PointerMotionMask | PointerMotionHintMask,
		    GrabModeAsync, GrabModeAsync,
		    vroot, UpperLeftCursor, CurrentTime);
		if (stat == GrabSuccess) break;
	    }

	    XmbTextExtents(Scr->SizeFont.font_set,
			   tmp_win->name, namelen,
			   &ink_rect, &logical_rect);
	    width = SIZE_HINDENT + ink_rect.width;
	    height = logical_rect.height + SIZE_VINDENT * 2;
	    XmbTextExtents(Scr->SizeFont.font_set,
			   ": ", 2,  &logical_rect, &logical_rect);
	    Scr->SizeStringOffset = width + logical_rect.width;

	    XResizeWindow (dpy, Scr->SizeWindow, Scr->SizeStringOffset +
				Scr->SizeStringWidth + SIZE_HINDENT, height);
	    XMapRaised(dpy, Scr->SizeWindow);
	    InstallRootColormap();
	    FB(Scr->DefaultC.fore, Scr->DefaultC.back);
	    XmbDrawImageString (dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
				Scr->NormalGC, SIZE_HINDENT,
				SIZE_VINDENT + Scr->SizeFont.ascent,
				tmp_win->name, namelen);

	    if (winbox) ConstrainedToWinBox (tmp_win, AddingX, AddingY, &AddingX, &AddingY);

	    AddingW = tmp_win->attr.width + bw2 + 2 * tmp_win->frame_bw3D;
	    AddingH = tmp_win->attr.height + tmp_win->title_height +
				bw2 + 2 * tmp_win->frame_bw3D;
	    MoveOutline(vroot,AddingX, AddingY, AddingW, AddingH,
			tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);

	    XmbDrawImageString (dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
				Scr->NormalGC, width,
				SIZE_VINDENT + Scr->SizeFont.ascent, ": ", 2);
	    DisplayPosition (tmp_win, AddingX, AddingY);

	    tmp_win->frame_width  = AddingW;
	    tmp_win->frame_height = AddingH;
	    /*SetFocus ((TwmWindow *) NULL, CurrentTime);*/
	    while (TRUE)
		{
#ifndef VMS			/* I'll try to implement this later.  RL */
		if (Scr->OpenWindowTimeout) {
		    fd = ConnectionNumber (dpy);
		    while (!XCheckMaskEvent (dpy, ButtonMotionMask | ButtonPressMask, &event)) {
			FD_ZERO (&mask);
			FD_SET  (fd, &mask);
			timeout.tv_sec  = Scr->OpenWindowTimeout;
			timeout.tv_usec = 0;
			found = select (fd + 1, (FDSET)&mask, (FDSET)0, (FDSET)0, &timeout);
			if (found == 0) break;
		    }
		    if (found == 0) break;
		} else {
#else
		{
#endif
		    found = 1;
		    XMaskEvent(dpy, ButtonPressMask | PointerMotionMask, &event);
		}
		if (event.type == MotionNotify) {
		    /* discard any extra motion events before a release */
		    while(XCheckMaskEvent(dpy,
			ButtonMotionMask | ButtonPressMask, &event))
			if (event.type == ButtonPress)
			    break;
		}
		FixRootEvent (&event);
		if (event.type == ButtonPress) {
		  AddingX = event.xbutton.x_root;
		  AddingY = event.xbutton.y_root;

		  TryToGrid (tmp_win, &AddingX, &AddingY);
		  if (Scr->PackNewWindows) TryToPack (tmp_win, &AddingX, &AddingY);

		  /* DontMoveOff prohibits user form off-screen placement */
		  if (Scr->DontMoveOff)	
                  {
                      ConstrainByBorders (tmp_win, &AddingX, AddingW, &AddingY, AddingH);
                  }
		  break;
		}

		if (event.type != MotionNotify) {
		    continue;
		}

		XQueryPointer(dpy, vroot, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		TryToGrid (tmp_win, &AddingX, &AddingY);
		if (Scr->PackNewWindows) TryToPack (tmp_win, &AddingX, &AddingY);
		if (Scr->DontMoveOff)
		{
		    ConstrainByBorders (tmp_win, &AddingX, AddingW, &AddingY, AddingH);
		}
		MoveOutline(vroot, AddingX, AddingY, AddingW, AddingH,
			    tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);

		DisplayPosition (tmp_win, AddingX, AddingY);
	    }

	  if (found) {
	    if (event.xbutton.button == Button2) {
		int lastx, lasty;

		XmbTextExtents(Scr->SizeFont.font_set,
			       ": ", 2,  &logical_rect, &logical_rect);
		Scr->SizeStringOffset = width + logical_rect.width;

		XResizeWindow (dpy, Scr->SizeWindow, Scr->SizeStringOffset +
			       Scr->SizeStringWidth + SIZE_HINDENT, height);

		XmbDrawImageString(dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
				   Scr->NormalGC, width,
				   SIZE_VINDENT + Scr->SizeFont.ascent, ": ", 2);

		if (0/*Scr->AutoRelativeResize*/) {
		    int dx = (tmp_win->attr.width / 4);
		    int dy = (tmp_win->attr.height / 4);

#define HALF_AVE_CURSOR_SIZE 8		/* so that it is visible */
		    if (dx < HALF_AVE_CURSOR_SIZE + Scr->BorderLeft)
                        dx = HALF_AVE_CURSOR_SIZE + Scr->BorderLeft;
		    if (dy < HALF_AVE_CURSOR_SIZE + Scr->BorderTop)
                        dy = HALF_AVE_CURSOR_SIZE + Scr->BorderTop;
#undef HALF_AVE_CURSOR_SIZE
		    dx += (tmp_win->frame_bw + 1);
		    dy += (bw2 + tmp_win->title_height + 1);
		    if (AddingX + dx >= Scr->rootw - Scr->BorderRight)
		      dx = Scr->rootw - Scr->BorderRight - AddingX - 1;
		    if (AddingY + dy >= Scr->rooth - Scr->BorderBottom)
		      dy = Scr->rooth - Scr->BorderBottom - AddingY - 1;
		    if (dx > 0 && dy > 0)
		      XWarpPointer (dpy, None, None, 0, 0, 0, 0, dx, dy);
		} else {
		    XWarpPointer (dpy, None, vroot, 0, 0, 0, 0,
				  AddingX + AddingW/2, AddingY + AddingH/2);
		}
		AddStartResize(tmp_win, AddingX, AddingY, AddingW, AddingH);

		lastx = -10000;
		lasty = -10000;
		while (TRUE)
		{
		    XMaskEvent(dpy,
			       ButtonReleaseMask | ButtonMotionMask, &event);

		    if (event.type == MotionNotify) {
			/* discard any extra motion events before a release */
			while(XCheckMaskEvent(dpy,
			    ButtonMotionMask | ButtonReleaseMask, &event))
			    if (event.type == ButtonRelease)
				break;
		    }
		    FixRootEvent (&event);

		    if (event.type == ButtonRelease)
		    {
			AddEndResize(tmp_win);
			break;
		    }

		    if (event.type != MotionNotify) {
			continue;
		    }

		    /*
		     * XXX - if we are going to do a loop, we ought to consider
		     * using multiple GXxor lines so that we don't need to 
		     * grab the server.
		     */
		    XQueryPointer(dpy, vroot, &JunkRoot, &JunkChild,
				  &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		    if (lastx != AddingX || lasty != AddingY)
		    {
			resizeWhenAdd = TRUE;
			DoResize(AddingX, AddingY, tmp_win);
			resizeWhenAdd = FALSE;

			lastx = AddingX;
			lasty = AddingY;
		    }

		}
	    } 
	    else if (event.xbutton.button == Button3)
	    {
		int maxw = Scr->rootw - Scr->BorderRight  - AddingX - bw2;
		int maxh = Scr->rooth - Scr->BorderBottom - AddingY - bw2;

		/*
		 * Make window go to bottom of screen, and clip to right edge.
		 * This is useful when popping up large windows and fixed
		 * column text windows.
		 */
		if (AddingW > maxw) AddingW = maxw;
		AddingH = maxh;

		ConstrainSize (tmp_win, &AddingW, &AddingH);  /* w/o borders */
		AddingW += bw2;
		AddingH += bw2;
		XMaskEvent(dpy, ButtonReleaseMask, &event);
	    }
	    else
	    {
		XMaskEvent(dpy, ButtonReleaseMask, &event);
	    }
	  }
	    MoveOutline(vroot, 0, 0, 0, 0, 0, 0);
	    XUnmapWindow(dpy, Scr->SizeWindow);
	    UninstallRootColormap();
	    XUngrabPointer(dpy, CurrentTime);

	    tmp_win->attr.x = AddingX;
	    tmp_win->attr.y = AddingY + tmp_win->title_height;
	    tmp_win->attr.width = AddingW - bw2 - 2 * tmp_win->frame_bw3D;
	    tmp_win->attr.height = AddingH - tmp_win->title_height -
				bw2 - 2 * tmp_win->frame_bw3D;

	    XUngrabServer(dpy);
	}
      }
    } else {				/* put it where asked, mod title bar */
	/* if the gravity is towards the top, move it by the title height */
	if (gravy < 0) tmp_win->attr.y -= gravy * tmp_win->title_height;
    }

#ifdef DEBUG
	fprintf(stderr, "  position window  %d, %d  %dx%d\n", 
	    tmp_win->attr.x,
	    tmp_win->attr.y,
	    tmp_win->attr.width,
	    tmp_win->attr.height);
#endif

    if (!Scr->ClientBorderWidth) {	/* need to adjust for twm borders */
	int delta = tmp_win->attr.border_width - tmp_win->frame_bw - tmp_win->frame_bw3D;
	tmp_win->attr.x += gravx * delta;
	tmp_win->attr.y += gravy * delta;
    }

    tmp_win->title_width = tmp_win->attr.width;

    tmp_win->icon_name = (char*) GetWMPropertyString(tmp_win->w, XA_WM_ICON_NAME);
    if (!tmp_win->icon_name)
	tmp_win->icon_name = tmp_win->name;

#ifdef CLAUDE
    if (strstr (tmp_win->icon_name, " - Mozilla")) {
      char *moz = strstr (tmp_win->icon_name, " - Mozilla");
      *moz = '\0';
    }
#endif

    XmbTextExtents (Scr->TitleBarFont.font_set, tmp_win->name, namelen, &ink_rect, &logical_rect);
    tmp_win->name_width = logical_rect.width;

    if (tmp_win->old_bw) XSetWindowBorderWidth (dpy, tmp_win->w, 0);

    tmp_win->squeezed = FALSE;
    tmp_win->iconified = FALSE;
    tmp_win->isicon = FALSE;
    tmp_win->icon_on = FALSE;

    XGrabServer(dpy);

    /*
     * Make sure the client window still exists.  We don't want to leave an
     * orphan frame window if it doesn't.  Since we now have the server 
     * grabbed, the window can't disappear later without having been 
     * reparented, so we'll get a DestroyNotify for it.  We won't have 
     * gotten one for anything up to here, however.
     */
    if (XGetGeometry(dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
		     &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0)
    {
	TwmWindow *prev = tmp_win->ring.prev, *next = tmp_win->ring.next;

	if (prev) prev->ring.next = next;
	if (next) next->ring.prev = prev;
	if (Scr->Ring == tmp_win) Scr->Ring = (next != tmp_win ? next : (TwmWindow *) NULL);
	if (!Scr->Ring || Scr->RingLeader == tmp_win) Scr->RingLeader = Scr->Ring;

	free((char *)tmp_win);
	XUngrabServer(dpy);
	return(NULL);
    }

    /* add the window into the twm list */
    tmp_win->next = Scr->FirstWindow;
    if (Scr->FirstWindow != NULL)
	Scr->FirstWindow->prev = tmp_win;
    tmp_win->prev = NULL;
    Scr->FirstWindow = tmp_win;

    /* get all the colors for the window */

    tmp_win->borderC.fore     = Scr->BorderColorC.fore;
    tmp_win->borderC.back     = Scr->BorderColorC.back;
    tmp_win->border_tile.fore = Scr->BorderTileC.fore;
    tmp_win->border_tile.back = Scr->BorderTileC.back;
    tmp_win->title.fore       = Scr->TitleC.fore;
    tmp_win->title.back       = Scr->TitleC.back;

    GetColorFromList(Scr->BorderColorL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->borderC.fore);
    GetColorFromList(Scr->BorderColorL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->borderC.back);
    GetColorFromList(Scr->BorderTileForegroundL, tmp_win->full_name,
	&tmp_win->class, &tmp_win->border_tile.fore);
    GetColorFromList(Scr->BorderTileBackgroundL, tmp_win->full_name,
	&tmp_win->class, &tmp_win->border_tile.back);
    GetColorFromList(Scr->TitleForegroundL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->title.fore);
    GetColorFromList(Scr->TitleBackgroundL, tmp_win->full_name, &tmp_win->class,
	&tmp_win->title.back);

    if (Scr->use3Dtitles  && !Scr->BeNiceToColormap) GetShadeColors (&tmp_win->title);
    if (Scr->use3Dborders && !Scr->BeNiceToColormap) {
	GetShadeColors (&tmp_win->borderC);
	GetShadeColors (&tmp_win->border_tile);
    }
    /* create windows */

    tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw - tmp_win->frame_bw
			- tmp_win->frame_bw3D;
    tmp_win->frame_y = tmp_win->attr.y - tmp_win->title_height +
	tmp_win->old_bw - tmp_win->frame_bw - tmp_win->frame_bw3D;
    tmp_win->frame_width = tmp_win->attr.width + 2 * tmp_win->frame_bw3D;
    tmp_win->frame_height = tmp_win->attr.height + tmp_win->title_height +
				2 * tmp_win->frame_bw3D;

    ConstrainSize (tmp_win, &tmp_win->frame_width, &tmp_win->frame_height);
    if (random_placed)
        ConstrainByBorders (tmp_win, &tmp_win->frame_x, tmp_win->frame_width,
                            &tmp_win->frame_y, tmp_win->frame_height);

    valuemask = CWBackPixmap | CWBorderPixel | CWCursor | CWEventMask | CWBackPixel;
    attributes.background_pixmap = None;
    attributes.border_pixel = tmp_win->border_tile.back;
    attributes.background_pixel = tmp_win->border_tile.back;
    attributes.cursor = Scr->FrameCursor;
    attributes.event_mask = (SubstructureRedirectMask | 
			     ButtonPressMask | ButtonReleaseMask |
			     EnterWindowMask | LeaveWindowMask | ExposureMask);
    if (Scr->BorderCursors) attributes.event_mask |= PointerMotionMask;
    if (tmp_win->attr.save_under) {
	attributes.save_under = True;
	valuemask |= CWSaveUnder;
    }
    if (tmp_win->hints.flags & PWinGravity) {
	attributes.win_gravity = tmp_win->hints.win_gravity;
	valuemask |= CWWinGravity;
    }

    if ((tmp_win->frame_x > Scr->rootw) ||
	(tmp_win->frame_y > Scr->rooth) ||
	((int)(tmp_win->frame_x + tmp_win->frame_width)  < 0) ||
	((int)(tmp_win->frame_y + tmp_win->frame_height) < 0)) {
      tmp_win->frame_x = 0;
      tmp_win->frame_y = 0;
    }

    DealWithNonSensicalGeometries(dpy, vroot, tmp_win);

    tmp_win->frame = XCreateWindow (dpy, vroot, tmp_win->frame_x, tmp_win->frame_y, 
				    (unsigned int) tmp_win->frame_width,
				    (unsigned int) tmp_win->frame_height,
				    (unsigned int) tmp_win->frame_bw,
				    Scr->d_depth,
				    (unsigned int) CopyFromParent,
				    Scr->d_visual, valuemask, &attributes);
    
    if (tmp_win->title_height)
    {
	valuemask = (CWEventMask | CWDontPropagate | CWBorderPixel | CWBackPixel);
	attributes.event_mask = (KeyPressMask | ButtonPressMask |
				 ButtonReleaseMask | ExposureMask);
	attributes.do_not_propagate_mask = PointerMotionMask;
	attributes.border_pixel = tmp_win->borderC.back;
	attributes.background_pixel = tmp_win->title.back;
	tmp_win->title_w = XCreateWindow (dpy, tmp_win->frame, 
					  tmp_win->frame_bw3D - tmp_win->frame_bw,
					  tmp_win->frame_bw3D - tmp_win->frame_bw,
					  (unsigned int) tmp_win->attr.width, 
					  (unsigned int) Scr->TitleHeight,
					  (unsigned int) tmp_win->frame_bw,
					  Scr->d_depth,
					  (unsigned int) CopyFromParent,
					  Scr->d_visual, valuemask,
					  &attributes);
    }
    else {
	tmp_win->title_w = 0;
	tmp_win->squeeze_info = NULL;
    }

    if (tmp_win->highlight)
    {
	if (Scr->use3Dtitles && (Scr->Monochrome != COLOR))
	    tmp_win->gray = XCreatePixmapFromBitmapData(dpy, vroot, 
		(char*)black_bits, gray_width, gray_height, 
		tmp_win->border_tile.fore, tmp_win->border_tile.back,
		Scr->d_depth);
	else
	    tmp_win->gray = XCreatePixmapFromBitmapData(dpy, vroot, 
		(char*)gray_bits, gray_width, gray_height, 
		tmp_win->border_tile.fore, tmp_win->border_tile.back,
		Scr->d_depth);

	tmp_win->hasfocusvisible = True;
	SetFocusVisualAttributes (tmp_win, False);
    }
    else
	tmp_win->gray = None;

    RaiseWindow(tmp_win);
	
    if (tmp_win->title_w) {
	ComputeTitleLocation (tmp_win);
	CreateWindowTitlebarButtons (tmp_win);
	XMoveWindow (dpy, tmp_win->title_w,
		     tmp_win->title_x, tmp_win->title_y);
	XDefineCursor(dpy, tmp_win->title_w, Scr->TitleCursor);
    }
    else {
	tmp_win->title_x = tmp_win->frame_bw3D - tmp_win->frame_bw;
	tmp_win->title_y = tmp_win->frame_bw3D - tmp_win->frame_bw;
    }

    valuemask = (CWEventMask | CWDontPropagate);
    attributes.event_mask = (StructureNotifyMask | PropertyChangeMask |
			     ColormapChangeMask | VisibilityChangeMask |
			     FocusChangeMask | 
			     EnterWindowMask | LeaveWindowMask);
    attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
    XChangeWindowAttributes (dpy, tmp_win->w, valuemask, &attributes);

    if (HasShape)
	XShapeSelectInput (dpy, tmp_win->w, ShapeNotifyMask);
	
    if (tmp_win->title_w) {
	XMapWindow (dpy, tmp_win->title_w);
    }

    if (HasShape) {
	int xws, yws, xbs, ybs;
	unsigned wws, hws, wbs, hbs;
	int boundingShaped, clipShaped;

	XShapeSelectInput (dpy, tmp_win->w, ShapeNotifyMask);
	XShapeQueryExtents (dpy, tmp_win->w,
			    &boundingShaped, &xws, &yws, &wws, &hws,
			    &clipShaped, &xbs, &ybs, &wbs, &hbs);
	tmp_win->wShaped = boundingShaped;
    }

    if (!tmp_win->iconmgr &&! iswman &&
	(tmp_win->w != Scr->workSpaceMgr.occupyWindow->w))
	XAddToSaveSet(dpy, tmp_win->w);

    XReparentWindow(dpy, tmp_win->w, tmp_win->frame, tmp_win->frame_bw3D,
		tmp_win->title_height + tmp_win->frame_bw3D);
    /*
     * Reparenting generates an UnmapNotify event, followed by a MapNotify.
     * Set the map state to FALSE to prevent a transition back to
     * WithdrawnState in HandleUnmapNotify.  Map state gets set correctly
     * again in HandleMapNotify.
     */
    tmp_win->mapped = FALSE;

    SetupFrame (tmp_win, tmp_win->frame_x, tmp_win->frame_y,
		tmp_win->frame_width, tmp_win->frame_height, -1, True);

    /* wait until the window is iconified and the icon window is mapped
     * before creating the icon window 
     */
    tmp_win->icon = (Icon*) 0;
    tmp_win->iconslist = (name_list*) 0;

    if (!tmp_win->iconmgr)
    {
	GrabButtons(tmp_win);
	GrabKeys(tmp_win);
    }

    (void) AddIconManager(tmp_win);

    XSaveContext(dpy, tmp_win->w, TwmContext, (XPointer) tmp_win);
    XSaveContext(dpy, tmp_win->w, ScreenContext, (XPointer) Scr);
    XSaveContext(dpy, tmp_win->frame, TwmContext, (XPointer) tmp_win);
    XSaveContext(dpy, tmp_win->frame, ScreenContext, (XPointer) Scr);

    if (tmp_win->title_height)
    {
	int i;
	int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

	XSaveContext(dpy, tmp_win->title_w, TwmContext, (XPointer) tmp_win);
	XSaveContext(dpy, tmp_win->title_w, ScreenContext, (XPointer) Scr);
	for (i = 0; i < nb; i++) {
	    XSaveContext(dpy, tmp_win->titlebuttons[i].window, TwmContext,
			 (XPointer) tmp_win);
	    XSaveContext(dpy, tmp_win->titlebuttons[i].window, ScreenContext,
			 (XPointer) Scr);
	}
	if (tmp_win->hilite_wl)
	{
	    XSaveContext(dpy, tmp_win->hilite_wl, TwmContext, (XPointer)tmp_win);
	    XSaveContext(dpy, tmp_win->hilite_wl, ScreenContext, (XPointer)Scr);
	}
	if (tmp_win->hilite_wr)
	{
	    XSaveContext(dpy, tmp_win->hilite_wr, TwmContext, (XPointer)tmp_win);
	    XSaveContext(dpy, tmp_win->hilite_wr, ScreenContext, (XPointer)Scr);
	}
	if (tmp_win->lolite_wl)
	{
	    XSaveContext(dpy, tmp_win->lolite_wl, TwmContext, (XPointer)tmp_win);
	    XSaveContext(dpy, tmp_win->lolite_wl, ScreenContext, (XPointer)Scr);
	}
	if (tmp_win->lolite_wr)
	{
	    XSaveContext(dpy, tmp_win->lolite_wr, TwmContext, (XPointer)tmp_win);
	    XSaveContext(dpy, tmp_win->lolite_wr, ScreenContext, (XPointer)Scr);
	}
    }

    XUngrabServer(dpy);

    /* if we were in the middle of a menu activated function, regrab
     * the pointer 
     */
    if (RootFunction) ReGrab();
    if (!iswman) WMapAddWindow (tmp_win);
    SetPropsIfCaptiveCtwm (tmp_win);
    savegeometry (tmp_win);
    return (tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	GetTwmWindow - finds the TwmWindow structure associated with
 *		a Window (if any), or NULL.
 *
 *  Returned Value:
 *	NULL	- it is not a Window we know about
 *	otherwise- the TwmWindow *
 *
 *  Inputs:
 *	w	- the window to check
 *
 *  Note:
 *  	This is a relatively cheap function since it does not involve
 *  	communication with the server. Probably faster than walking
 *  	the list of TwmWindows, since the lookup is by a hash table.
 *
 ***********************************************************************
 */
TwmWindow *GetTwmWindow(Window w)
{
    TwmWindow *twmwin;
    int stat;

    stat = XFindContext(dpy, w, TwmContext, (XPointer *)&twmwin);
    if (stat == XCNOENT)
	twmwin = NULL;

    return twmwin;
}

/***********************************************************************
 *
 *  Procedure:
 *	MappedNotOverride - checks to see if we should really
 *		put a twm frame on the window
 *
 *  Returned Value:
 *	TRUE	- go ahead and frame the window
 *	FALSE	- don't frame the window
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************
 */

int MappedNotOverride(Window w)
{
    XWindowAttributes wa;

    XGetWindowAttributes(dpy, w, &wa);
    return ((wa.map_state != IsUnmapped) && (wa.override_redirect != True));
}


/***********************************************************************
 *
 *  Procedure:
 *      AddDefaultBindings - attach default bindings so that naive users
 *      don't get messed up if they provide a minimal twmrc.
 */
static void do_add_binding (int button, int context, int modifier, int func)
{
    AddFuncButton (button, context, modifier, func, NULL, NULL);
}

void AddDefaultBindings (void)
{
#define NoModifierMask 0

    do_add_binding (Button1, C_TITLE, NoModifierMask, F_MOVE);
    do_add_binding (Button1, C_ICON, NoModifierMask, F_ICONIFY);
    do_add_binding (Button1, C_ICONMGR, NoModifierMask, F_ICONIFY);

    do_add_binding (Button2, C_TITLE, NoModifierMask, F_RAISELOWER);
    do_add_binding (Button2, C_ICON, NoModifierMask, F_ICONIFY);
    do_add_binding (Button2, C_ICONMGR, NoModifierMask, F_ICONIFY);

#undef NoModifierMask
}




/***********************************************************************
 *
 *  Procedure:
 *	GrabButtons - grab needed buttons for the window
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

#define AltMask (Alt1Mask | Alt2Mask | Alt3Mask | Alt4Mask | Alt5Mask)
#define grabbutton(button, modifier, window, pointer_mode) \
	XGrabButton (dpy, button, modifier, window,  \
		True, ButtonPressMask | ButtonReleaseMask, \
		pointer_mode, GrabModeAsync, None,  \
		Scr->FrameCursor);

void GrabButtons(TwmWindow *tmp_win)
{
    FuncButton *tmp;
    int i;
    unsigned int ModifierMask[8] = { ShiftMask, ControlMask, LockMask,
				     Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask,
				     Mod5Mask };

    for (tmp = Scr->FuncButtonRoot.next; tmp != NULL; tmp = tmp->next) {
	if ((tmp->cont != C_WINDOW) || (tmp->func == 0)) continue;
	grabbutton (tmp->num, tmp->mods, tmp_win->frame, GrabModeAsync);
	    
	if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask)) {
	    grabbutton (tmp->num, tmp->mods | LockMask, tmp_win->frame, GrabModeAsync);
	}
	for (i = 0 ; i < 8 ; i++) {
	    if ((Scr->IgnoreModifier & ModifierMask [i]) && !(tmp->mods & ModifierMask [i]))
		grabbutton (tmp->num, tmp->mods | ModifierMask [i],
			  tmp_win->frame, GrabModeAsync);
	}
    }
    if (Scr->ClickToFocus) {
	grabbutton (AnyButton, None, tmp_win->w, GrabModeSync);
	for (i = 0 ; i < 8 ; i++) {
	    grabbutton (AnyButton, ModifierMask [i], tmp_win->w, GrabModeSync);
	}
    } else
    if (Scr->RaiseOnClick) {
	grabbutton (Scr->RaiseOnClickButton, None, tmp_win->w, GrabModeSync);
	for (i = 0 ; i < 8 ; i++) {
	    grabbutton (Scr->RaiseOnClickButton,
			ModifierMask [i], tmp_win->w, GrabModeSync);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabKeys - grab needed keys for the window
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

#define MAX_KEYCODE 256
#define grabkey(funckey, modifier, window) \
	XGrabKey(dpy, funckey->keycode, funckey->mods | modifier, window, True, \
		GrabModeAsync, GrabModeAsync);
#define ungrabkey(funckey, modifier, window) \
	XUngrabKey (dpy, funckey->keycode, funckey->mods | modifier, window);

void GrabKeys(TwmWindow *tmp_win)
{
    FuncKey *tmp;
    IconMgr *p;
    int i;
    unsigned int ModifierMask[8] = { ShiftMask, ControlMask, LockMask,
				     Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask,
				     Mod5Mask };

    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	switch (tmp->cont)
	{
	case C_WINDOW:
	/* case C_WORKSPACE: */
	    if (tmp->mods & AltMask) break;
	    grabkey (tmp, 0, tmp_win->w);
	    if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask))
		grabkey (tmp, LockMask, tmp_win->w);
	    for (i = 0 ; i < 8 ; i++) {
		if ((Scr->IgnoreModifier & ModifierMask [i]) &&
		    !(tmp->mods & ModifierMask [i]))
		  grabkey (tmp, ModifierMask [i], tmp_win->w);
	    }
	    break;

	case C_ICON:
	    if (!tmp_win->icon || tmp_win->icon->w) break;
	    grabkey (tmp, 0, tmp_win->icon->w);
	    if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask))
		grabkey (tmp, LockMask, tmp_win->icon->w);
	    for (i = 0 ; i < 8 ; i++) {
		if ((Scr->IgnoreModifier & ModifierMask [i]) &&
		    !(tmp->mods & ModifierMask [i]))
		    grabkey (tmp, ModifierMask [i], tmp_win->icon->w);
	    }
	    break;

	case C_TITLE:
	    if (!tmp_win->title_w) break;
	    grabkey (tmp, 0, tmp_win->title_w);
	    if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask))
		grabkey (tmp, LockMask, tmp_win->title_w);
	    for (i = 0 ; i < 8 ; i++) {
		if ((Scr->IgnoreModifier & ModifierMask [i]) &&
		    !(tmp->mods & ModifierMask [i]))
		    grabkey (tmp, ModifierMask [i], tmp_win->title_w);
	    }
	    break;

	case C_NAME:
	    grabkey (tmp, 0, tmp_win->w);
	    if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask)) {
		grabkey (tmp, LockMask, tmp_win->w);
	    }
	    for (i = 0 ; i < 8 ; i++) {
		if ((Scr->IgnoreModifier & ModifierMask [i]) &&
		    !(tmp->mods & ModifierMask [i]))
		    grabkey (tmp, ModifierMask [i], tmp_win->w);
	    }
	    if (tmp_win->icon && tmp_win->icon->w) {
		grabkey (tmp, 0, tmp_win->icon->w);
		if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask))
		    grabkey (tmp, LockMask, tmp_win->icon->w);
		for (i = 0 ; i < 8 ; i++) {
		    if ((Scr->IgnoreModifier & ModifierMask [i]) &&
			!(tmp->mods & ModifierMask [i]))
			grabkey (tmp, ModifierMask [i], tmp_win->icon->w);
		}
	    }
	    if (tmp_win->title_w) {
		grabkey (tmp, 0, tmp_win->title_w);
		if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask))
		    grabkey (tmp, LockMask, tmp_win->title_w);
		for (i = 0 ; i < 8 ; i++) {
		    if ((Scr->IgnoreModifier & ModifierMask [i]) &&
			!(tmp->mods & ModifierMask [i]))
			grabkey (tmp, ModifierMask [i], tmp_win->title_w);
		}
	    }
	    break;
	/*
	case C_ROOT:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, Scr->Root, True,
		GrabModeAsync, GrabModeAsync);
	    break;
	*/
	}
    }
    for (tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->cont == C_ICONMGR && !Scr->NoIconManagers)
	{
	    for (p = Scr->iconmgr; p != NULL; p = p->next) {
		ungrabkey (tmp, 0, p->twm_win->w);
		if (Scr->IgnoreLockModifier && !(tmp->mods & LockMask))
		    ungrabkey (tmp, LockMask, p->twm_win->w);
		for (i = 0 ; i < 8 ; i++) {
		    if ((Scr->IgnoreModifier & ModifierMask [i]) &&
			!(tmp->mods & ModifierMask [i]))
			ungrabkey (tmp, ModifierMask [i], p->twm_win->w);
		}
	    }
	}
    }
}

void ComputeCommonTitleOffsets (void)
{
    int buttonwidth = (Scr->TBInfo.width + Scr->TBInfo.pad);

    Scr->TBInfo.leftx = Scr->TBInfo.rightoff = Scr->FramePadding;
    if (Scr->TBInfo.nleft  > 0) Scr->TBInfo.leftx    += Scr->ButtonIndent;
    if (Scr->TBInfo.nright > 0) Scr->TBInfo.rightoff += (Scr->ButtonIndent +
			       (Scr->TBInfo.nright * buttonwidth) -
				Scr->TBInfo.pad);

    Scr->TBInfo.titlex = (Scr->TBInfo.leftx +
				(Scr->TBInfo.nleft * buttonwidth) -
				Scr->TBInfo.pad +
				Scr->TitlePadding);
}

static void CreateHighlightWindows (TwmWindow *tmp_win)
{
    XSetWindowAttributes attributes;	/* attributes for create windows */
    GC gc;
    XGCValues gcv;
    unsigned long valuemask;
    int h = (Scr->TitleHeight - 2 * Scr->FramePadding);
    int y = Scr->FramePadding;

    if (! tmp_win->titlehighlight) {
	tmp_win->hilite_wl = (Window) 0;
	tmp_win->hilite_wr = (Window) 0;
	return;
    }
    /*
     * If a special highlight pixmap was given, use that.  Otherwise,
     * use a nice, even gray pattern.  The old horizontal lines look really
     * awful on interlaced monitors (as well as resembling other looks a
     * little bit too closely), but can be used by putting
     *
     *                 Pixmaps { TitleHighlight "hline2" }
     *
     * (or whatever the horizontal line bitmap is named) in the startup
     * file.  If all else fails, use the foreground color to look like a 
     * solid line.
     */

    if (! tmp_win->HiliteImage) {
	if (Scr->HighlightPixmapName) {
	    tmp_win->HiliteImage = GetImage (Scr->HighlightPixmapName, tmp_win->title);
	}
    }
    if (! tmp_win->HiliteImage) {
	Pixmap pm = None;
	Pixmap bm = None;

	if (Scr->use3Dtitles && (Scr->Monochrome != COLOR))
	    bm = XCreateBitmapFromData (dpy, tmp_win->title_w, 
					(char*)black_bits, gray_width, gray_height);
	else
	    bm = XCreateBitmapFromData (dpy, tmp_win->title_w, 
					(char*)gray_bits, gray_width, gray_height);

	pm = XCreatePixmap (dpy, tmp_win->title_w, gray_width, gray_height, Scr->d_depth);
	gcv.foreground = tmp_win->title.fore;
	gcv.background = tmp_win->title.back;
	gcv.graphics_exposures = False;
	gc = XCreateGC (dpy, pm, (GCForeground|GCBackground|GCGraphicsExposures), &gcv);
	if (gc) {
	    XCopyPlane (dpy, bm, pm, gc, 0, 0, gray_width, gray_height, 0, 0, 1);
	    tmp_win->HiliteImage = (Image*) malloc (sizeof (struct _Image));
	    tmp_win->HiliteImage->pixmap = pm;
	    tmp_win->HiliteImage->width  = gray_width;
	    tmp_win->HiliteImage->height = gray_height;
	    tmp_win->HiliteImage->mask   = None;
	    tmp_win->HiliteImage->next   = None;
	    XFreeGC (dpy, gc);
	} else {
	    XFreePixmap (dpy, pm);
	    pm = None;
	}
	XFreePixmap (dpy, bm);
    }
    if (tmp_win->HiliteImage) {
	valuemask = CWBackPixmap;
	attributes.background_pixmap = tmp_win->HiliteImage->pixmap;
    } else {
	valuemask = CWBackPixel;
	attributes.background_pixel = tmp_win->title.fore;
    }

    if (Scr->use3Dtitles) {
	y += Scr->TitleShadowDepth;
	h -= 2 * Scr->TitleShadowDepth;
    }
    if (Scr->TitleJustification == J_LEFT)
	tmp_win->hilite_wl = (Window) 0;
    else
	tmp_win->hilite_wl = XCreateWindow (dpy, tmp_win->title_w, 0, y,
		       (unsigned int) Scr->TBInfo.width, (unsigned int) h,
		       (unsigned int) 0, Scr->d_depth, (unsigned int) CopyFromParent,
		       Scr->d_visual, valuemask, &attributes);

    if (Scr->TitleJustification == J_RIGHT)
	tmp_win->hilite_wr = (Window) 0;
    else
	tmp_win->hilite_wr = XCreateWindow (dpy, tmp_win->title_w, 0, y,
		       (unsigned int) Scr->TBInfo.width, (unsigned int) h,
		       (unsigned int) 0,  Scr->d_depth, (unsigned int) CopyFromParent,
		       Scr->d_visual, valuemask, &attributes);
}

void DeleteHighlightWindows(TwmWindow *tmp_win)
{
    if (tmp_win->HiliteImage) {
	if (Scr->HighlightPixmapName) {
	    /*
	     * Image obtained from GetImage(): it is in a cache
	     * so we don't need to free it. There will not be multiple
	     * copies if the same xpm:foo image is requested again.
	     */
	} else {
	    XFreePixmap (dpy, tmp_win->HiliteImage->pixmap);
	    free(tmp_win->HiliteImage);
	}
	tmp_win->HiliteImage = NULL;
    }
}

static void CreateLowlightWindows (TwmWindow *tmp_win)
{
    XSetWindowAttributes attributes;    /* attributes for create windows */
    unsigned long valuemask;
    int h = (Scr->TitleHeight - 2 * Scr->FramePadding);
    int y = Scr->FramePadding;
    ColorPair cp;

    if (!Scr->UseSunkTitlePixmap || ! tmp_win->titlehighlight) {
	tmp_win->lolite_wl = (Window) 0;
	tmp_win->lolite_wr = (Window) 0;
	return;
    }
    /*
     * If a special highlight pixmap was given, use that.  Otherwise,
     * use a nice, even gray pattern.  The old horizontal lines look really
     * awful on interlaced monitors (as well as resembling other looks a
     * little bit too closely), but can be used by putting
     *
     *                 Pixmaps { TitleHighlight "hline2" }
     *
     * (or whatever the horizontal line bitmap is named) in the startup
     * file.  If all else fails, use the foreground color to look like a
     * solid line.
     */

    if (! tmp_win->LoliteImage) {
        if (Scr->HighlightPixmapName) {
            cp = tmp_win->title;
            cp.shadc = tmp_win->title.shadd;
            cp.shadd = tmp_win->title.shadc;
            tmp_win->LoliteImage = GetImage (Scr->HighlightPixmapName, cp);
        }
    }
    if (tmp_win->LoliteImage) {
        valuemask = CWBackPixmap;
        attributes.background_pixmap = tmp_win->LoliteImage->pixmap;
    } else {
        valuemask = CWBackPixel;
        attributes.background_pixel = tmp_win->title.fore;
    }

    if (Scr->use3Dtitles) {
        y += 2;
        h -= 4;
    }
    if (Scr->TitleJustification == J_LEFT)
        tmp_win->lolite_wl = (Window) 0;
    else
        tmp_win->lolite_wl = XCreateWindow (dpy, tmp_win->title_w, 0, y,
                       (unsigned int) Scr->TBInfo.width, (unsigned int) h,
                       (unsigned int) 0, Scr->d_depth, (unsigned int) CopyFromParent,
                       Scr->d_visual, valuemask, &attributes);

    if (Scr->TitleJustification == J_RIGHT)
        tmp_win->lolite_wr = (Window) 0;
    else
        tmp_win->lolite_wr = XCreateWindow (dpy, tmp_win->title_w, 0, y,
                       (unsigned int) Scr->TBInfo.width, (unsigned int) h,
                       (unsigned int) 0,  Scr->d_depth, (unsigned int) CopyFromParent,
                       Scr->d_visual, valuemask, &attributes);
}


void ComputeWindowTitleOffsets (TwmWindow *tmp_win, unsigned int width,
				Bool squeeze)
{
    int titlew = width - Scr->TBInfo.titlex - Scr->TBInfo.rightoff;

    switch (Scr->TitleJustification) {
	case J_LEFT :
	    tmp_win->name_x = Scr->TBInfo.titlex;
	    if (Scr->use3Dtitles) tmp_win->name_x += Scr->TitleShadowDepth + 2;
	    break;
	case J_CENTER :
	    tmp_win->name_x = Scr->TBInfo.titlex + (titlew - tmp_win->name_width) / 2;
	    break;
	case J_RIGHT :
	    tmp_win->name_x = Scr->TBInfo.titlex + titlew - tmp_win->name_width;
	    if (Scr->use3Dtitles) tmp_win->name_x -= Scr->TitleShadowDepth - 2;
	    break;
    }
    if (Scr->use3Dtitles) {
	if (tmp_win->name_x < (Scr->TBInfo.titlex + 2 * Scr->TitleShadowDepth))
		tmp_win->name_x = Scr->TBInfo.titlex + 2 * Scr->TitleShadowDepth;
    }
    else
    if (tmp_win->name_x < Scr->TBInfo.titlex) {
	tmp_win->name_x = Scr->TBInfo.titlex;
    }
    tmp_win->highlightxl = Scr->TBInfo.titlex;
    tmp_win->highlightxr = tmp_win->name_x + tmp_win->name_width + 2;

    if (Scr->use3Dtitles) {
	tmp_win->highlightxl += Scr->TitleShadowDepth;
    }
    if (tmp_win->hilite_wr || Scr->TBInfo.nright > 0) 
      tmp_win->highlightxr += Scr->TitlePadding;
    tmp_win->rightx = width - Scr->TBInfo.rightoff;
    if (squeeze && tmp_win->squeeze_info && !tmp_win->squeezed) {
	int rx = (tmp_win->highlightxr + 
		  (tmp_win->hilite_wr
		    ? Scr->TBInfo.width * 2 : 0) +
		  (Scr->TBInfo.nright > 0 ? Scr->TitlePadding : 0) +
		  Scr->FramePadding);
	if (rx < tmp_win->rightx) tmp_win->rightx = rx;
    }
    return;
}


/*
 * ComputeTitleLocation - calculate the position of the title window; we need
 * to take the frame_bw into account since we want (0,0) of the title window
 * to line up with (0,0) of the frame window.
 */
void ComputeTitleLocation (register TwmWindow *tmp)
{
    tmp->title_x = tmp->frame_bw3D - tmp->frame_bw;
    tmp->title_y = tmp->frame_bw3D - tmp->frame_bw;

    if (tmp->squeeze_info && !tmp->squeezed) {
	register SqueezeInfo *si = tmp->squeeze_info;
	int basex;
	int maxwidth = tmp->frame_width;
	int tw = tmp->title_width + 2 * tmp->frame_bw3D;

	/*
	 * figure label base from squeeze info (justification fraction)
	 */
	if (si->denom == 0) {		/* num is pixel based */
	    basex = si->num;
	} else {			/* num/denom is fraction */
	    basex = ((si->num * maxwidth) / si->denom);
	}
	if (si->num < 0)
	    basex += maxwidth;

	/*
	 * adjust for left (nop), center, right justify and clip
	 */
	switch (si->justify) {
	  case J_CENTER:
	    basex -= tw / 2;
	    break;
	  case J_RIGHT:
	    basex -= tw - 1;
	    break;
	}
	if (basex > maxwidth - tw)
	  basex = maxwidth - tw;
	if (basex < 0) basex = 0;

	tmp->title_x = basex - tmp->frame_bw + tmp->frame_bw3D;
    }
}


static void CreateWindowTitlebarButtons (TwmWindow *tmp_win)
{
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int leftx, rightx, y;
    TitleButton *tb;
    int nb;

    if (tmp_win->title_height == 0)
    {
	tmp_win->hilite_wl = (Window) 0;
	tmp_win->hilite_wr = (Window) 0;
	tmp_win->lolite_wl = (Window) 0;
	tmp_win->lolite_wr = (Window) 0;
	return;
    }


    /*
     * create the title bar windows; let the event handler deal with painting
     * so that we don't have to spend two pixmaps (or deal with hashing)
     */
    ComputeWindowTitleOffsets (tmp_win, tmp_win->attr.width, False);

    leftx = y = Scr->TBInfo.leftx;
    rightx = tmp_win->rightx;

    attributes.win_gravity = NorthWestGravity;
    attributes.background_pixel = tmp_win->title.back;
    attributes.border_pixel = tmp_win->title.fore;
    attributes.event_mask = (ButtonPressMask | ButtonReleaseMask |
			     ExposureMask);
    attributes.cursor = Scr->ButtonCursor;
    valuemask = (CWWinGravity | CWBackPixel | CWBorderPixel | CWEventMask |
		 CWCursor);

    tmp_win->titlebuttons = NULL;
    nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;
    if (nb > 0) {
	tmp_win->titlebuttons = (TBWindow *) malloc (nb * sizeof(TBWindow));
	if (!tmp_win->titlebuttons) {
	    fprintf (stderr, "%s:  unable to allocate %d titlebuttons\n", 
		     ProgramName, nb);
	} else {
	    TBWindow *tbw;
	    int boxwidth = (Scr->TBInfo.width + Scr->TBInfo.pad);
	    unsigned int h = (Scr->TBInfo.width - Scr->TBInfo.border * 2);

	    for (tb = Scr->TBInfo.head, tbw = tmp_win->titlebuttons; tb;
		 tb = tb->next, tbw++) {
		int x;
		if (tb->rightside) {
		    x = rightx;
		    rightx += boxwidth;
		    attributes.win_gravity = NorthEastGravity;
		} else {
		    x = leftx;
		    leftx += boxwidth;
		    attributes.win_gravity = NorthWestGravity;
		}
		tbw->window = XCreateWindow (dpy, tmp_win->title_w, x, y, h, h,
					     (unsigned int) Scr->TBInfo.border,
					     0, (unsigned int) CopyFromParent,
					     (Visual *) CopyFromParent,
					     valuemask, &attributes);
		tbw->image = GetImage (tb->name, tmp_win->title);
		if (! tbw->image) {
		    tbw->image = GetImage (TBPM_QUESTION, tmp_win->title);
		    if (! tbw->image) {		/* cannot happen (see util.c) */
			fprintf (stderr, "%s:  unable to add titlebar button \"%s\"\n",
				 ProgramName, tb->name);
		    }
		}
		tbw->info = tb;
	    }
	}
    }

    CreateHighlightWindows (tmp_win);
    CreateLowlightWindows  (tmp_win);
    XMapSubwindows(dpy, tmp_win->title_w);
    if (tmp_win->hilite_wl) XUnmapWindow(dpy, tmp_win->hilite_wl);
    if (tmp_win->hilite_wr) XUnmapWindow(dpy, tmp_win->hilite_wr);
    if (tmp_win->lolite_wl) XMapWindow(dpy, tmp_win->lolite_wl);
    if (tmp_win->lolite_wr) XMapWindow(dpy, tmp_win->lolite_wr);
    return;
}

void SetHighlightPixmap (char *filename)
{
#ifdef VMS
    char *ftemp;
    ftemp = (char *) malloc((strlen(filename)+1)*sizeof(char));
    Scr->HighlightPixmapName = strcpy (ftemp,filename);
#else
    Scr->HighlightPixmapName = (char*) strdup (filename);
#endif
}


void FetchWmProtocols (TwmWindow *tmp)
{
    unsigned long flags = 0L;
    Atom *protocols = NULL;
    int n;

    if (XGetWMProtocols (dpy, tmp->w, &protocols, &n)) {
	register int i;
	register Atom *ap;

	for (i = 0, ap = protocols; i < n; i++, ap++) {
	    if (*ap == _XA_WM_TAKE_FOCUS) flags |= DoesWmTakeFocus;
	    if (*ap == _XA_WM_SAVE_YOURSELF) flags |= DoesWmSaveYourself;
	    if (*ap == _XA_WM_DELETE_WINDOW) flags |= DoesWmDeleteWindow;
	}
	if (protocols) XFree ((char *) protocols);
    }
    tmp->protocols = flags;
}

TwmColormap *CreateTwmColormap(Colormap c)
{
    TwmColormap *cmap;
    cmap = (TwmColormap *) malloc(sizeof(TwmColormap));
    if (!cmap ||
	XSaveContext(dpy, c, ColormapContext, (XPointer) cmap)) {
	if (cmap) free((char *) cmap);
	return (NULL);
    }
    cmap->c = c;
    cmap->state = 0;
    cmap->install_req = 0;
    cmap->w = None;
    cmap->refcnt = 1;
    return (cmap);
}

ColormapWindow *CreateColormapWindow(Window w,
				     Bool creating_parent,
				     Bool property_window)
{
    ColormapWindow *cwin;
    TwmColormap *cmap;
    XWindowAttributes attributes;

    cwin = (ColormapWindow *) malloc(sizeof(ColormapWindow));
    if (cwin) {
	if (!XGetWindowAttributes(dpy, w, &attributes) ||
	    XSaveContext(dpy, w, ColormapContext, (XPointer) cwin)) {
	    free((char *) cwin);
	    return (NULL);
	}

	if (XFindContext(dpy, attributes.colormap,  ColormapContext,
		(XPointer *)&cwin->colormap) == XCNOENT) {
	    cwin->colormap = cmap = CreateTwmColormap(attributes.colormap);
	    if (!cmap) {
		XDeleteContext(dpy, w, ColormapContext);
		free((char *) cwin);
		return (NULL);
	    }
	} else {
	    cwin->colormap->refcnt++;
	}

	cwin->w = w;
	/*
	 * Assume that windows in colormap list are
	 * obscured if we are creating the parent window.
	 * Otherwise, we assume they are unobscured.
	 */
	cwin->visibility = creating_parent ?
	    VisibilityPartiallyObscured : VisibilityUnobscured;
	cwin->refcnt = 1;

	/*
	 * If this is a ColormapWindow property window and we
	 * are not monitoring ColormapNotify or VisibilityNotify
	 * events, we need to.
	 */
	if (property_window &&
	    (attributes.your_event_mask &
		(ColormapChangeMask|VisibilityChangeMask)) !=
		    (ColormapChangeMask|VisibilityChangeMask)) {
	    XSelectInput(dpy, w, attributes.your_event_mask |
		(ColormapChangeMask|VisibilityChangeMask));
	}
    }

    return (cwin);
}

int FetchWmColormapWindows (TwmWindow *tmp)
{
    register int i, j;
    Window *cmap_windows = NULL;
    Bool can_free_cmap_windows = False;
    int number_cmap_windows = 0;
    ColormapWindow **cwins = NULL;
    int previously_installed;

    number_cmap_windows = 0;

    if (/* SUPPRESS 560 */
	(previously_installed =
	 (Scr->cmapInfo.cmaps == &tmp->cmaps && tmp->cmaps.number_cwins))) {
	cwins = tmp->cmaps.cwins;
	for (i = 0; i < tmp->cmaps.number_cwins; i++)
	    cwins[i]->colormap->state = 0;
    }

    if (XGetWMColormapWindows (dpy, tmp->w, &cmap_windows, 
			       &number_cmap_windows) &&
	number_cmap_windows > 0) {

	can_free_cmap_windows = False;
	/*
	 * check if the top level is in the list, add to front if not
	 */
	for (i = 0; i < number_cmap_windows; i++) {
	    if (cmap_windows[i] == tmp->w) break;
	}
	if (i == number_cmap_windows) {	 /* not in list */
	    Window *new_cmap_windows =
	      (Window *) malloc (sizeof(Window) * (number_cmap_windows + 1));

	    if (!new_cmap_windows) {
		fprintf (stderr, 
			 "%s:  unable to allocate %d element colormap window array\n",
			ProgramName, number_cmap_windows+1);
		goto done;
	    }
	    new_cmap_windows[0] = tmp->w;  /* add to front */
	    for (i = 0; i < number_cmap_windows; i++) {	 /* append rest */
		new_cmap_windows[i+1] = cmap_windows[i];
	    }
	    XFree ((char *) cmap_windows);
	    can_free_cmap_windows = True;  /* do not use XFree any more */
	    cmap_windows = new_cmap_windows;
	    number_cmap_windows++;
	}

	cwins = (ColormapWindow **) malloc(sizeof(ColormapWindow *) *
		number_cmap_windows);
	if (cwins) {
	    for (i = 0; i < number_cmap_windows; i++) {

		/*
		 * Copy any existing entries into new list.
		 */
		for (j = 0; j < tmp->cmaps.number_cwins; j++) {
		    if (tmp->cmaps.cwins[j]->w == cmap_windows[i]) {
			cwins[i] = tmp->cmaps.cwins[j];
			cwins[i]->refcnt++;
			break;
		    }
		}

		/*
		 * If the colormap window is not being pointed by
		 * some other applications colormap window list,
		 * create a new entry.
		 */
		if (j == tmp->cmaps.number_cwins) {
		    if (XFindContext(dpy, cmap_windows[i], ColormapContext,
				     (XPointer *)&cwins[i]) == XCNOENT) {
			if ((cwins[i] = CreateColormapWindow(cmap_windows[i],
				    (Bool) tmp->cmaps.number_cwins == 0,
				    True)) == NULL) {
			    int k;
			    for (k = i + 1; k < number_cmap_windows; k++)
				cmap_windows[k-1] = cmap_windows[k];
			    i--;
			    number_cmap_windows--;
			}
		    } else
			cwins[i]->refcnt++;
		}
	    }
	}
    }

    /* No else here, in case we bailed out of clause above.
     */
    if (number_cmap_windows == 0) {

	number_cmap_windows = 1;

	cwins = (ColormapWindow **) malloc(sizeof(ColormapWindow *));
	if (XFindContext(dpy, tmp->w, ColormapContext, (XPointer *)&cwins[0]) ==
		XCNOENT)
	    cwins[0] = CreateColormapWindow(tmp->w,
			    (Bool) tmp->cmaps.number_cwins == 0, False);
	else
	    cwins[0]->refcnt++;
    }

    if (tmp->cmaps.number_cwins)
	free_cwins(tmp);

    tmp->cmaps.cwins = cwins;
    tmp->cmaps.number_cwins = number_cmap_windows;
    if (number_cmap_windows > 1)
	tmp->cmaps.scoreboard = 
	  (char *) calloc(1, ColormapsScoreboardLength(&tmp->cmaps));
		
    if (previously_installed) {
	InstallColormaps(PropertyNotify, NULL);
    }

  done:
    if (cmap_windows) {
	if (can_free_cmap_windows)
	  free ((char *) cmap_windows);
	else
	  XFree ((char *) cmap_windows);
    }

    return (0);
}


void GetWindowSizeHints (TwmWindow *tmp)
{
    long supplied = 0;
    XSizeHints *hints = &tmp->hints;

    if (!XGetWMNormalHints (dpy, tmp->w, hints, &supplied))
      hints->flags = 0;

    if (hints->flags & PResizeInc) {
	if (hints->width_inc == 0) hints->width_inc = 1;
	if (hints->height_inc == 0) hints->height_inc = 1;
    }

    if (!(supplied & PWinGravity) && (hints->flags & USPosition)) {
	static int gravs[] = { SouthEastGravity, SouthWestGravity,
			       NorthEastGravity, NorthWestGravity };
	int right =  tmp->attr.x + tmp->attr.width + 2 * tmp->old_bw;
	int bottom = tmp->attr.y + tmp->attr.height + 2 * tmp->old_bw;
	hints->win_gravity = 
	  gravs[((Scr->rooth - bottom <
		tmp->title_height + 2 * tmp->frame_bw3D) ? 0 : 2) |
		((Scr->rootw - right   <
		tmp->title_height + 2 * tmp->frame_bw3D) ? 0 : 1)];
	hints->flags |= PWinGravity;
    }

    /* Check for min size < max size */
    if ((hints->flags & (PMinSize|PMaxSize)) == (PMinSize|PMaxSize)) {
	if (hints->max_width < hints->min_width) {
	    if (hints->max_width > 0) {
		hints->min_width = hints->max_width;
	    } else if (hints->min_width > 0) {
		hints->max_width = hints->min_width;
	    } else {
		hints->max_width = hints->min_width = 1;
	    }
	}

	if (hints->max_height < hints->min_height) {
	    if (hints->max_height > 0) {
		hints->min_height = hints->max_height;
	    } else if (hints->min_height > 0) {
		hints->max_height = hints->min_height;
	    } else {
		hints->max_height = hints->min_height = 1;
	    }
	}
    }
}

void AnimateButton (TBWindow *tbw)
{
    Image	*image;
    XSetWindowAttributes attr;

    image = tbw->image;
    attr.background_pixmap = image->pixmap;
    XChangeWindowAttributes (dpy, tbw->window, CWBackPixmap, &attr);
    XClearWindow (dpy, tbw->window);
    tbw->image = image->next;
}

void AnimateHighlight (TwmWindow *t)
{
    Image	*image;
    XSetWindowAttributes attr;

    image = t->HiliteImage;
    attr.background_pixmap = image->pixmap;
    if (t->hilite_wl) {
	XChangeWindowAttributes (dpy, t->hilite_wl, CWBackPixmap, &attr);
	XClearWindow (dpy, t->hilite_wl);
    }
    if (t->hilite_wr) {
	XChangeWindowAttributes (dpy, t->hilite_wr, CWBackPixmap, &attr);
	XClearWindow (dpy, t->hilite_wr);
    }
    t->HiliteImage = image->next;
}

name_list **AddWindowRegion (char *geom, int grav1, int grav2)
{
    WindowRegion *wr;
    int mask;

    wr = (WindowRegion*) malloc (sizeof (WindowRegion));
    wr->next = NULL;

    if (!Scr->FirstWindowRegion) Scr->FirstWindowRegion = wr;

    wr->entries    = NULL;
    wr->clientlist = NULL;
    wr->grav1      = grav1;
    wr->grav2      = grav2;
    wr->x = wr->y = wr->w = wr->h = 0;

    mask = XParseGeometry (geom, &wr->x, &wr->y, (unsigned int*) &wr->w,
						 (unsigned int*) &wr->h);

    if (mask & XNegative) wr->x += Scr->rootw - wr->w;
    if (mask & YNegative) wr->y += Scr->rooth - wr->h;

    return (&(wr->clientlist));
}

void CreateWindowRegions (void) {
    WindowRegion  *wr, *wr1 = NULL, *wr2 = NULL;
    WorkSpace *wl;

    for (wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
	wl->FirstWindowRegion = NULL;
	wr2 = NULL;
	for (wr = Scr->FirstWindowRegion; wr != NULL; wr = wr->next) {
	    wr1  = (WindowRegion*) malloc (sizeof (WindowRegion));
	    *wr1 = *wr;
	    wr1->entries = (WindowEntry*) malloc (sizeof (WindowEntry));
	    wr1->entries->next = 0;
	    wr1->entries->x = wr1->x;
	    wr1->entries->y = wr1->y;
	    wr1->entries->w = wr1->w;
	    wr1->entries->h = wr1->h;
	    wr1->entries->twm_win = (TwmWindow*) 0;
	    wr1->entries->used = 0;
	    if (wr2) wr2->next = wr1; else wl->FirstWindowRegion = wr1;
	    wr2 = wr1;
	}
	if (wr1) wr1->next = NULL;
    }
}


Bool PlaceWindowInRegion (TwmWindow *tmp_win, int *final_x, int *final_y)
{
    WindowRegion  *wr;
    WindowEntry	  *we;
    int		  w, h;
    WorkSpace     *wl;

    if (!Scr->FirstWindowRegion) return (False);
    for (wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
	if (OCCUPY (tmp_win, wl)) break;
    }
    if (!wl) return (False);
    w = tmp_win->frame_width;
    h = tmp_win->frame_height;
    we = (WindowEntry*) 0;
    for (wr = wl->FirstWindowRegion; wr; wr = wr->next) {
	if (LookInList (wr->clientlist, tmp_win->full_name, &tmp_win->class)) {
	    for (we = wr->entries; we; we=we->next) {
	        if (we->used) continue;
	        if (we->w >= w && we->h >= h) break;
	    }
	    if (we) break;
	}
    }
    tmp_win->wr = (WindowRegion*) 0;
    if (!we) return (False);

    splitWindowRegionEntry (we, wr->grav1, wr->grav2, w, h);
    we->used = 1;
    we->twm_win = tmp_win;
    *final_x = we->x;
    *final_y = we->y;
    tmp_win->wr = wr;
    return (True);
}

static void splitWindowRegionEntry (WindowEntry *we, int grav1, int grav2,
				    int w, int h)
{
    WindowEntry	*new;

    switch (grav1) {
	case D_NORTH:
	case D_SOUTH:
	    if (w != we->w) splitWindowRegionEntry (we, grav2, grav1, w, we->h);
	    if (h != we->h) {
		new = (WindowEntry *) malloc (sizeof (WindowEntry));
		new->twm_win = 0;
		new->used = 0;
		new->next = we->next;
		we->next  = new;
		new->x    = we->x;
		new->h    = (we->h - h);
		new->w    = we->w;
		we->h     = h;
		if (grav1 == D_SOUTH) {
		    new->y = we->y;
		    we->y  = new->y + new->h;
		} else
		    new->y = we->y + we->h;
		}
	    break;
	case D_EAST:
	case D_WEST:
	    if (h != we->h) splitWindowRegionEntry (we, grav2, grav1, we->w, h);
	    if (w != we->w) {
		new = (WindowEntry *) malloc (sizeof (WindowEntry));
		new->twm_win = 0;
		new->used = 0;
		new->next = we->next;
		we->next  = new;
		new->y    = we->y;
		new->w    = (we->w - w);
		new->h    = we->h;
		we->w = w;
		if (grav1 == D_EAST) {
		    new->x = we->x;
		    we->x  = new->x + new->w;
		} else
		    new->x = we->x + we->w;
		}
	    break;
    }
}

static WindowEntry *findWindowEntry (WorkSpace *wl, TwmWindow *tmp_win,
				     WindowRegion **wrp)
{
    WindowRegion *wr;
    WindowEntry	 *we;

    for (wr = wl->FirstWindowRegion; wr; wr = wr->next) {
	for (we = wr->entries; we; we=we->next) {
	    if (we->twm_win == tmp_win) {
		if (wrp) *wrp = wr;
		return we;
	    }
	}
    }
    return (WindowEntry*) 0;
}

static WindowEntry *prevWindowEntry (WindowEntry *we, WindowRegion *wr)
{
    WindowEntry	*wp;

    if (we == wr->entries) return 0;
    for (wp = wr->entries; wp->next != we; wp=wp->next);
    return wp;
}

static void mergeWindowEntries (WindowEntry *old, WindowEntry *we)
{
    if (old->y == we->y) {
	we->w = old->w + we->w;
	if (old->x < we->x) we->x = old->x;
    } else {
	we->h = old->h + we->h;
	if (old->y < we->y) we->y = old->y;
    }
}

void RemoveWindowFromRegion (TwmWindow *tmp_win)
{
    WindowEntry  *we, *wp, *wn;
    WindowRegion *wr;
    WorkSpace    *wl;

    if (!Scr->FirstWindowRegion) return;
    we = (WindowEntry*) 0;
    for (wl = Scr->workSpaceMgr.workSpaceList; wl != NULL; wl = wl->next) {
	we = findWindowEntry (wl, tmp_win, &wr);
	if (we) break;
    }
    if (!we) return;

    we->twm_win = 0;
    we->used = 0;
    wp = prevWindowEntry (we, wr);
    wn = we->next;
    for (;;) {
	if (wp && wp->used == 0 &&
	       ((wp->x == we->x && wp->w == we->w) ||
	        (wp->y == we->y && wp->h == we->h))) {
	    wp->next = we->next;
	    mergeWindowEntries (we, wp);
	    free ((char *) we);
	    we = wp;
	    wp = prevWindowEntry (wp, wr);
	} else
	if (wn && wn->used == 0 &&
	       ((wn->x == we->x && wn->w == we->w) ||
	        (wn->y == we->y && wn->h == we->h))) {
	    we->next = wn->next;
	    mergeWindowEntries (wn, we);
	    free ((char *) wn);
	    wn = we->next;
	} else break;
    }
}

/*
 * This is largely for Xinerama support with VirtualScreens.
 * In this case, windows may be on something other then the main screen
 * on startup, or the mapping may be relative to the right side of the
 * screen, which is on a different monitor, which will cause issues with
 * the virtual screens.
 *
 * It probably needs to be congnizant of windows that are actually owned by
 * other workspaces, and ignore them (this needs to be revisited), or perhaps
 * that functionality is appropriate in AddWindow().  This needs to be dug into
 * more deply.
 * 
 * this approach assumes screens that are next to each other horizontally,
 * Other possibilities need to be investigated and accounted for.
 */
void DealWithNonSensicalGeometries(Display *mydpy, Window vroot, TwmWindow *tmp_win)
{
    Window		vvroot;
    int			x,y;
    unsigned int	w,h;
    unsigned int	j;
    virtualScreen	*myvs, *vs;
    int			dropx = 0;

    if(! vroot)
	return;

    if(!(XGetGeometry(mydpy, vroot, &vvroot, &x, &y, &w, &h, &j, &j)))
	return;

    myvs = findIfVScreenOf(x, y);

    /*
     * probably need to rethink this  for unmapped vs's.  ugh.
     */
    if(!myvs)
	return;

    for(vs = myvs->next; vs; vs = vs->next) {
	dropx += vs->w;
    }

    for(vs = Scr->vScreenList; vs && vs != myvs; vs = vs->next) {
	dropx -= vs->w;
    }

    if(tmp_win->frame_x > 0 && tmp_win->frame_x >= w) {
	tmp_win->frame_x -= abs(dropx);
    } else {
}

}
