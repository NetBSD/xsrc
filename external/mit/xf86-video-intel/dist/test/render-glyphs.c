#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include <X11/Xutil.h> /* for XDestroyImage */
#include <pixman.h> /* for pixman blt functions */

#include "test.h"

static const XRenderColor colors[] = {
	/* red, green, blue, alpha */
	{ 0 },
	{ 0, 0, 0, 0xffff },
	{ 0xffff, 0, 0, 0xffff },
	{ 0, 0xffff, 0, 0xffff },
	{ 0, 0, 0xffff, 0xffff },
	{ 0xffff, 0xffff, 0xffff, 0xffff },
};

static struct clip {
	void *func;
} clips[] = {
	{ NULL },
};

static int _x_error_occurred;

static int
_check_error_handler(Display     *display,
		     XErrorEvent *event)
{
	_x_error_occurred = 1;
	return False; /* ignored */
}

static void clear(struct test_display *dpy,
		  struct test_target *tt,
		  const XRenderColor *c)
{
	XRenderFillRectangle(dpy->dpy, PictOpClear, tt->picture, c,
			     0, 0, tt->width, tt->height);
}

static bool check_op(struct test_display *dpy, int op, struct test_target *tt)
{
	XRenderColor render_color = {0};

	XSync(dpy->dpy, True);
	_x_error_occurred = 0;

	XRenderFillRectangle(dpy->dpy, op,
			     tt->picture, &render_color,
			     0, 0, 0, 0);

	XSync(dpy->dpy, True);
	return _x_error_occurred == 0;
}

struct glyph_iter {
	enum {
		GLYPHS, OP, DST, SRC, MASK, CLIP,
	} stage;

	int glyph_format;
	int op;
	int dst_color;
	int src_color;
	int mask_format;
	int clip;

	struct {
		struct test_display *dpy;
		struct test_target tt;
		GlyphSet glyphset;
		Picture src;
		XRenderPictFormat *mask_format;
	} ref, out;
};

static void glyph_iter_init(struct glyph_iter *gi,
			    struct test *t, enum target target)
{
	memset(gi, 0, sizeof(*gi));

	gi->out.dpy = &t->out;
	test_target_create_render(&t->out, target, &gi->out.tt);

	gi->ref.dpy = &t->ref;
	test_target_create_render(&t->ref, target, &gi->ref.tt);

	gi->stage = GLYPHS;
	gi->glyph_format = -1;
	gi->op = -1;
	gi->dst_color = -1;
	gi->src_color = -1;
	gi->mask_format = -1;
	gi->clip = -1;
}

static void render_clear(char *image, int image_size, int bpp)
{
	memset(image, 0, image_size);
}

static void render_black(char *image, int image_size, int bpp)
{
	if (bpp == 4) {
		uint32_t *p = (uint32_t *)image;
		image_size /= 4;
		while (image_size--)
			*p++ = 0x000000ff;
	} else
		memset(image, 0x55, image_size);
}

static void render_green(char *image, int image_size, int bpp)
{
	if (bpp == 4) {
		uint32_t *p = (uint32_t *)image;
		image_size /= 4;
		while (image_size--)
			*p++ = 0xffff0000;
	} else
		memset(image, 0xaa, image_size);
}

static void render_white(char *image, int image_size, int bpp)
{
	memset(image, 0xff, image_size);
}

static GlyphSet create_glyphs(Display *dpy, int format_id)
{
#define N_GLYPHS 4
	XRenderPictFormat *format;
	XGlyphInfo glyph = { 8, 8, 0, 0, 8, 0 };
	char image[4*8*8];
	GlyphSet glyphset;
	Glyph gid;
	int image_size;
	int bpp;
	int n;

	format = XRenderFindStandardFormat(dpy, format_id);
	if (format == NULL)
		return 0;

	switch (format_id) {
	case PictStandardARGB32:
	case PictStandardRGB24:
		image_size = 4 * 8 * 8;
		bpp = 4;
		break;
	case PictStandardA8:
	case PictStandardA4:
		image_size = 8 * 8;
		bpp = 1;
		break;
	case PictStandardA1:
		image_size = 8;
		bpp = 0;
		break;
	default:
		return 0;
	}

	glyphset = XRenderCreateGlyphSet(dpy, format);
	for (n = 0; n < N_GLYPHS; n++) {
		gid = n;

		switch (n) {
		case 0: render_clear(image, image_size, bpp); break;
		case 1: render_black(image, image_size, bpp); break;
		case 2: render_green(image, image_size, bpp); break;
		case 3: render_white(image, image_size, bpp); break;
		}

		XRenderAddGlyphs(dpy, glyphset,
				 &gid, &glyph, 1, image, image_size);
	}

	return glyphset;
}

