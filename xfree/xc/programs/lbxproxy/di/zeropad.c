/* $Xorg: zeropad.c,v 1.4 2001/02/09 02:05:32 xorgcvs Exp $ */

/*

Copyright 1996, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/lbxproxy/di/zeropad.c,v 1.3 2004/04/03 22:38:54 tsi Exp $ */

/*
 * This module handles zeroing out unused pad bytes in X requests.
 * This will hopefully improve both stream and delta compression,
 * since we are removing the random values in pad bytes.
 */

#include <X11/Xproto.h>
#include "zeropad.h"

static void
ZeroEmptyReq (
    xReq *req)
{
    req->data = 0;
}

#define ZeroGetInputFocusReq ZeroEmptyReq
#define ZeroGetFontPathReq ZeroEmptyReq
#define ZeroGetKeyboardControlReq ZeroEmptyReq
#define ZeroGetPointerControlReq ZeroEmptyReq
#define ZeroGetPointerMappingReq ZeroEmptyReq
#define ZeroGetScreenSaverReq ZeroEmptyReq
#define ZeroGrabServerReq ZeroEmptyReq
#define ZeroListExtensionsReq ZeroEmptyReq
#define ZeroNoOperationReq ZeroEmptyReq
#define ZeroGetModifierMappingReq ZeroEmptyReq
#define ZeroQueryKeymapReq ZeroEmptyReq
#define ZeroUngrabServerReq ZeroEmptyReq


static void
ZeroResourceReq (
    xResourceReq *req)
{
    req->pad = 0;
}

#define ZeroFreePixmapReq ZeroResourceReq
#define ZeroGetAtomNameReq ZeroResourceReq
#define ZeroDestroySubwindowsReq ZeroResourceReq
#define ZeroDestroyWindowReq ZeroResourceReq
#define ZeroCloseFontReq ZeroResourceReq
#define ZeroQueryFontReq ZeroResourceReq
#define ZeroFreeCursorReq ZeroResourceReq
#define ZeroFreeGCReq ZeroResourceReq
#define ZeroGetGeometryReq ZeroResourceReq
#define ZeroGetSelectionOwnerReq ZeroResourceReq
#define ZeroGetWindowAttributesReq ZeroResourceReq
#define ZeroInstallColormapReq ZeroResourceReq
#define ZeroListInstalledColormapsReq ZeroResourceReq
#define ZeroListPropertiesReq ZeroResourceReq
#define ZeroMapSubwindowsReq ZeroResourceReq
#define ZeroMapWindowReq ZeroResourceReq
#define ZeroQueryPointerReq ZeroResourceReq
#define ZeroQueryTreeReq ZeroResourceReq
#define ZeroUngrabKeyboardReq ZeroResourceReq
#define ZeroUngrabPointerReq ZeroResourceReq
#define ZeroUninstallColormapReq ZeroResourceReq
#define ZeroUnmapSubwindowsReq ZeroResourceReq
#define ZeroUnmapWindowReq ZeroResourceReq
#define ZeroKillClientReq ZeroResourceReq
#define ZeroFreeColormapReq ZeroResourceReq


static void
ZeroChangeWindowAttributesReq (
    xChangeWindowAttributesReq *req)
{
    req->pad = 0;
}

static void
ZeroReparentWindowReq (
    xReparentWindowReq *req)
{
    req->pad = 0;
}

static void
ZeroConfigureWindowReq (
    xConfigureWindowReq *req)
{
    req->pad = 0;
    req->pad2 = 0;
}

static void
ZeroInternAtomReq (
    xInternAtomReq *req)
{
    req->pad = 0;
}

static void
ZeroChangePropertyReq (
    xChangePropertyReq *req)
{
    req->pad[0] = 0;
    req->pad[1] = 0;
    req->pad[2] = 0;
}

static void
ZeroDeletePropertyReq (
    xDeletePropertyReq *req)
{
    req->pad = 0;
}

static void
ZeroSetSelectionOwnerReq (
    xSetSelectionOwnerReq *req)
{
    req->pad = 0;
}

static void
ZeroConvertSelectionReq (
    xConvertSelectionReq *req)
{
    req->pad = 0;
}

static void
ZeroGrabButtonReq (
    xGrabButtonReq *req)
{
    req->pad = 0;
}

static void
ZeroUngrabButtonReq (
    xUngrabButtonReq *req)
{
    req->pad = 0;
}

static void
ZeroChangeActivePointerGrabReq (
    xChangeActivePointerGrabReq *req)
{
    req->pad = 0;
    req->pad2 = 0;
}

