/* $TOG: await.c /main/3 1998/02/06 15:11:25 kaleb $ */

/*

Copyright 1993, 1998  The Open Group

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
/* $XFree86: xc/lib/XIE/await.c,v 1.3 1999/06/13 16:18:07 dawes Exp $ */

#include "XIElibint.h"


void
XieAwait (
	Display		*display,
	unsigned long	name_space,
	unsigned long	flo_id)
{
    xieAwaitReq	*req;
    char	*pBuf;

    LockDisplay (display);

    GET_REQUEST (Await, pBuf);

    BEGIN_REQUEST_HEADER (Await, pBuf, req);

    STORE_REQUEST_HEADER (Await, req);
    req->nameSpace = name_space;
    req->floID = flo_id;

    END_REQUEST_HEADER (Await, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}
