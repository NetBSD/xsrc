/* $XConsortium: omGeneric.c /main/14 1996/02/01 06:31:46 kaleb $ */
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
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>

#define MAXFONTS		100

extern int _XmbDefaultTextEscapement(), _XwcDefaultTextEscapement();
extern int _XmbDefaultTextExtents(), _XwcDefaultTextExtents();
extern Status _XmbDefaultTextPerCharExtents(), _XwcDefaultTextPerCharExtents();
extern int _XmbDefaultDrawString(), _XwcDefaultDrawString();
extern void _XmbDefaultDrawImageString(), _XwcDefaultDrawImageString();

extern int _XmbGenericTextEscapement(), _XwcGenericTextEscapement();
extern int _XmbGenericTextExtents(), _XwcGenericTextExtents();
extern Status _XmbGenericTextPerCharExtents(), _XwcGenericTextPerCharExtents();
extern int _XmbGenericDrawString(), _XwcGenericDrawString();
extern void _XmbGenericDrawImageString(), _XwcGenericDrawImageString();

static Bool
init_fontset(oc)
    XOC oc;
{
    XOCGenericPart *gen;
    FontSet font_set;
    OMData data;
    int count;

    count = XOM_GENERIC(oc->core.om)->data_num;
    data = XOM_GENERIC(oc->core.om)->data;

    font_set = (FontSet) Xmalloc(sizeof(FontSetRec) * count);
    if (font_set == NULL)
	return False;
    bzero((char *) font_set, sizeof(FontSetRec) * count);

    gen = XOC_GENERIC(oc);
    gen->font_set_num = count;
    gen->font_set = font_set;

    for ( ; count-- > 0; data++, font_set++) {
	font_set->charset_count = data->charset_count;
	font_set->charset_list = data->charset_list;
	font_set->font_data_count = data->font_data_count;
	font_set->font_data = data->font_data;
    }

    return True;
}

static char *
get_prop_name(dpy, fs)
    Display *dpy;
    XFontStruct	*fs;
{
    unsigned long fp;

    if (XGetFontProperty(fs, XA_FONT, &fp))
	return XGetAtomName(dpy, fp); 

    return (char *) NULL;
}

static FontData
check_charset(font_set, font_name)
    FontSet font_set;
    char *font_name;
{
    FontData font_data;
    char *last;
    int count, length, name_len;

    name_len = strlen(font_name);
    last = font_name + name_len;

    count = font_set->font_data_count;
    font_data = font_set->font_data;
    for ( ; count-- > 0; font_data++) {
	length = strlen(font_data->name);
	if (length > name_len)
	    continue;
	
	if (_XlcCompareISOLatin1(last - length, font_data->name) == 0)
	    return font_data;
    }

    return (FontData) NULL;
}

static int
check_fontname(oc, name, found_num)
    XOC oc;
    char *name;
    int found_num;
{
    Display *dpy = oc->core.om->core.display;
    XOCGenericPart *gen = XOC_GENERIC(oc);
    FontData data;
    FontSet font_set;
    XFontStruct *fs_list;
    char **fn_list, *fname, *prop_fname = NULL;
    int list_num, font_set_num, i;
    int list2_num;
    char **fn2_list = NULL;

    fn_list = XListFonts(dpy, name, MAXFONTS, &list_num);
    if (fn_list == NULL)
	return found_num;

    for (i = 0; i < list_num; i++) {
	fname = fn_list[i];

	font_set = gen->font_set;
	font_set_num = gen->font_set_num;

	for ( ; font_set_num-- > 0; font_set++) {
	    if (font_set->font_name)
		continue;

	    if ((data = check_charset(font_set, fname)) == NULL) {
		if ((fn2_list = XListFontsWithInfo(dpy, name, MAXFONTS,
					      &list2_num, &fs_list))
		    && (prop_fname = get_prop_name(dpy, fs_list))
		    && (data = check_charset(font_set, prop_fname)))
		    fname = prop_fname;
	    }
	    if (data) {
		font_set->side = data->side;
		font_set->font_name = (char *) Xmalloc(strlen(fname) + 1);
		if (font_set->font_name) {
		    strcpy(font_set->font_name, fname);
		    found_num++;
		}
	    }
	    if (fn2_list) {
		XFreeFontInfo(fn2_list, fs_list, list2_num);
		fn2_list = NULL;
		if (prop_fname) {
		    Xfree(prop_fname);
		    prop_fname = NULL;
		}
	    }
	    if (found_num == gen->font_set_num)
		break;
	}
    }
    XFreeFontNames(fn_list);
    return found_num;
}

