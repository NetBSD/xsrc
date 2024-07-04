/*
 *
Copyright 1990, 1998  The Open Group

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
 * Author:  Keith Packard, MIT X Consortium
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xfuncproto.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>
#include <X11/Xmu/SysUtil.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>

extern char *_XawTextGetSTRING(TextWidget ctx, XawTextPosition left,
			       XawTextPosition right);

#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <sys/stat.h>
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#undef _POSIX_SOURCE
#else
#include <stdio.h>
#endif
#include <X11/Shell.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef HAS_OPENPTY
# ifdef HAVE_UTIL_H
#  include <util.h>
# endif
# ifdef HAVE_LIBUTIL_H
#  include <libutil.h>
# endif
# ifdef HAVE_PTY_H
#  include <pty.h>
# endif
#endif

static void inputReady(XtPointer w, int *source, XtInputId *id);
static long TextLength(Widget w);
static void TextReplace(Widget w, int start, int end, XawTextBlock *block);
static void TextAppend(Widget w, char *s, int len);
static void TextInsert(Widget w, char *s, int len);
static Bool ExceededMaxLines(Widget w);
static void ScrollLine(Widget w);

static Widget		top, text;

static XtInputId	input_id;

static FILE		*input;
static Boolean		regularFile = FALSE;

static Boolean		notified;
static Boolean		iconified;

static Atom		wm_delete_window;
static Atom		mit_console;

#define MIT_CONSOLE_LEN 12
#define MIT_CONSOLE "MIT_CONSOLE_"
static char		mit_console_name[255 + MIT_CONSOLE_LEN + 1] = MIT_CONSOLE;

static struct _app_resources {
    char    *file;
    Boolean stripNonprint;
    Boolean notify;
    Boolean daemon;
    Boolean verbose;
    Boolean exitOnFail;
    int     saveLines;
} app_resources;

#define Offset(field) XtOffsetOf(struct _app_resources, field)

static XtResource  resources[] = {
    {"file",	"File",	    XtRString,	sizeof (char *),
	Offset (file),	XtRString,  "console" },
    {"notify",	"Notify",   XtRBoolean,	sizeof (Boolean),
	Offset (notify), XtRImmediate, (XtPointer)True },
    {"stripNonprint",	"StripNonprint",    XtRBoolean, sizeof (Boolean),
	Offset (stripNonprint), XtRImmediate, (XtPointer)True },
    {"daemon",		"Daemon",	    XtRBoolean,	sizeof (Boolean),
	Offset (daemon), XtRImmediate, (XtPointer)False},
    {"verbose",		"Verbose",	    XtRBoolean,	sizeof (Boolean),
	Offset (verbose),XtRImmediate, (XtPointer)False},
    {"exitOnFail",	"ExitOnFail",    XtRBoolean,	sizeof (Boolean),
	Offset (exitOnFail),XtRImmediate, (XtPointer)False},
    {"saveLines",	"SaveLines",	XtRInt,	sizeof (int),
	Offset (saveLines), XtRImmediate, (XtPointer) 0 },
};

#undef Offset

static XrmOptionDescRec options[] = {
    {"-file",		"*file",		XrmoptionSepArg,	NULL},
    {"-notify",		"*notify",		XrmoptionNoArg,		"TRUE"},
    {"-nonotify",	"*notify",		XrmoptionNoArg,		"FALSE"},
    {"-daemon",		"*daemon",		XrmoptionNoArg,		"TRUE"},
    {"-verbose",	"*verbose",		XrmoptionNoArg,		"TRUE"},
    {"-exitOnFail",	"*exitOnFail",		XrmoptionNoArg,		"TRUE"},
    {"-saveLines",	"*saveLines",		XrmoptionSepArg,	NULL},
};


#ifdef linux
#define USE_FILE
#define FILE_NAME	"/dev/xconsole"
# if defined (__GLIBC__) && ((__GLIBC__ > 2) || (__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1))
/*
 * Linux distribution based on glibc 2.1 and higher should use
 * devpts. This is the fallback if open file FILE_NAME fails.
 * <werner@suse.de>
 */
#  define USE_PTS
# endif
#endif

#if !defined (USE_FILE) || defined (linux)
# include <sys/ioctl.h>
# if defined (SVR4) || defined (USE_PTS)
#  include <termios.h>
#  ifndef HAS_OPENPTY
#  include <sys/stropts.h>		/* for I_PUSH */
#  endif
#  ifdef sun
#   include <sys/strredir.h>
#  endif
# endif
# if defined(TIOCCONS) || defined(SRIOCSREDIR)
#  define USE_PTY
static int  tty_fd, pty_fd;
# endif
#endif

