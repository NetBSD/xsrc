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
 * $XConsortium: mtchvslinf.m,v 1.7 94/04/17 21:09:52 rws Exp $
 */
>>TITLE XMatchVisualInfo CH10
Status
XMatchVisualInfo(display, screen, depth, class, vinfo)
Display	*display = Dsp;
int 	screen = DefaultScreen(Dsp);
int 	depth;
int 	class;
XVisualInfo	*vinfo;
>>ASSERTION Good A
When a visual exists on screen
.A screen
of depth
.A depth
and of class
.A class,
then a call to xname returns the associated
.S XVisualInfo
structure in the
.A vinfo_return
argument, and returns non-zero.
>>STRATEGY
Initialise the list of class/depth pairs from parameter XT_VISUAL_CLASSES.
For each class/depth pair:
  Call xname.
  Verify that xname does not return zero.
  Verify that class and depth of returned structure are correct.
>>CODE
Status	s;
XVisualInfo	vi;

/* Initialise the list of class/depth pairs from parameter XT_VISUAL_CLASSES. */
	if(initvclass() < 0) {
		delete("The list of expected visual classes could not be initialised.");
		return;
	}
	vinfo = &vi;
/* For each class/depth pair: */
	for (resetvclass(); nextvclass(&class, &depth); ) {

/*   Call xname. */
		s = XCALL;

		trace("Matching depth=%d, class=%s", depth, displayclassname(class));

/*   Verify that xname does not return zero. */
		if (s == 0) {
			report("%s returned 0 with depth=%d, class=%s", 
				TestName, depth, displayclassname(class));
			FAIL;
/*   Verify that class and depth of returned structure are correct. */
		} else if (vi.class != class || vi.depth != depth) {
			/*
			 * Since the actual visual returned is not defined then
			 * all we can do is check that class and depth are OK
			 * and output all the other fields for regression test purposes.
			 */
			report(" Incorrect class or depth found");
			report(" Got %s, depth %d", displayclassname(vi.class), vi.depth);
			report(" Expecting %s, depth", displayclassname(class),
				depth);
			FAIL;
		} else {
			trace(" visualid=0x%x, screen=%d, depth=%u, class=%s",
				vi.visualid, vi.screen, vi.depth,
				displayclassname(vi.class));
			trace(" red_mask=0x%x, green_mask=0x%x, blue_mask=0x%x",
				vi.red_mask,
				vi.green_mask,
				vi.blue_mask);
			trace(" colormap_size=%d, bits_per_rgb=%d",
				vi.colormap_size,
				vi.bits_per_rgb);

			CHECK;
		}
	}

	CHECKPASS(nvclass());

>>ASSERTION Good A
When a visual does not exist on screen
.A screen
of depth
.A depth
and of class
.A class,
then a call to xname returns zero.
>>STRATEGY
Call xname with depth zero.
Verify that xname returns zero.
>>CODE
Status	s;

	/*
	 * Try with a depth of 0 which is always invalid.
	 */
	depth = 0;
	class = StaticColor;

	s = XCALL;

	if (s == 0)
		CHECK;
	else {
		report("Return value was %d", s);
		FAIL;
	}

	CHECKPASS(1);
