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

#include "d3d12_query.h"
#include "d3d12_context.h"
#include "d3d12_resource.h"
#include "d3d12_screen.h"

#include "util/u_dump.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

#include <dxguids/dxguids.h>

struct d3d12_query {
   enum pipe_query_type type;

   ID3D12QueryHeap *query_heap;
   unsigned curr_query, num_queries;
   size_t query_size;
   struct d3d12_query *subquery;

   D3D12_QUERY_TYPE d3d12qtype;

   pipe_resource *buffer;
   unsigned buffer_offset;
   uint64_t fence_value;

   struct list_head active_list;
   struct d3d12_resource *predicate;
};

static D3D12_QUERY_HEAP_TYPE
d3d12_query_heap_type(unsigned query_type)
{
   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
      return D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

   default:
      debug_printf("unknown query: %s\n",
                   util_str_query_type(query_type, true));
      unreachable("d3d12: unknown query type");
   }
}

static D3D12_QUERY_TYPE
d3d12_query_type(unsigned query_type)
{
   switch (query_type) {
   case PIPE_QUERY_OCCLUSION_COUNTER:
      return D3D12_QUERY_TYPE_OCCLUSION;
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
   case PIPE_QUERY_SO_STATISTICS:
      return D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0;
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
      return D3D12_QUERY_TYPE_TIMESTAMP;
   default:
      debug_printf("unknown query: %s\n",
                   util_str_query_type(query_type, true));
      unreachable("d3d12: unknown query type");
   }
}

static struct pipe_query *
d3d12_create_query(struct pipe_context *pctx,
                   unsigned query_type, unsigned index)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_screen *screen = d3d12_screen(pctx->screen);
   struct d3d12_query *query = CALLOC_STRUCT(d3d12_query);
   D3D12_QUERY_HEAP_DESC desc = {};

   if (!query)
      return NULL;

   query->type = (pipe_query_type)query_type;
   query->d3d12qtype = d3d12_query_type(query_type);
   query->num_queries = 16;

   /* With timer queries we want a few more queries, especially since we need two slots
    * per query for TIME_ELAPSED queries */
   if (unlikely(query->d3d12qtype == D3D12_QUERY_TYPE_TIMESTAMP))
      query->num_queries = 64;

   query->curr_query = 0;

   switch (query->d3d12qtype) {
   case D3D12_QUERY_TYPE_PIPELINE_STATISTICS:
      query->query_size = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
      break;
   case D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0:
      query->query_size = sizeof(D3D12_QUERY_DATA_SO_STATISTICS);
      break;
   default:
      query->query_size = sizeof(uint64_t);
      break;
   }

   desc.Count = query->num_queries;
   desc.Type = d3d12_query_heap_type(query_type);
   if (FAILED(screen->dev->CreateQueryHeap(&desc,
                                           IID_PPV_ARGS(&query->query_heap)))) {
      FREE(query);
      return NULL;
   }

   /* Query result goes into a readback buffer */
   size_t buffer_size = query->query_size * query->num_queries;
   u_suballocator_alloc(&ctx->query_allocator, buffer_size, 256,
                        &query->buffer_offset, &query->buffer);

   return (struct pipe_query *)query;
}

static void
d3d12_destroy_query(struct pipe_context *pctx,
                    struct pipe_query *q)
{
   struct d3d12_query *query = (struct d3d12_query *)q;
   pipe_resource *predicate = &query->predicate->base;
   if (query->subquery)
      d3d12_destroy_query(pctx, (struct pipe_query *)query->subquery);
   pipe_resource_reference(&predicate, NULL);
   query->query_heap->Release();
   FREE(query);
}

static bool
accumulate_result(struct d3d12_context *ctx, struct d3d12_query *q,
                  union pipe_query_result *result, bool write)
{
   struct pipe_transfer *transfer = NULL;
   struct d3d12_screen *screen = d3d12_screen(ctx->base.screen);
   unsigned access = PIPE_MAP_READ;
   void *results;

   if (write)
      access |= PIPE_MAP_WRITE;
   results = pipe_buffer_map_range(&ctx->base, q->buffer, q->buffer_offset,
                                   q->num_queries * q->query_size,
                                   access, &transfer);

   if (results == NULL)
      return false;

