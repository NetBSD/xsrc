/*
 * $XFree86: xc/lib/Xft/xftmatch.c,v 1.6 2001/09/21 19:54:53 keithp Exp $
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

#include <string.h>
#include <ctype.h>
#include "xftint.h"
#include <stdio.h>

static double
_XftCompareInteger (char *object, XftValue value1, XftValue value2)
{
    int	v;
    
    if (value2.type != XftTypeInteger || value1.type != XftTypeInteger)
	return -1.0;
    v = value2.u.i - value1.u.i;
    if (v < 0)
	v = -v;
    return (double) v;
}

static double
_XftCompareString (char *object, XftValue value1, XftValue value2)
{
    if (value2.type != XftTypeString || value1.type != XftTypeString)
	return -1.0;
    return (double) _XftStrCmpIgnoreCase (value1.u.s, value2.u.s) != 0;
}

static double
_XftCompareBool (char *object, XftValue value1, XftValue value2)
{
    if (value2.type != XftTypeBool || value1.type != XftTypeBool)
	return -1.0;
    return (double) value2.u.b != value1.u.b;
}

static double
_XftCompareSize (char *object, XftValue value1, XftValue value2)
{
    double  v1, v2, v;

    switch (value1.type) {
    case XftTypeInteger:
	v1 = value1.u.i;
	break;
    case XftTypeDouble:
	v1 = value1.u.d;
	break;
    default:
	return -1;
    }
    switch (value2.type) {
    case XftTypeInteger:
	v2 = value2.u.i;
	break;
    case XftTypeDouble:
	v2 = value2.u.d;
	break;
    default:
	return -1;
    }
    if (v2 == 0)
	return 0;
    v = v2 - v1;
    if (v < 0)
	v = -v;
    return v;
}

/*
 * Order is significant, it defines the precedence of
 * each value, earlier values are more significant than
 * later values
 */
static XftMatcher _XftMatchers [] = {
    { XFT_FOUNDRY,	_XftCompareString, },
    { XFT_ENCODING,	_XftCompareString, },
    { XFT_FAMILY,	_XftCompareString, },
    { XFT_SPACING,	_XftCompareInteger, },
    { XFT_PIXEL_SIZE,	_XftCompareSize, },
    { XFT_STYLE,	_XftCompareString, },
    { XFT_SLANT,	_XftCompareInteger, },
    { XFT_WEIGHT,	_XftCompareInteger, },
    { XFT_RASTERIZER,	_XftCompareString, },
    { XFT_ANTIALIAS,	_XftCompareBool, },
    { XFT_OUTLINE,	_XftCompareBool, },
};

#define NUM_MATCHER (sizeof _XftMatchers / sizeof _XftMatchers[0])

static Bool
_XftCompareValueList (const char    *object,
		      XftValueList  *v1orig,	/* pattern */
		      XftValueList  *v2orig,	/* target */
		      XftValue	    *bestValue,
		      double	    *value,
		      XftResult	    *result)
{
    XftValueList    *v1, *v2;
    double    	    v, best;
    int		    j;
    int		    i;
    
    for (i = 0; i < NUM_MATCHER; i++)
    {
	if (!_XftStrCmpIgnoreCase (_XftMatchers[i].object, object))
	    break;
    }
    if (i == NUM_MATCHER)
    {
	if (bestValue)
	    *bestValue = v2orig->value;
	return True;
    }
    
    best = 1e99;
    j = 0;
    for (v1 = v1orig; v1; v1 = v1->next)
    {
	for (v2 = v2orig; v2; v2 = v2->next)
	{
	    v = (*_XftMatchers[i].compare) (_XftMatchers[i].object,
					    v1->value,
					    v2->value);
	    if (v < 0)
	    {
		*result = XftResultTypeMismatch;
		return False;
	    }
	    if (_XftFontDebug () & XFT_DBG_MATCHV)
		printf (" v %g j %d ", v, j);
	    v = v * 100 + j;
	    if (v < best)
	    {
		if (bestValue)
		    *bestValue = v2->value;
		best = v;
	    }
	}
	j++;
    }
    if (_XftFontDebug () & XFT_DBG_MATCHV)
    {
	printf (" %s: %g ", object, best);
	XftValueListPrint (v1orig);
	printf (", ");
	XftValueListPrint (v2orig);
	printf ("\n");
    }
    value[i] += best;
    return True;
}

