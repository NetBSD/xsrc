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
 * $XConsortium: Expect.c,v 1.19 94/04/17 21:01:13 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include "XstlibInt.h"

#define Dont_Log_Twice(cl)	do {\
	int *countp = (Get_Test_Type(cl)==SETUP)?(&Xst_delete_count):(&Xst_error_count);\
	if (*countp>0) (*countp)--; } while (0)

extern int  errno;
extern int  sys_nerr;
extern char *sys_errlist[];

static int  max_extra = IBUFSIZE - sizeof (xReply);
static char rbuf[IBUFSIZE];
static char *rbp;
static char *enames ();
static char wanted[132];
static char *got;

static  xGenericReply dummy_reply = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char *expect_names[5] = {
    "REPLY",
    "ERROR",
    "EVENT",
    "NOTHING",
    "EVENTorNOTHING"
};

static char emsg[132];

static int this_client;

static  void
Timeout_Func () {
    Log_Msg ("Expect: wanted %s, got TIMEOUT! (server may be dead)\n", wanted);
    Finish (this_client);
}

static  void
Enable_Timeout (client)
int client;
{
    this_client = client;
    Set_Timer (EXPECT_TIMER_ID, Xst_timeout_value, Timeout_Func);
}

static  void
Disable_Timeout () {
    Stop_Timer (EXPECT_TIMER_ID);
}

/*
 * 	Routine: Expect_BadAccess()
 *	Call Expect_Error expecting BadAccess error.
 */
void
Expect_BadAccess(client)
int	client;
{
	xError *err;

	if ((err = Expect_Error(client, BadAccess)) == NULL) {
		Dont_Log_Twice(client);
		Log_Msg("client %d failed to receive Access error\n", client);
		Finish(client);
	}  else  {
		Log_Trace("client %d received Access error\n", client);
		Free_Error(err);
	}
}


/*
 * 	Routine: Expect_BadValue()
 *	Call Expect_Error expecting BadValue error.
 */
void
Expect_BadValue(client)
int	client;
{
	xError *err;

        if ((err = Expect_Error(client, BadValue)) == NULL) {
		Dont_Log_Twice(client);
	        Log_Msg("client %d failed to recv BadValue error\n", client);
		Finish(client);
	}  else  {
		Log_Trace("client %d received BadValue error\n", client);
		Free_Error(err);
	}
}

/*
 * 	Routine: Expect_BadLength()
 *	Call Expect_Error expecting BadLength error.
 */
void
Expect_BadLength(client)
int	client;
{
	xError *err;

        if ((err = Expect_Error(client, BadLength)) == NULL) {
		Dont_Log_Twice(client);
	        Log_Msg("client %d failed to recv BadLength error\n", client);
		Finish(client);
	}  else  {
		Log_Trace("client %d received BadLength error\n", client);
		Free_Error(err);
	}
}

/*
 * 	Routine: Expect_BadIDChoice()
 *	Call Expect_Error expecting BadIDChoice error.
 */
void
Expect_BadIDChoice(client)
int	client;
{
	xError *err;

        if ((err = Expect_Error(client, BadIDChoice)) == NULL) {
		Dont_Log_Twice(client);
	        Log_Msg("client %d failed to recv BadIDChoice error\n", client);
		Finish(client);
	}  else  {
		Log_Trace("client %d received BadIDChoice error\n", client);
		Free_Error(err);
	}
}

/*
 *	Routine: Expect - extract a response of the specified type and class 
 *      from the server
 *
 *	Input: client - integer between 0 and MAX_CLIENTS
 *             class -  one of {EXPECT_ERROR, EXPECT_EVENT, EXPECT_REPLY,
 *      			EXPECT_NOTHING, EXPECT_01EVENT}
 *	       type -   protocol type preceded by X_, e.g. X_GrabPointer
 *
 *	Output: response if found, else null
 *
 *	Returns:
 *
 *	Globals used: Xst_clients
 *
 *	Side Effects:
 *
 *	Methods:
 *
 */
#define THRESHOLD	42	/* number of unexpected, malformed or out
				 * of sequence replies/events/errors we will
				 * put up with.
				 */

