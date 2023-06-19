/*
 * xdpyinfo - print information about X display connection
 *
 *
Copyright 1988, 1998  The Open Group
Copyright 2005 Hitachi, Ltd.

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
# if HAVE_X11_EXTENSIONS_MULTIBUF_H
#  define MULTIBUFFER
# endif

# if HAVE_X11_EXTENSIONS_XSHM_H
#  define MITSHM
# endif

# if HAVE_X11_EXTENSIONS_XKB_H && HAVE_X11_XKBLIB_H
#  define XKB
# endif

# if HAVE_X11_EXTENSIONS_XF86VMODE_H && \
	(HAVE_X11_EXTENSIONS_XF86VMSTR_H || HAVE_X11_EXTENSIONS_XF86VMPROTO_H)
#  define XF86VIDMODE
# endif

# if (HAVE_X11_EXTENSIONS_XXF86DGA_H && HAVE_X11_EXTENSIONS_XF86DGAPROTO_H) \
  || (HAVE_X11_EXTENSIONS_XF86DGA_H && HAVE_X11_EXTENSIONS_XF86DGASTR_H)
#  define XFreeXDGA
# endif

# if HAVE_X11_EXTENSIONS_XF86MISC_H && HAVE_X11_EXTENSIONS_XF86MSCSTR_H
#  define XF86MISC
# endif

# if HAVE_X11_EXTENSIONS_XINPUT_H
#  define XINPUT
# endif

# if HAVE_X11_EXTENSIONS_XRENDER_H
#  define XRENDER
# endif

# if HAVE_X11_EXTENSIONS_XCOMPOSITE_H
#  define COMPOSITE
# endif

# if HAVE_X11_EXTENSIONS_XINERAMA_H
#  define PANORAMIX
# endif

# if HAVE_X11_EXTENSIONS_DMXEXT_H
#  define DMX
# endif

#endif

#ifdef WIN32
#include <X11/Xwindows.h>
#endif

#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef MULTIBUFFER
#include <X11/extensions/multibuf.h>
#endif
#include <X11/extensions/XTest.h>
#include <X11/extensions/sync.h>
#include <X11/Xproto.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/record.h>
#include <X11/extensions/shape.h>
#ifdef MITSHM
#include <X11/extensions/XShm.h>
#endif
#ifdef XKB
#include <X11/extensions/XKB.h>
#include <X11/XKBlib.h>
#endif
#ifdef XF86VIDMODE
#include <X11/extensions/xf86vmode.h>
# if HAVE_X11_EXTENSIONS_XF86VMPROTO_H /* xf86vidmodeproto 2.2.99.1 & later */
#  include <X11/extensions/xf86vmproto.h>
# else
#  include <X11/extensions/xf86vmstr.h>
# endif
#endif
#ifdef XFreeXDGA
# if HAVE_X11_EXTENSIONS_XXF86DGA_H && HAVE_X11_EXTENSIONS_XF86DGAPROTO_H
#  include <X11/extensions/Xxf86dga.h>
#  include <X11/extensions/xf86dgaproto.h>
# else
#  include <X11/extensions/xf86dga.h>
#  include <X11/extensions/xf86dgastr.h>
# endif
#endif
#ifdef XF86MISC
#include <X11/extensions/xf86misc.h>
#include <X11/extensions/xf86mscstr.h>
#endif
#ifdef XINPUT
#include <X11/extensions/XInput.h>
#endif
#ifdef XRENDER
#include <X11/extensions/Xrender.h>
#endif
#ifdef COMPOSITE
#include <X11/extensions/Xcomposite.h>
#endif
#ifdef PANORAMIX
#include <X11/extensions/Xinerama.h>
#endif
#ifdef DMX
#include <X11/extensions/dmxext.h>
#endif
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>

static char *ProgramName;
static Bool queryExtensions = False;

#if defined(XF86MISC) || defined(XFreeXDGA)
static int
silent_errors(_X_UNUSED Display *dpy, _X_UNUSED XErrorEvent *ev)
{
    return 0;
}

static int (*old_handler)(Display *, XErrorEvent *) = NULL;
#endif

static int print_event_mask(char *buf, int lastcol, int indent, long mask);

static int StrCmp(const void *a, const  void *b)
{
    return strcmp(*(const char * const *)a, *(const char * const *)b);
}

static void
print_extension_info(Display *dpy)
{
    int n = 0;
    char **extlist = XListExtensions (dpy, &n);

    printf ("number of extensions:    %d\n", n);

    if (extlist) {
	qsort(extlist, (size_t)n, sizeof(char *), StrCmp);

	if (!queryExtensions) {
	    for (int i = 0; i < n; i++) {
		printf ("    %s\n", extlist[i]);
	    }
	} else {
	    xcb_connection_t *xcb_conn = XGetXCBConnection (dpy);
	    xcb_query_extension_cookie_t *qe_cookies;

	    qe_cookies = calloc((size_t)n, sizeof(xcb_query_extension_cookie_t));
	    if (!qe_cookies) {
		perror ("calloc failed to allocate memory for extensions");
		return;
	    }

	    /*
	     * Generate all extension queries at once, so they can be
	     * sent to the xserver in a single batch
	     */
	    for (int i = 0; i < n; i++) {
		qe_cookies[i] = xcb_query_extension (xcb_conn,
						     (uint16_t)strlen(extlist[i]),
						     extlist[i]);
	    }

	    /*
	     * Start processing replies as they come in.
	     * The first call will flush the queue to the server, then
	     * each one will wait, if needed, for its reply.
	     */
	    for (int i = 0; i < n; i++) {
		xcb_query_extension_reply_t *rep
		    = xcb_query_extension_reply(xcb_conn, qe_cookies[i], NULL);

		printf ("    %s  (opcode: %d", extlist[i], rep->major_opcode);
		if (rep->first_event)
		    printf (", base event: %d", rep->first_event);
		if (rep->first_error)
		    printf (", base error: %d", rep->first_error);
		printf (")\n");

		free (rep);
	    }
	    free (qe_cookies);
	}
	/* do not free, Xlib can depend on contents being unaltered */
	/* XFreeExtensionList (extlist); */
    }
}

