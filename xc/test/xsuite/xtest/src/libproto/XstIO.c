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
 * $XConsortium: XstIO.c,v 1.11 94/04/17 21:01:38 rws Exp $
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
#include "XstosInt.h"

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

/*
 * The following routines are internal routines used by Xst for protocol
 * packet transmission and reception.
 *
 * XstIOError() will be called if any sort of system call error occurs.
 * This is assumed to be a fatal condition, i.e., XstIOError should not return.
 *
 * Routines declared with a return type of 'Status' return 0 on failure,
 * and non 0 on success.  Routines with no declared return type don't 
 * return anything.  Whenever possible routines that create objects return
 * the object they have created.
 */


long Xst_br;
long Xst_tr;
long Xst_size;

/* 
 * Xst_Read - Read bytes from the socket taking into account incomplete
 * reads.  This routine may have to be reworked if int < long.
 * This routine MUST be bracketted by Start/Stop_Timer calls as some
 * SVR4 systems erroneously return EAGAIN from closed connections so
 * we rely on the timer to rescue us.
 */
int
Xst_Read (dpy, data, size)
register    XstDisplay * dpy;
register char  *data;
register long   size;
{
    long   bytes_read = 0;
    long   this_read;


    if (size == 0) {
	return(0);
    }

    Xst_br = 0;
    Xst_tr = 0;
    Xst_size = size;

    Reset_Some();
    while (1) {
	Xst_tr = this_read = ReadFromServer (dpy -> fd, data, (int) size);
	Log_Some("Xst_Read(%d, 0x%x, %d) -> %d, errno = %d\n",dpy->fd,(unsigned)data,(int)size,(int)this_read,(int)errno);
	if (this_read == size)
		break;
	if (this_read > 0) {
	    size -= this_read;
	    data += this_read;
	    bytes_read += this_read;
	    Xst_size = size;
	    Xst_br = bytes_read;
	}

	else if (this_read == 0) {
		_XstWaitForReadable(dpy);
	}
	else {
		/* if we would block then round again, rely on timer to
		 * rescue us from spinning forever (known SVR4 bug).
		 */
#ifdef EWOULDBLOCK
		if (errno == EWOULDBLOCK)
			_XstWaitForReadable(dpy);
		else
#endif
#ifdef EAGAIN
		if (errno == EAGAIN)
			_XstWaitForReadable(dpy);
		else
#endif
#ifdef EINTR
		if (errno == EINTR)
			_XstWaitForReadable(dpy);
		else
#endif
		{
		    Reset_Some();
		    Log_Debug("Xst_Read(%d, 0x%x, %d) returning %d, errno = %d\n",
			dpy->fd,(unsigned)data,(int)size,(int)this_read,(int)errno);
		    return(this_read);
		}
	}
    }
    bytes_read += this_read;
    Xst_br = bytes_read;
    Xst_tr = this_read;
    Reset_Some();
    Log_Debug("Xst_Read(%d, 0x%x, %d) returning %d, errno = OK\n",
	dpy->fd,(unsigned)data,(int)size,(int)this_read);
    return(bytes_read);
}

/*ARGSUSED*/
void XstIOError (dpy,str,incperror)
XstDisplay * dpy;
char *str;
int incperror;	/* include system error info */
{
    char emsg[132];

    if (incperror) {
	if ((errno >= 0) && (errno < sys_nerr)) {
	    (void) strcpy(emsg,sys_errlist[errno]);
	}
	else {
	    sprintf(emsg,"UNKNOWN ERROR - %d",errno);
	}
    }
    else {
	emsg[0]='\0';
    }
    Log_Msg ("%s %s\n",str,emsg);
    Delete ();
}