xReply
* Expect (client, class, type)
int     client;   /* client number */
int     class;    /* expected class, e.g. event, error */
int     type;     /* request type */
{
    XstDisplay * dpy = Get_Display (client);
    xReply * rep = (xReply *) Xstmalloc (sizeof (xReply));
    xReply * match = NULL;
    int     done = 0;
    int     strange = 0;
    int     done_reason = EXPECT_NOTHING;
    unsigned long   extra;
    long so_far;
    int needswap = Xst_clients[client].cl_swap;
    void (*Log_Rtn)();

    /* use if-else rather than ?: to init. Log_Rtn to avoid Sequent cc bug */
    if (Get_Test_Type(client)==SETUP)
	Log_Rtn = Log_Del;
    else
	Log_Rtn = Log_Err;

    strcpy (wanted, enames (class, type));
/*
 *	If a request has gone out since the last poll, need to update
 *	our notion of "outstanding request"
 */
    if (Xst_clients[client].cl_pollout < dpy -> request) {
	Xst_clients[client].cl_reqout = dpy -> request;
    }
/*
 *	If we don't have a poll currently outstanding, send a poll
 *	to ensure that we hear from the server.
 */
    if (Xst_clients[client].cl_pollout == 0) {
	Poll_Server (client);
    }

/*
 *	Now cycle through the messages coming back until we get something
 *	which corresponds explicitly to the last request sent
 *	(ie. either a reply or an error), or a poll reply.
 *	
 *	Replies and errors for PAST or FUTURE requests are
 *	error-logged and discarded. (ie. sequence number error)
 *
 *	Replies or errors which explicitly correspond to the current
 *	request (ie. sequence numbers match) cause a return to the caller,
 *	with success/failure a function of what was "expected."
 *
 *	Events are checked for "expectedness". If an expected event is
 *	received, it is immediately returned to the caller. If an
 *	unexpected event is received, it is error-logged and discarded.
 *
 *	Poll replies which respond to polls prior to sending the current
 *	request are discarded. Poll replies which respond to polls after
 *	the current request cause a return to caller, with appropriate
 *	success/failure.
 *
 *	Timeouts on reads are always logged, and the test terminated.
 *
 *	Truncated reads are always logged, and the test terminated.
 *
 *	System errors on reads are always logged, and the test terminated.
 *
 *	Poll replies which appear malformed, or which are errors, are always
 *	logged, and the test terminated.
 *
 *	Any reply/error/event indicating a length greater than ???? is
 *	logged, and the test terminated.
 *
 */

    while (!done && strange < THRESHOLD) {

	Get_Me_That (client, rbuf, so_far = (long) sizeof (xReply));
/*
 *	unpack the constant header 
 */
	rbp = rbuf;

	rep->generic.type = unpack1(&rbp);
	rep->generic.data1 = unpack1(&rbp);
	rep->generic.sequenceNumber = unpack2(&rbp,needswap);
	rep->generic.length = unpack4(&rbp,needswap);

/* 
 *	Check for a poll reply
 */
	if ((Xst_clients[client].cl_pollout == rep -> generic.sequenceNumber) &&
	    (rep->generic.type == X_Reply)) {
/*
 *	Looks like a poll reply - check it out in more detail
 */
	    Rcv_Poll (rep,rbuf, client);
	    if (Xst_clients[client].cl_pollout < Xst_clients[client].cl_reqout) {
		Poll_Server (client); /* response to old poll, send another */
	    }
	    else {
		Xst_clients[client].cl_pollout = 0;
		done = 1;	/* reponse to current poll */
		done_reason = EXPECT_NOTHING;
	    }

	    continue;		/* in any case, no further interest in
				   this */
	}
/*
 *	check type of reply
 */

	switch (rep -> generic.type) {
	    case X_Reply: 
		extra = (rep -> generic.length << 2);
		if (extra > 0) {/* more to read */
		    if (extra > max_extra) {
			Log_Msg ("Expect: too big a reply");
			Show_Rep (rep, UNKNOWN_REQUEST_TYPE, so_far);
			Finish  (client);
		    }
		    so_far = (long) extra + sizeof (xReply);
		    rep = (xReply *) Xstrealloc ((char *) rep, (unsigned) so_far);
		    Get_Me_That (client, (char *) rbuf + sizeof (xReply), extra);
		}
/*
 *	Now the whole thing is in, lets look at it
 */
		if (!Rcv_Rep(rep,rbuf,Xst_clients[client].cl_reqtype,client)) {
		    match = NULL;
		    continue;
		}
		if (rep -> generic.sequenceNumber < (CARD16) Xst_clients[client].cl_reqout) {
		    (*Log_Rtn) ("Expect: reply for PAST request\n");
		    Show_Rep ((xReply *) rep, UNKNOWN_REQUEST_TYPE, so_far);
		    strange++;
		    continue;
		}
		if (rep -> generic.sequenceNumber > (CARD16)Xst_clients[client].cl_reqout) {
		    (*Log_Rtn) ("Expect: reply for FUTURE request\n");
		    Show_Rep ((xReply *) rep, UNKNOWN_REQUEST_TYPE, so_far);
		    strange++;
		    continue;
		}
/*
 *	this is a reply for the outstanding message
 */
		Log_Debug("Received reply:");
		Show_Rep(rep,type, so_far);
		done = 1;
		done_reason = EXPECT_REPLY;
		match = rep;
		break;
	    case X_Error: 
		if(!Rcv_Err(rep,rbuf,client)) {
		    continue;
		}
		if (rep -> generic.sequenceNumber < (CARD16)Xst_clients[client].cl_reqout) {
		    (*Log_Rtn) ("Expect: error for PAST request\n");
		    Show_Err ((xError *) rep);
		    strange++;
		    continue;
		}
		if (rep -> generic.sequenceNumber > (CARD16)Xst_clients[client].cl_reqout) {
		    (*Log_Rtn) ("Expect: error for FUTURE request\n");
		    Show_Err ((xError *) rep);
		    strange++;
		    continue;
		}
/*
 *	this is an error for the outstanding message
 */
		Log_Debug("Received error:");
		Show_Err(rep);
		done = 1;
		done_reason = EXPECT_ERROR;
		match = rep;
		break;
/*
 *	if its not a reply or an error, its an event!
 */
	    default: 
		if(!Rcv_Evt(rep,rbuf,client)) {
		    strange++;
		    continue;
		}
		if (real_type(rep -> event.u.u.type) != KeymapNotify) {
		    if (rep -> generic.sequenceNumber < (CARD16)Xst_clients[client].cl_reqout) {
			(*Log_Rtn) ("Expect: event for PAST request\n");
			Show_Evt ((xEvent *) rep);
			strange++;
			continue;
		    }
		    if (rep -> generic.sequenceNumber > (CARD16)Xst_clients[client].cl_reqout) {
			(*Log_Rtn) ("Expect: event for FUTURE request\n");
			Show_Evt ((xEvent *) rep);
			strange++;
			continue;
		    }
		}
		Log_Debug("Received event:");
		Show_Evt(rep);
		if ((class == EXPECT_EVENT || class == EXPECT_01EVENT) &&
			(real_type(rep -> event.u.u.type) == type)) {
		    done = 1;
		    done_reason = EXPECT_EVENT;
		    match = rep;
		    continue;
		}
		got = enames (EXPECT_EVENT, (int) real_type(rep -> event.u.u.type));
		(*Log_Rtn) ("Expect: wanted %s, got %s\n", wanted, got);
		Show_Evt ((xEvent *) rep);
		break;
	}
    }
    if (strange >= THRESHOLD) {
	    /* we'll have got here with the strange count exceeded, probably
	     * due to sequence errors or replies to requests we don't think
	     * think we sent.
	     */
	    (*Log_Rtn) ("Expect: wanted %s but got at least %d unexpected, malformed or out of sequence replies/errors/events.\n",
		wanted, strange);
	    return (NULL);
    }
    switch (done_reason) {
	case EXPECT_REPLY: 
	    if (class == EXPECT_REPLY) {
		return (match);	/* wanted one, got one */
	    }
            got = enames (EXPECT_REPLY, Xst_clients[client].cl_reqtype  |
		(Xst_clients[client].cl_minor << 8));
	    (*Log_Rtn) ("Expect: wanted %s, got %s\n", wanted, got);
	    Show_Rep (rep, UNKNOWN_REQUEST_TYPE, so_far);
	    return (NULL);
	case EXPECT_NOTHING: 
	    if (class == EXPECT_NOTHING) {
		return ((xReply *) & dummy_reply);
	    }
	    if (class == EXPECT_01EVENT)
		return ((xReply *) NULL);
	    got = enames (EXPECT_NOTHING, 0);
	    (*Log_Rtn) ("Expect: wanted %s, got %s\n", wanted, got);
	    return (NULL);
	case EXPECT_ERROR: 
	    if (class != EXPECT_ERROR) {
		got = enames (EXPECT_ERROR, (int) match -> error.errorCode);
		(*Log_Rtn) ("Expect: wanted %s, got %s\n", wanted, got);
		Show_Err ((xError *) rep);
		return (NULL);
	    }
	    if (type != match -> error.errorCode) {
		got = enames (EXPECT_ERROR, (int) match -> error.errorCode);
		(*Log_Rtn) ("Expect: wanted %s, got %s\n", wanted, got);
		Show_Err ((xError *) rep);
		return (NULL);
	    }
	    return (match);
	case EXPECT_EVENT: 
	/* the only way the "done_reason" would be EXPECT_EVENT is * if
	   we've already done the match */
	    return (match);
	default: 
	    break;
    }
    return ((xReply *) NULL);
}


