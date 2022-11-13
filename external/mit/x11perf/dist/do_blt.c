/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include "x11perf.h"
#include <stdio.h>

#define NUMPOINTS 100

static Pixmap   pix;
static XImage   *image;
static XPoint   points[NUMPOINTS];
static XSegment *segsa, *segsb;
static XSegment *segsa2, *segsb2;

#define NegMod(x, y) ((y) - (((-x)-1) % (7)) - 1)

static void 
InitBltLines(void)
{
    int x, y;

    points[0].x = points[0].y = y = 0;
    for (int i = 1; i != NUMPOINTS/2; i++) {
	if (i & 1) {
	    points[i].x = WIDTH-1;
	} else {
	    points[i].x = 0;
	}
	y += HEIGHT / (NUMPOINTS/2);
	points[i].y = y;
    }
    
    x = 0;
    for (int i = NUMPOINTS/2; i!= NUMPOINTS; i++) {
	if (i & 1) {
	    points[i].y = HEIGHT-1;
	} else {
	    points[i].y = 0;
	}
	x += WIDTH / (NUMPOINTS/2);
	points[i].x = x;
    }
}

int 
InitScroll(XParms xp, Parms p, int64_t reps)
{
    InitBltLines();
    XDrawLines(xp->d, xp->w, xp->fggc, points, NUMPOINTS, CoordModeOrigin);
    return reps;
}

void 
DoScroll(XParms xp, Parms p, int64_t reps)
{
    int size, x, y, xorg, yorg, delta;

    size = p->special;
    xorg = 0;   yorg = 0;
    x    = 0;   y    = 0;
    if (xp->version == VERSION1_2) {
	delta = 1;
    } else {
	/* Version 1.2 only scrolled up by 1 scanline, which made hardware
	   using page-mode access to VRAM look better on paper than it would
	   perform in a more realistic scroll.  So we've changed to scroll by
	   the height of the 6x13 fonts. */
	delta = 13;
    }

    for (int i = 0; i != reps; i++) {
	XCopyArea(xp->d, xp->w, xp->w, xp->fggc, x, y + delta,
	    size, size, x, y);
	y += size;
	if (y + size + delta > HEIGHT) {
	    yorg += delta;
	    if (yorg >= size || yorg + size + delta > HEIGHT) {
		yorg = 0;
		xorg++;
		if (xorg >= size || xorg + size > WIDTH) {
		    xorg = 0;
		}
	    }
	    y = yorg;
	    x += size;
	    if (x + size > WIDTH) {
		x = xorg;
	    }
	}
	CheckAbort ();
    }
}

void 
MidScroll(XParms xp, Parms p)
{
    XClearWindow(xp->d, xp->w);
    XDrawLines(xp->d, xp->w, xp->fggc, points, NUMPOINTS, CoordModeOrigin);
}

void 
EndScroll(XParms xp, Parms p)
{
}

static void 
InitCopyLocations(int size, int mul, int div, 
		  int64_t reps, XSegment **ap, XSegment **bp)
{
    int x1, y1, x2, y2;
    int xinc, yinc;
    int width, height;
    XSegment *a, *b;

    size = size * mul / div;
    /* Try to exercise all alignments of src and destination equally, as well
       as all 4 top-to-bottom/bottom-to-top, left-to-right, right-to-left
       copying directions.  Computation done here just to make sure slow
       machines aren't measuring anything but the XCopyArea calls.
    */
    xinc = (size & ~3) + 1;
    yinc = xinc + 3;

    width = (WIDTH - size) & ~31;
    height = (HEIGHT - size) & ~31;
    
    x1 = 0;
    y1 = 0;
    x2 = width;
    y2 = height;
    
    *ap = a = malloc(reps * sizeof(XSegment));
    *bp = b = malloc(reps * sizeof(XSegment));
    for (int i = 0; i != reps; i++) {
	a[i].x1 = x1 * div / mul;
	a[i].y1 = y1 * div / mul;
	a[i].x2 = x2 * div / mul;
	a[i].y2 = y2 * div / mul;

	/* Move x2, y2, location backward */
	x2 -= xinc;
	if (x2 < 0) {
	    x2 = NegMod(x2, width);
	    y2 -= yinc;
	    if (y2 < 0) {
		y2 = NegMod(y2, height);
	    }
	}

	b[i].x1 = x1 * div / mul;
	b[i].y1 = y1 * div / mul;
	b[i].x2 = x2 * div / mul;
	b[i].y2 = y2 * div / mul;

	/* Move x1, y1 location forward */
	x1 += xinc;
	if (x1 > width) {
	    x1 %= 32;
	    y1 += yinc;
	    if (y1 > height) {
		y1 %= 32;
	    }
	}
    } /* end for */
}


