/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * Id: vmware.h,v 1.6 2001/01/30 18:13:47 bennett Exp $
 * **********************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmware.h,v 1.3 2001/09/13 08:36:24 alanh Exp $ */

#ifndef VMWARE_H
#define VMWARE_H

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Resources.h"

#include "compiler.h"	/* inb/outb */

#include "xf86PciInfo.h"	/* pci vendor id */
#include "xf86Pci.h"		/* pci */

#include "mipointer.h"		/* sw cursor */
#include "mibstore.h"		/* backing store */
#include "micmap.h"		/* mi color map */
#include "vgaHW.h"		/* VGA hardware */
#include "fb.h"

#include "xf86cmap.h"		/* xf86HandleColormaps */

#include "vm_basic_types.h"
#include "svga_reg.h"

typedef struct {
	CARD32		svga_reg_enable;
	CARD32		svga_reg_width;
	CARD32		svga_reg_height;
} VMWARERegRec, *VMWARERegPtr;

typedef struct {
	DisplayModePtr	mode;
} VMWAREFBLayout;

typedef struct {
	EntityInfoPtr	pEnt;
	pciVideoPtr	PciInfo;
	PCITAG		PciTag;
	Bool		Primary;
	int		depth;
	int		bitsPerPixel;
	rgb		weight;
	rgb		offset;
	int		defaultVisual;
	int		videoRam;
	unsigned long	memPhysBase;
	unsigned long	fbOffset;
	unsigned long   fbPitch;
	unsigned long	ioBase;
	int		maxWidth;
	int		maxHeight;
	unsigned int	vmwareCapability;

	unsigned char*	FbBase;
	unsigned long	FbSize;

	VMWARERegRec	SavedReg;
	VMWARERegRec	ModeReg;

	VMWAREFBLayout	CurrentLayout;
	Bool noAccel;
	Bool hwCursor;
	Bool cursorDefined;
	Bool mouseHidden;

        unsigned int    cursorRemoveFromFB;
        unsigned int    cursorRestoreToFB;

	unsigned long	mmioPhysBase;
	unsigned long	mmioSize;

	unsigned char*	mmioVirtBase;
	CARD32*		vmwareFIFO;
	Bool		vmwareFIFOMarkSet;
	BoxRec		vmwareAccelArea;
	struct {
	BoxRec		Box;
	unsigned int	Width;
	unsigned int	Height;
	unsigned int	XHot;
	unsigned int	YHot;
		      } Mouse;
	Bool		checkCursorColor;
	unsigned int	vmwareBBLevel;
	unsigned long	Pmsk;

	uint16		indexReg, valueReg;

	ScreenRec	ScrnFuncs;
	/* ... */
} VMWARERec, *VMWAREPtr;

#define VMWAREPTR(p) ((VMWAREPtr)((p)->driverPrivate))

static __inline ScrnInfoPtr infoFromScreen(ScreenPtr s) {
   return xf86Screens[s->myNum];
}

#include "gcstruct.h"

#define MIN(a,b)    ((a)<(b)?(a):(b))
#define MAX(a,b)    ((a)>(b)?(a):(b))

#define GC_FUNC_PROLOGUE(pGC) \
{ \
    vmwarePrivGCPtr pGCPriv = (vmwarePrivGCPtr) (pGC)->devPrivates[vmwareGCPrivateIndex].ptr; \
    (pGC)->funcs = pGCPriv->wrapFuncs; \
    (pGC)->ops = pGCPriv->wrapOps; \
}

#define GC_FUNC_EPILOGUE(pGC) \
{ \
    vmwarePrivGCPtr pGCPriv = (vmwarePrivGCPtr) (pGC)->devPrivates[vmwareGCPrivateIndex].ptr; \
    pGCPriv->wrapFuncs = (pGC)->funcs; \
    (pGC)->funcs = &vmwareGCFuncs; \
    pGCPriv->wrapOps = (pGC)->ops; \
    (pGC)->ops = &vmwareGCOps; \
}

#define GC_OPS(pGC) ((vmwarePrivGCPtr) (pGC)->devPrivates[vmwareGCPrivateIndex].ptr)->wrapOps

#define MOUSE_ID 1

