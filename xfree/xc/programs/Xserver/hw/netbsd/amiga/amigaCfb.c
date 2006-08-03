
/* $XConsortium: amigaCfb.c,v 1.14 94/04/17 20:29:34 kaleb Exp $ */

/*
Copyright (c) 1990  X Consortium

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
 */

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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/****************************************************************/
/* Modified from  amigaCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

/* 
 * Copyright 1991, 1992, 1993 Kaleb S. Keithley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  Kaleb S. Keithley makes no 
 * representations about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include "amiga.h"
#include "cfb.h"
#include <stdio.h>
#include "miline.h"
    
#define GXZEROLINEBIAS	(OCTANT1 | OCTANT3 | OCTANT4 | OCTANT6)

static void CGUpdateColormap(pScreen, dex, count, rmap, gmap, bmap)
    ScreenPtr	pScreen;
    int		dex, count;
    u_char	*rmap, *gmap, *bmap;
{
    struct grf_colormap amigaCmap;

    amigaCmap.index = dex;
    amigaCmap.count = count;
    amigaCmap.red = &rmap[dex];
    amigaCmap.green = &gmap[dex];
    amigaCmap.blue = &bmap[dex];

    if (ioctl(amigaFbs[pScreen->myNum].fd, GRFIOCPUTCMAP, &amigaCmap) < 0) {
	Error("CGUpdateColormap");
	FatalError( "CGUpdateColormap: GRFIOCPUTCMAP failed\n" );
    }
}

void amigaInstallColormap(cmap)
    ColormapPtr	cmap;
{
    SetupScreen(cmap->pScreen);
    register int i;
    register Entry *pent;
    register VisualPtr pVisual = cmap->pVisual;
    u_char	  rmap[256], gmap[256], bmap[256];
    unsigned long rMask, gMask, bMask;
    int       oRed, oGreen, oBlue;

    if (cmap == pPrivate->installedMap)
	return;
    if (pPrivate->installedMap)
	WalkTree(pPrivate->installedMap->pScreen, TellLostMap,
		 (pointer) &(pPrivate->installedMap->mid));
    if ((pVisual->class | DynamicClass) == DirectColor) {
	if (pVisual->ColormapEntries < 256) {
	    rMask = pVisual->redMask;
	    gMask = pVisual->greenMask;
	    bMask = pVisual->blueMask;
	    oRed = pVisual->offsetRed;
	    oGreen = pVisual->offsetGreen;
	    oBlue = pVisual->offsetBlue;
	} else {
	    rMask = gMask = bMask = 255;
	    oRed = oGreen = oBlue = 0;
	}
	for (i = 0; i < 256; i++) {
	    rmap[i] = cmap->red[(i & rMask) >> oRed].co.local.red >> 8;
	    gmap[i] = cmap->green[(i & gMask) >> oGreen].co.local.green >> 8;
	    bmap[i] = cmap->blue[(i & bMask) >> oBlue].co.local.blue >> 8;
	}
    } else {
	for (i = 0, pent = cmap->red;
	     i < pVisual->ColormapEntries;
	     i++, pent++) {
	    if (pent->fShared) {
		rmap[i] = pent->co.shco.red->color >> 8;
		gmap[i] = pent->co.shco.green->color >> 8;
		bmap[i] = pent->co.shco.blue->color >> 8;
	    }
	    else {
		rmap[i] = pent->co.local.red >> 8;
		gmap[i] = pent->co.local.green >> 8;
		bmap[i] = pent->co.local.blue >> 8;
	    }
	}
    }
    pPrivate->installedMap = cmap;
    (*pPrivate->UpdateColormap) (cmap->pScreen, 0, 256, rmap, gmap, bmap);
    WalkTree(cmap->pScreen, TellGainedMap, (pointer) &(cmap->mid));
}

void amigaUninstallColormap(cmap)
    ColormapPtr	cmap;
{
    SetupScreen(cmap->pScreen);
    if (cmap == pPrivate->installedMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    ColormapPtr defMap = (ColormapPtr) LookupIDByType(defMapID,
							      RT_COLORMAP);

	    if (defMap)
		(*cmap->pScreen->InstallColormap)(defMap);
	    else
	        ErrorF("amigaFbs: Can't find default colormap\n");
	}
    }
}

int amigaListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr	pScreen;
    Colormap	*pCmapList;
{
    SetupScreen(pScreen);
    *pCmapList = pPrivate->installedMap->mid;
    return (1);
}

static void CGStoreColors(pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*pdefs;
{
    SetupScreen(pmap->pScreen);
    u_char	rmap[256], gmap[256], bmap[256];
    xColorItem	expanddefs[256];
    register int i;

    if (pPrivate->installedMap != NULL && pPrivate->installedMap != pmap)
	return;
    if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
	ndef = cfbExpandDirectColors(pmap, ndef, pdefs, expanddefs);
	pdefs = expanddefs;
    }
    while (ndef--) {
	i = pdefs->pixel;
	rmap[i] = pdefs->red >> 8;
	gmap[i] = pdefs->green >> 8;
	bmap[i] = pdefs->blue >> 8;
	(*pPrivate->UpdateColormap) (pmap->pScreen, i, 1, rmap, gmap, bmap);
	pdefs++;
    }
}

/* these are copies from STATIC section in cfb. Having them here doesn't
   force us to go STATIC_COLOR */
