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

 * Copyright 1990, 1991 UniSoft Group Limited.
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
 * $XConsortium: cnfgrwdw.m,v 1.12 94/04/17 21:02:55 rws Exp $
 */
>>TITLE XConfigureWindow CH03
void
xname(display, w, value_mask, values)
Display	*display = Dsp;
Window	w;
unsigned int 	value_mask = 0;
XWindowChanges	*values = &winchng;
>>EXTERN

/* The structure that is used as the default argument to the function. */
static XWindowChanges	winchng;

/*
 * Convenience routine that sets the 'w' argument to be simple window
 * with a background of W_FG at position (10,8) and * size 10x9.
 * The border width is 0.
 */
#define	OW_X	10
#define	OW_Y	8
#define	OW_WIDTH	23
#define	OW_HEIGHT	9

static	Window
onewin()
{
Window	base;
struct	area	area;

	base = defwin(display);
	setarea(&area, OW_X, OW_Y, OW_WIDTH, OW_HEIGHT);

	w = crechild(display, base, &area);
	XSetWindowBackground(display, w, W_FG);
	XClearWindow(display, w);

	return(base);
}

/*
 * A window with subwindows.
 */
static	char	*Tsub[] = {
	".",
	"top . (5, 5) 40x40",
	"sub1 top (1,3) 4x6",
	"sub2 top (8,3) 7x6",
	"sub3 top (3,14) 12x9",
	"sub4 top (28,10) 7x20",
	"sub5 top (14,25) 11x12",
};
#define	NTsub	(NELEM(Tsub))
#define	NTsubInf	(NTsub-1)	/* Number of inferiors excluding base */

