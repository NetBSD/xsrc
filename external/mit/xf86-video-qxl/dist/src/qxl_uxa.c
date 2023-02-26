/*
 * Copyright 2008 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** \file qxl_driver.c
 * \author Adam Jackson <ajax@redhat.com>
 * \author Søren Sandmann <sandmann@redhat.com>
 *
 * This is qxl, a driver for the Qumranet paravirtualized graphics device
 * in qemu.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qxl.h"
#include "dfps.h"
#include <spice/protocol.h>

#if HAS_DEVPRIVATEKEYREC
DevPrivateKeyRec uxa_pixmap_index;
#else
int uxa_pixmap_index;
#endif

static Bool
qxl_prepare_access (PixmapPtr pixmap, RegionPtr region, uxa_access_t access)
{
    return qxl_surface_prepare_access (get_surface (pixmap),
                                       pixmap, region, access);
}

static void
qxl_finish_access (PixmapPtr pixmap)
{
    qxl_surface_finish_access (get_surface (pixmap), pixmap);
}

static Bool
qxl_pixmap_is_offscreen (PixmapPtr pixmap)
{
    return !!get_surface (pixmap);
}

static Bool
good_alu_and_pm (DrawablePtr drawable, int alu, Pixel planemask)
{
    if (!UXA_PM_IS_SOLID (drawable, planemask))
	return FALSE;

    if (alu != GXcopy)
	return FALSE;

    return TRUE;
}

/*
 * Solid fill
 */
static Bool
qxl_check_solid (DrawablePtr drawable, int alu, Pixel planemask)
{
    if (!good_alu_and_pm (drawable, alu, planemask))
	return FALSE;

    return TRUE;
}

static Bool
qxl_prepare_solid (PixmapPtr pixmap, int alu, Pixel planemask, Pixel fg)
{
    qxl_surface_t *surface;

    if (!(surface = get_surface (pixmap)))
	return FALSE;

    return qxl_surface_prepare_solid (surface, fg);
}

static void
qxl_solid (PixmapPtr pixmap, int x1, int y1, int x2, int y2)
{
    qxl_surface_solid (get_surface (pixmap), x1, y1, x2, y2);
}

static void
qxl_done_solid (PixmapPtr pixmap)
{
}

/*
 * Copy
 */
static Bool
qxl_check_copy (PixmapPtr source, PixmapPtr dest,
                int alu, Pixel planemask)
{
    if (!good_alu_and_pm ((DrawablePtr)source, alu, planemask))
	return FALSE;

    if (source->drawable.bitsPerPixel != dest->drawable.bitsPerPixel)
    {
	ErrorF ("differing bitsperpixel - this shouldn't happen\n");
	return FALSE;
    }

    return TRUE;
}

static Bool
qxl_prepare_copy (PixmapPtr source, PixmapPtr dest,
                  int xdir, int ydir, int alu,
                  Pixel planemask)
{
    return qxl_surface_prepare_copy (get_surface (dest), get_surface (source));
}

static void
qxl_copy (PixmapPtr dest,
          int src_x1, int src_y1,
          int dest_x1, int dest_y1,
          int width, int height)
{
    qxl_surface_copy (get_surface (dest),
                      src_x1, src_y1,
                      dest_x1, dest_y1,
                      width, height);
}

static void
qxl_done_copy (PixmapPtr dest)
{
}

/*
 * Composite
 */
