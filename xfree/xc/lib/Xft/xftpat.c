/*
 * $XFree86: xc/lib/Xft/xftpat.c,v 1.5 2000/12/14 23:03:56 keithp Exp $
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
#include <string.h>
#include "xftint.h"

XftPattern *
XftPatternCreate (void)
{
    XftPattern	*p;

    p = (XftPattern *) malloc (sizeof (XftPattern));
    if (!p)
	return 0;
    p->num = 0;
    p->size = 0;
    p->elts = 0;
    return p;
}

void
XftValueDestroy (XftValue v)
{
    if (v.type == XftTypeString)
	free (v.u.s);
}

void
XftValueListDestroy (XftValueList *l)
{
    XftValueList    *next;
    for (; l; l = next)
    {
	if (l->value.type == XftTypeString)
	    free (l->value.u.s);
	next = l->next;
	free (l);
    }
}

void
XftPatternDestroy (XftPattern *p)
{
    int		    i;
    
    for (i = 0; i < p->num; i++)
	XftValueListDestroy (p->elts[i].values);

    if (p->elts)
    {
	free (p->elts);
	p->elts = 0;
    }
    p->num = p->size = 0;
    free (p);
}

XftPatternElt *
XftPatternFind (XftPattern *p, const char *object, Bool insert)
{
    int		    i;
    int		    s;
    XftPatternElt   *e;
    
    /* match existing */
    for (i = 0; i < p->num; i++)
    {
	if (!_XftStrCmpIgnoreCase (object, p->elts[i].object))
	    return &p->elts[i];
    }

    if (!insert)
	return 0;

    /* grow array */
    if (i == p->size)
    {
	s = p->size + 16;
	if (p->elts)
	    e = (XftPatternElt *) realloc (p->elts, s * sizeof (XftPatternElt));
	else
	    e = (XftPatternElt *) malloc (s * sizeof (XftPatternElt));
	if (!e)
	    return False;
	p->elts = e;
	while (p->size < s)
	{
	    p->elts[p->size].object = 0;
	    p->elts[p->size].values = 0;
	    p->size++;
	}
    }
    
    /* bump count */
    p->num++;
    
    return &p->elts[i];
}

Bool
XftPatternAdd (XftPattern *p, const char *object, XftValue value, Bool append)
{
    XftPatternElt   *e;
    XftValueList    *new, **prev;

    new = (XftValueList *) malloc (sizeof (XftValueList));
    if (!new)
	goto bail0;

    /* dup string */
    if (value.type == XftTypeString)
    {
	value.u.s = _XftSaveString (value.u.s);
	if (!value.u.s)
	    goto bail1;
    }
    new->value = value;
    new->next = 0;
    
    e = XftPatternFind (p, object, True);
    if (!e)
	goto bail2;
    
    e->object = object;
    if (append)
    {
	for (prev = &e->values; *prev; prev = &(*prev)->next);
	*prev = new;
    }
    else
    {
	new->next = e->values;
	e->values = new;
    }
    
    return True;

bail2:    
    if (value.type == XftTypeString)
        free (value.u.s);
bail1:
    free (new);
bail0:
    return False;
}

Bool
XftPatternDel (XftPattern *p, const char *object)
{
    XftPatternElt   *e;
    int		    i;

    e = XftPatternFind (p, object, False);
    if (!e)
	return False;

    i = e - p->elts;
    
    /* destroy value */
    XftValueListDestroy (e->values);
    
    /* shuffle existing ones down */
    memmove (e, e+1, (p->elts + p->num - (e + 1)) * sizeof (XftPatternElt));
    p->num--;
    p->elts[p->num].object = 0;
    p->elts[p->num].values = 0;
    return True;
}

Bool
XftPatternAddInteger (XftPattern *p, const char *object, int i)
{
    XftValue	v;

    v.type = XftTypeInteger;
    v.u.i = i;
    return XftPatternAdd (p, object, v, True);
}