static Bool
load_font(oc)
    XOC oc;
{
    Display *dpy = oc->core.om->core.display;
    XOCGenericPart *gen = XOC_GENERIC(oc);
    FontSet font_set = gen->font_set;
    int num = gen->font_set_num;

    for ( ; num-- > 0; font_set++) {
	if (font_set->font_name == NULL)
	    continue;

	if (font_set->font == NULL) {
	    font_set->font = XLoadQueryFont(dpy, font_set->font_name);
	    if (font_set->font == NULL)
		return False;
	    
	    if (font_set->font->min_byte1 || font_set->font->max_byte1)
		font_set->is_xchar2b = True;
	    else
		font_set->is_xchar2b = False;
	}
    }

    return True;
}

static Bool
load_font_info(oc)
    XOC oc;
{
    Display *dpy = oc->core.om->core.display;
    XOCGenericPart *gen = XOC_GENERIC(oc);
    FontSet font_set = gen->font_set;
    char **fn_list;
    int fn_num, num = gen->font_set_num;

    for ( ; num-- > 0; font_set++) {
	if (font_set->font_name == NULL)
	    continue;

	if (font_set->info == NULL) {
	    fn_list = XListFontsWithInfo(dpy, font_set->font_name, 1, &fn_num,
					 &font_set->info);
	    if (font_set->info == NULL)
		return False;
	    
	    XFreeFontNames(fn_list);
	}
    }

    return True;
}

static void
set_fontset_extents(oc)
    XOC oc;
{
    XRectangle *ink = &oc->core.font_set_extents.max_ink_extent;
    XRectangle *logical = &oc->core.font_set_extents.max_logical_extent;
    XFontStruct **font_list, *font;
    XCharStruct overall;
    int logical_ascent, logical_descent;
    int	num = oc->core.font_info.num_font;

    font_list = oc->core.font_info.font_struct_list;
    font = *font_list++;
    overall = font->max_bounds;
    overall.lbearing = font->min_bounds.lbearing;
    logical_ascent = font->ascent;
    logical_descent = font->descent;

    while (--num > 0) {
	font = *font_list++;
	overall.lbearing = min(overall.lbearing, font->min_bounds.lbearing);
	overall.rbearing = max(overall.rbearing, font->max_bounds.rbearing);
	overall.ascent = max(overall.ascent, font->max_bounds.ascent);
	overall.descent = max(overall.descent, font->max_bounds.descent);
	overall.width = max(overall.width, font->max_bounds.width);
	logical_ascent = max(logical_ascent, font->ascent);
	logical_descent = max(logical_descent, font->descent);
    }

    ink->x = overall.lbearing;
    ink->y = -(overall.ascent);
    ink->width = overall.rbearing - overall.lbearing;
    ink->height = overall.ascent + overall.descent;

    logical->x = 0;
    logical->y = -(logical_ascent);
    logical->width = overall.width;
    logical->height = logical_ascent + logical_descent;
}