>>ASSERTION Good A
When the
.S CWX
or the
.S CWY
bits are set in
.A value_mask ,
then a call to xname
moves the window so that the
coordinates of the upper-left outer corner of the window
are
.M x ,
.M y
relative to the origin of the parent window.
>>STRATEGY
Create test window with background of W_FG.
Set x and y.
Call xname.
Verify that window has moved on screen with checkarea().
>>CODE
Window	base;
struct	area	area;

	base = onewin();

	setarea(&area, 20, 50, OW_WIDTH, OW_HEIGHT);
	values->x = area.x;
	values->y = area.y;

	value_mask = CWX|CWY;

	XCALL;

	if (checkarea(display, base, &area, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Error in moving window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When the
.S CWWidth
or
.S CWHeight
bits are set in
.A value_mask ,
then on a call to xname the width and height of the inside
size of the window are set to
.M width
and
.M height .
>>STRATEGY
Create test window with background of W_FG.
Set width and height.
Call xname.
Verify new size on screen with checkarea().
>>CODE
Window	base;
struct	area	area;

	base = onewin();

	setarea(&area, OW_X, OW_Y, 23, 47);
	values->width = area.width;
	values->height = area.height;

	value_mask = CWWidth|CWHeight;

	XCALL;

	if (checkarea(display, base, &area, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Error in resizing window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When a call to xname resizes the window, then
the subwindows of the window are repositioned according to their win-gravity
attribute and a
.S GravityNotify
event is generated for each repositioned subwindow after the
.S ConfigureNotify
event.
>>STRATEGY
For each win-gravity attribute (apart from UnmapGravity)
  Create window with subwindows.
  Enable SubstructureNotify|StructureNotify events on all windows.
  Set win-gravity on window.
  Call xname to resize window.
  Verify window positions by pixel check.
  Verify ConfigureNotify on resized window.
  Verify ConfigureNotify on parent of resized window.
  If NorthWestGravity || StaticGravity
    Verify that no gravity events are received.
  else
    Verify GravityNotify events received on each repositioned subwindow.
    Verify gravity events are received on the parent of each subwindow.
  Verify that configure events arrive before gravity events.
>>EXTERN

extern	struct valname S_wingravity[];
extern	int 	NS_wingravity;

/* Values to resize onewin to (odd and even) */
#define	NEW_X	OW_X+5
#define	NEW_Y	OW_Y+5
#define	NEW_WIDTH	70
#define	NEW_HEIGHT	61

#define	BORDERW	1

>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*topbtp;
struct	valname 	*wgrav;
XSetWindowAttributes	setatts;
int 	i;

	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;
	value_mask = CWWidth|CWHeight;

	for (wgrav = S_wingravity; wgrav < S_wingravity+NS_wingravity; wgrav++) {

		if (wgrav->val == UnmapGravity)
			continue;

		trace("-- Testing win-gravity %s", wgrav->name);

		base = defwin(display);
		bt = buildtree(display, base, Tsub, NTsub);

		topbtp = btntobtp(bt, "top");

		/* Select for all structure events on all windows */
		for (i = 0; i < NTsub; i++) {
			XSelectInput(display, bt[i].wid,
			 	SubstructureNotifyMask|StructureNotifyMask);
		}

		/*
		 * Set the win_gravity on windows below 'top'.  Also set the
		 * borderwidths to make things more interesting, esp on mono
		 * displays.
		 */
		setatts.win_gravity = wgrav->val;
		for (i = 2; i < NTsub; i++) {
			XChangeWindowAttributes(display, bt[i].wid, CWWinGravity, &setatts);
			XSetWindowBorderWidth(display, bt[i].wid, BORDERW);
			XSetWindowBorder(display, bt[i].wid, W_BG);	/* to show up */
		}

		w = topbtp->wid;
		XCALL;

		PIXCHECK(display, base);

#ifndef GENERATE_PIXMAPS
		if (wingravevents(display, bt, wgrav->val))
			CHECK;
#endif
	}

	/*
	 * In the GENERATE_PIXMAPS case there will be a path-check error here,
	 * this is intentional. (To prove that wingravevents() is used in
	 * the verification case)
	 */
	CHECKPASS(2*(NS_wingravity-1));	/* -1 for UnmapGravity */
>>EXTERN

/*
 * Flags to say that we received particular events.
 */
#define	WIN_CONFIG	001
#define	WIN_GRAVITY	002
#define	PAR_CONFIG	004
#define	PAR_GRAVITY	010

static	void	calcxy();

/*
 * Routine that does the event checking for the gravity notify
 * test.
 */
static
wingravevents(display, bt, evtype)
Display	*display;
struct	buildtree	*bt;
int 	evtype;
{
XEvent	ev;
XConfigureEvent	confgood;
XGravityEvent	gravgood;
XConfigureEvent	*cnp;
XGravityEvent	*gnp;
struct	buildtree	*btp;
struct	buildtree	*top;
int 	gotgrav = 0;
int 	i;
int 	pass = 0, fail = 0;

	top = btntobtp(bt, "top");

	/* Set up events */
	confgood.type = ConfigureNotify;
	confgood.serial = 0L;
	confgood.send_event = False;
	confgood.display = display;
	confgood.above = None;	/* XXX */
	confgood.override_redirect = False;

	confgood.x = top->x;
	confgood.y = top->y;
	confgood.width = NEW_WIDTH;
	confgood.height = NEW_HEIGHT;
	confgood.border_width = 0;

	gravgood.type = GravityNotify;
	gravgood.serial = 0L;
	gravgood.send_event = False;
	gravgood.display = display;

	while (getevent(display, &ev) > 0) {

		switch (ev.type) {
		case ConfigureNotify:
			cnp = (XConfigureEvent*)&ev;

			btp = btwtobtp(bt, cnp->window);
			if (btp == NULL) {
				report("Event received on unknown window");
				FAIL;
				continue;
			}
			trace("Event received for window '%s'", btp->name);

			if (gotgrav) {
				report("Configure event received after gravity event");
				FAIL;
			} else
				CHECK;

			/*
			 * Work out if this event occurred on the parent or the window.
			 */
			if (cnp->window != cnp->event) {
				if (!btp->parent || btp->parent->wid != cnp->event) {
					report("Event received on other than the parent window");
					FAIL;
				} else
					btp->uflags |= PAR_CONFIG;
			} else {
				btp->uflags |= WIN_CONFIG;
			}

			confgood.event = cnp->event;
			confgood.window = cnp->window;

			if (checkevent((XEvent*)&confgood, &ev) == 0)
				CHECK;
			else
				FAIL;

			break;
		case GravityNotify:
			gnp = (XGravityEvent*)&ev;
			gotgrav = 1;

			btp = btwtobtp(bt, gnp->window);
			if (btp == NULL) {
				report("Event received on unknown window");
				FAIL;
				continue;
			}
			trace("Event received for window '%s'", btp->name);

			/*
			 * Work out if this event occurred on the parent or the window.
			 */
			if (gnp->window != gnp->event) {
				if (!btp->parent || btp->parent->wid != gnp->event) {
					report("Event received on other than the parent window");
					FAIL;
				} else
					btp->uflags |= PAR_GRAVITY;
			} else {
				btp->uflags |= WIN_GRAVITY;
			}

			gravgood.event = gnp->event;
			gravgood.window = gnp->window;
			gravgood.x = btp->x;
			gravgood.y = btp->y;
			(void) calcxy(top, evtype, &gravgood.x, &gravgood.y);

			if (checkevent((XEvent*)&gravgood, &ev) == 0)
				CHECK;
			else
				FAIL;

			break;
		default:
			report("Unexpected event type %s", eventname(ev.type));
			FAIL;
		}
	}

	/*
	 * Since the previous loop is executed a variable number of times
	 * check that it was executed at least once and reset the pass count.
	 */
	if (pass > 0)
		pass = 1;
	else
		delete("No CHECK marks in wingravevents() loop");

	/*
	 * Check correct events on the top (resized) window.
	 */
	if (top->uflags & WIN_CONFIG)
		CHECK;
	else {
		report("Configure event not received on window 'top'");
		FAIL;
	}
	if (top->uflags & PAR_CONFIG)
		CHECK;
	else {
		report("Configure event not received on parent of window 'top'");
		FAIL;
	}
	if (top->uflags & (PAR_GRAVITY|WIN_GRAVITY)) {
		report("Gravity events unexpectedly received on window 'top'");
		FAIL;
	} else
		CHECK;

	for (i = 2; i < NTsub; i++) {
		if (evtype == NorthWestGravity || evtype == StaticGravity) {
			if (bt[i].uflags & WIN_GRAVITY) {
				report("Gravity event unexpectedly received on window '%s'", bt[i].name);
				FAIL;
			} else
				CHECK;
			if (bt[i].uflags & PAR_GRAVITY) {
				report("Gravity event unexpectedly received on parent of window '%s'", bt[i].name);
				FAIL;
			} else
				CHECK;

		} else {

			if (bt[i].uflags & WIN_GRAVITY)
				CHECK;
			else {
				report("Gravity event not received on window '%s'", bt[i].name);
				FAIL;
			}
			if (bt[i].uflags & PAR_GRAVITY)
				CHECK;
			else {
				report("Gravity event not received on parent of window '%s'", bt[i].name);
				FAIL;
			}
		}
		if (bt[i].uflags & (PAR_CONFIG|WIN_CONFIG)) {
			report("Configure event unexpectedly received on window '%s'", bt[i].name);
			FAIL;
		}
	}
	if (fail == 0 && pass != 14)
		delete("Path check error in wingravevents got %d, expecting 14", pass);

	if (fail == 0)
		return(True);
	else
		return(False);
}

static void
calcxy(top, evtype, x, y)
struct	buildtree	*top;
int 	evtype;
int 	*x;
int 	*y;
{
int 	dx = NEW_WIDTH-top->width;
int 	dy = NEW_HEIGHT-top->height;

	switch (evtype) {
	case NorthWestGravity:
		break;
	case NorthGravity:
		*x += dx/2;
		break;
	case NorthEastGravity:
		*x += dx;
		break;
	case WestGravity:
		*y += dy/2;
		break;
	case CenterGravity:
		*x += dx/2;
		*y += dy/2;
		break;
	case EastGravity:
		*x += dx;
		*y += dy/2;
		break;
	case SouthWestGravity:
		*y += dy;
		break;
	case SouthGravity:
		*x += dx/2;
		*y += dy;
		break;
	case SouthEastGravity:
		*x += dx;
		*y += dy;
		break;
	case StaticGravity:
		break;
	default:
		delete("Internal error -- Unknown gravity in calcxy");
	}
}

>>ASSERTION Good A
When a call to xname resizes the window and the
win-gravity of a subwindow is
.S UnmapGravity 
and the subwindow is already mapped,
then the subwindow is unmapped without being moved and an 
.S UnmapNotify
event is generated.
>>STRATEGY
Create window with subwindows.
Enable events on all subwindows.
Enable events on window.
Set win-gravity to UnmapGravity.
Call xname to resize window.
Verify windows are removed from screen.
Verify window positions are unchanged.
Verify that UnmapNotify events received on each subwindow.
Verify that UnmapNotify events received on parent of each subwindow.
>>EXTERN

#define	ON_PARENT	0x01
#define	ON_WINDOW	0x02

>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*topbtp;
struct	buildtree	*btp;
XEvent	ev;
XUnmapEvent	good;
XUnmapEvent	*ump;
XSetWindowAttributes	setatts;
XWindowAttributes	atts;
int 	i;

	/* Set up good unmap event struct */
	good.type = UnmapNotify;
	good.serial = 0L;
	good.send_event = False;
	good.display = display;
	good.from_configure = True;

	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;
	value_mask = CWWidth|CWHeight;

	base = defwin(display);
	bt = buildtree(display, base, Tsub, NTsub);

	topbtp = btntobtp(bt, "top");

	/* Select for all structure events on all windows */
	for (i = 0; i < NTsub; i++) {
		XSelectInput(display, bt[i].wid,
			SubstructureNotifyMask|StructureNotifyMask);
	}

	/*
	 * Set the win_gravity on windows below 'top'.  Also set the
	 * borderwidths to make things more interesting, esp on mono
	 * displays.
	 */
	setatts.win_gravity = UnmapGravity;
	for (i = 2; i < NTsub; i++) {
		XChangeWindowAttributes(display, bt[i].wid, CWWinGravity, &setatts);
		XSetWindowBorderWidth(display, bt[i].wid, BORDERW);
	}

	w = topbtp->wid;
	XCALL;

	if (checkarea(display, w, (struct area *)0, W_FG, W_FG, CHECK_ALL))
		CHECK;
	else {
		report("UnmapGravity did not appear to remove subwindows");
		FAIL;
	}

	while (getevent(display, &ev) > 0) {

		if (ev.type != UnmapNotify)
			continue;

		ump = (XUnmapEvent*)&ev;

		btp = btwtobtp(bt, ump->window);
		if (btp == NULL) {
			report("Event received on unknown window");
			FAIL;
			continue;
		}
		trace("Event received for window '%s'", btp->name);

		/*
		 * Work out if this event occurred on the parent or the window.
		 */
		if (ump->window != ump->event) {
			if (!btp->parent || btp->parent->wid != ump->event) {
				report("Event received on other than the parent window");
				FAIL;
			} else
				btp->uflags |= ON_PARENT;
		} else {
			btp->uflags |= ON_WINDOW;
		}

		good.event = ump->event;
		good.window = ump->window;

		if (checkevent((XEvent*)&good, &ev) == 0)
			CHECK;
		else
			FAIL;
	}

	/*
	 * Go through the subwindows below top and check for events received
	 * and map_state and position unchanged.
	 */
	for (i = 2; i < NTsub; i++) {
		if (bt[i].uflags & ON_WINDOW)
			CHECK;
		else {
			report("Unmap event not received on window '%s'", bt[i].name);
			FAIL;
		}
		if (bt[i].uflags & ON_PARENT)
			CHECK;
		else {
			report("Unmap event not received on parent of window '%s'", bt[i].name);
			FAIL;
		}

		XGetWindowAttributes(display, bt[i].wid, &atts);
		if (atts.map_state != IsUnmapped) {
			report("map_state was %s, expecting IsUnmapped",
				mapstatename(atts.map_state));
			FAIL;
		} else
			CHECK;

		if (atts.x != bt[i].x || atts.y != bt[i].y) {
			report("Subwindow was moved after UnmapGravity used");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(1+6*(NTsub-2));
>>ASSERTION Good A
>># This assumes that the server either always does this or not.
>>#	I believe this is the case.	kieron
>># If the server is allowed to do this on some windows and not on
>># others then the assertion needs re-wording.
>>#	If it does ignore bit-gravity then it will also generate Expose
>>#	events.		kieron.
If the server uses the window's bit-gravity attribute:
When a call to xname resizes the window, then
the contents of the window are repositioned or discarded
according to the bit-gravity attribute.
Otherwise:
When a call to xname resizes the window, then the
contents of the window are discarded.
>>STRATEGY
For each value of bit-gravity.
  Create window.
  Draw into window.
  Call xname to resize window.
  Verify that either:
	Window is clear.
  else
	Contents have been repositioned correctly.
>>CODE
struct	valname	*bitgrav;
Window	base;
XSetWindowAttributes	setatts;
struct	area	area;
extern	struct	valname	S_bitgravity[];
extern	int 	NS_bitgravity;


	for (bitgrav = S_bitgravity; bitgrav < S_bitgravity+NS_bitgravity; bitgrav++) {

		trace("-- Trying bitgravity of %s", bitgrav->name);
		setarea(&area, OW_X, OW_Y, OW_WIDTH, OW_HEIGHT);

		base = onewin();
		XSetWindowBackground(display, w, W_BG);
		dset(display, w, W_FG);

		setatts.bit_gravity = bitgrav->val;
		XChangeWindowAttributes(display, w, CWBitGravity, &setatts);

		values->width = NEW_WIDTH;
		values->height = NEW_HEIGHT;
		value_mask = CWWidth|CWHeight;

		XCALL;

		switch (bitgrav->val) {
		case NorthGravity: case CenterGravity: case SouthGravity:
			area.x += (NEW_WIDTH-OW_WIDTH)/2;
			break;
		case NorthEastGravity: case EastGravity: case SouthEastGravity:
			area.x += (NEW_WIDTH-OW_WIDTH);
			break;
		}
		switch (bitgrav->val) {
		case CenterGravity: case WestGravity: case EastGravity:
			area.y += (NEW_HEIGHT-OW_HEIGHT)/2;
			break;
		case SouthGravity: case SouthEastGravity: case SouthWestGravity:
			area.y += (NEW_HEIGHT-OW_HEIGHT);
			break;
		}

		if (checkarea(display, base, (struct area *)0, W_BG, W_BG, CHECK_ALL|CHECK_DIFFER)) {

			/*
			 * The whole base window was clear, the server is not using
			 * bit-gravity, or else we are trying ForgetGravity.
			 */
			if (bitgrav->val != ForgetGravity)
				trace("server not using bit-gravity");
			CHECK;
		} else {
			/*
			 * For ForgetGravity then we must not get here.
			 */
			if (bitgrav->val == ForgetGravity) {
				report("Contents were not discarded with ForgetGravity");
				FAIL;
			} else if (checkarea(display, base, &area, W_FG, W_BG, CHECK_ALL)) {
				CHECK;
			} else {
				report("bits positioned incorrectly for bit-gravity of %s",
					bitgrav->name);
				FAIL;
			}
		}
	}

	CHECKPASS(NS_bitgravity);

>>ASSERTION Good A
When the
.S CWBorderWidth
bit is set in
.A value_mask ,
then the width of the border is set to
.M border_width
pixels.
>>STRATEGY
Create window.
Call xname to change border width.
Verify border by pixmap check.
>>CODE
int 	i;
Window	base;
static	int 	bords[] = {
	1, 2, 3, 4, 5, 10, 17, 29, 34, 51, 90, 234
};

	base = onewin();

	/*
	 * Set the background and border to highlight the border.
	 */
	XSetWindowBackground(display, w, W_BG);
	XClearWindow(display, w);
	XSetWindowBorder(display, w, W_FG);

	for (i = 0; i < NELEM(bords); i++) {

		values->border_width = bords[i];
		value_mask = CWBorderWidth;

		XCALL;

		PIXCHECK(display, base);
	}

	CHECKPASS(NELEM(bords));
>>EXTERN

/*
 * Window tree for the restacking operations.
 */
static char	*Tstack[] = {
	". borders",
	"bottom . (30,2) 60x34",
	"A . (10,5) 30x20",
	"B . (15, 10) 30x20",
	"mid . (20, 15) 30x20",
	"C . (25, 20) 30x20",
	"D . (30, 25) 30x20",
	"alone . (50,50) 20x20",	/* does not overlap with any window */
	"olmid . (3,32) 20x20",	/* Overlaps mid */
	"top . (5,80) 19x4",
};
#define	NTstack	(NELEM(Tstack))
#define	NTstackInf	(NELEM(Tstack)-1)
#define TstackTOS	(NTstackInf-1)	/* Top of stacking order number */

#ifdef GENERATE_PIXMAPS
#define	stackorder(a,b) (0)
#endif

>>ASSERTION Good A
When the
.S CWSibling
and
.S CWStackMode
bits are set in
.A value_mask
and
.M stack_mode
is
.S Above ,
then the window is placed just above the sibling.
>>STRATEGY
Create stack of windows.
Set value to Above.
Set sibling to a window in the middle of the stacking order.
For each sibling of 'mid'.
  Call xname on the window.
  Verify that window is now one above sibling in stacking order.
  Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*mid;
int 	sibso;
int 	winso;
int 	i;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	mid = btntobtp(bt, "mid");

	values->stack_mode = Above;
	values->sibling = mid->wid;
	value_mask = CWStackMode|CWSibling;

	for (i = 1; i < NTstack; i++) {

		if (bt+i == mid)
			continue;

		trace("Restacking '%s'", bt[i].name);
		w = bt[i].wid;

		XCALL;

		winso = stackorder(display, w);
		sibso = stackorder(display, mid->wid);
		if (winso != sibso+1) {
			report("Window not restacked to one above, got %d, expecting %d",
				winso, sibso+1);
			FAIL;
		} else
			CHECK;

		PIXCHECK(display, base);
	}

	CHECKPASS(2*(NTstackInf-1));

>>ASSERTION Good A
When the
.S CWSibling
and
.S CWStackMode
bits are set in
.A value_mask
and
.M stack_mode
is
.S Below ,
then the window is placed just below the sibling.
>>STRATEGY
Create stack of windows.
Set value to Below.
Set sibling to a window in the middle of the stacking order.
For each sibling window.
  Call xname on a window.
  Verify that window in now one below sibling in stacking order.
  Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*mid;
int 	sibso;
int 	winso;
int 	i;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	mid = btntobtp(bt, "mid");

	values->stack_mode = Below;
	values->sibling = mid->wid;
	value_mask = CWStackMode|CWSibling;

	for (i = 1; i < NTstack; i++) {

		if (bt+i == mid)
			continue;

		trace("Restacking '%s'", bt[i].name);
		w = bt[i].wid;

		XCALL;

		winso = stackorder(display, w);
		sibso = stackorder(display, mid->wid);
		if (winso != sibso-1) {
			report("Window not restacked to one below, got %d, expecting %d",
				winso, sibso+1);
			FAIL;
		} else
			CHECK;

		PIXCHECK(display, base);
	}

	CHECKPASS(2*(NTstackInf-1));

>>ASSERTION Good A
When the
.S CWSibling
and
.S CWStackMode
bits are set in
.A value_mask ,
.M stack_mode
is
.S TopIf
and the sibling occludes the window,
then the window is placed at the top of the stack.
>>STRATEGY
Create stack of windows.
Set value to TopIf.
Set sibling to a window in the middle of the stacking order.
Call xname on a window that is occluded by the sibling.
Verify that window in now at top of the stack.
Verify screen contents by pixmap checking.
Call xname on a window that is not occluded by the sibling.
Verify that window stacking position is unchanged.
>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*mid;
int 	oldso;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	mid = btntobtp(bt, "mid");

	values->stack_mode = TopIf;
	values->sibling = mid->wid;
	value_mask = CWStackMode|CWSibling;

	w = btntow(bt, "B");
	XCALL;

	winso = stackorder(display, w);
	if (winso != TstackTOS) {
		report("Window not at top of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	/* Now a window that is not occluded */
	w = btntow(bt, "D");
	oldso = stackorder(display, w);
	XCALL;

	winso = stackorder(display, w);
	if (winso != oldso) {
		report("Window stacking order changed from %d to %d", oldso, winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(4);

>>ASSERTION Good A
When the
.S CWSibling
and
.S CWStackMode
bits are set in
.A value_mask ,
.M stack_mode
is
.S BottomIf
and the window
occludes the sibling, then the window is placed at the bottom of the stack.
>>STRATEGY
Create stack of windows.
Set value to BottomIf.
Set sibling to a window in the middle of the stacking order.
Call xname on a window that occludes the sibling.
Verify that window in now at bottom of the stack.
Verify screen contents by pixmap checking.
Call xname on a window that does not occlude the sibling.
Verify that window position is unchanged.
>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*mid;
int 	oldso;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	mid = btntobtp(bt, "mid");

	values->stack_mode = BottomIf;
	values->sibling = mid->wid;
	value_mask = CWStackMode|CWSibling;

	w = btntow(bt, "D");
	XCALL;

	winso = stackorder(display, w);
	if (winso != 0) {
		report("Window not at bottom of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	/* Now a window that does not occlude */
	w = btntow(bt, "B");
	oldso = stackorder(display, w);
	XCALL;

	winso = stackorder(display, w);
	if (winso != oldso) {
		report("Window stacking order changed from %d to %d", oldso, winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(4);

>>ASSERTION Good A
When the
.S CWSibling
and
.S CWStackMode
bits are set in
.A value_mask ,
.M stack_mode
is
.S Opposite
and the window occludes the sibling,
then the window is placed at the bottom of the stack.
>>STRATEGY
Create stack of windows.
Set value to Opposite.
Set sibling to a window in the middle of the stacking order.
Call xname on a window that occludes the sibling.
Verify that window is now at bottom of stack.
Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*mid;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	mid = btntobtp(bt, "mid");

	values->stack_mode = Opposite;
	values->sibling = mid->wid;
	value_mask = CWStackMode|CWSibling;

	w = btntow(bt, "D");
	XCALL;

	winso = stackorder(display, w);
	if (winso != 0) {
		report("Window not at bottom of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION Good A
When the
.S CWSibling
and
.S CWStackMode
bits are set in
.A value_mask ,
.M stack_mode
is
.S Opposite
and the sibling occludes the window,
then the window is placed at the top of the stack.
>>STRATEGY
Create stack of windows.
Set value to Opposite.
Set sibling to a window in the middle of the stacking order.
Call xname on a window that is occluded by the sibling.
Verify that window is at the top of the stack.
Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
struct	buildtree	*mid;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	mid = btntobtp(bt, "mid");

	values->stack_mode = Opposite;
	values->sibling = mid->wid;
	value_mask = CWStackMode|CWSibling;

	/* A window that is occluded by sibling */
	w = btntow(bt, "B");
	XCALL;

	winso = stackorder(display, w);
	if (winso != TstackTOS) {
		report("Window not brought to top of stack (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION Good A
When the
.S CWStackMode
bit is set and the
.S CWSibling
bit is not set in
.A value_mask
and
.M stack_mode
is
.S Above ,
then the window is placed at the top of the stack.
>>STRATEGY
Create stack of windows.
Set value to Above.
Call xname on a window.
Verify that window is at top of stack.
Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->stack_mode = Above;
	value_mask = CWStackMode;

	w = btntow(bt, "mid");
	XCALL;

	winso = stackorder(display, w);
	if (winso != TstackTOS) {
		report("Window not at top of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION Good A
When the
.S CWStackMode
bit is set and the
.S CWSibling
bit is not set in
.A value_mask
and
.M stack_mode
is
.S Below ,
then the window is placed at the bottom of the stack.
>>STRATEGY
Create stack of windows.
Set value to Below.
Call xname on a window.
Verify that window is at the bottom of the stack.
Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->stack_mode = Below;
	value_mask = CWStackMode;

	w = btntow(bt, "mid");
	XCALL;

	winso = stackorder(display, w);
	if (winso != 0) {
		report("Window not at bottom of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION Good A
When the
.S CWStackMode
bit is set and the
.S CWSibling
bit is not set in
.A value_mask ,
.M stack_mode
is
.S TopIf
and a sibling occludes the window, then the window is placed at
the top of the stack.
>>STRATEGY
Create stack of windows.
Set value to TopIf.
Call xname on a window that is occluded by a sibling.
Verify that window is at the top of the stack.
Verify screen contents by pixmap checking.
Call xname on a window that is not occluded by a sibling.
Verify that window stacking position is unchanged.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	oldso;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->stack_mode = TopIf;
	value_mask = CWStackMode;

	w = btntow(bt, "mid");
	XCALL;

	winso = stackorder(display, w);
	if (winso != TstackTOS) {
		report("Window not at top of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	w = btntow(bt, "alone");
	oldso = stackorder(display, w);
	XCALL;

	winso = stackorder(display, w);
	if (winso != oldso) {
		report("Window stacking order changed from %d to %d", oldso, winso);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the
.S CWStackMode
bit is set and the
.S CWSibling
bit is not set in
.A value_mask ,
.M stack_mode
is
.S BottomIf
and the window occludes a sibling,
then the window is placed at the bottom of the stack.
>>STRATEGY
Create stack of windows.
Set value to BottomIf.
Call xname on a window that occludes a sibling.
Verify that window is at the bottom of the stack.
Verify screen contents by pixmap checking.
Call xname on a window that does not occlude a sibling.
Verify that window stacking position is unchanged.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	oldso;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->stack_mode = BottomIf;
	value_mask = CWStackMode;

	w = btntow(bt, "mid");
	XCALL;

	winso = stackorder(display, w);
	if (winso != 0) {
		report("Window not at bottom of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	w = btntow(bt, "alone");
	oldso = stackorder(display, w);
	XCALL;

	winso = stackorder(display, w);
	if (winso != oldso) {
		report("Window stacking order changed from %d to %d", oldso, winso);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the
.S CWStackMode
bit is set and the
.S CWSibling
bit is not set in
.A value_mask ,
.M stack_mode
is
.S Opposite
and the window occludes a sibling,
then the window is placed at the bottom of the stack.
>>STRATEGY
Create stack of windows.
Set value to Opposite.
Call xname on a window that occludes a sibling.
Verify that window is at the bottom of the stack.
Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->stack_mode = Opposite;
	value_mask = CWStackMode;

	w = btntow(bt, "olmid");
	XCALL;

	winso = stackorder(display, w);
	if (winso != 0) {
		report("Window not at bottom of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION Good A
When the
.S CWStackMode
bit is set and the
.S CWSibling
bit is not set in
.A value_mask ,
.M stack_mode
is
.S Opposite
and a sibling occludes the window,
then the window is placed at the top of the stack.
>>STRATEGY
Create stack of windows.
Set value to Opposite.
Call xname on a window that is occluded by a sibling.
Verify that window is at the top of the stack.
Verify screen contents by pixmap checking.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->stack_mode = Opposite;
	value_mask = CWStackMode;

	w = btntow(bt, "A");
	XCALL;

	winso = stackorder(display, w);
	if (winso != NTstackInf-1) {
		report("Window not at top of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION Good A
When the geometry of the window is changed as well as a restacking
operation performed, then the window's final geometry is used to determine
whether the window occludes or is occluded by siblings.
>>STRATEGY
Create stack of windows.
Set sibling to a window in the middle of the stack.
Choose window that is below sibling but is not currently occluded by it.
Set stack_mode to TopIf
Call xname to resize window so that it is now occluded by sibling.
Verify that the window is at the top of the stack.
>>CODE
Window	base;
struct	buildtree	*bt;
int 	winso;

	base = defwin(display);
	bt = buildtree(display, base, Tstack, NTstack);

	values->sibling = btntow(bt, "mid");
	values->stack_mode = Below;
	value_mask = CWStackMode;

	w = btntow(bt, "mid");
	XCALL;

	winso = stackorder(display, w);
	if (winso != 0) {
		report("Window not at top of stacking order (pos=%d)", winso);
		FAIL;
	} else
		CHECK;

	PIXCHECK(display, base);

	CHECKPASS(2);

>>ASSERTION def
>># t020
Values not specified are taken from the existing geometry of the window.
>># Since we have been only changing a few things at a time
>># if this was false then other tests would have failed.
>>ASSERTION Good A
When the window
is a root window, then a call to xname has no effect.
>>STRATEGY
Call xname on root window.
Touch test only.
>>CODE

	values->x = NEW_X;
	values->y = NEW_Y;
	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;
	values->stack_mode = Above;
	value_mask = CWX|CWY|CWWidth|CWHeight|CWStackMode;

	w = DefaultRootWindow(display);

	XCALL;

	if (fail == 0)
		PASS;
>>ASSERTION Good A
When the override-redirect attribute of the window is
.S False
and some
other client has selected
.S SubstructureRedirectMask
on the parent window, then a
.S ConfigureRequest
event is generated, and the window configuration is not changed.
>>STRATEGY
Create windows.
Set override-redirect to False.
Create second client.
Select SubstructureRedirectMask for second client on parent of window.
Set some parameters.
Call xname.
Verify that a ConfigureRequest event is generated.
Verify that window configuration has not changed on the screen.
>>CODE
Window	base;
XConfigureRequestEvent	good;
XSetWindowAttributes	setatts;
Display	*client2;
XEvent	ev;
XImage	*imp;
int 	n;

	base = onewin();

	setatts.override_redirect = False;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);

	client2 = opendisplay();
	XSelectInput(client2, base, SubstructureRedirectMask);
	XSync(client2, False);

	values->x = NEW_X;
	values->y = NEW_Y;
	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;

	value_mask = CWX|CWY|CWWidth|CWHeight;

	good.type = ConfigureRequest;
	good.serial = 0L;
	good.send_event = False;
	good.display = client2;
	good.parent = base;
	good.window = w;
	good.x = NEW_X;
	good.y = NEW_Y;
	good.width = NEW_WIDTH;
	good.height = NEW_HEIGHT;
	good.border_width = 0;
	good.above = None;
	good.detail = Above;
	good.value_mask = value_mask;

	imp = savimage(display, base);

	XCALL;
	XSync(client2, False);

	n = getevent(client2, &ev);
	if (n != 1) {
		report("Expecting 1 event");
		FAIL;
	} else
		CHECK;

	if (n > 0 && checkevent((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	if (compsavimage(display, base, imp))
		CHECK;
	else {
		report("Screen contents changed");
		FAIL;
	}

	CHECKPASS(3);

>>ASSERTION Good A
When another client has selected
.S ResizeRedirectMask
on the window and the size is being changed, then a
.S ResizeRequest
event is generated and the size is not changed.
>># Other changes take place though.
>>STRATEGY
Create windows.
Set override-redirect to False.
Create second client.
Select ResizeRedirectMask for second client on window.
Set parameters to move and resize window.
Call xname.
Verify that a ResizeRequest event is generated.
Verify that window has not changed size but that other changes have occurred.
>>CODE
Window	base;
XResizeRequestEvent	good;
XSetWindowAttributes	setatts;
Display	*client2;
XEvent	ev;
struct	area	area;
int 	n;

	base = onewin();

	setatts.override_redirect = False;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);

	client2 = opendisplay();
	XSelectInput(client2, w, ResizeRedirectMask);
	XSync(client2, False);

	values->x = NEW_X;
	values->y = NEW_Y;
	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;

	value_mask = CWX|CWY|CWWidth|CWHeight;

	good.type = ResizeRequest;
	good.serial = 0L;
	good.send_event = False;
	good.display = client2;
	good.window = w;
	good.width = NEW_WIDTH;
	good.height = NEW_HEIGHT;

	XCALL;
	XSync(client2, False);

	n = getevent(client2, &ev);
	if (n != 1) {
		report("Expecting 1 event");
		FAIL;
	} else
		CHECK;

	if (n > 0 && checkevent((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	/* Window moves, but does not change size */
	setarea(&area, NEW_X, NEW_Y, OW_WIDTH, OW_HEIGHT);
	if (checkarea(display, base, &area, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("New location and size of window not as expected");
		FAIL;
	}

	CHECKPASS(3);

>>ASSERTION Good A
When another client has selected
.S ResizeRedirectMask
on the window and another client has selected
.S SubstructureRedirectMask
on the parent window
and the override-redirect attribute of the window is
.S False ,
then a
.S ConfigureRequest
event is generated, and the window configuration is not changed.
>>STRATEGY
Create windows.
Set override-redirect to False.
Create second client.
Select ResizeRedirectMask for second client on window.
Create third client.
Select SubstructureRedirectMask for third client on parent of window.
Call xname.
Verify that a ConfigureRequest event is generated for client 3.
Verify that no ResizeRequest event is generated for client 2.
Verify that window configuration is not changed.
>>CODE
Window	base;
XConfigureRequestEvent	good;
XSetWindowAttributes	setatts;
Display	*client2;
Display	*client3;
XEvent	ev;
XImage	*imp;
int 	n;

	base = onewin();

	setatts.override_redirect = False;
	XChangeWindowAttributes(display, w, CWOverrideRedirect, &setatts);

	client2 = opendisplay();
	XSelectInput(client2, w, ResizeRedirectMask);
	XSync(client2, False);

	client3 = opendisplay();
	XSelectInput(client3, base, SubstructureRedirectMask);
	XSync(client3, False);

	values->x = NEW_X;
	values->y = NEW_Y;
	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;

	value_mask = CWX|CWY|CWWidth|CWHeight;

	imp = savimage(display, base);

	XCALL;
	XSync(client2, False);
	XSync(client3, False);

	n = getevent(client2, &ev);
	if (n != 0) {
		report("Got an event unexpectedly for client selecting ResizeRedirect");
		report("  Event type was %s", eventname(ev.type));
		FAIL;
	} else
		CHECK;

	good.type = ConfigureRequest;
	good.serial = 0L;
	good.send_event = False;
	good.display = client3;
	good.parent = base;
	good.window = w;
	good.x = NEW_X;
	good.y = NEW_Y;
	good.width = NEW_WIDTH;
	good.height = NEW_HEIGHT;
	good.border_width = 0;
	good.above = None;
	good.detail = Above;
	good.value_mask = value_mask;

	n = getevent(client3, &ev);
	if (n != 1) {
		report("Expecting one configure events");
		FAIL;
	} else
		CHECK;
	if (n > 0 && checkevent((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	if (compsavimage(display, base, imp))
		CHECK;
	else {
		report("Window changed when client was selecting SubstructureRedirect");
		FAIL;
	}
	CHECKPASS(4);

>>ASSERTION Good A
When the configuration actually changes, then a
.S ConfigureNotify
event is generated.
>>STRATEGY
Create windows.
Enable SubstructureNotify events.
Call xname such that the window configuration changes.
Verify that a ConfigureNotify event is generated.
Call xname again with the same parameters.
Verify that no ConfigureNotify event is generated.
>>CODE
XConfigureEvent	good;
XEvent	ev;
int 	n;

	(void) onewin();

	XSelectInput(display, w, StructureNotifyMask);

	values->x = NEW_X;
	values->y = NEW_Y;
	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;

	value_mask = CWX|CWY|CWWidth|CWHeight;

	XCALL;

	good.type = ConfigureNotify;
	good.serial = 0L;
	good.send_event = False;
	good.display = display;
	good.event = w;
	good.window = w;
	good.x = NEW_X;
	good.y = NEW_Y;
	good.width = NEW_WIDTH;
	good.height = NEW_HEIGHT;
	good.border_width = 0;
	good.above = None;
	good.override_redirect = False;

	n = getevent(display, &ev);
	if (n != 1) {
		report("Expecting 1 event, got %d", n);
		FAIL;
	} else
		CHECK;
	if (n > 0 && checkevent((XEvent*)&good, &ev) == 0)
		CHECK;
	else
		FAIL;

	/* Call again */
	XCALL;
	if ((n = XPending(display)) == 0)
		CHECK;
	else {
		report("Received event when configuration was not changed");
		FAIL;
	}

	CHECKPASS(3);

>># ASSERTION Good A
>># There are Expose events to consider here.
>># Also any window that is uncovered etc.
>>ASSERTION Good A
When a call to xname changes the size of the window, then
.S Expose
events are generated for regions that are newly visible or for
which the contents have been lost.
>>STRATEGY
Create windows.
Set test window background to W_BG.
Set up window with setforexpose().
Enable expose events.
Resize window with xname.
Verify that correct expose events were received with exposecheck().
>>CODE

	(void) onewin();

	XSetWindowBackground(display, w, W_BG);
	XClearWindow(display, w);
	setforexpose(display, w);

	XSelectInput(display, w, ExposureMask);

	values->width = NEW_WIDTH;
	values->height = NEW_HEIGHT;
	value_mask = CWWidth|CWHeight;

	XCALL;

	if (exposecheck(display, w))
		CHECK;
	else {
		report("Correct expose events not received after resize");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When a call to xname
uncovers part of any window that was formerly obscured, then
either
.S Expose
events are generated or the contents are restored from backing store.
>>STRATEGY
Create windows.
Create second client to receive events on.
Call setforexpose() on unobscured window.
Create other windows to partially obscure this window.
Reconfigure window with xname, ensuring that first window is now unobscured.
Verify for correct expose or backing store behaviour with exposecheck().
>>CODE
Window	base;
Display	*client2;
struct	buildtree	*bt;
int 	i;

	base = defwin(display);
	setforexpose(display, base);

	bt = buildtree(display, base, Tstack, NTstack);

	client2 = opendisplay();
	XSelectInput(client2, base, ExposureMask);
	XSync(client2, False);

	/*
	 * This loop will throw all the subwindows outside the borders of the
	 * parent window.
	 */
	values->x = W_STDWIDTH+9000;
	value_mask = CWX;
	for (i = 1; i < NTstack; i++) {
		w = bt[i].wid;
		XCALL;
	}
	XSync(client2, False);

	if (exposecheck(client2, base))
		CHECK;
	else {
		report("Neither Expose events or backing store processing");
		report("  could correctly restore the window contents.");
		FAIL;
	}
	CHECKPASS(1);
>>ASSERTION Good A
>># I complicated this one slightly and then split it into two. - kieron
When the window is moved without changing its size
and the window is mapped
and the window is not obscured by non-children,
then the current window contents are not lost.
>>STRATEGY
Create window.
Draw pattern in window.
Move window without a size change.
Verify that window contents are unchanged.
>>CODE
XImage	*imp;

	(void) onewin();

	pattern(display, w);

	values->x = NEW_X;
	values->y = NEW_Y;
	value_mask = CWX|CWY;

	imp = savimage(display, w);

	XCALL;

	if (compsavimage(display, w, imp))
		CHECK;
	else {
		report("Window contents affected by moving window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good D 3
>># I complicated this one slightly and then split it into two. - kieron
>>#	"backing store exists" is untestable, I reckon. - kieron
If backing-store is supported when mapped:
When the window is moved without changing its size
and the window is mapped
and backing store exists,
then the current window contents are not lost.
>># ASSERTION Good A
>># -- I don't know when this applies ..sr Ignore it, kieron.
>># When backing store is being maintained for the window, its
>># inferiors, or other newly visible windows, then the contents
>># are either discarded or changed to reflect the current screen contents.
>>ASSERTION Bad A
When
.S CWSibling
is set and
.S CWStackMode
is not set, then a
.S BadMatch
error occurs.
>>STRATEGY
Set CWSibling only.
Call xname.
Verify that a BadMatch error occurs.
>>CODE BadMatch
Window	base;
Window	win;
struct	area	area;

	base = defwin(display);

	setarea(&area, 2, 4, 9, 8);
	win = crechild(display, base, &area);
	w = crechild(display, base, &area);

	values->sibling = win;
	value_mask = CWSibling;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL; /* already done */
>>ASSERTION Bad A
When sibling is specified and it is not
actually a sibling, then a
.S BadMatch
error occurs.
>>STRATEGY
Create windows.
Set sibling to parent of the child window.
Set stack_mode to Above.
Call xname.
Verify BadMatch error occurs.
>>CODE BadMatch
Window	base;

	base = onewin();

	values->sibling = base;
	values->stack_mode = Above;
	value_mask = CWSibling|CWStackMode;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL; /* again */
>>ASSERTION Bad A
When
.M width
or
.M height
is zero, then a
.S BadValue
error occurs.
>>STRATEGY
Set width to zero.
Call xname.
Verify BadValue error.
Set height to zero.
Call xname.
Verify BadValue error.
Set both width and height to zero.
Call xname.
Verify BadValue error.
>>CODE BadValue

	(void) onewin();

	values->width = 0;
	values->height = 0;

	value_mask = CWWidth;
	XCALL;
	if (geterr() == BadValue)
		CHECK;

	value_mask = CWHeight;
	XCALL;
	if (geterr() == BadValue)
		CHECK;

	value_mask = CWWidth|CWHeight;
	XCALL;
	if (geterr() == BadValue)
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
When an attempt is made to set the border-width attribute of an
.S InputOnly
window to a non-zero value, then a
.S BadMatch
error occurs.
>>STRATEGY
Create input only window.
Attempt to set border-width.
Verify BadMatch error.
>>CODE BadMatch

	/*
	 * It would be better for iponlywin to have a parent arg, how
	 * might this test interact with window managers.
	 */
	w = iponlywin(display);

	values->border_width = 21;
	value_mask = CWBorderWidth;

	XCALL;
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL; /* again */

>># This assertion has been removed at the request of MIT, because there
>># is no requirement  for the Xlib implementation to generate protocol
>># errors for illegal maske bits in this case. (Bug report 58)
>># >>ASSERTION Bad A
>># .ER BadValue value_mask mask CWX CWY CWWidth CWHeight CWBorderWidth CWSibling CWStackMode
>>ASSERTION Bad A
.ER BadWindow
