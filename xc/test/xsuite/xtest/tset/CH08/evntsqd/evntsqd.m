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
 * $XConsortium: evntsqd.m,v 1.12 94/04/17 21:07:30 rws Exp $
 */
>>TITLE XEventsQueued CH08
int
XEventsQueued(display, mode)
Display *display = Dsp;
int mode;
>>EXTERN

/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_(return_value)	\
		_startcall(display);\
		return_value = XEventsQueued(display, mode);\
		_endcall(display)
>>ASSERTION Good A
When the number of events already in the event queue is non-zero,
then a call to xname
returns the number of events
in the event queue.
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAlready.
Verify that XEventsQueued returned the correct number of events.
Call XEventsQueued with mode QueuedAfterFlush.
Verify that XEventsQueued returned the correct number of events.
Call XEventsQueued with mode QueuedAfterReading.
Verify that XEventsQueued returned the correct number of events.
>>CODE
int	eventsput;
int	event_count;
XEvent	event;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAlready. */
	mode = QueuedAlready;
	_xcall_(event_count);
/* Verify that XEventsQueued returned the correct number of events. */
	if (event_count != eventsput) {
		report("Returned %d, expected %d", event_count, eventsput);
		FAIL;
	}
	else
		CHECK;
/* Call XEventsQueued with mode QueuedAfterFlush. */
	mode = QueuedAfterFlush;
	_xcall_(event_count);
/* Verify that XEventsQueued returned the correct number of events. */
	if (event_count != eventsput) {
		report("Returned %d, expected %d", event_count, eventsput);
		FAIL;
	}
	else
		CHECK;
/* Call XEventsQueued with mode QueuedAfterReading. */
	mode = QueuedAfterReading;
	_xcall_(event_count);
/* Verify that XEventsQueued returned the correct number of events. */
	if (event_count != eventsput) {
		report("Returned %d, expected %d", event_count, eventsput);
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(3);
>>ASSERTION Good A
When the number of events already in the event queue is non-zero,
then a call to xname
does not block.
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAlready
and verify that blocking did not occur.
Verify that XEventsQueued returned the expected number of events.
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAfterFlush
and verify that blocking did not occur.
Verify that XEventsQueued returned the expected number of events.
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAfterReading
and verify that blocking did not occur.
Verify that XEventsQueued returned the expected number of events.
>>CODE
int	eventsput;
XEvent	event;
Block_Info info;
int	block_status;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAlready */
/* and verify that blocking did not occur. */
	XEventsQueued_Type(info, QueuedAlready);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 1) {
		report("Blocking occurred when it should not have.");
		FAIL;
	}
	else
		CHECK;
/* Verify that XEventsQueued returned the expected number of events. */
	if (info.int_return != eventsput) {
		delete("Unexpected number of events in event queue: %d", info.int_return);
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAfterFlush */
/* and verify that blocking did not occur. */
	XEventsQueued_Type(info, QueuedAfterFlush);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 1) {
		report("Blocking occurred when it should not have.");
		FAIL;
	}
	else
		CHECK;
/* Verify that XEventsQueued returned the expected number of events. */
	if (info.int_return != eventsput) {
		delete("Unexpected number of events in event queue: %d", info.int_return);
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAfterReading */
/* and verify that blocking did not occur. */
	XEventsQueued_Type(info, QueuedAfterReading);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 1) {
		report("Blocking occurred when it should not have.");
		FAIL;
	}
	else
		CHECK;
/* Verify that XEventsQueued returned the expected number of events. */
	if (info.int_return != eventsput) {
		delete("Unexpected number of events in event queue: %d", info.int_return);
		return;
	}
	else
		CHECK;
	CHECKPASS(9);
