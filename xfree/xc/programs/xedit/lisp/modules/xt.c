/*
 * Copyright (c) 2001 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 *
 * Author: Paulo César Pereira de Andrade
 */

/* $XFree86: xc/programs/xedit/lisp/modules/xt.c,v 1.10 2001/10/20 00:19:36 paulo Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "internal.h"

/*
 * Types
 */
typedef struct {
    XrmQuark qname;
    XrmQuark qtype;
    Cardinal size;
} ResourceInfo;

typedef struct {
    WidgetClass widget_class;
    ResourceInfo **resources;
    Cardinal num_resources;
    Cardinal num_cons_resources;
} ResourceList;

typedef struct {
    Arg *args;
    Cardinal num_args;
} Resources;

typedef struct {
    LispMac *mac;
    LispObj *data;
    /* data is => (list* widget callback argument) */
} CallbackArgs;

/*
 * Prototypes
 */
int xtLoadModule(LispMac*);
void _LispXtCleanupCallback(Widget, XtPointer, XtPointer);

void _LispXtCallback(Widget, XtPointer, XtPointer);

LispObj *Lisp_XtCoerceToWidgetList(LispMac*, LispObj*, char*);
LispObj *Lisp_XtAddCallback(LispMac*, LispObj*, char*);
LispObj *Lisp_XtAppInitialize(LispMac*, LispObj*, char*);
LispObj *Lisp_XtAppMainLoop(LispMac*, LispObj*, char*);
LispObj *Lisp_XtAppPending(LispMac*, LispObj*, char*);
LispObj *Lisp_XtAppProcessEvent(LispMac*, LispObj*, char*);
LispObj *Lisp_XtCreateWidget(LispMac*, LispObj*, char*);
LispObj *Lisp_XtCreateManagedWidget(LispMac*, LispObj*, char*);
LispObj *Lisp_XtCreatePopupShell(LispMac*, LispObj*, char*);
LispObj *Lisp_XtDestroyWidget(LispMac*, LispObj*, char*);
LispObj *Lisp_XtGetValues(LispMac*, LispObj*, char*);
LispObj *Lisp_XtManageChild(LispMac*, LispObj*, char*);
LispObj *Lisp_XtPopup(LispMac*, LispObj*, char*);
LispObj *Lisp_XtPopdown(LispMac*, LispObj*, char*);
LispObj *Lisp_XtRealizeWidget(LispMac*, LispObj*, char*);
LispObj *Lisp_XtSetSensitive(LispMac*, LispObj*, char*);
LispObj *Lisp_XtSetValues(LispMac*, LispObj*, char*);
LispObj *Lisp_XtWidgetToApplicationContext(LispMac*, LispObj*, char*);

LispObj *_LispXtCreateWidget(LispMac*, LispObj*, char*, int);

static Resources *LispConvertResources(LispMac*, LispObj*, Widget,
				       ResourceList*, ResourceList*);
static void LispFreeResources(Resources*);

static int bcmp_action_resource(_Xconst void*, _Xconst void*);
static ResourceInfo *GetResourceInfo(char*, ResourceList*, ResourceList*);
static ResourceList *GetResourceList(WidgetClass);
static int bcmp_action_resource_list(_Xconst void*, _Xconst void*);
static ResourceList *FindResourceList(WidgetClass);
static int qcmp_action_resource_list(_Xconst void*, _Xconst void*);
static ResourceList *CreateResourceList(WidgetClass);
static int qcmp_action_resource(_Xconst void*, _Xconst void*);
static void BindResourceList(ResourceList*);

static void PopdownAction(Widget, XEvent*, String*, Cardinal*);
static void QuitAction(Widget, XEvent*, String*, Cardinal*);


/*
 * Initialization
 */
