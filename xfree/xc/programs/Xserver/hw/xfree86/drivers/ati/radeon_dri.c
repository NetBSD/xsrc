/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_dri.c,v 1.13 2001/05/02 15:06:08 dawes Exp $ */
/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario,
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Rickard E. Faith <faith@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */


				/* Driver data structures */
#include "radeon.h"
#include "radeon_dri.h"
#include "radeon_reg.h"
#include "radeon_sarea.h"
#include "radeon_version.h"

				/* X and server generic header files */
#include "xf86.h"
#include "windowstr.h"

				/* GLX/DRI/DRM definitions */
#define _XF86DRI_SERVER_
#include "GL/glxtokens.h"
#include "sarea.h"

#if defined(__alpha__)
# define PCIGART_ENABLED
#else
# undef PCIGART_ENABLED
#endif

/* ?? HACK - for now, put this here... */
/* ?? Alpha - this may need to be a variable to handle UP1x00 vs TITAN */
#if defined(__alpha__)
# define DRM_PAGE_SIZE 8192
#else
# define DRM_PAGE_SIZE 4096
#endif

/* Initialize the visual configs that are supported by the hardware.
   These are combined with the visual configs that the indirect
   rendering core supports, and the intersection is exported to the
   client. */
static Bool RADEONInitVisualConfigs(ScreenPtr pScreen)
{
    ScrnInfoPtr         pScrn              = xf86Screens[pScreen->myNum];
    RADEONInfoPtr       info               = RADEONPTR(pScrn);
    int                 numConfigs         = 0;
    __GLXvisualConfig   *pConfigs          = 0;
    RADEONConfigPrivPtr pRADEONConfigs     = 0;
    RADEONConfigPrivPtr *pRADEONConfigPtrs = 0;
    int                 i, accum, stencil;

    switch (info->CurrentLayout.pixel_code) {
    case 8:  /* 8bpp mode is not support */
    case 15: /* FIXME */
    case 24: /* FIXME */
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[dri] RADEONInitVisualConfigs failed (depth %d not supported).  "
		   "Disabling DRI.\n", info->CurrentLayout.pixel_code);
	return FALSE;

#define RADEON_USE_ACCUM   1
#define RADEON_USE_STENCIL 1

    case 16:
	numConfigs = 1;
	if (RADEON_USE_ACCUM)   numConfigs *= 2;
	if (RADEON_USE_STENCIL) numConfigs *= 2;

	if (!(pConfigs
	      = (__GLXvisualConfig*)xcalloc(sizeof(__GLXvisualConfig),
					      numConfigs))) {
	    return FALSE;
	}
	if (!(pRADEONConfigs
	      = (RADEONConfigPrivPtr)xcalloc(sizeof(RADEONConfigPrivRec),
					       numConfigs))) {
	    xfree(pConfigs);
	    return FALSE;
	}
	if (!(pRADEONConfigPtrs
	      = (RADEONConfigPrivPtr*)xcalloc(sizeof(RADEONConfigPrivPtr),
						numConfigs))) {
	    xfree(pConfigs);
	    xfree(pRADEONConfigs);
	    return FALSE;
	}

	i = 0;
	for (accum = 0; accum <= RADEON_USE_ACCUM; accum++) {
	    for (stencil = 0; stencil <= RADEON_USE_STENCIL; stencil++) {
		pRADEONConfigPtrs[i] = &pRADEONConfigs[i];

		pConfigs[i].vid                = (VisualID)(-1);
		pConfigs[i].class              = -1;
		pConfigs[i].rgba               = TRUE;
		pConfigs[i].redSize            = 5;
		pConfigs[i].greenSize          = 6;
		pConfigs[i].blueSize           = 5;
		pConfigs[i].alphaSize          = 0;
		pConfigs[i].redMask            = 0x0000F800;
		pConfigs[i].greenMask          = 0x000007E0;
		pConfigs[i].blueMask           = 0x0000001F;
		pConfigs[i].alphaMask          = 0x00000000;
		if (accum) { /* Simulated in software */
		    pConfigs[i].accumRedSize   = 16;
		    pConfigs[i].accumGreenSize = 16;
		    pConfigs[i].accumBlueSize  = 16;
		    pConfigs[i].accumAlphaSize = 0;
		} else {
		    pConfigs[i].accumRedSize   = 0;
		    pConfigs[i].accumGreenSize = 0;
		    pConfigs[i].accumBlueSize  = 0;
		    pConfigs[i].accumAlphaSize = 0;
		}
		pConfigs[i].doubleBuffer       = TRUE;
		pConfigs[i].stereo             = FALSE;
		pConfigs[i].bufferSize         = 16;
		pConfigs[i].depthSize          = 16;
		if (stencil)
		    pConfigs[i].stencilSize    = 8; /* Simulated in software */
		else
		    pConfigs[i].stencilSize    = 0;
		pConfigs[i].auxBuffers         = 0;
		pConfigs[i].level              = 0;
		if (accum || stencil) {
		   pConfigs[i].visualRating    = GLX_SLOW_VISUAL_EXT;
		} else {
		   pConfigs[i].visualRating    = GLX_NONE_EXT;
		}
		pConfigs[i].transparentPixel   = GLX_NONE;
		pConfigs[i].transparentRed     = 0;
		pConfigs[i].transparentGreen   = 0;
		pConfigs[i].transparentBlue    = 0;
		pConfigs[i].transparentAlpha   = 0;
		pConfigs[i].transparentIndex   = 0;
		i++;
	    }
	}
	break;
    case 32:
	numConfigs = 1;
	if (RADEON_USE_ACCUM)   numConfigs *= 2;
	if (RADEON_USE_STENCIL) numConfigs *= 2;

	if (!(pConfigs
	      = (__GLXvisualConfig*)xcalloc(sizeof(__GLXvisualConfig),
					      numConfigs))) {
	    return FALSE;
	}
	if (!(pRADEONConfigs
	      = (RADEONConfigPrivPtr)xcalloc(sizeof(RADEONConfigPrivRec),
					       numConfigs))) {
	    xfree(pConfigs);
	    return FALSE;
	}
	if (!(pRADEONConfigPtrs
	      = (RADEONConfigPrivPtr*)xcalloc(sizeof(RADEONConfigPrivPtr),
						numConfigs))) {
	    xfree(pConfigs);
	    xfree(pRADEONConfigs);
	    return FALSE;
	}

	i = 0;
	for (accum = 0; accum <= RADEON_USE_ACCUM; accum++) {
	    for (stencil = 0; stencil <= RADEON_USE_STENCIL; stencil++) {
		pRADEONConfigPtrs[i] = &pRADEONConfigs[i];

		pConfigs[i].vid                = (VisualID)(-1);
		pConfigs[i].class              = -1;
		pConfigs[i].rgba               = TRUE;
		pConfigs[i].redSize            = 8;
		pConfigs[i].greenSize          = 8;
		pConfigs[i].blueSize           = 8;
		pConfigs[i].alphaSize          = 8;
		pConfigs[i].redMask            = 0x00FF0000;
		pConfigs[i].greenMask          = 0x0000FF00;
		pConfigs[i].blueMask           = 0x000000FF;
		pConfigs[i].alphaMask          = 0xFF000000;
		if (accum) { /* Simulated in software */
		    pConfigs[i].accumRedSize   = 16;
		    pConfigs[i].accumGreenSize = 16;
		    pConfigs[i].accumBlueSize  = 16;
		    pConfigs[i].accumAlphaSize = 16;
		} else {
		    pConfigs[i].accumRedSize   = 0;
		    pConfigs[i].accumGreenSize = 0;
		    pConfigs[i].accumBlueSize  = 0;
		    pConfigs[i].accumAlphaSize = 0;
		}
		pConfigs[i].doubleBuffer       = TRUE;
		pConfigs[i].stereo             = FALSE;
		pConfigs[i].bufferSize         = 24;
		if (stencil) {
		    pConfigs[i].depthSize      = 24;
		    pConfigs[i].stencilSize    = 8;
		} else {
		    pConfigs[i].depthSize      = 24;
		    pConfigs[i].stencilSize    = 0;
		}
		pConfigs[i].auxBuffers         = 0;
		pConfigs[i].level              = 0;
		if (accum || stencil) {
		   pConfigs[i].visualRating    = GLX_SLOW_VISUAL_EXT;
		} else {
		   pConfigs[i].visualRating    = GLX_NONE_EXT;
		}
		pConfigs[i].transparentPixel   = GLX_NONE;
		pConfigs[i].transparentRed     = 0;
		pConfigs[i].transparentGreen   = 0;
		pConfigs[i].transparentBlue    = 0;
		pConfigs[i].transparentAlpha   = 0;
		pConfigs[i].transparentIndex   = 0;
		i++;
	    }
	}
	break;
    }

    info->numVisualConfigs   = numConfigs;
    info->pVisualConfigs     = pConfigs;
    info->pVisualConfigsPriv = pRADEONConfigs;
    GlxSetVisualConfigs(numConfigs, pConfigs, (void**)pRADEONConfigPtrs);
    return TRUE;
}

