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
 * $XConsortium: checkfont.c,v 1.9 94/04/17 21:00:37 rws Exp $
 */

#include	"stdlib.h"
#include	"stdio.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"Xatom.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"

extern	Display *Dsp;

extern	XFontStruct	xtfont0, xtfont1, xtfont2, xtfont3, xtfont4;
extern	XFontStruct xtfont5, xtfont6;
extern	char	*xtfont0cpright;
extern	char	*xtfont1cpright;
extern	char	*xtfont2cpright;
extern	char	*xtfont3cpright;
extern	char	*xtfont4cpright;
extern	char	*xtfont5cpright;
extern	char	*xtfont6cpright;

struct	fontinfo fontinfo[] = {
	{"xtfont0", &xtfont0, &xtfont0cpright},
	{"xtfont1", &xtfont1, &xtfont1cpright},
	{"xtfont2", &xtfont2, &xtfont2cpright},
	{"xtfont3", &xtfont3, &xtfont3cpright},
	{"xtfont4", &xtfont4, &xtfont4cpright},
	{"xtfont5", &xtfont5, &xtfont5cpright},
	{"xtfont6", &xtfont6, &xtfont6cpright},
};
int 	nfontinfo = NELEM(fontinfo);

#if TEST_ANSI
static int checkprops(XFontStruct *fsp, XFontStruct *good, char *str);
static XCharStruct *getmetric(XFontStruct *fsp, unsigned int c);
static check1prop(XFontStruct *fsp, XFontProp *fp, char *str);
#else
static int checkprops();
static XCharStruct *getmetric();
static check1prop();
#endif

/*
 * Check a XCharStruct item.  FAIL is issued on error, PASS is not.
 * Used by checkfsp().
 * Returns True if checking is OK.
 */
static int
checkcharstruct(csp, good, name)
XCharStruct	*csp;
XCharStruct	*good;
char	*name;
{
int 	pass = 0, fail = 0;

	if (csp->lbearing != good->lbearing) {
		report("%s, lbearing was %d, expecting %d",
			name, csp->lbearing, good->lbearing);
		FAIL;
	} else
		pass++;

	if (csp->rbearing != good->rbearing) {
		report("%s, rbearing was %d, expecting %d",
			name, csp->rbearing, good->rbearing);
		FAIL;
	} else
		pass++;

	if (csp->width != good->width) {
		report("%s, width was %d, expecting %d",
			name, csp->width, good->width);
		FAIL;
	} else
		pass++;

	if (csp->ascent != good->ascent) {
		report("%s, ascent was %d, expecting %d",
			name, csp->ascent, good->ascent);
		FAIL;
	} else
		pass++;

	if (csp->descent != good->descent) {
		report("%s, descent was %d, expecting %d",
			name, csp->descent, good->descent);
		FAIL;
	} else
		pass++;

	if (fail == 0 && pass == 5)
		return(True);
	else {
		if (fail == 0)
			report("Path check error in checkcharstruct");
		return(False);
	}
}


/*
 * Check a XFontStruct against a known good one.  The font property
 * checking is based on a knowledge of the properties defined in the xtest
 * fonts.
 */
