/* $TOG: CvtCache.h /main/8 1998/02/06 15:42:17 kaleb $ */

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
/* $XFree86: xc/lib/Xmu/CvtCache.h,v 1.5 1999/03/21 07:34:36 dawes Exp $ */

/*
 *			       Public Interfaces
 * 
 * XmuCvtCache *XmuCvtCacheLookupDisplay (dpy)
 *     Display *dpy;
 */

#ifndef _XMU_CVTCACHE_H_
#define _XMU_CVTCACHE_H_

#include <X11/Xmu/DisplayQue.h>
#include <X11/Xfuncproto.h>

typedef struct _XmuCvtCache {
    struct {
	char **bitmapFilePath;
    } string_to_bitmap;
    /* add other per-display data that needs to be cached */
} XmuCvtCache;

_XFUNCPROTOBEGIN

XmuCvtCache *_XmuCCLookupDisplay
(
 Display	*dpy
 );

extern void _XmuStringToBitmapInitCache(XmuCvtCache *c);
extern void _XmuStringToBitmapFreeCache(XmuCvtCache *c);

_XFUNCPROTOEND

#endif /* _XMU_CVTCACHE_H_ */
