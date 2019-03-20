/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

int main(int argc, char **argv)
{
	Display *dpy;
	XFixesCursorImage *cur;
	unsigned long *src; /* XXX deep sigh */
	unsigned x, y;
	png_struct *png;
	png_info *info;
	png_byte **rows;
	FILE *file;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		return 1;

	if (!XFixesQueryExtension(dpy, (int *)&x, (int *)&y))
		return 1;

	cur = XFixesGetCursorImage(dpy);
	if (cur == NULL)
		return 1;

	printf("Cursor on display '%s': %dx%d, (hotspot %dx%d)\n",
	       DisplayString(dpy),
	       cur->width, cur->height,
	       cur->xhot, cur->yhot);

	if (1) {
		int x, y;

		src = cur->pixels;
		for (y = 0; y < cur->height; y++) {
			for (x = 0; x < cur->width; x++) {
				if (x == cur->xhot && y == cur->yhot)
					printf("+");
				else
					printf("%c", *src ? *src >> 24 >= 127 ? 'x' : '.' : ' ');
				src++;
			}
			printf("\n");
		}
	}

	file = fopen("cursor.png", "wb");
	if (file == NULL)
		return 2;

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	png_init_io(png, file);
	png_set_IHDR(png, info,
		     cur->width, cur->height, 8,
		     PNG_COLOR_TYPE_RGB_ALPHA,
		     PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT,
		     PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	src = cur->pixels;
	rows = malloc(cur->height*sizeof(png_byte*));
	if (rows == NULL)
		return 3;

	for (y = 0; y < cur->height; y++) {
		rows[y] = malloc(cur->width * 4);
		for (x = 0; x < cur->width; x++) {
			uint32_t p = *src++;
			uint8_t r = p >> 0;
			uint8_t g = p >> 8;
			uint8_t b = p >> 16;
			uint8_t a = p >> 24;

			if (a > 0x00 && a < 0xff) {
				r = (r * 0xff + a /2) / a;
				g = (g * 0xff + a /2) / a;
				b = (b * 0xff + a /2) / a;
			}

			rows[y][4*x + 0] = b;
			rows[y][4*x + 1] = g;
			rows[y][4*x + 2] = r;
			rows[y][4*x + 3] = a;
		}
	}

	png_write_image(png, rows);
	png_write_end(png, NULL);
	fclose(file);

	return 0;
}
