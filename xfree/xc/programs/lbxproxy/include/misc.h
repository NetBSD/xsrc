/* $Xorg: misc.h,v 1.3 2000/08/17 19:53:57 cpqbld Exp $ */

/*

Copyright 1995, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
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

#ifndef MISC_H
#define MISC_H 1

#define NEED_EVENTS
#define NEED_REPLIES
#define _XLBX_SERVER_
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xmd.h>
#include "Xos.h"
#define ALLOCATE_LOCAL_FALLBACK(_size) Xalloc(_size)
#define DEALLOCATE_LOCAL_FALLBACK(_ptr) Xfree(_ptr)
#include "Xalloca.h"
#include "Xfuncs.h"
#include "Xfuncproto.h"

typedef void *pointer;
#ifndef _BOOL_ALREADY_DEFINED_
typedef int Bool;
#endif
#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif
typedef struct _Client *ClientPtr;
typedef struct _XServer *XServerPtr;

#ifndef NULL
#define NULL 0
#endif
#define DE_RESET     1
#define DE_TERMINATE 2
#define MILLI_PER_SECOND (1000)

/* XXX globals.h? */
extern int nextFreeClientID; 
extern int nClients;
extern char *display_name;
extern char dispatchException;
extern char isItTimeToYield;
extern int MaxClients;

/* The following byte swapping tools are duplicated in several places.
 * Do they deserve their own header file?  What else would logically go in
 * such a header?
 */

/* byte swap a 32-bit literal */
#define lswapl(x) ((((x) & 0xff) << 24) |\
		   (((x) & 0xff00) << 8) |\
		   (((x) & 0xff0000) >> 8) |\
		   (((x) >> 24) & 0xff))

/* byte swap a short literal */
#define lswaps(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))

/* byte swap a 32-bit value */
#define swapl(x, n) { \
		 n = ((char *) (x))[0];\
		 ((char *) (x))[0] = ((char *) (x))[3];\
		 ((char *) (x))[3] = n;\
		 n = ((char *) (x))[1];\
		 ((char *) (x))[1] = ((char *) (x))[2];\
		 ((char *) (x))[2] = n; }

/* byte swap a short */
#define swaps(x, n) { \
		 n = ((char *) (x))[0];\
		 ((char *) (x))[0] = ((char *) (x))[1];\
		 ((char *) (x))[1] = n; }

/* copy 32-bit value from src to dst byteswapping on the way */
#define cpswapl(src, dst) { \
                 ((char *)&(dst))[0] = ((char *) &(src))[3];\
                 ((char *)&(dst))[1] = ((char *) &(src))[2];\
                 ((char *)&(dst))[2] = ((char *) &(src))[1];\
                 ((char *)&(dst))[3] = ((char *) &(src))[0]; }

/* copy short from src to dst byteswapping on the way */
#define cpswaps(src, dst) { \
		 ((char *) &(dst))[0] = ((char *) &(src))[1];\
		 ((char *) &(dst))[1] = ((char *) &(src))[0]; }

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define lowbit(x) ((x) & (~(x) + 1))

#define REQUEST(type) \
	register type *stuff = (type *)client->requestBuffer

#endif