static LispBuiltin lispbuiltins[] = {
    {"XT-COERCE-TO-WIDGET-LIST",	Lisp_XtCoerceToWidgetList,	1,2,2,},
    {"XT-ADD-CALLBACK",			Lisp_XtAddCallback,		1,3,4,},
    {"XT-APP-INITIALIZE",		Lisp_XtAppInitialize,		1,2,4,},
    {"XT-APP-PENDING",			Lisp_XtAppPending,		1,1,1,},
    {"XT-APP-MAIN-LOOP",		Lisp_XtAppMainLoop,		1,1,1,},
    {"XT-APP-PROCESS-EVENT",		Lisp_XtAppProcessEvent,		1,1,2,},
    {"XT-CREATE-MANAGED-WIDGET",	Lisp_XtCreateManagedWidget,	1,3,4,},
    {"XT-CREATE-WIDGET",		Lisp_XtCreateWidget,		1,3,4,},
    {"XT-CREATE-POPUP-SHELL",		Lisp_XtCreatePopupShell,	1,3,4,},
    {"XT-DESTROY-WIDGET",		Lisp_XtDestroyWidget,		1,1,1,},
    {"XT-GET-VALUES",			Lisp_XtGetValues,		1,2,2,},
    {"XT-MANAGE-CHILD",			Lisp_XtManageChild,		1,1,1,},
    {"XT-POPUP",			Lisp_XtPopup,			1,2,2,},
    {"XT-POPDOWN",			Lisp_XtPopdown,			1,1,1,},
    {"XT-REALIZE-WIDGET",		Lisp_XtRealizeWidget,		1,1,1,},
    {"XT-SET-SENSITIVE",		Lisp_XtSetSensitive,		1,2,2,},
    {"XT-SET-VALUES",			Lisp_XtSetValues,		1,2,2,},
    {"XT-WIDGET-TO-APPLICATION-CONTEXT",Lisp_XtWidgetToApplicationContext,1,1,1,},
};

LispModuleData xtLispModuleData = {
    LISP_MODULE_VERSION,
    xtLoadModule,
};

static ResourceList **resource_list;
static Cardinal num_resource_list;

static Atom delete_window;
static int xtAppContext_t, xtWidget_t, xtWidgetClass_t, xtWidgetList_t;

static XtActionsRec actions[] = {
    {"xt-popdown",	PopdownAction},
    {"xt-quit",		QuitAction},
};

static XrmQuark qCardinal, qInt, qString, qWidget;

/*
 * Implementation
 */
int
xtLoadModule(LispMac *mac)
{
    int i;
    char *fname = "INTERNAL:XT-LOAD-MODULE";

    xtAppContext_t = LispRegisterOpaqueType(mac, "XtAppContext");
    xtWidget_t = LispRegisterOpaqueType(mac, "Widget");
    xtWidgetClass_t = LispRegisterOpaqueType(mac, "WidgetClass");
    xtWidgetList_t = LispRegisterOpaqueType(mac, "WidgetList");

    LispExecute(mac, "(DEFSTRUCT XT-WIDGET-LIST NUM-CHILDREN CHILDREN)\n");

    GCProtect();
    (void)LispSetVariable(mac, ATOM2("CORE-WIDGET-CLASS"),
			  OPAQUE(coreWidgetClass, xtWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TRANSIENT-SHELL-WIDGET-CLASS"),
			  OPAQUE(transientShellWidgetClass, xtWidgetClass_t),
			  fname, 0);

    /* parameters for XtPopup */
    (void)LispSetVariable(mac, ATOM2("XT-GRAB-EXCLUSIVE"),
			  REAL(XtGrabExclusive), fname, 0);
    (void)LispSetVariable(mac, ATOM2("XT-GRAB-NONE"),
			  REAL(XtGrabNone), fname, 0);
    (void)LispSetVariable(mac, ATOM2("XT-GRAB-NONE-EXCLUSIVE"),
			  REAL(XtGrabNonexclusive), fname, 0);

    /* parameters for XtAppProcessEvent */
    (void)LispSetVariable(mac, ATOM2("XT-IM-XEVENT"),
			  REAL(XtIMXEvent), fname, 0);
    (void)LispSetVariable(mac, ATOM2("XT-IM-TIMER"),
			  REAL(XtIMTimer), fname, 0);
    (void)LispSetVariable(mac, ATOM2("XT-IM-ALTERNATE-INPUT"),
			  REAL(XtIMAlternateInput), fname, 0);
    (void)LispSetVariable(mac, ATOM2("XT-IM-SIGNAL"),
			  REAL(XtIMSignal), fname, 0);
    (void)LispSetVariable(mac, ATOM2("XT-IM-ALL"),
			  REAL(XtIMSignal), fname, 0);
    GCUProtect();

    qCardinal = XrmPermStringToQuark(XtRCardinal);
    qInt = XrmPermStringToQuark(XtRInt);
    qString = XrmPermStringToQuark(XtRString);
    qWidget = XrmPermStringToQuark(XtRWidget);

    for (i = 0; i < sizeof(lispbuiltins) / sizeof(lispbuiltins[0]); i++)
	LispAddBuiltinFunction(mac, &lispbuiltins[i]);

    return (1);
}

void
_LispXtCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    CallbackArgs *args = (CallbackArgs*)user_data;
    LispMac *mac = args->mac;
    LispObj *code, *frm = FRM;

    GCProtect();
		/* callback name */               /* reall caller */
    code = CONS(QUOTE(CDR(CDR(args->data))), CONS(OPAQUE(w, xtWidget_t),
		CONS(CAR(CDR(args->data)), CONS(OPAQUE(call_data, 0), NIL))));
		     /* user arguments */
    FRM = CONS(code, FRM);
    GCUProtect();

    (void)Lisp_Funcall(mac, code, "FUNCALL");
    FRM = frm;
}

