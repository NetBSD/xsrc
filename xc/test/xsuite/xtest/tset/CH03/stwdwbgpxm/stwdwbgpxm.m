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
 * $XConsortium: stwdwbgpxm.m,v 1.8 94/04/17 21:03:19 rws Exp $
 */
>>TITLE XSetWindowBackgroundPixmap CH03
void

Display *display = Dsp;
Window	w;
Pixmap	background_pixmap;
>>EXTERN
static struct	area	ap;
static Window	parent;

static void	inittp()
{
	tpstartup();

	ap.x = 50;
	ap.y = 60;
	ap.width = 20;
	ap.height= 20;
}

static void
perform_map(display, w)
Display	*display;
Window	w;
{
	XEvent	event;
/* Await visibilty */
	XSelectInput(display, w , ExposureMask);
	XMapWindow(display, w);
	XWindowEvent(display, w, ExposureMask, &event);
}

>>SET	tpstartup inittp
>>ASSERTION Good A
A call to xname sets the background pixmap of the window
to the pixmap specified by
.A background_pixmap .
>>STRATEGY
Create a window with a background pixel.
Change the background-pixmap using xname.
Verify the background-pixmap was set.
>>CODE

	parent = defdraw(display, VI_WIN);

	w = creunmapchild(display, parent, &ap);
	XSetWindowBackground(display, w, W_FG);
	perform_map(display, w);

	background_pixmap = maketile(display, w);
	XCALL;

	XUnmapWindow(display, w);
	perform_map(display, w);

	if (checktile( display, w, NULL, 0, 0, background_pixmap))
		CHECK;
	else {
		report("%s did not set the background pixmap correctly",
			TestName);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
>># ### This should go in XClearWindow etc.
When 
.A background_pixmap
is
.S ParentRelative ,
then each time the background pixmap is required it is taken from the
parent window at the time that it is required,
>>#the background pixmap of the parent window is used
with the background tile origin aligned with the origin of
the parent window.
>># This is not 'the background is set to the background of the parent window'
>>STRATEGY
Set background-pixmap to ParentRelative.
For a variety of parent backgrounds
  Clear child window.
  Verify that background changes to that set for the parent.
>>CODE
Pixmap	pm;
unsigned int	border_width = 2;

	background_pixmap = ParentRelative;

	parent = defdraw(display, VI_WIN);
	w = creunmapchild(display, parent, &ap);
	XSetWindowBorderWidth(display, w, border_width);
	perform_map(display, w);
	XCALL;

	pm = maketile(display, parent);
	XSetWindowBackgroundPixmap(display, parent, pm);
	XClearWindow(display, w);

	if (checktile(display, w, (struct area *)0, -ap.x-border_width, -ap.y-border_width, pm))
		CHECK;
	else {
		report("ParentRelative check failed for 'maketile' background");
		FAIL;
	}

	pm = XCreatePixmap(display, parent, 10, 17, (int)getdepth(display, w));
	dclear(display, pm);
	pattern(display, pm);
	XSetWindowBackgroundPixmap(display, parent, pm);
	XClearWindow(display, w);
	if (checktile(display, w, (struct area *)0, -ap.x-border_width, -ap.y-border_width, pm))
		CHECK;
	else {
		report("ParentRelative check failed for 'pattern' background");
		FAIL;
	}

	XSetWindowBackground(display, parent, W_FG);
	XClearWindow(display, w);
	if (checkarea(display, w, (struct area *)0, W_FG, W_BG, CHECK_ALL))
		CHECK;
	else {
		report("ParentRelative check failed for plain background set to W_FG");
		FAIL;
	}

	CHECKPASS(3);

>>ASSERTION Good B 1
When the background pixmap of a root window is set to
.S ParentRelative ,
then the default background is restored.
>>ASSERTION Good A
When the background pixmap is set to
.S None , 
then the window has no defined background.
>>STRATEGY
Set window-pixmap to None by calling xname.
Map window over a patterned background.
Verify that contents of the window are the same as the parent's.
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);
		pattern(display, parent);

		w = mkwinchild(display, vp, &ap, False, parent, 0);

