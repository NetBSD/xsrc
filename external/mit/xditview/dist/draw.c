/*
 *
Copyright (c) 1991  X Consortium

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
 */

/*
 * draw.c
 *
 * accept dvi function calls and translate to X
 */

/*
  Support for ditroff drawing commands added: lines, circles, ellipses,
  arcs and splines.  Splines are approximated as short lines by iterating
  a simple approximation algorithm.  This seems good enough for previewing.

  David Evans <dre@cs.nott.ac.uk>, 14th March, 1990
*/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "DviP.h"
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950
#endif

/*	the following are for use in the spline approximation algorithm */

typedef struct Point {
    double x;
    double y;
    struct Point *next;
} Point;

#define	ITERATIONS	10      /* iterations to approximate spline */

#define	midx(p,q)	((p->x + q->x) / 2)     /* mid x point on pq */
#define	midy(p,q)	((p->y + q->y) / 2)     /* mid y point on pq */

#define	length(p,q)	sqrt(((q->x - p->x)*(q->x - p->x)) \
			     + ((q->y - p->y)*(q->y - p->y))) /* length of pq */

static Point *spline = (Point *) NULL;  /* head of spline linked list */

static void     ApproxSpline(int n);
static void     DeletePoint(Point *p);
static void     DrawSplineSegments(DviWidget dw);
static int      GetSpline(const char *s);
static void     InsertPoint(Point *p, Point *q);
static void     LineApprox(Point *p1, Point *p2, Point *p3);
static Point *  MakePoint(double x, double y);

#ifdef USE_XFT
static void
DrawText(DviWidget dw)
{
    XftFont *font = dw->dvi.cache.font;

    for (int i = 0; i <= dw->dvi.cache.index; i++) {
        if (dw->dvi.cache.cache[i].font)
            font = dw->dvi.cache.cache[i].font;
        XftDrawString8(dw->dvi.draw, &dw->dvi.black,
                       font,
                       dw->dvi.cache.cache[i].x,
                       dw->dvi.cache.start_y,
                       (unsigned char *) dw->dvi.cache.cache[i].chars,
                       dw->dvi.cache.cache[i].nchars);
    }
}
#endif

void
FlushCharCache(DviWidget dw)
{
    int xx = ToX(dw, dw->dvi.state->x);
    int yx = ToX(dw, dw->dvi.state->y);

    if (dw->dvi.cache.char_index != 0) {
#ifdef USE_XFT
        DrawText(dw);
#else
        XDrawText(XtDisplay(dw), XtWindow(dw), dw->dvi.normal_GC,
                  dw->dvi.cache.start_x, dw->dvi.cache.start_y,
                  dw->dvi.cache.cache, dw->dvi.cache.index + 1);
#endif
    }
    dw->dvi.cache.index = 0;
    dw->dvi.cache.max = DVI_TEXT_CACHE_SIZE;
    if (dw->dvi.noPolyText)
        dw->dvi.cache.max = 1;
    dw->dvi.cache.char_index = 0;
    dw->dvi.cache.cache[0].nchars = 0;
    dw->dvi.cache.start_x = dw->dvi.cache.x = xx;
    dw->dvi.cache.start_y = dw->dvi.cache.y = yx;
}

void
SetGCForDraw(DviWidget dw)
{
    if (dw->dvi.state->line_style != dw->dvi.line_style ||
        dw->dvi.state->line_width != dw->dvi.line_width) {
        int lw = ToX(dw, dw->dvi.state->line_width);

        if (lw <= 1)
            lw = 0;
        XSetLineAttributes(XtDisplay(dw), dw->dvi.normal_GC,
                           lw, LineSolid, CapButt, JoinMiter);
        dw->dvi.line_style = dw->dvi.state->line_style;
        dw->dvi.line_width = dw->dvi.state->line_width;
    }
}

void
DrawLine(DviWidget dw, int x, int y)
{
    if (dw->dvi.display_enable)
        XDrawLine(XtDisplay(dw), XtWindow(dw), dw->dvi.normal_GC,
                  ToX(dw, dw->dvi.state->x), ToX(dw, dw->dvi.state->y),
                  ToX(dw, dw->dvi.state->x + x), ToX(dw, dw->dvi.state->y + y));
    dw->dvi.state->x += x;
    dw->dvi.state->y += y;
}

