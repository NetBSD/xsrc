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
 * $XConsortium: commattr.mc,v 1.20 94/04/17 21:03:01 rws Exp $
 */
>>EXTERN

#if defined(T_XCreateWindow)
static Window	w;
#endif

static void
setinonly()
{
#if defined(T_XCreateWindow)
	class = InputOnly;
	depth = 0;
	visual = CopyFromParent;
	border_width = 0;
#else
	w = iponlywin(Dsp);
#endif
}


#define XCWA_NORMAL	0
#define XCWA_DEFCOLMAP	1
#define	XCWA_GRAVITY	2
#define XCWA_VISDEPTH	3

static Window
makeinout(mio_parent, mio_visual, mio_depth, mio_mode)
Window mio_parent;
Visual *mio_visual;
int	mio_depth;
int	mio_mode;
{
	Display	*mio_display;
	int	mio_x, mio_y;
	unsigned int	mio_width, mio_height;
	unsigned int	mio_border_width;
	unsigned int	mio_class;
	unsigned long	mio_valuemask;
	XSetWindowAttributes	*mio_aptr;
	XSetWindowAttributes	mio_a;
	

/* If we are passed a parent of None, then we need to set it ourselves. */
	if(mio_parent == None)
	{
		mio_parent = DRW(Dsp);
	}

	mio_display = Dsp;
	mio_x = 50;
	mio_y = 60;
	mio_width = 20;
	mio_height = 17;
	mio_border_width = 2;
	mio_class = InputOutput;
	mio_valuemask = 0;
	mio_aptr = &mio_a;

	switch(mio_mode)
	{
		case	XCWA_DEFCOLMAP:
			mio_valuemask = CWColormap;
			mio_a.colormap = makecolmap(mio_display, mio_visual, AllocNone);
			break;

		case	XCWA_GRAVITY:
			mio_valuemask = CWWinGravity;
			mio_a.win_gravity = NorthWestGravity;
			break;

                /*
                 * This new case is used whenever a window is to be created
                 * which has a different visual/depth than the parent window.
                 * It will ensure that all required window attributes are set
                 * to prevent a BadMatch error on window creation.
                 */
		case    XCWA_VISDEPTH:
			mio_valuemask = CWBorderPixel | CWColormap;
			mio_a.border_pixel = 0;
			mio_a.colormap = makecolmap(mio_display, mio_visual, 
						AllocNone);
			break;

		case	XCWA_NORMAL:
		default:
			break;
	}
	return(XCreateWindow(mio_display, mio_parent, mio_x, mio_y, mio_width,
			mio_height, mio_border_width, mio_depth, mio_class,
			mio_visual, mio_valuemask, mio_aptr));
}

>>ASSERTION Good A
When the background-pixmap
attribute is set to
.S None
and the
.M background_pixel
attribute is not being set at the same time,
then the window has no defined background.
>>STRATEGY
Set background-pixmap attribute to None.
Map window over a patterned background.
Verify that contents of the window are the same as the parent's.
>>CODE
XVisualInfo	*vp;


	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);
		pattern(display, parent);

		visual = vp->visual;
		depth  = vp->depth;
		valuemask = CWBackPixmap;
		attributes->background_pixmap = None;

#if defined(T_XCreateWindow)
		w = XCALL;
#else
		w = makeinout( parent , visual, depth, XCWA_NORMAL );
		XCALL;
#endif
		XMapWindow(display, w);
		if (isdeleted())
			continue;
		/*
		 * Use pixmap checking here because we want to check the border
		 * to prove that the window is really there, but transparent.
		 */
		PIXCHECK(display, parent);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
>># ### This should go in XClearWindow etc.
When the background-pixmap attribute is set to
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

	visual = CopyFromParent;
	depth = CopyFromParent;
	valuemask = CWBackPixmap;
	attributes->background_pixmap = ParentRelative;

	parent = defdraw(display, VI_WIN);
#if defined(T_XCreateWindow)
	w = XCALL;
	XMapWindow(display, w);
#else
	w = makeinout(parent, visual, depth, XCWA_NORMAL);
	XMapWindow(display, w);
	XCALL;
#endif

	pm = maketile(display, parent);
	XSetWindowBackgroundPixmap(display, parent, pm);
	XClearWindow(display, w);

	if (checktile(display, w, (struct area *)0, -x-border_width, -y-border_width, pm))
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
	if (checktile(display, w, (struct area *)0, -x-border_width, -y-border_width, pm))
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

