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
 * $XConsortium: XlibNoXtst.c,v 1.9 94/04/17 21:01:36 rws Exp $
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

/*
 * THIS IS AN OS DEPENDENT FILE! It should work on 4.2BSD derived
 * systems.  VMS and System V should plan to have their own version.
 * More portable functionality is available in XlibWithXTest.c if your
 * Xlib has the post R5 release patches to move auth/conn handling from
 * XOpenDis.c into XConDis.c and hence returned by _XConnectDisplay().
 * If not you can portably do client-native only testing with XlibOpaque.c
 * which uses XOpenDisplay to make the connection and then ConnectionNumber()
 * to get the fd. This route is also appropriate if your Xlib has a different
 * internal interface to the MIT release or else you don't have source.
 * See the documentation of the build parameter XP_OPEN_DIS for more details.
 */
#include <stdio.h>
#include <string.h>
#include "XstlibInt.h"
#include "XstosInt.h"
#include "DataMove.h"
#include <fcntl.h>
#include <sys/file.h>
#include <sys/socket.h>


#ifdef UNIXCONN
#include <sys/un.h>
#define X_UNIX_PATH "/tmp/.X11-unix/X"
#endif /* UNIXCONN */

/* time_proc used to set timeout routines and on error in Xst_EINTR_Read */
static void	(*time_proc)() = 0;

#define Xst_EINTR_Read(dpy, buffer, len) \
    while (errno=0, *(char *)buffer=xFalse, 0 > Xst_Read (dpy, (char *) buffer, (long) len)) \
	if (errno != EINTR) { \
		if (time_proc) \
			(*time_proc)(); \
		else { \
			Log_Msg ("read failed with errno = %d\n", errno); \
			Delete(); \
		} \
		/*NOTREACHED*/ \
	}

/* 
 * Attempts to connect to server, given display name. Returns file descriptor
 * (network socket) or -1 if connection fails. The expanded display name
 * of the form hostname:number.screen ("::" if DECnet) is returned in a result
 * parameter. The screen number to use is also returned.
 */
