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
 * $XConsortium: chngwdwatt.m,v 1.17 94/04/17 21:02:52 rws Exp $
 */
>>TITLE XChangeWindowAttributes CH03
void

Display	*display = Dsp;
Window	w = DRW(Dsp);
unsigned long	valuemask = 0;
XSetWindowAttributes	*attributes = &Atts;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN

static XSetWindowAttributes	Atts;
static Window  parent;
static int     x = 50;
static int     y = 60;
static unsigned int    border_width = 2;
static int     depth;
static Visual  *visual;

>>ASSERTION Good A
A call to xname changes the window attributes specified in
.A valuemask
to the values in the
.A attributes
structure.
>>STRATEGY
Create window with default attributes.
Change the window attributes to non-default values.
Get the window attributes with XGetWindowAttributes.
Verify that they are the same as the ones set.
>>EXTERN
#include	"cursorfont.h"
>>CODE
XWindowAttributes	getatts;
Colormap	cm;
Cursor		curs;
int 	n;

	cm = XCreateColormap(display, DRW(display), DefaultVisual(display, DefaultScreen(display)), AllocNone);
	curs = XCreateFontCursor(display, XC_coffee_mug);

	attributes->bit_gravity = SouthEastGravity;
	attributes->win_gravity = EastGravity;
	attributes->backing_store = WhenMapped;
	attributes->backing_planes = 0xaaaaaaaa;
	attributes->backing_pixel = 1;
	attributes->save_under = True;
	attributes->event_mask = PropertyChangeMask;
	attributes->do_not_propagate_mask = KeyPressMask;
	attributes->override_redirect = True;
	attributes->colormap = cm;
	attributes->cursor = curs;

	valuemask = CWBitGravity|CWWinGravity|CWBackingStore|CWBackingPlanes|
		CWBackingPixel|CWSaveUnder|CWEventMask|CWDontPropagate|
		CWOverrideRedirect|CWColormap|CWCursor;

	w = makeinout( DRW(display), (Visual *) CopyFromParent,
			CopyFromParent, XCWA_NORMAL);

	XCALL;

	XGetWindowAttributes(display, w, &getatts);
	if (isdeleted())
		return;

	XDestroyWindow(display, w);

	n = checkatts(attributes, &getatts, valuemask);
	if (n > 0) {
		report("There %s %d incorrect attribute%s", 
					(n>1)?"were":"was", n, (n>1)?"s":"");
		FAIL;
	} else if (n < 0) {
		/* already reported a path check error in checkatts */
		return;
	} else
		CHECK;

	CHECKPASS(1);

>>EXTERN
>># This is a copy of the identical function in crtwdw.m
>>#   Perhaps it should be in a library (or in commattr.mc) 
>>#					stuart

