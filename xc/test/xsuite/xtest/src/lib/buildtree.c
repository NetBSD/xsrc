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
 * $XConsortium: buildtree.c,v 1.6 94/04/17 21:00:35 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

#include	"string.h"


/*
 * Some standard trees for general purpose use.
 */
/*
 * Two overlapping windows with various piles of subwindows.
 */
char	*STreeGen[] = {
	".",
	"A . (5,5) 50x80",
	  "A1 A (5,10) 30x10",
		"A1a A1 (1,1) 8x8",
		  "A1b A1a (1,1) 6x6",
		    "A1c A1b (1,1) 4x4",
	  "A2 A (20,5) 10x40",
		"A2a A2 (2,2) 5x5",
		"A2b A2 (2,10) 5x5",
		"A2c A2 (2,20) 5x5",
		"A2d A2 (2,30) 5x5",
		"A2e A2 (2,36) 5x5",
	"B . (45,8) 50x80",
	  "B1 B (2,2) 45x75",
	    "B2 B1 (2,2) 40x70",
	      "B3 B2 (2,2) 35x65",
	        "B4 B3 (2,2) 30x60",
	          "B5 B4 (2,2) 25x55",
	            "B6 B5 (2,2) 20x50",
};
int 	NSTreeGen = NELEM(STreeGen);

/*
 * A single window with a variety of subwindows.
 */
char	*STreeSgl[] = {
	".",
	"top . (8, 8) 80x70",
	  "A top (10, 10) 15x15",
	    "A1 A (3, 5) 3x4",
	  "B top (40, 20) 17x14",
	    "B1 B (4, 5) 6x6",
		  "B1a B1 (1, 1) 1x1",
	    "B2 B (2, 8) 3x3",
	  "C top (20, 60) 20x30",
};
int 	NSTreeSgl = NELEM(STreeSgl);

/*
 * Collection of overlapping sibling windows designed for expose checking.
 */
char	*STreeOlsib[] = {
	". allfg",
	"A . (10,10) 70x3",
	"B . (15,3) 2x50",
	"C . (30,8) 4x60",
	"D . (17,20) 20x23",
	"E . (7,51) 60x6",
	"F . (4,78) 70x4",
	"G . (60,38) 10x4",
	"H . (60,38) 4x13",
	"I . (44,5)  20x21",
};
int 	NSTreeOlsib = NELEM(STreeOlsib);

/*
 * Duplicate a string. This is the functionallity sometimes provided by
 * strdup, but because it's not commonly supported, we provide our own.
 */
char *
xt_strdup(str)
char	*str;
{
char	*sp = NULL;

	if (str)
		sp = (char*)malloc(strlen(str)+1);
	if (sp)
		strcpy(sp, str);
	return(sp);
}

/*
 * Build a tree of windows given a description in list.
 * The tree has the given parent.
 */
struct	buildtree *
buildtree(disp, parent, list, nlist)
Display	*disp;
Window	parent;
char	**list;
int 	nlist;
{
struct	area	area;
struct	buildtree	*btbase;
struct	buildtree	*btp;
char	*line;
char	*str;
unsigned long 	pixel;
int 	depth;
int 	i;
int 	allfg = 0;
int 	borders = 0;

	pixel = W_FG;

	btbase = (struct buildtree *)calloc(sizeof(struct buildtree)*nlist, 1);
	if (btbase == NULL) {
		delete("Not enough memory in buildtree()");
		return((struct buildtree *) 0);
	}
	regid(disp, (union regtypes *)&btbase, REG_MALLOC);

	depth = getdepth(disp, parent);

	line = xt_strdup(list[0]);
	if (line == (char*)0) {
		delete("Out of memory in buildtree");
		return((struct buildtree *) 0);
	}

	btp = btbase;
	btp->name = strtok(line, " \t");
	btp->pname = (char*)0;
	btp->wid = parent;
	btp->num = nlist;
	btp->opts = 0;
	btp->uflags = 0;

	/* Pick up options */
	while ((str = strtok((char*)0, " \t")) != 0) {
		if (strcmp(str, "allfg") == 0)
			allfg = 1;
		else if (strcmp(str, "borders") == 0)
			borders = 1;
	}

	for (i = 1; i < nlist; i++) {
		btp = &btbase[i];

		line = xt_strdup(list[i]);
		if (line == (char*)0) {
			delete("Out of memory in buildtree");
			return((struct buildtree *) 0);
		}
		regid(disp, (union regtypes *)&line, REG_MALLOC);

		btp->opts = 0;
		btp->uflags = 0;

		/* Parse child description */
		btp->name = strtok(line, " ");
		btp->pname = strtok((char*)0, " ");
		btp->x = area.x = atoi(strtok((char*)0, " (,"));
		btp->y = area.y = atoi(strtok((char*)0, " ,)"));
		btp->width = area.width = atoi(strtok((char*)0, " x"));
		btp->height = area.height = atoi(strtok((char*)0, " x"));
		while ((str = strtok((char*)0, " \t")) != NULL) {
			if (strcmp(str, "unmap") == 0)
				btp->opts |= BT_UNMAP;
		}

		btp->parent = btntobtp(btbase, btp->pname);
		if (btp->parent == 0) {
			delete("Can't find window name '%s' in buildtree()", btp->pname);
			return((struct buildtree *) 0);
		}

		if (btp->opts & BT_UNMAP)
			btp->wid = creunmapchild(disp, btp->parent->wid, &area);
		else
			btp->wid = crechild(disp, btp->parent->wid, &area);

		/*
		 * Set the background pixel so overlapping of windows can be
		 * detected.
		 */
		XSetWindowBackground(disp, btp->wid, pixel);
		XClearWindow(disp, btp->wid);
		if (!allfg) {
			pixel++;
			pixel &= DEPTHMASK(depth);
		}
		if (borders) {
			XSetWindowBorderWidth(disp, btp->wid, 1);
			btp->borderwidth = 1;
		} else
			btp->borderwidth = 0;
	}
	return(btbase);
}

/*
 * Return a pointer to the buildtree structure corresponding to a given name.
 */
struct	buildtree *
btntobtp(list, name)
struct	buildtree	*list;
char	*name;
{
int 	n = list[0].num;

	while (n-- > 0) {
		if (strcmp(name, list->name) == 0)
			return(list);
		list++;
	}
	return(0);
}

/*
 * Return a pointer to the buildtree structure corresponding to a given window
 * ID.
 */
struct	buildtree *
btwtobtp(list, w)
struct	buildtree	*list;
Window	w;
{
int 	n = list[0].num;

	while (n-- > 0) {
		if (list->wid == w)
			return(list);
		list++;
	}
	return(0);
}

/*
 * Return the window id corresponding to the given name.
 */
Window
btntow(list, name)
struct	buildtree	*list;
char	*name;
{
struct	buildtree	*btp;

	btp = btntobtp(list, name);
	return(btp->wid);
}

/*
 * Return the window name corresponding to the given id.
 */
char *
btwton(list, w)
struct	buildtree	*list;
Window	w;
{
struct	buildtree	*btp;

	btp = btwtobtp(list, w);
	if (btp)
		return(btp->name);
	return (char *)NULL;
}