int XstConnectDisplay (display_name, expanded_name, screen_num,
		       auth_proto, auth_length, auth_string, auth_strlen,
		       xlib_dpy)
    char *display_name;
    char *expanded_name;	/* return */
    int *screen_num;		/* return */
    char **auth_proto;		/* return */
    int *auth_length;		/* return */
    char **auth_string;		/* return */
    int *auth_strlen;		/* return */
    Display **xlib_dpy;		/* return */
{
	char displaybuf[256];		/* Display string buffer */	
	register char *display_ptr;	/* Display string buffer pointer */
	register char *numbuf_ptr;	/* Server number buffer pointer */
	char *screen_ptr;		/* Pointer for locating screen num */
	int display_num;		/* Display number */
	struct sockaddr_in inaddr;	/* INET socket address. */
#ifdef UNIXCONN
	struct sockaddr_un unaddr;	/* UNIX socket address. */
#endif
	struct sockaddr *addr;		/* address to connect to */
        struct hostent *host_ptr;
	int addrlen;			/* length of address */
	extern char *getenv();
	extern struct hostent *gethostbyname();
        int fd;				/* Network socket */
	char numberbuf[16];
	char *dot_ptr = NULL;		/* Pointer to . before screen num */
#ifdef DNETCONN
	int dnet = 0;
	char objname[20];
	extern int dnet_conn();
#endif

	*auth_proto = "";
	*auth_length = 0;
	*auth_string = "";
	*auth_strlen = 0;
	*xlib_dpy = (Display *)NULL;
	/* 
	 * Find the ':' seperator and extract the hostname and the
	 * display number.
	 * NOTE - if DECnet is to be used, the display name is formatted
	 * as "host::number"
	 */
	(void) strncpy(displaybuf, display_name, sizeof(displaybuf));
	if ((display_ptr = SearchString(displaybuf,':')) == (char *)NULL) 
		return (-1);
#ifdef DNETCONN
	if (*(display_ptr + 1) == ':') {
	    dnet++;
	    *(display_ptr++) = '\0';
	}
#endif
	*(display_ptr++) = '\0';
 
	/* displaybuf now contains only a null-terminated host name, and
	 * display_ptr points to the display number.
	 * If the display number is missing there is an error. */

	if (*display_ptr == '\0') return(-1);

	/*
	 * Build a string of the form <display-number>.<screen-number> in
	 * numberbuf, using ".0" as the default.
	 */
	screen_ptr = display_ptr;
	numbuf_ptr = numberbuf;
	while (*screen_ptr != '\0') {
	    if (*screen_ptr == '.') {
		dot_ptr = numbuf_ptr;
		*(screen_ptr++) = '\0';
		*(numbuf_ptr++) = '.';
	    } else {
		*(numbuf_ptr++) = *(screen_ptr++);
	    }
	}

	/*
	 * If the spec doesn't include a screen number, add ".0" (or "0" if
	 * only "." is present.)
	 */
	if (dot_ptr == NULL) {
	    dot_ptr = numbuf_ptr;
	    *(numbuf_ptr++) = '.';
	    *(numbuf_ptr++) = '0';
	} else {
	    if (*(numbuf_ptr - 1) == '.')
		*(numbuf_ptr++) = '0';
	}
	*numbuf_ptr = '\0';

	/*
	 * Return the screen number in the result parameter
	 */
	*screen_num = atoi(dot_ptr + 1);

	/*
	 * Convert the server number string to an integer.
	 */
	display_num = atoi(display_ptr);

	/*
	 * If the display name is missing, use current host.
	 */
	if (displaybuf[0] == '\0')
#ifdef DNETCONN
	    if (dnet) 
		(void) strcpy (displaybuf, "0");
            else
#endif
#ifdef UNIXCONN
		;	/* Do nothing if UNIX DOMAIN. Will be handled below. */
#else
		(void) gethostname (displaybuf, sizeof(displaybuf));
#endif

#ifdef DNETCONN
	if (dnet) {
	    /*
	     * build the target object name.
	     */
	    sprintf(objname, "X%d", display_num);
	    /*
	     * Attempt to open the DECnet connection, return -1 if fails.
	     */
	    if ((fd = dnet_conn(displaybuf, 
		   objname, SOCK_STREAM, 0, 0, 0, 0)) < 0)
		return(-1);	    /* errno set by dnet_conn. */
	} else
#endif
	{
#ifdef UNIXCONN
	    if ((displaybuf[0] == '\0') || 
		(strcmp("unix", displaybuf) == 0)) {
		/* Connect locally using Unix domain. */
		unaddr.sun_family = AF_UNIX;
		(void) strcpy(unaddr.sun_path, X_UNIX_PATH);
		strcat(unaddr.sun_path, display_ptr);
		addr = (struct sockaddr *) &unaddr;
		addrlen = strlen(unaddr.sun_path) + 2;
		/*
		 * Open the network connection.
	 	 */
	        if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0)
		    return(-1);	    /* errno set by system call. */
	    } else
#endif
	    {
		/* Get the statistics on the specified host. */
		if ((inaddr.sin_addr.s_addr = inet_addr(displaybuf)) == -1) {
			if ((host_ptr = gethostbyname(displaybuf)) == NULL) {
				/* No such host! */
				errno = EINVAL;
				return(-1);
			}
			/* Check the address type for an internet host. */
			if (host_ptr->h_addrtype != AF_INET) {
				/* Not an Internet host! */
				errno = EPROTOTYPE;
				return(-1);
			}
 
			/* Set up the socket data. */
			inaddr.sin_family = host_ptr->h_addrtype;
			bcopy((char *)host_ptr->h_addr, 
			      (char *)&inaddr.sin_addr, 
			      sizeof(inaddr.sin_addr));
		} else {
			inaddr.sin_family = AF_INET;
		}
		addr = (struct sockaddr *) &inaddr;
		addrlen = sizeof (struct sockaddr_in);
		inaddr.sin_port = display_num;
		inaddr.sin_port += X_TCP_PORT;
		inaddr.sin_port = htons(inaddr.sin_port);
		/*
		 * Open the network connection.
		 */

		if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0)
		    return(-1);	    /* errno set by system call. */
		/* make sure to turn off TCP coalescence */
#ifdef TCP_NODELAY
		{
		int mi;
		setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &mi, sizeof (int));
		}
#endif
	    }
 

	    if (connect(fd, addr, addrlen) == -1) {
		(void) close (fd);
		return(-1); 	    /* errno set by system call. */
	    }
#ifdef FIOSNBIO
		{
		    int arg = 1;
		    ioctl (fd, FIOSNBIO, &arg);
		}
#else
		(void) fcntl (fd, F_SETFL, FNDELAY);
