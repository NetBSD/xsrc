/*
 *Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 *
 *Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 *"Software"), to deal in the Software without restriction, including
 *without limitation the rights to use, copy, modify, merge, publish,
 *distribute, sublicense, and/or sell copies of the Software, and to
 *permit persons to whom the Software is furnished to do so, subject to
 *the following conditions:
 *
 *The above copyright notice and this permission notice shall be
 *included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of the XFree86 Project
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from the XFree86 Project.
 *
 * Authors:	Dakshinamurthy Karra
 *		Suhaib M Siddiqi
 *		Peter Busch
 *		Harold L Hunt II
 */
/* $XFree86: xc/programs/Xserver/hw/xwin/win.h,v 1.6 2001/05/08 08:14:09 alanh Exp $ */

#ifndef _WIN_H_
#define _WIN_H_

#ifndef NO
#define NO			0
#endif
#ifndef YES
#define YES			1
#endif

/* Turn debug messages on or off */
#define CYGDEBUG		NO

/* Debugging macros */
#if CYGDEBUG
#define DEBUG_MSG(str) if (fDebugProcMsg == TRUE) MessageBox(NULL, str, szFunctionName, MB_OK )
#else
#define DEBUG_MSG(str)
#endif

#if CYGDEBUG
#define DEBUG_FN_NAME(str) PTSTR szFunctionName = str
#else
#define DEBUG_FN_NAME(str)
#endif

#if CYGDEBUG
#define DEBUGVARS BOOL fDebugProcCon = FALSE, fDebugProcMsg = FALSE
#else
#define DEBUGVARS
#endif

#if CYGDEBUG
#define DEBUGPROC_CON fDebugProcCon = TRUE
#define DEBUGPROC_MSG fDebugProcMsg = TRUE
#else
#define DEBUGPROC_CON
#define DEBUGPROC_MSG
#endif

/* Constant strings */
#define WINDOW_CLASS		"cygwin/xfree86"
#define WINDOW_TITLE		"Cygwin/XFree86"
#define WIN_SCR_PROP		"cyg_screen_prop"
#define WIN_MSG_QUEUE_FNAME	"/dev/windows"

#define NEED_EVENTS

#define WIN_DEFAULT_WIDTH	640
#define WIN_DEFAULT_HEIGHT	480
#define WIN_DEFAULT_DEPTH	16
#define WIN_DEFAULT_WHITEPIXEL	255
#define WIN_DEFAULT_BLACKPIXEL	0
#define WIN_DEFAULT_LINEBIAS	0
#define WIN_DEFAULT_E3B_TIME	50 /* milliseconds */

#define WIN_E3B_OFF		-1
#define WIN_E3B_TIMER_ID	1
#define WIN_FD_INVALID		-1

#define WIN_SERVER_NONE		0x0L	/* 0 */
#define WIN_SERVER_SHADOW_GDI	0x1L	/* 1 */
#define WIN_SERVER_SHADOW_DD	0x2L	/* 2 */
#define WIN_SERVER_SHADOW_DDNL	0x4L	/* 4 */
#define WIN_SERVER_PRIMARY_DD	0x8L	/* 8 */

#define AltMapIndex		Mod1MapIndex
#define NumLockMapIndex		Mod2MapIndex
#define AltLangMapIndex		Mod3MapIndex
#define KanaMapIndex		Mod4MapIndex
#define ScrollLockMapIndex	Mod5MapIndex

#define WIN_24BPP_MASK_RED	0x00FF0000
#define WIN_24BPP_MASK_GREEN	0x0000FF00
#define WIN_24BPP_MASK_BLUE	0x000000FF

/*
 * We need symbols for the scan codes of keys.
 */
#include "../xfree86/common/atKeynames.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#include <X11/XWDFile.h>

#ifdef HAS_MMAP
#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE 0
#endif /* MAP_FILE */
#endif /* HAS_MMAP */

