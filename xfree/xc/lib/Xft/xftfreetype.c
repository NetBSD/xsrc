/*
 * $XFree86: xc/lib/Xft/xftfreetype.c,v 1.5 2000/12/15 17:12:53 keithp Exp $
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

FT_Library  _XftFTlibrary;

typedef struct _XftFtEncoding {
    const char	*name;
    FT_Encoding	encoding;
} XftFtEncoding;

XftFtEncoding	xftFtEncoding[] = {
    { "iso10646-1",	    ft_encoding_unicode, },
    { "iso8859-1",	    ft_encoding_unicode, },
    { "adobe-fontspecific", ft_encoding_symbol,  },
    { "glyphs-fontspecific",ft_encoding_none,	 },
};

#define NUM_FT_ENCODINGS    (sizeof xftFtEncoding / sizeof xftFtEncoding[0])

XftPattern *
XftFreeTypeQuery (const char *file, int id, int *count)
{
    FT_Face	face;
    XftPattern	*pat;
    int		slant;
    int		weight;
    int		i, j;
    
    if (FT_New_Face (_XftFTlibrary, file, id, &face))
	return 0;

    *count = face->num_faces;
    
    pat = XftPatternCreate ();
    if (!pat)
	goto bail0;


    if (!XftPatternAddBool (pat, XFT_CORE, False))
	goto bail1;
    
    if (!XftPatternAddBool (pat, XFT_OUTLINE,
			    (face->face_flags & FT_FACE_FLAG_SCALABLE) != 0))
	goto bail1;
    
    if (!XftPatternAddBool (pat, XFT_SCALABLE,
			    (face->face_flags & FT_FACE_FLAG_SCALABLE) != 0))
	goto bail1;
    

    slant = XFT_SLANT_ROMAN;
    if (face->style_flags & FT_STYLE_FLAG_ITALIC)
	slant = (XFT_SLANT_ITALIC + XFT_SLANT_OBLIQUE) / 2;

    if (!XftPatternAddInteger (pat, XFT_SLANT, slant))
	goto bail1;
    
    weight = XFT_WEIGHT_MEDIUM;
    if (face->style_flags & FT_STYLE_FLAG_BOLD)
	weight = XFT_WEIGHT_BOLD;
    
    if (!XftPatternAddInteger (pat, XFT_WEIGHT, weight))
	goto bail1;
    
    if (!XftPatternAddString (pat, XFT_FAMILY, face->family_name))
	goto bail1;

    if (!XftPatternAddString (pat, XFT_STYLE, face->style_name))
	goto bail1;

    if (!XftPatternAddString (pat, XFT_FILE, file))
	goto bail1;

    if (!XftPatternAddInteger (pat, XFT_INDEX, id))
	goto bail1;
    
    if (!(face->face_flags & FT_FACE_FLAG_SCALABLE))
    {
	for (i = 0; i < face->num_fixed_sizes; i++)
	    if (!XftPatternAddDouble (pat, XFT_PIXEL_SIZE,
				      (double) face->available_sizes[i].height))
		goto bail1;
    }
    
    for (i = 0; i < face->num_charmaps; i++)
    {
#if 0	
	printf ("face %s encoding %d %c%c%c%c\n",
		face->family_name, i, 
		face->charmaps[i]->encoding >> 24,
		face->charmaps[i]->encoding >> 16,
		face->charmaps[i]->encoding >> 8,
		face->charmaps[i]->encoding >> 0);
#endif
	for (j = 0; j < NUM_FT_ENCODINGS; j++)
	{
	    if (face->charmaps[i]->encoding == xftFtEncoding[j].encoding)
	    {
		if (!XftPatternAddString (pat, XFT_ENCODING, 
					  xftFtEncoding[j].name))
		    goto bail1;
	    }
	}
    }

    if (!XftPatternAddString (pat, XFT_ENCODING, 
			      "glyphs-fontspecific"))
	goto bail1;
	

    FT_Done_Face (face);
    return pat;
    
bail1:
    XftPatternDestroy (pat);
bail0:
    FT_Done_Face (face);
    return 0;
}

XftFontStruct *
XftFreeTypeOpen (Display *dpy, XftPattern *pattern)
{
    char	    *file;
    int		    id;
    double	    size;
    int		    rgba;
    int		    spacing;
    int		    char_width;
    Bool	    antialias;
    Bool	    encoded;
    char	    *encoding_name;
    FT_Face	    face;
    XftFontStruct   *font;
    int		    j;
    FT_Encoding	    encoding;
    int		    charmap;
    int		    error;

    int		    height, ascent, descent;
    int		    extra;
    int		    div;
    
    XRenderPictFormat	pf, *format;
    
    if (XftPatternGetString (pattern, XFT_FILE, 0, &file) != XftResultMatch)
	goto bail0;
    
    if (XftPatternGetInteger (pattern, XFT_INDEX, 0, &id) != XftResultMatch)
	goto bail0;
    
    if (XftPatternGetString (pattern, XFT_ENCODING, 0, &encoding_name) != XftResultMatch)
	goto bail0;
    
    if (XftPatternGetDouble (pattern, XFT_PIXEL_SIZE, 0, &size) != XftResultMatch)
	goto bail0;
    
    switch (XftPatternGetInteger (pattern, XFT_RGBA, 0, &rgba)) {
    case XftResultNoMatch:
	rgba = XFT_RGBA_NONE;
	break;
    case XftResultMatch:
	break;
    default:
	goto bail0;
    }
    
    switch (XftPatternGetBool (pattern, XFT_ANTIALIAS, 0, &antialias)) {
    case XftResultNoMatch:
	antialias = True;
	break;
    case XftResultMatch:
	break;
    default:
	goto bail0;
    }
    
    if (XftPatternGetInteger (pattern, XFT_CHAR_WIDTH, 
			      0, &char_width) != XftResultMatch)
    {
	char_width = 0;
    }
    
    if (antialias)
    {
	if (rgba)
	{
	    pf.depth = 32;
	    pf.type = PictTypeDirect;
	    pf.direct.alpha = 24;
	    pf.direct.alphaMask = 0xff;
	    pf.direct.red = 16;
	    pf.direct.redMask = 0xff;
	    pf.direct.green = 8;
	    pf.direct.greenMask = 0xff;
	    pf.direct.blue = 0;
	    pf.direct.blueMask = 0xff;
	    format = XRenderFindFormat(dpy, 
				       PictFormatType|
				       PictFormatDepth|
				       PictFormatAlpha|
				       PictFormatAlphaMask|
				       PictFormatRed|
				       PictFormatRedMask|
				       PictFormatGreen|
				       PictFormatGreenMask|
				       PictFormatBlue|
				       PictFormatBlueMask,
				       &pf, 0);
	}
	else
	{
	    pf.depth = 8;
	    pf.type = PictTypeDirect;
	    pf.direct.alpha = 0;
	    pf.direct.alphaMask = 0xff;
	    format = XRenderFindFormat(dpy, 
				       PictFormatType|
				       PictFormatDepth|
				       PictFormatAlpha|
				       PictFormatAlphaMask,
				       &pf, 0);
	}
    }
    else
    {
	pf.depth = 1;
	pf.type = PictTypeDirect;
	pf.direct.alpha = 0;
	pf.direct.alphaMask = 0x1;
	format = XRenderFindFormat(dpy, 
				   PictFormatType|
				   PictFormatDepth|
				   PictFormatAlpha|
				   PictFormatAlphaMask,
				   &pf, 0);
    }
    
    if (!format)
	goto bail0;
    
    if (FT_New_Face (_XftFTlibrary, file, id, &face))
	goto bail0;

    font = (XftFontStruct *) malloc (sizeof (XftFontStruct));
    if (!font)
	goto bail1;
    
    font->size = (FT_F26Dot6) (size * 64.0);
    
    if ( FT_Set_Char_Size (face, font->size, font->size, 0, 0) )
	goto bail2;

    encoding = face->charmaps[0]->encoding;
    
    for (j = 0; j < NUM_FT_ENCODINGS; j++)
	if (!strcmp (encoding_name, xftFtEncoding[j].name))
	{
	    encoding = xftFtEncoding[j].encoding;
	    break;
	}
    
    if (encoding == ft_encoding_none)
	encoded = False;
    else
    {
	encoded = True;
	for (charmap = 0; charmap < face->num_charmaps; charmap++)
	    if (face->charmaps[charmap]->encoding == encoding)
		break;

	if (charmap == face->num_charmaps)
	    goto bail2;

	error = FT_Set_Charmap(face,
			       face->charmaps[charmap]);

	if (error)
	    goto bail2;
    }
    
    height = face->height;
    ascent = face->ascender;
    descent = face->descender;
    if (descent < 0) descent = - descent;
    extra = (height - (ascent + descent));
    if (extra > 0)
    {
	ascent = ascent + extra / 2;
	descent = height - ascent;
    }
    else if (extra < 0)
	height = ascent + descent;
    div = face->units_per_EM;
    if (height > div * 5)
	div *= 10;
    
    div = face->units_per_EM;
    if (height > div * 5)
	div *= 10;
    
    font->descent = descent * font->size / (64 * div);
    font->ascent = ascent * font->size / (64 * div);
    font->height = height * font->size / (64 * div);
    font->max_advance_width = face->max_advance_width * font->size / (64 * div);
    
    font->monospace = (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) != 0;
    if (char_width)
    {
	font->max_advance_width = char_width;
	font->monospace = True;
    }
    switch (XftPatternGetInteger (pattern, XFT_SPACING, 0, &spacing)) {
    case XftResultNoMatch:
	break;
    case XftResultMatch:
	if (spacing != XFT_PROPORTIONAL)
	    font->monospace = True;
	break;
    default:
	goto bail2;
    }
    
    font->glyphset = XRenderCreateGlyphSet (dpy, format);

    font->format = format;
    font->realized =0;
    font->nrealized = 0;
    font->rgba = rgba;
    font->antialias = antialias;
    font->encoded = encoded;
    font->face = face;

    return font;
    
bail2:
    free (font);
bail1:
    FT_Done_Face (font->face);
bail0:
    return 0;
}

void
XftFreeTypeClose (Display *dpy, XftFontStruct *font)
{
    XRenderFreeGlyphSet (dpy, font->glyphset);
    if (font->realized)
	free (font->realized);
    FT_Done_Face (font->face);
}
		  
XftFontStruct *
XftFreeTypeGet (XftFont *font)
{
    if (font->core)
	return 0;
    return font->u.ft.font;
}

/* #define XFT_DEBUG_FONTSET */

Bool
XftInitFtLibrary (void)
{
    char    **d;
    
    if (_XftFTlibrary)
	return True;
    if (FT_Init_FreeType (&_XftFTlibrary))
	return False;
    _XftFontSet = XftFontSetCreate ();
    if (!_XftFontSet)
	return False;
    for (d = XftConfigDirs; d && *d; d++)
    {
#ifdef XFT_DEBUG_FONTSET
	printf ("scan dir %s\n", *d);
#endif
	XftDirScan (_XftFontSet, *d);
    }
#ifdef XFT_DEBUG_FONTSET
    XftPrintFontSet (_XftFontSet);
#endif
    return True;
}

