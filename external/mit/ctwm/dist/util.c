/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */


/***********************************************************************
 *
 * $XConsortium: util.c,v 1.47 91/07/14 13:40:37 rws Exp $
 *
 * utility routines for twm
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 * Changed behavior of DontMoveOff/MoveOffResistance to allow
 * moving a window off screen less than #MoveOffResistance pixels.
 * New code will no longer "snap" windows to #MoveOffResistance
 * pixels off screen and instead movements will just be stopped and
 * then resume once movement of #MoveOffResistance have been attempted.
 *
 * 15-December-02 Bjorn Knutsson 
 *
 ***********************************************************************/

#define LEVITTE_TEST

#include "twm.h"
#include "util.h"
#include "events.h"
#include "add_window.h"
#include "gram.tab.h"
#include "screen.h"
#include "icons.h"
#include "cursor.h"
#include <stdio.h>

/*
 * Need this for the fixed-size uint_*'s used below.  stdint.h would be
 * the more appropriate include, but there exist systems that don't have
 * it, but do have inttypes.h (FreeBSD 4, Solaris 7-9 I've heard of,
 * probably more).
 */
#include <inttypes.h>

#ifdef VMS
#include <decw$include/Xos.h>
#include <decw$include/Xatom.h>
#include <decw$include/Xutil.h>
#include <X11Xmu/Drawing.h>
#include <X11Xmu/CharSet.h>
#include <X11Xmu/WinUtil.h>
#ifdef HAVE_XWDFILE_H
#include "XWDFile.h"		/* We do some tricks, since the original
				   has bugs...		/Richard Levitte */
#endif /* HAVE_XWDFILE_H */
#include <unixlib.h>
#include <starlet.h>
#include <ssdef.h>
#include <psldef.h>
#include <lib$routines.h>
#ifdef __DECC
#include <unistd.h>
#endif /* __DECC */
#define USE_SIGNALS
#ifndef F_OK
#  define F_OK 0
#endif
#ifndef X_OK
#  define X_OK 1
#endif
#ifndef W_OK
#  define W_OK 2
#endif
#ifndef R_OK
#  define R_OK 4
#endif
#else /* !VMS */
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Drawing.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/XWDFile.h>
#endif /* VMS */

#if defined(USE_SIGNALS) && defined(__sgi)
#  define _BSD_SIGNALS
#endif

#include <signal.h>
#ifndef VMS
#include <sys/time.h>
#endif

#if defined (XPM)
#ifdef VMS
#include "xpm.h"
#else
#   include <X11/xpm.h>
#endif
#endif

#ifdef JPEG
# include <setjmp.h>
# include <jpeglib.h>
# include <jerror.h>
  static Image *LoadJpegImage (char *name);
  static Image *GetJpegImage  (char *name);

  struct jpeg_error {
    struct jpeg_error_mgr pub;
    sigjmp_buf setjmp_buffer;
  };

  typedef struct jpeg_error *jerr_ptr;
#endif /* JPEG */

#ifdef IMCONV
#   include "im.h"
#   include "sdsc.h"
#endif

#define MAXANIMATIONSPEED 20

extern Atom _XA_WM_WORKSPACESLIST;

static Image *LoadBitmapImage (char  *name, ColorPair cp);
static Image *GetBitmapImage  (char  *name, ColorPair cp);
#if !defined(VMS) || defined(HAVE_XWDFILE_H)
static Image *LoadXwdImage    (char  *filename, ColorPair cp);
static Image *GetXwdImage     (char  *name, ColorPair cp);
#endif
#ifdef XPM
static Image *LoadXpmImage    (char  *name, ColorPair cp);
static Image *GetXpmImage     (char  *name, ColorPair cp);
static void   xpmErrorMessage (int status, char *name, char *fullname);
#endif
#ifdef IMCONV
static Image *GetImconvImage  (char *filename,
			       unsigned int *widthp, unsigned int *heightp);
#endif
static Pixmap CreateXLogoPixmap(unsigned int *widthp, unsigned int *heightp);
static Pixmap CreateResizePixmap(unsigned int *widthp, unsigned int *heightp);
static Pixmap CreateQuestionPixmap(unsigned int *widthp, unsigned int *heightp);
static Pixmap CreateMenuPixmap(unsigned int *widthp, unsigned int *heightp);
static Pixmap CreateDotPixmap (unsigned int *widthp, unsigned int *heightp);
static Image  *Create3DMenuImage (ColorPair cp);
static Image  *Create3DDotImage (ColorPair cp);
static Image  *Create3DResizeImage (ColorPair cp);
static Image  *Create3DZoomImage (ColorPair cp);
static Image  *Create3DBarImage (ColorPair cp);
static Image  *Create3DVertBarImage (ColorPair cp);
static Image  *Create3DResizeAnimation (Bool in, Bool left, Bool top,
					ColorPair cp);
static Image  *Create3DCrossImage (ColorPair cp);
static Image  *Create3DIconifyImage (ColorPair cp);
static Image  *Create3DSunkenResizeImage (ColorPair cp);
static Image  *Create3DBoxImage (ColorPair cp);

extern FILE *tracefile;

void FreeImage (Image *image);

void _swapshort (register char *bp, register unsigned n);
void _swaplong (register char *bp, register unsigned n);

static int    reportfilenotfound = 1;
static Colormap AlternateCmap = None;

int  HotX, HotY;

int  AnimationSpeed   = 0;
Bool AnimationActive  = False;
Bool MaybeAnimate     = True;
#ifdef USE_SIGNALS
   Bool AnimationPending = False;
#else
   struct timeval AnimateTimeout;
#endif /* USE_SIGNALS */

/***********************************************************************
 *
 *  Procedure:
 *	MoveOutline - move a window outline
 *
 *  Inputs:
 *	root	    - the window we are outlining
 *	x	    - upper left x coordinate
 *	y	    - upper left y coordinate
 *	width	    - the width of the rectangle
 *	height	    - the height of the rectangle
 *      bw          - the border width of the frame
 *      th          - title height
 *
 ***********************************************************************
 */

