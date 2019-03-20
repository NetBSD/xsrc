#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xutil.h> /* for XDestroyImage */
#include <pixman.h> /* for pixman blt functions */

#include "test.h"

static uint8_t clock_bits[] = {0x3C, 0x5E, 0xEF, 0xF7, 0x87, 0xFF, 0x7E, 0x3C};

/* https://bugs.freedesktop.org/show_bug.cgi?id=91499 */
static void draw_clock(struct test_display *t, Drawable d,
		       uint8_t alu, int x, int y, uint32_t fg, uint32_t bg)
{
	Pixmap pixmap;
	XGCValues val;
	GC gc;

	val.graphics_exposures = 0;
	val.function = alu;
	val.foreground = fg;
	val.background = fg;

	gc = XCreateGC(t->dpy, d,
		       GCGraphicsExposures | GCForeground | GCBackground | GCFunction,
		       &val);
	pixmap = XCreateBitmapFromData(t->dpy, d, (char *)clock_bits, 8, 8);

	XCopyPlane(t->dpy, pixmap, d, gc, 0, 0, 8, 8, x, y, 1);

	XFreePixmap(t->dpy, pixmap);
	XFreeGC(t->dpy, gc);
}

static void clear(struct test_display *dpy, struct test_target *tt)
{
	XRenderColor render_color = {0};
	XRenderFillRectangle(dpy->dpy, PictOpClear, tt->picture, &render_color,
			     0, 0, tt->width, tt->height);
}

static void clock_tests(struct test *t, int reps, int sets, enum target target)
{
	struct test_target out, ref;
	int r, s;

	printf("Testing clock (%s): ", test_target_name(target));
	fflush(stdout);

	test_target_create_render(&t->out, target, &out);
	clear(&t->out, &out);

	test_target_create_render(&t->ref, target, &ref);
	clear(&t->ref, &ref);

	for (s = 0; s < sets; s++) {
		for (r = 0; r < reps; r++) {
			int x = rand() % (out.width - 8);
			int y = rand() % (out.height - 8);
			uint8_t alu = rand() % (GXset + 1);
			uint32_t bg = rand();
			uint32_t fg = rand();

			draw_clock(&t->out, out.draw, alu, x, y, fg, bg);
			draw_clock(&t->ref, ref.draw, alu, x, y, fg, bg);
		}

		test_compare(t,
			     out.draw, out.format,
			     ref.draw, ref.format,
			     0, 0, out.width, out.height,
			     "");
	}

	printf("passed [%d iterations x %d]\n", reps, sets);

	test_target_destroy_render(&t->out, &out);
	test_target_destroy_render(&t->ref, &ref);
}

int main(int argc, char **argv)
{
	struct test test;
	int i;

	test_init(&test, argc, argv);

	for (i = 0; i <= DEFAULT_ITERATIONS; i++) {
		int reps = REPS(i), sets = SETS(i);
		enum target t;

		for (t = TARGET_FIRST; t <= TARGET_LAST; t++) {
			clock_tests(&test, reps, sets, t);
		}
	}

	return 0;
}
