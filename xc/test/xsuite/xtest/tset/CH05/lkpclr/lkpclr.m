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
 * $XConsortium: lkpclr.m,v 1.8 94/04/17 21:03:59 rws Exp $
 */
>>TITLE XLookupColor CH05
Status
LookupColor(display, colormap, color_name, exact_def_return, screen_def_return)
Display *display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
char *color_name = "";
XColor *exact_def_return = &dummycol;
XColor *screen_def_return = &dummycol;
>>EXTERN
XColor dummycol;
>>ASSERTION Good A
A call to xname obtains the exact and closest available RGB
values for the
.A colormap
argument to those specified for the colour named
.A color_name
in the database, and stores the exact values in the
.M red ,
.M green
and
.M blue
components of the
.S XColor
structure named by the
.A exact_def_return 
argument, and stores the closest available values in the
.M red ,
.M green
and
.M blue
components of the
.S XColor
structure named by the
.A screen_def_return
argument.
>>STRATEGY
For each visual class:
  Create a colourmap with alloc set to AllocNone.
  Lookup the exact and closest supported rgb values 
    for colour XT_GOOD_COLORNAME with XLookupColor.
  Verify that the function returned non-zero.
  Allocate a read/only cell using returned RGB values with XAllocColor 
    (which is assumed to return correct RGB values) .
  Verify that the RBG values from both calls are identical.
    (so XLookupColor previously returned correct RGB values)
