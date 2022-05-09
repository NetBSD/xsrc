/*
 * Copyright © 2019 Raspberry Pi
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 *
 * based in part on radv driver which is:
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef V3DV_PRIVATE_H
#define V3DV_PRIVATE_H

#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>
#include <vk_enum_to_str.h>

#include "vk_device.h"
#include "vk_instance.h"
#include "vk_image.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_shader_module.h"
#include "vk_util.h"

#include "vk_command_buffer.h"
#include "vk_queue.h"

#include <xf86drm.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x) ((void)0)
#endif

#include "v3dv_limits.h"

#include "common/v3d_device_info.h"
#include "common/v3d_limits.h"
#include "common/v3d_tiling.h"
#include "common/v3d_util.h"

#include "compiler/shader_enums.h"
#include "compiler/spirv/nir_spirv.h"

#include "compiler/v3d_compiler.h"

#include "vk_debug_report.h"
#include "util/set.h"
#include "util/hash_table.h"
#include "util/xmlconfig.h"
#include "u_atomic.h"

#include "v3dv_entrypoints.h"
#include "v3dv_bo.h"

#include "drm-uapi/v3d_drm.h"

#include "vk_alloc.h"
#include "simulator/v3d_simulator.h"

#include "v3dv_cl.h"

#include "wsi_common.h"

/* A non-fatal assert.  Useful for debugging. */
#ifdef DEBUG
#define v3dv_assert(x) ({ \
   if (unlikely(!(x))) \
      fprintf(stderr, "%s:%d ASSERT: %s", __FILE__, __LINE__, #x); \
})
#else
#define v3dv_assert(x)
#endif

#define perf_debug(...) do {                       \
   if (unlikely(V3D_DEBUG & V3D_DEBUG_PERF))       \
      fprintf(stderr, __VA_ARGS__);                \
} while (0)

struct v3dv_instance;

#ifdef USE_V3D_SIMULATOR
#define using_v3d_simulator true
#else
#define using_v3d_simulator false
#endif

struct v3d_simulator_file;

/* Minimum required by the Vulkan 1.1 spec */
#define MAX_MEMORY_ALLOCATION_SIZE (1ull << 30)

struct v3dv_physical_device {
   struct vk_physical_device vk;

   char *name;
   int32_t render_fd;
   int32_t display_fd;
   int32_t master_fd;

   /* We need these because it is not clear how to detect
    * valid devids in a portable way
     */
   bool has_primary;
   bool has_render;

   dev_t primary_devid;
   dev_t render_devid;

   uint8_t driver_build_sha1[20];
   uint8_t pipeline_cache_uuid[VK_UUID_SIZE];
   uint8_t device_uuid[VK_UUID_SIZE];
   uint8_t driver_uuid[VK_UUID_SIZE];

   struct disk_cache *disk_cache;

   mtx_t mutex;

   struct wsi_device wsi_device;

   VkPhysicalDeviceMemoryProperties memory;

   struct v3d_device_info devinfo;

   struct v3d_simulator_file *sim_file;

   const struct v3d_compiler *compiler;
   uint32_t next_program_id;

   struct {
      bool merge_jobs;
   } options;
};

VkResult v3dv_physical_device_acquire_display(struct v3dv_instance *instance,
                                              struct v3dv_physical_device *pdevice,
                                              VkIcdSurfaceBase *surface);

VkResult v3dv_wsi_init(struct v3dv_physical_device *physical_device);
void v3dv_wsi_finish(struct v3dv_physical_device *physical_device);
struct v3dv_image *v3dv_wsi_get_image_from_swapchain(VkSwapchainKHR swapchain,
                                                     uint32_t index);

void v3dv_meta_clear_init(struct v3dv_device *device);
void v3dv_meta_clear_finish(struct v3dv_device *device);

void v3dv_meta_blit_init(struct v3dv_device *device);
void v3dv_meta_blit_finish(struct v3dv_device *device);

void v3dv_meta_texel_buffer_copy_init(struct v3dv_device *device);
void v3dv_meta_texel_buffer_copy_finish(struct v3dv_device *device);

bool v3dv_meta_can_use_tlb(struct v3dv_image *image,
                           const VkOffset3D *offset,
                           VkFormat *compat_format);

struct v3dv_instance {
   struct vk_instance vk;

   int physicalDeviceCount;
   struct v3dv_physical_device physicalDevice;

   bool pipeline_cache_enabled;
   bool default_pipeline_cache_enabled;
};

/* Tracks wait threads spawned from a single vkQueueSubmit call */
struct v3dv_queue_submit_wait_info {
   /*  struct vk_object_base base; ?*/
   struct list_head list_link;

   struct v3dv_device *device;

   /* List of wait threads spawned for any command buffers in a particular
    * call to vkQueueSubmit.
    */
   uint32_t wait_thread_count;
   struct {
      pthread_t thread;
      bool finished;
   } wait_threads[16];

   /* The master wait thread for the entire submit. This will wait for all
    * other threads in this submit to complete  before processing signal
    * semaphores and fences.
    */
   pthread_t master_wait_thread;

   /* List of semaphores (and fence) to signal after all wait threads completed
    * and all command buffer jobs in the submission have been sent to the GPU.
    */
   uint32_t signal_semaphore_count;
   VkSemaphore *signal_semaphores;
   VkFence fence;
};

struct v3dv_queue {
   struct vk_queue vk;

   struct v3dv_device *device;

   /* A list of active v3dv_queue_submit_wait_info */
   struct list_head submit_wait_list;

   /* A mutex to prevent concurrent access to the list of wait threads */
   mtx_t mutex;

   struct v3dv_job *noop_job;
};

#define V3DV_META_BLIT_CACHE_KEY_SIZE              (4 * sizeof(uint32_t))
#define V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE (3 * sizeof(uint32_t) + \
                                                    sizeof(VkComponentMapping))

struct v3dv_meta_color_clear_pipeline {
   VkPipeline pipeline;
   VkRenderPass pass;
   bool cached;
   uint64_t key;
};

struct v3dv_meta_depth_clear_pipeline {
   VkPipeline pipeline;
   uint64_t key;
};

struct v3dv_meta_blit_pipeline {
   VkPipeline pipeline;
   VkRenderPass pass;
   VkRenderPass pass_no_load;
   uint8_t key[V3DV_META_BLIT_CACHE_KEY_SIZE];
};

struct v3dv_meta_texel_buffer_copy_pipeline {
   VkPipeline pipeline;
   VkRenderPass pass;
   VkRenderPass pass_no_load;
   uint8_t key[V3DV_META_TEXEL_BUFFER_COPY_CACHE_KEY_SIZE];
};

struct v3dv_pipeline_key {
   bool robust_buffer_access;
   uint8_t topology;
   uint8_t logicop_func;
   bool msaa;
   bool sample_coverage;
   bool sample_alpha_to_coverage;
   bool sample_alpha_to_one;
   uint8_t cbufs;
   struct {
      enum pipe_format format;
      const uint8_t *swizzle;
   } color_fmt[V3D_MAX_DRAW_BUFFERS];
   uint8_t f32_color_rb;
   uint32_t va_swap_rb_mask;
   bool has_multiview;
};

struct v3dv_pipeline_cache_stats {
   uint32_t miss;
   uint32_t hit;
   uint32_t count;
};

/* Equivalent to gl_shader_stage, but including the coordinate shaders
 *
 * FIXME: perhaps move to common
 */
enum broadcom_shader_stage {
   BROADCOM_SHADER_VERTEX,
   BROADCOM_SHADER_VERTEX_BIN,
   BROADCOM_SHADER_GEOMETRY,
   BROADCOM_SHADER_GEOMETRY_BIN,
   BROADCOM_SHADER_FRAGMENT,
   BROADCOM_SHADER_COMPUTE,
};

#define BROADCOM_SHADER_STAGES (BROADCOM_SHADER_COMPUTE + 1)

/* Assumes that coordinate shaders will be custom-handled by the caller */
static inline enum broadcom_shader_stage
gl_shader_stage_to_broadcom(gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      return BROADCOM_SHADER_VERTEX;
   case MESA_SHADER_GEOMETRY:
      return BROADCOM_SHADER_GEOMETRY;
   case MESA_SHADER_FRAGMENT:
      return BROADCOM_SHADER_FRAGMENT;
   case MESA_SHADER_COMPUTE:
      return BROADCOM_SHADER_COMPUTE;
   default:
      unreachable("Unknown gl shader stage");
   }
}

static inline gl_shader_stage
broadcom_shader_stage_to_gl(enum broadcom_shader_stage stage)
{
   switch (stage) {
   case BROADCOM_SHADER_VERTEX:
   case BROADCOM_SHADER_VERTEX_BIN:
      return MESA_SHADER_VERTEX;
   case BROADCOM_SHADER_GEOMETRY:
   case BROADCOM_SHADER_GEOMETRY_BIN:
      return MESA_SHADER_GEOMETRY;
   case BROADCOM_SHADER_FRAGMENT:
      return MESA_SHADER_FRAGMENT;
   case BROADCOM_SHADER_COMPUTE:
      return MESA_SHADER_COMPUTE;
   default:
      unreachable("Unknown broadcom shader stage");
   }
}