   uint64_t *results_u64 = (uint64_t *)results;
   D3D12_QUERY_DATA_PIPELINE_STATISTICS *results_stats = (D3D12_QUERY_DATA_PIPELINE_STATISTICS *)results;
   D3D12_QUERY_DATA_SO_STATISTICS *results_so = (D3D12_QUERY_DATA_SO_STATISTICS *)results;

   util_query_clear_result(result, q->type);
   for (unsigned i = 0; i < q->curr_query; ++i) {
      switch (q->type) {
      case PIPE_QUERY_OCCLUSION_PREDICATE:
      case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
         result->b |= results_u64[i] != 0;
         break;

      case PIPE_QUERY_OCCLUSION_COUNTER:
         result->u64 += results_u64[i];
         break;
      case PIPE_QUERY_TIMESTAMP:
         result->u64 = results_u64[i];
         break;

      case PIPE_QUERY_PIPELINE_STATISTICS:
         result->pipeline_statistics.ia_vertices += results_stats[i].IAVertices;
         result->pipeline_statistics.ia_primitives += results_stats[i].IAPrimitives;
         result->pipeline_statistics.vs_invocations += results_stats[i].VSInvocations;
         result->pipeline_statistics.gs_invocations += results_stats[i].GSInvocations;
         result->pipeline_statistics.gs_primitives += results_stats[i].GSPrimitives;
         result->pipeline_statistics.c_invocations += results_stats[i].CInvocations;
         result->pipeline_statistics.c_primitives += results_stats[i].CPrimitives;
         result->pipeline_statistics.ps_invocations += results_stats[i].PSInvocations;
         result->pipeline_statistics.hs_invocations += results_stats[i].HSInvocations;
         result->pipeline_statistics.ds_invocations += results_stats[i].DSInvocations;
         result->pipeline_statistics.cs_invocations += results_stats[i].CSInvocations;
         break;

      case PIPE_QUERY_PRIMITIVES_GENERATED:
         result->u64 += results_so[i].PrimitivesStorageNeeded;
         break;

      case PIPE_QUERY_PRIMITIVES_EMITTED:
         result->u64 += results_so[i].NumPrimitivesWritten;
         break;

      case PIPE_QUERY_TIME_ELAPSED:
         result->u64 += results_u64[2 * i + 1] - results_u64[2 * i];
         break;

      case PIPE_QUERY_SO_STATISTICS:
         result->so_statistics.num_primitives_written += results_so[i].NumPrimitivesWritten;
         result->so_statistics.primitives_storage_needed += results_so[i].PrimitivesStorageNeeded;
         break;

      default:
         debug_printf("unsupported query type: %s\n",
                      util_str_query_type(q->type, true));
         unreachable("unexpected query type");
      }
   }

   if (q->subquery) {
      union pipe_query_result subresult;

      accumulate_result(ctx, q->subquery, &subresult, false);
      q->subquery->curr_query = 0;
      if (q->type == PIPE_QUERY_PRIMITIVES_GENERATED)
         result->u64 += subresult.pipeline_statistics.ia_primitives;
   }

   if (write) {
      if (q->type == PIPE_QUERY_PIPELINE_STATISTICS) {
         results_stats[0].IAVertices = result->pipeline_statistics.ia_vertices;
         results_stats[0].IAPrimitives = result->pipeline_statistics.ia_primitives;
         results_stats[0].VSInvocations = result->pipeline_statistics.vs_invocations;
         results_stats[0].GSInvocations = result->pipeline_statistics.gs_invocations;
         results_stats[0].GSPrimitives = result->pipeline_statistics.gs_primitives;
         results_stats[0].CInvocations = result->pipeline_statistics.c_invocations;
         results_stats[0].CPrimitives = result->pipeline_statistics.c_primitives;
         results_stats[0].PSInvocations = result->pipeline_statistics.ps_invocations;
         results_stats[0].HSInvocations = result->pipeline_statistics.hs_invocations;
         results_stats[0].DSInvocations = result->pipeline_statistics.ds_invocations;
         results_stats[0].CSInvocations = result->pipeline_statistics.cs_invocations;
      } else if (q->type == PIPE_QUERY_SO_STATISTICS) {
         results_so[0].NumPrimitivesWritten = result->so_statistics.num_primitives_written;
         results_so[0].PrimitivesStorageNeeded = result->so_statistics.primitives_storage_needed;
      } else {
         if (unlikely(q->d3d12qtype == D3D12_QUERY_TYPE_TIMESTAMP)) {
            results_u64[0] = 0;
            results_u64[1] = result->u64;
         } else {
            results_u64[0] = result->u64;
         }
      }
   }

