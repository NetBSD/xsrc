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
 * $XConsortium: join-styl.mc,v 1.9 94/04/17 21:14:45 rws Exp $
 */
>>EXTERN

/*
 * List of data for joins
 */
static	struct linejoindata {
	int 	a;
	int 	b;
	int 	c;
	double	ang;
} linejoindata[] = {
	{60, 10, 10, 18.9},
	{30, 20, 40, 86.82},
	{45, 0, 30,  33.69},
	{51, 5, 5,   11.20},
	{50, 20, 30, 52.77},

	{60, 5, 5,    9.53},
	{20, -50, 60, 3.37},
	{50, -50, 60, 5.19},
	{60, 3, 3,    5.73},
	{60, 2, 2,    3.82},
	{52, 5, 5,    10.98},
};
	
#if T_XDrawArcs

/*
 * The code for the test purposes knows that the first two arcs 
 * meet at 0 deg and the * next two meet at 90deg (add any others at the end).
 */
XArc	arcjoindata[] = {
	{0, 10, 30, 30, DEG(90), DEG(-180)},
	{-5, 40, 40, 40, DEG(90), DEG(-180)},
	{20, 10, 40, 40, DEG(90), DEG(-90)},
	{40, 30, 40, 40, DEG(90), DEG(-90)},
	{60, 50, 40, 40, DEG(90), DEG(90)},
};
#endif

>>ASSERTION Good A
When the
.M join_style
is
.S JoinMiter
and the angle at which the lines join is more than or equal to 11 degrees,
then the outer edges of two lines extend to meet at an angle.
>>STRATEGY
Set GC component join_style to JoinMitre.
Set GC component line_width to 7.
Do graphics operation (with line joins all at angles >= 11 degrees). 
Pixmap verify.
For XDrawLines, in addition repeat with line joins at a
	variety of angles >= 11 degrees.
>>CODE
XVisualInfo	*vp;
#if T_XDrawLines
struct	linejoindata	*jp;
XPoint	pnts[6];
int 	xoffset;
#endif
int 	ndrawn;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		setargs();
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setjoinstyle(A_DISPLAY, A_GC, JoinMiter);
		setwidth(A_DISPLAY, A_GC, (unsigned)7);

		ndrawn = 0;
#if !T_XDrawArcs

		/*
		 * The default paths join at greater than 11 deg - so try them
		 * out.
		 */
		trace("Trying default path");
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
		ndrawn++;
#endif

#if T_XDrawLines
		xoffset = 30;

		for (jp = linejoindata; jp < &linejoindata[NELEM(linejoindata)]; jp++) {
			if (jp->ang < 11.0)
				continue;

			ndrawn++;
			trace("Trying angle of about %.2f", jp->ang);

#define	YOFF 10
			pnts[0].x = 0;               pnts[0].y = 8;
			pnts[1].x = xoffset - jp->b; pnts[1].y = YOFF;
			pnts[2].x = xoffset;         pnts[2].y = jp->a+YOFF;
			pnts[3].x = xoffset + jp->c; pnts[3].y = YOFF;
			pnts[4].x = 90;              pnts[4].y = 20;
#undef YOFF

			points = pnts;
			npoints = 5;

			dclear(A_DISPLAY, A_DRAWABLE);
			XCALL;
			xoffset += 3;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
		}
#endif
#if T_XDrawArcs
		arcs = &arcjoindata[2];
		narcs = NELEM(arcjoindata)-2;
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
		ndrawn++;
#endif
	}

	CHECKPASS(ndrawn * nvinf());

>>ASSERTION Good A
When the
.M join_style
is
.S JoinMiter
and the angle at which the lines join is less than 11 degrees,
then a
.S JoinBevel
join-style is used.
>>STRATEGY
Set GC component join_style to JoinMitre.
Set GC component line_width to 7.
Do graphics operation (with line joins all at angles < 11 degrees). 
Pixmap verify.
>>CODE
XVisualInfo	*vp;
#if T_XDrawLines
struct	linejoindata	*jp;
XPoint	pnts[3];
int 	xoffset;
#endif
int 	ndrawn;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setjoinstyle(A_DISPLAY, A_GC, JoinMiter);
		setwidth(A_DISPLAY, A_GC, (unsigned)7);

		ndrawn = 0;
