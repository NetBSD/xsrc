/*
 * Copyright © 2018 Collabora Ltd
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

#ifndef ST_TGSI_LOWER_DEPTH_CLAMP_H
#define ST_TGSI_LOWER_DEPTH_CLAMP_H

#include <stdbool.h>
struct tgsi_token;

const struct tgsi_token *
st_tgsi_lower_depth_clamp(const struct tgsi_token *tokens,
                          int depth_range_const,
                          bool clip_negative_one_to_one);

const struct tgsi_token *
st_tgsi_lower_depth_clamp_fs(const struct tgsi_token *tokens,
                             int depth_range_const);

#endif /* ST_TGSI_LOWER_DEPTH_CLAMP_H */
