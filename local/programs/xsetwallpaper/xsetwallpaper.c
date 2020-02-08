/* $NetBSD: xsetwallpaper.c,v 1.3 2020/02/08 20:29:30 jmcneill Exp $ */

/*-
 * Copyright (c) 2011 Jared D. McNeill <jmcneill@invisible.ca>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD: xsetwallpaper.c,v 1.3 2020/02/08 20:29:30 jmcneill Exp $");

#include <sys/endian.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "stbi.h"

#define DEFAULT_FILL_COLOR	"#000000"

static uint8_t *	resize_nn(const uint8_t *, int, int, int, int);

static void
usage(const char *pn)
{
	fprintf(stderr, "usage: %s [-f fillcolor] [-s] filename\n", pn);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	const char *fill_color = DEFAULT_FILL_COLOR;
	const char *pn = argv[0];
	uint8_t *data;
	unsigned int root_width, root_height, root_border_width, root_depth;
	int root_x, root_y;
	int bitmap_pad, srcx, srcy, dstx, dsty;
	int imagew, imageh, imagebpp;
	int ch, i;
	int screen, default_depth, byte_order;
	int scale = 0;
	Display *display;
	Colormap colormap;
	XImage *image;
	Pixmap pixmap;
	XColor color;
	Window window;
	GC gc;

	while ((ch = getopt(argc, argv, "f:sh")) != -1) {
		switch (ch) {
		case 'f':
			fill_color = optarg;
			break;
		case 's':
			scale = 1;
			break;
		case 'h':
		default:
			usage(pn);
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;
			
	if (argc != 1)
		usage(pn);

	/* Load the image */
	data = stbi_load(argv[0], &imagew, &imageh, &imagebpp, 4);
	if (data == NULL)
		errx(EXIT_FAILURE, "failed to load %s", argv[0]);

	/* swap red and blue */
	for (i = 0; i < imagew * imageh * 4; i += 4) {
		uint8_t p;
		p = data[i + 0];
		data[i + 0] = data[i + 2];
		data[i + 2] = p;
	}

#if _BYTE_ORDER == _BIG_ENDIAN
	for (i = 0; i < imagew * imageh * 4; i += 4) {
		uint32_t *p = (uint32_t *)&data[i];
		*p = bswap32(*p);
	}
#endif

#ifdef DEBUG
	printf("%s: %dx%d %dbpp\n", argv[0], imagew, imageh, imagebpp * 8);
#endif

	/* open the display */
	display = XOpenDisplay(NULL);
	if (display == NULL) {
		errx(EXIT_FAILURE, "couldn't open display: %s",
		    getenv("DISPLAY"));
	}
	screen = DefaultScreen(display);
	default_depth = DefaultDepth(display, screen);
	colormap = DefaultColormap(display, 0);
	byte_order = ImageByteOrder(display);

	/* get root window geometry */
	if (!XGetGeometry(display, XDefaultRootWindow(display), &window,
	    &root_x, &root_y, &root_width, &root_height,
	    &root_border_width, &root_depth)) {
		errx(EXIT_FAILURE, "couldn't get screen dimensions\n");
	}

#ifdef DEBUG
	printf("screen is %dx%d\n", root_width, root_height);
#endif

	XSync(display, False);

	if (scale) {
		data = resize_nn(data, imagew, imageh, root_width, root_height);
		if (data == NULL)
			err(EXIT_FAILURE, "couldn't resize image\n");
		imagew = root_width;
		imageh = root_height;
	}

	/* Parse the fill colour and allocate it */
	if (!XParseColor(display, colormap, fill_color, &color)) {
		errx(EXIT_FAILURE, "couldn't parse color '%s'", fill_color);
	}
	if (!XAllocColor(display, colormap, &color)) {
		errx(EXIT_FAILURE, "XAllocColor failed");
	}

	/* Create an XImage from our raw image data */
	if (default_depth >= 24)
		bitmap_pad = 32;
	else if (default_depth >= 16)
		bitmap_pad = 16;
	else
		bitmap_pad = 8;
	image = XCreateImage(display, CopyFromParent, default_depth,
	    ZPixmap, 0, (char *)data, imagew, imageh, bitmap_pad, 0);
	if (image == NULL) {
		errx(EXIT_FAILURE, "XCreateImage failed");
	}
	XInitImage(image);
	image->byte_order = byte_order;

	/* Create a graphics context for our new pixmap */
	gc = XCreateGC(display, window, 0, NULL);

	/* Create a pixmap the size of the root window */
	pixmap = XCreatePixmap(display, window,
	    root_width, root_height, root_depth);

	/* Fill the background with the specified fill colour */
	XSetForeground(display, gc, color.pixel);
	XFillRectangle(display, pixmap, gc, 0, 0, root_width, root_height); 

	/* Copy the image to the pixmal, centering it on screen */
	if ((unsigned int)imagew > root_width) {
		srcx = (imagew - root_width) / 2;
		dstx = 0;
	} else {
		srcx = 0;
		dstx = (root_width - imagew) / 2;
	}
	if ((unsigned int)imageh > root_height) {
		srcy = (imageh - root_height) / 2;
		dsty = 0;
	} else {
		srcy = 0;
		dsty = (root_height - imageh) / 2;
	}
	XPutImage(display, pixmap, gc, image,
	    srcx, srcy, dstx, dsty,
	    root_width, root_height);

	/* Set the background pixmap for the window */
	XSetWindowBackgroundPixmap(display, window, pixmap);
	XClearWindow(display, window);

	/* Cleanup */
	XFreePixmap(display, pixmap);
	XCloseDisplay(display);

	return EXIT_SUCCESS;
}

static uint8_t *
resize_nn(const uint8_t *data, int src_w, int src_h, int dst_w, int dst_h)
{
	const uint32_t *src;
	uint32_t *dst;
	int src_x, src_y, dst_x, dst_y;

	src = (const uint32_t *)data;
	dst = malloc(src_w * src_h * 4);
	if (dst == NULL)
		return NULL;

	for (dst_y = 0; dst_y < dst_h; dst_y++) {
		src_y = dst_y * src_h / dst_h;
		for (dst_x = 0; dst_x < dst_w; dst_x++) {
			src_x = dst_x * src_w / dst_w;
			dst[dst_y * dst_w + dst_x] = src[src_y * src_w + src_x];
		}
	}

	return (uint8_t *)dst;
}
