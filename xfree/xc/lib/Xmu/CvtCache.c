/* $TOG: CvtCache.c /main/9 1998/02/06 15:42:13 kaleb $ */

/*
 
Copyright 1989, 1998  The Open Group

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
/* $XFree86: xc/lib/Xmu/CvtCache.c,v 3.3 1998/10/03 09:06:24 dawes Exp $ */

/* 
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xmu/CvtCache.h>

#ifdef X_NOT_STDC_ENV
extern char *malloc();
#else
#include <stdlib.h>
#endif

/*
 * Prototypes
 */
static int _CloseDisplay(XmuDisplayQueue*, XmuDisplayQueueEntry*);
static int _FreeCCDQ(XmuDisplayQueue*);
static void _InitializeCvtCache(XmuCvtCache*);

/*
 * From LocBitmap.c
 */
void _XmuStringToBitmapFreeCache(register XmuCvtCache*);
void _XmuStringToBitmapInitCache(register XmuCvtCache*);

/*
 * Initialization
 */
static XmuDisplayQueue *dq = NULL;


/*
 * internal utility callbacks
 */

static int
_FreeCCDQ(XmuDisplayQueue *q)
{
    XmuDQDestroy (dq, False);
    dq = NULL;
    return (0);
}


static int
_CloseDisplay(XmuDisplayQueue *q, XmuDisplayQueueEntry *e)
{
    XmuCvtCache *c;

    if (e && (c = (XmuCvtCache *)(e->data))) {
	_XmuStringToBitmapFreeCache (c);
	/* insert calls to free any cached memory */

    }
    return 0;
}

static void
_InitializeCvtCache(register XmuCvtCache *c)
{
    _XmuStringToBitmapInitCache (c);
    /* insert calls to init any cached memory */
}


/*
 * XmuCCLookupDisplay - return the cache entry for the indicated display;
 * initialize the cache if necessary
 */
XmuCvtCache *
_XmuCCLookupDisplay(Display *dpy)
{
    XmuDisplayQueueEntry *e;

    /*
     * If no displays have been added before this, create the display queue.
     */
    if (!dq) {
	dq = XmuDQCreate (_CloseDisplay, _FreeCCDQ, NULL);
	if (!dq) return NULL;
    }
    
    /*
     * See if the display is already there
     */
    e = XmuDQLookupDisplay (dq, dpy);	/* see if it's there */
    if (!e) {				/* else create it */
	XmuCvtCache *c = (XmuCvtCache *) malloc (sizeof (XmuCvtCache));
	if (!c) return NULL;

	/*
	 * Add the display to the queue
	 */
	e = XmuDQAddDisplay (dq, dpy, (XPointer) c);
	if (!e) {
	    free ((char *) c);
	    return NULL;
	}

	/*
	 * initialize fields in cache
	 */
	_InitializeCvtCache (c);
    }

    /*
     * got it
     */
    return (XmuCvtCache *)(e->data);
}