>>ASSERTION Good A
When both
.M background_pixel
and
.M background_pixmap
are specified, then
.M background_pixel
overrides
.M background_pixmap .
>>STRATEGY
Set both the background-pixel attribute and the background-pixmap attribute.
Map and clear window to ensure background is refreshed.
Verify that background is set to the pixel value.
>>CODE
Pixmap	pm;

	parent = defdraw(display, VI_WIN);

	pm = maketile(display, parent);
	attributes->background_pixel = W_FG;
	attributes->background_pixmap = pm;
	valuemask = CWBackPixel|CWBackPixmap;

	/*
	 * For running with window managers during debugging, testing etc.
	 */
	if (config.debug_override_redirect) {
		attributes->override_redirect = True;
		valuemask |= CWOverrideRedirect;
	}

#if defined(T_XCreateWindow)
	w = XCALL;
	XMapWindow(display, w);
#else
	w = makeinout(parent, (Visual *)CopyFromParent,
		CopyFromParent, XCWA_NORMAL);
	XMapWindow(display, w);
	XCALL;
#endif
	XClearWindow(display, w);

	if (checkarea(display, w, (struct area *)0, W_FG, W_FG, CHECK_ALL))
		CHECK;
	else {
		report("background pixel did not override background pixmap");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When 
.M background_pixel 
is specified,
then it is truncated to the depth of the window.
>># Spec says 'of the visual', but what about depth arg?
>>STRATEGY
For each visual
  Try variety of background pixel values.
  Verify that they are truncated to depth using checkarea.
>>CODE
XVisualInfo	*vp;
static int 	pixlist[] = {
	0, 1, 3, 4, 17, 18, 200, 300, 303,
	0x1234, 0x12345, 0x123456, 0x1234567, 0x12345678};
long	pix;
int 	i;

	visual = CopyFromParent;
	depth = CopyFromParent;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);

		for (i = 0; i < NELEM(pixlist); i++) {
			pix = pixlist[i];

			visual = vp->visual;
			depth  = vp->depth;
			valuemask = CWBackPixel;
			attributes->background_pixel = pix;

#if defined(T_XCreateWindow)
			w = XCALL;
#else
			w = makeinout(parent, visual, depth, XCWA_NORMAL);
			XCALL;
#endif
			XMapWindow(display, w);
			XClearWindow(display, w);

			/*
			 * Check the whole background.  If the background was not all
			 * set to the same value then the error message could be
			 * confusing.
			 */
			debug(3, "  pix 0x%x", pix);
			debug(3, "  vp->depth 0x%x", vp->depth);
			debug(3, "  DEPTHMASK(vp->depth) 0x%x", DEPTHMASK(vp->depth));
			if (checkarea(display, w, (struct area *)0, pix&DEPTHMASK(vp->depth), 0, CHECK_ALL))
				CHECK;
			else {
				report("Background pixel was not truncated (value 0x%x)", pix);
				report("  Was 0x%x, expecting 0x%x", getpixel(display, w, 0, 0),
					pix&DEPTHMASK(vp->depth));
				FAIL;
			}
		}
	}

	CHECKPASS(nvinf() * NELEM(pixlist));

>>ASSERTION Good A
When
.M border_pixmap
is
.S CopyFromParent ,
then the border-pixmap attribute
is copied from the parent window.
>>STRATEGY
Create parent window.
Set parent border-pixmap attribute.
Set child window border-pixmap to CopyFromParent.
Ensure that window is mapped.
Pixmap verify to check that border is correct.
>>CODE
Pixmap	pm;
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);
		pm = maketile(display, parent);
		XSetWindowBorderPixmap(display, parent, pm);

		visual = vp->visual;
		depth = vp->depth;
		valuemask = CWBorderPixmap;
		attributes->border_pixmap = CopyFromParent;

#if defined(T_XCreateWindow)
		w = XCALL;
		XMapWindow(display, w);
#else
		w = makeinout(parent, visual, depth, XCWA_NORMAL);
		XMapWindow(display, w);
		XCALL;
#endif

		PIXCHECK(display, parent);
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.M border_pixel
is specified, then the value is truncated to the depth of the window.
>>STRATEGY
For each visual
  Set border-pixel to various values.
  Read one pixel back from the border.
  Verify that this pixel has been truncated to depth of window.
>>CODE
XVisualInfo	*vp;
static int 	pixlist[] = {
	0, 1, 3, 4, 17, 18, 200, 300, 303,
	0x1234, 0x12345, 0x123456, 0x1234567, 0x12345678};
