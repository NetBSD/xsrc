/*
 * Copyright © 2009 Red Hat, Inc.
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
 */

/* Authors:
 *    Ben Skeggs <bskeggs@redhat.com>
 */

#include "nv_include.h"

struct wfb_pixmap {
	PixmapPtr ppix;
	unsigned long base;
	unsigned long end;
	unsigned pitch;
	unsigned tile_height;
	unsigned horiz_tiles;
	uint64_t multiply_factor;
};

static struct wfb_pixmap wfb_pixmap[6];

static inline FbBits
nouveau_wfb_rd_linear(const void *src, int size)
{
	FbBits bits = 0;

	memcpy(&bits, src, size);
	return bits;
}

static inline void
nouveau_wfb_wr_linear(void *dst, FbBits value, int size)
{
	memcpy(dst, &value, size);
}

#define TP 6
#define TH wfb->tile_height
#define TPM ((1 << TP) - 1)
#define THM ((1 << TH) - 1)

static FbBits
nouveau_wfb_rd_tiled(const void *ptr, int size) {
	unsigned long offset = (unsigned long)ptr;
	struct wfb_pixmap *wfb = NULL;
	FbBits bits = 0;
	int x, y, i;

	for (i = 0; i < 6; i++) {
		if (offset >= wfb_pixmap[i].base &&
		    offset < wfb_pixmap[i].end) {
			wfb = &wfb_pixmap[i];
			break;
		}
	}

	if (!wfb || !wfb->pitch)
		return nouveau_wfb_rd_linear(ptr, size);

	offset -= wfb->base;

	y = (offset * wfb->multiply_factor) >> 36;
	x = offset - y * wfb->pitch;

	offset  = (x >> TP) + ((y >> TH) * wfb->horiz_tiles);
	offset *= (1 << (TH + TP));
	offset += ((y & THM) * (1 << TP)) + (x & TPM);

	memcpy(&bits, (void *)wfb->base + offset, size);
	return bits;
}

static void
nouveau_wfb_wr_tiled(void *ptr, FbBits value, int size) {
	unsigned long offset = (unsigned long)ptr;
	struct wfb_pixmap *wfb = NULL;
	int x, y, i;

	for (i = 0; i < 6; i++) {
		if (offset >= wfb_pixmap[i].base &&
		    offset < wfb_pixmap[i].end) {
			wfb = &wfb_pixmap[i];
			break;
		}
	}

	if (!wfb || !wfb->pitch) {
		nouveau_wfb_wr_linear(ptr, value, size);
		return;
	}

	offset -= wfb->base;

	y = (offset * wfb->multiply_factor) >> 36;
	x = offset - y * wfb->pitch;

	offset  = (x >> TP) + ((y >> TH) * wfb->horiz_tiles);
	offset *= (1 << (TH + TP));
	offset += ((y & THM) * (1 << TP)) + (x & TPM);

	memcpy((void *)wfb->base + offset, &value, size);
}

void
nouveau_wfb_setup_wrap(ReadMemoryProcPtr *pRead, WriteMemoryProcPtr *pWrite,
		       DrawablePtr pDraw)
{
	struct nouveau_bo *bo = NULL;
	struct wfb_pixmap *wfb;
	PixmapPtr ppix = NULL;
	int i, j, have_tiled = 0;

	if (!pRead || !pWrite)
		return;

	ppix = NVGetDrawablePixmap(pDraw);
	if (ppix) {
		struct nouveau_pixmap *priv = nouveau_pixmap(ppix);
		bo = priv ? priv->bo : NULL;
	}

	if (!ppix || !bo) {
		for (i = 0; i < 6; i++)
			if (wfb_pixmap[i].ppix && wfb_pixmap[i].pitch)
				have_tiled = 1;

		if (have_tiled) {
			*pRead = nouveau_wfb_rd_tiled;
			*pWrite = nouveau_wfb_wr_tiled;
		} else {
			*pRead = nouveau_wfb_rd_linear;
			*pWrite = nouveau_wfb_wr_linear;
		}
		return;
	}

	j = -1;
	for (i = 0; i < 6; i++) {
		if (!wfb_pixmap[i].ppix && j < 0)
			j = i;

		if (wfb_pixmap[i].ppix && wfb_pixmap[i].pitch)
			have_tiled = 1;
	}

	if (j == -1) {
		ErrorF("We ran out of wfb indices, this is not good.\n");
		goto out;
	}

	wfb = &wfb_pixmap[j];

	wfb->ppix = ppix;
	wfb->base = (unsigned long)ppix->devPrivate.ptr;
	wfb->end = wfb->base + bo->size;
	if (!nv50_style_tiled_pixmap(ppix)) {
		wfb->pitch = 0;
	} else {
		wfb->pitch = ppix->devKind;
		/* 8192x8192x4 is 28 bits max, 64 - 28 == 36. */
		wfb->multiply_factor = (((1ULL << 36) - 1) / wfb->pitch) + 1;
		if (bo->device->chipset < 0xc0)
			wfb->tile_height = (bo->config.nv50.tile_mode >> 4) + 2;
		else
			wfb->tile_height = (bo->config.nv50.tile_mode >> 4) + 3;
		wfb->horiz_tiles = wfb->pitch / 64;
		have_tiled = 1;
	}

out:
	if (have_tiled) {
		*pRead = nouveau_wfb_rd_tiled;
		*pWrite = nouveau_wfb_wr_tiled;
	} else {
		*pRead = nouveau_wfb_rd_linear;
		*pWrite = nouveau_wfb_wr_linear;
	}
}

void
nouveau_wfb_finish_wrap(DrawablePtr pDraw)
{
	PixmapPtr ppix;
	int i;

	ppix = NVGetDrawablePixmap(pDraw);
	if (!ppix)
		return;

	for (i = 0; i < 6; i++) {
		if (wfb_pixmap[i].ppix == ppix) {
			wfb_pixmap[i].ppix = NULL;
			wfb_pixmap[i].base = ~0UL;
			break;
		}
	}
}

