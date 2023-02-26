/*
 * Copyright 2010 Red Hat, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qxl.h"
#include "qxl_surface.h"/* send anything pending to the other side */


enum ROPDescriptor
{
    ROPD_INVERS_SRC = (1 << 0),
    ROPD_INVERS_BRUSH = (1 << 1),
    ROPD_INVERS_DEST = (1 << 2),
    ROPD_OP_PUT = (1 << 3),
    ROPD_OP_OR = (1 << 4),
    ROPD_OP_AND = (1 << 5),
    ROPD_OP_XOR = (1 << 6),
    ROPD_OP_BLACKNESS = (1 << 7),
    ROPD_OP_WHITENESS = (1 << 8),
    ROPD_OP_INVERS = (1 << 9),
    ROPD_INVERS_RES = (1 <<10),
};

static struct qxl_bo *
make_drawable (qxl_screen_t *qxl, qxl_surface_t *surf, uint8_t type,
	       const struct QXLRect *rect
	       /* , pRegion clip */)
{
    struct QXLDrawable *drawable;
    struct qxl_bo *draw_bo;
    int i;
   
    draw_bo = qxl->bo_funcs->cmd_alloc (qxl, sizeof *drawable, "drawable command");
    assert(draw_bo);
    drawable = qxl->bo_funcs->bo_map(draw_bo);
    assert(drawable);
    
    drawable->release_info.id = pointer_to_u64 (draw_bo);
    
    drawable->type = type;
    
    qxl->bo_funcs->bo_output_surf_reloc(qxl, offsetof(struct QXLDrawable, surface_id), draw_bo, surf);

    drawable->effect = QXL_EFFECT_OPAQUE;
    drawable->self_bitmap = 0;
    drawable->self_bitmap_area.top = 0;
    drawable->self_bitmap_area.left = 0;
    drawable->self_bitmap_area.bottom = 0;
    drawable->self_bitmap_area.right = 0;
    /* FIXME: add clipping */
    drawable->clip.type = SPICE_CLIP_TYPE_NONE;
    
    /*
     * surfaces_dest[i] should apparently be filled out with the
     * surfaces that we depend on, and surface_rects should be
     * filled with the rectangles of those surfaces that we
     * are going to use.
     */
    for (i = 0; i < 3; ++i)
	drawable->surfaces_dest[i] = -1;
    
    if (rect)
	drawable->bbox = *rect;
    
    /* No longer needed since spice-server commit c541d7e29 */
    if (!qxl->kms_enabled)
        drawable->mm_time = qxl->rom->mm_clock;
    else
        drawable->mm_time = 0;

    qxl->bo_funcs->bo_unmap(draw_bo);
    return draw_bo;
}

static void
push_drawable (qxl_screen_t *qxl, struct qxl_bo *drawable_bo)
{
    qxl->bo_funcs->write_command (qxl, QXL_CMD_DRAW, drawable_bo);
}

static void
submit_fill (qxl_screen_t *qxl, qxl_surface_t *surf,
	     const struct QXLRect *rect, uint32_t color)
{
    struct qxl_bo *drawable_bo;
    struct QXLDrawable *drawable;
    
    drawable_bo = make_drawable (qxl, surf, QXL_DRAW_FILL, rect);
    
    drawable = qxl->bo_funcs->bo_map(drawable_bo);
    drawable->u.fill.brush.type = SPICE_BRUSH_TYPE_SOLID;
    drawable->u.fill.brush.u.color = color;
    drawable->u.fill.rop_descriptor = ROPD_OP_PUT;
    drawable->u.fill.mask.flags = 0;
    drawable->u.fill.mask.pos.x = 0;
    drawable->u.fill.mask.pos.y = 0;
    drawable->u.fill.mask.bitmap = 0;
    
    qxl->bo_funcs->bo_unmap(drawable_bo);

    push_drawable (qxl, drawable_bo);
}

void
qxl_surface_flush (qxl_surface_t *surface)
{
    ;
}

