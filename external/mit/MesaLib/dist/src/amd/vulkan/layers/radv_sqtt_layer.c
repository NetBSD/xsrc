/*
 * Copyright © 2020 Valve Corporation
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

#include "radv_private.h"
#include "radv_shader.h"

#include "ac_rgp.h"
#include "ac_sqtt.h"

static void
radv_write_begin_general_api_marker(struct radv_cmd_buffer *cmd_buffer,
                                    enum rgp_sqtt_marker_general_api_type api_type)
{
   struct rgp_sqtt_marker_general_api marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API;
   marker.api_type = api_type;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

static void
radv_write_end_general_api_marker(struct radv_cmd_buffer *cmd_buffer,
                                  enum rgp_sqtt_marker_general_api_type api_type)
{
   struct rgp_sqtt_marker_general_api marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API;
   marker.api_type = api_type;
   marker.is_end = 1;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

static void
radv_write_event_marker(struct radv_cmd_buffer *cmd_buffer,
                        enum rgp_sqtt_marker_event_type api_type, uint32_t vertex_offset_user_data,
                        uint32_t instance_offset_user_data, uint32_t draw_index_user_data)
{
   struct rgp_sqtt_marker_event marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.api_type = api_type;
   marker.cmd_id = cmd_buffer->state.num_events++;
   marker.cb_id = 0;

   if (vertex_offset_user_data == UINT_MAX || instance_offset_user_data == UINT_MAX) {
      vertex_offset_user_data = 0;
      instance_offset_user_data = 0;
   }

   if (draw_index_user_data == UINT_MAX)
      draw_index_user_data = vertex_offset_user_data;

   marker.vertex_offset_reg_idx = vertex_offset_user_data;
   marker.instance_offset_reg_idx = instance_offset_user_data;
   marker.draw_index_reg_idx = draw_index_user_data;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

static void
radv_write_event_with_dims_marker(struct radv_cmd_buffer *cmd_buffer,
                                  enum rgp_sqtt_marker_event_type api_type, uint32_t x, uint32_t y,
                                  uint32_t z)
{
   struct rgp_sqtt_marker_event_with_dims marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   marker.event.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.event.api_type = api_type;
   marker.event.cmd_id = cmd_buffer->state.num_events++;
   marker.event.cb_id = 0;
   marker.event.has_thread_dims = 1;

   marker.thread_x = x;
   marker.thread_y = y;
   marker.thread_z = z;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

static void
radv_write_user_event_marker(struct radv_cmd_buffer *cmd_buffer,
                             enum rgp_sqtt_marker_user_event_type type, const char *str)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (type == UserEventPop) {
      assert(str == NULL);
      struct rgp_sqtt_marker_user_event marker = {0};
      marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT;
      marker.data_type = type;

      radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
   } else {
      assert(str != NULL);
      unsigned len = strlen(str);
      struct rgp_sqtt_marker_user_event_with_length marker = {0};
      marker.user_event.identifier = RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT;
      marker.user_event.data_type = type;
      marker.length = align(len, 4);

      uint8_t *buffer = alloca(sizeof(marker) + marker.length);
      memset(buffer, 0, sizeof(marker) + marker.length);
      memcpy(buffer, &marker, sizeof(marker));
      memcpy(buffer + sizeof(marker), str, len);

      radv_emit_thread_trace_userdata(cmd_buffer->device, cs, buffer,
                                      sizeof(marker) / 4 + marker.length / 4);
   }
}

void
radv_describe_begin_cmd_buffer(struct radv_cmd_buffer *cmd_buffer)
{
   uint64_t device_id = (uintptr_t)cmd_buffer->device;
   struct rgp_sqtt_marker_cb_start marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_CB_START;
   marker.cb_id = 0;
   marker.device_id_low = device_id;
   marker.device_id_high = device_id >> 32;
   marker.queue = cmd_buffer->queue_family_index;
   marker.queue_flags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT;

   if (cmd_buffer->queue_family_index == RADV_QUEUE_GENERAL)
      marker.queue_flags |= VK_QUEUE_GRAPHICS_BIT;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

void
radv_describe_end_cmd_buffer(struct radv_cmd_buffer *cmd_buffer)
{
   uint64_t device_id = (uintptr_t)cmd_buffer->device;
   struct rgp_sqtt_marker_cb_end marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_CB_END;
   marker.cb_id = 0;
   marker.device_id_low = device_id;
   marker.device_id_high = device_id >> 32;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

void
radv_describe_draw(struct radv_cmd_buffer *cmd_buffer)
{
   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   radv_write_event_marker(cmd_buffer, cmd_buffer->state.current_event_type, UINT_MAX, UINT_MAX,
                           UINT_MAX);
}

void
radv_describe_dispatch(struct radv_cmd_buffer *cmd_buffer, int x, int y, int z)
{
   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   radv_write_event_with_dims_marker(cmd_buffer, cmd_buffer->state.current_event_type, x, y, z);
}

void
radv_describe_begin_render_pass_clear(struct radv_cmd_buffer *cmd_buffer,
                                      VkImageAspectFlagBits aspects)
{
   cmd_buffer->state.current_event_type = (aspects & VK_IMAGE_ASPECT_COLOR_BIT)
                                             ? EventRenderPassColorClear
                                             : EventRenderPassDepthStencilClear;
}

void
radv_describe_end_render_pass_clear(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.current_event_type = EventInternalUnknown;
}

void
radv_describe_begin_render_pass_resolve(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.current_event_type = EventRenderPassResolve;
}

void
radv_describe_end_render_pass_resolve(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.current_event_type = EventInternalUnknown;
}

void
radv_describe_barrier_end_delayed(struct radv_cmd_buffer *cmd_buffer)
{
   struct rgp_sqtt_marker_barrier_end marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (likely(!cmd_buffer->device->thread_trace.bo) || !cmd_buffer->state.pending_sqtt_barrier_end)
      return;

   cmd_buffer->state.pending_sqtt_barrier_end = false;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BARRIER_END;
   marker.cb_id = 0;

   marker.num_layout_transitions = cmd_buffer->state.num_layout_transitions;

   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_WAIT_ON_EOP_TS)
      marker.wait_on_eop_ts = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_VS_PARTIAL_FLUSH)
      marker.vs_partial_flush = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_PS_PARTIAL_FLUSH)
      marker.ps_partial_flush = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_CS_PARTIAL_FLUSH)
      marker.cs_partial_flush = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_PFP_SYNC_ME)
      marker.pfp_sync_me = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_SYNC_CP_DMA)
      marker.sync_cp_dma = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_VMEM_L0)
      marker.inval_tcp = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_ICACHE)
      marker.inval_sqI = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_SMEM_L0)
      marker.inval_sqK = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_FLUSH_L2)
      marker.flush_tcc = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_L2)
      marker.inval_tcc = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_FLUSH_CB)
      marker.flush_cb = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_CB)
      marker.inval_cb = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_FLUSH_DB)
      marker.flush_db = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_DB)
      marker.inval_db = true;
   if (cmd_buffer->state.sqtt_flush_bits & RGP_FLUSH_INVAL_L1)
      marker.inval_gl1 = true;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);

   cmd_buffer->state.num_layout_transitions = 0;
}

void
radv_describe_barrier_start(struct radv_cmd_buffer *cmd_buffer, enum rgp_barrier_reason reason)
{
   struct rgp_sqtt_marker_barrier_start marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   radv_describe_barrier_end_delayed(cmd_buffer);
   cmd_buffer->state.sqtt_flush_bits = 0;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BARRIER_START;
   marker.cb_id = 0;
   marker.dword02 = reason;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

void
radv_describe_barrier_end(struct radv_cmd_buffer *cmd_buffer)
{
   cmd_buffer->state.pending_sqtt_barrier_end = true;
}

void
radv_describe_layout_transition(struct radv_cmd_buffer *cmd_buffer,
                                const struct radv_barrier_data *barrier)
{
   struct rgp_sqtt_marker_layout_transition marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_LAYOUT_TRANSITION;
   marker.depth_stencil_expand = barrier->layout_transitions.depth_stencil_expand;
   marker.htile_hiz_range_expand = barrier->layout_transitions.htile_hiz_range_expand;
   marker.depth_stencil_resummarize = barrier->layout_transitions.depth_stencil_resummarize;
   marker.dcc_decompress = barrier->layout_transitions.dcc_decompress;
   marker.fmask_decompress = barrier->layout_transitions.fmask_decompress;
   marker.fast_clear_eliminate = barrier->layout_transitions.fast_clear_eliminate;
   marker.fmask_color_expand = barrier->layout_transitions.fmask_color_expand;
   marker.init_mask_ram = barrier->layout_transitions.init_mask_ram;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);

   cmd_buffer->state.num_layout_transitions++;
}

static void
radv_describe_pipeline_bind(struct radv_cmd_buffer *cmd_buffer,
                            VkPipelineBindPoint pipelineBindPoint, struct radv_pipeline *pipeline)
{
   struct rgp_sqtt_marker_pipeline_bind marker = {0};
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (likely(!cmd_buffer->device->thread_trace.bo))
      return;

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BIND_PIPELINE;
   marker.cb_id = 0;
   marker.bind_point = pipelineBindPoint;
   marker.api_pso_hash[0] = pipeline->pipeline_hash;
   marker.api_pso_hash[1] = pipeline->pipeline_hash >> 32;

   radv_emit_thread_trace_userdata(cmd_buffer->device, cs, &marker, sizeof(marker) / 4);
}

/* TODO: Improve the way to trigger capture (overlay, etc). */
static void
radv_handle_thread_trace(VkQueue _queue)
{
   RADV_FROM_HANDLE(radv_queue, queue, _queue);
   static bool thread_trace_enabled = false;
   static uint64_t num_frames = 0;
   bool resize_trigger = false;

   if (thread_trace_enabled) {
      struct ac_thread_trace thread_trace = {0};

      radv_end_thread_trace(queue);
      thread_trace_enabled = false;

      /* TODO: Do something better than this whole sync. */
      radv_QueueWaitIdle(_queue);

      if (radv_get_thread_trace(queue, &thread_trace)) {
         ac_dump_rgp_capture(&queue->device->physical_device->rad_info, &thread_trace);
      } else {
         /* Trigger a new capture if the driver failed to get
          * the trace because the buffer was too small.
          */
         resize_trigger = true;
      }
   }

   if (!thread_trace_enabled) {
      bool frame_trigger = num_frames == queue->device->thread_trace.start_frame;
      bool file_trigger = false;
#ifndef _WIN32
      if (queue->device->thread_trace.trigger_file &&
          access(queue->device->thread_trace.trigger_file, W_OK) == 0) {
         if (unlink(queue->device->thread_trace.trigger_file) == 0) {
            file_trigger = true;
         } else {
            /* Do not enable tracing if we cannot remove the file,
             * because by then we'll trace every frame ... */
            fprintf(stderr, "RADV: could not remove thread trace trigger file, ignoring\n");
         }
      }
#endif

      if (frame_trigger || file_trigger || resize_trigger) {
         radv_begin_thread_trace(queue);
         assert(!thread_trace_enabled);
         thread_trace_enabled = true;
      }
   }
   num_frames++;
}

