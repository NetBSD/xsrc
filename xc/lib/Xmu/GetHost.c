/* $XConsortium: GetHost.c,v 1.7 94/04/17 20:16:07 rws Exp $ */
/* $XFree86: xc/lib/Xmu/GetHost.c,v 3.0 1996/05/06 05:54:33 dawes Exp $ */

/*

Copyright (c) 1989  X Consortium

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

*/

/*
 * Author:  Jim Fulton, MIT X Consortium
 *
 * _XGetHostname - similar to gethostname but allows special processing.
 */

#include <X11/Xosdefs.h>
#ifndef X_NOT_STDC_ENV
#include <string.h>
#endif

#ifdef WIN32
#include <winsock.h>
#endif

#ifdef USG
#define NEED_UTSNAME
#endif

#ifdef NEED_UTSNAME
#include <sys/utsname.h>
#endif

int XmuGetHostname (buf, maxlen)
    char *buf;
    int maxlen;
{
    int len;
#ifdef WIN32
    static WSADATA wsadata;

    if (!wsadata.wVersion && WSAStartup(MAKEWORD(1,1), &wsadata))
	return -1;
#endif

#ifdef NEED_UTSNAME
    /*
     * same host name crock as in server and xinit.
     */
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* hpux */
    return len;
}
