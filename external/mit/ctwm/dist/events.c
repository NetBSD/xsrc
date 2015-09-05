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


/***********************************************************************
 *
 * $XConsortium: events.c,v 1.182 91/07/17 13:59:14 dave Exp $
 *
 * twm event handling
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 ***********************************************************************/

#include <stdio.h>
#include <errno.h>
#ifndef VMS
#include <sys/time.h>
#endif
#if defined(AIXV3) || defined(_SYSTYPE_SVR4) || defined(ibm) || defined __QNX__
#include <sys/select.h>
#endif
#include <ctype.h>

#include "twm.h"
#ifdef VMS
#include <decw$include/Xatom.h>
#else
#include <X11/Xatom.h>
#endif
#include "add_window.h"
#include "clicktofocus.h"
#include "menus.h"
#include "events.h"
#include "resize.h"
#include "parse.h"
#include "util.h"
#include "screen.h"
#include "icons.h"
#include "iconmgr.h"
#include "version.h"

#ifdef VMS
#include <starlet.h>
#include <ssdef.h>
#include <lib$routines.h>
#define USE_SIGNALS
#else
#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#define ABS(x) ((x)<0?-(x):(x))

extern int iconifybox_width, iconifybox_height;
extern unsigned int mods_used;
extern int menuFromFrameOrWindowOrTitlebar;
extern char *CurrentSelectedWorkspace;
extern int RaiseDelay;

#ifdef USE_SIGNALS
extern Bool AnimationPending;
#else /* USE_SIGNALS */
extern struct timeval AnimateTimeout;
#endif /* USE_SIGNALS */
extern int  AnimationSpeed;
extern Bool AnimationActive;
extern Bool MaybeAnimate;

extern int  AlternateKeymap;
extern Bool AlternateContext;

static void CtwmNextEvent (Display *display, XEvent  *event);
static void RedoIcon(void);
static void do_key_menu (MenuRoot *menu,	/* menu to pop up */
			 Window w);		/* invoking window or None */
void RedoIconName(void);
extern void twmrc_error_prefix(void);

#ifdef SOUNDS
extern void play_sound(int snd);
#endif
FILE *tracefile = NULL;

#define MAX_X_EVENT 256
event_proc EventHandler[MAX_X_EVENT]; /* event handler jump table */
char *Action;
int Context = C_NO_CONTEXT;	/* current button press context */
TwmWindow *ButtonWindow;	/* button press window structure */
XEvent ButtonEvent;		/* button press event */
XEvent Event;			/* the current event */
TwmWindow *Tmp_win;		/* the current twm window */

extern Window captiveroot;
Window DragWindow;		/* variables used in moving windows */
int origDragX;
int origDragY;
int DragX;
int DragY;
unsigned int DragWidth;
unsigned int DragHeight;
unsigned int DragBW;
int CurrentDragX;
int CurrentDragY;

/* Vars to tell if the resize has moved. */
extern int ResizeOrigX;
extern int ResizeOrigY;

static int enter_flag;
static int leave_flag;
static int ColortableThrashing;
static TwmWindow *enter_win, *raise_win, *leave_win, *lower_win;

int ButtonPressed = -1;
int Cancel = FALSE;

void HandleCreateNotify(void);
void HandleShapeNotify (void);
void HandleFocusChange (void);
extern int ShapeEventBase, ShapeErrorBase;

extern Window lowerontop;

#ifdef GNOME
#  include "gnomewindefs.h"
  extern Atom _XA_WIN_WORKSPACE;
  extern Atom _XA_WIN_STATE;
#endif /* GNOME */

extern Atom _XA_WM_OCCUPATION;
extern Atom _XA_WM_CURRENTWORKSPACE;

int GnomeProxyButtonPress = -1;

/*#define TRACE_FOCUS*/
/*#define TRACE*/

static void dumpevent (XEvent *e);

#if defined(__hpux) && !defined(_XPG4_EXTENDED)
#   define FDSET int*
#else
#   define FDSET fd_set*
#endif

static unsigned int set_mask_ignore (unsigned int modifier)
{
    int i;
    unsigned int ModifierMask[8] = { ShiftMask, ControlMask, LockMask,
				     Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask,
				     Mod5Mask };

    if (Scr->IgnoreLockModifier) modifier &= ~LockMask;
    for (i = 0 ; i < 8 ; i++) {
	if (Scr->IgnoreModifier & ModifierMask [i]) modifier &= ~ModifierMask [i];
    }
    return modifier;
}

void AutoRaiseWindow (TwmWindow *tmp)
{
    RaiseWindow (tmp);

    if (ActiveMenu && ActiveMenu->w) XRaiseWindow (dpy, ActiveMenu->w);
    XSync (dpy, 0);
    enter_win = NULL;
    enter_flag = TRUE;
    raise_win = tmp;
    WMapRaise (tmp);
}

void SetRaiseWindow (TwmWindow *tmp)
{
    enter_flag = TRUE;
    enter_win = NULL;
    raise_win = tmp;
    leave_win = NULL;
    leave_flag = FALSE;
    lower_win = NULL;
    XSync (dpy, 0);
}

void AutoLowerWindow (TwmWindow *tmp)
{
    LowerWindow (tmp);

    if (ActiveMenu && ActiveMenu->w) XRaiseWindow (dpy, ActiveMenu->w);
    XSync (dpy, 0);
    enter_win = NULL;
    enter_flag = FALSE;
    raise_win = NULL;
    leave_win = NULL;
    leave_flag = TRUE;
    lower_win = tmp;
    WMapLower (tmp);
}



/***********************************************************************
 *
 *  Procedure:
 *	InitEvents - initialize the event jump table
 *
 ***********************************************************************
 */

void InitEvents(void)
{
    int i;


    ResizeWindow = (Window) 0;
    DragWindow = (Window) 0;
    enter_flag = FALSE;
    enter_win = raise_win = NULL;
    leave_flag = FALSE;
    leave_win = lower_win = NULL;

    for (i = 0; i < MAX_X_EVENT; i++)
	EventHandler[i] = HandleUnknown;

    EventHandler[Expose] = HandleExpose;
    EventHandler[CreateNotify] = HandleCreateNotify;
    EventHandler[DestroyNotify] = HandleDestroyNotify;
    EventHandler[MapRequest] = HandleMapRequest;
    EventHandler[MapNotify] = HandleMapNotify;
    EventHandler[UnmapNotify] = HandleUnmapNotify;
    EventHandler[MotionNotify] = HandleMotionNotify;
    EventHandler[ButtonRelease] = HandleButtonRelease;
    EventHandler[ButtonPress] = HandleButtonPress;
    EventHandler[EnterNotify] = HandleEnterNotify;
    EventHandler[LeaveNotify] = HandleLeaveNotify;
    EventHandler[ConfigureRequest] = HandleConfigureRequest;
    EventHandler[ClientMessage] = HandleClientMessage;
    EventHandler[PropertyNotify] = HandlePropertyNotify;
    EventHandler[KeyPress] = HandleKeyPress;
    EventHandler[KeyRelease] = HandleKeyRelease;
    EventHandler[ColormapNotify] = HandleColormapNotify;
    EventHandler[VisibilityNotify] = HandleVisibilityNotify;
    EventHandler[FocusIn] = HandleFocusChange;
    EventHandler[FocusOut] = HandleFocusChange;
    if (HasShape)
	EventHandler[ShapeEventBase+ShapeNotify] = HandleShapeNotify;
}




Time lastTimestamp = CurrentTime;	/* until Xlib does this for us */

Bool StashEventTime (register XEvent *ev)
{
    switch (ev->type) {
      case KeyPress:
      case KeyRelease:
	lastTimestamp = ev->xkey.time;
	return True;
      case ButtonPress:
      case ButtonRelease:
	lastTimestamp = ev->xbutton.time;
	return True;
      case MotionNotify:
	lastTimestamp = ev->xmotion.time;
	return True;
      case EnterNotify:
      case LeaveNotify:
	lastTimestamp = ev->xcrossing.time;
	return True;
      case PropertyNotify:
	lastTimestamp = ev->xproperty.time;
	return True;
      case SelectionClear:
	lastTimestamp = ev->xselectionclear.time;
	return True;
      case SelectionRequest:
	lastTimestamp = ev->xselectionrequest.time;
	return True;
      case SelectionNotify:
	lastTimestamp = ev->xselection.time;
	return True;
    }
    return False;
}



/*
 * WindowOfEvent - return the window about which this event is concerned; this
 * window may not be the same as XEvent.xany.window (the first window listed
 * in the structure).
 */
Window WindowOfEvent (XEvent *e)
{
    /*
     * Each window subfield is marked with whether or not it is the same as
     * XEvent.xany.window or is different (which is the case for some of the
     * notify events).
     */
    switch (e->type) {
      case KeyPress:
      case KeyRelease:  return e->xkey.window;			     /* same */
      case ButtonPress:
      case ButtonRelease:  return e->xbutton.window;		     /* same */
      case MotionNotify:  return e->xmotion.window;		     /* same */
      case EnterNotify:
      case LeaveNotify:  return e->xcrossing.window;		     /* same */
      case FocusIn:
      case FocusOut:  return e->xfocus.window;			     /* same */
      case KeymapNotify:  return e->xkeymap.window;		     /* same */
      case Expose:  return e->xexpose.window;			     /* same */
      case GraphicsExpose:  return e->xgraphicsexpose.drawable;	     /* same */
      case NoExpose:  return e->xnoexpose.drawable;		     /* same */
      case VisibilityNotify:  return e->xvisibility.window;	     /* same */
      case CreateNotify:  return e->xcreatewindow.window;	     /* DIFF */
      case DestroyNotify:  return e->xdestroywindow.window;	     /* DIFF */
      case UnmapNotify:  return e->xunmap.window;		     /* DIFF */
      case MapNotify:  return e->xmap.window;			     /* DIFF */
      case MapRequest:  return e->xmaprequest.window;		     /* DIFF */
      case ReparentNotify:  return e->xreparent.window;		     /* DIFF */
      case ConfigureNotify:  return e->xconfigure.window;	     /* DIFF */
      case ConfigureRequest:  return e->xconfigurerequest.window;    /* DIFF */
      case GravityNotify:  return e->xgravity.window;		     /* DIFF */
      case ResizeRequest:  return e->xresizerequest.window;	     /* same */
      case CirculateNotify:  return e->xcirculate.window;	     /* DIFF */
      case CirculateRequest:  return e->xcirculaterequest.window;    /* DIFF */
      case PropertyNotify:  return e->xproperty.window;		     /* same */
      case SelectionClear:  return e->xselectionclear.window;	     /* same */
      case SelectionRequest: return e->xselectionrequest.requestor;  /* DIFF */
      case SelectionNotify:  return e->xselection.requestor;	     /* same */
      case ColormapNotify:  return e->xcolormap.window;		     /* same */
      case ClientMessage:  return e->xclient.window;		     /* same */
      case MappingNotify:  return None;
    }
    return None;
}

void FixRootEvent (XEvent *e)
{
    if (Scr->Root == Scr->RealRoot)
	return;

    switch (e->type) {
      case KeyPress:
      case KeyRelease:
	  e->xkey.x_root -= Scr->rootx;
	  e->xkey.y_root -= Scr->rooty;
	  e->xkey.root    = Scr->Root;
	  break;
      case ButtonPress:
      case ButtonRelease:
	  e->xbutton.x_root -= Scr->rootx;
	  e->xbutton.y_root -= Scr->rooty;
	  e->xbutton.root    = Scr->Root;
	  break;
      case MotionNotify:
	  e->xmotion.x_root -= Scr->rootx;
	  e->xmotion.y_root -= Scr->rooty;
	  e->xmotion.root    = Scr->Root;
	  break;
      case EnterNotify:
      case LeaveNotify:
	  e->xcrossing.x_root -= Scr->rootx;
	  e->xcrossing.y_root -= Scr->rooty;
	  e->xcrossing.root    = Scr->Root;
	  break;
      default:
	  break;
    }
}



/* Move this next to GetTwmWindow()? */
static ScreenInfo *GetTwmScreen(XEvent *event)
{
    ScreenInfo *scr;

    if (XFindContext(dpy, event->xany.window, ScreenContext,
					(XPointer *)&scr) == XCNOENT) {
	scr = FindScreenInfo(WindowOfEvent(event));
    }

    return scr;
}

/***********************************************************************
 *
 *  Procedure:
 *	DispatchEvent2 -
 *      handle a single X event stored in global var Event
 *      this routine for is for a call during an f.move
 *
 ***********************************************************************
 */
Bool DispatchEvent2 (void)
{
    Window w = Event.xany.window;
    ScreenInfo *lastScr = Scr;   /* XXX_MIKE - assume Scr OK on entry... */
    StashEventTime (&Event);

    Tmp_win = GetTwmWindow(w);
    Scr = GetTwmScreen(&Event);

    dumpevent(&Event);

    if (!Scr) {
	Scr = lastScr;	    /* XXX_MIKE - try not to leave Scr NULL */
	return False;
    }
    FixRootEvent (&Event);

#ifdef SOUNDS
    play_sound(Event.type);
#endif

    if (menuFromFrameOrWindowOrTitlebar) {
	if (Event.type == Expose)
	    HandleExpose();
    } else {
	if (Event.type>= 0 && Event.type < MAX_X_EVENT)
	    (*EventHandler[Event.type])();
    }

    return True;
}

/***********************************************************************
 *
 *  Procedure:
 *	DispatchEvent - handle a single X event stored in global var Event
 *
 ***********************************************************************
 */
