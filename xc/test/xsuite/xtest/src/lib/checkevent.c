/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: checkevent.c,v 1.11 94/04/17 21:00:36 rws Exp $
 */
/***********************************************************
Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/**
* FACILITY: Regression Test Library
*
* ABSTRACT:
*
*	Check two arbitrary events to see if they match, report
*	an error if they don't
*
* AUTHOR:
*	Larry Jones	21-Dec-1987
*
* MODIFIED BY:
*
*	x-5	EJM0003		Erik J. Morse,		03-Feb-1989
*	Remove window check line for keymap events.
*
*	x-4	EJM0004		Erik J. Morse,		19-July-1988
*	Type case DCMP args to long to avoid lint errors.
*	Remove check for window from XMappinEvent since it is unused.
*	Only check xmapping.first_keycode and xmapping.count fields
*	if the event is a MappingKeyboard one.
*
*	x-3	EJM0003		Erik J. Morse,		19-May-1988
*	Remove if (!errflg) check
*
*	x-2	EJM0002		Erik J. Morse,		06-Jan-1988
*	Fix incompatabilites in event fields and make error reporting
*	clearer.
*
*	x-1	EJM0001		Erik J. Morse,		24-Dec-1987
*	Rewrite to use new check_item routines and to properly use
*	unions fields.
* 
* ----
*   Steve Ratcliffe, 14 Sep 1990
*   Rewritten for use within the xtest/phoenix system.
*   I have not verified that all the fields are covered by this routine.
*
* CALLING SEQUENCE:
*
*	checkevent(good, ev)
*
* FORMAL PARAMETERS:
*
*	good
*		pointer to an XEvent
*		a known good event
*
*	ev
*		pointer to an XEvent
*		the event pulled off the queue by a XNextEvent or
*		XPeekEvent call
*
* IMPLICIT INPUTS:
*
*	NONE
*
* IMPLICIT OUTPUTS:
*
*	NONE
*
* COMPLETION STATUS: (or ROUTINE VALUE:)
*
*	Returns number of fields that failed or -1 if the event was
*       not recognised as valid.
*
* SIDE EFFECTS:
*
*	NONE
*
**/

/* 
 * intentionally left out time field checks because we never know what
 * they will contain.
 */

#include "stdio.h"
#include "xtest.h"
#include "tet_api.h"
#include "Xlib.h"
#include "extensions/XIproto.h"
#include "extensions/XInput.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "pixval.h"

/* Compare values (address type) */
#define ACMP(g, t, str) if (g != t) {\
	report("Checking event type %s", eventname(good->type)); \
	report("found error in %s field, was 0x%lx, expecting 0x%lx", str, t, g);\
	fail++; \
	} else pass++;

/* Compare values (decimal type) */
#define DCMP(g, t, str) if (g != t) {\
	report("Checking event type %s", eventname(good->type)); \
	report("found error in %s field, was %ld, expecting %ld",str,t,g);\
	fail++; \
	} else pass++;

/* Compare values (unsigned type) */
#define UCMP(g, t, str) if (g != t) {\
	report("Checking event type %s", eventname(good->type)); \
	report("found error in %s field, was %lu, expecting %lu",str,t,g);\
	fail++; \
	} else pass++;

/*
 * Compare two events.
 * Returns the number of fields that compared false.
 * On error (eg. no such event type) returns -1.
 */
