/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
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
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/xf86cfg/options.c,v 1.4 2000/11/30 20:55:18 paulo Exp $
 */

#include "options.h"
#include "xf86config.h"
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Viewport.h>

/*
 * Prototypes
 */
static void PopdownCallback(Widget, XtPointer, XtPointer);
static void SelectOptionCallback(Widget, XtPointer, XtPointer);
static void AddOption(Widget, XtPointer, XtPointer);
static void RemoveOption(Widget, XtPointer, XtPointer);
static void UpdateOption(Widget, XtPointer, XtPointer);
static void UpdateOptionList(void);
#ifdef USE_MODULES
static void AddDriverOption(Widget, XtPointer, XtPointer);
#endif

/*
 * Initialization
 */
Widget optionsShell;
static XF86OptionPtr *options;
static Widget add, remov, update, list, name, value;
static char *option_str;
static int option_index, popped = False;

/*
 * Implementation
 */
void
CreateOptionsShell(void)
{
    optionsShell = XtCreatePopupShell("options", transientShellWidgetClass,
				      toplevel, NULL, 0);
}

#ifdef USE_MODULES
void
OptionsPopup(XF86OptionPtr *opts, char *driver, OptionInfoPtr drv_opts)
#else
void
OptionsPopup(XF86OptionPtr *opts)
#endif
{
    static int first = 1;
#ifdef USE_MODULES
    static Widget button, menu;
    static char label[256], menuName[16];
    Widget sme;
    char buf[256];
    int i = 0;
    Arg args[1];
    static int menuN;
#endif

    option_str = NULL;
    options = opts;
    if (first) {
	Widget pane, form, viewport, bottom, popdown;

	first = 0;

	if (optionsShell == NULL)
	    CreateOptionsShell();
	pane = XtCreateManagedWidget("pane", panedWidgetClass,
				     optionsShell, NULL, 0);
	form = XtCreateManagedWidget("commands", formWidgetClass,
				     pane, NULL, 0);
	add = XtCreateManagedWidget("add", commandWidgetClass,
				    form, NULL, 0);
	XtAddCallback(add, XtNcallback, AddOption, NULL);
	remov = XtCreateManagedWidget("remove", commandWidgetClass,
				      form, NULL, 0);
	XtAddCallback(remov, XtNcallback, RemoveOption, NULL);
	update = XtCreateManagedWidget("update", commandWidgetClass,
				       form, NULL, 0);
	XtAddCallback(update, XtNcallback, UpdateOption, NULL);
	form = XtCreateManagedWidget("form", formWidgetClass,
				     pane, NULL, 0);
	XtVaCreateManagedWidget("label1", labelWidgetClass, form,
				XtNlabel, " Option \"",
				NULL, 0);
	name = XtVaCreateManagedWidget("name", asciiTextWidgetClass, form,
				       XtNeditType, XawtextEdit,
				       NULL, 0);
	XtVaCreateManagedWidget("label2", labelWidgetClass,
				form,
				XtNlabel, "\" \"",
				NULL, 0);
	value = XtVaCreateManagedWidget("value", asciiTextWidgetClass, form,
					XtNeditType, XawtextEdit,
					NULL, 0);
	XtVaCreateManagedWidget("label3", labelWidgetClass, form,
				XtNlabel, "\" ",
				NULL, 0);
	viewport = XtCreateManagedWidget("viewport", viewportWidgetClass,
					 form, NULL, 0);
	list = XtCreateManagedWidget("list", listWidgetClass,
				     viewport, NULL, 0);
	XtAddCallback(list, XtNcallback, SelectOptionCallback, NULL);
	bottom = XtCreateManagedWidget("bottom", formWidgetClass,
				       pane, NULL, 0);
#ifdef USE_MODULES
	button = XtCreateManagedWidget("driverOpts", menuButtonWidgetClass,
					bottom, NULL, 0);
#endif
	popdown = XtVaCreateManagedWidget("popdown", commandWidgetClass,
					bottom, NULL, 0);
#ifdef USE_MODULES
	XtVaSetValues(popdown, XtNfromHoriz, button, NULL, 0);
#endif

	XtAddCallback(popdown, XtNcallback, PopdownCallback, NULL);
	XtRealizeWidget(optionsShell);
	XSetWMProtocols(DPY, XtWindow(optionsShell), &wm_delete_window, 1);

#ifdef USE_MODULES
	{
	    char *str;

	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(button, args, 1);
	    XmuSnprintf(label, sizeof(label), "%s", str);
	}
#endif
    }

#ifdef USE_MODULES
    if (menu)
	XtDestroyWidget(menu);
    XmuSnprintf(menuName, sizeof(buf), "optionM%d", menuN);
    menuN = !menuN;
    menu = XtCreatePopupShell(menuName, simpleMenuWidgetClass, button,
			      NULL, 0);
    XtVaSetValues(button, XtNmenuName, menuName, NULL, 0);
    if (drv_opts) {
	int len, longest = 0;
	char fmt[32];
	static char *types[] = {
	    "none", "integer", "(non null) string", "string", "real",
	    "boolean", "frequency",
	};

	for (i = 0; drv_opts[i].name != NULL; i++) {
	    len = strlen(drv_opts[i].name);
	    if (len > longest)
		longest = len;
	}
	XmuSnprintf(fmt, sizeof(fmt), "%c-%ds  %%s", '%', longest);
	for (; drv_opts->name != NULL; drv_opts++) {
	    char *type;

	    if (drv_opts->type >= OPTV_NONE && drv_opts->type <= OPTV_FREQ)
		type = types[drv_opts->type];
	    else
		type = "UNKNOWN";

	    XmuSnprintf(buf, sizeof(buf), fmt, drv_opts->name, type);
	    sme = XtVaCreateManagedWidget(drv_opts->name, smeBSBObjectClass,
					  menu, XtNlabel, buf, NULL, 0);
	    XtAddCallback(sme, XtNcallback, AddDriverOption, (XtPointer)drv_opts);
	}
    }
    if (i) {
	XmuSnprintf(buf, sizeof(buf), "%s%s", label, driver);
	XtSetArg(args[0], XtNlabel, buf);
	XtSetValues(button, args, 1);
	XtMapWidget(button);
    }
    else
	XtUnmapWidget(button);
#endif

    UpdateOptionList();
    popped = True;
    XtPopup(optionsShell, XtGrabExclusive);

    while (popped)
	XtAppProcessEvent(XtWidgetToApplicationContext(optionsShell), XtIMAll);
}

