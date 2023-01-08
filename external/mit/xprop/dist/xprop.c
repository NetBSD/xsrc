/*

Copyright 1990, 1998  The Open Group
Copyright (c) 2000  The XFree86 Project, Inc.

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.
  
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xutil.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#include <locale.h>
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#ifndef HAVE_WCTYPE_H
#define iswprint(x) isprint(x)
#endif

#include <X11/Xatom.h>

#include "dsimple.h"

#define MAXSTR 500000
#define MAXELEMENTS 64

#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

/* isprint() in "C" locale */
#define c_isprint(c) ((c) >= 0x20 && (c) < 0x7f)

static unsigned int term_width = 144 + 8;

/*
 *
 * The Thunk Manager - routines to create, add to, and free thunk lists
 *
 */

typedef struct {
  int thunk_count;
  const char *propname;
  long value;
  Atom extra_encoding;
  const char *extra_value;
  const char *format;
  const char *dformat;
} thunk;

static thunk *
Create_Thunk_List (void)
{
    thunk *tptr;

    tptr = malloc(sizeof(thunk));
    if (!tptr)
	Fatal_Error("Out of memory!");

    tptr->thunk_count = 0;

    return tptr;
}

static thunk *
Add_Thunk (thunk *list, thunk t)
{
    int i;

    i = list->thunk_count;

    list = realloc(list, (i+1)*sizeof(thunk));
    if (!list)
	Fatal_Error("Out of memory!");

    list[i++] = t;
    list->thunk_count = i;

    return list;
}

/*
 * Misc. routines
 */

static int
Read_Char (FILE *stream)
{
    int c;

    c = getc(stream);
    if (c == EOF)
	Fatal_Error("Bad format file: Unexpected EOF.");
    return c;
}

static void
Read_White_Space (FILE *stream)
{
    int c;

    while ((c = getc(stream)) == ' ' || c == '\n' || c == '\t');
    ungetc(c, stream);
}

static char _large_buffer[MAXSTR+10];

static char *
Read_Quoted (FILE *stream)
{
    char *ptr;
    int length;

    Read_White_Space(stream);
    if (Read_Char(stream)!='\'')
	Fatal_Error("Bad format file format: missing dformat.");

    ptr = _large_buffer; length = MAXSTR;
    for (;;) {
	int c;

	if (length < 0)
	    Fatal_Error("Bad format file format: dformat too long.");
	c = Read_Char(stream);
	if (c == (int) '\'')
	    break;
	ptr++[0] = c; length--;
	if (c == (int) '\\') {
	    c = Read_Char(stream);
	    if (c == '\n') {
		ptr--; length++;
	    } else {
		ptr++[0] = c; length--;
	    }
	}
    }
    ptr++[0] = '\0';

    ptr = strdup(_large_buffer);
    if (!ptr)
	Fatal_Error("Out of memory!");
    return ptr;
}

/*
 *
 * Parsing Routines: a group of routines to parse strings into values
 *
 * Routines: Parse_Atom, Scan_Long, Skip_Past_Right_Paren, Scan_Octal
 *
 * Routines of the form Parse_XXX take a string which is parsed to a value.
 * Routines of the form Scan_XXX take a string, parse the beginning to a value,
 * and return the rest of the string.  The value is returned via. the last
 * parameter.  All numeric values are longs!
 *
 */

static const char *
Skip_Digits (const char *string)
{
    while (isdigit((unsigned char) string[0])) string++;
    return string;
}

static const char *
Scan_Long (const char *string, long *value)
{
    if (!isdigit((unsigned char) *string))
	Fatal_Error("Bad number: %s.", string);

    *value = atol(string);
    return Skip_Digits(string);
}

static const char *
Scan_Octal (const char *string, unsigned long *value)
{
    if (sscanf(string, "%lo", value)!=1)
	Fatal_Error("Bad octal number: %s.", string);
    return Skip_Digits(string);
}

static Atom
Parse_Atom (const char *name, int only_if_exists)
{
    /* may return None = 0 */
    return XInternAtom(dpy, name, only_if_exists);
}

static const char *
Skip_Past_Right_Paren (const char *string)
{
    char c;
    int nesting = 0;

    while (c = string++[0], c != ')' || nesting)
	switch (c) {
	  case '\0':
	    Fatal_Error("Missing ')'.");
	  case '(':
	    nesting++;
	    break;
	  case ')':
	    nesting--;
	    break;
	  case '\\':
	    string++;
	    break;
	}
    return string;
}

/*
 *
 * Atom to format, dformat mapping Manager
 *
 */

#define D_FORMAT "0x"              /* Default format for properties */
#define D_DFORMAT " = $0+\n"       /* Default display pattern for properties */

static thunk *_property_formats = NULL;   /* Holds mapping */

static void
Apply_Default_Formats (const char **format, const char **dformat)
{
    if (!*format)
	*format = D_FORMAT;
    if (!*dformat)
	*dformat = D_DFORMAT;
}

static void
Lookup_Formats (Atom atom, const char **format, const char **dformat)
{
    if (_property_formats)
	for (int i = _property_formats->thunk_count-1; i >= 0; i--) {
	    if (_property_formats[i].value == (long) atom) {
		if (!*format)
		    *format = _property_formats[i].format;
		if (!*dformat)
		    *dformat = _property_formats[i].dformat;
		break;
	    }
	}
}

static void
Add_Mapping (Atom atom, const char *format, const char *dformat)
{
    thunk t = {0};

    if (!_property_formats)
	_property_formats = Create_Thunk_List();

    t.value = atom;
    t.format = format;
    t.dformat = dformat;

    _property_formats = Add_Thunk(_property_formats, t);
}

/*
 *
 * Setup_Mapping: Routine to setup default atom to format, dformat mapping:
 * 
 */

typedef struct _propertyRec {
    const char *	name;
    Atom		atom;
    const char *	format;
    const char *	dformat;
} propertyRec;

#define ARC_DFORMAT	":\n"\
"\t\tarc at $0, $1\n"\
"\t\tsize: $2 by $3\n"\
"\t\tfrom angle $4 to angle $5\n"

#define RECTANGLE_DFORMAT	":\n"\
"\t\tupper left corner: $0, $1\n"\
"\t\tsize: $2 by $3\n"

#define RGB_COLOR_MAP_DFORMAT	":\n"\
"\t\tcolormap id #: $0\n"\
"\t\tred-max: $1\n"\
"\t\tred-mult: $2\n"\
"\t\tgreen-max: $3\n"\
"\t\tgreen-mult: $4\n"\
"\t\tblue-max: $5\n"\
"\t\tblue-mult: $6\n"\
"\t\tbase-pixel: $7\n"\
"\t\tvisual id #: $8\n"\
"\t\tkill id #: $9\n"

#define WM_HINTS_DFORMAT	":\n"\
"?m0(\t\tClient accepts input or input focus: $1\n)"\
"?m1(\t\tInitial state is "\
"?$2=0(Don't Care State)"\
"?$2=1(Normal State)"\
"?$2=2(Zoomed State)"\
"?$2=3(Iconic State)"\
"?$2=4(Inactive State)"\
".\n)"\
"?m2(\t\tbitmap id # to use for icon: $3\n)"\
"?m5(\t\tbitmap id # of mask for icon: $7\n)"\
"?m3(\t\twindow id # to use for icon: $4\n)"\
"?m4(\t\tstarting position for icon: $5, $6\n)"\
"?m6(\t\twindow id # of group leader: $8\n)"\
"?m8(\t\tThe urgency hint bit is set\n)"

