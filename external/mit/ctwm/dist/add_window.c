/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
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

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#include "add_window.h"
#ifdef CAPTIVE
#include "captive.h"
#endif
#include "colormaps.h"
#include "ctwm_atoms.h"
#include "functions.h"
#include "events.h"
#ifdef EWMH
# include "ewmh_atoms.h"
#endif
#include "gram.tab.h"
#include "icons.h"
#include "iconmgr.h"
#include "image.h"
#include "list.h"
#include "mwmhints.h"
#include "occupation.h"
#include "otp.h"
#include "parse.h"
#include "r_area.h"
#include "r_layout.h"
#include "screen.h"
#ifdef SESSION
#include "session.h"
#endif
#include "util.h"
#include "vscreen.h"
#ifdef WINBOX
#include "windowbox.h"
#endif
#include "win_decorations.h"
#include "win_ops.h"
#include "win_regions.h"
#include "win_resize.h"
#include "win_ring.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "xparsegeometry.h"


int AddingX;
int AddingY;
unsigned int AddingW;
unsigned int AddingH;

static int PlaceX = -1;
static int PlaceY = -1;
#ifdef VSCREEN
static void DealWithNonSensicalGeometries(Display *dpy, Window vroot,
                TwmWindow *tmp_win);
#endif

char NoName[] = "Untitled"; /* name if no name is specified */
bool resizeWhenAdd;



/***********************************************************************
 *
 *  Procedure:
 *      AddWindow - add a new window to the twm list
 *
 *  Returned Value:
 *      (TwmWindow *) - pointer to the TwmWindow structure
 *
 *  Inputs:
 *      w       - the window id of the window to add
 *      wtype   - flag to tell if this is a normal window or some ctwm
 *                internal one.
 *
 *      iconp   - pointer to icon manager struct
 *
 ***********************************************************************
 */

