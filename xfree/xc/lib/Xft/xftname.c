/*
 * $XFree86: xc/lib/Xft/xftname.c,v 1.5 2000/12/14 23:03:56 keithp Exp $
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

typedef struct _XftObjectType {
    const char	*object;
    XftType	type;
} XftObjectType;

const XftObjectType _XftObjectTypes[] = {
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
_XftNameConvert (XftType type, char *string)
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
    default:
	break;
    }
    return v;
}

static const char *
_XftNameFindNext (const char *cur, const char *delim, char *save, char *last)
{
    char    c;
    
    while (*cur && !strchr (delim, *cur))
    {
	c = *cur++;
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
	name = _XftNameFindNext (name, "=-:", save, &delim);
	if (save[0])
	{
	    if (delim == '=' || delim == '-')
	    {
		t = XftNameGetType (save);
		for (;;)
		{
		    name = _XftNameFindNext (name, ":,", save, &delim);
		    if (save[0] && t)
		    {
			v = _XftNameConvert (t->type, save);
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