static inline bool
broadcom_shader_stage_is_binning(enum broadcom_shader_stage stage)
{
   switch (stage) {
   case BROADCOM_SHADER_VERTEX_BIN:
   case BROADCOM_SHADER_GEOMETRY_BIN:
      return true;
   default:
      return false;
   }
}

static inline bool
broadcom_shader_stage_is_render_with_binning(enum broadcom_shader_stage stage)
{
   switch (stage) {
   case BROADCOM_SHADER_VERTEX:
   case BROADCOM_SHADER_GEOMETRY:
      return true;
   default:
      return false;
   }
}

static inline enum broadcom_shader_stage
broadcom_binning_shader_stage_for_render_stage(enum broadcom_shader_stage stage)
{
   switch (stage) {
   case BROADCOM_SHADER_VERTEX:
      return BROADCOM_SHADER_VERTEX_BIN;
   case BROADCOM_SHADER_GEOMETRY:
      return BROADCOM_SHADER_GEOMETRY_BIN;
   default:
      unreachable("Invalid shader stage");
   }
}

static inline const char *
broadcom_shader_stage_name(enum broadcom_shader_stage stage)
{
   switch(stage) {
   case BROADCOM_SHADER_VERTEX_BIN:
      return "MESA_SHADER_VERTEX_BIN";
   case BROADCOM_SHADER_GEOMETRY_BIN:
      return "MESA_SHADER_GEOMETRY_BIN";
   default:
      return gl_shader_stage_name(broadcom_shader_stage_to_gl(stage));
   }
}

struct v3dv_pipeline_cache {
   struct vk_object_base base;

   struct v3dv_device *device;
   mtx_t mutex;

   struct hash_table *nir_cache;
   struct v3dv_pipeline_cache_stats nir_stats;

   struct hash_table *cache;
   struct v3dv_pipeline_cache_stats stats;

   /* For VK_EXT_pipeline_creation_cache_control. */
   bool externally_synchronized;
};

struct v3dv_device {
   struct vk_device vk;

   struct v3dv_instance *instance;
   struct v3dv_physical_device *pdevice;

   struct v3d_device_info devinfo;
   struct v3dv_queue queue;

   /* A sync object to track the last job submitted to the GPU. */
   uint32_t last_job_sync;

   /* A mutex to prevent concurrent access to last_job_sync from the queue */
   mtx_t mutex;

   /* Resources used for meta operations */
   struct {
      mtx_t mtx;
      struct {
         VkPipelineLayout p_layout;
         struct hash_table *cache; /* v3dv_meta_color_clear_pipeline */
      } color_clear;
      struct {
         VkPipelineLayout p_layout;
         struct hash_table *cache; /* v3dv_meta_depth_clear_pipeline */
      } depth_clear;
      struct {
         VkDescriptorSetLayout ds_layout;
         VkPipelineLayout p_layout;
         struct hash_table *cache[3]; /* v3dv_meta_blit_pipeline for 1d, 2d, 3d */
      } blit;
      struct {
         VkDescriptorSetLayout ds_layout;
         VkPipelineLayout p_layout;
         struct hash_table *cache[3]; /* v3dv_meta_texel_buffer_copy_pipeline for 1d, 2d, 3d */
      } texel_buffer_copy;
   } meta;

   struct v3dv_bo_cache {
      /** List of struct v3d_bo freed, by age. */
      struct list_head time_list;
      /** List of struct v3d_bo freed, per size, by age. */
      struct list_head *size_list;
      uint32_t size_list_size;

      mtx_t lock;

      uint32_t cache_size;
      uint32_t cache_count;
      uint32_t max_cache_size;
   } bo_cache;

   uint32_t bo_size;
   uint32_t bo_count;

   struct v3dv_pipeline_cache default_pipeline_cache;

   /* GL_SHADER_STATE_RECORD needs to speficy default attribute values. The
    * following covers the most common case, that is all attributes format
    * being float being float, allowing us to reuse the same BO for all
    * pipelines matching this requirement. Pipelines that need integer
    * attributes will create their own BO.
    */
   struct v3dv_bo *default_attribute_float;
   VkPhysicalDeviceFeatures features;
};

struct v3dv_device_memory {
   struct vk_object_base base;

   struct v3dv_bo *bo;
   const VkMemoryType *type;
   bool has_bo_ownership;
   bool is_for_wsi;
};

#define V3D_OUTPUT_IMAGE_FORMAT_NO 255
#define TEXTURE_DATA_FORMAT_NO     255

struct v3dv_format {
   bool supported;

   /* One of V3D33_OUTPUT_IMAGE_FORMAT_*, or OUTPUT_IMAGE_FORMAT_NO */
   uint8_t rt_type;

   /* One of V3D33_TEXTURE_DATA_FORMAT_*. */
   uint8_t tex_type;

   /* Swizzle to apply to the RGBA shader output for storing to the tile
    * buffer, to the RGBA tile buffer to produce shader input (for
    * blending), and for turning the rgba8888 texture sampler return
    * value into shader rgba values.
    */
   uint8_t swizzle[4];

   /* Whether the return value is 16F/I/UI or 32F/I/UI. */
   uint8_t return_size;

   /* If the format supports (linear) filtering when texturing. */
   bool supports_filtering;
};

struct v3d_resource_slice {
   uint32_t offset;
   uint32_t stride;
   uint32_t padded_height;
   /* Size of a single pane of the slice.  For 3D textures, there will be
    * a number of panes equal to the minified, power-of-two-aligned
    * depth.
    */
   uint32_t size;
   uint8_t ub_pad;
   enum v3d_tiling_mode tiling;
   uint32_t padded_height_of_output_image_in_uif_blocks;
};

struct v3dv_image {
   struct vk_image vk;

   const struct v3dv_format *format;
   uint32_t cpp;
   bool tiled;

   struct v3d_resource_slice slices[V3D_MAX_MIP_LEVELS];
   uint64_t size; /* Total size in bytes */
   uint32_t cube_map_stride;

   struct v3dv_device_memory *mem;
   VkDeviceSize mem_offset;
   uint32_t alignment;
};

VkImageViewType v3dv_image_type_to_view_type(VkImageType type);

/* Pre-generating packets needs to consider changes in packet sizes across hw
 * versions. Keep things simple and allocate enough space for any supported
 * version. We ensure the size is large enough through static asserts.
 */
#define V3DV_TEXTURE_SHADER_STATE_LENGTH 32
#define V3DV_SAMPLER_STATE_LENGTH 24
#define V3DV_BLEND_CFG_LENGTH 5
#define V3DV_CFG_BITS_LENGTH 4
#define V3DV_GL_SHADER_STATE_RECORD_LENGTH 36
#define V3DV_VCM_CACHE_SIZE_LENGTH 2
#define V3DV_GL_SHADER_STATE_ATTRIBUTE_RECORD_LENGTH 16
#define V3DV_STENCIL_CFG_LENGTH 6

struct v3dv_image_view {
   struct vk_image_view vk;

   const struct v3dv_format *format;
   bool swap_rb;
   uint32_t internal_bpp;
   uint32_t internal_type;
   uint32_t offset;

   /* Precomputed (composed from createinfo->components and formar swizzle)
    * swizzles to pass in to the shader key.
    *
    * This could be also included on the descriptor bo, but the shader state
    * packet doesn't need it on a bo, so we can just avoid a memory copy
    */
   uint8_t swizzle[4];

   /* Prepacked TEXTURE_SHADER_STATE. It will be copied to the descriptor info
    * during UpdateDescriptorSets.
    *
    * Empirical tests show that cube arrays need a different shader state
    * depending on whether they are used with a sampler or not, so for these
    * we generate two states and select the one to use based on the descriptor
    * type.
    */
   uint8_t texture_shader_state[2][V3DV_TEXTURE_SHADER_STATE_LENGTH];
};

uint32_t v3dv_layer_offset(const struct v3dv_image *image, uint32_t level, uint32_t layer);

struct v3dv_buffer {
   struct vk_object_base base;

   VkDeviceSize size;
   VkBufferUsageFlags usage;
   uint32_t alignment;

   struct v3dv_device_memory *mem;
   VkDeviceSize mem_offset;
};

struct v3dv_buffer_view {
   struct vk_object_base base;

   struct v3dv_buffer *buffer;

   VkFormat vk_format;
   const struct v3dv_format *format;
   uint32_t internal_bpp;
   uint32_t internal_type;

   uint32_t offset;
   uint32_t size;
   uint32_t num_elements;