long	pix;
long	borderpix;
int 	i;

	visual = CopyFromParent;
	depth = CopyFromParent;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);

		for (i = 0; i < NELEM(pixlist); i++) {
			pix = pixlist[i];

			attributes->border_pixel = pix;
			valuemask = CWBorderPixel;

#if defined(T_XCreateWindow)
			w = XCALL;
			XMapWindow(display, w);
#else
			w = makeinout(parent, visual, depth, XCWA_NORMAL);
			XMapWindow(display, w);
			XCALL;
#endif
			XClearWindow(display, w);

			borderpix = getpixel(display, w, -1, -1);
			debug(3, "  pix 0x%x", pix);
			debug(3, "  vp->depth 0x%x", vp->depth);
			debug(3, "  DEPTHMASK(vp->depth) 0x%x", DEPTHMASK(vp->depth));
			if (borderpix == (pix & DEPTHMASK(vp->depth)))
				CHECK;
			else {
				report("Border pixel was not truncated (value 0x%x)", pix);
				report("  Was 0x%x, expecting 0x%x", borderpix,
					pix & DEPTHMASK(vp->depth));
				FAIL;
			}
		}
	}

	CHECKPASS(nvinf() * NELEM(pixlist));

>>ASSERTION Good A
When
.M colormap
is
.S CopyFromParent ,
then the colormap attribute is copied from the parent window.
>>STRATEGY
Set colormap attribute to CopyFromParent.
Get attribute values.
Verify that colormap is same as for parent.
>>CODE
XVisualInfo	*vp;
XWindowAttributes	getatts;
XWindowAttributes	parentatts;

	depth = CopyFromParent;
	visual = CopyFromParent;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);
		XGetWindowAttributes(display, parent, &parentatts);

		attributes->colormap = CopyFromParent;
		valuemask = CWColormap;

#if defined(T_XCreateWindow)
		w = XCALL;
#else
		w = makeinout(parent, visual, depth, XCWA_NORMAL);
		XCALL;
#endif

		XGetWindowAttributes(display, w, &getatts);

		if (getatts.colormap != parentatts.colormap) {
			report("Colormap CopyFromParent: Got 0x%x, expecting 0x%x",
				getatts.colormap, parentatts.colormap);
			FAIL;
		} else
			CHECK;

	}

	CHECKPASS(nvinf());
>>ASSERTION Good B 1
When the cursor attribute is set to
.S None ,
then the cursor of the parent window is used for the window
and any change in the parent window's cursor will cause an
immediate change in the window's cursor.
>>STRATEGY
If extended testing is required:
  Create a parent window.
  Set the parent's cursor to a non-default cursor.
  Verify that the parent's cursor was set correctly.
  If XCreateWindow:
    Create a child window using xname.
  Otherwise:
    Create and map a child window.
    Change the cursor attribute of the window to None using xname.
  Warp the pointer to the child window.
  Verify that the current cursor is that of the parent.
  Verify that the child's cursor was set correctly.
  Set the parent's cursor to a different cursor.
  Verify that the parent's cursor was set correctly.
  Verify that the current cursor has changed to that of the parent.
>>CODE
Cursor pcur;

	/* If extended testing is required: */
	if(noext(0))
		return;

	pcur = makecur(display);
	valuemask = CWCursor;
	attributes->cursor = None;
	x = 10;
	y = 10;

		/* Create a parent window. */
	parent = defwin(display);
	
		/* Set the parent's cursor to a non-default cursor. */
	XDefineCursor(display, parent, pcur);

		/* Verify that the parent's cursor was set correctly. */
	if(curofwin(display, pcur, parent) == False) {
		delete("XDefineCursor() did not set the parent window's cursor correctly.");
		return;
	} else
		CHECK;

#if defined(T_XCreatWindow)
		/* Create a child window using xname. */
	w = XCALL;
#else
		/* Create and map a child window. */
	w = makeinout( parent , visual, depth, XCWA_NORMAL );
	XMapWindow(display, w);
		/* Change the cursor attribute of the window to None using XChangeWindowAttributes. */
	XCALL;