static void
print_display_info(Display *dpy)
{
    char dummybuf[40];
    const char *cp;
    int minkeycode, maxkeycode;
    int n;
    long req_size;
    XPixmapFormatValues *pmf;
    Window focuswin;
    int focusrevert;

    printf ("name of display:    %s\n", DisplayString (dpy));
    printf ("version number:    %d.%d\n",
	    ProtocolVersion (dpy), ProtocolRevision (dpy));
    printf ("vendor string:    %s\n", ServerVendor (dpy));
    printf ("vendor release number:    %d\n", VendorRelease (dpy));

    if (strstr(ServerVendor (dpy), "X.Org")) {
	int vendrel = VendorRelease(dpy);

	printf("X.Org version: ");
        if (vendrel >= 12100000) {
            vendrel -= 10000000; /* Y2.1K compliant */
            printf("%d.%d",
	       (vendrel /   100000) % 100,
	       (vendrel /     1000) % 100);
        } else {
            printf("%d.%d.%d", vendrel / 10000000,
                   (vendrel /   100000) % 100,
                   (vendrel /     1000) % 100);
        }
        if (vendrel % 1000)
            printf(".%d", vendrel % 1000);
        printf("\n");
    }
    else if (strstr(ServerVendor (dpy), "XFree86")) {
	int vendrel = VendorRelease(dpy);

	printf("XFree86 version: ");
	if (vendrel < 336) {
	    /*
	     * vendrel was set incorrectly for 3.3.4 and 3.3.5, so handle
	     * those cases here.
	     */
	    printf("%d.%d.%d", vendrel / 100,
			      (vendrel / 10) % 10,
			       vendrel       % 10);
	} else if (vendrel < 3900) {
	    /* 3.3.x versions, other than the exceptions handled above */
	    printf("%d.%d", vendrel / 1000,
			   (vendrel /  100) % 10);
	    if (((vendrel / 10) % 10) || (vendrel % 10)) {
		printf(".%d", (vendrel / 10) % 10);
		if (vendrel % 10) {
		    printf(".%d", vendrel % 10);
		}
	    }
	} else if (vendrel < 40000000) {
	    /* 4.0.x versions */
	    printf("%d.%d", vendrel / 1000,
			   (vendrel /   10) % 10);
	    if (vendrel % 10) {
		printf(".%d", vendrel % 10);
	    }
	} else {
	    /* post-4.0.x */
	    printf("%d.%d.%d", vendrel / 10000000,
			      (vendrel /   100000) % 100,
			      (vendrel /     1000) % 100);
	    if (vendrel % 1000) {
		printf(".%d", vendrel % 1000);
	    }
	}
	printf("\n");
    }

    if (strstr(ServerVendor (dpy), "DMX")) {
	int vendrel = VendorRelease(dpy);
        int major, minor, year, month, day;

        major    = vendrel / 100000000;
        vendrel -= major   * 100000000;
        minor    = vendrel /   1000000;
        vendrel -= minor   *   1000000;
        year     = vendrel /     10000;
        vendrel -= year    *     10000;
        month    = vendrel /       100;
        vendrel -= month   *       100;
        day      = vendrel;

                                /* Add other epoch tests here */
        if (major > 0 && minor > 0) year += 2000;

                                /* Do some sanity tests in case there is
                                 * another server with the same vendor
                                 * string.  That server could easily use
                                 * values < 100000000, which would have
                                 * the effect of keeping our major
                                 * number 0. */
        if (major > 0 && major <= 20
            && minor >= 0 && minor <= 99
            && year >= 2000
            && month >= 1 && month <= 12
            && day >= 1 && day <= 31)
            printf("DMX version: %d.%d.%04d%02d%02d\n",
                   major, minor, year, month, day);
    }

    req_size = XExtendedMaxRequestSize (dpy);
    if (!req_size) req_size = XMaxRequestSize (dpy);
    printf ("maximum request size:  %ld bytes\n", req_size * 4);
    printf ("motion buffer size:  %ld\n", XDisplayMotionBufferSize (dpy));

    switch (BitmapBitOrder (dpy)) {
      case LSBFirst:    cp = "LSBFirst"; break;
      case MSBFirst:    cp = "MSBFirst"; break;
      default:
	snprintf (dummybuf, sizeof(dummybuf),
                  "unknown order %d", BitmapBitOrder (dpy));
	cp = dummybuf;
	break;
    }
    printf ("bitmap unit, bit order, padding:    %d, %s, %d\n",
	    BitmapUnit (dpy), cp, BitmapPad (dpy));

    switch (ImageByteOrder (dpy)) {
      case LSBFirst:    cp = "LSBFirst"; break;
      case MSBFirst:    cp = "MSBFirst"; break;
      default:
	snprintf (dummybuf, sizeof(dummybuf),
                  "unknown order %d", ImageByteOrder (dpy));
	cp = dummybuf;
	break;
    }
    printf ("image byte order:    %s\n", cp);

    pmf = XListPixmapFormats (dpy, &n);
    printf ("number of supported pixmap formats:    %d\n", n);
    if (pmf) {
	printf ("supported pixmap formats:\n");
	for (int i = 0; i < n; i++) {
	    printf ("    depth %d, bits_per_pixel %d, scanline_pad %d\n",
		    pmf[i].depth, pmf[i].bits_per_pixel, pmf[i].scanline_pad);
	}
	XFree ((char *) pmf);
    }


    /*
     * when we get interfaces to the PixmapFormat stuff, insert code here
     */

    XDisplayKeycodes (dpy, &minkeycode, &maxkeycode);
    printf ("keycode range:    minimum %d, maximum %d\n",
	    minkeycode, maxkeycode);

    XGetInputFocus (dpy, &focuswin, &focusrevert);
    printf ("focus:  ");
    switch (focuswin) {
      case PointerRoot:
	printf ("PointerRoot\n");
	break;
      case None:
	printf ("None\n");
	break;
      default:
	printf("window 0x%lx, revert to ", focuswin);
	switch (focusrevert) {
	  case RevertToParent:
	    printf ("Parent\n");
	    break;
	  case RevertToNone:
	    printf ("None\n");
	    break;
	  case RevertToPointerRoot:
	    printf ("PointerRoot\n");
	    break;
	  default:			/* should not happen */
	    printf ("%d\n", focusrevert);
	    break;
	}
	break;
    }

    print_extension_info (dpy);

    printf ("default screen number:    %d\n", DefaultScreen (dpy));
    printf ("number of screens:    %d\n", ScreenCount (dpy));
}

