/*
 * $XFree86: xc/lib/Xft/xftlist.c,v 1.2 2000/12/07 23:57:28 keithp Exp $
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

#include <stdlib.h>
#include "xftint.h"

XftObjectSet *
XftObjectSetCreate (void)
{
    XftObjectSet    *os;

    os = (XftObjectSet *) malloc (sizeof (XftObjectSet));
    if (!os)
	return 0;
    os->nobject = 0;
    os->sobject = 0;
    os->objects = 0;
    return os;
}

Bool
XftObjectSetAdd (XftObjectSet *os, const char *object)
{
    int		s;
    const char	**objects;
    
    if (os->nobject == os->sobject)
    {
	s = os->sobject + 4;
	if (os->objects)
	    objects = (const char **) realloc ((void *) os->objects,
					       s * sizeof (const char **));
	else
	    objects = (const char **) malloc (s * sizeof (const char **));
	if (!objects)
	    return False;
	os->objects = objects;
	os->sobject = s;
    }
    os->objects[os->nobject++] = object;
    return True;
}

void
XftObjectSetDestroy (XftObjectSet *os)
{
    if (os->objects)
	free ((void *) os->objects);
    free (os);
}

#define _XftObjectSetVapBuild(__ret__, __first__, __va__) 		\
{									\
    XftObjectSet    *__os__;						\
    const char	    *__ob__;						\
									\
    __ret__ = 0;						    	\
    __os__ = XftObjectSetCreate ();					\
    if (!__os__)							\
	goto _XftObjectSetVapBuild_bail0;				\
    __ob__ = __first__;							\
    while (__ob__)							\
    {									\
	if (!XftObjectSetAdd (__os__, __ob__))				\
	    goto _XftObjectSetVapBuild_bail1;				\
	__ob__ = va_arg (__va__, const char *);				\
    }									\
    __ret__ = __os__;							\
									\
_XftObjectSetVapBuild_bail1:						\
    if (!__ret__ && __os__)					    	\
	XftObjectSetDestroy (__os__);					\
_XftObjectSetVapBuild_bail0:						\
    ;									\
}

XftObjectSet *
XftObjectSetVaBuild (const char *first, va_list va)
{
    XftObjectSet    *ret;

    _XftObjectSetVapBuild (ret, first, va);
    return ret;
}

XftObjectSet *
XftObjectSetBuild (const char *first, ...)
{
    va_list	    va;
    XftObjectSet    *os;

    va_start (va, first);
    _XftObjectSetVapBuild (os, first, va);
    va_end (va);
    return os;
}

Bool
XftListValueCompare (XftValue	v1,
		     XftValue	v2)
{
    return _XftConfigCompareValue (v1, XftOpEqual, v2);
}

Bool
XftListValueListCompare (XftValueList	*v1orig,
			 XftValueList	*v2orig,
			 XftQual	qual)
{
    XftValueList    *v1, *v2;

    for (v1 = v1orig; v1; v1 = v1->next)
    {
	for (v2 = v2orig; v2; v2 = v2->next)
	{
	    if (_XftConfigCompareValue (v1->value, XftOpEqual, v2->value))
	    {
		if (qual == XftQualAny)
		    return True;
		else
		    break;
	    }
	}
	if (qual == XftQualAll)
	{
	    if (!v2)
		return False;
	}
    }
    if (qual == XftQualAll)
	return True;
    else
	return False;
}

/*
 * True iff all objects in "p" match "font"
 */
Bool
XftListMatch (XftPattern    *p,
	      XftPattern    *font,
	      XftQual	    qual)
{
    int		    i;
    XftPatternElt   *e;

    for (i = 0; i < p->num; i++)
    {
	e = XftPatternFind (font, p->elts[i].object, False);
	if (!e)
	{
	    if (qual == XftQualAll)
		continue;
	    else
		return False;
	}
	if (!XftListValueListCompare (p->elts[i].values, e->values, qual))
	    return False;
    }
    return True;
}

Bool
XftListAppend (XftFontSet   *s,
	       XftPattern   *font,
	       XftObjectSet *os)
{
    int		    f;
    int		    o;
    XftPattern	    *l;
    XftPatternElt   *e;
    XftValueList    *v;

    for (f = 0; f < s->nfont; f++)
    {
	l = s->fonts[f];
	if (XftListMatch (l, font, XftQualAll))
	    return True;
    }
    l = XftPatternCreate ();
    if (!l)
	goto bail0;
    for (o = 0; o < os->nobject; o++)
    {
	e = XftPatternFind (font, os->objects[o], False);
	if (e)
	{
	    for (v = e->values; v; v = v->next)
	    {
		if (!XftPatternAdd (l, os->objects[o], v->value, True))
		    goto bail1;
	    }
	}
    }
    if (!XftFontSetAdd (s, l))
	goto bail1;
    return True;
bail1:
    XftPatternDestroy (l);
bail0:
    return False;
}

XftFontSet *
XftListFontSets (XftFontSet	**sets,
		 int		nsets,
		 XftPattern	*p,
		 XftObjectSet	*os)
{
    XftFontSet	*ret;
    XftFontSet	*s;
    int		f;
    int		set;

    ret = XftFontSetCreate ();
    if (!ret)
	goto bail0;
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	for (f = 0; f < s->nfont; f++)
	{
	    if (XftListMatch (p, s->fonts[f], XftQualAny))
	    {
		if (!XftListAppend (ret, s->fonts[f], os))
		    goto bail1;
	    }
	}
    }
    return ret;
bail1:
    XftFontSetDestroy (ret);
bail0:
    return 0;
}

XftFontSet *
XftListFontsPatternObjects (Display	    *dpy,
			    int		    screen,
			    XftPattern	    *pattern,
			    XftObjectSet    *os)
{
    XftFontSet	*sets[2];
    int		nsets = 0;
#ifdef FREETYPE2
    Bool	core, render;
    XftResult	result;
#endif

    if (!XftInit (0))
	return 0;

#ifdef FREETYPE2
    render = core = False;
    result = XftPatternGetBool (pattern, XFT_CORE, 0, &core);
    if (result != XftResultMatch)
	core = XftDefaultGetBool (dpy, XFT_CORE, screen,
				  !XftDefaultHasRender (dpy));

    result = XftPatternGetBool (pattern, XFT_RENDER, 0, &render);
    if (result != XftResultMatch)
	render = XftDefaultGetBool (dpy, XFT_RENDER, screen,
				    XftDefaultHasRender (dpy));
    if (render)
    {
	if (XftInitFtLibrary ())
	{
	    sets[nsets] = _XftFontSet;
	    if (sets[nsets])
		nsets++;
	}
    }
    if (core)
#endif
    {
	sets[nsets] = XftDisplayGetFontSet (dpy);
	if (sets[nsets])
	    nsets++;
    }
    return XftListFontSets (sets, nsets, pattern, os);
}

XftFontSet *
XftListFonts (Display	*dpy,
	      int	screen,
	      ...)
{
    va_list	    va;
    XftFontSet	    *fs;
    XftObjectSet    *os;
    XftPattern	    *pattern;
    const char	    *first;

    va_start (va, screen);
    
    _XftPatternVapBuild (pattern, 0, va);
    
    first = va_arg (va, const char *);
    _XftObjectSetVapBuild (os, first, va);
    
    va_end (va);
    
    fs = XftListFontsPatternObjects (dpy, screen, pattern, os);
    XftPatternDestroy (pattern);
    XftObjectSetDestroy (os);
    return fs;
}
