/* $TOG: CrPixFBit.c /main/5 1998/02/06 15:41:55 kaleb $ */

/*

Copyright 1988, 1998  The Open Group

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
/* $XFree86: xc/lib/Xmu/CrPixFBit.c,v 1.5 1998/10/03 09:06:23 dawes Exp $ */

/*
 * This file contains miscellaneous utility routines and is not part of the
 * Xlib standard.
 */

/*
 * Public entry points:
 *
 *     XmuCreatePixmapFromBitmap	make a pixmap from a bitmap
 */

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xmu/Drawing.h>

Pixmap
XmuCreatePixmapFromBitmap(Display *dpy, Drawable d, Pixmap bitmap,
			  unsigned int width, unsigned int height,
			  unsigned int depth,
			  unsigned long fore, unsigned long back)
     /*
      * dpy		-	connection to X server
      * d		-	drawable indicating screen
      * bitmap		-	single plane pixmap
      * width, height	-	dimensions of bitmap and pixmap
      * depth		-	depth of pixmap to create
      * fore, back	-	colors to use
      */
{
    Pixmap pixmap;

    pixmap = XCreatePixmap (dpy, d, width, height, depth);
    if (pixmap != None) {
	GC gc;
	XGCValues xgcv;

	xgcv.foreground = fore;
	xgcv.background = back;
	xgcv.graphics_exposures = False;

	gc = XCreateGC (dpy, d,
			(GCForeground | GCBackground | GCGraphicsExposures),
			&xgcv);
	if (gc) {
	    XCopyPlane (dpy, bitmap, pixmap, gc, 0, 0, width, height, 0, 0, 1);
	    XFreeGC (dpy, gc);
	} else {
	    XFreePixmap (dpy, pixmap);
	    pixmap = None;
	}
    }
    return pixmap;
}