/* ARGSUSED */
void MoveOutline(Window root,
		 int x, int y, int width, int height, int bw, int th)
{
    static int	lastx = 0;
    static int	lasty = 0;
    static int	lastWidth = 0;
    static int	lastHeight = 0;
    static int	lastBW = 0;
    static int	lastTH = 0;
    int		xl, xr, yt, yb, xinnerl, xinnerr, yinnert, yinnerb;
    int		xthird, ythird;
    XSegment	outline[18];
    register XSegment	*r;

    if (x == lastx && y == lasty && width == lastWidth && height == lastHeight
	&& lastBW == bw && th == lastTH)
	return;
    
    r = outline;

#define DRAWIT() \
    if (lastWidth || lastHeight)			\
    {							\
	xl = lastx;					\
	xr = lastx + lastWidth - 1;			\
	yt = lasty;					\
	yb = lasty + lastHeight - 1;			\
	xinnerl = xl + lastBW;				\
	xinnerr = xr - lastBW;				\
	yinnert = yt + lastTH + lastBW;			\
	yinnerb = yb - lastBW;				\
	xthird = (xinnerr - xinnerl) / 3;		\
	ythird = (yinnerb - yinnert) / 3;		\
							\
	r->x1 = xl;					\
	r->y1 = yt;					\
	r->x2 = xr;					\
	r->y2 = yt;					\
	r++;						\
							\
	r->x1 = xl;					\
	r->y1 = yb;					\
	r->x2 = xr;					\
	r->y2 = yb;					\
	r++;						\
							\
	r->x1 = xl;					\
	r->y1 = yt;					\
	r->x2 = xl;					\
	r->y2 = yb;					\
	r++;						\
							\
	r->x1 = xr;					\
	r->y1 = yt;					\
	r->x2 = xr;					\
	r->y2 = yb;					\
	r++;						\
							\
	r->x1 = xinnerl + xthird;			\
	r->y1 = yinnert;				\
	r->x2 = r->x1;					\
	r->y2 = yinnerb;				\
	r++;						\
							\
	r->x1 = xinnerl + (2 * xthird);			\
	r->y1 = yinnert;				\
	r->x2 = r->x1;					\
	r->y2 = yinnerb;				\
	r++;						\
							\
	r->x1 = xinnerl;				\
	r->y1 = yinnert + ythird;			\
	r->x2 = xinnerr;				\
	r->y2 = r->y1;					\
	r++;						\
							\
	r->x1 = xinnerl;				\
	r->y1 = yinnert + (2 * ythird);			\
	r->x2 = xinnerr;				\
	r->y2 = r->y1;					\
	r++;						\
							\
	if (lastTH != 0) {				\
	    r->x1 = xl;					\
	    r->y1 = yt + lastTH;			\
	    r->x2 = xr;					\
	    r->y2 = r->y1;				\
	    r++;					\
	}						\
    }

    /* undraw the old one, if any */
    DRAWIT ();

    lastx = x;
    lasty = y;
    lastWidth = width;
    lastHeight = height;
    lastBW = bw;
    lastTH = th;

    /* draw the new one, if any */
    DRAWIT ();

#undef DRAWIT


    if (r != outline)
    {
	XDrawSegments(dpy, root, Scr->DrawGC, outline, r - outline);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	Zoom - zoom in or out of an icon
 *
 *  Inputs:
 *	wf	- window to zoom from
 *	wt	- window to zoom to
 *
 ***********************************************************************
 */

void Zoom(Window wf, Window wt)
{
    int fx, fy, tx, ty;			/* from, to */
    unsigned int fw, fh, tw, th;	/* from, to */
    long dx, dy, dw, dh;
    long z;
    int j;

    if ((Scr->IconifyStyle != ICONIFY_NORMAL) || !Scr->DoZoom || Scr->ZoomCount < 1) return;

    if (wf == None || wt == None) return;

    XGetGeometry (dpy, wf, &JunkRoot, &fx, &fy, &fw, &fh, &JunkBW, &JunkDepth);
    XGetGeometry (dpy, wt, &JunkRoot, &tx, &ty, &tw, &th, &JunkBW, &JunkDepth);

    dx = (long) tx - (long) fx;	/* going from -> to */
    dy = (long) ty - (long) fy;	/* going from -> to */
    dw = (long) tw - (long) fw;	/* going from -> to */
    dh = (long) th - (long) fh;	/* going from -> to */
    z = (long) (Scr->ZoomCount + 1);

    for (j = 0; j < 2; j++) {
	long i;

	XDrawRectangle (dpy, Scr->Root, Scr->DrawGC, fx, fy, fw, fh);
	for (i = 1; i < z; i++) {
	    int x = fx + (int) ((dx * i) / z);
	    int y = fy + (int) ((dy * i) / z);
	    unsigned width = (unsigned) (((long) fw) + (dw * i) / z);
	    unsigned height = (unsigned) (((long) fh) + (dh * i) / z);
	
	    XDrawRectangle (dpy, Scr->Root, Scr->DrawGC,
			    x, y, width, height);
	}
	XDrawRectangle (dpy, Scr->Root, Scr->DrawGC, tx, ty, tw, th);
    }
}


char *ExpandFilePath (char *path)
{
    char *ret, *colon, *p;
    int  len;

    len = 0;
    p   = path;
    while ((colon = strchr (p, ':'))) {
	len += colon - p + 1;
	if (*p == '~') len += HomeLen - 1;
	p = colon + 1;
    }
    if (*p == '~') len += HomeLen - 1;
    len += strlen (p);
    ret = (char*) malloc (len + 1);
    *ret = 0;

    p   = path;
    while ((colon = strchr (p, ':'))) {
	*colon = '\0';
	if (*p == '~') {
	    strcat (ret, Home);
	    strcat (ret, p + 1);
	} else {
	    strcat (ret, p);
	}
	*colon = ':';
	strcat (ret, ":");
	p = colon + 1;
    }
    if (*p == '~') {
	strcat (ret, Home);
	strcat (ret, p + 1);
    } else {
	strcat (ret, p);
    }
    return ret;
}

/***********************************************************************
 *
 *  Procedure:
 *	ExpandFilename - expand the tilde character to HOME
 *		if it is the first character of the filename
 *
 *  Returned Value:
 *	a pointer to the new name
 *
 *  Inputs:
 *	name	- the filename to expand
 *
 ***********************************************************************
 */

char *ExpandFilename(char *name)
{
    char *newname;

    if (name[0] != '~') return name;

#ifdef VMS
    newname = (char *) malloc (HomeLen + strlen(name) + 1);
    if (!newname) {
        fprintf (stderr, 
 		 "%s:  unable to allocate %d bytes to expand filename %s%s\n",
 		 ProgramName, HomeLen + strlen(name) + 1, Home, &name[1]);
    } else {
        (void) sprintf (newname, "%s%s", Home, &name[1]);
    }
#else
    newname = (char *) malloc (HomeLen + strlen(name) + 2);
    if (!newname) {
	fprintf (stderr, 
		 "%s:  unable to allocate %lu bytes to expand filename %s/%s\n",
		 ProgramName, (unsigned long) HomeLen + strlen(name) + 2,
		 Home, &name[1]);
    } else {
	(void) sprintf (newname, "%s/%s", Home, &name[1]);
    }
#endif

    return newname;
}

char *ExpandPixmapPath (char *name)
{
    char    *ret, *colon;

    ret = NULL;
#ifdef VMS
    if (name[0] == '~') {
	ret = (char *) malloc (HomeLen + strlen (name) + 1);
	sprintf (ret, "%s%s", Home, &name[1]);
    }
    if (name[0] == '/') {
	ret = (char *) malloc (strlen (name));
	sprintf (ret, "%s", &name[1]);
    }
    else
    if (Scr->PixmapDirectory) {
	char *p = Scr->PixmapDirectory;
	while (colon = strchr (p, ':')) {
	    *colon = '\0';
	    ret = (char *) malloc (strlen (p) + strlen (name) + 1);
	    sprintf (ret, "%s%s", p, name);
	    *colon = ':';
	    if (!access (ret, R_OK)) return (ret);
	    p = colon + 1;
	}
        ret = (char *) malloc (strlen (Scr->PixmapDirectory) + strlen (name) + 1);
	sprintf (ret, "%s%s", Scr->PixmapDirectory, name);
    }
#else
    if (name[0] == '~') {
	ret = (char *) malloc (HomeLen + strlen (name) + 2);
	sprintf (ret, "%s/%s", Home, &name[1]);
    }
    else
    if (name[0] == '/') {
	ret = (char *) malloc (strlen (name) + 1);
	strcpy (ret, name);
    }
    else
    if (Scr->PixmapDirectory) {
	char *p = Scr->PixmapDirectory;
	while ((colon = strchr (p, ':'))) {
	    *colon = '\0';
	    ret = (char *) malloc (strlen (p) + strlen (name) + 2);
	    sprintf (ret, "%s/%s", p, name);
	    *colon = ':';
	    if (!access (ret, R_OK)) return (ret);
	    p = colon + 1;
	}
	ret = (char *) malloc (strlen (p) + strlen (name) + 2);
	sprintf (ret, "%s/%s", p, name);
    }
#endif
    return (ret);
}

/***********************************************************************
 *
 *  Procedure:
 *	GetUnknownIcon - read in the bitmap file for the unknown icon
 *
 *  Inputs:
 *	name - the filename to read
 *
 ***********************************************************************
 */

void GetUnknownIcon(char *name)
{
    Scr->UnknownImage = GetImage (name, Scr->IconC);
}

/***********************************************************************
 *
 *  Procedure:
 *	FindBitmap - read in a bitmap file and return size
 *
 *  Returned Value:
 *	the pixmap associated with the bitmap
 *      widthp	- pointer to width of bitmap
 *      heightp	- pointer to height of bitmap
 *
 *  Inputs:
 *	name	- the filename to read
 *
 ***********************************************************************
 */

Pixmap FindBitmap (char *name, unsigned int *widthp, unsigned int *heightp)
{
    char *bigname;
    Pixmap pm;

    if (!name) return None;

    /*
     * Names of the form :name refer to hardcoded images that are scaled to
     * look nice in title buttons.  Eventually, it would be nice to put in a
     * menu symbol as well....
     */
    if (name[0] == ':') {
	int i;
	static struct {
	    char *name;
	    Pixmap (*proc)(unsigned int *wp, unsigned int *hp);
	} pmtab[] = {
	    { TBPM_DOT,		CreateDotPixmap },
	    { TBPM_ICONIFY,	CreateDotPixmap },
	    { TBPM_RESIZE,	CreateResizePixmap },
	    { TBPM_XLOGO,	CreateXLogoPixmap },
	    { TBPM_DELETE,	CreateXLogoPixmap },
	    { TBPM_MENU,	CreateMenuPixmap },
	    { TBPM_QUESTION,	CreateQuestionPixmap },
	};
	
	for (i = 0; i < (sizeof pmtab)/(sizeof pmtab[0]); i++) {
	    if (XmuCompareISOLatin1 (pmtab[i].name, name) == 0)
	      return (*pmtab[i].proc) (widthp, heightp);
	}
	fprintf (stderr, "%s:  no such built-in bitmap \"%s\"\n",
		 ProgramName, name);
	return None;
    }

    /*
     * Generate a full pathname if any special prefix characters (such as ~)
     * are used.  If the bigname is different from name, bigname will need to
     * be freed.
     */
    bigname = ExpandFilename (name);
    if (!bigname) return None;

    /*
     * look along bitmapFilePath resource same as toolkit clients
     */
    pm = XmuLocateBitmapFile (ScreenOfDisplay(dpy, Scr->screen), bigname, NULL,
			      0, (int *)widthp, (int *)heightp, &HotX, &HotY);
    if (pm == None && Scr->IconDirectory && bigname[0] != '/') {
	if (bigname != name) free (bigname);
	/*
	 * Attempt to find icon in old IconDirectory (now obsolete)
	 */
#ifdef VMS
	bigname = (char *) malloc (strlen(name) + strlen(Scr->IconDirectory) + 1);
	if (!bigname) {
	    fprintf (stderr,
		"%s:  unable to allocate memory for \"%s%s\"\n",
		ProgramName, Scr->IconDirectory, name);
	    return None;
	}
	(void) sprintf (bigname, "%s%s", Scr->IconDirectory, name);
#else
	bigname = (char *) malloc (strlen(name) + strlen(Scr->IconDirectory) + 2);
	if (!bigname) {
	    fprintf (stderr,
		     "%s:  unable to allocate memory for \"%s/%s\"\n",
		     ProgramName, Scr->IconDirectory, name);
	    return None;
	}
	(void) sprintf (bigname, "%s/%s", Scr->IconDirectory, name);
#endif
	if (XReadBitmapFile (dpy, Scr->Root, bigname, widthp, heightp, &pm,
			     &HotX, &HotY) != BitmapSuccess) {
	    pm = None;
	}
    }
    if (bigname != name) free (bigname);
    if ((pm == None) && reportfilenotfound) {
	fprintf (stderr, "%s:  unable to find bitmap \"%s\"\n", ProgramName, name);
    }

    return pm;
}

Pixmap GetBitmap (char *name)
{
    return FindBitmap (name, &JunkWidth, &JunkHeight);
}

static Image *LoadBitmapImage (char  *name, ColorPair cp)
{
    Image	 *image;
    Pixmap	 bm;
    unsigned int width, height;
    XGCValues	 gcvalues;

    if (Scr->rootGC == (GC) 0) Scr->rootGC = XCreateGC (dpy, Scr->Root, 0, &gcvalues);
    bm = FindBitmap (name, &width, &height);
    if (bm == None) return (None);

    image = (Image*) malloc (sizeof (struct _Image));
    image->pixmap = XCreatePixmap (dpy, Scr->Root, width, height, Scr->d_depth);
    gcvalues.background = cp.back;
    gcvalues.foreground = cp.fore;
    XChangeGC   (dpy, Scr->rootGC, GCForeground | GCBackground, &gcvalues);
    XCopyPlane  (dpy, bm, image->pixmap, Scr->rootGC, 0, 0, width, height,
		0, 0, (unsigned long) 1);
    XFreePixmap (dpy, bm);
    image->mask   = None;
    image->width  = width;
    image->height = height;
    image->next   = None;
    return (image);
}

static Image *GetBitmapImage (char  *name, ColorPair cp)
{
    Image	*image, *r, *s;
    char	path [128], pref [128];
    char	*perc;
    int		i;

    if (! strchr (name, '%')) return (LoadBitmapImage (name, cp));
    s = image = None;
    strcpy (pref, name);
    perc  = strchr (pref, '%');
    *perc = '\0';
    reportfilenotfound = 0;
    for (i = 1;; i++) {
	sprintf (path, "%s%d%s", pref, i, perc + 1);
	r = LoadBitmapImage (path, cp);
	if (r == None) break;
	r->next = None;
	if (image == None) s = image = r;
	else {
	    s->next = r;
	    s = r;
	}
    }
    reportfilenotfound = 1;
    if (s != None) s->next = image;
    if (image == None) {
	fprintf (stderr, "Cannot open any %s bitmap file\n", name);
    }
    return (image);
}

#ifdef XPM
static int reportxpmerror = 1;

static Image *LoadXpmImage (char *name, ColorPair cp)
{
    char	*fullname;
    Image	*image;
    int		status;
    Colormap    stdcmap = Scr->RootColormaps.cwins[0]->colormap->c;
    XpmAttributes attributes;
    static XpmColorSymbol overrides[] = {
	{"Foreground", NULL, 0},
	{"Background", NULL, 0},
	{"HiShadow", NULL, 0},
	{"LoShadow", NULL, 0}
    };

    fullname = ExpandPixmapPath (name);
    if (! fullname) return (None);

    image = (Image*) malloc (sizeof (struct _Image));
    if (image == None) return (None);

    attributes.valuemask  = 0;
    attributes.valuemask |= XpmSize;
    attributes.valuemask |= XpmReturnPixels;
    attributes.valuemask |= XpmColormap;
    attributes.valuemask |= XpmDepth;
    attributes.valuemask |= XpmVisual;
    attributes.valuemask |= XpmCloseness;
    attributes.valuemask |= XpmColorSymbols;

    attributes.numsymbols = 4;
    attributes.colorsymbols = overrides;
    overrides[0].pixel = cp.fore;
    overrides[1].pixel = cp.back;
    overrides[2].pixel = cp.shadd;
    overrides[3].pixel = cp.shadc;


    attributes.colormap  = AlternateCmap ? AlternateCmap : stdcmap;
    attributes.depth     = Scr->d_depth;
    attributes.visual    = Scr->d_visual;
    attributes.closeness = 65535; /* Never fail */
    status = XpmReadFileToPixmap(dpy, Scr->Root, fullname,
				 &(image->pixmap), &(image->mask), &attributes);
    if (status != XpmSuccess) {
	xpmErrorMessage (status, name, fullname);
	free (image);
	return (None);
    }
    free (fullname);
    image->width  = attributes.width;
    image->height = attributes.height;
    image->next   = None;
    return (image);
}

static Image *GetXpmImage (char *name, ColorPair cp)
{
    char    path [128], pref [128];
    Image   *image, *r, *s;
    char    *perc;
    int     i;

    if (! strchr (name, '%')) return (LoadXpmImage (name, cp));
    s = image = None;
    strcpy (pref, name);
    perc  = strchr (pref, '%');
    *perc = '\0';
    reportfilenotfound = 0;
    for (i = 1;; i++) {
	sprintf (path, "%s%d%s", pref, i, perc + 1);
	r = LoadXpmImage (path, cp);
	if (r == None) break;
	r->next = None;
	if (image == None) s = image = r;
	else {
	    s->next = r;
	    s = r;
	}
    }
    reportfilenotfound = 1;
    if (s != None) s->next = image;
    if (image == None) {
	fprintf (stderr, "Cannot open any %s XPM file\n", name);
    }
    return (image);
}

static void xpmErrorMessage (int status, char *name, char *fullname)
{
    switch (status) {
	case XpmSuccess:
	    break;

	case XpmColorError:
	    if (reportxpmerror)
		fprintf (stderr,
			"Could not parse or alloc requested color : %s\n",
			fullname);
	    return;

	case XpmOpenFailed:
	    if (reportxpmerror && reportfilenotfound)
		fprintf (stderr, "unable to locate XPM file : %s\n", fullname);
	    return;

	case XpmFileInvalid:
	    fprintf (stderr, "invalid XPM file : %s\n", fullname);
	    return;

	case XpmNoMemory:
	    if (reportxpmerror)
		fprintf (stderr, "Not enough memory for XPM file : %s\n", fullname);
	    return;

	case XpmColorFailed:
	    if (reportxpmerror)
		fprintf (stderr, "Color not found in : %s\n", fullname);
	    return;

	default :
	    fprintf (stderr, "Unknown error in : %s\n", fullname);
	    return;
    }
}

#endif

void MaskScreen (char *file)
{
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    XEvent event;
    Cursor waitcursor;
    int x, y;
    ColorPair WelcomeCp;
    XColor black;

    NewFontCursor (&waitcursor, "watch");

    valuemask = (CWBackingStore | CWSaveUnder | CWBackPixel |
		 CWOverrideRedirect | CWEventMask | CWCursor);
    attributes.backing_store	 = NotUseful;
    attributes.save_under	 = False;
    attributes.override_redirect = True;
    attributes.event_mask	 = ExposureMask;
    attributes.cursor		 = waitcursor;
    attributes.background_pixel	 = Scr->Black;
    Scr->WindowMask = XCreateWindow (dpy, Scr->Root, 0, 0,
			(unsigned int) Scr->rootw,
			(unsigned int) Scr->rooth,
			(unsigned int) 0,
			CopyFromParent, (unsigned int) CopyFromParent,
			(Visual *) CopyFromParent, valuemask,
			&attributes);
    XMapWindow (dpy, Scr->WindowMask);
    XMaskEvent (dpy, ExposureMask, &event);

    if (Scr->Monochrome != COLOR) return;

    WelcomeCp.fore = Scr->Black;
    WelcomeCp.back = Scr->White;
    Scr->WelcomeCmap  = XCreateColormap (dpy, Scr->WindowMask, Scr->d_visual, AllocNone);
    if (! Scr->WelcomeCmap) return;
    XSetWindowColormap (dpy, Scr->WindowMask, Scr->WelcomeCmap);
    black.red   = 0;
    black.green = 0;
    black.blue  = 0;
    XAllocColor (dpy, Scr->WelcomeCmap, &black);

    reportfilenotfound = 0;
    AlternateCmap = Scr->WelcomeCmap;
    if (! file) {
	Scr->WelcomeImage  = GetImage ("xwd:welcome.xwd", WelcomeCp);
#ifdef XPM
	if (Scr->WelcomeImage == None)
		Scr->WelcomeImage  = GetImage ("xpm:welcome.xpm", WelcomeCp);
#endif
    }
    else {
	Scr->WelcomeImage  = GetImage (file, WelcomeCp);
    }
    AlternateCmap = None;
    reportfilenotfound = 1;
    if (Scr->WelcomeImage == None) return;

    if (captive) {
	XSetWindowColormap (dpy, Scr->WindowMask, Scr->WelcomeCmap);
	XSetWMColormapWindows (dpy, Scr->Root, &(Scr->WindowMask), 1);
    }
    else XInstallColormap (dpy, Scr->WelcomeCmap);

    Scr->WelcomeGC = XCreateGC (dpy, Scr->WindowMask, 0, NULL);
    x = (Scr->rootw  -  Scr->WelcomeImage->width) / 2;
    y = (Scr->rooth - Scr->WelcomeImage->height) / 2;

    XSetWindowBackground (dpy, Scr->WindowMask, black.pixel);
    XClearWindow (dpy, Scr->WindowMask);
    XCopyArea (dpy, Scr->WelcomeImage->pixmap, Scr->WindowMask, Scr->WelcomeGC, 0, 0,
		Scr->WelcomeImage->width, Scr->WelcomeImage->height, x, y);
}

void UnmaskScreen (void)
{
#ifdef VMS
    float  timeout;
#else
    struct timeval	timeout;
#endif
    Colormap            stdcmap = Scr->RootColormaps.cwins[0]->colormap->c;
    Colormap		cmap;
    XColor		colors [256], stdcolors [256];
    int			i, j, usec;

#ifdef VMS
    timeout = 0.017;
#else
    usec = 10000;
    timeout.tv_usec = usec % (unsigned long) 1000000;
    timeout.tv_sec  = usec / (unsigned long) 1000000;
#endif

    if (Scr->WelcomeImage) {
	Pixel pixels [256];

	cmap = Scr->WelcomeCmap;
	for (i = 0; i < 256; i++) {
	    pixels [i] = i;
	    colors [i].pixel = i;
	}
	XQueryColors (dpy, cmap, colors, 256);
	XFreeColors  (dpy, cmap, pixels, 256, 0L);
	XFreeColors  (dpy, cmap, pixels, 256, 0L); /* Ah Ah */

	for (i = 0; i < 256; i++) {
	    colors [i].pixel = i;
	    colors [i].flags = DoRed | DoGreen | DoBlue;
	    stdcolors [i].red   = colors [i].red;
	    stdcolors [i].green = colors [i].green;
	    stdcolors [i].blue  = colors [i].blue;
	}
	for (i = 0; i < 128; i++) {
	    for (j = 0; j < 256; j++) {
		colors [j].red   = stdcolors [j].red   * ((127.0 - i) / 128.0);
		colors [j].green = stdcolors [j].green * ((127.0 - i) / 128.0);
		colors [j].blue  = stdcolors [j].blue  * ((127.0 - i) / 128.0);
	    }
	    XStoreColors (dpy, cmap, colors, 256);
#ifdef VMS
	    lib$wait(&timeout);
#else
	    select (0, (void *) 0, (void *) 0, (void *) 0, &timeout);
#endif
	}
	XFreeColors   (dpy, cmap, pixels, 256, 0L);
	XFreeGC       (dpy, Scr->WelcomeGC);
	FreeImage     (Scr->WelcomeImage);
    }
    if (Scr->Monochrome != COLOR) goto fin;

    cmap = XCreateColormap (dpy, Scr->Root, Scr->d_visual, AllocNone);
    if (! cmap) goto fin;
    for (i = 0; i < 256; i++) {
	colors [i].pixel = i;
	colors [i].red   = 0;
	colors [i].green = 0;
	colors [i].blue  = 0;
	colors [i].flags = DoRed | DoGreen | DoBlue;
    }
    XStoreColors (dpy, cmap, colors, 256);

    if (captive) XSetWindowColormap (dpy, Scr->Root, cmap);
    else XInstallColormap (dpy, cmap);

    XUnmapWindow (dpy, Scr->WindowMask);
    XClearWindow (dpy, Scr->Root);
    XSync (dpy, 0);
    PaintAllDecoration ();

    for (i = 0; i < 256; i++) stdcolors [i].pixel = i;
    XQueryColors (dpy, stdcmap, stdcolors, 256);
    for (i = 0; i < 128; i++) {
	for (j = 0; j < 256; j++) {
	    colors [j].pixel = j;
	    colors [j].red   = stdcolors [j].red   * (i / 127.0);
	    colors [j].green = stdcolors [j].green * (i / 127.0);
	    colors [j].blue  = stdcolors [j].blue  * (i / 127.0);
	    colors [j].flags = DoRed | DoGreen | DoBlue;
	}
	XStoreColors (dpy, cmap, colors, 256);
#ifdef VMS
        lib$wait(&timeout);
#else
	select (0, (void *) 0, (void *) 0, (void *) 0, &timeout);
#endif
    }

    if (captive) XSetWindowColormap (dpy, Scr->Root, stdcmap);
    else XInstallColormap (dpy, stdcmap);

    XFreeColormap (dpy, cmap);

fin:
    if (Scr->WelcomeCmap) XFreeColormap (dpy, Scr->WelcomeCmap);
    XDestroyWindow (dpy, Scr->WindowMask);
    Scr->WindowMask = (Window) 0;
}

#ifdef VMS

/* use the VMS system services to request the timer to issue an AST */
void AnimateHandler (void);

unsigned int tv[2];
int status;
static unsigned long timefi;
/* unsigned long timefe = 17; */
unsigned long timefe;

#define TIMID 12L

void StartAnimation (void)
{
    if (AnimationSpeed > MAXANIMATIONSPEED) AnimationSpeed = MAXANIMATIONSPEED;
    if (AnimationSpeed <= 0) return;
    if (AnimationActive) return;

    if (!timefi) lib$get_ef(&timefi);
    if (!timefe) lib$get_ef(&timefe);

    tv[1] = 0xFFFFFFFF;                   /* quadword negative for relative */
    tv[0] = -(10000000 / AnimationSpeed); /* time. In units of 100ns. */
    sys$clref(timefe);
    status = sys$setimr (timefi, &tv, AnimateHandler, TIMID );
    if (status != SS$_NORMAL) lib$signal(status);
    AnimationActive = True;
}

void StopAnimation () {
    if (AnimationSpeed <= 0) return;
    if (! AnimationActive) return;
    AnimationActive = False;

    status = sys$cantim(TIMID, PSL$C_USER);
    if (status != SS$_NORMAL) lib$signal(status);
}

void SetAnimationSpeed (int speed)
{
    AnimationSpeed = speed;
    if (AnimationSpeed > MAXANIMATIONSPEED) AnimationSpeed = MAXANIMATIONSPEED;
}

void ModifyAnimationSpeed (int incr)
{
    if ((AnimationSpeed + incr) < 0) return;
    if ((AnimationSpeed + incr) == 0) {
	if (AnimationActive) StopAnimation ();
	AnimationSpeed = 0;
	return;
    }
    AnimationSpeed += incr;

    status = sys$cantim(TIMID, PSL$C_USER);
    if (status != SS$_NORMAL) lib$signal(status);

    tv[1] = 0xFFFFFFFF;
    tv[0] = -(10000000 / AnimationSpeed);

    sys$clref(timefe);
    status = sys$setimr (timefi, &tv, AnimateHandler, TIMID);
    if (status != SS$_NORMAL) lib$signal(status);

    AnimationActive = True;
}

void AnimateHandler (void) {
    AnimationPending = True;

    sys$setef(timefe);
    status = sys$setimr (timefi, &tv, AnimateHandler, TIMID);
    if (status != SS$_NORMAL) lib$signal(status);
}
#else /* VMS */

#ifdef USE_SIGNALS
SIGNAL_T AnimateHandler ();
#endif

#ifndef USE_SIGNALS
void TryToAnimate (void)
{
    struct timeval  tp;
    struct timezone tzp;
    static unsigned long lastsec;
    static long lastusec;
    unsigned long gap;

    gettimeofday (&tp, &tzp);
    gap = ((tp.tv_sec - lastsec) * 1000000) + (tp.tv_usec - lastusec);
    if (tracefile) {
	fprintf (tracefile, "Time = %lu, %ld, %ld, %ld, %lu\n", lastsec,
		lastusec, (long)tp.tv_sec, (long)tp.tv_usec, gap);
	fflush (tracefile);
    }
    gap *= AnimationSpeed;
    if (gap < 1000000) return;
    if (tracefile) {
	fprintf (tracefile, "Animate\n");
	fflush (tracefile);
    }
    Animate ();
    lastsec  = tp.tv_sec;
    lastusec = tp.tv_usec;
}
#endif /* USE_SIGNALS */

void StartAnimation (void)
{
#ifdef USE_SIGNALS
    struct itimerval tv;
#endif

    if (AnimationSpeed > MAXANIMATIONSPEED) AnimationSpeed = MAXANIMATIONSPEED;
    if (AnimationSpeed <= 0) AnimationSpeed = 0;
    if (AnimationActive) return;
#ifdef USE_SIGNALS
    if (AnimationSpeed == 0) return;
    signal (SIGALRM, AnimateHandler);
    if (AnimationSpeed == 1) {
	tv.it_interval.tv_sec  = 1;
	tv.it_interval.tv_usec = 0;
	tv.it_value.tv_sec     = 1;
	tv.it_value.tv_usec    = 0;
    }
    else {
	tv.it_interval.tv_sec  = 0;
	tv.it_interval.tv_usec = 1000000 / AnimationSpeed;
	tv.it_value.tv_sec     = 0;
	tv.it_value.tv_usec    = 1000000 / AnimationSpeed;
    }
    setitimer (ITIMER_REAL, &tv, (struct itimerval*) NULL);
#else /* USE_SIGNALS */
    switch (AnimationSpeed) {
	case 0 :
	    return;
	case 1 :
	    AnimateTimeout.tv_sec  = 1;
	    AnimateTimeout.tv_usec = 0;
	    break;
	default :
	    AnimateTimeout.tv_sec  = 0;
	    AnimateTimeout.tv_usec = 1000000 / AnimationSpeed;
    }
#endif /* USE_SIGNALS */
    AnimationActive = True;
}

void StopAnimation (void)
{
#ifdef USE_SIGNALS
    struct itimerval tv;

    if (AnimationSpeed <= 0) return;
    if (! AnimationActive) return;
    signal (SIGALRM, SIG_IGN);

    tv.it_value.tv_sec     = 0;
    tv.it_value.tv_usec    = 0;
    setitimer (ITIMER_REAL, &tv, (struct itimerval*) NULL);
#endif
    AnimationActive = False;
}

void SetAnimationSpeed (int speed)
{
    AnimationSpeed = speed;
    if (AnimationSpeed > MAXANIMATIONSPEED) AnimationSpeed = MAXANIMATIONSPEED;
}

void ModifyAnimationSpeed (int incr)
{
#ifdef USE_SIGNALS
    struct itimerval tv;
#endif

    if ((AnimationSpeed + incr) < 0) return;
    if ((AnimationSpeed + incr) == 0) {
	if (AnimationActive) StopAnimation ();
	AnimationSpeed = 0;
	return;
    }
    AnimationSpeed += incr;
    if (AnimationSpeed > MAXANIMATIONSPEED) AnimationSpeed = MAXANIMATIONSPEED;

#ifdef USE_SIGNALS
    signal (SIGALRM, AnimateHandler);
    if (AnimationSpeed == 1) {
	tv.it_interval.tv_sec  = 1;
	tv.it_interval.tv_usec = 0;
	tv.it_value.tv_sec     = 1;
	tv.it_value.tv_usec    = 0;
    }
    else {
	tv.it_interval.tv_sec  = 0;
	tv.it_interval.tv_usec = 1000000 / AnimationSpeed;
	tv.it_value.tv_sec     = 0;
	tv.it_value.tv_usec    = 1000000 / AnimationSpeed;
    }
    setitimer (ITIMER_REAL, &tv, (struct itimerval*) NULL);
#else /* USE_SIGNALS */
    if (AnimationSpeed == 1) {
	AnimateTimeout.tv_sec  = 1;
	AnimateTimeout.tv_usec = 0;
    }
    else {
	AnimateTimeout.tv_sec  = 0;
	AnimateTimeout.tv_usec = 1000000 / AnimationSpeed;
    }
#endif /* USE_SIGNALS */
    AnimationActive = True;
}

#ifdef USE_SIGNALS
SIGNAL_T AnimateHandler (int dummy)
{
    signal (SIGALRM, AnimateHandler);
    AnimationPending = True;
}
#endif
#endif /* VMS */

void Animate (void)
{
    TwmWindow	*t;
    int		scrnum;
    ScreenInfo	*scr;
    int		i;
    TBWindow	*tbw;
    int		nb;

    if (AnimationSpeed == 0) return;
#ifdef USE_SIGNALS
    AnimationPending = False;
#endif

    MaybeAnimate = False;
    for (scrnum = 0; scrnum < NumScreens; scrnum++) {
	if ((scr = ScreenList [scrnum]) == NULL) continue;

	for (t = scr->FirstWindow; t != NULL; t = t->next) {
	    if (! visible (t)) continue;
	    if (t->icon_on && t->icon && t->icon->bm_w && t->icon->image &&
		t->icon->image->next) {
		AnimateIcons (scr, t->icon);
		MaybeAnimate = True;
	    }
	    else
	    if (t->mapped && t->titlebuttons) {
		nb = scr->TBInfo.nleft + scr->TBInfo.nright;
		for (i = 0, tbw = t->titlebuttons; i < nb; i++, tbw++) {
		    if (tbw->image && tbw->image->next) {
			AnimateButton (tbw);
			MaybeAnimate = True;
		    }
		}
	    }
	}
	if (scr->Focus) {
	    t = scr->Focus;
	    if (t->mapped && t->titlehighlight && t->title_height &&
		t->HiliteImage && t->HiliteImage->next) {
		AnimateHighlight (t);
		MaybeAnimate = True;
	    }
	}
    }
    MaybeAnimate |= AnimateRoot ();
    XFlush (dpy);
    return;
}

void InsertRGBColormap (Atom a, XStandardColormap *maps, int nmaps,
			Bool replace)
{
    StdCmap *sc = NULL;

    if (replace) {			/* locate existing entry */
	for (sc = Scr->StdCmapInfo.head; sc; sc = sc->next) {
	    if (sc->atom == a) break;
	}
    }

    if (!sc) {				/* no existing, allocate new */
	sc = (StdCmap *) malloc (sizeof (StdCmap));
	if (!sc) {
	    fprintf (stderr, "%s:  unable to allocate %lu bytes for StdCmap\n",
		     ProgramName, (unsigned long) sizeof(StdCmap));
	    return;
	}
    }

    if (replace) {			/* just update contents */
	if (sc->maps) XFree ((char *) maps);
	if (sc == Scr->StdCmapInfo.mru) Scr->StdCmapInfo.mru = NULL;
    } else {				/* else appending */
	sc->next = NULL;
	sc->atom = a;
	if (Scr->StdCmapInfo.tail) {
	    Scr->StdCmapInfo.tail->next = sc;
	} else {
	    Scr->StdCmapInfo.head = sc;
	}
	Scr->StdCmapInfo.tail = sc;
    }
    sc->nmaps = nmaps;
    sc->maps = maps;

    return;
}

void RemoveRGBColormap (Atom a)
{
    StdCmap *sc, *prev;

    prev = NULL;
    for (sc = Scr->StdCmapInfo.head; sc; sc = sc->next) {  
	if (sc->atom == a) break;
	prev = sc;
    }
    if (sc) {				/* found one */
	if (sc->maps) XFree ((char *) sc->maps);
	if (prev) prev->next = sc->next;
	if (Scr->StdCmapInfo.head == sc) Scr->StdCmapInfo.head = sc->next;
	if (Scr->StdCmapInfo.tail == sc) Scr->StdCmapInfo.tail = prev;
	if (Scr->StdCmapInfo.mru == sc) Scr->StdCmapInfo.mru = NULL;
    }
    return;
}

void LocateStandardColormaps(void)
{
    Atom *atoms;
    int natoms;
    int i;

    atoms = XListProperties (dpy, Scr->Root, &natoms);
    for (i = 0; i < natoms; i++) {
	XStandardColormap *maps = NULL;
	int nmaps;

	if (XGetRGBColormaps (dpy, Scr->Root, &maps, &nmaps, atoms[i])) {
	    /* if got one, then append to current list */
	    InsertRGBColormap (atoms[i], maps, nmaps, False);
	}
    }
    if (atoms) XFree ((char *) atoms);
    return;
}

void GetColor(int kind, Pixel *what, char *name)
{
    XColor color;
    Colormap cmap = Scr->RootColormaps.cwins[0]->colormap->c;

#ifndef TOM
    if (!Scr->FirstTime)
	return;
#endif

    if (Scr->Monochrome != kind)
	return;

    if (! XParseColor (dpy, cmap, name, &color)) {
	fprintf (stderr, "%s:  invalid color name \"%s\"\n", ProgramName, name);
	return;
    }
    if (! XAllocColor (dpy, cmap, &color))
    {
	/* if we could not allocate the color, let's see if this is a
	 * standard colormap
	 */
	XStandardColormap *stdcmap = NULL;

	if (! XParseColor (dpy, cmap, name, &color)) {
	    fprintf (stderr, "%s:  invalid color name \"%s\"\n", ProgramName, name);
	    return;
	}

	/*
	 * look through the list of standard colormaps (check cache first)
	 */
	if (Scr->StdCmapInfo.mru && Scr->StdCmapInfo.mru->maps &&
	    (Scr->StdCmapInfo.mru->maps[Scr->StdCmapInfo.mruindex].colormap ==
	     cmap)) {
	    stdcmap = &(Scr->StdCmapInfo.mru->maps[Scr->StdCmapInfo.mruindex]);
	} else {
	    StdCmap *sc;

	    for (sc = Scr->StdCmapInfo.head; sc; sc = sc->next) {
		int i;

		for (i = 0; i < sc->nmaps; i++) {
		    if (sc->maps[i].colormap == cmap) {
			Scr->StdCmapInfo.mru = sc;
			Scr->StdCmapInfo.mruindex = i;
			stdcmap = &(sc->maps[i]);
			goto gotit;
		    }
		}
	    }
	}

      gotit:
	if (stdcmap) {
            color.pixel = (stdcmap->base_pixel +
			   ((Pixel)(((float)color.red / 65535.0) *
				    stdcmap->red_max + 0.5) *
			    stdcmap->red_mult) +
			   ((Pixel)(((float)color.green /65535.0) *
				    stdcmap->green_max + 0.5) *
			    stdcmap->green_mult) +
			   ((Pixel)(((float)color.blue  / 65535.0) *
				    stdcmap->blue_max + 0.5) *
			    stdcmap->blue_mult));
        } else {
	    fprintf (stderr, "%s:  unable to allocate color \"%s\"\n", 
		     ProgramName, name);
	    return;
	}
    }

    *what = color.pixel;
    return;
}

void GetShadeColors (ColorPair *cp)
{
    XColor	xcol;
    Colormap	cmap = Scr->RootColormaps.cwins[0]->colormap->c;
    int		save;
    float	clearfactor;
    float	darkfactor;
    char	clearcol [32], darkcol [32];

    clearfactor = (float) Scr->ClearShadowContrast / 100.0;
    darkfactor  = (100.0 - (float) Scr->DarkShadowContrast)  / 100.0;
    xcol.pixel = cp->back;
    XQueryColor (dpy, cmap, &xcol);

    sprintf (clearcol, "#%04x%04x%04x",
		xcol.red   + (unsigned short) ((65535 -   xcol.red) * clearfactor),
		xcol.green + (unsigned short) ((65535 - xcol.green) * clearfactor),
		xcol.blue  + (unsigned short) ((65535 -  xcol.blue) * clearfactor));
    sprintf (darkcol,  "#%04x%04x%04x",
		(unsigned short) (xcol.red   * darkfactor),
		(unsigned short) (xcol.green * darkfactor),
		(unsigned short) (xcol.blue  * darkfactor));

    save = Scr->FirstTime;
    Scr->FirstTime = True;
    GetColor (Scr->Monochrome, &cp->shadc, clearcol);
    GetColor (Scr->Monochrome, &cp->shadd,  darkcol);
    Scr->FirstTime = save;
}

void GetFont(MyFont *font)
{
    char *deffontname = "fixed,*";
    char **missing_charset_list_return;
    int missing_charset_count_return;
    char *def_string_return;
    XFontSetExtents *font_extents;
    XFontStruct **xfonts;
    char **font_names;
    register int i;
    int ascent;
    int descent;
    int fnum;
    char *basename2;

    if (font->font_set != NULL){
	XFreeFontSet(dpy, font->font_set);
    }

    basename2 = (char *)malloc(strlen(font->basename) + 3);
    if (basename2) sprintf(basename2, "%s,*", font->basename);
    else basename2 = font->basename;
    if( (font->font_set = XCreateFontSet(dpy, basename2,
				    &missing_charset_list_return,
				    &missing_charset_count_return,
				    &def_string_return)) == NULL) {
	if (Scr->DefaultFont.basename) {
	    deffontname = Scr->DefaultFont.basename;
	}
	if ((font->font_set = XCreateFontSet(dpy, deffontname,
					     &missing_charset_list_return,
					     &missing_charset_count_return,
					     &def_string_return)) == NULL)
	{
	    fprintf (stderr, "%s:  unable to open fonts \"%s\" or \"%s\"\n",
		     ProgramName, font->basename, deffontname);
	    exit(1);
	}
    }
    if (basename2 != font->basename) free(basename2);
    font_extents = XExtentsOfFontSet(font->font_set);

    fnum = XFontsOfFontSet(font->font_set, &xfonts, &font_names);
    for( i = 0, ascent = 0, descent = 0; i<fnum; i++){
	ascent = MaxSize(ascent, (*xfonts)->ascent);
	descent = MaxSize(descent, (*xfonts)->descent);
	xfonts++;
    }

    font->height = font_extents->max_logical_extent.height;
    font->y = ascent;
    font->ascent = ascent;
    font->descent = descent;
}


void SetFocusVisualAttributes (TwmWindow *tmp_win, Bool focus)
{
    if (! tmp_win) return;

    if (focus == tmp_win->hasfocusvisible) return;
    if (tmp_win->highlight) {
	if (Scr->use3Dborders) {
	    PaintBorders (tmp_win, focus);
	}
	else {
	    if (focus) {
		XSetWindowBorder (dpy, tmp_win->frame, tmp_win->borderC.back);
		if (tmp_win->title_w)
		    XSetWindowBorder (dpy, tmp_win->title_w, tmp_win->borderC.back);
	    } else {
		XSetWindowBorderPixmap (dpy, tmp_win->frame, tmp_win->gray);
		if (tmp_win->title_w)
		    XSetWindowBorderPixmap (dpy, tmp_win->title_w, tmp_win->gray);
	    }
	}
    }

    if (focus) {
	Bool hil = False;

	if (tmp_win->lolite_wl) XUnmapWindow (dpy, tmp_win->lolite_wl);
	if (tmp_win->lolite_wr) XUnmapWindow (dpy, tmp_win->lolite_wr);
	if (tmp_win->hilite_wl) {
	    XMapWindow (dpy, tmp_win->hilite_wl);
	    hil = True;
	}
	if (tmp_win->hilite_wr) {
	    XMapWindow (dpy, tmp_win->hilite_wr);
	    hil = True;
	}
	if (hil && tmp_win->HiliteImage && tmp_win->HiliteImage->next) {
	    MaybeAnimate = True;
	}
	if (tmp_win->iconmanagerlist)
	    ActiveIconManager (tmp_win->iconmanagerlist);
    }
    else {
	if (tmp_win->hilite_wl) XUnmapWindow (dpy, tmp_win->hilite_wl);
	if (tmp_win->hilite_wr) XUnmapWindow (dpy, tmp_win->hilite_wr);
	if (tmp_win->lolite_wl) XMapWindow (dpy, tmp_win->lolite_wl);
	if (tmp_win->lolite_wr) XMapWindow (dpy, tmp_win->lolite_wr);
	if (tmp_win->iconmanagerlist)
	    NotActiveIconManager (tmp_win->iconmanagerlist);
    }
    if (Scr->use3Dtitles && Scr->SunkFocusWindowTitle && tmp_win->title_height) {
	ButtonState bs;

	bs = focus ? on : off;
	Draw3DBorder (tmp_win->title_w, Scr->TBInfo.titlex, 0,
			tmp_win->title_width - Scr->TBInfo.titlex -
			Scr->TBInfo.rightoff - Scr->TitlePadding,
			Scr->TitleHeight, Scr->TitleShadowDepth,
			tmp_win->title, bs, False, False);
    }
    tmp_win->hasfocusvisible = focus;
}

static void move_to_head (TwmWindow *t)
{
    if (t == NULL) return;
    if (Scr->FirstWindow == t) return;

    if (t->prev) t->prev->next = t->next;
    if (t->next) t->next->prev = t->prev;

    t->next = Scr->FirstWindow;
    if (Scr->FirstWindow != NULL)
	Scr->FirstWindow->prev = t;
    t->prev = NULL;
    Scr->FirstWindow = t;
}
 
/*
 * SetFocus - separate routine to set focus to make things more understandable
 * and easier to debug
 */
void SetFocus (TwmWindow *tmp_win, Time	tim)
{
    Window w = (tmp_win ? tmp_win->w : PointerRoot);
    int f_iconmgr = 0;

    if (Scr->Focus && (Scr->Focus->iconmgr)) f_iconmgr = 1;
    if (Scr->SloppyFocus && (w == PointerRoot) && (!f_iconmgr)) return;

    XSetInputFocus (dpy, w, RevertToPointerRoot, tim);
    if (Scr->Focus == tmp_win) return;

    if (Scr->Focus) {
	if (Scr->Focus->AutoSqueeze && !Scr->Focus->squeezed) {
	    AutoSqueeze (Scr->Focus);
	}
	SetFocusVisualAttributes (Scr->Focus, False);
    }
    if (tmp_win)    {
	if (tmp_win->AutoSqueeze && tmp_win->squeezed) {
	    AutoSqueeze (tmp_win);
	}
	SetFocusVisualAttributes (tmp_win, True);
    }
    Scr->Focus = tmp_win;
    move_to_head (tmp_win);
}

#
#ifdef NOPUTENV
/*
 * define our own putenv() if the system doesn't have one.
 * putenv(s): place s (a string of the form "NAME=value") in
 * the environment; replacing any existing NAME.  s is placed in
 * environment, so if you change s, the environment changes (like
 * putenv on a sun).  Binding removed if you putenv something else
 * called NAME.
 */
int
putenv(s)
    char *s;
{
    char *v;
    int varlen, idx;
    extern char **environ;
    char **newenv;
    static int virgin = 1; /* true while "environ" is a virgin */

    v = strchr(s, '=');
    if(v == 0)
	return 0; /* punt if it's not of the right form */
    varlen = (v + 1) - s;

    for (idx = 0; environ[idx] != 0; idx++) {
	if (strncmp(environ[idx], s, varlen) == 0) {
	    if(v[1] != 0) { /* true if there's a value */
		environ[idx] = s;
		return 0;
	    } else {
		do {
		    environ[idx] = environ[idx+1];
		} while(environ[++idx] != 0);
		return 0;
	    }
	}
    }
    
    /* add to environment (unless no value; then just return) */
    if(v[1] == 0)
	return 0;
    if(virgin) {
	register i;

	newenv = (char **) malloc((unsigned) ((idx + 2) * sizeof(char*)));
	if(newenv == 0)
	    return -1;
	for(i = idx-1; i >= 0; --i)
	    newenv[i] = environ[i];
	virgin = 0;     /* you're not a virgin anymore, sweety */
    } else {
	newenv = (char **) realloc((char *) environ,
				   (unsigned) ((idx + 2) * sizeof(char*)));
	if (newenv == 0)
	    return -1;
    }

    environ = newenv;
    environ[idx] = s;
    environ[idx+1] = 0;
    
    return 0;
}
#endif /* NOPUTENV */


static Pixmap CreateXLogoPixmap (unsigned int *widthp, unsigned int *heightp)
{
    int h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (h < 0) h = 0;

    *widthp = *heightp = (unsigned int) h;
    if (Scr->tbpm.xlogo == None) {
	GC gc, gcBack;

	Scr->tbpm.xlogo = XCreatePixmap (dpy, Scr->Root, h, h, 1);
	gc = XCreateGC (dpy, Scr->tbpm.xlogo, 0L, NULL);
	XSetForeground (dpy, gc, 0);
	XFillRectangle (dpy, Scr->tbpm.xlogo, gc, 0, 0, h, h);
	XSetForeground (dpy, gc, 1);
	gcBack = XCreateGC (dpy, Scr->tbpm.xlogo, 0L, NULL);
	XSetForeground (dpy, gcBack, 0);

	/*
	 * draw the logo large so that it gets as dense as possible; then white
	 * out the edges so that they look crisp
	 */
	XmuDrawLogo (dpy, Scr->tbpm.xlogo, gc, gcBack, -1, -1, h + 2, h + 2);
	XDrawRectangle (dpy, Scr->tbpm.xlogo, gcBack, 0, 0, h - 1, h - 1);

	/*
	 * done drawing
	 */
	XFreeGC (dpy, gc);
	XFreeGC (dpy, gcBack);
    }
    return Scr->tbpm.xlogo;
}


static Pixmap CreateResizePixmap (unsigned int *widthp, unsigned int *heightp)
{
    int h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (h < 1) h = 1;

    *widthp = *heightp = (unsigned int) h;
    if (Scr->tbpm.resize == None) {
	XPoint	points[3];
	GC gc;
	int w;
	int lw;

	/*
	 * create the pixmap
	 */
	Scr->tbpm.resize = XCreatePixmap (dpy, Scr->Root, h, h, 1);
	gc = XCreateGC (dpy, Scr->tbpm.resize, 0L, NULL);
	XSetForeground (dpy, gc, 0);
	XFillRectangle (dpy, Scr->tbpm.resize, gc, 0, 0, h, h);
	XSetForeground (dpy, gc, 1);
	lw = h / 16;
	if (lw == 1)
	    lw = 0;
	XSetLineAttributes (dpy, gc, lw, LineSolid, CapButt, JoinMiter);

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
	XDrawLines (dpy, Scr->tbpm.resize, gc, points, 3, CoordModeOrigin);
	w = w / 2;
	points[0].x = w;
	points[0].y = 0;
	points[1].x = w;
	points[1].y = w;
	points[2].x = 0;
	points[2].y = w;
	XDrawLines (dpy, Scr->tbpm.resize, gc, points, 3, CoordModeOrigin);

	/*
	 * done drawing
	 */
	XFreeGC(dpy, gc);
    }
    return Scr->tbpm.resize;
}

static Pixmap CreateDotPixmap (unsigned int *widthp, unsigned int *heightp)
{
    int h = Scr->TBInfo.width - Scr->TBInfo.border * 2;

    h = h * 3 / 4;
    if (h < 1) h = 1;
    if (!(h & 1))
	h--;
    *widthp = *heightp = (unsigned int) h;
    if (Scr->tbpm.delete == None) {
	GC  gc;
	Pixmap pix;

	pix = Scr->tbpm.delete = XCreatePixmap (dpy, Scr->Root, h, h, 1);
	gc = XCreateGC (dpy, pix, 0L, NULL);
	XSetLineAttributes (dpy, gc, h, LineSolid, CapRound, JoinRound);
	XSetForeground (dpy, gc, 0L);
	XFillRectangle (dpy, pix, gc, 0, 0, h, h);
	XSetForeground (dpy, gc, 1L);
	XDrawLine (dpy, pix, gc, h/2, h/2, h/2, h/2);
	XFreeGC (dpy, gc);
    }
    return Scr->tbpm.delete;
}

static Image *Create3DCrossImage (ColorPair cp)
{
    Image *image;
    int        h;
    int    point;
    int midpoint;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;
    point = 4;
    midpoint = h/2;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);

#ifdef LEVITTE_TEST
    FB (cp.shadc, cp.shadd);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point+1, point-1, point-1, point+1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point+1, point, point, point+1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point-1, point+1, midpoint-2, midpoint);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, midpoint, midpoint+2, h-point-3, h-point-1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, point+1, h-point-3, h-point-2);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point-1, h-point-2, midpoint-2, midpoint);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, midpoint, midpoint-2, h-point-2, point-1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, h-point-2, h-point-2, point);
