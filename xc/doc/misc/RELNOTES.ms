.\" $XConsortium: RELNOTES.ms /main/11 1996/03/05 14:41:33 gildea $
.\" X11R6.1 Release Notes.  Use troff -ms macros
.ds Ts March 5, 1996\" updated by Emacs
.\"
.ie t \{
.nr LL 6.5i
.\}
.el \{
.nr LL 72n
.na
.if n .pl 9999v		\" no page breaks in nroff
.\}
.nr FL \n(LLu
.nr LT \n(LLu
.ll \n(LLu
.nr PS 11
.de nH
.NH \\$1
\\$2
.XS
\\*(SN \\$2
.XE
.LP
..
.de Ip
.IP "\fB\\$1\fP" \\$2
..
.\"
.ds CH \" empty center heading
.sp 8
.TL
\s+2\fBX Window System, Version 11, Release 6.1\fP\s-2

\s+1\fBRelease Notes\fP\s-1
.AU
.sp 6
\fIStephen Gildea\fP
.AI

X Consortium
.sp 6
\*(Ts
.LP
.if t \{\
.bp
\&
.sp 3
.\}
.sp 5
Copyright \(co 1996 X Consortium
.nr PS 9
.nr VS 11
.LP
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
\*QSoftware\*U), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
.LP
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
.LP
THE SOFTWARE IS PROVIDED \*QAS IS\*U, WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
.LP
Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
.LP
\fIX Window System\fP is a trademark of X Consortium, Inc.
.\"
.if t \{
.OH 'X Version 11, Release 6.1''X Window System Release Notes'
.EH 'X Window System Release Notes''X Version 11, Release 6.1'
.bp 1
.ds CF \\n(PN
.\}
.nr PS 11
.nr VS 13


.nH 1 "What Is Release 6.1"

This is the X Consortium implementation of the X Window System.
X is a vendor-neutral, system-architecture neutral
network-transparent window system and user interface standard.
X runs on a wide range of computing and graphics machines.
For an overview of X, see the \fIX\fP manual page.
.LP
R6.1 is an update to R6.  It is compatible with R6 at the source and
protocol levels in all respects, and binaries are upward-compatible.
.LP
The X Consortium is an independent, not-for-profit corporation.
See the \fIXConsortium\fP manual page for details.
.LP
See \fBxc/INSTALL.PS\fP (PostScript) or \fBxc/INSTALL.TXT\fP (plain
text) for instructions on how to build and install this software.

.nH 2 "Overview of the X Consortium Release"

The X Consortium software and documentation in Release 6.1 is in
directory \fBxc/\fP and contains the following:
.Ip "X Consortium Standards"
The X Consortium produces standards:  documents which define
network protocols, programming interfaces, and other aspects of
the X environment.  See the \fIXStandards\fP manual page for a
list of standards.
.Ip "Implementations"
For most of our standards, we provide high-quality implementations
to demonstrate proof of concept and to give early adoptors and vendors
a base to use.
These are not \fIreference\fP
implementations; the written specifications define the standards.
.Ip "Fonts"
.br
A collection of bitmap and outline fonts are included in the
distribution, contributed by various individuals and companies.
.Ip "Utility Libraries"
A number of libraries, such as \fIXmu\fP and
the \fIAthena Widget Set\fP, are included.
These are not standards, but are used in building
X Consortium applications and may be useful in building other applications.
.Ip "Programs"
We also provide a number of application programs.
A few of these programs, such as \fIxdm\fP,
should be considered essential in almost all environments.
The rest of the applications carry no special status; they
are simply programs that have been developed and/or maintained
by X Consortium staff.
In some cases, you will find better substitutes for these
programs contributed by others.

.nH 2 "Supported Systems"

We built and tested this release on the following systems:
.LP
.KS
.ID
AIX 4.1.4
HP-UX 10.01
IRIX 5.3
IRIX 6.2 beta
DEC OSF/1 3.2
Digital Unix 4.0 beta
FreeBSD 2.1.0R
Linux 1.2.11 (Slackware 2.3)
SCO 3.2
SunOS 4.1.3
SunOS 5.4 (Sparc and x86)
UNIX System V/386 Release 4.2 (Novell UnixWare) Version 2.02
UNIX System V/386/486 Release 4.0 (NCR) Version 3.0
Ultrix 4.4
Microsoft Windows NT 3.5
.DE
.KE
In addition, Cray has tested the beta release on UNICOS 8.0.
.LP
In all cases we have used the vendor's compiler.  On the two SunOS
releases we also test with \fIgcc\fP.

.nH 3 "Supported Display Devices"

This release includes the necessary device-dependent support for an X
server on the following platforms and devices:
.LP
.KS
.ID
AIX with Skyway display adaptor
HP-UX
DEC OSF/1 on Alpha AXP with PMAG-B frame buffer
Ultrix
Various Intel-based platforms: See the XF86_* manual pages.
Sun SPARCstations: See the Xsun(1) manual page for supported frame buffers.
.DE
.KE

.nH 2 "The XC Tree"

The general layout under \fBxc/\fP is as
follows:
.LP
.DS L
.ta 2i
config/ 	config files, \fIimake\fP, \fImakedepend\fP, build utilities
doc/    	all documentation other than per-program manual pages
fonts/  	BDF, Speedo, Type1 fonts
include/	include files shared by multiple directories
lib/    	all libraries
nls/    	national language support files
programs/	all programs, including the X server and \fIrgb\fP
test/   	X Test Suite and other test suites
util/   	\fIpatch\fP, \fIcompress\fP, other utilities
bug-report	bug reporting template
registry	X Registry
.DE
.LP
This file is \fBxc/RELNOTES.*\fP, in various formats.
The documentation source files \fBRELNOTES.ms\fP and \fBINSTALL.ms\fP
are in the new \fBxc/doc/misc/\fP directory.

.nH 2 "X Registry"

The X Consortium maintains a registry of certain X-related items to
aid in avoiding conflicts and to aid in sharing of such items.
.LP
The registry is in the file \fBxc/registry\fP in the distribution.
The latest version is also available by sending a
message to xstuff@x.org.  The message can have a subject line and no
body, or a single-line body and no subject; in either case the line
should look like this:
.DS
send docs registry
.DE

.nH 2 "Extensions supported"

The core distribution includes the following extensions:
BIG-REQUESTS,
DOUBLE-BUFFER,
MIT-SHM,
MIT-SUNDRY-NONSTANDARD,
Multi-Buffering,
RECORD,
SHAPE,
SYNC,
X3D-PEX,
XC-MISC,
XFree86-VidModeExtension,
XIE,
XInputExtension,
XKEYBOARD,
XTEST, and
XTestExtension1.
.LP
Not all of these extensions are standards; see the \fIXStandards\fP
manual page.

.nH 2 "Implementation Parameters"

Some of the specifications define some behavior as
implementation-dependent.
Implementations of X Consortium standards need to document how those
parameters are implemented; this section does so.
.IP "XFILESEARCHPATH default"
This default can be set at build time by setting the \fIimake\fP variables
.hw XAppLoadDir
XFileSearchPathDefault, XAppLoadDir, XFileSearchPathBase, and
ProjectRoot in \fBsite.def\fP.  See \fBxc/config/cf/Project.tmpl\fP
for how they are used.
.IP
By default ProjectRoot is /usr/X11R6.1 and
XFILESEARCHPATH has these components:
.ID
/usr/X11R6.1/lib/X11/%L/%T/%N%C%S
/usr/X11R6.1/lib/X11/%l/%T/%N%C%S
/usr/X11R6.1/lib/X11/%T/%N%C%S
/usr/X11R6.1/lib/X11/%L/%T/%N%S
/usr/X11R6.1/lib/X11/%l/%T/%N%S
/usr/X11R6.1/lib/X11/%T/%N%S
.DE
.IP "XUSERFILESEARCHPATH default"
If the environment variable XAPPLRESDIR is defined, the default value
of XUSERFILESEARCHPATH has the following components:
.ID
$XAPPLRESDIR/%L/%N%C
$XAPPLRESDIR/%l/%N%C
$XAPPLRESDIR/%N%C
$HOME/%N%C
$XAPPLRESDIR/%L/%N
$XAPPLRESDIR/%l/%N
$XAPPLRESDIR/%N
$HOME/%N
.DE
Otherwise it has these components:
.KS
.ID
$HOME/%L/%N%C
$HOME/%l/%N%C
$HOME/%N%C
$HOME/%L/%N
$HOME/%l/%N
$HOME/%N
.DE
.KE
.IP "XKEYSYMDB default"
Defaults to \fB/usr/X11R6.1/lib/X11/XKeysymDB\fP, assuming
\fBProjectRoot\fP is set to \fB/usr/X11R6.1\fP.
.IP "XCMSDB default"
Defaults to \fB/usr/X11R6.1/lib/X11/Xcms.txt\fP, assuming
\fBProjectRoot\fP is set to \fB/usr/X11R6.1\fP.
.IP "XLOCALEDIR default"
Defaults to the directory \fB/usr/X11R6.1/lib/X11/locale\fP, assuming
\fBProjectRoot\fP is set to \fB/usr/X11R6.1\fP.
The XLOCALEDIR variable can contain multiple colon-separate pathnames.
.IP "XErrorDB location"
The Xlib error database file is \fB/usr/X11R6.1/lib/X11/XErrorDB\fP, assuming
\fBProjectRoot\fP is set to \fB/usr/X11R6.1\fP.
.IP "XtErrorDB location"
The Xt error database file is \fB/usr/X11R6.1/lib/X11/XtErrorDB\fP, assuming
\fBProjectRoot\fP is set to \fB/usr/X11R6.1\fP.
.IP "Supported Locales"
X locales supported are in \fBlocale.dir\fP; the mapping between various
system locale names and X locale names is in \fBlocale.alias\fP.
Both files are in the \fBxc/nls/X11/locale/\fP directory.
.IP "Input Methods supported"
The core distribution does not include any input method servers.
However, Xlib supplies a default built-in input method that supports 
compose processing in 8-bit locales.
Compose files are provided for Latin-1 and Latin-2.
The built-in input method can support other locales, given suitable
compose files.
See \fBxc/nls/X11/locale/Compose/iso8859-*\fP for the supported
compositions.
.LP
There are input method servers in the contrib software.


.nH 1 "What is Unchanged in Release 6.1"

As this is an update release, there is a great deal of stability in the
standards, libraries, and clients.  No existing standards have changed
in a material way, though several documents have been updated with
editorial improvements.
Most of the libraries have no new interfaces.


.nH 1 "What Is New in Release 6.1"

This section describes changes in the X Consortium distribution since
Release 6.
.LP
All libraries, protocols, and servers are
compatible with Release 6.  That is, R6 clients and applications
will work with R6.1 libraries and servers and vice versa.
.LP
The major new functionality in R6.1 is the X Keyboard extension,
double buffering for smooth animation, and protocol recording.

.nH 2 "OS Support"

The following platforms now have a newer operating system version supported:
.LP
.DS 0
.ta 1.5i 3i
System	R6	R6.1
.sp 1
AIX	3.2.5	4.1.4
DEC OSF/1	1.0 and 1.3	3.2 and 4.0 (now called Digital Unix 4.0)
FreeBSD	1.1	2.1
HP-UX	9.1	10.01
IRIX	5.2	5.3 and 6.2
Linux	1.0.0	1.2.11
NCR SVR4	MP-RAS	Version 3.0
NEWS-OS	6.0	6.1
SunOS 5	5.3	5.4
Ultrix	4.3	4.4
UnixWare	1	2.02
Windows NT	3.1	3.5
.DE
.LP
Release 6.1 adds support for SCO Open Server Release 5.0.
.LP
Release 6.1 does not support the following platforms supported in
Release 6: A/UX, Mach (from Omron), Unix System V/860 (from Oki),
and BSD/OS.

.nH 2 "New Standards"

The following are new X Consortium standards in Release 6.1.
Each is described in its own section below.
.ID
XKEYBOARD (XKB)
RECORD
DOUBLE-BUFFER (DBE)
ICE X Rendevous
.DE

.nH 2 DBE

The Double Buffer Extension (DBE) provides a standard way to utilize
double-buffering, allowing flicker-free animation.
.LP
The older Multi-Buffering extension is not linked in to the X
server by default.
It will move to unsupported status at the next release.

.nH 2 XKB

An early version of the X Keyboard extension (XKB) was shipped as a work
in progress in R6.  In R6.1 it is now complete.
.LP
XKB provides detailed keyboard descriptions and enhanced keyboard
functionality, including support for the ISO 9995 keyboard model.
.LP
With XKB are several new core clients,
\fIxkbcomp\fP,
\fIxkbevd\fP, \fIxkbprint\fP,
each in their own directory under \fBxc/programs\fP, and
\fIxkbbell\fP, \fIxkbvleds\fP, and \fIxkbwatch\fP, in
\fBxc/programs/xkbutils\fP.
.\" There are also some XKB test programs in \fBcontrib/test/Xkb\fP.

.nH 2 RECORD

An early version of the RECORD extension was shipped as a work in
progress in R6.  It is now complete.
.LP
RECORD is an X protocol extension that supports the recording of all
core X protocol and arbitrary X extension protocol.
.LP
Documentation of RECORD is in \fBxc/doc/specs/Xext/record*.ms\fP and
\fBxc/doc/hardcopy/Xext/record*.PS.Z\fP.
A thorough test program is in \fBxc/test/record/\fP.

.nH 2 VidMode

This extension allows interactively adjusting
graphics frame buffer parameters on PC-based (primarily
Intel) hardware. 
It is not a Consortium standard.
.\" A client that uses the extension can be found in contrib/programs/xvidtune.

.nH 2 "ICE X Rendezvous"

The Inter-Client Exchange protocol (ICE), which became a standard in
X11R6, specifies a generic communication framework for data exchange
between arbitrary clients.  The ICE protocol itself does not specify
the manner in which two clients interested in communicating via ICE
are made aware of each other's existence.
.LP
The ICE X Rendezvous protocol is one standard protocol by which two
clients who have connections to a common X server can rendezvous.
This new protocol is included in the ICE Protocol Specification
document.

.nH 2 "Configuration"

The top-level Makefile is no longer over-ridden by the first build.
Instead a new file \fIxmakefile\fP is created.  Thus is it no longer
necessary to copy \fIMakefile.ini\fP to \fPMakefile\fP to reset the builds.
.LP
In R6, it was only necessary to supply BOOTSTRAPCFLAGS on the command
line of the first make World on platforms that needed it.
As of R6.1, this flag must always be passed in the make World
command.
However, Solaris 2 and USL systems no longer require BOOTSTRAPCFLAGS.
.LP
The file \fBxc/config/cf/README\fP has been greatly expanded.
It now provides more guidance on how to write an Imakefile, including
a list of variables that may be set in an Imakefile.
A must read for Imakefile authors.
.LP
The X configuration now supports Atria's clearmake, which allows
sharing of object files when using ClearCase.
.LP
The LaTeX text processor is now supported.  If you have LaTeX on your
system, turn on HasLatex to have the MakeLatexDoc rule use it.
.LP
We have added support for threads on more systems:
Standard conforming pthreads (POSIX threads) on AIX 4.1.x and Digital
Unix 4.0.
(As in R6, SVR4 threads are also supported on Solaris 2 and Unixware,
MS-Windows threads on NT 3.5, and Draft 4 pthreads on DEC OSF/1 2 and 3.)
POSIX threads are not supported on Solaris 2.5.
.LP
With System V Release 4 (SVR4) compilers, we now use the \|\-Xa (ANSI C
with native extensions) compiler flag rather than \|\-Xc (limit
environment to that specified in the standard).  This change provides
access to the full richness of the platform.  Unfortunately, it also
defines the preprocessor symbol _\^_STDC_\^_ to 0, instead of 1 as
specified by the standard.
Therefore we have changed all \*Q#if _\^_STDC_\^_\*U
tests in our code to \*Q#ifdef _\^_STDC_\^_\*U.  Changing the test does
not break any currently-supported compilers.
On HP-UX systems we now use the \|\-Ae compiler option instead of \|\-Aa,
also to access the full environment offered by the platform.
.LP
The database used by the \*Qman \|\-k\*U command is rebuilt when doing
\*Qmake install.man\*U at the top of the \fBxc\fP tree.
.LP
The \fImakestrs\fP program has moved from \fBxc/lib/Xt/util\fP to
\fBxc/config/util\fP so it can be used by libraries other than Xt.
It now has a manual page.  This enhancement was shipped in
R6 public patch 12.
.LP
The imake variables \fBInstallXdmConfig\fP, \fBInstallXinitConfig\fP,
and \fBInstallAppDefFiles\fP have changed semantics.  Previously,
turning off these variables would suppress ever installing the files
they controlled.  In R6.1, it suppresses overwriting existing files;
if the files didn't previously exist, the files are always installed.
The new interpretation makes bootstrapping a new system easier.

.nH 2 "Internationalization

Clarifications have been made to several sections of
Chapter 13 of the Xlib specification.
No changes to the Xlib standard are involved.

.nH 2 "Documentation"

The Release Notes document is split into two, the real release notes
(this document) and installation instructions.  Formatted versions of
both continue to be at top-level in \fBxc/RELNOTES.*\fP and
\fBxc/INSTALL.*\fP.
A new directory \fBxc/doc/misc/\fP holds the troff \-ms sources.
.LP
A new file \fBxc/doc/misc/xlogo.epsi\fP is the X logo in PostScript.

.nH 2 "Header Files"

\fBxc/include/Xalloca.h\fP is solely responsible for defining ALLOCATE_LOCAL
and DEALLOCATE_LOCAL.  You should be able to add or update a platform's
support for alloca() by editing this one file instead of finding and
changing the multiple definitions that existed previously.
.LP
\fBxc/include/Xpoll.h\fP allows more portable, consistent select() and
poll() use in the clients, including getting the fd_set properly
defined.
(The servers still use select on all systems, even those that have poll.)

.nH 2 "X Server"

.nH 3 "Device Support"

The following ddxen have been removed: macII, omron, and svga.
.LP
The XFree86 ddx has been updated to 3.1.2C.
.LP
The IBM ddx has been updated to work on AIX 4 as well as AIX 3.2.
.LP
There is a new HP ddx with support for a new graphics card,
the HCRX (HyperCRX, HPA4071A_Z), available in 8-bit or 24-bit deep options.  
The 24-bit version has a optional hardware accelerator, in which case
it's known as an HCRX24Z.
This ddx was distributed in R6 public patch 9.
.LP
The Xnest ddx now works on 64-bit machines.
.LP
The DEC ddx now works on an Alpha with a simple framebuffer (PMAG-B).

.nH 3 "Internal Changes"

To support DBE idioms, the new functions PeekNextRequest and
SkipRequests add the ability to do request lookahead and skipping.
See \fBxc/programs/server/os/io.c\fP, \fBxc/programs/server/include/os.h\fP, and
\fBxc/doc/specs/Xserver/ddx.tbl.ms\fP.
.LP
The pixelization of zero-width lines is now tunable so that you can
make the server match what your hardware does.  See
\fBxc/programs/Xserver/mi/miline.h\fP.  As a result of this work, clipping and
pixelization of zero-width lines are now consistent across cfb, mfb,
and mi.
.LP
Several new callback lists were introduced to support the RECORD
extension: DeviceEventCallback, ReplyCallback,
SkippedRequestsCallback, and FlushCallback.  The parameters of the
ClientStateCallback changed: instead of passing a pointer to the
client as the call_data, a pointer to a small structure containing a
pointer to the client and pointers to the connection setup information
is passed.

.nH 2 "New Programs"

There are new core programs \fIxkbcomp\fP, \fIxkbevd\fP,
\fIxkbprint\fP, \fIxkbbell\fP, \fIxkbvleds\fP, and \fIxkbwatch\fP.

.nH 2 "xmh"

The \fIxmh\fP mail reader is now session aware.
This enhancement was distributed in R6 public patch 8.

.nH 2 "xsm"

The \fIxsm\fP session manager has many enhancements.
It has been moved out of \fBxc/workInProgress\fP into \fBxc/programs\fP.
Most of the
enhancements were distributed in R6 public patch 8.
Advanced signal handling in \fIxsm\fP is appearing for the first time
in R6.1.

.nH 2 "makeg"

The \fImakeg\fP script runs \fImake\fP with the options necessary to
make a debuggable program.

.nH 2 "xterm"

The xterm terminal emulator has been minimally internationalized to use
the Xlib built-in input method with 8-bit character sets.

.nH 2 "Fonts"

Digital has contributed numerous fixes the the bitmap fonts.
These were distributed with R6 public patch 10.

.nH 2 "X Test Suite"

As of X11R6.1, the X Test Suite, which tests functionality only up
through X11R4, is considered unsupported software, and no further
development is planned.
.LP
For those who require a more up-to-date test suite with available support, 
X/Open Company Ltd. offers VSW5, a successor to the X Consortium Test Suite.
VSW5 includes many bug fixes and a large number of new tests for Xt and 
for new R5 functionality.  R6 tests are planned for the future.  Refer
to <http://www.xopen.org/public/test/vsw45.htm> for more information.

.nH 2 "ANSIfication"

As noted previously under \*QConfiguration Files\*U, for pragmatic
reasons we changed the way we use _\^_STDC_\^_ to test for standard C
compilers.  This is only a short-term issue, as R6.1 will be the last
release that will support traditional K&R C.  Future releases will
assume a standard C compiler and environment.

.nH 2 "Software No Longer Included"

The software described in this section has been removed from the core
distribution in R6.1.

.nH 3 MTXserver

The multi-threaded X server snapshot is no longer shipped.  It was in
\fBxc/workInProgress\fP in R6.  No further development has occurred.
Considerable would work have been necessary to
get the MTXserver sources back into a state where they could be compiled.

.nH 3 LBX

Low Bandwidth X, shipped in preliminary form in
\fBxc/workInProgress\fP in R6, has been removed from the distribution
pending completion of the protocol design and sample implementation.
It will reappear in a future release.

.nH 3 Fresco

Fresco, shipped in \fBxc/workInProgress\fP in R6, 
is now independently distributed.
Source and documentation
are available from <http://www.faslab.com/fresco/HomePage.html>.


.nH 1 "Filing Bug Reports"

If you find a reproducible bug in software in the \fBxc\fP directory,
or find bugs in the \fBxc\fP documentation, please send a bug report
to the X Consortium using the form in the file \fBxc/bug-report\fP and
this destination address:
.ID
xbugs@x.org
.DE
.LP
Please try to provide all of the information requested on the form if it is
applicable; the little extra time you spend on the report will make it
much easier for us to reproduce, find, and fix the bug.  Receipt of bug
reports is generally acknowledged, but sometimes it can be delayed by a
few weeks.
.LP
Bugs in \fBcontrib\fP software are not handled by X Consortium staff.
Consult the documentation for the individual software to
see where (if anywhere) to report the bug.
Many authors of contributed software
subscribe to the mailing list \*Qcontrib-bugs\*U hosted at
x.org, so this might be a useful place to report bugs.
(To subscribe to contrib-bugs yourself, send email to
contrib-bugs-request@x.org.)


.nH 1 "Acknowledgements"

Release 6.1 of X Version 11 is brought to you by the X staff at
X Consortium, Inc:
Donna Converse,
Stephen Gildea,
Kaleb Keithley,
Matt Landau,
Ralph Mor,
Bob Scheifler,
Ralph Swick,
Ray Tice,
Mark Welch,
and Dave Wiggins.
.LP
Many companies and individuals have cooperated and worked extremely hard to
make this release a reality, and our thanks go out to them.  You will find
many of them listed in the acknowledgements in the individual specifications.
.LP
Contributions were received from
.LP
Mike Patnode and SCO.
.LP
XKB: Erik Fortune (SGI) was the architect and major implementor.
Help was received from Will Walker (Digital).
Funding for some of this work was provided
by IBM.  The library spec was written by Gary Atkins and
Amber Benson.
The XFree86 Team used early versions and provided useful feedback and
bug reports.
.LP
DBE:
T. Alex Chen (IBM),
Peter Daifuku (SGI),
Ian Elliott (HP),
Jim Graham (Sun),
Larry Hare (AGE),
Jay Hersh (X Consortium),
Daryl Huff (Sun),
Deron Dann Johnson (Sun),
Louis Khouw (Sun),
Mark Kilgard (SGI),
Allen Leinwand (SGI).
Rob Lembree (Digital),
Alan Ricker (Metheus),
Michael Rosenblum (Digital),
Larry Seiler (Digital),
Jeanne Sparlin (IBM),
Jeff Stevenson (HP),
Walter Strand (Metheus),
Ken Tidwell (HP),
Tom Yip (HP).
.LP
RECORD:
Rob Chesler (Absol-puter),
Amnon Cohen (Mercury Interactive),
Kieron Drake (UniSoft),
Marc Evans (Synergytics)
Jim Fulton (NCD),
Jim Haggerty (Metheus)
Ken Miller (Digital),
Alan Ricker (Metheus),
Kent Siefkes (Performance Awareness),
and
Martha Zimet (NCD).
.LP
ddxen: HP, IBM, Digital, XFree86 Team.
.LP
fonts: Digital
.LP
Hidetoshi Tajima (Sun) tested the new internationalization code,
providing useful bug reports and fixes.
.LP
ICE X Rendevous: Will Walker (Digital), Keith Edwards (Georgia
Institute of Technology).
.ie t \{
.\"
.\" print Table of Contents
.if o .bp \" blank page to make count even
.bp 1
.af PN i
.PX
.\}
.el .pl \n(nlu+1v
.\"
.\" Local Variables:
.\" time-stamp-start: "^\\.ds Ts "
.\" time-stamp-end: "\\\\\""
.\" time-stamp-format: "%b %d, %y"
.\" End:
