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
 * $XConsortium: XlibOpaque.c,v 1.12 94/04/17 21:01:36 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/*
 * You can portably do client-native only testing with XlibOpaque.c
 * which uses XOpenDisplay to make the connection and then ConnectionNumber()
 * to get the fd. This route is also appropriate if your Xlib has a different
 * internal interface to the MIT release or else you don't have source.
 * More, and portable, functionality is available in XlibWithXTest.c if your
 * Xlib has the post R5 release patches to move auth/conn handling from
 * XOpenDis.c into XConDis.c and hence returned by _XConnectDisplay().
 * All byte-sexes can be tested with XlibNoXTest.c but that file is only
 * really for BSD type environments and may represent a portability constraint.
 * See the documentation of the build parameter XP_OPEN_DIS for more details.
 */
#include <stdio.h>
#include <string.h>
#include "Xlib.h"
#include "Xutil.h"
#include "XstlibInt.h"
#include "XstosInt.h"
#include "Xstos.h"
#include "DataMove.h"

/* 
 * Attempts to connect to server, given display name. Returns file descriptor
 * (network socket) or -1 if connection fails. The expanded display name
 * of the form hostname:number.screen ("::" if DECnet) is returned in a result
 * parameter. The screen number to use is also returned.
 */
int XstConnectDisplay (display_name, expanded_name, screen_num,
		       auth_proto, auth_length, auth_string, auth_strlen,
		       xlib_dpy)
    char *display_name;
    char *expanded_name;	/* return */
    int *screen_num;		/* return */
    char **auth_proto;		/* return */
    int *auth_length;		/* return */
    char **auth_string;		/* return */
    int *auth_strlen;		/* return */
    Display **xlib_dpy;		/* return */
{
	*expanded_name  = '\0';
	*auth_proto = "";
	*auth_length = 0;
	*auth_string = "";
	*auth_strlen = 0;
	*xlib_dpy = XOpenDisplay(display_name);
	if (*xlib_dpy == (Display *)NULL)
		return -1;
	(void)strcpy(expanded_name, display_name);
	*screen_num = DefaultScreen(*xlib_dpy);
	return ConnectionNumber(*xlib_dpy);
}

/* 
 * Disconnect from server.
 */

int XstDisconnectDisplay (server)

    int server;

{
    (void) close(server);
}

#undef NULL
#define NULL ((char *) 0)

_XstWaitForReadable(dpy)
  XstDisplay *dpy;
{
    int r_mask[MSKCNT];
    int result;
	
    CLEARBITS(r_mask);
    do {
	BITSET(r_mask, dpy->fd);
	result = select(dpy->fd + 1, r_mask, (int *) 0, (int *) 0, (struct timeval *)NULL);
	if (result == -1 && errno != EINTR) {
	    XstIOError(dpy,"_XstWaitForReadable",1);
	}
    } while (result <= 0);
}

static unsigned int padlength[4] = {0, 3, 2, 1};

XstSendClientPrefix (dpy, client, auth_proto, auth_string, needswap)
     XstDisplay *dpy;
     xConnClientPrefix *client;
     int needswap;
{
	BPRINTF1 ("OpenDisplay already done by Xlib XOpenDisplay()\n");
	return;
}

static int num_formats(xdpy)
Display *xdpy;
{
	int nf = 0;
	XPixmapFormatValues *xpfvp = XListPixmapFormats(xdpy, &nf);

	if (xpfvp != (XPixmapFormatValues *)NULL)
		XFree((char*)xpfvp);
	return nf;
}

static int calc_length(xdpy)
Display *xdpy;
{
	int total, nb, s;
	int nf = num_formats(xdpy);

	total = 8 + 2*nf;
	nb =  padup((int)strlen(ServerVendor(xdpy)));
	for(s=0; s < ScreenCount(xdpy); s++) {
		int nd = 0, d;
		int *depths;

		depths = XListDepths(xdpy, s, &nd);
		nb += sizeof(xWindowRoot);
		for(d=0; d < nd; d++) {
			int nv = 0;
			XVisualInfo template,
					*xvip;

			template.screen = s;
			template.depth = depths[d];
			xvip = XGetVisualInfo(xdpy,
				VisualScreenMask|VisualDepthMask,
				&template, &nv);
			nb += sizeof(xDepth) + nv * sizeof(xVisualType);
			if (xvip != (XVisualInfo *)NULL)
				XFree((char*)xvip);
		}
		if (depths)
			XFree((char*)depths);
	}
	total += nb/4;
	return total;
}

GetConnSetupPrefix (client, prefixp, needswap)
int client;
xConnSetupPrefix * prefixp;
int     needswap;
{
    XstDisplay * dpy;
    Display *xdpy;

    dpy = Get_Display(client);
    xdpy = dpy->xlib_dpy;

    BPRINTF1 ("Connection setup prefix:\n");
    prefixp -> success = True;
    BPRINTF2 ("\tsuccess = %s\n", boolname(prefixp->success));
    prefixp -> lengthReason = 0;
    BPRINTF2 ("\tlengthReason = %d\n", prefixp->lengthReason);
    prefixp -> majorVersion = ProtocolVersion(xdpy);
    BPRINTF2 ("\tmajorVersion = %d\n", prefixp->majorVersion);
    prefixp -> minorVersion = ProtocolRevision(xdpy);
    BPRINTF2 ("\tminorVersion = %d\n", prefixp->minorVersion);
    prefixp -> length = calc_length(xdpy);
    BPRINTF2 ("\tlength = %d\n", prefixp->length);
}

/* 
 *	GetConnSetupData - steals
 *		the rest of the connection setup data
 *		from out of the Xlib Display and makes it look
 *		as if it all came from the conection.
 */

