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
/*
 * xditview -- 
 *
 *   Display ditroff output in an X window
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <X11/Xos.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/AsciiText.h>

#include "Dvi.h"

#include "xdit.bm"
#include "xdit_mask.bm"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XtParseCommand is let loose. */

static XrmOptionDescRec options[] = {
{"-page",	    "*dvi.pageNumber",	    XrmoptionSepArg,	NULL},
{"-backingStore",   "*dvi.backingStore",    XrmoptionSepArg,	NULL},
{"-noPolyText",	    "*dvi.noPolyText",	    XrmoptionNoArg,	"TRUE"},
{"-resolution",	    "*dvi.screenResolution",XrmoptionSepArg,    NULL},
};

static char	current_file_name[1024];
static FILE	*current_file;

static void MakePrompt(Widget, const char *, void (*)(char *), char *);

/*
 * Report the syntax for calling xditview.
 */

static void
Syntax(const char *call)
{
    (void) printf ("Usage: %s [-fg <color>] [-bg <color>]\n%s\n", call,
                   "       [-bd <color>] [-bw <pixels>] [-help]\n"
                   "       [-display displayname] [-geometry geom]\n"
                   "       [-page <page-number>] [-backing <backing-store>]\n"
                   "       [-resolution <screen-resolution>]\n");
    exit(1);
}

static void	NewResolution (char *resString);
static void	NewFile (char *name);
static void	DisplayPageNumber (void);
static void	VisitFile (char *name, Boolean resetPage);
static Widget	toplevel, paned, porthole, dvi;
static Widget	popupMenu;
static Widget	menuBar;
static Widget	fileMenuButton, fileMenu;
static Widget	pageNumber;

static void	NextPage(Widget entry, XtPointer name, XtPointer data);
static void	PreviousPage(Widget entry, XtPointer name, XtPointer data);
static void	SetResolution(Widget entry, XtPointer name, XtPointer data);
static void	OpenFile(Widget entry, XtPointer name, XtPointer data);
static void	RevisitFile(Widget entry, XtPointer name, XtPointer data);
static void	Quit(Widget entry, XtPointer closure, XtPointer data);

struct menuEntry {
    char    *name;
    void    (*function)(Widget entry, XtPointer name, XtPointer data);
};

static struct menuEntry popupMenuEntries[] = {
    { "nextPage",	    NextPage },
    { "previousPage",	    PreviousPage },
    { "setResolution",	    SetResolution },
    { "openFile",	    OpenFile },
    { "revisitFile",	    RevisitFile },
    { "quit",		    Quit }
};

static struct menuEntry fileMenuEntries[] = {
    { "openFile",	    OpenFile },
    { "revisitFile",	    RevisitFile },
    { "setResolution",	    SetResolution },
    { "quit",		    Quit }
};

static void	NextPageAction(Widget, XEvent *, String *, Cardinal *);
static void	PreviousPageAction(Widget, XEvent *, String *, Cardinal *);
static void	SetResolutionAction(Widget, XEvent *, String *, Cardinal *);
static void	OpenFileAction(Widget, XEvent *, String *, Cardinal *);
static void	RevisitFileAction(Widget, XEvent *, String *, Cardinal *);
static void	QuitAction(Widget, XEvent *, String *, Cardinal *);
static void	AcceptAction(Widget, XEvent *, String *, Cardinal *);
static void	CancelAction(Widget, XEvent *, String *, Cardinal *);
static void	UpdatePageNumber(Widget, XEvent *, String *, Cardinal *);
static void	Noop(Widget, XEvent *, String *, Cardinal *);

static XtActionsRec xditview_actions[] = {
    { "NextPage",	    NextPageAction },
    { "PreviousPage",	    PreviousPageAction },
    { "SetResolution",	    SetResolutionAction },
    { "OpenFile",	    OpenFileAction },
    { "Quit",		    QuitAction },
    { "Accept",		    AcceptAction },
    { "Cancel",		    CancelAction },
    { "SetPageNumber",	    UpdatePageNumber },
    { "Noop",		    Noop }
};

static Atom wm_delete_window;


