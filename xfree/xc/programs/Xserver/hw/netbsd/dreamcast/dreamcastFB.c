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

/*
 * Modified from hpcFB.c of Xhpc
 */

#include "dreamcast.h"

#define FORCE_SEPARATE_PRIVATE
#include "mi.h"
#include "cfb.h"
#include "cfb16.h"

#include <stdio.h>

#undef CFB_NEED_SCREEN_PRIVATE
extern int cfb16ScreenPrivateIndex;
extern Bool cfb16CreateScreenResources(ScreenPtr pScreen);


static Bool
dreamcast16ScreenInit(ScreenPtr pScreen, pointer pbits, int xsize, int ysize,
    int dpix, int dpiy, int width)
    /* pointer pbits;		pointer to screen bitmap */
    /* int xsize, ysize;	in pixels */
    /* int dpix, dpiy;		dots per inch */
    /* int width;		pixel width of frame buffer */
{
#ifdef CFB_NEED_SCREEN_PRIVATE
    pointer oldDevPrivate;
#endif
    VisualPtr	visuals = NULL;
    DepthPtr	depths = NULL;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual = 0;
    dreamcastFbPtr pFb = dreamcastGetScreenFb(pScreen);
    struct wsdisplay_fbinfo *fbconf = &pFb->info;
    extern Bool cfb16CloseScreen(int index, ScreenPtr pScreen);

    if (!cfb16SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;

    /*
     * fbconf->hf_pixel_width == 16 is always true
     * because here is dreamcast16FinishScreenInit, dreamcast16 !
     */
    if (fbconf->depth == 16) {
        VisualID *vid = NULL;

        ndepths = 1;
        nvisuals = 1;
        depths = (DepthPtr)xalloc( sizeof(DepthRec) );
        visuals = (VisualPtr)xalloc( sizeof(VisualRec) );
        vid = (VisualID *)xalloc( sizeof(VisualID) );
        if( !depths || !visuals || !vid ) {
            xfree( depths );
            xfree( visuals );
            xfree( vid );
            return FALSE;
        }
        depths[0].depth = 16;
        depths[0].numVids = 1;
        depths[0].vids = vid;
        visuals[0].class = TrueColor;
        visuals[0].bitsPerRGBValue = 6;
        visuals[0].ColormapEntries = 1 << 6;
        visuals[0].nplanes = 16;
        visuals[0].vid = *vid = FakeClientID(0);
        visuals[0].redMask   = 0xf800;
        visuals[0].greenMask = 0x07e0;
        visuals[0].blueMask  = 0x001f;
        visuals[0].offsetRed   = 11;
        visuals[0].offsetGreen = 6;
        visuals[0].offsetBlue  = 0;
        rootdepth = 16;
        defaultVisual = *vid;
    }

#ifdef CFB_NEED_SCREEN_PRIVATE
    oldDevPrivate = pScreen->devPrivate;
#endif
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;

    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = cfb16CloseScreen;

    /* init backing store here so we can overwrite CloseScreen without stepping
     * on the backing store wrapped version */
    miInitializeBackingStore (pScreen);

#ifdef CFB_NEED_SCREEN_PRIVATE
    pScreen->CreateScreenResources = cfb16CreateScreenResources;
    pScreen->devPrivates[cfb16ScreenPrivateIndex].ptr = pScreen->devPrivate;
    pScreen->devPrivate = oldDevPrivate;
#endif
    return TRUE;
}

int
dreamcastSetDisplayMode(fd, mode, prevmode)
	int fd;
	int mode;
	int *prevmode;
{
    if (prevmode != NULL) {
	if (ioctl(fd, WSDISPLAYIO_GMODE, prevmode) < 0) {
	    dreamcastError("ioctl(WSDISPLAYIO_GMODE)");
	    return (-1);
	}
    }

    if (prevmode == NULL || *prevmode != mode) {
	    if (ioctl(fd, WSDISPLAYIO_SMODE, &mode) < 0) {
		dreamcastError("ioctl(WSDISPLAYIO_SMODE)");
		return (-1);
	    }
    }

    return (0);
}

Bool
dreamcastFBInit(screen, pScreen, argc, argv)
    int	    	  screen;    	/* what screen am I going to be */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
	int res;
	dreamcastFbPtr pFb = dreamcastGetScreenFb(pScreen);
	unsigned char *fb = pFb->fb;
	struct wsdisplay_fbinfo *fbconf = &pFb->info;

	pFb->EnterLeave = (void *)NoopDDA;

	if (!dreamcastAllocateScreenPrivate(pScreen))
	    return FALSE;

	if (!fb) {
            if (ioctl(pFb->fd, WSDISPLAYIO_GINFO, fbconf) < 0) {
                dreamcastError("ioctl(WSDISPLAYIO_GINFO)");
                return FALSE;
            }


	    if (dreamcastSetDisplayMode(pFb->fd, WSDISPLAYIO_MODE_MAPPED, NULL) < 0)
		return FALSE;

	    fb = dreamcastMemoryMap((size_t)fbconf->width * fbconf->height *fbconf->depth/8,
			      0, pFb->fd);
	    if (fb == NULL)
		return FALSE;

	    pFb->fb = fb;
	}

	switch (fbconf->depth) {
	case 16:
	    res = dreamcast16ScreenInit(pScreen, fb,
				  fbconf->width,
				  fbconf->height,
				  monitorResolution, monitorResolution,
				  fbconf->width);
		break;
	default:
		dreamcastErrorF(("%dbpp frame buffer is not supported\n",
		    fbconf->depth));
		return FALSE;
	}

	if (!res) return FALSE;

	/*
	 * It seems that backing store can't work.
	 */
	pScreen->backingStoreSupport = NotUseful;

	dreamcastColormapInit(pScreen);

	if (!dreamcastScreenInit(pScreen))
		return FALSE;
	(void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);

	res = cfbCreateDefColormap(pScreen);

	return res;
}
