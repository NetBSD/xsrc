/*
 * Copyright 2005 Eric Anholt
 * Copyright 2005 Benjamin Herrenschmidt
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <anholt@FreeBSD.org>
 *    Zack Rusin <zrusin@trolltech.com>
 *    Benjamin Herrenschmidt <benh@kernel.crashing.org>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "radeon.h"
#include "radeon_reg.h"
#include "r600_reg.h"
#ifdef XF86DRI
#include "radeon_drm.h"
#endif
#include "radeon_macros.h"
#include "radeon_probe.h"
#include "radeon_version.h"

#include "xf86.h"


/***********************************************************************/
#define RINFO_FROM_SCREEN(pScr) ScrnInfoPtr pScrn =  xf86Screens[pScr->myNum]; \
    RADEONInfoPtr info   = RADEONPTR(pScrn)

#define RADEON_TRACE_FALL 0
#define RADEON_TRACE_DRAW 0

#if RADEON_TRACE_FALL
#define RADEON_FALLBACK(x)     		\
do {					\
	ErrorF("%s: ", __FUNCTION__);	\
	ErrorF x;			\
	return FALSE;			\
} while (0)
#else
#define RADEON_FALLBACK(x) return FALSE
#endif

#if RADEON_TRACE_DRAW
#define TRACE do { ErrorF("TRACE: %s\n", __FUNCTION__); } while(0)
#else
#define TRACE
#endif

static struct {
    int rop;
    int pattern;
} RADEON_ROP[] = {
    { RADEON_ROP3_ZERO, RADEON_ROP3_ZERO }, /* GXclear        */
    { RADEON_ROP3_DSa,  RADEON_ROP3_DPa  }, /* Gxand          */
    { RADEON_ROP3_SDna, RADEON_ROP3_PDna }, /* GXandReverse   */
    { RADEON_ROP3_S,    RADEON_ROP3_P    }, /* GXcopy         */
    { RADEON_ROP3_DSna, RADEON_ROP3_DPna }, /* GXandInverted  */
    { RADEON_ROP3_D,    RADEON_ROP3_D    }, /* GXnoop         */
    { RADEON_ROP3_DSx,  RADEON_ROP3_DPx  }, /* GXxor          */
    { RADEON_ROP3_DSo,  RADEON_ROP3_DPo  }, /* GXor           */
    { RADEON_ROP3_DSon, RADEON_ROP3_DPon }, /* GXnor          */
    { RADEON_ROP3_DSxn, RADEON_ROP3_PDxn }, /* GXequiv        */
    { RADEON_ROP3_Dn,   RADEON_ROP3_Dn   }, /* GXinvert       */
    { RADEON_ROP3_SDno, RADEON_ROP3_PDno }, /* GXorReverse    */
    { RADEON_ROP3_Sn,   RADEON_ROP3_Pn   }, /* GXcopyInverted */
    { RADEON_ROP3_DSno, RADEON_ROP3_DPno }, /* GXorInverted   */
    { RADEON_ROP3_DSan, RADEON_ROP3_DPan }, /* GXnand         */
    { RADEON_ROP3_ONE,  RADEON_ROP3_ONE  }  /* GXset          */
};

/* Compute log base 2 of val. */
static __inline__ int
RADEONLog2(int val)
{
	int bits;
#if (defined __i386__ || defined __x86_64__) && (defined __GNUC__)
	__asm volatile("bsrl	%1, %0"
		: "=r" (bits)
		: "c" (val)
	);
	return bits;
#else
	for (bits = 0; val != 0; val >>= 1, ++bits)
		;
	return bits - 1;
#endif
}

static __inline__ uint32_t F_TO_DW(float val)
{
    union {
	float f;
	uint32_t l;
    } tmp;
    tmp.f = val;
    return tmp.l;
}


#ifdef XF86DRM_MODE

static inline void radeon_add_pixmap(struct radeon_cs *cs, PixmapPtr pPix, int read_domains, int write_domain)
{
    struct radeon_exa_pixmap_priv *driver_priv = exaGetPixmapDriverPrivate(pPix);

    radeon_cs_space_add_persistent_bo(cs, driver_priv->bo, read_domains, write_domain);
}

