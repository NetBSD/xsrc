/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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
 */

/* This file implements randomized texture blit tests. */

#include "si_pipe.h"
#include "util/rand_xor.h"
#include "util/u_surface.h"

static uint64_t seed_xorshift128plus[2];

#define RAND_NUM_SIZE 8

/* The GPU blits are emulated on the CPU using these CPU textures. */

struct cpu_texture {
   uint8_t *ptr;
   uint64_t size;
   uint64_t layer_stride;
   unsigned stride;
};

static void alloc_cpu_texture(struct cpu_texture *tex, struct pipe_resource *templ)
{
   tex->stride = align(util_format_get_stride(templ->format, templ->width0), RAND_NUM_SIZE);
   tex->layer_stride = (uint64_t)tex->stride * templ->height0;
   tex->size = tex->layer_stride * templ->array_size;
   tex->ptr = malloc(tex->size);
   assert(tex->ptr);
}

static void set_random_pixels(struct pipe_context *ctx, struct pipe_resource *tex,
                              struct cpu_texture *cpu)
{
   struct pipe_transfer *t;
   uint8_t *map;
   int x, y, z;

   map = pipe_texture_map_3d(ctx, tex, 0, PIPE_MAP_WRITE, 0, 0, 0, tex->width0, tex->height0,
                              tex->array_size, &t);
   assert(map);

   for (z = 0; z < tex->array_size; z++) {
      for (y = 0; y < tex->height0; y++) {
         uint64_t *ptr = (uint64_t *)(map + t->layer_stride * z + t->stride * y);
         uint64_t *ptr_cpu = (uint64_t *)(cpu->ptr + cpu->layer_stride * z + cpu->stride * y);
         unsigned size = cpu->stride / RAND_NUM_SIZE;

         assert(t->stride % RAND_NUM_SIZE == 0);
         assert(cpu->stride % RAND_NUM_SIZE == 0);

         for (x = 0; x < size; x++) {
            *ptr++ = *ptr_cpu++ = rand_xorshift128plus(seed_xorshift128plus);
         }
      }
   }

   pipe_texture_unmap(ctx, t);
}

static bool compare_textures(struct pipe_context *ctx, struct pipe_resource *tex,
                             struct cpu_texture *cpu)
{
   struct pipe_transfer *t;
   uint8_t *map;
   int y, z;
   bool pass = true;
   unsigned stride = util_format_get_stride(tex->format, tex->width0);

   map = pipe_texture_map_3d(ctx, tex, 0, PIPE_MAP_READ, 0, 0, 0, tex->width0, tex->height0,
                              tex->array_size, &t);
   assert(map);

   for (z = 0; z < tex->array_size; z++) {
      for (y = 0; y < tex->height0; y++) {
         uint8_t *ptr = map + t->layer_stride * z + t->stride * y;
         uint8_t *cpu_ptr = cpu->ptr + cpu->layer_stride * z + cpu->stride * y;

         if (memcmp(ptr, cpu_ptr, stride)) {
            pass = false;
            goto done;
         }
      }
   }
done:
   pipe_texture_unmap(ctx, t);
   return pass;
}

static enum pipe_format choose_format()
{
   enum pipe_format formats[] = {
      PIPE_FORMAT_R8_UINT,     PIPE_FORMAT_R16_UINT,          PIPE_FORMAT_R32_UINT,
      PIPE_FORMAT_R32G32_UINT, PIPE_FORMAT_R32G32B32A32_UINT, PIPE_FORMAT_G8R8_B8R8_UNORM,
   };
   return formats[rand() % ARRAY_SIZE(formats)];
}

static const char *array_mode_to_string(struct si_screen *sscreen, struct radeon_surf *surf)
{
   if (sscreen->info.chip_class >= GFX9) {
      switch (surf->u.gfx9.swizzle_mode) {
      case 0:
         return "  LINEAR";
      case 21:
         return " 4KB_S_X";
      case 22:
         return " 4KB_D_X";
      case 25:
         return "64KB_S_X";
      case 26:
         return "64KB_D_X";
      case 27:
         return "64KB_R_X";
      default:
         printf("Unhandled swizzle mode = %u\n", surf->u.gfx9.swizzle_mode);
         return " UNKNOWN";
      }
   } else {
      switch (surf->u.legacy.level[0].mode) {
      case RADEON_SURF_MODE_LINEAR_ALIGNED:
         return "LINEAR_ALIGNED";
      case RADEON_SURF_MODE_1D:
         return "1D_TILED_THIN1";
      case RADEON_SURF_MODE_2D:
         return "2D_TILED_THIN1";
      default:
         assert(0);
         return "       UNKNOWN";
      }
   }
}

static unsigned generate_max_tex_side(unsigned max_tex_side)
{
   switch (rand() % 4) {
   case 0:
      /* Try to hit large sizes in 1/4 of the cases. */
      return max_tex_side;
   case 1:
      /* Try to hit 1D tiling in 1/4 of the cases. */
      return 128;
   default:
      /* Try to hit common sizes in 2/4 of the cases. */
      return 2048;
   }
}

