/* $XConsortium: rcrdtest.c /main/8 1996/01/31 20:12:48 dpw $ */

/*
		     RECORD test client strategy
			   David P. Wiggins
			       8/16/95

This document specifies the tests that should be performed for each request
of the RECORD extension.  You may wish to view this using emacs' outline
mode.

* QueryVersion

Verify that the returned version == expected version.

* CreateContext

** Exercise error conditions

*** Match error

The following actions should cause a Match error.

Specify an invalid CLIENTSPEC.

*** Value error

The following actions should cause a Value error.

Specify core-requests with first > last
Specify core-replies with first > last
Specify ext-requests with ext_minor.first > ext_minor.last
Specify ext-requests with ext_major.first > ext_major.last (not in spec!)
Specify ext-requests with ext_major.first between 1 and 127 inclusive
Specify ext-requests with ext_major.last between 1 and 127 inclusive
Specify ext-replies with ext_minor.first > ext_minor.last
Specify ext-replies with ext_major.first > ext_major.last (not in spec!)
Specify ext-replies with ext_major.first between 1 and 127 inclusive
Specify ext-replies with ext_major.last between 1 and 127 inclusive
Specify delivered-events with first == 1
Specify delivered-events with last == 1
Specify delivered-events with first > last
Specify device-events with first == 1
Specify device-events with last == 1
Specify device-events with first > last
Specify errors with first > last

*** IDChoice error

Specify an invalid context ID.  (May not be possible from the
documented library interface.)

** Basic CreateContext functionality

None of the following actions should result in an error.  In each
case, the context should be verified by a GetContext.  Some of the
contexts should be freed by FreeContext.  Others should be left alone,
to be cleaned up when the client closes its connection.  This way we test
both destruction paths.

*** Empty ranges list

Create a context with an empty client-specifiers list and an empty
ranges list.

Create a context with a client-specifiers list containing
CurrentClients and an empty ranges list.

Create a context with a client-specifiers list containing
FutureClients and an empty ranges list.

Create a context with a client-specifiers list containing
AllClients and an empty ranges list.

Create a context with a client-specifiers list containing
the XIDBASE of an existing client and an empty ranges list.

*** Non-empty ranges list

Create a context with an empty client-specifiers list and an
non-empty, valid ranges list.

Create a context with a client-specifiers list containing
CurrentClients and an non-empty, valid ranges list.

Create a context with a client-specifiers list containing
FutureClients and an non-empty, valid ranges list.

Create a context with a client-specifiers list containing
AllClients and an non-empty, valid ranges list.

Create a context with a client-specifiers list containing the XIDBASE
of an existing client and an non-empty, valid ranges list.

*** Multiple clients 

Create a context with a client-specifiers list containing
FutureClients and the XIDBASE of an existing client and an non-empty,
valid ranges list.

Create a context with a client-specifiers list containing the XIDBASE
of two existing client and an non-empty, valid ranges list.

* RegisterClients

RegisterClients on an invalid context.  Verify that a RecordContext error
occurs.

We assume that the SI has good factorization, and that most of the
error conditions have already been tested by CreateContext.  They
won't be retested here.  As before, after every RegisterClients
request, the context should be verified with GetContext.

Create a context with no clients or protocol registered for use in
subsequent tests.  Get and verify the context.

Register a valid client with non-empty, valid ranges.

Specify an invalid CLIENTSPEC and verify that a Match error occurs.

Register another valid client with non-empty, valid ranges.

Register a client that's already registered with the context with
non-empty, valid ranges.

Register a client that's already registered with the context with
an empty ranges list.

Register CurrentClients with non-empty, valid ranges.

Register AllClients with non-empty, valid ranges.

Register FutureClients with non-empty, valid ranges.

Create a new context and enable it.  Subsequent tests will use this context.

Register a client on the context.

A: Have the newly registered client generate protocol that is selected by
the context.  Verify that the protocol comes back on the data
connection.  Then have the client generate some protocol that isn't
selected, and verify that it doesn't come back on the data connection.

Re-register the same client with different protocol.  Repeat the test
labeled A.

Register FutureClients on the context.  Make a new client connection.
Verify that the client is registered.  Repeat the test labeled A.

* UnregisterClients

UnregisterClients on an invalid context.  Verify that a RecordContext
error occurs.

The implementation should agree with the following table.  The column at the
far left specifies the initial CLIENTSPEC that a context is created with.
The CLIENTSPEC passed to Unregister is listed across the top.  The table
values specify which clients should be left in the context.  0 is no
clients.  s is the specific client that was given.  Future is the symbolic
value FutureClients.  current is the resolved list of clients that were
connected when the context was created.

Unregister ->	All	Current		Future		specific client
Create with:	
AllClients	0	Future		current		current - s + Future
CurrentClients	0	0		current		current - s
FutureClients	0	Future		0		Future
specific client	0	0		s		0

* GetContext

GetContext on an invalid context.  Verify that a RecordContext error
occurs.

GetContext is heavily used in other parts of the test program, so it
does not need further testing here.

* EnableContextAsync

Create a context.  Enable it on two different connections.  Verify that
a Match error occurs on the second enable.

Test all 8 combinations of flags in ELEMENT_HEADERS.  In each case,
verify that the correct header information is returned on the data
connection.

Verify that all message types (requests, replies, events, errors,
started, died) are recordable.
[this test still incompletely implemented]

Record a ClientDied message both with and without XRecordFromClientSequence
(not sure how to test, but will increase code coverage).

In each of the following cases, verify that the expected protocol is recorded
for each client, and that no unexpected protocol appears.

** multiple clients registered on a single context
*** with same protocol set
*** with different protocol set

** single clients registered on multiple contexts
*** with same protocol set
*** with different protocol set

** multiple clients registered on multiple contexts
*** with same protocol set
*** with different protocol set

** BIG-REQUESTS
Test that a big request is recorded correctly.  (Test not implemented.)


* EnableContext

Set an alarm.  Enable a context synchronously.  In the alarm handler,
generate some recordable protocol and disable the context.  Verify
that the expected data are recorded.

* DisableContext - not implemented

DisableContext on an invalid context.  Verify that a RecordContext error
occurs.

Disable a context that isn't enabled.  Verify that nothing happens.

Disable a context.  Verify that an EndOfData message is the last reply
that comes across the data connection.

Disable a context.  Generate protocol for a client that was being
recorded by the context, and verify that no further messages appear on
the data connection.

* FreeContext

FreeContext on an invalid context.  Verify that a RecordContext error
occurs.

FreeContext on an enabled context.  Verify that it is disabled.

FreeContext is heavily used in other parts of the test program, so it
does not need further testing here.

* XRecordFreeData

Test that you can free the data after closing the display
without creating leaks or referencing freed memory.
This test also increases code coverage.

*/

#include <stdio.h>
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/extensions/record.h>
#include <X11/extensions/recordstr.h>
#include <X11/extensions/XTest.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

/* globals */

int total_error_count = 0; /* total # of errors during entire run */
/* A testset is a group of logically related tests, e.g., all the tests
 * for a certain protocol request.
 */
char *testsetname;	/* string name of current testset */
int errs_this_testset;	/* number of errors that have occured this testset */
/* A test tries to validate a specific statement in the spec.  Roughly
 * equivalent to an IC (invocable component) in the TET framework.
 */
char *testname;		/* string name of current test */
int errs_this_test;	/* number of errors that have occured this test */
int total_tests = 0;	/* total number of tests executed */
int protocol_error;	/* error code of error caught by error handler */
int protocol_error_major;	/* major op code of error caught */
int protocol_error_minor;	/* minor op code of error caught */
int record_majorop;		/* opcode for RECORD */
int record_event, record_error;  /* event and error base for RECORD */
int BadContext;		/* record_error + XRecordBadContext */
XID client_mask;		/* resource id base mask */
int client_swap;	/* recorded data is swapped.  No way to set yet. */
int server_swap;		/* server is swapped.  A good guess. */
char *display_env = NULL;	/* -display argument */

/* some utilities for handling errors and demarcating start/end o
 * test modules
 */

/* Call this when an error occurs, passing a string that describes the error.*/

void
report_error(char *err_fmt, ...)
{
    va_list an;

    fprintf(stderr, "Error: %s %s: ", testsetname, testname);
    va_start(an, err_fmt);
    vfprintf(stderr, err_fmt, an);
    va_end(an);
    fprintf(stderr, "\n");
    fflush(stderr);
    errs_this_test++;
}

/* When you're not expecting a protocol error, make sure this is installed
 * via XSetErrorHandler.
 */
int
protocol_error_unexpected(dpy, errevent)
Display *dpy;
XErrorEvent *errevent;
{
    char buf[80];
    XGetErrorText(dpy, errevent->error_code, buf, sizeof (buf));
    report_error(buf);
    return 1;
}

/* When you *are* expecting a protocol error, make sure this is installed
 * via XSetErrorHandler.  You can check the global protocol_error to see
 * if the expected error actually occured.
 */
int
protocol_error_expected(dpy, errevent)
Display *dpy;
XErrorEvent *errevent;
{
    protocol_error = errevent->error_code;
    protocol_error_major = errevent->request_code;
    protocol_error_minor = errevent->minor_code;
    return 1;
}


/* Call this at the start of a test with a short string description of
 * the test.
 */
void
begin_test(char *t)
{
    testname = t;
    errs_this_test = 0;
    protocol_error = 0;
}

/* Call this at the end of a test. */
void end_test()
{
    if (errs_this_test)
    {
	printf("End test %s with %d errors\n\n", testname, errs_this_test);
	fflush(stdout);
    }
    errs_this_testset += errs_this_test;
    total_tests++;
}


/* Call this at the start of a testset with a short string description of the
 * testset.
 */
Display *
begin_test_set(tn)
char *tn;
{
    Display *dpy;
    union { int s; char c; } byte_order = { 1 };

    testsetname = tn;
    errs_this_testset = 0;
    printf("Start testset %s\n", testsetname);
    fflush(stdout);
    dpy = XOpenDisplay(display_env);
    if (!dpy)
    {
	report_error("Failed to open display\n");
	exit(1);
    }
    XSynchronize(dpy, True); /* so we get errors at convenient times */
    XSetErrorHandler(protocol_error_unexpected);
    if (!XQueryExtension(dpy, RECORD_NAME, &record_majorop, &record_event,
			 &record_error))
    {
	report_error("Failed to find RECORD extension");
	exit(1);
    }
    BadContext = record_error + XRecordBadContext;
    client_mask = XRecordIdBaseMask(dpy);
    switch (XImageByteOrder(dpy)) {
      case LSBFirst:
	server_swap = byte_order.c ? 0 : 1;
	break;
      case MSBFirst:
	server_swap = byte_order.c ? 1 : 0;
	break;
    }
    return dpy;
}