/* access */
static void
download_box_no_update (qxl_surface_t *surface, int x1, int y1, int x2, int y2)
{
    pixman_image_composite (PIXMAN_OP_SRC,
                            surface->dev_image,
                            NULL,
                            surface->host_image,
                            x1, y1, 0, 0, x1, y1, x2 - x1, y2 - y1);
}

void
qxl_download_box (qxl_surface_t *surface, int x1, int y1, int x2, int y2)
{
    assert (x2 >= x1 && y2 >= y1);

    if (x1 == x2 || y1 == y2)
        return;

    surface->qxl->bo_funcs->update_area(surface, x1, y1, x2, y2);

    download_box_no_update(surface, x1, y1, x2, y2);
}

Bool
qxl_surface_prepare_access (qxl_surface_t  *surface,
			    PixmapPtr       pixmap,
			    RegionPtr       region,
			    uxa_access_t    access)
{
    int n_boxes;
    BoxPtr boxes;
    ScreenPtr pScreen = pixmap->drawable.pScreen;
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    RegionRec new;

    if (!pScrn->vtSema)
        return FALSE;

    REGION_INIT (NULL, &new, (BoxPtr)NULL, 0);
    REGION_SUBTRACT (NULL, &new, region, &surface->access_region);

    if (access == UXA_ACCESS_RW)
	surface->access_type = UXA_ACCESS_RW;
    
    region = &new;
    
    n_boxes = REGION_NUM_RECTS (region);
    boxes = REGION_RECTS (region);

    if (n_boxes < 25)
    {
	while (n_boxes--)
	{
	    qxl_download_box (surface, boxes->x1, boxes->y1, boxes->x2, boxes->y2);
	    
	    boxes++;
	}
    }
    else
    {
	qxl_download_box (
	    surface,
	    new.extents.x1, new.extents.y1, new.extents.x2, new.extents.y2);
    }
    
    REGION_UNION (pScreen,
		  &(surface->access_region),
		  &(surface->access_region),
		      region);
    
    REGION_UNINIT (NULL, &new);
    
    pScreen->ModifyPixmapHeader(
	pixmap,
	pixmap->drawable.width,
	pixmap->drawable.height,
	-1, -1, -1,
	pixman_image_get_data (surface->host_image));

    pixmap->devKind = pixman_image_get_stride (surface->host_image);
    
    return TRUE;
}

static void
translate_rect (struct QXLRect *rect)
{
    rect->right -= rect->left;
    rect->bottom -= rect->top;
    rect->left = rect->top = 0;
}

static void
real_upload_box (qxl_surface_t *surface, int x1, int y1, int x2, int y2)
{
    struct QXLRect rect;
    struct QXLDrawable *drawable;
    struct qxl_bo *image_bo, *drawable_bo;
    qxl_screen_t *qxl = surface->qxl;
    uint32_t *data;
    int stride;
    
    rect.left = x1;
    rect.right = x2;
    rect.top = y1;
    rect.bottom = y2;
    
    drawable_bo = make_drawable (qxl, surface, QXL_DRAW_COPY, &rect);
    drawable = qxl->bo_funcs->bo_map(drawable_bo);
    drawable->u.copy.src_area = rect;
    translate_rect (&drawable->u.copy.src_area);
    drawable->u.copy.rop_descriptor = ROPD_OP_PUT;
    drawable->u.copy.scale_mode = 0;
    drawable->u.copy.mask.flags = 0;
    drawable->u.copy.mask.pos.x = 0;
    drawable->u.copy.mask.pos.y = 0;
    drawable->u.copy.mask.bitmap = 0;

    qxl->bo_funcs->bo_unmap(drawable_bo);

    data = pixman_image_get_data (surface->host_image);
    stride = pixman_image_get_stride (surface->host_image);
    
    image_bo = qxl_image_create (
	qxl, (const uint8_t *)data, x1, y1, x2 - x1, y2 - y1, stride, 
	surface->bpp == 24 ? 4 : surface->bpp / 8, TRUE);
    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.copy.src_bitmap),
				   drawable_bo, image_bo);
    push_drawable (qxl, drawable_bo);

    qxl->bo_funcs->bo_decref(qxl, image_bo);
}

