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
 * $XConsortium: Xstlib.h /main/7 1995/12/07 10:48:14 dpw $
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
 *	Includes for OS dependent headers
 */
#include "Xstos.h"

/*
 *	Includes of standard X headers
 */
#define NEED_REPLIES
#define NEED_EVENTS

#include <X11/Xproto.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/X.h>

/*
 *	The client structure (Xst_clients) is visible to
 *	tests to provide easy use of macros to get at the
 *	underlying display structure stuff. Tests should
 *	NEVER muck directly with the client or display structure!!
 *	Always use Get_* and Set_* to affect this.
 */

#define MAX_CLIENTS 8
#define LONG_LIVED_CLIENT (MAX_CLIENTS-1)

typedef enum {
	GOOD, BAD_LENGTH, TOO_LONG, JUST_TOO_LONG, SETUP, OPEN_DISPLAY,
        BAD_IDCHOICE1, BAD_IDCHOICE2, BAD_VALUE
} TestType;

#include "XstDisplay.h"

typedef struct cl {	/* client entry */
    XstDisplay *cl_dpy;	/* display structure for this client */
    int cl_bytesex;	/* byte sex for this client */
    int cl_swap;	/* need to swap to behave like this? */
    int cl_pollout;	/* seq # of outstanding poll */
    int cl_reqout;	/* seq # of outstanding request */
    short cl_imagewidth;	/* width of last image requested by GetImage */
    short cl_imageheight;	/* height of last image requested */
    int cl_reqtype;	/* type of outstanding request */
    Atom cl_atom;	/* default atom */
    Colormap cl_colormap;	/* default colormap */
    Cursor cl_cursor;	/* default cursor */
    Font cl_font;	/* default font */
    GContext cl_gc;	/* default gc */
    Pixmap cl_pixmap;	/* default pixmap */
    Pixmap cl_cursor_pixmap;
                        /* default cursor pixmap (depth = 1) */
    Window cl_window;	/* default window */
    xEvent cl_event;    /* default event */
    TestType cl_test_type;	/* test for success or two types of BadLength */
    int cl_minor;	/* minor opcode of outstanding request */
} CL;

extern CL Xst_clients[MAX_CLIENTS];

/* this is not a legal core request type (1..127, with gaps i.e.
 * X_CreateWindow .. X_NoOperation).
 */
#define UNKNOWN_REQUEST_TYPE	0
/* this is not a legal core event type (2..34) or a reply (1), but looks
 * like an error (0), but isn't.
 */
#define ARBITRARY_EVENT_TYPE	0
/* this is another value distinguishable from real requests. Problem is that
 * there isn't an X_OpenDisplay and we're worried about using any of the gaps
 * in the current encoding (120..126, inclusive, as of X11R5). We use -1
 * instead.
 */
#define OPEN_DISPLAY_REQUEST_TYPE ((int)-1)

/* Some initial checking for cocection replies, assumes at least one depth
 * pixmap must be supported (n == 1) and 1 screen with 1 depth (m == 1)
 * which comes out as a total length of 10 + 14 = 24 words.
 */
#define MIN_SETUP_DATA_LEN	24

/*
 *	Defines for selecting, on a per client basis, the apparent
 *	byte sex of that client.
 *	Note that SEX_BOTH has been added and is the default.
 *	In this case the test code is called twice with byte sex
 *	set to NATIVE and REVERSE.
 */
#define SEX_BOTH        0
#define SEX_NATIVE      1
#define SEX_REVERSE     2
#define SEX_MSB         3
#define SEX_LSB         4

/*
 *	Define expectations
 */

#define EXPECT_REPLY	0
#define EXPECT_ERROR	1
#define EXPECT_EVENT	2
#define EXPECT_NOTHING	3
#define EXPECT_01EVENT	4
/*
 *	General defines
 */
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#ifndef NULL
#define NULL 0
#endif

/*
 *	Defines for Expect_*
 */
#define Expect_Error(client,type)	((xError *)Expect(client,EXPECT_ERROR,type))
#define Expect_Event(client,type)	((xEvent *)Expect(client,EXPECT_EVENT,type))
#define Expect_01Event(client,type)	((xEvent *)Expect(client,EXPECT_01EVENT,type))
#define Expect_Nothing(client)	((int) Expect(client,EXPECT_NOTHING,0))
#define Expect_Reply(client,type)	((xReply *)Expect(client,EXPECT_REPLY,type))
#define Expect_Ext_Reply(client,type,major)((xReply *)Expect(client,EXPECT_REPLY,((type<<8)|major)))

/*
 *	Free_*
 */
#define	Free_Error(ep)	free((char *)(ep))
#define	Free_Event(ep)	free((char *)(ep))
#define	Free_Reply(rp)	free((char *)(rp))
#define	Free_Req(rp)	free((char *)(rp))



/*
 *	Macros to access display attributes.
 */

#define Get_Display(client)	(Xst_clients[client].cl_dpy)
#define Set_Display(client, val)	(Xst_clients[client].cl_dpy = (val))
#define Get_Root_Id(client)	(XstDefaultRootWindow(Get_Display(client)))
#define Get_Screen_Width(client) \
	(XstDisplayWidth(Get_Display(client),XstDefaultScreen(Get_Display(client))))
#define Get_Screen_Height(client) \
	(XstDisplayHeight(Get_Display(client),XstDefaultScreen(Get_Display(client))))
#define Get_Planes(client) \
        (XstDisplayPlanes(Get_Display(client),XstDefaultScreen(Get_Display(client))))
#define Get_Max_Request(client) \
	(Get_Display(client)->max_request_size)
/*
 *      Macros to access visual attributes.
 */

#define Get_Visual(client,screen) \
    (XstDefaultVisual(Get_Display(client),screen))