Get_Me_That (client, rbuf, size)
int     client;
char * rbuf;
unsigned long   size;
{
    XstDisplay * dpy = Get_Display (client);
    int     this_read;

    Enable_Timeout (client);
    while ((this_read = Xst_Read (dpy, (char *) rbuf, size)) < 0) {
	if (errno == EINTR) {
	    continue;
	}
	else {
	    if ((errno >= 0) && (errno < sys_nerr)) {
		(void) strcpy (emsg, sys_errlist[errno]);
	    }
	    else {
		sprintf (emsg, "UNKNOWN ERROR - %d", errno);
	    }
	    Log_Msg ("Expect: wanted %s, got SYSTEM ERROR - %s\n", wanted, emsg);
	    Finish (client);
	}
    }
    if (this_read < size) {
	Log_Msg ("Expect: wanted %s, got TRUNCATED\n", wanted);
	Log_Msg ("Expect: wanted %d additional, got %d\n",
		size, this_read);
	Show_Rep ((xReply *) rbuf, Xst_clients[client].cl_reqtype, (long)this_read);
	Finish (client);
    }
    Disable_Timeout ();
    return;
}

Poll_Server (client)
int     client;
{
    XstDisplay * dpy = Get_Display (client);
    xReq * req;
    int     type;
    TestType tt = Get_Test_Type(client);

    switch (tt) {
	case BAD_LENGTH:
	case JUST_TOO_LONG:
	case TOO_LONG:
		Set_Test_Type(client, GOOD);
		break;
    }
    req = Make_Req (client, X_GetInputFocus);
    _Send_Req (client, req,1);	/* tell 'em we're a poll */
    Xst_clients[client].cl_pollout = dpy -> request;
    Free_Req (req);
    Set_Test_Type(client, tt);
}

