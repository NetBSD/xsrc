/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/glint/glint.h,v 1.5.2.1 1998/07/30 06:23:40 hohndel Exp $ */
/*
 * Copyright 1997 by Alan Hourihane <alanh@fairlite.demon.co.uk>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *           Dirk Hohndel, <hohndel@suse.de>
 *	     Stefan Dirsch, <sndirsch@suse.de>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 */
#ifndef _GLINT_H_
#define _GLINT_H_

#define GLINT_PATCHLEVEL "0"

#ifndef LINKKIT

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "pixmap.h"
#include "region.h"
#include "gc.h"
#include "gcstruct.h"
#include "colormap.h"
#include "colormapst.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "mipointer.h"
#include "cursorstr.h"
#include "windowstr.h"
#include "compiler.h"
#include "misc.h"
#include "xf86.h"
#include "regionstr.h"
#include "xf86Procs.h"

#include "glintcurs.h"
#include "glint_regs.h"

extern volatile unsigned long *VidBase;

#include <X11/Xfuncproto.h>

#else /* !LINKKIT */
#include "X.h"
#include "input.h"
#include "misc.h"
#include "xf86.h"
#include "xf86_ansic.h"
#endif /* !LINKKIT */

#if !defined(__GNUC__) || defined(NO_INLINE)
#define __inline__ /**/
#endif

extern ScrnInfoRec glintInfoRec;
extern volatile pointer GLINTMMIOBase;

#ifndef LINKKIT
_XFUNCPROTOBEGIN

extern void (*glintImageReadFunc)(
    int, int, int, int, char *, int, int, int, unsigned long
);
extern void (*glintImageWriteFunc)(
    int, int, int, int, char *, int, int, int, short, unsigned long
);
extern void (*glintImageFillFunc)(
    int, int, int, int, char *, int, int, int, int, int, short, unsigned long
);

extern short glintalu[];
extern volatile pointer glintVideoMem;
extern ScreenPtr savepScreen;

extern int glintValidTokens[];

/* Function Prototypes */

/* glintConf.c */
/* glint.c */
void glintPrintIdent(
    void
);
Bool glintProbe(
    void
);
/* glintmisc.c */
Bool glintInitialize(
    int,
    ScreenPtr,
    int,
    char **
);
void glintEnterLeaveVT(
    Bool,
    int 
);
Bool glintCloseScreen(
    int,
    ScreenPtr
);
Bool glintSaveScreen(
    ScreenPtr,
    Bool 
);
Bool glintSwitchMode(
    DisplayModePtr 
);
void glintDPMSSet(
    int PowerManagementMode
);
void glintAdjustFrame(
    int,
    int 
);
/* glintcmap.c */
int glintListInstalledColormaps(
    ScreenPtr,
    Colormap *
);
void glintRestoreDACvalues(
    void
);
int glintGetInstalledColormaps(
    ScreenPtr,
    ColormapPtr *
);
void glintStoreColors(
    ColormapPtr,
    int,
    xColorItem *
);
void glintInstallColormap(
    ColormapPtr 
);
void glintUninstallColormap(
    ColormapPtr 
);
void glintRestoreColor0(
    ScreenPtr 
);
/* glintgc.c */
Bool glintCreateGC(
    GCPtr 
);
/* glintgc16.c */
Bool glintCreateGC16(
    GCPtr 
);
/* glintgc32.c */
Bool glintCreateGC32(
    GCPtr 
);
/* glintfs.c */
void glintSolidFSpans(
    DrawablePtr,
    GCPtr,
    int,
    DDXPointPtr,
    int *,
    int 
);
void glintTiledFSpans(
    DrawablePtr,
    GCPtr,
    int,
    DDXPointPtr,
    int *,
    int 
);
void glintStipFSpans(
    DrawablePtr,
    GCPtr,
    int,
    DDXPointPtr,
    int *,
    int 
);
void glintOStipFSpans(
    DrawablePtr,
    GCPtr,
    int,
    DDXPointPtr,
    int *,
    int 
);
/* glintss.c */
void glintSetSpans(
    DrawablePtr,
    GCPtr,
    char *,
    DDXPointPtr,
    int *,
    int,
    int 
);
/* glintgs.c */
void glintGetSpans(
    DrawablePtr,
    int,
    DDXPointPtr,
    int *,
    int,
    char *
);
/* glintwin.c */
void glintCopyWindow(
    WindowPtr,
    DDXPointRec,
    RegionPtr 
);
/* glintinit.c */
void glintCalcCRTCRegs(
    glintCRTCRegPtr, 
    DisplayModePtr
);
void glintSetCRTCRegs(
    glintCRTCRegPtr
);
void glintCleanUp(
    void
);
Bool glintInit(
    DisplayModePtr 
);
void glintInitEnvironment(
    void
);
void glintUnlock(
    void
);
/* glintim.c */
void glintImageInit(
    void
);
void glintImageWriteNoMem(
    int,
    int,
    int,
    int,
    char *,
    int,
    int,
    int,
    short,
    unsigned long
);
void glintImageStipple(
    int,
    int,
    int,
    int,
    char *,
    int,
    int,
    int,
    int,
    int,
    Pixel,
    short,
    unsigned long
);
void glintImageOpStipple(
    int,
    int,
    int,
    int,
    char *,
    int,
    int,
    int,
    int,
    int,
    Pixel,
    Pixel,
    short,
    unsigned long 
);
/* glintbstor.c */
void glintSaveAreas(
    PixmapPtr,
    RegionPtr,
    int,
    int,
    WindowPtr 
);
void glintRestoreAreas(
    PixmapPtr,
    RegionPtr,
    int,
    int,
    WindowPtr 
);
/* glintscrin.c */
Bool glintScreenInit(
    ScreenPtr,
    pointer,
    int,
    int,
    int,
    int,
    int 
);
/* glintblt.c */
RegionPtr glintCopyArea(
    DrawablePtr,
    DrawablePtr,
    GC *,
    int,
    int,
    int,
    int,
    int,
    int
);
void glintFindOrdering(
    DrawablePtr,
    DrawablePtr,
    GC *,
    int,
    BoxPtr,
    int,
    int,
    int,
    int,
    unsigned int *
);
RegionPtr glintCopyPlane(
    DrawablePtr,
    DrawablePtr,
    GCPtr,
    int,
    int,
    int,
    int,
    int,
    int,
    unsigned long 
);
/* glintplypt.c */
void glintPolyPoint(
    DrawablePtr,
    GCPtr,
    int,
    int,
    xPoint *
);
/* glintline.c */
void glintLine(
    DrawablePtr,
    GCPtr,
    int,
    int,
    DDXPointPtr 
);
/* glintseg.c */
void glintSegment(
    DrawablePtr,
    GCPtr,
    int,
    xSegment *
);
/* glintfrect.c */
void glintPolyFillRect(
    DrawablePtr,
    GCPtr,
    int,
    xRectangle *
);
void glintInitFrect(
    int,
    int,
    int
);
/* glinttext.c */
int glintNoCPolyText(
    DrawablePtr,
    GCPtr,
    int,
    int,
    int,
    char *,
    Bool 
);