/* Call this at the end of a testset. */
void
end_test_set(dpy)
Display *dpy;
{
    printf("End testset %s with %d errors\n\n", testsetname, errs_this_testset);
    fflush(stdout);
    total_error_count += errs_this_testset;
    XCloseDisplay(dpy);
}

void test_query_version()
{
    Display *dpy = begin_test_set("QueryVersion");
    int major_val, minor_val;
    char buf[80];

    begin_test("check returned version number");

    if (!XRecordQueryVersion(dpy, &major_val, &minor_val))
    {
	report_error("Failed to initialize RECORD extension");
    }

    if (major_val < RECORD_LOWEST_MAJOR_VERSION ||
        major_val > RECORD_MAJOR_VERSION ||
        minor_val < RECORD_LOWEST_MINOR_VERSION ||
        minor_val > RECORD_MINOR_VERSION)
    {
	report_error("invalid version number");
    }
    end_test();

    printf("RECORD information \"%s\":\n", DisplayString(dpy));
    printf("  Major version:       %d\n", major_val);
    printf("  Minor version:       %d\n", minor_val);
    printf("  First event number:  %d\n", record_event);
    printf("  First error number:  %d\n", record_error);

    XGetErrorText(dpy, BadContext, buf, sizeof(buf));
    printf("  XRecordBadContext error string: %s\n", buf);
    end_test_set(dpy);
}

/* utility to clear out some XRecordRanges */
void null_record_ranges(XRecordRange **ranges, int nranges)
{
    int i;

    for (i=0; i<nranges; i++)
	memset(ranges[i], 0, sizeof(XRecordRange));
}

/* utility to free a context */
void free_context(Display *dpy, XRecordContext rc)
{
    if (!XRecordFreeContext(dpy, rc))
    {
	report_error("Failed to free context");
    }
}

#ifndef min
#define min(_a, _b) ( ((_a) < (_b)) ? (_a) : (_b) )
#endif

#define CLIENT_BITS(id) ((id) & client_mask)

/* What follows are some tools to compare lists of XRecordRanges to see if
 * they are equivalent.  This is fairly tricky, and I still don't cover
 * all the theoretically possible cases, but we can't spend forever on this.
 */

/* Each type of protocol message has a different bitmask.  This is used to
 * keep track of which protocol message types specified by one XRecordRange
 * have been found in another list of XRecordRanges.
 */
 
enum { FoundCoreRequests = 1, FoundCoreReplies = 2, FoundExtRequests = 4,
       FoundExtReplies = 8, FoundDeliveredEvents = 16, FoundDeviceEvents = 32,
       FoundErrors = 64, FoundClientStarted = 128, FoundClientDied = 256,
       FoundEverything = 511};

/* See if all the protocol specified by r1/nr1 is also specified by r2/nr2
 * (r2/nr2 subsumes r1/nr1).  This was easier to code than a direct
 * equality test.  Equality is checked by checking subsumption in both
 * directions.
 */
void check_range_subsumption(XRecordRange **r1, int nr1,
			     XRecordRange **r2, int nr2)
{
    unsigned int foundmask;	/* which elements have we found */
    int nr_min = min(nr1, nr2);
    int i, j;

    for (i = 0; i < nr_min; i++) /* step thru ranges */
    {
	/* see if we can find everything specified in the single
	 * XRecordRange r1[i] somewhere in the list r2.
	 */

	foundmask = 0; /* haven't found anything yet */

	/* For each protocol message type that is *not* specified in r1[i],
	 * consider that protocol message type "found."  This lets us
	 * simplify the test at the end to check for FoundEverything.
	 */

	if (r1[i]->core_requests.first == 0 && r1[i]->core_requests.last == 0)
	    foundmask |= FoundCoreRequests;
	if (r1[i]->core_replies.first == 0 && r1[i]->core_replies.last == 0)
	    foundmask |= FoundCoreReplies;
	if (r1[i]->ext_requests.ext_major.first == 0 &&
	    r1[i]->ext_requests.ext_major.last == 0)
	    foundmask |= FoundExtRequests;
	if (r1[i]->ext_replies.ext_major.first == 0 && r1[i]->ext_replies.ext_major.last == 0)
	    foundmask |= FoundExtReplies;
	if (r1[i]->delivered_events.first == 0 && r1[i]->delivered_events.last == 0)
	    foundmask |= FoundDeliveredEvents;
	if (r1[i]->device_events.first == 0 && r1[i]->device_events.last == 0)
	    foundmask |= FoundDeviceEvents;
	if (r1[i]->errors.first == 0 && r1[i]->errors.last == 0)
	    foundmask |= FoundErrors;
	if (r1[i]->client_started == False)
	    foundmask |= FoundClientStarted;
	if (r1[i]->client_died == False)
	    foundmask |= FoundClientDied;

	/* Now step thru the XRecordRanges in r2 and compare each field to
	 * the corresponding field in r1.  If they're the same, set the
	 * appropriate Found bit.
	 */
	for (j = 0; j < nr2; j++)
	{
	    if (r1[i]->core_requests.first == r2[j]->core_requests.first &&
		r1[i]->core_requests.last == r2[j]->core_requests.last)
		foundmask |= FoundCoreRequests;
	    if (r1[i]->core_replies.first == r2[j]->core_replies.first &&
		r1[i]->core_replies.last == r2[j]->core_replies.last)
		foundmask |= FoundCoreReplies;
	    if (r1[i]->ext_requests.ext_major.first == r2[j]->ext_requests.ext_major.first &&
		r1[i]->ext_requests.ext_major.last == r2[j]->ext_requests.ext_major.last &&
		r1[i]->ext_requests.ext_minor.first == r2[j]->ext_requests.ext_minor.first &&
		r1[i]->ext_requests.ext_minor.last == r2[j]->ext_requests.ext_minor.last)
		foundmask |= FoundExtRequests;
	    if (r1[i]->ext_replies.ext_major.first == r2[j]->ext_replies.ext_major.first &&
		r1[i]->ext_replies.ext_major.last == r2[j]->ext_replies.ext_major.last &&
		r1[i]->ext_replies.ext_minor.first == r2[j]->ext_replies.ext_minor.first &&
		r1[i]->ext_replies.ext_minor.last == r2[j]->ext_replies.ext_minor.last)
		foundmask |= FoundExtReplies;
	    if (r1[i]->delivered_events.first == r2[j]->delivered_events.first &&
		r1[i]->delivered_events.last == r2[j]->delivered_events.last)
		foundmask |= FoundDeliveredEvents;
	    if (r1[i]->device_events.first == r2[j]->device_events.first &&
		r1[i]->device_events.last == r2[j]->device_events.last)
		foundmask |= FoundDeviceEvents;
	    if (r1[i]->errors.first == r2[j]->errors.first &&
		r1[i]->errors.last == r2[j]->errors.last)
		foundmask |= FoundErrors;
	    if (r1[i]->client_started == True && r2[j]->client_started == True)
		foundmask |= FoundClientStarted;
	    if (r1[i]->client_died == True && r2[j]->client_died == True)
		foundmask |= FoundClientDied;
	}

	/* issue errors for anything that wasn't found. */

	if (foundmask != FoundEverything)
	{
	    if (!(foundmask & FoundCoreRequests))
		report_error("core requests don't match");
	    if (!(foundmask & FoundCoreReplies))
		report_error("core replies don't match");
	    if (!(foundmask & FoundExtRequests))
		report_error("extension requests don't match");
	    if (!(foundmask & FoundExtReplies))
		report_error("extension replies don't match");
	    if (!(foundmask & FoundDeliveredEvents))
		report_error("delivered events don't match");
	    if (!(foundmask & FoundDeviceEvents))
		report_error("device events don't match");
	    if (!(foundmask & FoundErrors))
		report_error("errors don't match");
	    if (!(foundmask & FoundClientStarted))
		report_error("client started doesn't match");
	    if (!(foundmask & FoundClientDied))
		report_error("client died doesn't match");
	}
    }
}

/* Check that the given context has the specified state 
 * (registered clients and protocol)
 */

void verify_context(Display *dpy, XRecordContext rc, XRecordState *state)
{
    XRecordState *retstate;
    int i, j;
    Status status;

    status = XRecordGetContext(dpy, rc, &retstate);
    if (!status)
    {
	report_error("Failed to get record context");
	return;
    }
    if (state->enabled != retstate->enabled)
	report_error("enabled field incorrect");
    if (state->enabled != retstate->enabled)
	report_error("context enabled field incorrect");
    if (state->datum_flags != retstate->datum_flags)
	report_error("context datum_flags field incorrect");
    if (state->nclients != retstate->nclients)
	report_error("context nclients field incorrect: want %d got %d",
		     state->nclients, retstate->nclients);

    for (i = 0; i < min(state->nclients, retstate->nclients); i++)
    {
	for (j = 0; j < state->nclients; j++)
	{
	    XRecordClientInfo *s = state->client_info[i];
	    XRecordClientInfo *rs = retstate->client_info[j];

	    if (CLIENT_BITS(s->client) == CLIENT_BITS(rs->client))
	    { /* found matching client; compare XRecordRanges */
		if (s->nranges != rs->nranges) {
		    report_error("nranges incorrect: want %d got %d",
				 s->nranges, rs->nranges);
		}
		check_range_subsumption(s->ranges, s->nranges,
					rs->ranges, rs->nranges);
		check_range_subsumption(rs->ranges, rs->nranges,
					s->ranges, s->nranges);
	    }
	}
    }
    XRecordFreeState(retstate);
}


void test_create_context()
{
    Display *dpy = begin_test_set("CreateContext");
    Display *client2;
    XRecordContext rc;
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    XRecordState rcstate;
    XRecordClientInfo *cinfo_ptr[3];
    XRecordClientInfo cinfo[3];
    XRecordClientSpec dpyid;

    XSetErrorHandler(protocol_error_expected);

    /* convenient way to name this client */
    dpyid = (XRecordClientSpec)XGContextFromGC(XDefaultGC(dpy, 0));

    /* we never use more than two of these, so that's all we alloc */
    ranges[0] = XRecordAllocRange();
    ranges[1] = XRecordAllocRange();

/* The following actions should cause a Match error. */

    begin_test("Specify an invalid CLIENTSPEC, expect Match error");
    null_record_ranges(ranges, 1);
    ranges[0]->core_requests.last = 10;
    clients[0] = 0;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadMatch)
	report_error("wanted BadMatch");
    if (protocol_error_major != record_majorop)
	report_error("BadMatch opcode wrong");
    if (protocol_error_minor != X_RecordCreateContext)
	report_error("BadMatch minor opcode wrong");
    end_test();

