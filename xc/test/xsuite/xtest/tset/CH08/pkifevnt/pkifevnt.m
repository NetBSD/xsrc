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
 * $XConsortium: pkifevnt.m,v 1.9 94/04/17 21:08:01 rws Exp $
 */
>>TITLE XPeekIfEvent CH08
void
XPeekIfEvent(display, event_return, predicate, arg)
Display *display = Dsp;
XEvent	*event_return = &_event;
Predicate predicate = _predicate;
char *arg = (char *) 0;
>>EXTERN
/*
 * Can not use "xcall" because it empties the event queue.
 */
#define	_xcall_()	\
		_startcall(display);\
		XPeekIfEvent(display, event_return, predicate, arg);\
		_endcall(display)
static XEvent _event;

/*
 * _predicate - a predicate procedure
 *
 * _predicate returns True only after it has been invoked the specified
 * number of times (_pred_max).  _pred_cnt is used to keep track of the
 * number of invocations.  _pred_retval always contains the previous
 * _predicate return value.  _pred_true is set to True if _predicate is
 * ever invoked while _pred_retval is set to True.  _pred_event contains
 * a copy of the event most recently passed to _predicate.
 *
 * These variables should be initialized by a call to PRED_SETUP() prior
 * to (indirectly!) invoking the predicate procedure.  PRED_SETUP takes
 * an argument which specifies at which invocation _predicate should
 * start returning True.
 */
static int _pred_max;
static int _pred_cnt;		/* _predicate invocation counter */
static XEvent _pred_event;	/* last event passed to _predicate */
static int _pred_retval;	/* last returnvalue from _predicate */
static int _pred_true;		/* True when True previously returned */

static int
_predicate (display, event, arg)
Display *display;
XEvent *event;
char *arg;
{
#ifdef	lint
	XCloseDisplay(display);
	*arg = '\0';
#endif
	_pred_event = *event;
	if (_pred_retval == True)
		_pred_true = True;
	_pred_retval = ((++_pred_cnt >= _pred_max) ? True : False);
	return(_pred_retval);
}

#define	PRED_SETUP(max)	\
		_pred_max = (max);\
		_pred_cnt = 0;\
		_pred_retval = False;\
		_pred_true = False

>>ASSERTION Good A
A call to xname
calls
.A predicate
once for each event in the event queue until
.A predicate
returns
.S True .
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Set up predicate procedure.
Call XPeekIfEvent.
Verify that predicate was called the correct number of times.
Verify that predicate returned True at most recent invocation.
Verify that XIfEvent did not continue to call predicate
after predicate returned True.
>>CODE
XEvent	event;
int	callcnt;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = KeyPress;
	XPutBackEvent(display, &event);
	event.type = KeyRelease;
	XPutBackEvent(display, &event);
	event.type = ButtonPress;
	XPutBackEvent(display, &event);
/* Set up predicate procedure. */
	PRED_SETUP(callcnt = 2);
/* Call XPeekIfEvent. */
	_xcall_();
/* Verify that predicate was called the correct number of times. */
	if (_pred_cnt != callcnt) {
		report("predicate called %d times, expected %d", _pred_cnt, callcnt);
		FAIL;
	}
	else
		CHECK;
/* Verify that predicate returned True at most recent invocation. */
	if (_pred_retval != True) {
		report("predicate returned %d, expecting %d", _pred_retval, True);
		FAIL;
	}
	else
		CHECK;
/* Verify that XIfEvent did not continue to call predicate */
/* after predicate returned True. */
	if (_pred_true == True) {	
		report("Did not return when predicate returned True.");
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);
	
	CHECKPASS(3);
>>ASSERTION Good A
When
.A predicate
returns
.S True ,
then xname returns the
.A event
passed to
.A predicate
in
.A event_return .
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Set up predicate procedure.
Call XPeekIfEvent.
Verify that predicate returned True at most recent invocation.
Verify that event_return is the same as the event passed to predicate.
Verify that event_return is the expected event.
Verify that XIfEvent did not continue to call predicate
after predicate returned True.
>>CODE
XEvent	event;
int	callcnt;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = KeyPress;
	XPutBackEvent(display, &event);
	event.type = KeyRelease;
	XPutBackEvent(display, &event);
	event.type = ButtonPress;
	XPutBackEvent(display, &event);
/* Set up predicate procedure. */
	PRED_SETUP(callcnt = 2);