#include "X.h"
#include "Xproto.h"
#include "Xos.h"
#include "Xprotostr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "pixmap.h"
#include "region.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "colormap.h"
#include "colormapst.h"
#include "miscstruct.h"
#include "servermd.h"
#include "windowstr.h"
#include "mi.h"
#include "micmap.h"
#include "migc.h"
#include "mifillarc.h"
#include "mifpoly.h"
#include "mibstore.h"
#include "input.h"
#include "mipointer.h"
#include "keysym.h"
#include "mibstore.h"
#include "micoord.h"
#include "dix.h"
#include "miline.h"
#include "shadow.h"
#include "fb.h"

#ifdef RENDER
#include "mipict.h"
#include "picturestr.h"
#endif

/*
 * Windows headers
 */
#include "winms.h"

/* Cygwin's winuser.h does not define VK_KANA as of 28Mar2001 */
/* NOTE: Cygwin's winuser.h was fixed shortly after 28Mar2001. */
#ifndef VK_KANA
#define VK_KANA 15
#endif

#include <stdio.h>

/*
 * Typedefs for engine dependent function pointers
 */

typedef Bool (*winAllocateFBProcPtr)(ScreenPtr);

typedef void (*winShadowUpdateProcPtr)(ScreenPtr, PixmapPtr, RegionPtr);

typedef void *(*winShadowWindowProcPtr)(ScreenPtr,
					CARD32, CARD32,
					int, CARD32*);

typedef Bool (*winCloseScreenProcPtr)(int, ScreenPtr);

typedef Bool (*winInitVisualsProcPtr)(ScreenPtr);

typedef Bool (*winAdjustVideoModeProcPtr)(ScreenPtr);

typedef void (*winCreateBoundingWindowProcPtr)(ScreenPtr);

typedef Bool (*winFinishScreenInitProcPtr)(int, ScreenPtr, int, char **);

typedef Bool (*winBltExposedRegionsProcPtr)(ScreenPtr);

typedef Bool (*winActivateAppProcPtr)(ScreenPtr);

/*
 * Privates structures
 */

typedef struct {
  DWORD			dwDummy;
} winPrivWin;

typedef struct {
  DWORD			dwDummy;
} winPrivGC;

typedef struct
{
  ScreenPtr		pScreen;
  DWORD			dwScreen;
  DWORD			dwWidth;
  DWORD			dwPaddedWidth;
  DWORD			dwHeight;
  DWORD			dwDepth;
  DWORD			dwDPIx;
  DWORD			dwDPIy;
  DWORD			dwStrideBytes;
  DWORD			dwStride;
  DWORD			dwBPP;
  char			*pfb;
  XWDColor		*pXWDCmap;
  XWDFileHeader		*pXWDHeader;
  Pixel			pixelBlack;
  Pixel			pixelWhite;
  DWORD			dwLineBias;
  DWORD			dwEngine;
  DWORD			dwEnginePreferred;
  DWORD			dwEnginesSupported;
  Bool			fFullScreen;
  int			iE3BTimeout;
} winScreenInfo, *winScreenInfoPtr;

