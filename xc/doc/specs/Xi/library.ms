.\" $XConsortium: library.ms /main/16 1996/02/07 19:28:03 swick $
.\" Input Extension
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ps 11
.nr PS 11
\0
.sp 10
.ce 500
.ps 20
\fBX11 Input Extension Library Specification
.ps 12
.sp 2
X Consortium Standard
.sp 1
X Version 11, Release 6.1
.sp 16
.ps 15
\fBMark Patrick\0\0\0\0Ardent Computer
.sp 1
\fBGeorge Sachs\0\0\0\0Hewlett-Packard
.ps 12
.ce 0
.bp
\0
.sp 10
.fi
.LP
Copyright \(co 1989, 1990, 1991 by Hewlett-Packard Company, Ardent Computer. 
.LP
Permission to use, copy, modify, and distribute this documentation for
any purpose and without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.
Ardent, and Hewlett-Packard make no representations about the suitability 
for any purpose of the information in this document.  It is provided \`\`as is''
without express or implied warranty.
.sp 5
Copyright (c) 1989, 1990, 1991, 1992  X Consortium
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
.sp 3
\fIX Window System\fP is a trademark of X Consortium, Inc.
.ps
.vs
.bp 1
.EH '\fBX Input Extension Library Specification\fP''\fBX11, Release 6.1\fP'
.OH '\fBX Input Extension Library Specification\fP''\fBX11, Release 6.1\fP'
.EF ''\fB\\\\n(PN\fP''
.OF ''\fB\\\\n(PN\fP''
.\"  Force the heading counter for level 1 to one
.\"
.\"
.\"
.\"  Print table of contents to level 4 headings
.\"
.\"
.\"  Page eject for each level 1 heading
.\"
.\"
.\"  Define Ch to contain the chapter string.
.\"
.ds Ch Input Extension Overview
.\"
.\"
.\"  Pull in the layout macro package.
.\"
.\"
.tr ~
.NH 1
Input Extension Overview
.XS
\*(SN Input Extension Overview
.XE
.LP
This document describes an extension to
the X11 server.  The purpose of this extension is to support the use
of additional input devices beyond the pointer and keyboard devices
defined by the core X protocol.  This first section gives an overview
of the input extension.  The following sections correspond to 
chapters 7 and 8, "Window Manager functions" and 
"Events and Event-Handling Functions" of the "Xlib - C Language Interface"
manual and describe how to use the input extension.
.NH 2
Design Approach
.XS
\*(SN Design Approach
.XE
.LP
The design approach of the extension is to define functions
and events analogous to the core functions and events. This allows
extension input devices and events to be individually distinguishable from each other 
and from the core input devices and events .  These functions and events make use
of a device identifier and support the
reporting of n-dimensional motion data as well as other data that
is not currently reportable via the core input events.
.NH 2
Core Input Devices
.XS
\*(SN Core Input Devices
.XE
.LP
The X server core protocol supports two input devices:  a pointer and a
keyboard.  The pointer device has two major functions. 
First, it may be used to generate motion information
that client programs can detect. Second, it may also be used to indicate the
current location and focus of the X keyboard.  To accomplish this, the server 
echoes a cursor at the current position of the X pointer.  Unless the X
keyboard has been explicitly focused, this cursor also shows the current
location and focus of the X keyboard.
.LP
The X keyboard is used to generate input that client programs can detect.
.LP
The X keyboard and X pointer are referred to in this document as 
the \fIcore devices\fP, and the input
events they generate (\fBKeyPress\fP, \fBKeyRelease\fP, \fBButtonPress\fP, 
\fBButtonRelease\fP, and
\fBMotionNotify\fP) are known as the \fIcore input events\fP.  All other
input devices are referred to as \fIextension input devices\fP and the 
input events they generate are referred to as \fIextension input events\fP.
.NT
This input extension does not change the behavior or functionality of the
core input devices, core events, or core protocol requests, with the
exception of the core grab requests.  These requests may affect the
synchronization of events from extension devices.  See the explanation
in the section titled "Event Synchronization and Core Grabs".
.NE
.LP
Selection of the physical devices to be initially used by the server as the 
core devices is left implementation-dependent.  Functions are defined that
allow client programs to change which physical devices are used as the
core devices.
.NH 2
Extension Input Devices
.XS
\*(SN Extension Input Devices
.XE
.LP
The input extension controls access to input devices other than the X keyboard
and X pointer.  It allows client programs to select input from these devices 
independently
from each other and independently from the core devices.  Input events from
these devices are of extension types (\fBDeviceKeyPress\fP, 
\fBDeviceKeyRelease\fP,
\fBDeviceButtonPress\fP, \fBDeviceButtonRelease\fP, \fBDeviceMotionNotify\fP, 
etc.) and contain
a device identifier so that events of the same type coming from different
input devices can be distinguished.
.LP
Extension input events are not limited in size by the size of the server
32-byte wire events.  Extension input events
may be constructed by the server sending as many
wire sized events as necessary to return the information required for
that event.
The library event reformatting routines
are responsible for combining these into one or more client XEvents.
.LP
Any input device that generates key, button or motion data may be used as
an extension input device.
Extension input devices may have 0 or more keys, 0 or more buttons,
and may report 0 or more axes of motion.  Motion may be reported 
as relative movements from a previous position or as an absolute
position.  All valuators reporting motion information for a given
extension input device must report the same kind of motion information
(absolute or relative).
.LP
This extension is designed to accommodate new types of input devices that
may be added in the future.  The protocol requests that refer to
specific characteristics of input devices organize that information
by \fBinput device classes\fP.  Server implementors may add new
classes of input devices without changing the protocol requests.
.LP
All extension input
devices are treated like the core X keyboard in determining their location
and focus.  The server does not track the location of these devices on an
individual basis, and therefore
does not echo a cursor to indicate their current location.
Instead, their location is determined by the location of the core X pointer.
Like the core X keyboard, some may be explicitly focused. If they are
not explicitly focused,  their focus
is determined by the location of the core X pointer.
.NH 3
Input Device Classes
.XS
\*(SN Input Device Classes
.XE
.LP
Some of the input extension requests divide input devices into classes
based on their functionality.  This is intended to allow new classes of input
devices to be defined at a later time without changing the semantics of 
these functions.  The following input device classes are currently
defined:
.RS
.in +10n
.IP "\fBKEY\fP"
The device reports key events.
.IP "\fBBUTTON\fP"
The device reports button events.
.IP "\fBVALUATOR\fP"
The device reports valuator data in motion events.
.IP "\fBPROXIMITY\fP"
The device reports proximity events.
.IP "\fBFOCUS\fP"
The device can be focused.
.IP "\fBFEEDBACK\fP"
The device supports feedbacks.
.in -10n
.RE
.LP
Additional classes may be added in the future.
Functions that support multiple input classes, such as the 
\fBXListInputDevices\fP function that lists all available input devices,
organize the data they return by input class.  Client programs that
use these functions should not access data unless it matches a 
class defined at the time those clients were compiled.  In this way,
new classes can be added without forcing existing clients that use
these functions to be recompiled.
.NH 2
Using Extension Input Devices
.XS
\*(SN Using Extension Input Devices
.XE
.LP
A client that wishes to access an input device does so through the library
functions defined in the following sections.  A typical sequence of requests
that a client would make is as follows:
.IP \(bu 3n
XListInputDevices - list all of the available input devices.  From the 
information returned by this request, determine whether the desired input
device is attached to the server.  For a description of the 
\fBXListInputDevices\fP request, see the section entitled
"Listing Available Devices".
.IP \(bu 3n
XOpenDevice -  request that the server open the device for access by this
client.  This request returns an \fBXDevice\fP structure that is used
by most other input extension requests to identify the specified device.
For a description of the \fBXOpenDevice\fP request, see the section
entitled "Enabling and Disabling Extension Devices".
.IP \(bu 3n
Determine the event types and event clases needed to select the desired
input extension events, and identify them when they are received.
This is done via macros whose name corresponds to the desired event,
i.e. \fBDeviceKeyPress\fP.  For a description of these macros, see the 
section entitled "Selecting Extension Device Events".
.IP \(bu 3n
XSelectExtensionEvent - select the desired events from the server.
For a description of the \fBXSelextExtensionEvent\fP request, see the 
section entitled "Selecting Extension Device Events".
.IP \(bu 3n
XNextEvent - receive the next available event.  This is the core
\fBXNextEvent\fP function provided by the standard X libarary.
.LP
Other requests are defined to grab and focus extension devices, to
change their key, button, or modifier mappings, to control the
propagation of input extension events, to get motion history from an
extension device, and to send input extension events to another client.
These functions are described in the following sections.
.NH 1
Library Extension Requests
.XS
\*(SN Library Extension Requests
.XE
.LP
Extension input devices are accessed by client programs through the 
use of new protocol requests.
The following requests are provided as extensions to Xlib.  Constants
and structures referenced by these functions may be found in the 
files \fBXI.h\fP and \fBXInput.h\fP, which are attached to this document as 
appendix A.
.LP
The library will return \fBNoSuchExtension\fP if an extension request
is made to a server that does not support the input extension.
.LP
Input extension requests cannot be used to access the X keyboard and
X pointer devices.
.NH 2
Window Manager Functions
.XS
\*(SN Window Manager Functions
.XE
.NH 3
Changing The Core Devices
.XS
\*(SN Changing The Core Devices
.XE
.LP
These functions are provided to change which physical device is used
as the X pointer or X keyboard.
.NT
Using these functions may change the characteristics of the core devices.
The new pointer device may have a different number of buttons than the 
old one did, or the new keyboard device may have a different number of
keys or report a different range of keycodes.  Client programs may be
running that depend on those characteristics.  For example, a client
program could allocate an array based on the number of buttons on the
pointer device, and then use the button numbers received in button events
as indicies into that array.  Changing the core devices could cause
such client programs to behave improperly or abnormally terminate,
if they ignore the ChangeDeviceNotify event generated by these requests.
.NE
.LP
These functions change the X keyboard or X pointer device and generate
an \fBXChangeDeviceNotify\fP event and a \fBMappingNotify\fP event.  
The specified device becomes the
new X keyboard or X pointer device.  The location of the core device
does not change as a result of this request.
.LP
These requests fail and return \fBAlreadyGrabbed\fP if either the specified
device or the core device it would replace are grabbed by some other
client.  They fail and return \fBGrabFrozen\fP if either device is frozen
by the active grab of another client.
.LP
These requests fail with a \fBBadDevice\fP error if the specified device is
invalid, has not previously been opened via \fBXOpenDevice\fP, or is
not supported as a core device by the server implementation.
.sp 2
.LP
Once the device has successfully replaced one of the core devices, it
is treated as a core device until it is in turn replaced by another
\fBChangeDevice\fP request, or until the server terminates.  The termination
of the client that changed  the device will not cause it to change back.
Attempts to use the \fBXCloseDevice\fP request to close the new core device will
fail with a \fBBadDevice\fP error.
.LP
To change which physical device is used as the X keyboard, use
the \fBXChangeKeyboardDevice\fP function.
.LP
The specified device must support input class \fBKeys\fP (as reported in the
\fBListInputDevices\fP request) or the request will fail with a 
\fBBadMatch\fP error.
.LP
.DS
\fC
int
XChangeKeyboardDevice (display, device)
    Display *display;
    XDevice *device;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.in -.5i
.RE
.LP
If no error occurs, this function returns \fBSuccess\fP.
A \fBChangeDeviceNotify\fP event with the request field set to \fBNewKeyboard\fP
is sent to all clients selecting that event.  A \fBMappingNotify\fP event
with the request field set to \fBMappingKeyboard\fP is sent to all clients.
The requested device becomes the X keyboard, and the old keyboard becomes 
available as an extension input device.
The focus state of the new keyboard is the same as
the focus state of the old X keyboard.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadMatch\fP,
\fBAlreadyGrabbed\fP, and \fBGrabFrozen\fP.
.sp 2
.LP
To change which physical device is used as the X pointer,
use the \fBXChangePointerDevice\fP function.
The specified device must support input class \fBValuators\fP (as reported in 
the \fBXListInputDevices request\fP) and report at least two axes of motion,
or the request will fail with a \fBBadMatch\fP error.
If the specified device reports more than two axes, the two specified in
the xaxis and yaxis arguments will be used.  Data from other
valuators on the device will be ignored.
.LP
If the specified device reports absolute positional information, and the 
server implementation does not allow such a device to be used as the 
X pointer, the request will fail with a \fBBadDevice\fP error.
.DS
\fC
int
XChangePointerDevice (display, device, xaxis, yaxis)
    Display *display;
    XDevice *device;
    int     xaxis;
    int     yaxis;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIxaxis\fP"
Specifies the zero-based index of the axis to be used as the x-axis of the 
pointer device.
.IP "\fIyaxis\fP"
Specifies the zero-based index of the axis to be used as the y-axis of the 
pointer device.
.in -.5i
.RE
.LP
If no error occurs, this function returns \fBSuccess\fP.
A \fBChangeDeviceNotify\fP event with the request field set to \fBNewPointer\fP 
is sent to all clients selecting that event.  A \fBMappingNotify\fP event
with the request field set to \fBMappingPointer\fP is sent to all clients.
The requested device becomes the X pointer, and the old pointer becomes 
available as an extension input device.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadMatch\fP,
\fBAlreadyGrabbed\fP, and \fBGrabFrozen\fP.
.NH 3
Event Synchronization And Core Grabs
.XS
\*(SN Event Synchronization And Core Grabs
.XE
.LP
Implementation of the input extension requires an extension of the
meaning of event synchronization for the core grab requests.  This is
necessary in order to allow window managers to freeze all input devices
with a single request.
.LP
The core grab requests require a \fBpointer_mode\fP and \fBkeyboard_mode\fP
argument.  The meaning of these modes is changed by the input extension.
For the \fBXGrabPointer\fP and \fBXGrabButton\fP requests, \fBpointer_mode\fP
controls synchronization of the pointer device, and \fBkeyboard_mode\fP
controls the synchronization of all other input devices.  
For the \fBXGrabKeyboard\fP
and \fBXGrabKey\fP requests, \fBpointer_mode\fP controls the synchronization
of all input devices except the X keyboard, while \fBkeyboard_mode\fP controls
the synchronization of the keyboard.  When using one of the core grab
requests, the synchronization of extension devices
is controlled by the mode specified for the device not being grabbed.
.NH 3
Extension Active Grabs
.XS
\*(SN Extension Active Grabs
.XE
.LP
Active grabs of
extension devices are supported via the 
\fBXGrabDevice\fP
function in the same way that core devices are grabbed using
the core \fBXGrabKeyboard\fP function, except that a \fIDevice\fP is passed as
a function parameter.  
The \fBXUngrabDevice\fP function allows a
previous active grab for an extension device to be released.
.LP
Passive grabs of buttons and keys on extension devices are supported
via the \fBXGrabDeviceButton\fP and \fBXGrabDeviceKey\fP functions.
These passive grabs are released via the \fBXUngrabDeviceKey\fP and
\fBXUngrabDeviceButton\fP functions.
.sp 2
To grab an extension device, use the \fBXGrabDevice\fP function.
The device must have previously been opened using the
\fBXOpenDevice\fP function.
.DS
\fC
int
XGrabDevice (display, device, grab_window, owner_events, 
    event_count, event_list, this_device_mode, 
    other_device_mode, time)
        Display     *display;
        XDevice     *device;
        Window      grab_window;
        Bool        owner_events;
        int         event_count;
        XEventClass *event_list;
        int         this_device_mode;
        int         other_device_mode;
        Time        time;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIgrab_window\fP"
Specifies the ID of a window associated with the device specified above.
.IP "\fIowner_events\fR"
\fRSpecifies a boolean value of either \fBTrue\fP or \fBFalse.
.IP "\fIevent_count\fP"
Specifies the number of elements in the event_list array.
.IP "\fIevent_list\fP"
Specifies a pointer to a list of event classes that indicate which events
the client wishes to receive.  
These event classes must have been obtained
using the device being grabbed.
.IP "\fIthis_device_mode\fP"
Controls further processing of events from this device.  You can pass one
of these constants: \fBGrabModeSync\fP or \fBGrabModeAsync\fP.
.IP "\fIother_device_mode\fP"
Controls further processing of events from all other devices.  You can pass one
of these constants: \fBGrabModeSync\fP or \fBGrabModeAsync\fP.
.IP "\fItime\fP"
Specifies the time.  This may be either a timestamp expressed in
milliseconds, or \fBCurrentTime\fP.
.in -.5i
.RE
.LP
The \fBXGrabDevice\fP function actively grabs an 
extension input device, and generates 
\fBDeviceFocusIn\fP and \fBDeviceFocusOut\fP events.  Further input events from
this device are reported only to the grabbing client.  This function
overrides any previous active grab by this client for this 
device.  
.LP
The event-list parameter is a pointer to a list of event classes.  This list
indicates which events the client wishes to receive while the grab is active.
If owner_events
is \fBFalse\fP, input events from this device are reported with respect to 
grab_window and are only reported if specified in event_list.
If owner_events is \fBTrue\fP, then if a generated event would
normally be reported to this client, it is reported normally. Otherwise
the event is reported with respect to the grab_window, and is only
reported if specified in event_list.
.LP
The this_device_mode argument controls the further processing 
of events from this device, and the other_device_mode argument controls
the further processing of input events from all other devices.
.IP \(bu 3n
If the this_device_mode argument is 
\fBGrabModeAsync\fP,
device event processing continues
normally; if the device is currently frozen by this client, then
processing of device events is resumed.
If the this_device_mode  argument is
\fBGrabModeSync\fP,
the state of the grabbed device
(as seen by client applications) appears to freeze,
and no further device events are generated by the server until the
grabbing client issues a releasing 
\fBXAllowDeviceEvents\fP
call or until the device grab is released.
Actual
device input events are not lost while the device is frozen; they are
simply queued for later processing.
.IP \(bu 3n
If the other_device_mode is 
\fBGrabModeAsync\fP,
event processing from other input devices is unaffected
by activation of the grab.  
If other_device_mode is \fBGrabModeSync\fP,
the state of all devices except the grabbed device
(as seen by client applications) appears to freeze, and no further
events are generated by the server until the grabbing client issues a
releasing 
\fBXAllowEvents\fP or \fBXAllowDeviceEvents\fP
call or until the device grab is released.
Actual events are not lost
while the other devices are frozen; they are simply queued for later
processing.
.LP
\fBXGrabDevice\fP fails and returns:
.IP \(bu 3n
\fBAlreadyGrabbed\fP
If the device is actively grabbed by some other client.
.IP \(bu 3n
\fBGrabNotViewable\fP
If grab_window is not viewable.
.IP \(bu 3n
\fBGrabInvalidTime\fP
If the specified time is earlier
than the last-grab-time for the specified device
or later than the current X server time. Otherwise,
the last-grab-time for the specified device is set
to the specified time and 
\fBCurrentTime\fP
is replaced by the current X server time.
.IP \(bu 3n
\fBGrabFrozen\fP
If the device is frozen by an active grab of another client.
.LP
If a grabbed device is closed by a client while an active grab by that 
client is in
effect, that active grab will be released.  Any passive grabs established by
that client will be released.  If the device is frozen only by an active grab
of the requesting client, it is thawed.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadWindow\fP, 
\fBBadValue\fP, \fBBadClass\fP.
.sp 2
To release a grab of an extension device, use \fBXUngrabDevice\fP.
.DS
\fC
int
XUngrabDevice (display, device, time)
        Display *display;
        XDevice *device;
        Time    time;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fItime\fP"
Specifies the time.  This may be either a timestamp expressed in
milliseconds, or \fBCurrentTime\fP.
.in -.5i
.RE
.LP
This function allows a client to release an extension input device and any
queued events if this client has it grabbed from either \fBXGrabDevice\fP
or \fBXGrabDeviceKey\fP.  If any other devices are frozen by the grab,
\fBXUngrabDevice\fP thaws them. 
The function does not release the device and any
queued events if the specified time is earlier than the last-device-grab
time or is later than the current X server time.  It also generates 
\fBDeviceFocusIn\fP and \fBDeviceFocusOut\fP events.  The X server 
automatically performs an \fBXUngrabDevice\fP if the event window for an
active device grab becomes not viewable, or if the client terminates without
releasing the grab.
.LP
Errors returned by this function:  \fBBadDevice\fP.
.sp 2
.NH 3
Passively Grabbing A Key
.XS
\*(SN Passively Grabbing A Key
.XE
.LP
To passively grab a single key on an extension device, use \fBXGrabDeviceKey\fP.
That device must have previously been opened using the
\fBXOpenDevice\fP function, or the request will fail with a \fBBadDevice\fP
error.  
If the specified device does not support input class \fBKeys\fP,
the request will fail with a \fBBadMatch\fP error.
.DS
\fC
int
XGrabDeviceKey (display, device, keycode, modifiers, modifier_device
    grab_window, owner_events, event_count, event_list, 
    this_device_mode, other_device_mode)
        Display     *display;
        XDevice     *device;
        int         keycode;
        unsigned    int modifiers;
        XDevice     *modifier_device;
        Window      grab_window;
        Bool        owner_events;
        int         event_count;
        XEventClass *event_list;
        int         this_device_mode;
        int         other_device_mode;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIkeycode\fP"
Specifies the keycode of the key that is to be grabbed.  You can pass
either the keycode or \fBAnyKey\fR.
.IP "\fImodifiers\fP"
Specifies the set of keymasks.  This mask is the bitwise inclusive OR
of these keymask bits:  \fBShiftMask\fR, \fBLockMask\fR, \fBControlMask\fR, 
\fBMod1Mask\fR, \fBMod2Mask\fR, \fBMod3Mask\fR, \fBMod4Mask\fR, \fBMod5Mask\fR.
.IP "~"
You can also pass \fBAnyModifier\fP, which is equivalent to issuing the grab 
key request for all possible modifier combinations (including the combination
of no modifiers).
.IP "\fImodifier_device\fP"
Specifies the device whose modifiers are to be used.  If \fBNULL\fP is 
specified, the core X keyboard is used as the modifier_device.
.IP "\fIgrab_window\fP"
Specifies the ID of a window associated with the device specified above.
.IP "\fIowner_events\fR"
Specifies a boolean value of either \fBTrue\fR or \fBFalse\fR.
.IP "\fIevent_count\fP"
Specifies the number of elements in the event_list array.
.IP "\fIevent_list\fP"
Specifies a pointer to a list of event classes that indicate which events
the client wishes to receive.
.IP "\fIthis_device_mode\fP"
Controls further processing of events from this device.  You can pass one
of these constants: \fBGrabModeSync\fP or \fBGrabModeAsync\fP.
.IP "\fIother_device_mode\fP"
Controls further processing of events from all other devices.  You can pass one
of these constants: \fBGrabModeSync\fP or \fBGrabModeAsync\fP.
.in -.5i
.RE
.LP
This function is analogous to the core \fBXGrabKey\fP function.  It creates an
explicit passive grab for a key on an extension device.
.LP
The \fBXGrabDeviceKey\fP function establishes a passive grab on a device.
Consequently, in the future, 
.IP \(bu 3n
IF the device is not grabbed and the specified key, 
which itself can be a modifier key, is logically pressed
when the specified modifier keys logically are down on the specified
modifier device
(and no other keys are down),
.IP \(bu 3n
AND no other modifier keys logically are down,
.IP \(bu 3n
AND EITHER the grab window is an ancestor of (or is) the focus window
OR the grab window is a descendent of the focus window and contains the pointer,
.IP \(bu 3n
AND a passive grab on the same device and key combination does not exist on any
ancestor of the grab window,
.IP \(bu 3n
THEN the device is actively grabbed, as for \fBXGrabDevice\fP,
the last-device-grab time is set to the time at which the key was pressed
(as transmitted in the \fBDeviceKeyPress\fP event), and the 
\fBDeviceKeyPress\fP event is reported.
.LP
The interpretation of the remaining arguments is as for \fBXGrabDevice\fP.
The active grab is terminated automatically when the logical state of the
device has the specified key released
(independent of the logical state of the modifier keys).
.LP
Note that the logical state of a device (as seen by means of the X protocol)
may lag the physical state if device event processing is frozen.
.LP
A modifier of \fBAnyModifier\fP is equivalent to issuing the request for all
possible modifier combinations (including the combination of no modifiers).  
It is not required that all modifiers specified have
currently assigned keycodes.
A key of \fBAnyKey\fP is equivalent to issuing
the request for all possible keycodes.  Otherwise, the key must be in
the range specified by min_keycode and max_keycode in the 
information returned by the \fBXListInputDevices\fP
function.
If it is not within that range, \fBXGrabDeviceKey\fP generates a
\fBBadValue\fP error.
.LP
A \fBBadAccess\fP error is generated if some other client has issued a 
\fBXGrabDeviceKey\fP with the same device and key combination on the 
same window.  When using \fBAnyModifier\fP or \fBAnyKey\fP,
the request fails completely and the X server generates a \fBBadAccess\fP
error and no grabs are
established if there is a conflicting grab for any combination.
.LP
\fBXGrabDeviceKey\fP can generate \fBBadDevice\fP, \fBBadAccess\fP,
\fBBadMatch\fP, \fBBadWindow\fP, \fBBadClass\fP, and \fBBadValue\fP errors.
.LP
\fBXGrabDeviceKey\fP returns \fBSuccess\fP upon successful completion of the
request.
.sp 2
To release a passive grab of a single key on an extension device, 
use \fBXUngrabDeviceKey\fP.
.DS
\fC
int
XUngrabDeviceKey (display, device, keycode, modifiers, 
    modifier_device, ungrab_window)
        Display  *display;
        XDevice  *device;
        int      keycode;
        unsigned int modifiers;
        XDevice  *modifier_device;
        Window   ungrab_window;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIkeycode\fP"
Specifies the keycode of the key that is to be ungrabbed.  You can pass
either the keycode or \fBAnyKey\fR.
.IP "\fImodifiers\fP"
Specifies the set of keymasks.  This mask is the bitwise inclusive OR
of these keymask bits:  \fBShiftMask\fR, \fBLockMask\fR, \fBControlMask\fR, 
\fBMod1Mask\fR, \fBMod2Mask\fR, \fBMod3Mask\fR, \fBMod4Mask\fR, \fBMod5Mask\fR.
.IP "~"
You can also pass \fBAnyModifier\fP, which is equivalent to issuing the ungrab key 
request for all possible modifier combinations (including the combination
of no modifiers).
.IP "\fImodifier_device\fP"
Specifies the device whose modifiers are to be used.  If \fBNULL\fP is 
specified, the core X keyboard is used as the modifier_device.
.IP "\fIungrab_window\fP"
Specifies the ID of a window associated with the device specified above.
.in -.5i
.RE
.LP
.SH DESCRIPTION
.LP
This function is analogous to the core \fBXUngrabKey\fP function.  It releases
an explicit passive grab for a key on an extension input device.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadWindow\fP,
\fBBadValue\fP, \fBBadAlloc\fP, and \fBBadMatch\fP.
.sp 2
.NH 3
Passively Grabbing A Button
.XS
\*(SN Passively Grabbing A Button
.XE
.LP
To establish a passive grab for a single button on an extension device,
use \fBXGrabDeviceButton\fP.
The specified device must have previously been opened using the
\fBXOpenDevice\fP function, or the request will fail with a \fBBadDevice\fP
error.  If the specified device does not support input class \fBButtons\fP,
the request will fail with a \fBBadMatch\fP error.
.DS
\fC
int
XGrabDeviceButton (display, device, button, modifiers, 
    modifier_device, grab_window, owner_events, event_count, 
    event_list, this_device_mode, other_device_mode)
        Display      *display;
        XDevice      *device;
        unsigned int button;
        unsigned int modifiers;
        XDevice      *modifier_device;
        Window       grab_window;
        Bool         owner_events;
        int          event_count;
        XEventClass  *event_list;
        int          this_device_mode;
        int          other_device_mode;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIbutton\fP"
Specifies the code of the button that is to be grabbed.  You can pass
either the button or \fBAnyButton\fR.
.IP "\fImodifiers\fP"
Specifies the set of keymasks.  This mask is the bitwise inclusive OR
of these keymask bits:  \fBShiftMask\fR, \fBLockMask\fR, \fBControlMask\fR, 
\fBMod1Mask\fR, \fBMod2Mask\fR, \fBMod3Mask\fR, \fBMod4Mask\fR, \fBMod5Mask\fR.
.IP "~"
You can also pass \fBAnyModifier\fP, which is equivalent to issuing the grab
request for all possible modifier combinations (including the combination
of no modifiers).
.IP "\fImodifier_device\fP"
Specifies the device whose modifiers are to be used.  If \fBNULL\fP is 
specified, the core X keyboard is used as the modifier_device.
.IP "\fIgrab_window\fP"
Specifies the ID of a window associated with the device specified above.
.IP "\fIowner_events"
Specifies a boolean value of either \fBTrue\fR or \fBFalse\fR.
.IP "\fIevent_count\fP"
Specifies the number of elements in the event_list array.
.IP "\fIevent_list\fP"
Specifies a list of event classes that indicates which device events are to be 
reported to the client.
.IP "\fIthis_device_mode\fP"
Controls further processing of events from this device.  You can pass one
of these constants: \fBGrabModeSync\fP or \fBGrabModeAsync\fP.
.IP "\fIother_device_mode\fP"
Controls further processing of events from all other devices.  You can pass one
of these constants: \fBGrabModeSync\fP or \fBGrabModeAsync\fP.
.in -.5i
.RE
.LP
This function is analogous to the core \fBXGrabButton function\fP.  
It creates an
explicit passive grab for a button on an extension input device.  Since the
server does not track extension devices, no cursor is specified with this
request.  For the same reason, there is no confine_to parameter.
The device must have previously been opened using the
\fBXOpenDevice\fP function.
.LP
The \fBXGrabDeviceButton\fP function establishes a passive grab on a device.
Consequently, in the future, 
.IP \(bu 3n
IF the device is not grabbed and the specified button is logically pressed
when the specified modifier keys logically are down 
(and no other buttons or modifier keys are down),
.IP \(bu 3n
AND EITHER the grab window is an ancestor of (or is) the focus window
OR the grab window is a descendent of the focus window and contains the pointer,
.IP \(bu 3n
AND a passive grab on the same device and button/ key combination does not 
exist on any ancestor of the grab window,
.IP \(bu 3n
THEN the device is actively grabbed, as for \fBXGrabDevice\fP,
the last-grab time is set to the time at which the button was pressed
(as transmitted in the \fBDeviceButtonPress\fP event), and the 
\fBDeviceButtonPress\fP event is reported.
.LP
The interpretation of the remaining arguments is as for 
\fBXGrabDevice\fP.
The active grab is terminated automatically when logical state of the
device has all buttons released (independent of the logical state of 
the modifier keys).
.LP
Note that the logical state of a device (as seen by means of the X protocol)
may lag the physical state if device event processing is frozen.
.LP
A modifier of \fBAnyModifier\fP
is equivalent to issuing the request for all
possible modifier combinations (including the combination of no
modifiers).  
It is not required that all modifiers specified have
currently assigned keycodes.
A button of \fBAnyButton\fP is equivalent to issuing
the request for all possible buttons.
Otherwise, it is not required that the 
specified button be assigned to a physical button.
.LP
A 
\fBBadAccess\fP error is generated if some other client has issued a 
\fBXGrabDeviceButton\fP
with the same device and button combination on the same window.  
When using \fBAnyModifier\fP or \fBAnyButton\fP, the request fails completely 
and the X server generates a \fBBadAccess\fP
error and no grabs are
established if there is a conflicting grab for any combination.
.LP
\fBXGrabDeviceButton\fP can generate \fBBadDevice\fP, \fBBadMatch\fP,
\fBBadAccess\fP, \fBBadWindow\fP, \fBBadClass\fP, and \fBBadValue\fP errors.
.sp 2
To release a passive grab of a button on an extension device, use 
\fBXUngrabDeviceButton\fP.
.DS
\fC
int
XUngrabDeviceButton (display, device, button, modifiers, 
    modifier_device, ungrab_window)
        Display  *display;
        XDevice  *device;
        unsigned int button;
        unsigned int modifiers;
        XDevice  *modifier_device;
        Window   ungrab_window;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIbutton\fP"
Specifies the code of the button that is to be ungrabbed.  You can pass
either a button or \fBAnyButton\fR.
.IP "\fImodifiers\fP"
Specifies the set of keymasks.  This mask is the bitwise inclusive OR
of these keymask bits:  \fBShiftMask\fR, \fBLockMask\fR, \fBControlMask\fR, 
\fBMod1Mask\fR, \fBMod2Mask\fR, \fBMod3Mask\fR, \fBMod4Mask\fR, \fBMod5Mask\fR.
.IP "~"
You can also pass \fBAnyModifier\fP, which is equivalent to issuing the ungrab key 
request for all possible modifier combinations (including the combination
of no modifiers).
.IP "\fImodifier_device\fP"
Specifies the device whose modifiers are to be used.  If \fBNULL\fP is 
specified, the core X keyboard is used as the modifier_device.
.IP "\fIungrab_window\fP"
Specifies the ID of a window associated with the device specified above.
.in -.5i
.RE
.LP
This function is analogous to the core \fBXUngrabButton\fP function.  It
releases an explicit passive grab for a button on an extension device.
That device must have previously been opened using the
\fBXOpenDevice\fP function, or a \fBBadDevice\fP error will result.
.LP
A modifier of \fBAnyModifier\fP
is equivalent to issuing the request for all
possible modifier combinations (including the combination of no
modifiers).  
.LP
\fBXUngrabDeviceButton\fP can generate \fBBadDevice\fP, \fBBadMatch\fP,
\fBBadWindow\fP, \fBBadValue\fP, and \fBBadAlloc\fP errors.
.NH 3
Thawing A Device
.XS
\*(SN Thawing A Device
.XE
.LP
To allow further events to be processed when a device has been frozen,
use \fBXAllowDeviceEvents\fR.
.DS
\fC
int
XAllowDeviceEvents (display, device, event_mode, time)
        Display *display;
        XDevice *device;
        int     event_mode;
        Time    time;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIevent_mode\fP"
Specifies the event mode.  You can pass one of these constants:
\fBAsyncThisDevice\fP, \fBSyncThisDevice\fP, \fBAsyncOtherDevices\fP,
\fBReplayThisDevice\fP, \fBAsyncAll\fP, or \fBSyncAll\fP.
.IP "\fItime\fP"
Specifies the time.  This may be either a timestamp expressed in
milliseconds, or \fBCurrentTime\fP.
.in -.5i
.RE
.LP
The \fBXAllowDeviceEvents\fP function releases some queued events if the client
has caused a device to freeze.
The function has no effect if the specified time is earlier than the last-grab 
time of the most recent active grab for the client and device, 
or if the specified time is later than the current X server time.
The following describes the processing that occurs depending on what constant
you pass to the event_mode argument:
.IP \(bu 3n \fBAsyncThisDevice\fP
If the specified device is frozen by the client, event processing for that 
continues as usual.  If the device is frozen multiple times  by the client on 
behalf of multiple separate grabs, AsyncThisDevice thaws for all.
AsyncThisDevice has no effect if the specified device is not frozen by the 
client, but the device need not be grabbed by the client.
.IP \(bu 3n \fBSyncThisDevice\fP 
If the specified device is frozen and actively grabbed by the client,
event processing for that device continues normally until the next 
key or button event is reported to the client.
At this time, 
the specified device again appears to freeze.
However, if the reported event causes the grab to be released,
the specified device does not freeze.
SyncThisDevice has no effect if the specified device is not frozen by the client
or is not grabbed by the client.
.IP \(bu 3n \fBReplayThisDevice\fP
If the specified device is actively grabbed by the client and is frozen as the result of
an event having been sent to the client (either from the activation of a 
GrabDeviceButton or from a previous AllowDeviceEvents with mode SyncThisDevice, 
but not from a Grab),
the grab is released and that event is completely reprocessed.
This time, however, the request ignores any passive grabs at or above 
(towards the root) the grab-window of the grab just released.
The request has no effect if the specified device is not grabbed by the client
or if it is not frozen as the result of an event.
.IP \(bu 3n \fBAsyncOtherDevices\fP
If the remaining devices are frozen by the client,
event processing for them continues as usual.
If the other devices are frozen multiple times  by the client on behalf of 
multiple separate grabs,
AsyncOtherDevices ``thaws'' for all.
AsyncOtherDevices has no effect if the devices are not frozen by the client,
but those devices need not be grabbed by the client.
.IP \(bu 3n \fBSyncAll\fP
If all devices are frozen by the client,
event processing (for all devices) continues normally until the next
button or key event is reported
to the client for a grabbed device
at which time the devices again appear to
freeze.  However, if the reported event causes the grab to be released,
then the devices do not freeze (but if any device is still
grabbed, then a subsequent event for it will still cause all devices
to freeze).  
SyncAll has no effect unless all devices
are frozen by the client.  If any device is frozen twice
by the client on behalf of two separate grabs, 
SyncAll "thaws" for both (but a subsequent freeze for SyncAll
will only freeze each device once).
.IP \(bu 3n \fBAsyncAll\fP
If all devices are frozen by the
client, event processing (for all devices) continues normally.
If any device is frozen multiple times by the client on behalf of multiple
separate grabs, AsyncAll "thaws" for all.
If any device is frozen twice by the client on behalf of two separate grabs,
AsyncAll "thaws" for both.
AsyncAll has no effect unless all
devices are frozen by the client.
.LP
AsyncThisDevice, SyncThisDevice, and ReplayThisDevice 
have no effect on the processing of events from the remaining devices.
AsyncOtherDevices
has no effect on the processing of events from the specified device.
When the event_mode is SyncAll or AsyncAll, the 
device parameter is ignored.
.LP
It is possible for several grabs of different devices (by the same 
or different clients) to be active simultaneously.
If a device is frozen on behalf of any grab,
no event processing is performed for the device.
It is possible for a single device to be frozen because of several grabs.
In this case,
the freeze must be released on behalf of each grab before events can 
again be processed.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadValue\fP.
.NH 3
Controlling Device Focus
.XS
\*(SN Controlling Device Focus
.XE
.LP
The current focus window for an extension input device can be 
determined using the \fBXGetDeviceFocus\fP function.
Extension devices are focused using the \fBXSetDeviceFocus\fP
function in the same way that the keyboard is focused using
the core \fBXSetInputFocus\fP function, except that a device id is passed as
a function parameter.  One additional focus state, \fBFollowKeyboard\fP,
is provided for extension devices.
.LP
To get the current focus state, revert state, and focus time of an extension device,
use \fBXGetDeviceFocus\fP.
.DS
\fC
int
XGetDeviceFocus (display, device, focus_return, revert_to_return,
    focus_time_return)
        Display *display;
        XDevice *device;
        Window  *focus_return;
        int     *revert_to_return;
        Time    *focus_time_return;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIfocus_return\fP"
Specifies the address of a variable into which the server can return the ID of
the window that contains the device focus, 
or one of the constants \fBNone\fP,
\fBPointerRoot\fP, or \fBFollowKeyboard\fP.  
.IP "\fIrevert_to_return\fP"
Specifies the address of a variable into which the server can
return the current revert_to status for the device.
.IP "\fIfocus_time_return\fP"
Specifies the address of a variable into which the server can
return the focus time last set for the device.
.in -.5i
.RE
.LP
This function returns the focus state, the revert-to state,
and the last-focus-time for an extension input device.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadMatch\fP.
.sp 2
To set the focus of an extension device, use \fBXSetDeviceFocus\fP.
.DS
\fC
int
XSetDeviceFocus (display, device, focus, revert_to, time)
        Display *display;
        XDevice *device;
        Window  focus;
        int     revert_to;
        Time    time;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIfocus\fP"
Specifies the id of the window to which the device's focus should be set.
This may be a window id, or
\fBPointerRoot\fP, \fBFollowKeyboard\fP, or \fBNone\fP.
.IP "\fIrevert_to\fP"
Specifies to which window the focus of the device should revert
if the focus window becomes not viewable.  One of the following
constants may be passed:
\fBRevertToParent\fP, \fBRevertToPointerRoot\fP,
\fBRevertToNone\fP, or \fBRevertToFollowKeyboard\fP.
.IP "\fItime\fP"
Specifies the time.  You can pass either a timestamp, expressed in
milliseconds, or \fBCurrentTime\fP.
.in -.5i
.RE
.LP
This function changes the focus for an extension input device and the 
last-focus-change-time.  The function has no effect if the specified 
time is earlier than the last-focus-change-time or is later than the
current X server time.  Otherwise, the last-focus-change-time is set to the
specified time.
This function causes the X server to generate \fBDeviceFocusIn\fP and 
\fBDeviceFocusOut\fP events.
.LP
The action taken by the server when this function is requested depends
on the value of the focus argument:
.IP \(bu 3n
If the focus argument is \fBNone\fP, all input events from this device
will be discarded until a new focus window is set.  In this case, the
revert_to argument is ignored.
.IP \(bu 3n
If a window ID is assigned to the focus argument, it becomes the focus
window of the device.  If an input event from the device would normally
be reported to this window or to one of its inferiors, the event is 
reported normally.  Otherwise, the event is reported relative to the focus 
window.
.IP \(bu 3n
If you assign \fBPointerRoot\fP to the focus argument, the focus window is 
dynamically taken to be the root window of whatever screen the pointer is
on at each input event.  In this case, the revert_to argument is ignored.
.IP \(bu 3n
If you assign \fBFollowKeyboard\fP to the focus argument, the focus window is 
dynamically taken to be the same as the focus of the X keyboard at each
input event.
.LP
The specified focus window must be viewable at the time \fBXSetDeviceFocus\fP
is called.  Otherwise, it generates a \fBBadMatch error\fP.  If the focus window
later becomes not viewable, the X server evaluates the revert_to argument
to determine the new focus window.
.IP \(bu 3n
If you assign \fBRevertToParent\fP
to the revert_to argument, the focus reverts to the parent
(or the closest viewable ancestor), and the new revert_to value is taken to
be \fBRevertToNone\fP.  
.IP \(bu 3n
If you assign \fBRevertToPointerRoot\fP, \fBRevertToFollowKeyboard\fP,
or \fBRevertToNone\fP
to the revert_to argument, the focus reverts to that value.
.LP
When the focus reverts,
the X server generates \fBDeviceFocusIn\fP
and \fBDeviceFocusOut\fP
events, but the last-focus-change time is not affected.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadMatch\fP, 
\fBBadValue\fP, and \fBBadWindow\fP.
.NH 3
Controlling Device Feedback
.XS
\*(SN Controlling Device Feedback
.XE
.LP
To determine the current feedback settings of an extension input device, 
use \fBXGetFeedbackControl\fP.
.LP
.DS
\fC
XFeedbackState
*XGetFeedbackControl (display, device, num_feedbacks_return)
        Display          *display;
        XDevice          *device;
        int              *num_feedbacks_return;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fInum_feedbacks_return\fP"
Returns the number of feedbacks supported by the device.
.in -.5i
.RE
.LP
.IP \(bu 3n
This function returns a list of \fBFeedbackState\fP structures that 
describe the feedbacks supported by the specified device.  There is an
\fBXFeedbackState\fP structure for each clase of feedback.  These are of 
variable length, but the first three fields are common to all.
The common fields are as follows:
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
} XFeedbackState;
\fP
.DE
.LP
where \fBclass\fP identifies the class of feedback.  The \fBclass\fP
may be compared to constants defined in the file \fBXI.h\fP.  Currently
defined feedback constants include \fBKbdFeedbackClass\fP, 
\fBPtrFeedbackClass\fP, \fBStringFeedbackClass\fP, \fBIntegerFeedbackClass\fP, 
\fBLedFeedbackClass\fP, and \fBBellFeedbackClass\fP.
.LP
The \fBlength\fP specifies the length of the \fBFeedbackState\fP structure
and can be used by clients to traverse the list.
.LP
The \fBid\fP uniquely identifies a feedback for a given device and class.
This allows a device to support more than one feedback of the same class.
Other feedbacks of other classes or devices may have the same id.
.IP \(bu 3n
Those feedbacks equivalent to those
supported by the core keyboard are reported in class \fBKbdFeedback\fP
using the \fBXKbdFeedbackState\fP structure.
The members of that structure are as follows:
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     click;
    int     percent;
    int     pitch;
    int     duration;
    int     led_mask;
    int     global_auto_repeat;
    char    auto_repeats[32];
} XKbdFeedbackState;
\fP
.DE
.LP
The fields of the \fBXKbdFeedbackState\fP structure report the current state of
the feedback:
.IP \(bu 3n
\fBclick\fP specifies the key-click volume, and has a value in the range
0 (off) to 100 (loud).
.IP \(bu 3n
\fBpercent\fP specifies the bell volume, and has a value in the range
0 (off) to 100 (loud).
.IP \(bu 3n
\fBpitch\fP specifies the bell pitch in Hz. The range of the value is 
implementation-dependent.
.IP \(bu 3n
\fBduration\fP specifies the duration in milliseconds of the bell.
.IP \(bu 3n
\fBled_mask\fP is a bit mask that describes the current state of up to 
32 LEDs.  A value of 1 in a bit indicates that the corresponding LED is on.
.IP \(bu 3n
\fBglobal_auto_repeat\fP has a value of \fBAutoRepeatModeOn\fP or
\fBAutoRepeatModeOff\fP.
.IP \(bu 3n
The \fBauto_repeats\fP member is a bit vector.  Each bit set to 1 indicates
that auto-repeat is enabled for the corresponding key.  The vector is
represented as 32 bytes.  Byte N (from 0) contains the bits for keys
8N to 8N + 7, with the least significant bit int the byte representing
key 8N.
.LP
Those feedbacks equivalent to those
supported by the core pointer are reported in class \fBPtrFeedback\fP
using he \fBXPtrFeedbackState\fP structure.
The members of that structure are as follows:
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     accelNum;
    int     accelDenom;
    int     threshold;
} XPtrFeedbackState;
\fP
.DE
.LP
The fields of the \fBXPtrFeedbackState\fP structure report the current state of
the feedback:
.IP \(bu 3n
\fBaccelNum\fP returns the numerator for the acceleration multiplier.
.IP \(bu 3n
\fBaccelDenom\fP returns the denominator for the acceleration multiplier.
.IP \(bu 3n
\fBaccelDenom\fP returns the threshold for the acceleration.
.LP
\fBInteger\fP feedbacks are those capable of displaying integer numbers.  
The minimum and maximum values that they can display are reported.
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     resolution;
    int     minVal;
    int     maxVal;
} XIntegerFeedbackState;
\fP
.DE
.LP
The fields of the \fBXIntegerFeedbackState\fP structure report the capabilities
of the feedback:
.IP \(bu 3n
\fBresolution\fP specifies the number of digits that the feedback can display.
.IP \(bu 3n
\fBminVal\fP specifies the minimum value that the feedback can display.
.IP \(bu 3n
\fBmaxVal\fP specifies the maximum value that the feedback can display.
.LP
\fBString\fP feedbacks are those that can display character information.  
Clients set these feedbacks by passing a list of \fBKeySyms\fP to be displayed.
The \fBXGetFeedbackControl\fP function returns the
set of key symbols that the feedback can display, as well as the 
maximum number of symbols that can be displayed.
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     max_symbols;
    int     num_syms_supported;
    KeySym  *syms_supported;
} XStringFeedbackState;
\fP
.DE
.LP
The fields of the \fBXStringFeedbackState\fP structure report the capabilities
of the feedback:
.IP \(bu 3n
\fBmax_symbols\fP specifies the maximum number of symbols that can be displayed.
.IP \(bu 3n
\fBsyms_supported\fP is a pointer to the list of supported symbols.
.IP \(bu 3n
\fBnum_syms_supported\fP specifies the length of the list of supported symbols.
.LP
\fBBell\fP feedbacks are those that can generate a sound.  Some implementations
may support a bell as part of a \fBKbdFeedback\fP feedback.  Class
\fBBellFeedback\fP is provided for implementations that do not choose to do
so, and for devices that support multiple feedbacks that can produce sound.
The meaning of the fields is the same as that of the corresponding fields in
the \fBXKbdFeedbackState\fP structure.
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     percent;
    int     pitch;
    int     duration;
} XBellFeedbackState;
\fP
.DE
.LP
\fBLed\fP feedbacks are those that can generate a light.
Up to 32 lights per feedback are supported.  
Each bit in led_mask
corresponds to one supported light, and the corresponding bit in led_values
indicates whether that light is currently on (1) or off (0).
Some implementations may support leds as part of a \fBKbdFeedback\fP feedback.
Class \fBLedFeedback\fP is provided for implementations that do not choose to do
so, and for devices that support multiple led feedbacks.
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    Mask    led_values;
    Mask    led_mask;
} XLedFeedbackState;
\fP
.DE
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP.
.LP
To free the information returned by the \fBXGetFeedbackControl\fP function,
use \fBXFreeFeedbackList\fP.
.DS
\fC
void
XFreeFeedbackList (list)
        XFeedbackState *list;
\fP
.DE
.LP
.RS
.in +10n
.IP "\fIlist\fP" .75i
Specifies the pointer to the \fBXFeedbackState\fP structure returned by
a previous call to \fBXGetFeedbackControl\fP.
.in -10n
.RE
.LP
This function frees the list of feedback control information.
.LP
To change the settings of a feedback
on an extension device, use \fBXChangeFeedbackControl\fP.
This function modifies the current control values of the specified feedback
using information passed in the appropriate \fBXFeedbackControl\fP structure
for the feedback.
Which values are modified depends on the valuemask passed.
.LP
.DS
\fC
int
XChangeFeedbackControl (display, device, valuemask, value)
        Display          *display;
        XDevice          *device;
        unsigned long    valuemask;
        XFeedbackControl *value;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIvaluemask\fP"
Specifies one value for each bit in the mask (least to most significant
bit).  The values are associated with the feedbacks for the specified
device.
.IP "\fIvalue\fP"
Specifies a pointer to the \fBXFeedbackControl\fP structure.
.in -.5i
.RE
.LP
This function controls the device characteristics described by the
\fBXFeedbackControl\fP structure. 
There is an \fBXFeedbackControl\fP structure for each clase of feedback.  
These are of variable length, but the first 
two fields are common to all.  The common fields are as follows:
.LP
.DS
\fC
typedef struct {
    XID    class;
    int    length;
    XID    id;
} XFeedbackControl;
\fP
.DE
.LP
Feedback class \fBKbdFeedback\fP controls feedbacks equivalent to those
provided by the core keyboard using the \fBKbdFeedbackControl\fP structure.
The members of that structure are:
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     click;
    int     percent;
    int     pitch;
    int     duration;
    int     led_mask;
    int     led_value;
    int     key;
    int     auto_repeat_mode;
} XKbdFeedbackControl;
\fP
.DE
.LP
This class controls the device characteristics described by the
\fBXKbdFeedbackControl\fP structure.  These include the key_click_percent,
global_auto_repeat and individual key auto-repeat.  Valid modes
are \fBAutoRepeatModeOn\fP, \fBAutoRepeatModeOff\fP,
\fBAutoRepeatModeDefault\fP.
.LP
Valid masks are as follows:
.LP
.DS
#define DvKeyClickPercent          (1L << 0)
#define DvPercent                         (1L << 1)
#define DvPitch         	             (1L << 2)
#define DvDuration                       (1L << 3)
#define DvLed                              (1L << 4)
#define DvLedMode                       (1L << 5)
#define DvKey                             (1L << 6)
#define DvAutoRepeatMode          (1L << 7)
.DE
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP, 
\fBBadValue\fP.
.LP
Feedback class \fBPtrFeedback\fP controls feedbacks equivalent to those
provided by the core pointer using the \fBPtrFeedbackControl\fP structure.
The members of that structure are:
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     accelNum;
    int     accelDenom;
    int     threshold;
} XPtrFeedbackControl;
\fP
.DE
.LP
Which values are modified depends on the valuemask passed.
.LP
Valid masks are as follows:
.LP
.DS
#define DvAccelnum                 (1L << 0)
#define DvAccelDenom            (1L << 1)
#define DvThreshold                 (1L << 2)
.DE
.LP
The acceleration, expressed as a fraction, is a multiplier for movement. 
For example,
specifying 3/1 means the device moves three times as fast as normal.
The fraction may be rounded arbitrarily by the X server.  
Acceleration
only takes effect if the device moves more than threshold pixels at
once and only applies to the amount beyond the value in the threshold argument.
Setting a value to -1 restores the default.
The values of the accelNumerator and threshold fields must be nonzero for
the pointer values to be set.
Otherwise, the parameters will be unchanged.
Negative values generate a \fBBadValue\fP
error, as does a zero value
for the accelDenominator field.
.LP
This request fails with a \fBBadMatch\fP error if the specified device is not
currently reporting relative motion.  If a device that is capable of reporting
both relative and absolute motion has its mode changed from \fBRelative\fP to
\fBAbsolute\fP by an \fBXSetDeviceMode\fP request, valuator control values
will be ignored by the server while the device is in that mode.
.LP
Feedback class \fBIntegerFeedback\fP controls integer feedbacks displayed
on input devices, using the \fBIntegerFeedbackControl\fP structure.
The members of that structure are:
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     int_to_display;
} XIntegerFeedbackControl;
\fP
.DE
.LP
Valid masks are as follows:
.LP
.DS
#define DvInteger                    (1L << 0)
.DE
.LP
Feedback class \fBStringFeedback\fP controls string feedbacks displayed
on input devices, using the \fBStringFeedbackControl\fP structure.
The members of that structure are:
.LP
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     num_keysyms;
    KeySym  *syms_to_display;
} XStringFeedbackControl;
\fP
.DE
.LP
Valid masks are as follows:
.LP
.DS
#define DvString                     (1L << 0)
.DE
.LP
Feedback class \fBBellFeedback\fP controls a bell on an input device,
using the \fBBellFeedbackControl\fP structure.
The members of that structure are:
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     percent;
    int     pitch;
    int     duration;
} XBellFeedbackControl;
\fP
.DE
.LP
Valid masks are as follows:
.LP
.DS
#define DvPercent                    (1L << 1)
#define DvPitch                        (1L << 2)
#define DvDuration                  (1L << 3)
.DE
.LP
To ring a bell on an extension input device, use the \fBXDeviceBell\fP
protocol request.
.LP
Feedback class \fBLedFeedback\fP controls lights on an input device,
using the \fBLedFeedbackControl\fP structure.
The members of that structure are:
.DS
\fC
typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     led_mask;
    int     led_values;
} XLedFeedbackControl;
\fP
.DE
Valid masks are as follows:
.LP
.DS
#define DvLed                           (1L << 4)
#define DvLedMode                   (1L << 5)
.DE
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP,
\fBBadFeedBack\fP.
.NH 3
Ringing a Bell on an Input Device
.XS
\*(SN Ringing a Bell on an Input Device
.XE
.LP
To ring a bell on a extension input device, use \fBXDeviceBell\fP.
.DS
\fC
int
XDeviceBell (display, device, feedbackclass, feedbackid, percent)
        Display *display;
        XDevice *device;
        XID	feedbackclass, feedbackid;
        int     percent;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIfeedbackclass\fP"
Specifies the feedbackclass.  Valid values are KbdFeedbackClass and
BellFeedbackClass.
.IP "\fIfeedbackid\fP"
Specifies the id of the feedback that has the bell.
.IP "\fIpercent\fP"
Specifies the volume in the range -100 (quiet) to 100 percent (loud).
.in -.5i
.RE
.LP
This function is analogous to the core \fBXBell\fP function.  It rings the
specified bell on the specified input device feedback, using the specified 
volume.
The specified volume is relative to the base volume for the feedback.
If the value for the percent argument is not in the range -100 to 100
inclusive, a \fBBadValue\fP error results.
The volume at which the bell rings when the percent argument is nonnegative is:
.LP
.DS
	  base - [(base * percent) / 100] + percent
.DE
.LP
The volume at which the bell rings
when the percent argument is negative is:
.LP
.DS
	  base + [(base * percent) / 100]
.DE
.LP
To change the base volume of the bell, use \fBXChangeFeedbackControl\fP.
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadValue\fP.
.NH 3
Controlling Device Encoding
.XS
\*(SN Controlling Device Encoding
.XE
.LP
To get the key mapping of an extension device that supports input class 
\fBKeys\fP, use 
\fBXGetDeviceKeyMapping\fP.
.DS
\fC
KeySym
*XGetDeviceKeyMapping (display, device, first_keycode_wanted, 
    keycode_count, keysyms_per_keycode_return)
        Display *display;
        XDevice *device;
        KeyCode first_keycode_wanted;
        int     keycode_count;
        int     *keysyms_per_keycode_return;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIfirst_keycode_wanted\fP"
Specifies the first keycode that is to be returned.
.IP "\fIkeycode_count\fP"
Specifies the number of keycodes that are to be returned.
.IP "\fIkeysyms_per_keycode_return\fP"
Returns the number of keysyms per keycode.
.in -.5i
.RE
.LP
This function is analogous to the core \fBXGetKeyboardMapping\fP function.  
It returns the symbols for the specified number of keycodes for the 
specified extension device.
.LP
\fBXGetDeviceKeyMapping\fP returns the symbols for the 
specified number of keycodes for the 
specified extension device, starting with the specified keycode.
The first_keycode_wanted must be greater than or equal to
min-keycode as returned 
by the \fBXListInputDevices\fP request (else a \fBBadValue\fP error),
and
.LP
.DS
first_keycode_wanted + keycode_count \- 1
.DE
.LP
must be less than or equal to max-keycode as returned 
by the \fBXListInputDevices\fP request
(else a \fBBadValue\fP error).
.LP
The number of elements in the keysyms list is
.LP
.DS
keycode_count * keysyms_per_keycode_return
.DE
and KEYSYM number N (counting from zero) for keycode K has an index
(counting from zero) of
.LP
.DS
(K \- first_keycode_wanted) * keysyms_per_keycode_return + N
.DE
.LP
in keysyms.
The keysyms_per_keycode_return value is chosen arbitrarily by the server
to be large enough to report all requested symbols.
A special KEYSYM value of
\fBNoSymbol\fP
is used to fill in unused elements for individual keycodes.
.LP
You should use XFree to free the data returned by this function.
.LP
If the specified device has not first been opened by this client via
\fBXOpenDevice\fP, this request will fail with a \fBBadDevice\fP error.
If that device does not support input class Keys,
this request will fail with a \fBBadMatch\fP error.
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP,
\fBBadValue\fP.
.sp 2
.LP
To change the keyboard mapping of an extension device that supports input
class \fBKeys\fP, use 
\fBXChangeDeviceKeyMapping\fP.
.DS
\fC
int
XChangeDeviceKeyMapping (display, device, first_keycode, 
    keysyms_per_keycode, keysyms, num_codes)
        Display *display;
        XDevice *device;
        int     first_keycode;
        int     keysyms_per_keycode;
        KeySym  *keysyms;
        int     num_codes;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .78i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIfirst_keycode\fP"
Specifies the first keycode that is to be changed.
.IP "\fIkeysyms_per_keycode\fP"
Specifies the keysyms that are to be used.
.IP "\fIkeysyms\fP"
Specifies a pointer to an array of keysyms.
.IP "\fInum_codes\fP"
Specifies the number of keycodes that are to be changed.
.in -.5i
.RE
.LP
This function is analogous to the core \fBXChangeKeyboardMapping\fP function.  
It defines the symbols for the specified number of keycodes for the 
specified extension keyboard device.
.LP
If the specified device has not first been opened by this client via
\fBXOpenDevice\fP,
this request will fail with a \fBBadDevice\fP error.
If the specified device does not support input class Keys,
this request will fail with a \fBBadMatch\fP error.
.LP
The number of elements in the keysyms list must be a multiple of
keysyms_per_keycode.  Otherwise, \fBXChangeDeviceKeyMapping\fP generates
a \fBBadLength\fP error.  The specified first_keycode must be greater
than or equal to the min_keycode value returned by the \fBListInputDevices\fP
request, or this request will fail with a \fBBadValue\fP error.  In addition,
if the following expression is not less than the max_keycode value returned by
the ListInputDevices request, the request will fail with a BadValue
error:
.LP
.DS
	  first_keycode + (num_codes / keysyms_per_keycode) - 1
.DE
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP,
\fBBadValue\fP, \fBBadAlloc\fP.
.sp 2
.LP
To obtain the keycodes that are used as modifiers on an 
extension device that supports input class \fBKeys\fP, use
\fBXGetDeviceModifierMapping\fP.
.DS
\fC
XModifierKeymap 
*XGetDeviceModifierMapping (display, device)
        Display *display;
        XDevice *device;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.in -.5i
.RE
.LP
This function is analogous to the core \fBXGetModifierMapping\fP function.  
The \fBXGetDeviceModifierMapping\fP function returns a newly created
\fBXModifierKeymap\fP structure that contains the keys being used as
modifiers for the specified device.  The structure should be freed after
use with \fBXFreeModifierMapping\fP.  If only zero values appear in the set
for any modifier, that modifier is disabled.
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP.
.sp 2
.LP
To set which keycodes that are to be used as modifiers for an extension
device, use \fBXSetDeviceModifierMapping\fP.
.DS
\fC
int 
XSetDeviceModifierMapping (display, device, modmap)
        Display         *display;
        XDevice         *device;
        XModifierKeymap *modmap;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fImodmap\fP"
Specifies a pointer to the \fBXModifierKeymap\fP structure.
.in -.5i
.RE
.LP
This function is analogous to the core XSetModifierMapping function.  
The \fBXSetDeviceModifierMapping\fP function specifies the keycodes of 
the keys, if any, that are to be used as modifiers.  A zero value means
that no key should be used.  No two arguments can have the same nonzero
keycode value.  Otherwise, \fBXSetDeviceModifierMapping\fP generates a
\fBBadValue\fP error.
There are eight modifiers, and the modifiermap member of the 
\fBXModifierKeymap\fP structure contains eight sets of max_keypermod 
keycodes, one for each modifier in the
order Shift, Lock, Control, Mod1, Mod2, Mod3, Mod4, and Mod5.
Only nonzero keycodes have meaning in each set, and zero keycodes
are ignored.
In addition, all of the nonzero keycodes must be in the range specified by 
min_keycode and max_keycode reported by the \fBXListInputDevices\fP function.
Otherwise, \fBXSetModifierMapping\fP generates a \fBBadValue\fP
error.
No keycode may appear twice in the entire map. 
Otherwise, it generates a \fBBadValue\fP
error.
.LP
A X server can impose restrictions on how modifiers can be changed, 
for example,
if certain keys do not generate up transitions in hardware or if multiple
modifier keys are not supported.  
If some such restriction is violated, 
the status reply is
\fBMappingFailed,\fP
and none of the modifiers are changed.
If the new keycodes specified for a modifier differ from those
currently defined and any (current or new) keys for that modifier are
in the logically down state, 
the status reply is \fBMappingBusy\fP, 
and none of the modifiers are changed.
\fBXSetModifierMapping\fP
generates a \fBDeviceMappingNotify\fP
event on a \fBMappingSuccess\fP
status.
.LP
\fBXSetDeviceModifierMapping\fP can generate \fBBadDevice\fR, \fBBadMatch\fP,
\fBBadAlloc\fP, and \fBBadValue\fP errors.
.sp 2
.NH 3
Controlling Button Mapping
.XS
\*(SN Controlling Button Mapping
.XE
.LP
To set the mapping of the buttons on an extension device, use
\fBXSetDeviceButtonMapping\fP.
.DS
\fC
int
XSetDeviceButtonMapping (display, device, map, nmap)
        Display          *display;
        XDevice          *device;
        unsigned char    map[];
        int              nmap;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fImap\fP"
Specifies the mapping list.
.IP "\fInmap\fP"
Specifies the number of items in the mapping list.
.in -.5i
.RE
.LP
The
\fBXSetDeviceButtonMapping\fP function sets the mapping of the buttons on
an extension device.  If it succeeds, the X server generates a
\fBDeviceMappingNotify\fP event, and \fBXSetDeviceButtonMapping\fP
returns \fBMappingSuccess\fP.
Elements of the list are indexed starting from one.
The length of the list must be the same as
\fBXGetDeviceButtonMapping\fP would return, or a \fBBadValue\fP
error results.
The index is a button number, and the element of the list
defines the effective number.
A zero element disables a button, and elements are not restricted in
value by the number of physical buttons.
However, no two elements can have the same nonzero value,
or a \fBBadValue\fP error results.
If any of the buttons to be altered are logically in the down state,
\fBXSetDeviceButtonMapping\fP returns \fBMappingBusy\fP,
and the mapping is not changed.
.LP
\fBXSetDeviceButtonMapping\fP can generate \fBBadDevice\fP, \fBBadMatch\fP,
and \fBBadValue\fP errors.
.LP
.sp
To get the button mapping, use \fBXGetDeviceButtonMapping\fP.
.sp 2
.DS
\fC
int
XGetDeviceButtonMapping (display, device, map_return, nmap)
        Display          *display;
        XDevice          *device;
        unsigned char    map_return[];
        int              nmap;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fImap_return\fP"
Specifies the mapping list.
.IP "\fInmap\fP"
Specifies the number of items in the mapping list.
.in -.5i
.RE
.LP
The \fBXGetDeviceButtonMapping\fP
function returns the current mapping of the specified extension device.
Elements of the list are indexed starting from one.
\fBXGetDeviceButtonMapping\fP returns the number of physical buttons actually 
on the pointer.
The nominal mapping for the buttons is the identity mapping: map[i]=i.
The nmap argument specifies the length of the array where the button
mapping is returned, and only the first nmap elements are returned 
in map_return.
.LP
Errors returned by this function: \fBBadDevice\fP, \fBBadMatch\fP.
.sp 2
.NH 3
Obtaining The State Of A Device
.XS
\*(SN Obtaining The State Of A Device
.XE
.LP
To obtain information that describes the state of the keys, buttons and 
valuators of an extension device, use \fBXQueryDeviceState\fP.
.DS
\fC
XDeviceState 
*XQueryDeviceState (display, device)
        Display *display;
        XDevice *device;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.in -.5i
.RE
.LP
The \fBXQueryDeviceState\fP function returns a pointer to an
\fBXDeviceState\fP structure.  This structure points to a list of
structures that describe the state of the keys, buttons, and valuators 
on the device.
.DS
\fC
typedef struct {
        XID		device_id;
        int		num_classes;
        XInputClass	*data;
} XDeviceState;
\fP
.DE
.LP
.IP \(bu 3n
The structures are of variable length, but the first 
two fields are common to all.  The common fields are as follows:
.DS
\fC
typedef struct
    {
    unsigned char	class;
    unsigned char	length;
    } XInputClass;
\fP
.DE
.LP
The \fBclass\fP field contains a class identifier.  This identifier can be
compared with constants defined in the file \fBXI.h\fP.  Currently defined
constants are: \fBKeyClass\fP, \fBButtonClass\fP, and \fBValuatorClass\fP.
.LP
The \fBlength\fP field contains the length of the structure and can be used
by clients to traverse the list.
.LP
.IP \(bu 3n
The \fBXValuatorState\fP structure describes the current state of the valuators
on the device.  The \fBnum_valuators\fP field contains the number of valuators
on the device.  The \fBmode\fP 
field is a mask whose bits report the data mode and other state information
for the device.  The following bits are currently defined:
.DS
\fC
	DeviceMode	1 << 0	  Relative = 0, Absolute = 1
	ProximityState	1 << 1    InProximity = 0, OutOfProximity = 1
\fP
.DE
The \fBvaluators\fP field contains a pointer to an array of integers that
describe the current value of the valuators.  If the mode is \fBRelative\fP,
these values are undefined.
.LP
.DS
\fC
typedef struct {
    unsigned char	class;
    unsigned char	length;
    unsigned char	num_valuators;
    unsigned char	mode;
    int        		*valuators;
} XValuatorState;
\fP
.DE
.LP
.IP \(bu 3n
The \fBXKeyState\fP structure describes the current state of the keys
on the device.  Byte N (from 0) contains the
bits for key 8N to 8N+7 with the least significant bit in the
byte representing key 8N.
.DS
\fC
typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_keys;
    char        	keys[32];
} XKeyState;
\fP
.DE
.IP \(bu 3n
The \fBXButtonState\fP structure describes the current state of the buttons
on the device.  Byte N (from 0) contains the
bits for button 8N to 8N+7 with the least significant bit in the
byte representing button 8N.
.DS
\fC
typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_buttons;
    char        	buttons[32];
} XButtonState;
\fP
.DE
.LP
You should use \fBXFreeDeviceState\fP to free the data returned by this 
function.
.LP
Errors returned by this function: \fBBadDevice\fP.
.LP
.DS
\fC
void
XFreeDeviceState (state)
        XDeviceState *state;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIstate\fP" .75i
Specifies the pointer to the \fBXDeviceState\fP data returned by
a previous call to \fBXQueryDeviceState\fP.
.in -.5i
.RE
.LP
This function frees the device state data.
.NH 2
Events and Event-Handling Functions
.XS
\*(SN Events and Event-Handling Functions
.XE
.LP
The input extension creates input events analogous to the core input events.
These extension input events are generated by manipulating one of the
extension input devices.  The following sections describe these events
and explain how a client program can receive them.
.NH 3
Event Types
.XS
\*(SN Event Types
.XE
.LP
Event types are integer numbers that a client can use to determine what
kind of event it has received.  The client compares the type field of 
the event structure with known event types to make this determination.
.LP
The core input event types are constants and are defined in the header
file \fB<X11/X.h>\fP.  Extension event types are not constants.  Instead, they
are dynamically allocated by the extension's request to the X server
when the extension is initialized.  Because of this, extension event
types must be obtained by the client from the server.
.LP
The client program determines the event type for an extension event by using
the information returned by the \fBXOpenDevice\fP request.
This type can then be used for comparison with the type field
of events received by the client.
.LP
Extension events propagate up the window hierarchy in the same manner
as core events.  If a window is not interested in an extension event, 
it usually propagates to the closest ancestor that is interested,
unless the dont_propagate list prohibits it.
Grabs of extension devices may alter the set of windows that receive a particular
extension event.
.LP
The following table lists the event category and its associated event
type or types.
.sp
.TS
box center;
c | c
l | l.
Event Category	Event Type
=
Device key events	\fIDeviceKeyPress\fP,
	\fIDeviceKeyRelease\fP
_
Device motion events	\fIDeviceButtonPress\fP,
	\fIDeviceButtonRelease\fP,
	\fIDeviceMotionNotify\fP
_
Device input focus events	\fIDeviceFocusIn\fP,
	\fIDeviceFocusOut\fP
_
Device state notification events	\fIDeviceStateNotify\fP
_
Device proximity events	\fIProximityIn\fP,
	\fIProximityOut\fP
_
Device mapping events	\fIDeviceMappingNotify\fP
_
Device change events	\fIChangeDeviceNotify\fP
.TE
.sp
.NH 3
Event Classes
.XS
\*(SN Event Classes
.XE
.LP
Event classes are integer numbers that are used in the same way as the
core event masks.  They are used by a client program to indicate to the
server which events that client program wishes to receive.
.LP
The core input event masks are constants and are defined in the header
file \fB<X11/X.h>\fP.  Extension event classes are not constants.  Instead, 
they are dynamically allocated by the extension's request to the X server
when the extension is initialized.  Because of this, extension event
classes must be obtained by the client from the server.
.LP
The event class for an extension event and device is obtained from
information returned by the 
\fBXOpenDevice\fP function. 
This class can then be used in an \fBXSelectExtensionEvent\fP
request to ask that events of that type from that device be sent to
the client program.
.LP
For \fBDeviceButtonPress\fP events, the client may specify whether
or not an implicit passive grab should be done when the button is
pressed.  If the client wants to guarantee that it will receive
a \fBDeviceButtonRelease\fP event for each \fBDeviceButtonPress\fP
event it receives, it should specify the  \fBDeviceButtonPressGrab\fP
class in addition to the \fBDeviceButtonPress\fP class.
This restricts the client in that only one client at a time
may request \fBDeviceButtonPress\fP events from the same device and
window if any client specifies this class.
.LP
If any client has specified the \fBDeviceButtonPressGrab\fP class, any requests
by any other client that specify the same device and window and specify
either \fBDeviceButtonPress\fP or \fBDeviceButtonPressGrab\fP will
cause an \fBAccess\fP error to be generated.
.LP
If only the \fBDeviceButtonPress\fP class is specified, no implicit
passive grab will be done when a button is pressed on the device.
Multiple clients may use this class to specify the same device and
window combination.
.LP
The client may also select \fBDeviceMotion\fP events only when a 
button is down.  It does this by specifying the event classes 
\fBDeviceButton1Motion\fP through \fBDeviceButton5Motion\fP.  
An input device will only support
as many button motion classes as it has buttons.
.NH 3
Event Structures
.XS
\*(SN Event Structures
.XE
.LP
Each extension event type has a corresponding structure declared in
\fB<X11/extensions/XInput.h>\fP.  All event structures have the
following members:
.RS
.in +.5i
.IP "\fItype\fP" .80i
Set to the event type number that uniquely identifies it.  For example,
when the X server reports a \fBDeviceKeyPress\fP event to a client 
application, it sends an \fBXDeviceKeyPressEvent\fP structure.
.IP "\fIdisplay\fP"
Set to a pointer to a structure that defines the display the event was read on.
.IP "\fIsend_event\fP"
Set to \fBTrue\fP if the event came from an \fBXSendEvent\fP request.
.IP "\fIserial\fP"
Set from the serial number reported in the protocol but expanded from the
16-bit least-significant bits to a full 32-bit value.
.in -.5i
.RE
.LP
Extension event structures report the current position of the X pointer.
In addition, if the device reports motion data and is reporting absolute data,
the current value of any valuators the device contains is also reported.
.NH 4
Device Key Events
.XS
\*(SN Device Key Events
.XE
.LP
Key events from extension devices contain all the information that is
contained in a key event from the X keyboard.  In addition, they contain
a device id and report the current value of any valuators on the device,
if that device is reporting absolute data.
If data for more than six valuators is being reported, more than one
key event will be sent.
The axes_count field contains the number of axes that are being
reported.  The server sends as many of these events as are
needed to report the device data.  Each event contains the total number
of axes reported in the axes_count field, and the first axis reported
in the current event in the first_axis field.
If the device supports input class \fBValuators\fP, 
but is not reporting absolute mode data,
the axes_count field contains 0.
.LP
The location reported in 
the x,y and x_root,y_root fields is the location of the core X pointer.
.LP
The XDeviceKeyEvent structure is defined as follows:
.DS
\fC
.ps 8
typedef struct 
    {
    int            type;         /* of event */
    unsigned long  serial;       /* # of last request processed */
    Bool           send_event;   /* true if from SendEvent request */
    Display        *display;     /* Display the event was read from */
    Window         window;       /* "event" window reported relative to */
    XID            deviceid;
    Window         root;         /* root window event occurred on */
    Window         subwindow;    /* child window */
    Time           time;         /* milliseconds */
    int            x, y;         /* x, y coordinates in event window */
    int            x_root;       /* coordinates relative to root */
    int            y_root;       /* coordinates relative to root */
    unsigned int   state;        /* key or button mask */
    unsigned int   keycode;      /* detail */
    Bool           same_screen;  /* same screen flag */
    unsigned char  axes_count;
    unsigned char  first_axis;
    unsigned int   device_state; /* device key or button mask */
    int            axis_data[6];
    } XDeviceKeyEvent;

typedef XDeviceKeyEvent XDeviceKeyPressedEvent;
typedef XDeviceKeyEvent XDeviceKeyReleasedEvent;
\fP
.DE
.ps
.NH 4
Device Button Events
.XS
\*(SN Device Button Events
.XE
.LP
Button events from extension devices contain all the information that is
contained in a button event from the X pointer.  In addition, they contain
a device id and report the current value of any valuators on the device,
if that device is reporting absolute data.
If data for more than six valuators is being reported, more than one
button event may be sent.
The axes_count field contains the number of axes that are being
reported.  The server sends as many of these events as are
needed to report the device data.  Each event contains the total number
of axes reported in the axes_count field, and the first axis reported
in the current event in the first_axis field.
If the device supports input class \fBValuators\fP, 
but is not reporting absolute mode data,
the axes_count field contains 0.
.LP
The location reported in 
the x,y and x_root,y_root fields is the location of the core X pointer.
.DS
\fC
.ps 8
typedef struct {
    int           type;         /* of event */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    Window        window;       /* "event" window reported relative to */
    XID           deviceid;
    Window        root;         /* root window that the event occurred on */
    Window        subwindow;    /* child window */
    Time          time;         /* milliseconds */
    int           x, y;         /* x, y coordinates in event window */
    int           x_root;       /* coordinates relative to root */
    int           y_root;       /* coordinates relative to root */
    unsigned int  state;        /* key or button mask */
    unsigned int  button;       /* detail */
    Bool          same_screen;  /* same screen flag */
    unsigned char axes_count;
    unsigned char first_axis;
    unsigned int  device_state; /* device key or button mask */
    int           axis_data[6];
    } XDeviceButtonEvent;

typedef XDeviceButtonEvent XDeviceButtonPressedEvent;
typedef XDeviceButtonEvent XDeviceButtonReleasedEvent;
\fP
.DE
.ps
.NH 4
Device Motion Events
.XS
\*(SN Device Motion Events
.XE
.LP
Motion events from extension devices contain all the information that is
contained in a motion event from the X pointer.  In addition, they contain
a device id and report the current value of any valuators on the device.
.LP
The location reported in 
the x,y and x_root,y_root fields is the location of the core X pointer, 
and so is 2-dimensional.
.LP
Extension motion devices may report motion data for a variable number of 
axes.  
The axes_count field contains the number of axes that are being
reported.  The server sends as many of these events as are
needed to report the device data.  Each event contains the total number
of axes reported in the axes_count field, and the first axis reported
in the current event in the first_axis field.
.LP
.DS
\fC
.ps 8
typedef struct 
    {
    int           type;        /* of event */
    unsigned long serial;      /* # of last request processed by server */
    Bool          send_event;  /* true if from a SendEvent request */
    Display       *display;    /* Display the event was read from */
    Window        window;      /* "event" window reported relative to */
    XID           deviceid;
    Window        root;        /* root window that the event occurred on */
    Window        subwindow;   /* child window */
    Time          time;        /* milliseconds */
    int           x, y;        /* x, y coordinates in event window */
    int           x_root;      /* coordinates relative to root */
    int           y_root;      /* coordinates relative to root */
    unsigned int  state;       /* key or button mask */
    char          is_hint;     /* detail */
    Bool          same_screen; /* same screen flag */
    unsigned int  device_state; /* device key or button mask */
    unsigned char axes_count;
    unsigned char first_axis;
    int           axis_data[6];
    } XDeviceMotionEvent;
\fP
.DE
.ps
.NH 4
Device Focus Events
.XS
\*(SN Device Focus Events
.XE
.LP
These events are equivalent to the core focus events.
They contain the same information, with the addition
of a device id to identify which device has had a focus change,
and a timestamp.
.LP
\fBDeviceFocusIn\fP and \fBDeviceFocusOut\fP events are generated for
focus changes of extension devices in the same manner as core focus
events are generated.
.LP
.DS
\fC
.ps 8
typedef struct 
    {
    int           type;       /* of event */
    unsigned long serial;     /* # of last request processed by server */
    Bool          send_event; /* true if this came from a SendEvent request */
    Display       *display;   /* Display the event was read from */
    Window        window;     /* "event" window it is reported relative to */
    XID           deviceid;
    int           mode;       /* NotifyNormal, NotifyGrab, NotifyUngrab */
    int           detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior, 
	 * NotifyNonLinear,NotifyNonLinearVirtual, NotifyPointer,
	 * NotifyPointerRoot, NotifyDetailNone 
	 */
    Time                time;
    } XDeviceFocusChangeEvent;

typedef XDeviceFocusChangeEvent XDeviceFocusInEvent;
typedef XDeviceFocusChangeEvent XDeviceFocusOutEvent;
\fP
.ps
.DE
.NH 4
Device StateNotify Event
.XS
\*(SN Device StateNotify Event
.XE
.LP
This event is analogous to the core keymap event, but
reports the current state of the device for each
input class that it supports.
It is generated after every \fBDeviceFocusIn\fP event
and \fBEnterNotify\fP  event and is delivered to clients 
who have selected \fBXDeviceStateNotify\fP events.
.LP
If the device supports input class Valuators, the mode field in the
\fBXValuatorStatus\fP structure is a bitmask that reports the device mode,
proximity state and other state information.  The
following bits are currently defined:
.DS
\fC
	0x01		Relative = 0, Absolute = 1
	0x02		InProximity = 0, OutOfProximity = 1
\fP
.DE
.LP
If the device supports more valuators than can be reported in a single
\fBXEvent\fP, multiple \fBXDeviceStateNotify\fP events will be generated.
.LP
.DS
\fC
.ps 8
typedef struct
    {
    unsigned char	class;
    unsigned char	length;
    } XInputClass;

typedef struct {
    int           type;
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    Window        window;
    XID           deviceid;
    Time          time;
    int           num_classes;
    char	  data[64];
} XDeviceStateNotifyEvent;	
\fP
.ps
.DE
.LP
.DS
\fC
.ps 8
typedef struct {
    unsigned char	class;
    unsigned char	length;
    unsigned char	num_valuators;
    unsigned char	mode;
    int        		valuators[6];
} XValuatorStatus;
\fP
.ps
.DE
.LP
.DS
\fC
.ps 8
typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_keys;
    char        	keys[32];
} XKeyStatus;
\fP
.ps
.DE
.LP
.DS
\fC
.ps 8
typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_buttons;
    char        	buttons[32];
} XButtonStatus;
\fP
.ps
.DE
.NH 4
Device Mapping Event
.XS
\*(SN Device Mapping Event
.XE
.LP
This event is equivalent to the core MappingNotify event.
It notifies client programs when the mapping of keys,
modifiers, or buttons on an extension device has changed.
.LP
.DS
\fC
typedef struct {
    int            type;
    unsigned long  serial;	
    Bool           send_event;
    Display        *display;
    Window         window;
    XID            deviceid;
    Time           time;
    int            request;
    int            first_keycode;
    int            count;
} XDeviceMappingEvent;
\fP
.DE
.NH 4
ChangeDeviceNotify Event
.XS
\*(SN ChangeDeviceNotify Event
.XE
.LP
This event has no equivalent in the core protocol.  It notifies client
programs when one of the core devices has been changed.
.LP
.DS
\fC
typedef struct {
    int            type;
    unsigned long  serial;
    Bool           send_event;
    Display        *display;
    Window         window;
    XID            deviceid;
    Time           time;
    int            request;
} XChangeDeviceNotifyEvent;
\fP
.DE
.NH 4
Proximity Events
.XS
\*(SN Proximity Events
.XE
.LP
These events have no equivalent in the core protocol.  Some input
devices such as graphics tablets or touchscreens may send these
events to indicate that a stylus has moved into or out of contact
with a positional sensing surface.
.LP
The event contains the current value of any valuators on the device,
if that device is reporting absolute data.
If data for more than six valuators is being reported, more than one
proximity event may be sent.
The axes_count field contains the number of axes that are being
reported.  The server sends as many of these events as are
needed to report the device data.  Each event contains the total number
of axes reported in the axes_count field, and the first axis reported
in the current event in the first_axis field.
If the device supports input class \fBValuators\fP, 
but is not reporting absolute mode data,
the axes_count field contains 0.
.LP
.DS
\fC
.ps 8
typedef struct 
    {
    int             type;      /* ProximityIn or ProximityOut */        
    unsigned long   serial;    /* # of last request processed by server */
    Bool            send_event; /* true if this came from a SendEvent request */
    Display         *display;  /* Display the event was read from */
    Window          window;      
    XID	            deviceid;
    Window          root;            
    Window          subwindow;      
    Time            time;            
    int             x, y;            
    int             x_root, y_root;  
    unsigned int    state;           
    Bool            same_screen;     
    unsigned char   axes_count;
    unsigned char   first_axis;
    unsigned int    device_state; /* device key or button mask */
    int             axis_data[6];
    } XProximityNotifyEvent;
typedef XProximityNotifyEvent XProximityInEvent;
typedef XProximityNotifyEvent XProximityOutEvent;
\fP
.ps
.DE
.NH 3
Determining The Extension Version
.XS
\*(SN Determining The Extension Version
.XE
.LP
.DS
\fC
XExtensionVersion
*XGetExtensionVersion (display, name)
        Display *display;
        char    *name;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIname\fP"
Specifies the name of the desired extension.
.in -.5i
.RE
.LP
This function allows a client to determine if a server supports
the desired version of the input extension.
.LP
The \fBXExtensionVersion\fP structure returns information about the
version of the extension supported by the server.  The structure is
defined as follows:
.LP
.DS
\fC
typedef struct
    {
    Bool  present;   
    short major_version;
    short minor_version;
    } XExtensionVersion;
\fP
.DE
.LP
The major and minor versions can be compared with constants defined in
the header file \fBXI.h\fP.  Each version is a superset of the previous
versions.
.LP
You should use \fBXFree\fP to free the data returned by this function.
.NH 3
Listing Available Devices
.XS
\*(SN Listing Available Devices
.XE
.LP
A client program that wishes to access a specific device
must first determine whether that device is connected to the X server.  This
is done through the
\fBXListInputDevices\fP function, which will return a list of all devices that
can be opened by the X server.  The client program can use one
of the names defined in the \fBXI.h\fP header file in an XInternAtom 
request, to determine the device type of the desired device.  This type 
can then be compared with the device types returned by the 
\fBXListInputDevices\fP request.
.LP
.DS
\fC
XDeviceInfo 
*XListInputDevices (display, ndevices)
        Display *display;
        int     *ndevices;            /* RETURN */
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIndevices\fP"
Specifies the address of a variable into which the server
can return the number of input devices available to the X server.
.in -.5i
.RE
.LP
This function allows a client to determine which devices 
are available for X input and information about those devices.
An array of \fBXDeviceInfo\fP structures is returned, with one
element in the array for each device.  The number of devices
is returned in the \fBndevices\fP argument.
.LP
The X pointer device and X keyboard device are reported, as well as
all available extension input devices.  The use field of the 
\fBXDeviceInfo\fP structure specifies the current use of the device.
If the value of this field is \fBIsXPointer\fP, the device is the 
X pointer device.  If the value is \fBIsXKeyboard\fP, the device is
the X keyboard device.  If the value is \fBIsXExtensionDevice\fP, the
device is available for use as an extension input device.
.LP
Each \fBXDeviceInfo\fP entry contains a pointer to a list of
structures that describe the characteristics of each class
of input supported by that device.  The num_classes field
contains the number of entries in that list.
.LP
If the device supports input class \fBValuators\fP,
one of the structures pointed to by 
the \fBXDeviceInfo\fP structure will be an
\fBXValuatorInfo\fP structure.  The axes field of that structure
contains the address of an array of \fBXAxisInfo\fP structures.
There is one element in this array for each axis of motion
reported by the device.  The number of elements in this 
array is contained in the num_axes element of the 
\fBXValuatorInfo\fP structure.
The size of the motion buffer for the device is
reported in the motion_buffer field of the \fBXValuatorInfo\fP
structure.
.LP
The \fBXDeviceInfo\fP structure contains the following information:
.DS
\fC
typedef struct _XDeviceInfo
    {
    XID                 id;        
    Atom                type;
    char                *name;
    int                 num_classes;
    int                 use;
    XAnyClassPtr 	inputclassinfo;
    } XDeviceInfo;
\fP
.DE
.LP
The structures pointed to by the \fBXDeviceInfo\fP structure contain
the following information:
.DS
\fC
typedef struct _XKeyInfo
    {
    XID                 class;
    int                 length;
    unsigned short      min_keycode;
    unsigned short      max_keycode;
    unsigned short      num_keys;
    } XKeyInfo;

typedef struct _XButtonInfo {
    XID	        class;
    int         length;
    short 	num_buttons;
    } XButtonInfo;

typedef struct	_XValuatorInfo
    {
    XID                 class;
    int                 length;
    unsigned char       num_axes;
    unsigned char       mode;
    unsigned long       motion_buffer;
    XAxisInfoPtr        axes;
    } XValuatorInfo;
\fP
.DE
.LP
The \fBXAxisInfo\fP structure pointed to by the \fBXValuatorInfo\fP structure 
contains the following information.
.DS
\fC
typedef struct _XAxisInfo {
    int 	resolution;
    int 	min_value;
    int 	max_value;
    } XAxisInfo;
\fP
.DE
.LP
The following atom names are defined in the file \fBXI.h\fP:
.DS
\fC
\fBMOUSE\fP	
\fBTABLET\fP
\fBKEYBOARD\fP	
\fBTOUCHSCREEN\fP
\fBTOUCHPAD\fP	
\fBBUTTONBOX\fP	
\fBBARCODE\fP
\fBKNOB_BOX\fP
\fBTRACKBALL\fP	
\fBQUADRATURE\fP
\fBSPACEBALL\fP
\fBDATAGLOVE\fP
\fBEYETRACKER\fP
\fBCURSORKEYS\fP
\fBFOOTMOUSE\fP
\fBID_MODULE\fP
\fBONE_KNOB\fP
\fBNINE_KNOB\fP
\fP
.DE
.LP
These names can be used in an \fBXInternAtom\fP request to return an atom that
can be used for comparison with the type field of the 
\fBXDeviceInfo\fP structure.
.LP
This function returns NULL if there are no input devices to list.
You should use \fBXFreeDeviceList\fP to free the data returned by 
\fBXListInputDevices\fP.  
.LP
.DS 
\fC 
void
XFreeDeviceList (list) 
	XDeviceInfo *list; 
\fP
.DE
.LP
.RS
.in +5n
.IP "\fIlist\fP"
Specifies the pointer to the \fBXDeviceInfo\fP array returned by
a previous call to \fBXListInputDevices\fP.
.in -5n
.RE
.LP
This function frees the list of input device information.
.NH 3
Enabling And Disabling Extension Devices
.XS
\*(SN Enabling And Disabling Extension Devices
.XE
.LP
Each
client program that wishes to access an extension device must request that
the server open that device.  This is done via the \fBXOpenDevice\fP
request.  That request is defined as follows:
.LP
.DS
\fC
XDevice 
*XOpenDevice(display, device_id)
        Display *display;
        XID     device_id;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice_id\fP"
Specifies the ID that uniquely identifies the device to be opened.  This ID
is obtained from the \fBXListInputDevices\fP request.
.in -.5i
.RE
.LP
This function opens the device for the requesting client and returns an
\fBXDevice\fP structure on success.  That structure is defined as follows:
.LP
.DS
\fC
typedef struct {
        XID                    device_id;
        int                    num_classes;
        XInputClassInfo        *classes;
} XDevice;
\fP
.DE
.LP
The \fBXDevice\fP structure contains a pointer to 
an array of \fBXInputClassInfo\fP structures.  Each element in that array
contains information about events of a particular input class supported
by the input device.
.LP
The XInputClassInfo structure is defined as follows:
.LP
.DS
\fC
typedef struct {
        unsigned char input_class;
        unsigned char event_type_base;
} XInputClassInfo;
\fP
.DE
.LP
A client program can determine the event
type and event class for a given event by using macros defined by the 
input extension.  The name of the macro corresponds to the desired event,
and the macro is passed the structure that describes the device from which
input is desired, i.e.
.DS
\fC
	DeviceKeyPress (XDevice *device, event_type, event_class)
\fP
.DE
.LP
The macro will fill in the values of the event class to be used in an
\fBXSelectExtensionEvent\fP request to select the event, and the event
type to be used in comparing with the event types of events received
via \fBXNextEvent\fP.
.LP
Errors returned by this function:  \fBBadDevice\fP.
.sp 2
.LP
Before terminating, the client program should request that the server close
the device. This is done via the \fBXCloseDevice\fP request.
.LP
A client may open the same extension device more than once.  Requests
after the first successful one return an additional XDevice structure
with the same information as the first, but otherwise have no effect.
A single \fBXCloseDevice\fP request will terminate that client's access to
the device.
.LP
Closing a device releases any active or passive grabs the requesting client
has established.  If the device is frozen only by an active grab of the
requesting client, any queued events are released.
.LP
If a client program terminates without closing a device, the server will
automatically close that device on behalf of the client.  This does not
affect any other clients that may be accessing that device.
.DS
\fC
int
XCloseDevice(display, device)
        Display *display;
        XDevice *device;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the device to be closed.
.in -.5i
.RE	
.LP
This function closes the device for the requesting client, and frees
the \fBXDevice\fP structure.
.LP
Errors returned by this function:  \fBBadDevice\fP.
.NH 3
Changing The Mode Of A Device
.XS
\*(SN Changing The Mode Of A Device
.XE
.LP
Some devices are capable of reporting either relative or absolute motion
data.  To change the mode of a device from relative to absolute, use the
\fBXSetDeviceMode\fP function.  The valid values are \fBAbsolute\fP or 
\fBRelative\fP.
.DS
\fC
int
XSetDeviceMode (display, device, mode)
        Display *display;
        XDevice *device;
        int     mode;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the device whose mode should be changed.
.IP "\fImode\fP"
Specifies the mode.  You can specify one of these constants:
\fBAbsolute\fP or \fBRelative\fP.
.in -.5i
.RE	
.LP
This function allows a client to request the server to change the mode of a 
device that is capable of reporting either absolute positional data or relative
motion data.  If the device is invalid, or the client has not previously 
requested that the server open the device via an \fBXOpenDevice\fP request, 
this request will fail with a \fBBadDevice\fP error.
If the device does not support input class \fBValuators\fP, or if it is not
capable of reporting the specified mode, the request will fail with a 
\fBBadMatch\fP error.
.LP
This request will fail and return \fBDeviceBusy\fP if another client has 
already opened the device and requested a different mode.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadMatch\fP,
\fBBadMode\fP, \fBDeviceBusy\fP.
.NH 3
Initializing Valuators on an Input Device
.XS
\*(SN Initializing Valuators on an Input Device
.XE
.LP
Some devices that report absolute positional data can be initialized to a 
starting value.  Devices that are capable of reporting relative motion or
absolute positional data may require that their valuators be initialized 
to a starting value after the mode of the device is changed to \fBAbsolute\fP.
To initialize the valuators on such a device, use the \fBXSetDeviceValuators\fP
function.
.DS
\fC
Status
XSetDeviceValuators (display, device, valuators, first_valuator, 
        num_valuators)
        Display *display;
        XDevice *device;
        int     *valuators, first_valuator, num_valuators;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the device whose valuators should be initialized.
.IP "\fIvaluators\fP"
Specifies the values to which each valuator should be set.
.IP "\fIfirst_valuator\fP"
Specifies the first valuator to be set.
.IP "\fInum_valuators\fP"
Specifies the number of valuators to be set.
.in -.5i
.RE
.LP
This function initializes the specified valuators on the specified extension
input device.  Valuators are numbered beginning with zero.  Only the valuators
in the range specified by first_valuator and num_valuators are set.  If the
number of valuators supported by the device is less than the expression
.DS
\fC
      first_valuator + num_valuators, 
\fP
.DE
a \fBBadValue\fP error will result.
.LP
If the request succeeds, \fPSuccess\fB is returned.  If the specifed device is 
grabbed by some other client, the request will fail and a status of
\fBAlreadyGrabbed\fP will be returned.
.LP
This request can fail with \fBBadLength\fP, \fBBadDevice\fP, \fBBadMatch\fP,
and \fBBadValue\fP errors.
.NH 3 
Getting Input Device Controls
.XS
\*(SN Getting Input Device Controls
.XE
.LP
Some input devices support various configuration controls
that can be queried or changed by clients.  The set of supported
controls will vary from one input device to another.  Requests
to manipulate these controls will fail if either the target
X server or the target input device does not support the 
requested device control.
.LP
Each device control has a unique identifier.  Information
passed with each device control varies in length and is mapped
by data structures unique to that device control.
.LP
To query a device control use XGetDeviceControl.  
.LP     
.DS
\fC
XDeviceControl
*XGetDeviceControl (display, device, control)
	Display *display;
	XDevice *device;
	int control;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the device whose configuration control status is to be returned.
.IP "\fIcontrol\fP"
Identifies the specific device control to be queried.
.in -.5i
.RE
.LP
This request returns the current state of the specified device 
control.  If the target X server does not support that device
control, a BadValue error will be returned.  If the specified
device does not support that device control, a BadMatch error
will be returned.
.LP
If the request is successful, a pointer to a generic 
XDeviceState structure is returned.  The information 
returned varies according to the specified control and is 
mapped by a structure appropriate for that control.  The first
two fields are common to all device controls:
.LP
.DS
\fC
typedef struct {
      XID     control;
      int     length;
} XDeviceState;
\fP
.DE
.LP
The control may be compared to constants defined in the file
XI.h.  Currently defined device controls include DEVICE_RESOLUTION.
.LP
The information returned for the DEVICE_RESOLUTION control is
defined in the following structure:
include:
.LP
.DS
\fC
typedef struct {
      XID     control;
      int     length;
      int     num_valuators;
      int     *resolutions;
      int     *min_resolutions;
      int     *max_resolutions;
} XDeviceResolutionState;
\fP
.DE
.LP
This device control returns a list of valuators and the range of 
valid resolutions allowed for each.  Valuators are numbered 
beginning with 0.  Resolutions for all valuators on the device are 
returned.  For each valuator i on the device, resolutions[i] returns 
the current setting of the resolution, min_resolutions[i] returns 
the minimum valid setting, and max_resolutions[i] returns the 
maximum valid setting.
.LP
When this control is specified, XGetDeviceControl will fail with
a BadMatch error if the specified device has no valuators.
.LP
Other errors returned by this request:  BadValue.
.NH 3 
Changing Input Device Controls
.XS
\*(SN Changing Input Device Controls
.XE
.LP
Some input devices support various configuration controls
that can be changed by clients.  Typically, this would be
done to initialize the device to a known state or configuration.
The set of supported controls will vary from one input device 
to another.  Requests to manipulate these controls will fail if 
either the target X server or the target input device does not 
support the requested device control.  Setting the device control 
will also fail if the target input device is grabbed by another 
client, or is open by another client and has been set to a conflicting
state.
.LP
Each device control has a unique identifier.  Information
passed with each device control varies in length and is mapped
by data structures unique to that device control.
.LP
To change a device control use XChangeDeviceControl.  
.LP     
.DS
\fC
Status
XChangeDeviceControl (display, device, control, value)
	Display *display;
	XDevice *device;
	int control;
	XDeviceControl *value;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .80i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the device whose configuration control status is to be modified.
.IP "\fIcontrol\fP"
Identifies the specific device control to be changed.
.IP "\fIvalue\fP"
Specifies a pointer to an XDeviceControl structure that describes which 
control is to be changed, and how it is to be changed.
.in -.5i
.RE
.LP
This request changes the current state of the specified device 
control.  If the target X server does not support that device
control, a BadValue error will be returned.  If the specified
device does not support that device control, a BadMatch error
will be returned.  If another client has the target device 
grabbed, a status of AlreadyGrabbed will be returned.  If 
another client has the device open and has set it to a 
conflicting state, a status of DeviceBusy will be returned.
.LP
If the request fails for any reason, the device control will not
be changed.
.LP
If the request is successful, the device control will be changed
and a status of Success will be returned.  The information passed
varies according to the specified control and is mapped by a 
structure appropriate for that control.  The first two fields are 
common to all device controls:
.LP
.DS
\fC
typedef struct {
      XID     control;
      int     length;
} XDeviceControl;
\fP
.DE
.LP
The control may be set using constants defined in the file XI.h.  
Currently defined device controls include DEVICE_RESOLUTION.
.LP
The information that can be changed by the DEVICE_RESOLUTION 
control is defined in the following structure:
.LP
.DS
\fC
typedef struct {
      XID     control;
      int     length;
      int     first_valuator;
      int     num_valuators;
      int     *resolutions;
} XDeviceResolutionControl;
\fP
.DE
.LP
This device control changes the resolution of the specified 
valuators on the specified extension input device.  Valuators 
are numbered beginning with zero.  Only the valuators in the range 
specified by first_valuator and num_valuators are set.  A value 
of -1 in the resolutions list indicates that the resolution for 
this valuator is not to be changed.  num_valuators specifies the 
number of valuators in the resolutions list.
.LP
When this control is specified, XChangeDeviceControl will fail with 
a BadMatch error if the specified device has no valuators.  If a 
resolution is specified that is not within the range of valid values 
(as returned by XGetDeviceControl) the request will fail with a 
BadValue error.  If the number of valuators supported by the device 
is less than the expression 
.LP   
.DS
\fC
      first_valuator + num_valuators, 
\fP
.DE
.LP  
a BadValue error will result.
.LP  
.NH 3
Selecting Extension Device Events
.XS
\*(SN Selecting Extension Device Events
.XE
.LP
Device input events are selected using the \fBXSelectExtensionEvent\fP
function.
The parameters passed are a pointer to 
a list of classes that define the desired event types and devices, a count
of the number of elements in the list, and the id of the window from which 
events are desired.
.DS
\fC
int
XSelectExtensionEvent (display, window, event_list, event_count)
        Display     *display;
        Window      window;
        XEventClass *event_list;
        int         event_count;
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIwindow\fP"
Specifies the ID of the window from which the client wishes to receive events.
.IP "\fIevent_list\fP"
Specifies a pointer to a list of XEventClasses that specify which events are
desired.
.IP "\fIevent_count\fP"
Specifies the number of elements in the event_list.
.in -.5i
.RE	
.LP
This function requests the server to send events that match the events and
devices described by the event list and that come from the requested 
window.  
The elements of the XEventClass array are the event_class values
returned obtained by invoking a macro with the pointer to a Device
structure returned by the \fBXOpenDevice\fP request.
For example, the DeviceKeyPress macro, invoked in the form:
.DS
\fC
	DeviceKeyPress (XDevice *device, event_type, event_class)
\fP
.DE
.LP
returns the XEventClass for DeviceKeyPress events from the specified device.
.LP
Macros are defined for the following event classes:  \fBDeviceKeyPress,
DeviceKeyRelease, DeviceButtonPress, DeviceButtonRelease, DeviceMotionNotify,
DeviceFocusIn, DeviceFocusOut, ProximityIn, ProximityOut, DeviceStateNotify,
DeviceMappingNotify, ChangeDeviceNotify, DevicePointerMotionHint,
DeviceButton1Motion, DeviceButton2Motion, DeviceButton3Motion, 
DeviceButton4Motion, DeviceButton5Motion, DeviceButtonMotion,
DeviceOwnerGrabButton,\fP and \fBDeviceButtonPressGrab\fP.
To get the next available event from within a client program, use the 
core \fBXNextEvent\fP function.  This returns the next event whether it
came from a core device or an extension device.
.LP
Succeeding XSelectExtensionEvent requests using XEventClasses for the same 
device as was specified on a previous request will replace the previous
set of selected events from that device with the new set.
.LP
Errors returned by this function:  \fBBadWindow\fP, \fBBadAccess\fP, 
\fBBadClass\fP, \fBBadLength\fP.
.NH 3
Determining Selected Device Events
.XS
\*(SN Determining Selected Device Events
.XE
.LP
To determine which extension events are currently selected from a given
window, use \fBXGetSelectedExtensionEvents\fP.
.DS
\fC
int
XGetSelectedExtensionEvents (display, window, this_client_count, 
        this_client, all_clients_count, all_clients)
        Display     *display;
        Window      window;
        int         *this_client_count;  /* RETURN */
        XEventClass **this_client;       /* RETURN */
        int         *all_clients_count;  /* RETURN */
        XEventClass **all_clients;       /* RETURN */
\fP
.DE
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIwindow\fP"
Specifies the ID of the window from which the client wishes to receive events.
.IP "\fIthis_client_count\fP"
Specifies the number of elements in the this_client list.
.IP "\fIthis_client\fP"
Specifies a pointer to a list of XEventClasses that specify which events are
selected by this client.
.IP "\fIall_clients_count\fP"
Specifies the number of elements in the all_clients list.
.IP "\fIall_clients\fP"
Specifies a pointer to a list of XEventClasses that specify which events are
selected by all clients.
.in -.5i
.RE	
.LP
This function returns pointers to two event class arrays.  
One lists the extension events selected by this client from
the specified window.  The other lists the extension events selected by
all clients from the specified window.  This information is analogous
to that returned in the fields your_event_mask and all_event_masks of the
\fBXWindowAttributes\fP structure when an 
\fBXGetWindowAttributes\fP request is made.
.LP
You should use \fBXFree\fP to free the two arrays returned by this function.
.LP
Errors returned by this function: \fBBadWindow\fP.
.NH 3
Controlling Event Propagation
.XS
\*(SN Controlling Event Propagation
.XE
.LP
Extension events propagate up the window hierarchy in the same manner
as core events.  If a window is not interested in an extension event, 
it usually propagates to the closest ancestor that is interested,
unless the dont_propagate list prohibits it.
Grabs of extension devices may alter the set of windows that receive a 
particular extension event.
.LP
Client programs may control event propagation through the use
of the following two functions.  
.LP
\fBXChangeDeviceDontPropagateList\fP adds an event to or deletes an event from 
the do_not_propagate list of extension events for the specified window.
There is one list per window, and the list remains for the life of the window.
The list is not altered if a client that changed the list terminates.
.LP
Suppression of event propagation is not allowed for all events.  If a
specified XEventClass is invalid because suppression of that event is not
allowed, a \fBBadClass\fP error will result.
.LP
.DS
\fC
int
XChangeDeviceDontPropagateList (display, window, event_count, 
        events, mode)
	Display     *display;
	Window      window;
	int         event_count;
	XEventClass *events;
	int         mode;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIwindow\fP"
Specifies the desired window.
.IP "\fIevent_count\fP"
Specifies the number of elements in the events list.
.IP "\fIevents\fP"
Specifies a pointer to the list of XEventClasses.
.IP "\fImode\fP"
Specifies the mode.  You may use the constants \fBAddToList\fP or
\fBDeleteFromList\fP.
.in -.5i
.RE
.LP
This function can return \fBBadWindow\fP, \fBBadClass\fP, and
\fBBadMode\fP errors.
.sp 2
.LP
\fBXGetDeviceDontPropagateList\fP allows a client to determine the 
do_not_propagate list of extension events for the specified window.
.LP
.DS
\fC
XEventClass
*XGetDeviceDontPropagateList (display, window, event_count)
	Display *display;
	Window  window;
	int     *event_count;	/*RETURN */
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIwindow\fP"
Specifies the desired window.
.IP "\fIevent_count\fP"
Specifies the number of elements in the array returned by this function.
.in -.5i
.RE
.LP
An array of \fBXEventClasses\fP is returned by this function.
Each XEventClass represents a device/ event type pair.
.LP
This function can return a \fBBadWindow\fP error.
.LP
You should use \fBXFree\fP to free the data returned by this function.
.NH 3
Sending An Event
.XS
\*(SN Sending An Event
.XE
.LP
\fBXSendExtensionEvent\fP allows a client to send an extension event
to another client.
.LP
.DS
\fC
int
XSendExtensionEvent (display, device, window, propagate, 
        event_count, event_list, event)
	Display     *display;
	XDevice     *device;
	Window      window;
	Bool        propagate;
	int         event_count;
	XEventClass *event_list;
	XEvent      *event;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the device whose ID is recorded in the event.
.IP "\fIwindow\fP"
Specifies the destination window ID.  You can pass a window ID,
\fBPointerWindow\fP or \fBInputFocus\fP.
.IP "\fIpropagate\fP"
Specifies a boolean value that is either True or False.
.IP "\fIevent_count\fP"
Specifies the number of elements in the event_list array.
.IP "\fIevent_list\fP"
Specifies a pointer to an array of XEventClasses.
.IP "\fIevent\fP"
Specifies a pointer to the event that is to be sent.
.in -.5i
.RE
.LP
The XSendExtensionEvent function identifies the destination window,
determines which clients should receive the specified event, and ignores
any active grabs.  This function requires a list of XEventClasses to be
specified.  These are obtained by opening an input device with the
XOpenDevice request.
.LP
This function uses the \fBwindow\fP argument to identify the destination
window as follows:
.IP \(bu 3n
If you pass \fBPointerWindow\fP, the destination window is the window
that contains the pointer.
.IP \(bu 3n
If you pass \fBInputFocus\fP, and if the focus window contains the pointer,
the destination window is the window that contains the pointer.  If the 
focus window does not contain the pointer, the destination window is the 
focus window.
.LP
To determine which clients should receive the specified events, 
\fBXSendExtensionEvent\fP uses the propagate argument as follows:
.IP \(bu 3n
If propagate is \fBFalse\fP, the event is sent to every client selecting
from the destination window
any of the events specified in the event_list array.
.IP \(bu 3n
If propagate is \fBTrue\fP, and no clients have selected
from the destination window
any of the events specified in the event_list array, the destination is 
replaced with the closest ancestor of destination for which some client
has selected one of the specified events, and for which no intervening
window has that event in its do_not_propagate mask.  If no such window
exists, or if the window is an ancestor of the focus window, and 
\fBInputFocus\fP was originally specified as the destination, the event
is not sent to any clients.  Otherwise, the event is reported to every
client selecting on the final destination any of the events specified
in event_list.
.LP
The event in the \fBXEvent\fP structure must be one of the events defined
by the input extension, so that the X server can correctly byte swap the
contents as necessary.  The contents of the event are otherwise unaltered
and unchecked by the X server except to force send_event to \fBTrue\fP
in the forwarded event and to set the sequence number in the event correctly.
.LP
XSendExtensionEvent returns zero if the conversion-to-wire protocol
failed, otherwise it returns nonzero.
.LP
This function can generate \fBBadDevice\fP, \fBBadValue\fP, \fBBadWindow\fP, 
or \fBBadClass\fP errors.
.NH 3
Getting Motion History
.XS
\*(SN Getting Motion History
.XE
.LP
.DS
\fC
XDeviceTimeCoord 
*XGetDeviceMotionEvents (display, device, start, stop, 
        nevents_return, mode_return, axis_count_return);
        Display *display;
        XDevice *device;
        Time    start, stop;
        int     *nevents_return;
        int     *mode_return;
        int     *axis_count_return;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIdisplay\fP" .75i
Specifies the connection to the X server.
.IP "\fIdevice\fP"
Specifies the desired device.
.IP "\fIstart\fP"
Specifies the start time.
.IP "\fIstop\fP"
Specifies the stop time.
.IP "\fInevents_return\fP"
Specifies the address of a variable into which the server
will return the number of positions in the motion buffer returned
for this request.
.IP "\fImode_return\fP"
Specifies the address of a variable into which the server
will return the mode of the nevents information.  The mode will be
one of the following: \fBAbsolute\fP or \fBRelative\fP.
.IP "\fIaxis_count_return\fP"
Specifies the address of a variable into which the server
will return the number of axes reported in each of the positions returned.
.in -.5i
.RE
.LP
This function returns all positions in the device's motion history buffer
that fall between the specified start and stop times inclusive.  If the
start time is in the future, or is later than the stop time, no positions
are returned.
.LP
The return type for this function is a structure defined as follows:
.DS
\fC
typedef struct {
        Time time;
        unsigned int *data;
} XDeviceTimeCoord;
\fP
.DE
.LP
The data field of the \fBXDeviceTimeCoord\fP structure is a pointer to an 
array of data items.  Each item is of type int, and there is one data item
per axis of motion reported by the device.  The number of axes reported
by the device is returned in the axis_count variable.
.LP
The value of the data items depends on the mode of the device. 
The mode is returned in the mode variable.  If the 
mode is \fBAbsolute\fP, the data items are the raw values generated by
the device.  These may be scaled by the client program using the
maximum values that the device can generate for each axis of motion
that it reports.  The maximum value for each axis is reported in
the max_val field of the \fBXAxisInfo\fP structure.  This structure is 
part of the information returned by the \fBXListInputDevices\fP request.
.LP
If the mode is \fBRelative\fP, the data items are the relative values
generated by the device.  The client program must choose an initial
position for the device and maintain a current position by 
accumulating these relative values.
.LP
Consecutive calls to this function may return data of different modes, if
some client program has changed the mode of the device via an
\fBXSetDeviceMode\fP request.
.LP
You should use \fBXFreeDeviceMotionEvents\fP to free the data returned by this 
function.
.LP
Errors returned by this function:  \fBBadDevice\fP, \fBBadMatch\fP.
.LP
.DS
\fC
void
XFreeDeviceMotionEvents (events)
        XDeviceTimeCoord *events;
\fP
.DE
.LP
.RS
.in +.5i
.IP "\fIevents\fP"
Specifies the pointer to the \fBXDeviceTimeCoord\fP array returned by
a previous call to \fBXGetDeviceMotionEvents\fP.
.in -.5i
.RE
.LP
This function frees the array of motion information.
.\"
.\"
.\"  Appendicies
.\"
.\"
.bp
.ds Ch ~
.nr H1 1
.LP
The following information is contained in the \fB<X11/extensions/XInput.h>\fP
and \fB<X11/extensions/XI.h>\fP header files:
.DS 0
.cs CW 20
\fC
.ps 8
/* Definitions used by the library and client */

#ifndef _XINPUT_H_
#define _XINPUT_H_

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef _XI_H_
#include "XI.h"
#endif

#define _deviceKeyPress		0
#define _deviceKeyRelease	1

#define _deviceButtonPress	0
#define _deviceButtonRelease	1

#define _deviceMotionNotify	0

#define _deviceFocusIn		0
#define _deviceFocusOut		1

#define _proximityIn		0
#define _proximityOut		1

#define _deviceStateNotify	0
#define _deviceMappingNotify	1
#define _changeDeviceNotify	2

#define FindTypeAndClass(d, type, class, classid, offset) \
    { int i; XInputClassInfo *ip; \
    type = 0; class = 0; \
    for (i=0, ip= ((XDevice *) d)->classes; \
	 i< ((XDevice *) d)->num_classes; \
	 i++, ip++) \
	if (ip->input_class == classid) \
	    {type =  ip->event_type_base + offset; \
	     class =  ((XDevice *) d)->device_id << 8 | type;}}

#define DeviceKeyPress(d, type, class) \
    FindTypeAndClass(d, type, class, KeyClass, _deviceKeyPress)

#define DeviceKeyRelease(d, type, class) \
    FindTypeAndClass(d, type, class, KeyClass, _deviceKeyRelease)

#define DeviceButtonPress(d, type, class) \
    FindTypeAndClass(d, type, class, ButtonClass, _deviceButtonPress)

#define DeviceButtonRelease(d, type, class) \
    FindTypeAndClass(d, type, class, ButtonClass, _deviceButtonRelease)

#define DeviceMotionNotify(d, type, class) \
    FindTypeAndClass(d, type, class, ValuatorClass, _deviceMotionNotify)

#define DeviceFocusIn(d, type, class) \
    FindTypeAndClass(d, type, class, FocusClass, _deviceFocusIn)

#define DeviceFocusOut(d, type, class) \
    FindTypeAndClass(d, type, class, FocusClass, _deviceFocusOut)

#define ProximityIn(d, type, class) \
    FindTypeAndClass(d, type, class, ProximityClass, _proximityIn)

#define ProximityOut(d, type, class) \
    FindTypeAndClass(d, type, class, ProximityClass, _proximityOut)

#define DeviceStateNotify(d, type, class) \
    FindTypeAndClass(d, type, class, OtherClass, _deviceStateNotify)

#define DeviceMappingNotify(d, type, class) \
    FindTypeAndClass(d, type, class, OtherClass, _deviceMappingNotify)

#define ChangeDeviceNotify(d, type, class) \
    FindTypeAndClass(d, type, class, OtherClass, _changeDeviceNotify)

#define DevicePointerMotionHint(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _devicePointerMotionHint;}

#define DeviceButton1Motion(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButton1Motion;}

#define DeviceButton2Motion(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButton2Motion;}

#define DeviceButton3Motion(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButton3Motion;}

#define DeviceButton4Motion(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButton4Motion;}

#define DeviceButton5Motion(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButton5Motion;}

#define DeviceButtonMotion(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButtonMotion;}

#define DeviceOwnerGrabButton(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceOwnerGrabButton;}

#define DeviceButtonPressGrab(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _deviceButtonGrab;}

#define NoExtensionEvent(d, type, class) \
    { class =  ((XDevice *) d)->device_id << 8 | _noExtensionEvent;}

#define BadDevice(dpy, error) _xibaddevice(dpy, &error)

#define BadClass(dpy, error) _xibadclass(dpy, &error)

#define BadEvent(dpy, error) _xibadevent(dpy, &error)

#define BadMode(dpy, error) _xibadmode(dpy, &error)

#define DeviceBusy(dpy, error) _xidevicebusy(dpy, &error)

/***************************************************************
 *
 * DeviceKey events.  These events are sent by input devices that
 * support input class Keys.
 * The location of the X pointer is reported in the coordinate
 * fields of the x,y and x_root,y_root fields.
 *
 */

typedef struct 
    {
    int            type;         /* of event */
    unsigned long  serial;       /* # of last request processed */
    Bool           send_event;   /* true if from SendEvent request */
    Display        *display;     /* Display the event was read from */
    Window         window;       /* "event" window reported relative to */
    XID            deviceid;
    Window         root;         /* root window event occured on */
    Window         subwindow;    /* child window */
    Time           time;         /* milliseconds */
    int            x, y;         /* x, y coordinates in event window */
    int            x_root;       /* coordinates relative to root */
    int            y_root;       /* coordinates relative to root */
    unsigned int   state;        /* key or button mask */
    unsigned int   keycode;      /* detail */
    Bool           same_screen;  /* same screen flag */
    unsigned int   device_state; /* device key or button mask */
    unsigned char  axes_count;
    unsigned char  first_axis;
    int            axis_data[6];
    } XDeviceKeyEvent;

typedef XDeviceKeyEvent XDeviceKeyPressedEvent;
typedef XDeviceKeyEvent XDeviceKeyReleasedEvent;

/*******************************************************************
 *
 * DeviceButton events.  These events are sent by extension devices
 * that support input class Buttons.
 *
 */

typedef struct {
    int           type;         /* of event */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    Window        window;       /* "event" window reported relative to */
    XID           deviceid;
    Window        root;         /* root window that the event occured on */
    Window        subwindow;    /* child window */
    Time          time;         /* milliseconds */
    int           x, y;         /* x, y coordinates in event window */
    int           x_root;       /* coordinates relative to root */
    int           y_root;       /* coordinates relative to root */
    unsigned int  state;        /* key or button mask */
    unsigned int  button;       /* detail */
    Bool          same_screen;  /* same screen flag */
    unsigned int  device_state; /* device key or button mask */
    unsigned char axes_count;
    unsigned char first_axis;
    int           axis_data[6];
    } XDeviceButtonEvent;

typedef XDeviceButtonEvent XDeviceButtonPressedEvent;
typedef XDeviceButtonEvent XDeviceButtonReleasedEvent;

/*******************************************************************
 *
 * DeviceMotionNotify event.  These events are sent by extension devices
 * that support input class Valuators.
 *
 */

typedef struct 
    {
    int           type;        /* of event */
    unsigned long serial;      /* # of last request processed by server */
    Bool          send_event;  /* true if from a SendEvent request */
    Display       *display;    /* Display the event was read from */
    Window        window;      /* "event" window reported relative to */
    XID           deviceid;
    Window        root;        /* root window that the event occured on */
    Window        subwindow;   /* child window */
    Time          time;        /* milliseconds */
    int           x, y;        /* x, y coordinates in event window */
    int           x_root;      /* coordinates relative to root */
    int           y_root;      /* coordinates relative to root */
    unsigned int  state;       /* key or button mask */
    char          is_hint;     /* detail */
    Bool          same_screen; /* same screen flag */
    unsigned int  device_state; /* device key or button mask */
    unsigned char axes_count;
    unsigned char first_axis;
    int           axis_data[6];
    } XDeviceMotionEvent;

/*******************************************************************
 *
 * DeviceFocusChange events.  These events are sent when the focus
 * of an extension device that can be focused is changed.
 *
 */

typedef struct 
    {
    int           type;       /* of event */
    unsigned long serial;     /* # of last request processed by server */
    Bool          send_event; /* true if from a SendEvent request */
    Display       *display;   /* Display the event was read from */
    Window        window;     /* "event" window reported relative to */
    XID           deviceid;
    int           mode;       /* NotifyNormal, NotifyGrab, NotifyUngrab */
    int           detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior, 
	 * NotifyNonLinear,NotifyNonLinearVirtual, NotifyPointer,
	 * NotifyPointerRoot, NotifyDetailNone 
	 */
    Time                time;
    } XDeviceFocusChangeEvent;

typedef XDeviceFocusChangeEvent XDeviceFocusInEvent;
typedef XDeviceFocusChangeEvent XDeviceFocusOutEvent;

/*******************************************************************
 *
 * ProximityNotify events.  These events are sent by those absolute
 * positioning devices that are capable of generating proximity information.
 *
 */

typedef struct 
    {
    int             type;      /* ProximityIn or ProximityOut */        
    unsigned long   serial;    /* # of last request processed by server */
    Bool            send_event; /* true if this came from a SendEvent request */
    Display         *display;  /* Display the event was read from */
    Window          window;      
    XID	            deviceid;
    Window          root;            
    Window          subwindow;      
    Time            time;            
    int             x, y;            
    int             x_root, y_root;  
    unsigned int    state;           
    Bool            same_screen;     
    unsigned int    device_state; /* device key or button mask */
    unsigned char   axes_count;
    unsigned char   first_axis;
    int             axis_data[6];
    } XProximityNotifyEvent;
typedef XProximityNotifyEvent XProximityInEvent;
typedef XProximityNotifyEvent XProximityOutEvent;

/*******************************************************************
 *
 * DeviceStateNotify events are generated on EnterWindow and FocusIn 
 * for those clients who have selected DeviceState.
 *
 */

typedef struct
    {
    unsigned char	class;
    unsigned char	length;
    } XInputClass;

typedef struct {
    int           type;
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    Window        window;
    XID           deviceid;
    Time          time;
    int           num_classes;
    char	  data[64];
} XDeviceStateNotifyEvent;	

typedef struct {
    unsigned char	class;
    unsigned char	length;
    unsigned char	num_valuators;
    unsigned char	mode;
    int        		valuators[6];
} XValuatorStatus;

typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_keys;
    char        	keys[32];
} XKeyStatus;

typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_buttons;
    char        	buttons[32];
} XButtonStatus;

/*******************************************************************
 *
 * DeviceMappingNotify event.  This event is sent when the key mapping,
 * modifier mapping, or button mapping of an extension device is changed.
 *
 */

typedef struct {
    int           type;
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    Window        window;       /* unused */
    XID           deviceid;
    Time          time;
    int           request;      /* one of MappingModifier, MappingKeyboard,
                                    MappingPointer */
    int           first_keycode;/* first keycode */
    int           count;        /* defines range of change w. first_keycode*/
} XDeviceMappingEvent;

/*******************************************************************
 *
 * ChangeDeviceNotify event.  This event is sent when an 
 * XChangeKeyboard or XChangePointer request is made.
 *
 */

typedef struct {
    int           type;
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    Window        window;       /* unused */
    XID           deviceid;
    Time          time;
    int           request;      /* NewPointer or NewKeyboard */
} XChangeDeviceNotifyEvent;

/*******************************************************************
 *
 * Control structures for input devices that support input class
 * Feedback.  These are used by the XGetFeedbackControl and 
 * XChangeFeedbackControl functions.
 *
 */

typedef struct {
     XID            class;
     int            length;
     XID            id;
} XFeedbackState;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     click;
    int     percent;
    int     pitch;
    int     duration;
    int     led_mask;
    int     global_auto_repeat;
    char    auto_repeats[32];
} XKbdFeedbackState;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     accelNum;
    int     accelDenom;
    int     threshold;
} XPtrFeedbackState;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     resolution;
    int     minVal;
    int     maxVal;
} XIntegerFeedbackState;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     max_symbols;
    int     num_syms_supported;
    KeySym  *syms_supported;
} XStringFeedbackState;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     percent;
    int     pitch;
    int     duration;
} XBellFeedbackState;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     led_values;
    int     led_mask;
} XLedFeedbackState;

typedef struct {
     XID            class;
     int            length;
     XID	    id;
} XFeedbackControl;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     accelNum;
    int     accelDenom;
    int     threshold;
} XPtrFeedbackControl;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     click;
    int     percent;
    int     pitch;
    int     duration;
    int     led_mask;
    int     led_value;
    int     key;
    int     auto_repeat_mode;
} XKbdFeedbackControl;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     num_keysyms;
    KeySym  *syms_to_display;
} XStringFeedbackControl;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     int_to_display;
} XIntegerFeedbackControl;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     percent;
    int     pitch;
    int     duration;
} XBellFeedbackControl;

typedef struct {
    XID     class;
    int     length;
    XID     id;
    int     led_mask;
    int     led_values;
} XLedFeedbackControl;

/*******************************************************************
 *
 * An array of XDeviceList structures is returned by the 
 * XListInputDevices function.  Each entry contains information
 * about one input device.  Among that information is an array of 
 * pointers to structures that describe the characteristics of 
 * the input device.
 *
 */

typedef struct _XAnyClassinfo *XAnyClassPtr;

typedef struct _XAnyClassinfo {
    XID 	class;
    int 	length;
    } XAnyClassInfo;

typedef struct _XDeviceInfo *XDeviceInfoPtr;

typedef struct _XDeviceInfo
    {
    XID                 id;        
    Atom                type;
    char                *name;
    int                 num_classes;
    int                 use;
    XAnyClassPtr 	inputclassinfo;
    } XDeviceInfo;

typedef struct _XKeyInfo *XKeyInfoPtr;

typedef struct _XKeyInfo
    {
    XID			class;
    int			length;
    unsigned short      min_keycode;
    unsigned short      max_keycode;
    unsigned short      num_keys;
    } XKeyInfo;

typedef struct _XButtonInfo *XButtonInfoPtr;

typedef struct _XButtonInfo {
    XID		class;
    int		length;
    short 	num_buttons;
    } XButtonInfo;

typedef struct _XAxisInfo *XAxisInfoPtr;

typedef struct _XAxisInfo {
    int 	resolution;
    int 	min_value;
    int 	max_value;
    } XAxisInfo;

typedef struct _XValuatorInfo *XValuatorInfoPtr;

typedef struct	_XValuatorInfo
    {
    XID			class;
    int			length;
    unsigned char       num_axes;
    unsigned char       mode;
    unsigned long       motion_buffer;
    XAxisInfoPtr        axes;
    } XValuatorInfo;


/*******************************************************************
 *
 * An XDevice structure is returned by the XOpenDevice function.  
 * It contains an array of pointers to XInputClassInfo structures.
 * Each contains information about a class of input supported by the
 * device, including a pointer to an array of data for each type of event
 * the device reports.
 *
 */


typedef struct {
        unsigned char   input_class;
        unsigned char   event_type_base;
} XInputClassInfo;

typedef struct {
        XID                    device_id;
        int                    num_classes;
        XInputClassInfo        *classes;
} XDevice;


/*******************************************************************
 *
 * The following structure is used to return information for the 
 * XGetSelectedExtensionEvents function.
 *
 */

typedef struct {
        XEventClass     event_type;
        XID             device;
} XEventList;

/*******************************************************************
 *
 * The following structure is used to return motion history data from 
 * an input device that supports the input class Valuators.
 * This information is returned by the XGetDeviceMotionEvents function.
 *
 */

typedef struct {
        Time   time;
        int    *data;
} XDeviceTimeCoord;


/*******************************************************************
 *
 * Device state structure.
 *
 */

typedef struct {
        XID		device_id;
        int		num_classes;
        XInputClass	*data;
} XDeviceState;

typedef struct {
    unsigned char	class;
    unsigned char	length;
    unsigned char	num_valuators;
    unsigned char	mode;
    int        		*valuators;
} XValuatorState;

typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_keys;
    char        	keys[32];
} XKeyState;

typedef struct {
    unsigned char	class;
    unsigned char	length;
    short		num_buttons;
    char        	buttons[32];
} XButtonState;

/*******************************************************************
 *
 * Function definitions.
 *
 */

XDevice                *XOpenDevice();
XDeviceInfo            *XListInputDevices();
XDeviceTimeCoord       *XGetDeviceMotionEvents();
KeySym                 *XGetDeviceKeyMapping();
XModifierKeymap        *XGetDeviceModifierMapping();
XFeedbackState         *XGetFeedbackControl();
XExtensionVersion      *XGetExtensionVersion();
XDeviceState           *XQueryDeviceState();
XEventClass            *XGetDeviceDontPropagateList();
#endif /* _XINPUT_H_ */

/* Definitions used by the server, library and client */

#ifndef _XI_H_

#define _XI_H_

#define sz_xGetExtensionVersionReq		8
#define sz_xGetExtensionVersionReply		32
#define sz_xListInputDevicesReq			4
#define sz_xListInputDevicesReply		32
#define sz_xOpenDeviceReq			8
#define sz_xOpenDeviceReply			32
#define sz_xCloseDeviceReq			8
#define sz_xSetDeviceModeReq			8
#define sz_xSetDeviceModeReply			32
#define sz_xSelectExtensionEventReq		12
#define sz_xGetSelectedExtensionEventsReq	8
#define sz_xGetSelectedExtensionEventsReply	32
#define sz_xChangeDeviceDontPropagateListReq	12
#define sz_xGetDeviceDontPropagateListReq	8
#define sz_xGetDeviceDontPropagateListReply	32
#define sz_xGetDeviceMotionEventsReq		16
#define sz_xGetDeviceMotionEventsReply		32
#define sz_xChangeKeyboardDeviceReq		8
#define sz_xChangeKeyboardDeviceReply		32
#define sz_xChangePointerDeviceReq		8
#define sz_xChangePointerDeviceReply		32
#define sz_xGrabDeviceReq			20
#define sz_xGrabDeviceReply			32
#define sz_xUngrabDeviceReq			12
#define sz_xGrabDeviceKeyReq			20
#define sz_xGrabDeviceKeyReply			32
#define sz_xUngrabDeviceKeyReq			16
#define sz_xGrabDeviceButtonReq			20
#define sz_xGrabDeviceButtonReply		32
#define sz_xUngrabDeviceButtonReq		16
#define sz_xAllowDeviceEventsReq		12
#define sz_xGetDeviceFocusReq			8
#define sz_xGetDeviceFocusReply			32
#define sz_xSetDeviceFocusReq			16
#define sz_xGetFeedbackControlReq		8
#define sz_xGetFeedbackControlReply		32
#define sz_xChangeFeedbackControlReq		12
#define sz_xGetDeviceKeyMappingReq		8
#define sz_xGetDeviceKeyMappingReply		32
#define sz_xChangeDeviceKeyMappingReq		8
#define sz_xGetDeviceModifierMappingReq		8
#define sz_xSetDeviceModifierMappingReq		8
#define sz_xSetDeviceModifierMappingReply	32
#define sz_xGetDeviceButtonMappingReq		8
#define sz_xGetDeviceButtonMappingReply		32
#define sz_xSetDeviceButtonMappingReq		8
#define sz_xSetDeviceButtonMappingReply		32
#define sz_xQueryDeviceStateReq			8
#define sz_xQueryDeviceStateReply		32
#define sz_xSendExtensionEventReq		16
#define sz_xDeviceBellReq			8
#define sz_xSetDeviceValuatorsReq		8
#define sz_xSetDeviceValuatorsReply		32

#define INAME 			"XInputExtension"

#define XI_KEYBOARD	"KEYBOARD"
#define XI_MOUSE	"MOUSE"
#define XI_TABLET	"TABLET"
#define XI_TOUCHSCREEN	"TOUCHSCREEN"
#define XI_TOUCHPAD	"TOUCHPAD"
#define XI_BARCODE	"BARCODE"
#define XI_BUTTONBOX	"BUTTONBOX"
#define XI_KNOB_BOX	"KNOB_BOX"
#define XI_ONE_KNOB	"ONE_KNOB"
#define XI_NINE_KNOB	"NINE_KNOB"
#define XI_TRACKBALL	"TRACKBALL"
#define XI_QUADRATURE	"QUADRATURE"
#define XI_ID_MODULE	"ID_MODULE"
#define XI_SPACEBALL	"SPACEBALL"
#define XI_DATAGLOVE	"DATAGLOVE"
#define XI_EYETRACKER	"EYETRACKER"
#define XI_CURSORKEYS	"CURSORKEYS"
#define XI_FOOTMOUSE	"FOOTMOUSE"

#define Dont_Check			0
#define XInput_Initial_Release		1
#define XInput_Add_XDeviceBell		2
#define XInput_Add_XSetDeviceValuators	3

#define XI_Absent		0
#define XI_Present		1

#define XI_Initial_Release_Major		1
#define XI_Initial_Release_Minor		0

#define XI_Add_XDeviceBell_Major		1
#define XI_Add_XDeviceBell_Minor		1

#define XI_Add_XSetDeviceValuators_Major	1
#define XI_Add_XSetDeviceValuators_Minor	2

#define NoSuchExtension		1

#define COUNT			0
#define CREATE			1

#define NewPointer		0
#define NewKeyboard		1

#define XPOINTER		0
#define XKEYBOARD		1

#define UseXKeyboard		0

#define IsXPointer		0
#define IsXKeyboard		1
#define IsXExtensionDevice	2

#define AsyncThisDevice		0
#define SyncThisDevice		1
#define ReplayThisDevice	2
#define AsyncOtherDevices	3
#define AsyncAll		4
#define SyncAll			5

#define FollowKeyboard 		3
#define RevertToFollowKeyboard 	3

#define DvAccelNum              (1L << 0)
#define DvAccelDenom            (1L << 1)
#define DvThreshold             (1L << 2)

#define DvKeyClickPercent	(1L<<0)
#define DvPercent		(1L<<1)
#define DvPitch			(1L<<2)
#define DvDuration		(1L<<3)
#define DvLed			(1L<<4)
#define DvLedMode		(1L<<5)
#define DvKey			(1L<<6)
#define DvAutoRepeatMode	(1L<<7)

#define DvString                (1L << 0)

#define DvInteger               (1L << 0)

#define Relative                0
#define Absolute                1

#define AddToList               0
#define DeleteFromList          1

#define KeyClass  		0
#define ButtonClass  		1
#define ValuatorClass  		2
#define FeedbackClass  		3
#define ProximityClass  	4
#define FocusClass  		5
#define OtherClass  		6

#define KbdFeedbackClass  	0
#define PtrFeedbackClass  	1
#define StringFeedbackClass  	2
#define IntegerFeedbackClass  	3
#define LedFeedbackClass  	4
#define BellFeedbackClass  	5

#define _devicePointerMotionHint 0
#define _deviceButton1Motion	 1
#define _deviceButton2Motion	 2
#define _deviceButton3Motion	 3
#define _deviceButton4Motion	 4
#define _deviceButton5Motion	 5
#define _deviceButtonMotion	 6
#define _deviceButtonGrab	 7
#define _deviceOwnerGrabButton	 8
#define _noExtensionEvent	 9

#define XI_BadDevice	0
#define XI_BadEvent	1
#define XI_BadMode	2
#define XI_DeviceBusy	3
#define XI_BadClass	4

typedef	unsigned long	XEventClass;

/*******************************************************************
 *
 * Extension version structure.
 *
 */

typedef struct {
        int   	present;
        short	major_version;
        short	minor_version;
} XExtensionVersion;

#endif /* _XI_H_ */
\fP
.DE
.\" print Table of Contents
.if o .bp \" blank page to make count even
.bp 1
.af PN i
.PX