/* The following actions should cause a Value error. */

    begin_test("Specify core-requests with first > last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->core_requests.first = 11;
    ranges[0]->core_requests.last = 10;
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify core-replies with first > last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->core_replies.first = 20;
    ranges[0]->core_replies.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-requests with ext_minor.first > ext_minor.last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_requests.ext_major.first = 128;
    ranges[0]->ext_requests.ext_major.last = 255;
    ranges[0]->ext_requests.ext_minor.first = 128;
    ranges[0]->ext_requests.ext_minor.last = 0;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-requests with ext_major.first > ext_major.last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_requests.ext_major.first = 255;
    ranges[0]->ext_requests.ext_major.last = 128;
    ranges[0]->ext_requests.ext_minor.first = 0;
    ranges[0]->ext_requests.ext_minor.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-requests with ext_major.first between 1 and 127 inclusive, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_requests.ext_major.first = 127;
    ranges[0]->ext_requests.ext_major.last = 128;
    ranges[0]->ext_requests.ext_minor.first = 0;
    ranges[0]->ext_requests.ext_minor.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-requests with ext_major.last between 1 and 127 inclusive, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_requests.ext_major.first = 128;
    ranges[0]->ext_requests.ext_major.last = 120;
    ranges[0]->ext_requests.ext_minor.first = 0;
    ranges[0]->ext_requests.ext_minor.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-replies with ext_minor.first > ext_minor.last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_replies.ext_major.first = 128;
    ranges[0]->ext_replies.ext_major.last = 255;
    ranges[0]->ext_replies.ext_minor.first = 128;
    ranges[0]->ext_replies.ext_minor.last = 0;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-replies with ext_major.first > ext_major.last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_replies.ext_major.first = 255;
    ranges[0]->ext_replies.ext_major.last = 128;
    ranges[0]->ext_replies.ext_minor.first = 0;
    ranges[0]->ext_replies.ext_minor.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-replies with ext_major.first between 1 and 127 inclusive, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_replies.ext_major.first = 127;
    ranges[0]->ext_replies.ext_major.last = 128;
    ranges[0]->ext_replies.ext_minor.first = 0;
    ranges[0]->ext_replies.ext_minor.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify ext-replies with ext_major.last between 1 and 127 inclusive, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_replies.ext_major.first = 128;
    ranges[0]->ext_replies.ext_major.last = 120;
    ranges[0]->ext_replies.ext_minor.first = 0;
    ranges[0]->ext_replies.ext_minor.last = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify delivered-events with first == 1, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->delivered_events.first = 1;
    ranges[0]->delivered_events.last  = 3;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify delivered-events with last == 1, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->delivered_events.first = 0;
    ranges[0]->delivered_events.last  = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify delivered-events with first > last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->delivered_events.first = 4;
    ranges[0]->delivered_events.last  = 3;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify device-events with first == 1, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->device_events.first = 1;
    ranges[0]->device_events.last  = 3;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify device-events with last == 1, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->device_events.first = 0;
    ranges[0]->device_events.last  = 1;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify device-events with first > last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->device_events.first = 45;
    ranges[0]->device_events.last  = 31;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

    begin_test("Specify errors with first > last, expect Value error");
    null_record_ranges(ranges, 1);
    ranges[0]->errors.first = 22;
    ranges[0]->errors.last  = 13;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (protocol_error != BadValue) report_error("");
    end_test();

/* Specify an invalid context ID.  (May not be possible from the
  documented library interface.) */

/* Basic CreateContext functionality
 * 
 * None of the following actions should result in an error.  In each
 * case, the context should be verified by a GetContext.  Some of the
 * contexts should be freed by FreeContext.  Others should be left alone,
 * to be cleaned up when the client closes its connection.  This way we test
 * both destruction paths.
 */

    XSetErrorHandler(protocol_error_unexpected);

    begin_test("empty client list and empty ranges list");
    null_record_ranges(ranges, 1);
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);

    rcstate.enabled = False;
    rcstate.datum_flags = 0;
    rcstate.nclients = 0;
    rcstate.client_info = NULL;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * CurrentClients and an empty ranges list.
 */

    begin_test("CurrentClients and empty ranges list");
    null_record_ranges(ranges, 1);
    clients[0] = XRecordCurrentClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 0);
    rcstate.nclients = 1;
    rcstate.client_info = cinfo_ptr;
    cinfo_ptr[0] = &cinfo[0];
    cinfo_ptr[1] = &cinfo[1];
    cinfo_ptr[2] = &cinfo[2];
    cinfo[0].client = rc;
    cinfo[0].nranges = 0;
    cinfo[0].ranges = NULL;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * FutureClients and an empty ranges list.
 */

    begin_test("FutureClients and empty ranges list");
    null_record_ranges(ranges, 1);
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 0);
    cinfo[0].client = XRecordFutureClients;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * AllClients and an empty ranges list.
 */

    begin_test("AllClients and empty ranges list");
    null_record_ranges(ranges, 1);
    clients[0] = XRecordAllClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 0);
    rcstate.nclients = 2;
    cinfo[0].client = rc;
    cinfo[0].nranges = 0;
    cinfo[0].ranges = NULL;
    cinfo[1].client = XRecordFutureClients;
    cinfo[1].nranges = 0;
    cinfo[1].ranges = NULL;
    verify_context(dpy, rc, &rcstate);
    end_test();

    /* Don't free_context(dpy, rc) because the rc id is used in the next
     * test to identify the client.
     */

/* Create a context with a client-specifiers list containing
 * the XIDBASE of an existing client and an empty ranges list.
 */

    begin_test("specific client and empty ranges list");
    null_record_ranges(ranges, 1);
    clients[0] = dpyid;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 0);
    rcstate.nclients = 1;
    cinfo[0].client = rc;
    cinfo[0].nranges = 0;
    cinfo[0].ranges = NULL;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Tests with non-empty ranges list */

/* Create a context with an empty client-specifiers list and an
 * non-empty, valid ranges list.
 */

    begin_test("empty clients list and non-empty ranges list");
    null_record_ranges(ranges, 1);
    ranges[0]->core_requests.first = 0;
    ranges[0]->core_requests.last  = 127;
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 1);
    rcstate.nclients = 0;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * CurrentClients and an non-empty, valid ranges list.
 */

    begin_test("CurrentClients and non-empty ranges list");
    null_record_ranges(ranges, 2);
    ranges[0]->core_replies.first = 0;
    ranges[0]->core_replies.last  = 10;
    ranges[1]->core_replies.first = 20;
    ranges[1]->core_replies.last  = 30;
    clients[0] = XRecordCurrentClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 2);
    rcstate.nclients = 1;
    cinfo[0].client = rc;
    cinfo[0].nranges = 2;
    cinfo[0].ranges = ranges;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * FutureClients and an non-empty, valid ranges list.
 */

    begin_test("FutureClients and non-empty ranges list");
    null_record_ranges(ranges, 1);
    ranges[0]->errors.first = 0;
    ranges[0]->errors.last  = 127;
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    rcstate.nclients = 1;
    cinfo[0].client = XRecordFutureClients;
    cinfo[0].nranges = 1;
    cinfo[0].ranges = ranges;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * AllClients and an non-empty, valid ranges list.
 */

    begin_test("AllClients and non-empty ranges list");
    null_record_ranges(ranges, 2);
    ranges[0]->client_started = True;
    ranges[0]->ext_replies.ext_major.first = 128;
    ranges[0]->ext_replies.ext_major.last = 255;
    ranges[0]->ext_replies.ext_minor.first = 0;
    ranges[0]->ext_replies.ext_minor.last = 20;

    ranges[1]->ext_replies.ext_major.first = 128;
    ranges[1]->ext_replies.ext_major.last = 255;
    ranges[1]->ext_replies.ext_minor.first = 50;
    ranges[1]->ext_replies.ext_minor.last = 30000;
    clients[0] = XRecordAllClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 2);
    rcstate.nclients = 2;
    cinfo[0].client = XRecordFutureClients;
    cinfo[0].nranges = 2;
    cinfo[0].ranges = ranges;
    cinfo[1].client = dpyid;
    cinfo[1].nranges = 2;
    cinfo[1].ranges = ranges;
    verify_context(dpy, rc, &rcstate);
    end_test();

/* Create a context with a client-specifiers list containing the XIDBASE
 * of an existing client and an non-empty, valid ranges list.
 */

    begin_test("specific client and non-empty ranges list");
    null_record_ranges(ranges, 1);
    ranges[0]->client_died = True;
    clients[0] = dpyid;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    rcstate.nclients = 1;
    cinfo[0].client = dpyid;
    cinfo[0].nranges = 1;
    cinfo[0].ranges = ranges;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

/* Create a context with a client-specifiers list containing
 * FutureClients and the XIDBASE of an existing client and an non-empty,
 * valid ranges list.
 */

    begin_test("specific client and FutureClients and non-empty ranges list");
    null_record_ranges(ranges, 1);
    ranges[0]->delivered_events.first = 3;
    ranges[0]->delivered_events.last = 10;
    clients[0] = dpyid;
    clients[1] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 2, ranges, 1);
    rcstate.nclients = 2;
    cinfo[0].client = dpyid;
    cinfo[0].nranges = 1;
    cinfo[0].ranges = ranges;
    cinfo[1].client = XRecordFutureClients;
    cinfo[1].nranges = 1;
    cinfo[1].ranges = ranges;
    verify_context(dpy, rc, &rcstate);
    end_test();

/* Create a context with a client-specifiers list containing the XIDBASE
 * of two existing client and an non-empty, valid ranges list.
 */

    begin_test("two clients and non-empty ranges list");
    client2 = XOpenDisplay(display_env);
    if (!client2)
	report_error("Couldn't open display");
    else
    {
	null_record_ranges(ranges, 1);
	ranges[0]->device_events.first = 3;
	ranges[0]->device_events.last = 10;
	clients[0] = dpyid;
	clients[1] = XGContextFromGC(DefaultGC(client2, 0));
	rc = XRecordCreateContext(dpy, 0, clients, 2, ranges, 1);
	rcstate.nclients = 2;
	cinfo[0].client = dpyid;
	cinfo[0].nranges = 1;
	cinfo[0].ranges = ranges;
	cinfo[1].client = XGContextFromGC(DefaultGC(client2, 0));
	cinfo[1].nranges = 1;
	cinfo[1].ranges = ranges;
	verify_context(dpy, rc, &rcstate);
	free_context(dpy, rc);
	XCloseDisplay(client2);
    }
    end_test();