typedef struct
{
  winScreenInfoPtr	pScreenInfo;

  Bool			fEnabled;
  Bool			fClosed;
  Bool			fActive;
  Bool			fCursor;
  
  int			iDeltaZ;

  CloseScreenProcPtr	CloseScreen;

  DWORD			dwRedMask;
  DWORD			dwGreenMask;
  DWORD			dwBlueMask;
  DWORD			dwBitsPerRGB;

  DWORD			dwModeKeyStates;

  /* 3 button emulation variables */
  int			iE3BCachedPress;
  Bool			fE3BFakeButton2Sent;

  /* Privates used by shadow fb GDI server */
  HBITMAP		hbmpShadow;
  HDC			hdcScreen;
  HDC			hdcShadow;
  HWND			hwndScreen;
  
  /* Privates used by shadow fb and primary fb DirectDraw servers */
  LPDIRECTDRAW		pdd;
  LPDIRECTDRAWSURFACE	pddsPrimary;

  /* Privates used by shadow fb DirectDraw server */
  LPDIRECTDRAWSURFACE	pddsShadow;
  LPDDSURFACEDESC	pddsdShadow;

  /* Privates used by primary fb DirectDraw server */
  LPDIRECTDRAWSURFACE	pddsOffscreen;
  LPDDSURFACEDESC	pddsdOffscreen;
  LPDDSURFACEDESC	pddsdPrimary;

  /* Privates used by shadow fb DirectDraw Nonlocking server */
  LPDIRECTDRAW4		pdd4;
  LPDIRECTDRAWSURFACE4	pddsShadow4;
  LPDIRECTDRAWSURFACE4	pddsPrimary4;

  /* Privates used by both shadow fb DirectDraw servers */
  LPDIRECTDRAWCLIPPER	pddcPrimary;

  /* Engine specific functions */
  winAllocateFBProcPtr			pwinAllocateFB;
  winShadowUpdateProcPtr		pwinShadowUpdate;
  winShadowWindowProcPtr		pwinShadowWindow;
  winCloseScreenProcPtr			pwinCloseScreen;
  winInitVisualsProcPtr			pwinInitVisuals;
  winAdjustVideoModeProcPtr		pwinAdjustVideoMode;
  winCreateBoundingWindowProcPtr	pwinCreateBoundingWindow;
  winFinishScreenInitProcPtr		pwinFinishScreenInit;
  winBltExposedRegionsProcPtr		pwinBltExposedRegions;
  winActivateAppProcPtr			pwinActivateApp;
} winPrivScreenRec, *winPrivScreenPtr;

extern ColormapPtr		g_cmInstalledMaps[];
extern winScreenInfo		g_winScreens[];
extern char			*g_pcDisplay;
extern miPointerScreenFuncRec	g_winPointerCursorFuncs;
extern DWORD			g_dwEvents;
extern int			g_fdMessageQueue;
extern int			g_winScreenPrivateIndex;
extern unsigned long		g_winGeneration;
extern CARD32			g_c32LastInputEventTime;

/*
 * Screen privates macros
 */
#define winGetScreenPriv(pScreen) ((winPrivScreenPtr) \
				   (pScreen)->devPrivates[g_winScreenPrivateIndex].ptr)

#define winSetScreenPriv(pScreen,v) ((pScreen)->devPrivates[g_winScreenPrivateIndex].ptr = \
				     (pointer) v)

#define winScreenPriv(pScreen) winPrivScreenPtr pScreenPriv = winGetScreenPriv(pScreen)

/*
 * Window privates macros
 */
#define winGetWindowPrivate(_pWin) ((winPrivWin *)\
	(_pWin)->devPrivates[winWindowPrivateIndex].ptr)

/*
 * FIXME: Windows mouse wheel macro; should be in Cygwin w32api headers.
 * Has been fixed after May 05, 2001.  Remove this section after the
 * fixed headers are in distribution.
 */
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wparam)		((short)HIWORD (wparam))
#endif /* GET_WHEEL_DELTA_WPARAM */

/*
 * BEGIN DDX and DIX Function Prototypes
 */

/*
 * InitOutput.c
 */

DWORD
winBitsPerPixel (DWORD dwDepth);

/*
 * winallpriv.c
 */

Bool
winAllocatePrivates (ScreenPtr pScreen);

/*
 * winblock.c
 */

void
winBlockHandler (pointer pBlockData,
		 OSTimePtr pptv,
		 pointer pReadMask);

/*
 * winclip.c
 */

RegionPtr
winPixmapToRegionNativeGDI (PixmapPtr pPix);

/*
 * wincmap.c
 */

int
winListInstalledColormapsNativeGDI (ScreenPtr pScreen, Colormap *pmaps);

