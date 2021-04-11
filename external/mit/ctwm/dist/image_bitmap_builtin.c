/*
 * Builtin bitmap image generation/lookup.
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xmu/Drawing.h>

#include "screen.h"
#include "drawing.h"
#include "icons_builtin.h"

#include "image.h"
#include "image_bitmap_builtin.h"


/*
 * Firstly, the plain built-in titlebar symbols.  These are the ones
 * specified with names like ":resize".  For various reasons, these
 * currently return Pixmap's, unlike most of our other builtins that
 * generate Image's.  Possible cleanup candidate.
 */
#define DEF_BI_PPM(nm) Pixmap nm(unsigned int *widthp, unsigned int *heightp)
static DEF_BI_PPM(CreateXLogoPixmap);
static DEF_BI_PPM(CreateResizePixmap);
static DEF_BI_PPM(CreateQuestionPixmap);
static DEF_BI_PPM(CreateMenuPixmap);
static DEF_BI_PPM(CreateDotPixmap);



/*
 * Look up and return a ":something" (not a ":xpm:something").
 *
 * Names of the form :name refer to hardcoded images that are scaled to
 * look nice in title buttons.  Eventually, it would be nice to put in a
 * menu symbol as well....
 */
Pixmap
get_builtin_plain_pixmap(const char *name, unsigned int *widthp,
                         unsigned int *heightp)
{
	int i;
	static struct {
		char *name;
		DEF_BI_PPM((*proc));
	} pmtab[] = {
		/* Lookup table for our various default pixmaps */
		{ TBPM_DOT,         CreateDotPixmap },
		{ TBPM_ICONIFY,     CreateDotPixmap },
		{ TBPM_RESIZE,      CreateResizePixmap },
		{ TBPM_XLOGO,       CreateXLogoPixmap },
		{ TBPM_DELETE,      CreateXLogoPixmap },
		{ TBPM_MENU,        CreateMenuPixmap },
		{ TBPM_QUESTION,    CreateQuestionPixmap },
	};

	/* Seatbelts */
	if(!name || name[0] != ':') {
		return None;
	}
	if(!widthp || !heightp) {
		return None;
	}


	/* Find it */
	for(i = 0; i < (sizeof pmtab) / (sizeof pmtab[0]); i++) {
		if(strcasecmp(pmtab[i].name, name) == 0) {
			Pixmap pm = (*pmtab[i].proc)(widthp, heightp);
			if(pm == None) {
				fprintf(stderr, "%s:  unable to build bitmap \"%s\"\n",
				        ProgramName, name);
				return None;
			}
			return pm;
		}
	}

	/* Didn't find it */
	fprintf(stderr, "%s:  no such built-in bitmap \"%s\"\n",
	        ProgramName, name);
	return None;
}


/*
 * Individual generators for those plain pixmaps
 */
DEF_BI_PPM(CreateXLogoPixmap)
{
	int h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(h < 0) {
		h = 0;
	}

	*widthp = *heightp = (unsigned int) h;
	if(Scr->tbpm.xlogo == None) {
		GC gc, gcBack;

		Scr->tbpm.xlogo = XCreatePixmap(dpy, Scr->Root, h, h, 1);
		gc = XCreateGC(dpy, Scr->tbpm.xlogo, 0L, NULL);
		XSetForeground(dpy, gc, 0);
		XFillRectangle(dpy, Scr->tbpm.xlogo, gc, 0, 0, h, h);
		XSetForeground(dpy, gc, 1);
		gcBack = XCreateGC(dpy, Scr->tbpm.xlogo, 0L, NULL);
		XSetForeground(dpy, gcBack, 0);

		/*
		 * draw the logo large so that it gets as dense as possible; then white
		 * out the edges so that they look crisp
		 */
		XmuDrawLogo(dpy, Scr->tbpm.xlogo, gc, gcBack, -1, -1, h + 2, h + 2);
		XDrawRectangle(dpy, Scr->tbpm.xlogo, gcBack, 0, 0, h - 1, h - 1);

		/*
		 * done drawing
		 */
		XFreeGC(dpy, gc);
		XFreeGC(dpy, gcBack);
	}
	return Scr->tbpm.xlogo;
}


