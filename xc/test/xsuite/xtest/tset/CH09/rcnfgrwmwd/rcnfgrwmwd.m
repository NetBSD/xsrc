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
 * $XConsortium: rcnfgrwmwd.m,v 1.12 94/04/17 21:08:52 rws Exp $
 */
>>TITLE XReconfigureWMWindow CH09
Status
XReconfigureWMWIndow(display, w, screen_number, value_mask, values)
Display		*display = Dsp;
Window		w = DRW(Dsp);
int		screen_number = DefaultScreen(Dsp);
unsigned int	value_mask = 0;
XWindowChanges	*values = &wchanges;
>>EXTERN
XWindowChanges	wchanges;
>>ASSERTION Good A
A call to xname issues a 
.S ConfigureWindow 
request on the specified top-level window named by the
.A w
argument.
>>STRATEGY
Create a window using XCreateWindow.
Select ConfigureNotify events on the window using XSelectInput with SubstructureNotifyMask.
Generate a ConfigureWindow event on the window using XReconfigureWMWindow.
Verify that the call returned non-zero.
Verify that a single event was generated.
Verify that the event type was ConfigureWindow.
>>CODE
Status			status;
XEvent			ev;
int			nevents = 0;
XVisualInfo		*vp;
XWindowChanges		changes;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);
	XSelectInput(display, w, StructureNotifyMask);

	screen_number = DefaultScreen(display);
	value_mask = CWBorderWidth;
	changes.border_width = 5;
	values = &changes;

	status = XCALL;

	if(status == 0) {
		report("%s() returned zero.", TestName);
		FAIL;
	} else
		CHECK;

	if( (nevents = getevent(display, &ev)) == 0 ) {
		report("No event was generated.");
		FAIL;
		
	} else {

		CHECK;
		if(nevents != 1) {
			delete("There were %d events generated instead of 1.", nevents);
			return;
		} else	
			CHECK;

		if(ev.type != ConfigureNotify) {
			report("The type component of the generated event was %d instead of ConfigureWindow (%d)",
				ev.xclient.type, ConfigureNotify);
			FAIL;		
		} else
			CHECK;
	}

	CHECKPASS(4);

>>ASSERTION Good A
When the ConfigureWindow request fails with a 
.S BadMatch 
error, then a call to xname sends a synthetic 
.S ConfigureRequestEvent 
to the root of the window specified by the
.A w
argument, with the event
containing the same configuration parameters specified by the
.A values
and
.A valuemask
arguments
and having a
.M window
element of
.A w
and a
.M parent
element equal to the receiving root window,
using an event mask of
.S SubstructureRedirectMask |
.S SubstructureNotifyMask 
and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Select ConfigureRequest events using XSelectInput with SubstructureNotifyMask.
Call XReconfigureWMWindow using the root window as a sibling and no specified StackMode.
Verify that the call did not return zero.
Verify that one ConfigureRequest event is generated using XNextEvent.
Verify that the event components are correct.

Select ConfigureRequest events using XSelectInput with SubstructureRedirectMask.
Call XReconfigureWMWindow using the root window as a sibling and no specified StackMode.
Verify that the call did not return zero.
Verify that one ConfigureRequest event is generated using XNextEvent.
Verify that the event components are correct.
>>CODE
Status			status;
XEvent			rev, expectev;
int			nevents = 0;
int			i;
XVisualInfo		*vp;
XWindowChanges		changes;
unsigned long		event_mask[2];
unsigned long		valuemask = CWSibling /**/| CWStackMode/**/ | CWX | CWY | CWWidth | CWHeight | CWBorderWidth;

	event_mask[0] = SubstructureNotifyMask;
	event_mask[1] = SubstructureRedirectMask;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);
	
	expectev.type = ConfigureRequest;
	expectev.xany.display = display;
	
	expectev.xconfigurerequest.type = ConfigureRequest;
	expectev.xconfigurerequest.send_event = True;
	expectev.xconfigurerequest.display = display;
	expectev.xconfigurerequest.window = w;
	expectev.xconfigurerequest.parent = DRW(display);
	expectev.xconfigurerequest.x = 1;
	expectev.xconfigurerequest.y = 2;
	expectev.xconfigurerequest.width = 3;
	expectev.xconfigurerequest.height = 4;
	expectev.xconfigurerequest.border_width = 5;
	expectev.xconfigurerequest.above = DRW(display);
	expectev.xconfigurerequest.detail = Above;
	expectev.xconfigurerequest.value_mask = valuemask;
	
	screen_number = DefaultScreen(display);
	value_mask = valuemask; 
	changes.x = 1;
	changes.y = 2;
	changes.width = 3;
	changes.height = 4;
	changes.border_width = 5;
	changes.sibling = DRW(display);
	changes.stack_mode = Above;
	values = &changes;
	

	for(i=0; i<2; i++) {

		startcall(display);
		XSelectInput(display, DRW(display), event_mask[i]);
		endcall(display);

		if(geterr() != Success) {
			delete("XSelectInput() failed with an event mask of 0x%lx.", event_mask[i]);
			return;
		} else
			CHECK;

		status = XCALL;
	
		if(status == 0) {
			report("%s() returned zero.", TestName);
			FAIL;
		} else
			CHECK;
	
		rev.type = -1;
		if( (nevents = getevent(display, &rev)) == 0 ) {
			report("No event was generated.");
			FAIL;
	
		} else {
	
			CHECK;
			if(nevents != 1){
				delete("There were %d events generated instead of 1.", nevents);
				return;
			} else	
				CHECK;
	
			if( checkevent( &expectev, &rev) != 0)
				FAIL;
			else
				CHECK;
	
		}
	}

	CHECKPASS(10);

>>ASSERTION Bad B 1
>># Untestable, and not worth the effort of adding XTest extension facilities
>># to provoke the error.
When the
.S ConfigureRequestEvent 
is not successfully sent, then a call to xname returns zero.
>>ASSERTION Bad A
When the
.A value_mask
argument contains the bits
.S CWWidth
or
.S CWHeight
and either of the
.M width
or
.M height
components of the
.S XWindowChanges
structure named by the
.A values
argument is zero, then a
.S BadValue 
error occurs.
>>STRATEGY
Create a window using XCreateWindow.
Configure the window to have a height of zero using XReconfigureWMWindow
Verify that a BadValue error was generated.
Configure the window to have a width of zero using XReconfigureWMWindow
Verify that a BadValue error was generated.
>>CODE BadValue
Status			status;
XEvent			ev;
int			nevents = 0;
XVisualInfo		*vp;
XWindowChanges		changes;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	screen_number = DefaultScreen(display);
	value_mask = CWHeight ;
	changes.height = 0;
	values = &changes;
	status = XCALL;

	if(geterr() == BadValue)
		CHECK;

	changes.width = 0;
	value_mask = CWWidth;
	status = XCALL;

	if(geterr() == BadValue)
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad A
When the window argument does not name a valid window, then a
.S BadWindow
error occurs.
>>STRATEGY
Create a bad window by creating and destroying a window.
Call test function using bad window as the window argument.
Verify that a BadWindow error occurs.
>>CODE BadWindow
Status status;

        A_WINDOW = badwin(A_DISPLAY);

	status = XCALL;

        if (geterr() == BadWindow)
                PASS;
        else
                FAIL;

>># Completed	Kieron	Review
