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
 * $XConsortium: ptbckevnt.m,v 1.8 94/04/17 21:08:08 rws Exp $
 */
>>TITLE XPutBackEvent CH08
void
XPutBackEvent(display, event)
Display *display = Dsp;
XEvent	*event = &_event;
>>EXTERN
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_()	\
		_startcall(display);\
		XPutBackEvent(display, event);\
		_endcall(display)
static XEvent _event;
>>ASSERTION Good A
A call to xname pushes a copy of
.A event
onto the head of the display's event queue.
>>STRATEGY
Call XSync to empty event queue.
Call XPutBackEvent to push event onto the head of the event queue.
Call XPeekEvent to verify that first event is the event that was pushed.
Call XPutBackEvent to push another event onto the head of the event queue.
Call XPeekEvent to verify that first event is the event that was pushed.
>>CODE
XEvent	event_return;

/* Call XSync to empty event queue. */
	XSync(display, True);
/* Call XPutBackEvent to push event onto the head of the event queue. */
	event->type = ButtonPress;
	_xcall_();
/* Call XPeekEvent to verify that first event is the event that was pushed. */
	XPeekEvent(display, &event_return);
	if (event_return.type != event->type) {
		report("Returned %s, expected %s", eventname(event_return.type), eventname(event->type));
		FAIL;
	}
	else
		CHECK;
/* Call XPutBackEvent to push another event onto the head of the event queue. */
	event->type = KeyPress;
	_xcall_();
/* Call XPeekEvent to verify that first event is the event that was pushed. */
	XPeekEvent(display, &event_return);
	if (event_return.type != event->type) {
		report("Returned %s, expected %s", eventname(event_return.type), eventname(event->type));
		FAIL;
	}
	else
		CHECK;
	XSync(display, True);
	
	CHECKPASS(2);
>>ASSERTION Good B 5
A call to xname
can be made an unlimited number of times in succession.
>>STRATEGY
Call XPutBackEvent 1000 times.
>>CODE
int	i;

/* Call XPutBackEvent 1000 times. */
	event->type = ButtonPress;
	for (i=0; i<1000; i++) {
		_xcall_();
	}
	UNTESTED;
