/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: config.c,v 1.26 94/04/17 21:00:40 rws Exp $
 */

#include	"stdlib.h"
#include	"string.h"
#include	<ctype.h>
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	"ximtest.h"


/*
 * Calculate a value from a string, allowing hex or octal
 * in a similar, but almost completely different, way to strtol.
 */
int
atov( str )
char *str;
{
	int base = 10;
	int value= 0;
	char eval[2];
	int sign=1;

	if (str == NULL)
		return(0); 

	eval[1]='\0';

/* Strip leading whitespace */
	while(isspace(*str))
		str++;

	while(*str) {
		int a;

		eval[0] = *str;

		a = strcspn("00112233445566778899aAbBcCdDeEfFxX--", eval)/2;

		if (a==17 && value==0 && base==10) {		/* minus sign */
			sign = -1;
		} else if (a==0  && value==0 && base==10) { 	/* octal */
			base = 8;
		} else if (a==16 && value==0 && base==8 ) { 	/* hex */
			base = 16;
		} else if (a<base) {				/* number */
			value *= base;
			value += a*sign;
		} else {		/* Out of range, or not a number */
			return(value);
		}
		str++;
	}
	return(value);
}

struct	config	config;
struct	ximconfig	ximconfig;

#define	T_INT		1
#define	T_STRING	2
#define	T_YESNO		3
/*
 * Flags to control what action to take about missing or blank parameters.
 * Parameters marked DEBUG are also implicitly OPTIONAL.
 */
#define	FL_DEBUG	0x1	/* Not to be set for verification runs */
#define	FL_OPTIONAL	0x2	/* Optional parameter, absence not fatal */
#define	FL_EMPTYOK	0x4	/* Empty value is ok */

struct	getparam {
	char	*name;
	int 	type;
	char	*addr;
	int 	flags;
};

struct	getparam parm[] = {

	/* General configuration parameters */

	{"XT_DISPLAY", T_STRING, (char*)&config.display, 0},
	{"XT_ALT_SCREEN", T_INT, (char*)&config.alt_screen, 0},
	{"XT_FONTPATH", T_STRING, (char*)&config.fontpath, 0},
	{"XT_SPEEDFACTOR", T_INT, (char*)&config.speedfactor, 0},
	{"XT_RESET_DELAY", T_INT, (char*)&config.reset_delay, 0},
	{"XT_EXTENSIONS", T_YESNO, (char*)&config.extensions, 0},

	/* Configuration parameters for specific tests */

	{"XT_VISUAL_CLASSES", T_STRING, (char*)&config.visual_classes, 0},
	{"XT_FONTCURSOR_GOOD", T_INT, (char*)&config.fontcursor_good, 0},
	{"XT_FONTCURSOR_BAD", T_INT, (char*)&config.fontcursor_bad, 0},
	{"XT_FONTPATH_GOOD", T_STRING, (char*)&config.fontpath_good, 0},
	{"XT_FONTPATH_BAD", T_STRING, (char*)&config.fontpath_bad, 0},
	{"XT_BAD_FONT_NAME", T_STRING, (char*)&config.bad_font_name, 0},
	{"XT_GOOD_COLORNAME", T_STRING, (char*)&config.good_colorname, 0},
	{"XT_BAD_COLORNAME", T_STRING, (char*)&config.bad_colorname, 0},
	{"XT_DISPLAYMOTIONBUFFERSIZE", T_INT, (char*)&config.displaymotionbuffersize, 0},

	/* Configuration parameters for Display functions */

	{"XT_SCREEN_COUNT", T_INT, (char*)&config.screen_count, 0},
	{"XT_PIXMAP_DEPTHS", T_STRING, (char*)&config.pixmap_depths, 0},
	{"XT_BLACK_PIXEL", T_INT, (char*)&config.black_pixel, 0},
	{"XT_WHITE_PIXEL", T_INT, (char*)&config.white_pixel, 0},
	{"XT_HEIGHT_MM", T_INT, (char*)&config.height_mm, 0},
	{"XT_WIDTH_MM", T_INT, (char*)&config.width_mm, 0},
	{"XT_SERVER_VENDOR", T_STRING, (char*)&config.server_vendor, 0},
	{"XT_PROTOCOL_VERSION", T_INT, (char *)&config.protocol_version, 0},
	{"XT_PROTOCOL_REVISION", T_INT, (char *)&config.protocol_revision, 0},
	{"XT_VENDOR_RELEASE", T_INT, (char *)&config.vendor_release, 0},
	{"XT_DOES_SAVE_UNDERS", T_YESNO, (char *)&config.does_save_unders, 0},
	{"XT_DOES_BACKING_STORE", T_INT, (char *)&config.does_backing_store, 0},

