/* $XConsortium: omTextEsc.c,v 1.2 94/01/20 18:08:18 rws Exp $ */
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

static int
_XomGenericTextEscapement(oc, type, text, length)
    XOC oc;
    XOMTextType type;
    XPointer text;
    int length;
{
    XlcConv conv;
    XFontStruct *font;
    Bool is_xchar2b;
    XPointer args[2];
    XChar2b xchar2b_buf[BUFSIZ], *buf;
    int buf_len, left, width = 0;

    conv = _XomInitConverter(oc, type);
    if (conv == NULL)
	return width;
    
    args[0] = (XPointer) &font;
    args[1] = (XPointer) &is_xchar2b;

    while (length > 0) {
	buf = xchar2b_buf;
	left = buf_len = BUFSIZ;

	if (_XomConvert(oc, conv, (XPointer *) &text, &length,
			(XPointer *) &buf, &left, args, 2) < 0)
	    break;
	buf_len -= left;

	if (is_xchar2b)
	    width += XTextWidth16(font, xchar2b_buf, buf_len);
	else
	    width += XTextWidth(font, (char *) xchar2b_buf, buf_len);
    }

    return width;
}

int
#if NeedFunctionPrototypes
_XmbGenericTextEscapement(XOC oc, _Xconst char *text, int length)
#else
_XmbGenericTextEscapement(oc, text, length)
    XOC oc;
    char *text;
    int length;
#endif
{
    return _XomGenericTextEscapement(oc, XOMMultiByte, (XPointer) text, length);
}

int
#if NeedFunctionPrototypes
_XwcGenericTextEscapement(XOC oc, _Xconst wchar_t *text, int length)
#else
_XwcGenericTextEscapement(oc, text, length)
    XOC oc;
    wchar_t *text;
    int length;
#endif
{
    return _XomGenericTextEscapement(oc, XOMWideChar, (XPointer) text, length);
}
