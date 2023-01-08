/*
 *
Copyright 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 */


/*
 * Chooser - display a menu of names and let the user select one
 */

/*
 * Layout:
 *
 *  +--------------------------------------------------+
 *  |             +------------------+                 |
 *  |             |      Label       |                 |
 *  |             +------------------+                 |
 *  |    +-+--------------+                            |
 *  |    |^| name-1       |                            |
 *  |    ||| name-2       |                            |
 *  |    |v| name-3       |                            |
 *  |    | | name-4       |                            |
 *  |    | | name-5       |                            |
 *  |    | | name-6       |                            |
 *  |    +----------------+                            |
 *  |    cancel  accept  ping                          |
 *  +--------------------------------------------------+
 */

#include    <X11/Intrinsic.h>
#include    <X11/StringDefs.h>
#include    <X11/Xatom.h>

#include    <X11/Xaw/Paned.h>
#include    <X11/Xaw/Label.h>
#include    <X11/Xaw/Viewport.h>
#include    <X11/Xaw/List.h>
#include    <X11/Xaw/Box.h>
#include    <X11/Xaw/Command.h>

#include    "dm.h"

#include    <X11/Xdmcp.h>

#include    <sys/types.h>
#include    <stdio.h>
#include    <ctype.h>

#ifdef USE_XINERAMA
# include    <X11/extensions/Xinerama.h>
#endif

#if defined(SVR4)
# include    <sys/sockio.h>
#endif
#if defined(SVR4) && defined(PowerMAX_OS)
# include    <sys/stropts.h>
#endif
#if defined(SYSV) && defined(i386)
# include    <sys/stream.h>
#endif

#include    "dm_socket.h"

#include    <arpa/inet.h>

#include    <sys/ioctl.h>

#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
# ifdef BSD
#  if (BSD >= 199103)
#   define VARIABLE_IFREQ
#  endif
# endif
#endif

#ifdef XKB
# include <X11/extensions/XKBbells.h>
#endif

#define BROADCAST_HOSTNAME  "BROADCAST"

#ifndef ishexdigit
# define ishexdigit(c)	(isdigit(c) || ('a' <= (c) && (c) <= 'f'))
#endif

#ifdef hpux
# include <sys/utsname.h>
# ifdef HAS_IFREQ
#  include <net/if.h>
# endif
#else
# ifdef __convex__
#  include <sync/queue.h>
#  include <sync/sema.h>
# endif
# include <net/if.h>
#endif /* hpux */

#include    <netdb.h>
#include    <X11/keysym.h>

static int FromHex (char *s, char *d, int len);
static int oldline;

static Widget	    toplevel, label, viewport, paned, list, box, cancel, acceptit, ping;

static void	CvtStringToARRAY8(
    XrmValuePtr	args,
    Cardinal	*num_args,
    XrmValuePtr	fromVal,
    XrmValuePtr	toVal);

static struct _app_resources {
    ARRAY8Ptr   xdmAddress;
    ARRAY8Ptr	clientAddress;
    int		connectionType;
} app_resources;

#define offset(field) XtOffsetOf(struct _app_resources, field)

#define XtRARRAY8   "ARRAY8"

static XtResource  resources[] = {
    {"xdmAddress",	"XdmAddress",  XtRARRAY8,	sizeof (ARRAY8Ptr),
	offset (xdmAddress),	    XtRString,	NULL },
    {"clientAddress",	"ClientAddress",  XtRARRAY8,	sizeof (ARRAY8Ptr),
	offset (clientAddress),	    XtRString,	NULL },
    {"connectionType",	"ConnectionType",   XtRInt,	sizeof (int),
	offset (connectionType),    XtRImmediate,	(XtPointer) 0 }
};
#undef offset

static XrmOptionDescRec options[] = {
    { "-xdmaddress",	"*xdmAddress",	    XrmoptionSepArg,	NULL },
    { "-clientaddress",	"*clientAddress",   XrmoptionSepArg,	NULL },
    { "-connectionType","*connectionType",  XrmoptionSepArg,	NULL },
};

typedef struct _hostAddr {
    struct _hostAddr	*next;
    struct sockaddr	*addr;
    int			addrlen;
    xdmOpCode		type;
} HostAddr;

static HostAddr    *hostAddrdb;

typedef struct _hostName {
    struct _hostName	*next;
    char		*fullname;
    int			willing;
    ARRAY8		hostname, status;
    CARD16		connectionType;
    ARRAY8		hostaddr;
} HostName;

