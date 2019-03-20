#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.h"

enum edge {
	EDGE_SHARP = PolyEdgeSharp,
	EDGE_SMOOTH,
};

static void set_edge(Display *dpy, Picture p, enum edge edge)
{
	XRenderPictureAttributes a;

	a.poly_edge = edge;
	XRenderChangePicture(dpy, p, CPPolyEdge, &a);
}

static XRenderPictFormat *mask_format(Display *dpy, enum mask mask)
{
	switch (mask) {
	default:
	case MASK_NONE: return NULL;
	case MASK_A1: return XRenderFindStandardFormat(dpy, PictStandardA1);
	case MASK_A8: return XRenderFindStandardFormat(dpy, PictStandardA8);
	}
}

static const char *mask_name(enum mask mask)
{
	switch (mask) {
	default:
	case MASK_NONE: return "none";
	case MASK_A1: return "a1";
	case MASK_A8: return "a8";
	}
}

static const char *edge_name(enum edge edge)
{
	switch (edge) {
	default:
	case EDGE_SHARP: return "sharp";
	case EDGE_SMOOTH: return "smooth";
	}
}

static void clear(struct test_display *dpy, struct test_target *tt)
{
	XRenderColor render_color = {0};
	XRenderFillRectangle(dpy->dpy, PictOpClear, tt->picture, &render_color,
			     0, 0, tt->width, tt->height);
}

static void step_to_point(int step, int width, int height, XPointFixed *p)
{
	do {
		p->x = (step - 64) << 16;
		p->y = -64 << 16;

		step -= width - 128;
		if (step <= 0)
			return;

		p->x = (width + 64) << 16;
		p->y = (step - 64) << 16;
		step -= height - 128;

		if (step <= 0)
			return;

		p->x = (width + 64 - step) << 16;
		p->y = (height + 64) << 16;
		step -= width - 128;

		if (step <= 0)
			return;

		p->x = -64 << 16;
		p->y = (height + 64 - step) << 16;
		step -= height - 128;
	} while (step > 0);
}

static void edge_test(struct test *t,
		      enum mask mask,
		      enum edge edge,
		      enum target target)
{
	struct test_target out, ref;
	XRenderColor white = { 0xffff, 0xffff, 0xffff, 0xffff };
	Picture src_ref, src_out;
	XTriangle tri;
	unsigned step, max;

	test_target_create_render(&t->out, target, &out);
	set_edge(t->out.dpy, out.picture, edge);
	src_out = XRenderCreateSolidFill(t->out.dpy, &white);

	test_target_create_render(&t->ref, target, &ref);
	set_edge(t->ref.dpy, ref.picture, edge);
	src_ref = XRenderCreateSolidFill(t->ref.dpy, &white);

	printf("Testing edges (with mask %s and %s edges) (%s): ",
	       mask_name(mask),
	       edge_name(edge),
	       test_target_name(target));
	fflush(stdout);

	max = 2*(out.width + 128 + out.height+128);
	step = 0;
	for (step = 0; step <= max; step++) {
		char buf[80];

		step_to_point(step, out.width, out.height, &tri.p1);
		step_to_point(step + out.width + 128,
			      out.width, out.height,
			      &tri.p2);
		step_to_point(step + out.height + 128 + 2*(out.width + 128),
			      out.width, out.height,
			      &tri.p3);

		sprintf(buf,
			"tri=((%d, %d), (%d, %d), (%d, %d))\n",
			tri.p1.x >> 16, tri.p1.y >> 16,
			tri.p2.x >> 16, tri.p2.y >> 16,
			tri.p3.x >> 16, tri.p3.y >> 16);

		clear(&t->out, &out);
		XRenderCompositeTriangles(t->out.dpy,
					  PictOpSrc,
					  src_out,
					  out.picture,
					  mask_format(t->out.dpy, mask),
					  0, 0,
					  &tri, 1);

		clear(&t->ref, &ref);
		XRenderCompositeTriangles(t->ref.dpy,
					  PictOpSrc,
					  src_ref,
					  ref.picture,
					  mask_format(t->ref.dpy, mask),
					  0, 0,
					  &tri, 1);

		test_compare(t,
			     out.draw, out.format,
			     ref.draw, ref.format,
			     0, 0, out.width, out.height,
			     buf);
	}

	XRenderFreePicture(t->out.dpy, src_out);
	test_target_destroy_render(&t->out, &out);

	XRenderFreePicture(t->ref.dpy, src_ref);
	test_target_destroy_render(&t->ref, &ref);

	printf("pass\n");
}

int main(int argc, char **argv)
{
	struct test test;
	enum target target;
	enum mask mask;
	enum edge edge;

	test_init(&test, argc, argv);

	for (target = TARGET_FIRST; target <= TARGET_LAST; target++) {
		for (mask = MASK_NONE; mask <= MASK_A8; mask++)
			for (edge = EDGE_SHARP; edge <= EDGE_SMOOTH; edge++)
				edge_test(&test, mask, edge, target);
	}

	return 0;
}