static int
checkatts(setatts, getatts, vmask)
XSetWindowAttributes	*setatts;
XWindowAttributes	*getatts;
unsigned long 	vmask;
{
int 	pass = 0, fail = 0;

	/*
	 * Maybe we should alway check everything??? (No vmask)
	 */
	if ((vmask&CWBitGravity) && setatts->bit_gravity != getatts->bit_gravity) {
		report("bit_gravity got %s, expected %s",
			bitgravityname(getatts->bit_gravity),
			bitgravityname(setatts->bit_gravity));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWWinGravity) && setatts->win_gravity != getatts->win_gravity) {
		report("window_gravity got %s, expected %s",
			wingravityname(getatts->win_gravity),
			wingravityname(setatts->win_gravity));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWBackingStore) && setatts->backing_store != getatts->backing_store) {
		report("backing_store got %s, expected %s",
			backingstorename(getatts->backing_store),
			backingstorename(setatts->backing_store));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWBackingPlanes) && setatts->backing_planes != getatts->backing_planes) {
		report("backing_planes got 0x%x, expected 0x%x",
			getatts->backing_planes,
			setatts->backing_planes);
		FAIL;
	} else
		CHECK;

	if (setatts->backing_pixel != getatts->backing_pixel) {
		report("backing_pixel got 0x%x, expected 0x%x",
			getatts->backing_pixel,
			setatts->backing_pixel);
		FAIL;
	} else
		CHECK;

	if ((vmask&CWSaveUnder) && setatts->save_under != getatts->save_under) {
		report("save_under got %s, expected %s",
			boolname(getatts->save_under),
			boolname(setatts->save_under));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWEventMask) && setatts->event_mask != getatts->your_event_mask) {
		report("event_mask got %s, expected %s",
			eventmaskname(getatts->your_event_mask),
			eventmaskname(setatts->event_mask));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWDontPropagate) && setatts->do_not_propagate_mask != getatts->do_not_propagate_mask) {
		report("do_not_propagate_mask got %s, expected %s",
			eventmaskname(getatts->do_not_propagate_mask),
			eventmaskname(setatts->do_not_propagate_mask));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWOverrideRedirect) && setatts->override_redirect != getatts->override_redirect) {
		report("override_redirect got %s, expected %s",
			boolname(getatts->override_redirect),
			boolname(setatts->override_redirect));
		FAIL;
	} else
		CHECK;

	if ((vmask&CWColormap) && setatts->colormap != getatts->colormap) {
		report("colormap got 0x%x, expected 0x%x",
			getatts->colormap,
			setatts->colormap);
		FAIL;
	} else
		CHECK;

	if (fail == 0 && pass == 10)
		return(0);
	else {
		if (fail)
			return(fail);
		else
			delete("Path check error in checkatts");
	}
	return(-1);
}

>>ASSERTION Good A
When the background is changed, then the window contents do not change.
>># Until the next Expose event?	kieron
>>#
>># I am assuming that PIXCHECK does not cause such an event.
>>#					stuart
>>STRATEGY
Create a window.
Set the background-pixmap
Map window over a plain background.
Change background-pixmap to a patterned tile.
Verify that background has not changed.
>>CODE
XEvent event;
XVisualInfo	*vp;

	for(resetvinf(VI_WIN); nextvinf(&vp); ) {
		Pixmap	pm;
		parent = makedrawable(display, vp);

		w = makeinout(parent, (Visual *)CopyFromParent,
				CopyFromParent, XCWA_NORMAL);

		XSelectInput(display, w, ExposureMask);
		pm = makepixm(display,vp);
		XSetWindowBackgroundPixmap(display, w, pm);
		XMapWindow(display, w);
					/* Wait for window to be exposed */
		XWindowEvent(display,w,ExposureMask, &event);

		valuemask = CWBackPixmap;
		attributes->background_pixmap = maketile(display, w);

		XCALL;

		if ( isdeleted() )
			continue;

		PIXCHECK(display, parent);

	}

	if (getevent(display, &event) != 0)
		delete("Unexpected event: %s", eventname(event.type));
	else
		CHECKPASS(nvinf());

>>ASSERTION Good A
When the border is changed
>># What does the next bit mean?
>># Answer: The border tile orgin == backround tile origin so changing
>>#	    the latter (to the parent's tile origin iff background_pixmap
>>#		== ParentRelative, else to the window's origin) may cause
>>#		may cause a retiling.		kieron
or the background set such that the border tile origin changes,
then the border is repainted.
>>STRATEGY
Create a window.
Set the border-pixmap
Map window over a patterned background.
Change border-pixmap 
Verify that border has changed by pixel checking.
Create a window, with no border, and a tiled background.
Create a child window, with a parent relative background and a border pixmap.
Map windows.
Save image.
Change the child window background causing the border pixmap to be retiled
because the border_tile origin will change.
Verify the border changed.
>>CODE
struct	area	child_pos;
unsigned int tile_width, tile_height, t3_border_width;
Pixmap	bpm, tpm;
XVisualInfo	*vp;
XImage	*i_before;

	t3_border_width = 10;

/* This tests the border changing, but not the second condition */
	for(resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display,vp);

		w = makeinout(parent, (Visual *)CopyFromParent,
				CopyFromParent, XCWA_NORMAL);

		XMapWindow(display,w);
		bpm = makepixm(display, vp);
		XSetWindowBorderPixmap(display, w, bpm);

		attributes->border_pixmap = maketile(display,parent);
		valuemask = CWBorderPixmap;

		XCALL;

		if ( isdeleted() )
			continue;

		PIXCHECK(display, parent);