   /* Prepacked TEXTURE_SHADER_STATE. */
   uint8_t texture_shader_state[V3DV_TEXTURE_SHADER_STATE_LENGTH];
};

struct v3dv_subpass_attachment {
   uint32_t attachment;
   VkImageLayout layout;
};

struct v3dv_subpass {
   uint32_t input_count;
   struct v3dv_subpass_attachment *input_attachments;

   uint32_t color_count;
   struct v3dv_subpass_attachment *color_attachments;
   struct v3dv_subpass_attachment *resolve_attachments;

   struct v3dv_subpass_attachment ds_attachment;

   /* If we need to emit the clear of the depth/stencil attachment using a
    * a draw call instead of using the TLB (GFXH-1461).
    */
   bool do_depth_clear_with_draw;
   bool do_stencil_clear_with_draw;

   /* Multiview */
   uint32_t view_mask;
};

struct v3dv_render_pass_attachment {
   VkAttachmentDescription desc;

   uint32_t first_subpass;
   uint32_t last_subpass;

   /* When multiview is enabled, we no longer care about when a particular
    * attachment is first or last used in a render pass, since not all views
    * in the attachment will meet that criteria. Instead, we need to track
    * each individual view (layer) in each attachment and emit our stores,
    * loads and clears accordingly.
    */
   struct {
      uint32_t first_subpass;
      uint32_t last_subpass;
   } views[MAX_MULTIVIEW_VIEW_COUNT];

   /* If this is a multismapled attachment that is going to be resolved,
    * whether we can use the TLB resolve on store.
    */
   bool use_tlb_resolve;
};

struct v3dv_render_pass {
   struct vk_object_base base;

   bool multiview_enabled;

   uint32_t attachment_count;
   struct v3dv_render_pass_attachment *attachments;

   uint32_t subpass_count;
   struct v3dv_subpass *subpasses;

   struct v3dv_subpass_attachment *subpass_attachments;
};

struct v3dv_framebuffer {
   struct vk_object_base base;

   uint32_t width;
   uint32_t height;
   uint32_t layers;

   /* Typically, edge tiles in the framebuffer have padding depending on the
    * underlying tiling layout. One consequnce of this is that when the
    * framebuffer dimensions are not aligned to tile boundaries, tile stores
    * would still write full tiles on the edges and write to the padded area.
    * If the framebuffer is aliasing a smaller region of a larger image, then
    * we need to be careful with this though, as we won't have padding on the
    * edge tiles (which typically means that we need to load the tile buffer
    * before we store).
    */
   bool has_edge_padding;

   uint32_t attachment_count;
   uint32_t color_attachment_count;
   struct v3dv_image_view *attachments[0];
};

struct v3dv_frame_tiling {
   uint32_t width;
   uint32_t height;
   uint32_t layers;
   uint32_t render_target_count;
   uint32_t internal_bpp;
   bool     msaa;
   uint32_t tile_width;
   uint32_t tile_height;
   uint32_t draw_tiles_x;
   uint32_t draw_tiles_y;
   uint32_t supertile_width;
   uint32_t supertile_height;
   uint32_t frame_width_in_supertiles;
   uint32_t frame_height_in_supertiles;
};

void v3dv_framebuffer_compute_internal_bpp_msaa(const struct v3dv_framebuffer *framebuffer,
                                                const struct v3dv_subpass *subpass,
                                                uint8_t *max_bpp, bool *msaa);

bool v3dv_subpass_area_is_tile_aligned(struct v3dv_device *device,
                                       const VkRect2D *area,
                                       struct v3dv_framebuffer *fb,
                                       struct v3dv_render_pass *pass,
                                       uint32_t subpass_idx);

struct v3dv_cmd_pool {
   struct vk_object_base base;

   VkAllocationCallbacks alloc;
   struct list_head cmd_buffers;
};

enum v3dv_cmd_buffer_status {
   V3DV_CMD_BUFFER_STATUS_NEW           = 0,
   V3DV_CMD_BUFFER_STATUS_INITIALIZED   = 1,
   V3DV_CMD_BUFFER_STATUS_RECORDING     = 2,
   V3DV_CMD_BUFFER_STATUS_EXECUTABLE    = 3
};

union v3dv_clear_value {
   uint32_t color[4];
   struct {
      float z;
      uint8_t s;
   };
};

struct v3dv_cmd_buffer_attachment_state {
   /* The original clear value as provided by the Vulkan API */
   VkClearValue vk_clear_value;

   /* The hardware clear value */
   union v3dv_clear_value clear_value;
};

struct v3dv_viewport_state {
   uint32_t count;
   VkViewport viewports[MAX_VIEWPORTS];
   float translate[MAX_VIEWPORTS][3];
   float scale[MAX_VIEWPORTS][3];
};

struct v3dv_scissor_state {
   uint32_t count;
   VkRect2D scissors[MAX_SCISSORS];
};

/* Mostly a v3dv mapping of VkDynamicState, used to track which data as
 * defined as dynamic
 */
enum v3dv_dynamic_state_bits {
   V3DV_DYNAMIC_VIEWPORT                  = 1 << 0,
   V3DV_DYNAMIC_SCISSOR                   = 1 << 1,
   V3DV_DYNAMIC_STENCIL_COMPARE_MASK      = 1 << 2,
   V3DV_DYNAMIC_STENCIL_WRITE_MASK        = 1 << 3,
   V3DV_DYNAMIC_STENCIL_REFERENCE         = 1 << 4,
   V3DV_DYNAMIC_BLEND_CONSTANTS           = 1 << 5,
   V3DV_DYNAMIC_DEPTH_BIAS                = 1 << 6,
   V3DV_DYNAMIC_LINE_WIDTH                = 1 << 7,
   V3DV_DYNAMIC_COLOR_WRITE_ENABLE        = 1 << 8,
   V3DV_DYNAMIC_ALL                       = (1 << 9) - 1,
};

/* Flags for dirty pipeline state.
 */
enum v3dv_cmd_dirty_bits {
   V3DV_CMD_DIRTY_VIEWPORT                  = 1 << 0,
   V3DV_CMD_DIRTY_SCISSOR                   = 1 << 1,
   V3DV_CMD_DIRTY_STENCIL_COMPARE_MASK      = 1 << 2,
   V3DV_CMD_DIRTY_STENCIL_WRITE_MASK        = 1 << 3,
   V3DV_CMD_DIRTY_STENCIL_REFERENCE         = 1 << 4,
   V3DV_CMD_DIRTY_PIPELINE                  = 1 << 5,
   V3DV_CMD_DIRTY_COMPUTE_PIPELINE          = 1 << 6,
   V3DV_CMD_DIRTY_VERTEX_BUFFER             = 1 << 7,
   V3DV_CMD_DIRTY_INDEX_BUFFER              = 1 << 8,
   V3DV_CMD_DIRTY_DESCRIPTOR_SETS           = 1 << 9,
   V3DV_CMD_DIRTY_COMPUTE_DESCRIPTOR_SETS   = 1 << 10,
   V3DV_CMD_DIRTY_PUSH_CONSTANTS            = 1 << 11,
   V3DV_CMD_DIRTY_BLEND_CONSTANTS           = 1 << 12,
   V3DV_CMD_DIRTY_OCCLUSION_QUERY           = 1 << 13,
   V3DV_CMD_DIRTY_DEPTH_BIAS                = 1 << 14,
   V3DV_CMD_DIRTY_LINE_WIDTH                = 1 << 15,
   V3DV_CMD_DIRTY_VIEW_INDEX                = 1 << 16,
   V3DV_CMD_DIRTY_COLOR_WRITE_ENABLE        = 1 << 17,
};

struct v3dv_dynamic_state {
   /**
    * Bitmask of (1 << VK_DYNAMIC_STATE_*).
    * Defines the set of saved dynamic state.
    */
   uint32_t mask;

   struct v3dv_viewport_state viewport;

   struct v3dv_scissor_state scissor;

   struct {
      uint32_t front;
      uint32_t back;
   } stencil_compare_mask;

   struct {
      uint32_t front;
      uint32_t back;
   } stencil_write_mask;

   struct {
      uint32_t front;
      uint32_t back;
   } stencil_reference;

   float blend_constants[4];

   struct {
      float constant_factor;
      float depth_bias_clamp;
      float slope_factor;
   } depth_bias;

   float line_width;

   uint32_t color_write_enable;
};

extern const struct v3dv_dynamic_state default_dynamic_state;

void v3dv_viewport_compute_xform(const VkViewport *viewport,
                                 float scale[3],
                                 float translate[3]);

enum v3dv_ez_state {
   V3D_EZ_UNDECIDED = 0,
   V3D_EZ_GT_GE,
   V3D_EZ_LT_LE,
   V3D_EZ_DISABLED,
};