static Bool
can_accelerate_picture (qxl_screen_t *qxl, PicturePtr pict)
{
    if (!pict)
	return TRUE;

    if (pict->format != PICT_a8r8g8b8		&&
	pict->format != PICT_x8r8g8b8		&&
	pict->format != PICT_a8)
    {
        if (qxl->debug_render_fallbacks)
        {
            ErrorF ("Image with format %x can't be accelerated \n",
                    pict->format);
        }

        return FALSE;
    }

    if (!pict->pDrawable)
    {
        if (qxl->debug_render_fallbacks)
        {
            ErrorF ("Source image (of type %d) can't be accelerated\n",
                    pict->pSourcePict->type);
        }
     
	return FALSE;
    }

    if (pict->transform)
    {
	if (pict->transform->matrix[2][0] != 0	||
	    pict->transform->matrix[2][1] != 0	||
	    pict->transform->matrix[2][2] != pixman_int_to_fixed (1))
	{
            if (qxl->debug_render_fallbacks)
                ErrorF ("Image with non-affine transform can't be accelerated\n");

	    return FALSE;
	}
    }

    if (pict->filter != PictFilterBilinear	&&
	pict->filter != PictFilterNearest)
    {
        if (qxl->debug_render_fallbacks)
        {
            ErrorF ("Image with filter type %d can't be accelerated\n",
                    pict->filter);
        }

        return FALSE;
    }

    return TRUE;
}

#define QXL_HAS_CAP(qxl, cap)						\
    (((qxl)->rom->client_capabilities[(cap) / 8]) & (1 << ((cap) % 8)))

static Bool
qxl_has_composite (qxl_screen_t *qxl)
{
#ifdef XF86DRM_MODE
    if (qxl->kms_enabled) {
#if 0 /* KMS Composite support seems broken - needs better hw support */
	static Bool result, checked;
	if (!checked) {
	    result = qxl_kms_check_cap(qxl, SPICE_DISPLAY_CAP_COMPOSITE);
	    checked = TRUE;
	}
	return result;
#else
	return FALSE;
#endif
    }
#endif
#ifndef XSPICE
    return
	qxl->pci->revision >= 4			&&
	QXL_HAS_CAP (qxl, SPICE_DISPLAY_CAP_COMPOSITE);
#else
    /* FIXME */
    return FALSE;
#endif
}

static Bool
qxl_has_a8_surfaces (qxl_screen_t *qxl)
{
#ifdef XF86DRM_MODE
    if (qxl->kms_enabled) {
#if 0 /* KMS Composite support seems broken - needs better hw support */
        static Bool result, checked;
	if (!checked) {
            result = qxl_kms_check_cap(qxl, SPICE_DISPLAY_CAP_A8_SURFACE);
	    checked = TRUE;
	}
	return result;
#else
	return FALSE;
#endif
    }
#endif
#ifndef XSPICE
    if (qxl->pci->revision < 4)
    {
        if (qxl->debug_render_fallbacks)
        {
            ErrorF ("No a8 surface due to revision being %d, which is < 4\n",
                    qxl->pci->revision);
        }

        return FALSE;
    }

    if (!QXL_HAS_CAP (qxl, SPICE_DISPLAY_CAP_COMPOSITE))
    {
        if (qxl->debug_render_fallbacks)
        {
            ErrorF ("No composite due to client not providing SPICE_DISPLAY_CAP_A8_SURFACE\n");
        }

        return FALSE;
    }

    return TRUE;

#else
    /* FIXME */
    return FALSE;
#endif
}

static Bool
qxl_check_composite (int op,
		     PicturePtr pSrcPicture,
		     PicturePtr pMaskPicture,
		     PicturePtr pDstPicture,
		     int width, int height)
{
    int i;
    ScreenPtr pScreen = pDstPicture->pDrawable->pScreen;
    ScrnInfoPtr pScrn = xf86ScreenToScrn (pScreen);
    qxl_screen_t *qxl = pScrn->driverPrivate;

    static const int accelerated_ops[] =
    {
	PictOpClear, PictOpSrc, PictOpDst, PictOpOver, PictOpOverReverse,
	PictOpIn, PictOpInReverse, PictOpOut, PictOpOutReverse,
	PictOpAtop, PictOpAtopReverse, PictOpXor, PictOpAdd,
	PictOpSaturate, PictOpMultiply, PictOpScreen, PictOpOverlay,
	PictOpDarken, PictOpLighten, PictOpColorDodge, PictOpColorBurn,
	PictOpHardLight, PictOpSoftLight, PictOpDifference, PictOpExclusion,
	PictOpHSLHue, PictOpHSLSaturation, PictOpHSLColor, PictOpHSLLuminosity,
    };

    if (!qxl_has_composite (qxl))
	return FALSE;

    if (!can_accelerate_picture (qxl, pSrcPicture)	||
	!can_accelerate_picture (qxl, pMaskPicture)	||
	!can_accelerate_picture (qxl, pDstPicture))
    {
	return FALSE;
    }

    for (i = 0; i < sizeof (accelerated_ops) / sizeof (accelerated_ops[0]); ++i)
    {
	if (accelerated_ops[i] == op)
	    goto found;
    }

    if (qxl->debug_render_fallbacks)
        ErrorF ("Compositing operator %d can't be accelerated\n", op);

    return FALSE;

found:
    return TRUE;
}