#define WM_ICON_SIZE_DFORMAT	":\n"\
"\t\tminimum icon size: $0 by $1\n"\
"\t\tmaximum icon size: $2 by $3\n"\
"\t\tincremental size change: $4 by $5\n"

#define WM_SIZE_HINTS_DFORMAT ":\n"\
"?m0(\t\tuser specified location: $1, $2\n)"\
"?m2(\t\tprogram specified location: $1, $2\n)"\
"?m1(\t\tuser specified size: $3 by $4\n)"\
"?m3(\t\tprogram specified size: $3 by $4\n)"\
"?m4(\t\tprogram specified minimum size: $5 by $6\n)"\
"?m5(\t\tprogram specified maximum size: $7 by $8\n)"\
"?m6(\t\tprogram specified resize increment: $9 by $10\n)"\
"?m7(\t\tprogram specified minimum aspect ratio: $11/$12\n"\
"\t\tprogram specified maximum aspect ratio: $13/$14\n)"\
"?m8(\t\tprogram specified base size: $15 by $16\n)"\
"?m9(\t\twindow gravity: "\
"?$17=0(Forget)"\
"?$17=1(NorthWest)"\
"?$17=2(North)"\
"?$17=3(NorthEast)"\
"?$17=4(West)"\
"?$17=5(Center)"\
"?$17=6(East)"\
"?$17=7(SouthWest)"\
"?$17=8(South)"\
"?$17=9(SouthEast)"\
"?$17=10(Static)"\
"\n)"

#define WM_STATE_DFORMAT	 ":\n"\
"\t\twindow state: ?$0=0(Withdrawn)?$0=1(Normal)?$0=3(Iconic)\n"\
"\t\ticon window: $1\n"

static propertyRec windowPropTable[] = {
    {"ARC",		XA_ARC,		"16iiccii",   ARC_DFORMAT },
    {"ATOM",		XA_ATOM,	 "32a",	      0 },
    {"BITMAP",		XA_BITMAP,	 "32x",	      ": bitmap id # $0\n" },
    {"CARDINAL",	XA_CARDINAL,	 "0c",	      0 },
    {"COLORMAP",	XA_COLORMAP,	 "32x",	      ": colormap id # $0\n" },
    {"CURSOR",		XA_CURSOR,	 "32x",	      ": cursor id # $0\n" },
    {"DRAWABLE",	XA_DRAWABLE,	 "32x",	      ": drawable id # $0\n" },
    {"FONT",		XA_FONT,	 "32x",	      ": font id # $0\n" },
    {"INTEGER",		XA_INTEGER,	 "0i",	      0 },
    {"PIXMAP",		XA_PIXMAP,	 "32x",	      ": pixmap id # $0\n" },
    {"POINT",		XA_POINT,	 "16ii",      " = $0, $1\n" },
    {"RECTANGLE",	XA_RECTANGLE,	 "16iicc",    RECTANGLE_DFORMAT },
    {"RGB_COLOR_MAP",	XA_RGB_COLOR_MAP,"32xcccccccxx",RGB_COLOR_MAP_DFORMAT},
    {"STRING",		XA_STRING,	 "8s",	      0 },
    {"UTF8_STRING",		0,	 "8u",	      0 },
    {"WINDOW",		XA_WINDOW,	 "32x",	      ": window id # $0+\n" },
    {"VISUALID",	XA_VISUALID,	 "32x",	      ": visual id # $0\n" },
    {"WM_COLORMAP_WINDOWS",	0,	 "32x",       ": window id # $0+\n"},
    {"WM_COMMAND",	XA_WM_COMMAND,	 "8s",	      " = { $0+ }\n" },
    {"WM_HINTS",	XA_WM_HINTS,	 "32mbcxxiixx",	WM_HINTS_DFORMAT },
    {"WM_ICON_NAME",	XA_WM_ICON_NAME, "8t",	      0 },
    {"WM_ICON_SIZE",	XA_WM_ICON_SIZE, "32cccccc",  WM_ICON_SIZE_DFORMAT},
    {"WM_NAME",		XA_WM_NAME,	 "8t",	      0 },
    {"WM_PROTOCOLS",		0,	 "32a",	      ": protocols  $0+\n"},
    {"WM_SIZE_HINTS",	XA_WM_SIZE_HINTS,"32mii",     WM_SIZE_HINTS_DFORMAT },
    {"_NET_WM_ICON",            0,       "32o",        0 },
    {"WM_STATE",		0,	 "32cx",      WM_STATE_DFORMAT}
};
#undef ARC_DFORMAT
#undef RECTANGLE_DFORMAT
#undef RGB_COLOR_MAP_DFORMAT
#undef WM_ICON_SIZE_DFORMAT
#undef WM_HINTS_DFORMAT
#undef WM_SIZE_HINTS_DFORMAT
#undef WM_STATE_DFORMAT

/* 
 * Font-specific mapping of property names to types:
 */
static propertyRec fontPropTable[] = {

    /* XLFD name properties */

    { "FOUNDRY",		0, 	 		"32a",	0 },
    { "FAMILY_NAME",		XA_FAMILY_NAME,		"32a",	0 },
    { "WEIGHT_NAME",		0,			"32a",	0 },
    { "SLANT",			0,			"32a",	0 },
    { "SETWIDTH_NAME",		0,			"32a",	0 },
    { "ADD_STYLE_NAME",		0,			"32a",	0 },
    { "PIXEL_SIZE",		0,			"32c",	0 },
    { "POINT_SIZE",		XA_POINT_SIZE,		"32c",	0 },
    { "RESOLUTION_X",		0,			"32c",	0 },
    { "RESOLUTION_Y",		0,			"32c",	0 },
    { "SPACING",		0,			"32a",	0 },
    { "AVERAGE_WIDTH",		0,			"32c",	0 },
    { "CHARSET_REGISTRY",	0,			"32a",	0 },
    { "CHARSET_ENCODING",	0,			"32a",	0 },

    /* other font properties referenced in the XLFD */

    { "QUAD_WIDTH",		XA_QUAD_WIDTH,		"32i",	0 },
    { "RESOLUTION",		XA_RESOLUTION,		"32c",	0 },
    { "MIN_SPACE",		XA_MIN_SPACE,		"32c",	0 },
    { "NORM_SPACE",		XA_NORM_SPACE,		"32c",	0 },
    { "MAX_SPACE",		XA_MAX_SPACE,		"32c",	0 },
    { "END_SPACE",		XA_END_SPACE,		"32c",	0 },
    { "SUPERSCRIPT_X",		XA_SUPERSCRIPT_X,	"32i",	0 },
    { "SUPERSCRIPT_Y",		XA_SUPERSCRIPT_Y,	"32i",	0 },
    { "SUBSCRIPT_X",		XA_SUBSCRIPT_X,		"32i",	0 },
    { "SUBSCRIPT_Y",		XA_SUBSCRIPT_Y,		"32i",	0 },
    { "UNDERLINE_POSITION",	XA_UNDERLINE_POSITION,	"32i",	0 },
    { "UNDERLINE_THICKNESS",	XA_UNDERLINE_THICKNESS,	"32i",	0 },
    { "STRIKEOUT_ASCENT",	XA_STRIKEOUT_ASCENT,	"32i",	0 },
    { "STRIKEOUT_DESCENT",	XA_STRIKEOUT_DESCENT,	"32i",	0 },
    { "ITALIC_ANGLE",		XA_ITALIC_ANGLE,	"32i",	0 },
    { "X_HEIGHT",		XA_X_HEIGHT,		"32i",	0 },
    { "WEIGHT",			XA_WEIGHT,		"32i",	0 },
    { "FACE_NAME",		0,			"32a",	0 },
    { "COPYRIGHT",		XA_COPYRIGHT,		"32a",	0 },
    { "AVG_CAPITAL_WIDTH",	0,			"32i",	0 },
    { "AVG_LOWERCASE_WIDTH",	0,			"32i",	0 },
    { "RELATIVE_SETWIDTH",	0,			"32c",	0 },
    { "RELATIVE_WEIGHT",	0,			"32c",	0 },
    { "CAP_HEIGHT",		XA_CAP_HEIGHT,		"32c",	0 },
    { "SUPERSCRIPT_SIZE",	0,			"32c",	0 },
    { "FIGURE_WIDTH",		0,			"32i",	0 },
    { "SUBSCRIPT_SIZE",		0,			"32c",	0 },
    { "SMALL_CAP_SIZE",		0,			"32i",	0 },
    { "NOTICE",			XA_NOTICE,		"32a",	0 },
    { "DESTINATION",		0,			"32c",	0 },

    /* other font properties */

    { "FONT",			XA_FONT,		"32a",	0 },
    { "FONT_NAME",		XA_FONT_NAME,		"32a",	0 },
};    