VkResult
sqtt_QueuePresentKHR(VkQueue _queue, const VkPresentInfoKHR *pPresentInfo)
{
   VkResult result;

   result = radv_QueuePresentKHR(_queue, pPresentInfo);
   if (result != VK_SUCCESS)
      return result;

   radv_handle_thread_trace(_queue);

   return VK_SUCCESS;
}

#define EVENT_MARKER_ALIAS(cmd_name, api_name, ...)                                                \
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);                                   \
   radv_write_begin_general_api_marker(cmd_buffer, ApiCmd##api_name);                              \
   cmd_buffer->state.current_event_type = EventCmd##api_name;                                      \
   radv_Cmd##cmd_name(__VA_ARGS__);                                                                \
   cmd_buffer->state.current_event_type = EventInternalUnknown;                                    \
   radv_write_end_general_api_marker(cmd_buffer, ApiCmd##api_name);

#define EVENT_MARKER(cmd_name, ...) EVENT_MARKER_ALIAS(cmd_name, cmd_name, __VA_ARGS__);

void
sqtt_CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
             uint32_t firstVertex, uint32_t firstInstance)
{
   EVENT_MARKER(Draw, commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void
sqtt_CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                    uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
   EVENT_MARKER(DrawIndexed, commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset,
                firstInstance);
}

void
sqtt_CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                     uint32_t drawCount, uint32_t stride)
{
   EVENT_MARKER(DrawIndirect, commandBuffer, buffer, offset, drawCount, stride);
}

void
sqtt_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                            uint32_t drawCount, uint32_t stride)
{
   EVENT_MARKER(DrawIndexedIndirect, commandBuffer, buffer, offset, drawCount, stride);
}

