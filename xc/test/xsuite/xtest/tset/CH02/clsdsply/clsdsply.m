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
 * $XConsortium: clsdsply.m,v 1.25 94/04/17 21:02:04 rws Exp $
 */
>>TITLE XCloseDisplay CH02

XCloseDisplay(display)
Display	*display = Dsp;
>>EXTERN

#include	"string.h"
#include	"Xatom.h"

#define PROTECT(x)	_startcall(display);\
			x;\
			_endcall(display);\
			if(geterr() != Success) {\
				report("Got %s, Expecting %s.", errorname(geterr()), errorname(Success));\
				FAIL;\
			} else\
				CHECK;

#define OPEN(disp, closemode)	\
	if((disp = XOpenDisplay(config.display)) == (Display *) NULL) { \
		delete("XOpenDisplay() returned NULL."); \
		return; \
	} else \
		CHECK; \
	XSetCloseDownMode(disp, closemode);

#define DELETE_ONERR(line, errstr) \
	startcall(display); \
	line; \
	endcall(display); \
	if(geterr() != Success) { \
		delete("%s() generated a %s error.", errstr, errorname(geterr())); \
		return; \
	} else \
		CHECK;

#define FAIL_ONERR(line, errstr, id) \
	startcall(Dsp); \
	line; \
	endcall(Dsp); \
	if(geterr() == Success) { \
		report(errstr, TestName, (long) id); \
		FAIL; \
	} else \
		CHECK;

#define FAIL_ONERR2(line, errstr, id, mode) \
	startcall(Dsp); \
	line; \
	endcall(Dsp); \
	if(geterr() != Success) { \
		report(errstr, TestName, (long) id, mode); \
		report("Error was %s instead of Success", errorname(geterr())); \
		FAIL; \
	} else \
		CHECK;

#define FAIL_ONERR3(line, err, errstr, id) \
	startcall(Dsp); \
	line; \
	endcall(Dsp); \
	if(geterr() != err) { \
		report("Got error %s instead of %s.", errorname(geterr()), errorname(err)); \
		report(errstr, TestName, (long) id); \
		FAIL; \
	} else \
		CHECK;

#define FAIL_ONERR4(line, err, errstr, id, mode) \
	startcall(Dsp); \
	line; \
	endcall(Dsp); \
	if(geterr() != err) { \
		report(errstr, TestName, (long) id, mode); \
		report("Got error %s instead of %s", errorname(geterr()), errorname(err)); \
		FAIL; \
	} else \
		CHECK;

char	*cmode[2] = { 	" for a client with a closedown mode of RetainPermanent",
			" for a client with a closedown mode of RetainTemporary" };

int	closemodes[2] = { RetainPermanent, RetainTemporary };

>># COMMENT : See XSync - also B 3.
>>ASSERTION Good B 3
A call to xname flushes the output buffer and waits until each flushed request has
been received and processed by the X server.
>>ASSERTION Good B 3
A call to xname closes the connection specified by the
.A display
argument.
>>STRATEGY
If the system is POSIX compliant:
  Open a display using XOpenDisplay.
  Obtain the file descriptor of the connection using XConnectionNumber.
  Close the display using xname.
  Close the file descriptor using close.
  Verify that the call returned -1.
Otherwise:
  UNTESTED.
