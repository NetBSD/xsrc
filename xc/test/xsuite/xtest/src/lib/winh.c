/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: winh.c,v 1.17 94/04/17 21:01:08 rws Exp $
 */
/*LINTLIBRARY*/

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	<stdlib.h>
#include	<stdio.h>

/*
 * used when weeding
 */
#define	WEED_MARKED	(1<<0)	/* matches with an expected/delivered event */
#define	WEED_IGNORE	(1<<1)	/* ignore this event */

#define	FIRSTBORN(winh)	((winh)->prevsibling == (Winh *) NULL)

#define	ADD_TO_EXPECTED_GLOBAL(e)	\
	if ((winh_qexp = addto(winh_qexp, (e))) == (Winhe *) NULL)\
		return(1);\
	expected_events++

/*
 * This macro is called to add something to a node's expected list.
 */
#define	ADD_TO_EXPECTED(c, w, e)	\
	{\
		Display	*tmp_display;\
		Window tmp_window;\
		/* fudge display and window */\
		tmp_display = (e)->xany.display;\
		(e)->xany.display = (c)->display;\
		tmp_window = (e)->xany.window;\
		if (tmp_window == WINH_BAD)\
			(e)->xany.window = (w)->window;\
		if (((w)->expected = addto((w)->expected, (e))) == (Winhe *) NULL)\
			return(1);\
		debug(4, "Client 0x%x expecting %s on window 0x%x",\
			(c)->display,\
			eventname((e)->type),\
			(w)->window);\
		(e)->xany.display = tmp_display;\
		(e)->xany.window = tmp_window;\
	}\
	ADD_TO_EXPECTED_GLOBAL((e))

/*
 * Event characteristics
 */
#define	EC_NONE		(0)	/* nothing worth mentioning */
#define	EC_PROPS	(1<<0)	/* propagation occurs naturally */
#define	EC_ALL		(1<<1)	/* sent to all clients */

/*
 * Table of per-event information.
 *
 * The information in this table is not modified by the winh routines.
 */
static	struct {
	int	type;
	long	mask;	/* NoEventMask if none */
	long	info;
} event_info[] = {
	{ KeyPress,		KeyPressMask,		EC_PROPS },
	{ KeyRelease,		KeyReleaseMask,		EC_PROPS },
	{ ButtonPress,		ButtonPressMask,	EC_PROPS },
	{ ButtonRelease,	ButtonReleaseMask,	EC_PROPS },
	{ MotionNotify,		PointerMotionMask|
				PointerMotionHintMask|
				Button1MotionMask|
				Button2MotionMask|
				Button3MotionMask|
				Button4MotionMask|
				Button5MotionMask|
				ButtonMotionMask,	EC_PROPS },
	{ EnterNotify,		EnterWindowMask,	EC_NONE },
	{ LeaveNotify,		LeaveWindowMask,	EC_NONE },
	{ FocusIn,		FocusChangeMask,	EC_NONE },
	{ FocusOut,		FocusChangeMask,	EC_NONE },
	{ KeymapNotify,		KeymapStateMask,	EC_NONE },
	{ Expose,		ExposureMask,		EC_NONE },
	{ GraphicsExpose,	NoEventMask,		EC_NONE },
	{ NoExpose,		NoEventMask,		EC_NONE },
	{ VisibilityNotify,	VisibilityChangeMask,	EC_NONE },
	{ CreateNotify,		SubstructureNotifyMask,	EC_NONE },
	{ DestroyNotify,	SubstructureNotifyMask,	EC_NONE },
	{ UnmapNotify,		SubstructureNotifyMask,	EC_NONE },
	{ MapNotify,		SubstructureNotifyMask,	EC_NONE },
	{ MapRequest,		SubstructureRedirectMask,EC_NONE },
	{ ReparentNotify,	SubstructureNotifyMask,	EC_NONE },
	{ ConfigureNotify,	SubstructureNotifyMask,	EC_NONE },
	{ ConfigureRequest,	SubstructureRedirectMask,EC_NONE },
	{ GravityNotify,	SubstructureNotifyMask,	EC_NONE },
	{ ResizeRequest,	ResizeRedirectMask,	EC_NONE },
	{ CirculateNotify,	SubstructureNotifyMask,	EC_NONE },
	{ CirculateRequest,	SubstructureRedirectMask,EC_NONE },
	{ PropertyNotify,	PropertyChangeMask,	EC_NONE },
	{ SelectionClear,	NoEventMask,		EC_NONE },
	{ SelectionRequest,	NoEventMask,		EC_NONE },
	{ SelectionNotify,	NoEventMask,		EC_NONE },
	{ ColormapNotify,	ColormapChangeMask,	EC_NONE },
	{ ClientMessage,	NoEventMask,		EC_NONE },
	{ MappingNotify,	NoEventMask,		EC_ALL }
};

/*
 * Used to track event statistics by event type
 *
 * The information in this table is modified by the winh routines.
 *
 * There is a one-to-one correspondence between members of this array
 * and members of the event_info array.
 */
Winhs	winh_event_stats[NELEM(event_info)];

/*
 * Used to maintain sequencing information for events in hierarchy event queue.
 * Also used to track number of events in hierarchy event queue.
 */
static	int	sequence = 0;

/*
 * Used to track number of expected events.
 */