static Bool
init_core_part(oc)
    XOC oc;
{
    XOCGenericPart *gen = XOC_GENERIC(oc);
    FontSet font_set;
    int font_set_num;
    XFontStruct **font_struct_list;
    char **font_name_list, *font_name_buf;
    int	count, length;

    font_set = gen->font_set;
    font_set_num = gen->font_set_num;
    count = length = 0;

    for ( ; font_set_num-- > 0; font_set++) {
	if (font_set->font_name == NULL)
	    continue;

	length += strlen(font_set->font_name) + 1;
	count++;
    }
    if (count == 0)
        return False;

    font_struct_list = (XFontStruct **) Xmalloc(sizeof(XFontStruct *) * count);
    if (font_struct_list == NULL)
	return False;

    font_name_list = (char **) Xmalloc(sizeof(char *) * count);
    if (font_name_list == NULL)
	goto err;

    font_name_buf = (char *) Xmalloc(length);
    if (font_name_buf == NULL)
	goto err;

    oc->core.font_info.num_font = count;
    oc->core.font_info.font_name_list = font_name_list;
    oc->core.font_info.font_struct_list = font_struct_list;

    font_set = gen->font_set;
    font_set_num = gen->font_set_num;

    for (count = 0; font_set_num-- > 0; font_set++, count++) {
	if (font_set->font_name == NULL)
	    continue;

	font_set->id = count;
	if (font_set->font)
	    *font_struct_list++ = font_set->font;
	else
	    *font_struct_list++ = font_set->info;
	strcpy(font_name_buf, font_set->font_name);
	Xfree(font_set->font_name);
	*font_name_list++ = font_set->font_name = font_name_buf;
	font_name_buf += strlen(font_name_buf) + 1;
    }

    set_fontset_extents(oc);

    return True;

err:
    if (font_name_list)
	Xfree(font_name_list);
    Xfree(font_struct_list);

    return False;
}

static char *
get_font_name(oc, pattern)
    XOC oc;
    char *pattern;
{
    char **list, *name;
    int count;

    list = XListFonts(oc->core.om->core.display, pattern, 1, &count);
    if (list == NULL)
	return NULL;

    name = (char *) Xmalloc(strlen(*list) + 1);
    if (name)
	strcpy(name, *list);
    
    XFreeFontNames(list);

    return name;
}

