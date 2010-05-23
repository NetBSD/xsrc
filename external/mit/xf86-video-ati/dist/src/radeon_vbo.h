
#ifndef RADEON_VBO_H
#define RADEON_VBO_H

extern void r600_vb_no_space(ScrnInfoPtr pScrn, int vert_size);
extern void radeon_vbo_init_lists(ScrnInfoPtr pScrn);
extern void radeon_vbo_free_lists(ScrnInfoPtr pScrn);
extern void radeon_vbo_flush_bos(ScrnInfoPtr pScrn);
extern void radeon_vbo_get(ScrnInfoPtr pScrn);
extern void radeon_vbo_put(ScrnInfoPtr pScrn);

static inline void radeon_vbo_check(ScrnInfoPtr pScrn, int vert_size)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct radeon_accel_state *accel_state = info->accel_state;

    if ((accel_state->vb_offset + (3 * vert_size)) > accel_state->vb_total) {
	r600_vb_no_space(pScrn, vert_size);
    }
}

static inline void *
radeon_vbo_space(ScrnInfoPtr pScrn, int vert_size)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct radeon_accel_state *accel_state = info->accel_state;
    void *vb;
    
    /* we've ran out of space in the vertex buffer - need to get a
       new one */
    if ((accel_state->vb_offset + (3 * vert_size)) > accel_state->vb_total) {
	r600_vb_no_space(pScrn, vert_size);
    }
    accel_state->vb_op_vert_size = vert_size;
#if defined(XF86DRM_MODE)
    if (info->cs) {
	int ret;
	struct radeon_bo *bo = accel_state->vb_bo;

	if (!bo->ptr) {
	    ret = radeon_bo_map(bo, 1);
	    if (ret) {
		FatalError("Failed to map vb %d\n", ret);
		return NULL;
	    }
	}
	vb = (pointer)((char *)bo->ptr + accel_state->vb_offset);
    } else
#endif
	vb = (pointer)((char *)accel_state->vb_ptr + accel_state->vb_offset);
    return vb;
}

static inline void radeon_vbo_commit(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct radeon_accel_state *accel_state = info->accel_state;

    accel_state->vb_offset += 3 * accel_state->vb_op_vert_size;
}

#endif
