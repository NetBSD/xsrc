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
 * $XConsortium: addtsvst.m,v 1.10 94/04/17 21:06:05 rws Exp $
 */
>>TITLE XAddToSaveSet CH07
void
xname
Display	*display = Dsp;
Window	w;
>>EXTERN

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
setupsaveset(btret)
struct	buildtree	*btret[NELEM(T1)];
{
Display	*client1;
Display	*client2;
Window	base;
struct	buildtree	*bt1;
struct	buildtree	*btp;
XWindowAttributes	batts;
int 	i, j;
int 	pass = 0, fail = 0;

	client1 = XOpenDisplay(config.display);
	client2 = opendisplay();
	if (client1 == NULL || client2 == NULL) {
		delete("Could not open display");
		return;
	}

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
			display = client1;
			w = btp->wid;

			/*
			 * j==0 is the client1 window.
			 * j==1 and j==2 are not added to
			 * the save-set; the rest are.
			 */
			if (j > NOADD) {
				btp->uflags |= IN_SAVE_SET;
				XCALL;
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

	XCloseDisplay(client1);
	sleep(config.speedfactor);

	XSync(Dsp, False);
	XSync(client2, False);

}

>>ASSERTION Good A
A call to xname adds the specified window to the client's save-set.
>># We could test the following in XCloseDisplay etc.
>>STRATEGY
Create client1.
Create windows for client1.
Create client2.
Create windows for client2 that are inferiors of the windows created by
client1.
Add some of client2's windows to client1's save-set with xname.
Close client1.
Verify that the inferiors that were added to the save-set still exist
and that the ones that were not have been destroyed.
>>CODE
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
XWindowAttributes	atts;
int 	i, j;

	setupsaveset(bt);

	CATCH_ERROR(Dsp);
	for (i = 0; i < NELEM(T1); i++) {
		for (j = 1; j < NELEM(T2); j++) {
			btp = bt[i] + j;
			debug(1, "window %s", btp->name);

			if ((btp->uflags & CLIENT1_INFERIOR) == 0) {
				if (XGetWindowAttributes(Dsp, btp->wid, &atts) == True)
					CHECK;
				else {
					report("Window that was not inferior was destroyed");
					FAIL;
				}
			} else if (btp->uflags & IN_SAVE_SET) {
				if (XGetWindowAttributes(Dsp, btp->wid, &atts) == True)
					CHECK;
				else {
					report("Window in save-set destroyed");
					FAIL;
				}
			} else {
				if (XGetWindowAttributes(Dsp, btp->wid, &atts) == False)
					CHECK;
				else {
					report("Window not in save-set was not destroyed");
					FAIL;
				}
			}
		}
	}
	RESTORE_ERROR(Dsp);

	CHECKPASS(NELEM(T1)*(NELEM(T2)-1));
>>ASSERTION Good A
When the client's resources are destroyed,
>># Close or KILL CLIENT.
and the window specified in the call to xname is an inferior of
one of the windows created by the client,
then the specified window is reparented to the closest ancestor window
such that the save-set window is not an inferior of a window
created by the client.
>>STRATEGY
Setup windows so that they have been reparented, using setupsaveset.
Verify that each window that should have been reparented has the correct parent.
>>CODE
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
Window	parent;
Window	dummy;
Window	*children;
Window	base;
unsigned int 	nchild;
int 	i, j;

	setupsaveset(bt);

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
				
			if (parent != base) {
				report("Window not reparented to correct parent");
				FAIL;
			} else
				CHECK;
		}
	}
	CHECKPASS(NUM_REPARENT);
>>ASSERTION Good A
When a save-set window is reparented as a result of save-set
processing, then the absolute coordinates
of the upper-left outer corner of the save-set window are unchanged.
>>STRATEGY
Setup reparented windows with setupsaveset(). 
Obtain base window coordinates.
Verify the base window has not moved.
For each reparented window:
  Calculate original position relative to base window.
  Obtain new position of window.
  Verify that window has not moved relative to root window.