static int
parse_fontname(oc)
    XOC oc;
{
    XOCGenericPart *gen = XOC_GENERIC(oc);
    FontSet font_set;
    FontData font_data;
    char *pattern, *last, buf[BUFSIZ];
    int font_set_num, font_data_count, length, found_num = 0;
    Bool is_found;
    int count, num_fields;
    char *base_name, *font_name, **name_list, **cur_name_list;
    char *charset_p;
    Bool append_charset;
    /*
       append_charset flag should be set to True when the XLFD fontname
       doesn't contain a chaset part.
     */

    name_list = _XParseBaseFontNameList(oc->core.base_name_list, &count);
    if (name_list == NULL)
	return -1;
    cur_name_list = name_list;

    while (count-- > 0) {
        pattern = *cur_name_list++;
	if (pattern == NULL || *pattern == '\0')
	    continue;

 	is_found = False;
	append_charset = False;

	if (strchr(pattern, '*') == NULL &&
	    (font_name = get_font_name(oc, pattern))) {

	    font_set = gen->font_set;
	    font_set_num = gen->font_set_num;

	    for ( ; font_set_num-- > 0; font_set++) {
 		if (font_set->font_name)
 		    continue;
		font_data = check_charset(font_set, font_name);
		if (font_data == NULL) {
		    Display *dpy = oc->core.om->core.display;
		    char **fn_list = NULL, *prop_fname = NULL;
		    int list_num;
		    XFontStruct *fs_list;
		    if ((fn_list = XListFontsWithInfo(dpy, font_name,
						      MAXFONTS,
						      &list_num, &fs_list))
			&& (prop_fname = get_prop_name(dpy, fs_list))
			&& (font_data = check_charset(font_set, prop_fname))) {
			if (fn_list) {
			    XFreeFontInfo(fn_list, fs_list, list_num);
			    fn_list = NULL;
			}
			font_name = prop_fname;
		    }
		}
		if (font_data == NULL)
 		    continue;

		font_set->side = font_data->side;
		font_set->font_name = (char *) Xmalloc(strlen(font_name) + 1);
		if (font_set->font_name == NULL) {
		    Xfree(font_name);
		    goto err;
		}
		strcpy(font_set->font_name, font_name);

		found_num++;
		is_found = True;
	    }

	    Xfree(font_name);

	    if (is_found)
		continue;
	}

/*
 * The BaseFontSetList didn't specify explicit fonts. Now it's necessary 
 * to try to find a font by using the required charsets (specified in the 
 * locale database) in conjunction with the BaseFontName. See Section 13.3 
 * of the Xlib specification.
 */
	strcpy(buf, pattern);
	length = strlen(buf);
	last = buf + length - 1;
	for (num_fields = 0, base_name = buf; *base_name != '\0'; base_name++)
	    if (*base_name == '-') num_fields++;
	if (strchr(pattern, '*') == NULL) {
	    if (num_fields == 12) {
		append_charset = True;
		*++last = '-';
		last++;
	    } else
		continue;
	} else {
	    if (num_fields == 13 || num_fields == 14) {
	    /* 
	     * There are 14 fields in an XLFD name -- make certain the
	     * charset (& encoding) is placed in the correct field.
	     */
		append_charset = True;
		last = strrchr (buf, '-');
		if (num_fields == 14) {
		    *last = '\0';
		    last = strrchr (buf, '-');
		}
		last++;
	    } else if (*last == '*') {
		append_charset = True;
		*++last = '-';
		last++;
	    } else {
		last = strrchr (buf, '-');
		charset_p = last;
		charset_p = strrchr (buf, '-');
		while (*(--charset_p) != '-');
		charset_p++;
	    }
	}
	font_set = gen->font_set;
	font_set_num = gen->font_set_num;

	for ( ; font_set_num-- > 0; font_set++) {
	    if (font_set->font_name)
		continue;
	    
	    font_data = font_set->font_data;
	    font_data_count = font_set->font_data_count;

	    for ( ; font_data_count-- > 0; font_data++) {
		if (append_charset)
		    strcpy(last, font_data->name);
		else {
		    if (_XlcCompareISOLatin1(charset_p,
					     font_data->name)) {
			continue;
		    }
		}
		if ((font_set->font_name = get_font_name(oc, buf)))
		    break;
	    }

	    if (font_set->font_name == NULL)
		continue;

	    font_set->side = font_data->side;
 
	    found_num++;
	    is_found = True;
	}

	if (found_num == gen->font_set_num)
	    break;
	if (is_found == True)
	    continue;
 
	found_num = check_fontname(oc, pattern, found_num);
	if (found_num == gen->font_set_num)
	    break;
    }

    base_name = (char *) Xmalloc(strlen(oc->core.base_name_list) + 1);
    if (base_name == NULL)
	goto err;

    strcpy(base_name, oc->core.base_name_list);
    oc->core.base_name_list = base_name;

    XFreeStringList(name_list);		

    return found_num;

err:
    XFreeStringList(name_list);		

    return -1;
}

static Bool
set_missing_list(oc)
    XOC oc;
{
    XOCGenericPart *gen = XOC_GENERIC(oc);
    FontSet font_set;
    char **charset_list, *charset_buf;
    int	count, length, font_set_num;

    font_set = gen->font_set;
    font_set_num = gen->font_set_num;
    count = length = 0;

    for ( ; font_set_num-- > 0; font_set++) {
	if (font_set->info || font_set->font) 
	    continue;
	
	length += strlen(font_set->font_data->name) + 1;
	count++;
    }

    if (count < 1)
	return True;

    charset_list = (char **) Xmalloc(sizeof(char *) * count);
    if (charset_list == NULL)
	return False;

    charset_buf = (char *) Xmalloc(length);
    if (charset_buf == NULL) {
	Xfree(charset_list);
	return False;
    }

    oc->core.missing_list.charset_list = charset_list;
    oc->core.missing_list.charset_count = count;

    font_set = gen->font_set;
    font_set_num = gen->font_set_num;

    for ( ; font_set_num-- > 0; font_set++) {
	if (font_set->info || font_set->font) 
	    continue;

	strcpy(charset_buf, font_set->font_data->name);
	*charset_list++ = charset_buf;
	charset_buf += strlen(charset_buf) + 1;
    } 

    return True;
}

