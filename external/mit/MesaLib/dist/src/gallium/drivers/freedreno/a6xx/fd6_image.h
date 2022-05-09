/*
 * Copyright (C) 2017 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD6_IMAGE_H_
#define FD6_IMAGE_H_

#include "freedreno_context.h"

void fd6_emit_image_tex(struct fd_ringbuffer *ring,
                        const struct pipe_image_view *pimg) assert_dt;
void fd6_emit_ssbo_tex(struct fd_ringbuffer *ring,
                       const struct pipe_shader_buffer *pbuf) assert_dt;

struct ir3_shader_variant;
struct fd_ringbuffer *
fd6_build_ibo_state(struct fd_context *ctx, const struct ir3_shader_variant *v,
                    enum pipe_shader_type shader) assert_dt;

void fd6_image_init(struct pipe_context *pctx);

#endif /* FD6_IMAGE_H_ */