void
DrawCircle(DviWidget dw, int diameter)
{
    if (dw->dvi.display_enable)
        XDrawArc(XtDisplay(dw), XtWindow(dw), dw->dvi.normal_GC,
                 ToX(dw, dw->dvi.state->x),
                 ToX(dw, dw->dvi.state->y - (diameter / 2)),
                 ToX(dw, diameter), ToX(dw, diameter), 0, 360 * 64);
    dw->dvi.state->x += diameter;
}

void
DrawEllipse(DviWidget dw, int a, int b)
{
    if (dw->dvi.display_enable)
        XDrawArc(XtDisplay(dw), XtWindow(dw), dw->dvi.normal_GC,
                 ToX(dw, dw->dvi.state->x), ToX(dw, dw->dvi.state->y - (b / 2)),
                 ToX(dw, a), ToX(dw, b), 0, 360 * 64);
    dw->dvi.state->x += a;
}

/*	Convert angle in degrees to 64ths of a degree */

static int
ConvertAngle(int theta)
{
    return (theta * 64);
}

void
DrawArc(DviWidget dw, int x0, int y0, int x1, int y1)
{
    /* centre */
    int xc = dw->dvi.state->x + x0;
    int yc = dw->dvi.state->y + y0;

    /* to */
    int x2 = xc + x1;
    int y2 = yc + y1;

    dw->dvi.state->x = x2;
    dw->dvi.state->y = y2;

    if (dw->dvi.display_enable) {
        int angle1, angle2;

        /* radius */
        int r = (int) sqrt((float) x1 * x1 + (float) y1 * y1);

        /* start and finish angles */
        if (x0 == 0) {
            if (y0 >= 0)
                angle1 = 90;
            else
                angle1 = 270;
        }
        else {
            angle1 = (int) (atan((double) (y0) / (double) (x0)) * 180 / M_PI);
            if (x0 > 0)
                angle1 = 180 - angle1;
            else
                angle1 = -angle1;
        }

        if (x1 == 0) {
            if (y1 <= 0)
                angle2 = 90;
            else
                angle2 = 270;
        }
        else {
            angle2 = (int) (atan((double) (y1) / (double) (x1)) * 180 / M_PI);
            if (x1 < 0)
                angle2 = 180 - angle2;
            else
                angle2 = -angle2;
        }

        if (angle1 < 0)
            angle1 += 360;
        if (angle2 < 0)
            angle2 += 360;

        if (angle2 < angle1)
            angle1 -= 360;
        angle2 = angle2 - angle1;

        angle1 = ConvertAngle(angle1);
        angle2 = ConvertAngle(angle2);

        XDrawArc(XtDisplay(dw), XtWindow(dw), dw->dvi.normal_GC,
                 ToX(dw, xc - r), ToX(dw, yc - r),
                 ToX(dw, 2 * r), ToX(dw, 2 * r), angle1, angle2);
    }
}

/* copy next non-blank string from p to temp, update p */

static const char *
getstr(const char *p, char *temp, size_t temp_size)
{
    while (*p == ' ' || *p == '\t' || *p == '\n')
        p++;
    if (*p == '\0') {
        temp[0] = 0;
        return ((char *) NULL);
    }
    while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0') {
        /* leave room for trailing NIL byte */
        if (temp_size > 1) {
            *temp++ = *p++;
            temp_size--;
        }
    }
    *temp = '\0';
    return (p);
}

/*	Draw a spline by approximating with short lines.      */

/*ARGSUSED*/
void
DrawSpline(DviWidget dw, const char *s, int len)
{
    int n;

    /* get coordinate pairs into spline linked list */
    if ((n = GetSpline(s)) <= 0)
        return;

    ApproxSpline(n);

    DrawSplineSegments(dw);
}


/*	Parse string s to create a linked list of Point's with spline */
/*	as its head.  Return the number of coordinate pairs found.    */

