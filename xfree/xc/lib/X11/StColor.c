/* $Xorg: StColor.c,v 1.3 2000/08/17 19:44:55 cpqbld Exp $ */
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
/* $XFree86: xc/lib/X11/StColor.c,v 1.3 2001/01/17 19:41:44 dawes Exp $ */

#include "Xlibint.h"

int
XStoreColor(dpy, cmap, def)
register Display *dpy;
Colormap cmap;
XColor *def;
{
    xColorItem *citem;
    register xStoreColorsReq *req;
#ifdef MUSTCOPY
    xColorItem citemdata;
    long len = SIZEOF(xColorItem);

    citem = &citemdata;
#endif /* MUSTCOPY */

    LockDisplay(dpy);
    GetReqExtra(StoreColors, SIZEOF(xColorItem), req); /* assume size is 4*n */

    req->cmap = cmap;

#ifndef MUSTCOPY
    citem = (xColorItem *) NEXTPTR(req,xStoreColorsReq);
#endif /* not MUSTCOPY */

    citem->pixel = def->pixel;
    citem->red = def->red;
    citem->green = def->green;
    citem->blue = def->blue;
    citem->flags = def->flags; /* do_red, do_green, do_blue */

#ifdef MUSTCOPY
    dpy->bufptr -= SIZEOF(xColorItem);		/* adjust for GetReqExtra */
    Data (dpy, (char *) citem, len);
#endif /* MUSTCOPY */

    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}
