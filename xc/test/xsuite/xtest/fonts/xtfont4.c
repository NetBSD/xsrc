/* $XConsortium: xtfont4.c,v 1.2 94/04/17 21:00:15 rws Exp $ */
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
	{-8, -1, -8, 11, -8, 0},	/* 1 */
	{-7, -1, -8, 10, 0, 0},	/* 2 */
	{-7, 0, -8, 7, 3, 0},	/* 3 */
	{-7, -1, -7, 10, -8, 0},	/* 4 */
	{-7, -3, -7, 11, -8, 0},	/* 5 */
	{-10, 0, -10, 10, 0, 0},	/* 6 */
	{-3, 0, -4, 10, -5, 0},	/* 7 */
	{-3, -1, -5, 10, 0, 0},	/* 8 */
	{0, 0, 0, 10, -10, 0},	/* 9 */
	{0, 0, 0, 10, -10, 0},	/* 10 */
	{-6, -2, -6, 11, -8, 0},	/* 11 */
	{0, 0, 0, 10, -10, 0},	/* 12 */
	{0, 0, 0, 10, -10, 0},	/* 13 */
	{-7, -2, -7, 10, -8, 0},	/* 14 */
	{-16, -16, -16, 10, -10, 0},	/* 15 */
	{-8, -1, -8, 10, -8, 0},	/* 16 */
	{-13, 0, -14, 10, 0, 0},	/* 17 */
	{-13, 0, -14, 10, 0, 0},	/* 18 */
	{-16, 0, -16, 5, -3, 0},	/* 19 */
	{-8, 0, -9, 10, 0, 0},	/* 20 */
	{-8, 0, -9, 10, 0, 0},	/* 21 */
	{-8, 0, -8, 5, -3, 0},	/* 22 */
	{-7, -1, -7, 11, -8, 0},	/* 23 */
	{-12, -4, -16, 5, -3, 0},	/* 24 */
	{-9, -9, -9, 10, -10, 0},	/* 25 */
	{0, 0, 0, 10, -10, 0},	/* 26 */
	{0, 0, 0, 10, -10, 0},	/* 27 */
	{-8, -8, -8, 10, -10, 0},	/* 28 */
	{-6, -4, -6, -1, 3, 0},	/* 29 */
	{-8, -1, -8, 10, -8, 0},	/* 30 */
	{-7, -2, -7, 11, -7, 0},	/* 31 */
	{-7, -7, -7, 10, -10, 0},	/* 32 */
	{-3, -1, -4, 10, 0, 0},	/* 33 */
	{-6, -1, -7, 10, -6, 0},	/* 34 */
	{-11, -1, -12, 10, 0, 0},	/* 35 */
	{-8, -1, -9, 11, 1, 0},	/* 36 */
	{-13, -1, -14, 10, 0, 0},	/* 37 */
	{-10, -1, -11, 10, 0, 0},	/* 38 */
	{-3, 0, -4, 10, -5, 0},	/* 39 */
	{-5, -1, -6, 10, 2, 0},	/* 40 */
	{-5, -1, -6, 10, 2, 0},	/* 41 */
	{-6, -1, -7, 11, -5, 0},	/* 42 */
	{-9, -1, -10, 8, 0, 0},	/* 43 */
	{-3, -1, -4, 2, 3, 0},	/* 44 */
	{-8, -2, -9, 5, -3, 0},	/* 45 */
	{-3, -1, -4, 2, 0, 0},	/* 46 */
	{-7, -1, -8, 10, 0, 0},	/* 47 */
	{-8, -1, -9, 10, 0, 0},	/* 48 */
	{-7, -2, -9, 10, 0, 0},	/* 49 */
	{-8, -1, -9, 10, 0, 0},	/* 50 */
	{-8, -1, -9, 10, 0, 0},	/* 51 */
	{-8, -1, -9, 10, 0, 0},	/* 52 */
	{-8, -1, -9, 10, 0, 0},	/* 53 */
	{-8, -1, -9, 10, 0, 0},	/* 54 */
	{-8, -1, -9, 10, 0, 0},	/* 55 */
	{-8, -1, -9, 10, 0, 0},	/* 56 */
	{-8, -1, -9, 10, 0, 0},	/* 57 */
	{-3, -1, -4, 7, 0, 0},	/* 58 */
	{-3, -1, -4, 7, 3, 0},	/* 59 */
	{-7, -1, -8, 10, 0, 0},	/* 60 */
	{-7, -2, -8, 6, -1, 0},	/* 61 */
	{-7, -1, -8, 10, 0, 0},	/* 62 */
	{-7, -1, -8, 10, 0, 0},	/* 63 */
	{-13, -1, -14, 10, 0, 0},	/* 64 */
	{-8, 0, -9, 10, 0, 0},	/* 65 */
	{-7, 0, -8, 10, 0, 0},	/* 66 */
	{-8, 0, -9, 10, 0, 0},	/* 67 */
	{-8, 0, -9, 10, 0, 0},	/* 68 */
	{-7, 0, -8, 10, 0, 0},	/* 69 */
	{-7, 0, -8, 10, 0, 0},	/* 70 */
	{-9, 0, -10, 10, 0, 0},	/* 71 */
	{-8, 0, -9, 10, 0, 0},	/* 72 */
	{-2, 0, -3, 10, 0, 0},	/* 73 */
	{-6, 0, -7, 10, 0, 0},	/* 74 */
	{-8, 0, -9, 10, 0, 0},	/* 75 */
	{-6, 0, -7, 10, 0, 0},	/* 76 */
	{-11, 0, -12, 10, 0, 0},	/* 77 */
	{-8, 0, -9, 10, 0, 0},	/* 78 */
	{-9, 0, -10, 10, 0, 0},	/* 79 */
	{-7, 0, -8, 10, 0, 0},	/* 80 */
	{-9, 0, -10, 10, 1, 0},	/* 81 */
	{-8, 0, -9, 10, 0, 0},	/* 82 */
	{-7, 0, -8, 10, 0, 0},	/* 83 */
	{-8, 0, -9, 10, 0, 0},	/* 84 */
	{-8, 0, -9, 10, 0, 0},	/* 85 */
	{-8, 0, -9, 10, 0, 0},	/* 86 */
	{-13, 0, -14, 10, 0, 0},	/* 87 */
	{-8, 0, -9, 10, 0, 0},	/* 88 */
	{-8, 0, -9, 10, 0, 0},	/* 89 */
	{-8, 0, -9, 10, 0, 0},	/* 90 */
	{-6, -1, -7, 10, 3, 0},	/* 91 */
	{-7, -1, -8, 10, 0, 0},	/* 92 */
	{-6, -1, -7, 10, 3, 0},	/* 93 */
	{-7, -1, -8, 9, -1, 0},	/* 94 */
	{-9, -1, -10, 8, -2, 0},	/* 95 */
	{-3, 0, -4, 10, -5, 0},	/* 96 */
	{-7, 0, -8, 7, 0, 0},	/* 97 */
	{-7, 0, -8, 10, 0, 0},	/* 98 */
	{-7, 0, -8, 7, 0, 0},	/* 99 */
	{-7, 0, -8, 10, 0, 0},	/* 100 */
	{-7, 0, -8, 7, 0, 0},	/* 101 */
	{-5, 0, -6, 10, 0, 0},	/* 102 */
	{-7, 0, -8, 7, 3, 0},	/* 103 */
	{-7, 0, -8, 10, 0, 0},	/* 104 */
	{-2, 0, -3, 10, 0, 0},	/* 105 */
	{-4, 0, -5, 10, 2, 0},	/* 106 */
	{-7, 0, -8, 10, 0, 0},	/* 107 */
	{-2, 0, -3, 10, 0, 0},	/* 108 */
	{-10, 0, -11, 7, 0, 0},	/* 109 */
	{-7, 0, -8, 7, 0, 0},	/* 110 */
	{-7, 0, -8, 7, 0, 0},	/* 111 */
	{-7, 0, -8, 7, 3, 0},	/* 112 */
	{-7, 0, -8, 7, 3, 0},	/* 113 */
	{-5, 0, -6, 7, 0, 0},	/* 114 */
	{-6, 0, -7, 7, 0, 0},	/* 115 */
	{-5, 0, -6, 9, 0, 0},	/* 116 */
	{-7, 0, -8, 7, 0, 0},	/* 117 */
	{-8, 0, -9, 7, 0, 0},	/* 118 */
	{-11, 0, -12, 7, 0, 0},	/* 119 */
	{-6, 0, -7, 7, 0, 0},	/* 120 */
	{-7, 0, -8, 7, 3, 0},	/* 121 */
	{-6, 0, -7, 7, 0, 0},	/* 122 */
	{-7, -1, -8, 10, 3, 0},	/* 123 */
	{-4, -2, -5, 10, 3, 0},	/* 124 */
	{-7, -1, -8, 10, 3, 0},	/* 125 */
	{-9, -1, -10, 6, -3, 0},	/* 126 */
};
static XFontProp props[] = {
	{XA_COPYRIGHT, 0},
	{XA_MIN_SPACE, 2},
	{XA_NORM_SPACE, 4},
	{XA_RESOLUTION, 78},
	{XA_UNDERLINE_POSITION, 3},
};

char	*xtfont4cpright = "These glyphs are unencumbered";

XFontStruct xtfont4 = {
	(XExtData*)0,
	(Font)0,
	FontRightToLeft,	/* direction */
	1,	/* min_byte2 */
	126,	/* max_byte2 */
	0,	/* min_byte1 */
	0,	/* max_byte1 */
	1,	/* all chars exist */
	2,	/* default char */
	5,	/* n_properties */
	props,
	{-16, -16, -16, -1, -10, 0},
	{0, 0, 0, 11, 3, 0},
	perchar,
	11,	/* font ascent */
	3,	/* font descent */
};
