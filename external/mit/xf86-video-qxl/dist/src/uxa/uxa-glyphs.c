/*
 * Copyright © 2010 Intel Corporation
 * Partly based on code Copyright © 2008 Red Hat, Inc.
 * Partly based on code Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Intel not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Intel makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL INTEL
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Red Hat DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL Red Hat
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 * Based on code by: Keith Packard <keithp@keithp.com> and Owen Taylor <otaylor@fishsoup.net>
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include "uxa-priv.h"

#include "mipict.h"

/* Width of the pixmaps we use for the caches; this should be less than
 * max texture size of the driver; this may need to actually come from
 * the driver.
 */
#define CACHE_PICTURE_SIZE 1024
#define GLYPH_MIN_SIZE 8
#define GLYPH_MAX_SIZE 64
#define GLYPH_CACHE_SIZE (CACHE_PICTURE_SIZE * CACHE_PICTURE_SIZE / (GLYPH_MIN_SIZE * GLYPH_MIN_SIZE))

struct uxa_glyph {
	uxa_glyph_cache_t *cache;
	uint16_t x, y;
	uint16_t size, pos;
};

#if HAS_DEVPRIVATEKEYREC
static DevPrivateKeyRec uxa_glyph_key;
#else
static int uxa_glyph_key;
#endif

static inline struct uxa_glyph *uxa_glyph_get_private(GlyphPtr glyph)
{
#if HAS_DEVPRIVATEKEYREC
	return dixGetPrivate(&glyph->devPrivates, &uxa_glyph_key);
#else
	return dixLookupPrivate(&glyph->devPrivates, &uxa_glyph_key);
#endif
}

static inline void uxa_glyph_set_private(GlyphPtr glyph, struct uxa_glyph *priv)
{
	dixSetPrivate(&glyph->devPrivates, &uxa_glyph_key, priv);
}

#define NeedsComponent(f) (PICT_FORMAT_A(f) != 0 && PICT_FORMAT_RGB(f) != 0)

static void uxa_unrealize_glyph_caches(ScreenPtr pScreen)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pScreen);
	int i;

	for (i = 0; i < UXA_NUM_GLYPH_CACHE_FORMATS; i++) {
		uxa_glyph_cache_t *cache = &uxa_screen->glyphCaches[i];

		if (cache->picture)
			FreePicture(cache->picture, 0);

		if (cache->glyphs)
			free(cache->glyphs);
	}
}

void uxa_glyphs_fini(ScreenPtr pScreen)
{
	uxa_unrealize_glyph_caches(pScreen);
}

/* All caches for a single format share a single pixmap for glyph storage,
 * allowing mixing glyphs of different sizes without paying a penalty
 * for switching between source pixmaps. (Note that for a size of font
 * right at the border between two sizes, we might be switching for almost
 * every glyph.)
 *
 * This function allocates the storage pixmap, and then fills in the
 * rest of the allocated structures for all caches with the given format.
 */
static Bool uxa_realize_glyph_caches(ScreenPtr pScreen)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(pScreen);
	unsigned int formats[] = {
		PIXMAN_a8,
		PIXMAN_a8r8g8b8,
	};
	int i;

	memset(uxa_screen->glyphCaches, 0, sizeof(uxa_screen->glyphCaches));

	for (i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
		uxa_glyph_cache_t *cache = &uxa_screen->glyphCaches[i];
		PixmapPtr pixmap;
		PicturePtr picture;
		CARD32 component_alpha;
		int depth = PIXMAN_FORMAT_DEPTH(formats[i]);
		int error;
		PictFormatPtr pPictFormat = PictureMatchFormat(pScreen, depth, formats[i]);
		if (!pPictFormat)
			goto bail;

		/* Now allocate the pixmap and picture */
		pixmap = pScreen->CreatePixmap(pScreen,
					       CACHE_PICTURE_SIZE, CACHE_PICTURE_SIZE, depth,
					       0 /* INTEL_CREATE_PIXMAP_TILING_X -- FIXME */);
		if (!pixmap)
			goto bail;
#if 0
		assert (uxa_pixmap_is_offscreen(pixmap));
#endif

		component_alpha = NeedsComponent(pPictFormat->format);
		picture = CreatePicture(0, &pixmap->drawable, pPictFormat,
					CPComponentAlpha, &component_alpha,
					serverClient, &error);

		pScreen->DestroyPixmap(pixmap);

		if (!picture)
			goto bail;

		ValidatePicture(picture);

		cache->picture = picture;
		cache->glyphs = calloc(sizeof(GlyphPtr), GLYPH_CACHE_SIZE);
		if (!cache->glyphs)
			goto bail;

		cache->evict = rand() % GLYPH_CACHE_SIZE;
	}
	assert(i == UXA_NUM_GLYPH_CACHE_FORMATS);

	return TRUE;

