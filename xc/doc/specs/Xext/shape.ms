.\" Use -ms and macros.t
.\" $XConsortium: shape.ms /main/19 1996/02/07 17:25:09 swick $
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 10
.nr PS 10
\&
.sp 8
.ce 1
\s+2\fBX11 NONRECTANGULAR WINDOW SHAPE EXTENSION\fP\s-2
.sp 3
.ce 3
Version 1.0
X Consortium Standard
X Version 11, Release 6.1
.sp 6
.ce 4
\s-1Keith Packard
.sp 6p
MIT X Consortium
.ps 9
.nr PS 9
.sp 8
.LP
Copyright \(co 1989 X Consortium
.LP
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the ``Software''), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
.LP
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
.LP
THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
.LP
Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
.ps 10
.nr PS 10
.bp 1
.EH ''X11 Nonrectangular Window Shape Extension''
.OH ''X11 Nonrectangular Window Shape Extension''
.EF ''\fB % \fP''
.OF ''\fB % \fP''
.NH 1
Overview
.LP
This extension provides arbitrary window and border shapes within the X11
protocol.
.LP
The restriction of rectangular windows within the X protocol is a significant
limitation in the implementation of many styles of user interface.  For
example, many transient windows would like to display a ``drop shadow'' to
give the illusion of 3 dimensions.  As another example, some user interface
style guides call for buttons with rounded corners; the full simulation of a
nonrectangular shape, particularly with respect to event distribution and
cursor shape, is not possible within the core X protocol.  As a final
example, round clocks and nonrectangular icons are desirable visual addition
to the desktop.
.LP
This extension provides mechanisms for changing the visible shape of a
window to an arbitrary, possibly disjoint, nonrectangular form.  The intent
of the extension is to supplement the existing semantics, not replace them.
In particular, it is desirable for clients which are unaware of the
extension to still be able to cope reasonably with shaped windows.  For
example, window managers should still be able to negotiate screen
real-estate in rectangular pieces.  Toward this end, any shape specified for
a window is clipped by the bounding rectangle for the window as specified by
the window's geometry in the core protocol.  An expected convention would be
that client programs expand their shape to fill the area offered by the
window manager.
.NH 1
Description
.LP
Each window (even with no shapes specified) is defined by two regions:  the
\fBbounding region\fP and the \fBclip region\fP.  The bounding region is the area of the
parent window which the window will occupy (including border). The clip region
is the subset of the bounding region which is available for sub-windows and
graphics.  The area between the bounding region and the clip region is defined
to be the border of the window.
.LP
A non-shaped window will have a bounding region which is a rectangle
spanning the window including its border; the clip region will be a rectangle
filling the inside dimensions (not including the border).  In this document,
these areas are referred to as the \fBdefault bounding region\fP and the
\fBdefault clip region\fP.  For a window with inside size of \fIwidth\fP by
\fIheight\fP and border width \fIbwidth\fP, the default bounding and clip
regions are the rectangles (relative to the window origin):
.IP
bounding.x = -\fIbwidth\fP
.br
bounding.y = -\fIbwidth\fP
.br
bounding.width = \fIwidth\fP + 2 * \fIbwidth\fP
.br
bounding.height = \fIheight\fP + 2 * \fIbwidth\fP
.sp
clip.x = 0
.br
clip.y = 0
.br
clip.width = \fIwidth\fP
.br
clip.height = \fIheight\fP
.LP
This extension allows a client to modify either or both of the bounding or
clip regions by specifying new regions which combine with the default
regions.  These new regions are called the \fBclient bounding region\fP and
the \fBclient clip region\fP.  They are specified relative to the origin of
the window, and are always defined by offsets relative to the window origin
(that is, region adjustments are not required when the window is moved).
Three mechanisms for specifying regions are provided:  a list of rectangles,
a bitmap, and an existing bounding or clip region from a window.  This is
modeled on the specification of regions in graphics contexts in the core
protocol, and allows a variety of different uses of the extension.
.LP
When using an existing window shape as an operand in specifying a new shape,
the client region is used, unless none has been set in which case the
default region is used instead.
.LP
The \fBeffective bounding region\fP of a window is defined to be the intersection of
the client bounding region with the default bounding region.  Any portion of
the client bounding region which is not included in the default bounding
region will not be included in the effective bounding region on the screen.
This means that window managers (or other geometry managers) used to dealing
with rectangular client windows will be able to constrain the client to a
rectangular area of the screen.
.LP
Construction of the effective bounding region is dynamic; the client bounding
region is not mutated to obtain the effective bounding region.  If a client
bounding region is specified which extends beyond the current default bounding
region, and the window is later enlarged, the effective bounding region will
be enlarged to include more of the client bounding region.
.LP
The \fBeffective clip region\fP of a window is defined to be the intersection of the
client clip region with both the default clip region and the client bounding
region.  Any portion of the client clip region which is not included in both
the default clip region and the client bounding region will not be included in
the effective clip region on the screen.
.LP
Construction of the effective clip region is dynamic; the client clip region is
not mutated to obtain the effective clip region.  If a client clip region is
specified which extends beyond the current default clip region, and the
window or its bounding region is later enlarged, the effective clip region will
be enlarged to include more of the client clip region if it is included in
the effective bounding region.
.LP
The border of a window is defined to be the difference between the effective
bounding region and the effective clip region.  If this region is empty, no
border is displayed.  If this region is non-empty, the border is filled
using the border-tile or border-pixel of the window as specified in the core
protocol.  Note that a window with a nonzero border width will never be able
to draw beyond the default clip region of the window.  Also note that a zero
border width does not prevent a window from having a border, since the clip
shape can still be made smaller than the bounding shape.
.LP
All output to the window, and visible regions of any subwindows, will be
clipped to the effective clip region.  The server must not retain window
contents beyond the effective bounding region with backing store.  The window's
origin (for graphics operations, background tiling, and subwindow placement)
is not affected by the existence of a bounding region or clip region.
.LP 
Areas which are inside the default bounding region but outside the effective
bounding region are not part of the window; these areas of the screen will
be occupied by other windows.  Input events which occur within the default
bounding region but outside the effective bounding region will be delivered as
if the window was not occluding the event position.  Events which occur in
a nonrectangular border of a window will be delivered to that window, just
as for events which occur in a normal rectangular border.
.LP
An InputOnly window can have its bounding region set, but it is a
.PN Match
error to attempt to set a clip region on an InputOnly window, or to
specify its clip region as a source to a request in this extension.
.LP
The server must accept changes to the clip region of a root window, but
the server is permitted to ignore requested changes to the bounding region
of a root window.  If the server accepts bounding region changes, the contents
of the screen outside the bounding region are implementation dependent.
.NH 1
Types
.LP
The following types are used in the request and event definitions in
subsequent sections.
.LP
SHAPE_KIND:
.Pn { Bounding ,
.PN Clip }
.LP
SHAPE_OP:
.Pn { Set ,
.PN Union ,
.PN Intersect ,
.PN Subtract ,
.PN Invert }
.LP
.PN Set
means that the region specified as an explicit source in the request is stored
unaltered as the new destination client region.
.PN Union
means that the source and destination regions are unioned together to produce
the new destination client region.
.PN Intersect
means that the source and destination regions are intersected together to
produce the new destination client region.
.PN Subtract
means that the source region is subtracted from the destination region to
produce the new destination region.
.PN Invert
means that the destination region is subtracted from the source region to
produce the new destination region.
.NH 1
Requests
.LP
.PN "ShapeQueryVersion"
.LP
   =>