#endif /* XF86DRM_MODE */


/* Assumes that depth 15 and 16 can be used as depth 16, which is okay since we
 * require src and dest datatypes to be equal.
 */
Bool RADEONGetDatatypeBpp(int bpp, uint32_t *type)
{
	switch (bpp) {
	case 8:
		*type = ATI_DATATYPE_CI8;
		return TRUE;
	case 16:
		*type = ATI_DATATYPE_RGB565;
		return TRUE;
	case 24:
		*type = ATI_DATATYPE_CI8;
		return TRUE;
	case 32:
		*type = ATI_DATATYPE_ARGB8888;
		return TRUE;
	default:
		RADEON_FALLBACK(("Unsupported bpp: %d\n", bpp));
		return FALSE;
	}
}

static Bool RADEONPixmapIsColortiled(PixmapPtr pPix)
{
    RINFO_FROM_SCREEN(pPix->drawable.pScreen);

    /* This doesn't account for the back buffer, which we may want to wrap in
     * a pixmap at some point for the purposes of DRI buffer moves.
     */
    if (info->tilingEnabled && exaGetPixmapOffset(pPix) == 0)
	return TRUE;
    else
	return FALSE;
}

static Bool RADEONGetOffsetPitch(PixmapPtr pPix, int bpp, uint32_t *pitch_offset,
				 unsigned int offset, unsigned int pitch)
{
	RINFO_FROM_SCREEN(pPix->drawable.pScreen);

	if (pitch > 16320 || pitch % info->accel_state->exa->pixmapPitchAlign != 0)
		RADEON_FALLBACK(("Bad pitch 0x%08x\n", pitch));

	if (offset % info->accel_state->exa->pixmapOffsetAlign != 0)
		RADEON_FALLBACK(("Bad offset 0x%08x\n", offset));

	pitch = pitch >> 6;
	*pitch_offset = (pitch << 22) | (offset >> 10);

	/* If it's the front buffer, we've got to note that it's tiled? */
	if (RADEONPixmapIsColortiled(pPix))
		*pitch_offset |= RADEON_DST_TILE_MACRO;
	return TRUE;
}

Bool RADEONGetPixmapOffsetPitch(PixmapPtr pPix, uint32_t *pitch_offset)
{
	uint32_t pitch, offset;
	int bpp;

	bpp = pPix->drawable.bitsPerPixel;
	if (bpp == 24)
		bpp = 8;

	offset = radeonGetPixmapOffset(pPix);
	pitch = exaGetPixmapPitch(pPix);

	return RADEONGetOffsetPitch(pPix, bpp, pitch_offset, offset, pitch);
}

#if X_BYTE_ORDER == X_BIG_ENDIAN

static unsigned long swapper_surfaces[6];

static Bool RADEONPrepareAccess_BE(PixmapPtr pPix, int index)
{
    RINFO_FROM_SCREEN(pPix->drawable.pScreen);
    unsigned char *RADEONMMIO = info->MMIO;
    uint32_t offset = exaGetPixmapOffset(pPix);
    int bpp, soff;
    uint32_t size, flags;

    /* Front buffer is always set with proper swappers */
    if (offset == 0)
        return TRUE;

    /* If same bpp as front buffer, just do nothing as the main
     * swappers will apply
     */
    bpp = pPix->drawable.bitsPerPixel;
    if (bpp == pScrn->bitsPerPixel)
        return TRUE;

    /* We need to setup a separate swapper, let's request a
     * surface. We need to align the size first
     */
    size = exaGetPixmapSize(pPix);
    size = RADEON_ALIGN(size, RADEON_GPU_PAGE_SIZE);

    /* Set surface to tiling disabled with appropriate swapper */
    switch (bpp) {
    case 16:
        flags = RADEON_SURF_AP0_SWP_16BPP | RADEON_SURF_AP1_SWP_16BPP;
	break;
    case 32:
        flags = RADEON_SURF_AP0_SWP_32BPP | RADEON_SURF_AP1_SWP_32BPP;
	break;
    default:
        flags = 0;
    }
#if defined(XF86DRI)
    if (info->directRenderingEnabled && info->allowColorTiling) {
	struct drm_radeon_surface_alloc drmsurfalloc;
	int rc;

        drmsurfalloc.address = offset;
        drmsurfalloc.size = size;
	drmsurfalloc.flags = flags | 1; /* bogus pitch to please DRM */

        rc = drmCommandWrite(info->dri->drmFD, DRM_RADEON_SURF_ALLOC,
			     &drmsurfalloc, sizeof(drmsurfalloc));
	if (rc < 0) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "drm: could not allocate surface for access"
		       " swapper, err: %d!\n", rc);
	    return FALSE;
	}
	swapper_surfaces[index] = offset;

	return TRUE;
    }
