/*
 * $TOG: Fill.c /main/12 1998/02/06 14:39:37 kaleb $
 *
 * 
Copyright 1989, 1998  The Open Group

All Rights Reserved.

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
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/lib/Xdmcp/Fill.c,v 3.6 2000/05/11 18:14:10 tsi Exp $ */

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

#ifdef STREAMSCONN
#include <tiuser.h>
#else
#ifdef WIN32
#include <X11/Xwinsock.h>
#else
#ifndef MINIX
#ifndef Lynx
#include <sys/socket.h>
#else
#include <socket.h>
#endif /* !Lynx */
#endif /* !MINIX */
#endif
#endif

#ifndef MINIX
int
XdmcpFill (fd, buffer, from, fromlen)
    int		    fd;
    XdmcpBufferPtr  buffer;
    XdmcpNetaddr    from;	/* return */
    int		    *fromlen;	/* return */
{
    BYTE    *newBuf;
#ifdef STREAMSCONN
    struct t_unitdata dataunit;
    int gotallflag, result;
#endif

    if (buffer->size < XDM_MAX_MSGLEN)
    {
	newBuf = (BYTE *) Xalloc (XDM_MAX_MSGLEN);
	if (newBuf)
	{
	    Xfree (buffer->data);
	    buffer->data = newBuf;
	    buffer->size = XDM_MAX_MSGLEN;
	}
    }
    buffer->pointer = 0;
#ifdef STREAMSCONN
    dataunit.addr.buf = from;
    dataunit.addr.maxlen = *fromlen;
    dataunit.opt.maxlen = 0;	/* don't care to know about options */
    dataunit.udata.buf = (char *)buffer->data;
    dataunit.udata.maxlen = buffer->size;
    result = t_rcvudata (fd, &dataunit, &gotallflag);
    if (result < 0) {
	return FALSE;
    }
    buffer->count = dataunit.udata.len;
    *fromlen = dataunit.addr.len;
#else
    buffer->count = recvfrom (fd, (char*)buffer->data, buffer->size, 0,
			      (struct sockaddr *)from, (void *)fromlen);
#endif
    if (buffer->count < 6) {
	buffer->count = 0;
	return FALSE;
    }
    return TRUE;
}
#else /* MINIX */
int
MNX_XdmcpFill (fd, buffer, from, fromlen, data, datalen)
    int		    fd;
    XdmcpBufferPtr  buffer;
    XdmcpNetaddr    from;	/* return */
    int		    *fromlen;	/* return */
    char	    *data;
    int		    datalen;
{
    BYTE    *newBuf;
    struct sockaddr_in *from_addr;
    udp_io_hdr_t *udp_io_hdr;

    if (buffer->size < XDM_MAX_MSGLEN)
    {
	newBuf = (BYTE *) Xalloc (XDM_MAX_MSGLEN);
	if (newBuf)
	{
	    Xfree (buffer->data);
	    buffer->data = newBuf;
	    buffer->size = XDM_MAX_MSGLEN;
	}
    }
    buffer->pointer = 0;
    udp_io_hdr= (udp_io_hdr_t *)data;
    data += sizeof(udp_io_hdr_t) + udp_io_hdr->uih_ip_opt_len;
    datalen -= sizeof(udp_io_hdr_t) + udp_io_hdr->uih_ip_opt_len;
    buffer->count= udp_io_hdr->uih_data_len;
    if (buffer->count > datalen)
    {
    	buffer->count= 0;
    	return FALSE;
    }
    bcopy(data, (char *)buffer->data, buffer->count);
    from_addr= (struct sockaddr_in *)from;
    from_addr->sin_family= AF_INET;
    from_addr->sin_addr.s_addr= udp_io_hdr->uih_src_addr;
    from_addr->sin_port= udp_io_hdr->uih_src_port;
    if (buffer->count < 6) {
	buffer->count = 0;
	return FALSE;
    }
    return TRUE;
}
#endif /* !MINIX */
