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
 * $XConsortium: stackorder.c,v 1.9 94/04/17 21:01:05 rws Exp $
 */

#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "pixval.h"

/*
 * stackorder() takes as input a window and returns its
 * position in the stacking order, with 0 being the lowest.
 */
int
stackorder(disp, win)
Display	*disp;
Window	win;
{
Window	*children;
Window	parent, root;
Window	dummy;
unsigned int 	nchild;
int 	result;
int 	i;

	/* Get the parent of the input window */
	if(!XQueryTree(disp, win, &root, &parent, &children, &nchild)) {
		debug(2, "stackorder: 1st XQueryTree returns 0");
		return(-1);
	}
	if (nchild != 0 && children)
		XFree((char*)children);	/* Not needed here */
	
	result = -1;

	/*
	 * Now get all the siblings of the input window and
	 * search for the input window among them; return index
	 * if found.
	 */
	debug(2, "win=%d", win);
	if(!XQueryTree(disp, parent, &root, &dummy, &children, &nchild)) {
		debug(2, "stackorder: 2nd XQueryTree returns 0");
		return(-1);
	}
	for (i = 0; i < nchild; i++) {
		debug(2, "child=%d", children[i]);
		if (children[i] == win) {
			result = i;
			break;
		}
	}

	if (children)
		XFree((char*)children);

	return(result);
}
