/* $TOG: Xvarargs.h /main/5 1998/02/06 18:03:41 kaleb $ */
/*

Copyright 1985, 1986, 1987, 1988, 1989, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/X11/Xvarargs.h,v 1.2 2000/11/28 18:49:30 dawes Exp $ */

#ifndef _XVARARGS_H_
#define _XVARARGS_H_

#if NeedVarargsPrototypes
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

#endif /* _XVARARGS_H_ */
