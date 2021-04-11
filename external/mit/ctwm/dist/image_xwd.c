/*
 * XWD image handling
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/XWDFile.h>

#include "screen.h"
#include "animate.h"

#include "image.h"
#include "image_xwd.h"


static Image *LoadXwdImage(const char *filename, ColorPair cp);
static void compress(XImage *image, XColor *colors, int *ncolors);
static void swapshort(char *bp, unsigned n);
static void swaplong(char *bp, unsigned n);



/*
 * External entry
 */
Image *
GetXwdImage(const char *name, ColorPair cp)
{
	/* Non-animated */
	if(! strchr(name, '%')) {
		return (LoadXwdImage(name, cp));
	}

	/* Animated */
	return get_image_anim_cp(name, cp, LoadXwdImage);
}


/*
 * Internal backend
 */
static Image *
LoadXwdImage(const char *filename, ColorPair cp)
{
	FILE        *file;
	char        *fullname;
	XColor      colors [256];
	XWDColor    xwdcolors [256];
	unsigned    buffer_size;
	XImage      *image;
	unsigned char *imagedata;
	Pixmap      pixret;
	Visual      *visual;
	char        win_name [256];
	int         win_name_size;
	int         ispipe;
	int         i, len;
	int         w, h, depth, ncolors;
	int         scrn;
	Colormap    cmap;
	Colormap    stdcmap = Scr->RootColormaps.cwins[0]->colormap->c;
	GC          gc;
	XGCValues   gcvalues;
	XWDFileHeader header;
	Image       *ret;
	unsigned long swaptest = 1;

	ispipe = 0;
	if(filename [0] == '|') {
		file = (FILE *) popen(filename + 1, "r");
		if(file == NULL) {
			return NULL;
		}
		ispipe = 1;
		if(AnimationActive) {
			StopAnimation();
		}
		goto file_opened;
	}
	fullname = ExpandPixmapPath(filename);
	if(! fullname) {
		return NULL;
	}
	file = fopen(fullname, "r");
	free(fullname);
	if(file == NULL) {
		if(reportfilenotfound) {
			fprintf(stderr, "unable to locate %s\n", filename);
		}
		return NULL;
	}
file_opened:
	len = fread((char *) &header, sizeof(header), 1, file);
	if(len != 1) {
		fprintf(stderr, "ctwm: cannot read %s\n", filename);
		return NULL;
	}
	if(*(char *) &swaptest) {
		swaplong((char *) &header, sizeof(header));
	}
	if(header.file_version != XWD_FILE_VERSION) {
		fprintf(stderr, "ctwm: XWD file format version mismatch : %s\n", filename);
		return NULL;
	}
	win_name_size = header.header_size - sizeof(header);
	len = fread(win_name, win_name_size, 1, file);
	if(len != 1) {
		fprintf(stderr, "file %s has not the correct format\n", filename);
		return NULL;
	}

	if(header.pixmap_format == XYPixmap) {
		fprintf(stderr, "ctwm: XYPixmap XWD file not supported : %s\n", filename);
		return NULL;
	}
	w       = header.pixmap_width;
	h       = header.pixmap_height;
	depth   = header.pixmap_depth;
	ncolors = header.ncolors;
	len = fread((char *) xwdcolors, sizeof(XWDColor), ncolors, file);
	if(len != ncolors) {
		fprintf(stderr, "file %s has not the correct format\n", filename);
		return NULL;
	}
	if(*(char *) &swaptest) {
		for(i = 0; i < ncolors; i++) {
			swaplong((char *) &xwdcolors [i].pixel, 4);
			swapshort((char *) &xwdcolors [i].red, 3 * 2);
		}
	}
	for(i = 0; i < ncolors; i++) {
		colors [i].pixel = xwdcolors [i].pixel;
		colors [i].red   = xwdcolors [i].red;
		colors [i].green = xwdcolors [i].green;
		colors [i].blue  = xwdcolors [i].blue;
		colors [i].flags = xwdcolors [i].flags;
		colors [i].pad   = xwdcolors [i].pad;
	}

	scrn    = Scr->screen;
	cmap    = AlternateCmap ? AlternateCmap : stdcmap;
	visual  = Scr->d_visual;
	gc      = DefaultGC(dpy, scrn);

	buffer_size = header.bytes_per_line * h;
	imagedata = malloc(buffer_size);
	if(! imagedata) {
		fprintf(stderr, "cannot allocate memory for image %s\n", filename);
		return NULL;
	}
	len = fread(imagedata, (int) buffer_size, 1, file);
	if(len != 1) {
		free(imagedata);
		fprintf(stderr, "file %s has not the correct format\n", filename);
		return NULL;
	}
	if(ispipe) {
		pclose(file);
	}
	else {
		fclose(file);
	}

	image = XCreateImage(dpy, visual,  depth, header.pixmap_format,
	                     0, (char *) imagedata, w, h,
	                     header.bitmap_pad, header.bytes_per_line);
	if(image == NULL) {
		free(imagedata);
		fprintf(stderr, "cannot create image for %s\n", filename);
		return NULL;
	}
	if(header.pixmap_format == ZPixmap) {
		compress(image, colors, &ncolors);
	}
	if(header.pixmap_format != XYBitmap) {
		for(i = 0; i < ncolors; i++) {
			XAllocColor(dpy, cmap, &(colors [i]));
		}
		for(i = 0; i < buffer_size; i++) {
			imagedata [i] = (unsigned char) colors [imagedata [i]].pixel;
		}
	}
	if(w > Scr->rootw) {
		w = Scr->rootw;
	}
	if(h > Scr->rooth) {
		h = Scr->rooth;
	}

	ret = AllocImage();
	if(! ret) {
		fprintf(stderr, "unable to allocate memory for image : %s\n", filename);
		free(image);
		free(imagedata);
		for(i = 0; i < ncolors; i++) {
			XFreeColors(dpy, cmap, &(colors [i].pixel), 1, 0L);
		}
		return NULL;
	}
	if(header.pixmap_format == XYBitmap) {
		gcvalues.foreground = cp.fore;
		gcvalues.background = cp.back;
		XChangeGC(dpy, gc, GCForeground | GCBackground, &gcvalues);
	}
	if((w > (Scr->rootw / 2)) || (h > (Scr->rooth / 2))) {
		int x, y;

		pixret = XCreatePixmap(dpy, Scr->Root, Scr->rootw,
		                       Scr->rooth, Scr->d_depth);
		x = (Scr->rootw  - w) / 2;
		y = (Scr->rooth - h) / 2;
		XFillRectangle(dpy, pixret, gc, 0, 0, Scr->rootw, Scr->rooth);
		XPutImage(dpy, pixret, gc, image, 0, 0, x, y, w, h);
		ret->width  = Scr->rootw;
		ret->height = Scr->rooth;
	}
	else {
		pixret = XCreatePixmap(dpy, Scr->Root, w, h, depth);
		XPutImage(dpy, pixret, gc, image, 0, 0, 0, 0, w, h);
		ret->width  = w;
		ret->height = h;
	}
	XDestroyImage(image);

	ret->pixmap = pixret;
	ret->mask   = None;
	ret->next   = NULL;
	return ret;
}


