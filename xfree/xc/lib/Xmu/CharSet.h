/* $Xorg: CharSet.h,v 1.3 2000/08/17 19:45:59 cpqbld Exp $ */

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

/* $XFree86: xc/lib/Xmu/CharSet.h,v 1.7 2001/01/17 19:42:53 dawes Exp $ */

/*
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_CHARSET_H_
#define _XMU_CHARSET_H_

#include <X11/Xfuncproto.h>

_XFUNCPROTOBEGIN

void XmuCopyISOLatin1Lowered
(
 char		*dst_return,
 _Xconst char	*src
 );

void XmuCopyISOLatin1Uppered
(
 char		*dst_return,
 _Xconst char	*src
 );

int XmuCompareISOLatin1
(
 _Xconst char	*first,
 _Xconst char	*second
 );

void XmuNCopyISOLatin1Lowered
(
 char		*dst_return,
 _Xconst char	*src,
 int		 size
 );

void XmuNCopyISOLatin1Uppered
(
 char		*dst_return,
 _Xconst char	*src,
 int		size
 );

_XFUNCPROTOEND

#endif /* _XMU_CHARSET_H_ */