Bool
winInitVisualsNativeGDI (ScreenPtr pScreen);

void
winStoreColorsNativeGDI (ColormapPtr pmap, int ndef, xColorItem *pdefs);

void
winInstallColormapNativeGDI (ColormapPtr pmap);

void
winUninstallColormapNativeGDI (ColormapPtr pmap);

void
winResolveColorNativeGDI (unsigned short *pred,
			  unsigned short *pgreen,
			  unsigned short *pblue,
			  VisualPtr	pVisual);

Bool
winInitializeColormapNativeGDI (ColormapPtr pmap);

int
winExpandDirectColorsNativeGDI (ColormapPtr pmap, int ndef,
				xColorItem *indefs, xColorItem *outdefs);

Bool
winCreateDefColormapNativeGDI (ScreenPtr pScreen);

Bool
winSetVisualTypesNativeGDI (int nDepth, int nVisuals, int nBitsPerRGB);

void
winClearVisualTypes (void);

/*
 * wincursor.c
 */

Bool
winCursorOffScreen (ScreenPtr *ppScreen, int *x, int *y);

void
winCrossScreen (ScreenPtr pScreen, Bool fEntering);

/*
 * winfillsp.c
 */

void
winFillSpansNativeGDI (DrawablePtr	pDrawable,
		       GCPtr		pGC,
		       int		nSpans,
		       DDXPointPtr	pPoints,
		       int		*pWidths,
		       int		fSorted);

/*
 * winfont.c
 */

Bool
winRealizeFontNativeGDI (ScreenPtr pScreen, FontPtr pFont);

Bool
winUnrealizeFontNativeGDI (ScreenPtr pScreen, FontPtr pFont);

/*
 * wingc.c
 */

Bool
winCreateGCNativeGDI (GCPtr pGC);

void
winChangeGCNativeGDI (GCPtr pGC, unsigned long ulChanges);

void
winPadPixmapNativdGDI (PixmapPtr pPixmap);

void
winValidateGCNativeGDI (GCPtr pGC,
			unsigned long changes,
			DrawablePtr pDrawable);

void
winCopyGCNativeGDI (GCPtr pGCsrc, unsigned long ulMask, GCPtr pGCdst);

void
winDestroyGCNativeGDI (GCPtr pGC);

void
winChangeClipNativeGDI (GCPtr pGC, int nType, pointer pValue, int nRects);

void
winDestroyClipNativeGDI (GCPtr pGC);

void
winCopyClipNativeGDI (GCPtr pGCdst, GCPtr pGCsrc);

/*
 * wingetsp.c
 */

void
winGetSpansNativeGDI (DrawablePtr	pDrawable, 
		      int		wMax, 
		      DDXPointPtr	pPoints, 
		      int		*pWidths, 
		      int		nSpans, 
		      char		*pDst);

/*
 * winkeybd.c
 */

void
winTranslateKey (WPARAM wParam, LPARAM lParam, int *piScanCode);

void
winGetKeyMappings (KeySymsPtr pKeySyms, CARD8 *pModMap);

void
winKeybdBell (int iPercent, DeviceIntPtr pDeviceInt,
	      pointer pCtrl, int iClass);

void
winKeybdCtrl (DeviceIntPtr pDevice, KeybdCtrl *pCtrl);

int
winKeybdProc (DeviceIntPtr pDeviceInt, int iState);

void
winInitializeModeKeyStates (void);

void
winStoreModeKeyStates (ScreenPtr pScreen);

void
winRestoreModeKeyStates (ScreenPtr pScreen);

Bool
winIsFakeCtrl_L (UINT message, WPARAM wParam, LPARAM lParam);

/*
 * winmisc.c
 */

void
winQueryBestSizeNativeGDI (int class, unsigned short *pWidth,
			   unsigned short *pHeight, ScreenPtr pScreen);

CARD8
winCountBits (DWORD dw);