/* Create the Radeon-specific context information */
static Bool RADEONCreateContext(ScreenPtr pScreen, VisualPtr visual,
				drmContext hwContext, void *pVisualConfigPriv,
				DRIContextType contextStore)
{
#ifdef PER_CONTEXT_SAREA
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RADEONDRIContextPtr ctx_info;

    ctx_info = (RADEONDRIContextPtr)contextStore;
    if (!ctx_info) return FALSE;

    if (drmAddMap(info->drmFD, 0,
		  info->perctx_sarea_size,
		  DRM_SHM,
		  DRM_REMOVABLE,
		  &ctx_info->sarea_handle) < 0) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "[dri] could not create private sarea for ctx id (%d)\n",
		   (int)hwContext);
        return FALSE;
    }

    if (drmAddContextPrivateMapping(info->drmFD, hwContext,
				    ctx_info->sarea_handle) < 0) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "[dri] could not associate private sarea to ctx id (%d)\n",
		   (int)hwContext);
        drmRmMap(info->drmFD, ctx_info->sarea_handle);
        return FALSE;
    }

    ctx_info->ctx_id = hwContext;
#endif
    return TRUE;
}

/* Destroy the Radeon-specific context information */
static void RADEONDestroyContext(ScreenPtr pScreen, drmContext hwContext,
				 DRIContextType contextStore)
{
#ifdef PER_CONTEXT_SAREA
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RADEONDRIContextPtr ctx_info;

    ctx_info = (RADEONDRIContextPtr) contextStore;
    if (!ctx_info) return;

    if (drmRmMap(info->drmFD, ctx_info->sarea_handle) < 0) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "[dri] could not remove private sarea for ctx id (%d)\n",
		   (int)hwContext);
    }
#endif
}

/* Called when the X server is woken up to allow the last client's
   context to be saved and the X server's context to be loaded.  This is
   not necessary for the Radeon since the client detects when it's
   context is not currently loaded and then load's it itself.  Since the
   registers to start and stop the CP are privileged, only the X server
   can start/stop the engine. */
static void RADEONEnterServer(ScreenPtr pScreen)
{
    ScrnInfoPtr   pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info  = RADEONPTR(pScrn);

    if (info->accel) info->accel->NeedToSync = TRUE;
}

/* Called when the X server goes to sleep to allow the X server's
   context to be saved and the last client's context to be loaded.  This
   is not necessary for the Radeon since the client detects when it's
   context is not currently loaded and then load's it itself.  Since the
   registers to start and stop the CP are privileged, only the X server
   can start/stop the engine. */
static void RADEONLeaveServer(ScreenPtr pScreen)
{
    ScrnInfoPtr   pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info  = RADEONPTR(pScrn);
    RING_LOCALS;

    /* The CP is always running, but if we've generated any CP commands
     * we must flush them to the kernel module now.
     */
    if (info->CPInUse) {
	RADEON_FLUSH_CACHE();
	RADEON_WAIT_UNTIL_IDLE();
	RADEONCPReleaseIndirect(pScrn);

	info->CPInUse = FALSE;
    }
}

/* Contexts can be swapped by the X server if necessary.  This callback
   is currently only used to perform any functions necessary when
   entering or leaving the X server, and in the future might not be
   necessary. */
static void RADEONDRISwapContext(ScreenPtr pScreen, DRISyncType syncType,
				 DRIContextType oldContextType,
				 void *oldContext,
				 DRIContextType newContextType,
				 void *newContext)
{
    if ((syncType==DRI_3D_SYNC) && (oldContextType==DRI_2D_CONTEXT) &&
	(newContextType==DRI_2D_CONTEXT)) { /* Entering from Wakeup */
	RADEONEnterServer(pScreen);
    }
    if ((syncType==DRI_2D_SYNC) && (oldContextType==DRI_NO_CONTEXT) &&
	(newContextType==DRI_2D_CONTEXT)) { /* Exiting from Block Handler */
	RADEONLeaveServer(pScreen);
    }
}

/* The Radeon has depth tiling on all the time, so we have to convert
 * the x,y coordinates into the memory bus address (mba) in the same
 * manner as the engine.  In each case, the linear block address (ba)
 * is calculated, and then wired with x and y to produce the final
 * memory address.
 */
static CARD32 radeon_mba_z16(RADEONInfoPtr info,
			     int x, int y)
{
    CARD32 pitch = info->frontPitch;
    CARD32 ba, address = 0;			/* a[0]    = 0           */

    ba = (y / 16) * (pitch / 32) + (x / 32);

    address |= (x & 0x7) << 1;			/* a[1..3] = x[0..2]     */
    address |= (y & 0x7) << 4;			/* a[4..6] = y[0..2]     */
    address |= (x & 0x8) << 4;			/* a[7]    = x[3]        */
    address |= (ba & 0x3) << 8;			/* a[8..9] = ba[0..1]    */
    address |= (y & 0x8) << 7;			/* a[10]   = y[3]        */
    address |= ((x & 0x10) ^ (y & 0x10)) << 7;	/* a[11]   = x[4] ^ y[4] */
    address |= (ba & ~0x3u) << 10;		/* a[12..] = ba[2..]     */

    return address;
}

static CARD32 radeon_mba_z32(RADEONInfoPtr info,
			     int x, int y)
{
    CARD32 pitch = info->frontPitch;
    CARD32 ba, address = 0;			/* a[0..1] = 0           */

    ba = (y / 16) * (pitch / 16) + (x / 16);

    address |= (x & 0x7) << 2;			/* a[2..4] = x[0..2]     */
    address |= (y & 0x3) << 5;			/* a[5..6] = y[0..1]     */
    address |=
	(((x & 0x10) >> 2) ^ (y & 0x4)) << 5;	/* a[7]    = x[4] ^ y[2] */
    address |= (ba & 0x3) << 8;			/* a[8..9] = ba[0..1]    */

    address |= (y & 0x8) << 7;			/* a[10]   = y[3]        */
    address |=
	(((x & 0x8) << 1) ^ (y & 0x10)) << 7;	/* a[11]   = x[3] ^ y[4] */
    address |= (ba & ~0x3u) << 10;		/* a[12..] = ba[2..]     */

    return address;
}

/* 16-bit depth buffer functions */
#define WRITE_DEPTH16(_x, _y, d)					\
    *(CARD16 *)(pointer)(buf + radeon_mba_z16(info, (_x), (_y))) = (d)

