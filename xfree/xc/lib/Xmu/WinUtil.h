/* $TOG: WinUtil.h /main/8 1998/02/06 15:46:44 kaleb $ */

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
/* $XFree86: xc/lib/Xmu/WinUtil.h,v 1.5 1998/10/03 09:06:38 dawes Exp $ */

/*
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_WINDOWUTIL_H_
#define _XMU_WINDOWUTIL_H_

#include <X11/Xutil.h>
#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

Window XmuClientWindow
(
 Display	*dpy,
 Window 	win
 );

Bool XmuUpdateMapHints
(
 Display	*dpy,
 Window		win,
 XSizeHints	*hints
 );

Screen *XmuScreenOfWindow
(
 Display	*dpy,
 Window 	w
);

_XFUNCPROTOEND

#endif /* _XMU_WINDOWUTIL_H_ */
