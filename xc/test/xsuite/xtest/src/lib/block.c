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
 * $XConsortium: block.c,v 1.10 94/04/17 21:00:35 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	<setjmp.h>
#include	<stdio.h>
#include	<signal.h>
#include	<unistd.h>

/*
 * Possible child exit codes
 */
#define	CHILD_EXIT_BLOCKING	0
#define	CHILD_EXIT_NOBLOCKING	1
#define	CHILD_EXIT_ERROR	2

/*
 * passed as validresults argument to tet_fork
 */
#define	CHILD_EXIT_MASK		0x3

/*
 * how long child waits before generating event
 */
#define	CHILD_SLEEP_TIME	(5+(10*config.speedfactor))

/*
 * how long parent waits before timing out
 */
#define	TIMEOUT_TIME	(30+(10*config.speedfactor))

static	char	block_file[] = BLOCK_FILE;
static	jmp_buf	jumptohere;

/*
 * Global variables corresponding to block() arguments.
 * These are set by block() so that block_parent_proc() can access them.
 */
static	Display	*gdisplay;
static	XEvent	*gevent;
static	Block_Info	*ginfo;

/*
 * Status variable used to track problems encountered in block_parent_proc()
 */
static	int	parent_status;

static	void	block_child_proc();
static	void	block_parent_proc();
static	int 	blocker();

/*
 * Used to test whether or not a procedure blocks.  If event is NULL,
 * then it is assumed that the delivery of any event is sufficient
 * to unblock procedure.  See block.man for more details.
 */
Status
block(display, event, info)
Display	*display;
XEvent	*event;
Block_Info	*info;
{
	FILE	*fp;
	int	exit_status;

	gdisplay = display;
	gevent = event;
	ginfo = info;
	if (config.speedfactor < 1) {
		delete("Unsupported speedfactor value: %d", config.speedfactor);
		return(-1);
	}
	fp = fopen(block_file, "w");
	if (fp == (FILE *) NULL) {
		delete("Could not create block file: %s", block_file);
		return(-1);
	}
	if (setjmp(jumptohere)) {
		delete("Timeout in block routine");
		(void) unlink(block_file);
		return(-1);
	}
	parent_status = 1;
	exit_status = tet_fork(block_child_proc, block_parent_proc, 0, CHILD_EXIT_MASK);
	/*
	 * try removing block file just in case it still exists...
	 */
	(void) unlink(block_file);
	/*
	 * check for problems in block_parent_proc
	 */
	if (parent_status == -1)
		return(-1);
	/*
	 * verify that we got the expected event
	 * do not verify for types with Ignore_Event_Return set 
	 */
	if (!(info->p_type & Ignore_Event_Return)) {
		if (event == (XEvent *) NULL) {
			if (info->event_return.type != MappingNotify) {
				delete("Unexpected event type: %s", eventname(info->event_return.type));
				return(-1);
			}
		}
		else {
			int	err = 0;

#define	ER	((XAnyEvent *)(&(info->event_return)))
#define	ES	((XAnyEvent *)event)
			if (ER->type != ES->type) {
				delete("expected %s, got %s", eventname(ES->type), eventname(ER->type));
				err++;
			}
			if (ER->send_event != True) {
				delete("send_event not se in returned event");
				err++;
			}
			if (ER->window != ES->window) {
				delete("wrong window, expected %d, got %d", ES->window, ER->window);
				err++;
			}
			if (err)
				return(-1);
		}
	}
	switch (exit_status) {
		default:
		case CHILD_EXIT_ERROR:
			delete("Error return from block's child");
			return(-1);
		case CHILD_EXIT_BLOCKING:
			return(1);
		case CHILD_EXIT_NOBLOCKING:
			return(0);
	}
	/*NOTREACHED*/
}

/*ARGSUSED*/
static void
block_alarm(sig)
int 	sig;
{
	longjmp(jumptohere, 1);
}

/*
 * block_child_proc
 */
static void
block_child_proc()
{
	Display	*display;
	XAnyEvent *event = (XAnyEvent *) gevent;

	display = opendisplay();
	if (display == (Display *) NULL)
		exit(CHILD_EXIT_ERROR);
	sleep(CHILD_SLEEP_TIME);
	if (access(block_file, F_OK))
		exit(CHILD_EXIT_NOBLOCKING);
	if (gevent == (XEvent *) NULL) {
		int	retval;
		unsigned char	buf[512];

		retval = XGetPointerMapping(display, buf, sizeof(buf));
		if (XSetPointerMapping(display, buf, retval) != MappingSuccess)
			exit(CHILD_EXIT_ERROR);
	}
	else if (!XSendEvent(display, event->window, False, NoEventMask, gevent))
		exit(CHILD_EXIT_ERROR);
	XCloseDisplay(display);
	exit(CHILD_EXIT_BLOCKING);
}

static void
block_parent_proc()
{
	(void) signal(SIGALRM, block_alarm);
	alarm(TIMEOUT_TIME);
	parent_status = blocker(gdisplay, ginfo);
	alarm(0);
	if (parent_status == -1)
		return;
	if (access(block_file, F_OK)) {
		delete("Block file mysteriously disappeared: %s", block_file);
		parent_status = -1;
		return;
	}
	if (unlink(block_file)) {
		/*
		 * return value of unlink() does not always indicate
		 * whether or not the file was removed...pc
		 */
		if (!access(block_file, F_OK)) {
			delete("Block file could not be removed: %s", block_file);
			parent_status = -1;
			return;
		}
	}
}

/*
 * invoke the procedure which is being tested for blocking
 *
 * returns -1 on unexpected error
 */
static int
blocker(display, info)
Display	*display;
Block_Info	*info;
{
	_startcall(display);
	if (isdeleted())
		return(-1);
	switch (info->p_type) {
		default:
			delete("Unrecognized argument type in block: %x", info->p_type);
			return(-1);
		case XEventsQueued_Like:
			info->int_return = (*(info->XEventsQueued_Like_Proc))
				(display,
				info->XEventsQueued_Args.mode);
			break;
		case XPending_Like:
			info->int_return = (*(info->XPending_Like_Proc))
				(display);
			break;
		case XIfEvent_Like:
		case XPeekIfEvent_Like:
			(*(info->XIfEvent_Like_Proc))
				(display,
				&(info->event_return),
				info->XIfEvent_Args.predicate,
				info->XIfEvent_Args.arg);
			break;
		case XMaskEvent_Like:
			(*(info->XMaskEvent_Like_Proc))
				(display,
				info->XMaskEvent_Args.event_mask,
				&(info->event_return));
			break;
		case XNextEvent_Like:
		case XPeekEvent_Like:
			(*(info->XNextEvent_Like_Proc))
				(display,
				&(info->event_return));
			break;
		case XWindowEvent_Like:
			(*(info->XWindowEvent_Like_Proc))
				(display,
				info->XWindowEvent_Args.w,
				info->XWindowEvent_Args.event_mask,
				&(info->event_return));
			break;
	}
	_endcall(display);
	return(0);
}
