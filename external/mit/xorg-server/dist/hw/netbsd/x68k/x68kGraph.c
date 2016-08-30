/* $NetBSD: x68kGraph.c,v 1.4 2016/08/30 07:50:55 mrg Exp $ */
/*-------------------------------------------------------------------------
 * Copyright (c) 1996 Yasushi Yamasaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yasushi Yamasaki
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------*/

/* $XConsortium: sunCfb.c,v 1.15.1.2 95/01/12 18:54:42 kaleb Exp $ */

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

#include "x68k.h"
#include "mi.h"
#include "micmap.h"

#include "fb.h"

/* local functions */
static Bool x68kCfbFinishScreenInit(ScreenPtr pScreen, pointer pbits,
                                    int xsize, int ysize,
                                    int dpix, int dpiy, int width);
static void x68kInstallColormap(ColormapPtr cmap);
static void x68kUninstallColormap(ColormapPtr cmap);
static int x68kListInstalledColormaps(ScreenPtr pScreen, Colormap *pCmapList);
static void x68kStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs);

static void x68kUpdateColormap(ScreenPtr pScreen, int dex, int count,
                               u_char *rmap, u_char *gmap, u_char *bmap);

/*-------------------------------------------------------------------------
 * function "x68kGraphOpen"                          [ X68kFBProc function ]
 *
 *  purpose:  call common frame buffer opening procedure
 *            then set hardware colormap for several static color modes. 
 *  argument: (X68kScreenRec *)pPriv : X68k private screen record
 *  returns:  (Bool): TRUE  if successed
 *                    FALSE otherwise
 *-----------------------------------------------------------------------*/
Bool
x68kGraphOpen(X68kScreenRec *pPriv)
{
    if( !x68kFbCommonOpen(pPriv, "/dev/grf1") )
        return FALSE;

    /* initialize hardware colormap
       in cases of static visual class */
    if (pPriv->depth == 15 && pPriv->class == TrueColor) {
        /* for 32768 TrueColor mode */
	int i;
	u_short x = 0x0001;
	for ( i = 0; i < 256; ) {
	    pPriv->reg->gpal[i++] = x;
	    pPriv->reg->gpal[i++] = x;
 	    x += 0x0202;
        }
    }
    if (pPriv->depth == 4 && pPriv->class == StaticGray ) {
        /* for 16 StaticGray mode */
        int i;
        for( i = 0; i < 16; i++ )
            pPriv->reg->gpal[i] = (i*2 << 11) | (i*2 << 6) | (i*2 << 1);
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * function "x68kGraphClose"                        [ X68kFBProc function ]
 *
 *  purpose:  close graphic frame buffer
 *  argument: (X68kScreenRec *)pPriv : X68k private screen record
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
void
x68kGraphClose(X68kScreenRec *pPriv)
{
    x68kFbCommonClose(pPriv);
}

/*-------------------------------------------------------------------------
 * function "x68kGraphInit"                     [ called by DIX AddScreen ]
 *
 *  purpose:  initialize graphic frame buffer
 *  argument: (ScreenPtr)pScreen       : DIX screen record
 *            (int)argc, (char **)argv : standard C arguments
 *  returns:  (Bool) : TRUE  if succeeded
 *                     FALSE otherwise
 *-----------------------------------------------------------------------*/
Bool
x68kGraphInit(ScreenPtr pScreen, int argc, char *argv[])
{
    X68kScreenRec *pPriv;

    /* get private screen record set by X68KConfig */
    pPriv = x68kGetScreenRecByType(X68K_FB_GRAPHIC);
    
    /* store private record into screen */
    if (!dixRegisterPrivateKey(&x68kScreenPrivateKeyRec, PRIVATE_SCREEN, 0)) {
        ErrorF("dixRegisterPrivateKey failed");
        return FALSE;
    }
    x68kSetScreenPrivate(pScreen, pPriv);
    
    /* register normal cfb screen functions */
    if (!fbSetupScreen(pScreen, pPriv->fb,
                       pPriv->scr_width, pPriv->scr_height,
                       pPriv->dpi, pPriv->dpi, pPriv->fb_width,
		       pPriv->depth))
	return FALSE;

    /* register colormap functions */
    pScreen->InstallColormap = x68kInstallColormap;
    pScreen->UninstallColormap = x68kUninstallColormap;
    pScreen->ListInstalledColormaps = x68kListInstalledColormaps;
    pScreen->StoreColors = x68kStoreColors;
    
    /* visual initialization and etc.. */
    if (!x68kCfbFinishScreenInit(pScreen, pPriv->fb,
                                 pPriv->scr_width, pPriv->scr_height,
                                 pPriv->dpi, pPriv->dpi, pPriv->fb_width))
        return FALSE;

    if ( !miDCInitialize(pScreen, &x68kPointerScreenFuncs) )
        return FALSE;

    pScreen->whitePixel = 1;
    pScreen->blackPixel = 0;
    if ( !miCreateDefColormap(pScreen) )
        return FALSE;
    
    return TRUE;
}

/*-------------------------------------------------------------------------
 * function "x68kCfbFinishScreenInit"
 *
 *  purpose:  initialize visuals and perform miscellaneous settings
 *  argument: (ScreenPtr)pScreen     : DIX screen record
 *            (pointer)pbits         : frame buffer
 *            (int)xsize, (int)ysize : screen size
 *            (int)dpix, (int)dpiy   : screen resolution in dots per inch
 *            (int)width             : pixel width of frame buffer
 *  returns:  (Bool) : TRUE  if succeeded
 *                     FALSE otherwise
 *-----------------------------------------------------------------------*/
static Bool
x68kCfbFinishScreenInit(
    ScreenPtr pScreen,
    pointer pbits,
    int xsize, int ysize,
    int dpix, int dpiy,
    int width)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(pScreen);
    VisualPtr	visuals;
    int		nvisuals;
    DepthPtr	depths;
    int		ndepths;
    VisualID	defaultVisual;
    int		rootdepth = 0;

    /* for 15/16bit TrueColor visual mode */
    if (pPriv->depth == 15 && pPriv->class == TrueColor) {
        VisualID *vid = NULL;

        ndepths = 1;
        nvisuals = 1;
        depths = (DepthPtr)malloc( sizeof(DepthRec) );
        visuals = (VisualPtr)malloc( sizeof(VisualRec) );
        vid = (VisualID *)malloc( sizeof(VisualID) );
        if( !depths || !visuals || !vid ) {
            free( depths );
            free( visuals );
            free( vid );
            return FALSE;
        }
        depths[0].depth = 15;
        depths[0].numVids = 1;
        depths[0].vids = vid;
        visuals[0].class = TrueColor;
        visuals[0].bitsPerRGBValue = 5;
        visuals[0].ColormapEntries = 1 << 5;
        visuals[0].nplanes = 15;
        visuals[0].vid = *vid = FakeClientID(0);
        visuals[0].greenMask = 0xf800;
        visuals[0].redMask   = 0x07c0;
        visuals[0].blueMask  = 0x003e;
        visuals[0].offsetGreen = 11;
        visuals[0].offsetRed   = 6;
        visuals[0].offsetBlue  = 1;
        rootdepth = 15;
        defaultVisual = *vid;
    }
    /* for 4/16bit StaticGray visual mode */
    else if (pPriv->depth == 4 && pPriv->class == StaticGray ) {
        VisualID *vid = NULL;
        
        ndepths = 1;
        nvisuals = 1;
        depths = (DepthPtr)malloc( sizeof(DepthRec) );
        visuals = (VisualPtr)malloc( sizeof(VisualRec) );
        vid = (VisualID *)malloc( sizeof(VisualID) );
        if( !depths || !visuals || !vid ) {
            free( depths );
            free( visuals );
            free( vid );
            return FALSE;
        }
        depths[0].depth = 4;
        depths[0].numVids = 1;
        depths[0].vids = vid;
        visuals[0].class = StaticGray;
        visuals[0].bitsPerRGBValue = 4;
        visuals[0].ColormapEntries = 1 << 4;
        visuals[0].nplanes = 4;
        visuals[0].vid = *vid = FakeClientID(0);
        visuals[0].greenMask = 0;
        visuals[0].redMask   = 0;
        visuals[0].blueMask  = 0;
        visuals[0].offsetGreen = 0;
        visuals[0].offsetRed   = 0;
        visuals[0].offsetBlue  = 0;
        rootdepth = 4;
        defaultVisual = *vid;
    }
    /* for other modes ( supports all visuals ) */
    else if (!miInitVisuals(&visuals, &depths, &nvisuals, &ndepths,
                            &rootdepth, &defaultVisual, 1 << (16 - 1),
                            5, PseudoColor))
	return FALSE;

    if (!miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
                      rootdepth, ndepths, depths,
                      defaultVisual, nvisuals, visuals))
	return FALSE;
    
    pScreen->CloseScreen = fbCloseScreen;
    pScreen->SaveScreen = x68kSaveScreen;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * function "x68kInstallColormap"                      [ screen function ]
 *
 *  purpose:  install colormap, then update hardware colormap.
 *  argument: (ColormapPtr)cmap : colormap to install
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kInstallColormap(ColormapPtr cmap)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(cmap->pScreen);
    register int i;
    register Entry *pent;
    register VisualPtr pVisual = cmap->pVisual;
    u_char   rmap[256], gmap[256], bmap[256];
    unsigned long rMask, gMask, bMask;
    int	oRed, oGreen, oBlue;

    if (cmap == pPriv->installedMap)
	return;
    if (pPriv->installedMap)
	WalkTree(pPriv->installedMap->pScreen, TellLostMap,
		 (pointer) &(pPriv->installedMap->mid));

    if (pPriv->class & DynamicClass) {
        if ((cmap->pVisual->class | DynamicClass) == DirectColor) {
            rMask = pVisual->redMask;
            gMask = pVisual->greenMask;
            bMask = pVisual->blueMask;
            oRed = pVisual->offsetRed;
            oGreen = pVisual->offsetGreen;
            oBlue = pVisual->offsetBlue;
            for (i = 0; i < 1<<(pPriv->depth); i++) {
                rmap[i] = cmap->red[(i & rMask) >> oRed].co.local.red >> 11;
                gmap[i] = cmap->green[(i & gMask) >> oGreen].co.local.green >> 11;
                bmap[i] = cmap->blue[(i & bMask) >> oBlue].co.local.blue >> 11;
            }
        } else {
            for (i = 0, pent = cmap->red;
                 i < pVisual->ColormapEntries;
                 i++, pent++) {
                if (pent->fShared) {
                    rmap[i] = pent->co.shco.red->color >> 11;
                    gmap[i] = pent->co.shco.green->color >> 11;
                    bmap[i] = pent->co.shco.blue->color >> 11;
                }
                else {
                    rmap[i] = pent->co.local.red >> 11;
                    gmap[i] = pent->co.local.green >> 11;
                    bmap[i] = pent->co.local.blue >> 11;
                }
            }
        }
        x68kUpdateColormap(cmap->pScreen, 0, 1<<(pPriv->depth), rmap, gmap, bmap);
    }
    pPriv->installedMap = cmap;
    WalkTree(cmap->pScreen, TellGainedMap, (pointer) &(cmap->mid));
}