void
sqtt_CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                          uint32_t maxDrawCount, uint32_t stride)
{
   EVENT_MARKER(DrawIndirectCount, commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                maxDrawCount, stride);
}

void
sqtt_CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                 VkDeviceSize offset, VkBuffer countBuffer,
                                 VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                 uint32_t stride)
{
   EVENT_MARKER(DrawIndexedIndirectCount, commandBuffer, buffer, offset, countBuffer,
                countBufferOffset, maxDrawCount, stride);
}

void
sqtt_CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
   EVENT_MARKER(Dispatch, commandBuffer, x, y, z);
}

void
sqtt_CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
   EVENT_MARKER(DispatchIndirect, commandBuffer, buffer, offset);
}

void
sqtt_CmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo)
{
   EVENT_MARKER_ALIAS(CopyBuffer2KHR, CopyBuffer, commandBuffer, pCopyBufferInfo);
}

void
sqtt_CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                   VkDeviceSize fillSize, uint32_t data)
{
   EVENT_MARKER(FillBuffer, commandBuffer, dstBuffer, dstOffset, fillSize, data);
}

void
sqtt_CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                     VkDeviceSize dataSize, const void *pData)
{
   EVENT_MARKER(UpdateBuffer, commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void
sqtt_CmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo)
{
   EVENT_MARKER_ALIAS(CopyImage2KHR, CopyImage, commandBuffer, pCopyImageInfo);
}

void
sqtt_CmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                              const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo)
{
   EVENT_MARKER_ALIAS(CopyBufferToImage2KHR, CopyBufferToImage, commandBuffer,
                      pCopyBufferToImageInfo);
}