static int XpropMode;
#define XpropWindowProperties 0
#define XpropFontProperties   1

static void
Setup_Mapping (void)
{
    int n;
    propertyRec *p;
    
    if (XpropMode == XpropWindowProperties) {
	n = sizeof(windowPropTable) / sizeof(propertyRec);
	p = windowPropTable;
    } else {
	n = sizeof (fontPropTable) / sizeof (propertyRec);
	p = fontPropTable;
    }
    for ( ; --n >= 0; p++) {
	if (! p->atom) {
	    p->atom = XInternAtom(dpy, p->name, True);
	    if (p->atom == None)
		continue;
	}
	Add_Mapping(p->atom, p->format, p->dformat);
    }	
}

static const char *
GetAtomName (Atom atom)
{
    int n;
    propertyRec *p;

    if (XpropMode == XpropWindowProperties) {
	n = sizeof(windowPropTable) / sizeof(propertyRec);
	p = windowPropTable;
    } else {
	n = sizeof (fontPropTable) / sizeof (propertyRec);
	p = fontPropTable;
    }
    for ( ; --n >= 0; p++)
	if (p->atom == atom)
	    return p->name;

    return NULL;
}

/*
 * Read_Mapping: routine to read in additional mappings from a stream
 *               already open for reading.
 */

static void
Read_Mappings (FILE *stream)
{
    char format_buffer[100];
    char name[1000];
    int count;

    while ((count = fscanf(stream," %990s %90s ",name,format_buffer)) != EOF) {
	const char *dformat, *format;
	int c;
	Atom atom;

	if (count != 2)
	    Fatal_Error("Bad format file format.");

	atom = Parse_Atom(name, False);
	format = strdup(format_buffer);
	if (!format)
	    Fatal_Error("Out of memory!");

	Read_White_Space(stream);
	dformat = D_DFORMAT;
	c = getc(stream);
	ungetc(c, stream);
	if (c == (int) '\'')
	    dformat = Read_Quoted(stream);

	Add_Mapping(atom, format, dformat);
    }
}

/*
 *
 * Formatting Routines: a group of routines to translate from various
 *                      values to a static read-only string useful for output.
 *
 * Routines: Format_Hex, Format_Unsigned, Format_Signed, Format_Atom,
 *           Format_Mask_Word, Format_Bool, Format_String, Format_Len_String.
 *
 * All of the above routines take a long except for Format_String and
 * Format_Len_String.
 *
 */
static char _formatting_buffer[MAXSTR+100];
static char _formatting_buffer2[21];

static const char *
Format_Hex (long wrd)
{
    snprintf(_formatting_buffer2, sizeof(_formatting_buffer2), "0x%lx", wrd);
    return _formatting_buffer2;
}

static const char *
Format_Unsigned (long wrd)
{
    snprintf(_formatting_buffer2, sizeof(_formatting_buffer2), "%lu", wrd);
    return _formatting_buffer2;
}

static const char *
Format_Signed (long wrd)
{
    snprintf(_formatting_buffer2, sizeof(_formatting_buffer2), "%ld", wrd);
    return _formatting_buffer2;
}

/*ARGSUSED*/
static int
ignore_errors (Display *display, XErrorEvent *ev)
{
    return 0;
}

static const char *
Format_Atom (Atom atom)
{
    const char *found;
    char *name;
    XErrorHandler handler;

    if ((found = GetAtomName(atom)) != NULL)
	return found;

    handler = XSetErrorHandler (ignore_errors);
    name = XGetAtomName(dpy, atom);
    XSetErrorHandler(handler);
    if (! name)
	snprintf(_formatting_buffer, sizeof(_formatting_buffer),
		 "undefined atom # 0x%lx", atom);
    else {
	size_t namelen = strlen(name);
	if (namelen > MAXSTR) namelen = MAXSTR;
	memcpy(_formatting_buffer, name, namelen);
	_formatting_buffer[namelen] = '\0';
	XFree(name);
    }
    return _formatting_buffer;
}

static const char *
Format_Mask_Word (long wrd)
{
    unsigned long bit_mask, bit;
    int seen = 0;

    strcpy(_formatting_buffer, "{MASK: ");
    for (bit=0, bit_mask=1; bit <= sizeof(long)*8; bit++, bit_mask<<=1) {
	if (bit_mask & wrd) {
	    if (seen) {
		strcat(_formatting_buffer, ", ");
	    }
	    seen = 1;
	    strcat(_formatting_buffer, Format_Unsigned(bit));
	}
    }
    strcat(_formatting_buffer, "}");

    return _formatting_buffer;
}

static const char *
Format_Bool (long value)
{
    if (!value)
	return "False";

    return "True";
}

static char *_buf_ptr;
static int _buf_len;

static void
_put_char (char c)
{
    if (_buf_len <= 0) {
	_buf_ptr[0] = '\0';
	return;
    }
    _buf_ptr++[0] = c;
    _buf_len--;
}

static void
_format_char (char c, int unicode)
{
    switch (c) {
      case '\\':
      case '\"':
	_put_char('\\');
	_put_char(c);
	break;
      case '\n':
	_put_char('\\');
	_put_char('n');
	break;
      case '\t':
	_put_char('\\');
	_put_char('t');
	break;
      default:
	if (!c_isprint(c)) {
	    if (unicode && (c & 0x80)) {
		_put_char(c);
	    } else {
		_put_char('\\');
		snprintf(_buf_ptr, _buf_len, "%03o", (unsigned char) c);
		_buf_ptr += 3;
		_buf_len -= 3;
	    }
	} else
	  _put_char(c);
    }
}

static const char *
Format_String (const char *string, int unicode)
{
    char c;

    _buf_ptr = _formatting_buffer;
    _buf_len = MAXSTR;
    _put_char('\"');

    while ((c = string++[0]))
	_format_char(c, unicode);

    *_buf_ptr++ = '"';
    *_buf_ptr++ = '\0';
    return _formatting_buffer;
}

static const char *
Format_Len_String (const char *string, int len, int unicode)
{
    char *data;
    const char *result;

    data = malloc(len+1);
    if (!data)
	Fatal_Error("Out of memory!");

    memcpy(data, string, len);
    data[len] = '\0';

    result = Format_String(data, unicode);
    free(data);

    return result;
}  

static int
is_utf8_locale (void)
{
#ifdef HAVE_LANGINFO_H
    char *charmap = nl_langinfo (CODESET);

    return charmap && strcmp (charmap, "UTF-8") == 0;
#else
    return 0;
#endif
}