static ColormapPtr InstalledMaps[MAXSCREENS];

static int
amigacfbListInstalledColormaps(pScreen, pmaps)
    ScreenPtr	pScreen;
    Colormap	*pmaps;
{
    /* By the time we are processing requests, we can guarantee that there
     * is always a colormap installed */
    *pmaps = InstalledMaps[pScreen->myNum]->mid;
    return (1);
}


static void
amigacfbInstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr oldpmap = InstalledMaps[index];

    if(pmap != oldpmap)
    {
	/* Uninstall pInstalledMap. No hardware changes required, just
	 * notify all interested parties. */
	if(oldpmap != (ColormapPtr)None)
	    WalkTree(pmap->pScreen, TellLostMap, (char *)&oldpmap->mid);
	/* Install pmap */
	InstalledMaps[index] = pmap;
	WalkTree(pmap->pScreen, TellGainedMap, (char *)&pmap->mid);

    }
}

static void
amigacfbUninstallColormap(pmap)
    ColormapPtr	pmap;
{
    int index = pmap->pScreen->myNum;
    ColormapPtr curpmap = InstalledMaps[index];

    if(pmap == curpmap)
    {
	if (pmap->mid != pmap->pScreen->defColormap)
	{
	    curpmap = (ColormapPtr) LookupIDByType(pmap->pScreen->defColormap,
						   RT_COLORMAP);
	    (*pmap->pScreen->InstallColormap)(curpmap);
	}
    }
}


void CGScreenInit (pScreen)
    ScreenPtr	pScreen;
{

#ifndef STATIC_COLOR /* { */
    SetupScreen (pScreen);
    if (pScreen->rootDepth == 8) {
      pScreen->InstallColormap = amigaInstallColormap;
      pScreen->UninstallColormap = amigaUninstallColormap;
      pScreen->ListInstalledColormaps = amigaListInstalledColormaps;
      pScreen->StoreColors = CGStoreColors;
      pPrivate->UpdateColormap = CGUpdateColormap;
      if (amigaFlipPixels) {
	pScreen->whitePixel = (Pixel) 1;
	pScreen->blackPixel = (Pixel) 0;
      } else {
	pScreen->whitePixel = (Pixel) 0;
	pScreen->blackPixel = (Pixel) 1;
      }
    }
    else {
      /* initialize colors 0 and 1 so sprite is visible */
      static unsigned char bw[2] = { 0, 255 };
      CGUpdateColormap (pScreen, 0, 2, bw, bw, bw);

      pScreen->InstallColormap = amigacfbInstallColormap;
      pScreen->UninstallColormap = amigacfbUninstallColormap;
      pScreen->ListInstalledColormaps = amigacfbListInstalledColormaps;
      pScreen->StoreColors = (void (*)())NoopDDA;
      if (pScreen->rootDepth > 16)
	pScreen->backingStoreSupport = NotUseful;
    }

#endif /* } */
}