/* Create a context with a client-specifiers list containing the XIDBASE
 * of the same client twice and an non-empty, valid ranges list.
 */

    begin_test("duplicate client and non-empty ranges list");
    null_record_ranges(ranges, 2);
    ranges[0]->ext_replies.ext_major.first = 128;
    ranges[0]->ext_replies.ext_major.last = 128;
    ranges[0]->ext_replies.ext_minor.first = 0;
    ranges[0]->ext_replies.ext_minor.last = 20;

    ranges[1]->ext_replies.ext_major.first = 129;
    ranges[1]->ext_replies.ext_major.last = 129;
    ranges[1]->ext_replies.ext_minor.first = 50;
    ranges[1]->ext_replies.ext_minor.last = 30000;
    clients[0] = dpyid;
    clients[1] = dpyid;
    rc = XRecordCreateContext(dpy, 0, clients, 2, ranges, 2);
    rcstate.nclients = 1;
    cinfo[0].client = dpyid;
    cinfo[0].nranges = 2;
    cinfo[0].ranges = ranges;
    verify_context(dpy, rc, &rcstate);
    free_context(dpy, rc);
    end_test();

    XFree(ranges[0]);
    end_test_set(dpy);
}

/* Here is some machinery to avoid getting stuck waiting for a reply on
 * the data connection that never comes.  Basically, we wait for 10
 * seconds before timing out.
 */

enum {WAITING, TIMEDOUT, CORRECTREPLY, INCORRECTREPLY} reply_status;

int record_category;

void sigalarm_handler(int sig)
{
    reply_status = TIMEDOUT;
}

void set_alarm()
{
    alarm(10);
    (void)signal(SIGALRM, sigalarm_handler);
    reply_status = WAITING;
}

void wait_for_data_on(Display *dataconn, char *fail_msg)
{
    set_alarm();
    while (reply_status != TIMEDOUT  &&  record_category != XRecordEndOfData)
    {
	XRecordProcessReplies(dataconn);
#ifdef HAS_USLEEP
	usleep(100000);
#else
	sleep(1);
#endif
    }
    if (reply_status == TIMEDOUT)
	report_error("expected protocol never arrived %s", fail_msg);
}

/* Functions of type ProtocolCompareProcPtr compare two protocol messages that
 * are supposed to be identical, returning True if they are.  It is necessary
 * to have message-specific comparison functions instead of just using
 * memcmp because of padding occurs in different places for different
 * messages, is not initialized (thus contains random values), and
 * is not significant (differences in pad bytes don't affect whether two
 * requests are identical).
 */

typedef Bool (*ProtocolCompareProcPtr) (void *r1, void *r2);

/*
 * All the info you need to compare protocol elements
 */
#define COMP_DATA_SIZE 7

typedef struct {
    ProtocolCompareProcPtr compareproc[COMP_DATA_SIZE];
    XRecordInterceptData xrid[COMP_DATA_SIZE];
    int data_index;
} ComparisonData;

/*
 * This is supposed to be called once per recorded protocol element.
 * expected_data->data_len can be -1 to mean don't check
 */
void protocol_element_callback(XPointer closure,
		   XRecordInterceptData *recorded_data)
{
    ComparisonData *expected_data = (ComparisonData *)closure;
    Bool found = True;
    int idi = expected_data->data_index;

#ifndef NOPRINT
    printf("recorded category %d", recorded_data->category);
    if ( recorded_data->category != XRecordEndOfData &&
	 recorded_data->category != XRecordStartOfData)
	printf(", len %lu, client 0x%x",
	       recorded_data->data_len, recorded_data->id_base);
    printf("\n");
#endif

    record_category = recorded_data->category;

    if (expected_data->xrid[idi].id_base  != recorded_data->id_base)
    {
	report_error("id_base doesn't match: want 0x%x got 0x%x",
		     expected_data->xrid[idi].id_base,
		     recorded_data->id_base);
	found = False;
    }
    if (expected_data->xrid[idi].category != recorded_data->category)
    {
	report_error("category doesn't match: want %d got %d",
		     expected_data->xrid[idi].category,
		     recorded_data->category);
	found = False;
    }
    if (expected_data->xrid[idi].client_swapped != recorded_data->client_swapped)
    {
	report_error("client_swapped doesn't match");
	found = False;
    }
    /* use -1 for server-dependent data of variable length */
    if ((long)(expected_data->xrid[idi].data_len) != -1 &&
	expected_data->xrid[idi].data_len != recorded_data->data_len)
    {
	report_error("data_len doesn't match: want %d got %d",
		     expected_data->xrid[idi].data_len,
		     recorded_data->data_len);
	found = False;
    }
    if (expected_data->xrid[idi].data_len != 0) {
	if (!expected_data->xrid[idi].data || !recorded_data->data) {
	    report_error("void protocol data");
	    found = False;
	} else if (! (*expected_data->compareproc[idi])
		         ((xReq *)expected_data->xrid[idi].data,
		          (xReq *)recorded_data->data))
	{
	    /* comparproc reports its own error */
	    found = False;
	}
    }
    if (found) reply_status = CORRECTREPLY;
    else reply_status = INCORRECTREPLY;

    XRecordFreeData(recorded_data);
    expected_data->data_index++;
}

/* This compares two generic replies. */
Bool compare_generic_reply(void *p1, void *p2)
{
    xGenericReply *r1 = (xGenericReply *)p1;
    xGenericReply *r2 = (xGenericReply *)p2;

    if (r1->type != r2->type) {
	report_error("reply protocol data type mismatch: wanted %d got %d",
		     r1->type, r2->type);
	return False;
    }
    if (r1->length != r2->length) {
	report_error("reply protocol data length mismatch: wanted %d got %d",
		     r1->length, r2->length);
	return False;
    }
    return True;
}

/* This compares two NoOp requests. */
Bool compare_noop(void *p1, void *p2)
{
    xReq *r1 = (xReq *)p1;
    xReq *r2 = (xReq *)p2;

    if (r1->reqType != r2->reqType) {
	report_error("protocol data request type mismatch: wanted %d got %d",
		     r1->reqType, r2->reqType);
	return False;
    }
    if (r1->length != r2->length) {
	report_error("request protocol data length mismatch: wanted %d got %d",
		     r1->length, r2->length);
	return False;
    }
    return True;
}

/* This compares two MotionNotify events. */
Bool compare_motionevent(void *p1, void *p2)
{
    xEvent *r1 = (xEvent *)p1;
    xEvent *r2 = (xEvent *)p2;

    if (r1->u.u.type != r2->u.u.type) {
	report_error("motion event type mismatch: wanted %d got %d",
		     r1->u.u.type, r2->u.u.type);
	return False;
    }
    if (r1->u.keyButtonPointer.rootX != r2->u.keyButtonPointer.rootX
	|| r1->u.keyButtonPointer.rootY != r2->u.keyButtonPointer.rootY) {
	report_error("motion event coords mismatch: wanted %d,%d got %d,%d",
		     r1->u.keyButtonPointer.rootX,
		     r1->u.keyButtonPointer.rootY,
		     r2->u.keyButtonPointer.rootX,
		     r2->u.keyButtonPointer.rootY);
	return False;
    }
    if (r1->u.keyButtonPointer.root != r2->u.keyButtonPointer.root) {
	report_error("motion event root mismatch: wanted %d got %d",
		     r1->u.keyButtonPointer.root,
		     r2->u.keyButtonPointer.root);
	return False;
    }
    return True;
}

/* This compares two errors. */
Bool compare_error(void *p1, void *p2)
{
    xError *r1 = (xError *)p1;
    xError *r2 = (xError *)p2;

    if (r1->type != r2->type) {
	report_error("protocol data error type mismatch: wanted %d got %d",
		     r1->type, r2->type);
	return False;
    }
    if (r1->errorCode != r2->errorCode) {
	report_error("protocol data error code mismatch: wanted %d got %d",
		     r1->errorCode, r2->errorCode);
	return False;
    }
    if (r1->resourceID != r2->resourceID) {
	report_error("protocol data error resource mismatch: wanted 0x%x got 0x%x",
		     r1->resourceID, r2->resourceID);
	return False;
    }
    if (r1->minorCode != r2->minorCode) {
	report_error("protocol data error minor op mismatch: wanted %d got %d",
		     r1->minorCode, r2->minorCode);
	return False;
    }
    if (r1->majorCode != r2->majorCode) {
	report_error("protocol data error major op mismatch: wanted %d got %d",
		     r1->majorCode, r2->majorCode);
	return False;
    }
    return True;
}

/* checks that the client setup message is correct.  p1 is not used */
Bool check_setup(void *p1, void *p2)
{
    xConnSetupPrefix *s2 = (xConnSetupPrefix*)p2;
    if (s2->success == xTrue &&
	s2->majorVersion == X_PROTOCOL &&
	s2->minorVersion == X_PROTOCOL_REVISION)
	return True;

    report_error("connection setup bad");
    return False;
}

/*
 * for setting up comparison data
 */
void make_sod(ComparisonData *cd, int dindex)
{
    cd->xrid[dindex].id_base = 0;
    cd->xrid[dindex].category = XRecordStartOfData;
    cd->xrid[dindex].client_swapped = server_swap;
    cd->xrid[dindex].data_len = 0;
    cd->xrid[dindex].data = NULL;
    cd->compareproc[dindex] = compare_noop;
    cd->data_index = dindex;	/* convenience */
}

void make_eod(ComparisonData *cd, int dindex)
{
    cd->xrid[dindex].id_base = 0;
    cd->xrid[dindex].category = XRecordEndOfData;
    cd->xrid[dindex].client_swapped = server_swap;
    cd->xrid[dindex].data_len = 0;
    cd->xrid[dindex].data = NULL;
    cd->compareproc[dindex] = compare_noop;
}

void make_clientdied(ComparisonData *cd, int dindex, XID client)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordClientDied;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 0;
    cd->xrid[dindex].data = NULL;
    cd->compareproc[dindex] = compare_noop;
}

void make_noop_req(ComparisonData *cd, int dindex,
		   XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromClient;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 1;
    cd->xrid[dindex].data = buffer;
    ((xReq*)cd->xrid[dindex].data)->reqType = X_NoOperation;
    ((xReq*)cd->xrid[dindex].data)->length = 1;
    cd->compareproc[dindex] = compare_noop;
}

void make_getscreensaver_req(ComparisonData *cd, int dindex,
			     XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromClient;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 1;
    cd->xrid[dindex].data = buffer;
    ((xReq*)cd->xrid[dindex].data)->reqType = X_GetScreenSaver;
    ((xReq*)cd->xrid[dindex].data)->length = 1;
    cd->compareproc[dindex] = compare_noop;
}

void make_getscreensaver_reply(ComparisonData *cd, int dindex,
			       XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromServer;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 8;
    cd->xrid[dindex].data = buffer;
    ((xGenericReply*)cd->xrid[dindex].data)->type = X_Reply;
    ((xGenericReply*)cd->xrid[dindex].data)->length = 0;
    cd->compareproc[dindex] = compare_generic_reply;
}

