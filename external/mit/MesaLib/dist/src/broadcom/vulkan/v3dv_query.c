/*
 * Copyright © 2020 Raspberry Pi
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

#include "v3dv_private.h"

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_CreateQueryPool(VkDevice _device,
                     const VkQueryPoolCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkQueryPool *pQueryPool)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);

   assert(pCreateInfo->queryType == VK_QUERY_TYPE_OCCLUSION ||
          pCreateInfo->queryType == VK_QUERY_TYPE_TIMESTAMP);
   assert(pCreateInfo->queryCount > 0);

   struct v3dv_query_pool *pool =
      vk_object_zalloc(&device->vk, pAllocator, sizeof(*pool),
                       VK_OBJECT_TYPE_QUERY_POOL);
   if (pool == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   pool->query_type = pCreateInfo->queryType;
   pool->query_count = pCreateInfo->queryCount;

   VkResult result;

   const uint32_t pool_bytes = sizeof(struct v3dv_query) * pool->query_count;
   pool->queries = vk_alloc2(&device->vk.alloc, pAllocator, pool_bytes, 8,
                             VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pool->queries == NULL) {
      result = vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail;
   }

   if (pool->query_type == VK_QUERY_TYPE_OCCLUSION) {
      /* The hardware allows us to setup groups of 16 queries in consecutive
       * 4-byte addresses, requiring only that each group of 16 queries is
       * aligned to a 1024 byte boundary.
       */
      const uint32_t query_groups = DIV_ROUND_UP(pool->query_count, 16);
      const uint32_t bo_size = query_groups * 1024;
      pool->bo = v3dv_bo_alloc(device, bo_size, "query", true);
      if (!pool->bo) {
         result = vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto fail;
      }
      if (!v3dv_bo_map(device, pool->bo, bo_size)) {
         result = vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
         goto fail;
      }
   }

   uint32_t i;
   for (i = 0; i < pool->query_count; i++) {
      pool->queries[i].maybe_available = false;
      switch (pool->query_type) {
      case VK_QUERY_TYPE_OCCLUSION: {
         const uint32_t query_group = i / 16;
         const uint32_t query_offset = query_group * 1024 + (i % 16) * 4;
         pool->queries[i].bo = pool->bo;
         pool->queries[i].offset = query_offset;
         break;
         }
      case VK_QUERY_TYPE_TIMESTAMP:
         pool->queries[i].value = 0;
         break;
      default:
         unreachable("Unsupported query type");
      }
   }

   *pQueryPool = v3dv_query_pool_to_handle(pool);

   return VK_SUCCESS;

fail:
   if (pool->bo)
      v3dv_bo_free(device, pool->bo);
   if (pool->queries)
      vk_free2(&device->vk.alloc, pAllocator, pool->queries);
   vk_object_free(&device->vk, pAllocator, pool);

   return result;
}

VKAPI_ATTR void VKAPI_CALL
v3dv_DestroyQueryPool(VkDevice _device,
                      VkQueryPool queryPool,
                      const VkAllocationCallbacks *pAllocator)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   if (!pool)
      return;

   if (pool->bo)
      v3dv_bo_free(device, pool->bo);

   if (pool->queries)
      vk_free2(&device->vk.alloc, pAllocator, pool->queries);

   vk_object_free(&device->vk, pAllocator, pool);
}

static void
write_query_result(void *dst, uint32_t idx, bool do_64bit, uint64_t value)
{
   if (do_64bit) {
      uint64_t *dst64 = (uint64_t *) dst;
      dst64[idx] = value;
   } else {
      uint32_t *dst32 = (uint32_t *) dst;
      dst32[idx] = (uint32_t) value;
   }
}

static VkResult
get_occlusion_query_result(struct v3dv_device *device,
                           struct v3dv_query_pool *pool,
                           uint32_t query,
                           bool do_wait,
                           bool *available,
                           uint64_t *value)
{
   assert(pool && pool->query_type == VK_QUERY_TYPE_OCCLUSION);

   struct v3dv_query *q = &pool->queries[query];
   assert(q->bo && q->bo->map);

   if (do_wait) {
      /* From the Vulkan 1.0 spec:
       *
       *    "If VK_QUERY_RESULT_WAIT_BIT is set, (...) If the query does not
       *     become available in a finite amount of time (e.g. due to not
       *     issuing a query since the last reset), a VK_ERROR_DEVICE_LOST
       *     error may occur."
       */
      if (!q->maybe_available)
         return vk_error(device, VK_ERROR_DEVICE_LOST);

      if (!v3dv_bo_wait(device, q->bo, 0xffffffffffffffffull))
         return vk_error(device, VK_ERROR_DEVICE_LOST);

      *available = true;
   } else {
      *available = q->maybe_available && v3dv_bo_wait(device, q->bo, 0);
   }

   const uint8_t *query_addr = ((uint8_t *) q->bo->map) + q->offset;
   *value = (uint64_t) *((uint32_t *)query_addr);
   return VK_SUCCESS;
}