DEF_BI_PPM(CreateResizePixmap)
{
	int h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(h < 1) {
		h = 1;
	}

	*widthp = *heightp = (unsigned int) h;
	if(Scr->tbpm.resize == None) {
		XPoint  points[3];
		GC gc;
		int w;
		int lw;

		/*
		 * create the pixmap
		 */
		Scr->tbpm.resize = XCreatePixmap(dpy, Scr->Root, h, h, 1);
		gc = XCreateGC(dpy, Scr->tbpm.resize, 0L, NULL);
		XSetForeground(dpy, gc, 0);
		XFillRectangle(dpy, Scr->tbpm.resize, gc, 0, 0, h, h);
		XSetForeground(dpy, gc, 1);
		lw = h / 16;
		if(lw == 1) {
			lw = 0;
		}
		XSetLineAttributes(dpy, gc, lw, LineSolid, CapButt, JoinMiter);

		/*
		 * draw the resize button,
		 */
		w = (h * 2) / 3;
		points[0].x = w;
		points[0].y = 0;
		points[1].x = w;
		points[1].y = w;
		points[2].x = 0;
		points[2].y = w;
		XDrawLines(dpy, Scr->tbpm.resize, gc, points, 3, CoordModeOrigin);
		w = w / 2;
		points[0].x = w;
		points[0].y = 0;
		points[1].x = w;
		points[1].y = w;
		points[2].x = 0;
		points[2].y = w;
		XDrawLines(dpy, Scr->tbpm.resize, gc, points, 3, CoordModeOrigin);

		/*
		 * done drawing
		 */
		XFreeGC(dpy, gc);
	}
	return Scr->tbpm.resize;
}


#define questionmark_width 8
#define questionmark_height 8
static char questionmark_bits[] = {
	0x38, 0x7c, 0x64, 0x30, 0x18, 0x00, 0x18, 0x18
};

DEF_BI_PPM(CreateQuestionPixmap)
{
	*widthp = questionmark_width;
	*heightp = questionmark_height;
	if(Scr->tbpm.question == None) {
		Scr->tbpm.question = XCreateBitmapFromData(dpy, Scr->Root,
		                     questionmark_bits,
		                     questionmark_width,
		                     questionmark_height);
	}
	/*
	 * this must succeed or else we are in deep trouble elsewhere
	 */
	return Scr->tbpm.question;
}
#undef questionmark_height
#undef questionmark_width


DEF_BI_PPM(CreateMenuPixmap)
{
	return (CreateMenuIcon(Scr->TBInfo.width - Scr->TBInfo.border * 2, widthp,
	                       heightp));
}

DEF_BI_PPM(CreateDotPixmap)
{
	int h = Scr->TBInfo.width - Scr->TBInfo.border * 2;

	h = h * 3 / 4;
	if(h < 1) {
		h = 1;
	}
	if(!(h & 1)) {
		h--;
	}
	*widthp = *heightp = (unsigned int) h;
	if(Scr->tbpm.delete == None) {
		GC  gc;
		Pixmap pix;

		pix = Scr->tbpm.delete = XCreatePixmap(dpy, Scr->Root, h, h, 1);
		gc = XCreateGC(dpy, pix, 0L, NULL);
		XSetLineAttributes(dpy, gc, h, LineSolid, CapRound, JoinRound);
		XSetForeground(dpy, gc, 0L);
		XFillRectangle(dpy, pix, gc, 0, 0, h, h);
		XSetForeground(dpy, gc, 1L);
		XDrawLine(dpy, pix, gc, h / 2, h / 2, h / 2, h / 2);
		XFreeGC(dpy, gc);
	}
	return Scr->tbpm.delete;
}