#ifdef USE_PTY
static int get_pty(int *pty, int *tty);
#endif

/* Copied from xterm/ptyx.h */
#ifndef PTYCHAR1
#define PTYCHAR1        "pqrstuvwxyzPQRSTUVWXYZ"
#endif  /* !PTYCHAR1 */

#ifndef PTYCHAR2
#ifdef __FreeBSD__
#define PTYCHAR2        "0123456789abcdefghijklmnopqrstuv"
#else /* !__FreeBSD__ */
#define PTYCHAR2        "0123456789abcdef"
#endif /* !__FreeBSD__ */
#endif  /* !PTYCHAR2 */

static void
OpenConsole(void)
{
    input = 0;
    if (app_resources.file)
    {
	if (!strcmp (app_resources.file, "console"))
	{
	    /* must be owner and have read/write permission */
#if !defined(__NetBSD__) && !defined(__OpenBSD__)
	    struct stat sbuf;
# if !defined (linux)
	    if (!stat("/dev/console", &sbuf) &&
		(sbuf.st_uid == getuid()) &&
		!access("/dev/console", R_OK|W_OK))
# endif
#endif
	    {
#ifdef USE_FILE
# ifdef linux
		if (!stat(FILE_NAME, &sbuf))
# endif
		    input = fopen (FILE_NAME, "r");
#endif

#ifdef USE_PTY
		if (!input && get_pty (&pty_fd, &tty_fd) == 0)
		{
# ifdef TIOCCONS
		    int on = 1;
		    if (ioctl (tty_fd, TIOCCONS, (char *) &on) != -1)
			input = fdopen (pty_fd, "r");
# else
		    int consfd = open("/dev/console", O_RDONLY);
		    if (consfd >= 0)
		    {
			if (ioctl(consfd, SRIOCSREDIR, tty_fd) != -1)
			    input = fdopen (pty_fd, "r");
			close(consfd);
		    }
# endif
		}
#endif /* USE_PTY */
	    }
	    if (input && app_resources.verbose)
	    {
		char	*hostname;
		TextAppend (text, "Console log for ", 16);
		hostname = mit_console_name + MIT_CONSOLE_LEN;
		TextAppend (text, hostname, strlen (hostname));
		TextAppend (text, "\n", 1);
	    }
	}
	else
	{
	    regularFile = FALSE;
	    if (access(app_resources.file, R_OK) == 0)
	    {
		int fd  = open (app_resources.file,
				O_RDONLY | O_NONBLOCK | O_NOCTTY);
		if (fd != -1) {
		    input = fdopen (fd, "r");

		    if (input) {
			struct stat sbuf;

			if ((fstat(fd, &sbuf) == 0) && S_ISREG(sbuf.st_mode))
			    regularFile = TRUE;
		    }
		    else
			close(fd);
		}
	    }
	}
	if (!input)
	{
	    if (app_resources.exitOnFail)
		exit(0);
	    TextAppend (text, "Couldn't open ", 14);
	    TextAppend (text, app_resources.file, strlen (app_resources.file));
	    TextAppend (text, "\n", 1);
	}
    }
    else
	input = stdin;

    if (input)
    {
	input_id = XtAddInput (fileno (input), (XtPointer) XtInputReadMask,
			       inputReady, (XtPointer) text);
    }
}

static void
CloseConsole (void)
{
    if (input)
    {
	XtRemoveInput (input_id);
	fclose (input);
    }
#ifdef USE_PTY
    close (tty_fd);
#endif
}

/*ARGSUSED*/
static void _X_NORETURN
Quit(Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
    exit (0);
}

static void
Notify(void)
{
    Arg	    arglist[1];
    char    *oldName;
    char    *newName;

    if (!iconified || !app_resources.notify || notified)
	return;
    XtSetArg (arglist[0], XtNiconName, &oldName);
    XtGetValues (top, arglist, 1);
    newName = malloc (strlen (oldName) + 3);
    if (!newName)
	return;
    sprintf (newName, "%s *", oldName);
    XtSetArg (arglist[0], XtNiconName, newName);
    XtSetValues (top, arglist, 1);
    free (newName);
    notified = True;
}

