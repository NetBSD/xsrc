/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ac_exp_param.h"
#include "ac_shader_util.h"
#include "compiler/nir/nir_serialize.h"
#include "nir/tgsi_to_nir.h"
#include "si_build_pm4.h"
#include "sid.h"
#include "util/crc32.h"
#include "util/disk_cache.h"
#include "util/hash_table.h"
#include "util/mesa-sha1.h"
#include "util/u_async_debug.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "tgsi/tgsi_from_mesa.h"

/* SHADER_CACHE */

/**
 * Return the IR key for the shader cache.
 */
void si_get_ir_cache_key(struct si_shader_selector *sel, bool ngg, bool es,
                         unsigned char ir_sha1_cache_key[20])
{
   struct blob blob = {};
   unsigned ir_size;
   void *ir_binary;

   if (sel->nir_binary) {
      ir_binary = sel->nir_binary;
      ir_size = sel->nir_size;
   } else {
      assert(sel->nir);

      blob_init(&blob);
      nir_serialize(&blob, sel->nir, true);
      ir_binary = blob.data;
      ir_size = blob.size;
   }

   /* These settings affect the compilation, but they are not derived
    * from the input shader IR.
    */
   unsigned shader_variant_flags = 0;

   if (ngg)
      shader_variant_flags |= 1 << 0;
   if (sel->nir)
      shader_variant_flags |= 1 << 1;
   if (si_get_wave_size(sel->screen, sel->info.stage, ngg, es) == 32)
      shader_variant_flags |= 1 << 2;
   if (sel->info.stage == MESA_SHADER_FRAGMENT &&
       /* Derivatives imply helper invocations so check for needs_quad_helper_invocations. */
       sel->info.base.fs.needs_quad_helper_invocations &&
       sel->info.base.fs.uses_discard &&
       sel->screen->debug_flags & DBG(FS_CORRECT_DERIVS_AFTER_KILL))
      shader_variant_flags |= 1 << 3;
   /* use_ngg_culling disables NGG passthrough for non-culling shaders to reduce context
    * rolls, which can be changed with AMD_DEBUG=nonggc or AMD_DEBUG=nggc.
    */
   if (sel->screen->use_ngg_culling)
      shader_variant_flags |= 1 << 4;

   /* bit gap */

   if (sel->screen->options.no_infinite_interp)
      shader_variant_flags |= 1 << 7;
   if (sel->screen->options.clamp_div_by_zero)
      shader_variant_flags |= 1 << 8;
   if (sel->screen->debug_flags & DBG(GISEL))
      shader_variant_flags |= 1 << 9;
   if ((sel->info.stage == MESA_SHADER_VERTEX ||
        sel->info.stage == MESA_SHADER_TESS_EVAL ||
        sel->info.stage == MESA_SHADER_GEOMETRY) &&
       !es &&
       sel->screen->options.vrs2x2)
      shader_variant_flags |= 1 << 10;
   if (sel->screen->options.inline_uniforms)
      shader_variant_flags |= 1 << 11;

   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);
   _mesa_sha1_update(&ctx, &shader_variant_flags, 4);
   _mesa_sha1_update(&ctx, ir_binary, ir_size);
   if (sel->info.stage == MESA_SHADER_VERTEX || sel->info.stage == MESA_SHADER_TESS_EVAL ||
       sel->info.stage == MESA_SHADER_GEOMETRY)
      _mesa_sha1_update(&ctx, &sel->so, sizeof(sel->so));
   _mesa_sha1_final(&ctx, ir_sha1_cache_key);

   if (ir_binary == blob.data)
      blob_finish(&blob);
}

/** Copy "data" to "ptr" and return the next dword following copied data. */
static uint32_t *write_data(uint32_t *ptr, const void *data, unsigned size)
{
   /* data may be NULL if size == 0 */
   if (size)
      memcpy(ptr, data, size);
   ptr += DIV_ROUND_UP(size, 4);
   return ptr;
}

/** Read data from "ptr". Return the next dword following the data. */
static uint32_t *read_data(uint32_t *ptr, void *data, unsigned size)
{
   memcpy(data, ptr, size);
   ptr += DIV_ROUND_UP(size, 4);
   return ptr;
}

/**
 * Write the size as uint followed by the data. Return the next dword
 * following the copied data.
 */
static uint32_t *write_chunk(uint32_t *ptr, const void *data, unsigned size)
{
   *ptr++ = size;
   return write_data(ptr, data, size);
}

/**
 * Read the size as uint followed by the data. Return both via parameters.
 * Return the next dword following the data.
 */
static uint32_t *read_chunk(uint32_t *ptr, void **data, unsigned *size)
{
   *size = *ptr++;
   assert(*data == NULL);
   if (!*size)
      return ptr;
   *data = malloc(*size);
   return read_data(ptr, *data, *size);
}

/**
 * Return the shader binary in a buffer. The first 4 bytes contain its size
 * as integer.
 */
static void *si_get_shader_binary(struct si_shader *shader)
{
   /* There is always a size of data followed by the data itself. */
   unsigned llvm_ir_size =
      shader->binary.llvm_ir_string ? strlen(shader->binary.llvm_ir_string) + 1 : 0;

   /* Refuse to allocate overly large buffers and guard against integer
    * overflow. */
   if (shader->binary.elf_size > UINT_MAX / 4 || llvm_ir_size > UINT_MAX / 4)
      return NULL;

   unsigned size = 4 + /* total size */
                   4 + /* CRC32 of the data below */
                   align(sizeof(shader->config), 4) + align(sizeof(shader->info), 4) + 4 +
                   align(shader->binary.elf_size, 4) + 4 + align(llvm_ir_size, 4);
   void *buffer = CALLOC(1, size);
   uint32_t *ptr = (uint32_t *)buffer;

   if (!buffer)
      return NULL;

   *ptr++ = size;
   ptr++; /* CRC32 is calculated at the end. */

   ptr = write_data(ptr, &shader->config, sizeof(shader->config));
   ptr = write_data(ptr, &shader->info, sizeof(shader->info));
   ptr = write_chunk(ptr, shader->binary.elf_buffer, shader->binary.elf_size);
   ptr = write_chunk(ptr, shader->binary.llvm_ir_string, llvm_ir_size);
   assert((char *)ptr - (char *)buffer == size);

   /* Compute CRC32. */
   ptr = (uint32_t *)buffer;
   ptr++;
   *ptr = util_hash_crc32(ptr + 1, size - 8);

   return buffer;
}

static bool si_load_shader_binary(struct si_shader *shader, void *binary)
{
   uint32_t *ptr = (uint32_t *)binary;
   uint32_t size = *ptr++;
   uint32_t crc32 = *ptr++;
   unsigned chunk_size;
   unsigned elf_size;

   if (util_hash_crc32(ptr, size - 8) != crc32) {
      fprintf(stderr, "radeonsi: binary shader has invalid CRC32\n");
      return false;
   }

   ptr = read_data(ptr, &shader->config, sizeof(shader->config));
   ptr = read_data(ptr, &shader->info, sizeof(shader->info));
   ptr = read_chunk(ptr, (void **)&shader->binary.elf_buffer, &elf_size);
   shader->binary.elf_size = elf_size;
   ptr = read_chunk(ptr, (void **)&shader->binary.llvm_ir_string, &chunk_size);

   return true;
}

/**
 * Insert a shader into the cache. It's assumed the shader is not in the cache.
 * Use si_shader_cache_load_shader before calling this.
 */
void si_shader_cache_insert_shader(struct si_screen *sscreen, unsigned char ir_sha1_cache_key[20],
                                   struct si_shader *shader, bool insert_into_disk_cache)
{
   void *hw_binary;
   struct hash_entry *entry;
   uint8_t key[CACHE_KEY_SIZE];
   bool memory_cache_full = sscreen->shader_cache_size >= sscreen->shader_cache_max_size;

   if (!insert_into_disk_cache && memory_cache_full)
      return;

   entry = _mesa_hash_table_search(sscreen->shader_cache, ir_sha1_cache_key);
   if (entry)
      return; /* already added */

   hw_binary = si_get_shader_binary(shader);
   if (!hw_binary)
      return;

   if (!memory_cache_full) {
      if (_mesa_hash_table_insert(sscreen->shader_cache,
                                  mem_dup(ir_sha1_cache_key, 20),
                                  hw_binary) == NULL) {
          FREE(hw_binary);
          return;
      }
      /* The size is stored at the start of the binary */
      sscreen->shader_cache_size += *(uint32_t*)hw_binary;
   }

   if (sscreen->disk_shader_cache && insert_into_disk_cache) {
      disk_cache_compute_key(sscreen->disk_shader_cache, ir_sha1_cache_key, 20, key);
      disk_cache_put(sscreen->disk_shader_cache, key, hw_binary, *((uint32_t *)hw_binary), NULL);
   }

   if (memory_cache_full)
      FREE(hw_binary);
}

bool si_shader_cache_load_shader(struct si_screen *sscreen, unsigned char ir_sha1_cache_key[20],
                                 struct si_shader *shader)
{
   struct hash_entry *entry = _mesa_hash_table_search(sscreen->shader_cache, ir_sha1_cache_key);

   if (entry) {
      if (si_load_shader_binary(shader, entry->data)) {
         p_atomic_inc(&sscreen->num_memory_shader_cache_hits);
         return true;
      }
   }
   p_atomic_inc(&sscreen->num_memory_shader_cache_misses);

   if (!sscreen->disk_shader_cache)
      return false;

   unsigned char sha1[CACHE_KEY_SIZE];
   disk_cache_compute_key(sscreen->disk_shader_cache, ir_sha1_cache_key, 20, sha1);

   size_t binary_size;
   uint8_t *buffer = disk_cache_get(sscreen->disk_shader_cache, sha1, &binary_size);
   if (buffer) {
      if (binary_size >= sizeof(uint32_t) && *((uint32_t *)buffer) == binary_size) {
         if (si_load_shader_binary(shader, buffer)) {
            free(buffer);
            si_shader_cache_insert_shader(sscreen, ir_sha1_cache_key, shader, false);
            p_atomic_inc(&sscreen->num_disk_shader_cache_hits);
            return true;
         }
      } else {
         /* Something has gone wrong discard the item from the cache and
          * rebuild/link from source.
          */
         assert(!"Invalid radeonsi shader disk cache item!");
         disk_cache_remove(sscreen->disk_shader_cache, sha1);
      }
   }

   free(buffer);
   p_atomic_inc(&sscreen->num_disk_shader_cache_misses);
   return false;
}

static uint32_t si_shader_cache_key_hash(const void *key)
{
   /* Take the first dword of SHA1. */
   return *(uint32_t *)key;
}

static bool si_shader_cache_key_equals(const void *a, const void *b)
{
   /* Compare SHA1s. */
   return memcmp(a, b, 20) == 0;
}

static void si_destroy_shader_cache_entry(struct hash_entry *entry)
{
   FREE((void *)entry->key);
   FREE(entry->data);
}

bool si_init_shader_cache(struct si_screen *sscreen)
{
   (void)simple_mtx_init(&sscreen->shader_cache_mutex, mtx_plain);
   sscreen->shader_cache =
      _mesa_hash_table_create(NULL, si_shader_cache_key_hash, si_shader_cache_key_equals);
   sscreen->shader_cache_size = 0;
   /* Maximum size: 64MB on 32 bits, 1GB else */
   sscreen->shader_cache_max_size = ((sizeof(void *) == 4) ? 64 : 1024) * 1024 * 1024;

   return sscreen->shader_cache != NULL;
}

void si_destroy_shader_cache(struct si_screen *sscreen)
{
   if (sscreen->shader_cache)
      _mesa_hash_table_destroy(sscreen->shader_cache, si_destroy_shader_cache_entry);
   simple_mtx_destroy(&sscreen->shader_cache_mutex);
}

/* SHADER STATES */

bool si_shader_mem_ordered(struct si_shader *shader)
{
   if (shader->selector->screen->info.chip_class < GFX10)
      return false;

   const struct si_shader_info *info = &shader->selector->info;
   const struct si_shader_info *prev_info =
      shader->previous_stage_sel ? &shader->previous_stage_sel->info : NULL;

   bool sampler_or_bvh = info->uses_vmem_return_type_sampler_or_bvh;
   bool other = info->uses_vmem_return_type_other ||
                info->uses_indirect_descriptor ||
                shader->config.scratch_bytes_per_wave ||
                (info->stage == MESA_SHADER_FRAGMENT &&
                 (info->base.fs.uses_fbfetch_output ||
                  shader->key.part.ps.prolog.poly_stipple));

   if (prev_info) {
      sampler_or_bvh |= prev_info->uses_vmem_return_type_sampler_or_bvh;
      other |= prev_info->uses_vmem_return_type_other ||
               prev_info->uses_indirect_descriptor;
   }

   /* Return true if both types of VMEM that return something are used. */
   return sampler_or_bvh && other;
}

static void si_set_tesseval_regs(struct si_screen *sscreen, const struct si_shader_selector *tes,
                                 struct si_shader *shader)
{
   const struct si_shader_info *info = &tes->info;
   unsigned tes_prim_mode = info->base.tess.primitive_mode;
   unsigned tes_spacing = info->base.tess.spacing;
   bool tes_vertex_order_cw = !info->base.tess.ccw;
   bool tes_point_mode = info->base.tess.point_mode;
   unsigned type, partitioning, topology, distribution_mode;

   switch (tes_prim_mode) {
   case GL_LINES:
      type = V_028B6C_TESS_ISOLINE;
      break;
   case GL_TRIANGLES:
      type = V_028B6C_TESS_TRIANGLE;
      break;
   case GL_QUADS:
      type = V_028B6C_TESS_QUAD;
      break;
   default:
      assert(0);
      return;
   }

   switch (tes_spacing) {
   case TESS_SPACING_FRACTIONAL_ODD:
      partitioning = V_028B6C_PART_FRAC_ODD;
      break;
   case TESS_SPACING_FRACTIONAL_EVEN:
      partitioning = V_028B6C_PART_FRAC_EVEN;
      break;
   case TESS_SPACING_EQUAL:
      partitioning = V_028B6C_PART_INTEGER;
      break;
   default:
      assert(0);
      return;
   }

   if (tes_point_mode)
      topology = V_028B6C_OUTPUT_POINT;
   else if (tes_prim_mode == GL_LINES)
      topology = V_028B6C_OUTPUT_LINE;
   else if (tes_vertex_order_cw)
      /* for some reason, this must be the other way around */
      topology = V_028B6C_OUTPUT_TRIANGLE_CCW;
   else
      topology = V_028B6C_OUTPUT_TRIANGLE_CW;

   if (sscreen->info.has_distributed_tess) {
      if (sscreen->info.family == CHIP_FIJI || sscreen->info.family >= CHIP_POLARIS10)
         distribution_mode = V_028B6C_TRAPEZOIDS;
      else
         distribution_mode = V_028B6C_DONUTS;
   } else
      distribution_mode = V_028B6C_NO_DIST;

   shader->vgt_tf_param = S_028B6C_TYPE(type) | S_028B6C_PARTITIONING(partitioning) |
                          S_028B6C_TOPOLOGY(topology) |
                          S_028B6C_DISTRIBUTION_MODE(distribution_mode);
}

/* Polaris needs different VTX_REUSE_DEPTH settings depending on
 * whether the "fractional odd" tessellation spacing is used.
 *
 * Possible VGT configurations and which state should set the register:
 *
 *   Reg set in | VGT shader configuration   | Value
 * ------------------------------------------------------
 *     VS as VS | VS                         | 30
 *     VS as ES | ES -> GS -> VS             | 30
 *    TES as VS | LS -> HS -> VS             | 14 or 30
 *    TES as ES | LS -> HS -> ES -> GS -> VS | 14 or 30
 */
static void polaris_set_vgt_vertex_reuse(struct si_screen *sscreen, struct si_shader_selector *sel,
                                         struct si_shader *shader)
{
   if (sscreen->info.family < CHIP_POLARIS10 || sscreen->info.chip_class >= GFX10)
      return;

   /* VS as VS, or VS as ES: */
   if ((sel->info.stage == MESA_SHADER_VERTEX &&
        (!shader->key.as_ls && !shader->is_gs_copy_shader)) ||
       /* TES as VS, or TES as ES: */
       sel->info.stage == MESA_SHADER_TESS_EVAL) {
      unsigned vtx_reuse_depth = 30;

      if (sel->info.stage == MESA_SHADER_TESS_EVAL &&
          sel->info.base.tess.spacing == TESS_SPACING_FRACTIONAL_ODD)
         vtx_reuse_depth = 14;

      shader->vgt_vertex_reuse_block_cntl = vtx_reuse_depth;
   }
}

static struct si_pm4_state *si_get_shader_pm4_state(struct si_shader *shader)
{
   si_pm4_clear_state(&shader->pm4);
   shader->pm4.is_shader = true;
   return &shader->pm4;
}

static unsigned si_get_num_vs_user_sgprs(struct si_shader *shader,
                                         unsigned num_always_on_user_sgprs)
{
   struct si_shader_selector *vs =
      shader->previous_stage_sel ? shader->previous_stage_sel : shader->selector;
   unsigned num_vbos_in_user_sgprs = vs->num_vbos_in_user_sgprs;

   /* 1 SGPR is reserved for the vertex buffer pointer. */
   assert(num_always_on_user_sgprs <= SI_SGPR_VS_VB_DESCRIPTOR_FIRST - 1);

   if (num_vbos_in_user_sgprs)
      return SI_SGPR_VS_VB_DESCRIPTOR_FIRST + num_vbos_in_user_sgprs * 4;

   /* Add the pointer to VBO descriptors. */
   return num_always_on_user_sgprs + 1;
}

/* Return VGPR_COMP_CNT for the API vertex shader. This can be hw LS, LSHS, ES, ESGS, VS. */
static unsigned si_get_vs_vgpr_comp_cnt(struct si_screen *sscreen, struct si_shader *shader,
                                        bool legacy_vs_prim_id)
{
   assert(shader->selector->info.stage == MESA_SHADER_VERTEX ||
          (shader->previous_stage_sel && shader->previous_stage_sel->info.stage == MESA_SHADER_VERTEX));

   /* GFX6-9   LS    (VertexID, RelAutoIndex,           InstanceID / StepRate0, InstanceID)
    * GFX6-9   ES,VS (VertexID, InstanceID / StepRate0, VSPrimID,               InstanceID)
    * GFX10    LS    (VertexID, RelAutoIndex,           UserVGPR1,              UserVGPR2 or InstanceID)
    * GFX10    ES,VS (VertexID, UserVGPR1,              UserVGPR2 or VSPrimID,  UserVGPR3 or InstanceID)
    */
   bool is_ls = shader->selector->info.stage == MESA_SHADER_TESS_CTRL || shader->key.as_ls;
   unsigned max = 0;

   if (shader->info.uses_instanceid) {
      if (sscreen->info.chip_class >= GFX10)
         max = MAX2(max, 3);
      else if (is_ls)
         max = MAX2(max, 2); /* use (InstanceID / StepRate0) because StepRate0 == 1 */
      else
         max = MAX2(max, 1); /* use (InstanceID / StepRate0) because StepRate0 == 1 */
   }

   if (legacy_vs_prim_id)
      max = MAX2(max, 2); /* VSPrimID */

   if (is_ls)
      max = MAX2(max, 1); /* RelAutoIndex */

   return max;
}

static void si_shader_ls(struct si_screen *sscreen, struct si_shader *shader)
{
   struct si_pm4_state *pm4;
   uint64_t va;

   assert(sscreen->info.chip_class <= GFX8);

   pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   va = shader->bo->gpu_address;
   si_pm4_set_reg(pm4, R_00B520_SPI_SHADER_PGM_LO_LS, va >> 8);

   shader->config.rsrc1 = S_00B528_VGPRS((shader->config.num_vgprs - 1) / 4) |
                          S_00B528_SGPRS((shader->config.num_sgprs - 1) / 8) |
                          S_00B528_VGPR_COMP_CNT(si_get_vs_vgpr_comp_cnt(sscreen, shader, false)) |
                          S_00B528_DX10_CLAMP(1) | S_00B528_FLOAT_MODE(shader->config.float_mode);
   shader->config.rsrc2 =
      S_00B52C_USER_SGPR(si_get_num_vs_user_sgprs(shader, SI_VS_NUM_USER_SGPR)) |
      S_00B52C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0);
}

static void si_shader_hs(struct si_screen *sscreen, struct si_shader *shader)
{
   struct si_pm4_state *pm4;
   uint64_t va;

   pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   va = shader->bo->gpu_address;

   if (sscreen->info.chip_class >= GFX9) {
      if (sscreen->info.chip_class >= GFX10) {
         si_pm4_set_reg(pm4, R_00B520_SPI_SHADER_PGM_LO_LS, va >> 8);
      } else {
         si_pm4_set_reg(pm4, R_00B410_SPI_SHADER_PGM_LO_LS, va >> 8);
      }

      unsigned num_user_sgprs = si_get_num_vs_user_sgprs(shader, GFX9_TCS_NUM_USER_SGPR);

      shader->config.rsrc2 = S_00B42C_USER_SGPR(num_user_sgprs) |
                             S_00B42C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0);

      if (sscreen->info.chip_class >= GFX10)
         shader->config.rsrc2 |= S_00B42C_USER_SGPR_MSB_GFX10(num_user_sgprs >> 5);
      else
         shader->config.rsrc2 |= S_00B42C_USER_SGPR_MSB_GFX9(num_user_sgprs >> 5);
   } else {
      si_pm4_set_reg(pm4, R_00B420_SPI_SHADER_PGM_LO_HS, va >> 8);
      si_pm4_set_reg(pm4, R_00B424_SPI_SHADER_PGM_HI_HS,
                     S_00B424_MEM_BASE(sscreen->info.address32_hi >> 8));

      shader->config.rsrc2 = S_00B42C_USER_SGPR(GFX6_TCS_NUM_USER_SGPR) | S_00B42C_OC_LDS_EN(1) |
                             S_00B42C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0);
   }

   si_pm4_set_reg(
      pm4, R_00B428_SPI_SHADER_PGM_RSRC1_HS,
      S_00B428_VGPRS((shader->config.num_vgprs - 1) / (sscreen->ge_wave_size == 32 ? 8 : 4)) |
         (sscreen->info.chip_class <= GFX9 ? S_00B428_SGPRS((shader->config.num_sgprs - 1) / 8)
                                           : 0) |
         S_00B428_DX10_CLAMP(1) | S_00B428_MEM_ORDERED(si_shader_mem_ordered(shader)) |
         S_00B428_WGP_MODE(sscreen->info.chip_class >= GFX10) |
         S_00B428_FLOAT_MODE(shader->config.float_mode) |
         S_00B428_LS_VGPR_COMP_CNT(sscreen->info.chip_class >= GFX9
                                      ? si_get_vs_vgpr_comp_cnt(sscreen, shader, false)
                                      : 0));

   if (sscreen->info.chip_class <= GFX8) {
      si_pm4_set_reg(pm4, R_00B42C_SPI_SHADER_PGM_RSRC2_HS, shader->config.rsrc2);
   }
}