static void
ZeroGrabKeyboardReq (
    xGrabKeyboardReq *req)
{
    req->pad = 0;
}

static void
ZeroGrabKeyReq (
    xGrabKeyReq *req)
{
    req->pad1 = 0;
    req->pad2 = 0;
    req->pad3  = 0;
}

static void
ZeroUngrabKeyReq (
    xUngrabKeyReq *req)
{
    req->pad = 0;
}

static void
ZeroGetMotionEventsReq (
    xGetMotionEventsReq *req)
{
    req->pad = 0;
}

static void
ZeroTranslateCoordsReq (
    xTranslateCoordsReq *req)
{
    req->pad = 0;
}

static void
ZeroWarpPointerReq (
    xWarpPointerReq *req)
{
    req->pad = 0;
}

static void
ZeroOpenFontReq (
    xOpenFontReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
    req->pad2 = 0;
}

static void
ZeroListFontsReq (
    xListFontsReq *req)
{
    req->pad = 0;
}

#define ZeroListFontsWithInfoReq ZeroListFontsReq

static void
ZeroSetFontPathReq (
    xSetFontPathReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
    req->pad2 = 0;
}

static void
ZeroCreateGCReq (
    xCreateGCReq *req)
{
    req->pad = 0;
}

static void
ZeroChangeGCReq (
    xChangeGCReq *req)
{
    req->pad = 0;
}    

static void
ZeroCopyGCReq (
    xCopyGCReq *req)
{
    req->pad = 0;
}    

static void
ZeroSetDashesReq (
    xSetDashesReq *req)
{
    req->pad = 0;
}    

static void
ZeroCopyAreaReq (
    xCopyAreaReq *req)
{
    req->pad = 0;
}    

static void
ZeroCopyPlaneReq (
    xCopyPlaneReq *req)
{
    req->pad = 0;
}    

static void
ZeroPolySegmentReq (
    xPolySegmentReq *req)
{
    req->pad = 0;
}    

#define ZeroPolyArcReq ZeroPolySegmentReq
#define ZeroPolyRectangleReq ZeroPolySegmentReq
#define ZeroPolyFillRectangleReq ZeroPolySegmentReq
#define ZeroPolyFillArcReq ZeroPolySegmentReq

static void
ZeroFillPolyReq (
    xFillPolyReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
}    

static void
ZeroPutImageReq (
    xPutImageReq *req)
{
    req->pad = 0;
}    

static void
ZeroPolyTextReq (
    xPolyTextReq *req)
{
    req->pad = 0;
}    

#define ZeroPolyText8Req ZeroPolyTextReq
#define ZeroPolyText16Req ZeroPolyTextReq

static void
ZeroCopyColormapAndFreeReq (
    xCopyColormapAndFreeReq *req)
{
    req->pad = 0;
}    

static void
ZeroAllocColorReq (
    xAllocColorReq *req)
{
    req->pad = 0;
    req->pad2 = 0;
}    

static void
ZeroAllocNamedColorReq (
    xAllocNamedColorReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
    req->pad2 = 0;
}    

static void
ZeroFreeColorsReq (
    xFreeColorsReq *req)
{
    req->pad = 0;
}    

static void
ZeroStoreColorsReq (
    xStoreColorsReq *req)
{
    req->pad = 0;
}    

static void
ZeroStoreNamedColorReq (
    xStoreNamedColorReq *req)
{
    req->pad1 = 0;
    req->pad2 = 0;
}

static void
ZeroQueryColorsReq (
    xQueryColorsReq *req)
{
    req->pad = 0;
}    

static void
ZeroLookupColorReq (
    xLookupColorReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
    req->pad2 = 0;
}    

static void
ZeroCreateCursorReq (
    xCreateCursorReq *req)
{
    req->pad = 0;
}    

static void
ZeroCreateGlyphCursorReq (
    xCreateGlyphCursorReq *req)
{
    req->pad = 0;
}    

static void
ZeroRecolorCursorReq (
    xRecolorCursorReq *req)
{
    req->pad = 0;
}    

static void
ZeroQueryExtensionReq (
    xQueryExtensionReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
    req->pad2 = 0;
}

static void
ZeroGetKeyboardMappingReq (
    xGetKeyboardMappingReq *req)
{
    req->pad = 0;
    req->pad1 = 0;
}    

static void
ZeroChangeKeyboardMappingReq (
    xChangeKeyboardMappingReq *req)
{
    req->pad1 = 0;
}

static void
ZeroChangeKeyboardControlReq (
    xChangeKeyboardControlReq *req)
{
    req->pad = 0;
}    

