/* $Xorg: ccimake.c,v 1.3 2000/08/17 19:41:50 cpqbld Exp $ */
/*

Copyright (c) 1993, 1994, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group .

*/
/* $XFree86: xc/config/imake/ccimake.c,v 1.2 2001/04/01 13:59:56 tsi Exp $ */

/* 
 * Warning:  This file must be kept as simple as posible so that it can 
 * compile without any special flags on all systems.  Do not touch it unless
 * you *really* know what you're doing.  Make changes in imakemdep.h, not here.
 */

#define CCIMAKE			/* only get imake_ccflags definitions */
#include "imakemdep.h"		/* things to set when porting imake */

#ifndef imake_ccflags
#define imake_ccflags "-O"
#endif

int
main()
{
	write(1, imake_ccflags, sizeof(imake_ccflags) - 1);
	return 0;
}