/*ARGSUSED*/
static void
Deiconified(Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
    Arg	    arglist[1];
    char    *oldName;
    char    *newName;
    size_t  oldlen;

    iconified = False;
    if (!app_resources.notify || !notified)
	return;
    XtSetArg (arglist[0], XtNiconName, &oldName);
    XtGetValues (top, arglist, 1);
    oldlen = strlen (oldName);
    if (oldlen >= 2)
    {
	newName = malloc (oldlen - 1);
	if (!newName)
	    return;
	strncpy (newName, oldName, oldlen - 2);
	newName[oldlen - 2] = '\0';
	XtSetArg (arglist[0], XtNiconName, newName);
	XtSetValues (top, arglist, 1);
	free (newName);
    }
    notified = False;
}

/*ARGSUSED*/
static void
Iconified(Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
    iconified = True;
}

/*ARGSUSED*/
static void
Clear(Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
    long	    last;
    XawTextBlock    block;

    last = TextLength (text);
    block.ptr = "";
    block.firstPos = 0;
    block.length = 0;
    block.format = FMT8BIT;
    TextReplace (text, 0, last, &block);
}

static XtActionsRec actions[] = {
    { "Quit",		Quit },
    { "Iconified",	Iconified },
    { "Deiconified",	Deiconified },
    { "Clear",		Clear },
};

static void
stripNonprint(char *b)
{
    char    *c;

    c = b;
    while (*b)
    {
	if (isprint (*b) || (isspace (*b) && *b != '\r'))
	{
	    if (c != b)
		*c = *b;
	    ++c;
	}
	++b;
    }
    *c = '\0';
}

static void
inputReady(XtPointer w, int *source, XtInputId *id)
{
    char    buffer[1025];
    int	    n;

    n = read (*source, buffer, sizeof (buffer) - 1);
    if (n <= 0)
    {
	if (app_resources.file && regularFile && n == 0)
	{
	    if (XPending(XtDisplay(w)))
		return;

	    sleep(1);
	    return;
	}

	fclose (input);
	XtRemoveInput (*id);

	/* try to reopen if pipe; this can be caused by syslog restart */
	if (app_resources.file && !regularFile && n == 0)
	{
	    OpenConsole();
	}
    } else {
	Notify();
	buffer[n] = '\0';
	if (app_resources.stripNonprint)
	{
	    stripNonprint (buffer);
	    n = strlen (buffer);
	}

	TextAppend ((Widget) text, buffer, n);
    }
}