/* Set the background to ensure it gets unset */
		XSetWindowBackground(display, w, W_FG);

		background_pixmap = None;
		XCALL;

		perform_map(display, w);

		if (isdeleted())
			continue;

		if( !checkpattern(display, parent, &ap))
		{	
			report("%s did not leave the child", TestName);
			report("window background transparent");
			FAIL;
		}
		else
			CHECK;
		
	}

	CHECKPASS(nvinf());

>>ASSERTION Good B 1
When the background pixmap of
a root window is set to
.S None ,
then the default background is restored.
>>ASSERTION Good A
.ER BadPixmap ParentRelative None 
>>ASSERTION Bad C
If windows with depth other than one are supported:
When
.A background_pixmap
and the window do not have the same depth, then a
.S BadMatch
error occurs.
>>STRATEGY
Use depth of 1 for the pixmap.
Find a visual not of depth 1.
If not such a visual
  UNSUPPORTED
else
  Attempt to set background_pixmap to the depth 1 pixmap.
  Verify that a BadMatch error occurs.
>>CODE BadMatch
Pixmap	pm;
XVisualInfo	*vp;
int 	found = 0;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		if (vp->depth != 1) {
			found = 1;
			break;
		}
	}

	if (!found) {
		unsupported("Only windows with depth one are supported");
		return;
	}

	pm = XCreatePixmap(display, DRW(display), 2, 2, 1);
	background_pixmap = pm;

	parent = defdraw(display, VI_WIN);
	w = mkwinchild(display, vp, &ap, False, parent, 2);

	XCALL;
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
	XFreePixmap(display, pm);
>>ASSERTION Bad C
If multiple screens are supported:
When
.A background_pixmap
and the window are not created for the same screen, then a
.S BadMatch
error occurs.
>>STRATEGY
If there is a pixmap depth on the alternate screen that has the same
depth as a visual on the test screen then
  Create pixmap on alternate screen.
  Attempt to set background-pixmap with the pixmap
  Verify that a BadMatch error occurs.
else
  UNSUPPORTED
>>CODE BadMatch
XVisualInfo	*vp;
Pixmap	pm;
int 	*depths;
int 	count;
int 	found = 0;
int 	i;

	if (config.alt_screen == -1) {
		unsupported("No alternate screen supported");
		return;
	}

	depths = XListDepths(display, config.alt_screen, &count);

	for (resetvinf(VI_WIN); (!found && nextvinf(&vp)); ) {
		for (i = 0; i < count; i++) {
			if (depths[i] == vp->depth)
			{
				found = 1;
				break;
			}	
		}
	}

	if (!found) {
		unsupported("Pixmaps of same depth as a window not supported on alt screen");
		return;
	}

	parent = defdraw(display, VI_WIN);
	w = creunmapchild(display, parent, &ap);

	pm = XCreatePixmap(display, RootWindow(display, config.alt_screen),
		2, 3, vp->depth);
	background_pixmap = pm;

	XCALL;
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreePixmap(display, pm);
>>ASSERTION Bad C
If multiple window depths are supported:
When
.A background_pixmap
is
.S ParentRelative
and the window and the parent window do not have the same depth, then a
.S BadMatch
error occurs.
>>STRATEGY
If two different depth windows are supported.
  Create window with different depth to parent.
  Attempt to set background_pixmap to ParentRelative.
  Verify that a BadMatch error occurs.
else
  UNSUPPORTED.
>>CODE BadMatch
XVisualInfo	*vp;
XVisualInfo	*vp2 = 0;
int 	found = 0;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		if (vp2 == 0) {
			vp2 = vp;
		} else if (vp->depth != vp2->depth) {
			found = 1;
			break;
		}
	}

	if (!found) {
		unsupported("Only one depth of window is supported");
		return;
	}

	parent = makedrawable(display, vp2);
	w      = mkwinchild(display, vp, &ap, False, parent, 1);

	background_pixmap = ParentRelative;
	(void)XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
.ER BadMatch wininputonly
>>ASSERTION Bad A
.ER BadWindow 
