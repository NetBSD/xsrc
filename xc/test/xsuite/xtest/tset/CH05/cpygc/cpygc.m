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
 * $XConsortium: cpygc.m,v 1.8 94/04/17 21:03:51 rws Exp $
 */
>>TITLE XCopyGC CH05
void
XCopyGC(display, src, valuemask, dest)
Display *display = Dsp;
GC src;
unsigned long valuemask;
GC dest;
>>SET need-gc-flush
>>ASSERTION Good A
A call to xname copies the components 
specified by the
.A valuemask
argument 
from the
.A src
argument
to the
.A dest
argument.
>>STRATEGY
Create a source GC with function set to GXxor.
Create a different, destination GC.
Copy the source GC to the destination GC with XCopyGC.
Verify that GCValues structures are identical for the destination and source GC with XGetGCValues.
Verify the destination GC component function is used in graphics operations:
   Plot point (0,0) with XDrawPoint.
   Set fg to W_FG ^ W_BG with XSetForeground.
   Plot point (0,0) with XDrawPoint.
   Verify that pixel at (0,0) is W_BG.

>>CODE
XVisualInfo	*vp;
Window w;
XGCValues srcgcv, destgcv;
unsigned long vm =  (unsigned long)GCFunction | GCPlaneMask | GCForeground |
		GCBackground | GCLineWidth | GCLineStyle | GCCapStyle |
		GCJoinStyle | GCFillStyle | GCFillRule |
		GCTileStipXOrigin | GCTileStipYOrigin |
		GCSubwindowMode | GCGraphicsExposures | GCClipXOrigin |
		GCClipYOrigin | GCDashOffset | GCArcMode;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	/* set up an XGCValues struct for source GC */

	srcgcv.function = (int)GXxor;
	srcgcv.plane_mask = ~0L;
	srcgcv.foreground = (unsigned long) W_FG;
	srcgcv.background = (unsigned long) W_FG;
	srcgcv.line_width = (int)0;
	srcgcv.line_style = (int)LineSolid;
	srcgcv.cap_style = (int)CapButt;
	srcgcv.join_style = (int)JoinMiter;
	srcgcv.fill_style = (int)FillSolid;
	srcgcv.fill_rule = (int)EvenOddRule;
	srcgcv.arc_mode = (int)ArcPieSlice;
	srcgcv.tile = (Pixmap)0;
	srcgcv.stipple = (Pixmap)0;
	srcgcv.ts_x_origin = (int)0;
	srcgcv.ts_y_origin = (int)0;
	srcgcv.font = (Font)0;
	srcgcv.subwindow_mode = (int)ClipByChildren;
	srcgcv.graphics_exposures = (Bool)True;
	srcgcv.clip_x_origin = (int)0;
	srcgcv.clip_y_origin = (int)0;
	srcgcv.clip_mask = (Pixmap)None;
	srcgcv.dash_offset = (int)0;

	/* create a GC using the source XGCValues struct */

	src = XCreateGC(display, w, vm, &srcgcv);

	if( XGetGCValues(display, src, valuemask, &srcgcv)  == False ) {
		delete("XGetGCValues failed on source GC");
		return;
	}

	/* set up an XGCValues struct for destination GC */

	destgcv.function = (int)GXnoop;
	destgcv.plane_mask = 1L;
	destgcv.foreground = (unsigned long)W_BG;
	destgcv.background = (unsigned long)W_BG;
	destgcv.line_width = (int)5;
	destgcv.line_style = (int)LineDoubleDash;
	destgcv.cap_style = (int)CapRound;
	destgcv.join_style = (int)JoinRound;
	destgcv.fill_style = (int)FillTiled;
	destgcv.fill_rule = (int)WindingRule;
	destgcv.arc_mode = (int)ArcChord;
	destgcv.tile = (Pixmap)0;
	destgcv.stipple = (Pixmap)0;
	destgcv.ts_x_origin = (int)4;
	destgcv.ts_y_origin = (int)4;
	destgcv.font = (Font)0;
	destgcv.subwindow_mode = (int)IncludeInferiors;
	destgcv.graphics_exposures = (Bool)False;
	destgcv.clip_x_origin = (int)0;
	destgcv.clip_y_origin = (int)0;
	destgcv.clip_mask = (Pixmap)None;
	destgcv.dash_offset = (int)0;

	/* create a GC using the destination XGCValues struct */

	dest = XCreateGC(display, w, vm, &destgcv);

	/* copy source GC into destination GC */

	valuemask = vm;
	XCALL;

	/* find out what destination GC looks like */

	if( XGetGCValues(display, dest, valuemask, &destgcv)  == False ) {
		delete("XGetGCValues failed on destination GC");
		return;
	}

	/* compare the source and destination GC's */

	if( destgcv.function  == srcgcv.function  ) {
		CHECK;
	} else {
		report("dest GCValue function differs from source");
		FAIL;
	}
	if( destgcv.plane_mask  == srcgcv.plane_mask  ) {
		CHECK;
	} else {
		report("dest GCValue plane_mask differs from source");
		FAIL;
	}
	if( destgcv.foreground  == srcgcv.foreground  ) {
		CHECK;
	} else {
		report("dest GCValue foreground differs from source");
		FAIL;
	}
	if( destgcv.background  == srcgcv.background  ) {
		CHECK;
	} else {
		report("dest GCValue background differs from source");
		FAIL;
	}
	if( destgcv.line_width  == srcgcv.line_width  ) {
		CHECK;
	} else {
		report("dest GCValue line_width differs from source");
		FAIL;
	}
	if( destgcv.line_style  == srcgcv.line_style  ) {
		CHECK;
	} else {
		report("dest GCValue line_style differs from source");
		FAIL;
	}
	if( destgcv.cap_style  == srcgcv.cap_style  ) {
		CHECK;
	} else {
		report("dest GCValue cap_style differs from source");
		FAIL;
	}
	if( destgcv.join_style  == srcgcv.join_style  ) {
		CHECK;
	} else {
		report("dest GCValue join_style differs from source");
		FAIL;
	}
	if( destgcv.fill_style  == srcgcv.fill_style  ) {
		CHECK;
	} else {
		report("dest GCValue fill_style differs from source");
		FAIL;
	}
	if( destgcv.fill_rule  == srcgcv.fill_rule  ) {
		CHECK;
	} else {
		report("dest GCValue fill_rule differs from source");
		FAIL;
	}
	if( destgcv.arc_mode  == srcgcv.arc_mode  ) {
		CHECK;
	} else {
		report("dest GCValue arc_mode differs from source");
		FAIL;
	}
	if( destgcv.tile  == srcgcv.tile  ) {
		CHECK;
	} else {
		report("dest GCValue tile differs from source");
		FAIL;
	}
	if( destgcv.stipple  == srcgcv.stipple  ) {
		CHECK;
	} else {
		report("dest GCValue stipple differs from source");
		FAIL;
	}
	if( destgcv.ts_x_origin  == srcgcv.ts_x_origin  ) {
		CHECK;
	} else {
		report("dest GCValue ts_x_origin differs from source");
		FAIL;
	}
	if( destgcv.ts_y_origin  == srcgcv.ts_y_origin  ) {
		CHECK;
	} else {
		report("dest GCValue ts_y_origin differs from source");
		FAIL;
	}
	if( destgcv.font  == srcgcv.font  ) {
		CHECK;
	} else {
		report("dest GCValue font differs from source");
		FAIL;
	}
	if( destgcv.subwindow_mode  == srcgcv.subwindow_mode  ) {
		CHECK;
	} else {
		report("dest GCValue subwindow_mode differs from source");
		FAIL;
	}
	if( destgcv.graphics_exposures  == srcgcv.graphics_exposures  ) {
		CHECK;
	} else {
		report("dest GCValue graphics_exposures differs from source");
		FAIL;
	}
	if( destgcv.clip_x_origin  == srcgcv.clip_x_origin  ) {
		CHECK;
	} else {
		report("dest GCValue clip_x_origin differs from source");
		FAIL;
	}
	if( destgcv.clip_y_origin  == srcgcv.clip_y_origin  ) {
		CHECK;
	} else {
		report("dest GCValue clip_y_origin differs from source");
		FAIL;
	}
	if( destgcv.clip_mask  == srcgcv.clip_mask  ) {
		CHECK;
	} else {
		report("dest GCValue clip_mask differs from source");
		FAIL;
	}
	if( destgcv.dash_offset  == srcgcv.dash_offset  ) {
		CHECK;
	} else {
		report("dest GCValue dash_offset differs from source");
		FAIL;
	}

	XDrawPoint(display, w, dest, 0, 0);
	XSetForeground(display, dest, W_FG ^ W_BG);
	XDrawPoint(display, w, dest, 0, 0);

	if( checkpixel(display, w, 0, 0, W_BG) == 0) {
		report("XCopyGC() did not set the function component of the GC.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(23);

>>ASSERTION Bad B 1
.ER Alloc
>>ASSERTION Bad C
When the
.A src
and
.A dest
arguments do not have the same depth,
then a
.S BadMatch 
error occurs.
>>STRATEGY
If multiple pixmap depths are supported:
   Create a pixmap with a different depth to the root window
   Create a GC on the root window
   Create a GC on the pixmap
   Copy from the first GC to the second using XCopyGC.
   Verify that a BadMatch error was generated.
>>CODE BadMatch
Window w;
XWindowAttributes atts;
Status  s;
int newdepth;
Pixmap pm;
int nitems, *depthlist;
unsigned long vm =  (unsigned long)GCFunction | GCPlaneMask | GCForeground |
		GCBackground | GCLineWidth | GCLineStyle | GCCapStyle |
		GCJoinStyle | GCFillStyle | GCFillRule |
		GCTileStipXOrigin | GCTileStipYOrigin |
		GCSubwindowMode | GCGraphicsExposures | GCClipXOrigin |
		GCClipYOrigin | GCDashOffset | GCArcMode;

		/* can not ask XGetGCValues for GCClipMask  or GCDashList,
		   also need pixmaps for GCTile, GCStipple and GCFont */

	w = DefaultRootWindow(display);

	s = XGetWindowAttributes(display, w, &atts);
	if (s == False) {
		delete("XGetWindowAttributes failed");
		return;
	}

	depthlist = XListDepths(display,XScreenNumberOfScreen(atts.screen),&nitems);

	/* search for a different depth to create pixmap */

	for( newdepth = atts.depth; nitems-- > 0 && depthlist; depthlist++ ) {
		if( newdepth != *depthlist ) {
			newdepth = *depthlist;
			break;
		}
	}
	if( atts.depth == newdepth ) {
		report("Screen only supports 1 depth");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	src = XCreateGC(display, w, 0L, (XGCValues *)NULL);

	pm = XCreatePixmap(display, w, 100, 100, newdepth );

	dest = XCreateGC(display, pm, 0L, (XGCValues *)NULL);

	valuemask = vm;

	XCALL;

	if (geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad C
When the
.A src
and
.A dest
arguments were not created for the same root,
then a
.S BadMatch 
error occurs.
>>STRATEGY
If multiple roots are supported:
   Create a GC for two different roots.
   Copy from the first to the second.
   Verify that a BadMatch error occurred.
   Copy from the second to the first.
   Verify that a BadMatch error occurred.
>>CODE BadMatch
XVisualInfo     vi;
char    *altroot;
int     scr_num;

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


	src = XCreateGC(display, RootWindow(display,scr_num), 0L, (XGCValues*)0);
	dest = XCreateGC(display, RootWindow(display, DefaultScreen(display)), 0L, (XGCValues*)0);
	valuemask =   (unsigned long)GCFunction | GCPlaneMask | GCForeground |
			GCBackground | GCLineWidth | GCLineStyle | GCCapStyle |
			GCJoinStyle | GCFillStyle | GCFillRule |
			GCTileStipXOrigin | GCTileStipYOrigin |
			GCSubwindowMode | GCGraphicsExposures | GCClipXOrigin |
			GCClipYOrigin | GCDashOffset | GCArcMode;
	XCALL;

	if (geterr() == BadMatch)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);

>>ASSERTION Bad A
When either the
.A src
or
.A dest
argument does not name a defined 
.S GC ,
then a
.S BadGC
error occurs.
>>STRATEGY
Create a bad GC
Create a good GC
Call XCopyGC with bad source, good destination GC's
Verify that BadGC error occurs
Call XCopyGC with good source, bad destination GC's
Verify that BadGC error occurs
Call XCopyGC with bad source, bad destination GC's
Verify that BadGC error occurs
>>CODE BadGC
Window w;
GC	gcgood, gcbad;
unsigned long vm =  (unsigned long)GCFunction | GCPlaneMask | GCForeground |
		GCBackground | GCLineWidth | GCLineStyle | GCCapStyle |
		GCJoinStyle | GCFillStyle | GCFillRule |
		GCTileStipXOrigin | GCTileStipYOrigin |
		GCSubwindowMode | GCGraphicsExposures | GCClipXOrigin |
		GCClipYOrigin | GCDashOffset | GCArcMode;

	w = DefaultRootWindow(display);

	gcgood = XCreateGC(display, w, 0L, (XGCValues *)NULL);

	gcbad = badgc(display);

	/* test bad source, good destination */

	src = gcbad;
	dest = gcgood;
	valuemask = vm;

	trace("test bad source GC, good destination GC" );

	XCALL;

	if (geterr() == BadGC)
		CHECK;
	else
		FAIL;

	/* test good source, bad destination */

	src = gcgood;
	dest = gcbad;
	valuemask = vm;

	trace("test good source GC, bad destination GC" );

	XCALL;

	if (geterr() == BadGC)
		CHECK;
	else
		FAIL;

	/* test for bad source and bad destination */

	src = gcbad;
	dest = gcbad;
	valuemask = vm;

	trace("test bad source GC, bad destination GC" );

	XCALL;

	if (geterr() == BadGC)
		CHECK;
	else
		FAIL;

	CHECKPASS(3);

>>#HISTORY	Steve	Completed	Written in old format.
>>#HISTORY	Cal	Completed	Written in new format and style.
>>#HISTORY	Kieron	Completed		<Have a look>.
>>#HISTORY	Cal	Action		Enhanced existing CODE sections.
