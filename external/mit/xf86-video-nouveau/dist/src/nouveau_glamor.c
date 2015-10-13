/*
 * Copyright 2014 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs <bskeggs@redhat.com>
 */

#include "nouveau_glamor.h"
#ifdef HAVE_GLAMOR

static DevPrivateKeyRec glamor_private;

void
nouveau_glamor_pixmap_set(PixmapPtr pixmap, struct nouveau_pixmap *priv)
{
	dixSetPrivate(&pixmap->devPrivates, &glamor_private, priv);
}

struct nouveau_pixmap *
nouveau_glamor_pixmap_get(PixmapPtr pixmap)
{
	return dixGetPrivate(&pixmap->devPrivates, &glamor_private);
}

static Bool
nouveau_glamor_destroy_pixmap(PixmapPtr pixmap)
{
	struct nouveau_pixmap *priv = nouveau_glamor_pixmap_get(pixmap);
	if (pixmap->refcnt == 1) {
		glamor_egl_destroy_textured_pixmap(pixmap);
		if (priv)
			nouveau_bo_ref(NULL, &priv->bo);
	}
	fbDestroyPixmap(pixmap);
	return TRUE;
}

static PixmapPtr
nouveau_glamor_create_pixmap(ScreenPtr screen, int w, int h, int depth,
			     unsigned usage)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	struct nouveau_pixmap *priv;
	PixmapPtr pixmap;
	int pitch;

	if (usage != CREATE_PIXMAP_USAGE_SHARED)
		return glamor_create_pixmap(screen, w, h, depth, usage);
	if (depth == 1)
		return fbCreatePixmap(screen, w, h, depth, usage);
	if (w > 32767 || h > 32767)
		return NullPixmap;

	pixmap = fbCreatePixmap(screen, 0, 0, depth, usage);
	if (pixmap == NullPixmap || !w || !h)
		return pixmap;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
		goto fail_priv;

	if (!nouveau_allocate_surface(scrn, w, h,
				     pixmap->drawable.bitsPerPixel,
				     usage, &pitch, &priv->bo))
		goto fail_bo;

	nouveau_glamor_pixmap_set(pixmap, priv);
	screen->ModifyPixmapHeader(pixmap, w, h, 0, 0, pitch, NULL);

	if (!glamor_egl_create_textured_pixmap(pixmap, priv->bo->handle,
					       pixmap->devKind)) {
		xf86DrvMsg(scrn->scrnIndex, X_WARNING,
			   "[GLAMOR] failed to create textured PRIME pixmap.");
		return pixmap;
	}

	return pixmap;
fail_bo:
	free(priv);
fail_priv:
	fbDestroyPixmap(pixmap);
	return fbCreatePixmap(screen, w, h, depth, usage);
}

static Bool
nouveau_glamor_share_pixmap_backing(PixmapPtr pixmap, ScreenPtr slave,
				    void **phandle)
{
	struct nouveau_pixmap *priv = nouveau_glamor_pixmap_get(pixmap);
	int ret, handle;

	ret = nouveau_bo_set_prime(priv->bo, &handle);
	if (ret)
		return FALSE;

	priv->shared = TRUE;
	*phandle = (void *)(long)handle;
	return TRUE;
}

static Bool
nouveau_glamor_set_shared_pixmap_backing(PixmapPtr pixmap, void *_handle)
{
	struct nouveau_pixmap *priv = nouveau_glamor_pixmap_get(pixmap);
	ScreenPtr screen = pixmap->drawable.pScreen;
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);
	struct nouveau_bo *bo;
	int ret, handle = (int)(long)_handle;

	ret = nouveau_bo_prime_handle_ref(pNv->dev, handle, &bo);
	if (ret)
		return FALSE;

	priv->bo = bo;
	priv->shared = TRUE;
	close(handle);

	if (!glamor_egl_create_textured_pixmap(pixmap, priv->bo->handle,
					       pixmap->devKind)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "[GLAMOR] failed to get PRIME drawable\n");
		return FALSE;
	}

	return TRUE;
}

static void
nouveau_glamor_flush(ScrnInfoPtr pScrn)
{
	glamor_block_handler(pScrn->pScreen);
}

Bool
nouveau_glamor_create_screen_resources(ScreenPtr screen)
{
	PixmapPtr ppix = screen->GetScreenPixmap(screen);
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);

	if (!glamor_glyphs_init(screen))
		return FALSE;

	if (!glamor_egl_create_textured_screen_ext(screen,
						   pNv->scanout->handle,
						   scrn->displayWidth *
						   scrn->bitsPerPixel / 8,
						   NULL))
		return FALSE;

	if (!nouveau_glamor_pixmap_get(ppix)) {
		struct nouveau_pixmap *priv = calloc(1, sizeof(*priv));
		if (priv) {
			nouveau_bo_ref(pNv->scanout, &priv->bo);
			nouveau_glamor_pixmap_set(ppix, priv);
		}
	}

	return TRUE;
}

Bool
nouveau_glamor_pre_init(ScrnInfoPtr scrn)
{
	NVPtr pNv = NVPTR(scrn);
	pointer glamor_module;

	if (scrn->depth < 24) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "[GLAMOR] requires depth >= 24\n");
		return FALSE;
	}

	if ((glamor_module = xf86LoadSubModule(scrn, GLAMOR_EGL_MODULE_NAME))) {
		if (!glamor_egl_init(scrn, pNv->dev->fd)) {
			xf86DrvMsg(scrn->scrnIndex, X_ERROR,
				   "[GLAMOR] failed to initialise EGL\n");
			return FALSE;
		}
	} else {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR, "[GLAMOR] unavailable\n");
		return FALSE;
	}

	xf86DrvMsg(scrn->scrnIndex, X_INFO, "[GLAMOR] EGL initialised\n");
	return TRUE;
}

Bool
nouveau_glamor_init(ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	NVPtr pNv = NVPTR(scrn);

	if (!glamor_init(screen, GLAMOR_INVERTED_Y_AXIS |
				 GLAMOR_USE_EGL_SCREEN |
				 GLAMOR_USE_SCREEN |
				 GLAMOR_USE_PICTURE_SCREEN)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "[GLAMOR] failed to initialise\n");
		return FALSE;
	}

	if (!glamor_egl_init_textured_pixmap(screen)) {
		xf86DrvMsg(scrn->scrnIndex, X_ERROR,
			   "[GLAMOR] failed to initialize screen pixmap\n");
		return FALSE;
	}

	if (!dixRegisterPrivateKey(&glamor_private, PRIVATE_PIXMAP, 0))
		return FALSE;

	screen->CreatePixmap = nouveau_glamor_create_pixmap;
	screen->DestroyPixmap = nouveau_glamor_destroy_pixmap;
	screen->SharePixmapBacking = nouveau_glamor_share_pixmap_backing;
	screen->SetSharedPixmapBacking = nouveau_glamor_set_shared_pixmap_backing;

	xf86DrvMsg(scrn->scrnIndex, X_INFO, "[GLAMOR] initialised\n");
	pNv->Flush = nouveau_glamor_flush;
	return TRUE;
}

XF86VideoAdaptorPtr
nouveau_glamor_xv_init(ScreenPtr pScreen, int num_adapt)
{
	return glamor_xv_init(pScreen, num_adapt);
}
#endif