void make_getwindowattrs_req(ComparisonData *cd, int dindex,
			     XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromClient;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 2;
    cd->xrid[dindex].data = buffer;
    ((xReq*)cd->xrid[dindex].data)->reqType = X_GetWindowAttributes;
    ((xReq*)cd->xrid[dindex].data)->length = 2;
    cd->compareproc[dindex] = compare_noop;
}

void make_getwindowattrs_reply(ComparisonData *cd, int dindex,
			       XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromServer;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 11;
    cd->xrid[dindex].data = buffer;
    ((xGenericReply*)cd->xrid[dindex].data)->type = X_Reply;
    ((xGenericReply*)cd->xrid[dindex].data)->length = 3;
    cd->compareproc[dindex] = compare_generic_reply;
}

void make_getgeometry_req(ComparisonData *cd, int dindex,
			     XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromClient;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 2;
    cd->xrid[dindex].data = buffer;
    ((xReq*)cd->xrid[dindex].data)->reqType = X_GetGeometry;
    ((xReq*)cd->xrid[dindex].data)->length = 2;
    cd->compareproc[dindex] = compare_noop;
}

void make_getgeometry_reply(ComparisonData *cd, int dindex,
			       XID client, unsigned char *buffer)
{
    cd->xrid[dindex].id_base = CLIENT_BITS(client);
    cd->xrid[dindex].category = XRecordFromServer;
    cd->xrid[dindex].client_swapped = client_swap;
    cd->xrid[dindex].data_len = 8;
    cd->xrid[dindex].data = buffer;
    ((xGenericReply*)cd->xrid[dindex].data)->type = X_Reply;
    ((xGenericReply*)cd->xrid[dindex].data)->length = 0;
    cd->compareproc[dindex] = compare_generic_reply;
}

void test_register_clients()
{
    Display *dpy = begin_test_set("RegisterClients");
    Display *client2;
    XRecordClientSpec c2id;
    Display *dataconn;
    XRecordContext rc;
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    XRecordState rcstate;	/* known good state */
    XRecordClientInfo *cinfo_ptr[10];
    XRecordClientInfo cinfo[10];
    ComparisonData cd;
    int idi;
    unsigned char protocol_buffer[256];
    int i;

    /* we never use more than two of these, so that's all we alloc */
    ranges[0] = XRecordAllocRange();
    ranges[1] = XRecordAllocRange();

    /* we never change these */
    rcstate.enabled = False;
    rcstate.datum_flags = 0;
    rcstate.client_info = cinfo_ptr;
    for (i = 0; i < 10; i++)
	cinfo_ptr[i] = &cinfo[i];

    begin_test("invalid context id, expect Context error");
    XSetErrorHandler(protocol_error_expected);
    rc = 0;
    /* set up valid clients and ranges so there's only one possible
     * error return
     */
    clients[0] = XRecordFutureClients;
    null_record_ranges(ranges, 1);
    ranges[0]->client_started = True;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	report_error("cannot register clients");
    if (protocol_error != BadContext)
	report_error("wanted RecordContext error");
    if (protocol_error_major != record_majorop)
	report_error("RecordContext error opcode wrong");
    if (protocol_error_minor != X_RecordRegisterClients)
	report_error("RecordContext error minor opcode wrong");
    end_test();

    /* Create a context with no clients or protocol registered for use in
     * subsequent tests.  Get and verify the context.
     */
    begin_test("create empty context");
    XSetErrorHandler(protocol_error_unexpected);
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    rcstate.nclients = 0;
    verify_context(dpy, rc, &rcstate);
    end_test();

    begin_test("Specify an invalid CLIENTSPEC, expect Match error");
    XSetErrorHandler(protocol_error_expected);
    null_record_ranges(ranges, 1);
    ranges[0]->core_replies.last = 100;
    clients[0] = 0;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	report_error("cannot register client");
    if (protocol_error != BadMatch) report_error("");
    end_test();

    begin_test("valid client with non-empty, valid ranges");
    XSetErrorHandler(protocol_error_unexpected);

    /* note: this test uses the second element of ranges because the range
     * data is also needed for the next test, whcih uses the first range.
     */
    null_record_ranges(ranges + 1, 1);
    ranges[1]->errors.last = 100;
    clients[0] = rc;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges + 1, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[0]->nranges = 1;
	rcstate.client_info[0]->ranges = ranges + 1;
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("second client with non-empty, valid ranges");
    XSetErrorHandler(protocol_error_unexpected);
    client2 = XOpenDisplay(display_env);
    if (!client2)
	report_error("Couldn't open display");
    else
    {
	null_record_ranges(ranges, 1);
	ranges[0]->core_replies.last = 50;
	clients[0] = XGContextFromGC(DefaultGC(client2, 0));
	if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	    report_error("");
	else
	{
	    rcstate.nclients = 2;
	    rcstate.client_info[0]->client = clients[0];
	    rcstate.client_info[0]->nranges = 1;
	    rcstate.client_info[0]->ranges = ranges;
	    /* residual from previous test */
	    rcstate.client_info[1]->client = rc;
	    rcstate.client_info[1]->nranges = 1;
	    rcstate.client_info[1]->ranges = ranges + 1;
	    verify_context(dpy, rc, &rcstate);
	}
	XCloseDisplay(client2);
    }
    end_test();
    
    begin_test("re-register client with different ranges");
    null_record_ranges(ranges, 1);
    ranges[0]->device_events.first = 2;
    ranges[0]->device_events.last = 100;
    clients[0] = rc;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[0]->nranges = 1;
	rcstate.client_info[0]->ranges = ranges;
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("re-register client with empty ranges");
    clients[0] = rc;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, NULL, 0))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[0]->nranges = 0;
	rcstate.client_info[0]->ranges = NULL;
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("CurrentClients with valid ranges");
    null_record_ranges(ranges, 1);
    ranges[0]->delivered_events.first = 2;
    ranges[0]->delivered_events.last = 100;
    clients[0] = XRecordCurrentClients;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[0]->nranges = 1;
	rcstate.client_info[0]->ranges = ranges;
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("AllClients with valid ranges");
    null_record_ranges(ranges, 1);
    ranges[0]->ext_requests.ext_major.first = 128;
    ranges[0]->ext_requests.ext_major.last = 128;
    ranges[0]->ext_requests.ext_minor.first = 0;
    ranges[0]->ext_requests.ext_minor.last = 65535;
    clients[0] = XRecordAllClients;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	report_error("");
    else
    {
	rcstate.nclients = 2;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[0]->nranges = 1;
	rcstate.client_info[0]->ranges = ranges;
	rcstate.client_info[1]->client = XRecordFutureClients;
	rcstate.client_info[1]->nranges = 1;
	rcstate.client_info[1]->ranges = ranges;
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("FutureClients with valid ranges");
    null_record_ranges(&ranges[1], 1);
    ranges[1]->ext_requests.ext_major.first = 128;
    ranges[1]->ext_requests.ext_major.last = 255;
    ranges[1]->ext_requests.ext_minor.first = 0;
    ranges[1]->ext_requests.ext_minor.last = 32767;
    clients[0] = XRecordFutureClients;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, &ranges[1], 1))
	report_error("");
    else
    {
	rcstate.nclients = 2;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[0]->nranges = 1;
	rcstate.client_info[0]->ranges = &ranges[0];
	rcstate.client_info[1]->client = XRecordFutureClients;
	rcstate.client_info[1]->nranges = 1;
	rcstate.client_info[1]->ranges = &ranges[1];
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("FutureClients, then multiple clients, with valid ranges");
    null_record_ranges(&ranges[0], 1);
    ranges[0]->ext_requests.ext_major.first = 128;
    ranges[0]->ext_requests.ext_major.last = 255;
    ranges[0]->ext_requests.ext_minor.first = 0;
    ranges[0]->ext_requests.ext_minor.last = 32767;
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    {
	Display *clients[9];
	int i;

	rcstate.nclients = 1;
	rcstate.client_info[0]->client = XRecordFutureClients;
	rcstate.client_info[0]->nranges = 1;
	rcstate.client_info[0]->ranges = &ranges[0];

	for (i = 0; i < 9; i++)
	{
	    clients[i] = XOpenDisplay(display_env);
	    if (!clients[i])
	    {
		report_error("Failed to open display");
		continue;
	    }
	    rcstate.client_info[rcstate.nclients]->client = XGContextFromGC(
						DefaultGC(clients[i], 0));
	    rcstate.client_info[rcstate.nclients]->nranges = 1;
	    rcstate.client_info[rcstate.nclients]->ranges = &ranges[0];
	    rcstate.nclients++;
	}
 	verify_context(dpy, rc, &rcstate);
	for (i = 0; i < 9; i++)
	    XCloseDisplay(clients[i]);
	rcstate.nclients = 1;
 	verify_context(dpy, rc, &rcstate);
    }
    end_test();

    begin_test("register while enabled");

    /* set up some stuff for subsequent tests */

    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    client2 = XOpenDisplay(display_env);
    dataconn = XOpenDisplay(display_env);
    if (!client2 || !dataconn)
    {
	if (client2) XCloseDisplay(client2);
	if (dataconn) XCloseDisplay(dataconn);
	report_error("Couldn't open display");
	return;
    }
    XSynchronize(client2, True);
    c2id = XGContextFromGC(DefaultGC(client2, 0));

    /* set up comparison data */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_eod(&cd, idi++);

    if (!XRecordEnableContextAsync(dataconn, rc, protocol_element_callback,
				   (XPointer)&cd))
    {
	report_error("XRecordEnableContextAsync failed");
	return;
    }

    ranges[0]->core_requests.first = X_NoOperation;
    ranges[0]->core_requests.last =  X_NoOperation;
    clients[0] = c2id;
    if (!XRecordRegisterClients(dpy, rc, 0, clients, 1, ranges, 1))
	report_error("XRecordRegisterClients failed");

    XNoOp(client2);		/* record this */

    if (!XRecordFreeContext(dpy, rc))
	report_error("XRecordFreeContext failed");
    XFlush(dpy);
    wait_for_data_on(dataconn, "");
    XCloseDisplay(client2);
    XCloseDisplay(dataconn);
    end_test();

    XFree(ranges[0]);
    XFree(ranges[1]);
    end_test_set(dpy);
}