static Bool
create_fontset(oc)
    XOC oc;
{
    XOMGenericPart *gen = XOM_GENERIC(oc->core.om);
    int found_num;

    if (init_fontset(oc) == False)
        return False;

    found_num = parse_fontname(oc);
    if (found_num <= 0) {
	if (found_num == 0)
	    set_missing_list(oc);
	return False;
    }

    if (gen->on_demand_loading == True) {
	if (load_font_info(oc) == False)
	    return False;
    } else {
	if (load_font(oc) == False)
	    return False;
    }

    if (init_core_part(oc) == False)
	return False;

    if (set_missing_list(oc) == False)
	return False;

    return True;
}

static void
destroy_oc(oc)
    XOC oc;
{
    Display *dpy = oc->core.om->core.display;
    XOCGenericPart *gen = XOC_GENERIC(oc);
    XFontStruct **font_list, *font;
    int count;

    if (gen->mbs_to_cs)
	_XlcCloseConverter(gen->mbs_to_cs);

    if (gen->wcs_to_cs)
	_XlcCloseConverter(gen->wcs_to_cs);

    if (gen->font_set)
	Xfree(gen->font_set);

    if (oc->core.base_name_list)
	Xfree(oc->core.base_name_list);

    if (oc->core.font_info.font_name_list)
	XFreeStringList(oc->core.font_info.font_name_list);

    if (font_list = oc->core.font_info.font_struct_list) {
	for (count = oc->core.font_info.num_font; count-- > 0; font_list++) {
	    if (font = *font_list) {
		if (font->fid)
		    XFreeFont(dpy, font);
		else
		    XFreeFontInfo(NULL, font, 1);
	    }
	}
	Xfree(oc->core.font_info.font_struct_list);
    }

    if (oc->core.missing_list.charset_list)
	XFreeStringList(oc->core.missing_list.charset_list);

#ifdef notdef
    if (oc->core.res_name)
	Xfree(oc->core.res_name);
    if (oc->core.res_class)
	Xfree(oc->core.res_class);
#endif
    
    Xfree(oc);
}

static char *
set_oc_values(oc, args, num_args)
    XOC oc;
    XlcArgList args;
    int num_args;
{
    if (oc->core.resources == NULL)
	return NULL;

    return _XlcSetValues((XPointer) oc, oc->core.resources,
			 oc->core.num_resources, args, num_args, XlcSetMask);
}

static char *
get_oc_values(oc, args, num_args)
    XOC oc;
    XlcArgList args;
    int num_args;
{
    if (oc->core.resources == NULL)
	return NULL;

    return _XlcGetValues((XPointer) oc, oc->core.resources,
			 oc->core.num_resources, args, num_args, XlcGetMask);
}

static XOCMethodsRec oc_default_methods = {
    destroy_oc,
    set_oc_values,
    get_oc_values,
    _XmbDefaultTextEscapement,
    _XmbDefaultTextExtents,
    _XmbDefaultTextPerCharExtents,
    _XmbDefaultDrawString,
    _XmbDefaultDrawImageString,
    _XwcDefaultTextEscapement,
    _XwcDefaultTextExtents,
    _XwcDefaultTextPerCharExtents,
    _XwcDefaultDrawString,
    _XwcDefaultDrawImageString
};

static XOCMethodsRec oc_generic_methods = {
    destroy_oc,
    set_oc_values,
    get_oc_values,
    _XmbGenericTextEscapement,
    _XmbGenericTextExtents,
    _XmbGenericTextPerCharExtents,
    _XmbGenericDrawString,
    _XmbGenericDrawImageString,
    _XwcGenericTextEscapement,
    _XwcGenericTextExtents,
    _XwcGenericTextPerCharExtents,
    _XwcGenericDrawString,
    _XwcGenericDrawImageString
};

typedef struct _XOCMethodsListRec {
    char *name;
    XOCMethods methods;
} XOCMethodsListRec, *XOCMethodsList;

