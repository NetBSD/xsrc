/* $TOG: StName.c /main/10 1998/02/06 17:54:29 kaleb $ */
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
/* $XFree86: xc/lib/X11/StName.c,v 1.2 1999/05/09 10:50:15 dawes Exp $ */

#include <X11/Xlibint.h>
#include <X11/Xatom.h>

int
#if NeedFunctionPrototypes
XStoreName (
    register Display *dpy,
    Window w,
    _Xconst char *name)
#else
XStoreName (dpy, w, name)
    register Display *dpy;
    Window w;
    char *name;
#endif
{
    return XChangeProperty(dpy, w, XA_WM_NAME, XA_STRING, 
			   8, PropModeReplace, (unsigned char *)name,
			   name ? strlen(name) : 0);
}

int
#if NeedFunctionPrototypes
XSetIconName (
    register Display *dpy,
    Window w,
    _Xconst char *icon_name)
#else
XSetIconName (dpy, w, icon_name)
    register Display *dpy;
    Window w;
    char *icon_name;
#endif
{
    return XChangeProperty(dpy, w, XA_WM_ICON_NAME, XA_STRING, 
			   8, PropModeReplace, (unsigned char *)icon_name,
			   icon_name ? strlen(icon_name) : 0);
}