void
_LispXtCleanupCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    CallbackArgs *args = (CallbackArgs*)user_data;
    LispMac *mac = args->mac;

    UPROTECT(CAR(args->data), args->data);
}

LispObj *
Lisp_XtCoerceToWidgetList(LispMac *mac, LispObj *list, char *fname)
{
    int i;
    WidgetList children;
    Cardinal num_children;
    LispObj *obj, *cdr, *wid;

    if (CAR(list)->type != LispReal_t)
	LispDestroy(mac, "expecting number, at %s", fname);
    if (!CHECKO(CAR(CDR(list)), xtWidgetList_t))
	LispDestroy(mac, "cannot convert %s to XawListReturnStruct, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    num_children = CAR(list)->data.real;
    children = (WidgetList)(CAR(CDR(list))->data.opaque.data);

    GCProtect();
    wid = cdr = NIL;
    for (i = 0; i < num_children; i++) {
	obj = CONS(OPAQUE(children[i], xtWidget_t), NIL);
	if (wid == NIL)
	    wid = cdr = CONS(OPAQUE(children[i], xtWidget_t), NIL);
	else {
	    CDR(cdr) = CONS(OPAQUE(children[i], xtWidget_t), NIL);
	    cdr = CDR(cdr);
	}
    }

    obj = EVAL(CONS(ATOM("MAKE-XT-WIDGET-LIST"),
		    CONS(ATOM(":NUM-CHILDREN"),
			 CONS(REAL(num_children),
			      CONS(ATOM(":CHILDREN"),
				   CONS(QUOTE(wid), NIL))))));
    GCUProtect();

    return (obj);
}

LispObj *
Lisp_XtAddCallback(LispMac *mac, LispObj *list, char *fname)
{
    CallbackArgs *arguments;
    LispObj *widget, *name, *callback, *args, *data;

    widget = CAR(list);
    if (!CHECKO(widget, xtWidget_t))
	LispDestroy(mac,
		    "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    list = CDR(list);

    name = CAR(list);
    if (name->type != LispString_t)
	LispDestroy(mac, "expecting string, at %s", fname);
    list = CDR(list);

    callback = CAR(list);
    if (callback->type != LispAtom_t)
	LispDestroy(mac, "expecting atom, at %s", fname);
    list = CDR(list);

    GCProtect();
    if (list == NIL)
	args = list;
    else
	args = QUOTE(CAR(list));

    data = CONS(widget, CONS(args, callback));
    PROTECT(widget, data);
    GCUProtect();

    arguments = XtNew(CallbackArgs);
    arguments->mac = mac;
    arguments->data = data;

    XtAddCallback((Widget)(widget->data.opaque.data), STRPTR(name),
		  _LispXtCallback, (XtPointer)arguments);
    XtAddCallback((Widget)(widget->data.opaque.data), XtNdestroyCallback,
		  _LispXtCleanupCallback, (XtPointer)arguments);

    return (NIL);
}

LispObj *
Lisp_XtAppInitialize(LispMac *mac, LispObj *list, char *fname)
{
    XtAppContext appcon;
    char *app, *cname;
    Widget shell;
    int zero = 0;
    Resources *resources = NULL;
    String *fallback = NULL;

    if (CAR(list)->type != LispAtom_t)
	LispDestroy(mac, "expecting atom, at %s", fname);
    app = STRPTR(CAR(list));
    list = CDR(list);

    if (CAR(list)->type != LispString_t) {
	LispDestroy(mac, "cannot convert %s to string, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    }
    cname = STRPTR(CAR(list));

    /* check if fallback resources given */
    if (list != NIL && CDR(list)->type == LispCons_t &&
	CDR(CDR(list))->type == LispCons_t) {
	int count;
	LispObj *ptr, *obj = CAR(CDR(CDR(list)));

	if (obj->type != LispCons_t)
	    LispDestroy(mac, "expecting string list, at %s", fname);

	for (ptr = obj, count = 0; ptr->type == LispCons_t;
	     ptr = CDR(ptr), count++)
	    if (CAR(ptr)->type != LispString_t)
		LispDestroy(mac, "%s is not a string, at %s",
			    LispStrObj(mac, CAR(ptr)), fname);

	/* fallback resources was correctly specified */
	fallback = LispMalloc(mac, sizeof(String) * (count + 1));
	for (ptr = obj, count = 0; ptr->type == LispCons_t;
	     ptr = CDR(ptr), count++)
	    fallback[count] = STRPTR(CAR(ptr));
	fallback[count] = NULL;
    }

    GCProtect();
    shell = XtAppInitialize(&appcon, cname, NULL, 0, &zero, NULL,
			    fallback, NULL, 0);
    if (fallback)
	LispFree(mac, fallback);
    (void)LispSetVariable(mac, ATOM(app), OPAQUE(appcon, xtAppContext_t),
			  fname, 0);
    GCUProtect();

    XtAppAddActions(appcon, actions, XtNumber(actions));

    list = CDR(list);
    if (list == NIL || CAR(list) == NIL)
	resources = NULL;
    else if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "expecting argument list, at %s", fname);
    else {
	resources = LispConvertResources(mac, CAR(list), shell,
					 GetResourceList(XtClass(shell)),
					 NULL);
	if (resources) {
	    XtSetValues(shell, resources->args, resources->num_args);
	    LispFreeResources(resources);
	}
    }

    return (OPAQUE(shell, xtWidget_t));
}

LispObj *
Lisp_XtAppMainLoop(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xtAppContext_t))
	LispDestroy(mac,
		    "cannot convert %s to XtAppContext, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    XtAppMainLoop((XtAppContext)(CAR(list)->data.opaque.data));

    return (NIL);
}

LispObj *
Lisp_XtAppPending(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xtAppContext_t))
	LispDestroy(mac,
		    "cannot convert %s to XtAppContext, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (REAL(XtAppPending((XtAppContext)(CAR(list)->data.opaque.data))));
}

LispObj *
Lisp_XtAppProcessEvent(LispMac *mac, LispObj *list, char *fname)
{
    XtInputMask mask;
    XtAppContext appcon;

    if (!CHECKO(CAR(list), xtAppContext_t))
	LispDestroy(mac,
		    "cannot convert %s to XtAppContext, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    appcon = (XtAppContext)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (list == NIL)
	mask = XtIMAll;
    else if (!INTEGER_P(CAR(list)))
	LispDestroy(mac, "expecting XtInputMask, at %s", fname);
    mask = NUMBER_VALUE(CAR(list));
    switch (mask) {
	case 0:
	    break;
	case XtIMXEvent:
	case XtIMTimer:
	case XtIMAlternateInput:
	case XtIMSignal:
	case XtIMAll:
	    XtAppProcessEvent(appcon, mask);
	    break;
	default:
	    LispDestroy(mac, "bad XtInputMask, at %s", mask, fname);
	    break;
    }

    return (NIL);
}

LispObj *
Lisp_XtRealizeWidget(LispMac *mac, LispObj *list, char *fname)
{
    Widget widget;

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac,
		    "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    widget = (Widget)(CAR(list)->data.opaque.data);
    XtRealizeWidget(widget);

    if (XtIsSubclass(widget, shellWidgetClass)) {
	if (!delete_window)
	    delete_window = XInternAtom(XtDisplay(widget),
					"WM_DELETE_WINDOW", False);
	(void)XSetWMProtocols(XtDisplay(widget), XtWindow(widget),
			      &delete_window, 1);
    }

    return (NIL);
}

LispObj *
Lisp_XtDestroyWidget(LispMac *mac, LispObj *list, char *fname)
{
    Widget widget;

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac,
		    "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    widget = (Widget)(CAR(list)->data.opaque.data);
    XtDestroyWidget(widget);

    return (NIL);
}

#define UNMANAGED	0
#define MANAGED		1
#define SHELL		2
LispObj *
Lisp_XtCreateWidget(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispXtCreateWidget(mac, list, fname, UNMANAGED));
}

LispObj *
Lisp_XtCreateManagedWidget(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispXtCreateWidget(mac, list, fname, MANAGED));
}