static Boolean
ConvertSelection(Widget w, Atom *selection, Atom *target, Atom *type,
		 XtPointer *value, unsigned long *length, int *format)
{
    Display* d = XtDisplay(w);
    XSelectionRequestEvent* req =
	XtGetSelectionRequest(w, *selection, (XtRequestId)NULL);

    if (*target == XA_TARGETS(d))
    {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, req->time, selection, target, type,
				    (XPointer *)&std_targets, &std_length,
				    format);
	*value = (XtPointer)XtMalloc(sizeof(Atom)*(std_length + 5));
	targetP = *(Atom**)value;
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
	*targetP++ = XA_CHARACTER_POSITION(d);
	*length = std_length + (targetP - (*(Atom **) value));
	memmove( (char*)targetP, (char*)std_targets, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

    if (*target == XA_LIST_LENGTH(d) ||
	*target == XA_LENGTH(d))
    {
	long * temp;

	temp = (long *) XtMalloc(sizeof(long));
	if (*target == XA_LIST_LENGTH(d))
	  *temp = 1L;
	else			/* *target == XA_LENGTH(d) */
	  *temp = (long) TextLength (text);

	*value = (XtPointer) temp;
	*type = XA_INTEGER;
	*length = 1L;
	*format = 32;
	return True;
    }

    if (*target == XA_CHARACTER_POSITION(d))
    {
	long * temp;

	temp = (long *) XtMalloc(2 * sizeof(long));
	temp[0] = (long) 0;
	temp[1] = TextLength (text);
	*value = (XtPointer) temp;
	*type = XA_SPAN(d);
	*length = 2L;
	*format = 32;
	return True;
    }

    if (*target == XA_STRING ||
      *target == XA_TEXT(d) ||
      *target == XA_COMPOUND_TEXT(d))
    {
	if (*target == XA_COMPOUND_TEXT(d))
	    *type = *target;
	else
	    *type = XA_STRING;
	*length = TextLength (text);
	*value = (XtPointer)_XawTextGetSTRING((TextWidget) text, 0, *length);
	*format = 8;
	/*
	 * Drop our connection to the file; the new console program
	 * will open as soon as it receives the selection contents; there
	 * is a small window where console output will not be redirected,
	 * but I see no way of avoiding that without having two programs
	 * attempt to redirect console output at the same time, which seems
	 * worse
	 */
	CloseConsole ();
	return True;
    }

    if (XmuConvertStandardSelection(w, req->time, selection, target, type,
				    (XPointer *)value, length, format))
	return True;

    return False;
}

static void _X_NORETURN
LoseSelection(Widget w, Atom *selection)
{
    Quit (w, (XEvent*)NULL, (String*)NULL, (Cardinal*)NULL);
}

/*ARGSUSED*/
static void
InsertSelection(Widget w, XtPointer client_data, Atom *selection, Atom *type,
		XtPointer value, unsigned long *length, int *format)
{
    if (*type != XT_CONVERT_FAIL)
	TextInsert (text, (char *) value, *length);
    XtOwnSelection(top, mit_console, CurrentTime,
		   ConvertSelection, LoseSelection, NULL);
    OpenConsole ();
}

static void _X_NORETURN
usage(int exitval)
{
    const char *usage_message =
    "usage: xconsole [-toolkitoption  ...] [-file file-name] [-notify|-nonotify]\n"
    "                [-stripNonprint] [-daemon] [-verbose] [-exitOnFail]\n"
    "                [-saveLines count]\n"
    "       xconsole -help\n"
    "       xconsole -version\n";

    fputs(usage_message, stderr);
    exit(exitval);
}

int
main(int argc, char *argv[])
{
    Arg arglist[10];
    Cardinal num_args;

    XtSetLanguageProc(NULL,NULL,NULL);

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

    top = XtInitialize ("xconsole", "XConsole", options, XtNumber (options),
			&argc, argv);
    if (argc != 1) {
        fputs("Unknown argument(s):", stderr);
        for (int n = 1; n < argc; n++) {
            fprintf(stderr, " %s", argv[n]);
        }
        fputs("\n\n", stderr);
        usage(1);
    }

    XtGetApplicationResources (top, (XtPointer)&app_resources, resources,
			       XtNumber (resources), NULL, 0);

    if (app_resources.daemon)
	if (fork ()) exit (0);
    XtAddActions (actions, XtNumber (actions));

    text = XtCreateManagedWidget ("text", asciiTextWidgetClass,
				  top, NULL, 0);

    XtRealizeWidget (top);
    num_args = 0;
    XtSetArg(arglist[num_args], XtNiconic, &iconified); num_args++;
    XtGetValues(top, arglist, num_args);
    if (iconified)
	Iconified((Widget)NULL, (XEvent*)NULL, (String*)NULL, (Cardinal*)NULL);
    else
	Deiconified((Widget)NULL,(XEvent*)NULL,(String*)NULL,(Cardinal*)NULL);
    wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(top), XtWindow(top),
                            &wm_delete_window, 1);

    XmuGetHostname (mit_console_name + MIT_CONSOLE_LEN, 255);

    mit_console = XInternAtom(XtDisplay(top), mit_console_name, False);

    if (XGetSelectionOwner (XtDisplay (top), mit_console))
    {
	XtGetSelectionValue(top, mit_console, XA_STRING, InsertSelection,
			    NULL, CurrentTime);
    }
    else
    {
	XtOwnSelection(top, mit_console, CurrentTime,
		       ConvertSelection, LoseSelection, NULL);
	OpenConsole ();
    }
    XtMainLoop ();
    return 0;
}

static long
TextLength(Widget w)
{
    return XawTextSourceScan (XawTextGetSource (w),
			      (XawTextPosition) 0,
			      XawstAll, XawsdRight, 1, TRUE);
}

static void
TextReplace(Widget w, int start, int end, XawTextBlock *block)
{
    Arg		    arg;
    Widget	    source;
    XawTextEditType edit_mode;

    source = XawTextGetSource (w);
    XtSetArg (arg, XtNeditType, &edit_mode);
    XtGetValues (source, &arg, ONE);
    XtSetArg (arg, XtNeditType, XawtextEdit);
    XtSetValues (source, &arg, ONE);
    XawTextReplace (w, start, end, block);
    XtSetArg (arg, XtNeditType, edit_mode);
    XtSetValues (source, &arg, ONE);
}

static void
TextAppend(Widget w, char *s, int len)
{
    long	    last, current;
    XawTextBlock    block;

    current = XawTextGetInsertionPoint (w);
    last = TextLength (w);
    block.ptr = s;
    block.firstPos = 0;
    block.length = len;
    block.format = FMT8BIT;
    /*
     * If saveLines is 1, just replace the entire contents of the widget
     * each time, so the test in ExceededMaxLines() isn't fooled.
     */
    if (app_resources.saveLines == 1)
	TextReplace (w, 0, last, &block);
    else
	TextReplace (w, last, last, &block);
    if (current == last)
	XawTextSetInsertionPoint (w, last + block.length);
    if (ExceededMaxLines(w))
	ScrollLine(w);
}

