/* $XConsortium: Geometry.c,v 1.63 94/04/17 20:14:07 kaleb Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts
Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or Sun not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*

Copyright (c) 1987, 1988, 1994  X Consortium

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

*/

#include "IntrinsicI.h"
#include "ShellP.h"

static void ClearRectObjAreas(r, old)
    RectObj r;
    XWindowChanges* old;
{
    Widget pw = _XtWindowedAncestor((Widget)r);
    int bw2;

    bw2 = old->border_width << 1;
    XClearArea( XtDisplay(pw), XtWindow(pw),
		old->x, old->y,
		old->width + bw2, old->height + bw2,
		TRUE );

    bw2 = r->rectangle.border_width << 1;
    XClearArea( XtDisplay(pw), XtWindow(pw),
		(int)r->rectangle.x, (int)r->rectangle.y,
		(unsigned int)(r->rectangle.width + bw2),
	        (unsigned int)(r->rectangle.height + bw2),
		TRUE );
}

/*
 * Internal function used by XtMakeGeometryRequest and XtSetValues.
 * Returns more data than the public interface.  Does not convert
 * XtGeometryDone to XtGeometryYes.
 *
 * clear_rect_obj - *** RETURNED ***  
 *		    TRUE if the rect obj has been cleared, false otherwise.
 */

