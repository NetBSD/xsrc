/* $XConsortium: xedit.c,v 1.28 94/03/26 17:06:28 rws Exp $ */
 
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be 
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */
/* $XFree86: xc/programs/xedit/xedit.c,v 1.17 2002/09/22 07:09:05 paulo Exp $ */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "xedit.h"
#include <X11/Xaw/SmeBSB.h>
#include <time.h>
#include <sys/stat.h>
#include <X11/CoreP.h>

#include <stdlib.h> 

#define randomize()	srand((unsigned)time((time_t*)NULL))

static XtActionsRec actions[] = {
{"quit", QuitAction},
{"save-file", SaveFile},
{"load-file", LoadFile},
{"find-file", FindFile},
{"cancel-find-file", CancelFindFile},
{"file-completion", FileCompletion},
{"popup-menu", PopupMenu},
{"kill-file", KillFile},
{"split-window", SplitWindow},
{"dir-window", DirWindow},
{"delete-window", DeleteWindow},
{"xedit-focus", XeditFocus},
{"other-window", OtherWindow},
{"switch-source", SwitchSource},
{"lisp-eval", XeditLispEval},
{"xedit-print-lisp-eval", XeditPrintLispEval},
{"xedit-keyboard-reset",XeditKeyboardReset},
{"ispell", IspellAction},
{"line-edit", LineEditAction},
{"tags", TagsAction}
};

#define DEF_HINT_INTERVAL	300	/* in seconds, 5 minutes */

static Atom wm_delete_window;
static Widget hintswindow;
static int position_format_mask;
static XawTextPositionInfo infos[3];

Widget topwindow, textwindow, messwidget, labelwindow, filenamewindow;
Widget scratch, hpane, vpanes[2], labels[3], texts[3], forms[3], positions[3];
Widget options_popup, dirlabel, dirwindow;
Boolean international;
Boolean line_edit;
XawTextWrapMode wrapmodes[3];

extern void ResetSourceChanged(xedit_flist_item*);

static void makeButtonsAndBoxes(Widget);
static void HintsTimer(XtPointer, XtIntervalId*);
static void PositionChanged(Widget, XtPointer, XtPointer);
static void StartFormatPosition(void);
static void StartHints(void);

Display *CurDpy;

struct _app_resources app_resources;
struct _xedit_flist flist;

#define Offset(field) XtOffsetOf(struct _app_resources, field)

static XtResource resources[] = {
   {"enableBackups", "EnableBackups", XtRBoolean, sizeof(Boolean),
         Offset(enableBackups), XtRImmediate, FALSE},
   {"backupNamePrefix", "BackupNamePrefix", XtRString, sizeof(char *),
         Offset(backupNamePrefix),XtRString, ""},
   {"backupNameSuffix", "BackupNameSuffix", XtRString, sizeof(char *),
         Offset(backupNameSuffix),XtRString, ".BAK"},
   {"hints", "Hint", XtRString, sizeof(char *),
	 Offset(hints.resource), XtRImmediate, NULL},
   {"hintsInterval", XtCInterval, XtRInt, sizeof(long),
	 Offset(hints.interval), XtRImmediate, (XtPointer)DEF_HINT_INTERVAL},
   {"changedBitmap", XtRBitmap, XtRString, sizeof(char*),
	 Offset(changed_pixmap_name), XtRString, "dot"},
   {"positionFormat", "Format", XtRString, sizeof(char*),
	 Offset(position_format), XtRString, "L%l"},
   {"autoReplace", "Replace", XtRString, sizeof(char*),
	 Offset(auto_replace), XtRImmediate, NULL},
   {"tagsName", "TagsName", XtRString, sizeof(char *),
         Offset(tagsName), XtRString, "tags"},
   {"loadTags", "LoadTags", XtRBoolean, sizeof(Boolean),
	 Offset(loadTags), XtRImmediate, (XtPointer)TRUE},
};

#undef Offset

