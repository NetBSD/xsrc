/*
 * $XFree86: xc/lib/Xft/xftrender.c,v 1.5 2000/12/08 07:51:28 keithp Exp $
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

void
XftRenderString8 (Display *dpy, Picture src, 
		  XftFontStruct *font, Picture dst,
		  int srcx, int srcy,
		  int x, int y,
		  XftChar8 *string, int len)
{
    unsigned int    missing[XFT_NMISSING];
    int		    nmissing;
    XftChar8	    *s;
    int		    l;

    s = string;
    l = len;
    nmissing = 0;
    while (l--)
	XftGlyphCheck (dpy, font, (unsigned int) *s++, missing, &nmissing);
    if (nmissing)
	XftGlyphLoad (dpy, font, missing, nmissing);
    XRenderCompositeString8 (dpy, PictOpOver, src, dst,
			     font->format, font->glyphset,
			     srcx, srcy, x, y, (char *) string, len);
}

void
XftRenderString16 (Display *dpy, Picture src, 
		   XftFontStruct *font, Picture dst,
		   int srcx, int srcy,
		   int x, int y,
		   XftChar16 *string, int len)
{
    unsigned int    missing[XFT_NMISSING];
    int		    nmissing;
    XftChar16	    *s;
    int		    l;

    s = string;
    l = len;
    nmissing = 0;
    while (l--)
	XftGlyphCheck (dpy, font, (unsigned int) *s++, missing, &nmissing);
    if (nmissing)
	XftGlyphLoad (dpy, font, missing, nmissing);
    XRenderCompositeString16 (dpy, PictOpOver, src, dst,
			      font->format, font->glyphset,
			      srcx, srcy, x, y, string, len);
}

void
XftRenderString32 (Display *dpy, Picture src, 
		   XftFontStruct *font, Picture dst,
		   int srcx, int srcy,
		   int x, int y,
		   unsigned int *string, int len)
{
    unsigned int    missing[XFT_NMISSING];
    int		    nmissing;
    unsigned int    *s;
    int		    l;

    s = string;
    l = len;
    nmissing = 0;
    while (l--)
	XftGlyphCheck (dpy, font, (unsigned int) *s++, missing, &nmissing);
    if (nmissing)
	XftGlyphLoad (dpy, font, missing, nmissing);
    XRenderCompositeString32 (dpy, PictOpOver, src, dst,
			      font->format, font->glyphset,
			      srcx, srcy, x, y, string, len);
}

void
XftRenderExtents8 (Display	    *dpy,
		   XftFontStruct    *font,
		   XftChar8    *string, 
		   int		    len,
		   XGlyphInfo	    *extents)
{
    unsigned int    missing[XFT_NMISSING];
    int		    nmissing;
    XftChar8	    *s, c;
    int		    l;
    XGlyphInfo	    *gi;
    int		    x, y;

    s = string;
    l = len;
    nmissing = 0;
    while (l--)
	XftGlyphCheck (dpy, font, (unsigned int) *s++, missing, &nmissing);
    if (nmissing)
	XftGlyphLoad (dpy, font, missing, nmissing);
    
    gi = 0;
    while (len)
    {
	c = *string++;
	len--;
	gi = c < font->nrealized ? font->realized[c] : 0;
	if (gi)
	    break;
    }
    if (len == 0 && !gi)
    {
	extents->width = 0;
	extents->height = 0;
	extents->x = 0;
	extents->y = 0;
	extents->yOff = 0;
	extents->xOff = 0;
	return;
    }
    *extents = *gi;
    x = gi->xOff;
    y = gi->yOff;
    while (len--)
    {
	c = *string++;
	gi = c < font->nrealized ? font->realized[c] : 0;
	if (!gi)
	    continue;
	if (gi->x + x < extents->x)
	    extents->x = gi->x + x;
	if (gi->y + y < extents->y)
	    extents->y = gi->y + y;
	if (gi->width + x > extents->width)
	    extents->width = gi->width + x;
	if (gi->height + y > extents->height)
	    extents->height = gi->height + y;
	x += gi->xOff;
	y += gi->yOff;
    }
    extents->xOff = x;
    extents->yOff = y;
}

void
XftRenderExtents16 (Display	    *dpy,
		    XftFontStruct   *font,
		    XftChar16  *string, 
		    int		    len,
		    XGlyphInfo	    *extents)
{
    unsigned int    missing[XFT_NMISSING];
    int		    nmissing;
    XftChar16	    *s, c;
    int		    l;
    XGlyphInfo	    *gi;
    int		    x, y;

    s = string;
    l = len;
    nmissing = 0;
    while (l--)
	XftGlyphCheck (dpy, font, (unsigned int) *s++, missing, &nmissing);
    if (nmissing)
	XftGlyphLoad (dpy, font, missing, nmissing);
    
    gi = 0;
    while (len)
    {
	c = *string++;
	len--;
	gi = c < font->nrealized ? font->realized[c] : 0;
	if (gi)
	    break;
    }
    if (len == 0 && !gi)
    {
	extents->width = 0;
	extents->height = 0;
	extents->x = 0;
	extents->y = 0;
	extents->yOff = 0;
	extents->xOff = 0;
	return;
    }
    *extents = *gi;
    x = gi->xOff;
    y = gi->yOff;
    while (len--)
    {
	c = *string++;
	gi = c < font->nrealized ? font->realized[c] : 0;
	if (!gi)
	    continue;
	if (gi->x + x < extents->x)
	    extents->x = gi->x + x;
	if (gi->y + y < extents->y)
	    extents->y = gi->y + y;
	if (gi->width + x > extents->width)
	    extents->width = gi->width + x;
	if (gi->height + y > extents->height)
	    extents->height = gi->height + y;
	x += gi->xOff;
	y += gi->yOff;
    }
    extents->xOff = x;
    extents->yOff = y;
}

void
XftRenderExtents32 (Display	    *dpy,
		    XftFontStruct   *font,
		    unsigned int    *string, 
		    int		    len,
		    XGlyphInfo	    *extents)
{
    unsigned int    missing[XFT_NMISSING];
    int		    nmissing;
    unsigned int    *s, c;
    int		    l;
    XGlyphInfo	    *gi;
    int		    x, y;

    s = string;
    l = len;
    nmissing = 0;
    while (l--)
	XftGlyphCheck (dpy, font, (unsigned int) *s++, missing, &nmissing);
    if (nmissing)
	XftGlyphLoad (dpy, font, missing, nmissing);
    
    gi = 0;
    while (len)
    {
	c = *string++;
	len--;
	gi = c < font->nrealized ? font->realized[c] : 0;
	if (gi)
	    break;
    }
    if (len == 0 && !gi)
    {
	extents->width = 0;
	extents->height = 0;
	extents->x = 0;
	extents->y = 0;
	extents->yOff = 0;
	extents->xOff = 0;
	return;
    }
    *extents = *gi;
    x = gi->xOff;
    y = gi->yOff;
    while (len--)
    {
	c = *string++;
	gi = c < font->nrealized ? font->realized[c] : 0;
	if (!gi)
	    continue;
	if (gi->x + x < extents->x)
	    extents->x = gi->x + x;
	if (gi->y + y < extents->y)
	    extents->y = gi->y + y;
	if (gi->width + x > extents->width)
	    extents->width = gi->width + x;
	if (gi->height + y > extents->height)
	    extents->height = gi->height + y;
	x += gi->xOff;
	y += gi->yOff;
    }
    extents->xOff = x;
    extents->yOff = y;
}