static	int	expected_events = 0;

Winhe	*winh_qexp = (Winhe *) NULL;	/* list of expected events */
Winhe	*winh_qdel = (Winhe *) NULL;	/* list of delivered events */

Winh	*guardian = (Winh *) NULL;

/*
 * static globals used to communicate between winh_walk() procedures and
 * the routines which call winh_walk().
 */
static	Display	*_display_;
static	Window	_window_;
static	Winh	*_foundit_;
static	XEvent	*_event_;
static	int	_winh_walk_first_time_;
static	long	_event_mask_;
static	long	_winhmask_;
static	int	_event_type_;
static	int	_eindex_;		/* index into event arrays */

static	char	*winhmalloc();
static	void	add_child(), add_sibling();
static	Winh	*getguardian(), *initguardian();
static	Winhe	*addto();
static	int 	free_eventlist();
static	int 	winh_print();

static	int	_winh_create();
static	int	_winh_find();
static	int	_winh_free();
static	int	_winh_ignore_event();
static	int	_winh_plant();
static	int	_winh_selectinput();
static	int	_winh_weed();
static	int	_winh_walk_depth();
static	int	_winh_walk();
static	int	_winh();

/*
 * winh_adopt -	add to window hierarchy
 */
Winh *
winh_adopt(display, parent, valuemask, attributes, winhg, winhmask)
Display	*display;
Winh	*parent;
unsigned long valuemask;
XSetWindowAttributes *attributes;
Winhg	*winhg;
long	winhmask;
{
	Winh	*child;

	if (winhmask & WINH_GUARDIAN) {
		delete("Invalid mask in winh_adopt(): WINH_GUARDIAN");
		return((Winh *) NULL);
	}
	if (guardian == (Winh *) NULL)
		guardian = initguardian(display);
	if (guardian == (Winh *) NULL)
		return((Winh *) NULL);
	if (parent == (Winh *) NULL)
		parent = getguardian(display);
	child = (Winh *) winhmalloc(sizeof(*child), "winh_adopt");
	if (child == (Winh *) NULL)
		return((Winh *) NULL);
	child->window = WINH_BAD;
	add_child(parent, child);
	if (winhmask & WINH_INHERIT) {
		child->valuemask  = parent->valuemask;
		child->attributes  = parent->attributes;
	}
	else if (attributes == (XSetWindowAttributes *) NULL) {
		child->valuemask = 0;
	}
	else {
		child->valuemask = valuemask;
		child->attributes = *attributes;
	}
	child->winhmask = winhmask;
	child->clients = (Winhc *) NULL;
	child->expected = (Winhe *) NULL;
	child->delivered = (Winhe *) NULL;
	if (!(winhmask & WINH_IGNORE_GEOMETRY) && winhg == (Winhg *) NULL) {
		/*
		 * dynamically generate geometries
		 *
		 *	dimensions one fourth minus a little bit of parent
		 *	tile window on parent window with
		 *		1st child upper left,
		 *		2nd child upper right,
		 *		3rd child lower left,
		 *		4th child lower right, and
		 *		remaining centered
		 */
		child->winhg = child->parent->winhg;
		child->winhg.area.width /= 2;
		child->winhg.area.width -= 6;
		child->winhg.area.height /= 2;
		child->winhg.area.height -= 6;
		child->winhg.area.x = 2;
		child->winhg.area.y = 2;
		switch (child->parent->numchildren) {
			case 1:
				break;
			case 2:
				child->winhg.area.x = child->winhg.area.width + 6;
				break;
			case 4:
				child->winhg.area.x = child->winhg.area.width + 6;
				/*FALLTHROUGH*/
			case 3:
				child->winhg.area.y = child->winhg.area.height + 6;
				break;
			default:
				child->winhg.area.x = child->parent->winhg.area.width / 2;
				child->winhg.area.x -= child->winhg.area.width/2;
				child->winhg.area.y = child->parent->winhg.area.height / 2;
				child->winhg.area.y -= child->winhg.area.height/2;
		}
	}
	else if (!(winhmask & WINH_IGNORE_GEOMETRY))
		child->winhg = *winhg;
	return(child);
}

/*
 * winh_create -	create windows in hierarchy
 */
int
winh_create(display, winh, winhmask)
Display	*display;
Winh	*winh;
long	winhmask;
{
	_display_ = display;	/* save in global... */
	_winhmask_ = winhmask;
	return(winh_walk(winh, 0, _winh_create));
}

static int
_winh_create(winh)
Winh	*winh;
{
	struct area	*ap;
	int	border_width;

	/* ignore guardians, they don't "have" windows */
	/* don't create a window which is marked as created */
	if (winh->winhmask & WINH_CREATED)
		return(0);
	if (winh->winhmask & WINH_IGNORE_GEOMETRY) {
		ap = (struct area *) NULL;
		border_width = 1;
	}
	else {
		ap = &(winh->winhg.area);
		border_width = winh->winhg.border_width;
	}
	winh->window = mkwinchild(_display_, (XVisualInfo *) NULL, ap, 0, winh->parent->window, border_width);
	if (winh->valuemask)
		XChangeWindowAttributes(_display_, winh->window, winh->valuemask, &(winh->attributes));
	if ((winh->winhmask | _winhmask_) & WINH_MAP)
		XMapWindow(_display_, winh->window);
	winh->winhmask |= WINH_CREATED;
	return(0);
}