static void
print_visual_info(XVisualInfo *vip)
{
    char errorbuf[40];			/* for sprintfing into */
    const char *class = NULL;		/* for printing */

    switch (vip->class) {
      case StaticGray:    class = "StaticGray"; break;
      case GrayScale:    class = "GrayScale"; break;
      case StaticColor:    class = "StaticColor"; break;
      case PseudoColor:    class = "PseudoColor"; break;
      case TrueColor:    class = "TrueColor"; break;
      case DirectColor:    class = "DirectColor"; break;
      default:
	snprintf (errorbuf, sizeof(errorbuf), "unknown class %d", vip->class);
	class = errorbuf;
	break;
    }

    printf ("  visual:\n");
    printf ("    visual id:    0x%lx\n", vip->visualid);
    printf ("    class:    %s\n", class);
    printf ("    depth:    %d plane%s\n", vip->depth,
	    vip->depth == 1 ? "" : "s");
    if (vip->class == TrueColor || vip->class == DirectColor)
	printf ("    available colormap entries:    %d per subfield\n",
		vip->colormap_size);
    else
	printf ("    available colormap entries:    %d\n",
		vip->colormap_size);
    printf ("    red, green, blue masks:    0x%lx, 0x%lx, 0x%lx\n",
	    vip->red_mask, vip->green_mask, vip->blue_mask);
    printf ("    significant bits in color specification:    %d bits\n",
	    vip->bits_per_rgb);
}

static void
print_screen_info(Display *dpy, int scr)
{
    Screen *s = ScreenOfDisplay (dpy, scr);  /* opaque structure */
    XVisualInfo viproto;		/* fill in for getting info */
    XVisualInfo *vip;			/* returned info */
    int nvi;				/* number of elements returned */
    char eventbuf[80];			/* want 79 chars per line + nul */
    static const char *yes = "YES", *no = "NO", *when = "WHEN MAPPED";
    double xres, yres;
    int ndepths = 0, *depths = NULL;
    unsigned int width, height;

    /*
     * there are 2.54 centimeters to an inch; so there are 25.4 millimeters.
     *
     *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
     *         = N pixels / (M inch / 25.4)
     *         = N * 25.4 pixels / M inch
     */

    xres = ((((double) DisplayWidth(dpy,scr)) * 25.4) /
	    ((double) DisplayWidthMM(dpy,scr)));
    yres = ((((double) DisplayHeight(dpy,scr)) * 25.4) /
	    ((double) DisplayHeightMM(dpy,scr)));

    printf ("\n");
    printf ("screen #%d:\n", scr);
    printf ("  dimensions:    %dx%d pixels (%dx%d millimeters)\n",
	    XDisplayWidth (dpy, scr),  XDisplayHeight (dpy, scr),
	    XDisplayWidthMM(dpy, scr), XDisplayHeightMM (dpy, scr));
    printf ("  resolution:    %dx%d dots per inch\n",
	    (int) (xres + 0.5), (int) (yres + 0.5));
    depths = XListDepths (dpy, scr, &ndepths);
    if (!depths) ndepths = 0;
    printf ("  depths (%d):    ", ndepths);
    for (int i = 0; i < ndepths; i++) {
	printf ("%d", depths[i]);
	if (i < ndepths - 1) {
	    putchar (',');
	    putchar (' ');
	}
    }
    putchar ('\n');
    if (depths) XFree ((char *) depths);
    printf ("  root window id:    0x%lx\n", RootWindow (dpy, scr));
    printf ("  depth of root window:    %d plane%s\n",
	    DisplayPlanes (dpy, scr),
	    DisplayPlanes (dpy, scr) == 1 ? "" : "s");
    printf ("  number of colormaps:    minimum %d, maximum %d\n",
	    MinCmapsOfScreen(s), MaxCmapsOfScreen(s));
    printf ("  default colormap:    0x%lx\n", DefaultColormap (dpy, scr));
    printf ("  default number of colormap cells:    %d\n",
	    DisplayCells (dpy, scr));
    printf ("  preallocated pixels:    black %ld, white %ld\n",
	    BlackPixel (dpy, scr), WhitePixel (dpy, scr));
    printf ("  options:    backing-store %s, save-unders %s\n",
	    (DoesBackingStore (s) == NotUseful) ? no :
	    ((DoesBackingStore (s) == Always) ? yes : when),
	    DoesSaveUnders (s) ? yes : no);
    XQueryBestSize (dpy, CursorShape, RootWindow (dpy, scr), 65535, 65535,
		    &width, &height);
    if (width == 65535 && height == 65535)
	printf ("  largest cursor:    unlimited\n");
    else
	printf ("  largest cursor:    %dx%d\n", width, height);
    printf ("  current input event mask:    0x%lx\n", EventMaskOfScreen (s));
    (void) print_event_mask (eventbuf, 79, 4, EventMaskOfScreen (s));

    nvi = 0;
    viproto.screen = scr;
    vip = XGetVisualInfo (dpy, VisualScreenMask, &viproto, &nvi);
    printf ("  number of visuals:    %d\n", nvi);
    printf ("  default visual id:  0x%lx\n",
	    XVisualIDFromVisual (DefaultVisual (dpy, scr)));
    for (int i = 0; i < nvi; i++) {
	print_visual_info (vip+i);
    }
    if (vip) XFree ((char *) vip);
}

/*
 * The following routine prints out an event mask, wrapping events at nice
 * boundaries.
 */

#define MASK_NAME_WIDTH 25