#define TILE_WIDTH 512
#define TILE_HEIGHT 512

void
qxl_upload_box (qxl_surface_t *surface, int x1, int y1, int x2, int y2)
{
    int tile_x1, tile_y1;

    for (tile_y1 = y1; tile_y1 < y2; tile_y1 += TILE_HEIGHT)
    {
	for (tile_x1 = x1; tile_x1 < x2; tile_x1 += TILE_WIDTH)
	{
	    int tile_x2 = tile_x1 + TILE_WIDTH;
	    int tile_y2 = tile_y1 + TILE_HEIGHT;

	    if (tile_x2 > x2)
		tile_x2 = x2;
	    if (tile_y2 > y2)
		tile_y2 = y2;

	    real_upload_box (surface, tile_x1, tile_y1, tile_x2, tile_y2);
	}
    }
}

static void
upload_one_primary_region(qxl_screen_t *qxl, PixmapPtr pixmap, BoxPtr b)
{
    struct QXLRect rect;
    struct qxl_bo *drawable_bo, *image_bo;
    struct QXLDrawable *drawable;
    FbBits *data;
    int stride;
    int bpp;

    if (b->x1 >= qxl->virtual_x || b->y1 >= qxl->virtual_y)
        return;

    rect.left = b->x1;
    rect.right = min(b->x2, qxl->virtual_x);
    rect.top = b->y1;
    rect.bottom = min(b->y2, qxl->virtual_y);

    drawable_bo = make_drawable (qxl, qxl->primary, QXL_DRAW_COPY, &rect);
    drawable = qxl->bo_funcs->bo_map(drawable_bo);
    drawable->u.copy.src_area = rect;
    translate_rect (&drawable->u.copy.src_area);
    drawable->u.copy.rop_descriptor = ROPD_OP_PUT;
    drawable->u.copy.scale_mode = 0;
    drawable->u.copy.mask.flags = 0;
    drawable->u.copy.mask.pos.x = 0;
    drawable->u.copy.mask.pos.y = 0;
    drawable->u.copy.mask.bitmap = 0;
    qxl->bo_funcs->bo_unmap(drawable_bo);

    fbGetPixmapBitsData(pixmap, data, stride, bpp);
    image_bo = qxl_image_create (
	qxl, (const uint8_t *)data, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, stride * sizeof(*data),
	bpp == 24 ? 4 : bpp / 8, TRUE);
    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.copy.src_bitmap),
				   drawable_bo, image_bo);

    push_drawable (qxl, drawable_bo);
    qxl->bo_funcs->bo_decref(qxl, image_bo);
}

void
qxl_surface_upload_primary_regions(qxl_screen_t *qxl, PixmapPtr pixmap, RegionRec *r)
{
    int n_boxes;
    BoxPtr boxes;

    n_boxes = RegionNumRects(r);
    boxes = RegionRects(r);

    while (n_boxes--)
    {
        upload_one_primary_region(qxl, pixmap, boxes);
        boxes++;
    }
}

void
qxl_surface_finish_access (qxl_surface_t *surface, PixmapPtr pixmap)
{
    ScreenPtr pScreen = pixmap->drawable.pScreen;
    int w = pixmap->drawable.width;
    int h = pixmap->drawable.height;
    int n_boxes;
    BoxPtr boxes;

    n_boxes = REGION_NUM_RECTS (&surface->access_region);
    boxes = REGION_RECTS (&surface->access_region);

    if (surface->access_type == UXA_ACCESS_RW)
    {
	if (n_boxes < 25)
	{
	    while (n_boxes--)
	    {
		qxl_upload_box (surface, boxes->x1, boxes->y1, boxes->x2, boxes->y2);
		
		boxes++;
	    }
	}
	else
	{
	    qxl_upload_box (surface,
			surface->access_region.extents.x1,
			surface->access_region.extents.y1,
			surface->access_region.extents.x2,
			surface->access_region.extents.y2);
	}
    }

    REGION_EMPTY (pScreen, &surface->access_region);
    surface->access_type = UXA_ACCESS_RO;
    
    pScreen->ModifyPixmapHeader(pixmap, w, h, -1, -1, 0, NULL);
}