#define UPDATE_ACCEL_AREA(pVMWARE,box) \
{ \
	if (!(pVMWARE)->vmwareFIFOMarkSet) { \
		(pVMWARE)->vmwareFIFOMarkSet = TRUE; \
		(pVMWARE)->vmwareAccelArea.x1 = (box).x1; \
		(pVMWARE)->vmwareAccelArea.y1 = (box).y1; \
		(pVMWARE)->vmwareAccelArea.x2 = (box).x2; \
		(pVMWARE)->vmwareAccelArea.y2 = (box).y2; \
	} else { \
		if ((box).x1 < (pVMWARE)->vmwareAccelArea.x1) \
			(pVMWARE)->vmwareAccelArea.x1 = (box).x1; \
		if ((box).y1 < (pVMWARE)->vmwareAccelArea.y1) \
			(pVMWARE)->vmwareAccelArea.y1 = (box).y1; \
		if ((box).x2 > (pVMWARE)->vmwareAccelArea.x2) \
			(pVMWARE)->vmwareAccelArea.x2 = (box).x2; \
		if ((box).y2 > (pVMWARE)->vmwareAccelArea.y2) \
			(pVMWARE)->vmwareAccelArea.y2 = (box).y2; \
	} \
}

#define ABS(x)	(((x) >= 0) ? (x) : -(x))
#define BOX_INTERSECT(a, b) \
		(ABS(((a).x1 + (a).x2) - ((b).x1 + (b).x2)) <= \
		((a).x2 - (a).x1) + ((b).x2 - (b).x1) && \
		ABS(((a).y1 + (a).y2) - ((b).y1 + (b).y2)) <= \
		((a).y2 - (a).y1) + ((b).y2 - (b).y1))

#define HIDE_CURSOR(vmPtr,box) \
{ \
    if (!(vmPtr)->mouseHidden) { \
	if ((vmPtr)->hwCursor && (vmPtr)->cursorDefined && \
	    BOX_INTERSECT((vmPtr)->Mouse.Box, box)) { \
		(vmPtr)->mouseHidden = TRUE; \
		if ((vmPtr)->vmwareCapability & SVGA_CAP_CURSOR_BYPASS) { \
		    vmwareWriteReg(vmPtr, SVGA_REG_CURSOR_ID, MOUSE_ID); \
		    vmwareWriteReg(vmPtr, SVGA_REG_CURSOR_ON, \
                                   (vmPtr)->cursorRemoveFromFB); \
		} else { \
		    vmwareWriteWordToFIFO(vmPtr, SVGA_CMD_DISPLAY_CURSOR); \
		    vmwareWriteWordToFIFO(vmPtr, MOUSE_ID); \
		    vmwareWriteWordToFIFO(vmPtr, 0); \
		    UPDATE_ACCEL_AREA(vmPtr,(vmPtr)->Mouse.Box); \
		} \
	} \
    } \
}

#define HIDE_CURSOR_ACCEL(vmPtr,box) \
{ \
    if (!(vmPtr)->mouseHidden) { \
        if ((vmPtr)->hwCursor && (vmPtr)->cursorDefined && \
            !((vmPtr)->vmwareCapability & SVGA_CAP_CURSOR_BYPASS) && \
            BOX_INTERSECT((vmPtr)->Mouse.Box, box)) { \
                (vmPtr)->mouseHidden = TRUE; \
                vmwareWriteWordToFIFO(vmPtr, SVGA_CMD_DISPLAY_CURSOR); \
                vmwareWriteWordToFIFO(vmPtr, MOUSE_ID); \
                vmwareWriteWordToFIFO(vmPtr, 0); \
                UPDATE_ACCEL_AREA(vmPtr,(vmPtr)->Mouse.Box); \
        } \
    } \
}


#define SHOW_CURSOR(vmPtr,box) \
{ \
    if ((vmPtr)->mouseHidden) { \
	if ((vmPtr)->hwCursor && (vmPtr)->cursorDefined && \
	    BOX_INTERSECT((vmPtr)->Mouse.Box, box)) { \
		(vmPtr)->mouseHidden = FALSE; \
		if ((vmPtr)->vmwareCapability & SVGA_CAP_CURSOR_BYPASS) { \
		    vmwareWriteReg(vmPtr, SVGA_REG_CURSOR_ID, MOUSE_ID); \
		    vmwareWriteReg(vmPtr, SVGA_REG_CURSOR_ON, \
                                   (vmPtr)->cursorRestoreToFB); \
		} else { \
		    vmwareWriteWordToFIFO(vmPtr, SVGA_CMD_DISPLAY_CURSOR); \
		    vmwareWriteWordToFIFO(vmPtr, MOUSE_ID); \
		    vmwareWriteWordToFIFO(vmPtr, 1); \
		    UPDATE_ACCEL_AREA(vmPtr,(vmPtr)->Mouse.Box); \
		} \
	} \
    } \
}