static struct _event_table {
    const char *name;
    long value;
} event_table[] = {
    { "KeyPressMask             ", KeyPressMask },
    { "KeyReleaseMask           ", KeyReleaseMask },
    { "ButtonPressMask          ", ButtonPressMask },
    { "ButtonReleaseMask        ", ButtonReleaseMask },
    { "EnterWindowMask          ", EnterWindowMask },
    { "LeaveWindowMask          ", LeaveWindowMask },
    { "PointerMotionMask        ", PointerMotionMask },
    { "PointerMotionHintMask    ", PointerMotionHintMask },
    { "Button1MotionMask        ", Button1MotionMask },
    { "Button2MotionMask        ", Button2MotionMask },
    { "Button3MotionMask        ", Button3MotionMask },
    { "Button4MotionMask        ", Button4MotionMask },
    { "Button5MotionMask        ", Button5MotionMask },
    { "ButtonMotionMask         ", ButtonMotionMask },
    { "KeymapStateMask          ", KeymapStateMask },
    { "ExposureMask             ", ExposureMask },
    { "VisibilityChangeMask     ", VisibilityChangeMask },
    { "StructureNotifyMask      ", StructureNotifyMask },
    { "ResizeRedirectMask       ", ResizeRedirectMask },
    { "SubstructureNotifyMask   ", SubstructureNotifyMask },
    { "SubstructureRedirectMask ", SubstructureRedirectMask },
    { "FocusChangeMask          ", FocusChangeMask },
    { "PropertyChangeMask       ", PropertyChangeMask },
    { "ColormapChangeMask       ", ColormapChangeMask },
    { "OwnerGrabButtonMask      ", OwnerGrabButtonMask },
    { NULL, 0 }};

static int
print_event_mask(char *buf,     /* string to write into */
                 int lastcol,   /* strlen(buf)+1 */
                 int indent,    /* amount by which to indent */
                 long mask)     /* event mask */
{
    int len;
    int bitsfound = 0;

    buf[0] = buf[lastcol] = '\0';	/* just in case */

#define INDENT() do { len = indent; memset(buf, ' ', indent); } while (0)

    INDENT ();

    for (struct _event_table *etp = event_table; etp->name; etp++) {
	if (mask & etp->value) {
	    if (len + MASK_NAME_WIDTH > lastcol) {
		puts (buf);
		INDENT ();
	    }
	    strcpy (buf+len, etp->name);
	    len += MASK_NAME_WIDTH;
	    bitsfound++;
	}
    }

    if (bitsfound) puts (buf);

#undef INDENT

    return (bitsfound);
}

static void
print_standard_extension_info(Display *dpy, const char *extname,
			      int majorrev, int minorrev)
{
    int opcode, event, error;

    printf("%s version %d.%d ", extname, majorrev, minorrev);

    XQueryExtension(dpy, extname, &opcode, &event, &error);
    printf ("opcode: %d", opcode);
    if (event)
	printf (", base event: %d", event);
    if (error)
	printf (", base error: %d", error);
    printf("\n");
}

#ifdef MULTIBUFFER
static int
print_multibuf_info(Display *dpy, const char *extname)
{
#define MULTIBUF_FMT "    visual id, max buffers, depth:    0x%lx, %d, %d\n"
    int majorrev, minorrev;

    if (!XmbufGetVersion(dpy, &majorrev, &minorrev))
	return 0;

    print_standard_extension_info(dpy, extname, majorrev, minorrev);

    for (int i = 0; i < ScreenCount (dpy); i++)
    {
        int nmono, nstereo;		/* count */
        XmbufBufferInfo *mono_info = NULL, *stereo_info = NULL; /* arrays */
        const int scr = 0;

	if (!XmbufGetScreenInfo (dpy, RootWindow(dpy, scr), &nmono, &mono_info,
				 &nstereo, &stereo_info)) {
	    fprintf (stderr,
		     "%s:  unable to get multibuffer info for screen %d\n",
		     ProgramName, scr);
	} else {
	    printf ("  screen %d number of mono multibuffer types:    %d\n", i, nmono);
	    for (int j = 0; j < nmono; j++) {
		printf (MULTIBUF_FMT, mono_info[j].visualid,
			mono_info[j].max_buffers, mono_info[j].depth);
	    }
	    printf ("  number of stereo multibuffer types:    %d\n", nstereo);
	    for (int j = 0; j < nstereo; j++) {
		printf (MULTIBUF_FMT, stereo_info[j].visualid,
			stereo_info[j].max_buffers, stereo_info[j].depth);
	    }
	    if (mono_info) XFree ((char *) mono_info);
	    if (stereo_info) XFree ((char *) stereo_info);
	}
    }
    return 1;
} /* end print_multibuf_info */
#endif

static int
print_xtest_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev, foo;

    if (!XTestQueryExtension(dpy, &foo, &foo, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);
    return 1;
}

static int
print_sync_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;
    XSyncSystemCounter *syscounters;
    int ncounters;

    if (!XSyncInitialize(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);

    syscounters = XSyncListSystemCounters(dpy, &ncounters);
    printf("  system counters: %d\n", ncounters);
    for (int i = 0; i < ncounters; i++)
    {
	printf("    %s  id: 0x%08x  resolution_lo: %d  resolution_hi: %d\n",
	       syscounters[i].name, (unsigned int)syscounters[i].counter,
	       XSyncValueLow32(syscounters[i].resolution),
	       XSyncValueHigh32(syscounters[i].resolution));
    }
    XSyncFreeSystemCounterList(syscounters);
    return 1;
}

static int
print_shape_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;

    if (!XShapeQueryVersion(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);
    return 1;
}

