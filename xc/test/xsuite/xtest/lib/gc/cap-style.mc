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
 * $XConsortium: cap-style.mc,v 1.18 94/04/17 21:14:35 rws Exp $
 */
>>ASSERTION Good A
>># this first one was in line-style but doesn't apply to rectangle(s), but
>># whenever this assertion is required then cap-style is in as well so
>># put this at the beginning of cap-style so it looks as if it was at the
>># end of the line-style stuff (because line-style normally immediately
>># precedes cap-style in auto. inclusion order).	kieron
When 
.M line_width
is greater than or equal to one,
and 
.M line_style
is 
.S LineSolid ,
and a line is drawn from [x1, y1] to [x2, y2],
and a line is drawn from [x2, y2] to [x1, y1],
then the same pixels are drawn in each case.
>>STRATEGY
Draw lines in one direction.
Save image.
Clear drawable.
Draw lines in opposite direction.
Verify that the image is the same as that saved and that something was drawn.
>>CODE
XVisualInfo	*vp;
struct	linedata {
	int 	x1;
	int 	y1;
	int 	x2;
	int 	y2;
};
static struct	linedata linedata[] = {
	{1, 1, 70, 10},
	{3, 5, 10, 50},
	{50, 25, 10, 20},
	{45, 18, 34, 80},
	{1, 77, 38, 22},
	{88, 12, 54, 23},
	};
struct	linedata	*lp;
int 	ndata = NELEM(linedata);
XImage	*lsimp;
unsigned int 	lswidth;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		lswidth = 1;
		for (lp = linedata; lp < &linedata[ndata]; lp++) {
			setwidth(A_DISPLAY, A_GC, lswidth++);
			drawline(lp->x1, lp->y1, lp->x2, lp->y2);
		}

		lsimp = savimage(A_DISPLAY, A_DRAWABLE);
		dclear(A_DISPLAY, A_DRAWABLE);

		/* Draw the lines reversed */
		lswidth = 1;
		for (lp = linedata; lp < &linedata[ndata]; lp++) {
			setwidth(A_DISPLAY, A_GC, lswidth++);
			drawline(lp->x2, lp->y2, lp->x1, lp->y1);
		}

		/* Compare the results */
		if (compsavimage(A_DISPLAY, A_DRAWABLE, lsimp))
			CHECK;
		else {
			report("Lines not same when drawn in opposite direction");
			FAIL;
		}

		/* assume we're not allowed to draw nothing, complain if so */
		if (!checkarea(A_DISPLAY, A_DRAWABLE,
			(struct area *)0, W_BG, W_BG, CHECK_IN | CHECK_DIFFER))
			CHECK;
		else {
			report("%s didn't draw anything in opposite direction", TestName);
			FAIL;
		}
	}

	CHECKPASS(2*nvinf());

>>EXTERN

#define	CAP_X1 15
#define	CAP_Y1 15
#define	CAP_X2 63
#define	CAP_Y2 42

>>ASSERTION Good A
When the
.M cap_style
is
.S CapNotLast
and the
.M line_width
is greater than zero (> 0),
then this is equivalent to 
.S CapButt .
>>STRATEGY
For a variety of line widths, both odd and even.
  Draw line with CapNotLast.
  Save the image on the drawable.
  Draw line with CapButt.
  Verify that the images drawn were the same.
>>CODE
XVisualInfo	*vp;
XImage	*capsav;
unsigned int 	capw;
static unsigned int 	capwidths[] = {
	1, 2, 5, 6, 10, 11, 40, 41};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for (capw = 0; capw < NELEM(capwidths); capw++) {
			setwidth(A_DISPLAY, A_GC, capwidths[capw]);

			setcapstyle(A_DISPLAY, A_GC, CapNotLast);
			XCALL;
			capsav = savimage(A_DISPLAY, A_DRAWABLE);
			dclear(A_DISPLAY, A_DRAWABLE);

			setcapstyle(A_DISPLAY, A_GC, CapButt);
			XCALL;
			if (compsavimage(A_DISPLAY, A_DRAWABLE, capsav))
				CHECK;
			else {
				report("CapNotLast was not equivalent to CapButt for linewidth %d", capwidths[capw]);
				FAIL;
			}
			/* assume we're not allowed to draw nothing, complain if so */
			if (!checkarea(A_DISPLAY, A_DRAWABLE,
				(struct area *)0, W_BG, W_BG, CHECK_IN | CHECK_DIFFER))
				CHECK;
			else {
				report("%s didn't draw anything with CapButt", TestName);
				FAIL;
			}
			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}
	CHECKPASS(2*nvinf()*NELEM(capwidths));

