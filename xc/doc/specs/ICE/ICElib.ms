.\" $XConsortium: ICElib.ms /main/20 1996/02/06 10:43:13 swick $
.\" 
.\" Use tbl, -ms, and macros.t
.\" 
.\" macro: start marker
.de sM
.ne 4
.sp 1
\\h'-0.3i'\\L'-1v'\\v'3p'\\l'1v'\\v'1v-3p'
.sp -1
..
.\" macro: end marker
.de eM
.sp -1
\\h'-0.3i'\\L'-1v'\\v'1v+4p'\\l'1v'\\v'-4p'
.sp 1
..
.EH ''''
.OH ''''
.EF ''''
.OF ''''
.ad b
.sp 10
.TL
\s+2\fBInter-Client Exchange (ICE) Library\fP\s-2
.sp
Version 1.0
.sp
X Consortium Standard
.sp
X Version 11, Release 6.1
.AU
Ralph Mor
.AI
X Consortium
.LP
.DS C
Copyright \(co 1993, 1994 X Consortium
.DE
.LP
.sp 5
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
.sp 5
X Window System is a trademark of X Consortium, Inc.
.bp
.EH '\fBInter-Client Exchange Library\fP''\fBX11, Release 6.1\fP'
.OH '\fBInter-Client Exchange Library\fP''\fBX11, Release 6.1\fP'
.bp 1
.EF ''\- \\\\n(PN \-''
.OF ''\- \\\\n(PN \-''
.NH 1
Acknowledgements
.XS
\*(SN Acknowledgements
.XE
.LP
Thanks to Bob Scheifler for his thoughtful input on the design
of the ICE library.  Thanks also to Jordan Brown, Larry Cable, Donna Converse,
Clive Feather, Stephen Gildea, Vania Joloboff, Kaleb Keithley,
Stuart Marks, Hiro Miyamoto, Ralph Swick, Jim VanGilder, and Mike Wexler.
.LP
.NH 1
Overview of ICE
.XS
\*(SN Overview of ICE
.XE
.LP
There are numerous possible "inter-client" protocols, with many similarities
and common needs - authentication, version negotiation, byte
order negotiation, etc.  The ICE protocol is intended to provide a framework
for building such protocols, allowing them to make use of common negotiation
mechanisms and to be multiplexed over a single transport connection.
.LP
.NH 1
The ICE Library - A "C" Language Interface to ICE
.XS
\*(SN The ICE Library - A "C" Language Interface to ICE
.XE
.LP
A client that wishes to utilize ICE must first register the protocols it
understands with the ICE library.  Each protocol is dynamically assigned
a major opcode ranging from 1-255 (two clients can use different
major opcodes for the same protocol).  The next step for the client is to
either open a connection with another client, or to wait for connections made
by other clients.  Authentication may be required.  A client can both
initiate connections with other clients and be
waiting for clients to connect to itself (a nested session manager is an
example).  Once an ICE connection is established between the two clients, one
of the clients needs to initiate a \fIProtocol Setup\fP\^ in order to
"activate" a given protocol.  Once the other client accepts the
\fIProtocol Setup\fP\^ (once again, authentication may be required), the
two clients are ready to start passing messages specific to that protocol to
each other.  Multiple protocols may be active on a single ICE connection.
Clients are responsible for notifying the ICE library when a protocol is no
longer active on an ICE connection, although ICE does not define how each
sub-protocol triggers a protocol shutdown.
.LP
The ICE library utilizes callbacks to process incoming messages.  Using
callbacks allows \fIProtocol Setups\fP\^ and authentication to happen
"behind the scenes."  An additional benefit is that messages never need
to be buffered up by the library when the client "blocks" waiting for a
particular message.
.LP
.NH 1
Intended Audience
.XS
\*(SN Intended Audience
.XE
.LP
This document is intended primarily for implementors of protocol libraries
layered on top of ICE.  Typically, applications that wish to utilize ICE
will make calls into individual protocol libraries rather than directly
make calls into the ICE library.  However, some applications will have to
make some initial calls into the ICE library in order to accept ICE
connections (for example, a session manager accepting connections from
clients).  But in general, protocol libraries should be designed to hide
the inner details of ICE from applications.
.LP
.NH 1
Header Files and Library Name
.XS
\*(SN Header Files and Library Name
.XE
.LP
The header file \fI<X11/ICE/ICElib.h>\fP\^ defines all of the ICElib data
structures and function prototypes.  \fIICElib.h\fP\^ includes the
header file \fI<X11/ICE/ICE.h>\fP\^ which defines all of the ICElib constants.
Protocol libraries that need to read and write messages should include
the header file \fI<X11/ICE/ICEmsg.h>\fP\^.
.LP
Applications should link against ICElib using \fI-lICE\fP\^.
.LP
.NH 1
Note on prefixes
.XS
\*(SN Note on prefixes
.XE
.LP
The following name prefixes are used in the library to distinguish between
a client that initiates a \fIProtocol Setup\fP\^ and a client which
responds with a \fIProtocol Reply\fP\^:
.LP
.sp 0.5
.PN IcePo
- \fIIce Protocol Originator\fP\^
.LP
.PN IcePa
- \fIIce Protocol Acceptor\fP\^
.LP
.bp
.NH 1
Protocol Registration
.XS
\*(SN Protocol Registration
.XE
.LP
In order for two clients to exchange messages for a given protocol, each
side must register the protocol with the ICE library.  The purpose of
registration is for each side to obtain a major opcode for the protocol,
and to provide callbacks for processing messages and handling authentication.
There are two separate registration functions - one to handle the side that
does a \fIProtocol Setup\fP\^, and one to handle the side that responds with a
\fIProtocol Reply\fP\^.
.LP
It is recommended that protocol registration occur before the two clients
establish an ICE connection.  If protocol registration occurs after an
ICE connection is created, there can be a brief interval of time in which
a \fIProtocol Setup\fP\^ is received, but the protocol is not registered.
If it is not possible to register a protocol before the creation of an
ICE connection, proper precautions should be taken to avoid the above race
condition.
.LP
The
.PN IceRegisterForProtocolSetup
function should be called for the client that initiates a
\fIProtocol Setup\fP\^.
.LP
.sM
.FD 0
int IceRegisterForProtocolSetup\^(\^\fIprotocol_name\fP, \fIvendor\fP\^, \fIrelease\fP\^,
.br
                    \fIversion_count\fP\^, \fIversion_recs\fP\^, \fIauth_count\fP\^, \fIauth_names\fP\^, \fIauth_procs\fP\^, \fIio_error_proc\fP\^)
.br
     char *\fIprotocol_name\fP\^;
.br
     char *\fIvendor\fP\^;
.br
     char *\fIrelease\fP\^;
.br
     int \fIversion_count\fP\^;
.br
     IcePoVersionRec *\fIversion_recs\fP\^;
.br
     int \fIauth_count\fP\^;
.br
     char **\fIauth_names\fP\^;
.br
     IcePoAuthProc *\fIauth_procs\fP\^;
.br
     IceIOErrorProc \fIio_error_proc\fP\^;
.FN
.IP \fIprotocol_name\fP 1i
A string specifying the name of the protocol to register.
.IP \fIvendor\fP 1i
A vendor string with semantics specified by the protocol.
.IP \fIrelease\fP 1i
A release string with semantics specified by the protocol.
.IP \fIversion_count\fP 1i
The number of different versions of the protocol supported.
.IP \fIversion_recs\fP 1i
List of versions and associated callbacks.
.IP \fIauth_count\fP 1i
The number of authentication methods supported.
.IP \fIauth_names\fP 1i
The list of authentication methods supported.
.IP \fIauth_procs\fP 1i
The list of authentication callbacks, one for each authentication method.
.IP \fIio_error_proc\fP 1i
IO Error handler.
.LP
.eM
.PN IceRegisterForProtocolSetup
returns the major opcode reserved, or -1 if an error occurred.  In order
to actually activate the protocol, the
.PN IceProtocolSetup
function needs to be called with this major opcode.  Once the protocol is
activated, all messages for the protocol should be sent using this major
opcode.
.LP
A protocol library may support multiple versions of the same protocol.
\fIversion_recs\fP\^ specifies a list of supported versions of the protocol,
prioritized in decreasing order of preference.  Each version record
consists of a major and minor version of the protocol, as well as a
callback to be used for processing incoming messages.
.LP
.sM
typedef struct {
.br
	int major_version;
.br
	int minor_version;
.br
	IcePoProcessMsgProc process_msg_proc;
.br
} IcePoVersionRec;
.LP
.eM
The
.PN IcePoProcessMsgProc
callback is responsible for processing the set of messages that can be
received by the client that initiated the \fIProtocol Setup\fP\^.
The details of how this callback works is described in the section titled
\fICallbacks for Processing Messages\fP\^.
.LP
Authentication may be required before the protocol can become active.
The protocol library must register the authentication methods that it
supports with the ICE library.  \fIauth_names\fP and \fIauth_procs\fP
are a list of authentication names and callbacks, prioritized in decreasing
order of preference.  The details of how the
.PN IcePoAuthProc
callback works is described in the section titled
\fIAuthentication Methods\fP\^.
.LP
The
.PN IceIOErrorProc
callback is invoked if the ICE connection unexpectedly breaks.  Pass
.PN NULL
for \fIio_error_proc\fP\^ if not interested in being notified.
See the section titled \fIError Handling\fP\^ for more details on
this callback.
.LP
.sp
The
.PN IceRegisterForProtocolReply
function should be called for the client that responds to a
\fIProtocol Setup\fP\^ with a \fIProtocol Reply\fP\^.
.sM
.FD 0
int IceRegisterForProtocolReply\^(\^\fIprotocol_name\fP, \fIvendor\fP\^, \fIrelease\fP\^, \fIversion_count\fP\^, \fIversion_recs\fP\^,
.br
                    \fIauth_count\fP\^, \fIauth_names\fP\^, \fIauth_procs\fP\^, \fIhost_based_auth_proc\fP\^,
.br
                    \fIprotocol_setup_proc\fP\^, \fIprotocol_activate_proc\fP\^, \fIio_error_proc\fP\^)
.br
     char *\fIprotocol_name\fP\^;
.br
     char *\fIvendor\fP\^;
.br
     char *\fIrelease\fP\^;
.br
     int \fIversion_count\fP\^;
.br
     IcePaVersionRec *\fIversion_recs\fP\^;
.br
     int \fIauth_count\fP\^;
.br
     char **\fIauth_names\fP\^;
.br
     IcePaAuthProc *\fIauth_procs\fP\^;
.br
     IceHostBasedAuthProc \fIhost_based_auth_proc\fP\^;
.br
     IceProtocolSetupProc \fIprotocol_setup_proc\fP\^;
.br
     IceProtocolActivateProc \fIprotocol_activate_proc\fP\^;
.br
     IceIOErrorProc \fIio_error_proc\fP\^;
.FN
.IP \fIprotocol_name\fP 1i
A string specifying the name of the protocol to register.
.IP \fIvendor\fP 1i
A vendor string with semantics specified by the protocol.
.IP \fIrelease\fP 1i
A release string with semantics specified by the protocol.
.IP \fIversion_count\fP 1i
The number of different versions of the protocol supported.
.IP \fIversion_recs\fP 1i
List of versions and associated callbacks.
.IP \fIauth_count\fP 1i
The number of authentication methods supported.
.IP \fIauth_names\fP 1i
The list of authentication methods supported.
.IP \fIauth_procs\fP 1i
The list of authentication callbacks, one for each authentication method.
.IP \fIhost_based_auth_proc\fP 1i
Host based authentication callback.
.IP \fIprotocol_setup_proc\fP 1i
A callback to be invoked when authentication has succeeded for a
\fIProtocol Setup\fP\^, \fIbefore\fP\^ the \fIProtocol Reply\fP\^ is sent.
.IP \fIprotocol_activate_proc\fP 1i
A callback to be invoked \fIafter\fP\^ the \fIProtocol Reply\fP\^ is sent.
.IP \fIio_error_proc\fP 1i
IO Error handler.
.LP
.eM
.PN IceRegisterForProtocolReply
returns the major opcode reserved, or -1 if an error occurred.  The major
opcode should be used in all subsequent messages sent for this protocol.
.LP
A protocol library may support multiple versions of the same protocol.
\fIversion_recs\fP\^ specifies a list of supported versions of the protocol,
prioritized in decreasing order of preference.  Each version record
consists of a major and minor version of the protocol, as well as a
callback to be used for processing incoming messages.
.LP
.sM
typedef struct {
.br
	int major_version;
.br
	int minor_version;
.br
	IcePaProcessMsgProc process_msg_proc;
.br
} IcePaVersionRec;
.LP
.eM
The
.PN IcePaProcessMsgProc
callback is responsible for processing the set of messages that can be
received by the client that accepted the \fIProtocol Setup\fP\^.
The details of how this callback works is described in the section titled
\fICallbacks for Processing Messages\fP\^.
.LP
Authentication may be required before the protocol can become active.
The protocol library must register the authentication methods that it
supports with the ICE library.  \fIauth_names\fP and \fIauth_procs\fP
are a list of authentication names and callbacks, prioritized in decreasing
order of preference.  The details of how the
.PN IcePaAuthProc
callback works is described in the section titled
\fIAuthentication Methods\fP\^.
.LP
If authentication fails and the client attempting to initiate
the \fIProtocol Setup\fP\^ has not required authentication, the
.PN IceHostBasedAuthProc
callback is invoked with the host name of the originating client.
If the callback returns
.PN True ,
the \fIProtocol Setup\fP\^ will succeed, even though the original
authentication failed.  Note that authentication can effectively be
disabled by registering an
.PN IceHostBasedAuthProc
which always returns
.PN True .
If no host based
authentication is allowed, pass
.PN NULL
for \fIhost_based_auth_proc\fP\^.
.LP
.sM
typedef Bool (*IceHostBasedAuthProc) ();
.FD 0
Bool HostBasedAuthProc\^(\^\fIhost_name\fP\^)
.br
    char *\fIhost_name\fP\^;
.FN
.IP \fIhost_name\fP 1i
The host name of the client that sent the \fIProtocol Setup\fP\^.
.LP
.eM
\fIhost_name\fP\^ is a string of the form "protocol/hostname", where
\fIprotocol\fP\^ is one of {tcp, decnet, local}.
.LP
.sp 0.5
Since \fIProtocol Setups\fP\^ and authentication happen "behind the scenes"
via callbacks, the protocol library needs some way of being notified when the
\fIProtocol Setup\fP\^ has completed.  This occurs in two phases.  In the
first phase, the
.PN IceProtocolSetupProc
callback is invoked after authentication has
successfully completed, \fIbefore\fP the ICE library sends a
\fIProtocol Reply\fP\^.  Any resources required for this protocol should be
allocated at this time.  If the
.PN IceProtocolSetupProc
returns a successful status, the ICE library will
send the \fIProtocol Reply\fP\^ and then invoke the
.PN IceProtocolActivateProc
callback.  Otherwise, an error will be sent to the
other client in response to the \fIProtocol Setup\fP\^.
.LP
The
.PN IceProtocolActivateProc
is an optional callback, and should be registered only if the protocol
library intends to generate a message immediately following the
\fIProtocol Reply\fP\^.  Pass
.PN NULL
for \fIprotocol_activate_proc\fP\^ if not interested in this callback.
.LP
.sM
typedef Status (*IceProtocolSetupProc) ();
.FD 0
Status ProtocolSetupProc\^(\^\fIice_conn\fP, \fImajor_version\fP\^, \fIminor_version\fP\^, \fIvendor\fP\^, \fIrelease\fP\^,
.br
                    \fIclient_data_ret\fP\^, \fIfailure_reason_ret\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fImajor_version\fP\^;
.br
     int \fIminor_version\fP\^;
.br
     char *\fIvendor\fP\^;
.br
     char *\fIrelease\fP\^;
.br
     IcePointer *\fIclient_data_ret\fP\^;
.br
     char **\fIfailure_reason_ret\fP\^;
.FN
.IP \fIice_conn\fP 1.2i
The ICE connection object.
.IP \fImajor_version\fP 1.2i
The major version of the protocol.
.IP \fIminor_version\fP 1.2i
The minor version of the protocol.
.IP \fIvendor\fP 1.2i
The vendor string registered by the protocol originator.
.IP \fIrelease\fP 1.2i
The release string registered by the protocol originator.
.IP \fIclient_data_ret\fP 1.2i
Client data to be set by callback.
.IP \fIfailure_reason_ret\fP 1.2i
Failure reason returned.
.LP
.eM
The pointer stored in the \fIclient_data_ret\fP argument will be passed
to the
.PN IcePaProcessMsgProc
callback whenever a message has arrived for this protocol on the
ICE connection.
.LP
The \fIvendor\fP\^ and \fIrelease\fP\^ strings should be freed with free()
when they are no longer needed.
.LP
If a failure occurs, the
.PN IceProtocolSetupProc
should return a zero status, as well as allocate and return a failure
reason string in \fIfailure_reason_ret\fP\^.  The ICE library will be
responsible for freeing this memory.
.LP
.sp 0.5
The
.PN IceProtocolActivateProc
discussed above is defined as follows:
.LP
.sM
typedef void (*IceProtocolActivateProc)();
.LP
.FD 0
void ProtocolActivateProc\^(\^\fIice_conn\fP, \fIclient_data\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection object.
.IP \fIclient_data\fP 1i
The client data set in the
.PN IceProtocolSetupProc
callback.
.LP
.eM
.LP
.sp 0.5
The
.PN IceIOErrorProc
callback is invoked if the ICE connection unexpectedly breaks.  Pass
.PN NULL
for \fIio_error_proc\fP\^ if not interested in being notified.
See the section titled \fIError Handling\fP\^ for more details on
this callback.
.LP
.NH 2
Callbacks for Processing Messages
.XS
\*(SN Callbacks for Processing Messages
.XE
.LP
When an application detects that there is a new data to read on an ICE
connection (via select), it calls the
.PN IceProcessMessages
function (discussed in the section titled \fIProcessing Messages\fP\^).
When
.PN IceProcessMessages
reads an ICE message header with a major opcode other than
zero (reserved for the ICE protocol), it needs to call a function which will
read the rest of the message, unpack it, and process it accordingly.
.LP
If the message arrives at the client which initiated the
\fIProtocol Setup\fP\^, the
.PN IcePoProcessMsgProc
callback is invoked.
.LP
.sM
typedef void (*IcePoProcessMsgProc)();
.FD 0
void PoProcessMsgProc\^(\^\fIice_conn\fP, \fIclient_data\fP\^, \fIopcode\fP\^, \fIlength\fP\^, \fIswap\fP\^, \fIreply_wait\fP\^, \fIreply_ready_ret\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.br
    int \fIopcode\fP\^;
.br
    unsigned long \fIlength\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    IceReplyWaitInfo *\fIreply_wait\fP\^;
.br
    Bool *\fIreply_ready_ret\fP\^;
.FN
.bp
.IP \fIice_conn\fP 1i
The ICE connection object.
.IP \fIclient_data\fP 1i
Client data associated with this protocol on the ICE connection.
.IP \fIopcode\fP 1i
The minor opcode of the message.
.IP \fIlength\fP 1i
The length (in 8 byte units) of the message beyond the ICE header.
.IP \fIswap\fP 1i
A flag which indicates if byte swapping is necessary.
.IP \fIreply_wait\fP 1i
Indicates if the invoking client is waiting for a reply.
.IP \fIreply_ready_ret\fP 1i
If set to
.PN True ,
a reply is ready.
.LP
.eM
If the message arrives at the client which accepted
the \fIProtocol Setup\fP\^, the
.PN IcePaProcessMsgProc
callback is invoked.
.LP
.sM
typedef void (*IcePaProcessMsgProc)();
.FD 0
void PaProcessMsgProc\^(\^\fIice_conn\fP, \fIclient_data\fP\^, \fIopcode\fP\^, \fIlength\fP\^, \fIswap\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.br
    int \fIopcode\fP\^;
.br
    unsigned long \fIlength\fP\^;
.br
    Bool \fIswap\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection object.
.IP \fIclient_data\fP 1i
Client data associated with this protocol on the ICE connection.
.IP \fIopcode\fP 1i
The minor opcode of the message.
.IP \fIlength\fP 1i
The length (in 8 byte units) of the message beyond the ICE header.
.IP \fIswap\fP 1i
A flag which indicates if byte swapping is necessary.
.LP
.eM
In order to read the message, both of the above callbacks should use the
macros defined in the section of this document titled
\fIReading ICE Messages\fP\^.  Note that byte swapping may be necessary.  As a
convenience, the length field in the ICE header will be swapped by ICElib
if necessary.
.LP
In both of the above callbacks, \fIclient_data\fP\^ is a pointer to client
data that was registered at \fIProtocol Setup\fP\^ time.  In the case of
.PN IcePoProcessMsgProc ,
the client data was set in the call to
.PN IceProtocolSetup .
In the case of
.PN IcePaProcessMsgProc ,
the client data was set in the
.PN IceProtocolSetupProc
callback.
.LP
The
.PN IcePoProcessMsgProc
callback needs to check the \fIreply_wait\fP\^ argument.
If \fIreply_wait\fP\^ is
.PN NULL ,
the ICE library expects the function to
pass the message to the client via a callback.  For example, if
this is a Session Management \fISave Yourself\fP\^ message, this function
should notify the client of the \fISave Yourself\fP\^ via a callback.  The
details of how such a callback would be defined is implementation dependent.
.LP
However, if \fIreply_wait\fP\^ is not
.PN NULL ,
then the client is waiting for
a reply or an error for a message it previously sent.  \fIreply_wait\fP\^
is of type
.PN IceReplyWaitInfo .
.LP
.sM
typedef struct {
.br
	unsigned long sequence_of_request;
.br
	int major_opcode_of_request;
.br
	int minor_opcode_of_request;
.br
	IcePointer reply;
.br
} IceReplyWaitInfo;
.LP
.eM
.PN IceReplyWaitInfo
contains the major/minor opcodes and sequence number of
the message for which a reply is being awaited.  It also contains
a pointer to the reply message to be filled in (the protocol library
should cast this
.PN IcePointer
to the appropriate reply type).  In most
cases, the reply will have some fixed-size part, and the client waiting
for the reply will have provided a pointer to a structure to hold
this fixed-size data.  If there is variable-length data, it would be
expected that the
.PN IcePoProcessMsgProc
callback will have to allocate additional
memory and store pointer(s) to that memory in the fixed-size
structure.  If the entire data is variable length (e.g., a single
variable-length string), then the client waiting for the reply would probably
just pass a pointer to fixed-size space to hold a pointer, and the
.PN IcePoProcessMsgProc
callback would allocate the storage and store the pointer.
It is the responsibility of the client receiving the reply to
free any memory allocated on its behalf.
.LP
If \fIreply_wait\fP\^ is not
.PN NULL
and
.PN IcePoProcessMsgProc
has a reply or error to return in response to this \fIreply_wait\fP\^
(i.e. no callback was generated), then the \fIreply_ready_ret\fP\^ argument
should be set to
.PN True .
Note that an error should only be returned
if it corresponds to the reply being waited for.  Otherwise, the
.PN IcePoProcessMsgProc
should either handle the error internally, or invoke an error handler
for its library.
.LP
If \fIreply_wait\fP\^ is
.PN NULL ,
then care must be taken not to store any value in \fIreply_ready_ret\fP\^
since this pointer may also be
.PN NULL .
.LP
.sp 0.5       
The
.PN IcePaProcessMsgProc
callback, on the other hand, should always pass
the message to the client via a callback.  For example, if this is a Session
Management \fIInteract Request\fP\^ message, this function should notify the
client of the \fIInteract Request\fP\^ via a callback.
.LP
The reason the
.PN IcePaProcessMsgProc
callback does not have a \fIreply_wait\fP\^ like
.PN IcePoProcessMsgProc
does, is because a process that is acting as
a "server" should never block for a reply (infinite blocking can
occur if the connecting client does not act properly, denying access
to other clients).
.LP
.NH 2
Authentication Methods
.XS
\*(SN Authentication Methods
.XE
.LP
As discussed earlier, a protocol library must register the authentication
methods that it supports with the ICE library.  For each authentication
method, there are two callbacks that may be registered - one to handle
the side that initiates a \fIProtocol Setup\fP\^, and one to handle the
side that accepts or rejects this request.
.LP
.PN IcePoAuthProc
is the callback invoked for the client that initiated
the \fIProtocol Setup\fP\^.  This callback must be able to respond
to the initial \fIAuthentication Required\fP\^ message or subsequent
\fIAuthentication Next Phase\fP\^ messages sent by the other client.
.LP
.sM
typedef IcePoAuthStatus (*IcePoAuthProc)();
.FD 0
IcePoAuthStatus PoAuthProc\^(\^\fIice_conn\fP, \fIauth_state_ptr\fP\^, \fIclean_up\fP\^, \fIswap\fP\^,
.br
                    \fIauth_datalen\fP\^, \fIauth_data\fP\^, \fIreply_datalen_ret\fP\^, \fIreply_data_ret\fP\^, \fIerror_string_ret\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer *\fIauth_state_ptr\fP\^;
.br
    Bool \fIclean_up\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIauth_datalen\fP\^;
.br
    IcePointer \fIauth_data\fP\^;
.br
    int *\fIreply_datalen_ret\fP\^;
.br
    IcePointer *\fIreply_data_ret\fP\^;
.br
    char **\fIerror_string_ret\fP\^;
.FN
.IP \fIice_conn\fP 1.1i
The ICE connection object.
.IP \fIauth_state_ptr\fP 1.1i
A pointer to state for use by the authentication callback procedure.
.IP \fIclean_up\fP 1.1i
If
.PN True ,
authentication is over, and the function
should clean up any state it was maintaining.  The
last 6 arguments should be ignored.
.IP \fIswap\fP 1.1i
If
.PN True ,
the \fIauth_data\fP\^ may have to be byte swapped
(depending on its contents).
.IP \fIauth_datalen\fP 1.1i
The length (in bytes) of the authenticator data.
.IP \fIauth_data\fP 1.1i
The data from the authenticator.
.IP \fIreply_datalen_ret\fP 1.1i
The length (in bytes) of the reply data returned.
.IP \fIreply_data_ret\fP 1.1i
The reply data returned.
.IP \fIerror_string_ret\fP 1.1i
If the authentication procedure encounters an error during
authentication, it should allocate and return
an error string.
.LP
.eM
Authentication may require several phases, depending on the authentication
method.  As a result, the
.PN IcePoAuthProc
may be called more than once when authenticating a client, and
some state will have to be maintained between each invocation.
At the start of each \fIProtocol Setup\fP\^, *\fIauth_state_ptr\fP\^ is
.PN NULL ,
and the function should initialize its state and set
this pointer.  In subsequent invocations of the callback, the pointer
should be used to get at any state previously stored by the callback.
.LP
If needed, the network ID of the client accepting the \fIProtocol Setup\fP\^
can be obtained by calling the
.PN IceConnectionString
function.
.LP
ICElib will be responsible for freeing the \fIreply_data_ret\fP and
\fIerror_string_ret\fP pointers with free().
.LP
The \fIauth_data\fP\^ pointer may point to a volatile block of memory.
If the data must be kept beyond this invocation of the callback, be sure
to make a copy of it.
.LP
.sp 0.5
The
.PN IcePoAuthProc
should return one of four values:
.LP
.TS
lw(2i) lw(5i).
T{
.PN IcePoAuthHaveReply
T}	T{
A reply is available
T}
.sp 4p
T{
.PN IcePoAuthRejected
T}	T{
Authentication rejected
T}
.sp 4p
T{
.PN IcePoAuthFailed
T}	T{
Authentication failed
T}
.sp 4p
T{
.PN IcePoAuthDoneCleanup
T}	T{
Done cleaning up
T}
.TE
.LP
.sp 0.5
.PN IcePaAuthProc
is the callback invoked for the client that received the
\fIProtocol Setup\fP\^.
.LP
.sM
typedef IcePaAuthStatus (*IcePaAuthProc) ();
.FD 0
IcePaAuthStatus PaAuthProc\^(\^\fIice_conn\fP, \fIauth_state_ptr\fP\^, \fIswap\fP\^,
.br
                    \fIauth_datalen\fP\^, \fIauth_data\fP\^, \fIreply_datalen_ret\fP\^, \fIreply_data_ret\fP\^, \fIerror_string_ret\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer *\fIauth_state_ptr\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIauth_datalen\fP\^;
.br
    IcePointer \fIauth_data\fP\^;
.br
    int *\fIreply_datalen_ret\fP\^;
.br
    IcePointer *\fIreply_data_ret\fP\^;
.br
    char **\fIerror_string_ret\fP\^;
.FN
.IP \fIice_conn\fP 1.1i
The ICE connection object.
.IP \fIauth_state_ptr\fP 1.1i
A pointer to state for use by the authentication callback procedure.
.IP \fIswap\fP 1.1i
If
.PN True ,
the \fIauth_data\fP\^ may have to be byte swapped
(depending on its contents).
.IP \fIauth_datalen\fP 1.1i
The length (in bytes) of the protocol originator authentication data.
.IP \fIauth_data\fP 1.1i
The authentication data from the protocol originator.
.IP \fIreply_datalen_ret\fP 1.1i
The length of the authentication data returned.
.IP \fIreply_data_ret\fP 1.1i
The authentication data returned.
.IP \fIerror_string_ret\fP 1.1i
If authentication is rejected or fails, an error
string is returned.
.LP
.eM
.LP
Authentication may require several phases, depending on the authentication
method.  As a result, the
.PN IcePaAuthProc
may be called more than once when authenticating a client, and
some state will have to be maintained between each invocation.
At the start of each \fIProtocol Setup\fP\^, \fIauth_datalen\fP is zero,
*\fIauth_state_ptr\fP\^ is
.PN NULL ,
and the function should initialize its state and set
this pointer.  In subsequent invocations of the callback, the pointer
should be used to get at any state previously stored by the callback.
.LP
If needed, the network ID of the client accepting the \fIProtocol Setup\fP\^
can be obtained by calling the
.PN IceConnectionString
function.
.LP
The \fIauth_data\fP\^ pointer may point to a volatile block of memory.
If the data must be kept beyond this invocation of the callback, be sure
to make a copy of it.
.LP
ICElib will be responsible for freeing the \fIreply_data_ret\fP and
\fIerror_string_ret\fP pointers with free().
.LP
.sp 0.5
The
.PN IcePaAuthProc
should return one of four values:
.LP
.TS
lw(2i) lw(5i).
T{
.PN IcePaAuthContinue
T}	T{
Continue (or start) authentication
T}
.sp 4p
T{
.PN IcePaAuthAccepted
T}	T{
Authentication accepted
T}
.sp 4p
T{
.PN IcePaAuthRejected
T}	T{
Authentication rejected
T}
.sp 4p
T{
.PN IcePaAuthFailed
T}	T{
Authentication failed
T}
.TE
.LP
.NH 1
ICE Connections
.XS
\*(SN ICE Connections
.XE
.LP
In order for two clients to establish an ICE connection, one client has
to be "waiting" for connections, and the other client has to initiate the
connection.  Most clients will initiate connections, so we discuss that first.
.LP
.NH 2
Opening an ICE Connection
.XS
\*(SN Opening an ICE Connection
.XE
.LP
In order to open an ICE connection with another client (that is waiting
for connections), call the
.PN IceOpenConnection
function.
.LP
.sM
.FD 0
IceConn IceOpenConnection\^(\^\fInetwork_ids_list\fP, \fIcontext\fP\^, \fImust_authenticate\fP\^, \fImajor_opcode_check\fP\^,
                    \fIerror_length\fP\^, \fIerror_string_ret\fP\^)
.br
     char *\fInetwork_ids_list\fP\^;
.br
     IcePointer \fIcontext\fP\^;
.br
     Bool \fImust_authenticate\fP\^;
.br
     int \fImajor_opcode_check\fP\^;
.br
     int  \fIerror_length\fP\^;
.br
     char *\fIerror_string_ret\fP\^;
.FN
.IP \fInetwork_ids_list\fP 1.3i
Specifies the network ID(s) of the other client.
.IP \fIcontext\fP 1.3i
A pointer to an opaque object, or NULL.  Used to determine if an
ICE connection can be shared (see below).
.IP \fImust_authenticate\fP 1.3i
If
.PN True ,
the other client may not bypass authentication.
.IP \fImajor_opcode_check\fP 1.3i
Used to force a new ICE connection to be created (see below).
.IP \fIerror_length\fP 1.3i
Length of the \fIerror_string_ret\fP\^ argument passed in.
.IP \fIerror_string_ret\fP 1.3i
Returns a null terminated error message, if any.  \fIerror_string_ret\fP
points to user supplied memory.  No more than \fIerror_length\fP\^ bytes
are used.
.LP
.eM
.PN IceOpenConnection
returns an opaque ICE connection object if it succeeds,
.PN NULL
otherwise.
.LP
\fInetwork_ids_list\fP\^ contains a list of network IDs separated by commas.
An attempt will be made to use the first network ID.  If that fails,
an attempt will be made using the second network ID, and so on.
Each network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<objname>	or
	local/<hostname>:<path>	
.TE
.LP
Most protocol libraries will have some sort of "open" function which should
internally make a call into
.PN IceOpenConnection .
When
.PN IceOpenConnection
is called, it may be possible to use a previously opened ICE connection (if
the target client is the same).  However, there are cases in which shared
ICE connections are not desired.
.LP
The \fIcontext\fP\^ argument is used to determine if an ICE connection can
be shared.  If \fIcontext\fP\^ is
.PN NULL ,
then the caller is always willing to share the connection.
If \fIcontext\fP\^ is not
.PN NULL ,
then the caller is not willing to use a previously opened ICE connection
that has a different non-NULL context associated with it.
.LP
In addition, if
\fImajor_opcode_check\fP\^ contains a non-zero major opcode value, a
previously created ICE connection will be used only if the major opcode
is not active on the connection.  This can be used to force multiple ICE
connections between two clients for the same protocol.
.LP
Any authentication requirements are handled internally by the ICE library.
The method by which the authentication data is obtained is implementation
dependent.\(dg
.FS \(dg
The X Consortium's ICElib implementation uses an .ICEauthority file (see
Appendix A).
.FE
.LP
After
.PN IceOpenConnection
is called, the client is ready to send a \fIProtocol Setup\fP\^ (provided that
.PN IceRegisterForProtocolSetup
was called), or receive a \fIProtocol Setup\fP\^ (provided that
.PN IceRegisterForProtocolReply
was called).
.LP
.NH 2
Listening for ICE Connections
.XS
\*(SN Listening for ICE Connections
.XE
.LP
Clients wishing to accept ICE connections must first call
.PN IceListenForConnections
so they can listen for connections.  A list of opaque "listen" objects are
returned, one for each type of transport method that is available
(for example, Unix Domain, TCP, DECnet, etc...).
.LP
.sM
.FD 0
Status IceListenForConnections\^(\^\fIcount_ret\fP, \fIlisten_objs_ret\fP\^, \fIerror_length\fP\^, \fIerror_string_ret\fP\^)
.br
     int  *\fIcount_ret\fP\^;
.br
     IceListenObj **\fIlisten_objs_ret\fP\^;
.br
     int  \fIerror_length\fP\^;
.br
     char *\fIerror_string_ret\fP\^;
.FN
.IP \fIcount_ret\fP 1i
The number of listen objects returned.
.IP \fIlisten_objs_ret\fP 1i
Returns a list of opaque listen objects.
.IP \fIerror_length\fP 1i
The length of the \fIerror_string_ret\fP\^ argument passed in.
.IP \fIerror_string_ret\fP 1i
Returns a null terminated error message, if any.  \fIerror_string_ret\fP
points to user supplied memory.  No more than \fIerror_length\fP\^ bytes
are used.
.LP
.eM
The return value of
.PN IceListenForConnections
is zero for failure, and a positive value for success.
.LP
.sp 0.5
Call
.PN IceFreeListenObjs
to close and free the listen objects.
.LP
.sM
.FD 0
void IceFreeListenObjs\^(\^\fIcount\fP, \fIlisten_objs\fP\^)
.br
    int \fIcount\fP\^;
.br
    IceListenObj *\fIlisten_objs\fP\^;
.FN
.IP \fIcount\fP 1i
The number of listen objects.
.IP \fIlisten_objs\fP 1i
The listen objects.
.LP
.eM
.LP
In order to detect a new connection on a listen object, select() must be
called on the descriptor associated with the listen object.  To obtain
the descriptor, call the
.PN IceGetListenConnectionNumber
function.
.LP
.sM
.FD 0
int IceGetListenConnectionNumber\^(\^\fIlisten_obj\fP\^)
.br
    IceListenObj \fIlisten_obj\fP\^;
.FN
.IP \fIlisten_obj\fP 1i
The listen object.
.LP
.eM
.LP
To obtain the network ID string associated with a listen object, call the
.PN IceGetListenConnectionString
function.
.LP
.sM
.FD 0
char *IceGetListenConnectionString\^(\^\fIlisten_obj\fP\^)
.br
    IceListenObj \fIlisten_obj\fP\^;
.FN
.IP \fIlisten_obj\fP 1i
The listen object.
.LP
.eM
.LP
A network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<objname>	or
	local/<hostname>:<path>	
.TE
.LP
To compose a string containing a list of network IDs separated by commas
(the format recognized by
.PN IceOpenConnection ),
call the
.PN IceComposeNetworkIdList
function.
.LP
.sM
.FD 0
char *IceComposeNetworkIdList\^(\^\fIcount\fP, \fIlisten_objs\fP\^)
.br
    int \fIcount\fP\^;
.br
    IceListenObj *\fIlisten_objs\fP\^;
.FN
.IP \fIcount\fP 1i
The number of listen objects.
.IP \fIlisten_objs\fP 1i
The listen objects.
.LP
.eM
.NH 2
Host Based Authentication for ICE Connections
.XS
\*(SN Host Based Authentication for ICE Connections
.XE
.LP
If authentication fails when a client attempts to open an
ICE connection, and the initiating client has not required authentication,
a host based authentication procedure may be invoked to provide
a last chance for the client to connect.  Each listen object has such a
callback associated with it, and this callback is set using the
.PN IceSetHostBasedAuthProc
function.
.LP
.sM
.FD 0
void IceSetHostBasedAuthProc\^(\^\fIlisten_obj\fP, \fIhost_based_auth_proc\fP\^)
.br
    IceListenObj \fIlisten_obj\fP\^;
.br
    IceHostBasedAuthProc \fIhost_based_auth_proc\fP\^;
.FN
.IP \fIlisten_obj\fP 1.4i
The listen object.
.IP \fIhost_based_auth_proc\fP 1.4i
The host based authentication procedure.
.LP
.eM
By default, each listen object has no host based authentication procedure
associated with it.  Passing
.PN NULL
for \fIhost_based_auth_proc\fP\^ turns off host based authentication if
it was previously set.
.LP
.bp
.sM
typedef Bool (*IceHostBasedAuthProc) ();
.FD 0
Bool HostBasedAuthProc\^(\^\fIhost_name\fP\^)
.br
    char *\fIhost_name\fP\^;
.FN
.IP \fIhost_name\fP 1i
The host name of the client that tried to open an ICE connection.
.LP
.eM
\fIhost_name\fP\^ is a string of the form "protocol/hostname" where
\fIprotocol\fP\^ is one of {tcp, decnet, local}.
.LP
If
.PN IceHostBasedAuthProc
returns
.PN True ,
access will be granted, even though the original
authentication failed.  Note that authentication can effectively be
disabled by registering an
.PN IceHostBasedAuthProc
which always returns
.PN True .
.LP
Host based authentication is also allowed at \fIProtocol Setup\fP\^ time.
The callback is specified in the
.PN IceRegisterForProtocolReply
function discussed earlier.
.LP
.NH 2
Accepting ICE Connections
.XS
\*(SN Accepting ICE Connections
.XE
.LP
After a connection attempt is detected on a listen object returned by
.PN IceListenForConnections ,
.PN IceAcceptConnection
should be called.  This returns a new opaque ICE connection object.
.LP
.sM
.FD 0
IceConn IceAcceptConnection\^(\^\fIlisten_obj\fP, \fI\^status_ret\fP\^)
.br
    IceListenObj \fIlisten_obj\fP\^;
.br
    IceAcceptStatus *\fIstatus_ret\fP\^;
.FN
.IP \fIlisten_obj\fP 1i
The listen object on which a new connection was detected.
.IP \fIstatus_ret\fP 1i
Return status information.
.LP
.eM
The \fIstatus_ret\fP\^ argument is set to one of the following values:
.LP
.TS
lw(2i) lw(4i).
T{
.PN IceAcceptSuccess :
T}	T{
The accept operation succeeded.  The function returns a new connection object.
T}
.sp 4p
T{
.PN IceAcceptFailure :
T}	T{
The accept operation failed.  The function returns NULL.
T}
.sp 4p
T{
.PN IceAcceptBadMalloc :
T}	T{
A memory allocation failed.  The function returns NULL.
T}
.TE
.LP
.sp 0.5
.LP
In general, in order to detect new connections, the application will call
select() on the file descriptors associated with the listen objects.  When
a new connection is detected, the
.PN IceAcceptConnection
function should be called.  
.PN IceAcceptConnection
may return a new ICE connection that is in a "pending" state.  This is because
before the connection can become valid, authentication may be necessary.
Since the ICE library cannot block and wait for the connection to
become valid (infinite blocking can occur if the connecting client
does not act properly), the application must wait for the connection status
to become "valid".
.LP
The following pseudo-code demonstrates how connections are accepted:
.LP
.sp 0.5
.bp
.vs 10
.nf

	new_ice_conn = IceAcceptConnection (listen_obj);
	status = IceConnectionStatus (new_ice_conn);
	time_start = time_now;

	while (status == IceConnectPending)
	{
		select() on {new_ice_conn, all open connections}

		for (each ice_conn in the list of open connections)
			if (data ready on ice_conn)
			{
				status = IceProcessMessages (ice_conn, NULL, NULL);
				if (status == IceProcessMessagesIOError)
					IceCloseConnection (ice_conn);
			}

		if (data ready on new_ice_conn)
		{
			/*
			 * IceProcessMessages is called until the connection
			 * is non-pending.  Doing so handles the connection
			 * setup request and any authentication requirements.
			 */

			IceProcessMessages (new_ice_conn, NULL, NULL);
			status = IceConnectionStatus (new_ice_conn);
		}
		else
		{
			if (time_now - time_start > MAX_WAIT_TIME)
				status = IceConnectRejected;
		}
	}

	if (status == IceConnectAccepted)
	{
		Add new_ice_conn to the list of open connections
	}
	else
	{
		IceCloseConnection (new_ice_conn);
	}
.fi
.vs
.LP
.sp 0.5
After
.PN IceAcceptConnection
is called and the connection has been
validated, the client is ready to receive a \fIProtocol Setup\fP\^ (provided
that
.PN IceRegisterForProtocolReply
was called), or send a \fIProtocol Setup\fP\^
(provided that
.PN IceRegisterForProtocolSetup
was called).
.NH 2
Closing ICE Connections
.XS
\*(SN Closing ICE Connections
.XE
.LP
To close an ICE connection created with
.PN IceOpenConnection
or
.PN IceAcceptConnection ,
call the
.PN IceCloseConnection
function.
.LP
.sM
.FD 0
IceCloseStatus IceCloseConnection\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection to close.
.LP
.eM
In order to actually close an ICE connection, the following conditions
must be met:
.LP
.IP -
The \fIopen reference count\fP\^ must have reached zero on this ICE connection.
When
.PN IceOpenConnection
is called, it tries to use a previously opened
ICE connection.  If it is able to use an existing connection, it increments
the \fIopen reference count\fP\^ on the connection by one.  So in order to
close an ICE connection, each call to
.PN IceOpenConnection
must be matched with a call to
.PN IceCloseConnection .
The connection can be closed only
on the last call to
.PN IceCloseConnection .
.LP
.IP -
The \fIactive protocol count\fP\^ must have reached zero.  Each time a
\fIProtocol Setup\fP\^ succeeds on the connection
the \fIactive protocol count\fP\^
is incremented by one.  When the client no longer expects to use the
protocol on the connection, the
.PN IceProtocolShutdown
function should be called, which decrements the \fIactive protocol count\fP\^
by one (see the \fIProtocol Setup and Shutdown\fP section).
.LP
.IP -
If shutdown negotiation is enabled on the connection, the client on the other
side of the ICE connection must agree to have the connection closed.
.LP
.sp 0.5
.PN IceCloseConnection
returns one of the following values:
.LP
.TS
lw(2i) lw(4i).
T{
.PN IceClosedNow :
T}	T{
The ICE connection was closed at this time.  The watch procedures were
invoked and the connection was freed.
T}
.sp 4p
T{
.PN IceClosedASAP :
T}	T{
An IO error had occurred on the connection, but
.PN IceCloseConnection
is being called within a nested
.PN IceProcessMessages .
The watch procedures have been invoked at this time, but the connection
will be freed as soon as possible (when the nesting level reaches zero and
.PN IceProcessMessages
returns a status of
.PN IceProcessMessagesConnectionClosed ).
T}
.sp 4p
T{
.PN IceConnectionInUse :
T}	T{
The connection was not closed at this time because it is being used by
other active protocols.
T}
.sp 4p
T{
.PN IceStartedShutdownNegotiation :
T}	T{
The connection was not closed at this time and shutdown negotiation started
with the client on the other side of the ICE connection.  When the connection
is actually closed,
.PN IceProcessMessages
will return a status of
.PN IceProcessMessagesConnectionClosed .
T}
.TE
.LP
.sp 0.5
When it is known that the client on the other side of the ICE connection
has terminated the connection without initiating shutdown negotiation, the
.PN IceSetShutdownNegotiation
function should be called to turn off shutdown negotiation.  This will prevent
.PN IceCloseConnection
from writing to a broken connection.
.LP
.sM
.FD 0
void IceSetShutdownNegotiation\^(\^\fIice_conn\fP, \fInegotiate\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    Bool \fInegotiate\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fInegotiate\fP 1i
If
.PN False ,
shutdown negotiating will be turned off.
.LP
.eM
In order to check the shutdown negotiation status of an ICE connection,
call the
.PN IceCheckShutdownNegotiation
function.
.LP
.sM
.FD 0
Bool IceCheckShutdownNegotiation\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.LP
.eM
.PN IceCheckShutdownNegotiation
returns
.PN True
if shutdown negotiation will take place on the connection,
.PN False
otherwise.  Negotiation is on by default for a connection.  It
can only be changed with the
.PN IceSetShutdownNegotiation
function.
.LP
.NH 2
Connection Watch Procedures
.XS
\*(SN Connection Watch Procedures
.XE
.LP
In order to add a watch procedure which will be called
each time ICElib opens a new connection via
.PN IceOpenConnection
or
.PN IceAcceptConnection ,
or closes a connection via
.PN IceCloseConnection ,
call the
.PN IceAddConnectionWatch
function.
.LP
.sM
.FD 0
Status IceAddConnectionWatch\^(\^\fIwatch_proc\fP, \fIclient_data\fP\^)
.br
    IceWatchProc \fIwatch_proc\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.FN
.IP \fIwatch_proc\fP 1i
The watch procedure to invoke when ICElib opens or
closes a connection.
.IP \fIclient_data\fP 1i
This pointer will be passed to the watch procedure.
.LP
.eM
The return value of
.PN IceAddConnectionWatch
is zero for failure, and a positive value for success.
.LP
Note that several calls to
.PN IceOpenConnection
might share the same ICE connection.  In such a case, the watch procedure
is only invoked when the connection is first created (after authentication
succeeds).  Similarly, since
connections might be shared, the watch procedure is called only if
.PN IceCloseConnection
actually closes the connection (right before the IceConn is freed).
.LP
The watch procedures are very useful for applications which
need to add a file descriptor to a select mask when a new connection
is created, and remove the file descriptor when the connection is destroyed.
Since connections are shared, knowing when to add and remove the file
descriptor from the select mask would be difficult without the watch
procedures.
.LP
Multiple watch procedures may be registered with the ICE library.
No assumptions should be made about their order of invocation.
.LP
If one or more ICE connections were already created by the ICE library at the
time the watch procedure is registered, the watch procedure will instantly
be invoked for each of these ICE connections (with the \fIopening\fP\^ flag
set to
.PN True ).
.LP
.sp 0.5
The watch procedure is of type
.PN IceWatchProc .
.LP
.sM
typedef void (*IceWatchProc)();
.LP
.FD 0
void WatchProc\^(\^\fIice_conn\fP, \fIclient_data\fP\^, \fIopening\fP\^, \fIwatch_data\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.br
    Bool \fIopening\fP\^;
.br
    IcePointer *\fIwatch_data\fP\^;
.FN
.IP \fIice_conn\fP\^ 1i
The opened or closed ICE connection.  Call
.PN IceConnectionNumber
to get the file descriptor associated with this connection.
.IP \fIclient_data\fP\^ 1i
Client data specified in the call to
.PN IceAddConnectionWatch .
.IP \fIopening\fP\^ 1i
If
.PN True ,
the connection is being opened.  If
.PN False ,
the connection is being closed.
.IP \fIwatch_data\fP\^ 1i
Can be used to save a pointer to client data.
.LP
.eM
If \fIopening\fP\^ is
.PN True ,
the client should set the *\fIwatch_data\fP\^
pointer to any data it may need to save until the connection is closed
and the watch procedure is
invoked again with \fIopening\fP\^ set to
.PN False .
.LP
.sp 0.5
To remove a watch procedure, call the
.PN IceRemoveConnectionWatch
function.
.LP
.bp
.sM
.FD 0
void IceRemoveConnectionWatch\^(\^\fIwatch_proc\fP, \fIclient_data\fP\^)
.br
    IceWatchProc \fIwatch_proc\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.LP
.FN
.IP \fIwatch_proc\fP 1i
The watch procedure that was passed to
.PN IceAddConnectionWatch .
.IP \fIclient_data\fP 1i
The \fIclient_data\fP\^ pointer that was passed to
.PN IceAddConnectionWatch .
.LP
.eM
.NH 1
Protocol Setup and Shutdown
.XS
\*(SN Protocol Setup and Shutdown
.XE
.LP
In order to activate a protocol on a given ICE connection, call the
.PN IceProtocolSetup
function.
.LP
.sM
.FD 0
IceProtocolSetupStatus IceProtocolSetup\^(\^\fIice_conn\fP, \fImy_opcode\fP\^, \fIclient_data\fP\^, \fImust_authenticate\fP\^,
.br
                    \fImajor_version_ret\fP\^, \fIminor_version_ret\fP\^, \fIvendor_ret\fP\^, \fIrelease_ret\fP\^, \fIerror_length\fP\^, \fIerror_string_ret\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    int \fImy_opcode\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.br
    Bool \fImust_authenticate\fP\^;
.br
    int *\fImajor_version_ret\fP\^;
.br
    int *\fIminor_version_ret\fP\^;
.br
    char **\fIvendor_ret\fP\^;
.br
    char **\fIrelease_ret\fP\^;
.br
    int \fIerror_length\fP\^;
.br
    char *\fIerror_string_ret\fP\^;
.FN
.IP \fIice_conn\fP 1.1i
A valid ICE connection object.
.IP \fImy_opcode\fP 1.1i
The major opcode of the protocol to be set up, as returned by
.PN IceRegisterForProtocolSetup .
.IP \fIclient_data\fP 1.1i
The client data stored in this pointer will be passed to the
.PN IcePoProcessMsgProc
callback.
.IP \fImust_authenticate\fP 1.1i
If
.PN True ,
the other client may not bypass authentication.
.IP \fImajor_version_ret\fP 1.1i
The major version of the protocol to be used is returned.
.IP \fIminor_version_ret\fP 1.1i
The minor version of the protocol to be used is returned.
.IP \fIvendor_ret\fP 1.1i
The vendor string specified by the protocol acceptor.
.IP \fIrelease_ret\fP 1.1i  
The release string specified by the protocol acceptor.
.IP \fIerror_length\fP 1.1i
Specifies the length of the \fIerror_string_ret\fP\^ argument passed in.
.IP \fIerror_string_ret\fP 1.1i
Returns a null terminated error message, if any.  \fIerror_string_ret\fP
points to user supplied memory.  No more than \fIerror_length\fP\^ bytes
are used.
.LP
.eM
The \fIvendor_ret\fP\^ and \fIrelease_ret\fP\^ strings should be freed
with free() when no longer needed.
.LP
.sp 0.5
.PN IceProtocolSetup
returns one of the following values:
.LP
.TS
lw(2i) lw(4i).
T{
.PN IceProtocolSetupSuccess :
T}	T{
\fImajor_version_ret\fP\^, \fIminor_version_ret\fP\^, \fIvendor_ret\fP\^, \fIrelease_ret\fP\^ are set.
T}
.sp 4p
T{
.PN IceProtocolSetupFailure
or
.br
.PN IceProtocolSetupIOError :
T}	T{
Check \fIerror_string_ret\fP\^ for failure reason.  \fImajor_version_ret\fP\^, \fIminor_version_ret\fP\^, \fIvendor_ret\fP\^, \fIrelease_ret\fP\^ are NOT set.
T}
.sp 4p
T{
.PN IceProtocolAlreadyActive :
T}	T{
This protocol is already active on this connection.  \fImajor_version_ret\fP\^, \fIminor_version_ret\fP\^, \fIvendor_ret\fP\^, \fIrelease_ret\fP\^ are NOT set.
T}
.TE
.LP
.sp 0.5
In order to notify the ICE library when a given protocol
will no longer be used on an ICE connection, call the
.PN IceProtocolShutdown
function.
.LP
.sM
.FD 0
Status IceProtocolShutdown\^(\^\fIice_conn\fP, \fImajor_opcode\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    int \fImajor_opcode\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fImajor_opcode\fP 1i
The major opcode of the protocol to shut down.
.LP
.eM
The return value of
.PN IceProtocolShutdown
is zero for failure, and a positive value for success.
.LP
Failure will occur if the major opcode was never registered OR the protocol
of the major opcode was never "activated" on the connection.  By "activated"
we mean that a \fIProtocol Setup\fP\^ succeeded on the connection.
Note that ICE does not define how each sub-protocol triggers a
protocol shutdown.
.LP
.NH 1
Processing Messages
.XS
\*(SN Processing Messages
.XE
.LP
In order to process incoming messages on an ICE connection, the
.PN IceProcessMessages
function should be called.
.LP
.sM
.FD 0
IceProcessMessagesStatus IceProcessMessages\^(\^\fIice_conn\fP, \fIreply_wait\fP\^, \fIreply_ready_ret\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IceReplyWaitInfo *\fIreply_wait\fP\^;
.br
    Bool *\fIreply_ready_ret\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIreply_wait\fP 1i
Indicates if a reply is being waited for.
.IP \fIreply_ready_ret\fP 1i
If set to
.PN True
on return, a reply is ready.
.LP
.eM
This function is used in two ways.  In the first, a client may
generate a message and "block" by calling
.PN IceProcessMessages
repeatedly until it gets its reply.  In the second case, a
client calls
.PN IceProcessMessages
with \fIreply_wait\fP\^ set to
.PN NULL
in response to select() showing that there is data to read on the
ICE connection.  The ICE library may process zero or more complete messages.
Note that messages which are not "blocked" for are always processed by
invoking callbacks.
.LP
.PN IceReplyWaitInfo
contains the major/minor opcodes and sequence number
of the message for which a reply is being awaited.  It also contains
a pointer to the reply message to be filled in (the protocol library
should cast this
.PN IcePointer
to the appropriate reply type).  In most
cases, the reply will have some fixed-size part, and the client waiting
for the reply will have provided a pointer to a structure to hold
this fixed-size data.  If there is variable-length data, it would be
expected that the
.PN IcePoProcessMsgProc
callback will have to allocate additional
memory and store pointer(s) to that memory in the fixed-size
structure.  If the entire data is variable length (e.g., a single
variable-length string), then the client waiting for the reply would probably
just pass a pointer to fixed-size space to hold a pointer, and the
.PN IcePoProcessMsgProc
callback would allocate the storage and store the pointer.
It is the responsibility of the client receiving the reply to
free up any memory allocated on its behalf.
.LP
.sM
typedef struct {
.br
	unsigned long sequence_of_request;
.br
	int major_opcode_of_request;
.br
	int minor_opcode_of_request;
.br
	IcePointer reply;
.br
} IceReplyWaitInfo;
.LP
.eM
.sp 0.5
If \fIreply_wait\fP\^ is not
.PN NULL
and
.PN IceProcessMessages
has a reply or error to return in response to this \fIreply_wait\fP\^
(i.e. no callback was generated), then the \fIreply_ready_ret\fP\^ argument
will be set to
.PN True .
.LP
If \fIreply_wait\fP\^ is
.PN NULL ,
then the caller may also pass
.PN NULL
for \fIreply_ready_ret\fP\^ and be guaranteed that no value will be stored
in this pointer.
.LP
.sp 0.5
.PN IceProcessMessages
returns one of the following values:
.LP
.TS
lw(2.5i) lw(4i).
T{
.PN IceProcessMessagesSuccess :
T}	T{
No error occurred.
T}
.sp 4p
T{
.PN IceProcessMessagesIOError :
T}	T{
An IO error occurred.  The caller must explicitly close the connection
by calling
.PN IceCloseConnection .
T}
.sp 4p
T{
.PN IceProcessMessagesConnectionClosed :
T}	T{
The ICE connection has been closed (closing of the connection was deferred
because of shutdown negotiation, or because the
.PN IceProcessMessages
nesting level was not zero).  Do not attempt
to access the ICE connection at this point, since it has been freed.
T}
.TE
.LP
.NH 1
Ping
.XS
\*(SN Ping
.XE
.LP
To send a \fIPing\fP\^ message to the client on the other side of the
ICE connection, call the
.PN IcePing
function.
.LP
.sM
.FD 0
Status IcePing\^(\^\fIice_conn\fP, \fIping_reply_proc\fP\^, \fIclient_data\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePingReplyProc \fIping_reply_proc\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIping_reply_proc\fP 1i
The callback to invoke when the \fIPing\fP\^ reply arrives.
.IP \fIclient_data\fP 1i
This pointer will be passed to the
.PN IcePingReplyProc
callback.
.LP
.eM
The return value of
.PN IcePing
is zero for failure, and a positive value for success.
.LP
When
.PN IceProcessMessages
processes the Ping reply, it will invoke the
.PN IcePingReplyProc
callback.
.LP
.sM
typedef void (*IcePingReplyProc)();
.LP
.FD 0
void PingReplyProc\^(\^\fIice_conn\fP, \fIclient_data\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    IcePointer \fIclient_data\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection object.
.IP \fIclient_data\fP 1i
The client data specified in the call to
.PN IcePing .
.LP
.eM
.NH 1
Informational Functions
.XS
\*(SN Informational Functions
.XE
.LP
.sM
.FD 0
IceConnectStatus IceConnectionStatus\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the status of an ICE connection.  The possible return values are:
.LP
.TS
lw(2i) lw(4i).
T{
.PN IceConnectPending :
T}	T{
The connection is not valid yet (i.e. authentication is taking place).
Only relevant to connections created by
.PN IceAcceptConnection .
T}
.sp 4p
T{
.PN IceConnectAccepted :
T}	T{
The connection has been accepted.  Only relevant to connections created by
.PN IceAcceptConnection .
T}
.sp 4p
T{
.PN IceConnectRejected :
T}	T{
The connection had been rejected  (i.e. authentication failed).
Only relevant to connections created by
.PN IceAcceptConnection .
T}
.sp 4p
T{
.PN IceConnectIOError :
T}	T{
An IO error has occurred on the connection.
T}
.TE
.LP
.sp 0.5
.sM
.FD 0
char *IceVendor\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the ICE library vendor identification for the other side of the connection.
The string should be freed with a call to free() when no longer needed.
.LP
.sp 0.5
.sM
.FD 0
char *IceRelease\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the release identification of the ICE library on the other side of the connection.
The string should be freed with a call to free() when no longer needed.
.LP
.sp 0.5
.sM
.FD 0
int IceProtocolVersion\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the major version of the ICE protocol on this connection.
.LP
.sp 0.5
.sM
.FD 0
int IceProtocolRevision\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the minor version of the ICE protocol on this connection.
.LP
.sp 0.5
.sM
.FD 0
int IceConnectionNumber\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the file descriptor of this ICE connection.
.LP
.sp 0.5
.sM
.FD 0
char *IceConnectionString\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the network ID of the client which
accepted this connection.  The string should be freed with a call to
free() when no longer needed.
.LP
.sp 0.5
.sM
.FD 0
unsigned long IceLastSentSequenceNumber\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the sequence number of the last message sent on this ICE connection.
.LP
.sp 0.5
.sM
.FD 0
unsigned long IceLastReceivedSequenceNumber\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the sequence number of the last message received on this
ICE connection.
.LP
.sp 0.5
.sM
.FD 0
Bool IceSwapping\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns
.PN True
if byte swapping is necessary when reading messages on the ICE connection.
.LP
.sp 0.5
.sM
.FD 0
IcePointer IceGetContext\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
Returns the context associated with a connection created by
.PN IceOpenConnection .
.LP
.NH 1
ICE Messages
.XS
\*(SN ICE Messages
.XE
.LP
All ICE messages have a standard 8 byte header.  The ICElib macros which
read and write messages rely on the following naming convention for message
headers:
.LP
	CARD8	major_opcode;
.br
	CARD8	minor_opcode;
.br
	CARD8	data[2];
.br
	CARD32	length B32;
.LP
The 3rd and 4th bytes of the message header can be used as needed.  The length
field is specified in \fBunits of 8 bytes\fP.
.LP
.NH 2
Sending ICE Messages
.XS
\*(SN Sending ICE Messages
.XE
.LP
The ICE library maintains an output buffer used for generating messages.
Protocol libraries layered on top of ICE may choose to batch messages
together and flush the output buffer at appropriate times.
.LP
If an IO error has occurred on an ICE connection, all write operations
will be ignored.  Refer to the section titled \fIError Handling\fP\^ for
more discussion on handling IO errors.
.LP
.sp 0.5
To get the size of the ICE output buffer, call the
.PN IceGetOutBufSize
function.
.LP
.sM
.FD 0
int IceGetOutBufSize\^(\^\fIice_conn\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.LP
.eM
.LP
To flush the ICE output buffer, call the
.PN IceFlush
function.
.LP
.sM
.FD 0
IceFlush\^(\^\fIice_conn\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.LP
.eM
Note that the output buffer may be implicitly flushed if there is insufficient
space to generate a message.
.LP
.sp 0.5
The following macros can be used to generate ICE messages:
.sM
.FD 0
IceGetHeader\^(\^\fIice_conn\fP, \fImajor_opcode\fP\^, \fIminor_opcode\fP\^, \fIheader_size\fP\^, \fI<C_data_type>\fP\^, \fIpmsg\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fImajor_opcode\fP\^;
.br
     int \fIminor_opcode\fP\^;
.br
     int \fIheader_size\fP\^;
.br
     <C_data_type> *\fIpmsg\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fImajor_opcode\fP 1i
The major opcode of the message.
.IP \fIminor_opcode\fP 1i
The minor opcode of the message.
.IP \fIheader_size\fP 1i
The size of the message header (in bytes).
.IP \fI<C_data_type>\fP 1i
The actual C data type of the message header.
.IP \fIpmsg\fP 1i
The message header pointer.  After this macro is called, the
library can store data in the message header.
.LP
.eM
.PN IceGetHeader
is used to set up a message header on an ICE connection.
It sets the major and minor opcodes of the message, and initializes
the message's length to the length of the header.  If additional
variable length data follows, the message's length field should be
updated.
.LP
.sp 0.5
.sM
.FD 0
IceGetHeaderExtra\^(\^\fIice_conn\fP, \fImajor_opcode\fP\^, \fIminor_opcode\fP\^, \fIheader_size\fP\^, \fIextra\fP\^, \fI<C_data_type>\fP\^, \fIpmsg\fP\^, \fIpdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fImajor_opcode\fP\^;
.br
     int \fIminor_opcode\fP\^;
.br
     int \fIheader_size\fP\^;
.br
     int \fIextra\fP\^;
.br
     <C_data_type> *\fIpmsg\fP\^;
.br
     char *\fIpdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fImajor_opcode\fP 1i
The major opcode of the message.
.IP \fIminor_opcode\fP 1i
The minor opcode of the message.
.IP \fIheader_size\fP 1i
The size of the message header (in bytes).
.IP \fIextra\fP 1i
The size of the extra data beyond the header (in 8 byte units).
.IP \fI<C_data_type>\fP 1i
The actual C data type of the message header.
.IP \fIpmsg\fP 1i
The message header pointer.  After this macro is called, the
library can store data in the message header.
.IP \fIpdata\fP 1i
Returns a pointer to the ICE output buffer which points
immediately after the message header.  The variable length
data should be stored here.  If there was not enough room
in the ICE output buffer, \fIpdata\fP\^ is set to
.PN NULL .
.LP
.eM
.PN IceGetHeaderExtra
is used to generate a message with a fixed (and relatively small) amount
of variable length data.  The complete message must fit in the ICE output
buffer.
.LP
.sp 0.5
.bp
.sM
.FD 0
IceSimpleMessage\^(\^\fIice_conn\fP, \fImajor_opcode\fP\^, \fIminor_opcode\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fImajor_opcode\fP\^;
.br
     int \fIminor_opcode\fP\^;
.FN
.br
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fImajor_opcode\fP 1i
The major opcode of the message.
.IP \fIminor_opcode\fP 1i
The minor opcode of the message.
.LP
.eM
.PN IceSimpleMessage
is used to generate a message which is identical
in size to the ICE header message, and has no additional data.
.LP
.sp 0.5
.sM
.FD 0
IceErrorHeader\^(\^\fIice_conn\fP, \fIoffending_major_opcode\fP\^, \fIoffending_minor_opcode\fP\^, \fIoffending_sequence_num\fP\^,
.br
                    \fIseverity\fP\^, \fIerror_class\fP\^, \fIdata_length\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIoffending_major_opcode\fP\^;
.br
     int \fIoffending_minor_opcode\fP\^;
.br
     int \fIoffending_sequence_num\fP\^;
.br
     int \fIseverity\fP\^;
.br
     int \fIerror_class\fP\^;
.br
     int \fIdata_length\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIoffending_major_opcode\fP 1i
The major opcode of the protocol in which an error was detected.
.IP \fIoffending_minor_opcode\fP 1i
The minor opcode of the protocol in which an error was detected.
.IP \fIoffending_sequence_num\fP 1i
The sequence number of the message that caused the error.
.IP \fIseverity\fP 1i
.PN IceCanContinue ,
.PN IceFatalToProtocol ,
or
.PN IceFatalToConnection .
.IP \fIerror_class\fP 1i
The error class.  See below.
.IP \fIdata_length\fP 1i
Length of data (in 8 byte units) to be written after the header.
.LP
.eM
.PN IceErrorHeader
sets up an error message header.
.LP
Note that the two clients connected by ICE may be using different
major opcodes for a given protocol.  The \fIoffending_major_opcode\fP\^ passed
to this macro is the major opcode of the protocol for the client
sending the error message.
.LP
Generic errors which are common to all protocols have classes
in the range 0x8000..0xFFFF.  See the \fIInter-Client Exchange Protocol\fP\^
document for more details.
.TS
lw(1i) lw(1i).
T{
.PN IceBadMinor
T}	T{
0x8000
T}
.sp 4p
T{
.PN IceBadState
T}	T{
0x8001
T}
.sp 4p
T{
.PN IceBadLength
T}	T{
0x8002
T}
.sp 4p
T{
.PN IceBadValue
T}	T{
0x8003
T}
.TE
.LP
Per-protocol errors have classes in the range 0x0000-0x7fff.
.LP
.sp 0.5
To write data to an ICE connection, use the
.PN IceWriteData
macro.  If the data fits into the ICE output buffer, it is copied there.
Otherwise, the ICE output buffer is flushed and the data is directly sent.
.LP
This macro is used in conjunction with
.PN IceGetHeader
and
.PN IceErrorHeader .
.LP
.sM
.FD 0
IceWriteData\^(\^\fIice_conn\fP, \fIbytes\fP\^, \fIdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.br
     char *\fIdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to write.
.IP \fIdata\fP 1i
The data to write.
.LP
.eM
To write data as 16 bit quantities, use the
.PN IceWriteData16
macro.
.sM
.FD 0
IceWriteData16\^(\^\fIice_conn\fP, \fIbytes\fP\^, \fIdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.br
     short *\fIdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to write.
.IP \fIdata\fP 1i
The data to write.
.LP
.eM
To write data as 32 bit quantities, use the
.PN IceWriteData32
macro.
.sM
.FD 0
IceWriteData32\^(\^\fIice_conn\fP, \fIbytes\fP\^, \fIdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.br
     long *\fIdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to write.
.IP \fIdata\fP 1i
The data to write.
.LP
.eM
.LP
.sp 0.5
To bypass copying data to the ICE output buffer,
use the
.PN IceSendData
to directly send data over the network connection.  If necessary, the
ICE output buffer is first flushed.
.sM
.FD 0
IceSendData\^(\^\fIice_conn\fP, \fIbytes\fP\^, \fI(char *) data\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.br
     char *\fIdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to send.
.IP \fIdata\fP 1i
The data to send.
.LP
.eM
.LP
.sp 0.5
To force 32 or 64 bit alignment, use the
.PN IceWritePad
macro.  A maximum of 7 pad bytes can be specified.
.sM
.FD 0
IceWritePad\^(\^\fIice_conn\fP, \fIbytes\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of pad bytes.
.LP
.eM
.LP
.NH 2
Reading ICE Messages
.XS
\*(SN Reading ICE Messages
.XE
.LP
The ICE library maintains an input buffer used for reading messages.
If the ICE library chooses to perform non-blocking reads (this is
implementation dependent), then for every read operation that it makes,
zero or more complete messages may be read into the input buffer.  As
a result, for all of the macros described in this section which "read"
messages, an actual read operation will occur on the connection only if
the data is not already present in the input buffer.
.LP
.sp 0.5
To get the size of the ICE input buffer, call the
.PN IceGetInBufSize
function.
.LP
.sM
.FD 0
int IceGetInBufSize\^(\^\fIice_conn\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.LP
.eM
.LP
When reading messages, care must be taken to check for IO errors.  If
any IO error occurs in reading any part of a message, the message should
be thrown out.  After using any of the macros described below for reading
messages, the
.PN IceValidIO
macro can be used to check if an IO error occurred on the
connection.  After an IO error has occurred on an ICE connection, all
read operations will be ignored.  Refer to the section titled
\fIError Handling\fP\^ for more discussion on handling IO errors.
.LP
.sM
.FD 0
Bool IceValidIO\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.eM
.sp 0.5
The following macros can be used to read ICE messages:
.sM
.FD 0
IceReadSimpleMessage\^(\^\fIice_conn\fP, \fI<C_data_type>\fP\^, \fIpmsg\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     <C_data_type> *\fIpmsg\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fI<C_data_type>\fP 1i
The actual C data type of the message header.
.IP \fIpmsg\fP 1i
This pointer is set to the message header.
.LP
.eM
.PN IceReadSimpleMessage
is used for messages which are identical in size to the 8 byte ICE header, but
use the spare 2 bytes in the header to encode additional data.  Note that the
ICE library always reads in these first 8 bytes so it can obtain the major
opcode of the message.
.PN IceReadSimpleMessage
simply returns a pointer to these 8 bytes, it does not actually read any data
into the input buffer.
.LP
For a message with variable length data, there are two ways of reading
the message.  One method involves reading the complete message in one
pass using
.PN IceReadCompleteMessage .
The second method involves reading the message header (note that this may
be larger than the 8 byte ICE header), then reading
the variable length data in chunks (see
.PN IceReadMessageHeader
and
.PN IceReadData ).
.LP
.sM
.FD 0
IceReadCompleteMessage\^(\^\fIice_conn\fP, \fIheader_size\fP\^, \fI<C_data_type>\fP\^, \fIpmsg\fP\^, \fIpdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIheader_size\fP\^;
.br
     <C_data_type> *\fIpmsg\fP\^;
.br
     char *\fIpdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIheader_size\fP 1i
The size of the message header (in bytes).
.IP \fI<C_data_type>\fP 1i
The actual C data type of the message header.
.IP \fIpmsg\fP 1i
This pointer is set to the message header.
.IP \fIpdata\fP 1i
This pointer is set to the variable length data of the message.
.LP
.eM
If the ICE input buffer has sufficient space,
.PN IceReadCompleteMessage
will read the complete message into the
ICE input buffer.  Otherwise, a buffer will be allocated to hold the
variable length data.  After the call, the \fIpdata\fP\^ argument should
be checked against
.PN NULL
to make sure that there was sufficient memory to allocate the buffer.
.LP
.sp 0.5
After calling
.PN IceReadCompleteMessage
and processing the message,
.PN IceDisposeCompleteMessage
should be called.
.LP
.sM
.FD 0
IceDisposeCompleteMessage\^(\^\fIice_conn\fP, \fIpdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     char *\fIpdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIpdata\fP 1i
The pointer to the variable length data returned in
.PN IceReadCompleteMessage .
.LP
.eM
If a buffer had to be allocated to hold the variable length data (because
it didn't fit in the ICE input buffer), it is freed here by ICElib.
.LP
.sp 0.5
.sM
.FD 0
IceReadMessageHeader\^(\^\fIice_conn\fP, \fIheader_size\fP\^, \fI<C_data_type>\fP\^, \fIpmsg\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIheader_size\fP\^;
.br
     <C_data_type> *\fIpmsg\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIheader_size\fP 1i
The size of the message header (in bytes).
.IP \fI<C_data_type>\fP 1i
The actual C data type of the message header.
.IP \fIpmsg\fP 1i
This pointer is set to the message header.
.LP
.eM
.PN IceReadMessageHeader
reads just the message header.  The rest
of the data should be read with the
.PN IceReadData
family of macros.  This method of reading a message should be used when the
variable length data must be read in chunks.
.LP
.sp 0.5
.bp
To read data directly into a user supplied buffer, use the
.PN IceReadData
macro.
.sM
.FD 0
IceReadData\^(\^\fIice_conn\fP, \fIbytes\fP\^, \fIpdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.br
     char *\fIpdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of bytes to read.
.IP \fIpdata\fP 1i
The data is read into this user supplied buffer.
.LP
.eM
To read data as 16 bit quantities, use the
.PN IceReadData16
macro.
.sM
.FD 0
IceReadData16\^(\^\fIice_conn\fP, \fIswap\fP\^, \fIbytes\fP\^, \fIpdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     Bool \fIswap\fP\^;
.br
     int \fIbytes\fP\^;
.br
     short *\fIpdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIswap\fP 1i
If
.PN True,
the values will be byte swapped.
.IP \fIbytes\fP 1i
The number of bytes to read.
.IP \fIpdata\fP 1i
The data is read into this user supplied buffer.
.LP
.eM
To read data as 32 bit quantities, use the
.PN IceReadData32
macro.
.sM
.FD 0
IceReadData32\^(\^\fIice_conn\fP, \fIswap\fP\^, \fIbytes\fP\^, \fIpdata\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     Bool \fIswap\fP\^;
.br
     int \fIbytes\fP\^;
.br
     long *\fIpdata\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIswap\fP 1i
If
.PN True,
the values will be byte swapped.
.IP \fIbytes\fP 1i
The number of bytes to read.
.IP \fIpdata\fP 1i
The data is read into this user supplied buffer.
.LP
.eM
.LP
.sp 0.5
To force 32 or 64 bit alignment, use the
.PN IceReadPad
macro.  A maximum of 7 pad bytes can be specified.
.sM
.FD 0
IceReadPad\^(\^\fIice_conn\fP, \fIbytes\fP\^)
.br
     IceConn \fIice_conn\fP\^;
.br
     int \fIbytes\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIbytes\fP 1i
The number of pad bytes.
.LP
.eM
.LP
.NH 1
Error Handling
.XS
\*(SN Error Handling
.XE
.LP
There are two default error handlers in ICElib: 
one to handle typically fatal conditions (for example, 
a connection dying because a machine crashed) 
and one to handle ICE-specific protocol errors.
These error handlers can be changed to user-supplied routines if you
prefer your own error handling and can be changed as often as you like.
.LP
.sp 0.5
To set the ICE error handler, use
.PN IceSetErrorHandler .
.LP
.sM
.FD 0
IceErrorHandler IceSetErrorHandler\^(\^\fIhandler\fP\^)
.br
    IceErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The ICE error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN IceSetErrorHandler
returns the previous error handler.
.LP
The ICE error handler is invoked when an unexpected ICE protocol
error (major opcode 0) is encountered.  The action of the default
handler is to print an explanatory message to stderr and if the
severity is fatal, call exit() with a non-zero value.  If exiting
is undesirable, the application should register its own error handler.
.LP
Note that errors in other protocol
domains should be handled by their respective libraries (these libraries
should have their own error handlers).
.LP
An ICE error handler has the type of
.PN IceErrorHandler :
.LP
.sp 0.5
.sM
typedef void (*IceErrorHandler)();
.FD 0
void ErrorHandler\^(\^\fIice_conn\fP, \fIswap\fP\^, \fIoffending_minor_opcode\fP\^, \fIoffending_sequence_num\fP\^, \fIerror_class\fP\^, \fIseverity\fP\^, \fIvalues\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    Bool \fIswap\fP\^;
.br
    int \fIoffending_minor_opcode\fP\^;
.br
    unsigned long \fIoffending_sequence_num\fP\^;
.br
    int \fIerror_class\fP\^;
.br
    int \fIseverity\fP\^;
.br
    IcePointer \fIvalues\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection object.
.IP \fIswap\fP 1i
A flag which indicates if the \fIvalues\fP\^ need byte swapping.
.IP \fIoffending_minor_opcode\fP 1i
The ICE minor opcode of the offending message.
.IP \fIoffending_sequence_num\fP 1i
The sequence number of the offending message.
.IP \fIerror_class\fP 1i
The error class of the offending message.
.IP \fIseverity\fP 1i
.PN IceCanContinue ,
.PN IceFatalToProtocol ,
or
.PN IceFatalToConnection .
.IP \fIvalues\fP 1i
Any additional error values specific to the minor opcode and class.
.LP
.eM
The following error classes are defined at the ICE level.  Refer to the
\fIInter-Client Exchange Protocol\fP\^ document for more details.
.LP
.PN IceBadMinor ,
.PN IceBadState ,
.PN IceBadLength ,
.PN IceBadValue ,
.PN IceBadMajor ,
.PN IceNoAuth ,
.PN IceNoVersion ,
.PN IceSetupFailed ,
.PN IceAuthRejected ,
.PN IceAuthFailed ,
.PN IceProtocolDuplicate ,
.PN IceMajorOpcodeDuplicate ,
or
.PN IceUnknownProtocol .
.LP
.sp 0.5
To handle fatal I/O errors, use
.PN IceSetIOErrorHandler .
.LP
.sM
.FD 0
IceIOErrorHandler IceSetIOErrorHandler\^(\^\fIhandler\fP\^)
.br
    IceIOErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The I/O error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN IceSetIOErrorHandler
returns the previous IO error handler.
.LP
An ICE I/O error handler has the type of
.PN IceIOErrorHandler :
.LP
.sM
typedef void (*IceIOErrorHandler)();
.LP
.FD 0
void IOErrorHandler\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection object.
.LP
.eM
There are two ways of handling IO errors in ICElib.
.LP
In the first model, the IO error handler does whatever is necessary
to respond to the IO error and then returns, but it does not call
.PN IceCloseConnection .
The ICE connection is given a "bad IO" status, and all future reads
and writes to the connection are ignored.  The next time
.PN IceProcessMessages
is called it will return a status of
.PN IceProcessMessagesIOError .
At that time, the application should call
.PN IceCloseConnection .
.LP
In the second model, the IO error handler does call
.PN IceCloseConnection ,
and then uses the longjmp() call to get back to the application's
main event loop.  setjmp() and longjmp() may not work properly on all
platforms and special care must be taken to avoid memory leaks, so
this second model is less desirable.
.LP
.sp 0.5
Before the application I/O error handler is invoked, protocol libraries
that were interested in being notified of I/O errors will have their
.PN IceIOErrorProc
handlers invoked.  This handler is set up in the protocol registration
functions (see
.PN IceRegisterForProtocolSetup
and 
.PN IceRegisterForProtocolReply ),
and could be used to clean up
state specific to the protocol.
.LP
.sM
typedef void (*IceIOErrorProc)();
.LP
.FD 0
void IOErrorProc\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection object.
.LP
.eM
Note that every
.PN IceIOErrorProc
callback must return.  This is required
because each active protocol must be notified of the broken connection,
and the application IO error handler must be invoked afterwards.
.LP
.NH 1
Multi-Threading Support
.XS
\*(SN Multi-Threading Support
.XE
.LP
To declare that multiple threads in an application will be using the ICE
library, call
.PN IceInitThreads .
.LP
.sM
.FD 0
Status IceInitThreads\^()
.FN
.LP
.eM
The
.PN IceInitThreads
function must be the first ICElib function a
multi-threaded program calls.  It must complete before any other ICElib
call is made.  IceInitThreads returns a non-zero status if and only if it
was able to successfully initialize the threads package.  It is safe to
call this function more than once, although the threads package will only
be initialized once.
.LP
.sp
Protocol libraries layered on top of ICElib will have to lock critical
sections of code that access an ICE connection (for example, when
generating messages).  Two calls, which are generally implemented as
macros, are provided:
.LP
.sM
.FD 0
IceLockConn\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
.sp
IceUnlockConn\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection.
.LP
.eM
To keep an ICE connection locked across several ICElib calls, applications use
.PN IceAppLockConn
and
.PN IceAppUnlockConn .
.LP
.sM
.FD 0
void IceAppLockConn\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection to lock.
.LP
.eM
The
.PN IceAppLockConn
function completely locks out other threads from ICElib
until
.PN IceAppUnlockConn
is called.  Other threads attempting to use ICElib will block.
If the program has not previously called
.PN IceInitThreads ,
.PN IceAppLockConn
has no effect.
.LP
.sM
.FD 0
void IceAppUnlockConn\^(\^\fIice_conn\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.FN
.IP \fIice_conn\fP 1i
The ICE connection to unlock.
.LP
.eM
The
.PN IceAppUnlockConn
function allows other threads to complete ICElib
calls which were blocked by a previous call to
.PN IceAppLockConn
from this thread.  If the program has not previously called
.PN IceInitThreads ,
.PN IceAppUnlockConn
has no effect.
.LP
.NH 1
Miscellaneous Functions
.XS
\*(SN Miscellaneous Functions
.XE
.LP
To allocate scratch space (for example, when generating
messages with variable data), use the
.PN IceAllocScratch
function.  Each ICE connection has one scratch space associated
with it.  The scratch space starts off as empty and grows as needed.
The contents of the scratch space is not guaranteed to be preserved
after any ICElib function is called.
.LP
.sM
.FD 0
char *IceAllocScratch\^(\^\fIice_conn\fP, \fIsize\fP\^)
.br
    IceConn \fIice_conn\fP\^;
.br
    unsigned long \fIsize\fP\^;
.FN
.IP \fIice_conn\fP 1i
A valid ICE connection object.
.IP \fIsize\fP 1i
The number of bytes required.
.LP
.eM
The memory returned by
.PN IceAllocScratch
should \fInot\fP\^ be freed by the caller!  The ICE library will free
the memory when the ICE connection is closed.
.bp
.XS
Appendix A \- Authentication Utility Functions
.XE
.ce 10
.sp 5
\s+2\fBAppendix A\fP\s-2
.sp 
\s+1\fBAuthentication Utility Functions\fP\s-1
.ce 0
.sp
.LP
As discussed in this document, the means by which authentication data
is obtained by the ICE library (for \fIConnection Setups\fP\^ or
\fIProtocol Setups\fP\^) is implementation dependent.\(dg
.FS \(dg
The X Consortium's ICElib implementation assumes the presence of an
ICE authority file.
.FE
.LP
This appendix describes some utility functions which manipulate an
ICE authority file.  The authority file can be used to pass authentication
data between clients.
.LP
The basic operations on the .ICEauthority file are : get file name, lock,
unlock, read entry, write entry, and search for entry.  These are fairly
low level operations, and it is expected that a program like "iceauth" would
exist to add, remove, and display entries in the file.
.LP
In order to use these utility functions, the \fI<X11/ICE/ICEutil.h>\fP\^
header file must be included.
.LP
.sp 0.5
An entry in the .ICEauthority file is defined by the following data structure:
.LP
.sp 0.5
.sM
typedef struct {
.br
	char *protocol_name;
.br
	unsigned short protocol_data_length;
.br
	char *protocol_data;
.br
	char *network_id;
.br
	char *auth_name;
.br
	unsigned short auth_data_length;
.br
	char *auth_data;
.br
} IceAuthFileEntry;
.LP
.eM
The \fIprotocol_name\fP\^ is either "ICE" for connection setup authentication,
or the sub-protocol name, such as "XSMP".  For each entry, protocol specific
data can be specified in the \fIprotocol_data\fP\^ field.  This can be used
to search for old entries that need to be removed from the file.
.LP
\fInetwork_id\fP\^ is the network ID of the client accepting authentication (for
example, the network ID of a session manager).  A network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<objname>	or
	local/<hostname>:<path>	
.TE
.LP
\fIauth_name\fP\^ is the name of the authentication method.  \fIauth_data\fP\^
is the actual authentication data, and \fIauth_data_length\fP\^ is the number
of bytes in the data.
.LP
.sp 0.5
To obtain the default authorization file name, call the
.PN IceAuthFileName
function.
.LP
.sM
.FD 0
char *IceAuthFileName\^()
.FN
.LP
.eM
If the ICEAUTHORITY environment variable if set, this value is returned.
Otherwise, the default authorization file name is $HOME/.ICEauthority.
This name is statically allocated and should not be freed.
.LP
In order to synchronously update the authorization file, the file must
be locked with a call to
.PN IceLockAuthFile .
This function takes advantage of the fact that the "link" system call
will fail if the name of the new link already exists.
.LP
.sM
.FD 0
int IceLockAuthFile\^(\^\fIfile_name\fP, \fIretries\fP\^, \fItimeout\fP\^, \fIdead\fP\^)
.br
    char *\fIfile_name\fP\^;
.br
    int \fIretries\fP\^;
.br
    int \fItimeout\fP\^;
.br
    long \fIdead\fP\^;
.FN
.IP \fIfile_name\fP 1i
The authorization file to lock.
.IP \fIretries\fP 1i
The number of retries.
.IP \fItimeout\fP 1i
The number of seconds before each retry.
.IP \fIdead\fP 1i
If a lock already exists that is \fIdead\fP\^ seconds old, it is broken.
A value of zero is used to unconditionally break an old lock.
.LP
.eM
One of three values is returned:
.TS
lw(2i) lw(5i).
T{
.PN IceAuthLockSuccess
T}	T{
The lock succeeded.
T}
.sp 4p
T{
.PN IceAuthLockError
T}	T{
A system error occurred.  errno may prove useful.
T}
.sp 4p
T{
.PN IceAuthLockTimeout
T}	T{
\fIretries\fP\^ attempts failed.
T}
.TE
.LP
.sp
To unlock an authorization file, call the
.PN IceUnlockAuthFile
function.
.LP
.sM
.FD 0
void IceUnlockAuthFile\^(\^\fIfile_name\fP\^)
.br
    char *\fIfile_name\fP\^;
.FN
.IP \fIfile_name\fP 1i
The authorization file to unlock.
.LP
.eM
.LP
To read the next entry in an authorization file, call the
.PN IceReadAuthFileEntry
function.
.LP
.sM
.FD 0
IceAuthFileEntry *IceReadAuthFileEntry\^(\^\fIauth_file\fP\^)
.br
    FILE *\fIauth_file\fP\^;
.FN
.IP \fIauth_file\fP 1i
The authorization file.
.LP
.eM
Note that it is the responsibility of the application to open the file
for reading before calling this function.  If an error is encountered,
or there are no more entries to read,
.PN NULL
is returned.
.LP
Entries should be free with a call to
.PN IceFreeAuthFileEntry
(see below).
.LP
.sp
To write an entry in an authorization file, call the
.PN IceWriteAuthFileEntry
function.
.LP
.sM
.FD 0
Status IceWriteAuthFileEntry\^(\^\fIauth_file\fP, \fIentry\fP\^)
.br
    FILE *\fIauth_file\fP\^;
.br
    IceAuthFileEntry *\fIentry\fP\^;
.FN
.IP \fIauth_file\fP 1i
The authorization file.
.IP \fIentry\fP 1i
The entry to write.
.LP
.eM
Note that it is the responsibility of the application to open the file
for writing before calling this function.  The function returns a non-zero
status if the operation was successful.
.LP
.sp
To search the default authorization file for an entry matching a given
protocol_name/network_id/auth_name tuple, call the
.PN IceGetAuthFileEntry
function.
.LP
.sM
.FD 0
IceAuthFileEntry *IceGetAuthFileEntry\^(\^\fIprotocol_name\fP, \fInetwork_id\fP\^, \fIauth_name\fP\^)
.br
    char *\fIprotocol_name\fP\^;
.br
    char *\fInetwork_id\fP\^;
.br
    char *\fIauth_name\fP\^;
.FN
.IP \fIprotocol_name\fP 1i
The name of the protocol to search on.
.IP \fInetwork_id\fP 1i
The network ID to search on.
.IP \fIauth_name\fP 1i
The authentication method to search on.
.LP
.eM
If
.PN IceGetAuthFileEntry
fails to find such an entry,
.PN NULL
is returned.
.LP
.sp
To free an entry returned by
.PN IceReadAuthFileEntry
or
.PN IceGetAuthFileEntry ,
call the
.PN IceFreeAuthFileEntry
function.
.sM
.FD 0
void IceFreeAuthFileEntry\^(\^\fIentry\fP\^)
.br
    IceAuthFileEntry *\fIentry\fP\^;
.FN
.IP \fIentry\fP 1i
The entry to free.
.LP
.eM
.bp
.XS
Appendix B \- MIT-MAGIC-COOKIE-1 Authentication
.XE
.ce 10
.sp 5
\s+2\fBAppendix B\fP\s-2
.sp 
\s+1\fBMIT-MAGIC-COOKIE-1 Authentication\fP\s-1
.ce 0
.sp
.LP
The X Consortium's ICElib implementation supports a simple
MIT-MAGIC-COOKIE-1 authentication scheme using the authority file utilities
described in Appendix A.
.LP
In this model, an application such as a session manager, obtains a
magic cookie by calling
.PN IceGenerateMagicCookie ,
and then stores it in the user's local .ICEauthority file
so that local clients can connect.  In order to allow remote clients to
connect, some remote execution mechanism should be used to store the
magic cookie in the user's .ICEauthority file on a remote machine.
.LP
In addition to storing the magic cookie in the .ICEauthority file, the
application needs to call the
.PN IceSetPaAuthData
function in order to store the magic cookie in memory.  When it comes time
for the MIT-MAGIC-COOKIE-1 authentication procedure to accept or reject the
connection, it will compare the magic cookie presented by the requestor to
the magic cookie in memory.
.LP
.sM
.FD 0
char *IceGenerateMagicCookie\^(\^\fIlength\fP\^)
.br
    int \fIlength\fP\^;
.FN
.IP \fIlength\fP 1i
The desired length of the magic cookie.
.LP
.eM
.LP
The magic cookie returned will be null terminated.  If memory can not be
allocated for the magic cookie, the function will return
.PN NULL .
Otherwise, the magic cookie should be freed with a call to free().
.LP
.sp 0.5
In order to store the authentication data in memory, call the
.PN IceSetPaAuthData
function.  Currently, this function is only used for MIT-MAGIC-COOKIE-1
authentication, but it may be used for additional authentication
methods in the future.
.LP
.sM
.FD 0
void IceSetPaAuthData\^(\^\fInum_entries\fP, \fIentries\fP\^)
.br
    int \fInum_entries\fP\^;
.br
    IceAuthDataEntry *\fIentries\fP\^;
.FN
.IP \fInum_entries\fP 1i
The number of authentication data entries.
.IP \fIentries\fP 1i
The list of authentication data entries.
.LP
.eM
Each entry has associated with it a protocol name (e.g. "ICE" for ICE
connection setup authentication, "XSMP" for session management
authentication), a network ID for the "accepting" client, an authentication
name (e.g. MIT-MAGIC-COOKIE-1), and authentication data.  The ICE library
will merge these entries with previously set entries, based on the
(protocol_name, network_id, auth_name) tuple.
.LP
.sp 0.5
.sM
typedef struct {
.br
	char *protocol_name;
.br
	char *network_id;
.br
	char *auth_name;
.br
	unsigned short auth_data_length;
.br
	char *auth_data;
.br
} IceAuthDataEntry;
.LP
.eM
.LP
.EH ''''
.OH ''''
.YZ 3