LispObj *
Lisp_XtCreatePopupShell(LispMac *mac, LispObj *list, char *fname)
{
    return (_LispXtCreateWidget(mac, list, fname, SHELL));
}

LispObj *
_LispXtCreateWidget(LispMac *mac, LispObj *list, char *fname, int options)
{
    char *name;
    WidgetClass widget_class;
    Widget widget, parent;
    Resources *resources = NULL;

    if (CAR(list)->type != LispString_t)
	LispDestroy(mac, "cannot convert %s to char*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    name = STRPTR(CAR(list));
    list = CDR(list);

    if (!CHECKO(CAR(list), xtWidgetClass_t))
	LispDestroy(mac, "cannot convert %s to WidgetClass, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    widget_class = (WidgetClass)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    parent = (Widget)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (options != SHELL)
	widget = XtCreateWidget(name, widget_class, parent, NULL, 0);
    else
	widget = XtCreatePopupShell(name, widget_class, parent, NULL, 0);

    if (list == NIL || CAR(list) == NIL)
	resources = NULL;
    else if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "expecting argument list, at %s", fname);
    else {
	resources = LispConvertResources(mac, CAR(list), widget,
					 GetResourceList(widget_class),
					 GetResourceList(XtClass(parent)));
	XtSetValues(widget, resources->args, resources->num_args);
    }
    if (options == MANAGED)
	XtManageChild(widget);
    if (resources)
	LispFreeResources(resources);

    return (OPAQUE(widget, xtWidget_t));
}

LispObj *
Lisp_XtGetValues(LispMac *mac, LispObj *list, char *fname)
{
    Arg args[1];
    Widget widget;
    ResourceList *rlist, *plist;
    ResourceInfo *resource;
    LispObj *obj, *res, *ptr;
    char c1;
    short c2;
    int c4;
#ifdef LONG64
    long c8;
#endif

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    widget = (Widget)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "expecting string list, at %s", fname);

    rlist = GetResourceList(XtClass(widget));
    plist =  XtParent(widget) ? GetResourceList(XtClass(XtParent(widget))) : NULL;

    GCProtect();
    res = NIL;
    for (list = CAR(list); list != NIL; list = CDR(list)) {
	if (CAR(list)->type != LispString_t)
	    LispDestroy(mac, "%s is not a string, at %s",
			LispStrObj(mac, CAR(list)), fname);
	if ((resource = GetResourceInfo(STRPTR(CAR(list)), rlist, plist))
	     == NULL) {
	    int i;
	    Widget child;

	    for (i = 0; i < rlist->num_resources; i++) {
		if (rlist->resources[i]->qtype == qWidget) {
		    XtSetArg(args[0],
			     XrmQuarkToString(rlist->resources[i]->qname),
			     &child);
		    XtGetValues(widget, args, 1);
		    if (child && XtParent(child) == widget) {
			resource =
			    GetResourceInfo(STRPTR(CAR(list)),
					    GetResourceList(XtClass(child)),
					    NULL);
			if (resource)
			    break;
		    }
		}
	    }
	    if (resource == NULL) {
		fprintf(stderr, "resource %s not available.\n",
			STRPTR(CAR(list)));
		continue;
	    }
	}
	switch (resource->size) {
	    case 1:
		XtSetArg(args[0], STRPTR(CAR(list)), &c1);
		break;
	    case 2:
		XtSetArg(args[0], STRPTR(CAR(list)), &c2);
		break;
	    case 4:
		XtSetArg(args[0], STRPTR(CAR(list)), &c4);
		break;
#ifdef LONG64
	    case 1:
		XtSetArg(args[0], STRPTR(CAR(list)), &c8);
		break;
#endif
	}
	XtGetValues(widget, args, 1);

	/* special resources */
	if (resource->qtype == qString) {
#ifdef LONG64
	    obj = CONS(CAR(list), STRING((char*)c8));
#else
	    obj = CONS(CAR(list), STRING((char*)c4));
#endif
	}
	else if (resource->qtype == qCardinal || resource->qtype == qInt) {
#ifdef LONG64
	    if (sizeof(int) == 8)
		obj = CONS(CAR(list), REAL(c8));
	    else
#endif
	    obj = CONS(CAR(list), REAL(c4));
	}
	else {
	    switch (resource->size) {
		case 1:
		    obj = CONS(CAR(list), OPAQUE(c1, 0));
		    break;
		case 2:
		    obj = CONS(CAR(list), OPAQUE(c2, 0));
		    break;
		case 4:
		    obj = CONS(CAR(list), OPAQUE(c4, 0));
		    break;
#ifdef LONG64
		case 8:
		    obj = CONS(CAR(list), OPAQUE(c8, 0));
		    break;
#endif
	    }
	}

	if (res == NIL)
	    res = ptr = CONS(obj, NIL);
	else {
	    CDR(ptr) = CONS(obj, NIL);
	    ptr = CDR(ptr);
	}
    }
    GCUProtect();

    return (res);
}