static char
           *enames (class, type)
int     class;
int     type;
{
    static char prtbuf[132];
    char *(*namefunc)();
    extern char *errorname();
    extern char *eventname();
    extern char *protoname();

    switch (class) {
    case EXPECT_REPLY:
	namefunc = protoname;
	break;
    case EXPECT_ERROR:
	namefunc = errorname;
	break;
    case EXPECT_EVENT:
    case EXPECT_01EVENT:
	namefunc = eventname;
	break;
    case EXPECT_NOTHING:
	sprintf (prtbuf, "%s", expect_names[class]);
	return prtbuf;
	/*NOTREACHED*/
	break;
    default:
	Log_Msg("INTERNAL ERROR: enames(%d,%d) - first arg not one of {%d,%d,%d,%d}\n",
		class, type, EXPECT_REPLY, EXPECT_ERROR, EXPECT_EVENT,
		EXPECT_NOTHING);
	Delete ();
	/*NOTREACHED*/
	break;
    }
    sprintf (prtbuf, "%s - %s ", expect_names[class], (*namefunc)(type));
    return (prtbuf);
}

int     Rcv_Poll (rep, rbuf, client)
        xReply * rep;
	char rbuf[];
	int client;
{
    return(Rcv_Rep(rep,rbuf, X_GetInputFocus, client));
}
