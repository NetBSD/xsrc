/*
 * Copyright (c) 2000, 2004, 2022, Oracle and/or its affiliates.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*

Copyright 1985, 1986, 1987, 1998  The Open Group

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

X Window System is a trademark of The Open Group.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TCPCONN
#define NEEDSOCKETS
#endif
#ifdef UNIXCONN
#define NEEDSOCKETS
#endif

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <X11/Xfuncs.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <X11/Xauth.h>
#include <X11/Xmu/Error.h>
#include <stdlib.h>

#ifdef NEEDSOCKETS
#ifdef WIN32
#include <X11/Xwinsock.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif
#endif /* NEEDSOCKETS */

#ifndef WIN32
#include <arpa/inet.h>
#endif


#ifdef SECURE_RPC
#include <pwd.h>
#include <rpc/rpc.h>
#ifdef X_POSIX_C_SOURCE
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <limits.h>
#undef _POSIX_C_SOURCE
#else
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif
#ifndef NGROUPS_MAX
#include <sys/param.h>
#define NGROUPS_MAX NGROUPS
#endif
#ifdef sun
/* Go figure, there's no getdomainname() prototype available */
extern int getdomainname(char *name, size_t len);
#endif
#endif

#ifdef USE_GETTEXT
# include <locale.h>	/* setlocale()  */
# include <libintl.h>	/* gettext(), textdomain(), etc. */
#else
# define gettext(a) (a)
#endif

static int change_host(Display *dpy, char *name, Bool add);
static const char *get_hostname(XHostAddress *ha);
static int local_xerror(Display *dpy, XErrorEvent *rep);
static void nameserver_lost(int sig);

#define NAMESERVER_TIMEOUT 5	/* time to wait for nameserver */

static volatile int nameserver_timedout;

static char *ProgramName;

#ifdef NEEDSOCKETS
static int 
XFamily(int af)
{
    unsigned int i;
    static struct _familyMap {
	int af, xf;
    } familyMap[] = {
#ifdef	AF_DECnet
        { AF_DECnet, FamilyDECnet },
#endif
#ifdef	AF_CHAOS
        { AF_CHAOS, FamilyChaos },
#endif
#ifdef	AF_INET
        { AF_INET, FamilyInternet },
#if defined(IPv6) && defined(AF_INET6)
        { AF_INET6, FamilyInternet6 },
#endif
#endif
};

#define FAMILIES ((sizeof familyMap)/(sizeof familyMap[0]))

    for (i = 0; i < FAMILIES; i++)
	if (familyMap[i].af == af) return familyMap[i].xf;
    return -1;
}
#endif /* NEEDSOCKETS */

#if defined(__CYGWIN__) || defined(WIN32)
void sethostent(int x)
{}

void endhostent()
{}
#endif

