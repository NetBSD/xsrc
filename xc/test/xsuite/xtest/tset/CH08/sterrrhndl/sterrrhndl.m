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
 * $XConsortium: sterrrhndl.m,v 1.13 94/04/17 21:08:17 rws Exp $
 */
>>TITLE XSetErrorHandler CH08
int ((*)())()
XSetErrorHandler(handler)
int (*handler)();
>>EXTERN
#include <signal.h>
#include <sys/wait.h>
#include "Xproto.h"

#define	_xcall_(rvalue)		rvalue = XSetErrorHandler(handler)

static	int	counter = 0;
static	int	lastserial = 0;
static	int	lastrequest_code = 0;

static int
errorhandler(display, errorevent)
Display *display;
XErrorEvent *errorevent;
{
	lastserial = errorevent->serial;
	lastrequest_code = errorevent->request_code;
	return(counter++);
}

static int
_errorhandler(display, errorevent)
Display *display;
XErrorEvent *errorevent;
{
	return(counter--);
}

>>ASSERTION Good A
A call to xname
sets the error handler to
.A handler .
>>STRATEGY
Call XSetErrorHandler to set error handler to errorhandler.
Generate an error.
Verify that errorhandler was called.
>>CODE
int	oldcounter;
Window	w;
int	(*proc)();
Display	*display = Dsp;

/* Call XSetErrorHandler to set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Generate an error. */
	w = badwin(display);
	oldcounter = counter;
	XDestroyWindow(display, w);
	XSync(display, True);