int 
InitCopyWin(XParms xp, Parms p, int64_t reps)
{
    (void) InitScroll(xp, p, reps);
    InitCopyLocations(p->special, 1, 1, reps, &segsa, &segsb);
    return reps;
}

int 
InitCopyPix(XParms xp, Parms p, int64_t reps)
{
    GC		pixgc;
    (void) InitCopyWin(xp, p, reps);

    /* Create pixmap to write stuff into, and initialize it */
    pix = XCreatePixmap(xp->d, xp->w, WIDTH, HEIGHT, xp->vinfo.depth);
    pixgc = XCreateGC(xp->d, pix, 0, NULL);
    /* need a gc with GXcopy cos pixmaps contain junk on creation. mmm */
    XCopyArea(xp->d, xp->w, pix, pixgc, 0, 0, WIDTH, HEIGHT, 0, 0);
    XFreeGC(xp->d, pixgc);
    return reps;
}

int 
InitGetImage(XParms xp, Parms p, int64_t reps)
{
    (void) InitCopyWin(xp, p, reps);

    /* Create image to stuff bits into */
    image = XGetImage(xp->d, xp->w, 0, 0, WIDTH, HEIGHT, xp->planemask,
		      p->font==NULL?ZPixmap:XYPixmap);
    if(image==NULL){
	printf("XGetImage failed\n");
	return False;
    }	
    return reps;
}

int 
InitPutImage(XParms xp, Parms p, int64_t reps)
{
    if(!InitGetImage(xp, p, reps))return False;
    XClearWindow(xp->d, xp->w);
    return reps;
}

static void 
CopyArea(XParms xp, Parms p, int64_t reps, Drawable src, Drawable dst)
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sa->x1, sa->y1, size, size, sa->x2, sa->y2);
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sa->x2, sa->y2, size, size, sa->x1, sa->y1);
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sb->x2, sb->y2, size, size, sb->x1, sb->y1);
	XCopyArea(xp->d, src, dst, xp->fggc,
	    sb->x1, sb->y1, size, size, sb->x2, sb->y2);
	CheckAbort ();
    }
}

void 
DoCopyWinWin(XParms xp, Parms p, int64_t reps)
{
    CopyArea(xp, p, reps, xp->w, xp->w);
}

void 
DoCopyPixWin(XParms xp, Parms p, int64_t reps)
{
    CopyArea(xp, p, reps, pix, xp->w);
}

void 
DoCopyWinPix(XParms xp, Parms p, int64_t reps)
{
    CopyArea(xp, p, reps, xp->w, pix);
    xp->p = pix;	/* HardwareSync will now sync on pixmap */
}

void 
DoCopyPixPix(XParms xp, Parms p, int64_t reps)
{
    CopyArea(xp, p, reps, pix, pix);
    xp->p = pix;	/* HardwareSync will now sync on pixmap */
}

void 
DoGetImage(XParms xp, Parms p, int64_t reps)
{
    int i, size;
    XSegment *sa, *sb;
    int format;

    size = p->special;
    format = (p->font == NULL) ? ZPixmap : XYPixmap;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sa->x1, sa->y1, size, size,
	    xp->planemask, format);
	if (image) XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sa->x2, sa->y2, size, size,
	    xp->planemask, format);
	if (image) XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sb->x2, sb->y2, size, size,
	    xp->planemask, format);
	if (image) XDestroyImage(image);
	image = XGetImage(xp->d, xp->w, sb->x1, sb->y1, size, size,
	    xp->planemask, format);
/*

One might expect XGetSubImage to be slightly faster than XGetImage.  Go look
at the code in Xlib.  MIT X11R3 ran approximately 30 times slower for a 500x500
rectangle.

	(void) XGetSubImage(xp->d, xp->w, sa->x1, sa->y1, size, size,
	    xp->planemask, ZPixmap, image, sa->x2, sa->y2);
	(void) XGetSubImage(xp->d, xp->w, sa->x2, sa->y2, size, size,
	    xp->planemask, ZPixmap, image, sa->x1, sa->y1);
	(void) XGetSubImage(xp->d, xp->w, sb->x2, sb->y2, size, size,
	    xp->planemask, ZPixmap, image, sb->x2, sb->y2);
	(void) XGetSubImage(xp->d, xp->w, sb->x1, sb->y1, size, size,
	    xp->planemask, ZPixmap, image, sb->x2, sb->y2);
*/
	CheckAbort ();
    }
}

