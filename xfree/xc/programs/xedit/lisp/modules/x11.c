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

/* $XFree86: xc/programs/xedit/lisp/modules/x11.c,v 1.4 2001/10/15 07:05:53 paulo Exp $ */

#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include <X11/Xlib.h>

/*
 * Prototypes
 */
int x11LoadModule(LispMac*);

LispObj *Lisp_XOpenDisplay(LispMac*, LispObj*, char*);
LispObj *Lisp_XCloseDisplay(LispMac*, LispObj*, char*);
LispObj *Lisp_XDefaultRootWindow(LispMac*, LispObj*, char*);
LispObj *Lisp_XDefaultScreenOfDisplay(LispMac*, LispObj*, char*);
LispObj *Lisp_XBlackPixelOfScreen(LispMac*, LispObj*, char*);
LispObj *Lisp_XWhitePixelOfScreen(LispMac*, LispObj*, char*);
LispObj *Lisp_XDefaultGCOfScreen(LispMac*, LispObj*, char*);
LispObj *Lisp_XCreateSimpleWindow(LispMac*, LispObj*, char*);
LispObj *Lisp_XMapWindow(LispMac*, LispObj*, char*);
LispObj *Lisp_XDestroyWindow(LispMac*, LispObj*, char*);
LispObj *Lisp_XFlush(LispMac*, LispObj*, char*);

LispObj *Lisp_XDrawLine(LispMac*, LispObj*, char*);

/*
 * Initialization
 */
static LispBuiltin lispbuiltins[] = {
    {"X-OPEN-DISPLAY",			Lisp_XOpenDisplay,		1,0,1,},
    {"X-CLOSE-DISPLAY",			Lisp_XCloseDisplay,		1,1,1,},
    {"X-DEFAULT-ROOT-WINDOW",		Lisp_XDefaultRootWindow,	1,1,1,},
    {"X-DEFAULT-SCREEN-OF-DISPLAY",	Lisp_XDefaultScreenOfDisplay,	1,1,1,},
    {"X-BLACK-PIXEL-OF-SCREEN",		Lisp_XBlackPixelOfScreen,	1,1,1,},
    {"X-WHITE-PIXEL-OF-SCREEN",		Lisp_XWhitePixelOfScreen,	1,1,1,},
    {"X-DEFAULT-GC-OF-SCREEN",		Lisp_XDefaultGCOfScreen,	1,1,1,},
    {"X-CREATE-SIMPLE-WINDOW",		Lisp_XCreateSimpleWindow,	1,9,9,},
    {"X-MAP-WINDOW",			Lisp_XMapWindow,		1,2,2,},
    {"X-DESTROY-WINDOW",		Lisp_XDestroyWindow,		1,2,2,},
    {"X-FLUSH",				Lisp_XFlush,			1,1,1,},
    {"X-DRAW-LINE",			Lisp_XDrawLine,			1,7,7,},
};

LispModuleData x11LispModuleData = {
    LISP_MODULE_VERSION,
    x11LoadModule
};

static int x11Display_t, x11Screen_t, x11Window_t, x11GC_t;

/*
 * Implementation
 */
int
x11LoadModule(LispMac *mac)
{
    int i;

    x11Display_t = LispRegisterOpaqueType(mac, "Display*");
    x11Screen_t = LispRegisterOpaqueType(mac, "Screen*");
    x11Window_t = LispRegisterOpaqueType(mac, "Window");
    x11GC_t = LispRegisterOpaqueType(mac, "GC");

    for (i = 0; i < sizeof(lispbuiltins) / sizeof(lispbuiltins[0]); i++)
	LispAddBuiltinFunction(mac, &lispbuiltins[i]);

    return (1);
}

LispObj *
Lisp_XOpenDisplay(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *nam;
    char *dname;

    if (list == NIL)
	dname = NULL;
    else if ((nam = CAR(list))->type == LispString_t)
	dname = STRPTR(nam);
    else
	LispDestroy(mac, "%s is not a valid display name, at %s",
		    LispStrObj(mac, nam), fname);

    return (OPAQUE(XOpenDisplay(dname), x11Display_t));
}

LispObj *
Lisp_XCloseDisplay(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    XCloseDisplay((Display*)(CAR(list)->data.opaque.data));

    return (NIL);
}

LispObj *
Lisp_XDefaultRootWindow(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (OPAQUE(XDefaultRootWindow((Display*)(CAR(list)->data.opaque.data)),
		   x11Window_t));
}

LispObj *
Lisp_XDefaultScreenOfDisplay(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (OPAQUE(XDefaultScreenOfDisplay((Display*)(CAR(list)->data.opaque.data)),
		   x11Screen_t));
}

