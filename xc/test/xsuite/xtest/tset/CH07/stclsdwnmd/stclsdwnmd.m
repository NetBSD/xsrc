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
 * $XConsortium: stclsdwnmd.m,v 1.4 94/04/17 21:06:50 rws Exp $
 */
>>TITLE XSetCloseDownMode CH07
void

Display	*display = Dsp;
int 	close_mode;
>>#
>># The way that closedown mode acts on the last connection close to
>># a server is covered in XCloseDisplay rather than here.
>># There is more overlap with XCloseDisplay.
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good A
When
.A close_mode
is set to
.S DestroyAll
and the client is subsequently closed down,
then all
.S Window ,
.S Font ,
.S Pixmap ,
>># .S Bitmap ,
.S Colormap ,
.S Cursor
and
.S GContext
resources allocated by the client are destroyed.
>>STRATEGY
Create a new connection client1.
Create resources for client1.
Call xname on client1 with close_mode of DestroyAll.
Close client1.
Verify that the resources are destroyed.
>>CODE
Display	*client1;
Window	win;
Font	font;
Pixmap	pix;
Colormap	colmap;
Cursor	cursor;
GC  	gc;

	regdisable();
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}

	win = defwin(client1);
	font = XLoadFont(client1, "xtfont0");
	pix = maketile(client1, win);
	colmap = makecolmap(client1, DefaultVisual(client1, DefaultScreen(client1)),
		AllocNone);
	cursor = makecur(client1);
	gc = makegc(client1, win);
	regenable();

	XSync(client1, False);
	if (isdeleted())
		return;

	display = client1;
	close_mode = DestroyAll;
#ifdef TESTBED
	close_mode = RetainTemporary;