XtGeometryResult 
_XtMakeGeometryRequest (widget, request, reply, clear_rect_obj)
    Widget widget;
    XtWidgetGeometry *request, *reply;
    Boolean * clear_rect_obj;
{
    XtWidgetGeometry    junk;
    XtGeometryHandler manager;
    XtGeometryResult returnCode;
    Widget parent = widget->core.parent;
    Boolean managed, parentRealized, rgm = False;
    XtConfigureHookDataRec req;
    Widget hookobj;

    *clear_rect_obj = FALSE;

    if (XtIsShell(widget)) {
	ShellClassExtension ext;
	LOCK_PROCESS;
	for (ext = (ShellClassExtension)((ShellWidgetClass)XtClass(widget))
		   ->shell_class.extension;
	     ext != NULL && ext->record_type != NULLQUARK;
	     ext = (ShellClassExtension)ext->next_extension);

	if (ext != NULL) {
	    if (  ext->version == XtShellExtensionVersion
		  && ext->record_size == sizeof(ShellClassExtensionRec)) {
		manager = ext->root_geometry_manager;
		rgm = True;
	    } else {
		String params[1];
		Cardinal num_params = 1;
		params[0] = XtClass(widget)->core_class.class_name;
		XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		     "invalidExtension", "xtMakeGeometryRequest",
		     XtCXtToolkitError,
		     "widget class %s has invalid ShellClassExtension record",
		     params, &num_params);
	    }
	} else {
	    XtAppErrorMsg(XtWidgetToApplicationContext(widget),
			  "internalError", "xtMakeGeometryRequest",
			  XtCXtToolkitError,
			  "internal error; ShellClassExtension is NULL",
			  NULL, NULL);
	}
	managed = True;
	parentRealized = TRUE;
	UNLOCK_PROCESS;
    } else if (parent == NULL) {
	XtAppErrorMsg(XtWidgetToApplicationContext(widget),
		      "invalidParent","xtMakeGeometryRequest",XtCXtToolkitError,
		      "non-shell has no parent in XtMakeGeometryRequest",
		      (String *)NULL, (Cardinal *)NULL);
    } else /* not shell */ {
	managed = XtIsManaged(widget);

	if (XtIsComposite(parent)) {
	    parentRealized = XtIsRealized(parent);
	    LOCK_PROCESS;
	    manager = ((CompositeWidgetClass) (parent->core.widget_class))
		    ->composite_class.geometry_manager;
	    UNLOCK_PROCESS;
	} else if (managed) {
	    /* Should never happen - XtManageChildren should have checked */
	    XtAppErrorMsg(XtWidgetToApplicationContext(widget),
			  "invalidParent", "xtMakeGeometryRequest",
			  XtCXtToolkitError,
			  "XtMakeGeometryRequest - parent not composite",
			  (String *)NULL, (Cardinal *)NULL);
	} else {
	    /* no need to waste time checking if parent is actually realized
	     * at this point; since the child is unmanaged we need to perform
	     * the configure iff the child is realized, so we dummy the
	     * parentRealized checks below.
	     */
	    parentRealized = TRUE;
	}
    }

    if (managed && manager == (XtGeometryHandler) NULL) {
	XtErrorMsg("invalidGeometryManager","xtMakeGeometryRequest",
                 XtCXtToolkitError,
                 "XtMakeGeometryRequest - parent has no geometry manager",
                  (String *)NULL, (Cardinal *)NULL);
    }

    if (widget->core.being_destroyed) return XtGeometryNo;

    /* see if requesting anything to change */
    req.changeMask = 0;
    if (request->request_mode & CWStackMode
	&& request->stack_mode != XtSMDontChange) {
	    req.changeMask |= CWStackMode;
	    if (request->request_mode & CWSibling) {
		XtCheckSubclass(request->sibling, rectObjClass,
				"XtMakeGeometryRequest");
		req.changeMask |= CWSibling;
	    }
    }
    if (request->request_mode & CWX
	&& widget->core.x != request->x) req.changeMask |= CWX;
    if (request->request_mode & CWY
	&& widget->core.y != request->y) req.changeMask |= CWY;
    if (request->request_mode & CWWidth
	&& widget->core.width != request->width) req.changeMask |= CWWidth;
    if (request->request_mode & CWHeight
	&& widget->core.height != request->height) req.changeMask |= CWHeight;
    if (request->request_mode & CWBorderWidth
	&& widget->core.border_width != request->border_width)
	req.changeMask |= CWBorderWidth;
    if (! req.changeMask) return XtGeometryYes;
    req.changeMask |= (request->request_mode & XtCWQueryOnly);

    if ( !(req.changeMask & XtCWQueryOnly) && XtIsRealized(widget) ) {
	/* keep record of the current geometry so we know what's changed */
	req.changes.x = widget->core.x ;
	req.changes.y = widget->core.y ;
	req.changes.width = widget->core.width ;
	req.changes.height = widget->core.height ;
	req.changes.border_width = widget->core.border_width ;
    }

    if (!managed || !parentRealized) {
	/* Don't get parent's manager involved--assume the answer is yes */
	if (req.changeMask & XtCWQueryOnly) {
	    /* He was just asking, don't change anything, just tell him yes */
	    return XtGeometryYes;
	} else {
	    /* copy values from request to widget */
	    if (request->request_mode & CWX)
		widget->core.x = request->x;
	    if (request->request_mode & CWY)
		widget->core.y = request->y;
	    if (request->request_mode & CWWidth)
		widget->core.width = request->width;
	    if (request->request_mode & CWHeight)
		widget->core.height = request->height;
	    if (request->request_mode & CWBorderWidth)
		widget->core.border_width = request->border_width;
	    if (!parentRealized) return XtGeometryYes;
	    else returnCode = XtGeometryYes;
	}
    } else {
	/* go ask the widget's geometry manager */
	if (reply == (XtWidgetGeometry *) NULL) {
	    returnCode = (*manager)(widget, request, &junk);
	} else {
	    returnCode = (*manager)(widget, request, reply);
	}
    }

    /*
     * If Unrealized, not a XtGeometryYes, or a query-only then we are done.
     */

    if ((returnCode != XtGeometryYes) || 
	(req.changeMask & XtCWQueryOnly) || !XtIsRealized(widget)) {

	return returnCode;
    }

    if (XtIsWidget(widget)) {	/* reconfigure the window (if needed) */

	if (rgm) return returnCode;

	if (req.changes.x != widget->core.x) {
 	    req.changeMask |= CWX;
 	    req.changes.x = widget->core.x;
 	}
 	if (req.changes.y != widget->core.y) {
 	    req.changeMask |= CWY;
 	    req.changes.y = widget->core.y;
 	}
 	if (req.changes.width != widget->core.width) {
 	    req.changeMask |= CWWidth;
 	    req.changes.width = widget->core.width;
 	}
 	if (req.changes.height != widget->core.height) {
 	    req.changeMask |= CWHeight;
 	    req.changes.height = widget->core.height;
 	}
 	if (req.changes.border_width != widget->core.border_width) {
 	    req.changeMask |= CWBorderWidth;
 	    req.changes.border_width = widget->core.border_width;
 	}
	if (req.changeMask & CWStackMode) {
	    req.changes.stack_mode = request->stack_mode;
	    if (req.changeMask & CWSibling)
		if (XtIsWidget(request->sibling))
		    req.changes.sibling = XtWindow(request->sibling);
		else
		    req.changeMask &= ~(CWStackMode | CWSibling);
	}

	XConfigureWindow(XtDisplay(widget), XtWindow(widget),
			 req.changeMask, &req.changes);
    }
    else {			/* RectObj child of realized Widget */
	*clear_rect_obj = TRUE;
	ClearRectObjAreas((RectObj)widget, &req.changes);
    }
    hookobj = XtHooksOfDisplay(XtDisplayOfObject(widget));;
    if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	req.type = XtHconfigure;
	req.widget = widget;
	XtCallCallbackList(hookobj,
		((HookObject)hookobj)->hooks.confighook_callbacks,
		(XtPointer)&req);
    }

    return returnCode;
} /* _XtMakeGeometryRequest */