TwmWindow *
AddWindow(Window w, AWType wtype, IconMgr *iconp, VirtualScreen *vs)
{
	TwmWindow *tmp_win;                 /* new twm window structure */
	bool ask_user;               /* don't know where to put the window */
	int gravx, gravy;                   /* gravity signs for positioning */
	int namelen;
	int bw2;
#ifdef SESSION
	short restore_icon_x, restore_icon_y;
	bool restore_iconified = false;
	bool restore_icon_info_present = false;
#endif
	bool restoredFromPrevSession = false;
	int saved_occupation = 0; /* <== [ Matthew McNeill Feb 1997 ] == */
	bool random_placed = false;
#ifdef WINBOX
	WindowBox *winbox;
#endif
	Window vroot;

#ifdef DEBUG
	fprintf(stderr, "AddWindow: w = 0x%x\n", w);
#endif

#ifdef CAPTIVE
	/*
	 * Possibly this window should be in a captive sub-ctwm?  If so, we
	 * shouldn't mess with it at all.
	 */
	if(!CLarg.is_captive && RedirectToCaptive(w)) {
		/* XXX x-ref comment by SetNoRedirect() */
		return (NULL);
	}
#endif


	/*
	 * Allocate and initialize our tracking struct
	 */
	tmp_win = calloc(1, sizeof(TwmWindow));
	if(tmp_win == NULL) {
		fprintf(stderr, "%s: Unable to allocate memory to manage window ID %lx.\n",
		        ProgramName, w);
		return NULL;
	}

	/*
	 * Some of these initializations are strictly unnecessary, since they
	 * evaluate out to 0, and calloc() gives us an already zero'd buffer.
	 * I'm leaving them anyway because a couple unnecessary stores are
	 * near enough to free considering everything we're doing, that the
	 * value as documentation stupendously outweighs the cost.
	 */
	tmp_win->w = w;
	tmp_win->zoomed = ZOOM_NONE;
	tmp_win->isiconmgr = (wtype == AWT_ICON_MANAGER);
	tmp_win->iconmgrp = iconp;
	tmp_win->iswspmgr = (wtype == AWT_WORKSPACE_MANAGER);
	tmp_win->isoccupy = (wtype == AWT_OCCUPY);
#ifdef WINBOX
	tmp_win->iswinbox = (wtype == AWT_WINDOWBOX);
#endif
	tmp_win->vs = vs;
	tmp_win->parent_vs = vs;
	tmp_win->savevs = NULL;
	tmp_win->cmaps.number_cwins = 0;
	tmp_win->savegeometry.width = -1;
	tmp_win->widthEverChangedByUser = false;
	tmp_win->heightEverChangedByUser = false;
	tmp_win->nameChanged = false;
	tmp_win->squeezed = false;
	tmp_win->iconified = false;
	tmp_win->isicon = false;
	tmp_win->icon_on = false;
	tmp_win->ring.cursor_valid = false;
	tmp_win->squeeze_info = NULL;
	tmp_win->squeeze_info_copied = false;



	/*
	 * Fetch a few bits of info about the window from the server, and
	 * tell the server to tell us about property changes; we'll need to
	 * know what happens.
	 *
	 * It's important that these remain relatively disconnected "early"
	 * bits; generally, they shouldn't rely on anything but the X Window
	 * in tmp_win->w to do their stuff.  e.g., anything that relies on
	 * other values in our ctwm TwmWindow tmp_win (window name, various
	 * flags, etc) has to come later.
	 */
	XSelectInput(dpy, tmp_win->w, PropertyChangeMask);
	XGetWindowAttributes(dpy, tmp_win->w, &tmp_win->attr);
	FetchWmProtocols(tmp_win);
	FetchWmColormapWindows(tmp_win);
#ifdef EWMH
	EwmhGetProperties(tmp_win);
#endif /* EWMH */


	/*
	 * Some other simple early initialization that has to follow those
	 * bits.
	 */
	tmp_win->old_bw = tmp_win->attr.border_width;


	/*
	 * Setup window name and class bits.  A lot of following code starts
	 * to care about this; in particular, anything looking in our
	 * name_lists generally goes by the name/class, so we need to get
	 * these set pretty early in the process.
	 */
	tmp_win->names.ctwm_wm_name = GetWMPropertyString(tmp_win->w,
	                              XA_CTWM_WM_NAME);
#ifdef EWMH
	tmp_win->names.net_wm_name = GetWMPropertyString(tmp_win->w,
	                             XA__NET_WM_NAME);
#endif
	tmp_win->names.wm_name = GetWMPropertyString(tmp_win->w, XA_WM_NAME);
	set_window_name(tmp_win);
	namelen = strlen(tmp_win->name);

	/* Setup class.  x-ref XXX in ctwm_main() about NoClass */
	tmp_win->class = NoClass;
	XGetClassHint(dpy, tmp_win->w, &tmp_win->class);
	if(tmp_win->class.res_name == NULL) {
		tmp_win->class.res_name = NoName;
	}
	if(tmp_win->class.res_class == NULL) {
		tmp_win->class.res_class = NoName;
	}

	/* Grab the icon name too */
	tmp_win->names.ctwm_wm_icon_name = GetWMPropertyString(tmp_win->w,
	                                   XA_CTWM_WM_ICON_NAME);
#ifdef EWMH
	tmp_win->names.net_wm_icon_name = GetWMPropertyString(tmp_win->w,
	                                  XA__NET_WM_ICON_NAME);
#endif
	tmp_win->names.wm_icon_name = GetWMPropertyString(tmp_win->w,
	                              XA_WM_ICON_NAME);
	set_window_icon_name(tmp_win);


	/* Convenience macro */
#define CHKL(lst) IsInList(Scr->lst, tmp_win)


	/* Is it a transient?  Or should we ignore that it is? */
	tmp_win->istransient = XGetTransientForHint(dpy, tmp_win->w,
	                       &tmp_win->transientfor);
	if(tmp_win->istransient) {
		/*
		 * XXX Should this be looking up transientfor instead of tmp_win?
		 * It seems like IgnoreTransient {} would list the windows that
		 * have transients we should ignore, while this condition makes
		 * it list the transient window names we should ignore.  Probably
		 * not trivial to fix if that's right, since it might b0rk
		 * existing configs...
		 */
		if(CHKL(IgnoreTransientL)) {
			tmp_win->istransient = false;
		}
	}


#ifdef SESSION
	/*
	 * Look up saved X Session info for the window if we have it.
	 */
	{
		short saved_x, saved_y;
		unsigned short saved_width, saved_height;
		bool width_ever_changed_by_user;
		bool height_ever_changed_by_user;

		if(GetWindowConfig(tmp_win,
		                   &saved_x, &saved_y, &saved_width, &saved_height,
		                   &restore_iconified, &restore_icon_info_present,
		                   &restore_icon_x, &restore_icon_y,
		                   &width_ever_changed_by_user,
		                   &height_ever_changed_by_user,
		                   &saved_occupation)) {
			/* Got saved info, use it */
			restoredFromPrevSession = true;

			tmp_win->attr.x = saved_x;
			tmp_win->attr.y = saved_y;

			tmp_win->widthEverChangedByUser = width_ever_changed_by_user;
			tmp_win->heightEverChangedByUser = height_ever_changed_by_user;

			if(width_ever_changed_by_user) {
				tmp_win->attr.width = saved_width;
			}

			if(height_ever_changed_by_user) {
				tmp_win->attr.height = saved_height;
			}
		}
	}
#endif


	/*
	 * Clip window to maximum size (either built-in ceiling, or
	 * config MaxWindowSize).
	 *
	 * Should look at window gravity?
	 */
	if(tmp_win->attr.width > Scr->MaxWindowWidth) {
		tmp_win->attr.width = Scr->MaxWindowWidth;
	}
	if(tmp_win->attr.height > Scr->MaxWindowHeight) {
		tmp_win->attr.height = Scr->MaxWindowHeight;
	}


	/*
	 * Setup WM_HINTS bits.  If we get nothing, we hardcode an
	 * assumption.
	 */
	tmp_win->wmhints = XGetWMHints(dpy, tmp_win->w);
	if(!tmp_win->wmhints) {
		tmp_win->wmhints = gen_synthetic_wmhints(tmp_win);
		if(!tmp_win->wmhints) {
			fprintf(stderr, "Failed allocating memory for hints!\n");
			free(tmp_win); // XXX leaky
			return NULL;
		}
	}

#ifdef SESSION
	/*
	 * Override a few bits with saved stuff from previous session, if we
	 * have it.
	 */
	if(restore_iconified) {
		tmp_win->wmhints->initial_state = IconicState;
		tmp_win->wmhints->flags |= StateHint;
	}

	if(restore_icon_info_present) {
		tmp_win->wmhints->icon_x = restore_icon_x;
		tmp_win->wmhints->icon_y = restore_icon_y;
		tmp_win->wmhints->flags |= IconPositionHint;
	}
#endif

	/* Munge as necessary for other stuff */
	munge_wmhints(tmp_win, tmp_win->wmhints);


	/*
	 * Various flags that may be screen-wide or window specific.
	 */
	tmp_win->highlight = Scr->Highlight && !CHKL(NoHighlight);
	tmp_win->stackmode = Scr->StackMode && !CHKL(NoStackModeL);
	tmp_win->titlehighlight = Scr->TitleHighlight && !CHKL(NoTitleHighlight);
	tmp_win->AlwaysSqueezeToGravity = Scr->AlwaysSqueezeToGravity
	                                  || CHKL(AlwaysSqueezeToGravityL);
	tmp_win->DontSetInactive = CHKL(DontSetInactive);
	tmp_win->AutoSqueeze = CHKL(AutoSqueeze);
	tmp_win->StartSqueezed =
#ifdef EWMH
	        (tmp_win->ewmhFlags & EWMH_STATE_SHADED) ||
#endif /* EWMH */
	        CHKL(StartSqueezed);

	tmp_win->auto_raise = Scr->AutoRaiseDefault || CHKL(AutoRaise);
	if(tmp_win->auto_raise) {
		Scr->NumAutoRaises++;
	}

	tmp_win->auto_lower = Scr->AutoLowerDefault || CHKL(AutoLower);
	if(tmp_win->auto_lower) {
		Scr->NumAutoLowers++;
	}

	tmp_win->OpaqueMove = Scr->DoOpaqueMove;
	if(CHKL(OpaqueMoveList)) {
		tmp_win->OpaqueMove = true;
	}
	else if(CHKL(NoOpaqueMoveList)) {
		tmp_win->OpaqueMove = false;
	}

	tmp_win->OpaqueResize = Scr->DoOpaqueResize;
	if(CHKL(OpaqueResizeList)) {
		tmp_win->OpaqueResize = true;
	}
	else if(CHKL(NoOpaqueResizeList)) {
		tmp_win->OpaqueResize = false;
	}


	/*
	 * If a window is listed in IconifyByUnmapping {}, we always iconify
	 * by unmapping.  Else, if it's DontIconifyByUnmapping {} or is an
	 * icon manager, we don't i_b_u.  Else, we go with the Scr-wide
	 * default.
	 */
	{
		bool ibum = CHKL(IconifyByUn);
		if(!ibum) {
			if(tmp_win->isiconmgr || CHKL(DontIconify)) {
				ibum = false; // redundant
			}
			else {
				ibum = Scr->IconifyByUnmapping;
			}
		}
		tmp_win->iconify_by_unmapping = ibum;
	}


	/*
	 * For transient windows or group members, we copy in UBMFA from its
	 * parent/leader/etc if we can find it.  Otherwise, it's just whether
	 * it's in the config list.
	 */
	tmp_win->UnmapByMovingFarAway = CHKL(UnmapByMovingFarAway);
	if(tmp_win->istransient || tmp_win->group) {
		TwmWindow *t = NULL;
		if(tmp_win->istransient) {
			t = GetTwmWindow(tmp_win->transientfor);
		}
		if(!t && tmp_win->group) {
			t = GetTwmWindow(tmp_win->group);
		}
		if(t) {
			tmp_win->UnmapByMovingFarAway = t->UnmapByMovingFarAway;
		}
	}


	/*
	 * Link it up into the window ring if we should.  If it's in
	 * WindowRing {}, we should.  Otherwise, we shouldn't unless
	 * WindowRingAll is set.  If it is, we still exclude several special
	 * ctwm windows, stuff in WindowRingExclude {}, and some special EWMH
	 * settings.
	 */
	if(CHKL(WindowRingL) ||
	                (Scr->WindowRingAll && !tmp_win->iswspmgr
	                 && !tmp_win->isiconmgr
#ifdef EWMH
	                 && EwmhOnWindowRing(tmp_win)
#endif /* EWMH */
	                 && !CHKL(WindowRingExcludeL))) {
		AddWindowToRing(tmp_win);
	}
	else {
		InitWindowNotOnRing(tmp_win);
	}


	/*
	 * Setup squeezing info.  We don't bother unless the server has Shape
	 * available, and the window isn't in our DontSqueezeTitle list.
	 * Else, we do/not based on the SqueezeTitle setting.  Note that
	 * "SqueezeTitle" being specified at all squeezes everything; its
	 * argument list lets you set specific squeeze params for specific
	 * windows, but other windows still get the default.
	 *
	 * Note that this does not have to be freed yet since it is coming
	 * from the screen list or from default_squeeze.  Places that change
	 * it [re]set squeeze_info_copied, and then the destroy handler looks
	 * at that to determine whether to free squeeze_info.
	 *
	 * XXX Technically, the HasShape test is redundant, since the config
	 * file parsing would never set Scr->SqueezeTitle unless HasShape
	 * were true anyway...
	 */
	if(HasShape && Scr->SqueezeTitle && !CHKL(DontSqueezeTitleL)) {
		tmp_win->squeeze_info = LookInListWin(Scr->SqueezeTitleL, tmp_win);
		if(!tmp_win->squeeze_info) {
			static SqueezeInfo default_squeeze = { SIJ_LEFT, 0, 0 };
			tmp_win->squeeze_info = &default_squeeze;
		}
	}


	/*
	 * Motif WM hints are used in setting up border and titlebar bits, so
	 * put them in a block here to scope the MWM var.
	 */
	{
		MotifWmHints mwmHints;
		bool have_title;

		GetMWMHints(tmp_win->w, &mwmHints);

		/*
		 * Figure border bits.  These are all exclusive cases, so it
		 * winds up being first-match.
		 *
		 * - EWMH, MWM hints, and NoBorder{} can tell us to use none.
		 * - ThreeDBorderWidth means use it and no regular 2d frame_bw.
		 * - ClientBorderWidth tells us to use the XWindowAttributes
		 *   border size rather than ours.
		 * - Else, our BorderWidth is the [2d] border size.
		 *
		 * X-ref comments in win_decorations.c:SetBorderCursor() about
		 * the somewhat differing treatment of 3d vs non-3d border widths
		 * and their effects on the window coordinates.
		 */
		tmp_win->frame_bw3D = Scr->ThreeDBorderWidth;
		if(
#ifdef EWMH
		        !EwmhHasBorder(tmp_win) ||
#endif /* EWMH */
		        (mwm_has_border(&mwmHints) == 0) ||
		        CHKL(NoBorder)) {
			tmp_win->frame_bw = 0;
			tmp_win->frame_bw3D = 0;
		}
		else if(tmp_win->frame_bw3D != 0) {
			tmp_win->frame_bw = 0;
		}
		else if(Scr->ClientBorderWidth) {
			tmp_win->frame_bw = tmp_win->old_bw;
		}
		else {
			tmp_win->frame_bw = Scr->BorderWidth;
		}
		bw2 = tmp_win->frame_bw * 2;  // Used repeatedly later


		/*
		 * Now, what about the titlebar?
		 *
		 * - Default to showing,
		 * - Then EWMH gets to say no in some special cases,
		 * - Then MWM can say yes/no (or refuse to say anything),
		 * - NoTitle (general setting) gets to override all of that,
		 * - Specific MakeTitle beats general NoTitle,
		 * - And specific NoTitle overrides MakeTitle.
		 */
		have_title = true;
		ALLOW_DEAD_STORE(have_title);
#ifdef EWMH
		have_title = EwmhHasTitle(tmp_win);
#endif /* EWMH */
		if(mwm_sets_title(&mwmHints)) {
			have_title = mwm_has_title(&mwmHints);
		}
		if(Scr->NoTitlebar) {
			have_title = false;
		}
		if(CHKL(MakeTitle)) {
			have_title = true;
		}
		if(CHKL(NoTitle)) {
			have_title = false;
		}

		/*
		 * Now mark up how big to make it.  title_height sets how tall
		 * the titlebar is, with magic treating 0 as "don't make a
		 * titlebar".  We only care about adding frame_bw and never
		 * frame_bw3D, since the 3d case interprets all the inner
		 * coordinates differently (x-ref above x-ref).
		 *
		 * Transients may not be decorated regardless of the above
		 * figuring, so handle that here too.
		 */
		if(tmp_win->istransient && !Scr->DecorateTransients) {
			tmp_win->title_height = 0;
		}
		else if(have_title) {
			tmp_win->title_height = Scr->TitleHeight + tmp_win->frame_bw;
		}
		else {
			tmp_win->title_height = 0;
		}
	}


#ifdef EWMH
	/*
	 * Now that we know the title_height and the frame border width, we
	 * can set an EWMH property to tell the client how much we're adding
	 * around them.
	 */
	EwmhSet_NET_FRAME_EXTENTS(tmp_win);
#endif


	/*
	 * Need the GetWindowAttributes() call and setting ->old_bw and
	 * ->frame_bw3D for some of the math in looking up the
	 *  WM_NORMAL_HINTS bits, so now we can do that.
	 */
	GetWindowSizeHints(tmp_win);


	/* Maybe we're ordering it to start off iconified? */
	if(CHKL(StartIconified)) {
		tmp_win->wmhints->initial_state = IconicState;
		tmp_win->wmhints->flags |= StateHint;
	}


	/*
	 * Figure gravity bits.  When restoring from a previous session, we
	 * always use NorthWest gravity.
	 */
	if(restoredFromPrevSession) {
		gravx = gravy = -1;
	}
	else {
		GetGravityOffsets(tmp_win, &gravx, &gravy);
	}


	/* So far that's the end of where we're using this */
#undef CHKL


	/*
	 * Now we start getting more into the active bits of things.  Start
	 * figuring out how we'll decide where to position it.  ask_user is
	 * slightly misnamed, as it doesn't actually mean ask the user, but
	 * rather whether the user/WM gets to choose or whether the
	 * application does.  That is, we only case about whether or not
	 * RandomPlacement if(ask_user==true) anyway.  ask_user=false means
	 * we just go with what's in the window's XWindowAttributes bits.
	 *
	 * We don't even consider overriding the window if:
	 *
	 * - It's a transient, or
	 * - the WM_NORMAL_HINTS property gave us a user-specified position
	 *   (USPosition), or
	 * - the hints gave us a a program-specific position (PPosition), and
	 *   the UsePPosition config param specifies we should use it.
	 *
	 * x-ref ICCCM discussion of WM_NORMAL_HINTS for some details on the
	 * flags
	 * (https://www.x.org/releases/X11R7.7/doc/xorg-docs/icccm/icccm.html#Client_Properties)
	 */
	ask_user = true;
	if(tmp_win->istransient) {
		ask_user = false;
	}
	else if(tmp_win->hints.flags & USPosition) {
		ask_user = false;
	}
	else if(tmp_win->hints.flags & PPosition) {
		if(Scr->UsePPosition == PPOS_ON) {
			ask_user = false;
		}
		else if(Scr->UsePPosition == PPOS_NON_ZERO
		                && (tmp_win->attr.x != 0 || tmp_win->attr.y != 0)) {
			ask_user = false;
		}
	}


	/*
	 * Set the window occupation.  If we pulled previous Session info,
	 * saved_occupation may have data from it that will be used;
	 * otherwise it's already zeroed and has no effect.  X-ref XXX
	 * comment on SetupOccupation() for notes on order of application of
	 * various sources for occupation.
	 *
	 * Note that SetupOccupation() may update tmp_win->{parent_,}vs if
	 * needed to make the window visible in another vscreen.  It may also
	 * set tmp_win->vs to NULL if it has no occupation in the current
	 * workspace.
	 */
	SetupOccupation(tmp_win, saved_occupation);


#ifdef WINBOX
	/* Does it go in a window box? */
	winbox = findWindowBox(tmp_win);
#endif


	/*
	 * Set some values for the frame size.
	 *
	 * These get redone down below when we create the frame, when they're
	 * actually useful.  So why bother here?  There is some code down in
	 * the block where we prompt for a window position that calls some
	 * functions that need plausible values in them.  However, those code
	 * blocks calculate and set values themselves, so there shouldn't be
	 * any actual need for them here.  Left #if'd out for the present in
	 * case something turns up; this should be GC'd at some point if
	 * nothing does.
	 */
#if 0
	tmp_win->frame_width  = tmp_win->attr.width  + 2 * tmp_win->frame_bw3D;
	tmp_win->frame_height = tmp_win->attr.height + 2 * tmp_win->frame_bw3D +
	                        tmp_win->title_height;
	ConstrainSize(tmp_win, &tmp_win->frame_width, &tmp_win->frame_height);
#endif


	/*
	 * See if there's a WindowRegion we should honor.  If so, it'll set
	 * the X/Y coords, and we'll want to accept them instead of doing our
	 * own (or the user's) positioning.
	 *
	 * This needs the frame_{width,height}.
	 */
	if(PlaceWindowInRegion(tmp_win, &(tmp_win->attr.x), &(tmp_win->attr.y))) {
		ask_user = false;
	}


	/*
	 * Maybe we have WindowGeometries {} set for it?  If so, we'll take
	 * that as our specifics too.
	 */
	{
		char *geom = LookInListWin(Scr->WindowGeometries, tmp_win);
		if(geom) {
			int mask = RLayoutXParseGeometry(Scr->Layout, geom,
			                                 &tmp_win->attr.x, &tmp_win->attr.y,
			                                 (unsigned int *) &tmp_win->attr.width,
			                                 (unsigned int *) &tmp_win->attr.height);

			if(mask & XNegative) {
				tmp_win->attr.x += Scr->rootw - tmp_win->attr.width;
			}
			if(mask & YNegative) {
				tmp_win->attr.y += Scr->rooth - tmp_win->attr.height;
			}
			ask_user = false;
		}
	}


	/* Figure up what root window we should be working in */
	if(tmp_win->parent_vs) {
		vroot = tmp_win->parent_vs->window;
	}
	else {
		vroot = Scr->Root;      /* never */
		tmp_win->parent_vs = Scr->currentvs;
	}
#ifdef WINBOX
	if(winbox) {
		vroot = winbox->window;
	}
#endif


	/*
	 * Handle positioning of the window.  If we're very early in startup
	 * (setting up ctwm's own windows, taking over windows already on the
	 * screen), or restoring defined session stuff, or otherwise
	 * ask_user=false'd above, we just take the already set position
	 * info.  Otherwise, we handle it via RandomPlacement or user outline
	 * setting.
	 *
	 * XXX Somebody should go through these blocks in more detail,
	 * they're sure to need further cleaning and commenting.  IWBNI they
	 * could be encapsulated well enough to move out into separate
	 * functions, for extra readability...
	 */
	if(HandlingEvents && ask_user && !restoredFromPrevSession) {
		if((Scr->RandomPlacement == RP_ALL) ||
		                ((Scr->RandomPlacement == RP_UNMAPPED) &&
		                 ((tmp_win->wmhints->initial_state == IconicState) ||
		                  (! visible(tmp_win))))) {
			/* just stick it somewhere */

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
			if(PlaceX < 0 && PlaceY < 0) {
				if(Scr->RandomDisplacementX >= 0) {
					PlaceX = Scr->BorderLeft + 5;
				}
				else {
					PlaceX = Scr->rootw - tmp_win->attr.width - Scr->BorderRight - 5;
				}
				if(Scr->RandomDisplacementY >= 0) {
					PlaceY = Scr->BorderTop + 5;
				}
				else
					PlaceY = Scr->rooth - tmp_win->attr.height - tmp_win->title_height
					         - Scr->BorderBottom - 5;
			}

			/* For a positive horizontal displacement, if the right edge
			   of the window would fall outside of the screen, start over
			   by placing the left edge of the window 5 pixels inside
			   the left edge of the screen.*/
			if(Scr->RandomDisplacementX >= 0
			                && (PlaceX + tmp_win->attr.width
			                    > Scr->rootw - Scr->BorderRight - 5)) {
				PlaceX = Scr->BorderLeft + 5;
			}

			/* For a negative horizontal displacement, if the left edge
			   of the window would fall outside of the screen, start over
			   by placing the right edge of the window 5 pixels inside
			   the right edge of the screen.*/
			if(Scr->RandomDisplacementX < 0 && PlaceX < Scr->BorderLeft + 5) {
				PlaceX = Scr->rootw - tmp_win->attr.width - Scr->BorderRight - 5;
			}

			/* For a positive vertical displacement, if the bottom edge
			   of the window would fall outside of the screen, start over
			   by placing the top edge of the window 5 pixels inside the
			   top edge of the screen.  Because we add the title height
			   further down, we need to count with it here as well.  */
			if(Scr->RandomDisplacementY >= 0
			                && (PlaceY + tmp_win->attr.height + tmp_win->title_height
			                    > Scr->rooth - Scr->BorderBottom - 5)) {
				PlaceY = Scr->BorderTop + 5;
			}

			/* For a negative vertical displacement, if the top edge of
			   the window would fall outside of the screen, start over by
			   placing the bottom edge of the window 5 pixels inside the
			   bottom edge of the screen.  Because we add the title height
			   further down, we need to count with it here as well.  */
			if(Scr->RandomDisplacementY < 0 && PlaceY < Scr->BorderTop + 5)
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
			if(Scr->DontMoveOff) {
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
				if((tmp_win->attr.x + tmp_win->attr.width)  > Scr->rootw) {
					available = Scr->rootw - tmp_win->attr.width
					            - 2 * tmp_win->frame_bw3D - bw2;

#ifdef DEBUG
					fprintf(stderr, "DEBUG[DontMoveOff]: availableX: %d\n",
					        available);
#endif

					/* If the window is wider than the screen or exactly the width
					 of the screen, the availability is exactly 0.  The result
					 will be to have the window placed as much to the left as
					 possible. */
					if(available <= 0) {
						available = 0;
					}

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
				if((tmp_win->attr.y + tmp_win->attr.height)  > Scr->rooth) {
					available = Scr->rooth - tmp_win->attr.height
					            - tmp_win->title_height
					            - 2 * tmp_win->frame_bw3D - bw2;

#ifdef DEBUG
					fprintf(stderr, "DEBUG[DontMoveOff]: availableY: %d\n",
					        available);
#endif

					/* If the window is taller than the screen or exactly the
					 height of the screen, the availability is exactly 0.
					 The result will be to have the window placed as much to
					 the top as possible. */
					if(available <= 0) {
						available = 0;
					}

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
			if(PlaceX - tmp_win->attr.x < 15
			                || PlaceY - (tmp_win->attr.y - tmp_win->title_height) < 15) {
				PlaceX += Scr->RandomDisplacementX;
				PlaceY += Scr->RandomDisplacementY;
			}

			random_placed = true;
		}
		else if(!(tmp_win->wmhints->flags & StateHint &&
		                tmp_win->wmhints->initial_state == IconicState)) {
			/* else prompt */
			bool firsttime = true;
			int found = 0;
			int width, height;
			XEvent event;

			/* better wait until all the mouse buttons have been
			 * released.
			 */
			while(1) {
				unsigned int qpmask;
				Window qproot;
				int stat;

				XUngrabServer(dpy);
				XSync(dpy, 0);
				XGrabServer(dpy);

				qpmask = 0;
				if(!XQueryPointer(dpy, Scr->Root, &qproot,
				                  &JunkChild, &JunkX, &JunkY,
				                  &AddingX, &AddingY, &qpmask)) {
					qpmask = 0;
				}

				/* Clear out any but the Button bits */
				qpmask &= (Button1Mask | Button2Mask | Button3Mask |
				           Button4Mask | Button5Mask);

				/*
				 * watch out for changing screens
				 */
				if(firsttime) {
					if(qproot != Scr->Root) {
						int scrnum;
						for(scrnum = 0; scrnum < NumScreens; scrnum++) {
							if(qproot == RootWindow(dpy, scrnum)) {
								break;
							}
						}
						if(scrnum != NumScreens) {
							PreviousScreen = scrnum;
						}
					}
					if(Scr->currentvs) {
						vroot = Scr->currentvs->window;
					}
					firsttime = false;
				}
#ifdef WINBOX
				if(winbox) {
					vroot = winbox->window;
				}
#endif

				/*
				 * wait for buttons to come up; yuck
				 */
				if(qpmask != 0) {
					continue;
				}

				/*
				 * this will cause a warp to the indicated root
				 */
				stat = XGrabPointer(dpy, vroot, False,
				                    ButtonPressMask | ButtonReleaseMask |
				                    PointerMotionMask | PointerMotionHintMask,
				                    GrabModeAsync, GrabModeAsync,
				                    vroot, UpperLeftCursor, CurrentTime);
				if(stat == GrabSuccess) {
					break;
				}
			}

			{
				XRectangle ink_rect;
				XRectangle logical_rect;

				XmbTextExtents(Scr->SizeFont.font_set,
				               tmp_win->name, namelen,
				               &ink_rect, &logical_rect);
				width = SIZE_HINDENT + ink_rect.width;
				height = Scr->SizeFont.height + SIZE_VINDENT * 2;

				XmbTextExtents(Scr->SizeFont.font_set,
				               ": ", 2,  NULL, &logical_rect);
				Scr->SizeStringOffset = width + logical_rect.width;
			}

			MoveResizeSizeWindow(AddingX, AddingY,
			                     Scr->SizeStringOffset + Scr->SizeStringWidth + SIZE_HINDENT,
			                     height);
			XMapRaised(dpy, Scr->SizeWindow);
			InstallRootColormap();
			FB(Scr->DefaultC.fore, Scr->DefaultC.back);
			XmbDrawImageString(dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
			                   Scr->NormalGC, SIZE_HINDENT,
			                   SIZE_VINDENT + Scr->SizeFont.ascent,
			                   tmp_win->name, namelen);

#ifdef WINBOX
			if(winbox) {
				ConstrainedToWinBox(tmp_win, AddingX, AddingY, &AddingX, &AddingY);
			}
#endif

			AddingW = tmp_win->attr.width + bw2 + 2 * tmp_win->frame_bw3D;
			AddingH = tmp_win->attr.height + tmp_win->title_height +
			          bw2 + 2 * tmp_win->frame_bw3D;
			MoveOutline(vroot, AddingX, AddingY, AddingW, AddingH,
			            tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);

			XmbDrawImageString(dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
			                   Scr->NormalGC, width,
			                   SIZE_VINDENT + Scr->SizeFont.ascent, ": ", 2);
			DisplayPosition(tmp_win, AddingX, AddingY);

			/*
			 * The TryTo*() and DoResize() calls below rely on having
			 * frame_{width,height} set, so set them.
			 */
			tmp_win->frame_width  = AddingW - bw2;
			tmp_win->frame_height = AddingH - bw2;
			/*SetFocus (NULL, CurrentTime);*/
			while(1) {
				if(Scr->OpenWindowTimeout) {
					const int fd = ConnectionNumber(dpy);
					while(!XCheckMaskEvent(dpy, ButtonMotionMask | ButtonPressMask, &event)) {
						fd_set mask;
						struct timeval timeout = {
							.tv_sec  = Scr->OpenWindowTimeout,
							.tv_usec = 0,
						};

						FD_ZERO(&mask);
						FD_SET(fd, &mask);
						found = select(fd + 1, &mask, NULL, NULL, &timeout);
						if(found == 0) {
							break;
						}
					}
					if(found == 0) {
						break;
					}
				}
				else {
					found = 1;
					XMaskEvent(dpy, ButtonPressMask | PointerMotionMask, &event);
				}
				if(event.type == MotionNotify) {
					/* discard any extra motion events before a release */
					while(XCheckMaskEvent(dpy,
					                      ButtonMotionMask | ButtonPressMask, &event))
						if(event.type == ButtonPress) {
							break;
						}
				}
				FixRootEvent(&event);
				if(event.type == ButtonPress) {
					AddingX = event.xbutton.x_root;
					AddingY = event.xbutton.y_root;

					/* TryTo*() need tmp_win->frame_{width,height} */
					TryToGrid(tmp_win, &AddingX, &AddingY);
					if(Scr->PackNewWindows) {
						TryToPack(tmp_win, &AddingX, &AddingY);
					}

					/* DontMoveOff prohibits user from off-screen placement */
					if(Scr->DontMoveOff) {
						ConstrainByBorders(tmp_win, &AddingX, AddingW, &AddingY, AddingH);
					}
					break;
				}

				if(event.type != MotionNotify) {
					continue;
				}

				XQueryPointer(dpy, vroot, &JunkRoot, &JunkChild,
				              &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

				TryToGrid(tmp_win, &AddingX, &AddingY);
				if(Scr->PackNewWindows) {
					TryToPack(tmp_win, &AddingX, &AddingY);
				}
				if(Scr->DontMoveOff) {
					ConstrainByBorders(tmp_win, &AddingX, AddingW, &AddingY, AddingH);
				}
				MoveOutline(vroot, AddingX, AddingY, AddingW, AddingH,
				            tmp_win->frame_bw, tmp_win->title_height + tmp_win->frame_bw3D);

				DisplayPosition(tmp_win, AddingX, AddingY);
			}

			if(found) {
				if(event.xbutton.button == Button2) {
					int lastx, lasty;
					XRectangle logical_rect;

					XmbTextExtents(Scr->SizeFont.font_set,
					               ": ", 2,  NULL, &logical_rect);
					Scr->SizeStringOffset = width + logical_rect.width;

					MoveResizeSizeWindow(event.xbutton.x_root, event.xbutton.y_root,
					                     Scr->SizeStringOffset + Scr->SizeStringWidth + SIZE_HINDENT,
					                     height);

					XmbDrawImageString(dpy, Scr->SizeWindow, Scr->SizeFont.font_set,
					                   Scr->NormalGC, width,
					                   SIZE_VINDENT + Scr->SizeFont.ascent, ": ", 2);

					if(0/*Scr->AutoRelativeResize*/) {
						int dx = (tmp_win->attr.width / 4);
						int dy = (tmp_win->attr.height / 4);

#define HALF_AVE_CURSOR_SIZE 8          /* so that it is visible */
						if(dx < HALF_AVE_CURSOR_SIZE + Scr->BorderLeft) {
							dx = HALF_AVE_CURSOR_SIZE + Scr->BorderLeft;
						}
						if(dy < HALF_AVE_CURSOR_SIZE + Scr->BorderTop) {
							dy = HALF_AVE_CURSOR_SIZE + Scr->BorderTop;
						}
#undef HALF_AVE_CURSOR_SIZE
						dx += (tmp_win->frame_bw + 1);
						dy += (bw2 + tmp_win->title_height + 1);
						if(AddingX + dx >= Scr->rootw - Scr->BorderRight) {
							dx = Scr->rootw - Scr->BorderRight - AddingX - 1;
						}
						if(AddingY + dy >= Scr->rooth - Scr->BorderBottom) {
							dy = Scr->rooth - Scr->BorderBottom - AddingY - 1;
						}
						if(dx > 0 && dy > 0) {
							XWarpPointer(dpy, None, None, 0, 0, 0, 0, dx, dy);
						}
					}
					else {
						XWarpPointer(dpy, None, vroot, 0, 0, 0, 0,
						             AddingX + AddingW / 2, AddingY + AddingH / 2);
					}
					AddStartResize(tmp_win, AddingX, AddingY, AddingW, AddingH);

					lastx = -10000;
					lasty = -10000;
					while(1) {
						XMaskEvent(dpy,
						           ButtonReleaseMask | ButtonMotionMask, &event);

						if(event.type == MotionNotify) {
							/* discard any extra motion events before a release */
							while(XCheckMaskEvent(dpy,
							                      ButtonMotionMask | ButtonReleaseMask, &event))
								if(event.type == ButtonRelease) {
									break;
								}
						}
						FixRootEvent(&event);

						if(event.type == ButtonRelease) {
							AddEndResize(tmp_win);
							break;
						}

						if(event.type != MotionNotify) {
							continue;
						}

						/*
						 * XXX - if we are going to do a loop, we ought to consider
						 * using multiple GXxor lines so that we don't need to
						 * grab the server.
						 */
						XQueryPointer(dpy, vroot, &JunkRoot, &JunkChild,
						              &JunkX, &JunkY, &AddingX, &AddingY,
						              &JunkMask);

						if(lastx != AddingX || lasty != AddingY) {
							resizeWhenAdd = true;
							/*
							 * DR() calls SetupWindow(), which uses
							 * frame_{width,height}.
							 */
							DoResize(AddingX, AddingY, tmp_win);
							resizeWhenAdd = false;

							lastx = AddingX;
							lasty = AddingY;
						}

					}
				}
				else if(event.xbutton.button == Button3) {
					RArea area;
					int max_bottom, max_right;

					area = RAreaNew(AddingX, AddingY, AddingW, AddingH);

					max_bottom = RLayoutFindMonitorBottomEdge(Scr->BorderedLayout, &area) - bw2;
					max_right = RLayoutFindMonitorRightEdge(Scr->BorderedLayout, &area) - bw2;

					/*
					 * Make window go to bottom of screen, and clip to right edge.
					 * This is useful when popping up large windows and fixed
					 * column text windows.
					 */
					if(AddingX + AddingW - 1 > max_right) {
						AddingW = max_right - AddingX + 1;
					}
					AddingH = max_bottom - AddingY + 1;

					ConstrainSize(tmp_win, &AddingW, &AddingH);   /* w/o borders */
					AddingW += bw2;
					AddingH += bw2;
					XMaskEvent(dpy, ButtonReleaseMask, &event);
				}
				else {
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
	else {
		/*
		 * Put it where asked, mod title bar.  If the gravity is towards
		 * the top, move it by the title height.
		 */
		if(gravy < 0) {
			tmp_win->attr.y -= gravy * tmp_win->title_height;
		}
	}


#ifdef DEBUG
	fprintf(stderr, "  position window  %d, %d  %dx%d\n",
	        tmp_win->attr.x,
	        tmp_win->attr.y,
	        tmp_win->attr.width,
	        tmp_win->attr.height);
#endif


	/*
	 * Possibly need to tweak what it thinks of as its position to
	 * account for borders.  XXX Verify conditionalization and math.
	 */
	if(!Scr->ClientBorderWidth) {
		int delta = tmp_win->attr.border_width - tmp_win->frame_bw -
		            tmp_win->frame_bw3D;
		tmp_win->attr.x += gravx * delta;
		tmp_win->attr.y += gravy * delta;
	}


	/*
	 * Init the title width to the window's width.  This will be right as
	 * long as you're not SqueezeTitle'ing; if you are, we rejigger it in
	 * SetupFrame().
	 */
	tmp_win->title_width = tmp_win->attr.width;


	/*
	 * Figure initial screen size of writing out the window name.  This
	 * is needed when laying out titlebar bits (down in the call chain
	 * inside SetupFrame()).  The event handler updates this when it
	 * changes.
	 */
	{
		XRectangle logical_rect;
		XmbTextExtents(Scr->TitleBarFont.font_set, tmp_win->name, namelen,
		               NULL, &logical_rect);
		tmp_win->name_width = logical_rect.width;
	}


	/* Remove original border if there is one; we make our own now */
	if(tmp_win->old_bw) {
		XSetWindowBorderWidth(dpy, tmp_win->w, 0);
	}


	/*
	 * Setup various color bits
	 */
#define SETC(lst, save) GetColorFromList(Scr->lst, tmp_win->name, \
                &tmp_win->class, &tmp_win->save)

	/* No distinction fore/back for borders in the lists */
	tmp_win->borderC.fore = Scr->BorderColorC.fore;
	tmp_win->borderC.back = Scr->BorderColorC.back;
	SETC(BorderColorL, borderC.fore);
	SETC(BorderColorL, borderC.back);

	tmp_win->border_tile.fore = Scr->BorderTileC.fore;
	tmp_win->border_tile.back = Scr->BorderTileC.back;
	SETC(BorderTileForegroundL, border_tile.fore);
	SETC(BorderTileBackgroundL, border_tile.back);

	tmp_win->title.fore = Scr->TitleC.fore;
	tmp_win->title.back = Scr->TitleC.back;
	SETC(TitleForegroundL, title.fore);
	SETC(TitleBackgroundL, title.back);

#undef SETC

	/* Shading on 3d bits */
	if(Scr->use3Dtitles  && !Scr->BeNiceToColormap) {
		GetShadeColors(&tmp_win->title);
	}
	if(Scr->use3Dborders && !Scr->BeNiceToColormap) {
		GetShadeColors(&tmp_win->borderC);
		GetShadeColors(&tmp_win->border_tile);
	}


	/*
	 * Following bits are more active, and we want to make sure nothing
	 * else gets to do anything with the server while we're doing it.
	 *
	 * Minor investigations seems to suggest we could pull a number of
	 * these things out (mostly to later, but probably some to earlier)
	 * so we keep the server grabbed for a shorter period of time.  I'm
	 * not putting significant effort into finding out what we could pull
	 * out because it's already plenty fast, but there is probably fruit
	 * that could be plucked if somebody finds it not so.
	 */
	XGrabServer(dpy);


	/*
	 * Make sure the client window still exists.  We don't want to leave an
	 * orphan frame window if it doesn't.  Since we now have the server
	 * grabbed, the window can't disappear later without having been
	 * reparented, so we'll get a DestroyNotify for it.  We won't have
	 * gotten one for anything up to here, however.
	 */
	if(XGetGeometry(dpy, tmp_win->w, &JunkRoot, &JunkX, &JunkY,
	                &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth) == 0) {
		UnlinkWindowFromRing(tmp_win);

		/* XXX Leaky as all hell */
		free(tmp_win);
		XUngrabServer(dpy);
		return(NULL);
	}


	/* Link the window into our list of all the TwmWindow's */
	tmp_win->next = Scr->FirstWindow;
	if(Scr->FirstWindow != NULL) {
		Scr->FirstWindow->prev = tmp_win;
	}
	tmp_win->prev = NULL;
	Scr->FirstWindow = tmp_win;



	/*
	 * Start creating the other X windows we wrap around it for
	 * decorations.  X-ref discussion in win_decorations.c for the
	 * details of what they all are and why they're there.
	 *
	 * XXX Good candidate for moving out into a helper function...
	 */


	/*
	 * First, the frame
	 */
	{
		unsigned long valuemask;
		XSetWindowAttributes attributes;

		/*
		 * Figure size/position.
		 */
		tmp_win->frame_x = tmp_win->attr.x + tmp_win->old_bw
		                   - tmp_win->frame_bw - tmp_win->frame_bw3D;
		tmp_win->frame_y = tmp_win->attr.y - tmp_win->title_height
		                   + tmp_win->old_bw
		                   - tmp_win->frame_bw - tmp_win->frame_bw3D;
		tmp_win->frame_width  = tmp_win->attr.width  + 2 * tmp_win->frame_bw3D;
		tmp_win->frame_height = tmp_win->attr.height + 2 * tmp_win->frame_bw3D
		                        + tmp_win->title_height;

		/* Adjust based on hints */
		ConstrainSize(tmp_win, &tmp_win->frame_width, &tmp_win->frame_height);

		/*
		 * Adjust as necessary to keep things on-screen.  If we [ctwm]
		 * chose the position, CBB() involves checking things like
		 * MoveOffResistance etc to keep it on.
		 */
		if(random_placed) {
			ConstrainByBorders(tmp_win, &tmp_win->frame_x, tmp_win->frame_width,
			                   &tmp_win->frame_y, tmp_win->frame_height);
		}

		/* No matter what, make sure SOME part of the window is on-screen */
		{
			RArea area;
			int min_x, min_y, max_bottom, max_right;
			const RLayout *layout = Scr->BorderedLayout;

			area = RAreaNew(tmp_win->frame_x, tmp_win->frame_y,
			                (int)tmp_win->frame_width,
			                (int)tmp_win->frame_height);

#ifdef EWMH
			// Hack: windows with EWMH struts defined are trying to
			// reserve a bit of the screen for themselves.  We currently
			// do that by hacking strut'ed space into the BorderedLayout,
			// which is a bogus way of doing things.  But it also means
			// that here we're forcing the windows to be outside their
			// own struts, which is nonsensical.
			//
			// Hack around that by making strut'd windows just use
			// Layout, rather than BorderedLayout.  This is Wrong(tm)
			// because the whole point of BorderedLayout is space
			// reservation by the user, which we'd now be ignoring.  Also
			// just because a window has its own struts doesn't mean it
			// should get to ignore everyone else's struts too. However,
			// this is at least consistent with pre-4.1.0 behavior, so
			// it's not a _new_ bug.  And forcing windows outside their
			// own reservation is way stupider...
			if(tmp_win->ewmhFlags & EWMH_HAS_STRUT) {
				layout = Scr->Layout;
			}
#endif

			RLayoutFindTopBottomEdges(layout, &area,
			                          &min_y, &max_bottom);
			RLayoutFindLeftRightEdges(layout, &area,
			                          &min_x, &max_right);

			// These conditions would only be true if the window was
			// completely off-screen; in that case, the RLayout* calls
			// above would have found the closest edges to move it to.
			// We wind up sticking it in the top-left of the
			// bottom-right-most monitor it would touch.
			if(area.x > max_right || area.y > max_bottom ||
			                area.x + area.width <= min_x ||
			                area.y + area.height <= min_y) {
				tmp_win->frame_x = min_x;
				tmp_win->frame_y = min_y;
			}
		}

#ifdef VSCREEN
		/* May need adjusting for vscreens too */
		DealWithNonSensicalGeometries(dpy, vroot, tmp_win);
#endif


		/*
		 * Setup the X attributes for the frame.
		 */
		valuemask = CWBackPixmap | CWBorderPixel | CWBackPixel
		            | CWCursor | CWEventMask;
		attributes.background_pixmap = None;
		attributes.border_pixel = tmp_win->border_tile.back;
		attributes.background_pixel = tmp_win->border_tile.back;
		attributes.cursor = Scr->FrameCursor;
		attributes.event_mask = (SubstructureRedirectMask
		                         | ButtonPressMask | ButtonReleaseMask
		                         | EnterWindowMask | LeaveWindowMask
		                         | ExposureMask);

		/*
		 * If we have BorderResizeCursors, we need to know about motions
		 * in the window to know when to change (e.g., for corners).
		 */
		if(Scr->BorderCursors) {
			attributes.event_mask |= PointerMotionMask;
		}

		/*
		 * If the real window specified save_under or a specific gravity,
		 * set them on the frame too.
		 */
		if(tmp_win->attr.save_under) {
			attributes.save_under = True;
			valuemask |= CWSaveUnder;
		}
		if(tmp_win->hints.flags & PWinGravity) {
			attributes.win_gravity = tmp_win->hints.win_gravity;
			valuemask |= CWWinGravity;
		}


		/* And create */
		tmp_win->frame = XCreateWindow(dpy, vroot,
		                               tmp_win->frame_x, tmp_win->frame_y,
		                               tmp_win->frame_width,
		                               tmp_win->frame_height,
		                               tmp_win->frame_bw,
		                               Scr->d_depth, CopyFromParent,
		                               Scr->d_visual, valuemask, &attributes);
		if(Scr->NameDecorations) {
			XStoreName(dpy, tmp_win->frame, "CTWM frame");
		}
	}


	/*
	 * Next, the titlebar, if we have one
	 */
	if(tmp_win->title_height) {
		unsigned long valuemask;
		XSetWindowAttributes attributes;
		int x, y;

		/*
		 * We need to know about keys/buttons and exposure of the
		 * titlebar, for bindings and repaining.  And leave to X server
		 * bits about border/background.
		 */
		valuemask = (CWEventMask | CWDontPropagate
		             | CWBorderPixel | CWBackPixel);
		attributes.event_mask = (KeyPressMask | ButtonPressMask
		                         | ButtonReleaseMask | ExposureMask);
		attributes.do_not_propagate_mask = PointerMotionMask;
		attributes.border_pixel = tmp_win->borderC.back;
		attributes.background_pixel = tmp_win->title.back;


		/* Create */
		x = y = tmp_win->frame_bw3D - tmp_win->frame_bw;
		tmp_win->title_w = XCreateWindow(dpy, tmp_win->frame, x, y,
		                                 tmp_win->attr.width,
		                                 Scr->TitleHeight, tmp_win->frame_bw,
		                                 Scr->d_depth, CopyFromParent,
		                                 Scr->d_visual, valuemask, &attributes);
		if(Scr->NameDecorations) {
			XStoreName(dpy, tmp_win->title_w, "CTWM titlebar");
		}
	}
	else {
		tmp_win->title_w = None;
		tmp_win->squeeze_info = NULL;
	}


	/*
	 * If we're highlighting borders on focus, we need the pixmap to do
	 * it.
	 *
	 * XXX I'm not at all sure this can't just be global and shared, so
	 * we don't have to create one per window...
	 */
	if(tmp_win->highlight) {
		char *which;

		if(Scr->use3Dtitles && (Scr->Monochrome != COLOR)) {
			which = "black";
		}
		else {
			which = "gray";
		}
		tmp_win->gray = mk_blackgray_pixmap(which, vroot,
		                                    tmp_win->border_tile.fore,
		                                    tmp_win->border_tile.back);

		tmp_win->hasfocusvisible = true;
		SetFocusVisualAttributes(tmp_win, false);
	}
	else {
		tmp_win->gray = None;
	}


	/*
	 * Setup OTP bits for stacking
	 */
	OtpAdd(tmp_win, WinWin);


	/*
	 * Setup the stuff inside the titlebar window, if we have it.  If we
	 * don't, fake up the coordinates where the titlebar would be for
	 * <reasons>.
	 */
	if(tmp_win->title_w) {
		ComputeTitleLocation(tmp_win);
		CreateWindowTitlebarButtons(tmp_win);
		XMoveWindow(dpy, tmp_win->title_w,
		            tmp_win->title_x, tmp_win->title_y);
		XDefineCursor(dpy, tmp_win->title_w, Scr->TitleCursor);
	}
	else {
		tmp_win->title_x = tmp_win->frame_bw3D - tmp_win->frame_bw;
		tmp_win->title_y = tmp_win->frame_bw3D - tmp_win->frame_bw;
	}


	/*
	 * Setup various events we want to hear about related to this window.
	 */
	{
		unsigned long valuemask;
		XSetWindowAttributes attributes;

		valuemask = (CWEventMask | CWDontPropagate);
		attributes.event_mask = (StructureNotifyMask | PropertyChangeMask
		                         | ColormapChangeMask | VisibilityChangeMask
		                         | FocusChangeMask
		                         | EnterWindowMask | LeaveWindowMask);
		attributes.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask
		                                   | PointerMotionMask;
		XChangeWindowAttributes(dpy, tmp_win->w, valuemask, &attributes);
	}


	/*
	 * Map up the title window if we have one.  As a sub-window of the
	 * frame, it'll only actually show up in the screen if the frame
	 * does, of course.
	 */
	if(tmp_win->title_w) {
		XMapWindow(dpy, tmp_win->title_w);
	}


	/*
	 * If the server's got Shape, look up info about the window's
	 * Shape'ing, and subscribe to notifications about changes in it.
	 * Actually, it's only the bounding we care about; the rest is
	 * thrown away.
	 */
	if(HasShape) {
		int xws, yws, xbs, ybs;
		unsigned wws, hws, wbs, hbs;
		int boundingShaped, clipShaped;

		XShapeSelectInput(dpy, tmp_win->w, ShapeNotifyMask);
		XShapeQueryExtents(dpy, tmp_win->w,
		                   &boundingShaped, &xws, &yws, &wws, &hws,
		                   &clipShaped, &xbs, &ybs, &wbs, &hbs);
		tmp_win->wShaped = boundingShaped;
	}


	/*
	 * If it's a normal window (i.e., not one of ctwm's internal ones),
	 * add it to the "save set", which means that even if ctwm disappears
	 * without doing any cleanup, it'll still show back up on the screen
	 * like normal.  Otherwise, if you kill or segfault ctwm, all the
	 * other things you're running get their windows lost.
	 *
	 * XXX Conditional may be a little on the short side; I'm not sure it
	 * catches all of our internals...
	 */
	if(!(tmp_win->isiconmgr || tmp_win->iswspmgr || tmp_win->isoccupy)) {
		XAddToSaveSet(dpy, tmp_win->w);
	}


	/*
	 * Now reparent the real window into our frame.
	 */
	XReparentWindow(dpy, tmp_win->w, tmp_win->frame, tmp_win->frame_bw3D,
	                tmp_win->title_height + tmp_win->frame_bw3D);

	/*
	 * Reparenting generates an UnmapNotify event, followed by a
	 * MapNotify.  Set the map state to false to prevent a transition
	 * back to WithdrawnState in HandleUnmapNotify.  ->mapped gets set
	 * correctly again in HandleMapNotify.
	 */
	tmp_win->mapped = false;


	/*
	 * Call SetupFrame() which does all sorta of magic figuring to set
	 * the various coordinates and offsets and whatnot for all the pieces
	 * inside our frame.
	 */
	SetupFrame(tmp_win, tmp_win->frame_x, tmp_win->frame_y,
	           tmp_win->frame_width, tmp_win->frame_height, -1, true);


	/*
	 * Don't setup the icon window and its bits; when the window is
	 * iconified the first time, that handler will do what needs to be
	 * done for it, so we don't have to.
	 */


	/*
	 * If it's anything other than our own icon manager, setup button/key
	 * bindings for it.  For icon managers, this is done for them at the
	 * end of CreateIconManagers(), not here.  X-ref comments there and
	 * on the function defs below for some discussion about whether it
	 * _should_ work this way.
	 */
	if(!tmp_win->isiconmgr) {
		GrabButtons(tmp_win);
		GrabKeys(tmp_win);
	}


	/* Add this window to the appropriate icon manager[s] */
	AddIconManager(tmp_win);


	/*
	 * Stash up info about this TwmWindow and its screen in contexts on
	 * the real window and our various decorations around it.  This is
	 * how we find out what TwmWindow things like events are happening
	 * in.
	 */
#define SETCTXS(win) do { \
                XSaveContext(dpy, win, TwmContext, (XPointer) tmp_win); \
                XSaveContext(dpy, win, ScreenContext, (XPointer) Scr); \
        } while(0)

	/* The real window and our frame */
	SETCTXS(tmp_win->w);
	SETCTXS(tmp_win->frame);

	/* Cram that all into any titlebar [sub]windows too */
	if(tmp_win->title_height) {
		int i;
		int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

		SETCTXS(tmp_win->title_w);

		for(i = 0; i < nb; i++) {
			SETCTXS(tmp_win->titlebuttons[i].window);
		}
		if(tmp_win->hilite_wl) {
			SETCTXS(tmp_win->hilite_wl);
		}
		if(tmp_win->hilite_wr) {
			SETCTXS(tmp_win->hilite_wr);
		}
		if(tmp_win->lolite_wl) {
			SETCTXS(tmp_win->lolite_wl);
		}
		if(tmp_win->lolite_wr) {
			SETCTXS(tmp_win->lolite_wr);
		}
	}

#undef SETCTXS

	/*
	 * OK, that's all we need to do while the server's grabbed.  After
	 * this point, other clients might sneak in stuff between our
	 * actions, so they can't be considered atomic anymore.
	 */
	XUngrabServer(dpy);


	/*
	 * If we were in the middle of a menu activated function that was
	 * deferred (x-ref comments on DeferExecution()), re-grab to re-set
	 * the special cursor, since we may have reset it above.
	 *
	 * Why could that possibly happen?  It would require a window coming
	 * up and needing to be Add'd in the middle of selecting a window to
	 * apply a function to, which is a pretty rare case, but I s'pose not
	 * impossible...
	 */
	if(RootFunction) {
		ReGrab();
	}


	/*
	 * Add to the workspace manager.  Unless this IS the workspace
	 * manager, in which case that would just be silly.
	 */
	if(!tmp_win->iswspmgr) {
		WMapAddWindow(tmp_win);
	}


#ifdef CAPTIVE
	/*
	 * If ths window being created is a new captive [sub-]ctwm, we setup
	 * a property on it for unclear reasons.  x-ref comments on the
	 * function.
	 */
	SetPropsIfCaptiveCtwm(tmp_win);
#endif


	/*
	 * Init saved geometry with the current, as if f.savegeometry was
	 * called on the window right away.  That way f.restoregeometry can't
	 * get confuzzled.
	 */
	savegeometry(tmp_win);


	/*
	 * And that's it; we created all the bits!
	 */
	return tmp_win;
}




/*
 * XXX GrabButtons() and GrabKeys() are in a slightly odd state.  They're
 * almost strictly a piece of the window-adding process, which is why
 * they're here.  They're not static because the icon manager setup in
 * CreateIconManagers() calls them explicitly, because they're also
 * explicitly skipped in AddWindow() for icon manager windows.  I'm not
 * sure that's necessary; x-ref comment in CIM() about some ideas on the
 * matter.
 *
 * This should be resolved.  Until it is, they're left exported so the
 * iconmgr code can all them, and they're left here (rather than moved to
 * win_utils) on the guess that it may well be resolvable and so they'd
 * stay here and be staticized in the end.
 */

/***********************************************************************
 *
 *  Procedure:
 *      GrabButtons - grab needed buttons for the window
 *
 *  Inputs:
 *      tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

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
	                                 Mod5Mask
	                               };

	for(tmp = Scr->FuncButtonRoot.next; tmp != NULL; tmp = tmp->next) {
		if((tmp->cont != C_WINDOW) || (tmp->func == 0)) {
			continue;
		}
		grabbutton(tmp->num, tmp->mods, tmp_win->frame, GrabModeAsync);

		for(i = 0 ; i < 8 ; i++) {
			if((Scr->IgnoreModifier & ModifierMask [i]) && !(tmp->mods & ModifierMask [i]))
				grabbutton(tmp->num, tmp->mods | ModifierMask [i],
				           tmp_win->frame, GrabModeAsync);
		}
	}
	if(Scr->ClickToFocus) {
		grabbutton(AnyButton, None, tmp_win->w, GrabModeSync);
		for(i = 0 ; i < 8 ; i++) {
			grabbutton(AnyButton, ModifierMask [i], tmp_win->w, GrabModeSync);
		}
	}
	else if(Scr->RaiseOnClick) {
		grabbutton(Scr->RaiseOnClickButton, None, tmp_win->w, GrabModeSync);
		for(i = 0 ; i < 8 ; i++) {
			grabbutton(Scr->RaiseOnClickButton,
			           ModifierMask [i], tmp_win->w, GrabModeSync);
		}
	}
}
#undef grabbutton

/***********************************************************************
 *
 *  Procedure:
 *      GrabKeys - grab needed keys for the window
 *
 *  Inputs:
 *      tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

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
	                                 Mod5Mask
	                               };

	for(tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next) {
		switch(tmp->cont) {
			case C_WINDOW:
				/* case C_WORKSPACE: */
#define AltMask (Alt1Mask | Alt2Mask | Alt3Mask | Alt4Mask | Alt5Mask)
				if(tmp->mods & AltMask) {
					break;
				}
#undef AltMask

				grabkey(tmp, 0, tmp_win->w);

				for(i = 0 ; i < 8 ; i++) {
					if((Scr->IgnoreModifier & ModifierMask [i]) &&
					                !(tmp->mods & ModifierMask [i])) {
						grabkey(tmp, ModifierMask [i], tmp_win->w);
					}
				}
				break;

			case C_ICON:
				if(!tmp_win->icon || tmp_win->icon->w) {
					break;
				}

				grabkey(tmp, 0, tmp_win->icon->w);

				for(i = 0 ; i < 8 ; i++) {
					if((Scr->IgnoreModifier & ModifierMask [i]) &&
					                !(tmp->mods & ModifierMask [i])) {
						grabkey(tmp, ModifierMask [i], tmp_win->icon->w);
					}
				}
				break;

			case C_TITLE:
				if(!tmp_win->title_w) {
					break;
				}

				grabkey(tmp, 0, tmp_win->title_w);

				for(i = 0 ; i < 8 ; i++) {
					if((Scr->IgnoreModifier & ModifierMask [i]) &&
					                !(tmp->mods & ModifierMask [i])) {
						grabkey(tmp, ModifierMask [i], tmp_win->title_w);
					}
				}
				break;

			case C_NAME:
				grabkey(tmp, 0, tmp_win->w);
				for(i = 0 ; i < 8 ; i++) {
					if((Scr->IgnoreModifier & ModifierMask [i]) &&
					                !(tmp->mods & ModifierMask [i])) {
						grabkey(tmp, ModifierMask [i], tmp_win->w);
					}
				}
				if(tmp_win->icon && tmp_win->icon->w) {
					grabkey(tmp, 0, tmp_win->icon->w);

					for(i = 0 ; i < 8 ; i++) {
						if((Scr->IgnoreModifier & ModifierMask [i]) &&
						                !(tmp->mods & ModifierMask [i])) {
							grabkey(tmp, ModifierMask [i], tmp_win->icon->w);
						}
					}
				}
				if(tmp_win->title_w) {
					grabkey(tmp, 0, tmp_win->title_w);

					for(i = 0 ; i < 8 ; i++) {
						if((Scr->IgnoreModifier & ModifierMask [i]) &&
						                !(tmp->mods & ModifierMask [i])) {
							grabkey(tmp, ModifierMask [i], tmp_win->title_w);
						}
					}
				}
				break;

#ifdef EWMH_DESKTOP_ROOT
			case C_ROOT:
				if(tmp_win->ewmhWindowType != wt_Desktop) {
					break;
				}

				grabkey(tmp, 0, tmp_win->w);

				for(i = 0 ; i < 8 ; i++) {
					if((Scr->IgnoreModifier & ModifierMask [i]) &&
					                !(tmp->mods & ModifierMask [i])) {
						grabkey(tmp, ModifierMask [i], tmp_win->w);
					}
				}
				break;
#endif /* EWMH */

				/*
				case C_ROOT:
				    XGrabKey(dpy, tmp->keycode, tmp->mods, Scr->Root, True,
				        GrabModeAsync, GrabModeAsync);
				    break;
				*/
		}
	}
	for(tmp = Scr->FuncKeyRoot.next; tmp != NULL; tmp = tmp->next) {
		if(tmp->cont == C_ICONMGR && !Scr->NoIconManagers) {
			for(p = Scr->iconmgr; p != NULL; p = p->next) {
				ungrabkey(tmp, 0, p->twm_win->w);

				for(i = 0 ; i < 8 ; i++) {
					if((Scr->IgnoreModifier & ModifierMask [i]) &&
					                !(tmp->mods & ModifierMask [i])) {
						ungrabkey(tmp, ModifierMask [i], p->twm_win->w);
					}
				}
			}
		}
	}
}
#undef grabkey
#undef ungrabkey


#ifdef VSCREEN
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
static void
DealWithNonSensicalGeometries(Display *mydpy, Window vroot,
                              TwmWindow *tmp_win)
{
	Window              vvroot;
	int                 x, y;
	unsigned int        w, h;
	unsigned int        j;
	VirtualScreen       *myvs, *vs;
	int                 dropx = 0;

	if(! vroot) {
		return;
	}

	if(!(XGetGeometry(mydpy, vroot, &vvroot, &x, &y, &w, &h, &j, &j))) {
		return;
	}

	myvs = findIfVScreenOf(x, y);

	/*
	 * probably need to rethink this  for unmapped vs's.  ugh.
	 */
	if(!myvs) {
		return;
	}

	for(vs = myvs->next; vs; vs = vs->next) {
		dropx += vs->w;
	}

	for(vs = Scr->vScreenList; vs && vs != myvs; vs = vs->next) {
		dropx -= vs->w;
	}

	if(tmp_win->frame_x > 0 && tmp_win->frame_x >= w) {
		tmp_win->frame_x -= abs(dropx);
	}
	else {
	}

}
#endif // VSCREEN
