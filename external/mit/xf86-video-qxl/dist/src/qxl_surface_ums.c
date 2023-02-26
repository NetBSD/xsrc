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

/* The life cycle of surfaces
 *
 *    free => live => dead => destroyed => free
 *
 * A 'free' surface is one that is not allocated on the device. These
 * are stored on the 'free_surfaces' list.
 *
 * A 'live' surface is one that the X server is using for something. It
 * has an associated pixmap. It is allocated in the device. These are stored on
 * the "live_surfaces" list.
 *
 * A 'dead' surface is one that the X server is no using any more, but
 * is still allocated in the device. These surfaces may be stored in the
 * cache, from where they can be resurrected. The cache holds a ref on the
 * surfaces. 
 *
 * A 'destroyed' surface is one whose ref count has reached 0. It is no
 * longer being referenced by either the server or the device or the cache.
 * When a surface enters this state, the associated pixman images are freed, and
 * a destroy command is sent. This will eventually trigger a 'recycle' call,
 * which puts the surface into the 'free' state.
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qxl.h"
#include "qxl_surface.h"
#ifdef DEBUG_SURFACE_LIFECYCLE
#include <stdio.h>

static FILE* surface_log;
#endif

typedef struct evacuated_surface_t evacuated_surface_t;

struct evacuated_surface_t
{
    pixman_image_t	*image;
    PixmapPtr		 pixmap;
    int			 bpp;

    evacuated_surface_t *prev;
    evacuated_surface_t *next;
};

#define N_CACHED_SURFACES 64

/*
 * Surface cache
 */
struct surface_cache_t
{
    qxl_screen_t *qxl;
    
    /* Array of surfaces (not a linked list).
     * All surfaces, excluding the primary one, indexed by surface id.
     */
    qxl_surface_t *all_surfaces;

    /* All surfaces that the driver is currently using (linked through next/prev) */
    qxl_surface_t *live_surfaces;

    /* All surfaces that need to be allocated (linked through next, but not prev) */
    qxl_surface_t *free_surfaces;

    /* Surfaces that are already allocated, but not in used by the driver,
     * linked through next
     */
    qxl_surface_t *cached_surfaces[N_CACHED_SURFACES];
};

#ifdef DEBUG_SURFACE_LIFECYCLE
static void debug_surface_open(void)
{
    if (surface_log)
        return;
    surface_log = fopen("/tmp/xf86-video-qxl.surface.log", "w+");
    if (!surface_log)
    {
        fprintf(stderr, "error creating surface log file (DEBUG_SURFACE_LIFECYCLE)\n");
        exit(-1);
    }
}

static int surface_count(qxl_surface_t *surface)
{
    int i;

    for (i = 0; surface ;++i, surface = surface->next);
    return i;
}

static void debug_surface_log(surface_cache_t *cache)
{
    int  live_n, free_n;

    debug_surface_open();
    live_n = surface_count(cache->live_surfaces);
    free_n = surface_count(cache->free_surfaces);
    fprintf(surface_log, "live,free,sum = %d, %d, %d\n", live_n, free_n,
            live_n + free_n);
    fflush(surface_log);
}

#else
#define debug_surface_log(cache)
#endif