/* Public routines */

XtGeometryResult XtMakeGeometryRequest (widget, request, reply)
    Widget         widget;
    XtWidgetGeometry *request, *reply;
{
    Boolean junk;
    XtGeometryResult r;
    XtGeometryHookDataRec call_data;
    Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(widget));
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if (XtHasCallbacks(hookobj, XtNgeometryHook) == XtCallbackHasSome) {
	call_data.type = XtHpreGeometry;
	call_data.widget = widget;
	call_data.request = request;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
	call_data.result = r = 
	    _XtMakeGeometryRequest(widget, request, reply, &junk);
	call_data.type = XtHpostGeometry;
	call_data.reply = reply;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
    } else {
	r = _XtMakeGeometryRequest(widget, request, reply, &junk);
    }
    UNLOCK_APP(app);

    return ((r == XtGeometryDone) ? XtGeometryYes : r);
}

#if NeedFunctionPrototypes
XtGeometryResult 
XtMakeResizeRequest(
    Widget	widget,
    _XtDimension width,
    _XtDimension height,
    Dimension	*replyWidth,
    Dimension	*replyHeight)
#else
XtGeometryResult 
XtMakeResizeRequest (widget, width, height, replyWidth, replyHeight)
    Widget	widget;
    Dimension	width, height;
    Dimension	*replyWidth, *replyHeight;
#endif
{
    XtWidgetGeometry request, reply;
    XtGeometryResult r;
    XtGeometryHookDataRec call_data;
    Boolean junk;
    Widget hookobj = XtHooksOfDisplay(XtDisplayOfObject(widget));
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    request.request_mode = CWWidth | CWHeight;
    request.width = width;
    request.height = height;

    if (XtHasCallbacks(hookobj, XtNgeometryHook) == XtCallbackHasSome) {
	call_data.type = XtHpreGeometry;
	call_data.widget = widget;
	call_data.request = &request;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
	call_data.result = r = 
	    _XtMakeGeometryRequest(widget, &request, &reply, &junk);
	call_data.type = XtHpostGeometry;
	call_data.reply = &reply;
	XtCallCallbackList(hookobj, 
		((HookObject)hookobj)->hooks.geometryhook_callbacks, 
		(XtPointer)&call_data);
    } else {
	r = _XtMakeGeometryRequest(widget, &request, &reply, &junk);
    }
    if (replyWidth != NULL)
	if (r == XtGeometryAlmost && reply.request_mode & CWWidth)
	    *replyWidth = reply.width;
	else
	    *replyWidth = width;
    if (replyHeight != NULL)
	if (r == XtGeometryAlmost && reply.request_mode & CWHeight)
	    *replyHeight = reply.height;
	else
	    *replyHeight = height;
    UNLOCK_APP(app);
    return r;
} /* XtMakeResizeRequest */

void XtResizeWindow(w)
    Widget w;
{
    XtConfigureHookDataRec req;
    Widget hookobj;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    if (XtIsRealized(w)) {
	XWindowChanges changes;
	req.changes.width = w->core.width;
	req.changes.height = w->core.height;
	req.changes.border_width = w->core.border_width;
	req.changeMask = CWWidth | CWHeight | CWBorderWidth;
	XConfigureWindow(XtDisplay(w), XtWindow(w),
	    (unsigned) req.changeMask, &req.changes);
	hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));;
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    req.type = XtHconfigure;
	    req.widget = w;
	    XtCallCallbackList(hookobj,
			((HookObject)hookobj)->hooks.confighook_callbacks,
			(XtPointer)&req);
	}
    }
    UNLOCK_APP(app);
} /* XtResizeWindow */