#ifdef DEBUG_REGIONS
static void
print_region (const char *header, RegionPtr pRegion)
{
    int nbox = REGION_NUM_RECTS (pRegion);
    BoxPtr pbox = REGION_RECTS (pRegion);
    
    ErrorF ("%s", header);

    if (nbox == 0)
	ErrorF (" (empty)\n");
    else
	ErrorF ("\n");
    
    while (nbox--)
    {
	ErrorF ("   %d %d %d %d (size: %d %d)\n",
		pbox->x1, pbox->y1, pbox->x2, pbox->y2,
		pbox->x2 - pbox->x1, pbox->y2 - pbox->y1);
	
	pbox++;
    }
}
#endif // DEBUG_REGIONS

/* solid */
Bool
qxl_surface_prepare_solid (qxl_surface_t *destination,
			   Pixel	  fg)
{
    if (!REGION_NIL (&(destination->access_region)))
    {
	ErrorF (" solid not in vmem\n");
    }

#ifdef DEBUG_REGIONS
    print_region ("prepare solid", &(destination->access_region));
#endif
    
    destination->u.solid_pixel = fg; //  ^ (rand() >> 16);

    return TRUE;
}

void
qxl_surface_solid (qxl_surface_t *destination,
		   int	          x1,
		   int	          y1,
		   int	          x2,
		   int	          y2)
{
    qxl_screen_t *qxl = destination->qxl;
    struct QXLRect qrect;
    uint32_t p;

    qrect.top = y1;
    qrect.bottom = y2;
    qrect.left = x1;
    qrect.right = x2;

    p = destination->u.solid_pixel;
    
    submit_fill (qxl, destination, &qrect, p);
}

/* copy */
Bool
qxl_surface_prepare_copy (qxl_surface_t *dest,
			  qxl_surface_t *source)
{
    if (!REGION_NIL (&(dest->access_region))	||
	!REGION_NIL (&(source->access_region)))
    {
	return FALSE;
    }

    dest->u.copy_src = source;

    return TRUE;
}


static struct qxl_bo *
image_from_surface_internal(qxl_screen_t *qxl,
			    qxl_surface_t *surface)
{
    struct qxl_bo *image_bo = qxl->bo_funcs->bo_alloc (qxl, sizeof(struct QXLImage), "image struct for surface");
    struct QXLImage *image = qxl->bo_funcs->bo_map(image_bo);

    image->descriptor.id = 0;
    image->descriptor.type = SPICE_IMAGE_TYPE_SURFACE;
    image->descriptor.width = 0;
    image->descriptor.height = 0;
    qxl->bo_funcs->bo_unmap(image_bo);
    return image_bo;
}

static struct qxl_bo *image_from_surface(qxl_screen_t *qxl, qxl_surface_t *dest)
{
    if (!dest->image_bo)
      dest->image_bo = image_from_surface_internal(qxl, dest);

    qxl->bo_funcs->bo_incref(qxl, dest->image_bo);
    qxl->bo_funcs->bo_output_surf_reloc(qxl, offsetof(struct QXLImage, surface_image.surface_id), dest->image_bo, dest);

    return dest->image_bo;
}

