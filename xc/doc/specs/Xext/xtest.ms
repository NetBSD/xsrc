.\" Use -ms and macros.t
.\" $XConsortium: xtest.ms,v 1.9 94/04/17 20:06:14 rws Exp $
.de lP
.ne 8
.LP
..
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 10
.nr PS 10
\&
.sp 8
.ce 1
\s+2\fBX11 XTEST EXTENSION\fP\s-2
.sp 3
.ce 3
Version 2.2
X Consortium Standard
.sp 6
.ce 4
\s-1Kieron Drake
.sp 6p
UniSoft Ltd.\s+1
.bp
.sp 10
.ps 9
.nr PS 9
.sp 8
.lP
Copyright \(co 1992 by UniSoft Group Ltd.
.lP
Permission to use, copy, modify, and distribute this documentation for any
purpose and without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.  UniSoft makes no
representations about the suitability for any purpose of the information in
this document.  This documentation is provided ``as is'' without express or
implied warranty.
.lP
.sp 5
Copyright \(co 1992, 1994 X Consortium
.lP
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the ``Software''), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
.lP
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
.lP
THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
.lP
Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
.ps 10
.nr PS 10
.bp 1
.EH ''X11 XTEST Extension''
.OH ''X11 XTEST Extension''
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Overview
.lP
This extension is a minimal set of client and server extensions
required to completely test the X11 server with no user intervention.
.lP
This extension is not intended to support general journaling and
playback of user actions.  This is a difficult area [XTrap, 89] as it attempts
to synchronize synthetic user interactions with their effects; it is at the
higher level of dialogue recording/playback rather than at the strictly lexical
level.  We are only interested in the latter, simpler, case.  A more detailed
discussion and justification of the extension functionality is given in
[Drake, 91].
.lP
We are only aiming to provide a minimum set of facilities that
solve immediate testing and validation problems.  The testing extension
itself needs testing, where possible, and so should be as simple as possible.
.lP
We have also tried to:
.IP \(bu
Confine the extension to an appropriate, high, level within the server
to minimize portability problems.  In practice this means that the extension
should be at the DIX level, or use the DIX/DDX interface, or both.  This
has effects, in particular, on the level at which \*Qinput synthesis\*U
can occur.
.IP \(bu
Minimize the changes required in the rest of the server.
.IP \(bu
Minimize performance penalties on normal server operation.
.lP
.NH 1
Description
.lP
The functions provided by this extension fall into two groups:
.IP "\fBClient Operations\fP" 10
These routines manipulate otherwise hidden client-side behaviour.  The
actual implementation will depend on the details of the actual language
binding and what degree of request buffering, GContext caching etc., is
provided.  In the C binding, defined in section 7, routines are provided
to access the internals of two opaque data structures \*- \fIGC\fPs and
\fIVisual\fPs \*- and to discard any requests pending within the
output buffer of a connection.  The exact details can be expected to differ for
other language bindings.
.IP "\fBServer Requests\fP" 10
The first of these requests is similar to that provided in most
extensions: it allows a client to specify a major and minor version
number to the server and for the server to respond with major and minor
versions of its own.  The remaining two requests
.RS
.IP \fIa)\fP
allow access to an otherwise \*Qwrite-only\*U server resource: the cursor
associated with a given window; and
.IP \fIb)\fP
perhaps most importantly, allow limited synthesis of input device
events \*- almost as if a cooperative user had moved the pointing device
or pressed a key or button.
.RE
.lP
.NH 1
Types
.lP
The following types are used in the request and event definitions in
subsequent sections.
.lP
FAKE_EVENT_TYPE:
.Pn { KeyPress ,
.PN KeyRelease ,
.PN MotionNotify ,
.PN ButtonPress ,
.PN ButtonRelease }
.lP
FAKE_EVENT:
.PN [
type\^: FAKE_EVENT_TYPE;
detail\^: BYTE;
time\^: TIME;
root\^: WINDOW;
rootX\^: INT16;
rootY\^: INT16;
.PN ]
.lP
CurrentCursor
.PN "\*-\ 1"
.lP
.NH 1
Client Operations
.lP
These are abstract definitions of functionality.  They refer to client-side
objects like \*QGC\*U and \*QVISUAL\*U which are quoted to
denote their abstract nature.  Concrete versions of these functions are
only defined for particular language bindings.  In some circumstances
a particular language binding may not implement the relevant abstract
type or may provide it as a transparent, rather than opaque type, with
the result that the corresponding function does not make sense or is
not required, respectively.
.lP
.PN "XTestSetGContextOfGC"
.IP
\fIgc\fP\^: \*QGC\*U
.br
\fIgid\fP\^: GCONTEXT
.IP
Set the GCONTEXT within the \*QGC\*U \fIgc\fP to have the value specified by
\fIgid\fP.
.lP
.PN "XTestSetVisualIDOfVisual"
.IP
\fIvisual\fP\^: \*QVISUAL\*U
.br
\fIvisualid\fP\^: VISUALID
.IP
Set the VISUALID within the \*QVISUAL\*U \fIvisual\fP to have the value
specified by \fIvisualid\fP.
.lP
.PN "XTestDiscard"
.IP
\fIdpy\fP\^: \*QCONNECTION\*U
.lP
   =>
.IP
\fIstatus\fP\^: BOOL
.IP
Discard any requests that are present in the request buffer associated with
the \*QCONNECTION\*U \fIdpy\fP.
The \fIstatus\fP returned is
.PN True
if there were one or more requests
in the buffer and
.PN False
otherwise.
.lP
.NH 1
Server Requests
.lP
.PN "XTestGetVersion"
.IP
\fIclientMajorVersion\fP\^: CARD16
.br
\fIclientMinorVersion\fP\^: CARD16
.lP
   =>
.IP
\fIserverMajorVersion\fP\^: CARD16
.br
\fIserverMinorVersion\fP\^: CARD16
.IP
Errors:
.PN Length
.IP
This request can be used to ensure that the server version of the XTEST
extension is usable by the client.  This document defines major version two
(2), minor version one (1).
.lP
.PN "XTestCompareCursor"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIcursor\fP\^: CURSOR or CurrentCursor or None
.lP
   =>
.IP
\fIsame\fP\^: BOOL
.IP
Errors:
.PN Window ,
.PN Length ,
.PN Cursor
.IP
This request looks up the cursor associated with \fIwindow\fP and
compares it with
.RS
.IP \fIeither\fP
the null cursor if \fIcursor\fP is
.PN None ,
.IP \fIor\fP
the current cursor (i.e., that being displayed)
.IP \fIor\fP
the cursor whose ID is \fIcursor\fP,
.RE
.IP
returning the result of the comparison in \fIsame\fP.
.lP
.PN "XTestFakeInput"
.IP
\fIevents\fP\^: LISTofFAKE_EVENT
.IP
Errors:
.PN Window ,
.PN Length ,
.PN Alloc ,
.PN Value
.IP
This request simulates the limited set of core protocol
events within the set FAKE_EVENT_TYPE.  Only the following event fields,
defined in FAKE_EVENT, are interpreted:
.RS
.IP \fItype\fP
This must be one of
.PN KeyPress ,
.PN KeyRelease ,
.PN MotionNotify ,
.PN ButtonPress or
.PN ButtonRelease
or else a
.PN Value
error occurs.
.IP \fIdetail\fP
For key events, this field is interpreted as the physical keycode. 
If the keycode is less than min-keycode or greater than max-keycode,
as returned in the connection setup, then a
.PN Value
error occurs.
For button events, this field is interpreted as the physical (or core) button,
meaning it will be mapped to the corresponding logical button according to
the most recent
.PN SetPointerMapping
request.
If the button number is less than one or greater than the number of physical
buttons, then a
.PN Value
error occurs.
For motion events, if this field is
.PN True ,
then \fIrootX\fP and \fIrootY\fP
are relative distances from the current pointer location; if this field is
.PN False,
then they are absolute positions.
.IP \fItime\fP
This is either
.PN CurrentTime
(meaning no delay)
or the delay in milli-seconds that the server should wait before
simulating this event.  No other requests from this client will be
processed until this delay, if any, has expired and subsequent processing
of the simulated event has been completed.
.IP \fIroot\fP
In the case of motion events this field is the ID of the root window on
which the new motion is to take place.  If
.PN None
is specified, the root window of the screen the pointer is currently on
is used instead.
If this field is not a valid window then a
.PN Window
error occurs.
.IP "\fIrootX\fP & \fIrootY\fP"
In the case of motion events these fields indicate relative distance or
absolute pointer coordinates, according to the setting of \fIdetail\fP.
If the specified coordinates are off-screen, the closest on-screen
coordinates will be substituted.
.RE
.IP
When the simulated event(s) are processed they cause event propagation,
passive grab activation, etc., just as if the corresponding input device
action had occurred.  However, motion events might not be recorded in the
motion history buffer.
.IP
For the currently supported event types, the event list must have length one,
otherwise a
.PN BadLength
error occurs.
.lP
.PN "XTestGrabControl"
.IP
\fIimpervious\fP\^: BOOL
.IP
If \fIimpervious\fP is
.PN True ,
then the executing client becomes impervious to server grabs;
that is, it can continue executing requests even if another client
grabs the server.
If \fIimpervious\fP is
.PN False ,
then the executing client returns to the normal state of being
susceptible to server grabs.
.lP
.NH 1
Encoding
.lP
Please refer to the X11 Protocol Encoding document as this document uses
conventions established there.
.lP
The name of this extension is ``XTEST''.
.lP
.SH
\s+2New types\s-2
.lP
.Ds 0
.TA .75i 1.75i
.ta .75i 1.75i
.R
FAKE_EVENT_TYPE
	2	KeyPress
	3	KeyRelease
	4	ButtonPress
	5	ButtonRelease
	6	MotionNotify
.De
.lP
NOTE that the above values are defined to be the same as those for
the corresponding core protocol event types.
.lP
.SH
\s+2Requests\s-2
.lP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN XTestGetVersion
	1	CARD8		opcode
	1	0		xtest opcode
	2	2		request length
	1	CARD8		client major version
	1			unused
	2	CARD16		client minor version
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
 	1	1		Reply
	1	CARD8		server major version
	2	CARD16		sequence number
	4	0		reply length
	2	CARD16		server minor version
	22			unused
.De
.lP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN XTestCompareCursor
	1	CARD8		opcode
	1	1		xtest opcode
	2	3		request length
	4	WINDOW		window
	4	CURSOR		cursor
		0	\fINone\fP
		1	\fICurrentCursor\fP
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1	BOOL		cursors are the same
	2	CARD16		sequence number
	4	0		reply length
	24			unused
.De
.lP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN XTestFakeInput
	1	CARD8		opcode
	1	2		xtest opcode
	2	1+(1*8)		request length
	1	FAKE_EVENT_TYPE		fake device event type
	1	BYTE		detail: button or keycode
	2			unused
	4	TIME		delay (milli-seconds)
		0	\fICurrentTime\fP
	4	WINDOW		root window for \fIMotionNotify\fP
		0	\fINone\fP
	8			unused
	2	INT16		x position for \fIMotionNotify\fP
	2	INT16		y position for \fIMotionNotify\fP
	8			unused
.De
.lP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN XTestGrabControl
	1	CARD8		opcode
	1	3		xtest opcode
	2	2		request length
	1	BOOL		impervious
	3			unused
.De
.lP
.NH 1
C language Xlib Binding
.lP
The C routines either
provide direct access to the protocol and add no additional
semantics to those
defined in section 5 or they correspond directly to the abstract descriptions
of client operations in section 4.
.lP
All XTEST extension functions and procedures, and all manifest
constants and macros, will start with the string \*Q\fCXTest\fR\*U.
All operations are classified as
server/client (\fCServer\fR) or client-only (\fCClient\fR).
All routines that have return type \fCStatus\fR will return non-zero for
\*Qsuccess\*U and zero for \*Qfailure\*U.  Even if the XTEST extension is
supported the server may withdraw such facilities arbitrarily; in which case
they will subsequently return zero.
.lP
The include file for this extension is
.Pn < X11/extensions/XTest.h >.
.lP
Bool
.br
XTestQueryExtension (display, event_base, error_base, major_version, minor_version)
.RS
Display	*display;
.br
int *event_base; /* RETURN */
.br
int *error_base; /* RETURN */
.br
int *major_version; /* RETURN */
.br
int *minor_version; /* RETURN */
.RE
.IP
Returns
.PN True
if the specified display supports the XTEST extension else
.PN False .
If the extension is supported, *event_base would be set to the event number for
the first event for this extension and
*error_base would be set to the error number for the first error for
this extension.  As no errors or events are defined for this version of the extension,
the values returned here are not defined (nor useful).
If the extension is supported, *major_version and *minor_version are set to
the major and minor version numbers of the extension supported by the
display.  Otherwise none of the arguments are set.
.lP
Bool
.br
XTestCompareCursorWithWindow (display, window, cursor)
.RS
Display *display;
.br
Window window;
.br
Cursor cursor;
.RE
.IP
If the extension is supported, performs a comparison of the cursor
whose ID is \fIcursor\fP (which may be
.PN None )
with the cursor of the window \fIwindow\fP returning
.PN True
if they are the same and
.PN False
otherwise.  If the extension is not supported, then the request is ignored and
zero (0) is returned.
.lP
Bool
.br
XTestCompareCurrentCursorWithWindow (display, window)
.RS
Display *display;
.br
Window window;
.RE
.IP
If the extension is supported, performs a comparison of the current cursor
with the cursor of the window \fIwindow\fP returning
.PN True
if they are the same and
.PN False
otherwise.  If the extension is not supported, then the request is ignored and
zero (0) is returned.
.lP
XTestFakeKeyEvent (display, keycode, is_press, delay)
.RS
Display *display;
.br
unsigned int keycode;
.br
Bool is_press;
.br
unsigned long delay;
.RE
.IP
If the extension is supported, requests the server to simulate either
a
.PN KeyPress
(if \fIis_press\fP is
.PN True )
or a
.PN KeyRelease
(if \fIis_press\fP is
.PN False )
of the key with keycode \fIkeycode\fP,
otherwise the request is ignored.
.IP
If the extension is supported,
the simulated event will not be processed until \fIdelay\fP milli-seconds
after the request is received (if \fIdelay\fP is
.PN CurrentTime
then this is interpreted as no delay at all).  No other requests from
this client will be processed until this delay, if any, has expired
and subsequent processing of the simulated event has been completed.
.lP
XTestFakeButtonEvent (display, button, is_press, delay)
.RS
Display *display;
.br
unsigned int button;
.br
Bool is_press;
.br
unsigned long delay;
.RE
.IP
If the extension is supported, requests the server to simulate either
a
.PN ButtonPress
(if \fIis_press\fP is
.PN True )
or a
.PN ButtonRelease
(if \fIis_press\fP is
.PN False )
of the logical button numbered \fIbutton\fP,
otherwise the request is ignored.
.IP
If the extension is supported,
the simulated event will not be processed until \fIdelay\fP milli-seconds
after the request is received (if \fIdelay\fP is
.PN CurrentTime
then this is interpreted as no delay at all).  No other requests from
this client will be processed until this delay, if any, has expired
and subsequent processing of the simulated event has been completed.
.lP
XTestFakeMotionEvent (display, screen_number, x, y, delay)
.RS
Display *display;
.br
int screen_number;
.br
int x, y;
.br
unsigned long delay;
.RE
.IP
If the extension is supported, requests the server to simulate
a movement of the pointer to position (\fIx\fP, \fIy\fP) on the
root window of screen number \fIscreen_number\fP,
otherwise the request is ignored.  If \fIscreen_number\fP is -1, the
current screen (that the pointer is on) is used.
.IP
If the extension is supported,
the simulated event will not be processed until \fIdelay\fP milli-seconds
after the request is received (if \fIdelay\fP is
.PN CurrentTime
then this is interpreted as no delay at all).  No other requests from
this client will be processed until this delay, if any, has expired
and subsequent processing of the simulated event has been completed.
.lP
XTestFakeRelativeMotionEvent (display, screen_number, x, y, delay)
.RS
Display *display;
.br
int screen_number;
.br
int x, y;
.br
unsigned long delay;
.RE
.IP
If the extension is supported, requests the server to simulate
a movement of the pointer by offsets (\fIx\fP, \fIy\fP) relative
to the current pointer position on screen number \fIscreen_number\fP,
otherwise the request is ignored.  If \fIscreen_number\fP is -1, the
current screen (that the pointer is on) is used.
.IP
If the extension is supported,
the simulated event will not be processed until \fIdelay\fP milli-seconds
after the request is received (if \fIdelay\fP is
.PN CurrentTime
then this is interpreted as no delay at all).  No other requests from
this client will be processed until this delay, if any, has expired
and subsequent processing of the simulated event has been completed.
.lP
XTestGrabControl (display, impervious)
.RS
Display *display;
.br
Bool impervious;
.RE
.IP
If \fIimpervious\fP is
.PN True ,
then the executing client becomes impervious to server grabs.
If \fIimpervious\fP is
.PN False ,
then the executing client returns to the normal state of being
susceptible to server grabs.
.lP
Bool
.lP
XTestSetGContextOfGC (gc, gid)
.RS
GC gc;
.br
GContext gid;
.RE
.IP
Sets the GContext within the opaque datatype referenced by \fIgc\fP to
be that specified by \fIgid\fP.
.lP
XTestSetVisualIDOfVisual (visual, visualid)
.RS
Visual *visual;
.br
VisualID visualid;
.RE
.IP
Sets the VisualID within the opaque datatype referenced by \fIvisual\fP to
be that specified by \fIvisualid\fP.
.lP
Bool
.br
XTestDiscard (display)
.RS
Display *display;
.RE
.IP
Discard any requests within the output buffer for display \fIdisplay\fP,
returning
.PN True
if any requests were discarded, otherwise return
.PN False .
.NH 1
Bibliography
.XP
Annicchiarico, D., et al., \fIXTrap: The XTrap Architecture\fP\^
Digital Equipment Corporation, July 1991.
.XP
Drake, K.J., \fISome Proposals for a Minimum X11 Testing Extension\fP\^
UniSoft Ltd., June 1991.
.LP
