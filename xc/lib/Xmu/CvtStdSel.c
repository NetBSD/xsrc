/* $XConsortium: CvtStdSel.c /main/37 1996/01/12 15:08:34 kaleb $ */
/* $XFree86: xc/lib/Xmu/CvtStdSel.c,v 3.6 1996/05/06 05:54:30 dawes Exp $ */

/*
 
Copyright (c) 1988  X Consortium

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
 * This file contains routines to handle common selection targets.
 *
 * Public entry points:
 *
 *	XmuConvertStandardSelection()	return a known selection
 */

#ifdef SYSVNET
#include <interlan/il_types.h>
#define __TYPES__		/* prevent #include <sys/types.h> in Xlib.h */
#include <interlan/netdb.h>
#include <interlan/socket.h>
#endif /* SYSVNET */

#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>
#include <X11/ShellP.h>
#ifdef XTHREADS
#include <X11/Xthreads.h>
#endif
#include <stdio.h>

#ifndef SYSVNET
#ifdef WIN32
#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#else
#ifndef MINIX
#ifdef Lynx
#include <sys/types.h>
#endif
#include <netdb.h>
#include <sys/socket.h>
#else
#include <net/gen/netdb.h>
#endif /* !MINIX */
#endif
#endif

#include <X11/Xos.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif
#include "Atoms.h"
#include "StdSel.h"
#include "SysUtil.h"
#include <X11/Xfuncs.h>

#ifndef OS_NAME
#ifndef X_OS_FILE
#ifdef SYSV			/* keep separate until makedepend fixed */
#define USE_UNAME
#endif
#ifdef SVR4
#define USE_UNAME
#endif
#ifdef ultrix
#define USE_UNAME
#endif
#endif /*X_OS_FILE*/
#ifdef USE_UNAME
#ifdef ultrix
#ifndef __STDC__
#include <limits.h>		/* fixed in Ultrix 3.0 */
#endif
#endif
#include <sys/utsname.h>
#endif
#endif

static char *get_os_name ()
{
#ifdef OS_NAME
	return XtNewString(OS_NAME);
#else
	FILE *f = NULL;

#ifdef USE_UNAME
	struct utsname utss;

	if (uname (&utss) == 0) {
	    char *os_name;
	    int len = strlen(utss.sysname);
#ifndef hpux				/* because of hostname length crock */
	    len += 1 + strlen(utss.release);
#endif
	    os_name = XtMalloc (len);
	    strcpy (os_name, utss.sysname);
#ifndef hpux
	    strcat (os_name, " ");
	    strcat (os_name, utss.release);
#endif
	    return os_name;
	}
#endif

#ifdef X_OS_FILE
	f = fopen(X_OS_FILE, "r");
	if (!f)
#endif
#ifdef MOTD_FILE
	       f = fopen(MOTD_FILE, "r");
#endif
	if (f) {
	    char motd[512];
	    motd[0] = '\0';
	    (void) fgets(motd, 511, f);
	    fclose(f);
	    motd[511] = '\0';
	    if (motd[0] != '\0') {
		int len = strlen(motd);
		if (motd[len - 1] == '\n')
		    motd[len - 1] = '\0';
		return XtNewString(motd);
	    }
	}

#ifdef sun
	return XtNewString("SunOS");
#else
# if !defined(SYSV) && defined(unix)
	return XtNewString("BSD");
# else
	return NULL;
# endif
#endif

#endif /*OS_NAME*/
}

/* This is a trick/kludge.  To make shared libraries happier (linking
 * against Xmu but not linking against Xt, and apparently even work
 * as we desire on SVR4, we need to avoid an explicit data reference
 * to applicationShellWidgetClass.  XtIsTopLevelShell is known
 * (implementation dependent assumption!) to use a bit flag.  So we
 * go that far.  Then, we test whether it is an applicationShellWidget
 * class by looking for an explicit class name.  Seems pretty safe.
 */
static Bool isApplicationShell(w)
    Widget w;
{
    register WidgetClass c;

    if (!XtIsTopLevelShell(w))
	return False;
    for (c = XtClass(w); c; c = c->core_class.superclass) {
	if (!strcmp(c->core_class.class_name, "ApplicationShell"))
	    return True;
    }
    return False;
}