enum v3dv_job_type {
   V3DV_JOB_TYPE_GPU_CL = 0,
   V3DV_JOB_TYPE_GPU_CL_SECONDARY,
   V3DV_JOB_TYPE_GPU_TFU,
   V3DV_JOB_TYPE_GPU_CSD,
   V3DV_JOB_TYPE_CPU_RESET_QUERIES,
   V3DV_JOB_TYPE_CPU_END_QUERY,
   V3DV_JOB_TYPE_CPU_COPY_QUERY_RESULTS,
   V3DV_JOB_TYPE_CPU_SET_EVENT,
   V3DV_JOB_TYPE_CPU_WAIT_EVENTS,
   V3DV_JOB_TYPE_CPU_COPY_BUFFER_TO_IMAGE,
   V3DV_JOB_TYPE_CPU_CSD_INDIRECT,
   V3DV_JOB_TYPE_CPU_TIMESTAMP_QUERY,
};

struct v3dv_reset_query_cpu_job_info {
   struct v3dv_query_pool *pool;
   uint32_t first;
   uint32_t count;
};

struct v3dv_end_query_cpu_job_info {
   struct v3dv_query_pool *pool;
   uint32_t query;

   /* This is one unless multiview is used */
   uint32_t count;
};

struct v3dv_copy_query_results_cpu_job_info {
   struct v3dv_query_pool *pool;
   uint32_t first;
   uint32_t count;
   struct v3dv_buffer *dst;
   uint32_t offset;
   uint32_t stride;
   VkQueryResultFlags flags;
};

struct v3dv_event_set_cpu_job_info {
   struct v3dv_event *event;
   int state;
};

struct v3dv_event_wait_cpu_job_info {
   /* List of events to wait on */
   uint32_t event_count;
   struct v3dv_event **events;

   /* Whether any postponed jobs after the wait should wait on semaphores */
   bool sem_wait;
};

struct v3dv_copy_buffer_to_image_cpu_job_info {
   struct v3dv_image *image;
   struct v3dv_buffer *buffer;
   uint32_t buffer_offset;
   uint32_t buffer_stride;
   uint32_t buffer_layer_stride;
   VkOffset3D image_offset;
   VkExtent3D image_extent;
   uint32_t mip_level;
   uint32_t base_layer;
   uint32_t layer_count;
};

struct v3dv_csd_indirect_cpu_job_info {
   struct v3dv_buffer *buffer;
   uint32_t offset;
   struct v3dv_job *csd_job;
   uint32_t wg_size;
   uint32_t *wg_uniform_offsets[3];
   bool needs_wg_uniform_rewrite;
};

struct v3dv_timestamp_query_cpu_job_info {
   struct v3dv_query_pool *pool;
   uint32_t query;

   /* This is one unless multiview is used */
   uint32_t count;
};

struct v3dv_job {
   struct list_head list_link;

   /* We only create job clones when executing secondary command buffers into
    * primaries. These clones don't make deep copies of the original object
    * so we want to flag them to avoid freeing resources they don't own.
    */
   bool is_clone;

   enum v3dv_job_type type;

   struct v3dv_device *device;

   struct v3dv_cmd_buffer *cmd_buffer;

   struct v3dv_cl bcl;
   struct v3dv_cl rcl;
   struct v3dv_cl indirect;

   /* Set of all BOs referenced by the job. This will be used for making
    * the list of BOs that the kernel will need to have paged in to
    * execute our job.
    */
   struct set *bos;
   uint32_t bo_count;
   uint64_t bo_handle_mask;

   struct v3dv_bo *tile_alloc;
   struct v3dv_bo *tile_state;

   bool tmu_dirty_rcl;

   uint32_t first_subpass;

   /* When the current subpass is split into multiple jobs, this flag is set
    * to true for any jobs after the first in the same subpass.
    */
   bool is_subpass_continue;

   /* If this job is the last job emitted for a subpass. */
   bool is_subpass_finish;

   struct v3dv_frame_tiling frame_tiling;

   enum v3dv_ez_state ez_state;
   enum v3dv_ez_state first_ez_state;

   /* If we have already decided if we need to disable Early Z/S completely
    * for this job.
    */
   bool decided_global_ez_enable;

   /* If this job has been configured to use early Z/S clear */
   bool early_zs_clear;

   /* Number of draw calls recorded into the job */
   uint32_t draw_count;

   /* A flag indicating whether we want to flush every draw separately. This
    * can be used for debugging, or for cases where special circumstances
    * require this behavior.
    */
   bool always_flush;

   /* Whether we need to serialize this job in our command stream */
   bool serialize;

   /* If this is a CL job, whether we should sync before binning */
   bool needs_bcl_sync;

   /* Job specs for CPU jobs */
   union {
      struct v3dv_reset_query_cpu_job_info          query_reset;
      struct v3dv_end_query_cpu_job_info            query_end;
      struct v3dv_copy_query_results_cpu_job_info   query_copy_results;
      struct v3dv_event_set_cpu_job_info            event_set;
      struct v3dv_event_wait_cpu_job_info           event_wait;
      struct v3dv_copy_buffer_to_image_cpu_job_info copy_buffer_to_image;
      struct v3dv_csd_indirect_cpu_job_info         csd_indirect;
      struct v3dv_timestamp_query_cpu_job_info      query_timestamp;
   } cpu;

   /* Job specs for TFU jobs */
   struct drm_v3d_submit_tfu tfu;

   /* Job specs for CSD jobs */
   struct {
      struct v3dv_bo *shared_memory;
      uint32_t wg_count[3];
      uint32_t wg_base[3];
      struct drm_v3d_submit_csd submit;
   } csd;
};

void v3dv_job_init(struct v3dv_job *job,
                   enum v3dv_job_type type,
                   struct v3dv_device *device,
                   struct v3dv_cmd_buffer *cmd_buffer,
                   int32_t subpass_idx);
void v3dv_job_destroy(struct v3dv_job *job);

void v3dv_job_add_bo(struct v3dv_job *job, struct v3dv_bo *bo);
void v3dv_job_add_bo_unchecked(struct v3dv_job *job, struct v3dv_bo *bo);

void v3dv_job_start_frame(struct v3dv_job *job,
                          uint32_t width,
                          uint32_t height,
                          uint32_t layers,
                          bool allocate_tile_state_for_all_layers,
                          uint32_t render_target_count,
                          uint8_t max_internal_bpp,
                          bool msaa);

struct v3dv_job *
v3dv_job_clone_in_cmd_buffer(struct v3dv_job *job,
                             struct v3dv_cmd_buffer *cmd_buffer);

struct v3dv_job *v3dv_cmd_buffer_create_cpu_job(struct v3dv_device *device,
                                                enum v3dv_job_type type,
                                                struct v3dv_cmd_buffer *cmd_buffer,
                                                uint32_t subpass_idx);

void
v3dv_cmd_buffer_ensure_array_state(struct v3dv_cmd_buffer *cmd_buffer,
                                   uint32_t slot_size,
                                   uint32_t used_count,
                                   uint32_t *alloc_count,
                                   void **ptr);

void v3dv_cmd_buffer_emit_pre_draw(struct v3dv_cmd_buffer *cmd_buffer);

/* FIXME: only used on v3dv_cmd_buffer and v3dvx_cmd_buffer, perhaps move to a
 * cmd_buffer specific header?
 */
struct v3dv_draw_info {
   uint32_t vertex_count;
   uint32_t instance_count;
   uint32_t first_vertex;
   uint32_t first_instance;
};

struct v3dv_vertex_binding {
   struct v3dv_buffer *buffer;
   VkDeviceSize offset;
};

struct v3dv_descriptor_state {
   struct v3dv_descriptor_set *descriptor_sets[MAX_SETS];
   uint32_t valid;
   uint32_t dynamic_offsets[MAX_DYNAMIC_BUFFERS];
};

struct v3dv_cmd_pipeline_state {
   struct v3dv_pipeline *pipeline;

   struct v3dv_descriptor_state descriptor_state;
};

struct v3dv_cmd_buffer_state {
   struct v3dv_render_pass *pass;
   struct v3dv_framebuffer *framebuffer;
   VkRect2D render_area;

   /* Current job being recorded */
   struct v3dv_job *job;

   uint32_t subpass_idx;

   struct v3dv_cmd_pipeline_state gfx;
   struct v3dv_cmd_pipeline_state compute;

   struct v3dv_dynamic_state dynamic;

   uint32_t dirty;
   VkShaderStageFlagBits dirty_descriptor_stages;
   VkShaderStageFlagBits dirty_push_constants_stages;

   /* Current clip window. We use this to check whether we have an active
    * scissor, since in that case we can't use TLB clears and need to fallback
    * to drawing rects.
    */
   VkRect2D clip_window;

   /* Whether our render area is aligned to tile boundaries. If this is false
    * then we have tiles that are only partially covered by the render area,
    * and therefore, we need to be careful with our loads and stores so we don't
    * modify pixels for the tile area that is not covered by the render area.
    * This means, for example, that we can't use the TLB to clear, since that
    * always clears full tiles.
    */
   bool tile_aligned_render_area;