void
qxl_surface_copy (qxl_surface_t *dest,
		  int  src_x1, int src_y1,
		  int  dest_x1, int dest_y1,
		  int width, int height)
{
    qxl_screen_t *qxl = dest->qxl;
    struct qxl_bo *drawable_bo;
    struct QXLDrawable *drawable;
    struct QXLRect qrect;

#ifdef DEBUG_REGIONS
    print_region (" copy src", &(dest->u.copy_src->access_region));
    print_region (" copy dest", &(dest->access_region));
#endif

    qrect.top = dest_y1;
    qrect.bottom = dest_y1 + height;
    qrect.left = dest_x1;
    qrect.right = dest_x1 + width;
    
    if (dest->id == dest->u.copy_src->id)
    {
	drawable_bo = make_drawable (qxl, dest, QXL_COPY_BITS, &qrect);

	drawable = qxl->bo_funcs->bo_map(drawable_bo);
	drawable->u.copy_bits.src_pos.x = src_x1;
	drawable->u.copy_bits.src_pos.y = src_y1;
	qxl->bo_funcs->bo_unmap(drawable_bo);

	push_drawable (qxl, drawable_bo);

    }
    else
    {
	struct qxl_bo *image_bo;

	dest->u.copy_src->ref_count++;

	image_bo = image_from_surface(qxl, dest->u.copy_src);

	drawable_bo = make_drawable (qxl, dest, QXL_DRAW_COPY, &qrect);

	drawable = qxl->bo_funcs->bo_map(drawable_bo);
	qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.copy.src_bitmap),
				       drawable_bo, image_bo);
	drawable->u.copy.src_area.left = src_x1;
	drawable->u.copy.src_area.top = src_y1;
	drawable->u.copy.src_area.right = src_x1 + width;
	drawable->u.copy.src_area.bottom = src_y1 + height;
	drawable->u.copy.rop_descriptor = ROPD_OP_PUT;
	drawable->u.copy.scale_mode = 0;
	drawable->u.copy.mask.flags = 0;
	drawable->u.copy.mask.pos.x = 0;
	drawable->u.copy.mask.pos.y = 0;
	drawable->u.copy.mask.bitmap = 0;

	qxl->bo_funcs->bo_output_surf_reloc(qxl, offsetof(struct QXLDrawable, surfaces_dest[0]), drawable_bo, dest->u.copy_src);
	drawable->surfaces_rects[0] = drawable->u.copy.src_area;
 	
	assert (src_x1 >= 0);
	assert (src_y1 >= 0);

	if (width > pixman_image_get_width (dest->u.copy_src->host_image))
	{
	    ErrorF ("dest w: %d   src w: %d\n",
		    width, pixman_image_get_width (dest->u.copy_src->host_image));
	}
	
	assert (width <= pixman_image_get_width (dest->u.copy_src->host_image));
	assert (height <= pixman_image_get_height (dest->u.copy_src->host_image));

	qxl->bo_funcs->bo_unmap(drawable_bo);
	push_drawable (qxl, drawable_bo);
	qxl->bo_funcs->bo_decref(qxl, image_bo);
    }
}

/* composite */
Bool
qxl_surface_prepare_composite (int op,
			       PicturePtr	src_picture,
			       PicturePtr	mask_picture,
			       PicturePtr	dest_picture,
			       qxl_surface_t *	src,
			       qxl_surface_t *	mask,
			       qxl_surface_t *	dest)
{
    dest->u.composite.op = op;
    dest->u.composite.src_picture = src_picture;
    dest->u.composite.mask_picture = mask_picture;
    dest->u.composite.dest_picture = dest_picture;
    dest->u.composite.src = src;
    dest->u.composite.mask = mask;
    dest->u.composite.dest = dest;
    
    return TRUE;
}

static struct qxl_bo *
image_from_picture (qxl_screen_t *qxl,
		    PicturePtr picture,
		    qxl_surface_t *surface,
		    int *force_opaque)
{
    if (picture->format == PICT_x8r8g8b8)
	*force_opaque = TRUE;
    else
	*force_opaque = FALSE;

    return image_from_surface(qxl, surface);
}

static struct qxl_bo *
get_transform (qxl_screen_t *qxl, PictTransform *transform)
{
    if (transform)
    {
	struct qxl_bo *qxform_bo = qxl->bo_funcs->bo_alloc (qxl, sizeof (QXLTransform), "transform");
	QXLTransform *qxform = qxl->bo_funcs->bo_map(qxform_bo);

	qxform->t00 = transform->matrix[0][0];
	qxform->t01 = transform->matrix[0][1];
	qxform->t02 = transform->matrix[0][2];
	qxform->t10 = transform->matrix[1][0];
	qxform->t11 = transform->matrix[1][1];
	qxform->t12 = transform->matrix[1][2];

	qxl->bo_funcs->bo_unmap(qxform_bo);
	return qxform_bo;
    }
    else
    {
	return NULL;
    }
}