static int
is_truecolor_term (void)
{
    char *colorterm = getenv( "COLORTERM" );

    if (colorterm && !strcmp(colorterm,"truecolor"))
	return 1;

    return 0;
}

static const char *
Format_Icons (const unsigned long *icon, int len)
{
    static char *result = NULL;
    char *tail = NULL;
    int alloced;
    const unsigned long *end = icon + len / sizeof (unsigned long);

    free(result);
    result = NULL;

    alloced = 0;

    while (icon < end)
    {
	unsigned long width, height, display_width;
	unsigned int icon_pixel_bytes;
	unsigned int icon_line_bytes;
	int offset;
	
	width = *icon++;
	height = *icon++;
	display_width = width * 2; /* Two characters per icon pixel. */

	icon_pixel_bytes = 1;
	if (is_truecolor_term())
	    icon_pixel_bytes = 25; /* 16 control characters, and up to 9 chars of RGB. */
	else if (is_utf8_locale())
	    icon_pixel_bytes = 3; /* Up to 3 bytes per character in that mode. */

	/* Initial tab, pixels, and newline. */
	icon_line_bytes = 8 + display_width * icon_pixel_bytes + 1;

	offset = (tail - result);
	
	alloced += 80;				/* For the header, final newline, color reset */
	alloced += icon_line_bytes * height;	/* For the rows */
	
	result = realloc (result, alloced);
	if (!result)
	    Fatal_Error("Out of memory!");
	tail = &result[offset];

	if (end - icon < width * height)
	    break;

	tail += sprintf (tail, "\tIcon (%lu x %lu):\n", width, height);

	if ((display_width + 8) > term_width || height > 144)
	{
	    tail += sprintf (tail, "\t(not shown)\n");
	    icon += width * height;
	    continue;
	}
	
	for (unsigned int h = 0; h < height; ++h)
	{
	    tail += sprintf (tail, "\t");
	    
	    for (unsigned int w = 0; w < width; ++w)
	    {
		unsigned char a, r, g, b;
		unsigned long pixel = *icon++;
		unsigned long brightness;
		
		a = (pixel & 0xff000000) >> 24;
		r = (pixel & 0x00ff0000) >> 16;
		g = (pixel & 0x0000ff00) >> 8;
		b = (pixel & 0x000000ff);
		
		brightness =
		    (a / 255.0) * (1000 - ((299 * (r / 255.0)) +
					   (587 * (g / 255.0)) +
					   (114 * (b / 255.0))));

		if (is_truecolor_term())
		{
		    float opacity = a / 255.0;

		    r = r * opacity;
		    g = g * opacity;
		    b = b * opacity;

		    tail += sprintf (tail, "\033[38;2;%d;%d;%dm\342\226\210\342\226\210", r, g, b );
		}
		else if (is_utf8_locale())
		{
		    static const char palette[][4] =
		    {
			" ",
			"\342\226\221",		/* 25% */
			"\342\226\222",		/* 50% */
			"\342\226\223",		/* 75% */
			"\342\226\210",		/* 100% */
		    };
		    int idx;

		    idx = (brightness * ((sizeof (palette)/sizeof(palette[0])) - 1)) / 1000;

		    tail += sprintf (tail, "%s%s", palette[idx], palette[idx]);
		}
		else
		{
		    static const char palette[] =
			" .'`,^:\";~-_+<>i!lI?/\\|()1{}[]rcvunxzjftLCJUYXZO0Qoahkbdpqwm*WMB8&%$#@";
		    int idx;
		    
		    idx = (brightness * (sizeof(palette) - 2)) / 1000;
		    
		    *tail++ = palette[idx];
		    *tail++ = palette[idx];
		}
	    }

	    tail += sprintf (tail, "\n");
	}

	/* Reset colors. */
	if (is_truecolor_term())
	    tail += sprintf (tail, "\033[0m");

	tail += sprintf (tail, "\n");
    }

    return result;
}

static const char *
Format_Len_Text (const char *string, int len, Atom encoding)
{
    XTextProperty textprop;
    char **start_list;
    int count;

    /* Try to convert to local encoding. */
    textprop.encoding = encoding;
    textprop.format = 8;
    textprop.value = (unsigned char *) string;
    textprop.nitems = len;
    if (XmbTextPropertyToTextList(dpy, &textprop, &start_list, &count) == Success) {
	char **list = start_list;
	_buf_ptr = _formatting_buffer;
	_buf_len = MAXSTR;
	*_buf_ptr++ = '"';
	while (count > 0) {
	    string = *list++;
	    len = strlen(string);
	    while (len > 0) {
		wchar_t wc;
		int n = mbtowc(&wc, string, len);
		if (n > 0 && iswprint(wc)) {
		    if (_buf_len >= n) {
			memcpy(_buf_ptr, string, n);
			_buf_ptr += n;
			_buf_len -= n;
		    }
		    string += n;
		    len -= n;
		} else {
		    _put_char('\\');
		    snprintf(_buf_ptr, _buf_len, "%03o", (unsigned char) *string);
		    _buf_ptr += 3;
		    _buf_len -= 3;
		    string++;
		    len--;
		}
	    }
	    count--;
	    if (count > 0) {
		snprintf(_buf_ptr, _buf_len, "\\000");
	        _buf_ptr += 4;
		_buf_len -= 4;
	    }
	}
	XFreeStringList(start_list);
	*_buf_ptr++ = '"';
	*_buf_ptr++ = '\0';
	return _formatting_buffer;
    } else
	return Format_Len_String(string, len, 0);
}

/*
 * Validate a string as UTF-8 encoded according to RFC 3629
 *
 * Simply, a unicode code point (up to 21-bits long) is encoded as follows:
 *
 *    Char. number range  |        UTF-8 octet sequence
 *       (hexadecimal)    |              (binary)
 *    --------------------+---------------------------------------------
 *    0000 0000-0000 007F | 0xxxxxxx
 *    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * Validation is done left-to-right, and an error condition, if any, refers to
 * only the left-most problem in the string.
 *
 * Return values:
 *   UTF8_VALID: Valid UTF-8 encoded string
 *   UTF8_OVERLONG: Using more bytes than needed for a code point
 *   UTF8_SHORT_TAIL: Not enough bytes in a multi-byte sequence
 *   UTF8_LONG_TAIL: Too many bytes in a multi-byte sequence
 *   UTF8_FORBIDDEN_VALUE: Forbidden prefix or code point outside 0x10FFFF
 */
#define UTF8_VALID 0
#define UTF8_FORBIDDEN_VALUE 1
#define UTF8_OVERLONG 2
#define UTF8_SHORT_TAIL 3
#define UTF8_LONG_TAIL 4
static int
is_valid_utf8 (const char *string, int len)
{
    unsigned long codepoint = 0;
    int rem = 0;

    for (int i = 0; i < len; i++) {
	 unsigned char c = (unsigned char) string[i];

	/* Order of type check:
	 *   - Single byte code point
	 *   - Non-starting byte of multi-byte sequence
	 *   - Start of 2-byte sequence
	 *   - Start of 3-byte sequence
	 *   - Start of 4-byte sequence
	 */
	if (!(c & 0x80)) {
	    if (rem > 0) return UTF8_SHORT_TAIL;
	    rem = 0;
	    codepoint = c;
	} else if ((c & 0xC0) == 0x80) {
	    if (rem == 0) return UTF8_LONG_TAIL;
	    rem--;
	    codepoint |= (c & 0x3F) << (rem * 6);
	    if (codepoint == 0) return UTF8_OVERLONG;
	} else if ((c & 0xE0) == 0xC0) {
	    if (rem > 0) return UTF8_SHORT_TAIL;
	    rem = 1;
	    codepoint = (c & 0x1F) << 6;
	    if (codepoint == 0) return UTF8_OVERLONG;
	} else if ((c & 0xF0) == 0xE0) {
	    if (rem > 0) return UTF8_SHORT_TAIL;
	    rem = 2;
	    codepoint = (c & 0x0F) << 12;
	} else if ((c & 0xF8) == 0xF0) {
	    if (rem > 0) return UTF8_SHORT_TAIL;
	    rem = 3;
	    codepoint = (c & 0x07) << 18;
	    if (codepoint > 0x10FFFF) return UTF8_FORBIDDEN_VALUE;
	} else
	    return UTF8_FORBIDDEN_VALUE;
    }

    return UTF8_VALID;
}

