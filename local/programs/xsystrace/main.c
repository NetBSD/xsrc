/* $OpenBSD: main.c,v 1.3 2002/09/29 21:30:34 matthieu Exp $ */
/*
 * Copyright (c) 2002 Matthieu Herrb
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "callbacks.h"
#include "interface.h"

extern char *__progname;

static void exitAction(Widget, XEvent *, String *, Cardinal *);

static struct XsystraceResources {
	Boolean center;
	Boolean nearMouse;
	int timeout_secs;
	Dimension maxHeight;
	Dimension maxWidth;
} nres;

#define offset(field) XtOffsetOf(struct XsystraceResources, field)
static XtResource resources[] = {
	{ "center", "Center", XtRBoolean, sizeof (Boolean),
	  offset(center), XtRString, "false" },
	{ "nearMouse", "NearMouse", XtRBoolean, sizeof (Boolean),
	  offset(nearMouse), XtRString, "false" },
	{ "timeout", "Timeout", XtRInt, sizeof (int),
	  offset(timeout_secs), XtRInt, 0 },
	{ "maxHeight", "Maximum", XtRDimension, sizeof (Dimension),
	  offset(maxHeight), XtRDimension, 0 },
	{ "maxWidth", "Maximum", XtRDimension, sizeof (Dimension),
	  offset(maxWidth), XtRDimension, 0 },
};
#undef offset

static XrmOptionDescRec optionList[] = {
	{ "-center",  ".center", XrmoptionNoArg, (XPointer) "on" },
	{ "-nearmouse", ".nearMouse", XrmoptionNoArg, (XPointer) "on" },
	{ "-timeout", ".timeout", XrmoptionSepArg, (XPointer) NULL },
};

static String fallbackResources[] = {
	"*.allow*width: 100",
	NULL,
};

static XtActionsRec actionsList[] = {
	{ "filter-changed", on_filter_entry_changed },
	{ "errorcode-changed", on_error_entry_changed },
	{ "exit", exitAction},
};

static String topTranslations = 
"<ClientMessage>WM_PROTOCOLS: exit(1)\n";

/*
 * Handle ICCCM delete_window
 */

static void
exitAction(Widget w, XEvent *event, String *params, Cardinal *numParams)
{
	int exitStatus = 0;
	
	if (event->type == ClientMessage 
	    && event->xclient.data.l[0] != wm_delete_window) 
		return;
	if (*numParams == 1) {
		exitStatus = atoi(params[0]);
	}
	exit(exitStatus);
}

/*
 * Handle timeout
 */
static void
timeOut(XtPointer clientData, XtIntervalId *iid)
{
	exit(0);
}

int
main(int argc, char *argv[])
{
	Widget top;

	XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL);
	top = XtAppInitialize(&appContext, "Xsystrace",
			      optionList, XtNumber(optionList),
			      &argc, argv, fallbackResources, 
			      NULL, 0);
	XtGetApplicationResources(top, (XtPointer)&nres, resources,
				  XtNumber(resources), NULL, 0);

	wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW",
				      False);
	XtAppAddActions(appContext, actionsList, XtNumber(actionsList));
	XtOverrideTranslations(top, XtParseTranslationTable(topTranslations));

	/* Create the form */
	if (makeForm(top) == NULL) {
		fprintf(stderr, "%s: unable to create form\n", __progname);
		exit(1);
	}

	XtSetMappedWhenManaged(top, FALSE);
	
	XtRealizeWidget(top);
	/* do WM_DELETE_WINDOW */
	XSetWMProtocols(XtDisplay(top), XtWindow(top), &wm_delete_window, 1);
		
	/* Register timeout */
	if (nres.timeout_secs) 
		XtAppAddTimeOut(appContext, 1000*nres.timeout_secs, 
				timeOut, NULL);
	
	setvbuf(stdin, NULL, _IOLBF, 0);
	setvbuf(stdout, NULL, _IOLBF, 0);

	inputId = XtAppAddInput(appContext, fileno(stdin), 
				(XtPointer)XtInputReadMask, 
				getInput, top);

	done = False;
	while (!done) {
		XEvent event;

		XtAppNextEvent(appContext, &event);
		XtDispatchEvent(&event);

	}

	exit(0);
}