.IP
\fImajorVersion\fP\^: CARD16
.br
\fIminorVersion\fP\^: CARD16
.IP
This request can be used to ensure that the server version of the SHAPE
extension is usable by the client.  This document defines major version one
(1), minor version zero (0).
.LP
.PN "ShapeRectangles"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIop\fP\^: SHAPE_OP
.br
\fIxOff, yOff\fP\^: INT16
.br
\fIrectangles\fP\^: LISTofRECTANGLES
.br
\fIordering\fP\^: {UnSorted, YSorted, YXSorted, YXBanded}
.IP
Errors:
.PN Window ,
.PN Length ,
.PN Match ,
.PN Value
.IP
This request specifies an array of rectangles, relative to the origin of the
window plus the specified offset (\fIxOff\fP and \fIyOff\fP) which together
define a region.  This region is combined (as specified by the operator
\fIop\fP) with the existing client region (specified by \fIdestKind\fP) of the
destination window, and the result is stored as the specified client region of
the destination window.  Note that the list of rectangles can be empty,
specifying an empty region; this is not the same as passing
.PN None
to
.PN ShapeMask .
.IP
If known by the client, 
ordering relations on the rectangles can be specified with the ordering 
argument.
This may provide faster operation by the server.
The meanings of the ordering values are the same as in the core protocol
.PN SetClipRectangles
request.
If an incorrect ordering is specified, 
the server may generate a 
.PN Match 
error, but it is not required to do so.
If no error is generated,
the graphics results are undefined.
Except for
.PN UnSorted ,
the rectangles should be nonintersecting, or the resulting region will
be undefined.
.PN UnSorted 
means that the rectangles are in arbitrary order.
.PN YSorted 
means that the rectangles are nondecreasing in their Y origin.
.PN YXSorted 
additionally constrains 
.PN YSorted 
order in that all rectangles with an equal Y origin are
nondecreasing in their X origin.
.PN YXBanded 
additionally constrains 
.PN YXSorted 
by requiring that, for every possible Y scanline,
all rectangles that include that scanline have identical Y origins and Y
extents.
.LP
.PN "ShapeMask"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIop\fP\^: SHAPE_OP
.br
\fIxOff, yOff\fP\^: INT16
.br
\fIsource\fP\^: PIXMAP or None
.IP
Errors:
.PN Window ,
.PN Pixmap ,
.PN Match ,
.PN Value
.IP
The source in this request is a 1-bit deep pixmap, or
.PN None .
If \fIsource\fP is 
.PN None ,
the specified client region is removed from the window causing the effective
region to revert to the default region.  The ShapeNotify event generated by
this request and subsequent ShapeQueryExtents will report that a client
shape has not been specified.  If a valid pixmap is specified, it is converted
to a region, with bits set to one included in the region and bits set to
zero excluded, and an offset from the window origin as specified by
\fIxOff\fP and \fIyOff\fP.  The resulting region is then combined (as
specified by the operator \fIop\fP) with the existing client region
(indicated by \fIdestKind\fP) of the destination window, and the result is
stored as the specified client region of the destination window.  The source
pixmap and destination window must have been created on the same screen or else
a
.PN Match
error results.
.LP
.PN "ShapeCombine"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIop\fP\^: SHAPE_OP
.br
\fIxOff, yOff\fP\^: INT16
.br
\fIsource\fP\^: WINDOW
.br
\fIsourceKind\fP\^: SHAPE_KIND
.IP
Errors:
.PN Window ,
.PN Match ,
.PN Value
.IP
The client region, indicated by \fIsourceKind\fP, of the source window is
offset from the window origin by \fIxOff\fP and \fIyOff\fP and combined with
the client region, indicated by \fIdestKind\fP, of the destination window.
The result is stored as the specified client region of the destination
window.
The source and destination windows must be on the same screen or else a
.PN Match
error results.
.LP
.PN "ShapeOffset"
.IP
\fIdest\fP\^: WINDOW
.br
\fIdestKind\fP\^: SHAPE_KIND
.br
\fIxOff, yOff\fP\^: INT16
.IP
Errors:
.PN Window ,
.PN Match ,
.PN Value
.IP
The client region, indicated by \fIdestKind\fP, is moved relative to its
current position by the amounts \fIxOff\fP and \fIyOff\fP.
.LP
.PN "ShapeQueryExtents"
.IP
\fIdest\fP\^: WINDOW
.LP
   =>