#define Get_Visual_ID(visual) (visual->visualid)
#define Get_Visual_Class(visual) (visual->class)
#define Get_Visual_Depth(visual) (visual->depth)
/*
 *	Get access to the 'value-list' portion of a request and the
 *	length (in bytes) of the value-list
 *      rp - pointer to request
 *      type - type of request (e.g. GrabButton)
 */

#ifdef __STDC__
#define Get_Value_Ptr(rp,type) ((unsigned long *) (((unsigned long) (rp) +   				  sizeof (x##type##Req))))
#else
#define Get_Value_Ptr(rp,type) ((unsigned long *) (((unsigned long) (rp) +   				  sizeof (x/**/type/**/Req))))
#endif

#ifdef __STDC__
#define Get_Value_Len(rp,type) ((unsigned long) (((rp)->length<<2) - \
				   sizeof (x##type##Req)))
#else
#define Get_Value_Len(rp,type) ((unsigned long) (((rp)->length<<2) - \
				   sizeof (x/**/type/**/Req)))
#endif

/*
 *	Test Resource Management
 */

#define	Get_Default_Atom(cl)	(Xst_clients[cl].cl_atom)
#define	Get_Default_Colormap(cl)	(Xst_clients[cl].cl_colormap)
#define	Get_Default_Cursor(cl)	(Xst_clients[cl].cl_cursor)
#define	Get_Default_Font(cl)	(Xst_clients[cl].cl_font)
#define	Get_Default_GContext(cl)	(Xst_clients[cl].cl_gc)
#define	Get_Default_Pixmap(cl)	(Xst_clients[cl].cl_pixmap)
#define	Get_Default_Cursor_Pixmap(cl)	(Xst_clients[cl].cl_cursor_pixmap)
#define	Get_Default_Window(cl)	(Xst_clients[cl].cl_window)
#define	Get_Default_Event(cl)	(Xst_clients[cl].cl_event)
#define Get_Test_Type(cl)	(Xst_clients[cl].cl_test_type)
#define Get_Req_Type(cl)	(Xst_clients[cl].cl_reqtype)

#define	Set_Default_Atom(cl,val)	(Xst_clients[cl].cl_atom = (val))
#define	Set_Default_Colormap(cl,val)	(Xst_clients[cl].cl_colormap = (val))
#define	Set_Default_Cursor(cl,val)	(Xst_clients[cl].cl_cursor = (val))
#define	Set_Default_Font(cl,val)	(Xst_clients[cl].cl_font = (val))
#define	Set_Default_GContext(cl,val)	(Xst_clients[cl].cl_gc = (val))
#define	Set_Default_Pixmap(cl,val)	(Xst_clients[cl].cl_pixmap = (val))
#define	Set_Default_Cursor_Pixmap(cl,val)	(Xst_clients[cl].cl_cursor_pixmap = (val))
#define	Set_Default_Window(cl,val)	(Xst_clients[cl].cl_window = (val))
#define	Set_Default_Event(cl,val)	(Xst_clients[cl].cl_event = (val))
#define	Set_Test_Type(cl,val)	(Xst_clients[cl].cl_test_type = (val))
#define	Set_Req_Type(cl,val)	(Xst_clients[cl].cl_reqtype = (val))

/*
 *	Some dummy request types used for testing only
 */
#define	Xst_BadType	254
#define Xst_BadLength	255

/*
 * defines for real and fake events
 */

#define real_type(event_type) (event_type & 0x7f)
#define is_fake(event_type) ((event_type & 0x80) != 0)

/*
 *	Routine definitions for TET startup and cleanup.
 */
void	protostartup();
void	protocleanup();
void	openprotostartup();
void	openprotocleanup();
void	fontprotostartup();
void	fontprotocleanup();
void	checkconfig();

/*
 *	Xstlib routine definitions
 */
void	Abort();
void	Delete ();
void	Finish ();
Atom	Create_Atom();
void	Create_Client ();
int	Create_Client_Tested ();
Colormap	Create_Colormap();
Cursor	Create_Cursor();
Atom	Create_Default_Atom();
Colormap	Create_Default_Colormap();
Cursor	Create_Default_Cursor();
Font	Create_Default_Font();
GContext	Create_Default_GContext();
Pixmap	Create_Default_Pixmap();
Window	Create_Default_Window();
Font	Create_Font();
GContext	Create_GContext();
Pixmap	Create_Pixmap();
Window	Create_Window();
void	Exit();
void	Exit_OK();
xReply *Expect();
char	*Get_Date();
int	Get_Timer();
XID	Get_Resource_Id();
int	Log_Close();
void	Reset_Some();
/*VARARGS1*/
void	Log_Debug();
/*VARARGS1*/
void	Log_Debug2();
/*VARARGS1*/
void	Log_Debug3();
/*VARARGS1*/
void	Log_Some();
/*VARARGS1*/
void	Log_Err();
/*VARARGS1*/
void	Log_Del();
/*VARARGS1*/
void	Log_Err_Detail();
/*VARARGS1*/
void	Log_Msg();
void	Log_Open();
/*VARARGS1*/
void	Log_Trace();
xReq	*Make_Req();
xReq	*Clear_Counted_Value();
xReq	*Add_Counted_Value();
void	Map_Window();
void	Send_Req();	/* send the request pointed to */
void	Server_Close();	/* close connection to X server */
int	Server_Open();	/* establish connection to X server */
int	Set_Timer();
void	Set_Value1();
void	Set_Value2();
void	Set_Value4();
void	Set_Byte_Sex();
void	Show_Err();
void	Show_Evt();
void	Show_Rep();
void	Show_Req();
int	Stop_Timer();
xReq	*Add_Masked_Value();
