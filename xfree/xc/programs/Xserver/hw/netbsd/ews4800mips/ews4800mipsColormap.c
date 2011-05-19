/* $XConsortium: sunCfb.c,v 1.15.1.2 95/01/12 18:54:42 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/sun/sunCfb.c,v 3.2 1995/02/12 02:36:22 dawes Exp $ */
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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
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
/* Modified from  sunCG4C.c for X11R3 by Tom Jarmolowski	*/
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

#include "ews4800mips.h"
#include "cfb.h"

#include <stdio.h>

static void
ews4800mipsUpdateColormap(ScreenPtr pScreen, int dex, int count, u_char *rmap,
    u_char *gmap, u_char *bmap)
{
	struct wsdisplay_cmap cmap;
	ews4800mipsFbPtr pFb = ews4800mipsGetScreenFb(pScreen);

	cmap.index = dex;
	cmap.count = count;
	cmap.red = &rmap[dex];
	cmap.green = &gmap[dex];
	cmap.blue = &bmap[dex];

	if (ioctl(pFb->fd, WSDISPLAYIO_PUTCMAP, &cmap) < 0) {
		ews4800mipsError("UpdateColormap");
		ews4800mipsFatalError(("UpdateColormap: WSDISPLAY_PUTCMAP failed\n"));
	}
}

static void
ews4800mipsInstallColormap(ColormapPtr cmap)
{
	ews4800mipsScreenPtr pPrivate = ews4800mipsGetScreenPrivate(cmap->pScreen);
	ews4800mipsFbPtr pFb = ews4800mipsGetScreenFb(cmap->pScreen);
	register int i;
	register Entry *pent;
	register VisualPtr pVisual = cmap->pVisual;
	u_char	  rmap[256], gmap[256], bmap[256];
	unsigned long rMask, gMask, bMask;
	int	oRed, oGreen, oBlue;

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
			} else {
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

static void
ews4800mipsUninstallColormap(ColormapPtr cmap)
{
	ews4800mipsScreenPtr pPrivate = ews4800mipsGetScreenPrivate(cmap->pScreen);

	if (cmap == pPrivate->installedMap) {
		Colormap defMapID = cmap->pScreen->defColormap;

		if (cmap->mid != defMapID) {
			ColormapPtr defMap;
			defMap = (ColormapPtr) LookupIDByType(defMapID, RT_COLORMAP);
			if (defMap)
				(*cmap->pScreen->InstallColormap)(defMap);
			else
				ews4800mipsErrorF(("ews4800mipsUninstallColormap: Can't find default colormap\n"));
		}
	}
}

static int
ews4800mipsListInstalledColormaps(ScreenPtr pScreen, Colormap *pCmapList)
{
	ews4800mipsScreenPtr pPrivate = ews4800mipsGetScreenPrivate(pScreen);

	*pCmapList = pPrivate->installedMap->mid;
	return (1);
}

static void
ews4800mipsStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
	ews4800mipsScreenPtr pPrivate = ews4800mipsGetScreenPrivate(pmap->pScreen);
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

Bool
ews4800mipsInitializeColormap(register ColormapPtr pmap)
{
	int i;
	VisualPtr pVisual;
	ScreenPtr pScreen;
	ews4800mipsFbPtr pFb;

	pVisual = pmap->pVisual;
	pScreen = pmap->pScreen;
	pFb = ews4800mipsGetScreenFb(pScreen);

	switch (pVisual->class) {
	case StaticGray:
	case GrayScale:
		return FALSE;
		break;
	case StaticColor: {
		struct wsdisplay_cmap cmap;
		u_char	  rmap[256], gmap[256], bmap[256];

		cmap.index = 0;
		cmap.count = 256;
		cmap.red = rmap;
		cmap.green = gmap;
		cmap.blue = bmap;

		if (ioctl(pFb->fd, WSDISPLAYIO_GETCMAP, &cmap) < 0) {
			ews4800mipsFatalError(("ews4800mipsInitialiseColormap: WSDISPLAY_GETCMAP failed\n"));
		}
		for (i = 0; i < 256; i++) {
			pmap->red[i].co.local.red = (long)rmap[i] << 8;
			pmap->red[i].co.local.green = (long)gmap[i] << 8;
			pmap->red[i].co.local.blue = (long)bmap[i] << 8;
		}
	}
	break;
	case PseudoColor:
	case TrueColor:
	case DirectColor:
	default:
		return FALSE;
		break;
	}
}

void
ews4800mipsColormapInit(ScreenPtr pScreen)
{
	ews4800mipsScreenPtr pPrivate = ews4800mipsGetScreenPrivate(pScreen);
	ews4800mipsFbPtr pFb = ews4800mipsGetScreenFb(pScreen);

	if (pScreen->rootDepth == 8) {
		pScreen->InstallColormap = ews4800mipsInstallColormap;
		pScreen->UninstallColormap = ews4800mipsUninstallColormap;
		pScreen->ListInstalledColormaps = ews4800mipsListInstalledColormaps;
		pScreen->StoreColors = ews4800mipsStoreColors;
		pPrivate->UpdateColormap = ews4800mipsUpdateColormap;
	} else {
		pScreen->InstallColormap = cfbInstallColormap;
		pScreen->UninstallColormap = cfbUninstallColormap;
		pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
		pScreen->StoreColors = (void (*)())NoopDDA;
	}
}
