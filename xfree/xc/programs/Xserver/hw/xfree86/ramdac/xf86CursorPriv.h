/* $XFree86: xc/programs/Xserver/hw/xfree86/ramdac/xf86CursorPriv.h,v 1.2 2001/05/18 20:22:31 tsi Exp $ */

#ifndef _XF86CURSORPRIV_H
#define _XF86CURSORPRIV_H

#include "xf86Cursor.h"
#include "mipointrst.h"

typedef struct {
    Bool			SWCursor;
    Bool			isUp;
    Bool			showTransparent;
    short			HotX;
    short			HotY;
    short			x;
    short			y;
    CursorPtr			CurrentCursor, CursorToRestore;
    xf86CursorInfoPtr		CursorInfoPtr;
    CloseScreenProcPtr          CloseScreen;
    RecolorCursorProcPtr	RecolorCursor;
    InstallColormapProcPtr	InstallColormap;
    QueryBestSizeProcPtr	QueryBestSize;
    miPointerSpriteFuncPtr	spriteFuncs;
    Bool			PalettedCursor;
    ColormapPtr			pInstalledMap;
    Bool                	(*SwitchMode)(int, DisplayModePtr,int);
    Bool                	(*EnterVT)(int, int);
    void                	(*LeaveVT)(int, int);
    int				(*SetDGAMode)(int, int, DGADevicePtr);
} xf86CursorScreenRec, *xf86CursorScreenPtr;

CARD32 xf86ReverseBitOrder(CARD32 data);

#endif /* _XF86CURSORPRIV_H */