#endif /* FIOSNBIO */

        }
	/*
	 * Return the id if the connection succeeded. Rebuild the expanded
	 * spec and return it in the result parameter.
	 */
	display_ptr = displaybuf;
	while (*(++display_ptr) != '\0')
	    ;
	*(display_ptr++) = ':';
#ifdef DNETCONN
	if (dnet)
	    *(display_ptr++) = ':';
#endif
	numbuf_ptr = numberbuf;
	while (*numbuf_ptr != '\0')
	    *(display_ptr++) = *(numbuf_ptr++);
	*display_ptr = '\0';
	(void) strcpy(expanded_name, displaybuf);
	return(fd);
}

/* 
 * Disconnect from server.
 */

int XstDisconnectDisplay (server)

    int server;

{
    (void) close(server);
}

#undef NULL
#define NULL ((char *) 0)

_XstWaitForReadable(dpy)
  XstDisplay *dpy;
{
    int r_mask[MSKCNT];
    int result;
	
    CLEARBITS(r_mask);
    do {
	BITSET(r_mask, dpy->fd);
	result = select(dpy->fd + 1, r_mask, (int *) 0, (int *) 0, (struct timeval *)NULL);
	if (result == -1 && errno != EINTR) {
	    XstIOError(dpy,"_XstWaitForReadable",1);
	}
    } while (result <= 0);
}

static unsigned int padlength[4] = {0, 3, 2, 1};

XstSendClientPrefix (dpy, client, auth_proto, auth_string, needswap)
     XstDisplay *dpy;
     xConnClientPrefix *client;
     int needswap;
{
	/*
	 * Authorization string stuff....  Must always transmit multiple of 4
	 * bytes.
	 */

	int auth_length, auth_strlen;
	char pad[3];
	char buffer[BUFSIZ], *bptr;

        int bytes=0;

        auth_length = client->nbytesAuthProto;
        auth_strlen = client->nbytesAuthString;

	bytes = (sizeof(xConnClientPrefix) + 
                       auth_length + padlength[auth_length & 3] +
                       auth_strlen + padlength[auth_strlen & 3]);

	/* bcopy(client, buffer, sizeof(xConnClientPrefix)); */
	bptr = buffer;
	BPRINTF1 ("OpenDisplay message:\n");
	pack1(&bptr,client->byteOrder);
	BPRINTF2 ("\tbyteOrder = 0x%x\n", (unsigned) client->byteOrder);
	packpad(&bptr,sizeof(client->pad));
	BPRINTF2 ("\tpad = %d\n", (int) *(bptr-1));
	pack2(&bptr,(short)client->majorVersion,needswap);
	BPRINTF2 ("\tmajorVersion = %d\n", client->majorVersion);
	pack2(&bptr,(short)client->minorVersion,needswap);
	BPRINTF2 ("\tminorVersion = %d\n", client->minorVersion);
	pack2(&bptr,(short)client->nbytesAuthProto,needswap);
	BPRINTF2 ("\tnbytesAuthProto = %d\n", client->nbytesAuthProto);
	pack2(&bptr,(short)client->nbytesAuthString,needswap);
	BPRINTF2 ("\tnbytesAuthString = %d\n", client->nbytesAuthString);
	packpad(&bptr,sizeof(client->pad2));
	BPRINTF2 ("\tpad2 = %d\n", (int) *(bptr-1));

        /* bptr = buffer + sizeof(xConnClientPrefix); */
	BPRINTF2 ("\tAuthProtoName = %d bytes\n", auth_length);
        if (auth_length)
	{
	    bcopy(auth_proto, bptr, auth_length);
            bptr += auth_length;
            if (padlength[auth_length & 3])
	    {
		bcopy(pad, bptr, padlength[auth_length & 3]);
	        bptr += padlength[auth_length & 3];
		BPRINTF2 ("\tAuthProtoName pad = %d bytes\n", padlength[auth_length & 3]);
	    }
	}
	BPRINTF2 ("\tAuthProtoData = %d bytes\n", auth_strlen);
        if (auth_strlen)
	{
	    bcopy(auth_string, bptr, auth_strlen);
            bptr += auth_strlen;
            if (padlength[auth_strlen & 3])
	    {
		bcopy(pad, bptr, padlength[auth_strlen & 3]);
	        bptr += padlength[auth_strlen & 3];
		BPRINTF2 ("\tAuthProtoData pad = %d bytes\n", padlength[auth_strlen & 3]);
	    }
	}
	BPRINTF2 ("\tTotal OpenDisplay message length = %d bytes\n", bytes);
	BPRINTF2 ("\t\ton fd %d\n", dpy->fd);
	BPRINTF2 ("\t\t%d bytes used of buffer\n", bptr - buffer);
	(void) WriteToServer(dpy->fd, buffer, bytes);
	return;
}

