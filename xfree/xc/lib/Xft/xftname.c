/*
 * $XFree86: xc/lib/Xft/xftname.c,v 1.10 2001/03/30 18:50:18 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "xftint.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct _XftObjectType {
    const char	*object;
    XftType	type;
} XftObjectType;

static const XftObjectType _XftObjectTypes[] = {
    { XFT_FAMILY,	XftTypeString, },
    { XFT_STYLE,	XftTypeString, },
    { XFT_SLANT,	XftTypeInteger, },
    { XFT_WEIGHT,	XftTypeInteger, },
    { XFT_SIZE,		XftTypeDouble, },
    { XFT_PIXEL_SIZE,	XftTypeDouble, },
    { XFT_ENCODING,	XftTypeString, },
    { XFT_SPACING,	XftTypeInteger, },
    { XFT_FOUNDRY,	XftTypeString, },
    { XFT_CORE,		XftTypeBool, },
    { XFT_ANTIALIAS,	XftTypeBool, },
    { XFT_XLFD,		XftTypeString, },
    { XFT_FILE,		XftTypeString, },
    { XFT_INDEX,	XftTypeInteger, },
    { XFT_RASTERIZER,	XftTypeString, },
    { XFT_OUTLINE,	XftTypeBool, },
    { XFT_SCALABLE,	XftTypeBool, },
    { XFT_RGBA,		XftTypeInteger, },
    { XFT_SCALE,	XftTypeDouble, },
    { XFT_RENDER,	XftTypeBool, },
    { XFT_MINSPACE,	XftTypeBool, },
    { XFT_CHAR_WIDTH,	XftTypeInteger },
    { XFT_CHAR_HEIGHT,	XftTypeInteger },
    { XFT_MATRIX,	XftTypeMatrix },
};

#define NUM_OBJECT_TYPES    (sizeof _XftObjectTypes / sizeof _XftObjectTypes[0])

static const XftObjectType *
XftNameGetType (const char *object)
{
    int	    i;
    
    for (i = 0; i < NUM_OBJECT_TYPES; i++)
    {
	if (!_XftStrCmpIgnoreCase (object, _XftObjectTypes[i].object))
	    return &_XftObjectTypes[i];
    }
    return 0;
}

typedef struct _XftConstant {
    const char  *name;
    const char	*object;
    int		value;
} XftConstant;

static XftConstant XftConstants[] = {
    { "light",		"weight",   XFT_WEIGHT_LIGHT, },
    { "medium",		"weight",   XFT_WEIGHT_MEDIUM, },
    { "demibold",	"weight",   XFT_WEIGHT_DEMIBOLD, },
    { "bold",		"weight",   XFT_WEIGHT_BOLD, },
    { "black",		"weight",   XFT_WEIGHT_BLACK, },

    { "roman",		"slant",    XFT_SLANT_ROMAN, },
    { "italic",		"slant",    XFT_SLANT_ITALIC, },
    { "oblique",	"slant",    XFT_SLANT_OBLIQUE, },

    { "proportional",	"spacing",  XFT_PROPORTIONAL, },
    { "mono",		"spacing",  XFT_MONO, },
    { "charcell",	"spacing",  XFT_CHARCELL, },

    { "rgb",		"rgba",	    XFT_RGBA_RGB, },
    { "bgr",		"rgba",	    XFT_RGBA_BGR, },
    { "vrgb",		"rgba",	    XFT_RGBA_VRGB },
    { "vbgr",		"rgba",	    XFT_RGBA_VBGR },
};

#define NUM_XFT_CONSTANTS   (sizeof XftConstants/sizeof XftConstants[0])

static XftConstant *
_XftNameConstantLookup (char *string)
{
    int	i;
    
    for (i = 0; i < NUM_XFT_CONSTANTS; i++)
	if (!_XftStrCmpIgnoreCase (string, XftConstants[i].name))
	    return &XftConstants[i];
    return 0;
}

Bool
XftNameConstant (char *string, int *result)
{
    XftConstant	*c;

    if ((c = _XftNameConstantLookup(string)))
    {
	*result = c->value;
	return True;
    }
    return False;
}

static XftValue
_XftNameConvert (XftType type, char *string, XftMatrix *m)
{
    XftValue	v;

    v.type = type;
    switch (v.type) {
    case XftTypeInteger:
	if (!XftNameConstant (string, &v.u.i))
	    v.u.i = atoi (string);
	break;
    case XftTypeString:
	v.u.s = string;
	break;
    case XftTypeBool:
	v.u.b = XftDefaultParseBool (string);
	break;
    case XftTypeDouble:
	v.u.d = strtod (string, 0);
	break;
    case XftTypeMatrix:
	v.u.m = m;
	sscanf (string, "%lg %lg %lg %lg", &m->xx, &m->xy, &m->yx, &m->yy);
	break;
    default:
	break;
    }
    return v;
}

static const char *
_XftNameFindNext (const char *cur, const char *delim, char *save, char *last)
{
    char    c;
    
    while ((c = *cur))
    {
	if (c == '\\')
	{
	    ++cur;
	    if (!(c = *cur))
		break;
	}
	else if (strchr (delim, c))
	    break;
	++cur;
	*save++ = c;
    }
    *save = 0;
    *last = *cur;
    if (*cur)
	cur++;
    return cur;
}

XftPattern *
XftNameParse (const char *name)
{
    char		*save;
    XftPattern		*pat;
    double		d;
    char		*e;
    char		delim;
    XftValue		v;
    XftMatrix		m;
    const XftObjectType	*t;
    XftConstant		*c;

    save = malloc (strlen (name) + 1);
    if (!save)
	goto bail0;
    pat = XftPatternCreate ();
    if (!pat)
	goto bail1;

    for (;;)
    {
	name = _XftNameFindNext (name, "-,:", save, &delim);
	if (save[0])
	{
	    if (!XftPatternAddString (pat, XFT_FAMILY, save))
		goto bail2;
	}
	if (delim != ',')
	    break;
    }
    if (delim == '-')
    {
	for (;;)
	{
	    name = _XftNameFindNext (name, "-,:", save, &delim);
	    d = strtod (save, &e);
	    if (e != save)
	    {
		if (!XftPatternAddDouble (pat, XFT_SIZE, d))
		    goto bail2;
	    }
	    if (delim != ',')
		break;
	}
    }
    while (delim == ':')
    {
	name = _XftNameFindNext (name, "=_:", save, &delim);
	if (save[0])
	{
	    if (delim == '=' || delim == '_')
	    {
		t = XftNameGetType (save);
		for (;;)
		{
		    name = _XftNameFindNext (name, ":,", save, &delim);
		    if (save[0] && t)
		    {
			v = _XftNameConvert (t->type, save, &m);
			if (!XftPatternAdd (pat, t->object, v, True))
			    goto bail2;
		    }
		    if (delim != ',')
			break;
		}
	    }
	    else
	    {
		if ((c = _XftNameConstantLookup (save)))
		{
		    if (!XftPatternAddInteger (pat, c->object, c->value))
			goto bail2;
		}
	    }
	}
    }

    free (save);
    return pat;

bail2:
    XftPatternDestroy (pat);
bail1:
    free (save);
bail0:
    return 0;
}

static Bool
_XftNameUnparseString (const char *string, char *escape, char **destp, int *lenp)
{
    int	    len = *lenp;
    char    *dest = *destp;
    char    c;

    while ((c = *string++))
    {
	if (escape && strchr (escape, c))
	{
	    if (len-- == 0)
		return False;
	    *dest++ = escape[0];
	}
	if (len-- == 0)
	    return False;
	*dest++ = c;
    }
    *destp = dest;
    *lenp = len;
    return True;
}

static Bool
_XftNameUnparseValue (XftValue v, char *escape, char **destp, int *lenp)
{
    char    temp[1024];
    
    switch (v.type) {
    case XftTypeVoid:
	return True;
    case XftTypeInteger:
	sprintf (temp, "%d", v.u.i);
	return _XftNameUnparseString (temp, 0, destp, lenp);
    case XftTypeDouble:
	sprintf (temp, "%g", v.u.d);
	return _XftNameUnparseString (temp, 0, destp, lenp);
    case XftTypeString:
	return _XftNameUnparseString (v.u.s, escape, destp, lenp);
    case XftTypeBool:
	return _XftNameUnparseString (v.u.b ? "True" : "False", 0, destp, lenp);
    case XftTypeMatrix:
	sprintf (temp, "%g %g %g %g", 
		 v.u.m->xx, v.u.m->xy, v.u.m->yx, v.u.m->yy);
	return _XftNameUnparseString (temp, 0, destp, lenp);
    }
    return False;
}

static Bool
_XftNameUnparseValueList (XftValueList *v, char *escape, char **destp, int *lenp)
{
    while (v)
    {
	if (!_XftNameUnparseValue (v->value, escape, destp, lenp))
	    return False;
	if ((v = v->next))
	    if (!_XftNameUnparseString (",", 0, destp, lenp))
		return False;
    }
    return True;
}

#define XFT_ESCAPE_FIXED    "\\-:,"
#define XFT_ESCAPE_VARIABLE "\\=_:,"

Bool
XftNameUnparse (XftPattern *pat, char *dest, int len)
{
    int			i;
    XftPatternElt	*e;
    const XftObjectType *o;

    e = XftPatternFind (pat, XFT_FAMILY, False);
    if (e)
    {
	if (!_XftNameUnparseValueList (e->values, XFT_ESCAPE_FIXED,
				       &dest, &len))
	    return False;
    }
    e = XftPatternFind (pat, XFT_SIZE, False);
    if (e)
    {
	if (!_XftNameUnparseString ("-", 0, &dest, &len))
	    return False;
	if (!_XftNameUnparseValueList (e->values, XFT_ESCAPE_FIXED, &dest, &len))
	    return False;
    }
    for (i = 0; i < NUM_OBJECT_TYPES; i++)
    {
	o = &_XftObjectTypes[i];
	if (!strcmp (o->object, XFT_FAMILY) || 
	    !strcmp (o->object, XFT_SIZE) ||
	    !strcmp (o->object, XFT_FILE))
	    continue;
	
	e = XftPatternFind (pat, o->object, False);
	if (e)
	{
	    if (!_XftNameUnparseString (":", 0, &dest, &len))
		return False;
	    if (!_XftNameUnparseString (o->object, XFT_ESCAPE_VARIABLE, 
					&dest, &len))
		return False;
	    if (!_XftNameUnparseString ("=", 0, &dest, &len))
		return False;
	    if (!_XftNameUnparseValueList (e->values, XFT_ESCAPE_VARIABLE, 
					   &dest, &len))
		return False;
	}
    }
    if (len == 0)
	return False;
    *dest = '\0';
    return True;
}
