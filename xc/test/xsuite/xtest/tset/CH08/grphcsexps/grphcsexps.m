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
 * $XConsortium: grphcsexps.m,v 1.10 94/04/17 21:07:40 rws Exp $
 */
>>TITLE GraphicsExpose CH08
>>EXTERN
#define	EVENT		GraphicsExpose
>>ASSERTION Good A
When xname events are generated,
then they are contiguously delivered by the server.
>>STRATEGY
Create a pair of windows.
Create a gc with graphics-exposures flag set to True.
Generate GraphicsExpose events.
Verify that events were generated.
Verify that events were delivered contiguously.
>>CODE
Display	*display = Dsp;
int	i;
int	expected;
int	lastcount;
Window  w1, w2;
XVisualInfo *vp;
GC	gc;
int	numevents;
unsigned int	width;

/* Create a pair of windows. */
	resetvinf(VI_WIN);
	nextvinf(&vp);
		
	winpair(display, vp, &w1, &w2);
	getsize(display, w1, &width, (unsigned int *) NULL);
/* Create a gc with graphics-exposures flag set to True. */
	gc = makegc(display, w2);
	XSetGraphicsExposures(display, gc, True);
	XSelectInput(display, w1, NoEventMask);	/* is this necessary?...pc */
	XSelectInput(display, w2, NoEventMask);
/* Generate GraphicsExpose events. */
	XSync(display, True);
	XCopyPlane(display, w1, w2, gc, 50, 0, width, 40, 0, 5, 1);
	XSync(display, False);

/* Verify that events were generated. */
	numevents = XPending(display);
	if (numevents == 0) {
		delete("No events delivered");
		return;
	}
	else
		CHECK;
/* Verify that events were delivered contiguously. */
	expected = -1;
	for (i=0; i<numevents; i++) {
		XEvent	event_return;

		if (i == 0)
			CHECK;
		XNextEvent(display, &event_return);
		if (event_return.type != EVENT) {
			report("Received %s event while only expecting %s types",
				eventname(event_return.type), eventname(EVENT));
			delete("Unexpected event received.");
			return;
		}
		lastcount = event_return.xexpose.count;
		if (expected == -1) {
			expected = lastcount;
		}
		else {
			if (lastcount > expected) {
				report("Count (%d) greater than expected (%d)",
					lastcount, expected);
				FAIL;
				break;
			}
			else if (lastcount < expected)
				expected--;
		}
	}
	if (lastcount != 0) {
		report("Last %s had count set to %d, not zero",
			eventname(EVENT), lastcount);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>#NOTEm >>ASSERTION
>>#NOTEm When a call to
>>#NOTEm .F XCopyArea
>>#NOTEm or
>>#NOTEm .F XCopyPlane
>>#NOTEm is made
>>#NOTEm and the source region is obscured
>>#NOTEm and the graphics-expose attribute
>>#NOTEm of the graphics context is set to
>>#NOTEm .S True ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEm >>ASSERTION
>>#NOTEm When a call to
>>#NOTEm .F XCopyArea
>>#NOTEm or
>>#NOTEm .F XCopyPlane
>>#NOTEm is made
>>#NOTEm and the source region is out-of-bounds
>>#NOTEm and the graphics-expose attribute
>>#NOTEm of the graphics context is set to
>>#NOTEm .S True ,
>>#NOTEm then ARTICLE xname event is generated.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M type
>>#NOTEs is set to
>>#NOTEs xname.
>>#NOTEs >>ASSERTION
>>#NOTEs >>#NOTE The method of expansion is not clear.
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M serial
>>#NOTEs is set
>>#NOTEs from the serial number reported in the protocol
>>#NOTEs but expanded from the 16-bit least-significant bits
>>#NOTEs to a full 32-bit value.
>>#NOTEm >>ASSERTION
>>#NOTEm When ARTICLE xname event is delivered
>>#NOTEm and the event came from a
>>#NOTEm .S SendEvent
>>#NOTEm protocol request,
>>#NOTEm then
>>#NOTEm .M send_event
>>#NOTEm is set to
>>#NOTEm .S True .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered
>>#NOTEs and the event was not generated by a
>>#NOTEs .S SendEvent
>>#NOTEs protocol request,
>>#NOTEs then
>>#NOTEs .M send_event
>>#NOTEs is set to
>>#NOTEs .S False .
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M display
>>#NOTEs is set to
>>#NOTEs a pointer to the display on which the event was read.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M drawable
>>#NOTEs is set to
>>#NOTEs the drawable of the destination region on
>>#NOTEs which the graphics request was to be performed.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M x
>>#NOTEs and
>>#NOTEs .M y
>>#NOTEs are set to
>>#NOTEs the coordinates relative to the window's origin
>>#NOTEs and indicate the upper-left corner of the rectangle defining the exposed region.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M width
>>#NOTEs and
>>#NOTEs .M height
>>#NOTEs are set to
>>#NOTEs the size (extent) of the rectangle.
>>#NOTEs >>ASSERTION
>>#NOTEs When
>>#NOTEs .M count
>>#NOTEs is set to zero,
>>#NOTEs then no further xname events are to follow for the exposed DRAWABLE.
>>#NOTEs >>ASSERTION
>>#NOTEs When
>>#NOTEs .M count
>>#NOTEs is greater than zero,
>>#NOTEs then at least
>>#NOTEs .M count
>>#NOTEs xname events
>>#NOTEs are to follow for the exposed DRAWABLE.
>>#NOTEs >>ASSERTION
>>#NOTEs When ARTICLE xname event is delivered,
>>#NOTEs then
>>#NOTEs .M major_code
>>#NOTEs is set to either
>>#NOTEs .S X_CopyArea
>>#NOTEs or
>>#NOTEs .S X_CopyPlane
>>#NOTEs and
>>#NOTEs .M minor_code
>>#NOTEs is set to zero.