#undef DEF_BI_PPM



/*
 * Next, the "3D/scalable" builtins.  These are the ones specified with
 * names like ":xpm:resize".  I'm not entirely clear on how these differ
 * from ":resize"; they both vary by UseThreeDTitles and look the same.
 * But, whatever.
 *
 * These yield [ctwm struct] Image's rather than [X11 type] Pixmap's.
 */
#define DEF_BI_SPM(nm) Image *nm(ColorPair cp)
static DEF_BI_SPM(Create3DMenuImage);
static DEF_BI_SPM(Create3DDotImage);
static DEF_BI_SPM(Create3DResizeImage);
static DEF_BI_SPM(Create3DZoomImage);
static DEF_BI_SPM(Create3DBarImage);
static DEF_BI_SPM(Create3DVertBarImage);
static DEF_BI_SPM(Create3DCrossImage);
static DEF_BI_SPM(Create3DIconifyImage);
static DEF_BI_SPM(Create3DSunkenResizeImage);
static DEF_BI_SPM(Create3DBoxImage);


/*
 * Main lookup
 *
 * This is where we find ":xpm:something".  Note that these are _not_
 * XPM's, and have no relation to the configurable XPM support, which we
 * get with images specified as "xpm:something" (no leading colon).
 * That's not confusing at all.
 */
Image *
get_builtin_scalable_pixmap(const char *name, ColorPair cp)
{
	int    i;
	static struct {
		char *name;
		DEF_BI_SPM((*proc));
	} pmtab[] = {
		/* Lookup for ":xpm:" pixmaps */
		{ TBPM_3DDOT,       Create3DDotImage },
		{ TBPM_3DRESIZE,    Create3DResizeImage },
		{ TBPM_3DMENU,      Create3DMenuImage },
		{ TBPM_3DZOOM,      Create3DZoomImage },
		{ TBPM_3DBAR,       Create3DBarImage },
		{ TBPM_3DVBAR,      Create3DVertBarImage },
		{ TBPM_3DCROSS,     Create3DCrossImage },
		{ TBPM_3DICONIFY,   Create3DIconifyImage },
		{ TBPM_3DBOX,       Create3DBoxImage },
		{ TBPM_3DSUNKEN_RESIZE, Create3DSunkenResizeImage },
	};

	/* Seatbelts */
	if(!name || (strncmp(name, ":xpm:", 5) != 0)) {
		return NULL;
	}

	for(i = 0; i < (sizeof pmtab) / (sizeof pmtab[0]); i++) {
		if(strcasecmp(pmtab[i].name, name) == 0) {
			Image *image = (*pmtab[i].proc)(cp);
			if(image == NULL) {
				fprintf(stderr, "%s:  unable to build pixmap \"%s\"\n",
				        ProgramName, name);
				return NULL;
			}
			return image;
		}
	}

	fprintf(stderr, "%s:  no such built-in pixmap \"%s\"\n", ProgramName, name);
	return NULL;
}



#define LEVITTE_TEST
static DEF_BI_SPM(Create3DCrossImage)
{
	Image *image;
	int        h;
	int    point;
	int midpoint;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}
	point = 4;
	midpoint = h / 2;

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);

#ifdef LEVITTE_TEST
	FB(cp.shadc, cp.shadd);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point + 1, point - 1, point - 1,
	          point + 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point + 1, point, point,
	          point + 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point - 1, point + 1, midpoint - 2,
	          midpoint);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, midpoint, midpoint + 2,
	          h - point - 3, h - point - 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, point + 1, h - point - 3,
	          h - point - 2);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point - 1, h - point - 2,
	          midpoint - 2, midpoint);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, midpoint, midpoint - 2,
	          h - point - 2, point - 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, h - point - 2,
	          h - point - 2, point);