static QXLRect
full_rect (qxl_surface_t *surface)
{
    QXLRect r;
    int w = pixman_image_get_width (surface->host_image);
    int h = pixman_image_get_height (surface->host_image);
	    
    r.left = r.top = 0;
    r.right = w;
    r.bottom = h;

    return r;
}

void
qxl_surface_composite (qxl_surface_t *dest,
		       int src_x, int src_y,
		       int mask_x, int mask_y,
		       int dest_x, int dest_y,
		       int width, int height)
{
    qxl_screen_t *qxl = dest->qxl;
    PicturePtr src = dest->u.composite.src_picture;
    qxl_surface_t *qsrc = dest->u.composite.src;
    PicturePtr mask = dest->u.composite.mask_picture;
    qxl_surface_t *qmask = dest->u.composite.mask;
    int op = dest->u.composite.op;
    struct QXLDrawable *drawable;
    struct qxl_bo *drawable_bo;
    QXLComposite *composite;
    QXLRect rect;
    struct qxl_bo *trans_bo, *img_bo;
    int n_deps = 0;
    int force_opaque;
    struct qxl_bo *derefs[4];
    int n_derefs = 0, i;
#if 0
    ErrorF ("QXL Composite: src:       %x (%d %d) id: %d; \n"
	    "               mask:      id: %d\n"
	    "               dest:      %x %d %d %d %d (id: %d)\n",
	    dest->u.composite.src_picture->format,
	    dest->u.composite.src_picture->pDrawable->width,
	    dest->u.composite.src_picture->pDrawable->height,
	    dest->u.composite.src->id,
	    dest->u.composite.mask? dest->u.composite.mask->id : -1,
	    dest->u.composite.dest_picture->format,
	    dest_x, dest_y, width, height,
	    dest->id
	);
#endif

    rect.left = dest_x;
    rect.right = dest_x + width;
    rect.top = dest_y;
    rect.bottom = dest_y + height;
    
    drawable_bo = make_drawable (qxl, dest, QXL_DRAW_COMPOSITE, &rect);

    drawable = qxl->bo_funcs->bo_map(drawable_bo);

    composite = &drawable->u.composite;

    composite->flags = 0;

    if (dest->u.composite.dest_picture->format == PICT_x8r8g8b8)
	composite->flags |= SPICE_COMPOSITE_DEST_OPAQUE;
    
    composite->flags |= (op & 0xff);

    img_bo = image_from_picture (qxl, src, qsrc, &force_opaque);
    if (force_opaque)
	composite->flags |= SPICE_COMPOSITE_SOURCE_OPAQUE;
    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.composite.src),
				   drawable_bo, img_bo);
    derefs[n_derefs++] = img_bo;

    composite->flags |= (src->filter << 8);
    composite->flags |= (src->repeat << 14);
    trans_bo = get_transform (qxl, src->transform);
    if (trans_bo) {
	qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.composite.src_transform),
				       drawable_bo, trans_bo);
	derefs[n_derefs++] = trans_bo;
    } else
	composite->src_transform = 0;

    qxl->bo_funcs->bo_output_surf_reloc(qxl, offsetof(struct QXLDrawable, surfaces_dest[n_deps]), drawable_bo, qsrc);
    drawable->surfaces_rects[n_deps] = full_rect (qsrc);

    n_deps++;
    
    if (mask)
    {
	img_bo = image_from_picture (qxl, mask, qmask, &force_opaque);
	if (force_opaque)
	    composite->flags |= SPICE_COMPOSITE_MASK_OPAQUE;

	qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.composite.mask),
				       drawable_bo, img_bo);
	derefs[n_derefs++] = img_bo;
	composite->flags |= (mask->filter << 11);
	composite->flags |= (mask->repeat << 16);
	composite->flags |= (mask->componentAlpha << 18);

	qxl->bo_funcs->bo_output_surf_reloc(qxl, offsetof(struct QXLDrawable, surfaces_dest[n_deps]), drawable_bo, qmask);
	drawable->surfaces_rects[n_deps] = full_rect (qmask);
	n_deps++;
	
	trans_bo = get_transform (qxl, src->transform);
	if (trans_bo) {
	    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.composite.mask_transform),
					   drawable_bo, trans_bo);
	    derefs[n_derefs++] = trans_bo;
	}
	else
	  composite->mask_transform = 0;
    }
    else
    {
	composite->mask = 0x00000000;
	composite->mask_transform = 0x00000000;
    }

    qxl->bo_funcs->bo_output_surf_reloc(qxl, offsetof(struct QXLDrawable, surfaces_dest[n_deps]), drawable_bo, dest);
    drawable->surfaces_rects[n_deps] = full_rect (dest);
    
    composite->src_origin.x = src_x;
    composite->src_origin.y = src_y;
    composite->mask_origin.x = mask_x;
    composite->mask_origin.y = mask_y;

    drawable->effect = QXL_EFFECT_BLEND;
    
    qxl->bo_funcs->bo_unmap(drawable_bo);
    push_drawable (qxl, drawable_bo);

    for (i = 0; i < n_derefs; i++)
      qxl->bo_funcs->bo_decref(qxl, derefs[i]);
}