static const char *glyph_name(int n)
{
	switch (n) {
	case 0: return "clear";
	case 1: return "black";
	case 2: return "green";
	case 3: return "white";
	default: return "unknown";
	}
}

static bool glyph_iter_next(struct glyph_iter *gi)
{
restart:
	if (gi->stage == GLYPHS) {
		if (++gi->glyph_format == PictStandardNUM)
			return false;

		if (gi->out.glyphset)
			XRenderFreeGlyphSet(gi->out.dpy->dpy,
					    gi->out.glyphset);
		gi->out.glyphset = create_glyphs(gi->out.dpy->dpy,
					       gi->glyph_format);

		if (gi->ref.glyphset)
			XRenderFreeGlyphSet(gi->ref.dpy->dpy,
					    gi->ref.glyphset);
		gi->ref.glyphset = create_glyphs(gi->ref.dpy->dpy,
					       gi->glyph_format);

		gi->stage++;
	}

	if (gi->stage == OP) {
		do {
			if (++gi->op == 255)
				goto reset_op;
		} while (!check_op(gi->out.dpy, gi->op, &gi->out.tt) ||
			 !check_op(gi->ref.dpy, gi->op, &gi->ref.tt));

		gi->stage++;
	}

	if (gi->stage == DST) {
		if (++gi->dst_color == ARRAY_SIZE(colors))
			goto reset_dst;

		gi->stage++;
	}

	if (gi->stage == SRC) {
		if (++gi->src_color == ARRAY_SIZE(colors))
			goto reset_src;

		if (gi->ref.src)
			XRenderFreePicture(gi->ref.dpy->dpy, gi->ref.src);
		gi->ref.src = XRenderCreateSolidFill(gi->ref.dpy->dpy,
						     &colors[gi->src_color]);

		if (gi->out.src)
			XRenderFreePicture(gi->out.dpy->dpy, gi->out.src);
		gi->out.src = XRenderCreateSolidFill(gi->out.dpy->dpy,
						     &colors[gi->src_color]);

		gi->stage++;
	}

	if (gi->stage == MASK) {
		if (++gi->mask_format > PictStandardNUM)
			goto reset_mask;

		if (gi->mask_format == PictStandardRGB24)
			gi->mask_format++;

		if (gi->mask_format < PictStandardNUM) {
			gi->out.mask_format = XRenderFindStandardFormat(gi->out.dpy->dpy,
									gi->mask_format);
			gi->ref.mask_format = XRenderFindStandardFormat(gi->ref.dpy->dpy,
									gi->mask_format);
		} else {
			gi->out.mask_format = NULL;
			gi->ref.mask_format = NULL;
		}

		gi->stage++;
	}

	if (gi->stage == CLIP) {
		if (++gi->clip == ARRAY_SIZE(clips))
			goto reset_clip;

		gi->stage++;
	}

	gi->stage--;
	return true;

reset_op:
	gi->op = -1;
reset_dst:
	gi->dst_color = -1;
reset_src:
	gi->src_color = -1;
reset_mask:
	gi->mask_format = -1;
reset_clip:
	gi->clip = -1;
	gi->stage--;
	goto restart;
}

static void glyph_iter_fini(struct glyph_iter *gi)
{
	if (gi->out.glyphset)
		XRenderFreeGlyphSet (gi->out.dpy->dpy, gi->out.glyphset);
	if (gi->ref.glyphset)
		XRenderFreeGlyphSet (gi->ref.dpy->dpy, gi->ref.glyphset);

	test_target_destroy_render(gi->out.dpy, &gi->out.tt);
	test_target_destroy_render(gi->ref.dpy, &gi->ref.tt);
}