#define READ_DEPTH16(d, _x, _y)						\
    (d) = *(CARD16 *)(pointer)(buf + radeon_mba_z16(info, (_x), (_y)))

/* 24 bit depth, 8 bit stencil depthbuffer functions */
#define WRITE_DEPTH32(_x, _y, d)					\
do {									\
    CARD32 tmp =							\
	*(CARD32 *)(pointer)(buf + radeon_mba_z32(info, (_x), (_y)));	\
    tmp &= 0xff000000;							\
    tmp |= ((d) & 0x00ffffff);						\
    *(CARD32 *)(pointer)(buf + radeon_mba_z32(info, (_x), (_y))) = tmp;	\
} while (0)

#define READ_DEPTH32(d, _x, _y)						\
    d = *(CARD32 *)(pointer)(buf + radeon_mba_z32(info, (_x), (_y))) & 0x00ffffff

/* Screen to screen copy of data in the depth buffer */
static void RADEONScreenToScreenCopyDepth(ScrnInfoPtr pScrn,
					  int xa, int ya,
					  int xb, int yb,
					  int w, int h)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    int           xstart, xend, xdir;
    int           ystart, yend, ydir;
    int           x, y, d;
    unsigned char *buf = info->FB + info->depthOffset;

    if (xa < xb) xdir = -1, xstart = w-1, xend = 0;
    else         xdir =  1, xstart = 0,   xend = w-1;

    if (ya < yb) ydir = -1, ystart = h-1, yend = 0;
    else         ydir =  1, ystart = 0,   yend = h-1;

    switch (pScrn->bitsPerPixel) {
    case 16:
	for (x = xstart; x != xend; x += xdir) {
	    for (y = ystart; y != yend; y += ydir) {
		READ_DEPTH16(d, xa+x, ya+y);
		WRITE_DEPTH16(xb+x, yb+y, d);
	    }
	}
	break;
    case 32:
	for (x = xstart; x != xend; x += xdir) {
	    for (y = ystart; y != yend; y += ydir) {
		READ_DEPTH32(d, xa+x, ya+y);
		WRITE_DEPTH32(xb+x, yb+y, d);
	    }
	}
	break;
    default: break;
    }
}

/* Initialize the state of the back and depth buffers. */
static void RADEONDRIInitBuffers(WindowPtr pWin, RegionPtr prgn, CARD32 indx)
{
    /* FIXME: This routine needs to have acceleration turned on */
    ScreenPtr          pScreen = pWin->drawable.pScreen;
    ScrnInfoPtr        pScrn   = xf86Screens[pScreen->myNum];
    RADEONInfoPtr      info    = RADEONPTR(pScrn);
    RADEONSAREAPrivPtr pSAREAPriv;
    BoxPtr             pbox;
    int                nbox;
    unsigned int       color, depth;
    unsigned int       color_mask, depth_mask;

    /* FIXME: This should be based on the __GLXvisualConfig info */
    color = 0;
    switch (pScrn->bitsPerPixel) {
    case 16:
       depth = 0x0000ffff;
       color_mask = 0x0000ffff;
       depth_mask = 0xffffffff;
       break;
    case 32:
       depth = 0x00ffffff;
       color_mask = 0xffffffff;
       depth_mask = 0xffffffff;
       break;
    default:
       depth = 0x00000000;
       color_mask = 0x00000000;
       depth_mask = 0x00000000;
       break;
    }

    /* FIXME: Copy XAAPaintWindow() and use REGION_TRANSLATE() */
    /* FIXME: Only initialize the back and depth buffers for contexts
       that request them */

    FLUSH_RING();

    pSAREAPriv = (RADEONSAREAPrivPtr)DRIGetSAREAPrivate(pScreen);

    pbox = REGION_RECTS(prgn);
    nbox = REGION_NUM_RECTS(prgn);

    for (; nbox; nbox--, pbox++) {
	int ret;

	/* drmRadeonClear uses the clip rects to draw instead of the
           rect passed to it; however, it uses the rect for the depth
           clears */
	pSAREAPriv->boxes[0].x1 = pbox->x1;
	pSAREAPriv->boxes[0].x2 = pbox->x2;
	pSAREAPriv->boxes[0].y1 = pbox->y1;
	pSAREAPriv->boxes[0].y2 = pbox->y2;
	pSAREAPriv->nbox = 1;

	ret = drmRadeonClear(info->drmFD,
			     DRM_RADEON_BACK | DRM_RADEON_DEPTH,
			     color, depth, color_mask, depth_mask,
			     pSAREAPriv->boxes, pSAREAPriv->nbox);
	if (ret) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "[dri] DRIInitBuffers timed out, resetting engine...\n");
	    RADEONEngineReset(pScrn);
	    RADEONEngineRestore(pScrn);
	    RADEONCP_RESET(pScrn, info);
	    RADEONCP_START(pScrn, info);
	    return;
	}
    }

    /* Mark the X server as the last context owner */
    pSAREAPriv->ctxOwner = DRIGetContext(pScreen);

    RADEONSelectBuffer(pScrn, RADEON_FRONT);
    info->accel->NeedToSync = TRUE;
}

/* Copy the back and depth buffers when the X server moves a window.
 *
 * This routine is a modified form of XAADoBitBlt with the calls to
 * ScreenToScreenBitBlt built in. My routine has the prgnSrc as source
 * instead of destination. My origin is upside down so the ydir cases
 * are reversed.
 */
