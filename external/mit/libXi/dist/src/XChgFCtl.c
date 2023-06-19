/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

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

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/***********************************************************************
 *
 * XChangeFeedbackControl - Change the control attributes of feedbacks on
 * an extension device.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/extutil.h>
#include "XIint.h"

int
XChangeFeedbackControl(
    register Display	*dpy,
    XDevice		*dev,
    unsigned long	 mask,
    XFeedbackControl	*f)
{
    int length;
    xChangeFeedbackControlReq *req;
    XExtDisplayInfo *info = XInput_find_display(dpy);

    LockDisplay(dpy);
    if (_XiCheckExtInit(dpy, XInput_Initial_Release, info) == -1)
	return (NoSuchExtension);

    GetReq(ChangeFeedbackControl, req);
    req->reqType = info->codes->major_opcode;
    req->ReqType = X_ChangeFeedbackControl;
    req->deviceid = dev->device_id;
    req->mask = mask;
    req->feedbackid = f->class;

    if (f->class == KbdFeedbackClass) {
	XKbdFeedbackControl *K;
	xKbdFeedbackCtl k = {0};

	K = (XKbdFeedbackControl *) f;
	k.class = KbdFeedbackClass;
	k.length = sizeof(xKbdFeedbackCtl);
	k.id = K->id;
	k.click = K->click;
	k.percent = K->percent;
	k.pitch = K->pitch;
	k.duration = K->duration;
	k.led_mask = K->led_mask;
	k.led_values = K->led_value;
	k.key = K->key;
	k.auto_repeat_mode = K->auto_repeat_mode;
	length = ((unsigned)(k.length + 3) >> 2);
	req->length += length;
	length <<= 2;
	Data(dpy, (char *)&k, length);
    } else if (f->class == PtrFeedbackClass) {
	XPtrFeedbackControl *P;
	xPtrFeedbackCtl p = {0};

	P = (XPtrFeedbackControl *) f;
	p.class = PtrFeedbackClass;
	p.length = sizeof(xPtrFeedbackCtl);
	p.id = P->id;
	p.num = P->accelNum;
	p.denom = P->accelDenom;
	p.thresh = P->threshold;
	length = ((unsigned)(p.length + 3) >> 2);
	req->length += length;
	length <<= 2;
	Data(dpy, (char *)&p, length);
    } else if (f->class == IntegerFeedbackClass) {
	XIntegerFeedbackControl *I;
	xIntegerFeedbackCtl i = {0};

	I = (XIntegerFeedbackControl *) f;
	i.class = IntegerFeedbackClass;
	i.length = sizeof(xIntegerFeedbackCtl);
	i.id = I->id;
	i.int_to_display = I->int_to_display;
	length = ((unsigned)(i.length + 3) >> 2);
	req->length += length;
	length <<= 2;
	Data(dpy, (char *)&i, length);
    } else if (f->class == StringFeedbackClass) {
	XStringFeedbackControl *S;
	xStringFeedbackCtl s = {0};

	S = (XStringFeedbackControl *) f;
	s.class = StringFeedbackClass;
	s.length = sizeof(xStringFeedbackCtl) +
	    (S->num_keysyms * sizeof(KeySym));
	s.id = S->id;
	s.num_keysyms = S->num_keysyms;
	req->length += ((unsigned)(s.length + 3) >> 2);
	length = sizeof(xStringFeedbackCtl);
	Data(dpy, (char *)&s, length);
	length = (s.num_keysyms * sizeof(KeySym));
	Data(dpy, (char *)S->syms_to_display, length);
    } else if (f->class == BellFeedbackClass) {
	XBellFeedbackControl *B;
	xBellFeedbackCtl b = {0};

	B = (XBellFeedbackControl *) f;
	b.class = BellFeedbackClass;
	b.length = sizeof(xBellFeedbackCtl);
	b.id = B->id;
	b.percent = B->percent;
	b.pitch = B->pitch;
	b.duration = B->duration;
	length = ((unsigned)(b.length + 3) >> 2);
	req->length += length;
	length <<= 2;
	Data(dpy, (char *)&b, length);
    } else if (f->class == LedFeedbackClass) {
	XLedFeedbackControl *L;
	xLedFeedbackCtl l = {0};

	L = (XLedFeedbackControl *) f;
	l.class = LedFeedbackClass;
	l.length = sizeof(xLedFeedbackCtl);
	l.id = L->id;
	l.led_mask = L->led_mask;
	l.led_values = L->led_values;
	length = ((unsigned)(l.length + 3) >> 2);
	req->length += length;
	length <<= 2;
	Data(dpy, (char *)&l, length);
    } else {
	xFeedbackCtl u = {0};

	u.class = f->class;
	u.length = f->length - sizeof(int);
	u.id = f->id;
	length = ((unsigned)(u.length + 3) >> 2);
	req->length += length;
	length <<= 2;
	Data(dpy, (char *)&u, length);
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return (Success);
}
