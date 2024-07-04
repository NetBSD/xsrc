/*

Copyright (c) 1991  X Consortium

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "Eyes.h"
#include <stdio.h>
#include <stdlib.h>
#include "eyes.bit"
#include "eyesmask.bit"

/* Exit with message describing command line format */

static void _X_NORETURN
usage(int exitval)
{
    fprintf(stderr,
	    "usage: xeyes [-display [{host}]:{vs}]\n"
	    "             [-geometry [{width}][x{height}][{+-}{xoff}[{+-}{yoff}]]]\n"
	    "             [-fg {color}] [-bg {color}] [-bd {color}] [-bw {pixels}]\n"
	    "             [-shape | +shape] [-outline {color}] [-center {color}]\n"
	    "             [-backing {backing-store}] [-distance]\n"
	    "             [-biblicallyAccurate]\n"
#ifdef XRENDER
	    "             [-render | +render]\n"
#endif
#ifdef PRESENT
	    "             [-present | +present]\n"
#endif
            "       xeyes -help\n"
            "       xeyes -version\n");
    exit(exitval);
}

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{(char *)"-outline",	(char *)"*eyes.outline",	XrmoptionSepArg,	NULL},
{(char *)"-center",	(char *)"*eyes.center",		XrmoptionSepArg,	NULL},
{(char *)"-backing",	(char *)"*eyes.backingStore",	XrmoptionSepArg,	NULL},
{(char *)"-shape",	(char *)"*eyes.shapeWindow",	XrmoptionNoArg,		(char *)"TRUE"},
{(char *)"+shape",	(char *)"*eyes.shapeWindow",	XrmoptionNoArg,		(char *)"FALSE"},
#ifdef XRENDER
{(char *)"-render",	(char *)"*eyes.render",		XrmoptionNoArg,		(char *)"TRUE"},
{(char *)"+render",	(char *)"*eyes.render",		XrmoptionNoArg,		(char *)"FALSE"},
#endif
#ifdef PRESENT
{(char *)"-present",	(char *)"*eyes.present",	XrmoptionNoArg,		(char *)"TRUE"},
{(char *)"+present",	(char *)"*eyes.present",	XrmoptionNoArg,		(char *)"FALSE"},
#endif
{(char *)"-distance",	(char *)"*eyes.distance",	XrmoptionNoArg,		(char *)"TRUE"},
{(char *)"-biblicallyAccurate",	(char *)"*eyes.biblicallyAccurate",	XrmoptionNoArg,		(char *)"TRUE"},
};

static Atom wm_delete_window;

/*ARGSUSED*/
static void
quit(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->type == ClientMessage &&
	event->xclient.data.l[0] != wm_delete_window) {
	XBell(XtDisplay(w), 0);
    } else {
	XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
	exit(0);
    }
}

static XtActionsRec actions[] = {
    {(char *) "quit",	quit}
};

int
main(int argc, char **argv)
{
    XtAppContext app_context;
    Widget toplevel;
    Arg arg[2];
    Cardinal i;

    XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

    /* Handle args that don't require opening a display */
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

    toplevel = XtAppInitialize(&app_context, "XEyes",
			       options, XtNumber(options), &argc, argv,
			       NULL, arg, (Cardinal) 0);

    if (argc != 1) {
	fputs("Unknown argument(s):", stderr);
	for (int n = 1; n < argc; n++) {
	    if ((n < (argc -1)) || (argv[n][0] == '-')) {
		fprintf(stderr, " %s", argv[n]);
	    }
	}
	fputs("\n\n", stderr);
	usage(1);
    }

    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    XtAppAddActions(app_context, actions, XtNumber(actions));
    XtOverrideTranslations
	(toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));

    i = 0;
    XtSetArg (arg[i], XtNiconPixmap,
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     (char *)eyes_bits, eyes_width, eyes_height));
    i++;
    XtSetArg (arg[i], XtNiconMask,
	      XCreateBitmapFromData (XtDisplay(toplevel),
				     XtScreen(toplevel)->root,
				     (char *)eyesmask_bits,
				     eyesmask_width, eyesmask_height));
    i++;
    XtSetValues (toplevel, arg, i);

    (void) XtCreateManagedWidget ("eyes", eyesWidgetClass, toplevel, NULL, 0);
    XtRealizeWidget (toplevel);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);
    XtAppMainLoop(app_context);

    return 0;
}
