/* $TOG: Host.c /main/8 1998/02/06 17:30:43 kaleb $ */
/*

Copyright 1986, 1998  The Open Group

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

*/
/* $XFree86: xc/lib/X11/Host.c,v 1.2 1999/05/09 10:49:33 dawes Exp $ */

/* this might be rightly reguarded an os dependent file */

#include "Xlibint.h"

int
XAddHost (dpy, host)
    register Display *dpy;
    XHostAddress *host;
    {
    register xChangeHostsReq *req;
    register int length = (host->length + 3) & ~0x3;	/* round up */

    LockDisplay(dpy);
    GetReqExtra (ChangeHosts, length, req);
    req->mode = HostInsert;
    req->hostFamily = host->family;
    req->hostLength = host->length;
    memcpy((char *) NEXTPTR(req,xChangeHostsReq), host->address, host->length);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
    }

int
XRemoveHost (dpy, host)
    register Display *dpy;
    XHostAddress *host;
    {
    register xChangeHostsReq *req;
    register int length = (host->length + 3) & ~0x3;	/* round up */

    LockDisplay(dpy);
    GetReqExtra (ChangeHosts, length, req);
    req->mode = HostDelete;
    req->hostFamily = host->family;
    req->hostLength = host->length;
    memcpy((char *) NEXTPTR(req,xChangeHostsReq), host->address, host->length);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
    }

int
XAddHosts (dpy, hosts, n)
    register Display *dpy;
    XHostAddress *hosts;
    int n;
{
    register int i;
    for (i = 0; i < n; i++) {
	(void) XAddHost(dpy, &hosts[i]);
      }
    return 1;
}

int
XRemoveHosts (dpy, hosts, n)
    register Display *dpy;
    XHostAddress *hosts;
    int n;
{
    register int i;
    for (i = 0; i < n; i++) {
	(void) XRemoveHost(dpy, &hosts[i]);
      }
    return 1;
}