#endif

	FB(cp.shadd, cp.shadc);
#ifdef LEVITTE_TEST
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point + 2, point + 1,
	          h - point - 1, h - point - 2);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point + 2, point, midpoint,
	          midpoint - 2);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, midpoint + 2, midpoint, h - point,
	          h - point - 2);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, h - point, h - point - 2,
	          h - point - 2, h - point);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, h - point - 1, h - point - 2,
	          h - point - 2, h - point - 1);
#else
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, point, h - point - 1,
	          h - point - 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point - 1, point, h - point - 1,
	          h - point);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, point - 1, h - point,
	          h - point - 1);
#endif

#ifdef LEVITTE_TEST
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, h - point - 1, point,
	          h - point - 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, h - point - 1, point,
	          h - point - 1, point);
#else
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, h - point - 1,
	          h - point - 1, point);
#endif
#ifdef LEVITTE_TEST
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point + 1, h - point - 1,
	          h - point - 1, point + 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point + 1, h - point, midpoint,
	          midpoint + 2);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, midpoint + 2, midpoint, h - point,
	          point + 1);
#else
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point - 1, h - point - 1,
	          h - point - 1, point - 1);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, h - point, h - point,
	          point);
#endif

	image->width  = h;
	image->height = h;

	return (image);
}
#undef LEVITTE_TEST

static DEF_BI_SPM(Create3DIconifyImage)
{
	Image *image;
	int     h;
	int point;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}
	point = ((h / 2 - 2) * 2 + 1) / 3;

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	FB(cp.shadd, cp.shadc);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, point, h / 2, h - point);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, point, point, h - point, point);

	FB(cp.shadc, cp.shadd);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, h - point, point, h / 2 + 1,
	          h - point);
	XDrawLine(dpy, image->pixmap, Scr->NormalGC, h - point - 1, point + 1,
	          h / 2 + 1, h - point - 1);

	image->width  = h;
	image->height = h;

	return (image);
}

static DEF_BI_SPM(Create3DSunkenResizeImage)
{
	int     h;
	Image *image;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap, 3, 3, h - 6, h - 6, 1, cp, on, true, false);
	Draw3DBorder(image->pixmap, 3, ((h - 6) / 3) + 3, ((h - 6) * 2 / 3) + 1,
	             ((h - 6) * 2 / 3) + 1, 1, cp, on, true, false);
	Draw3DBorder(image->pixmap, 3, ((h - 6) * 2 / 3) + 3, ((h - 6) / 3) + 1,
	             ((h - 6) / 3) + 1, 1, cp, on, true, false);

	image->width  = h;
	image->height = h;

	return (image);
}

static DEF_BI_SPM(Create3DBoxImage)
{
	int     h;
	Image   *image;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap, (h / 2) - 4, (h / 2) - 4, 9, 9, 1, cp,
	             off, true, false);

	image->width  = h;
	image->height = h;

	return (image);
}

static DEF_BI_SPM(Create3DDotImage)
{
	Image *image;
	int   h;
	static int idepth = 2;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap, (h / 2) - idepth,
	             (h / 2) - idepth,
	             2 * idepth + 1,
	             2 * idepth + 1,
	             idepth, cp, off, true, false);
	image->width  = h;
	image->height = h;
	return (image);
}

static DEF_BI_SPM(Create3DBarImage)
{
	Image *image;
	int   h;
	static int idepth = 2;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap,
	             Scr->TitleButtonShadowDepth + 2,
	             (h / 2) - idepth,
	             h - 2 * (Scr->TitleButtonShadowDepth + 2),
	             2 * idepth + 1,
	             idepth, cp, off, true, false);
	image->width  = h;
	image->height = h;
	return (image);
}

