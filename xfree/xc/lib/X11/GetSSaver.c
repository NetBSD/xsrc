/* $TOG: GetSSaver.c /main/6 1998/02/06 17:28:26 kaleb $ */
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
/* $XFree86: xc/lib/X11/GetSSaver.c,v 1.2 1999/05/09 10:49:29 dawes Exp $ */

#define NEED_REPLIES
#include "Xlibint.h"

int
XGetScreenSaver(dpy, timeout, interval, prefer_blanking, allow_exp)
     register Display *dpy;
     /* the following are return only vars */
     int *timeout, *interval;
     int *prefer_blanking, *allow_exp;  /*boolean */
     
{       
    xGetScreenSaverReply rep;
    register xReq *req;
    LockDisplay(dpy);
    GetEmptyReq(GetScreenSaver, req);

    (void) _XReply (dpy, (xReply *)&rep, 0, xTrue);
    *timeout = rep.timeout;
    *interval = rep.interval;
    *prefer_blanking = rep.preferBlanking;
    *allow_exp = rep.allowExposures;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