static HostName    *hostNamedb;

static int  socketFD;
#if defined(IPv6) && defined(AF_INET6)
static int  socket6FD;
#endif

static int  pingTry;

#define PING_INTERVAL	2000
#define TRIES		3

static XdmcpBuffer	directBuffer, broadcastBuffer;
static XdmcpBuffer	buffer;

#if (defined(SVR4) && !defined(sun) && !defined(__sgi) && !defined(NCR)) && defined(SIOCGIFCONF)

/* Deal with different SIOCGIFCONF ioctl semantics on these OSs */

static int
ifioctl (int fd, int cmd, char *arg)
{
    struct strioctl ioc;
    int ret;

    bzero((char *) &ioc, sizeof(ioc));
    ioc.ic_cmd = cmd;
    ioc.ic_timout = 0;
    if (cmd == SIOCGIFCONF)
    {
	ioc.ic_len = ((struct ifconf *) arg)->ifc_len;
	ioc.ic_dp = ((struct ifconf *) arg)->ifc_buf;
    }
    else
    {
	ioc.ic_len = sizeof(struct ifreq);
	ioc.ic_dp = arg;
    }
    ret = ioctl(fd, I_STR, (char *) &ioc);
    if (ret >= 0 && cmd == SIOCGIFCONF)
	((struct ifconf *) arg)->ifc_len = ioc.ic_len;
    return(ret);
}
#else /* (SVR4 && !sun && !NCR) && SIOCGIFCONF */
# define ifioctl ioctl
#endif /* (SVR4 && !sun && !NCR) && SIOCGIFCONF */


/* ARGSUSED */
static void
PingHosts (XtPointer closure, XtIntervalId *id)
{
    HostAddr	*hosts;
    int		 sfd = socketFD;

    for (hosts = hostAddrdb; hosts; hosts = hosts->next)
    {
#if defined(IPv6) && defined(AF_INET6)
	if ( ((struct sockaddr *) hosts->addr)->sa_family == AF_INET6 )
	    sfd = socket6FD;
	else
	    sfd = socketFD;
#endif
	if (hosts->type == QUERY)
	    XdmcpFlush (sfd, &directBuffer,
			(XdmcpNetaddr) hosts->addr, hosts->addrlen);
	else
	    XdmcpFlush (sfd, &broadcastBuffer,
			(XdmcpNetaddr) hosts->addr, hosts->addrlen);
    }
    if (++pingTry < TRIES)
	XtAddTimeOut (PING_INTERVAL, PingHosts, (XtPointer) 0);
}

char	**NameTable;
int	NameTableSize;

static int
HostnameCompare (const void *a, const void *b)
{
    return strcmp (*(char **)a, *(char **)b);
}

static void
RebuildTable (int size)
{
    char	**newTable = NULL;
    HostName	*names;
    int		i;

    if (size)
    {
	newTable = malloc (size * sizeof (char *));
	if (!newTable)
	    return;
	for (names = hostNamedb, i = 0; names; names = names->next, i++)
	    newTable[i] = names->fullname;
	qsort (newTable, size, sizeof (char *), HostnameCompare);
    }
    XawListChange (list, newTable, size, 0, TRUE);
    free (NameTable);
    NameTable = newTable;
    NameTableSize = size;
}