   pipe_buffer_unmap(&ctx->base, transfer);

   if (q->type == PIPE_QUERY_TIME_ELAPSED ||
       q->type == PIPE_QUERY_TIMESTAMP)
      result->u64 = static_cast<uint64_t>(screen->timestamp_multiplier * result->u64);

   return true;
}

static void
begin_query(struct d3d12_context *ctx, struct d3d12_query *q, bool restart)
{
   if (restart) {
      q->curr_query = 0;
   } else if (q->curr_query == q->num_queries) {
      union pipe_query_result result;

      /* Accumulate current results and store in first slot */
      d3d12_flush_cmdlist_and_wait(ctx);
      accumulate_result(ctx, q, &result, true);
      q->curr_query = 1;
   }

   if (q->subquery)
      begin_query(ctx, q->subquery, restart);

   ctx->cmdlist->BeginQuery(q->query_heap, q->d3d12qtype, q->curr_query);
}


static void
begin_timer_query(struct d3d12_context *ctx, struct d3d12_query *q, bool restart)
{
   /* For PIPE_QUERY_TIME_ELAPSED we record one time with BeginQuery and one in
    * EndQuery, so we need two query slots */
   unsigned query_index = 2 * q->curr_query;

   if (restart) {
      q->curr_query = 0;
      query_index = 0;
   } else if (query_index == q->num_queries) {
      union pipe_query_result result;

      /* Accumulate current results and store in first slot */
      d3d12_flush_cmdlist_and_wait(ctx);
      accumulate_result(ctx, q, &result, true);
      q->curr_query = 2;
   }

   ctx->cmdlist->EndQuery(q->query_heap, q->d3d12qtype, query_index);
}

static bool
d3d12_begin_query(struct pipe_context *pctx,
                  struct pipe_query *q)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)q;

   assert(query->type != PIPE_QUERY_TIMESTAMP);

   if (unlikely(query->type == PIPE_QUERY_TIME_ELAPSED))
      begin_timer_query(ctx, query, true);
   else {
      begin_query(ctx, query, true);
      list_addtail(&query->active_list, &ctx->active_queries);
   }

   return true;
}

static void
end_query(struct d3d12_context *ctx, struct d3d12_query *q)
{
   uint64_t offset = 0;
   struct d3d12_batch *batch = d3d12_current_batch(ctx);
   struct d3d12_resource *res = (struct d3d12_resource *)q->buffer;
   ID3D12Resource *d3d12_res = d3d12_resource_underlying(res, &offset);

   /* End subquery first so that we can use fence value from parent */
   if (q->subquery)
      end_query(ctx, q->subquery);

   /* With QUERY_TIME_ELAPSED we have recorded one value at
    * (2 * q->curr_query), and now we record a value at (2 * q->curr_query + 1)
    * and when resolving the query we subtract the latter from the former */

   unsigned resolve_count = q->type == PIPE_QUERY_TIME_ELAPSED ? 2 : 1;
   unsigned resolve_index = resolve_count * q->curr_query;
   unsigned end_index = resolve_index + resolve_count - 1;

   offset += q->buffer_offset + resolve_index * q->query_size;
   ctx->cmdlist->EndQuery(q->query_heap, q->d3d12qtype, end_index);
   d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_BIND_INVALIDATE_FULL);
   d3d12_apply_resource_states(ctx);
   ctx->cmdlist->ResolveQueryData(q->query_heap, q->d3d12qtype, resolve_index,
                                  resolve_count, d3d12_res, offset);

   d3d12_batch_reference_object(batch, q->query_heap);
   d3d12_batch_reference_resource(batch, res);

   assert(q->curr_query < q->num_queries);
   q->curr_query++;
}

static bool
d3d12_end_query(struct pipe_context *pctx,
               struct pipe_query *q)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)q;

   end_query(ctx, query);

   if (query->type != PIPE_QUERY_TIMESTAMP &&
       query->type != PIPE_QUERY_TIME_ELAPSED)
      list_delinit(&query->active_list);

   query->fence_value = ctx->fence_value;
   return true;
}