void 
DoPutImage(XParms xp, Parms p, int64_t reps)
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sa->x1, sa->y1, sa->x2, sa->y2, size, size);
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sa->x2, sa->y2, sa->x1, sa->y1, size, size);
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sb->x2, sb->y2, sb->x2, sb->y2, size, size);
	XPutImage(xp->d, xp->w, xp->fggc, image,
	    sb->x1, sb->y1, sb->x2, sb->y2, size, size);
	CheckAbort ();
    }
}

#ifdef MITSHM

#include <sys/types.h>
#ifndef Lynx
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#include <ipc.h>
#include <shm.h>
#endif
#include <X11/extensions/XShm.h>

static XImage		shm_image;
static XShmSegmentInfo	shm_info;

static int haderror;
static int (*origerrorhandler)(Display *, XErrorEvent *);

static int 
shmerrorhandler(Display *d, XErrorEvent *e)
{
    haderror++;
    if(e->error_code==BadAccess) {
	fprintf(stderr,"failed to attach shared memory\n");
	return 0;
    } else 
	return (*origerrorhandler)(d,e);
}

static int
InitShmImage(XParms xp, Parms p, int64_t reps, Bool read_only)
{
    int	image_size;

    if(!InitGetImage(xp, p, reps))return False;
    if (!XShmQueryExtension(xp->d)) {
	/*
 	 * Clean up here because cleanup function is not called if this
	 * function fails
	 */
       	if (image)
      	    XDestroyImage(image);
    	image = NULL;
    	free(segsa);
    	free(segsb);
    	return False;
    }	
    shm_image = *image;
    image_size = image->bytes_per_line * image->height;
    /* allow XYPixmap choice: */
    if(p->font)image_size *= xp->vinfo.depth;
    shm_info.shmid = shmget(IPC_PRIVATE, image_size, IPC_CREAT|0777);
    if (shm_info.shmid < 0)
    {
	/*
	 * Clean up here because cleanup function is not called if this
	 * function fails
	 */
	if (image)
	    XDestroyImage(image);
	image = NULL;
	free(segsa);
	free(segsb);
	perror ("shmget");
	return False;
    }
    shm_info.shmaddr = (char *) shmat(shm_info.shmid, NULL, 0);
    if (shm_info.shmaddr == ((char *) -1))
    {
	/*
	 * Clean up here because cleanup function is not called if this
	 * function fails
	 */
	if (image)
	    XDestroyImage(image);
	image = NULL;
	free(segsa);
	free(segsb);
	perror ("shmat");
	shmctl (shm_info.shmid, IPC_RMID, NULL);
	return False;
    }
    shm_info.readOnly = read_only;
    XSync(xp->d,True);
    haderror = False;
    origerrorhandler = XSetErrorHandler(shmerrorhandler);
    XShmAttach (xp->d, &shm_info);
    XSync(xp->d,True);	/* wait for error or ok */
    XSetErrorHandler(origerrorhandler);
    if(haderror){
	/*
	 * Clean up here because cleanup function is not called if this
	 * function fails
	 */
	if (image)
	    XDestroyImage(image);
	image = NULL;
	free(segsa);
	free(segsb);
	if(shmdt (shm_info.shmaddr)==-1)
	    perror("shmdt:");
	if(shmctl (shm_info.shmid, IPC_RMID, NULL)==-1)
	    perror("shmctl rmid:");
	return False;
    }
    shm_image.data = shm_info.shmaddr;
    memmove( shm_image.data, image->data, image_size);
    shm_image.obdata = (char *) &shm_info;
    return reps;
}

int
InitShmPutImage(XParms xp, Parms p, int64_t reps)
{
    if (!InitShmImage(xp, p, reps, True)) return False;
    XClearWindow(xp->d, xp->w);
    return reps;
}

int
InitShmGetImage(XParms xp, Parms p, int64_t reps)
{
    return InitShmImage(xp, p, reps, False);
}

void 
DoShmPutImage(XParms xp, Parms p, int64_t reps)
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sa->x1, sa->y1, sa->x2, sa->y2, size, size, False);
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sa->x2, sa->y2, sa->x1, sa->y1, size, size, False);
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sb->x2, sb->y2, sb->x2, sb->y2, size, size, False);
	XShmPutImage(xp->d, xp->w, xp->fggc, &shm_image,
	    sb->x1, sb->y1, sb->x2, sb->y2, size, size, False);
	CheckAbort ();
    }
}

