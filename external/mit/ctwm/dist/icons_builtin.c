/*
 * Built-in icon building
 *
 * This conceptually overlaps pretty strongly with
 * image_bitmap_builtin.c, since both are about drawing some built-in
 * images used for internal stuff.  They're kept separate because i_b_b
 * is backend bits for GetImage() calls for ":xpm:", "%xpm:", and ":"
 * images, while these are called directly from the code.  Perhaps they
 * should be subsumed under that, but they haven't been so far, so we're
 * keeping them separate for now.
 */

#include "ctwm.h"

#include <stdlib.h>

#include "drawing.h"
#include "screen.h"
#include "iconmgr.h"

#include "icons_builtin.h"


struct Colori {
	Pixel color;
	Pixmap pix;
	struct Colori *next;
};


/*
 * The icons on menu items for submenus, in UseThreeDMenus and non
 * variants.
 */
Pixmap
Create3DMenuIcon(unsigned int height,
                 unsigned int *widthp, unsigned int *heightp,
                 ColorPair cp)
{
	unsigned int h, w;
	int         i;
	struct Colori *col;
	static struct Colori *colori = NULL;

	h = height;
	w = h * 7 / 8;
	if(h < 1) {
		h = 1;
	}
	if(w < 1) {
		w = 1;
	}
	*widthp  = w;
	*heightp = h;

	for(col = colori; col; col = col->next) {
		if(col->color == cp.back) {
			break;
		}
	}
	if(col != NULL) {
		return (col->pix);
	}
	col = malloc(sizeof(struct Colori));
	col->color = cp.back;
	col->pix   = XCreatePixmap(dpy, Scr->Root, h, h, Scr->d_depth);
	col->next = colori;
	colori = col;

	Draw3DBorder(col->pix, 0, 0, w, h, 1, cp, off, true, false);
	for(i = 3; i + 5 < h; i += 5) {
		Draw3DBorder(col->pix, 4, i, w - 8, 3, 1, Scr->MenuC, off, true, false);
	}
	return (colori->pix);
}

Pixmap
CreateMenuIcon(int height, unsigned int *widthp, unsigned int *heightp)
{
	int h, w;
	int ih, iw;
	int ix, iy;
	int mh, mw;
	int tw, th;
	int lw, lh;
	int lx, ly;
	int lines, dly;
	int offset;
	int bw;

	h = height;
	w = h * 7 / 8;
	if(h < 1) {
		h = 1;
	}
	if(w < 1) {
		w = 1;
	}
	*widthp = w;
	*heightp = h;
	if(Scr->tbpm.menu == None) {
		Pixmap  pix;
		GC      gc;

		pix = Scr->tbpm.menu = XCreatePixmap(dpy, Scr->Root, w, h, 1);
		gc = XCreateGC(dpy, pix, 0L, NULL);
		XSetForeground(dpy, gc, 0L);
		XFillRectangle(dpy, pix, gc, 0, 0, w, h);
		XSetForeground(dpy, gc, 1L);
		ix = 1;
		iy = 1;
		ih = h - iy * 2;
		iw = w - ix * 2;
		offset = ih / 8;
		mh = ih - offset;
		mw = iw - offset;
		bw = mh / 16;
		if(bw == 0 && mw > 2) {
			bw = 1;
		}
		tw = mw - bw * 2;
		th = mh - bw * 2;
		XFillRectangle(dpy, pix, gc, ix, iy, mw, mh);
		XFillRectangle(dpy, pix, gc, ix + iw - mw, iy + ih - mh, mw, mh);
		XSetForeground(dpy, gc, 0L);
		XFillRectangle(dpy, pix, gc, ix + bw, iy + bw, tw, th);
		XSetForeground(dpy, gc, 1L);
		lw = tw / 2;
		if((tw & 1) ^ (lw & 1)) {
			lw++;
		}
		lx = ix + bw + (tw - lw) / 2;

		lh = th / 2 - bw;
		if((lh & 1) ^ ((th - bw) & 1)) {
			lh++;
		}
		ly = iy + bw + (th - bw - lh) / 2;

		lines = 3;
		if((lh & 1) && lh < 6) {
			lines--;
		}
		dly = lh / (lines - 1);
		while(lines--) {
			XFillRectangle(dpy, pix, gc, lx, ly, lw, bw);
			ly += dly;
		}
		XFreeGC(dpy, gc);
	}
	return Scr->tbpm.menu;
}



/*
 * Icon used in the icon manager for iconified windows.
 *
 * For the 2d case, there's just one icon stored screen-wide, which is
 * XCopyPlane()'d into the icon manager.  This works because it's just a
 * 2-color thing represented as a bitmap, and we color it to match the
 * FG/BG of the row at the time.
 *
 * The 3d variant is more complicated, and doesn't just use the row's
 * FG/BG colors; it draws various shades from them.  So since each row in
 * an icon manager may be a different FG/BG color, we have to make a new
 * one for each row.
 */

const unsigned int im_iconified_icon_width = 11;
const unsigned int im_iconified_icon_height = 11;
static unsigned char im_iconified_icon_bits[] = {
	0xff, 0x07, 0x01, 0x04, 0x0d, 0x05, 0x9d, 0x05, 0xb9, 0x04, 0x51, 0x04,
	0xe9, 0x04, 0xcd, 0x05, 0x85, 0x05, 0x01, 0x04, 0xff, 0x07
};

Pixmap
Create3DIconManagerIcon(ColorPair cp)
{
	struct Colori *col;
	static struct Colori *colori = NULL;
	const unsigned int w = im_iconified_icon_width;
	const unsigned int h = im_iconified_icon_height;

	/*
	 * Keep a list of ones we've made, and if we've already made one this
	 * color, just hand it back.
	 */
	for(col = colori; col; col = col->next) {
		if(col->color == cp.back) {
			return col->pix;
		}
	}

	/* Don't have one this color yet, make it */
	col = malloc(sizeof(struct Colori));
	col->color = cp.back;
	col->pix   = XCreatePixmap(dpy, Scr->Root, w, h, Scr->d_depth);
	Draw3DBorder(col->pix, 0, 0, w, h, 4, cp, off, true, false);

	/* Add to the cache list so we hit the above next time */
	col->next = colori;
	colori = col;

	return colori->pix;
}

Pixmap
Create2DIconManagerIcon(void)
{
	char *bits = (char *)im_iconified_icon_bits;
	const unsigned int w = im_iconified_icon_width;
	const unsigned int h = im_iconified_icon_height;

	return XCreatePixmapFromBitmapData(dpy, Scr->Root, bits, w, h, 1, 0, 1);
}