#ifdef XFreeXDGA
static int
print_dga_info(Display *dpy, const char *extname)
{
    unsigned int offset;
    int majorrev, minorrev, width, bank, ram, flags;

    if (!XF86DGAQueryVersion(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);

    if (!XF86DGAQueryDirectVideo(dpy, DefaultScreen(dpy), &flags)
	|| ! (flags & XF86DGADirectPresent) )
    {
	printf("  DGA not available on screen %d.\n", DefaultScreen(dpy));
	return 1;
    }

    old_handler = XSetErrorHandler(silent_errors);

    if (!XF86DGAGetVideoLL(dpy, DefaultScreen(dpy), &offset,
			    &width, &bank, &ram))
	return 0;
    printf("  Base address = 0x%X, Width = %d, Bank size = %d,"
	   " RAM size = %dk\n", offset, width, bank, ram);

    XSetErrorHandler(old_handler);

    return 1;
}
#endif

#ifdef XF86VIDMODE
#define V_PHSYNC        0x001
#define V_NHSYNC        0x002
#define V_PVSYNC        0x004
#define V_NVSYNC        0x008
#define V_INTERLACE     0x010
#define V_DBLSCAN       0x020
#define V_CSYNC         0x040
#define V_PCSYNC        0x080
#define V_NCSYNC        0x100

static void
print_XF86VidMode_modeline(
    unsigned int        dotclock,
    unsigned short      hdisplay,
    unsigned short      hsyncstart,
    unsigned short      hsyncend,
    unsigned short      htotal,
    unsigned short      vdisplay,
    unsigned short      vsyncstart,
    unsigned short      vsyncend,
    unsigned short      vtotal,
    unsigned int        flags)
{
    printf("    %6.2f   %4d %4d %4d %4d   %4d %4d %4d %4d ",
	   dotclock/1000.0,
	   hdisplay, hsyncstart, hsyncend, htotal,
	   vdisplay, vsyncstart, vsyncend, vtotal);
    if (flags & V_PHSYNC)    printf(" +hsync");
    if (flags & V_NHSYNC)    printf(" -hsync");
    if (flags & V_PVSYNC)    printf(" +vsync");
    if (flags & V_NVSYNC)    printf(" -vsync");
    if (flags & V_INTERLACE) printf(" interlace");
    if (flags & V_CSYNC)     printf(" composite");
    if (flags & V_PCSYNC)    printf(" +csync");
    if (flags & V_NCSYNC)    printf(" -csync");
    if (flags & V_DBLSCAN)   printf(" doublescan");
    printf("\n");
}

static int
print_XF86VidMode_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;
    XF86VidModeMonitor monitor;

    if (!XF86VidModeQueryVersion(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);

    if (XF86VidModeGetMonitor(dpy, DefaultScreen(dpy), &monitor)) {
	printf("  Monitor Information:\n");
	printf("    Vendor: %s, Model: %s\n",
	       monitor.vendor == NULL ? "" : monitor.vendor,
	       monitor.model == NULL ? "" : monitor.model);
	printf("    Num hsync: %d, Num vsync: %d\n",
	       monitor.nhsync, monitor.nvsync);
	for (int i = 0; i < monitor.nhsync; i++) {
	    printf("    hsync range %d: %6.2f - %6.2f\n", i,
		   monitor.hsync[i].lo, monitor.hsync[i].hi);
	}
	for (int i = 0; i < monitor.nvsync; i++) {
	    printf("    vsync range %d: %6.2f - %6.2f\n", i,
		   monitor.vsync[i].lo, monitor.vsync[i].hi);
	}
	XFree(monitor.vendor);
	XFree(monitor.model);
	XFree(monitor.hsync);
	XFree(monitor.vsync);
    } else {
	printf("  Monitor Information not available\n");
    }

    if ((majorrev > 0) || (majorrev == 0 && minorrev > 5)) {
      int modecount, dotclock;
      XF86VidModeModeLine modeline;
      XF86VidModeModeInfo **modelines;

      if (XF86VidModeGetAllModeLines(dpy, DefaultScreen(dpy), &modecount,
				     &modelines)) {
	  printf("  Available Video Mode Settings:\n");
	  printf("     Clock   Hdsp Hbeg Hend Httl   Vdsp Vbeg Vend Vttl  Flags\n");
	  for (int i = 0; i < modecount; i++) {
	      print_XF86VidMode_modeline
		  (modelines[i]->dotclock, modelines[i]->hdisplay,
		   modelines[i]->hsyncstart, modelines[i]->hsyncend,
		   modelines[i]->htotal, modelines[i]->vdisplay,
		   modelines[i]->vsyncstart, modelines[i]->vsyncend,
		   modelines[i]->vtotal, modelines[i]->flags);
	  }
	  XFree(modelines);
      } else {
	  printf("  Available Video Mode Settings not available\n");
      }

      if (XF86VidModeGetModeLine(dpy, DefaultScreen(dpy),
				 &dotclock, &modeline)) {
	  printf("  Current Video Mode Setting:\n");
	  print_XF86VidMode_modeline(dotclock,
				     modeline.hdisplay, modeline.hsyncstart,
				     modeline.hsyncend, modeline.htotal,
				     modeline.vdisplay, modeline.vsyncstart,
				     modeline.vsyncend, modeline.vtotal,
				     modeline.flags);
      } else {
	  printf("  Current Video Mode Setting not available\n");
      }
    }

    return 1;
}
#endif

#ifdef XF86MISC

static const char *kbdtable[] = {
		     "Unknown", "84-key", "101-key", "Other", "Xqueue" };
static const char *msetable[] = {
		     "None", "Microsoft", "MouseSystems", "MMSeries",
		     "Logitech", "BusMouse", "Mouseman", "PS/2", "MMHitTab",
		     "GlidePoint", "IntelliMouse", "ThinkingMouse",
		     "IMPS/2", "ThinkingMousePS/2", "MouseManPlusPS/2",
		     "GlidePointPS/2", "NetMousePS/2", "NetScrollPS/2",
		     "SysMouse", "Auto" };
static const char *flgtable[] = {
		     "None", "ClearDTR", "ClearRTS", "ClearDTR and ClearRTS" };

static int
print_XF86Misc_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;

    if (!XF86MiscQueryVersion(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);

    old_handler = XSetErrorHandler(silent_errors);

    if ((majorrev > 0) || (majorrev == 0 && minorrev > 0)) {
      XF86MiscKbdSettings kbdinfo;
      XF86MiscMouseSettings mouseinfo;

      if (!XF86MiscGetKbdSettings(dpy, &kbdinfo))
	return 0;
      printf("  Keyboard Settings-    Type: %s, Rate: %d, Delay: %d, ServerNumLock: %s\n",
	kbdtable[kbdinfo.type], kbdinfo.rate, kbdinfo.delay,
	(kbdinfo.servnumlock? "yes": "no"));

      if (!XF86MiscGetMouseSettings(dpy, &mouseinfo))
	return 0;
      printf("  Mouse Settings-       Device: %s, Type: ",
	strlen(mouseinfo.device) == 0 ? "None": mouseinfo.device);
      XFree(mouseinfo.device);
      if (mouseinfo.type == MTYPE_XQUEUE)
	printf("Xqueue\n");
      else if (mouseinfo.type == MTYPE_OSMOUSE)
	printf("OSMouse\n");
      else if (mouseinfo.type <= MTYPE_AUTOMOUSE)
	printf("%s\n", msetable[mouseinfo.type+1]);
      else
	printf("Unknown\n");
      printf("                        BaudRate: %d, SampleRate: %d, Resolution: %d\n",
	mouseinfo.baudrate, mouseinfo.samplerate, mouseinfo.resolution);
      printf("                        Emulate3Buttons: %s, Emulate3Timeout: %d ms\n",
	mouseinfo.emulate3buttons? "yes": "no", mouseinfo.emulate3timeout);
      printf("                        ChordMiddle: %s, Flags: %s\n",
	mouseinfo.chordmiddle? "yes": "no",
	flgtable[(mouseinfo.flags & MF_CLEAR_DTR? 1: 0)
		+(mouseinfo.flags & MF_CLEAR_RTS? 1: 0)] );
      printf("                        Buttons: %d\n", mouseinfo.buttons);
    }

    XSetErrorHandler(old_handler);

    return 1;
}
#endif

#ifdef MITSHM
static int
print_mitshm_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;
    Bool sharedPixmaps;

    if (!XShmQueryVersion(dpy, &majorrev, &minorrev, &sharedPixmaps))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);
    printf("  shared pixmaps: ");
    if (sharedPixmaps)
    {
	int format = XShmPixmapFormat(dpy);
	printf("yes, format: %d\n", format);
    }
    else
    {
	printf("no\n");
    }
    return 1;
}
#endif /* MITSHM */