static void
UpdateOptionList(void)
{
    Arg args[2];
    char **ops, **oldops;
    int nops, oldnops;
    XF86OptionPtr opt;

    ops = NULL;
    nops = 0;
    XawListUnhighlight(list);
    XtSetArg(args[0], XtNlist, &oldops);
    XtSetArg(args[1], XtNnumberStrings, &oldnops);
    XtGetValues(list, args, 2);
    opt = *options;
    while (opt != NULL) {
	if (nops % 16 == 0)
	    ops = (char**)XtRealloc((XtPointer)ops, (nops + 16) *
				    sizeof(char*));
	ops[nops++] = XtNewString(opt->opt_name);
	opt = (XF86OptionPtr)(opt->list.next);
    }
    if (nops == 0) {
	ops = (char**)XtMalloc(sizeof(char*));
	ops[0] = XtNewString("");
	nops = 1;
    }
    XtSetArg(args[0], XtNlist, ops);
    XtSetArg(args[1], XtNnumberStrings, nops);
    XtSetValues(list, args, 2);
    if (oldnops > 0 &&
	(oldnops != 1 || XtName(list) != oldops[0])) {
	while (--oldnops >= 0)
	    XtFree(oldops[oldnops]);
	XtFree((XtPointer)oldops);
    }

    XtSetArg(args[0], XtNstring, "");
    XtSetValues(name, args, 1);
    XtSetValues(value, args, 1);

    /* force relayout */
    XtUnmanageChild(list);
    XtManageChild(list);

    XtSetSensitive(remov, False);
    XtSetSensitive(update, False);
}

/*ARGSUSED*/
static void
PopdownCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtPopdown(optionsShell);
    popped = False;
}

/*ARGSUSED*/
void
OptionsCancelAction(Widget w, XEvent *event,
		    String *params, Cardinal *num_params)
{
    PopdownCallback(w, NULL, NULL);
}

/*ARGSUSED*/
static void
SelectOptionCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    XF86OptionPtr option;
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;

    option_str = info->string;
    option_index = info->list_index;
    if ((option = xf86findOption(*options, info->string)) != NULL) {
	XtSetArg(args[0], XtNstring, option->opt_name);
	XtSetValues(name, args, 1);
	XtSetArg(args[0], XtNstring,
		 option->opt_val != NULL ? option->opt_val : "");
	XtSetValues(value, args, 1);
    }
    XtSetSensitive(remov, True);
    XtSetSensitive(update, True);
}

/*ARGSUSED*/
static void
AddOption(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    char *nam, *val;

    XtSetArg(args[0], XtNstring, &nam);
    XtGetValues(name, args, 1);
    XtSetArg(args[0], XtNstring, &val);
    XtGetValues(value, args, 1);
    if (xf86findOption(*options, nam) != NULL || strlen(nam) == 0)
	/* XXX xf86addNewOption will trash the option linked list if
	 * the options being added already exists.
	 */
	return;
    *options = xf86addNewOption(*options, XtNewString(nam),
				val && strlen(val) ? XtNewString(val) : NULL);
    UpdateOptionList();
}

#ifdef USE_MODULES
/*ARGSUSED*/
static void
AddDriverOption(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    OptionInfoPtr opt = (OptionInfoPtr)user_data;
    XF86OptionPtr option;

    XtSetArg(args[0], XtNstring, opt->name);
    XtSetValues(name, args, 1);
    if ((option = xf86findOption(*options, opt->name)) == NULL)
	XtSetArg(args[0], XtNstring, "");
    else
	XtSetArg(args[0], XtNstring, option->opt_val);
    XtSetValues(value, args, 1);
}
#endif

/*ARGSUSED*/
static void
RemoveOption(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    char *str;

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(name, args, 1);
    xf86removeOption(options, str);
    UpdateOptionList();
}

/*ARGSUSED*/
static void
UpdateOption(Widget w, XtPointer user_data, XtPointer call_data)
{
/*    xf86removeOption(options, option_str);
    AddOption(w, user_data, call_data);
    UpdateOptionList();*/

    Arg args[1];
    char *nam, *val;
    XF86OptionPtr option;

    XtSetArg(args[0], XtNstring, &nam);
    XtGetValues(name, args, 1);
    XtSetArg(args[0], XtNstring, &val);
    XtGetValues(value, args, 1);
    if ((option = xf86findOption(*options, option_str)) == NULL)
	return;
    XtFree(option->opt_name);
    option->opt_name = option_str = XtNewString(nam);
    XtFree(option->opt_val);
    if (val && strlen(val))
	option->opt_val = XtNewString(val);
    else
	option->opt_val = NULL;

    UpdateOptionList();
    XawListHighlight(list, option_index);
    XtSetArg(args[0], XtNstring, option->opt_name);
    XtSetValues(name, args, 1);
    XtSetArg(args[0], XtNstring, option->opt_val);
    XtSetValues(value, args, 1);

    XtSetSensitive(remov, True);
    XtSetSensitive(update, True);
}