#endif
    soff = (index + 1) * 0x10;
    OUTREG(RADEON_SURFACE0_INFO + soff, flags);
    OUTREG(RADEON_SURFACE0_LOWER_BOUND + soff, offset);
    OUTREG(RADEON_SURFACE0_UPPER_BOUND + soff, offset + size - 1);
    swapper_surfaces[index] = offset;
    return TRUE;
}

static void RADEONFinishAccess_BE(PixmapPtr pPix, int index)
{
    RINFO_FROM_SCREEN(pPix->drawable.pScreen);
    unsigned char *RADEONMMIO = info->MMIO;
    uint32_t offset = exaGetPixmapOffset(pPix);
    int soff;

    /* Front buffer is always set with proper swappers */
    if (offset == 0)
        return;

    if (swapper_surfaces[index] == 0)
        return;
#if defined(XF86DRI)
    if (info->directRenderingEnabled && info->allowColorTiling) {
	struct drm_radeon_surface_free drmsurffree;

	drmsurffree.address = offset;
	drmCommandWrite(info->dri->drmFD, DRM_RADEON_SURF_FREE,
			&drmsurffree, sizeof(drmsurffree));
	swapper_surfaces[index] = 0;
	return;
    }
#endif
    soff = (index + 1) * 0x10;
    OUTREG(RADEON_SURFACE0_INFO + soff, 0);
    OUTREG(RADEON_SURFACE0_LOWER_BOUND + soff, 0);
    OUTREG(RADEON_SURFACE0_UPPER_BOUND + soff, 0);
    swapper_surfaces[index] = 0;
}

#endif /* X_BYTE_ORDER == X_BIG_ENDIAN */

#ifdef XF86DRM_MODE
Bool RADEONPrepareAccess_CS(PixmapPtr pPix, int index)
{
    ScreenPtr pScreen = pPix->drawable.pScreen;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct radeon_exa_pixmap_priv *driver_priv;
    int ret;

#if X_BYTE_ORDER == X_BIG_ENDIAN
    /* May need to handle byte swapping in DownloadFrom/UploadToScreen */
    if (pPix->drawable.bitsPerPixel > 8 &&
	pPix != pScreen->GetScreenPixmap(pScreen))
	return FALSE;
#endif

    driver_priv = exaGetPixmapDriverPrivate(pPix);
    if (!driver_priv)
      return FALSE;

    /* if we have more refs than just the BO then flush */
    if (radeon_bo_is_referenced_by_cs(driver_priv->bo, info->cs))
        radeon_cs_flush_indirect(pScrn);
    
    /* flush IB */
    ret = radeon_bo_map(driver_priv->bo, 1);
    if (ret) {
      FatalError("failed to map pixmap %d\n", ret);
      return FALSE;
    }
    driver_priv->bo_mapped = TRUE;

    pPix->devPrivate.ptr = driver_priv->bo->ptr;

    return TRUE;
}

void RADEONFinishAccess_CS(PixmapPtr pPix, int index)
{
    struct radeon_exa_pixmap_priv *driver_priv;

    driver_priv = exaGetPixmapDriverPrivate(pPix);
    if (!driver_priv || !driver_priv->bo_mapped)
        return;

    radeon_bo_unmap(driver_priv->bo);
    pPix->devPrivate.ptr = NULL;
}


void *RADEONEXACreatePixmap(ScreenPtr pScreen, int size, int align)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct radeon_exa_pixmap_priv *new_priv;

