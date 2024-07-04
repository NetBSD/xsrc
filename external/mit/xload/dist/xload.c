/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
/*
 * Copyright (c) 2000, 2023, Oracle and/or its affiliates.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


/*
 * xload - display system load average in a window
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xmu/SysUtil.h>
#include "xload.h"

#ifdef USE_GETTEXT
# include <X11/Xlocale.h>
# include <libintl.h>
#else
# define gettext(a) (a)
#endif

#include "xload.bit"

static char *ProgramName;

static void quit(Widget w, XEvent *event, String *params, Cardinal *num_params);
static void ClearLights(Display *dpy);
static void SetLights(XtPointer data, XtIntervalId *timer);


/*
 * Command line options table.  Only resources are entered here...there is a
 * pass over the remaining options after XtParseCommand is let loose.
 */

static XrmOptionDescRec options[] = {
 {"-scale",		"*load.minScale",	XrmoptionSepArg,	NULL},
 {"-update",		"*load.update",		XrmoptionSepArg,	NULL},
 {"-hl",		"*load.highlight",	XrmoptionSepArg,	NULL},
 {"-highlight",		"*load.highlight",	XrmoptionSepArg,	NULL},
 {"-label",		"*label.label",		XrmoptionSepArg,	NULL},
 {"-nolabel",		"*showLabel",	        XrmoptionNoArg,       "False"},
 {"-lights",		"*useLights",		XrmoptionNoArg,	      "True"},
 {"-jumpscroll",	"*load.jumpScroll",	XrmoptionSepArg,	NULL},
 {"-remote",            "*remote",              XrmoptionSepArg,        NULL},
};

/*
 * The structure containing the resource information for the
 * Xload application resources.
 */

#define Offset(field) (XtOffsetOf(XLoadResources, field))

static XtResource my_resources[] = {
  {"showLabel", XtCBoolean, XtRBoolean, sizeof(Boolean),
     Offset(show_label), XtRImmediate, (XtPointer) TRUE},
  {"useLights", XtCBoolean, XtRBoolean, sizeof(Boolean),
    Offset(use_lights), XtRImmediate, (XtPointer) FALSE},
  {"remote", XtCString, XtRString, sizeof(XtRString),
    Offset(remote), XtRImmediate, (XtPointer) FALSE},

};

#undef Offset

XLoadResources resources;

static XtActionsRec xload_actions[] = {
    { "quit",	quit },
};
static Atom wm_delete_window;
static int light_update = 10 * 1000;

/*
 * Exit with message describing command line format.
 */

static void _X_NORETURN
usage(int exitval)
{
    fprintf (stderr, gettext("usage:  %s [-options ...]\n\n%s\n"),
             ProgramName, gettext(
      "where options include:\n"
      "    -display <display>      X server on which to display\n"
      "    -geometry <geometry>    size and location of window\n"
      "    -fn <font>              font to use in label\n"
      "    -scale <number>         minimum number of scale lines\n"
      "    -update <seconds>       interval between updates\n"
      "    -label <string>         annotation text\n"
      "    -bg <color>             background color\n"
      "    -fg <color>             graph color\n"
      "    -hl <color>             scale and text color\n"
      "    -nolabel                removes the label from above the chart.\n"
      "    -jumpscroll <value>     number of pixels to scroll on overflow\n"
      "    -lights                 use keyboard leds to display current load\n"
      "    -help                   print this message\n"
      "    -version                print version info\n"
                 ));
    exit(exitval);
}

