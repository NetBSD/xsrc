.\" $XConsortium: SMlib.ms /main/14 1996/02/06 14:48:45 swick $
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
\s+2\fBX Session Management Library\fP\s-2
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
.sp 3
X Window System is a trademark of X Consortium, Inc.
.bp
.bp 1
.EH '\fBX Session Management Library\fP''\fBX11, Release 6.1\fP'
.OH '\fBX Session Management Library\fP''\fBX11, Release 6.1\fP'
.EF ''\- \\\\n(PN \-''
.OF ''\- \\\\n(PN \-''
.NH 1
Acknowledgements
.XS
\*(SN Acknowledgements
.XE
.LP
Thanks to the following people for their participation in the
X Session Management design: Jordan Brown, Ellis Cohen,
Donna Converse, Stephen Gildea, Vania Joloboff, Stuart Marks, Bob Scheifler,
Ralph Swick, and Mike Wexler.
.LP
.NH 1
Overview of Session Management
.XS
\*(SN Overview of Session Management
.XE
.LP
The purpose of the X Session Management Protocol (XSMP) is to provide a
uniform mechanism for users to save and restore their sessions.  A
\fIsession\fP is a group of clients, each of which has a particular state.
The session is controlled by a network service called the \fIsession
manager\fP\^.  The session manager issues commands to its clients on behalf
of the user.  These commands may cause clients to save their state or to
terminate.  It is expected that the client will save its state in such a
way that the client can be restarted at a later time and resume its
operation as if it had never been terminated.  A client's state might
include information about the file currently being edited, the current
position of the insertion point within the file, or the start of an 
uncommitted transaction.  The means by which clients are restarted is
unspecified by this protocol.
.LP
For purposes of this protocol, a \fIclient\fP\^ of the session manager is
defined as a connection to the session manager.  A client is typically,
though not necessarily, a process running an application program connected
to an X display.  However, a client may be connected to more
than one X display or not be connected to any X displays at all.
.LP
.NH 1
The Session Management Library
.XS
\*(SN The Session Management Library
.XE
.LP
The Session Management Library (SMlib) is a low level "C" language
interface to XSMP.  It is expected that higher level toolkits, such as
Xt, will hide many of
the details of session management from clients.  Higher level toolkits
might also be developed for session managers to use, but no such effort
is currently under way.
.LP
SMlib has two parts to it - one set of functions for clients that want to
be part of a session, and one set of functions for session managers to call.
Some applications will use both sets of functions and act as \fInested
session managers\fP\^ - they will be both a session manager and a client of
another session.  An example is a mail program that could start a text
editor for editing the text of a mail message.  The mail program is part of
a regular session, and at the same time, is also acting as a session manager
to the editor.
.LP
Clients initialize by connecting to the session manager and obtaining
a \fIclient-ID\fP\^ which uniquely identifies them in the session.
The session manager maintains a list of properties for each client in
the session.  These properties describe the client's environment,
and most importantly, describe how the client can be restarted (via an
\fISmRestartCommand\fP\^).
Clients are expected to save their state in such a way as to allow multiple
instantiations of themselves to be managed independently.  For example,
clients may use their \fIclient-ID\fP\^ as part of a filename in which
to store the state for a particular instantiation.  The \fIclient-ID\fP\^
should be saved as part of the \fISmRestartCommand\fP\^ so that the
client will retain the same ID after it is restarted.
.LP
Once the client initializes itself with the session manager, it must be
ready to respond to messages from the session manager.  For example, it
might be asked to save its state or to terminate.  In the case of a shutdown,
the session manager might give each client a chance to interact with the
user and cancel the shutdown.
.LP
.NH 1
Understanding SMlib's Dependence on ICE
.XS
\*(SN Understanding SMlib's Dependence on ICE
.XE
.LP
The X Session Management Protocol is layered on top of the Inter-Client
Exchange (ICE) Protocol.  The ICE protocol is designed to multiplex several
protocols over a single connection.  As a result, working with SMlib requires
a little knowledge of how the ICE library works.
.LP
The ICE library utilizes callbacks to process messages.  When a client
detects that there is data to read on an ICE connection, it should call
the
.PN IceProcessMessages
function.
.PN IceProcessMessages
will read the message header and look at the major opcode in order
to determine which protocol the message was intended for.  The appropriate
protocol library will then be triggered to unpack the message and hand it
off to the client via a callback.
.LP
The main point to be aware of is that an application using SMlib must
have some code which detects when there is data to read on an ICE connection.
This can be done via a select() call on the file descriptor for the
ICE connection, but more typically,
.PN XtAppAddInput
will be used to register a callback which will invoke
.PN IceProcessMessages
each time there is data to read on the ICE connection.
.LP
To further complicate things, knowing which file descriptors to select()
on requires an understanding of how ICE connections are created.
On the client side, a call must be made to
.PN SmcOpenConnection
in order to open a connection with a session manager.
.PN SmcOpenConnection
will internally make a call into
.PN IceOpenConnection
which will in turn determine if an ICE connection already exists between
the client and session manager.  Most likely, a connection will not already
exist and a new ICE connection will be created.  The main point to be aware
of is that on the client side, it is not obvious when ICE connections get
created or destroyed, since connections are shared when possible.
To deal with this, the ICE library lets the application register
\fIwatch procedures\fP\^ which
will be invoked each time an ICE connection is opened or closed.  These watch
procedures could be used to add or remove ICE file descriptors from the list of
descriptors to select() on.
.LP
On the session manager side, things work a bit differently.  The session
manager has complete control over the creation
of ICE connections.  The session manager has to first call
.PN IceListenForConnections
in order to start listening for connections from clients.  Once a connection
attempt is detected,
.PN IceAcceptConnection
must be called, and the session manager can simply add the new ICE
file descriptor to the list of descriptors to select() on.
.LP
Please refer to the \fIInter-Client Exchange Library\fP\^ document for
more details on the library functions related to ICE connections.
.LP
.NH 1
Header Files and Library Name
.XS
\*(SN Header Files and Library Name
.XE
.LP
Applications (both session managers and clients) should include the
header file \fI<X11/SM/SMlib.h>\fP\^.  This header file defines all of the
SMlib data structures and function prototypes.  \fISMlib.h\fP\^ includes the
header file \fI<X11/SM/SM.h>\fP\^ which defines all of the SMlib constants.
.LP
Since SMlib is dependent on ICE, applications should link against
SMlib and ICElib by using \fI-lSM -lICE\fP\^.
.LP
.bp
.NH 1
Session Management Client (Smc) Functions
.XS
\*(SN Session Management Client (Smc) Functions
.XE
.LP
.NH 2
Connecting to the Session Manager
.XS
\*(SN Connecting to the Session Manager
.XE
.LP
To open a connection with a session manager, call the
.PN SmcOpenConnection
function.
.LP
.sM
.FD 0
SmcConn SmcOpenConnection\^(\^\fInetwork_ids_list\fP, \fIcontext\fP\^, \fIxsmp_major_rev\fP\^, \fIxsmp_minor_rev\fP\^,
.br
                    \fImask\fP\^, \fIcallbacks\fP\^, \fIprevious_id\fP\^, \fIclient_id_ret\fP\^, \fIerror_length\fP\^, \fIerror_string_ret\fP\^)
.br
    char *\fInetwork_ids_list\fP\^;
.br
    SmPointer \fIcontext\fP\^;
.br
    int \fIxsmp_major_rev\fP\^;
.br
    int \fIxsmp_minor_rev\fP\^;
.br
    unsigned long \fImask\fP\^;
.br
    SmcCallbacks *\fIcallbacks\fP\^;
.br
    char *\fIprevious_id\fP\^;
.br
    char **\fIclient_id_ret\fP\^;
.br
    int \fIerror_length\fP\^;
.br
    char *\fIerror_string_ret\fP\^;
.FN
.IP \fInetwork_ids_list\fP 1.1i
Specifies the network ID(s) of the session manager.
.IP \fIcontext\fP 1.1i
A pointer to an opaque object, or NULL.  Used to determine if an
ICE connection can be shared (see below).
.IP \fIxsmp_major_rev\fP 1.1i
The highest major version of the XSMP the application supports.
.IP \fIxsmp_minor_rev\fP 1.1i
The highest minor version of the XSMP the application supports (for the
specified \fIxsmp_major_rev\fP\^).
.IP \fImask\fP\^ 1.1i
A mask indicating which callbacks to register.
.IP \fIcallbacks\fP 1.1i
The callbacks to register.  These callbacks are used to respond to messages
from the session manager.
.IP \fIprevious_id\fP 1.1i
The client ID from the previous session.
.IP \fIclient_id_ret\fP 1.1i
The client ID for the current session is returned.
.IP \fIerror_length\fP 1.1i
Length of the \fIerror_string_ret\fP\^ argument passed in.
.IP \fIerror_string_ret\fP 1.1i
Returns a null terminated error message, if any.  \fIerror_string_ret\fP
points to user supplied memory.  No more than \fIerror_length\fP\^ bytes
are used.
.LP
.eM
\fInetwork_ids_list\fP\^ is a null terminated string containing a list of
network IDs for the session manager, separated by commas.
If \fInetwork_ids_list\fP\^ is
.PN NULL ,
the value of the
.PN SESSION_MANAGER
environment variable will be used.
Each network ID has the form...
.br
.TS
lw(0.25i) lw(2.5i) lw(1i).
	tcp/<hostname>:<portnumber>	or
	decnet/<hostname>::<objname>	or
	local/<hostname>:<path>	
.TE
.LP
An attempt will be made to use the first network ID.  If that fails,
an attempt will be made using the second network ID, and so on.
.LP
After the connection is established,
.PN SmcOpenConnection
registers the client with the session manager.  If the client is being
restarted from a previous session, \fIprevious_id\fP\^ should contain a null
terminated string representing the client ID from the previous session.
If the client is first joining the session, \fIprevious_id\fP\^ should be
set to
.PN NULL .
If \fIprevious_id\fP\^ is specified, but is determined to be invalid by the
session manager, SMlib will re-register
the client with \fIprevious_id\fP\^ set to
.PN NULL .
.LP
If
.PN SmcOpenConnection
succeeds, the function returns an opaque connection pointer of type
.PN SmcConn
and the \fIclient_id_ret\fP\^ argument contains the client ID to be used for
this session.  \fIclient_id_ret\fP\^ should be freed with a call to free()
when no longer needed.  On failure,
.PN SmcOpenConnection
returns
.PN NULL
and the reason for failure is returned in \fIerror_string_ret\fP\^.
.LP
Note that SMlib uses the ICE protocol to establish a connection with
the session manager.  If an ICE connection already exists between the
client and session manager, it might be possible for the same ICE connection
to be used for session management.  
.LP
The \fIcontext\fP\^ argument indicates how willing the client is to share
the ICE connection with other protocols.  If \fIcontext\fP\^ is
.PN NULL ,
then the caller is always willing to share the connection.
If \fIcontext\fP\^ is not
.PN NULL ,
then the caller is not willing to use a previously opened ICE connection
that has a different non-NULL context associated with it.
.LP
As discussed in the section titled \fIUnderstanding SMlib's Dependence
on ICE\fP\^, the client will have to keep track of when ICE connections
are created or destroyed (using 
.PN IceAddConnectionWatch
and
.PN IceRemoveConnectionWatch ),
and will have to call
.PN IceProcessMessages
each time select() shows that there is data to read on an ICE connection.
Please refer to the \fIInter-Client Exchange Library\fP\^ document for
more details.
.LP
.sp 0.5
\fIcallbacks\fP\^ contains a set of callbacks used to respond to session
manager events.  The \fImask\fP\^ argument specifies which callbacks are set.
All of the callbacks specified in this version of SMlib are mandatory.  The
\fImask\fP\^ argument is necessary in order to maintain backwards compatibility
in future versions of the library.
.LP
The following values may be ORed together to obtain a mask value:
.LP
	
.PN SmcSaveYourselfProcMask
.br
	
.PN SmcDieProcMask
.br
	
.PN SmcSaveCompleteProcMask
.br
	
.PN SmcShutdownCancelledProcMask
.LP

.LP
For each callback, the client can register a pointer to client data.
When SMlib invokes the callback, it will pass the client data pointer.
.LP
.sM
	typedef struct {
.sp 0.5
		struct {
.br
			SmcSaveYourselfProc callback;
.br
			SmPointer client_data;
.br
		} save_yourself;
.sp 0.5
		struct {
.br
			SmcDieProc callback;
.br
			SmPointer client_data;
.br
		} die;
.sp 0.5
		struct {
.br
			SmcSaveCompleteProc callback;
.br
			SmPointer client_data;
.br
		} save_complete;
.sp 0.5
		struct {
.br
			SmcShutdownCancelledProc callback;
.br
			SmPointer client_data;
.sp 0.5
		} shutdown_cancelled;
.sp 0.5
	} SmcCallbacks;
.LP
.eM
.LP
.NH 3
The Save Yourself Callback
.XS
\*(SN The Save Yourself Callback
.XE
.LP
The \fISave Yourself\fP\^ callback is of type
.PN SmcSaveYourselfProc .
.LP
.sM
typedef void (*SmcSaveYourselfProc)();
.LP
.FD 0
void SaveYourselfProc\^(\^\fIsmc_conn\fP, \fIclient_data\fP\^, \fIsave_type\fP\^, \fIshutdown\fP\^, \fIinteract_style\fP\^, \fIfast\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.br
    int \fIsave_type\fP\^;
.br
    Bool \fIshutdown\fP\^;
.br
    int \fIinteract_style\fP\^;
.br
    Bool \fIfast\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIclient_data\fP 1i
Client data specified when the callback was registered.
.IP \fIsave_type\fP 1i
Specifies the type of information that should be saved.
.IP \fIshutdown\fP 1i
Specifies if a shutdown is taking place.
.IP \fIinteract_style\fP 1i
The type of interaction allowed with the user.
.IP \fIfast\fP 1i
If
.PN True ,
the client should save its state as quickly as possible.
.LP
.eM
The session manager sends a \fISave Yourself\fP\^ message to a client 
either to checkpoint it or just before
termination so that it can save its state.
The client responds with zero or more calls to
.PN SmcSetProperties
to update the properties indicating how to restart the client.
When all the properties have been set, the client calls
.PN SmcSaveYourselfDone .
.LP
If \fIinteract_style\fP\^ is
.PN SmInteractStyleNone ,
the client must not interact with the
user while saving state.
If \fIinteract_style\fP\^ is 
.PN SmInteractStyleErrors ,
the client may interact with the user only if an error condition arises.  If
\fIinteract_style\fP\^ is 
.PN SmInteractStyleAny ,
then the client may interact with the user for any purpose.
Since only one client can interact with the user at a time, the client
must call
.PN SmcInteractRequest
and wait for an \fIInteract\fP\^ message from the session manager.
When the client is done interacting with the user, it calls
.PN SmcInteractDone .
The client may only call
.PN SmcInteractRequest
after it receives a \fISave Yourself\fP\^ message and before it
calls
.PN SmcSaveYourselfDone .
.LP
If \fIsave_type\fP\^ is
.PN SmSaveLocal ,
the client must update the properties to reflect its current state.
Specifically, it should save enough information to restore
the state as seen by the user of this client.  It should not affect the
state as seen by other users.  If \fIsave_type\fP\^ is
.PN SmSaveGlobal
the user wants the client to commit all of its data to permanent,
globally accessible storage.  If \fIsave_type\fP\^ is
.PN SmSaveBoth ,
the client should do both of these (it should first commit the data to
permanent storage before updating its properties).
.LP
Some examples:
.LP
.IP
If a word processor were sent a \fISave Yourself\fP\^ with a type of 
.PN SmSaveLocal ,
it could create a temporary file that included the
current contents of the file, the location of the cursor, and
other aspects of the current editing session.  It would then update
its 
.PN SmRestartCommand
property with enough information to find this 
temporary file.
.sp
If a word processor were sent a \fISave Yourself\fP\^ with a type of
.PN SmSaveGlobal ,
it would simply save the currently edited file.
.sp
If a word processor were sent a \fISave Yourself\fP\^ with a type of
.PN SmSaveBoth ,
it would first save the currently edited file.  It would then create a
temporary file with information such as the current position of the cursor
and what file is being edited.  Finally, it would update its 
.PN SmRestartCommand
property with enough information to find the temporary file.
.LP
.sp 0.5
The \fIshutdown\fP\^ parameter specifies whether the system is being
shut down.  The interaction is different depending on whether or not
\fIshutdown\fP\^ is set.  If not shutting down, the client should save its
state and wait for a \fISave Complete\fP\^ message.  If shutting down,
the client must save state and
then prevent interaction until it receives either a \fIDie\fP\^
or a \fP\^Shutdown Cancelled\fP\^.
.LP
The \fIfast\fP\^ parameter specifies that the client should save its state
as quickly as possible.  For example, if the session manager knows that
power is about to fail, it would set \fIfast\fP\^ to
.PN True .
.LP
.NH 3
The Die Callback
.XS
\*(SN The Die Callback
.XE
.LP
The \fIDie\fP\^ callback is of type
.PN SmcDieProc .
.LP
.sM
typedef void (*SmcDieProc)();
.LP
.FD 0
void DieProc\^(\^\fIsmc_conn\fP, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIclient_data\fP 1i
Client data specified when the callback was registered.
.LP
.eM
The session manager sends a \fIDie\fP\^ message to a client 
when it wants it to die.  The client should respond by calling
.PN SmcCloseConnection .
A session manager that behaves properly will send a
\fISave Yourself\fP\^ message before the \fIDie\fP\^ message.
.LP
.NH 3
The Save Complete Callback
.XS
\*(SN The Save Complete Callback
.XE
.LP
The \fISave Complete\fP\^ callback is of type
.PN SmcSaveCompleteProc .
.LP
.sM
typedef void (*SmcSaveCompleteProc)();
.LP
.FD 0
void SaveCompleteProc\^(\^\fIsmc_conn\fP, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIclient_data\fP 1i
Client data specified when the callback was registered.
.LP
.eM
When the session manager is done with a checkpoint, it will send each of
the clients a \fISave Complete\fP\^ message.  The client is then free to
change its state.
.LP
.NH 3
The Shutdown Cancelled Callback
.XS
\*(SN The Shutdown Cancelled Callback
.XE
.LP
The \fIShutdown Cancelled\fP\^ callback is of type
.PN SmcShutdownCancelledProc .
.LP
.sM
typedef void (*SmcShutdownCancelledProc)();
.LP
.FD 0
void ShutdownCancelledProc\^(\^\fIsmc_conn\fP, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIclient_data\fP 1i
Client data specified when the callback was registered.
.LP
.eM
The session manager sends a \fIShutdown Cancelled\fP\^ message
when the user cancelled the shutdown during an interaction
(see the section titled \fIInteracting With the User\fP\^).
The client can now continue as if the shutdown had never happened.
If the client has not called
.PN SmcSaveYourselfDone
yet, it can either abort the save and then call
.PN SmcSaveYourselfDone
with the \fIsuccess\fP\^ argument set to
.PN False ,
or it can continue with the save and then call
.PN SmcSaveYourselfDone
with the \fIsuccess\fP\^ argument set to reflect the outcome of the save.
.LP
.NH 2
Closing the Connection
.XS
\*(SN Closing the Connection
.XE
.LP
To close a connection with a session manager, call the
.PN SmcCloseConnection
function.
.LP
.sM
.FD 0
SmcCloseStatus SmcCloseConnection\^(\^\fIsmc_conn\fP, \fIcount\fP\^, \fIreason_msgs\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    int \fIcount\fP\^;
.br
    char **\fIreason_msgs\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIcount\fP 1i
The number of reason messages.
.IP \fIreason_msgs\fP 1i
The reasons for closing the connection.
.LP
.eM
\fIreason_msgs\fP\^ will most likely be
.PN NULL
if resignation is expected by the client.  Otherwise, it contains a list
of null terminated Compound Text strings representing the reason for
termination.  The session manager should display these reason messages
to the user.
.LP
Note that SMlib used the ICE protocol to establish a connection with
the session manager, and various protocols other than session management
may be active on the ICE connection.  When
.PN SmcCloseConnection
is called, the ICE connection will be closed only if all protocols
have been shutdown on the connection.  Check the ICElib
documentation for
.PN IceAddConnectionWatch
and
.PN IceRemoveConnectionWatch
to learn how to set up a callback to be invoked each time an ICE connection is
opened or closed.  Typically this callback adds/removes the ICE file
descriptor from the list of active descriptors to select() on (or calls
.PN XtAppAddInput
/
.PN XtRemoveInput ).
.LP
.sp 0.5
.PN SmcCloseConnection
returns one of the following values:
.LP
.TS
lw(2i) lw(4i).
T{
.PN SmcClosedNow :
T}	T{
The ICE connection was closed at this time.  The watch procedures were
invoked and the connection was freed.
T}
.sp 4p
T{
.PN SmcClosedASAP :
T}	T{
An IO error had occurred on the connection, but
.PN SmcCloseConnection
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
.PN SmcConnectionInUse :
T}	T{
The connection was not closed at this time because it is being used by
other active protocols.
T}
.TE
.LP
.NH 2
Modifying callbacks
.XS
\*(SN Modifying callbacks
.XE
.LP
To modify callbacks set up in
.PN SmcOpenConnection ,
call the
.PN SmcModifyCallbacks
function.
.LP
.sM
.FD 0
void SmcModifyCallbacks\^(\^\fIsmc_conn\fP, \fImask\fP\^, \fIcallbacks\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    unsigned long \fImask\fP\^;
.br
    SmcCallbacks *\fIcallbacks\fP\^;
.FN
.ne 4
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fImask\fP 1i
A mask indicating which callbacks to modify.
.IP \fIcallbacks\fP 1i
The new callbacks.
.LP
.eM
When specifying a value for \fImask\fP\^, the following
values may be ORed together:
.LP
	
.PN SmcSaveYourselfProcMask
.br
	
.PN SmcDieProcMask
.br
	
.PN SmcSaveCompleteProcMask
.br
	
.PN SmcShutdownCancelledProcMask
.LP
.NH 2
Setting, Deleting, and Retrieving Session Management Properties
.XS
\*(SN Setting, Deleting, and Retrieving Session Management Properties
.XE
.LP
To set session management properties for this client, call the
.PN SmcSetProperties
function.
.sM
.FD 0
void SmcSetProperties\^(\^\fIsmc_conn\fP, \fInum_props\fP\^, \fIprops\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    int \fInum_props\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fInum_props\fP 1i
The number of properties.
.IP \fIprops\fP 1i
The list of properties to set.
.LP
.eM
The properties are specified as an array of property pointers.
Previously set property values may be over-written using the
.PN SmcSetProperties
function.  Note that the session manager is not
expected to restore property values when the session is restarted.  Because
of this, clients should not try to use the session manager as
a database for storing application specific state.
.LP
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled \fISession Management Properties\fP\^.
.LP
.sp 0.5
.LP
To delete properties previously set by the client, call the
.PN SmcDeleteProperties
function.
.sM
.FD 0
void SmcDeleteProperties\^(\^\fIsmc_conn\fP, \fInum_props\fP\^, \fIprop_names\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    int \fInum_props\fP\^;
.br
    char **\fIprop_names\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fInum_props\fP 1i
The number of properties.
.IP \fIprop_names\fP 1i
The list of properties to delete.
.LP
.eM
.sp 0.5
To get properties previously stored by the client, call the
.PN SmcGetProperties
function.
.sM
.FD 0
Status SmcGetProperties\^(\^\fIsmc_conn\fP, \fIprop_reply_proc\fP\^, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmcPropReplyProc \fIprop_reply_proc\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.ne 4
.IP \fIsmc_conn\fP 1.1i
The session management connection object.
.IP \fIprop_reply_proc\fP 1.1i
The callback to be invoked when the properties reply comes back.
.IP \fIclient_data\fP 1.1i
This pointer to client data will be passed to the
.PN SmcPropReplyProc
callback.
.LP
.eM
The return value of
.PN SmcGetProperties
is zero for failure, and a positive value for success.
.LP
Note that the library does not block until the properties reply comes back.
Rather, a callback of type
.PN SmcPropReplyProc
is invoked when the data is ready.
.LP
.sM
typedef void (*SmcPropReplyProc)();
.LP
.FD 0
void PropReplyProc\^(\^\fIsmc_conn\fP, \fIclient_data\fP\^, \fInum_props\fP\^, \fIprops\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.br
    int \fInum_props\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIclient_data\fP 1i
Client data specified when the callback was registered.
.IP \fInum_props\fP 1i
The number of properties returned.
.IP \fIprops\fP 1i
The list of properties returned.
.LP
.eM
In order to free each property, call the
.PN SmFreeProperty
function (see the section titled \fIFreeing Data\fP\^).
Free the actual array of pointers with a call to free().
.LP
.NH 2
Interacting With the User
.XS
\*(SN Interacting With the User
.XE
.LP
After receiving a \fISave Yourself\fP\^ message with an \fIinteract_style\fP\^
of
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny ,
the client may choose to interact with the user.
Since only one client can interact with the user at a time, the client
must call
.PN SmcInteractRequest
and wait for an \fIInteract\fP\^ message from the session manager.
.sM
.FD 0
Status SmcInteractRequest\^(\^\fIsmc_conn\fP, \fIdialog_type\fP\^, \fIinteract_proc\fP\^, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    int \fIdialog_type\fP\^;
.br
    SmcInteractProc \fIinteract_proc\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIdialog_type\fP 1i
The type of dialog the client wishes to present to the user.
.IP \fIinteract_proc\fP 1i
The callback to be invoked when the \fIInteract\fP\^ message arrives from
the session manager.
.IP \fIclient_data\fP 1i
This pointer to client data will be passed to the
.PN SmcInteractProc
callback when the \fIInteract\fP\^ message arrives.
.LP
.eM
The return value of
.PN SmcInteractRequest
is zero for failure, and a positive value for success.
.LP
The \fIdialog_type\fP argument specifies either
.PN SmDialogError
indicating that the client wants to start an error dialog,
or
.PN SmDialogNormal ,
meaning that the client wishes to start a non-error dialog.
.LP
Note that if a shutdown is in progress, the user may have the option of
cancelling the shutdown.  If the shutdown is cancelled, the clients that
have not interacted yet with the user will receive a
\fIShutdown Cancelled\fP\^ message instead of the \fIInteract\fP\^ message.
.LP
The
.PN SmcInteractProc
callback will be invoked when the \fIInteract message\fP\^ arrives from
the session manager.
.LP
.sM
typedef void (*SmcInteractProc)();
.LP
.FD 0
void InteractProc\^(\^\fIsmc_conn\fP, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIclient_data\fP 1i
Client data specified when the callback was registered.
.LP
.eM
After interacting with the user (in response to an \fIInteract\fP\^ message),
call the
.PN SmcInteractDone
function.
.PN 
.sM
.FD 0
void SmcInteractDone\^(\^\fIsmc_conn\fP, \fIcancel_shutdown\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    Bool \fIcancel_shutdown\fP\^;
.FN
.IP \fIsmc_conn\fP 1.1i
The session management connection object.
.IP \fIcancel_shutdown\fP 1.1i
If
.PN True ,
indicates that the user requests that the entire shutdown be cancelled.
.LP
.eM
\fIcancel_shutdown\fP may only be
.PN True
if the corresponding \fISave Yourself\fP specified
.PN True
for \fIshutdown\fP\^ and
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny
for the \fIinteract_style\fP\^.
.LP
.NH 2
Requesting a Save Yourself
.XS
\*(SN Requesting a Save Yourself
.XE
.LP
To request a checkpoint from the session manager, call the
.PN SmcRequestSaveYourself
function.
.LP
.sM
.FD 0
void SmcRequestSaveYourself\^(\^\fIsmc_conn\fP, \fIsave_type\fP\^, \fIshutdown\fP\^, \fIinteract_style\fP\^, \fIfast\fP\^, \fIglobal\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    int \fIsave_type\fP\^;
.br
    Bool \fIshutdown\fP\^;
.br
    int \fIinteract_style\fP\^;
.br
    Bool \fIfast\fP\^;
.br
    Bool \fIglobal\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIsave_type\fP 1i
Specifies the type of information that should be saved.
.IP \fIshutdown\fP 1i
Specifies if a shutdown is taking place.
.IP \fIinteract_style\fP 1i
The type of interaction allowed with the user.
.IP \fIfast\fP 1i
If
.PN True ,
the client should save its state as quickly as possible.
.IP \fIglobal\fP 1i
Controls who gets the \fISave Yourself\fP\^.
.LP
.eM
The \fIsave_type\fP\^, \fIshutdown\fP\^, \fIinteract_style\fP\^, and
\fIfast\fP\^ parameters are discussed in the previous section titled
\fIThe Save Yourself Callback\fP\^.
.LP
If \fIglobal\fP\^ is set to
.PN True ,
then the resulting \fISave Yourself\fP\^ should be
sent to all clients in the session.  For example, a vendor of a
UPS (Uninterruptible Power Supply) might include an
SM client that would monitor the status of the UPS and generate
a fast shutdown if the power is about to be lost.
.LP
If \fIglobal\fP\^ is set to
.PN False ,
then the \fISave Yourself\fP\^ should only be sent to the client which
requested the \fISave Yourself\fP\^.
.LP
.NH 2
Requesting a Save Yourself Phase 2
.XS
\*(SN Requesting a Save Yourself Phase 2
.XE
.LP
In response to a \fISave Yourself\fP\^, the client may request to be informed
when all the other clients are quiescent so it can save their state.  To
do so, the
.PN SmcRequestSaveYourselfPhase2
function should be called.
.LP
.sM
.FD 0
Status SmcRequestSaveYourselfPhase2\^(\^\fIsmc_conn\fP, \fIsave_yourself_phase2_proc\fP\^, \fIclient_data\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    SmcSaveYourselfPhase2Proc \fIsave_yourself_phase2_proc\fP\^;
.br
    SmPointer \fIclient_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIsave_yourself_phase2_proc\fP 1i
The callback to be invoked when the \fISave Yourself Phase 2\fP\^ message
arrives from the session manager.
.IP \fIclient_data\fP 1i
This pointer to client data will be passed to the
.PN SmcSaveYourselfPhase2Proc
callback when the \fISave Yourself Phase 2\fP\^ message arrives.
.LP
.eM
The return value of
.PN SmcRequestSaveYourselfPhase2
is zero for failure, and a positive value for success.
.LP
This request is needed by clients that manage other clients (e.g. window
managers, workspace managers, etc.).  The manager must make sure that all
of the clients that are being managed are in an idle state, so that their
state can be saved.
.LP
.NH 2
Completing a Save Yourself
.XS
\*(SN Completing a Save Yourself
.XE
.LP
After saving state in response to a \fISave Yourself\fP\^ message,
call the
.PN SmcSaveYourselfDone
function.
.sM
.FD 0
void SmcSaveYourselfDone\^(\^\fIsmc_conn\fP, \fIsuccess\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.br
    Bool \fIsuccess\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIsuccess\fP 1i
If
.PN True ,
the \fISave Yourself\fP\^ operation was completed successfully.
.LP
.eM
Before calling
.PN SmcSaveYourselfDone ,
the client must have set each required property at least once since
the client registered with the session manager.
.LP
.NH 2
Informational Functions
.XS
\*(SN Informational Functions
.XE
.LP
.sM
.FD 0
int SmcProtocolVersion\^(\^\fIsmc_conn\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.FN
.eM
Returns the major version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
int SmcProtocolRevision\^(\^\fIsmc_conn\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.FN
.eM
Returns the minor version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
char *SmcVendor\^(\^\fIsmc_conn\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.FN
.eM
Returns a string that provides some identification of the owner of
the session manager.  The string should be freed with a call to free().
.LP
.sp 0.5
.sM
.FD 0
char *SmcRelease\^(\^\fIsmc_conn\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.FN
.eM
Returns a string that provides the release number of the session manager.
The string should be freed with a call to free().
.LP
.sp 0.5
.sM
.FD 0
char *SmcClientID\^(\^\fIsmc_conn\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.FN
.eM
Returns a null terminated string for the client Id associated with
this connection.  This information was also returned in
.PN SmcOpenConnection
(it is provided here for convenience).
.LP
Call free() on this pointer when the client Id is no longer needed.
.LP
.sp 0.5
.sM
.FD 0
IceConn SmcGetIceConnection\^(\^\fIsmc_conn\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
.FN
.eM
Returns the ICE connection object associated with this session management
connection object.  The ICE connection object can be used to get some
additional information about the connection.  Some of the more useful
functions which can be used on the IceConn are IceConnectionNumber,
IceConnectionString, IceLastSentSequenceNumber, IceLastReceivedSequenceNumber,
and IcePing.  Check the ICElib documentation for more details on these
functions.
.LP
.NH 2
Error Handling
.XS
\*(SN Error Handling
.XE
.LP
If the client receives an unexpected protocol error from the session manager,
an error handler is invoked by SMlib.  A default error handler exists which
simply prints the error message to stderr and exits if the severity of the
error is fatal.  The client can change this error handler by calling the
.PN SmcSetErrorHandler
function.
.LP
.sM
.FD 0
SmcErrorHandler SmcSetErrorHandler\^(\^\fIhandler\fP\^)
.br
    SmcErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN SmcSetErrorHandler
returns the previous error handler.
.LP
The
.PN SmcErrorHandler
has the following type:
.LP
.sp 0.5
.sM
typedef void (*SmcErrorHandler)();
.br
.FD 0
void ErrorHandler\^(\^\fIsmc_conn\fP, \fIswap\fP\^, \fIoffending_minor_opcode\fP\^, \fIoffending_sequence_num\fP\^, \fIerror_class\fP\^, \fIseverity\fP\^, \fIvalues\fP\^)
.br
    SmcConn \fIsmc_conn\fP\^;
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
.bp
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fIswap\fP 1i
A flag which indicates if the \fIvalues\fP\^ need byte swapping.
.IP \fIoffending_minor_opcode\fP 1i
The minor opcode of the offending message.
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
Note that this error handler is invoked for protocol related errors.
To install an error handler to be invoked when an IO error occurs, use
the
.PN IceSetIOErrorHandler
function described in the \fIInter-Client Exchange Library\fP\^ document.
.LP
.bp
.NH 1
Session Management Server (Sms) Functions
.XS
\*(SN Session Management Server (Sms) Functions
.XE
.LP
.NH 2
Initializing the Library
.XS
\*(SN Initializing the Library
.XE
.LP
.PN SmsInitialize
is the first SMlib function that should be called by a
session manager.  It provides information about the session manager,
and registers a callback which will be invoked each
time a new client connects to the session manager.
.LP
.sM
.FD 0
Status SmsInitialize\^(\^\fIvendor\fP, \fIrelease\fP\^, \fInew_client_proc\fP\^, \fImanager_data\fP\^,
.br
                    \fIhost_based_auth_proc\fP\^, \fIerror_length\fP\^, \fIerror_string_ret\fP\^)
.br
    char *\fIvendor\fP\^;
.br
    char *\fIrelease\fP\^;
.br
    SmsNewClientProc \fInew_client_proc\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    IceHostBasedAuthProc \fIhost_based_auth_proc\fP\^;
.br
    int \fIerror_length\fP\^;
.br
    char *\fIerror_string_ret\fP\^;
.FN
.IP \fIvendor\fP 1i
A string specifying the session manager vendor.
.IP \fIrelease\fP 1i
A string specifying the session manager release number.
.IP \fInew_client_proc\fP 1i
Callback to be invoked each time a new client connects to the session manager.
.IP \fImanager_data\fP 1i
When the
.PN SmsNewClientProc
callback is invoked, this pointer to manager data will be passed.
.IP \fIhost_based_auth_proc\fP 1i
Host based authentication callback.
.IP \fIerror_length\fP 1i
Length of the \fIerror_string_ret\fP\^ argument passed in.
.IP \fIerror_string_ret\fP 1i
Returns a null terminated error message, if any.  \fIerror_string_ret\fP
points to user supplied memory.  No more than \fIerror_length\fP\^ bytes
are used.
.LP
.eM
After the
.PN SmsInitialize
function is called, the session manager should call the
.PN IceListenForConnections
function to listen for new connections.  Afterwards, each time a
client connects, the session manager should call
.PN IceAcceptConnection .
.LP
Refer to the section of this document titled \fIAuthentication of Clients\fP\^
for more details on authentication (including host based authentication).
Also refer to the \fIInter-Client
Exchange Library\fP\^ document for further details on listening for and
accepting ICE connections.
.LP
Each time a new client connects to the session manager, the
.PN SmsNewClientProc
callback is invoked.  The session manager obtains a new opaque connection
object which it should use for all future interaction with the client.  At
this time, the session manager must also register a set of callbacks to
respond to the different messages that the client might send.
.LP
.sM
typedef Status (*SmsNewClientProc)();
.LP
.FD 0
Status NewClientProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fImask_ret\fP\^, \fIcallbacks_ret\fP\^, \fIfailure_reason_ret\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    unsigned long *\fImask_ret\fP\^;
.br
    SmsCallbacks *\fIcallbacks_ret\fP\^;
.br
    char **\fIfailure_reason_ret\fP\^;
.FN
.bp
.IP \fIsms_conn\fP 1.1i
A new opaque connection object.
.IP \fImanager_data\fP 1.1i
Manager data specified when the callback was registered.
.IP \fImask_ret\fP 1.1i
On return, indicates which callbacks were set by the session manager.
.IP \fIcallbacks_ret\fP 1.1i
On return, contains the callbacks registered by the session manager.
.IP \fIfailure_reason_ret\fP 1.1i
Failure reason returned.
.LP
.eM
If a failure occurs, the
.PN SmsNewClientProc
should return a zero status, as well as allocate and return a failure
reason string in \fIfailure_reason_ret\fP\^.  SMlib will be
responsible for freeing this memory.
.LP
The session manager must register a set of callbacks to respond to client
events.  The \fImask_ret\fP\^ argument specifies which callbacks are set.
All of the callbacks specified in this version of SMlib are mandatory.  The
\fImask_ret\fP\^ argument is necessary in order to maintain backwards
compatibility in future versions of the library.
.LP
The following values may be ORed together to obtain a mask value:
.LP
	
.PN SmsRegisterClientProcMask
.br
	
.PN SmsInteractRequestProcMask
.br
	
.PN SmsInteractDoneProcMask
.br
	
.PN SmsSaveYourselfRequestProcMask
.br
	
.PN SmsSaveYourselfP2RequestProcMask
.br
	
.PN SmsSaveYourselfDoneProcMask
.br
	
.PN SmsCloseConnectionProcMask
.br
	
.PN SmsSetPropertiesProcMask
.br
	
.PN SmsDeletePropertiesProcMask
.br
	
.PN SmsGetPropertiesProcMask
.LP

.LP
For each callback, the session manager can register a pointer to manager
data specific to that callback.  This pointer will be passed to the callback
when it is invoked by SMlib.
.LP
.sM
	typedef struct {
.sp 0.5
		struct {
.br
			SmsRegisterClientProc callback;
.br
			SmPointer manager_data;
.br
		} register_client;
.sp 0.5	
		struct {
.br
			SmsInteractRequestProc callback;
.br
			SmPointer manager_data;
.br
		} interact_request;
.sp 0.5
		struct {
.br
			SmsInteractDoneProc callback;
.br
			SmPointer manager_data;
.br
		} interact_done;
.sp 0.5
		struct {
.br
			SmsSaveYourselfRequestProc callback;
.br
			SmPointer manager_data;
.br
		} save_yourself_request;
.sp 0.5
		struct {
.br
			SmsSaveYourselfPhase2RequestProc callback;
.br
			SmPointer manager_data;
.br
		} save_yourself_phase2_request;
.bp

		struct {
.br
			SmsSaveYourselfDoneProc callback;
.br
			SmPointer manager_data;
.br
		} save_yourself_done;
.sp 0.5
		struct {
.br
			SmsCloseConnectionProc callback;
.br
			SmPointer manager_data;
.br
		} close_connection;
.sp 0.5
		struct {
.br
			SmsSetPropertiesProc callback;
.br
			SmPointer manager_data;
.br
		} set_properties;
.sp 0.5
		struct {
.br
			SmsDeletePropertiesProc callback;
.br
			SmPointer manager_data;
.br
		} delete_properties;
.sp 0.5
		struct {
.br
			SmsGetPropertiesProc callback;
.br
			SmPointer manager_data;
.br
		} get_properties;
.sp 0.5
	} SmsCallbacks;
.LP
.eM
.NH 3
The Register Client Callback
.XS
\*(SN The Register Client Callback
.XE
.LP
The \fIRegister Client\fP\^ callback is the first callback that will be
invoked after the client connects to the session manager.  Its type is
.PN SmsRegisterClientProc .
.LP
.sM
typedef Status (*SmsRegisterClientProc();
.LP
.FD 0
Status RegisterClientProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fIprevious_id\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    char *\fIprevious_id\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fIprevious_id\fP 1i
The client ID from the previous session.
.LP
.eM
Before any further interaction takes place with the client,
the client must be registered with the session manager.
.LP
If the client is being restarted from a previous session,
\fIprevious_id\fP\^ will contain a null terminated string representing
the client ID from the previous session.  Call free() on the
\fIprevious_id\fP\^ pointer when it is no longer needed.
If the client is first joining the session, \fIprevious_id\fP\^ will be
.PN NULL .
.LP
If \fIprevious_id\fP\^ is invalid, the session manager should not register
the client at this time.  This callback should return a status of 0, which
will cause an error message to be sent to the client.  The client should
re-register with \fIprevious_id\fP\^ set to
.PN NULL .
.LP
Otherwise, the session manager should register the client with a
unique client ID by calling the
.PN SmsRegisterClientReply
function (to be discussed shortly), and the
.PN SmsRegisterClientProc
callback should return a status of 1.
.LP
.NH 3
The Interact Request Callback
.XS
\*(SN The Interact Request Callback
.XE
.LP
The \fIInteract Request\fP\^ callback is of type
.PN SmsInteractRequestProc .
.LP
.sM
typedef void (*SmsInteractRequestProc)();
.LP
.FD 0
void InteractRequestProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fIdialog_type\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    int \fIdialog_type\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fIdialog_type\fP 1i
The type of dialog the client wishes to present to the user.
.LP
.eM	  
When a client receives a \fISave Yourself\fP\^ message with an
\fIinteract_style\fP\^ of
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny ,
the client may choose to interact with the user.
Since only one client can interact with the user at a time, the client
must request to interact with the user.  The session manager should keep
a queue of all clients wishing to interact.  It should send an \fIInteract\fP\^
message to one client at a time and wait for an \fIInteract Done\fP\^ message
before continuing with the next client.
.LP
The \fIdialog_type\fP argument specifies either
.PN SmDialogError
indicating that the client wants to start an error dialog,
or
.PN SmDialogNormal ,
meaning that the client wishes to start a non-error dialog.
.LP
If a shutdown is in progress, the user may have the option of cancelling
the shutdown.  If the shutdown is cancelled (specified in the \fIInteract
Done\fP\^ message), the session manager should send a
\fIShutdown Cancelled\fP\^ message to each client that requested to interact.
.LP
.NH 3
The Interact Done Callback
.XS
\*(SN The Interact Done Callback
.XE
.LP
When the client is done interacting with the user, the
.PN SmsInteractDoneProc
callback will be invoked.
.LP
.sM
typedef void (*SmsInteractDoneProc)();
.LP
.FD 0
void InteractDoneProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fIcancel_shutdown\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    Bool \fIcancel_shutdown\fP\^;
.FN
.IP \fIsms_conn\fP 1.1i
The session management connection object.
.IP \fImanager_data\fP 1.1i
Manager data specified when the callback was registered.
.IP \fIcancel_shutdown\fP 1.1i
Specifies if the user requests that the entire shutdown be cancelled.
.LP
.eM
Note that the shutdown can be cancelled only if the corresponding
\fISave Yourself\fP specified
.PN True
for \fIshutdown\fP\^ and
.PN SmInteractStyleErrors
or
.PN SmInteractStyleAny
for the \fIinteract_style\fP\^.
.LP
.bp
.NH 3
The Save Yourself Request Callback
.XS
\*(SN The Save Yourself Request Callback
.XE
.LP
The \fISave Yourself Request\fP\^ callback is of type
.PN SmsSaveYourselfRequestProc .
.LP
.sM
typedef void (*SmsSaveYourselfRequestProc)();
.LP
.FD 0
void SaveYourselfRequestProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fIsave_type\fP\^, \fIshutdown\fP\^, \fIinteract_style\fP\^, \fIfast\fP\^, \fIglobal\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    int \fIsave_type\fP\^;
.br
    Bool \fIshutdown\fP\^;
.br
    int \fIinteract_style\fP\^;
.br
    Bool \fIfast\fP\^;
.br
    Bool \fIglobal\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fIsave_type\fP 1i
Specifies the type of information that should be saved.
.IP \fIshutdown\fP 1i
Specifies if a shutdown is taking place.
.IP \fIinteract_style\fP 1i
The type of interaction allowed with the user.
.IP \fIfast\fP 1i
If
.PN True ,
the client should save its state as quickly as possible.
.IP \fIglobal\fP 1i
Controls who gets the \fISave Yourself\fP\^.
.LP
.eM
The \fISave Yourself Request\fP\^ prompts the session manager to
initiate a checkpoint or shutdown.
The \fIsave_type\fP\^, \fIshutdown\fP\^, \fIinteract_style\fP\^, and
\fIfast\fP\^ parameters are discussed in the upcoming section titled
\fISending a Save Yourself Message\fP\^.
.LP
If \fIglobal\fP\^ is set to
.PN True ,
then the resulting \fISave Yourself\fP\^ should be
sent to all applications.  If \fIglobal\fP\^ is set to
.PN False ,
then the \fISave Yourself\fP\^ should only be sent to the client which
requested the \fISave Yourself\fP\^.
.LP
.NH 3
The Save Yourself Phase 2 Request Callback
.XS
\*(SN The Save Yourself Phase 2 Request Callback
.XE
.LP
The \fISave Yourself Phase 2 Request\fP\^ callback is of type
.PN SmsSaveYourselfPhase2RequestProc .
.LP
.sM
typedef void (*SmsSaveYourselfPhase2RequestProc)();
.LP
.FD 0
void SmsSaveYourselfPhase2RequestProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.LP
.eM
This request is sent by clients that manage other clients (e.g. window
managers, workspace managers, etc.).  Such managers must make sure that all
of the clients that are being managed are in an idle state, so that their
state can be saved.
.LP
.NH 3
The Save Yourself Done Callback
.XS
\*(SN The Save Yourself Done Callback
.XE
.LP
When the client is done saving its state in response to a
\fISave Yourself\fP\^ message, the
.PN SmsSaveYourselfDoneProc
will be invoked.
.LP
.sM
typedef void (*SmsSaveYourselfDoneProc)();
.LP
.FD 0
void SaveYourselfDoneProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fIsuccess\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    Bool \fIsuccess\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fIsuccess\fP 1i
If
.PN True ,
the \fISave Yourself\fP\^ operation was completed successfully.
.LP
.eM
Before the \fISave Yourself Done\fP\^ was sent, the client must have
set each required property at least once since it registered with the
session manager.
.LP
.NH 3
The Connection Closed Callback
.XS
\*(SN The Connection Closed Callback
.XE
.LP
If the client properly terminates (i.e. it calls
.PN SmcCloseConnection ),
the
.PN SmsCloseConnectionProc
callback is invoked.
.LP
.sM
typedef void (*SmsCloseConnectionProc)();
.LP
.FD 0
void CloseConnectionProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fIcount\fP\^, \fIreason_msgs\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    int \fIcount\fP\^;
.br
    char **\fIreason_msgs\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fIcount\fP 1i
The number of reason messages.
.IP \fIreason_msgs\fP 1i
The reasons for closing the connection.
.LP
.eM
\fIreason_msgs\fP\^ will most likely be
.PN NULL
and count 0
if resignation is expected by the user.  Otherwise, it contains a list
of null terminated Compound Text strings representing the reason for
termination.  The session manager should display these reason messages
to the user.
.LP
Call
.PN SmFreeReasons
to free the reason messages.  See the section titled \fIFreeing Data\fP\^.
.LP
.NH 3
The Set Properties Callback
.XS
\*(SN The Set Properties Callback
.XE
.LP
When the client sets session management properties, the
.PN SmsSetPropertiesProc
callback will be invoked.
.LP
.sM
typedef void (*SmsSetPropertiesProc)();
.LP
.FD 0
void SetPropertiesProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fInum_props\fP\^, \fIprops\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    int \fInum_props\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.bp
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fInum_props\fP 1i
The number of properties.
.IP \fIprops\fP 1i
The list of properties to set.
.LP
.eM
The properties are specified as an array of property pointers.
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled
\fISession Management Properties\fP\^.
.LP
Previously set property values may be over-written.  Some properties
have predefined semantics.
The session manager is required to store
non-predefined properties.
.LP
In order to free each property, call the
.PN SmFreeProperty
function.  See the section titled \fIFreeing Data\fP\^.
Free the actual array of pointers with a call to free().
.LP
.NH 3
The Delete Properties Callback
.XS
\*(SN The Delete Properties Callback
.XE
.LP
When the client deletes session management properties, the
.PN SmsDeletePropertiesProc
callback will be invoked.
.LP
.sM
typedef void (*SmsDeletePropertiesProc)();
.LP
.FD 0
void DeletePropertiesProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^, \fInum_props\fP\^, \fIprop_names\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.br
    int \fInum_props\fP\^;
.br
    char **\fIprop_names\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.IP \fInum_props\fP 1i
The number of properties.
.IP \fIprop_names\fP 1i
The list of properties to delete.
.LP
.eM
The properties are specified as an array of strings.
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled
\fISession Management Properties\fP\^.
.LP
.NH 3
The Get Properties Callback
.XS
\*(SN The Get Properties Callback
.XE
.LP
The
.PN SmsGetPropertiesProc
callback is invoked when the client wants to retrieve properties it set.
.LP
.sM
typedef void (*SmsGetPropertiesProc)();
.LP
.FD 0
void GetPropertiesProc\^(\^\fIsms_conn\fP, \fImanager_data\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    SmPointer \fImanager_data\fP\^;
.FN
.IP \fIsmc_conn\fP 1i
The session management connection object.
.IP \fImanager_data\fP 1i
Manager data specified when the callback was registered.
.LP
.eM
The session manager should respond by calling
.PN SmsReturnProperties .
All of the properties set for this client should be returned.
.LP
.NH 2
Registering the Client
.XS
\*(SN Registering the Client
.XE
.LP
In order to register a client (in response to a
.PN SmsRegisterClientProc
callback), call the
.PN SmsRegisterClientReply
function.
.LP
.sM
.FD 0
Status SmsRegisterClientReply\^(\^\fIsms_conn\fP, \fIclient_id\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    char *\fIclient_id\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fIclient_id\fP 1i
A null terminated string representing a unique client ID.
.LP
.eM
The return value of
.PN SmsRegisterClientReply
is zero for failure, and a positive value for success.  Failure will
occur if SMlib can not allocate memory to hold a copy of the client ID
for it's own internal needs.
.LP
If a non-NULL \fIprevious_id\fP\^ was specified when the client registered
itself, \fIclient_id\fP\^ should be identical to \fIprevious_id\fP\^.
.LP
Otherwise, \fIclient_id\fP\^ should be a unique ID freshly generated by
the session manager.  In addition, the session manager should send
a \fISave Yourself\fP\^ message with type = Local, shutdown = False,
interact-style = None, and fast = False immediately after registering the
client.
.LP
Note that once a client ID has been assigned to the client, the client keeps
this ID indefinitely.  If the client is terminated and restarted, it
will be reassigned the same ID.  It is desirable to be able to pass
client IDs around from machine to machine, from user to user, and
from session manager to session manager, while retaining the
identity of the client.  This, combined with the indefinite
persistence of client IDs, means that client IDs need to be globally
unique.
.LP
Call the
.PN SmsGenerateClientID
function to generate a globally unique client ID.
.LP
.sp 0.5
.sM
.FD 0
char *SmsGenerateClientID\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
.PN NULL
will be returned if the ID could not be generated.  Otherwise, the return
value of the function is the client ID.  It should be freed with a call to
free() when no longer needed.
.LP
.NH 2
Sending a Save Yourself Message
.XS
\*(SN Sending a Save Yourself Message
.XE
.LP
In order to send a \fISave Yourself\fP\^ to a client, call the
.PN SmsSaveYourself
function.
.LP
.sM
.FD 0
void SmsSaveYourself\^(\^\fIsms_conn\fP, \fIsave_type\fP\^, \fIshutdown\fP\^, \fIinteract_style\fP\^, \fIfast\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    int \fIsave_type\fP\^;
.br
    Bool \fIshutdown\fP\^;
.br
    int \fIinteract_style\fP\^;
.br
    Bool \fIfast\fP\^;
.FN
.ne 7
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fIsave_type\fP 1i
Specifies the type of information that should be saved.
.IP \fIshutdown\fP 1i
Specifies if a shutdown is taking place.
.IP \fIinteract_style\fP 1i
The type of interaction allowed with the user.
.IP \fIfast\fP 1i
If
.PN True ,
the client should save its state as quickly as possible.
.LP
.eM
The session manager sends a \fISave Yourself\fP\^ message to a client 
either to checkpoint it or just before
termination so that it can save its state.
The client responds with zero or more \fISet Properties\fP\^ messages
to update the properties indicating how to restart the client.
When all the properties have been set, the client sends a
\fISave Yourself Done\fP\^ message.
.LP
If \fIinteract_style\fP\^ is
.PN SmInteractStyleNone ,
the client must not interact with the
user while saving state.
If \fIinteract_style\fP\^ is 
.PN SmInteractStyleErrors ,
the client may interact with the user only if an error condition arises.  If
\fIinteract_style\fP\^ is 
.PN SmInteractStyleAny ,
then the client may interact with the user for any purpose.
The client must send an \fIInteract Request\fP\^ message
and wait for an \fIInteract\fP\^ message from the session manager
before it can interact with the user.  When the client is done
interacting with the user, it should send an \fIInteract Done\fP\^ message.
The \fIInteract Request\fP\^ message can be sent any time after a
\fISave Yourself\fP\^ and before a \fISave Yourself Done\fP\^.
.LP
If \fIsave_type\fP\^ is
.PN SmSaveLocal ,
the client must update the properties to reflect its current state.
Specifically, it should save enough information to restore
the state as seen by the user of this client.  It should not affect the
state as seen by other users.  If \fIsave_type\fP\^ is
.PN SmSaveGlobal
the user wants the client to commit all of its data to permanent,
globally accessible storage.  If \fIsave_type\fP\^ is
.PN SmSaveBoth ,
the client should do both of these (it should first commit the data to
permanent storage before updating its properties).
.LP
.sp 0.5
The \fIshutdown\fP\^ parameter specifies whether the session is being
shut down.  The interaction is different depending on whether or not
\fIshutdown\fP\^ is set.  If not shutting down, then the client can save and
resume normal operation.  If shutting down, the client must save and
then must prevent interaction until it receives either a \fIDie\fP\^
or a \fP\^Shutdown Cancelled\fP\^, because anything the user does after
the save will be lost.
.LP
The \fIfast\fP\^ parameter specifies that the client should save its state
as quickly as possible.  For example, if the session manager knows that
power is about to fail, it should set \fIfast\fP\^ to
.PN True .
.LP
.NH 2
Sending a Save Yourself Phase 2 Message
.XS
\*(SN Sending a Save Yourself Phase 2 Message
.XE
.LP
In order to send a \fISave Yourself Phase 2\fP\^ message to a client, call the
.PN SmsSaveYourselfPhase2
function.
.LP
.sM
.FD 0
void SmsSaveYourselfPhase2\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
The session manager sends this message to a client that has previously sent a
\fISave Yourself Phase 2 Request\fP\^ message.  This message informs the
client that all other clients are in a fixed state and this client can save
state that is associated with other clients.
.LP
.NH 2
Sending an Interact Message
.XS
\*(SN Sending an Interact Message
.XE
.LP
To send an \fIInteract\fP\^ message to a client, call the
.PN SmsInteract
function.
.ne 8
.sM
.FD 0
void SmsInteract\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
The \fIInteract\fP\^ message grants the client the privilege of interacting
with the user.  When the client is done interacting with the user, it must
send an \fIInteract Done\fP\^ message to the session manager.
.LP
.NH 2
Sending a Save Complete Message
.XS
\*(SN Sending a Save Complete  Message
.XE
.LP
To send a \fISave Complete\fP\^ message to a client, call the
.PN SmsSaveComplete
function.
.LP
.ne 8
.sM
.FD 0
void SmsSaveComplete\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
The session manager sends this message when it is done with a checkpoint.
The client is then free to change its state.
.LP
.NH 2
Sending a Die Message
.XS
\*(SN Sending a Die Message
.XE
.LP
To send a \fIDie\fP\^ message to a client, call the
.PN SmsDie
function.
.LP
.sM
.FD 0
void SmsDie\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
Before the session manager terminates, it should wait for a
\fIConnection Closed\fP\^ message from each client that it sent
a \fIDie\fP\^ message to, timing out appropriately.
.LP
.NH 2
Cancelling a Shutdown
.XS
\*(SN Cancelling a Shutdown
.XE
.LP
To cancel a shutdown, call the
.PN SmsShutdownCancelled
function.
.LP
.sM
.FD 0
void SmsShutdownCancelled\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
The client can now continue as if the shutdown had never happened.
If the client has not sent a \fISave Yourself Done\fP\^ yet, it can
either abort the save and send a \fISave Yourself Done\fP\^ 
with the \fIsuccess\fP\^ field set to
.PN False ,
or it can continue with the save and send a \fISave Yourself Done\fP\^ 
with the \fIsuccess\fP\^ field set to reflect the outcome of the save.
.LP
.NH 2
Returning Properties
.XS
\*(SN Returning Properties
.XE
.LP
In response to a \fIGet Properties\fP\^ message, the session manager should
call the
.PN SmsReturnProperties
function.
.LP
.sM
.FD 0
void SmsReturnProperties\^(\^\fIsms_conn\fP\^, \fInum_props\fP\^, \fIprops\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.br
    int \fInum_props\fP\^;
.br
    SmProp **\fIprops\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fInum_props\fP 1i
The number of properties.
.IP \fIprops\fP 1i
The list of properties to return to the client.
.LP
.eM
The properties are returned as an array of property pointers.
For a description of session management properties and the
.PN SmProp
structure, refer to the section titled
\fISession Management Properties\fP\^.
.LP
.NH 2
Pinging a Client
.XS
\*(SN Pinging a Client
.XE
.LP
In order to check that a client is still alive, use the
.PN IcePing
function provided by the ICE library.  In order to do so, the ICE
connection must be obtained using the
.PN SmsGetIceConnection
discussed in a later section.
.LP
.sM
.FD 0
void IcePing\^(\^\fIice_conn\fP, \fIping_reply_proc\fP\^, \fIclient_data\fP\^)
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
When the Ping reply is ready (if ever), the
.PN IcePingReplyProc
callback will be invoked.  A session manager should have some sort
of timeout period, after which it assumes the client has unexpectedly died.
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
.LP
.NH 2
Cleaning Up After a Client Disconnects
.XS
\*(SN Cleaning Up After a Client Disconnects
.XE
.LP
When the session manager receives a \fIConnection Closed\fP message or
otherwise detects that the client aborted the connection, it should
call the
.PN SmsCleanUp
function in order to free up the connection object.
.LP
.sM
.FD 0
void SmsCleanUp\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.IP \fIsms_conn\fP 1i
The session management connection object.
.LP
.eM
.LP
.NH 2
Informational Functions
.XS
\*(SN Informational Functions
.XE
.LP
.sM
.FD 0
int SmsProtocolVersion\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.eM
Returns the major version of the session management protocol
associated with this session.
.LP
.sp 0.5
.sM
.FD 0
int SmsProtocolRevision\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.eM
Returns the minor version of the session management protocol
associated with this session.
.LP
.sp 0.5
.LP
.sp 0.5
.sM
.FD 0
char *SmsClientID\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.eM
Returns a null terminated string for the client Id associated with
this connection.
.LP
Call free() on this pointer when the client Id is no longer needed.
.LP
.sp 0.5
To obtain the host name of a client, call the
.PN SmsClientHostName
function.  This host name will be needed to restart the client.
.LP
.sM
.FD 0
char *SmsClientHostName\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.eM
The string returned is of the form "protocol/hostname", where
\fIprotocol\fP\^ is one of {tcp, decnet, local}.
.LP
Call free() on the string returned when it is no longer needed.
.LP
.sp 0.5
.sM
.FD 0
IceConn SmsGetIceConnection\^(\^\fIsms_conn\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
.FN
.eM
Returns the ICE connection object associated with this session management
connection object.  The ICE connection object can be used to get some
additional information about the connection.  Some of the more useful
functions which can be used on the IceConn are IceConnectionNumber,
and IceLastSequenceNumber.  Check the \fIInter-Client Exchange Library\fP\^
document for more details on these functions.
.LP
.NH 2
Error Handling
.XS
\*(SN Error Handling
.XE
.LP
If the session manager receives an unexpected protocol error from a client,
an error handler is invoked by SMlib.  A default error handler exists which
simply prints the error message (it does not exit).  The session manager can
change this error handler by calling the
.PN SmsSetErrorHandler
function.
.LP
.sM
.FD 0
SmsErrorHandler SmsSetErrorHandler\^(\^\fIhandler\fP\^)
.br
    SmsErrorHandler \fIhandler\fP\^;
.FN
.IP \fIhandler\fP 1i
The error handler.  Pass
.PN NULL
to restore the default handler.
.LP
.eM
.PN SmsSetErrorHandler
returns the previous error handler.
.LP
The
.PN SmsErrorHandler
has the following type:
.LP
.sp 0.5
.sM
typedef void (*SmsErrorHandler)();
.br
.FD 0
void ErrorHandler\^(\^\fIsms_conn\fP, \fIswap\fP\^, \fIoffending_minor_opcode\fP\^, \fIoffending_sequence_num\fP\^, \fIerror_class\fP\^, \fIseverity\fP\^, \fIvalues\fP\^)
.br
    SmsConn \fIsms_conn\fP\^;
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
.IP \fIsms_conn\fP 1i
The session management connection object.
.IP \fIswap\fP 1i
A flag which indicates if the \fIvalues\fP\^ need byte swapping.
.IP \fIoffending_minor_opcode\fP 1i
The minor opcode of the offending message.
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
Note that this error handler is invoked for protocol related errors.
To install an error handler to be invoked when an IO error occurs, use
the
.PN IceSetIOErrorHandler
function described in the \fIInter-Client Exchange Library\fP\^ document.
.LP
.bp
.NH 1
Session Management Properties
.XS
\*(SN Session Management Properties
.XE
.LP
Each property is defined by the
.PN SmProc
structure:
.LP
	typedef struct {
.br
		char *name;			/* name of property */
.br
		char *type;			/* type of property */
.br
		int num_vals;		/* number of values */
.br
		SmPropValue *vals;	/* the list of values */
.br
	} SmProp;
.br
.LP	
	typedef struct {
.br
		int length;			/* the length of the value */
.br
		SmPointer value;		/* the value */
.br
	} SmPropValue;
.LP
.sp 0.5
The X Session Management Protocol defines a list of predefined properties,
several of which are required to be set by the client.  Below is a table
which specifies the predefined properties, indicating which ones are required.
Each property has a type associated with it.
.LP
A type of
.PN SmCARD8
indicates that there is a single 1 byte value.
.LP
A type of
.PN SmARRAY8
indicates that there is a single array of bytes.
.LP
A type of
.PN SmLISTofARRAY8
indicates that there is a list of array of bytes.
.LP
.TS H
l l l c .
_
.sp 6p
.B
Name	Type	POSIX Type	Required?
.R
.sp 6p
_
.sp 6p
.TH
SmCloneCommand	OS-specific	SmLISTofARRAY8	Yes
SmCurrentDirectory	OS-specific	SmARRAY8	No
SmDiscardCommand	OS-specific	SmLISTofARRAY8	No*
SmEnvironment	OS-specific	SmLISTofARRAY8	No
SmProcessID	OS-specific	SmARRAY8	No
SmProgram	OS-specific	SmARRAY8	Yes
SmRestartCommand	OS-specific	SmLISTofARRAY8	Yes
SmResignCommand	OS-specific	SmLISTofARRAY8	No
SmRestartStyleHint	SmCARD8	SmCARD8	No
SmShutdownCommand	OS-specific	SmLISTofARRAY8	No
SmUserID	SmARRAY8	SmARRAY8	Yes
.sp 6p
_
.TE
.LP
* Required if any state is stored in an external repository (e.g., state file).
.LP
.IP SmCloneCommand 3
This is like the 
.PN SmRestartCommand 
except it restarts a copy of the
application.  The only difference is that the application doesn't
supply its client id at register time.  On POSIX systems this should
be of type
.PN SmLISTofARRAY8 .
.IP SmCurrentDirectory 3
On POSIX-based systems specifies the value of the current directory that
needs to be set up prior to starting the
.PN SmProgram
and should of type
.PN SmARRAY8 .
.IP SmDiscardCommand 3
The discard command contains a command that when delivered to the host that 
the client is running on (determined from the connection), will
cause it to discard any information about the current state.  If this command
is not specified, the SM will assume that all of the client's state is encoded
in the 
.PN SmRestartCommand.
On POSIX systems the type should be
.PN SmLISTofARRAY8 .
.IP SmEnvironment 3
On POSIX based systems, this will be of type
.PN SmLISTofARRAY8
where the ARRAY8s alternate between environment variable name and environment
variable value.  
.IP SmProcessID 3
This specifies an OS-specific identifier for the process.  On POSIX
systems this should contain the return value of getpid() turned into
a Latin-1 (decimal) string.
.IP SmProgram 3
The name of the program that is running.  On POSIX systems this should be 
first parameter passed to execve and should be of type
.PN SmARRAY8 .
.IP SmRestartCommand 3
The restart command contains a command that when delivered to the
host that the client is running on (determined from the connection),
will cause the client to restart in
its current state.  On POSIX-based systems this is of type
.PN SmLISTofARRAY8
and each of the elements in the array represents an element in
the argv array.
This restart command should ensure that the client restarts with the specified
client-ID.
.IP SmResignCommand 3
A client that sets the
.PN SmRestartStyleHint
to
.PN SmRestartAnway
uses this property to specify a command 
that undoes the effect of the client and removes
any saved state.
As an example, consider a user that runs xmodmap.
xmodmap registers with the SM, sets 
.PN SmRestartStyleHint
to 
.PN SmRestartAnyway,
and then terminates.  In order to allow the SM (at the
user's request) to undo this, xmodmap would register a
.PN SmResignCommand
that undoes the effects of the xmodmap.
.IP SmRestartStyleHint 3
.RS
.LP
If the RestartStyleHint property is present, it will contain the 
style of restarting the client prefers.  If this flag isn't specified,
.PN SmRestartIfRunning
is assumed.
The possible values are as follows:
.TS H
l n.
_
.sp 6p
.B
Name	Value
.sp 6p
_
.sp 6p
.TH
.R
SmRestartIfRunning	0
SmRestartAnyway	1
SmRestartImmediately	2
SmRestartNever	3
.sp 6p
_
.TE
.LP
The
.PN SmRestartIfRunning
style is used in the usual case.  The client should
be restarted in the next session if it was running at the end of the
current session.
.LP
The
.PN SmRestartAnyway
style is used to tell the SM that the application
should be restarted in the next session even if it exits before the 
current session is terminated.  It should be noted that this is only
a hint and the SM will follow the policies specified by its users in
determining what applications to restart.
.LP
A client that uses
.PN SmRestartAnyway
should also set the
.PN SmResignCommand
and
.PN SmShutdownCommand
properties to commands that undo the state of the client
after it exits.
.LP
The
.PN SmRestartImmediately
style is like
.PN SmRestartAnyway ,
but in addition, the
client is meant to run continuously.  If the client exits, the
SM should try to restart it in the current session.
.LP
.PN SmRestartNever
style specifies that the client 
does not wish to be restarted in the next session.
.RE
.IP SmShutdownCommand
This command is executed at shutdown time to clean up after a client that
is no longer running but retained its state by setting
.PN SmRestartStyleHint
to 
.PN SmRestartAnyway.
The client must not remove any saved state as the client is still part of
the session.  As an example, consider a client that turns on a camera
at start up time.  This client then
exits.  At session shutdown, the user wants the camera turned off. This client
would set the 
.PN SmRestartStyleHint
to 
.PN SmRestartAnyway
and would register a 
.PN SmShutdownCommand
that would turn off the camera.
.IP SmUserID 3
Specifies the user's ID.  On POSIX-based systems this
will contain the user's name (the pw_name field of struct passwd).
.LP
.NH 1
Freeing Data
.XS
\*(SN Freeing Data
.XE
.LP
To free an individual property, call the
.PN SmFreeProperty
function.
.LP
.sM
.FD 0
void SmFreeProperty\^(\^\fIprop\fP\^)
.br
    SmProp *\fIprop\fP\^;
.FN
.IP \fIprop\fP 1i
The property to free.
.LP
.eM
.LP
To free the reason strings from the
.PN SmsCloseConnectionProc
callback, call the
.PN SmFreeReasons
function.
.LP
.sM
.FD 0
void SmFreeReasons\^(\^\fIcount\fP, \fIreasons\fP\^)
.br
    int \fIcount\fP\^;
.br
    char **\fIreasons\fP\^;
.FN
.IP \fIcount\fP 1i
The number of reason strings.
.IP \fIreasons\fP 1i
The list of reason strings to free.
.LP
.eM
.LP
.NH 1
Authentication of Clients
.XS
\*(SN Authentication of Clients
.XE
.LP
As you will recall, the session management protocol is layered on top
of ICE.  Authentication occurs at two levels in the ICE protocol.  The first is
when an ICE connection is opened.  The second is when a \fIProtocol
Setup\fP\^ occurs on an ICE connection.  The authentication methods that
are available are implementation dependent (i.e., dependent on the ICElib
and SMlib implementations in use).  Refer to the \fIInter-Client Exchange
Library\fP\^ for a general discussion on authentication.
.LP
.NH 1
Working in a Multi-Threaded Environment
.XS
\*(SN Working in a Multi-Threaded Environment
.XE
.LP
To declare that multiple threads in an application will be using SMlib
(or any other library layered on top of ICElib), the
.PN IceInitThreads
function should be called.  This function, along with a general discussion
of multi-threading in the ICE environment, is discussed in the
\fIInter-Client Exchange Library\fP\^ document.
.LP
.EH ''''
.OH ''''
.YZ 3