static const char *
Format_Len_Unicode (const char *string, int len)
{
    int validity = is_valid_utf8(string, len);

    if (validity != UTF8_VALID) {
	char *data;
	const char *result, *error;

	switch (validity) {
	  case UTF8_FORBIDDEN_VALUE:
	    error = "<Invalid UTF-8 string: Forbidden value> "; break;
	  case UTF8_OVERLONG:
	    error = "<Invalid UTF-8 string: Overlong encoding> "; break;
	  case UTF8_SHORT_TAIL:
	    error = "<Invalid UTF-8 string: Tail too short> "; break;
	  case UTF8_LONG_TAIL:
	    error = "<Invalid UTF-8 string: Tail too long> "; break;
	  default:
	    error = "<Invalid UTF-8 string: Unknown error>"; break;
	}

	result = Format_Len_String(string, len, 0);
	/* result is stored in _formatting_buffer, so make a temporary
	   copy before we overwrite _formatting_buffer with error */
	data = strdup(result);
	if (!data)
	    Fatal_Error("Out of memory!");

	memcpy(_formatting_buffer, error, strlen(error)+1);
	strcat(_formatting_buffer, data);
	free(data);

	return _formatting_buffer;
    }

    return Format_Len_String(string, len, is_utf8_locale());
}

/*
 *
 * The Format Manager: a group of routines to manage "formats"
 *
 */

static int
Is_A_Format (const char *string)
{
    return isdigit((unsigned char) string[0]);
}

static int
Get_Format_Size (const char *format)
{
    long size;

    Scan_Long(format, &size);

    /* Check for legal sizes */
    if (size != 0 && size != 8 && size != 16 && size != 32)
	Fatal_Error("bad format: %s", format);

    return (int) size;
}

static char
Get_Format_Char (const char *format, int i)
{
    long size;

    /* Remove # at front of format */
    format = Scan_Long(format, &size);
    if (!*format)
	Fatal_Error("bad format: %s", format);

    /* Last character repeats forever... */
    if (i >= (int)strlen(format))
	i = strlen(format)-1;

    return format[i];
}

static const char *
Format_Thunk (thunk t, char format_char)
{
    long value;
    value = t.value;

    switch (format_char) {
      case 's':
	return Format_Len_String(t.extra_value, (int)t.value, 0);
      case 'u':
	return Format_Len_Unicode(t.extra_value, (int)t.value);
      case 't':
	return Format_Len_Text(t.extra_value, (int)t.value, t.extra_encoding);
      case 'x':
	return Format_Hex(value);
      case 'c':
	return Format_Unsigned(value);
      case 'i':
	return Format_Signed(value);
      case 'b':
	return Format_Bool(value);
      case 'm':
	return Format_Mask_Word(value);
      case 'a':
	return Format_Atom(value);
      case 'o':
	  return Format_Icons((const unsigned long *)t.extra_value, (int)t.value);
      default:
	Fatal_Error("bad format character: %c", format_char);
    }
}

static const char *
Format_Thunk_I (thunk *thunks, const char *format, int i)
{
    if (i >= thunks->thunk_count)
	return "<field not available>";

    return Format_Thunk(thunks[i], Get_Format_Char(format, i));
}

static long
Mask_Word (thunk *thunks, const char *format)
{
    for (int j = 0; j  < (int)strlen(format); j++)
	if (Get_Format_Char(format, j) == 'm')
	    return thunks[j].value;
    return 0;
}

/*
 *
 * The Display Format Manager:
 *
 */

static int
Is_A_DFormat (const char *string)
{
    return string[0] && string[0] != '-'
	   && !(isalpha((unsigned char) string[0]) || string[0] == '_');
}

static const char *
Handle_Backslash (const char *dformat)
{
    char c;
    unsigned long i;

    if (!(c = *(dformat++)))
	return dformat;

    switch (c) {
      case 'n':
	putchar('\n');
	break;
      case 't':
	putchar('\t');
	break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
	dformat = Scan_Octal(dformat, &i);
	putchar((int) i);
	break;
      default:
	putchar(c);
	break;
    }
    return dformat;
}

static const char *
Handle_Dollar_sign (const char *dformat, thunk *thunks, const char *format)
{
    long i;

    dformat = Scan_Long(dformat, &i);

    if (dformat[0] == '+') {
	int seen = 0;
	dformat++;
	for (; i < thunks->thunk_count; i++) {
	    if (seen)
		printf(", ");
	    seen = 1;
	    printf("%s", Format_Thunk_I(thunks, format, (int) i));
	}
    } else
	printf("%s", Format_Thunk_I(thunks, format, (int) i));

    return dformat;
}

static int
Mask_Bit_I (thunk *thunks, const char *format, int i)
{
    long value;

    value = Mask_Word(thunks, format);

    value = value & (1L<<i);
    if (value)
	value = 1;
    return value;
}

static const char *
Scan_Term (const char *string, thunk *thunks, const char *format, long *value)
{
    long i;

    *value = 0;

    if (isdigit((unsigned char) *string))
	string = Scan_Long(string, value);
    else if (*string == '$') {
	string = Scan_Long(++string, &i);
	if (i >= thunks->thunk_count)
	    i = thunks->thunk_count;
	*value = thunks[i].value;
    } else if (*string == 'm') {
	string = Scan_Long(++string, &i);
	*value = Mask_Bit_I(thunks, format, (int) i);
    } else
	Fatal_Error("Bad term: %s.", string);

    return string;
}

static const char *
Scan_Exp (const char *string, thunk *thunks, const char *format, long *value)
{
    long temp;

    if (string[0] == '(') {
	string = Scan_Exp(++string, thunks, format, value);
	if (string[0]!=')')
	    Fatal_Error("Missing ')'");
	return ++string;
    }
    if (string[0] == '!') {
	string = Scan_Exp(++string, thunks, format, value);
	*value = !*value;
	return string;
    }

    string = Scan_Term(string, thunks, format, value);

    if (string[0] == '=') {
	string = Scan_Exp(++string, thunks, format, &temp);
	*value = *value == temp;
    }

    return string;
}

static const char *
Handle_Question_Mark (const char *dformat, thunk *thunks, const char *format)
{
    long true;

    dformat = Scan_Exp(dformat, thunks, format, &true);

    if (*dformat != '(')
	Fatal_Error("Bad conditional: '(' expected: %s.", dformat);
    ++dformat;

    if (!true)
	dformat = Skip_Past_Right_Paren(dformat);

    return dformat;
}

static void
Display_Property (thunk *thunks, const char *dformat, const char *format)
{
    char c;

    while ((c = *(dformat++)))
	switch (c) {
	  case ')':
	    continue;
	  case '\\':
	    dformat = Handle_Backslash(dformat);
	    continue;
	  case '$':
	    dformat = Handle_Dollar_sign(dformat, thunks, format);
	    continue;
	  case '?':
	    dformat = Handle_Question_Mark(dformat, thunks, format);
	    continue;
	  default:
	    putchar(c);
	    continue;
	}
}

