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

/* $XFree86: xc/programs/xedit/lisp.c,v 1.3 2001/09/11 06:42:54 paulo Exp $ */

#include "xedit.h"
#include "lisp/lisp.h"
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <sys/wait.h>

/*
 * Prototypes
 */
static void XeditLispInitialize(void);
static void XeditRunLisp(void);
static void LispInputCallback(XtPointer, int*, XtInputId*);
static void LispErrorInputCallback(XtPointer, int*, XtInputId*);
static void XeditDoLispEval(Widget, XEvent*, String*, Cardinal*);
static int XeditCheckLispChild(void);

/*
 * Initialization
 */
static struct {
    int pid;
    int ifd[2];
    int ofd[2];
    int efd[2];
    XtInputId id;
    XtInputId eid;
    XtAppContext appcon;
    Widget output;
    Bool expect;
    Bool running;
} lisp;

extern XtAppContext appcon;
extern Widget scratch;

/*
 * Implementation
 */
void
XeditLispEval(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    lisp.output = messwidget;
    XeditDoLispEval(w, event, params, num_params);
}

void
XeditPrintLispEval(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (XawTextGetSource(textwindow) == scratch) {
	lisp.output = textwindow;
	XtCallActionProc(w, "newline", event, params, *num_params);
	XeditDoLispEval(w, event, params, num_params);
    }
    else
	XtCallActionProc(w, "newline-and-indent", event, params, *num_params);
}

void
XeditKeyboardReset(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    (void)XeditCheckLispChild();
    if (lisp.running) {
	kill(lisp.pid, SIGINT);
	lisp.running = False;

	/* redisplay */
	XtUnmapWidget(XtParent(messwidget));
	XtMapWidget(XtParent(messwidget));
    }
    XtCallActionProc(w, "keyboard-reset", event, params, *num_params);
}

static void
XeditDoLispEval(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    Widget src;
    XawTextBlock block;
    XawTextPosition position, end;
    XtInputMask mask;
    int gotchars = 0;

    XeditLispInitialize();

    if (XtClass(w) != asciiTextWidgetClass) {
	Feep();
	return;
    }

    /* get lisp expression */
    if ((position = XawTextGetInsertionPoint(w)) == 0) {
	Feep();
	return;
    }
    end = position;
    --position;

    src = XawTextGetSource(w);
    while (position >= 0) {
	(void)XawTextSourceRead(src, position, &block, 1);
	if (!isspace(block.ptr[0])) {
	    ++gotchars;
	    break;
	}
	--gotchars;
	--position;
    }

    if (block.ptr[0] != ')') {
	while (position >= 0) {
	    (void)XawTextSourceRead(src, position, &block, 1);
	    if (isspace(block.ptr[0]) ||
		block.ptr[0] == '(' ||
		block.ptr[0] == ')' ||
		block.ptr[0] == '\'')
		break;
	    ++gotchars;
	    --position;
	}
	++position;
	if (position == end || gotchars <= 0) {
	    Feep();
	    return;
	}
	if (block.ptr[0] == '\'' && position > 0)
	    --position;
    }
    else {
	/* XXX note that embedded '(' and ')' will confuse this code */
	XawTextPosition last, tmp;
	int level = 0;
	char ptr[2];

	last = position;
	ptr[1] = '\0';
	block.ptr = ptr;
	do {
	    block.ptr[0] = '(';
	    position = XawTextSourceSearch(src, last, XawsdLeft, &block);
	    if (position == XawTextSearchError) {
		Feep();
		return;
	    }
	    block.ptr[0] = ')';
	    tmp = position;
	    do {
		tmp = XawTextSourceSearch(src, tmp, XawsdRight, &block);
		if (tmp == XawTextSearchError) {
		    Feep();
		    return;
		}
		if (tmp <= last)
		    ++level;
	    } while (++tmp <= last);
	    --level;
	    last = position;
	} while (level);
	/* check for quoted expression */
	if (position) {
	    (void)XawTextSourceRead(src, position - 1, &block, 1);
	    if (block.ptr[0] == '\'')
		--position;
	}
    }

    while (position < end) {
	(void)XawTextSourceRead(src, position, &block, end - position);
	write(lisp.ofd[1], block.ptr, block.length);
	position += block.length;
    }
    write(lisp.ofd[1], "\n", 1);
    lisp.running = lisp.expect = True;

    /* block waiting for lisp process to finish, need to block or
     * user won't know if it is stalled.
     */
    while (lisp.running) {
	mask = XtAppPending(lisp.appcon);
	if (mask & XtIMAlternateInput)
	    XtAppProcessEvent(lisp.appcon, XtIMAlternateInput);
	else if (mask & XtIMXEvent) {
	    /* only keyboard events allowed */
	    XEvent key;

	    XtAppNextEvent(lisp.appcon, &key);
	    if (key.type == KeyPress ||
		key.type == KeyRelease) {
		/* XXX Ctrl<G> harcoded */
		if (key.xkey.state & ControlMask) {
		    char buffer[2];

		    XLookupString((XKeyEvent*)&key, &buffer[0],
				  sizeof(buffer), NULL, NULL);
		    if (*buffer == '\a')
			XtDispatchEvent(&key);
		}
	    }
	}
	if (XeditCheckLispChild())
	    break;
    }
}