static DEF_BI_SPM(Create3DVertBarImage)
{
	Image *image;
	int   h;
	static int idepth = 2;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap,
	             (h / 2) - idepth,
	             Scr->TitleButtonShadowDepth + 2,
	             2 * idepth + 1,
	             h - 2 * (Scr->TitleButtonShadowDepth + 2),
	             idepth, cp, off, true, false);
	image->width  = h;
	image->height = h;
	return (image);
}

static DEF_BI_SPM(Create3DMenuImage)
{
	Image *image;
	int   h, i;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	for(i = 4; i < h - 7; i += 5) {
		Draw3DBorder(image->pixmap, 4, i, h - 8, 4, 2, cp, off, true, false);
	}
	image->width  = h;
	image->height = h;
	return (image);
}

static DEF_BI_SPM(Create3DResizeImage)
{
	Image *image;
	int   h;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap, 0, h / 4, ((3 * h) / 4) + 1, ((3 * h) / 4) + 1,
	             2, cp, off, true, false);
	Draw3DBorder(image->pixmap, 0, h / 2, (h / 2) + 1, (h / 2) + 1, 2, cp, off,
	             true, false);
	image->width  = h;
	image->height = h;
	return (image);
}

static DEF_BI_SPM(Create3DZoomImage)
{
	Image *image;
	int         h;
	static int idepth = 2;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = AllocImage();
	if(! image) {
		return (None);
	}
	image->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	if(image->pixmap == None) {
		free(image);
		return (None);
	}

	Draw3DBorder(image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
	             off, true, false);
	Draw3DBorder(image->pixmap, Scr->TitleButtonShadowDepth + 2,
	             Scr->TitleButtonShadowDepth + 2,
	             h - 2 * (Scr->TitleButtonShadowDepth + 2),
	             h - 2 * (Scr->TitleButtonShadowDepth + 2),
	             idepth, cp, off, true, false);

	image->width  = h;
	image->height = h;
	return (image);
}

#undef DEF_BI_SPM



/*
 * And the animated builtins.  These are the ones specified with names
 * like "%xpm:resize".
 *
 * These yield [ctwm struct] Image's.
 */
#define DEF_BI_ASPM(nm) Image *nm(ColorPair cp)

/* Backend generators */
static Image *Create3DResizeAnimation(bool in, bool left, bool top,
                                      ColorPair cp);
static Image *Create3DMenuAnimation(bool up, ColorPair cp);
static Image *Create3DZoomAnimation(bool in, bool out, int n, ColorPair cp);

/* Frontends */
/* Using: ResizeAnimation */
static DEF_BI_ASPM(Create3DResizeInTopAnimation);
static DEF_BI_ASPM(Create3DResizeOutTopAnimation);
static DEF_BI_ASPM(Create3DResizeInBotAnimation);
static DEF_BI_ASPM(Create3DResizeOutBotAnimation);
/* Using: MenuAnimation */
static DEF_BI_ASPM(Create3DMenuUpAnimation);
static DEF_BI_ASPM(Create3DMenuDownAnimation);
/* Using: ZoomAnimation */
static DEF_BI_ASPM(Create3DMazeOutAnimation);
static DEF_BI_ASPM(Create3DMazeInAnimation);
static DEF_BI_ASPM(Create3DZoomInAnimation);
static DEF_BI_ASPM(Create3DZoomOutAnimation);
static DEF_BI_ASPM(Create3DZoomInOutAnimation);


/*
 * Entry for animated pixmaps
 *
 * This is where we find "%xpm:something".  Note that as above, these are
 * _not_ XPM's, and have no relation to the configurable XPM support,
 * which we get with images specified as "xpm:something" (no leading
 * colon).  Still not confusing at _all_.
 */
