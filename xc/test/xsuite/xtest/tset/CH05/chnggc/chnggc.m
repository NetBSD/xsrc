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
 * $XConsortium: chnggc.m,v 1.13 94/04/17 21:03:49 rws Exp $
 */
>>TITLE XChangeGC CH05
void
XChangeGC(display, gc, valuemask, values)
Display		*display = Dsp;
GC		gc;
unsigned 	long valuemask = GCTile;
XGCValues	*values = &srcgcv;
>>EXTERN
XGCValues	srcgcv = { GXxor, AllPlanes, 0, 1, 0, LineSolid, CapButt,
			   JoinMiter, FillSolid, EvenOddRule, ArcPieSlice,
			   None, };
>>SET need-gc-flush
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good A
A call to xname changes the components
specified by the
.A valuemask
argument 
in the specified GC
to the values in the
.A values
argument.
>>STRATEGY
Create a GC fg = W_FG, bg = W_BG, function = GXCopy.
Change the function component of the GC to GXxor using XChangeGC.
Plot point (0,0) with XDrawPoint.
Set fg to W_FG ^ W_BG using XChangeGC.
Plot point (0,0) with XDrawPoint.
Verify that pixel at (0,0) is W_BG.
>>CODE
XVisualInfo	*vp;
Window		win;
XRectangle	rect;
Pixmap		pmap;
XGCValues	gcv;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);
	gc = makegc(display, win);
	
	gcv.function = GXxor;
	valuemask = GCFunction;
	values = &gcv;
	XCALL;

	XDrawPoint(display, win, gc, 0, 0);
	XSetForeground(display, gc, W_FG ^ W_BG);
	XDrawPoint(display, win, gc, 0, 0);

	if( ! checkpixel(display, win, 0, 0, W_BG) ) {
		report("The GC function component was not set to GXxor");
		report("by a call to XChangeGC");
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good A
When a call to xname changes the 
.M clip-mask
component of
.A gc,
then any previous 
.S XSetClipRectangles 
request on the specified GC is overridden.
>>STRATEGY
Create a GC.
Draw line from (0, 0) to (100, 0) using XDrawLine.
Save image on drawable using XGetImage.
Clear drawable.
Change the clip_mask component of the GC using XSetClipRectangles.
Draw line from (0, 0) to (100, 0) using XDrawLine.
Verify using XGetImage that the image on the drawable is altered.
Change the GC clip_mask component using XChangeGC to original.
Draw line from (0, 0) to (100, 0) using XDrawLine.
Verify using XGetImage that the image is restored.
>>CODE
XVisualInfo *vp;
XGCValues   srcgcv;
GC gc;
Window win;
XImage	*im;
XRectangle *rectangles;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	srcgcv.foreground = W_FG;
	srcgcv.line_width = 10;
	srcgcv.clip_mask = None;

	/* Create a GC. */
	gc = XCreateGC(display, win, (GCClipMask | GCLineWidth | GCForeground),
								 &srcgcv);

	/* Draw line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Save image on drawable using XGetImage. */
	im = savimage(display, win);

	/* Clear drawable. */
	dclear(display, win);

	/* Change the clip_mask component of the GC using XSetClipRectangles. */
	XSetClipRectangles(display, gc, 0, 0, rectangles, 0, Unsorted);

	/* Draw line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Verify using XGetImage that the image on the drawable is altered. */
	if(diffsavimage(display, win, im)) {
		delete("XSetClipRectangles did not set clip_mask component of GC");
		return;
	} else
		CHECK;

	/* Change the GC clip_mask component using XChangeGC to original. */
	srcgcv.clip_mask = None;
	values = &srcgcv;
	valuemask = GCClipMask;
	XCALL;

	/* Draw line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Verify using XGetImage that the image is restored. */
	if( ! compsavimage(display, win, im)) {
		report("XChangeGC with GCClipMask did not override call to XSetClipRectangles");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Good A
When a call to xname changes the
.M dash-offset
or
.M dashes
component of
.A gc,
then any previous
.S XSetDashes 
request on the specified GC is overridden.
>>STRATEGY
Create a GC.
Draw dashed line from (0, 0) to (100, 0) using XDrawLine.
Save image on drawable using XGetImage.

Part 1. Verify cancelling when GCDashList is used.
Change the dashes component of the GC
Draw dashed line from (0, 0) to (100, 0) using XDrawLine.
Verify using XGetImage that XSetDashes altered image on drawable.
Change the GC dashes component using XChangeGC to original value.
Draw dashed line from (0, 0) to (100, 0) using XDrawLine.
Verify using XGetImage that the image is as before XSetDashes.

Part 2. Verify cancelling when GCDashOffset is used.
Change the dash_offset component of the GC using XSetDashes.
Draw dashed line from (0, 0) to (100, 0) using XDrawLine.
Verify using XGetImage that XSetDashes altered image on drawable.
Change the GC dash_offset component using XChangeGC to original value.
Draw dashed line from (0, 0) to (100, 0) using XDrawLine.
Verify using XGetImage that the image is as before XSetDashes.
>>CODE
XVisualInfo *vp;
XGCValues   srcgcv;
GC gc;
Window win;
XImage	*im;
static char	dashes[] = {20, 10};
static char	odashes[] = {10, 10};

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	srcgcv.foreground = W_FG;
	srcgcv.background = W_BG;
	srcgcv.line_style = LineDoubleDash;
	srcgcv.line_width = 10;
	srcgcv.dashes = 10;
	srcgcv.dash_offset = 0;

	/* Create a GC. */
	gc = XCreateGC(display, win, (GCLineWidth | GCLineStyle | GCDashOffset |
			GCDashList | GCForeground | GCBackground), &srcgcv);

	/* Draw dashed line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Save image on drawable using XGetImage. */
	im = savimage(display, win);

	/* Part 1. Verify cancelling when GCDashList is used. */
	/* Change the dashes component of the GC */
	XSetDashes(display, gc, 0, dashes, sizeof(dashes)/sizeof(char));

	/* Draw dashed line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Verify using XGetImage that XSetDashes altered image on drawable. */
	if(diffsavimage(display, win, im)) {
		delete("XSetDashes did not set dashes component of GC");
		return;
	} else
		CHECK;

	/* Change the GC dashes component using XChangeGC to original value. */
	srcgcv.dashes = 10;
	values = &srcgcv;
	valuemask = GCDashList;
	XCALL;

	/* Draw dashed line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Verify using XGetImage that the image is as before XSetDashes. */
	if( ! compsavimage(display, win, im)) {
		report("XChangeGC with GCDashList did not override call to XSetDashes");
		FAIL;
	} else
		CHECK;

	/* Part 2. Verify cancelling when GCDashOffset is used. */
	/* Change the dash_offset component of the GC using XSetDashes. */
	XSetDashes(display, gc, 5, odashes, sizeof(odashes)/sizeof(char));

	/* Draw dashed line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Verify using XGetImage that XSetDashes altered image on drawable. */
	if(diffsavimage(display, win, im)) {
		delete("XSetDashes did not set dash_offset component of GC");
		return;
	} else
		CHECK;

	/* Change the GC dash_offset component using XChangeGC to original value. */
	srcgcv.dash_offset = 0;
	values = &srcgcv;
	valuemask = GCDashOffset;
	XCALL;

	/* Draw dashed line from (0, 0) to (100, 0) using XDrawLine. */
	XDrawLine(display, win, gc, 0, 0, 100, 0);

	/* Verify using XGetImage that the image is as before XSetDashes. */
	if( ! compsavimage(display, win, im)) {
		report("XChangeGC with GCDashOffset did not override call to XSetDashes");
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Bad A
.ER GC
>>ASSERTION Bad A
When the
.M font
does not name a valid font,
and the GCFont bit is set in 
.A valuemask ,
then a
.S BadFont
error occurs.
>>STRATEGY
Create a font for the default screen and free font with XFreeFont.
Create a GC with root window as the drawable.
Change GC using XChangeGC with bad font as the font component.
Verify that a BadFont error occurs. 
>>CODE BadFont
Window w;
Font font;
XGCValues vals;

	vals.font = badfont(Dsp);

	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCFont;
	XCALL;

	if( geterr() == BadFont) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the 
.M tile
does not name a valid pixmap, 
and the GCTile bit is set in 
.A valuemask ,
then a 
.A BadPixmap
error occurs.
>>STRATEGY
Create a bad pixmap for the root window and free pixmap with XFreePixmap.
Create a GC with window as the drawable.
Change GC using XChangeGC with pixmap as the tile.
Verify that a BadPixmap error occurs. 
>>CODE BadPixmap
XGCValues vals;

	vals.tile = badpixm(display);

	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCTile;
	XCALL;

	if( geterr() == BadPixmap) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the 
.M stipple
does not name a valid pixmap, 
and the GCStipple bit is set in 
.A valuemask ,
then a 
.A BadPixmap
error occurs.
>>STRATEGY
Create a bad pixmap for the root window and free pixmap with XFreePixmap.
Create a GC with window as the drawable.
Change GC using XChangeGC with pixmap as the stipple.
Verify that a BadPixmap error occurs. 
>>CODE BadPixmap
XGCValues vals;

	vals.stipple = badpixm(display);

	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCStipple;
	XCALL;

	if( geterr() == BadPixmap) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the 
.M clip-mask
does not name a valid pixmap, or 
.S None , 
and the GCClipMask bit is set in 
.A valuemask ,
then a 
.A BadPixmap
error occurs.
>>STRATEGY
Create a bad pixmap for the root window and free pixmap with XFreePixmap.
Create a GC with window as the drawable.
Change GC using XChangeGC with pixmap as the clip_mask.
Verify that a BadPixmap error occurs. 
>>CODE BadPixmap
Window w;
Pixmap pm;
XGCValues vals;

	vals.clip_mask = badpixm(display);

	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCClipMask;
	XCALL;

	if (geterr() == BadPixmap)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad C
When the graphics context and the
.M tile
pixmap do not have the same depth, 
and the GCTile bit is set in 
.A valuemask ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
For all non-depth1 drawables:
	Created such a drawable.
	Create a depth 1 pixmap.
	Create a GC with window as the drawable.
	Change GC using XChangeGC with pixmap as the tile.
	Verify that a BadMatch error occurs. 
>>CODE BadMatch
int		count;
XGCValues 	vals;
XVisualInfo	*vp;
Drawable	d;

	count = 0;
	vals.tile = XCreatePixmap(display, DRW(display), 1,1,1);
	valuemask = GCTile;
	values = &vals;
	for(resetvinf(VI_WIN); nextvinf(&vp);)
		if(vp->depth != 1) {
			trace("Testing a window of depth %d",vp->depth);
			count++;
			d = makewin(display, vp);
			gc = makegc(display, d);
			XCALL;
			if(geterr() == BadMatch)
				CHECK;
			else
				FAIL;
		}

	for(resetvinf(VI_PIX); nextvinf(&vp);)
		if(vp->depth != 1) {
			trace("Testing a window of depth %d",vp->depth);
			count++;
			d = makepixm(display, vp);
			gc = makegc(display, d);
			XCALL;
			if(geterr() == BadMatch)
				CHECK;
			else
				FAIL;
		}


	if(count == 0) {
		tet_result(TET_UNSUPPORTED);
		report("Only depth one drawables are supported.");
		return;
	} else
		CHECKPASS(count);

>>ASSERTION Bad A
When the graphics context and the
.M tile
pixmap were not created for the same root, 
and the GCTile bit is set in 
.A valuemask ,
then a
.S BadMatch 
error occurs. 
>>STRATEGY
If multiple roots are supported:
	Create a pixmap for one root.
	Create a GC for another root 
	Change GC using XChangeGC with pixmap as the tile component.
	Verify that a BadMatch error occurs.
>>CODE BadMatch
char    *altroot;
int     scr_num;
XGCValues vals;

	altroot = tet_getvar("XT_ALT_SCREEN");
	if (altroot == NULL) {
		delete("XT_ALT_SCREEN not set");
		return;
	}
	if (*altroot == 'U') {
		report("Only one screen supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	scr_num = atoi(altroot);
	if (scr_num == DefaultScreen(display)) {
		delete("The Alternate root was the same as the one under test");
		return;
	}
	if (scr_num >= ScreenCount(display)) {
		delete("Screen given in XT_ALT_SCREEN could not be accessed");
		return;
	}

	/*
	 * Create a 1x1 depth 1 pixmap on other screen
	 * and use it to create a gc
	 */
	
	vals.tile =  XCreatePixmap(display, RootWindow(display, scr_num), 1, 1, 1);
	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCTile;
	XCALL;

	if (geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad C
When the
.M stipple
pixmap does not have depth one, 
and the GCStipple bit is set in 
.A valuemask ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
If pixmaps with depth other than one are supported:
	Create a pixmap with depth other than one.
  	Create a GC 
	Change GC using XChangeGC with the pixmap as the stipple component.
  	Verify that a BadMatch error occurs.
>>CODE BadMatch
XGCValues vals;

	if((vals.stipple = nondepth1pixmap(display, DRW(display))) == (Pixmap) 0) {
		tet_result(TET_UNSUPPORTED);
		report("Only depth 1 pixmaps are supported.");
		return;
	} else
		CHECK;

	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCStipple;
	XCALL;
	
	if(geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);

>>ASSERTION Bad C
When the graphics context and the 
.M stipple
pixmap were not created for the same root, 
and the GCStipple bit is set in 
.A valuemask ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
If multiple roots are supported:
	Create a pixmap for one root.
	Create a GC for another root 
	Change GC using XChangeGC with pixmap as the stipple component.
  	Verify that a BadMatch error occurs.
>>CODE BadMatch
char    *altroot;
int     scr_num;
XGCValues vals;

	altroot = tet_getvar("XT_ALT_SCREEN");
	if (altroot == NULL) {
		delete("XT_ALT_SCREEN not set");
		return;
	}
	if (*altroot == 'U') {
		report("Only one screen supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	scr_num = atoi(altroot);
	if (scr_num == DefaultScreen(display)) {
		delete("The Alternate root was the same as the one under test");
		return;
	}
	if (scr_num >= ScreenCount(display)) {
		delete("Screen given in XT_ALT_SCREEN could not be accessed");
		return;
	}

	/*
	 * Create a 1x1 depth 1 pixmap on other screen
	 * and use it to create a gc
	 */
	
	vals.stipple = XCreatePixmap(display, RootWindow(display, scr_num), 1, 1, 1);
	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCStipple;
	XCALL;

	if (geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad C
When the
.M clip-mask
is set to a pixmap, and
the 
.M clip-mask
does not have depth one,
and the GCClipMask bit is set in 
.A valuemask ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
If pixmaps with depth other than one are supported:
	Create a pixmap with depth other than one.
  	Create a GC 
	Change GC using XChangeGC with the pixmap as the clip_mask component.
  	Verify that a BadMatch error occurs.
>>CODE BadMatch
XGCValues vals;

	if((vals.clip_mask = nondepth1pixmap(display, DRW(display))) == (Pixmap) 0) {
		tet_result(TET_UNSUPPORTED);
		report("Only depth 1 pixmaps are supported.");
		return;
	} else
		CHECK;

	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCClipMask;
	XCALL;

	if(geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);

>>ASSERTION Bad C
When the
.M clip-mask
is set to a pixmap, and
the graphics context and the
.M clip-mask
were not created for the same root,
and the GCClipMask bit is set in 
.A valuemask ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
If multiple roots are supported:
	Create a pixmap for one root.
	Create a GC for another root 
	Change GC using XChangeGC with the pixmap as the clip_mask component.
  	Verify that a BadMatch error occurs.
>>CODE BadMatch
char    *altroot;
int     scr_num;
XGCValues	vals;

	altroot = tet_getvar("XT_ALT_SCREEN");
	if (altroot == NULL) {
		delete("XT_ALT_SCREEN not set");
		return;
	}
	if (*altroot == 'U') {
		report("Only one screen supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	scr_num = atoi(altroot);
	if (scr_num == DefaultScreen(display)) {
		delete("The Alternate root was the same as the one under test");
		return;
	}
	if (scr_num >= ScreenCount(display)) {
		delete("Screen given in XT_ALT_SCREEN could not be accessed");
		return;
	}

	/*
	 * Create a 1x1 depth 1 pixmap on other screen
	 * and use it to create a gc
	 */

	vals.clip_mask = XCreatePixmap(display, RootWindow(display, scr_num), 1, 1, 1);
	gc = makegc(display, DRW(display));
	values = &vals;
	valuemask = GCClipMask;
	XCALL;

	if (geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Good A
When the
.A valuemask
argument is other than a bitwise OR of none or any of
.S GCFunction ,
.S GCPlaneMask ,
.S GCForeground ,
.S GCBackground ,
.S GCLineWidth ,
.S GCLineStyle ,
.S GCCapStyle ,
.S GCJoinStyle ,
.S GCFillStyle ,
.S GCFillRule ,
.S GCTile ,
.S GCStipple ,
.S GCTileStipXOrigin ,
.S GCTileStipYOrigin ,
.S GCFont ,
.S GCSubWindowMode ,
.S GCGraphicsExposures ,
.S GCClipXOrigin ,
.S GCClipYOrigin ,
.S GCClipMask ,
.S GCDashOffset ,
.S GCDashList ,
or 
.S GCArcMode ,
then a
.S BadValue 
error occurs.
>>STRATEGY
Create a gc 
Change GC using XChangeGC with function component GXcopy 
    and foreground component = 1 using a mask of GCFunction | GCForeground | 
    ~(GCFunction | GCPlaneMask | GCForeground | GCBackground |
    GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle |
    GCFillStyle | GCFillRule | GCTile | GCStipple |
    GCTileStipXOrigin | GCTileStipYOrigin | GCFont | GCSubwindowMode |
    GCGraphicsExposures | GCClipXOrigin | GCClipYOrigin | GCClipMask |
    GCDashOffset | GCDashList | GCArcMode)
Verify that a bad value error occurred.
>>CODE BadValue
Window w;
XGCValues srcgcv;
int i;
unsigned char *p;

	i = sizeof(XGCValues);
	p = (unsigned char *)&srcgcv;
	while( i-- > 0 )
		*p++ = 0xff;

	gc = makegc(display, DRW(display));
	values = &srcgcv;
	srcgcv.foreground = 1;
	srcgcv.function = ~0;
	valuemask = GCFunction | GCForeground | ~(GCFunction | GCPlaneMask | GCForeground | GCBackground |
		    GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle |
		    GCFillStyle | GCFillRule | GCTile | GCStipple |
		    GCTileStipXOrigin | GCTileStipYOrigin | GCFont | GCSubwindowMode |
		    GCGraphicsExposures | GCClipXOrigin | GCClipYOrigin | GCClipMask |
		    GCDashOffset | GCDashList | GCArcMode);
	XCALL;

	if(geterr() == BadValue)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the 
.M function
is other than
.S GXclear ,
.S GXand ,
.S GXandReverse ,
.S GXcopy ,
.S GXandInverted ,
.S GXnoop ,
.S GXxor ,
.S GXor ,
.S GXnor ,
.S GXequiv ,
.S GXinvert ,
.S GXorReverse ,
.S GXcopyInverted ,
.S GXorInverted ,
.S GXnand
or
.S GXset ,
and the GCFunction bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC function to GXclear + GXand + GXandReverse + GXcopy + 
			GXandInverted + GXnoop + GXxor + GXor + GXnor + 
			GXequiv + GXinvert + GXorReverse +
			GXcopyInverted + GXorInverted + GXnand + GXset + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.function = GXclear + GXand + GXandReverse + GXcopy + 
			GXandInverted + GXnoop + GXxor + GXor + GXnor + 
			GXequiv + GXinvert + GXorReverse +
			GXcopyInverted + GXorInverted + GXnand + GXset + 1;
	values = &srcgcv;
	valuemask = GCFunction;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the
.M line_style
is other than
.S LineSolid ,
.S LineDoubleDash
or
.S LineOnOffDash ,
and the GCLineStyle bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC line_style to LineSolid + LineDoubleDash + LineOnOffDash + 1;
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.line_style = LineSolid + LineDoubleDash + LineOnOffDash + 1;
	values = &srcgcv;
	valuemask = GCLineStyle;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the
.M cap_style
is other than
.S CapNotLast ,
.S CapButt ,
.S CapRound
or
.S CapProjecting ,
and the GCCapStyle bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC cap_style to CapNotLast + CapButt + CapRound + CapProjecting + 1
Verify that a BadValue error occurred
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.cap_style = CapNotLast + CapButt + CapRound + CapProjecting + 1;
	values = &srcgcv;
	valuemask = GCCapStyle;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the
.M join_style
is other than
.S JoinMiter ,
.S JoinRound
or
.S JoinBevel ,
and the GCJoinStyle bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC join_style to JoinMiter + JoinRound + JoinBevel + 1
Verify that a BadValue error occurs
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.join_style = JoinMiter + JoinRound + JoinBevel + 1;
	values = &srcgcv;
	valuemask = GCJoinStyle;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the
.M fill_style
is other than
.S FillSolid ,
.S FillTiled ,
.S FillStippled
or
.S FillOpaqueStippled ,
and the GCFillStyle bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC fill_style to FillSolid + FillTiled + FillStippled + FillOpaqueStippled + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.fill_style = FillSolid + FillTiled + FillStippled + FillOpaqueStippled + 1;
	values = &srcgcv;
	valuemask = GCFillStyle;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the
.M fill_rule
is other than
.S EvenOddRule 
or
.S WindingRule ,
and the GCFillRule bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC fill_rule to EvenOddRule + WindingRule + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.fill_rule = EvenOddRule + WindingRule + 1;
	values = &srcgcv;
	valuemask = GCFillRule;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When
.M arc_mode
is other than
.S ArcChord
or
.S ArcPieSlice ,
and the GCArcMode bit is set in 
.A valuemask ,
then a
.S BadValue 
error occurs.
>>STRATEGY
Create a GC.
Change GC arc_mode to ArcChord + ArcPieSlice + 1.
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.arc_mode = ArcChord + ArcPieSlice + 1;
	values = &srcgcv;
	valuemask = GCArcMode;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When the
.M subwindow_mode
is other than
.S ClipByChildren
or
.S IncludeInferiors ,
and the GCSubwindowMode bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC subwindow_mode to ClipByChildren + IncludeInferiors + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.subwindow_mode = ClipByChildren + IncludeInferiors + 1;
	values = &srcgcv;
	valuemask = GCSubwindowMode;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When
.M graphics_exposure
is other than
.S True
or
.S False ,
and the GCGraphicsExposures bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC graphics_exposures component to (int) False + (int) True + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display,DRW(display));
	srcgcv.graphics_exposures = (int) False + (int) True + 1;
	values = &srcgcv;
	valuemask = GCGraphicsExposures;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);


>>ASSERTION Bad A
When
.M dashes
is set to zero, 
and the GCDashList bit is set in 
.A valuemask ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a GC.
Change GC using XChangeGC with dashes component = 0
Verify that a BadValue error occurs.
>>CODE BadValue
XGCValues srcgcv;

	gc = makegc(display, DRW(display));
	srcgcv.dashes = (char) 0;
	values = &srcgcv;
	valuemask = GCDashList;
	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>># HISTORY 	steve	Completed	Automatic conversion to new(ish) form.
>># HISTORY	cal	Completed	Checked for new style and format.
>># HISTORY	cal	Action		Writing code.