static XOCMethodsListRec oc_methods_list[] = {
    { "default", &oc_default_methods },
    { "generic", &oc_generic_methods }
};

static XlcResource oc_resources[] = {
    { XNBaseFontName, NULLQUARK, sizeof(char *),
      XOffsetOf(XOCRec, core.base_name_list), XlcCreateMask | XlcGetMask },
    { XNOMAutomatic, NULLQUARK, sizeof(Bool),
      XOffsetOf(XOCRec, core.om_automatic), XlcGetMask },
    { XNMissingCharSet, NULLQUARK, sizeof(XOMCharSetList),
      XOffsetOf(XOCRec, core.missing_list), XlcGetMask },
    { XNDefaultString, NULLQUARK, sizeof(char *),
      XOffsetOf(XOCRec, core.default_string), XlcGetMask },
    { XNOrientation, NULLQUARK, sizeof(XOrientation),
      XOffsetOf(XOCRec, core.orientation), XlcSetMask | XlcGetMask },
    { XNResourceName, NULLQUARK, sizeof(char *),
      XOffsetOf(XOCRec, core.res_name), XlcSetMask | XlcGetMask },
    { XNResourceClass, NULLQUARK, sizeof(char *),
      XOffsetOf(XOCRec, core.res_class), XlcSetMask | XlcGetMask },
    { XNFontInfo, NULLQUARK, sizeof(XOMFontInfo),
      XOffsetOf(XOCRec, core.font_info), XlcGetMask }
};

static XOC
create_oc(om, args, num_args)
    XOM om;
    XlcArgList args;
    int num_args;
{
    XOC oc;
    XOMGenericPart *gen = XOM_GENERIC(om);
    XOCMethodsList methods_list = oc_methods_list;
    int count;

    oc = (XOC) Xmalloc(sizeof(XOCGenericRec));
    if (oc == NULL)
	return (XOC) NULL;
    bzero((char *) oc, sizeof(XOCGenericRec));
    
    oc->core.om = om;

    if (oc_resources[0].xrm_name == NULLQUARK)
	_XlcCompileResourceList(oc_resources, XlcNumber(oc_resources));
    
    if (_XlcSetValues((XPointer) oc, oc_resources, XlcNumber(oc_resources),
		      args, num_args, XlcCreateMask | XlcDefaultMask))
	goto err;

    if (oc->core.base_name_list == NULL)
	goto err;

    oc->core.resources = oc_resources;
    oc->core.num_resources = XlcNumber(oc_resources);

    if (create_fontset(oc) == False)
	goto err;

    oc->methods = &oc_generic_methods;

    if (gen->object_name) {
	count = XlcNumber(oc_methods_list);

	for ( ; count-- > 0; methods_list++) {
	    if (!_XlcCompareISOLatin1(gen->object_name, methods_list->name)) {
		oc->methods = methods_list->methods;
		break;
	    }
	}
    }

    return oc;

err:
    destroy_oc(oc);

    return (XOC) NULL;
}

static Status
close_om(om)
    XOM om;
{
    XOMGenericPart *gen = XOM_GENERIC(om);
    OMData data;
    int count;

    if (data = gen->data) {
	for (count = gen->data_num; count-- > 0; data++) {
	    if (data->charset_list)
		Xfree(data->charset_list);

	    if (data->font_data) {
		if (data->font_data->name)
		    Xfree(data->font_data->name);
		Xfree(data->font_data);
	    }
	}
	Xfree(gen->data);
    }

    if (gen->object_name)
	Xfree(gen->object_name);

    if (om->core.res_name)
	Xfree(om->core.res_name);
    if (om->core.res_class)
	Xfree(om->core.res_class);
    if (om->core.required_charset.charset_list &&
	om->core.required_charset.charset_count > 0)
	XFreeStringList(om->core.required_charset.charset_list);
    else
	Xfree((char*)om->core.required_charset.charset_list);
    if (om->core.orientation_list.orientation)
	Xfree(om->core.orientation_list.orientation);

    Xfree(om);

    return 1;
}