static bool
d3d12_get_query_result(struct pipe_context *pctx,
                      struct pipe_query *q,
                      bool wait,
                      union pipe_query_result *result)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)q;

   if (ctx->cmdqueue_fence->GetCompletedValue() < query->fence_value) {
      if (!wait)
         return false;
      d3d12_flush_cmdlist_and_wait(ctx);
   }

   return accumulate_result(ctx, query, result, false);
}

void
d3d12_suspend_queries(struct d3d12_context *ctx)
{
   list_for_each_entry(struct d3d12_query, query, &ctx->active_queries, active_list) {
      end_query(ctx, query);
   }
}

void
d3d12_resume_queries(struct d3d12_context *ctx)
{
   list_for_each_entry(struct d3d12_query, query, &ctx->active_queries, active_list) {
      begin_query(ctx, query, false);
   }
}

void
d3d12_validate_queries(struct d3d12_context *ctx)
{
   bool have_xfb = !!ctx->gfx_pipeline_state.num_so_targets;

   list_for_each_entry(struct d3d12_query, query, &ctx->active_queries, active_list) {
      if (query->type == PIPE_QUERY_PRIMITIVES_GENERATED && !have_xfb && !query->subquery) {
         struct pipe_query *subquery = d3d12_create_query(&ctx->base, PIPE_QUERY_PIPELINE_STATISTICS, 0);
         query->subquery = (struct d3d12_query *)subquery;
         if (!ctx->queries_disabled)
            begin_query(ctx, query->subquery, true);
      }
   }
}

static void
d3d12_set_active_query_state(struct pipe_context *pctx, bool enable)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   ctx->queries_disabled = !enable;

   if (enable)
      d3d12_resume_queries(ctx);
   else
      d3d12_suspend_queries(ctx);
}

static void
d3d12_render_condition(struct pipe_context *pctx,
                       struct pipe_query *pquery,
                       bool condition,
                       enum pipe_render_cond_flag mode)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   struct d3d12_query *query = (struct d3d12_query *)pquery;

   if (query == nullptr) {
      ctx->cmdlist->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);
      ctx->current_predication = nullptr;
      return;
   }

   if (!query->predicate)
      query->predicate = d3d12_resource(pipe_buffer_create(pctx->screen, 0,
                                                           PIPE_USAGE_DEFAULT, sizeof(uint64_t)));

   if (mode == PIPE_RENDER_COND_WAIT) {
      d3d12_flush_cmdlist_and_wait(ctx);
      union pipe_query_result result;
      accumulate_result(ctx, (d3d12_query *)pquery, &result, true);
   }

   struct d3d12_resource *res = (struct d3d12_resource *)query->buffer;
   d3d12_transition_resource_state(ctx, res, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_BIND_INVALIDATE_FULL);
   d3d12_transition_resource_state(ctx, query->predicate, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_BIND_INVALIDATE_NONE);
   d3d12_apply_resource_states(ctx);
   ctx->cmdlist->CopyBufferRegion(d3d12_resource_resource(query->predicate), 0,
                                  d3d12_resource_resource(res), 0,
                                  sizeof(uint64_t));

   d3d12_transition_resource_state(ctx, query->predicate, D3D12_RESOURCE_STATE_PREDICATION, D3D12_BIND_INVALIDATE_NONE);
   d3d12_apply_resource_states(ctx);

   ctx->current_predication = query->predicate;
   /* documentation of ID3D12GraphicsCommandList::SetPredication method:
    * "resource manipulation commands are _not_ actually performed
    *  if the resulting predicate data of the predicate is equal to
    *  the operation specified."
    */
   ctx->cmdlist->SetPredication(d3d12_resource_resource(query->predicate), 0,
                                condition ? D3D12_PREDICATION_OP_NOT_EQUAL_ZERO :
                                D3D12_PREDICATION_OP_EQUAL_ZERO);
}

void
d3d12_context_query_init(struct pipe_context *pctx)
{
   struct d3d12_context *ctx = d3d12_context(pctx);
   list_inithead(&ctx->active_queries);

   u_suballocator_init(&ctx->query_allocator, &ctx->base, 4096, 0, PIPE_USAGE_STAGING,
                         0, true);

   pctx->create_query = d3d12_create_query;
   pctx->destroy_query = d3d12_destroy_query;
   pctx->begin_query = d3d12_begin_query;
   pctx->end_query = d3d12_end_query;
   pctx->get_query_result = d3d12_get_query_result;
   pctx->set_active_query_state = d3d12_set_active_query_state;
   pctx->render_condition = d3d12_render_condition;
}
