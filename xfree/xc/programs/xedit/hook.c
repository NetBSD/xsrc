/*
 * Copyright (c) 1999 by The XFree86 Project, Inc.
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

/* $XFree86: xc/programs/xedit/hook.c,v 1.3 1999/06/06 08:49:14 dawes Exp $ */

/*
 * This file is intended to be used to add all the necessary hooks to xedit
 * emulate certain features of emacs (and other text editors) that are better
 * kept only in xedit, to avoid unnecessary code in the Text Widget.
 *
 * The code here is not finished, and will probably be changed frequently.
 */

#include "xedit.h"
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif
#include <string.h>
#include <ctype.h>

/*
 * Types
 */
typedef struct _ReplaceList {
    char *word;
    char *replace;
    struct _ReplaceList *next;
} ReplaceList;

/*
 * Prototypes
 */
static void ActionHook(Widget, XtPointer, String, XEvent*, String*, Cardinal*);
static void AutoReplaceHook(Widget, String, XEvent*);
static Bool StartAutoReplace(void);
static char *ReplacedWord(char*, char*);
static void AutoReplace(Widget, XEvent*);
static void AutoReplaceCallback(Widget, XtPointer, XtPointer);

/*
 * Initialization
 */
#define STRTBLSZ	11
static ReplaceList *replace_list[STRTBLSZ];

/*
 * Implementation
 */
Bool
StartHooks(XtAppContext app)
{
    static Bool first_time = True;

    if (first_time) {
	if (StartAutoReplace())
	    (void)XtAppAddActionHook(app, ActionHook, NULL);
	first_time = False;

	return (True);
    }
    return (False);
}

/*ARGSUSED*/
static void
ActionHook(Widget w, XtPointer client_data, String action, XEvent *event,
	   String *params, Cardinal *num_params)
{
    AutoReplaceHook(w, action, event);
}

/*** auto replace ***/
struct {
    Widget widget;
    String text;
    Cardinal length;
    XawTextPosition left, right;
    Bool replace;
} auto_replace;

static void
AutoReplaceHook(Widget w, String action, XEvent *event)
{
    static Bool multiply;

    if (w != textwindow)
	return;

    if (auto_replace.widget != textwindow) {
	if (auto_replace.replace) {
	    auto_replace.replace = False;
	    XtRemoveCallback(auto_replace.widget, XtNpositionCallback,
			     AutoReplaceCallback, NULL);
	}
    }
    else if (strcmp(action, "multiply") == 0) {
	multiply = True;
	return;
    }
    else if (strcmp(action, "numeric") == 0) {
	if (multiply)
	    return;
    }
    else if (strcmp(action, "insert-char") && strcmp(action, "newline") &&
	strcmp(action, "newline-and-indent")) {
	return;
    }
    multiply = False;

    AutoReplace(w, event);
}

static Bool
StartAutoReplace(void)
{
    Bool esc;
    int len, llen, rlen, count = 0;
    char ch, *tmp, *left, *right, *replace = app_resources.auto_replace;

    if (!replace || !*replace)
	return (False);

    left = XtMalloc(llen = 256);
    right = XtMalloc(rlen = 256);
    while (*replace) {
	/* skip white spaces */
	while (*replace && isspace(*replace))
	    ++replace;
	if (!*replace)
	    break;

	/* read left */
	tmp = replace;
	while (*replace && !isspace(*replace))
	    ++replace;
	len = replace - tmp;
	if (len >= llen)
	    left = XtRealloc(left, llen = len + 1);
	strncpy(left, tmp, len);
	left[len] = '\0';

	/* skip white spaces */
	while (*replace && isspace(*replace))
	    ++replace;

	/* read right */
	len = 0;
	esc = False;
	while ((ch = *replace) != '\0') {
	    ++replace;
	    if (len + 2 >= rlen)
		right = XtRealloc(right, rlen += 256);
	    if (ch == '\\') {
		if (esc)
		    right[len++] = '\\';
		esc = !esc;
		continue;
	    }
	    else if (ch == '\n' && !esc)
		break;
	    else
		right[len++] = ch;
	    esc = False;
	}
	right[len] = '\0';

	(void)ReplacedWord(left, right);
	++count;
    }
    XtFree(left);
    XtFree(right);

    return (count > 0);
}