static const char *stdformat_to_str(int id)
{
	switch (id) {
	case PictStandardARGB32: return "ARGB32";
	case PictStandardRGB24: return "RGB24";
	case PictStandardA8: return "A8";
	case PictStandardA4: return "A4";
	case PictStandardA1: return "A1";
	default: return "none";
	}
}

static char *glyph_iter_to_string(struct glyph_iter *gi,
				  const char *format,
				  ...)
{
	static char buf[100];
	va_list ap;
	int len;

	len = sprintf(buf, "glyphs=%s, op=%d, dst=%08x, src=%08x, mask=%s",
		      stdformat_to_str(gi->glyph_format), gi->op,
		      xrender_color(&colors[gi->dst_color]),
		      xrender_color(&colors[gi->src_color]),
		      stdformat_to_str(gi->mask_format));

	if (format) {
		buf[len++] = ' ';
		va_start(ap, format);
		vsprintf(buf+len, format, ap);
		va_end(ap);
	}

	return buf;
}

static void single(struct test *t, enum target target)
{
	struct glyph_iter gi;
	int n;

	printf("Testing single glyph (%s): ", test_target_name(target));
	fflush(stdout);

	glyph_iter_init(&gi, t, target);
	while (glyph_iter_next(&gi)) {
		XGlyphElt8 elt;
		char id[N_GLYPHS];

		for (n = 0; n < N_GLYPHS; n++) {
			id[n] = n;

			elt.chars = &id[n];
			elt.nchars = 1;
			elt.xOff = 0;
			elt.yOff = 0;

			clear(gi.out.dpy, &gi.out.tt, &colors[gi.dst_color]);
			elt.glyphset = gi.out.glyphset;
			XRenderCompositeText8 (gi.out.dpy->dpy, gi.op,
					       gi.out.src,
					       gi.out.tt.picture,
					       gi.out.mask_format,
					       0, 0,
					       0, 8,
					       &elt, 1);

			clear(gi.ref.dpy, &gi.ref.tt, &colors[gi.dst_color]);
			elt.glyphset = gi.ref.glyphset;
			XRenderCompositeText8 (gi.ref.dpy->dpy, gi.op,
					       gi.ref.src,
					       gi.ref.tt.picture,
					       gi.ref.mask_format,
					       0, 0,
					       0, 8,
					       &elt, 1);
			test_compare(t,
				     gi.out.tt.draw, gi.out.tt.format,
				     gi.ref.tt.draw, gi.ref.tt.format,
				     0, 0, gi.out.tt.width, gi.out.tt.height,
				     glyph_iter_to_string(&gi,
							  "glyph=%s",
							  glyph_name(n)));
		}

		elt.chars = &id[0];
		elt.nchars = n;
		clear(gi.out.dpy, &gi.out.tt, &colors[gi.dst_color]);
		elt.glyphset = gi.out.glyphset;
		XRenderCompositeText8 (gi.out.dpy->dpy, gi.op,
				       gi.out.src,
				       gi.out.tt.picture,
				       gi.out.mask_format,
				       0, 0,
				       0, 8,
				       &elt, 1);

		clear(gi.ref.dpy, &gi.ref.tt, &colors[gi.dst_color]);
		elt.glyphset = gi.ref.glyphset;
		XRenderCompositeText8 (gi.ref.dpy->dpy, gi.op,
				       gi.ref.src,
				       gi.ref.tt.picture,
				       gi.ref.mask_format,
				       0, 0,
				       0, 8,
				       &elt, 1);
		test_compare(t,
			     gi.out.tt.draw, gi.out.tt.format,
			     gi.ref.tt.draw, gi.ref.tt.format,
			     0, 0, gi.out.tt.width, gi.out.tt.height,
			     glyph_iter_to_string(&gi, "all"));
	}
	glyph_iter_fini(&gi);
}

int main(int argc, char **argv)
{
	struct test test;
	int t;

	test_init(&test, argc, argv);
	XSetErrorHandler(_check_error_handler);

	for (t = TARGET_FIRST; t <= TARGET_LAST; t++) {
		single(&test, t);
		//overlapping(&test, t);
		//gap(&test, t);
		//mixed(&test, t);
	}

	return 0;
}
