/*
 * $XFree86: xc/programs/Xserver/miext/shadow/shadow.h,v 1.3 2000/09/12 19:40:13 keithp Exp $
 *
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _SHADOW_H_
#define _SHADOW_H_

#ifdef RENDER
#include "picturestr.h"
#endif

typedef void (*ShadowUpdateProc) (ScreenPtr pScreen, 
				  PixmapPtr pShadow,
				  RegionPtr damage);

#define SHADOW_WINDOW_RELOCATE 1
#define SHADOW_WINDOW_READ 2
#define SHADOW_WINDOW_WRITE 4

typedef void *(*ShadowWindowProc) (ScreenPtr	pScreen,
				   CARD32	row,
				   CARD32	offset,
				   int		mode,
				   CARD32	*size);

typedef struct _shadowScrPriv {
    PaintWindowBackgroundProcPtr PaintWindowBackground;
    PaintWindowBorderProcPtr	PaintWindowBorder;
    CopyWindowProcPtr		CopyWindow;
    CloseScreenProcPtr		CloseScreen;
    CreateGCProcPtr		CreateGC;
    GetImageProcPtr		GetImage;
#ifdef RENDER
    CompositeProcPtr		Composite;
    GlyphsProcPtr		Glyphs;
#endif    
    ShadowUpdateProc		update;
    ShadowWindowProc		window;
    RegionRec			damage;
} shadowScrPrivRec, *shadowScrPrivPtr;

extern int shadowScrPrivateIndex;

#define shadowGetScrPriv(pScr)  ((shadowScrPrivPtr) (pScr)->devPrivates[shadowScrPrivateIndex].ptr)
#define shadowScrPriv(pScr)	shadowScrPrivPtr    pScrPriv = shadowGetScrPriv(pScr)

Bool
shadowInit (ScreenPtr pScreen, ShadowUpdateProc update, ShadowWindowProc window);

void *
shadowAlloc (int width, int height, int bpp);

void
shadowUpdatePacked (ScreenPtr pScreen,
		    PixmapPtr pShadow,
		    RegionPtr damage);

void
shadowUpdatePlanar4 (ScreenPtr pScreen,
		     PixmapPtr pShadow,
		     RegionPtr damage);

void
shadowUpdatePlanar4x8 (ScreenPtr pScreen,
		       PixmapPtr pShadow,
		       RegionPtr damage);

void
shadowUpdateRotate8 (ScreenPtr pScreen,
		     PixmapPtr pShadow,
		     RegionPtr damage);

void
shadowUpdateRotate16 (ScreenPtr pScreen,
		      PixmapPtr pShadow,
		      RegionPtr damage);

void
shadowUpdateRotate32 (ScreenPtr pScreen,
		      PixmapPtr pShadow,
		      RegionPtr damage);

#endif /* _SHADOW_H_ */
