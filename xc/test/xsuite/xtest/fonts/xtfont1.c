/* $XConsortium: xtfont1.c,v 1.2 94/04/17 21:00:12 rws Exp $ */
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
 */
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"Xatom.h"

static XCharStruct perchar[] = {
	{0, 0, 7, 0, 0, 0},	/* 0 */
	{0, 6, 7, 5, 0, 0},	/* 1 */
	{0, 6, 7, 7, 1, 0},	/* 2 */
	{0, 6, 7, 9, 2, 0},	/* 3 */
	{0, 6, 7, 9, 2, 0},	/* 4 */
	{0, 6, 7, 9, 2, 0},	/* 5 */
	{0, 6, 7, 9, 2, 0},	/* 6 */
	{0, 6, 7, 8, -4, 0},	/* 7 */
	{0, 6, 7, 8, 1, 0},	/* 8 */
	{0, 6, 7, 9, 2, 0},	/* 9 */
	{0, 6, 7, 9, 2, 0},	/* 10 */
	{0, 4, 7, 10, -2, 0},	/* 11 */
	{0, 4, 7, 4, 3, 0},	/* 12 */
	{2, 7, 7, 4, 3, 0},	/* 13 */
	{2, 7, 7, 10, -2, 0},	/* 14 */
	{0, 7, 7, 10, 3, 0},	/* 15 */
	{0, 7, 7, 8, -6, 0},	/* 16 */
	{0, 7, 7, 6, -4, 0},	/* 17 */
	{0, 7, 7, 4, -2, 0},	/* 18 */
	{0, 7, 7, 2, 0, 0},	/* 19 */
	{0, 7, 7, 0, 2, 0},	/* 20 */
	{2, 7, 7, 10, 3, 0},	/* 21 */
	{0, 4, 7, 10, 3, 0},	/* 22 */
	{0, 7, 7, 10, -2, 0},	/* 23 */
	{0, 7, 7, 4, 3, 0},	/* 24 */
	{2, 4, 7, 10, 3, 0},	/* 25 */
	{0, 6, 7, 8, 1, 0},	/* 26 */
	{0, 6, 7, 8, 1, 0},	/* 27 */
	{0, 6, 7, 5, 1, 0},	/* 28 */
	{0, 6, 7, 8, 1, 0},	/* 29 */
	{0, 6, 7, 8, 1, 0},	/* 30 */
	{2, 4, 7, 4, -2, 0},	/* 31 */
	{0, 0, 7, 0, 0, 0},	/* 32 */
	{2, 4, 7, 8, 1, 0},	/* 33 */
	{1, 6, 7, 8, -5, 0},	/* 34 */
	{1, 6, 7, 8, 1, 0},	/* 35 */
	{0, 6, 7, 8, 1, 0},	/* 36 */
	{0, 6, 7, 8, 1, 0},	/* 37 */
	{0, 6, 7, 8, 1, 0},	/* 38 */
	{1, 5, 7, 8, -4, 0},	/* 39 */
	{1, 5, 7, 8, 1, 0},	/* 40 */
	{1, 5, 7, 8, 1, 0},	/* 41 */
	{0, 6, 7, 6, 0, 0},	/* 42 */
	{0, 6, 7, 6, 0, 0},	/* 43 */
	{1, 5, 7, 2, 2, 0},	/* 44 */
	{0, 6, 7, 4, -2, 0},	/* 45 */
	{1, 5, 7, 1, 2, 0},	/* 46 */
	{0, 6, 7, 8, 1, 0},	/* 47 */
	{0, 6, 7, 8, 1, 0},	/* 48 */
	{0, 6, 7, 8, 1, 0},	/* 49 */
	{0, 6, 7, 8, 1, 0},	/* 50 */
	{0, 6, 7, 8, 1, 0},	/* 51 */
	{0, 6, 7, 8, 1, 0},	/* 52 */
	{0, 6, 7, 8, 1, 0},	/* 53 */
	{0, 6, 7, 8, 1, 0},	/* 54 */
	{0, 6, 7, 8, 1, 0},	/* 55 */
	{0, 6, 7, 8, 1, 0},	/* 56 */
	{0, 6, 7, 8, 1, 0},	/* 57 */
	{1, 5, 7, 6, 2, 0},	/* 58 */
	{1, 5, 7, 6, 2, 0},	/* 59 */
	{0, 6, 7, 8, 1, 0},	/* 60 */
	{0, 6, 7, 6, -1, 0},	/* 61 */
	{0, 6, 7, 8, 1, 0},	/* 62 */
	{0, 6, 7, 8, 1, 0},	/* 63 */
	{0, 6, 7, 8, 1, 0},	/* 64 */
	{0, 6, 7, 8, 1, 0},	/* 65 */
	{0, 6, 7, 8, 1, 0},	/* 66 */
	{0, 6, 7, 8, 1, 0},	/* 67 */
	{0, 6, 7, 8, 1, 0},	/* 68 */
	{0, 6, 7, 8, 1, 0},	/* 69 */
	{0, 6, 7, 8, 1, 0},	/* 70 */
	{0, 6, 7, 8, 1, 0},	/* 71 */
	{0, 6, 7, 8, 1, 0},	/* 72 */
	{0, 6, 7, 8, 1, 0},	/* 73 */
	{0, 6, 7, 8, 1, 0},	/* 74 */
	{0, 6, 7, 8, 1, 0},	/* 75 */
	{0, 6, 7, 8, 1, 0},	/* 76 */
	{0, 6, 7, 8, 1, 0},	/* 77 */
	{0, 6, 7, 8, 1, 0},	/* 78 */
	{0, 6, 7, 8, 1, 0},	/* 79 */
	{0, 6, 7, 8, 1, 0},	/* 80 */
	{0, 6, 7, 8, 2, 0},	/* 81 */
	{0, 6, 7, 8, 1, 0},	/* 82 */
	{0, 6, 7, 8, 1, 0},	/* 83 */
	{0, 6, 7, 8, 1, 0},	/* 84 */
	{0, 6, 7, 8, 1, 0},	/* 85 */
	{0, 6, 7, 8, 1, 0},	/* 86 */
	{0, 6, 7, 8, 1, 0},	/* 87 */
	{0, 6, 7, 8, 1, 0},	/* 88 */
	{0, 6, 7, 8, 1, 0},	/* 89 */
	{0, 6, 7, 8, 1, 0},	/* 90 */
	{1, 5, 7, 8, 1, 0},	/* 91 */
	{0, 6, 7, 8, 1, 0},	/* 92 */
	{1, 5, 7, 8, 1, 0},	/* 93 */
	{0, 6, 7, 8, -4, 0},	/* 94 */
	{0, 6, 7, 0, 2, 0},	/* 95 */
	{1, 5, 7, 8, -4, 0},	/* 96 */
	{0, 6, 7, 5, 1, 0},	/* 97 */
	{0, 6, 7, 8, 1, 0},	/* 98 */
	{0, 6, 7, 5, 1, 0},	/* 99 */
	{0, 6, 7, 8, 1, 0},	/* 100 */
	{0, 6, 7, 5, 1, 0},	/* 101 */
	{0, 6, 7, 8, 1, 0},	/* 102 */
	{0, 6, 7, 5, 3, 0},	/* 103 */
	{0, 6, 7, 8, 1, 0},	/* 104 */
	{0, 6, 7, 8, 1, 0},	/* 105 */
	{0, 6, 7, 8, 3, 0},	/* 106 */
	{0, 6, 7, 8, 1, 0},	/* 107 */
	{0, 6, 7, 8, 1, 0},	/* 108 */
	{0, 6, 7, 5, 1, 0},	/* 109 */
	{0, 6, 7, 5, 1, 0},	/* 110 */
	{0, 6, 7, 5, 1, 0},	/* 111 */
	{0, 6, 7, 5, 3, 0},	/* 112 */
	{0, 6, 7, 5, 3, 0},	/* 113 */
	{0, 6, 7, 5, 1, 0},	/* 114 */
	{0, 6, 7, 5, 1, 0},	/* 115 */
	{0, 6, 7, 7, 1, 0},	/* 116 */
	{0, 6, 7, 5, 1, 0},	/* 117 */
	{0, 6, 7, 5, 1, 0},	/* 118 */
	{0, 6, 7, 5, 1, 0},	/* 119 */
	{0, 6, 7, 5, 1, 0},	/* 120 */
	{0, 6, 7, 5, 3, 0},	/* 121 */
	{0, 6, 7, 5, 1, 0},	/* 122 */
	{1, 5, 7, 8, 1, 0},	/* 123 */
	{2, 4, 7, 8, 1, 0},	/* 124 */
	{1, 5, 7, 8, 1, 0},	/* 125 */
	{0, 6, 7, 8, -5, 0},	/* 126 */
	{1, 6, 7, 10, 3, 0},	/* 127 */
};
static XFontProp props[] = {
	{XA_COPYRIGHT, 0},
	{XA_RESOLUTION, 100},
	{XA_MIN_SPACE, 4},
	{XA_NORM_SPACE, 4},
};

char	*xtfont1cpright = "Public domain font.  Share and enjoy.";

XFontStruct xtfont1 = {
	(XExtData*)0,
	(Font)0,
	FontLeftToRight,	/* direction */
	0,	/* min_byte2 */
	127,	/* max_byte2 */
	0,	/* min_byte1 */
	0,	/* max_byte1 */
	1,	/* all chars exist */
	0,	/* default char */
	4,	/* n_properties */
	props,
	{0, 0, 7, 0, -6, 0},
	{2, 7, 7, 10, 3, 0},
	perchar,
	10,	/* font ascent */
	3,	/* font descent */
};
