/*
 * Copyright (c) 2014 Intel Corporation
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

#ifndef __XASSERT_H__
#define __XASSERT_H__

/* Rewrap the traditional assert so that we can capture the error message
 * via Xorg.0.log
 */

#include <assert.h>

#ifndef NDEBUG
#include <os.h>
#include "compiler.h"

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,6,0,0,0)
#define xorg_backtrace()
#endif

#undef assert
#define assert(E) do if (unlikely(!(E))) { \
	xorg_backtrace(); \
	FatalError("%s:%d assertion '%s' failed\n", __func__, __LINE__, #E); \
} while (0)

#define warn_unless(E) \
({ \
	bool fail = !(E); \
	if (unlikely(fail)) { \
		static int __warn_once__; \
		if (!__warn_once__) { \
			xorg_backtrace(); \
			ErrorF("%s:%d assertion '%s' failed\n", __func__, __LINE__, #E); \
			__warn_once__ = 1; \
		} \
	} \
	unlikely(fail); \
})

#define dbg(EXPR) EXPR

#else

#define warn_unless(E) ({ bool fail = !(E); unlikely(fail); })
#define dbg(EXPR)

#endif

#endif /* __XASSERT_H__ */