bail:
	uxa_unrealize_glyph_caches(pScreen);
	return FALSE;
}


Bool uxa_glyphs_init(ScreenPtr pScreen)
{
#if HAS_DIXREGISTERPRIVATEKEY
	if (!dixRegisterPrivateKey(&uxa_glyph_key, PRIVATE_GLYPH, 0))
		return FALSE;
#else
	if (!dixRequestPrivate(&uxa_glyph_key, 0))
		return FALSE;
#endif

	if (!uxa_realize_glyph_caches(pScreen))
		return FALSE;

	return TRUE;
}

/* The most efficient thing to way to upload the glyph to the screen
 * is to use CopyArea; uxa pixmaps are always offscreen.
 */
static void
uxa_glyph_cache_upload_glyph(ScreenPtr screen,
			     uxa_glyph_cache_t * cache,
			     GlyphPtr glyph,
			     int x, int y)
{
	PicturePtr pGlyphPicture = GetGlyphPicture(glyph, screen);
	PixmapPtr pGlyphPixmap = (PixmapPtr) pGlyphPicture->pDrawable;
	PixmapPtr pCachePixmap = (PixmapPtr) cache->picture->pDrawable;
	PixmapPtr scratch;
	GCPtr gc;

	gc = GetScratchGC(pCachePixmap->drawable.depth, screen);
	if (!gc)
		return;

	ValidateGC(&pCachePixmap->drawable, gc);

	scratch = pGlyphPixmap;
	/* Create a temporary bo to stream the updates to the cache */
	if (pGlyphPixmap->drawable.depth != pCachePixmap->drawable.depth ||
	    !uxa_pixmap_is_offscreen(scratch)) {
		scratch = screen->CreatePixmap(screen,
					       glyph->info.width,
					       glyph->info.height,
					       pCachePixmap->drawable.depth,
					       UXA_CREATE_PIXMAP_FOR_MAP);
		if (scratch) {
			if (pGlyphPixmap->drawable.depth != pCachePixmap->drawable.depth) {
				PicturePtr picture;
				int error;

				picture = CreatePicture(0, &scratch->drawable,
							PictureMatchFormat(screen,
									   pCachePixmap->drawable.depth,
									   cache->picture->format),
							0, NULL,
							serverClient, &error);
				if (picture) {
					ValidatePicture(picture);
					uxa_composite(PictOpSrc, pGlyphPicture, NULL, picture,
						      0, 0,
						      0, 0,
						      0, 0,
						      glyph->info.width, glyph->info.height);
					FreePicture(picture, 0);
				}
			} else {
				uxa_copy_area(&pGlyphPixmap->drawable,
					      &scratch->drawable,
					      gc,
					      0, 0,
					      glyph->info.width, glyph->info.height,
					      0, 0);
			}
		} else {
			scratch = pGlyphPixmap;
		}
	}

	uxa_copy_area(&scratch->drawable, &pCachePixmap->drawable, gc,
		      0, 0,
		      glyph->info.width, glyph->info.height,
		      x, y);

	if (scratch != pGlyphPixmap)
		screen->DestroyPixmap(scratch);

	FreeScratchGC(gc);
}

void
uxa_glyph_unrealize(ScreenPtr pScreen,
		    GlyphPtr pGlyph)
{
	struct uxa_glyph *priv;

	priv = uxa_glyph_get_private(pGlyph);
	if (priv == NULL)
		return;

	priv->cache->glyphs[priv->pos] = NULL;

	uxa_glyph_set_private(pGlyph, NULL);
	free(priv);
}