int
main(int argc, char *argv[])
{
    register char *arg;
    int i, nhosts = 0;
    const char *hostname;
    int nfailed = 0;
    XHostAddress *list;
    Bool enabled = False;
    Display *dpy;
 
#ifdef USE_GETTEXT
    const char *domaindir;

    setlocale(LC_ALL, "");
    textdomain("xhost");

    /* mainly for debugging */
    if ((domaindir = getenv("TEXTDOMAINDIR")) == NULL) {
	domaindir = LOCALEDIR;
    }
    bindtextdomain("xhost", domaindir);
#endif

    ProgramName = argv[0];

    if (argc == 2 && !strcmp(argv[1], "-help")) {
	fprintf(stderr, gettext("usage: %s [[+-]hostname ...]\n"), argv[0]);
	exit(1);
    }

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
	fprintf(stderr,
		gettext("%s:  unable to open display \"%s\"\n"),
		ProgramName, XDisplayName (NULL));
	exit(1);
    }

    XSetErrorHandler(local_xerror);
 
 
    if (argc == 1) {
	sethostent(1);		/* don't close the data base each time */
	list = XListHosts(dpy, &nhosts, &enabled);
	if (enabled)
	    printf (gettext("access control enabled, only authorized clients can connect\n"));
	else
	    printf (gettext("access control disabled, clients can connect from any host\n"));

	if (nhosts != 0) {
	    for (i = 0; i < nhosts; i++ )  {
		hostname = get_hostname(&list[i]);
		if (hostname) {
		    switch (list[i].family) {
		    case FamilyInternet:
			printf("INET:");
			break;
		    case FamilyInternet6:
			printf("INET6:");
			break;
		    case FamilyDECnet:
			printf("DNET:");
			break;
		    case FamilyNetname:
			printf("NIS:");
			break;
		    case FamilyKrb5Principal:
			printf("KRB:");
			break;
		    case FamilyLocalHost:
			printf("LOCAL:");
			break;
		    case FamilyServerInterpreted:
			printf("SI:");
			break;
		    default:
			printf(gettext("<unknown family type %d>:"), list[i].family);
			break;
		    }
		    printf ("%s", hostname);
		} else {
		    printf (gettext("<unknown address in family %d>"),
			    list[i].family);
		}
		if (nameserver_timedout) {
		    printf(gettext("\t(no nameserver response within %d seconds)\n"),
			   NAMESERVER_TIMEOUT);
		    nameserver_timedout = 0;
		} else
		    printf("\n");
	    }
	    free(list);
	    endhostent();
	}
	exit(0);
    }
 
    for (i = 1; i < argc; i++) {
	arg = argv[i];
	if (*arg == '-') {
	    
	    if (!argv[i][1] && ((i+1) == argc)) {
		printf (gettext("access control enabled, only authorized clients can connect\n"));
		XEnableAccessControl(dpy);
	    } else {
		arg = argv[i][1]? &argv[i][1] : argv[++i];
		if (!change_host (dpy, arg, False)) {
		    fprintf (stderr, gettext("%s:  bad hostname \"%s\"\n"),
			     ProgramName, arg);
		    nfailed++;
		}
	    }
	} else {
	    if (*arg == '+' && !argv[i][1] && ((i+1) == argc)) {
		printf (gettext("access control disabled, clients can connect from any host\n"));
		XDisableAccessControl(dpy);
	    } else {
		if (*arg == '+') {
		    arg = argv[i][1]? &argv[i][1] : argv[++i];
		}
		if (!change_host (dpy, arg, True)) {
		    fprintf (stderr, gettext("%s:  bad hostname \"%s\"\n"),
			     ProgramName, arg);
		    nfailed++;
		}
	    }
	}
    }
    XCloseDisplay (dpy);	/* does an XSync first */
    exit(nfailed);
}

 

/*
 * change_host - edit the list of hosts that may connect to the server;
 * it parses DECnet names (expo::), Internet addresses (18.30.0.212), or
 * Internet names (expo.lcs.mit.edu); if 4.3bsd macro h_addr is defined
 * (from <netdb.h>), it will add or remove all addresses with the given
 * address.
 */