int
checkfsp(fsp, good, str)
XFontStruct	*fsp;
XFontStruct	*good; /* Known good XFontStruct */
char	*str;	/* Known good copyright string */
{
int 	i;
int 	nchars;
int 	metrics_correct;
int 	pass = 0, fail = 0;
	
	if (fsp == NULL) {
		report("returned XFontStruct pointer was NULL");
		FAIL;
		return(False);
	}

	/*
	 * No clear definition of what this hint is
	 */
	if (fsp->direction != FontLeftToRight && fsp->direction != FontRightToLeft){
		report("direction was %d, expecting %d", fsp->direction, good->direction);
		FAIL;
	} else
		pass++;

	if (fsp->min_char_or_byte2 != good->min_char_or_byte2) {
		report("min_char_or_byte2 was %d, expecting %d", fsp->min_char_or_byte2, good->min_char_or_byte2);
		FAIL;
	} else
		pass++;

	if (fsp->max_char_or_byte2 != good->max_char_or_byte2) {
		report("max_char_or_byte2 was %d, expecting %d", fsp->max_char_or_byte2,
			good->max_char_or_byte2);
		FAIL;
	} else
		pass++;

	if (fsp->min_byte1 != good->min_byte1) {
		report("min_byte1 was %d, expecting %d", fsp->min_byte1,
			good->min_byte1);
		FAIL;
	} else
		pass++;

	if (fsp->max_byte1 != good->max_byte1) {
		report("max_byte1 was %d, expecting %d", fsp->max_byte1,
			good->max_byte1);
		FAIL;
	} else
		pass++;

	if (fsp->all_chars_exist != good->all_chars_exist) {
		report("all_chars_exist was %d, expecting %d", fsp->all_chars_exist,
			good->all_chars_exist);
		FAIL;
	} else
		pass++;

	if (fsp->default_char != good->default_char) {
		report("default_char was %d, expecting %d", fsp->default_char,
			good->default_char);
		FAIL;
	} else
		pass++;

	/*
	 * There may be extra properties added by the font compiler, so
	 * we can only check that there are at least enough properties.
	 */
	if (fsp->n_properties < good->n_properties) {
		report("n_properties was %d, expecting a number greater than %d",
			fsp->n_properties, good->n_properties);
		FAIL;
	} else
		pass++;

	if (fsp->ascent != good->ascent) {
		report("ascent was %d, expecting %d", fsp->ascent, good->ascent);
		FAIL;
	} else
		pass++;

	if (fsp->descent != good->descent) {
		report("descent was %d, expecting %d", fsp->descent, good->descent);
		FAIL;
	} else
		pass++;

	/*
	 * Check that the per_char pointer is NULL or not as appropriate.
	 */
	if (good->per_char == NULL) {
		if (fsp->per_char == NULL)
			pass++;
		else {
			report("The per_char member of XFontStruct was not NULL");
			FAIL;
		}
	} else {
		if (fsp->per_char == NULL) {
			report("The per_char member of XFontStruct was NULL");
			FAIL;
		} else {
			pass++;
		}
	}

	if (checkcharstruct(&fsp->min_bounds, &good->min_bounds, "min_bounds"))
		pass++;
	else {
		report("min_bound check failed");
		FAIL;
	}
	if (checkcharstruct(&fsp->max_bounds, &good->max_bounds, "max_bounds"))
		pass++;
	else {
		report("max_bounds check failed");
		FAIL;
	}


	if (fsp->per_char != NULL && good->per_char != NULL) {
	int 	giveup;

#define percharsize(FSP) ( (FSP->max_char_or_byte2 - FSP->min_char_or_byte2 + 1) * (FSP->max_byte1 - FSP->min_byte1 + 1))

		nchars = percharsize(good);
		/*
		 * Make sure that we don't run off the end of the array, if
		 * the min and max chars are wrong then will probably fail,
		 * but try anyway.
		 */
		if (nchars > percharsize(fsp))
			nchars = percharsize(fsp);

		/*
		 * If the max and min bytes are wrong then there is not much
		 * chance that the per_char metrics will tally, so if there
		 * have been previous errors then abandon checking per_char
		 * after a few errors.
		 */
		giveup = 9999;
		if (fail)
			giveup = 3;

		metrics_correct = 0;

		for (i = 0; i < nchars; i++) {
		char	mess[32];

			sprintf(mess, "char %d", i);

			if (checkcharstruct(&fsp->per_char[i], &good->per_char[i], mess))
				metrics_correct++;
			else if (--giveup == 0) {
				report("Abandoning checks to other characters");
				break;
			}
		}

		if (metrics_correct == nchars)
			pass++;
		else {
			if (giveup == 0) {
				report("Metrics bad, %d correct seen before abandoning checks",
					metrics_correct);
			} else {
				report("Metrics for %d out of %d chars incorrect",
					nchars-metrics_correct, nchars);
			}
			FAIL;
		}
	} else {
		/* Nothing to check */
		pass++;
	}

	if (checkprops(fsp, good, str))
		pass++;
	else {
		report("Property checking failed");
		FAIL;
	}

	if (fail == 0 && pass == 15) {
		return(True);
	} else {
		if (fail == 0)
			report("Path check error in checkfsp");
		return(False);
	}
}

/*
 * Check each of the properties that we have chosen to test.  There may be
 * other properties defined in the returned XFontStruct which are ignored.
 */
