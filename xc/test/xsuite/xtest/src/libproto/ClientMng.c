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
 * $XConsortium: ClientMng.c,v 1.10 94/04/17 21:01:11 rws Exp $
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
#include <X11/Xatom.h>

CL Xst_clients[MAX_CLIENTS];


XstDisplay *XstOpenDisplay();
static  void OutOfMemory();
static  void ReleaseMemory();


/*
 *	Routine: native_byte_sex
 *
 *	Input: none
 *
 *	Output: 
 *
 *	Returns: 0 if machine is LSB, 1 if MSB.
 *
 *	Globals used:
 *
 *	Side Effects:
 *
 *	Methods:  stuff a 1 into a longword.  Then pick up the 0th byte 
 *      of the longword.  MSB should have a 1 in it.  LSB should have a 0 
 *      in it.                         
 */

unsigned char
native_byte_sex ()
{
    int realbytesex;

    realbytesex = 1;
    return ( *((unsigned char *) &realbytesex));
}


/*
 *	Routine: Create_Client - open a connection to the server for client, 
 *      and stash information about the server and display
 *
 *	Input: client - integer from 0 to MAX_CLIENTS. an arbitrary number 
 *      assigned to this client. numbers can be reused once the connection 
 *      from this client to the server is closed.
 *
 *	Output: Xst_clients[client] is filled in with some data from the server
 *
 *	Returns:
 *
 *	Globals used:
 *
 *	Side Effects:
 *
 *	Methods: code was copied from XOpenDisplay in XLIB R2 for establishing
 *      the client-server connection and stashing information.  Display data 
 *      structure (Xst_client[client].cl_dpy) should be identical to that of 
 *      XLIB, sort of......
 *
 */

void
Create_Client(client)
int client;
{
    int bytesex = Xst_byte_sex;
    unsigned char creal;          /* what byte sex this machine is */
    unsigned char cfake;          /* what byte sex we'll make it appear */
    XstDisplay *dpy;		  /* local copy of display as XstOpen...
				     will assign to the dpy field of the
				     client structure (that's why it's
				     passed the client number now) so that
				     the GetConn routines can be passed
				     client number instead of display
				     and can then use Get_Display. Phew!
				   */

    creal = native_byte_sex ();

    switch (bytesex) {
    case SEX_NATIVE:
	cfake = creal;
	break;
    case SEX_REVERSE:
	cfake = !creal;
	break;
    case SEX_MSB:
	cfake = 1;
	break;
    case SEX_LSB:
	cfake = 0;
	break;
    }
    Xst_clients[client].cl_swap = creal ^ cfake;
    Xst_clients[client].cl_bytesex = cfake;

    Xst_clients[client].cl_pollout = 0;
    Xst_clients[client].cl_reqout = 0;
    Xst_clients[client].cl_reqtype = UNKNOWN_REQUEST_TYPE;
    Set_Test_Type(client, SETUP);

    dpy = XstOpenDisplay (Xst_server_node, Xst_clients[client].cl_bytesex, Xst_clients[client].cl_swap, client);

    if (dpy == NULL) {
	Log_Msg ("Create_Client: unable to open display %s",Xst_server_node);
	Delete ();
    }
}

/*
 *	As Create_Client, but for OpenDisplay testing so check if we're
 *	supposed to have failed, depends on test_type, and fail if something
 *	wasn't as expected, rather than deleting the test. Also returns
 *	a boolean indicating whether display opened correctly.
 */
int
Create_Client_Tested(client, test_type)
int client;
TestType test_type;
{
    int bytesex = Xst_byte_sex;
    unsigned char creal;          /* what byte sex this machine is */
    unsigned char cfake;          /* what byte sex we'll make it appear */
    XstDisplay *dpy;		  /* local copy of display as XstOpen...
				     will assign to the dpy field of the
				     client structure (that's why it's
				     passed the client number now) so that
				     the GetConn routines can be passed
				     client number instead of display
				     and can then use Get_Display. Phew!
				   */

    creal = native_byte_sex ();

    switch (bytesex) {
    case SEX_NATIVE:
	cfake = creal;
	break;
    case SEX_REVERSE:
	cfake = !creal;
	break;
    case SEX_MSB:
	cfake = 1;
	break;
    case SEX_LSB:
	cfake = 0;
	break;
    }
    Xst_clients[client].cl_swap = creal ^ cfake;
    Xst_clients[client].cl_bytesex = cfake;

    Xst_clients[client].cl_pollout = 0;
    Xst_clients[client].cl_reqout = 0;
    Xst_clients[client].cl_reqtype = OPEN_DISPLAY_REQUEST_TYPE;

    Set_Test_Type(client, test_type);

    dpy = XstOpenDisplay (Xst_server_node, Xst_clients[client].cl_bytesex, Xst_clients[client].cl_swap, client);

    return (dpy != NULL);
}