#ifdef EXA_MIXED_PIXMAPS
    if (info->accel_state->exa->flags & EXA_MIXED_PIXMAPS) {
        if (size != 0 && !info->exa_force_create &&
	    info->exa_pixmaps == FALSE)
            return NULL;
    }
#endif
	    
    new_priv = xcalloc(1, sizeof(struct radeon_exa_pixmap_priv));
    if (!new_priv)
	return NULL;

    if (size == 0)
	return new_priv;

    new_priv->bo = radeon_bo_open(info->bufmgr, 0, size, align,
				  RADEON_GEM_DOMAIN_VRAM, 0);
    if (!new_priv->bo) {
	xfree(new_priv);
	ErrorF("Failed to alloc memory\n");
	return NULL;
    }
    
    return new_priv;

}

void *RADEONEXACreatePixmap2(ScreenPtr pScreen, int width, int height,
			     int depth, int usage_hint, int bitsPerPixel,
			     int *new_pitch)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct radeon_exa_pixmap_priv *new_priv;
    int padded_width;
    uint32_t size;
    uint32_t tiling = 0;
    int pixmap_align;

#ifdef EXA_MIXED_PIXMAPS
    if (info->accel_state->exa->flags & EXA_MIXED_PIXMAPS) {
	if (width != 0 && height != 0 && !info->exa_force_create &&
	    info->exa_pixmaps == FALSE)
            return NULL;
    }
#endif

    if (usage_hint) {
	if (info->allowColorTiling) {
    	    if (usage_hint & RADEON_CREATE_PIXMAP_TILING_MACRO)
 	   	tiling |= RADEON_TILING_MACRO;
    	    if (usage_hint & RADEON_CREATE_PIXMAP_TILING_MICRO)
                tiling |= RADEON_TILING_MICRO;
	}
    }

    if (tiling) {
	height = RADEON_ALIGN(height, 16);
	pixmap_align = 256;
    } else
	pixmap_align = 64;

    padded_width = ((width * bitsPerPixel + FB_MASK) >> FB_SHIFT) * sizeof(FbBits);
    padded_width = RADEON_ALIGN(padded_width, pixmap_align);
    size = height * padded_width;

    new_priv = xcalloc(1, sizeof(struct radeon_exa_pixmap_priv));
    if (!new_priv)
	return NULL;

    if (size == 0)
	return new_priv;

    *new_pitch = padded_width;

    new_priv->bo = radeon_bo_open(info->bufmgr, 0, size, 0,
				  RADEON_GEM_DOMAIN_VRAM, 0);
    if (!new_priv->bo) {
	xfree(new_priv);
	ErrorF("Failed to alloc memory\n");
	return NULL;
    }

    if (tiling)
	radeon_bo_set_tiling(new_priv->bo, tiling, *new_pitch);

    return new_priv;
}

void RADEONEXADestroyPixmap(ScreenPtr pScreen, void *driverPriv)
{
    struct radeon_exa_pixmap_priv *driver_priv = driverPriv;

    if (!driverPriv)
      return;

    if (driver_priv->bo)
	radeon_bo_unref(driver_priv->bo);
    xfree(driverPriv);
}

struct radeon_bo *radeon_get_pixmap_bo(PixmapPtr pPix)
{
    struct radeon_exa_pixmap_priv *driver_priv;
    driver_priv = exaGetPixmapDriverPrivate(pPix);
    return driver_priv->bo;
}

void radeon_set_pixmap_bo(PixmapPtr pPix, struct radeon_bo *bo)
{
    struct radeon_exa_pixmap_priv *driver_priv;

    driver_priv = exaGetPixmapDriverPrivate(pPix);
    if (driver_priv) {
	if (driver_priv->bo)
	    radeon_bo_unref(driver_priv->bo);

	radeon_bo_ref(bo);
	driver_priv->bo = bo;
    }
}

Bool RADEONEXAPixmapIsOffscreen(PixmapPtr pPix)
{
    struct radeon_exa_pixmap_priv *driver_priv;

    driver_priv = exaGetPixmapDriverPrivate(pPix);

    if (!driver_priv)
       return FALSE;
    if (driver_priv->bo)
       return TRUE;
    return FALSE;
}
#endif