static void RADEONDRIMoveBuffers(WindowPtr pParent, DDXPointRec ptOldOrg,
				 RegionPtr prgnSrc, CARD32 indx)
{
    ScreenPtr     pScreen  = pParent->drawable.pScreen;
    ScrnInfoPtr   pScrn    = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info     = RADEONPTR(pScrn);

    BoxPtr        pboxTmp, pboxNext, pboxBase;
    DDXPointPtr   pptTmp;
    int           xdir, ydir;

    int           screenwidth = pScrn->virtualX;
    int           screenheight = pScrn->virtualY;

    BoxPtr        pbox     = REGION_RECTS(prgnSrc);
    int           nbox     = REGION_NUM_RECTS(prgnSrc);

    BoxPtr        pboxNew1 = NULL;
    BoxPtr        pboxNew2 = NULL;
    DDXPointPtr   pptNew1  = NULL;
    DDXPointPtr   pptNew2  = NULL;
    DDXPointPtr   pptSrc   = &ptOldOrg;

    int           dx       = pParent->drawable.x - ptOldOrg.x;
    int           dy       = pParent->drawable.y - ptOldOrg.y;

    /* If the copy will overlap in Y, reverse the order */
    if (dy > 0) {
	ydir = -1;

	if (nbox > 1) {
	    /* Keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
	    if (!pboxNew1) return;
	    pptNew1 = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
	    if (!pptNew1) {
		DEALLOCATE_LOCAL(pboxNew1);
		return;
	    }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox) {
		while ((pboxNext >= pbox) && (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
		pboxTmp = pboxNext+1;
		pptTmp  = pptSrc + (pboxTmp - pbox);
		while (pboxTmp <= pboxBase) {
		    *pboxNew1++ = *pboxTmp++;
		    *pptNew1++  = *pptTmp++;
		}
		pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox      = pboxNew1;
	    pptNew1  -= nbox;
	    pptSrc    = pptNew1;
	}
    } else {
	/* No changes required */
	ydir = 1;
    }

    /* If the regions will overlap in X, reverse the order */
    if (dx > 0) {
	xdir = -1;

	if (nbox > 1) {
	    /* reverse order of rects in each band */
	    pboxNew2 = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
	    pptNew2  = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
	    if (!pboxNew2 || !pptNew2) {
		DEALLOCATE_LOCAL(pptNew2);
		DEALLOCATE_LOCAL(pboxNew2);
		DEALLOCATE_LOCAL(pptNew1);
		DEALLOCATE_LOCAL(pboxNew1);
		return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox) {
		while ((pboxNext < pbox+nbox)
		       && (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
		pboxTmp = pboxNext;
		pptTmp  = pptSrc + (pboxTmp - pbox);
		while (pboxTmp != pboxBase) {
		    *pboxNew2++ = *--pboxTmp;
		    *pptNew2++  = *--pptTmp;
		}
		pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox      = pboxNew2;
	    pptNew2  -= nbox;
	    pptSrc    = pptNew2;
	}
    } else {
	/* No changes are needed */
	xdir = 1;
    }

    (*info->accel->SetupForScreenToScreenCopy)(pScrn, xdir, ydir, GXcopy,
					       (CARD32)(-1), -1);

    for (; nbox-- ; pbox++) {
	int xa    = pbox->x1;
	int ya    = pbox->y1;
	int destx = xa + dx;
	int desty = ya + dy;
	int w     = pbox->x2 - xa + 1;
	int h     = pbox->y2 - ya + 1;

	if (destx < 0)                xa -= destx, w += destx, destx = 0;
	if (desty < 0)                ya -= desty, h += desty, desty = 0;
	if (destx + w > screenwidth)  w = screenwidth  - destx;
	if (desty + h > screenheight) h = screenheight - desty;

	if (w <= 0) continue;
	if (h <= 0) continue;

	RADEONSelectBuffer(pScrn, RADEON_BACK);
	(*info->accel->SubsequentScreenToScreenCopy)(pScrn,
						     xa, ya,
						     destx, desty,
						     w, h);
	RADEONSelectBuffer(pScrn, RADEON_DEPTH);

	if (info->depthMoves)
	    RADEONScreenToScreenCopyDepth(pScrn,
					  xa, ya,
					  destx, desty,
					  w, h);
    }

    RADEONSelectBuffer(pScrn, RADEON_FRONT);

    DEALLOCATE_LOCAL(pptNew2);
    DEALLOCATE_LOCAL(pboxNew2);
    DEALLOCATE_LOCAL(pptNew1);
    DEALLOCATE_LOCAL(pboxNew1);

    info->accel->NeedToSync = TRUE;
}

/* Initialize the AGP state.  Request memory for use in AGP space, and
   initialize the Radeon registers to point to that memory. */
static Bool RADEONDRIAgpInit(RADEONInfoPtr info, ScreenPtr pScreen)
{
    unsigned char *RADEONMMIO = info->MMIO;
    unsigned long mode;
    unsigned int  vendor, device;
    int           ret;
    int           s, l;

    if (drmAgpAcquire(info->drmFD) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] AGP not available\n");
	return FALSE;
    }

				/* Modify the mode if the default mode is
                                   not appropriate for this particular
                                   combination of graphics card and AGP
                                   chipset. */

    mode   = drmAgpGetMode(info->drmFD);	/* Default mode */
    vendor = drmAgpVendorId(info->drmFD);
    device = drmAgpDeviceId(info->drmFD);

    mode &= ~RADEON_AGP_MODE_MASK;
    switch (info->agpMode) {
    case 4:          mode |= RADEON_AGP_4X_MODE;
    case 2:          mode |= RADEON_AGP_2X_MODE;
    case 1: default: mode |= RADEON_AGP_1X_MODE;
    }

    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] Mode 0x%08lx [AGP 0x%04x/0x%04x; Card 0x%04x/0x%04x]\n",
	       mode, vendor, device,
	       info->PciInfo->vendor,
	       info->PciInfo->chipType);

    if (drmAgpEnable(info->drmFD, mode) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] AGP not enabled\n");
	drmAgpRelease(info->drmFD);
	return FALSE;
    }

    info->agpOffset = 0;

    if ((ret = drmAgpAlloc(info->drmFD, info->agpSize*1024*1024, 0, NULL,
			   &info->agpMemHandle)) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] Out of memory (%d)\n", ret);
	drmAgpRelease(info->drmFD);
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] %d kB allocated with handle 0x%08x\n",
	       info->agpSize*1024, info->agpMemHandle);

    if (drmAgpBind(info->drmFD,
		   info->agpMemHandle, info->agpOffset) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] Could not bind\n");
	drmAgpFree(info->drmFD, info->agpMemHandle);
	drmAgpRelease(info->drmFD);
	return FALSE;
    }

				/* Initialize the CP ring buffer data */
    info->ringStart       = info->agpOffset;
    info->ringMapSize     = info->ringSize*1024*1024 + DRM_PAGE_SIZE;
    info->ringSizeLog2QW  = RADEONMinBits(info->ringSize*1024*1024/8)-1;

    info->ringReadOffset  = info->ringStart + info->ringMapSize;
    info->ringReadMapSize = DRM_PAGE_SIZE;

				/* Reserve space for vertex/indirect buffers */
    info->bufStart        = info->ringReadOffset + info->ringReadMapSize;
    info->bufMapSize      = info->bufSize*1024*1024;

				/* Reserve the rest for AGP textures */
    info->agpTexStart     = info->bufStart + info->bufMapSize;
    s = (info->agpSize*1024*1024 - info->agpTexStart);
    l = RADEONMinBits((s-1) / RADEON_NR_TEX_REGIONS);
    if (l < RADEON_LOG_TEX_GRANULARITY) l = RADEON_LOG_TEX_GRANULARITY;
    info->agpTexMapSize   = (s >> l) << l;
    info->log2AGPTexGran  = l;

    if (drmAddMap(info->drmFD, info->ringStart, info->ringMapSize,
		  DRM_AGP, DRM_READ_ONLY, &info->ringHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not add ring mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] ring handle = 0x%08lx\n", info->ringHandle);

    if (drmMap(info->drmFD, info->ringHandle, info->ringMapSize,
	       (drmAddressPtr)&info->ring) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[agp] Could not map ring\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] Ring mapped at 0x%08lx\n",
	       (unsigned long)info->ring);

    if (drmAddMap(info->drmFD, info->ringReadOffset, info->ringReadMapSize,
		  DRM_AGP, DRM_READ_ONLY, &info->ringReadPtrHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not add ring read ptr mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] ring read ptr handle = 0x%08lx\n",
	       info->ringReadPtrHandle);

    if (drmMap(info->drmFD, info->ringReadPtrHandle, info->ringReadMapSize,
	       (drmAddressPtr)&info->ringReadPtr) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not map ring read ptr\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] Ring read ptr mapped at 0x%08lx\n",
	       (unsigned long)info->ringReadPtr);

    if (drmAddMap(info->drmFD, info->bufStart, info->bufMapSize,
		  DRM_AGP, 0, &info->bufHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not add vertex/indirect buffers mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] vertex/indirect buffers handle = 0x%08lx\n",
	       info->bufHandle);

    if (drmMap(info->drmFD, info->bufHandle, info->bufMapSize,
	       (drmAddressPtr)&info->buf) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not map vertex/indirect buffers\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] Vertex/indirect buffers mapped at 0x%08lx\n",
	       (unsigned long)info->buf);

    if (drmAddMap(info->drmFD, info->agpTexStart, info->agpTexMapSize,
		  DRM_AGP, 0, &info->agpTexHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not add AGP texture map mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] AGP texture map handle = 0x%08lx\n",
	       info->agpTexHandle);

    if (drmMap(info->drmFD, info->agpTexHandle, info->agpTexMapSize,
	       (drmAddressPtr)&info->agpTex) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[agp] Could not map AGP texture map\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[agp] AGP Texture map mapped at 0x%08lx\n",
	       (unsigned long)info->agpTex);

				/* Initialize Radeon's AGP registers */
    /* Ring buffer is at AGP offset 0 */
    OUTREG(RADEON_AGP_BASE, info->ringHandle);

				/* Enable bus mastering in PCI config
				   space */
    xf86EnablePciBusMaster(info->PciInfo, TRUE);

    return TRUE;
}

