/* $XFree86: xc/programs/Xserver/hw/xfree86/ramdac/xf86Cursor.c,v 1.7 2000/04/24 23:40:27 mvojkovi Exp $ */

#include "misc.h"
#include "xf86.h"
#include "xf86_ansic.h"
#include "xf86_OSproc.h"
#include "xf86str.h"

#include "X.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "xf86str.h"
#include "cursorstr.h"
#include "colormapst.h"
#include "mi.h"
#include "xf86Cursor.h"

int xf86CursorScreenIndex = -1;
static unsigned long xf86CursorGeneration = 0;

/* sprite functions */

static Bool xf86CursorRealizeCursor(ScreenPtr, CursorPtr);
static Bool xf86CursorUnrealizeCursor(ScreenPtr, CursorPtr);
static void xf86CursorSetCursor(ScreenPtr, CursorPtr, int, int);
static void xf86CursorMoveCursor(ScreenPtr, int, int);

static miPointerSpriteFuncRec xf86CursorSpriteFuncs = {
   xf86CursorRealizeCursor,
   xf86CursorUnrealizeCursor,
   xf86CursorSetCursor,
   xf86CursorMoveCursor
};

/* Screen functions */

static void xf86CursorInstallColormap(ColormapPtr);
static void xf86CursorRecolorCursor(ScreenPtr, CursorPtr, Bool);
static Bool xf86CursorCloseScreen(int, ScreenPtr);
static void xf86CursorQueryBestSize(int, unsigned short*, unsigned short*, 
				ScreenPtr);

/* ScrnInfoRec functions */

static Bool xf86CursorSwitchMode(int, DisplayModePtr,int);
static Bool xf86CursorEnterVT(int, int);
static void xf86CursorLeaveVT(int, int);
static int  xf86SetDGAMode(int, int, DGADevicePtr);


Bool 
xf86InitCursor(
   ScreenPtr pScreen, 
   xf86CursorInfoPtr infoPtr
)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    xf86CursorScreenPtr ScreenPriv;
    miPointerScreenPtr PointPriv;
 
    if(xf86CursorGeneration != serverGeneration) {
	if((xf86CursorScreenIndex = AllocateScreenPrivateIndex()) < 0)
		return FALSE;
	xf86CursorGeneration = serverGeneration; 	
    }

    if(!xf86InitHardwareCursor(pScreen, infoPtr))
	return FALSE;

    ScreenPriv = xcalloc(1, sizeof(xf86CursorScreenRec));
    if(!ScreenPriv) return FALSE;

    pScreen->devPrivates[xf86CursorScreenIndex].ptr = (pointer)ScreenPriv;

    ScreenPriv->SWCursor = TRUE;
    ScreenPriv->isUp = FALSE;
    ScreenPriv->CurrentCursor = NULL;
    ScreenPriv->CursorInfoPtr = infoPtr;
    ScreenPriv->PalettedCursor = FALSE;
    ScreenPriv->pInstalledMap = NULL;

    ScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = xf86CursorCloseScreen;
    ScreenPriv->QueryBestSize = pScreen->QueryBestSize;
    pScreen->QueryBestSize = xf86CursorQueryBestSize;
    ScreenPriv->RecolorCursor = pScreen->RecolorCursor;
    pScreen->RecolorCursor = xf86CursorRecolorCursor;

    if((infoPtr->pScrn->bitsPerPixel == 8) &&
    		!(infoPtr->Flags & HARDWARE_CURSOR_TRUECOLOR_AT_8BPP)) {
	ScreenPriv->InstallColormap = pScreen->InstallColormap;
	pScreen->InstallColormap = xf86CursorInstallColormap;
	ScreenPriv->PalettedCursor = TRUE;
    }

    PointPriv = 
	(miPointerScreenPtr)pScreen->devPrivates[miPointerScreenIndex].ptr;

    ScreenPriv->spriteFuncs = PointPriv->spriteFuncs;
    PointPriv->spriteFuncs = &xf86CursorSpriteFuncs; 

    ScreenPriv->SwitchMode = pScrn->SwitchMode;
    ScreenPriv->EnterVT = pScrn->EnterVT;
    ScreenPriv->LeaveVT = pScrn->LeaveVT;
    ScreenPriv->SetDGAMode = pScrn->SetDGAMode;

    if(pScrn->SwitchMode)
	pScrn->SwitchMode = xf86CursorSwitchMode;
    pScrn->EnterVT = xf86CursorEnterVT; 
    pScrn->LeaveVT = xf86CursorLeaveVT;
    pScrn->SetDGAMode = xf86SetDGAMode;

    return TRUE;
}

