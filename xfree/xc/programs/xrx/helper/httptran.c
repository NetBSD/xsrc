/* $TOG: httptran.c /main/2 1998/02/10 18:29:50 kaleb $ */
/*

Copyright 1996, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT
SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABIL-
ITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization from
The Open Group.

*/

/* $XFree86: xc/programs/xrx/helper/httptran.c,v 1.3 1998/10/04 09:41:47 dawes Exp $ */

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define TRANS(func) _HttpTrans##func
#else
#define TRANS(func) _HttpTrans/**/func
#endif
static char* __xtransname = "_HttpTrans";

#include "transport.c"