#if defined(PCIGART_ENABLED)
/* Initialize the PCIGART state.  Request memory for use in PCI space,
   and initialize the Radeon registers to point to that memory. */
static Bool RADEONDRIPciInit(RADEONInfoPtr info, ScreenPtr pScreen)
{
    int ret;
    int flags;

    info->agpOffset = 0;

    ret = drmScatterGatherAlloc(info->drmFD, info->agpSize*1024*1024,
				&info->pciMemHandle);
    if (ret < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[pci] Out of memory (%d)\n", ret);
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] %d kB allocated with handle 0x%08x\n",
	       info->agpSize*1024, info->pciMemHandle);

				/* Initialize the CCE ring buffer data */
    info->ringStart       = info->agpOffset;
    info->ringMapSize     = info->ringSize*1024*1024 + DRM_PAGE_SIZE;
    info->ringSizeLog2QW  = RADEONMinBits(info->ringSize*1024*1024/8)-1;

    info->ringReadOffset  = info->ringStart + info->ringMapSize;
    info->ringReadMapSize = DRM_PAGE_SIZE;

				/* Reserve space for vertex/indirect buffers */
    info->bufStart        = info->ringReadOffset + info->ringReadMapSize;
    info->bufMapSize      = info->bufSize*1024*1024;

    flags = DRM_READ_ONLY | DRM_LOCKED | DRM_KERNEL;

    if (drmAddMap(info->drmFD, info->ringStart, info->ringMapSize,
		  DRM_SCATTER_GATHER, flags, &info->ringHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[pci] Could not add ring mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] ring handle = 0x%08lx\n", info->ringHandle);

    if (drmMap(info->drmFD, info->ringHandle, info->ringMapSize,
	       (drmAddressPtr)&info->ring) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[pci] Could not map ring\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] Ring mapped at 0x%08lx\n",
	       (unsigned long)info->ring);
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] Ring contents 0x%08lx\n",
	       *(unsigned long *)info->ring);

    if (drmAddMap(info->drmFD, info->ringReadOffset, info->ringReadMapSize,
		  DRM_SCATTER_GATHER, flags, &info->ringReadPtrHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[pci] Could not add ring read ptr mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] ring read ptr handle = 0x%08lx\n",
	       info->ringReadPtrHandle);

    if (drmMap(info->drmFD, info->ringReadPtrHandle, info->ringReadMapSize,
	       (drmAddressPtr)&info->ringReadPtr) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[pci] Could not map ring read ptr\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] Ring read ptr mapped at 0x%08lx\n",
	       (unsigned long)info->ringReadPtr);
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] Ring read ptr contents 0x%08lx\n",
	       *(unsigned long *)info->ringReadPtr);

    if (drmAddMap(info->drmFD, info->bufStart, info->bufMapSize,
		  DRM_SCATTER_GATHER, 0, &info->bufHandle) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[pci] Could not add vertex/indirect buffers mapping\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] vertex/indirect buffers handle = 0x%08lx\n",
	       info->bufHandle);

    if (drmMap(info->drmFD, info->bufHandle, info->bufMapSize,
	       (drmAddressPtr)&info->buf) < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[pci] Could not map vertex/indirect buffers\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] Vertex/indirect buffers mapped at 0x%08lx\n",
	       (unsigned long)info->buf);
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[pci] Vertex/indirect buffers contents 0x%08lx\n",
	       *(unsigned long *)info->buf);

    return TRUE;
}
#endif

/* Add a map for the MMIO registers that will be accessed by any
   DRI-based clients. */
static Bool RADEONDRIMapInit(RADEONInfoPtr info, ScreenPtr pScreen)
{
				/* Map registers */
    info->registerSize = RADEON_MMIOSIZE;
    if (drmAddMap(info->drmFD, info->MMIOAddr, info->registerSize,
		  DRM_REGISTERS, DRM_READ_ONLY, &info->registerHandle) < 0) {
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[drm] register handle = 0x%08lx\n", info->registerHandle);

    return TRUE;
}

/* Initialize the kernel data structures. */
static int RADEONDRIKernelInit(RADEONInfoPtr info, ScreenPtr pScreen)
{
    ScrnInfoPtr   pScrn       = xf86Screens[pScreen->myNum];
    int           cpp         = info->CurrentLayout.pixel_bytes;
    drmRadeonInit drmInfo;

    drmInfo.sarea_priv_offset   = sizeof(XF86DRISAREARec);
    drmInfo.is_pci              = info->IsPCI;
    drmInfo.cp_mode             = info->CPMode;
    drmInfo.agp_size            = info->agpSize*1024*1024;
    drmInfo.ring_size           = info->ringSize*1024*1024;
    drmInfo.usec_timeout        = info->CPusecTimeout;

    drmInfo.fb_bpp              = info->CurrentLayout.pixel_code;
    drmInfo.depth_bpp           = info->CurrentLayout.pixel_code;

    drmInfo.front_offset        = info->frontOffset;
    drmInfo.front_pitch         = info->frontPitch * cpp;
    drmInfo.back_offset         = info->backOffset;
    drmInfo.back_pitch          = info->backPitch * cpp;
    drmInfo.depth_offset        = info->depthOffset;
    drmInfo.depth_pitch         = info->depthPitch * cpp;

    drmInfo.fb_offset           = info->LinearAddr;
    drmInfo.mmio_offset         = info->registerHandle;
    drmInfo.ring_offset         = info->ringHandle;
    drmInfo.ring_rptr_offset    = info->ringReadPtrHandle;
    drmInfo.buffers_offset      = info->bufHandle;
    drmInfo.agp_textures_offset = info->agpTexHandle;

    if (drmRadeonInitCP(info->drmFD, &drmInfo) < 0) return FALSE;

    /* drmRadeonInitCP does an engine reset, which resets some engine
       registers back to their default values, so we need to restore
       those engine register here. */
    RADEONEngineRestore(pScrn);

    return TRUE;
}

/* Add a map for the vertex buffers that will be accessed by any
   DRI-based clients. */
static Bool RADEONDRIBufInit(RADEONInfoPtr info, ScreenPtr pScreen)
{
				/* Initialize vertex buffers */
    if (info->IsPCI) {
#if !defined(PCIGART_ENABLED)
	return TRUE;
#else
	info->bufNumBufs = drmAddBufs(info->drmFD,
				      info->bufMapSize / RADEON_BUFFER_SIZE,
				      RADEON_BUFFER_SIZE,
				      DRM_SG_BUFFER,
				      info->bufStart);
#endif
    } else {
	info->bufNumBufs = drmAddBufs(info->drmFD,
				      info->bufMapSize / RADEON_BUFFER_SIZE,
				      RADEON_BUFFER_SIZE,
				      DRM_AGP_BUFFER,
				      info->bufStart);
    }
    if (info->bufNumBufs <= 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[drm] Could not create vertex/indirect buffers list\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[drm] Added %d %d byte vertex/indirect buffers\n",
	       info->bufNumBufs, RADEON_BUFFER_SIZE);

    if (!(info->buffers = drmMapBufs(info->drmFD))) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[drm] Failed to map vertex/indirect buffers list\n");
	return FALSE;
    }
    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[drm] Mapped %d vertex/indirect buffers\n",
	       info->buffers->count);

    return TRUE;
}

/* Initialize the CP state, and start the CP (if used by the X server) */
static void RADEONDRICPInit(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

				/* Turn on bus mastering */
    info->BusCntl &= ~RADEON_BUS_MASTER_DIS;

				/* Make sure the CP is on for the X server */
    RADEONCP_START(pScrn, info);
    RADEONSelectBuffer(pScrn, RADEON_FRONT);
}