/*
 *
 * Routines to convert property data to thunks
 *
 */

static long
Extract_Value (const char **pointer, int *length, int size, int signedp)
{
    long value;

    switch (size) {
      case 8:
	if (signedp)
	    value = * (const signed char *) *pointer;
	else
	    value = * (const unsigned char *) *pointer;
	*pointer += 1;
	*length -= 1;
	break;
      case 16:
	if (signedp)
	    value = * (const short *) *pointer;
	else
	    value = * (const unsigned short *) *pointer;
	*pointer += sizeof(short);
	*length -= sizeof(short);
	break;
      case 32:
	if (signedp)
	    value = * (const long *) *pointer;
	else
	    value = * (const unsigned long *) *pointer & 0xffffffff;
	*pointer += sizeof(long);
	*length -= sizeof(long);
	break;
      default:
	abort();
    }
    return value;
}

static long
Extract_Len_String (const char **pointer, int *length, int size, const char **string)
{
    int len;

    if (size != 8)
	Fatal_Error("can't use format character 's' with any size except 8.");
    len = 0; *string = *pointer;
    while ((len++, --*length, *((*pointer)++)) && *length>0);

    return len;
}

static long
Extract_Icon (const char **pointer, int *length, int size, const char **icon)
{
    int len = 0;

    if (size != 32)
	Fatal_Error("can't use format character 'o' with any size except 32.");

    len = *length;
    *icon = *pointer;
    *length = 0;
    return len;
}

static thunk *
Break_Down_Property (const char *pointer, int length, Atom type, const char *format, int size)
{
    thunk *thunks;
    thunk t = {0};
    int i;

    thunks = Create_Thunk_List();
    i = 0;

    while (length >= size/8) {
	char format_char = Get_Format_Char(format, i);

	if (format_char == 's' || format_char == 'u')
	    t.value = Extract_Len_String(&pointer,&length,size,&t.extra_value);
	else if (format_char == 't') {
	    t.extra_encoding = type;
	    t.value = Extract_Len_String(&pointer,&length,size,&t.extra_value);
	}
	else if (format_char == 'o')
	    t.value = Extract_Icon (&pointer,&length,size,&t.extra_value);
	else
	    t.value = Extract_Value(&pointer,&length,size,format_char=='i');
	thunks = Add_Thunk(thunks, t);
	i++;
    }

    return thunks;
}

/*
 * Variables set by main()
 */

static Window target_win = 0;
static int notype = 0;
static int max_len = MAXSTR;
static XFontStruct *font;
static unsigned long _font_prop;

/*
 *
 * Other Stuff (temp.):
 *
 */

static const char *
Get_Font_Property_Data_And_Type (Atom atom,
                                 long *length, Atom *type, int *size)
{
    *type = None;
	
    for (int i = 0; i < font->n_properties; i++)
	if (atom == font->properties[i].name) {
	    _font_prop = font->properties[i].card32;
	    *length = sizeof(long);
	    *size = 32;
	    return (const char *) &_font_prop;
	}
    *size = 0;
    return NULL;
}

static const char *
Get_Window_Property_Data_And_Type (Atom atom,
                                   long *length, Atom *type, int *size)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long nbytes;
    unsigned long bytes_after;
    static unsigned char *prop = NULL;
    int status;

    if (prop)
    {
	XFree(prop);
	prop = NULL;
    }

    status = XGetWindowProperty(dpy, target_win, atom, 0, (max_len+3)/4,
				False, AnyPropertyType, &actual_type,
				&actual_format, &nitems, &bytes_after,
				&prop);
    if (status == BadWindow)
	Fatal_Error("window id # 0x%lx does not exists!", target_win);
    if (status != Success)
	Fatal_Error("XGetWindowProperty failed!");

    if (actual_format == 32)
	nbytes = sizeof(long);
    else if (actual_format == 16)
	nbytes = sizeof(short);
    else if (actual_format == 8)
	nbytes = 1;
    else if (actual_format == 0)
        nbytes = 0;
    else
	abort();
    *length = min(nitems * nbytes, max_len);
    *type = actual_type;
    *size = actual_format;
    return (const char *)prop;
}

static const char *
Get_Property_Data_And_Type (Atom atom, long *length, Atom *type, int *size)
{
    if (target_win == (Window) -1)
	return Get_Font_Property_Data_And_Type(atom, length, type, size);
    else
	return Get_Window_Property_Data_And_Type(atom, length, type, size);
}

static void
Show_Prop (const char *format, const char *dformat, const char *prop)
{
    const char *data;
    long length;
    Atom atom, type;
    thunk *thunks;
    int size, fsize;

    printf("%s", prop);
    atom = Parse_Atom(prop, True);
    if (atom == None) {
	printf(":  no such atom on any window.\n");
	return;
    }

    data = Get_Property_Data_And_Type(atom, &length, &type, &size);
    if (!size) {
	puts(":  not found.");
	return;
    }

    if (!notype && type != None)
	printf("(%s)", Format_Atom(type));

    Lookup_Formats(atom, &format, &dformat);
    if (type != None)
	Lookup_Formats(type, &format, &dformat);
    Apply_Default_Formats(&format, &dformat);

    fsize = Get_Format_Size(format);
    if (fsize != size && fsize != 0) {
	printf(": Type mismatch: assumed size %d bits, actual size %d bits.\n",
	       fsize, size);
	return;
    }

    thunks = Break_Down_Property(data, (int)length, type, format, size);
    Display_Property(thunks, dformat, format);
    free(thunks);
}

static void
Show_All_Props (void)
{
    if (target_win != (Window) -1) {
	int count;
	Atom *atoms = XListProperties(dpy, target_win, &count);
	for (int i = 0; i < count; i++) {
	    const char *name = Format_Atom(atoms[i]);
	    Show_Prop(NULL, NULL, name);
	}
	XFree(atoms);
    } else {
	for (int i = 0; i < font->n_properties; i++) {
	    Atom atom = font->properties[i].name;
	    const char *name = Format_Atom(atom);
	    Show_Prop(NULL, NULL, name);
	}
    }
}

static thunk *
Handle_Prop_Requests (int argc, char **argv)
{
    char *format, *dformat, *prop;
    thunk *thunks, t = {0};

    /* if no prop referenced, by default list all properties for given window */
    if (!argc) {
	Show_All_Props();
	return NULL;
    }

    thunks = Create_Thunk_List();

    while (argc > 0) {
	format = NULL;
	dformat = NULL;

	/* Get overriding formats, if any */
	if (Is_A_Format(argv[0])) {
	    format = argv++[0]; argc--;
	    if (!argc) usage("format specified without atom");
	}
	if (Is_A_DFormat(argv[0])) {
	    dformat = argv++[0]; argc--;
	    if (!argc) usage("dformat specified without atom");
	}

	/* Get property name */
	prop = argv++[0]; argc--;

	t.propname = prop;
	t.value = Parse_Atom(prop, True);
	t.format = format;
	t.dformat = dformat;
	if (t.value)
	    thunks = Add_Thunk(thunks, t);
	Show_Prop(format, dformat, prop);
    }
    return thunks;
}

static void
Remove_Property (Display *display, Window w, const char *propname)
{
    Atom id = XInternAtom (display, propname, True);

    if (id == None) {
	fprintf (stderr, "%s:  no such property \"%s\"\n",
		 program_name, propname);
	return;
    }
    XDeleteProperty (display, w, id);
}