int
main(int argc, char **argv)
{
    char	    *file_name = NULL;
    XtAppContext    xtcontext;
    Arg		    topLevelArgs[2];
    Widget          entry;

    XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

    toplevel = XtAppInitialize(&xtcontext, "Xditview",
			       options, XtNumber (options),
			       &argc, argv, NULL, NULL, 0);
    if (argc > 2)
	Syntax(argv[0]);

    XtAppAddActions(xtcontext, xditview_actions, XtNumber (xditview_actions));
    XtOverrideTranslations
	(toplevel, XtParseTranslationTable ("<Message>WM_PROTOCOLS: Quit()"));

    XtSetArg (topLevelArgs[0], XtNiconPixmap,
	      XCreateBitmapFromData (XtDisplay (toplevel),
				     XtScreen(toplevel)->root,
				     (char *) xdit_bits,
				     xdit_width, xdit_height));
				    
    XtSetArg (topLevelArgs[1], XtNiconMask,
	      XCreateBitmapFromData (XtDisplay (toplevel),
				     XtScreen(toplevel)->root,
				     (char *) xdit_mask_bits, 
				     xdit_mask_width, xdit_mask_height));
    XtSetValues (toplevel, topLevelArgs, 2);
    if (argc > 1)
	file_name = argv[1];

    /*
     * create the popup menu and insert the entries
     */
    popupMenu = XtCreatePopupShell ("popupMenu", simpleMenuWidgetClass, toplevel,
				    NULL, 0);
    for (Cardinal i = 0; i < XtNumber (popupMenuEntries); i++) {
	entry = XtCreateManagedWidget(popupMenuEntries[i].name, 
				      smeBSBObjectClass, popupMenu,
				      NULL, (Cardinal) 0);
	XtAddCallback(entry, XtNcallback, popupMenuEntries[i].function, NULL);
    }

    paned = XtCreateManagedWidget("paned", panedWidgetClass, toplevel,
				    NULL, (Cardinal) 0);
    menuBar = XtCreateManagedWidget ("menuBar", boxWidgetClass, paned, NULL, 0);

    fileMenuButton = XtCreateManagedWidget ("fileMenuButton", menuButtonWidgetClass,
				    menuBar, NULL, (Cardinal) 0);
    fileMenu = XtCreatePopupShell ("fileMenu", simpleMenuWidgetClass,
				    fileMenuButton, NULL, (Cardinal) 0);
    for (Cardinal i = 0; i < XtNumber (fileMenuEntries); i++) {
	entry = XtCreateManagedWidget(fileMenuEntries[i].name,
				      smeBSBObjectClass, fileMenu,
				      NULL, (Cardinal) 0);
	XtAddCallback (entry, XtNcallback, fileMenuEntries[i].function, NULL);
    }

    (void) XtCreateManagedWidget ("prevButton", commandWidgetClass,
				  menuBar, NULL, (Cardinal) 0);

    pageNumber = XtCreateManagedWidget("pageNumber", asciiTextWidgetClass,
					menuBar, NULL, (Cardinal) 0);
  
    (void) XtCreateManagedWidget ("nextButton", commandWidgetClass,
				  menuBar, NULL, (Cardinal) 0);

    porthole = XtCreateManagedWidget ("viewport", viewportWidgetClass,
				      paned, NULL, 0);
    dvi = XtCreateManagedWidget ("dvi", dviWidgetClass, porthole, NULL, 0);
    if (file_name)
	VisitFile (file_name, FALSE);
    XtRealizeWidget (toplevel);
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(toplevel), XtWindow(toplevel),
                            &wm_delete_window, 1);
    XtAppMainLoop(xtcontext);

    return 0;
}

static void
DisplayPageNumber (void)
{
    Arg	arg[2];
    int	actual_number, last_page;
    XawTextBlock    text;
    int		    length;
    char	    value[128];
    char	    *cur;

    XtSetArg (arg[0], XtNpageNumber, &actual_number);
    XtSetArg (arg[1], XtNlastPageNumber, &last_page);
    XtGetValues (dvi, arg, 2);
    if (actual_number == 0)
	snprintf (value, sizeof(value), "<none>");
    else if (last_page > 0)
	snprintf (value, sizeof(value), "%d of %d", actual_number, last_page);
    else
	snprintf (value, sizeof(value), "%d", actual_number);
    text.firstPos = 0;
    text.length = strlen (value);
    text.ptr = value;
    text.format = FMT8BIT;
    XtSetArg (arg[0], XtNstring, &cur);
    XtGetValues (XawTextGetSource (pageNumber), arg, 1);
    length = strlen (cur);
    XawTextReplace (pageNumber, 0, length, &text);
}

static void
SetPageNumber (int number)
{
    Arg	arg[1];

    XtSetArg (arg[0], XtNpageNumber, number);
    XtSetValues (dvi, arg, 1);
    DisplayPageNumber ();
}