int
main(int argc, char *argv[])
{
    Boolean		exists;
    char		*filename;
    FileAccess		file_access;
    Widget		source;
    XtAppContext	appcon;
    Boolean		show_dir;
    xedit_flist_item	*first_item;
    unsigned int	i, lineno;

    lineno = 0;
    show_dir = FALSE;
    first_item = NULL;

    /* Handle args that don't require opening a display */
    for (int n = 1; n < argc; n++) {
	const char *argn = argv[n];
	/* accept single or double dash for -help & -version */
	if (argn[0] == '-' && argn[1] == '-') {
	    argn++;
	}
	if (strcmp(argn, "-help") == 0) {
	    fprintf(stderr,
                "usage: %s [-toolkitoption] [-help] [-version] [filename...]\n",
                    argv[0]);
            exit(0);
	}
	if (strcmp(argn, "-version") == 0) {
	    puts(PACKAGE_STRING);
	    exit(0);
	}
    }

    topwindow = XtAppInitialize(&appcon, "Xedit", NULL, 0, &argc, argv,
				NULL,
				NULL, 0);

    XtAppAddActions(appcon, actions, XtNumber(actions));
    XtOverrideTranslations(topwindow,
			   XtParseTranslationTable("<Message>WM_PROTOCOLS: quit()"));

    XtGetApplicationResources(topwindow, (XtPointer) &app_resources, resources,
			      XtNumber(resources), NULL, 0);

    CurDpy = XtDisplay(topwindow);
    XawSimpleMenuAddGlobalActions(appcon);
    XtRegisterGrabAction(PopupMenu, True,
			 ButtonPressMask | ButtonReleaseMask,
			 GrabModeAsync, GrabModeAsync);

    makeButtonsAndBoxes(topwindow);

    StartHints();
    StartFormatPosition();
    (void)StartHooks(appcon);
    if (position_format_mask == 0) {
	for (i = 0; i < 3; i++)
	    XtRemoveCallback(texts[i], XtNpositionCallback,
			     PositionChanged, NULL);
    }
    XtRealizeWidget(topwindow);

    XeditLispInitialize();

    options_popup = XtCreatePopupShell("optionsMenu", simpleMenuWidgetClass,
				       topwindow, NULL, 0);
    XtRealizeWidget(options_popup);
    XtAddCallback(XtCreateManagedWidget("ispell", smeBSBObjectClass,
					options_popup, NULL, 0),
		  XtNcallback, IspellCallback, NULL);
    CreateEditPopup();

    wm_delete_window = XInternAtom(XtDisplay(topwindow), "WM_DELETE_WINDOW",
				   False);
    (void)XSetWMProtocols(XtDisplay(topwindow), XtWindow(topwindow),
			  &wm_delete_window, 1);

    /* This first call is just to save the default font and colors */
    UpdateTextProperties(0);

    if (argc > 1) {
	xedit_flist_item	*item;
	Arg			args[2];
	unsigned int		num_args;

	for (i = 1; i < argc; i++) {
	    struct stat st;

	    if (argv[i][0] == '+') {
		char	*endptr;

		lineno = strtol(argv[i], &endptr, 10);
		/* Don't warn about incorrect input? */
		if (*endptr)
		    lineno = 0;
		continue;
	    }

	    filename = ResolveName(argv[i]);
	    if (filename == NULL || FindTextSource(NULL, filename) != NULL)
		continue;

	    num_args = 0;
	    if (stat(filename, &st) == 0 && !S_ISREG(st.st_mode)) {
		if (S_ISDIR(st.st_mode)) {
		    if (!first_item) {
			char path[BUFSIZ + 1];

			strncpy(path, filename, sizeof(path) - 2);
			path[sizeof(path) - 2] = '\0';
			if (*path) {
			    if (path[strlen(path) - 1] != '/')
				strcat(path, "/");
			}
			else
			    strcpy(path, "./");
			XtSetArg(args[0], XtNlabel, "");
			XtSetValues(dirlabel, args, 1);
			SwitchDirWindow(True);
			DirWindowCB(dirwindow, path, NULL);
			show_dir = True;
		    }
		    continue;
		}
	    }

	    switch (file_access = CheckFilePermissions(filename, &exists)) {
	    case NO_READ:
		if (exists)
		    XeditPrintf("File %s exists, and could not be opened for "
				"reading.\n", argv[i]);
		else
		    XeditPrintf("File %s does not exist, and the directory "
				"could not be opened for writing.\n", argv[i]);
		break;
	    case READ_OK:
		XtSetArg(args[num_args], XtNeditType, XawtextRead); num_args++;
		XeditPrintf("File %s opened READ ONLY.\n", argv[i]);
		break;
	    case WRITE_OK:
		XtSetArg(args[num_args], XtNeditType, XawtextEdit); num_args++;
		XeditPrintf("File %s opened read - write.\n", argv[i]);
		break;
	    }
	    if (file_access != NO_READ) {
		int flags;

		if (exists) {
		    flags = EXISTS_BIT;
		    XtSetArg(args[num_args], XtNstring, filename);num_args++;
		}
		else {
		    flags = 0;
		    XtSetArg(args[num_args], XtNstring, NULL);	  num_args++;
		}
		source = XtVaCreateWidget("textSource", international ?
					  multiSrcObjectClass
					  : asciiSrcObjectClass, topwindow,
					  XtNtype, XawAsciiFile,
					  XtNeditType, XawtextEdit,
					  NULL, NULL);
		XtSetValues(source, args, num_args);
		item = AddTextSource(source, argv[i], filename,
				     flags, file_access);
		XtAddCallback(item->source, XtNcallback, SourceChanged,
			      (XtPointer)item);
		if (exists && file_access == WRITE_OK) {
		    item->mode = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		    item->mtime = st.st_mtime;
		}
		if (!first_item && !show_dir)
		    first_item = item;
		ResetSourceChanged(item);
	    }
	}
    }

    if (!flist.pixmap && strlen(app_resources.changed_pixmap_name)) {
	XrmValue from, to;

	from.size = strlen(app_resources.changed_pixmap_name);
	from.addr = app_resources.changed_pixmap_name;
	to.size = sizeof(Pixmap);
	to.addr = (XtPointer)&(flist.pixmap);

	XtConvertAndStore(flist.popup, XtRString, &from, XtRBitmap, &to);
    }

    if (first_item == NULL) {
	XtSetKeyboardFocus(topwindow, filenamewindow);
	XtVaSetValues(textwindow, XtNwrap, XawtextWrapLine, NULL);
    }
    else {
	SwitchTextSource(first_item);
	XtSetKeyboardFocus(topwindow, textwindow);
	if (lineno) {
	    XawTextPosition position;

	    source = XawTextGetSource(textwindow);
	    position = RSCAN(XawTextGetInsertionPoint(textwindow),
			     lineno, False);
	    position = LSCAN(position, 1, False);
	    XawTextSetInsertionPoint(textwindow, position);
	}
    }

    XtAppMainLoop(appcon);
    return EXIT_SUCCESS;
}