/*
 * winh_find -	locate node for window in hierarchy
 */
Winh *
winh_find(winh, window)
Winh	*winh;
Window	window;
{
	_window_ = window;
	if (!winh_walk(winh, 0, _winh_find))
		return((Winh *) NULL);
	return(_foundit_);
}

static	int
_winh_find(winh)
Winh	*winh;
{
	if (winh->window == _window_) {
		_foundit_ = winh;
		return(1);
	}
	return(0);
}

/*
 * winh_plant -	plant events where they belong in heirarchy
 *
 *	o free and re-initialize the hierarchy event queue data
 *	o initialize the high, low, and sequence counts for each event type
 */
int
winh_plant(source, event, event_mask, winhmask)
Winh	*source;
XEvent	*event;
long	event_mask;
long	winhmask;
{
	Winh	*winh;

	if (winh_qdel != (Winhe *) NULL)
		(void) free_eventlist();
	if (event == (XEvent *) NULL)
		return(0);
	if ((_eindex_ = winh_eventindex(event->type)) == -1)
		return(1);
	if (event_mask == NoEventMask)
		event_mask = event_info[_eindex_].mask;
	if (source == (Winh *) NULL) {
		/* deliver event to all windows in hierarchy */
		_event_ = event;
		_event_mask_ = event_mask;
		return(winh_walk((Winh *) NULL, 0, _winh_plant));
	}
	/* search for event window */
	for (winh = source; winh != (Winh *) NULL; winh = winh->parent) {
		int	found = 0;
		Winhc	*winhc;

		/* search for selecting clients */
		for (winhc = winh->clients; winhc != (Winhc *) NULL; winhc = winhc->next) {
			if (winhc->event_mask & event_mask) {
				found++;
				ADD_TO_EXPECTED(winhc, winh, event);
			}
		}
		if (found)
			break;	/* this is the event window */
		if (!(winhmask & WINH_DEL_PROPOGATE) &&
		    !(event_info[_eindex_].info & EC_PROPS))
			break;	/* there is no propagation */
		if (winh->valuemask & CWDontPropagate &&
		    (winh->attributes.do_not_propagate_mask & event_mask))
			break;	/* propagation of this event turned off */
	}
	return(0);
}

static	int
_winh_plant(winh)
Winh	*winh;
{
	long	emask;
	Winhc	*winhc;

	emask = event_info[_eindex_].mask;
	for (winhc = winh->clients; winhc != (Winhc *) NULL; winhc = winhc->next) {
		/* add to expected lists if selected */
		if (_event_mask_ == NoEventMask ||
		    emask == NoEventMask ||
		    (winhc->event_mask & _event_mask_)) {
			ADD_TO_EXPECTED(winhc, winh, _event_);
		}
	}
	return(0);
}

/*
 * addto -	add event to end of specified Winhe list
 *		returns pointer to beginning of list
 */
static	Winhe *
addto(winhe, event)
Winhe	*winhe;
XEvent	*event;
{
	Winhe	*head = winhe;
	Winhe	*last;

	/* locate end of list */
	if (winhe == (Winhe *) NULL)
		last = (Winhe *) NULL;
	else {
		while (winhe->next != (Winhe *) NULL)
			winhe = winhe->next;
		last = winhe;
	}
	/* allocate memory for new member */
	winhe = (Winhe *) winhmalloc(sizeof(*winhe), "addto");
	if (winhe == (Winhe *) NULL)
		return((Winhe *) NULL);
	winhe->event = (XEvent *) winhmalloc(sizeof(*(winhe->event)), "addto");
	if (winhe->event == (XEvent *) NULL) {
		free(winhe);
		return((Winhe *) NULL);
	}
	*(winhe->event) = *event;
	winhe->sequence = sequence;
	winhe->flags = 0;
	winhe->next = (Winhe *) NULL;
	if (head == (Winhe *) NULL)
		head = winhe;
	if (last != (Winhe *) NULL)
		last->next = winhe;
	return(head);
}

/*
 * winh_pending -	return number of events in heirarchy event queue
 */
int
winh_pending(expected)
int	expected;
{
	return(expected ? expected_events : sequence);
}

/*
 * winh_harvest -	move events from event queue into heirarchy
 *
 * ASSUMES:	Caller has XSync'd as is necessary.
 *
 *	o move all events in event queue to internal event queue.
 *	o link up these events in delivered lists
 *	o maintain event sequence information
 *	o maintain high and low sequence counts for each event type
 *	o maintain event count statistics
 */