static int
checkprops(fsp, good, str)
XFontStruct	*fsp;
XFontStruct	*good;
char	*str;
{
XFontProp	*fp;
int 	pass = 0, fail = 0;

	for (fp = good->properties;
		  fp < &good->properties[good->n_properties];
		  fp++) {
		if (check1prop(fsp, fp, str))
			pass++;
		else
			fail++;
	}
	if (fail == 0 && pass == good->n_properties)
		return(True);
	else
		return(False);
}

/*
 * Check a single property
 */
static
check1prop(fsp, fp, str)
XFontStruct	*fsp;
XFontProp	*fp;
char	*str;
{
XFontProp	*testp;
char	*teststr;
int 	found;
int 	pass = 0, fail = 0;

	found = 0;
	for (testp = fsp->properties; testp < &fsp->properties[fsp->n_properties];
			testp++){
		
		if (testp->name == fp->name) {
			found = 1;

			if (fp->name == XA_COPYRIGHT) {

				/*
				 * This is a string value, so have to get the atom value.
				 * Have to disable default error processing because the
				 * atom may not exist.
				 */
				XSetErrorHandler(error_status);
				reseterr();
				teststr = XGetAtomName(Dsp, testp->card32);
				XSetErrorHandler(unexp_err);

				switch (geterr()) {
				case Success:
					break;
				case BadAtom:
					report("XA_COPYRIGHT atom value was not a valid atom");
					FAIL;
					break;
				default:
					delete("Unexpected error on XGetAtomName");
					return(False);
				}

				if (str && strcmp(teststr, str) != 0) {
					report("XA_COPYRIGHT string..");
					report(" was '%s'", teststr);
					report(" expecting '%s'", str);
					FAIL;
				}
				else
					pass++;
			} else {
				if (testp->card32 != fp->card32) {
					report("Value of %s was %d, expecting %d",
						atomname(fp->name), testp->card32, fp->card32);
					FAIL;
				}
				else
					pass++;
			}
		}
	}
	if (!found) {
		report("Font property %s not found", atomname(fp->name));
		FAIL;
	}
	else
		pass++;
	if (fail == 0 && pass == 2)
		return(True);
	else
		return(False);
}


#define	min(a, b) ((a)<(b)? (a): (b))
#define	max(a, b) ((a)>(b)? (a): (b))

/*
 * Direct calculation of extents.
 */
void
txtextents(fsp, str, n, dir, ascent, descent, overall)
XFontStruct	*fsp;
unsigned char	*str;
int 	n;
int 	*dir;	/*NOTUSED*/
int 	*ascent;
int 	*descent;
XCharStruct	*overall;
{
int 	i;
unsigned int 	c;
XCharStruct	*cm;
extern XCharStruct	*getmetric();
short 	width;
short 	rbearing = 0;
short 	lbearing = 0;
short  	oascent = 0;
short  	odescent = 0;
int 	firstchar = 1;

	*ascent = fsp->ascent;
	*descent = fsp->descent;

	/*
	 * If there are no per_char metrics then use max_bounds
	 * of the font width.
	 */
	if (fsp->per_char == NULL) {
		rbearing = max(fsp->max_bounds.rbearing, (n-1)*fsp->max_bounds.width + fsp->max_bounds.rbearing);
		lbearing = min(fsp->max_bounds.lbearing, (n-1)*fsp->max_bounds.width + fsp->max_bounds.lbearing);
		oascent = fsp->max_bounds.ascent;
		odescent = fsp->max_bounds.descent;
		width = n*fsp->max_bounds.width;
	}

	width = 0;
	for (i = 0; i < n; i++) {
		c = str[i];

		cm = getmetric(fsp, c);
		if (cm == NULL)
			cm = getmetric(fsp, fsp->default_char);

		if (cm == NULL)
			continue;

		if (firstchar) {
			firstchar = 0;
			rbearing = cm->rbearing;
			lbearing = cm->lbearing;
			oascent = cm->ascent;
			odescent = cm->descent;
			width = cm->width;
		} else {
			rbearing = max(width+cm->rbearing, rbearing);
			lbearing = min(width+cm->lbearing, lbearing);
			oascent = max(oascent, cm->ascent);
			odescent = max(odescent, cm->descent);
			width += cm->width;
		}
	}
	overall->rbearing = rbearing;
	overall->lbearing = lbearing;
	overall->width = width;
	overall->ascent = oascent;
	overall->descent = odescent;
}

