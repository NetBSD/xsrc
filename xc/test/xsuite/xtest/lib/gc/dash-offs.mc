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
 * $XConsortium: dash-offs.mc,v 1.10 94/04/17 21:14:38 rws Exp $
 */
>>EXTERN
static	char dolist[] = {
	8, 1, 2, 3, 1, 1, 1, 1, 1, 11, 2, 8};
>>ASSERTION Good A
The
.M dash_offset
defines the phase of the pattern,
specifying how many pixels into the dash-list the pattern
begins in any single graphics request.
>>STRATEGY
For both odd and even line widths do:
	Set graphics coordinates for dashed lines 
		(includes horizontal and vertical cases,
		and includes joins and caps where relevant).
	Set the line_style of the GC to LineOnOffDash using XSetLineAttributes.
	For various dash_offsets:
		Set the dash_offset of the GC using XSetDashes.
		Clear drawable.
		Draw lines.
		Verify that dashes drawn correspond to dash_list (use pixmap checking).
	Set dash_offset of the GC to zero using XSetDashes.
	Clear drawable.
	Draw lines.
	Save the image on the drawable.
	Set dash_offset of the GC to multiple of combined dash length using XSetDashes.
	Clear drawable.
	Draw lines.
	Verify that the images drawn were the same.
>>CODE
static	unsigned short	dashoff[] = {
	0x0, 0x2, 0x7, 0x99, 0xfe78};
XVisualInfo	*vp;
XImage	*dosav;
int 	i;
int 	combinedlen = 0;
unsigned int	lw;

	/*
	 * Calculate total length of list needed to check circularity.
	 */
	for (i = 0; i < NELEM(dolist); i++)
		combinedlen += dolist[i];

	if (NELEM(dolist) & 1)	/* Odd length */
		combinedlen *= 2;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setfordash();
		setlinestyle(A_DISPLAY, A_GC, LineOnOffDash);
		for(lw=10; lw <= 11; lw++) { /* both odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);

			for (i = 0; i < NELEM(dashoff); i++) {
				trace("dash offset of %d", dashoff[i]);
				XSetDashes(A_DISPLAY, A_GC, dashoff[i], dolist, NELEM(dolist));
				
				dclear(A_DISPLAY, A_DRAWABLE);
				XCALL;

				PIXCHECK(A_DISPLAY, A_DRAWABLE);
			}

			XSetDashes(A_DISPLAY, A_GC, 0, dolist, NELEM(dolist));
			dclear(A_DISPLAY, A_DRAWABLE);
			XCALL;
			dosav = savimage(A_DISPLAY, A_DRAWABLE);

			trace("dash offset of the combined length of dash list %d", 
									combinedlen);
			XSetDashes(A_DISPLAY, A_GC, combinedlen, dolist, NELEM(dolist));
			dclear(A_DISPLAY, A_DRAWABLE);
			XCALL;
			if (compsavimage(A_DISPLAY, A_DRAWABLE, dosav))
				CHECK;
			else {
				report("dash offset of 0 not equivalent to sum of elements in dash list (%d) for linewidth %u", combinedlen, lw);
				FAIL;
			}

			dclear(A_DISPLAY, A_DRAWABLE);
		} /* odd and even widths */
	}

	CHECKPASS(2*(NELEM(dashoff)+1)*nvinf());