#endif

    FB (cp.shadd, cp.shadc);
#ifdef LEVITTE_TEST
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point+2, point+1, h-point-1, h-point-2);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point+2, point, midpoint, midpoint-2);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, midpoint+2, midpoint, h-point, h-point-2);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, h-point, h-point-2, h-point-2, h-point);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, h-point-1, h-point-2, h-point-2, h-point-1);
#else
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, point, h-point-1, h-point-1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point-1, point, h-point-1, h-point);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, point-1, h-point, h-point-1);
#endif

#ifdef LEVITTE_TEST
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, h-point-1, point, h-point-1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, h-point-1, point, h-point-1, point);
#else
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, h-point-1, h-point-1, point);
#endif
#ifdef LEVITTE_TEST
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point+1, h-point-1, h-point-1, point+1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point+1, h-point, midpoint, midpoint+2);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, midpoint+2, midpoint, h-point, point+1);
#else
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point-1, h-point-1, h-point-1, point-1);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, h-point, h-point, point);
#endif

    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;

    return (image);
}

static Image *Create3DIconifyImage (ColorPair cp)
{
    Image *image;
    int     h;
    int point;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;
    point = ((h/2-2) * 2+1) / 3;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    FB (cp.shadd, cp.shadc);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, point, h/2, h-point);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, point, point, h-point, point);

    FB (cp.shadc, cp.shadd);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, h-point, point, h/2+1, h-point);
    XDrawLine (dpy, image->pixmap, Scr->NormalGC, h-point-1, point+1, h/2+1, h-point-1);

    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;

    return (image);
}

