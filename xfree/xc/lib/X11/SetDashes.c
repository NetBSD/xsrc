/* $TOG: SetDashes.c /main/9 1998/02/06 17:51:44 kaleb $ */
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
/* $XFree86: xc/lib/X11/SetDashes.c,v 1.2 1999/05/09 10:50:07 dawes Exp $ */

#include "Xlibint.h"

int
#if NeedFunctionPrototypes
XSetDashes (
    register Display *dpy,
    GC gc,
    int dash_offset,
    _Xconst char *list,
    int n)
#else
XSetDashes (dpy, gc, dash_offset, list, n)
    register Display *dpy;
    GC gc;
    int dash_offset;
    char *list;
    int n;
#endif
    {
    register xSetDashesReq *req;

    LockDisplay(dpy);
    GetReq (SetDashes,req);
    req->gc = gc->gid;
    req->dashOffset = gc->values.dash_offset = dash_offset;
    req->nDashes = n;
    req->length += (n+3)>>2;
    gc->dashes = 1;
    gc->dirty &= ~(GCDashList | GCDashOffset);
    Data (dpy, list, (long)n);
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
    }
    