void
sqtt_CmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                              const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo)
{
   EVENT_MARKER_ALIAS(CopyImageToBuffer2KHR, CopyImageToBuffer, commandBuffer,
                      pCopyImageToBufferInfo);
}

void
sqtt_CmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo)
{
   EVENT_MARKER_ALIAS(BlitImage2KHR, BlitImage, commandBuffer, pBlitImageInfo);
}

void
sqtt_CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image_h, VkImageLayout imageLayout,
                        const VkClearColorValue *pColor, uint32_t rangeCount,
                        const VkImageSubresourceRange *pRanges)
{
   EVENT_MARKER(ClearColorImage, commandBuffer, image_h, imageLayout, pColor, rangeCount, pRanges);
}

void
sqtt_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image_h,
                               VkImageLayout imageLayout,
                               const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                               const VkImageSubresourceRange *pRanges)
{
   EVENT_MARKER(ClearDepthStencilImage, commandBuffer, image_h, imageLayout, pDepthStencil,
                rangeCount, pRanges);
}

void
sqtt_CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                         const VkClearAttachment *pAttachments, uint32_t rectCount,
                         const VkClearRect *pRects)
{
   EVENT_MARKER(ClearAttachments, commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void
sqtt_CmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                         const VkResolveImageInfo2KHR *pResolveImageInfo)
{
   EVENT_MARKER_ALIAS(ResolveImage2KHR, ResolveImage, commandBuffer, pResolveImageInfo);
}

void
sqtt_CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                   VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                   uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                   uint32_t bufferMemoryBarrierCount,
                   const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                   uint32_t imageMemoryBarrierCount,
                   const VkImageMemoryBarrier *pImageMemoryBarriers)
{
   EVENT_MARKER(WaitEvents, commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask,
                memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void
sqtt_CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                        VkPipelineStageFlags destStageMask, VkBool32 byRegion,
                        uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                        uint32_t bufferMemoryBarrierCount,
                        const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                        uint32_t imageMemoryBarrierCount,
                        const VkImageMemoryBarrier *pImageMemoryBarriers)
{
   EVENT_MARKER(PipelineBarrier, commandBuffer, srcStageMask, destStageMask, byRegion,
                memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void
sqtt_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                       uint32_t queryCount)
{
   EVENT_MARKER(ResetQueryPool, commandBuffer, queryPool, firstQuery, queryCount);
}

void
sqtt_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                             uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                             VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
   EVENT_MARKER(CopyQueryPoolResults, commandBuffer, queryPool, firstQuery, queryCount, dstBuffer,
                dstOffset, stride, flags);
}

#undef EVENT_MARKER
#define API_MARKER_ALIAS(cmd_name, api_name, ...)                                                  \
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);                                   \
   radv_write_begin_general_api_marker(cmd_buffer, ApiCmd##api_name);                              \
   radv_Cmd##cmd_name(__VA_ARGS__);                                                                \
   radv_write_end_general_api_marker(cmd_buffer, ApiCmd##api_name);

#define API_MARKER(cmd_name, ...) API_MARKER_ALIAS(cmd_name, cmd_name, __VA_ARGS__);

void
sqtt_CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                     VkPipeline _pipeline)
{
   RADV_FROM_HANDLE(radv_pipeline, pipeline, _pipeline);

   API_MARKER(BindPipeline, commandBuffer, pipelineBindPoint, _pipeline);

   if (radv_is_instruction_timing_enabled())
      radv_describe_pipeline_bind(cmd_buffer, pipelineBindPoint, pipeline);
}

void
sqtt_CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                           VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount,
                           const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                           const uint32_t *pDynamicOffsets)
{
   API_MARKER(BindDescriptorSets, commandBuffer, pipelineBindPoint, layout, firstSet,
              descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void
sqtt_CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                        VkIndexType indexType)
{
   API_MARKER(BindIndexBuffer, commandBuffer, buffer, offset, indexType);
}

void
sqtt_CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                          uint32_t bindingCount, const VkBuffer *pBuffers,
                          const VkDeviceSize *pOffsets)
{
   API_MARKER(BindVertexBuffers, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

void
sqtt_CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                   VkQueryControlFlags flags)
{
   API_MARKER(BeginQuery, commandBuffer, queryPool, query, flags);
}

void
sqtt_CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
   API_MARKER(EndQuery, commandBuffer, queryPool, query);
}