static void
Set_Property (Display *display, Window w, const char *propname, const char *value)
{
    Atom atom;
    const char *format;
    const char *dformat;
    int size;
    char format_char;
    Atom type = 0;
    const unsigned char *data = NULL;
    int nelements = 0;

    atom = Parse_Atom(propname, False);

    format = dformat = NULL;
    Lookup_Formats(atom, &format, &dformat);
    if (format == NULL)
	Fatal_Error("unsupported conversion for %s", propname);

    size = Get_Format_Size(format);

    format_char = Get_Format_Char(format, 0);
    switch (format_char) {
      case 's':
	if (size != 8)
	    Fatal_Error("can't use format character 's' with any size except 8.");
	type = XA_STRING;
	data = (const unsigned char *) value;
	nelements = strlen(value);
	break;
      case 'u':
	if (size != 8)
	    Fatal_Error("can't use format character 'u' with any size except 8.");
	type = XInternAtom(display, "UTF8_STRING", False);
	data = (const unsigned char *) value;
	nelements = strlen(value);
	break;
      case 't': {
	XTextProperty textprop;
	if (size != 8)
	    Fatal_Error("can't use format character 't' with any size except 8.");
	if (XmbTextListToTextProperty(display, (char **) &value, 1,
				      XStdICCTextStyle, &textprop) != Success) {
	    fprintf(stderr, "cannot convert %s argument to STRING or COMPOUND_TEXT.\n", propname);
	    return;
	}
	type = textprop.encoding;
	data = textprop.value;
	nelements = textprop.nitems;
	break;
      }
      case 'x':
      case 'c': {
	static unsigned char data8[MAXELEMENTS];
	static unsigned short data16[MAXELEMENTS];
	static unsigned long data32[MAXELEMENTS];
	unsigned long intvalue;
	char * value2 = strdup(value);
	char * tmp = strtok(value2,",");
	nelements = 1;
	intvalue = strtoul(tmp, NULL, 0);
	switch(size) {
	    case 8:
	        data8[0] = intvalue; data = (const unsigned char *) data8; break;
	    case 16:    
	        data16[0] = intvalue; data = (const unsigned char *) data16; break;
	    case 32:    
	        data32[0] = intvalue; data = (const unsigned char *) data32; break;
	}
	tmp = strtok(NULL,",");
	while(tmp != NULL){
	    intvalue = strtoul(tmp, NULL,0);
	    switch(size) {
		case 8:
	    	    data8[nelements] = intvalue; break;
		case 16:    
	    	    data16[nelements] = intvalue; break;
		case 32:    
	    	    data32[nelements] = intvalue; break;
	    }
	    nelements++;
	    if(nelements == MAXELEMENTS){
		fprintf(stderr, "Maximum number of elements reached (%d). List truncated.\n",MAXELEMENTS);
		break;
	    }
	    tmp = strtok(NULL,",");
	}
	
	type = XA_CARDINAL;
	free(value2);
	break;
      }
      case 'i': {
	static unsigned char data8[MAXELEMENTS];
	static unsigned short data16[MAXELEMENTS];
	static unsigned long data32[MAXELEMENTS];
	unsigned long intvalue;
	char * value2 = strdup(value);
	char * tmp = strtok(value2,",");
	nelements = 1;
	intvalue = strtoul(tmp, NULL, 0);
	switch(size) {
	    case 8:
	        data8[0] = intvalue; data = (const unsigned char *) data8; break;
	    case 16:    
	        data16[0] = intvalue; data = (const unsigned char *) data16; break;
	    case 32:    
	        data32[0] = intvalue; data = (const unsigned char *) data32; break;
	}
	tmp = strtok(NULL,",");
	while(tmp != NULL){
	    intvalue = strtoul(tmp, NULL,0);
	    switch(size) {
		case 8:
	    	    data8[nelements] = intvalue; break;
		case 16:    
	    	    data16[nelements] = intvalue; break;
		case 32:    
	    	    data32[nelements] = intvalue; break;
	    }
	    nelements++;
	    if(nelements == MAXELEMENTS){
		fprintf(stderr, "Maximum number of elements reached (%d). List truncated.\n",MAXELEMENTS);
		break;
	    }
	    tmp = strtok(NULL,",");
	}
	
	type = XA_INTEGER;
	free(value2);
	break;
      }
      case 'b': {
	unsigned long boolvalue;
	static unsigned char data8;
	static unsigned short data16;
	static unsigned long data32;
	if (!strcmp(value, "True"))
	    boolvalue = 1;
	else if (!strcmp(value, "False"))
	    boolvalue = 0;
	else {
	    fprintf(stderr, "cannot convert %s argument to Bool\n", propname);
	    return;
	}
	type = XA_INTEGER;
	switch (size) {
	  case 8:
	    data8 = boolvalue; data = (const unsigned char *) &data8; break;
	  case 16:
	    data16 = boolvalue; data = (const unsigned char *) &data16; break;
	  case 32: default:
	    data32 = boolvalue; data = (const unsigned char *) &data32; break;
	}
	nelements = 1;
	break;
      }
      case 'a': {
	static Atom avalue;
	avalue = Parse_Atom(value, False);
	type = XA_ATOM;
	data = (const unsigned char *) &avalue;
	nelements = 1;
	break;
      }
      case 'm':
	/* NYI */
      default:
	Fatal_Error("bad format character: %c", format_char);
    }

    XChangeProperty(display, target_win, atom, type, size, PropModeReplace,
		    data, nelements);
}

/*
 * 
 * Routines for parsing command line:
 *
 */

static void
print_help (void)
{
    static const char *help_message =
"where options include:\n"
"    -help                          print out a summary of command line options\n"
"    -grammar                       print out full grammar for command line\n"
"    -display host:dpy              the X server to contact\n"
"    -id id                         resource id of window to examine\n"
"    -name name                     name of window to examine\n"
"    -font name                     name of font to examine\n"
"    -remove propname               remove a property\n"
"    -set propname value            set a property to a given value\n"
"    -root                          examine the root window\n"
"    -len n                         display at most n bytes of any property\n"
"    -notype                        do not display the type field\n"
"    -fs filename                   where to look for formats for properties\n"
"    -frame                         don't ignore window manager frames\n"
"    -f propname format [dformat]   formats to use for property of given name\n"
"    -spy                           examine window properties forever\n"
"    -version                       print program version\n";


    fflush (stdout);

    fprintf (stderr,
	     "usage:  %s [-options ...] [[format [dformat]] atom] ...\n\n", 
	     program_name);
    fprintf (stderr, "%s\n", help_message);
}

static inline void _X_NORETURN _X_COLD
help (void)
{
	print_help();
	exit(0);
}

void _X_NORETURN _X_COLD
usage (const char *errmsg)
{
    if (errmsg != NULL)
	fprintf (stderr, "%s: %s\n\n", program_name, errmsg);

    print_help();
    exit (1);
}

static void _X_NORETURN _X_COLD
grammar (void)
{
    printf ("Grammar for xprop:\n\n");
    printf("\t%s [<disp>] [<select option>] <option>* <mapping>* <spec>*",
	   program_name);
    printf("\n\n\tdisp ::= -display host:dpy\
\n\tselect option ::= -root | -id <id> | -font <font> | -name <name>\
\n\toption ::= -len <n> | -notype | -spy | {-formats|-fs} <format file>\
\n\tmapping ::= {-f|-format} <atom> <format> [<dformat>]\
\n\t            | -remove <propname>\
\n\t            | -set <propname> <value>\
\n\tspec ::= [<format> [<dformat>]] <atom>\
\n\tformat ::= {0|8|16|32}{a|b|c|i|m|s|t|x}*\
\n\tdformat ::= <unit><unit>*             (can't start with a letter or '-')\
\n\tunit ::= ?<exp>(<unit>*) | $<n> | <display char>\
\n\texp ::= <term> | <term>=<exp> | !<exp>\
\n\tterm ::= <n> | $<n> | m<n>\
\n\tdisplay char ::= <normal char> | \\<non digit char> | \\<octal number>\
\n\tnormal char ::= <any char except a digit, $, ?, \\, or )>\
\n\n");
    exit(0);
}

