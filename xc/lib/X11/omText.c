/* $XConsortium: omText.c,v 1.2 94/01/20 18:08:14 rws Exp $ */
/*
 * Copyright 1992, 1993 by TOSHIBA Corp.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of TOSHIBA not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. TOSHIBA make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * TOSHIBA DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * TOSHIBA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Katsuhisa Yano	TOSHIBA Corp.
 *			   	mopi@osa.ilab.toshiba.co.jp
 */

#include "Xlibint.h"
#include "XomGeneric.h"
#include <stdio.h>

int
_XomGenericDrawString(dpy, d, oc, gc, x, y, type, text, length)
    Display *dpy;
    Drawable d;
    XOC oc;
    GC gc;
    int x, y;
    XOMTextType type;
    XPointer text;
    int length;
{
    XlcConv conv;
    XFontStruct *font;
    Bool is_xchar2b;
    XPointer args[2];
    XChar2b xchar2b_buf[BUFSIZ], *buf;
    int buf_len, left, start_x = x;

    conv = _XomInitConverter(oc, type);
    if (conv == NULL)
	return -1;
    
    args[0] = (XPointer) &font;
    args[1] = (XPointer) &is_xchar2b;

    while (length > 0) {
	buf = xchar2b_buf;
	left = buf_len = BUFSIZ;

	if (_XomConvert(oc, conv, (XPointer *) &text, &length,
			(XPointer *) &buf, &left, args, 2) < 0)
	    break;
	buf_len -= left;

	XSetFont(dpy, gc, font->fid);

	if (is_xchar2b) {
	    XDrawString16(dpy, d, gc, x, y, xchar2b_buf, buf_len);
	    x += XTextWidth16(font, xchar2b_buf, buf_len);
        } else {
	    XDrawString(dpy, d, gc, x, y, (char *) xchar2b_buf, buf_len);
	    x += XTextWidth(font, (char *) xchar2b_buf, buf_len);
	}
    }

    x -= start_x;

    return x;
}

int
#if NeedFunctionPrototypes
_XmbGenericDrawString(Display *dpy, Drawable d, XOC oc, GC gc, int x, int y,
		      _Xconst char *text, int length)
#else
_XmbGenericDrawString(dpy, d, oc, gc, x, y, text, length)
    Display *dpy;
    Drawable d;
    XOC oc;
    GC gc;
    int x, y;
    char *text;
    int length;
#endif
{
    return _XomGenericDrawString(dpy, d, oc, gc, x, y, XOMMultiByte,
				 (XPointer) text, length);
}

int
#if NeedFunctionPrototypes
_XwcGenericDrawString(Display *dpy, Drawable d, XOC oc, GC gc, int x, int y,
		      _Xconst wchar_t *text, int length)
#else
_XwcGenericDrawString(dpy, d, oc, gc, x, y, text, length)
    Display *dpy;
    Drawable d;
    XOC oc;
    GC gc;
    int x, y;
    wchar_t *text;
    int length;
#endif
{
    return _XomGenericDrawString(dpy, d, oc, gc, x, y, XOMWideChar,
				 (XPointer) text, length);
}