int
winh_harvest(display, winh)
Display	*display;
Winh	*winh;
{
	int	i;
	int	status = 0;
	Winh	*prev = (Winh *) NULL;
	Window	prevwindow = WINH_BAD;

	while (XPending(display) > 0) {
		for (i = XPending(display); i > 0; i--) {
			Winh	*ptr;
			XEvent	event_return;
			int	j;

			sequence++;
			XNextEvent(display, &event_return);
			/*
			 * Update event statistics
			 */
			j = winh_eventindex(event_return.type);
			if (j == -1)
				return(1);
			winh_event_stats[j].high = sequence;
			if (!winh_event_stats[j].count)
				winh_event_stats[j].low = sequence;
			winh_event_stats[j].count++;
			if (event_return.type == KeymapNotify) {
				if (prev == (Winh *) NULL) {
					delete("KeymapNotify without preceeding EnterNotify or FocusIn event");
					status = 1;
					continue;
				}
				ptr = prev;
				event_return.xany.window = prevwindow;
			}
			else
			{
				ptr = winh_find(winh, event_return.xany.window);
				if (ptr == (Winh *) NULL) {
					report("Event type %s received on window (0x%x) outside of specified hierarchy", eventname(event_return.type), event_return.xany.window);
					delete("Event received on unsupported window");
					return(1);
				}
				prev = ptr;
			}
			if ((winh_qdel = addto(winh_qdel, &event_return)) == (Winhe *) NULL)
				return(1);
			if ((ptr->delivered = addto(ptr->delivered, &event_return)) == (Winhe *) NULL)
				return(1);
			prevwindow = event_return.xany.window;
		}
	}
	return(status);
}

/*
 * winh_weed -	check heirarchy for undesirable or missing events
 */
int
winh_weed(winh, event_type, winhmask)
Winh	*winh;
int	event_type;
long	winhmask;
{
	_winhmask_ = winhmask;
	_event_type_ = event_type;
	return(winh_walk(winh, 0, _winh_weed));
}

static	int
_winh_weed(winh)
Winh	*winh;
{
	int	status = 0;

	if (_winhmask_ & WINH_WEED_IDENTITY) {
		Winhe	*e = winh->expected;

		/* for each member in the expected list... */
		for (; e != (Winhe *) NULL; e = e->next) {
			Winhe	*d = winh->delivered;

			/* search for a matching member in delivered list */
			for (; d != (Winhe *) NULL; d = d->next) {
				if (e->event->type == d->event->type &&
				    e->event->xany.window == d->event->xany.window &&
				    e->event->xany.display == d->event->xany.display) {
					if (!(d->flags & (WEED_MARKED|WEED_IGNORE))) {
						d->flags |= WEED_MARKED;
						e->flags |= WEED_MARKED;
						break;
					}
					debug(4, "0x%x delivered flags: 0x%x",
						winh->window, d->flags);
				}
			}
			if (d == (Winhe *) NULL) {
				report("Expected %s event on window 0x%x from client 0x%x",
					eventname(e->event->type),
					winh->window,
					e->event->xany.display);
				status = 1;
			}
		}
		/* verify we expected all the events that were delivered */
		e = winh->delivered;
		for (; e != (Winhe *) NULL; e = e->next)
			if (!(e->flags & (WEED_MARKED|WEED_IGNORE))) {
				report("Unexpectedly delivered %s event on 0x%x window",
					eventname(e->event->type), winh->window);
				status = 1;
			}
	}
	else {
		delete("Unsupported winh_weed mask: 0x%x", _winhmask_);
		return(-1);
	}
	return(status);
}

/*
 * winh_ignore_event -	ignore an event type while weeding
 */
int
winh_ignore_event(winh, event_type, winhmask)
Winh	*winh;
int	event_type;
long	winhmask;
{
	_event_type_ = event_type;
	_winhmask_ = winhmask;
	return(winh_walk(winh, 0, _winh_ignore_event));
}

static	int
_winh_ignore_event(winh)
Winh	*winh;
{
	Winhe	*d = winh->delivered;

	/* search for all matching events in delivered list */
	for (; d != (Winhe *) NULL; d = d->next) {
		if (d->event->type == _event_type_)
			d->flags |= WEED_IGNORE;
	}
	return(0);
}

/*
 * winh_selectinput -	update hierarchy to reflect event selection
 */
int
winh_selectinput(display, winh, event_mask)
Display	*display;
Winh	*winh;
long	event_mask;
{
	_display_ = display;
	_event_mask_ = event_mask;
	if (winh == (Winh *) NULL)
		return(winh_walk((Winh *) NULL, 0, _winh_selectinput));
	return(_winh_selectinput(winh));
}

static	int
_winh_selectinput(winh)
Winh	*winh;
{
	Winhc	*cl, *last;

	if (winh == (Winh *) NULL) {
		delete("NULL winh in _winh_selectinput");
		return(-1);
	}
	if (!(winh->winhmask & WINH_CREATED)) {
		delete("Corresponding window not created in _winh_selectinput");
		return(-1);
	}
	XSelectInput(_display_, winh->window, _event_mask_);
	/* See if this client is already in client list. */
	for (cl = winh->clients; cl != (Winhc *) NULL; cl = cl->next)
		if (cl->display == _display_)
			break;
		else
			last = cl;
	if (cl == (Winhc *) NULL) {
		if (_event_mask_ == NoEventMask)
			return(0);	/* nothing of note */
		/* New client, append to client list. */
		cl = (Winhc *) winhmalloc(sizeof(*cl), "winh_selectinput");
		if (cl == (Winhc *) NULL)
			return(-1);
		if (winh->clients == (Winhc *) NULL)
			winh->clients = cl;
		else
			last->next = cl;
		cl->display = _display_;
		cl->next = (Winhc *) NULL;
		cl->node = winh;
	}
	else if (_event_mask_ == NoEventMask) {
		/* delete entry from list */
		if (cl == winh->clients) {
			/* first in list */
			winh->clients = cl->next;
		}
		else {
			/* not first in list */
			last->next = cl->next;
		}
		free(cl);
		return(0);
	}
	cl->event_mask = _event_mask_;
	return(0);
}

