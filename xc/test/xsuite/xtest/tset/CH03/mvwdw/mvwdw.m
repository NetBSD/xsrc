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
 * $XConsortium: mvwdw.m,v 1.9 94/04/17 21:03:14 rws Exp $
 */
>>TITLE XMoveWindow CH03
void

Display	*display = Dsp;
Window	w;
int	x;
int	y;
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

#define	NEW_X	OW_X+5
#define	NEW_Y	OW_Y+5


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
>>ASSERTION Good A
A call to xname moves the window so that the
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
	x = area.x;
	y = area.y;

	XCALL;

	if (checkarea(display, base, &area, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("Error in moving window");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When the window is moved
and the window is mapped
and the window is not obscured by non-children, then the current window
contents are not lost.
>>STRATEGY
Create window.
Draw pattern in window.
Move window without a size change.
Verify that window contents are unchanged.
>>CODE
XImage	*imp;

	(void) onewin();

	pattern(display, w);

	x = NEW_X;
	y = NEW_Y;

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
When the window is moved
and the window is mapped
and backing store is being maintained for the window, then the current window
contents are not lost.
>># Kieron reckoned a sibling assertion to this in XConfigureWindow
>># was D 3. I have no reason to doubt this! stuart.
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
	x = W_STDWIDTH+9000;
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
When the window
is a root window, then a call to xname has no effect.
>>STRATEGY
Call xname on root window.
Touch test only.
>>CODE

	w = DefaultRootWindow(display);

	XCALL;

	if (fail == 0)
		PASS;
>>ASSERTION Good A
When the override-redirect flag of the window is
.S False
and some
other client has selected
.S SubstructureRedirectMask
on the parent window, then a
.S ConfigureRequest
event is generated, and the window position is not changed.
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

	x = NEW_X;
	y = NEW_Y;

	good.type = ConfigureRequest;
	good.serial = 0L;
	good.send_event = False;
	good.display = client2;
	good.parent = base;
	good.window = w;
	good.x = NEW_X;
	good.y = NEW_Y;
	good.width = OW_WIDTH;
	good.height = OW_HEIGHT;
	good.border_width = 0;
	good.above = None;
	good.detail = Above;
	good.value_mask = CWX | CWY;

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
When the position actually changes, then a
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

	x = NEW_X;
	y = NEW_Y;

	XCALL;

	good.type = ConfigureNotify;
	good.serial = 0L;
	good.send_event = False;
	good.display = display;
	good.event = w;
	good.window = w;
	good.x = NEW_X;
	good.y = NEW_Y;
	good.width = OW_WIDTH;
	good.height = OW_HEIGHT;
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

>>ASSERTION Bad A
.ER BadWindow