Bool DispatchEvent (void)
{
    Window w = Event.xany.window;
    ScreenInfo *lastScr = Scr;   /* XXX_MIKE - assume Scr OK on entry... */
    StashEventTime (&Event);

    Tmp_win = GetTwmWindow(w);
    Scr = GetTwmScreen(&Event);

    dumpevent(&Event);

    if (!Scr) {
	Scr = lastScr;	    /* XXX_MIKE - try not to leave Scr NULL */
	return False;
    }

    if (captive) {
      if ((Event.type == ConfigureNotify) && (Event.xconfigure.window == Scr->CaptiveRoot)) {
	ConfigureRootWindow (&Event);
	return (False);
      }
    }
    FixRootEvent (&Event);
    if (Event.type>= 0 && Event.type < MAX_X_EVENT) {
#ifdef SOUNDS
        play_sound(Event.type);
#endif
	(*EventHandler[Event.type])();
    }
    return True;
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleEvents - handle X events
 *
 ***********************************************************************
 */

void HandleEvents(void)
{
    while (TRUE)
    {
	if (enter_flag && !QLength(dpy)) {
	    if (enter_win && enter_win != raise_win) {
		AutoRaiseWindow (enter_win);  /* sets enter_flag T */
	    } else {
		enter_flag = FALSE;
	    }
	}
	if ( leave_flag && !QLength(dpy) ) {
	    if (leave_win && leave_win != lower_win) {
		AutoLowerWindow (leave_win); /* sets leave_flag T */
	    } else {
		leave_flag = FALSE;
	    }
	}
	if (ColortableThrashing && !QLength(dpy) && Scr) {
	    InstallColormaps(ColormapNotify, NULL);
	}
	WindowMoved = FALSE;

	CtwmNextEvent (dpy, &Event);

	if (Event.type < 0 || Event.type >= MAX_X_EVENT)
	    XtDispatchEvent (&Event);
	else

	(void) DispatchEvent ();
    }
}

#define nextEvent(event) XtAppNextEvent(appContext, event);

#ifdef VMS
extern unsigned long timefe;
#endif

static void CtwmNextEvent (Display *display, XEvent  *event)
{
    int animate = (AnimationActive && MaybeAnimate);

#ifdef VMS
    if (QLength (display) != 0) {
	nextEvent (event);
	return;
    }
    if (animate && AnimationPending) Animate ();
    while (1) {
       sys$waitfr(timefe);
       sys$clref(timefe);

       if (animate && AnimationPending) Animate ();
       if (QLength (display) != 0) {
	  nextEvent (event);
	  return;
       }
    }
#else /* VMS */
    int		found;
    fd_set	mask;
    int		fd;
    struct timeval timeout, *tout = NULL;

    if (RestartFlag)
	DoRestart(CurrentTime);
    if (XEventsQueued (display, QueuedAfterFlush) != 0) {
	nextEvent (event);
	return;
    }
    fd = ConnectionNumber (display);

#ifdef USE_SIGNALS
    if (animate && AnimationPending) Animate ();
    while (1) {
	FD_ZERO (&mask);
	FD_SET  (fd, &mask);
	found = select (fd + 1, (FDSET)&mask, (FDSET) 0, (FDSET) 0, 0);
	if (RestartFlag)
	    DoRestart(CurrentTime);
	if (found < 0) {
	    if (errno == EINTR) {
		if (animate)
		    Animate ();
	    }
	    else perror ("select");
	    continue;
	}
	if (FD_ISSET (fd, &mask)) {
	    nextEvent (event);
	    return;
	}
    }
#else /* USE_SIGNALS */
    if (animate) TryToAnimate ();
    if (RestartFlag)
	DoRestart(CurrentTime);
    if (! MaybeAnimate) {
	nextEvent (event);
	return;
    }
    if (animate) tout = (AnimationSpeed > 0) ? &timeout : NULL;
    while (1) {
	FD_ZERO (&mask);
	FD_SET  (fd, &mask);
	if (animate) {
	    timeout = AnimateTimeout;
	}
	found = select (fd + 1, (FDSET)&mask, (FDSET) 0, (FDSET) 0, tout);
	if (RestartFlag)
	    DoRestart(CurrentTime);
	if (found < 0) {
	    if (errno != EINTR) perror ("select");
	    continue;
	}
	if (FD_ISSET (fd, &mask)) {
	    nextEvent (event);
	    return;
	}
	if (found == 0) {
	    if (animate) TryToAnimate ();
	    if (RestartFlag)
		DoRestart(CurrentTime);
	    if (! MaybeAnimate) {
		nextEvent (event);
		return;
	    }
	    continue;
	}
    }
#endif /* USE_SIGNALS */
#endif /* VMS */
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleColormapNotify - colormap notify event handler
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
    if (XFindContext(dpy, cevent->window, ColormapContext, (XPointer *)&cwin) == XCNOENT)
	return;
    cmap = cwin->colormap;

    if (cevent->new)
    {
	if (XFindContext(dpy, cevent->colormap, ColormapContext,
			 (XPointer *)&cwin->colormap) == XCNOENT)
	    cwin->colormap = CreateTwmColormap(cevent->colormap);
	else
	    cwin->colormap->refcnt++;

	cmap->refcnt--;

	if (cevent->state == ColormapUninstalled)
	    cmap->state &= ~CM_INSTALLED;
	else
	    cmap->state |= CM_INSTALLED;

	if (cmap->state & CM_INSTALLABLE) {
	    InstallColormaps(ColormapNotify, NULL);
	}

	if (cmap->refcnt == 0)
	{
	    XDeleteContext(dpy, cmap->c, ColormapContext);
	    free((char *) cmap);
	}

	return;
    }

    if (cevent->state == ColormapUninstalled &&
	(cmap->state & CM_INSTALLABLE))
    {
	if (!(cmap->state & CM_INSTALLED))
	    return;
	cmap->state &= ~CM_INSTALLED;

	if (!ColortableThrashing)
	{
	    ColortableThrashing = TRUE;
	    XSync(dpy, 0);
	}

	if (cevent->serial >= Scr->cmapInfo.first_req)
	{
	    number_cwins = Scr->cmapInfo.cmaps->number_cwins;

	    /*
	     * Find out which colortables collided.
	     */

	    cwins = Scr->cmapInfo.cmaps->cwins;
	    for (lost = won = -1, n = 0;
		 (lost == -1 || won == -1) && n < number_cwins;
		 n++)
	    {
		if (lost == -1 && cwins[n] == cwin)
		{
		    lost = n;	/* This is the window which lost its colormap */
		    continue;
		}

		if (won == -1 &&
		    cwins[n]->colormap->install_req == cevent->serial)
		{
		    won = n;	/* This is the window whose colormap caused */
		    continue;	/* the de-install of the previous colormap */
		}
	    }

	    /*
	    ** Cases are:
	    ** Both the request and the window were found:
	    **		One of the installs made honoring the WM_COLORMAP
	    **		property caused another of the colormaps to be
	    **		de-installed, just mark the scoreboard.
	    **
	    ** Only the request was found:
	    **		One of the installs made honoring the WM_COLORMAP
	    **		property caused a window not in the WM_COLORMAP
	    **		list to lose its map.  This happens when the map
	    **		it is losing is one which is trying to be installed,
	    **		but is getting getting de-installed by another map
	    **		in this case, we'll get a scoreable event later,
	    **		this one is meaningless.
	    **
	    ** Neither the request nor the window was found:
	    **		Somebody called installcolormap, but it doesn't
	    **		affect the WM_COLORMAP windows.  This case will
	    **		probably never occur.
	    **
	    ** Only the window was found:
	    **		One of the WM_COLORMAP windows lost its colormap
	    **		but it wasn't one of the requests known.  This is
	    **		probably because someone did an "InstallColormap".
	    **		The colormap policy is "enforced" by re-installing
	    **		the colormaps which are believed to be correct.
	    */

	    if (won != -1) {
		if (lost != -1)
		{
		    /* lower diagonal index calculation */
		    if (lost > won)
			n = lost*(lost-1)/2 + won;
		    else
			n = won*(won-1)/2 + lost;
		    Scr->cmapInfo.cmaps->scoreboard[n] = 1;
		} else
		{
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
	    } else if (lost != -1) {
		InstallColormaps(ColormapNotify, NULL);
	    } else {
		ColortableThrashing = FALSE; /* Gross Hack for HP WABI. CL. */
	    }
	}
    }

    else if (cevent->state == ColormapUninstalled)
	cmap->state &= ~CM_INSTALLED;

    else if (cevent->state == ColormapInstalled)
	cmap->state |= CM_INSTALLED;
}



/*
 * LastFocusEvent -- skip over focus in/out events for this
 *		window.
 */

static XEvent *LastFocusEvent(Window w, XEvent *first)
{
	static XEvent current;
	XEvent *last, new;

	new= *first;
	last=NULL;
	
	do {
		if ( (new.type == FocusIn || new.type == FocusOut) 
		    && new.xfocus.mode == NotifyNormal 
		    && (new.xfocus.detail == NotifyNonlinear 
			|| new.xfocus.detail == NotifyPointer
			|| new.xfocus.detail == NotifyAncestor
			|| (new.xfocus.detail == NotifyNonlinearVirtual)
			))
		{
			current=new;
			last= &current;
#ifdef TRACE_FOCUS
			printf("! %s 0x%x mode=%d, detail=%d\n", 
			       new.xfocus.type == FocusIn?"in":"out",
			       Tmp_win,new.xfocus.mode, new.xfocus.detail);
#endif       
		}
		else
		{
#ifdef TRACE_FOCUS
			printf("~ %s 0x%x mode=%d, detail=%d\n", 
			       new.xfocus.type == FocusIn?"in":"out",
			       Tmp_win,new.xfocus.mode, new.xfocus.detail);
#endif
		}
	} while (XCheckWindowEvent(dpy, w, FocusChangeMask, &new));
	return last;
}

/*
 * HandleFocusIn -- deal with the focus moving under us.
 */

void HandleFocusIn(XFocusInEvent *event)
{

#ifdef TRACE_FOCUS
	printf("HandleFocusIn : +0x%x (0x%x, 0x%x), mode=%d, detail=%d\n", 
	       Tmp_win, Tmp_win->w, event->window, event->mode, event->detail);
#endif

    if (Tmp_win->iconmgr) return;
    if (Tmp_win->wmhints && ! Tmp_win->wmhints->input) return;
    if (Scr->Focus == Tmp_win) return;
    if (Tmp_win->AutoSqueeze && Tmp_win->squeezed) AutoSqueeze (Tmp_win);
    SetFocusVisualAttributes (Tmp_win, True);
    Scr->Focus = Tmp_win;
}

void HandleFocusOut(XFocusOutEvent *event)
{
#ifdef TRACE_FOCUS
	printf("HandleFocusOut : -0x%x (0x%x, 0x%x), mode=%d, detail=%d\n", 
	       Tmp_win, Tmp_win->w, event->window, event->mode, event->detail);
#endif

    if (Tmp_win->iconmgr) return;
    if (Scr->Focus != Tmp_win) return;
    if (Scr->SloppyFocus) return;
    if (Tmp_win->AutoSqueeze && !Tmp_win->squeezed) AutoSqueeze (Tmp_win);
    SetFocusVisualAttributes (Tmp_win, False);
    Scr->Focus= NULL;
}

void HandleFocusChange(void)
{
	XEvent *event;
	
	if (Tmp_win)
	{
		event = LastFocusEvent(Event.xany.window,&Event);
		
		if ( event != NULL)
		{
			if (event->type == FocusIn)
			  HandleFocusIn(&event->xfocus);
			else
			  HandleFocusOut(&event->xfocus);
		}
	}
}

void SynthesiseFocusOut(Window w)
{
	XEvent event;

#ifdef TRACE_FOCUS
	printf ("Synthesizing FocusOut on %x\n", w);
#endif

	event.type=FocusOut;
	event.xfocus.window=w;
	event.xfocus.mode=NotifyNormal;
	event.xfocus.detail=NotifyPointer;
	
	XPutBackEvent(dpy, &event);
}


void SynthesiseFocusIn(Window w)
{
	XEvent event;

#ifdef TRACE_FOCUS
	printf ("Synthesizing FocusIn on %x\n", w);
#endif

	event.type=FocusIn;
	event.xfocus.window=w;
	event.xfocus.mode=NotifyNormal;
	event.xfocus.detail=NotifyPointer;
	
	XPutBackEvent(dpy, &event);

}


/***********************************************************************
 *
 *  Procedure:
 *	HandleVisibilityNotify - visibility notify event handler
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

    if (XFindContext(dpy, vevent->window, ColormapContext, (XPointer *)&cwin) == XCNOENT)
	return;
    
    /*
     * when Saber complains about retreiving an <int> from an <unsigned int>
     * just type "touch vevent->state" and "cont"
     */
    cmap = cwin->colormap;
    if ((cmap->state & CM_INSTALLABLE) &&
	vevent->state != cwin->visibility &&
	(vevent->state == VisibilityFullyObscured ||
	 cwin->visibility == VisibilityFullyObscured) &&
	cmap->w == cwin->w) {
	cwin->visibility = vevent->state;
	InstallWindowColormaps(VisibilityNotify, (TwmWindow *) NULL);
    } else
	cwin->visibility = vevent->state;
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleKeyRelease - key release event handler
 *
 ***********************************************************************
 */

void HandleKeyRelease(void)
{
  if (Tmp_win == Scr->currentvs->wsw->twm_win)
    WMgrHandleKeyReleaseEvent (Scr->currentvs, &Event);
}
/***********************************************************************
 *
 *  Procedure:
 *	HandleKeyPress - key press event handler
 *
 ***********************************************************************
 */

void HandleKeyPress(void)
{
    FuncKey *key;
    int len;
    unsigned int modifier;
    Window w;

    if (InfoLines) XUnmapWindow(dpy, Scr->InfoWindow);

    if (ActiveMenu != NULL) {
	MenuItem *item;
	int	 offset;
	char *keynam;
	KeySym keysym;
	int xx, yy, wx, wy;
	Window junkW;

	item = (MenuItem*) 0;

	keysym = XLookupKeysym  ((XKeyEvent*) &Event, 0);
	if (! keysym) return;
	keynam = XKeysymToString (keysym);
	if (! keynam) return;

	if (!strcmp (keynam, "Down") || !strcmp (keynam, "space")) {
	    xx = Event.xkey.x;
	    yy = Event.xkey.y + Scr->EntryHeight;
	    XTranslateCoordinates (dpy, Scr->Root, ActiveMenu->w, xx, yy, &wx, &wy, &junkW);
	    if ((wy < 0) || (wy > ActiveMenu->height))
		yy -= (wy - (Scr->EntryHeight / 2) - 2);
	    if ((wx < 0) || (wx > ActiveMenu->width))
		xx -= (wx - (ActiveMenu->width / 2));
	    XWarpPointer (dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			ActiveMenu->width, ActiveMenu->height, xx, yy);
	    return;
	}
	else
	if (!strcmp (keynam, "Up")) {
	    xx = Event.xkey.x;
	    yy = Event.xkey.y - Scr->EntryHeight;
	    XTranslateCoordinates (dpy, Scr->Root, ActiveMenu->w, xx, yy, &wx, &wy, &junkW);
	    if ((wy < 0) || (wy > ActiveMenu->height))
		yy -= (wy - ActiveMenu->height + (Scr->EntryHeight / 2) + 2);
	    if ((wx < 0) || (wx > ActiveMenu->width))
		xx -= (wx - (ActiveMenu->width / 2));
	    XWarpPointer (dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			ActiveMenu->width, ActiveMenu->height, xx, yy);
	    return;
	}
	else
	if (!strcmp (keynam, "Right") || !strcmp (keynam, "Return")) {
	    item = ActiveItem;
	}
	else
	if (!strcmp (keynam, "Left") || !strcmp(keynam, "Escape")) {
	    MenuRoot *menu;

	    if (ActiveMenu->pinned) return;
	    if (!ActiveMenu->prev || MenuDepth == 1) {
		PopDownMenu ();
		XUngrabPointer  (dpy, CurrentTime);
		return;
	    }
	    xx = Event.xkey.x;
	    yy = Event.xkey.y;
	    menu = ActiveMenu->prev;
	    XTranslateCoordinates (dpy, Scr->Root, menu->w, xx, yy, &wx, &wy, &junkW);
	    xx -= (wx - (menu->width / 2));
	    if (menu->lastactive)
		yy -= (wy - menu->lastactive->item_num * Scr->EntryHeight -
			(Scr->EntryHeight / 2) - 2);
	    else
		yy -= (wy - (Scr->EntryHeight / 2) - 2);
	    XUnmapWindow (dpy, ActiveMenu->w);
	    if (Scr->Shadow) XUnmapWindow (dpy, ActiveMenu->shadow);
	    XWarpPointer (dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			menu->width, menu->height, xx, yy);
	    return;
	}
	else
	if (strlen (keynam) == 1) {
	    MenuItem *startitem;
	    xx = Event.xkey.x;
	    yy = Event.xkey.y;

	    startitem = ActiveItem ? ActiveItem : ActiveMenu->first;
	    item = startitem->next;
	    if (item == (MenuItem*) 0) item = ActiveMenu->first;
	    modifier = (Event.xkey.state & mods_used);
	    modifier = set_mask_ignore (modifier);

	    while (item != startitem) {
		Boolean	 matched = False;
		offset = 0;
		switch (item->item [0]) {
		    case '^' :
			if ((modifier & ControlMask) &&
			    (keynam [0] == Tolower (item->item [1])))
			    matched = True;
			break;
		    case '~' :
			if ((modifier & Mod1Mask) &&
			    (keynam [0] == Tolower (item->item [1])))
			    matched = True;
			break;
		    case ' ' :
			offset = 1;
		    default :
			if (((Scr->IgnoreCaseInMenuSelection) &&
			    (keynam [0] == Tolower (item->item [offset]))) ||

			     ((modifier & ShiftMask) && Isupper (item->item [offset]) &&
			     (keynam [0] == Tolower (item->item [offset]))) ||

			    (!(modifier & ShiftMask) && Islower (item->item [offset]) &&
			     (keynam [0] == item->item [offset]))) matched = True;
			break;
		}
		if (matched) break;
		item = item->next;
		if (item == (MenuItem*) 0) item = ActiveMenu->first;
	    }
	    if (item == startitem) return;
	    wx = ActiveMenu->width / 2;
	    wy = (item->item_num * Scr->EntryHeight) + (Scr->EntryHeight / 2) + 2;
	    XTranslateCoordinates (dpy, ActiveMenu->w, Scr->Root, wx, wy, &xx, &yy, &junkW);
	    XWarpPointer (dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
			ActiveMenu->width, ActiveMenu->height, xx, yy);
	    return;
	}
	else return;
	if (item) {
	    switch (item->func) {
		case 0 :
		case F_TITLE :
		    break;

		case F_MENU :
		    if (!strcmp (keynam, "Return")) {
			if (ActiveMenu == Scr->Workspaces) {
			    PopDownMenu();
			    XUngrabPointer  (dpy, CurrentTime);
			    GotoWorkSpaceByName (Scr->currentvs, item->action + 8);
			}
			else {
			    ExecuteFunction (item->func, item->action,
				ButtonWindow ? ButtonWindow->frame : None,
				ButtonWindow, &Event, Context, FALSE);
			    PopDownMenu();
			}
			return;
		    }
		    xx = Event.xkey.x;
		    yy = Event.xkey.y;
		    XTranslateCoordinates (dpy, Scr->Root, ActiveMenu->w, xx, yy,
				&wx, &wy, &junkW);
		    if (ActiveItem) {
			ActiveItem->state = 0;
			PaintEntry (ActiveMenu, ActiveItem,  False);
			ActiveItem = NULL;
		    }
		    xx -= (wx - ActiveMenu->width);
		    yy -= (wy - item->item_num * Scr->EntryHeight - (Scr->EntryHeight / 2) - 2);
		    Event.xkey.x_root = xx;
		    Event.xkey.y_root = yy;
		    XWarpPointer (dpy, Scr->Root, Scr->Root, Event.xkey.x, Event.xkey.y,
				ActiveMenu->width, ActiveMenu->height, xx, yy);
		    if (ActiveMenu == Scr->Workspaces)
			CurrentSelectedWorkspace = item->item;
		    do_key_menu (item->sub, None);
		    CurrentSelectedWorkspace = NULL;
		    break;

		default :
		    if (item->func != F_PIN) PopDownMenu();
		    ExecuteFunction (item->func, item->action,
			ButtonWindow ? ButtonWindow->frame : None,
			ButtonWindow, &Event, Context, FALSE);
	    }
	}
	else {
	    PopDownMenu();
	    XUngrabPointer  (dpy, CurrentTime);
	}
	return;
    }

    Context = C_NO_CONTEXT;
    if (Event.xany.window == Scr->Root) {
	if (AlternateContext) {
	    XUngrabPointer  (dpy, CurrentTime);
	    XUngrabKeyboard (dpy, CurrentTime);
	    AlternateContext = False;
	    Context = C_ALTERNATE;
	}
	else
	if (AlternateKeymap && Event.xkey.subwindow) {
	    w = Event.xkey.subwindow;
	    Tmp_win = GetTwmWindow(w);
	    if (Tmp_win) Event.xany.window = Tmp_win->w;
	}
	else Context = C_ROOT;
    }
    if (Tmp_win)
    {
	if (Event.xany.window == Tmp_win->title_w)
	    Context = C_TITLE;
	if (Event.xany.window == Tmp_win->w)
	    Context = C_WINDOW;
	if (Tmp_win->icon && (Event.xany.window == Tmp_win->icon->w))
	    Context = C_ICON;
	if (Event.xany.window == Tmp_win->frame)
	    Context = C_FRAME;
	if (Tmp_win->iconmanagerlist) {
	    if (Event.xany.window == Tmp_win->iconmanagerlist->w ||
		Event.xany.window == Tmp_win->iconmanagerlist->icon)
		Context = C_ICONMGR;
	}
	if (Tmp_win->wspmgr)
	    Context = C_WORKSPACE;
    }

    modifier = (Event.xkey.state | AlternateKeymap) & mods_used;
    modifier = set_mask_ignore (modifier);
    if (AlternateKeymap) {
	XUngrabPointer  (dpy, CurrentTime);
	XUngrabKeyboard (dpy, CurrentTime);
	AlternateKeymap = 0;
    }
    for (key = Scr->FuncKeyRoot.next; key != NULL; key = key->next)
    {
	if (key->keycode == Event.xkey.keycode &&
	    key->mods == modifier &&
	    (key->cont == Context || key->cont == C_NAME))
	{
	    /* weed out the functions that don't make sense to execute
	     * from a key press 
	     * TODO: add keyboard moving/resizing of windows.
	     */
	    if (key->func == F_MOVE || key->func == F_RESIZE)
		return;

	    if (key->cont != C_NAME)
	    {
		if (key->func == F_MENU) {
		    ButtonEvent = Event;
		    ButtonWindow = Tmp_win;
		    do_key_menu (key->menu, (Window) None);
		}
		else {
		    ExecuteFunction(key->func, key->action, Event.xany.window,
			Tmp_win, &Event, Context, FALSE);
		    if (!AlternateKeymap && !AlternateContext)
			XUngrabPointer(dpy, CurrentTime);
		}
		return;
	    }
	    else
	    {
		int matched = FALSE;
		len = strlen(key->win_name);

		/* try and match the name first */
		for (Tmp_win = Scr->FirstWindow; Tmp_win != NULL;
		    Tmp_win = Tmp_win->next)
		{
		    if (!strncmp(key->win_name, Tmp_win->name, len))
		    {
			matched = TRUE;
			ExecuteFunction(key->func, key->action, Tmp_win->frame,
			    Tmp_win, &Event, C_FRAME, FALSE);
			if (!AlternateKeymap && !AlternateContext)
			    XUngrabPointer(dpy, CurrentTime);
		    }
		}

		/* now try the res_name */
		if (!matched)
		    for (Tmp_win = Scr->FirstWindow; Tmp_win != NULL;
			 Tmp_win = Tmp_win->next)
		    {
			if (!strncmp(key->win_name, Tmp_win->class.res_name, len))
			{
			    matched = TRUE;
			    ExecuteFunction(key->func, key->action, Tmp_win->frame,
					    Tmp_win, &Event, C_FRAME, FALSE);
			    if (!AlternateKeymap && !AlternateContext)
				XUngrabPointer(dpy, CurrentTime);
			}
		    }

		/* now try the res_class */
		if (!matched)
		    for (Tmp_win = Scr->FirstWindow; Tmp_win != NULL;
			 Tmp_win = Tmp_win->next)
		    {
			if (!strncmp(key->win_name, Tmp_win->class.res_class, len))
			{
			    matched = TRUE;
			    ExecuteFunction(key->func, key->action, Tmp_win->frame,
					    Tmp_win, &Event, C_FRAME, FALSE);
			    if (!AlternateKeymap && !AlternateContext)
				XUngrabPointer(dpy, CurrentTime);
			}
		    }
		if (matched)
		    return;
	    }
	}
    }

    /* if we get here, no function key was bound to the key.  Send it
     * to the client if it was in a window we know about.
     */
    if (Tmp_win)
    {
        /* if (Tmp_win == Scr->currentvs->wsw->twm_win) */
	if (Tmp_win->wspmgr) {
	  WMgrHandleKeyPressEvent (Scr->currentvs, &Event);
	  return;
        }
        if (Tmp_win->icon && ((Event.xany.window == Tmp_win->icon->w) ||
	    (Event.xany.window == Tmp_win->frame) ||
	    (Event.xany.window == Tmp_win->title_w) ||
	    (Tmp_win->iconmanagerlist &&
	     (Event.xany.window == Tmp_win->iconmanagerlist->w))))
        {
            Event.xkey.window = Tmp_win->w;
            XSendEvent(dpy, Tmp_win->w, False, KeyPressMask, &Event);
        }
    }

}



static void free_window_names (TwmWindow *tmp,
			       Bool nukefull, Bool nukename, Bool nukeicon)
{
/*
 * XXX - are we sure that nobody ever sets these to another constant (check
 * twm windows)?
 */
    if ((tmp->name == tmp->full_name) && (tmp->name == tmp->icon_name)) {
	if (nukefull && nukename && nukeicon)
	    FreeWMPropertyString(tmp->name);
    } else
    if (tmp->name == tmp->full_name) {
	if (nukename && nukefull)
	    FreeWMPropertyString(tmp->name);
	if (nukeicon)
	    FreeWMPropertyString(tmp->icon_name);
    } else
    if (tmp->name == tmp->icon_name) {
	if (nukename && nukeicon)
	    FreeWMPropertyString(tmp->name);
	if (nukefull)
	    FreeWMPropertyString(tmp->full_name);
    } else
    if (tmp->icon_name == tmp->full_name) {
	if (nukeicon && nukefull)
	    FreeWMPropertyString(tmp->icon_name);
	if (nukename)
	    FreeWMPropertyString(tmp->name);
    } else {
	if (nukefull)
	    FreeWMPropertyString(tmp->full_name);
	if (nukename)
	    FreeWMPropertyString(tmp->name);
	if (nukeicon)
	    FreeWMPropertyString(tmp->icon_name);
    }
    return;
}



void free_cwins (TwmWindow *tmp)
{
    int i;
    TwmColormap *cmap;

    if (tmp->cmaps.number_cwins) {
	for (i = 0; i < tmp->cmaps.number_cwins; i++) {
	     if (--tmp->cmaps.cwins[i]->refcnt == 0) {
		cmap = tmp->cmaps.cwins[i]->colormap;
		if (--cmap->refcnt == 0) {
		    XDeleteContext(dpy, cmap->c, ColormapContext);
		    free((char *) cmap);
		}
		XDeleteContext(dpy, tmp->cmaps.cwins[i]->w, ColormapContext);
		free((char *) tmp->cmaps.cwins[i]);
	    }
	}
	free((char *) tmp->cmaps.cwins);
	if (tmp->cmaps.number_cwins > 1) {
	    free(tmp->cmaps.scoreboard);
	    tmp->cmaps.scoreboard = NULL;
	}
	tmp->cmaps.number_cwins = 0;
    }
}



/***********************************************************************
 *
 *  Procedure:
 *	HandlePropertyNotify - property notify event handler
 *
 ***********************************************************************
 */

void HandlePropertyNotify(void)
{
    unsigned char *prop = NULL;
    Atom actual = None;
    int actual_format;
    unsigned long nitems, bytesafter;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    Pixmap pm;
    int icon_change;
    XRectangle inc_rect;
    XRectangle logical_rect;

    unsigned char *gwkspc;

    /* watch for standard colormap changes */
    if (Event.xproperty.window == Scr->Root) {
	XStandardColormap *maps = NULL;
	int nmaps;

	if (Event.xproperty.atom == _XA_WM_CURRENTWORKSPACE) {
	    switch (Event.xproperty.state) {
		case PropertyNewValue:
		    if (XGetWindowProperty (dpy, Scr->Root, _XA_WM_CURRENTWORKSPACE,
				0L, 200L, False, XA_STRING, &actual, &actual_format,
				&nitems, &bytesafter, &prop) == Success) {
			if (nitems == 0) return;
			GotoWorkSpaceByName (Scr->vScreenList, (char*)prop);
			XFree ((char*) prop);
		    }
		    return;

		default:
		    return;
	    }
	}
	switch (Event.xproperty.state) {
	  case PropertyNewValue:
	    if (XGetRGBColormaps (dpy, Scr->Root, &maps, &nmaps, 
				  Event.xproperty.atom)) {
		/* if got one, then replace any existing entry */
		InsertRGBColormap (Event.xproperty.atom, maps, nmaps, True);
	    }
	    return;

	  case PropertyDelete:
	    RemoveRGBColormap (Event.xproperty.atom);
	    return;
	}
    }

    if (!Tmp_win) return;		/* unknown window */

#define MAX_NAME_LEN 200L		/* truncate to this many */
#define MAX_ICON_NAME_LEN 200L		/* ditto */

    switch (Event.xproperty.atom) {
      case XA_WM_NAME:
	prop = GetWMPropertyString(Tmp_win->w, XA_WM_NAME);
	if (prop == NULL) return;
#ifdef CLAUDE
	if (strstr (prop, " - Mozilla")) {
	  char *moz = strstr (prop, " - Mozilla");
	  *moz = '\0';
	}
#endif
	free_window_names (Tmp_win, True, True, False);

	Tmp_win->full_name = (char*) prop;
	Tmp_win->name = (char*) prop;
	Tmp_win->nameChanged = 1;
	XmbTextExtents(Scr->TitleBarFont.font_set,
		       Tmp_win->name, strlen (Tmp_win->name),
		       &inc_rect, &logical_rect);
	Tmp_win->name_width = logical_rect.width;

	SetupWindow (Tmp_win, Tmp_win->frame_x, Tmp_win->frame_y,
		     Tmp_win->frame_width, Tmp_win->frame_height, -1);

	if (Tmp_win->title_w) XClearArea(dpy, Tmp_win->title_w, 0,0,0,0, True);
	if (Scr->AutoOccupy) WmgrRedoOccupation (Tmp_win);

	/* Experimental, not yet working.
	{
	    ColorPair cp;
	    int f, b;

	    f = GetColorFromList (Scr->TitleForegroundL, Tmp_win->full_name,
							&Tmp_win->class, &cp.fore);
	    b = GetColorFromList (Scr->TitleBackgroundL, Tmp_win->full_name,
							&Tmp_win->class, &cp.back);
	    if (f || b) {
		if (Scr->use3Dtitles  && !Scr->BeNiceToColormap) GetShadeColors (&cp);
		Tmp_win->title = cp;
	    }
	    f = GetColorFromList (Scr->BorderColorL, Tmp_win->full_name,
						    &Tmp_win->class, &cp.fore);
	    b = GetColorFromList (Scr->BorderColorL, Tmp_win->full_name,
						    &Tmp_win->class, &cp.back);
	    if (f || b) {
		if (Scr->use3Dborders && !Scr->BeNiceToColormap) GetShadeColors (&cp);
		Tmp_win->borderC = cp;
	    }

	    f = GetColorFromList (Scr->BorderTileForegroundL, Tmp_win->full_name,
							     &Tmp_win->class, &cp.fore);
	    b = GetColorFromList (Scr->BorderTileBackgroundL, Tmp_win->full_name,
							     &Tmp_win->class, &cp.back);
	    if (f || b) {
		if (Scr->use3Dborders && !Scr->BeNiceToColormap) GetShadeColors (&cp);
		Tmp_win->border_tile = cp;
	    }
	}
	*/
	/*
	 * if the icon name is NoName, set the name of the icon to be
	 * the same as the window 
	 */
	if (Tmp_win->icon_name == NoName) {
	    Tmp_win->icon_name = Tmp_win->name;
	    RedoIcon();
	}
	break;

      case XA_WM_ICON_NAME:
	prop = GetWMPropertyString(Tmp_win->w, XA_WM_ICON_NAME);
	if (prop == NULL) return;
#ifdef CLAUDE
	if (strstr (prop, " - Mozilla")) {
	  char *moz = strstr (prop, " - Mozilla");
	  *moz = '\0';
	}
#endif
	icon_change = strcmp (Tmp_win->icon_name, (char*) prop);
	free_window_names (Tmp_win, False, False, True);
	Tmp_win->icon_name = (char*) prop;

	if (icon_change) {
	    RedoIcon();
	}
	break;

      case XA_WM_HINTS:
	if (Tmp_win->wmhints) XFree ((char *) Tmp_win->wmhints);
	Tmp_win->wmhints = XGetWMHints(dpy, Event.xany.window);

	if (Tmp_win->wmhints && (Tmp_win->wmhints->flags & WindowGroupHint)) {
	    Tmp_win->group = Tmp_win->wmhints->window_group;
	    if (Tmp_win->group && !GetTwmWindow(Tmp_win->group))
		Tmp_win->group = 0;	/* see comment in AddWindow() */
	}

	if (!Tmp_win->forced && Tmp_win->wmhints &&
	    Tmp_win->wmhints->flags & IconWindowHint) {
	    if (Tmp_win->icon && Tmp_win->icon->w) {
	    	int icon_x, icon_y;

		/*
		 * There's already an icon window.
		 * Try to find out where it is; if we succeed, move the new
		 * window to where the old one is.
		 */
		if (XGetGeometry (dpy, Tmp_win->icon->w, &JunkRoot, &icon_x,
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
		if (Tmp_win->isicon)
		    XMapWindow(dpy, Tmp_win->wmhints->icon_window);

		/*
		 * Now, if the old window isn't ours, unmap it, otherwise
		 * just get rid of it completely.
		 */
		if (Tmp_win->icon_not_ours) {
		    if (Tmp_win->icon->w != Tmp_win->wmhints->icon_window)
			XUnmapWindow(dpy, Tmp_win->icon->w);
		} else
		    XDestroyWindow(dpy, Tmp_win->icon->w);

		/*
		 * The new icon window isn't our window, so note that fact
		 * so that we don't treat it as ours.
		 */
		Tmp_win->icon_not_ours = TRUE;

		/*
		 * Now make the new window the icon window for this window,
		 * and set it up to work as such (select for key presses
		 * and button presses/releases, set up the contexts for it,
		 * and define the cursor for it).
		 */
		Tmp_win->icon->w = Tmp_win->wmhints->icon_window;
		XSelectInput (dpy, Tmp_win->icon->w,
		  KeyPressMask | ButtonPressMask | ButtonReleaseMask);
		XSaveContext(dpy, Tmp_win->icon->w, TwmContext, (XPointer)Tmp_win);
		XSaveContext(dpy, Tmp_win->icon->w, ScreenContext, (XPointer)Scr);
		XDefineCursor(dpy, Tmp_win->icon->w, Scr->IconCursor);
	    }
	}

	if (Tmp_win->icon && Tmp_win->icon->w && !Tmp_win->forced && Tmp_win->wmhints &&
	    (Tmp_win->wmhints->flags & IconPixmapHint)) {
	    int x;
	    if (!XGetGeometry (dpy, Tmp_win->wmhints->icon_pixmap, &JunkRoot,
			       &JunkX, &JunkY, (unsigned int *)&Tmp_win->icon->width, 
			       (unsigned int *)&Tmp_win->icon->height, &JunkBW,
				&JunkDepth)) {
		return;
	    }

	    pm = XCreatePixmap (dpy, Scr->Root, Tmp_win->icon->width,
				Tmp_win->icon->height, Scr->d_depth);

	    FB(Tmp_win->icon->iconc.fore, Tmp_win->icon->iconc.back);

	    if (JunkDepth == Scr->d_depth)
		XCopyArea  (dpy, Tmp_win->wmhints->icon_pixmap, pm, Scr->NormalGC,
			0,0, Tmp_win->icon->width, Tmp_win->icon->height, 0, 0);
	    else
		XCopyPlane(dpy, Tmp_win->wmhints->icon_pixmap, pm, Scr->NormalGC,
			0,0, Tmp_win->icon->width, Tmp_win->icon->height, 0, 0, 1 );

	    if (Tmp_win->icon->image) {
		if (Tmp_win->icon->image->pixmap)
		    XFreePixmap (dpy, Tmp_win->icon->image->pixmap);
		Tmp_win->icon->image->pixmap = pm;
		Tmp_win->icon->image->width  = Tmp_win->icon->width;
		Tmp_win->icon->image->height = Tmp_win->icon->height;
		Tmp_win->icon->image->mask   = None;
		Tmp_win->icon->image->next   = None;
	    }

	    valuemask = CWBackPixmap;
	    attributes.background_pixmap = pm;

	    if (Tmp_win->icon->bm_w)
		XDestroyWindow(dpy, Tmp_win->icon->bm_w);

	    x = GetIconOffset (Tmp_win->icon);
	    Tmp_win->icon->bm_w =
	      XCreateWindow (dpy, Tmp_win->icon->w, x, 0,
			     (unsigned int) Tmp_win->icon->width,
			     (unsigned int) Tmp_win->icon->height,
			     (unsigned int) 0, Scr->d_depth,
			     (unsigned int) CopyFromParent, Scr->d_visual,
			     valuemask, &attributes);

	    if (! (Tmp_win->wmhints->flags & IconMaskHint)) {
		XRectangle rect;

		rect.x      = x;
		rect.y      = 0;
		rect.width  = Tmp_win->icon->width;
		rect.height = Tmp_win->icon->height;
		XShapeCombineRectangles (dpy, Tmp_win->icon->w, ShapeBounding, 0,
					0, &rect, 1, ShapeUnion, 0);
	    }
	    XMapSubwindows (dpy, Tmp_win->icon->w);
	    RedoIconName();
	}
	if (Tmp_win->icon && Tmp_win->icon->w && !Tmp_win->forced && Tmp_win->wmhints &&
	    (Tmp_win->wmhints->flags & IconMaskHint)) {
	    int x;
	    Pixmap mask;
	    GC gc;

	    if (!XGetGeometry (dpy, Tmp_win->wmhints->icon_mask, &JunkRoot,
			       &JunkX, &JunkY, &JunkWidth, &JunkHeight, &JunkBW,
			       &JunkDepth)) {
		return;
	    }
	    if (JunkDepth != 1) return;

	    mask = XCreatePixmap (dpy, Scr->Root, JunkWidth, JunkHeight, 1);
	    if (!mask) return;
	    gc = XCreateGC (dpy, mask, 0, NULL);
	    if (!gc) return;
	    XCopyArea (dpy, Tmp_win->wmhints->icon_mask, mask, gc,
		       0, 0, JunkWidth, JunkHeight, 0, 0);
	    XFreeGC (dpy, gc);
	    x = GetIconOffset (Tmp_win->icon);
	    XShapeCombineMask (dpy, Tmp_win->icon->bm_w, ShapeBounding, 0, 0, mask, ShapeSet);
	    XShapeCombineMask (dpy, Tmp_win->icon->w,    ShapeBounding, x, 0, mask, ShapeSet);
	    if (Tmp_win->icon->image) {
		if (Tmp_win->icon->image->mask) XFreePixmap (dpy, Tmp_win->icon->image->mask);
		Tmp_win->icon->image->mask = mask;
		RedoIconName ();
	    }
	}
	break;

      case XA_WM_NORMAL_HINTS:
      {
	GetWindowSizeHints (Tmp_win);
	break;
      }
      default:
	if (Event.xproperty.atom == _XA_WM_COLORMAP_WINDOWS) {
	    FetchWmColormapWindows (Tmp_win);	/* frees old data */
	    break;
	} else if (Event.xproperty.atom == _XA_WM_PROTOCOLS) {
	    FetchWmProtocols (Tmp_win);
	    break;
	} else if (Event.xproperty.atom == _XA_WM_OCCUPATION) {
	  if (XGetWindowProperty (dpy, Tmp_win->w, Event.xproperty.atom, 0L, MAX_NAME_LEN, False,
				  XA_STRING, &actual, &actual_format, &nitems,
				  &bytesafter, &prop) != Success ||
	      actual == None) return;
	  ChangeOccupation (Tmp_win, GetMaskFromProperty (prop, nitems));
	  XFree ((char *)prop);
	}
#ifdef GNOME
	else if (Event.xproperty.atom == _XA_WIN_WORKSPACE){
	  if(XGetWindowProperty(dpy, Tmp_win->w, Event.xproperty.atom, 0L, 32, False,
				XA_CARDINAL, &actual, &actual_format, &nitems, &bytesafter,
				&gwkspc) != Success || actual == None) return;
	  ChangeOccupation (Tmp_win, 1 << (int)(*gwkspc));
	  XFree ((char *)gwkspc);
	}
#endif /* GNOME */
	break;
    }
}



static void RedoIcon(void)
{
    Icon *icon;
    char *pattern;

    if (Tmp_win->icon_not_ours) {
	RedoIconName ();
	return;
    }
    icon = (Icon*) 0;
    if ((pattern = LookPatternInNameList (Scr->IconNames, Tmp_win->icon_name))) {
	icon = (Icon*) LookInNameList (Tmp_win->iconslist, pattern);
    }
    else
    if ((pattern = LookPatternInNameList (Scr->IconNames, Tmp_win->full_name))) {
	icon = (Icon*) LookInNameList (Tmp_win->iconslist, pattern);
    }
    else
    if ((pattern = LookPatternInList (Scr->IconNames, Tmp_win->full_name, &Tmp_win->class))) {
	icon = (Icon*) LookInNameList (Tmp_win->iconslist, pattern);
    }
    if (pattern == NULL) {
	RedoIconName ();
	return;
    }
    if (icon != NULL) {
	if (Tmp_win->icon == icon) {
	    RedoIconName ();
	    return;
	}
	if (Tmp_win->icon_on && visible (Tmp_win)) {
	    IconDown (Tmp_win);
	    if (Tmp_win->icon && Tmp_win->icon->w) XUnmapWindow (dpy, Tmp_win->icon->w);
	    Tmp_win->icon = icon;
	    IconUp (Tmp_win);
	    XMapRaised (dpy, Tmp_win->icon->w);
        }
	else {
	    Tmp_win->icon = icon;
	}
	RedoIconName ();
    }
    else {
	if (Tmp_win->icon_on && visible (Tmp_win)) {
	    IconDown (Tmp_win);
	    if (Tmp_win->icon && Tmp_win->icon->w) XUnmapWindow (dpy, Tmp_win->icon->w);
	    CreateIconWindow (Tmp_win, -100, -100);
	    XMapRaised (dpy, Tmp_win->icon->w);
	}
	else {
	    Tmp_win->icon = (Icon*) 0;
	    WMapUpdateIconName (Tmp_win);
	}
	RedoIconName ();
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	RedoIconName - procedure to re-position the icon window and name
 *
 ***********************************************************************
 */

void RedoIconName(void)
{
    int x;
    XRectangle ink_rect;
    XRectangle logical_rect;

    if (Scr->NoIconTitlebar || 
	LookInNameList (Scr->NoIconTitle, Tmp_win->icon_name) ||
	LookInList (Scr->NoIconTitle, Tmp_win->full_name, &Tmp_win->class)) goto wmapupd;
    if (Tmp_win->iconmanagerlist)
    {
	/* let the expose event cause the repaint */
	XClearArea(dpy, Tmp_win->iconmanagerlist->w, 0,0,0,0, True);

	if (Scr->SortIconMgr)
	    SortIconManager(Tmp_win->iconmanagerlist->iconmgr);
    }

    if (!Tmp_win->icon  || !Tmp_win->icon->w) goto wmapupd;

    if (Tmp_win->icon_not_ours) goto wmapupd;

    XmbTextExtents(Scr->IconFont.font_set,
		   Tmp_win->icon_name, strlen(Tmp_win->icon_name),
		   &ink_rect, &logical_rect);
    Tmp_win->icon->w_width = logical_rect.width;
    Tmp_win->icon->w_width += 2 * Scr->IconManagerShadowDepth + 6;
    if (Tmp_win->icon->w_width > Scr->MaxIconTitleWidth)
	Tmp_win->icon->w_width = Scr->MaxIconTitleWidth;

    if (Tmp_win->icon->w_width < Tmp_win->icon->width)
    {
	Tmp_win->icon->x = (Tmp_win->icon->width - Tmp_win->icon->w_width)/2;
	Tmp_win->icon->x += Scr->IconManagerShadowDepth + 3;
	Tmp_win->icon->w_width = Tmp_win->icon->width;
    }
    else
    {
	Tmp_win->icon->x = Scr->IconManagerShadowDepth + 3;
    }

    x = GetIconOffset (Tmp_win->icon);
    Tmp_win->icon->y = Tmp_win->icon->height + Scr->IconFont.height +
				Scr->IconManagerShadowDepth;
    Tmp_win->icon->w_height = Tmp_win->icon->height + Scr->IconFont.height +
				2 * Scr->IconManagerShadowDepth + 6;

    XResizeWindow(dpy, Tmp_win->icon->w, Tmp_win->icon->w_width, Tmp_win->icon->w_height);
    if (Tmp_win->icon->bm_w)
    {
	XRectangle rect;

	XMoveWindow(dpy, Tmp_win->icon->bm_w, x, 0);
	XMapWindow(dpy, Tmp_win->icon->bm_w);
	if (Tmp_win->icon->image && Tmp_win->icon->image->mask) {
	    XShapeCombineMask(dpy, Tmp_win->icon->bm_w, ShapeBounding, 0, 0,
				Tmp_win->icon->image->mask, ShapeSet);
	    XShapeCombineMask(dpy, Tmp_win->icon->w, ShapeBounding, x, 0,
				Tmp_win->icon->image->mask, ShapeSet);
	} else if (Tmp_win->icon->has_title) {
	    rect.x      = x;
	    rect.y      = 0;
	    rect.width  = Tmp_win->icon->width;
	    rect.height = Tmp_win->icon->height;
	    XShapeCombineRectangles (dpy, Tmp_win->icon->w, ShapeBounding,
			0, 0, &rect, 1, ShapeSet, 0);
	}
	if (Tmp_win->icon->has_title) {
	    if (Scr->ShrinkIconTitles && Tmp_win->icon->title_shrunk) {
		rect.x      = x;
		rect.y      = Tmp_win->icon->height;
		rect.width  = Tmp_win->icon->width;
		rect.height = Tmp_win->icon->w_height - Tmp_win->icon->height;
	    } else {
		rect.x      = 0;
		rect.y      = Tmp_win->icon->height;
		rect.width  = Tmp_win->icon->w_width;
		rect.height = Tmp_win->icon->w_height - Tmp_win->icon->height;
	    }
	    XShapeCombineRectangles (dpy,  Tmp_win->icon->w, ShapeBounding, 0,
				     0, &rect, 1, ShapeUnion, 0);
	}
    }
    if (Scr->ShrinkIconTitles &&
	Tmp_win->icon->title_shrunk &&
	Tmp_win->icon_on && (visible (Tmp_win))) {
	IconDown (Tmp_win);
	IconUp (Tmp_win);
    }
    if (Tmp_win->isicon)
    {
	XClearArea(dpy, Tmp_win->icon->w, 0, 0, 0, 0, True);
    }
wmapupd:
    WMapUpdateIconName (Tmp_win);
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleClientMessage - client message event handler
 *
 ***********************************************************************
 */

void HandleClientMessage(void)
{
    TwmWindow *twm_win;
    int i;

    if (Event.xclient.message_type == _XA_WM_CHANGE_STATE) {
	if (Tmp_win != NULL) {
	    if (Event.xclient.data.l[0] == IconicState && !Tmp_win->isicon) {
		XEvent button;
		XQueryPointer (dpy, Scr->Root, &JunkRoot, &JunkChild,
			       &(button.xmotion.x_root),
			       &(button.xmotion.y_root),
			       &JunkX, &JunkY, &JunkMask);

		ExecuteFunction (F_ICONIFY, NULLSTR, Event.xany.window,
				 Tmp_win, &button, FRAME, FALSE);
		XUngrabPointer (dpy, CurrentTime);
	    }
	}
	return;
    }
#ifdef GNOME
    /* 6/19/1999 nhd for GNOME compliance */
    if (Event.xclient.message_type == _XA_WIN_WORKSPACE) {
      /* XXXXX
	 supposedly works with a single screen, but is less certain with
	 multiple screens */
      GotoWorkSpaceByNumber (Scr->currentvs, Event.xclient.data.l[0]);
      return;
    }
    if (Event.xclient.message_type == _XA_WIN_STATE) {
      unsigned long new_stuff = (unsigned long) Event.xclient.data.l [1];
      unsigned long old_stuff = (unsigned long) Event.xclient.data.l [0];
      Window	      tmp_win = Event.xclient.window;
      for (twm_win = Scr->FirstWindow; twm_win != NULL; twm_win = twm_win->next)
	if (twm_win->w == tmp_win) break;
      if (twm_win == NULL) return;
      for (i = 1; i < (1 << 10); i <<= 1){
	switch (old_stuff & i) {
	  case WIN_STATE_STICKY: /* sticky */
	    if (new_stuff & i) OccupyAll (twm_win);
	    else ChangeOccupation (twm_win, (1<<(Scr->currentvs->wsw->currentwspc->number)));
	    break;
	  case WIN_STATE_MINIMIZED: /* minimized - reserved */
	    break;
	  case WIN_STATE_MAXIMIZED_VERT: /* window in maximized V state */
	    break;
	  case WIN_STATE_MAXIMIZED_HORIZ: /* maximized horizontally */
	    break;
	  case WIN_STATE_HIDDEN: /* hidden - what does this mean?? */
	    break;
	  case WIN_STATE_SHADED: /* shaded (squeezed) */
	    Squeeze (twm_win);
	    break;
	  case WIN_STATE_HID_WORKSPACE: /* not on this workspace */
	    break;
	  case WIN_STATE_HID_TRANSIENT: /* owner of transient hidden ? */
	    break;
	  case WIN_STATE_FIXED_POSITION: /* position fixed, don't move */
	    break;
	  case WIN_STATE_ARRANGE_IGNORE: /* ignore when auto-arranging */
	    break;
	}
      }
    }
#endif /* GNOME */
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleExpose - expose event handler
 *
 ***********************************************************************
 */

static void flush_expose(Window w);

void HandleExpose(void)
{
    MenuRoot *tmp;
    virtualScreen *vs;

    if (XFindContext(dpy, Event.xany.window, MenuContext, (XPointer *)&tmp) == 0)
    {
	PaintMenu(tmp, &Event);
	return;
    }

    if (Event.xexpose.count != 0)
	return;

    if (Event.xany.window == Scr->InfoWindow && InfoLines)
    {
	int i;
	int height;

	Draw3DBorder (Scr->InfoWindow, 0, 0,
		InfoWidth, InfoHeight, 2, Scr->DefaultC, off, True, False);

	FB(Scr->DefaultC.fore, Scr->DefaultC.back);

	height = Scr->DefaultFont.height+2;
	for (i = 0; i < InfoLines; i++)
	{
	    XmbDrawString(dpy, Scr->InfoWindow, Scr->DefaultFont.font_set,
			  Scr->NormalGC, 5,
			  (i*height) + Scr->DefaultFont.y + 5,
			  Info[i], strlen(Info[i]));
	}
	flush_expose (Event.xany.window);
    }
    else if (Tmp_win != NULL)
    {
	if (Scr->use3Dborders && (Event.xany.window == Tmp_win->frame)) {
	    PaintBorders (Tmp_win, ((Tmp_win == Scr->Focus) ? True : False));
	    flush_expose (Event.xany.window);
	    return;
	}
	else
	if (Event.xany.window == Tmp_win->title_w)
	{
	    PaintTitle (Tmp_win);
	    flush_expose (Event.xany.window);
	    return;
	}
	else if (Tmp_win->icon && (Event.xany.window == Tmp_win->icon->w) &&
		! Scr->NoIconTitlebar &&
		! LookInList (Scr->NoIconTitle, Tmp_win->full_name, &Tmp_win->class))
	{
	    PaintIcon (Tmp_win);
	    flush_expose (Event.xany.window);
	    return;
	} else if (Tmp_win->titlebuttons) {
	    int i;
	    TBWindow *tbw;
	    Window w = Event.xany.window;
	    int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

	    for (i = 0, tbw = Tmp_win->titlebuttons; i < nb; i++, tbw++) {
		if (w == tbw->window) {
		    PaintTitleButton (Tmp_win, tbw);
		    flush_expose (tbw->window);
                    return;
                }
            }
	}
	for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
	  if (Tmp_win == vs->wsw->twm_win) {
	    WMgrHandleExposeEvent (vs, &Event);
	    flush_expose (Event.xany.window);
	    return;
	  }
	}
	if (Tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win) {
	    PaintOccupyWindow ();
	    flush_expose (Event.xany.window);
	    return;
	} else 	if (Tmp_win->iconmanagerlist) {
	    WList *iconmanagerlist = Tmp_win->iconmanagerlist;

	    if (Event.xany.window == iconmanagerlist->w)
	    {
		int offs;

		DrawIconManagerBorder(iconmanagerlist, True);

		FB(iconmanagerlist->cp.fore, iconmanagerlist->cp.back);
		offs = Scr->use3Diconmanagers ? Scr->IconManagerShadowDepth : 2;
		if (Scr->use3Diconmanagers && (Scr->Monochrome != COLOR))
		    XmbDrawImageString(dpy, Event.xany.window,
				       Scr->IconManagerFont.font_set,
				       Scr->NormalGC, 
				       iconmgr_textx,
				       Scr->IconManagerFont.y + offs + 2,
				       Tmp_win->icon_name,
				       strlen(Tmp_win->icon_name));
		else
		    XmbDrawString(dpy, Event.xany.window,
				  Scr->IconManagerFont.font_set, Scr->NormalGC,
				  iconmgr_textx,
				  Scr->IconManagerFont.y + offs + 2,
				  Tmp_win->icon_name,
				  strlen(Tmp_win->icon_name));
		flush_expose (Event.xany.window);
		return;
	    }
	    if (Event.xany.window == iconmanagerlist->icon)
	    {
		if (Scr->use3Diconmanagers && iconmanagerlist->iconifypm) {
		    XCopyArea(dpy, iconmanagerlist->iconifypm,
				iconmanagerlist->icon,
				Scr->NormalGC, 0, 0,
				iconifybox_width, iconifybox_height, 0, 0);
		}
		else {
		    FB(iconmanagerlist->cp.fore, iconmanagerlist->cp.back);
		    XCopyPlane(dpy, Scr->siconifyPm, iconmanagerlist->icon,
			    Scr->NormalGC, 0,0,
			    iconifybox_width, iconifybox_height, 0, 0, 1);
		}
		flush_expose (Event.xany.window);
		return;
	    }
	}
    }
}



static void remove_window_from_ring (TwmWindow *tmp)
{
    TwmWindow *prev = tmp->ring.prev, *next = tmp->ring.next;

    if (enter_win == tmp) {
	enter_flag = FALSE;
	enter_win = NULL;
    }
    if (raise_win == Tmp_win) raise_win = NULL;
    if (leave_win == tmp) {
	leave_flag = FALSE;
	leave_win = NULL;
    }
    if (lower_win == Tmp_win) lower_win = NULL;

    /*
     * 1. Unlink window
     * 2. If window was only thing in ring, null out ring
     * 3. If window was ring leader, set to next (or null)
     */
    if (prev) prev->ring.next = next;
    if (next) next->ring.prev = prev;
    if (Scr->Ring == tmp) 
      Scr->Ring = (next != tmp ? next : (TwmWindow *) NULL);

    if (!Scr->Ring || Scr->RingLeader == tmp) Scr->RingLeader = Scr->Ring;
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleDestroyNotify - DestroyNotify event handler
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

    if (Tmp_win == NULL)
	return;

    RemoveWindowFromRegion (Tmp_win);
#ifdef GNOME
    GnomeDeleteClientWindow (Tmp_win); /* Fix the gnome client list */
#endif /* GNOME */
    if (Tmp_win == Scr->Focus)
    {
	Scr->Focus = (TwmWindow*) NULL;
	FocusOnRoot();
    }
    if (Scr->SaveWorkspaceFocus) {
	struct WorkSpace *ws;
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    if (ws->save_focus == Tmp_win)
		ws->save_focus = NULL;
	}
    }
    XDeleteContext(dpy, Tmp_win->w, TwmContext);
    XDeleteContext(dpy, Tmp_win->w, ScreenContext);
    XDeleteContext(dpy, Tmp_win->frame, TwmContext);
    XDeleteContext(dpy, Tmp_win->frame, ScreenContext);
    if (Tmp_win->icon && Tmp_win->icon->w)
    {
	XDeleteContext(dpy, Tmp_win->icon->w, TwmContext);
	XDeleteContext(dpy, Tmp_win->icon->w, ScreenContext);
    }
    if (Tmp_win->title_height) {
	int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

	XDeleteContext(dpy, Tmp_win->title_w, TwmContext);
	XDeleteContext(dpy, Tmp_win->title_w, ScreenContext);
	if (Tmp_win->hilite_wl) {
	    XDeleteContext(dpy, Tmp_win->hilite_wl, TwmContext);
	    XDeleteContext(dpy, Tmp_win->hilite_wl, ScreenContext);
	}
	if (Tmp_win->hilite_wr) {
	    XDeleteContext(dpy, Tmp_win->hilite_wr, TwmContext);
	    XDeleteContext(dpy, Tmp_win->hilite_wr, ScreenContext);
	}
	if (Tmp_win->lolite_wr) {
	    XDeleteContext(dpy, Tmp_win->lolite_wr, TwmContext);
	    XDeleteContext(dpy, Tmp_win->lolite_wr, ScreenContext);
	}
	if (Tmp_win->lolite_wl) {
	    XDeleteContext(dpy, Tmp_win->lolite_wl, TwmContext);
	    XDeleteContext(dpy, Tmp_win->lolite_wl, ScreenContext);
	}
	if (Tmp_win->titlebuttons) {
	    int i;

	    for (i = 0; i < nb; i++) {
		XDeleteContext (dpy, Tmp_win->titlebuttons[i].window,
				TwmContext);
		XDeleteContext (dpy, Tmp_win->titlebuttons[i].window,
				ScreenContext);
	    }
        }
	/*
	 * The hilite_wl etc windows don't need to be XDestroyWindow()ed
	 * since that will happen when the parent is destroyed (??)
	 */
    }

    if (Scr->cmapInfo.cmaps == &Tmp_win->cmaps) {
	InstallColormaps(DestroyNotify, &Scr->RootColormaps);
    }

    /*
     * TwmWindows contain the following pointers
     * 
     *     1.  full_name
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
    WMapDestroyWindow (Tmp_win);
    if (Tmp_win->gray) XFreePixmap (dpy, Tmp_win->gray);

    /*
     * According to the manual page, the following destroys all child windows
     * of the frame too, which is most of the windows we're concerned with, so
     * anything related to them must be done before here.
     * Icons are not child windows.
     */
    XDestroyWindow(dpy, Tmp_win->frame);
    if (Tmp_win->icon) {
	if (Tmp_win->icon->w && !Tmp_win->icon_not_ours) {
	    XDestroyWindow(dpy, Tmp_win->icon->w);
	    IconDown (Tmp_win);
	}
	free (Tmp_win->icon);
	Tmp_win->icon = NULL;
    }
    Tmp_win->occupation = 0;
    RemoveIconManager(Tmp_win);					/* 7 */
    if (Scr->FirstWindow == Tmp_win)
	Scr->FirstWindow = Tmp_win->next;
    if (Tmp_win->prev != NULL)
	Tmp_win->prev->next = Tmp_win->next;
    if (Tmp_win->next != NULL)
	Tmp_win->next->prev = Tmp_win->prev;
    if (Tmp_win->auto_raise) Scr->NumAutoRaises--;
    if (Tmp_win->auto_lower) Scr->NumAutoLowers--;
    if (Tmp_win->frame == lowerontop) lowerontop = -1;

    free_window_names (Tmp_win, True, True, True);		/* 1, 2, 3 */
    if (Tmp_win->wmhints)					/* 4 */
      XFree ((char *)Tmp_win->wmhints);
    if (Tmp_win->class.res_name && Tmp_win->class.res_name != NoName)  /* 5 */
      XFree ((char *)Tmp_win->class.res_name);
    if (Tmp_win->class.res_class && Tmp_win->class.res_class != NoName) /* 6 */
      XFree ((char *)Tmp_win->class.res_class);
    free_cwins (Tmp_win);					/* 9 */
    if (Tmp_win->titlebuttons) { 				/* 10 */ 
	free(Tmp_win->titlebuttons);
	Tmp_win->titlebuttons = NULL;
    }
    
    remove_window_from_ring (Tmp_win);				/* 11 */
    if (Tmp_win->squeeze_info_copied) { 			/* 12 */
	free(Tmp_win->squeeze_info);
	Tmp_win->squeeze_info = NULL;
    }
    DeleteHighlightWindows(Tmp_win);				/* 13 */
    DeleteIconsList (Tmp_win);					/* 14 */

    free((char *)Tmp_win);
    Tmp_win = NULL;

    if (Scr->ClickToFocus || Scr->SloppyFocus)
	set_last_window (Scr->currentvs->wsw->currentwspc);
}



void HandleCreateNotify(void)
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "CreateNotify w = 0x%x\n", Event.xcreatewindow.window);
    fflush(stderr);
    XBell(dpy, 0);
    XSync(dpy, 0);
#endif
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleMapRequest - MapRequest event handler
 *
 ***********************************************************************
 */

void HandleMapRequest(void)
{
    int zoom_save;

    Event.xany.window = Event.xmaprequest.window;
    Tmp_win = GetTwmWindow(Event.xany.window);

    /* If the window has never been mapped before ... */
    if (Tmp_win == NULL)
    {
	/* Add decorations. */
	Tmp_win = AddWindow(Event.xany.window, FALSE, (IconMgr *) NULL);
	if (Tmp_win == NULL) return;
#ifdef GNOME
	GnomeAddClientWindow (Tmp_win); /* add the new window to the gnome client list */
#endif /* GNOME */
    }
    else
    {
	/*
	 * If the window has been unmapped by the client, it won't be listed
	 * in the icon manager.  Add it again, if requested.
	 */
	if (Tmp_win->iconmanagerlist == NULL)
	    (void) AddIconManager (Tmp_win);
    }

    if (Tmp_win->iconmgr) return;
    if (Tmp_win->squeezed) return;

    if (Scr->WindowMask) XRaiseWindow (dpy, Scr->WindowMask);

    /* If it's not merely iconified, and we have hints, use them. */
    if (! Tmp_win->isicon)
    {
	int state;
	Window icon;

	state = NormalState;
	/* use WM_STATE if enabled */
	if (!(RestartPreviousState && GetWMState(Tmp_win->w, &state, &icon) &&
	      (state == NormalState || state == IconicState || state == InactiveState))) {
	    if (Tmp_win->wmhints && (Tmp_win->wmhints->flags & StateHint))
		state = Tmp_win->wmhints->initial_state;
	}
	switch (state) 
	{
	    case DontCareState:
	    case NormalState:
	    case ZoomState:
		if (Tmp_win->StartSqueezed)
		    Squeeze (Tmp_win);
		else
		    XMapWindow(dpy, Tmp_win->w);
		XMapWindow(dpy, Tmp_win->frame);
		SetMapStateProp(Tmp_win, NormalState);
		SetRaiseWindow (Tmp_win);
		Tmp_win->mapped = TRUE;
		if (Scr->ClickToFocus &&
		    Tmp_win->wmhints  &&
		    Tmp_win->wmhints->input) SetFocus (Tmp_win, CurrentTime);
		    /* kai */
		if (Scr->AutoFocusToTransients &&
		    Tmp_win->transient &&
		    Tmp_win->wmhints   &&
		    Tmp_win->wmhints->input) SetFocus (Tmp_win, CurrentTime);
		break;

	    case InactiveState:
		Tmp_win->mapped = TRUE;
		if (Tmp_win->UnmapByMovingFarAway) {
		    XMoveWindow (dpy, Tmp_win->frame, Scr->rootw + 1, Scr->rooth + 1);
		    XMapWindow  (dpy, Tmp_win->w);
		    XMapWindow  (dpy, Tmp_win->frame);
		}
		if (Tmp_win->StartSqueezed) Squeeze (Tmp_win);
		break;

	    case IconicState:
		zoom_save = Scr->DoZoom;
		Scr->DoZoom = FALSE;
		Iconify(Tmp_win, -100, -100);
		Scr->DoZoom = zoom_save;
		break;
	}
    }
    /* If no hints, or currently an icon, just "deiconify" */
    else
    {
      if (1/*OCCUPY (Tmp_win, Scr->workSpaceMgr.activeWSPC)*/) {
	if (Tmp_win->StartSqueezed) Squeeze (Tmp_win);
	DeIconify(Tmp_win);
	SetRaiseWindow (Tmp_win);
      }
      else {
	Tmp_win->mapped = TRUE;
      }
    }
    if (Tmp_win->mapped) WMapMapWindow (Tmp_win);
    MaybeAnimate = True;
}



void SimulateMapRequest (Window w)
{
    Event.xmaprequest.window = w;
    HandleMapRequest ();
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleMapNotify - MapNotify event handler
 *
 ***********************************************************************
 */

void HandleMapNotify(void)
{
    if (Tmp_win == NULL)
	return;

    /*
     * Need to do the grab to avoid race condition of having server send
     * MapNotify to client before the frame gets mapped; this is bad because
     * the client would think that the window has a chance of being viewable
     * when it really isn't.
     */

    XGrabServer (dpy);
    if (Tmp_win->icon && Tmp_win->icon->w) XUnmapWindow(dpy, Tmp_win->icon->w);
    if (Tmp_win->title_w) XMapSubwindows(dpy, Tmp_win->title_w);
    XMapSubwindows(dpy, Tmp_win->frame);
    if (Scr->Focus != Tmp_win && Tmp_win->hilite_wl) XUnmapWindow(dpy, Tmp_win->hilite_wl);
    if (Scr->Focus != Tmp_win && Tmp_win->hilite_wr) XUnmapWindow(dpy, Tmp_win->hilite_wr);
    if (Scr->Focus == Tmp_win && Tmp_win->lolite_wl) XUnmapWindow(dpy, Tmp_win->lolite_wl);
    if (Scr->Focus == Tmp_win && Tmp_win->lolite_wr) XUnmapWindow(dpy, Tmp_win->lolite_wr);

    XMapWindow(dpy, Tmp_win->frame);
    XUngrabServer (dpy);
    XFlush (dpy);
    Tmp_win->mapped = TRUE;
    Tmp_win->isicon = FALSE;
    Tmp_win->icon_on = FALSE;
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleUnmapNotify - UnmapNotify event handler
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
    if (Tmp_win == NULL)
    {
	Event.xany.window = Event.xunmap.window;
	Tmp_win = GetTwmWindow(Event.xany.window);
    }

    if (Tmp_win == NULL || Event.xunmap.window == Tmp_win->frame ||
	(Tmp_win->icon && Event.xunmap.window == Tmp_win->icon->w) ||
	(!Tmp_win->mapped && !Tmp_win->isicon))
	return;
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
    XDeleteProperty (dpy, Tmp_win->w, _XA_WM_OCCUPATION);
*/
    XGrabServer (dpy);
    if (XTranslateCoordinates (dpy, Event.xunmap.window, Tmp_win->attr.root,
			       0, 0, &dstx, &dsty, &dumwin)) {
	XEvent ev;
	Bool reparented = XCheckTypedWindowEvent (dpy, Event.xunmap.window, 
						  ReparentNotify, &ev);
	SetMapStateProp (Tmp_win, WithdrawnState);
	if (reparented) {
	    if (Tmp_win->old_bw) XSetWindowBorderWidth (dpy,
							Event.xunmap.window, 
							Tmp_win->old_bw);
	    if (Tmp_win->wmhints && (Tmp_win->wmhints->flags & IconWindowHint))
	      XUnmapWindow (dpy, Tmp_win->wmhints->icon_window);
	} else {
	    XReparentWindow (dpy, Event.xunmap.window, Tmp_win->attr.root,
			     dstx, dsty);
	    RestoreWithdrawnLocation (Tmp_win);
	}
	XRemoveFromSaveSet (dpy, Event.xunmap.window);
	XSelectInput (dpy, Event.xunmap.window, NoEventMask);
	HandleDestroyNotify ();		/* do not need to mash event before */
    } /* else window no longer exists and we'll get a destroy notify */
    XUngrabServer (dpy);
    XFlush (dpy);
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleMotionNotify - MotionNotify event handler
 *
 ***********************************************************************
 */

void HandleMotionNotify(void)
{
    if (ResizeWindow != (Window) 0)
    {
	XQueryPointer( dpy, Event.xany.window,
	    &(Event.xmotion.root), &JunkChild,
	    &(Event.xmotion.x_root), &(Event.xmotion.y_root),
	    &(Event.xmotion.x), &(Event.xmotion.y),
	    &JunkMask);

	FixRootEvent (&Event);
	/* Set WindowMoved appropriately so that f.deltastop will
	   work with resize as well as move. */
	if (abs (Event.xmotion.x - ResizeOrigX) >= Scr->MoveDelta
	    || abs (Event.xmotion.y - ResizeOrigY) >= Scr->MoveDelta)
	  WindowMoved = TRUE;

	Tmp_win = GetTwmWindow(ResizeWindow);
	if (Tmp_win && Tmp_win->winbox) {
	    XTranslateCoordinates (dpy, Scr->Root, Tmp_win->winbox->window,
		Event.xmotion.x_root, Event.xmotion.y_root,
		&(Event.xmotion.x_root), &(Event.xmotion.y_root), &JunkChild);
	}
	DoResize(Event.xmotion.x_root, Event.xmotion.y_root, Tmp_win);
    }
    else
    if (Scr->BorderCursors && Tmp_win && Event.xany.window == Tmp_win->frame) {
	SetBorderCursor (Tmp_win, Event.xmotion.x, Event.xmotion.y);
    }
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonRelease - ButtonRelease event handler
 *
 ***********************************************************************
 */
void HandleButtonRelease(void)
{
    int xl, yt, w, h;
    unsigned mask;

#ifdef GNOME
    if (GnomeProxyButtonPress == Event.xbutton.button) {
      GnomeProxyButtonPress = -1;
      XSendEvent (dpy, Scr->currentvs->wsw->w, False, SubstructureNotifyMask, &Event);
    }
#endif /* GNOME */
    if (InfoLines) 		/* delete info box on 2nd button release  */
      if (Context == C_IDENTIFY) {
	XUnmapWindow(dpy, Scr->InfoWindow);
	InfoLines = 0;
	Context = C_NO_CONTEXT;
      }

    if (DragWindow != None)
    {
	MoveOutline(Scr->XineramaRoot, 0, 0, 0, 0, 0, 0);

	Tmp_win = GetTwmWindow(DragWindow);
	if (Tmp_win->winbox) {
	    XTranslateCoordinates (dpy, Scr->Root, Tmp_win->winbox->window,
		Event.xbutton.x_root, Event.xbutton.y_root,
		&(Event.xbutton.x_root), &(Event.xbutton.y_root), &JunkChild);
	}
	if (DragWindow == Tmp_win->frame)
	{
	    xl = Event.xbutton.x_root - DragX - Tmp_win->frame_bw;
	    yt = Event.xbutton.y_root - DragY - Tmp_win->frame_bw;
	    w = DragWidth + 2 * Tmp_win->frame_bw;
	    h = DragHeight + 2 * Tmp_win->frame_bw;
	}
	else
	{
	    xl = Event.xbutton.x_root - DragX - DragBW;
	    yt = Event.xbutton.y_root - DragY - DragBW;
	    w = DragWidth + 2 * DragBW;
	    h = DragHeight + 2 * DragBW;
	}

	if (ConstMove)
	{
	    if (ConstMoveDir == MOVE_HORIZ)
		yt = ConstMoveY;

	    if (ConstMoveDir == MOVE_VERT)
		xl = ConstMoveX;

	    if (ConstMoveDir == MOVE_NONE)
	    {
		yt = ConstMoveY;
		xl = ConstMoveX;
	    }
	}

	if (Scr->DontMoveOff && MoveFunction != F_FORCEMOVE)
	    TryToGrid (Tmp_win, &xl, &yt);
	if (MoveFunction == F_MOVEPUSH &&
	    Scr->OpaqueMove &&
	    DragWindow == Tmp_win->frame) TryToPush (Tmp_win,  xl,  yt, 0);
	if (MoveFunction == F_MOVEPACK ||
	    (MoveFunction == F_MOVEPUSH &&
	     DragWindow == Tmp_win->frame)) TryToPack (Tmp_win, &xl, &yt);
	if (Scr->DontMoveOff && MoveFunction != F_FORCEMOVE)
	{
            ConstrainByBorders (Tmp_win, &xl, w, &yt, h);
	}

	CurrentDragX = xl;
	CurrentDragY = yt;
	/*
	 * sometimes getScreenOf() replies with the wrong window when moving
	 * y to a negative number.  Need to figure out why... [XXX]
	 */
	if(xl < 0 || yt < 0 || xl > Scr->rootw || yt > Scr->rooth) {
		int odestx, odesty;
		int destx, desty;
		Window cr;
		virtualScreen *newvs;

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
	if (DragWindow == Tmp_win->frame)
	    SetupWindow (Tmp_win, xl, yt,
		       Tmp_win->frame_width, Tmp_win->frame_height, -1);
	else
	    XMoveWindow (dpy, DragWindow, xl, yt);

	if (!Scr->NoRaiseMove) /* && !Scr->OpaqueMove)    opaque already did */
	    RaiseFrame(DragWindow);

	if (!Scr->OpaqueMove)
	    UninstallRootColormap();
	else
	    XSync(dpy, 0);

	if (Scr->NumAutoRaises) {
	    enter_flag = TRUE;
	    enter_win = NULL;
	    raise_win = ((DragWindow == Tmp_win->frame && !Scr->NoRaiseMove)
			 ? Tmp_win : NULL);
	}

	/* CCC equivalent code for auto lower not needed? */

#if 0
	if (Scr->NumAutoLowers) {
	    leave_flag = TRUE;
	    leave_win = NULL;
	    lower_win = ((DragWindow == Tmp_win->frame)
			 ? Tmp_win : NULL);
	}
#endif

	DragWindow = (Window) 0;
	ConstMove = FALSE;
    }

    if (ResizeWindow != (Window) 0)
    {
	EndResize();
    }

    if (ActiveMenu != NULL && RootFunction == 0)
    {
	if (ActiveItem)
	{
	    int func = ActiveItem->func;
	    Action = ActiveItem->action;
	    switch (func) {
	      case F_TITLE:
		if (Scr->StayUpMenus) 	{
		    ButtonPressed = -1;
		    if (Scr->WarpToDefaultMenuEntry && ActiveMenu->defaultitem) {
			WarpCursorToDefaultEntry (ActiveMenu);
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
	    if (func != F_PIN && func != F_MENU) PopDownMenu();
	    ExecuteFunction(func, Action,
		ButtonWindow ? ButtonWindow->frame : None,
		ButtonWindow, &Event/*&ButtonEvent*/, Context, TRUE);
	    Context = C_NO_CONTEXT;
	    ButtonWindow = NULL;

	    /* if we are not executing a defered command, then take down the
	     * menu
	     */
	    if (ActiveMenu) PopDownMenu();
	}
	else
	if (Scr->StayUpMenus && !ActiveMenu->entered) {
	    ButtonPressed = -1;
	    if (Scr->WarpToDefaultMenuEntry && ActiveMenu->defaultitem) {
		WarpCursorToDefaultEntry (ActiveMenu);
	    }
	    return;
	}
	else
	    PopDownMenu();
    }

    mask = (Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask);
    switch (Event.xbutton.button)
    {
	case Button1: mask &= ~Button1Mask; break;
	case Button2: mask &= ~Button2Mask; break;
	case Button3: mask &= ~Button3Mask; break;
	case Button4: mask &= ~Button4Mask; break;
	case Button5: mask &= ~Button5Mask; break;
    }

    if (RootFunction != 0 ||
	ResizeWindow != None ||
	DragWindow != None)
	ButtonPressed = -1;

    if (AlternateKeymap || AlternateContext) {
	ButtonPressed = -1;
	return;
    }

    if (RootFunction == 0 &&
	(Event.xbutton.state & mask) == 0 &&
	DragWindow == None &&
	ResizeWindow == None)
    {
	XUngrabPointer(dpy, CurrentTime);
	XUngrabServer(dpy);
	XFlush(dpy);
	EventHandler[EnterNotify] = HandleEnterNotify;
	EventHandler[LeaveNotify] = HandleLeaveNotify;
	ButtonPressed = -1;
	if (DownIconManager)
	{
	    DownIconManager->down = FALSE;
	    if (Scr->Highlight) DrawIconManagerBorder(DownIconManager, False);
	    DownIconManager = NULL;
	}
	Cancel = FALSE;
    }
}



static void do_menu (MenuRoot *menu,	/* menu to pop up */
		     Window w)		/* invoking window or None */
{
    int x = Event.xbutton.x_root;
    int y = Event.xbutton.y_root;
    Bool center;

    if (!Scr->NoGrabServer)
	XGrabServer(dpy);
    if (w) {
	int h = Scr->TBInfo.width - Scr->TBInfo.border;
	Window child;

	(void) XTranslateCoordinates (dpy, w, Scr->Root, 0, h, &x, &y, &child);
	center = False;
    } else {
	center = True;
    }
    if (PopUpMenu (menu, x, y, center)) {
	UpdateMenu();
    } else {
	XBell (dpy, 0);
    }
}

static void do_key_menu (MenuRoot *menu,	/* menu to pop up */
			 Window w)		/* invoking window or None */
{
    int x = Event.xkey.x_root;
    int y = Event.xkey.y_root;
    Bool center;

/* I don't think this is necessary.
    if (!Scr->NoGrabServer) XGrabServer(dpy);
*/
    if (w) {
	int h = Scr->TBInfo.width - Scr->TBInfo.border;
	Window child;

	(void) XTranslateCoordinates (dpy, w, Scr->Root, 0, h, &x, &y, &child);
	center = False;
    } else {
	center = True;
    }
    if (PopUpMenu (menu, x, y, center)) {
	UpdateMenu();
    } else {
	XBell (dpy, 0);
    }

}



/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonPress - ButtonPress event handler
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

    GnomeProxyButtonPress = -1;

    /* pop down the menu, if any */

    if (XFindContext (dpy, Event.xbutton.window, MenuContext, (XPointer *) &mr) != XCSUCCESS) {
	mr = (MenuRoot*) 0;
    }
    if (ActiveMenu && (! ActiveMenu->pinned) &&
		(Event.xbutton.subwindow != ActiveMenu->w)) {
	PopDownMenu();
	return;
    }
    if ((ActiveMenu != NULL) && (RootFunction != 0) && (mr != ActiveMenu)) PopDownMenu();

    XSync(dpy, 0);
			/* XXX - remove? */

    if (ButtonPressed != -1 && !InfoLines) /* want menus if we have info box */
    {
	/* we got another butt press in addition to one still held
	 * down, we need to cancel the operation we were doing
	 */
	Cancel = TRUE;
	CurrentDragX = origDragX;
	CurrentDragY = origDragY;
	if (!menuFromFrameOrWindowOrTitlebar) {
	  if (Scr->OpaqueMove && DragWindow != None) {
	    XMoveWindow (dpy, DragWindow, origDragX, origDragY);
	  } else {
	    MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
	  }
	}
	XUnmapWindow(dpy, Scr->SizeWindow);
	if (!Scr->OpaqueMove)
	    UninstallRootColormap();
	ResizeWindow = None;
	DragWindow = None;
	cur = LeftButt;
	if (Event.xbutton.button == Button2)
	    cur = MiddleButt;
	else if (Event.xbutton.button >= Button3)
	    cur = RightButt;

	XGrabPointer(dpy, Scr->Root, True,
	    ButtonReleaseMask | ButtonPressMask,
	    GrabModeAsync, GrabModeAsync,
	    Scr->Root, cur, CurrentTime);

	return;
    }
    else
	ButtonPressed = Event.xbutton.button;

    if ((ActiveMenu != NULL) && (ActiveMenu->pinned)) {
	if (Event.xbutton.window == ActiveMenu->w) {
	    modifier = (Event.xbutton.state & mods_used);
	    modifier = set_mask_ignore (modifier);
	    if ((ActiveItem && (ActiveItem->func == F_TITLE)) || (modifier == 8)) {
		MoveMenu (&Event);
		/*ButtonPressed = -1;*/
	    }
	}
	Context = C_ROOT;
	return;
    }

    if (ResizeWindow != None ||
	DragWindow != None  ||
	ActiveMenu != NULL)
	return;

    /* check the title bar buttons */
    if (Tmp_win && Tmp_win->title_height && Tmp_win->titlebuttons)
    {
	register int i;
	register TBWindow *tbw;
	register TitleButtonFunc *tbf;
	int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

	modifier = Event.xbutton.state & mods_used;
	modifier = set_mask_ignore (modifier);

	for (i = 0, tbw = Tmp_win->titlebuttons; i < nb; i++, tbw++) {
	    if (Event.xany.window == tbw->window) {
		for (tbf = tbw->info->funs; tbf; tbf = tbf->next) {
		    if (tbf->num == ButtonPressed
			&& tbf->mods == modifier) {
			switch (tbf->func) {
			case F_MENU :
			    Context = C_TITLE;
			    ButtonEvent = Event;
			    ButtonWindow = Tmp_win;
			    do_menu (tbf->menuroot, tbw->window);
			    break;

			default :
			    ExecuteFunction (tbf->func, tbf->action,
					     Event.xany.window, Tmp_win,
					     &Event, C_TITLE, FALSE);
			}
			return;
		    }
		}
	    }
	}
    }

    Context = C_NO_CONTEXT;

    if (Event.xany.window == Scr->InfoWindow)
      Context = C_IDENTIFY;

    if (Event.xany.window == Scr->Root) {
	if (AlternateContext) {
	    XUngrabPointer  (dpy, CurrentTime);
	    XUngrabKeyboard (dpy, CurrentTime);
	    AlternateContext = False;
	    Context = C_ALTERNATE;
	}
	else
	if (AlternateKeymap && Event.xbutton.subwindow) {
	    int dx, dy;
	    Window child;

	    w = Event.xbutton.subwindow;
	    Tmp_win = GetTwmWindow(w);
	    if (Tmp_win) {
		Event.xany.window    = Tmp_win->frame;
		XTranslateCoordinates (dpy, Scr->Root, Tmp_win->frame,
			Event.xbutton.x, Event.xbutton.y, &dx, &dy, &child);
		Event.xbutton.x = dx;
		Event.xbutton.x = dy;
		Event.xbutton.subwindow = child;
	    }
	}
	else
	    Context = C_ROOT;
    }
    if (Tmp_win)
    {
	if (Tmp_win->iconmanagerlist && (RootFunction != 0) &&
		((Event.xany.window == Tmp_win->iconmanagerlist->icon) ||
		 (Event.xany.window == Tmp_win->iconmanagerlist->w))) {
	    Tmp_win = Tmp_win->iconmanagerlist->iconmgr->twm_win;
	    if (Tmp_win) {
		XTranslateCoordinates(dpy, Event.xany.window, Tmp_win->w,
		    Event.xbutton.x, Event.xbutton.y, 
		    &JunkX, &JunkY, &JunkChild);

		Event.xbutton.x = JunkX - Tmp_win->frame_bw3D;
		Event.xbutton.y = JunkY - Tmp_win->title_height - Tmp_win->frame_bw3D;
		Event.xany.window = Tmp_win->w;
		Context = C_WINDOW;
	    }
	}
	else if (Event.xany.window == Tmp_win->title_w) {
	    if (Scr->ClickToFocus &&
		Tmp_win->wmhints &&
		Tmp_win->wmhints->input) SetFocus (Tmp_win, CurrentTime);
	    Context = C_TITLE;
	}
	else if (Event.xany.window == Tmp_win->w) {
	    if (Scr->ClickToFocus || Scr->RaiseOnClick) {
		if (Scr->ClickToFocus &&
		    Tmp_win->wmhints &&
		    Tmp_win->wmhints->input) {
		    SetFocus (Tmp_win, CurrentTime);
		}
		if (Scr->RaiseOnClick) {
		    RaiseWindow (Tmp_win);
		    WMapRaise   (Tmp_win);
		}
		XSync (dpy, 0);
		XAllowEvents (dpy, ReplayPointer, CurrentTime);
		XSync (dpy, 0);
		ButtonPressed = -1;
		return;
	    }
	    else {
		printf("ERROR! ERROR! ERROR! YOU SHOULD NOT BE HERE!!!\n");
		Context = C_WINDOW;
	    }
	}
	else if (Tmp_win->icon && (Event.xany.window == Tmp_win->icon->w))
	{
	    Context = C_ICON;
	}
	else if (Event.xany.window == Tmp_win->frame) 
	{
	    /* since we now place a button grab on the frame instead
             * of the window, (see GrabButtons() in add_window.c), we
             * need to figure out where the pointer exactly is before
             * assigning Context.  If the pointer is on the application
             * window we will change the event structure to look as if
             * it came from the application window.
	     */
	    if (Event.xbutton.subwindow == Tmp_win->w) {
	      XTranslateCoordinates (dpy, Event.xany.window, Tmp_win->w,
			Event.xbutton.x, Event.xbutton.y,
			&Event.xbutton.x, &Event.xbutton.y, &JunkChild);
	      Event.xbutton.window = Tmp_win->w;

	      if (Tmp_win->iswinbox && JunkChild) {
		    XTranslateCoordinates (dpy, Tmp_win->w, JunkChild,
			Event.xbutton.x, Event.xbutton.y,
			&JunkX, &JunkY, &JunkChild);
		    if (JunkChild && (Tmp_win = GetTwmWindow(JunkChild))) {
			Event.xany.window = JunkChild;
			Event.xbutton.x   = JunkX;
			Event.xbutton.y   = JunkY;
		    }
	      }
	      Context = C_WINDOW;
	    }
	    else
	    if (Event.xbutton.subwindow && (Event.xbutton.subwindow == Tmp_win->title_w)) {
		Context = C_TITLE;
	    }
            else {
		Context = C_FRAME;
	    }
	    if (Scr->ClickToFocus &&
		Tmp_win->wmhints &&
		Tmp_win->wmhints->input) SetFocus (Tmp_win, CurrentTime);
	}
	else if (Tmp_win->wspmgr ||
		 (Tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win)) {
	    Context = C_WINDOW;
	}
	else if (Tmp_win->iconmanagerlist) {
	    if ((Event.xany.window == Tmp_win->iconmanagerlist->icon) ||
		(Event.xany.window == Tmp_win->iconmanagerlist->w)) {
		Tmp_win->iconmanagerlist->down = TRUE;
		if (Scr->Highlight)
		    DrawIconManagerBorder(Tmp_win->iconmanagerlist, False);
		DownIconManager = Tmp_win->iconmanagerlist;
		Context = C_ICONMGR;
	    }
	}
    }

    /* this section of code checks to see if we were in the middle of
     * a command executed from a menu
     */
    if (RootFunction != 0)
    {
	if (Event.xany.window == Scr->Root)
	{
	    Window win;
	    /* if the window was the Root, we don't know for sure it
	     * it was the root.  We must check to see if it happened to be
	     * inside of a client that was getting button press events.
	     */
	    XTranslateCoordinates(dpy, Scr->Root, Scr->Root,
		Event.xbutton.x, 
		Event.xbutton.y, 
		&JunkX, &JunkY, &Event.xany.window);

	    if (Event.xany.window != 0 &&
		(Tmp_win = GetTwmWindow(Event.xany.window))) {
		if (Tmp_win && Tmp_win->iswinbox) {
		    XTranslateCoordinates (dpy, Scr->Root, Event.xany.window,
			JunkX, JunkY,  &JunkX, &JunkY, &win);
		    XTranslateCoordinates (dpy, Event.xany.window, win,
			JunkX, JunkY,  &JunkX, &JunkY, &win);
		    if (win != 0) Event.xany.window = win;
		}
	    }
	    if (Event.xany.window == 0 ||
		!(Tmp_win = GetTwmWindow(Event.xany.window)))
	    {
		RootFunction = 0;
		XBell(dpy, 0);
		return;
	    }
	    XTranslateCoordinates(dpy, Scr->Root, Event.xany.window,
		Event.xbutton.x, 
		Event.xbutton.y, 
		&JunkX, &JunkY, &JunkChild);

	    Event.xbutton.x = JunkX;
	    Event.xbutton.y = JunkY;
	    Context = C_WINDOW;
	}
	else
	if (mr != (MenuRoot*) 0) {
	    RootFunction = 0;
	    XBell(dpy, 0);
	    return;
	}

	/* make sure we are not trying to move an identify window */
	if (Event.xany.window != Scr->InfoWindow)
	  ExecuteFunction(RootFunction, Action, Event.xany.window,
			  Tmp_win, &Event, Context, FALSE);

	RootFunction = 0;
	return;
    }

    ButtonEvent = Event;
    ButtonWindow = Tmp_win;

    /* if we get to here, we have to execute a function or pop up a 
     * menu
     */
    modifier = (Event.xbutton.state | AlternateKeymap) & mods_used;
    modifier = set_mask_ignore (modifier);
    if (AlternateKeymap) {
	XUngrabPointer  (dpy, CurrentTime);
	XUngrabKeyboard (dpy, CurrentTime);
	AlternateKeymap = 0;
    }
    if ((Context == C_NO_CONTEXT) || (Context == C_IDENTIFY))
	return;

    RootFunction = 0;

    /* see if there already is a key defined for this context */
    for (tmp = Scr->FuncButtonRoot.next; tmp != NULL; tmp = tmp->next) {
	if ((tmp->num  == Event.xbutton.button) &&
	    (tmp->cont == Context) && (tmp->mods == modifier))
	    break;
    }
    if (tmp) {
	func = tmp->func;
	switch (func) {
	    case F_MENU :
		do_menu (tmp->menu, (Window) None);
		break;

	    default :
		if (func != 0) {
		    Action = tmp->item ? tmp->item->action : NULL;
		    ExecuteFunction (func,
			Action, Event.xany.window, Tmp_win, &Event, Context, FALSE);
		}
	}
    }
    else {
	if (Tmp_win == Scr->currentvs->wsw->twm_win) {
	  WMgrHandleButtonEvent (Scr->currentvs, &Event);
	  return;
	}
    }
    if (Tmp_win == Scr->workSpaceMgr.occupyWindow->twm_win)
    {
	OccupyHandleButtonEvent (&Event);
    }
    else if (func == 0 && Scr->DefaultFunction.func != 0)
    {
	if (Scr->DefaultFunction.func == F_MENU)
	{
	    do_menu (Scr->DefaultFunction.menu, (Window) None);
	}
	else
	{
	    Action = Scr->DefaultFunction.item ?
		Scr->DefaultFunction.item->action : NULL;
	    ExecuteFunction(Scr->DefaultFunction.func, Action,
	       Event.xany.window, Tmp_win, &Event, Context, FALSE);
	}
    }
#ifdef GNOME1 /* Makes DeferExecution (in menus.c) fail. TODO. */
    else {
        /* GNOME: Pass on the event to any applications listening for root window clicks */
        GnomeProxyButtonPress = Event.xbutton.button;
	ButtonPressed = -1;
	XUngrabPointer (dpy, CurrentTime);
	XSendEvent (dpy, Scr->currentvs->wsw->twm_win->w, False,
		    SubstructureNotifyMask, &Event);
    }
#endif /* GNOME1 */
}



/***********************************************************************
 *
 *  Procedure:
 *	HENQueueScanner - EnterNotify event q scanner
 *
 *	Looks at the queued events and determines if any matching
 *	LeaveNotify events or EnterEvents deriving from the
 *	termination of a grab are behind this event to allow
 *	skipping of unnecessary processing.
 *
 ***********************************************************************
 */

typedef struct HENScanArgs {
    Window w;		/* Window we are currently entering */
    Bool leaves;	/* Any LeaveNotifies found for this window */
    Bool inferior;	/* Was NotifyInferior the mode for LeaveNotify */
    Bool enters;	/* Any EnterNotify events with NotifyUngrab */
} HENScanArgs;

/* ARGSUSED*/
static Bool HENQueueScanner(Display *display, XEvent *ev, char *_args)
{
    HENScanArgs *args = (void *)_args;

    if (ev->type == LeaveNotify) {
	if (ev->xcrossing.window == args->w &&
	    ev->xcrossing.mode == NotifyNormal) {
	    args->leaves = True;
	    /*
	     * Only the last event found matters for the Inferior field.
	     */
	    args->inferior =
		(ev->xcrossing.detail == NotifyInferior);
	}
    } else if (ev->type == EnterNotify) {
	if (ev->xcrossing.mode == NotifyUngrab)
	    args->enters = True;
    }

    return (False);
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleEnterNotify - EnterNotify event handler
 *
 ***********************************************************************
 */

void HandleEnterNotify(void)
{
    MenuRoot *mr, *tmp;
    XEnterWindowEvent *ewp = &Event.xcrossing;
    HENScanArgs scanArgs;
    XEvent dummy;
    virtualScreen *vs;

    /*
     * if we aren't in the middle of menu processing
     */
    if (!ActiveMenu) {
	/*
	 * We're not interested in pseudo Enter/Leave events generated
	 * from grab initiations.
	 */
	if (ewp->mode == NotifyGrab)
	    return;

	/*
	 * Scan for Leave and Enter Notify events to see if we can avoid some
	 * unnecessary processing.
	 */
	scanArgs.w = ewp->window;
	scanArgs.leaves = scanArgs.enters = False;
	(void) XCheckIfEvent(dpy, &dummy, HENQueueScanner, (void *) &scanArgs);

	/*
	 * if entering root window, restore twm default colormap so that 
	 * titlebars are legible
	 */
	if (ewp->window == Scr->Root) {
	    Window forus_ret;
	    int focus_rev;

	    if (!scanArgs.leaves && !scanArgs.enters) {
		InstallColormaps(EnterNotify, &Scr->RootColormaps);
	    }
	    if (! Scr->FocusRoot) return;
	    XGetInputFocus (dpy, &forus_ret, &focus_rev);
	    if ((forus_ret != PointerRoot) && (forus_ret != None)) {
		SetFocus ((TwmWindow *) NULL, Event.xcrossing.time);
	    }
	    return;
	}
	for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
	  if (ewp->window == vs->window) {
	    Scr->Root  = vs->window;
	    Scr->rootx = Scr->crootx + vs->x;
	    Scr->rooty = Scr->crooty + vs->y;
	    Scr->rootw = vs->w;
	    Scr->rooth = vs->h;
	    Scr->currentvs = vs;
	    /*fprintf (stderr, "entering new vs : 0x%x, 0x%x, %d, %d, %d, %d\n",
	      vs, Scr->Root, vs->x, vs->y, vs->w, vs->h);*/
	    return;
	  }
	}

	/* Handle RaiseDelay, if any.....
	 */
	if (RaiseDelay > 0) {
	    if (Tmp_win && Tmp_win->auto_raise &&
		    (!Tmp_win->iconmanagerlist ||
		      Tmp_win->iconmanagerlist->w != ewp->window)) {
		ColormapWindow *cwin;
#ifdef VMS
		float timeout = 0.0125;
#else
		static struct timeval tout, timeout = {0,12500};
#endif

		if (XFindContext(dpy, Tmp_win->w, ColormapContext,
				 (XPointer *)&cwin) == XCNOENT) {
		    cwin = (ColormapWindow *)NULL;
		}

		if ((ewp->detail != NotifyInferior
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
		    for (i = 25; i < RaiseDelay; i += 25) {
#ifdef VMS
			lib$wait(&timeout);
#else
			tout = timeout;
			select(0, 0, 0, 0, &tout);
#endif
			/* Did we leave this window already? */
			scanArgs.w = ewp->window;
			scanArgs.leaves = scanArgs.enters = False;
			(void) XCheckIfEvent(dpy, &dummy, HENQueueScanner,
					     (void *) &scanArgs);
			if (scanArgs.leaves && !scanArgs.inferior) return;

			XQueryPointer(dpy, Scr->Root, &w, &w, &x, &y,
				      &d, &d, (unsigned int *)&d);

			/* Has the pointer moved?  If so reset the loop cnt.
			 * We want the pointer to be still for RaiseDelay
			 * milliseconds before terminating the loop
			 */
			if (x != px || y != py) {
			    i = 0; px = x; py = y;
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
	    (void) XCheckIfEvent(dpy, &dummy, HENQueueScanner, (void *) &scanArgs);

	    /*
	     * if entering root window, restore twm default colormap so that 
	     * titlebars are legible
	     */
	    if (ewp->window == Scr->Root) {
		if (!scanArgs.leaves && !scanArgs.enters) {
		    InstallColormaps(EnterNotify, &Scr->RootColormaps);
		}
		return;
	    }
	}
	/* End of RaiseDelay modification. */
  
	/*
	 * if we have an event for a specific one of our windows
	 */
	if (Tmp_win) {
	    /*
	     * If currently in PointerRoot mode (indicated by FocusRoot), then
	     * focus on this window
	     */
	    if (Scr->FocusRoot && (!scanArgs.leaves || scanArgs.inferior)) {
		Bool accinput;

		if (Scr->ShrinkIconTitles &&
		    Tmp_win->icon &&
		    ewp->window == Tmp_win->icon->w &&
		    ewp->detail != NotifyInferior) {
		    if (Scr->AutoRaiseIcons) XRaiseWindow (dpy, Tmp_win->icon->w);
		    ExpandIconTitle (Tmp_win);
		    return;
		}

		if (Tmp_win->iconmanagerlist)
		    CurrentIconManagerEntry (Tmp_win->iconmanagerlist);

		accinput = Tmp_win->mapped && Tmp_win->wmhints && Tmp_win->wmhints->input;
		if (Tmp_win->iconmanagerlist &&
		    ewp->window == Tmp_win->iconmanagerlist->w &&
		    !accinput &&
		    Tmp_win->iconmanagerlist->iconmgr &&
		    Tmp_win->iconmanagerlist->iconmgr->twm_win) {
			SetFocus(Tmp_win->iconmanagerlist->iconmgr->twm_win,
				 CurrentTime);
			return;
		}

		if (Tmp_win->mapped) {
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
		    if (Scr->BorderCursors && ewp->window == Tmp_win->frame) {
			SetBorderCursor (Tmp_win, ewp->x, ewp->y);
		    }
		    if (ewp->window == Tmp_win->frame ||
			(Scr->IconManagerFocus &&
			 Tmp_win->iconmanagerlist &&
			 ewp->window == Tmp_win->iconmanagerlist->w)) {

			if (!scanArgs.leaves && !scanArgs.enters) {
			    InstallColormaps (EnterNotify,	/* 2 */
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
			if (Tmp_win->wmhints && Tmp_win->wmhints->input) {
				
				/* if 4 or 4a, focus on the window */
				if (Scr->TitleFocus ||  
				    (Tmp_win->iconmanagerlist && 
				     (Tmp_win->iconmanagerlist->w == ewp->window))) {
				  SetFocus (Tmp_win, ewp->time);
				}
			}
			    
			if (Scr->TitleFocus &&
			    (Tmp_win->protocols & DoesWmTakeFocus)){    /* 5 */

				/* for both locally or globally active */
				SendTakeFocusMessage (Tmp_win, ewp->time);
			}
			else if (!Scr->TitleFocus 
				 && Tmp_win->wmhints 
				 && Tmp_win->wmhints->input
				 && Event.xcrossing.focus) {
			    SynthesiseFocusIn(Tmp_win->w);
			}

 		    } else if (ewp->window == Tmp_win->w) {
			/*
			 * If we are entering the application window, install
			 * its colormap(s).
			 */
			if (Scr->BorderCursors) SetBorderCursor (Tmp_win, -1000, -1000);
			if (!scanArgs.leaves || scanArgs.inferior) {
			    InstallWindowColormaps(EnterNotify, Tmp_win);
			}

			if (Event.xcrossing.focus) {
				SynthesiseFocusIn(Tmp_win->w);
			}

			/* must deal with WM_TAKE_FOCUS clients now, if 
			   we're not in TitleFocus mode */

			if (!(Scr->TitleFocus) &&
			    (Tmp_win->protocols & DoesWmTakeFocus)) {

				/* locally active clients need help from WM
				   to get the input focus */
				
				if (Tmp_win->wmhints &&
				    Tmp_win->wmhints->input)
				  SetFocus(Tmp_win, ewp->time);

				/* for both locally & globally active clnts */

				SendTakeFocusMessage(Tmp_win, ewp->time);
			}
		    }
		}			/* end if Tmp_win->mapped */
		if (Tmp_win->wmhints != NULL &&
			ewp->window == Tmp_win->wmhints->icon_window &&
			(!scanArgs.leaves || scanArgs.inferior)) {
			    InstallWindowColormaps(EnterNotify, Tmp_win);
		}
	    }				/* end if FocusRoot */
	    else
	    if (Scr->BorderCursors && (ewp->window == Tmp_win->w)) {
		SetBorderCursor (Tmp_win, -1000, -1000);
	    }
	    /*
	     * If this window is to be autoraised, mark it so
	     */
	    if (Tmp_win->auto_raise) {
		enter_win = Tmp_win;
		if (enter_flag == FALSE) AutoRaiseWindow (Tmp_win);
	    } else if (enter_flag && raise_win == Tmp_win)
	      enter_win = Tmp_win;
	    /*
	     * set ring leader
	     */
	    if (Tmp_win->ring.next && (!enter_flag || raise_win == enter_win))
	      Scr->RingLeader = Tmp_win;
	    XSync (dpy, 0);
	    return;
	}				/* end if Tmp_win */
    }					/* end if !ActiveMenu */

    /*
     * Find the menu that we are dealing with now; punt if unknown
     */
    if (XFindContext (dpy, ewp->window, MenuContext, (XPointer *)&mr) != XCSUCCESS) return;

    if (! ActiveMenu && mr->pinned && (RootFunction == 0)) {
	PopUpMenu (mr, 0, 0, 0);
	Context = C_ROOT;
	UpdateMenu ();
	return;
    }
    mr->entered = TRUE;
    if (RootFunction == 0) {
	for (tmp = ActiveMenu; tmp; tmp = tmp->prev) {
	    if (tmp == mr) break;
	}
	if (! tmp) return;

	for (tmp = ActiveMenu; tmp != mr; tmp = tmp->prev) {
	    if (tmp->pinned) break;
	    HideMenu (tmp);
	    MenuDepth--;
	}
	UninstallRootColormap ();

	if (ActiveItem) {
	    ActiveItem->state = 0;
	    PaintEntry (ActiveMenu, ActiveItem,  False);
	}
	ActiveItem = NULL;
	ActiveMenu = mr;
	if (1/*Scr->StayUpMenus*/) {
	    int i, x, y, x_root, y_root, entry;
	    MenuItem *mi;

	    XQueryPointer (dpy, ActiveMenu->w, &JunkRoot, &JunkChild, &x_root, &y_root,
			&x, &y, &JunkMask);
	    if ((x > 0) && (y > 0) && (x < ActiveMenu->width) && (y < ActiveMenu->height)) {
		entry = y / Scr->EntryHeight;
		for (i = 0, mi = ActiveMenu->first; mi != NULL; i++, mi=mi->next) {
		    if (i == entry) break;
		}
		if (mi) {
		    ActiveItem = mi;
		    ActiveItem->state = 1;
		    PaintEntry (ActiveMenu, ActiveItem, False);
		}
	    }
	}
	if (ActiveMenu->pinned) XUngrabPointer(dpy, CurrentTime);
    }
    return;
}



/***********************************************************************
 *
 *  Procedure:
 *	HLNQueueScanner - LeaveNotify event q scanner
 *
 *	Looks at the queued events and determines if any
 *	EnterNotify events are behind this event to allow
 *	skipping of unnecessary processing.
 *
 ***********************************************************************
 */

typedef struct HLNScanArgs {
    Window w;		/* The window getting the LeaveNotify */
    Bool enters;	/* Any EnterNotify event at all */
    Bool matches;	/* Any matching EnterNotify events */
} HLNScanArgs;

/* ARGSUSED*/
static Bool HLNQueueScanner(Display *display, XEvent *ev, char *_args)
{
    HLNScanArgs *args = (void *)_args;

    if (ev->type == EnterNotify && ev->xcrossing.mode != NotifyGrab) {
	args->enters = True;
	if (ev->xcrossing.window == args->w)
	    args->matches = True;
    }

    return (False);
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleLeaveNotify - LeaveNotify event handler
 *
 ***********************************************************************
 */

void HandleLeaveNotify(void)
{
    HLNScanArgs scanArgs;
    XEvent dummy;

    if (ActiveMenu && ActiveMenu->pinned && (Event.xcrossing.window == ActiveMenu->w)) {
	PopDownMenu ();
    }

    if (Tmp_win != NULL)
    {
	Bool inicon;

	/*
	 * We're not interested in pseudo Enter/Leave events generated
	 * from grab initiations and terminations.
	 */
	if (Event.xcrossing.mode != NotifyNormal)
	    return;

	if (Scr->ShrinkIconTitles &&
	    Tmp_win->icon &&
	    Event.xcrossing.window == Tmp_win->icon->w &&
	    Event.xcrossing.detail != NotifyInferior) {
	    ShrinkIconTitle (Tmp_win);
	    return;
	}

	inicon = (Tmp_win->iconmanagerlist &&
		  Tmp_win->iconmanagerlist->w == Event.xcrossing.window);

	if (Scr->RingLeader && Scr->RingLeader == Tmp_win &&
	    (Event.xcrossing.detail != NotifyInferior &&
	     Event.xcrossing.window != Tmp_win->w)) {
#ifdef DEBUG
	     fprintf(stderr, "HandleLeaveNotify: Event.xcrossing.window %x != Tmp_win->w %x\n", Event.xcrossing.window, Tmp_win->w);
#endif
	    if (!inicon) {
		if (Event.xcrossing.window != Tmp_win->frame /*was: Tmp_win->mapped*/) {
		    Tmp_win->ring.cursor_valid = False;
#ifdef DEBUG
		    fprintf(stderr, "HandleLeaveNotify: cursor_valid = False\n");
#endif
		} else {	/* Event.xcrossing.window == Tmp_win->frame */
		    Tmp_win->ring.cursor_valid = True;
		    Tmp_win->ring.curs_x = (Event.xcrossing.x_root -
					    Tmp_win->frame_x);
		    Tmp_win->ring.curs_y = (Event.xcrossing.y_root -
					    Tmp_win->frame_y);
#ifdef DEBUG
		    fprintf(stderr, "HandleLeaveNotify: cursor_valid = True; x = %d (%d-%d), y = %d (%d-%d)\n", Tmp_win->ring.curs_x, Event.xcrossing.x_root, Tmp_win->frame_x, Tmp_win->ring.curs_y, Event.xcrossing.y_root, Tmp_win->frame_y);
#endif
		}
	    }
	    Scr->RingLeader = (TwmWindow *) NULL;
	}
	if (Scr->FocusRoot) {

	    if (Event.xcrossing.detail != NotifyInferior) {

		/*
		 * Scan for EnterNotify events to see if we can avoid some
		 * unnecessary processing.
		 */
		scanArgs.w = Event.xcrossing.window;
		scanArgs.enters = scanArgs.matches = False;
		(void) XCheckIfEvent(dpy, &dummy, HLNQueueScanner,
				     (char *) &scanArgs);

		if (Event.xcrossing.window == Tmp_win->frame && !scanArgs.matches) {
		    if (Scr->TitleFocus ||
			Tmp_win->protocols & DoesWmTakeFocus)
		      SetFocus ((TwmWindow *) NULL, Event.xcrossing.time);
		    /* pretend there was a focus out as sometimes 
		       * we don't get one. */
		    if ( Event.xcrossing.focus)
                      SynthesiseFocusOut(Tmp_win->w);
		}
		else
		if (Scr->IconManagerFocus && inicon) {
		    if (! Tmp_win->mapped ||
			! Tmp_win->wmhints ||
			! Tmp_win->wmhints->input) {
			return;
		    }
		    if (Scr->TitleFocus || Tmp_win->protocols & DoesWmTakeFocus)
			SetFocus ((TwmWindow *) NULL, Event.xcrossing.time);
			if (Event.xcrossing.focus) SynthesiseFocusOut (Tmp_win->w);
		} else if (Event.xcrossing.window == Tmp_win->w &&
				!scanArgs.enters) {
		    InstallColormaps(LeaveNotify, &Scr->RootColormaps);
		}
	    }
	}
	/* Autolower modification. */
	if (Tmp_win->auto_lower) {
	    leave_win = Tmp_win;
	    if (leave_flag == FALSE) AutoLowerWindow (Tmp_win);
	} else if (leave_flag && lower_win == Tmp_win)
	    leave_win = Tmp_win;

	XSync (dpy, 0);
	return;
    }
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleConfigureRequest - ConfigureRequest event handler
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
    Bool sendEvent;

#ifdef DEBUG_EVENTS
    fprintf(stderr, "ConfigureRequest\n");
    if (cre->value_mask & CWX)
	fprintf(stderr, "  x = %d\n", cre->x);
    if (cre->value_mask & CWY)
	fprintf(stderr, "  y = %d\n", cre->y);
    if (cre->value_mask & CWWidth)
	fprintf(stderr, "  width = %d\n", cre->width);
    if (cre->value_mask & CWHeight)
	fprintf(stderr, "  height = %d\n", cre->height);
    if (cre->value_mask & CWSibling)
	fprintf(stderr, "  above = 0x%x\n", cre->above);
    if (cre->value_mask & CWStackMode)
	fprintf(stderr, "  stack = %d\n", cre->detail);
#endif

    /*
     * Event.xany.window is Event.xconfigurerequest.parent, so Tmp_win will
     * be wrong
     */
    Event.xany.window = cre->window;	/* mash parent field */
    Tmp_win = GetTwmWindow(cre->window);

    /*
     * According to the July 27, 1988 ICCCM draft, we should ignore size and
     * position fields in the WM_NORMAL_HINTS property when we map a window.
     * Instead, we'll read the current geometry.  Therefore, we should respond
     * to configuration requests for windows which have never been mapped.
     */
    if (!Tmp_win || (Tmp_win->icon && (Tmp_win->icon->w == cre->window))) {
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

    sendEvent = False;
    if ((cre->value_mask & CWStackMode) && Tmp_win->stackmode) {
	TwmWindow *otherwin;

	xwc.sibling = (((cre->value_mask & CWSibling) &&
			(otherwin = GetTwmWindow(cre->above)))
		       ? otherwin->frame : cre->above);
	xwc.stack_mode = cre->detail;
	XConfigureWindow (dpy, Tmp_win->frame, 
			  cre->value_mask & (CWSibling | CWStackMode), &xwc);
	sendEvent = True;
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
    GetGravityOffsets (Tmp_win, &gravx, &gravy);

    if (cre->value_mask & CWBorderWidth) {
	int bwdelta = cre->border_width - Tmp_win->old_bw;  /* posit growth */
	if (bwdelta && Scr->ClientBorderWidth) {  /* if change allowed */
	    x += gravx * bwdelta;	/* change default values only */
	    y += gravy * bwdelta;	/* ditto */
	    bw = cre->border_width;
	    if (Tmp_win->title_height) height += bwdelta;
	    x += (gravx < 0) ? bwdelta : -bwdelta;
	    y += (gravy < 0) ? bwdelta : -bwdelta;
	}
	Tmp_win->old_bw = cre->border_width;  /* for restoring */
    }

    if (cre->value_mask & CWX) {	/* override even if border change */
	x = cre->x - bw;
	x -= ((gravx < 0) ? 0 : Tmp_win->frame_bw3D);
    }
    if (cre->value_mask & CWY) {
	y = cre->y - ((gravy < 0) ? 0 : Tmp_win->title_height) - bw;
	y -= ((gravy < 0) ? 0 : Tmp_win->frame_bw3D);
    }

    if (cre->value_mask & CWWidth) {
	width = cre->width + 2 * Tmp_win->frame_bw3D;
    }
    if (cre->value_mask & CWHeight) {
	height = cre->height + Tmp_win->title_height + 2 * Tmp_win->frame_bw3D;
    }

    if (width != Tmp_win->frame_width || height != Tmp_win->frame_height)
	Tmp_win->zoomed = ZOOM_NONE;

    /* Workaround for Java 1.4 bug that freezes the application whenever
     * a new window is displayed. (When UsePPosition is on and either
     * UseThreeDBorders or BorderWidth 0 is set.)
     */
    if (!bw)
        sendEvent = True; 

    /*
     * SetupWindow (x,y) are the location of the upper-left outer corner and
     * are passed directly to XMoveResizeWindow (frame).  The (width,height)
     * are the inner size of the frame.  The inner width is the same as the 
     * requested client window width; the inner height is the same as the
     * requested client window height plus any title bar slop.
     */
    SetupFrame (Tmp_win, x, y, width, height, bw, sendEvent);
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleShapeNotify - shape notification event handler
 *
 ***********************************************************************
 */
void HandleShapeNotify (void)
{
    XShapeEvent	    *sev = (XShapeEvent *) &Event;

    if (Tmp_win == NULL)
	return;
    if (sev->kind != ShapeBounding)
	return;
    if (!Tmp_win->wShaped && sev->shaped) {
	XShapeCombineMask (dpy, Tmp_win->frame, ShapeClip, 0, 0, None,
			   ShapeSet);
    }
    Tmp_win->wShaped = sev->shaped;
    SetFrameShape (Tmp_win);
}



/***********************************************************************
 *
 *  Procedure:
 *	HandleUnknown - unknown event handler
 *
 ***********************************************************************
 */

void HandleUnknown(void)
{
#ifdef DEBUG_EVENTS
    fprintf(stderr, "type = %d\n", Event.type);
#endif
}



/***********************************************************************
 *
 *  Procedure:
 *	Transient - checks to see if the window is a transient
 *
 *  Returned Value:
 *	TRUE	- window is a transient
 *	FALSE	- window is not a transient
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************
 */

int Transient(Window w, Window *propw)
{
    return (XGetTransientForHint(dpy, w, propw));
}



/***********************************************************************
 *
 *  Procedure:
 *	FindScreenInfo - get ScreenInfo struct associated with a given window
 *
 *  Returned Value:
 *	ScreenInfo struct
 *
 *  Inputs:
 *	w	- the window
 *
 ***********************************************************************
 */

ScreenInfo *FindScreenInfo(Window w)
{
    XWindowAttributes attr;
    int scrnum;

    attr.screen = NULL;
    if (XGetWindowAttributes(dpy, w, &attr)) {
	for (scrnum = 0; scrnum < NumScreens; scrnum++) {
	    if (ScreenList[scrnum] != NULL &&
		(ScreenOfDisplay(dpy, ScreenList[scrnum]->screen) ==
		 attr.screen))
	      return ScreenList[scrnum];
	}
    }

    return NULL;
}



static void flush_expose (Window w)
{
    XEvent dummy;

				/* SUPPRESS 530 */
    while (XCheckTypedWindowEvent (dpy, w, Expose, &dummy)) ;
}



/***********************************************************************
 *
 *  Procedure:
 *	InstallWindowColormaps - install the colormaps for one twm window
 *
 *  Inputs:
 *	type	- type of event that caused the installation
 *	tmp	- for a subset of event types, the address of the
 *		  window structure, whose colormaps are to be installed.
 *
 ***********************************************************************
 */

int InstallWindowColormaps (int type, TwmWindow *tmp)
{
    if (tmp) {
	return InstallColormaps (type, &tmp->cmaps);
    } else {
	return InstallColormaps (type, NULL);
    }
}

int InstallColormaps (int type, Colormaps *cmaps)
{
    int i, j, n, number_cwins, state;
    ColormapWindow **cwins, *cwin, **maxcwin = NULL;
    TwmColormap *cmap;
    char *row, *scoreboard;

    switch (type) {
    case EnterNotify:
    case LeaveNotify:
    case DestroyNotify:
    default:
	/* Save the colormap to be loaded for when force loading of
	 * root colormap(s) ends.
	 */
	Scr->cmapInfo.pushed_cmaps = cmaps;
	/* Don't load any new colormap if root colormap(s) has been
	 * force loaded.
	 */
	if (Scr->cmapInfo.root_pushes)
	    return (0);
	/* Don't reload the current window colormap list.
	if (Scr->cmapInfo.cmaps == cmaps)
	    return (0);
	 */
	if (Scr->cmapInfo.cmaps) {
	    for (i = Scr->cmapInfo.cmaps->number_cwins,
		 cwins = Scr->cmapInfo.cmaps->cwins; i-- > 0; cwins++) {
		(*cwins)->colormap->state &= ~CM_INSTALLABLE;
	    }
	}
	Scr->cmapInfo.cmaps = cmaps;
	break;
    
    case PropertyNotify:
    case VisibilityNotify:
    case ColormapNotify:
	break;
    }

    number_cwins = Scr->cmapInfo.cmaps->number_cwins;
    cwins = Scr->cmapInfo.cmaps->cwins;
    scoreboard = Scr->cmapInfo.cmaps->scoreboard;

    ColortableThrashing = FALSE; /* in case installation aborted */

    state = CM_INSTALLED;

      for (i = n = 0; i < number_cwins; i++) {
	cwins[i]->colormap->state &= ~CM_INSTALL;
      }
      for (i = n = 0; i < number_cwins && n < Scr->cmapInfo.maxCmaps; i++) {
	cwin = cwins[i];
	cmap = cwin->colormap;
	if (cmap->state & CM_INSTALL) continue;
	cmap->state |= CM_INSTALLABLE;
	cmap->w = cwin->w;
	if (cwin->visibility != VisibilityFullyObscured) {
	    row = scoreboard + (i*(i-1)/2);
	    for (j = 0; j < i; j++)
		if (row[j] && (cwins[j]->colormap->state & CM_INSTALL))
		    break;
	    if (j != i) continue;
	    n++;
	    maxcwin = &cwins[i];
	    state &= (cmap->state & CM_INSTALLED);
	    cmap->state |= CM_INSTALL;
	}
    }
    Scr->cmapInfo.first_req = NextRequest(dpy);

    for ( ; n > 0 && maxcwin >= &cwins[0]; maxcwin--) {
	cmap = (*maxcwin)->colormap;
	if (cmap->state & CM_INSTALL) {
	    cmap->state &= ~CM_INSTALL;
	    if (!(state & CM_INSTALLED)) {
		cmap->install_req = NextRequest(dpy);
		/* printf ("XInstallColormap : %x, %x\n", cmap, cmap->c); */
		XInstallColormap(dpy, cmap->c);
	    }
	    cmap->state |= CM_INSTALLED;
	    n--;
	}
    }
    return (1);
}



/***********************************************************************
 *
 *  Procedures:
 *	<Uni/I>nstallRootColormap - Force (un)loads root colormap(s)
 *
 *	   These matching routines provide a mechanism to insure that
 *	   the root colormap(s) is installed during operations like
 *	   rubber banding or menu display that require colors from
 *	   that colormap.  Calls may be nested arbitrarily deeply,
 *	   as long as there is one UninstallRootColormap call per
 *	   InstallRootColormap call.
 *
 *	   The final UninstallRootColormap will cause the colormap list
 *	   which would otherwise have be loaded to be loaded, unless
 *	   Enter or Leave Notify events are queued, indicating some
 *	   other colormap list would potentially be loaded anyway.
 ***********************************************************************
 */

void InstallRootColormap(void)
{
    Colormaps *tmp;
    if (Scr->cmapInfo.root_pushes == 0) {
	/*
	 * The saving and restoring of cmapInfo.pushed_window here
	 * is a slimy way to remember the actual pushed list and
	 * not that of the root window.
	 */
	tmp = Scr->cmapInfo.pushed_cmaps;
	InstallColormaps(0, &Scr->RootColormaps);
	Scr->cmapInfo.pushed_cmaps = tmp;
    }
    Scr->cmapInfo.root_pushes++;
}



/* ARGSUSED*/
static Bool UninstallRootColormapQScanner(Display *display, XEvent *ev, char *args)
{
    if (!*args) {
	if (ev->type == EnterNotify) {
	    if (ev->xcrossing.mode != NotifyGrab)
		*args = 1;
	} else if (ev->type == LeaveNotify) {
	    if (ev->xcrossing.mode == NotifyNormal)
		*args = 1;
	}
    }

    return (False);
}



void UninstallRootColormap(void)
{
    char args;
    XEvent dummy;

    if (Scr->cmapInfo.root_pushes)
	Scr->cmapInfo.root_pushes--;
    
    if (!Scr->cmapInfo.root_pushes) {
	/*
	 * If we have subsequent Enter or Leave Notify events,
	 * we can skip the reload of pushed colormaps.
	 */
	XSync (dpy, 0);
	args = 0;
	(void) XCheckIfEvent(dpy, &dummy, UninstallRootColormapQScanner, &args);

	if (!args)
	    InstallColormaps(0, Scr->cmapInfo.pushed_cmaps);
    }
}

void ConfigureRootWindow (XEvent *ev)
{
    Window       root, child;
    int          x, y;
    unsigned int w, h, bw, d, oldw, oldh;

    XGetGeometry (dpy, Scr->CaptiveRoot, &root, &x, &y, &w, &h, &bw, &d);
    XTranslateCoordinates (dpy, Scr->CaptiveRoot, root, 0, 0, &Scr->crootx, &Scr->crooty, &child);

    oldw = Scr->crootw;
    oldh = Scr->crooth;
    Scr->crootw = ev->xconfigure.width;
    Scr->crooth = ev->xconfigure.height;
    /*
    fprintf (stderr, "ConfigureRootWindow: cx = %d, cy = %d, cw = %d, ch = %d\n",
	     Scr->crootx, Scr->crooty, Scr->crootw, Scr->crooth);
    */
    if (Scr->currentvs) {
      Scr->rootx = Scr->crootx + Scr->currentvs->x;
      Scr->rooty = Scr->crooty + Scr->currentvs->y;
    }
    Scr->rootw = Scr->crootw;
    Scr->rooth = Scr->crooth;

    if (captive && ((Scr->crootw != oldw) || (Scr->crooth != oldh))) {
      twmrc_error_prefix ();
      fprintf (stderr, "You cannot change root window geometry with virtual screens active,\n");
      fprintf (stderr, "from now on, the ctwm behaviour is unpredictable.\n");
    }
}

static void dumpevent (XEvent *e)
{
    char *name = "Unknown event";

    if (! tracefile) return;
    switch (e->type) {
      case KeyPress:  name = "KeyPress"; break;
      case KeyRelease:  name = "KeyRelease"; break;
      case ButtonPress:  name = "ButtonPress"; break;
      case ButtonRelease:  name = "ButtonRelease"; break;
      case MotionNotify:  name = "MotionNotify"; break;
      case EnterNotify:  name = "EnterNotify"; break;
      case LeaveNotify:  name = "LeaveNotify"; break;
      case FocusIn:  name = "FocusIn"; break;
      case FocusOut:  name = "FocusOut"; break;
      case KeymapNotify:  name = "KeymapNotify"; break;
      case Expose:  name = "Expose"; break;
      case GraphicsExpose:  name = "GraphicsExpose"; break;
      case NoExpose:  name = "NoExpose"; break;
      case VisibilityNotify:  name = "VisibilityNotify"; break;
      case CreateNotify:  name = "CreateNotify"; break;
      case DestroyNotify:  name = "DestroyNotify"; break;
      case UnmapNotify:  name = "UnmapNotify"; break;
      case MapNotify:  name = "MapNotify"; break;
      case MapRequest:  name = "MapRequest"; break;
      case ReparentNotify:  name = "ReparentNotify"; break;
      case ConfigureNotify:  name = "ConfigureNotify"; break;
      case ConfigureRequest:  name = "ConfigureRequest"; break;
      case GravityNotify:  name = "GravityNotify"; break;
      case ResizeRequest:  name = "ResizeRequest"; break;
      case CirculateNotify:  name = "CirculateNotify"; break;
      case CirculateRequest:  name = "CirculateRequest"; break;
      case PropertyNotify:  name = "PropertyNotify"; break;
      case SelectionClear:  name = "SelectionClear"; break;
      case SelectionRequest:  name = "SelectionRequest"; break;
      case SelectionNotify:  name = "SelectionNotify"; break;
      case ColormapNotify:  name = "ColormapNotify"; break;
      case ClientMessage:  name = "ClientMessage"; break;
      case MappingNotify:  name = "MappingNotify"; break;
    }
    fprintf (tracefile, "event:  %s in window 0x%x\n", name,
	     (unsigned int)e->xany.window);
    switch (e->type) {
      case KeyPress:
      case KeyRelease:
	  fprintf (tracefile, "     :  +%d,+%d (+%d,+%d)  state=%d, keycode=%d\n",
		   e->xkey.x, e->xkey.y,
		   e->xkey.x_root, e->xkey.y_root,
		   e->xkey.state, e->xkey.keycode);
	  break;
      case ButtonPress:
      case ButtonRelease:
	  fprintf (tracefile, "     :  +%d,+%d (+%d,+%d)  state=%d, button=%d\n",
		   e->xbutton.x, e->xbutton.y,
		   e->xbutton.x_root, e->xbutton.y_root,
		   e->xbutton.state, e->xbutton.button);
	  break;
    }
}