static int
GetSpline(const char *s)
{
    double x, y;
    int n = 0;
    Point *pt;
    const char *p = s;
    char d[10];

    if (!*p)
        return (n);

    pt = spline = MakePoint(0.0, 0.0);
    n = 1;
    x = y = 0.0;
    p = s;
    while (p && *p) {
        double x1, y1;

        if ((p = getstr(p, d, sizeof(d))) == (const char *) NULL)
            break;
        x1 = x + atof(d);
        if ((p = getstr(p, d, sizeof(d))) == (const char *) NULL)
            break;
        y1 = y + atof(d);
        pt->next = MakePoint(x1, y1);
        pt = pt->next;
        x = pt->x;
        y = pt->y;
        n++;
    }

    /* number of pairs of points */

    return (n);
}

/*	Approximate a spline by lines generated by iterations of the	  */
/*	approximation algorithm from the original n points in the spline. */

static void
ApproxSpline(int n)
{
    int mid;
    Point *p1, *p2, *p3, *p;

    if (n < 3)
        return;

    /* number of mid-points to calculate */
    mid = n - 3;

    /* remember original points are stored as an array of n points */
    /* so I can index it directly to calculate mid-points only.    */
    if (mid > 0) {
        int j;

        p = spline->next;
        j = 1;
        while (j < n - 2) {
            p1 = p;
            p = p->next;
            p2 = p;
            InsertPoint(p1, MakePoint(midx(p1, p2), midy(p1, p2)));
            j++;
        }
    }

    /* Now approximate curve by line segments.             */
    /* There *should* be the correct number of points now! */

    p = spline;
    while (p != (Point *) NULL) {
        p1 = p;
        if ((p = p->next) == (Point *) NULL)
            break;
        p2 = p;
        if ((p = p->next) == (Point *) NULL)
            break;
        p3 = p;         /* This point becomes first point of next curve */

        LineApprox(p1, p2, p3);
    }
}

/*	p1, p2, and p3 are initially 3 *consecutive* points on the curve. */
/*	For each adjacent pair of points find the mid-point, insert this  */
/*	in the linked list, delete the first of the two used (unless it   */
/*	is the first for this curve).  Repeat this ITERATIONS times.	  */

/*ARGSUSED*/
static void
LineApprox(Point *p1, Point *p2, Point *p3)
{
    int reps = ITERATIONS;

    while (reps) {
        for (Point *p = p1; p != (Point *) NULL && p != p3;) {
            Point *p4 = MakePoint(midx(p, p->next), midy(p, p->next));

            InsertPoint(p, p4);
            if (p != p1)
                DeletePoint(p);
            p = p4->next;       /* skip inserted point! */
        }
        reps--;
    }
}


/*	Traverse the linked list, calling DrawLine to approximate the */
/*	spline curve.  Rounding errors are taken into account so that */
/*	the "curve" is continuous, and ends up where expected.	      */

static void
DrawSplineSegments(DviWidget dw)
{
    Point *p = spline;

    /* save the start position */
    double xpos = dw->dvi.state->x;
    double ypos = dw->dvi.state->y;

    double x1 = 0.0, y1 = 0.0;

    while (p != (Point *) NULL) {
        int dx = p->x - x1 + 0.5;
        int dy = p->y - y1 + 0.5;

        DrawLine(dw, dx, dy);

        x1 = p->x;
        y1 = p->y;
        dw->dvi.state->x = xpos + x1;
        dw->dvi.state->y = ypos + y1;

        Point *q = p;

        p = p->next;
        XtFree((char *) q);
    }
    spline = (Point *) NULL;
}

/*	Malloc memory for a Point, and initialise the elements to x, y, NULL */
/*	Return a pointer to the new Point.				     */

static Point *
MakePoint(double x, double y)
{
    Point *p = (Point *) XtMalloc(sizeof(Point));

    p->x = x;
    p->y = y;
    p->next = (Point *) NULL;

    return (p);
}

/*	Insert point q in linked list after point p. */

static void
InsertPoint(Point *p, Point *q)
{
    /* point q to the next point */
    q->next = p->next;

    /* point p to new inserted one */
    p->next = q;
}

/*	Delete point p from the linked list. */

static void
DeletePoint(Point *p)
{
    Point *tmp = p->next;

    p->x = p->next->x;
    p->y = p->next->y;
    p->next = p->next->next;
    XtFree((char *) tmp);
}
