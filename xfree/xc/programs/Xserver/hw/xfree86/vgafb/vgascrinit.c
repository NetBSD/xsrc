/* $XFree86: xc/programs/Xserver/hw/xfree86/vgafb/vgascrinit.c,v 1.2 1998/07/25 16:58:24 dawes Exp $ */
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
/* $XConsortium: vgascrinit.c /main/4 1996/10/25 06:21:10 kaleb $ */

#include "vgafb.h"
#include "mibstore.h"

/*
 * we need some global variables in this module; let's make them module
 * specific and figure out how to fill them in from the values in the
 * ScrnInfoRec
 */
void *vgaReadBottom = NULL;
void *vgaReadTop = NULL;
void *vgaWriteBottom = NULL;
void *vgaWriteTop = NULL;
Bool vgaReadFlag, vgaWriteFlag;
int vgaSegmentShift,vgaSegmentMask,vgaSegmentSize;
int vgaBase,vgaLinearBase;
Bool vgaUseLinearAddressing;
void (*vgaSetReadFunc)(int);
void (*vgaSetWriteFunc)(int);
void (*vgaSetReadWriteFunc)(int);
CfbfuncRec vga256LowlevFuncs;

/*
 * Most of this code is copied from cfbscrinit.c. It is necessary
 * to copy the code because of the call to miInitializeBackingStore()
 * which can't be called twice due to the wrapper mechanism used.
 *
 * This code should be kept in sync with Xserver/cfb/cfbscrinit.c.
 */

BSFuncRec vga256BSFuncRec = {
    vga256SaveAreas,
    vga256RestoreAreas,
    (BackingStoreSetClipmaskRgnProcPtr) 0,
    (BackingStoreGetImagePixmapProcPtr) 0,
    (BackingStoreGetSpansPixmapProcPtr) 0,
};

vga256FinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
#ifdef CFB_NEED_SCREEN_PRIVATE
    pointer oldDevPrivate;
#endif
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;
    /*
     * we need to figure out where to get this information from
     */
    Bool	vgaDAC8BitComponents = FALSE; 

    rootdepth = 0;
    if (!cfbInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
			 &defaultVisual,((unsigned long)1<<(PSZ-1)),
			 vgaDAC8BitComponents ? 8 : 6))
	return FALSE;
#ifdef CFB_NEED_SCREEN_PRIVATE
    oldDevPrivate = pScreen->devPrivate;
#endif
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;
    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = cfbCloseScreen;
    pScreen->BackingStoreFuncs = vga256BSFuncRec;
#ifdef CFB_NEED_SCREEN_PRIVATE
    pScreen->CreateScreenResources = cfbCreateScreenResources;
    pScreen->devPrivates[cfbScreenPrivateIndex].ptr = pScreen->devPrivate;
    pScreen->devPrivate = oldDevPrivate;
#endif
    pScreen->GetScreenPixmap = cfbGetScreenPixmap;
    pScreen->SetScreenPixmap = cfbSetScreenPixmap;
    return TRUE;
}

Bool
vga256ScreenInit(ScreenPtr pScreen, vgaHWPtr hwp, pointer pbits,
                 int xsize, int ysize, int dpix, int dpiy, int width)
{
    /*
     * first we init our globals from the information that was passed in
     */
    vgaReadBottom =   hwp->ReadBottom;
    vgaReadTop =      hwp->ReadTop;
    vgaWriteBottom =  hwp->WriteBottom;
    vgaWriteTop =     hwp->WriteTop;
    vgaSegmentShift = hwp->SegmentShift;
    vgaSegmentMask =  hwp->SegmentMask;
    vgaSegmentSize =  hwp->SegmentSize;
    vgaBase =         hwp->Base;
    vgaLinearBase =   hwp->LinearBase;
    vgaUseLinearAddressing = hwp->UseLinearAddressing;
    
    vgaSetReadFunc =  hwp->SetReadFunc;
    vgaSetWriteFunc = hwp->SetWriteFunc;
    vgaSetReadWriteFunc = hwp->SetReadWriteFunc;
    
    if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;

    /* These overrides what was being set in cfbSetupScreen() */

    pScreen->GetImage = vga256GetImage;
    pScreen->GetSpans = vga256GetSpans;
    pScreen->PaintWindowBackground = vga256PaintWindow;
    pScreen->PaintWindowBorder = vga256PaintWindow;
    pScreen->CopyWindow = vga256CopyWindow;
    pScreen->CreateGC = vga256CreateGC;

    mfbRegisterCopyPlaneProc (pScreen, vga256CopyPlane);

    return vga256FinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
}