static Bool
surface_cache_init (surface_cache_t *cache, qxl_screen_t *qxl)
{
    int n_surfaces = qxl->rom->n_surfaces;
    int i;

    if (!cache->all_surfaces)
    {
        /* all_surfaces is not freed when evacuating, since surfaces are still
         * tied to pixmaps that may be destroyed after evacuation before
         * recreation */
        cache->all_surfaces = calloc (n_surfaces, sizeof (qxl_surface_t));
        if (!cache->all_surfaces)
            return FALSE;
    }

    memset (cache->all_surfaces, 0, n_surfaces * sizeof (qxl_surface_t));
    memset (cache->cached_surfaces, 0, N_CACHED_SURFACES * sizeof (qxl_surface_t *));
    
    cache->free_surfaces = NULL;
    cache->live_surfaces = NULL;
    
    for (i = 0; i < n_surfaces; ++i)
    {
	cache->all_surfaces[i].id = i;
	cache->all_surfaces[i].cache = cache;
	cache->all_surfaces[i].qxl = qxl;
	cache->all_surfaces[i].dev_image = NULL;
	cache->all_surfaces[i].host_image = NULL;
	cache->all_surfaces[i].evacuated = NULL;
	
	REGION_INIT (
	    NULL, &(cache->all_surfaces[i].access_region), (BoxPtr)NULL, 0);
	cache->all_surfaces[i].access_type = UXA_ACCESS_RO;

	if (i) /* surface 0 is the primary surface */
	{
	    cache->all_surfaces[i].next = cache->free_surfaces;
	    cache->free_surfaces = &(cache->all_surfaces[i]);
	    cache->all_surfaces[i].in_use = FALSE;
	}
    }

    return TRUE;
}

surface_cache_t *
qxl_surface_cache_create (qxl_screen_t *qxl)
{
    surface_cache_t *cache = malloc (sizeof *cache);

    if (!cache)
	return NULL;

    memset(cache, 0, sizeof(*cache));
    cache->qxl = qxl;
    if (!surface_cache_init (cache, qxl))
    {
	free (cache);
	return NULL;
    }

    return cache;
}

void
qxl_surface_cache_sanity_check (surface_cache_t *qxl)
{
#if 0
    qxl_surface_t *s;

    for (s = qxl->live_surfaces; s != NULL; s = s->next)
    {
	PixmapPtr pixmap = s->pixmap;

	if (! (get_surface (pixmap) == s) )
	{
	    ErrorF ("Surface %p has pixmap %p, but pixmap %p has surface %p\n",
		    s, pixmap, pixmap, get_surface (pixmap));

	    assert (0);
	}
    }
#endif
}

static void
print_cache_info (surface_cache_t *cache)
{
    int i;
    int n_surfaces = 0;

    ErrorF ("Cache contents:  ");
    for (i = 0; i < N_CACHED_SURFACES; ++i)
    {
	if (cache->cached_surfaces[i])
	{
	    ErrorF ("%4d ", cache->cached_surfaces[i]->id);
	    n_surfaces++;
	}
	else
	    ErrorF ("null ");
    }

    ErrorF ("    total: %d\n", n_surfaces);
}

static qxl_surface_t *
surface_get_from_cache (surface_cache_t *cache, int width, int height, int bpp)
{
    int i;

    for (i = 0; i < N_CACHED_SURFACES; ++i)
    {
	qxl_surface_t *s = cache->cached_surfaces[i];

	if (s && bpp == s->bpp)
	{
	    int w = pixman_image_get_width (s->host_image);
	    int h = pixman_image_get_height (s->host_image);
	    
	    if (width <= w && width * 4 > w && height <= h && height * 4 > h)
	    {
		cache->cached_surfaces[i] = NULL;

		return s;
	    }
	}
    }

    return NULL;
}

static int n_live;

void
qxl_surface_recycle (surface_cache_t *cache, uint32_t id)
{
    qxl_surface_t *surface = cache->all_surfaces + id;

    n_live--;
    if (surface->bo)
	cache->qxl->bo_funcs->bo_decref (cache->qxl, surface->bo);
    surface->bo = NULL;
    surface->next = cache->free_surfaces;
    cache->free_surfaces = surface;
}

/*
 * mode is used for the whole virtual screen, not for a specific head.
 * For a single head where virtual size is equal to the head size, they are
 * equal. For multiple heads this mode will not match any existing heads and
 * will be the containing virtual size.
 */
qxl_surface_t *
qxl_surface_cache_create_primary (qxl_screen_t *qxl,
				  struct QXLMode	*mode)
{
    pixman_format_code_t format;
    uint8_t *dev_addr;
    pixman_image_t *dev_image, *host_image;
    qxl_surface_t *surface;
    surface_cache_t	*cache = qxl->surface_cache;
    struct qxl_bo *bo;

