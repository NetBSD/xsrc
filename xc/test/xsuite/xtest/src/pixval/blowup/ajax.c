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
 * $XConsortium: ajax.c,v 1.16 94/04/17 21:01:42 rws Exp $
 */

#include	"stdlib.h"
#include	"xtest.h"
#include	"stdio.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	<string.h>

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define	FGPIX	BlackPixel(disp, DefaultScreen(disp))
#define	BGPIX	WhitePixel(disp, DefaultScreen(disp))

int 	Zoom = 4;
int 	Width, Height;
int 	depth;

static	Pixmap	Shadepm = None;
static	Pixmap	Crosspm = None;
static  int LastZoom = -1;
static	int Colour = 0;
static	char *font_name = NULL;

int	readimage();

/* Is this an error or dat file */
int 	FileType;
#define	TYPE_ERROR	1
#define	TYPE_DATA	2

main(argc, argv)
int 	argc;
char	**argv;
{
Display	*disp;
Window	win;
GC		gc;
char	*ps;
char	*file = NULL;
char	*disp_name = NULL;
char	*prog_name = argv[0];
int	errs = 0;

	font_name = NULL;
	while (--argc > 0) {
		ps = *++argv;
		if (*ps++ == '-') {
			switch (*ps) {
			case 'c':
				Colour = 1;
				break;
			case 'z':
				if (argc-- <= 0 || (Zoom=atoi(*++argv)) <= 0)
					errs++;
				break;
			case 'd':
				if (argc-- > 0)
					disp_name = *++argv;
				else
					errs++;
				break;
			case 'f':
				if (argc-- > 0)
					font_name = *++argv;
				else
					errs++;
				break;
			default:
				printf("Bad flag (%c)\n", *ps);
				errs++;
				break;
			}
		} else {
			file = *argv;
			break;
		}
	}

	if (errs > 0) {
		printf("USAGE: %s [-z zoom_factor] [-d display] [-colour] [-f font] file(s)\n",
			prog_name);
		exit(EXIT_FAILURE);
	}
	if (file == NULL) {
		printf("No file name given\n");
		exit(EXIT_FAILURE);
	}

	if ((disp = XOpenDisplay(disp_name)) == NULL) {
		printf("Can't open display '%s'\n", XDisplayName(disp_name));
		exit(EXIT_FAILURE);
	}


	win = XCreateSimpleWindow(disp, DefaultRootWindow(disp),
		200, 0,
		(unsigned)1, (unsigned)1,
		(unsigned)1,
		(unsigned long)1, (unsigned long)0
		);
	XSelectInput(disp, win,
		StructureNotifyMask|ExposureMask|ButtonPressMask|KeyPressMask);

	gc = XCreateGC(disp, win, 0L, (XGCValues *)0);
	XSetForeground(disp, gc, FGPIX);
	XSetBackground(disp, gc, BGPIX);
	XSetFunction(disp, gc, GXcopy);
	LastZoom = -1; /* force setzoom to do something first time */
	setzoom(disp, win, gc);

	do {
		if (processfile(disp, win, gc, file) == -1)
			break;
		file = *++argv;
	} while (--argc > 0);
}

processfile(disp, win, gc, file)
Display	*disp;
Window	win;
GC		gc;
char	*file;
{
FILE	*fp;
int	ret;

	fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr, "Can't open %s\n", file);
		exit(EXIT_FAILURE);
	}

	/*
	 * TEMP XXX
	 * This should be from info in the file -- note this will not work
	 * if a file name of foo/Err is used
	 */
	if (*file == 'E')
		FileType = TYPE_ERROR;
	else
		FileType = TYPE_DATA;

	/* setzoom(disp, win, gc); */

	while ((ret=proc(disp, win, gc, fp)) == 1)
		;
	fclose(fp);
	return ret;
}

proc(disp, win, gc, fp)
Display	*disp;
Window	win;
GC		gc;
FILE	*fp;
{
static	int 	mapped = 0;
XImage	*images[2];
int	nim;
int	ret;

	nim = readimage(fp, images, disp);
	if (nim <= 0)
		return 0;

	if (Width && Height)
		XResizeWindow(disp, win, Width*Zoom, Height*Zoom);

	if (!mapped) {
	XEvent	event;
		mapped++;
		XMapWindow(disp, win);
		XWindowEvent(disp, win, ExposureMask, &event);
		while (XCheckMaskEvent(disp, StructureNotifyMask, &event))
			;
	} else {
		XClearWindow(disp, win);
	}
	ret = dispimage(disp, win, gc, images, nim) ? 1 : -1;
	while(nim-- > 0)
		XDestroyImage(images[nim]);
	return ret;
}


