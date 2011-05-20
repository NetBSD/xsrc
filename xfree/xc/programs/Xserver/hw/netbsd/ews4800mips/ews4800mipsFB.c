/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "ews4800mips.h"

#define	FORCE_SEPARATE_PRIVATE
#include "cfb.h"
#include "mi.h"
#include <stdio.h>

extern BSFuncRec cfbBSFuncRec;
Bool ews4800mips8ScreenInit(ScreenPtr, pointer,	int, int, int, int, int);

Bool
ews4800mipsFBInit(
	int screen,		/* what screen am I going to be */
	ScreenPtr pScreen,	/* The Screen to initialize */
	int argc,		/* The number of the Server's arguments. */
	char **argv		/* The arguments themselves. Don't change! */
    )
{
	int res;
	ews4800mipsFbPtr pFb = ews4800mipsGetScreenFb(pScreen);
	unsigned char *fb = pFb->fb;
	int rowsize;
	struct wsdisplay_fbinfo *fbconf = &pFb->info;

	pFb->EnterLeave = (void (*)())NoopDDA;

	if (!ews4800mipsAllocateScreenPrivate(pScreen))
		return FALSE;

	if (!fb) {
		if (ioctl(pFb->fd, WSDISPLAYIO_GINFO, fbconf) < 0) {
			ews4800mipsError("ioctl(WSDISPLAYIO_GINFO)");
			return FALSE;
		}

		if (ews4800mipsSetDisplayMode(pFb->fd, WSDISPLAYIO_MODE_MAPPED,
		    NULL) < 0) {
			ews4800mipsError("ioctl(WSDISPLAYIO_MODE_MAPPED)");
			return FALSE;
		}

		fb = ews4800mipsMemoryMap((size_t)2048 * /*XXX -uch*/
		    fbconf->height * fbconf->depth / 8, 0, pFb->fd);
		if (fb == NULL) {
			ews4800mipsError("can't map frame buffer.");
			return FALSE;
		}

		pFb->fb = fb;
	}

	switch (fbconf->depth) {
	case 8:
		res = ews4800mips8ScreenInit(pScreen, fb,
		    fbconf->width,
		    fbconf->height,
		    monitorResolution, monitorResolution,
		    2048/*XXX -uch*/);
		break;
	default:
		ews4800mipsErrorF(("%dbpp frame buffer is not supported\n",
		    fbconf->depth));
		return FALSE;
	}

	if (!res)
		return FALSE;

	/*
	 * It seems that backing store can't work.
	 */
	pScreen->backingStoreSupport = NotUseful;

	ews4800mipsColormapInit(pScreen);

	if (!ews4800mipsScreenInit(pScreen))
		return FALSE;
	(void)(*pScreen->SaveScreen)(pScreen, SCREEN_SAVER_OFF);

	res = cfbCreateDefColormap(pScreen);

	return res;
}

int
ews4800mipsSetDisplayMode(int fd, int mode, int *prevmode)
{

	if (prevmode != NULL) {
		if (ioctl(fd, WSDISPLAYIO_GMODE, prevmode) < 0) {
			ews4800mipsError("ioctl(WSDISPLAYIO_GMODE)");
			return -1;
		}
	}

	if (prevmode == NULL || *prevmode != mode) {
		if (ioctl(fd, WSDISPLAYIO_SMODE, &mode) < 0) {
			ews4800mipsError("ioctl(WSDISPLAYIO_SMODE)");
			return -1;
		}
	}

	return 0;
}

Bool
ews4800mips8ScreenInit(
	ScreenPtr pScreen,
	pointer pbits,		/* pointer to screen bitmap */
	int xsize, int ysize,	/* in pixels */
	int dpix, int dpiy,	/* dots per inch */
	int width		/* pixel width of frame buffer */
    )
{
	int i, j;
	VisualPtr visuals;
	DepthPtr depths;
	int nvisuals;
	int ndepths;
	int rootdepth;
	VisualID defaultVisual;
	VisualID *vid = NULL;

	if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
		return FALSE;

	ndepths = 1;
	nvisuals = 1;
	depths = (DepthPtr)xalloc(sizeof(DepthRec));
	visuals = (VisualPtr)xalloc(sizeof(VisualRec));
	vid = (VisualID *)xalloc( sizeof(VisualID));
	if (!depths || !visuals || !vid) {
		xfree(depths);
		xfree(visuals);
		xfree(vid);
		return FALSE;
	}

	depths[0].depth = 8;
	depths[0].numVids = 1;
	depths[0].vids = vid;
	visuals[0].class = PseudoColor;
	visuals[0].bitsPerRGBValue = 8;
	visuals[0].ColormapEntries = 256;
	visuals[0].nplanes = 1;
	visuals[0].vid = *vid = FakeClientID(0);
	visuals[0].redMask   = 0x00e0;
	visuals[0].greenMask = 0x001c;
	visuals[0].blueMask  = 0x0003;
	visuals[0].offsetRed   = 5;
	visuals[0].offsetGreen = 2;
	visuals[0].offsetBlue  = 0;
	rootdepth = 8;
	defaultVisual = *vid;

	if (!miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
	    rootdepth, ndepths, depths, defaultVisual, nvisuals, visuals))
		return FALSE;

	/* overwrite miCloseScreen with our own */
	pScreen->CloseScreen = cfbCloseScreen;

	/* init backing store here so we can overwrite CloseScreen
	 * without stepping on the backing store wrapped version */
	pScreen->BackingStoreFuncs = cfbBSFuncRec;

	return TRUE;
}