static Image *Create3DSunkenResizeImage (ColorPair cp)
{
    int     h;
    Image *image;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap, 3, 3, h-6, h-6, 1, cp, on, True, False);
    Draw3DBorder (image->pixmap, 3, ((h-6)/3)+3, ((h-6)*2/3)+1,
     ((h-6)*2/3)+1, 1, cp, on, True, False);
    Draw3DBorder (image->pixmap, 3, ((h-6)*2/3)+3, ((h-6)/3)+1,
     ((h-6)/3)+1, 1, cp, on, True, False);

    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;

    return (image);
}

static Image *Create3DBoxImage (ColorPair cp)
{
    int     h;
    Image   *image;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (! (h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap, (h / 2) - 4, (h / 2) - 4, 9, 9, 1, cp,
     off, True, False);

    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;

    return (image);
}

static Image *Create3DDotImage (ColorPair cp)
{
    Image *image;
    int	  h;
    static int idepth = 2;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap, (h / 2) - idepth,
				 (h / 2) - idepth,
				 2 * idepth + 1,
				 2 * idepth + 1,
				 idepth, cp, off, True, False);
    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;
    return (image);
}

static Image *Create3DBarImage (ColorPair cp)
{
    Image *image;
    int	  h;
    static int idepth = 2;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap,
			Scr->TitleButtonShadowDepth + 2,
			(h / 2) - idepth,
			h - 2 * (Scr->TitleButtonShadowDepth + 2),
			2 * idepth + 1,
			idepth, cp, off, True, False);
    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;
    return (image);
}

static Image *Create3DVertBarImage (ColorPair cp)
{
    Image *image;
    int	  h;
    static int idepth = 2;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap,
			(h / 2) - idepth,
			Scr->TitleButtonShadowDepth + 2,
			2 * idepth + 1,
			h - 2 * (Scr->TitleButtonShadowDepth + 2),
			idepth, cp, off, True, False);
    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;
    return (image);
}

static Image *Create3DMenuImage (ColorPair cp)
{
    Image *image;
    int	  h, i;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    for (i = 4; i < h - 7; i += 5) {
	Draw3DBorder (image->pixmap, 4, i, h - 8, 4, 2, cp, off, True, False);
    }
    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;
    return (image);
}

static Image *Create3DResizeImage (ColorPair cp)
{
    Image *image;
    int	  h;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap, 0, h / 4, ((3 * h) / 4) + 1, ((3 * h) / 4) + 1, 2,
		cp, off, True, False);
    Draw3DBorder (image->pixmap, 0, h / 2, (h / 2) + 1, (h / 2) + 1, 2, cp, off, True, False);
    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;
    return (image);
}