static void
makeButtonsAndBoxes(Widget parent)
{
    Widget outer, b_row, viewport;
    Arg arglist[10];
    Cardinal num_args;
    xedit_flist_item *item;
    static char *labelWindow = "labelWindow", *editWindow = "editWindow";
    static char *formWindow = "formWindow", *positionWindow = "positionWindow";

    outer = XtCreateManagedWidget("paned", panedWidgetClass, parent,
				  NULL, ZERO);
 
    b_row = XtCreateManagedWidget("buttons", panedWidgetClass, outer, NULL, ZERO);
    {
	MakeCommandButton(b_row, "quit", DoQuit);
	MakeCommandButton(b_row, "save", DoSave);
	MakeCommandButton(b_row, "load", DoLoad);
	filenamewindow = MakeStringBox(b_row, "filename", NULL);
    }
    hintswindow = XtCreateManagedWidget("bc_label", labelWidgetClass,
					outer, NULL, ZERO);

    num_args = 0;
    XtSetArg(arglist[num_args], XtNeditType, XawtextEdit);		++num_args;
    messwidget = XtCreateManagedWidget("messageWindow", asciiTextWidgetClass,
				       outer, arglist, num_args);

    num_args = 0;
    XtSetArg(arglist[num_args], XtNorientation, XtorientHorizontal);	++num_args;
    hpane = XtCreateManagedWidget("hpane", panedWidgetClass, outer,
				  arglist, num_args);

    num_args = 0;
    XtSetArg(arglist[num_args], XtNorientation, XtorientVertical);	++num_args;
    vpanes[0] = XtCreateManagedWidget("vpane", panedWidgetClass, hpane,
				      arglist, num_args);
    XtSetArg(arglist[num_args], XtNheight, 1);				++num_args;
    XtSetArg(arglist[num_args], XtNwidth, 1);				++num_args;
    vpanes[1] = XtCreateWidget("vpane", panedWidgetClass, hpane,
			       arglist, num_args);

    forms[0] = XtCreateManagedWidget(formWindow, formWidgetClass,
				     vpanes[0], NULL, 0);
    labelwindow = XtCreateManagedWidget(labelWindow,labelWidgetClass,
					forms[0], NULL, 0);
    labels[0] = labelwindow;
    positions[0] = XtCreateManagedWidget(positionWindow,labelWidgetClass,
					 forms[0], NULL, 0);

    forms[2] = XtCreateWidget(formWindow, formWidgetClass,
			      vpanes[1], NULL, 0);
    labels[2] = XtCreateManagedWidget(labelWindow,labelWidgetClass,
				      forms[2], NULL, 0);
    positions[2] = XtCreateManagedWidget(positionWindow,labelWidgetClass,
					 forms[2], NULL, 0);

    num_args = 0;
    XtSetArg(arglist[num_args], XtNtype, XawAsciiFile);			++num_args;
    XtSetArg(arglist[num_args], XtNeditType, XawtextEdit);		++num_args;
    textwindow =  XtCreateManagedWidget(editWindow, asciiTextWidgetClass,
					vpanes[0], arglist, num_args);

    /* Get international resource value form the textwindow */
    num_args = 0;
    XtSetArg(arglist[num_args], XtNinternational, &international);	++num_args;
    XtGetValues(textwindow, arglist, num_args);

    num_args = 0;
    XtSetArg(arglist[num_args], XtNtype, XawAsciiFile);			++num_args;
    XtSetArg(arglist[num_args], XtNeditType, XawtextEdit);		++num_args;
    scratch = XtVaCreateWidget("textSource", international ?
			       multiSrcObjectClass :
			       asciiSrcObjectClass, topwindow,
			       XtNtype, XawAsciiFile,
			       XtNeditType, XawtextEdit,
			       NULL, NULL);
    XtSetValues(scratch, arglist, num_args);

    num_args = 0;
    XtSetArg(arglist[num_args], XtNtextSource, scratch);		++num_args;
    XtSetValues(textwindow, arglist, num_args);

    texts[0] = textwindow;
    num_args = 0;
    XtSetArg(arglist[num_args], XtNtextSource, scratch);		++num_args;
    XtSetArg(arglist[num_args], XtNdisplayCaret, False);		++num_args;
    texts[2] = XtCreateWidget(editWindow, asciiTextWidgetClass,
			      vpanes[1], arglist, num_args);

    forms[1] = XtCreateWidget(formWindow, formWidgetClass,
			      vpanes[0], NULL, 0);
    labels[1] = XtCreateManagedWidget(labelWindow,labelWidgetClass,
				      forms[1], NULL, 0);
    positions[1] = XtCreateManagedWidget(positionWindow,labelWidgetClass,
					 forms[1], NULL, 0);

    texts[1] = XtCreateWidget(editWindow, asciiTextWidgetClass,
			      vpanes[0], arglist, num_args);

    dirlabel = XtCreateWidget("dirlabel", labelWidgetClass,
			      vpanes[1], NULL, 0);
    num_args = 0;
    XtSetArg(arglist[num_args], XtNheight, 1);				++num_args;
    XtSetArg(arglist[num_args], XtNwidth, 1);				++num_args;
    viewport = XtCreateWidget("viewport", viewportWidgetClass,
			      vpanes[1], arglist, num_args);
    dirwindow = XtCreateManagedWidget("dirwindow", listWidgetClass,
				      viewport, NULL, 0);

    item = AddTextSource(scratch, "*scratch*", "*scratch*",
			 0, WRITE_OK);
    item->wrap = XawtextWrapLine;
    item->flags |= WRAP_BIT;
    XtAddCallback(item->source, XtNcallback, SourceChanged,
		  (XtPointer)item);
    ResetSourceChanged(item);
    flist.current = item;

    for (num_args = 0; num_args < 3; num_args++)
	XtAddCallback(texts[num_args], XtNpositionCallback, PositionChanged, NULL);

    for (num_args = 0; num_args < 3; num_args++) {
	XtSetArg(arglist[0], XtNwrap, &wrapmodes[num_args]);
	XtGetValues(texts[num_args], arglist, 1);
    }

    XtAddCallback(dirwindow, XtNcallback, DirWindowCB, NULL);
}