static void si_emit_shader_es(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.es;
   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_028AAC_VGT_ESGS_RING_ITEMSIZE,
                              SI_TRACKED_VGT_ESGS_RING_ITEMSIZE,
                              shader->selector->esgs_itemsize / 4);

   if (shader->selector->info.stage == MESA_SHADER_TESS_EVAL)
      radeon_opt_set_context_reg(sctx, R_028B6C_VGT_TF_PARAM, SI_TRACKED_VGT_TF_PARAM,
                                 shader->vgt_tf_param);

   if (shader->vgt_vertex_reuse_block_cntl)
      radeon_opt_set_context_reg(sctx, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL,
                                 SI_TRACKED_VGT_VERTEX_REUSE_BLOCK_CNTL,
                                 shader->vgt_vertex_reuse_block_cntl);
   radeon_end_update_context_roll(sctx);
}

static void si_shader_es(struct si_screen *sscreen, struct si_shader *shader)
{
   struct si_pm4_state *pm4;
   unsigned num_user_sgprs;
   unsigned vgpr_comp_cnt;
   uint64_t va;
   unsigned oc_lds_en;

   assert(sscreen->info.chip_class <= GFX8);

   pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   pm4->atom.emit = si_emit_shader_es;
   va = shader->bo->gpu_address;

   if (shader->selector->info.stage == MESA_SHADER_VERTEX) {
      vgpr_comp_cnt = si_get_vs_vgpr_comp_cnt(sscreen, shader, false);
      num_user_sgprs = si_get_num_vs_user_sgprs(shader, SI_VS_NUM_USER_SGPR);
   } else if (shader->selector->info.stage == MESA_SHADER_TESS_EVAL) {
      vgpr_comp_cnt = shader->selector->info.uses_primid ? 3 : 2;
      num_user_sgprs = SI_TES_NUM_USER_SGPR;
   } else
      unreachable("invalid shader selector type");

   oc_lds_en = shader->selector->info.stage == MESA_SHADER_TESS_EVAL ? 1 : 0;

   si_pm4_set_reg(pm4, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
   si_pm4_set_reg(pm4, R_00B324_SPI_SHADER_PGM_HI_ES,
                  S_00B324_MEM_BASE(sscreen->info.address32_hi >> 8));
   si_pm4_set_reg(pm4, R_00B328_SPI_SHADER_PGM_RSRC1_ES,
                  S_00B328_VGPRS((shader->config.num_vgprs - 1) / 4) |
                     S_00B328_SGPRS((shader->config.num_sgprs - 1) / 8) |
                     S_00B328_VGPR_COMP_CNT(vgpr_comp_cnt) | S_00B328_DX10_CLAMP(1) |
                     S_00B328_FLOAT_MODE(shader->config.float_mode));
   si_pm4_set_reg(pm4, R_00B32C_SPI_SHADER_PGM_RSRC2_ES,
                  S_00B32C_USER_SGPR(num_user_sgprs) | S_00B32C_OC_LDS_EN(oc_lds_en) |
                     S_00B32C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0));

   if (shader->selector->info.stage == MESA_SHADER_TESS_EVAL)
      si_set_tesseval_regs(sscreen, shader->selector, shader);

   polaris_set_vgt_vertex_reuse(sscreen, shader->selector, shader);
}

void gfx9_get_gs_info(struct si_shader_selector *es, struct si_shader_selector *gs,
                      struct gfx9_gs_info *out)
{
   unsigned gs_num_invocations = MAX2(gs->info.base.gs.invocations, 1);
   unsigned input_prim = gs->info.base.gs.input_primitive;
   bool uses_adjacency =
      input_prim >= PIPE_PRIM_LINES_ADJACENCY && input_prim <= PIPE_PRIM_TRIANGLE_STRIP_ADJACENCY;

   /* All these are in dwords: */
   /* We can't allow using the whole LDS, because GS waves compete with
    * other shader stages for LDS space. */
   const unsigned max_lds_size = 8 * 1024;
   const unsigned esgs_itemsize = es->esgs_itemsize / 4;
   unsigned esgs_lds_size;

   /* All these are per subgroup: */
   const unsigned max_out_prims = 32 * 1024;
   const unsigned max_es_verts = 255;
   const unsigned ideal_gs_prims = 64;
   unsigned max_gs_prims, gs_prims;
   unsigned min_es_verts, es_verts, worst_case_es_verts;

   if (uses_adjacency || gs_num_invocations > 1)
      max_gs_prims = 127 / gs_num_invocations;
   else
      max_gs_prims = 255;

   /* MAX_PRIMS_PER_SUBGROUP = gs_prims * max_vert_out * gs_invocations.
    * Make sure we don't go over the maximum value.
    */
   if (gs->info.base.gs.vertices_out > 0) {
      max_gs_prims =
         MIN2(max_gs_prims, max_out_prims / (gs->info.base.gs.vertices_out * gs_num_invocations));
   }
   assert(max_gs_prims > 0);

   /* If the primitive has adjacency, halve the number of vertices
    * that will be reused in multiple primitives.
    */
   min_es_verts = gs->gs_input_verts_per_prim / (uses_adjacency ? 2 : 1);

   gs_prims = MIN2(ideal_gs_prims, max_gs_prims);
   worst_case_es_verts = MIN2(min_es_verts * gs_prims, max_es_verts);

   /* Compute ESGS LDS size based on the worst case number of ES vertices
    * needed to create the target number of GS prims per subgroup.
    */
   esgs_lds_size = esgs_itemsize * worst_case_es_verts;

   /* If total LDS usage is too big, refactor partitions based on ratio
    * of ESGS item sizes.
    */
   if (esgs_lds_size > max_lds_size) {
      /* Our target GS Prims Per Subgroup was too large. Calculate
       * the maximum number of GS Prims Per Subgroup that will fit
       * into LDS, capped by the maximum that the hardware can support.
       */
      gs_prims = MIN2((max_lds_size / (esgs_itemsize * min_es_verts)), max_gs_prims);
      assert(gs_prims > 0);
      worst_case_es_verts = MIN2(min_es_verts * gs_prims, max_es_verts);

      esgs_lds_size = esgs_itemsize * worst_case_es_verts;
      assert(esgs_lds_size <= max_lds_size);
   }

   /* Now calculate remaining ESGS information. */
   if (esgs_lds_size)
      es_verts = MIN2(esgs_lds_size / esgs_itemsize, max_es_verts);
   else
      es_verts = max_es_verts;

   /* Vertices for adjacency primitives are not always reused, so restore
    * it for ES_VERTS_PER_SUBGRP.
    */
   min_es_verts = gs->gs_input_verts_per_prim;

   /* For normal primitives, the VGT only checks if they are past the ES
    * verts per subgroup after allocating a full GS primitive and if they
    * are, kick off a new subgroup.  But if those additional ES verts are
    * unique (e.g. not reused) we need to make sure there is enough LDS
    * space to account for those ES verts beyond ES_VERTS_PER_SUBGRP.
    */
   es_verts -= min_es_verts - 1;

   out->es_verts_per_subgroup = es_verts;
   out->gs_prims_per_subgroup = gs_prims;
   out->gs_inst_prims_in_subgroup = gs_prims * gs_num_invocations;
   out->max_prims_per_subgroup = out->gs_inst_prims_in_subgroup * gs->info.base.gs.vertices_out;
   out->esgs_ring_size = esgs_lds_size;

   assert(out->max_prims_per_subgroup <= max_out_prims);
}

static void si_emit_shader_gs(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.gs;
   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);

   /* R_028A60_VGT_GSVS_RING_OFFSET_1, R_028A64_VGT_GSVS_RING_OFFSET_2
    * R_028A68_VGT_GSVS_RING_OFFSET_3 */
   radeon_opt_set_context_reg3(
      sctx, R_028A60_VGT_GSVS_RING_OFFSET_1, SI_TRACKED_VGT_GSVS_RING_OFFSET_1,
      shader->ctx_reg.gs.vgt_gsvs_ring_offset_1, shader->ctx_reg.gs.vgt_gsvs_ring_offset_2,
      shader->ctx_reg.gs.vgt_gsvs_ring_offset_3);

   /* R_028AB0_VGT_GSVS_RING_ITEMSIZE */
   radeon_opt_set_context_reg(sctx, R_028AB0_VGT_GSVS_RING_ITEMSIZE,
                              SI_TRACKED_VGT_GSVS_RING_ITEMSIZE,
                              shader->ctx_reg.gs.vgt_gsvs_ring_itemsize);

   /* R_028B38_VGT_GS_MAX_VERT_OUT */
   radeon_opt_set_context_reg(sctx, R_028B38_VGT_GS_MAX_VERT_OUT, SI_TRACKED_VGT_GS_MAX_VERT_OUT,
                              shader->ctx_reg.gs.vgt_gs_max_vert_out);

   /* R_028B5C_VGT_GS_VERT_ITEMSIZE, R_028B60_VGT_GS_VERT_ITEMSIZE_1
    * R_028B64_VGT_GS_VERT_ITEMSIZE_2, R_028B68_VGT_GS_VERT_ITEMSIZE_3 */
   radeon_opt_set_context_reg4(
      sctx, R_028B5C_VGT_GS_VERT_ITEMSIZE, SI_TRACKED_VGT_GS_VERT_ITEMSIZE,
      shader->ctx_reg.gs.vgt_gs_vert_itemsize, shader->ctx_reg.gs.vgt_gs_vert_itemsize_1,
      shader->ctx_reg.gs.vgt_gs_vert_itemsize_2, shader->ctx_reg.gs.vgt_gs_vert_itemsize_3);

   /* R_028B90_VGT_GS_INSTANCE_CNT */
   radeon_opt_set_context_reg(sctx, R_028B90_VGT_GS_INSTANCE_CNT, SI_TRACKED_VGT_GS_INSTANCE_CNT,
                              shader->ctx_reg.gs.vgt_gs_instance_cnt);

   if (sctx->chip_class >= GFX9) {
      /* R_028A44_VGT_GS_ONCHIP_CNTL */
      radeon_opt_set_context_reg(sctx, R_028A44_VGT_GS_ONCHIP_CNTL, SI_TRACKED_VGT_GS_ONCHIP_CNTL,
                                 shader->ctx_reg.gs.vgt_gs_onchip_cntl);
      /* R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP */
      radeon_opt_set_context_reg(sctx, R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP,
                                 SI_TRACKED_VGT_GS_MAX_PRIMS_PER_SUBGROUP,
                                 shader->ctx_reg.gs.vgt_gs_max_prims_per_subgroup);
      /* R_028AAC_VGT_ESGS_RING_ITEMSIZE */
      radeon_opt_set_context_reg(sctx, R_028AAC_VGT_ESGS_RING_ITEMSIZE,
                                 SI_TRACKED_VGT_ESGS_RING_ITEMSIZE,
                                 shader->ctx_reg.gs.vgt_esgs_ring_itemsize);

      if (shader->key.part.gs.es->info.stage == MESA_SHADER_TESS_EVAL)
         radeon_opt_set_context_reg(sctx, R_028B6C_VGT_TF_PARAM, SI_TRACKED_VGT_TF_PARAM,
                                    shader->vgt_tf_param);
      if (shader->vgt_vertex_reuse_block_cntl)
         radeon_opt_set_context_reg(sctx, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL,
                                    SI_TRACKED_VGT_VERTEX_REUSE_BLOCK_CNTL,
                                    shader->vgt_vertex_reuse_block_cntl);
   }
   radeon_end_update_context_roll(sctx);

   /* These don't cause any context rolls. */
   radeon_begin_again(&sctx->gfx_cs);
   if (sctx->chip_class >= GFX7) {
      radeon_opt_set_sh_reg(sctx, R_00B21C_SPI_SHADER_PGM_RSRC3_GS,
                            SI_TRACKED_SPI_SHADER_PGM_RSRC3_GS,
                            shader->ctx_reg.gs.spi_shader_pgm_rsrc3_gs);
   }
   if (sctx->chip_class >= GFX10) {
      radeon_opt_set_sh_reg(sctx, R_00B204_SPI_SHADER_PGM_RSRC4_GS,
                            SI_TRACKED_SPI_SHADER_PGM_RSRC4_GS,
                            shader->ctx_reg.gs.spi_shader_pgm_rsrc4_gs);
   }
   radeon_end();
}

static void si_shader_gs(struct si_screen *sscreen, struct si_shader *shader)
{
   struct si_shader_selector *sel = shader->selector;
   const ubyte *num_components = sel->info.num_stream_output_components;
   unsigned gs_num_invocations = sel->info.base.gs.invocations;
   struct si_pm4_state *pm4;
   uint64_t va;
   unsigned max_stream = util_last_bit(sel->info.base.gs.active_stream_mask);
   unsigned offset;

   pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   pm4->atom.emit = si_emit_shader_gs;

   offset = num_components[0] * sel->info.base.gs.vertices_out;
   shader->ctx_reg.gs.vgt_gsvs_ring_offset_1 = offset;

   if (max_stream >= 2)
      offset += num_components[1] * sel->info.base.gs.vertices_out;
   shader->ctx_reg.gs.vgt_gsvs_ring_offset_2 = offset;

   if (max_stream >= 3)
      offset += num_components[2] * sel->info.base.gs.vertices_out;
   shader->ctx_reg.gs.vgt_gsvs_ring_offset_3 = offset;

   if (max_stream >= 4)
      offset += num_components[3] * sel->info.base.gs.vertices_out;
   shader->ctx_reg.gs.vgt_gsvs_ring_itemsize = offset;

   /* The GSVS_RING_ITEMSIZE register takes 15 bits */
   assert(offset < (1 << 15));

   shader->ctx_reg.gs.vgt_gs_max_vert_out = sel->info.base.gs.vertices_out;

   shader->ctx_reg.gs.vgt_gs_vert_itemsize = num_components[0];
   shader->ctx_reg.gs.vgt_gs_vert_itemsize_1 = (max_stream >= 2) ? num_components[1] : 0;
   shader->ctx_reg.gs.vgt_gs_vert_itemsize_2 = (max_stream >= 3) ? num_components[2] : 0;
   shader->ctx_reg.gs.vgt_gs_vert_itemsize_3 = (max_stream >= 4) ? num_components[3] : 0;

   shader->ctx_reg.gs.vgt_gs_instance_cnt =
      S_028B90_CNT(MIN2(gs_num_invocations, 127)) | S_028B90_ENABLE(gs_num_invocations > 0);

   /* Copy over fields from the GS copy shader to make them easily accessible from GS. */
   shader->pa_cl_vs_out_cntl = sel->gs_copy_shader->pa_cl_vs_out_cntl;

   va = shader->bo->gpu_address;

   if (sscreen->info.chip_class >= GFX9) {
      unsigned input_prim = sel->info.base.gs.input_primitive;
      gl_shader_stage es_stage = shader->key.part.gs.es->info.stage;
      unsigned es_vgpr_comp_cnt, gs_vgpr_comp_cnt;

      if (es_stage == MESA_SHADER_VERTEX) {
         es_vgpr_comp_cnt = si_get_vs_vgpr_comp_cnt(sscreen, shader, false);
      } else if (es_stage == MESA_SHADER_TESS_EVAL)
         es_vgpr_comp_cnt = shader->key.part.gs.es->info.uses_primid ? 3 : 2;
      else
         unreachable("invalid shader selector type");

      /* If offsets 4, 5 are used, GS_VGPR_COMP_CNT is ignored and
       * VGPR[0:4] are always loaded.
       */
      if (sel->info.uses_invocationid)
         gs_vgpr_comp_cnt = 3; /* VGPR3 contains InvocationID. */
      else if (sel->info.uses_primid)
         gs_vgpr_comp_cnt = 2; /* VGPR2 contains PrimitiveID. */
      else if (input_prim >= PIPE_PRIM_TRIANGLES)
         gs_vgpr_comp_cnt = 1; /* VGPR1 contains offsets 2, 3 */
      else
         gs_vgpr_comp_cnt = 0; /* VGPR0 contains offsets 0, 1 */

      unsigned num_user_sgprs;
      if (es_stage == MESA_SHADER_VERTEX)
         num_user_sgprs = si_get_num_vs_user_sgprs(shader, GFX9_VSGS_NUM_USER_SGPR);
      else
         num_user_sgprs = GFX9_TESGS_NUM_USER_SGPR;

      if (sscreen->info.chip_class >= GFX10) {
         si_pm4_set_reg(pm4, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
      } else {
         si_pm4_set_reg(pm4, R_00B210_SPI_SHADER_PGM_LO_ES, va >> 8);
      }

      uint32_t rsrc1 = S_00B228_VGPRS((shader->config.num_vgprs - 1) / 4) | S_00B228_DX10_CLAMP(1) |
                       S_00B228_MEM_ORDERED(si_shader_mem_ordered(shader)) |
                       S_00B228_WGP_MODE(sscreen->info.chip_class >= GFX10) |
                       S_00B228_FLOAT_MODE(shader->config.float_mode) |
                       S_00B228_GS_VGPR_COMP_CNT(gs_vgpr_comp_cnt);
      uint32_t rsrc2 = S_00B22C_USER_SGPR(num_user_sgprs) |
                       S_00B22C_ES_VGPR_COMP_CNT(es_vgpr_comp_cnt) |
                       S_00B22C_OC_LDS_EN(es_stage == MESA_SHADER_TESS_EVAL) |
                       S_00B22C_LDS_SIZE(shader->config.lds_size) |
                       S_00B22C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0);

      if (sscreen->info.chip_class >= GFX10) {
         rsrc2 |= S_00B22C_USER_SGPR_MSB_GFX10(num_user_sgprs >> 5);
      } else {
         rsrc1 |= S_00B228_SGPRS((shader->config.num_sgprs - 1) / 8);
         rsrc2 |= S_00B22C_USER_SGPR_MSB_GFX9(num_user_sgprs >> 5);
      }

      si_pm4_set_reg(pm4, R_00B228_SPI_SHADER_PGM_RSRC1_GS, rsrc1);
      si_pm4_set_reg(pm4, R_00B22C_SPI_SHADER_PGM_RSRC2_GS, rsrc2);

      shader->ctx_reg.gs.spi_shader_pgm_rsrc3_gs = S_00B21C_CU_EN(0xffff) |
                                                   S_00B21C_WAVE_LIMIT(0x3F);
      shader->ctx_reg.gs.spi_shader_pgm_rsrc4_gs =
         S_00B204_CU_EN(0xffff) | S_00B204_SPI_SHADER_LATE_ALLOC_GS_GFX10(0);

      shader->ctx_reg.gs.vgt_gs_onchip_cntl =
         S_028A44_ES_VERTS_PER_SUBGRP(shader->gs_info.es_verts_per_subgroup) |
         S_028A44_GS_PRIMS_PER_SUBGRP(shader->gs_info.gs_prims_per_subgroup) |
         S_028A44_GS_INST_PRIMS_IN_SUBGRP(shader->gs_info.gs_inst_prims_in_subgroup);
      shader->ctx_reg.gs.vgt_gs_max_prims_per_subgroup =
         S_028A94_MAX_PRIMS_PER_SUBGROUP(shader->gs_info.max_prims_per_subgroup);
      shader->ctx_reg.gs.vgt_esgs_ring_itemsize = shader->key.part.gs.es->esgs_itemsize / 4;

      if (es_stage == MESA_SHADER_TESS_EVAL)
         si_set_tesseval_regs(sscreen, shader->key.part.gs.es, shader);

      polaris_set_vgt_vertex_reuse(sscreen, shader->key.part.gs.es, shader);
   } else {
      shader->ctx_reg.gs.spi_shader_pgm_rsrc3_gs = S_00B21C_CU_EN(0xffff) |
                                                   S_00B21C_WAVE_LIMIT(0x3F);

      si_pm4_set_reg(pm4, R_00B220_SPI_SHADER_PGM_LO_GS, va >> 8);
      si_pm4_set_reg(pm4, R_00B224_SPI_SHADER_PGM_HI_GS,
                     S_00B224_MEM_BASE(sscreen->info.address32_hi >> 8));

      si_pm4_set_reg(pm4, R_00B228_SPI_SHADER_PGM_RSRC1_GS,
                     S_00B228_VGPRS((shader->config.num_vgprs - 1) / 4) |
                        S_00B228_SGPRS((shader->config.num_sgprs - 1) / 8) |
                        S_00B228_DX10_CLAMP(1) | S_00B228_FLOAT_MODE(shader->config.float_mode));
      si_pm4_set_reg(pm4, R_00B22C_SPI_SHADER_PGM_RSRC2_GS,
                     S_00B22C_USER_SGPR(GFX6_GS_NUM_USER_SGPR) |
                        S_00B22C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0));
   }
}

bool gfx10_is_ngg_passthrough(struct si_shader *shader)
{
   struct si_shader_selector *sel = shader->selector;

   /* Never use NGG passthrough if culling is possible even when it's not used by this shader,
    * so that we don't get context rolls when enabling and disabling NGG passthrough.
    */
   if (sel->screen->use_ngg_culling)
      return false;

   /* The definition of NGG passthrough is:
    * - user GS is turned off (no amplification, no GS instancing, and no culling)
    * - VGT_ESGS_RING_ITEMSIZE is ignored (behaving as if it was equal to 1)
    * - vertex indices are packed into 1 VGPR
    * - Dimgrey and later chips can optionally skip the gs_alloc_req message
    *
    * NGG passthrough still allows the use of LDS.
    */
   return sel->info.stage != MESA_SHADER_GEOMETRY && !shader->key.opt.ngg_culling;
}