int
main(int argc, char **argv)
{
    XtAppContext app_con;
    Widget toplevel, load, pane, label_wid, load_parent;
    Arg args[1];
    Pixmap icon_pixmap = None;
    char *label, host[256];
    const char *domaindir;

    XtSetLanguageProc ( NULL, NULL, NULL );

    ProgramName = argv[0];

    /* Handle args that don't require opening a display or load info source */
    for (int n = 1; n < argc; n++) {
	const char *argn = argv[n];
	/* accept single or double dash for -help & -version */
	if (argn[0] == '-' && argn[1] == '-') {
	    argn++;
	}
	if (strcmp(argn, "-help") == 0) {
	    usage(0);
	}
	if (strcmp(argn, "-version") == 0) {
	    puts(PACKAGE_STRING);
	    exit(0);
	}
    }

    /* For security reasons, we reset our uid/gid after doing the necessary
       system initialization and before calling any X routines. */
    InitLoadPoint();

#if !defined(_WIN32) || defined(__CYGWIN__)
    /* reset gid first while still (maybe) root */
    if (setgid(getgid()) == -1) {
	    fprintf(stderr, gettext("%s: setgid failed: %s\n"),
		ProgramName, strerror(errno));
	    exit(1);
    }
    if (setuid(getuid()) == -1) {
	    fprintf(stderr, gettext("%s: setuid failed: %s\n"),
		ProgramName, strerror(errno));
	    exit(1);
    }
#endif

    XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

    toplevel = XtAppInitialize(&app_con, "XLoad", options, XtNumber(options),
			       &argc, argv, NULL, NULL, (Cardinal) 0);

#ifdef USE_GETTEXT
    textdomain("xload");

    if ((domaindir = getenv ( "TEXTDOMAINDIR" )) == NULL) {
	domaindir = LOCALEDIR;
    }
    bindtextdomain("xload", domaindir);
#endif

    if (argc != 1) {
	fputs(gettext("Unknown argument(s):"), stderr);
	for (int n = 1; n < argc; n++) {
	    fprintf(stderr, " %s", argv[n]);
	}
	fputs("\n\n", stderr);
	usage(1);
    }

    XtGetApplicationResources( toplevel, (XtPointer) &resources,
			      my_resources, XtNumber(my_resources),
			      NULL, (Cardinal) 0);

    if (resources.use_lights)
    {
	char	    name[1024];
	XrmString   type;
	XrmValue    db_value;
	XrmValue    int_value;
	Bool	    found = False;

	snprintf (name, sizeof(name), "%s.paned.load.update", XtName(toplevel));
	found = XrmGetResource (XtScreenDatabase(XtScreen(toplevel)),
				name, "XLoad.Paned.StripChart.Interval",
				&type, &db_value);
	if (found) {
	    int_value.size = sizeof(int);
	    int_value.addr = (XPointer) &light_update;
	    found = XtConvertAndStore(toplevel, type, &db_value, XtRInt,
				      &int_value);
	    if (found) light_update *= 1000;
	}
	ClearLights (XtDisplay (toplevel));
	SetLights ((XtPointer) toplevel, (XtIntervalId *) 0);
    }
    else
    {
    	/*
     	 * This is a hack so that f.delete will do something useful in this
     	 * single-window application.
     	 */
    	XtAppAddActions (app_con, xload_actions, XtNumber(xload_actions));
    	XtOverrideTranslations(toplevel,
		    	XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

    	XtSetArg (args[0], XtNiconPixmap, &icon_pixmap);
    	XtGetValues(toplevel, args, ONE);
    	if (icon_pixmap == None) {
	    XtSetArg(args[0], XtNiconPixmap,
		     XCreateBitmapFromData(XtDisplay(toplevel),
				       	   XtScreen(toplevel)->root,
				       	   (char *)xload_bits,
				       	   xload_width, xload_height));
	    XtSetValues (toplevel, args, ONE);
    	}

    	if (resources.show_label) {
      	  pane = XtCreateManagedWidget ("paned", panedWidgetClass,
				    	toplevel, NULL, ZERO);

      	  label_wid = XtCreateManagedWidget ("label", labelWidgetClass,
					     pane, NULL, ZERO);

      	  XtSetArg (args[0], XtNlabel, &label);
      	  XtGetValues(label_wid, args, ONE);

      	  if ( strcmp("label", label) == 0 ) {
	    (void) XmuGetHostname (host, 255);
	    XtSetArg (args[0], XtNlabel, host);
	    XtSetValues (label_wid, args, ONE);
      	  }

      	  load_parent = pane;
    	}
    	else
      	  load_parent = toplevel;

    	load = XtCreateManagedWidget ("load", stripChartWidgetClass,
				      load_parent, NULL, ZERO);

    	if (resources.remote)
	  XtAddCallback(load, XtNgetValue, GetRLoadPoint, NULL);
	else
	  XtAddCallback(load, XtNgetValue, GetLoadPoint, NULL);

    	XtRealizeWidget (toplevel);
    	wm_delete_window = XInternAtom (XtDisplay(toplevel), "WM_DELETE_WINDOW",
				    	False);
    	(void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
			    	&wm_delete_window, 1);
    }
    XtAppMainLoop(app_con);

    return 0;
}

static unsigned long	current_leds;

static void
ClearLights (Display *dpy)
{
    XKeyboardControl	cntrl = {
	.led_mode = LedModeOff
    };
    XChangeKeyboardControl (dpy, KBLedMode, &cntrl);
    current_leds = 0;
}

static void
SetLights (XtPointer data, XtIntervalId *timer)
{
    Widget		toplevel;
    Display		*dpy;
    double		value;
    unsigned long	new_leds, change, bit;
    int			i;

    toplevel = (Widget) data;
    dpy = XtDisplay (toplevel);
    if (resources.remote)
      GetRLoadPoint (toplevel, (XtPointer) 0, (XtPointer) &value);
    else
      GetLoadPoint (toplevel, (XtPointer) 0, (XtPointer) &value);
    new_leds = (1 << (int) (value + 0.1)) - 1;
    change = new_leds ^ current_leds;
    i = 1;
    bit = 1;
    while (current_leds != new_leds)
    {
	if (change & bit)
	{
	    XKeyboardControl	cntrl = {
		.led = i,
		.led_mode = new_leds & bit ? LedModeOn : LedModeOff
	    };
	    XChangeKeyboardControl (dpy, KBLed|KBLedMode, &cntrl);
	    current_leds ^= bit;
	}
	i++;
	bit <<= 1;
    }
    XtAppAddTimeOut(XtWidgetToApplicationContext(toplevel), light_update,
		    SetLights, data);
}

static void quit (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->type == ClientMessage &&
        event->xclient.data.l[0] != wm_delete_window) {
        XBell (XtDisplay(w), 0);
        return;
    }
    if (resources.use_lights)
	ClearLights (XtDisplay (w));
    XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
    exit (0);
}