void test_unregister_clients()
{
    Display *dpy = begin_test_set("UnregisterClients");
    Display *client2;
    XRecordContext rc;
    XRecordClientSpec dpyid, c2id;
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    XRecordState rcstate;
    XRecordClientInfo *cinfo_ptr[3];
    XRecordClientInfo cinfo[3];
    int i;

    /* convenient way to name this client */
    dpyid = (XRecordClientSpec)XGContextFromGC(XDefaultGC(dpy, 0));

    /* we never use more than one of these, so that's all we alloc */
    ranges[0] = XRecordAllocRange();

    begin_test("invalid context id, expect Context error");
    XSetErrorHandler(protocol_error_expected);
    clients[0] = XRecordCurrentClients;
    if (!XRecordUnregisterClients(dpy, 0, clients, 1))
	report_error("XRecordUnregisterClients");
    if (protocol_error != BadContext)
	report_error("");
    end_test();

    /* we don't change these for the rest of the testset */
    XSetErrorHandler(protocol_error_unexpected);
    rcstate.enabled = False;
    rcstate.datum_flags = 0;
    rcstate.client_info = cinfo_ptr;
    cinfo_ptr[0] = &cinfo[0];
    cinfo_ptr[1] = &cinfo[1];
    cinfo_ptr[2] = &cinfo[2];
    null_record_ranges(ranges, 1);
    ranges[0]->client_started = True;
    ranges[0]->client_died = True;
    for (i = 0; i < 3; i++)
    {
	rcstate.client_info[i]->nranges = 1;
	rcstate.client_info[i]->ranges = ranges;
    }
    client2 = XOpenDisplay(display_env);
    if (!client2)
    {
	report_error("Couldn't open display");
	return; /* everything else would fail anyway */
    }
    c2id = XGContextFromGC(DefaultGC(client2, 0));

    begin_test("AllClients - AllClients");
    clients[0] = XRecordAllClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("AllClients - CurrentClients");
    clients[0] = XRecordAllClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordCurrentClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = XRecordFutureClients;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("AllClients - FutureClients");
    clients[0] = XRecordAllClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordFutureClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 2;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[1]->client = c2id;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("AllClients - specific client");
    clients[0] = XRecordAllClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = rc;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 2;
	rcstate.client_info[0]->client = XRecordFutureClients;
	rcstate.client_info[1]->client = c2id;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    /* creating with CurrentClients */

    begin_test("CurrentClients - AllClients");
    clients[0] = XRecordCurrentClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordAllClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("CurrentClients - CurrentClients");
    clients[0] = XRecordCurrentClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("CurrentClients - FutureClients");
    clients[0] = XRecordCurrentClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordFutureClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 2;
	rcstate.client_info[0]->client = rc;
	rcstate.client_info[1]->client = c2id;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("CurrentClients - specific client");
    clients[0] = XRecordCurrentClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = c2id;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = rc;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

   /* creating with FutureClients */

    begin_test("FutureClients - AllClients");
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordAllClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("FutureClients - CurrentClients");
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordCurrentClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = XRecordFutureClients;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("FutureClients - FutureClients");
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("FutureClients - specific client");
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = c2id;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = XRecordFutureClients;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

   /* creating with specific client */

    begin_test("specific client - AllClients");
    clients[0] = c2id;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordAllClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("specific client - CurrentClients");
    clients[0] = dpyid;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordCurrentClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("specific client - FutureClients");
    clients[0] = c2id;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    clients[0] = XRecordFutureClients;
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 1;
	rcstate.client_info[0]->client = c2id;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    begin_test("specific client - specific client");
    clients[0] = dpyid;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (!XRecordUnregisterClients(dpy, rc, clients, 1))
	report_error("");
    else
    {
	rcstate.nclients = 0;
	verify_context(dpy, rc, &rcstate);
    }
    free_context(dpy, rc);
    end_test();

    XCloseDisplay(client2);
    XFree(ranges[0]);
    end_test_set(dpy);
}

void test_get_context()
{
    Display *dpy = begin_test_set("GetContext");
    XRecordState *rs;
    Status status;

    begin_test("invalid context id, expect Context error");
    XSetErrorHandler(protocol_error_expected);
    status = XRecordGetContext(dpy, 0, &rs);
    if (protocol_error != BadContext)	report_error("");
    if (status) {
	report_error("XRecordGetContext should have failed but didn't");
	XRecordFreeState(rs);
    }
    end_test();

    end_test_set(dpy);
}

/*
 * compares two simple requests
 * and saves their time and seqs for the caller to check
 */
unsigned long seq1, seq2;
Time time1, time2;
int current_seq_time = 1;

void time_seq_callback(XPointer closure, XRecordInterceptData *recorded_data)
{
    if (recorded_data->category == XRecordFromClient
	|| recorded_data->category == XRecordFromServer)
    {
	if (current_seq_time == 1) {
	    seq1 = recorded_data->client_seq;
	    time1 = recorded_data->server_time;
	    current_seq_time = 2;
	} else {
	    seq2 = recorded_data->client_seq;
	    time2 = recorded_data->server_time;
	    current_seq_time = 1;
	}
    }
    /* do the basic processing */
    protocol_element_callback(closure, recorded_data);       
}

/*
 * enable a context and wait until it is enabled.
 * The wait won't be nec once we have StartOfData
 */
void enable_async(Display *datadpy, Display *cntldpy,
		  XRecordContext rc,
		  XRecordInterceptProc time_seq_callback, ComparisonData *cdp)
{
    Status s;

    s = XRecordEnableContextAsync (datadpy, rc, time_seq_callback,
				   (XPointer)cdp);
    if (!s)
	report_error("cannot enable async");
}

/*
 * enable recording and generate protocol from multiple clients
 */
void enable_multi_record(Display *datadpy1, Display *datadpy2,
			 Display *dpy1, Display *dpy2,
			 XRecordContext rc1, XRecordContext rc2,
			 ComparisonData *cdp1, ComparisonData *cdp2)
{
    int ig;

    enable_async(datadpy1, dpy1, rc1, protocol_element_callback, cdp1);
    if (rc1 != rc2)
	enable_async(datadpy2, dpy1, rc2, protocol_element_callback, cdp2);
	
    XNoOp(dpy1);
    XNoOp(dpy2);
    XGetScreenSaver(dpy1, &ig, &ig, &ig, &ig);
    XGetScreenSaver(dpy2, &ig, &ig, &ig, &ig);

    if (!XRecordDisableContext(dpy1, rc1))
	report_error("XRecordDisableContext 1");
    if (!XRecordDisableContext(dpy1, rc2))
	report_error("XRecordDisableContext w");

    /* wait for all the Record data */
    XSync(datadpy1, False);
    XSync(datadpy2, False);
}


/*
 * test recording multiple clients on multiple contexts.
 * The two recorded clients may be the same, as may be the contexts.
 * The first recorded client doubles as the control connection.
 *
 * can do either subtest: with same protocol set or different protocol set
 */
void enable_multi_client_context(Display *datadpy1, Display *datadpy2,
				 Display *dpy1, Display *dpy2,
				 XRecordContext rc1, XRecordContext rc2,
				 ComparisonData *cdp1, ComparisonData *cdp2,
				 Bool same)
{
    XRecordRange *range1[2];
    XRecordRange *range2[2];
    XRecordClientSpec client1 = XGContextFromGC(DefaultGC(dpy1, 0));
    XRecordClientSpec client2 = XGContextFromGC(DefaultGC(dpy2, 0));
    int nranges;

    range1[0] = XRecordAllocRange();
    range1[1] = XRecordAllocRange();
    range2[0] = XRecordAllocRange();
    range2[1] = XRecordAllocRange();

    /*
     */
    /* set up ranges */
    range1[0]->core_requests.first = X_NoOperation;
    range1[0]->core_requests.last = X_NoOperation;
    range2[0]->core_requests.first = X_GetScreenSaver;
    range2[0]->core_requests.last = X_GetScreenSaver;
    if (same) {
	range1[1]->core_requests.first = X_GetScreenSaver;
	range1[1]->core_requests.last = X_GetScreenSaver;
	range2[1]->core_requests.first = X_NoOperation;
	range2[1]->core_requests.last = X_NoOperation;
	nranges = 2;
    } else {
	nranges = 1;
    }

    if (!XRecordRegisterClients(dpy1, rc1, 0, &client1, 1, range1, nranges))
	report_error("XRecordRegisterClients 1");
    if (!XRecordRegisterClients(dpy1, rc2, 0, &client2, 1, range2, nranges))
	report_error("XRecordRegisterClients 2");

    enable_multi_record(datadpy1, datadpy2, dpy1, dpy2, rc1, rc2, cdp1, cdp2);

    if (!XRecordUnregisterClients(dpy1, rc1, &client1, 1))
	report_error("XRecordUnregisterClients 1");
    if (!XRecordUnregisterClients(dpy1, rc2, &client2, 1))
	report_error("XRecordUnregisterClients 1");

    XFree(range1[0]);
    XFree(range1[1]);
    XFree(range2[0]);
    XFree(range2[1]);
}