/* Common tail code for NGG primitive shaders. */
static void gfx10_emit_shader_ngg_tail(struct si_context *sctx, struct si_shader *shader)
{
   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,
                              SI_TRACKED_GE_MAX_OUTPUT_PER_SUBGROUP,
                              shader->ctx_reg.ngg.ge_max_output_per_subgroup);
   radeon_opt_set_context_reg(sctx, R_028B4C_GE_NGG_SUBGRP_CNTL, SI_TRACKED_GE_NGG_SUBGRP_CNTL,
                              shader->ctx_reg.ngg.ge_ngg_subgrp_cntl);
   radeon_opt_set_context_reg(sctx, R_028A84_VGT_PRIMITIVEID_EN, SI_TRACKED_VGT_PRIMITIVEID_EN,
                              shader->ctx_reg.ngg.vgt_primitiveid_en);
   radeon_opt_set_context_reg(sctx, R_028A44_VGT_GS_ONCHIP_CNTL, SI_TRACKED_VGT_GS_ONCHIP_CNTL,
                              shader->ctx_reg.ngg.vgt_gs_onchip_cntl);
   radeon_opt_set_context_reg(sctx, R_028B90_VGT_GS_INSTANCE_CNT, SI_TRACKED_VGT_GS_INSTANCE_CNT,
                              shader->ctx_reg.ngg.vgt_gs_instance_cnt);
   radeon_opt_set_context_reg(sctx, R_028AAC_VGT_ESGS_RING_ITEMSIZE,
                              SI_TRACKED_VGT_ESGS_RING_ITEMSIZE,
                              shader->ctx_reg.ngg.vgt_esgs_ring_itemsize);
   radeon_opt_set_context_reg(sctx, R_0286C4_SPI_VS_OUT_CONFIG, SI_TRACKED_SPI_VS_OUT_CONFIG,
                              shader->ctx_reg.ngg.spi_vs_out_config);
   radeon_opt_set_context_reg2(
      sctx, R_028708_SPI_SHADER_IDX_FORMAT, SI_TRACKED_SPI_SHADER_IDX_FORMAT,
      shader->ctx_reg.ngg.spi_shader_idx_format, shader->ctx_reg.ngg.spi_shader_pos_format);
   radeon_opt_set_context_reg(sctx, R_028818_PA_CL_VTE_CNTL, SI_TRACKED_PA_CL_VTE_CNTL,
                              shader->ctx_reg.ngg.pa_cl_vte_cntl);
   radeon_opt_set_context_reg(sctx, R_028838_PA_CL_NGG_CNTL, SI_TRACKED_PA_CL_NGG_CNTL,
                              shader->ctx_reg.ngg.pa_cl_ngg_cntl);

   radeon_end_update_context_roll(sctx);

   /* These don't cause a context roll. */
   radeon_begin_again(&sctx->gfx_cs);
   radeon_opt_set_uconfig_reg(sctx, R_030980_GE_PC_ALLOC, SI_TRACKED_GE_PC_ALLOC,
                              shader->ctx_reg.ngg.ge_pc_alloc);
   radeon_opt_set_sh_reg(sctx, R_00B21C_SPI_SHADER_PGM_RSRC3_GS,
                         SI_TRACKED_SPI_SHADER_PGM_RSRC3_GS,
                         shader->ctx_reg.ngg.spi_shader_pgm_rsrc3_gs);
   radeon_opt_set_sh_reg(sctx, R_00B204_SPI_SHADER_PGM_RSRC4_GS,
                         SI_TRACKED_SPI_SHADER_PGM_RSRC4_GS,
                         shader->ctx_reg.ngg.spi_shader_pgm_rsrc4_gs);
   radeon_end();
}

static void gfx10_emit_shader_ngg_notess_nogs(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.gs;
   if (!shader)
      return;

   gfx10_emit_shader_ngg_tail(sctx, shader);
}

static void gfx10_emit_shader_ngg_tess_nogs(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.gs;
   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_028B6C_VGT_TF_PARAM, SI_TRACKED_VGT_TF_PARAM,
                              shader->vgt_tf_param);
   radeon_end_update_context_roll(sctx);

   gfx10_emit_shader_ngg_tail(sctx, shader);
}

static void gfx10_emit_shader_ngg_notess_gs(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.gs;
   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_028B38_VGT_GS_MAX_VERT_OUT, SI_TRACKED_VGT_GS_MAX_VERT_OUT,
                              shader->ctx_reg.ngg.vgt_gs_max_vert_out);
   radeon_end_update_context_roll(sctx);

   gfx10_emit_shader_ngg_tail(sctx, shader);
}

static void gfx10_emit_shader_ngg_tess_gs(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.gs;

   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_028B38_VGT_GS_MAX_VERT_OUT, SI_TRACKED_VGT_GS_MAX_VERT_OUT,
                              shader->ctx_reg.ngg.vgt_gs_max_vert_out);
   radeon_opt_set_context_reg(sctx, R_028B6C_VGT_TF_PARAM, SI_TRACKED_VGT_TF_PARAM,
                              shader->vgt_tf_param);
   radeon_end_update_context_roll(sctx);

   gfx10_emit_shader_ngg_tail(sctx, shader);
}

unsigned si_get_input_prim(const struct si_shader_selector *gs, const struct si_shader_key *key)
{
   if (gs->info.stage == MESA_SHADER_GEOMETRY)
      return gs->info.base.gs.input_primitive;

   if (gs->info.stage == MESA_SHADER_TESS_EVAL) {
      if (gs->info.base.tess.point_mode)
         return PIPE_PRIM_POINTS;
      if (gs->info.base.tess.primitive_mode == GL_LINES)
         return PIPE_PRIM_LINES;
      return PIPE_PRIM_TRIANGLES;
   }

   if (key->opt.ngg_culling & SI_NGG_CULL_LINES)
      return PIPE_PRIM_LINES;

   return PIPE_PRIM_TRIANGLES; /* worst case for all callers */
}

static unsigned si_get_vs_out_cntl(const struct si_shader_selector *sel,
                                   const struct si_shader *shader, bool ngg)
{
   /* Clip distances can be killed, but cull distances can't. */
   unsigned clipcull_mask = (sel->clipdist_mask & ~shader->key.opt.kill_clip_distances) |
                            sel->culldist_mask;
   bool writes_psize = sel->info.writes_psize && !shader->key.opt.kill_pointsize;
   bool misc_vec_ena = writes_psize || (sel->info.writes_edgeflag && !ngg) ||
                       sel->screen->options.vrs2x2 ||
                       sel->info.writes_layer || sel->info.writes_viewport_index;

   return S_02881C_VS_OUT_CCDIST0_VEC_ENA((clipcull_mask & 0x0F) != 0) |
          S_02881C_VS_OUT_CCDIST1_VEC_ENA((clipcull_mask & 0xF0) != 0) |
          S_02881C_USE_VTX_POINT_SIZE(writes_psize) |
          S_02881C_USE_VTX_EDGE_FLAG(sel->info.writes_edgeflag && !ngg) |
          S_02881C_USE_VTX_VRS_RATE(sel->screen->options.vrs2x2) |
          S_02881C_USE_VTX_RENDER_TARGET_INDX(sel->info.writes_layer) |
          S_02881C_USE_VTX_VIEWPORT_INDX(sel->info.writes_viewport_index) |
          S_02881C_VS_OUT_MISC_VEC_ENA(misc_vec_ena) |
          S_02881C_VS_OUT_MISC_SIDE_BUS_ENA(misc_vec_ena);
}

/**
 * Prepare the PM4 image for \p shader, which will run as a merged ESGS shader
 * in NGG mode.
 */
static void gfx10_shader_ngg(struct si_screen *sscreen, struct si_shader *shader)
{
   const struct si_shader_selector *gs_sel = shader->selector;
   const struct si_shader_info *gs_info = &gs_sel->info;
   const gl_shader_stage gs_stage = shader->selector->info.stage;
   const struct si_shader_selector *es_sel =
      shader->previous_stage_sel ? shader->previous_stage_sel : shader->selector;
   const struct si_shader_info *es_info = &es_sel->info;
   const gl_shader_stage es_stage = es_sel->info.stage;
   unsigned num_user_sgprs;
   unsigned nparams, es_vgpr_comp_cnt, gs_vgpr_comp_cnt;
   uint64_t va;
   bool window_space = gs_info->stage == MESA_SHADER_VERTEX ?
                          gs_info->base.vs.window_space_position : 0;
   bool es_enable_prim_id = shader->key.mono.u.vs_export_prim_id || es_info->uses_primid;
   unsigned gs_num_invocations = MAX2(gs_sel->info.base.gs.invocations, 1);
   unsigned input_prim = si_get_input_prim(gs_sel, &shader->key);
   bool break_wave_at_eoi = false;
   struct si_pm4_state *pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   if (es_stage == MESA_SHADER_TESS_EVAL) {
      pm4->atom.emit = gs_stage == MESA_SHADER_GEOMETRY ? gfx10_emit_shader_ngg_tess_gs
                                                       : gfx10_emit_shader_ngg_tess_nogs;
   } else {
      pm4->atom.emit = gs_stage == MESA_SHADER_GEOMETRY ? gfx10_emit_shader_ngg_notess_gs
                                                       : gfx10_emit_shader_ngg_notess_nogs;
   }

   va = shader->bo->gpu_address;

   if (es_stage == MESA_SHADER_VERTEX) {
      es_vgpr_comp_cnt = si_get_vs_vgpr_comp_cnt(sscreen, shader, false);

      if (es_info->base.vs.blit_sgprs_amd) {
         num_user_sgprs =
            SI_SGPR_VS_BLIT_DATA + es_info->base.vs.blit_sgprs_amd;
      } else {
         num_user_sgprs = si_get_num_vs_user_sgprs(shader, GFX9_VSGS_NUM_USER_SGPR);
      }
   } else {
      assert(es_stage == MESA_SHADER_TESS_EVAL);
      es_vgpr_comp_cnt = es_enable_prim_id ? 3 : 2;
      num_user_sgprs = GFX9_TESGS_NUM_USER_SGPR;

      if (es_enable_prim_id || gs_info->uses_primid)
         break_wave_at_eoi = true;
   }

   /* If offsets 4, 5 are used, GS_VGPR_COMP_CNT is ignored and
    * VGPR[0:4] are always loaded.
    *
    * Vertex shaders always need to load VGPR3, because they need to
    * pass edge flags for decomposed primitives (such as quads) to the PA
    * for the GL_LINE polygon mode to skip rendering lines on inner edges.
    */
   if (gs_info->uses_invocationid ||
       (gfx10_edgeflags_have_effect(shader) && !gfx10_is_ngg_passthrough(shader)))
      gs_vgpr_comp_cnt = 3; /* VGPR3 contains InvocationID, edge flags. */
   else if ((gs_stage == MESA_SHADER_GEOMETRY && gs_info->uses_primid) ||
            (gs_stage == MESA_SHADER_VERTEX && shader->key.mono.u.vs_export_prim_id))
      gs_vgpr_comp_cnt = 2; /* VGPR2 contains PrimitiveID. */
   else if (input_prim >= PIPE_PRIM_TRIANGLES && !gfx10_is_ngg_passthrough(shader))
      gs_vgpr_comp_cnt = 1; /* VGPR1 contains offsets 2, 3 */
   else
      gs_vgpr_comp_cnt = 0; /* VGPR0 contains offsets 0, 1 */

   unsigned wave_size = si_get_shader_wave_size(shader);
   unsigned late_alloc_wave64, cu_mask;

   ac_compute_late_alloc(&sscreen->info, true, shader->key.opt.ngg_culling,
                         shader->config.scratch_bytes_per_wave > 0,
                         &late_alloc_wave64, &cu_mask);

   si_pm4_set_reg(pm4, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
   si_pm4_set_reg(
      pm4, R_00B228_SPI_SHADER_PGM_RSRC1_GS,
      S_00B228_VGPRS((shader->config.num_vgprs - 1) / (wave_size == 32 ? 8 : 4)) |
         S_00B228_FLOAT_MODE(shader->config.float_mode) | S_00B228_DX10_CLAMP(1) |
         S_00B228_MEM_ORDERED(si_shader_mem_ordered(shader)) |
         /* Disable the WGP mode on gfx10.3 because it can hang. (it happened on VanGogh)
          * Let's disable it on all chips that disable exactly 1 CU per SA for GS. */
         S_00B228_WGP_MODE(sscreen->info.chip_class == GFX10) |
         S_00B228_GS_VGPR_COMP_CNT(gs_vgpr_comp_cnt));
   si_pm4_set_reg(pm4, R_00B22C_SPI_SHADER_PGM_RSRC2_GS,
                  S_00B22C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0) |
                     S_00B22C_USER_SGPR(num_user_sgprs) |
                     S_00B22C_ES_VGPR_COMP_CNT(es_vgpr_comp_cnt) |
                     S_00B22C_USER_SGPR_MSB_GFX10(num_user_sgprs >> 5) |
                     S_00B22C_OC_LDS_EN(es_stage == MESA_SHADER_TESS_EVAL) |
                     S_00B22C_LDS_SIZE(shader->config.lds_size));

   shader->ctx_reg.ngg.spi_shader_pgm_rsrc3_gs = S_00B21C_CU_EN(cu_mask) |
                                                 S_00B21C_WAVE_LIMIT(0x3F);
   shader->ctx_reg.ngg.spi_shader_pgm_rsrc4_gs =
      S_00B204_CU_EN(0xffff) | S_00B204_SPI_SHADER_LATE_ALLOC_GS_GFX10(late_alloc_wave64);

   nparams = MAX2(shader->info.nr_param_exports, 1);
   shader->ctx_reg.ngg.spi_vs_out_config =
      S_0286C4_VS_EXPORT_COUNT(nparams - 1) |
      S_0286C4_NO_PC_EXPORT(shader->info.nr_param_exports == 0);

   shader->ctx_reg.ngg.spi_shader_idx_format =
      S_028708_IDX0_EXPORT_FORMAT(V_028708_SPI_SHADER_1COMP);
   shader->ctx_reg.ngg.spi_shader_pos_format =
      S_02870C_POS0_EXPORT_FORMAT(V_02870C_SPI_SHADER_4COMP) |
      S_02870C_POS1_EXPORT_FORMAT(shader->info.nr_pos_exports > 1 ? V_02870C_SPI_SHADER_4COMP
                                                                  : V_02870C_SPI_SHADER_NONE) |
      S_02870C_POS2_EXPORT_FORMAT(shader->info.nr_pos_exports > 2 ? V_02870C_SPI_SHADER_4COMP
                                                                  : V_02870C_SPI_SHADER_NONE) |
      S_02870C_POS3_EXPORT_FORMAT(shader->info.nr_pos_exports > 3 ? V_02870C_SPI_SHADER_4COMP
                                                                  : V_02870C_SPI_SHADER_NONE);

   shader->ctx_reg.ngg.vgt_primitiveid_en =
      S_028A84_PRIMITIVEID_EN(es_enable_prim_id) |
      S_028A84_NGG_DISABLE_PROVOK_REUSE(shader->key.mono.u.vs_export_prim_id ||
                                        gs_sel->info.writes_primid);

   if (gs_stage == MESA_SHADER_GEOMETRY) {
      shader->ctx_reg.ngg.vgt_esgs_ring_itemsize = es_sel->esgs_itemsize / 4;
      shader->ctx_reg.ngg.vgt_gs_max_vert_out = gs_sel->info.base.gs.vertices_out;
   } else {
      shader->ctx_reg.ngg.vgt_esgs_ring_itemsize = 1;
   }

   if (es_stage == MESA_SHADER_TESS_EVAL)
      si_set_tesseval_regs(sscreen, es_sel, shader);

   shader->ctx_reg.ngg.vgt_gs_onchip_cntl =
      S_028A44_ES_VERTS_PER_SUBGRP(shader->ngg.hw_max_esverts) |
      S_028A44_GS_PRIMS_PER_SUBGRP(shader->ngg.max_gsprims) |
      S_028A44_GS_INST_PRIMS_IN_SUBGRP(shader->ngg.max_gsprims * gs_num_invocations);
   shader->ctx_reg.ngg.ge_max_output_per_subgroup =
      S_0287FC_MAX_VERTS_PER_SUBGROUP(shader->ngg.max_out_verts);
   shader->ctx_reg.ngg.ge_ngg_subgrp_cntl = S_028B4C_PRIM_AMP_FACTOR(shader->ngg.prim_amp_factor) |
                                            S_028B4C_THDS_PER_SUBGRP(0); /* for fast launch */
   shader->ctx_reg.ngg.vgt_gs_instance_cnt =
      S_028B90_CNT(gs_num_invocations) | S_028B90_ENABLE(gs_num_invocations > 1) |
      S_028B90_EN_MAX_VERT_OUT_PER_GS_INSTANCE(shader->ngg.max_vert_out_per_gs_instance);

   /* Output hw-generated edge flags if needed and pass them via the prim
    * export to prevent drawing lines on internal edges of decomposed
    * primitives (such as quads) with polygon mode = lines.
    */
   shader->ctx_reg.ngg.pa_cl_ngg_cntl =
      S_028838_INDEX_BUF_EDGE_FLAG_ENA(gfx10_edgeflags_have_effect(shader)) |
      /* Reuse for NGG. */
      S_028838_VERTEX_REUSE_DEPTH(sscreen->info.chip_class >= GFX10_3 ? 30 : 0);
   shader->pa_cl_vs_out_cntl = si_get_vs_out_cntl(shader->selector, shader, true);

   /* Oversubscribe PC. This improves performance when there are too many varyings. */
   unsigned oversub_pc_factor = 1;

   if (shader->key.opt.ngg_culling) {
      /* Be more aggressive with NGG culling. */
      if (shader->info.nr_param_exports > 4)
         oversub_pc_factor = 4;
      else if (shader->info.nr_param_exports > 2)
         oversub_pc_factor = 3;
      else
         oversub_pc_factor = 2;
   }

   unsigned oversub_pc_lines =
      late_alloc_wave64 ? (sscreen->info.pc_lines / 4) * oversub_pc_factor : 0;
   shader->ctx_reg.ngg.ge_pc_alloc = S_030980_OVERSUB_EN(oversub_pc_lines > 0) |
                                     S_030980_NUM_PC_LINES(oversub_pc_lines - 1);

   shader->ge_cntl = S_03096C_PRIM_GRP_SIZE(shader->ngg.max_gsprims) |
                     S_03096C_VERT_GRP_SIZE(shader->ngg.hw_max_esverts) |
                     S_03096C_BREAK_WAVE_AT_EOI(break_wave_at_eoi);

   /* On gfx10, the GE only checks against the maximum number of ES verts after
    * allocating a full GS primitive. So we need to ensure that whenever
    * this check passes, there is enough space for a full primitive without
    * vertex reuse. VERT_GRP_SIZE=256 doesn't need this. We should always get 256
    * if we have enough LDS.
    *
    * Tessellation is unaffected because it always sets GE_CNTL.VERT_GRP_SIZE = 0.
    */
   if ((sscreen->info.chip_class == GFX10) &&
       (es_stage == MESA_SHADER_VERTEX || gs_stage == MESA_SHADER_VERTEX) && /* = no tess */
       shader->ngg.hw_max_esverts != 256 &&
       shader->ngg.hw_max_esverts > 5) {
      /* This could be based on the input primitive type. 5 is the worst case
       * for primitive types with adjacency.
       */
      shader->ge_cntl &= C_03096C_VERT_GRP_SIZE;
      shader->ge_cntl |= S_03096C_VERT_GRP_SIZE(shader->ngg.hw_max_esverts - 5);
   }

   if (window_space) {
      shader->ctx_reg.ngg.pa_cl_vte_cntl = S_028818_VTX_XY_FMT(1) | S_028818_VTX_Z_FMT(1);
   } else {
      shader->ctx_reg.ngg.pa_cl_vte_cntl =
         S_028818_VTX_W0_FMT(1) | S_028818_VPORT_X_SCALE_ENA(1) | S_028818_VPORT_X_OFFSET_ENA(1) |
         S_028818_VPORT_Y_SCALE_ENA(1) | S_028818_VPORT_Y_OFFSET_ENA(1) |
         S_028818_VPORT_Z_SCALE_ENA(1) | S_028818_VPORT_Z_OFFSET_ENA(1);
   }

   shader->ctx_reg.ngg.vgt_stages.u.ngg = 1;
   shader->ctx_reg.ngg.vgt_stages.u.streamout = gs_sel->so.num_outputs;
   shader->ctx_reg.ngg.vgt_stages.u.ngg_passthrough = gfx10_is_ngg_passthrough(shader);
}

static void si_emit_shader_vs(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.vs;
   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_028A40_VGT_GS_MODE, SI_TRACKED_VGT_GS_MODE,
                              shader->ctx_reg.vs.vgt_gs_mode);
   radeon_opt_set_context_reg(sctx, R_028A84_VGT_PRIMITIVEID_EN, SI_TRACKED_VGT_PRIMITIVEID_EN,
                              shader->ctx_reg.vs.vgt_primitiveid_en);

   if (sctx->chip_class <= GFX8) {
      radeon_opt_set_context_reg(sctx, R_028AB4_VGT_REUSE_OFF, SI_TRACKED_VGT_REUSE_OFF,
                                 shader->ctx_reg.vs.vgt_reuse_off);
   }

   radeon_opt_set_context_reg(sctx, R_0286C4_SPI_VS_OUT_CONFIG, SI_TRACKED_SPI_VS_OUT_CONFIG,
                              shader->ctx_reg.vs.spi_vs_out_config);

   radeon_opt_set_context_reg(sctx, R_02870C_SPI_SHADER_POS_FORMAT,
                              SI_TRACKED_SPI_SHADER_POS_FORMAT,
                              shader->ctx_reg.vs.spi_shader_pos_format);

   radeon_opt_set_context_reg(sctx, R_028818_PA_CL_VTE_CNTL, SI_TRACKED_PA_CL_VTE_CNTL,
                              shader->ctx_reg.vs.pa_cl_vte_cntl);

   if (shader->selector->info.stage == MESA_SHADER_TESS_EVAL)
      radeon_opt_set_context_reg(sctx, R_028B6C_VGT_TF_PARAM, SI_TRACKED_VGT_TF_PARAM,
                                 shader->vgt_tf_param);

   if (shader->vgt_vertex_reuse_block_cntl)
      radeon_opt_set_context_reg(sctx, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL,
                                 SI_TRACKED_VGT_VERTEX_REUSE_BLOCK_CNTL,
                                 shader->vgt_vertex_reuse_block_cntl);

   /* Required programming for tessellation. (legacy pipeline only) */
   if (sctx->chip_class >= GFX10 && shader->selector->info.stage == MESA_SHADER_TESS_EVAL) {
      radeon_opt_set_context_reg(sctx, R_028A44_VGT_GS_ONCHIP_CNTL,
                                 SI_TRACKED_VGT_GS_ONCHIP_CNTL,
                                 S_028A44_ES_VERTS_PER_SUBGRP(250) |
                                 S_028A44_GS_PRIMS_PER_SUBGRP(126) |
                                 S_028A44_GS_INST_PRIMS_IN_SUBGRP(126));
   }

   radeon_end_update_context_roll(sctx);

   /* GE_PC_ALLOC is not a context register, so it doesn't cause a context roll. */
   if (sctx->chip_class >= GFX10) {
      radeon_begin_again(&sctx->gfx_cs);
      radeon_opt_set_uconfig_reg(sctx, R_030980_GE_PC_ALLOC, SI_TRACKED_GE_PC_ALLOC,
                                 shader->ctx_reg.vs.ge_pc_alloc);
      radeon_end();
   }
}

/**
 * Compute the state for \p shader, which will run as a vertex shader on the
 * hardware.
 *
 * If \p gs is non-NULL, it points to the geometry shader for which this shader
 * is the copy shader.
 */
static void si_shader_vs(struct si_screen *sscreen, struct si_shader *shader,
                         struct si_shader_selector *gs)
{
   const struct si_shader_info *info = &shader->selector->info;
   struct si_pm4_state *pm4;
   unsigned num_user_sgprs, vgpr_comp_cnt;
   uint64_t va;
   unsigned nparams, oc_lds_en;
   bool window_space = info->stage == MESA_SHADER_VERTEX ?
                          info->base.vs.window_space_position : 0;
   bool enable_prim_id = shader->key.mono.u.vs_export_prim_id || info->uses_primid;

   pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   pm4->atom.emit = si_emit_shader_vs;