static unsigned long infer_mask(xdpy)
Display *xdpy;
{
	int i;
	unsigned long mask = 0;

	for (i=0; i<256; i++) /* we don't need millions, per test */
		mask |= XAllocID(xdpy);

	return mask;
}

GetConnSetupData (client, setupdp, len, needswap)
int client;
xConnSetup * setupdp;
int     len;
int     needswap;
{
    XstDisplay * dpy;
    char   *sptr;		/* pointer into setup data area */
    int     pad;
    int i;
    int d;
    int s;
    int v;
    int f;
    int nvisuals;
    Display *xdpy;
    XPixmapFormatValues *xpfvp;

    dpy = Get_Display(client);
    xdpy = dpy->xlib_dpy;

    {
	int mink, maxk, junk;

	setupdp -> release = VendorRelease(xdpy);
	setupdp -> ridBase = XAllocID(xdpy);
	setupdp -> ridMask = infer_mask(xdpy);
	setupdp -> motionBufferSize = 0;
	setupdp -> nbytesVendor = strlen(ServerVendor(xdpy));
	setupdp -> maxRequestSize = XMaxRequestSize(xdpy);
	setupdp -> numRoots = ScreenCount(xdpy);
	setupdp -> numFormats = num_formats(xdpy);
	setupdp -> imageByteOrder = ImageByteOrder(xdpy);
	setupdp -> bitmapBitOrder = BitmapBitOrder(xdpy);
	setupdp -> bitmapScanlineUnit = BitmapUnit(xdpy);
	setupdp -> bitmapScanlinePad = BitmapPad(xdpy);
	XDisplayKeycodes(xdpy, &mink, &maxk);
	setupdp -> minKeyCode = mink;
	setupdp -> maxKeyCode = maxk;
	setupdp -> pad2 = 0;

	sptr = (char *) (setupdp + 1);

/*	get the vendor string */
	bcopy (ServerVendor(xdpy), sptr, setupdp -> nbytesVendor);
	pad = (setupdp -> nbytesVendor + 3) & ~3;
	sptr += pad;

 /* Z axis screen format info */
 /* NOTE - this counts on only 1 byte quantities in the format!! */
	xpfvp = XListPixmapFormats(xdpy, &junk);
	for (f = 0; f < (int)setupdp->numFormats; f++) {
		xPixmapFormat *pp = (xPixmapFormat *)sptr;

		pp->depth = xpfvp[f].depth;
		pp->bitsPerPixel = xpfvp[f].bits_per_pixel;
		pp->scanLinePad = xpfvp[f].scanline_pad;
		sptr += sizeof (xPixmapFormat);
	}
	if (xpfvp != (XPixmapFormatValues *)NULL)
		XFree((char*)xpfvp);
 /* Screen structures */
	for (s = 0; s < (int)setupdp->numRoots; s++) {
		xWindowRoot *xwp = (xWindowRoot *)sptr;
		Screen *scr = ScreenOfDisplay(xdpy, s);
		int ndepths;
		int *depths;

		xwp->windowId = RootWindow(xdpy, s); sptr += 4;
		xwp->defaultColormap = DefaultColormap(xdpy, s); sptr += 4;
		xwp->whitePixel = WhitePixel(xdpy, s); sptr += 4;
		xwp->blackPixel = BlackPixel(xdpy, s); sptr += 4;
		xwp->currentInputMask = 0L; sptr += 4;
		xwp->pixWidth = DisplayWidth(xdpy, s); sptr += 2;
		xwp->pixHeight = DisplayHeight(xdpy, s); sptr += 2;
		xwp->mmWidth = DisplayWidthMM(xdpy, s); sptr += 2;
		xwp->mmHeight = DisplayHeightMM(xdpy, s); sptr += 2;
		xwp->minInstalledMaps = MinCmapsOfScreen(scr); sptr += 2;
		xwp->maxInstalledMaps = MaxCmapsOfScreen(scr); sptr += 2;
		xwp->rootVisualID = XVisualIDFromVisual(DefaultVisualOfScreen(scr)); sptr += 4;
		xwp->backingStore = DoesBackingStore(scr); sptr++;
		xwp->saveUnders = DoesSaveUnders(scr); sptr++;
		xwp->rootDepth = PlanesOfScreen(scr); sptr++;
		depths = XListDepths(xdpy, s, &ndepths);
		xwp->nDepths = ndepths; sptr++;
	for (d = 0; d < ndepths; d++) {
		int nvisuals;
		XVisualInfo template,
			   *xvip;
		xDepth *xdp = (xDepth *)sptr;

		template.screen = s;
		template.depth = depths[d];
		xvip = XGetVisualInfo(xdpy, VisualScreenMask|VisualDepthMask,
			&template, &nvisuals);

		xdp->depth = template.depth;
		xdp->nVisuals = nvisuals;
		sptr += sizeof(xDepth);
	    for (v = 0; v < nvisuals; v++) {
		xVisualType *xvp = (xVisualType *)sptr;

		xvp->visualID = xvip[v].visualid;
		xvp->class = xvip[v].class;
		xvp->bitsPerRGB = xvip[v].bits_per_rgb;
		xvp->colormapEntries = xvip[v].colormap_size;
		xvp->redMask = xvip[v].red_mask;
		xvp->greenMask = xvip[v].green_mask;
		xvp->blueMask = xvip[v].blue_mask;

		sptr += sizeof(xVisualType);
	    }
		if (xvip != (XVisualInfo *)NULL)
			XFree((char*)xvip);
	}
	if (depths)
		XFree((char*)depths);
	}
    }
}