static void
TextInsert(Widget w, char *s, int len)
{
    XawTextBlock    block;
    long	    current;

    current = XawTextGetInsertionPoint (w);
    block.ptr = s;
    block.firstPos = 0;
    block.length = len;
    block.format = FMT8BIT;
    TextReplace (w, 0, 0, &block);
    if (current == 0)
	XawTextSetInsertionPoint (w, len);
    if (ExceededMaxLines(w))
	ScrollLine(w);
}

static Bool
ExceededMaxLines(Widget w)
{
    XawTextPosition end_of_last_line;
    Bool retval = False;

    if (app_resources.saveLines > 0)
    {
    /*
     * XawTextSourceScan() will return the end of the widget if it cannot
     * find what it is searching for.
     */
	end_of_last_line = XawTextSourceScan (XawTextGetSource (w),
					      (XawTextPosition) 0,
					      XawstEOL, XawsdRight,
					      app_resources.saveLines, TRUE);
	if (TextLength(w) > end_of_last_line)
	    retval = True;
	else
	    retval = False;
    }
    else
	retval = False;
    return retval;
}

static void
ScrollLine(Widget w)
{
    XawTextPosition firstnewline;
    XawTextBlock    block;

    /*
     * This is pretty inefficient but should work well enough unless the
     * console device is getting totally spammed.  Generally, new lines
     * only come in one at a time anyway.
     */
    firstnewline = XawTextSourceScan (XawTextGetSource (w),
				      (XawTextPosition) 0,
				      XawstEOL, XawsdRight, 1, TRUE);
    block.ptr = "";
    block.firstPos = 0;
    block.length = 0;
    block.format = FMT8BIT;
    TextReplace (w, 0, firstnewline, &block);
}

#ifdef USE_PTY
/*
 * This function opens up a pty master and stuffs its value into pty.
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */

static int
get_pty(int *pty, int *tty)
{
#ifdef HAS_OPENPTY
	if (openpty(pty, tty, NULL, NULL, NULL) == -1) {
		return 1;
	}
	return 0;
#else
	static char ttydev[64], ptydev[64];

#if defined (SVR4) || defined (USE_PTS)
	if ((*pty = open ("/dev/ptmx", O_RDWR)) < 0)
	    return 1;
	grantpt(*pty);
	unlockpt(*pty);
	strcpy(ttydev, (char *)ptsname(*pty));
	if ((*tty = open(ttydev, O_RDWR)) >= 0)
	{
	    (void)ioctl(*tty, I_PUSH, "ttcompat");
	    return 0;
	}
	if (*pty >= 0)
	    close (*pty);
#else /* !SVR4, need lots of code */
#ifdef USE_GET_PSEUDOTTY
	if ((*pty = getpseudotty (&ttydev, &ptydev)) >= 0 &&
	    (*tty = open (ttydev, O_RDWR)) >= 0)
	    return 0;
	if (*pty >= 0)
	    close (*pty);
#else
	static int devindex, letter = 0;

	strcpy (ttydev, "/dev/ttyxx");
	strcpy (ptydev, "/dev/ptyxx");
	while (PTYCHAR1[letter]) {
	    ttydev [strlen(ttydev) - 2]  = ptydev [strlen(ptydev) - 2] =
		    PTYCHAR1 [letter];

	    while (PTYCHAR2[devindex]) {
		ttydev [strlen(ttydev) - 1] = ptydev [strlen(ptydev) - 1] =
			PTYCHAR2 [devindex];
		if ((*pty = open (ptydev, O_RDWR)) >= 0 &&
		    (*tty = open (ttydev, O_RDWR)) >= 0)
		{
			/*
			 * We need to set things up for our next entry
			 * into this function!
			 */
			(void) devindex++;
			return(0);
		}
		if (*pty >= 0)
		    close (*pty);
		devindex++;
	    }
	    devindex = 0;
	    (void) letter++;
	}
#endif /* USE_GET_PSEUDOTTY */
#endif /* SVR4 */
	/*
	 * We were unable to allocate a pty master!  Return an error
	 * condition and let our caller terminate cleanly.
	 */
	return(1);
#endif /* HAS_OPENPTY */
}
#endif