   /* We always write VGT_GS_MODE in the VS state, because every switch
    * between different shader pipelines involving a different GS or no
    * GS at all involves a switch of the VS (different GS use different
    * copy shaders). On the other hand, when the API switches from a GS to
    * no GS and then back to the same GS used originally, the GS state is
    * not sent again.
    */
   if (!gs) {
      unsigned mode = V_028A40_GS_OFF;

      /* PrimID needs GS scenario A. */
      if (enable_prim_id)
         mode = V_028A40_GS_SCENARIO_A;

      shader->ctx_reg.vs.vgt_gs_mode = S_028A40_MODE(mode);
      shader->ctx_reg.vs.vgt_primitiveid_en = enable_prim_id;
   } else {
      shader->ctx_reg.vs.vgt_gs_mode =
         ac_vgt_gs_mode(gs->info.base.gs.vertices_out, sscreen->info.chip_class);
      shader->ctx_reg.vs.vgt_primitiveid_en = 0;
   }

   if (sscreen->info.chip_class <= GFX8) {
      /* Reuse needs to be set off if we write oViewport. */
      shader->ctx_reg.vs.vgt_reuse_off = S_028AB4_REUSE_OFF(info->writes_viewport_index);
   }

   va = shader->bo->gpu_address;

   if (gs) {
      vgpr_comp_cnt = 0; /* only VertexID is needed for GS-COPY. */
      num_user_sgprs = SI_GSCOPY_NUM_USER_SGPR;
   } else if (shader->selector->info.stage == MESA_SHADER_VERTEX) {
      vgpr_comp_cnt = si_get_vs_vgpr_comp_cnt(sscreen, shader, enable_prim_id);

      if (info->base.vs.blit_sgprs_amd) {
         num_user_sgprs = SI_SGPR_VS_BLIT_DATA + info->base.vs.blit_sgprs_amd;
      } else {
         num_user_sgprs = si_get_num_vs_user_sgprs(shader, SI_VS_NUM_USER_SGPR);
      }
   } else if (shader->selector->info.stage == MESA_SHADER_TESS_EVAL) {
      vgpr_comp_cnt = enable_prim_id ? 3 : 2;
      num_user_sgprs = SI_TES_NUM_USER_SGPR;
   } else
      unreachable("invalid shader selector type");

   /* VS is required to export at least one param. */
   nparams = MAX2(shader->info.nr_param_exports, 1);
   shader->ctx_reg.vs.spi_vs_out_config = S_0286C4_VS_EXPORT_COUNT(nparams - 1);

   if (sscreen->info.chip_class >= GFX10) {
      shader->ctx_reg.vs.spi_vs_out_config |=
         S_0286C4_NO_PC_EXPORT(shader->info.nr_param_exports == 0);
   }

   shader->ctx_reg.vs.spi_shader_pos_format =
      S_02870C_POS0_EXPORT_FORMAT(V_02870C_SPI_SHADER_4COMP) |
      S_02870C_POS1_EXPORT_FORMAT(shader->info.nr_pos_exports > 1 ? V_02870C_SPI_SHADER_4COMP
                                                                  : V_02870C_SPI_SHADER_NONE) |
      S_02870C_POS2_EXPORT_FORMAT(shader->info.nr_pos_exports > 2 ? V_02870C_SPI_SHADER_4COMP
                                                                  : V_02870C_SPI_SHADER_NONE) |
      S_02870C_POS3_EXPORT_FORMAT(shader->info.nr_pos_exports > 3 ? V_02870C_SPI_SHADER_4COMP
                                                                  : V_02870C_SPI_SHADER_NONE);
   unsigned late_alloc_wave64, cu_mask;
   ac_compute_late_alloc(&sscreen->info, false, false,
                         shader->config.scratch_bytes_per_wave > 0,
                         &late_alloc_wave64, &cu_mask);

   shader->ctx_reg.vs.ge_pc_alloc = S_030980_OVERSUB_EN(late_alloc_wave64 > 0) |
                                    S_030980_NUM_PC_LINES(sscreen->info.pc_lines / 4 - 1);
   shader->pa_cl_vs_out_cntl = si_get_vs_out_cntl(shader->selector, shader, false);

   oc_lds_en = shader->selector->info.stage == MESA_SHADER_TESS_EVAL ? 1 : 0;

   if (sscreen->info.chip_class >= GFX7) {
      si_pm4_set_reg(pm4, R_00B118_SPI_SHADER_PGM_RSRC3_VS,
                     S_00B118_CU_EN(cu_mask) | S_00B118_WAVE_LIMIT(0x3F));
      si_pm4_set_reg(pm4, R_00B11C_SPI_SHADER_LATE_ALLOC_VS, S_00B11C_LIMIT(late_alloc_wave64));
   }

   si_pm4_set_reg(pm4, R_00B120_SPI_SHADER_PGM_LO_VS, va >> 8);
   si_pm4_set_reg(pm4, R_00B124_SPI_SHADER_PGM_HI_VS,
                  S_00B124_MEM_BASE(sscreen->info.address32_hi >> 8));

   uint32_t rsrc1 =
      S_00B128_VGPRS((shader->config.num_vgprs - 1) / (sscreen->ge_wave_size == 32 ? 8 : 4)) |
      S_00B128_VGPR_COMP_CNT(vgpr_comp_cnt) | S_00B128_DX10_CLAMP(1) |
      S_00B128_MEM_ORDERED(si_shader_mem_ordered(shader)) |
      S_00B128_FLOAT_MODE(shader->config.float_mode);
   uint32_t rsrc2 = S_00B12C_USER_SGPR(num_user_sgprs) | S_00B12C_OC_LDS_EN(oc_lds_en) |
                    S_00B12C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0);

   if (sscreen->info.chip_class >= GFX10)
      rsrc2 |= S_00B12C_USER_SGPR_MSB_GFX10(num_user_sgprs >> 5);
   else if (sscreen->info.chip_class == GFX9)
      rsrc2 |= S_00B12C_USER_SGPR_MSB_GFX9(num_user_sgprs >> 5);

   if (sscreen->info.chip_class <= GFX9)
      rsrc1 |= S_00B128_SGPRS((shader->config.num_sgprs - 1) / 8);

   if (!sscreen->use_ngg_streamout) {
      rsrc2 |= S_00B12C_SO_BASE0_EN(!!shader->selector->so.stride[0]) |
               S_00B12C_SO_BASE1_EN(!!shader->selector->so.stride[1]) |
               S_00B12C_SO_BASE2_EN(!!shader->selector->so.stride[2]) |
               S_00B12C_SO_BASE3_EN(!!shader->selector->so.stride[3]) |
               S_00B12C_SO_EN(!!shader->selector->so.num_outputs);
   }

   si_pm4_set_reg(pm4, R_00B128_SPI_SHADER_PGM_RSRC1_VS, rsrc1);
   si_pm4_set_reg(pm4, R_00B12C_SPI_SHADER_PGM_RSRC2_VS, rsrc2);

   if (window_space)
      shader->ctx_reg.vs.pa_cl_vte_cntl = S_028818_VTX_XY_FMT(1) | S_028818_VTX_Z_FMT(1);
   else
      shader->ctx_reg.vs.pa_cl_vte_cntl =
         S_028818_VTX_W0_FMT(1) | S_028818_VPORT_X_SCALE_ENA(1) | S_028818_VPORT_X_OFFSET_ENA(1) |
         S_028818_VPORT_Y_SCALE_ENA(1) | S_028818_VPORT_Y_OFFSET_ENA(1) |
         S_028818_VPORT_Z_SCALE_ENA(1) | S_028818_VPORT_Z_OFFSET_ENA(1);

   if (shader->selector->info.stage == MESA_SHADER_TESS_EVAL)
      si_set_tesseval_regs(sscreen, shader->selector, shader);

   polaris_set_vgt_vertex_reuse(sscreen, shader->selector, shader);
}

static unsigned si_get_ps_num_interp(struct si_shader *ps)
{
   struct si_shader_info *info = &ps->selector->info;
   unsigned num_colors = !!(info->colors_read & 0x0f) + !!(info->colors_read & 0xf0);
   unsigned num_interp =
      ps->selector->info.num_inputs + (ps->key.part.ps.prolog.color_two_side ? num_colors : 0);

   assert(num_interp <= 32);
   return MIN2(num_interp, 32);
}

static unsigned si_get_spi_shader_col_format(struct si_shader *shader)
{
   unsigned spi_shader_col_format = shader->key.part.ps.epilog.spi_shader_col_format;
   unsigned value = 0, num_mrts = 0;
   unsigned i, num_targets = (util_last_bit(spi_shader_col_format) + 3) / 4;

   /* Remove holes in spi_shader_col_format. */
   for (i = 0; i < num_targets; i++) {
      unsigned spi_format = (spi_shader_col_format >> (i * 4)) & 0xf;

      if (spi_format) {
         value |= spi_format << (num_mrts * 4);
         num_mrts++;
      }
   }

   return value;
}

static void si_emit_shader_ps(struct si_context *sctx)
{
   struct si_shader *shader = sctx->queued.named.ps;
   if (!shader)
      return;

   radeon_begin(&sctx->gfx_cs);
   /* R_0286CC_SPI_PS_INPUT_ENA, R_0286D0_SPI_PS_INPUT_ADDR*/
   radeon_opt_set_context_reg2(sctx, R_0286CC_SPI_PS_INPUT_ENA, SI_TRACKED_SPI_PS_INPUT_ENA,
                               shader->ctx_reg.ps.spi_ps_input_ena,
                               shader->ctx_reg.ps.spi_ps_input_addr);

   radeon_opt_set_context_reg(sctx, R_0286E0_SPI_BARYC_CNTL, SI_TRACKED_SPI_BARYC_CNTL,
                              shader->ctx_reg.ps.spi_baryc_cntl);
   radeon_opt_set_context_reg(sctx, R_0286D8_SPI_PS_IN_CONTROL, SI_TRACKED_SPI_PS_IN_CONTROL,
                              shader->ctx_reg.ps.spi_ps_in_control);

   /* R_028710_SPI_SHADER_Z_FORMAT, R_028714_SPI_SHADER_COL_FORMAT */
   radeon_opt_set_context_reg2(sctx, R_028710_SPI_SHADER_Z_FORMAT, SI_TRACKED_SPI_SHADER_Z_FORMAT,
                               shader->ctx_reg.ps.spi_shader_z_format,
                               shader->ctx_reg.ps.spi_shader_col_format);

   radeon_opt_set_context_reg(sctx, R_02823C_CB_SHADER_MASK, SI_TRACKED_CB_SHADER_MASK,
                              shader->ctx_reg.ps.cb_shader_mask);
   radeon_end_update_context_roll(sctx);
}