>>ASSERTION Good A
When the
.M cap_style
is
.S CapNotLast
and the
.M line_width
is zero (0),
then this is equivalent to 
.S CapButt
except that the final endpoint is not drawn.
>>STRATEGY
Draw line with CapButt.
Set GC component function to GXxor.
Draw line with CapNotLast.
Verify that only the final end point is set.
>>CODE
XVisualInfo	*vp;
int 	capx1, capy1, capx2, capy2;
struct	area	area;

	capx1 = 10; capy1 = 10;
	capx2 = 70; capy2 = 30;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);

		setcapstyle(A_DISPLAY, A_GC, CapButt);
		drawline(capx1, capy1, capx2, capy2);

		XSetFunction(A_DISPLAY, A_GC, GXxor);
		setcapstyle(A_DISPLAY, A_GC, CapNotLast);
		drawline(capx1, capy1, capx2, capy2);

		/* Only the final endpoint should be set */
#if T_XDrawArc || T_XDrawArcs
		setarea(&area, capx2, capy1, 1, 1);
#else
		setarea(&area, capx2, capy2, 1, 1);
#endif
		if (checkarea(A_DISPLAY, A_DRAWABLE, &area, W_FG, W_BG, CHECK_ALL))
			CHECK;
		else {
			report("Cap style incorrect for thin line and CapNotLast");
			FAIL;
		}
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When the
.M cap_style
is
.S CapButt ,
then the line is square at the endpoint (perpendicular to the slope of the line)
with no projection beyond.
>>STRATEGY
Draw line with CapButt and odd or even line width.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for(lw=8; lw <= 9; lw++) { /* odd or even line widths */
			setwidth(A_DISPLAY, A_GC, lw);
			setcapstyle(A_DISPLAY, A_GC, CapButt);

			drawline(15, 15, 50, 33);

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(2*nvinf());
>>ASSERTION Good A
When the
.M cap_style
is
.S CapRound
and
.M line-width
is zero,
then this is equivalent to 
.S CapButt .
>>STRATEGY
Draw line with CapRound.
Save the image on the drawable.
Draw line with CapButt.
Verify that the images drawn were the same.
>>CODE
XVisualInfo	*vp;
XImage	*savimp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);
		setcapstyle(A_DISPLAY, A_GC, CapRound);

		drawline(15, 15, 52, 35);
		savimp = savimage(A_DISPLAY, A_DRAWABLE);

		dclear(A_DISPLAY, A_DRAWABLE);
		setcapstyle(A_DISPLAY, A_GC, CapButt);
		drawline(15, 15, 52, 35);

		if (compsavimage(A_DISPLAY, A_DRAWABLE, savimp))
			CHECK;
		else {
			report("CapRound not equivalent to CapButt for thin line");
			FAIL;
		}

	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When the
.M cap_style
is
.S CapRound
and
.M line-width
is not zero,
then the line has a circular arc, with the diamater equal to the
.M line_width ,
centred on the endpoint.
>>STRATEGY
For a variety of line widths.
  Draw line with CapRound.
  Pixmap verify.