static
XModifierKeymap * XstNewModifiermap ();


/* 
 * Connects to a server, creates a XstDisplay object and returns a pointer to
 * the newly created XstDisplay back to the caller.
 */
XstDisplay * XstOpenDisplay (display, bytesex, needswap, cl)
register char  *display;
int     bytesex;
int     needswap;
int	cl; /* client number */
{
    register    XstDisplay * dpy;	/* New XstDisplay object being created. */
    register int    i;
    int     j,
            k;			/* random iterator indexes */
    char   *display_name;	/* pointer to display name */
    xConnClientPrefix client;	/* client information */
    xConnSetupPrefix prefix;	/* prefix information */
    int     vendorlen;		/* length of vendor string */
    char   *setup;		/* memory allocated at startup */
    char    displaybuf[256];	/* buffer to receive expanded name */
    int     screen_num;		/* screen number */
    union {
	xConnSetup * setup;
	char   *failure;
	char   *vendor;
	xPixmapFormat * sf;
	xWindowRoot * rp;
	xDepth * dp;
	xVisualType * vp;
    } u;
    long    setuplength;	/* number of bytes in setup message */
    void    (*complain)();
    char *auth_proto;
    int auth_length;
    char *auth_string;
    int auth_strlen;
    int using_xo;

    extern int  XstSendClientPrefix ();
    extern int  XstConnectDisplay ();
    extern char *getenv ();
    extern  XID Get_Resource_Id();

 /* 
  * If the display specifier string supplied as an argument to this 
  * routine is NULL or a pointer to NULL, read the DISPLAY variable.
  */
    if (display == NULL || *display == '\0') {
	if ((display_name = getenv ("DISPLAY")) == NULL) {
	/* Oops! No DISPLAY environment variable - error. */
	    Log_Msg ("No XT_DISPLAY and no DISPLAY environment variable\n");
	    Delete ();
	}
    }
    else {
    /* Display is non-NULL, copy the pointer */
	display_name = display;
    }

/*
 * Attempt to allocate a display structure. Delete if allocation fails.
 */
    if ((dpy = (XstDisplay *) Xstcalloc (1, sizeof (XstDisplay))) == NULL) {
	errno = ENOMEM;
	Log_Msg ("Failed to allocate enough memory for display structure\n");
	Delete ();
    }
/*
 * Now we've got the display structure record it in the client structure
 */
    Set_Display(cl, dpy); /* required to be there for GetConn routines */

/*
 * Call the Connect routine to get the network socket. If 0 is returned, the
 * connection failed. The connect routine will return the expanded display
 * name in displaybuf.
 */

    dpy -> xlib_dpy = (Display *)NULL;
    if ((dpy -> fd = XstConnectDisplay (display_name, displaybuf, &screen_num,
					&auth_proto, &auth_length,
					&auth_string, &auth_strlen,
					&(dpy->xlib_dpy)))
	    < 0) {
	Log_Msg ("Could not create network connection to %s\n",display_name);
	Delete ();
	/*NOTREACHED*/
	free ((char *) dpy);
	return (NULL);		/* errno set by XConnectDisplay */
    }
    using_xo = (dpy->xlib_dpy != (Display *)NULL);


/*
 * First byte is the byte order byte.
 */

    if (bytesex) {
	client.byteOrder = 0x6C; /* 'l' */
    }
    else {
	client.byteOrder = 0x42; /* 'B' */
    }
    switch (Get_Test_Type(cl)) {
    case GOOD:
    case TOO_LONG:
    case BAD_LENGTH:
    case JUST_TOO_LONG:
    case SETUP:
    case BAD_IDCHOICE1:
    case BAD_IDCHOICE2:
    case BAD_VALUE:
	if (using_xo && needswap) {
		Log_Msg("Using XOpenDisplay won't allow non client-native testing\n");
		XCloseDisplay(dpy->xlib_dpy);
		free((char *)dpy);
		Delete();
		/*NOTREACHED*/
	}
	break;
    case OPEN_DISPLAY:
	if (using_xo) {
		Log_Msg("Using XOpenDisplay won't allow bad byteOrder testing\n");
		XCloseDisplay(dpy->xlib_dpy);
		free((char *)dpy);
		Untested(); /* 'cos we know its an extended assertion */
		/*NOTREACHED*/
	}
	client.byteOrder ^= 0xFF; /* now not one of 0x42 or 0x6C */
	Log_Trace ("About to send OpenDisplay with byte_order = 0x%x\n", client.byteOrder);
	break;
    default:
        Log_Del ("INTERNAL ERROR: bad TestType %d when about to OpenDisplay\n", Get_Test_Type(cl));
	Exit ();
	/* NOTREACHED */
	break;
    }
    client.majorVersion = X_PROTOCOL;
    client.minorVersion = X_PROTOCOL_REVISION;
    client.nbytesAuthProto = auth_length;
    client.nbytesAuthString = auth_strlen;
    /* some of these routines are cheats if using_xo is true... */
    XstSendClientPrefix (dpy, &client, auth_proto, auth_string, needswap);
/*
 * Now see if connection was accepted...
 */
    GetConnSetupPrefix (cl, &prefix, needswap);

/*
 * If the connection was not accepted by the server due to problems,
 * give error message to the user....
 */
    if (Get_Req_Type(cl) != OPEN_DISPLAY_REQUEST_TYPE) {
	/* OK, we required this to work to set up the test, which is other than
	 * the open display test.
	 */
    	if (prefix.success != xTrue) {
	    static char reason_buf[132];
	    char *pref_res = ((char *)&prefix) + sizeof(prefix);

	    if (prefix.lengthReason <= 0)
		strcpy(reason_buf, "<None Given>");
	    else {
		int n = min(sizeof(reason_buf)-10, prefix.lengthReason);

		strncpy(reason_buf, pref_res, n);
		if (n < (int)prefix.lengthReason)
			strcat(reason_buf, "... etc."); /* the 10 above is to leave room for this */
	    }
	    Log_Del ("Failed to OpenDisplay, reason was %s\n", reason_buf);
	    free ((char *) dpy);
	    return (NULL);
	}
    } else {
	/* OK, we're in the open display test, did we expect failure? */
	CARD8 expectation = (Get_Test_Type(cl) == OPEN_DISPLAY) ?
				xFalse : xTrue;

	if (prefix.success != expectation) {
	    static char reason_buf[132];
	    char *pref_res = ((char *)&prefix) + sizeof(prefix);

	    if (prefix.lengthReason <= 0)
		strcpy(reason_buf, "<None Given>");
	    else {
		int n = min(sizeof(reason_buf)-10, prefix.lengthReason);

		strncpy(reason_buf, pref_res, n);
		if (n < (int)prefix.lengthReason)
			strcat(reason_buf, "... etc."); /* the 10 above is to leave room for this */
	    }
	    Log_Trace ("OpenDisplay failure, got %s, expecting %s, with reason %s\n",
		boolname(prefix.success), boolname(expectation), reason_buf);
	    free ((char *) dpy);
	    return (NULL);
	}
	if (expectation == xFalse) {
	/*
	 * We failed, as expected, so no point in trying to read rest and
	 * already setup only enough to allow the dpy to be closed down
	 */
	    return dpy;
	}
    }

    if ((int)prefix.majorVersion < Xst_protocol_version) {
	Log_Msg ("Warning: Client (%d) built for newer server (%d)!\n", Xst_protocol_version, prefix.majorVersion);
    }
    if (prefix.minorVersion != Xst_protocol_revision) {
	Log_Msg (
		"Warning: Protocol rev. of client (%d) does not match server (%d)!\n",
		Xst_protocol_revision, prefix.minorVersion);
    }

    setuplength = prefix.length << 2;
    if (setuplength < MIN_SETUP_DATA_LEN) {
	/* junk reply, so no need to proceed further. */
	if (Get_Req_Type(cl) == OPEN_DISPLAY_REQUEST_TYPE)
		complain = Log_Err;
	else
		complain = Log_Del;
	(*complain)("Short setup data (%ld bytes instead of at least %ld bytes)\n",
			setuplength, (long)MIN_SETUP_DATA_LEN);
	if (using_xo)
		XCloseDisplay(dpy->xlib_dpy);
	free((char *)dpy);
	return NULL;
    }
    if ((u.setup = (xConnSetup *) (setup = (char *) Xstmalloc ((unsigned) setuplength)))
	    == (xConnSetup *)NULL) {
	errno = ENOMEM;
	Log_Msg ("Not enough memory to allocate rest of connection data (%d bytes)\n", setuplength);
	if (using_xo)
		XCloseDisplay(dpy->xlib_dpy);
	free((char *)dpy);
	Delete ();
	/*NOTREACHED*/
    }
    /* may be faked if using_xo is true.... */
    GetConnSetupData (cl, (char *) u.setup, setuplength, needswap);

/*
 * We succeeded at authorization, so let us move the data into
 * the display structure.
 */
    dpy -> proto_major_version = prefix.majorVersion;
    dpy -> proto_minor_version = prefix.minorVersion;
    dpy -> release = u.setup -> release;
    dpy -> resource_base = u.setup -> ridBase;
    dpy -> resource_mask = u.setup -> ridMask;
    dpy -> min_keycode = u.setup -> minKeyCode;
    dpy -> max_keycode = u.setup -> maxKeyCode;
    dpy -> keysyms = (KeySym *) NULL;
    dpy -> modifiermap = XstNewModifiermap (0);
    dpy -> keysyms_per_keycode = 0;
    dpy -> current = None;
    dpy -> xdefaults = (char *) NULL;
    dpy -> scratch_length = 0L;
    dpy -> scratch_buffer = NULL;
    dpy -> motion_buffer = u.setup -> motionBufferSize;
    dpy -> nformats = u.setup -> numFormats;
    dpy -> nscreens = u.setup -> numRoots;
    dpy -> byte_order = u.setup -> imageByteOrder;
    dpy -> bitmap_unit = u.setup -> bitmapScanlineUnit;
    dpy -> bitmap_pad = u.setup -> bitmapScanlinePad;
    dpy -> bitmap_bit_order = u.setup -> bitmapBitOrder;
    dpy -> max_request_size = u.setup -> maxRequestSize;
#ifdef notrequired
    dpy -> ext_procs = (struct _XExten *) NULL;
    dpy -> ext_number = 0;
#endif
    dpy -> ext_data = (XExtData *) NULL;
    dpy -> event_vec[X_Error] = NULL;
    dpy -> event_vec[X_Reply] = NULL;
    dpy -> wire_vec[X_Error] = NULL;
    dpy -> wire_vec[X_Reply] = NULL;
    for (i = KeyPress; i < LASTEvent; i++) {
	dpy -> event_vec[i] = NULL;
	dpy -> wire_vec[i] = NULL;
    }
    for (i = LASTEvent; i < 128; i++) {
	dpy -> event_vec[i] = NULL;
	dpy -> wire_vec[i] = NULL;
    }
    dpy -> resource_id = 0;
    dpy -> resource_shift = ffs ((int) dpy -> resource_mask) - 1;
/*  dpy -> db = (struct _XrmHashBucketRec *) NULL; */
/* 
 * Initialize pointers to NULL so that XstFreeDisplayStructure will
 * work if we run out of memory
 */

    dpy -> screens = NULL;
    dpy -> display_name = NULL;
    dpy -> buffer = NULL;

/*
 * now extract the vendor string...  String must be null terminated,
 * padded to multiple of 4 bytes.
 */
    dpy -> vendor = (char *) Xstmalloc (u.setup -> nbytesVendor + 1);
    if (dpy -> vendor == NULL) {
	OutOfMemory (dpy, setup);
	return (NULL);
    }
    vendorlen = u.setup -> nbytesVendor;
    u.setup += 1;		/* can't touch information in XConnSetup
				   anymore.. */
    (void) strncpy (dpy -> vendor, u.vendor, vendorlen);
    u.vendor += (vendorlen + 3) & ~3;
/*
 * Now iterate down setup information.....
 */
    dpy -> pixmap_format =
	(XstScreenFormat *) Xstmalloc (
	    (unsigned) (dpy -> nformats * sizeof (XstScreenFormat)));
    if (dpy -> pixmap_format == NULL) {
	OutOfMemory (dpy, setup);
	return (NULL);
    }
/*
 * First decode the Z axis Screen format information.
 */
    for (i = 0; i < dpy -> nformats; i++) {
	register    XstScreenFormat * fmt = &dpy -> pixmap_format[i];
	fmt -> depth = u.sf -> depth;
	fmt -> bits_per_pixel = u.sf -> bitsPerPixel;
	fmt -> scanline_pad = u.sf -> scanLinePad;
	fmt -> ext_data = NULL;
	u.sf += 1;
    }

/*
 * next the Screen structures.
 */
    dpy -> screens =
	(XstScreen *) Xstmalloc ((unsigned) dpy -> nscreens * sizeof (XstScreen));
    if (dpy -> screens == NULL) {
	OutOfMemory (dpy, setup);
	return (NULL);
    }
/*
 * Now go deal with each screen structure.
 */
    for (i = 0; i < dpy -> nscreens; i++) {
	register    XstScreen * sp = &dpy -> screens[i];
	VisualID root_visualID = u.rp -> rootVisualID;
	sp -> display = dpy;
	sp -> root = u.rp -> windowId;
	sp -> cmap = u.rp -> defaultColormap;
	sp -> white_pixel = u.rp -> whitePixel;
	sp -> black_pixel = u.rp -> blackPixel;
	sp -> root_input_mask = u.rp -> currentInputMask;
	sp -> width = u.rp -> pixWidth;
	sp -> height = u.rp -> pixHeight;
	sp -> mwidth = u.rp -> mmWidth;
	sp -> mheight = u.rp -> mmHeight;
	sp -> min_maps = u.rp -> minInstalledMaps;
	sp -> max_maps = u.rp -> maxInstalledMaps;
	sp -> root_visual = NULL;
				/* filled in later, when we alloc Visuals 
				*/
	sp -> backing_store = u.rp -> backingStore;
	sp -> save_unders = u.rp -> saveUnders;
	sp -> root_depth = u.rp -> rootDepth;
	sp -> ndepths = u.rp -> nDepths;
	sp -> ext_data = NULL;
	u.rp += 1;
/*
 * lets set up the depth structures.
 */
	sp -> depths = (XstDepth *) Xstmalloc (
		(unsigned) sp -> ndepths * sizeof (XstDepth));
	if (sp -> depths == NULL) {
	    OutOfMemory (dpy, setup);
	    return (NULL);
	}
    /* 
     * for all depths on this screen.
     */
	for (j = 0; j < sp -> ndepths; j++) {
	    XstDepth * dp = &sp -> depths[j];
	    dp -> depth = u.dp -> depth;
	    dp -> nvisuals = u.dp -> nVisuals;
	    u.dp += 1;
	    dp -> visuals =
		(XstVisual *) Xstmalloc ((unsigned) dp -> nvisuals * sizeof (XstVisual));
	    if (dp -> visuals == NULL) {
		OutOfMemory (dpy, setup);
		return (NULL);
	    }
	    for (k = 0; k < dp -> nvisuals; k++) {
		register    XstVisual * vp = &dp -> visuals[k];
		if ((vp -> visualid = u.vp -> visualID) == root_visualID)
		    sp -> root_visual = vp;
		vp -> class = u.vp -> class;
		vp -> bits_per_rgb = u.vp -> bitsPerRGB;
		vp -> map_entries = u.vp -> colormapEntries;
		vp -> red_mask = u.vp -> redMask;
		vp -> green_mask = u.vp -> greenMask;
		vp -> blue_mask = u.vp -> blueMask;
		vp -> ext_data = NULL;
		u.vp += 1;
	    }
	}
    }


/*
 * Setup other information in this display structure.
 */
    dpy -> vnumber = X_PROTOCOL;
    dpy -> resource_alloc = Get_Resource_Id;
    dpy -> synchandler = NULL;
    dpy -> request = using_xo ?
	((NextRequest(dpy->xlib_dpy) <= 0) ? 0 : (NextRequest(dpy->xlib_dpy)-1))
	: 0;
    dpy -> last_request_read = 0;
    dpy -> default_screen = screen_num;
				/* Value returned by ConnectDisplay */
    dpy -> last_req = (char *) NULL;

 /* Salt away the host:display string for later use */
    if ((dpy -> display_name = (char *) Xstmalloc (
		    (unsigned) (strlen (displaybuf) + 1))) == NULL) {
	OutOfMemory (dpy, setup);
	return (NULL);
    }
    (void) strcpy (dpy -> display_name, displaybuf);

/* Set up the output buffers. */
    if ((dpy -> bufptr = dpy -> buffer = (char *) Xstmalloc (OBUFSIZE)) == NULL) {
	OutOfMemory (dpy, setup);
	return (NULL);
    }
    dpy -> bufmax = dpy -> buffer + OBUFSIZE;

 /* Set up the input event queue and input event queue parameters. */
/*  dpy -> head = dpy -> tail = NULL; */
    dpy -> qlen = 0;

/*
 * Now start talking to the server to setup all other information...
 */

    free (setup);		/* all finished with setup information */
/*
 * chain this stucture onto global list.
 */

    return (dpy);
}