static int 
change_host(Display *dpy, char *name, Bool add)
{
    XHostAddress ha;
    char *lname;
    size_t namelen, i;
    int family = FamilyWild;
#ifdef K5AUTH
    krb5_principal princ;
    krb5_data kbuf;
#endif
#ifdef NEEDSOCKETS
    static struct in_addr addr;	/* so we can point at it */
#if defined(IPv6) && defined(AF_INET6)
    static struct in6_addr addr6; /* so we can point at it */
#else
    struct hostent *hp;
#endif
#endif
    char *cp;
    const char *add_msg = gettext("being added to access control list");
    const char *remove_msg = gettext("being removed from access control list");

    namelen = strlen(name);
    if ((lname = (char *)malloc(namelen+1)) == NULL) {
	fprintf (stderr, gettext("%s: malloc bombed in change_host\n"),
		 ProgramName);
	exit (1);
    }
    for (i = 0; i < namelen; i++) {
	lname[i] = tolower(name[i]);
    }
    lname[namelen] = '\0';
    if (!strncmp("inet:", lname, 5)) {
#ifdef TCPCONN
	family = FamilyInternet;
	name += 5;
#else
	fprintf (stderr, gettext("%s: not compiled for TCP/IP\n"), ProgramName);
	free(lname);
	return 0;
#endif
    }
    else if (!strncmp("inet6:", lname, 6)) {
#if defined(TCPCONN) && defined(IPv6) && defined(AF_INET6)
	family = FamilyInternet6;
	name += 6;
#else
	fprintf (stderr, gettext("%s: not compiled for IPv6\n"), ProgramName);
	free(lname);
	return 0;
#endif
    }
#ifdef ACCEPT_INETV6 /* Allow inetv6 as an alias for inet6 for compatibility
			with original X11 over IPv6 draft. */
    else if (!strncmp("inetv6:", lname, 7)) {
#if defined(TCPCONN) && defined(IPv6) && defined(AF_INET6)
	family = FamilyInternet6;
	name += 7;
#else
	fprintf (stderr, gettext("%s: not compiled for IPv6\n"), ProgramName);
	free(lname);
	return 0;
#endif
    }
#endif /* ACCEPT_INETV6 */
    else if (!strncmp("dnet:", lname, 5)) {
	fprintf (stderr, gettext("%s: not compiled for DECnet\n"), ProgramName);
	free(lname);
	return 0;
    }
    else if (!strncmp("nis:", lname, 4)) {
#ifdef SECURE_RPC
	family = FamilyNetname;
	name += 4;
#else
	fprintf (stderr, gettext("%s: not compiled for Secure RPC\n"), ProgramName);
	free(lname);
	return 0;
#endif
    }
    else if (!strncmp("krb:", lname, 4)) {
#ifdef K5AUTH
	family = FamilyKrb5Principal;
	name +=4;
#else
	fprintf (stderr, gettext("%s: not compiled for Kerberos 5\n"), ProgramName);
	free(lname);
	return 0;
#endif
    }
    else if (!strncmp("local:", lname, 6)) {
	family = FamilyLocalHost;
    }
    else if (!strncmp("si:", lname, 3)) {
	family = FamilyServerInterpreted;
	name += 3;
    }
    if (family == FamilyWild && (cp = strchr(lname, ':'))) {
#ifdef IPv6
	/*
	 * Check to see if inet_pton() can grok it as an IPv6 address
	 */
	if (inet_pton(AF_INET6, lname, &addr6.s6_addr) == 1) {
	    family = FamilyInternet6;
	} else
#endif
	{
	    *cp = '\0';
	    fprintf (stderr, gettext("%s: unknown address family \"%s\"\n"),
		     ProgramName, lname);
	    free(lname);
	    return 0;
	}
    }
    free(lname);

    if (family == FamilyServerInterpreted) {
	XServerInterpretedAddress siaddr;
	int rc;

	cp = strchr(name, ':');
	if (cp == NULL || cp == name) {
	    fprintf(stderr, gettext(
	  "%s: type must be specified for server interpreted family \"%s\"\n"),
	      ProgramName, name);
	    return 0;
	}
	siaddr.type = name;
	siaddr.typelength = cp - name;
	siaddr.value = ++cp;
	siaddr.valuelength = strlen(cp);
	ha.family = FamilyServerInterpreted;
	ha.address = (char *) &siaddr;
	if (add)
	    rc = XAddHost(dpy, &ha);
	else
	    rc = XRemoveHost(dpy, &ha);
	printf( "%s %s%s\n", name, rc == 1 ? "" : "failed when ",
		add ? add_msg : remove_msg);
	if (rc != 1)
	    return 0;
	return 1;
    }

#ifdef K5AUTH
    if (family == FamilyKrb5Principal) {
	krb5_error_code retval;

	retval = krb5_parse_name(name, &princ);
	if (retval) {
	    krb5_init_ets();	/* init krb errs for error_message() */
	    fprintf(stderr, gettext("%s: cannot parse Kerberos name: %s\n"),
		    ProgramName, error_message(retval));
	    return 0;
	}
	XauKrb5Encode(princ, &kbuf);
	ha.length = kbuf.length;
	ha.address = kbuf.data;
	ha.family = family;
	if (add)
	    XAddHost(dpy, &ha);
	else
	    XRemoveHost(dpy, &ha);
	krb5_free_principal(princ);
	free(kbuf.data);
	printf( "%s %s\n", name, add ? add_msg : remove_msg);
	return 1;
    }
#endif
    if (family == FamilyLocalHost) {
	char empty[] = "";
	ha.length = 0;
	ha.address = empty;
	ha.family = family;
	if (add)
	    XAddHost(dpy, &ha);
	else
	    XRemoveHost(dpy, &ha);
	printf( gettext("non-network local connections %s\n"), add ? add_msg : remove_msg);
	return 1;
    }
    /*
     * If it has an '@', it's a netname
     */
    if ((family == FamilyNetname && (cp = strchr(name, '@'))) ||
	(cp = strchr(name, '@'))) {
        char *netname = name;
#ifdef SECURE_RPC
	static char username[MAXNETNAMELEN];

	if (!cp[1]) {
	    struct passwd *pwd;
	    static char domainname[128];

	    *cp = '\0';
	    pwd = getpwnam(name);
	    if (!pwd) {
		fprintf(stderr, gettext("no such user \"%s\"\n"), name);
		return 0;
	    }
	    getdomainname(domainname, sizeof(domainname));
	    if (!user2netname(username, pwd->pw_uid, domainname)) {
		fprintf(stderr, gettext("failed to get netname for \"%s\"\n"), name);
		return 0;
	    }
	    netname = username;
	}
#endif
	ha.family = FamilyNetname;
	ha.length = strlen(netname);
	ha.address = netname;
	if (add)
	    XAddHost (dpy, &ha);
	else
	    XRemoveHost (dpy, &ha);
	if (netname != name)
	    printf ("%s@ (%s) %s\n", name, netname, add ? add_msg : remove_msg);
	else
	    printf ("%s %s\n", netname, add ? add_msg : remove_msg);
        return 1;
    }
#ifdef NEEDSOCKETS
    /*
     * First see if inet_aton/inet_addr can grok the name; if so, then use it.
     */
    if (((family == FamilyWild) || (family == FamilyInternet)) &&
#ifdef HAVE_INET_ATON
	(inet_aton (name, &addr) != 0)
#else
	((addr.s_addr = inet_addr(name)) != -1)
#endif
        ) {
	ha.family = FamilyInternet;
	ha.length = sizeof(addr.s_addr);
	ha.address = (char *) &addr.s_addr;
	if (add) {
	    XAddHost (dpy, &ha);
	    printf ("%s %s\n", name, add_msg);
	} else {
	    XRemoveHost (dpy, &ha);
	    printf ("%s %s\n", name, remove_msg);
	}
	return 1;
    } 
#if defined(IPv6) && defined(AF_INET6)
    /*
     * Check to see if inet_pton() can grok it as an IPv6 address
     */
    else if (((family == FamilyWild) || (family == FamilyInternet6)) &&
	     (inet_pton(AF_INET6, name, &addr6.s6_addr) == 1)) {
	ha.family = FamilyInternet6;
	ha.length = sizeof(addr6.s6_addr);		
	ha.address = (char *) &addr6.s6_addr; 
	if (add) {
	    XAddHost (dpy, &ha);
	    printf ("%s %s\n", name, add_msg);
	} else {
	    XRemoveHost (dpy, &ha);
	    printf ("%s %s\n", name, remove_msg);
	}
	return 1;
    } else {
    /*
     * Is it in the namespace?  
     *
     * If no family was specified, use both Internet v4 & v6 addresses.
     * Otherwise, use only addresses matching specified family.
     */
	struct addrinfo *addresses;
	struct addrinfo *a;
	Bool didit = False;

	if (getaddrinfo(name, NULL, NULL, &addresses) != 0)
	    return 0;

	for (a = addresses; a != NULL; a = a->ai_next) {
	    if ( ((a->ai_family == AF_INET) && (family != FamilyInternet6))
	      || ((a->ai_family == AF_INET6) && (family != FamilyInternet)) ) {
		char ad[INET6_ADDRSTRLEN];
		ha.family = XFamily(a->ai_family);
		if (a->ai_family == AF_INET6) {
		    ha.address = (char *)
		      &((struct sockaddr_in6 *) a->ai_addr)->sin6_addr;
		    ha.length = 
		      sizeof (((struct sockaddr_in6 *) a->ai_addr)->sin6_addr);
		} else {
		    ha.address = (char *)
		      &((struct sockaddr_in *) a->ai_addr)->sin_addr;
		    ha.length = 
		      sizeof (((struct sockaddr_in *) a->ai_addr)->sin_addr);
		}
		inet_ntop(a->ai_family, ha.address, ad, sizeof(ad));
	/* printf("Family: %d\nLength: %d\n", a->ai_family, ha.length); */
		/* printf("Address: %s\n", ad); */

		if (add) {
		    XAddHost (dpy, &ha);
		} else {
		    XRemoveHost (dpy, &ha);
		}
		didit = True;
	    }
	}
	if (didit == True) {
	    printf ("%s %s\n", name, add ? add_msg : remove_msg);
	} else {
	    const char *familyMsg = "";

	    if (family == FamilyInternet6) {
		familyMsg = "inet6 ";
	    } else if (family == FamilyInternet) {
		familyMsg = "inet ";
	    }

	    fprintf(stderr, gettext("%s: unable to get %saddress for \"%s\"\n"),
		ProgramName, familyMsg, name);
	}
	freeaddrinfo(addresses);
	return 1;
    }
#else /* !IPv6 */
    /*
     * Is it in the namespace?
     */
    else if (((hp = gethostbyname(name)) == (struct hostent *)NULL)
	     || hp->h_addrtype != AF_INET) {
	return 0;
    } else {
	ha.family = XFamily(hp->h_addrtype);
	ha.length = hp->h_length;
#ifdef h_addr			/* new 4.3bsd version of gethostent */
    {
	char **list;

	/* iterate over the hosts */
	for (list = hp->h_addr_list; *list; list++) {
	    ha.address = *list;
	    if (add) {
		XAddHost (dpy, &ha);
	    } else {
		XRemoveHost (dpy, &ha);
	    }
	}
    }
#else
	ha.address = hp->h_addr;
	if (add) {
	    XAddHost (dpy, &ha);
	} else {
	    XRemoveHost (dpy, &ha);
	}
#endif
	printf ("%s %s\n", name, add ? add_msg : remove_msg);
	return 1;
    }
#endif /* IPv6 */
#else /* NEEDSOCKETS */
    return 0;
#endif /* NEEDSOCKETS */
}