void
DoShmGetImage(XParms xp, Parms p, int64_t reps)
{
    int i, size;
    XSegment *sa, *sb;

    size = p->special;

    shm_image.width = size;
    shm_image.height = size;

    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	/* compute offsets into image data? */
	XShmGetImage(xp->d, xp->w, &shm_image, sa->x1, sa->y1, xp->planemask);
	XShmGetImage(xp->d, xp->w, &shm_image, sa->x2, sa->y2, xp->planemask);
	XShmGetImage(xp->d, xp->w, &shm_image, sb->x2, sb->y2, xp->planemask);
	XShmGetImage(xp->d, xp->w, &shm_image, sb->x1, sb->y1, xp->planemask);
	CheckAbort ();
    }
}

static void
EndShmImage(XParms xp, Parms p)
{
    EndGetImage (xp, p);
    XShmDetach (xp->d, &shm_info);
    XSync(xp->d, False);	/* need server to detach so can remove id */
    if(shmdt (shm_info.shmaddr)==-1)
	perror("shmdt:");
    if(shmctl (shm_info.shmid, IPC_RMID, NULL)==-1)
	perror("shmctl rmid:");
}

void
EndShmGetImage(XParms xp, Parms p)
{
    EndShmImage(xp, p);
}

void
EndShmPutImage(XParms xp, Parms p)
{
    EndShmImage(xp, p);
}
#endif


void 
MidCopyPix(XParms xp, Parms p)
{
    XClearWindow(xp->d, xp->w);
}

void 
EndCopyWin(XParms xp, Parms p)
{
    EndScroll(xp, p);
    free(segsa);
    free(segsb);
    if (segsa2)
	free (segsa2);
    if (segsb2)
	free (segsb2);
    segsa = segsb = segsa2 = segsb2 = NULL;
}

void 
EndCopyPix(XParms xp, Parms p)
{
    EndCopyWin(xp, p);
    XFreePixmap(xp->d, pix);
    /*
     * Ensure that the next test doesn't try and sync on the pixmap
     */
    xp->p = (Pixmap)0;
}

void 
EndGetImage(XParms xp, Parms p)
{
    EndCopyWin(xp, p);
    if (image) XDestroyImage(image);
}

int
InitCopyPlane(XParms xp, Parms p, int64_t reps)
{
    XGCValues   gcv;
    GC		pixgc;

    InitBltLines();
    InitCopyLocations(p->special, 1, 1, reps, &segsa, &segsb);

    /* Create pixmap to write stuff into, and initialize it */
    pix = XCreatePixmap(xp->d, xp->w, WIDTH, HEIGHT, 
	    p->font==NULL ? 1 : xp->vinfo.depth);
    gcv.graphics_exposures = False;
    gcv.foreground = 0;
    gcv.background = 1;
    pixgc = XCreateGC(xp->d, pix, 
		GCForeground | GCBackground | GCGraphicsExposures, &gcv);
    XFillRectangle(xp->d, pix, pixgc, 0, 0, WIDTH, HEIGHT);
    gcv.foreground = 1;
    gcv.background = 0;
    XChangeGC(xp->d, pixgc, GCForeground | GCBackground, &gcv);
    XDrawLines(xp->d, pix, pixgc, points, NUMPOINTS, CoordModeOrigin);
    XFreeGC(xp->d, pixgc);

    return reps;
}

void 
DoCopyPlane(XParms xp, Parms p, int64_t reps)
{
    int		i, size;
    XSegment    *sa, *sb;

    size = p->special;
    for (sa = segsa, sb = segsb, i = 0; i != reps; i++, sa++, sb++) {
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sa->x1, sa->y1, size, size, sa->x2, sa->y2, 1);
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sa->x2, sa->y2, size, size, sa->x1, sa->y1, 1);
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sb->x2, sb->y2, size, size, sb->x1, sb->y1, 1);
	XCopyPlane(xp->d, pix, xp->w, xp->fggc,
	    sb->x1, sb->y1, size, size, sb->x2, sb->y2, 1);
	CheckAbort ();
    }
}

#include <X11/extensions/Xrender.h>

static Picture	winPict, pixPict;