Image *
get_builtin_animated_pixmap(const char *name, ColorPair cp)
{
	int    i;
	static struct {
		char *name;
		DEF_BI_ASPM((*proc));
	} pmtab[] = {
		/* Lookup for "%xpm:" pixmaps */
		{ "%xpm:resize-out-top", Create3DResizeInTopAnimation },
		{ "%xpm:resize-in-top",  Create3DResizeOutTopAnimation },
		{ "%xpm:resize-out-bot", Create3DResizeInBotAnimation },
		{ "%xpm:resize-in-bot",  Create3DResizeOutBotAnimation },
		{ "%xpm:menu-up",        Create3DMenuUpAnimation },
		{ "%xpm:menu-down",      Create3DMenuDownAnimation },
		{ "%xpm:maze-out",       Create3DMazeOutAnimation },
		{ "%xpm:maze-in",        Create3DMazeInAnimation },
		{ "%xpm:resize",         Create3DZoomOutAnimation }, // compat
		{ "%xpm:zoom-out",       Create3DZoomOutAnimation },
		{ "%xpm:zoom-in",        Create3DZoomInAnimation },
		{ "%xpm:zoom-inout",     Create3DZoomInOutAnimation },
	};

	/* Seatbelts */
	if(!name || (strncmp(name, "%xpm:", 5) != 0)) {
		return NULL;
	}

	for(i = 0; i < (sizeof pmtab) / (sizeof pmtab[0]); i++) {
		if(strcasecmp(pmtab[i].name, name) == 0) {
			Image *image = (*pmtab[i].proc)(cp);
			if(image == NULL) {
				fprintf(stderr, "%s:  unable to build pixmap \"%s\"\n",
				        ProgramName, name);
				return NULL;
			}
			return image;
		}
	}

	fprintf(stderr, "%s:  no such built-in pixmap \"%s\"\n", ProgramName, name);
	return (None);
}


/*
 * First a couple generator functions the actual functions use
 */
static Image *
Create3DResizeAnimation(bool in, bool left, bool top,
                        ColorPair cp)
{
	int         h, i, j;
	Image       *image, *im, *im1;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = im1 = NULL;
	for(i = (in ? 0 : (h / 4) - 1); (i < h / 4) && (i >= 0); i += (in ? 1 : -1)) {
		im = AllocImage();
		if(! im) {
			return NULL;
		}
		im->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
		if(im->pixmap == None) {
			free(im);
			return NULL;
		}
		Draw3DBorder(im->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
		             off, true, false);
		for(j = i; j <= h; j += (h / 4)) {
			Draw3DBorder(im->pixmap, (left ? 0 : j), (top ? 0 : j),
			             h - j, h - j, 2, cp, off, true, false);
		}
		im->mask   = None;
		im->width  = h;
		im->height = h;
		im->next   = NULL;
		if(image == NULL) {
			image = im1 = im;
		}
		else {
			im1->next = im;
			im1 = im;
		}
	}
	if(im1 != None) {
		im1->next = image;
	}
	return image;
}

static Image *
Create3DMenuAnimation(bool up, ColorPair cp)
{
	int   h, i, j;
	Image *image, *im, *im1;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	image = im1 = NULL;
	for(j = (up ? 4 : 0); j != (up ? -1 : 5); j += (up ? -1 : 1)) {
		im = AllocImage();
		if(! im) {
			return NULL;
		}
		im->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
		if(im->pixmap == None) {
			free(im);
			return NULL;
		}
		Draw3DBorder(im->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp,
		             off, true, false);
		for(i = j; i < h - 3; i += 5) {
			Draw3DBorder(im->pixmap, 4, i, h - 8, 4, 2, cp, off, true, false);
		}
		im->mask   = None;
		im->width  = h;
		im->height = h;
		im->next   = NULL;
		if(image == NULL) {
			image = im1 = im;
		}
		else {
			im1->next = im;
			im1 = im;
		}
	}
	if(im1 != None) {
		im1->next = image;
	}
	return image;
}