/*
 * winh_changewindowattributes -	update hierarchy to reflect event selection
 */
int
winh_changewindowattributes(display, winh, valuemask, attributes)
Display	*display;
Winh	*winh;
unsigned long	valuemask;
XSetWindowAttributes *attributes;
{
	if (winh == (Winh *) NULL) {
		delete("NULL winh in winh_changewindowattributes");
		return(-1);
	}
	if (!(winh->winhmask & WINH_CREATED)) {
		delete("Corresponding window not created in winh_changewindowattributes");
		return(-1);
	}
	/*
	 * redirect event_mask requests through winh_selectinput()
	 */
	if (valuemask & CWEventMask) {
		int	status;

		status = winh_selectinput(display, winh, attributes->event_mask);
		if (status)
			return(status);
		valuemask &= ~CWEventMask;
	}
	winh->valuemask |= valuemask;
	if (valuemask & CWBackPixmap)
		winh->attributes.background_pixmap = attributes->background_pixmap;
	if (valuemask & CWBackPixel)
		winh->attributes.background_pixel = attributes->background_pixel;
	if (valuemask & CWBorderPixmap)
		winh->attributes.border_pixmap = attributes->border_pixmap;
	if (valuemask & CWBorderPixel)
		winh->attributes.border_pixel = attributes->border_pixel;
	if (valuemask & CWBitGravity)
		winh->attributes.bit_gravity = attributes->bit_gravity;
	if (valuemask & CWWinGravity)
		winh->attributes.win_gravity = attributes->win_gravity;
	if (valuemask & CWBackingStore)
		winh->attributes.backing_store = attributes->backing_store;
	if (valuemask & CWBackingPlanes)
		winh->attributes.backing_planes = attributes->backing_planes;
	if (valuemask & CWBackingPixel)
		winh->attributes.backing_pixel = attributes->backing_pixel;
	if (valuemask & CWOverrideRedirect)
		winh->attributes.override_redirect = attributes->override_redirect;
	if (valuemask & CWSaveUnder)
		winh->attributes.save_under = attributes->save_under;
	if (valuemask & CWDontPropagate)
		winh->attributes.do_not_propagate_mask = attributes->do_not_propagate_mask;
	if (valuemask & CWColormap)
		winh->attributes.colormap = attributes->colormap;
	if (valuemask & CWCursor)
		winh->attributes.cursor = attributes->cursor;
	XChangeWindowAttributes(display, winh->window, winh->valuemask, &(winh->attributes));
	return(0);
}

/*
 * getguardian -	return guardian corresponding to specified display
 */
static	Winh *
getguardian(display)
Display	*display;
{
	Winh	*winh;

	if (guardian == (Winh *) NULL) {
		delete("Uninitialized guardian in getguardian()");
		return((Winh *) NULL);
	}
	for (winh = guardian; winh != (Winh *) NULL; winh = winh->nextsibling)
		if (winh->screen == DefaultScreen(display))
			return(winh);
	return((Winh *) NULL);
}

/*
 * initguardian -	return an initialize guardian
 *			consult config.alt_screen
 */
static	Winh *
initguardian(display)
Display	*display;
{
	Winh	*winh;
	Winh	*awinh;

	(void) free_eventlist();
	if ((winh = (Winh *) winhmalloc(sizeof(*winh), "initguardian")) == (Winh *) NULL)
		return((Winh *) NULL);
	winh->window = DRW(display);
	winh->parent = (Winh *) NULL;
	winh->nextsibling = (Winh *) NULL;
	winh->prevsibling = (Winh *) NULL;
	winh->firstchild = (Winh *) NULL;
	winh->numchildren = 0;
	winh->valuemask = 0;
	winh->winhmask = WINH_GUARDIAN | WINH_IGNORE_GEOMETRY | WINH_CREATED;
	winh->clients = (Winhc *) NULL;
	winh->expected = (Winhe *) NULL;
	winh->delivered = (Winhe *) NULL;
	winh->depth = 0;
	winh->screen = DefaultScreen(display);
	winh->winhg.border_width = 1;
	winh->winhg.area.x = 2;
	winh->winhg.area.y = 2;
	winh->winhg.area.width = DisplayWidth(display, winh->screen);
	winh->winhg.area.height = DisplayHeight(display, winh->screen);
	regid(display, (union regtypes *) &winh, REG_WINH);
	/* determine whether or not an alternate screen is supported */
	if (config.alt_screen == -1 || config.alt_screen == winh->screen)
		return(winh);
	if ((awinh = (Winh *) winhmalloc(sizeof(*awinh), "initguardian")) == (Winh *) NULL)
		return((Winh *) NULL);
	awinh->screen = config.alt_screen;
	awinh->window = RootWindow(display, awinh->screen);
	awinh->parent = (Winh *) NULL;
	awinh->nextsibling = (Winh *) NULL;
	awinh->prevsibling = (Winh *) NULL;
	awinh->firstchild = (Winh *) NULL;
	awinh->numchildren = 0;
	awinh->valuemask = 0;
	awinh->winhmask = winh->winhmask;
	awinh->clients = (Winhc *) NULL;
	awinh->expected = (Winhe *) NULL;
	awinh->delivered = (Winhe *) NULL;
	awinh->depth = 0;
	awinh->winhg.border_width = 1;
	awinh->winhg.area.x = 2;
	awinh->winhg.area.y = 2;
	awinh->winhg.area.width = DisplayWidth(display, awinh->screen);
	awinh->winhg.area.height = DisplayHeight(display, awinh->screen);
	/* link up the siblings */
	add_sibling(winh, awinh);
	return(winh);
}

