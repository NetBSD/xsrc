/*
 * Copyright (c) 2011 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include <pixman.h>

#if defined(sse2)
#pragma GCC push_options
#pragma GCC target("sse2,inline-all-stringops,fpmath=sse")
#pragma GCC optimize("Ofast")
#include <xmmintrin.h>

#if __x86_64__
#define have_sse2() 1
#else
static bool have_sse2(void)
{
	static int sse2_present = -1;

	if (sse2_present == -1)
		sse2_present = sna_cpu_detect() & SSE2;

	return sse2_present;
}
#endif

static force_inline __m128i
xmm_create_mask_32(uint32_t mask)
{
	return _mm_set_epi32(mask, mask, mask, mask);
}

static force_inline __m128i
xmm_load_128(const __m128i *src)
{
	return _mm_load_si128(src);
}

static force_inline __m128i
xmm_load_128u(const __m128i *src)
{
	return _mm_loadu_si128(src);
}

static force_inline void
xmm_save_128(__m128i *dst, __m128i data)
{
	_mm_store_si128(dst, data);
}

static force_inline void
xmm_save_128u(__m128i *dst, __m128i data)
{
	_mm_storeu_si128(dst, data);
}

static force_inline void
to_sse128xN(uint8_t *dst, const uint8_t *src, int bytes)
{
	int i;

	for (i = 0; i < bytes / 128; i++) {
		__m128i xmm0, xmm1, xmm2, xmm3;
		__m128i xmm4, xmm5, xmm6, xmm7;

		xmm0 = xmm_load_128u((const __m128i*)src + 0);
		xmm1 = xmm_load_128u((const __m128i*)src + 1);
		xmm2 = xmm_load_128u((const __m128i*)src + 2);
		xmm3 = xmm_load_128u((const __m128i*)src + 3);
		xmm4 = xmm_load_128u((const __m128i*)src + 4);
		xmm5 = xmm_load_128u((const __m128i*)src + 5);
		xmm6 = xmm_load_128u((const __m128i*)src + 6);
		xmm7 = xmm_load_128u((const __m128i*)src + 7);

		xmm_save_128((__m128i*)dst + 0, xmm0);
		xmm_save_128((__m128i*)dst + 1, xmm1);
		xmm_save_128((__m128i*)dst + 2, xmm2);
		xmm_save_128((__m128i*)dst + 3, xmm3);
		xmm_save_128((__m128i*)dst + 4, xmm4);
		xmm_save_128((__m128i*)dst + 5, xmm5);
		xmm_save_128((__m128i*)dst + 6, xmm6);
		xmm_save_128((__m128i*)dst + 7, xmm7);

		dst += 128;
		src += 128;
	}
}

static force_inline void
to_sse64(uint8_t *dst, const uint8_t *src)
{
	__m128i xmm1, xmm2, xmm3, xmm4;

	xmm1 = xmm_load_128u((const __m128i*)src + 0);
	xmm2 = xmm_load_128u((const __m128i*)src + 1);
	xmm3 = xmm_load_128u((const __m128i*)src + 2);
	xmm4 = xmm_load_128u((const __m128i*)src + 3);

	xmm_save_128((__m128i*)dst + 0, xmm1);
	xmm_save_128((__m128i*)dst + 1, xmm2);
	xmm_save_128((__m128i*)dst + 2, xmm3);
	xmm_save_128((__m128i*)dst + 3, xmm4);
}

static force_inline void
to_sse32(uint8_t *dst, const uint8_t *src)
{
	__m128i xmm1, xmm2;

	xmm1 = xmm_load_128u((const __m128i*)src + 0);
	xmm2 = xmm_load_128u((const __m128i*)src + 1);

	xmm_save_128((__m128i*)dst + 0, xmm1);
	xmm_save_128((__m128i*)dst + 1, xmm2);
}

static force_inline void
to_sse16(uint8_t *dst, const uint8_t *src)
{
	xmm_save_128((__m128i*)dst, xmm_load_128u((const __m128i*)src));
}

static void to_memcpy(uint8_t *dst, const uint8_t *src, unsigned len)
{
	assert(len);
	if ((uintptr_t)dst & 15) {
		if (len <= 16 - ((uintptr_t)dst & 15)) {
			memcpy(dst, src, len);
			return;
		}

		if ((uintptr_t)dst & 1) {
			assert(len >= 1);
			*dst++ = *src++;
			len--;
		}
		if ((uintptr_t)dst & 2) {
			assert(((uintptr_t)dst & 1) == 0);
			assert(len >= 2);
			*(uint16_t *)dst = *(const uint16_t *)src;
			dst += 2;
			src += 2;
			len -= 2;
		}
		if ((uintptr_t)dst & 4) {
			assert(((uintptr_t)dst & 3) == 0);
			assert(len >= 4);
			*(uint32_t *)dst = *(const uint32_t *)src;
			dst += 4;
			src += 4;
			len -= 4;
		}
		if ((uintptr_t)dst & 8) {
			assert(((uintptr_t)dst & 7) == 0);
			assert(len >= 8);
			*(uint64_t *)dst = *(const uint64_t *)src;
			dst += 8;
			src += 8;
			len -= 8;
		}
	}

	assert(((uintptr_t)dst & 15) == 0);
	while (len >= 64) {
		to_sse64(dst, src);
		dst += 64;
		src += 64;
		len -= 64;
	}
	if (len == 0)
		return;

	if (len & 32) {
		to_sse32(dst, src);
		dst += 32;
		src += 32;
	}
	if (len & 16) {
		to_sse16(dst, src);
		dst += 16;
		src += 16;
	}
	if (len & 8) {
		*(uint64_t *)dst = *(uint64_t *)src;
		dst += 8;
		src += 8;
	}
	if (len & 4) {
		*(uint32_t *)dst = *(uint32_t *)src;
		dst += 4;
		src += 4;
	}
	memcpy(dst, src, len & 3);
}

static void
memcpy_to_tiled_x__swizzle_0__sse2(const void *src, void *dst, int bpp,
				   int32_t src_stride, int32_t dst_stride,
				   int16_t src_x, int16_t src_y,
				   int16_t dst_x, int16_t dst_y,
				   uint16_t width, uint16_t height)
{
	const unsigned tile_width = 512;
	const unsigned tile_height = 8;
	const unsigned tile_size = 4096;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	unsigned offset_x, length_x;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	if (src_x | src_y)
		src = (const uint8_t *)src + src_y * src_stride + src_x * cpp;
	width *= cpp;
	assert(src_stride >= width);

	if (dst_x & tile_mask) {
		offset_x = (dst_x & tile_mask) * cpp;
		length_x = min(tile_width - offset_x, width);
	} else
		length_x = 0;
	dst = (uint8_t *)dst + (dst_x >> tile_shift) * tile_size;

	while (height--) {
		unsigned w = width;
		const uint8_t *src_row = src;
		uint8_t *tile_row = dst;

		src = (const uint8_t *)src + src_stride;

		tile_row += dst_y / tile_height * dst_stride * tile_height;
		tile_row += (dst_y & (tile_height-1)) * tile_width;
		dst_y++;

		if (length_x) {
			to_memcpy(tile_row + offset_x, src_row, length_x);

			tile_row += tile_size;
			src_row = (const uint8_t *)src_row + length_x;
			w -= length_x;
		}
		while (w >= tile_width) {
			assert(((uintptr_t)tile_row & (tile_width - 1)) == 0);
			to_sse128xN(assume_aligned(tile_row, tile_width),
				    src_row, tile_width);
			tile_row += tile_size;
			src_row = (const uint8_t *)src_row + tile_width;
			w -= tile_width;
		}
		if (w) {
			assert(((uintptr_t)tile_row & (tile_width - 1)) == 0);
			to_memcpy(assume_aligned(tile_row, tile_width),
				  src_row, w);
		}
	}
}

static force_inline void
from_sse128xNu(uint8_t *dst, const uint8_t *src, int bytes)
{
	int i;

	assert(((uintptr_t)src & 15) == 0);

	for (i = 0; i < bytes / 128; i++) {
		__m128i xmm0, xmm1, xmm2, xmm3;
		__m128i xmm4, xmm5, xmm6, xmm7;

		xmm0 = xmm_load_128((const __m128i*)src + 0);
		xmm1 = xmm_load_128((const __m128i*)src + 1);
		xmm2 = xmm_load_128((const __m128i*)src + 2);
		xmm3 = xmm_load_128((const __m128i*)src + 3);
		xmm4 = xmm_load_128((const __m128i*)src + 4);
		xmm5 = xmm_load_128((const __m128i*)src + 5);
		xmm6 = xmm_load_128((const __m128i*)src + 6);
		xmm7 = xmm_load_128((const __m128i*)src + 7);

		xmm_save_128u((__m128i*)dst + 0, xmm0);
		xmm_save_128u((__m128i*)dst + 1, xmm1);
		xmm_save_128u((__m128i*)dst + 2, xmm2);
		xmm_save_128u((__m128i*)dst + 3, xmm3);
		xmm_save_128u((__m128i*)dst + 4, xmm4);
		xmm_save_128u((__m128i*)dst + 5, xmm5);
		xmm_save_128u((__m128i*)dst + 6, xmm6);
		xmm_save_128u((__m128i*)dst + 7, xmm7);

		dst += 128;
		src += 128;
	}
}

static force_inline void
from_sse128xNa(uint8_t *dst, const uint8_t *src, int bytes)
{
	int i;

	assert(((uintptr_t)dst & 15) == 0);
	assert(((uintptr_t)src & 15) == 0);

	for (i = 0; i < bytes / 128; i++) {
		__m128i xmm0, xmm1, xmm2, xmm3;
		__m128i xmm4, xmm5, xmm6, xmm7;

		xmm0 = xmm_load_128((const __m128i*)src + 0);
		xmm1 = xmm_load_128((const __m128i*)src + 1);
		xmm2 = xmm_load_128((const __m128i*)src + 2);
		xmm3 = xmm_load_128((const __m128i*)src + 3);
		xmm4 = xmm_load_128((const __m128i*)src + 4);
		xmm5 = xmm_load_128((const __m128i*)src + 5);
		xmm6 = xmm_load_128((const __m128i*)src + 6);
		xmm7 = xmm_load_128((const __m128i*)src + 7);

		xmm_save_128((__m128i*)dst + 0, xmm0);
		xmm_save_128((__m128i*)dst + 1, xmm1);
		xmm_save_128((__m128i*)dst + 2, xmm2);
		xmm_save_128((__m128i*)dst + 3, xmm3);
		xmm_save_128((__m128i*)dst + 4, xmm4);
		xmm_save_128((__m128i*)dst + 5, xmm5);
		xmm_save_128((__m128i*)dst + 6, xmm6);
		xmm_save_128((__m128i*)dst + 7, xmm7);

		dst += 128;
		src += 128;
	}
}

static force_inline void
from_sse64u(uint8_t *dst, const uint8_t *src)
{
	__m128i xmm1, xmm2, xmm3, xmm4;

	assert(((uintptr_t)src & 15) == 0);

	xmm1 = xmm_load_128((const __m128i*)src + 0);
	xmm2 = xmm_load_128((const __m128i*)src + 1);
	xmm3 = xmm_load_128((const __m128i*)src + 2);
	xmm4 = xmm_load_128((const __m128i*)src + 3);

	xmm_save_128u((__m128i*)dst + 0, xmm1);
	xmm_save_128u((__m128i*)dst + 1, xmm2);
	xmm_save_128u((__m128i*)dst + 2, xmm3);
	xmm_save_128u((__m128i*)dst + 3, xmm4);
}

static force_inline void
from_sse64a(uint8_t *dst, const uint8_t *src)
{
	__m128i xmm1, xmm2, xmm3, xmm4;

	assert(((uintptr_t)dst & 15) == 0);
	assert(((uintptr_t)src & 15) == 0);

	xmm1 = xmm_load_128((const __m128i*)src + 0);
	xmm2 = xmm_load_128((const __m128i*)src + 1);
	xmm3 = xmm_load_128((const __m128i*)src + 2);
	xmm4 = xmm_load_128((const __m128i*)src + 3);

	xmm_save_128((__m128i*)dst + 0, xmm1);
	xmm_save_128((__m128i*)dst + 1, xmm2);
	xmm_save_128((__m128i*)dst + 2, xmm3);
	xmm_save_128((__m128i*)dst + 3, xmm4);
}

static force_inline void
from_sse32u(uint8_t *dst, const uint8_t *src)
{
	__m128i xmm1, xmm2;

	xmm1 = xmm_load_128((const __m128i*)src + 0);
	xmm2 = xmm_load_128((const __m128i*)src + 1);

	xmm_save_128u((__m128i*)dst + 0, xmm1);
	xmm_save_128u((__m128i*)dst + 1, xmm2);
}

static force_inline void
from_sse32a(uint8_t *dst, const uint8_t *src)
{
	__m128i xmm1, xmm2;

	assert(((uintptr_t)dst & 15) == 0);
	assert(((uintptr_t)src & 15) == 0);

	xmm1 = xmm_load_128((const __m128i*)src + 0);
	xmm2 = xmm_load_128((const __m128i*)src + 1);

	xmm_save_128((__m128i*)dst + 0, xmm1);
	xmm_save_128((__m128i*)dst + 1, xmm2);
}

static force_inline void
from_sse16u(uint8_t *dst, const uint8_t *src)
{
	assert(((uintptr_t)src & 15) == 0);

	xmm_save_128u((__m128i*)dst, xmm_load_128((const __m128i*)src));
}

static force_inline void
from_sse16a(uint8_t *dst, const uint8_t *src)
{
	assert(((uintptr_t)dst & 15) == 0);
	assert(((uintptr_t)src & 15) == 0);

	xmm_save_128((__m128i*)dst, xmm_load_128((const __m128i*)src));
}

static void
memcpy_from_tiled_x__swizzle_0__sse2(const void *src, void *dst, int bpp,
				     int32_t src_stride, int32_t dst_stride,
				     int16_t src_x, int16_t src_y,
				     int16_t dst_x, int16_t dst_y,
				     uint16_t width, uint16_t height)
{
	const unsigned tile_width = 512;
	const unsigned tile_height = 8;
	const unsigned tile_size = 4096;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	unsigned length_x, offset_x;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	if (dst_x | dst_y)
		dst = (uint8_t *)dst + dst_y * dst_stride + dst_x * cpp;
	width *= cpp;
	assert(dst_stride >= width);
	if (src_x & tile_mask) {
		offset_x = (src_x & tile_mask) * cpp;
		length_x = min(tile_width - offset_x, width);
		dst_stride -= width;
		dst_stride += (width - length_x) & 15;
	} else {
		offset_x = 0;
		dst_stride -= width & ~15;
	}
	assert(dst_stride >= 0);
	src = (const uint8_t *)src + (src_x >> tile_shift) * tile_size;

	while (height--) {
		unsigned w = width;
		const uint8_t *tile_row = src;

		tile_row += src_y / tile_height * src_stride * tile_height;
		tile_row += (src_y & (tile_height-1)) * tile_width;
		src_y++;

		if (offset_x) {
			memcpy(dst, tile_row + offset_x, length_x);
			tile_row += tile_size;
			dst = (uint8_t *)dst + length_x;
			w -= length_x;
		}

		if ((uintptr_t)dst & 15) {
			while (w >= tile_width) {
				from_sse128xNu(dst,
					       assume_aligned(tile_row, tile_width),
					       tile_width);
				tile_row += tile_size;
				dst = (uint8_t *)dst + tile_width;
				w -= tile_width;
			}
			while (w >= 64) {
				from_sse64u(dst, tile_row);
				tile_row += 64;
				dst = (uint8_t *)dst + 64;
				w -= 64;
			}
			if (w & 32) {
				from_sse32u(dst, tile_row);
				tile_row += 32;
				dst = (uint8_t *)dst + 32;
			}
			if (w & 16) {
				from_sse16u(dst, tile_row);
				tile_row += 16;
				dst = (uint8_t *)dst + 16;
			}
			memcpy(dst, assume_aligned(tile_row, 16), w & 15);
		} else {
			while (w >= tile_width) {
				from_sse128xNa(assume_aligned(dst, 16),
					       assume_aligned(tile_row, tile_width),
					       tile_width);
				tile_row += tile_size;
				dst = (uint8_t *)dst + tile_width;
				w -= tile_width;
			}
			while (w >= 64) {
				from_sse64a(dst, tile_row);
				tile_row += 64;
				dst = (uint8_t *)dst + 64;
				w -= 64;
			}
			if (w & 32) {
				from_sse32a(dst, tile_row);
				tile_row += 32;
				dst = (uint8_t *)dst + 32;
			}
			if (w & 16) {
				from_sse16a(dst, tile_row);
				tile_row += 16;
				dst = (uint8_t *)dst + 16;
			}
			memcpy(assume_aligned(dst, 16),
			       assume_aligned(tile_row, 16),
			       w & 15);
		}
		dst = (uint8_t *)dst + dst_stride;
	}
}

static void
memcpy_between_tiled_x__swizzle_0__sse2(const void *src, void *dst, int bpp,
					int32_t src_stride, int32_t dst_stride,
					int16_t src_x, int16_t src_y,
					int16_t dst_x, int16_t dst_y,
					uint16_t width, uint16_t height)
{
	const unsigned tile_width = 512;
	const unsigned tile_height = 8;
	const unsigned tile_size = 4096;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	unsigned ox, lx;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	width *= cpp;
	dst_stride *= tile_height;
	src_stride *= tile_height;

	assert((dst_x & tile_mask) == (src_x & tile_mask));
	if (dst_x & tile_mask) {
		ox = (dst_x & tile_mask) * cpp;
		lx = min(tile_width - ox, width);
		assert(lx != 0);
	} else
		lx = 0;

	if (dst_x)
		dst = (uint8_t *)dst + (dst_x >> tile_shift) * tile_size;
	if (src_x)
		src = (const uint8_t *)src + (src_x >> tile_shift) * tile_size;

	while (height--) {
		const uint8_t *src_row;
		uint8_t *dst_row;
		unsigned w = width;

		dst_row = dst;
		dst_row += dst_y / tile_height * dst_stride;
		dst_row += (dst_y & (tile_height-1)) * tile_width;
		dst_y++;

		src_row = src;
		src_row += src_y / tile_height * src_stride;
		src_row += (src_y & (tile_height-1)) * tile_width;
		src_y++;

		if (lx) {
			to_memcpy(dst_row + ox, src_row + ox, lx);
			dst_row += tile_size;
			src_row += tile_size;
			w -= lx;
		}
		while (w >= tile_width) {
			assert(((uintptr_t)dst_row & (tile_width - 1)) == 0);
			assert(((uintptr_t)src_row & (tile_width - 1)) == 0);
			to_sse128xN(assume_aligned(dst_row, tile_width),
				    assume_aligned(src_row, tile_width),
				    tile_width);
			dst_row += tile_size;
			src_row += tile_size;
			w -= tile_width;
		}
		if (w) {
			assert(((uintptr_t)dst_row & (tile_width - 1)) == 0);
			assert(((uintptr_t)src_row & (tile_width - 1)) == 0);
			to_memcpy(assume_aligned(dst_row, tile_width),
				  assume_aligned(src_row, tile_width),
				  w);
		}
	}
}

#pragma GCC push_options
#endif

fast void
memcpy_blt(const void *src, void *dst, int bpp,
	   int32_t src_stride, int32_t dst_stride,
	   int16_t src_x, int16_t src_y,
	   int16_t dst_x, int16_t dst_y,
	   uint16_t width, uint16_t height)
{
	const uint8_t *src_bytes;
	uint8_t *dst_bytes;
	int byte_width;

	assert(src);
	assert(dst);
	assert(width && height);
	assert(bpp >= 8);
	assert(width*bpp <= 8*src_stride);
	assert(width*bpp <= 8*dst_stride);

	DBG(("%s: src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));

	bpp /= 8;

	src_bytes = (const uint8_t *)src + src_stride * src_y + src_x * bpp;
	dst_bytes = (uint8_t *)dst + dst_stride * dst_y + dst_x * bpp;

	byte_width = width * bpp;
	if (byte_width == src_stride && byte_width == dst_stride) {
		byte_width *= height;
		height = 1;
	}

	switch (byte_width) {
	case 1:
		do {
			*dst_bytes = *src_bytes;
			src_bytes += src_stride;
			dst_bytes += dst_stride;
		} while (--height);
		break;

	case 2:
		do {
			*(uint16_t *)dst_bytes = *(const uint16_t *)src_bytes;
			src_bytes += src_stride;
			dst_bytes += dst_stride;
		} while (--height);
		break;

	case 4:
		do {
			*(uint32_t *)dst_bytes = *(const uint32_t *)src_bytes;
			src_bytes += src_stride;
			dst_bytes += dst_stride;
		} while (--height);
		break;

	case 8:
		do {
			*(uint64_t *)dst_bytes = *(const uint64_t *)src_bytes;
			src_bytes += src_stride;
			dst_bytes += dst_stride;
		} while (--height);
		break;
	case 16:
		do {
			((uint64_t *)dst_bytes)[0] = ((const uint64_t *)src_bytes)[0];
			((uint64_t *)dst_bytes)[1] = ((const uint64_t *)src_bytes)[1];
			src_bytes += src_stride;
			dst_bytes += dst_stride;
		} while (--height);
		break;

	default:
		do {
			memcpy(dst_bytes, src_bytes, byte_width);
			src_bytes += src_stride;
			dst_bytes += dst_stride;
		} while (--height);
		break;
	}
}

static fast_memcpy void
memcpy_to_tiled_x__swizzle_0(const void *src, void *dst, int bpp,
			     int32_t src_stride, int32_t dst_stride,
			     int16_t src_x, int16_t src_y,
			     int16_t dst_x, int16_t dst_y,
			     uint16_t width, uint16_t height)
{
	const unsigned tile_width = 512;
	const unsigned tile_height = 8;
	const unsigned tile_size = 4096;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	if (src_x | src_y)
		src = (const uint8_t *)src + src_y * src_stride + src_x * cpp;
	assert(src_stride >= width * cpp);
	src_stride -= width * cpp;

	while (height--) {
		unsigned w = width * cpp;
		uint8_t *tile_row = dst;

		tile_row += dst_y / tile_height * dst_stride * tile_height;
		tile_row += (dst_y & (tile_height-1)) * tile_width;
		if (dst_x) {
			tile_row += (dst_x >> tile_shift) * tile_size;
			if (dst_x & tile_mask) {
				const unsigned x = (dst_x & tile_mask) * cpp;
				const unsigned len = min(tile_width - x, w);
				memcpy(assume_misaligned(tile_row + x, tile_width, x),
				       src, len);

				tile_row += tile_size;
				src = (const uint8_t *)src + len;
				w -= len;
			}
		}
		while (w >= tile_width) {
			memcpy(assume_aligned(tile_row, tile_width),
			       src, tile_width);
			tile_row += tile_size;
			src = (const uint8_t *)src + tile_width;
			w -= tile_width;
		}
		memcpy(assume_aligned(tile_row, tile_width), src, w);
		src = (const uint8_t *)src + src_stride + w;
		dst_y++;
	}
}

static fast_memcpy void
memcpy_from_tiled_x__swizzle_0(const void *src, void *dst, int bpp,
			       int32_t src_stride, int32_t dst_stride,
			       int16_t src_x, int16_t src_y,
			       int16_t dst_x, int16_t dst_y,
			       uint16_t width, uint16_t height)
{
	const unsigned tile_width = 512;
	const unsigned tile_height = 8;
	const unsigned tile_size = 4096;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	if (dst_x | dst_y)
		dst = (uint8_t *)dst + dst_y * dst_stride + dst_x * cpp;
	assert(dst_stride >= width * cpp);
	dst_stride -= width * cpp;

	while (height--) {
		unsigned w = width * cpp;
		const uint8_t *tile_row = src;

		tile_row += src_y / tile_height * src_stride * tile_height;
		tile_row += (src_y & (tile_height-1)) * tile_width;
		if (src_x) {
			tile_row += (src_x >> tile_shift) * tile_size;
			if (src_x & tile_mask) {
				const unsigned x = (src_x & tile_mask) * cpp;
				const unsigned len = min(tile_width - x, w);
				memcpy(dst, assume_misaligned(tile_row + x, tile_width, x), len);

				tile_row += tile_size;
				dst = (uint8_t *)dst + len;
				w -= len;
			}
		}
		while (w >= tile_width) {
			memcpy(dst,
			       assume_aligned(tile_row, tile_width),
			       tile_width);

			tile_row += tile_size;
			dst = (uint8_t *)dst + tile_width;
			w -= tile_width;
		}
		memcpy(dst, assume_aligned(tile_row, tile_width), w);
		dst = (uint8_t *)dst + dst_stride + w;
		src_y++;
	}
}

static fast_memcpy void
memcpy_between_tiled_x__swizzle_0(const void *src, void *dst, int bpp,
				  int32_t src_stride, int32_t dst_stride,
				  int16_t src_x, int16_t src_y,
				  int16_t dst_x, int16_t dst_y,
				  uint16_t width, uint16_t height)
{
	const unsigned tile_width = 512;
	const unsigned tile_height = 8;
	const unsigned tile_size = 4096;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);
	assert((dst_x & tile_mask) == (src_x & tile_mask));

	while (height--) {
		unsigned w = width * cpp;
		uint8_t *dst_row = dst;
		const uint8_t *src_row = src;

		dst_row += dst_y / tile_height * dst_stride * tile_height;
		dst_row += (dst_y & (tile_height-1)) * tile_width;
		if (dst_x)
			dst_row += (dst_x >> tile_shift) * tile_size;
		dst_y++;

		src_row += src_y / tile_height * src_stride * tile_height;
		src_row += (src_y & (tile_height-1)) * tile_width;
		if (src_x)
			src_row += (src_x >> tile_shift) * tile_size;
		src_y++;

		if (dst_x & tile_mask) {
			const unsigned x = (dst_x & tile_mask) * cpp;
			const unsigned len = min(tile_width - x, w);

			memcpy(assume_misaligned(dst_row + x, tile_width, x),
			       assume_misaligned(src_row + x, tile_width, x),
			       len);

			dst_row += tile_size;
			src_row += tile_size;
			w -= len;
		}

		while (w >= tile_width) {
			memcpy(assume_aligned(dst_row, tile_width),
			       assume_aligned(src_row, tile_width),
			       tile_width);
			dst_row += tile_size;
			src_row += tile_size;
			w -= tile_width;
		}
		memcpy(assume_aligned(dst_row, tile_width),
		       assume_aligned(src_row, tile_width),
		       w);
	}
}

#define memcpy_to_tiled_x(swizzle) \
fast_memcpy static void \
memcpy_to_tiled_x__##swizzle (const void *src, void *dst, int bpp, \
			      int32_t src_stride, int32_t dst_stride, \
			      int16_t src_x, int16_t src_y, \
			      int16_t dst_x, int16_t dst_y, \
			      uint16_t width, uint16_t height) \
{ \
	const unsigned tile_width = 512; \
	const unsigned tile_height = 8; \
	const unsigned tile_size = 4096; \
	const unsigned cpp = bpp / 8; \
	const unsigned stride_tiles = dst_stride / tile_width; \
	const unsigned swizzle_pixels = 64 / cpp; \
	const unsigned tile_pixels = ffs(tile_width / cpp) - 1; \
	const unsigned tile_mask = (1 << tile_pixels) - 1; \
	unsigned x, y; \
	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n", \
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride)); \
	src = (const uint8_t *)src + src_y * src_stride + src_x * cpp; \
	for (y = 0; y < height; ++y) { \
		const uint32_t dy = y + dst_y; \
		const uint32_t tile_row = \
			(dy / tile_height * stride_tiles * tile_size + \
			 (dy & (tile_height-1)) * tile_width); \
		const uint8_t *src_row = (const uint8_t *)src + src_stride * y; \
		uint32_t dx = dst_x; \
		x = width * cpp; \
		if (dx & (swizzle_pixels - 1)) { \
			const uint32_t swizzle_bound_pixels = ALIGN(dx + 1, swizzle_pixels); \
			const uint32_t length = min(dst_x + width, swizzle_bound_pixels) - dx; \
			uint32_t offset = \
				tile_row + \
				(dx >> tile_pixels) * tile_size + \
				(dx & tile_mask) * cpp; \
			memcpy((char *)dst + swizzle(offset), src_row, length * cpp); \
			src_row += length * cpp; \
			x -= length * cpp; \
			dx += length; \
		} \
		while (x >= 64) { \
			uint32_t offset = \
				tile_row + \
				(dx >> tile_pixels) * tile_size + \
				(dx & tile_mask) * cpp; \
			memcpy(assume_aligned((char *)dst+swizzle(offset),64), \
			       src_row, 64); \
			src_row += 64; \
			x -= 64; \
			dx += swizzle_pixels; \
		} \
		if (x) { \
			uint32_t offset = \
				tile_row + \
				(dx >> tile_pixels) * tile_size + \
				(dx & tile_mask) * cpp; \
			memcpy(assume_aligned((char *)dst + swizzle(offset), 64), src_row, x); \
		} \
	} \
}

#define memcpy_from_tiled_x(swizzle) \
fast_memcpy static void \
memcpy_from_tiled_x__##swizzle (const void *src, void *dst, int bpp, \
				int32_t src_stride, int32_t dst_stride, \
				int16_t src_x, int16_t src_y, \
				int16_t dst_x, int16_t dst_y, \
				uint16_t width, uint16_t height) \
{ \
	const unsigned tile_width = 512; \
	const unsigned tile_height = 8; \
	const unsigned tile_size = 4096; \
	const unsigned cpp = bpp / 8; \
	const unsigned stride_tiles = src_stride / tile_width; \
	const unsigned swizzle_pixels = 64 / cpp; \
	const unsigned tile_pixels = ffs(tile_width / cpp) - 1; \
	const unsigned tile_mask = (1 << tile_pixels) - 1; \
	unsigned x, y; \
	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n", \
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride)); \
	dst = (uint8_t *)dst + dst_y * dst_stride + dst_x * cpp; \
	for (y = 0; y < height; ++y) { \
		const uint32_t sy = y + src_y; \
		const uint32_t tile_row = \
			(sy / tile_height * stride_tiles * tile_size + \
			 (sy & (tile_height-1)) * tile_width); \
		uint8_t *dst_row = (uint8_t *)dst + dst_stride * y; \
		uint32_t sx = src_x; \
		x = width * cpp; \
		if (sx & (swizzle_pixels - 1)) { \
			const uint32_t swizzle_bound_pixels = ALIGN(sx + 1, swizzle_pixels); \
			const uint32_t length = min(src_x + width, swizzle_bound_pixels) - sx; \
			uint32_t offset = \
				tile_row + \
				(sx >> tile_pixels) * tile_size + \
				(sx & tile_mask) * cpp; \
			memcpy(dst_row, (const char *)src + swizzle(offset), length * cpp); \
			dst_row += length * cpp; \
			x -= length * cpp; \
			sx += length; \
		} \
		while (x >= 64) { \
			uint32_t offset = \
				tile_row + \
				(sx >> tile_pixels) * tile_size + \
				(sx & tile_mask) * cpp; \
			memcpy(dst_row, assume_aligned((const char *)src + swizzle(offset), 64), 64); \
			dst_row += 64; \
			x -= 64; \
			sx += swizzle_pixels; \
		} \
		if (x) { \
			uint32_t offset = \
				tile_row + \
				(sx >> tile_pixels) * tile_size + \
				(sx & tile_mask) * cpp; \
			memcpy(dst_row, assume_aligned((const char *)src + swizzle(offset), 64), x); \
		} \
	} \
}

#define swizzle_9(X) ((X) ^ (((X) >> 3) & 64))
memcpy_to_tiled_x(swizzle_9)
memcpy_from_tiled_x(swizzle_9)
#undef swizzle_9

#define swizzle_9_10(X) ((X) ^ ((((X) ^ ((X) >> 1)) >> 3) & 64))
memcpy_to_tiled_x(swizzle_9_10)
memcpy_from_tiled_x(swizzle_9_10)
#undef swizzle_9_10

#define swizzle_9_11(X) ((X) ^ ((((X) ^ ((X) >> 2)) >> 3) & 64))
memcpy_to_tiled_x(swizzle_9_11)
memcpy_from_tiled_x(swizzle_9_11)
#undef swizzle_9_11

#define swizzle_9_10_11(X) ((X) ^ ((((X) ^ ((X) >> 1) ^ ((X) >> 2)) >> 3) & 64))
memcpy_to_tiled_x(swizzle_9_10_11)
memcpy_from_tiled_x(swizzle_9_10_11)
#undef swizzle_9_10_11

static fast_memcpy void
memcpy_to_tiled_x__gen2(const void *src, void *dst, int bpp,
			int32_t src_stride, int32_t dst_stride,
			int16_t src_x, int16_t src_y,
			int16_t dst_x, int16_t dst_y,
			uint16_t width, uint16_t height)
{
	const unsigned tile_width = 128;
	const unsigned tile_height = 16;
	const unsigned tile_size = 2048;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	if (src_x | src_y)
		src = (const uint8_t *)src + src_y * src_stride + src_x * cpp;
	assert(src_stride >= width * cpp);
	src_stride -= width * cpp;

	while (height--) {
		unsigned w = width * cpp;
		uint8_t *tile_row = dst;

		tile_row += dst_y / tile_height * dst_stride * tile_height;
		tile_row += (dst_y & (tile_height-1)) * tile_width;
		if (dst_x) {
			tile_row += (dst_x >> tile_shift) * tile_size;
			if (dst_x & tile_mask) {
				const unsigned x = (dst_x & tile_mask) * cpp;
				const unsigned len = min(tile_width - x, w);
				memcpy(assume_misaligned(tile_row + x, tile_width, x), src, len);

				tile_row += tile_size;
				src = (const uint8_t *)src + len;
				w -= len;
			}
		}
		while (w >= tile_width) {
			memcpy(assume_aligned(tile_row, tile_width),
			       src, tile_width);

			tile_row += tile_size;
			src = (const uint8_t *)src + tile_width;
			w -= tile_width;
		}
		memcpy(assume_aligned(tile_row, tile_width), src, w);
		src = (const uint8_t *)src + src_stride + w;
		dst_y++;
	}
}

static fast_memcpy void
memcpy_from_tiled_x__gen2(const void *src, void *dst, int bpp,
			  int32_t src_stride, int32_t dst_stride,
			  int16_t src_x, int16_t src_y,
			  int16_t dst_x, int16_t dst_y,
			  uint16_t width, uint16_t height)
{
	const unsigned tile_width = 128;
	const unsigned tile_height = 16;
	const unsigned tile_size = 2048;

	const unsigned cpp = bpp / 8;
	const unsigned tile_pixels = tile_width / cpp;
	const unsigned tile_shift = ffs(tile_pixels) - 1;
	const unsigned tile_mask = tile_pixels - 1;

	DBG(("%s(bpp=%d): src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d\n",
	     __FUNCTION__, bpp, src_x, src_y, dst_x, dst_y, width, height, src_stride, dst_stride));
	assert(src != dst);

	if (dst_x | dst_y)
		dst = (uint8_t *)dst + dst_y * dst_stride + dst_x * cpp;
	assert(dst_stride >= width * cpp);
	dst_stride -= width * cpp;

	while (height--) {
		unsigned w = width * cpp;
		const uint8_t *tile_row = src;

		tile_row += src_y / tile_height * src_stride * tile_height;
		tile_row += (src_y & (tile_height-1)) * tile_width;
		if (src_x) {
			tile_row += (src_x >> tile_shift) * tile_size;
			if (src_x & tile_mask) {
				const unsigned x = (src_x & tile_mask) * cpp;
				const unsigned len = min(tile_width - x, w);
				memcpy(dst, assume_misaligned(tile_row + x, tile_width, x), len);

				tile_row += tile_size;
				dst = (uint8_t *)dst + len;
				w -= len;
			}
		}
		while (w >= tile_width) {
			memcpy(dst,
			       assume_aligned(tile_row, tile_width),
			       tile_width);

			tile_row += tile_size;
			dst = (uint8_t *)dst + tile_width;
			w -= tile_width;
		}
		memcpy(dst, assume_aligned(tile_row, tile_width), w);
		dst = (uint8_t *)dst + dst_stride + w;
		src_y++;
	}
}

void choose_memcpy_tiled_x(struct kgem *kgem, int swizzling, unsigned cpu)
{
	if (kgem->gen < 030) {
		if (swizzling == I915_BIT_6_SWIZZLE_NONE) {
			DBG(("%s: gen2, no swizzling\n", __FUNCTION__));
			kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__gen2;
			kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__gen2;
		} else
			DBG(("%s: no detiling with swizzle functions for gen2\n", __FUNCTION__));
		return;
	}

	switch (swizzling) {
	default:
		DBG(("%s: unknown swizzling, %d\n", __FUNCTION__, swizzling));
		break;
	case I915_BIT_6_SWIZZLE_NONE:
		DBG(("%s: no swizzling\n", __FUNCTION__));
#if defined(sse2)
		if (cpu & SSE2) {
			kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__swizzle_0__sse2;
			kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__swizzle_0__sse2;
			kgem->memcpy_between_tiled_x = memcpy_between_tiled_x__swizzle_0__sse2;
		} else
#endif
	       	{
			kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__swizzle_0;
			kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__swizzle_0;
			kgem->memcpy_between_tiled_x = memcpy_between_tiled_x__swizzle_0;
		}
		break;
	case I915_BIT_6_SWIZZLE_9:
		DBG(("%s: 6^9 swizzling\n", __FUNCTION__));
		kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__swizzle_9;
		kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__swizzle_9;
		break;
	case I915_BIT_6_SWIZZLE_9_10:
		DBG(("%s: 6^9^10 swizzling\n", __FUNCTION__));
		kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__swizzle_9_10;
		kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__swizzle_9_10;
		break;
	case I915_BIT_6_SWIZZLE_9_11:
		DBG(("%s: 6^9^11 swizzling\n", __FUNCTION__));
		kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__swizzle_9_11;
		kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__swizzle_9_11;
		break;
	case I915_BIT_6_SWIZZLE_9_10_11:
		DBG(("%s: 6^9^10^11 swizzling\n", __FUNCTION__));
		kgem->memcpy_to_tiled_x = memcpy_to_tiled_x__swizzle_9_10_11;
		kgem->memcpy_from_tiled_x = memcpy_from_tiled_x__swizzle_9_10_11;
		break;
	}
}

void
memmove_box(const void *src, void *dst,
	    int bpp, int32_t stride,
	    const BoxRec *box,
	    int dx, int dy)
{
#define FORCE_MEMMOVE 0
	union {
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	} tmp;
	const uint8_t *src_bytes;
	uint8_t *dst_bytes;
	int width, height;

	assert(src);
	assert(dst);
	assert(src != dst);
	assert(bpp >= 8);
	assert(box->x2 > box->x1);
	assert(box->y2 > box->y1);

	DBG(("%s: box=(%d, %d), (%d, %d), pitch=%d, bpp=%d, dx=%d, dy=%d\n",
	     __FUNCTION__,
	     box->x1, box->y1, box->x2, box->y2,
	     stride, bpp, dx, dy));

	bpp /= 8;
	width = box->y1 * stride + box->x1 * bpp;
	src_bytes = (const uint8_t *)src + width;
	dst_bytes = (uint8_t *)dst + width;
	assert(dst_bytes != src_bytes);

	width = (box->x2 - box->x1) * bpp;
	height = (box->y2 - box->y1);
	assert(width <= stride);
	if (width == stride) {
		width *= height;
		height = 1;
	}

	if (dy >= 0) {
		switch (width) {
		case 1:
			do {
				*dst_bytes = tmp.u8 = *src_bytes;
				src_bytes += stride;
				dst_bytes += stride;
			} while (--height);
			break;

		case 2:
			do {
				*(uint16_t *)dst_bytes = tmp.u16 = *(const uint16_t *)src_bytes;
				src_bytes += stride;
				dst_bytes += stride;
			} while (--height);
			break;

		case 4:
			do {
				*(uint32_t *)dst_bytes = tmp.u32 = *(const uint32_t *)src_bytes;
				src_bytes += stride;
				dst_bytes += stride;
			} while (--height);
			break;

		case 8:
			do {
				*(uint64_t *)dst_bytes = tmp.u64 = *(const uint64_t *)src_bytes;
				src_bytes += stride;
				dst_bytes += stride;
			} while (--height);
			break;

		default:
			if (FORCE_MEMMOVE ||
			    (dst_bytes < src_bytes + width &&
			     src_bytes < dst_bytes + width)) {
				do {
					memmove(dst_bytes, src_bytes, width);
					src_bytes += stride;
					dst_bytes += stride;
				} while (--height);
			} else {
				do {
					memcpy(dst_bytes, src_bytes, width);
					src_bytes += stride;
					dst_bytes += stride;
				} while (--height);
			}
			break;
		}
	} else {
		src_bytes += (height-1) * stride;
		dst_bytes += (height-1) * stride;

		switch (width) {
		case 1:
			do {
				*dst_bytes = tmp.u8 = *src_bytes;
				src_bytes -= stride;
				dst_bytes -= stride;
			} while (--height);
			break;

		case 2:
			do {
				*(uint16_t *)dst_bytes = tmp.u16 = *(const uint16_t *)src_bytes;
				src_bytes -= stride;
				dst_bytes -= stride;
			} while (--height);
			break;

		case 4:
			do {
				*(uint32_t *)dst_bytes = tmp.u32 = *(const uint32_t *)src_bytes;
				src_bytes -= stride;
				dst_bytes -= stride;
			} while (--height);
			break;

		case 8:
			do {
				*(uint64_t *)dst_bytes = tmp.u64 = *(const uint64_t *)src_bytes;
				src_bytes -= stride;
				dst_bytes -= stride;
			} while (--height);
			break;

		default:
			if (FORCE_MEMMOVE ||
			    (dst_bytes < src_bytes + width &&
			     src_bytes < dst_bytes + width)) {
				do {
					memmove(dst_bytes, src_bytes, width);
					src_bytes -= stride;
					dst_bytes -= stride;
				} while (--height);
			} else {
				do {
					memcpy(dst_bytes, src_bytes, width);
					src_bytes -= stride;
					dst_bytes -= stride;
				} while (--height);
			}
			break;
		}
	}
}

void
memcpy_xor(const void *src, void *dst, int bpp,
	   int32_t src_stride, int32_t dst_stride,
	   int16_t src_x, int16_t src_y,
	   int16_t dst_x, int16_t dst_y,
	   uint16_t width, uint16_t height,
	   uint32_t and, uint32_t or)
{
	const uint8_t *src_bytes;
	uint8_t *dst_bytes;
	int i, w;

	assert(width && height);
	assert(bpp >= 8);
	assert(width*bpp <= 8*src_stride);
	assert(width*bpp <= 8*dst_stride);

	DBG(("%s: src=(%d, %d), dst=(%d, %d), size=%dx%d, pitch=%d/%d, bpp=%d, and=%x, xor=%x\n",
	     __FUNCTION__,
	     src_x, src_y, dst_x, dst_y,
	     width, height,
	     src_stride, dst_stride,
	     bpp, and, or));

	bpp /= 8;
	src_bytes = (const uint8_t *)src + src_stride * src_y + src_x * bpp;
	dst_bytes = (uint8_t *)dst + dst_stride * dst_y + dst_x * bpp;

	if (and == 0xffffffff) {
		switch (bpp) {
		case 1:
			if (width & 1) {
				do {
					for (i = 0; i < width; i++)
						dst_bytes[i] = src_bytes[i] | or;

					src_bytes += src_stride;
					dst_bytes += dst_stride;
				} while (--height);
				break;
			} else {
				width /= 2;
				or |= or << 8;
			}
		case 2:
			if (width & 1) {
				do {
					uint16_t *d = (uint16_t *)dst_bytes;
					const uint16_t *s = (const uint16_t *)src_bytes;

					for (i = 0; i < width; i++)
						d[i] = s[i] | or;

					src_bytes += src_stride;
					dst_bytes += dst_stride;
				} while (--height);
				break;
			} else {
				width /= 2;
				or |= or << 16;
			}
		case 4:
			w = width;
			if (w * 4 == dst_stride && dst_stride == src_stride) {
				w *= height;
				height = 1;
			}

#if defined(sse2) && __x86_64__
			if (have_sse2()) {
				do {
					uint32_t *d = (uint32_t *)dst_bytes;
					const uint32_t *s = (const uint32_t *)src_bytes;
					__m128i mask = xmm_create_mask_32(or);

					i = w;
					while (i && (uintptr_t)d & 15) {
						*d++ = *s++ | or;
						i--;
					}

					while (i >= 16) {
						__m128i xmm1, xmm2, xmm3, xmm4;

						xmm1 = xmm_load_128u((const __m128i*)s + 0);
						xmm2 = xmm_load_128u((const __m128i*)s + 1);
						xmm3 = xmm_load_128u((const __m128i*)s + 2);
						xmm4 = xmm_load_128u((const __m128i*)s + 3);

						xmm_save_128((__m128i*)d + 0,
							     _mm_or_si128(xmm1, mask));
						xmm_save_128((__m128i*)d + 1,
							     _mm_or_si128(xmm2, mask));
						xmm_save_128((__m128i*)d + 2,
							     _mm_or_si128(xmm3, mask));
						xmm_save_128((__m128i*)d + 3,
							     _mm_or_si128(xmm4, mask));

						d += 16;
						s += 16;
						i -= 16;
					}

					if (i & 8) {
						__m128i xmm1, xmm2;

						xmm1 = xmm_load_128u((const __m128i*)s + 0);
						xmm2 = xmm_load_128u((const __m128i*)s + 1);

						xmm_save_128((__m128i*)d + 0,
							     _mm_or_si128(xmm1, mask));
						xmm_save_128((__m128i*)d + 1,
							     _mm_or_si128(xmm2, mask));
						d += 8;
						s += 8;
						i -= 8;
					}

					if (i & 4) {
						xmm_save_128((__m128i*)d,
							     _mm_or_si128(xmm_load_128u((const __m128i*)s),
									  mask));

						d += 4;
						s += 4;
						i -= 4;
					}

					while (i) {
						*d++ = *s++ | or;
						i--;
					}

					src_bytes += src_stride;
					dst_bytes += dst_stride;
				} while (--height);
			} else
#else
				do {
					uint32_t *d = (uint32_t *)dst_bytes;
					uint32_t *s = (uint32_t *)src_bytes;

					for (i = 0; i < w; i++)
						d[i] = s[i] | or;

					src_bytes += src_stride;
					dst_bytes += dst_stride;
				} while (--height);
#endif
			break;
		}
	} else {
		switch (bpp) {
		case 1:
			do {
				for (i = 0; i < width; i++)
					dst_bytes[i] = (src_bytes[i] & and) | or;

				src_bytes += src_stride;
				dst_bytes += dst_stride;
			} while (--height);
			break;

		case 2:
			do {
				uint16_t *d = (uint16_t *)dst_bytes;
				const uint16_t *s = (const uint16_t *)src_bytes;

				for (i = 0; i < width; i++)
					d[i] = (s[i] & and) | or;

				src_bytes += src_stride;
				dst_bytes += dst_stride;
			} while (--height);
			break;

		case 4:
			do {
				uint32_t *d = (uint32_t *)dst_bytes;
				const uint32_t *s = (const uint32_t *)src_bytes;

				for (i = 0; i < width; i++)
					d[i] = (s[i] & and) | or;

				src_bytes += src_stride;
				dst_bytes += dst_stride;
			} while (--height);
			break;
		}
	}
}

#define BILINEAR_INTERPOLATION_BITS 4
static inline int
bilinear_weight(pixman_fixed_t x)
{
	return (x >> (16 - BILINEAR_INTERPOLATION_BITS)) &
		((1 << BILINEAR_INTERPOLATION_BITS) - 1);
}

#if BILINEAR_INTERPOLATION_BITS <= 4
/* Inspired by Filter_32_opaque from Skia */
static inline uint32_t
bilinear_interpolation(uint32_t tl, uint32_t tr,
		       uint32_t bl, uint32_t br,
		       int distx, int disty)
{
	int distxy, distxiy, distixy, distixiy;
	uint32_t lo, hi;

	distx <<= (4 - BILINEAR_INTERPOLATION_BITS);
	disty <<= (4 - BILINEAR_INTERPOLATION_BITS);

	distxy = distx * disty;
	distxiy = (distx << 4) - distxy;	/* distx * (16 - disty) */
	distixy = (disty << 4) - distxy;	/* disty * (16 - distx) */
	distixiy =
		16 * 16 - (disty << 4) -
		(distx << 4) + distxy; /* (16 - distx) * (16 - disty) */

	lo = (tl & 0xff00ff) * distixiy;
	hi = ((tl >> 8) & 0xff00ff) * distixiy;

	lo += (tr & 0xff00ff) * distxiy;
	hi += ((tr >> 8) & 0xff00ff) * distxiy;

	lo += (bl & 0xff00ff) * distixy;
	hi += ((bl >> 8) & 0xff00ff) * distixy;

	lo += (br & 0xff00ff) * distxy;
	hi += ((br >> 8) & 0xff00ff) * distxy;

	return ((lo >> 8) & 0xff00ff) | (hi & ~0xff00ff);
}
#elif SIZEOF_LONG > 4
static inline uint32_t
bilinear_interpolation(uint32_t tl, uint32_t tr,
		       uint32_t bl, uint32_t br,
		       int distx, int disty)
{
	uint64_t distxy, distxiy, distixy, distixiy;
	uint64_t tl64, tr64, bl64, br64;
	uint64_t f, r;

	distx <<= (8 - BILINEAR_INTERPOLATION_BITS);
	disty <<= (8 - BILINEAR_INTERPOLATION_BITS);

	distxy = distx * disty;
	distxiy = distx * (256 - disty);
	distixy = (256 - distx) * disty;
	distixiy = (256 - distx) * (256 - disty);

	/* Alpha and Blue */
	tl64 = tl & 0xff0000ff;
	tr64 = tr & 0xff0000ff;
	bl64 = bl & 0xff0000ff;
	br64 = br & 0xff0000ff;

	f = tl64 * distixiy + tr64 * distxiy + bl64 * distixy + br64 * distxy;
	r = f & 0x0000ff0000ff0000ull;

	/* Red and Green */
	tl64 = tl;
	tl64 = ((tl64 << 16) & 0x000000ff00000000ull) | (tl64 & 0x0000ff00ull);

	tr64 = tr;
	tr64 = ((tr64 << 16) & 0x000000ff00000000ull) | (tr64 & 0x0000ff00ull);

	bl64 = bl;
	bl64 = ((bl64 << 16) & 0x000000ff00000000ull) | (bl64 & 0x0000ff00ull);

	br64 = br;
	br64 = ((br64 << 16) & 0x000000ff00000000ull) | (br64 & 0x0000ff00ull);

	f = tl64 * distixiy + tr64 * distxiy + bl64 * distixy + br64 * distxy;
	r |= ((f >> 16) & 0x000000ff00000000ull) | (f & 0xff000000ull);

	return (uint32_t)(r >> 16);
}
#else
static inline uint32_t
bilinear_interpolation(uint32_t tl, uint32_t tr,
		       uint32_t bl, uint32_t br,
		       int distx, int disty)
{
	int distxy, distxiy, distixy, distixiy;
	uint32_t f, r;

	distx <<= (8 - BILINEAR_INTERPOLATION_BITS);
	disty <<= (8 - BILINEAR_INTERPOLATION_BITS);

	distxy = distx * disty;
	distxiy = (distx << 8) - distxy;	/* distx * (256 - disty) */
	distixy = (disty << 8) - distxy;	/* disty * (256 - distx) */
	distixiy =
		256 * 256 - (disty << 8) -
		(distx << 8) + distxy;		/* (256 - distx) * (256 - disty) */

	/* Blue */
	r = ((tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy +
	     (bl & 0x000000ff) * distixy  + (br & 0x000000ff) * distxy);

	/* Green */
	f = ((tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy +
	     (bl & 0x0000ff00) * distixy  + (br & 0x0000ff00) * distxy);
	r |= f & 0xff000000;

	tl >>= 16;
	tr >>= 16;
	bl >>= 16;
	br >>= 16;
	r >>= 16;

	/* Red */
	f = ((tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy +
	     (bl & 0x000000ff) * distixy  + (br & 0x000000ff) * distxy);
	r |= f & 0x00ff0000;

	/* Alpha */
	f = ((tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy +
	     (bl & 0x0000ff00) * distixy  + (br & 0x0000ff00) * distxy);
	r |= f & 0xff000000;

	return r;
}
#endif

static inline uint32_t convert_pixel(const uint8_t *p, int x)
{
	return ((uint32_t *)p)[x];
}

fast void
affine_blt(const void *src, void *dst, int bpp,
	   int16_t src_x, int16_t src_y,
	   int16_t src_width, int16_t src_height,
	   int32_t src_stride,
	   int16_t dst_x, int16_t dst_y,
	   uint16_t dst_width, uint16_t dst_height,
	   int32_t dst_stride,
	   const struct pixman_f_transform *t)
{
	static const uint8_t zero[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	const pixman_fixed_t ux = pixman_double_to_fixed(t->m[0][0]);
	const pixman_fixed_t uy = pixman_double_to_fixed(t->m[1][0]);
	int i, j;

	assert(bpp == 32);

	for (j = 0; j < dst_height; j++) {
		pixman_fixed_t x, y;
		struct pixman_f_vector v;
		uint32_t *b;

		/* reference point is the center of the pixel */
		v.v[0] = dst_x + 0.5;
		v.v[1] = dst_y + j + 0.5;
		v.v[2] = 1.0;

		pixman_f_transform_point_3d(t, &v);

		x = pixman_double_to_fixed(v.v[0]);
		x += pixman_int_to_fixed(src_x - dst_x);
		y = pixman_double_to_fixed(v.v[1]);
		y +=  pixman_int_to_fixed(src_y - dst_y);

		b = (uint32_t*)((uint8_t *)dst + (dst_y + j) * dst_stride + dst_x * bpp / 8);
		for (i = 0; i < dst_width; i++) {
			const uint8_t *row1;
			const uint8_t *row2;
			int x1, y1, x2, y2;
			uint32_t tl, tr, bl, br;
			int32_t fx, fy;

			x1 = x - pixman_fixed_1/2;
			y1 = y - pixman_fixed_1/2;

			fx = bilinear_weight(x1);
			fy = bilinear_weight(y1);

			x1 = pixman_fixed_to_int(x1);
			x2 = x1 + 1;
			y1 = pixman_fixed_to_int(y1);
			y2 = y1 + 1;

			if (x1 >= src_width  || x2 < 0 ||
			    y1 >= src_height || y2 < 0) {
				b[i] = 0;
				goto next;
			}

			if (y2 == 0) {
				row1 = zero;
			} else {
				row1 = (uint8_t *)src + src_stride * y1;
				row1 += bpp / 8 * x1;
			}

			if (y1 == src_height - 1) {
				row2 = zero;
			} else {
				row2 = (uint8_t *)src + src_stride * y2;
				row2 += bpp / 8 * x1;
			}

			if (x2 == 0) {
				tl = 0;
				bl = 0;
			} else {
				tl = convert_pixel(row1, 0);
				bl = convert_pixel(row2, 0);
			}

			if (x1 == src_width - 1) {
				tr = 0;
				br = 0;
			} else {
				tr = convert_pixel(row1, 1);
				br = convert_pixel(row2, 1);
			}

			b[i] = bilinear_interpolation(tl, tr, bl, br, fx, fy);

next:
			x += ux;
			y += uy;
		}
	}
}
