.\" Use eqn, tbl, and -ms
.\" $XConsortium: xdmcp.ms /main/26 1996/02/06 15:03:19 swick $
.EQ
delim @@
define oc % "\\fR{\\fP" %
define cc % "\\fR}\\fP" %
.EN
.de PT
..
.de BT
..
.ps 10
.nr PS 10
\&
.TL
\s+2\fBX Display Manager Control Protocol\fP\s-2
.sp
Version 1.0
.sp
X Consortium Standard
.sp
X Version 11, Release 6.1
.AU
Keith Packard
.AI
X Consortium
Laboratory for Computer Science
Massachusetts Institute of Technology
.AB
.LP
Since the X Display Manager (xdm) may be used to manage remote displays (such
as X terminals), a protocol for requesting service over the network is
needed.  Since the display manager is a scarce resource, the X Display
Manager Control Protocol (also called XDMCP) is designed to use unreliable
datagrams and places the bulk of the burden for sequencing and retransmission
on the display.  A standard byte order and synchronous responses reduces the
complexity of the protocol.
.AE
.LP
.bp
.br
\&
.sp 15
Copyright \(co 1989 X Consortium
.sp 3
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
.LP
.sp 3
\fIX Window System\fP is a trademark of X Consortium, Inc.
.de PT
.ie o .tl 'XDMCP''X Display Manager Control Protocol '
.el .tl 'X Display Manager Control Protocol ''XDMCP'
..
.bp 1
.de BT
.tl ''\fB % \fP''
..
.NH 1
Purpose and Goals
.LP
The purpose of XDMCP is to provide a uniform mechanism for an autonomous
display to request login service from a remote host.  By "autonomous" we mean
the display consists of hardware and processes that are independent of any
particular host where login service is desired.  (For example, the server
cannot simply be started by a fork/exec sequence on the host.)  An "X terminal"
(screen, keyboard, mouse, processor, network interface) is a prime example of
an autonomous display.
.LP
From the point of view of the end user, it is very important to make
autonomous displays as easy to use as traditional hardwired character
terminals.  Specifically, you can typically just power on a hardwired
terminal, and be greeted with a login prompt.  The same should be possible
with autonomous displays.  However, in a network environment with multiple
hosts, the end user may want to choose which host(s) to connect to.  In an
environment with many displays and many hosts, a site administrator may want
to associate particular collections of hosts with particular displays.  We
would like to support the following options:
.RS
.LP
The display has a "hardwired" host that it should connect to.  It should be
possible to "power on" the display and receive a login prompt, without user
intervention.
.LP
Any one of several hosts on a (sub)net may be acceptable for accepting login
from the display.  (For example, the user's file systems can be mounted onto
any such host, providing comparable environments.)  It should be possible
for the display to "broadcast" to find such hosts, and to have the display
either automatically choose a host, or present the possible hosts to the
user for selection.
.LP
The display has a fixed set of hosts that it can connect to.  It should be
possible for the display to have that set "wired in", but it should also be
possible for a site administrator to be able to maintain host sets for a
large number of displays using a centralized facility, without having to
interact (physically or electronically) with each individual display.
Particular hosts should be allowed to refuse login service, based on
whatever local criteria are desired.
.RE
.LP
The control protocol should be designed in such a way that it can be used over
a reasonable variety of communication transport layers.  In fact, it is quite
desirable if every major network protocol family that supports the standard X
protocol is also capable of supporting XDMCP, since the end result of XDMCP
negotiation will be standard X protocol connections to the display.  However,
since the number of displays per host may be large, a connection-based protocol
appears less desirable than a connection-less protocol.
.LP
In order to keep the burden on displays at a minimum (since display cost is not
a factor that can be ignored), it is desirable that displays not be required to
maintain permanent state (across power cycles) for the purposes of the control
protocol, and it is desirable to keep required state at a minimum while the
display is powered on.
.LP
Security is an important consideration, and must be an integral part of the
design.  The important security goals in the context of XDMCP are:
.RS
.LP
It should be possible for the display to verify that it is communicating
with a legitimate host login service.  Since the user will present
credentials (e.g. password) to this service, it is important to avoid spoof
attacks.
.LP
It should be possible for the display and the login service to negotiate the
authorization mechanism to be used for the standard X protocol.
.LP
It should be possible to provide the same level of security in verifying the
login service as is provided by the negotiated authorization mechanism.
.LP
Since there are no firm standards yet in the area of security, XDMCP must be
flexible enough to accomodate a variety of security mechanisms.
.RE
.NH 1
Overview of the protocol
.LP
XDMCP is designed to provide authenticated access to display management
services for remote displays.  A new network server, called a \fBDisplay
Manager\fP will use XDMCP to communicate with displays to negotiate the
startup of X sessions.  The protocol allows the display to authenticate the
manager.  It also allows most of the configuration information to be
centralized with the manager, to ease the burden of system administration in
a large network of displays.  The essential goal is to provide plug-and-play
services similar to those provided in the familiar mainframe/terminal world.
.LP
Displays may be turned off by the user at any time.  Any existing session
running on a display which has been turned off must be identifiable.  This
is made possible by requiring a three-way handshake to start a session.  If
the handshake succeeds, any existing session is terminated immediately and a
new session started.  There is the problem (at least with TCP) that
connections may not be closed when the display is turned off.  In most
environments, the manager should reduce this problem by periodically XSync'ing
on its own connection, perhaps every five to ten minutes, and terminating the
session if its own connection ever closes.
.LP
Displays should not be required to retain permanent state for purposes of
the control protocol.  One solution to packets received out of sequence
would be to use monotonically-increasing message identifiers in each message
to allow both sides to ignore messages which arrive out-of-sequence.  For
this to work, displays would at a minimum have to increment a stable "crash
count" each time they are powered on, and use that number as part of a
larger sequence number.  But if displays cannot retain permanent state this
cannot work.  Instead, the manager assumes the responsibility for permanent
state by generating unique numbers which identify a particular session and
the protocol simply ignores packets which correspond to an invalid session.
.LP
The Manager must not be responsible for packet reception.  To prevent the
Manager from becoming stuck because of a hostile display, no portion of the
protocol requires the Manager to retransmit a packet.  Part of this means
that any valid packet which the Manager does receive \fImust\fP be
acknowledged in some way, to prevent the display from continuously resending
packets.  The display can keep the protocol running as it will always know
when the Manager has received (at least one copy of) a packet.  On the
Manager side, this means that any packet may be received more than once (if
the response was lost) and duplicates must be ignored.
.NH 1
Data Types
.LP
XDMCP packets contain several types of data.  Integer values are always
stored most significant byte first in the packet (``Big Endian'' order).
As XDMCP will not be used to transport large quantities of data, this
restriction will not substantially hamper the efficiency of any
implementation.  Also, no padding of any sort will occur within the packets.
.TS
expand;
c c c
c c c
l l l.
Type Name	Length	Description
	(in bytes)
CARD8	1	A single byte unsigned integer
CARD16	2	Two byte unsigned integer
CARD32	4	Four byte unsigned integer
ARRAY8	n+2	This is actually a CARD16 followed by
		a collection of CARD8.  The value of the CARD16
		field (n) specifies the number of CARD8 values to
		follow
ARRAY16	2*m+1	This is a CARD8 (m) which specifies the
		number of CARD16 values to follow
ARRAY32	4*l+1	This is a CARD8 (l) which specifies the
		number of CARD32 values to follow
ARRAYofARRAY8	?	This is a CARD8 which specifies the
		number of ARRAY8 values to follow.
		
.TE
.NH 1
Packet Format
.LP
All XDMCP packets have the following information:
.TS
expand;
c c c c
c c c c
_ _ _
| c l l | c
| c l l | c
| c l l | c
_ _ _
c l l c.
Length in	Field	Description of field
Bytes	Type	

2	CARD16	version number
2	CARD16	opcode	packet header
2	CARD16	n = length of remaining data in bytes

n	???	packet-specific data
.TE
.LP
The fields are as follows:
.LP
Version Number
.RS
This specifies the version of XDMCP that generated this packet in
case changes in this protocol are required.  Displays and
managers may choose to support older versions for compatibility.
This field will initially be 1.
.RE
.LP
Opcode
.RS
This specifies what step of the protocol this packet represents and should
contain one of the following values (encoding provided in section below):
\fBBroadcastQuery\fP, \fBQuery\fP, \fBIndirectQuery\fP, \fBForwardQuery\fP,
\fBWilling\fP, \fBUnwilling\fP, \fBRequest\fP, \fBAccept\fP, \fBDecline\fP,
\fBManage\fP, \fBRefuse\fP, \fBFailed\fP, \fBKeepAlive\fP, \fBAlive\fP.
.RE
.LP
Length of data in bytes
.RS
This specifies the length of the information following the first 6 bytes.
Each packet-type has a different format, and will need to be separately
length-checked against this value.  As every data item has either an
explicit length, or an implicit length, this can be easily accomplished.
Packets that have too little or too much data should be ignored.
.RE
.LP	
Packets should be checked to make sure that they satisfy the following
conditions:
.RS
.IP 1
They must contain valid opcodes.
.IP 2
The length of the remaining data should correspond to the sum of the 
lengths of the individual remaining data items.
.IP 3
The
\fCopcode\fP
should be expected (a finite state diagram is given
in a later section).
.IP 4
If the packet is of type \fBManage\fP or \fBRefuse\fP, the \fCSession ID\fP
should match the value sent in the preceding \fBAccept\fP packet.
.RE
.NH 1
Protocol
.LP
Each of the opcodes is described below.  Since a given packet type is only
ever sent one way, each packet description below indicates the direction.
Most of the packets have additional information included beyond the
description above.  The additional information is appended to the packet
header in the order described without padding, and the length field is
computed accordingly.
.LP
\fBQuery\fP
.br
\fBBroadcastQuery\fP
.br
\fBIndirectQuery\fP
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\fCAuthentication Names\fP:
ARRAYofARRAY8
.RS
A list of authentication names which the display supports.  The manager will
choose one of these and return it in the \fBWilling\fP packet.
.RE
.RE
Semantics:
.RS
A \fBQuery\fP packet is sent from the display to a specific host to ask if
that host is willing to provide management services to this display.  The
host should respond with \fBWilling\fP if it is willing to service the
display or \fBUnwilling\fP if it is not.
.LP
A \fBBroadcastQuery\fP packet is similar to the \fBQuery\fP packet except
that it is intended to be received by all hosts on the network (or
sub-network).  However, unlike \fBQuery\fP requests, hosts that are not
willing to service the display should simply ignore \fBBroadcastQuery\fP
requests.
.LP
An \fBIndirectQuery\fP packet is sent to a well-known manager which forwards
the request to a larger collection of secondary managers using
\fBForwardQuery\fP packets.  In this way, the collection of managers which
respond can be grouped on other than network boundaries; the use of a
central manager reduces system administrative overhead.  The primary manager
may also send a \fBWilling\fP packet in response to this packet.
.LP
Each packet type has slightly different semantics:
.IP
The \fBQuery\fP packet is destined only for a single host.  If the display
is instructed to \fBQuery\fP multiple managers, it will send multiple
\fBQuery\fP packets.  The \fBQuery\fP packet also demands a response from
the manager, either \fBWilling\fP or \fBUnwilling\fP.
.IP
The \fBBroadcastQuery\fP packet is sent to many hosts.  Each manager which
receives this packet will not respond with an \fBUnwilling\fP packet.
.IP
The \fBIndirectQuery\fP packet is sent to only one manager, with the request
that the request be forwarded to a larger list of managers using
\fBForwardQuery\fP packets.  This list is expected to be maintained at one
central site to reduce administrative overhead.  The function of this packet
type is similar to \fBBroadcastQuery\fP except that \fBBroadcastQuery\fP is
not forwarded.
.RE
Valid Responses:
.RS
\fBWilling\fP, \fBUnwilling\fP
.RE
Problems/Solutions:
.RS
Problem:
.RS
Not all managers receive the query packet.
.RE
.RS
Indication:
.RS
none if \fBBroadcastQuery\fP or \fBIndirectQuery\fP was sent, else failure
to receive \fBWilling\fP.
.RE
Solution:
.RS
Repeatedly send the packet while waiting for user to choose a manager.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
An exponential backoff algorithm should be used here to reduce network load
for long-standing idle displays.  Start at 2 seconds, back off by factors of
2 to 32 seconds and discontinue retransmit after 126 seconds.  The display
should reset the timeout when user-input is detected.  In this way, the
display will ``wakeup'' when touched by the user.
.RE
.RE
.LP
\fBForwardQuery\fP
.RS
Primary Manager \(-> Secondary Manager
.br
Additional Fields:
.RS
\fCClient Address\fP:
ARRAY8
.RS
The network address of the client display.
.RE
\fCClient Port\fP:
ARRAY8
.RS
An identification of the client task on the client display.
.RE
\fCAuthentication Names\fP:
ARRAYofARRAY8
.RS
This is a duplicate of \fCAuthentication Names\fP array which was received
in the \fBIndirectQuery\fP
packet.
.RE
.RE
Semantics:
.RS
When primary manager receives a \fBIndirectQuery\fP packet, it is
responsible for sending \fBForwardQuery\fP packets to an appropriate list of
managers which can provide service to the display using the same network
type as the one the original \fBIndirectQuery\fP packet was received from.
The \fCClient Address\fP and \fCClient Port\fP fields must contain an
address which the secondary manager can use to reach the display also using
this same network.  Each secondary manager sends a \fBWilling\fP packet to
the display if it is willing to provide service.
.LP
\fBForwardQuery\fP packets are similar to \fBBroadcastQuery\fP packets in
that managers which are not willing to service particular displays should
not send a \fBUnwilling\fP packet.
.RE
Valid Responses:
.RS
\fBWilling\fP
.RE
Problems/Solutions:
.RS
Identical to \fBBroadcastQuery\fP
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from a manager, this packet should never be
retransmitted.
.RE
.RE
.LP
\fBWilling\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\fCAuthentication Name\fP:
ARRAY8
.RS
This specifies the authentication method, selected from the list offered in
the \fBQuery\fP, \fBBroadcastQuery\fP or \fBIndirectQuery\fP packet that the
manger expects the display to use in the subsequent \fBRequest\fP packet.
This choice should remain as constant as feasible so that displays which
send multiple \fBQuery\fP packets can use the \fCAuthentication Name\fP from
any \fBWilling\fP packet which arrives.
.LP
The display is free to ignore managers which request an insufficient level
of authentication.
.RE
\fCHostname\fP:
ARRAY8
.RS
A human readable string describing the host from which the packet was sent.
The protocol specifies no interpretation of the data in this field.
.RE
\fCStatus\fP:
ARRAY8
.RS
A human readable string describing the ``status'' of the host.  This could
include load average/number of users connected or other information.  The
protocol specifies no interpretation of the data in this field.
.RE
.RE
Semantics:
.RS
A \fBWilling\fP packet is sent by managers that may service connections from
this display.  It is sent in response to either a \fBQuery\fP,
\fBBroadcastQuery\fP or \fBForwardQuery\fP but does not imply a commitment
to provide service (e.g. it may later decide that it has accepted enough
connections already).
.RE
Problems/Solutions:
.RS
Problem:
.RS
\fBWilling\fP not received by the display.
.br
Indication:
.RS
none if \fBBroadcastQuery\fP or \fBIndirectQuery\fP was sent, else failure to
receive \fBWilling\fP.
.RE
Solution:
.RS
The display should continue to send the query until a response is received.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from the manager to the display, this packet should
never be retransmitted.
.RE
.RE
.LP
\fBUnwilling\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
The \fCHostname\fP and \fCStatus\fP fields as in the \fBWilling\fP packet.
The \fCStatus\fP field
should indicate to the user a reason for the refusal of service.
.RE
Semantics:
.RS
An \fBUnwilling\fP packet is sent by managers in response to direct
\fBQuery\fP requests (as opposed to \fBBroadcastQuery\fP or
\fBIndirectQuery\fP requests) if the manager will not accept requests for
management.  This is typically sent by managers that wish to only service
particular displays or which handle a limited number of displays at once.
.RE
Problems/Solutions:
.RS
Problem:
.RS
\fBUnwilling\fP not received by the display.
.br
Indication:
.RS
Display fails to receive \fBUnwilling\fP.
.RE
Solution:
.RS
The display should continue to send \fBQuery\fP messages until a response is
received.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from the manager to the display, this packet should
never be retransmitted.
.RE
.RE
.LP
\fBRequest\fP
.br
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\fCDisplay Number\fP:
CARD16
.RS
The index of this particular server for the host on which the display is
resident.  This value will be zero for most autonomous displays.
.RE
\fCConnection Types\fP:
ARRAY16
.RS
An array indicating the stream services accepted by the display.  If the
high-order byte in a particular entry is zero, the low-order byte
corresponds to an X-protocol host family type.
.RE
\fCConnection Addresses\fP:
ARRAYofARRAY8
.RS
For each connection type in the previous array, the corresponding entry in
this array indicates the network address of the display device.
.RE
\fCAuthentication Name\fP:
ARRAY8
.br
\fCAuthentication Data\fP:
ARRAY8
.RS
This specifies the authentication protocol that the display expects
the manager to validate itself with.  The Authentication Data is
expected to contain data which the manager will interpret, modify
and use to authenticate itself.
.RE
\fCAuthorization Names\fP:
ARRAYofARRAY8
.RS
This array specifies which types of authorization the display supports.  The
manager may decide to reject displays with which it cannot perform
authorization.
.RE
\fCManufacturer Display ID\fP:
ARRAY8
.RS
This field can be used by the manager to determine how to decrypt the
Authentication Data field in this packet.  See the section below on
Manufacturer Display ID Format.
.RE
.RE
Semantics:
.RS
A \fBRequest\fP packet is sent by a display to a specific host to request a
session id in preparation for a establishing a connection.  If the manager
is willing to service a connection to this display, it should return an
\fBAccept\fP packet with a valid session id and should be ready for a
subsequent Manage request.  Otherwise, it should return a \fBDecline\fP
packet.
.RE
Valid Responses:
.RS
\fBAccept\fP, \fBDecline\fP
.RE
Problems/Solutions:
.RS
Problem:
.RS
Request not received by manager.
.br
Indication:
.RS
Display timeout waiting for response.
.RE
Solution:
.RS
Display resends \fBRequest\fP message.
.RE
.RE
Problem:
.RS
Message received out of order by manager.
.br
Indication:
.RS
none
.RE
Solution:
.RS
Each time a \fBRequest\fP is sent, the manager sends the \fCSession ID\fP
associated with the next session in the \fBAccept\fP.  If that next
session is not yet started, the manager will simply resend with the same
\fCSession ID\fP.  If the session is in progress, the manager will reply
with a new \fCSession ID\fP; in which case, the \fBAccept\fP will be
discarded by the display.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Timeout after 2 seconds, exponential backoff to 32 seconds.  After no more
than 126 seconds, give up and report an error to the user.
.RE
.RE
.LP
\fBAccept\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\fCSession ID\fP:
CARD32
.RS
This identifies the session which can be started by the manager.
.RE
\fCAuthentication Name\fP:
ARRAY8
.br
\fCAuthentication Data\fP:
ARRAY8
.RS
This data is sent back to the display to authenticate the manager.
If the Authentication Data is not the value expected by the display, it
should terminate the protocol at this point and display an error to the user.
.RE
\fCAuthorization Name\fP:
ARRAY8
.br
\fCAuthorization Data\fP:
ARRAY8
.RS
This data is sent to the display to indicate the type of authorization the
manager will be using in the first XOpenDisplay request after the
Manage packet is received.
.RE
.RE
Semantics:
.RS
An \fBAccept\fP packet is sent by a manager in response to a \fBRequest\fP
packet if the manager is willing to establish a connection for the display.
The \fCSession ID\fP is used to identify this connection from any preceding
ones and will be used by the display in its subsequent \fBManage\fP packet.
The \fCSession ID\fP is a 32 bit number which is incremented each time an
\fBAccept\fP packet is sent as it must be reasonably unique over a long
period of time.
.LP
If the authentication information is invalid, a \fBDecline\fP packet will be
returned with an appropriate \fCStatus\fP message.
.RE
Problems/Solutions:
.RS
Problem:
.RS
\fBAccept\fP or \fBDecline\fP not received by display.
.br
Indication:
.RS
Display timeout waiting for response to \fBRequest\fP.
.RE
Solution:
.RS
Display resends \fBRequest\fP message.
.RE
.RE
Problem:
.RS
Message received out of order by display.
.br
Indication:
.RS
Display receives \fBAccept\fP after \fBManage\fP has been sent.
.RE
Solution:
.RS
Display discards \fBAccept\fP messages after it has sent a \fBManage\fP
message.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from the manager to the display, this packet should
never be retransmitted.
.RE
.RE
.LP
\fBDecline\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\fCStatus\fP:
ARRAY8
.RS
This is a human readable string indicating the reason for refusal of
service.
.RE
\fCAuthentication Name\fP:
ARRAY8
.br
\fCAuthentication Data\fP:
ARRAY8
.RS
This data is sent back to the display to authenticate the manager.  If the
\fCAuthentication Data\fP is not the value expected by the display, it
should terminate the protocol at this point and display an error to the user.
.RE
.RE
Semantics:
.RS
A \fBDecline\fP packet is sent by a manager in response to a \fBRequest\fP
packet if the manager is unwilling to establish a connection for the
display.  This is allowed even if the manager had responded \fBWilling\fP to
a previous query.
.RE
Problems/Solutions:
.RS
same as for
\fBAccept\fP.
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from a manager to a display, this packet should never
be retransmitted.
.RE
.RE
.LP
\fBManage\fP
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\fCSession ID\fP:
CARD32
.RS
This field should contain the non-zero session id returned
in the
\fBAccept\fP
packet.
.RE
\fCDisplay Number\fP:
CARD16
.RS
This field must match the value sent in the previous
\fBRequest\fP
packet.
.RE
\fCDisplay Class\fP:
ARRAY8
.RS
This array specifies the class of the display.  Please refer to the section
below (Display Class Format) which discusses the format of this field.
.RE
.RE
Semantics:
.RS
A \fBManage\fP packet is sent by a display to ask the manager to begin a
session on the display.  If the \fCSession ID\fP is correct the manager
should open a connection, otherwise it should respond with a \fBRefuse\fP or
\fBFailed\fP packet, unless the \fCSession ID\fP matches a currently
running session, or a session which has not yet successfully opened the
display but has not given up the attempt.  In this latter case, the
\fBManage\fP packet should be ignored.  This will work as stream connections
give positive success indication to both halves of the stream, and positive
failure indication to the connection initiator (which will eventually
generate a \fBFailed\fP packet).
.RE
Valid Responses:
.RS
X connection with correct auth info,
\fBRefuse\fP,
\fBFailed\fP.
.RE
Problems/Solutions:
.RS
Problem:
.RS
\fBManage\fP
not received by manager.
.br
Indication:
.RS
Display timeout waiting for response.
.RE
Solution:
.RS
Display resends
\fBManage\fP
message.
.RE
.RE
Problem:
.RS
\fBManage\fP received out of order by manager.
.br
Indication:
.RS
session already in progress with matching \fCSession ID\fP.
.RE
Solution:
.RS
\fBManage\fP packet ignored.
.RE
Indication:
.RS
\fCSession ID\fP doesn't match next \fCSession ID\fP
.RE
Solution:
.RS
\fBRefuse\fP message is sent.
.RE
.RE
Problem:
.RS
Display cannot be opened on selected stream.
.br
Indication:
.RS
open display fails.
.RE
Solution:
.RS
\fBFailed\fP message is sent including a human readable reason.
.RE
.RE
Problem:
.RS
Display open does not succeed before a second manage packet is received
because of a timeout occuring in the display.
.br
Indication:
.RS
\fBManage\fP packet received with \fCSession ID\fP matching the session
attempting to connect to the display.
.RE
Solution:
.RS
\fBManage\fP packet is ignored.  As the stream connection will either
succeed, which will result in an active session, or the stream will
eventually give up hope of connecting and send a \fCFailed\fP packet, no
response to this \fBManage\fP packet is necessary.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Timeout after 2 seconds, exponential backoff to 32 seconds.  After no more
than 126 seconds, give up and report an error to the user.
.RE
.RE
.LP
\fBRefuse\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\fCSession ID\fP:
.RS
This field should be set to the
\fCSession ID\fP
received in the
\fBManage\fP
packet.
.RE
.RE
Semantics:
.RS
A \fBRefuse\fP packet is sent by a manager when the \fCSession ID\fP
received in the \fBManage\fP packet does not match the current \fCSession
ID\fP.  The display should assume that it received an old \fBAccept\fP
packet and should resend its \fBRequest\fP packet.
.RE
Problems/Solutions:
.RS
Problem:
.RS
Error message is lost.
.br
Indication:
.RS
display times out waiting for OpenDisplay, \fBRefuse\fP or \fBFailed\fP.
.RE
Solution:
.RS
display resends \fBManage\fP message.
.RE
.RE
.RE
Timeout/Retransmission policy:
.RS
Like all packets sent from a manager to a display, this packet should never be
retransmitted.
.RE
.RE
.LP
\fBFailed\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\fCSession ID\fP:
CARD32
.RS
This field should be set to the \fCSession ID\fP received in the
\fBManage\fP packet.
.RE
\fCStatus\fP:
ARRAY8
.RS
A human readable string indicating the reason for failure.
.RE
.RE
Semantics:
.RS
A \fBFailed\fP packet is sent by a manager when it has problems establishing
the initial X connection in response to the \fBManage\fP packet.
.RE
Problems/Solutions
.RS
Same as for \fBRefuse\fP.
.RE
.RE
.LP
\fBKeepAlive\fP
.RS
Display \(-> Manager
.br
Additional Fields:
.RS
\fCDisplay Number\fP:
CARD16
.RS
Set to the display index for the display host.
.RE
.RE
.RS
\fCSession ID\fP:
CARD32
.RS
This field should be set to the \fCSession ID\fP received in the
\fBManage\fP packet during the negotiation for the current session.
.RE
.RE
Sematics:
.RS
A \fBKeepAlive\fP packet can be sent at any time during the session by a
display to discover if the manager is running.  The manager should respond
with \fBAlive\fP whenever it receives this type of packet.
.LP
This allows the display to discover when the manager host is no longer running.
A display is not required to send \fBKeepAlive\fP packets, and, upon lack of
receipt of \fBAlive\fP packets, is not required to perform any specific action.
.LP
The expected use of this packet is to terminate an active session when the
manager host or network link fails.  The display should keep track of the
time since any packet has been received from the manager host and
use \fBKeepAlive\fP packets when a substantial time has elapsed since the
most recent packet.
.RE
Valid Responses:
.RS
\fBAlive\fP
.RE
Problems/Solutions:
.RS
Problem:
.RS
Manager doesn't receive the packet or display doesn't receive the response.
.RE
.RS
Indication:
.RS
No \fBAlive\fP packet returned
.RE
Solution:
.RS
Retransmit the packet with an exponential backoff; start at 2 seconds and
assume the host is not up after no less than 30 seconds.
.RE
.RE
.RE
.RE
.LP
\fBAlive\fP
.RS
Manager \(-> Display
.br
Additional Fields:
.RS
\fCSession Running\fP:
CARD8
.RS
This field indicates that the session indicated by \fCSession ID\fP is
currently active.  The value is 0 if no session is active, 1 if a session
is active.
.RE
\fCSession ID\fP:
CARD32
.RS
The ID of the currently running session; if any.  When no session is active
this field should be zero.
.RE
.RE
Semantics:
.RS
An \fBAlive\fP packet is sent in response to a \fBKeepAlive\fP request.  If
a session is currently active on the display, the manager includes the
\fCSession ID\fP in the packet.  The display can use this information to
determine the status of the manager.
.RE
.RE
.NH 1
Session Termination
.LP
When the session is over, the initial connection with the display (the one
which ack's the \fBManage\fP packet) will be closed by the manager.  If only
a single session was active on the display, all other connections should be
closed by the display and the display should be reset.  If multiple sessions
are active simultaneously, and the display can identify which connections
belong to the terminated sesssion, those connections should be closed.
Otherwise all connections should be closed and the display reset only when
all sessions have been terminated (i.e. all initial connections closed).
.LP
The session may also be terminated at any time by the display if the
managing host no longer responds to \fBKeepAlive\fP packets.  The exact
time-outs for sending \fBKeepAlive\fP packets is not specified in this
protocol as the trade off should not be fixed between loading an otherwise
idle system with spurious \fBKeepAlive\fP packets and not noticing that the
manager host is down for a long time.
.NH 1
State Diagrams
.LP
These state diagrams are designed to cover all actions of both
the display and the manager.  Any packet which is received out-of-sequence
will be ignored.
.LP
Display:

.RS
.LP
\fIstart\fP:
.RS
user-requested connect to one host \(-> \fIquery\fP

.br
user-requested connect to some host \(-> \fIbroadcast\fP

.br
user-requested connect to site host-list \(-> \fIindirect\fP
.RE

.LP
\fIquery\fP:
.RS
Send \fBQuery\fP packet
.br
\(-> \fIcollect-query\fP
.RE

.LP
\fIcollect-query\fP:
.RS
receive \fBWilling\fP \(-> \fIstart-connection\fP

.br
receive \fBUnwilling\fP \(-> \fIstop-connection\fP

.br
timeout \(-> \fIquery\fP
.RE	

.LP
\fIbroadcast\fP:
.RS
Send \fBBroadcastQuery\fP packet
.br
\(-> \fIcollect-broadcast-query\fP
.RE

.LP
\fIcollect-broadcast-query\fP:
.RS
receive \fBWilling\fP \(-> \fIupdate-broadcast-willing\fP

.br
user-requested connect to one host \(-> \fIstart-connection\fP

.br
timeout \(-> \fIbroadcast\fP
.RE

.LP
\fIupdate-broadcast-willing\fP:
.RS
Add new host to the host list presented to the user.
.br
\(-> \fIcollect-broadcast-query\fP
.RE

.LP
\fIindirect\fP:
.RS
Send \fBIndirectQuery\fP packet
.br
\(-> \fIcollect-indirect-query\fP
.RE

.LP
\fIcollect-indirect-query\fP:
.RS
receive \fBWilling\fP \(-> \fIupdate-indirect-willing\fP

.br
user-requested connect to one host \(-> \fIstart-connection\fP

.br
timeout \(-> \fIindirect\fP
.RE

.LP
\fIupdate-indirect-willing\fP:
.RS
Add new host to the host list presented to the user.
.br
\(-> \fIcollect-indirect-query\fP
.RE

.LP
\fIstart-connection\fP:
.RS
Send \fBRequest\fP packet
.br
\(-> \fIawait-request-response\fP
.RE

.LP
\fIawait-request-response\fP:
.RS
receive \fBAccept\fP \(-> \fImanage\fP

.br
receive \fBDecline\fP \(-> \fIstop-connection\fP

.br
timeout \(-> \fIstart-connection\fP
.RE

.LP
\fImanage\fP:
.RS
Save \fCSession ID\fP
.br
Send \fBManage\fP packet with \fCSession ID\fP
.br
\(-> \fIawait-manage-response\fP
.RE	

.LP
\fIawait-manage-response\fP:
.RS
receive XOpenDisplay: \(-> \fIrun-session\fP

.br
receive \fBRefuse\fP with matching \fCSession ID\fP \(-> \fIstart-connection\fP

.br
receive \fBFailed\fP with matching \fCSession ID\fP \(-> \fIstop-connection\fP

.br
timeout \(-> \fImanage\fP
.RE

.LP
\fIstop-connection\fP:
.RS
Display cause of termination to user
.br
\(-> \fIstart\fP

.RE	

\fIrun-session\fP:
.RS
Decide to send \fBKeepAlive\fP packet \(-> \fIkeep-alive\fP

await close of first display connection

.br
\(-> \fIreset-display\fP
.RE	

.LP
\fIkeep-alive\fP:
.RS
send \fBKeepAlive\fP packet with current \fCSession ID\fP

.br
\(-> \fIawait-alive\fP
.RE

.LP
\fIawait-alive\fP:
.RS
Receive \fBAlive\fP with matching \fCSession ID\fP \(-> \fIrun-session\fP

.br
Receive \fBAlive\fP with non-matching \fCSession ID\fP or
FALSE \fCSession Running\fP \(-> \fIreset-display\fP

.br
Final Timeout without receiving \fBAlive\fP packet \(-> \fIreset-display\fP

.br
timeout \(-> \fIkeep-alive\fP
.RE

.LP
\fIreset-display\fP:
.RS
(if possible) \(-> close all display connections associated with this session

.br
last session \(-> close all display connections

.br
\(-> \fIstart\fP
.RE

.RE
.LP
Manager:
.RS

.LP
\fIidle\fP:
.RS
receive \fBQuery\fP \(-> \fIquery-respond\fP

.br
receive \fBBroadcastQuery\fP \(-> \fIbroadcast-respond\fP

.br
receive \fBIndirectQuery\fP \(-> \fIindirect-respond\fP

.br
receive \fBForwardQuery\fP \(-> \fIforward-respond\fP

.br
receive \fBRequest\fP \(-> \fIrequest-respond\fP

.br
receive \fBManage\fP \(-> \fImanage\fP

.br
an active session terminates \(-> \fIfinish-session\fP

.br
receive \fBKeepAlive\fP \(-> \fIsend-alive\fP

.br
\(-> \fIidle\fP
.RE	

.LP
\fIquery-respond\fP:
.RS
if willing to manage \(-> \fIsend-willing\fP

.br
\(-> \fIsend-unwilling\fP
.RE

.LP
\fIbroadcast-respond\fP:
.RS
if willing to manage \(-> \fIsend-willing\fP

.br
\(-> \fIidle\fP
.RE

.LP
\fIindirect-respond\fP:
.RS
Send \fBForwardQuery\fP packets to all managers on redirect list.
.br
if willing to manage \(-> \fIsend-willing\fP

.br
\(-> \fIidle\fP
.RE

.LP
\fIforward-respond\fP:
.RS
Decode destination address, if willing to manage \(-> \fIsend-willing\fP

.br
\(-> \fIidle\fP
.RE

.LP
\fIsend-willing\fP:
.RS
Send \fBWilling\fP packet
.br
\(-> \fIidle\fP
.RE

.LP
\fIsend-unwilling\fP:
.RS
Send \fBUnwilling\fP packet
.br
\(-> \fIidle\fP
.RE

.LP
\fIrequest-respond\fP:
.RS
if manager is willing to allow a session on display \(-> \fIaccept-session\fP

.br
\(-> \fIdecline-session\fP
.RE

.LP
\fIaccept-session\fP:
.RS
Generate \fCSession ID\fP.  Save \fCSession ID\fP, display address and
display number somewhere
.br
Send \fBAccept\fP packet
.br
\(-> \fIidle\fP
.RE	

.LP
\fIdecline-session\fP:
.RS
Send \fBDecline\fP packet
.br
\(-> \fIidle\fP
.RE	

.LP
\fImanage\fP:
.RS
If \fCSession ID\fP matches saved \fCSession ID\fP \(-> \fIrun-session\fP
.br
If \fCSession ID\fP matches \fCSession ID\fP of session in process of
starting up, or currently active session \(-> \fIidle\fP

.br
\(-> \fIrefuse\fP

.RE	
.LP

\fIrefuse\fP:
.RS
Send
\fBRefuse\fP
packet
.br
\(-> 
\fIidle\fP

.RE	

.LP
\fIrun-session\fP:
.RS
Terminate any session in progress
.br
XOpenDisplay
.br
open display succeeds \(-> 
\fIstart-session\fP

.br
\(-> 
\fIfailed\fP
.RE

.LP
\fIfailed\fP:
.RS
send \fBFailed\fP packet
.br
\(-> \fIidle\fP
.RE	

.LP
\fIstart-session\fP:
.RS
Start a new session
.br
\(-> \fIidle\fP
.RE

.LP
\fIfinish-session\fP:
.RS
XCloseDisplay

.br
\(-> \fIidle\fP
.RE

.LP
\fIsend-alive\fP:
.RS
Send \fBAlive\fP packet containing current status.

.br
\(-> \fIidle\fP
.RE

.RE
.NH 1
Protocol Encoding
.LP
When XDMCP is implemented on top of UDP (the Internet User Datagram Protocol),
port number 177 is to be used.
.LP
The version number in all packets will be 1.
.LP
Packet opcodes are 16 bit integers.
.RS
.TS
c c
l l.
Packet Name	Encoding
_
BroadcastQuery	1
Query	2
IndirectQuery	3
ForwardQuery	4
Willing	5
Unwilling	6
Request	7
Accept	8
Decline	9
Manage	10
Refuse	11
Failed	12
KeepAlive	13\(dg
Alive	14\(dg
.TE
.RE
.FS \(dg
A previous version of this document incorrectly reversed the opcodes
of \fBAlive\fP and \fBKeepAlive\fP.
.FE
.LP
Per packet information follows:
.LP
\fBQuery\fP
.br
\fBBroadcastQuery\fP
.br
\fBIndirectQuery\fP
.RS
These packets are identical except for the opcode field.
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBQuery\fP, \fBBroadcastQuery\fP or \fBIndirectQuery\fP)
2	CARD16	length
1	CARD8	number of \fCAuthentication Names\fP sent (m)
2	CARD16	length of first \fCAuthentication Name\fP (m\d\s-21\s+2\u)
m\d\s-21\s+2\u	CARD8	first \fCAuthentication Name\fP
\&...		Other \fCAuthentication Names\fP
.TE
.RE
.LP
\fBForwardQuery\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBForwardQuery\fP)
2	CARD16	length
2	CARD16	length of \fCClient Address\fP (m)
m	CARD8	\fCClient Address\fP
2	CARD16	length of \fCClient Port\fP (n)
n	CARD8	\fCClient Port\fP
1	CARD8	number of \fCAuthentication Names\fP sent (o)
2	CARD16	length of first \fCAuthentication Name\fP (o\d\s-21\s+2\u)
o\d\s-21\s+2\u	CARD8	first \fCAuthentication Name\fP
\&...		Other \fCAuthentication Names\fP
.TE
.RE
.LP
\fBWilling\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBWilling\fP)
2	CARD16	length (6 + m + n + o)
2	CARD16	Length of \fCAuthentication Name\fP (m)
m	CARD8	\fCAuthentication Name\fP
2	CARD16	\fCHostname\fP length (n)
n	CARD8	\fCHostname\fP
2	CARD16	\fCStatus\fP length (o)
o	CARD8	\fCStatus\fP
.TE
.RE
.LP
\fBUnwilling\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBUnwilling\fP)
2	CARD16	length (4 + m + n)
2	CARD16	\fCHostname\fP length (m)
m	CARD8	\fCHostname\fP
2	CARD16	\fCStatus\fP length (n)
n	CARD8	\fCStatus\fP
.TE
.RE
.LP
\fBRequest\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBRequest\fP)
2	CARD16	length
2	CARD16	\fCDisplay Number\fP
1	CARD8	Count of \fCConnection Types\fP (m)
2 \(mu m	CARD16	\fCConnection Types\fP
1	CARD8	Count of \fCConnection Addresses\fP (n)
2	CARD16	Length of first \fCConnection Address\fP (n\s-2\d1\u\s+2)
n\s-2\d1\u\s+2	CARD8	First \fCConnection Address\fP
\&...		Other connection addresses
2	CARD16	Length of \fCAuthentication Name\fP (o)
o	CARD8	\fCAuthentication Name\fP
2	CARD16	Length of \fCAuthentication Data\fP (p)
p	CARD8	\fCAuthentication Data\fP
1	CARD8	Count of \fCAuthorization Names\fP (q)
2	CARD16	Length of first \fCAuthorization Name\fP (q\s-2\d1\u\s+2)
q\s-2\d1\u\s+2	CARD8	First \fCAuthorization Name\fP
\&...		Other authorization names
2	CARD16	Length of \fCManufacturer Display ID\fP (r)
r	CARD8	\fCManufacturer Display ID\fP
.TE
.RE
.LP
\fBAccept\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBAccept\fP)
2	CARD16	length (12 + n + m + o + p)
4	CARD32	\fCSession ID\fP
2	CARD16	Length of \fCAuthentication Name\fP (n)
n	CARD8	\fCAuthentication Name\fP
2	CARD16	Length of \fCAuthentication Data\fP (m)
m	CARD8	\fCAuthentication Data\fP
2	CARD16	Length of \fCAuthorization Name\fP (o)
o	CARD8	\fCAuthorization Name\fP
2	CARD16	Length of \fCAuthorization Data\fP (p)
p	CARD8	\fCAuthorization Data\fP
.TE
.RE
.LP
\fBDecline\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBDecline\fP)
2	CARD16	length (6 + m + n + o)
2	CARD16	Length of \fCStatus\fP (m)
m	CARD8	\fCStatus\fP
2	CARD16	Length of \fCAuthentication Name\fP (n)
n	CARD8	\fCAuthentication Name\fP
2	CARD16	Length of \fCAuthentication Data\fP (o)
o	CARD8	\fCAuthentication Data\fP
.TE
.RE
.LP
\fBManage\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBManage\fP)
2	CARD16	length (8 + m)
4	CARD32	\fCSession ID\fP
2	CARD16	\fCDisplay Number\fP
2	CARD16	Length of \fCDisplay Class\fP (m)
m	CARD8	\fCDisplay Class\fP
.TE
.RE
.LP
\fBRefuse\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBRefuse\fP)
2	CARD16	length (4)
4	CARD32	\fCSession ID\fP
.TE
.RE
.LP
\fBFailed\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBFailed\fP)
2	CARD16	length (6 + m)
4	CARD32	\fCSession ID\fP
2	CARD16	Length of \fCStatus\fP (m)
m	CARD8	\fCStatus\fP
.TE
.RE
.LP
\fBKeepAlive\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBKeepAlive\fP)
2	CARD16	length (6)
2	CARD16	\fCDisplay Number\fP
4	CARD32	\fCSession ID\fP
.TE
.RE
.LP
\fBAlive\fP
.RS
.TS
c c c
c l l.
Length	Type	Description
_
2	CARD16	version number (always 1)
2	CARD16	opcode (always \fBAlive\fP)
2	CARD16	length (5)
1	CARD8	\fCSession Running\fP (0: not running 1: running)
4	CARD32	\fCSession ID\fP (0: not running)
.TE
.RE
.NH 1
Display Class Format
.LP
The \fCDisplay Class\fP field of the \fBManage\fP packet is used by the
display manager to collect common sorts of
displays into manageable groups.  This field is a string encoded of
ISO-LATIN-1 characters in the following format:
.nf
.sp
.ta 1i
	ManufacturerID-ModelNumber
.fi
.sp
.LP
Both elements of this string must exclude characters of the set { \fB-\fP,
\&\fB.\fP, \fB:\fP, \fB*\fP, \fB?\fP, \fI<space>\fP }.  The ManufacturerID is a
string which should be registered with the X Consortium.  The ModelNumber is
designed to identify characteristics of the display within the manufacturer's
product line.  This string should be documented in the users manual for the
particular device.  This string should probably not be specifiable by the
display user to avoid unexpected configuration errors.
.NH 1
Manufacturer Display ID Format
.LP
To authenticate the manager, the display and manager will share a private
key.  The manager, then, must be able to discover which key to use for a
particular device.
The \fCManufacturer Display ID\fP field of the \fBRequest\fP packet is
intended for this purpose.  Typically, the manager host will
contain a map between this number and the key.  This field is intended to be
unique per display, possibly the ethernet address of the display in the form:
.nf
.sp
.ta 1i
	-Ethernet-8:0:2b:a:f:d2
.sp
.fi
or string
of the form:
.nf
.sp
.ta 1i
	ManufacturerID-ModelNumber-SerialNumber
.sp
.fi
where ManufacturerID, ModelNumber and SerialNumber are encoded using
ISO-LATIN-1 characters, excluding  { \fB-\fP,
\&\fB.\fP, \fB*\fP, \fB?\fP, \fI<space>\fP }
.LP
When the display is shipped to a customer, it should include both the
\fCManufacturer Display ID\fP
and the private key in the documentation set.  This information should not
be modifiable by the display user.
.NH 1
Authentication
.LP
In an environment where authentication is not needed, XDMCP can disable
authentication by having the display send empty \fCAuthentication Name\fP
and \fCAuthentication Data\fP fields in the \fBRequest\fP packet.  In this
case, the manager will not attempt to authenticate itself.  Other
authentication protocols may be developed, depending on local needs.
.LP
In an unsecure environment, the display must be able to verify that the
source of the various packets is a trusted manager.  These packets will
contain authentication information.  As an example of such a system, the
following discussion describes the "XDM-AUTHENTICATION-1" authentication
system.  This system uses a 56 bit shared private key, and 64 bits of
authentication data.  An associated example X authorization protocol
"XDM-AUTHORIZATION-1" will also be discussed.  The 56 bit key is represented
as a 64 bit number in network order (big endian).  This means that the first
octet in the representation will be zero.  When incrementing a 64 bit value,
the 8 octets of data will be interpreted in network order (big endian). I.e.
the last octet will be incremented, subsequent carries propogate towards the
first octet.
.LP
Assumptions:
.IP
The display and manager share a private key.  This key could be programmed
into the display by the manufacturer and shipped with the unit.  It must not
be available from the display itself, but should allow the value to be
modified in some way.  The system administrator would be responsible for
managing a database of terminal keys.
.IP
The display can generate random authentication numbers.
.LP
Some definitions first:
.EQ
oc D cc sup kappa mark = "encryption of plain text " D " by key " kappa
.EN C
.EQ
oc DELTA cc * sup kappa lineup = "decryption of crypto text " DELTA " with key " kappa
.EN C
.EQ
{ tau } lineup = "private key shared by display and manager"
.EN C
.EQ
rho lineup = "64 bit random number generated by display"
.EN C
.EQ
alpha lineup = "authentication data in XDMCP packets"
.EN C
.EQ
sigma lineup = "per-session private key, generated by manager"
.EN C
.EQ
beta lineup = "authorization data"
.EN
.LP
Encryption will use the DES; blocks shorter than 64 bits will be zero-filled
on the right to 64 bits.  Blocks longer than 64 bits will use block chaining:
.EQ
oc { D } cc sup kappa lineup = oc { D sub 1 } cc sup kappa " "
oc { D sub 2 } " " xor " " oc { D sub 1 } cc sup kappa cc sup kappa
.EN
.LP
The display generates the first authentication data in the
\fBRequest\fP
packet:
.EQ
alpha sub roman Request mark = oc rho cc sup tau
.EN
.LP
For the
\fBAccept\fP
packet, the manager decrypts the initial message and returns
@alpha sub roman Accept@:
.EQ
rho lineup = oc alpha sub roman Request cc * sup tau
.EN C
.EQ
alpha sub roman Accept lineup = oc rho + 1 cc sup tau
.EN
.LP
The \fBAccept\fP packet also contains the authorization intended for use by
the X server.  A description of authorization type ``XDM-AUTHORIZATION-1''
follows:
.LP
The \fBAccept\fP packet contains the authorization name
``XDM-AUTHORIZATION-1''.  The authorization data is the string:
.EQ
beta sub Accept mark = oc sigma cc sup tau
.EN
.LP
To create authorization information for connection setup with the X server
using the XDM-AUTHORIZATION-1 authorization protocol, the client computes the
following:
.EQ
N mark = "X client identifier"
.EN C
.EQ
T lineup = "Current time in seconds on client host (32 bits)"
.EN
.EQ C
beta lineup = oc rho N T cc sup sigma
.EN
.LP
For TCP connections @N@ is 48 bits long and contains the 32 bit IP address of
the client host followed by the 16 bit port number of the client socket.
Formats for other connections must be registered.
The resulting value, @beta@, is 192 bits of authorization data which is sent
in the connection setup to the server.  The server receives the packet,
decrypts the contents.  To accept the connection, the following must hold:
.IP 1
@rho@ must match the value generated for the most recent XDMCP negotiation.
.IP 2
@T@ must be within 1200 seconds of the internally stored time.  If no time
been received before, the current time is set to @T@.
.IP 3
No packet containing the same pair (@N@, @T@) can have been received
in the last 1200 seconds (20 minutes).
