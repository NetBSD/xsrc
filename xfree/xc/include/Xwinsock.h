/* $TOG: Xwinsock.h /main/2 1998/02/09 11:19:29 kaleb $ */
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

/*
 * This header file has for sole purpose to allow to include winsock.h
 * without getting any name conflicts with our code.
 * Conflicts come from the fact that including winsock.h actually pulls
 * in the whole Windows API...
 */

#define BOOL wBOOL
#undef Status
#define Status wStatus
#define ATOM wATOM
#define FreeResource wFreeResource
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#undef ATOM
#undef FreeResource
#undef CreateWindowA
#undef RT_FONT
#undef RT_CURSOR