#if NeedFunctionPrototypes
void XtResizeWidget(
    Widget w,
    _XtDimension width,
    _XtDimension height,
    _XtDimension borderWidth
    )
#else
void XtResizeWidget(w, width, height, borderWidth)
    Widget w;
    Dimension width, height, borderWidth;
#endif
{
    XtConfigureHookDataRec req;
    Widget hookobj;
    Dimension old_width, old_height, old_borderWidth;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    req.changeMask = 0;
    if ((old_width = w->core.width) != width) {
	req.changes.width = w->core.width = width;
	req.changeMask |= CWWidth;
    }

    if ((old_height = w->core.height) != height) {
	req.changes.height = w->core.height = height;
	req.changeMask |= CWHeight;
    }

    if ((old_borderWidth = w->core.border_width) != borderWidth) {
	req.changes.border_width = w->core.border_width = borderWidth;
	req.changeMask |= CWBorderWidth;
    }

    if (req.changeMask != 0) {
	if (XtIsRealized(w)) {
	    if (XtIsWidget(w))
		XConfigureWindow(XtDisplay(w), XtWindow(w), 
				 req.changeMask, &req.changes);
	    else {
		Widget pw = _XtWindowedAncestor(w);
		old_width += (old_borderWidth << 1);
		old_height += (old_borderWidth << 1);
		if ((Dimension)(width + (borderWidth << 1)) > old_width)
		    old_width = width + (borderWidth << 1);
		if ((Dimension)(height + (borderWidth << 1)) > old_height)
		    old_height = height + (borderWidth << 1);
		XClearArea(XtDisplay(pw), XtWindow(pw),
			   (int)w->core.x, (int)w->core.y,
			   (unsigned int)old_width, (unsigned int)old_height,
			   TRUE);
	    }
	}
	hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));;
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    req.type = XtHconfigure;
	    req.widget = w;
	    XtCallCallbackList(hookobj,
			((HookObject)hookobj)->hooks.confighook_callbacks,
			(XtPointer)&req);
	}
	{
	XtWidgetProc resize;

	LOCK_PROCESS;
	resize = XtClass(w)->core_class.resize;
	UNLOCK_PROCESS;
	if ((req.changeMask & (CWWidth | CWHeight)) && 
	    resize != (XtWidgetProc) NULL)
	    (*resize)(w);
	}
    }
    UNLOCK_APP(app);
} /* XtResizeWidget */

#if NeedFunctionPrototypes
void XtConfigureWidget(
    Widget w,
    _XtPosition x,
    _XtPosition y,
    _XtDimension width,
    _XtDimension height,
    _XtDimension borderWidth
    )
#else
void XtConfigureWidget(w, x, y, width, height, borderWidth)
    Widget w;
    Position x, y;
    Dimension width, height, borderWidth;
#endif
{
    XtConfigureHookDataRec req;
    Widget hookobj;
    XWindowChanges old;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    req.changeMask = 0;
    if ((old.x = w->core.x) != x) {
	req.changes.x = w->core.x = x;
	req.changeMask |= CWX;
    }

    if ((old.y = w->core.y) != y) {
	req.changes.y = w->core.y = y;
	req.changeMask |= CWY;
    }

    if ((old.width = w->core.width) != width) {
	req.changes.width = w->core.width = width;
	req.changeMask |= CWWidth;
    }

    if ((old.height = w->core.height) != height) {
	req.changes.height = w->core.height = height;
	req.changeMask |= CWHeight;
    }

    if ((old.border_width = w->core.border_width) != borderWidth) {
	req.changes.border_width = w->core.border_width = borderWidth;
	req.changeMask |= CWBorderWidth;
    }

    if (req.changeMask != 0) {
	if (XtIsRealized(w)) {
	    if (XtIsWidget(w))
		XConfigureWindow(XtDisplay(w), XtWindow(w), 
				 req.changeMask, &req.changes);
	    else
		ClearRectObjAreas((RectObj)w, &old);
	}
	hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));;
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    req.type = XtHconfigure;
	    req.widget = w;
	    XtCallCallbackList(hookobj,
			((HookObject)hookobj)->hooks.confighook_callbacks,
			(XtPointer)&req);
	}
	{
	XtWidgetProc resize;

	LOCK_PROCESS;
	resize = XtClass(w)->core_class.resize;
	UNLOCK_PROCESS;
	if ((req.changeMask & (CWWidth | CWHeight)) && 
	    resize != (XtWidgetProc) NULL)
	    (*resize)(w);
	}
    }
    UNLOCK_APP(app);
} /* XtConfigureWidget */