void si_test_blit(struct si_screen *sscreen)
{
   struct pipe_screen *screen = &sscreen->b;
   struct pipe_context *ctx = screen->context_create(screen, NULL, 0);
   struct si_context *sctx = (struct si_context *)ctx;
   uint64_t max_alloc_size;
   unsigned i, iterations, num_partial_copies, max_tex_side;
   unsigned num_pass = 0, num_fail = 0;

   max_tex_side = screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_2D_SIZE);

   /* Max 128 MB allowed for both textures. */
   max_alloc_size = 128 * 1024 * 1024;

   /* the seed for random test parameters */
   srand(0x9b47d95b);
   /* the seed for random pixel data */
   s_rand_xorshift128plus(seed_xorshift128plus, false);

   iterations = 1000000000; /* just kill it when you are bored */
   num_partial_copies = 30;

   /* These parameters are randomly generated per test:
    * - whether to do one whole-surface copy or N partial copies per test
    * - which tiling modes to use (LINEAR_ALIGNED, 1D, 2D)
    * - which texture dimensions to use
    * - whether to use VRAM (all tiling modes) and GTT (staging, linear
    *   only) allocations
    * - random initial pixels in src
    * - generate random subrectangle copies for partial blits
    */
   for (i = 0; i < iterations; i++) {
      struct pipe_resource tsrc = {}, tdst = {}, *src, *dst;
      struct si_texture *sdst;
      struct si_texture *ssrc;
      struct cpu_texture src_cpu, dst_cpu;
      unsigned max_width, max_height, max_depth, j, num;
      unsigned gfx_blits = 0, cs_blits = 0, max_tex_side_gen;
      unsigned max_tex_layers;
      bool pass;
      bool do_partial_copies = rand() & 1;

      /* generate a random test case */
      tsrc.target = tdst.target = PIPE_TEXTURE_2D_ARRAY;
      tsrc.depth0 = tdst.depth0 = 1;

      tsrc.format = tdst.format = choose_format();

      max_tex_side_gen = generate_max_tex_side(max_tex_side);
      max_tex_layers = rand() % 4 ? 1 : 5;

      tsrc.width0 = (rand() % max_tex_side_gen) + 1;
      tsrc.height0 = (rand() % max_tex_side_gen) + 1;
      tsrc.array_size = (rand() % max_tex_layers) + 1;

      if (tsrc.format == PIPE_FORMAT_G8R8_B8R8_UNORM)
         tsrc.width0 = align(tsrc.width0, 2);

      /* Have a 1/4 chance of getting power-of-two dimensions. */
      if (rand() % 4 == 0) {
         tsrc.width0 = util_next_power_of_two(tsrc.width0);
         tsrc.height0 = util_next_power_of_two(tsrc.height0);
      }

      if (!do_partial_copies) {
         /* whole-surface copies only, same dimensions */
         tdst = tsrc;
      } else {
         max_tex_side_gen = generate_max_tex_side(max_tex_side);
         max_tex_layers = rand() % 4 ? 1 : 5;

         /* many partial copies, dimensions can be different */
         tdst.width0 = (rand() % max_tex_side_gen) + 1;
         tdst.height0 = (rand() % max_tex_side_gen) + 1;
         tdst.array_size = (rand() % max_tex_layers) + 1;

         /* Have a 1/4 chance of getting power-of-two dimensions. */
         if (rand() % 4 == 0) {
            tdst.width0 = util_next_power_of_two(tdst.width0);
            tdst.height0 = util_next_power_of_two(tdst.height0);
         }
      }

      /* check texture sizes */
      if ((uint64_t)util_format_get_nblocks(tsrc.format, tsrc.width0, tsrc.height0) *
                tsrc.array_size * util_format_get_blocksize(tsrc.format) +
             (uint64_t)util_format_get_nblocks(tdst.format, tdst.width0, tdst.height0) *
                tdst.array_size * util_format_get_blocksize(tdst.format) >
          max_alloc_size) {
         /* too large, try again */
         i--;
         continue;
      }

      /* VRAM + the tiling mode depends on dimensions (3/4 of cases),
       * or GTT + linear only (1/4 of cases)
       */
      tsrc.usage = rand() % 4 ? PIPE_USAGE_DEFAULT : PIPE_USAGE_STAGING;
      tdst.usage = rand() % 4 ? PIPE_USAGE_DEFAULT : PIPE_USAGE_STAGING;

      /* Allocate textures (both the GPU and CPU copies).
       * The CPU will emulate what the GPU should be doing.
       */
      src = screen->resource_create(screen, &tsrc);
      dst = screen->resource_create(screen, &tdst);
      assert(src);
      assert(dst);
      sdst = (struct si_texture *)dst;
      ssrc = (struct si_texture *)src;
      alloc_cpu_texture(&src_cpu, &tsrc);
      alloc_cpu_texture(&dst_cpu, &tdst);

      printf("%4u: dst = (%5u x %5u x %u, %s), "
             " src = (%5u x %5u x %u, %s), format = %s, ",
             i, tdst.width0, tdst.height0, tdst.array_size,
             array_mode_to_string(sscreen, &sdst->surface), tsrc.width0, tsrc.height0,
             tsrc.array_size, array_mode_to_string(sscreen, &ssrc->surface),
             util_format_description(tsrc.format)->name);
      fflush(stdout);

      /* set src pixels */
      set_random_pixels(ctx, src, &src_cpu);

      /* clear dst pixels */
      uint32_t zero = 0;
      si_clear_buffer(sctx, dst, 0, sdst->surface.surf_size, &zero, 4, SI_OP_SYNC_BEFORE_AFTER,
                      SI_COHERENCY_SHADER, SI_AUTO_SELECT_CLEAR_METHOD);
      memset(dst_cpu.ptr, 0, dst_cpu.layer_stride * tdst.array_size);

      /* preparation */
      max_width = MIN2(tsrc.width0, tdst.width0);
      max_height = MIN2(tsrc.height0, tdst.height0);
      max_depth = MIN2(tsrc.array_size, tdst.array_size);

      num = do_partial_copies ? num_partial_copies : 1;
      for (j = 0; j < num; j++) {
         int width, height, depth;
         int srcx, srcy, srcz, dstx, dsty, dstz;
         struct pipe_box box;
         unsigned old_num_draw_calls = sctx->num_draw_calls;
         unsigned old_num_cs_calls = sctx->num_compute_calls;

         if (!do_partial_copies) {
            /* copy whole src to dst */
            width = max_width;
            height = max_height;
            depth = max_depth;

            srcx = srcy = srcz = dstx = dsty = dstz = 0;
         } else {
            /* random sub-rectangle copies from src to dst */
            depth = (rand() % max_depth) + 1;
            srcz = rand() % (tsrc.array_size - depth + 1);
            dstz = rand() % (tdst.array_size - depth + 1);

            /* special code path to hit the tiled partial copies */
            if (!ssrc->surface.is_linear && !sdst->surface.is_linear && rand() & 1) {
               if (max_width < 8 || max_height < 8)
                  continue;
               width = ((rand() % (max_width / 8)) + 1) * 8;
               height = ((rand() % (max_height / 8)) + 1) * 8;

               srcx = rand() % (tsrc.width0 - width + 1) & ~0x7;
               srcy = rand() % (tsrc.height0 - height + 1) & ~0x7;

               dstx = rand() % (tdst.width0 - width + 1) & ~0x7;
               dsty = rand() % (tdst.height0 - height + 1) & ~0x7;
            } else {
               /* just make sure that it doesn't divide by zero */
               assert(max_width > 0 && max_height > 0);

               width = (rand() % max_width) + 1;
               height = (rand() % max_height) + 1;

               srcx = rand() % (tsrc.width0 - width + 1);
               srcy = rand() % (tsrc.height0 - height + 1);

               dstx = rand() % (tdst.width0 - width + 1);
               dsty = rand() % (tdst.height0 - height + 1);
            }

            /* special code path to hit out-of-bounds reads in L2T */
            if (ssrc->surface.is_linear && !sdst->surface.is_linear && rand() % 4 == 0) {
               srcx = 0;
               srcy = 0;
               srcz = 0;
            }
         }

         /* GPU copy */
         u_box_3d(srcx, srcy, srcz, width, height, depth, &box);
         si_resource_copy_region(ctx, dst, 0, dstx, dsty, dstz, src, 0, &box);

         /* See which engine was used. */
         gfx_blits += sctx->num_draw_calls > old_num_draw_calls;
         cs_blits += sctx->num_compute_calls > old_num_cs_calls;

         /* CPU copy */
         util_copy_box(dst_cpu.ptr, tdst.format, dst_cpu.stride, dst_cpu.layer_stride, dstx, dsty,
                       dstz, width, height, depth, src_cpu.ptr, src_cpu.stride,
                       src_cpu.layer_stride, srcx, srcy, srcz);
      }

      pass = compare_textures(ctx, dst, &dst_cpu);
      if (pass)
         num_pass++;
      else
         num_fail++;

      printf("BLITs: GFX = %2u, CS = %2u, %s [%u/%u]\n", gfx_blits, cs_blits,
             pass ? "pass" : "fail", num_pass, num_pass + num_fail);

      /* cleanup */
      pipe_resource_reference(&src, NULL);
      pipe_resource_reference(&dst, NULL);
      free(src_cpu.ptr);
      free(dst_cpu.ptr);
   }

   ctx->destroy(ctx);
   exit(0);
}
