/* $TOG: FreeGC.c /main/9 1998/02/06 17:23:35 kaleb $ */
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
/* $XFree86: xc/lib/X11/FreeGC.c,v 1.2 1999/05/09 10:49:23 dawes Exp $ */

#include "Xlibint.h"

/* FreeEData.c */
extern int _XFreeExtData();

int
XFreeGC (dpy, gc)
    register Display *dpy;
    GC gc;
    {
    register xResourceReq *req;
    register _XExtension *ext;
    LockDisplay(dpy);
    /* call out to any extensions interested */
    for (ext = dpy->ext_procs; ext; ext = ext->next)
	if (ext->free_GC) (*ext->free_GC)(dpy, gc, &ext->codes);
    GetResReq (FreeGC, gc->gid, req);
    UnlockDisplay(dpy);
    SyncHandle();
    _XFreeExtData(gc->ext_data);
    Xfree ((char *) gc);
    return 1;
    }
    