/* Cut and paste from render/glyph.c - probably should export it instead */
static void
uxa_glyph_extents(int nlist,
		  GlyphListPtr list, GlyphPtr * glyphs, BoxPtr extents)
{
	int x1, x2, y1, y2;
	int x, y, n;

	x1 = y1 = MAXSHORT;
	x2 = y2 = MINSHORT;
	x = y = 0;
	while (nlist--) {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		list++;
		while (n--) {
			GlyphPtr glyph = *glyphs++;
			int v;

			v = x - glyph->info.x;
			if (v < x1)
			    x1 = v;
			v += glyph->info.width;
			if (v > x2)
			    x2 = v;

			v = y - glyph->info.y;
			if (v < y1)
			    y1 = v;
			v += glyph->info.height;
			if (v > y2)
			    y2 = v;

			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
	}

	extents->x1 = x1 < MINSHORT ? MINSHORT : x1;
	extents->x2 = x2 > MAXSHORT ? MAXSHORT : x2;
	extents->y1 = y1 < MINSHORT ? MINSHORT : y1;
	extents->y2 = y2 > MAXSHORT ? MAXSHORT : y2;
}

/**
 * Returns TRUE if the glyphs in the lists intersect.  Only checks based on
 * bounding box, which appears to be good enough to catch most cases at least.
 */
static Bool
uxa_glyphs_intersect(int nlist, GlyphListPtr list, GlyphPtr * glyphs)
{
	int x1, x2, y1, y2;
	int n;
	int x, y;
	BoxRec extents;
	Bool first = TRUE;

	x = 0;
	y = 0;
	extents.x1 = 0;
	extents.y1 = 0;
	extents.x2 = 0;
	extents.y2 = 0;
	while (nlist--) {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		list++;
		while (n--) {
			GlyphPtr glyph = *glyphs++;

			if (glyph->info.width == 0 || glyph->info.height == 0) {
				x += glyph->info.xOff;
				y += glyph->info.yOff;
				continue;
			}

			x1 = x - glyph->info.x;
			if (x1 < MINSHORT)
				x1 = MINSHORT;
			y1 = y - glyph->info.y;
			if (y1 < MINSHORT)
				y1 = MINSHORT;
			x2 = x1 + glyph->info.width;
			if (x2 > MAXSHORT)
				x2 = MAXSHORT;
			y2 = y1 + glyph->info.height;
			if (y2 > MAXSHORT)
				y2 = MAXSHORT;

			if (first) {
				extents.x1 = x1;
				extents.y1 = y1;
				extents.x2 = x2;
				extents.y2 = y2;
				first = FALSE;
			} else {
				if (x1 < extents.x2 && x2 > extents.x1 &&
				    y1 < extents.y2 && y2 > extents.y1) {
					return TRUE;
				}

				if (x1 < extents.x1)
					extents.x1 = x1;
				if (x2 > extents.x2)
					extents.x2 = x2;
				if (y1 < extents.y1)
					extents.y1 = y1;
				if (y2 > extents.y2)
					extents.y2 = y2;
			}
			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
	}

	return FALSE;
}

static void
uxa_check_glyphs(CARD8 op,
		 PicturePtr src,
		 PicturePtr dst,
		 PictFormatPtr maskFormat,
		 INT16 xSrc,
		 INT16 ySrc, int nlist, GlyphListPtr list, GlyphPtr * glyphs)
{
	ScreenPtr pScreen = dst->pDrawable->pScreen;
	pixman_image_t *image;
	PixmapPtr scratch;
	PicturePtr mask;
	int width = 0, height = 0;
	int x, y, n;
	int xDst = list->xOff, yDst = list->yOff;
	BoxRec extents = { 0, 0, 0, 0 };

	if (maskFormat) {
		pixman_format_code_t format;
		CARD32 component_alpha;
		int error;

		uxa_glyph_extents(nlist, list, glyphs, &extents);
		if (extents.x2 <= extents.x1 || extents.y2 <= extents.y1)
			return;

		width = extents.x2 - extents.x1;
		height = extents.y2 - extents.y1;

		format = maskFormat->format |
			(BitsPerPixel(maskFormat->depth) << 24);
		image =
			pixman_image_create_bits(format, width, height, NULL, 0);
		if (!image)
			return;

		scratch = GetScratchPixmapHeader(dst->pDrawable->pScreen, width, height,
						 PIXMAN_FORMAT_DEPTH(format),
						 PIXMAN_FORMAT_BPP(format),
						 pixman_image_get_stride(image),
						 pixman_image_get_data(image));

		if (!scratch) {
			pixman_image_unref(image);
			return;
		}

		component_alpha = NeedsComponent(maskFormat->format);
		mask = CreatePicture(0, &scratch->drawable,
				     maskFormat, CPComponentAlpha,
				     &component_alpha, serverClient, &error);
		if (!mask) {
			FreeScratchPixmapHeader(scratch);
			pixman_image_unref(image);
			return;
		}
		ValidatePicture(mask);

		x = -extents.x1;
		y = -extents.y1;
	} else {
		mask = dst;
		x = 0;
		y = 0;
	}

	while (nlist--) {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		while (n--) {
			GlyphPtr glyph = *glyphs++;
			PicturePtr g = GetGlyphPicture(glyph, pScreen);
			if (g) {
				if (maskFormat) {
					CompositePicture(PictOpAdd, g, NULL, mask,
							 0, 0,
							 0, 0,
							 x - glyph->info.x,
							 y - glyph->info.y,
							 glyph->info.width,
							 glyph->info.height);
				} else {
					CompositePicture(op, src, g, dst,
							 xSrc + (x - glyph->info.x) - xDst,
							 ySrc + (y - glyph->info.y) - yDst,
							 0, 0,
							 x - glyph->info.x,
							 y - glyph->info.y,
							 glyph->info.width,
							 glyph->info.height);
				}
			}

			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
		list++;
	}

	if (maskFormat) {
		x = extents.x1;
		y = extents.y1;
		CompositePicture(op, src, mask, dst,
				 xSrc + x - xDst,
				 ySrc + y - yDst,
				 0, 0,
				 x, y,
				 width, height);
		FreePicture(mask, 0);
		FreeScratchPixmapHeader(scratch);
		pixman_image_unref(image);
	}
}

static inline unsigned int
uxa_glyph_size_to_count(int size)
{
	size /= GLYPH_MIN_SIZE;
	return size * size;
}

static inline unsigned int
uxa_glyph_count_to_mask(int count)
{
	return ~(count - 1);
}

static inline unsigned int
uxa_glyph_size_to_mask(int size)
{
	return uxa_glyph_count_to_mask(uxa_glyph_size_to_count(size));
}

static PicturePtr
uxa_glyph_cache(ScreenPtr screen, GlyphPtr glyph, int *out_x, int *out_y)
{
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	PicturePtr glyph_picture = GetGlyphPicture(glyph, screen);
	uxa_glyph_cache_t *cache = &uxa_screen->glyphCaches[PICT_FORMAT_RGB(glyph_picture->format) != 0];
	struct uxa_glyph *priv = NULL;
	int size, mask, pos, s;

	if (glyph->info.width > GLYPH_MAX_SIZE || glyph->info.height > GLYPH_MAX_SIZE)
		return NULL;

	for (size = GLYPH_MIN_SIZE; size <= GLYPH_MAX_SIZE; size *= 2)
		if (glyph->info.width <= size && glyph->info.height <= size)
			break;

	s = uxa_glyph_size_to_count(size);
	mask = uxa_glyph_count_to_mask(s);
	pos = (cache->count + s - 1) & mask;
	if (pos < GLYPH_CACHE_SIZE) {
		cache->count = pos + s;
	} else {
		for (s = size; s <= GLYPH_MAX_SIZE; s *= 2) {
			int i = cache->evict & uxa_glyph_size_to_mask(s);
			GlyphPtr evicted = cache->glyphs[i];
			if (evicted == NULL)
				continue;

			priv = uxa_glyph_get_private(evicted);
			if (priv->size >= s) {
				cache->glyphs[i] = NULL;
				uxa_glyph_set_private(evicted, NULL);
				pos = cache->evict & uxa_glyph_size_to_mask(size);
			} else
				priv = NULL;
			break;
		}
		if (priv == NULL) {
			int count = uxa_glyph_size_to_count(size);
			mask = uxa_glyph_count_to_mask(count);
			pos = cache->evict & mask;
			for (s = 0; s < count; s++) {
				GlyphPtr evicted = cache->glyphs[pos + s];
				if (evicted != NULL) {
					if (priv != NULL)
						free(priv);

					priv = uxa_glyph_get_private(evicted);
					uxa_glyph_set_private(evicted, NULL);
					cache->glyphs[pos + s] = NULL;
				}
			}
		}

		/* And pick a new eviction position */
		cache->evict = rand() % GLYPH_CACHE_SIZE;
	}

	if (priv == NULL) {
		priv = malloc(sizeof(struct uxa_glyph));
		if (priv == NULL)
			return NULL;
	}

	uxa_glyph_set_private(glyph, priv);
	cache->glyphs[pos] = glyph;

	priv->cache = cache;
	priv->size = size;
	priv->pos = pos;
	s = pos / ((GLYPH_MAX_SIZE / GLYPH_MIN_SIZE) * (GLYPH_MAX_SIZE / GLYPH_MIN_SIZE));
	priv->x = s % (CACHE_PICTURE_SIZE / GLYPH_MAX_SIZE) * GLYPH_MAX_SIZE;
	priv->y = (s / (CACHE_PICTURE_SIZE / GLYPH_MAX_SIZE)) * GLYPH_MAX_SIZE;
	for (s = GLYPH_MIN_SIZE; s < GLYPH_MAX_SIZE; s *= 2) {
		if (pos & 1)
			priv->x += s;
		if (pos & 2)
			priv->y += s;
		pos >>= 2;
	}

	uxa_glyph_cache_upload_glyph(screen, cache, glyph, priv->x, priv->y);

	*out_x = priv->x;
	*out_y = priv->y;
	return cache->picture;
}

static int
uxa_glyphs_to_dst(CARD8 op,
		  PicturePtr pSrc,
		  PicturePtr pDst,
		  INT16 src_x, INT16 src_y,
		  INT16 xDst, INT16 yDst,
		  int nlist, GlyphListPtr list, GlyphPtr * glyphs,
		  BoxPtr extents)
{
	ScreenPtr screen = pDst->pDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	PixmapPtr src_pixmap, dst_pixmap;
	PicturePtr localSrc, glyph_atlas;
	int x, y, n, nrect;
	BoxRec box;

	if (uxa_screen->info->check_composite_texture &&
	    uxa_screen->info->check_composite_texture(screen, pSrc)) {
		if (pSrc->pDrawable) {
			int src_off_x, src_off_y;

			src_pixmap = uxa_get_offscreen_pixmap(pSrc->pDrawable, &src_off_x, &src_off_y);
			if (src_pixmap == NULL)
				return -1;

			src_x += pSrc->pDrawable->x + src_off_x;
			src_y += pSrc->pDrawable->y + src_off_y;
		} else {
			src_pixmap = NULL;
		}
		localSrc = pSrc;
	} else {
		int width, height;

		if (extents == NULL) {
			uxa_glyph_extents(nlist, list, glyphs, &box);
			extents = &box;
		}

		width  = extents->x2 - extents->x1;
		height = extents->y2 - extents->y1;
		if (width == 0 || height == 0)
			return 0;

		if (pSrc->pDrawable) {
			int src_off_x, src_off_y;

			src_off_x = extents->x1 - xDst;
			src_off_y = extents->y1 - yDst;
			localSrc = uxa_acquire_drawable(screen, pSrc,
							src_x + src_off_x, src_y + src_off_y,
							width, height,
							&src_x, &src_y);
			if (uxa_screen->info->check_composite_texture &&
			    !uxa_screen->info->check_composite_texture(screen, localSrc)) {
				if (localSrc != pSrc)
					FreePicture(localSrc, 0);
				return -1;
			}

			src_pixmap = uxa_get_offscreen_pixmap(localSrc->pDrawable, &src_off_x, &src_off_y);
			if (src_pixmap == NULL) {
				if (localSrc != pSrc)
					FreePicture(localSrc, 0);
				return -1;
			}

			src_x += localSrc->pDrawable->x + src_off_x;
			src_y += localSrc->pDrawable->y + src_off_y;
		} else {
			localSrc = uxa_acquire_pattern(screen, pSrc,
						       PIXMAN_a8r8g8b8, 0, 0, width, height);
			if (!localSrc)
				return 1;

			src_pixmap = uxa_get_drawable_pixmap(localSrc->pDrawable);
			if (src_pixmap == NULL) {
				FreePicture(localSrc, 0);
				return -1;
			}

			src_x = src_y = 0;
		}
	}

	dst_pixmap = uxa_get_offscreen_pixmap(pDst->pDrawable, &x, &y);
	x += xDst + pDst->pDrawable->x - list->xOff;
	y += yDst + pDst->pDrawable->y - list->yOff;

	glyph_atlas = NULL;
	while (nlist--) {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		while (n--) {
			GlyphPtr glyph = *glyphs++;
			PicturePtr this_atlas;
			int mask_x, mask_y;
			struct uxa_glyph *priv;

			if (glyph->info.width == 0 || glyph->info.height == 0)
				goto next_glyph;

			priv = uxa_glyph_get_private(glyph);
			if (priv != NULL) {
				mask_x = priv->x;
				mask_y = priv->y;
				this_atlas = priv->cache->picture;
			} else {
				if (glyph_atlas) {
					uxa_screen->info->done_composite(dst_pixmap);
					glyph_atlas = NULL;
				}
				this_atlas = uxa_glyph_cache(screen, glyph, &mask_x, &mask_y);
				if (this_atlas == NULL) {
					/* no cache for this glyph */
					this_atlas = GetGlyphPicture(glyph, screen);
					mask_x = mask_y = 0;
				}
			}

			if (this_atlas != glyph_atlas) {
				PixmapPtr mask_pixmap;

				if (glyph_atlas)
					uxa_screen->info->done_composite(dst_pixmap);

				mask_pixmap =
					uxa_get_drawable_pixmap(this_atlas->pDrawable);
				assert (uxa_pixmap_is_offscreen(mask_pixmap));

				if (!uxa_screen->info->prepare_composite(op,
									 localSrc, this_atlas, pDst,
									 src_pixmap, mask_pixmap, dst_pixmap))
					return -1;

				glyph_atlas = this_atlas;
			}

			nrect = REGION_NUM_RECTS(pDst->pCompositeClip);
			if (nrect == 1) {
				uxa_screen->info->composite(dst_pixmap,
							    x + src_x, y + src_y,
							    mask_x, mask_y,
							    x - glyph->info.x,
							    y - glyph->info.y,
							    glyph->info.width,
							    glyph->info.height);
			} else {
				BoxPtr rects = REGION_RECTS(pDst->pCompositeClip);
				do {
					int x1 = x - glyph->info.x, dx = 0;
					int y1 = y - glyph->info.y, dy = 0;
					int x2 = x1 + glyph->info.width;
					int y2 = y1 + glyph->info.height;

					if (x1 < rects->x1)
						dx = rects->x1 - x1, x1 = rects->x1;
					if (x2 > rects->x2)
						x2 = rects->x2;
					if (y1 < rects->y1)
						dy = rects->y1 - y1, y1 = rects->y1;
					if (y2 > rects->y2)
						y2 = rects->y2;

					if (x1 < x2 && y1 < y2) {
						uxa_screen->info->composite(dst_pixmap,
									    x1 + src_x, y1 + src_y,
									    dx + mask_x, dy + mask_y,
									    x1, y1,
									    x2 - x1, y2 - y1);
					}
					rects++;
				} while (--nrect);
			}

next_glyph:
			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
		list++;
	}
	if (glyph_atlas)
		uxa_screen->info->done_composite(dst_pixmap);

	if (localSrc != pSrc)
		FreePicture(localSrc, 0);

	return 0;
}

static void
uxa_clear_pixmap(ScreenPtr screen,
		 uxa_screen_t *uxa_screen,
		 PixmapPtr pixmap)
{
	if (uxa_screen->info->check_solid &&
	    !uxa_screen->info->check_solid(&pixmap->drawable, GXcopy, FB_ALLONES))
		goto fallback;

	if (!uxa_screen->info->prepare_solid(pixmap, GXcopy, FB_ALLONES, 0))
		goto fallback;

	uxa_screen->info->solid(pixmap,
				0, 0,
				pixmap->drawable.width,
				pixmap->drawable.height);

	uxa_screen->info->done_solid(pixmap);
	return;

fallback:
	{
		GCPtr gc;

		gc = GetScratchGC(pixmap->drawable.depth, screen);
		if (gc) {
			xRectangle rect;

			ValidateGC(&pixmap->drawable, gc);

			rect.x = 0;
			rect.y = 0;
			rect.width  = pixmap->drawable.width;
			rect.height = pixmap->drawable.height;
			gc->ops->PolyFillRect(&pixmap->drawable, gc, 1, &rect);

			FreeScratchGC(gc);
		}
	}
}

static int
uxa_glyphs_via_mask(CARD8 op,
		    PicturePtr pSrc,
		    PicturePtr pDst,
		    PictFormatPtr maskFormat,
		    INT16 xSrc, INT16 ySrc,
		    INT16 xDst, INT16 yDst,
		    int nlist, GlyphListPtr list, GlyphPtr * glyphs,
		    BoxPtr extents)
{
	ScreenPtr screen = pDst->pDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	CARD32 component_alpha;
	PixmapPtr pixmap;
	PicturePtr glyph_atlas, mask;
	int x, y, width, height;
	int dst_off_x, dst_off_y;
	int n, error;
	BoxRec box;

	if (!extents) {
		uxa_glyph_extents(nlist, list, glyphs, &box);

		if (box.x2 <= box.x1 || box.y2 <= box.y1)
			return 0;

		extents = &box;
		dst_off_x = box.x1;
		dst_off_y = box.y1;
	} else {
		dst_off_x = dst_off_y = 0;
	}

	width  = extents->x2 - extents->x1;
	height = extents->y2 - extents->y1;
	x = -extents->x1;
	y = -extents->y1;

	if (maskFormat->depth == 1) {
		PictFormatPtr a8Format =
			PictureMatchFormat(screen, 8, PICT_a8);

		if (!a8Format)
			return -1;

		maskFormat = a8Format;
	}

	pixmap = screen->CreatePixmap(screen, width, height,
				      maskFormat->depth,
				      CREATE_PIXMAP_USAGE_SCRATCH);
	if (!pixmap)
		return 1;

	uxa_clear_pixmap(screen, uxa_screen, pixmap);

	if (!uxa_pixmap_is_offscreen(pixmap)) {
		screen->DestroyPixmap(pixmap);
		return 1;
	}
	
	component_alpha = NeedsComponent(maskFormat->format);
	mask = CreatePicture(0, &pixmap->drawable,
			      maskFormat, CPComponentAlpha,
			      &component_alpha, serverClient, &error);
	screen->DestroyPixmap(pixmap);

	if (!mask)
		return 1;

	ValidatePicture(mask);

	glyph_atlas = NULL;
	while (nlist--) {
		x += list->xOff;
		y += list->yOff;
		n = list->len;
		while (n--) {
			GlyphPtr glyph = *glyphs++;
			PicturePtr this_atlas;
			int src_x, src_y;
			struct uxa_glyph *priv;

			if (glyph->info.width == 0 || glyph->info.height == 0)
				goto next_glyph;

			priv = uxa_glyph_get_private(glyph);
			if (priv != NULL) {
				src_x = priv->x;
				src_y = priv->y;
				this_atlas = priv->cache->picture;
			} else {
				if (glyph_atlas) {
					uxa_screen->info->done_composite(pixmap);
					glyph_atlas = NULL;
				}
				this_atlas = uxa_glyph_cache(screen, glyph, &src_x, &src_y);
				if (this_atlas == NULL) {
					/* no cache for this glyph */
					this_atlas = GetGlyphPicture(glyph, screen);
					src_x = src_y = 0;
				}
			}

			if (this_atlas != glyph_atlas) {
				PixmapPtr src_pixmap;

				if (glyph_atlas)
					uxa_screen->info->done_composite(pixmap);

				src_pixmap =
					uxa_get_drawable_pixmap(this_atlas->pDrawable);
				assert (uxa_pixmap_is_offscreen(src_pixmap));

				if (!uxa_screen->info->prepare_composite(PictOpAdd,
									 this_atlas, NULL, mask,
									 src_pixmap, NULL, pixmap)) {
				        FreePicture(mask, 0);
					return -1;
				}

				glyph_atlas = this_atlas;
			}

			uxa_screen->info->composite(pixmap,
						    src_x, src_y,
						    0, 0,
						    x - glyph->info.x,
						    y - glyph->info.y,
						    glyph->info.width,
						    glyph->info.height);

next_glyph:
			x += glyph->info.xOff;
			y += glyph->info.yOff;
		}
		list++;
	}
	if (glyph_atlas)
		uxa_screen->info->done_composite(pixmap);

	uxa_composite(op,
		      pSrc, mask, pDst,
		      dst_off_x + xSrc - xDst,
		      dst_off_y + ySrc - yDst,
		      0, 0,
		      dst_off_x, dst_off_y,
		      width, height);

	FreePicture(mask, 0);
	return 0;
}

void
uxa_glyphs(CARD8 op,
	   PicturePtr pSrc,
	   PicturePtr pDst,
	   PictFormatPtr maskFormat,
	   INT16 xSrc, INT16 ySrc,
	   int nlist, GlyphListPtr list, GlyphPtr * glyphs)
{
	ScreenPtr screen = pDst->pDrawable->pScreen;
	uxa_screen_t *uxa_screen = uxa_get_screen(screen);
	int xDst = list->xOff, yDst = list->yOff;
	BoxRec extents = { 0, 0, 0, 0 };
	Bool have_extents = FALSE;
	int width = 0, height = 0, ret;
	PicturePtr localDst = pDst;

	if (!uxa_screen->info->prepare_composite ||
	    uxa_screen->swappedOut ||
	    uxa_screen->force_fallback ||
	    !uxa_drawable_is_offscreen(pDst->pDrawable) ||
	    pDst->alphaMap || pSrc->alphaMap) {
fallback:
	    uxa_check_glyphs(op, pSrc, pDst, maskFormat, xSrc, ySrc, nlist, list, glyphs);
	    return;
	}

	/* basic sanity check */
	if (uxa_screen->info->check_composite &&
	    !uxa_screen->info->check_composite(op, pSrc, NULL, pDst, 0, 0)) {
		goto fallback;
	}

	ValidatePicture(pSrc);
	ValidatePicture(pDst);

	if (!maskFormat) {
		/* If we don't have a mask format but all the glyphs have the same format,
		 * require ComponentAlpha and don't intersect, use the glyph format as mask
		 * format for the full benefits of the glyph cache.
		 */
		if (NeedsComponent(list[0].format->format)) {
			Bool sameFormat = TRUE;
			int i;

			maskFormat = list[0].format;

			for (i = 0; i < nlist; i++) {
				if (maskFormat->format != list[i].format->format) {
					sameFormat = FALSE;
					break;
				}
			}

			if (!sameFormat ||
			    uxa_glyphs_intersect(nlist, list, glyphs))
				maskFormat = NULL;
		}
	}

	if (!maskFormat &&
	    uxa_screen->info->check_composite_target &&
	    !uxa_screen->info->check_composite_target(uxa_get_drawable_pixmap(pDst->pDrawable))) {
		int depth = pDst->pDrawable->depth;
		PixmapPtr pixmap;
		int x, y, error;
		GCPtr gc;

		pixmap = uxa_get_drawable_pixmap(pDst->pDrawable);
		if (uxa_screen->info->check_copy &&
		    !uxa_screen->info->check_copy(pixmap, pixmap, GXcopy, FB_ALLONES))
			goto fallback;

		uxa_glyph_extents(nlist, list, glyphs, &extents);

		/* clip against dst bounds */
		if (extents.x1 < 0)
			extents.x1 = 0;
		if (extents.y1 < 0)
			extents.y1 = 0;
		if (extents.x2 > pDst->pDrawable->width)
			extents.x2 = pDst->pDrawable->width;
		if (extents.y2 > pDst->pDrawable->height)
			extents.y2 = pDst->pDrawable->height;

		if (extents.x2 <= extents.x1 || extents.y2 <= extents.y1)
			return;
		width  = extents.x2 - extents.x1;
		height = extents.y2 - extents.y1;
		x = -extents.x1;
		y = -extents.y1;
		have_extents = TRUE;

		xDst += x;
		yDst += y;

		pixmap = screen->CreatePixmap(screen,
					      width, height, depth,
					      CREATE_PIXMAP_USAGE_SCRATCH);
		if (!pixmap)
			return;

		gc = GetScratchGC(depth, screen);
		if (!gc) {
			screen->DestroyPixmap(pixmap);
			return;
		}

		ValidateGC(&pixmap->drawable, gc);
		gc->ops->CopyArea(pDst->pDrawable, &pixmap->drawable, gc,
				  extents.x1, extents.y1,
				  width, height,
				  0, 0);
		FreeScratchGC(gc);

		localDst = CreatePicture(0, &pixmap->drawable,
					 PictureMatchFormat(screen, depth, pDst->format),
					 0, 0, serverClient, &error);
		screen->DestroyPixmap(pixmap);

		if (!localDst)
			return;

		ValidatePicture(localDst);
	}

	if (maskFormat) {
		ret = uxa_glyphs_via_mask(op,
					  pSrc, localDst, maskFormat,
					  xSrc, ySrc,
					  xDst, yDst,
					  nlist, list, glyphs,
					  have_extents ? &extents : NULL);
	} else {
		ret = uxa_glyphs_to_dst(op,
					pSrc, localDst,
					xSrc, ySrc,
					xDst, yDst,
					nlist, list, glyphs,
					have_extents ? &extents : NULL);
	}
	if (ret) {
		if (localDst != pDst)
			FreePicture(localDst, 0);

		goto fallback;
	}

	if (localDst != pDst) {
		GCPtr gc;

		gc = GetScratchGC(pDst->pDrawable->depth, screen);
		if (gc) {
			ValidateGC(pDst->pDrawable, gc);
			gc->ops->CopyArea(localDst->pDrawable, pDst->pDrawable, gc,
					  0, 0,
					  width, height,
					  extents.x1, extents.y1);
			FreeScratchGC(gc);
		}

		FreePicture(localDst, 0);
	}
}
