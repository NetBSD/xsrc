/* $XConsortium: xtfont3.c,v 1.2 94/04/17 21:00:14 rws Exp $ */
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
	{1, 8, 8, 11, -8, 0},	/* 1 */
	{1, 7, 8, 10, 0, 0},	/* 2 */
	{0, 7, 8, 7, 3, 0},	/* 3 */
	{1, 7, 7, 10, -8, 0},	/* 4 */
	{3, 7, 7, 11, -8, 0},	/* 5 */
	{0, 10, 10, 10, 0, 0},	/* 6 */
	{0, 3, 4, 10, -5, 0},	/* 7 */
	{1, 3, 5, 10, 0, 0},	/* 8 */
	{0, 0, 0, 10, -10, 0},	/* 9 */
	{0, 0, 0, 10, -10, 0},	/* 10 */
	{2, 6, 6, 11, -8, 0},	/* 11 */
	{0, 0, 0, 10, -10, 0},	/* 12 */
	{0, 0, 0, 10, -10, 0},	/* 13 */
	{2, 7, 7, 10, -8, 0},	/* 14 */
	{16, 16, 16, 10, -10, 0},	/* 15 */
	{1, 8, 8, 10, -8, 0},	/* 16 */
	{0, 13, 14, 10, 0, 0},	/* 17 */
	{0, 13, 14, 10, 0, 0},	/* 18 */
	{0, 16, 16, 5, -3, 0},	/* 19 */
	{0, 8, 9, 10, 0, 0},	/* 20 */
	{0, 8, 9, 10, 0, 0},	/* 21 */
	{0, 8, 8, 5, -3, 0},	/* 22 */
	{1, 7, 7, 11, -8, 0},	/* 23 */
	{4, 12, 16, 5, -3, 0},	/* 24 */
	{9, 9, 9, 10, -10, 0},	/* 25 */
	{0, 0, 0, 10, -10, 0},	/* 26 */
	{0, 0, 0, 10, -10, 0},	/* 27 */
	{8, 8, 8, 10, -10, 0},	/* 28 */
	{4, 6, 6, -1, 3, 0},	/* 29 */
	{1, 8, 8, 10, -8, 0},	/* 30 */
	{2, 7, 7, 11, -7, 0},	/* 31 */
	{7, 7, 7, 10, -10, 0},	/* 32 */
	{1, 3, 4, 10, 0, 0},	/* 33 */
	{1, 6, 7, 10, -6, 0},	/* 34 */
	{1, 11, 12, 10, 0, 0},	/* 35 */
	{1, 8, 9, 11, 1, 0},	/* 36 */
	{1, 13, 14, 10, 0, 0},	/* 37 */
	{1, 10, 11, 10, 0, 0},	/* 38 */
	{0, 3, 4, 10, -5, 0},	/* 39 */
	{1, 5, 6, 10, 2, 0},	/* 40 */
	{1, 5, 6, 10, 2, 0},	/* 41 */
	{1, 6, 7, 11, -5, 0},	/* 42 */
	{1, 9, 10, 8, 0, 0},	/* 43 */
	{1, 3, 4, 2, 3, 0},	/* 44 */
	{2, 8, 9, 5, -3, 0},	/* 45 */
	{1, 3, 4, 2, 0, 0},	/* 46 */
	{1, 7, 8, 10, 0, 0},	/* 47 */
	{1, 8, 9, 10, 0, 0},	/* 48 */
	{2, 7, 9, 10, 0, 0},	/* 49 */
	{1, 8, 9, 10, 0, 0},	/* 50 */
	{1, 8, 9, 10, 0, 0},	/* 51 */
	{1, 8, 9, 10, 0, 0},	/* 52 */
	{1, 8, 9, 10, 0, 0},	/* 53 */
	{1, 8, 9, 10, 0, 0},	/* 54 */
	{1, 8, 9, 10, 0, 0},	/* 55 */
	{1, 8, 9, 10, 0, 0},	/* 56 */
	{1, 8, 9, 10, 0, 0},	/* 57 */
	{1, 3, 4, 7, 0, 0},	/* 58 */
	{1, 3, 4, 7, 3, 0},	/* 59 */
	{1, 7, 8, 10, 0, 0},	/* 60 */
	{2, 7, 8, 6, -1, 0},	/* 61 */
	{1, 7, 8, 10, 0, 0},	/* 62 */
	{1, 7, 8, 10, 0, 0},	/* 63 */
	{1, 13, 14, 10, 0, 0},	/* 64 */
	{0, 8, 9, 10, 0, 0},	/* 65 */
	{0, 7, 8, 10, 0, 0},	/* 66 */
	{0, 8, 9, 10, 0, 0},	/* 67 */
	{0, 8, 9, 10, 0, 0},	/* 68 */
	{0, 7, 8, 10, 0, 0},	/* 69 */
	{0, 7, 8, 10, 0, 0},	/* 70 */
	{0, 9, 10, 10, 0, 0},	/* 71 */
	{0, 8, 9, 10, 0, 0},	/* 72 */
	{0, 2, 3, 10, 0, 0},	/* 73 */
	{0, 6, 7, 10, 0, 0},	/* 74 */
	{0, 8, 9, 10, 0, 0},	/* 75 */
	{0, 6, 7, 10, 0, 0},	/* 76 */
	{0, 11, 12, 10, 0, 0},	/* 77 */
	{0, 8, 9, 10, 0, 0},	/* 78 */
	{0, 9, 10, 10, 0, 0},	/* 79 */
	{0, 7, 8, 10, 0, 0},	/* 80 */
	{0, 9, 10, 10, 1, 0},	/* 81 */
	{0, 8, 9, 10, 0, 0},	/* 82 */
	{0, 7, 8, 10, 0, 0},	/* 83 */
	{0, 8, 9, 10, 0, 0},	/* 84 */
	{0, 8, 9, 10, 0, 0},	/* 85 */
	{0, 8, 9, 10, 0, 0},	/* 86 */
	{0, 13, 14, 10, 0, 0},	/* 87 */
	{0, 8, 9, 10, 0, 0},	/* 88 */
	{0, 8, 9, 10, 0, 0},	/* 89 */
	{0, 8, 9, 10, 0, 0},	/* 90 */
	{1, 6, 7, 10, 3, 0},	/* 91 */
	{1, 7, 8, 10, 0, 0},	/* 92 */
	{1, 6, 7, 10, 3, 0},	/* 93 */
	{1, 7, 8, 9, -1, 0},	/* 94 */
	{1, 9, 10, 8, -2, 0},	/* 95 */
	{0, 3, 4, 10, -5, 0},	/* 96 */
	{0, 7, 8, 7, 0, 0},	/* 97 */
	{0, 7, 8, 10, 0, 0},	/* 98 */
	{0, 7, 8, 7, 0, 0},	/* 99 */
	{0, 7, 8, 10, 0, 0},	/* 100 */
	{0, 7, 8, 7, 0, 0},	/* 101 */
	{0, 5, 6, 10, 0, 0},	/* 102 */
	{0, 7, 8, 7, 3, 0},	/* 103 */
	{0, 7, 8, 10, 0, 0},	/* 104 */
	{0, 2, 3, 10, 0, 0},	/* 105 */
	{0, 4, 5, 10, 2, 0},	/* 106 */
	{0, 7, 8, 10, 0, 0},	/* 107 */
	{0, 2, 3, 10, 0, 0},	/* 108 */
	{0, 10, 11, 7, 0, 0},	/* 109 */
	{0, 7, 8, 7, 0, 0},	/* 110 */
	{0, 7, 8, 7, 0, 0},	/* 111 */
	{0, 7, 8, 7, 3, 0},	/* 112 */
	{0, 7, 8, 7, 3, 0},	/* 113 */
	{0, 5, 6, 7, 0, 0},	/* 114 */
	{0, 6, 7, 7, 0, 0},	/* 115 */
	{0, 5, 6, 9, 0, 0},	/* 116 */
	{0, 7, 8, 7, 0, 0},	/* 117 */
	{0, 8, 9, 7, 0, 0},	/* 118 */
	{0, 11, 12, 7, 0, 0},	/* 119 */
	{0, 6, 7, 7, 0, 0},	/* 120 */
	{0, 7, 8, 7, 3, 0},	/* 121 */
	{0, 6, 7, 7, 0, 0},	/* 122 */
	{1, 7, 8, 10, 3, 0},	/* 123 */
	{2, 4, 5, 10, 3, 0},	/* 124 */
	{1, 7, 8, 10, 3, 0},	/* 125 */
	{1, 9, 10, 6, -3, 0},	/* 126 */
};
static XFontProp props[] = {
	{XA_COPYRIGHT, 0},
	{XA_RESOLUTION, 78},
	{XA_UNDERLINE_POSITION, 3},
};

char	*xtfont3cpright = "These glyphs are unencumbered";

XFontStruct xtfont3 = {
	(XExtData*)0,
	(Font)0,
	FontLeftToRight,	/* direction */
	1,	/* min_byte2 */
	126,	/* max_byte2 */
	0,	/* min_byte1 */
	0,	/* max_byte1 */
	1,	/* all chars exist */
	2,	/* default char */
	3,	/* n_properties */
	props,
	{0, 0, 0, -1, -10, 0},
	{16, 16, 16, 11, 3, 0},
	perchar,
	11,	/* font ascent */
	3,	/* font descent */
};