static Bool
qxl_check_composite_target (PixmapPtr pixmap)
{
    return TRUE;
}

static Bool
qxl_check_composite_texture (ScreenPtr screen,
			     PicturePtr pPicture)
{
    return TRUE;
}

static Bool
qxl_prepare_composite (int op,
		       PicturePtr pSrcPicture,
		       PicturePtr pMaskPicture,
		       PicturePtr pDstPicture,
		       PixmapPtr pSrc,
		       PixmapPtr pMask,
		       PixmapPtr pDst)
{
    return qxl_surface_prepare_composite (
	op, pSrcPicture, pMaskPicture, pDstPicture,
	get_surface (pSrc),
	pMask? get_surface (pMask) : NULL,
	get_surface (pDst));
}

static void
qxl_composite (PixmapPtr pDst,
	       int src_x, int src_y,
	       int mask_x, int mask_y,
	       int dst_x, int dst_y,
	       int width, int height)
{
    qxl_surface_composite (
	get_surface (pDst),
	src_x, src_y,
	mask_x, mask_y,
	dst_x, dst_y, width, height);
}

static void
qxl_done_composite (PixmapPtr pDst)
{
    ;
}

static Bool
qxl_put_image (PixmapPtr pDst, int x, int y, int w, int h,
               char *src, int src_pitch)
{
    qxl_surface_t *surface = get_surface (pDst);

    if (surface)
	return qxl_surface_put_image (surface, x, y, w, h, src, src_pitch);

    return FALSE;
}

static void
qxl_set_screen_pixmap (PixmapPtr pixmap)
{
    pixmap->drawable.pScreen->devPrivate = pixmap;
}

static PixmapPtr
qxl_create_pixmap (ScreenPtr screen, int w, int h, int depth, unsigned usage)
{
    ScrnInfoPtr    scrn = xf86ScreenToScrn (screen);
    PixmapPtr      pixmap;
    qxl_screen_t * qxl = scrn->driverPrivate;
    qxl_surface_t *surface;

    if (w > 32767 || h > 32767)
	return NULL;

    qxl_surface_cache_sanity_check (qxl->surface_cache);

#if 0
    ErrorF ("Create pixmap: %d %d @ %d (usage: %d)\n", w, h, depth, usage);
#endif

    if (qxl->kms_enabled)
	goto fallback;
    if (uxa_swapped_out (screen))
	goto fallback;

    if (depth == 8 && !qxl_has_a8_surfaces (qxl))
    {
	/* FIXME: When we detect a _change_ in the property of having a8
	 * surfaces, we should copy all existing a8 surface to host memory
	 * and then destroy the ones on the device.
	 */
	goto fallback;
    }

    if (!w || !h)
      goto fallback;

    surface = qxl->bo_funcs->create_surface (qxl, w, h, depth);
    if (surface)
    {
	/* ErrorF ("   Successfully created surface in video memory\n"); */

	pixmap = fbCreatePixmap (screen, 0, 0, depth, usage);

	screen->ModifyPixmapHeader (pixmap, w, h,
	                            -1, -1, -1,
	                            NULL);

#if 0
	ErrorF ("Create pixmap %p with surface %p\n", pixmap, surface);
#endif
	set_surface (pixmap, surface);
	qxl_surface_set_pixmap (surface, pixmap);

	qxl_surface_cache_sanity_check (qxl->surface_cache);
    }
    else
    {
#if 0
	ErrorF ("   Couldn't allocate %d x %d @ %d surface in video memory\n",
	        w, h, depth);
#endif
    fallback:
	pixmap = fbCreatePixmap (screen, w, h, depth, usage);

#if 0
	ErrorF ("Create pixmap %p without surface\n", pixmap);
#endif
    }

    return pixmap;
}

