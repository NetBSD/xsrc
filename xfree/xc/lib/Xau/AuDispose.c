/* $Xorg: AuDispose.c,v 1.3 2000/08/17 19:45:28 cpqbld Exp $ */

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
/* $XFree86: xc/lib/Xau/AuDispose.c,v 1.3 2001/01/17 19:42:23 dawes Exp $ */

#include <X11/Xauth.h>

#ifdef X_NOT_STDC_ENV
extern void free();
#else
#include <stdlib.h>
#endif

void
XauDisposeAuth (auth)
Xauth	*auth;
{
    if (auth) {
	if (auth->address) (void) free (auth->address);
	if (auth->number) (void) free (auth->number);
	if (auth->name) (void) free (auth->name);
	if (auth->data) {
	    (void) bzero (auth->data, auth->data_length);
	    (void) free (auth->data);
	}
	free ((char *) auth);
    }
    return;
}