#if NeedFunctionPrototypes
void XtMoveWidget(
    Widget w,
    _XtPosition x,
    _XtPosition y
    )
#else
void XtMoveWidget(w, x, y)
    Widget w;
    Position x, y;
#endif
{
    XtConfigureHookDataRec req;
    XWindowChanges old;
    Widget hookobj;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    req.changeMask = 0;
    if ((old.x = w->core.x) != x) {
	req.changes.x = w->core.x = x;
	req.changeMask |= CWX;
    }

    if ((old.y = w->core.y) != y) {
	req.changes.y = w->core.y = y;
	req.changeMask |= CWY;
    }

    if (req.changeMask != 0) {
	if (XtIsRealized(w)) {
	    if (XtIsWidget(w))
		XMoveWindow(XtDisplay(w), XtWindow(w), w->core.x, w->core.y);
	    else {
		old.width = w->core.width;
		old.height = w->core.height;
		old.border_width = w->core.border_width;
		ClearRectObjAreas((RectObj)w, &old);
	    }
	}
	hookobj = XtHooksOfDisplay(XtDisplayOfObject(w));;
	if (XtHasCallbacks(hookobj, XtNconfigureHook) == XtCallbackHasSome) {
	    req.type = XtHconfigure;
	    req.widget = w;
	    XtCallCallbackList(hookobj,
			((HookObject)hookobj)->hooks.confighook_callbacks,
			(XtPointer)&req);
	}
    }
    UNLOCK_APP(app);
} /* XtMoveWidget */

#if NeedFunctionPrototypes
void XtTranslateCoords(
    register Widget w,
    _XtPosition x,
    _XtPosition y,
    register Position *rootx,	/* return */
    register Position *rooty	/* return */
    )
#else
void XtTranslateCoords(w, x, y, rootx, rooty)
    register Widget w;
    Position x, y;
    register Position *rootx, *rooty;	/* return */
#endif
{
    Position garbagex, garbagey;
    XtAppContext app = XtWidgetToApplicationContext(w);

    LOCK_APP(app);
    if (rootx == NULL) rootx = &garbagex;
    if (rooty == NULL) rooty = &garbagey;

    *rootx = x;
    *rooty = y;

    for (; w != NULL && ! XtIsShell(w); w = w->core.parent) {
	*rootx += w->core.x + w->core.border_width;
	*rooty += w->core.y + w->core.border_width;
    }

    if (w == NULL)
        XtAppWarningMsg(app,
		"invalidShell","xtTranslateCoords",XtCXtToolkitError,
                "Widget has no shell ancestor",
		(String *)NULL, (Cardinal *)NULL);
    else {
	Position x, y;
	extern void _XtShellGetCoordinates();
	_XtShellGetCoordinates( w, &x, &y );
	*rootx += x + w->core.border_width;
	*rooty += y + w->core.border_width;
    }
    UNLOCK_APP(app);
}

XtGeometryResult XtQueryGeometry(widget, intended, reply)
    Widget widget;
    register XtWidgetGeometry *intended; /* parent's changes; may be NULL */
    XtWidgetGeometry *reply;	/* child's preferred geometry; never NULL */
{
    XtWidgetGeometry null_intended;
    XtGeometryHandler query;
    XtGeometryResult result;
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    LOCK_PROCESS;
    query = XtClass(widget)->core_class.query_geometry;
    UNLOCK_PROCESS;
    reply->request_mode = 0;
    if (query != NULL) {
	if (intended == NULL) {
	    null_intended.request_mode = 0;
	    intended = &null_intended;
	}
	result = (*query) (widget, intended, reply);
    }
    else {
	result = XtGeometryYes;
    }

#define FillIn(mask, field) \
	if (!(reply->request_mode & mask)) reply->field = widget->core.field;

    FillIn(CWX, x);
    FillIn(CWY, y);
    FillIn(CWWidth, width);
    FillIn(CWHeight, height);
    FillIn(CWBorderWidth, border_width);
#undef FillIn

    if (!reply->request_mode & CWStackMode) 
	reply->stack_mode = XtSMDontChange;
    UNLOCK_APP(app);
    return result;
}