LispObj *
Lisp_XtManageChild(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    XtManageChild((Widget)(CAR(list)->data.opaque.data));

    return (NIL);
}

LispObj *
Lisp_XtPopup(LispMac *mac, LispObj *list, char *fname)
{
    XtGrabKind kind;

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    if (CAR(CDR(list))->type != LispReal_t)
	LispDestroy(mac, "cannot convert %s to XtGrabKind, at %s",
		    LispStrObj(mac, CAR(CDR(list))), fname);
    kind = (XtGrabKind)(CAR(CDR(list))->data.real);
    if (kind != XtGrabExclusive && kind != XtGrabNone &&
	kind != XtGrabNonexclusive)
	LispDestroy(mac, "cannot convert %d to XtGrabKind, at %s",
		    kind, fname);
    XtPopup((Widget)(CAR(list)->data.opaque.data), kind);

    return (NIL);
}

LispObj *
Lisp_XtPopdown(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    XtPopdown((Widget)(CAR(list)->data.opaque.data));

    return (NIL);
}

LispObj *
Lisp_XtSetSensitive(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    XtSetSensitive((Widget)(CAR(list)->data.opaque.data),
		   CAR(CDR(list)) != NIL);

    return (CAR(CDR(list)) == NIL ? NIL : T);
}