/*	Function Name: Feep
 *	Description: feeps the bell.
 *	Arguments: none.
 *	Returns: none.
 */

void
Feep(void)
{
  XBell(CurDpy, 0);
}

#define	l_BIT		0x01
#define	c_BIT		0x02
#define	p_BIT		0x04
#define	s_BIT		0x08
#define MAX_FMT_LEN	30

static void
StartFormatPosition(void)
{
    char *fmt = app_resources.position_format;

    if (fmt)
	while (*fmt)
	    if (*fmt++ == '%') {
		int len = 0;

		if (*fmt == '-') {
		    ++fmt;
		    ++len;
		}
		while (*fmt >= '0' && *fmt <= '9') {
		    ++fmt;
		    if (++len >= MAX_FMT_LEN) {
			XtAppWarning(XtWidgetToApplicationContext(topwindow),
				     "Format too large to formatPosition");
			position_format_mask = 0;
			return;
		    }
		}
		switch (*fmt++) {
		    case 'l':	position_format_mask |= l_BIT;	break;
		    case 'c':	position_format_mask |= c_BIT;	break;
		    case 'p':	position_format_mask |= p_BIT;	break;
		    case 's':	position_format_mask |= s_BIT;	break;
		    case '%':	break;
		    default: {
			char msg[256];

			XmuSnprintf(msg, sizeof(msg),
				    "Unknown format \"%%%c\" in positionFormat",
				    fmt[-1]);
			XtAppWarning(XtWidgetToApplicationContext(topwindow),
				     msg);
			position_format_mask = 0;
			return;
		    }
		}
	    }
}

