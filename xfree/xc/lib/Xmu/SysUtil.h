/* $TOG: SysUtil.h /main/5 1998/02/06 15:46:12 kaleb $ */

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
/* $XFree86: xc/lib/Xmu/SysUtil.h,v 1.6 1998/10/03 09:06:36 dawes Exp $ */

#ifndef _SYSUTIL_H_
#define _SYSUTIL_H_

#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

int XmuGetHostname
(
    char		*buf_return,
    int			maxlen
);

int XmuSnprintf
(
 char			*str,
 int			size,
 _Xconst char		*fmt,
    ...
 );

_XFUNCPROTOEND

#endif /* _SYSUTIL_H_ */