    if (mode->bits == 16)
    {
	format = PIXMAN_x1r5g5b5;
    }
    else if (mode->bits == 32)
    {
	format = PIXMAN_x8r8g8b8;
    }
    else
    {
	xf86DrvMsg (qxl->pScrn->scrnIndex, X_ERROR,
		    "Unknown bit depth %d\n", mode->bits);
	return NULL;
    }

    bo = qxl->bo_funcs->create_primary(qxl, mode->x_res, mode->y_res, mode->stride, mode->bits);

    dev_addr = qxl->bo_funcs->bo_map(bo);
    dev_image = pixman_image_create_bits (format, mode->x_res, mode->y_res,
					  (uint32_t *)dev_addr, (qxl->kms_enabled ? mode->stride : -mode->stride));

    host_image = pixman_image_create_bits (format, 
					   qxl->virtual_x, qxl->virtual_y,
					   NULL, mode->stride);
#if 0
    xf86DrvMsg(cache->qxl->pScrn->scrnIndex, X_ERROR,
               "testing dev_image memory (%d x %d)\n",
               mode->x_res, mode->y_res);
    memset(qxl->ram, 0, mode->stride * mode->y_res);
    xf86DrvMsg(cache->qxl->pScrn->scrnIndex, X_ERROR,
               "testing host_image memory\n");
    memset(qxl->fb, 0, mode->stride * mode->y_res);
#endif

    surface = malloc (sizeof *surface);
    surface->id = 0;
    surface->dev_image = dev_image;
    surface->host_image = host_image;
    surface->cache = cache;
    surface->qxl = qxl;
    surface->bpp = mode->bits;
    surface->next = NULL;
    surface->prev = NULL;
    surface->evacuated = NULL;
    surface->bo = bo;
    surface->image_bo = NULL;
    
    REGION_INIT (NULL, &(surface->access_region), (BoxPtr)NULL, 0);
    surface->access_type = UXA_ACCESS_RO;
    
    return surface;
}

void *
qxl_surface_get_host_bits(qxl_surface_t *surface)
{
    if (!surface)
	return NULL;
    return (void *) pixman_image_get_data(surface->host_image);
}




static struct qxl_bo *
make_surface_cmd (surface_cache_t *cache, uint32_t id, QXLSurfaceCmdType type)
{
    struct qxl_bo *cmd_bo;
    struct QXLSurfaceCmd *cmd;
    qxl_screen_t *qxl = cache->qxl;

    cmd_bo = qxl->bo_funcs->cmd_alloc (qxl, sizeof *cmd, "surface command");
    cmd = qxl->bo_funcs->bo_map(cmd_bo);

    cmd->release_info.id = pointer_to_u64 (cmd_bo) | 2;
    cmd->type = type;
    cmd->flags = 0;
    cmd->surface_id = id;
    
    qxl->bo_funcs->bo_unmap(cmd_bo);
    return cmd_bo;
}

static void
push_surface_cmd (surface_cache_t *cache, struct qxl_bo *cmd_bo)
{
    qxl_screen_t *qxl = cache->qxl;
    
    qxl->bo_funcs->write_command (qxl, QXL_CMD_SURFACE, cmd_bo);
}


static qxl_surface_t *
surface_get_from_free_list (surface_cache_t *cache)
{
    qxl_surface_t *result = NULL;

    if (cache->free_surfaces)
    {
	qxl_surface_t *s;

	result = cache->free_surfaces;
	cache->free_surfaces = cache->free_surfaces->next;

	result->next = NULL;
	result->in_use = TRUE;
	result->ref_count = 1;
	result->pixmap = NULL;

	for (s = cache->free_surfaces; s; s = s->next)
	{
	    if (s->id == result->id)
		ErrorF ("huh: %d to be returned, but %d is in list\n",
			s->id, result->id);

	    assert (s->id != result->id);
	}
    }
    
    return result;
}

