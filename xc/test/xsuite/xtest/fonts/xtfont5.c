/* $XConsortium: xtfont5.c,v 1.2 94/04/17 21:00:16 rws Exp $ */
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
	{0, 128, 129, 66, -3, 0},	/* 0 */
	{0, 127, 128, 63, 1, 0},	/* 1 */
	{-1, 125, 63, 64, 0, 0},	/* 2 */
	{1, 126, 63, 64, 0, 0},	/* 3 */
};
static XFontProp props[] = {
	{XA_COPYRIGHT, 0},
};

char	*xtfont5cpright = "These glyphs are unencumbered";

XFontStruct xtfont5 = {
	(XExtData*)0,
	(Font)0,
	FontLeftToRight,	/* direction */
	0,	/* min_byte2 */
	3,	/* max_byte2 */
	0,	/* min_byte1 */
	0,	/* max_byte1 */
	1,	/* all chars exist */
	1234,	/* default char */
	1,	/* n_properties */
	props,
	{-1, 125, 63, 63, -3, 0},
	{1, 128, 129, 66, 1, 0},
	perchar,
	66,	/* font ascent */
	1,	/* font descent */
};