/* Now check change of background causes border retiling */
	
/* Make a 1 border width parent, with tpm as the tile */

		parent = makewin(display, vp);
		XUnmapWindow(display, parent);

		tpm = maketile(display, parent);
		getsize(display, tpm, &tile_width, &tile_height);

		valuemask = CWBackPixmap;
		attributes->background_pixmap = tpm;
		w = parent;
		XCALL;

		XMapWindow(display, parent);

 /*This numerical offset is the amount we expect the border pm to move by */
		child_pos.x = tile_width + 2; 
		child_pos.y = tile_height+ 2;	

		while(child_pos.x <= t3_border_width) {
			child_pos.x += tile_width;
		}
		while(child_pos.y <= t3_border_width) {
			child_pos.y += tile_height;
		}
		child_pos.width = 30;
		child_pos.height= 30;

		w = crechild(display, parent, &child_pos);
		XUnmapWindow(display, w);
		XSetWindowBorderWidth(display, w, t3_border_width);
		valuemask = CWBorderPixmap | CWBackPixmap;
		attributes->background_pixmap = ParentRelative;
		attributes->border_pixmap = tpm;

		XCALL;
		XMapWindow(display, w);
		XSync(display,True);

		i_before = savimage(display, parent);
		valuemask = CWBackPixmap;
		attributes->background_pixmap = tpm;
		XCALL;
/* Window contents are expected to have changed */
		if(diffsavimage(display, parent, i_before))
		{
			report("Changing the border tile origin by changing");
			report("the background pixmap from ParentRelative");
			report("did not cause a border retiling.");
			FAIL;
		}
		else
			CHECK;
	
	}

	CHECKPASS(nvinf()*2);