/*
 * Utils
 */
static void
compress(XImage *image, XColor *colors, int *ncolors)
{
	unsigned char ind  [256];
	unsigned int  used [256];
	int           i, j, size, nused;
	unsigned char color;
	XColor        newcolors [256];
	unsigned char *imagedata;

	for(i = 0; i < 256; i++) {
		used [i] = 0;
		ind  [i] = 0;
	}
	nused = 0;
	size  = image->bytes_per_line * image->height;
	imagedata = (unsigned char *) image->data;
	for(i = 0; i < size; i++) {
		if((i % image->bytes_per_line) > image->width) {
			continue;
		}
		color = imagedata [i];
		if(used [color] == 0) {
			for(j = 0; j < nused; j++) {
				if((colors [color].red   == newcolors [j].red)   &&
				                (colors [color].green == newcolors [j].green) &&
				                (colors [color].blue  == newcolors [j].blue)) {
					break;
				}
			}
			ind  [color] = j;
			used [color] = 1;
			if(j == nused) {
				newcolors [j].red   = colors [color].red;
				newcolors [j].green = colors [color].green;
				newcolors [j].blue  = colors [color].blue;
				nused++;
			}
		}
	}
	for(i = 0; i < size; i++) {
		imagedata [i] = ind [imagedata [i]];
	}
	for(i = 0; i < nused; i++) {
		colors [i] = newcolors [i];
	}
	*ncolors = nused;
}


static void
swapshort(char *bp, unsigned n)
{
	char c;
	char *ep = bp + n;

	while(bp < ep) {
		c = *bp;
		*bp = *(bp + 1);
		bp++;
		*bp++ = c;
	}
}


static void
swaplong(char *bp, unsigned n)
{
	char c;
	char *ep = bp + n;
	char *sp;

	while(bp < ep) {
		sp = bp + 3;
		c = *sp;
		*sp = *bp;
		*bp++ = c;
		sp = bp + 1;
		c = *sp;
		*sp = *bp;
		*bp++ = c;
		bp += 2;
	}
}