static int
AddHostname (ARRAY8Ptr hostname, ARRAY8Ptr status, struct sockaddr *addr, int willing)
{
    HostName	*new, **names, *name;
    ARRAY8	hostAddr = {0, NULL};
    CARD16	connectionType;
    int		fulllen;

    switch (addr->sa_family)
    {
    case AF_INET:
	hostAddr.data = (CARD8 *) &((struct sockaddr_in *) addr)->sin_addr;
	hostAddr.length = 4;
	connectionType = FamilyInternet;
	break;
#if defined(IPv6) && defined(AF_INET6)
    case AF_INET6:
	hostAddr.data = (CARD8 *) &((struct sockaddr_in6 *) addr)->sin6_addr;
	hostAddr.length = 16;
	connectionType = FamilyInternet6;
	break;
#endif
    default:
	hostAddr.data = (CARD8 *) "";
	hostAddr.length = 0;
	connectionType = FamilyLocal;
	break;
    }
    for (names = &hostNamedb; *names; names = & (*names)->next)
    {
	name = *names;
	if (connectionType == name->connectionType &&
	    XdmcpARRAY8Equal (&hostAddr, &name->hostaddr))
	{
	    if (XdmcpARRAY8Equal (status, &name->status))
	    {
		return 0;
	    }
	    break;
	}
    }
    if (!*names)
    {
	new = calloc (1, sizeof (HostName));
	if (!new)
	    return 0;
	if (hostname->length)
	{
	    switch (addr->sa_family)
	    {
	    case AF_INET:
#if defined(IPv6) && defined(AF_INET6)
	    case AF_INET6:
#endif
		{
		    struct hostent  *hostent;
		    char	    *host;

		    hostent = gethostbyaddr ((char *)hostAddr.data, hostAddr.length, addr->sa_family);
		    if (hostent)
		    {
			XdmcpDisposeARRAY8 (hostname);
			host = (char *)hostent->h_name;
			XdmcpAllocARRAY8 (hostname, strlen (host));
			memcpy(hostname->data, host, hostname->length);
		    }
		}
	    }
	}
	if (!XdmcpAllocARRAY8 (&new->hostaddr, hostAddr.length))
	{
	    free (new);
	    return 0;
	}
	memcpy(new->hostaddr.data, hostAddr.data, hostAddr.length);
	new->connectionType = connectionType;
	new->hostname = *hostname;

	*names = new;
	new->next = NULL;
	NameTableSize++;
    }
    else
    {
	new = *names;
	free (new->fullname);
	XdmcpDisposeARRAY8 (&new->status);
	XdmcpDisposeARRAY8 (hostname);
    }
    new->willing = willing;
    new->status = *status;

    hostname = &new->hostname;
    fulllen = hostname->length;
    if (fulllen < 30)
	fulllen = 30;
    fulllen += status->length + 10;
    new->fullname = malloc (fulllen);
    if (!new->fullname)
    {
	new->fullname = "Unknown";
    }
    else
    {
	snprintf(new->fullname, fulllen, "%-30.*s %*.*s",
		 hostname->length, hostname->data,
		 status->length, status->length, status->data);
    }
    RebuildTable (NameTableSize);
    return 1;
}

static void
DisposeHostname (HostName *host)
{
    XdmcpDisposeARRAY8 (&host->hostname);
    XdmcpDisposeARRAY8 (&host->hostaddr);
    XdmcpDisposeARRAY8 (&host->status);
    free (host->fullname);
    free (host);
}

static void
EmptyHostnames (void)
{
    HostName	*hosts, *next;

    for (hosts = hostNamedb; hosts; hosts = next)
    {
	next = hosts->next;
	DisposeHostname (hosts);
    }
    NameTableSize = 0;
    hostNamedb = NULL;
    RebuildTable (NameTableSize);
}

/* ARGSUSED */
static void
ReceivePacket (XtPointer closure, int *source, XtInputId *id)
{
    XdmcpHeader	    header;
    ARRAY8	    authenticationName = {0, NULL};
    ARRAY8	    hostname = {0, NULL};
    ARRAY8	    status = {0, NULL};
    int		    saveHostname = 0;
#if defined(IPv6) && defined(AF_INET6)
    struct sockaddr_storage addr;
#else
    struct sockaddr addr;
#endif
    int		    addrlen;
    int		    sfd = * (int *) closure;

    addrlen = sizeof (addr);
    if (!XdmcpFill (sfd, &buffer, (XdmcpNetaddr) &addr, &addrlen))
	return;
    if (!XdmcpReadHeader (&buffer, &header))
	return;
    if (header.version != XDM_PROTOCOL_VERSION)
	return;
    switch (header.opcode) {
    case WILLING:
	if (XdmcpReadARRAY8 (&buffer, &authenticationName) &&
	    XdmcpReadARRAY8 (&buffer, &hostname) &&
	    XdmcpReadARRAY8 (&buffer, &status))
	{
	    if (header.length == 6 + authenticationName.length +
		hostname.length + status.length)
	    {
		if (AddHostname (&hostname, &status, (struct sockaddr *) &addr,
				 header.opcode == (int) WILLING))
		    saveHostname = 1;
	    }
	}
	XdmcpDisposeARRAY8 (&authenticationName);
	break;
    case UNWILLING:
	if (XdmcpReadARRAY8 (&buffer, &hostname) &&
	    XdmcpReadARRAY8 (&buffer, &status))
	{
	    if (header.length == 4 + hostname.length + status.length)
	    {
		if (AddHostname (&hostname, &status, (struct sockaddr *) &addr,
				 header.opcode == (int) WILLING))
		    saveHostname = 1;

	    }
	}
	break;
    default:
	break;
    }
    if (!saveHostname)
    {
	XdmcpDisposeARRAY8 (&hostname);
	XdmcpDisposeARRAY8 (&status);
    }
}