/* Only use this for debugging */

#define HIDE_CURSOR_ALWAYS(vmPtr,box) \
{ \
	vmwareWriteWordToFIFO(vmPtr, SVGA_CMD_DISPLAY_CURSOR); \
	vmwareWriteWordToFIFO(vmPtr, MOUSE_ID); \
	vmwareWriteWordToFIFO(vmPtr, 0); \
	(vmPtr)->mouseHidden = TRUE; \
	vmwareFIFOMarkSet = TRUE; \
        UPDATE_ACCEL_AREA(vmPtr,(vmPtr)->Mouse.Box); \
}

#define SHOW_CURSOR_ALWAYS(vmPtr,box) \
{ \
	vmwareWriteWordToFIFO(vmPtr, SVGA_CMD_DISPLAY_CURSOR); \
	vmwareWriteWordToFIFO(vmPtr, MOUSE_ID); \
	vmwareWriteWordToFIFO(vmPtr, 1); \
	(vmPtr)->mouseHidden = FALSE; \
	vmwareFIFOMarkSet = TRUE; \
}

/*#define DEBUG_LOGGING*/
#undef DEBUG_LOGGING
#ifdef DEBUG_LOGGING
#define	VmwareLog(args)		ErrorF args
#define TRACEPOINT		VmwareLog((__FUNCTION__ ":" __FILE__ "\n"));
#else
#define	VmwareLog(args)
#define TRACEPOINT
#endif

/* Undefine this to kill all acceleration */
#define ACCELERATE_OPS

typedef struct vmwarePrivGC
{
    GCFuncs *wrapFuncs;
    GCOps *wrapOps;
}
vmwarePrivGC, *vmwarePrivGCPtr;


extern int vmwareGCPrivateIndex;

#define GEN_FUNC_WRAPPER(cond,init,setBB,op)                           \
    if ((cond)) {                                                      \
	init;                                                          \
	                                                               \
	if (pVMWARE->vmwareBBLevel == 0) {                             \
	    BoxRec BB;                                                 \
                                                                       \
	    setBB;						       \
	    HIDE_CURSOR(pVMWARE, BB);                                  \
	    vmwareWaitForFB(pVMWARE);                                  \
	    pVMWARE->vmwareBBLevel++;                                  \
	    op;                                                        \
	    pVMWARE->vmwareBBLevel--;                                  \
	    vmwareSendSVGACmdUpdate(pVMWARE, &BB);                     \
	    SHOW_CURSOR(pVMWARE, BB);                                  \
	} else {                                                       \
	    vmwareWaitForFB(pVMWARE);                                  \
	    op;                                                        \
	}                                                              \
    } else {                                                           \
	op;							       \
    }
#define VM_FUNC_WRAPPER(cond,setBB,op) \
    GEN_FUNC_WRAPPER(cond,,setBB,op)

#define GC_FUNC_WRAPPER(cond,screen,setBB,op)                          \
    GEN_FUNC_WRAPPER(cond, VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(screen)), setBB, op)

#define GC_FUNC_ACCEL_WRAPPER(cond,screen,setBB,accelcond,accel,op)	\
    GEN_FUNC_WRAPPER(cond,						\
	VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(screen));		\
        if (accelcond) {						\
	    BoxRec BB;							\
	    Bool hidden = pVMWARE->mouseHidden;				\
									\
	    setBB;							\
	    if (!hidden) {						\
		HIDE_CURSOR_ACCEL(pVMWARE, BB);				\
	    }								\
	    accel;							\
	    if (!hidden) {						\
		SHOW_CURSOR(pVMWARE, BB);				\
	    }								\
	    UPDATE_ACCEL_AREA(pVMWARE, BB);				\
	    return;							\
	},								\
	setBB, op)

void vmwareWriteReg(
#if NeedFunctionPrototypes
    VMWAREPtr pVMWARE, int index, CARD32 value
#endif
    );

void vmwareWriteWordToFIFO(
#if NeedFunctionPrototypes
    VMWAREPtr pVMWARE, CARD32 value
#endif
    );