static VkResult
get_timestamp_query_result(struct v3dv_device *device,
                           struct v3dv_query_pool *pool,
                           uint32_t query,
                           bool do_wait,
                           bool *available,
                           uint64_t *value)
{
   assert(pool && pool->query_type == VK_QUERY_TYPE_TIMESTAMP);

   struct v3dv_query *q = &pool->queries[query];

   if (do_wait) {
      /* From the Vulkan 1.0 spec:
       *
       *    "If VK_QUERY_RESULT_WAIT_BIT is set, (...) If the query does not
       *     become available in a finite amount of time (e.g. due to not
       *     issuing a query since the last reset), a VK_ERROR_DEVICE_LOST
       *     error may occur."
       */
      if (!q->maybe_available)
         return vk_error(device, VK_ERROR_DEVICE_LOST);

      *available = true;
   } else {
      *available = q->maybe_available;
   }

   *value = q->value;
   return VK_SUCCESS;
}

static VkResult
get_query_result(struct v3dv_device *device,
                 struct v3dv_query_pool *pool,
                 uint32_t query,
                 bool do_wait,
                 bool *available,
                 uint64_t *value)
{
   switch (pool->query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      return get_occlusion_query_result(device, pool, query, do_wait,
                                        available, value);
   case VK_QUERY_TYPE_TIMESTAMP:
      return get_timestamp_query_result(device, pool, query, do_wait,
                                        available, value);
   default:
      unreachable("Unsupported query type");
   }
}

VkResult
v3dv_get_query_pool_results_cpu(struct v3dv_device *device,
                                struct v3dv_query_pool *pool,
                                uint32_t first,
                                uint32_t count,
                                void *data,
                                VkDeviceSize stride,
                                VkQueryResultFlags flags)
{
   assert(first < pool->query_count);
   assert(first + count <= pool->query_count);
   assert(data);

   const bool do_64bit = flags & VK_QUERY_RESULT_64_BIT;
   const bool do_wait = flags & VK_QUERY_RESULT_WAIT_BIT;
   const bool do_partial = flags & VK_QUERY_RESULT_PARTIAL_BIT;

   VkResult result = VK_SUCCESS;
   for (uint32_t i = first; i < first + count; i++) {
      bool available = false;
      uint64_t value = 0;
      VkResult query_result =
         get_query_result(device, pool, i, do_wait, &available, &value);
      if (query_result == VK_ERROR_DEVICE_LOST)
         result = VK_ERROR_DEVICE_LOST;

      /**
       * From the Vulkan 1.0 spec:
       *
       *    "If VK_QUERY_RESULT_WAIT_BIT and VK_QUERY_RESULT_PARTIAL_BIT are
       *     both not set then no result values are written to pData for queries
       *     that are in the unavailable state at the time of the call, and
       *     vkGetQueryPoolResults returns VK_NOT_READY. However, availability
       *     state is still written to pData for those queries if
       *     VK_QUERY_RESULT_WITH_AVAILABILITY_BIT is set."
       */
      uint32_t slot = 0;

      const bool write_result = available || do_partial;
      if (write_result)
         write_query_result(data, slot, do_64bit, value);
      slot++;

      if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT)
         write_query_result(data, slot++, do_64bit, available ? 1u : 0u);

      if (!write_result && result != VK_ERROR_DEVICE_LOST)
         result = VK_NOT_READY;

      data += stride;
   }

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetQueryPoolResults(VkDevice _device,
                         VkQueryPool queryPool,
                         uint32_t firstQuery,
                         uint32_t queryCount,
                         size_t dataSize,
                         void *pData,
                         VkDeviceSize stride,
                         VkQueryResultFlags flags)
{
   V3DV_FROM_HANDLE(v3dv_device, device, _device);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   return v3dv_get_query_pool_results_cpu(device, pool, firstQuery, queryCount,
                                          pData, stride, flags);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdResetQueryPool(VkCommandBuffer commandBuffer,
                       VkQueryPool queryPool,
                       uint32_t firstQuery,
                       uint32_t queryCount)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   v3dv_cmd_buffer_reset_queries(cmd_buffer, pool, firstQuery, queryCount);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer,
                             VkQueryPool queryPool,
                             uint32_t firstQuery,
                             uint32_t queryCount,
                             VkBuffer dstBuffer,
                             VkDeviceSize dstOffset,
                             VkDeviceSize stride,
                             VkQueryResultFlags flags)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);
   V3DV_FROM_HANDLE(v3dv_buffer, dst, dstBuffer);

   v3dv_cmd_buffer_copy_query_results(cmd_buffer, pool,
                                      firstQuery, queryCount,
                                      dst, dstOffset, stride, flags);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdBeginQuery(VkCommandBuffer commandBuffer,
                   VkQueryPool queryPool,
                   uint32_t query,
                   VkQueryControlFlags flags)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   v3dv_cmd_buffer_begin_query(cmd_buffer, pool, query, flags);
}

VKAPI_ATTR void VKAPI_CALL
v3dv_CmdEndQuery(VkCommandBuffer commandBuffer,
                 VkQueryPool queryPool,
                 uint32_t query)
{
   V3DV_FROM_HANDLE(v3dv_cmd_buffer, cmd_buffer, commandBuffer);
   V3DV_FROM_HANDLE(v3dv_query_pool, pool, queryPool);

   v3dv_cmd_buffer_end_query(cmd_buffer, pool, query);
}
