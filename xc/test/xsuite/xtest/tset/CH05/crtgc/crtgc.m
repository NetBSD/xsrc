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
 * $XConsortium: crtgc.m,v 1.15 94/04/17 21:03:52 rws Exp $
 */
>>TITLE XCreateGC CH05
GC
XCreateGC(display, d, valuemask, values)
Display *display = Dsp;
Drawable d;
unsigned long valuemask = 0L;
XGCValues *values = 0;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>ASSERTION Good A
A call to xname creates a graphics context and returns a GC
which may be used with any destination drawable having the same root
and depth as the drawable
.A d .
>>STRATEGY
For pixmaps and windows:
   Create a gc for the drawable using XCreateGC.
   Set pixel (0,0) of the drawable.
   Verify that the pixel was set.
>>CODE
XVisualInfo	*vp;
Window		win;
int		nvis;
Pixmap		pmap;
GC		gc;

	for(resetvinf(VI_WIN); 	nextvinf(&vp);) {
		d = makewin(display, vp);

		gc = XCALL;
		XDrawPoint(display,d,gc,0,0);

		/* 0 is the default foreground pixel value. */
		if( ! checkpixel(display, d, 0, 0, 0) ) {  
			report("For window depth %d, pixel was not set to foreground",
								vp -> depth);
			FAIL;
		} else
			CHECK;
	}

	nvis = nvinf();
	for(resetvinf(VI_PIX); 	nextvinf(&vp);) {
		d = makepixm(display, vp);		

		gc = XCALL;
		XDrawPoint(display,d,gc,0,0);

		/* 0 is the default foreground pixel value. */
		if( ! checkpixel(display, d, 0, 0, 0) ) {  
			report("For pixmap depth %d, pixel was not set to foreground",
								vp -> depth);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(nvis + nvinf());

>>ASSERTION Good A
A call to xname initialises the components
specified by the
.A valuemask
argument 
in the newly created GC
to the values in the
.A values
argument.
>>STRATEGY
Create a window.
Initialise XGCValues structure with fg W_FG, fn GXxor.
Create a gc for the window with a valuemask of GCFunction | GCForeground.
Plot point (0,0) with XDrawPoint.
Verify that pixel at (0,0) is W_FG.
Initialise XGCValues structure with fg W_FG ^ W_BG, fn GXxor.
Create a gc for the window with a valuemask of GCFunction | GCForeground.
Plot point (0,0) with XDrawPoint.
Verify that pixel at (0,0) is W_BG.
>>CODE
GC gc;
XWindowAttributes atts;
Status  s;
Pixmap pm;
XGCValues vals;
XVisualInfo *vp;

/* Function */
	
	resetvinf(VI_WIN);
	nextvinf(&vp);

	d = makewin(display, vp);
	vals.function = GXxor;
	vals.foreground = W_FG;
	values = &vals;
	valuemask = (GCFunction | GCForeground);
	gc = XCALL;

	XDrawPoint(display, d, gc, 0, 0);

	if( ! checkpixel(display, d, 0, 0, W_FG)) {
		delete("Pixel at (0, 0) was not set to foreground.");
		return;
	} else
		CHECK;

	vals.foreground = W_FG ^ W_BG;
	values = &vals;
	valuemask = (GCFunction | GCForeground);
	gc = XCALL;

	XDrawPoint(display, d, gc, 0, 0);

	if( ! checkpixel(display, d, 0, 0, W_BG)) {
		report("Pixel at (0, 0) was not set to background.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>># This assertion is incorrect - rewritten 13/4/91.
>>#.A tile
>>#is
>>#.S None
>>#and the
>>#.M fill_style
>>#is
>>#.S FillTiled
>>#and the foreground pixel value is specified, then
>>#a call to xname creates a
>>#.S GC
>>#in which the foreground pixel value of the newly created
>>#.S GC
>>#is used for tiling.
>>ASSERTION Good A
When the
.M tile
pixmap is not set and the
.M fill_style
is
.S FillTiled ,
then a call to xname creates a
.S GC
in which a pixel set to  the foreground value of
.S GC
is used for tiling.
>>STRATEGY
Create a gc with the fill_style FillTiled, foreground set to W_FG, tile set to NULL.
Create a window.
Tile entire window with XFillRectangle.
Verify that all the window pixels were W_FG.
Plot point (0,0) with XDrawPoint.
Set fg to W_FG ^ W_BG.
Plot point (0,0) with XDrawPoint.
Verify that pixel at (0,0) is W_BG.
>>CODE
XVisualInfo	*vp;
XGCValues	vals;
GC		gc;

	resetvinf(VI_WIN); 
	nextvinf(&vp);
	d = makewin(display, vp);
	vals.fill_style = FillTiled;
	vals.foreground = W_FG;
	valuemask = GCForeground | GCFillStyle;
	values = &vals;
	gc = XCALL;

	XFillRectangle(display, d, gc, 0, 0, W_STDWIDTH, W_STDHEIGHT);

	if( checkarea(display, d, (struct area *) 0, W_FG, 0, CHECK_IN) == False) {
		report("Window was not filled with foreground.");
		FAIL;
	} else
		PASS;

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad A
.ER BadDrawable
>>STRATEGY
Create a window and destroy the window.
Create a GC with window as the drawable.
Verify that a BadDrawable error occurs. 
>>CODE BadDrawable

	d = badwin(Dsp);
	valuemask = 0L;
	values = (XGCValues *)NULL;
	XCALL;
	if(geterr() == BadDrawable)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

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
Create a GC with root window as the drawable and bad font as the font component.
Verify that a BadFont error occurs. 
>>CODE BadFont
Window w;
Font font;
XGCValues vals;

	w = DefaultRootWindow(Dsp);
	font = badfont(Dsp);

	vals.font = font;
	d = w;
	valuemask = GCFont;
	values = &vals;

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
Create a GC with window as the drawable and pixmap as the tile.
Verify that a BadPixmap error occurs. 
>>CODE BadPixmap
XGCValues vals;

	vals.tile = badpixm(display);

	values = &vals;
	d = DRW(display);
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
Create a GC with window as the drawable and pixmap as the stipple.
Verify that a BadPixmap error occurs. 
>>CODE BadPixmap
XGCValues vals;

	vals.stipple = badpixm(display);

	values = &vals;
	d = DRW(display);
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
Create a GC with window as the drawable and pixmap as the clip_mask.
Verify that a BadPixmap error occurs. 
>>CODE BadPixmap
Window w;
Pixmap pm;
XGCValues vals;

	vals.clip_mask = badpixm(display);

	values = &vals;
	d = DRW(display);
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
	Create a GC with window as the drawable and pixmap as the tile.
	Verify that a BadMatch error occurs. 
>>CODE BadMatch
int		count;
XGCValues 	vals;
XVisualInfo	*vp;


	count = 0;
	vals.tile = XCreatePixmap(display, DRW(display), 1,1,1);
	valuemask = GCTile;
	values = &vals;
	for(resetvinf(VI_WIN); nextvinf(&vp);)
		if(vp->depth != 1) {
			trace("Testing a window of depth %d",vp->depth);
			count++;
			d = makewin(display, vp);
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
	Create a GC for another root with pixmap as the tile component.
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
	values = &vals;
	valuemask = GCTile;
	d = DRW(display);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

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
  	Create a GC with the pixmap as the stipple component.
  	Verify that a BadMatch error occurs.
>>CODE BadMatch
XGCValues vals;

	if((vals.stipple = nondepth1pixmap(display, DRW(display))) == (Pixmap) 0) {
		tet_result(TET_UNSUPPORTED);
		report("Only depth 1 pixmaps are supported.");
		return;
	} else
		CHECK;

	d = DRW(display);
	valuemask = GCStipple;
	values = &vals;
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
	Create a GC for another root with pixmap as the stipple component.
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
	
	d = DRW(display);
	vals.stipple = XCreatePixmap(display, RootWindow(display, scr_num), 1, 1, 1);
	values = &vals;
	valuemask = GCStipple;
	
	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

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
  	Create a GC with the pixmap as the clip_mask component.
  	Verify that a BadMatch error occurs.
>>CODE BadMatch
XGCValues vals;

	if((vals.clip_mask = nondepth1pixmap(display, DRW(display))) == (Pixmap) 0) {
		tet_result(TET_UNSUPPORTED);
		report("Only depth 1 pixmaps are supported.");
		return;
	} else
		CHECK;

	d = DRW(display);
	valuemask = GCClipMask;
	values = &vals;
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
	Create a GC for another root with the pixmap as the clip_mask component.
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

	d = DRW(display);
	vals.clip_mask = XCreatePixmap(display, RootWindow(display, scr_num), 1, 1, 1);
	valuemask = GCClipMask;
	values = &vals;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
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
.S GCSubwindowMode ,
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
Create a GC with valuemask = GCFunction | GCForeground | 
	 ~(GCFunction | GCPlaneMask | GCForeground | GCBackground |
	  GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle |
	  GCFillStyle | GCFillRule | GCTile | GCStipple |
	  GCTileStipXOrigin | GCTileStipYOrigin | GCFont | GCSubwindowMode |
	  GCGraphicsExposures | GCClipXOrigin | GCClipYOrigin | GCClipMask |
	  GCDashOffset | GCDashList | GCArcMode.
Verify that a BadValue error occurred.
>>CODE BadValue
Window w;
XGCValues vals;
int i;
unsigned char *p;


	/* first fill XGCValues with ff's and try creating that rubbish */

	i = sizeof(XGCValues);
	p = (unsigned char *)&vals;
	while( i-- > 0 )
		*p++ = 0xff;

	d = DRW(display);
	values = &vals;
	vals.foreground = 1;
	vals.function = ~0;
	valuemask = GCFunction | GCForeground | 
	 ~(GCFunction | GCPlaneMask | GCForeground | GCBackground |
	  GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle |
	  GCFillStyle | GCFillRule | GCTile | GCStipple |
	  GCTileStipXOrigin | GCTileStipYOrigin | GCFont | GCSubwindowMode |
	  GCGraphicsExposures | GCClipXOrigin | GCClipYOrigin | GCClipMask |
	  GCDashOffset | GCDashList | GCArcMode);
	XCALL;

	if(geterr() == BadValue)
		PASS;
	else
		FAIL;

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
Create a GC with function = GXclear + GXand + GXandReverse + GXcopy + 
	GXandInverted + GXnoop + GXxor + GXor + GXnor + GXequiv + GXinvert +
	GXorReverse + GXcopyInverted + GXorInverted + GXnand + GXset + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.function = GXclear + GXand + GXandReverse + GXcopy + 
	  GXandInverted + GXnoop + GXxor + GXor + GXnor + GXequiv + GXinvert + 
	  GXorReverse + GXcopyInverted + GXorInverted + GXnand + GXset + 1;
	values = &vals;
	valuemask = GCFunction;
	d = DRW(display);
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
Create a GC with line_style = LineSolid + LineDoubleDash + LineOnOffDash + 1;
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.line_style = LineSolid + LineDoubleDash + LineOnOffDash + 1;
	values = &vals;
	d = DRW(display);
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
Create a GC with cap_style = CapNotLast + CapButt + CapRound + CapProjecting + 1
Verify that a BadValue error occurred
>>CODE BadValue
XGCValues vals;

	vals.cap_style = CapNotLast + CapButt + CapRound + CapProjecting + 1;
	values = &vals;
	valuemask = GCCapStyle;
	d = DRW(display);
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
Create a GC with join_style = JoinMiter + JoinRound + JoinBevel + 1
Verify that a BadValue error occurs
>>CODE BadValue
XGCValues vals;

	vals.join_style = JoinMiter + JoinRound + JoinBevel + 1;
	values = &vals;
	d = DRW(display);
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
Create a GC with fill_style = FillSolid + FillTiled + FillStippled + FillOpaqueStippled + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.fill_style = FillSolid + FillTiled + FillStippled + FillOpaqueStippled + 1;
	values = &vals;
	d = DRW(display);
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
Create a gc with fill_rule = EvenOddRule + WindingRule + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.fill_rule = EvenOddRule + WindingRule + 1;
	values = &vals;
	d = DRW(display);
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
Create a gc with arc_mode = ArcChord + ArcPieSlice + 1.
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.arc_mode = ArcChord + ArcPieSlice + 1;
	values = &vals;
	d = DRW(display);
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
Create a GC with subwindow_mode = ClipByChildren + IncludeInferiors + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.subwindow_mode = ClipByChildren + IncludeInferiors + 1;
	values = &vals;
	valuemask = GCSubwindowMode;
	d = DRW(display);
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
Create a GC with graphics_exposures component set to  
	(int) False + (int) True + 1
Verify that a BadValue error occurred.
>>CODE BadValue
XGCValues vals;

	vals.graphics_exposures = (int) False + (int) True + 1;
	values = &vals;
	valuemask = GCGraphicsExposures;
	d = DRW(display);
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
Create a GC with dashes component = 0
Verify that a BadValue error occurs.
>>CODE BadValue
XGCValues vals;

	vals.dashes = (char) 0;
	values = &vals;
	d = DRW(display);
	valuemask = GCDashList;

	XCALL;

	if( geterr() == BadValue) 
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>># HISTORY 	steve/mcy 	Completed		First prototype
>># HISTORY 	steve 		Completed		Automatic conversion to new(ish) form
>># HISTORY 	kieron 		Completed		Added dash_list assertions
>># HISTORY 	Cal 		Completed		Check and update to new format.
>># HISTORY	Kieron		Completed		<Have a look>
>># HISTORY	Cal		Action			Writing code.