void glintFontStipple(
    int,
    int,
    int,
    int,
    unsigned char *,
    int,
    Pixel
);


int glintNoCImageText(
    DrawablePtr,
    GCPtr,
    int,
    int,
    int,
    char *,
    Bool 
);
/* glintfont.c */
Bool glintRealizeFont(
    ScreenPtr,
    FontPtr 
);
Bool glintUnrealizeFont(
    ScreenPtr,
    FontPtr 
);
/* glintfcach.c */
void glintFontCache8Init(
    void
);
/* glintbcach.c */
void glintCacheMoveBlock(
    int,
    int,
    int,
    int,
    int,
    int,
    unsigned int
);
/* glintcurs.c */
Bool glintCursorInit(
    char *,
    ScreenPtr 
);
void glintShowCursor(
    void
);
void glintHideCursor(
    void
);
void glintRestoreCursor(
    ScreenPtr 
);
void glintRepositionCursor(
    ScreenPtr 
);
void glintRenewCursorColor(
    ScreenPtr 
);
void glintWarpCursor(
    ScreenPtr,
    int,
    int 
);
void glintQueryBestSize(
    int,
    unsigned short *,
    unsigned short *,
    ScreenPtr 
);
/* glintBtCurs.c */
void glintBtRecolorCursor(
    ScreenPtr, CursorPtr, Bool
);
/* glintdline.c */
void glintDline(
    DrawablePtr,
    GCPtr,
    int,
    int,
    DDXPointPtr 
);
/* glintdseg.c */
void glintDsegment(
    DrawablePtr,
    GCPtr,
    int,
    xSegment *
);
/* glintgtimg.c */
void glintGetImage(
    DrawablePtr,
    int,
    int,
    int,
    int,
    unsigned int,
    unsigned long,
    char * 
);
/* glintTiCursor.c */
void glintOutTiIndReg(
    unsigned char,
    unsigned char,
    unsigned char 
);
unsigned char glintInTiIndReg(
    unsigned char 
);
Bool glintTiRealizeCursor(
    ScreenPtr,
    CursorPtr 
);
void glintTiCursorOn(
    void
);
void glintTiCursorOff(
    void
);
void glintTiMoveCursor(
    ScreenPtr,
    int,
    int 
);
void glintTiRecolorCursor(
    ScreenPtr,
    CursorPtr 
);
void glintTiLoadCursor(
    ScreenPtr,
    CursorPtr,
    int,
    int 
);

_XFUNCPROTOEND

#endif /* !LINKKIT */
#endif /* _I128_H_ */

