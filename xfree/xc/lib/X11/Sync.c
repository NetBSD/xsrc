/* $TOG: Sync.c /main/8 1998/02/06 17:54:56 kaleb $ */
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
/* $XFree86: xc/lib/X11/Sync.c,v 1.2 1999/05/09 10:50:16 dawes Exp $ */

#define NEED_REPLIES
#define NEED_EVENTS
#include "Xlibint.h"

/* Synchronize with errors and events, optionally discarding pending events */

int
XSync (dpy, discard)
    register Display *dpy;
    Bool discard;
{
    xGetInputFocusReply rep;
    register xReq *req;

    LockDisplay(dpy);
    GetEmptyReq(GetInputFocus, req);
    (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);

    if (discard && dpy->head) {
       _XQEvent *qelt;

       for (qelt=dpy->head; qelt; qelt=qelt->next)
	   qelt->qserial_num = 0;

       ((_XQEvent *)dpy->tail)->next = dpy->qfree;
       dpy->qfree = (_XQEvent *)dpy->head;
       dpy->head = dpy->tail = NULL;
       dpy->qlen = 0;
    }
    UnlockDisplay(dpy);
    return 1;
}