#endif

		/* Warp the pointer to the child window. */
	(void) warppointer(display, w, 0,0);

		/* Verify that the current cursor is that of the parent. */
	if(spriteiswin(display, parent) == False) {	
		report("The cursor used for the child window was not that of its parent.");
		FAIL;
	} else
		CHECK;

		/* Verify that the child's cursor was set correctly. */
	if(curofwin(display, None, w) == False) {
		report("Window's cursor was not set to None.");
		FAIL;
	} else
		CHECK;

	pcur = makecur2(display);

		/* Set the parent's cursor to a different cursor. */
	XDefineCursor(display, parent, pcur);

		/* Verify that the parent's cursor was set correctly. */
	if(curofwin(display, pcur, parent) == False) {
		delete("XDefineCursor() did not set the parent window's cursor correctly.");
		return;
	} else
		CHECK;

		/* Verify that the current cursor has changed to that of the parent. */
	if(spriteiswin(display, parent) == False) {	
		report("The cursor used for the child window was not that of its parent.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad A
When
.M background_pixmap
is not a valid pixmap,
.S None ,
or
.S ParentRelative ,
then a
.S BadPixmap
error occurs.
>>STRATEGY
Call xname with background_pixmap set to an invalid value.
Verify that a BadPixmap error occurs.
>>CODE BadPixmap

	seterrdef();


	attributes->background_pixmap = badpixm(display);
	valuemask = CWBackPixmap;

#if defined(T_XCreateWindow)
	XCALL;
#else
	parent = defdraw(display, VI_WIN);	/* Needed for XCreateWindow? */
	w = makeinout(None, (Visual *)CopyFromParent,
		CopyFromParent, XCWA_NORMAL);
	XCALL;
#endif

	if (geterr() == BadPixmap)
		PASS;
	else
		FAIL;	/* Already reported */
>>ASSERTION Bad A
When
.M border_pixmap
is not a valid pixmap
or
.S CopyFromParent ,
then a
.S BadPixmap
error occurs.
>>STRATEGY
Call xname with border_pixmap set to an invalid value.
Verify that a BadPixmap error occurs.
>>CODE BadPixmap

	seterrdef();

	attributes->border_pixmap = badpixm(display);
	valuemask = CWBorderPixmap;

#if defined(T_XCreateWindow)
	XCALL;
#else
	parent = defdraw(display, VI_WIN);	/* Needed for XCreateWindow? */
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
	XCALL;
#endif

	if (geterr() == BadPixmap)
		PASS;
	else
		FAIL;	/* Already reported */
>>ASSERTION Bad A
When
.M colormap
is not a valid Colormap resource
or
.S CopyFromParent ,
then a
.S BadColor
error occurs.
>>STRATEGY
Call xname with colormap set to an invalid value.
Verify that a BadColor error occurs.
>>CODE BadColor

	seterrdef();

	attributes->colormap = badcolormap(display, DRW(display)); /* XXX */
	valuemask = CWColormap;

#if defined(T_XCreateWindow)
	XCALL;
#else
	parent = defdraw(display, VI_WIN);	/* Needed for XCreateWindow? */
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
	XCALL;
#endif

	if (geterr() == BadColor)
		PASS;
	else
		FAIL;	/* Already reported */
>>ASSERTION Bad A
When
.M cursor
is not a valid Cursor resource
or
.S None ,
then a
.S BadCursor
error occurs.
>>STRATEGY
Call xname with cursor set to an invalid value.
Verify that a BadCursor error occurs.
>>CODE BadCursor

	seterrdef();

	/* Set to a pixmap */
	attributes->cursor = maketile(display, DRW(display));
	valuemask = CWCursor;

#if defined(T_XCreateWindow)
	XCALL;
#else
	parent = defdraw(display, VI_WIN);	/* Needed for XCreateWindow? */
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
	XCALL;
#endif

	if (geterr() == BadCursor)
		PASS;
	else
		FAIL;	/* Already reported */
>>ASSERTION Bad A
When the window has class
.S InputOnly
and
.A valuemask
contains a bit set other than
.S CWWinGravity ,
.S CWEventMask ,
.S CWDontPropagate ,
.S CWOverrideRedirect
and
.S CWCursor ,
then a
.S BadMatch
error occurs.
>>STRATEGY
Set value mask to contain invalid bits.
Verify in each case, that a BadMatch error occurs.
>>CODE BadMatch
int 	i;
int 	n;
unsigned long	vals[NM_LEN];
static	unsigned long	validbits[] = {
	CWWinGravity,
	CWEventMask,
	CWDontPropagate,
	CWOverrideRedirect,
	CWCursor,
	};

	seterrdef();
	setinonly();

	n = notmaskmember(validbits, NELEM(validbits), vals);

	for (i = 0; i < n; i++) {

		debug(1, "Trying arg of %d", vals[i]);

		valuemask = vals[i];
#if defined(T_XCreateWindow)
		XCALL;
#else
		XCALL;
#endif

		if (geterr() == BadMatch)
			CHECK;
		else {
			trace("Value of %d did not give BadMatch", vals[i]);
			FAIL;
		}
	}

	CHECKPASS(n);
>>ASSERTION Bad C
If windows with depth other than one are supported:
When
.M background_pixmap
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

	depth = vp->depth;
	visual = vp->visual;
	pm = XCreatePixmap(display, DRW(display), 2, 2, 1);
	attributes->background_pixmap = pm;
	valuemask = CWBackPixmap;

#if defined(T_XCreateWindow)
	(void)XCALL;
#else
	parent = defdraw(display, VI_WIN);
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
	(void)XCALL;
#endif

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
	XFreePixmap(display, pm);
>>ASSERTION Bad C
If multiple screens are supported:
When
.M background_pixmap
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

	pm = XCreatePixmap(display, RootWindow(display, config.alt_screen),
		2, 3, vp->depth);

	attributes->background_pixmap = pm;
	valuemask = CWBackPixmap;

#if defined(T_XCreateWindow)
	XCALL;
#else
	parent = defdraw(display, VI_WIN);
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
	XCALL;
#endif

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreePixmap(display, pm);
>>ASSERTION Bad C
If multiple window depths are supported:
When
.M background_pixmap
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
	visual = vp->visual;
	depth = vp->depth;

	attributes->background_pixmap = ParentRelative;
	valuemask = CWBackPixmap;

#if defined(T_XCreateWindow)
	(void)XCALL;
#else
	w = makeinout(parent, visual, depth, XCWA_VISDEPTH);
	(void)XCALL;
#endif

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When
.M colormap
and the window are not created for the same screen, then a
.S BadMatch
error occurs.
>>STRATEGY
If multiple screens are supported:
  Obtain visual information about the alternate screen.
  Determine a visual type shared between the main and alternate screen.
  Create a colormap on the alternate screen.
  Attempt to set the colormap.
  Verify that a BadMatch error occurs.
else
  UNSUPPORTED
>># In order to avoid a BadMatch because the visual 
>># types do not match, we should attempt to create a colormap and window
>># of the same visual type, on different screens. 
>># Of course, I don't know how many platforms will allow this. Stuart
>>CODE BadMatch
Colormap	cm;
XVisualInfo	*vp,*vp1, *vinfo, vi;
int 	count;
int 	found = 0;
int 	i;

	if (config.alt_screen == -1) {
		unsupported("No alternate screen supported");
		return;
	}

	vi.screen = config.alt_screen;
	vinfo = XGetVisualInfo(Dsp, VisualScreenMask, &vi, &count);

	for (resetvinf(VI_WIN); (!found && nextvinf(&vp)); ) {
		for (i = 0; i < count; i++) {
			vp1 = &vinfo[i];
			if (vp1->visual == vp->visual)
			{
				found = 1;
				break;
			}	
		}
	}

	if (!found) {
		unsupported("Cannot create a colormap on the alternate screen of the same visual type as a window on the main screen");
		return;
	}

	cm = XCreateColormap(display, RootWindow(display, config.alt_screen),
			vp1->visual, AllocNone);

	attributes->colormap = cm;
	valuemask = CWColormap;

#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreeColormap(display, cm);
	for(i=0; i<count; i++)
		XFree((char*)&vinfo[i]);
>>ASSERTION Bad A
When
.M colormap
and the window do not have the same visual type, then a
.S BadMatch
error occurs.
>>STRATEGY
If two different visual types are supported:
  Create colour map of different visual to window.
  Attempt to set colourmap to created colour map.
  Verify that a BadMatch error occurs.
else
  UNSUPPORTED.
>>CODE BadMatch
XWindowAttributes p_attributes;
Colormap	cm;
int 	found = 0;
XVisualInfo	*vp;

#if	defined(T_XChangeWindowAttributes)
	/* This is set for XCreateWindow, but not for XChangeWindowAttributes */
	parent = DRW(Dsp);
#endif

	XGetWindowAttributes(display, parent, &p_attributes);

	visual = p_attributes.visual;
	depth = p_attributes.depth;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		if (vp->visual != visual)
		{
			found = 1;
			break;
		}
	}

	if (!found) {
		unsupported("Only one visual type supported");
		return;
	}


	cm = XCreateColormap(display, DRW(display) , vp->visual, AllocNone);

	attributes->colormap = cm;
	valuemask = CWColormap;

#if defined(T_XCreateWindow)
	(void)XCALL;
#else
	w = makeinout(parent, visual, depth, XCWA_NORMAL);
	(void)XCALL;
#endif

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreeColormap(display, cm);
>>ASSERTION Bad A
When
.M colormap
is
.S CopyFromParent
and the parent window has a
.M colormap
of
.S None ,
then a
.S BadMatch
error occurs.
>>STRATEGY
Create a window with a colormap.
Free the colormap the window.
Call function with colormap of CopyFromParent.
Verify BadMatch error occurred.
>>CODE BadMatch
Colormap	cm;

	parent = makeinout( DRW(display),
		XDefaultVisual(display, XDefaultScreen(display) ) ,
		CopyFromParent, XCWA_NORMAL);

	cm = XCreateColormap(display, parent ,
			XDefaultVisual(display, XDefaultScreen(display)),
			AllocNone);

	XSetWindowColormap(display, parent, cm);

	attributes->colormap = CopyFromParent;
	valuemask = CWColormap;

#if defined(T_XChangeWindowAttributes)
	w = makeinout(parent, (Visual *)CopyFromParent,
		CopyFromParent, XCWA_NORMAL);
#endif
	XFreeColormap(display, cm);
	(void)XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When the window has class
.S InputOnly
and
.M border_width
is not zero, then a
.S BadMatch
error occurs.
>>STRATEGY
Set border_width to contain non zero.
Verify that a BadMatch error occurs.
>>CODE BadMatch

#if defined(T_XCreateWindow)
	seterrdef();
	setinonly();

	border_width = 1;

		XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
#else
	notinuse("%s cannot be used to change the border_width",TestName);
#endif
>>ASSERTION Bad A
When
.M bit_gravity
is other than
.S ForgetGravity ,
.S NorthWestGravity ,
.S NorthGravity ,
.S NorthEastGravity ,
.S WestGravity ,
.S CenterGravity ,
.S EastGravity ,
.S SouthWestGravity ,
.S SouthGravity ,
.S SouthEastGravity or
.S StaticGravity ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set bit_gravity to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
unsigned long	vals[NM_LEN];
static	unsigned long	validbits[] = {
	ForgetGravity ,
	NorthWestGravity ,
	NorthGravity ,
	NorthEastGravity ,
	WestGravity ,
	CenterGravity ,
	EastGravity ,
	SouthWestGravity ,
	SouthGravity ,
	SouthEastGravity ,
	StaticGravity };

	seterrdef();


#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWBitGravity;

	n = notmaskmember(validbits, NELEM(validbits), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying bit_gravity of %d", vals[i]);

		attributes->bit_gravity = vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>>ASSERTION Bad A
When
.M win_gravity
is other than
.S UnmapGravity ,
.S NorthWestGravity ,
.S NorthGravity ,
.S NorthEastGravity ,
.S WestGravity ,
.S CenterGravity ,
.S EastGravity ,
.S SouthWestGravity ,
.S SouthGravity ,
.S SouthEastGravity or
.S StaticGravity ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set win_gravity to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
unsigned long	vals[NM_LEN];
static	unsigned long	validbits[] = {
	UnmapGravity ,
	NorthWestGravity ,
	NorthGravity ,
	NorthEastGravity ,
	WestGravity ,
	CenterGravity ,
	EastGravity ,
	SouthWestGravity ,
	SouthGravity ,
	SouthEastGravity ,
	StaticGravity };

	seterrdef();


#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWWinGravity;

	n = notmaskmember(validbits, NELEM(validbits), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying win_gravity of %d", vals[i]);

		attributes->win_gravity = vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>>ASSERTION Bad A
When
.M backing_store
is other than
.S NotUseful ,
.S WhenMapped
or
.S Always ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set backing_store to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
long	vals[NM_LEN];
static	int 	validvalues[] = {
	NotUseful,
	WhenMapped,
	Always };

	seterrdef();


#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWBackingStore;

	n = notmember(validvalues, NELEM(validvalues), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying backing_store of %d", vals[i]);

		attributes->backing_store = vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>>ASSERTION Bad A
When
.M save_under
is other than
.S True
or
.S False ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set save_under to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
long	vals[NM_LEN];
static	int 	validvalues[] = {
	True,
	False };

	seterrdef();

#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWSaveUnder;

	n = notmember(validvalues, NELEM(validvalues), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying save_under of %d", vals[i]);

		attributes->save_under= vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>>ASSERTION Bad A
When
.M event_mask
is other than a bitwise OR of any
of
.S NoEventMask ,
.S KeyPressMask ,
.S KeyReleaseMask ,
.S ButtonPressMask ,
.S ButtonReleaseMask ,
.S EnterWindowMask ,
.S LeaveWindowMask ,
.S PointerMotionMask ,
.S PointerMotionHintMask ,
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
.S KeymapStateMask ,
.S ExposureMask ,
.S VisibilityChangeMask ,
.S StructureNotifyMask ,
.S ResizeRedirectMask ,
.S SubstructureNotifyMask ,
.S SubstructureRedirectMask ,
.S FocusChangeMask ,
.S PropertyChangeMask ,
.S ColormapChangeMask
or
.S OwnerGrabButtonMask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set event_mask to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
unsigned long	vals[NM_LEN];
static	unsigned long	validbits[] = {
	NoEventMask ,
	KeyPressMask ,
	KeyReleaseMask ,
	ButtonPressMask ,
	ButtonReleaseMask ,
	EnterWindowMask ,
	LeaveWindowMask ,
	PointerMotionMask ,
	PointerMotionHintMask ,
	Button1MotionMask ,
	Button2MotionMask ,
	Button3MotionMask ,
	Button4MotionMask ,
	Button5MotionMask ,
	ButtonMotionMask ,
	KeymapStateMask ,
	ExposureMask ,
	VisibilityChangeMask ,
	StructureNotifyMask ,
	ResizeRedirectMask ,
	SubstructureNotifyMask ,
	SubstructureRedirectMask ,
	FocusChangeMask ,
	PropertyChangeMask ,
	ColormapChangeMask ,
	OwnerGrabButtonMask };

	seterrdef();

#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWEventMask;

	n = notmaskmember(validbits, NELEM(validbits), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying event_mask of %d", vals[i]);

		attributes->event_mask = vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>>ASSERTION Bad A
When
.M do_not_propagate_mask
is other than a bitwise OR of any
of
.S NoEventMask ,
.S KeyPressMask ,
.S KeyReleaseMask ,
.S ButtonPressMask ,
.S ButtonReleaseMask ,
.S EnterWindowMask ,
.S LeaveWindowMask ,
.S PointerMotionMask ,
.S PointerMotionHintMask ,
.S Button1MotionMask ,
.S Button2MotionMask ,
.S Button3MotionMask ,
.S Button4MotionMask ,
.S Button5MotionMask ,
.S ButtonMotionMask ,
.S KeymapStateMask ,
.S ExposureMask ,
.S VisibilityChangeMask ,
.S StructureNotifyMask ,
.S ResizeRedirectMask ,
.S SubstructureNotifyMask ,
.S SubstructureRedirectMask ,
.S FocusChangeMask ,
.S PropertyChangeMask ,
.S ColormapChangeMask
or
.S OwnerGrabButtonMask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set do_not_propagate_mask to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
unsigned long	vals[NM_LEN];
static	unsigned long	validbits[] = {
	NoEventMask ,
	KeyPressMask ,
	KeyReleaseMask ,
	ButtonPressMask ,
	ButtonReleaseMask ,
	EnterWindowMask ,
	LeaveWindowMask ,
	PointerMotionMask ,
	PointerMotionHintMask ,
	Button1MotionMask ,
	Button2MotionMask ,
	Button3MotionMask ,
	Button4MotionMask ,
	Button5MotionMask ,
	ButtonMotionMask ,
	KeymapStateMask ,
	ExposureMask ,
	VisibilityChangeMask ,
	StructureNotifyMask ,
	ResizeRedirectMask ,
	SubstructureNotifyMask ,
	SubstructureRedirectMask ,
	FocusChangeMask ,
	PropertyChangeMask ,
	ColormapChangeMask ,
	OwnerGrabButtonMask };

	seterrdef();

#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWDontPropagate ;

	n = notmaskmember(validbits, NELEM(validbits), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying do_not_propagate_mask of %d", vals[i]);

		attributes->do_not_propagate_mask = vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>>ASSERTION Bad A
When
.M override_redirect
is other than
.S True
or
.S False ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set override_redirect to a bad value.
Verify that BadValue is generated.
>>CODE BadValue
int	i;
int	n;
long	vals[NM_LEN];
static	int 	validvalues[] = {
	True,
	False };

	seterrdef();

#if defined(T_XChangeWindowAttributes)
	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
#endif
	
	valuemask = CWOverrideRedirect;

	n = notmember(validvalues, NELEM(validvalues), vals);

	for (i = 0; i < n ; i++)
	{
		debug(1, "Trying override_redirect of %d", vals[i]);

		attributes->override_redirect= vals[i];

		(void) XCALL;

		if(geterr() == BadValue)
			CHECK;
		else {
			trace("Value of %d did not give BadValue", vals[i]);
			FAIL;
		}
	}	

	CHECKPASS(n);
>># This assertion has been removed at the request of MIT, because there
>># is no requirement for the Xlib implementation to generated protocol
>># errors for illegal mask bits in this case. (Bug reports 57,59)
>># >>ASSERTION Bad A
>># When the window has class
>># .S InputOutput
>># and
>># .A valuemask
>># contains a bit set other than
>># .S CWBackPixmap ,
>># .S CWBackPixel ,
>># .S CWBorderPixmap ,
>># .S CWBorderPixel ,
>># .S CWBitGravity ,
>># .S CWWinGravity ,
>># .S CWBackingStore ,
>># .S CWBackingPlanes ,
>># .S CWBackingPixel ,
>># .S CWOverrideRedirect ,
>># .S CWSaveUnder ,
>># .S CWEventMask ,
>># .S CWDontPropagate ,
>># .S CWColormap ,
>># .S CWCursor ,
>># .S CWX ,
>># .S CWY ,
>># .S CWWidth ,
>># .S CWHeight ,
>># .S CWBorderWidth ,
>># .S CWSibling or
>># .S CWStackMode ,
>># then a
>># .S BadValue
>># error occurs.
>># >>STRATEGY
>># Set valuemask to a bad value.
>># Verify that BadValue is generated.
>># >>CODE BadValue
>># int	i;
>># int	n;
>># unsigned long	vals[NM_LEN];
>># static	unsigned long	validbits[] = {
>># 	CWBackPixmap ,
>># 	CWBackPixel ,
>># 	CWBorderPixmap ,
>># 	CWBorderPixel ,
>># 	CWBitGravity ,
>># 	CWWinGravity ,
>># 	CWBackingStore ,
>># 	CWBackingPlanes ,
>># 	CWBackingPixel ,
>># 	CWOverrideRedirect ,
>># 	CWSaveUnder ,
>># 	CWEventMask ,
>># 	CWDontPropagate ,
>># 	CWColormap ,
>># 	CWCursor ,
>># 	CWX ,
>># 	CWY ,
>># 	CWWidth ,
>># 	CWHeight ,
>># 	CWBorderWidth ,
>># 	CWSibling ,
>># 	CWStackMode };
>># 
>># 	seterrdef();
>># 
>># #if defined(T_XChangeWindowAttributes)
>># 	w = makeinout(None, (Visual *)CopyFromParent, CopyFromParent, XCWA_NORMAL);
>># #endif
>># 	n = notmaskmember(validbits, NELEM(validbits), vals);
>># 
>># 	for (i = 0; i < n ; i++)
>># 	{
>># 		debug(1, "Trying valuemask of %d", vals[i]);
>># 
>># 		valuemask = vals[i];
>># 
>># 		(void) XCALL;
>># 
>># 		if(geterr() == BadValue)
>># 			CHECK;
>># 		else {
>># 			trace("Value of %d did not give BadValue", vals[i]);
>># 			FAIL;
>># 		}
>># 	}	
>># 
>># 	CHECKPASS(n);
>># 
>>ASSERTION Bad C
If windows with depth other than one are supported:
When
.M border_pixmap
and the window do not have the same depth, then a
.S BadMatch
error occurs.
>>STRATEGY
If a window with depth other than one is supported:
  Attempt to set border_pixmap with depth one.
  Verify that a BadMatch error occurs with a window depth other than one.
else
  UNSUPPORTED.
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

	parent = makewin(display, vp);
	pm =  XCreatePixmap(display, parent, 1, 1, 1);

	visual = vp->visual;
	depth = vp->depth;
	attributes->border_pixmap = pm ;
	valuemask = CWBorderPixmap;

#if defined(T_XChangeWindowAttributes)
	w = makeinout(parent, visual, depth, XCWA_NORMAL);
#endif
	(void)XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreePixmap(display, pm);
>>ASSERTION Bad C
>>#Subsequent changes to the parent window's border attribute do not affect
>>#the window.
If multiple window depths are supported:
When
.M border_pixmap
is
.S CopyFromParent ,
and the window does not have the same depth as the parent window,
then a
.S BadMatch
error occurs.
>>STRATEGY
If two different depth windows are supported.
  Create window with different depth to parent.
  Attempt to set border_pixmap to CopyFromParent.
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
	visual = vp->visual;
	depth = vp->depth;

	attributes->border_pixmap = CopyFromParent;
	valuemask = CWBorderPixmap;

#if defined(T_XCreateWindow)
	(void)XCALL;
#else
	w = makeinout(parent, visual, depth, XCWA_VISDEPTH);
	(void)XCALL;
#endif

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
>>ASSERTION Bad A
When
.M colormap
is
.S CopyFromParent
and the window does not have the same visual type as the parent window,
then a
.S BadMatch
error occurs.
>>STRATEGY
If two different visual types are supported:
  Create a parent of one visual type
  Attempt to set colourmap to CopyFromParent on window of different visual type.
  Verify that a BadMatch error occurs.
else
  UNSUPPORTED.
>>CODE BadMatch
int 	found = 0;
XVisualInfo	*vp;
XVisualInfo	*vp2;


	vp2 = 0;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		if(vp2 == 0)
		{
			vp2 = vp;
		}
		else if (vp->visual !=  vp2->visual)
		{
			found = 1;
			break;
		}
	}

	if (!found) {
		unsupported("Only one visual type supported");
		return;
	}

	parent = (Window)makedrawable( display, vp2 );

	visual = vp->visual;
	depth  = vp->depth;

	attributes->colormap = CopyFromParent;
	valuemask = CWColormap;

#if defined(T_XCreateWindow)
	(void)XCALL;
#else
	w = makeinout(parent, visual, depth, XCWA_VISDEPTH);
	(void)XCALL;
#endif
	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