static Bool
qxl_destroy_pixmap (PixmapPtr pixmap)
{
    ScreenPtr      screen = pixmap->drawable.pScreen;
    ScrnInfoPtr    scrn = xf86ScreenToScrn (screen);
    qxl_screen_t * qxl = scrn->driverPrivate;
    qxl_surface_t *surface = NULL;

    qxl_surface_cache_sanity_check (qxl->surface_cache);

    if (pixmap->refcnt == 1)
    {
	surface = get_surface (pixmap);

#if 0
	ErrorF ("- Destroy %p (had surface %p)\n", pixmap, surface);
#endif

	if (surface)
	{
	    qxl->bo_funcs->destroy_surface(surface);
	    set_surface (pixmap, NULL);

	    qxl_surface_cache_sanity_check (qxl->surface_cache);
	}
    }

    fbDestroyPixmap (pixmap);
    return TRUE;
}

static void
set_uxa_functions(qxl_screen_t *qxl, ScreenPtr screen)
{
    /* Solid fill */
    qxl->uxa->check_solid = qxl_check_solid;
    qxl->uxa->prepare_solid = qxl_prepare_solid;
    qxl->uxa->solid = qxl_solid;
    qxl->uxa->done_solid = qxl_done_solid;

    /* Copy */
    qxl->uxa->check_copy = qxl_check_copy;
    qxl->uxa->prepare_copy = qxl_prepare_copy;
    qxl->uxa->copy = qxl_copy;
    qxl->uxa->done_copy = qxl_done_copy;

    /* Composite */
    qxl->uxa->check_composite = qxl_check_composite;
    qxl->uxa->check_composite_target = qxl_check_composite_target;
    qxl->uxa->check_composite_texture = qxl_check_composite_texture;
    qxl->uxa->prepare_composite = qxl_prepare_composite;
    qxl->uxa->composite = qxl_composite;
    qxl->uxa->done_composite = qxl_done_composite;

    /* PutImage */
    qxl->uxa->put_image = qxl_put_image;

    /* Prepare access */
    qxl->uxa->prepare_access = qxl_prepare_access;
    qxl->uxa->finish_access = qxl_finish_access;

    qxl->uxa->pixmap_is_offscreen = qxl_pixmap_is_offscreen;

    screen->SetScreenPixmap = qxl_set_screen_pixmap;
    screen->CreatePixmap = qxl_create_pixmap;
    screen->DestroyPixmap = qxl_destroy_pixmap;
}

Bool
qxl_uxa_init (qxl_screen_t *qxl, ScreenPtr screen)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn (screen);

#if HAS_DIXREGISTERPRIVATEKEY
    if (!dixRegisterPrivateKey (&uxa_pixmap_index, PRIVATE_PIXMAP, 0))
	return FALSE;
#else
    if (!dixRequestPrivate (&uxa_pixmap_index, 0))
	return FALSE;
#endif

    qxl->uxa = uxa_driver_alloc ();
    if (qxl->uxa == NULL)
	return FALSE;

    memset (qxl->uxa, 0, sizeof (*qxl->uxa));

    qxl->uxa->uxa_major = 1;
    qxl->uxa->uxa_minor = 0;

    if (qxl->deferred_fps)
        dfps_set_uxa_functions(qxl, screen);
    else
        set_uxa_functions(qxl, screen);

    if (!uxa_driver_init (screen, qxl->uxa))
    {
	xf86DrvMsg (scrn->scrnIndex, X_ERROR,
	            "UXA initialization failed\n");
	free (qxl->uxa);
	return FALSE;
    }

#if 0
    uxa_set_fallback_debug (screen, FALSE);
#endif

#if 0
    if (!uxa_driver_init (screen, qxl->uxa))
	return FALSE;
#endif

    return TRUE;
}