static void si_shader_ps(struct si_screen *sscreen, struct si_shader *shader)
{
   struct si_shader_info *info = &shader->selector->info;
   struct si_pm4_state *pm4;
   unsigned spi_ps_in_control, spi_shader_col_format, cb_shader_mask;
   unsigned spi_baryc_cntl = S_0286E0_FRONT_FACE_ALL_BITS(1);
   uint64_t va;
   unsigned input_ena = shader->config.spi_ps_input_ena;

   /* we need to enable at least one of them, otherwise we hang the GPU */
   assert(G_0286CC_PERSP_SAMPLE_ENA(input_ena) || G_0286CC_PERSP_CENTER_ENA(input_ena) ||
          G_0286CC_PERSP_CENTROID_ENA(input_ena) || G_0286CC_PERSP_PULL_MODEL_ENA(input_ena) ||
          G_0286CC_LINEAR_SAMPLE_ENA(input_ena) || G_0286CC_LINEAR_CENTER_ENA(input_ena) ||
          G_0286CC_LINEAR_CENTROID_ENA(input_ena) || G_0286CC_LINE_STIPPLE_TEX_ENA(input_ena));
   /* POS_W_FLOAT_ENA requires one of the perspective weights. */
   assert(!G_0286CC_POS_W_FLOAT_ENA(input_ena) || G_0286CC_PERSP_SAMPLE_ENA(input_ena) ||
          G_0286CC_PERSP_CENTER_ENA(input_ena) || G_0286CC_PERSP_CENTROID_ENA(input_ena) ||
          G_0286CC_PERSP_PULL_MODEL_ENA(input_ena));

   /* Validate interpolation optimization flags (read as implications). */
   assert(!shader->key.part.ps.prolog.bc_optimize_for_persp ||
          (G_0286CC_PERSP_CENTER_ENA(input_ena) && G_0286CC_PERSP_CENTROID_ENA(input_ena)));
   assert(!shader->key.part.ps.prolog.bc_optimize_for_linear ||
          (G_0286CC_LINEAR_CENTER_ENA(input_ena) && G_0286CC_LINEAR_CENTROID_ENA(input_ena)));
   assert(!shader->key.part.ps.prolog.force_persp_center_interp ||
          (!G_0286CC_PERSP_SAMPLE_ENA(input_ena) && !G_0286CC_PERSP_CENTROID_ENA(input_ena)));
   assert(!shader->key.part.ps.prolog.force_linear_center_interp ||
          (!G_0286CC_LINEAR_SAMPLE_ENA(input_ena) && !G_0286CC_LINEAR_CENTROID_ENA(input_ena)));
   assert(!shader->key.part.ps.prolog.force_persp_sample_interp ||
          (!G_0286CC_PERSP_CENTER_ENA(input_ena) && !G_0286CC_PERSP_CENTROID_ENA(input_ena)));
   assert(!shader->key.part.ps.prolog.force_linear_sample_interp ||
          (!G_0286CC_LINEAR_CENTER_ENA(input_ena) && !G_0286CC_LINEAR_CENTROID_ENA(input_ena)));

   /* Validate cases when the optimizations are off (read as implications). */
   assert(shader->key.part.ps.prolog.bc_optimize_for_persp ||
          !G_0286CC_PERSP_CENTER_ENA(input_ena) || !G_0286CC_PERSP_CENTROID_ENA(input_ena));
   assert(shader->key.part.ps.prolog.bc_optimize_for_linear ||
          !G_0286CC_LINEAR_CENTER_ENA(input_ena) || !G_0286CC_LINEAR_CENTROID_ENA(input_ena));

   pm4 = si_get_shader_pm4_state(shader);
   if (!pm4)
      return;

   /* If multiple state sets are allowed to be in a bin, break the batch on a new PS. */
   if (sscreen->dpbb_allowed &&
       (sscreen->pbb_context_states_per_bin > 1 ||
        sscreen->pbb_persistent_states_per_bin > 1)) {
      si_pm4_cmd_add(pm4, PKT3(PKT3_EVENT_WRITE, 0, 0));
      si_pm4_cmd_add(pm4, EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
   }

   pm4->atom.emit = si_emit_shader_ps;

   /* SPI_BARYC_CNTL.POS_FLOAT_LOCATION
    * Possible vaules:
    * 0 -> Position = pixel center
    * 1 -> Position = pixel centroid
    * 2 -> Position = at sample position
    *
    * From GLSL 4.5 specification, section 7.1:
    *   "The variable gl_FragCoord is available as an input variable from
    *    within fragment shaders and it holds the window relative coordinates
    *    (x, y, z, 1/w) values for the fragment. If multi-sampling, this
    *    value can be for any location within the pixel, or one of the
    *    fragment samples. The use of centroid does not further restrict
    *    this value to be inside the current primitive."
    *
    * Meaning that centroid has no effect and we can return anything within
    * the pixel. Thus, return the value at sample position, because that's
    * the most accurate one shaders can get.
    */
   spi_baryc_cntl |= S_0286E0_POS_FLOAT_LOCATION(2);

   if (info->base.fs.pixel_center_integer)
      spi_baryc_cntl |= S_0286E0_POS_FLOAT_ULC(1);

   spi_shader_col_format = si_get_spi_shader_col_format(shader);
   cb_shader_mask = ac_get_cb_shader_mask(shader->key.part.ps.epilog.spi_shader_col_format);

   /* Ensure that some export memory is always allocated, for two reasons:
    *
    * 1) Correctness: The hardware ignores the EXEC mask if no export
    *    memory is allocated, so KILL and alpha test do not work correctly
    *    without this.
    * 2) Performance: Every shader needs at least a NULL export, even when
    *    it writes no color/depth output. The NULL export instruction
    *    stalls without this setting.
    *
    * Don't add this to CB_SHADER_MASK.
    *
    * GFX10 supports pixel shaders without exports by setting both
    * the color and Z formats to SPI_SHADER_ZERO. The hw will skip export
    * instructions if any are present.
    */
   if ((sscreen->info.chip_class <= GFX9 || info->base.fs.uses_discard ||
        shader->key.part.ps.epilog.alpha_func != PIPE_FUNC_ALWAYS) &&
       !spi_shader_col_format && !info->writes_z && !info->writes_stencil &&
       !info->writes_samplemask)
      spi_shader_col_format = V_028714_SPI_SHADER_32_R;

   shader->ctx_reg.ps.spi_ps_input_ena = input_ena;
   shader->ctx_reg.ps.spi_ps_input_addr = shader->config.spi_ps_input_addr;

   unsigned num_interp = si_get_ps_num_interp(shader);

   /* Set interpolation controls. */
   spi_ps_in_control = S_0286D8_NUM_INTERP(num_interp) |
                       S_0286D8_PS_W32_EN(sscreen->ps_wave_size == 32);

   shader->ctx_reg.ps.num_interp = num_interp;
   shader->ctx_reg.ps.spi_baryc_cntl = spi_baryc_cntl;
   shader->ctx_reg.ps.spi_ps_in_control = spi_ps_in_control;
   shader->ctx_reg.ps.spi_shader_z_format =
      ac_get_spi_shader_z_format(info->writes_z, info->writes_stencil, info->writes_samplemask);
   shader->ctx_reg.ps.spi_shader_col_format = spi_shader_col_format;
   shader->ctx_reg.ps.cb_shader_mask = cb_shader_mask;

   va = shader->bo->gpu_address;
   si_pm4_set_reg(pm4, R_00B020_SPI_SHADER_PGM_LO_PS, va >> 8);
   si_pm4_set_reg(pm4, R_00B024_SPI_SHADER_PGM_HI_PS,
                  S_00B024_MEM_BASE(sscreen->info.address32_hi >> 8));

   uint32_t rsrc1 =
      S_00B028_VGPRS((shader->config.num_vgprs - 1) / (sscreen->ps_wave_size == 32 ? 8 : 4)) |
      S_00B028_DX10_CLAMP(1) | S_00B028_MEM_ORDERED(si_shader_mem_ordered(shader)) |
      S_00B028_FLOAT_MODE(shader->config.float_mode);

   if (sscreen->info.chip_class < GFX10) {
      rsrc1 |= S_00B028_SGPRS((shader->config.num_sgprs - 1) / 8);
   }

   si_pm4_set_reg(pm4, R_00B028_SPI_SHADER_PGM_RSRC1_PS, rsrc1);
   si_pm4_set_reg(pm4, R_00B02C_SPI_SHADER_PGM_RSRC2_PS,
                  S_00B02C_EXTRA_LDS_SIZE(shader->config.lds_size) |
                     S_00B02C_USER_SGPR(SI_PS_NUM_USER_SGPR) |
                     S_00B32C_SCRATCH_EN(shader->config.scratch_bytes_per_wave > 0));
}

static void si_shader_init_pm4_state(struct si_screen *sscreen, struct si_shader *shader)
{
   switch (shader->selector->info.stage) {
   case MESA_SHADER_VERTEX:
      if (shader->key.as_ls)
         si_shader_ls(sscreen, shader);
      else if (shader->key.as_es)
         si_shader_es(sscreen, shader);
      else if (shader->key.as_ngg)
         gfx10_shader_ngg(sscreen, shader);
      else
         si_shader_vs(sscreen, shader, NULL);
      break;
   case MESA_SHADER_TESS_CTRL:
      si_shader_hs(sscreen, shader);
      break;
   case MESA_SHADER_TESS_EVAL:
      if (shader->key.as_es)
         si_shader_es(sscreen, shader);
      else if (shader->key.as_ngg)
         gfx10_shader_ngg(sscreen, shader);
      else
         si_shader_vs(sscreen, shader, NULL);
      break;
   case MESA_SHADER_GEOMETRY:
      if (shader->key.as_ngg)
         gfx10_shader_ngg(sscreen, shader);
      else
         si_shader_gs(sscreen, shader);
      break;
   case MESA_SHADER_FRAGMENT:
      si_shader_ps(sscreen, shader);
      break;
   default:
      assert(0);
   }
}

static void si_clear_vs_key_inputs(struct si_context *sctx, struct si_shader_key *key,
                                   struct si_vs_prolog_bits *prolog_key)
{
   prolog_key->instance_divisor_is_one = 0;
   prolog_key->instance_divisor_is_fetched = 0;
   key->mono.vs_fetch_opencode = 0;
   memset(key->mono.vs_fix_fetch, 0, sizeof(key->mono.vs_fix_fetch));
}

void si_vs_key_update_inputs(struct si_context *sctx)
{
   struct si_shader_selector *vs = sctx->shader.vs.cso;
   struct si_vertex_elements *elts = sctx->vertex_elements;
   struct si_shader_key *key = &sctx->shader.vs.key;

   if (!vs)
      return;

   if (vs->info.base.vs.blit_sgprs_amd) {
      si_clear_vs_key_inputs(sctx, key, &key->part.vs.prolog);
      key->opt.prefer_mono = 0;
      sctx->uses_nontrivial_vs_prolog = false;
      return;
   }

   bool uses_nontrivial_vs_prolog = false;

   if (elts->instance_divisor_is_one || elts->instance_divisor_is_fetched)
      uses_nontrivial_vs_prolog = true;

   key->part.vs.prolog.instance_divisor_is_one = elts->instance_divisor_is_one;
   key->part.vs.prolog.instance_divisor_is_fetched = elts->instance_divisor_is_fetched;
   key->opt.prefer_mono = elts->instance_divisor_is_fetched;

   unsigned count_mask = (1 << vs->info.num_inputs) - 1;
   unsigned fix = elts->fix_fetch_always & count_mask;
   unsigned opencode = elts->fix_fetch_opencode & count_mask;

   if (sctx->vertex_buffer_unaligned & elts->vb_alignment_check_mask) {
      uint32_t mask = elts->fix_fetch_unaligned & count_mask;
      while (mask) {
         unsigned i = u_bit_scan(&mask);
         unsigned log_hw_load_size = 1 + ((elts->hw_load_is_dword >> i) & 1);
         unsigned vbidx = elts->vertex_buffer_index[i];
         struct pipe_vertex_buffer *vb = &sctx->vertex_buffer[vbidx];
         unsigned align_mask = (1 << log_hw_load_size) - 1;
         if (vb->buffer_offset & align_mask || vb->stride & align_mask) {
            fix |= 1 << i;
            opencode |= 1 << i;
         }
      }
   }

   memset(key->mono.vs_fix_fetch, 0, sizeof(key->mono.vs_fix_fetch));

   while (fix) {
      unsigned i = u_bit_scan(&fix);
      uint8_t fix_fetch = elts->fix_fetch[i];

      key->mono.vs_fix_fetch[i].bits = fix_fetch;
      if (fix_fetch)
         uses_nontrivial_vs_prolog = true;
   }
   key->mono.vs_fetch_opencode = opencode;
   if (opencode)
      uses_nontrivial_vs_prolog = true;

   sctx->uses_nontrivial_vs_prolog = uses_nontrivial_vs_prolog;

   /* draw_vertex_state (display lists) requires a trivial VS prolog that ignores
    * the current vertex buffers and vertex elements.
    *
    * We just computed the prolog key because we needed to set uses_nontrivial_vs_prolog,
    * so that we know whether the VS prolog should be updated when we switch from
    * draw_vertex_state to draw_vbo. Now clear the VS prolog for draw_vertex_state.
    * This should happen rarely because the VS prolog should be trivial in most
    * cases.
    */
   if (uses_nontrivial_vs_prolog && sctx->force_trivial_vs_prolog)
      si_clear_vs_key_inputs(sctx, key, &key->part.vs.prolog);
}

void si_get_vs_key_inputs(struct si_context *sctx, struct si_shader_key *key,
                          struct si_vs_prolog_bits *prolog_key)
{
   prolog_key->instance_divisor_is_one = sctx->shader.vs.key.part.vs.prolog.instance_divisor_is_one;
   prolog_key->instance_divisor_is_fetched = sctx->shader.vs.key.part.vs.prolog.instance_divisor_is_fetched;

   key->mono.vs_fetch_opencode = sctx->shader.vs.key.mono.vs_fetch_opencode;
   memcpy(key->mono.vs_fix_fetch, sctx->shader.vs.key.mono.vs_fix_fetch,
          sizeof(key->mono.vs_fix_fetch));
}

void si_update_ps_inputs_read_or_disabled(struct si_context *sctx)
{
   struct si_shader_selector *ps = sctx->shader.ps.cso;

   /* Find out if PS is disabled. */
   bool ps_disabled = true;
   if (ps) {
      bool ps_modifies_zs = ps->info.base.fs.uses_discard || ps->info.writes_z || ps->info.writes_stencil ||
                            ps->info.writes_samplemask ||
                            sctx->queued.named.blend->alpha_to_coverage ||
                            sctx->queued.named.dsa->alpha_func != PIPE_FUNC_ALWAYS;
      unsigned ps_colormask = si_get_total_colormask(sctx);

      ps_disabled = sctx->queued.named.rasterizer->rasterizer_discard ||
                    (!ps_colormask && !ps_modifies_zs && !ps->info.base.writes_memory);
   }

   sctx->ps_inputs_read_or_disabled = ps_disabled ? 0 : ps->inputs_read;
}

static void si_get_vs_key_outputs(struct si_context *sctx, struct si_shader_selector *vs,
                                  struct si_shader_key *key)
{

   key->opt.kill_clip_distances = vs->clipdist_mask & ~sctx->queued.named.rasterizer->clip_plane_enable;

   /* Find out which VS outputs aren't used by the PS. */
   uint64_t outputs_written = vs->outputs_written_before_ps;
   uint64_t linked = outputs_written & sctx->ps_inputs_read_or_disabled;

   key->opt.kill_outputs = ~linked & outputs_written;

   if (vs->info.stage != MESA_SHADER_GEOMETRY) {
      key->opt.ngg_culling = sctx->ngg_culling;
      key->mono.u.vs_export_prim_id = sctx->shader.ps.cso && sctx->shader.ps.cso->info.uses_primid;
   } else {
      key->opt.ngg_culling = 0;
      key->mono.u.vs_export_prim_id = 0;
   }

   key->opt.kill_pointsize = vs->info.writes_psize &&
                             sctx->current_rast_prim != PIPE_PRIM_POINTS &&
                             !sctx->queued.named.rasterizer->polygon_mode_is_points;
}

static void si_clear_vs_key_outputs(struct si_context *sctx, struct si_shader_selector *vs,
                                    struct si_shader_key *key)
{
   key->opt.kill_clip_distances = 0;
   key->opt.kill_outputs = 0;
   key->opt.ngg_culling = 0;
   key->mono.u.vs_export_prim_id = 0;
   key->opt.kill_pointsize = 0;
}

void si_ps_key_update_framebuffer(struct si_context *sctx)
{
   struct si_shader_selector *sel = sctx->shader.ps.cso;
   struct si_shader_key *key = &sctx->shader.ps.key;

   if (!sel)
      return;

   if (sel->info.color0_writes_all_cbufs &&
       sel->info.colors_written == 0x1)
      key->part.ps.epilog.last_cbuf = MAX2(sctx->framebuffer.state.nr_cbufs, 1) - 1;
   else
      key->part.ps.epilog.last_cbuf = 0;

   /* ps_uses_fbfetch is true only if the color buffer is bound. */
   if (sctx->ps_uses_fbfetch) {
      struct pipe_surface *cb0 = sctx->framebuffer.state.cbufs[0];
      struct pipe_resource *tex = cb0->texture;

      /* 1D textures are allocated and used as 2D on GFX9. */
      key->mono.u.ps.fbfetch_msaa = sctx->framebuffer.nr_samples > 1;
      key->mono.u.ps.fbfetch_is_1D =
         sctx->chip_class != GFX9 &&
         (tex->target == PIPE_TEXTURE_1D || tex->target == PIPE_TEXTURE_1D_ARRAY);
      key->mono.u.ps.fbfetch_layered =
         tex->target == PIPE_TEXTURE_1D_ARRAY || tex->target == PIPE_TEXTURE_2D_ARRAY ||
         tex->target == PIPE_TEXTURE_CUBE || tex->target == PIPE_TEXTURE_CUBE_ARRAY ||
         tex->target == PIPE_TEXTURE_3D;
   } else {
      key->mono.u.ps.fbfetch_msaa = 0;
      key->mono.u.ps.fbfetch_is_1D = 0;
      key->mono.u.ps.fbfetch_layered = 0;
   }
}

void si_ps_key_update_framebuffer_blend(struct si_context *sctx)
{
   struct si_shader_selector *sel = sctx->shader.ps.cso;
   struct si_shader_key *key = &sctx->shader.ps.key;
   struct si_state_blend *blend = sctx->queued.named.blend;

   if (!sel)
      return;

   /* Select the shader color format based on whether
    * blending or alpha are needed.
    */
   key->part.ps.epilog.spi_shader_col_format =
      (blend->blend_enable_4bit & blend->need_src_alpha_4bit &
       sctx->framebuffer.spi_shader_col_format_blend_alpha) |
      (blend->blend_enable_4bit & ~blend->need_src_alpha_4bit &
       sctx->framebuffer.spi_shader_col_format_blend) |
      (~blend->blend_enable_4bit & blend->need_src_alpha_4bit &
       sctx->framebuffer.spi_shader_col_format_alpha) |
      (~blend->blend_enable_4bit & ~blend->need_src_alpha_4bit &
       sctx->framebuffer.spi_shader_col_format);
   key->part.ps.epilog.spi_shader_col_format &= blend->cb_target_enabled_4bit;

   /* The output for dual source blending should have
    * the same format as the first output.
    */
   if (blend->dual_src_blend) {
      key->part.ps.epilog.spi_shader_col_format |=
         (key->part.ps.epilog.spi_shader_col_format & 0xf) << 4;
   }

   /* If alpha-to-coverage is enabled, we have to export alpha
    * even if there is no color buffer.
    */
   if (!(key->part.ps.epilog.spi_shader_col_format & 0xf) && blend->alpha_to_coverage)
      key->part.ps.epilog.spi_shader_col_format |= V_028710_SPI_SHADER_32_AR;

   /* On GFX6 and GFX7 except Hawaii, the CB doesn't clamp outputs
    * to the range supported by the type if a channel has less
    * than 16 bits and the export format is 16_ABGR.
    */
   if (sctx->chip_class <= GFX7 && sctx->family != CHIP_HAWAII) {
      key->part.ps.epilog.color_is_int8 = sctx->framebuffer.color_is_int8;
      key->part.ps.epilog.color_is_int10 = sctx->framebuffer.color_is_int10;
   }

   /* Disable unwritten outputs (if WRITE_ALL_CBUFS isn't enabled). */
   if (!key->part.ps.epilog.last_cbuf) {
      key->part.ps.epilog.spi_shader_col_format &= sel->colors_written_4bit;
      key->part.ps.epilog.color_is_int8 &= sel->info.colors_written;
      key->part.ps.epilog.color_is_int10 &= sel->info.colors_written;
   }

   /* Eliminate shader code computing output values that are unused.
    * This enables dead code elimination between shader parts.
    * Check if any output is eliminated.
    */
   if (sel->colors_written_4bit &
       ~(sctx->framebuffer.colorbuf_enabled_4bit & blend->cb_target_enabled_4bit))
      key->opt.prefer_mono = 1;
   else
      key->opt.prefer_mono = 0;
}

void si_ps_key_update_blend_rasterizer(struct si_context *sctx)
{
   struct si_shader_key *key = &sctx->shader.ps.key;
   struct si_state_blend *blend = sctx->queued.named.blend;
   struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

   key->part.ps.epilog.alpha_to_one = blend->alpha_to_one && rs->multisample_enable;
}

void si_ps_key_update_rasterizer(struct si_context *sctx)
{
   struct si_shader_selector *sel = sctx->shader.ps.cso;
   struct si_shader_key *key = &sctx->shader.ps.key;
   struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

   if (!sel)
      return;

   key->part.ps.prolog.color_two_side = rs->two_side && sel->info.colors_read;
   key->part.ps.prolog.flatshade_colors = rs->flatshade && sel->info.uses_interp_color;
   key->part.ps.epilog.clamp_color = rs->clamp_fragment_color;
}

void si_ps_key_update_dsa(struct si_context *sctx)
{
   struct si_shader_key *key = &sctx->shader.ps.key;

   key->part.ps.epilog.alpha_func = sctx->queued.named.dsa->alpha_func;
}

static void si_ps_key_update_primtype_shader_rasterizer_framebuffer(struct si_context *sctx)
{
   struct si_shader_key *key = &sctx->shader.ps.key;
   struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

   bool is_poly = !util_prim_is_points_or_lines(sctx->current_rast_prim);
   bool is_line = util_prim_is_lines(sctx->current_rast_prim);

   key->part.ps.prolog.poly_stipple = rs->poly_stipple_enable && is_poly;
   key->part.ps.epilog.poly_line_smoothing =
      ((is_poly && rs->poly_smooth) || (is_line && rs->line_smooth)) &&
      sctx->framebuffer.nr_samples <= 1;
}

void si_ps_key_update_sample_shading(struct si_context *sctx)
{
   struct si_shader_selector *sel = sctx->shader.ps.cso;
   struct si_shader_key *key = &sctx->shader.ps.key;

   if (!sel)
      return;

   if (sctx->ps_iter_samples > 1 && sel->info.reads_samplemask)
      key->part.ps.prolog.samplemask_log_ps_iter = util_logbase2(sctx->ps_iter_samples);
   else
      key->part.ps.prolog.samplemask_log_ps_iter = 0;
}

void si_ps_key_update_framebuffer_rasterizer_sample_shading(struct si_context *sctx)
{
   struct si_shader_selector *sel = sctx->shader.ps.cso;
   struct si_shader_key *key = &sctx->shader.ps.key;
   struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

   if (!sel)
      return;

   bool uses_persp_center = sel->info.uses_persp_center ||
                            (!rs->flatshade && sel->info.uses_persp_center_color);
   bool uses_persp_centroid = sel->info.uses_persp_centroid ||
                              (!rs->flatshade && sel->info.uses_persp_centroid_color);
   bool uses_persp_sample = sel->info.uses_persp_sample ||
                            (!rs->flatshade && sel->info.uses_persp_sample_color);

   if (rs->force_persample_interp && rs->multisample_enable &&
       sctx->framebuffer.nr_samples > 1 && sctx->ps_iter_samples > 1) {
      key->part.ps.prolog.force_persp_sample_interp =
         uses_persp_center || uses_persp_centroid;

      key->part.ps.prolog.force_linear_sample_interp =
         sel->info.uses_linear_center || sel->info.uses_linear_centroid;

      key->part.ps.prolog.force_persp_center_interp = 0;
      key->part.ps.prolog.force_linear_center_interp = 0;
      key->part.ps.prolog.bc_optimize_for_persp = 0;
      key->part.ps.prolog.bc_optimize_for_linear = 0;
      key->mono.u.ps.interpolate_at_sample_force_center = 0;
   } else if (rs->multisample_enable && sctx->framebuffer.nr_samples > 1) {
      key->part.ps.prolog.force_persp_sample_interp = 0;
      key->part.ps.prolog.force_linear_sample_interp = 0;
      key->part.ps.prolog.force_persp_center_interp = 0;
      key->part.ps.prolog.force_linear_center_interp = 0;
      key->part.ps.prolog.bc_optimize_for_persp =
         uses_persp_center && uses_persp_centroid;
      key->part.ps.prolog.bc_optimize_for_linear =
         sel->info.uses_linear_center && sel->info.uses_linear_centroid;
      key->mono.u.ps.interpolate_at_sample_force_center = 0;
   } else {
      key->part.ps.prolog.force_persp_sample_interp = 0;
      key->part.ps.prolog.force_linear_sample_interp = 0;

      /* Make sure SPI doesn't compute more than 1 pair
       * of (i,j), which is the optimization here. */
      key->part.ps.prolog.force_persp_center_interp = uses_persp_center +
                                                      uses_persp_centroid +
                                                      uses_persp_sample > 1;

      key->part.ps.prolog.force_linear_center_interp = sel->info.uses_linear_center +
                                                       sel->info.uses_linear_centroid +
                                                       sel->info.uses_linear_sample > 1;
      key->part.ps.prolog.bc_optimize_for_persp = 0;
      key->part.ps.prolog.bc_optimize_for_linear = 0;
      key->mono.u.ps.interpolate_at_sample_force_center = sel->info.uses_interp_at_sample;
   }
}

/* Compute the key for the hw shader variant */
static inline void si_shader_selector_key(struct pipe_context *ctx, struct si_shader_selector *sel,
                                          struct si_shader_key *key)
{
   struct si_context *sctx = (struct si_context *)ctx;

   switch (sel->info.stage) {
   case MESA_SHADER_VERTEX:
      if (!sctx->shader.tes.cso && !sctx->shader.gs.cso)
         si_get_vs_key_outputs(sctx, sel, key);
      else
         si_clear_vs_key_outputs(sctx, sel, key);
      break;
   case MESA_SHADER_TESS_CTRL:
      if (sctx->chip_class >= GFX9) {
         si_get_vs_key_inputs(sctx, key, &key->part.tcs.ls_prolog);
         key->part.tcs.ls = sctx->shader.vs.cso;
      }
      break;
   case MESA_SHADER_TESS_EVAL:
      if (!sctx->shader.gs.cso)
         si_get_vs_key_outputs(sctx, sel, key);
      else
         si_clear_vs_key_outputs(sctx, sel, key);
      break;
   case MESA_SHADER_GEOMETRY:
      if (sctx->chip_class >= GFX9) {
         if (sctx->shader.tes.cso) {
            si_clear_vs_key_inputs(sctx, key, &key->part.gs.vs_prolog);
            key->part.gs.es = sctx->shader.tes.cso;
         } else {
            si_get_vs_key_inputs(sctx, key, &key->part.gs.vs_prolog);
            key->part.gs.es = sctx->shader.vs.cso;
         }

         /* Only NGG can eliminate GS outputs, because the code is shared with VS. */
         if (sctx->ngg)
            si_get_vs_key_outputs(sctx, sel, key);
         else
            si_clear_vs_key_outputs(sctx, sel, key);
      }
      break;
   case MESA_SHADER_FRAGMENT:
      si_ps_key_update_primtype_shader_rasterizer_framebuffer(sctx);
      break;
   default:
      assert(0);
   }
}

static void si_build_shader_variant(struct si_shader *shader, int thread_index, bool low_priority)
{
   struct si_shader_selector *sel = shader->selector;
   struct si_screen *sscreen = sel->screen;
   struct ac_llvm_compiler *compiler;
   struct pipe_debug_callback *debug = &shader->compiler_ctx_state.debug;

   if (thread_index >= 0) {
      if (low_priority) {
         assert(thread_index < ARRAY_SIZE(sscreen->compiler_lowp));
         compiler = &sscreen->compiler_lowp[thread_index];
      } else {
         assert(thread_index < ARRAY_SIZE(sscreen->compiler));
         compiler = &sscreen->compiler[thread_index];
      }
      if (!debug->async)
         debug = NULL;
   } else {
      assert(!low_priority);
      compiler = shader->compiler_ctx_state.compiler;
   }

   if (!compiler->passes)
      si_init_compiler(sscreen, compiler);

   if (unlikely(!si_create_shader_variant(sscreen, compiler, shader, debug))) {
      PRINT_ERR("Failed to build shader variant (type=%u)\n", sel->info.stage);
      shader->compilation_failed = true;
      return;
   }

   if (shader->compiler_ctx_state.is_debug_context) {
      FILE *f = open_memstream(&shader->shader_log, &shader->shader_log_size);
      if (f) {
         si_shader_dump(sscreen, shader, NULL, f, false);
         fclose(f);
      }
   }

   si_shader_init_pm4_state(sscreen, shader);
}

static void si_build_shader_variant_low_priority(void *job, void *gdata, int thread_index)
{
   struct si_shader *shader = (struct si_shader *)job;

   assert(thread_index >= 0);

   si_build_shader_variant(shader, thread_index, true);
}

static const struct si_shader_key zeroed;

static bool si_check_missing_main_part(struct si_screen *sscreen, struct si_shader_selector *sel,
                                       struct si_compiler_ctx_state *compiler_state,
                                       const struct si_shader_key *key)
{
   struct si_shader **mainp = si_get_main_shader_part(sel, key);

   if (!*mainp) {
      struct si_shader *main_part = CALLOC_STRUCT(si_shader);

      if (!main_part)
         return false;

      /* We can leave the fence as permanently signaled because the
       * main part becomes visible globally only after it has been
       * compiled. */
      util_queue_fence_init(&main_part->ready);

      main_part->selector = sel;
      main_part->key.as_es = key->as_es;
      main_part->key.as_ls = key->as_ls;
      main_part->key.as_ngg = key->as_ngg;
      main_part->is_monolithic = false;

      if (!si_compile_shader(sscreen, compiler_state->compiler, main_part,
                             &compiler_state->debug)) {
         FREE(main_part);
         return false;
      }
      *mainp = main_part;
   }
   return true;
}

/* A helper to copy *key to *local_key and return local_key. */
static const struct si_shader_key *
use_local_key_copy(const struct si_shader_key *key, struct si_shader_key *local_key)
{
   if (key != local_key)
      memcpy(local_key, key, sizeof(*key));

   return local_key;
}

/**
 * Select a shader variant according to the shader key.
 *
 * \param optimized_or_none  If the key describes an optimized shader variant and
 *                           the compilation isn't finished, don't select any
 *                           shader and return an error.
 */
int si_shader_select_with_key(struct si_context *sctx, struct si_shader_ctx_state *state,
                              const struct si_shader_key *key, int thread_index,
                              bool optimized_or_none)
{
   struct si_screen *sscreen = sctx->screen;
   struct si_shader_selector *sel = state->cso;
   struct si_shader_selector *previous_stage_sel = NULL;
   struct si_shader *current = state->current;
   struct si_shader *iter, *shader = NULL;
   /* si_shader_select_with_key must not modify 'key' because it would affect future shaders.
    * If we need to modify it for this specific shader (eg: to disable optimizations), we
    * use a copy.
    */
   struct si_shader_key local_key;

   if (unlikely(sscreen->debug_flags & DBG(NO_OPT_VARIANT))) {
      /* Disable shader variant optimizations. */
      key = use_local_key_copy(key, &local_key);
      memset(&local_key.opt, 0, sizeof(key->opt));
   }

again:
   /* Check if we don't need to change anything.
    * This path is also used for most shaders that don't need multiple
    * variants, it will cost just a computation of the key and this
    * test. */
   if (likely(current && memcmp(&current->key, key, sizeof(*key)) == 0)) {
      if (unlikely(!util_queue_fence_is_signalled(&current->ready))) {
         if (current->is_optimized) {
            if (optimized_or_none)
               return -1;

            key = use_local_key_copy(key, &local_key);
            memset(&local_key.opt, 0, sizeof(key->opt));
            goto current_not_ready;
         }

         util_queue_fence_wait(&current->ready);
      }

      return current->compilation_failed ? -1 : 0;
   }
current_not_ready:

   /* This must be done before the mutex is locked, because async GS
    * compilation calls this function too, and therefore must enter
    * the mutex first.
    *
    * Only wait if we are in a draw call. Don't wait if we are
    * in a compiler thread.
    */
   if (thread_index < 0)
      util_queue_fence_wait(&sel->ready);

   simple_mtx_lock(&sel->mutex);

   /* Compute the size of the key without the uniform values. */
   size_t s = (void*)&key->opt.inlined_uniform_values - (void*)key;
   int variant_count = 0;
   const int max_inline_uniforms_variants = 5;

   /* Find the shader variant. */
   for (iter = sel->first_variant; iter; iter = iter->next_variant) {
      if (memcmp(&iter->key, key, s) == 0) {
         /* Check the inlined uniform values separatly, and count
          * the number of variants based on them.
          */
         if (key->opt.inline_uniforms &&
             memcmp(iter->key.opt.inlined_uniform_values,
                    key->opt.inlined_uniform_values,
                    MAX_INLINABLE_UNIFORMS * 4) != 0) {
            if (variant_count++ > max_inline_uniforms_variants) {
               key = use_local_key_copy(key, &local_key);
               /* Too many variants. Disable inlining for this shader. */
               local_key.opt.inline_uniforms = 0;
               memset(local_key.opt.inlined_uniform_values, 0, MAX_INLINABLE_UNIFORMS * 4);
               simple_mtx_unlock(&sel->mutex);
               goto again;
            }
            continue;
         }

         simple_mtx_unlock(&sel->mutex);

         if (unlikely(!util_queue_fence_is_signalled(&iter->ready))) {
            /* If it's an optimized shader and its compilation has
             * been started but isn't done, use the unoptimized
             * shader so as not to cause a stall due to compilation.
             */
            if (iter->is_optimized) {
               if (optimized_or_none)
                  return -1;

               key = use_local_key_copy(key, &local_key);
               memset(&local_key.opt, 0, sizeof(key->opt));
               goto again;
            }

            util_queue_fence_wait(&iter->ready);
         }

         if (iter->compilation_failed) {
            return -1; /* skip the draw call */
         }

         state->current = iter;
         return 0;
      }
   }

   /* Build a new shader. */
   shader = CALLOC_STRUCT(si_shader);
   if (!shader) {
      simple_mtx_unlock(&sel->mutex);
      return -ENOMEM;
   }

   util_queue_fence_init(&shader->ready);

   if (!sctx->compiler.passes)
      si_init_compiler(sctx->screen, &sctx->compiler);

   shader->selector = sel;
   shader->key = *key;
   shader->compiler_ctx_state.compiler = &sctx->compiler;
   shader->compiler_ctx_state.debug = sctx->debug;
   shader->compiler_ctx_state.is_debug_context = sctx->is_debug;

   /* If this is a merged shader, get the first shader's selector. */
   if (sscreen->info.chip_class >= GFX9) {
      if (sel->info.stage == MESA_SHADER_TESS_CTRL)
         previous_stage_sel = key->part.tcs.ls;
      else if (sel->info.stage == MESA_SHADER_GEOMETRY)
         previous_stage_sel = key->part.gs.es;

      /* We need to wait for the previous shader. */
      if (previous_stage_sel && thread_index < 0)
         util_queue_fence_wait(&previous_stage_sel->ready);
   }

   bool is_pure_monolithic =
      sscreen->use_monolithic_shaders || memcmp(&key->mono, &zeroed.mono, sizeof(key->mono)) != 0;

   /* Compile the main shader part if it doesn't exist. This can happen
    * if the initial guess was wrong.
    */
   if (!is_pure_monolithic) {
      bool ok = true;

      /* Make sure the main shader part is present. This is needed
       * for shaders that can be compiled as VS, LS, or ES, and only
       * one of them is compiled at creation.
       *
       * It is also needed for GS, which can be compiled as non-NGG
       * and NGG.
       *
       * For merged shaders, check that the starting shader's main
       * part is present.
       */
      if (previous_stage_sel) {
         struct si_shader_key shader1_key = zeroed;

         if (sel->info.stage == MESA_SHADER_TESS_CTRL) {
            shader1_key.as_ls = 1;
         } else if (sel->info.stage == MESA_SHADER_GEOMETRY) {
            shader1_key.as_es = 1;
            shader1_key.as_ngg = key->as_ngg; /* for Wave32 vs Wave64 */
         } else {
            assert(0);
         }

         simple_mtx_lock(&previous_stage_sel->mutex);
         ok = si_check_missing_main_part(sscreen, previous_stage_sel, &shader->compiler_ctx_state,
                                         &shader1_key);
         simple_mtx_unlock(&previous_stage_sel->mutex);
      }

      if (ok) {
         ok = si_check_missing_main_part(sscreen, sel, &shader->compiler_ctx_state, key);
      }

      if (!ok) {
         FREE(shader);
         simple_mtx_unlock(&sel->mutex);
         return -ENOMEM; /* skip the draw call */
      }
   }

   /* Keep the reference to the 1st shader of merged shaders, so that
    * Gallium can't destroy it before we destroy the 2nd shader.
    *
    * Set sctx = NULL, because it's unused if we're not releasing
    * the shader, and we don't have any sctx here.
    */
   si_shader_selector_reference(NULL, &shader->previous_stage_sel, previous_stage_sel);

   /* Monolithic-only shaders don't make a distinction between optimized
    * and unoptimized. */
   shader->is_monolithic =
      is_pure_monolithic || memcmp(&key->opt, &zeroed.opt, sizeof(key->opt)) != 0;

   shader->is_optimized = !is_pure_monolithic &&
                          memcmp(&key->opt, &zeroed.opt, sizeof(key->opt)) != 0;

   /* If it's an optimized shader, compile it asynchronously. */
   if (shader->is_optimized && thread_index < 0) {
      /* Compile it asynchronously. */
      util_queue_add_job(&sscreen->shader_compiler_queue_low_priority, shader, &shader->ready,
                         si_build_shader_variant_low_priority, NULL, 0);

      /* Add only after the ready fence was reset, to guard against a
       * race with si_bind_XX_shader. */
      if (!sel->last_variant) {
         sel->first_variant = shader;
         sel->last_variant = shader;
      } else {
         sel->last_variant->next_variant = shader;
         sel->last_variant = shader;
      }

      /* Use the default (unoptimized) shader for now. */
      key = use_local_key_copy(key, &local_key);
      memset(&local_key.opt, 0, sizeof(key->opt));
      simple_mtx_unlock(&sel->mutex);

      if (sscreen->options.sync_compile)
         util_queue_fence_wait(&shader->ready);

      if (optimized_or_none)
         return -1;
      goto again;
   }

   /* Reset the fence before adding to the variant list. */
   util_queue_fence_reset(&shader->ready);

   if (!sel->last_variant) {
      sel->first_variant = shader;
      sel->last_variant = shader;
   } else {
      sel->last_variant->next_variant = shader;
      sel->last_variant = shader;
   }

   simple_mtx_unlock(&sel->mutex);

   assert(!shader->is_optimized);
   si_build_shader_variant(shader, thread_index, false);

   util_queue_fence_signal(&shader->ready);

   if (!shader->compilation_failed)
      state->current = shader;

   return shader->compilation_failed ? -1 : 0;
}

int si_shader_select(struct pipe_context *ctx, struct si_shader_ctx_state *state)
{
   struct si_context *sctx = (struct si_context *)ctx;

   si_shader_selector_key(ctx, state->cso, &state->key);
   return si_shader_select_with_key(sctx, state, &state->key, -1, false);
}

static void si_parse_next_shader_property(const struct si_shader_info *info, bool streamout,
                                          struct si_shader_key *key)
{
   gl_shader_stage next_shader = info->base.next_stage;

   switch (info->stage) {
   case MESA_SHADER_VERTEX:
      switch (next_shader) {
      case MESA_SHADER_GEOMETRY:
         key->as_es = 1;
         break;
      case MESA_SHADER_TESS_CTRL:
      case MESA_SHADER_TESS_EVAL:
         key->as_ls = 1;
         break;
      default:
         /* If POSITION isn't written, it can only be a HW VS
          * if streamout is used. If streamout isn't used,
          * assume that it's a HW LS. (the next shader is TCS)
          * This heuristic is needed for separate shader objects.
          */
         if (!info->writes_position && !streamout)
            key->as_ls = 1;
      }
      break;

   case MESA_SHADER_TESS_EVAL:
      if (next_shader == MESA_SHADER_GEOMETRY || !info->writes_position)
         key->as_es = 1;
      break;

   default:;
   }
}

/**
 * Compile the main shader part or the monolithic shader as part of
 * si_shader_selector initialization. Since it can be done asynchronously,
 * there is no way to report compile failures to applications.
 */
static void si_init_shader_selector_async(void *job, void *gdata, int thread_index)
{
   struct si_shader_selector *sel = (struct si_shader_selector *)job;
   struct si_screen *sscreen = sel->screen;
   struct ac_llvm_compiler *compiler;
   struct pipe_debug_callback *debug = &sel->compiler_ctx_state.debug;

   assert(!debug->debug_message || debug->async);
   assert(thread_index >= 0);
   assert(thread_index < ARRAY_SIZE(sscreen->compiler));
   compiler = &sscreen->compiler[thread_index];

   if (!compiler->passes)
      si_init_compiler(sscreen, compiler);

   /* The GS copy shader is always pre-compiled. */
   if (sel->info.stage == MESA_SHADER_GEOMETRY &&
       (!sscreen->use_ngg || !sscreen->use_ngg_streamout || /* also for PRIMITIVES_GENERATED */
        sel->tess_turns_off_ngg)) {
      sel->gs_copy_shader = si_generate_gs_copy_shader(sscreen, compiler, sel, debug);
      if (!sel->gs_copy_shader) {
         fprintf(stderr, "radeonsi: can't create GS copy shader\n");
         return;
      }

      si_shader_vs(sscreen, sel->gs_copy_shader, sel);
   }

   /* Serialize NIR to save memory. Monolithic shader variants
    * have to deserialize NIR before compilation.
    */
   if (sel->nir) {
      struct blob blob;
      size_t size;

      blob_init(&blob);
      /* true = remove optional debugging data to increase
       * the likehood of getting more shader cache hits.
       * It also drops variable names, so we'll save more memory.
       */
      nir_serialize(&blob, sel->nir, true);
      blob_finish_get_buffer(&blob, &sel->nir_binary, &size);
      sel->nir_size = size;
   }

   /* Compile the main shader part for use with a prolog and/or epilog.
    * If this fails, the driver will try to compile a monolithic shader
    * on demand.
    */
   if (!sscreen->use_monolithic_shaders) {
      struct si_shader *shader = CALLOC_STRUCT(si_shader);
      unsigned char ir_sha1_cache_key[20];

      if (!shader) {
         fprintf(stderr, "radeonsi: can't allocate a main shader part\n");
         return;
      }

      /* We can leave the fence signaled because use of the default
       * main part is guarded by the selector's ready fence. */
      util_queue_fence_init(&shader->ready);

      shader->selector = sel;
      shader->is_monolithic = false;
      si_parse_next_shader_property(&sel->info, sel->so.num_outputs != 0, &shader->key);

      if (sscreen->use_ngg && (!sel->so.num_outputs || sscreen->use_ngg_streamout) &&
          ((sel->info.stage == MESA_SHADER_VERTEX && !shader->key.as_ls) ||
           sel->info.stage == MESA_SHADER_TESS_EVAL || sel->info.stage == MESA_SHADER_GEOMETRY))
         shader->key.as_ngg = 1;

      if (sel->nir) {
         si_get_ir_cache_key(sel, shader->key.as_ngg, shader->key.as_es, ir_sha1_cache_key);
      }

      /* Try to load the shader from the shader cache. */
      simple_mtx_lock(&sscreen->shader_cache_mutex);

      if (si_shader_cache_load_shader(sscreen, ir_sha1_cache_key, shader)) {
         simple_mtx_unlock(&sscreen->shader_cache_mutex);
         si_shader_dump_stats_for_shader_db(sscreen, shader, debug);
      } else {
         simple_mtx_unlock(&sscreen->shader_cache_mutex);

         /* Compile the shader if it hasn't been loaded from the cache. */
         if (!si_compile_shader(sscreen, compiler, shader, debug)) {
            FREE(shader);
            fprintf(stderr, "radeonsi: can't compile a main shader part\n");
            return;
         }

         simple_mtx_lock(&sscreen->shader_cache_mutex);
         si_shader_cache_insert_shader(sscreen, ir_sha1_cache_key, shader, true);
         simple_mtx_unlock(&sscreen->shader_cache_mutex);
      }

      *si_get_main_shader_part(sel, &shader->key) = shader;

      /* Unset "outputs_written" flags for outputs converted to
       * DEFAULT_VAL, so that later inter-shader optimizations don't
       * try to eliminate outputs that don't exist in the final
       * shader.
       *
       * This is only done if non-monolithic shaders are enabled.
       */
      if ((sel->info.stage == MESA_SHADER_VERTEX ||
           sel->info.stage == MESA_SHADER_TESS_EVAL ||
           sel->info.stage == MESA_SHADER_GEOMETRY) &&
          !shader->key.as_ls && !shader->key.as_es) {
         unsigned i;

         for (i = 0; i < sel->info.num_outputs; i++) {
            unsigned semantic = sel->info.output_semantic[i];
            unsigned ps_input_cntl = shader->info.vs_output_ps_input_cntl[semantic];

            /* OFFSET=0x20 means DEFAULT_VAL, which means VS doesn't export it. */
            if (G_028644_OFFSET(ps_input_cntl) != 0x20)
               continue;

            unsigned id;

            /* Remove the output from the mask. */
            if ((semantic <= VARYING_SLOT_VAR31 || semantic >= VARYING_SLOT_VAR0_16BIT) &&
                semantic != VARYING_SLOT_POS &&
                semantic != VARYING_SLOT_PSIZ &&
                semantic != VARYING_SLOT_CLIP_VERTEX &&
                semantic != VARYING_SLOT_EDGE) {
               id = si_shader_io_get_unique_index(semantic, true);
               sel->outputs_written_before_ps &= ~(1ull << id);
            }
         }
      }
   }

   /* Free NIR. We only keep serialized NIR after this point. */
   if (sel->nir) {
      ralloc_free(sel->nir);
      sel->nir = NULL;
   }
}

void si_schedule_initial_compile(struct si_context *sctx, gl_shader_stage stage,
                                 struct util_queue_fence *ready_fence,
                                 struct si_compiler_ctx_state *compiler_ctx_state, void *job,
                                 util_queue_execute_func execute)
{
   util_queue_fence_init(ready_fence);

   struct util_async_debug_callback async_debug;
   bool debug = (sctx->debug.debug_message && !sctx->debug.async) || sctx->is_debug ||
                si_can_dump_shader(sctx->screen, stage);

   if (debug) {
      u_async_debug_init(&async_debug);
      compiler_ctx_state->debug = async_debug.base;
   }

   util_queue_add_job(&sctx->screen->shader_compiler_queue, job, ready_fence, execute, NULL, 0);

   if (debug) {
      util_queue_fence_wait(ready_fence);
      u_async_debug_drain(&async_debug, &sctx->debug);
      u_async_debug_cleanup(&async_debug);
   }

   if (sctx->screen->options.sync_compile)
      util_queue_fence_wait(ready_fence);
}

/* Return descriptor slot usage masks from the given shader info. */
void si_get_active_slot_masks(const struct si_shader_info *info, uint64_t *const_and_shader_buffers,
                              uint64_t *samplers_and_images)
{
   unsigned start, num_shaderbufs, num_constbufs, num_images, num_msaa_images, num_samplers;

   num_shaderbufs = info->base.num_ssbos;
   num_constbufs = info->base.num_ubos;
   /* two 8-byte images share one 16-byte slot */
   num_images = align(info->base.num_images, 2);
   num_msaa_images = align(util_last_bit(info->base.msaa_images), 2);
   num_samplers = BITSET_LAST_BIT(info->base.textures_used);

   /* The layout is: sb[last] ... sb[0], cb[0] ... cb[last] */
   start = si_get_shaderbuf_slot(num_shaderbufs - 1);
   *const_and_shader_buffers = u_bit_consecutive64(start, num_shaderbufs + num_constbufs);

   /* The layout is:
    *   - fmask[last] ... fmask[0]     go to [15-last .. 15]
    *   - image[last] ... image[0]     go to [31-last .. 31]
    *   - sampler[0] ... sampler[last] go to [32 .. 32+last*2]
    *
    * FMASKs for images are placed separately, because MSAA images are rare,
    * and so we can benefit from a better cache hit rate if we keep image
    * descriptors together.
    */
   if (num_msaa_images)
      num_images = SI_NUM_IMAGES + num_msaa_images; /* add FMASK descriptors */

   start = si_get_image_slot(num_images - 1) / 2;
   *samplers_and_images = u_bit_consecutive64(start, num_images / 2 + num_samplers);
}

static void *si_create_shader_selector(struct pipe_context *ctx,
                                       const struct pipe_shader_state *state)
{
   struct si_screen *sscreen = (struct si_screen *)ctx->screen;
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *sel = CALLOC_STRUCT(si_shader_selector);
   int i;

   if (!sel)
      return NULL;

   sel->screen = sscreen;
   sel->compiler_ctx_state.debug = sctx->debug;
   sel->compiler_ctx_state.is_debug_context = sctx->is_debug;

   sel->so = state->stream_output;

   if (state->type == PIPE_SHADER_IR_TGSI) {
      sel->nir = tgsi_to_nir(state->tokens, ctx->screen, true);
   } else {
      assert(state->type == PIPE_SHADER_IR_NIR);
      sel->nir = state->ir.nir;
   }

   si_nir_scan_shader(sel->nir, &sel->info);

   const enum pipe_shader_type type = pipe_shader_type_from_mesa(sel->info.stage);
   sel->pipe_shader_type = type;
   sel->const_and_shader_buf_descriptors_index =
      si_const_and_shader_buffer_descriptors_idx(type);
   sel->sampler_and_images_descriptors_index =
      si_sampler_and_image_descriptors_idx(type);

   p_atomic_inc(&sscreen->num_shaders_created);
   si_get_active_slot_masks(&sel->info, &sel->active_const_and_shader_buffers,
                            &sel->active_samplers_and_images);

   /* Record which streamout buffers are enabled. */
   for (i = 0; i < sel->so.num_outputs; i++) {
      sel->enabled_streamout_buffer_mask |= (1 << sel->so.output[i].output_buffer)
                                            << (sel->so.output[i].stream * 4);
   }

   sel->num_vs_inputs =
      sel->info.stage == MESA_SHADER_VERTEX && !sel->info.base.vs.blit_sgprs_amd
         ? sel->info.num_inputs
         : 0;
   unsigned num_vbos_in_sgprs = si_num_vbos_in_user_sgprs_inline(sscreen->info.chip_class);
   sel->num_vbos_in_user_sgprs = MIN2(sel->num_vs_inputs, num_vbos_in_sgprs);

   /* The prolog is a no-op if there are no inputs. */
   sel->vs_needs_prolog = sel->info.stage == MESA_SHADER_VERTEX && sel->info.num_inputs &&
                          !sel->info.base.vs.blit_sgprs_amd;

   if (sel->info.stage == MESA_SHADER_VERTEX ||
       sel->info.stage == MESA_SHADER_TESS_CTRL ||
       sel->info.stage == MESA_SHADER_TESS_EVAL ||
       sel->info.stage == MESA_SHADER_GEOMETRY) {
      if (sel->info.stage == MESA_SHADER_TESS_CTRL) {
         /* Always reserve space for these. */
         sel->patch_outputs_written |=
            (1ull << si_shader_io_get_unique_index_patch(VARYING_SLOT_TESS_LEVEL_INNER)) |
            (1ull << si_shader_io_get_unique_index_patch(VARYING_SLOT_TESS_LEVEL_OUTER));
      }
      for (i = 0; i < sel->info.num_outputs; i++) {
         unsigned semantic = sel->info.output_semantic[i];

         if (semantic == VARYING_SLOT_TESS_LEVEL_INNER ||
             semantic == VARYING_SLOT_TESS_LEVEL_OUTER ||
             (semantic >= VARYING_SLOT_PATCH0 && semantic < VARYING_SLOT_TESS_MAX)) {
            sel->patch_outputs_written |= 1ull << si_shader_io_get_unique_index_patch(semantic);
         } else if ((semantic <= VARYING_SLOT_VAR31 || semantic >= VARYING_SLOT_VAR0_16BIT) &&
                    semantic != VARYING_SLOT_EDGE) {
            sel->outputs_written |= 1ull << si_shader_io_get_unique_index(semantic, false);

            /* Ignore outputs that are not passed from VS to PS. */
            if (semantic != VARYING_SLOT_POS &&
                semantic != VARYING_SLOT_PSIZ &&
                semantic != VARYING_SLOT_CLIP_VERTEX) {
               sel->outputs_written_before_ps |= 1ull
                                                 << si_shader_io_get_unique_index(semantic, true);
            }
         }
      }
   }

   switch (sel->info.stage) {
   case MESA_SHADER_GEOMETRY:
      /* Only possibilities: POINTS, LINE_STRIP, TRIANGLES */
      sel->rast_prim = sel->info.base.gs.output_primitive;
      if (util_rast_prim_is_triangles(sel->rast_prim))
         sel->rast_prim = PIPE_PRIM_TRIANGLES;

      sel->gsvs_vertex_size = sel->info.num_outputs * 16;
      sel->max_gsvs_emit_size = sel->gsvs_vertex_size * sel->info.base.gs.vertices_out;
      sel->gs_input_verts_per_prim =
         u_vertices_per_prim(sel->info.base.gs.input_primitive);

      /* EN_MAX_VERT_OUT_PER_GS_INSTANCE does not work with tesselation so
       * we can't split workgroups. Disable ngg if any of the following conditions is true:
       * - num_invocations * gs.vertices_out > 256
       * - LDS usage is too high
       */
      sel->tess_turns_off_ngg = sscreen->info.chip_class >= GFX10 &&
                                (sel->info.base.gs.invocations * sel->info.base.gs.vertices_out > 256 ||
                                 sel->info.base.gs.invocations * sel->info.base.gs.vertices_out *
                                 (sel->info.num_outputs * 4 + 1) > 6500 /* max dw per GS primitive */);
      break;

   case MESA_SHADER_VERTEX:
   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_TESS_EVAL:
      sel->esgs_itemsize = util_last_bit64(sel->outputs_written) * 16;
      sel->lshs_vertex_stride = sel->esgs_itemsize;

      /* Add 1 dword to reduce LDS bank conflicts, so that each vertex
       * will start on a different bank. (except for the maximum 32*16).
       */
      if (sel->lshs_vertex_stride < 32 * 16)
         sel->lshs_vertex_stride += 4;

      /* For the ESGS ring in LDS, add 1 dword to reduce LDS bank
       * conflicts, i.e. each vertex will start at a different bank.
       */
      if (sctx->chip_class >= GFX9)
         sel->esgs_itemsize += 4;

      assert(((sel->esgs_itemsize / 4) & C_028AAC_ITEMSIZE) == 0);

      sel->tcs_vgpr_only_inputs = ~sel->info.base.tess.tcs_cross_invocation_inputs_read &
                                  ~sel->info.base.inputs_read_indirectly &
                                  sel->info.base.inputs_read;

      /* Only for TES: */
      if (sel->info.stage == MESA_SHADER_TESS_EVAL) {
         if (sel->info.base.tess.point_mode)
            sel->rast_prim = PIPE_PRIM_POINTS;
         else if (sel->info.base.tess.primitive_mode == GL_LINES)
            sel->rast_prim = PIPE_PRIM_LINE_STRIP;
         else
            sel->rast_prim = PIPE_PRIM_TRIANGLES;
      } else {
         sel->rast_prim = PIPE_PRIM_TRIANGLES;
      }
      break;

   case MESA_SHADER_FRAGMENT:
      for (i = 0; i < sel->info.num_inputs; i++) {
         unsigned semantic = sel->info.input[i].semantic;

         if ((semantic <= VARYING_SLOT_VAR31 || semantic >= VARYING_SLOT_VAR0_16BIT) &&
             semantic != VARYING_SLOT_PNTC) {
            sel->inputs_read |= 1ull << si_shader_io_get_unique_index(semantic, true);
         }
      }

      for (i = 0; i < 8; i++)
         if (sel->info.colors_written & (1 << i))
            sel->colors_written_4bit |= 0xf << (4 * i);

      for (i = 0; i < sel->info.num_inputs; i++) {
         if (sel->info.input[i].semantic == VARYING_SLOT_COL0)
            sel->color_attr_index[0] = i;
         else if (sel->info.input[i].semantic == VARYING_SLOT_COL1)
            sel->color_attr_index[1] = i;
      }
      break;
   default:;
   }

   bool ngg_culling_allowed =
      sscreen->info.chip_class >= GFX10 &&
      sscreen->use_ngg_culling &&
      (sel->info.stage == MESA_SHADER_VERTEX ||
       sel->info.stage == MESA_SHADER_TESS_EVAL) &&
      sel->info.writes_position &&
      !sel->info.writes_viewport_index && /* cull only against viewport 0 */
      !sel->info.base.writes_memory && !sel->so.num_outputs &&
      (sel->info.stage != MESA_SHADER_VERTEX ||
       (!sel->info.base.vs.blit_sgprs_amd &&
        !sel->info.base.vs.window_space_position));

   sel->ngg_cull_vert_threshold = UINT_MAX; /* disabled (changed below) */

   if (ngg_culling_allowed) {
      if (sel->info.stage == MESA_SHADER_VERTEX) {
         if (sscreen->debug_flags & DBG(ALWAYS_NGG_CULLING_ALL))
            sel->ngg_cull_vert_threshold = 0; /* always enabled */
         else if (sscreen->options.shader_culling ||
                  sscreen->info.chip_class == GFX10_3 ||
                  (sscreen->info.chip_class == GFX10 &&
                   sscreen->info.is_pro_graphics)) {
            sel->ngg_cull_vert_threshold = 128;
         }
      } else if (sel->info.stage == MESA_SHADER_TESS_EVAL) {
         if (sel->rast_prim != PIPE_PRIM_POINTS &&
             (sscreen->debug_flags & DBG(ALWAYS_NGG_CULLING_ALL) ||
              sscreen->debug_flags & DBG(ALWAYS_NGG_CULLING_TESS) ||
              sscreen->info.chip_class == GFX10_3))
            sel->ngg_cull_vert_threshold = 0; /* always enabled */
      }
   }

   sel->clipdist_mask = sel->info.writes_clipvertex ? SIX_BITS :
                           u_bit_consecutive(0, sel->info.base.clip_distance_array_size);
   sel->culldist_mask = u_bit_consecutive(0, sel->info.base.cull_distance_array_size) <<
                        sel->info.base.clip_distance_array_size;

   /* DB_SHADER_CONTROL */
   sel->db_shader_control = S_02880C_Z_EXPORT_ENABLE(sel->info.writes_z) |
                            S_02880C_STENCIL_TEST_VAL_EXPORT_ENABLE(sel->info.writes_stencil) |
                            S_02880C_MASK_EXPORT_ENABLE(sel->info.writes_samplemask) |
                            S_02880C_KILL_ENABLE(sel->info.base.fs.uses_discard);

   if (sel->info.stage == MESA_SHADER_FRAGMENT) {
      switch (sel->info.base.fs.depth_layout) {
      case FRAG_DEPTH_LAYOUT_GREATER:
         sel->db_shader_control |= S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_GREATER_THAN_Z);
         break;
      case FRAG_DEPTH_LAYOUT_LESS:
         sel->db_shader_control |= S_02880C_CONSERVATIVE_Z_EXPORT(V_02880C_EXPORT_LESS_THAN_Z);
         break;
      default:;
      }

      /* Z_ORDER, EXEC_ON_HIER_FAIL and EXEC_ON_NOOP should be set as following:
       *
       *   | early Z/S | writes_mem | allow_ReZ? |      Z_ORDER       | EXEC_ON_HIER_FAIL | EXEC_ON_NOOP
       * --|-----------|------------|------------|--------------------|-------------------|-------------
       * 1a|   false   |   false    |   true     | EarlyZ_Then_ReZ    |         0         |     0
       * 1b|   false   |   false    |   false    | EarlyZ_Then_LateZ  |         0         |     0
       * 2 |   false   |   true     |   n/a      |       LateZ        |         1         |     0
       * 3 |   true    |   false    |   n/a      | EarlyZ_Then_LateZ  |         0         |     0
       * 4 |   true    |   true     |   n/a      | EarlyZ_Then_LateZ  |         0         |     1
       *
       * In cases 3 and 4, HW will force Z_ORDER to EarlyZ regardless of what's set in the register.
       * In case 2, NOOP_CULL is a don't care field. In case 2, 3 and 4, ReZ doesn't make sense.
       *
       * Don't use ReZ without profiling !!!
       *
       * ReZ decreases performance by 15% in DiRT: Showdown on Ultra settings, which has pretty complex
       * shaders.
       */
      if (sel->info.base.fs.early_fragment_tests) {
         /* Cases 3, 4. */
         sel->db_shader_control |= S_02880C_DEPTH_BEFORE_SHADER(1) |
                                   S_02880C_Z_ORDER(V_02880C_EARLY_Z_THEN_LATE_Z) |
                                   S_02880C_EXEC_ON_NOOP(sel->info.base.writes_memory);
      } else if (sel->info.base.writes_memory) {
         /* Case 2. */
         sel->db_shader_control |= S_02880C_Z_ORDER(V_02880C_LATE_Z) | S_02880C_EXEC_ON_HIER_FAIL(1);
      } else {
         /* Case 1. */
         sel->db_shader_control |= S_02880C_Z_ORDER(V_02880C_EARLY_Z_THEN_LATE_Z);
      }

      if (sel->info.base.fs.post_depth_coverage)
         sel->db_shader_control |= S_02880C_PRE_SHADER_DEPTH_COVERAGE_ENABLE(1);
   }

   (void)simple_mtx_init(&sel->mutex, mtx_plain);

   si_schedule_initial_compile(sctx, sel->info.stage, &sel->ready, &sel->compiler_ctx_state,
                               sel, si_init_shader_selector_async);
   return sel;
}