>>CODE
XWindowAttributes	batts, atts;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
struct	buildtree *cli1btp;
int 	oldx, oldy;
int 	newx, newy;
int 	i, j;

	setupsaveset(bt);

	XGetWindowAttributes(Dsp, bt[0][0].wid, &batts);
	if (batts.x != bt[0][0].x || batts.y != bt[0][0].y) {
		report("Window reparenting caused the base window to move");
		report("Pre-reparenting  (%d,%d)" , bt[0][0].x, bt[0][0].y);
		report("Post-reparenting (%d,%d)" , batts.x, batts.y);
		FAIL;
	} else
		CHECK;

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

			oldx = bt[0][0].x + bt[0][0].borderwidth + 
				cli1btp->x + cli1btp->borderwidth + btp->x;
			oldy = bt[0][0].y + bt[0][0].borderwidth +
				cli1btp->y + cli1btp->borderwidth + btp->y;

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
	CHECKPASS(1 + NUM_REPARENT);
>>ASSERTION Good A
When a save-set window is reparented as a result of save-set
processing and it was originally mapped, then
the window is unmapped before being reparented and
.S UnmapNotify
events are generated.
>>STRATEGY
Create save set with setupsaveset.
Get all unmap events.
Compare event against expected values.
Verify that unmap events were received for all save-set windows that
were inferiors of client1.
>>CODE
XEvent	ev;
XUnmapEvent	good;
XUnmapEvent	*ump;
Window	base;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
int 	loop;
int 	i, j;

	setupsaveset(bt);

	base = bt[0][0].wid;

	defsetevent(good, Dsp, UnmapNotify);
	good.from_configure = False;

	loop = 0;
	while (getevent(Dsp, &ev)) {
		if (ev.type != UnmapNotify)
			continue;
		ump = (XUnmapEvent*)&ev;

		/*
		 * Find the window that this event refers to.
		 */
		for (btp = NULL, i = 0; btp == NULL && i < NELEM(T1); i++)
			btp = btwtobtp(bt[i], ump->window);

		if (btp == NULL) {
			report("Unmap event received for unrecognised window");
			FAIL;
			return;
		}

		if (ump->event == ump->window) {
			/* On window itself */
			if (btp->uflags & ON_WINDOW) {
				report("Repeated unmap event on window");
				FAIL;
			} else if (loop == 0)
				CHECK;
			btp->uflags |= ON_WINDOW;
			good.event = btp->wid;
		} else {
			/* Shouldn't happen at all */
			if (btp->uflags & ON_PARENT) {
				report("Repeated unmap event on parent of window");
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

	loop = 0;
	for (i = 0; i < NELEM(T1); i++) {
		for (j = 1; j < NELEM(T2); j++) {
			btp = bt[i] + j;

			debug(1, "window %s", btp->name);

			/*
			 * An unmap should be received when:
			 *   Was mapped && In save set && Was client1 inferior
			 */
			if (       (btp->uflags & IN_SAVE_SET)
					&& (btp->uflags & CLIENT1_INFERIOR)
					&& (btp->opts & BT_UNMAP) == 0
					) {

				if (btp->uflags & ON_WINDOW) {
					if (loop == 0)
						CHECK;
				} else {
					report("Unmap event was not received");
					FAIL;
				}
			} else if (btp->uflags & ON_WINDOW) {

				if ((btp->uflags & CLIENT1_INFERIOR) == 0) {
					report("Unmap event was received for window that was not");
					report("  an inferior of the destroyed client");
					FAIL;
				}
				if ((btp->uflags & IN_SAVE_SET) == 0) {
					report("Unmap event was received for window that was not in save-set");
					FAIL;
				}
				if (btp->opts & BT_UNMAP) {
					report("Unmap event was received for window that was already not mapped");
					FAIL;
				}
			} else {
				if (loop == 0)
					CHECK;
			}

			loop++;
		}
	}

	CHECKPASS(2 + 1);
>>ASSERTION Good A
When a save-set window is reparented as a result of save-set processing, then a
.S ReparentNotify
event is generated on the window and the new parent window.
>>STRATEGY
Create save set with setupsaveset.
Get all reparent events.
Compare event against expected values.
Verify that reparent events were received for all save-set windows that
were inferiors of client1.
Verify that reparent events were received for the parents of all save-set
windows that were inferiors of client1.
>>CODE
XEvent	ev;
XReparentEvent	good;
XReparentEvent	*rpp;
Window	base;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
int 	loop;
int 	i, j;

	setupsaveset(bt);

	base = bt[0][0].wid;

	defsetevent(good, Dsp, ReparentNotify);
	good.parent = base;
	good.override_redirect = False;

	loop = 0;
	while (getevent(Dsp, &ev)) {
		if (ev.type != ReparentNotify)
			continue;
		rpp = (XReparentEvent*)&ev;

		/*
		 * Find the window that this event refers to.
		 */
		for (i = 0, btp = NULL; btp == NULL && i < NELEM(T1); i++)
			btp = btwtobtp(bt[i], rpp->window);

		if (btp == NULL) {
			report("Reparent event received for unrecognised window");
			FAIL;
			return;
		}

		if (rpp->event == rpp->window) {
			/* On window itself */
			if (btp->uflags & ON_WINDOW) {
				report("Repeated reparent event on window");
				FAIL;
			} else if (loop == 0)
				CHECK;
			btp->uflags |= ON_WINDOW;
			good.event = btp->wid;
		} else {
			if (btp->uflags & ON_PARENT) {
				report("Repeated reparent event on parent of window");
				FAIL;
			} else if (loop == 0)
				CHECK;
			btp->uflags |= ON_PARENT;
			good.event = base;
		}

		good.window = btp->wid;
		good.x = btp->x + btp->parent->borderwidth + btp->parent->x;
		good.y = btp->y + btp->parent->borderwidth + btp->parent->y;

		if (checkevent((XEvent*)&good, &ev))
			FAIL;
		else if (loop == 0)
			CHECK;

		loop++;
	}

	loop = 0;
	for (i = 0; i < NELEM(T1); i++) {
		for (j = 1; j < NELEM(T2); j++) {
			btp = bt[i] + j;

			debug(1, "window %s", btp->name);

			/*
			 * A reparent should be received when:
			 *   In save set && Was client1 inferior
			 */
			if (       (btp->uflags & IN_SAVE_SET)
					&& (btp->uflags & CLIENT1_INFERIOR)
					) {

				if (btp->uflags & ON_WINDOW) {
					if (loop == 0)
						CHECK;
				} else {
					report("Reparent event was not received on window");
					FAIL;
				}
				if (btp->uflags & ON_PARENT) {
					if (loop == 0)
						CHECK;
				} else {
					report("Reparent event was not received on parent of window");
					FAIL;
				}
			} else if (btp->uflags & (ON_WINDOW|ON_PARENT)) {

				if ((btp->uflags & CLIENT1_INFERIOR) == 0) {
					report("Reparent event was received for window that was");
					report("  not an inferior of the destroyed client");
					FAIL;
				}
				if ((btp->uflags & IN_SAVE_SET) == 0) {
					report("Reparent event was received for window that was not in save-set");
					FAIL;
				}
			} else {
				if (loop == 0)
					CHECK;
			}

			loop++;
		}
	}

	CHECKPASS(2 + 1);
>>ASSERTION Good A
When a client is destroyed,
then all its save-set windows that are not mapped or that are reparented as
a result of save-set processing are mapped and
.S MapNotify
or
.S MapRequest
events are generated.
>>STRATEGY
Create save set with setupsaveset.
Get all map events.
Compare each event against expected values.
Verify that map events were received for all save-set windows that
were inferiors of client1 or were not originally mapped.
Verify that map events were received for the parents of all save-set
windows that were inferiors of client1 or were not originally mapped.
>>CODE
XEvent	ev;
XMapEvent	good;
XMapEvent	*mp;
Window	base;
struct	buildtree *bt[NELEM(T1)];
struct	buildtree *btp;
int 	loop;
int 	i, j;

	setupsaveset(bt);

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

	CHECKPASS(2 + (2*NELEM(T1)*(NELEM(T2)-1)));
>>ASSERTION Bad A
When the specified window was created by the same client, then a
.S BadMatch
error occurs.
>>STRATEGY
Create a window using default client.
Call xname with this client and window.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	w = defwin(display);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadWindow
