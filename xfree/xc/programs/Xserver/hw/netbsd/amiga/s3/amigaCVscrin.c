
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

/*
 * Modified for the CyberVision 64 by Michael Teske
 */

#include <stdio.h>
#include	"amiga.h"

#include	"Xmd.h"
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"mifillarc.h"
#include	"fontstruct.h"
#include	"dixfontstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#include	"mergerop.h"
#include	"amigaCV.h"
#include	"migc.h"
#include	"mi.h"
#include	<stdio.h>

Bool UseCVHardwareCursor = FALSE;
int amigaVirtualWidth = 0;
int amigaVirtualHeight = 0;
int amigaRealWidth;
int amigaRealHeight;

extern Bool amigaCVCursorInit();

extern void amigaCVSaveAreas();
extern void amigaCVRestoreAreas();

extern RegionPtr amigaCVCopyPlane();

extern int cfb16ScreenPrivateIndex, cfb32ScreenPrivateIndex;
extern Bool cfb16CreateScreenResources(), 
	    cfb32CreateScreenResources();

extern Bool cfbCloseScreen(), cfb16CloseScreen(), cfb32CloseScreen();
/*
miBSFuncRec amigaCVBSFuncRec = {
    amigaCVSaveAreas,
    amigaCVRestoreAreas,
    (void (*)()) 0,
    (PixmapPtr (*)()) 0,
    (PixmapPtr (*)()) 0,
};          
*/

extern  void CGScreenInit (ScreenPtr	pScreen);

/* extern miBSFuncRec cfbBSFuncRec; */

extern int amigaCVGXInit(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    fbFd* /* fb */
#endif
);

ScreenPtr amigaCVsavepScreen;

extern Bool s3RealizeFont(), s3UnrealizeFont();

static void checkMono (argc, argv)
    int argc;
    char** argv;
{
    int i;
      
    for (i = 1; i < argc; i++)
        if (strcmp (argv[i], "-mono") == 0)
            ErrorF ("-mono not supported on Cybervision64\n");
}



Bool amigaCVInit (screen, pScreen, argc, argv)
    int		screen;		/* The index of pScreen in the ScreenInfo */
    ScreenPtr	pScreen;	/* The Screen to initialize */
    int		argc;		/* The number of the Server's arguments. */
    char**	argv;		/* The arguments themselves. Don't change! */
{
    pointer	fb;

    VisualPtr visual, visuals;
    DepthPtr depths; 
    int nvisuals, ndepths;
    int rootdepth = 0;
    int bitsPerRGB;
    VisualID defaultVisual; 
    int i;
    pointer oldDevPrivate;

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

   amigaRealHeight = amigaFbs[screen].info.gd_fbheight; 
   amigaRealWidth = amigaFbs[screen].info.gd_fbwidth;

   if (amigaVirtualWidth) {
	int ok;
 
	/* do some checks first */
	switch (amigaVirtualWidth){
		case 640:
		case 800:
		case 1024:
		case 1152:
		case 1280:
		case 1600:
			ok = 1;
			break;
		default:
			ok = 0;
	}

	if (!ok) 
		FatalError("Illegal virtual width! Only 640 800 1024 1152 1280 1600 are allowed!\n");

	if ((amigaFbs[screen].info.gd_fbsize - 2048) < amigaVirtualWidth *
	    max(amigaVirtualHeight, amigaFbs[screen].info.gd_fbheight) *
	    amigaFbs[screen].info.gd_planes /8)
		FatalError("Virtual screen dimensions exceed Framebuffer size!\n");

	amigaFbs[screen].info.gd_fbwidth = amigaVirtualWidth;
   } else
	amigaVirtualWidth = amigaFbs[screen].info.gd_fbwidth;

   if (amigaVirtualHeight)
	amigaFbs[screen].info.gd_fbheight = amigaVirtualHeight;
   else
	amigaVirtualHeight = amigaFbs[screen].info.gd_fbheight;

    /* XXX I hate globals, but... */
    amigaCVsavepScreen = pScreen;

    amigaFbs[screen].EnterLeave = (void (*)())NoopDDA;
    if (!amigaCfbSetupScreen (pScreen, amigaFbs[screen].fb,
			      amigaFbs[screen].info.gd_fbwidth,
			      amigaFbs[screen].info.gd_fbheight,
			      monitorResolution, monitorResolution,
			      amigaFbs[screen].info.gd_fbwidth,
			      amigaFbs[screen].info.gd_planes))
	return FALSE;
    if (amigaNoGX == FALSE) {
	if (!amigaCVGXInit (pScreen, &amigaFbs[screen]))
	    return FALSE;
    }


    switch (amigaFbs[screen].info.gd_planes) {
    case 8:
	bitsPerRGB = 6;
    case 15:
	bitsPerRGB = 5;
    case 16:
	bitsPerRGB = 6;
	break;
    case 24:
    case 32:
	bitsPerRGB = 8;
	break;
    }

    if (amigaFbs[screen].info.gd_planes != 8) {
	if (!cfbSetVisualTypes (amigaFbs[screen].info.gd_planes,
				(1 << TrueColor),bitsPerRGB))
		return FALSE;
    }

    if (!cfbInitVisuals(&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
         &defaultVisual, 1<<(amigaFbs[screen].info.gd_planes - 1), 
         bitsPerRGB))
           return FALSE;       
#if 0
    if (!amigaCfbFinishScreenInit(pScreen, amigaFbs[screen].fb,
				  amigaFbs[screen].info.gd_fbwidth,
				  amigaFbs[screen].info.gd_fbheight,
				  monitorResolution, monitorResolution,
				  amigaFbs[screen].info.gd_fbwidth,
				  amigaFbs[screen].info.gd_planes))
	return FALSE;
#endif



    if (amigaFbs[screen].info.gd_planes != 8) {
	int rweight, bweight, gweight;

	switch (amigaFbs[screen].info.gd_planes) {
		case 15:
			rweight = gweight = bweight = 5;
			break;
		case 16:
			rweight = bweight = 5;
			gweight = 6;
			break;
		case 24:
		case 32:
			rweight = gweight = bweight = 8;
			break;
	}
#if 0
	visual = visuals = pScreen->visuals;
	nvisuals = pScreen->numVisuals;
#endif
	for (i = 0, visual = visuals; i < nvisuals; i++, visual++)
	    if (visual->class == DirectColor || visual->class == TrueColor) {
		switch (amigaFbs[screen].info.gd_planes) {
			case 15:
			case 16:
				visual->offsetRed = gweight + bweight;
				visual->offsetGreen = bweight;
				visual->offsetBlue = 0;
				visual->redMask =
				    ((1 << rweight) - 1) << visual->offsetRed;
				visual->greenMask =
				    ((1 << gweight) - 1) << visual->offsetGreen;
				visual->blueMask = (1 << bweight) - 1;
				break;
			case 24:
			case 32:
				visual->offsetRed = gweight + bweight;
				visual->offsetGreen = bweight;
				visual->offsetBlue = 0;
				visual->redMask =
				    ((1 << rweight) - 1) << visual->offsetRed;
				visual->greenMask =
				    ((1 << gweight) - 1) << visual->offsetGreen;
				visual->blueMask =
				    ((1 << bweight) - 1) << visual->offsetBlue;
				break;
		}
	    }
    }

    if (rootdepth != 8)
        oldDevPrivate = pScreen->devPrivate; 
    if (!miScreenInit(pScreen, amigaFbs[screen].fb,
                                  amigaFbs[screen].info.gd_fbwidth,   
                                  amigaFbs[screen].info.gd_fbheight,
                                  monitorResolution, monitorResolution,
                                  amigaFbs[screen].info.gd_fbwidth,
                                  rootdepth, ndepths, depths, 
				  defaultVisual, nvisuals, visuals))
/*
				  ,(miBSFuncPtr) 0))   
*/
	return (FALSE);

    if (rootdepth > 16) {
 	pScreen->CloseScreen = cfb32CloseScreen;
        pScreen->CreateScreenResources = cfb32CreateScreenResources;
        pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr = pScreen->devPrivate;
        pScreen->devPrivate = oldDevPrivate;
    }
    else if (rootdepth > 8) {
	pScreen->CloseScreen = cfb16CloseScreen;
        pScreen->CreateScreenResources = cfb16CreateScreenResources;
        pScreen->devPrivates[cfb16ScreenPrivateIndex].ptr = pScreen->devPrivate;
        pScreen->devPrivate = oldDevPrivate;
    }    
    else { 
	pScreen->CloseScreen = cfbCloseScreen;
    }
    /* init backing store here so we can overwrite CloseScreen without stepping
     * on the backing store wrapped version */  
/*    miInitializeBackingStore (pScreen, &amigaCVBSFuncRec); */
	miInitializeBackingStore(pScreen);


    mfbRegisterCopyPlaneProc (pScreen, amigaCVCopyPlane);

    CGScreenInit (pScreen);

    pScreen->RealizeFont = s3RealizeFont;
    pScreen->UnrealizeFont = s3UnrealizeFont;  


    if (amigaUseHWC)
	UseCVHardwareCursor = TRUE;
    else
	UseGfxHardwareCursor = TRUE;

    if (!amigaScreenInit (pScreen))
	return FALSE;
    UseCVHardwareCursor = FALSE;
    UseGfxHardwareCursor = FALSE;

    if (amigaUseHWC)
	amigaCVCursorInit(pScreen);


    amigaSaveScreen (pScreen, SCREEN_SAVER_OFF);

    return cfbCreateDefColormap(pScreen);
}