/* Initialize the DRI specific hardware state stored in the SAREA.
   Currently, this involves setting up the 3D hardware state. */
static void RADEONDRISAREAInit(ScreenPtr pScreen,
			       RADEONSAREAPrivPtr pSAREAPriv)
{
    ScrnInfoPtr           pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr         info  = RADEONPTR(pScrn);
    radeon_context_regs_t *ctx;
    radeon_texture_regs_t *tex;
    CARD32                color_fmt, depth_fmt;
    int                   i;

    switch (info->CurrentLayout.pixel_code) {
    case 16:
	color_fmt = RADEON_COLOR_FORMAT_RGB565;
	depth_fmt = RADEON_DEPTH_FORMAT_16BIT_INT_Z;
	break;
    case 32:
	color_fmt = RADEON_COLOR_FORMAT_ARGB8888;
	depth_fmt = RADEON_DEPTH_FORMAT_24BIT_INT_Z;
	break;
    default:
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[dri] RADEONDRISAREAInit failed: Unsupported depth (%d bpp).  Disabling DRI.\n",
		   info->CurrentLayout.pixel_code);
	return;
    }

    /* Initialize the context state */
    ctx = &pSAREAPriv->ContextState;

    ctx->pp_misc = (RADEON_ALPHA_TEST_PASS |
		    RADEON_CHROMA_FUNC_FAIL |
		    RADEON_CHROMA_KEY_NEAREST |
		    RADEON_SHADOW_FUNC_EQUAL |
		    RADEON_SHADOW_PASS_1 |
		    RADEON_RIGHT_HAND_CUBE_OGL);

    ctx->pp_fog_color = ((0x00000000 & RADEON_FOG_COLOR_MASK) |
			 RADEON_FOG_VERTEX |
			 RADEON_FOG_USE_DEPTH);

    ctx->re_solid_color = 0x00000000;

    ctx->rb3d_blendcntl = (RADEON_SRC_BLEND_GL_ONE |
			   RADEON_DST_BLEND_GL_ZERO );

    ctx->rb3d_depthoffset = info->depthOffset;

    ctx->rb3d_depthpitch = ((info->depthPitch & RADEON_DEPTHPITCH_MASK) |
			    RADEON_DEPTH_ENDIAN_NO_SWAP);

    ctx->rb3d_zstencilcntl = (depth_fmt |
			      RADEON_Z_TEST_LESS |
			      RADEON_STENCIL_TEST_ALWAYS |
			      RADEON_STENCIL_S_FAIL_KEEP |
			      RADEON_STENCIL_ZPASS_KEEP |
			      RADEON_STENCIL_ZFAIL_KEEP |
			      RADEON_Z_WRITE_ENABLE);

    ctx->pp_cntl = (RADEON_SCISSOR_ENABLE |
		    RADEON_ANTI_ALIAS_NONE);

    ctx->rb3d_cntl = (RADEON_PLANE_MASK_ENABLE |
		      color_fmt |
		      RADEON_ZBLOCK16);

    ctx->rb3d_coloroffset = (info->backOffset & RADEON_COLOROFFSET_MASK);

    ctx->re_width_height = ((0x7ff << RADEON_RE_WIDTH_SHIFT) |
			    (0x7ff << RADEON_RE_HEIGHT_SHIFT));

    ctx->rb3d_colorpitch = ((info->backPitch & RADEON_COLORPITCH_MASK) |
			    RADEON_COLOR_ENDIAN_NO_SWAP);

    ctx->se_cntl = (RADEON_FFACE_CULL_CW |
		    RADEON_BFACE_SOLID |
		    RADEON_FFACE_SOLID |
		    RADEON_FLAT_SHADE_VTX_LAST |
		    RADEON_DIFFUSE_SHADE_GOURAUD |
		    RADEON_ALPHA_SHADE_GOURAUD |
		    RADEON_SPECULAR_SHADE_GOURAUD |
		    RADEON_FOG_SHADE_GOURAUD |
		    RADEON_VPORT_XY_XFORM_ENABLE |
		    RADEON_VTX_PIX_CENTER_OGL |
		    RADEON_ROUND_MODE_TRUNC |
		    RADEON_ROUND_PREC_8TH_PIX);

    ctx->se_coord_fmt = (RADEON_VTX_XY_PRE_MULT_1_OVER_W0 |
			 RADEON_VTX_Z_PRE_MULT_1_OVER_W0 |
			 RADEON_TEX1_W_ROUTING_USE_Q1);

    ctx->re_line_pattern = ((0x0000 & RADEON_LINE_PATTERN_MASK) |
			    (0 << RADEON_LINE_REPEAT_COUNT_SHIFT) |
			    (0 << RADEON_LINE_PATTERN_START_SHIFT) |
			    RADEON_LINE_PATTERN_LITTLE_BIT_ORDER);

    ctx->re_line_state = ((0 << RADEON_LINE_CURRENT_PTR_SHIFT) |
			  (0 << RADEON_LINE_CURRENT_COUNT_SHIFT));

    ctx->se_line_width = 0x0000000;

    ctx->pp_lum_matrix = 0x00000000;

    ctx->pp_rot_matrix_0 = 0x00000000;
    ctx->pp_rot_matrix_1 = 0x00000000;

    ctx->rb3d_stencilrefmask =
	(CARD32)((0x000 << RADEON_STENCIL_REF_SHIFT) |
		 (0x0ff << RADEON_STENCIL_MASK_SHIFT) |
		 (0x0ff << RADEON_STENCIL_WRITEMASK_SHIFT));

    ctx->rb3d_ropcntl   = 0x00000000;
    ctx->rb3d_planemask = 0xffffffff;

    ctx->se_vport_xscale  = 0x00000000;
    ctx->se_vport_xoffset = 0x00000000;
    ctx->se_vport_yscale  = 0x00000000;
    ctx->se_vport_yoffset = 0x00000000;
    ctx->se_vport_zscale  = 0x00000000;
    ctx->se_vport_zoffset = 0x00000000;

    ctx->se_cntl_status = (RADEON_VC_NO_SWAP |
			   RADEON_TCL_BYPASS);

#ifdef TCL_ENABLE
   /* FIXME: Obviously these need to be properly initialized */
    ctx->se_tcl_material_emmissive.red   = 0x00000000;
    ctx->se_tcl_material_emmissive.green = 0x00000000;
    ctx->se_tcl_material_emmissive.blue  = 0x00000000;
    ctx->se_tcl_material_emmissive.alpha = 0x00000000;

    ctx->se_tcl_material_ambient.red     = 0x00000000;
    ctx->se_tcl_material_ambient.green   = 0x00000000;
    ctx->se_tcl_material_ambient.blue    = 0x00000000;
    ctx->se_tcl_material_ambient.alpha   = 0x00000000;

    ctx->se_tcl_material_diffuse.red     = 0x00000000;
    ctx->se_tcl_material_diffuse.green   = 0x00000000;
    ctx->se_tcl_material_diffuse.blue    = 0x00000000;
    ctx->se_tcl_material_diffuse.alpha   = 0x00000000;

    ctx->se_tcl_material_specular.red    = 0x00000000;
    ctx->se_tcl_material_specular.green  = 0x00000000;
    ctx->se_tcl_material_specular.blue   = 0x00000000;
    ctx->se_tcl_material_specular.alpha  = 0x00000000;

    ctx->se_tcl_shininess                = 0x00000000;
    ctx->se_tcl_output_vtx_fmt           = 0x00000000;
    ctx->se_tcl_output_vtx_sel           = 0x00000000;
    ctx->se_tcl_matrix_select_0          = 0x00000000;
    ctx->se_tcl_matrix_select_1          = 0x00000000;
    ctx->se_tcl_ucp_vert_blend_ctl       = 0x00000000;
    ctx->se_tcl_texture_proc_ctl         = 0x00000000;
    ctx->se_tcl_light_model_ctl          = 0x00000000;
    for ( i = 0 ; i < 4 ; i++ ) {
	ctx->se_tcl_per_light_ctl[i]     = 0x00000000;
    }