int
InitCompositeWin(XParms xp, Parms p, int64_t reps)
{
    XRenderPictFormat	*format;

    (void) InitScroll (xp, p, reps);
    InitCopyLocations(p->special, 1, 1, reps, &segsa, &segsb);
    if (p->fillStyle) {
	int mul = 0x10000;
	int div = p->fillStyle;
	InitCopyLocations (p->special, mul, div, reps, &segsa2, &segsb2);
    }
    format = XRenderFindVisualFormat (xp->d, xp->vinfo.visual);
    winPict = XRenderCreatePicture (xp->d, xp->w, format, 0, NULL);
    return reps;
}

int
InitCompositePix(XParms xp, Parms p, int64_t reps)
{
    XRenderPictFormat	*format = NULL;
    int			depth;
    static XRenderColor c = { 0xffff, 0x0000, 0xffff, 0xffff };

    (void) InitCompositeWin (xp, p, reps);
    
    /* Create pixmap to write stuff into, and initialize it */
    switch (xp->planemask) {
    case PictStandardNative:
	depth = xp->vinfo.depth;
	format = XRenderFindVisualFormat (xp->d, xp->vinfo.visual);
	break;
    case PictStandardRGB24:
	depth = 24;
	break;
    case PictStandardARGB32:
	depth = 32;
	break;
    case PictStandardA8:
	depth = 8;
	break;
    case PictStandardA4:
	depth = 4;
	break;
    case PictStandardA1:
	depth = 1;
	break;
    default:
	depth = 0;
	break;
    }
    if (!format)
	format = XRenderFindStandardFormat (xp->d, xp->planemask);
    
    pix = XCreatePixmap(xp->d, xp->w, WIDTH, HEIGHT, depth);
    pixPict = XRenderCreatePicture (xp->d, pix, format, 0, NULL);
    
    XRenderComposite (xp->d, PictOpClear,
		      winPict, None, pixPict,
		      0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
    
    XRenderFillRectangle (xp->d, PictOpSrc,
			  pixPict, &c, 0, 0, WIDTH, HEIGHT);
#if 1
    XRenderComposite (xp->d, PictOpSrc,
		      winPict, None, pixPict,
		      0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
#endif
    if (p->fillStyle) {
	XTransform		transform;
	memset (&transform, '\0', sizeof (transform));
	transform.matrix[0][0] = ((long long) 0x10000 * 0x10000) / p->fillStyle;
	transform.matrix[1][1] = ((long long) 0x10000 * 0x10000) / p->fillStyle;
	transform.matrix[2][2] = 0x10000;
	XRenderSetPictureTransform (xp->d, pixPict, &transform);
	XRenderSetPictureFilter (xp->d, pixPict, FilterBilinear, NULL, 0);
    }
    return reps;
}

void
EndCompositeWin (XParms xp, Parms p)
{
    if (winPict)
    {
	XRenderFreePicture (xp->d, winPict);
	winPict = None;
    }
    if (pixPict)
    {
	XRenderFreePicture (xp->d, pixPict);
	pixPict = None;
    }
}

static void 
CompositeArea(XParms xp, Parms p, int64_t reps, Picture src, Picture dst)
{
    int size;
    XSegment *sa, *sb;
    XSegment *sa2, *sb2;
    

    size = p->special;
    sa = segsa;
    sb = segsb;
    sa2 = segsa2 ? segsa2 : segsa;
    sb2 = segsb2 ? segsb2 : segsb;
    for (int i = 0; i < reps; i++) {
	XRenderComposite (xp->d, xp->func,
			  src, None, dst,
			  sa2->x1, sa2->y1, 0, 0, sa->x2, sa->y2, size, size);
	XRenderComposite (xp->d, xp->func,
			  src, None, dst,
			  sa2->x2, sa2->y2, 0, 0, sa->x1, sa->y1, size, size);
	XRenderComposite (xp->d, xp->func,
			  src, None, dst,
			  sb2->x2, sb2->y2, 0, 0, sb->x1, sb->y1, size, size);
	XRenderComposite (xp->d, xp->func,
			  src, None, dst,
			  sb2->x1, sb2->y1, 0, 0, sb->x2, sb->y2, size, size);
	CheckAbort ();
	sa++; sb++;
	sa2++; sb2++;
    }
}

void
DoCompositeWinWin (XParms xp, Parms p, int64_t reps)
{
    CompositeArea (xp, p, reps, winPict, winPict);
}

void
DoCompositePixWin (XParms xp, Parms p, int64_t reps)
{
    CompositeArea (xp, p, reps, pixPict, winPict);
}