/* Verify that errorhandler was called. */
	/* assume that if it was not called it was not set... */
	if (counter == oldcounter) {
		report("Error handler not set.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(1);
>>ASSERTION Good A
A call to xname
returns the previous error handler.
>>STRATEGY
Call XSetErrorHandler to set error handler to errorhandler.
Call XSetErrorHandler to set error handler to _errorhandler.
Verify that errorhandler was returned.
Call XSetErrorHandler to set error handler to errorhandler.
Verify that _errorhandler was returned.
>>CODE
int	(*proc)();

/* Call XSetErrorHandler to set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Call XSetErrorHandler to set error handler to _errorhandler. */
	handler = _errorhandler;
	_xcall_(proc);
/* Verify that errorhandler was returned. */
	if (proc != errorhandler) {
		report("Returned 0x%x, expected 0x%x", proc, errorhandler);
		FAIL;
	}
	else
		CHECK;
/* Call XSetErrorHandler to set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Verify that _errorhandler was returned. */
	if (proc != _errorhandler) {
		report("Returned 0x%x, expected 0x%x", proc, _errorhandler);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good B 1
>>#NOTE Don't know the default error handler...
A call to xname with
.A handler
set to
.S NULL
sets the error handler to the default error handler.
>>ASSERTION Good A
The default error handler prints a message and exits.
>>STRATEGY
Get default error handler.
Create child process.
Child calls default error handler and loops forever.
Parent sleeps for 10 seconds.
Parent verifies that child no longer exists.
>>EXTERN

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#define	MESSBUF	55

>>CODE
int	(*proc)();
pid_t	child;
int	stat_loc;
int	waitstatus;
Display	*display = Dsp;
int 	p[2];
int 	gotmessage = 0;
char	buf[MESSBUF];
FILE	*fp;

/* Get default error handler. */
	handler = (int (*)()) NULL;
	_xcall_(proc);
	/* requires two calls! */
	_xcall_(proc);

	if (pipe(p) == -1) {
		delete("Could not create pipe");
		return;
	}

/* Create child process. */
	child = fork();
	if (!child) {
		XErrorEvent	event;
		event.error_code = BadWindow;

		close(p[0]);
		/*
		 * Capture both stdout and stderr into the pipe.
		 */
		dup2(p[1], 1);
		dup2(p[1], 2);

/* Child calls default error handler and loops forever. */
		(*proc)(display, &event);

		/*
		 * Now close the pipe to make sure that the parent will not hang.
		 */
		close(p[1]);
		close(1);
		close(2);

		for (;;)
			continue;
		/*NOTREACHED*/
	}
	else
		CHECK;

	close(p[1]);

/* Parent sleeps for 10 seconds. */
	sleep(10);

	/*
	 * Read message in reasonable size chunks.
	 */
	fp = fdopen(p[0], "r");
	if (fp == NULL) {
		delete("Could not fdopen pipe");
		return;
	}
	trace("The message produced by the default handler:");
	gotmessage = 0;
	while (fgets(buf, MESSBUF-1, fp)) {
		gotmessage = 1;
		buf[MESSBUF-1] = '\0';
		trace("Message: %s", buf);
	}

	if (!gotmessage) {
		report("No message was printed");
		FAIL;
	} else
		CHECK;

/* Parent verifies that child no longer exists. */
	waitstatus = waitpid(child, &stat_loc, WNOHANG);
	if (waitstatus != child) {
		report("Child did not exit.");
		FAIL;
		(void) kill(child, SIGKILL);
		(void) waitpid(child, &stat_loc, WNOHANG);
	}
	else
		CHECK;
	CHECKPASS(3);
>>ASSERTION Good B 5
>>#NOTE Untestable, life is too short!
There is no limit to the number of times xname may be called.
>>STRATEGY
Set handler to errorhandler.
Call XSetErrorHandler 1000 times.
Report untested.
>>CODE
int	(*proc)();
int	i;

/* Set handler to errorhandler. */
	handler = errorhandler;
/* Call XSetErrorHandler 1000 times. */
	for (i=0; i<1000; i++) {
		if (i == 0)
			CHECK;
		proc = XCALL;
	}
/* Report untested. */
	CHECKUNTESTED(1);
>>ASSERTION Good A
When a
.S BadName
error occurs from a call to
.S XLoadQueryFont ,
.S XLookupColor ,
or
.S XAllocNamedColor ,
then
.A handler
is not called.
>>STRATEGY
Set error handler to errorhandler.
Generate a BadName error through a call to XLoadQueryFont.
Verify that XLoadQueryFont returned NULL.
Verify that errorhandler was not called.
Generate a BadName error through a call to XLookupColor.
Verify that XLookupColor returned 0.
Verify that errorhandler was not called.
Generate a BadName error through a call to XAllocNamedColor.
Verify that XAllocNamedColor returned 0.
Verify that errorhandler was not called.
>>CODE
int	(*proc)();
Display	*display = Dsp;
int	oldcounter;
int	status;
XFontStruct	*font_struct;
Colormap	colormap;
XColor	exact_def, screen_def;

/* Set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Generate a BadName error through a call to XLoadQueryFont. */
	oldcounter = counter;
	font_struct = XLoadQueryFont(display, config.bad_font_name);
	XSync(display, True);
/* Verify that XLoadQueryFont returned NULL. */
	if (font_struct != (XFontStruct *) NULL) {
		delete("XLoadQueryFont did not return NULL.");
		XFreeFont(display, font_struct);
		return;
	}
	else
		CHECK;
/* Verify that errorhandler was not called. */
	if (counter != oldcounter) {
		report("Error handler was called.");
		FAIL;
	}
	else
		CHECK;
/* Generate a BadName error through a call to XLookupColor. */
	oldcounter = counter;
	colormap = makecolmap(display, DefaultVisual(display, DefaultScreen(display)), AllocNone);
	status = XLookupColor(display, colormap, config.bad_colorname, &exact_def, &screen_def);
	XSync(display, True);
/* Verify that XLookupColor returned 0. */
	if (status != 0) {
		delete("XLookupColor returned %d, expected 0.", status);
		return;
	}
	else
		CHECK;
/* Verify that errorhandler was not called. */
	if (counter != oldcounter) {
		report("Error handler was called.");
		FAIL;
	}
	else
		CHECK;
/* Generate a BadName error through a call to XAllocNamedColor. */
	oldcounter = counter;
	/* re-use colormap from previous test. */
	status = XAllocNamedColor(display, colormap, config.bad_colorname, &exact_def, &screen_def);
	XSync(display, True);
/* Verify that XAllocNamedColor returned 0. */
	if (status != 0) {
		delete("XAllocNamedColor returned %d, expected 0.", status);
		return;
	}
	else
		CHECK;
/* Verify that errorhandler was not called. */
	if (counter != oldcounter) {
		report("Error handler was called.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(6);
>>ASSERTION Good A
When a
.S BadFont
error occurs from a
.S QueryFont
protocol request,
then
.A handler
is not called.
>>STRATEGY
Set error handler to errorhandler.
Create a bad font ID.
Call XQueryFont to generate a BadFont error.
Verify that XQueryFont returned NULL.
Verify that errorhandler was not called.
>>CODE
int	(*proc)();
Display	*display = Dsp;
int	oldcounter;
Font	font;
XFontStruct	*font_struct;

/* Set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Create a bad font ID. */
	font = badfont(display);
/* Call XQueryFont to generate a BadFont error. */
	oldcounter = counter;
	font_struct = XQueryFont(display, font);
	XSync(display, True);
/* Verify that XQueryFont returned NULL. */
	if (font_struct != (XFontStruct *) NULL) {
		delete("XQueryFont did not return NULL.");
		XFreeFont(display, font_struct);
		return;
	}
	else
		CHECK;
/* Verify that errorhandler was not called. */
	if (counter != oldcounter) {
		report("Error handler was called.");
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good A
On a call to
.A handler ,
the
.M serial
member in the
.M XErrorEvent
structure is set to the number that was the value of
.F NextRequest
immediately before the protocol request was sent.
>>STRATEGY
Call XSetErrorHandler to set error handler to errorhandler.
Create window.
Destroy window.
Call NextRequest to get the serial number to be used in the next request.
Call XDestroyWindow to generate a BadWindow error.
Verify that errorhandler was called.
Verify that the serial member in the XErrorEvent structure
was set correctly.
>>CODE
int	(*proc)();
Display	*display = Dsp;
int	oldcounter;
Window	w;
unsigned long	nextrequest;

/* Call XSetErrorHandler to set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Create window. */
	w = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 5, 5, 0, 0, 0);
/* Destroy window. */
	XDestroyWindow(display, w);
	XSync(display, True);
/* Call NextRequest to get the serial number to be used in the next request. */
	nextrequest = NextRequest(display);
/* Call XDestroyWindow to generate a BadWindow error. */
	oldcounter = counter;
	XDestroyWindow(display, w);
	XSync(display, True);
/* Verify that errorhandler was called. */
	if (counter == oldcounter) {
		delete("Error handler was not called.");
		return;
	}
	else
		CHECK;
/* Verify that the serial member in the XErrorEvent structure */
/* was set correctly. */
	if (nextrequest != lastserial) {
		report("Returned %d, expected %d", lastserial, nextrequest);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
>>ASSERTION Good A
On a call to
.A handler ,
the
.M request_code
member in the
.M XErrorEvent
structure is set to the protocol request of the procedure that failed.
>>STRATEGY
Call XSetErrorHandler to set error handler to errorhandler.
Create window.
Destroy window.
Call XDestroyWindow to generate a BadWindow error.
Verify that errorhandler was called.
Verify that the request_code member in the XErrorEvent structure
was set correctly.
>>CODE
int	(*proc)();
Display	*display = Dsp;
int	oldcounter;
Window	w;
unsigned long	nextrequest;

/* Call XSetErrorHandler to set error handler to errorhandler. */
	handler = errorhandler;
	_xcall_(proc);
/* Create window. */
	w = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 5, 5, 0, 0, 0);
/* Destroy window. */
	XDestroyWindow(display, w);
	XSync(display, True);
/* Call XDestroyWindow to generate a BadWindow error. */
	oldcounter = counter;
	/* make sure that it does not already equal X_DestroyWindow */
	lastrequest_code = X_DestroyWindow + 1;
	XDestroyWindow(display, w);
	XSync(display, True);
/* Verify that errorhandler was called. */
	if (counter == oldcounter) {
		delete("Error handler was not called.");
		return;
	}
	else
		CHECK;
/* Verify that the request_code member in the XErrorEvent structure */
/* was set correctly. */
	if (lastrequest_code != X_DestroyWindow) {
		report("Returned %d, expected %d", lastrequest_code, X_DestroyWindow);
		FAIL;
	}
	else
		CHECK;
	CHECKPASS(2);