/*
 * Return a value indicating the distance between the two lists of
 * values
 */

static Bool
_XftCompare (XftPattern	*pat,
	     XftPattern *fnt,
	     double	*value,
	     XftResult	*result)
{
    int		    i, i1, i2;
    
    for (i = 0; i < NUM_MATCHER; i++)
	value[i] = 0.0;
    
    for (i1 = 0; i1 < pat->num; i1++)
    {
	for (i2 = 0; i2 < fnt->num; i2++)
	{
	    if (!_XftStrCmpIgnoreCase (pat->elts[i1].object,
				       fnt->elts[i2].object))
	    {
		if (!_XftCompareValueList (pat->elts[i1].object,
					   pat->elts[i1].values,
					   fnt->elts[i2].values,
					   0,
					   value,
					   result))
		    return False;
		break;
	    }
	}
#if 0
	/*
	 * Overspecified patterns are slightly penalized in
	 * case some other font includes the requested field
	 */
	if (i2 == fnt->num)
	{
	    for (i2 = 0; i2 < NUM_MATCHER; i2++)
	    {
		if (!_XftStrCmpIgnoreCase (_XftMatchers[i2].object,
					   pat->elts[i1].object))
		{
		    value[i2] = 1.0;
		    break;
		}
	    }
	}
#endif
    }
    return True;
}

XftPattern *
XftFontSetMatch (XftFontSet	**sets, 
		 int		nsets, 
		 XftPattern	*p, 
		 XftResult	*result)
{
    double    	    score[NUM_MATCHER], bestscore[NUM_MATCHER];
    int		    f;
    XftFontSet	    *s;
    XftPattern	    *best;
    XftPattern	    *new;
    XftPatternElt   *fe, *pe;
    XftValue	    v;
    int		    i;
    int		    set;

    for (i = 0; i < NUM_MATCHER; i++)
	bestscore[i] = 0;
    best = 0;
    if (_XftFontDebug () & XFT_DBG_MATCH)
    {
	printf ("Match ");
	XftPatternPrint (p);
    }
    for (set = 0; set < nsets; set++)
    {
	s = sets[set];
	for (f = 0; f < s->nfont; f++)
	{
	    if (_XftFontDebug () & XFT_DBG_MATCH)
	    {
		printf ("Font %d ", f);
		XftPatternPrint (s->fonts[f]);
	    }
	    if (!_XftCompare (p, s->fonts[f], score, result))
		return 0;
	    if (_XftFontDebug () & XFT_DBG_MATCH)
	    {
		printf ("Score");
		for (i = 0; i < NUM_MATCHER; i++)
		{
		    printf (" %g", score[i]);
		}
		printf ("\n");
	    }
	    for (i = 0; i < NUM_MATCHER; i++)
	    {
		if (best && bestscore[i] < score[i])
		    break;
		if (!best || score[i] < bestscore[i])
		{
		    for (i = 0; i < NUM_MATCHER; i++)
			bestscore[i] = score[i];
		    best = s->fonts[f];
		    break;
		}
	    }
	}
    }
    if (_XftFontDebug () & XFT_DBG_MATCH)
    {
	printf ("Best score");
	for (i = 0; i < NUM_MATCHER; i++)
	    printf (" %g", bestscore[i]);
	XftPatternPrint (best);
    }
    if (!best)
    {
	*result = XftResultNoMatch;
	return 0;
    }
    new = XftPatternCreate ();
    if (!new)
	return 0;
    for (i = 0; i < best->num; i++)
    {
	fe = &best->elts[i];
	pe = XftPatternFind (p, fe->object, False);
	if (pe)
	{
	    if (!_XftCompareValueList (pe->object, pe->values, 
				       fe->values, &v, score, result))
	    {
		XftPatternDestroy (new);
		return 0;
	    }
	}
	else
	    v = fe->values->value;
	XftPatternAdd (new, fe->object, v, True);
    }
    for (i = 0; i < p->num; i++)
    {
	pe = &p->elts[i];
	fe = XftPatternFind (best, pe->object, False);
	if (!fe)
	    XftPatternAdd (new, pe->object, pe->values->value, True);
    }
    return new;
}