Bool
winUpdateFBPointer (ScreenPtr pScreen, void *pbits);

/*
 * winmouse.c
 */

void
winMouseCtrl (DeviceIntPtr pDevice, PtrCtrl *pCtrl);

int
winMouseProc (DeviceIntPtr pDeviceInt, int iState);

int
winMouseWheel (ScreenPtr pScreen, int iDeltaZ);

void
winMouseButtonsSendEvent (int iEventType, int iButton);

int
winMouseButtonsHandle (ScreenPtr pScreen,
		       int iEventType, int iButton,
		       WPARAM wParam);

/*
 * winpfbddd.c
 */

Bool
winAllocateFBPrimaryDD (ScreenPtr pScreen);

Bool
winCloseScreenPrimaryDD (int nIndex, ScreenPtr pScreen);

Bool
winInitVisualsPrimaryDD (ScreenPtr pScreen);

Bool
winAdjustVideoModePrimaryDD (ScreenPtr pScreen);

Bool
winActivateAppPrimaryDD (ScreenPtr pScreen);

Bool
winSetEngineFunctionsPrimaryDD (ScreenPtr pScreen);

/*
 * winpixmap.c
 */

PixmapPtr
winCreatePixmapNativeGDI (ScreenPtr pScreen, int width, int height, int depth);

Bool
winDestroyPixmapNativeGDI (PixmapPtr pPixmap);

void
winXRotatePixmapNativeGDI (PixmapPtr pPix, int rw);

void
winYRotatePixmapNativeGDI (PixmapPtr pPix, int rh);

void
winCopyRotatePixmapNativeGDI (PixmapPtr psrcPix, PixmapPtr *ppdstPix,
			      int xrot, int yrot);

/*
 * winpntwin.c
 */

void
winPaintWindowNativeGDI (WindowPtr pWin, RegionPtr pRegion, int what);

/*
 * winpolyline.c
 */

void
winPolyLineNativeGDI (DrawablePtr	pDrawable,
		      GCPtr		pGC,
		      int		mode,
		      int		npt,
		      DDXPointPtr	ppt);

/*
 * winscrinit.c
 */

Bool
winScreenInit (int index,
	       ScreenPtr pScreen,
	       int argc, char **argv);

Bool
winFinishScreenInitFB (int index,
		       ScreenPtr pScreen,
		       int argc, char **argv);

Bool
winFinishScreenInitNativeGDI (int index,
			      ScreenPtr pScreen,
			      int argc, char **argv);

Bool
winSaveScreen (ScreenPtr pScreen, int on);

PixmapPtr
winGetWindowPixmap (WindowPtr pWin);

void
winSetWindowPixmap (WindowPtr pWin, PixmapPtr pPix);

Bool
winDetectSupportedEngines (ScreenPtr pScreen);

Bool
winSetEngine (ScreenPtr pScreen);

void
winCreateBoundingWindowFullScreen (ScreenPtr pScreen);

void
winCreateBoundingWindowWindowed (ScreenPtr pScreen);

/*
 * winsetsp.c
 */

void
winSetSpansNativeGDI (DrawablePtr	pDrawable,
		      GCPtr		pGC,
		      char		*pSrc,
		      DDXPointPtr	pPoints,
		      int		*pWidth,
		      int		nSpans,
		      int		fSorted);

/*
 * winshaddd.c
 */

Bool
winAllocateFBShadowDD (ScreenPtr pScreen);

void
winShadowUpdateDD (ScreenPtr pScreen, 
		   PixmapPtr pShadow,
		   RegionPtr damage);

void *
winShadowSetWindowLinearDD (ScreenPtr	pScreen,
			    CARD32	dwRow,
			    CARD32	dwOffset,
			    int		mode,
			    CARD32	*pdwSize);

void *
winShadowWindowDD (ScreenPtr	pScreen,
		   CARD32	row,
		   CARD32	offset,
		   int		mode,
		   CARD32	*size);