   uint32_t attachment_alloc_count;
   struct v3dv_cmd_buffer_attachment_state *attachments;

   struct v3dv_vertex_binding vertex_bindings[MAX_VBS];

   struct {
      VkBuffer buffer;
      VkDeviceSize offset;
      uint8_t index_size;
   } index_buffer;

   /* Current uniforms */
   struct {
      struct v3dv_cl_reloc vs_bin;
      struct v3dv_cl_reloc vs;
      struct v3dv_cl_reloc gs_bin;
      struct v3dv_cl_reloc gs;
      struct v3dv_cl_reloc fs;
   } uniforms;

   /* Current view index for multiview rendering */
   uint32_t view_index;

   /* Used to flag OOM conditions during command buffer recording */
   bool oom;

   /* Whether we have recorded a pipeline barrier that we still need to
    * process.
    */
   bool has_barrier;
   bool has_bcl_barrier;

   /* Secondary command buffer state */
   struct {
      bool occlusion_query_enable;
   } inheritance;

   /* Command buffer state saved during a meta operation */
   struct {
      uint32_t subpass_idx;
      VkRenderPass pass;
      VkFramebuffer framebuffer;

      uint32_t attachment_alloc_count;
      uint32_t attachment_count;
      struct v3dv_cmd_buffer_attachment_state *attachments;

      bool tile_aligned_render_area;
      VkRect2D render_area;

      struct v3dv_dynamic_state dynamic;

      struct v3dv_cmd_pipeline_state gfx;
      bool has_descriptor_state;

      uint32_t push_constants[MAX_PUSH_CONSTANTS_SIZE / 4];
   } meta;

   /* Command buffer state for queries */
   struct {
      /* A list of vkCmdQueryEnd commands recorded in the command buffer during
       * a render pass. We queue these here and then schedule the corresponding
       * CPU jobs for them at the time we finish the GPU job in which they have
       * been recorded.
       */
      struct {
         uint32_t used_count;
         uint32_t alloc_count;
         struct v3dv_end_query_cpu_job_info *states;
      } end;

      /* This BO is not NULL if we have an active query, that is, we have
       * called vkCmdBeginQuery but not vkCmdEndQuery.
       */
      struct {
         struct v3dv_bo *bo;
         uint32_t offset;
      } active_query;
   } query;
};

/* The following struct represents the info from a descriptor that we store on
 * the host memory. They are mostly links to other existing vulkan objects,
 * like the image_view in order to access to swizzle info, or the buffer used
 * for a UBO/SSBO, for example.
 *
 * FIXME: revisit if makes sense to just move everything that would be needed
 * from a descriptor to the bo.
 */
struct v3dv_descriptor {
   VkDescriptorType type;

   union {
      struct {
         struct v3dv_image_view *image_view;
         struct v3dv_sampler *sampler;
      };

      struct {
         struct v3dv_buffer *buffer;
         uint32_t offset;
         uint32_t range;
      };

      struct v3dv_buffer_view *buffer_view;
   };
};

struct v3dv_query {
   bool maybe_available;
   union {
      /* Used by GPU queries (occlusion) */
      struct {
         struct v3dv_bo *bo;
         uint32_t offset;
      };
      /* Used by CPU queries (timestamp) */
      uint64_t value;
   };
};

struct v3dv_query_pool {
   struct vk_object_base base;

   struct v3dv_bo *bo; /* Only used with GPU queries (occlusion) */

   VkQueryType query_type;
   uint32_t query_count;
   struct v3dv_query *queries;
};

VkResult v3dv_get_query_pool_results_cpu(struct v3dv_device *device,
                                         struct v3dv_query_pool *pool,
                                         uint32_t first,
                                         uint32_t count,
                                         void *data,
                                         VkDeviceSize stride,
                                         VkQueryResultFlags flags);

typedef void (*v3dv_cmd_buffer_private_obj_destroy_cb)(VkDevice device,
                                                       uint64_t pobj,
                                                       VkAllocationCallbacks *alloc);
struct v3dv_cmd_buffer_private_obj {
   struct list_head list_link;
   uint64_t obj;
   v3dv_cmd_buffer_private_obj_destroy_cb destroy_cb;
};

struct v3dv_cmd_buffer {
   struct vk_command_buffer vk;

   struct v3dv_device *device;

   struct v3dv_cmd_pool *pool;
   struct list_head pool_link;

   /* Used at submit time to link command buffers in the submission that have
    * spawned wait threads, so we can then wait on all of them to complete
    * before we process any signal sempahores or fences.
    */
   struct list_head list_link;

   VkCommandBufferUsageFlags usage_flags;
   VkCommandBufferLevel level;

   enum v3dv_cmd_buffer_status status;

   struct v3dv_cmd_buffer_state state;

   /* FIXME: we have just one client-side and bo for the push constants,
    * independently of the stageFlags in vkCmdPushConstants, and the
    * pipelineBindPoint in vkCmdBindPipeline. We could probably do more stage
    * tunning in the future if it makes sense.
    */
   uint32_t push_constants_data[MAX_PUSH_CONSTANTS_SIZE / 4];
   struct v3dv_cl_reloc push_constants_resource;

   /* Collection of Vulkan objects created internally by the driver (typically
    * during recording of meta operations) that are part of the command buffer
    * and should be destroyed with it.
    */
   struct list_head private_objs; /* v3dv_cmd_buffer_private_obj */

   /* Per-command buffer resources for meta operations. */
   struct {
      struct {
         /* The current descriptor pool for blit sources */
         VkDescriptorPool dspool;
      } blit;
      struct {
         /* The current descriptor pool for texel buffer copy sources */
         VkDescriptorPool dspool;
      } texel_buffer_copy;
   } meta;

   /* List of jobs in the command buffer. For primary command buffers it
    * represents the jobs we want to submit to the GPU. For secondary command
    * buffers it represents jobs that will be merged into a primary command
    * buffer via vkCmdExecuteCommands.
    */
   struct list_head jobs;
};

struct v3dv_job *v3dv_cmd_buffer_start_job(struct v3dv_cmd_buffer *cmd_buffer,
                                           int32_t subpass_idx,
                                           enum v3dv_job_type type);
void v3dv_cmd_buffer_finish_job(struct v3dv_cmd_buffer *cmd_buffer);

struct v3dv_job *v3dv_cmd_buffer_subpass_start(struct v3dv_cmd_buffer *cmd_buffer,
                                               uint32_t subpass_idx);
struct v3dv_job *v3dv_cmd_buffer_subpass_resume(struct v3dv_cmd_buffer *cmd_buffer,
                                                uint32_t subpass_idx);

void v3dv_cmd_buffer_subpass_finish(struct v3dv_cmd_buffer *cmd_buffer);

void v3dv_cmd_buffer_meta_state_push(struct v3dv_cmd_buffer *cmd_buffer,
                                     bool push_descriptor_state);
void v3dv_cmd_buffer_meta_state_pop(struct v3dv_cmd_buffer *cmd_buffer,
                                    uint32_t dirty_dynamic_state,
                                    bool needs_subpass_resume);

void v3dv_cmd_buffer_reset_queries(struct v3dv_cmd_buffer *cmd_buffer,
                                   struct v3dv_query_pool *pool,
                                   uint32_t first,
                                   uint32_t count);

void v3dv_cmd_buffer_begin_query(struct v3dv_cmd_buffer *cmd_buffer,
                                 struct v3dv_query_pool *pool,
                                 uint32_t query,
                                 VkQueryControlFlags flags);

void v3dv_cmd_buffer_end_query(struct v3dv_cmd_buffer *cmd_buffer,
                               struct v3dv_query_pool *pool,
                               uint32_t query);

void v3dv_cmd_buffer_copy_query_results(struct v3dv_cmd_buffer *cmd_buffer,
                                        struct v3dv_query_pool *pool,
                                        uint32_t first,
                                        uint32_t count,
                                        struct v3dv_buffer *dst,
                                        uint32_t offset,
                                        uint32_t stride,
                                        VkQueryResultFlags flags);

void v3dv_cmd_buffer_add_tfu_job(struct v3dv_cmd_buffer *cmd_buffer,
                                 struct drm_v3d_submit_tfu *tfu);

void v3dv_cmd_buffer_rewrite_indirect_csd_job(struct v3dv_csd_indirect_cpu_job_info *info,
                                              const uint32_t *wg_counts);

void v3dv_cmd_buffer_add_private_obj(struct v3dv_cmd_buffer *cmd_buffer,
                                     uint64_t obj,
                                     v3dv_cmd_buffer_private_obj_destroy_cb destroy_cb);

struct v3dv_semaphore {
   struct vk_object_base base;

   /* A syncobject handle associated with this semaphore */
   uint32_t sync;

   /* A temporary syncobject handle produced from a vkImportSemaphoreFd. */
   uint32_t temp_sync;
};