#define ENTER_DRAW(x) TRACE
#define LEAVE_DRAW(x) TRACE
/***********************************************************************/

#define ACCEL_MMIO
#define ACCEL_PREAMBLE()	unsigned char *RADEONMMIO = info->MMIO
#define BEGIN_ACCEL(n)		RADEONWaitForFifo(pScrn, (n))
#define OUT_ACCEL_REG(reg, val)	OUTREG(reg, val)
#define OUT_ACCEL_REG_F(reg, val) OUTREG(reg, F_TO_DW(val))
#define OUT_RELOC(x, read, write)            do {} while(0)
#define FINISH_ACCEL()

#ifdef RENDER
#include "radeon_exa_render.c"
#endif
#include "radeon_exa_funcs.c"

#undef ACCEL_MMIO
#undef ACCEL_PREAMBLE
#undef BEGIN_ACCEL
#undef OUT_ACCEL_REG
#undef OUT_ACCEL_REG_F
#undef FINISH_ACCEL
#undef OUT_RELOC

#ifdef XF86DRI

#define ACCEL_CP
#define ACCEL_PREAMBLE()						\
    RING_LOCALS;							\
    RADEONCP_REFRESH(pScrn, info)
#define BEGIN_ACCEL(n)		BEGIN_RING(2*(n))
#define OUT_ACCEL_REG(reg, val)	OUT_RING_REG(reg, val)
#define FINISH_ACCEL()		ADVANCE_RING()
#define OUT_RELOC(x, read, write) OUT_RING_RELOC(x, read, write)

#define OUT_RING_F(x) OUT_RING(F_TO_DW(x))

#ifdef RENDER
#include "radeon_exa_render.c"
#endif
#include "radeon_exa_funcs.c"

#undef ACCEL_CP
#undef ACCEL_PREAMBLE
#undef BEGIN_ACCEL
#undef OUT_ACCEL_REG
#undef FINISH_ACCEL
#undef OUT_RING_F

#endif /* XF86DRI */

/*
 * Once screen->off_screen_base is set, this function
 * allocates the remaining memory appropriately
 */
Bool RADEONSetupMemEXA (ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    int cpp = info->CurrentLayout.pixel_bytes;
    int screen_size;
    int byteStride = pScrn->displayWidth * cpp;

    if (info->accel_state->exa != NULL) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "Memory map already initialized\n");
	return FALSE;
    }
    info->accel_state->exa = exaDriverAlloc();
    if (info->accel_state->exa == NULL)
	return FALSE;

    /* Need to adjust screen size for 16 line tiles, and then make it align to.
     * the buffer alignment requirement.
     */
    if (info->allowColorTiling)
	screen_size = RADEON_ALIGN(pScrn->virtualY, 16) * byteStride;
    else
	screen_size = pScrn->virtualY * byteStride;

    info->accel_state->exa->memoryBase = info->FB;
    info->accel_state->exa->memorySize = info->FbMapSize - info->FbSecureSize;
    info->accel_state->exa->offScreenBase = screen_size;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Allocating from a screen of %ld kb\n",
	       info->accel_state->exa->memorySize / 1024);

    /* Reserve static area for hardware cursor */
    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
        int cursor_size = 64 * 4 * 64;
        int align = IS_AVIVO_VARIANT ? 4096 : 256;
        int c;

        for (c = 0; c < xf86_config->num_crtc; c++) {
            xf86CrtcPtr crtc = xf86_config->crtc[c];
            RADEONCrtcPrivatePtr radeon_crtc = crtc->driver_private;

            radeon_crtc->cursor_offset =
                RADEON_ALIGN(info->accel_state->exa->offScreenBase, align);
            info->accel_state->exa->offScreenBase = radeon_crtc->cursor_offset + cursor_size;

            xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                       "Will use %d kb for hardware cursor %d at offset 0x%08x\n",
                       (cursor_size * xf86_config->num_crtc) / 1024,
                       c,
                       (unsigned int)radeon_crtc->cursor_offset);
        }
    }