#endif
	XCALL;

	XCloseDisplay(client1);

	/*
	 * Attempt to destroy all the resources.  Since they should have already
	 * been destroyed, then we expect the appropriate Bad<Resource> error
	 * for each one.
	 */
	CATCH_ERROR(Dsp);
	XDestroyWindow(Dsp, win);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadWindow)
		CHECK;
	else {
		report("Window resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XUnloadFont(Dsp, font);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadFont)
		CHECK;
	else {
		report("Font resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreePixmap(Dsp, pix);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadPixmap)
		CHECK;
	else {
		report("Pixmap resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeColormap(Dsp, colmap);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadColor)
		CHECK;
	else {
		report("Colormap resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeCursor(Dsp, cursor);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadCursor)
		CHECK;
	else {
		report("Cursor resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeGC(Dsp, gc);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadGC)
		CHECK;
	else {
		report("GC resource was not destroyed");
		FAIL;
	}

	CHECKPASS(6);
>>ASSERTION Good A
When
.A close_mode
is set to
.S DestroyAll
and the client is subsequently closed down,
then all the windows in the client's save-set that are
inferiors of a window created by the client are reparented, 
with no change in position relative to the root window,
to the closest ancestor that is not an inferior of the client's window.
>>STRATEGY
Create new connection client1.
Create window with client1.
Create inferior of window with Dsp.
Add this window to client1's save-set.
Call xname on client1 with close_mode of DestroyAll.
Close client1.
Verify that save-set window is reparented with no change in position.
>>CODE
Display	*client1;
Window	base;
Window	win;
Window	inf;
struct	area	area;
XWindowAttributes	atts;

	base = defwin(Dsp);

	if ((client1 = XOpenDisplay(config.display)) == 0) {
		report("Could not open display");
		FAIL;
	}

	win = crechild(client1, base, (struct area *)0);
	XSync(client1, False);

	setarea(&area, 3, 4, 4, 4);
	inf = crechild(Dsp, win, &area);
	if (isdeleted())
		return;

	XAddToSaveSet(client1, inf);

	display = client1;
	close_mode = DestroyAll;
	XCALL;

	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, inf, &atts) == False) {
		report("save-set window was destroyed");
		FAIL;
	} else
		CHECK;
	RESTORE_ERROR(Dsp);

	/*
	 * Compare coord relative to base window.
	 */
	if (atts.x != area.x || atts.y != area.y) {
		report("Absolute co-ordinates relative to root changed");
		report("  (relative to base was (%d, %d) expecting (%d, %d))",
			atts.x, atts.y,
			area.x, area.y);
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When
.A close_mode
is set to
.S DestroyAll
and the client is subsequently closed down,
then
all windows in the client's save set that were not mapped are mapped.
>>STRATEGY
Create new connection client1.
Create window with client1.
Create unmapped inferior of window with Dsp.
Add this window to client1's save-set.
Call xname on client1 with close_mode of DestroyAll.
Verify that save-set window is mapped.
>>CODE
Display	*client1;
Window	base;
Window	win;
Window	inf;
struct	area	area;
XWindowAttributes	atts;

	base = defwin(Dsp);

	if ((client1 = XOpenDisplay(config.display)) == 0) {
		report("Could not open display");
		FAIL;
	}

	win = crechild(client1, base, (struct area *)0);
	XSync(client1, False);

	setarea(&area, 3, 4, 4, 4);
	inf = creunmapchild(Dsp, win, &area);
	if (isdeleted())
		return;

	XAddToSaveSet(client1, inf);

	display = client1;
	close_mode = DestroyAll;
#ifdef TESTBED
	close_mode = RetainPermanent;
#endif
	XCALL;

	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, inf, &atts) == False) {
		report("save-set window was destroyed");
		FAIL;
	} else
		CHECK;
	RESTORE_ERROR(Dsp);

	if (atts.map_state == IsUnmapped) {
		report("save-set window was not mapped after save-set processing");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>#
>># Retain
>>#
>>ASSERTION Good A
When
.A close_mode
is
.S RetainPermanent
and the client is subsequently closed down,
then the resources allocated by the client are not
destroyed and are marked as permanent.
>>STRATEGY
Create client1.
Create resource for client1.
Call xname for client1 with close_mode of RetainPermanent.
Close client1.
Verify that client1 resource still exists.
Call XKillClient with AllTemporary.
Verify that client1 resource still exists.
>>CODE
Display	*client1;
Window	win;
Font	font;
Pixmap	pix;
Colormap	colmap;
Cursor	cursor;
GC  	gc;
XWindowAttributes	atts;

	regdisable();
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}

	win = defwin(client1);
	font = XLoadFont(client1, "xtfont0");
	pix = maketile(client1, win);
	colmap = makecolmap(client1, DefaultVisual(client1, DefaultScreen(client1)),
		AllocNone);
	cursor = makecur(client1);
	gc = makegc(client1, win);
	regenable();

	XSync(client1, False);
	if (isdeleted())
		return;

	display = client1;
	close_mode = RetainPermanent;
#ifdef TESTBED
	close_mode = RetainTemporary;
#endif
	XCALL;

	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, win, &atts) == False) {
		report("Client1 resources destroyed with RetainPermanent");
		FAIL;
		return;
	} else
		CHECK;

	XKillClient(Dsp, AllTemporary);

	/*
	 * Attempt to destroy all the resources.  Each resource should
	 * still exist and so all the destroy operations should be
	 * Successful.
	 */
	CATCH_ERROR(Dsp);
	XDestroyWindow(Dsp, win);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == Success)
		CHECK;
	else {
		report("Window resource was destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XUnloadFont(Dsp, font);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == Success)
		CHECK;
	else {
		report("Font resource was destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreePixmap(Dsp, pix);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == Success)
		CHECK;
	else {
		report("Pixmap resource was destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeColormap(Dsp, colmap);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == Success)
		CHECK;
	else {
		report("Colormap resource was destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeCursor(Dsp, cursor);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == Success)
		CHECK;
	else {
		report("Cursor resource was destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeGC(Dsp, gc);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == Success)
		CHECK;
	else {
		report("GC resource was destroyed");
		FAIL;
	}

	CHECKPASS(7);
>>ASSERTION Good A
When
.A close_mode
is
.S RetainTemporary
and the client is subsequently closed down,
then all resources allocated by the client are not destroyed and
are marked as temporary.
>>STRATEGY
Create client1.
Create resource for client1.
Call xname for client1 with close_mode of RetainTemporary.
Close client1.
Verify that client1 resource still exists.
Call XKillClient with AllTemporary.
Verify that client1 resource no longer exists.
>>CODE
Display	*client1;
Window	win;
Font	font;
Pixmap	pix;
Colormap	colmap;
Cursor	cursor;
GC  	gc;
XWindowAttributes	atts;

	regdisable();
	if ((client1 = opendisplay()) == 0) {
		delete("Could not open display");
		return;
	}

	win = defwin(client1);
	font = XLoadFont(client1, "xtfont0");
	pix = maketile(client1, win);
	colmap = makecolmap(client1, DefaultVisual(client1, DefaultScreen(client1)),
		AllocNone);
	cursor = makecur(client1);
	gc = makegc(client1, win);
	regenable();

	XSync(client1, False);
	if (isdeleted())
		return;

	display = client1;
	close_mode = RetainTemporary;
#ifdef TESTBED
	close_mode = DestroyAll;
#endif
	XCALL;

	XCloseDisplay(client1);

	CATCH_ERROR(Dsp);
	if (XGetWindowAttributes(Dsp, win, &atts) == False) {
		report("Window was destroyed after close-down in RetainTemporary mode");
		FAIL;
	} else
		CHECK;
	RESTORE_ERROR(Dsp);

	/* No point of continuing if failed so far */
	if (fail)
		return;

	XKillClient(Dsp, AllTemporary);

	/*
	 * Attempt to destroy all the resources.  Since they should have already
	 * been destroyed, then we expect the appropriate Bad<Resource> error
	 * for each one.
	 */
	CATCH_ERROR(Dsp);
	XDestroyWindow(Dsp, win);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadWindow)
		CHECK;
	else {
		report("Window resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XUnloadFont(Dsp, font);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadFont)
		CHECK;
	else {
		report("Font resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreePixmap(Dsp, pix);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadPixmap)
		CHECK;
	else {
		report("Pixmap resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeColormap(Dsp, colmap);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadColor)
		CHECK;
	else {
		report("Colormap resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeCursor(Dsp, cursor);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadCursor)
		CHECK;
	else {
		report("Cursor resource was not destroyed");
		FAIL;
	}

	CATCH_ERROR(Dsp);
	XFreeGC(Dsp, gc);
	RESTORE_ERROR(Dsp);
	if (GET_ERROR(Dsp) == BadGC)
		CHECK;
	else {
		report("GC resource was not destroyed");
		FAIL;
	}

	CHECKPASS(7);
>>ASSERTION Good A
When
.A close_mode
is
.S RetainPermanent
or
.S RetainTemporary
and the client is subsequently closed down,
then all windows in the client's save-set are unaffected.
>>STRATEGY
Create new connection client1.
Create window with client1.
Create unmapped inferior of window with Dsp.
Add this window to client1's save-set.
Call xname on client1 with close_mode of RetainPermanent.
Close client1 connection.
Verify that save-set window is not mapped.
>>CODE
Display	*client1;
Window	base;
Window	win;
Window	inf;
struct	area	area;
XWindowAttributes	atts;
int 	i;
static	int 	modes[] = {
	RetainPermanent, RetainTemporary
#ifdef TESTBED
	,DestroyAll
#endif
	};

	base = defwin(Dsp);

	for (i = 0; i < NELEM(modes); i++) {
		if ((client1 = XOpenDisplay(config.display)) == 0) {
			report("Could not open display");
			FAIL;
		}

		win = crechild(client1, base, (struct area *)0);
		XSync(client1, False);

		setarea(&area, 3, 4, 4, 4);
		inf = creunmapchild(Dsp, win, &area);
		if (isdeleted())
			return;

		XAddToSaveSet(client1, inf);

		display = client1;
		close_mode = modes[i];
		XCALL;

		XCloseDisplay(client1);

		CATCH_ERROR(Dsp);
		if (XGetWindowAttributes(Dsp, inf, &atts) == False) {
			report("save-set window was destroyed");
			FAIL;
		} else
			CHECK;
		RESTORE_ERROR(Dsp);

		if (atts.map_state == IsUnmapped)
			CHECK;
		else {
			report("For close-down mode %d", modes[i]);
			report("save-set window was affected");
			FAIL;
		}
	}

	CHECKPASS(2*NELEM(modes));
>>ASSERTION Bad A
.ER BadValue close_mode DestroyAll RetainPermanent RetainTemporary