static char *
ReplacedWord(char *word, char *replace)
{
    ReplaceList *list;
    int ii = 0;
    char *pp = word;

    while (*pp)
	ii = (ii << 1) ^ *pp++;
    if (ii < 0)
	ii = -ii;
    ii %= STRTBLSZ;
    for (list = replace_list[ii]; list; list = list->next)
	if (strcmp(list->word, word) == 0) {
	    if (replace) {
		XtFree(list->replace);
		list->replace = XtNewString(replace);
	    }
	    return (list->replace);
	}

    if (!replace)
	return (NULL);

    list = XtNew(ReplaceList);
    list->word = XtNewString(word);
    list->replace = XtNewString(replace);
    list->next = replace_list[ii];
    replace_list[ii] = list;

    return (list->replace);
}

static void
AutoReplace(Widget w, XEvent *event)
{
    static XComposeStatus compose = {NULL, 0};
    KeySym keysym;
    XawTextBlock block;
    XawTextPosition left, right, pos;
    Widget source;
    int i, len, size;
    char *str, buf[32], mb[sizeof(wchar_t)];

    size = XLookupString((XKeyEvent*)event, mb, sizeof(mb), &keysym, &compose);

    if (size != 1 || isalnum(*mb))
	return;

    source = XawTextGetSource(w);
    right = XawTextGetInsertionPoint(w);
    left = XawTextSourceScan(source, right, XawstWhiteSpace,
			     XawsdLeft, 1, False);

    if (left < 0 || left == right)
	return;

    len = 0;
    str = buf;
    size = sizeof(buf);
    pos = left;
    while (pos < right) {
	pos = XawTextSourceRead(source, pos, &block, right - pos);
	for (i = 0; i < block.length; i++) {
	    if (block.format == FMT8BIT)
		*mb = block.ptr[i];
	    else
		wctomb(mb, ((wchar_t*)block.ptr)[i]);
	    str[len++] = *mb;
	    if (len + 2 >= size) {
		if (str == buf)
		    str = XtMalloc(size += sizeof(buf));
		else
		    str = XtRealloc(str, size += sizeof(buf));
	    }
	}
    }
    str[len] = '\0';
    if ((auto_replace.text = ReplacedWord(str, NULL)) != NULL) {
	auto_replace.length = strlen(auto_replace.text);
	auto_replace.left = left;
	auto_replace.right = right;
	auto_replace.replace = True;
	XtAddCallback(auto_replace.widget = w, XtNpositionCallback,
		      AutoReplaceCallback, NULL);
    }
    if (str != buf)
	XtFree(str);
}

/*ARGSUSED*/
static void
AutoReplaceCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, inc;
    XawTextBlock block, text;
    char buffer[1024], mb[sizeof(wchar_t)];
    XawTextPosition left, right, pos;

    if (!auto_replace.replace || w != auto_replace.widget)
	return;

    XtRemoveCallback(auto_replace.widget, XtNpositionCallback,
		     AutoReplaceCallback, NULL);
    auto_replace.replace = False;

    inc = XawTextGetInsertionPoint(w) - auto_replace.right;
    if (auto_replace.length + inc > sizeof(buffer))
	block.ptr = XtMalloc(auto_replace.length + inc);
    else
	block.ptr = buffer;
    memcpy(block.ptr, auto_replace.text, auto_replace.length);

    block.length = auto_replace.length;
    pos = left = auto_replace.right;
    right = left + inc;
    while (pos < right) {
	pos = XawTextSourceRead(XawTextGetSource(w), pos, &text, inc);
	for (i = 0; i < text.length; i++) {
	    if (text.format == FMT8BIT)
		*mb = text.ptr[i];
	    else
		wctomb(mb, ((wchar_t*)text.ptr)[i]);
	    block.ptr[block.length++] = *mb;
	}
    }

    block.firstPos = 0;
    block.format = FMT8BIT;

    if (XawTextReplace(w, auto_replace.left, auto_replace.right + inc,
		       &block) == XawEditDone)
	XawTextSetInsertionPoint(w, auto_replace.left + block.length);

    if (block.ptr != buffer)
	XtFree(block.ptr);
}