#if defined(XF86DRI)
    if (info->directRenderingEnabled) {
	int depthCpp = (info->dri->depthBits - 8) / 4, l, next, depth_size;

	info->dri->frontOffset = 0;
	info->dri->frontPitch = pScrn->displayWidth;

        xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Will use %d kb for front buffer at offset 0x%08x\n",
	       screen_size / 1024, info->dri->frontOffset);
	RADEONDRIAllocatePCIGARTTable(pScreen);
	
	if (info->cardType==CARD_PCIE)
	  xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		     "Will use %d kb for PCI GART at offset 0x%08x\n",
		     info->dri->pciGartSize / 1024,
		     (int)info->dri->pciGartOffset);

	/* Reserve a static area for the back buffer the same size as the
	 * visible screen.  XXX: This would be better initialized in ati_dri.c
	 * when GLX is set up, but the offscreen memory manager's allocations
	 * don't last through VT switches, while the kernel's understanding of
	 * offscreen locations does.
	 */
	info->dri->backPitch = pScrn->displayWidth;
	next = RADEON_ALIGN(info->accel_state->exa->offScreenBase, RADEON_GPU_PAGE_SIZE);
	if (!info->dri->noBackBuffer &&
	    next + screen_size <= info->accel_state->exa->memorySize)
	{
	    info->dri->backOffset = next;
	    info->accel_state->exa->offScreenBase = next + screen_size;
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Will use %d kb for back buffer at offset 0x%08x\n",
		       screen_size / 1024, info->dri->backOffset);
	}

	/* Reserve the static depth buffer, and adjust pitch and height to
	 * handle tiling.
	 */
	info->dri->depthPitch = RADEON_ALIGN(pScrn->displayWidth, 32);
	depth_size = RADEON_ALIGN(pScrn->virtualY, 16) * info->dri->depthPitch * depthCpp;
	next = RADEON_ALIGN(info->accel_state->exa->offScreenBase, RADEON_GPU_PAGE_SIZE);
	if (next + depth_size <= info->accel_state->exa->memorySize)
	{
	    info->dri->depthOffset = next;
	    info->accel_state->exa->offScreenBase = next + depth_size;
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Will use %d kb for depth buffer at offset 0x%08x\n",
		       depth_size / 1024, info->dri->depthOffset);
	}
	
	info->dri->textureSize *= (info->accel_state->exa->memorySize -
				   info->accel_state->exa->offScreenBase) / 100;

	l = RADEONLog2(info->dri->textureSize / RADEON_NR_TEX_REGIONS);
	if (l < RADEON_LOG_TEX_GRANULARITY)
	    l = RADEON_LOG_TEX_GRANULARITY;
	info->dri->textureSize = (info->dri->textureSize >> l) << l;
	if (info->dri->textureSize >= 512 * 1024) {
	    info->dri->textureOffset = info->accel_state->exa->offScreenBase;
	    info->accel_state->exa->offScreenBase += info->dri->textureSize;
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Will use %d kb for textures at offset 0x%08x\n",
		       info->dri->textureSize / 1024, info->dri->textureOffset);
	} else {
	    /* Minimum texture size is for 2 256x256x32bpp textures */
	    info->dri->textureSize = 0;
	}
    } else
#endif /* XF86DRI */
    	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "Will use %d kb for front buffer at offset 0x%08x\n",
		       screen_size / 1024, 0);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Will use %ld kb for X Server offscreen at offset 0x%08lx\n",
	       (info->accel_state->exa->memorySize - info->accel_state->exa->offScreenBase) /
	       1024, info->accel_state->exa->offScreenBase);

    return TRUE;
}

#ifdef XF86DRI

#ifndef ExaOffscreenMarkUsed
extern void ExaOffscreenMarkUsed(PixmapPtr);
#endif

unsigned long long
RADEONTexOffsetStart(PixmapPtr pPix)
{
    RINFO_FROM_SCREEN(pPix->drawable.pScreen);
    unsigned long long offset;

    if (exaGetPixmapDriverPrivate(pPix))
	return -1;

    exaMoveInPixmap(pPix);
    ExaOffscreenMarkUsed(pPix);

    offset = exaGetPixmapOffset(pPix);

    if (offset > info->FbMapSize)
	return ~0ULL;
    else
	return info->fbLocation + offset;
}
#endif