void test_enable_context_async()
{
    Display *dpy = begin_test_set("EnableContextAsync");
    Display *client2, *client3, *datadpy2;
    XRecordContext rc, rc2;
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    ComparisonData cd, cd2;
    int idi;
    XRecordClientSpec c2id, c3id;
    int srvtime, clitime, clisequence;
    int req_rep;
    XWindowAttributes win_attrs_ig;
    unsigned char protocol_buffer[256];
    int i;
    Status s;
    xRecordDisableContextReq *disable;
    short rootx = 0xc9;
    short rooty = 0x67;
    Window rootwin;

    ranges[0] = XRecordAllocRange();
    ranges[1] = XRecordAllocRange();

    begin_test("Verify get Match error on second enable of same RC");
    client2 = XOpenDisplay(display_env);
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    if (!rc)
	report_error("Failed to create context");
    idi = 0;
    make_sod(&cd, idi++);
    make_eod(&cd, idi++);
    s = XRecordEnableContextAsync(client2, rc, protocol_element_callback,
			      (XPointer)&cd);
    if (!s)
	report_error("XRecordEnableContextAsync returned False");
    XSetErrorHandler(protocol_error_expected);
    s = XRecordEnableContextAsync(dpy, rc, protocol_element_callback,
			      (XPointer)NULL);
    if (s)
	report_error("invalid XRecordEnableContextAsync succeeded");
    if (protocol_error != BadMatch) report_error("no BadMatch");
    XSetErrorHandler(protocol_error_unexpected);
    if (!XRecordDisableContext(dpy, rc))
	report_error("disable context");
    if (!XRecordFreeContext(dpy, rc))
	report_error("free context");
    XFlush(dpy);
    XCloseDisplay(client2);
    end_test();

    begin_test("all 8 datum_flags combos");
    client2 = XOpenDisplay(display_env);
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    make_sod(&cd, 0);
    make_eod(&cd, 4);
    for (req_rep=0; req_rep<2; req_rep++) {
	if (req_rep == 1) {
	    end_test();
	    begin_test("all 8 datum_flags combos (reply)");
	}
	for (srvtime=0; srvtime<=XRecordFromServerTime;
	     srvtime += XRecordFromServerTime) {
	    for (clitime=0; clitime<=XRecordFromClientTime;
		 clitime += XRecordFromClientTime) {
		for (clisequence=0;
		     clisequence<=XRecordFromClientSequence;
		     clisequence += XRecordFromClientSequence) {
		    if (req_rep == 0) {
			/* test of requests */
			ranges[0]->core_requests.first = X_GetWindowAttributes;
			ranges[0]->core_requests.last = X_GetGeometry;
			ranges[0]->client_died = True;
		    } else {
			/* test of replies */
			ranges[0]->core_requests.first = 0;
			ranges[0]->core_requests.last = 0;
			ranges[0]->core_replies.first = X_GetWindowAttributes;
			ranges[0]->core_replies.last = X_GetGeometry;
			ranges[0]->client_died = True;
		    }
		    client3 = XOpenDisplay(display_env);
		    c3id = XGContextFromGC(DefaultGC(client3, 0));
		    clients[0] = c3id;
		    /* what to expect */
		    if (req_rep == 0) {
			make_getwindowattrs_req(&cd, 1, c3id, protocol_buffer);
			make_getgeometry_req(&cd, 2, c3id, protocol_buffer+64);
			make_clientdied(&cd, 3, c3id);
		    } else {
			make_getwindowattrs_reply(&cd, 1, c3id, protocol_buffer);
			make_getgeometry_reply(&cd, 2, c3id, protocol_buffer+64);
			make_clientdied(&cd, 3, c3id);
		    }
		    cd.data_index = 0;
		    XRecordRegisterClients
			(client2, rc, srvtime|clitime|clisequence,
			 clients, 1, ranges, 1);
		    XSync(client2, False);
		    enable_async(dpy, client2, rc, time_seq_callback, &cd);
		    /* This function is useful because in R6 Xlib
		       it sends off both requests then waits for
		       both replies, so we are more likely to get both
		       in the same Record reply. */
		    XGetWindowAttributes(client3,
					 XDefaultRootWindow(client3),
					 &win_attrs_ig);
		    XCloseDisplay(client3);
		    if (!XRecordDisableContext(client2, rc))
			report_error("disable");
		    XFlush(client2);
		    wait_for_data_on(dpy, "pair, EndOfData");
		    if (req_rep == 0) {
			if (clitime && time1 == time2)
			    report_error("probably should get from-client-time: %lu==%lu",
					 time1, time2);
			if (!clitime && time1 != time2)
			    report_error("probably shouldn't get from-client-time, %lu!=%lu",
					 time1, time2);
			if (clisequence && seq1 == seq2)
			    report_error("didn't get from-client-seq, %lu==%lu",
					 seq1, seq1);
			if (!clisequence && seq1 != seq2)
			    report_error("probably shouldn't get from-client-seq, %lu!=%lu",
					 seq1, seq2);
		    } else {
			if (srvtime && time1 == time2)
			    report_error("probably should get from-server-time, %lu==%lu",
					 time1, time2);
			if (!srvtime && time1 != time2)
			    report_error("probably shouldn't get from-server-time, %lu!=%lu",
					 time1, time2);
		    }
		}
	    }
	}
    }
    XCloseDisplay(client2);
    if (!XRecordFreeContext(dpy, rc))
	report_error("free context");
    end_test();

    begin_test("try all categories");
    /* events, dev events, errors, started, died */
    client2 = XOpenDisplay(display_env); /* control client */
    XSynchronize(client2, True);
    ranges[0]->device_events.first = MotionNotify;
    ranges[0]->device_events.last = MotionNotify;
    ranges[0]->delivered_events.first = MotionNotify;
    ranges[0]->delivered_events.last = MotionNotify;
    ranges[0]->errors.first = BadCursor;
    ranges[0]->errors.last = BadCursor;
    ranges[0]->client_started = True;
    ranges[0]->client_died = True;
    clients[0] = XRecordFutureClients;
    rc = XRecordCreateContext(client2, 0, clients, 1, ranges, 1);
    /* what we expect: start, dev_event, delivered event, error, died */
    idi = 0;
    make_sod(&cd, idi++);
    cd.xrid[idi].category = XRecordClientStarted;
    cd.xrid[idi].client_swapped = client_swap;
    cd.xrid[idi].data_len = -1;	/* variable */
    cd.xrid[idi].data = protocol_buffer; /* not used */
    cd.compareproc[idi] = check_setup;
    idi++;
    cd.xrid[idi].category = XRecordFromServer; /* device event */
    cd.xrid[idi].client_swapped = 0; /* 1.13: always per recording client */
    cd.xrid[idi].id_base = 0;
    cd.xrid[idi].data_len = 8;
    cd.xrid[idi].data = protocol_buffer+0;
    ((xEvent*)cd.xrid[idi].data)->u.u.type = MotionNotify;
    ((xEvent*)cd.xrid[idi].data)->u.keyButtonPointer.rootX = rootx;
    ((xEvent*)cd.xrid[idi].data)->u.keyButtonPointer.rootY = rooty;

    cd.compareproc[idi] = compare_motionevent;
    idi++;
    cd.xrid[idi].category = XRecordFromServer; /* delivered event */
    cd.xrid[idi].client_swapped = client_swap;
    cd.xrid[idi].data_len = 8;
    cd.xrid[idi].data = protocol_buffer+32;
    ((xEvent*)cd.xrid[idi].data)->u.u.type = MotionNotify;
    ((xEvent*)cd.xrid[idi].data)->u.keyButtonPointer.rootX = rootx;
    ((xEvent*)cd.xrid[idi].data)->u.keyButtonPointer.rootY = rooty;
    cd.compareproc[idi] = compare_motionevent;
    idi++;
    cd.xrid[idi].category = XRecordFromServer; /* Cursor error */
    cd.xrid[idi].client_swapped = client_swap;
    cd.xrid[idi].data_len = 8;
    cd.xrid[idi].data = protocol_buffer+64;
    ((xError*)cd.xrid[idi].data)->type = X_Error;
    ((xError*)cd.xrid[idi].data)->errorCode = BadCursor;
    ((xError*)cd.xrid[idi].data)->resourceID = 9;
    ((xError*)cd.xrid[idi].data)->minorCode = 0;
    ((xError*)cd.xrid[idi].data)->majorCode = X_FreeCursor;
    cd.compareproc[idi] = compare_error;
    idi++;
    make_clientdied(&cd, idi++, 0); /* id to be filled in later */
    make_eod(&cd, idi++);
    if (!XRecordEnableContextAsync(dpy, rc, protocol_element_callback,
			      (XPointer)&cd))
	report_error("");

    client3 = XOpenDisplay(display_env); /* the recorded client */
    c3id = XGContextFromGC(DefaultGC(client3, 0));
    /* fill in the common stuff */
    for (i=0; i<7; i++) {
	cd.xrid[i].id_base = c3id;
    }
    cd.xrid[0].id_base = 0;	/* start of data */
    cd.xrid[2].id_base = 0;	/* the expected device event */
    rootwin = XRootWindow(client3, 0);
    ((xEvent*)cd.xrid[3].data)->u.keyButtonPointer.root = rootwin;
    ((xEvent*)cd.xrid[2].data)->u.keyButtonPointer.root = rootwin;
    cd.xrid[6].id_base = 0;	/* end of data */
    XSelectInput(client3, XRootWindow(client3, 0), PointerMotionMask);
    XTestFakeMotionEvent(client3, 0, rootx, rooty, 0);
    /* ... which generates the expected delivered event */
    XSetErrorHandler(protocol_error_expected);
    XFreeCursor(client3, 9);	/* generate an error */
    XSync(client3, False);	/* get the error while we're expecting it */
    XSetErrorHandler(protocol_error_unexpected);
    XCloseDisplay(client3);
    if (!XRecordDisableContext(client2, rc))
	report_error("");
    XFlush(client2);
    wait_for_data_on(dpy, "");
    XCloseDisplay(client2);
    end_test();

    begin_test("multiple clients on single context");
    client2 = XOpenDisplay(display_env);
    client3 = XOpenDisplay(display_env);
    /* this will generate extra protocol, which shouldn't get recorded */
    XSynchronize(client2, True); /* desirable anyway for control ops */
    XSynchronize(client3, True);
    datadpy2 = XOpenDisplay(display_env);
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    c2id = XGContextFromGC(DefaultGC(client2, 0));
    c3id = XGContextFromGC(DefaultGC(client3, 0));
    /* same protocol set */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_noop_req(&cd, idi++, c3id, protocol_buffer);
    make_getscreensaver_req(&cd, idi++, c2id, protocol_buffer+8);
    make_getscreensaver_req(&cd, idi++, c3id, protocol_buffer+8);
    make_eod(&cd, idi++);
    enable_multi_client_context(dpy, datadpy2, client2, client3,
				rc, rc, &cd, &cd, True);
    /* different protocol set */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_getscreensaver_req(&cd, idi++, c3id, protocol_buffer+8);
    make_eod(&cd, idi++);
    enable_multi_client_context(dpy, datadpy2, client2, client3,
				rc, rc, &cd, &cd, False);
    if (!XRecordFreeContext(dpy, rc))
	report_error("");
    end_test();

    begin_test("single client on multiple contexts");
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    rc2 = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    /* what to expect: same protocol set */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_getscreensaver_req(&cd, idi++, c2id, protocol_buffer+8);
    make_getscreensaver_req(&cd, idi++, c2id, protocol_buffer+8);
    make_eod(&cd, idi++);
    idi = 0;
    make_sod(&cd2, idi++);
    make_noop_req(&cd2, idi++, c2id, protocol_buffer);
    make_noop_req(&cd2, idi++, c2id, protocol_buffer);
    make_getscreensaver_req(&cd2, idi++, c2id, protocol_buffer+8);
    make_getscreensaver_req(&cd2, idi++, c2id, protocol_buffer+8);
    make_eod(&cd2, idi++);
    enable_multi_client_context(dpy, datadpy2, client2, client2,
				rc, rc2, &cd, &cd2, True);

    /* what to expect: different protocol set */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_eod(&cd, idi++);
    idi = 0;
    make_sod(&cd2, idi++);
    make_getscreensaver_req(&cd2, idi++, c2id, protocol_buffer+8);
    make_getscreensaver_req(&cd2, idi++, c2id, protocol_buffer+8);
    make_eod(&cd2, idi++);
    enable_multi_client_context(dpy, datadpy2, client2, client2,
				rc, rc2, &cd, &cd2, False);

    if (!XRecordFreeContext(dpy, rc))
	report_error("");
    if (!XRecordFreeContext(dpy, rc2))
	report_error("");
    end_test();

    begin_test("multiple clients on multiple contexts");
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    rc2 = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    /* what to expect: same protocol set */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_getscreensaver_req(&cd, idi++, c2id, protocol_buffer+8);
    make_eod(&cd, idi++);
    idi = 0;
    make_sod(&cd2, idi++);
    make_noop_req(&cd2, idi++, c3id, protocol_buffer);
    make_getscreensaver_req(&cd2, idi++, c3id, protocol_buffer+8);
    make_eod(&cd2, idi++);
    enable_multi_client_context(dpy, datadpy2, client2, client3,
				rc, rc2, &cd, &cd2, True);

    /* what to expect: different protocol set */
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_eod(&cd, idi++);
    idi = 0;
    make_sod(&cd2, idi++);
    make_getscreensaver_req(&cd2, idi++, c3id, protocol_buffer+8);
    make_eod(&cd2, idi++);
    enable_multi_client_context(dpy, datadpy2, client2, client3,
				rc, rc2, &cd, &cd2, False);
    if (!XRecordFreeContext(dpy, rc))
	report_error("");
    if (!XRecordFreeContext(dpy, rc2))
	report_error("");
    XCloseDisplay(client2);
    XCloseDisplay(client3);
    XCloseDisplay(datadpy2);
    end_test();

    begin_test("requests on datadpy wait until Disable");
    client2 = XOpenDisplay(display_env);
    c2id = XGContextFromGC(DefaultGC(dpy, 0));
    c3id = XGContextFromGC(DefaultGC(client2, 0));
    clients[0] = c2id;
    clients[1] = c3id;		/* we shouldn't see anything from this */
    ranges[0]->core_requests.first = X_NoOperation;
    ranges[0]->core_requests.last = X_NoOperation;
    ranges[0]->ext_requests.ext_major.first = record_majorop;
    ranges[0]->ext_requests.ext_major.last = record_majorop;
    ranges[0]->ext_requests.ext_minor.last = X_RecordDisableContext;
    rc = XRecordCreateContext(dpy, 0, clients, 2, ranges, 1);
    if (!rc)
	report_error("Failed to create context");
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    cd.xrid[idi].id_base = CLIENT_BITS(c2id);
    cd.xrid[idi].category = XRecordFromClient;
    cd.xrid[idi].client_swapped = client_swap;
    cd.xrid[idi].data_len = 2;
    cd.compareproc[idi] = compare_noop;
    cd.xrid[idi].data = protocol_buffer+8;
    disable = (xRecordDisableContextReq *)cd.xrid[idi].data;
    disable->reqType = record_majorop;
    disable->recordReqType = X_RecordDisableContext;
    disable->length = 2;
    disable->context = rc;
    idi++;
    make_eod(&cd, idi++);
    s = XRecordEnableContextAsync(client2, rc, protocol_element_callback,
			      (XPointer)&cd);
    if (!s) report_error("XRecordEnableContextAsync returned False");

    /* these requests should NOT be executed now */
    XRecordDisableContext(client2, rc);
    XFreeCursor(client2, 0);	/* generate an error */
    XFlush(client2);

    /* these requests SHOULD be executed now */
    XSync(dpy, False);		/* if false error above, encourage it now */
    XNoOp(dpy);
    XRecordProcessReplies(client2);
    
    /* the error sent above should happen now */
    XSetErrorHandler(protocol_error_expected);
    XRecordDisableContext(dpy, rc);
    XSync(client2, False);	/* insist on getting the error now */
    if (protocol_error != BadCursor)
	report_error("BadCursor error arrived at wrong time");
    XSetErrorHandler(protocol_error_unexpected);
    XCloseDisplay(client2);
    end_test();

    XFree(ranges[0]);
    XFree(ranges[1]);

    end_test_set(dpy);
}