/***** Screen functions *****/

static Bool
xf86CursorCloseScreen(int i, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    miPointerScreenPtr PointPriv = 
	(miPointerScreenPtr)pScreen->devPrivates[miPointerScreenIndex].ptr;
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    pScreen->CloseScreen = ScreenPriv->CloseScreen;
    pScreen->QueryBestSize = ScreenPriv->QueryBestSize;
    pScreen->RecolorCursor = ScreenPriv->RecolorCursor;
    if(ScreenPriv->InstallColormap)
	pScreen->InstallColormap = ScreenPriv->InstallColormap;

    PointPriv->spriteFuncs = ScreenPriv->spriteFuncs;

    pScrn->SwitchMode = ScreenPriv->SwitchMode;
    pScrn->EnterVT = ScreenPriv->EnterVT; 
    pScrn->LeaveVT = ScreenPriv->LeaveVT; 
    pScrn->SetDGAMode = ScreenPriv->SetDGAMode;

    xfree ((pointer) ScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

static void
xf86CursorQueryBestSize(
   int class, 
   unsigned short *width,
   unsigned short *height,
   ScreenPtr pScreen
){
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    if(class == CursorShape) {
	*width = ScreenPriv->CursorInfoPtr->MaxWidth;
	*height = ScreenPriv->CursorInfoPtr->MaxHeight;
    } else
	(*ScreenPriv->QueryBestSize)(class, width, height, pScreen);
}
   
static void 
xf86CursorInstallColormap(ColormapPtr pMap)
{
    xf86CursorScreenPtr ScreenPriv = 	
     (xf86CursorScreenPtr)pMap->pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    ScreenPriv->pInstalledMap = pMap;
    
    (*ScreenPriv->InstallColormap)(pMap);
}


static void 
xf86CursorRecolorCursor(
    ScreenPtr pScreen, 
    CursorPtr pCurs, 
    Bool displayed )
{
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    if(!displayed) return;

    if(ScreenPriv->SWCursor)
	(*ScreenPriv->RecolorCursor)(pScreen, pCurs, displayed);
    else 
	xf86RecolorCursor(pScreen, pCurs, displayed);
}

/***** ScrnInfoRec functions *********/

static Bool 
xf86CursorSwitchMode(int index, DisplayModePtr mode, int flags)
{
    Bool ret;
    ScreenPtr pScreen = screenInfo.screens[index];
    xf86CursorScreenPtr ScreenPriv = 
     (xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    if(ScreenPriv->isUp) {
	xf86SetCursor(pScreen, 0, ScreenPriv->x, ScreenPriv->y);
	ScreenPriv->isUp = FALSE;
    }

    ret = (*ScreenPriv->SwitchMode)(index, mode, flags);

    if(ScreenPriv->CurrentCursor)
	xf86CursorSetCursor(pScreen, ScreenPriv->CurrentCursor, 
					ScreenPriv->x, ScreenPriv->y);

    return ret;
}


static Bool 
xf86CursorEnterVT(int index, int flags)
{
    Bool ret;
    ScreenPtr pScreen = screenInfo.screens[index];
    xf86CursorScreenPtr ScreenPriv = 
     (xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    ret = (*ScreenPriv->EnterVT)(index, flags);

    if(ScreenPriv->CurrentCursor)
	xf86CursorSetCursor(pScreen, ScreenPriv->CurrentCursor, 
			ScreenPriv->x, ScreenPriv->y);
    return ret;
}

static void 
xf86CursorLeaveVT(int index, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    xf86CursorScreenPtr ScreenPriv = 
     (xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    if(ScreenPriv->isUp) {   
	xf86SetCursor(pScreen, 0, ScreenPriv->x, ScreenPriv->y);
	ScreenPriv->isUp = FALSE;
    }
    ScreenPriv->SWCursor = TRUE;

    (*ScreenPriv->LeaveVT)(index, flags);
}


static int  
xf86SetDGAMode(int index, int num, DGADevicePtr devRet)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    xf86CursorScreenPtr ScreenPriv = 
     (xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;
    int ret;

    if(num && ScreenPriv->isUp) {
	xf86SetCursor(pScreen, 0, ScreenPriv->x, ScreenPriv->y);
	ScreenPriv->isUp = FALSE;
	ScreenPriv->SWCursor = TRUE;
    }

    ret = (*ScreenPriv->SetDGAMode)(index, num, devRet);

    if(ScreenPriv->CurrentCursor && (!num || (ret != Success))) {
	xf86CursorSetCursor(pScreen, ScreenPriv->CurrentCursor, 
			ScreenPriv->x, ScreenPriv->y);
    }

    return ret;
}

    
/****** miPointerSpriteFunctions *******/


static Bool
xf86CursorRealizeCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    if(pCurs->refcnt <= 1)
	pCurs->devPriv[pScreen->myNum] = NULL;

    return((*ScreenPriv->spriteFuncs->RealizeCursor)(pScreen, pCurs));
}

static Bool
xf86CursorUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;
    
    if(pCurs->refcnt <= 1) {
	pointer privData = pCurs->devPriv[pScreen->myNum];
	if(privData) xfree(privData);
	pCurs->devPriv[pScreen->myNum] = NULL;
    }

    return((*ScreenPriv->spriteFuncs->UnrealizeCursor)(pScreen, pCurs));
}


static void
xf86CursorSetCursor(ScreenPtr pScreen, CursorPtr pCurs, int x, int y)
{
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;
    xf86CursorInfoPtr infoPtr = ScreenPriv->CursorInfoPtr;
    miPointerScreenPtr PointPriv;

    ScreenPriv->CurrentCursor = pCurs;
    ScreenPriv->x = x;
    ScreenPriv->y = y;

    if(!pCurs) {  /* means we're supposed to remove the cursor */
	if(ScreenPriv->SWCursor)
	    (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, 0, x, y);
	else {
	    if(ScreenPriv->isUp) xf86SetCursor(pScreen, 0, x, y);
	    ScreenPriv->isUp = FALSE;
	}
	return;
    } 

    ScreenPriv->HotX = pCurs->bits->xhot;
    ScreenPriv->HotY = pCurs->bits->yhot;
	
    PointPriv = 
	(miPointerScreenPtr)pScreen->devPrivates[miPointerScreenIndex].ptr;
    
    if( infoPtr->pScrn->vtSema &&
	(pCurs->bits->height <= infoPtr->MaxHeight) &&
	(pCurs->bits->width <= infoPtr->MaxWidth) &&
	(!infoPtr->UseHWCursor || (*infoPtr->UseHWCursor)(pScreen, pCurs))){

	if(ScreenPriv->SWCursor) /* remove the SW cursor */
	      (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, 0, x, y);

	xf86SetCursor(pScreen, pCurs, x, y);
	ScreenPriv->SWCursor = FALSE;
	ScreenPriv->isUp = TRUE;
	PointPriv->waitForUpdate = !infoPtr->pScrn->silkenMouse;
	return; 
    }

    PointPriv->waitForUpdate = TRUE;
    
    if(ScreenPriv->isUp) {
	/* remove the HW cursor */
	xf86SetCursor(pScreen, 0, x, y);
	ScreenPriv->isUp = FALSE;
    }

    ScreenPriv->SWCursor = TRUE;

    (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, pCurs, x, y);
}

static void
xf86CursorMoveCursor(ScreenPtr pScreen, int x, int y)
{
    xf86CursorScreenPtr ScreenPriv = 
	(xf86CursorScreenPtr)pScreen->devPrivates[xf86CursorScreenIndex].ptr;

    ScreenPriv->x = x;
    ScreenPriv->y = y;

    if(ScreenPriv->SWCursor)
	(*ScreenPriv->spriteFuncs->MoveCursor)(pScreen, x, y);
    else if(ScreenPriv->isUp)	
	xf86MoveCursor(pScreen, x, y); 
}



xf86CursorInfoPtr 
xf86CreateCursorInfoRec(void)
{
    return(xcalloc(1,sizeof(xf86CursorInfoRec)));
}


void 
xf86DestroyCursorInfoRec(xf86CursorInfoPtr infoPtr)
{
    if(infoPtr) xfree(infoPtr);
}








