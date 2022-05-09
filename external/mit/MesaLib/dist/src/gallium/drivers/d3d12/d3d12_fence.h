/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_FENCE_H
#define D3D12_FENCE_H

#include "util/u_inlines.h"

#ifndef _WIN32
#include <wsl/winadapter.h>
#endif

#include <directx/d3d12.h>

struct pipe_screen;
struct d3d12_screen;

struct d3d12_fence {
   struct pipe_reference reference;
   ID3D12Fence *cmdqueue_fence;
   HANDLE event;
   int event_fd;
   uint64_t value;
   bool signaled;
};

static inline struct d3d12_fence *
d3d12_fence(struct pipe_fence_handle *pfence)
{
   return (struct d3d12_fence *)pfence;
}

struct d3d12_fence *
d3d12_create_fence(struct d3d12_screen *screen, struct d3d12_context *ctx);

void
d3d12_fence_reference(struct d3d12_fence **ptr, struct d3d12_fence *fence);

bool
d3d12_fence_finish(struct d3d12_fence *fence, uint64_t timeout_ns);

void
d3d12_screen_fence_init(struct pipe_screen *pscreen);

#endif
