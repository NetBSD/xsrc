/*
 * Copyright (C) 2012-2018 Rob Clark <robclark@freedesktop.org>
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

#ifndef MSM_PRIV_H_
#define MSM_PRIV_H_

#include "freedreno_priv.h"

#include "util/slab.h"
#include "util/timespec.h"

#include "pipe/p_defines.h"

#ifndef __user
#define __user
#endif

#include "drm-uapi/msm_drm.h"

struct msm_device {
   struct fd_device base;
   struct util_queue submit_queue;
};
FD_DEFINE_CAST(fd_device, msm_device);

struct fd_device *msm_device_new(int fd, drmVersionPtr version);

struct msm_pipe {
   struct fd_pipe base;
   uint32_t pipe;
   uint32_t gpu_id;
   uint64_t chip_id;
   uint64_t gmem_base;
   uint32_t gmem;
   uint32_t queue_id;
   struct slab_parent_pool ring_pool;

   /* BO for suballocating long-lived objects on the pipe. */
   struct fd_bo *suballoc_bo;
   uint32_t suballoc_offset;

   /**
    * The last fence seqno that was flushed to kernel (doesn't mean that it
    * is complete, just that the kernel knows about it)
    */
   uint32_t last_submit_fence;

   uint32_t last_enqueue_fence;   /* just for debugging */

   /**
    * If we *ever* see an in-fence-fd, assume that userspace is
    * not relying on implicit fences.
    */
   bool no_implicit_sync;
};
FD_DEFINE_CAST(fd_pipe, msm_pipe);

struct fd_pipe *msm_pipe_new(struct fd_device *dev, enum fd_pipe_id id,
                             uint32_t prio);

struct fd_ringbuffer *msm_ringbuffer_new_object(struct fd_pipe *pipe,
                                                uint32_t size);
struct fd_ringbuffer *msm_ringbuffer_sp_new_object(struct fd_pipe *pipe,
                                                   uint32_t size);

struct fd_submit *msm_submit_new(struct fd_pipe *pipe);
struct fd_submit *msm_submit_sp_new(struct fd_pipe *pipe);
void msm_pipe_sp_flush(struct fd_pipe *pipe, uint32_t fence);

void msm_pipe_sp_ringpool_init(struct msm_pipe *msm_pipe);
void msm_pipe_sp_ringpool_fini(struct msm_pipe *msm_pipe);

struct msm_bo {
   struct fd_bo base;
   uint64_t offset;
   uint32_t idx;
};
FD_DEFINE_CAST(fd_bo, msm_bo);

int msm_bo_new_handle(struct fd_device *dev, uint32_t size, uint32_t flags,
                      uint32_t *handle);
struct fd_bo *msm_bo_from_handle(struct fd_device *dev, uint32_t size,
                                 uint32_t handle);

static inline void
msm_dump_submit(struct drm_msm_gem_submit *req)
{
   for (unsigned i = 0; i < req->nr_bos; i++) {
      struct drm_msm_gem_submit_bo *bos = U642VOID(req->bos);
      struct drm_msm_gem_submit_bo *bo = &bos[i];
      ERROR_MSG("  bos[%d]: handle=%u, flags=%x", i, bo->handle, bo->flags);
   }
   for (unsigned i = 0; i < req->nr_cmds; i++) {
      struct drm_msm_gem_submit_cmd *cmds = U642VOID(req->cmds);
      struct drm_msm_gem_submit_cmd *cmd = &cmds[i];
      struct drm_msm_gem_submit_reloc *relocs = U642VOID(cmd->relocs);
      ERROR_MSG("  cmd[%d]: type=%u, submit_idx=%u, submit_offset=%u, size=%u",
                i, cmd->type, cmd->submit_idx, cmd->submit_offset, cmd->size);
      for (unsigned j = 0; j < cmd->nr_relocs; j++) {
         struct drm_msm_gem_submit_reloc *r = &relocs[j];
         ERROR_MSG(
            "    reloc[%d]: submit_offset=%u, or=%08x, shift=%d, reloc_idx=%u"
            ", reloc_offset=%" PRIu64,
            j, r->submit_offset, r->or, r->shift, r->reloc_idx,
            (uint64_t)r->reloc_offset);
      }
   }
}

static inline void
get_abs_timeout(struct drm_msm_timespec *tv, uint64_t ns)
{
   struct timespec t;

   if (ns == PIPE_TIMEOUT_INFINITE)
      ns = 3600ULL * NSEC_PER_SEC; /* 1 hour timeout is almost infinite */

   clock_gettime(CLOCK_MONOTONIC, &t);
   tv->tv_sec = t.tv_sec + ns / NSEC_PER_SEC;
   tv->tv_nsec = t.tv_nsec + ns % NSEC_PER_SEC;
   if (tv->tv_nsec >= NSEC_PER_SEC) { /* handle nsec overflow */
      tv->tv_nsec -= NSEC_PER_SEC;
      tv->tv_sec++;
   }
}

#endif /* MSM_PRIV_H_ */