int
checkevent(good, ev)
XEvent *good;
XEvent *ev;
{
int 	i;
int 	fail;
int 	pass;
char	tempstr[50];

	fail = 0;
	pass = 0;

	/*
	 * First check the type.  If the type is incorrect then we return
	 * immediately since there is almost certainly no point in testing
	 * the other fields.
	 */
	if (good->type != ev->type) {
		report("Incorrect event type %s, was expecting %s",
			eventname(ev->type), eventname(good->type));
		fail++;
		return(1);
	} else
		pass++;

	ACMP((char*)good->xany.display, (char*)ev->xany.display, "display");

	switch (good->type) {
	case KeyPress:
	case KeyRelease:
	case ButtonPress:
	case ButtonRelease:
	case MotionNotify:
	case EnterNotify:
	case LeaveNotify:
		UCMP(good->xkey.window, ev->xkey.window, "window");
		UCMP(good->xkey.root, ev->xkey.root, "root");
		UCMP(good->xkey.subwindow, ev->xkey.subwindow, "subwindow");
		DCMP((long)good->xkey.x, (long)ev->xkey.x, "x coord");
		DCMP((long)good->xkey.y, (long)ev->xkey.y, "y coord");
		DCMP((long)good->xkey.x_root, (long)ev->xkey.x_root, "x_root");
		DCMP((long)good->xkey.y_root, (long)ev->xkey.y_root, "y_root");

		switch (good->type) {
		case KeyPress:
		case KeyRelease:
		case ButtonPress:
		case ButtonRelease:
			UCMP((unsigned long)good->xkey.state,
				(unsigned long)ev->xkey.state, "state");	      
			DCMP((long)good->xkey.same_screen,
				(long)ev->xkey.same_screen, "same screen");
			break;
		case MotionNotify:
			UCMP((unsigned long)good->xmotion.state,
				(unsigned long)ev->xmotion.state, "state");	      
			DCMP((long)good->xmotion.is_hint,
				(long)ev->xmotion.is_hint, "is hint");
			DCMP((long)good->xmotion.same_screen,
				(long)ev->xmotion.same_screen, "same screen");
			break;
		default:
			DCMP((long)good->xcrossing.mode,
				(long)ev->xcrossing.mode, "mode");
			DCMP((long)good->xcrossing.detail,
				(long)ev->xcrossing.detail, "detail");
			DCMP((long)good->xcrossing.same_screen,
				(long)ev->xcrossing.same_screen, "same screen");
			DCMP((long)good->xcrossing.focus,
				(long)ev->xcrossing.focus, "focus");
			UCMP((unsigned long)good->xcrossing.state,
				(unsigned long)ev->xcrossing.state, "state");	      
		}

		switch (good->type) {
		case KeyPress:
		case KeyRelease:
			UCMP((unsigned long)good->xkey.keycode,
				(unsigned long)ev->xkey.keycode, "keycode");
			break;
		case ButtonPress:
		case ButtonRelease:
			UCMP((unsigned long)good->xbutton.button,
				(unsigned long)ev->xbutton.button, "button");
			break;
		}
		break;

	case FocusIn:
	case FocusOut:
		UCMP(good->xfocus.window, ev->xfocus.window,
			"window");
		DCMP((long)good->xfocus.mode, (long)ev->xfocus.mode,
			"mode");
		DCMP((long)good->xfocus.detail, (long)ev->xfocus.detail,
			"detail");
		break;

	case KeymapNotify:
		UCMP(good->xkeymap.window, ev->xkeymap.window,
			"window");
		for (i = 0; i < 32; i++) {
			(void)sprintf(tempstr, "key vector[%d]",i);
			DCMP((long)good->xkeymap.key_vector[i],
				(long)ev->xkeymap.key_vector[i], tempstr);
		}
		break;

	case Expose:
		UCMP(good->xexpose.window, ev->xexpose.window,
			"window");
		DCMP((long)good->xexpose.x, (long)ev->xexpose.x,
			"x coord");
		DCMP((long)good->xexpose.y, (long)ev->xexpose.y,
			"y coord");
		DCMP((long)good->xexpose.width, (long)ev->xexpose.width,
			"width");
		DCMP((long)good->xexpose.height, (long)ev->xexpose.height,
			"height");
		DCMP((long)good->xexpose.count, (long)ev->xexpose.count,
			"count");
		break;

	case GraphicsExpose:
		UCMP(good->xgraphicsexpose.drawable,
			ev->xgraphicsexpose.drawable, "drawable");
		DCMP((long)good->xgraphicsexpose.x,
			(long)ev->xgraphicsexpose.x, "x coord");
		DCMP((long)good->xgraphicsexpose.y,
			(long)ev->xgraphicsexpose.y, "y coord");
		DCMP((long)good->xgraphicsexpose.width,
			(long)ev->xgraphicsexpose.width, "width");
		DCMP((long)good->xgraphicsexpose.height,
			(long)ev->xgraphicsexpose.height, "height");
		DCMP((long)good->xgraphicsexpose.count,
			(long)ev->xgraphicsexpose.count, "count");
		DCMP((long)good->xgraphicsexpose.major_code,
			(long)ev->xgraphicsexpose.major_code, "major code");
		DCMP((long)good->xgraphicsexpose.minor_code,
			(long)ev->xgraphicsexpose.minor_code, "minor code");
		break;

	case NoExpose:
		UCMP(good->xnoexpose.drawable, ev->xnoexpose.drawable,
			"drawable");
		DCMP((long)good->xnoexpose.major_code,
			(long)ev->xnoexpose.major_code, "major code");
		DCMP((long)good->xnoexpose.minor_code,
			(long)ev->xnoexpose.minor_code, "minor code");
		break;

	case VisibilityNotify:
		UCMP(good->xvisibility.window, ev->xvisibility.window,
			"window");
		DCMP((long)good->xvisibility.state,
			(long)ev->xvisibility.state, "state");
		break;

	case CreateNotify:
		UCMP(good->xcreatewindow.parent, ev->xcreatewindow.parent,
			"window parent");
		UCMP(good->xcreatewindow.window, ev->xcreatewindow.window,
			"window");
		DCMP((long)good->xcreatewindow.x, (long)ev->xcreatewindow.x,
			"x coord");
		DCMP((long)good->xcreatewindow.y, (long)ev->xcreatewindow.y,
			"y coord");
		DCMP((long)good->xcreatewindow.width,
			(long)ev->xcreatewindow.width, "width");
		DCMP((long)good->xcreatewindow.height,
			(long)ev->xcreatewindow.height, "height");
		DCMP((long)good->xcreatewindow.border_width,
			(long)ev->xcreatewindow.border_width, "border width");
		DCMP((long)good->xcreatewindow.override_redirect,
			(long)ev->xcreatewindow.override_redirect,
			"override redirect");
		break;

	case DestroyNotify:
		UCMP(good->xdestroywindow.event, ev->xdestroywindow.event,
			"event window");
		UCMP(good->xdestroywindow.window, ev->xdestroywindow.window,
			"window");
		break;

	case UnmapNotify:
		UCMP(good->xunmap.event, ev->xunmap.event,
			"event window");
		UCMP(good->xunmap.window, ev->xunmap.window,
			"window");
		DCMP((long)good->xunmap.from_configure,
			(long)ev->xunmap.from_configure, "from configure");
		break;

	case MapNotify:
		UCMP(good->xmap.event, ev->xmap.event,
			"event window");
		UCMP(good->xmap.window, ev->xmap.window, "window");
		DCMP((long)good->xmap.override_redirect,
			(long)ev->xmap.override_redirect, "override redirect");
		break;

	case MapRequest:
		UCMP(good->xmaprequest.parent, ev->xmaprequest.parent,
			"window parent");
		UCMP(good->xmaprequest.window, ev->xmaprequest.window,
			"window");
		break;

	case ReparentNotify:
		UCMP(good->xreparent.event, ev->xreparent.event,
			"event window");
		UCMP(good->xreparent.window, ev->xreparent.window, "window");
		UCMP(good->xreparent.parent, ev->xreparent.parent,
			"window parent");
		DCMP((long)good->xreparent.x, (long)ev->xreparent.x, "x coord");
		DCMP((long)good->xreparent.y, (long)ev->xreparent.y, "y coord");
		DCMP((long)good->xreparent.override_redirect,
			(long)ev->xreparent.override_redirect,
			"override redirect");
		break;

	case ConfigureNotify:
		UCMP(good->xconfigure.event, ev->xconfigure.event,
			"event window");
		UCMP(good->xconfigure.window, ev->xconfigure.window, "window");
		DCMP((long)good->xconfigure.x, (long)ev->xconfigure.x,
			"x coord");
		DCMP((long)good->xconfigure.y, (long)ev->xconfigure.y,
			"y coord");
		DCMP((long)good->xconfigure.width, (long)ev->xconfigure.width,
			"width");
		DCMP((long)good->xconfigure.height, (long)ev->xconfigure.height,
			"height");
		DCMP((long)good->xconfigure.border_width,
			(long)ev->xconfigure.border_width, "border width");
		UCMP(good->xconfigure.above, ev->xconfigure.above,
			"window above");
		DCMP((long)good->xconfigure.override_redirect,
			(long)ev->xconfigure.override_redirect,
			"override redirect");
		break;

	case ConfigureRequest:
		UCMP(good->xconfigurerequest.parent,
			ev->xconfigurerequest.parent, "window parent");
		UCMP(good->xconfigurerequest.window,
			ev->xconfigurerequest.window, "window");
		DCMP((long)good->xconfigurerequest.x,
			(long)ev->xconfigurerequest.x, "x coord");
		DCMP((long)good->xconfigurerequest.y,
			(long)ev->xconfigurerequest.y, "y coord");
		DCMP((long)good->xconfigurerequest.width,
			(long)ev->xconfigurerequest.width, "width");
		DCMP((long)good->xconfigurerequest.height,
			(long)ev->xconfigurerequest.height, "height");
		DCMP((long)good->xconfigurerequest.border_width,
			(long)ev->xconfigurerequest.border_width,
			"border width");
		UCMP(good->xconfigurerequest.above, ev->xconfigurerequest.above,
			"window above");
		DCMP((long)good->xconfigurerequest.detail,
			(long)ev->xconfigurerequest.detail, "detail");
		UCMP(good->xconfigurerequest.value_mask,
			ev->xconfigurerequest.value_mask, "window value mask");
		break;

	case GravityNotify:
		UCMP(good->xgravity.event, ev->xgravity.event, "event window");
		UCMP(good->xgravity.window, ev->xgravity.window, "window");
		DCMP((long)good->xgravity.x, (long)ev->xgravity.x, "x coord");
		DCMP((long)good->xgravity.y, (long)ev->xgravity.y, "y coord");
		break;

	case ResizeRequest:
		UCMP(good->xresizerequest.window, ev->xresizerequest.window,
			"window");
		DCMP((long)good->xresizerequest.width,
			(long)ev->xresizerequest.width, "width");
		DCMP((long)good->xresizerequest.height,
			(long)ev->xresizerequest.height, "height");
		break;

	case CirculateNotify:
		UCMP(good->xcirculate.event, ev->xcirculate.event,
			"event window");
		UCMP(good->xcirculate.window, ev->xcirculate.window, "window");
		DCMP((long)good->xcirculate.place, (long)ev->xcirculate.place,
			"place");
		break;

	case CirculateRequest:
		UCMP(good->xcirculaterequest.parent,
			ev->xcirculaterequest.parent, "window parent");
		UCMP(good->xcirculaterequest.window,
			ev->xcirculaterequest.window, "window");
		DCMP((long)good->xcirculaterequest.place,
			(long)ev->xcirculaterequest.place, "place");
		break;

	case PropertyNotify:
		UCMP(good->xproperty.window, ev->xproperty.window, "window");
		UCMP(good->xproperty.atom, ev->xproperty.atom, "atom");
		DCMP((long)good->xproperty.state, (long)ev->xproperty.state,
			"state");
		break;

	case SelectionClear:
		UCMP(good->xselectionclear.window, ev->xselectionclear.window,
			"window");
		UCMP(good->xselectionclear.selection,
			ev->xselectionclear.selection, "selection");
		break;

	case SelectionRequest:
		UCMP(good->xselectionrequest.owner, ev->xselectionrequest.owner,
			"window owner");
		UCMP(good->xselectionrequest.requestor,
			ev->xselectionrequest.requestor, "window requestor");
		UCMP(good->xselectionrequest.selection,
			ev->xselectionrequest.selection, "atom selection");
		UCMP(good->xselectionrequest.target,
			ev->xselectionrequest.target, "atom target");
		UCMP(good->xselectionrequest.property,
			ev->xselectionrequest.property, "atom property");
		break;

	case SelectionNotify:
		UCMP(good->xselection.requestor, ev->xselection.requestor,
			"window requestor");
		UCMP(good->xselection.selection, ev->xselection.selection,
			"atom selection");
		UCMP(good->xselection.target, ev->xselection.target,
			"atom target");
		UCMP(good->xselection.property, ev->xselection.property,
			"atom property");
		break;

	case ColormapNotify:
		UCMP(good->xcolormap.window, ev->xcolormap.window, "window");
		UCMP(good->xcolormap.colormap, ev->xcolormap.colormap,
			"color map");
		DCMP((long)good->xcolormap.new, (long)ev->xcolormap.new,
			"new");
		DCMP((long)good->xcolormap.state, (long)ev->xcolormap.state,
			"state");
		break;

	case ClientMessage:
		UCMP(good->xclient.window, ev->xclient.window, "window");
		UCMP(good->xclient.message_type, ev->xclient.message_type,
			"message type");
		DCMP((long)good->xclient.format, (long)ev->xclient.format,
			"format");

		switch(good->xclient.format) {

		      case 8:
			for (i = 0; i < 20; i++) {
				(void)sprintf(tempstr, "data.b[%d]", i);
				DCMP((long)good->xclient.data.b[i],
				     (long)ev->xclient.data.b[i], tempstr);
			}
			break;

		      case 16:
			for (i = 0; i < 10; i++) {
				(void)sprintf(tempstr, "data.s[%d]", i);
				DCMP((long)good->xclient.data.s[i],
				     (long)ev->xclient.data.s[i], tempstr);
			}
			break;

		      case 32:
		      default:
			for (i = 0; i < 5; i++) {
				(void)sprintf(tempstr, "data.l[%d]", i);
				DCMP((long)good->xclient.data.l[i],
				     (long)ev->xclient.data.l[i], tempstr);
			}
			
		}


	case MappingNotify:
		DCMP((long)good->xmapping.request, (long)ev->xmapping.request,
			"request");
		if (good->xmapping.request == MappingKeyboard) {
			DCMP((long)good->xmapping.first_keycode,
				(long)ev->xmapping.first_keycode,
				"first keycode");
			DCMP((long)good->xmapping.count,
				(long)ev->xmapping.count, "count");
		}
		break;

	default:
		fail = check_ext_event(good,ev);
		    break;
	}                                                               

	/*
	 * We need to check that the correct number of passes have occurred
	 * on an induvidual basis.  At present check that at least we didn't
	 * just fall through without checking anything.
	 */ 
	if (fail == 0 && pass == 0)
		fail = -1;

	return(fail);
}