>>ASSERTION Good B 1
When the background-pixmap attribute of a root window is set to
.S None ,
then the default background pixmap is restored.
>>ASSERTION Good B 1
When the background-pixmap attribute of a root window is set to
.S ParentRelative ,
then the default background pixmap is restored.
>>ASSERTION Good B 1
When the border-pixmap attribute of a root window is set to
.S CopyFromParent ,
then the default border pixmap is restored.
>>ASSERTION Good A
When the win-gravity attribute is changed, then the current position of the
window is not changed.
>>STRATEGY
Create window
Ascertain window position by calling XGetWindowAttributes
Change win-gravity by calling XChangeWindowAttributes
Ascertain window position by calling XGetWindowAttributes
Verify window position is unchanged.
>>CODE
XWindowAttributes	before,after;

	w = makeinout((Drawable)None, (Visual *)CopyFromParent,
		CopyFromParent, XCWA_GRAVITY);

	XGetWindowAttributes(display, w, &before);

	valuemask = CWWinGravity;
	attributes->win_gravity = SouthWestGravity;

	XCALL;

	XGetWindowAttributes(display, w, &after);

	if( (before.x != after.x) || (before.y != before.y) )
	{
		report("Changing win_gravity changed window positon");
		report("Before: x=%d y=%d", before.x, before.y);
		report("After : x=%d y=%d", after.x, after.y);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When the colormap attribute of a window is changed, then a
.S ColormapNotify
event is generated.
>>STRATEGY
Create window
Select ColormapNotify event
Create Colormap for the window
Change the window colormap by calling xname
Verify that a ColormapNotify event was generated
>>CODE
Colormap cm;
XEvent	ev;
int	events;

	w = makeinout((Drawable)None, (Visual *)CopyFromParent,
		CopyFromParent, XCWA_NORMAL);

	XSelectInput(display, w, ColormapChangeMask);

	cm = makecolmap(display,
		XDefaultVisual( display, XDefaultScreen(display) ),
		AllocNone);

	valuemask = CWColormap;
	attributes->colormap = cm;

	XCALL;

	events = getevent(display, &ev);

	if (events != 1)
	{
		report("Expected one event on the event queue (ColormapNotify)");
		report("Got: %d events.", events);
		report("The first event was of type %s", eventname(ev.type));
		FAIL;
	}
	else
	if( ev.type != ColormapNotify )
	{
		report("Unexpected event received upon changing Colormap");
		report("Expecting: ColormapNotify");
		report("Received:  %s", eventname(ev.type));
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
>># Tested elsewhere, in Chapter 8 I believe.	Kieron.
>>#
>># It is tested elsewhere but only for XSelectInput. I guess its
>># important enough that it should be tested here too.
>># The wording used in XSelectInput is better in that it gets rid of the 
>># word "can" and says what actually happens, but it would need altering 
>># for use here. Here it is anyway :
>>#
>>#When multiple clients make a call to xname
>>#requesting the same event on the same window
>>#and
>>#that window is the event window for the requested event,
>>#then the event is reported to each client.
>>#								Dave
>># Also there are assertions for XSelectInput for the three that should 
>># give errors -but again we need to keep those here (they're all in one 
>># assertion at the moment, that doesn't really matter I suppose.)
>>#
>>#Multiple clients can select the same events on the same window
>>#apart from the event masks
>>#.S SubstructureRedirectMask ,
>>#.S ResizeRedirectMask ,
>>#and
>>#.S ButtonPressMask .
>>#>>>
When more than one client sets a mask other than
.S SubstructureRedirectMask ,
.S ResizeRedirectMask ,
and
.S ButtonPressMask
in the event-mask attribute, then the selected events are delivered
to each such client.
>>STRATEGY
Create client1.
Create window with client1.
Select MapNotify events with client1 on this window.
Create client2.
Select MapNotify events with client2 on this window.
Map window.
Verify that client1 received a single MapNotify event for this window.
Verify that client1 received no other events.
Verify that client2 received a single MapNotify event for this window.
Verify that client2 received no other events.
>>CODE
int	num;
Display *client1;
Display *client2;
XEvent	event;

/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select MapNotify events with client1 on this window. */
	valuemask = CWEventMask;
	attributes->event_mask = StructureNotifyMask;
	BASIC_STARTCALL(client1);
	XChangeWindowAttributes(client1, w, valuemask, attributes);
	BASIC_ENDCALL(client1, Success);
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select MapNotify events with client2 on this window. */
	valuemask = CWEventMask;
	attributes->event_mask = StructureNotifyMask;
	BASIC_STARTCALL(client2);
	XChangeWindowAttributes(client2, w, valuemask, attributes);
	BASIC_ENDCALL(client2, Success);
/* Map window. */
	XSync(client1, True);
	XSync(client2, True);
	XMapWindow(client1, w);
	XSync(client1, False);
	XSync(client2, False);
/* Verify that client1 received a single MapNotify event for this window. */

	if((num = getevent(client1, &event)) == 0)
	{
		report("No events were delivered to client1.");
		report("Expecting a single MapNotify event.");
		FAIL;
	}
	else
	if (event.type != MapNotify)
	{
		report("Selected event was not delivered to client1.");
		report("Delivered event was: %s", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Verify that client1 received no other events. */
	if (num > 1) {
		report("Unexpected events were delivered to client1.");
		report("Expecting a single MapNotify event.");
		report("Got %d events", num);
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received a single MapNotify event for this window. */
	if((num = getevent(client2, &event)) == 0)
	{
		report("No events were delivered to client2.");
		report("Expecting a single MapNotify event.");
		FAIL;
	}
	else
	if (event.type != MapNotify)
	{
		report("Selected event was not delivered to client2.");
		report("Delivered event was: %s", eventname(event.type));
		FAIL;
	}
	else
		CHECK;
/* Verify that client2 received no other events. */
	if (num > 1) {
		report("Unexpected events were delivered to client2.");
		report("Expecting a single MapNotify event.");
		report("Got %d events", num);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(6);
>>ASSERTION Good A
When the event-mask attribute is changed, then the event-mask attribute
does not change for other clients.
>>STRATEGY
Create client1.
Create window with client1.
Select NoEventMask events with client1 on this window.
Call XGetWindowAttributes to get event mask for client1 for window.
Verify event mask is as expected.
Create client2.
Select ALLEVENTS events with client2 on this window.
Call XGetWindowAttributes to get event mask for client2 for window.
Verify event mask is as expected.
Call XGetWindowAttributes to get event mask for client1 for window.
Verify event mask has not changed.
Select KeyPressMask events with client1 on this window.
Call XGetWindowAttributes to get event mask for client1 for window.
Verify event mask is as expected.
Call XGetWindowAttributes to get event mask for client2 for window.
Verify event mask has not changed.
>>CODE
Display *client1;
Display *client2;
XWindowAttributes attrs;

/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
/* Select NoEventMask events with client1 on this window. */
	valuemask = CWEventMask;
	attributes->event_mask = NoEventMask;
	display = client1;

	XCALL;

/* Call XGetWindowAttributes to get event mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != NoEventMask) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Select ALLEVENTS events with client2 on this window. */
	valuemask = CWEventMask;
	attributes->event_mask = ALLEVENTS;
	display = client2;

	XCALL;

/* Call XGetWindowAttributes to get event mask for client2 for window. */
	if (!XGetWindowAttributes(client2, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != ALLEVENTS) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Call XGetWindowAttributes to get event mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask has not changed. */
	if (attrs.your_event_mask != NoEventMask) {
		report("Event mask incorrect.");
		FAIL;
	}
	else
		CHECK;
/* Select KeyPressMask events with client1 on this window. */
	valuemask = CWEventMask;
	attributes->event_mask = KeyPressMask;
	display = client1;

	XCALL;

/* Call XGetWindowAttributes to get event mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask is as expected. */
	if (attrs.your_event_mask != KeyPressMask) {
		delete("Unexpected event mask value.");
		return;
	}
	else
		CHECK;
/* Call XGetWindowAttributes to get event mask for client2 for window. */
	if (!XGetWindowAttributes(client2, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify event mask has not changed. */
	if (attrs.your_event_mask != ALLEVENTS) {
		report("Event mask incorrect.");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(12);

>>ASSERTION Good B 1
When the cursor attribute of a root window is changed to
.S None ,
then the default cursor is restored.
>>STRATEGY
If extended testing is required:
  If the server supports two screens with the same default cursor:
    Set the root window cursor to a non-default cursor.
    Verify that the cursor was set correctly.
    Warp the pointer into the root window.
    Verify that the current cursor is that of the root window.
    Warp the pointer to the alternate root window.
    Verify that the current cursor is not the same as that of the default root window.
    Reset the cursor of the root window to the default cursor using xname.
    Verify that the current cursor is the same as that of the default root window.
  Otherwise :
    Set the root window cursor to a non-default cursor.
    Verify that the cursor was set correctly.
    Warp the pointer to the root window.
    Verify that the current cursor is that of the root window.
    Reset the cursor of the root window to the default cursor using xname.
    Verify that the root window cursor is no longer the non-default cursor.
>>CODE
Window		altroot;
Cursor		cursor;
Bool		samedefcursor;

	/* If extended testing is required: */
	if(noext(0))
		return;

	if(config.alt_screen != -1) {
		(void) warppointer(display, DRW(display), 0,0);
		altroot = RootWindow(display, config.alt_screen);
		samedefcursor = spriteiswin(display, altroot);
	}

		/* If the server supports two screens with the same default cursor: */
	if(config.alt_screen != -1 && samedefcursor) {

			/* Set the root window cursor to a non-default cursor. */
		cursor = makecur(display);
		w = DRW(display);
		XDefineCursor(display, w, cursor);
		
			/* Verify that the cursor was set correctly. */
		if(curofwin(display, cursor, w) == False) {
			delete("XDefineCursor() did not set the root window's cursor correctly.");
			return;
		} else
			CHECK;

			/* Warp the pointer into the root window. */
		(void) warppointer(display, w, 0,0);

			/* Verify that the current cursor is that of the root window. */
		if(spriteiswin(display, w) == False) {
			delete("Current cursor is not that of the root window.");
			return;
		} else
			CHECK;

			/* Warp the pointer to the alternate root window. */
		(void) warppointer(display, altroot, 0,0);

			/* Verify that the current cursor is not the same as that of the default root window. */
		if(spriteiswin(display, DRW(display)) != False) {
			delete("The alternate root window's cursor was not set to the default cursor.");
			return;
		} else
			CHECK;

			/* Reset the cursor of the root window to the default cursor using xname. */
		valuemask = CWCursor;
		attributes->cursor = None;
		XCALL;

			/* Verify that the current cursor is the same as that of the default root window. */
		if(spriteiswin(display, DRW(display)) == False) {
			report("Root window's cursor was not set to the default cursor.");
			FAIL;
		} else
			CHECK;

	 	CHECKPASS(4);
	} else {

		/* Otherwise : */

			/* Set the root window cursor to a non-default cursor. */
		cursor = makecur2(display);
		w = DRW(display);
		XDefineCursor(display, w, cursor);

			/* Verify that the cursor was set correctly. */
		if(curofwin(display, cursor, w) == False) {
			delete("XDefineCursor() did not set the root window's cursor correctly.");
			return;
		} else
			CHECK;

			/* Warp the pointer to the root window. */
		(void) warppointer(display, w, 0,0);

			/* Verify that the current cursor is that of the root window. */
		if(spriteiswin(display, w) == False) {
			delete("Current cursor is not that of the root window.");
			return;
		} else
			CHECK;

			/* Reset the cursor of the root window to the default cursor using xname. */
		valuemask = CWCursor;
		attributes->cursor = None;
		XCALL;

			/* Verify that the root window cursor is no longer the non-default cursor. */
		if(curofwin(display, cursor, w) != False) {
			report("%s() did not set the root window's cursor to the default cursor.", TestName);
			FAIL;
		} else
			CHECK;

		CHECKUNTESTED(3);
	}

>>ASSERTION Good A
When the do-not-propagate-mask attribute is changed, then the change is
effective for all clients.
>>STRATEGY
Create client1.
Create window with client1.
Call XGetWindowAttributes to get do-not-propagate-mask for client1 for window.
Verify do-not-propagate-mask is as expected.
Create client2.
Call XGetWindowAttributes to get do-not-propagate-mask for client2 for window.
Verify do-not-propagate-mask is as expected.
Set do-not-propagate-mask to KeyPressMask events with client1 on this window.
Call XGetWindowAttributes to get do-not-propagate-mask for client1 for window.
Verify do-not-propagate-mask has changed.
Call XGetWindowAttributes to get do-not-propagate-mask for client2 for window.
Verify do-not-propagate-mask has changed.
>>CODE
Display *client1;
Display *client2;
XWindowAttributes attrs;
unsigned long dont_all;

	dont_all = ALLEVENTS & (!0xffffc0b0);	/* This defines the
							SETofDEVICEEVENT */

/* Create client1. */
	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Create window with client1. */
	w = mkwin(client1, (XVisualInfo *) NULL, (struct area *) NULL, False);
	valuemask = CWDontPropagate;
	attributes->do_not_propagate_mask = dont_all;
	display = client1;

	XCALL;

/* Call XGetWindowAttributes to get do-not-propagate-mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify do-not-propagate-mask is as expected. */
	if (attrs.do_not_propagate_mask != dont_all) {
		delete("Unexpected do-not-propagate-mask value.");
		report("Expecting: %s", eventmaskname(dont_all));
		report("Got: %s", eventmaskname(attrs.do_not_propagate_mask));
		return;
	}
	else
		CHECK;
/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Call XGetWindowAttributes to get do-not-propagate-mask for client2 for window. */
	if (!XGetWindowAttributes(client2, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify do-not-propagate-mask is as expected. */
	if (attrs.do_not_propagate_mask != dont_all) {
		delete("Unexpected do-not-propagate-mask value.");
		report("Expecting: %s", eventmaskname(dont_all));
		report("Got: %s", eventmaskname(attrs.do_not_propagate_mask));
		return;
	}
	else
		CHECK;

/* Set do-not-propagate-mask to KeyPressMask events with client1 on this window. */
	valuemask = CWDontPropagate;
	attributes->do_not_propagate_mask = KeyPressMask;
	display = client1;

	XCALL;

/* Call XGetWindowAttributes to get do-not-propagate-mask for client1 for window. */
	if (!XGetWindowAttributes(client1, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify do-not-propagate-mask has changed. */
	if (attrs.do_not_propagate_mask != KeyPressMask) {
		report("Incorrect do-not-propagate-mask.");
		report("Expecting: %s", eventmaskname(KeyPressMask));
		report("Got: %s", eventmaskname(attrs.do_not_propagate_mask));
		FAIL;
	}
	else
		CHECK;
/* Call XGetWindowAttributes to get do-not-propagate-mask for client2 for window. */
	if (!XGetWindowAttributes(client2, w, &attrs)) {
		delete("A call to XGetWindowAttributes failed.");
		return;
	}
	else
		CHECK;
/* Verify do-not-propagate-mask has changed. */
	if (attrs.do_not_propagate_mask != KeyPressMask) {
		report("Incorrect do-not-propagate-mask.");
		report("Expecting: %s", eventmaskname(KeyPressMask));
		report("Got: %s", eventmaskname(attrs.do_not_propagate_mask));
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(10);
>>INCLUDE ../crtwdw/commattr.mc
>>ASSERTION Bad A
When another client has selected with an event mask
.S SubstructureRedirectMask ,
then on a call to xname
with
.S SubstructureRedirectMask
bits set in
.A event_mask
a
.S BadAccess
error occurs.
>>STRATEGY
Create window with client1.
Select SubstructureRedirectMask event mask with client1 on this window.
Create client2.
Call xname with an event_mask set to SubstructureRedirectMask using client2
on this window.
Verify that a BadAccess error was generated.
>>CODE BadAccess
Display *client1;
Display *client2;

	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	} else
		CHECK;

	w = defwin(client1);
	XSelectInput(client1, w, SubstructureRedirectMask);
	XSync(client1, False);

	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	} else
		CHECK;

	valuemask = CWEventMask;
	attributes->event_mask = SubstructureRedirectMask;
	display = client2;

	XCALL;

	if (geterr() != BadAccess) {
		report("A call to %s did not generate a BadAccess error", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
When another client has selected with an event mask
.S ResizeRedirectMask ,
then on a call to xname
with
.S ResizeRedirectMask
bits set in
.A event_mask
a
.S BadAccess
error occurs.
>>STRATEGY
Create client1.
Create window with client1.
Select ResizeRedirectMask event mask with client1 on this window.
Create client2.
Call xname with an event_mask of ResizeRedirectMask using client2 on
this window.
Verify that a BadAccess error was generated.
>>CODE BadAccess
Display *client1;
Display *client2;

	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	} else
		CHECK;

	w = defwin(client1);
	XSelectInput(client1, w, ResizeRedirectMask);
	XSync(client1, False);

	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	} else
		CHECK;

	valuemask= CWEventMask;
	attributes->event_mask = ResizeRedirectMask ;
	display = client2;
	XCALL;

	if (geterr() != BadAccess) {
		report("A call to %s did not generate a BadAccess error", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
>>ASSERTION Bad A
When another client has selected with an event mask
.S ButtonPressMask ,
then on a call to xname
with
.S ButtonPressMask
bits set in
.A event_mask
a
.S BadAccess
error occurs.
>>STRATEGY
Create client1.
Create window with client1.
Select ButtonPressMask event mask with client1 on this window.
Create client2.
Call xname with event_mask ButtonPressMask using client2 on this window.
Verify that a BadAccess error was generated.
>>CODE BadAccess
Display *client1;
Display *client2;

	client1 = opendisplay();
	if (client1 == (Display *) NULL) {
		delete("Can not open display");
		return;
	} else
		CHECK;

	w = defwin(client1);
	XSelectInput(client1, w, ButtonPressMask);
	XSync(client1, False);

	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	} else
		CHECK;

	valuemask= CWEventMask;
	attributes->event_mask = ButtonPressMask ;
	display = client2;
	XCALL;

	if (geterr() != BadAccess) {
		report("A call to %s did not generate a BadAccess error", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);
