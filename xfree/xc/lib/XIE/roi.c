/* $TOG: roi.c /main/3 1998/02/06 15:12:57 kaleb $ */

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
/* $XFree86: xc/lib/XIE/roi.c,v 1.3 1999/06/13 16:18:12 dawes Exp $ */

#include "XIElibint.h"


XieRoi
XieCreateROI (Display *display)
{
    xieCreateROIReq	*req;
    char		*pBuf;
    XieRoi		id;

    LockDisplay (display);

    id = XAllocID (display);

    GET_REQUEST (CreateROI, pBuf);

    BEGIN_REQUEST_HEADER (CreateROI, pBuf, req);

    STORE_REQUEST_HEADER (CreateROI, req);
    req->roi = id;

    END_REQUEST_HEADER (CreateROI, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);

    return (id);
}


void
XieDestroyROI (Display *display, XieRoi roi)
{
    xieDestroyROIReq	*req;
    char		*pBuf;

    LockDisplay (display);

    GET_REQUEST (DestroyROI, pBuf);

    BEGIN_REQUEST_HEADER (DestroyROI, pBuf, req);

    STORE_REQUEST_HEADER (DestroyROI, req);
    req->roi = roi;

    END_REQUEST_HEADER (DestroyROI, pBuf, req);

    UnlockDisplay (display);
    SYNC_HANDLE (display);
}