/*
 * Direct calculation of extents with 16bit strings.
 */
void
txtextents16(fsp, str, n, dir, ascent, descent, overall)
XFontStruct	*fsp;
XChar2b	*str;
int 	n;
int 	*dir;
int 	*ascent;
int 	*descent;
XCharStruct	*overall;
{
int 	i;
unsigned int 	c;
XCharStruct	*cm;
extern XCharStruct	*getmetric();
short	width;
short	rbearing = 0;
short	lbearing = 0;
short	oascent = 0;
short	odescent = 0;
int 	firstchar = 1;

	*ascent = fsp->ascent;
	*descent = fsp->descent;

	/*
	 * If there are no per_char metrics then use max_bounds
	 * of the font width.
	 */
	if (fsp->per_char == NULL) {
		rbearing = max(fsp->max_bounds.rbearing, (n-1)*fsp->max_bounds.width + fsp->max_bounds.rbearing);
		lbearing = min(fsp->max_bounds.lbearing, (n-1)*fsp->max_bounds.width + fsp->max_bounds.lbearing);
		oascent = fsp->max_bounds.ascent;
		odescent = fsp->max_bounds.descent;
		width = n*fsp->max_bounds.width;
	}

	width = 0;
	for (i = 0; i < n; i++) {
		c = str[i].byte1;
		c <<= 8;
		c += str[i].byte2;

		cm = getmetric(fsp, c);
		if (cm == NULL)
			cm = getmetric(fsp, fsp->default_char);

		if (cm == NULL)
			continue;

		if (firstchar) {
			firstchar = 0;
			rbearing = cm->rbearing;
			lbearing = cm->lbearing;
			oascent = cm->ascent;
			odescent = cm->descent;
			width = cm->width;
		} else {
			rbearing = max(width+cm->rbearing, rbearing);
			lbearing = min(width+cm->lbearing, lbearing);
			oascent = max(oascent, cm->ascent);
			odescent = max(odescent, cm->descent);
			width += cm->width;
		}
	}
	overall->rbearing = rbearing;
	overall->lbearing = lbearing;
	overall->width = width;
	overall->ascent = oascent;
	overall->descent = odescent;
}

/*
 * Direct calculation of width.
 */
int
txtwidth(fsp, str, n)
XFontStruct	*fsp;
unsigned char	*str;
int 	n;
{
XCharStruct	cm;
int 	dum;

	txtextents(fsp, str, n, &dum, &dum, &dum, &cm);
	return(cm.width);
}

/*
 * Direct calculation of width for 16 bit strings.
 */
int
txtwidth16(fsp, str, n)
XFontStruct	*fsp;
XChar2b	*str;
int 	n;
{
XCharStruct	cm;
int 	dum;

	txtextents16(fsp, str, n, &dum, &dum, &dum, &cm);
	return(cm.width);
}

/*
 * Get a metric.  Note that the spec states that a character is non-existant
 * when all metrics are zero.  Xlib only checks width, lbearing and rbearing.
 * All metrics zero is used here.
 */
static XCharStruct *
getmetric(fsp, c)
XFontStruct	*fsp;
unsigned int 	c;
{
XCharStruct	*cm;
int 	byte1;
int 	byte2;

	byte1 = (c>>8)&0xff;
	byte2 = c&0xff;

	/*
	 * Out of range character.
	 */
	if (byte1 < fsp->min_byte1 || byte1 > fsp->max_byte1)
		return((XCharStruct *)0);
	if (byte2 < fsp->min_char_or_byte2 || byte2 > fsp->max_char_or_byte2)
		return((XCharStruct *)0);

	c = (byte1-fsp->min_byte1)*(fsp->max_char_or_byte2 - fsp->min_char_or_byte2+1)
		+ byte2 - fsp->min_char_or_byte2;
	cm = &fsp->per_char[c];
	/*
	 * Check for non-existant character.
	 */
	if (cm->lbearing == 0 && cm->rbearing == 0 &&
			cm->ascent == 0 && cm->descent == 0 &&
			cm->width == 0) {
		return((XCharStruct*)0);
	}
	return(cm);
}
