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
 * $XConsortium: verimage.c,v 1.17 94/04/17 21:01:08 rws Exp $
 */

#include	"stdio.h"
#include	"string.h"

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"

#define	BUF_LEN	512

extern	int 	tet_thistest;
extern	struct	tet_testlist tet_testlist[];


int 	Errnum;	/* Number of error record */

Status
verifyimage(disp, d, ap)
Display		*disp;
Drawable	d;
struct	area	*ap;
{
static FILE	*fp;
long	initfpos;
XImage	*imp;
register int 	x, y;
register unsigned long	ipix;
unsigned long	pix;
unsigned long 	count;
int 	good, bad;
int 	ic;
unsigned int width, height;
long depth;
long	imdepth;
char	buf[BUF_LEN];
char	name[128];
static	int lasttest;
static	int lastvinf;
extern	int CurVinf;

	if(!config.save_server_image && config.debug_no_pixcheck) {
		trace("pixcheck code subverted");
		return(True);
	}

	if (ap == NULL) {
		getsize(disp, d, &width, &height);
		x = 0;
		y = 0;
	} else {
		x = ap->x;
		y = ap->y;
		width = ap->width;
		height = ap->height;
	}

	depth = getdepth(disp, d);
	imp = XGetImage(disp, d, x, y, width, height, AllPlanes, ZPixmap);
	if (imp == 0) {
		delete("get image failed");
		return(False);
	}

	ic = tet_testlist[tet_thistest-1].icref;
	(void) sprintf(name, "a%d.dat", ic);

#ifdef GENERATE_PIXMAPS
	if (tet_thistest != lasttest || CurVinf == lastvinf) {
		dumpimage(imp, name, ap);
		XDestroyImage(imp);
		lasttest = tet_thistest;
		lastvinf = CurVinf;
		report("Created reference image file");
		return(True);
	}
#endif

	if (tet_thistest != lasttest || CurVinf != lastvinf) {
		if (fp)
			fclose(fp);
		fp = fopen(name, "r");
		lasttest = tet_thistest;
		lastvinf = CurVinf;
	}

	/*
	 * If option to dump out server generated versions of files, do this
	 * here.
	 */
	if (config.save_server_image) {
		(void) sprintf(name, "a%d.sav", ic);
		dumpimage(imp, name, ap);
		trace("Created server image file %s", name);
	}

	if(config.debug_no_pixcheck) {
		XDestroyImage(imp);
		trace("pixcheck code subverted");
		return(True);
	}

	if (fp == NULL) {
		XDestroyImage(imp);
		delete("Could not open pixel validation data file %s", name);
		return(True);	/* We don't want to generate a FAIL */
	}

	initfpos = ftell(fp);

	do {
	    if (fgets(buf, BUF_LEN, fp) == NULL)
		goto badformat;
	} while (buf[0] == '!');
	if (sscanf(buf, "%d %d %d", &width, &height, &imdepth) < 3) {
badformat:
	    delete("Bad format pixel validation data file %s", name);
	    XDestroyImage(imp);
	    return(False);
	}

	/*
	 * Choose the smaller of the depths in the image and in the drawable.
	 */
	if (imdepth < depth)
		depth = imdepth;

	if (width != imp->width) {
		delete("width mismatch");
		XDestroyImage(imp);
		return(False);
	}
	if (height != imp->height) {
		delete("height mismatch");
		XDestroyImage(imp);
		return(False);
	}

	count = 0;
	good  = 0;
	bad   = 0;

	x = y = 0;

	while (fgets(buf, BUF_LEN, fp) != NULL) {
		if (strchr(buf, ',') != NULL) {
			if (sscanf(buf, "%x,%x", &count, &pix) < 2)
			    goto badformat;
		} else {
			count = 1;
			if (sscanf(buf, "%x", &pix) < 1)
			    goto badformat;
		}
		pix &= (1<<depth)-1;

		for (; count; count--) {
			ipix = XGetPixel(imp, x, y);
			ipix &= (1<<depth)-1;
			if (pix == ipix) {
				good++;
			} else {
				bad++;
			}
			if (++x >= width) {
				x = 0;
				y++;
			}
			if (y >= height)
				goto ok;
		}
	}

	fclose(fp);

ok:

	if (bad) {
	/*
	 * Make this separate routine XXX
	 */
	char	buf[BUF_LEN];
	char	errfile[64];
	long	newpos;
	int 	n;
	FILE	*errfp;

		report("A total of %d out of %d pixels were bad", bad, good+bad);
		(void) sprintf(errfile, "Err%04d.err", Errnum);
		(void) unlink(errfile);
		dumpimage(imp, errfile, ap);

		newpos = ftell(fp);
		errfp = fopen(errfile, "a");
		if (errfp == NULL) {
			report("Could not open pixel error file %s", errfile);
		} else {
			fseek(fp, initfpos, 0);
			for (n = newpos-initfpos; n > 0; ) {
				fread(buf, 1, (n>BUF_LEN)? BUF_LEN: n, fp);
				fwrite(buf, 1, (n>BUF_LEN)? BUF_LEN: n, errfp);
				n -= BUF_LEN;
			}
			report("Pixel check failed. See file %s for results", errfile);
			Errnum++;
			fclose(errfp);
		}
	}
	if (good + bad < width*height) {
		delete("Early end of file in pixmap checking");
		/*
		 * Return is true so that the test does not give a failure;
		 * it is the data file that needs attention.
		 */
		XDestroyImage(imp);
		return(True);
	}

	XDestroyImage(imp);
	if (good == width*height && bad == 0)
		return(True);
	else
		return(False);
}