static Image *
Create3DZoomAnimation(bool in, bool out, int n, ColorPair cp)
{
	int         h, i, j, k;
	Image       *image, *im, *im1;

	h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
	if(!(h & 1)) {
		h--;
	}

	if(n == 0) {
		n = (h / 2) - 2;
	}

	image = im1 = NULL;
	for(j = (out ? -1 : 1) ; j < (in ? 2 : 0); j += 2) {
		for(k = (j > 0 ? 0 : n - 1) ; (k >= 0) && (k < n); k += j) {
			im = AllocImage();
			im->pixmap = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
			Draw3DBorder(im->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth,
			             cp, off, true, false);
			for(i = 2 + k; i < (h / 2); i += n) {
				Draw3DBorder(im->pixmap, i, i, h - (2 * i), h - (2 * i), 2, cp,
				             off, true, false);
			}
			im->mask   = None;
			im->width  = h;
			im->height = h;
			im->next   = NULL;
			if(image == NULL) {
				image = im1 = im;
			}
			else {
				im1->next = im;
				im1 = im;
			}
		}
	}
	if(im1 != None) {
		im1->next = image;
	}
	return image;
}


/*
 * And the wrapper funcs for making the images
 */
static DEF_BI_ASPM(Create3DResizeInTopAnimation)
{
	return Create3DResizeAnimation(true, false, true, cp);
}

static DEF_BI_ASPM(Create3DResizeOutTopAnimation)
{
	return Create3DResizeAnimation(false, false, true, cp);
}

static DEF_BI_ASPM(Create3DResizeInBotAnimation)
{
	return Create3DResizeAnimation(true, true, false, cp);
}

static DEF_BI_ASPM(Create3DResizeOutBotAnimation)
{
	return Create3DResizeAnimation(false, true, false, cp);
}


static DEF_BI_ASPM(Create3DMenuUpAnimation)
{
	return Create3DMenuAnimation(true, cp);
}

static DEF_BI_ASPM(Create3DMenuDownAnimation)
{
	return Create3DMenuAnimation(false, cp);
}


static DEF_BI_ASPM(Create3DMazeInAnimation)
{
	return Create3DZoomAnimation(true, false, 6, cp);
}

static DEF_BI_ASPM(Create3DMazeOutAnimation)
{
	return Create3DZoomAnimation(false, true, 6, cp);
}

static DEF_BI_ASPM(Create3DZoomInAnimation)
{
	return Create3DZoomAnimation(true, false, 0, cp);
}

static DEF_BI_ASPM(Create3DZoomOutAnimation)
{
	return Create3DZoomAnimation(false, true, 0, cp);
}

static DEF_BI_ASPM(Create3DZoomInOutAnimation)
{
	return Create3DZoomAnimation(true, true, 0, cp);
}

#undef DEF_BI_ASPM


/*
 * Lastly, some gray/black pixmaps that are used in window border and
 * hilite bars.
 */
#define BG_WIDTH  2
#define BG_HEIGHT 2

Pixmap
mk_blackgray_pixmap(const char *which, Drawable dw,
                    unsigned long fg, unsigned long bg)
{
	unsigned char gray_bits[]  = { 0x02, 0x01 };
	unsigned char black_bits[] = { 0xFF, 0xFF };
	char *bits;

	/* Which are we asking for? */
	if(strcmp(which, "black") == 0) {
		bits = (char *)black_bits;
	}
	else if(strcmp(which, "gray") == 0) {
		bits = (char *)gray_bits;
	}
	else {
		fprintf(stderr, "%s(): Invalid which arg '%s'\n", __func__, which);
		return None;
	}

	/* Make it */
	return XCreatePixmapFromBitmapData(dpy, dw,
	                                   bits, BG_WIDTH, BG_HEIGHT,
	                                   fg, bg, Scr->d_depth);
}

void
get_blackgray_size(int *width, int *height)
{
	if(width) {
		*width  = BG_WIDTH;
	}
	if(height) {
		*height = BG_HEIGHT;
	}
}

#undef BG_HEIGHT
#undef BG_WIDTH