void
XeditLispCleanUp(void)
{
    if (lisp.pid) {
	kill(lisp.pid, SIGTERM);
	lisp.pid = 0;
    }
}

static void
XeditLispInitialize(void)
{
    if (lisp.pid)
	return;

    pipe(lisp.ifd);
    pipe(lisp.ofd);
    pipe(lisp.efd);

    if ((lisp.pid = fork()) == 0) {
	setlocale(LC_NUMERIC, "C");
	close(0);
	close(1);
	close(2);
	dup2(lisp.ofd[0], 0);
	dup2(lisp.ifd[1], 1);
	dup2(lisp.efd[1], 2);
	close(lisp.ifd[0]);
	close(lisp.ifd[1]);
	close(lisp.ofd[0]);
	close(lisp.ofd[1]);
	close(lisp.efd[0]);
	close(lisp.efd[1]);
	XeditRunLisp();
	exit(0);
    }
    else if (lisp.pid < 0) {
	fprintf(stderr, "Cannot fork\n");
	exit(1);
    }

    lisp.appcon = XtWidgetToApplicationContext(topwindow);
    lisp.id = XtAppAddInput(lisp.appcon, lisp.ifd[0],
			    (XtPointer)XtInputReadMask,
			    LispInputCallback, NULL);
    fcntl(lisp.ifd[0], F_SETFL, O_NONBLOCK);
    lisp.eid = XtAppAddInput(lisp.appcon, lisp.efd[0],
			     (XtPointer)XtInputReadMask,
			     LispErrorInputCallback, NULL);
    fcntl(lisp.efd[0], F_SETFL, O_NONBLOCK);
}

static void
LispInputCallback(XtPointer closure, int *source, XtInputId *id)
{
    int len;
    char str[8192];

    len = read(lisp.ifd[0], str, sizeof(str) - 1);
    if (len && len < PROTOMAXSIZE && *str == PROTOPREFFIX) {
	char *res = NULL;

	str[len] = '\0';
	len = 0;
	lisp.expect = 0;

	if (XeditProto(str + 1, &res) == False) {
	    if (res)
		XeditPrintf(res);
	}
	else {
	    write(lisp.ofd[1], res, strlen(res));
	    write(lisp.ofd[1], "\n", 1);
	}
    }
    if (len > 0) {
	if (lisp.output == messwidget) {
	    str[len] = '\0';
	    XeditPrintf(str);
	}
	else {
	    XawTextBlock block;
	    XawTextPosition pos = XawTextGetInsertionPoint(lisp.output);

	    block.firstPos = 0;
	    block.format = FMT8BIT;
	    block.length = len;
	    block.ptr = str;
	    XawTextReplace(lisp.output, pos, pos, &block);
	    pos += len;
	    XawTextSetInsertionPoint(lisp.output, pos);
	}
	lisp.expect = 0;
    }
    /* If anything was printed, than it is not busy anymore */
    lisp.running = 0;
    XeditCheckLispChild();
}

static void
LispErrorInputCallback(XtPointer closure, int *source, XtInputId *id)
{
    int len;
    char str[1024];

    Feep();
    len = read(lisp.efd[0], str, sizeof(str) - 1);
    if (len > 0) {
	str[len] = '\0';
	XeditPrintf(str);
    }
    /* If anything was printed, than it is not busy anymore */
    lisp.running = 0;
    XeditCheckLispChild();
}

static int
XeditCheckLispChild(void)
{
    int status;

    if (lisp.pid) {
	waitpid(lisp.pid, &status, WNOHANG);
	if (WIFEXITED(status) || errno == ECHILD) {
	    Feep();
	    /* redisplay */
	    XtUnmapWidget(XtParent(messwidget));
	    XtMapWidget(XtParent(messwidget));

	    XeditPrintf("Warning: lisp child process exited.\n");
	    lisp.pid = 0;
	    XtRemoveInput(lisp.id);
	    XtRemoveInput(lisp.eid);
	    close(lisp.ifd[0]);
	    close(lisp.ifd[1]);
	    close(lisp.ofd[0]);
	    close(lisp.ofd[1]);
	    close(lisp.efd[0]);
	    close(lisp.efd[1]);
	    lisp.running = lisp.expect = 0;

	    return (1);
	}
    }

    return (0);
}

static void
XeditRunLisp(void)
{
    LispMac *mac = LispBegin(0, NULL);

    LispSetPrompt(mac, NULL);
    LispExecute(mac, "(require \"fun\")\n");
    LispMachine(mac);

    LispEnd(mac);
}
