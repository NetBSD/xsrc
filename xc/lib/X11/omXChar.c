/* $XConsortium: omXChar.c,v 1.3 94/02/06 15:10:11 rws Exp $ */
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
#include "XlcPublic.h"
#include "XomGeneric.h"
#include <stdio.h>

static FontSet
_XomGetFontSetFromCharSet(oc, charset)
    XOC oc;
    XlcCharSet charset;
{
    register FontSet font_set = XOC_GENERIC(oc)->font_set;
    register num = XOC_GENERIC(oc)->font_set_num;
    XlcCharSet *charset_list;
    int charset_count;

    for ( ; num-- > 0; font_set++) {
	charset_count = font_set->charset_count;
	charset_list = font_set->charset_list;
	for ( ; charset_count-- > 0; charset_list++)
	    if (*charset_list == charset)
		return font_set;
    }

    return (FontSet) NULL;
}

#ifdef MUSTCOPY
static void
cs_to_xchar2b(from, to, length)
    register char *from;
    register XChar2b *to;
    register length;
{
    while (length-- > 0) {
	to->byte1 = *from++;
	to->byte2 = *from++;
	to++;
    }
}

static void
cs_to_xchar2b_gl(from, to, length)
    register char *from;
    register XChar2b *to;
    register length;
{
    while (length-- > 0) {
	to->byte1 = *from++ & 0x7f;
	to->byte2 = *from++ & 0x7f;
	to++;
    }
}

static void
cs_to_xchar2b_gr(from, to, length)
    register char *from;
    register XChar2b *to;
    register length;
{
    while (length-- > 0) {
	to->byte1 = *from++ | 0x80;
	to->byte2 = *from++ | 0x80;
	to++;
    }
}
#endif

static void
shift_to_gl(text, length)
    register char *text;
    register length;
{
    while (length-- > 0)
	*text++ &= 0x7f;
}

static void
shift_to_gr(text, length)
    register char *text;
    register length;
{
    while (length-- > 0)
	*text++ |= 0x80;
}

static Bool
load_font(oc, font_set)
    XOC oc;
    FontSet font_set;
{
    font_set->font = XLoadQueryFont(oc->core.om->core.display,
			oc->core.font_info.font_name_list[font_set->id]);
    if (font_set->font == NULL)
	return False;
    
    oc->core.font_info.font_struct_list[font_set->id] = font_set->font;
    XFreeFontInfo(NULL, font_set->info, 1);
    font_set->info = NULL;

    if (font_set->font->min_byte1 || font_set->font->max_byte1)
	font_set->is_xchar2b = True;
    else
	font_set->is_xchar2b = False;

    return True;
}

int
_XomConvert(oc, conv, from, from_left, to, to_left, args, num_args)
    XOC oc;
    XlcConv conv;
    XPointer *from;
    int *from_left;
    XPointer *to;
    int *to_left;
    XPointer *args;
    int num_args;
{
    XPointer cs, lc_args[1];
    XlcCharSet charset;
    int length, cs_left, ret;
    FontSet font_set;
#ifdef MUSTCOPY
    XChar2b *xchar2b;
    char *buf, buf_local[BUFSIZ];
#endif
    
    cs = *to;
    cs_left = *to_left;
    lc_args[0] = (XPointer) &charset;

    ret = _XlcConvert(conv, from, from_left, &cs, &cs_left, lc_args, 1);
    if (ret < 0)
	return -1;

    font_set = _XomGetFontSetFromCharSet(oc, charset);
    if (font_set == NULL)
	return -1;

    if (font_set->font == NULL && load_font(oc, font_set) == False)
	return -1;

    length = *to_left - cs_left;

#ifdef MUSTCOPY
    if (font_set->is_xchar2b) {
	buf = (length > BUFSIZ) ? Xmalloc(length) : buf_local;
	if (buf == NULL)
	    return -1;
	memcpy(buf, (char *) *to, length);

	xchar2b = (XChar2b *) *to;
	length >>= 1;

	if (font_set->side == charset->side)
	    cs_to_xchar2b(buf, xchar2b, length);
	else if (font_set->side == XlcGL)
	    cs_to_xchar2b_gl(buf, xchar2b, length);
	else if (font_set->side == XlcGR)
	    cs_to_xchar2b_gr(buf, xchar2b, length);
	else
	    cs_to_xchar2b(buf, xchar2b, length);
	
	if (buf != buf_local)
	    Xfree(buf);

	*to = (XPointer) (xchar2b + length);
	*to_left -= length;
    } else
#endif
    {
	if (font_set->side != charset->side) {
	    if (font_set->side == XlcGL)
		shift_to_gl(*to, length);
	    else if (font_set->side == XlcGR)
		shift_to_gr(*to, length);
	}

	if (font_set->is_xchar2b)
	    length >>= 1;

	*to = cs;
	*to_left -= length;
    }

    *((XFontStruct **) args[0]) = font_set->font;
    *((Bool *) args[1]) = font_set->is_xchar2b;

    return ret;
}

XlcConv
_XomInitConverter(oc, type)
    XOC oc;
    XOMTextType type;
{
    XOCGenericPart *gen = XOC_GENERIC(oc);
    XlcConv conv;
    char *conv_type;
    XLCd lcd;

    if (type == XOMWideChar) {
	conv = gen->wcs_to_cs;
	conv_type = XlcNWideChar;
    } else {
	conv = gen->mbs_to_cs;
	conv_type = XlcNMultiByte;
    }

    if (conv) {
	_XlcResetConverter(conv);
	return conv;
    }

    lcd = oc->core.om->core.lcd;

    conv = _XlcOpenConverter(lcd, conv_type, lcd, XlcNCharSet);
    if (conv == (XlcConv) NULL)
	return (XlcConv) NULL;

    if (type == XOMWideChar)
	gen->wcs_to_cs = conv;
    else
	gen->mbs_to_cs = conv;

    return conv;
}