check_ext_event(good,ev)
XEvent *good, *ev;
{
int i;
int 	fail;
int 	pass;
extern int XInputFirstEvent;
char	tempstr[50];

	fail = 0;
	pass = 0;
	switch (good->type-XInputFirstEvent) {
	case XI_DeviceKeyPress:
	case XI_DeviceKeyRelease:
	case XI_DeviceButtonPress:
	case XI_DeviceButtonRelease:
	case XI_DeviceMotionNotify:
	case XI_ProximityIn:
	case XI_ProximityOut:
		{
		XDeviceKeyEvent *g = (XDeviceKeyEvent *) good;
		XDeviceKeyEvent *e = (XDeviceKeyEvent *) ev;

		UCMP(g->window, e->window, "window");
		UCMP(g->root, e->root, "root");
		UCMP(g->subwindow, e->subwindow, "subwindow");
		DCMP((long)g->x, (long)e->x, "x coord");
		DCMP((long)g->y, (long)e->y, "y coord");
		DCMP((long)g->x_root, (long)e->x_root, "x_root");
		DCMP((long)g->y_root, (long)e->y_root, "y_root");

		switch (g->type) {
		case XI_DeviceKeyPress:
		case XI_DeviceKeyRelease:
		case XI_DeviceButtonPress:
		case XI_DeviceButtonRelease:
			UCMP((unsigned long)g->state,
				(unsigned long)e->state, "state");	      
			DCMP((long)g->same_screen,
				(long)e->same_screen, "same screen");
			UCMP((unsigned long)g->device_state,
				(unsigned long)e->device_state, "device_state");
			ACMP((unsigned char)g->first_axis,
				(unsigned char)e->first_axis, "first_axis");
			ACMP((unsigned char)g->axes_count,
				(unsigned char)e->axes_count, "axes_count");
			for(i=0; i<g->axes_count; i++)
				ACMP((unsigned char)g->axis_data[i],
				(unsigned char)e->axis_data[i], "axes_count");
			break;
		case XI_DeviceMotionNotify:
			{
			XDeviceMotionEvent *g = (XDeviceMotionEvent *) good;
			XDeviceMotionEvent *e = (XDeviceMotionEvent *) ev;

			UCMP((unsigned long)g->state,
				(unsigned long)e->state, "state");	      
			DCMP((long)g->is_hint,
				(long)e->is_hint, "is hint");
			DCMP((long)g->same_screen,
				(long)e->same_screen, "same screen");
			UCMP((unsigned long)g->device_state,
				(unsigned long)e->device_state, "device_state");
			ACMP((unsigned char)g->first_axis,
				(unsigned char)e->first_axis, "first_axis");
			ACMP((unsigned char)g->axes_count,
				(unsigned char)e->axes_count, "axes_count");
			for(i=0; i<g->axes_count; i++)
				ACMP((unsigned char)g->axis_data[i],
				(unsigned char)e->axis_data[i], "axes_count");
			}
			break;
		}

		switch (g->type) {
		case XI_DeviceKeyPress:
		case XI_DeviceKeyRelease:

			UCMP((unsigned long)g->keycode,
				(unsigned long)e->keycode, "keycode");
			break;
		case XI_DeviceButtonPress:
		case XI_DeviceButtonRelease:
			{
			XDeviceButtonEvent *g = (XDeviceButtonEvent *) good;
			XDeviceButtonEvent *e = (XDeviceButtonEvent *) ev;

			UCMP((unsigned long)g->button,
				(unsigned long)e->button, "button");
			break;
			}
		}
		break;

		}
	case XI_DeviceFocusIn:
	case XI_DeviceFocusOut:
		{
		XDeviceFocusChangeEvent *g = (XDeviceFocusChangeEvent *) good;
		XDeviceFocusChangeEvent *e = (XDeviceFocusChangeEvent *) ev;

		UCMP(g->window, e->window, "window");
		UCMP(g->deviceid, e->deviceid, "deviceid");
		DCMP((long)g->mode, (long)e->mode, "mode");
		DCMP((long)g->detail, (long)e->detail, "detail");
		}
		break;

	case XI_DeviceStateNotify:
		{
		XDeviceStateNotifyEvent *g = (XDeviceStateNotifyEvent *) good;
		XDeviceStateNotifyEvent *e = (XDeviceStateNotifyEvent *) ev;
		XKeyStatus *kg = (XKeyStatus *) g->data;
		XKeyStatus *ke = (XKeyStatus *) e->data;

		UCMP(g->window, e->window, "window");
		UCMP(g->deviceid, e->deviceid, "deviceid");
		DCMP((long)g->num_classes, (long)e->num_classes, "num_classes");
		UCMP(kg->class, ke->class, "class");
		UCMP(kg->length, ke->length, "length");
		if (kg->class == KeyClass) {
			DCMP(kg->num_keys, ke->num_keys, "num_keys");
			for (i = 0; i < kg->num_keys/8; i++) {
			(void)sprintf(tempstr, "key vector[%d]",i);
			DCMP(kg->keys[i], ke->keys[i], tempstr);
			}
		}
		else if (kg->class == ButtonClass) {
			XButtonStatus *bg = (XButtonStatus *) g->data;
			XButtonStatus *be = (XButtonStatus *) e->data;

			DCMP(bg->num_buttons, be->num_buttons, "num_buttons");
			for (i = 0; i < bg->num_buttons/8; i++) {
			(void)sprintf(tempstr, "button vector[%d]",i);
			DCMP(bg->buttons[i], be->buttons[i], tempstr);
			}
		}
		else if (kg->class == ValuatorClass) {
			XValuatorStatus *vg = (XValuatorStatus *) g->data;
			XValuatorStatus *ve = (XValuatorStatus *) e->data;
			UCMP(vg->mode, ve->mode, "num_valuators");
			DCMP(vg->num_valuators, ve->num_valuators, "num_valuators");
			for (i = 0; i < vg->num_valuators; i++) {
			(void)sprintf(tempstr, "valuator vector[%d]",i);
			DCMP(vg->valuators[i], ve->valuators[i], tempstr);
			}
		}
		}
		break;
	case XI_DeviceMappingNotify:
		{
		XDeviceMappingEvent *g = (XDeviceMappingEvent *) good;
		XDeviceMappingEvent *e = (XDeviceMappingEvent *) ev;

		UCMP(g->window, e->window, "window");
		UCMP(g->deviceid, e->deviceid, "deviceid");
		DCMP((long)g->request, (long)e->request, "request");
		if (g->request == MappingKeyboard) {
			DCMP((long)g->first_keycode, (long)e->first_keycode,
				"first keycode");
			DCMP((long)g->count, (long)e->count, "count");
		}
		}
		break;
	case XI_ChangeDeviceNotify:
		{
		XChangeDeviceNotifyEvent *g = (XChangeDeviceNotifyEvent *) good;
		XChangeDeviceNotifyEvent *e = (XChangeDeviceNotifyEvent *) ev;

		UCMP(g->window, e->window, "window");
		UCMP(g->deviceid, e->deviceid, "deviceid");
		DCMP((long)g->request, (long)e->request, "request");
		}
		break;
	default:
		report("Invalid event type %d", good->type);
		tet_result(TET_UNRESOLVED);
		fail = -1;
	        return(fail);
		break;
	}                                                               

	/*
	 * We need to check that the correct number of passes have occurred
	 * on an induvidual basis.  At present check that at least we didn't
	 * just fall through without checking anything.
	 */ 
	if (fail == 0 && pass == 0)
		fail = -1;

	return(fail);
}