Display *enable_context_dpy;
XRecordContext enable_context_rc;

void sync_enable_alarm_handler(int ignored_signum)
{
    XNoOp(enable_context_dpy);
    if (!XRecordDisableContext(enable_context_dpy, enable_context_rc))
	report_error("");
    XFlush(enable_context_dpy);
}

void test_enable_context()	/* with synchronous library interface */
{
    Display *dpy = begin_test_set("EnableContext");
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    ComparisonData cd;
    int idi;
    XRecordClientSpec c2id;
    unsigned char protocol_buffer[20];

    ranges[0] = XRecordAllocRange();

    begin_test("synchronous enable");
    enable_context_dpy = XOpenDisplay(display_env);
    c2id = XGContextFromGC(DefaultGC(enable_context_dpy, 0));
    clients[0] = c2id;
    ranges[0]->core_requests.first = X_NoOperation;
    ranges[0]->core_requests.last = X_NoOperation;
    enable_context_rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);

    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, c2id, protocol_buffer);
    make_eod(&cd, idi++);

    signal(SIGALRM, sync_enable_alarm_handler);
    alarm(1);
    if (!XRecordEnableContext(dpy, enable_context_rc,
			 protocol_element_callback, (XPointer)&cd))
	report_error("");
    XCloseDisplay(enable_context_dpy);
    end_test();

    XFree(ranges[0]);
    end_test_set(dpy);
}

void test_disable_context()
{
    Display *dpy = begin_test_set("DisableContext");
    Display *datadpy;
    XRecordContext rc;
    ComparisonData cd;
    int idi;
    XRecordClientSpec cid;
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    unsigned char buffer[12];

    begin_test("disable disabled context");
    rc = XRecordCreateContext(dpy, 0, clients, 0, ranges, 0);
    /* nothing should happen */
    if (!XRecordDisableContext(dpy, rc))
	report_error("");
    if (!XRecordFreeContext(dpy, rc))
	report_error("");
    end_test();

    begin_test("DisableContext ends with EndOfData");
    datadpy = XOpenDisplay(display_env);
    cid = XGContextFromGC(DefaultGC(dpy, 0));
    clients[0] = cid;
    ranges[0] = XRecordAllocRange();
    ranges[0]->core_requests.first = X_NoOperation;
    ranges[0]->core_requests.last =  X_NoOperation;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    idi = 0;
    make_sod(&cd, idi++);
    make_noop_req(&cd, idi++, cid, buffer);
    if (!XRecordEnableContextAsync(datadpy, rc, protocol_element_callback,
				   (XPointer)&cd))
	report_error("");
    XNoOp(dpy);			/* should be recorded */
    XSync(dpy, False);
    XRecordProcessReplies(datadpy);
    /* still should only have gotten the StartOfData by now */
    /* now disable and expect the EndOfData */
    make_eod(&cd, 0);
    cd.data_index = 0;
    if (!XRecordDisableContext(dpy, rc)) report_error("");
    XSync(datadpy, False);	/* make sure nothing else comes back */
    cd.data_index = 0;
    XNoOp(dpy);			/* should no longer be recorded */
    XSync(dpy, False);
    XRecordProcessReplies(datadpy);
    if (!XRecordFreeContext(dpy, rc)) report_error("");
    XCloseDisplay(datadpy);
    XFree(ranges[0]);
    end_test();

    end_test_set(dpy);
}

void test_free_context()
{
    Display *dpy = begin_test_set("FreeContext");
    Display *datadpy;
    XRecordContext rc;
    ComparisonData cd;
    int idi;
    begin_test("invalid context id, expect Context error");
    XSetErrorHandler(protocol_error_expected);
    if (!XRecordFreeContext(dpy, 0) || (protocol_error != BadContext))
	report_error("");
    end_test();

    begin_test("freeing enabled context disables it");
    datadpy = XOpenDisplay(display_env);
    rc = XRecordCreateContext(dpy, 0, NULL, 0, NULL, 0);
    idi = 0;
    make_sod(&cd, idi++);
    make_eod(&cd, idi++);
    if (!XRecordEnableContextAsync(datadpy, rc, protocol_element_callback,
				   (XPointer)&cd))
	report_error("");
    if (!XRecordFreeContext(dpy, rc)) report_error("");
    /* we should be able to sync on data connection without hanging */
    XSync(datadpy, False);
    /* if disabled correctly, will see EOD */
    if (cd.data_index != 2)
	report_error("context not disabled by free");
    XCloseDisplay(datadpy);
    end_test();

    end_test_set(dpy);
}

/*
 * this callback does nothing.  In particular, it doesn't
 * always call XRecordFreeData.
 */
void do_nothing_callback(XPointer closure,
			 XRecordInterceptData *recorded_data)
{
    XRecordInterceptData **saved_data = (XRecordInterceptData **)closure;

    switch(recorded_data->category) {
    case XRecordFromClient:
	saved_data[0] = recorded_data;
	break;
    case XRecordFromServer:
	saved_data[1] = recorded_data;
	break;
    default:
	XRecordFreeData(recorded_data);
    }
}

void test_xrecord_free_data()	/* exercise library code */
{
    Display *dpy = begin_test_set("XRecordFreeData");
    Display *datadpy;
    XRecordContext rc;
    ComparisonData cd;
    int idi;
    XRecordClientSpec cid;
    XRecordClientSpec clients[3];
    XRecordRange *ranges[3];
    unsigned char buffer[64];
    XRecordInterceptData *intercept_data[2];
    int ig;

    begin_test("free data after closing display");
    datadpy = XOpenDisplay(display_env);
    cid = XGContextFromGC(DefaultGC(dpy, 0));
    clients[0] = cid;
    ranges[0] = XRecordAllocRange();
    ranges[0]->core_requests.first = X_GetScreenSaver;
    ranges[0]->core_requests.last =  X_GetScreenSaver;
    ranges[0]->core_replies.first = X_GetScreenSaver;
    ranges[0]->core_replies.last = X_GetScreenSaver;
    rc = XRecordCreateContext(dpy, 0, clients, 1, ranges, 1);
    idi = 0;
    make_sod(&cd, idi++);
    make_getscreensaver_req(&cd, idi++, cid, buffer);
    make_getscreensaver_reply(&cd, idi++, cid, buffer+32);
    make_eod(&cd, idi++);
    if (!XRecordEnableContextAsync(datadpy, rc, do_nothing_callback,
				   (XPointer)intercept_data))
	report_error("");
    XGetScreenSaver(dpy, &ig, &ig, &ig, &ig);
    if (!XRecordFreeContext(dpy, rc))
	report_error("");
    XCloseDisplay(datadpy);
    XFree(ranges[0]);
    XRecordFreeData(intercept_data[0]);
    XRecordFreeData(intercept_data[1]);
    end_test();

    end_test_set(dpy);
}



int main(argc, argv)
    int argc;
    char **argv;
{
    if (argc >= 2  &&  strcmp(argv[1], "-display") == 0)
	display_env = argv[2];

    test_query_version();	/* works */
    test_create_context();	/* works */
    test_register_clients();	/* works */
    test_unregister_clients();	/* works */
    test_get_context();		/* works */
    test_enable_context_async(); /* almost done */
    test_enable_context();	/* works */
    test_disable_context();	/* done */
    test_free_context();	/* works */
    test_xrecord_free_data();

    printf("RECORD total tests: %d,   total errors: %d,   errors/test: %f\n",
	   total_tests, total_error_count,
	   (float)total_error_count / total_tests);
    return total_error_count;
}