>>CODE
XVisualInfo	*vp;
int 	i;
static 	unsigned int 	capwidths[] = {
	1, 2, 4, 5, 10, 11, 40,41};

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setcapstyle(A_DISPLAY, A_GC, CapRound);

		for (i = 0; i < NELEM(capwidths); i++) {
			trace("Trying CapRound with width of %d", capwidths[i]);
			setwidth(A_DISPLAY, A_GC, capwidths[i]);
			drawline(CAP_X1, CAP_Y1, CAP_X2, CAP_Y2);

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(NELEM(capwidths)*nvinf());
>>ASSERTION Good A
When the
.M cap_style
is
.S CapProjecting
and
.M line-width
is zero,
then this is equivalent to 
.S CapButt .
>>STRATEGY
Draw line with CapProjecting.
Save the image on the drawable.
Draw line with CapButt.
Verify that the images drawn were the same.
>>CODE
XVisualInfo	*vp;
XImage	*savimp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);
		setcapstyle(A_DISPLAY, A_GC, CapProjecting);

		drawline(CAP_X1, CAP_Y1, CAP_X2, CAP_Y2);
		savimp = savimage(A_DISPLAY, A_DRAWABLE);

		dclear(A_DISPLAY, A_DRAWABLE);
		setcapstyle(A_DISPLAY, A_GC, CapButt);
		drawline(CAP_X1, CAP_Y1, CAP_X2, CAP_Y2);

		if (compsavimage(A_DISPLAY, A_DRAWABLE, savimp))
			CHECK;
		else {
			report("CapProjecting not equivalent to CapButt for thin line");
			FAIL;
		}
	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When the
.M cap_style
is
.S CapProjecting
and
.M line-width
is not zero,
then the line is square at the end, but the path continues beyond the endpoint 
for a distance equal to half the
.M line-width .
>>STRATEGY
Draw horizontal line.
Verify directly that path continues beyond end points.
Draw arbitrary line.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
struct	area	area;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setcapstyle(A_DISPLAY, A_GC, CapProjecting);

		for(lw=11; lw <= 12; lw++) { /* odd and even linewidths */
			setwidth(A_DISPLAY, A_GC, lw);
			drawline(20, 20, 70, 20);

			setarea(&area, 20-lw/2, 20-lw/2, (70-20)+lw, lw);
			if (checkarea(A_DISPLAY, A_DRAWABLE, &area, W_FG, W_BG, CHECK_ALL))
				CHECK;
			else {
				report("CapProjecting on horizontal line failed (width = %u)", lw);
				FAIL;
			}

			dclear(A_DISPLAY, A_DRAWABLE);
			drawline(CAP_X1, CAP_Y1, CAP_X2, CAP_Y2);

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(4*nvinf());
>>ASSERTION Good A
>>#	added during review of assertions
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M cap_style
is applied to both endpoints and the
.M line_width
is equal to zero and the
.M cap_style
is
.S CapNotLast ,
then the results are device-dependent, 
but the desired effect is that nothing is drawn.
>>STRATEGY
Draw line with zero width and length.
Verify that nothing is drawn.
(Test always passes whatever the result.)
>>CODE
XVisualInfo	*vp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setcapstyle(A_DISPLAY, A_GC, CapNotLast);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);
		drawline(20, 20, 20, 20);

		if (checkarea(A_DISPLAY, A_DRAWABLE, (struct area *)0, W_BG, W_BG,
				CHECK_DIFFER|CHECK_ALL)) {

			trace("zero width, length line - Nothing was drawn (desired effect)");
		} else {
			trace("zero width, length line - Something was drawn (not the desired effect)");
		}

		CHECK;
	}

	CHECKPASS(nvinf());

>>#ASSERTION
>>#	suppressed during drafting of assertions.
>>#When a line has coincident endpoints (x1=x2, y1=y2), 
>>#and the
>>#.M cap_style
>>#is applied to both endpoints and the
>>#.M line_width
>>#is equal to zero and the
>>#.M cap_style
>>#is
>>#.S CapButt ,
>>#then the results are device-dependent, 
>>#but the desired effect is that a single pixel is drawn.
>>ASSERTION Good A
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M cap_style
is applied to both endpoints and the
.M line_width
is equal to zero and the
.M cap_style
is
.S CapRound ,
then the results are the same as for
.S CapButt
with
.M line_width
equal to zero.
>>STRATEGY
Draw zero length line with CapRound.
Save the image on the drawable.
Draw zero length line with CapButt.
Verify that the images drawn were the same.
>>CODE
XVisualInfo	*vp;
XImage	*savimp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);
		setcapstyle(A_DISPLAY, A_GC, CapRound);

		drawline(15, 15, 15, 15);
		savimp = savimage(A_DISPLAY, A_DRAWABLE);

		dclear(A_DISPLAY, A_DRAWABLE);
		setcapstyle(A_DISPLAY, A_GC, CapButt);
		drawline(15, 15, 15, 15);

		if (compsavimage(A_DISPLAY, A_DRAWABLE, savimp))
			CHECK;
		else {
			report("CapRound not equivalent to CapButt for zero length thin line");
			FAIL;
		}

	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M cap_style