static int
align (int x)
{
    return x;
}

static qxl_surface_t *
surface_send_create (surface_cache_t *cache,
		     int	      width,
		     int	      height,
		     int	      bpp)
{
    SpiceSurfaceFmt format;
    pixman_format_code_t pformat;
    struct QXLSurfaceCmd *cmd;
    int stride;
    uint32_t *dev_addr;
    int n_attempts = 0;
    qxl_screen_t *qxl = cache->qxl;
    qxl_surface_t *surface;
    struct qxl_bo *bo, *cmd_bo;
    void *dev_ptr;
    qxl_get_formats (bpp, &format, &pformat);
    
    width = align (width);
    height = align (height);
    
    stride = width * PIXMAN_FORMAT_BPP (pformat) / 8;
    stride = (stride + 3) & ~3;

    /* the final + stride is to work around a bug where the device apparently 
     * scribbles after the end of the image
     */
    qxl_garbage_collect (qxl);
retry2:
    bo = qxl_ums_surf_mem_alloc(qxl, stride * height + stride);

    if (!bo)
    {
	ErrorF ("- %dth attempt\n", n_attempts++);

	if (qxl_garbage_collect (qxl))
	    goto retry2;

	ErrorF ("- OOM at %d %d %d (= %d bytes)\n", width, height, bpp, width * height * (bpp / 8));
	print_cache_info (cache);
	
	if (qxl_handle_oom (qxl))
	{
	    while (qxl_garbage_collect (qxl))
		;
	    goto retry2;
	}

	ErrorF ("Out of video memory: Could not allocate %d bytes\n",
		stride * height + stride);
	
	return NULL;
    }

retry:
    surface = surface_get_from_free_list (cache);
    if (!surface)
    {
	if (!qxl_handle_oom (cache->qxl))
	{
	    ErrorF ("  Out of surfaces\n");
	    qxl->bo_funcs->bo_decref (qxl, bo);
	    return NULL;
	}
	else
	    goto retry;
    }

    surface->bo = bo;
    
    cmd_bo = make_surface_cmd (cache, surface->id, QXL_SURFACE_CMD_CREATE);

    cmd = qxl->bo_funcs->bo_map(cmd_bo);
    cmd->u.surface_create.format = format;
    cmd->u.surface_create.width = width;
    cmd->u.surface_create.height = height;
    cmd->u.surface_create.stride = - stride;
    qxl->bo_funcs->bo_unmap(cmd_bo);

    qxl->bo_funcs->bo_output_bo_reloc(qxl, offsetof(struct QXLSurfaceCmd, u.surface_create.data), cmd_bo, surface->bo);

    push_surface_cmd (cache, cmd_bo);

    dev_ptr = qxl->bo_funcs->bo_map(surface->bo);
    dev_addr
	= (uint32_t *)((uint8_t *)dev_ptr + stride * (height - 1));

    surface->dev_image = pixman_image_create_bits (
	pformat, width, height, dev_addr, - stride);

    surface->host_image = pixman_image_create_bits (
	pformat, width, height, NULL, -1);

    qxl->bo_funcs->bo_unmap(surface->bo);
    surface->bpp = bpp;

    n_live++;
    
    return surface;
}