/*
 * add_sibling -	add a sibling to the end of a sibling list
 */
static	void
add_sibling (sfirst, s)
Winh	*sfirst, *s;
{
	/* find last sibling in list */
	while (sfirst->nextsibling != (Winh *) NULL)
		sfirst = sfirst->nextsibling;
	sfirst->nextsibling = s;
	s->nextsibling = (Winh *) NULL;
	s->prevsibling = sfirst;
}

/*
 * add_child -	add child to parent, initializing relevant data
 */
static	void
add_child(parent, child)
Winh	*parent, *child;
{

	child->parent = parent;
	child->numchildren = 0;
	child->depth = parent->depth + 1;
	child->screen = parent->screen;
	child->firstchild = (Winh *) NULL;
	if (parent->firstchild == (Winh *) NULL) {
		parent->firstchild = child;
		child->nextsibling = (Winh *) NULL;
		child->prevsibling = (Winh *) NULL;
	}
	else
		add_sibling(parent->firstchild, child);
	parent->numchildren++;
}

/*
 * winhmalloc - memory allocation occurs here
 */
static	char *
winhmalloc(bytes, msg)
unsigned int bytes;
char	*msg;
{
	char	*new;

	new = (char *) malloc(bytes);
	if (new == (char *) NULL) {
		report("Memory allocation failed in %s: %d bytes", msg, bytes);
		delete("malloc failed in winh routines");
	}
	return(new);
}

/*
 * winh_free -	free winh resources by walking hierarchy calling _winh_free
 */
void
winh_free(winh)
Winh *winh;
{
	(void) free_eventlist();
	if (winh == (Winh *) NULL)
		winh = guardian;
	if (winh == (Winh *) NULL)
		return;
	(void) winh_walk(winh, 1, _winh_free);
	guardian = (Winh *) NULL;
}

static int
_winh_free(winh)
Winh *winh;
{
	Winhc	*cl;

	/* free the client list */
	for (cl = winh->clients; cl != (Winhc *) NULL;) {
		Winhc	*next = cl->next;

		free(cl);
		cl = next;
	}
	/* free this node */
	free(winh);
	return(0);
}

/*
 * winhe_free -	free members in specified Winhe list
 */
static	void
winhe_free(winhe)
Winhe	*winhe;
{
	while (winhe != (Winhe *) NULL) {
		Winhe	*next = winhe->next;

		free(winhe->event);
		free(winhe);
		winhe = next;
	}
}

static	int
_free_eventlist(winh)
Winh	*winh;
{
	winhe_free(winh->expected);
	winh->expected = (Winhe *) NULL;
	winhe_free(winh->delivered);
	winh->delivered = (Winhe *) NULL;
	return(0);
}

/*
 * free all Winhe-type event lists
 */
static	int
free_eventlist()
{
	int	i;

	for (i=0; i<NELEM(winh_event_stats); i++) {
		winh_event_stats[i].high = -1;
		winh_event_stats[i].low = -1;
		winh_event_stats[i].count = 0;
	}
	/* Free global queue members */
	if (winh_qexp != (Winhe *) NULL) {
		winhe_free(winh_qexp);
		winh_qexp = (Winhe *) NULL;
	}
	if (winh_qdel != (Winhe *) NULL) {
		winhe_free(winh_qdel);
		winh_qdel = (Winhe *) NULL;
	}
	if (guardian != (Winh *) NULL &&
	    (i = winh_walk((Winh *) NULL, 0, _free_eventlist)))
		return(i);
	sequence = 0;
	expected_events = 0;
	return(0);
}

/*
 * winh_walk -	descend through the hierarchy calling procedure at each node
 *		procedure is passed the current hierarchy node
 *		depthfirst is set to specify a depth-first traversal
 *		stop walk when procedure returns non-zero or
 *		tree is completely traversed
 *
 *		returns non-zero if procedure returned non-zero, otherwise
 *		zero is returned
 *
 *		if winh is NULL, then guardian is assumed
 *		if procedure is NULL, then winh_print is assumed
 *
 *	This might be simpler if there were a parent to all guardians.
 */
int
winh_walk(winh, depthfirst, procedure)
Winh	*winh;
int	depthfirst;
int	(*procedure)();
{
	int	depth;
	extern	int	winh_print();

	if (winh == (Winh *) NULL) {
		winh = guardian;
		depth = 1;
	}
	else
		depth = 0;
	if (winh == (Winh *) NULL)
		return(-1);
	if (procedure == (int (*)()) NULL)
		procedure = winh_print;
	_winh_walk_first_time_ = 1;
	if (depthfirst)
		return(_winh_walk_depth(winh, procedure, depth));
	else
		return(_winh_walk(winh, procedure, depth));
/*NOTREACHED*/
}

/*
 * _winh_walk_depth -	do a depth-first hierarchy walk
 *
 *		take care to protect against procedure corrupting
 *		the current node.
 */