void
sqtt_CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                       VkQueryPool queryPool, uint32_t flags)
{
   API_MARKER(WriteTimestamp, commandBuffer, pipelineStage, queryPool, flags);
}

void
sqtt_CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                      VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                      const void *pValues)
{
   API_MARKER(PushConstants, commandBuffer, layout, stageFlags, offset, size, pValues);
}

void
sqtt_CmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                         const VkRenderPassBeginInfo *pRenderPassBeginInfo,
                         const VkSubpassBeginInfo *pSubpassBeginInfo)
{
   API_MARKER_ALIAS(BeginRenderPass2, BeginRenderPass, commandBuffer, pRenderPassBeginInfo,
                    pSubpassBeginInfo);
}

void
sqtt_CmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                     const VkSubpassEndInfo *pSubpassEndInfo)
{
   API_MARKER_ALIAS(NextSubpass2, NextSubpass, commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}

void
sqtt_CmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo)
{
   API_MARKER_ALIAS(EndRenderPass2, EndRenderPass, commandBuffer, pSubpassEndInfo);
}

void
sqtt_CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                        const VkCommandBuffer *pCmdBuffers)
{
   API_MARKER(ExecuteCommands, commandBuffer, commandBufferCount, pCmdBuffers);
}

void
sqtt_CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                    const VkViewport *pViewports)
{
   API_MARKER(SetViewport, commandBuffer, firstViewport, viewportCount, pViewports);
}

