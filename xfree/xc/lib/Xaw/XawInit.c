/*
 * $TOG: XawInit.c /main/4 1998/02/06 12:53:40 kaleb $
 *
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
 * 
 * 
 * 			    XawInitializeWidgetSet
 * 
 * This routine forces a reference to vendor shell so that the one in this
 * widget is installed.  Any other cross-widget set initialization should be
 * done here as well.  All Athena widgets should include "XawInit.h" and
 * call this routine from their ClassInitialize procs (this routine may be
 * used as the class init proc).
 */
/* $XFree86: xc/lib/Xaw/XawInit.c,v 1.8 1999/06/06 08:48:22 dawes Exp $ */

#include <X11/Intrinsic.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>
#include "Private.h"

void
XawInitializeWidgetSet(void)
{
    static Boolean firsttime = True;

    if (firsttime) {
	firsttime = False;
#ifndef OLDXAW
	XawPixmapsInitialize();
	XawInitializeDefaultConverters();
#endif
	XtInitializeWidgetClass(vendorShellWidgetClass);
    }
}