>>ASSERTION Good A
When the number of events already in the event queue is non-zero,
then a call to xname
does not flush the output buffer.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAlready.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Verify that the output buffer was not flushed by effect on server.
Discard all events on the event queue.
Create pixmap.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAfterFlush.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Verify that the output buffer was not flushed by effect on server.
Discard all events on the event queue.
Create pixmap.
Call XPutBackEvent to put events on the event queue.
Call XEventsQueued with mode QueuedAfterReading.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Verify that the output buffer was not flushed by effect on server.
Discard all left-over events in the event queue.
>>CODE
XEvent	event;
int	eventsput;
int	event_count;
Pixmap	pm;
Display *client2;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAlready. */
	mode = QueuedAlready;
	_xcall_(event_count);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Verify that the output buffer was not flushed by effect on server. */
	_startcall(client2);
	XFreePixmap(client2, pm);
	XSync(client2, True);
	_endcall(client2);
	if (geterr() == Success) {
		/* pixmap was free'd */
		report("The output buffer was flushed.");
		FAIL;
	}
	else	/* no need to free as not created */
		CHECK;

	if (event_count != eventsput) {
		report("Incorrect number of events returned.");
		FAIL;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAfterFlush. */
	mode = QueuedAfterFlush;
	_xcall_(event_count);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Verify that the output buffer was not flushed by effect on server. */
	_startcall(client2);
	XFreePixmap(client2, pm);
	XSync(client2, True);
	_endcall(client2);
	if (geterr() == Success) {
		/* pixmap was free'd */
		report("The output buffer was flushed.");
		FAIL;
	}
	else	/* no need to free as not created */
		CHECK;

	if (event_count != eventsput) {
		report("Incorrect number of events returned.");
		FAIL;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = MapNotify;
	eventsput = 0;
	XPutBackEvent(display, &event), eventsput++;
/* Call XEventsQueued with mode QueuedAfterReading. */
	mode = QueuedAfterReading;
	_xcall_(event_count);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Verify that the output buffer was not flushed by effect on server. */
	_startcall(client2);
	XFreePixmap(client2, pm);
	XSync(client2, True);
	_endcall(client2);
	if (geterr() == Success) {
		/* pixmap was free'd */
		report("The output buffer was flushed.");
		FAIL;
	}
	else	/* no need to free as not created. */
		CHECK;

	if (event_count != eventsput) {
		report("Incorrect number of events returned.");
		FAIL;
	}
	else
		CHECK;
/* Discard all left-over events in the event queue. */
	_startcall(display);
	XSync(display, True);
	_endcall(display);

	CHECKPASS(7);
>>ASSERTION Good A
When there are no events in the event queue,
then a call to xname with
.A mode
set to
.S QueuedAlready
returns zero.
>>STRATEGY
Discard all events on the event queue.
Call XEventsQueued with mode QueuedAlready.
Verify that XEventsQueued returned the correct number of events.
>>CODE
int	event_count;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XEventsQueued with mode QueuedAlready. */
	mode = QueuedAlready;
	_xcall_(event_count);
/* Verify that XEventsQueued returned the correct number of events. */
	if (event_count != 0) {
		report("Returned %d, expected none", event_count);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When there are no events in the event queue,
then a call to xname with
.A mode
set to
.S QueuedAfterFlush
flushes the output buffer.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Call XEventsQueued with mode QueuedAfterFlush.
Empty the buffer.
Ensure the server has dealt with anything flushed to it: do XSync()
Verify that the output buffer was flushed by effect on server.
Verify that XEventsQueued returned the correct number of events.
Verify that errors did not occur after call to XEventsQueued.
>>CODE
int	event_count;
Pixmap	pm;
Display *client2;

/* Create client2. */
	client2 = opendisplay();
	if (client2 == (Display *) NULL) {
		delete("Can not open display");
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Create pixmap. */
	/* avoid using makepixm() */
	pm = XCreatePixmap(display, DRW(display), 10, 10, 1);
/* Call XEventsQueued with mode QueuedAfterFlush. */
	mode = QueuedAfterFlush;
	_xcall_(event_count);
/* Empty the buffer. */
	(void)XTestDiscard(display);
/* Ensure the server has dealt with anything flushed to it: do XSync() */
	XSync(display, False);
/* Verify that the output buffer was flushed by effect on server. */
	_startcall(client2);
	XFreePixmap(client2, pm);
	XSync(client2, True);
	_endcall(client2);
	if (geterr() != Success) {
		report("The output buffer was not flushed.");
		XFreePixmap(display, pm);
		FAIL;
	}
	else
		CHECK;
/* Verify that XEventsQueued returned the correct number of events. */
	if (event_count != 0) {
		report("Returned %d, expected none", event_count);
		FAIL;
	}
	else
		CHECK;
/* Verify that errors did not occur after call to XEventsQueued. */
	if (geterr() != Success) {
		report("The output buffer was flushed.");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Good A
When there are no events in the event queue,
then a call to xname with
.A mode
set to
.S QueuedAfterFlush
or
.S QueuedAfterReading
attempts to read more events out of the client's connection
without blocking
and returns the number read.
>>STRATEGY
Discard all events on the event queue.
Call XEventsQueued with mode QueuedAfterFlush
and verify that blocking did not occur.
Discard all events on the event queue.
Call XEventsQueued with mode QueuedAfterReading
and verify that blocking did not occur.
>>CODE
Block_Info info;
int	block_status;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XEventsQueued with mode QueuedAfterFlush */
/* and verify that blocking did not occur. */
	XEventsQueued_Type(info, QueuedAfterFlush);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 1) {
		report("Blocking occurred with QueuedAfterFlush when it should not have.");
		FAIL;
	}
	else
		CHECK;
	if (info.int_return != 0) {
		delete("Unexpected number of events in event queue: %d", info.int_return);
		return;
	}
	else
		CHECK;
/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XEventsQueued with mode QueuedAfterReading */
/* and verify that blocking did not occur. */
	XEventsQueued_Type(info, QueuedAfterReading);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 1) {
		report("Blocking occurred with QueuedAfterReading when it should not have.");
		FAIL;
	}
	else
		CHECK;
	if (info.int_return != 0) {
		delete("Unexpected number of events in event queue: %d", info.int_return);
		return;
	}
	else
		CHECK;
	CHECKPASS(6);
