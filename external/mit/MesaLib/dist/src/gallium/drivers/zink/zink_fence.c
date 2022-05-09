/*
 * Copyright 2018 Collabora Ltd.
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

#include "zink_batch.h"
#include "zink_context.h"
#include "zink_fence.h"

#include "zink_resource.h"
#include "zink_screen.h"

#include "util/set.h"
#include "util/u_memory.h"

static void
destroy_fence(struct zink_screen *screen, struct zink_tc_fence *mfence)
{
   mfence->fence = NULL;
   tc_unflushed_batch_token_reference(&mfence->tc_token, NULL);
   FREE(mfence);
}

struct zink_tc_fence *
zink_create_tc_fence(void)
{
   struct zink_tc_fence *mfence = CALLOC_STRUCT(zink_tc_fence);
   if (!mfence)
      return NULL;
   pipe_reference_init(&mfence->reference, 1);
   util_queue_fence_init(&mfence->ready);
   return mfence;
}

struct pipe_fence_handle *
zink_create_tc_fence_for_tc(struct pipe_context *pctx, struct tc_unflushed_batch_token *tc_token)
{
   struct zink_tc_fence *mfence = zink_create_tc_fence();
   if (!mfence)
      return NULL;
   util_queue_fence_reset(&mfence->ready);
   tc_unflushed_batch_token_reference(&mfence->tc_token, tc_token);
   return (struct pipe_fence_handle*)mfence;
}

void
zink_fence_reference(struct zink_screen *screen,
                     struct zink_tc_fence **ptr,
                     struct zink_tc_fence *mfence)
{
   if (pipe_reference(&(*ptr)->reference, &mfence->reference))
      destroy_fence(screen, *ptr);

   *ptr = mfence;
}

static void
fence_reference(struct pipe_screen *pscreen,
                struct pipe_fence_handle **pptr,
                struct pipe_fence_handle *pfence)
{
   zink_fence_reference(zink_screen(pscreen), (struct zink_tc_fence **)pptr,
                        zink_tc_fence(pfence));
}

static bool
tc_fence_finish(struct zink_context *ctx, struct zink_tc_fence *mfence, uint64_t *timeout_ns)
{
   if (!util_queue_fence_is_signalled(&mfence->ready)) {
      int64_t abs_timeout = os_time_get_absolute_timeout(*timeout_ns);
      if (mfence->tc_token) {
         /* Ensure that zink_flush will be called for
          * this mfence, but only if we're in the API thread
          * where the context is current.
          *
          * Note that the batch containing the flush may already
          * be in flight in the driver thread, so the mfence
          * may not be ready yet when this call returns.
          */
         threaded_context_flush(&ctx->base, mfence->tc_token, *timeout_ns == 0);
      }

      /* this is a tc mfence, so we're just waiting on the queue mfence to complete
       * after being signaled by the real mfence
       */
      if (*timeout_ns == PIPE_TIMEOUT_INFINITE) {
         util_queue_fence_wait(&mfence->ready);
      } else {
         if (!util_queue_fence_wait_timeout(&mfence->ready, abs_timeout))
            return false;
      }
      if (*timeout_ns && *timeout_ns != PIPE_TIMEOUT_INFINITE) {
         int64_t time_ns = os_time_get_nano();
         *timeout_ns = abs_timeout > time_ns ? abs_timeout - time_ns : 0;
      }
   }

   return true;
}

bool
zink_vkfence_wait(struct zink_screen *screen, struct zink_fence *fence, uint64_t timeout_ns)
{
   if (screen->device_lost)
      return true;
   if (p_atomic_read(&fence->completed))
      return true;

   assert(fence->batch_id);
   assert(fence->submitted);

   bool success = false;

   VkResult ret;
   if (timeout_ns)
      ret = VKSCR(WaitForFences)(screen->dev, 1, &fence->fence, VK_TRUE, timeout_ns);
   else
      ret = VKSCR(GetFenceStatus)(screen->dev, fence->fence);
   success = zink_screen_handle_vkresult(screen, ret);

   if (success) {
      p_atomic_set(&fence->completed, true);
      zink_batch_state(fence)->usage.usage = 0;
      zink_screen_update_last_finished(screen, fence->batch_id);
   }
   return success;
}

static bool
zink_fence_finish(struct zink_screen *screen, struct pipe_context *pctx, struct zink_tc_fence *mfence,
                  uint64_t timeout_ns)
{
   pctx = threaded_context_unwrap_sync(pctx);
   struct zink_context *ctx = zink_context(pctx);

   if (screen->device_lost)
      return true;

   if (pctx && mfence->deferred_ctx == pctx) {
      if (mfence->fence == ctx->deferred_fence) {
         zink_context(pctx)->batch.has_work = true;
         /* this must be the current batch */
         pctx->flush(pctx, NULL, !timeout_ns ? PIPE_FLUSH_ASYNC : 0);
         if (!timeout_ns)
            return false;
      }
   }

   /* need to ensure the tc mfence has been flushed before we wait */
   bool tc_finish = tc_fence_finish(ctx, mfence, &timeout_ns);
   /* the submit thread hasn't finished yet */
   if (!tc_finish)
      return false;
   /* this was an invalid flush, just return completed */
   if (!mfence->fence)
      return true;

   struct zink_fence *fence = mfence->fence;

   unsigned submit_diff = zink_batch_state(mfence->fence)->submit_count - mfence->submit_count;
   /* this batch is known to have finished because it has been submitted more than 1 time
    * since the tc fence last saw it
    */
   if (submit_diff > 1)
      return true;

   if (fence->submitted && zink_screen_check_last_finished(screen, fence->batch_id))
      return true;

   return zink_vkfence_wait(screen, fence, timeout_ns);
}

static bool
fence_finish(struct pipe_screen *pscreen, struct pipe_context *pctx,
                  struct pipe_fence_handle *pfence, uint64_t timeout_ns)
{
   return zink_fence_finish(zink_screen(pscreen), pctx, zink_tc_fence(pfence),
                            timeout_ns);
}

void
zink_fence_server_sync(struct pipe_context *pctx, struct pipe_fence_handle *pfence)
{
   struct zink_tc_fence *mfence = zink_tc_fence(pfence);

   if (mfence->deferred_ctx == pctx)
      return;

   if (mfence->deferred_ctx) {
      zink_context(pctx)->batch.has_work = true;
      /* this must be the current batch */
      pctx->flush(pctx, NULL, 0);
   }
   zink_fence_finish(zink_screen(pctx->screen), pctx, mfence, PIPE_TIMEOUT_INFINITE);
}

void
zink_screen_fence_init(struct pipe_screen *pscreen)
{
   pscreen->fence_reference = fence_reference;
   pscreen->fence_finish = fence_finish;
}
