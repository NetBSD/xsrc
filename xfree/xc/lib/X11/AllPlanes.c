/* $TOG: AllPlanes.c /main/7 1998/02/06 17:03:08 kaleb $ */
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
/* $XFree86: xc/lib/X11/AllPlanes.c,v 1.2 2000/09/26 15:56:51 tsi Exp $ */

#define NEED_REPLIES
#include "Xlibint.h"

Status XAllocColorPlanes(dpy, cmap, contig, pixels, ncolors, nreds, ngreens, 
                         nblues, rmask, gmask, bmask)
register Display *dpy;
Colormap cmap;
Bool contig;
unsigned long *pixels; /* LISTofCARD32 */ /* RETURN */
int ncolors;
int nreds, ngreens, nblues;
unsigned long *rmask, *gmask, *bmask; /* CARD32 */ /* RETURN */
{
    xAllocColorPlanesReply rep;
    Status status;
    register xAllocColorPlanesReq *req;

    LockDisplay(dpy);
    GetReq(AllocColorPlanes,req);

    req->cmap = cmap;
    req->colors = ncolors;
    req->red = nreds;
    req->green = ngreens;
    req->blue = nblues;
    req->contiguous = contig;

    status = _XReply(dpy, (xReply *)&rep, 0, xFalse);


    if (status) {
	*rmask = rep.redMask;
	*gmask = rep.greenMask;
	*bmask = rep.blueMask;

	/* sizeof(CARD32) = 4 */
	_XRead32 (dpy, (long *) pixels, (long)(ncolors * 4));
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return(status);
}    
