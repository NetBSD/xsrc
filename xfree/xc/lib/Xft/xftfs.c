/*
 * $XFree86: xc/lib/Xft/xftfs.c,v 1.1 2000/11/29 08:39:22 keithp Exp $
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

XftFontSet *
XftFontSetCreate (void)
{
    XftFontSet	*s;

    s = (XftFontSet *) malloc (sizeof (XftFontSet));
    if (!s)
	return 0;
    s->nfont = 0;
    s->sfont = 0;
    s->fonts = 0;
    return s;
}

void
XftFontSetDestroy (XftFontSet *s)
{
    int	    i;

    for (i = 0; i < s->nfont; i++)
	XftPatternDestroy (s->fonts[i]);
    if (s->fonts)
	free (s->fonts);
    free (s);
}

Bool
XftFontSetAdd (XftFontSet *s, XftPattern *font)
{
    XftPattern	**f;
    int		sfont;
    
    if (s->nfont == s->sfont)
    {
	sfont = s->sfont + 32;
	if (s->fonts)
	    f = (XftPattern **) realloc (s->fonts, sfont * sizeof (XftPattern *));
	else
	    f = (XftPattern **) malloc (sfont * sizeof (XftPattern *));
	if (!f)
	    return False;
	s->sfont = sfont;
	s->fonts = f;
    }
    s->fonts[s->nfont++] = font;
    return True;
}
