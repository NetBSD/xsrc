/* $NetBSD: hpcFB.c,v 1.6 2006/03/15 02:11:35 uwe Exp $	*/
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

#include "hpc.h"

#define FORCE_SEPARATE_PRIVATE
#include "cfb.h"

#include <stdio.h>

/* Adapt cfb logic */
#undef CFB_NEED_SCREEN_PRIVATE
#if 0 /* XXX */
#if !defined(SINGLEDEPTH) || defined(FORCE_SEPARATE_PRIVATE)
#define CFB_NEED_SCREEN_PRIVATE
#endif
#endif /* XXX */

extern BSFuncRec cfbBSFuncRec, cfb16BSFuncRec;
extern int cfb16ScreenPrivateIndex;
extern Bool cfbCreateScreenResources(ScreenPtr pScreen);
extern Bool cfb16CreateScreenResources(ScreenPtr pScreen);

Bool
hpc8ScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int	i, j;
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;
    VisualID *vid = NULL;
    hpcFbPtr pFb = hpcGetScreenFb(pScreen);
    struct hpcfb_fbconf *fbconf = &pFb->info;

    if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;

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
    depths[0].depth = 8;
    depths[0].numVids = 1;
    depths[0].vids = vid;
    if (fbconf->hf_access_flags & HPCFB_ACCESS_STATIC) {
	    visuals[0].class = StaticColor;
    } else {
	    visuals[0].class = PseudoColor;
    }
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

    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;

    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = cfbCloseScreen;

    /* init backing store here so we can overwrite CloseScreen without stepping
     * on the backing store wrapped version */
    pScreen->BackingStoreFuncs = cfbBSFuncRec;

    return TRUE;
}

Bool
hpc16ScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    int	i, j;
#ifdef CFB_NEED_SCREEN_PRIVATE
    pointer oldDevPrivate;
#endif
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;
    hpcFbPtr pFb = hpcGetScreenFb(pScreen);
    struct hpcfb_fbconf *fbconf = &pFb->info;
    extern Bool cfb16CloseScreen(int index, ScreenPtr pScreen);

    if (!cfb16SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;

    /*
     * fbconf->hf_pixel_width == 16 is always true
     * because here is hpc16FinishScreenInit, hpc16 !
     */
    if (fbconf->hf_pixel_width == 16) {
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
        visuals[0].nplanes = 15;
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
    pScreen->BackingStoreFuncs = cfb16BSFuncRec;

#ifdef CFB_NEED_SCREEN_PRIVATE
    pScreen->CreateScreenResources = cfb16CreateScreenResources;
    pScreen->devPrivates[cfb16ScreenPrivateIndex].ptr = pScreen->devPrivate;
    pScreen->devPrivate = oldDevPrivate;
#endif
    return TRUE;
}

int
hpcSetDisplayMode(fd, mode, prevmode)
	int fd;
	int mode;
	int *prevmode;
{
    if (prevmode != NULL)
	if (ioctl(fd, WSDISPLAYIO_GMODE, prevmode) < 0)
	    return -1;

    if (prevmode == NULL || *prevmode != mode)
	if (ioctl(fd, WSDISPLAYIO_SMODE, &mode) < 0)
	    return -1;

    return 0;
}

Bool
hpcFBInit(screen, pScreen, argc, argv)
    int	    	  screen;    	/* what screen am I going to be */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
	int res;
	hpcFbPtr pFb = hpcGetScreenFb(pScreen);
	unsigned char *fb = pFb->fb;
	int rowsize;
	struct hpcfb_fbconf *fbconf = &pFb->info;

	pFb->EnterLeave = (void (*)())NoopDDA;

	if (!hpcAllocateScreenPrivate(pScreen))
	    return FALSE;

	if (!fb) {
	    fbconf->hf_conf_index = HPCFB_CURRENT_CONFIG;
	    if (ioctl(pFb->fd, HPCFBIO_GCONF, fbconf) < 0) {
		hpcError("ioctl(HPCFBIO_GCONF)");
		return FALSE;
	    }
	    /* this isn't error */
	    hpcErrorF(("%s: %dx%d (%dbytes/line) %dbit offset=%lx\n",
		pFb->devname,
		fbconf->hf_width,
		fbconf->hf_height,
		fbconf->hf_bytes_per_line,
		fbconf->hf_pixel_width,
		fbconf->hf_offset));

	    if (hpcSetDisplayMode(pFb->fd, WSDISPLAYIO_MODE_MAPPED, NULL) < 0) {
		Error("ioctl(WSDISPLAYIO_SMODE)");
		return FALSE;
	    }

	    fb = hpcMemoryMap((size_t)fbconf->hf_bytes_per_line * fbconf->hf_height,
			      fbconf->hf_offset, pFb->fd);
	    if (fb == NULL)
		return FALSE;

	    pFb->fb = fb;
	}

	switch (fbconf->hf_pixel_width) {
	case 1:
	    res = mfbScreenInit(pScreen, fb,
				fbconf->hf_width,
				fbconf->hf_height,
				monitorResolution, monitorResolution,
				fbconf->hf_bytes_per_line * 8);
	    break;
	case 8:
	    res = hpc8ScreenInit(pScreen, fb,
				 fbconf->hf_width,
				 fbconf->hf_height,
				 monitorResolution, monitorResolution,
				 fbconf->hf_bytes_per_line);
	    break;
	case 16:
	    res = hpc16ScreenInit(pScreen, fb,
				  fbconf->hf_width,
				  fbconf->hf_height,
				  monitorResolution, monitorResolution,
				  fbconf->hf_bytes_per_line / 2);
		break;
	default:
		hpcErrorF(("%dbpp frame buffer is not supported\n",
		    fbconf->hf_pixel_width));
		return FALSE;
	}

	if (!res) return FALSE;

	/*
	 * It seems that backing store can't work.
	 */
	pScreen->backingStoreSupport = NotUseful;

	hpcColormapInit(pScreen);

	if (!hpcScreenInit(pScreen))
		return FALSE;
	(void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);

	res = cfbCreateDefColormap(pScreen);

	return res;
}
