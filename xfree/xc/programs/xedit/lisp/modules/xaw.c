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

/* $XFree86: xc/programs/xedit/lisp/modules/xaw.c,v 1.7 2001/10/18 03:15:25 paulo Exp $ */

#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xaw/AsciiSink.h>
#include <X11/Xaw/AsciiSrc.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/MultiSink.h>
#include <X11/Xaw/MultiSrc.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Repeater.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Text.h>
#include <X11/Xaw/TextSink.h>
#include <X11/Xaw/TextSrc.h>
#include <X11/Xaw/Tip.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Tree.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Vendor.h>
#include "internal.h"

/*
 * Prototypes
 */
int xawLoadModule(LispMac*);

LispObj *Lisp_XawCoerceToListReturnStruct(LispMac*, LispObj*, char*);
LispObj *Lisp_XawFormDoLayout(LispMac*, LispObj*, char*);
LispObj *Lisp_XawListHighlight(LispMac*, LispObj*, char*);
LispObj *Lisp_XawListUnhighlight(LispMac*, LispObj*, char*);
LispObj *Lisp_XawTextGetSource(LispMac*, LispObj*, char*);
LispObj *Lisp_XawTextLastPosition(LispMac*, LispObj*, char*);
LispObj *Lisp_XawTextGetInsertionPoint(LispMac*, LispObj*, char*);
LispObj *Lisp_XawTextSetInsertionPoint(LispMac*, LispObj*, char*);

/*
 * Initialization
 */

static LispBuiltin lispbuiltins[] = {
    {"XAW-COERCE-TO-LIST-RETURN-STRUCT",Lisp_XawCoerceToListReturnStruct,1,1,1,},
    {"XAW-FORM-DO-LAYOUT",		Lisp_XawFormDoLayout,		 1,2,2,},
    {"XAW-LIST-HIGHLIGHT",		Lisp_XawListHighlight,		 1,2,2,},
    {"XAW-LIST-UNHIGHLIGHT",		Lisp_XawListUnhighlight,	 1,1,1,},
    {"XAW-TEXT-GET-SOURCE",		Lisp_XawTextGetSource,		 1,1,1,},
    {"XAW-TEXT-LAST-POSITION",		Lisp_XawTextLastPosition,	 1,1,1,},
    {"XAW-TEXT-GET-INSERTION-POINT",	Lisp_XawTextGetInsertionPoint,	 1,1,1,},
    {"XAW-TEXT-SET-INSERTION-POINT",	Lisp_XawTextSetInsertionPoint,	 1,2,2,},
};

LispModuleData xawLispModuleData = {
    LISP_MODULE_VERSION,
    xawLoadModule
};

static int xawWidget_t, xawWidgetClass_t, xawListReturnStruct_t, charpp_t;

/*
 * Implementation
 */
