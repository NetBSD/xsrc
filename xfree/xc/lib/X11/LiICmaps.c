/* $TOG: LiICmaps.c /main/7 1998/02/06 17:42:04 kaleb $ */
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
/* $XFree86: xc/lib/X11/LiICmaps.c,v 1.2 2000/09/26 15:56:51 tsi Exp $ */

#define NEED_REPLIES
#include "Xlibint.h"

Colormap *XListInstalledColormaps(dpy, win, n)
register Display *dpy;
Window win;
int *n;  /* RETURN */
{
    long nbytes;
    Colormap *cmaps;
    xListInstalledColormapsReply rep;
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(ListInstalledColormaps, win, req);

    if(_XReply(dpy, (xReply *) &rep, 0, xFalse) == 0) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    *n = 0;
	    return((Colormap *) NULL);
	}

    if (rep.nColormaps) {
	nbytes = rep.nColormaps * sizeof(Colormap);
	cmaps = (Colormap *) Xmalloc((unsigned) nbytes);
	nbytes = rep.nColormaps << 2;
	if (! cmaps) {
	    _XEatData(dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return((Colormap *) NULL);
	}
	_XRead32 (dpy, (long *) cmaps, nbytes);
    }
    else cmaps = (Colormap *) NULL;
    
    *n = rep.nColormaps;
    UnlockDisplay(dpy);
    SyncHandle();
    return(cmaps);
}