/* Call XPeekIfEvent. */
	_xcall_();
/* Verify that predicate returned True at most recent invocation. */
	if (_pred_retval != True) {
		report("predicate returned %d, expecting %d", _pred_retval, True);
		FAIL;
	}
	else
		CHECK;
/* Verify that event_return is the same as the event passed to predicate. */
	if (event_return->type != _pred_event.type) {	
		report("Returned %s, expected %s", eventname(event_return->type), eventname(_pred_event.type));
		FAIL;
	}
	else
		CHECK;
/* Verify that event_return is the expected event. */
	if (event_return->type != KeyRelease) {	
		report("Returned %s, expected %s", eventname(event_return->type), eventname(KeyRelease));
		FAIL;
	}
	else
		CHECK;
/* Verify that XIfEvent did not continue to call predicate */
/* after predicate returned True. */
	if (_pred_true == True) {	
		report("Did not return when predicate returned True.");
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(4);
>>ASSERTION Good A
A call to xname
does not remove
.A event_return
from the event queue.
>>STRATEGY
Discard all events on the event queue.
Call XPutBackEvent to put events on the event queue.
Call XPending to get the current event queue size.
Set up predicate procedure.
Call XPeekIfEvent.
Call XPending to get the current event queue size.
Verify that size of the event queue has not changed.
Verify that the returned event was not removed from the event queue.
>>CODE
XEvent	event;
XEvent	nextevent;
int	callcnt;
int	oldqsize;
int	newqsize;

/* Discard all events on the event queue. */
	XSync(display, True);
/* Call XPutBackEvent to put events on the event queue. */
	event.type = KeyPress;
	XPutBackEvent(display, &event);
	event.type = KeyRelease;
	XPutBackEvent(display, &event);
	event.type = ButtonPress;
	XPutBackEvent(display, &event);
/* Call XPending to get the current event queue size. */
	oldqsize = XPending(display);
/* Set up predicate procedure. */
	PRED_SETUP(callcnt = 1);
/* Call XPeekIfEvent. */
	_xcall_();
/* Call XPending to get the current event queue size. */
	newqsize = XPending(display);
/* Verify that size of the event queue has not changed. */
	if (newqsize != oldqsize) {
		report("Event queue size %d, expected %d", newqsize, oldqsize);
		FAIL;
	}
	else
		CHECK;
/* Verify that the returned event was not removed from the event queue. */
	XNextEvent(display, &nextevent);
	if (event_return->type != nextevent.type) {
		report("Event removed from queue, returned %s, expected %s", eventname(event_return->type), eventname(ButtonPress));
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When on a call to xname
.A predicate
has not returned
.S True
after having been called once for each event in the event queue,
then xname flushes the output buffer and blocks until a matching
event is received.
>>STRATEGY
Create client2.
Discard all events on the event queue.
Create pixmap.
Call XPutBackEvent to put an event on the event queue.
Set up predicate procedure.
Call XPeekIfEvent and verify that blocking did occur.
Verify that the output buffer was flushed.
Verify that predicate was called the correct number of times.
Verify that predicate returned True at most recent invocation.
>>CODE
XEvent	event;
int	callcnt;
Block_Info info;
int	block_status;
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
/* Call XPutBackEvent to put an event on the event queue. */
	event.type = KeyPress;
	XPutBackEvent(display, &event);
/* Set up predicate procedure. */
	PRED_SETUP(callcnt = 2);
/* Call XPeekIfEvent and verify that blocking did occur. */
	XPeekIfEvent_Type(info, predicate, arg);
	block_status = block(display, (XEvent *) NULL, &info);
	if (block_status == -1)
		return;
	else
		CHECK;
	if (block_status == 0) {
		report("Blocking did not occur.");
		FAIL;
	}
	else
		CHECK;
/* Verify that the output buffer was flushed. */
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
/* Verify that predicate was called the correct number of times. */
	if (_pred_cnt != callcnt) {
		report("predicate called %d times, expected %d", _pred_cnt, callcnt);
		FAIL;
	}
	else
		CHECK;
/* Verify that predicate returned True at most recent invocation. */
	if (_pred_retval != True) {
		report("predicate returned %d, expecting %d", _pred_retval, True);
		FAIL;
	}
	else
		CHECK;
	/* empty event queue */
	XSync(display, True);

	CHECKPASS(6);