static void checkMono (argc, argv)
    int argc;
    char** argv;
{
    int i;

    for (i = 1; i < argc; i++)
	if (strcmp (argv[i], "-mono") == 0)
	    ErrorF ("-mono not appropriate for 3rd party gfx boards\n");
}

#ifdef GFX_CARD_SUPPORT
Bool UseGfxHardwareCursor = FALSE;

Bool amigaGRFInit (screen, pScreen, argc, argv)
    int	    	  screen;    	/* what screen am I going to be */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    pointer	fb;

    checkMono (argc, argv);

    if (!amigaScreenAllocate (pScreen))
	return FALSE;
    if (!amigaFbs[screen].fb) {
	if ((fb = amigaMemoryMap (amigaFbs[screen].info.gd_fbsize,
			     amigaFbs[screen].info.gd_regsize,
			     amigaFbs[screen].fd)) == NULL)
	    return FALSE;
	amigaFbs[screen].fb = fb;
    }
    amigaFbs[screen].EnterLeave = (void (*)())NoopDDA;

    if (!amigaCfbScreenInit (pScreen, 
	    amigaFbs[screen].fb,
	    amigaFbs[screen].info.gd_fbwidth, 
	    amigaFbs[screen].info.gd_fbheight,
	    monitorResolution, monitorResolution, 
	    amigaFbs[screen].info.gd_fbwidth,
	    amigaFbs[screen].info.gd_planes))
	return FALSE;

    CGScreenInit (pScreen);

    if (amigaUseHWC)
	UseGfxHardwareCursor = TRUE; 
    if (!amigaScreenInit (pScreen))
	return FALSE;
    UseGfxHardwareCursor = FALSE; 

    amigaSaveScreen (pScreen, SCREEN_SAVER_OFF);
    return cfbCreateDefColormap(pScreen);
}

#ifdef RETINAZ3_SUPPORT

Bool amigaRZ3Init (screen, pScreen, argc, argv)
    int		screen;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	pScreen;  	/* The Screen to initialize */
    int		argc;	    	/* The number of the Server's arguments. */
    char**	argv;   	/* The arguments themselves. Don't change! */
{
    pointer	fb;

    checkMono (argc, argv);

    if (!amigaScreenAllocate (pScreen))
	return FALSE;
    if (!amigaFbs[screen].fb) {
	if ((fb = amigaMemoryMap (amigaFbs[screen].info.gd_fbsize,
			     amigaFbs[screen].info.gd_regsize,
			     amigaFbs[screen].fd)) == NULL)
	    return FALSE;
	amigaFbs[screen].fb = fb;
	if ((fb = amigaMemoryMap (amigaFbs[screen].info.gd_regsize,
				  0, amigaFbs[screen].fd)) == NULL)
	    return FALSE;
	amigaFbs[screen].regs = fb;
    }
    amigaFbs[screen].EnterLeave = (void (*)())NoopDDA;
    if (!amigaCfbSetupScreen (pScreen, 
	    amigaFbs[screen].fb,
	    amigaFbs[screen].info.gd_fbwidth, 
	    amigaFbs[screen].info.gd_fbheight,
	    monitorResolution, monitorResolution, 
	    amigaFbs[screen].info.gd_fbwidth,
	    amigaFbs[screen].info.gd_planes))
	return FALSE;
    if (amigaNoGX == FALSE) {
	if (!amigaGXInit (pScreen, &amigaFbs[screen]))
	    return FALSE;
    }
    if (amigaFbs[screen].info.gd_planes != 8) {
	if (!cfbSetVisualTypes (amigaFbs[screen].info.gd_planes,
			      ((1 << StaticGray) | (1 << TrueColor)),
			      amigaFbs[screen].info.gd_planes))
	return FALSE;
    }
    if (!amigaCfbFinishScreenInit(pScreen,
	    amigaFbs[screen].fb,
	    amigaFbs[screen].info.gd_fbwidth, 
	    amigaFbs[screen].info.gd_fbheight,
	    monitorResolution, monitorResolution, 
	    amigaFbs[screen].info.gd_fbwidth,
	    amigaFbs[screen].info.gd_planes))
	return FALSE;
    if (amigaNoGX == FALSE) {
	miSetZeroLineBias(pScreen, GXZEROLINEBIAS);
    }
    CGScreenInit (pScreen);

    if (amigaUseHWC)
	UseGfxHardwareCursor = TRUE; 
    if (!amigaScreenInit (pScreen))
	return FALSE;
    UseGfxHardwareCursor = FALSE;

    amigaSaveScreen (pScreen, SCREEN_SAVER_OFF);
    return cfbCreateDefColormap(pScreen);
}
#endif /* RETINAZ3_SUPPORT */