static void *si_create_shader(struct pipe_context *ctx, const struct pipe_shader_state *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_screen *sscreen = (struct si_screen *)ctx->screen;
   bool cache_hit;
   struct si_shader_selector *sel = (struct si_shader_selector *)util_live_shader_cache_get(
      ctx, &sscreen->live_shader_cache, state, &cache_hit);

   if (sel && cache_hit && sctx->debug.debug_message) {
      if (sel->main_shader_part)
         si_shader_dump_stats_for_shader_db(sscreen, sel->main_shader_part, &sctx->debug);
      if (sel->main_shader_part_ls)
         si_shader_dump_stats_for_shader_db(sscreen, sel->main_shader_part_ls, &sctx->debug);
      if (sel->main_shader_part_es)
         si_shader_dump_stats_for_shader_db(sscreen, sel->main_shader_part_es, &sctx->debug);
      if (sel->main_shader_part_ngg)
         si_shader_dump_stats_for_shader_db(sscreen, sel->main_shader_part_ngg, &sctx->debug);
      if (sel->main_shader_part_ngg_es)
         si_shader_dump_stats_for_shader_db(sscreen, sel->main_shader_part_ngg_es, &sctx->debug);
   }
   return sel;
}

static void si_update_streamout_state(struct si_context *sctx)
{
   struct si_shader_selector *shader_with_so = si_get_vs(sctx)->cso;

   if (!shader_with_so)
      return;

   sctx->streamout.enabled_stream_buffers_mask = shader_with_so->enabled_streamout_buffer_mask;
   sctx->streamout.stride_in_dw = shader_with_so->so.stride;
}