#endif

    ctx->re_top_left = ((0 << RADEON_RE_LEFT_SHIFT) |
			(0 << RADEON_RE_TOP_SHIFT) );

    ctx->re_misc = ((0 << RADEON_STIPPLE_X_OFFSET_SHIFT) |
		    (0 << RADEON_STIPPLE_Y_OFFSET_SHIFT) |
		    RADEON_STIPPLE_LITTLE_BIT_ORDER);

    /* Initialize the texture state */
    for (i = 0; i < RADEON_MAX_TEXTURE_UNITS; i++) {
	tex = &pSAREAPriv->TexState[i];

	tex->pp_txfilter     = 0x00000000;
	tex->pp_txformat     = 0x00000000;
	tex->pp_txoffset     = 0x00000000;
	tex->pp_txcblend     = 0x00000000;
	tex->pp_txablend     = 0x00000000;
	tex->pp_tfactor      = 0x00000000;
	tex->pp_border_color = 0x00000000;
    }

    /* Mark the context as dirty */
    pSAREAPriv->dirty    = RADEON_UPLOAD_ALL;

    /* Mark the X server as the last context owner */
    pSAREAPriv->ctxOwner = DRIGetContext(pScreen);
}

/* Initialize the screen-specific data structures for the DRI and the
   Radeon.  This is the main entry point to the device-specific
   initialization code.  It calls device-independent DRI functions to
   create the DRI data structures and initialize the DRI state. */
Bool RADEONDRIScreenInit(ScreenPtr pScreen)
{
    ScrnInfoPtr   pScrn   = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info    = RADEONPTR(pScrn);
    DRIInfoPtr    pDRIInfo;
    RADEONDRIPtr  pRADEONDRI;
    int           major, minor, patch;
    drmVersionPtr version;

    /* Check that the GLX, DRI, and DRM modules have been loaded by testing
     * for known symbols in each module. */
    if (!xf86LoaderCheckSymbol("GlxSetVisualConfigs")) return FALSE;
    if (!xf86LoaderCheckSymbol("DRIScreenInit"))       return FALSE;
    if (!xf86LoaderCheckSymbol("drmAvailable"))        return FALSE;
    if (!xf86LoaderCheckSymbol("DRIQueryVersion")) {
      xf86DrvMsg(pScreen->myNum, X_ERROR,
                 "[dri] RADEONDRIScreenInit failed (libdri.a too old)\n");
      return FALSE;
    }

    /* Check the DRI version */
    DRIQueryVersion(&major, &minor, &patch);
    if (major != 4 || minor < 0) {
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[dri] RADEONDRIScreenInit failed because of a version mismatch.\n"
		   "[dri] libDRI version is %d.%d.%d but version 4.0.x is needed.\n"
		   "[dri] Disabling DRI.\n",
		   major, minor, patch);
	return FALSE;
    }

    switch (info->CurrentLayout.pixel_code) {
    case 8:
    case 15:
    case 24:
	/* These modes are not supported (yet). */
	xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[dri] RADEONInitVisualConfigs failed (depth %d not supported).  "
		   "Disabling DRI.\n", info->CurrentLayout.pixel_code);
	return FALSE;

	/* Only 16 and 32 color depths are supports currently. */
    case 16:
    case 32:
	break;
    }

    /* Create the DRI data structure, and fill it in before calling the
       DRIScreenInit(). */
    if (!(pDRIInfo = DRICreateInfoRec())) return FALSE;

    info->pDRIInfo                       = pDRIInfo;
    pDRIInfo->drmDriverName              = RADEON_DRIVER_NAME;
    pDRIInfo->clientDriverName           = RADEON_DRIVER_NAME;
    pDRIInfo->busIdString                = xalloc(64);
    sprintf(pDRIInfo->busIdString,
	    "PCI:%d:%d:%d",
	    info->PciInfo->bus,
	    info->PciInfo->device,
	    info->PciInfo->func);
    pDRIInfo->ddxDriverMajorVersion      = RADEON_VERSION_MAJOR;
    pDRIInfo->ddxDriverMinorVersion      = RADEON_VERSION_MINOR;
    pDRIInfo->ddxDriverPatchVersion      = RADEON_VERSION_PATCH;
    pDRIInfo->frameBufferPhysicalAddress = info->LinearAddr;
    pDRIInfo->frameBufferSize            = info->FbMapSize;
    pDRIInfo->frameBufferStride          = (pScrn->displayWidth *
					    info->CurrentLayout.pixel_bytes);
    pDRIInfo->ddxDrawableTableEntry      = RADEON_MAX_DRAWABLES;
    pDRIInfo->maxDrawableTableEntry      = (SAREA_MAX_DRAWABLES
					    < RADEON_MAX_DRAWABLES
					    ? SAREA_MAX_DRAWABLES
					    : RADEON_MAX_DRAWABLES);
#ifdef PER_CONTEXT_SAREA
    /* This is only here for testing per-context SAREAs.  When used, the
       magic number below would be properly defined in a header file. */
    info->perctx_sarea_size = 64 * 1024;
#endif

#ifdef NOT_DONE
    /* FIXME: Need to extend DRI protocol to pass this size back to
     * client for SAREA mapping that includes a device private record
     */
    pDRIInfo->SAREASize =
	((sizeof(XF86DRISAREARec) + 0xfff) & 0x1000); /* round to page */
    /* + shared memory device private rec */
#else
    /* For now the mapping works by using a fixed size defined
     * in the SAREA header
     */
    if (sizeof(XF86DRISAREARec)+sizeof(RADEONSAREAPriv)>SAREA_MAX) {
	ErrorF("Data does not fit in SAREA\n");
	return FALSE;
    }
    pDRIInfo->SAREASize = SAREA_MAX;
#endif

    if (!(pRADEONDRI = (RADEONDRIPtr)xcalloc(sizeof(RADEONDRIRec),1))) {
	DRIDestroyInfoRec(info->pDRIInfo);
	info->pDRIInfo = NULL;
	return FALSE;
    }
    pDRIInfo->devPrivate     = pRADEONDRI;
    pDRIInfo->devPrivateSize = sizeof(RADEONDRIRec);
    pDRIInfo->contextSize    = sizeof(RADEONDRIContextRec);

    pDRIInfo->CreateContext  = RADEONCreateContext;
    pDRIInfo->DestroyContext = RADEONDestroyContext;
    pDRIInfo->SwapContext    = RADEONDRISwapContext;
    pDRIInfo->InitBuffers    = RADEONDRIInitBuffers;
    pDRIInfo->MoveBuffers    = RADEONDRIMoveBuffers;
    pDRIInfo->bufferRequests = DRI_ALL_WINDOWS;

    pDRIInfo->createDummyCtx     = TRUE;
    pDRIInfo->createDummyCtxPriv = FALSE;

    if (!DRIScreenInit(pScreen, pDRIInfo, &info->drmFD)) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[dri] DRIScreenInit failed.  Disabling DRI.\n");
	xfree(pDRIInfo->devPrivate);
	pDRIInfo->devPrivate = NULL;
	DRIDestroyInfoRec(pDRIInfo);
	pDRIInfo = NULL;
	return FALSE;
    }

    /* Check the radeon DRM version */
    version = drmGetVersion(info->drmFD);
    if (version) {
	if (version->version_major != 1 ||
	    version->version_minor < 1) {
            /* incompatible drm version */
            xf86DrvMsg(pScreen->myNum, X_ERROR,
                "[dri] RADEONDRIScreenInit failed because of a version mismatch.\n"
		"[dri] radeon.o kernel module version is %d.%d.%d but version 1.1.x is needed.\n"
		"[dri] Disabling DRI.\n",
                version->version_major,
                version->version_minor,
                version->version_patchlevel);
            drmFreeVersion(version);
            RADEONDRICloseScreen(pScreen);
            return FALSE;
	}
	drmFreeVersion(version);
    }

