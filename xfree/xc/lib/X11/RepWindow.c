/* $TOG: RepWindow.c /main/6 1998/02/06 17:50:42 kaleb $ */
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
/* $XFree86: xc/lib/X11/RepWindow.c,v 1.2 1999/05/09 10:50:02 dawes Exp $ */

#include "Xlibint.h"

int
XReparentWindow(dpy, w, p, x, y)
    register Display *dpy;
    Window w, p;
    int x, y;
{
    register xReparentWindowReq *req;

    LockDisplay(dpy);
    GetReq(ReparentWindow, req);
    req->window = w;
    req->parent = p;
    req->x = x;
    req->y = y;
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