/*
 * get_hostname - Given an internet address, return a name (CHARON.MIT.EDU)
 * or a string representing the address (18.58.0.13) if the name cannot
 * be found.
 */


static const char *
get_hostname(XHostAddress *ha)
{
#if defined(TCPCONN) && (!defined(IPv6) || !defined(AF_INET6))
    static struct hostent *hp = NULL;
#endif
#ifdef K5AUTH
    krb5_principal princ;
    krb5_data kbuf;
    char *kname;
    static char kname_out[255];
#endif
#ifdef SIGALRM
    struct sigaction sa;
#endif

#ifdef TCPCONN
#if defined(IPv6) && defined(AF_INET6)
    if ((ha->family == FamilyInternet) || (ha->family == FamilyInternet6)) {
	struct sockaddr_storage saddr;
	static char inetname[NI_MAXHOST];
	unsigned int saddrlen;

	inetname[0] = '\0';
	memset(&saddr, 0, sizeof saddr);
	if (ha->family == FamilyInternet) {
	    struct sockaddr_in *sin = (struct sockaddr_in *) &saddr;
#ifdef BSD44SOCKETS
	    sin->sin_len = sizeof(struct sockaddr_in);
#endif
	    sin->sin_family = AF_INET;
	    sin->sin_port = 0;
	    if (sizeof(sin->sin_addr) > ha->length)
		return "";
	    memcpy(&sin->sin_addr, ha->address, sizeof(sin->sin_addr));
	    saddrlen = sizeof(struct sockaddr_in);
	} else {
	    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &saddr;
#ifdef SIN6_LEN
	    sin6->sin6_len = sizeof(struct sockaddr_in6);
#endif
	    sin6->sin6_family = AF_INET6;
	    sin6->sin6_port = 0;
	    if (sizeof(sin6->sin6_addr) > ha->length)
		return "";
	    memcpy(&sin6->sin6_addr, ha->address, sizeof(sin6->sin6_addr));
	    saddrlen = sizeof(struct sockaddr_in6);
	}

	/* gethostbyaddr can take a LONG time if the host does not exist.
	   Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
	   that something is wrong and do not make the user wait.
	   gethostbyaddr will continue after a signal, so we have to
	   jump out of it. 
	   */
#ifdef SIGALRM
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = nameserver_lost;
	sa.sa_flags = 0;	/* don't restart syscalls */
	sigaction(SIGALRM, &sa, NULL);
	alarm(NAMESERVER_TIMEOUT);
#endif
	getnameinfo((struct sockaddr *) &saddr, saddrlen, inetname,
		    sizeof(inetname), NULL, 0, 0);
#ifdef SIGALRM
	alarm(0);
#endif
	if (nameserver_timedout || inetname[0] == '\0')
	    inet_ntop(((struct sockaddr *)&saddr)->sa_family, ha->address,
		inetname, sizeof(inetname));
	return inetname;	      
    }
#else
    if (ha->family == FamilyInternet) {
	/* gethostbyaddr can take a LONG time if the host does not exist.
	   Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
	   that something is wrong and do not make the user wait.
	   gethostbyaddr will continue after a signal, so we have to
	   jump out of it. 
	   */
#ifdef SIGALRM
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = nameserver_lost;
	sa.sa_flags = 0;	/* don't restart syscalls */
	sigaction(SIGALRM, &sa, NULL);
	alarm(4);
#endif
	hp = gethostbyaddr (ha->address, ha->length, AF_INET);
#ifdef SIGALRM
	alarm(0);
#endif
	if (hp)
	    return (hp->h_name);
	else return (inet_ntoa(*((struct in_addr *)(ha->address))));
    }
#endif /* IPv6 */
#endif
    if (ha->family == FamilyNetname) {
	static char netname[512];
	int len;
#ifdef SECURE_RPC
	int gidlen;
	uid_t uid;
	gid_t gid, gidlist[NGROUPS_MAX];
#endif

	if (ha->length < sizeof(netname) - 1)
	    len = ha->length;
	else
	    len = sizeof(netname) - 1;
	memmove( netname, ha->address, len);
	netname[len] = '\0';
#ifdef SECURE_RPC
	if (netname2user(netname, &uid, &gid, &gidlen, gidlist)) {
	    struct passwd *pwd;

	    pwd = getpwuid(uid);
	    if (pwd)
		snprintf(netname, sizeof(netname), "%s@ (%*.*s)",
			 pwd->pw_name, ha->length, ha->length, ha->address);
	}
#endif
	return (netname);
    }
#ifdef K5AUTH
    if (ha->family == FamilyKrb5Principal) {
	kbuf.data = ha->address;
	kbuf.length = ha->length;
	XauKrb5Decode(kbuf, &princ);
	krb5_unparse_name(princ, &kname);
	krb5_free_principal(princ);
	strncpy(kname_out, kname, sizeof (kname_out));
	free(kname);
	return kname_out;
    }
#endif
    if (ha->family == FamilyLocalHost) {
	return "";
    }
    if (ha->family == FamilyServerInterpreted) {
	XServerInterpretedAddress *sip;
	static char *addressString;
	static size_t addressStringSize;
	size_t neededSize;

	sip = (XServerInterpretedAddress *) ha->address;
	neededSize = sip->typelength + sip->valuelength + 2;

	if (addressStringSize < neededSize) {
	    if (addressString != NULL) {
		free(addressString);
	    }
	    addressStringSize = neededSize;
	    addressString = malloc(addressStringSize);
	}
	if (addressString != NULL) {
	    char *cp = addressString;

	    memcpy(cp, sip->type, sip->typelength);
	    cp += sip->typelength;
	    *cp++ = ':';
	    memcpy(cp, sip->value, sip->valuelength);
	    cp += sip->valuelength;
	    *cp = '\0';
	}
	return addressString;
    }
    return (NULL);
}

/*ARGUSED*/
static void
nameserver_lost(_X_UNUSED int sig)
{
    nameserver_timedout = 1;
}

/*
 * local_xerror - local non-fatal error handling routine. If the error was
 * that an X_GetHosts request for an unknown address format was received, just
 * return, otherwise print the normal error message and continue.
 */
static int 
local_xerror(Display *dpy, XErrorEvent *rep)
{
    if ((rep->error_code == BadAccess) && (rep->request_code == X_ChangeHosts)) {
	fprintf (stderr, 
		 gettext("%s:  must be on local machine to add or remove hosts.\n"),
		 ProgramName);
	return 1;
    } else if ((rep->error_code == BadAccess) && 
	       (rep->request_code == X_SetAccessControl)) {
	fprintf (stderr, 
		 gettext("%s:  must be on local machine to enable or disable access control.\n"),
		 ProgramName);
	return 1;
    } else if ((rep->error_code == BadValue) && 
	       (rep->request_code == X_ListHosts)) {
	return 1;
    }

    XmuPrintDefaultErrorMessage (dpy, rep, stderr);
    return 0;
}
