/*
 * Copyright © 2011-2013 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifndef GEN6_COMMON_H
#define GEN6_COMMON_H

#include "sna.h"

#define NO_RING_SWITCH(sna) (!(sna)->kgem.has_semaphores)
#define PREFER_RENDER 0 /* -1 -> BLT, 1 -> RENDER */

static inline bool is_uncached(struct sna *sna,
			       struct kgem_bo *bo)
{
	return bo->io || (bo->scanout && !sna->kgem.has_wt);
}

inline static bool can_switch_to_blt(struct sna *sna,
				     struct kgem_bo *bo,
				     unsigned flags)
{
	if (sna->kgem.ring != KGEM_RENDER)
		return true;

	if (bo && RQ_IS_BLT(bo->rq))
		return true;

	if (bo && bo->tiling == I915_TILING_Y)
		return false;

	if (bo && !kgem_bo_can_blt(&sna->kgem, bo))
		return false;

	if (sna->render_state.gt < 2)
		return true;

	if (bo && RQ_IS_RENDER(bo->rq))
		return false;

	if (NO_RING_SWITCH(sna))
		return false;

	if (flags & COPY_LAST)
		return true;

	return kgem_ring_is_idle(&sna->kgem, KGEM_BLT);
}

static inline bool untiled_tlb_miss(struct kgem_bo *bo)
{
	if (kgem_bo_is_render(bo))
		return false;

	return bo->tiling == I915_TILING_NONE && bo->pitch >= 4096;
}

static int prefer_blt_bo(struct sna *sna,
			 struct kgem_bo *src,
			 struct kgem_bo *dst)
{
	assert(dst != NULL);

	if (PREFER_RENDER)
		return PREFER_RENDER < 0;

	if (dst->rq)
		return RQ_IS_BLT(dst->rq);

	if (sna->flags & SNA_POWERSAVE)
		return true;

	if (src) {
		if (sna->render_state.gt > 1)
			return false;

		if (src->rq)
			return RQ_IS_BLT(src->rq);

		if (src->tiling == I915_TILING_Y)
			return false;
        } else {
                if (sna->render_state.gt > 2)
                        return false;
        }

	if (sna->render_state.gt < 2)
		return true;

	return dst->tiling == I915_TILING_NONE || is_uncached(sna, dst);
}

inline static bool force_blt_ring(struct sna *sna, struct kgem_bo *bo)
{
	if (sna->kgem.mode == KGEM_RENDER)
		return false;

	if (NO_RING_SWITCH(sna))
		return sna->kgem.ring == KGEM_BLT;

	if (bo->tiling == I915_TILING_Y)
		return false;

	if (sna->flags & SNA_POWERSAVE)
		return true;

	if (sna->render_state.gt < 2)
		return true;

	return false;
}

nonnull inline static bool
prefer_blt_ring(struct sna *sna, struct kgem_bo *bo, unsigned flags)
{
	if (PREFER_RENDER)
		return PREFER_RENDER < 0;

	assert(!force_blt_ring(sna, bo));
	assert(!kgem_bo_is_render(bo) || NO_RING_SWITCH(sna));

	if (kgem_bo_is_blt(bo))
		return true;

	return can_switch_to_blt(sna, bo, flags);
}

nonnull inline static bool
prefer_render_ring(struct sna *sna, struct kgem_bo *bo)
{
	if (sna->kgem.ring == KGEM_RENDER)
		return true;

	if (sna->kgem.ring != KGEM_NONE && NO_RING_SWITCH(sna))
                return false;

	if (kgem_bo_is_render(bo))
		return true;

	if (sna->flags & SNA_POWERSAVE)
		return false;

	if (!prefer_blt_bo(sna, NULL, bo))
		return true;

	return !kgem_ring_is_idle(&sna->kgem, KGEM_RENDER);
}

inline static bool
prefer_blt_composite(struct sna *sna, struct sna_composite_op *tmp)
{
	if (PREFER_RENDER)
		return PREFER_RENDER < 0;

	if (untiled_tlb_miss(tmp->dst.bo) ||
	    untiled_tlb_miss(tmp->src.bo))
		return true;

	if (force_blt_ring(sna, tmp->dst.bo))
		return true;

	if (prefer_render_ring(sna, tmp->dst.bo))
		return false;

	if (!prefer_blt_ring(sna, tmp->dst.bo, 0))
		return false;

	return prefer_blt_bo(sna, tmp->src.bo, tmp->dst.bo);
}

nonnull static inline bool
prefer_blt_fill(struct sna *sna, struct kgem_bo *bo, unsigned flags)
{
	if (PREFER_RENDER)
		return PREFER_RENDER < 0;

	if (untiled_tlb_miss(bo))
		return true;

	if (force_blt_ring(sna, bo))
		return true;

	if ((flags & (FILL_POINTS | FILL_SPANS)) == 0) {
		if (prefer_render_ring(sna, bo))
			return false;

		if (!prefer_blt_ring(sna, bo, 0))
			return false;
	} else {
	    if (can_switch_to_blt(sna, bo, COPY_LAST))
		    return true;
	}

	return prefer_blt_bo(sna, NULL, bo);
}

void gen6_render_context_switch(struct kgem *kgem, int new_mode);
void gen6_render_retire(struct kgem *kgem);

#endif /* GEN6_COMMON_H */