static void
RegisterHostaddr (struct sockaddr *addr, int len, xdmOpCode type)
{
    HostAddr		*host, **prev;

    host = malloc (sizeof (HostAddr));
    if (!host)
	return;
    host->addr = malloc (len);
    if (!host->addr)
    {
	free (host);
	return;
    }
    memcpy(host->addr, addr, len);
    host->addrlen = len;
    host->type = type;
    for (prev = &hostAddrdb; *prev; prev = &(*prev)->next)
	;
    *prev = host;
    host->next = NULL;
}

/*
 * Register the address for this host.
 * Called with each of the names on the command line.
 * The special name "BROADCAST" looks up all the broadcast
 *  addresses on the local host.
 */

/* Handle variable length ifreq in BNR2 and later */
#ifdef VARIABLE_IFREQ
# define ifr_size(p) (sizeof (struct ifreq) + \
		     (p->ifr_addr.sa_len > sizeof (p->ifr_addr) ? \
		      p->ifr_addr.sa_len - sizeof (p->ifr_addr) : 0))
#else
# define ifr_size(p) (sizeof (struct ifreq))
#endif

static void
RegisterHostname (char *name)
{
#if !defined(IPv6) || !defined(AF_INET6)
    struct hostent	*hostent;
#endif
    struct sockaddr_in	in_addr;
    struct ifconf	ifc;
    register struct ifreq *ifr;
    struct sockaddr	broad_addr;
    char		buf[2048], *cp, *cplim;

    if (!strcmp (name, BROADCAST_HOSTNAME))
    {
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ifioctl (socketFD, (int) SIOCGIFCONF, (char *) &ifc) < 0)
	    return;

	cplim = (char *) ifc.ifc_req + ifc.ifc_len;

	for (cp = (char *) ifc.ifc_req; cp < cplim; cp += ifr_size (ifr))
	{
	    ifr = (struct ifreq *) cp;
	    if (ifr->ifr_addr.sa_family != AF_INET)
		continue;

	    broad_addr = ifr->ifr_addr;
	    ((struct sockaddr_in *) &broad_addr)->sin_addr.s_addr =
		htonl (INADDR_BROADCAST);
#ifdef SIOCGIFBRDADDR
	    {
		struct ifreq    broad_req;

		broad_req = *ifr;
		if (ifioctl (socketFD, SIOCGIFFLAGS, (char *) &broad_req) != -1 &&
		    (broad_req.ifr_flags & IFF_BROADCAST) &&
		    (broad_req.ifr_flags & IFF_UP)
		    )
		{
		    broad_req = *ifr;
		    if (ifioctl (socketFD, SIOCGIFBRDADDR, &broad_req) != -1)
			broad_addr = broad_req.ifr_addr;
		    else
			continue;
		}
		else
		    continue;
	    }
#endif
	    in_addr = *((struct sockaddr_in *) &broad_addr);
	    in_addr.sin_port = htons (XDM_UDP_PORT);
#ifdef BSD44SOCKETS
	    in_addr.sin_len = sizeof(in_addr);
#endif
	    RegisterHostaddr ((struct sockaddr *)&in_addr, sizeof (in_addr),
			      BROADCAST_QUERY);
	}
    }
    else
    {
	/* address as hex string, e.g., "12180022" (deprecated) */
	if (strlen(name) == 8 &&
	    FromHex(name, (char *)&in_addr.sin_addr, strlen(name)) == 0)
	{
	    in_addr.sin_family = AF_INET;
	    in_addr.sin_port = htons (XDM_UDP_PORT);
#ifdef BSD44SOCKETS
	    in_addr.sin_len = sizeof(in_addr);
#endif
	    RegisterHostaddr ((struct sockaddr *)&in_addr, sizeof (in_addr),
				QUERY);
	}
#if defined(IPv6) && defined(AF_INET6)
	else {
	    char sport[8];
	    struct addrinfo *ai, *nai, hints;
	    bzero(&hints,sizeof(hints));
	    hints.ai_socktype = SOCK_DGRAM;
	    snprintf(sport, sizeof(sport), "%d", XDM_UDP_PORT);
	    if (getaddrinfo(name, sport, &hints, &ai) == 0) {
		for (nai = ai ; nai != NULL ; nai = nai->ai_next) {
		    if ((nai->ai_family == AF_INET) ||
		        (nai->ai_family == AF_INET6)) {
			if (((nai->ai_family == AF_INET) &&
			  IN_MULTICAST(((struct sockaddr_in *) nai->ai_addr)
			    ->sin_addr.s_addr))
			  || ((nai->ai_family == AF_INET6) &&
			    IN6_IS_ADDR_MULTICAST(
				&((struct sockaddr_in6 *) nai->ai_addr)
				  ->sin6_addr)))
			{
			    RegisterHostaddr(nai->ai_addr, nai->ai_addrlen,
			      BROADCAST_QUERY);
			} else {
			    RegisterHostaddr(nai->ai_addr, nai->ai_addrlen,
			      QUERY);
			}
		    }
		}
		freeaddrinfo(ai);
	    }
	}
#else
	/* Per RFC 1123, check first for IP address in dotted-decimal form */
	else if ((in_addr.sin_addr.s_addr = inet_addr(name)) != -1)
	    in_addr.sin_family = AF_INET;
	else
	{
	    hostent = gethostbyname (name);
	    if (!hostent)
		return;
	    if (hostent->h_addrtype != AF_INET || hostent->h_length != 4)
		return;
	    in_addr.sin_family = hostent->h_addrtype;
	    memcpy(&in_addr.sin_addr, hostent->h_addr, 4);
	}
	in_addr.sin_port = htons (XDM_UDP_PORT);
# ifdef BSD44SOCKETS
	in_addr.sin_len = sizeof(in_addr);
# endif
	RegisterHostaddr ((struct sockaddr *)&in_addr, sizeof (in_addr),
			  QUERY);
#endif /* IPv6 */
    }
}