static void
ZeroChangePointerControlReq (
    xChangePointerControlReq *req)
{
    req->pad = 0;
}    

static void
ZeroSetScreenSaverReq (
    xSetScreenSaverReq *req)
{
    req->pad = 0;
    req->pad2 = 0;
}    

static void
ZeroChangeHostsReq (
    xChangeHostsReq *req)
{
    req->pad = 0;
}    

static void
ZeroListHostsReq (
    xListHostsReq *req)
{
    req->pad = 0;
}

static void
ZeroRotatePropertiesReq (
    xRotatePropertiesReq *req)
{
    req->pad = 0;
}

ZPREQ_T ZeroPadReqVector[128] =
{
    (ZPREQ_T)0,
    (ZPREQ_T)0, /* CreateWindowReq */
    (ZPREQ_T)ZeroChangeWindowAttributesReq,
    (ZPREQ_T)ZeroGetWindowAttributesReq,
    (ZPREQ_T)ZeroDestroyWindowReq,
    (ZPREQ_T)ZeroDestroySubwindowsReq,		/* 5 */
    (ZPREQ_T)0, /* ChangeSaveSetReq */
    (ZPREQ_T)ZeroReparentWindowReq,
    (ZPREQ_T)ZeroMapWindowReq,
    (ZPREQ_T)ZeroMapSubwindowsReq,
    (ZPREQ_T)ZeroUnmapWindowReq,		/* 10 */
    (ZPREQ_T)ZeroUnmapSubwindowsReq,
    (ZPREQ_T)ZeroConfigureWindowReq,
    (ZPREQ_T)0, /* CirculateWindowReq */
    (ZPREQ_T)ZeroGetGeometryReq,
    (ZPREQ_T)ZeroQueryTreeReq,			/* 15 */
    (ZPREQ_T)ZeroInternAtomReq,
    (ZPREQ_T)ZeroGetAtomNameReq,
    (ZPREQ_T)ZeroChangePropertyReq,
    (ZPREQ_T)ZeroDeletePropertyReq,
    (ZPREQ_T)0, /* GetPropertyReq */		/* 20 */
    (ZPREQ_T)ZeroListPropertiesReq,
    (ZPREQ_T)ZeroSetSelectionOwnerReq,
    (ZPREQ_T)ZeroGetSelectionOwnerReq,
    (ZPREQ_T)ZeroConvertSelectionReq,
    (ZPREQ_T)0, /* SendEventReq */		/* 25 */
    (ZPREQ_T)0, /* GrabPointerReq */
    (ZPREQ_T)ZeroUngrabPointerReq,
    (ZPREQ_T)ZeroGrabButtonReq,
    (ZPREQ_T)ZeroUngrabButtonReq,
    (ZPREQ_T)ZeroChangeActivePointerGrabReq,	/* 30 */
    (ZPREQ_T)ZeroGrabKeyboardReq,
    (ZPREQ_T)ZeroUngrabKeyboardReq,
    (ZPREQ_T)ZeroGrabKeyReq,
    (ZPREQ_T)ZeroUngrabKeyReq,
    (ZPREQ_T)0, /* AllowEventsReq */		/* 35 */
    (ZPREQ_T)ZeroGrabServerReq,
    (ZPREQ_T)ZeroUngrabServerReq,
    (ZPREQ_T)ZeroQueryPointerReq,
    (ZPREQ_T)ZeroGetMotionEventsReq,
    (ZPREQ_T)ZeroTranslateCoordsReq,		/* 40 */
    (ZPREQ_T)ZeroWarpPointerReq,
    (ZPREQ_T)0, /* SetInputFocusReq */
    (ZPREQ_T)ZeroGetInputFocusReq,
    (ZPREQ_T)ZeroQueryKeymapReq,
    (ZPREQ_T)ZeroOpenFontReq,			/* 45 */
    (ZPREQ_T)ZeroCloseFontReq,
    (ZPREQ_T)ZeroQueryFontReq,
    (ZPREQ_T)0, /* QueryTextExtentsReq */
    (ZPREQ_T)ZeroListFontsReq,
    (ZPREQ_T)ZeroListFontsWithInfoReq,		/* 50 */
    (ZPREQ_T)ZeroSetFontPathReq,
    (ZPREQ_T)ZeroGetFontPathReq,
    (ZPREQ_T)0, /* CreatePixmapReq */
    (ZPREQ_T)ZeroFreePixmapReq,
    (ZPREQ_T)ZeroCreateGCReq,			/* 55 */
    (ZPREQ_T)ZeroChangeGCReq,
    (ZPREQ_T)ZeroCopyGCReq,
    (ZPREQ_T)ZeroSetDashesReq,
    (ZPREQ_T)0, /* SetClipRectanglesReq */
    (ZPREQ_T)ZeroFreeGCReq,			/* 60 */
    (ZPREQ_T)0, /* ClearToBackgroundReq */
    (ZPREQ_T)ZeroCopyAreaReq,
    (ZPREQ_T)ZeroCopyPlaneReq,
    (ZPREQ_T)0, /* PolyPointReq */
    (ZPREQ_T)0, /* PolyLineReq */		/* 65 */
    (ZPREQ_T)ZeroPolySegmentReq,
    (ZPREQ_T)ZeroPolyRectangleReq,
    (ZPREQ_T)ZeroPolyArcReq,
    (ZPREQ_T)ZeroFillPolyReq,
    (ZPREQ_T)ZeroPolyFillRectangleReq,		/* 70 */
    (ZPREQ_T)ZeroPolyFillArcReq,
    (ZPREQ_T)ZeroPutImageReq,
    (ZPREQ_T)0, /* GetImageReq */
    (ZPREQ_T)ZeroPolyText8Req,
    (ZPREQ_T)ZeroPolyText16Req,			/* 75 */
    (ZPREQ_T)0, /* ImageText8Req */
    (ZPREQ_T)0, /* ImageText16Req */
    (ZPREQ_T)0, /* CreateColormapReq */
    (ZPREQ_T)ZeroFreeColormapReq,
    (ZPREQ_T)ZeroCopyColormapAndFreeReq,	/* 80 */
    (ZPREQ_T)ZeroInstallColormapReq,
    (ZPREQ_T)ZeroUninstallColormapReq,
    (ZPREQ_T)ZeroListInstalledColormapsReq,
    (ZPREQ_T)ZeroAllocColorReq,
    (ZPREQ_T)ZeroAllocNamedColorReq,		/* 85 */
    (ZPREQ_T)0, /* AllocColorCellsReq */
    (ZPREQ_T)0, /* AllocColorPlanesReq */
    (ZPREQ_T)ZeroFreeColorsReq,
    (ZPREQ_T)ZeroStoreColorsReq,
    (ZPREQ_T)ZeroStoreNamedColorReq,		/* 90 */
    (ZPREQ_T)ZeroQueryColorsReq,
    (ZPREQ_T)ZeroLookupColorReq,
    (ZPREQ_T)ZeroCreateCursorReq,
    (ZPREQ_T)ZeroCreateGlyphCursorReq,
    (ZPREQ_T)ZeroFreeCursorReq,			/* 95 */
    (ZPREQ_T)ZeroRecolorCursorReq,
    (ZPREQ_T)0, /* QueryBestSizeReq */
    (ZPREQ_T)ZeroQueryExtensionReq,
    (ZPREQ_T)ZeroListExtensionsReq,
    (ZPREQ_T)ZeroChangeKeyboardMappingReq,	/* 100 */
    (ZPREQ_T)ZeroGetKeyboardMappingReq,
    (ZPREQ_T)ZeroChangeKeyboardControlReq,
    (ZPREQ_T)ZeroGetKeyboardControlReq,
    (ZPREQ_T)0, /* BellReq */
    (ZPREQ_T)ZeroChangePointerControlReq,	/* 105 */
    (ZPREQ_T)ZeroGetPointerControlReq,
    (ZPREQ_T)ZeroSetScreenSaverReq,
    (ZPREQ_T)ZeroGetScreenSaverReq,
    (ZPREQ_T)ZeroChangeHostsReq,
    (ZPREQ_T)ZeroListHostsReq,			/* 110 */
    (ZPREQ_T)0, /* ChangeAccessControlReq */
    (ZPREQ_T)0, /* ChangeCloseDownModeReq */
    (ZPREQ_T)ZeroKillClientReq,
    (ZPREQ_T)ZeroRotatePropertiesReq,
    (ZPREQ_T)0, /* ForceScreenSaverReq */	/* 115 */
    (ZPREQ_T)0, /* SetPointerMappingReq */
    (ZPREQ_T)ZeroGetPointerMappingReq,
    (ZPREQ_T)0, /* SetModifierMappingReq */
    (ZPREQ_T)ZeroGetModifierMappingReq,
    (ZPREQ_T)0,					/* 120 */
    (ZPREQ_T)0,
    (ZPREQ_T)0,
    (ZPREQ_T)0,
    (ZPREQ_T)0,
    (ZPREQ_T)0,					/* 125 */
    (ZPREQ_T)0,
    (ZPREQ_T)ZeroNoOperationReq
};