qxl_surface_t *
qxl_surface_create (qxl_screen_t *qxl,
		    int			 width,
		    int			 height,
		    int			 bpp)
{
    qxl_surface_t *surface;
    surface_cache_t *cache = qxl->surface_cache;

    if (!qxl->enable_surfaces)
	return NULL;
    
    if ((bpp & 3) != 0)
    {
	ErrorF ("%s: Bad bpp: %d (%d)\n", __FUNCTION__, bpp, bpp & 7);
	return NULL;
    }

#if 0
    if (bpp == 8)
      {
	static int warned;
	if (!warned)
	{
	    warned = 1;
	    ErrorF ("bpp == 8 triggers bugs in spice apparently\n");
	}
	
	return NULL;
      }
#endif
    
    if (bpp != 8 && bpp != 16 && bpp != 32 && bpp != 24)
    {
	ErrorF ("%s: Unknown bpp\n", __FUNCTION__);
	return NULL;
    }

    if (width == 0 || height == 0)
    {
	ErrorF ("%s: Zero width or height\n", __FUNCTION__);
	return NULL;
    }

    if (!(surface = surface_get_from_cache (cache, width, height, bpp)))
	if (!(surface = surface_send_create (cache, width, height, bpp)))
	    return NULL;

    surface->next = cache->live_surfaces;
    surface->prev = NULL;
    if (cache->live_surfaces)
	cache->live_surfaces->prev = surface;
    cache->live_surfaces = surface;
    
    return surface;
}

void
qxl_surface_set_pixmap (qxl_surface_t *surface, PixmapPtr pixmap)
{
    surface->pixmap = pixmap;

    assert (get_surface (pixmap) == surface);
}

static void
unlink_surface (qxl_surface_t *surface)
{
    if (surface->id != 0)
    {
        if (surface->prev)
            surface->prev->next = surface->next;
        else
            surface->cache->live_surfaces = surface->next;
    }

    debug_surface_log(surface->cache);
    
    if (surface->next)
	surface->next->prev = surface->prev;

    surface->pixmap = NULL;
    
    surface->prev = NULL;
    surface->next = NULL;
}

static void
surface_destroy (qxl_surface_t *surface)
{
    struct qxl_bo *cmd_bo;

    if (surface->dev_image)
	pixman_image_unref (surface->dev_image);
    if (surface->host_image)
	pixman_image_unref (surface->host_image);

#if 0
    ErrorF("destroy %ld\n", (long int)surface->end - (long int)surface->address);
#endif
    cmd_bo = make_surface_cmd (surface->cache, surface->id, QXL_SURFACE_CMD_DESTROY);

    push_surface_cmd (surface->cache, cmd_bo);

    surface->cache->qxl->bo_funcs->bo_decref(surface->cache->qxl, surface->bo);
}

static void
surface_add_to_cache (qxl_surface_t *surface)
{
    surface_cache_t *cache = surface->cache;
    int oldest = -1;
    int n_surfaces = 0;
    int i, delta;
    int destroy_id = -1;
    qxl_surface_t *destroy_surface = NULL;

    surface->ref_count++;
    
    for (i = 0; i < N_CACHED_SURFACES; ++i)
    {
	if (cache->cached_surfaces[i])
	{
	    oldest = i;
	    n_surfaces++;
	}
    }
    
    if (n_surfaces == N_CACHED_SURFACES)
    {
	destroy_id = cache->cached_surfaces[oldest]->id;
	
	destroy_surface = cache->cached_surfaces[oldest];
	
	cache->cached_surfaces[oldest] = NULL;
	
	for (i = 0; i < N_CACHED_SURFACES; ++i)
	    assert (!cache->cached_surfaces[i] ||
		    cache->cached_surfaces[i]->id != destroy_id);
    }
    
    delta = 0;
    for (i = N_CACHED_SURFACES - 1; i >= 0; i--)
    {
	if (cache->cached_surfaces[i])
	{
	    if (delta > 0)
	    {
		cache->cached_surfaces[i + delta] =
		    cache->cached_surfaces[i];
		
		assert (cache->cached_surfaces[i + delta]->id != destroy_id);
		
		cache->cached_surfaces[i] = NULL;
	    }
	}
	else
	{
	    delta++;
	}
    }
    
    assert (delta > 0);
    
    cache->cached_surfaces[i + delta] = surface;
    
    for (i = 0; i < N_CACHED_SURFACES; ++i)
	assert (!cache->cached_surfaces[i] || cache->cached_surfaces[i]->id != destroy_id);

    /* Note that sending a destroy command can trigger callbacks into
     * this function (due to memory management), so we have to
     * do this after updating the cache
     */
    if (destroy_surface)
	qxl_surface_unref (destroy_surface->cache, destroy_surface->id);
}