int
xawLoadModule(LispMac *mac)
{
    int i;
    char *fname = "INTERNAL:XAW-LOAD-MODULE";

    xawWidget_t = LispRegisterOpaqueType(mac, "Widget");
    xawWidgetClass_t = LispRegisterOpaqueType(mac, "WidgetClass");
    xawListReturnStruct_t = LispRegisterOpaqueType(mac, "XawListReturnStruct");
    charpp_t = LispRegisterOpaqueType(mac, "char**");

    LispExecute(mac, "(DEFSTRUCT XAW-LIST-RETURN-STRUCT STRING INDEX)\n");

    GCProtect();
    (void)LispSetVariable(mac, ATOM2("ASCII-SINK-OBJECT-CLASS"),
			  OPAQUE(asciiSinkObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("ASCII-SRC-OBJECT-CLASS"),
			  OPAQUE(asciiSinkObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("ASCII-TEXT-WIDGET-CLASS"),
			  OPAQUE(asciiTextWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("BOX-WIDGET-CLASS"),
			  OPAQUE(boxWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("COMMAND-WIDGET-CLASS"),
			  OPAQUE(commandWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("DIALOG-WIDGET-CLASS"),
			  OPAQUE(dialogWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("FORM-WIDGET-CLASS"),
			  OPAQUE(formWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("GRIP-WIDGET-CLASS"),
			  OPAQUE(gripWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("LABEL-WIDGET-CLASS"),
			  OPAQUE(labelWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("LIST-WIDGET-CLASS"),
			  OPAQUE(listWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("MENU-BUTTON-WIDGET-CLASS"),
			  OPAQUE(menuButtonWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("MULTI-SINK-OBJEC-TCLASS"),
			  OPAQUE(multiSinkObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("MULTI-SRC-OBJECT-CLASS"),
			  OPAQUE(multiSrcObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("PANED-WIDGET-CLASS"),
			  OPAQUE(panedWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("PANNER-WIDGET-CLASS"),
			  OPAQUE(pannerWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("PORTHOLE-WIDGET-CLASS"),
			  OPAQUE(portholeWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("REPEATER-WIDGET-CLASS"),
			  OPAQUE(repeaterWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("SCROLLBAR-WIDGET-CLASS"),
			  OPAQUE(scrollbarWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("SIMPLE-MENU-WIDGET-CLASS"),
			  OPAQUE(simpleMenuWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("SIMPLE-WIDGET-CLASS"),
			  OPAQUE(simpleWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("SME-BSB-OBJECT-CLASS"),
			  OPAQUE(smeBSBObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("SME-LINE-OBJECT-CLASS"),
			  OPAQUE(smeLineObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("SME-OBJECT-CLASS"),
			  OPAQUE(smeObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("STRIP-CHART-WIDGET-CLASS"),
			  OPAQUE(stripChartWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TEXT-WIDGET-CLASS"),
			  OPAQUE(textWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TEXT-SINKOBJECT-CLASS"),
			  OPAQUE(textSinkObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TEXT-SRC-OBJECT-CLASS"),
			  OPAQUE(textSrcObjectClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TIP-WIDGET-CLASS"),
			  OPAQUE(tipWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TOGGLE-WIDGET-CLASS"),
			  OPAQUE(toggleWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("TREE-WIDGET-CLASS"),
			  OPAQUE(treeWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("VIEWPORT-WIDGET-CLASS"),
			  OPAQUE(viewportWidgetClass, xawWidgetClass_t),
			  fname, 0);
    (void)LispSetVariable(mac, ATOM2("VENDOR-SHELL-WIDGET-CLASS"),
			  OPAQUE(vendorShellWidgetClass, xawWidgetClass_t),
			  fname, 0);
    GCUProtect();

    for (i = 0; i < sizeof(lispbuiltins) / sizeof(lispbuiltins[0]); i++)
	LispAddBuiltinFunction(mac, &lispbuiltins[i]);

    return (1);
}

LispObj *
Lisp_XawCoerceToListReturnStruct(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *res, *code, *frm = FRM;
    XawListReturnStruct *retlist;

    if (!CHECKO(CAR(list), xawListReturnStruct_t))
	LispDestroy(mac, "cannot convert %s to XawListReturnStruct, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    retlist = (XawListReturnStruct*)(CAR(list)->data.opaque.data);

    GCProtect();
    code = CONS(ATOM("MAKE-XAW-LIST-RETURN-STRUCT"),
		CONS(ATOM(":STRING"),
		       CONS(STRING(retlist->string),
			    CONS(ATOM(":INDEX"),
				 CONS(REAL(retlist->list_index), NIL)))));
    FRM = CONS(code, FRM);
    GCUProtect();

    res = EVAL(code);
    FRM = frm;

    return (res);
}

LispObj *
Lisp_XawFormDoLayout(LispMac *mac, LispObj *list, char *fname)
{
    int force;

    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    force = CAR(CDR(list)) != NIL;
    XawFormDoLayout((Widget)(CAR(list)->data.opaque.data), force);

    return (NIL);
}

LispObj *
Lisp_XawTextGetSource(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (OPAQUE(XawTextGetSource((Widget)(CAR(list)->data.opaque.data)),
		   xawWidget_t));
}

LispObj *
Lisp_XawTextLastPosition(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (REAL(XawTextLastPosition((Widget)(CAR(list)->data.opaque.data))));
}

LispObj *
Lisp_XawTextGetInsertionPoint(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (REAL(XawTextGetInsertionPoint((Widget)(CAR(list)->data.opaque.data))));
}

LispObj *
Lisp_XawTextSetInsertionPoint(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    if (CAR(CDR(list))->type != LispReal_t)
	LispDestroy(mac, "cannot convert %s to XawTextPosition, at %s",
		    LispStrObj(mac, CAR(CDR(list))), fname);

    XawTextSetInsertionPoint((Widget)(CAR(list)->data.opaque.data),
			     (XawTextPosition)(CAR(CDR(list))->data.real));

    return (NIL);
}

LispObj *
Lisp_XawListHighlight(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    if (CAR(CDR(list))->type != LispReal_t)
	LispDestroy(mac, "expecting number, at %s", fname);
    XawListHighlight((Widget)(CAR(list)->data.opaque.data),
		     (int)(CAR(CDR(list))->data.real));

    return (NIL);
}

LispObj *
Lisp_XawListUnhighlight(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), xawWidget_t))
	LispDestroy(mac, "cannot convert %s to Widget, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    XawListUnhighlight((Widget)(CAR(list)->data.opaque.data));

    return (NIL);
}
