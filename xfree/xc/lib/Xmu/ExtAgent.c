/* $TOG: ExtAgent.c /main/2 1998/02/06 15:43:39 kaleb $ */

/*

Copyright 1994, 1998  The Open Group

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
/* $XFree86: xc/lib/Xmu/ExtAgent.c,v 1.5 1998/10/03 09:06:29 dawes Exp $ */

#include <X11/Intrinsic.h>
#include <X11/Xmu/ExtAgent.h>

/* ARGSUSED */
void
XmuRegisterExternalAgent(Widget w, XtPointer data,
			 XEvent *event, Boolean *cont)
{
/* 
* This is a stub.  Writers of protocol libraries
* that want to add protocol to Xaw, such as RAP, will reimplement this
* procedure in their own libraries.
*/
}
