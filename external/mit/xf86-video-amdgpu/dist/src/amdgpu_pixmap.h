/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including
 * the next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMDGPU_PIXMAP_H
#define AMDGPU_PIXMAP_H

#include "amdgpu_drv.h"

struct amdgpu_pixmap {
	uint_fast32_t gpu_read;
	uint_fast32_t gpu_write;

	uint64_t tiling_info;

	struct amdgpu_buffer *bo;

	/* GEM handle for pixmaps shared via DRI2/3 */
	Bool handle_valid;
	uint32_t handle;
};

extern DevPrivateKeyRec amdgpu_pixmap_index;

static inline struct amdgpu_pixmap *amdgpu_get_pixmap_private(PixmapPtr pixmap)
{
	return dixGetPrivate(&pixmap->devPrivates, &amdgpu_pixmap_index);
}

static inline void amdgpu_set_pixmap_private(PixmapPtr pixmap,
					     struct amdgpu_pixmap *priv)
{
	dixSetPrivate(&pixmap->devPrivates, &amdgpu_pixmap_index, priv);
}

static inline Bool amdgpu_set_pixmap_bo(PixmapPtr pPix, struct amdgpu_buffer *bo)
{
	struct amdgpu_pixmap *priv;

	priv = amdgpu_get_pixmap_private(pPix);
	if (priv == NULL && bo == NULL)
		return TRUE;

	if (priv) {
		if (priv->bo) {
			if (priv->bo == bo)
				return TRUE;

			amdgpu_bo_unref(&priv->bo);
			priv->handle_valid = FALSE;
		}

		if (!bo) {
			free(priv);
			priv = NULL;
		}
	}

	if (bo) {
		if (!priv) {
			priv = calloc(1, sizeof(struct amdgpu_pixmap));
			if (!priv)
				return FALSE;
		}
		amdgpu_bo_ref(bo);
		priv->bo = bo;
	}

	amdgpu_set_pixmap_private(pPix, priv);
	return TRUE;
}

static inline struct amdgpu_buffer *amdgpu_get_pixmap_bo(PixmapPtr pPix)
{
	struct amdgpu_pixmap *priv;
	priv = amdgpu_get_pixmap_private(pPix);
	return priv ? priv->bo : NULL;
}

enum {
	AMDGPU_CREATE_PIXMAP_DRI2    = 0x08000000,
	AMDGPU_CREATE_PIXMAP_LINEAR  = 0x04000000,
	AMDGPU_CREATE_PIXMAP_SCANOUT = 0x02000000,
	AMDGPU_CREATE_PIXMAP_GTT     = 0x01000000,
};

extern Bool amdgpu_pixmap_init(ScreenPtr screen);

#endif /* AMDGPU_PIXMAP_H */
