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
 * $XConsortium: undfncrsr.m,v 1.12 94/04/17 21:05:57 rws Exp $
 */
>>TITLE XUndefineCursor CH06
void
XUndefineCursor(display, w)
Display *display = Dsp;
Window w;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good B 1
When the pointer is in the
.A window ,
and the 
.A window
is not the root window, 
then the parent's cursor will be used.
>>STRATEGY
If extended testing is required:
  Create a window.
  Set the cursor of that window to a non-default cursor.
  Verify that the parent window's cursor is correctly set.
  Warp the pointer into the parent.
  Verify that the current cursor is that of the parent.
  Create a child of the window.
  Set the cursor of the child to a different cursor.
  Verify that the child window's cursor is correctly set.
  Warp the pointer into the child.
  Verify that the current cursor is not that of the parent.
  Verify that the current cursor is that of the child.
  Undefine the child's cursor using xname.
  Verify that the current cursor is that of the parent window.
Otherwise:
  Create cursor.
  Create windows.
  Define cursor for non-root window.
  Call XUndefineCursor on non-root window.
>>CODE
Cursor		cursor;
Cursor		cursor2;
XVisualInfo	*vp;
Window		parent;
struct area	ar;

	/* If extended testing is required: */
	if(noext(0) == False) {

		/* Create a window. */
		parent = defwin(display);

		cursor2 = makecur2(display);
		/* Set the cursor of that window to a non-default cursor. */
		XDefineCursor(display, parent, cursor2);
		
		/* Verify that the parent window's cursor is correctly set. */
		if(curofwin(display, cursor2, parent) == False) {
			delete("XDefineCursor() did not set the parent's cursor correctly.");
			return;
		} else
			CHECK;

		/* Warp the pointer into the parent. */
		(void) warppointer(display, parent, 0,0);

		/* Verify that the current cursor is that of the parent. */
		if(spriteiswin(display, parent) == False) {
			delete("Current cursor is not that of the parent.");
			return;
		} else
			CHECK;

		cursor = makecur(display);
		/* Create a child of the window. */
		ar.x = 10;
		ar.y = 10;
		ar.width = 20;
		ar.height = 20;
		w = crechild(display, parent, &ar);

		/* Set the cursor of the child to a different cursor. */
		XDefineCursor(display, w, cursor);

		/* Verify that the child window's cursor is correctly set. */
		if(curofwin(display, cursor, w) == False) {
			delete("XDefineCursor() did not set the child's cursor correctly.");
			return;
		} else
			CHECK;

		/* Warp the pointer into the child. */
		(void) warppointer(display, w , 0,0);

		/* Verify that the current cursor is not that of the parent. */
		if(spriteiswin(display, parent) != False) {
			delete("Parent and child have the same cursor.");
			return;
		} else
			CHECK;

		/* Verify that the current cursor is that of the child. */
		if(spriteiswin(display, w) == False) {
			delete("Current cursor is not that of the child.");
			return;
		} else
			CHECK;

		/* Undefine the child's cursor using xname. */
		XCALL;		

		/* Verify that the current cursor is that of the parent window. */
		if(spriteiswin(display, parent) == False) {
			report("Cursor did not change to that of the window.");
			FAIL;
		} else
			CHECK;

		CHECKPASS(6);

	} else {

	/* Otherwise: */

		/* Create cursor. */
		cursor = makecur(display);

		/* Create windows. */
		for (resetvinf(VI_WIN); nextvinf(&vp); ) {
			w = makewin(display, vp);

		/* Define cursor for non-root window. */
			XDefineCursor(display, w, cursor);
	
		/* Call XUndefineCursor on non-root window. */
			XCALL;
			if (geterr() != Success)
				FAIL;
			else
				CHECK;
		}
		CHECKUNTESTED(nvinf());
	}

>>ASSERTION Good B 1
When the pointer is in the
.A window ,
and the 
.A window
is the root window, 
then
the default cursor will be used.
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
Otherwise:
  Create cursor.
  Set window to root window.
  Define cursor for root window.
  Call XUndefineCursor on root window.
>>CODE
Window		altroot;
Cursor		cursor;
XVisualInfo	*vp;
Bool		samedefcursor;

	/* If extended testing is required: */
	if(noext(0) == False) {

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
			XCALL;

			/* Verify that the root window cursor is no longer the non-default cursor. */
			if(curofwin(display, cursor, w) != False) {
				report("%s() did not set the root window's cursor to the default cursor.", TestName);
				FAIL;
			} else
				CHECK;

			CHECKUNTESTED(3);
		}


	} else {

	/* Otherwise: */

		/* Create cursor. */
		cursor = makecur(display);
	
		/* Set window to root window. */
		w = DRW(display);
	
		/* Define cursor for root window. */
		XDefineCursor(display, w, cursor);
		
		/* Call XUndefineCursor on root window. */
		XCALL;
		if (geterr() != Success)
			FAIL;
		else
			CHECK;
	
		CHECKUNTESTED(1);
	}
>>ASSERTION Bad A
.ER BadWindow 
>>#HISTORY peterc Completed Wrote STRATEGY and CODE