.IP
\fIboundingShaped\fP\^: BOOL
.br
\fIclipShaped\fP\^: BOOL
.br
\fIxBoundingShape\fP\^: INT16
.br
\fIyBoundingShape\fP\^: INT16
.br
\fIwidthBoundingShape\fP\^: CARD16
.br
\fIheightBoundingShape\fP\^: CARD16
.br
\fIxClipShape\fP\^: INT16
.br
\fIyClipShape\fP\^: INT16
.br
\fIwidthClipShape\fP\^: CARD16
.br
\fIheightClipShape\fP\^: CARD16
.IP
Errors:
.PN Window
.IP
The \fIboundingShaped\fP and \fIclipShaped\fP results are 
.PN True
if the corresponding client regions have been specified, else they are
.PN False .
The x, y, width, and height values define the extents of the client regions,
when a client region has not been specified, the extents of the
corresponding default region are reported.
.LP
.PN "ShapeSelectInput"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIenable\fP\^: BOOL
.IP
Errors:
.PN Window ,
.PN Value
.IP
Specifying \fIenable\fP as
.PN True
causes the server to send the requesting client a
.PN ShapeNotify
event whenever the bounding or clip region of the specified window is
altered by any client.
Specifying \fIenable\fP as
.PN False
causes the server to stop sending such events.
.LP
.PN "ShapeInputSelected"
.IP
\fIwindow\fP\^: WINDOW
.LP
   =>