static void
UpdatePageNumber (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    char    *string;
    Arg	    arg[1];

    XtSetArg (arg[0], XtNstring, &string);
    XtGetValues (XawTextGetSource(pageNumber), arg, 1);
    SetPageNumber (atoi(string));
}

static void
NewResolution(char *resString)
{
    int	res;
    Arg	arg[1];
    
    res = atoi (resString);
    if (res <= 0)
	return;
    XtSetArg (arg[0], XtNscreenResolution, res);
    XtSetValues (dvi, arg, 1);
}

static void
VisitFile (char *name, Boolean resetPage)
{
    Arg	    arg[3];
    char    *n;
    FILE    *new_file;
    Boolean seek = 0;
    int	    i;

    if (current_file) {
	if (!strcmp (current_file_name, "-"))
	    ;
	else if (current_file_name[0] == '|')
	    pclose (current_file);
	else
	    fclose (current_file);
    }
    if (!strcmp (name, "-"))
	new_file = stdin;
    else if (name[0] == '|')
	new_file = popen (name+1, "r");
    else {
        struct stat stbuf;

	new_file = fopen (name, "r");

        if (!new_file) {
            perror(name);
            return;
        }
        /* Make sure it is a regular file */
        if (fstat(fileno(new_file), &stbuf) != 0) {
            perror(name);
            fclose(new_file);
            return;
        }
        if (! S_ISREG(stbuf.st_mode)){
            fprintf(stderr, "%s is not a regular file.\n", name);
            fclose(new_file);
            return;
        }

	seek = 1;
    }
    if (!new_file) {
	/* XXX display error message */
	return;
    }
    i = 0;
    XtSetArg (arg[i], XtNfile, new_file); i++;
    XtSetArg (arg[i], XtNseek, seek); i++;
    if (resetPage) {
	XtSetArg (arg[i], XtNpageNumber, 1); i++;
    }
    XtSetValues (dvi, arg, i);
    XtSetArg (arg[0], XtNtitle, name);
    if (name[0] != '/' && (n = strrchr (name, '/')))
	n = n + 1;
    else
	n = name;
    XtSetArg (arg[1], XtNiconName, n);
    XtSetValues (toplevel, arg, 2);
    strcpy (current_file_name, name);
    current_file = new_file;
    DisplayPageNumber ();
}

static void
NewFile (char *name)
{
    VisitFile (name, TRUE);
}

static char fileBuf[1024];
static char resolutionBuf[1024];

static void
ResetMenuEntry (Widget entry)
{
    Arg	arg[1];

    XtSetArg (arg[0], XtNpopupOnEntry, entry);
    XtSetValues (XtParent(entry) , arg, (Cardinal) 1);
}

/*ARGSUSED*/
static void
NextPage (Widget entry, XtPointer name, XtPointer data)
{
    NextPageAction(entry, NULL, NULL, NULL);
    ResetMenuEntry (entry);
}

static void
NextPageAction (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    Arg	args[1];
    int	number;

    XtSetArg (args[0], XtNpageNumber, &number);
    XtGetValues (dvi, args, 1);
    SetPageNumber (number+1);
}

/*ARGSUSED*/
static void
PreviousPage (Widget entry, XtPointer name, XtPointer data)
{
    PreviousPageAction (entry, NULL, NULL, NULL);
    ResetMenuEntry (entry);
}

static void
PreviousPageAction (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    Arg	args[1];
    int	number;

    XtSetArg (args[0], XtNpageNumber, &number);
    XtGetValues (dvi, args, 1);
    SetPageNumber (number-1);
}

/*ARGSUSED*/
static void
SetResolution (Widget entry, XtPointer name, XtPointer data)
{
    SetResolutionAction (entry, NULL, NULL, NULL);
    ResetMenuEntry (entry);
}

static void
SetResolutionAction (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    Arg	    args[1];
    int	    cur;

    XtSetArg (args[0], XtNscreenResolution, &cur);
    XtGetValues (dvi, args, 1);
    snprintf (resolutionBuf, sizeof(resolutionBuf), "%d", cur);
    MakePrompt (toplevel, "Screen resolution:", NewResolution, resolutionBuf);
}

/*ARGSUSED*/
static void
OpenFile (Widget entry, XtPointer name, XtPointer data)
{
    OpenFileAction (entry, NULL, NULL, NULL);
    ResetMenuEntry (entry);
}

