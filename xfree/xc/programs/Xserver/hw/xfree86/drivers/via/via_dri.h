/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/via/via_dri.h,v 1.3 2004/12/10 16:07:03 alanh Exp $ */
/*
 * Copyright 1998-2003 VIA Technologies, Inc. All Rights Reserved.
 * Copyright 2001-2003 S3 Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * VIA, S3 GRAPHICS, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _VIA_DRI_H_
#define _VIA_DRI_H_ 1

#include "drm.h"
#include "xf86drm.h"
#include "via_common.h"

#define VIA_MAX_DRAWABLES 256

#define VIA_DRI_VERSION_MAJOR		4
#define VIA_DRI_VERSION_MINOR		1

/*
 * BEGIN compatibility with the DRM module via_drm.h
 * Structures here MUST be the same as used in the drm module.
 */

#define VIA_NR_SAREA_CLIPRECTS          8
#define VIA_NR_TEX_REGIONS              64

#define VIA_NR_XVMC_PORTS               10
#define VIA_NR_XVMC_LOCKS               5
#define VIA_MAX_CACHELINE_SIZE          64
 
#define XVMCLOCKPTR(saPriv,lockNo) \
    ((volatile drmLock *)(((((unsigned long) (saPriv)->XvMCLockArea) +	\
		      (VIA_MAX_CACHELINE_SIZE - 1)) &			\
		      ~(VIA_MAX_CACHELINE_SIZE - 1)) +			\
			  VIA_MAX_CACHELINE_SIZE*(lockNo)))

/*
 * Potentially suspending lock for the decoder hardware.
 */

#define XVMC_DECODER_FUTEX(saPriv) XVMCLOCKPTR(saPriv,0)

typedef struct _drm_via_tex_region {
    unsigned char next, prev;       /* indices to form a circular LRU  */
    unsigned char inUse;            /* owned by a client, or free? */
    int age;                        /* tracked by clients to update */
                                    /*local LRU's */
} drm_via_tex_region_t;


typedef struct _drm_via_sarea {
    unsigned int dirty;
    unsigned int nbox;
    drm_clip_rect_t boxes[VIA_NR_SAREA_CLIPRECTS];
    drm_via_tex_region_t texList[VIA_NR_TEX_REGIONS + 1];
    int texAge;             /* last time texture was uploaded */
    int ctxOwner;           /* last context to upload state */
    int vertexPrim;

    /*
     * Below, common variables for XvMC.
     */ 

    /*
     * We want the lock integers alone on, and aligned to, a cache line.
     * Therefore this somewhat strange construct.
     */

    char XvMCLockArea[VIA_MAX_CACHELINE_SIZE * (VIA_NR_XVMC_LOCKS + 1)];

    unsigned XvMCDisplaying[VIA_NR_XVMC_PORTS];
    unsigned XvMCSubPicOn[VIA_NR_XVMC_PORTS];
    unsigned XvMCCtxNoGrabbed;
		     
} drm_via_sarea_t;

/*
 * END compatibility region.
 */


typedef drm_via_sarea_t VIASAREAPriv;

typedef struct {
    drm_handle_t handle;
    drmSize size;
    drmAddress map;
} viaRegion, *viaRegionPtr;

typedef struct {
    viaRegion regs, agp;
    int deviceID;
    int width;
    int height;
    int mem;
    int bytesPerPixel;
    int priv1;
    int priv2;
    int fbOffset;
    int fbSize;
    Bool drixinerama;
    int backOffset;
    int depthOffset;
    int textureOffset;
    int textureSize;
    int irqEnabled;
    unsigned int scrnX, scrnY;
    int sarea_priv_offset;
    int ringBufActive;
    unsigned int reg_pause_addr;
} VIADRIRec, *VIADRIPtr;

typedef struct {
    int dummy;
} VIAConfigPrivRec, *VIAConfigPrivPtr;

typedef struct {
    int dummy;
} VIADRIContextRec, *VIADRIContextPtr;

#ifdef XFree86Server

#include "screenint.h"

Bool VIADRIScreenInit(ScreenPtr pScreen);
void VIADRICloseScreen(ScreenPtr pScreen);
Bool VIADRIFinishScreenInit(ScreenPtr pScreen);
void viaDRILeaveVT(int scrnIndex);
void viaDRIEnterVT(int scrnIndex);



#endif /* XFree86Server */
#endif /* _VIA_DRI_H_ */
