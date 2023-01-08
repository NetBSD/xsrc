/*
 *
Copyright 1989, 1998  The Open Group

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
 * protodpy.c
 *
 * manage a collection of proto-displays.  These are displays for
 * which sessionID's have been generated, but no session has been
 * started.
 */

#include "dm.h"
#include "dm_error.h"
#include "dm_socket.h"

#ifdef XDMCP

# include <sys/types.h>
# include <time.h>
# define Time_t time_t

static struct protoDisplay	*protoDisplays;

# ifdef DEBUG
static void
PrintProtoDisplay (struct protoDisplay *pdpy)
{
    Debug ("ProtoDisplay %p\n", pdpy);
    Debug ("\taddress: %s", pdpy->address);
    //PrintSockAddr (pdpy->address, pdpy->addrlen);
    Debug ("\tdate %lu (%lu from now)\n", pdpy->date, time(0) - pdpy->date);
    Debug ("\tdisplay Number %d\n", pdpy->displayNumber);
    Debug ("\tsessionID %lu\n", (unsigned long)pdpy->sessionID);
}
# endif

struct protoDisplay *
FindProtoDisplay (
    XdmcpNetaddr    address,
    int		    addrlen,
    CARD16	    displayNumber)
{
    struct protoDisplay	*pdpy;

    Debug ("FindProtoDisplay\n");
    for (pdpy = protoDisplays; pdpy; pdpy=pdpy->next)
    {
	if (pdpy->displayNumber == displayNumber &&
	    addressEqual (address, addrlen, pdpy->address, pdpy->addrlen))
	{
	    return pdpy;
	}
    }
    return (struct protoDisplay *) 0;
}

static void
TimeoutProtoDisplays (Time_t now)
{
    struct protoDisplay	*pdpy, *next;

    for (pdpy = protoDisplays; pdpy; pdpy = next)
    {
	next = pdpy->next;
	if (pdpy->date < now - PROTO_TIMEOUT)
	    DisposeProtoDisplay (pdpy);
    }
}

struct protoDisplay *
NewProtoDisplay (
    XdmcpNetaddr    address,
    int		    addrlen,
    CARD16	    displayNumber,
    CARD16	    connectionType,
    ARRAY8Ptr	    connectionAddress,
    CARD32	    sessionID)
{
    struct protoDisplay	*pdpy;
    Time_t date;

    Debug ("NewProtoDisplay\n");
    time (&date);
    TimeoutProtoDisplays (date);
    pdpy = malloc (sizeof *pdpy);
    if (!pdpy)
	return NULL;
    pdpy->address = malloc (addrlen);
    if (!pdpy->address)
    {
	free (pdpy);
	return NULL;
    }
    pdpy->addrlen = addrlen;
    memcpy(pdpy->address, address, addrlen);
    pdpy->displayNumber = displayNumber;
    pdpy->connectionType = connectionType;
    pdpy->date = date;
    if (!XdmcpCopyARRAY8 (connectionAddress, &pdpy->connectionAddress))
    {
	free (pdpy->address);
	free (pdpy);
	return NULL;
    }
    pdpy->sessionID = sessionID;
    pdpy->fileAuthorization = (Xauth *) NULL;
    pdpy->xdmcpAuthorization = (Xauth *) NULL;
    pdpy->next = protoDisplays;
    protoDisplays = pdpy;
    return pdpy;
}

void
DisposeProtoDisplay (struct protoDisplay *pdpy)
{
    struct protoDisplay	*p, *prev;

    prev = NULL;
    for (p = protoDisplays; p; p=p->next)
    {
	if (p == pdpy)
	    break;
	prev = p;
    }
    if (!p)
	return;
    if (prev)
	prev->next = pdpy->next;
    else
	protoDisplays = pdpy->next;
    bzero(&pdpy->key, sizeof(pdpy->key));
    if (pdpy->fileAuthorization)
	XauDisposeAuth (pdpy->fileAuthorization);
    if (pdpy->xdmcpAuthorization)
	XauDisposeAuth (pdpy->xdmcpAuthorization);
    XdmcpDisposeARRAY8 (&pdpy->connectionAddress);
    free (pdpy->address);
    free (pdpy);
}

#endif /* XDMCP */
