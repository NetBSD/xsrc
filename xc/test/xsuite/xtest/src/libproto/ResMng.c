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
 * $XConsortium: ResMng.c,v 1.3 94/04/17 21:01:21 rws Exp $
 */
/****************************************************************************
 * Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                          *
 *                                                                          *
 *                         All Rights Reserved                              *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation for any purpose and without fee is hereby granted,         *
 * provided that the above copyright notice appears in all copies and that  *
 * both that copyright notice and this permission notice appear in          *
 * supporting documentation, and that the name of Sequent not be used       *
 * in advertising or publicity pertaining to distribution or use of the     *
 * software without specific, written prior permission.                     *
 *                                                                          *
 * SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS; IN NO EVENT SHALL *
 * SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 * SOFTWARE.                                                                *
 ****************************************************************************/

#include "XstlibInt.h"


Atom
Create_Atom (client)
int     client;
{
    xInternAtomReq * req;
    xInternAtomReply * rep;
    Atom aid;

    req = (xInternAtomReq *) Make_Req (client, X_InternAtom);
    Send_Req (client, (xReq *) req);
    Log_Trace ("client %d sent default InternAtom request\n", client);

    if ((rep = (xInternAtomReply *) Expect_Reply (client,X_InternAtom)) == NULL) {
	Log_Err ("client %d failed to recv InternAtom reply\n", client);
	Exit ();
    }
    else {
	Log_Trace ("client %d received InternAtom reply\n", client);
    /* do any reply checking here */
	aid = rep -> atom;
	Free_Reply ((char *)rep);
    }
    Free_Req (req);
    return (aid);
}


Atom
Create_Default_Atom (client)
int     client;
{
    Atom rid;
    rid = Create_Atom (client);
    Set_Default_Atom (client, rid);
    return (rid);
}

/* 
   intent:	  return the resource ID of an allocated Colormap
   input:	  client - integer from 0 to MAX_CLIENTS
   output:	  Colormap ID - integer from ? to ?
   global input:  
   side effects:  binds one resource ID
   methods:	 	
*/

Colormap
Create_Colormap (client)
int     client;
{
    xCreateColormapReq * req;
    Colormap mid;

    req = (xCreateColormapReq *) Make_Req (client, X_CreateColormap);
    Send_Req (client, (xReq *) req);
    mid = req -> mid;
    Log_Trace ("client %d sent CreateColormap for colormap %d\n", client, mid);
    Expect_Nothing (client);
    Free_Req (req);
    return (mid);
}


/* 
   intent:	 
   input:	 	
   output:	 
   global input: 
   side effects: 	
   methods:	 	
*/

Colormap
Create_Default_Colormap (client)
int     client;
{
    Colormap rid;
    rid = Create_Colormap (client);
    Set_Default_Colormap (client, rid);
    return (rid);
}


Cursor
Create_Cursor (client)
int     client;
{
    xCreateCursorReq * req;
    Cursor cid;

    req = (xCreateCursorReq *) Make_Req (client, X_CreateCursor);
    Send_Req (client, (xReq *) req);
    Log_Trace ("client %d sent default CreateCursor request\n", client);
    Expect_Nothing (client);
    cid = req -> cid;
    Free_Req (req);
    return (cid);
}


Cursor
Create_Default_Cursor (client)
int     client;
{
    Cursor rid;
    rid = Create_Cursor (client);
    Set_Default_Cursor (client, rid);
    return (rid);
}



Font
Create_Font (client)
int     client;
{
    xOpenFontReq * req;
    Font fid;

    req = (xOpenFontReq *) Make_Req (client, X_OpenFont);
    Send_Req (client, (xReq *) req);
    Log_Trace ("client %d sent default OpenFont request\n", client);
    Expect_Nothing (client);
    fid = req -> fid;
    Free_Req (req);
    return (fid);
}


Font
Create_Default_Font (client)
int     client;
{
    Font rid;
    rid = Create_Font (client);
    Set_Default_Font (client, rid);
    return (rid);
}