#if T_XDrawLines
		xoffset = 30;

		for (jp = linejoindata; jp < &linejoindata[NELEM(linejoindata)]; jp++) {
			if (jp->ang > 11.0)
				continue;

			ndrawn++;
			trace("Trying angle of about %.2f", jp->ang);

			pnts[0].x = xoffset - jp->b; pnts[0].y = 0;
			pnts[1].x = xoffset; pnts[1].y = jp->a;
			pnts[2].x = xoffset + jp->c; pnts[2].y = 0;

			points = pnts;
			npoints = 3;

			dclear(A_DISPLAY, A_DRAWABLE);
			XCALL;
			xoffset += 3;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
		}
#endif
#if T_XDrawRectangle
		/*
		 * In a sense, height of a rectangle is zero then the
		 * lines join at 0 deg so this assertion applies.
		 * Probably a Grey Area though.
		 */
		height = 0;
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
		ndrawn++;
#endif
#if T_XDrawRectangles
		{
		XRectangle rect;

			/*
			 * In a sense, height of a rectangle is zero then the
			 * lines join at 0 deg so this assertion applies.
			 * Probably a Grey Area though.
			 */
			rect.x = 15; rect.y = 15;
			rect.width = 70;
			rect.height = 0;
			rectangles = &rect;
			nrectangles = 1;

			XCALL;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
			ndrawn++;
		}
#endif
#if T_XDrawArcs
		/* The first pair of arcs join at 0 deg. */
		arcs = arcjoindata;
		narcs = 2;
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
		ndrawn++;
#endif
	}

	CHECKPASS(ndrawn * nvinf());

>>ASSERTION Good A
When the
.M join_style
is
.S JoinRound ,
then the corner is a circular arc with the diameter equal to the line-width, 
centered on the joinpoint.
>>STRATEGY
Set GC component join_style to JoinRound.
Set GC component line_width to 7.
Do graphics operation.
Pixmap verify.
For XDrawLines, in addition repeat with line joins at a variety of angles.
>>CODE
XVisualInfo	*vp;
#if T_XDrawLines
struct	linejoindata	*jp;
XPoint	pnts[3];
int 	xoffset;
#endif

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		setargs();
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setjoinstyle(A_DISPLAY, A_GC, JoinRound);
		setwidth(A_DISPLAY, A_GC, (unsigned)7);

#if !T_XDrawArcs
		/*
		 * Try the default path.
		 */
		trace("Draw the default path");
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
#endif

#if T_XDrawArcs
		arcs = arcjoindata;
		narcs = NELEM(arcjoindata);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
#endif

#if T_XDrawLines
		xoffset = 30;
		for (jp = linejoindata; jp < &linejoindata[NELEM(linejoindata)]; jp++) {

			trace("Trying angle of about %.2f", jp->ang);

			dclear(A_DISPLAY, A_DRAWABLE);

			pnts[0].x = xoffset - jp->b; pnts[0].y = 0;
			pnts[1].x = xoffset; pnts[1].y = jp->a;
			pnts[2].x = xoffset + jp->c; pnts[2].y = 0;

			points = pnts;
			npoints = 3;

			XCALL;
			xoffset += 3;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
		}
#endif
	}

#if T_XDrawLines
	CHECKPASS((1 + NELEM(linejoindata)) * nvinf());
#else
	CHECKPASS(nvinf());
#endif

>>ASSERTION Good A
When the
.M join_style
is
.S JoinBevel ,
then the corner has
.S CapButt 
endpoint styles with the triangular notch filled.
>>STRATEGY
Set GC component join_style to JoinBevel.
Set GC component line_width to 7.
Do graphics operation.
Pixmap verify.
For XDrawLines, in addition repeat with line joins at a variety of angles.
>>CODE
XVisualInfo	*vp;
#if T_XDrawLines
struct	linejoindata	*jp;
XPoint	pnts[3];
int 	xoffset;
#endif

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		setargs();
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setjoinstyle(A_DISPLAY, A_GC, JoinBevel);
		setwidth(A_DISPLAY, A_GC, (unsigned)7);

