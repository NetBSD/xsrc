/* $OpenBSD: callbacks.c,v 1.3 2002/06/06 18:24:47 matthieu Exp $ */
/*
 * Copyright (c) 2002 Matthieu Herrb and Niels Provos
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
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Shell.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include "callbacks.h"
#include "interface.h"

typedef enum {
	WAIT_SYSCALL,
	WAIT_POLICY,
	WAIT_ACK,
} notificatition_state_t;

static notificatition_state_t state = WAIT_SYSCALL;
static Widget policyText;
static char *errorcode;

static void TextAppend(Widget, char *, int);
static void TextReplace(Widget, int, int, XawTextBlock *);
static long  TextLength(Widget);

XtInputId inputId;

static void
dprintf(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	if (getenv("NOTIFICATION_DEBUG") != NULL) {
		vfprintf(stderr, format, ap);
	}
	va_end(ap);
}
	
static char *
freadline(char *line, int size, int fd)
{
	char *p = line;
	int n;

	do {
		if ((n = read(fd, p, 1)) <= 0) {
			dprintf("got null line n=%d state %d\n", n, state);
			XtRemoveInput(inputId);
			return NULL;
		}
		if (*p == '\n') break;
		p++;
	} while (1);
	
	*p = '\0';

	dprintf("state %d got line: %s\n", state, line);
	return (line);
}

void
getInput(XtPointer clientData, int *file, XtInputId *inputId)
{
	char line[1024], *p;
        char *name, *id, *polname, *filters;
        time_t curtime;
	int nfilters;

	if (freadline(line, sizeof(line), *file) == NULL) {
		done = True;
		return;
	}

	switch (state) {
	case WAIT_POLICY:
		if (strcmp(line, "WRONG") == 0) {
			state = WAIT_ACK;
			return;
		}
		TextAppend(policyText, line, strlen(line));
		TextAppend(policyText, "\n", 1);
		return;

	case WAIT_ACK:
		if (strcmp(line, "OKAY") == 0) {
			XtUnmapWidget((Widget)clientData);
			state = WAIT_SYSCALL;
			return;
		} 
		if (strcmp(line, "WRONG") == 0) {
			return;
		}
		XtVaSetValues(status, XtNlabel, line, NULL);
		return;

	case WAIT_SYSCALL:
		p = line;
		name = strsep(&p, ",");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		p++;
		strsep(&p, " ");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		id = strsep(&p, "(");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		strsep(&p, ":");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		p++;
		polname = strsep(&p, ",");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		strsep(&p, ":");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		p++;
		filters = strsep(&p, ",");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		nfilters = atoi(filters);
		strsep(&p, ":");
		if (p == NULL || *p == '\0')
			errx(1, "Bad input line");
		p++;
		
		XtVaSetValues(pName, XtNlabel, name, NULL);
		XtVaSetValues(pId, XtNlabel, id, NULL);
		XtVaSetValues(policyName, XtNlabel, polname, NULL);
		XtVaSetValues(syscallName, XtNlabel, p, NULL);
		XtVaSetValues(status, XtNlabel, "", NULL);
		
		curtime = time(NULL);
		snprintf(line, sizeof(line), "%.25s", ctime(&curtime));
		XtVaSetValues(timeline, XtNlabel, line, NULL);
		
		position_near_center((Widget)clientData);
		XtMapWidget((Widget)clientData);
		state = WAIT_ACK;
		return;
	} /* switch */
}

void 
on_error_select(Widget w, XtPointer userData, XtPointer clientData)
{
	XawTextBlock    block;
	
	errorcode = (char *)userData;
	block.ptr = errorcode;
	block.firstPos = 0;
	block.length = strlen(errorcode);
	block.format = FMT8BIT;
	TextReplace(errorCodeText, 0, TextLength(errorCodeText), &block);
}

void 
on_error_entry_changed(Widget w, XEvent *event, String *params, 
			Cardinal *num_params)
{
	Arg args[1];

	XtSetArg(args[0], XtNstring, &errorcode);
	XtGetValues(w, args, 1);
	dprintf("new error code %s\n", errorcode);
}

void
on_denyone_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	Arg args[1];

	XtSetArg(args[0], XtNstring, &errorcode);
	XtGetValues(w, args, 1);

	dprintf("deny-now[%s]\n", errorcode);
	printf("deny-now[%s]\n", errorcode);
}

void
on_permitonce_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	printf("permit-now\n");
}

void
on_deny_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	Arg args[1];

	XtSetArg(args[0], XtNstring, &errorcode);
	XtGetValues(w, args, 1);
	
	dprintf("deny[%s]\n", errorcode);
	printf("deny[%s]\n", errorcode);
}

void
on_permit_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	printf("permit\n");
}

void 
on_filter_entry_changed(Widget w, XEvent *event, String *params, 
			Cardinal *num_params)
{
	Arg args[1];
	char *name;

	XtSetArg(args[0], XtNstring, &name);
	XtGetValues(filter, args, 1);
	
	printf("%s\n", name);
}

void
on_filteradd_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	Arg args[1];
	char *name;

	XtSetArg(args[0], XtNstring, &name);
	XtGetValues(filter, args, 1);
	
	printf("%s\n", name);
}

void
on_detachbutton_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	printf("detach\n");
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
	TextReplace (w, last, last, &block);
	if (current == last)
		XawTextSetInsertionPoint (w, last + block.length);
}

void
on_reviewbutton_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	Widget shell, form, b;

	printf("review\n");
	
	shell = XtVaCreatePopupShell("Review", transientShellWidgetClass,
				     w, NULL, 0);
	form = XtCreateManagedWidget("review-form", formWidgetClass, shell,
				     NULL, 0);
	XtCreateManagedWidget("review-label", labelWidgetClass, form,
			      NULL, 0);
	policyText = XtCreateManagedWidget("review-text", asciiTextWidgetClass,
				     form, NULL, 0);
	b = XtCreateManagedWidget("done-button", commandWidgetClass, form,
				  NULL, 0);
	XtAddCallback(b, XtNcallback, on_done_button, (XtPointer)shell);
	XtRealizeWidget(shell);
	XSetWMProtocols(XtDisplay(shell), XtWindow(shell), 
			&wm_delete_window, 1);
	XtPopup(shell, XtGrabNone);
	state = WAIT_POLICY;
}

void
on_done_button(Widget w, XtPointer closure, XtPointer clientData)
{
	Widget shell = (Widget)closure;

	XtPopdown(shell);
	XtDestroyWidget(shell);
}

void
on_killbutton_clicked(Widget w, XtPointer closure, XtPointer clientData)
{
	printf("kill\n");
}