static void si_update_clip_regs(struct si_context *sctx, struct si_shader_selector *old_hw_vs,
                                struct si_shader *old_hw_vs_variant,
                                struct si_shader_selector *next_hw_vs,
                                struct si_shader *next_hw_vs_variant)
{
   if (next_hw_vs &&
       (!old_hw_vs ||
        (old_hw_vs->info.stage == MESA_SHADER_VERTEX && old_hw_vs->info.base.vs.window_space_position) !=
        (next_hw_vs->info.stage == MESA_SHADER_VERTEX && next_hw_vs->info.base.vs.window_space_position) ||
        old_hw_vs->clipdist_mask != next_hw_vs->clipdist_mask ||
        old_hw_vs->culldist_mask != next_hw_vs->culldist_mask || !old_hw_vs_variant ||
        !next_hw_vs_variant ||
        old_hw_vs_variant->pa_cl_vs_out_cntl != next_hw_vs_variant->pa_cl_vs_out_cntl))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.clip_regs);
}

static void si_update_rasterized_prim(struct si_context *sctx)
{
   enum pipe_prim_type rast_prim;

   if (sctx->shader.gs.cso) {
      /* Only possibilities: POINTS, LINE_STRIP, TRIANGLES */
      rast_prim = sctx->shader.gs.cso->rast_prim;
   } else if (sctx->shader.tes.cso) {
      /* Only possibilities: POINTS, LINE_STRIP, TRIANGLES */
      rast_prim = sctx->shader.tes.cso->rast_prim;
   } else {
      /* Determined by draw calls. */
      return;
   }

   if (rast_prim != sctx->current_rast_prim) {
      if (util_prim_is_points_or_lines(sctx->current_rast_prim) !=
          util_prim_is_points_or_lines(rast_prim))
         si_mark_atom_dirty(sctx, &sctx->atoms.s.guardband);

      sctx->current_rast_prim = rast_prim;
   }
}

static void si_update_common_shader_state(struct si_context *sctx, struct si_shader_selector *sel,
                                          enum pipe_shader_type type)
{
   si_set_active_descriptors_for_shader(sctx, sel);

   sctx->uses_bindless_samplers = si_shader_uses_bindless_samplers(sctx->shader.vs.cso) ||
                                  si_shader_uses_bindless_samplers(sctx->shader.gs.cso) ||
                                  si_shader_uses_bindless_samplers(sctx->shader.ps.cso) ||
                                  si_shader_uses_bindless_samplers(sctx->shader.tcs.cso) ||
                                  si_shader_uses_bindless_samplers(sctx->shader.tes.cso);
   sctx->uses_bindless_images = si_shader_uses_bindless_images(sctx->shader.vs.cso) ||
                                si_shader_uses_bindless_images(sctx->shader.gs.cso) ||
                                si_shader_uses_bindless_images(sctx->shader.ps.cso) ||
                                si_shader_uses_bindless_images(sctx->shader.tcs.cso) ||
                                si_shader_uses_bindless_images(sctx->shader.tes.cso);

   if (type == PIPE_SHADER_VERTEX || type == PIPE_SHADER_TESS_EVAL || type == PIPE_SHADER_GEOMETRY)
      sctx->ngg_culling = 0; /* this will be enabled on the first draw if needed */

   si_invalidate_inlinable_uniforms(sctx, type);
   sctx->do_update_shaders = true;
}

static void si_bind_vs_shader(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *old_hw_vs = si_get_vs(sctx)->cso;
   struct si_shader *old_hw_vs_variant = si_get_vs(sctx)->current;
   struct si_shader_selector *sel = state;

   if (sctx->shader.vs.cso == sel)
      return;

   sctx->shader.vs.cso = sel;
   sctx->shader.vs.current = sel ? sel->first_variant : NULL;
   sctx->num_vs_blit_sgprs = sel ? sel->info.base.vs.blit_sgprs_amd : 0;
   sctx->vs_uses_draw_id = sel ? sel->info.uses_drawid : false;
   sctx->fixed_func_tcs_shader.key.mono.u.ff_tcs_inputs_to_copy = sel ? sel->outputs_written : 0;

   if (si_update_ngg(sctx))
      si_shader_change_notify(sctx);

   si_update_common_shader_state(sctx, sel, PIPE_SHADER_VERTEX);
   si_select_draw_vbo(sctx);
   si_update_vs_viewport_state(sctx);
   si_update_streamout_state(sctx);
   si_update_clip_regs(sctx, old_hw_vs, old_hw_vs_variant, si_get_vs(sctx)->cso,
                       si_get_vs(sctx)->current);
   si_update_rasterized_prim(sctx);
   si_vs_key_update_inputs(sctx);
}

static void si_update_tess_uses_prim_id(struct si_context *sctx)
{
   sctx->ia_multi_vgt_param_key.u.tess_uses_prim_id =
      (sctx->shader.tes.cso && sctx->shader.tes.cso->info.uses_primid) ||
      (sctx->shader.tcs.cso && sctx->shader.tcs.cso->info.uses_primid) ||
      (sctx->shader.gs.cso && sctx->shader.gs.cso->info.uses_primid) ||
      (sctx->shader.ps.cso && !sctx->shader.gs.cso && sctx->shader.ps.cso->info.uses_primid);
}

bool si_update_ngg(struct si_context *sctx)
{
   if (!sctx->screen->use_ngg) {
      assert(!sctx->ngg);
      return false;
   }

   bool new_ngg = true;

   if (sctx->shader.gs.cso && sctx->shader.tes.cso && sctx->shader.gs.cso->tess_turns_off_ngg) {
      new_ngg = false;
   } else if (!sctx->screen->use_ngg_streamout) {
      struct si_shader_selector *last = si_get_vs(sctx)->cso;

      if ((last && last->so.num_outputs) || sctx->streamout.prims_gen_query_enabled)
         new_ngg = false;
   }

   if (new_ngg != sctx->ngg) {
      /* Transitioning from NGG to legacy GS requires VGT_FLUSH on Navi10-14.
       * VGT_FLUSH is also emitted at the beginning of IBs when legacy GS ring
       * pointers are set.
       */
      if (sctx->screen->info.has_vgt_flush_ngg_legacy_bug && !new_ngg) {
         sctx->flags |= SI_CONTEXT_VGT_FLUSH;
         if (sctx->chip_class == GFX10) {
            /* Workaround for https://gitlab.freedesktop.org/mesa/mesa/-/issues/2941 */
            si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
         }
      }

      sctx->ngg = new_ngg;
      sctx->last_gs_out_prim = -1; /* reset this so that it gets updated */
      si_select_draw_vbo(sctx);
      return true;
   }
   return false;
}

static void si_bind_gs_shader(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *old_hw_vs = si_get_vs(sctx)->cso;
   struct si_shader *old_hw_vs_variant = si_get_vs(sctx)->current;
   struct si_shader_selector *sel = state;
   bool enable_changed = !!sctx->shader.gs.cso != !!sel;
   bool ngg_changed;

   if (sctx->shader.gs.cso == sel)
      return;

   sctx->shader.gs.cso = sel;
   sctx->shader.gs.current = sel ? sel->first_variant : NULL;
   sctx->ia_multi_vgt_param_key.u.uses_gs = sel != NULL;

   si_update_common_shader_state(sctx, sel, PIPE_SHADER_GEOMETRY);
   si_select_draw_vbo(sctx);
   sctx->last_gs_out_prim = -1; /* reset this so that it gets updated */

   ngg_changed = si_update_ngg(sctx);
   if (ngg_changed || enable_changed)
      si_shader_change_notify(sctx);
   if (enable_changed) {
      if (sctx->ia_multi_vgt_param_key.u.uses_tess)
         si_update_tess_uses_prim_id(sctx);
   }
   si_update_vs_viewport_state(sctx);
   si_update_streamout_state(sctx);
   si_update_clip_regs(sctx, old_hw_vs, old_hw_vs_variant, si_get_vs(sctx)->cso,
                       si_get_vs(sctx)->current);
   si_update_rasterized_prim(sctx);
}

static void si_bind_tcs_shader(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *sel = state;
   bool enable_changed = !!sctx->shader.tcs.cso != !!sel;

   if (sctx->shader.tcs.cso == sel)
      return;

   sctx->shader.tcs.cso = sel;
   sctx->shader.tcs.current = sel ? sel->first_variant : NULL;
   sctx->shader.tcs.key.part.tcs.epilog.invoc0_tess_factors_are_def =
      sel ? sel->info.tessfactors_are_def_in_all_invocs : 0;
   si_update_tess_uses_prim_id(sctx);

   si_update_common_shader_state(sctx, sel, PIPE_SHADER_TESS_CTRL);

   if (enable_changed)
      sctx->last_tcs = NULL; /* invalidate derived tess state */
}

static void si_bind_tes_shader(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *old_hw_vs = si_get_vs(sctx)->cso;
   struct si_shader *old_hw_vs_variant = si_get_vs(sctx)->current;
   struct si_shader_selector *sel = state;
   bool enable_changed = !!sctx->shader.tes.cso != !!sel;

   if (sctx->shader.tes.cso == sel)
      return;

   sctx->shader.tes.cso = sel;
   sctx->shader.tes.current = sel ? sel->first_variant : NULL;
   sctx->ia_multi_vgt_param_key.u.uses_tess = sel != NULL;
   si_update_tess_uses_prim_id(sctx);

   sctx->shader.tcs.key.part.tcs.epilog.prim_mode =
   sctx->fixed_func_tcs_shader.key.part.tcs.epilog.prim_mode =
      sel ? sel->info.base.tess.primitive_mode : 0;

   sctx->shader.tcs.key.part.tcs.epilog.tes_reads_tess_factors =
   sctx->fixed_func_tcs_shader.key.part.tcs.epilog.tes_reads_tess_factors =
      sel ? sel->info.reads_tess_factors : 0;

   si_update_common_shader_state(sctx, sel, PIPE_SHADER_TESS_EVAL);
   si_select_draw_vbo(sctx);
   sctx->last_gs_out_prim = -1; /* reset this so that it gets updated */

   bool ngg_changed = si_update_ngg(sctx);
   if (ngg_changed || enable_changed)
      si_shader_change_notify(sctx);
   if (enable_changed)
      sctx->last_tes_sh_base = -1; /* invalidate derived tess state */
   si_update_vs_viewport_state(sctx);
   si_update_streamout_state(sctx);
   si_update_clip_regs(sctx, old_hw_vs, old_hw_vs_variant, si_get_vs(sctx)->cso,
                       si_get_vs(sctx)->current);
   si_update_rasterized_prim(sctx);
}

void si_update_ps_kill_enable(struct si_context *sctx)
{
   if (!sctx->shader.ps.cso)
      return;

   unsigned db_shader_control = sctx->shader.ps.cso->db_shader_control |
                                S_02880C_KILL_ENABLE(sctx->queued.named.dsa->alpha_func != PIPE_FUNC_ALWAYS);

   if (sctx->ps_db_shader_control != db_shader_control) {
      sctx->ps_db_shader_control = db_shader_control;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
      if (sctx->screen->dpbb_allowed)
         si_mark_atom_dirty(sctx, &sctx->atoms.s.dpbb_state);
   }
}

void si_update_vrs_flat_shading(struct si_context *sctx)
{
   if (sctx->chip_class >= GFX10_3 && sctx->shader.ps.cso) {
      struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;
      struct si_shader_info *info = &sctx->shader.ps.cso->info;
      bool allow_flat_shading = info->allow_flat_shading;

      if (allow_flat_shading &&
          (rs->line_smooth || rs->poly_smooth || rs->poly_stipple_enable ||
           (!rs->flatshade && info->uses_interp_color)))
         allow_flat_shading = false;

      if (sctx->allow_flat_shading != allow_flat_shading) {
         sctx->allow_flat_shading = allow_flat_shading;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
      }
   }
}

static void si_bind_ps_shader(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *old_sel = sctx->shader.ps.cso;
   struct si_shader_selector *sel = state;

   /* skip if supplied shader is one already in use */
   if (old_sel == sel)
      return;

   sctx->shader.ps.cso = sel;
   sctx->shader.ps.current = sel ? sel->first_variant : NULL;

   si_update_common_shader_state(sctx, sel, PIPE_SHADER_FRAGMENT);
   if (sel) {
      if (sctx->ia_multi_vgt_param_key.u.uses_tess)
         si_update_tess_uses_prim_id(sctx);

      if (!old_sel || old_sel->info.colors_written != sel->info.colors_written)
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cb_render_state);

      if (sctx->screen->has_out_of_order_rast &&
          (!old_sel || old_sel->info.base.writes_memory != sel->info.base.writes_memory ||
           old_sel->info.base.fs.early_fragment_tests !=
              sel->info.base.fs.early_fragment_tests))
         si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);
   }
   si_update_ps_colorbuf0_slot(sctx);

   si_ps_key_update_framebuffer(sctx);
   si_ps_key_update_framebuffer_blend(sctx);
   si_ps_key_update_blend_rasterizer(sctx);
   si_ps_key_update_rasterizer(sctx);
   si_ps_key_update_dsa(sctx);
   si_ps_key_update_sample_shading(sctx);
   si_ps_key_update_framebuffer_rasterizer_sample_shading(sctx);
   si_update_ps_inputs_read_or_disabled(sctx);
   si_update_ps_kill_enable(sctx);
   si_update_vrs_flat_shading(sctx);
}

static void si_delete_shader(struct si_context *sctx, struct si_shader *shader)
{
   if (shader->is_optimized) {
      util_queue_drop_job(&sctx->screen->shader_compiler_queue_low_priority, &shader->ready);
   }

   util_queue_fence_destroy(&shader->ready);

   /* If destroyed shaders were not unbound, the next compiled
    * shader variant could get the same pointer address and so
    * binding it to the same shader stage would be considered
    * a no-op, causing random behavior.
    */
   int state_index = -1;

   switch (shader->selector->info.stage) {
   case MESA_SHADER_VERTEX:
      if (shader->key.as_ls) {
         if (sctx->chip_class <= GFX8)
            state_index = SI_STATE_IDX(ls);
      } else if (shader->key.as_es) {
         if (sctx->chip_class <= GFX8)
            state_index = SI_STATE_IDX(es);
      } else if (shader->key.as_ngg) {
         state_index = SI_STATE_IDX(gs);
      } else {
         state_index = SI_STATE_IDX(vs);
      }
      break;
   case MESA_SHADER_TESS_CTRL:
      state_index = SI_STATE_IDX(hs);
      break;
   case MESA_SHADER_TESS_EVAL:
      if (shader->key.as_es) {
         if (sctx->chip_class <= GFX8)
            state_index = SI_STATE_IDX(es);
      } else if (shader->key.as_ngg) {
         state_index = SI_STATE_IDX(gs);
      } else {
         state_index = SI_STATE_IDX(vs);
      }
      break;
   case MESA_SHADER_GEOMETRY:
      if (shader->is_gs_copy_shader)
         state_index = SI_STATE_IDX(vs);
      else
         state_index = SI_STATE_IDX(gs);
      break;
   case MESA_SHADER_FRAGMENT:
      state_index = SI_STATE_IDX(ps);
      break;
   default:;
   }

   si_shader_selector_reference(sctx, &shader->previous_stage_sel, NULL);
   si_shader_destroy(shader);
   si_pm4_free_state(sctx, &shader->pm4, state_index);
}

static void si_destroy_shader_selector(struct pipe_context *ctx, void *cso)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *sel = (struct si_shader_selector *)cso;
   struct si_shader *p = sel->first_variant, *c;
   enum pipe_shader_type type = pipe_shader_type_from_mesa(sel->info.stage);

   util_queue_drop_job(&sctx->screen->shader_compiler_queue, &sel->ready);

   if (sctx->shaders[type].cso == sel) {
      sctx->shaders[type].cso = NULL;
      sctx->shaders[type].current = NULL;
   }

   while (p) {
      c = p->next_variant;
      si_delete_shader(sctx, p);
      p = c;
   }

   if (sel->main_shader_part)
      si_delete_shader(sctx, sel->main_shader_part);
   if (sel->main_shader_part_ls)
      si_delete_shader(sctx, sel->main_shader_part_ls);
   if (sel->main_shader_part_es)
      si_delete_shader(sctx, sel->main_shader_part_es);
   if (sel->main_shader_part_ngg)
      si_delete_shader(sctx, sel->main_shader_part_ngg);
   if (sel->gs_copy_shader)
      si_delete_shader(sctx, sel->gs_copy_shader);

   util_queue_fence_destroy(&sel->ready);
   simple_mtx_destroy(&sel->mutex);
   ralloc_free(sel->nir);
   free(sel->nir_binary);
   free(sel);
}

static void si_delete_shader_selector(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_shader_selector *sel = (struct si_shader_selector *)state;

   si_shader_selector_reference(sctx, &sel, NULL);
}

/**
 * Writing CONFIG or UCONFIG VGT registers requires VGT_FLUSH before that.
 */
static void si_cs_preamble_add_vgt_flush(struct si_context *sctx)
{
   /* We shouldn't get here if registers are shadowed. */
   assert(!sctx->shadowed_regs);

   if (sctx->cs_preamble_has_vgt_flush)
      return;

   /* Done by Vulkan before VGT_FLUSH. */
   si_pm4_cmd_add(sctx->cs_preamble_state, PKT3(PKT3_EVENT_WRITE, 0, 0));
   si_pm4_cmd_add(sctx->cs_preamble_state, EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));

   /* VGT_FLUSH is required even if VGT is idle. It resets VGT pointers. */
   si_pm4_cmd_add(sctx->cs_preamble_state, PKT3(PKT3_EVENT_WRITE, 0, 0));
   si_pm4_cmd_add(sctx->cs_preamble_state, EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));
   sctx->cs_preamble_has_vgt_flush = true;
}