GContext
Create_GContext (client)
int     client;
{
    xCreateGCReq * req;
    GContext gc;

    req = (xCreateGCReq *) Make_Req (client, X_CreateGC);
    Send_Req (client, (xReq *) req);
    Log_Trace ("client %d sent default CreateGC request\n", client);
    Expect_Nothing (client);
    gc = req -> gc;
    Free_Req (req);
    return (gc);
}

GContext
Create_Default_GContext (client)
int     client;
{
    GContext rid;
    rid = Create_GContext (client);
    Set_Default_GContext (client, rid);
    return (rid);
}

Pixmap
Create_Pixmap (client)
int     client;
{
    xCreatePixmapReq * req;
    Pixmap pid;

    req = (xCreatePixmapReq *) Make_Req (client, X_CreatePixmap);
    Send_Req (client, (xReq *) req);
    Log_Trace ("client %d sent default CreatePixmap request\n", client);
    Expect_Nothing (client);
    pid = req -> pid;
    Free_Req (req);
    return (pid);
}

Pixmap
Create_Default_Pixmap (client)
int     client;
{
    Pixmap rid;
    rid = Create_Pixmap (client);
    Set_Default_Pixmap (client, rid);
    return (rid);
}

Pixmap
Create_Cursor_Pixmap (client)
int     client;
{
    xCreatePixmapReq * req;
    Pixmap pid;

    req = (xCreatePixmapReq *) Make_Req (client, X_CreatePixmap);
    req->depth = 1;
    Send_Req (client, (xReq *) req);
    Log_Trace ("client %d sent default CreatePixmap request\n", client);
    Expect_Nothing (client);
    pid = req -> pid;
    Free_Req (req);
    return (pid);
}

Pixmap
Create_Default_Cursor_Pixmap (client)
int     client;
{
    Pixmap rid;
    rid = Create_Cursor_Pixmap (client);
    Set_Default_Cursor_Pixmap (client, rid);
    return (rid);
}

Window
Create_Window (client)
int     client;
{
    xCreateWindowReq * req;
    Window wid;

    req = (xCreateWindowReq *) Make_Req (client, X_CreateWindow);
    Send_Req (client, (xReq *) req);
    wid = req -> wid;
    Log_Trace ("client %d sent default CreateWindow for window %d\n", client, wid);
    Expect_Nothing (client);
    Free_Req (req);
    return (wid);
}

Window
Create_Child_Window (client, parent)
int     client;
Window	parent;
{
    xCreateWindowReq * req;
    Window wid;

    req = (xCreateWindowReq *) Make_Req (client, X_CreateWindow);
    req->parent = parent;
    Send_Req (client, (xReq *) req);
    wid = req -> wid;
    Log_Trace ("client %d sent CreateWindow for window %d, a child of window %d\n", client, wid, parent);
    Expect_Nothing (client);
    Free_Req (req);
    return (wid);
}


/* 
   intent:	 set up the client data structure to have a default window
   input:	 client ID - integer from 0 to MAX_CLIENTS
   output:	 window ID - integer from ? to ?
   global input: 
   side effects: default window ID field in client data structure is 
                 overwritten
   methods:	 	
*/

Window
Create_Default_Window (client)
int     client;
{
    Window rid;
    rid = Create_Window (client);
    Set_Default_Window (client, rid);
    return (rid);
}

xEvent
Create_Event (client, event_type)
int     client;
int event_type;
{
    xEvent event;
    int i;


    switch (event_type) {
	case ClientMessage:
	    event.u.u.type = 33;
	    event.u.u.detail = 8;
	    event.u.clientMessage.window = Get_Default_Window(client);
	    event.u.clientMessage.u.b.type = Get_Default_Atom(client);
	    for (i = 0; i < 20; i++) /* set this for debug purposes only */
/*		    event.u.clientMessage.u.b.bytes[i] = 0xf;*/
		    event.u.clientMessage.u.b.bytes[i] = 0x6;
	    break;
	default:
	    DEFAULT_ERROR;
	    break;
    }

    return (event);
}

void
Create_Default_Event (client, event_type)
int     client;
int event_type;
{
    Set_Default_Event(client, Create_Event(client, event_type));
}