	/* Configuration parameters for connection tests */

	{"XT_POSIX_SYSTEM", T_YESNO, (char*)&config.posix_system, 0},
	{"XT_DECNET", T_YESNO, (char *)&config.decnet, 0},
	{"XT_TCP", T_YESNO, (char *)&config.tcp, 0},
	{"XT_DISPLAYHOST", T_STRING, (char *)&config.displayhost, 0},
	{"XT_LOCAL", T_YESNO, (char *)&config.local, 0},

	/* Parameters which do not affect test results */

	{"XT_OPTION_NO_CHECK", T_YESNO, (char*)&config.option_no_check,
		FL_OPTIONAL},
	{"XT_OPTION_NO_TRACE", T_YESNO, (char*)&config.option_no_trace,
		FL_OPTIONAL},
	{"XT_SAVE_SERVER_IMAGE", T_YESNO, (char*)&config.save_server_image,
		FL_OPTIONAL},

	/* Parameters which should not be set on verification test runs */

	{"XT_DEBUG", T_INT, (char*)&config.debug, FL_DEBUG},
	{"XT_DEBUG_OVERRIDE_REDIRECT", T_YESNO, (char*)&config.debug_override_redirect,
		FL_DEBUG},
	{"XT_DEBUG_PAUSE_AFTER", T_YESNO, (char*)&config.debug_pause_after,
		FL_DEBUG},
	{"XT_DEBUG_PIXMAP_ONLY", T_YESNO, (char*)&config.debug_pixmap_only,
		FL_DEBUG},
	{"XT_DEBUG_WINDOW_ONLY", T_YESNO, (char*)&config.debug_window_only,
		FL_DEBUG},
	{"XT_DEBUG_DEFAULT_DEPTHS", T_YESNO, (char*)&config.debug_default_depths,
		FL_DEBUG},
	{"XT_DEBUG_BYTE_SEX", T_STRING, (char*)&config.debug_byte_sex,
		FL_DEBUG},
	{"XT_DEBUG_VISUAL_CHECK", T_INT, (char*)&config.debug_visual_check,
		FL_DEBUG},
	{"XT_DEBUG_NO_PIXCHECK", T_YESNO, (char*)&config.debug_no_pixcheck,
		FL_DEBUG},
	{"XT_DEBUG_VISUAL_IDS", T_STRING, (char*)&config.debug_visual_ids,
		FL_DEBUG},

	/* Parameters only used during test development */
	{"XT_FONTDIR", T_STRING, (char*)&config.fontdir, FL_OPTIONAL},

	{"XT_LOCALE", T_STRING, (char*)&ximconfig.locale, FL_OPTIONAL},
	{"XT_LOCALE_MODIFIERS", T_STRING,
		(char*)&ximconfig.locale_modifiers, FL_OPTIONAL},
	{"XT_FONTSET", T_STRING, (char*)&ximconfig.fontsets, FL_OPTIONAL},
	{"XT_SAVE_IM", T_YESNO, (char*)&ximconfig.save_im, FL_OPTIONAL},

};

/*
 * Initialise the config structure by getting all the execution
 * parameters.
 */
void
initconfig()
{
char	*var;
struct	getparam	*gp;

	for (gp = parm; gp < parm+NELEM(parm); gp++) {
		var = tet_getvar(gp->name);
		if (var == NULL) {
			if (!(gp->flags&(FL_OPTIONAL|FL_DEBUG)))
				report("Required parameter %s was not set", gp->name);
			continue;
		}

		if (var && *var == '\0') {
			if (!(gp->flags&(FL_EMPTYOK|FL_DEBUG)))
				report("Parameter %s had an empty value", gp->name);
			continue;
		}

		debug(2, "Variable %s=%s", gp->name, var);
		switch (gp->type) {
		case T_STRING:
			*(char**)gp->addr = var;
			break;
		case T_INT:
			if (strcmp(var, "UNSUPPORTED") == 0) {
				*(int*)gp->addr = -1;	/* XXX */
			} else {
				*(int*)gp->addr = atov(var);
			}
			debug(3, "  int val=%d", *(int*)gp->addr);
			break;
		case T_YESNO:
			if (*var == 'Y' || *var == 'y')
				*(int*)gp->addr = 1;
			else if (*var == 'N' || *var == 'n')
				*(int*)gp->addr = 0;
			else {
				report("Parameter %s was not set to 'Y' or 'N'", gp->name);
				report("  was %s", var);
			}
			debug(3, "  yesno val=%d", *(int*)gp->addr);
			break;
		default:
			report("Unrecognised type in initconfig");
			break;
		}
	}
}