static int
_winh_walk_depth(winh, procedure, depth)
Winh	*winh;
int	(*procedure)();
int	depth;
{
	int	i;
	int	firstborn;
	Winh	*sibling;

	/* process children first */
	if (winh->firstchild != (Winh *) NULL)
		if (i=_winh_walk_depth(winh->firstchild, procedure, depth+1))
			return(i);
	/* save next sibling and firstborn status */
	firstborn = FIRSTBORN(winh);
	sibling = winh->nextsibling;
	/* process this node */
	if (i=(*procedure)(winh))
		return(i);
	/* if not first in child list then we are done */
	if (!firstborn)
		return(0);
	/* deal with siblings */
	if (depth == 0)
		return(0);
	for (winh = sibling; winh != (Winh *) NULL; winh = sibling) {
		sibling = winh->nextsibling;
		if (i=_winh_walk_depth(winh, procedure, depth+1))
			return(i);
	}
	return(0);
}

static int
_winh_walk(winh, procedure, depth)
Winh	*winh;
int	(*procedure)();
int	depth;
{
	int	i;

	/* process this node */
	if (i=(*procedure)(winh))
		return(i);
	/* ignore siblings if appropriate */
	if (depth == 0) {
		if (winh->firstchild != (Winh *) NULL)
			if (i=_winh_walk(winh->firstchild, procedure, depth+1))
				return(i);
		return(0);
	}
	/* deal with siblings */
	if (FIRSTBORN(winh)) {
		Winh	*ptr = winh->nextsibling;

		for (; ptr != (Winh *) NULL; ptr = ptr->nextsibling)
			if (i=(*procedure)(ptr))
				return(i);
		/* deal with children of each sibling */
		for (; winh != (Winh *) NULL; winh = winh->nextsibling) {
			if (winh->firstchild == (Winh *) NULL)
				continue;
			if (i=_winh_walk(winh->firstchild, procedure, depth+1))
				return(i);
		}
	}
	return(0);
}

/*
 * winh_climb -	climb from start to stop
 */
int
winh_climb(start, stop, procedure)
Winh	*start;
Winh	*stop;
int	(*procedure)();
{
	Winh	*current, *previous;

	if (start == (Winh *) NULL) {
		delete("NULL start point in winh_climb");
		return(-1);
	}
	if (stop == (Winh *) NULL) {
		delete("NULL stop point in winh_climb");
		return(-1);
	}
	current = start;
	previous = (Winh *) NULL;
	do {
		int	status;

		status = (*procedure)(start, stop, current, previous);
		if (status)
			return(status);
		previous = current;
		current = current->parent;
	} while (current != (Winh *) NULL && previous != stop);
	if (previous != stop) {
		report("winh_climb climbed from 0x%x to top without reaching 0x%x",
			start->window, stop->window);
		delete("Stop point not encountered in winh_climb");
		return(-1);
	}
	return(0);
}

/*
 * winh_print -	print data associated with specified node
 *		returns 1 on error, else returns 0
 */