#ifdef XKB
static int
print_xkb_info(Display *dpy, const char *extname)
{
    int opcode, eventbase, errorbase, majorrev, minorrev;

    if (!XkbQueryExtension(dpy, &opcode, &eventbase, &errorbase,
			   &majorrev, &minorrev)) {
        return 0;
    }
    printf("%s version %d.%d ", extname, majorrev, minorrev);

    printf ("opcode: %d", opcode);
    if (eventbase)
	printf (", base event: %d", eventbase);
    if (errorbase)
	printf (", base error: %d", errorbase);
    printf("\n");

    return 1;
}
#endif

static int
print_dbe_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;
    XdbeScreenVisualInfo *svi;
    int numscreens = 0;

    if (!XdbeQueryExtension(dpy, &majorrev, &minorrev))
	return 0;

    print_standard_extension_info(dpy, extname, majorrev, minorrev);
    svi = XdbeGetVisualInfo(dpy, (Drawable *)NULL, &numscreens);
    for (int iscrn = 0; iscrn < numscreens; iscrn++)
    {
	printf("  Double-buffered visuals on screen %d\n", iscrn);
	for (int ivis = 0; ivis < svi[iscrn].count; ivis++)
	{
	    printf("    visual id 0x%lx  depth %d  perflevel %d\n",
		   svi[iscrn].visinfo[ivis].visual,
		   svi[iscrn].visinfo[ivis].depth,
		   svi[iscrn].visinfo[ivis].perflevel);
	}
    }
    XdbeFreeVisualInfo(svi);
    return 1;
}

static int
print_record_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev;

    if (!XRecordQueryVersion(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);
    return 1;
}

#ifdef XINPUT
static int
print_xinput_info(Display *dpy, const char *extname)
{
  int           loop, num_extensions;
  char          **extensions;
  XExtensionVersion *ext;

  ext = XGetExtensionVersion(dpy, extname);

  if (!ext || (ext == (XExtensionVersion*) NoSuchExtension))
      return 0;

  print_standard_extension_info(dpy, extname, ext->major_version,
				ext->minor_version);
  XFree(ext);

  extensions = XListExtensions(dpy, &num_extensions);
  for (loop = 0; loop < num_extensions &&
         (strcmp(extensions[loop], extname) != 0); loop++);
  XFreeExtensionList(extensions);
  if (loop != num_extensions) {
      int           num_devices;
      XDeviceInfo   *devices;

      printf("  Extended devices :\n");
      devices = XListInputDevices(dpy, &num_devices);
      for(loop=0; loop<num_devices; loop++) {
	  printf("	\"%s\"	[", devices[loop].name ? devices[loop].name : "<noname>");
	  switch(devices[loop].use) {
	  case IsXPointer:
	      printf("XPointer]\n");
	      break;
	  case IsXKeyboard:
	      printf("XKeyboard]\n");
	      break;
	  case IsXExtensionDevice:
	      printf("XExtensionDevice]\n");
	      break;
#ifdef IsXExtensionKeyboard
	  case IsXExtensionKeyboard:
	      printf("XExtensionKeyboard]\n");
	      break;
#endif
#ifdef IsXExtensionPointer
	  case IsXExtensionPointer:
	      printf("XExtensionPointer]\n");
	      break;
#endif
	  default:
	      printf("invalid value]\n");
	      break;
	  }
        }
      XFreeDeviceList(devices);
      return 1;
    }
  else
      return 0;
}
#endif