is applied to both endpoints and the
.M line_width
is equal to zero and the
.M cap_style
is
.S CapProjecting ,
then the results are the same as for
.S CapButt
with
.M line_width
equal to zero.
>>STRATEGY
Draw zero length line with CapProjecting.
Save the image on the drawable.
Draw zero length line with CapButt.
Verify that the images drawn were the same.
>>CODE
XVisualInfo	*vp;
XImage	*savimp;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setwidth(A_DISPLAY, A_GC, (unsigned)0);
		setcapstyle(A_DISPLAY, A_GC, CapProjecting);

		drawline(15, 15, 15, 15);
		savimp = savimage(A_DISPLAY, A_DRAWABLE);

		dclear(A_DISPLAY, A_DRAWABLE);
		setcapstyle(A_DISPLAY, A_GC, CapButt);
		drawline(15, 15, 15, 15);

		if (compsavimage(A_DISPLAY, A_DRAWABLE, savimp))
			CHECK;
		else {
			report("CapProjecting not equivalent to CapButt for zero length thin line");
			FAIL;
		}

	}

	CHECKPASS(nvinf());
>>ASSERTION Good A
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M cap_style
is applied to both endpoints and the
.M line_width
is not equal to zero and the
.M cap_style
is
.S CapButt ,
then nothing is drawn.
>>STRATEGY
Draw zero length line with CapButt.
Verify that the drawable is clear.
>>CODE
XVisualInfo	*vp;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for(lw=10; lw <= 11; lw++) { /* odd and even widths */
			setwidth(A_DISPLAY, A_GC, lw);
			setcapstyle(A_DISPLAY, A_GC, CapButt);

			drawline(20, 20, 20, 20);

			if (checkarea(A_DISPLAY, A_DRAWABLE, (struct area *)0, W_BG, W_BG, CHECK_ALL))
				CHECK;
			else {
				report("Something was drawn with zero length line and CapButt (width = %u)", lw);
				FAIL;
			}
			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(2*nvinf());
>>ASSERTION Good A
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M cap_style
is applied to both endpoints and the
.M line_width
is not equal to zero and the
.M cap_style
is
.S CapRound ,
then the closed path is a circle, centered at the endpoint, and
with the diameter equal to the line-width.
>>STRATEGY
Draw zero length line with CapRound.
Pixmap verify.
>>CODE
XVisualInfo	*vp;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for(lw=4; lw <= 5; lw++) { /* odd and even line widths */
			setwidth(A_DISPLAY, A_GC, lw);
			setcapstyle(A_DISPLAY, A_GC, CapRound);

			drawline(20, 20, 20, 20);

			PIXCHECK(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(2*nvinf());
>>ASSERTION Good A
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M cap_style
is applied to both endpoints and the
.M line_width
is not equal to zero and the
.M cap_style
is
.S CapProjecting ,
then the closed path is a square, aligned with the coordinate axes, centered at the
endpoint, and with the sides equal to the line-width.
>>STRATEGY
Draw zero length line with CapProjecting.
Verify area drawn directly.
>>CODE
XVisualInfo	*vp;
struct	area	area;
unsigned int	lw;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		for(lw=21; lw <= 22; lw++) { /* odd and even line widths */
			setwidth(A_DISPLAY, A_GC, lw);
			setcapstyle(A_DISPLAY, A_GC, CapProjecting);
			drawline(40, 40, 40, 40);

			setarea(&area, 40-lw/2, 40-lw/2, lw, lw);
			if (checkarea(A_DISPLAY, A_DRAWABLE, &area, W_FG, W_BG, CHECK_ALL))
				CHECK;
			else {
				report("CapProjecting with zero length line did not draw a square (width = %u)", lw);
				FAIL;
			}
			dclear(A_DISPLAY, A_DRAWABLE);
		}
	}

	CHECKPASS(2*nvinf());

>>ASSERTION def
>># The first test in dash-list.mc will check this, if checkable.
When a 
.M cap_style 
is applied at an end point, then the phase of the
dash pattern is reset to the 
.M dash_offset .