/* OutOfMemory is called if Xstmalloc fails.  XOpenDisplay returns NULL
   after this returns. */

static  void OutOfMemory (dpy, setup)
        XstDisplay * dpy;
	char   *setup;
{
    ReleaseMemory (dpy, setup);
    errno = ENOMEM;
    Log_Del ("Not enough memory for holding connection setup info.\n");
}

static  void ReleaseMemory (dpy, setup)
        XstDisplay * dpy;
	char   *setup;
{
    XstDisconnectDisplay (dpy -> fd);
    XstFreeDisplayStructure (dpy);
    free (setup);
}


/* XstFreeDisplayStructure frees all the storage associated with a 
 * Display.  It is used by XOpenDisplay if it runs out of memory,
 * and also by XCloseDisplay.   It needs to check whether all pointers
 * are non-NULL before dereferencing them, since it may be called
 * by XstOpenDisplay before the XstDisplay structure is fully formed.
 * XstOpenDisplay must be sure to initialize all the pointers to NULL
 * before the first possible call on this.
 */

XstFreeDisplayStructure (dpy)
register    XstDisplay * dpy;
{
    /* if we have used XOpenDisplay to get the fd then free the xlib things */
    if (dpy -> xlib_dpy != (Display *)NULL) {
	(void)XCloseDisplay(dpy->xlib_dpy);
	dpy->xlib_dpy = (Display  *)NULL;
    }

    /* OK, now on with ours.... */
    if (dpy -> screens) {
	register int    i;

	for (i = 0; i < dpy -> nscreens; i++) {
	    XstScreen * sp = &dpy -> screens[i];

	    if (sp -> depths) {
		register int    j;

		for (j = 0; j < sp -> ndepths; j++) {
		    XstDepth * dp = &sp -> depths[j];

		    if (dp -> visuals) {
			free ((char *) dp -> visuals);
		    }
		}

		free ((char *) sp -> depths);
	    }

	}

	free ((char *) dpy -> screens);
    }

    if (dpy -> pixmap_format) {

	free ((char *) dpy -> pixmap_format);
    }

    if (dpy -> display_name)
	free (dpy -> display_name);

    if (dpy -> buffer)
	free (dpy -> buffer);
    if (dpy -> keysyms)
	free ((char *) dpy -> keysyms);
    if (dpy -> xdefaults)
	free (dpy -> xdefaults);


    (void) free ((char *) dpy);
}

static
XModifierKeymap *
XstNewModifiermap (keyspermodifier)
int     keyspermodifier;
{
    XModifierKeymap * res = (XModifierKeymap *) Xstmalloc ((sizeof (XModifierKeymap
    )));
    res -> max_keypermod = keyspermodifier;
    res -> modifiermap = (keyspermodifier > 0 ?
	(KeyCode *) Xstmalloc (8 * keyspermodifier)
	: (KeyCode *) NULL);
    return (res);
}

Destroy_Client(client)
int client;
{
	XstDisplay *dpy = Get_Display(client);
	int 	tmpfd = -1;

	if (dpy == (XstDisplay *)NULL)
		return;

	tmpfd = dpy->fd;
	dpy->fd = -1;

	XstFreeDisplayStructure(dpy);

	if (tmpfd != -1)
		(void) close(tmpfd);
}