.IP
\fIenable\fP\^: BOOL
.IP
Errors:
.PN Window
.IP
If \fIenable\fP is
.PN True
then
.PN ShapeNotify
events for the window are generated for this client.
.LP
.PN "ShapeGetRectangles"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIkind\fP\^: SHAPE_KIND
.LP
   =>
.IP
\fIrectangles\fP: LISTofRECTANGLE
.br
\fIordering\fP\^: {UnSorted, YSorted, YXSorted, YXBanded}
.IP
Errors:
.PN Window,
.PN Match
.IP
A list of rectangles describing the region indicated by \fIkind\fP, and the
ordering of those rectangles, is returned.  The meaning of the \fIordering\fP
values is the same as in the
.PN ShapeRectangles
request.
.NH 1
Events
.LP
.PN "ShapeNotify"
.IP
\fIwindow\fP\^: WINDOW
.br
\fIkind\fP\^: SHAPE_KIND
.br
\fIshaped\fP\^: BOOL
.br
\fIx\fP, \fIy\fP\^: INT16
.br
\fIwidth\fP, \fIheight\fP\^: CARD16
.br
\fItime\fP\^: TIMESTAMP
.IP
Whenever the client bounding or clip shape of a window is modified, a
.PN ShapeNotify
event is sent to each client which has used
.PN ShapeSelectInput
to request it.
.IP
\fIkind\fP indicates which client region (bounding or clip) has been modified.
\fIshaped\fP is
.PN True
when the window has a client shape of type \fIkind\fP, and is
.PN False
when the window no longer has a client shape of this type.
\fIx\fP, \fIy\fP, \fIwidth\fP and \fIheight\fP indicate the extents of the
current shape.  When \fIshaped\fP is
.PN False
these will indicate the extents of the default region.  The timestamp
indicates the server time when the shape was changed.
.NH 1
Encoding
.LP
Please refer to the X11 Protocol Encoding document as this document uses
conventions established there.
.LP
The name of this extension is ``SHAPE''.
.LP
.SH
\s+2New types\s-2
.LP
.Ds 0
.TA .75i 1.75i
.ta .75i 1.75i
.R
SHAPE_KIND
	0	Bounding
	1	Clip
.De
.LP
.Ds 0
.TA .75i 1.75i
.ta .75i 1.75i
.R
SHAPE_OP
	0	Set
	1	Union
	2	Intersect
	3	Subtract
	4	Invert
.De
.SH
\s+2Requests\s-2
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeQueryVersion
	1	CARD8		opcode
	1	0		shape opcode
	2	1		request length
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
 	1	1		Reply
	1			unused
	2	CARD16		sequence number
	4	0		length
	2	CARD16		major version
	2	CARD16		minor version
	20			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeRectangles
	1	CARD8		opcode
	1	1		shape opcode
	2	4+2n		request length
	1	SHAPE_OP		operation
	1	SHAPE_KIND		destination kind
	1			ordering
		0	UnSorted
		1	YSorted
		2	YXSorted
		3	YXBanded
	1			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
	8n	LISTofRECTANGLE		rectangles
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeMask
	1	CARD8		opcode
	1	2		shape opcode
	2	5		request length
	1	SHAPE_OP		operation
	1	SHAPE_KIND		destination kind
	2			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
	4	PIXMAP		source bitmap
		0	None
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeCombine
	1	CARD8		opcode
	1	3		shape opcode
	2	5		request length
	1	SHAPE_OP		operation
	1	SHAPE_KIND		destination kind
	1	SHAPE_KIND		source kind
	1			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
	4	WINDOW		source window
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeOffset
	1	CARD8		opcode
	1	4		shape opcode
	2	4		request length
	1	SHAPE_KIND		destination kind
	3			unused
	4	WINDOW		destination window
	2	INT16		x offset
	2	INT16		y offset
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeQueryExtents
	1	CARD8		opcode
	1	5		shape opcode
	2	2		request length
	4	WINDOW		destination window
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1			unused
	2	CARD16		sequence number
	4	0		reply length
	1	BOOL		bounding shaped
	1	BOOL		clip shaped
	2			unused
	2	INT16		bounding shape extents x
	2	INT16		bounding shape extents y
	2	CARD16		bounding shape extents width
	2	CARD16		bounding shape extents height
	2	INT16		clip shape extents x
	2	INT16		clip shape extents y
	2	CARD16		clip shape extents width
	2	CARD16		clip shape extents height
	4			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeSelectInput
	1	CARD8		opcode
	1	6		shape opcode
	2	3		request length
	4	WINDOW		destination window
	1	BOOL		enable
	3			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeInputSelected
	1	CARD8		opcode
	1	6		shape opcode
	2	2		request length
	4	WINDOW		destination window
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1	BOOL		enabled
	2	CARD16		sequence number
	4	0		reply length
	24			unused
