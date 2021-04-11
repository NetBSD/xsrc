/*
 * JPEG image handling functions
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "screen.h"
#include "image.h"
#include "image_jpeg.h"

/* Bits needed for libjpeg and interaction */
#include <setjmp.h>
#include <jpeglib.h>
#include <jerror.h>

#include <X11/Xlib.h>


/* Various internal bits */
static Image *LoadJpegImage(const char *name);
static Image *LoadJpegImageCp(const char *name, ColorPair cp);
static void convert_for_16(int w, int x, int y, int r, int g, int b);
static void convert_for_32(int w, int x, int y, int r, int g, int b);
static void jpeg_error_exit(j_common_ptr cinfo);

struct jpeg_error {
	struct jpeg_error_mgr pub;
	sigjmp_buf setjmp_buffer;
};

typedef struct jpeg_error *jerr_ptr;

static uint16_t *buffer_16bpp;
static uint32_t *buffer_32bpp;


/*
 * External entry point
 */
Image *
GetJpegImage(const char *name)
{
	ColorPair dummy = {0};

	/* Non-animated */
	if(! strchr(name, '%')) {
		return (LoadJpegImage(name));
	}

	/* Animated */
	return get_image_anim_cp(name, dummy, LoadJpegImageCp);
}


/*
 * Internal backend func
 */

/* Trivial thunk for get_image_anim_cp() callback */
static Image *
LoadJpegImageCp(const char *name, ColorPair cp)
{
	return LoadJpegImage(name);
}

/* The actual loader */
static Image *
LoadJpegImage(const char *name)
{
	char   *fullname;
	XImage *ximage;
	FILE   *infile;
	Image  *image;
	Pixmap pixret;
	void (*store_data)(int w, int x, int y, int r, int g, int b);
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error jerr;
	JSAMPARRAY buffer;
	int width, height;
	int row_stride;
	int g, i, a;
	int bpix;
	GC  gc;

	fullname = ExpandPixmapPath(name);
	if(! fullname) {
		return NULL;
	}

	image = AllocImage();
	if(image == NULL) {
		free(fullname);
		return NULL;
	}

	if((infile = fopen(fullname, "rb")) == NULL) {
		if(reportfilenotfound) {
			fprintf(stderr, "unable to locate %s\n", fullname);
		}
		fflush(stdout);
		free(image);
		free(fullname);
		return NULL;
	}
	free(fullname);
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpeg_error_exit;

	if(sigsetjmp(jerr.setjmp_buffer, 1)) {
		jpeg_destroy_decompress(&cinfo);
		free(image);
		fclose(infile);
		return NULL;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, FALSE);
	cinfo.do_fancy_upsampling = FALSE;
	cinfo.do_block_smoothing = FALSE;
	jpeg_start_decompress(&cinfo);
	width  = cinfo.output_width;
	height = cinfo.output_height;

	if(Scr->d_depth == 16) {
		store_data = &convert_for_16;
		buffer_16bpp = malloc((width) * (height) * 2);
		ximage = XCreateImage(dpy, CopyFromParent, Scr->d_depth, ZPixmap, 0,
		                      (char *) buffer_16bpp, width, height, 16, width * 2);
	}
	else if(Scr->d_depth == 24 || Scr->d_depth == 32) {
		store_data = &convert_for_32;
		buffer_32bpp = malloc(width * height * 4);
		ximage = XCreateImage(dpy, CopyFromParent, Scr->d_depth, ZPixmap, 0,
		                      (char *) buffer_32bpp, width, height, 32, width * 4);
	}
	else {
		fprintf(stderr, "Image %s unsupported depth : %d\n", name, Scr->d_depth);
		free(image);
		fclose(infile);
		return NULL;
	}
	if(ximage == NULL) {
		fprintf(stderr, "cannot create image for %s\n", name);
		free(image);
		fclose(infile);
		return NULL;
	}
	g = 0;
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
	         ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

	bpix = cinfo.output_components;
	while(cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		a = 0;
		for(i = 0; i < bpix * cinfo.output_width; i += bpix) {
			(*store_data)(width, a, g, buffer[0][i],  buffer[0][i + 1], buffer[0][i + 2]);
			a++;
		}
		g++;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	gc = DefaultGC(dpy, Scr->screen);
	if((width > (Scr->rootw / 2)) || (height > (Scr->rooth / 2))) {
		int x, y;

		pixret = XCreatePixmap(dpy, Scr->Root, Scr->rootw, Scr->rooth, Scr->d_depth);
		x = (Scr->rootw  -  width) / 2;
		y = (Scr->rooth  - height) / 2;
		XFillRectangle(dpy, pixret, gc, 0, 0, Scr->rootw, Scr->rooth);
		XPutImage(dpy, pixret, gc, ximage, 0, 0, x, y, width, height);
		image->width  = Scr->rootw;
		image->height = Scr->rooth;
	}
	else {
		pixret = XCreatePixmap(dpy, Scr->Root, width, height, Scr->d_depth);
		XPutImage(dpy, pixret, gc, ximage, 0, 0, 0, 0, width, height);
		image->width  = width;
		image->height = height;
	}
	if(ximage) {
		XDestroyImage(ximage);
	}
	image->pixmap = pixret;

	return image;
}



/*
 * Utils
 */
static void
convert_for_16(int w, int x, int y, int r, int g, int b)
{
	buffer_16bpp [y * w + x] = ((r >> 3) << 11) + ((g >> 2) << 5) + (b >> 3);
}

static void
convert_for_32(int w, int x, int y, int r, int g, int b)
{
	buffer_32bpp [y * w + x] = ((r << 16) + (g << 8) + b) & 0xFFFFFFFF;
}

static void
jpeg_error_exit(j_common_ptr cinfo)
{
	jerr_ptr errmgr = (jerr_ptr) cinfo->err;
	cinfo->err->output_message(cinfo);
	siglongjmp(errmgr->setjmp_buffer, 1);
	return;
}