void
qxl_surface_unref (surface_cache_t *cache, uint32_t id)
{
    if (id != 0)
    {
	qxl_surface_t *surface = cache->all_surfaces + id;

	if (--surface->ref_count == 0)
	    surface_destroy (surface);
    }
}

void
qxl_surface_kill (qxl_surface_t *surface)
{
    struct evacuated_surface_t *ev = surface->evacuated;

    if (ev)
    {
        /* server side surface is already destroyed (via reset), don't
         * resend a destroy. Just mark surface as not to be recreated */
        ev->pixmap = NULL;
        if (ev->image)
            pixman_image_unref (ev->image);
        if (ev->next)
            ev->next->prev = ev->prev;
        if (ev->prev)
            ev->prev->next = ev->next;
        free(ev);
        surface->evacuated = NULL;
        return;
    }

    unlink_surface (surface);

    if (!surface->cache->all_surfaces) {
        return;
    }

    if (surface->id != 0					&&
        surface->host_image                                     &&
	pixman_image_get_width (surface->host_image) >= 128	&&
	pixman_image_get_height (surface->host_image) >= 128)
    {
	surface_add_to_cache (surface);
    }
    
    qxl_surface_unref (surface->cache, surface->id);
}


void *
qxl_surface_cache_evacuate_all (surface_cache_t *cache)
{
    evacuated_surface_t *evacuated_surfaces = NULL;
    qxl_surface_t *s;
    int i;

    for (i = 0; i < N_CACHED_SURFACES; ++i)
    {
	if (cache->cached_surfaces[i])
	{
            surface_destroy (cache->cached_surfaces[i]);
	    cache->cached_surfaces[i] = NULL;
	}
    }

    s = cache->live_surfaces;
    while (s != NULL)
    {
	qxl_surface_t *next = s->next;
	evacuated_surface_t *evacuated = malloc (sizeof (evacuated_surface_t));
	int width, height;

	width = pixman_image_get_width (s->host_image);
	height = pixman_image_get_height (s->host_image);

	qxl_download_box (s, 0, 0, width, height);

	evacuated->image = s->host_image;
	evacuated->pixmap = s->pixmap;

	assert (get_surface (evacuated->pixmap) == s);
	
	evacuated->bpp = s->bpp;
	
	s->host_image = NULL;

	unlink_surface (s);
	
	evacuated->prev = NULL;
	evacuated->next = evacuated_surfaces;
        if (evacuated_surfaces)
            evacuated_surfaces->prev = evacuated;
	evacuated_surfaces = evacuated;
        s->evacuated = evacuated;

	s = next;
    }

    cache->live_surfaces = NULL;
    cache->free_surfaces = NULL;

    return evacuated_surfaces;
}

void
qxl_surface_cache_replace_all (surface_cache_t *cache, void *data)
{
    evacuated_surface_t *ev;

    if (!surface_cache_init (cache, cache->qxl))
    {
	/* FIXME: report the error */
	return;
    }
    
    ev = data;
    while (ev != NULL)
    {
	evacuated_surface_t *next = ev->next;
	int width = pixman_image_get_width (ev->image);
	int height = pixman_image_get_height (ev->image);
	qxl_surface_t *surface;

	surface = qxl_surface_create (cache->qxl, width, height, ev->bpp);

	assert (surface->host_image);
	assert (surface->dev_image);

	pixman_image_unref (surface->host_image);
	surface->host_image = ev->image;

	qxl_upload_box (surface, 0, 0, width, height);

	set_surface (ev->pixmap, surface);

	qxl_surface_set_pixmap (surface, ev->pixmap);

	free (ev);
	
	ev = next;
    }

    qxl_surface_cache_sanity_check (cache);

}