.De
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeGetRectangles
	1	CARD8		opcode
	1	7		shape opcode
	2	3		request length
	4	WINDOW		window
	1	SHAPE_KIND		source kind
	3			unused
.De
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
 =>
	1	1		Reply
	1			ordering
		0	UnSorted
		1	YSorted
		2	YXSorted
		3	YXBanded
	2	CARD16		sequence number
	4	2n		reply length
	4	CARD32		nrects
	20			unused
	8n	LISTofRECTANGLE		rectangles
.De
.SH
\s+2Events\s-2
.LP
.LP
.Ds 0
.TA .2i .5i 1.5i 2.5i
.ta .2i .5i 1.5i 2.5i
.R
.PN ShapeNotify
	1	CARD8		type (0 + extension event base)
	1	SHAPE_KIND		shape kind
	2	CARD16		sequence number
	4	WINDOW		affected window
	2	INT16		x value of extents
	2	INT16		y value of extents
	2	CARD16		width of extents
	2	CARD16		height of extents
	4	TIMESTAMP		server time
	1	BOOL		shaped
	11			unused
.De
.NH 1
C Language Binding
.LP
The C routines provide direct access to the protocol and add no additional
semantics.
.LP
The include file for this extension is
.Pn < X11/extensions/shape.h >.
The defined shape kinds are
.PN ShapeBounding
and
.PN ShapeClip .
The defined region operations are
.PN ShapeSet ,
.PN ShapeUnion ,
.PN ShapeIntersect ,
.PN ShapeSubtract ,
and
.PN ShapeInvert .
.LP
Bool
.br
XShapeQueryExtension (display, event_base, error_base)
.RS
Display	*display;
.br
int *event_base; /* RETURN */
.br
int *error_base; /* RETURN */
.RE
.IP
Returns
.PN True
if the specified display supports the SHAPE extension else
.PN False .
If the extension is supported, *event_base is set to the event number for
.PN ShapeNotify
events and *error_base would be set to the error number for the first error for
this extension.  As no errors are defined for this version of the extension,
the value returned here is not defined (nor useful).
.LP
Status
.br
XShapeQueryVersion (display, major_version, minor_version)
.RS
Display *display;
.br
int *major_version, *minor_version;  /* RETURN */
.RE
.IP
If the extension is supported, the major and minor version numbers of the
extension supported by the display are set and a non-zero value is returned.
Otherwise the arguments are not set and 0 is returned.
.LP
XShapeCombineRegion (display, dest, dest_kind, x_off, y_off, region, op)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, op, x_off, y_off;
.br
REGION *region;
.RE
.IP
Converts the specified region into a list of rectangles and calls
.PN XShapeCombineRectangles .
.LP
XShapeCombineRectangles (display, dest, dest_kind, x_off, y_off, rectangles, n_rects, op, ordering)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, n_rects, op, x_off, y_off, ordering;
.br
XRectangle *rectangles;
.RE
.IP
If the extension is supported, performs a ShapeRectangles operation;
otherwise, the request is ignored.
.LP
XShapeCombineMask (display, dest, dest_kind, x_off, y_off, src, op)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, op, x_off, y_off;
.br
Pixmap	src;
.RE
.IP
If the extension is supported, performs a ShapeMask operation;
otherwise, the request is ignored.
.LP
XShapeCombineShape (display, dest, dest_kind, x_off, y_off, src, src_kind, op)
.RS
Display *display;
.br
Window dest, src;
.br
int dest_kind, src_kind, op, x_off, y_off;
.RE
.IP
If the extension is supported, performs a ShapeCombine operation;
otherwise, the request is ignored.
.LP
XShapeOffsetShape (display, dest, dest_kind, x_off, y_off)
.RS
Display *display;
.br
Window dest;
.br
int dest_kind, x_off, y_off;
.RE
.IP
If the extension is supported, performs an ShapeOffset operation;
otherwise, the request is ignored.
.LP
Status XShapeQueryExtents
'in +1i
(display, window, bounding_shaped, x_bounding, y_bounding, w_bounding, h_bounding, clip_shaped, x_clip, y_clip, w_clip, h_clip)
'in 0
.RS
Display *display;
.br
Window window;
.br
Bool *bounding_shaped, *clip_shaped; /* RETURN */
.br
int *x_bounding, *y_bounding, *x_clip, *y_clip; /* RETURN */
.br
unsigned int *w_bounding, *h_bounding, *w_clip, *h_clip; /* RETURN */
.RE
.IP
If the extension is supported,
x_bounding, y_bounding, w_bounding, h_bounding are set to the extents of the
bounding shape, and x_clip, y_clip, w_clip, h_clip are set to the extents of
the clip shape.  For unspecified client regions, the extents of the
corresponding default region are used.
.IP
If the extension is supported a non-zero value is returned; otherwise, 0
is returned.
.LP
XShapeSelectInput (display, window, mask)
.RS
Display *display;
.br
Window window;
.br
unsigned long mask;
.RE
.IP
To make this extension more compatible with other interfaces, although
only one event type can be selected via the extension, this C interface
provides a general mechanism similar to the standard Xlib binding for
window events.  A mask value has been defined,
.PN ShapeNotifyMask ,
which is the only valid bit in \fImask\fP which may be specified.
The structure for this event is defined as follows:
.IP
.TA 2.5i
.ta 2.5i
typedef struct {
    int type;	/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;	/* true if this came frome a SendEvent request */
    Display *display;	/* Display the event was read from */
    Window window;	/* window of event */
    int kind;	/* ShapeBounding or ShapeClip */
    int x, y;	/* extents of new region */
    unsigned width, height;
    Time time;	/* server timestamp when region changed */
    Bool shaped;	/* true if the region exists */
.br
} XShapeEvent;
.LP
unsigned long
.br
XShapeInputSelected (display, window)
.RS
Display *display
.br
Window window;
.RE
.IP
This returns the current input mask for extension events on the specified
window; the value returned if
.PN ShapeNotify
is selected for is
.PN ShapeNotifyMask ,
otherwise it returns zero.
.IP
If the extension is not supported, 0 is returned.
.LP
XRectangle *
.br
XShapeGetRectangles (display, window, kind, count, ordering)
.RS
Display *display;
.br
Window window;
.br
int kind;
.br
int *count; /* RETURN */
.br
int *ordering; /* RETURN */
.RE
.LP
.IP
If the extension is not supported, NULL is returned.  Otherwise,
a list of rectangles describing the region specified by \fIkind\fP
is returned.
.NH 1
Glossary
.LP
.IP "\fBbounding region\fP"
The area of the parent window which this window will occupy.  This area is
divided into two parts:  the border and the interior.
.IP "\fBclip region\fP"
The interior of the window, as a subset of the \fBbounding region\fP.  This
region describes the area which will be painted with the window background
when the window is cleared, will contain all graphics output to the window,
and will clip any subwindows.
.IP "\fBdefault bounding region\fP"
The rectangular area, as described by the core protocol window size, which
covers the interior of the window and its border.
.IP "\fBdefault clip region\fP"
The rectangular area as described by the core protocol window size which
covers the interior of the window and excludes the border.
.IP "\fBclient bounding region\fP"
The region associated with a window which is directly modified via this
extension when specified by \fIShapeBounding\fP.  This region is used in
conjunction with the \fBdefault bounding region\fP to produce the
\fBeffective bounding region\fP.
.IP "\fBclient clip region\fP"
The region associated with a window which is directly modified via this
extension when specified by \fIShapeClip\fP.  This region is used in
conjunction with the \fBdefault clip region\fP and the \fBclient bounding
region\fP to produce the \fBeffective clip region\fP.
.IP "\fBeffective bounding region\fP"
The actual shape of the window on the screen, including border and interior
(but excluding the effects of overlapping windows).  When a window has a client
bounding region, the effective bounding region is the intersection of the
default bounding region and the client bounding region.  Otherwise, the
effective bounding region is the same as the default bounding region.
.IP "\fBeffective clip region\fP"
The actual shape of the interior of the window on the screen (excluding the
effects of overlapping windows).  When a window has a client clip region or
a client bounding region, the effective clip region is the intersection of
the default clip region, the client clip region (if any) and the client
bounding region (if any).  Otherwise, the effective clip region is the
same as the default clip region.