dispimage(disp, win, gc, images, nim)
Display *disp;
Window win;
GC		gc;
XImage	*images[2];
int	nim;
{
int 	w, h, size;
static	XImage	*lastim1, *lastim2;
XImage		*image;
void	repaint();

	/*
	 * Use the last image(s) if this one is NULL XXX
	 */
	if (images == NULL || nim <= 0) {
		if (lastim1 == NULL)
			return 0;
		image = lastim1;
		nim = 1;
	} else
		image = images[0];
	
	lastim1 = image;
	if (nim >= 2)
		lastim2 = images[1];

	XSetState(disp, gc, FGPIX^BGPIX, 0, GXxor, FGPIX|BGPIX);

	w = Width*Zoom;
	h = Height*Zoom;
	size = (Width < Height) ? Width : Height;
	size = (size < 250) ? 250 : size;
	return VBlowup(disp, win, gc, 0, 0, w, h, size, 2, None,
		image, (nim>=2)?images[1]:NULL,
		0, 0, W_BG, w/2, h/2, 1, Colour, repaint, &Zoom, font_name);
}

void repaint(disp, win, gc, image1, image2, compare_colour, evp, which)
Display		*disp;
Window		win;
GC		gc;
XImage		*image1, *image2;
int		compare_colour;
unsigned int	which;
{
	int	x,y;
	unsigned long pix1, pix2, mask, bg = W_BG;

	if (image2 == NULL)
		which = 0x1; /* "bad" one only */
	else if (which == 0x2) {/* "good" only */
		image1 = image2;
		image2 = NULL;
	}
	if (image1 == NULL) {
		printf("No image file to display\n");
		return;
	}
	if (which == 0x3 && image2 == NULL) {
		printf("expected to be given two image files\n");
		return;
	}
	XClearWindow(disp, win);
	setzoom(disp, win, gc);
	mask = DEPTHMASK(image1->depth) & DEPTHMASK( image2 ? image2->depth : 32 );

#ifdef DEBUG
printf("repaint: W=%d, H=%d\n",Width, Height);
printf("image1: w=%d, h=%d\n",image1->width, image1->height);
if(image2) printf("image2: w=%d, h=%d\n",image2->width, image2->height);
#endif /* DEBUG */
	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			pix2 = pix1 = XGetPixel(image1, x, y);
			if (image2 != NULL) {
				pix2 = XGetPixel(image2, x, y);
			}
			pix1 &= mask;
			pix2 &= mask;
			if (which == 0x3) { /* show both */
			/*** Need to keep colour table in file, really to
			     ensure different values for Black/WhitePixel
			     on server and pixval library don't confuse.
			***/
			/***	pix1 = (pix1 != bg);	***/
				if (pix1 == pix2) {
				    if (pix1)
					    XFillRectangle(disp, win, gc,
						x*Zoom, y*Zoom, Zoom, Zoom);
				} else if (pix1) {
#ifdef DEBUG
printf("Cross at (%d,%d), pix1=%lx, pix2=%lx\n",x,y,pix1,pix2);
#endif /* DEBUG */
					XCopyPlane(disp, Crosspm, win, gc, 0, 0,
						Zoom, Zoom, x*Zoom, y*Zoom, 1);
				} else {
#ifdef DEBUG
printf("Shade at (%d,%d), pix1=%lx, pix2=%lx\n",x,y,pix1,pix2);
#endif /* DEBUG */
					XCopyPlane(disp, Shadepm, win, gc, 0, 0,
						Zoom, Zoom, x*Zoom, y*Zoom, 1);
				}
			} else if ((pix1&0x1) != (bg&0x1)) {
				XFillRectangle(disp, win, gc, x*Zoom, y*Zoom, Zoom, Zoom);
			}
		}
	}
#ifdef DEBUG
printf("repaint: Done\n");
#endif /* DEBUG */
}

int
readimage(fp, images, disp)
FILE	*fp;
XImage	*images[2];
Display *disp;
{
char	buf[512];
unsigned long	*ip;
int	r1im();
XImage	*ximage;
int	i = 0;
int	scrn;
Visual	*visual;
unsigned int bitmap_pad; /* debugging only */

	images[0] = images[1] = NULL;

	do {
		if (fgets(buf, 512, fp) == NULL)
			return 0;
	} while (buf[0] == '!');

	if (sscanf(buf, "%d %d %d", &Width, &Height, &depth) < 3)
		return 0;

	scrn = DefaultScreen(disp); /* XXX */
	visual = DefaultVisual(disp, scrn);

	/* at least one to do... */
	ximage = XCreateImage(disp,visual,depth,ZPixmap,0,NULL,
		Width, Height, bitmap_pad=32/* 8/16/32 */,0); /** XXX **/
	if (ximage == NULL)
		return 0;
	ip = (unsigned long*)calloc(ximage->bytes_per_line*ximage->height, 1);
	if (ip == NULL) {
		XDestroyImage(ximage);
		printf("Out of memory\n");
		return 0;
	}
	ximage->data = (char *)ip;

	if (FileType == TYPE_ERROR) {
		/* actually two to do */
		if (!r1im(fp, ximage, 1)) {
			XDestroyImage(ximage);
			return 0;
		}
		images[i++] = ximage;
	
		/* now next one */
		do {
			if (fgets(buf, 512, fp) == NULL) {
				XDestroyImage(images[i-1]);
				return 0;
			}
		} while (buf[0] == '!');
		if (sscanf(buf, "%d %d %d", &Width, &Height, &depth) < 3) {
			XDestroyImage(images[i-1]);
			return 0;
		}
		ximage = XCreateImage(disp,visual,depth,ZPixmap,0,NULL,
			Width, Height, bitmap_pad=32/* 8/16/32 */,0); /** XXX **/
		if (ximage == NULL) {
			XDestroyImage(images[i-1]);
			return 0;
		}
		ip = (unsigned long*)calloc(ximage->bytes_per_line*ximage->height, 1);
		if (ip == NULL) {
			printf("Out of memory\n");
			XDestroyImage(images[i-1]);
			XDestroyImage(ximage);
			return 0;
		}
		ximage->data = (char *)ip;
		if (!r1im(fp, ximage, 2)) {
			XDestroyImage(images[i-1]);
			XDestroyImage(ximage);
			return 0;
		}
	} else {
		if (!r1im(fp, ximage, 3)) {
			XDestroyImage(ximage);
			return 0;
		}
	}
	images[i++] = ximage;
	return i;
}

