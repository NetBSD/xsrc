#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xutil.h> /* for XDestroyImage */
#include <pixman.h> /* for pixman blt functions */

#include "test.h"

static const XPoint points[]= {
	/* top */
	{ 0, 0},
	{ 1, 0},
	{ 2, 0},
	{ 3, 0},
	{ 4, 0},
	{ 5, 0},
	{ 6, 0},
	{ 7, 0},
	{ 8, 0},
	/* right */
	{ 8, 1},
	{ 8, 2},
	{ 8, 3},
	{ 8, 4},
	{ 8, 5},
	{ 8, 6},
	{ 8, 7},
	{ 8, 8},
	/* bottom */
	{ 7, 8},
	{ 6, 8},
	{ 5, 8},
	{ 4, 8},
	{ 3, 8},
	{ 2, 8},
	{ 1, 8},
	{ 0, 8},
	/* left */
	{ 0, 7},
	{ 0, 6},
	{ 0, 5},
	{ 0, 4},
	{ 0, 3},
	{ 0, 2},
	{ 0, 1},
	{ 0, 0} /* and origin again for luck */
};
#define NUM_POINTS (sizeof(points)/sizeof(points[0]))

static void clear(struct test_display *dpy, struct test_target *tt)
{
	XRenderColor render_color = {0};
	XRenderFillRectangle(dpy->dpy, PictOpClear, tt->picture, &render_color,
			     0, 0, tt->width, tt->height);
}

static void draw(struct test_display *dpy, struct test_target *tt,
		 int alu, int width, int style, int cap,
		 XSegment *seg, int nseg)
{
	XGCValues val;
	GC gc;

	val.function = alu;
	val.foreground = WhitePixel(dpy->dpy, 0);
	val.line_width = width;
	val.line_style = style;
	val.cap_style = cap;

	gc = XCreateGC(dpy->dpy, tt->draw,
		       GCForeground |
		       GCFunction |
		       GCLineWidth |
		       GCLineStyle |
		       GCCapStyle,
		       &val);
	XDrawSegments(dpy->dpy, tt->draw, gc, seg, nseg);
	XFreeGC(dpy->dpy, gc);
}

static void hv0(struct test *t, enum target target)
{
	char buf[1024];
	struct test_target out, ref;
	int a, alu, cap;
	XSegment seg[(NUM_POINTS+1)*8];
	int n, x, y, nseg;

	printf("Testing drawing of zero-width line segments (%s): ",
	       test_target_name(target));
	fflush(stdout);

	test_target_create_render(&t->out, target, &out);
	test_target_create_render(&t->ref, target, &ref);

	y = x = n = 0;
	for (a = 0; a <= NUM_POINTS; a++) {
		seg[n].x1 = a + 64;
		seg[n].y1 = y + 64;
		seg[n].x2 = NUM_POINTS + 64;
		seg[n].y2 = y + 64;
		n++; y++;

		seg[n].x1 = NUM_POINTS - a + 64;
		seg[n].y1 = y + 64;
		seg[n].x2 = 0 + 64;
		seg[n].y2 = y + 64;
		n++; y++;

		seg[n].x2 = a + 64;
		seg[n].y2 = y + 64;
		seg[n].x1 = NUM_POINTS + 64;
		seg[n].y1 = y + 64;
		n++; y++;

		seg[n].x2 = NUM_POINTS - a + 64;
		seg[n].y2 = y + 64;
		seg[n].x1 = 0 + 64;
		seg[n].y1 = y + 64;
		n++; y++;


		seg[n].y1 = a + 64;
		seg[n].x1 = x + 64;
		seg[n].y2 = NUM_POINTS + 64;
		seg[n].x2 = x + 64;
		n++; x++;

		seg[n].y1 = NUM_POINTS - a + 64;
		seg[n].x1 = x + 64;
		seg[n].y2 = 0 + 64;
		seg[n].x2 = x + 64;
		n++; x++;

		seg[n].y2 = a + 64;
		seg[n].x2 = x + 64;
		seg[n].y1 = NUM_POINTS + 64;
		seg[n].x1 = x + 64;
		n++; x++;

		seg[n].y2 = NUM_POINTS - a + 64;
		seg[n].x2 = x + 64;
		seg[n].y1 = 0 + 64;
		seg[n].x1 = x + 64;
		n++; x++;
	}

	for (alu = 0; alu < 16; alu++) {
		for (cap = CapNotLast; cap <= CapProjecting; cap++) {
			for (nseg = 0; nseg < n; nseg++) {
				sprintf(buf,
					"cap=%d, alu=%d, nseg=%d",
					cap, alu, nseg);

				clear(&t->out, &out);
				clear(&t->ref, &ref);

				draw(&t->out, &out, alu, 0, LineSolid, cap,
				     seg, nseg);
				draw(&t->ref, &ref, alu, 0, LineSolid, cap,
				     seg, nseg);

				test_compare(t,
					     out.draw, out.format,
					     ref.draw, ref.format,
					     0, 0, out.width, out.height,
					     buf);
			}
		}
	}

	test_target_destroy_render(&t->out, &out);
	test_target_destroy_render(&t->ref, &ref);

	printf("\n");
}

static void general(struct test *t, enum target target)
{
	char buf[1024];
	struct test_target out, ref;
	int a, b, alu, lw, style, cap;
	XSegment seg[NUM_POINTS*NUM_POINTS];
	int n = 0;

	printf("Testing drawing of general line segments (%s): ",
	       test_target_name(target));
	fflush(stdout);

	test_target_create_render(&t->out, target, &out);
	test_target_create_render(&t->ref, target, &ref);

	style = LineSolid;

	for (a = 0; a < NUM_POINTS; a++) {
		for (b = 0; b < NUM_POINTS; b++) {
			seg[n].x1 = points[a].x + 64;
			seg[n].y1 = points[a].y + 64;
			seg[n].x2 = points[b].x + 64;
			seg[n].y2 = points[b].y + 64;
			n++;
		}
	}

	for (alu = 0; alu < 16; alu++) {
		for (cap = CapNotLast; cap <= CapProjecting; cap++) {
			for (lw = 0; lw <= 4; lw++) {
				sprintf(buf,
					"width=%d, cap=%d, alu=%d",
					lw, cap, alu);

				clear(&t->out, &out);
				clear(&t->ref, &ref);

				draw(&t->out, &out, alu, lw, style, cap,
					  seg, n);
				draw(&t->ref, &ref, alu, lw, style, cap,
					  seg, n);

				test_compare(t,
					     out.draw, out.format,
					     ref.draw, ref.format,
					     0, 0, out.width, out.height,
					     buf);
			}
		}
	}

	test_target_destroy_render(&t->out, &out);
	test_target_destroy_render(&t->ref, &ref);

	printf("\n");
}

int main(int argc, char **argv)
{
	struct test test;
	enum target t;

	test_init(&test, argc, argv);

	for (t = TARGET_FIRST; t <= TARGET_LAST; t++) {
		hv0(&test, t);
		general(&test, t);
	}

	return 0;
}