#if T_XDrawArcs
		arcs = arcjoindata;
		narcs = NELEM(arcjoindata);
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
#else
		/*
		 * Try the default path.
		 */
		trace("Draw the default path");
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
#endif

#if T_XDrawLines
		xoffset = 30;
		for (jp = linejoindata; jp < &linejoindata[NELEM(linejoindata)]; jp++) {

			trace("Trying angle of about %.2f", jp->ang);

			dclear(A_DISPLAY, A_DRAWABLE);

			pnts[0].x = xoffset - jp->b; pnts[0].y = 0;
			pnts[1].x = xoffset; pnts[1].y = jp->a;
			pnts[2].x = xoffset + jp->c; pnts[2].y = 0;

			points = pnts;
			npoints = 3;

			XCALL;
			xoffset += 3;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
		}
#endif
	}

#if T_XDrawLines
	CHECKPASS((1 + NELEM(linejoindata)) * nvinf());
#else
	CHECKPASS(nvinf());
#endif

>>ASSERTION Good A
When a line has coincident endpoints (x1=x2, y1=y2), 
and the
.M join_style
is applied at one or both endpoints, 
and the path consists of more than just this line,
then the effect is as though the line was removed from the overall path.
>>STRATEGY
Set GC component join_style to JoinBevel.
Set GC component cap_style to CapRound.
Set GC component line_width to 7.
For XDrawRectangles and XDrawRectangle (GREY AREA):
	Do graphics operation.
	Pixmap verify.
For XDrawLines and XDrawArcs:
	Do graphics operation.
	Save image on the drawable.
	Do graphics operation with zero length segment joined to path.
	Verify image is the same as that saved.
>>CODE
XVisualInfo	*vp;
#if T_XDrawLines
XPoint	pnts[3];
#endif
XImage	*jssav;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setjoinstyle(A_DISPLAY, A_GC, JoinMiter);
		setcapstyle(A_DISPLAY, A_GC, CapRound);
		setwidth(A_DISPLAY, A_GC, (unsigned)7);

#if T_XDrawLines

		pnts[0].x = 10; pnts[0].y = 10;
		pnts[1].x = 80; pnts[1].y = 14;
		pnts[2].x = 80; pnts[2].y = 14;

		points = pnts;
		npoints = 3;

		XCALL;

		jssav = savimage(A_DISPLAY, A_DRAWABLE);

		dclear(A_DISPLAY, A_DRAWABLE);
		npoints = 2;
		XCALL;

		if (compsavimage(A_DISPLAY, A_DRAWABLE, jssav))
			CHECK;
		else {
			report("Effect was not the same as removing zero length line");
			FAIL;
		}
#endif
#if T_XDrawRectangle
		/*
		 * Since CapStyle is not documented to affect Rectangles
		 * this is a Grey Area.
		 * MIT resolved that CapStyle does affect Rectangles 
		 * with zero width and height - still unclear on zero height.
		 * 					DPJ Cater - 12/3/91
		 */
		height = 0;
		XCALL;
		PIXCHECK(A_DISPLAY, A_DRAWABLE);
#endif
#if T_XDrawRectangles
		{
		XRectangle rect;

			/*
			 * Since CapStyle is not documented to affect Rectangles
			 * this is a Grey Area.
			 * MIT resolved that CapStyle does affect Rectangles 
			 * with zero width and height - still unclear on zero height.
			 * 					DPJ Cater - 12/3/91
			 */
			rect.x = 15; rect.y = 15;
			rect.width = 70;
			rect.height = 0;

			rectangles = &rect;
			nrectangles = 1;

			XCALL;
			PIXCHECK(A_DISPLAY, A_DRAWABLE);
		}
#endif
#if T_XDrawArcs
		{
		static XArc	jsarc[] = {
			{0, 10, 60, 40, DEG(90), DEG(-180)},
			{0, 50, 60, 40, DEG(90), DEG(0)},
		};

			arcs = jsarc;
			narcs = 2;

			XCALL;
			jssav = savimage(A_DISPLAY, A_DRAWABLE);

			dclear(A_DISPLAY, A_DRAWABLE);
			narcs = 1;	/* Remove Zero length arc */
			XCALL;

			if (compsavimage(A_DISPLAY, A_DRAWABLE, jssav))
				CHECK;
			else {
				report("Effect was not the same as removing zero length arc");
				FAIL;
			}
		}