Boolean XmuConvertStandardSelection(w, time, selection, target,
				    type, value, length, format)
    Widget w;
    Time time;
    Atom *selection, *target, *type;
    XPointer *value;
    unsigned long *length;
    int *format;
{
    Display *d = XtDisplay(w);
    if (*target == XA_TIMESTAMP(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = time;
	else {
	    long temp = time;
	    (void) memmove((char*)*value, ((char*)&temp)+sizeof(long)-4, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_HOSTNAME(d)) {
	char hostname[1024];
	hostname[0] = '\0';
	*length = XmuGetHostname (hostname, sizeof hostname);
	*value = XtNewString(hostname);
	*type = XA_STRING;
	*format = 8;
	return True;
    }
#if defined(TCPCONN) || defined(MNX_TCPCONN)
    if (*target == XA_IP_ADDRESS(d)) {
	char hostname[1024];
#if defined(XTHREADS) && defined(XUSE_MTSAFE_API)
#ifdef _POSIX_REENTRANT_FUNCTIONS
#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
#if defined(AIXV3) || defined(AIXV4) || defined(__osf__)
#define _POSIX_THREAD_SAFE_FUNCTIONS 1
#endif
#endif
#endif
#ifdef sun
#ifdef _POSIX_THREAD_SAFE_FUNCTIONS     /* Sun lies in Solaris 2.5 */
#undef _POSIX_THREAD_SAFE_FUNCTIONS
#endif
#endif
	struct hostent hent;
#ifndef _POSIX_THREAD_SAFE_FUNCTIONS
#define Gethostbyname(h) gethostbyname_r((h),&hent,hbuf,sizeof hbuf,&herr)
#define HostAddrType hent.h_addrtype
#define HostAddr hent.h_addr
#define HostLength hent.h_length
#define CallFailed NULL
	char hbuf[LINE_MAX];
	int herr;
	struct hostent *hostp;
#else
#define Gethostbyname(h) gethostbyname_r((h),&hent,&hdata)
#define HostAddrType hent.h_addrtype
#define HostAddr hent.h_addr
#define HostLength hent.h_length
#define CallFailed -1
	struct hostent_data hdata;
	int hostp;
#endif
#else
#define Gethostbyname(h) gethostbyname((h))
#define HostAddrType hostp->h_addrtype
#define HostAddr hostp->h_addr
#define HostLength hostp->h_length
#define CallFailed NULL
	struct hostent *hostp;
#endif

	hostname[0] = '\0';
	(void) XmuGetHostname (hostname, sizeof hostname);

#if defined(XTHREADS) && defined(XUSE_MTSAFE_API) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
	bzero((char*)&hdata, sizeof hdata);
#endif
	if ((hostp = Gethostbyname (hostname)) == CallFailed)
	    return False;

	if (HostAddrType != AF_INET) return False;
	*length = HostLength;
	*value = XtMalloc(*length);
	(void) memmove (*value, HostAddr, *length);
	*type = XA_NET_ADDRESS(d);
	*format = 8;
	return True;
    }
#endif
#ifdef DNETCONN
    if (*target == XA_DECNET_ADDRESS(d)) {
	return False;		/* XXX niy */
    }
#endif
    if (*target == XA_USER(d)) {
	char *name = (char*)getenv("USER");
	if (name == NULL) return False;
	*value = XtNewString(name);
	*type = XA_STRING;
	*length = strlen(name);
	*format = 8;
	return True;
    }
    if (*target == XA_CLASS(d)) {
	Widget parent = XtParent(w);
	char *class;
	int len;
	while (parent != NULL && !isApplicationShell(w)) {
	    w = parent;
	    parent = XtParent(w);
	}
	if (isApplicationShell(w))
	    class = ((ApplicationShellWidget) w)->application.class;
	else
	    class = XtClass(w)->core_class.class_name;
	*length = (len=strlen(w->core.name)) + strlen(class) + 2;
	*value = XtMalloc(*length);
	strcpy( (char*)*value, w->core.name );
	strcpy( (char*)*value+len+1, class );
	*type = XA_STRING;
	*format = 8;
	return True;
    }
    if (*target == XA_NAME(d)) {
	Widget parent = XtParent(w);

	while (parent != NULL && !XtIsWMShell(w)) {
	    w = parent;
	    parent = XtParent(w);
	}
	if (!XtIsWMShell(w)) return False;
	*value = XtNewString( ((WMShellWidget) w)->wm.title );
	*length = strlen(*value);
	*type = XA_STRING;
	*format = 8;
	return True;
    }
    if (*target == XA_CLIENT_WINDOW(d)) {
	Widget parent = XtParent(w);
	while (parent != NULL) {
	    w = parent;
	    parent = XtParent(w);
	}
	*value = XtMalloc(sizeof(Window));
	*(Window*)*value = w->core.window;
	*type = XA_WINDOW;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_OWNER_OS(d)) {
	*value = get_os_name();
	if (*value == NULL) return False;
	*type = XA_STRING;
	*length = strlen(*value);
	*format = 8;
	return True;
    }
    if (*target == XA_TARGETS(d)) {
#if defined(unix) && defined(DNETCONN)
#  define NUM_TARGETS 9
#else
#  if defined(unix) || defined(DNETCONN)
#    define NUM_TARGETS 8
#  else
#    define NUM_TARGETS 7
#  endif
#endif
	Atom* std_targets = (Atom*)XtMalloc(NUM_TARGETS*sizeof(Atom));
	int i = 0;
	std_targets[i++] = XA_TIMESTAMP(d);
	std_targets[i++] = XA_HOSTNAME(d);
	std_targets[i++] = XA_IP_ADDRESS(d);
	std_targets[i++] = XA_USER(d);
	std_targets[i++] = XA_CLASS(d);
	std_targets[i++] = XA_NAME(d);
	std_targets[i++] = XA_CLIENT_WINDOW(d);
#ifdef unix
	std_targets[i++] = XA_OWNER_OS(d);
#endif
#ifdef DNETCONN
	std_targets[i++] = XA_DECNET_ADDRESS(d);
#endif
	*value = (XPointer)std_targets;
	*type = XA_ATOM;
	*length = NUM_TARGETS;
	*format = 32;
	return True;
    }
    /* else */
    return False;
}