Bool
winCloseScreenShadowDD (int nIndex, ScreenPtr pScreen);

Bool
winInitVisualsShadowDD (ScreenPtr pScreen);

Bool
winAdjustVideoModeShadowDD (ScreenPtr pScreen);

Bool
winBltExposedRegionsShadowDD (ScreenPtr pScreen);

Bool
winActivateAppShadowDD (ScreenPtr pScreen);

Bool
winSetEngineFunctionsShadowDD (ScreenPtr pScreen);

/*
 * winshadddnl.c
 */

Bool
winAllocateFBShadowDDNL (ScreenPtr pScreen);

void
winShadowUpdateDDNL (ScreenPtr pScreen, 
		     PixmapPtr pShadow,
		     RegionPtr damage);

void *
winShadowSetWindowLinearDDNL (ScreenPtr	pScreen,
			      CARD32	dwRow,
			      CARD32	dwOffset,
			      int	mode,
			      CARD32	*pdwSize);

void *
winShadowWindowDDNL (ScreenPtr	pScreen,
		     CARD32	row,
		     CARD32	offset,
		     int	mode,
		     CARD32	*size);

Bool
winCloseScreenShadowDDNL (int nIndex, ScreenPtr pScreen);

Bool
winInitVisualsShadowDDNL (ScreenPtr pScreen);

Bool
winAdjustVideoModeShadowDDNL (ScreenPtr pScreen);

Bool
winBltExposedRegionsShadowDDNL (ScreenPtr pScreen);

Bool
winActivateAppShadowDDNL (ScreenPtr pScreen);

Bool
winSetEngineFunctionsShadowDDNL (ScreenPtr pScreen);

/*
 * winshadgdi.c
 */

Bool
winAllocateFBShadowGDI (ScreenPtr pScreen);

void
winShadowUpdateGDI (ScreenPtr pScreen, 
		    PixmapPtr pShadow,
		    RegionPtr damage);

void *
winShadowSetWindowLinearGDI (ScreenPtr	pScreen,
			     CARD32	dwRow,
			     CARD32	dwOffset,
			     int	mode,
			     CARD32	*pdwSize);

void *
winShadowWindowGDI (ScreenPtr	pScreen,
		    CARD32	row,
		    CARD32	offset,
		    int		mode,
		    CARD32	*size);

Bool
winCloseScreenShadowGDI (int nIndex, ScreenPtr pScreen);

Bool
winInitVisualsShadowGDI (ScreenPtr pScreen);

Bool
winAdjustVideoModeShadowGDI (ScreenPtr pScreen);

Bool
winActivateAppShadowGDI (ScreenPtr pScreen);

Bool
winSetEngineFunctionsShadowGDI (ScreenPtr pScreen);

/*
 * winwakeup.c
 */

void
winWakeupHandler (pointer pWakeupData,
		  int err,
		  pointer pReadmask);

/*
 * winwindow.c
 */

Bool
winCreateWindowNativeGDI (WindowPtr pWin);

Bool
winDestroyWindowNativeGDI (WindowPtr pWin);

Bool
winPositionWindowNativeGDI (WindowPtr pWin, int x, int y);

void 
winCopyWindowNativeGDI (WindowPtr pWin,
			DDXPointRec ptOldOrg,
			RegionPtr prgnSrc);

Bool
winChangeWindowAttributesNativeGDI (WindowPtr pWin, unsigned long mask);

Bool
winUnmapWindowNativeGDI (WindowPtr pWindow);

Bool
winMapWindowNativeGDI (WindowPtr pWindow);

/*
 * winwndproc.c
 */

LRESULT CALLBACK
winWindowProc (HWND hWnd, UINT message, 
	       WPARAM wParam, LPARAM lParam);

void
winRestoreModeKeyStates (ScreenPtr pScreen);

/*
 * END DDX and DIX Function Prototypes
 */

#endif /* _WIN_H_ */