#ifdef CIRRUS_SUPPORT

Bool amigaCLInit (screen, pScreen, argc, argv)
    int		screen;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	pScreen;  	/* The Screen to initialize */
    int		argc;	    	/* The number of the Server's arguments. */
    char**	argv;   	/* The arguments themselves. Don't change! */
{
    pointer	fb;

    checkMono (argc, argv);

    if (!amigaScreenAllocate (pScreen))
	return FALSE;
    if (!amigaFbs[screen].fb) {
	if ((fb = amigaMemoryMap (amigaFbs[screen].info.gd_fbsize,
			     amigaFbs[screen].info.gd_regsize,
			     amigaFbs[screen].fd)) == NULL)
	    return FALSE;
	amigaFbs[screen].fb = fb;
	if ((fb = amigaMemoryMap (amigaFbs[screen].info.gd_regsize,
				  0, amigaFbs[screen].fd)) == NULL)
	    return FALSE;
	amigaFbs[screen].regs = fb;
    }
    amigaFbs[screen].EnterLeave = (void (*)())NoopDDA;
    if (!amigaCfbSetupScreen (pScreen, 
	    amigaFbs[screen].fb,
	    amigaFbs[screen].info.gd_fbwidth, 
	    amigaFbs[screen].info.gd_fbheight,
	    monitorResolution, monitorResolution, 
	    amigaFbs[screen].info.gd_fbwidth,
	    amigaFbs[screen].info.gd_planes))
	return FALSE;
    if (amigaNoGX == FALSE) {
	if (!amigaCLGXInit (pScreen, &amigaFbs[screen]))
	    return FALSE;
    }
    if (amigaFbs[screen].info.gd_planes != 8) {
	if (!cfbSetVisualTypes (amigaFbs[screen].info.gd_planes,
			      ((1 << StaticGray) | (1 << TrueColor)),
			      amigaFbs[screen].info.gd_planes))
	return FALSE;
    }
    if (!amigaCfbFinishScreenInit(pScreen,
	    amigaFbs[screen].fb,
	    amigaFbs[screen].info.gd_fbwidth, 
	    amigaFbs[screen].info.gd_fbheight,
	    monitorResolution, monitorResolution, 
	    amigaFbs[screen].info.gd_fbwidth,
	    amigaFbs[screen].info.gd_planes))
	return FALSE;
    if (amigaNoGX == FALSE) {
	miSetZeroLineBias(pScreen, GXZEROLINEBIAS);
    }
    CGScreenInit (pScreen);

    if (amigaUseHWC)
	UseGfxHardwareCursor = TRUE; 
    if (!amigaScreenInit (pScreen))
	return FALSE;
    UseGfxHardwareCursor = FALSE; 

    amigaSaveScreen (pScreen, SCREEN_SAVER_OFF);
    return cfbCreateDefColormap(pScreen);
}

#endif /* CIRRUS_SUPPORT */
#endif /* GFX_CARD_SUPPORT */