int
r1im(fp, image, val)
FILE	*fp;
XImage	*image;
int 	val;
{
char	buf[512];
int 	count;
unsigned long 	pix;
int 	x, y;

	x = 0; y = 0;
	while (fgets(buf, 512, fp) != NULL) {
		if (strchr(buf, ',') != NULL) {
			sscanf(buf, "%x,%lx", &count, &pix);
		} else {
			count = 1;
			sscanf(buf, "%lx", &pix);
		}
		if (depth != 32)
			pix &= (1<<depth)-1;

		for (; count; count--) {
			if (pix) {
				XPutPixel(image, x, y, pix);
			}
			if (++x >=Width) {
				x = 0;
				y++;
			}
			if (y >= Height)
				return 1;
		}
	}
	printf("ERROR\n");
	return 0;
}

setzoom(disp, win, gc)
Display	*disp;
Window	win;
GC	gc;
{
Pixmap	pm;
GC		gc1;
GC		gc2;
int 	i, j;

	if (LastZoom == Zoom)
		return;
	LastZoom = Zoom;
	pm = XCreatePixmap(disp, win, Zoom*2, Zoom*2, DefaultDepth(disp, DefaultScreen(disp)));
	if (Crosspm != None)
		XFreePixmap(disp, Crosspm);
	Crosspm = XCreatePixmap(disp, win, Zoom, Zoom, 1);
	if (Shadepm != None)
		XFreePixmap(disp, Shadepm);
	Shadepm = XCreatePixmap(disp, win, Zoom, Zoom, 1);

	gc1 = XCreateGC(disp, Crosspm, 0L, (XGCValues *)0);
	gc2 = XCreateGC(disp, DefaultRootWindow(disp), 0L, (XGCValues *)0);

	XSetForeground(disp, gc2, BGPIX);
	XFillRectangle(disp, pm, gc2, 0, 0, Zoom*2+1, Zoom*2+1);
	XSetForeground(disp, gc2, FGPIX);

	XSetForeground(disp, gc1, (unsigned long)0);
	XFillRectangle(disp, Crosspm, gc1, 0, 0, Zoom+1, Zoom+1);
	XFillRectangle(disp, Shadepm, gc1, 0, 0, Zoom+1, Zoom+1);

	XSetForeground(disp, gc1, (unsigned long)1);
	XSetBackground(disp, gc1, (unsigned long)0);

	XSetDashes(disp, gc2, 0, "\1", 1);
	XSetLineAttributes(disp, gc2, 0, LineOnOffDash, CapButt, JoinRound);
	XSetFunction(disp, gc2, GXcopy);

	XDrawLine(disp, pm, gc2, 0, 0, 0, Zoom*2);
	XDrawLine(disp, pm, gc2, Zoom, 0, Zoom, Zoom*2);
	XDrawLine(disp, pm, gc2, 0, 0, Zoom*2, 0);
	XDrawLine(disp, pm, gc2, 0, Zoom, Zoom*2, Zoom);

	XSetWindowBackgroundPixmap(disp, win, pm);
	XFreePixmap(disp, pm);

	for (i = 0; i < Zoom; i++) {
		XDrawPoint(disp, Crosspm, gc1, i, i);
		XDrawPoint(disp, Crosspm, gc1, i, Zoom-i);
	}
	for (i = 0; i < Zoom; i++) {
		for (j = 0; j < Zoom; j++) {
			if ((i-j) & 1) {
				XDrawPoint(disp, Shadepm, gc1, i, j);
			}
		}
	}
	XFreeGC(disp, gc1);
	XFreeGC(disp, gc2);

	if (Width && Height)
		XResizeWindow(disp, win, Width*Zoom, Height*Zoom);
}