/*ARGSUSED*/
static void
PositionChanged(Widget w, XtPointer client_data, XtPointer call_data)
{
    int idx;
    XawTextPositionInfo *info = (XawTextPositionInfo*)call_data;

    for (idx = 0; idx < 3; idx++)
	if (w == texts[idx])
	    break;
    if (idx > 2)
	return;

    if (((position_format_mask & l_BIT)
	  && infos[idx].line_number != info->line_number)
	|| ((position_format_mask & c_BIT)
	    && infos[idx].column_number != info->column_number)
	|| ((position_format_mask & p_BIT)
	    && infos[idx].insert_position != info->insert_position)
	|| ((position_format_mask & s_BIT)
	    && infos[idx].last_position != info->last_position)
	|| infos[idx].overwrite_mode != info->overwrite_mode) {
	int len = 6;
	Arg args[1];
	char buffer[256], *str = app_resources.position_format;
	char fmt_buf[MAX_FMT_LEN + 2], *fmt;

	memcpy(&infos[idx], info, sizeof(XawTextPositionInfo));
	if (info->overwrite_mode)
	    strcpy(buffer, "Ovrwt ");
	else
	    strcpy(buffer, "      ");
	while (*str) {
	    switch (*str) {
		case '%':
		    fmt = fmt_buf;
		    *fmt++ = *str++;
		    if (*str == '-')
			*fmt++ = *str++;
		    /*CONSTCOND*/
		    while (*str >= '0' && *str <= '9') {
			/* StartPositionFormat() already checked the format
			 * length.
			 */
			*fmt++ = *str++;
		    }
		    *fmt++ = 'd';
		    *fmt = '\0';
		    switch (*str) {
			case 'l':
			    XmuSnprintf(&buffer[len], sizeof(buffer) - len,
					fmt_buf, info->line_number);
			    break;
			case 'c':
			    XmuSnprintf(&buffer[len], sizeof(buffer) - len,
					fmt_buf, info->column_number);
			    break;
			case 'p':
			    XmuSnprintf(&buffer[len], sizeof(buffer) - len,
					fmt_buf, info->insert_position);
			    break;
			case 's':
			    XmuSnprintf(&buffer[len], sizeof(buffer) - len,
					fmt_buf, info->last_position);
			    break;
			case '%':
			    strcpy(&buffer[len], "%");
			    break;
		    }
		    len += strlen(&buffer[len]);
		    break;
		default:
		    buffer[len++] = *str;
		    break;
	    }
	    if (len >= sizeof(buffer) - 1)
		break;
	    ++str;
	}
	buffer[len] = '\0';

	XtSetArg(args[0], XtNlabel, buffer);
	XtSetValues(positions[idx], args, 1);
    }
}