struct v3dv_fence {
   struct vk_object_base base;

   /* A syncobject handle associated with this fence */
   uint32_t sync;

   /* A temporary syncobject handle produced from a vkImportFenceFd. */
   uint32_t temp_sync;
};

struct v3dv_event {
   struct vk_object_base base;
   int state;
};

struct v3dv_shader_variant {
   enum broadcom_shader_stage stage;

   union {
      struct v3d_prog_data *base;
      struct v3d_vs_prog_data *vs;
      struct v3d_gs_prog_data *gs;
      struct v3d_fs_prog_data *fs;
      struct v3d_compute_prog_data *cs;
   } prog_data;

   /* We explicitly save the prog_data_size as it would make easier to
    * serialize
    */
   uint32_t prog_data_size;

   /* The assembly for this variant will be uploaded to a BO shared with all
    * other shader stages in that pipeline. This is the offset in that BO.
    */
   uint32_t assembly_offset;

   /* Note: it is really likely that qpu_insts would be NULL, as it will be
    * used only temporarily, to upload it to the shared bo, as we compile the
    * different stages individually.
    */
   uint64_t *qpu_insts;
   uint32_t qpu_insts_size;
};

/*
 * Per-stage info for each stage, useful so shader_module_compile_to_nir and
 * other methods doesn't have so many parameters.
 *
 * FIXME: for the case of the coordinate shader and the vertex shader, module,
 * entrypoint, spec_info and nir are the same. There are also info only
 * relevant to some stages. But seemed too much a hassle to create a new
 * struct only to handle that. Revisit if such kind of info starts to grow.
 */
struct v3dv_pipeline_stage {
   struct v3dv_pipeline *pipeline;

   enum broadcom_shader_stage stage;

   const struct vk_shader_module *module;
   const char *entrypoint;
   const VkSpecializationInfo *spec_info;

   nir_shader *nir;

   /* The following is the combined hash of module+entrypoint+spec_info+nir */
   unsigned char shader_sha1[20];

   /** A name for this program, so you can track it in shader-db output. */
   uint32_t program_id;

   VkPipelineCreationFeedbackEXT feedback;
};

/* We are using the descriptor pool entry for two things:
 * * Track the allocated sets, so we can properly free it if needed
 * * Track the suballocated pool bo regions, so if some descriptor set is
 *   freed, the gap could be reallocated later.
 *
 * Those only make sense if the pool was not created with the flag
 * VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
 */
struct v3dv_descriptor_pool_entry
{
   struct v3dv_descriptor_set *set;
   /* Offset and size of the subregion allocated for this entry from the
    * pool->bo
    */
   uint32_t offset;
   uint32_t size;
};

struct v3dv_descriptor_pool {
   struct vk_object_base base;

   /* If this descriptor pool has been allocated for the driver for internal
    * use, typically to implement meta operations.
    */
   bool is_driver_internal;

   struct v3dv_bo *bo;
   /* Current offset at the descriptor bo. 0 means that we didn't use it for
    * any descriptor. If the descriptor bo is NULL, current offset is
    * meaningless
    */
   uint32_t current_offset;

   /* If VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT is not set the
    * descriptor sets are handled as a whole as pool memory and handled by the
    * following pointers. If set, they are not used, and individually
    * descriptor sets are allocated/freed.
    */
   uint8_t *host_memory_base;
   uint8_t *host_memory_ptr;
   uint8_t *host_memory_end;

   uint32_t entry_count;
   uint32_t max_entry_count;
   struct v3dv_descriptor_pool_entry entries[0];
};

struct v3dv_descriptor_set {
   struct vk_object_base base;

   struct v3dv_descriptor_pool *pool;

   const struct v3dv_descriptor_set_layout *layout;

   /* Offset relative to the descriptor pool bo for this set */
   uint32_t base_offset;

   /* The descriptors below can be indexed (set/binding) using the set_layout
    */
   struct v3dv_descriptor descriptors[0];
};

struct v3dv_descriptor_set_binding_layout {
   VkDescriptorType type;

   /* Number of array elements in this binding */
   uint32_t array_size;

   /* Index into the flattend descriptor set */
   uint32_t descriptor_index;

   uint32_t dynamic_offset_count;
   uint32_t dynamic_offset_index;

   /* Offset into the descriptor set where this descriptor lives (final offset
    * on the descriptor bo need to take into account set->base_offset)
    */
   uint32_t descriptor_offset;

   /* Offset in the v3dv_descriptor_set_layout of the immutable samplers, or 0
    * if there are no immutable samplers.
    */
   uint32_t immutable_samplers_offset;
};

struct v3dv_descriptor_set_layout {
   struct vk_object_base base;

   VkDescriptorSetLayoutCreateFlags flags;

   /* Number of bindings in this descriptor set */
   uint32_t binding_count;

   /* Total bo size needed for this descriptor set
    */
   uint32_t bo_size;

   /* Shader stages affected by this descriptor set */
   uint16_t shader_stages;

   /* Number of descriptors in this descriptor set */
   uint32_t descriptor_count;

   /* Number of dynamic offsets used by this descriptor set */
   uint16_t dynamic_offset_count;

   /* Bindings in this descriptor set */
   struct v3dv_descriptor_set_binding_layout binding[0];
};

struct v3dv_pipeline_layout {
   struct vk_object_base base;

   struct {
      struct v3dv_descriptor_set_layout *layout;
      uint32_t dynamic_offset_start;
   } set[MAX_SETS];

   uint32_t num_sets;

   /* Shader stages that are declared to use descriptors from this layout */
   uint32_t shader_stages;

   uint32_t dynamic_offset_count;
   uint32_t push_constant_size;
};

/*
 * We are using descriptor maps for ubo/ssbo and texture/samplers, so we need
 * it to be big enough to include the max value for all of them.
 *
 * FIXME: one alternative would be to allocate the map as big as you need for
 * each descriptor type. That would means more individual allocations.
 */
#define DESCRIPTOR_MAP_SIZE MAX3(V3D_MAX_TEXTURE_SAMPLERS, \
                                 MAX_UNIFORM_BUFFERS,      \
                                 MAX_STORAGE_BUFFERS)


struct v3dv_descriptor_map {
   /* TODO: avoid fixed size array/justify the size */
   unsigned num_desc; /* Number of descriptors  */
   int set[DESCRIPTOR_MAP_SIZE];
   int binding[DESCRIPTOR_MAP_SIZE];
   int array_index[DESCRIPTOR_MAP_SIZE];
   int array_size[DESCRIPTOR_MAP_SIZE];

   /* NOTE: the following is only for sampler, but this is the easier place to
    * put it.
    */
   uint8_t return_size[DESCRIPTOR_MAP_SIZE];
};

struct v3dv_sampler {
   struct vk_object_base base;

   bool compare_enable;
   bool unnormalized_coordinates;
   bool clamp_to_transparent_black_border;

   /* Prepacked SAMPLER_STATE, that is referenced as part of the tmu
    * configuration. If needed it will be copied to the descriptor info during
    * UpdateDescriptorSets
    */
   uint8_t sampler_state[V3DV_SAMPLER_STATE_LENGTH];
};

struct v3dv_descriptor_template_entry {
   /* The type of descriptor in this entry */
   VkDescriptorType type;

   /* Binding in the descriptor set */
   uint32_t binding;

   /* Offset at which to write into the descriptor set binding */
   uint32_t array_element;

   /* Number of elements to write into the descriptor set binding */
   uint32_t array_count;

   /* Offset into the user provided data */
   size_t offset;

   /* Stride between elements into the user provided data */
   size_t stride;
};

struct v3dv_descriptor_update_template {
   struct vk_object_base base;

   VkPipelineBindPoint bind_point;

   /* The descriptor set this template corresponds to. This value is only
    * valid if the template was created with the templateType
    * VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET.
    */
   uint8_t set;

   /* Number of entries in this template */
   uint32_t entry_count;

   /* Entries of the template */
   struct v3dv_descriptor_template_entry entries[0];
};


/* We keep two special values for the sampler idx that represents exactly when a
 * sampler is not needed/provided. The main use is that even if we don't have
 * sampler, we still need to do the output unpacking (through
 * nir_lower_tex). The easier way to do this is to add those special "no
 * sampler" in the sampler_map, and then use the proper unpacking for that
 * case.
 *
 * We have one when we want a 16bit output size, and other when we want a
 * 32bit output size. We use the info coming from the RelaxedPrecision
 * decoration to decide between one and the other.
 */
#define V3DV_NO_SAMPLER_16BIT_IDX 0
#define V3DV_NO_SAMPLER_32BIT_IDX 1

/*
 * Following two methods are using on the combined to/from texture/sampler
 * indices maps at v3dv_pipeline.
 */
static inline uint32_t
v3dv_pipeline_combined_index_key_create(uint32_t texture_index,
                                        uint32_t sampler_index)
{
   return texture_index << 24 | sampler_index;
}