static Image *Create3DZoomImage (ColorPair cp)
{
    Image *image;
    int		h;
    static int idepth = 2;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = (Image*) malloc (sizeof (struct _Image));
    if (! image) return (None);
    image->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    if (image->pixmap == None) return (None);

    Draw3DBorder (image->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
    Draw3DBorder (image->pixmap, Scr->TitleButtonShadowDepth + 2,
				 Scr->TitleButtonShadowDepth + 2,
				 h - 2 * (Scr->TitleButtonShadowDepth + 2),
				 h - 2 * (Scr->TitleButtonShadowDepth + 2),
				 idepth, cp, off, True, False);

    image->mask   = None;
    image->width  = h;
    image->height = h;
    image->next   = None;
    return (image);
}

struct Colori {
    Pixel color;
    Pixmap pix;
    struct Colori *next;
};

Pixmap Create3DMenuIcon (unsigned int height,
			 unsigned int *widthp, unsigned int *heightp,
			 ColorPair cp)
{
    unsigned int h, w;
    int		i;
    struct Colori *col;
    static struct Colori *colori = NULL;

    h = height;
    w = h * 7 / 8;
    if (h < 1) h = 1;
    if (w < 1) w = 1;
    *widthp  = w;
    *heightp = h;

    for (col = colori; col; col = col->next) {
	if (col->color == cp.back) break;
    }
    if (col != NULL) {
	return (col->pix);
    }
    col = (struct Colori*) malloc (sizeof (struct Colori));
    col->color = cp.back;
    col->pix   = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
    col->next = colori;
    colori = col;

    Draw3DBorder (col->pix, 0, 0, w, h, 1, cp, off, True, False);
    for (i = 3; i + 5 < h; i += 5) {
	Draw3DBorder (col->pix, 4, i, w - 8, 3, 1, Scr->MenuC, off, True, False);
    }
    return (colori->pix);
}

#include "siconify.bm"

Pixmap Create3DIconManagerIcon (ColorPair cp)
{
    unsigned int w, h;
    struct Colori *col;
    static struct Colori *colori = NULL;

    w = (unsigned int) siconify_width;
    h = (unsigned int) siconify_height;

    for (col = colori; col; col = col->next) {
	if (col->color == cp.back) break;
    }
    if (col != NULL) {
	return (col->pix);
    }
    col = (struct Colori*) malloc (sizeof (struct Colori));
    col->color = cp.back;
    col->pix   = XCreatePixmap (dpy, Scr->Root, w, h, Scr->d_depth);
    Draw3DBorder (col->pix, 0, 0, w, h, 4, cp, off, True, False);
    col->next = colori;
    colori = col;

    return (colori->pix);
}

static Image *Create3DResizeAnimation (Bool in, Bool left, Bool top,
				       ColorPair cp)
{
    int		h, i, j;
    Image	*image, *im, *im1;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = im1 = None;
    for (i = (in ? 0 : (h/4)-1); (i < h/4) && (i >= 0); i += (in ? 1 : -1)) {
	im = (Image*) malloc (sizeof (struct _Image));
	if (! im) return (None);
	im->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
	if (im->pixmap == None) {
	    free (im);
	    return (None);
	}
	Draw3DBorder (im->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
	for (j = i; j <= h; j += (h/4)){
	    Draw3DBorder (im->pixmap, (left ? 0 : j), (top ? 0 : j),
			  h - j, h - j, 2, cp, off, True, False);
	}
	im->mask   = None;
	im->width  = h;
	im->height = h;
	im->next   = None;
	if (image == None) {
	    image = im1 = im;
	}
	else {
	    im1->next = im;
	    im1 = im;
	}
    }
    if (im1 != None) im1->next = image;
    return (image);
}

static Image *Create3DResizeInTopAnimation (ColorPair cp)
{
    return Create3DResizeAnimation (TRUE, FALSE, TRUE, cp);
}

static Image *Create3DResizeOutTopAnimation (ColorPair cp)
{
    return Create3DResizeAnimation (False, FALSE, TRUE, cp);
}

static Image *Create3DResizeInBotAnimation (ColorPair cp)
{
    return Create3DResizeAnimation (TRUE, TRUE, FALSE, cp);
}

static Image *Create3DResizeOutBotAnimation (ColorPair cp)
{
    return Create3DResizeAnimation (False, TRUE, FALSE, cp);
}

static Image *Create3DMenuAnimation (Bool up, ColorPair cp)
{
    int               h, i, j;
    Image     *image, *im, *im1;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    image = im1 = None;
    for (j = (up ? 4 : 0); j != (up ? -1 : 5); j+= (up ? -1 : 1)) {
	im = (Image*) malloc (sizeof (struct _Image));
	if (! im) return (None);
	im->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
	if (im->pixmap == None) {
	    free (im);
	    return (None);
	}
	Draw3DBorder (im->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
	for (i = j; i < h - 3; i += 5) {
	    Draw3DBorder (im->pixmap, 4, i, h - 8, 4, 2, cp, off, True, False);
	}
	im->mask   = None;
	im->width  = h;
	im->height = h;
	im->next   = None;
	if (image == None) {
	    image = im1 = im;
	}
	else {
	    im1->next = im;
	    im1 = im;
	}
    }
    if (im1 != None) im1->next = image;
    return (image);
}

static Image *Create3DMenuUpAnimation (ColorPair cp)
{
    return Create3DMenuAnimation (TRUE, cp);
}

static Image *Create3DMenuDownAnimation (ColorPair cp)
{
    return Create3DMenuAnimation (False, cp);
}

static Image *Create3DZoomAnimation (Bool in, Bool out, int n, ColorPair cp)
{
    int		h, i, j, k;
    Image	*image, *im, *im1;

    h = Scr->TBInfo.width - Scr->TBInfo.border * 2;
    if (!(h & 1)) h--;

    if (n == 0) n = (h/2) - 2;

    image = im1 = None;
    for (j = (out ? -1 : 1) ; j < (in ? 2 : 0); j += 2){
	for(k = (j > 0 ? 0 : n-1) ; (k >= 0) && (k < n); k += j){
	    im = (Image*) malloc (sizeof (struct _Image));
	    im->pixmap = XCreatePixmap (dpy, Scr->Root, h, h, Scr->d_depth);
	    Draw3DBorder (im->pixmap, 0, 0, h, h, Scr->TitleButtonShadowDepth, cp, off, True, False);
	    for (i = 2 + k; i < (h / 2); i += n) {
		Draw3DBorder (im->pixmap, i, i, h - (2 * i), h - (2 * i), 2, cp, off, True, False);
	    }
	    im->mask   = None;
	    im->width  = h;
	    im->height = h;
	    im->next   = None;
	    if (image == None) {
		image = im1 = im;
	    }
	    else {
		im1->next = im;
		im1 = im;
	    }
	}
    }
    if (im1 != None) im1->next = image;
    return (image);
}

static Image *Create3DMazeInAnimation (ColorPair cp)
{
    return Create3DZoomAnimation(TRUE, FALSE, 6, cp);
}

static Image *Create3DMazeOutAnimation (ColorPair cp)
{
    return Create3DZoomAnimation(FALSE, TRUE, 6, cp);
}

static Image *Create3DZoomInAnimation (ColorPair cp)
{
    return Create3DZoomAnimation(TRUE, FALSE, 0, cp);
}

static Image *Create3DZoomOutAnimation (ColorPair cp)
{
    return Create3DZoomAnimation(FALSE, TRUE, 0, cp);
}

static Image *Create3DZoomInOutAnimation (ColorPair cp)
{
    return Create3DZoomAnimation(TRUE, TRUE, 0, cp);
}

#define questionmark_width 8
#define questionmark_height 8
static char questionmark_bits[] = {
   0x38, 0x7c, 0x64, 0x30, 0x18, 0x00, 0x18, 0x18};

static Pixmap CreateQuestionPixmap (unsigned int *widthp,
				    unsigned int *heightp)
{
    *widthp = questionmark_width;
    *heightp = questionmark_height;
    if (Scr->tbpm.question == None) {
	Scr->tbpm.question = XCreateBitmapFromData (dpy, Scr->Root,
						    questionmark_bits,
						    questionmark_width,
						    questionmark_height);
    }
    /*
     * this must succeed or else we are in deep trouble elsewhere
     */
    return Scr->tbpm.question;
}


static Pixmap CreateMenuPixmap (unsigned int *widthp, unsigned int *heightp)
{
    return (CreateMenuIcon (Scr->TBInfo.width - Scr->TBInfo.border * 2,widthp,heightp));
}

Pixmap CreateMenuIcon (int height, unsigned int *widthp, unsigned int *heightp)
{
    int h, w;
    int ih, iw;
    int	ix, iy;
    int	mh, mw;
    int	tw, th;
    int	lw, lh;
    int	lx, ly;
    int	lines, dly;
    int offset;
    int	bw;

    h = height;
    w = h * 7 / 8;
    if (h < 1)
	h = 1;
    if (w < 1)
	w = 1;
    *widthp = w;
    *heightp = h;
    if (Scr->tbpm.menu == None) {
	Pixmap  pix;
	GC	gc;

	pix = Scr->tbpm.menu = XCreatePixmap (dpy, Scr->Root, w, h, 1);
	gc = XCreateGC (dpy, pix, 0L, NULL);
	XSetForeground (dpy, gc, 0L);
	XFillRectangle (dpy, pix, gc, 0, 0, w, h);
	XSetForeground (dpy, gc, 1L);
	ix = 1;
	iy = 1;
	ih = h - iy * 2;
	iw = w - ix * 2;
	offset = ih / 8;
	mh = ih - offset;
	mw = iw - offset;
	bw = mh / 16;
	if (bw == 0 && mw > 2)
	    bw = 1;
	tw = mw - bw * 2;
	th = mh - bw * 2;
	XFillRectangle (dpy, pix, gc, ix, iy, mw, mh);
	XFillRectangle (dpy, pix, gc, ix + iw - mw, iy + ih - mh, mw, mh);
	XSetForeground (dpy, gc, 0L);
	XFillRectangle (dpy, pix, gc, ix+bw, iy+bw, tw, th);
	XSetForeground (dpy, gc, 1L);
	lw = tw / 2;
	if ((tw & 1) ^ (lw & 1))
	    lw++;
	lx = ix + bw + (tw - lw) / 2;

	lh = th / 2 - bw;
	if ((lh & 1) ^ ((th - bw) & 1))
	    lh++;
	ly = iy + bw + (th - bw - lh) / 2;

	lines = 3;
	if ((lh & 1) && lh < 6)
	{
	    lines--;
	}
	dly = lh / (lines - 1);
	while (lines--)
	{
	    XFillRectangle (dpy, pix, gc, lx, ly, lw, bw);
	    ly += dly;
	}
	XFreeGC (dpy, gc);
    }
    return Scr->tbpm.menu;
}

#define FBGC(gc, fix_fore, fix_back)\
    Gcv.foreground = fix_fore;\
    Gcv.background = fix_back;\
    XChangeGC(dpy, gc, GCForeground|GCBackground,&Gcv)

void Draw3DBorder (Window w, int x, int y, int width, int height, int bw,
		   ColorPair cp, int state, int fill, int forcebw)
{
    int		  i;
    XGCValues	  gcv;
    unsigned long gcm;

    if ((width < 1) || (height < 1)) return;
    if (Scr->Monochrome != COLOR) {
	if (fill) {
	    gcm = GCFillStyle;
	    gcv.fill_style = FillOpaqueStippled;
	    XChangeGC (dpy, Scr->BorderGC, gcm, &gcv);
	    XFillRectangle (dpy, w, Scr->BorderGC, x, y, width, height);
	}
	gcm  = 0;
	gcm |= GCLineStyle;		
	gcv.line_style = (state == on) ? LineSolid : LineDoubleDash;
	gcm |= GCFillStyle;
	gcv.fill_style = FillSolid;
	XChangeGC (dpy, Scr->BorderGC, gcm, &gcv);
	for (i = 0; i < bw; i++) {
	    XDrawLine (dpy, w, Scr->BorderGC, x,                 y + i,
					    x + width - i - 1, y + i);
	    XDrawLine (dpy, w, Scr->BorderGC, x + i,                  y,
					    x + i, y + height - i - 1);
	}

	gcm  = 0;
	gcm |= GCLineStyle;		
	gcv.line_style = (state == on) ? LineDoubleDash : LineSolid;
	gcm |= GCFillStyle;
	gcv.fill_style = FillSolid;
	XChangeGC (dpy, Scr->BorderGC, gcm, &gcv);
	for (i = 0; i < bw; i++) {
	    XDrawLine (dpy, w, Scr->BorderGC, x + width - i - 1,          y + i,
					    x + width - i - 1, y + height - 1);
	    XDrawLine (dpy, w, Scr->BorderGC, x + i,         y + height - i - 1,
					    x + width - 1, y + height - i - 1);
	}
	return;
    }

    if (fill) {
	FBGC (Scr->BorderGC, cp.back, cp.fore);
	XFillRectangle (dpy, w, Scr->BorderGC, x, y, width, height);
    }
    if (Scr->BeNiceToColormap) {
	int dashoffset = 0;

	gcm  = 0;
	gcm |= GCLineStyle;		
	gcv.line_style = (forcebw) ? LineSolid : LineDoubleDash;
	gcm |= GCBackground;
	gcv.background = cp.back;
	XChangeGC (dpy, Scr->BorderGC, gcm, &gcv);
	    
	if (state == on)
	    XSetForeground (dpy, Scr->BorderGC, Scr->Black);
	else
	    XSetForeground (dpy, Scr->BorderGC, Scr->White);
	for (i = 0; i < bw; i++) {
	    XDrawLine (dpy, w, Scr->BorderGC, x + i,     y + dashoffset,
					    x + i, y + height - i - 1);
	    XDrawLine (dpy, w, Scr->BorderGC, x + dashoffset,    y + i,
					    x + width - i - 1, y + i);
	    dashoffset = 1 - dashoffset;
	}
	XSetForeground (dpy, Scr->BorderGC, ((state == on) ? Scr->White : Scr->Black));
	for (i = 0; i < bw; i++) {
	    XDrawLine (dpy, w, Scr->BorderGC, x + i,         y + height - i - 1,
					    x + width - 1, y + height - i - 1);
	    XDrawLine (dpy, w, Scr->BorderGC, x + width - i - 1,          y + i,
					    x + width - i - 1, y + height - 1);
	}
	return;
    }
    if (state == on) { FBGC (Scr->BorderGC, cp.shadd, cp.shadc); }
    else             { FBGC (Scr->BorderGC, cp.shadc, cp.shadd); }
    for (i = 0; i < bw; i++) {
	XDrawLine (dpy, w, Scr->BorderGC, x,                 y + i,
					  x + width - i - 1, y + i);
	XDrawLine (dpy, w, Scr->BorderGC, x + i,                  y,
					  x + i, y + height - i - 1);
    }

    if (state == on) { FBGC (Scr->BorderGC, cp.shadc, cp.shadd); }
    else             { FBGC (Scr->BorderGC, cp.shadd, cp.shadc); }
    for (i = 0; i < bw; i++) {
	XDrawLine (dpy, w, Scr->BorderGC, x + width - i - 1,          y + i,
					  x + width - i - 1, y + height - 1);
	XDrawLine (dpy, w, Scr->BorderGC, x + i,         y + height - i - 1,
					  x + width - 1, y + height - i - 1);
    }
    return;
}

void Draw3DCorner (Window w,
		   int x, int y, int width, int height, int thick, int bw,
		   ColorPair cp, int type)
{
    XRectangle rects [2];

    switch (type) {
	case 0 :
	    Draw3DBorder (w, x, y, width, height, bw, cp, off, True, False);
	    Draw3DBorder (w, x + thick - bw, y + thick - bw,
			width - thick + 2 * bw, height - thick + 2 * bw,
			bw, cp, on, True, False);
	    break;
	case 1 :
	    Draw3DBorder (w, x, y, width, height, bw, cp, off, True, False);
	    Draw3DBorder (w, x, y + thick - bw,
			width - thick + bw, height - thick,
			bw, cp, on, True, False);
	    break;
	case 2 :
	    rects [0].x      = x + width - thick;
	    rects [0].y      = y;
	    rects [0].width  = thick;
	    rects [0].height = height;
	    rects [1].x      = x;
	    rects [1].y      = y + width - thick;
	    rects [1].width  = width - thick;
	    rects [1].height = thick;
	    XSetClipRectangles (dpy, Scr->BorderGC, 0, 0, rects, 2, Unsorted);
	    Draw3DBorder (w, x, y, width, height, bw, cp, off, True, False);
	    Draw3DBorder (w, x, y,
			width - thick + bw, height - thick + bw,
			bw, cp, on, True, False);
	    XSetClipMask (dpy, Scr->BorderGC, None);
	    break;
	case 3 :
	    rects [0].x      = x;
	    rects [0].y      = y;
	    rects [0].width  = thick;
	    rects [0].height = height;
	    rects [1].x      = x + thick;
	    rects [1].y      = y + height - thick;
	    rects [1].width  = width - thick;
	    rects [1].height = thick;
	    XSetClipRectangles (dpy, Scr->BorderGC, 0, 0, rects, 2, Unsorted);
	    Draw3DBorder (w, x, y, width, height, bw, cp, off, True, False);
	    Draw3DBorder (w, x + thick - bw, y,
			width - thick, height - thick + bw,
			bw, cp, on, True, False);
	    XSetClipMask (dpy, Scr->BorderGC, None);
	    break;
    }
    return;
}

void PaintAllDecoration (void)
{
    TwmWindow *tmp_win;
    virtualScreen *vs;

    for (tmp_win = Scr->FirstWindow; tmp_win != NULL; tmp_win = tmp_win->next) {
	if (! visible (tmp_win)) continue;
	if (tmp_win->mapped == TRUE) {
	    if (tmp_win->frame_bw3D) {
		if (tmp_win->highlight && tmp_win == Scr->Focus)
		    PaintBorders (tmp_win, True);
		else
		    PaintBorders (tmp_win, False);
	    }
	    if (tmp_win->title_w)      PaintTitle        (tmp_win);
	    if (tmp_win->titlebuttons) PaintTitleButtons (tmp_win);
	}
	else
	if ((tmp_win->icon_on == TRUE)  &&
		!tmp_win->icon_not_ours &&
		!Scr->NoIconTitlebar    &&
		tmp_win->icon           &&
		tmp_win->icon->w        &&
		! LookInList (Scr->NoIconTitle, tmp_win->full_name, &tmp_win->class)) {
	    PaintIcon (tmp_win);
	}
    }
    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      PaintWorkSpaceManager (vs);
    }
}

void PaintBorders (TwmWindow *tmp_win, Bool focus)
{
    ColorPair cp;

    cp = (focus && tmp_win->highlight) ? tmp_win->borderC : tmp_win->border_tile;
    if (tmp_win->title_height == 0) {
	Draw3DBorder (tmp_win->frame,
	    0,
	    0,
	    tmp_win->frame_width,
	    tmp_win->frame_height,
	    Scr->BorderShadowDepth, cp, off, True, False);
	Draw3DBorder (tmp_win->frame,
	    tmp_win->frame_bw3D - Scr->BorderShadowDepth,
	    tmp_win->frame_bw3D - Scr->BorderShadowDepth,
	    tmp_win->frame_width  - 2 * tmp_win->frame_bw3D + 2 * Scr->BorderShadowDepth,
	    tmp_win->frame_height - 2 * tmp_win->frame_bw3D + 2 * Scr->BorderShadowDepth,
	    Scr->BorderShadowDepth, cp, on, True, False);
	return;
    }
    Draw3DCorner (tmp_win->frame,
		tmp_win->title_x - tmp_win->frame_bw3D,
		0,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		tmp_win->frame_bw3D, Scr->BorderShadowDepth, cp, 0);
    Draw3DCorner (tmp_win->frame,
		tmp_win->title_x + tmp_win->title_width - Scr->TitleHeight,
		0,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		tmp_win->frame_bw3D, Scr->BorderShadowDepth, cp, 1);
    Draw3DCorner (tmp_win->frame,
		tmp_win->frame_width  - (Scr->TitleHeight + tmp_win->frame_bw3D),
		tmp_win->frame_height - (Scr->TitleHeight + tmp_win->frame_bw3D),
		Scr->TitleHeight + tmp_win->frame_bw3D,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		tmp_win->frame_bw3D, Scr->BorderShadowDepth, cp, 2);
    Draw3DCorner (tmp_win->frame,
		0,
		tmp_win->frame_height - (Scr->TitleHeight + tmp_win->frame_bw3D),
		Scr->TitleHeight + tmp_win->frame_bw3D,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		tmp_win->frame_bw3D, Scr->BorderShadowDepth, cp, 3);

    Draw3DBorder (tmp_win->frame,
		tmp_win->title_x + Scr->TitleHeight,
		0,
		tmp_win->title_width - 2 * Scr->TitleHeight,
		tmp_win->frame_bw3D,
		Scr->BorderShadowDepth, cp, off, True, False);
    Draw3DBorder (tmp_win->frame,
		tmp_win->frame_bw3D + Scr->TitleHeight,
		tmp_win->frame_height - tmp_win->frame_bw3D,
		tmp_win->frame_width - 2 * (Scr->TitleHeight + tmp_win->frame_bw3D),
		tmp_win->frame_bw3D,
		Scr->BorderShadowDepth, cp, off, True, False);
    Draw3DBorder (tmp_win->frame,
		0,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		tmp_win->frame_bw3D,
		tmp_win->frame_height - 2 * (Scr->TitleHeight + tmp_win->frame_bw3D),
		Scr->BorderShadowDepth, cp, off, True, False);
    Draw3DBorder (tmp_win->frame,
		tmp_win->frame_width  - tmp_win->frame_bw3D,
		Scr->TitleHeight + tmp_win->frame_bw3D,
		tmp_win->frame_bw3D,
		tmp_win->frame_height - 2 * (Scr->TitleHeight + tmp_win->frame_bw3D),
		Scr->BorderShadowDepth, cp, off, True, False);

    if (tmp_win->squeeze_info && !tmp_win->squeezed) {
	Draw3DBorder (tmp_win->frame,
		0,
		Scr->TitleHeight,
		tmp_win->title_x,
		tmp_win->frame_bw3D,
		Scr->BorderShadowDepth, cp, off, True, False);
	Draw3DBorder (tmp_win->frame,
		tmp_win->title_x + tmp_win->title_width,
		Scr->TitleHeight,
		tmp_win->frame_width - tmp_win->title_x - tmp_win->title_width,
		tmp_win->frame_bw3D,
		Scr->BorderShadowDepth, cp, off, True, False);
    }
}

void PaintTitle (TwmWindow *tmp_win)
{
    int width, mwidth, len;
    XRectangle inc_rect;
    XRectangle logical_rect;

    if (Scr->use3Dtitles) {
	if (Scr->SunkFocusWindowTitle && (Scr->Focus == tmp_win) &&
	    (tmp_win->title_height != 0))
	    Draw3DBorder (tmp_win->title_w, Scr->TBInfo.titlex, 0,
		tmp_win->title_width - Scr->TBInfo.titlex -
		Scr->TBInfo.rightoff - Scr->TitlePadding,
		Scr->TitleHeight, Scr->TitleShadowDepth,
		tmp_win->title, on, True, False);
	else
	    Draw3DBorder (tmp_win->title_w, Scr->TBInfo.titlex, 0,
		tmp_win->title_width - Scr->TBInfo.titlex -
		Scr->TBInfo.rightoff - Scr->TitlePadding,
		Scr->TitleHeight, Scr->TitleShadowDepth,
		tmp_win->title, off, True, False);
    }
    FB(tmp_win->title.fore, tmp_win->title.back);
    if (Scr->use3Dtitles) {
	len    = strlen(tmp_win->name);
	XmbTextExtents(Scr->TitleBarFont.font_set,
		       tmp_win->name, strlen (tmp_win->name),
		       &inc_rect, &logical_rect);
	width = logical_rect.width;
	mwidth = tmp_win->title_width  - Scr->TBInfo.titlex -
		 Scr->TBInfo.rightoff  - Scr->TitlePadding  -
		 Scr->TitleShadowDepth - 4;
	while ((len > 0) && (width > mwidth)) {
	    len--;
	    XmbTextExtents(Scr->TitleBarFont.font_set,
			   tmp_win->name, len,
			   &inc_rect, &logical_rect);
	    width = logical_rect.width;
	}
	if (Scr->Monochrome != COLOR) {
	    XmbDrawImageString(dpy, tmp_win->title_w, Scr->TitleBarFont.font_set,
			     Scr->NormalGC,
			     tmp_win->name_x,
			     Scr->TitleBarFont.y + Scr->TitleShadowDepth, 
			     tmp_win->name, len);
	}
	else {
	    XmbDrawString (dpy, tmp_win->title_w, Scr->TitleBarFont.font_set,
			   Scr->NormalGC, tmp_win->name_x,
			   Scr->TitleBarFont.y + Scr->TitleShadowDepth, 
			   tmp_win->name, len);
	}
    }
    else
        XmbDrawString (dpy, tmp_win->title_w, Scr->TitleBarFont.font_set,
		       Scr->NormalGC,
		       tmp_win->name_x, Scr->TitleBarFont.y, 
		       tmp_win->name, strlen(tmp_win->name));
}

void PaintIcon (TwmWindow *tmp_win)
{
    int		width, twidth, mwidth, len, x;
    Icon	*icon;
    XRectangle inc_rect;
    XRectangle logical_rect;

    if (!tmp_win || !tmp_win->icon) return;
    icon = tmp_win->icon;
    if (!icon->has_title) return;

    x     = 0;
    width = icon->w_width;
    if (Scr->ShrinkIconTitles && icon->title_shrunk) {
	x     = GetIconOffset (icon);
	width = icon->width;
    }
    len    = strlen (tmp_win->icon_name);
    XmbTextExtents(Scr->IconFont.font_set,
		   tmp_win->icon_name, len,
		   &inc_rect, &logical_rect);
    twidth = logical_rect.width;
    mwidth = width - 2 * Scr->IconManagerShadowDepth - 6;
    if (Scr->use3Diconmanagers) {
	Draw3DBorder (icon->w, x, icon->height, width,
		Scr->IconFont.height + 2 * Scr->IconManagerShadowDepth + 6,
		Scr->IconManagerShadowDepth, icon->iconc, off, False, False);
    }
    while ((len > 0) && (twidth > mwidth)) {
	len--;
	XmbTextExtents(Scr->IconFont.font_set,
		       tmp_win->icon_name, len,
		       &inc_rect, &logical_rect);
	twidth = logical_rect.width;
    }
    FB (icon->iconc.fore, icon->iconc.back);
    XmbDrawString(dpy, icon->w, Scr->IconFont.font_set, Scr->NormalGC,
		  x + ((mwidth - twidth)/2) + Scr->IconManagerShadowDepth + 3,
		  icon->y, tmp_win->icon_name, len);
}

void PaintTitleButton (TwmWindow *tmp_win, TBWindow  *tbw)
{
    TitleButton *tb = tbw->info;

    XCopyArea (dpy, tbw->image->pixmap, tbw->window, Scr->NormalGC,
		tb->srcx, tb->srcy, tb->width, tb->height,
		tb->dstx, tb->dsty);
    return;
}

void PaintTitleButtons (TwmWindow *tmp_win)
{
    int i;
    TBWindow *tbw;
    int nb = Scr->TBInfo.nleft + Scr->TBInfo.nright;

    for (i = 0, tbw = tmp_win->titlebuttons; i < nb; i++, tbw++) {
	if (tbw) PaintTitleButton (tmp_win, tbw);
    }
}

void adoptWindow (void)
{
    unsigned long	data [2];
    Window		localroot, w;
    unsigned char	*prop;
    unsigned long	bytesafter;
    unsigned long	len;
    Atom		actual_type;
    int			actual_format;
    XEvent		event;
    Window		root, parent, child, *children;
    unsigned int	nchildren, key_buttons;
    int			root_x, root_y, win_x, win_y;
    int			ret;

    localroot = w = RootWindow (dpy, Scr->screen);
    XGrabPointer (dpy, localroot, False,
			ButtonPressMask | ButtonReleaseMask,
			GrabModeAsync, GrabModeAsync,
		        None, Scr->SelectCursor, CurrentTime);

    XMaskEvent (dpy, ButtonPressMask | ButtonReleaseMask, &event);
    child = event.xbutton.subwindow;
    while (1) {
	if (child == (Window) 0) break;

	w = XmuClientWindow (dpy, child);
	ret = XGetWindowProperty (dpy, w, _XA_WM_WORKSPACESLIST, 0L, 512,
			False, XA_STRING, &actual_type, &actual_format, &len,
			&bytesafter, &prop);
	XFree ((char *)prop); /* Don't ever do anything with it */
	if (ret != Success)
		break;
	if (len == 0) /* it is not a local root window */
		break; /* it is not a local root window */
	localroot = w;
	XQueryPointer (dpy, localroot, &root, &child, &root_x, &root_y,
					&win_x, &win_y, &key_buttons);
    }
    XMaskEvent (dpy, ButtonPressMask | ButtonReleaseMask, &event);
    XUngrabPointer (dpy, CurrentTime);

    if (localroot == Scr->Root) return;
    if (w == localroot) {  /* try to not adopt an ancestor */
	XQueryTree (dpy, Scr->Root, &root, &parent, &children, &nchildren);
	while (parent != (Window) 0) {
	    XFree ((char *) children);
	    if (w == parent) return;
	    XQueryTree (dpy, parent, &root, &parent, &children, &nchildren);
	}
	XFree ((char *) children);
	if (w == root) return;
    }
    if (localroot == RootWindow (dpy, Scr->screen)) {
	XWithdrawWindow (dpy, w, Scr->screen);
    }
    else {
	XUnmapWindow (dpy, w);
    }
    XReparentWindow (dpy, w, Scr->Root, 0, 0);

    data [0] = (unsigned long) NormalState;
    data [1] = (unsigned long) None;

    XChangeProperty (dpy, w, _XA_WM_STATE, _XA_WM_STATE, 32, 
			PropModeReplace, (unsigned char *) data, 2);
    XFlush (dpy);
    SimulateMapRequest (w);
    return;
}

void DebugTrace (char *file)
{
    if (!file) return;
    if (tracefile) {
	fprintf (stderr, "stop logging events\n");
	if (tracefile != stderr) fclose (tracefile);
	tracefile = NULL;
    }
    else {
	if (strcmp (file, "stderr"))
	    tracefile = fopen (file, "w");
	else
	    tracefile = stderr;
	fprintf (stderr, "logging events to : %s\n", file);
    }
}

extern Cursor	TopRightCursor, TopLeftCursor, BottomRightCursor, BottomLeftCursor,
		LeftCursor, RightCursor, TopCursor, BottomCursor;

void SetBorderCursor (TwmWindow *tmp_win, int x, int y)
{
    Cursor cursor;
    XSetWindowAttributes attr;
    int h, fw, fh, wd;

    if (!tmp_win)
	return;

    /* Use the max of these, but since one is always 0 we can add them. */
    wd = tmp_win->frame_bw + tmp_win->frame_bw3D;
    h = Scr->TitleHeight + wd;
    fw = tmp_win->frame_width;
    fh = tmp_win->frame_height;

#if defined DEBUG && DEBUG
    fprintf(stderr, "wd=%d h=%d, fw=%d fh=%d x=%d y=%d\n",
	    wd, h, fw, fh, x, y);
#endif

    /*
     * If not using 3D borders:
     *
     * The left border has negative x coordinates,
     * The top border (above the title) has negative y coordinates.
     * The title is TitleHeight high, the next wd pixels are border.
     * The bottom border has coordinates >= the frame height.
     * The right border has coordinates >= the frame width.
     *
     * If using 3D borders: all coordinates are >= 0, and all coordinates
     * are higher by the border width.
     *
     * Since we only get events when we're actually in the border, we simply
     * allow for both cases at the same time.
     */

    if ((x < -wd) || (y < -wd)) {
	cursor = Scr->FrameCursor;
    } else if (x < h) {
	if (y < h)
	    cursor = TopLeftCursor;
	else if (y >= fh - h)
	    cursor = BottomLeftCursor;
	else
	    cursor = LeftCursor;
    } else if (x >= fw - h) {
	if (y < h)
	    cursor = TopRightCursor;
	else if (y >= fh - h)
	    cursor = BottomRightCursor;
	else
	    cursor = RightCursor;
    } else if (y < h) {	/* also include title bar in top border area */
	cursor = TopCursor;
    } else if (y >= fh - h) {
	cursor = BottomCursor;
    } else {
	cursor = Scr->FrameCursor;
    }
    attr.cursor = cursor;
    XChangeWindowAttributes (dpy, tmp_win->frame, CWCursor, &attr);
    tmp_win->curcurs = cursor;
}

Image *GetImage (char *name, ColorPair cp)
{
    name_list **list;
    char fullname [256];
    Image *image;

    if (name == NULL) return (None);
    image = None;

    list = &Scr->ImageCache;
#ifdef XPM
    if ((name [0] == '@') || (strncmp (name, "xpm:", 4) == 0)) {
	sprintf (fullname, "%s%dx%d", name, (int) cp.fore, (int) cp.back);

	if ((image = (Image*) LookInNameList (*list, fullname)) == None) {
	    int startn = (name [0] == '@') ? 1 : 4;
	    if ((image = GetXpmImage (name + startn, cp)) != None) {
	        AddToList (list, fullname, (char*) image);
	    }
	}
    }
    else
#endif
#ifdef JPEG
    if (strncmp (name, "jpeg:", 5) == 0) {
	if ((image = (Image*) LookInNameList (*list, name)) == None) {
	    if ((image = GetJpegImage (&name [5])) != None) {
		AddToList (list, name, (char*) image);
	    }
	}
    }
    else
#endif
#ifdef IMCONV
    if (strncmp (name, "im:", 3) == 0) {
	if ((image = (Image*) LookInNameList (*list, name)) == None) {
	    if ((image = GetImconvImage (&name [3])) != None) {
		AddToList (list, name, (char*) image);
	    }
	}
    }
    else
#endif
#if !defined(VMS) || defined(HAVE_XWDFILE_H)
    if ((strncmp (name, "xwd:", 4) == 0) || (name [0] == '|')) {
	int startn = (name [0] == '|') ? 0 : 4;
	if ((image = (Image*) LookInNameList (*list, name)) == None) {
	    if ((image = GetXwdImage (&name [startn], cp)) != None) {
		AddToList (list, name, (char*) image);
	    }
	}
    }
    else
#endif
    if (strncmp (name, ":xpm:", 5) == 0) {
	int    i;
	static struct {
	    char *name;
	    Image* (*proc)(ColorPair colorpair);
	} pmtab[] = {
	    { TBPM_3DDOT,	Create3DDotImage },
	    { TBPM_3DRESIZE,	Create3DResizeImage },
	    { TBPM_3DMENU,	Create3DMenuImage },
	    { TBPM_3DZOOM,	Create3DZoomImage },
	    { TBPM_3DBAR,	Create3DBarImage },
	    { TBPM_3DVBAR,	Create3DVertBarImage },
	    { TBPM_3DCROSS,     Create3DCrossImage },
	    { TBPM_3DICONIFY,   Create3DIconifyImage },
	    { TBPM_3DSUNKEN_RESIZE,     Create3DSunkenResizeImage },
	    { TBPM_3DBOX,       Create3DBoxImage }
	};
	
	sprintf (fullname, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
	if ((image = (Image*) LookInNameList (*list, fullname)) == None) {
	    for (i = 0; i < (sizeof pmtab) / (sizeof pmtab[0]); i++) {
		if (XmuCompareISOLatin1 (pmtab[i].name, name) == 0) {
		    image = (*pmtab[i].proc) (cp);
		    if (image == None) {
			fprintf (stderr,
			    "%s:  unable to build pixmap \"%s\"\n", ProgramName, name);
			return (None);
		    }
		    break;
		}
	    }
	    if (image == None) {
		fprintf (stderr, "%s:  no such built-in pixmap \"%s\"\n", ProgramName, name);
		return (None);
	    }
	    AddToList (list, fullname, (char*) image);
	}
    }
    else
    if (strncmp (name, "%xpm:", 5) == 0) {
	int    i;
	static struct {
	    char *name;
	    Image* (*proc)(ColorPair colorpair);
	} pmtab[] = {
	    { "%xpm:menu-up", Create3DMenuUpAnimation },
	    { "%xpm:menu-down", Create3DMenuDownAnimation },
	    { "%xpm:resize", Create3DZoomOutAnimation }, /* compatibility */
	    { "%xpm:resize-out-top", Create3DResizeInTopAnimation },
	    { "%xpm:resize-in-top", Create3DResizeOutTopAnimation },
	    { "%xpm:resize-out-bot", Create3DResizeInBotAnimation },
	    { "%xpm:resize-in-bot", Create3DResizeOutBotAnimation },
	    { "%xpm:maze-out", Create3DMazeOutAnimation },
	    { "%xpm:maze-in", Create3DMazeInAnimation },
	    { "%xpm:zoom-out", Create3DZoomOutAnimation },
	    { "%xpm:zoom-in", Create3DZoomInAnimation },
	    { "%xpm:zoom-inout", Create3DZoomInOutAnimation }
	};
	
	sprintf (fullname, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
	if ((image = (Image*) LookInNameList (*list, fullname)) == None) {
	    for (i = 0; i < (sizeof pmtab) / (sizeof pmtab[0]); i++) {
		if (XmuCompareISOLatin1 (pmtab[i].name, name) == 0) {
		    image = (*pmtab[i].proc) (cp);
		    if (image == None) {
			fprintf (stderr,
			    "%s:  unable to build pixmap \"%s\"\n", ProgramName, name);
			return (None);
		    }
		    break;
		}
	    }
	    if (image == None) {
		fprintf (stderr, "%s:  no such built-in pixmap \"%s\"\n", ProgramName, name);
		return (None);
	    }
	    AddToList (list, fullname, (char*) image);
	}
    }
    else
    if (name [0] == ':') {
	int		i;
	unsigned int	width, height;
	Pixmap		pm = 0;
	XGCValues	gcvalues;
	static struct {
	    char *name;
	    Pixmap (*proc)(unsigned int *widthp, unsigned int *heightp);
	} pmtab[] = {
	    { TBPM_DOT,		CreateDotPixmap },
	    { TBPM_ICONIFY,	CreateDotPixmap },
	    { TBPM_RESIZE,	CreateResizePixmap },
	    { TBPM_XLOGO,	CreateXLogoPixmap },
	    { TBPM_DELETE,	CreateXLogoPixmap },
	    { TBPM_MENU,	CreateMenuPixmap },
	    { TBPM_QUESTION,	CreateQuestionPixmap },
	};
	
	sprintf (fullname, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
	if ((image = (Image*) LookInNameList (*list, fullname)) == None) {
	    for (i = 0; i < (sizeof pmtab) / (sizeof pmtab[0]); i++) {
		if (XmuCompareISOLatin1 (pmtab[i].name, name) == 0) {
		    pm = (*pmtab[i].proc) (&width, &height);
		    if (pm == None) {
			fprintf (stderr,
			    "%s:  unable to build pixmap \"%s\"\n", ProgramName, name);
			return (None);
		    }
		    break;
		}
	    }
	    if (pm == None) {
		fprintf (stderr, "%s:  no such built-in bitmap \"%s\"\n", ProgramName, name);
		return (None);
	    }
	    image = (Image*) malloc (sizeof (struct _Image));
	    image->pixmap = XCreatePixmap (dpy, Scr->Root, width, height, Scr->d_depth);
	    if (Scr->rootGC == (GC) 0) Scr->rootGC = XCreateGC (dpy, Scr->Root, 0, &gcvalues);
	    gcvalues.background = cp.back;
	    gcvalues.foreground = cp.fore;
	    XChangeGC   (dpy, Scr->rootGC, GCForeground | GCBackground, &gcvalues);
	    XCopyPlane  (dpy, pm, image->pixmap, Scr->rootGC, 0, 0, width, height, 0, 0,
			(unsigned long) 1);
	    image->mask   = None;
	    image->width  = width;
	    image->height = height;
	    image->next   = None;
	    AddToList (list, fullname, (char*) image);
	}
    }
    else {
	sprintf (fullname, "%s%dx%d", name, (int) cp.fore, (int) cp.back);
	if ((image = (Image*) LookInNameList (*list, fullname)) == None) {
	    if ((image = GetBitmapImage (name, cp)) != None) {
		AddToList (list, fullname, (char*) image);
	    }
	}
    }
    return (image);
}

void FreeImage (Image *image)
{
    Image *im, *im2;

    im = image;
    while (im != None) {
	if (im->pixmap) XFreePixmap (dpy, im->pixmap);
	if (im->mask)   XFreePixmap (dpy, im->mask);
	im2 = im->next;
	free (im);
	im = im2;
    }
}

#if !defined(VMS) || defined(HAVE_XWDFILE_H)
static void compress (XImage *image, XColor *colors, int *ncolors);

static Image *LoadXwdImage (char *filename, ColorPair cp)
{
    FILE	*file;
    char	*fullname;
    XColor	colors [256];
    XWDColor	xwdcolors [256];
    unsigned	buffer_size;
    XImage	*image;
    unsigned char *imagedata;
    Pixmap	pixret;
    Visual	*visual;
    char	win_name [256];
    int		win_name_size;
    int		ispipe;
    int		i, len;
    int		w, h, depth, ncolors;
    int		scrn;
    Colormap	cmap;
    Colormap	stdcmap = Scr->RootColormaps.cwins[0]->colormap->c;
    GC		gc;
    XGCValues   gcvalues;
    XWDFileHeader header;
    Image	*ret;
    Bool	anim;
    unsigned long swaptest = 1;

    ispipe = 0;
    anim   = False;
#ifndef VMS
    if (filename [0] == '|') {
	file = (FILE*) popen (filename + 1, "r");
	if (file == NULL) return (None);
	ispipe = 1;
	anim = AnimationActive;
	if (anim) StopAnimation ();
	goto file_opened;
    }
#endif
    fullname = ExpandPixmapPath (filename);
    if (! fullname) return (None);
    file = fopen (fullname, "r");
    free (fullname);
    if (file == NULL) {
	if (reportfilenotfound) fprintf (stderr, "unable to locate %s\n", filename);
        return (None);
    }
file_opened:
    len = fread ((char *) &header, sizeof (header), 1, file);
    if (len != 1) {
	fprintf (stderr, "ctwm: cannot read %s\n", filename);
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }
    if (*(char *) &swaptest) _swaplong ((char *) &header, sizeof (header));
    if (header.file_version != XWD_FILE_VERSION) {
	fprintf(stderr,"ctwm: XWD file format version mismatch : %s\n", filename);
	return (None);
    }
    win_name_size = header.header_size - sizeof (header);
    len = fread (win_name, win_name_size, 1, file);
    if (len != 1) {
	fprintf (stderr, "file %s has not the correct format\n", filename);
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }

    if (header.pixmap_format == XYPixmap) {
	fprintf (stderr,"ctwm: XYPixmap XWD file not supported : %s\n", filename);
	return (None);
    }
    w       = header.pixmap_width;
    h       = header.pixmap_height;
    depth   = header.pixmap_depth;
    ncolors = header.ncolors;
    len = fread ((char *) xwdcolors, sizeof (XWDColor), ncolors, file);
    if (len != ncolors) {
	fprintf (stderr, "file %s has not the correct format\n", filename);
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }
    if (*(char *) &swaptest) {
	for (i = 0; i < ncolors; i++) {
	    _swaplong  ((char *) &xwdcolors [i].pixel, 4);
	    _swapshort ((char *) &xwdcolors [i].red, 3 * 2);
	}
    }
    for (i = 0; i < ncolors; i++) {
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
    gc      = DefaultGC (dpy, scrn);

    buffer_size = header.bytes_per_line * h;
    imagedata = (unsigned char*) malloc (buffer_size);
    if (! imagedata) {
	fprintf (stderr, "cannot allocate memory for image %s\n", filename);
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }
    len = fread (imagedata, (int) buffer_size, 1, file);
    if (len != 1) {
	free (imagedata);
	fprintf (stderr, "file %s has not the correct format\n", filename);
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }
#ifndef VMS
    if (ispipe)
      pclose (file);
    else
#endif
      fclose (file);

    image = XCreateImage (dpy, visual,  depth, header.pixmap_format,
                          0, (char*) imagedata, w, h,
                          header.bitmap_pad, header.bytes_per_line);
    if (image == None) {
	free (imagedata);
	fprintf (stderr, "cannot create image for %s\n", filename);
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }
    if (header.pixmap_format == ZPixmap) {
	compress (image, colors, &ncolors);
    }
    if (header.pixmap_format != XYBitmap) {
	for (i = 0; i < ncolors; i++) {
            XAllocColor (dpy, cmap, &(colors [i]));
	}
	for (i = 0; i < buffer_size; i++) {
            imagedata [i] = (unsigned char) colors [imagedata [i]].pixel;
	}
    }
    if (w > Scr->rootw)  w = Scr->rootw;
    if (h > Scr->rooth) h = Scr->rooth;

    ret = (Image*) malloc (sizeof (struct _Image));
    if (! ret) {
	fprintf (stderr, "unable to allocate memory for image : %s\n", filename);
	free (image);
	free (imagedata);
	for (i = 0; i < ncolors; i++) {
            XFreeColors (dpy, cmap, &(colors [i].pixel), 1, 0L);
	}
#ifdef USE_SIGNALS
	if (ispipe && anim) StartAnimation ();
#endif
	return (None);
    }
    if (header.pixmap_format == XYBitmap) {
	gcvalues.foreground = cp.fore;
	gcvalues.background = cp.back;
	XChangeGC (dpy, gc, GCForeground | GCBackground, &gcvalues);
    }
    if ((w > (Scr->rootw / 2)) || (h > (Scr->rooth / 2))) {
	int x, y;

	pixret = XCreatePixmap (dpy, Scr->Root, Scr->rootw,
				Scr->rooth, Scr->d_depth);
	x = (Scr->rootw  - w) / 2;
	y = (Scr->rooth - h) / 2;
	XFillRectangle (dpy, pixret, gc, 0, 0, Scr->rootw, Scr->rooth);
	XPutImage (dpy, pixret, gc, image, 0, 0, x, y, w, h);
	ret->width  = Scr->rootw;
	ret->height = Scr->rooth;
    }
    else {
	pixret = XCreatePixmap (dpy, Scr->Root, w, h, depth);
	XPutImage (dpy, pixret, gc, image, 0, 0, 0, 0, w, h);
	ret->width  = w;
	ret->height = h;
    }
    XDestroyImage (image);

    ret->pixmap = pixret;
    ret->mask   = None;
    ret->next   = None;
#ifdef USE_SIGNALS
    if (ispipe && anim) StartAnimation ();
#endif
    return (ret);
}

static Image *GetXwdImage (char *name, ColorPair cp)
{
    Image *image, *r, *s;
    char  path [128];
    char  pref [128], *perc;
    int   i;

    if (! strchr (name, '%')) return (LoadXwdImage (name, cp));
    s = image = None;
    strcpy (pref, name);
    perc  = strchr (pref, '%');
    *perc = '\0';
    reportfilenotfound = 0;
    for (i = 1;; i++) {
	sprintf (path, "%s%d%s", pref, i, perc + 1);
	r = LoadXwdImage (path, cp);
	if (r == None) break;
	r->next   = None;
	if (image == None) s = image = r;
	else {
	    s->next = r;
	    s = r;
	}
    }
    reportfilenotfound = 1;
    if (s != None) s->next = image;
    if (image == None) {
	fprintf (stderr, "Cannot open any %s xwd file\n", name);
    }
    return (image);
}

static void compress (XImage *image, XColor *colors, int *ncolors)
{
    unsigned char ind  [256];
    unsigned int  used [256];  
    int           i, j, size, nused;
    unsigned char color;
    XColor        newcolors [256];
    unsigned char *imagedata;

    for (i = 0; i < 256; i++) {
	used [i] = 0;
	ind  [i] = 0;
    }
    nused = 0;
    size  = image->bytes_per_line * image->height;
    imagedata = (unsigned char *) image->data;
    for (i = 0; i < size; i++) {
	if ((i % image->bytes_per_line) > image->width) continue;
        color = imagedata [i];
        if (used [color] == 0) {
            for (j = 0; j < nused; j++) {
                if ((colors [color].red   == newcolors [j].red)   &&
                    (colors [color].green == newcolors [j].green) &&
                    (colors [color].blue  == newcolors [j].blue)) break;
            }
            ind  [color] = j;
            used [color] = 1;
            if (j == nused) {
                newcolors [j].red   = colors [color].red;
                newcolors [j].green = colors [color].green;
                newcolors [j].blue  = colors [color].blue;
                nused++;
            }
        }
    }
    for (i = 0; i < size; i++) {
        imagedata [i] = ind [imagedata [i]];
    }
    for (i = 0; i < nused; i++) {
        colors [i] = newcolors [i];
    }
    *ncolors = nused;
}
#endif

#ifdef IMCONV

static void free_images  ();

static Image *GetImconvImage (char *filename,
			      unsigned int *widthp, unsigned int *heightp)
{
    TagTable		*toolInTable;
    ImVfb		*sourceVfb;
    ImVfbPtr		vptr;
    ImClt		*clt;
    int			i, j, ij, k, retval;

    XColor		colors [256];
    unsigned		buffer_size;
    XImage		*image;
    unsigned char	*imagedata;
    Pixmap		pixret;
    Visual		*visual;
    int			w, h, depth, ncolors;
    int			scrn;
    Colormap		cmap;
    Colormap		stdcmap = Scr->RootColormaps.cwins[0]->colormap->c;
    GC			gc;
    unsigned char	red, green, blue;
    int			icol;
    char		*fullname;

    TagEntry		*dataEntry;
    FILE		*fp;
    char		the_format[1024];
    char		*tmp_format;
    Image		*ret;

    if (*filename == NULL) return (None);
    fullname = ExpandPixmapPath (filename);
    if (! fullname) return (None);

    fp = fopen (fullname, "r");
    if (!fp) {
	if (reportfilenotfound) fprintf (stderr, "Cannot open the image %s\n", filename);
	free (fullname);
	return (None);
    }
    if ((toolInTable = TagTableAlloc ()) == TAGTABLENULL ) {
	fprintf (stderr, "TagTableAlloc failed\n");
	free_images (toolInTable);
	free (fullname);
	return (None);
    }
    if ((tmp_format = ImFileQFFormat (fp, fullname)) == NULL)  {
	fprintf (stderr, "Cannot determine image type of %s\n", filename);
	free_images  (toolInTable);
	free (fullname);
	return (None);
    }
    strcpy (the_format, tmp_format);
    retval = ImFileFRead (fp, the_format, NULL, toolInTable);
    if(retval < 0) {
	fprintf(stderr, "Cannot read image file %s: ", fullname);
	switch(ImErrNo) {
	    case IMESYS:
		fprintf (stderr, "System call error\n");
		break;
	    case IMEMALLOC:
		fprintf (stderr, "Cannot allocate memory\n");
		break;
	    case IMEFORMAT:
		fprintf (stderr, "Data in file is corrupt\n");
		break;
	    case IMENOREAD:
		fprintf (stderr, "Sorry, this format is write-only\n");
		break;
	    case IMEMAGIC:
		fprintf (stderr, "Bad magic number in image file\n");
		break;
	    case IMEDEPTH:
		fprintf (stderr, "Unknown image depth\n");
		break;
	    default:
		fprintf(stderr, "Unknown error\n");
		break;
	}
	free_images (toolInTable);
	free (fullname);
	return (None);
    }

    if (TagTableQNEntry (toolInTable, "image vfb") == 0)  {
	fprintf (stderr, "Image file %s contains no images\n", fullname);
	free_images (toolInTable);
	free (fullname);
	return (None);
    }
    dataEntry = TagTableQDirect (toolInTable, "image vfb", 0);
    TagEntryQValue (dataEntry, &sourceVfb);
    fclose (fp);

    w = ImVfbQWidth  (sourceVfb);
    h = ImVfbQHeight (sourceVfb);
    depth = 8 * ImVfbQNBytes (sourceVfb);
    if (depth != 8) {
	fprintf (stderr, "I don't know yet how to deal with images not of 8 planes depth\n");
	free_images (toolInTable);
	return (None);
    }

    *width  = w;
    *height = h;

    scrn   = Scr->screen;
    cmap   = AlternateCmap ? AlternateCmap : stdcmap;
    visual = Scr->d_visual;
    gc     = DefaultGC (dpy, scrn);

    buffer_size = w * h;
    imagedata = (unsigned char*) malloc (buffer_size);
    if (imagedata == (unsigned char*) 0) {
	fprintf (stderr, "Can't alloc enough space for background images\n");
	free_images (toolInTable);
	return (None);
    }

    clt  = ImVfbQClt   (sourceVfb);
    vptr = ImVfbQFirst (sourceVfb);
    ncolors = 0;
    for (i = 0; i < h - 1; i++) {
	for (j = 0; j < w; j++) {
	    ij = (i * w) + j;
	    red   = ImCltQRed   (ImCltQPtr (clt, ImVfbQIndex (sourceVfb, vptr)));
	    green = ImCltQGreen (ImCltQPtr (clt, ImVfbQIndex (sourceVfb, vptr)));
	    blue  = ImCltQBlue  (ImCltQPtr (clt, ImVfbQIndex (sourceVfb, vptr)));
	    for (k = 0; k < ncolors; k++) {
		if ((colors [k].red   == red) &&
		    (colors [k].green == green) &&
		    (colors [k].blue  == blue)) {
		    icol = k;
		    break;
		}
	    }
	    if (k == ncolors) {
		icol = ncolors;
		ncolors++;
	    }
	    imagedata [ij] = icol;
	    colors [icol].red   = red;
	    colors [icol].green = green;
	    colors [icol].blue  = blue;
	    ImVfbSInc (sourceVfb, vptr);
	}
    }
    for (i = 0; i < ncolors; i++) {
	colors [i].red   *= 256;
	colors [i].green *= 256;
	colors [i].blue  *= 256;
    }
    for (i = 0; i < ncolors; i++) {
        if (! XAllocColor (dpy, cmap, &(colors [i]))) {
	    fprintf (stderr, "can't alloc color for image %s\n", filename);
	}
    }
    for (i = 0; i < buffer_size; i++) {
        imagedata [i] = (unsigned char) colors [imagedata [i]].pixel;
    }

    image  = XCreateImage  (dpy, visual, depth, ZPixmap, 0, (char*) imagedata, w, h, 8, 0);
    if (w > Scr->rootw)  w = Scr->rootw;
    if (h > Scr->rooth) h = Scr->rooth;

    if ((w > (Scr->rootw / 2)) || (h > (Scr->rooth / 2))) {
	int x, y;

	pixret = XCreatePixmap (dpy, Scr->Root, Scr->rootw, Scr->rooth, depth);
	x = (Scr->rootw  - w) / 2;
	y = (Scr->rooth - h) / 2;
	XFillRectangle (dpy, pixret, gc, 0, 0, Scr->rootw, Scr->rooth);
	XPutImage (dpy, pixret, gc, image, 0, 0, x, y, w, h);
	ret->width  = Scr->rootw;
	ret->height = Scr->rooth;
    }
    else {
	pixret = XCreatePixmap (dpy, Scr->Root, w, h, depth);
	XPutImage (dpy, pixret, gc, image, 0, 0, 0, 0, w, h);
	ret->width  = w;
	ret->height = h;
    }
    XFree (image);
    ret = (Image*) malloc (sizeof (struct _Image));
    ret->pixmap = pixret;
    ret->mask   = None;
    ret->next   = None;
    return (ret);

}

static void free_images (table)
TagTable *table;
{
    int		i, n;
    ImVfb	*v;
    ImClt	*c;
    TagEntry	*dataEntry;

    n = TagTableQNEntry (table, "image vfb");
    for (i = 0 ; i < n ; i++) {
	dataEntry = TagTableQDirect (table, "image vfb", i);
	TagEntryQValue (dataEntry, &v);
	ImVfbFree (v);
    }
    n = TagTableQNEntry (table, "image clt");
    for (i = 0 ; i < n ; i++) {
	dataEntry = TagTableQDirect (table, "image clt", i );
	TagEntryQValue (dataEntry, &c);
	ImCltFree (c);
    }
    TagTableFree (table);
}

#endif

void _swapshort (register char *bp, register unsigned n)
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
	c = *bp;
	*bp = *(bp + 1);
	bp++;
	*bp++ = c;
    }
}

void _swaplong (register char *bp, register unsigned n)
{
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while (bp < ep) {
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

/***********************************************************************
 *
 *  Procedure:
 *	GetWMPropertyString - Get Window Manager text property and
 *				convert it to a string.
 *
 *  Returned Value:
 *	(char *) - pointer to the malloc'd string or NULL
 *
 *  Inputs:
 *	w	- the id of the window whose property is to be retrieved
 *	prop	- property atom (typically WM_NAME or WM_ICON_NAME)
 *
 ***********************************************************************
 */

unsigned char *GetWMPropertyString(Window w, Atom prop)
{
    XTextProperty	text_prop;
    char 		**text_list;
    int 		text_list_count;
    Atom 		XA_COMPOUND_TEXT = XInternAtom(dpy, "COMPOUND_TEXT", False);
    unsigned char	*stringptr;
    int			status, len = -1;

    (void)XGetTextProperty(dpy, w, &text_prop, prop);
    if (text_prop.value != NULL) {
	if (text_prop.encoding == XA_STRING
	    || text_prop.encoding == XA_COMPOUND_TEXT) {
	    /* property is encoded as compound text - convert to locale string */
	    status = XmbTextPropertyToTextList(dpy, &text_prop,
					       &text_list, &text_list_count);
	    if (text_list_count == 0) {
		stringptr = NULL;
	    } else
	    if (text_list == (char **)0) {
		stringptr = NULL;
	    } else
	    if (text_list [0] == (char *)0) {
		stringptr = NULL;
	    } else
	    if (status < 0 || text_list_count < 0) {
		switch (status) {
		case XConverterNotFound:
		    fprintf (stderr, "%s: Converter not found; unable to convert property %s of window ID %lx.\n",
			     ProgramName, XGetAtomName(dpy, prop), w);
		    break;
		case XNoMemory:
		    fprintf (stderr, "%s: Insufficient memory; unable to convert property %s of window ID %lx.\n",
			     ProgramName, XGetAtomName(dpy, prop), w);
		    break;
		case XLocaleNotSupported:
		    fprintf (stderr, "%s: Locale not supported; unable to convert property %s of window ID %lx.\n",
			     ProgramName, XGetAtomName(dpy, prop), w);
		    break;
		}
		stringptr = NULL;
		/*
		   don't call XFreeStringList - text_list appears to have
		   invalid address if status is bad
		   XFreeStringList(text_list);
		*/
	    } else {
		len = strlen(text_list[0]);
		stringptr = memcpy(malloc(len+1), text_list[0], len+1);
		XFreeStringList(text_list);
	    }
	} else {
	    /* property is encoded in a format we don't understand */
	    fprintf (stderr, "%s: Encoding not STRING or COMPOUND_TEXT; unable to decode property %s of window ID %lx.\n",
		     ProgramName, XGetAtomName(dpy, prop), w);
	    stringptr = NULL;
	}
	XFree (text_prop.value); 
    } else {
	stringptr = NULL;
    }

    return stringptr;
}

void FreeWMPropertyString(char *prop)
{
    if (prop && (char *)prop != NoName) {
	free(prop);
    }
}

static void ConstrainLeftTop (int *value, int border)
{
  if (*value < border) {
    if (Scr->MoveOffResistance < 0 ||
	*value > border - Scr->MoveOffResistance)
    {
        *value = border;
    } else if (Scr->MoveOffResistance > 0 &&
	       *value <= border - Scr->MoveOffResistance)
    {
      *value = *value + Scr->MoveOffResistance;
    }
    }
}

static void ConstrainRightBottom (int *value, int size1, int border, int size2)
{
    if (*value + size1 > size2 - border) {
      if (Scr->MoveOffResistance < 0 ||
         *value + size1 < size2 - border + Scr->MoveOffResistance)
    {
        *value = size2 - size1 - border;
	} else if (Scr->MoveOffResistance > 0 &&
		   *value + size1 >= size2 - border + Scr->MoveOffResistance) {
	  *value = *value - Scr->MoveOffResistance;
	}
    }
}

void ConstrainByBorders1 (int *left, int width, int *top, int height)
{
    ConstrainRightBottom (left, width, Scr->BorderRight, Scr->rootw);
    ConstrainLeftTop     (left, Scr->BorderLeft);
    ConstrainRightBottom (top, height, Scr->BorderBottom, Scr->rooth);
    ConstrainLeftTop     (top, Scr->BorderTop);
}

void ConstrainByBorders (TwmWindow *twmwin,
			 int *left, int width, int *top, int height)
{
    if (twmwin->winbox) {
	XWindowAttributes attr;
	XGetWindowAttributes (dpy, twmwin->winbox->window, &attr);
	ConstrainRightBottom (left, width, 0, attr.width);
	ConstrainLeftTop     (left, 0);
	ConstrainRightBottom (top, height, 0, attr.height);
	ConstrainLeftTop     (top, 0);
    } else {
	ConstrainByBorders1 (left, width, top, height);
    }
}

#ifdef JPEG

uint16_t *buffer_16bpp;
uint32_t *buffer_32bpp;

static void convert_for_16 (int w, int x, int y, int r, int g, int b) {
  buffer_16bpp [y * w + x] = ((r >> 3) << 11) + ((g >> 2) << 5) + (b >> 3);
}

static void convert_for_32 (int w, int x, int y, int r, int g, int b) {
  buffer_32bpp [y * w + x] = ((r << 16) + (g << 8) + b) & 0xFFFFFFFF; 
}

static void jpeg_error_exit (j_common_ptr cinfo) {
  jerr_ptr errmgr = (jerr_ptr) cinfo->err;
  cinfo->err->output_message (cinfo);
  siglongjmp (errmgr->setjmp_buffer, 1);
  return;
}

static Image *GetJpegImage (char *name)
{
    Image *image, *r, *s;
    char  path [128];
    char  pref [128], *perc;
    int   i;

    if (! strchr (name, '%')) return (LoadJpegImage (name));
    s = image = None;
    strcpy (pref, name);
    perc  = strchr (pref, '%');
    *perc = '\0';
    reportfilenotfound = 0;
    for (i = 1;; i++) {
	sprintf (path, "%s%d%s", pref, i, perc + 1);
	r = LoadJpegImage (path);
	if (r == None) break;
	r->next   = None;
	if (image == None) s = image = r;
	else {
	    s->next = r;
	    s = r;
	}
    }
    reportfilenotfound = 1;
    if (s != None) s->next = image;
    if (image == None) {
	fprintf (stderr, "Cannot open any %s jpeg file\n", name);
    }
    return (image);
}

static Image *LoadJpegImage (char *name)
{
  char   *fullname;
  XImage *ximage;
  FILE   *infile;
  Image  *image;
  Pixmap pixret;
  void   (*store_data) (int w, int x, int y, int r, int g, int b);
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error jerr;
  JSAMPARRAY buffer;
  int width, height;
  int row_stride;
  int g, i, a;
  int bpix;
  GC  gc;

  fullname = ExpandPixmapPath (name);
  if (! fullname) return (None);

  image = (Image*) malloc (sizeof (struct _Image));
  if (image == None) return (None);

  if ((infile = fopen (fullname, "rb")) == NULL) {
    if (!reportfilenotfound) fprintf (stderr, "unable to locate %s\n", fullname);
    fflush (stdout);
    return None;
  }
  cinfo.err = jpeg_std_error (&jerr.pub);
  jerr.pub.error_exit = jpeg_error_exit;

  if (sigsetjmp(jerr.setjmp_buffer, 1)) {
    jpeg_destroy_decompress (&cinfo);
    fclose (infile);
    return None;
  }
  jpeg_create_decompress (&cinfo);
  jpeg_stdio_src (&cinfo, infile);
  jpeg_read_header (&cinfo, FALSE);
  cinfo.do_fancy_upsampling = FALSE;
  cinfo.do_block_smoothing = FALSE;
  jpeg_start_decompress (&cinfo);
  width  = cinfo.output_width;
  height = cinfo.output_height;

  if (Scr->d_depth == 16) {
    store_data = &convert_for_16;
    buffer_16bpp = (unsigned short int *) malloc ((width) * (height) * 2);
    ximage = XCreateImage (dpy, CopyFromParent, Scr->d_depth, ZPixmap, 0,
			   (char *) buffer_16bpp, width, height, 16, width * 2);
  } else {
    if (Scr->d_depth == 24) {
      store_data = &convert_for_32;
      buffer_32bpp = malloc (width * height * 4);
      ximage = XCreateImage (dpy, CopyFromParent, Scr->d_depth, ZPixmap, 0,
			     (char *) buffer_32bpp, width, height, 32, width * 4);
    } else
    if (Scr->d_depth == 32) {
      store_data = &convert_for_32;
      buffer_32bpp = malloc (width * height * 4);
      ximage = XCreateImage (dpy, CopyFromParent, Scr->d_depth, ZPixmap, 0,
			     (char *) buffer_32bpp, width, height, 32, width * 4);
    } else {
      fprintf (stderr, "Image %s unsupported depth : %d\n", name, Scr->d_depth);
      return None;
    }
  }
  if (ximage == None) {
    fprintf (stderr, "cannot create image for %s\n", name);
  }
  g = 0;
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

  bpix = cinfo.output_components;
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines (&cinfo, buffer, 1);
    a = 0;
    for (i = 0; i < bpix * cinfo.output_width; i += bpix) {
      (*store_data) (width, a, g, buffer[0][i],  buffer[0][i + 1], buffer[0][i + 2]);
      a++;
    }
    g++;
  }
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  fclose (infile);

  gc = DefaultGC (dpy, Scr->screen);
  if ((width > (Scr->rootw / 2)) || (height > (Scr->rooth / 2))) {
    int x, y;

    pixret = XCreatePixmap (dpy, Scr->Root, Scr->rootw, Scr->rooth, Scr->d_depth);
    x = (Scr->rootw  -  width) / 2;
    y = (Scr->rooth  - height) / 2;
    XFillRectangle (dpy, pixret, gc, 0, 0, Scr->rootw, Scr->rooth);
    XPutImage (dpy, pixret, gc, ximage, 0, 0, x, y, width, height);
    image->width  = Scr->rootw;
    image->height = Scr->rooth;
  } else {
    pixret = XCreatePixmap (dpy, Scr->Root, width, height, Scr->d_depth);
    XPutImage (dpy, pixret, gc, ximage, 0, 0, 0, 0, width, height);
    image->width  = width;
    image->height = height;
  }
  XDestroyImage (ximage);
  image->pixmap = pixret;
  image->mask   = None;
  image->next   = None;

  return image;
}

#endif /* JPEG */