void
sqtt_CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                   const VkRect2D *pScissors)
{
   API_MARKER(SetScissor, commandBuffer, firstScissor, scissorCount, pScissors);
}

void
sqtt_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
   API_MARKER(SetLineWidth, commandBuffer, lineWidth);
}

void
sqtt_CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                     float depthBiasClamp, float depthBiasSlopeFactor)
{
   API_MARKER(SetDepthBias, commandBuffer, depthBiasConstantFactor, depthBiasClamp,
              depthBiasSlopeFactor);
}

void
sqtt_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
   API_MARKER(SetBlendConstants, commandBuffer, blendConstants);
}

void
sqtt_CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
   API_MARKER(SetDepthBounds, commandBuffer, minDepthBounds, maxDepthBounds);
}

void
sqtt_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                              uint32_t compareMask)
{
   API_MARKER(SetStencilCompareMask, commandBuffer, faceMask, compareMask);
}

void
sqtt_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                            uint32_t writeMask)
{
   API_MARKER(SetStencilWriteMask, commandBuffer, faceMask, writeMask);
}

void
sqtt_CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                            uint32_t reference)
{
   API_MARKER(SetStencilReference, commandBuffer, faceMask, reference);
}

/* VK_EXT_debug_marker */
void
sqtt_CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                            const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventPush, pMarkerInfo->pMarkerName);
}

void
sqtt_CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventPop, NULL);
}

void
sqtt_CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer,
                             const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   radv_write_user_event_marker(cmd_buffer, UserEventTrigger, pMarkerInfo->pMarkerName);
}

VkResult
sqtt_DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo)
{
   /* no-op */
   return VK_SUCCESS;
}

VkResult
sqtt_DebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo)
{
   /* no-op */
   return VK_SUCCESS;
}

/* Pipelines */
static enum rgp_hardware_stages
radv_mesa_to_rgp_shader_stage(struct radv_pipeline *pipeline, gl_shader_stage stage)
{
   struct radv_shader_variant *shader = pipeline->shaders[stage];

   switch (stage) {
   case MESA_SHADER_VERTEX:
      if (shader->info.vs.as_ls)
         return RGP_HW_STAGE_LS;
      else if (shader->info.vs.as_es)
         return RGP_HW_STAGE_ES;
      else if (shader->info.is_ngg)
         return RGP_HW_STAGE_GS;
      else
         return RGP_HW_STAGE_VS;
   case MESA_SHADER_TESS_CTRL:
      return RGP_HW_STAGE_HS;
   case MESA_SHADER_TESS_EVAL:
      if (shader->info.tes.as_es)
         return RGP_HW_STAGE_ES;
      else if (shader->info.is_ngg)
         return RGP_HW_STAGE_GS;
      else
         return RGP_HW_STAGE_VS;
   case MESA_SHADER_GEOMETRY:
      return RGP_HW_STAGE_GS;
   case MESA_SHADER_FRAGMENT:
      return RGP_HW_STAGE_PS;
   case MESA_SHADER_COMPUTE:
      return RGP_HW_STAGE_CS;
   default:
      unreachable("invalid mesa shader stage");
   }
}

