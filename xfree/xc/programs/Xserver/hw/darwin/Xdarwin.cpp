.\" $XFree86: xc/programs/Xserver/hw/darwin/Xdarwin.cpp,v 1.1 2000/12/11 20:29:39 dawes Exp $
.\"
.TH XDARWIN 1 "Release 4.0.2" "XFree86"
.SH NAME
Xdarwin \- X window system server for Darwin operating system
.SH SYNOPSIS
.B Xdarwin
[ options ] ...
.SH DESCRIPTION
.I Xdarwin
is the window server for Version 11 of the X window system on the Darwin
operating system. It uses IOKit services to accesss the display framebuffer,
mouse and keyboard and to provide a layer of hardware abstraction.
.I Xdarwin
will normally be started by the \fIxdm(1)\fP display manager or by a script
that runs the program \fIxinit(1)\fP.
.SH OPTIONS
.PP
In addition to the normal server options described in the \fIXserver(1)\fP
manual page, \fIXdarwin\fP accepts the following command line switches:
.TP 8
.B \-fakebuttons
Emulates a 3 button mouse using the Command and Option keys. Clicking the
first mouse button while holding down Command will act like clicking
button 2. Holding down Option will simulate button 3.
.TP 8
.B \-nofakebuttons
Do not emulate a 3 button mouse. This is the default.
.TP 8
.B "\-size \fIwidth\fP \fIheight\fP"
Sets the screeen resolution for the X server to use.
.TP 8
.B "\-depth \fIdepth\fP"
Specifies the color bit depth to use. Currently only 8, 15, and 24 color bits
per pixel are supported.
.TP 8
.B "\-refresh \fIrate\fP"
Gives the refresh rate to use in Hz. For LCD displays this should be 0.
.TP 8
.B \-showconfig
Print out the server version and patchlevel.
.TP 8
.B \-version
Same as \fB\-showconfig\fP.
.SH "SEE ALSO"
.PP
X(__miscmansuffix__), Xserver(1), xdm(1), xinit(1)
.SH BUGS
.I Xdarwin
and this man page still have many limitations. Some of the more obvious
ones are:
.br
- Only one display is supported.
.br
- The display mode can not be changed once the X server has started.
.br
- A screen saver is not supported.
.br
- The X server does not wake from sleep correctly.
.br
- The key repeat rate can not be changed.
.PP
.SH AUTHORS
Original Port to Mac OS X Server - John Carmack
.br
Port to Darwin 1.0 - Dave Zarzycki
.br
Improvements and bug fixes - Torrey T. Lyons

