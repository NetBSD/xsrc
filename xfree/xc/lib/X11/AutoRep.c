/* $Xorg: AutoRep.c,v 1.3 2000/08/17 19:44:30 cpqbld Exp $ */
/*

Copyright 1985, 1998  The Open Group

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
/* $XFree86: xc/lib/X11/AutoRep.c,v 1.3 2001/01/17 19:41:32 dawes Exp $ */

#include "Xlibint.h"

int
XAutoRepeatOn (dpy)
register Display *dpy;
{
	XKeyboardControl values;
	values.auto_repeat_mode = AutoRepeatModeOn;
	XChangeKeyboardControl (dpy, KBAutoRepeatMode, &values);
	return 1;
}

int
XAutoRepeatOff (dpy)
register Display *dpy;
{
	XKeyboardControl values;
	values.auto_repeat_mode = AutoRepeatModeOff;
	XChangeKeyboardControl (dpy, KBAutoRepeatMode, &values);
	return 1;
}