static VkResult
radv_add_code_object(struct radv_device *device, struct radv_pipeline *pipeline)
{
   struct ac_thread_trace_data *thread_trace_data = &device->thread_trace;
   struct rgp_code_object *code_object = &thread_trace_data->rgp_code_object;
   struct rgp_code_object_record *record;

   record = malloc(sizeof(struct rgp_code_object_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   record->shader_stages_mask = 0;
   record->num_shaders_combined = 0;
   record->pipeline_hash[0] = pipeline->pipeline_hash;
   record->pipeline_hash[1] = pipeline->pipeline_hash;

   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct radv_shader_variant *shader = pipeline->shaders[i];
      uint8_t *code;
      uint64_t va;

      if (!shader)
         continue;

      code = malloc(shader->code_size);
      if (!code) {
         free(record);
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
      memcpy(code, shader->code_ptr, shader->code_size);

      va = radv_shader_variant_get_va(shader);

      record->shader_data[i].hash[0] = (uint64_t)(uintptr_t)shader;
      record->shader_data[i].hash[1] = (uint64_t)(uintptr_t)shader >> 32;
      record->shader_data[i].code_size = shader->code_size;
      record->shader_data[i].code = code;
      record->shader_data[i].vgpr_count = shader->config.num_vgprs;
      record->shader_data[i].sgpr_count = shader->config.num_sgprs;
      record->shader_data[i].scratch_memory_size = shader->config.scratch_bytes_per_wave;
      record->shader_data[i].wavefront_size = shader->info.wave_size;
      record->shader_data[i].base_address = va & 0xffffffffffff;
      record->shader_data[i].elf_symbol_offset = 0;
      record->shader_data[i].hw_stage = radv_mesa_to_rgp_shader_stage(pipeline, i);
      record->shader_data[i].is_combined = false;

      record->shader_stages_mask |= (1 << i);
      record->num_shaders_combined++;
   }

   simple_mtx_lock(&code_object->lock);
   list_addtail(&record->list, &code_object->record);
   code_object->record_count++;
   simple_mtx_unlock(&code_object->lock);

   return VK_SUCCESS;
}

static VkResult
radv_register_pipeline(struct radv_device *device, struct radv_pipeline *pipeline)
{
   bool result;
   uint64_t base_va = ~0;

   result = ac_sqtt_add_pso_correlation(&device->thread_trace, pipeline->pipeline_hash);
   if (!result)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   /* Find the lowest shader BO VA. */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      struct radv_shader_variant *shader = pipeline->shaders[i];
      uint64_t va;

      if (!shader)
         continue;

      va = radv_shader_variant_get_va(shader);
      base_va = MIN2(base_va, va);
   }

   result =
      ac_sqtt_add_code_object_loader_event(&device->thread_trace, pipeline->pipeline_hash, base_va);
   if (!result)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   result = radv_add_code_object(device, pipeline);
   if (result != VK_SUCCESS)
      return result;

   return VK_SUCCESS;
}

static void
radv_unregister_pipeline(struct radv_device *device, struct radv_pipeline *pipeline)
{
   struct ac_thread_trace_data *thread_trace_data = &device->thread_trace;
   struct rgp_pso_correlation *pso_correlation = &thread_trace_data->rgp_pso_correlation;
   struct rgp_loader_events *loader_events = &thread_trace_data->rgp_loader_events;
   struct rgp_code_object *code_object = &thread_trace_data->rgp_code_object;

   /* Destroy the PSO correlation record. */
   simple_mtx_lock(&pso_correlation->lock);
   list_for_each_entry_safe(struct rgp_pso_correlation_record, record, &pso_correlation->record,
                            list)
   {
      if (record->pipeline_hash[0] == pipeline->pipeline_hash) {
         pso_correlation->record_count--;
         list_del(&record->list);
         free(record);
         break;
      }
   }
   simple_mtx_unlock(&pso_correlation->lock);

   /* Destroy the code object loader record. */
   simple_mtx_lock(&loader_events->lock);
   list_for_each_entry_safe(struct rgp_loader_events_record, record, &loader_events->record, list)
   {
      if (record->code_object_hash[0] == pipeline->pipeline_hash) {
         loader_events->record_count--;
         list_del(&record->list);
         free(record);
         break;
      }
   }
   simple_mtx_unlock(&loader_events->lock);

   /* Destroy the code object record. */
   simple_mtx_lock(&code_object->lock);
   list_for_each_entry_safe(struct rgp_code_object_record, record, &code_object->record, list)
   {
      if (record->pipeline_hash[0] == pipeline->pipeline_hash) {
         uint32_t mask = record->shader_stages_mask;
         int i;

         /* Free the disassembly. */
         while (mask) {
            i = u_bit_scan(&mask);
            free(record->shader_data[i].code);
         }

         code_object->record_count--;
         list_del(&record->list);
         free(record);
         break;
      }
   }
   simple_mtx_unlock(&code_object->lock);
}

VkResult
sqtt_CreateGraphicsPipelines(VkDevice _device, VkPipelineCache pipelineCache, uint32_t count,
                             const VkGraphicsPipelineCreateInfo *pCreateInfos,
                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VkResult result;

   result = radv_CreateGraphicsPipelines(_device, pipelineCache, count, pCreateInfos, pAllocator,
                                         pPipelines);
   if (result != VK_SUCCESS)
      return result;

   if (radv_is_instruction_timing_enabled()) {
      for (unsigned i = 0; i < count; i++) {
         RADV_FROM_HANDLE(radv_pipeline, pipeline, pPipelines[i]);

         if (!pipeline)
            continue;

         result = radv_register_pipeline(device, pipeline);
         if (result != VK_SUCCESS)
            goto fail;
      }
   }

   return VK_SUCCESS;

fail:
   for (unsigned i = 0; i < count; i++) {
      sqtt_DestroyPipeline(_device, pPipelines[i], pAllocator);
      pPipelines[i] = VK_NULL_HANDLE;
   }
   return result;
}

VkResult
sqtt_CreateComputePipelines(VkDevice _device, VkPipelineCache pipelineCache, uint32_t count,
                            const VkComputePipelineCreateInfo *pCreateInfos,
                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VkResult result;

   result = radv_CreateComputePipelines(_device, pipelineCache, count, pCreateInfos, pAllocator,
                                        pPipelines);
   if (result != VK_SUCCESS)
      return result;

   if (radv_is_instruction_timing_enabled()) {
      for (unsigned i = 0; i < count; i++) {
         RADV_FROM_HANDLE(radv_pipeline, pipeline, pPipelines[i]);

         if (!pipeline)
            continue;

         result = radv_register_pipeline(device, pipeline);
         if (result != VK_SUCCESS)
            goto fail;
      }
   }

   return VK_SUCCESS;

fail:
   for (unsigned i = 0; i < count; i++) {
      sqtt_DestroyPipeline(_device, pPipelines[i], pAllocator);
      pPipelines[i] = VK_NULL_HANDLE;
   }
   return result;
}

void
sqtt_DestroyPipeline(VkDevice _device, VkPipeline _pipeline,
                     const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_pipeline, pipeline, _pipeline);

   if (!_pipeline)
      return;

   if (radv_is_instruction_timing_enabled())
      radv_unregister_pipeline(device, pipeline);

   radv_DestroyPipeline(_device, _pipeline, pAllocator);
}

#undef API_MARKER