/*ARGSUSED*/
static void
HintsTimer(XtPointer closure, XtIntervalId *id)
{
    Arg args[1];
    xedit_hints *hints = (xedit_hints*)closure;

    hints->cur_hint = rand() % hints->num_hints;

    XtSetArg(args[0], XtNlabel, hints->hints[hints->cur_hint]);
    XtSetValues(hintswindow, args, 1);

    hints->timer = XtAppAddTimeOut(XtWidgetToApplicationContext(topwindow),
				   hints->interval, HintsTimer, closure);
}

#define MAX_HINT_LEN		255
#define MIN_HINT_INTERVAL	5
static void
StartHints(void)
{
    char *str, *p;
    unsigned i, len;
    xedit_hints *hints = &(app_resources.hints);

    /* if resource was not set, or was overriden */
    if (hints->resource == NULL || !*hints->resource)
	return;

    randomize();

    if (hints->interval < MIN_HINT_INTERVAL)
	hints->interval = DEF_HINT_INTERVAL;
    hints->interval *= 1000;
    hints->hints = (char**)XtMalloc(sizeof(char*));
    hints->hints[hints->cur_hint = 0] = p = hints->resource;
    hints->num_hints = 1;

    while ((p = strchr(p, '\n')) != NULL) {
	if (*++p == '\0')
	    break;
	hints->hints = (char**)
	    XtRealloc((char*)hints->hints,
		      sizeof(char*) * (hints->num_hints + 1));
	hints->hints[hints->num_hints++] = p;
    }

    /* make a private copy of the resource values, so that one can change
     * the Xrm database safely.
     */
    for (i = 0; i < hints->num_hints; i++) {
	if ((p = strchr(hints->hints[i], '\n')) != NULL)
	    len = p - hints->hints[i];
	else
	    len = strlen(hints->hints[i]);
	if (len > MAX_HINT_LEN)
	    len = MAX_HINT_LEN;
	str = XtMalloc(len + 1);
	strncpy(str, hints->hints[i], len);
	str[len] = '\0';
	hints->hints[i] = str;
    }

    hints->timer = XtAppAddTimeOut(XtWidgetToApplicationContext(topwindow),
				   hints->interval, HintsTimer,
				   (XtPointer)hints);
}