static	int
winh_print(winh)
Winh	*winh;
{
	int	i;
	char	in[512];
	static	struct {
		int	value;
		char	*name;
	} winhmaskinfo[] = {
		{WINH_CREATED, "WINH_CREATED"},
		{WINH_DEL_PROPOGATE, "WINH_DEL_PROPOGATE"},
		{WINH_DEL_SEND_EVENT, "WINH_DEL_SEND_EVENT"},
		{WINH_GUARDIAN, "WINH_GUARDIAN"},
		{WINH_INHERIT, "WINH_INHERIT"},
		{WINH_MAP, "WINH_MAP"},
		{WINH_WEED_IDENTITY, "WINH_WEED_IDENTITY"},
		{WINH_WEED_MINIMUM, "WINH_WEED_MINIMUM"},
		{WINH_WEED_TYPE, "WINH_WEED_TYPE"},
		{WINH_IGNORE_GEOMETRY, "WINH_IGNORE_GEOMETRY"}
	}, attrinfo[] = {
		{CWBackPixmap, "CWBackPixmap"},
		{CWBackPixel, "CWBackPixel"},
		{CWBorderPixmap, "CWBorderPixmap"},
		{CWBorderPixel, "CWBorderPixel"},
		{CWBitGravity, "CWBitGravity"},
		{CWWinGravity, "CWWinGravity"},
		{CWBackingStore, "CWBackingStore"},
		{CWBackingPlanes, "CWBackingPlanes"},
		{CWBackingPixel, "CWBackingPixel"},
		{CWOverrideRedirect, "CWOverrideRedirect"},
		{CWSaveUnder, "CWSaveUnder"},
		{CWEventMask, "CWEventMask"},
		{CWDontPropagate, "CWDontPropagate"},
		{CWColormap, "CWColormap"},
		{CWCursor, "CWCursor"}
	};

	if (_winh_walk_first_time_) {
		_winh_walk_first_time_ = 0;
		(void) fprintf(stderr, "\n\n\n");
	}
	if (winh == (Winh *) NULL)
		return(-1);
	/* indent a tab per depth */
	for (i = 0; i < (NELEM(in)-1) && i < winh->depth; i++)
		in[i] = '\t';
	in[i] = '\0';
	(void) fprintf(stderr, "%s========================================\n",
		in);
	if (winh->window == WINH_BAD)
		(void) fprintf(stderr, "%sWindow: None",
			in);
	else
		(void) fprintf(stderr, "%sWindow: 0x%x",
			in, winh->window);
	if (winh->parent == (Winh *) NULL)
		(void) fprintf(stderr, ", Parent: None\n");
	else
		(void) fprintf(stderr, ", Parent: 0x%x\n",winh->parent->window);
	(void) fprintf(stderr, "%sFirstborn: %s, Children: %2d\n",
		in,
		(FIRSTBORN(winh) ? "Yes" : "No "), winh->numchildren);
	(void) fprintf(stderr, "%sValuemask: 0x%04x, Winhmask: 0x%04x\n",
		in,
		winh->valuemask, winh->winhmask);
	if (winh->valuemask != 0) {
		(void) fprintf(stderr, "%sValuemask strings:\n",
			in);
		for (i = 0; i<NELEM(attrinfo); i++)
			if (winh->valuemask & attrinfo[i].value)
				(void) fprintf(stderr, "%s    %s\n",
					in,
					attrinfo[i].name);
	}
	if (winh->winhmask != WINH_NOMASK) {
		(void) fprintf(stderr, "%sWinhmask strings:\n",
			in);
		for (i = 0; i<NELEM(winhmaskinfo); i++)
			if (winh->winhmask & winhmaskinfo[i].value)
				(void) fprintf(stderr, "%s    %s\n",
					in,
					winhmaskinfo[i].name);
	}
	(void) fprintf(stderr, "%sClients: %s, Expected: %s, Delivered: %s\n",
		in,
		((winh->clients == (Winhc *) NULL) ? "No " : "Yes"),
		((winh->expected == (Winhe *) NULL) ? "No " : "Yes"),
		((winh->delivered == (Winhe *) NULL) ? "No " : "Yes"));
	(void) fprintf(stderr, "%sDepth: %2d, Screen: %2d\n",
		in,
		winh->depth, winh->screen);
	if (!(winh->winhmask & WINH_IGNORE_GEOMETRY)) {
		(void) fprintf(stderr, "%s%dx%d (%d,%d) border width: %d\n",
			in,
			winh->winhg.area.width, winh->winhg.area.height,
			winh->winhg.area.x, winh->winhg.area.y,
			winh->winhg.border_width);
	}
	return(0);
}

/*
 * winh_eventindex -	return the index into the event arrays
 *			corresponding to the specified event type
 *
 *	return -1 if unrecognized event_type
 */
int
winh_eventindex(event_type)
int	event_type;
{
	int	i;

	for (i=0; i<NELEM(event_info); i++)
		if (event_info[i].type == event_type)
			return(i);
	report("Unrecognized event type: %d", event_type);
	delete("Bad event type in winh routines.");
	return(-1);
}

/*
 * winh_ordercheck -	return 0 if all before events occur before the after events
 *			return 1 if they do not
 *			return -1 on error
 */
int
winh_ordercheck(before, after)
int	before;
int after;
{
	int	ibefore, iafter;

	if (before == after) {
		report("before and after set to %s", eventname(before));
		delete("identical event types in winh_ordercheck");
		return(-1);
	}
	if ((ibefore = winh_eventindex(before)) == -1)
		return(-1);
	if ((iafter = winh_eventindex(after)) == -1)
		return(-1);
	if (winh_event_stats[ibefore].count == 0) {
		report("No %s events delivered", eventname(before));
		delete("Event ordering could not be compared due to missing events");
		return(-1);
	}
	else if (winh_event_stats[iafter ].count == 0) {
		report("No %s events delivered", eventname(after));
		delete("Event ordering could not be compared due to missing events");
		return(-1);
	}
	if (winh_event_stats[ibefore].high > winh_event_stats[iafter].low) {
		report("%s events delivered before %s events",
			eventname(after), eventname(before));
		return(1);
	}
	return(0);
}

/*
 * winh -	create a standard symmetrical window hierarchy
 */
int
winh(display, depth, winhmask)
Display	*display;
int	depth;
long	winhmask;
{
	int	status;

	debug(4, "winh(): depth %d, winhmask 0x%x", depth, winhmask);
	status = _winh(display, (Winh *) NULL, depth - 1, winhmask);
	if (status)
		return(status);
	if (winhmask & WINH_BOTH_SCREENS) {
		if (config.alt_screen != -1 &&
		    config.alt_screen != guardian->screen) {

			status = _winh(display, guardian->nextsibling, depth - 1, winhmask);
			if (status)
				return(status);
		}
	}
	return(winh_create(display, (Winh *) NULL, winhmask));
}

static	int
_winh(display, parent, depth, winhmask)
Display	*display;
Winh	*parent;
int	depth;
long	winhmask;
{
	int	i;

	if (depth < 0)
		return(0);
	for (i=0; i<4; i++) {
		Winh	*ptr;
		int	status;

		ptr = winh_adopt(display, parent, 0L, (XSetWindowAttributes *) NULL, (Winhg *) NULL, winhmask);
		if (ptr == (Winh *) NULL) {
			delete("Could not create hierarchy member (%d,%d)", depth, i);
			return(-1);
		}
		status = _winh(display, ptr, depth-1, winhmask);
		if (status)
			return(status);
	}
	return(0);
}