/*
 * The Timeout function(s) must all exit -- we don't want to get
 * into longjmp from out of signal (SIGALARM) handlers etc. So
 * we have to use the test type and request type to work out
 * what messages to put out and what exit code to give back to the TET.
 *
 * Behaviour is as follows:
 *
 * Request Type: OPEN_DISPLAY_REQUEST_TYPE    || anything else
 * Test Type: OPEN_DISPLAY   |  anything else || anything at all
 * __________(bad byte order)| (0x42 or 0x6C) || (0x42 or 0x6C)
 * Getting:  \---------------+----------------++------------------
 * 	      |		     |		      ||
 * SetupPrefix|	PASS(Exit_OK)|  FAIL(Abort)   || Delete
 * 	      |		     |		      ||
 * SetupData  |	Not Reached  |  FAIL(Abort)   || Delete
 *	      | (Delete)     |		      ||
 *
 */


#define		PASS_action	1
#define		FAIL_action	2
#define		DELETE_action	3

static char *nothing = "No reply from server when trying to connect to %s\n";

static void
Timeout_Func(action)
int action;
{
    char *server = Xst_server_node == NULL ? "Default Server" : Xst_server_node;

    switch(action) {
    case PASS_action:
	Log_Trace("No prefix sent in response to bad byte order open request.");
	Exit_OK ();
	/*NOTREACHED*/
	break;
    case FAIL_action:
	Log_Msg(nothing, server);
	Abort ();
	/*NOTREACHED*/
	break;
    case DELETE_action:
	Log_Msg(nothing, server);
	Delete ();
	/*NOTREACHED*/
	break;
    default:
	Log_Msg("INTERNAL TEST SUITE ERROR: bad action (%d) in Timeout_Func with server %s.", action, server);
	Delete ();
	/*NOTREACHED*/
	break;
    }
}

static void
Normal_Timeout_Func() {
    Timeout_Func(DELETE_action);
}

static void
Good_Open_Timeout_Func() {
    Timeout_Func(FAIL_action);
}

static void
Bad_Open_Timeout_Func() {
    Timeout_Func(PASS_action);
}

GetConnSetupPrefix (client, prefixp, needswap)
int client;
xConnSetupPrefix * prefixp;
int     needswap;
{
    XstDisplay * dpy;
    char    buffer[OBUFSIZE];
    char   *bptr;

    dpy = Get_Display(client);

    if (Get_Req_Type(client) == OPEN_DISPLAY_REQUEST_TYPE) {
	if (Get_Test_Type(client) == OPEN_DISPLAY)
		time_proc = Bad_Open_Timeout_Func;
	else
		time_proc = Good_Open_Timeout_Func;
    } else
	time_proc = Normal_Timeout_Func;

    Set_Timer (CONNECT_TIMER_ID, Xst_timeout_value, time_proc);

    Xst_EINTR_Read (dpy, (char *) buffer, (long) sizeof (xConnSetupPrefix));

    Stop_Timer (CONNECT_TIMER_ID);

    BPRINTF1 ("Connection setup prefix:\n");
    bptr = buffer;
    prefixp -> success = unpack1 (&bptr);
    BPRINTF2 ("\tsuccess = %s\n", boolname(prefixp->success));
    prefixp -> lengthReason = unpack1 (&bptr);
    BPRINTF2 ("\tlengthReason = %d\n", prefixp->lengthReason);
    prefixp -> majorVersion = unpack2 (&bptr, needswap);
    BPRINTF2 ("\tmajorVersion = %d\n", prefixp->majorVersion);
    prefixp -> minorVersion = unpack2 (&bptr, needswap);
    BPRINTF2 ("\tminorVersion = %d\n", prefixp->minorVersion);
    prefixp -> length = unpack2 (&bptr, needswap);
    BPRINTF2 ("\tlength = %d\n", prefixp->length);
}

/* 
 *	GetConnSetupData - reads & byte swaps (as appropriate)
 *		the rest of the connection setup data
 *		(Note this is somewhat redundant with logic
 *		in XOpenDisplay - might merge later)
 */