>>CODE
int	fd;

	if(config.posix_system == 0) {
		untested("This assertion can only be tested on a POSIX system.");
		return;
	} else
		CHECK;

	if((display = XOpenDisplay(config.display)) == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		return;
	} else
		CHECK;

	fd = XConnectionNumber(display);

	PROTECT(XCloseDisplay(display))

	if( close(fd) != -1) {
		report("%s() did not close file descriptor %d.", TestName, fd);
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Good A
A call to xname disowns all selections made by the client.
>>STRATEGY
Open a display using XOpenDisplay.
Create a client window using XCreateWindow.
Acquire the XA_PRIMARY selection using XSetSelectionOwner.
Verify that the client owns the selection with XGetSelectionOwner.
Close the display using xname.
Verify that the XA_PRIMARY selection is not owned with XGetSelectionOwner.
>>CODE
Window	client;
Atom	selection = XA_PRIMARY;

	if((display = XOpenDisplay(config.display)) == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		return;
	} else
		CHECK;

	regdisable();
	client = defwin(display);
	regenable();

	XSetSelectionOwner(display, selection, client, CurrentTime);
	
	if(XGetSelectionOwner(display, selection) != client) {
		delete("Failed to get selection XA_PRIMARY.");
		XCloseDisplay(display);
		return;
	} else
		CHECK;

	PROTECT(XCloseDisplay(display))
	sleep(config.speedfactor);

	if(XGetSelectionOwner(Dsp, selection) != (Window) None) {
		report("%s() did not disown selection XA_PRIMARY.", TestName);
		return;
	} else
		CHECK;

	CHECKPASS(4);


>>ASSERTION Good A
When the client has actively grabbed the pointer, then a call to xname
performs a
.S XUngrabPointer .
>>STRATEGY
Open a display using XOpenDisplay.
Grab the pointer using XGrabPointer.
Verify that the call returned GrabSuccess.
Close the display using xname.
Grab the pointer using XGrabPointer.
Verify that the call returned GrabSuccess.
>>CODE
Window	gw, ow;

	if( (display = XOpenDisplay(config.display)) == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		return;
	} else
		CHECK;

	regdisable();
	gw = defwin(display);
	ow = defwin(Dsp);
	regenable();

	if(XGrabPointer(display, gw, True, 0L, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		delete("XGrabPointer did not return GrabSuccess.");
		XCloseDisplay(display);
		return;
	} else
		CHECK;

	XSync(display, False);

	if(XGrabPointer(Dsp, ow, True, 0L, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != AlreadyGrabbed) {
		delete("XGrabPointer did not return AlreadyGrabbed.");
		XCloseDisplay(display);
		return;
	} else
		CHECK;

	PROTECT(XCloseDisplay(display))
	sleep(config.speedfactor);

	if(XGrabPointer(Dsp, ow, True, 0L, GrabModeAsync, GrabModeAsync, None, None, CurrentTime) != GrabSuccess) {
		report("%s() did not ungrab the pointer.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

	XUngrabPointer(Dsp, CurrentTime);
	
>>ASSERTION Good A
When the client has actively grabbed the keyboard, then a call to xname
performs a
.S XUngrabKeyboard .
>>STRATEGY
Open a display using XOpenDisplay.
Grab the keyboard using XGrabKeyboard.
Verify that the call returned GrabSuccess.
Close the display using xname.
Grab the keyboard using XGrabKeyboard.
Verify that the call returned GrabSuccess.
>>CODE
Window	gw, ow;

	if( (display = XOpenDisplay(config.display)) == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		return;
	} else
		CHECK;

	regdisable();
	gw = defwin(display);
	ow = defwin(Dsp);
	regenable();

	if(XGrabKeyboard(display, gw, True, GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess) {
		delete("XGrabKeyboard did not return GrabSuccess.");
		XCloseDisplay(display);
		return;
	} else
		CHECK;

	XSync(display, False);

	if(XGrabKeyboard(Dsp, ow, True, GrabModeAsync, GrabModeAsync, CurrentTime) != AlreadyGrabbed) {
		delete("XGrabKeyboard did not return AlreadyGrabbed.");
		XCloseDisplay(display);
		return;
	} else
		CHECK;

	PROTECT(XCloseDisplay(display))
	sleep(config.speedfactor);

	if(XGrabKeyboard(Dsp, ow, True, GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess) {
		report("%s() did not ungrab the keyboard.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

	XUngrabKeyboard(Dsp, CurrentTime);
	
>>ASSERTION Good A
When the client has grabbed the server, then
a call to xname performs a
.S XUngrabServer .
>>STRATEGY
Create a connection for client1 using XOpenDisplay.
Create a connection for client2 using XOpenDisplay.
Create a window using XCreateWindow.
Grab the server for the default display using XGrabServer.
Create a process using tet_fork.
In child process:
   Generate a ChangeProperty request using XChangeProperty.
Wait sufficient time for the ChangeProperty request to be processed.
Verify that no PropertyChange event has been generated by client2 using XCheckWindowEvent.
Close the client1 connection using xname.
Wait suffient time for the ChangeProperty request to be processed.
Verify that a ChangeProperty request was generated by client2.
>>EXTERN
int	waittime = -1;
Atom	at;
Window	win;
Display	*client2;
char	*atname = "XT_TEST_Atom";
static void
t006p()
{
XEvent	ev;
int	pass=0, fail=0;

	sleep(waittime);

	if(XCheckWindowEvent(display, win, PropertyChangeMask, &ev) == True) {
		delete("A request was processed for a non-grabbing client.");
		return;
	} else
		CHECK;

	XCloseDisplay(display);
	
	sleep(waittime);	
	
	if(XCheckWindowEvent(client2, win, PropertyChangeMask, &ev) == False) {
		report("%s() did not ungrab the server.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
}

static void
t006c()
{
long	val;

	val = 666;
	settimeout(waittime * 3);
	XChangeProperty(client2, win, at, XA_INTEGER, 32, PropModeReplace, (unsigned char *) &val, 1);
	XFlush(client2);
	cleartimeout();
	exit(0);
}
>>CODE

	waittime = 2*config.speedfactor + 5;

	win = defwin(Dsp);


	if((display = XOpenDisplay(config.display)) == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		return;
	}

	client2 = opendisplay();
	
	if(client2 == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		XCloseDisplay(display);
		return;
	}

	XSelectInput(client2, win, PropertyChangeMask);
	XSelectInput(display, win, PropertyChangeMask);
	at = XInternAtom(display, atname, False);
	XGrabServer(display);
	XSync(display, False);


	if(tet_fork(t006c, t006p, waittime, 1) == TIMEOUT_EXIT)
		delete("Child process timed out.");

>>ASSERTION Good B 1
A call to xname releases all passive grabs made by the client.
>>ASSERTION Good A
A call to xname frees the default GC for the client.
>>STRATEGY
Open a connection using XOpenDisplay.
Obtain the default gc id using XDefaultGC.
Close the display using xname.
Call XDrawPoint using the default gc id.
Verify that a BadGC error occurred.
>>CODE
GC		gc;

	if((display = XOpenDisplay(config.display)) == (Display *) NULL) {
		delete("XOpenDisplay() returned NULL.");
		return;
	} else
		CHECK;

	gc = XDefaultGC(display, DefaultScreen(display));

	PROTECT(XCloseDisplay(display))
	sleep(config.speedfactor);

	startcall(Dsp);
	XDrawPoint(Dsp, DRW(Dsp), gc, 0,0);
	endcall(Dsp);

	if(geterr() != BadGC) {
		report("%s() did not free the default GC.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good B 1
When the closedown mode of the client is
.S RetainPermanent ,
then all resources allocated by the client are marked as permanent.
>>ASSERTION Good B 1
When the closedown mode of the client is
.S RetainTemporary ,
then all resources allocated by the client are marked as temporary.
>>#
>># Close-down mode of DestroyAll section.
>>#
>>ASSERTION Good A
When the closedown mode of the client is
.S DestroyAll ,
then on a call to xname all windows in the client's save-set that are
inferiors of a window created by the client are reparented, 
with no change in position relative to the root window,
to the closest ancestor such that it is not an inferior of a client's window.
>>STRATEGY
Open a display using XOpenDisplay.
Set the closedown mode of the display to DestroyAll using XSetCloseDownMode.
Create save set with setupsaveset.
Close the display using xname.
Verify reparenting for the save-set occurs and positioning is correct.
>>EXTERN
/*
 * This code is identical to that in addtsvst.m, and should stay that way.
 */

/*
 * These windows are created by client1.
 */
char	*T1[] = {
	"base borders",
	"client1-A base (10,40) 9x4",
	"client1-C base (50,60) 60x60",
	"client1-D base (5,50) 35x70",
	"client1-B base (30,6) 60x30",
};

/*
 * These windows are created by client2 using client1's windows as parent.
 * This will lead to some windows being clipped by their (client1) parent,
 * when the client2 windows are reparented then these should reappear.
 */
char	*T2[] = {
	". borders",
	"client2-a-noadd-nomap . (15, 5) 12x4 unmap",
	"client2-b-noadd . (10, 10) 12x6",
#define	NOADD	2	/* Number not added */
	"client2-c-unmap . (30, 28) 14x7 unmap",
	"client2-d . (45, 15) 9x7",
	"client2-e . (15, 23) 9x7",
};

/*
 * The number of windows that should be reparented.  NOADD of the client2
 * windows is not added to the save set.
 */
#define	NUM_REPARENT ((NELEM(T1)-1)*(NELEM(T2)-NOADD-1))

/* Flags used in uflags */
#define	IN_SAVE_SET	0x01
#define	CLIENT1_INFERIOR	0x02
#define	ON_PARENT	0x04
#define	ON_WINDOW	0x10

/*
 * Create a bunch of windows.  Some are created by client1.  Others are
 * created by client2.  Of the latter some are inferiors of client1 and
 * are marked CLIENT1_INFERIOR.  Some of client2's windows are added to
 * client1's save-set - these are marked IN_SAVE_SET.
 * Client2 windows that are originally unmapped will be marked BT_UNMAP.
 * Structure events are selected for on display Dsp for all appropriate
 * windows.
 * Before returning, client1 is destroyed.
 */
void
setupsaveset(btret, client1, client2)
struct	buildtree	*btret[NELEM(T1)];
Display			*client1;
Display			*client2;
{
Window	base;
Window	w;
XWindowAttributes	batts;
struct	buildtree	*bt1;
struct	buildtree	*btp;
int 	i, j;
int 	pass = 0, fail = 0;

	display = client1;
	base = defwin(Dsp);
	XSync(Dsp, False);

	/*
	 * Build a tree for client1, build a tree for client2 on each of
	 * the client1 windows + the base window.
	 */
	bt1 = buildtree(client1, base, T1, NELEM(T1));
	for (i = 0; i < NELEM(T1); i++) {
		btret[i] = buildtree(client2, bt1[i].wid, T2, NELEM(T2));
		/*
		 * Transfer some of the information down from the client1 windows
		 * into the returned array.  This means that all the positions
		 * in the returned array will be relative to base.
		 */
		btret[i][0].x = bt1[i].x;
		btret[i][0].y = bt1[i].y;
		btret[i][0].borderwidth = bt1[i].borderwidth;
	}

	XSync(client2, False);

	for (i = 0; i < NELEM(T1); i++) {
		for (j = 0; j < NELEM(T2); j++) {

			btp = &btret[i][j];
			w = btp->wid;

			/*
			 * For j==0 --> client1 window.
			 * The window at j==1 is not added to the save-set;
			 * the rest are.
			 */
			if (j > NOADD) {
				btp->uflags |= IN_SAVE_SET;
				XAddToSaveSet(display, w);
			} /* else not added to save set */

			/*
			 * The windows with i=0 are inferiors of the base window
			 * and not one of client1's window.
			 */
			if (i != 0)
				btp->uflags |= CLIENT1_INFERIOR;

			XSelectInput(Dsp, w, StructureNotifyMask);
		}
	}
	XSelectInput(Dsp, base, SubstructureNotifyMask);

/* Store base window coordinates to allow for absolute coordinate checking */
	XGetWindowAttributes(Dsp, base, &batts);
	btret[0][0].x = batts.x;
	btret[0][0].y = batts.y;
	btret[0][0].borderwidth = batts.border_width;

	XSync(Dsp, False);

	PROTECT(XCloseDisplay(display));
	sleep(config.speedfactor);

	XSync(Dsp, False);
	XSync(client2, False);

}
>>CODE
Display	*client1, *client2;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
Window	parent;
Window	dummy;
Window	*children;
Window	base;
unsigned int 	nchild;
int 	i, j;
struct	buildtree *cli1btp;
int 	oldx, oldy;
int 	newx, newy;
XWindowAttributes	atts, batts;


	client1 = XOpenDisplay(config.display);
	client2 = opendisplay();

	if(client1 == (Display *) NULL || client2 == (Display *) NULL) {
		delete("Could not open the display.");
		if(client1)
			XCloseDisplay(client1);
		return;
	} else
		CHECK;

	XSetCloseDownMode(client1, DestroyAll);
	setupsaveset(bt, client1, client2);

	/*
	 * Base will be stored in bt[0][0] all the windows should be reparented
	 * to there.
	 */
	base = bt[0][0].wid;

/* Get the current base window coordinates for absolute calculations */
	XGetWindowAttributes(Dsp, base, &batts);
	if (batts.x != bt[0][0].x || batts.y != bt[0][0].y) {
		report("Window reparenting caused the base window to move");
		report("Pre-reparenting  (%d,%d)" , bt[0][0].x, bt[0][0].y);
		report("Post-reparenting (%d,%d)" , batts.x, batts.y);
		FAIL;
	} else
		CHECK;

	for (i = 0; i < NELEM(T1); i++) {
		for (j = 1; j < NELEM(T2); j++) {

			btp = bt[i] + j;

			/*
			 * Skip if it shouldn't have been reparented.
			 */
			btp->uflags &= (IN_SAVE_SET|CLIENT1_INFERIOR);
			if (btp->uflags != (IN_SAVE_SET|CLIENT1_INFERIOR))
				continue;

			/* Get new parent */
			XQueryTree(Dsp, btp->wid, &dummy, &parent, &children, &nchild);
			if (children && nchild > 0)
				XFree((char*)children);
				
			if (parent != base) {
				report("Window not reparented to correct parent");
				FAIL;
			} else
				CHECK;
		}
	}

	for (i = 0; i < NELEM(T1); i++) {

		/*
		 * Get the information relating to the original client1 window
		 * (that no longer exists).
		 */
		cli1btp = bt[i];

		for (j = 1; j < NELEM(T2); j++) {
			btp = bt[i] + j;

			/*
			 * Skip if it shouldn't have been reparented.
			 */
			btp->uflags &= (IN_SAVE_SET|CLIENT1_INFERIOR);
			if (btp->uflags != (IN_SAVE_SET|CLIENT1_INFERIOR))
				continue;

			debug(1, "window %s", btp->name);

			oldx = bt[0][0].x + bt[0][0].borderwidth + cli1btp->x + cli1btp->borderwidth + btp->x;
			oldy = bt[0][0].y + bt[0][0].borderwidth + cli1btp->y + cli1btp->borderwidth + btp->y;

			(void) XGetWindowAttributes(Dsp, btp->wid, &atts);

			newx = batts.x + batts.border_width + atts.x;
			newy = batts.y + batts.border_width + atts.y;

			/*
			 * Most R4 servers seem to have a bug here.  When the window
			 * to which the windows are being reparented is not at the
			 * origin, then the windows get displaced by an amount equal
			 * to the position of the window.
			 */
			if (oldx != newx || oldy != newy) {
				report("Coords relative to root changed after reparenting");
				report(" (%d, %d), expecting (%d, %d)"
					, newx, newy, oldx, oldy);
				FAIL;
			} else
				CHECK;
		}
	}


	CHECKPASS(2 + NUM_REPARENT + NUM_REPARENT);

>>ASSERTION Good A
When the closedown mode of the client is
.S DestroyAll ,
then a call to xname performs a
.S XMapWindow
request on all unmapped windows in the client's save-set.
>># The strategy and code is very similar to a test for addtsvst.m, and it 
>># should stay that way.
>>STRATEGY
Open a display using XOpenDisplay.
Create save set with setupsaveset.
Close the display using xname.
Get all map events.
Compare each event against expected values.
Verify that map events were received for all save-set windows that
were inferiors of client1 or were not originally mapped.
Verify that map events were received for the parents of all save-set
windows that were inferiors of client1 or were not originally mapped.
>>CODE
Display		*client1, *client2;
XEvent	ev;
XMapEvent	good;
XMapEvent	*mp;
Window	base;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
int 	loop;
int 	i, j;


	client1 = XOpenDisplay(config.display);
	client2 = opendisplay();

	if(client1 == (Display *) NULL || client1 == (Display *) NULL) {
		delete("Could not open the display.");
		if(client1)
			XCloseDisplay(client1);
		return;
	} else
		CHECK;

	XSetCloseDownMode(client1, DestroyAll);
	setupsaveset(bt, client1, client2);

	base = bt[0][0].wid;

	defsetevent(good, Dsp, MapNotify);
	good.override_redirect = False;

	loop = 0;
	while (getevent(Dsp, &ev)) {
		if (ev.type != MapNotify)
			continue;
		mp = (XMapEvent*)&ev;

		/*
		 * Find the window that this event refers to.
		 */
		for (i = 0, btp = NULL; btp == NULL && i < NELEM(T1); i++)
			btp = btwtobtp(bt[i], mp->window);

		if (btp == NULL) {
			report("Map event received for unrecognised window");
			FAIL;
			XCloseDisplay(client1);
			return;
		}

		if (mp->event == mp->window) {
			/* On window itself */
			if (btp->uflags & ON_WINDOW) {
				report("Repeated map event on window");
				FAIL;
			} else if (loop == 0)
				CHECK;
			btp->uflags |= ON_WINDOW;
			good.event = btp->wid;
		} else {
			if (btp->uflags & ON_PARENT) {
				report("Repeated map event on parent of window");
				FAIL;
			} else if (loop == 0)
				CHECK;
			btp->uflags |= ON_PARENT;
			good.event = base;
		}

		good.window = btp->wid;

		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else if (loop == 0)
			CHECK;

		loop++;
	}

	for (i = 0; i < NELEM(T1); i++) {
		for (j = 1; j < NELEM(T2); j++) {
			btp = bt[i] + j;

			debug(1, "window %s", btp->name);

			/*
			 * A map should be received when:
			 *    IN_SAVE_SET && CLIENT1_INFERIOR
			 * || IN_SAVE_SET && BT_UNMAP
			 */
			if (( (btp->uflags & IN_SAVE_SET) &&
				(btp->uflags & CLIENT1_INFERIOR) )
			|| ( (btp->uflags & IN_SAVE_SET) &&
				(btp->opts & BT_UNMAP) ))
			{

				if (btp->uflags & ON_WINDOW) {
					CHECK;
				} else {
					report("Map event was not received on window");
					FAIL;
				}
				if (btp->uflags & ON_PARENT) {
					CHECK;
				} else {
					report("Map event was not received on parent of window");
					FAIL;
				}
			} else if (btp->uflags & (ON_WINDOW|ON_PARENT)) {

				if ((btp->uflags & IN_SAVE_SET) == 0) {
					report("Map event was received for window that was not in save-set");
					FAIL;
				}
				if ((btp->opts & BT_UNMAP) == 0) {
					report("Map event was received for window that was already mapped");
					FAIL;
				}
			} else {
				CHECK;
				CHECK;	/* For balance with above */
			}

		}
	}

	CHECKPASS(1 + 2 + (2*NELEM(T1)*(NELEM(T2)-1)));

>>SET startup fontfocusstartup
>>SET cleanup fontfocuscleanup
>>ASSERTION Good A
When the closedown mode of the client is
.S DestroyAll ,
then a call to xname destroys all
.S Window ,
.S Font ,
.S Pixmap ,
>># COMMENT : .S Bitmap ,
>># COMMENT : Removed since Pixmap is a bitmap.
>># - Cal.
.S Colormap ,
.S Cursor
and
.S GContext
resources allocated by the client.
>>STRATEGY
Open a display using XOpenDisplay.
Set the closedown mode of the display to DestroyAll using XSetCloseDownMode.
Create a font using XLoadFont.
Create a window using XCreateSimpleWindow.
Create a pixmap using XCreatePixmap.
Create a colormap using XCreateColormap.
Create a cursor using XCreateFontCursor.
Create a GC using XCreateGC.
Close the display using xname.
Free the GC using XFreeGC.
Verify that a BadGC error occurred.
Free the cursor using XFreeCursor.
Verify that a BadCursor error occurred.
Free the colormap using XFreeColormap.
Verify that a BadColor error occurred.
Draw on the pixmap with XDrawPoint.
Verify that a BadDrawable error occurred.
Draw on the window using XDrawPoint.
Verify that a BadDrawable error occurred.
Free the font using XUnloadFont.
Verify that a BadFont error occurred.
>>CODE
Font		font;
Window		win;
Pixmap		pm;
Colormap	cmap;
Cursor		curse;
GC		gc;
char		*fontname = "xtfont1";

	/* 
	 * Only really need to set up test font path if a later test purpose
	 * (which resets font path on close of last client with mode DestroyAll)
	 * has been executed before this test purpose.
	 */
	setxtfontpath();

	OPEN(display, DestroyAll)

	
	DELETE_ONERR(font = XLoadFont(display, fontname),
			"XLoadFont");

	DELETE_ONERR(win = XCreateSimpleWindow(display, DRW(display), 0,0, 5,5, 1, 1, 1), 
			"XCreateSimpleWindow");

	DELETE_ONERR(pm = XCreatePixmap(display, DRW(display), 5,6, DefaultDepth(display, DefaultScreen(display))),
			"XCreatePixmap");

	DELETE_ONERR(cmap = XCreateColormap(display, DRW(display), DefaultVisual(display, DefaultScreen(display)), AllocNone),
			"XCreateColormap");

	DELETE_ONERR(curse = XCreateFontCursor(display, config.fontcursor_good),
			"XCreateFontCursor");

	DELETE_ONERR(gc = XCreateGC(display, DRW(display), 0L, 0L),
			"XCreateGC.");

	PROTECT(XCloseDisplay(display))
	sleep(config.speedfactor);

	FAIL_ONERR3(XFreeGC(Dsp, gc), BadGC,
			"%s() did not destroy GC id %lx", gc);

	FAIL_ONERR3(XFreeCursor(Dsp, curse), BadCursor,
			"%s() did not destroy cursor id %lx", curse);

	FAIL_ONERR3(XFreeColormap(Dsp, cmap), BadColor,
			"%s() did not destroy colormap id %lx.", cmap);


	FAIL_ONERR3(XDrawPoint(Dsp, pm, DefaultGC(Dsp, DefaultScreen(Dsp)), 0,0), BadDrawable,
			"%s() did not destroy pixmap id %lx.", pm)


	FAIL_ONERR3(XDrawPoint(Dsp, win, DefaultGC(Dsp, DefaultScreen(Dsp)), 0,0), BadDrawable,
			"%s() did not destroy window id %lx.", win)

	FAIL_ONERR3(XUnloadFont(Dsp, font), BadFont,
			"%s() did not unload font id %ld.", font)

	CHECKPASS(14);

>>ASSERTION Good A
When the last connection to the X server closes with a closedown mode of
.S DestroyAll ,
then the server destroys all lingering resources from clients that have
terminated in
.S RetainPermanent
or
.S RetainTemporary
mode.
>>STRATEGY
Set the closedown mode of client Dsp to DestroyAll using XSetCloseDownMode.
Open 2 clients with XOpenDisplay.
Set the closedown mode of the first to RetainPermanent using XSetCloseDownMode.
Set the closedown mode of the second to RetainTemporary using XSetCloseDownMode.
Create a font for each client using XLoadFont.
Create a window for each client using XCreateSimpleWindow.
Create a pixmap for each client using XCreatePixmap.
Create a colormap for each client using XCreateColormap.
Create a cursor for each client using XCreateFontCursor.
Create a gc for each client using XCreateGC.
Close the 2 clients.
Close client Dsp using xname.

Open client Dsp using XOpenDisplay.
Free the GCs using XFreeGC.
Verify that a BadGC error occurred.
Free the cursors using XFreeCursor.
Verify that a BadCursor error occurred.
Free the colormaps using XFreeColormap.
Verify that a BadColor occurred.
Draw on the pixmaps with XDrawPoint.
Verify that a BadDrawable error occurred.
Draw on the windows using XDrawPoint.
Verify that a BadDrawable error occurred.
Free the fonts using XUnloadFont.
Verify that a BadFont error occurred.
>>CODE
Display		*d[2];
Font		font[2];
Window		win[2];
Pixmap		pm[2];
Colormap	cm[2];
Cursor		curse[2];
GC		gc[2];
int		i;
char		*fontname = "xtfont1";

	/* 
	 * Only really need to set up test font path if a later test purpose
	 * (which resets font path on close of last client with mode DestroyAll)
	 * has been executed before this test purpose.
	 */
	setxtfontpath();

	XSetCloseDownMode(Dsp, DestroyAll);

	OPEN(d[0], RetainPermanent);   /* CHECK */
	OPEN(d[1], RetainTemporary);   /* CHECK */

	for(i=0; i<2; i++) {
		display = d[i];
		DELETE_ONERR(font[i] = XLoadFont(display, fontname), "XLoadFont");
		DELETE_ONERR(win[i] = XCreateSimpleWindow(display, DRW(display), 0,0, 5,5, 1, 1, 1), "XCreateSimpleWindow");
		DELETE_ONERR(pm[i] = XCreatePixmap(display, DRW(display), 5,6, DefaultDepth(display, DefaultScreen(display))),	"XCreatePixmap");
		DELETE_ONERR(cm[i] = XCreateColormap(d[i], DRW(d[i]), DefaultVisual(d[i], DefaultScreen(d[i])), AllocNone), "XCreateColormap");
		DELETE_ONERR(curse[i] = XCreateFontCursor(display, config.fontcursor_good), "XCreateFontCursor");
		DELETE_ONERR(gc[i] = XCreateGC(display, DRW(display), 0L, 0L), "XCreateGC.");
	}
		 /* 12 CHECKS */

	PROTECT(XCloseDisplay(d[0]));   /* CHECK */
	PROTECT(XCloseDisplay(d[1]));   /* CHECK */
	PROTECT(XCloseDisplay(Dsp));    /* CHECK */

	/*
	 * Pause whilst the X server is resetting.
	 */
	reset_delay();

	OPEN(Dsp, DestroyAll);          /* CHECK */

	for(i=0; i<2; i++) {

		FAIL_ONERR4(XUnloadFont(Dsp, font[i]), BadFont,
			"%s() did not unload font id %lx%s.", font[i], cmode[i])

		FAIL_ONERR4(XDrawPoint(Dsp, pm[i], DefaultGC(Dsp, DefaultScreen(Dsp)), 0,0), BadDrawable,
			"%s() did not destroy pixmap id %lx%s.", pm[i], cmode[i])

		FAIL_ONERR4(XDrawPoint(Dsp, win[i], DefaultGC(Dsp, DefaultScreen(Dsp)), 0,0), BadDrawable,
			"%s() did not destroy window id %lx%s.", win[i], cmode[i])

		FAIL_ONERR4(XFreeColormap(Dsp, cm[i]), BadColor,
			"%s() did not destroy colourmap %lx%s.", cm[i], cmode[i]);

		FAIL_ONERR4(XFreeCursor(Dsp, curse[i]), BadCursor,
			"%s() did not destroy cursor id %lx%s.", curse[i], cmode[i]);

		FAIL_ONERR4(XFreeGC(Dsp, gc[i]), BadGC,
			"%s() did not destroy GC id %lx%s", gc[i], cmode[i]);

	}

		/* 12 CHECKS */

	CHECKPASS(2 + 12 + 4 + 12);

>>EXTERN
Atom	predefd[] = {	XA_PRIMARY,
			XA_SECONDARY,
			XA_ARC,
			XA_ATOM,
			XA_BITMAP,
			XA_CARDINAL,
			XA_COLORMAP,
			XA_CURSOR,
			XA_CUT_BUFFER0,
			XA_CUT_BUFFER1,
			XA_CUT_BUFFER2,
			XA_CUT_BUFFER3,
			XA_CUT_BUFFER4,
			XA_CUT_BUFFER5,
			XA_CUT_BUFFER6,
			XA_CUT_BUFFER7,
			XA_DRAWABLE,
			XA_FONT,
			XA_INTEGER,
			XA_PIXMAP,
			XA_POINT,
			XA_RECTANGLE,
			XA_RESOURCE_MANAGER,
			XA_RGB_COLOR_MAP,
			XA_RGB_BEST_MAP,
			XA_RGB_BLUE_MAP,
			XA_RGB_DEFAULT_MAP,
			XA_RGB_GRAY_MAP,
			XA_RGB_GREEN_MAP,
			XA_RGB_RED_MAP,
			XA_STRING,
			XA_VISUALID,
			XA_WINDOW,
			XA_WM_COMMAND,
			XA_WM_HINTS,
			XA_WM_CLIENT_MACHINE,
			XA_WM_ICON_NAME,
			XA_WM_ICON_SIZE,
			XA_WM_NAME,
			XA_WM_NORMAL_HINTS,
			XA_WM_SIZE_HINTS,
			XA_WM_ZOOM_HINTS,
			XA_MIN_SPACE,
			XA_NORM_SPACE,
			XA_MAX_SPACE,
			XA_END_SPACE,
			XA_SUPERSCRIPT_X,
			XA_SUPERSCRIPT_Y,
			XA_SUBSCRIPT_X,
			XA_SUBSCRIPT_Y,
			XA_UNDERLINE_POSITION,
			XA_UNDERLINE_THICKNESS,
			XA_STRIKEOUT_ASCENT,
			XA_STRIKEOUT_DESCENT,
			XA_ITALIC_ANGLE,
			XA_X_HEIGHT,
			XA_QUAD_WIDTH,
			XA_WEIGHT,
			XA_POINT_SIZE,
			XA_RESOLUTION,
			XA_COPYRIGHT,
			XA_NOTICE,
			XA_FONT_NAME,
			XA_FAMILY_NAME,
			XA_FULL_NAME,
			XA_CAP_HEIGHT,
			XA_WM_CLASS,
			XA_WM_TRANSIENT_FOR,
			XA_LAST_PREDEFINED
};

>>ASSERTION Good A
When the last connection to the X server closes with a closedown mode of
.S DestroyAll ,
then the server deletes all but the predefined atom IDs.
>>STRATEGY
Set the closedown mode to DestroyAll using XSetCloseDownMode.
Create and intern the atom XT_TEST_Atom using XInternAtom.
Close all the server displays.
Open the display.
Verify that the atom XT_TEST_Atom no longer exists.
Verify that each of the predefined atoms exists.
>>CODE
int	i;

	display = Dsp;
	XSetCloseDownMode(display, DestroyAll);
	(void) XInternAtom(display, atname, False);
	PROTECT(XCloseDisplay(display));

	/*
	 * Pause whilst the X server is resetting.
	 */
	reset_delay();

	OPEN(Dsp, DestroyAll);
	display = Dsp;

	if(XInternAtom(display, atname, True) != None) {
		report("The atom XT_TEST_Atom was not destroyed.");
		FAIL;
	} else
		CHECK;

	for(i = 0; i< NELEM(predefd); i++) {
	char *name;

		startcall(display);
		name = XGetAtomName(display, predefd[i]);
		endcall(display);
		if((name == (char *) NULL) || (geterr() != Success)) {
			report("Predefined atom number %ld was destroyed.", (long) predefd[i]);
			FAIL;
		} else {
			CHECK;
			XFree((char*)name);
		}
	}

	CHECKPASS(3 + NELEM(predefd));
>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S DestroyAll ,
then the server deletes all properties on all the root windows.
>>STRATEGY
For each screen:
  Obtain the root window of the screen using RootWindow.
  Set the XA_PRIMARY property on the window using XChangeProperty.
Close all connections to the server using xname.
Open a server connection using XOpenDisplay.
For each screen: 
  Obtain the value of the XA_PRIMARY property using XGetWindowProperty.
  Verify that returned type was None, the returned format was 0 and 
  that the bytes_after_return was 0.
>>CODE
int			i;
int			scount;
Atom			at;
unsigned char		*data = (unsigned char *) "XT_property data.";
unsigned char		*rprop= (unsigned char *) ".................";
int			len = 1 + strlen((char*)data);
Atom			type;
int			format = -1;
unsigned long		nitems;
unsigned long		ba;

	scount = ScreenCount(Dsp);
	XSetCloseDownMode(Dsp, DestroyAll);
	if((at = XInternAtom(Dsp, atname, False)) == None) {
		delete("XInternAtom() returned None.");
		return;
	}

	at = XA_PRIMARY;

	for(i=0; i< scount; i++)
		XChangeProperty(Dsp, RootWindow(Dsp, i), at, XA_STRING, 8, PropModeReplace, data, len);	


	PROTECT(XCloseDisplay(Dsp));

	/*
	 * Pause whilst the X server is resetting.
	 */
	reset_delay();

	OPEN(Dsp, DestroyAll);

	for(i=0; i<scount; i++) {
		/* Set to non-zero values to show that they are being written to */
		type = None+1;
		format = 1;
		ba = 1;
		if( XGetWindowProperty(Dsp, RootWindow(Dsp, i), at , 0L, (long) len, False, AnyPropertyType, &type, &format, &nitems, &ba, &rprop) != Success ) {
			delete("XGetWindowProperty() did not return Success for atom XA_PRIMARY.");
			return;
		} else
			CHECK;

		if((type != None) || (format != 0) || (ba != 0) ) {
			report("%s() did not destroy all properties on root window of screen %d.", TestName, i);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS( 2 * scount + 2);


>>ASSERTION Good B 1
When the last connection to the server closes with a closedown mode of
.S DestroyAll ,
then the server resets all device maps, attributes and the access control list.
>>ASSERTION Good B 1
When the last connection to the server closes with a closedown mode of
.S DestroyAll ,
then the server restores the standard root tiles and cursors.
>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S DestroyAll ,
then the server restores the default font path.
>>STRATEGY
Set the closedown mode of a connection to DestroyAll.
Set the font path to XT_FONTPATH using XSetFontPath
Close all connections to the server using xname.
Open a connection using XOpenDisplay.
Set the closedown mode of the connection to DestroyAll.
Set the font path to XT_FONTPATH_GOOD using XSetFontPath
Close all connections to the server using xname.
Verify that the font path is restored in each case to the same font path.
Set the font path to the restored font path using XSetFontPath.
Verify that no error occurs.
Verify that the cursor font can be accessed.
>>EXTERN
int
setfontpath(disp, fpathlist, var)
Display	*disp;
char	*fpathlist;
char	*var;
{
char	*fpathtmp;
char	*ndir_array[MAX_DIRS];
char	*strtok();
int 	ndirs;
int 	i;

	/*
	 * Set font path to specified list.
	 */
	if (fpathlist == NULL || *fpathlist == '\0') {
		delete("%s not set in config file", var);
		return(0);
	}
	fpathtmp = (char *)calloc(strlen(fpathlist)+1, sizeof(char));
	strcpy(fpathtmp, fpathlist);

	for (i = 0; i < MAX_DIRS; i++) {
		ndir_array[i] = strtok((i==0)? fpathtmp: (char*)0, SEP);
		if (ndir_array[i] == NULL)
			break;
		debug(1, "ndir_array entry %d - '%s'", i, ndir_array[i]);
	}
	ndirs = i;

	if (ndirs <= 0) {
		delete("No components in supplied parameter %s", var);
		return(0);
	}
	XSetFontPath(disp, ndir_array, ndirs);

	/*
	 * Sync and clear out queue.
	 */
	XSync(disp, True);
	return(1);
}
>>CODE
char	**path1;
char	**path2;
int 	nret1;
int 	nret2;
int 	i;
Cursor	qstat;
unsigned int shape;

	shape = config.fontcursor_good;
	if (shape == -1) {
		delete("A value of UNSUPPORTED is not allowed for XT_FONTCURSOR_GOOD");
		return;
	}
	if (config.fontpath == NULL || *config.fontpath == '\0') {
		delete("XT_FONTPATH not set");
		return;
	}
	if (config.fontpath_good == NULL || *config.fontpath_good == '\0') {
		delete("XT_FONTPATH_GOOD not set");
		return;
	}
	if(!strcmp(config.fontpath, config.fontpath_good)) {
		delete("XT_FONTPATH and XT_FONTPATH_GOOD must not be identical");
		return;
	}
	XSetCloseDownMode(Dsp, DestroyAll);
	if(!setfontpath(Dsp, config.fontpath, "XT_FONTPATH"))
		return;
	PROTECT(XCloseDisplay(Dsp));

	/*
	 * Pause whilst the X server is resetting.
	 */
	reset_delay();

	OPEN(Dsp, DestroyAll);

	/* Save font path, hypothesise that this is the default */
	path1 = XGetFontPath(Dsp, &nret1);

	XSetCloseDownMode(Dsp, DestroyAll);
	if(!setfontpath(Dsp, config.fontpath_good, "XT_FONTPATH_GOOD"))
		return;
	PROTECT(XCloseDisplay(Dsp));

	/*
	 * Pause whilst the X server is resetting.
	 */
	reset_delay();

	OPEN(Dsp, DestroyAll);

	/* Check saved fontpaths are the same */
	path2 = XGetFontPath(Dsp, &nret2);

	if (nret1 == nret2)
		CHECK;
	else {
		report("Number of directories was not set to default");
		report("  was %d, expecting %d", nret2, nret1);
		FAIL;
	}

	if(nret1 < nret2)
		nret2 = nret1;
	for (i = 0; i < nret2; i++) {
		debug(1, "Using XT_FONTPATH, got list item '%s'", path1[i]);
		debug(1, "Using XT_FONTPATH_GOOD, got list item '%s'", path2[i]);
		if (strcmp(path1[i], path2[i]) == 0)
			CHECK;
		else {
			report("Font path component was not set to default");
			report("  was '%s', expecting '%s'", path2[i], path1[i]);
			FAIL;
		}
	}
	
	startcall(Dsp);
	XSetFontPath(Dsp, path1, nret1);
	endcall(Dsp);
	if (geterr() != Success) {
		report("When XSetFontPath() is used with restored font path,");
		report("got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	startcall(Dsp);
	qstat = XCreateFontCursor(Dsp, shape);
	endcall(Dsp);

	/* Verify that XCreateFontCursor returns non-zero. */
	if (qstat == 0) {
		report("When XCreateFontCursor() is used with restored font path,");
		report("returned wrong value %ld", (long) qstat);
		FAIL;
	} else
		CHECK;
	if (geterr() != Success) {
		report("When XCreateFontCursor() is used with restored font path,");
		report("got %s, Expecting Success", errorname(geterr()));
		FAIL;
	} else
		CHECK;

	XFreeFontPath(path1);
	XFreeFontPath(path2);
	CHECKPASS(8+nret1);

>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S DestroyAll ,
then the server restores the input focus to
.S PointerRoot .
>>STRATEGY
Set the closedown mode of a connection to DestroyAll.
Set the input focus of a connection to None with XSetInputFocus.
Close all connections to the server using xname.
Open a connection using XOpenDisplay.
Obtain the current input focus using XGetInputFocus.
Verify that the input focus is PointerRoot.
>>CODE
Window	fr;
int	rr;

	XSetCloseDownMode(Dsp, DestroyAll);
	XSetInputFocus(Dsp, None, RevertToNone, CurrentTime);
	PROTECT(XCloseDisplay(Dsp));

	/*
	 * Pause whilst the X server is resetting.
	 */
	reset_delay();

	OPEN(Dsp, DestroyAll);
	XGetInputFocus(Dsp, &fr, &rr);	

	if(fr != PointerRoot) {
		report("%s() set the input focus to %lx instead of PointerRoot (%lx).", TestName, (long) fr, (long) PointerRoot);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>#	
>># Retain
>>#
>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not affect any 
window in the client's save-set.
>>STRATEGY
Open a display using XOpenDisplay.
Set the closedown mode of the display to RetainPermanent using XSetCloseDownMode.
Create save set with setupsaveset.
Close the display using xname.
Verify parents and positions of the save-set members stay the same.
Open a display using XOpenDisplay.
Set the closedown mode of the display to RetainTemporary using XSetCloseDownMode.
Create save set with setupsaveset.
Close the display using xname.
Verify parents and positions of the save-set members stay the same.
>>CODE
Display	*client1, *client2;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
Window	parent;
Window	dummy;
Window	*children;
Window	base;
char	*modestr[2];
unsigned int 	nchild;
int 	i, j, k;
struct	buildtree *cli1btp;
int 	oldx, oldy;
int 	newx, newy;
XWindowAttributes	atts;


	modestr[0] = "RetainPermanent";
	modestr[1] = "RetainTemporary";

	for(k=0; k<2; k++) {
	
		client1 = XOpenDisplay(config.display);
		client2 = opendisplay();
	
		if(client1 == (Display *) NULL || client1 == (Display *) NULL) {
			delete("Could not open the display.");
			if(client1)
				XCloseDisplay(client1);
			return;
		} else
			CHECK;
	
		XSetCloseDownMode(client1, closemodes[k]);
		setupsaveset(bt, client1, client2);
	
		/*
		 * Base will be stored in bt[0][0] all the windows should be reparented
		 * to there.
		 */
		base = bt[0][0].wid;
	
		for (i = 0; i < NELEM(T1); i++) {
			for (j = 1; j < NELEM(T2); j++) {
	
				btp = bt[i] + j;
	
				/*
				 * Skip if it shouldn't have been reparented.
				 */
				btp->uflags &= (IN_SAVE_SET|CLIENT1_INFERIOR);
				if (btp->uflags != (IN_SAVE_SET|CLIENT1_INFERIOR))
					continue;
	
				/* Get new parent */
				XQueryTree(Dsp, btp->wid, &dummy, &parent, &children, &nchild);
				if (children && nchild > 0)
					XFree((char*)children);
					
				if (parent != btp->parent->wid) {
					report("Closedown mode %s:", modestr[k]);
					report("Window did not retain its original parent.");
					FAIL;
				} else
					CHECK;
			}
		}
	

		for (i = 0; i < NELEM(T1); i++) {
	
			/*
			 * Get the information relating to the original client1 window
			 */
			cli1btp = bt[i];
	
			for (j = 1; j < NELEM(T2); j++) {
				btp = bt[i] + j;
	
				/*
				 * Skip if it shouldn't have been reparented.
				 */
				btp->uflags &= (IN_SAVE_SET|CLIENT1_INFERIOR);
				if (btp->uflags != (IN_SAVE_SET|CLIENT1_INFERIOR))
					continue;
	
				debug(1, "window %s", btp->name);
	
				oldx = btp->x;
				oldy = btp->y;
	
				(void) XGetWindowAttributes(Dsp, btp->wid, &atts);
	
				newx = atts.x;
				newy = atts.y;
	
				if (oldx != newx || oldy != newy) {
					report("Closedown mode %s:", modestr[k]);
					report("Coords relative to parent changed");
					report(" (relative to parent win got (%d, %d), expecting (%d, %d)"
						, newx, newy, oldx, oldy);
					FAIL;
				} else
					CHECK;
			}
		}

	}

	CHECKPASS(2 * (1 + NUM_REPARENT + NUM_REPARENT));

>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not delete any
.S Window ,
.S Font ,
.S Pixmap ,
>># .S Bitmap ,
>>#  Pixmap === Bitmap, anyhow!
.S Colormap ,
.S Cursor
and
.S GContext
resources allocated by the client.
>>STRATEGY
Set the closedown mode of the display to RetainPermanent using XSetCloseDownMode.
Create a font using XLoadFont.
Create a window using XCreateSimpleWindow.
Create a pixmap using XCreatePixmap.
Create a colormap using XCreateColormap.
Create a cursor using XCreateFontCursor.
Create a GC using XCreateGC.

Close the display using xname.
Open the display using XOpenDisplay.
Free the GC using XFreeGC.
Verify that no errors occurred.
Free the cursor using XFreeCursor.
Verify that no errors occurred.
Free the colormap using XFreeColormap.
Verify that no errors occurred.
Draw on the pixmap with XDrawPoint.
Verify that no errors occurred.
Draw on the window using XDrawPoint.
Verify that no errors occurred.
Free the font using XUnloadFont.
Verify that no errors occurred.

Set the closedown mode of the display to RetainTemporary using XSetCloseDownMode.
Create a font using XLoadFont.
Create a window using XCreateSimpleWindow.
Create a pixmap using XCreatePixmap.
Create a colormap using XCreateColormap.
Create a cursor using XCreateFontCursor.
Create a GC using XCreateGC.

Close the display using xname.
Open the display using XOpenDisplay.
Free the GC using XFreeGC.
Verify that no errors occurred.
Free the cursor using XFreeCursor.
Verify that no errors occurred.
Free the colormap using XFreeColormap.
Verify that no errors occurred.
Draw on the pixmap with XDrawPoint.
Verify that no errors occurred.
Draw on the window using XDrawPoint.
Verify that no errors occurred.
Free the font using XUnloadFont.
Verify that no errors occurred.
>>CODE
int		k;
Font		font;
Window		win;
Pixmap		pm;
Colormap	cmap;
Cursor		curse;
GC		gc;
char		*fontname = "xtfont1";

	/* 
	 * Need to set up test font path - when test purposes are all
	 * executed in default order, an earlier test purpose will have 
	 * reset font path on close of last client with mode DestroyAll.
	 */
	setxtfontpath();

	for(k=0; k<2; k++) {
	
		XSetCloseDownMode(display, closemodes[k]);
		
		DELETE_ONERR(font = XLoadFont(display, fontname),
				"XLoadFont");
	
		DELETE_ONERR(win = XCreateSimpleWindow(display, DRW(display), 0,0, 5,5, 1, 1, 1), 
				"XCreateSimpleWindow");
	
		DELETE_ONERR(pm = XCreatePixmap(display, DRW(display), 5,6, DefaultDepth(display, DefaultScreen(display))),
				"XCreatePixmap");
	
		DELETE_ONERR(cmap = XCreateColormap(display, DRW(display), DefaultVisual(display, DefaultScreen(display)), AllocNone),
				"XCreateColormap");
	
		DELETE_ONERR(curse = XCreateFontCursor(display, config.fontcursor_good),
				"XCreateFontCursor");
	
		DELETE_ONERR(gc = XCreateGC(display, DRW(display), 0L, 0L),
				"XCreateGC");
	
		PROTECT(XCloseDisplay(display))

		/*
		 * Pause whilst the X server is resetting.
		 */
		reset_delay();

		OPEN(display, DestroyAll);
		/*
		 * The next line is important - Dsp is used in the macros
		 * for example. Although we could use `display' below, 
		 * rather than Dsp, I think its clearer to use Dsp.
		 */
		Dsp = display;
	
		FAIL_ONERR2(XFreeGC(Dsp, gc),
				"%s() destroyed GC id %lx%s.", gc, cmode[k]);
	
		FAIL_ONERR2(XFreeCursor(Dsp, curse),
				"%s() destroyed cursor id %lx%s.", curse, cmode[k]);
	
		FAIL_ONERR2(XFreeColormap(Dsp, cmap),
				"%s() destroyed colormap id %lx%s.", cmap, cmode[k]);
	
	
		FAIL_ONERR2(XDrawPoint(Dsp, pm, DefaultGC(Dsp, DefaultScreen(Dsp)), 0,0),
				"%s() destroyed pixmap id %lx%s.", pm, cmode[k])
	
		FAIL_ONERR2(XDrawPoint(Dsp, win, DefaultGC(Dsp, DefaultScreen(Dsp)), 0,0),
				"%s() destroyed window id %lx%s.", win, cmode[k])
	
		FAIL_ONERR2(XUnloadFont(Dsp, font),
				"%s() unloaded font id %ld%s.", font, cmode[k])
	}

	CHECKPASS(2 * 14);

>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not delete any of the atom IDs.
>>STRATEGY
Set the closedown mode to RetainPermanent using XSetCloseDownMode.
Create and intern the atom XT_TEST_Atom using XInternAtom.
Close all connections to the server using xname.
Open a server connection using XOpenDisplay.
Verify that the atom XT_TEST_Atom is still interned.
Verify that all the predefined atoms still exist.

Set the closedown mode to RetainTemporary using XSetCloseDownMode.
Create and intern the atom XT_TEST_Atom using XInternAtom.
Close all connections to the server using xname.
Open a server connection using XOpenDisplay.
Verify that the atom XT_TEST_Atom is still interned.
Verify that all the predefined atoms still exist.
>>CODE
int		i;
int		k;
char		*name;

	for(k=0; k<2; k++) {

		XSetCloseDownMode(Dsp, closemodes[k]);
		(void) XInternAtom(Dsp, atname, False);
		PROTECT(XCloseDisplay(Dsp));

		/*
		 * Pause whilst the X server is resetting.
		 */
		reset_delay();

		OPEN(Dsp, closemodes[k]); 	

		if(XInternAtom(Dsp, atname, True) == None) {
			report("The atom XT_TEST_Atom was destroyed.");
			FAIL;
		} else
			CHECK;
	
		for(i = 0; i< NELEM(predefd); i++) {
	
			startcall(Dsp);
			name = XGetAtomName(Dsp, predefd[i]);
			endcall(Dsp);
			if((name == (char *) NULL)  || (geterr() == BadAtom)) {
				report("%s() %s destroyed predefined atom number %ld.",TestName, cmode[k], (long) predefd[i]);
				FAIL;
			} else {
				CHECK;
				XFree((char*)name);
			}
		}
	
	}

	CHECKPASS(2 * (3 + NELEM(predefd)));

>>ASSERTION Good B 1
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not delete any property on any root window.
>>ASSERTION Good B 1
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not affect any device map, any attributes or the access control list.
>>ASSERTION Good B 1
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not affect the standard root tiles and cursors.
>>ASSERTION Good B 1
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not affect the default font path.
>>ASSERTION Good A
When the last connection to the server closes with a closedown mode of
.S RetainPermanent
or
.S RetainTemporary ,
then a call to xname does not affect the input focus.
>>STRATEGY
Set the closedown mode to RetainPermanent using XSetCloseDownMode.
Set the input focus to None with XSetInputFocus.
Close all connections to the server using xname.
Open a connection using XOpenDisplay.
Obtain the current input focus using XGetInputFocus.
Verify that the input focus is None.

Set the closedown mode to RetainTemporary using XSetCloseDownMode.
Set the input focus to None with XSetInputFocus.
Close all connections to the server using xname.
Open a connection using XOpenDisplay.
Obtain the current input focus using XGetInputFocus.
Verify that the input focus is None.
>>CODE
Window	fr;
int	rr;
int	k;

	for(k=0; k<2; k++) {

		XSetCloseDownMode(Dsp, closemodes[k]);
		XSetInputFocus(Dsp, None, RevertToNone, CurrentTime);
		PROTECT(XCloseDisplay(Dsp));

		/*
		 * Pause whilst the X server is resetting.
		 */
		reset_delay();

		OPEN(Dsp, DestroyAll);
		XGetInputFocus(Dsp, &fr, &rr);	
		
		if(fr != None) {
			report("%s() set the input focus to %lx instead of None (%lx)%s.",
				 TestName, (long) fr, (long) None, cmode[k]);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2 * 3);
	
