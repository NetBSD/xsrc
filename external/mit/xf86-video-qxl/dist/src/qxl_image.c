/*
 * Copyright 2009, 2010 Red Hat, Inc.
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
 *
 */
/** \file QXLImage.c
 * \author Søren Sandmann <sandmann@redhat.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <spice/macros.h>

#include "qxl.h"
#include "murmurhash3.h"

static unsigned int
hash_and_copy (const uint8_t *src, int src_stride,
	       uint8_t *dest, int dest_stride,
	       int bytes_per_pixel, int width, int height,
	       uint32_t hash)
{
    int i;
  
    for (i = 0; i < height; ++i)
    {
	const uint8_t *src_line = src + i * src_stride;
	uint8_t *dest_line = dest + i * dest_stride;
	int n_bytes = width * bytes_per_pixel;
	if (n_bytes > src_stride)
	    n_bytes = src_stride;

	if (dest)
	    memcpy (dest_line, src_line, n_bytes);

	MurmurHash3_x86_32 (src_line, n_bytes, hash, &hash);
    }

    return hash;
}

struct qxl_bo *
qxl_image_create (qxl_screen_t *qxl, const uint8_t *data,
		  int x, int y, int width, int height,
		  int stride, int Bpp, Bool fallback)
{
	uint32_t hash;
	struct QXLImage *image;
	struct qxl_bo *head_bo, *tail_bo;
	struct qxl_bo *image_bo;
	int dest_stride = (width * Bpp + 3) & (~3);
	int h;
	int chunk_size;

	data += y * stride + x * Bpp;

#if 0
	ErrorF ("Must create new image of size %d %d\n", width, height);
#endif
	
	/* Chunk */

	/* FIXME: Check integer overflow */

	head_bo = tail_bo = NULL;

	hash = 0;
	h = height;

	chunk_size = MAX (512 * 512, dest_stride);

#ifdef XF86DRM_MODE
	/* ensure we will not create too many pieces and overflow
	 * the command buffer (MAX_RELOCS).  if so, increase the chunk_size.
	 * each loop creates at least 2 cmd buffer entries, and
	 * we have to leave room when we're done.
	 */
	if (height / (chunk_size / dest_stride) > (MAX_RELOCS / 4)) {
		chunk_size = height / (MAX_RELOCS/4) * dest_stride;
#if 0
		ErrorF ("adjusted chunk_size to %d\n", chunk_size);
#endif
	}
#endif

	while (h)
	{
	    int n_lines = MIN ((chunk_size / dest_stride), h);
	    struct qxl_bo *bo = qxl->bo_funcs->bo_alloc (qxl, sizeof (QXLDataChunk) + n_lines * dest_stride, "image data");

	    QXLDataChunk *chunk = qxl->bo_funcs->bo_map(bo);
	    chunk->data_size = n_lines * dest_stride;
	    hash = hash_and_copy (data, stride,
				  chunk->data, dest_stride,
				  Bpp, width, n_lines, hash);
	    
	    if (tail_bo)
	    {
		qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDataChunk, next_chunk),
					       tail_bo, bo);
		qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDataChunk, prev_chunk),
					       bo, tail_bo);

		chunk->next_chunk = 0;
		
		tail_bo = bo;
	    }
	    else
	    {
		head_bo = tail_bo = bo;
		chunk->next_chunk = 0;
		chunk->prev_chunk = 0;
	    }

	    qxl->bo_funcs->bo_unmap(bo);
	    if (bo != head_bo)
		qxl->bo_funcs->bo_decref(qxl, bo);
	    data += n_lines * stride;
	    h -= n_lines;
	}

	/* Image */
	image_bo = qxl->bo_funcs->bo_alloc (qxl, sizeof *image, "image struct");
	image = qxl->bo_funcs->bo_map(image_bo);

	image->descriptor.id = 0;
	image->descriptor.type = SPICE_IMAGE_TYPE_BITMAP;
	
	image->descriptor.flags = 0;
	image->descriptor.width = width;
	image->descriptor.height = height;

	if (Bpp == 2)
	{
	    image->bitmap.format = SPICE_BITMAP_FMT_16BIT;
	}
	else if (Bpp == 1)
	{
	    image->bitmap.format = SPICE_BITMAP_FMT_8BIT_A;
	}
	else if (Bpp == 4)
	{
	    image->bitmap.format = SPICE_BITMAP_FMT_RGBA;
	}
	else
	{
	    abort();
	}

	image->bitmap.flags = SPICE_BITMAP_FLAGS_TOP_DOWN;
	image->bitmap.x = width;
	image->bitmap.y = height;
	image->bitmap.stride = dest_stride;
	image->bitmap.palette = 0;
	qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLImage, bitmap.data),
				       image_bo, head_bo);

	qxl->bo_funcs->bo_decref(qxl, head_bo);
	/* Add to hash table if caching is enabled */
	if ((fallback && qxl->enable_fallback_cache)	||
	    (!fallback && qxl->enable_image_cache))
	{
            image->descriptor.id = hash;
            image->descriptor.flags = QXL_IMAGE_CACHE;
#if 0
            ErrorF ("added with hash %u\n", hash);
#endif
	}

	qxl->bo_funcs->bo_unmap(image_bo);
	return image_bo;
}

void
qxl_image_destroy (qxl_screen_t *qxl,
		   struct qxl_bo *image_bo)
{
    struct QXLImage *image;
    uint64_t chunk, prev_chunk;

    image = qxl->bo_funcs->bo_map(image_bo);
    qxl->bo_funcs->bo_unmap(image_bo);

    image = qxl->bo_funcs->bo_map(image_bo);
    chunk = image->bitmap.data;
    while (chunk)
    {
	struct qxl_bo *bo;
	struct QXLDataChunk *virtual;

	bo = qxl_ums_lookup_phy_addr(qxl, chunk);
	assert(bo);
	virtual = qxl->bo_funcs->bo_map(bo);
	chunk = virtual->next_chunk;
	prev_chunk = virtual->prev_chunk;

	qxl->bo_funcs->bo_unmap(bo);
	qxl->bo_funcs->bo_decref (qxl, bo);
	if (prev_chunk) {
	    bo = qxl_ums_lookup_phy_addr(qxl, prev_chunk);
	    assert(bo);
	    qxl->bo_funcs->bo_decref (qxl, bo);
	}
    }
    qxl->bo_funcs->bo_unmap(image_bo);
    qxl->bo_funcs->bo_decref (qxl, image_bo);
}