LispObj *
Lisp_XBlackPixelOfScreen(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), x11Screen_t))
	LispDestroy(mac, "cannot convert %s to Screen*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (REAL((double)
		 XBlackPixelOfScreen((Screen*)(CAR(list)->data.opaque.data))));
}

LispObj *
Lisp_XWhitePixelOfScreen(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), x11Screen_t))
	LispDestroy(mac, "cannot convert %s to Screen*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (REAL((double)
		 XWhitePixelOfScreen((Screen*)(CAR(list)->data.opaque.data))));
}

LispObj *
Lisp_XDefaultGCOfScreen(LispMac *mac, LispObj *list, char *fname)
{
    if (!CHECKO(CAR(list), x11Screen_t))
	LispDestroy(mac, "cannot convert %s to Screen*, at %s",
		    LispStrObj(mac, CAR(list)), fname);

    return (OPAQUE(XDefaultGCOfScreen((Screen*)(CAR(list)->data.opaque.data)),
		   x11GC_t));
}

LispObj *
Lisp_XCreateSimpleWindow(LispMac *mac, LispObj *list, char *fname)
{
    Display *display;
    Window parent;
    int x, y;
    unsigned int width, height, border_width;
    unsigned long border, background;

    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    display = (Display*)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (!CHECKO(CAR(list), x11Window_t))
	LispDestroy(mac, "cannot convert %s to Window, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    parent = (Window)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    x = (int)(CAR(list)->data.real);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    y = (int)(CAR(list)->data.real);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	CAR(list)->data.real < 0 ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to unsigned int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    width = (unsigned int)(CAR(list)->data.real);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	CAR(list)->data.real < 0 ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to unsigned int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    height = (unsigned int)(CAR(list)->data.real);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	CAR(list)->data.real < 0 ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to unsigned int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    border_width = (unsigned int)(CAR(list)->data.real);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	CAR(list)->data.real < 0 ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to unsigned long, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    border = (unsigned int)(CAR(list)->data.real);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t ||
	CAR(list)->data.real < 0 ||
	(int)(CAR(list)->data.real) != CAR(list)->data.real)
	LispDestroy(mac, "Cannot convert %s to unsigned long, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    background = (unsigned int)(CAR(list)->data.real);

    return (OPAQUE(
	    XCreateSimpleWindow(display, parent, x, y, width, height,
				border_width, border, background),
	    x11Window_t));
}

LispObj *
Lisp_XMapWindow(LispMac *mac, LispObj *list, char *fname)
{
    Display *display;
    Window window;

    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    display = (Display*)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (!CHECKO(CAR(list), x11Window_t))
	LispDestroy(mac, "cannot convert %s to Window, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    window = (Window)(CAR(list)->data.opaque.data);

    XMapWindow(display, window);

    return (CAR(list));
}

LispObj *
Lisp_XDestroyWindow(LispMac *mac, LispObj *list, char *fname)
{
    Display *display;
    Window window;

    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    display = (Display*)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (!CHECKO(CAR(list), x11Window_t))
	LispDestroy(mac, "cannot convert %s to Window, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    window = (Window)(CAR(list)->data.opaque.data);

    XDestroyWindow(display, window);

    return (NIL);
}

LispObj *
Lisp_XFlush(LispMac *mac, LispObj *list, char *fname)
{
    Display *display;

    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    display = (Display*)(CAR(list)->data.opaque.data);

    XFlush(display);

    return (NIL);
}

LispObj *
Lisp_XDrawLine(LispMac *mac, LispObj *list, char *fname)
{
    Display *display;
    Drawable window;
    GC gc;
    int x0, y0, x1, y1;

    if (!CHECKO(CAR(list), x11Display_t))
	LispDestroy(mac, "cannot convert %s to Display*, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    display = (Display*)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (!CHECKO(CAR(list), x11Window_t))
	LispDestroy(mac, "cannot convert %s to Drawable, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    window = (Drawable)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (!CHECKO(CAR(list), x11GC_t))
	LispDestroy(mac, "cannot convert %s to GC, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    gc = (GC)(CAR(list)->data.opaque.data);
    list = CDR(list);

    if (CAR(list)->type != LispReal_t)
	LispDestroy(mac, "Cannot convert %s to int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    x0 = (int)(CAR(list)->data.real);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t)
	LispDestroy(mac, "Cannot convert %s to int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    y0 = (int)(CAR(list)->data.real);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t)
	LispDestroy(mac, "Cannot convert %s to int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    x1 = (int)(CAR(list)->data.real);
    list = CDR(list);
    if (CAR(list)->type != LispReal_t)
	LispDestroy(mac, "Cannot convert %s to int, at %s",
		    LispStrObj(mac, CAR(list)), fname);
    y1 = (int)(CAR(list)->data.real);
    list = CDR(list);

    XDrawLine(display, window, gc, x0, y0, x1, y1);

    return (NIL);
}