LispObj *
Lisp_XtSetValues(LispMac *mac, LispObj *list, char *fname)
{
    Widget widget;
    Resources *resources;

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    widget = (Widget)(CAR(list)->data.opaque.data);
    list = CDR(list);
    if (CAR(list)->type != LispCons_t)
	LispDestroy(mac, "expecting string list, at %s", fname);

    resources = LispConvertResources(mac, CAR(list), widget,
				     GetResourceList(XtClass(widget)),
				     XtParent(widget) ?
					GetResourceList(XtClass(XtParent(widget))) :
					NULL);
    XtSetValues(widget, resources->args, resources->num_args);
    LispFreeResources(resources);

    return (NIL);
}

LispObj *
Lisp_XtWidgetToApplicationContext(LispMac *mac, LispObj *list, char *fname)
{
    Widget widget;
    XtAppContext appcon;

    if (!CHECKO(CAR(list), xtWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    widget = (Widget)(CAR(list)->data.opaque.data);
    appcon = XtWidgetToApplicationContext(widget);

    return (OPAQUE(appcon, xtAppContext_t));
}

static Resources *
LispConvertResources(LispMac *mac, LispObj *list, Widget widget,
		     ResourceList *rlist, ResourceList *plist)
{
    char c1;
    short c2;
    int c4;   
#ifdef LONG64
    long c8;
#endif
    XrmValue from, to;
    LispObj *arg, *val;
    ResourceInfo *resource;
    char *fname = "XT-INTERNAL:CONVERT-RESOURCES";
    Resources *resources = (Resources*)XtCalloc(1, sizeof(Resources));

    for (; list != NIL; list = CDR(list)) {
	if (list->type != LispCons_t || CAR(list)->type != LispCons_t) {
	    XtFree((XtPointer)resources);
	    LispDestroy(mac, "expecting cons, at %s", fname);
	}
	arg = CAR(CAR(list));
	val = CDR(CAR(list));

	if (arg->type != LispString_t) {
	    XtFree((XtPointer)resources);
	    LispDestroy(mac, "resource name must be a string, at %s", fname);
	}

	if ((resource = GetResourceInfo(STRPTR(arg), rlist, plist)) == NULL) {
	    int i;
	    Arg args[1];
	    Widget child;

	    for (i = 0; i < rlist->num_resources; i++) {
		if (rlist->resources[i]->qtype == qWidget) {
		    XtSetArg(args[0],
			     XrmQuarkToString(rlist->resources[i]->qname),
			     &child);
		    XtGetValues(widget, args, 1);
		    if (child && XtParent(child) == widget) {
			resource =
			    GetResourceInfo(STRPTR(arg),
					    GetResourceList(XtClass(child)),
					    NULL);
			if (resource)
			    break;
		    }
		}
	    }
	    if (resource == NULL) {
		fprintf(stderr, "resource %s not available.\n", STRPTR(arg));
		continue;
	    }
	}

	if (val->type == LispReal_t || val->type == LispOpaque_t) {
	    resources->args = (Arg*)
		XtRealloc((XtPointer)resources->args,
			  sizeof(Arg) * (resources->num_args + 1));
	    if (val->type == LispReal_t)
		XtSetArg(resources->args[resources->num_args],
			 XrmQuarkToString(resource->qname), (int)val->data.real);
	    else
		XtSetArg(resources->args[resources->num_args],
			 XrmQuarkToString(resource->qname), val->data.opaque.data);
	    ++resources->num_args;
	    continue;
	}
	else if (val->type != LispString_t) {
	    XtFree((XtPointer)resources);
	    LispDestroy(mac,
			"resource value must be string, number or opaque, not %s at %s",
			LispStrObj(mac, val), fname);
	}

	from.size = val == NIL ? 1 : strlen(STRPTR(val)) + 1;
	from.addr = val == NIL ? "" : STRPTR(val);
	switch (to.size = resource->size) {
	    case 1:
		to.addr = (XtPointer)&c1;
		break;
	    case 2:
		to.addr = (XtPointer)&c2;
		break;
	    case 4:
		to.addr = (XtPointer)&c4;
		break;
#ifdef LONG64
	    case 8:
		to.addr = (XtPointer)&c8;
		break;
#endif
	    default:
		fprintf(stderr, "bad resource size %d, for %s.\n",
			to.size, STRPTR(arg));
		continue;
	}

	if (qString == resource->qtype)
#ifdef LONG64
	    c8 = (long)from.addr;
#else
	    c4 = (long)from.addr;
#endif
	else if (!XtConvertAndStore(widget, XtRString, &from,
				    XrmQuarkToString(resource->qtype), &to))
	    /* The type converter already have printed an error message */
	    continue;

	resources->args = (Arg*)
	    XtRealloc((XtPointer)resources->args,
		      sizeof(Arg) * (resources->num_args + 1));
	switch (to.size) {
	    case 1:
		XtSetArg(resources->args[resources->num_args],
			 XrmQuarkToString(resource->qname), c1);
		break;
	    case 2:
		XtSetArg(resources->args[resources->num_args],
			 XrmQuarkToString(resource->qname), c2);
		break;
	    case 4:
		XtSetArg(resources->args[resources->num_args],
			 XrmQuarkToString(resource->qname), c4);
		break;
#ifdef LONG64
	    case 8:
		XtSetArg(resources->args[resources->num_args],
			 XrmQuarkToString(resource->qname), c8);
		break;
#endif
	}
	++resources->num_args;
    }

    return (resources);
}

static void
LispFreeResources(Resources *resources)
{
    if (resources) {
	XtFree((XtPointer)resources->args);
	XtFree((XtPointer)resources);
    }
}

static int
bcmp_action_resource(_Xconst void *string, _Xconst void *resource)
{
    return (strcmp((String)string,
		   XrmQuarkToString((*(ResourceInfo**)resource)->qname)));
}   

static ResourceInfo *
GetResourceInfo(char *name, ResourceList *rlist, ResourceList *plist)
{
    ResourceInfo **resource = NULL;

    if (rlist->resources)
	resource = (ResourceInfo**)
	    bsearch(name, rlist->resources, rlist->num_resources,
		    sizeof(ResourceInfo*), bcmp_action_resource);

    if (resource == NULL && plist) {
	resource = (ResourceInfo**)
	  bsearch(name, &plist->resources[plist->num_resources],
		  plist->num_cons_resources, sizeof(ResourceInfo*),
		  bcmp_action_resource);
    }

    return (resource ? *resource : NULL);
}

static ResourceList *
GetResourceList(WidgetClass wc)
{
    ResourceList *list;

    if ((list = FindResourceList(wc)) == NULL)
	list = CreateResourceList(wc);

    return (list);
}

static int
bcmp_action_resource_list(_Xconst void *wc, _Xconst void *list)
{
    return ((char*)wc - (char*)((*(ResourceList**)list)->widget_class));
}

static ResourceList *
FindResourceList(WidgetClass wc)
{  
    ResourceList **list;

    if (!resource_list)
	return (NULL);

    list = (ResourceList**)
	bsearch(wc, resource_list, num_resource_list,
		sizeof(ResourceList*),  bcmp_action_resource_list);

    return (list ? *list : NULL);
}

static int
qcmp_action_resource_list(_Xconst void *left, _Xconst void *right)
{
    return ((char*)((*(ResourceList**)left)->widget_class) -
	    (char*)((*(ResourceList**)right)->widget_class));
}

static ResourceList *
CreateResourceList(WidgetClass wc)
{
    ResourceList *list;

    list = (ResourceList*)XtMalloc(sizeof(ResourceList));
    list->widget_class = wc;
    list->num_resources = list->num_cons_resources = 0;
    list->resources = NULL;

    resource_list = (ResourceList**)
	XtRealloc((XtPointer)resource_list, sizeof(ResourceList*) *
		  (num_resource_list + 1));
    resource_list[num_resource_list++] = list;
    qsort(resource_list, num_resource_list, sizeof(ResourceList*),
	  qcmp_action_resource_list);
    BindResourceList(list);

    return (list);
}

static int
qcmp_action_resource(_Xconst void *left, _Xconst void *right)
{
    return (strcmp(XrmQuarkToString((*(ResourceInfo**)left)->qname),
		   XrmQuarkToString((*(ResourceInfo**)right)->qname)));
}

static void
BindResourceList(ResourceList *list)
{
    XtResourceList xt_list, cons_list;
    Cardinal i, num_xt, num_cons;

    XtGetResourceList(list->widget_class, &xt_list, &num_xt);
    XtGetConstraintResourceList(list->widget_class, &cons_list, &num_cons);
    list->num_resources = num_xt;
    list->num_cons_resources = num_cons;

    list->resources = (ResourceInfo**)
	XtMalloc(sizeof(ResourceInfo*) * (num_xt + num_cons));

    for (i = 0; i < num_xt; i++) {
	list->resources[i] = (ResourceInfo*)XtMalloc(sizeof(ResourceInfo));
	list->resources[i]->qname =
	    XrmPermStringToQuark(xt_list[i].resource_name);
	list->resources[i]->qtype =
	    XrmPermStringToQuark(xt_list[i].resource_type);
	list->resources[i]->size = xt_list[i].resource_size;
    }

    for (; i < num_xt + num_cons; i++) {
	list->resources[i] = (ResourceInfo*)XtMalloc(sizeof(ResourceInfo));
	list->resources[i]->qname =
	    XrmPermStringToQuark(cons_list[i - num_xt].resource_name);
	list->resources[i]->qtype =
	    XrmPermStringToQuark(cons_list[i - num_xt].resource_type);
	list->resources[i]->size = cons_list[i - num_xt].resource_size;
    }

    XtFree((XtPointer)xt_list);
    if (cons_list)
	XtFree((XtPointer)cons_list);

    qsort(list->resources, list->num_resources, sizeof(ResourceInfo*),
	  qcmp_action_resource);
    if (num_cons)
	qsort(&list->resources[num_xt], list->num_cons_resources,
	      sizeof(ResourceInfo*), qcmp_action_resource);
}

/*ARGSUSED*/
static void
PopdownAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtPopdown(w);
}

/*ARGSUSED*/
static void
QuitAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    exit(0);
}
