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
 * $XConsortium: wthdrwwdw.m,v 1.13 94/04/17 21:09:16 rws Exp $
 */
>>TITLE XWithdrawWindow CH09
Status
XWidthdrawWindow(display, w, screen_number)
Display		*display = Dsp;
Window		w = DRW(Dsp);
int		screen_number = DefaultScreen(Dsp);
>>ASSERTION Good A
A call to xname unmaps the window 
specified by the
.A w
argument
and sends,
to the root window of the screen specified by the
.A screen_number
argument, a synthetic 
.S UnmapNotify 
event with a
.M window
of
.A w ,
an
.M event
equal to the receiving root window and a
.M from_configure
element of
.S False
using an event mask of
.S SubstructureRedirectMask |
.S SubstructureNotifyMask
and returns non-zero.
>>STRATEGY
Create a window using XCreateWindow.
Select UnmapNotify events on the root using XSelectInput with SubstructureNotifyMask.
Unmap the window using XWithdrawWindow.
Verify that the window is unmapped using XGetWindowAttributes.
Verify that a non-synthetic UnmapNotify event is generated.
Verify that a synthetic UnmapNotify event is generated.

Create a window using XCreateWindow.
Select UnmapNotify events on the root using XSelectInput with SubstructureRedirectMask.
Unmap the window using XWithdrawWindow.
Verify that the window is unmapped using XGetWindowAttributes.
Verify that a non-synthetic UnmapNotify event is generated.
>>CODE
int			nevents;
XVisualInfo		*vp;
XWindowAttributes	atts;
XEvent			ev;
XEvent			rev;


	resetvinf(VI_WIN);
	nextvinf(&vp);

	ev.type = UnmapNotify;
	ev.xany.display = display;
	ev.xunmap.type = UnmapNotify;
	ev.xunmap.send_event = False;
	ev.xunmap.display = display;
	ev.xunmap.event = DRW(display);
	ev.xunmap.from_configure = False;

	CATCH_ERROR(display);
	XSelectInput(display, DRW(display), SubstructureNotifyMask);
	RESTORE_ERROR(display);

	if(GET_ERROR(display) != Success) {
		delete("XSelectInput() failed with an event mask of 0x%lx.", SubstructureNotifyMask);
		return;
	} else
		CHECK;

	w = makewin(display, vp);
	XCALL;
	
	if(XGetWindowAttributes(display, w, &atts) == False){
		delete("XGetWindowAttributes Failed on window id %lx.", (long) w);
		return;
	} else
		CHECK;
	
	if(atts.map_state != IsUnmapped) {
		report("Window ID %lx was not unmapped", (long) w);
		FAIL;
	} else
		CHECK;
	
	ev.xunmap.window = w;
	ev.xunmap.send_event = False;
	rev.type = -1;
	if( (nevents = getevent(display, &rev)) == 0 ) {
		report("No event was generated.");
		FAIL;
	} else {
		CHECK;
		if( checkevent(&ev, &rev) != 0 )
			FAIL;
		else
			CHECK;
	}
	
	ev.xunmap.send_event = True;
	rev.type = -1;

	if( (nevents = getevent(display, &rev)) == 0 ) {
		report("No event was generated.");
		FAIL;
	} else {
		CHECK;
		if( checkevent(&ev, &rev) != 0 )
			FAIL;
		else
			CHECK;
	}

	CATCH_ERROR(display);
	XSelectInput(display, DRW(display), SubstructureRedirectMask);
	RESTORE_ERROR(display);

	if(GET_ERROR(display) != Success) {
		delete("XSelectInput() failed with an event mask of 0x%lx.",SubstructureRedirectMask);
		return;
	} else
		CHECK;

	w = makewin(display, vp);
	XCALL;
	
	if(XGetWindowAttributes(display, w, &atts) == False){
		delete("XGetWindowAttributes Failed on window id %lx.", (long) w);
		return;
	} else
		CHECK;
	
	if(atts.map_state != IsUnmapped) {
		report("Window ID %lx was not unmapped", (long) w);
		FAIL;
	} else
		CHECK;
	
	ev.xunmap.window = w;
	ev.xunmap.send_event = False;
	rev.type = -1;
	if( (nevents = getevent(display, &rev)) == 0 ) {
		report("No event was generated.");
		FAIL;
	} else {
		CHECK;
		if( checkevent(&ev, &rev) != 0 )
			FAIL;
		else
			CHECK;
	}
	
	CHECKPASS(12);

>>ASSERTION Bad B 1
>># Untestable, and not worth the effort of adding XTest extension facilities
>># to provoke the error.
When the 
.S UnmapNotify 
event is  not successfully sent, then
xname returns zero. 
>>ASSERTION Good A
.ER BadWindow
>># Completed	Kieron	Review