Bool
XftPatternAddDouble (XftPattern *p, const char *object, double d)
{
    XftValue	v;

    v.type = XftTypeDouble;
    v.u.d = d;
    return XftPatternAdd (p, object, v, True);
}


Bool
XftPatternAddString (XftPattern *p, const char *object, const char *s)
{
    XftValue	v;

    v.type = XftTypeString;
    v.u.s = (char *) s;
    return XftPatternAdd (p, object, v, True);
}


Bool
XftPatternAddBool (XftPattern *p, const char *object, Bool b)
{
    XftValue	v;

    v.type = XftTypeBool;
    v.u.b = b;
    return XftPatternAdd (p, object, v, True);
}

XftResult
XftPatternGet (XftPattern *p, const char *object, int id, XftValue *v)
{
    XftPatternElt   *e;
    XftValueList    *l;

    e = XftPatternFind (p, object, False);
    if (!e)
	return XftResultNoMatch;
    for (l = e->values; l; l = l->next)
    {
	if (!id)
	{
	    *v = l->value;
	    return XftResultMatch;
	}
	id--;
    }
    return XftResultNoId;
}

XftResult
XftPatternGetInteger (XftPattern *p, const char *object, int id, int *i)
{
    XftValue	v;
    XftResult	r;

    r = XftPatternGet (p, object, id, &v);
    if (r != XftResultMatch)
	return r;
    switch (v.type) {
    case XftTypeDouble:
	*i = (int) v.u.d;
	break;
    case XftTypeInteger:
	*i = v.u.i;
	break;
    default:
        return XftResultTypeMismatch;
    }
    return XftResultMatch;
}

XftResult
XftPatternGetDouble (XftPattern *p, const char *object, int id, double *d)
{
    XftValue	v;
    XftResult	r;

    r = XftPatternGet (p, object, id, &v);
    if (r != XftResultMatch)
	return r;
    switch (v.type) {
    case XftTypeDouble:
	*d = v.u.d;
	break;
    case XftTypeInteger:
	*d = (double) v.u.i;
	break;
    default:
        return XftResultTypeMismatch;
    }
    return XftResultMatch;
}

XftResult
XftPatternGetString (XftPattern *p, const char *object, int id, char **s)
{
    XftValue	v;
    XftResult	r;

    r = XftPatternGet (p, object, id, &v);
    if (r != XftResultMatch)
	return r;
    if (v.type != XftTypeString)
        return XftResultTypeMismatch;
    *s = v.u.s;
    return XftResultMatch;
}

XftResult
XftPatternGetBool (XftPattern *p, const char *object, int id, Bool *b)
{
    XftValue	v;
    XftResult	r;

    r = XftPatternGet (p, object, id, &v);
    if (r != XftResultMatch)
	return r;
    if (v.type != XftTypeBool)
        return XftResultTypeMismatch;
    *b = v.u.b;
    return XftResultMatch;
}

XftPattern *
XftPatternDuplicate (XftPattern *orig)
{
    XftPattern	    *new;
    int		    i;
    XftValueList    *l;

    new = XftPatternCreate ();
    if (!new)
	goto bail0;

    for (i = 0; i < orig->num; i++)
    {
	for (l = orig->elts[i].values; l; l = l->next)
	    if (!XftPatternAdd (new, orig->elts[i].object, l->value, True))
		goto bail1;
    }

    return new;

bail1:
    XftPatternDestroy (new);
bail0:
    return 0;
}

XftPattern *
XftPatternVaBuild (XftPattern *orig, va_list va)
{
    XftPattern	*ret;
    
    _XftPatternVapBuild (ret, orig, va);
    return ret;
}

XftPattern *
XftPatternBuild (XftPattern *orig, ...)
{
    va_list	va;
    
    va_start (va, orig);
    _XftPatternVapBuild (orig, orig, va);
    va_end (va);
    return orig;
}