static char *
set_om_values(om, args, num_args)
    XOM om;
    XlcArgList args;
    int num_args;
{
    if (om->core.resources == NULL)
	return NULL;

    return _XlcSetValues((XPointer) om, om->core.resources,
			 om->core.num_resources, args, num_args, XlcSetMask);
}

static char *
get_om_values(om, args, num_args)
    XOM om;
    XlcArgList args;
    int num_args;
{
    if (om->core.resources == NULL)
	return NULL;

    return _XlcGetValues((XPointer) om, om->core.resources,
			 om->core.num_resources, args, num_args, XlcGetMask);
}

static XOMMethodsRec methods = {
    close_om,
    set_om_values,
    get_om_values,
    create_oc
};

static XlcResource om_resources[] = {
    { XNRequiredCharSet, NULLQUARK, sizeof(XOMCharSetList),
      XOffsetOf(XOMRec, core.required_charset), XlcGetMask },
    { XNQueryOrientation, NULLQUARK, sizeof(XOMOrientation),
      XOffsetOf(XOMRec, core.orientation_list), XlcGetMask },
    { XNDirectionalDependentDrawing, NULLQUARK, sizeof(Bool),
      XOffsetOf(XOMRec, core.directional_dependent), XlcGetMask },
    { XNContextualDrawing, NULLQUARK, sizeof(Bool),
      XOffsetOf(XOMRec, core.contextual_drawing), XlcGetMask }
};

static XOM
create_om(lcd, dpy, rdb, res_name, res_class)
    XLCd lcd;
    Display *dpy;
    XrmDatabase rdb;
    char *res_name;
    char *res_class;
{
    XOM om;

    om = (XOM) Xmalloc(sizeof(XOMGenericRec));
    if (om == NULL)
	return (XOM) NULL;
    bzero((char *) om, sizeof(XOMGenericRec));
    
    om->methods = &methods;
    om->core.lcd = lcd;
    om->core.display = dpy;
    om->core.rdb = rdb;
    if (res_name) {
	om->core.res_name = (char *) Xmalloc(strlen(res_name) + 1);
	if (om->core.res_name == NULL)
	    goto err;
	strcpy(om->core.res_name, res_name);
    }
    if (res_class) {
	om->core.res_class = (char *) Xmalloc(strlen(res_class) + 1);
	if (om->core.res_class == NULL)
	    goto err;
	strcpy(om->core.res_class, res_class);
    }

    if (om_resources[0].xrm_name == NULLQUARK)
	_XlcCompileResourceList(om_resources, XlcNumber(om_resources));
    
    om->core.resources = om_resources;
    om->core.num_resources = XlcNumber(om_resources);

    return om;

err:
    close_om(om);

    return (XOM) NULL;
}

static OMData
add_data(om)
    XOM om;
{
    XOMGenericPart *gen = XOM_GENERIC(om);
    OMData new;
    int num;

    if (num = gen->data_num)
        new = (OMData) Xrealloc(gen->data, (num + 1) * sizeof(OMDataRec));
    else
        new = (OMData) Xmalloc(sizeof(OMDataRec));

    if (new == NULL)
        return NULL;

    gen->data_num = num + 1;
    gen->data = new;

    new += num;
    bzero((char *) new, sizeof(OMDataRec));

    return new;
}