static inline void
v3dv_pipeline_combined_index_key_unpack(uint32_t combined_index_key,
                                        uint32_t *texture_index,
                                        uint32_t *sampler_index)
{
   uint32_t texture = combined_index_key >> 24;
   uint32_t sampler = combined_index_key & 0xffffff;

   if (texture_index)
      *texture_index = texture;

   if (sampler_index)
      *sampler_index = sampler;
}

struct v3dv_descriptor_maps {
   struct v3dv_descriptor_map ubo_map;
   struct v3dv_descriptor_map ssbo_map;
   struct v3dv_descriptor_map sampler_map;
   struct v3dv_descriptor_map texture_map;
};

/* The structure represents data shared between different objects, like the
 * pipeline and the pipeline cache, so we ref count it to know when it should
 * be freed.
 */
struct v3dv_pipeline_shared_data {
   uint32_t ref_cnt;

   unsigned char sha1_key[20];

   struct v3dv_descriptor_maps *maps[BROADCOM_SHADER_STAGES];
   struct v3dv_shader_variant *variants[BROADCOM_SHADER_STAGES];

   struct v3dv_bo *assembly_bo;
};

struct v3dv_pipeline {
   struct vk_object_base base;

   struct v3dv_device *device;

   VkShaderStageFlags active_stages;

   struct v3dv_render_pass *pass;
   struct v3dv_subpass *subpass;

   /* Note: We can't use just a MESA_SHADER_STAGES array because we also need
    * to track binning shaders. Note these will be freed once the pipeline
    * has been compiled.
    */
   struct v3dv_pipeline_stage *vs;
   struct v3dv_pipeline_stage *vs_bin;
   struct v3dv_pipeline_stage *gs;
   struct v3dv_pipeline_stage *gs_bin;
   struct v3dv_pipeline_stage *fs;
   struct v3dv_pipeline_stage *cs;

   /* Flags for whether optional pipeline stages are present, for convenience */
   bool has_gs;

   /* Spilling memory requirements */
   struct {
      struct v3dv_bo *bo;
      uint32_t size_per_thread;
   } spill;

   struct v3dv_dynamic_state dynamic_state;

   struct v3dv_pipeline_layout *layout;

   /* Whether this pipeline enables depth writes */
   bool z_updates_enable;

   enum v3dv_ez_state ez_state;

   bool msaa;
   bool sample_rate_shading;
   uint32_t sample_mask;

   bool primitive_restart;

   /* Accessed by binding. So vb[binding]->stride is the stride of the vertex
    * array with such binding
    */
   struct v3dv_pipeline_vertex_binding {
      uint32_t stride;
      uint32_t instance_divisor;
   } vb[MAX_VBS];
   uint32_t vb_count;

   /* Note that a lot of info from VkVertexInputAttributeDescription is
    * already prepacked, so here we are only storing those that need recheck
    * later. The array must be indexed by driver location, since that is the
    * order in which we need to emit the attributes.
    */
   struct v3dv_pipeline_vertex_attrib {
      uint32_t binding;
      uint32_t offset;
      VkFormat vk_format;
   } va[MAX_VERTEX_ATTRIBS];
   uint32_t va_count;

   enum pipe_prim_type topology;

   struct v3dv_pipeline_shared_data *shared_data;

   /* In general we can reuse v3dv_device->default_attribute_float, so note
    * that the following can be NULL.
    *
    * FIXME: the content of this BO will be small, so it could be improved to
    * be uploaded to a common BO. But as in most cases it will be NULL, it is
    * not a priority.
    */
   struct v3dv_bo *default_attribute_values;

   struct vpm_config vpm_cfg;
   struct vpm_config vpm_cfg_bin;

   /* If the pipeline should emit any of the stencil configuration packets */
   bool emit_stencil_cfg[2];

   /* Blend state */
   struct {
      /* Per-RT bit mask with blend enables */
      uint8_t enables;
      /* Per-RT prepacked blend config packets */
      uint8_t cfg[V3D_MAX_DRAW_BUFFERS][V3DV_BLEND_CFG_LENGTH];
      /* Flag indicating whether the blend factors in use require
       * color constants.
       */
      bool needs_color_constants;
      /* Mask with enabled color channels for each RT (4 bits per RT) */
      uint32_t color_write_masks;
   } blend;

   /* Depth bias */
   struct {
      bool enabled;
      bool is_z16;
   } depth_bias;

   /* Packets prepacked during pipeline creation
    */
   uint8_t cfg_bits[V3DV_CFG_BITS_LENGTH];
   uint8_t shader_state_record[V3DV_GL_SHADER_STATE_RECORD_LENGTH];
   uint8_t vcm_cache_size[V3DV_VCM_CACHE_SIZE_LENGTH];
   uint8_t vertex_attrs[V3DV_GL_SHADER_STATE_ATTRIBUTE_RECORD_LENGTH *
                        MAX_VERTEX_ATTRIBS];
   uint8_t stencil_cfg[2][V3DV_STENCIL_CFG_LENGTH];
};

static inline VkPipelineBindPoint
v3dv_pipeline_get_binding_point(struct v3dv_pipeline *pipeline)
{
   assert(pipeline->active_stages == VK_SHADER_STAGE_COMPUTE_BIT ||
          !(pipeline->active_stages & VK_SHADER_STAGE_COMPUTE_BIT));
   return pipeline->active_stages == VK_SHADER_STAGE_COMPUTE_BIT ?
      VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
}

static inline struct v3dv_descriptor_state*
v3dv_cmd_buffer_get_descriptor_state(struct v3dv_cmd_buffer *cmd_buffer,
                                     struct v3dv_pipeline *pipeline)
{
   if (v3dv_pipeline_get_binding_point(pipeline) == VK_PIPELINE_BIND_POINT_COMPUTE)
      return &cmd_buffer->state.compute.descriptor_state;
   else
      return &cmd_buffer->state.gfx.descriptor_state;
}

const nir_shader_compiler_options *v3dv_pipeline_get_nir_options(void);

uint32_t v3dv_physical_device_vendor_id(struct v3dv_physical_device *dev);
uint32_t v3dv_physical_device_device_id(struct v3dv_physical_device *dev);

#ifdef DEBUG
#define v3dv_debug_ignored_stype(sType) \
   fprintf(stderr, "%s: ignored VkStructureType %u:%s\n\n", __func__, (sType), vk_StructureType_to_str(sType))
#else
#define v3dv_debug_ignored_stype(sType)
#endif

const uint8_t *v3dv_get_format_swizzle(struct v3dv_device *device, VkFormat f);
uint8_t v3dv_get_tex_return_size(const struct v3dv_format *vf, bool compare_enable);
const struct v3dv_format *
v3dv_get_compatible_tfu_format(struct v3dv_device *device,
                               uint32_t bpp, VkFormat *out_vk_format);
bool v3dv_buffer_format_supports_features(struct v3dv_device *device,
                                          VkFormat vk_format,
                                          VkFormatFeatureFlags features);

struct v3dv_cl_reloc v3dv_write_uniforms(struct v3dv_cmd_buffer *cmd_buffer,
                                         struct v3dv_pipeline *pipeline,
                                         struct v3dv_shader_variant *variant);

struct v3dv_cl_reloc v3dv_write_uniforms_wg_offsets(struct v3dv_cmd_buffer *cmd_buffer,
                                                    struct v3dv_pipeline *pipeline,
                                                    struct v3dv_shader_variant *variant,
                                                    uint32_t **wg_count_offsets);

struct v3dv_shader_variant *
v3dv_get_shader_variant(struct v3dv_pipeline_stage *p_stage,
                        struct v3dv_pipeline_cache *cache,
                        struct v3d_key *key,
                        size_t key_size,
                        const VkAllocationCallbacks *pAllocator,
                        VkResult *out_vk_result);

struct v3dv_shader_variant *
v3dv_shader_variant_create(struct v3dv_device *device,
                           enum broadcom_shader_stage stage,
                           struct v3d_prog_data *prog_data,
                           uint32_t prog_data_size,
                           uint32_t assembly_offset,
                           uint64_t *qpu_insts,
                           uint32_t qpu_insts_size,
                           VkResult *out_vk_result);

void
v3dv_shader_variant_destroy(struct v3dv_device *device,
                            struct v3dv_shader_variant *variant);

static inline void
v3dv_pipeline_shared_data_ref(struct v3dv_pipeline_shared_data *shared_data)
{
   assert(shared_data && shared_data->ref_cnt >= 1);
   p_atomic_inc(&shared_data->ref_cnt);
}

void
v3dv_pipeline_shared_data_destroy(struct v3dv_device *device,
                                  struct v3dv_pipeline_shared_data *shared_data);