static void
OpenFileAction (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    if (current_file_name[0])
	strcpy (fileBuf, current_file_name);
    else
	fileBuf[0] = '\0';
    MakePrompt (toplevel, "File to open:", NewFile, fileBuf);
}

/*ARGSUSED*/
static void
RevisitFile (Widget entry, XtPointer name, XtPointer data)
{
    RevisitFileAction (entry, NULL, NULL, NULL);
    ResetMenuEntry (entry);
}

static void
RevisitFileAction (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    if (current_file_name[0])
	VisitFile (current_file_name, FALSE);
}

/*ARGSUSED*/
static void
Quit (Widget entry, XtPointer closure, XtPointer data)
{
    QuitAction (entry, NULL, NULL, NULL);
}

static void
QuitAction (Widget w, XEvent *xev, String *s, Cardinal *c)
{
    exit (0);
}

static Widget promptShell, promptDialog;
static void (*promptfunction)(char *);

/* ARGSUSED */
static
void CancelAction (Widget widget, XEvent *event,
		   String *params, Cardinal *num_params)
{
    if (promptShell) {
	XtSetKeyboardFocus(toplevel, (Widget) None);
	XtDestroyWidget(promptShell);
	promptShell = (Widget) 0;
    }
}


/* ARGSUSED */
static
void AcceptAction (Widget widget, XEvent *event,
		   String *params, Cardinal *num_params)
{
    (*promptfunction)(XawDialogGetValueString(promptDialog));
    CancelAction (widget, event, params, num_params);
}

static
void Noop (Widget w, XEvent *xev, String *s, Cardinal *c)
{
}

static void
MakePrompt(Widget centerw, const char *prompt, void (*func)(char *), char *def)
{
    static Arg dialogArgs[] = {
	{XtNlabel, (XtArgVal) 0},
	{XtNvalue, (XtArgVal) 0},
    };
    Arg valueArgs[1];
    Arg centerArgs[2];
    Position	source_x, source_y;
    Position	dest_x, dest_y;
    Dimension center_width, center_height;
    Dimension prompt_width, prompt_height;
    Widget  valueWidget;
    
    CancelAction ((Widget)NULL, (XEvent *) 0, (String *) 0, (Cardinal *) 0);
    promptShell = XtCreatePopupShell ("promptShell", transientShellWidgetClass,
				      toplevel, NULL, (Cardinal) 0);
    dialogArgs[0].value = (XtArgVal)prompt;
    dialogArgs[1].value = (XtArgVal)def;
    promptDialog = XtCreateManagedWidget( "promptDialog", dialogWidgetClass,
		    promptShell, dialogArgs, XtNumber (dialogArgs));
    XawDialogAddButton(promptDialog, "accept", NULL, NULL);
    XawDialogAddButton(promptDialog, "cancel", NULL, NULL);
    valueWidget = XtNameToWidget (promptDialog, "value");
    if (valueWidget) {
    	XtSetArg (valueArgs[0], XtNresizable, TRUE);
    	XtSetValues (valueWidget, valueArgs, 1);
	/*
	 * as resizable isn't set until just above, the
	 * default value will be displayed incorrectly.
	 * rectify the situation by resetting the values
	 */
        XtSetValues (promptDialog, dialogArgs, XtNumber (dialogArgs));
    }
    XtSetKeyboardFocus (promptDialog, valueWidget);
    XtSetKeyboardFocus (toplevel, valueWidget);
    XtRealizeWidget (promptShell);
    /*
     * place the widget in the center of the "parent"
     */
    XtSetArg (centerArgs[0], XtNwidth, &center_width);
    XtSetArg (centerArgs[1], XtNheight, &center_height);
    XtGetValues (centerw, centerArgs, 2);
    XtSetArg (centerArgs[0], XtNwidth, &prompt_width);
    XtSetArg (centerArgs[1], XtNheight, &prompt_height);
    XtGetValues (promptShell, centerArgs, 2);
    source_x = (int)(center_width - prompt_width) / 2;
    source_y = (int)(center_height - prompt_height) / 3;
    XtTranslateCoords (centerw, source_x, source_y, &dest_x, &dest_y);
    XtSetArg (centerArgs[0], XtNx, dest_x);
    XtSetArg (centerArgs[1], XtNy, dest_y);
    XtSetValues (promptShell, centerArgs, 2);
    XtMapWidget(promptShell);
    promptfunction = func;
}