>>CODE
XVisualInfo *vp;
Status status;
XColor exactcol, screencol, testcol;
unsigned long vmask;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	if( (color_name = tet_getvar("XT_GOOD_COLORNAME")) == (char *) 0) {
		delete("XT_GOOD_COLORNAME is not defined.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocNone);
		exact_def_return = &exactcol;
		screen_def_return = &screencol;
		status = XCALL;

		if( status == (Status) 0) {
			report("%s failed to return non-zero.", TestName);
			FAIL;
			continue;
		} else
			CHECK;

		testcol = screencol;
		trace("Screen: r %u g %u b %u", screencol.red, screencol.green, screencol.blue);
		trace("Exact : r %u g %u b %u", exactcol.red, exactcol.green, exactcol.blue);
		trace("Test : r %u g %u b %u", testcol.red, testcol.green, testcol.blue);

		status = XAllocColor(display, colormap, &testcol);
		if(status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;
		} else
			CHECK;

		trace("Exact : r %u g %u b %u", exactcol.red, exactcol.green, exactcol.blue);

		if((screencol.red != testcol.red) ||
		   (screencol.green != testcol.green) ||
	 	   (screencol.blue != testcol.blue) ) {
			report("%s return RGB values r %u g %u b %u instead of r %u g %u b %u.", TestName,
				screencol.red, screencol.green, screencol.blue,
				testcol.red, testcol.green, testcol.blue);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(3 * nsupvis());

>>ASSERTION  Good A
Upper and lower case characters  in the
.A color_name
argument refer to the same colour.
>>STRATEGY
For each supported visual type:
  Create a colomap with alloc set to AllocNone.
  Look up the rgb value of the colour name XT_GOOD_COLORNAME 
    in the database with XLookupNamedColor.
  Look up the rbg value of the colour name XT_GOOD_COLORNAME, 
    with alternating characters in alternating case,
    in the colourmap with XAllocNamedColor.
  Verify that the function returned non-zero.
  Verify that the exact and closest supported rbg values from both calls 
    are identical.
>>CODE
XVisualInfo *vp;
char *cp, *goodname, *casename;
Status status;
XColor screencol, exactcol, alscreencol, alexactcol;
unsigned long vmask;
unsigned short trunc;
int i;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	if( ( goodname = tet_getvar("XT_GOOD_COLORNAME")) == (char *) 0) {
		delete("XT_GOOD_COLORNAME is not defined.");
		return;
	}

	casename = (char *) malloc( strlen(goodname) + 1);
	strcpy(casename, goodname);

	for(i=0, cp=casename; *cp; i++)
		if(i&1)
			*cp++ = tolower(*cp);
		else
			*cp++ = toupper(*cp);


	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp -> visual, AllocNone);
		color_name = goodname;
		screen_def_return = &screencol;
		exact_def_return = &exactcol;
		status = XCALL;

		if( status == (Status) 0) {
			report("%s failed to return non-zero with color %s.", 
				TestName, goodname);
			FAIL;
			continue;
		} else
			CHECK;
			
		color_name = casename;
		trace("Testing colourname %s", color_name);
		screen_def_return = &alscreencol;
		exact_def_return = &alexactcol;
		status = XCALL;

		if( status == (Status) 0) {
			report("%s failed to return non-zero with color %s.", 
				TestName, casename);
			FAIL;
			continue;
		} else 
			CHECK;

		if((exactcol.red != alexactcol.red) ||
		   (exactcol.green != alexactcol.green) ||
		   (exactcol.blue != alexactcol.blue)) {
			report("%s for name %s", TestName, casename);
			report("returned exact RGB values r %u g %u b %u",
			  alexactcol.red, alexactcol.green, alexactcol.blue);
			report("%s for name %s", TestName, goodname);
			report("returned exact RGB values r %u g %u b %u",
			  exactcol.red, exactcol.green, exactcol.blue);
			FAIL;
		} else
			CHECK;

		if((screencol.red != alscreencol.red) ||
		   (screencol.green != alscreencol.green) ||
		   (screencol.blue != alscreencol.blue)) {
			report("%s for name %s", TestName, casename);
			report("returned closest RGB values r %u g %u b %u",
			  alscreencol.red, alscreencol.green, alscreencol.blue);
			report("%s for name %s", TestName, goodname);
			report("returned closest RGB values r %u g %u b %u",
			  screencol.red, screencol.green, screencol.blue);
			FAIL;
		} else
			CHECK;
	}

	free(casename);
	CHECKPASS(4 * nsupvis());

>>ASSERTION Good A
When the 
.A color_name
argument refers to a colour in the colour database, then xname
returns non-zero.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap.
  Lookup the rgb values for XT_GOOD_COLOR_NAME with XLookupColor.
  Verify that the function returned non-zero.
>>CODE
XVisualInfo *vp;
Status status;
char *goodname;
XColor exactcol, screencol;
unsigned long vmask;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	if( (goodname = tet_getvar("XT_GOOD_COLORNAME")) == (char *) 0) {
		delete("XT_GOOD_COLORNAME is not defined.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		
		colormap = makecolmap(display, vp->visual, AllocNone);
		color_name = goodname;
		exact_def_return = &exactcol;
		screen_def_return = &screencol;
		status = XCALL;

		if( status == (Status) 0) {
			report("%s failed to return non-zero with color %s.", 
				TestName, goodname);
			FAIL;
		} else 
			CHECK;
	}

	CHECKPASS(nsupvis());

>>ASSERTION Good A
When the
.A color_name
argument does not refer to a colour in the colour database, then xname
returns zero.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap.
  Lookup the rgb values for XT_BAD_COLORNAME with XLookupColor.
  Verify that the function returned zero.
>>CODE
XVisualInfo *vp;
char *badname;
Status status;
XColor exactcol, screencol;
unsigned long vmask;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	if( (badname = tet_getvar("XT_BAD_COLORNAME")) == (char *) 0) {
		delete("XT_BAD_COLORNAME is not defined.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		
		colormap = makecolmap(display, vp->visual, AllocNone);
		color_name = badname;
		exact_def_return = &exactcol;
		screen_def_return = &screencol;
		status = XCALL;

		if( status != (Status) 0) {
			report("%s failed to return zero with color %s.", 
				TestName, badname);
			FAIL;
		} else 
			CHECK;
	}

	CHECKPASS(nsupvis());

>>#HISTORY	Cal	Completed	Written in new style and format.
>>#HISTORY	kieron	Completed	<have a look>
>>#HISTORY	Cal	Action		Writing code.