/*-------------------------------------------------------------------------
 * function "x68kUninstallColormap"                    [ screen function ]
 *
 *  purpose:  uninstall colormap
 *  argument: (ColormapPtr)cmap : colormap to uninstall
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kUninstallColormap(ColormapPtr cmap)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(cmap->pScreen);

    if (cmap == pPriv->installedMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    pointer retval;
	    ColormapPtr defMap;
	    dixLookupResourceByType(&retval, defMapID, RT_COLORMAP,
		serverClient, DixReadAccess);
	    defMap = (ColormapPtr) retval;
	    (*cmap->pScreen->InstallColormap)(defMap);
	}
    }
}

/*-------------------------------------------------------------------------
 * function "x68kListInstalledColormaps"               [ screen function ]
 *
 *  purpose:  Get the list of currently installed colormaps.
 *            In X68k, installed colormap in always only one.
 *  argument: (ScreenPtr)pScreen    : screen
 *            (Colormap *)pCmapList : colormap list got
 *  returns:  (int)
 *-----------------------------------------------------------------------*/
static int
x68kListInstalledColormaps(ScreenPtr pScreen, Colormap *pCmapList)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(pScreen);
    *pCmapList = pPriv->installedMap->mid;
    return 1;
}

/*-------------------------------------------------------------------------
 * function "x68kStoreColors"                          [ screen function ]
 *
 *  purpose:  store specified color items
 *  argument: (ColormapPtr)pmap   : colormap
 *            (int)ndef           : the number of items
 *            (xColorItem *)pdefs : items
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(pmap->pScreen);
    u_char     rmap[256], gmap[256], bmap[256];
    xColorItem expanddefs[256];
    register int i;

    if (pPriv->installedMap != NULL && pPriv->installedMap != pmap)
	return;
    if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
	ndef = fbExpandDirectColors(pmap, ndef, pdefs, expanddefs);
	pdefs = expanddefs;
    }
    while (ndef--) {
	i = pdefs->pixel;
	rmap[i] = pdefs->red >> 11;
	gmap[i] = pdefs->green >> 11;
	bmap[i] = pdefs->blue >> 11;
	x68kUpdateColormap(pmap->pScreen, i, 1, rmap, gmap, bmap);
	pdefs++;
    }
}

/*-------------------------------------------------------------------------
 * function "x68kUpdateColormap"
 *
 *  purpose:  update hardware colormap
 *  argument: (ScreenPtr)pScreen: screen
 *            (int)dex          : colormap index
 *            (int)count        : count for updating
 *            (u_char *)[rgb]map: each map
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kUpdateColormap(ScreenPtr pScreen, int dex, int count,
                               u_char *rmap, u_char *gmap, u_char *bmap)
{
    X68kScreenRec *pPriv = x68kGetScreenPrivate(pScreen);
    volatile u_short *pal = pPriv->reg->gpal;

    for( ; count > 0; count--,dex++ ) {
        pal[dex] = (u_short)gmap[dex] << 11 |
                   (u_short)rmap[dex] << 6 |
                   (u_short)bmap[dex] << 1;
    }
}

/* EOF x68kGraph.c */