static void
Parse_Format_Mapping (int *argc, char ***argv)
{
#define ARGC (*argc)
#define ARGV (*argv)
#define OPTION ARGV[0]
#define NXTOPT if (++ARGV, --ARGC==0) usage("insufficient arguments for -format")
    char *type_name, *format, *dformat;
  
    NXTOPT; type_name = OPTION;

    NXTOPT; format = OPTION;
    if (!Is_A_Format(format))
	Fatal_Error("Bad format: %s.", format);

    dformat = NULL;
    if (ARGC>1 && Is_A_DFormat(ARGV[1])) {
	ARGV++; ARGC--; dformat = OPTION;
    }

    Add_Mapping(Parse_Atom(type_name, False), format, dformat);
}

/*
 *
 * The Main Program:
 *
 */

static int spy = 0;

static int (*old_error_handler)(Display *display, XErrorEvent *ev);

static int spy_error_handler(Display *display, XErrorEvent *ev)
{
    if (ev->error_code == BadWindow || ev->error_code == BadMatch) {
	/* Window was destroyed */
	puts("");
	exit(0);
    }

    if (old_error_handler)
	return old_error_handler(display, ev);

    return 0;
}

int
main (int argc, char **argv)
{
    FILE *stream;
    char *name;
    thunk *props;
    thunk *remove_props = NULL;
    thunk *set_props = NULL;
    Bool frame_only = False;
    int n;
    char **nargv;

#ifdef TIOCGWINSZ
    struct winsize ws;
    ws.ws_col = 0;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0)
	term_width = ws.ws_col;
#endif

    INIT_NAME;

    /* Set locale for XmbTextProptertyToTextList and iswprint(). */
    setlocale(LC_CTYPE, "");

    /* Handle display name, opening the display */
    Setup_Display_And_Screen(&argc, argv);

    /* Handle selecting the window to display properties for */
    target_win = Select_Window_Args(&argc, argv);

    /* Set up default atom to format, dformat mapping */
    XpropMode = XpropWindowProperties;
    for (n = argc, nargv = argv; n; nargv++, n--)
	if (! strcmp(nargv[0], "-font")) {
	    XpropMode = XpropFontProperties;
	    break;
	}
    Setup_Mapping();
    if ((name = getenv("XPROPFORMATS"))) {
	if (!(stream = fopen(name, "r")))
	    Fatal_Error("unable to open file %s for reading.", name);
	Read_Mappings(stream);
	fclose(stream);
    }

    /* Handle '-' options to setup xprop, select window to work on */
    while (argv++, --argc>0 && **argv == '-') {
	if (!strcmp(argv[0], "-"))
	    continue;
	if (!strcmp(argv[0], "-help")) {
	    help ();
	    /* NOTREACHED */
	}
	if (!strcmp(argv[0], "-grammar")) {
	    grammar ();
	    /* NOTREACHED */
	}
	if (!strcmp(argv[0], "-notype")) {
	    notype = 1;
	    continue;
	}
	if (!strcmp(argv[0], "-spy")) {
	    spy = 1;
	    continue;
	}
	if (!strcmp(argv[0], "-len")) {
	    if (++argv, --argc == 0) usage("-len requires an argument");
	    max_len = atoi(argv[0]);
	    continue;
	}
	if (!strcmp(argv[0], "-formats") || !strcmp(argv[0], "-fs")) {
	    if (++argv, --argc == 0) usage("-fs requires an argument");
	    if (!(stream = fopen(argv[0], "r")))
		Fatal_Error("unable to open file %s for reading.", argv[0]);
	    Read_Mappings(stream);
	    fclose(stream);
	    continue;
	}
	if (!strcmp(argv[0], "-font")) {
	    if (++argv, --argc == 0) usage("-font requires an argument");
	    font = Open_Font(argv[0]);
	    target_win = (Window) -1;
	    continue;
	}
	if (!strcmp(argv[0], "-remove")) {
	    thunk t = {0};
	    if (++argv, --argc == 0) usage("-remove requires an argument");
	    t.propname = argv[0];
	    if (remove_props == NULL) remove_props = Create_Thunk_List();
	    remove_props = Add_Thunk(remove_props, t);
	    continue;
	}
	if (!strcmp(argv[0], "-set")) {
	    thunk t = {0};
	    if (argc < 3) usage("insufficient arguments for -set");
	    t.propname = argv[1];
	    t.extra_value = argv[2];
	    argv += 3; argc -= 3;
	    if (set_props == NULL) set_props = Create_Thunk_List();
	    set_props = Add_Thunk(set_props, t);
	    continue;
	}
	if (!strcmp(argv[0], "-frame")) {
	    frame_only = True;
	    continue;
	}
	if (!strcmp(argv[0], "-f") || !strcmp(argv[0], "-format")) {
	    Parse_Format_Mapping(&argc, &argv);
	    continue;
	}
	if (!strcmp(argv[0], "-version")) {
	    puts(PACKAGE_STRING);
	    exit(0);
	}
	fprintf (stderr, "%s: unrecognized argument %s\n\n",
		 program_name, argv[0]);
	usage(NULL);
    }

    if ((remove_props != NULL || set_props != NULL) && argc > 0) {
	fprintf (stderr, "%s: unrecognized argument %s\n\n",
		 program_name, argv[0]);
	usage(NULL);
    }

    if (target_win == None)
	target_win = Select_Window(dpy, !frame_only);

    if (remove_props != NULL) {
	int count;

	if (target_win == (Window) -1)
	    Fatal_Error("-remove works only on windows, not fonts");

	count = remove_props->thunk_count;
	for (; count > 0; remove_props++, count--)
	    Remove_Property (dpy, target_win, remove_props->propname);
    }

    if (set_props != NULL) {
	int count;

	if (target_win == (Window) -1)
	    Fatal_Error("-set works only on windows, not fonts");

	count = set_props->thunk_count;
	for (; count > 0; set_props++, count--)
	    Set_Property (dpy, target_win, set_props->propname,
			  set_props->extra_value);
    }

    if (remove_props != NULL || set_props != NULL) {
	XCloseDisplay (dpy);
	exit (0);
    }

    props = Handle_Prop_Requests(argc, argv);

    if (spy && target_win != (Window) -1) {
	XEvent event;
	const char *format, *dformat;
	
	XSelectInput(dpy, target_win, PropertyChangeMask | StructureNotifyMask);
 	old_error_handler = XSetErrorHandler(spy_error_handler);
	for (;;) {
	    fflush(stdout);
	    XNextEvent(dpy, &event);
 	    if (event.type == DestroyNotify)
 		break;
 	    if (event.type != PropertyNotify)
 		continue;
	    format = dformat = NULL;
	    if (props) {
		int i;
		for (i = 0; i < props->thunk_count; i++)
		    if (props[i].value == (long) event.xproperty.atom)
			break;
		if (i >= props->thunk_count)
		    continue;
		format = props[i].format;
		dformat = props[i].dformat;
	    }
	    Show_Prop(format, dformat, Format_Atom(event.xproperty.atom));
	}
    }
    exit (0);
}