#ifdef XRENDER
static int
print_xrender_info(Display *dpy, const char *extname)
{
  int		    loop, num_extensions;
  char		    **extensions;
  int		    major, minor;

  if (!XRenderQueryVersion (dpy, &major, &minor))
    return 0;

  print_standard_extension_info(dpy, extname, major, minor);

  extensions = XListExtensions(dpy, &num_extensions);
  for (loop = 0; loop < num_extensions &&
         (strcmp(extensions[loop], extname) != 0); loop++);
  XFreeExtensionList(extensions);
  if (loop != num_extensions) {
    XRenderPictFormat *pictform;

    printf ("  Render formats :\n");
    for (int count = 0; (pictform = XRenderFindFormat (dpy, 0, NULL, count));
         count++)
    {
      printf  ("  pict format:\n");
      printf  ("\tformat id:    0x%lx\n", pictform->id);
      printf  ("\ttype:         %s\n",
	     pictform->type == PictTypeIndexed ? "Indexed" : "Direct");
      printf  ("\tdepth:        %d\n", pictform->depth);
      if (pictform->type == PictTypeDirect) {
	printf("\talpha:        %2d mask 0x%x\n", pictform->direct.alpha, pictform->direct.alphaMask);
	printf("\tred:          %2d mask 0x%x\n", pictform->direct.red, pictform->direct.redMask);
	printf("\tgreen:        %2d mask 0x%x\n", pictform->direct.green, pictform->direct.greenMask);
	printf("\tblue:         %2d mask 0x%x\n", pictform->direct.blue, pictform->direct.blueMask);
      }
      else
	printf("\tcolormap      0x%lx\n", pictform->colormap);
    }
    printf ("  Screen formats :\n");
    for (int i = 0; i < ScreenCount (dpy); i++) {
      int	     nvi;		/* number of elements returned */
      XVisualInfo    viproto;		/* fill in for getting info */
      XVisualInfo    *vip;		/* returned info */
      int 	     ndepths = 0, *depths = NULL;
#if RENDER_MAJOR > 0 || RENDER_MINOR >= 6
      XFilters	    *filters;
#endif

      nvi = 0;
      viproto.screen = i;
      vip = XGetVisualInfo (dpy, VisualScreenMask, &viproto, &nvi);
      printf ("    Screen %d", i);
#if RENDER_MAJOR > 0 || RENDER_MINOR >= 6
      switch (XRenderQuerySubpixelOrder (dpy, i)) {
      case SubPixelUnknown: printf (" (sub-pixel order Unknown)"); break;
      case SubPixelHorizontalRGB: printf (" (sub-pixel order Horizontal RGB)"); break;
      case SubPixelHorizontalBGR: printf (" (sub-pixel order Horizontal BGR)"); break;
      case SubPixelVerticalRGB: printf (" (sub-pixel order Vertical RGB)"); break;
      case SubPixelVerticalBGR: printf (" (sub-pixel order Vertical BGR)"); break;
      case SubPixelNone: printf (" (sub-pixel order None)"); break;
      }
      printf ("\n");
      filters = XRenderQueryFilters (dpy, RootWindow (dpy, i));
      if (filters)
      {
	printf ("      filters: ");
	for (int f = 0; f < filters->nfilter; f++)
	{
	  printf ("%s", filters->filter[f]);
	  if (f < filters->nalias && filters->alias[f] != FilterAliasNone)
	    printf ("(%s)", filters->filter[filters->alias[f]]);
	  if (f < filters->nfilter - 1)
	    printf (", ");
	}
	XFree (filters);
      }
#endif
      printf ("\n");
      for (int j = 0; j < nvi; j++)
      {
	printf  ("      visual format:\n");
	printf  ("        visual id:      0x%lx\n", vip[j].visualid);
	pictform = XRenderFindVisualFormat (dpy, vip[j].visual);
	if (pictform)
	  printf("        pict format id: 0x%lx\n", pictform->id);
	else
	  printf("        pict format id: None\n");
      }
      if (vip) XFree ((char *) vip);
      depths = XListDepths (dpy, i, &ndepths);
      if (!depths) ndepths = 0;
      for (int j = 0; j < ndepths; j++)
      {
	XRenderPictFormat templ;

	templ.depth = depths[j];
	printf  ("     depth formats:\n");
	printf  ("       depth           %d\n", depths[j]);
	for (int count = 0;
             (pictform = XRenderFindFormat (dpy, PictFormatDepth, &templ, count));
             count++) {
	  printf("       pict format id: 0x%lx\n", pictform->id);
        }
      }
      if (depths) XFree (depths);
    }
    return 1;
  }
  else
    return 0;
}
#endif /* XRENDER */

#ifdef COMPOSITE
static int
print_composite_info(Display *dpy, const char *extname)
{
    int majorrev, minorrev, foo;

    if (!XCompositeQueryExtension(dpy, &foo, &foo))
	return 0;
    if (!XCompositeQueryVersion(dpy, &majorrev, &minorrev))
	return 0;
    print_standard_extension_info(dpy, extname, majorrev, minorrev);
    return 1;
}
#endif

#ifdef PANORAMIX

static int
print_xinerama_info(Display *dpy, const char *extname)
{
  int              majorrev, minorrev;

  if (!XineramaQueryVersion (dpy, &majorrev, &minorrev))
    return 0;

  print_standard_extension_info(dpy, extname, majorrev, minorrev);

  if (!XineramaIsActive(dpy)) {
    printf("  Xinerama is inactive.\n");
  } else {
    int count = 0;
    XineramaScreenInfo *xineramaScreens = XineramaQueryScreens(dpy, &count);

    for (int i = 0; i < count; i++) {
      XineramaScreenInfo *xs = &xineramaScreens[i];
      printf("  head #%d: %dx%d @ %d,%d\n", xs->screen_number,
             xs->width, xs->height, xs->x_org, xs->y_org);
    }

    XFree(xineramaScreens);
  }

  return 1;
}

#endif /* PANORAMIX */

#ifdef DMX
static const char *core(DMXInputAttributes *iinfo)
{
    if (iinfo->isCore)         return "core";
    else if (iinfo->sendsCore) return "extension (sends core)";
    else                       return "extension";
}

static int print_dmx_info(Display *dpy, const char *extname)
{
    int                  event_base, error_base;
    int                  major_version, minor_version, patch_version;
    int                  count;

    if (!DMXQueryExtension(dpy, &event_base, &error_base)
        || !DMXQueryVersion(dpy, &major_version, &minor_version,
                            &patch_version)) return 0;
    print_standard_extension_info(dpy, extname, major_version, minor_version);
    printf("  Version stamp: %d\n", patch_version);

    if (!DMXGetScreenCount(dpy, &count)) return 1;
    printf("  Screen count: %d\n", count);
    for (int i = 0; i < count; i++) {
        DMXScreenAttributes  sinfo;

        if (DMXGetScreenAttributes(dpy, i, &sinfo)) {
            printf("    %2d %s %ux%u+%d+%d %d @%dx%d\n",
                   i, sinfo.displayName,
                   sinfo.screenWindowWidth, sinfo.screenWindowHeight,
                   sinfo.screenWindowXoffset, sinfo.screenWindowYoffset,
                   sinfo.logicalScreen,
                   sinfo.rootWindowXorigin, sinfo.rootWindowYorigin);
        }
    }

    if (major_version != 1
        || minor_version < 1
        || !DMXGetInputCount(dpy, &count))
        return 1;

    printf("  Input count = %d\n", count);
    for (int i = 0; i < count; i++) {
        DMXInputAttributes   iinfo;
#ifdef XINPUT
        Display *backend;
        char    *backendname = NULL;
#endif
        if (DMXGetInputAttributes(dpy, i, &iinfo)) {
            switch (iinfo.inputType) {
            case DMXLocalInputType:
                printf("    %2d local %s", i, core(&iinfo));
                break;
            case DMXConsoleInputType:
                printf("    %2d console %s %s", i, core(&iinfo),
                       iinfo.name);
                break;
            case DMXBackendInputType:
#ifdef XINPUT
                if (iinfo.physicalId >= 0) {
                    if ((backend = XOpenDisplay(iinfo.name))) {
                        XExtensionVersion *ext
                            = XGetExtensionVersion(backend, INAME);
                        if (ext
                            && ext != (XExtensionVersion *)NoSuchExtension) {

                            int         dcount;
                            XDeviceInfo *devInfo = XListInputDevices(backend,
                                                                     &dcount);
                            if (devInfo) {
                                for (int d = 0; d < dcount; d++) {
                                    if ((unsigned)iinfo.physicalId
                                        == devInfo[d].id
                                        && devInfo[d].name) {
                                        backendname = strdup(devInfo[d].name);
                                        break;
                                    }
                                }
                                XFreeDeviceList(devInfo);
                            }
                        }
                        XCloseDisplay(backend);
                    }
                }
#endif
                printf("    %2d backend %s o%d/%s",i, core(&iinfo),
                       iinfo.physicalScreen, iinfo.name);
                if (iinfo.physicalId >= 0) printf("/id%d", iinfo.physicalId);
#ifdef XINPUT
                if (backendname) {
                    printf("=%s", backendname);
                    free(backendname);
                }
#endif
                break;
            }
        }
        printf("\n");
    }
    return 1;
}