static inline void
v3dv_pipeline_shared_data_unref(struct v3dv_device *device,
                                struct v3dv_pipeline_shared_data *shared_data)
{
   assert(shared_data && shared_data->ref_cnt >= 1);
   if (p_atomic_dec_zero(&shared_data->ref_cnt))
      v3dv_pipeline_shared_data_destroy(device, shared_data);
}

struct v3dv_descriptor *
v3dv_descriptor_map_get_descriptor(struct v3dv_descriptor_state *descriptor_state,
                                   struct v3dv_descriptor_map *map,
                                   struct v3dv_pipeline_layout *pipeline_layout,
                                   uint32_t index,
                                   uint32_t *dynamic_offset);

const struct v3dv_sampler *
v3dv_descriptor_map_get_sampler(struct v3dv_descriptor_state *descriptor_state,
                                struct v3dv_descriptor_map *map,
                                struct v3dv_pipeline_layout *pipeline_layout,
                                uint32_t index);

struct v3dv_cl_reloc
v3dv_descriptor_map_get_sampler_state(struct v3dv_device *device,
                                      struct v3dv_descriptor_state *descriptor_state,
                                      struct v3dv_descriptor_map *map,
                                      struct v3dv_pipeline_layout *pipeline_layout,
                                      uint32_t index);

struct v3dv_cl_reloc
v3dv_descriptor_map_get_texture_shader_state(struct v3dv_device *device,
                                             struct v3dv_descriptor_state *descriptor_state,
                                             struct v3dv_descriptor_map *map,
                                             struct v3dv_pipeline_layout *pipeline_layout,
                                             uint32_t index);

const struct v3dv_format*
v3dv_descriptor_map_get_texture_format(struct v3dv_descriptor_state *descriptor_state,
                                       struct v3dv_descriptor_map *map,
                                       struct v3dv_pipeline_layout *pipeline_layout,
                                       uint32_t index,
                                       VkFormat *out_vk_format);

struct v3dv_bo*
v3dv_descriptor_map_get_texture_bo(struct v3dv_descriptor_state *descriptor_state,
                                   struct v3dv_descriptor_map *map,
                                   struct v3dv_pipeline_layout *pipeline_layout,
                                   uint32_t index);

static inline const struct v3dv_sampler *
v3dv_immutable_samplers(const struct v3dv_descriptor_set_layout *set,
                        const struct v3dv_descriptor_set_binding_layout *binding)
{
   assert(binding->immutable_samplers_offset);
   return (const struct v3dv_sampler *) ((const char *) set + binding->immutable_samplers_offset);
}

void v3dv_pipeline_cache_init(struct v3dv_pipeline_cache *cache,
                              struct v3dv_device *device,
                              VkPipelineCacheCreateFlags,
                              bool cache_enabled);

void v3dv_pipeline_cache_finish(struct v3dv_pipeline_cache *cache);

void v3dv_pipeline_cache_upload_nir(struct v3dv_pipeline *pipeline,
                                    struct v3dv_pipeline_cache *cache,
                                    nir_shader *nir,
                                    unsigned char sha1_key[20]);

nir_shader* v3dv_pipeline_cache_search_for_nir(struct v3dv_pipeline *pipeline,
                                               struct v3dv_pipeline_cache *cache,
                                               const nir_shader_compiler_options *nir_options,
                                               unsigned char sha1_key[20]);

struct v3dv_pipeline_shared_data *
v3dv_pipeline_cache_search_for_pipeline(struct v3dv_pipeline_cache *cache,
                                        unsigned char sha1_key[20],
                                        bool *cache_hit);

void
v3dv_pipeline_cache_upload_pipeline(struct v3dv_pipeline *pipeline,
                                    struct v3dv_pipeline_cache *cache);

struct v3dv_bo *
v3dv_pipeline_create_default_attribute_values(struct v3dv_device *device,
                                              struct v3dv_pipeline *pipeline);

void v3dv_shader_module_internal_init(struct v3dv_device *device,
                                      struct vk_shader_module *module,
                                      nir_shader *nir);

#define V3DV_FROM_HANDLE(__v3dv_type, __name, __handle)			\
   VK_FROM_HANDLE(__v3dv_type, __name, __handle)

VK_DEFINE_HANDLE_CASTS(v3dv_cmd_buffer, vk.base, VkCommandBuffer,
                       VK_OBJECT_TYPE_COMMAND_BUFFER)
VK_DEFINE_HANDLE_CASTS(v3dv_device, vk.base, VkDevice, VK_OBJECT_TYPE_DEVICE)
VK_DEFINE_HANDLE_CASTS(v3dv_instance, vk.base, VkInstance,
                       VK_OBJECT_TYPE_INSTANCE)
VK_DEFINE_HANDLE_CASTS(v3dv_physical_device, vk.base, VkPhysicalDevice,
                       VK_OBJECT_TYPE_PHYSICAL_DEVICE)
VK_DEFINE_HANDLE_CASTS(v3dv_queue, vk.base, VkQueue, VK_OBJECT_TYPE_QUEUE)

VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_cmd_pool, base, VkCommandPool,
                               VK_OBJECT_TYPE_COMMAND_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_buffer, base, VkBuffer,
                               VK_OBJECT_TYPE_BUFFER)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_buffer_view, base, VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_device_memory, base, VkDeviceMemory,
                               VK_OBJECT_TYPE_DEVICE_MEMORY)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_descriptor_pool, base, VkDescriptorPool,
                               VK_OBJECT_TYPE_DESCRIPTOR_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_descriptor_set, base, VkDescriptorSet,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_descriptor_set_layout, base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_descriptor_update_template, base,
                               VkDescriptorUpdateTemplate,
                               VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_event, base, VkEvent, VK_OBJECT_TYPE_EVENT)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_fence, base, VkFence, VK_OBJECT_TYPE_FENCE)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_framebuffer, base, VkFramebuffer,
                               VK_OBJECT_TYPE_FRAMEBUFFER)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_image, vk.base, VkImage,
                               VK_OBJECT_TYPE_IMAGE)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_image_view, vk.base, VkImageView,
                               VK_OBJECT_TYPE_IMAGE_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_pipeline, base, VkPipeline,
                               VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_pipeline_cache, base, VkPipelineCache,
                               VK_OBJECT_TYPE_PIPELINE_CACHE)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_pipeline_layout, base, VkPipelineLayout,
                               VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_query_pool, base, VkQueryPool,
                               VK_OBJECT_TYPE_QUERY_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_render_pass, base, VkRenderPass,
                               VK_OBJECT_TYPE_RENDER_PASS)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_sampler, base, VkSampler,
                               VK_OBJECT_TYPE_SAMPLER)
VK_DEFINE_NONDISP_HANDLE_CASTS(v3dv_semaphore, base, VkSemaphore,
                               VK_OBJECT_TYPE_SEMAPHORE)

static inline int
v3dv_ioctl(int fd, unsigned long request, void *arg)
{
   if (using_v3d_simulator)
      return v3d_simulator_ioctl(fd, request, arg);
   else
      return drmIoctl(fd, request, arg);
}

/* Flags OOM conditions in command buffer state.
 *
 * Note: notice that no-op jobs don't have a command buffer reference.
 */
static inline void
v3dv_flag_oom(struct v3dv_cmd_buffer *cmd_buffer, struct v3dv_job *job)
{
   if (cmd_buffer) {
      cmd_buffer->state.oom = true;
   } else {
      assert(job);
      if (job->cmd_buffer)
         job->cmd_buffer->state.oom = true;
   }
}

#define v3dv_return_if_oom(_cmd_buffer, _job) do {                  \
   const struct v3dv_cmd_buffer *__cmd_buffer = _cmd_buffer;        \
   if (__cmd_buffer && __cmd_buffer->state.oom)                     \
      return;                                                       \
   const struct v3dv_job *__job = _job;                             \
   if (__job && __job->cmd_buffer && __job->cmd_buffer->state.oom)  \
      return;                                                       \
} while(0)                                                          \

static inline uint32_t
u64_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(uint64_t));
}

static inline bool
u64_compare(const void *key1, const void *key2)
{
   return memcmp(key1, key2, sizeof(uint64_t)) == 0;
}

/* Helper to call hw ver speficic functions */
#define v3dv_X(device, thing) ({                      \
   __typeof(&v3d42_##thing) v3d_X_thing;              \
   switch (device->devinfo.ver) {                     \
   case 42:                                           \
      v3d_X_thing = &v3d42_##thing;                   \
      break;                                          \
   default:                                           \
      unreachable("Unsupported hardware generation"); \
   }                                                  \
   v3d_X_thing;                                       \
})


/* v3d_macros from common requires v3dX and V3DX definitions. Below we need to
 * define v3dX for each version supported, because when we compile code that
 * is not version-specific, all version-specific macros need to be already
 * defined.
 */
#ifdef v3dX
#  include "v3dvx_private.h"
#else
#  define v3dX(x) v3d42_##x
#  include "v3dvx_private.h"
#  undef v3dX
#endif

#endif /* V3DV_PRIVATE_H */
