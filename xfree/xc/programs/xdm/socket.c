/* $TOG: socket.c /main/37 1998/02/09 13:56:31 kaleb $ */
/*

Copyright 1988, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/socket.c,v 3.6 2000/05/11 18:14:43 tsi Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * socket.c - Support for BSD sockets
 */

#include "dm.h"
#include "dm_error.h"

#ifdef XDMCP
#ifndef STREAMSCONN

#include <errno.h>
#include "dm_socket.h"

#ifndef MINIX
#ifndef X_NO_SYS_UN
#ifndef Lynx
#include <sys/un.h>
#else
#include <un.h>
#endif
#endif
#include <netdb.h>
#else /* MINIX */
#include <net/hton.h>
#include <net/netlib.h>
#include <net/gen/in.h>
#include <net/gen/tcp.h>
#include <net/gen/tcp_io.h>
#include <net/gen/udp.h>
#include <net/gen/udp_io.h>
#include <sys/ioctl.h>
#include <sys/nbio.h>
#endif /* !MINIX */

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif


extern int	xdmcpFd;
extern int	chooserFd;

extern FD_TYPE	WellKnownSocketsMask;
extern int	WellKnownSocketsMax;

void
CreateWellKnownSockets (void)
{
#ifndef MINIX
    struct sockaddr_in	sock_addr;
#else /* MINIX */
    char *tcp_device, *udp_device;
    nwio_udpopt_t udpopt;
    nwio_tcpconf_t tcpconf;
    int flags;
    nbio_ref_t ref;
#endif /* !MINIX */
    char *name;

    if (request_port == 0)
	    return;
    Debug ("creating socket %d\n", request_port);
#ifdef MINIX
    udp_device= getenv("UDP_DEVICE");
    if (udp_device == NULL)
    	udp_device= UDP_DEVICE;
    xdmcpFd = open(udp_device, O_RDWR);
#else
    xdmcpFd = socket (AF_INET, SOCK_DGRAM, 0);
#endif
    if (xdmcpFd == -1) {
	LogError ("XDMCP socket creation failed, errno %d\n", errno);
	return;
    }
    name = localHostname ();
    registerHostname (name, strlen (name));
    RegisterCloseOnFork (xdmcpFd);
#ifdef MINIX
    udpopt.nwuo_flags= NWUO_SHARED | NWUO_LP_SET | NWUO_EN_LOC |
	NWUO_EN_BROAD | NWUO_RP_ANY | NWUO_RA_ANY | NWUO_RWDATALL |
	NWUO_DI_IPOPT;
    udpopt.nwuo_locport= htons(request_port);
    if (ioctl(xdmcpFd, NWIOSUDPOPT, &udpopt) == -1)
    {
	LogError ("error %d binding socket address %d\n", errno, request_port);
	close (xdmcpFd);
	xdmcpFd = -1;
	return;
    }
    if ((flags= fcntl(xdmcpFd, F_GETFD)) == -1)
    {
	LogError ("F_GETFD failed: %s\n", strerror(errno));
	close (xdmcpFd);
	xdmcpFd = -1;
	return;
    }
    if (fcntl(xdmcpFd, F_SETFD, flags | FD_ASYNCHIO) == -1)
    {
	LogError ("F_SETFD failed: %s\n", strerror(errno));
	close (xdmcpFd);
	xdmcpFd = -1;
	return;
    }
    nbio_register(xdmcpFd);
    ref.ref_int= xdmcpFd;
    nbio_setcallback(xdmcpFd, ASIO_READ, udp_read_cb, ref);
#else
    /* zero out the entire structure; this avoids 4.4 incompatibilities */
    bzero ((char *) &sock_addr, sizeof (sock_addr));
#ifdef BSD44SOCKETS
    sock_addr.sin_len = sizeof(sock_addr);
#endif
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons ((short) request_port);
    sock_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (xdmcpFd, (struct sockaddr *)&sock_addr, sizeof (sock_addr)) == -1)
    {
	LogError ("error %d binding socket address %d\n", errno, request_port);
	close (xdmcpFd);
	xdmcpFd = -1;
	return;
    }
#endif
    WellKnownSocketsMax = xdmcpFd;
    FD_SET (xdmcpFd, &WellKnownSocketsMask);

#ifdef MINIX
    tcp_device= getenv("TCP_DEVICE");
    if (tcp_device == NULL)
    	tcp_device= TCP_DEVICE;
    chooserFd = open(tcp_device, O_RDWR);
#else
    chooserFd = socket (AF_INET, SOCK_STREAM, 0);
#endif
    Debug ("Created chooser socket %d\n", chooserFd);
    if (chooserFd == -1)
    {
	LogError ("chooser socket creation failed, errno %d\n", errno);
	return;
    }
#ifdef MINIX
    tcpconf.nwtc_flags= NWTC_EXCL | NWTC_LP_SEL | NWTC_UNSET_RA |
    	NWTC_UNSET_RP;
    if (ioctl(chooserFd, NWIOSTCPCONF, &tcpconf) == -1)
    {
    	LogError("NWIOSTCPCONF failed: %s\n", strerror(errno));
    	close(chooserFd);
    	chooserFd= -1;
    	return;
    }
    GetChooserAddr(NULL, NULL);
    if ((flags= fcntl(chooserFd, F_GETFD)) == -1)
    {
	LogError ("F_GETFD failed: %s\n", strerror(errno));
	close (chooserFd);
	chooserFd = -1;
	return;
    }
    if (fcntl(chooserFd, F_SETFD, flags | FD_ASYNCHIO) == -1)
    {
	LogError ("F_SETFD failed: %s\n", strerror(errno));
	close (chooserFd);
	chooserFd = -1;
	return;
    }
    nbio_register(chooserFd);
    ref.ref_int= chooserFd;
    nbio_setcallback(chooserFd, ASIO_IOCTL, tcp_listen_cb, ref);
#else
    listen (chooserFd, 5);
#endif
    if (chooserFd > WellKnownSocketsMax)
	WellKnownSocketsMax = chooserFd;
    FD_SET (chooserFd, &WellKnownSocketsMask);
}

int
GetChooserAddr (
    char	*addr,
    int		*lenp)
{
#ifndef MINIX
    struct sockaddr_in	in_addr;
    int			len;

    len = sizeof in_addr;
    if (getsockname (chooserFd, (struct sockaddr *)&in_addr, (void *)&len) < 0)
	return -1;
    Debug ("Chooser socket port: %d\n", ntohs(in_addr.sin_port));
    memmove( addr, (char *) &in_addr, len);
    *lenp = len;

#else /* MINIX */

    static struct sockaddr_in	in_addr;
    static int first_time= 1;
    int			len;
    nwio_tcpconf_t tcpconf;

    if (first_time)
    {
    	    first_time= 0;
	    if (ioctl(chooserFd, NWIOGTCPCONF, &tcpconf) == -1)
	    {
		LogError("NWIOGTCPCONF failed: %s\n", strerror(errno));
		return -1;
	    }
	    in_addr.sin_family= AF_INET;
	    in_addr.sin_port= tcpconf.nwtc_locport;
	    in_addr.sin_addr.s_addr= tcpconf.nwtc_locaddr;
	    if (addr == NULL)
	    	return 0;
    }
    len = sizeof in_addr;
    memmove( addr, (char *) &in_addr, len);
    *lenp = len;
#endif /* !MINIX */
    return 0;
}

#endif /* !STREAMSCONN */
#endif /* XDMCP */
