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
 * $XConsortium: XstDisplay.h,v 1.4 94/04/17 21:00:02 rws Exp $
 */

#define XstConnectionNumber(dpy) 	((dpy)->fd)
#define XstRootWindow(dpy, scr) 	(((dpy)->screens[(scr)]).root)
#define XstDefaultScreen(dpy) 	((dpy)->default_screen)
#define XstDefaultRootWindow(dpy) 	(((dpy)->screens[(dpy)->default_screen]).root)
#define XstDefaultVisual(dpy, scr) (((dpy)->screens[(scr)]).root_visual)
#define XstDefaultGC(dpy, scr) 	(((dpy)->screens[(scr)]).default_gc)
#define XstBlackPixel(dpy, scr) 	(((dpy)->screens[(scr)]).black_pixel)
#define XstWhitePixel(dpy, scr) 	(((dpy)->screens[(scr)]).white_pixel)
#define XstAllPlanes 		(~0)
#define XstQLength(dpy) 		((dpy)->qlen)
#define XstDisplayWidth(dpy, scr) 	(((dpy)->screens[(scr)]).width)
#define XstDisplayHeight(dpy, scr) (((dpy)->screens[(scr)]).height)
#define XstDisplayWidthMM(dpy, scr)(((dpy)->screens[(scr)]).mwidth)
#define XstDisplayHeightMM(dpy, scr)(((dpy)->screens[(scr)]).mheight)
#define XstDisplayPlanes(dpy, scr) (((dpy)->screens[(scr)]).root_depth)
#define XstDisplayCells(dpy, scr) 	(XstDefaultVisual((dpy), (scr))->map_entries)
#define XstScreenCount(dpy) 	((dpy)->nscreens)
#define XstServerVendor(dpy) 	((dpy)->vendor)
#define XstProtocolVersion(dpy) 	((dpy)->proto_major_version)
#define XstProtocolRevision(dpy) 	((dpy)->proto_minor_version)
#define XstVendorRelease(dpy) 	((dpy)->release)
#define XstDisplayString(dpy) 	((dpy)->display_name)
#define XstDefaultDepth(dpy, scr) 	(((dpy)->screens[(scr)]).root_depth)
#define XstDefaultColormap(dpy, scr)(((dpy)->screens[(scr)]).cmap)
#define XstBitmapUnit(dpy) 	((dpy)->bitmap_unit)
#define XstBitmapBitOrder(dpy) 	((dpy)->bitmap_bit_order)
#define XstBitmapPad(dpy) 		((dpy)->bitmap_pad)
#define XstImageByteOrder(dpy) 	((dpy)->byte_order)
#define XstNextRequest(dpy)	((dpy)->request + 1)
#define XstLastKnownRequestProcessed(dpy)	((dpy)->last_request_read)

/* macros for screen oriented applications (toolkit) */
#define XstScreenOfDisplay(dpy, scr)(&((dpy)->screens[(scr)]))
#define XstDefaultScreenOfDisplay(dpy) (&((dpy)->screens[(dpy)->default_screen]))
#define XstDisplayOfScreen(s)	((s)->display)
#define XstRootWindowOfScreen(s)	((s)->root)
#define XstBlackPixelOfScreen(s)	((s)->black_pixel)
#define XstWhitePixelOfScreen(s)	((s)->white_pixel)
#define XstDefaultColormapOfScreen(s)((s)->cmap)
#define XstDefaultDepthOfScreen(s)	((s)->root_depth)
#define XstDefaultGCOfScreen(s)	((s)->default_gc)
#define XstDefaultVisualOfScreen(s)((s)->root_visual)
#define XstWidthOfScreen(s)	((s)->width)
#define XstHeightOfScreen(s)	((s)->height)
#define XstWidthMMOfScreen(s)	((s)->mwidth)
#define XstHeightMMOfScreen(s)	((s)->mheight)
#define XstPlanesOfScreen(s)	((s)->root_depth)
#define XstCellsOfScreen(s)	(XstDefaultVisualOfScreen((s))->map_entries)
#define XstMinCmapsOfScreen(s)	((s)->min_maps)
#define XstMaxCmapsOfScreen(s)	((s)->max_maps)
#define XstDoesSaveUnders(s)	((s)->save_unders)
#define XstDoesBackingStore(s)	((s)->backing_store)
#define XstEventMaskOfScreen(s)	((s)->root_input_mask)


/*
 * Visual structure; contains information about colormapping possible.
 */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
#if defined(__cplusplus) || defined(c_plusplus)
	int c_class;		/* C++ class of screen (monochrome, etc.) */
#else
	int class;		/* class of screen (monochrome, etc.) */
#endif
	unsigned long red_mask, green_mask, blue_mask;	/* mask values */
	int bits_per_rgb;	/* log base 2 of distinct color values */
	int map_entries;	/* color map entries */
} XstVisual;

/*
 * Depth structure; contains information for each possible depth.
 */	
typedef struct {
	int depth;		/* this depth (Z) of the depth */
	int nvisuals;		/* number of Visual types at this depth */
	XstVisual *visuals;	/* list of visuals possible at this depth */
} XstDepth;