GetConnSetupData (client, setupdp, len, needswap)
int client;
xConnSetup * setupdp;
int     len;
int     needswap;
{
    XstDisplay * dpy;
    char    buffer[OBUFSIZE];
    char   *bptr;
    char   *sptr;		/* pointer into setup data area */
    int     pad;
    int i;
    int d;
    int s;
    int v;
    int f;
    int ndepths;
    int nvisuals;

    dpy = Get_Display(client);

    if (Get_Req_Type(client) == OPEN_DISPLAY_REQUEST_TYPE) {
	if (Get_Test_Type(client) == OPEN_DISPLAY) {
		Log_Msg ("INTERNAL ERROR: should not be getting SetupData with TestType == OPEN_DISPLAY.");
		Delete();
		/*NOTREACHED*/
	}
	time_proc = Good_Open_Timeout_Func;
    } else
	time_proc = Normal_Timeout_Func;

    Set_Timer (CONNECT_TIMER_ID, Xst_timeout_value, time_proc);

    if (!needswap) {
	Xst_EINTR_Read (dpy, (char *) setupdp, len);
	Stop_Timer (CONNECT_TIMER_ID);
	return;
    }
    else {
	Xst_EINTR_Read (dpy, (char *) buffer, len);
	Stop_Timer (CONNECT_TIMER_ID);
	bptr = buffer;

	setupdp -> release = unpack4 (&bptr, needswap);
	setupdp -> ridBase = unpack4 (&bptr, needswap);
	setupdp -> ridMask = unpack4 (&bptr, needswap);
	setupdp -> motionBufferSize = unpack4 (&bptr, needswap);
	setupdp -> nbytesVendor = unpack2 (&bptr, needswap);
	setupdp -> maxRequestSize = unpack2 (&bptr, needswap);
	setupdp -> numRoots = unpack1 (&bptr);
	setupdp -> numFormats = unpack1 (&bptr);
	setupdp -> imageByteOrder = unpack1 (&bptr);
	setupdp -> bitmapBitOrder = unpack1 (&bptr);
	setupdp -> bitmapScanlineUnit = unpack1 (&bptr);
	setupdp -> bitmapScanlinePad = unpack1 (&bptr);
	setupdp -> minKeyCode = unpack1 (&bptr);
	setupdp -> maxKeyCode = unpack1 (&bptr);
	setupdp -> pad2 = unpack4 (&bptr, needswap);

	sptr = (char *) (setupdp + 1);

/*	get the vendor string */
	bcopy (bptr, sptr, setupdp -> nbytesVendor);
	pad = (setupdp -> nbytesVendor + 3) & ~3;
	bptr += pad;
	sptr += pad;

 /* Z axis screen format info */
 /* NOTE - this counts on only 1 byte quantities in the format!! */
	for (f = 0; f < setupdp->numFormats; f++) {
	bcopy (bptr, sptr, sizeof (xPixmapFormat));
	bptr += sizeof (xPixmapFormat);
	sptr += sizeof (xPixmapFormat);
	}
 /* Screen structures */
	for (s = 0; s < setupdp->numRoots; s++) {
	for (i = 0; i < 5; i++) {
	    swapcplp (bptr, sptr);
	    bptr += 4;
	    sptr += 4;
	}
	for (i = 0; i < 6; i++) {
	    swapcpsp (bptr, sptr);
	    bptr += 2;
	    sptr += 2;
	}
	swapcplp (bptr, sptr);	/* visualID */
	bptr += 4;
	sptr += 4;
	bcopy (bptr, sptr, 4);
	ndepths = bptr[3];	/* pull out nDepths */
	bptr += 4;
	sptr += 4;
	for (d = 0; d < ndepths; d++) {
	    *sptr++ = *bptr++;
	    *sptr++ = *bptr++;
	    swapcpsp (bptr, sptr);/* nVisuals */
	    nvisuals = * (short *) sptr;
	    bptr += 2;
	    sptr += 2;

	    bptr += 4;		/* pad */
	    sptr += 4;

	    for (v = 0; v < nvisuals; v++) {
		swapcplp (bptr, sptr);/* visualid */
		bptr += 4;
		sptr += 4;
		*sptr++ = *bptr++;
		*sptr++ = *bptr++;
		swapcpsp (bptr, sptr);/* colormapEntries */
		bptr += 2;
		sptr += 2;
		for (i = 0; i < 4; i++) {
		    swapcplp (bptr, sptr);
		    bptr += 4;
		    sptr += 4;
		}
	    }
	}
	}
    }
}