#endif /* DMX */

/* utilities to manage the list of recognized extensions */


typedef int (*ExtensionPrintFunc)(
    Display *, const char *
);

typedef struct {
    const char *extname;
    ExtensionPrintFunc printfunc;
    Bool printit;
} ExtensionPrintInfo;

static ExtensionPrintInfo known_extensions[] =
{
#ifdef MITSHM
    {"MIT-SHM",	print_mitshm_info, False},
#endif /* MITSHM */
#ifdef XKB
    {XkbName, print_xkb_info, False},
#endif /* XKB */
#ifdef MULTIBUFFER
    {MULTIBUFFER_PROTOCOL_NAME,	print_multibuf_info, False},
#endif
    {"SHAPE", print_shape_info, False},
    {SYNC_NAME, print_sync_info, False},
#ifdef XFreeXDGA
    {XF86DGANAME, print_dga_info, False},
#endif /* XFreeXDGA */
#ifdef XF86VIDMODE
    {XF86VIDMODENAME, print_XF86VidMode_info, False},
#endif /* XF86VIDMODE */
#ifdef XF86MISC
    {XF86MISCNAME, print_XF86Misc_info, False},
#endif /* XF86MISC */
    {XTestExtensionName, print_xtest_info, False},
    {"DOUBLE-BUFFER", print_dbe_info, False},
    {"RECORD", print_record_info, False},
#ifdef XINPUT
    {INAME, print_xinput_info, False},
#endif
#ifdef XRENDER
    {RENDER_NAME, print_xrender_info, False},
#endif
#ifdef COMPOSITE
    {COMPOSITE_NAME, print_composite_info, False},
#endif
#ifdef PANORAMIX
    {"XINERAMA", print_xinerama_info, False},
#endif
#ifdef DMX
    {"DMX", print_dmx_info, False},
#endif
    /* add new extensions here */
};

static const int num_known_extensions = sizeof known_extensions / sizeof known_extensions[0];

static void
print_known_extensions(FILE *f)
{
    int i, col;
    for (i = 0, col = 6; i < num_known_extensions; i++)
    {
	int extlen = (int) strlen(known_extensions[i].extname) + 1;

	if ((col + extlen) > 79)
	{
		col = 6;
		fprintf(f, "\n     ");
	}
	fprintf(f, "%s ", known_extensions[i].extname);
	col += extlen;
    }
}

static void
mark_extension_for_printing(const char *extname)
{
    if (strcmp(extname, "all") == 0)
    {
	for (int i = 0; i < num_known_extensions; i++)
	    known_extensions[i].printit = True;
    }
    else
    {
	for (int i = 0; i < num_known_extensions; i++)
	{
	    if (strcmp(extname, known_extensions[i].extname) == 0)
	    {
		known_extensions[i].printit = True;
		return;
	    }
	}
	printf("%s extension not supported by %s\n", extname, ProgramName);
    }
}

static void
print_marked_extensions(Display *dpy)
{
    for (int i = 0; i < num_known_extensions; i++)
    {
	if (known_extensions[i].printit)
	{
	    printf("\n");
	    if (! (*known_extensions[i].printfunc)(dpy,
					known_extensions[i].extname))
	    {
		printf("%s extension not supported by server\n",
		       known_extensions[i].extname);
	    }
	}
    }
}

static void _X_NORETURN
usage(void)
{
    fprintf (stderr, "usage:  %s [options]\n%s", ProgramName,
             "-display displayname\tserver to query\n"
             "-version\t\tprint program version and exit\n"
             "-queryExtensions\tprint info returned by XQueryExtension\n"
             "-ext all\t\tprint detailed info for all supported extensions\n"
             "-ext extension-name\tprint detailed info for extension-name if one of:\n     ");
    print_known_extensions(stderr);
    fprintf (stderr, "\n");
    exit (1);
}

int
main(int argc, char *argv[])
{
    Display *dpy;			/* X connection */
    char *displayname = NULL;		/* server to contact */

    ProgramName = argv[0];

    for (int i = 1; i < argc; i++) {
	char *arg = argv[i];
	size_t len = strlen(arg);

	if (!strncmp("-display", arg, len)) {
	    if (++i >= argc) {
		fprintf (stderr, "%s: -display requires an argument\n",
			 ProgramName);
		usage ();
	    }
	    displayname = argv[i];
	} else if (!strncmp("-queryExtensions", arg, len)) {
	    queryExtensions = True;
	} else if (!strncmp("-ext", arg, len)) {
	    if (++i >= argc) {
		fprintf (stderr, "%s: -ext requires an argument\n",
			 ProgramName);
		usage ();
	    }
	    mark_extension_for_printing(argv[i]);
        } else if (!strncmp("-version", arg, len)) {
            printf("%s\n", PACKAGE_STRING);
            exit (0);
	} else {
	    fprintf (stderr, "%s: unrecognized argument '%s'\n",
		     ProgramName, arg);
	    usage ();
	}
    }

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\".\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }

    print_display_info (dpy);
    for (int i = 0; i < ScreenCount (dpy); i++) {
	print_screen_info (dpy, i);
    }

    print_marked_extensions(dpy);

    XCloseDisplay (dpy);
    exit (0);
}