void vmwareWaitForFB(
#ifdef NeedFunctionPrototypes
    VMWAREPtr pVMWARE
#endif
    );

void vmwareSendSVGACmdUpdate(
#if NeedFunctionPrototypes
    VMWAREPtr pVMWARE, BoxPtr pBB
#endif
    );

/* vmwaregc.c */

void vmwareInitGC(
#if NeedFunctionPrototypes
    void
#endif
    );

Bool vmwareCreateGC(
#if NeedFunctionPrototypes
    GCPtr pGC
#endif
    );

void vmwareValidateGC(
#if NeedFunctionPrototypes
    GCPtr pGC, unsigned long changes, DrawablePtr pDrawable
#endif
    );

void vmwareChangeGC(
#if NeedFunctionPrototypes
    GCPtr pGC, unsigned long changes
#endif
    );

void vmwareCopyGC(
#if NeedFunctionPrototypes
    GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst
#endif
    );

void vmwareDestroyGC(
#if NeedFunctionPrototypes
    GCPtr pGC
#endif
    );

void vmwareChangeClip(
#if NeedFunctionPrototypes
    GCPtr pGC, int type, pointer pValue, int nrects
#endif
    );

void vmwareDestroyClip(
#if NeedFunctionPrototypes
    GCPtr pGC
#endif
    );

void vmwareCopyClip(
#if NeedFunctionPrototypes
    GCPtr pGCDst, GCPtr pGCSrc
#endif
    );

/* vmwareinit.c */

void vmwareInitEnvironment(
#if NeedFunctionPrototypes
    void
#endif
    );

void vmwareInitDisplay(
#if NeedFunctionPrototypes
    int screen_idx
#endif
    );

void vmwareCleanUp(
#if NeedFunctionPrototypes
    void
#endif
    );

/* vmwarescrin.c */
CARD32 vmwareCalculateOffset(CARD32 mask);

Bool vmwareScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr pScreen,
    pointer pbits, int xsize, int ysize, int dpix, int dpiy, int width
#endif
    );

/* vmwarevga.c */

void vmwareSaveVGAInfo(
#if NeedFunctionPrototypes
    int screen_idx
#endif
    );

void vmwareRestoreVGAInfo(
#if NeedFunctionPrototypes
    void
#endif
    );

/* vmwarecmap.c */

void vmwareSetPaletteBase(
#if NeedFunctionPrototypes
    int pal
#endif
    );

int vmwareListInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr pScreen, Colormap * pmaps
#endif
    );

int vmwareGetInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr pScreen, ColormapPtr * pmap
#endif
    );

void vmwareStoreColors(
#if NeedFunctionPrototypes
    ColormapPtr pmap, int ndef, xColorItem * pdefs
#endif
    );

void vmwareInstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr pmap
#endif
    );

void vmwareUninstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr pmap
#endif
    );

void vmwareRestoreColor0(
#if NeedFunctionPrototypes
    ScreenPtr pScreen
#endif
    );

/* vmwarecurs.c */
Bool vmwareCursorInit(
#if NeedFunctionPrototypes
    char *pm, ScreenPtr pScr
#endif
    );

Bool vmwareRealizeCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr, CursorPtr pCurs
#endif
    );

Bool vmwareUnrealizeCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr, CursorPtr pCurs
#endif
    );

void vmwareSetCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr, CursorPtr pCurs, int x, int y
#endif
    );

void vmwareRepositionCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr
#endif
    );

void vmwareRestoreCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr
#endif
    );

void vmwareMoveCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr, int x, int y
#endif
    );

void vmwareRenewCursorColor(
#if NeedFunctionPrototypes
    ScreenPtr pScr
#endif
    );

void vmwareRecolorCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr, CursorPtr pCurs, Bool displayed
#endif
    );

void vmwareWarpCursor(
#if NeedFunctionPrototypes
    ScreenPtr pScr, int x, int y
#endif
    );

void vmwareQueryBestSize(
#if NeedFunctionPrototypes
    int class, unsigned short *pwidth, unsigned short *pheight, ScreenPtr pScr
#endif
    );

void vmwareCursorOff(
#if NeedFunctionPrototypes
    VMWAREPtr pVMWARE
#endif
    );

void vmwareClearSavedCursor(
#if NeedFunctionPrototypes
    int scr_index
#endif
    );