#if !defined(PCIGART_ENABLED)
				/* Initialize AGP */
    if (!info->IsPCI && !RADEONDRIAgpInit(info, pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }

				/* Initialize PCI */
    if (info->IsPCI) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "[dri] PCI cards not yet supported.  Disabling DRI.\n");
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }
#else
				/* Initialize AGP */
    if (!info->IsPCI && !RADEONDRIAgpInit(info, pScreen)) {
	info->IsPCI = TRUE;
	xf86DrvMsg(pScreen->myNum, X_WARNING,
		   "[agp] AGP failed to initialize -- falling back to PCI mode.\n");
	xf86DrvMsg(pScreen->myNum, X_WARNING,
		   "[agp] Make sure you have the agpgart kernel module loaded.\n");
    }

				/* Initialize PCI */
    if (info->IsPCI && !RADEONDRIPciInit(info, pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }
#endif

				/* DRIScreenInit doesn't add all the
                                   common mappings.  Add additional
                                   mappings here. */
    if (!RADEONDRIMapInit(info, pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }

				/* FIXME: When are these mappings unmapped? */

    if (!RADEONInitVisualConfigs(pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "[dri] Visual configs initialized\n");

    return TRUE;
}

/* Finish initializing the device-dependent DRI state, and call
   DRIFinishScreenInit() to complete the device-independent DRI
   initialization. */
Bool RADEONDRIFinishScreenInit(ScreenPtr pScreen)
{
    ScrnInfoPtr        pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr      info  = RADEONPTR(pScrn);
    RADEONSAREAPrivPtr pSAREAPriv;
    RADEONDRIPtr       pRADEONDRI;

    info->pDRIInfo->driverSwapMethod = DRI_HIDE_X_CONTEXT;
    /* info->pDRIInfo->driverSwapMethod = DRI_SERVER_SWAP; */

    /* NOTE: DRIFinishScreenInit must be called before *DRIKernelInit
       because *DRIKernelInit requires that the hardware lock is held by
       the X server, and the first time the hardware lock is grabbed is
       in DRIFinishScreenInit. */
    if (!DRIFinishScreenInit(pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }

    /* Initialize the kernel data structures */
    if (!RADEONDRIKernelInit(info, pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }

    /* Initialize the vertex buffers list */
    if (!RADEONDRIBufInit(info, pScreen)) {
	RADEONDRICloseScreen(pScreen);
	return FALSE;
    }

    /* Initialize and start the CP if required */
    RADEONDRICPInit(pScrn);

    /* Initialize the SAREA private data structure */
    pSAREAPriv = (RADEONSAREAPrivPtr)DRIGetSAREAPrivate(pScreen);
    memset(pSAREAPriv, 0, sizeof(*pSAREAPriv));

    RADEONDRISAREAInit(pScreen, pSAREAPriv);

    pRADEONDRI                    = (RADEONDRIPtr)info->pDRIInfo->devPrivate;

    pRADEONDRI->deviceID          = info->Chipset;
    pRADEONDRI->width             = pScrn->virtualX;
    pRADEONDRI->height            = pScrn->virtualY;
    pRADEONDRI->depth             = pScrn->depth;
    pRADEONDRI->bpp               = pScrn->bitsPerPixel;

    pRADEONDRI->IsPCI             = info->IsPCI;
    pRADEONDRI->AGPMode           = info->agpMode;

    pRADEONDRI->frontOffset       = info->frontOffset;
    pRADEONDRI->frontPitch        = info->frontPitch;
    pRADEONDRI->backOffset        = info->backOffset;
    pRADEONDRI->backPitch         = info->backPitch;
    pRADEONDRI->depthOffset       = info->depthOffset;
    pRADEONDRI->depthPitch        = info->depthPitch;
    pRADEONDRI->textureOffset     = info->textureOffset;
    pRADEONDRI->textureSize       = info->textureSize;
    pRADEONDRI->log2TexGran       = info->log2TexGran;

    pRADEONDRI->registerHandle    = info->registerHandle;
    pRADEONDRI->registerSize      = info->registerSize;

    pRADEONDRI->statusHandle      = info->ringReadPtrHandle;
    pRADEONDRI->statusSize        = info->ringReadMapSize;

    pRADEONDRI->agpTexHandle      = info->agpTexHandle;
    pRADEONDRI->agpTexMapSize     = info->agpTexMapSize;
    pRADEONDRI->log2AGPTexGran    = info->log2AGPTexGran;
    pRADEONDRI->agpTexOffset      = info->agpTexStart;

    pRADEONDRI->sarea_priv_offset = sizeof(XF86DRISAREARec);

#ifdef PER_CONTEXT_SAREA
    /* Set per-context SAREA size */
    pRADEONDRI->perctx_sarea_size = info->perctx_sarea_size;
#endif

    return TRUE;
}

/* The screen is being closed, so clean up any state and free any
   resources used by the DRI. */
void RADEONDRICloseScreen(ScreenPtr pScreen)
{
    ScrnInfoPtr   pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info  = RADEONPTR(pScrn);

				/* Stop the CP */
    if (info->directRenderingEnabled) {
	RADEONCP_STOP(pScrn, info);
    }

				/* De-allocate vertex buffers */
    if (info->buffers) {
	drmUnmapBufs(info->buffers);
	info->buffers = NULL;
    }

				/* De-allocate all kernel resources */
    drmRadeonCleanupCP(info->drmFD);

				/* De-allocate all AGP resources */
    if (info->agpTex) {
	drmUnmap(info->agpTex, info->agpTexMapSize);
	info->agpTex = NULL;
    }
    if (info->buf) {
	drmUnmap(info->buf, info->bufMapSize);
	info->buf = NULL;
    }
    if (info->ringReadPtr) {
	drmUnmap(info->ringReadPtr, info->ringReadMapSize);
	info->ringReadPtr = NULL;
    }
    if (info->ring) {
	drmUnmap(info->ring, info->ringMapSize);
	info->ring = NULL;
    }
    if (info->agpMemHandle) {
	drmAgpUnbind(info->drmFD, info->agpMemHandle);
	drmAgpFree(info->drmFD, info->agpMemHandle);
	info->agpMemHandle = 0;
	drmAgpRelease(info->drmFD);
    }
    if (info->pciMemHandle) {
	drmScatterGatherFree(info->drmFD, info->pciMemHandle);
	info->pciMemHandle = 0;
    }

				/* De-allocate all DRI resources */
    DRICloseScreen(pScreen);

				/* De-allocate all DRI data structures */
    if (info->pDRIInfo) {
	if (info->pDRIInfo->devPrivate) {
	    xfree(info->pDRIInfo->devPrivate);
	    info->pDRIInfo->devPrivate = NULL;
	}
	DRIDestroyInfoRec(info->pDRIInfo);
	info->pDRIInfo = NULL;
    }
    if (info->pVisualConfigs) {
	xfree(info->pVisualConfigs);
	info->pVisualConfigs = NULL;
    }
    if (info->pVisualConfigsPriv) {
	xfree(info->pVisualConfigsPriv);
	info->pVisualConfigsPriv = NULL;
    }
}