Bool
qxl_surface_put_image (qxl_surface_t *dest,
		       int x, int y, int width, int height,
		       const char *src, int src_pitch)
{
    struct qxl_bo *drawable_bo;
    struct QXLDrawable *drawable;
    qxl_screen_t *qxl = dest->qxl;
    struct QXLRect rect;
    struct qxl_bo *image_bo;

    rect.left = x;
    rect.right = x + width;
    rect.top = y;
    rect.bottom = y + height;

    drawable_bo = make_drawable (qxl, dest, QXL_DRAW_COPY, &rect);

    drawable = qxl->bo_funcs->bo_map(drawable_bo);
    drawable->u.copy.src_area.top = 0;
    drawable->u.copy.src_area.bottom = height;
    drawable->u.copy.src_area.left = 0;
    drawable->u.copy.src_area.right = width;

    drawable->u.copy.rop_descriptor = ROPD_OP_PUT;
    drawable->u.copy.scale_mode = 0;
    drawable->u.copy.mask.flags = 0;
    drawable->u.copy.mask.pos.x = 0;
    drawable->u.copy.mask.pos.y = 0;
    drawable->u.copy.mask.bitmap = 0;

    image_bo = qxl_image_create (
	qxl, (const uint8_t *)src, 0, 0, width, height, src_pitch,
	dest->bpp == 24 ? 4 : dest->bpp / 8, FALSE);
    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(QXLDrawable, u.copy.src_bitmap),
				   drawable_bo, image_bo);

    qxl->bo_funcs->bo_unmap(drawable_bo);
    
    push_drawable (qxl, drawable_bo);
    qxl->bo_funcs->bo_decref(qxl, image_bo);    
    return TRUE;
}

void
qxl_get_formats (int bpp, SpiceSurfaceFmt *format, pixman_format_code_t *pformat)
{
    switch (bpp)
    {
    case 8:
	*format = SPICE_SURFACE_FMT_8_A;
	*pformat = PIXMAN_a8;
	break;

    case 16:
	*format = SPICE_SURFACE_FMT_16_565;
	*pformat = PIXMAN_r5g6b5;
	break;

    case 24:
	*format = SPICE_SURFACE_FMT_32_xRGB;
	*pformat = PIXMAN_a8r8g8b8;
	break;
	
    case 32:
	*format = SPICE_SURFACE_FMT_32_ARGB;
	*pformat = PIXMAN_a8r8g8b8;
	break;

    default:
	*format = -1;
	*pformat = -1;
	break;
    }
}