static Bool
init_om(om)
    XOM om;
{
    XLCd lcd = om->core.lcd;
    XOMGenericPart *gen = XOM_GENERIC(om);
    OMData data;
    XlcCharSet *charset_list;
    FontData font_data;
    char **required_list;
    XOrientation *orientation;
    char **value, buf[BUFSIZ], *bufptr;
    int count, num, length = 0;

    _XlcGetResource(lcd, "XLC_FONTSET", "on_demand_loading", &value, &count);
    if (count > 0 && _XlcCompareISOLatin1(*value, "True") == 0)
	gen->on_demand_loading = True;

    _XlcGetResource(lcd, "XLC_FONTSET", "object_name", &value, &count);
    if (count > 0) {
	gen->object_name = (char *) Xmalloc(strlen(*value) + 1);
	if (gen->object_name == NULL)
	    return False;
	strcpy(gen->object_name, *value);
    }

    for (num = 0; ; num++) {
	sprintf(buf, "fs%d.charset", num);
	_XlcGetResource(lcd, "XLC_FONTSET", buf, &value, &count);
	if (count < 1)
	    break;

	data = add_data(om);
	if (data == NULL)
	    return False;
	
	charset_list = (XlcCharSet *) Xmalloc(sizeof(XlcCharSet) * count);
	if (charset_list == NULL)
	    return False;
	data->charset_list = charset_list;
	data->charset_count = count;

	while (count-- > 0)
	    *charset_list++ = _XlcGetCharSet(*value++);

	sprintf(buf, "fs%d.font", num);
	_XlcGetResource(lcd, "XLC_FONTSET", buf, &value, &count);
	if (count < 1)
	    return False;

	font_data = (FontData) Xmalloc(sizeof(FontDataRec) * count);
	if (font_data == NULL)
	    return False;
	bzero((char *) font_data, sizeof(FontDataRec) * count);
	data->font_data = font_data;
	data->font_data_count = count;

	for ( ; count-- > 0; font_data++) {
	    strcpy(buf, *value++);
	    if (bufptr = strrchr(buf, ':'))
		*bufptr++ = '\0';
	    font_data->name = (char *) Xmalloc(strlen(buf) + 1);
	    if (font_data->name == NULL)
		return False;
	    strcpy(font_data->name, buf);
	    if (bufptr && _XlcCompareISOLatin1(bufptr, "GL") == 0)
		font_data->side = XlcGL;
	    else if (bufptr && _XlcCompareISOLatin1(bufptr, "GR") == 0)
		font_data->side = XlcGR;
	    else
		font_data->side = XlcGLGR;
	}

	length += strlen(data->font_data->name) + 1;
    }

    /* required charset list */
    required_list = (char **) Xmalloc(sizeof(char *) * gen->data_num);
    if (required_list == NULL)
	return False;

    bufptr = (char *) Xmalloc(length);
    if (bufptr == NULL) {
	Xfree(required_list);
	return False;
    }

    om->core.required_charset.charset_list = required_list;
    om->core.required_charset.charset_count = gen->data_num;

    count = gen->data_num;
    data = gen->data;

    for ( ; count-- > 0; data++) {
	strcpy(bufptr, data->font_data->name);
	*required_list++ = bufptr;
	bufptr += strlen(bufptr) + 1;
    }

    /* orientation list */
    orientation = (XOrientation *) Xmalloc(sizeof(XOrientation));
    if (orientation == NULL)
	return False;

    *orientation = XOMOrientation_LTR_TTB;
    om->core.orientation_list.orientation = orientation;
    om->core.orientation_list.num_orientation = 1;

    /* directional dependent drawing */
    om->core.directional_dependent = False;

    /* contexual drawing */
    om->core.contextual_drawing = False;

    /* context dependent */
    om->core.context_dependent = False;

    return True;
}

XOM
#if NeedFunctionPrototypes
_XomGenericOpenOM(XLCd lcd, Display *dpy, XrmDatabase rdb,
		  _Xconst char *res_name, _Xconst char *res_class)
#else
_XomGenericOpenOM(lcd, dpy, rdb, res_name, res_class)
    XLCd lcd;
    Display *dpy;
    XrmDatabase rdb;
    char *res_name;
    char *res_class;
#endif
{
    XOM om;

    om = create_om(lcd, dpy, rdb, res_name, res_class);
    if (om == NULL)
	return (XOM) NULL;
    
    if (init_om(om) == False)
	goto err;

    return om;

err:
    close_om(om);

    return (XOM) NULL;
}

Bool
_XInitOM(lcd)
    XLCd lcd;
{
    lcd->methods->open_om = _XomGenericOpenOM;

    return True;
}