/**
 * Writing CONFIG or UCONFIG VGT registers requires VGT_FLUSH before that.
 */
static void si_emit_vgt_flush(struct radeon_cmdbuf *cs)
{
   radeon_begin(cs);

   /* This is required before VGT_FLUSH. */
   radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));

   /* VGT_FLUSH is required even if VGT is idle. It resets VGT pointers. */
   radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));
   radeon_end();
}

/* Initialize state related to ESGS / GSVS ring buffers */
bool si_update_gs_ring_buffers(struct si_context *sctx)
{
   struct si_shader_selector *es =
      sctx->shader.tes.cso ? sctx->shader.tes.cso : sctx->shader.vs.cso;
   struct si_shader_selector *gs = sctx->shader.gs.cso;
   struct si_pm4_state *pm4;

   /* Chip constants. */
   unsigned num_se = sctx->screen->info.max_se;
   unsigned wave_size = 64;
   unsigned max_gs_waves = 32 * num_se; /* max 32 per SE on GCN */
   /* On GFX6-GFX7, the value comes from VGT_GS_VERTEX_REUSE = 16.
    * On GFX8+, the value comes from VGT_VERTEX_REUSE_BLOCK_CNTL = 30 (+2).
    */
   unsigned gs_vertex_reuse = (sctx->chip_class >= GFX8 ? 32 : 16) * num_se;
   unsigned alignment = 256 * num_se;
   /* The maximum size is 63.999 MB per SE. */
   unsigned max_size = ((unsigned)(63.999 * 1024 * 1024) & ~255) * num_se;

   /* Calculate the minimum size. */
   unsigned min_esgs_ring_size = align(es->esgs_itemsize * gs_vertex_reuse * wave_size, alignment);

   /* These are recommended sizes, not minimum sizes. */
   unsigned esgs_ring_size =
      max_gs_waves * 2 * wave_size * es->esgs_itemsize * gs->gs_input_verts_per_prim;
   unsigned gsvs_ring_size = max_gs_waves * 2 * wave_size * gs->max_gsvs_emit_size;

   min_esgs_ring_size = align(min_esgs_ring_size, alignment);
   esgs_ring_size = align(esgs_ring_size, alignment);
   gsvs_ring_size = align(gsvs_ring_size, alignment);

   esgs_ring_size = CLAMP(esgs_ring_size, min_esgs_ring_size, max_size);
   gsvs_ring_size = MIN2(gsvs_ring_size, max_size);

   /* Some rings don't have to be allocated if shaders don't use them.
    * (e.g. no varyings between ES and GS or GS and VS)
    *
    * GFX9 doesn't have the ESGS ring.
    */
   bool update_esgs = sctx->chip_class <= GFX8 && esgs_ring_size &&
                      (!sctx->esgs_ring || sctx->esgs_ring->width0 < esgs_ring_size);
   bool update_gsvs =
      gsvs_ring_size && (!sctx->gsvs_ring || sctx->gsvs_ring->width0 < gsvs_ring_size);

   if (!update_esgs && !update_gsvs)
      return true;

   if (update_esgs) {
      pipe_resource_reference(&sctx->esgs_ring, NULL);
      sctx->esgs_ring =
         pipe_aligned_buffer_create(sctx->b.screen,
                                    SI_RESOURCE_FLAG_UNMAPPABLE | SI_RESOURCE_FLAG_DRIVER_INTERNAL,
                                    PIPE_USAGE_DEFAULT,
                                    esgs_ring_size, sctx->screen->info.pte_fragment_size);
      if (!sctx->esgs_ring)
         return false;
   }

   if (update_gsvs) {
      pipe_resource_reference(&sctx->gsvs_ring, NULL);
      sctx->gsvs_ring =
         pipe_aligned_buffer_create(sctx->b.screen,
                                    SI_RESOURCE_FLAG_UNMAPPABLE | SI_RESOURCE_FLAG_DRIVER_INTERNAL,
                                    PIPE_USAGE_DEFAULT,
                                    gsvs_ring_size, sctx->screen->info.pte_fragment_size);
      if (!sctx->gsvs_ring)
         return false;
   }

   /* Set ring bindings. */
   if (sctx->esgs_ring) {
      assert(sctx->chip_class <= GFX8);
      si_set_ring_buffer(sctx, SI_ES_RING_ESGS, sctx->esgs_ring, 0, sctx->esgs_ring->width0, true,
                         true, 4, 64, 0);
      si_set_ring_buffer(sctx, SI_GS_RING_ESGS, sctx->esgs_ring, 0, sctx->esgs_ring->width0, false,
                         false, 0, 0, 0);
   }
   if (sctx->gsvs_ring) {
      si_set_ring_buffer(sctx, SI_RING_GSVS, sctx->gsvs_ring, 0, sctx->gsvs_ring->width0, false,
                         false, 0, 0, 0);
   }

   if (sctx->shadowed_regs) {
      /* These registers will be shadowed, so set them only once. */
      struct radeon_cmdbuf *cs = &sctx->gfx_cs;

      assert(sctx->chip_class >= GFX7);

      si_emit_vgt_flush(cs);

      radeon_begin(cs);

      /* Set the GS registers. */
      if (sctx->esgs_ring) {
         assert(sctx->chip_class <= GFX8);
         radeon_set_uconfig_reg(R_030900_VGT_ESGS_RING_SIZE,
                                sctx->esgs_ring->width0 / 256);
      }
      if (sctx->gsvs_ring) {
         radeon_set_uconfig_reg(R_030904_VGT_GSVS_RING_SIZE,
                                sctx->gsvs_ring->width0 / 256);
      }
      radeon_end();
      return true;
   }

   /* The codepath without register shadowing. */
   /* Create the "cs_preamble_gs_rings" state. */
   pm4 = CALLOC_STRUCT(si_pm4_state);
   if (!pm4)
      return false;

   if (sctx->chip_class >= GFX7) {
      if (sctx->esgs_ring) {
         assert(sctx->chip_class <= GFX8);
         si_pm4_set_reg(pm4, R_030900_VGT_ESGS_RING_SIZE, sctx->esgs_ring->width0 / 256);
      }
      if (sctx->gsvs_ring)
         si_pm4_set_reg(pm4, R_030904_VGT_GSVS_RING_SIZE, sctx->gsvs_ring->width0 / 256);
   } else {
      if (sctx->esgs_ring)
         si_pm4_set_reg(pm4, R_0088C8_VGT_ESGS_RING_SIZE, sctx->esgs_ring->width0 / 256);
      if (sctx->gsvs_ring)
         si_pm4_set_reg(pm4, R_0088CC_VGT_GSVS_RING_SIZE, sctx->gsvs_ring->width0 / 256);
   }

   /* Set the state. */
   if (sctx->cs_preamble_gs_rings)
      si_pm4_free_state(sctx, sctx->cs_preamble_gs_rings, ~0);
   sctx->cs_preamble_gs_rings = pm4;

   si_cs_preamble_add_vgt_flush(sctx);

   /* Flush the context to re-emit both cs_preamble states. */
   sctx->initial_gfx_cs_size = 0; /* force flush */
   si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);

   return true;
}

static void si_shader_lock(struct si_shader *shader)
{
   simple_mtx_lock(&shader->selector->mutex);
   if (shader->previous_stage_sel) {
      assert(shader->previous_stage_sel != shader->selector);
      simple_mtx_lock(&shader->previous_stage_sel->mutex);
   }
}

static void si_shader_unlock(struct si_shader *shader)
{
   if (shader->previous_stage_sel)
      simple_mtx_unlock(&shader->previous_stage_sel->mutex);
   simple_mtx_unlock(&shader->selector->mutex);
}

/**
 * @returns 1 if \p sel has been updated to use a new scratch buffer
 *          0 if not
 *          < 0 if there was a failure
 */
static int si_update_scratch_buffer(struct si_context *sctx, struct si_shader *shader)
{
   uint64_t scratch_va = sctx->scratch_buffer->gpu_address;

   if (!shader)
      return 0;

   /* This shader doesn't need a scratch buffer */
   if (shader->config.scratch_bytes_per_wave == 0)
      return 0;

   /* Prevent race conditions when updating:
    * - si_shader::scratch_bo
    * - si_shader::binary::code
    * - si_shader::previous_stage::binary::code.
    */
   si_shader_lock(shader);

   /* This shader is already configured to use the current
    * scratch buffer. */
   if (shader->scratch_bo == sctx->scratch_buffer) {
      si_shader_unlock(shader);
      return 0;
   }

   assert(sctx->scratch_buffer);

   /* Replace the shader bo with a new bo that has the relocs applied. */
   if (!si_shader_binary_upload(sctx->screen, shader, scratch_va)) {
      si_shader_unlock(shader);
      return -1;
   }

   /* Update the shader state to use the new shader bo. */
   si_shader_init_pm4_state(sctx->screen, shader);

   si_resource_reference(&shader->scratch_bo, sctx->scratch_buffer);

   si_shader_unlock(shader);
   return 1;
}

static struct si_shader *si_get_tcs_current(struct si_context *sctx)
{
   if (!sctx->shader.tes.cso)
      return NULL; /* tessellation disabled */

   return sctx->shader.tcs.cso ? sctx->shader.tcs.current : sctx->fixed_func_tcs_shader.current;
}

static bool si_update_scratch_relocs(struct si_context *sctx)
{
   struct si_shader *tcs = si_get_tcs_current(sctx);
   int r;

   /* Update the shaders, so that they are using the latest scratch.
    * The scratch buffer may have been changed since these shaders were
    * last used, so we still need to try to update them, even if they
    * require scratch buffers smaller than the current size.
    */
   r = si_update_scratch_buffer(sctx, sctx->shader.ps.current);
   if (r < 0)
      return false;
   if (r == 1)
      si_pm4_bind_state(sctx, ps, sctx->shader.ps.current);

   r = si_update_scratch_buffer(sctx, sctx->shader.gs.current);
   if (r < 0)
      return false;
   if (r == 1)
      si_pm4_bind_state(sctx, gs, sctx->shader.gs.current);

   r = si_update_scratch_buffer(sctx, tcs);
   if (r < 0)
      return false;
   if (r == 1)
      si_pm4_bind_state(sctx, hs, tcs);

   /* VS can be bound as LS, ES, or VS. */
   r = si_update_scratch_buffer(sctx, sctx->shader.vs.current);
   if (r < 0)
      return false;
   if (r == 1) {
      if (sctx->shader.vs.current->key.as_ls)
         si_pm4_bind_state(sctx, ls, sctx->shader.vs.current);
      else if (sctx->shader.vs.current->key.as_es)
         si_pm4_bind_state(sctx, es, sctx->shader.vs.current);
      else if (sctx->shader.vs.current->key.as_ngg)
         si_pm4_bind_state(sctx, gs, sctx->shader.vs.current);
      else
         si_pm4_bind_state(sctx, vs, sctx->shader.vs.current);
   }

   /* TES can be bound as ES or VS. */
   r = si_update_scratch_buffer(sctx, sctx->shader.tes.current);
   if (r < 0)
      return false;
   if (r == 1) {
      if (sctx->shader.tes.current->key.as_es)
         si_pm4_bind_state(sctx, es, sctx->shader.tes.current);
      else if (sctx->shader.tes.current->key.as_ngg)
         si_pm4_bind_state(sctx, gs, sctx->shader.tes.current);
      else
         si_pm4_bind_state(sctx, vs, sctx->shader.tes.current);
   }

   return true;
}

bool si_update_spi_tmpring_size(struct si_context *sctx, unsigned bytes)
{
   /* SPI_TMPRING_SIZE.WAVESIZE must be constant for each scratch buffer.
    * There are 2 cases to handle:
    *
    * - If the current needed size is less than the maximum seen size,
    *   use the maximum seen size, so that WAVESIZE remains the same.
    *
    * - If the current needed size is greater than the maximum seen size,
    *   the scratch buffer is reallocated, so we can increase WAVESIZE.
    *
    * Shaders that set SCRATCH_EN=0 don't allocate scratch space.
    * Otherwise, the number of waves that can use scratch is
    * SPI_TMPRING_SIZE.WAVES.
    */
   sctx->max_seen_scratch_bytes_per_wave = MAX2(sctx->max_seen_scratch_bytes_per_wave, bytes);

   unsigned scratch_needed_size = sctx->max_seen_scratch_bytes_per_wave * sctx->scratch_waves;
   unsigned spi_tmpring_size;

   if (scratch_needed_size > 0) {
      if (!sctx->scratch_buffer || scratch_needed_size > sctx->scratch_buffer->b.b.width0) {
         /* Create a bigger scratch buffer */
         si_resource_reference(&sctx->scratch_buffer, NULL);

         sctx->scratch_buffer = si_aligned_buffer_create(
            &sctx->screen->b,
            SI_RESOURCE_FLAG_UNMAPPABLE | SI_RESOURCE_FLAG_DRIVER_INTERNAL,
            PIPE_USAGE_DEFAULT, scratch_needed_size,
            sctx->screen->info.pte_fragment_size);
         if (!sctx->scratch_buffer)
            return false;

         si_context_add_resource_size(sctx, &sctx->scratch_buffer->b.b);
      }

      if (!si_update_scratch_relocs(sctx))
         return false;
   }

   /* The LLVM shader backend should be reporting aligned scratch_sizes. */
   assert((scratch_needed_size & ~0x3FF) == scratch_needed_size &&
          "scratch size should already be aligned correctly.");

   spi_tmpring_size = S_0286E8_WAVES(sctx->scratch_waves) |
                      S_0286E8_WAVESIZE(sctx->max_seen_scratch_bytes_per_wave >> 10);
   if (spi_tmpring_size != sctx->spi_tmpring_size) {
      sctx->spi_tmpring_size = spi_tmpring_size;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.scratch_state);
   }
   return true;
}

void si_init_tess_factor_ring(struct si_context *sctx)
{
   assert(!sctx->tess_rings);
   assert(((sctx->screen->tess_factor_ring_size / 4) & C_030938_SIZE) == 0);

   /* The address must be aligned to 2^19, because the shader only
    * receives the high 13 bits.
    */
   sctx->tess_rings = pipe_aligned_buffer_create(
      sctx->b.screen, SI_RESOURCE_FLAG_32BIT | SI_RESOURCE_FLAG_DRIVER_INTERNAL, PIPE_USAGE_DEFAULT,
      sctx->screen->tess_offchip_ring_size + sctx->screen->tess_factor_ring_size, 1 << 19);
   if (!sctx->tess_rings)
      return;

   if (sctx->screen->info.has_tmz_support) {
      sctx->tess_rings_tmz = pipe_aligned_buffer_create(
         sctx->b.screen,
         PIPE_RESOURCE_FLAG_ENCRYPTED | SI_RESOURCE_FLAG_32BIT | SI_RESOURCE_FLAG_DRIVER_INTERNAL,
         PIPE_USAGE_DEFAULT,
         sctx->screen->tess_offchip_ring_size + sctx->screen->tess_factor_ring_size, 1 << 19);
   }

   uint64_t factor_va =
      si_resource(sctx->tess_rings)->gpu_address + sctx->screen->tess_offchip_ring_size;

   if (sctx->shadowed_regs) {
      /* These registers will be shadowed, so set them only once. */
      /* TODO: tmz + shadowed_regs support */
      struct radeon_cmdbuf *cs = &sctx->gfx_cs;

      assert(sctx->chip_class >= GFX7);

      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, si_resource(sctx->tess_rings),
                                RADEON_USAGE_READWRITE, RADEON_PRIO_SHADER_RINGS);
      si_emit_vgt_flush(cs);

      /* Set tessellation registers. */
      radeon_begin(cs);
      radeon_set_uconfig_reg(R_030938_VGT_TF_RING_SIZE,
                             S_030938_SIZE(sctx->screen->tess_factor_ring_size / 4));
      radeon_set_uconfig_reg(R_030940_VGT_TF_MEMORY_BASE, factor_va >> 8);
      if (sctx->chip_class >= GFX10) {
         radeon_set_uconfig_reg(R_030984_VGT_TF_MEMORY_BASE_HI_UMD,
                                S_030984_BASE_HI(factor_va >> 40));
      } else if (sctx->chip_class == GFX9) {
         radeon_set_uconfig_reg(R_030944_VGT_TF_MEMORY_BASE_HI,
                                S_030944_BASE_HI(factor_va >> 40));
      }
      radeon_set_uconfig_reg(R_03093C_VGT_HS_OFFCHIP_PARAM,
                             sctx->screen->vgt_hs_offchip_param);
      radeon_end();
      return;
   }

   /* The codepath without register shadowing. */
   si_cs_preamble_add_vgt_flush(sctx);

   /* Append these registers to the init config state. */
   if (sctx->chip_class >= GFX7) {
      si_pm4_set_reg(sctx->cs_preamble_state, R_030938_VGT_TF_RING_SIZE,
                     S_030938_SIZE(sctx->screen->tess_factor_ring_size / 4));
      si_pm4_set_reg(sctx->cs_preamble_state, R_030940_VGT_TF_MEMORY_BASE, factor_va >> 8);
      if (sctx->chip_class >= GFX10)
         si_pm4_set_reg(sctx->cs_preamble_state, R_030984_VGT_TF_MEMORY_BASE_HI_UMD,
                        S_030984_BASE_HI(factor_va >> 40));
      else if (sctx->chip_class == GFX9)
         si_pm4_set_reg(sctx->cs_preamble_state, R_030944_VGT_TF_MEMORY_BASE_HI,
                        S_030944_BASE_HI(factor_va >> 40));
      si_pm4_set_reg(sctx->cs_preamble_state, R_03093C_VGT_HS_OFFCHIP_PARAM,
                     sctx->screen->vgt_hs_offchip_param);
   } else {
      struct si_pm4_state *pm4 = CALLOC_STRUCT(si_pm4_state);

      si_pm4_set_reg(pm4, R_008988_VGT_TF_RING_SIZE,
                     S_008988_SIZE(sctx->screen->tess_factor_ring_size / 4));
      si_pm4_set_reg(pm4, R_0089B8_VGT_TF_MEMORY_BASE, factor_va >> 8);
      si_pm4_set_reg(pm4, R_0089B0_VGT_HS_OFFCHIP_PARAM,
                     sctx->screen->vgt_hs_offchip_param);
      sctx->cs_preamble_tess_rings = pm4;

      if (sctx->screen->info.has_tmz_support) {
         pm4 = CALLOC_STRUCT(si_pm4_state);
         uint64_t factor_va_tmz =
            si_resource(sctx->tess_rings_tmz)->gpu_address + sctx->screen->tess_offchip_ring_size;
         si_pm4_set_reg(pm4, R_008988_VGT_TF_RING_SIZE,
                     S_008988_SIZE(sctx->screen->tess_factor_ring_size / 4));
         si_pm4_set_reg(pm4, R_0089B8_VGT_TF_MEMORY_BASE, factor_va_tmz >> 8);
         si_pm4_set_reg(pm4, R_0089B0_VGT_HS_OFFCHIP_PARAM,
                        sctx->screen->vgt_hs_offchip_param);
         sctx->cs_preamble_tess_rings_tmz = pm4;
      }
   }

   /* Flush the context to re-emit the cs_preamble state.
    * This is done only once in a lifetime of a context.
    */
   sctx->initial_gfx_cs_size = 0; /* force flush */
   si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
}

struct si_pm4_state *si_build_vgt_shader_config(struct si_screen *screen, union si_vgt_stages_key key)
{
   struct si_pm4_state *pm4 = CALLOC_STRUCT(si_pm4_state);
   uint32_t stages = 0;

   if (key.u.tess) {
      stages |= S_028B54_LS_EN(V_028B54_LS_STAGE_ON) | S_028B54_HS_EN(1) | S_028B54_DYNAMIC_HS(1);

      if (key.u.gs)
         stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_DS) | S_028B54_GS_EN(1);
      else if (key.u.ngg)
         stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_DS);
      else
         stages |= S_028B54_VS_EN(V_028B54_VS_STAGE_DS);
   } else if (key.u.gs) {
      stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_REAL) | S_028B54_GS_EN(1);
   } else if (key.u.ngg) {
      stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_REAL);
   }

   if (key.u.ngg) {
      stages |= S_028B54_PRIMGEN_EN(1) |
                S_028B54_NGG_WAVE_ID_EN(key.u.streamout) |
                S_028B54_PRIMGEN_PASSTHRU_EN(key.u.ngg_passthrough) |
                S_028B54_PRIMGEN_PASSTHRU_NO_MSG(key.u.ngg_passthrough &&
                                                 screen->info.family >= CHIP_DIMGREY_CAVEFISH);
   } else if (key.u.gs)
      stages |= S_028B54_VS_EN(V_028B54_VS_STAGE_COPY_SHADER);

   if (screen->info.chip_class >= GFX9)
      stages |= S_028B54_MAX_PRIMGRP_IN_WAVE(2);

   if (screen->info.chip_class >= GFX10 && screen->ge_wave_size == 32) {
      stages |= S_028B54_HS_W32_EN(1) |
                S_028B54_GS_W32_EN(key.u.ngg) | /* legacy GS only supports Wave64 */
                S_028B54_VS_W32_EN(1);
   }

   si_pm4_set_reg(pm4, R_028B54_VGT_SHADER_STAGES_EN, stages);
   return pm4;
}

static void si_emit_scratch_state(struct si_context *sctx)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   radeon_begin(cs);
   radeon_set_context_reg(R_0286E8_SPI_TMPRING_SIZE, sctx->spi_tmpring_size);
   radeon_end();

   if (sctx->scratch_buffer) {
      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, sctx->scratch_buffer, RADEON_USAGE_READWRITE,
                                RADEON_PRIO_SCRATCH_BUFFER);
   }
}

void si_init_screen_live_shader_cache(struct si_screen *sscreen)
{
   util_live_shader_cache_init(&sscreen->live_shader_cache, si_create_shader_selector,
                               si_destroy_shader_selector);
}

void si_init_shader_functions(struct si_context *sctx)
{
   sctx->atoms.s.scratch_state.emit = si_emit_scratch_state;

   sctx->b.create_vs_state = si_create_shader;
   sctx->b.create_tcs_state = si_create_shader;
   sctx->b.create_tes_state = si_create_shader;
   sctx->b.create_gs_state = si_create_shader;
   sctx->b.create_fs_state = si_create_shader;

   sctx->b.bind_vs_state = si_bind_vs_shader;
   sctx->b.bind_tcs_state = si_bind_tcs_shader;
   sctx->b.bind_tes_state = si_bind_tes_shader;
   sctx->b.bind_gs_state = si_bind_gs_shader;
   sctx->b.bind_fs_state = si_bind_ps_shader;

   sctx->b.delete_vs_state = si_delete_shader_selector;
   sctx->b.delete_tcs_state = si_delete_shader_selector;
   sctx->b.delete_tes_state = si_delete_shader_selector;
   sctx->b.delete_gs_state = si_delete_shader_selector;
   sctx->b.delete_fs_state = si_delete_shader_selector;
}
