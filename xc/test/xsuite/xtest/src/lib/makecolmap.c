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
 * $XConsortium: makecolmap.c,v 1.5 94/04/17 21:00:52 rws Exp $
 */
#include	"stdlib.h"
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Make a colourmap for the screen associated with the Default
 * root window.
 */

Colormap
makecolmap(disp, visual, alloc)
Display *disp;
Visual *visual;
int alloc;
{
	Colormap colormap;

	colormap = XCreateColormap(disp, DRW(disp), visual, alloc);
	regid(disp, (union regtypes *) &colormap, REG_COLORMAP);
	return(colormap);
}


/*
 * return floor of log to base two.
 * if x is zero returns -1 ( lg(x) != -1 for any integer x)
 */
int lg(x)
	unsigned long x;
{
	int count = -1;

	for(; x != 0; x>>=1, count++);
	return count;
}

/*
 * return a sting representing the argument value as a binary number.
 */
char *bitstr(val, spare)
	unsigned long val;
	char spare[];
{
	int i;

	for(i=0; i< sizeof(unsigned long)<<3; i++,val <<= 1)
		if(val & ((unsigned long)1<<((sizeof(unsigned long)<<3) -1)))
			spare[i] = '1';
		else
			spare[i] = '0';
	spare[i] = (char)0;
	return(spare);
}
