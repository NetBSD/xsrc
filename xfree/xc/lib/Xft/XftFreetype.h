/*
 * $XFree86: xc/lib/Xft/XftFreetype.h,v 1.7 2000/12/15 17:12:52 keithp Exp $
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

#ifndef _XFTFREETYPE_H_
#define _XFTFREETYPE_H_

#include <X11/Xft/Xft.h>
#include <freetype/freetype.h>

#include <X11/Xfuncproto.h>
#include <X11/Xosdefs.h>

extern FT_Library	_XftFTlibrary;

struct _XftFontStruct {
    FT_Face		face;      /* handle to face object */
    GlyphSet		glyphset;
    int			min_char;
    int			max_char;
    int			size;
    int			ascent;
    int			descent;
    int			height;
    int			max_advance_width;
    Bool		monospace;
    int			rgba;
    Bool		antialias;
    Bool		encoded;    /* use charmap */
    XRenderPictFormat	*format;
    XGlyphInfo		**realized;
    int			nrealized;
};

_XFUNCPROTOBEGIN

/* xftfreetype.c */
XftFontStruct *
XftFreeTypeOpen (Display *dpy, XftPattern *pattern);

void
XftFreeTypeClose (Display *dpy, XftFontStruct *font);

/* xftglyphs.c */
void
XftGlyphLoad (Display		*dpy,
	      XftFontStruct	*font,
	      XftChar32		*glyphs,
	      int		nglyph);

void
XftGlyphCheck (Display		*dpy,
	       XftFontStruct	*font,
	       XftChar32	glyph,
	       XftChar32	*missing,
	       int		*nmissing);

Bool
XftFreeTypeGlyphExists (Display		*dpy,
			XftFontStruct	*font,
			XftChar32	glyph);

/* xftrender.c */

void
XftRenderString8 (Display *dpy, Picture src, 
		  XftFontStruct *font, Picture dst,
		  int srcx, int srcy,
		  int x, int y,
		  XftChar8 *string, int len);

void
XftRenderString16 (Display *dpy, Picture src, 
		   XftFontStruct *font, Picture dst,
		   int srcx, int srcy,
		   int x, int y,
		   XftChar16 *string, int len);

void
XftRenderString32 (Display *dpy, Picture src, 
		   XftFontStruct *font, Picture dst,
		   int srcx, int srcy,
		   int x, int y,
		   XftChar32 *string, int len);

void
XftRenderExtents8 (Display	    *dpy,
		   XftFontStruct    *font,
		   XftChar8	    *string, 
		   int		    len,
		   XGlyphInfo	    *extents);

void
XftRenderExtents16 (Display	    *dpy,
		    XftFontStruct   *font,
		    XftChar16	    *string, 
		    int		    len,
		    XGlyphInfo	    *extents);

void
XftRenderExtents32 (Display	    *dpy,
		    XftFontStruct   *font,
		    XftChar32	    *string, 
		    int		    len,
		    XGlyphInfo	    *extents);

XftFontStruct *
XftFreeTypeGet (XftFont *font);

_XFUNCPROTOEND

#endif /* _XFTFREETYPE_H_ */