#endif
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When the total path consists of, or is reduced to, a single point joined
with itself, then the effect is the same as applying the
.M cap_style
at both endpoints.
>>STRATEGY
Set GC component join_style to JoinMiter.
Set GC component cap_style to CapRound.
Set GC component line_width to 12.
Draw zero length line.
Save image on the drawable.
Draw path that is reduced to single point.
Verify image is the same as that saved.
(Note that the actual rendering of the cap-style is tested else where)
>>CODE
XVisualInfo	*vp;
#if T_XDrawLines
XPoint	pnts[3];
#endif
XImage	*jssav;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		A_DRAWABLE = makewin(A_DISPLAY, vp);
		A_GC = makegc(A_DISPLAY, A_DRAWABLE);

		setjoinstyle(A_DISPLAY, A_GC, JoinMiter);
		setcapstyle(A_DISPLAY, A_GC, CapRound);
		setwidth(A_DISPLAY, A_GC, (unsigned)12);

		XDrawLine(A_DISPLAY, A_DRAWABLE, A_GC, 30, 30, 30, 30);
		jssav = savimage(A_DISPLAY, A_DRAWABLE);
		dclear(A_DISPLAY, A_DRAWABLE);

#if T_XDrawLines

		pnts[0].x = 30; pnts[0].y = 30;
		pnts[1].x = 30; pnts[1].y = 30;
		pnts[2].x = 30; pnts[2].y = 30;

		points = pnts;
		npoints = 3;

		XCALL;

		if (compsavimage(A_DISPLAY, A_DRAWABLE, jssav))
			CHECK;
		else {
			report("Effect was not the same as cap-style applied to both end points");
			FAIL;
		}
#endif
#if T_XDrawRectangle
		/*
		 * Since CapStyle is not documented to affect Rectangles
		 * this is a Grey Area.
		 * MIT resolved that CapStyle does affect Rectangles 
		 * with zero width and height.
		 * DPJ Cater - 12/3/91
		 */
		x = 30; y = 30;
		height = 0;
		width  = 0;
		XCALL;
		if (compsavimage(A_DISPLAY, A_DRAWABLE, jssav))
			CHECK;
		else {
			report("Effect was not the same as cap-style applied to both end points");
			FAIL;
		}
#endif
#if T_XDrawRectangles
		{
		XRectangle rect;

			/*
			 * Since CapStyle is not documented to affect Rectangles
			 * this is a Grey Area.
			 * MIT resolved that CapStyle does affect Rectangles 
			 * with zero width and height.		
			 * DPJ Cater - 12/3/91
			 */
			rect.x = 30; rect.y = 30;
			rect.width = 0;
			rect.height = 0;

			rectangles = &rect;
			nrectangles = 1;

			XCALL;
			if (compsavimage(A_DISPLAY, A_DRAWABLE, jssav))
				CHECK;
			else {
				report("Effect was not the same as cap-style applied to both end points");
				FAIL;
			}
		}
#endif
#if T_XDrawArcs
		{
		static XArc	jsarc[] = {
			{0, 10, 60, 40, DEG(90), DEG(0)},
			{0, 50, 60, 40, DEG(90), DEG(0)},
		};

			arcs = jsarc;
			narcs = 2;

			XCALL;
			jssav = savimage(A_DISPLAY, A_DRAWABLE);

			narcs = 1;	/* Remove Zero length arc */
			XCALL;

			if (compsavimage(A_DISPLAY, A_DRAWABLE, jssav))
				CHECK;
			else {
				report("Effect was not the same as cap-style applied to both end points");
				FAIL;
			}
		}
#endif
	}

	CHECKPASS(nvinf());

>>ASSERTION def
>># The first test in dash-list.mc will check this, if checkable.
When path elements are combined with a 
.M join_style ,
then dashing is continuous.