void vmwareBlockHandler(
#if NeedFunctionPrototypes
    int i, pointer blockData, pointer pTimeout, pointer pReadmask
#endif
    );

/* BEGIN Screen functions that draw */

void vmwareCopyWindow(
#if NeedFunctionPrototypes
    WindowPtr pWindow, DDXPointRec ptOldOrg, RegionPtr prgnSrc
#endif
    );

void vmwarePaintWindow(
#if NeedFunctionPrototypes
    WindowPtr pWindow, RegionPtr pRegion, int what
#endif
    );

void vmwareGetSpans(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    int wMax, DDXPointPtr pPoints, int *pWidths, int nSpans, char *pDst
#endif
    );

void vmwareGetImage(
#if NeedFunctionPrototypes
    DrawablePtr src,
    int x,
    int y,
    int w,
    int h, unsigned int format, unsigned long planeMask, char *pBinImage
#endif
    );

void vmwareSaveDoomedAreas(
#if NeedFunctionPrototypes
    WindowPtr pWin, RegionPtr prgnSave, int xorg, int yorg
#endif
    );

RegionPtr vmwareRestoreAreas(
#if NeedFunctionPrototypes
    WindowPtr pWin,
    RegionPtr prgnRestore
#endif
    );

/* END Screen functions that draw */

/* BEGIN GCOps */

void vmwareFillSpans(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC, int nInit, DDXPointPtr pptInit, int *pwidthInit, int fSorted
#endif
    );

void vmwareSetSpans(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC,
    char *psrc, DDXPointPtr ppt, int *pwidth, int nspans, int fSorted
#endif
    );

void vmwarePutImage(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC,
    int depth,
    int x, int y, int w, int h, int leftPad, int format, char *pBits
#endif
    );

void
vmwareDoBitblt(DrawablePtr  pSrc,
	       DrawablePtr  pDst,
	       GCPtr	    pGC,
	       BoxPtr	    pbox,
	       int	    nbox,
	       int	    dx,
	       int	    dy,
	       Bool	    reverse,
	       Bool	    upsidedown,
	       Pixel	    bitplane,
	       void	    *closure);

RegionPtr vmwareCopyArea(
#if NeedFunctionPrototypes
    DrawablePtr pSrc,
    DrawablePtr pDst,
    GCPtr pGC, int srcx, int srcy, int w, int h, int dstx, int dsty
#endif
    );

RegionPtr vmwareCopyPlane(
#if NeedFunctionPrototypes
    DrawablePtr pSrcDrawable,
    DrawablePtr pDstDrawable,
    GCPtr pGC,
    int srcx,
    int srcy,
    int width, int height, int dstx, int dsty, unsigned long bitPlane
#endif
    );

void vmwarePolyPoint(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int mode, int npt, DDXPointPtr pptInit
#endif
    );

void vmwarePolylines(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int mode, int npt, DDXPointPtr pptInit
#endif
    );

void vmwarePolySegment(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int nseg, xSegment * pSegs
#endif
    );

void vmwarePolyRectangle(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int nrects, xRectangle * pRects
#endif
    );

void vmwarePolyArc(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int narcs, xArc * parcs
#endif
    );

void vmwareFillPolygon(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC, int shape, int mode, int count, DDXPointPtr pPts
#endif
    );

void vmwarePolyFillRect(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle * prectInit
#endif
    );

void vmwarePolyFillArc(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int narcs, xArc * parcs
#endif
    );

int vmwarePolyText8(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int x, int y, int count, char *chars
#endif
    );

int vmwarePolyText16(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, unsigned short *chars
#endif
    );

void vmwareImageText8(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable, GCPtr pGC, int x, int y, int count, char *chars
#endif
    );

void vmwareImageText16(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC, int x, int y, int count, unsigned short *chars
#endif
    );

void vmwareImageGlyphBlt(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC,
    int x, int y, unsigned int nglyph, CharInfoPtr * ppci, pointer pglyphBase
#endif
    );

void vmwarePolyGlyphBlt(
#if NeedFunctionPrototypes
    DrawablePtr pDrawable,
    GCPtr pGC,
    int x, int y, unsigned int nglyph, CharInfoPtr * ppci, pointer pglyphBase
#endif
    );

void vmwarePushPixels(
#if NeedFunctionPrototypes
    GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDst, int w, int h, int x, int y
#endif
    );

/* END GCOps */
#endif
