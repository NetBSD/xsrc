/*
 * $Xorg: xmore.c,v 1.1 2004/04/30 02:05:54 gisburn Exp $
 *
Copyright 2004 Roland Mainz <roland.mainz@nrubsig.org>

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
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Force ANSI C prototypes from X11 headers */
#ifndef FUNCPROTO 
#define FUNCPROTO 15
#endif /* !FUNCPROTO */

#include <X11/StringDefs.h> 
#include <X11/Intrinsic.h> 
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h> 
#include <X11/Xaw/Cardinals.h>

#include "xmore.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/* Global vars */
static Widget        toplevel          = NULL;
static Widget        text              = NULL;
static const char   *ProgramName;  /* program name   (from argv[0]) */
static const char   *viewFileName; /* file to browse (from argv[1]) */

/* prototypes */
static void quitAction(Widget w,  XEvent *event, String *params, Cardinal *num_params);
static void quitXtProc(Widget w, XtPointer client_data, XtPointer callData);

static XrmOptionDescRec options[] = {
{
  "-v", "*verbose", XrmoptionNoArg, (XPointer) "on" },
};


static XtActionsRec actions[] = {
    { "quit",          quitAction      },
};

/* See xmore.h */
static XMoreResourceData userOptions;

#define Offset(field) XtOffsetOf(XMoreResourceData, field)

static XtResource resources[] = {
  {"verbose", "Verbose", XtRBoolean, sizeof(Boolean),  Offset(verbose),  XtRImmediate, (XtPointer)False},
  {"textfont", XtCFont,  XtRFontSet, sizeof(XFontSet), Offset(textfont), XtRString,    STANDARDFONT},
};


static String fallback_resources[] = {
#ifdef NOTYET
    "*iconPixmap:    xmore32",
    "*iconMask:      xmore32",
#endif /* NOTYET */
    "*textfont: " STANDARDFONT,
    "*international: True", /* set this globally for ALL widgets to avoid wierd crashes */
    "*text.Translations: #override \\n\\"
        "\tCtrl<Key>S:     no-op(RingBell)\\n\\"
        "\tCtrl<Key>R:     no-op(RingBell)\\n\\"
        "\t<Key>space:     next-page()\\n\\"
        "\t<Key>F:         next-page()\\n\\"
        "\tCtrl<Key>B:     previous-page()\\n\\"
        "\t<Key>B:         previous-page()\\n\\"
        "\t<Key>K:         scroll-one-line-down()\\n\\"
        "\t<Key>Y:         scroll-one-line-down()\\n\\"
        "\t<Key>Return:    scroll-one-line-up()\\n\\"
        "\t<Key>J:         scroll-one-line-up()\\n\\"
        "\t<Key>E:         scroll-one-line-up()\\n\\"
        "\t<Key>q:         quit()\\n",
    "*text.baseTranslations: #override \\n\\"
        "\t<Key>space:     next-page()\\n\\"
        "\t<Key>F:         next-page()\\n\\"
        "\tCtrl<Key>B:     previous-page()\\n\\"
        "\t<Key>K:         scroll-one-line-down()\\n\\"
        "\t<Key>Y:         scroll-one-line-down()\\n\\"
        "\t<Key>Return:    scroll-one-line-up()\\n\\"
        "\t<Key>J:         scroll-one-line-up()\\n\\"
        "\t<Key>E:         scroll-one-line-up()\\n\\"
        "\t<Key>q:         quit()\\n",
    NULL,
};

static void
quitAction(Widget w, _X_UNUSED XEvent *event,
           _X_UNUSED String *params, _X_UNUSED Cardinal *num_params)
{
    XtAppSetExitFlag(XtWidgetToApplicationContext(w));
}

static void
quitXtProc(Widget w, _X_UNUSED XtPointer client_data,
           _X_UNUSED XtPointer callData)
{
    XtCallActionProc(w, "quit", NULL, NULL, 0);
}

_X_NORETURN _X_COLD
static void
usage(FILE *out, int exitval)
{
  fprintf(out,
          "usage: %s [ x options ] [-help|-version] filename\n",
          ProgramName);
  exit(exitval);
}

int main( int argc, char *argv[] )
{
  XtAppContext app;
  Widget       form;
  Widget       quitbutton;
  int          n;
  Arg          args[8];

  ProgramName = argv[0];

  /* Handle args that don't require opening a display */
  for (int i = 1; i < argc; i++) {
    const char *argn = argv[i];
    /* accept single or double dash for -help & -version */
    if (argn[0] == '-' && argn[1] == '-') {
      argn++;
    }
    if (strcmp (argn, "-help") == 0) {
      usage(stdout, EXIT_SUCCESS);
    }
    if (strcmp (argn, "-version") == 0) {
      puts(PACKAGE_STRING);
      exit(EXIT_SUCCESS);
    }
  }

  XtSetLanguageProc(NULL, NULL, NULL);
  toplevel = XtOpenApplication(&app, "XMore",
                               options, XtNumber(options), 
                               &argc, argv, fallback_resources,
                               sessionShellWidgetClass, NULL, ZERO);

  if (argc != 2)
  {
    fputs("Unknown argument(s):", stderr);
    for (int i = 1; i < argc; i++) {
      fprintf(stderr, " %s", argv[i]);
    }
    fputs("\n\n", stderr);
    usage(stderr, EXIT_FAILURE);
  }

  XtGetApplicationResources(toplevel, (XtPointer)&userOptions, resources, 
                            XtNumber(resources), NULL, 0);
 
  XtAppAddActions(app, actions, XtNumber(actions));

  viewFileName = argv[1];

  form = XtCreateManagedWidget("form", formWidgetClass, toplevel, NULL, 0);

  n = 0;
  XtSetArg(args[n], XtNtype,             XawAsciiFile);            n++;
  XtSetArg(args[n], XtNstring,           viewFileName);            n++;
  XtSetArg(args[n], XtNwidth,            700);                     n++;
  XtSetArg(args[n], XtNheight,           300);                     n++;
  XtSetArg(args[n], XtNscrollHorizontal, XawtextScrollAlways);     n++;
  XtSetArg(args[n], XtNscrollVertical,   XawtextScrollAlways);     n++;
  XtSetArg(args[n], XtNfontSet,          userOptions.textfont);    n++;
  text = XtCreateManagedWidget("text", asciiTextWidgetClass, form, args, n);

  n = 0;
  XtSetArg(args[n], XtNfromHoriz,       NULL);                   n++;
  XtSetArg(args[n], XtNfromVert,        text);                   n++;
  XtSetArg(args[n], XtNlabel,           "Quit");      n++;
  quitbutton = XtCreateManagedWidget("quit", commandWidgetClass, form, args, n);
  XtAddCallback(quitbutton, XtNcallback, quitXtProc, NULL);
  
  XtRealizeWidget(toplevel);
  
  XtAppMainLoop(app);

  return EXIT_SUCCESS;
}
         