/*
 * Information about the screen.
 */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XstDisplay *display;/* back pointer to display structure */
	Window root;		/* Root window id. */
	int width, height;	/* width and height of screen */
	int mwidth, mheight;	/* width and height of  in millimeters */
	int ndepths;		/* number of depths possible */
	XstDepth *depths;		/* list of allowable depths on the screen */
	int root_depth;		/* bits per pixel */
	XstVisual *root_visual;	/* root visual */
#ifdef NotRequired
	GC default_gc;		/* GC for the root root visual */
#endif /* NotRequired */
	Colormap cmap;		/* default color map */
	unsigned long white_pixel;
	unsigned long black_pixel;	/* White and Black pixel values */
	int max_maps, min_maps;	/* max and min color maps */
	int backing_store;	/* Never, WhenMapped, Always */
	Bool save_unders;	
	long root_input_mask;	/* initial root input mask */
} XstScreen;

/*
 * Format structure; describes ZFormat data the screen will understand.
 */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	int depth;		/* depth of this image format */
	int bits_per_pixel;	/* bits/pixel at this depth */
	int scanline_pad;	/* scanline must padded to this multiple */
} XstScreenFormat;

/*
 * Display datatype maintaining display specific data.
 */
typedef struct _XstDisplay {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XstDisplay *next; /* next open Display on list */
	int fd;			/* Network socket. */
	int lock;		/* is someone in critical section? */
	int proto_major_version;/* maj. version of server's X protocol */
	int proto_minor_version;/* minor version of servers X protocol */
	char *vendor;		/* vendor of the server hardware */
        long resource_base;	/* resource ID base */
	long resource_mask;	/* resource ID mask bits */
	long resource_id;	/* allocator current ID */
	int resource_shift;	/* allocator shift to correct bits */
	XID (*resource_alloc)(); /* allocator function */
	int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
	int bitmap_unit;	/* padding and data requirements */
	int bitmap_pad;		/* padding requirements on bitmaps */
	int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
	int nformats;		/* number of pixmap formats in list */
	XstScreenFormat *pixmap_format;	/* pixmap format list */
	int vnumber;		/* Xlib's X protocol version number. */
	int release;		/* release of the server */
#ifdef NotRequired
	struct _XSQEvent *head, *tail;	/* Input event queue. */
#endif /* NotRequired */
	int qlen;		/* Length of input event queue */
	unsigned long last_request_read; /* seq number of last event read */
	unsigned long request;	/* sequence number of last request. */
	char *last_req;		/* beginning of last request, or dummy */
	char *buffer;		/* Output buffer starting address. */
	char *bufptr;		/* Output buffer index pointer. */
	char *bufmax;		/* Output buffer maximum+1 address. */
	unsigned max_request_size; /* maximum number 32 bit words in request*/
#ifdef NotRequired
	struct _XrmHashBucketRec *db;
#endif /* NotRequired */
	int (*synchandler)();	/* Synchronization handler */
	char *display_name;	/* "host:display" string used on this connect*/
	int default_screen;	/* default screen for operations */
	int nscreens;		/* number of screens on this server*/
	XstScreen *screens;	/* pointer to list of screens */
	unsigned long motion_buffer;	/* size of motion buffer */
	Window current;		/* for use internally for Keymap notify */
	int min_keycode;	/* minimum defined keycode */
	int max_keycode;	/* maximum defined keycode */
	KeySym *keysyms;	/* This server's keysyms */
	XModifierKeymap *modifiermap;	/* This server's modifier keymap */
	int keysyms_per_keycode;/* number of rows */
	char *xdefaults;	/* contents of defaults from server */
	char *scratch_buffer;	/* place to hang scratch buffer */
	unsigned long scratch_length;	/* length of scratch buffer */
#ifdef NotRequired
	int ext_number;		/* extension number on this display */
	_XExtension *ext_procs;	/* extensions initialized on this display */
#endif /* NotRequired */
	/*
	 * the following can be fixed size, as the protocol defines how
	 * much address space is available. 
	 * While this could be done using the extension vector, there
	 * may be MANY events processed, so a search through the extension
	 * list to find the right procedure for each event might be
	 * expensive if many extensions are being used.
	 */
	Bool (*event_vec[128])();  /* vector for wire to event */
	Status (*wire_vec[128])(); /* vector for event to wire */
#ifdef NotRequired
	KeySym lock_meaning;	   /* for XLookupString */
	struct XKeytrans *key_bindings; /* for XLookupString */
#endif /* NotRequired */
	Font cursor_font;	   /* for XCreateFontCursor */
	/*
	 * ICCCM information, version 1
	 */
	struct _DisplayAtoms *atoms;
	struct {		   /* for XReconfigureWMWindow */
	    long sequence_number;
	    int (*old_handler)();
	    Bool succeeded;
	} reconfigure_wm_window;
	/*
	 * additional connection info
	 */
	unsigned long flags;	   /* internal connection flags */
	unsigned int mode_switch;  /* keyboard group modifiers */
	/*
	 * For X Protocol Testing use only. This is where we store
	 * the Xlib Display pointer, for subsequent closing, if we
	 * had to use XOpenDisplay to get at the fd. If not it's NULL.
	 */
	Display *xlib_dpy;
} XstDisplay;
