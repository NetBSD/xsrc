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
 * $XConsortium: lstpxmpfrm.m,v 1.10 94/04/17 21:02:29 rws Exp $
 */
>>TITLE XListPixmapFormats CH02
XPixmapFormatValues *
XListPixmapFormats(display, count_return)
Display	*display = Dsp;
int	*count_return;
>>EXTERN
#include <string.h>
#define SEPS " "
>>ASSERTION Good A
A call to xname returns an array of
.S XPixampFormatValues
structures which can be freed with
.S XFree
of length returned in the
.A count_return
argument that describe the types of Z format images that are supported
by the display specified by the
.A display
argument.
>>STRATEGY
Obtain a list of pixmap formats using xname.
Verify the depths match those specified in XT_PIXMAP_DEPTHS.
Free the list using XFree.
>>CODE
XPixmapFormatValues	*plst = (XPixmapFormatValues *) NULL;
XPixmapFormatValues	*pt = (XPixmapFormatValues *) NULL;
char			*tok;
char			*dp;
int			*Pdepth;
int			Ndepth;
int			nr=0;
int			i;
int			j;
int			found;

	count_return = &nr;
	pt = plst = XCALL;
	if(plst == (XPixmapFormatValues *) NULL) {
		delete("%s() returned NULL", TestName);
		return;
	} else
		CHECK;

	dp = config.pixmap_depths;
	if (dp == (char*)0) {
		delete("XT_PIXMAP_DEPTHS not set");
		return;
	} else
		CHECK;

	/*
	 * The number of pixmap depths in the string is less than the
	 * length of the string.
	 */
	Pdepth = (int*)malloc(strlen(dp) * sizeof(int));
	if (Pdepth == (int*)0) {
		delete("Could not allocate memory for pixmap depth array");
		return;
	} else
		CHECK;

	Ndepth = 0;
	for (tok = strtok(dp, SEPS); tok; tok = strtok((char*)0, SEPS))
		Pdepth[Ndepth++]  = atov(tok);

	if(nr != Ndepth) {
		report("%s() returned %d structures", TestName, nr);
		report("Expected %d structures", Ndepth);
		FAIL;
	} else
		CHECK;

	for(j = 0; j < Ndepth; j++) {
		trace("pixmap depth expected %d", Pdepth[j]);
		found = 0;
		for(pt = plst, i = 0; i < nr; i++, pt++) {
			trace("pixmap depth found %d", pt->depth);
			if(pt->depth == Pdepth[j]) {
				found = 1;
				break;
			}
		}
		if(!found) {
			report("Pixmap depth %d not returned by %s", Pdepth[j],
								TestName);
			FAIL;
		} else
			CHECK;
	}

	for(pt = plst, i = 0; i < nr; pt++) {
		trace("Pixmap format %d : depth %d, bits_per_pixel %d, scanline_pad %d.", ++i,
			pt->depth, pt-> bits_per_pixel, pt->scanline_pad);
	}
	XFree( (char *) plst);
	CHECKPASS(4+Ndepth);

>>ASSERTION Bad B 1
When insufficient memory is available, then a call to xname returns NULL.