static ARRAYofARRAY8	AuthenticationNames;

static int
InitXDMCP (char **argv)
{
    int	soopts = 1;
    XdmcpHeader	header;
    int	i;

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) BROADCAST_QUERY;
    header.length = 1;
    for (i = 0; i < (int)AuthenticationNames.length; i++)
	header.length += 2 + AuthenticationNames.data[i].length;
    XdmcpWriteHeader (&broadcastBuffer, &header);
    XdmcpWriteARRAYofARRAY8 (&broadcastBuffer, &AuthenticationNames);

    header.version = XDM_PROTOCOL_VERSION;
    header.opcode = (CARD16) QUERY;
    header.length = 1;
    for (i = 0; i < (int)AuthenticationNames.length; i++)
	header.length += 2 + AuthenticationNames.data[i].length;
    XdmcpWriteHeader (&directBuffer, &header);
    XdmcpWriteARRAYofARRAY8 (&directBuffer, &AuthenticationNames);
    if ((socketFD = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	return 0;
#if defined(IPv6) && defined(AF_INET6)
    socket6FD = socket (AF_INET6, SOCK_DGRAM, 0);
#endif
#ifdef SO_BROADCAST
    soopts = 1;
    if (setsockopt (socketFD, SOL_SOCKET, SO_BROADCAST, (char *)&soopts, sizeof (soopts)) < 0)
	perror ("setsockopt");
#endif

    XtAddInput (socketFD, (XtPointer) XtInputReadMask, ReceivePacket,
		(XtPointer) &socketFD);
#if defined(IPv6) && defined(AF_INET6)
    if (socket6FD != -1)
    XtAddInput (socket6FD, (XtPointer) XtInputReadMask, ReceivePacket,
		(XtPointer) &socket6FD);
#endif
    while (*argv)
    {
	RegisterHostname (*argv);
	++argv;
    }
    pingTry = 0;
    PingHosts ((XtPointer)NULL, (XtIntervalId *)NULL);
    return 1;
}

static void
Choose (HostName *h)
{
    if (app_resources.xdmAddress)
    {
	struct sockaddr_in  in_addr;
#if defined(IPv6) && defined(AF_INET6)
	struct sockaddr_in6 in6_addr;
#endif
	struct sockaddr	*addr = NULL;
	int		family;
	int		len = 0;
	int		fd;
	char		buf[1024];
	XdmcpBuffer	buffer;
	char		*xdm;

	xdm = (char *) app_resources.xdmAddress->data;
	family = (xdm[0] << 8) + xdm[1];
	switch (family) {
	case AF_INET:
#ifdef BSD44SOCKETS
	    in_addr.sin_len = sizeof(in_addr);
#endif
	    in_addr.sin_family = family;
	    memcpy(&in_addr.sin_port, xdm + 2, 2);
	    memcpy(&in_addr.sin_addr, xdm + 4, 4);
	    addr = (struct sockaddr *) &in_addr;
	    len = sizeof (in_addr);
	    break;
#if defined(IPv6) && defined(AF_INET6)
	case AF_INET6:
	    bzero(&in6_addr, sizeof(in6_addr));
# ifdef SIN6_LEN
	    in6_addr.sin6_len = sizeof(in6_addr);
# endif
	    in6_addr.sin6_family = family;
	    memcpy(&in6_addr.sin6_port, xdm + 2, 2);
	    memcpy(&in6_addr.sin6_addr, xdm + 4, 16);
	    addr = (struct sockaddr *) &in6_addr;
	    len = sizeof (in6_addr);
	    break;
#endif
	}
	if ((fd = socket (family, SOCK_STREAM, 0)) == -1)
	{
	    fprintf (stderr, "Cannot create response socket\n");
	    exit (REMANAGE_DISPLAY);
	}
	if (connect (fd, addr, len) == -1)
	{
	    fprintf (stderr, "Cannot connect to xdm\n");
	    exit (REMANAGE_DISPLAY);
	}
	buffer.data = (BYTE *) buf;
	buffer.size = sizeof (buf);
	buffer.pointer = 0;
	buffer.count = 0;
	XdmcpWriteARRAY8 (&buffer, app_resources.clientAddress);
	XdmcpWriteCARD16 (&buffer, (CARD16) app_resources.connectionType);
	XdmcpWriteARRAY8 (&buffer, &h->hostaddr);
	write (fd, (char *)buffer.data, buffer.pointer);
	close (fd);
    }
    else
    {
	int i;

	printf ("%u\n", h->connectionType);
	for (i = 0; i < (int)h->hostaddr.length; i++)
	    printf ("%u%s", h->hostaddr.data[i],
		    i == h->hostaddr.length - 1 ? "\n" : " ");
    }
}

/*
  next_line returns the next line in a list
  across the list end.
  (0, 1, 2, 3, 0, 1, 2, 3 ....)
*/

static int
next_line(unsigned int current, unsigned int size, int event)
{
  switch(event) {
    case Button5:
      return (current + 1) % size;
    case Button4:
      return (current + size - 1) % size;
    case XK_Down:
      return (current + 1) % size;
    case XK_Up:
      return (current + size - 1) % size;
  }
  return -1;
}

/*
  Hostselect selects a given chooser line.
  Returns 1 when host is willing and 0 if not
*/

static int
Hostselect (int line)
{
  XawListReturnStruct	*r;
  HostName		*h;

  /* Assume the next host is willing */
  XawListHighlight (list,line);
  r = XawListShowCurrent (list);
  /* copied from DoCheckWilling */
  for (h = hostNamedb; h; h = h->next)
  {
    if (!strcmp (r->string, h->fullname))
    {
      if (!h->willing)
	XawListUnhighlight (list);
      else
      {
	/* Scroll viewport to make sure new selection is visible */
	Arg		size[2];
	Dimension	height, portheight;
	Position	y;
	int		lineheight, liney;

	XtSetArg (size[0], XtNheight, &height);
	XtSetArg (size[1], XtNy, &y);
	XtGetValues (list, size, (Cardinal) 2);

	XtSetArg (size[0], XtNheight, &portheight);
	XtGetValues (viewport, size, (Cardinal) 1);

	lineheight = height / NameTableSize;
	liney = lineheight * line;

	if ((y + liney) < 0) {
	    XawViewportSetCoordinates(viewport, 0, liney);
	} else if ((y + liney + lineheight) > portheight) {
	    XawViewportSetCoordinates(viewport, 0,
				      (liney + lineheight) - portheight);
	}

	XtFree((char *) r);
	return 1;
      }
    }
  }
  XtFree((char *) r);
  return 0;
}

/*
  Selectfirst selects the first willing host
  in the chooser list (so you can select a host without
  presence of mouse, but with pressing space or return,
  or XK_Down / XK_Up
*/

static void
Selectfirst (void)
{
  int line;

  for (line = 0; line < NameTableSize; line++)
  {
    if (Hostselect(line))
      return;
  }
  return;
}

/*
  Storeold stores the selected line into global int oldentry
*/

static void
Storeold (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  XawListReturnStruct	*r = XawListShowCurrent(list);

  oldline = r->list_index;
  XtFree((char *) r);
}

/*
  Setold restores the global int oldentry
  when you try to select a host with your mouse
  who is unwilling as follows:
  <Btn1Down>:     Store() Set() CheckWilling() Setold() \n\
*/

static void
Setold (Widget w, XEvent *event, String *params, Cardinal *num_params)
{

  if ( (XawListShowCurrent(list))->list_index == XAW_LIST_NONE && oldline != XAW_LIST_NONE)
    XawListHighlight (list, oldline);
}

/*
  HostCycle tries to select the next willing host across
  the list end and will stop at the first found willing host
  or after trying all entries.
*/

static void
HostCycle(unsigned int line, unsigned int size, KeySym keysym)
{
  unsigned int newline = line;
  /* Do it only once around the chooser list, either direction */
  while ( (newline = next_line(newline,size,keysym)) != line && newline != -1)
  {
    if (Hostselect(newline))
      return;
  }
  /* No other willing host could be found, stay at the old one*/
  XawListHighlight (list, line);
  return;
}

/*
  Switch_Key handles XK_Up and XK_Down
  and highlights an appropriate line in the chooser list.
*/

static void
Switch_Key (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  char strbuf[128];
  KeySym keysym = 0;
  static XComposeStatus compose_status = {NULL, 0};
  XawListReturnStruct	*r;

  XLookupString (&event->xkey, strbuf, sizeof (strbuf),
                           &keysym, &compose_status);

  if (keysym != XK_Up && keysym != XK_Down)
    return;

  r = XawListShowCurrent (list);

  if (r->list_index == XAW_LIST_NONE)
    Selectfirst();
  else
    HostCycle(r->list_index,NameTableSize,keysym);

  XtFree((char *) r);
  return;
}



/*
  Switch_Btn handles ScrollWheel Forward (Button5) and Backward
  (Button4) and highlights an appropriate line in the chooser list.
*/

static void
Switch_Btn (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XawListReturnStruct	*r;
    r = XawListShowCurrent (list);

    if (r->list_index == XAW_LIST_NONE)
      Selectfirst();
    else
      HostCycle(r->list_index,NameTableSize,event->xbutton.button);

    XtFree((char *) r);
    return;
}


/* ARGSUSED */
static void
DoAccept (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XawListReturnStruct	*r;
    HostName		*h;

    r = XawListShowCurrent (list);
    if (r->list_index == XAW_LIST_NONE)
#ifdef XKB
	XkbStdBell(XtDisplay(toplevel),XtWindow(w),0,XkbBI_MinorError);
#else
	XBell (XtDisplay (toplevel), 0);
#endif
    else
    {
	for (h = hostNamedb; h; h = h->next)
	    if (!strcmp (r->string, h->fullname))
	    {
		Choose (h);
	    }
	exit (OBEYSESS_DISPLAY);
    }
    XtFree((char *) r);
}

/* ARGSUSED */
static void
DoCheckWilling (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XawListReturnStruct	*r;
    HostName		*h;

    r = XawListShowCurrent (list);
    if (r->list_index != XAW_LIST_NONE) {
	for (h = hostNamedb; h; h = h->next)
	    if (!strcmp (r->string, h->fullname))
		if (!h->willing)
		    XawListUnhighlight (list);
    }
    XtFree((char *) r);
}

/* ARGSUSED */
_X_NORETURN
static void
DoCancel (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    exit (OBEYSESS_DISPLAY);
}

/* ARGSUSED */
static void
DoPing (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    EmptyHostnames ();
    pingTry = 0;
    PingHosts ((XtPointer)NULL, (XtIntervalId *)NULL);
}

static XtActionsRec app_actions[] = {
    { "Accept",	      DoAccept },
    { "Cancel",	      DoCancel },
    { "CheckWilling", DoCheckWilling },
    { "Ping",	      DoPing },
    { "KeySwitch",    Switch_Key },
    { "BtnSwitch",    Switch_Btn },
    { "Store",	      Storeold },
    { "Setold",	      Setold },
};

int
main (int argc, char **argv)
{
    Arg		position[3];
    Dimension   width, height;
    Position	x, y;
#ifdef USE_XINERAMA
    XineramaScreenInfo *screens;
    int                 s_num;
#endif


    toplevel = XtInitialize (argv[0], "Chooser", options, XtNumber(options), &argc, argv);

    XtAddConverter(XtRString, XtRARRAY8, CvtStringToARRAY8, NULL, 0);

    XtGetApplicationResources (toplevel, (XtPointer) &app_resources, resources,
			       XtNumber (resources), NULL, (Cardinal) 0);

    XtAddActions (app_actions, XtNumber (app_actions));
    paned = XtCreateManagedWidget ("paned", panedWidgetClass, toplevel, NULL, 0);
    label = XtCreateManagedWidget ("label", labelWidgetClass, paned, NULL, 0);
    viewport = XtCreateManagedWidget ("viewport", viewportWidgetClass, paned, NULL, 0);
    list = XtCreateManagedWidget ("list", listWidgetClass, viewport, NULL, 0);
    box = XtCreateManagedWidget ("box", boxWidgetClass, paned, NULL, 0);
    cancel = XtCreateManagedWidget ("cancel", commandWidgetClass, box, NULL, 0);
    acceptit = XtCreateManagedWidget ("accept", commandWidgetClass, box, NULL, 0);
    ping = XtCreateManagedWidget ("ping", commandWidgetClass, box, NULL, 0);

    /*
     * center ourselves on the screen
     */
    XtSetMappedWhenManaged(toplevel, FALSE);
    XtRealizeWidget (toplevel);

    XtSetArg (position[0], XtNwidth, &width);
    XtSetArg (position[1], XtNheight, &height);
    XtGetValues (toplevel, position, (Cardinal) 2);
#ifdef USE_XINERAMA
    if (
	XineramaIsActive(XtDisplay(toplevel)) &&
	(screens = XineramaQueryScreens(XtDisplay(toplevel), &s_num)) != NULL
       )
    {
	x = (Position)(screens[0].x_org + (screens[0].width - width) / 2);
	y = (Position)(screens[0].y_org + (screens[0].height - height) / 3);

	XFree(screens);
    }
    else
#endif
    {
	x = (Position)(WidthOfScreen (XtScreen (toplevel)) - width) / 2;
	y = (Position)(HeightOfScreen (XtScreen (toplevel)) - height) / 3;
    }
    XtSetArg (position[0], XtNx, x);
    XtSetArg (position[1], XtNy, y);
    XtSetValues (toplevel, position, (Cardinal) 2);

    /*
     * Run
     */
    XtMapWidget(toplevel);
    InitXDMCP (argv + 1);
    XtMainLoop ();
    exit(0);
    /*NOTREACHED*/
}

/* Converts the hex string s of length len into the byte array d.
   Returns 0 if s was a legal hex string, 1 otherwise.
   */
static int
FromHex (char *s, char *d, int len)
{
    int	t;
    int ret = len&1;		/* odd-length hex strings are illegal */
    while (len >= 2)
    {
#define HexChar(c)  ('0' <= (c) && (c) <= '9' ? (c) - '0' : (c) - 'a' + 10)

	if (!ishexdigit(*s))
	    ret = 1;
	t = HexChar (*s) << 4;
	s++;
	if (!ishexdigit(*s))
	    ret = 1;
	t += HexChar (*s);
	s++;
	*d++ = t;
	len -= 2;
    }
    return ret;
}

/*ARGSUSED*/
static void
CvtStringToARRAY8 (XrmValuePtr args, Cardinal *num_args, XrmValuePtr fromVal, XrmValuePtr toVal)
{
    static ARRAY8Ptr	dest;
    char	*s;
    int		len;

    dest = (ARRAY8Ptr) XtMalloc (sizeof (ARRAY8));
    len = fromVal->size;
    s = (char *) fromVal->addr;
    if (!XdmcpAllocARRAY8 (dest, len >> 1))
	XtStringConversionWarning ((char *) fromVal->addr, XtRARRAY8);
    else
    {
	FromHex (s, (char *) dest->data, len);
    }
    toVal->addr = (caddr_t) &dest;
    toVal->size = sizeof (ARRAY8Ptr);
}
