/*
 * Copyright © 2007 Red Hat, Inc.
 * Copyright © 2008 Maarten Maathuis
 *
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
 *    Dave Airlie <airlied@redhat.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xorgVersion.h"

#include "nv_include.h"
#include "xf86drmMode.h"
#include "X11/Xatom.h"

#include <sys/ioctl.h>
#ifdef HAVE_LIBUDEV
#include "libudev.h"
#endif

static Bool drmmode_xf86crtc_resize(ScrnInfoPtr scrn, int width, int height);
typedef struct {
    int fd;
    uint32_t fb_id;
    int cpp;
    drmEventContext event_context;
#ifdef HAVE_LIBUDEV
    struct udev_monitor *uevent_monitor;
#endif
} drmmode_rec, *drmmode_ptr;

typedef struct {
    drmmode_ptr drmmode;
    drmModeCrtcPtr mode_crtc;
    int hw_crtc_index;
    struct nouveau_bo *cursor;
    struct nouveau_bo *rotate_bo;
    int rotate_pitch;
    PixmapPtr rotate_pixmap;
    uint32_t rotate_fb_id;
    Bool cursor_visible;
    int scanout_pixmap_x;
    int dpms_mode;
} drmmode_crtc_private_rec, *drmmode_crtc_private_ptr;

typedef struct {
	drmModePropertyPtr mode_prop;
	int index; /* Index within the kernel-side property arrays for
		    * this connector. */
	int num_atoms; /* if range prop, num_atoms == 1; if enum prop,
			* num_atoms == num_enums + 1 */
	Atom *atoms;
} drmmode_prop_rec, *drmmode_prop_ptr;

typedef struct {
    drmmode_ptr drmmode;
    int output_id;
    drmModeConnectorPtr mode_output;
    drmModeEncoderPtr mode_encoder;
    drmModePropertyBlobPtr edid_blob;
    drmModePropertyBlobPtr tile_blob;
    int num_props;
    drmmode_prop_ptr props;
} drmmode_output_private_rec, *drmmode_output_private_ptr;

static void drmmode_output_dpms(xf86OutputPtr output, int mode);

static drmmode_ptr
drmmode_from_scrn(ScrnInfoPtr scrn)
{
	if (scrn) {
		xf86CrtcConfigPtr conf = XF86_CRTC_CONFIG_PTR(scrn);
		drmmode_crtc_private_ptr crtc = conf->crtc[0]->driver_private;

		return crtc->drmmode;
	}

	return NULL;
}

static inline struct nouveau_pixmap *
drmmode_pixmap(PixmapPtr ppix)
{
	return nouveau_pixmap(ppix);
}

int
drmmode_crtc(xf86CrtcPtr crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	return drmmode_crtc->mode_crtc->crtc_id;
}

Bool
xf86_crtc_on(xf86CrtcPtr crtc)
{
    drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;

    return crtc->enabled && drmmode_crtc->dpms_mode == DPMSModeOn;
}

int
drmmode_head(xf86CrtcPtr crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	return drmmode_crtc->hw_crtc_index;
}

void
drmmode_swap(ScrnInfoPtr scrn, uint32_t next, uint32_t *prev)
{
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	*prev = drmmode->fb_id;
	drmmode->fb_id = next;
}

struct drmmode_event {
	struct xorg_list head;
	drmmode_ptr drmmode;
	uint64_t name;
	void (*func)(void *, uint64_t, uint64_t, uint32_t);
};

static struct xorg_list
drmmode_events = {
	.next = &drmmode_events,
	.prev = &drmmode_events,
};

static bool warned = false;

static void
drmmode_event_handler(int fd, unsigned int frame, unsigned int tv_sec,
		      unsigned int tv_usec, void *event_data)
{
	const uint64_t ust = (uint64_t)tv_sec * 1000000 + tv_usec;
	struct drmmode_event *e = event_data;

	int counter = 0;

	xorg_list_for_each_entry(e, &drmmode_events, head) {
		counter++;
		if (e == event_data) {
			xorg_list_del(&e->head);
			e->func((void *)(e + 1), e->name, ust, frame);
			free(e);
			break;
		}
	}

	if (counter > 100 && !warned) {
		xf86DrvMsg(0, X_WARNING,
			   "Event handler iterated %d times\n", counter);
		warned = true;
	}
}

void
drmmode_event_abort(ScrnInfoPtr scrn, uint64_t name, bool pending)
{
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	struct drmmode_event *e, *t;

	xorg_list_for_each_entry_safe(e, t, &drmmode_events, head) {
		if (e->drmmode == drmmode && e->name == name) {
			xorg_list_del(&e->head);
			if (!pending)
				free(e);
			break;
		}
	}
}

void *
drmmode_event_queue(ScrnInfoPtr scrn, uint64_t name, unsigned size,
		    void (*func)(void *, uint64_t, uint64_t, uint32_t),
		    void **event_data)
{
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	struct drmmode_event *e;

	e = *event_data = calloc(1, sizeof(*e) + size);
	if (e) {
		e->drmmode = drmmode;
		e->name = name;
		e->func = func;
		xorg_list_append(&e->head, &drmmode_events);
		return (void *)(e + 1);
	}

	return NULL;
}

int
drmmode_event_flush(ScrnInfoPtr scrn)
{
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	return drmHandleEvent(drmmode->fd, &drmmode->event_context);
}

void
drmmode_event_fini(ScrnInfoPtr scrn)
{
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	struct drmmode_event *e, *t;

	xorg_list_for_each_entry_safe(e, t, &drmmode_events, head) {
		if (e->drmmode == drmmode) {
			xorg_list_del(&e->head);
			free(e);
		}
	}
}

void
drmmode_event_init(ScrnInfoPtr scrn)
{
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	drmmode->event_context.version = DRM_EVENT_CONTEXT_VERSION;
	drmmode->event_context.vblank_handler = drmmode_event_handler;
	drmmode->event_context.page_flip_handler = drmmode_event_handler;
}

static PixmapPtr
drmmode_pixmap_wrap(ScreenPtr pScreen, int width, int height, int depth,
		    int bpp, int pitch, struct nouveau_bo *bo, void *data)
{
	NVPtr pNv = NVPTR(xf86ScreenToScrn(pScreen));
	PixmapPtr ppix;

	if (pNv->AccelMethod > NONE)
		data = NULL;

	ppix = pScreen->CreatePixmap(pScreen, 0, 0, depth, 0);
	if (!ppix)
		return NULL;

	pScreen->ModifyPixmapHeader(ppix, width, height, depth, bpp,
				    pitch, data);
	if (pNv->AccelMethod > NONE)
		nouveau_bo_ref(bo, &drmmode_pixmap(ppix)->bo);

	return ppix;
}

static void
drmmode_ConvertFromKMode(ScrnInfoPtr scrn, drmModeModeInfo *kmode,
			 DisplayModePtr	mode)
{
	memset(mode, 0, sizeof(DisplayModeRec));
	mode->status = MODE_OK;

	mode->Clock = kmode->clock;

	mode->HDisplay = kmode->hdisplay;
	mode->HSyncStart = kmode->hsync_start;
	mode->HSyncEnd = kmode->hsync_end;
	mode->HTotal = kmode->htotal;
	mode->HSkew = kmode->hskew;

	mode->VDisplay = kmode->vdisplay;
	mode->VSyncStart = kmode->vsync_start;
	mode->VSyncEnd = kmode->vsync_end;
	mode->VTotal = kmode->vtotal;
	mode->VScan = kmode->vscan;

	mode->Flags = kmode->flags; //& FLAG_BITS;
	mode->name = strdup(kmode->name);

	if (kmode->type & DRM_MODE_TYPE_DRIVER)
		mode->type = M_T_DRIVER;
	if (kmode->type & DRM_MODE_TYPE_PREFERRED)
		mode->type |= M_T_PREFERRED;
	xf86SetModeCrtc (mode, scrn->adjustFlags);
}

static void
drmmode_ConvertToKMode(ScrnInfoPtr scrn, drmModeModeInfo *kmode,
		       DisplayModePtr mode)
{
	memset(kmode, 0, sizeof(*kmode));

	kmode->clock = mode->Clock;
	kmode->hdisplay = mode->HDisplay;
	kmode->hsync_start = mode->HSyncStart;
	kmode->hsync_end = mode->HSyncEnd;
	kmode->htotal = mode->HTotal;
	kmode->hskew = mode->HSkew;

	kmode->vdisplay = mode->VDisplay;
	kmode->vsync_start = mode->VSyncStart;
	kmode->vsync_end = mode->VSyncEnd;
	kmode->vtotal = mode->VTotal;
	kmode->vscan = mode->VScan;

	kmode->flags = mode->Flags; //& FLAG_BITS;
	if (mode->name)
		strncpy(kmode->name, mode->name, DRM_DISPLAY_MODE_LEN);
	kmode->name[DRM_DISPLAY_MODE_LEN-1] = 0;

}

static void
drmmode_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_crtc->dpms_mode = mode;
}

void
drmmode_fbcon_copy(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
	NVPtr pNv = NVPTR(pScrn);
	if (nouveau_bo_map(pNv->scanout, NOUVEAU_BO_WR, pNv->client))
		return;
	memset(pNv->scanout->map, 0x00, pNv->scanout->size);
}

static Bool
drmmode_set_mode_major(xf86CrtcPtr crtc, DisplayModePtr mode,
		       Rotation rotation, int x, int y)
{
	ScrnInfoPtr pScrn = crtc->scrn;
	NVPtr pNv = NVPTR(pScrn);
	xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	uint32_t *output_ids;
	int output_count = 0;
	int ret = TRUE;
	int i;
	int fb_id;
	drmModeModeInfo kmode;

	if (drmmode->fb_id == 0) {
		unsigned int pitch =
			pScrn->displayWidth * (pScrn->bitsPerPixel / 8);

		ret = drmModeAddFB(drmmode->fd,
				   pScrn->virtualX, pScrn->virtualY,
				   pScrn->depth, pScrn->bitsPerPixel,
				   pitch, pNv->scanout->handle,
				   &drmmode->fb_id);
		if (ret < 0) {
			ErrorF("failed to add fb\n");
			return FALSE;
		}
	}

	if (!xf86CrtcRotate(crtc))
		return FALSE;

	output_ids = calloc(sizeof(uint32_t), xf86_config->num_output);
	if (!output_ids)
		return FALSE;

	for (i = 0; i < xf86_config->num_output; i++) {
		xf86OutputPtr output = xf86_config->output[i];
		drmmode_output_private_ptr drmmode_output;

		if (output->crtc != crtc)
			continue;

		drmmode_output = output->driver_private;
		if (drmmode_output->output_id == -1)
			continue;
		output_ids[output_count] =
			drmmode_output->mode_output->connector_id;
		output_count++;
	}

	drmmode_ConvertToKMode(crtc->scrn, &kmode, mode);

	fb_id = drmmode->fb_id;
#ifdef NOUVEAU_PIXMAP_SHARING
	if (crtc->randr_crtc && crtc->randr_crtc->scanout_pixmap) {
		x = drmmode_crtc->scanout_pixmap_x;
		y = 0;
	} else
#endif
	if (drmmode_crtc->rotate_fb_id) {
		fb_id = drmmode_crtc->rotate_fb_id;
		x = 0;
		y = 0;
	}

	ret = drmModeSetCrtc(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
			     fb_id, x, y, output_ids, output_count, &kmode);
	free(output_ids);

	if (ret) {
		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "failed to set mode: %s\n", strerror(-ret));
		return FALSE;
	}

	/* Work around some xserver stupidity */
	for (i = 0; i < xf86_config->num_output; i++) {
		xf86OutputPtr output = xf86_config->output[i];

		if (output->crtc != crtc)
			continue;

		drmmode_output_dpms(output, DPMSModeOn);
	}

	crtc->funcs->gamma_set(crtc, crtc->gamma_red, crtc->gamma_green,
			       crtc->gamma_blue, crtc->gamma_size);

#ifdef HAVE_XF86_CURSOR_RESET_CURSOR
	xf86CursorResetCursor(crtc->scrn->pScreen);
#else
	xf86_reload_cursors(crtc->scrn->pScreen);
#endif

	return TRUE;
}

static void
drmmode_set_cursor_position (xf86CrtcPtr crtc, int x, int y)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	drmModeMoveCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id, x, y);
}

static void
convert_cursor(CARD32 *dst, CARD32 *src, int dw, int sw)
{
	int i, j;

	for (j = 0;  j < sw; j++) {
		for (i = 0; i < sw; i++) {
			dst[j * dw + i] = src[j * sw + i];
		}
	}
}

static void
drmmode_load_cursor_argb (xf86CrtcPtr crtc, CARD32 *image)
{
	NVPtr pNv = NVPTR(crtc->scrn);
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	struct nouveau_bo *cursor = drmmode_crtc->cursor;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	nouveau_bo_map(cursor, NOUVEAU_BO_WR, pNv->client);
	convert_cursor(cursor->map, image, 64, nv_cursor_width(pNv));

	if (drmmode_crtc->cursor_visible) {
		drmModeSetCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
				 cursor->handle, 64, 64);
	}
}

static void
drmmode_hide_cursor (xf86CrtcPtr crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	drmModeSetCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
			 0, 64, 64);
	drmmode_crtc->cursor_visible = FALSE;
}

static void
drmmode_show_cursor (xf86CrtcPtr crtc)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	drmModeSetCursor(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
			 drmmode_crtc->cursor->handle, 64, 64);
	drmmode_crtc->cursor_visible = TRUE;
}

static void *
drmmode_crtc_shadow_allocate(xf86CrtcPtr crtc, int width, int height)
{
	ScrnInfoPtr scrn = crtc->scrn;
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	void *virtual;
	int ret;

	ret = nouveau_allocate_surface(scrn, width, height,
				       scrn->bitsPerPixel,
				       NOUVEAU_CREATE_PIXMAP_SCANOUT,
				       &drmmode_crtc->rotate_pitch,
				       &drmmode_crtc->rotate_bo);
	if (!ret) {
		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "Couldn't allocate shadow memory for rotated CRTC\n");
		return NULL;
	}

	ret = nouveau_bo_map(drmmode_crtc->rotate_bo, NOUVEAU_BO_RDWR,
			     NVPTR(scrn)->client);
	if (ret) {
		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "Couldn't get virtual address of shadow scanout\n");
		nouveau_bo_ref(NULL, &drmmode_crtc->rotate_bo);
		return NULL;
	}
	virtual = drmmode_crtc->rotate_bo->map;

	ret = drmModeAddFB(drmmode->fd, width, height, crtc->scrn->depth,
			   crtc->scrn->bitsPerPixel, drmmode_crtc->rotate_pitch,
			   drmmode_crtc->rotate_bo->handle,
			   &drmmode_crtc->rotate_fb_id);
	if (ret) {
		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "Error adding FB for shadow scanout: %s\n",
			   strerror(-ret));
		nouveau_bo_ref(NULL, &drmmode_crtc->rotate_bo);
		return NULL;
	}

	return virtual;
}

static PixmapPtr
drmmode_crtc_shadow_create(xf86CrtcPtr crtc, void *data, int width, int height)
{
	ScrnInfoPtr pScrn = crtc->scrn;
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	PixmapPtr rotate_pixmap;

	if (!data)
		data = drmmode_crtc_shadow_allocate (crtc, width, height);

	rotate_pixmap = drmmode_pixmap_wrap(pScrn->pScreen, width, height,
					    pScrn->depth, pScrn->bitsPerPixel,
					    drmmode_crtc->rotate_pitch,
					    drmmode_crtc->rotate_bo, data);

	drmmode_crtc->rotate_pixmap = rotate_pixmap;
	return drmmode_crtc->rotate_pixmap;
}

static void
drmmode_crtc_shadow_destroy(xf86CrtcPtr crtc, PixmapPtr rotate_pixmap, void *data)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;

	if (rotate_pixmap)
		FreeScratchPixmapHeader(rotate_pixmap);

	if (data) {
		drmModeRmFB(drmmode->fd, drmmode_crtc->rotate_fb_id);
		drmmode_crtc->rotate_fb_id = 0;
		nouveau_bo_ref(NULL, &drmmode_crtc->rotate_bo);
		drmmode_crtc->rotate_pixmap = NULL;
	}
}

static void
drmmode_gamma_set(xf86CrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue,
		  int size)
{
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	int ret;

	ret = drmModeCrtcSetGamma(drmmode->fd, drmmode_crtc->mode_crtc->crtc_id,
				  size, red, green, blue);
	if (ret != 0) {
		xf86DrvMsg(crtc->scrn->scrnIndex, X_ERROR,
			   "failed to set gamma with %d entries: %s\n",
			   size, strerror(-ret));
	}
}

#ifdef NOUVEAU_PIXMAP_SHARING
static Bool
drmmode_set_scanout_pixmap(xf86CrtcPtr crtc, PixmapPtr ppix)
{
	ScreenPtr screen = xf86ScrnToScreen(crtc->scrn);
	PixmapPtr screenpix = screen->GetScreenPixmap(screen);
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(crtc->scrn);
	drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
	drmmode_ptr drmmode = drmmode_crtc->drmmode;
	int c, total_width = 0, max_height = 0, this_x = 0;
	if (!ppix) {
		if (crtc->randr_crtc->scanout_pixmap) {
#ifdef HAS_DIRTYTRACKING_DRAWABLE_SRC
			PixmapStopDirtyTracking(&crtc->randr_crtc->scanout_pixmap->drawable, screenpix);
#else
			PixmapStopDirtyTracking(crtc->randr_crtc->scanout_pixmap, screenpix);
#endif
			if (drmmode && drmmode->fb_id) {
				drmModeRmFB(drmmode->fd, drmmode->fb_id);
				drmmode->fb_id = 0;
			}
		}
		drmmode_crtc->scanout_pixmap_x = 0;
		return TRUE;
	}

	/* iterate over all the attached crtcs -
	   work out bounding box */
	for (c = 0; c < xf86_config->num_crtc; c++) {
		xf86CrtcPtr iter = xf86_config->crtc[c];
		if (!iter->enabled && iter != crtc)
			continue;
		if (iter == crtc) {
			this_x = total_width;
			total_width += ppix->drawable.width;
			if (max_height < ppix->drawable.height)
				max_height = ppix->drawable.height;
		} else {
			total_width += iter->mode.HDisplay;
			if (max_height < iter->mode.VDisplay)
				max_height = iter->mode.VDisplay;
		}
	}

	if (total_width != screenpix->drawable.width ||
	    max_height != screenpix->drawable.height) {
		Bool ret;
		ret = drmmode_xf86crtc_resize(crtc->scrn, total_width, max_height);
		if (ret == FALSE)
			return FALSE;

		screenpix = screen->GetScreenPixmap(screen);
		screen->width = screenpix->drawable.width = total_width;
		screen->height = screenpix->drawable.height = max_height;
	}
	drmmode_crtc->scanout_pixmap_x = this_x;

#ifdef HAS_DIRTYTRACKING_DRAWABLE_SRC
	PixmapStartDirtyTracking(&ppix->drawable, screenpix, 0, 0, this_x, 0, RR_Rotate_0);
#else
	PixmapStartDirtyTracking(ppix, screenpix, 0, 0, this_x, 0, RR_Rotate_0);
#endif
	return TRUE;
}
#endif

static const xf86CrtcFuncsRec drmmode_crtc_funcs = {
	.dpms = drmmode_crtc_dpms,
	.set_mode_major = drmmode_set_mode_major,
	.set_cursor_position = drmmode_set_cursor_position,
	.show_cursor = drmmode_show_cursor,
	.hide_cursor = drmmode_hide_cursor,
	.load_cursor_argb = drmmode_load_cursor_argb,
	.shadow_create = drmmode_crtc_shadow_create,
	.shadow_allocate = drmmode_crtc_shadow_allocate,
	.shadow_destroy = drmmode_crtc_shadow_destroy,
	.gamma_set = drmmode_gamma_set,

#ifdef NOUVEAU_PIXMAP_SHARING
	.set_scanout_pixmap = drmmode_set_scanout_pixmap,
#endif
};


static unsigned int
drmmode_crtc_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, drmModeResPtr mode_res, int num)
{
	NVPtr pNv = NVPTR(pScrn);
	NVEntPtr pNVEnt = NVEntPriv(pScrn);
	xf86CrtcPtr crtc;
	drmmode_crtc_private_ptr drmmode_crtc;
	int ret;

	crtc = xf86CrtcCreate(pScrn, &drmmode_crtc_funcs);
	if (crtc == NULL)
		return 0;

	drmmode_crtc = XNFcallocarray(sizeof(drmmode_crtc_private_rec), 1);
	drmmode_crtc->mode_crtc = drmModeGetCrtc(drmmode->fd,
						 mode_res->crtcs[num]);
	drmmode_crtc->drmmode = drmmode;
	drmmode_crtc->hw_crtc_index = num;

	ret = nouveau_bo_new(pNv->dev, NOUVEAU_BO_GART | NOUVEAU_BO_MAP, 0,
			     64*64*4, NULL, &drmmode_crtc->cursor);
	assert(ret == 0);

	crtc->driver_private = drmmode_crtc;

	/* Mark num'th crtc as in use on this device. */
	pNVEnt->assigned_crtcs |= (1 << num);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Allocated crtc nr. %d to this screen.\n", num);

	return 1;
}

static xf86OutputStatus
drmmode_output_detect(xf86OutputPtr output)
{
	/* go to the hw and retrieve a new output struct */
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	xf86OutputStatus status;

	if (drmmode_output->output_id == -1)
		return XF86OutputStatusDisconnected;

	drmModeFreeConnector(drmmode_output->mode_output);

	drmmode_output->mode_output =
		drmModeGetConnector(drmmode->fd, drmmode_output->output_id);

	if (!drmmode_output->mode_output) {
		drmmode_output->output_id = -1;
		return XF86OutputStatusDisconnected;
	}

	switch (drmmode_output->mode_output->connection) {
	case DRM_MODE_CONNECTED:
		status = XF86OutputStatusConnected;
		break;
	case DRM_MODE_DISCONNECTED:
		status = XF86OutputStatusDisconnected;
		break;
	default:
	case DRM_MODE_UNKNOWNCONNECTION:
		status = XF86OutputStatusUnknown;
		break;
	}
	return status;
}

static Bool
drmmode_output_mode_valid(xf86OutputPtr output, DisplayModePtr mode)
{
	if (mode->type & M_T_DEFAULT)
		/* Default modes are harmful here. */
		return MODE_BAD;

	return MODE_OK;
}

static int
koutput_get_prop_idx(int fd, drmModeConnectorPtr koutput,
		     int type, const char *name)
{
	int idx = -1;

	for (int i = 0; i < koutput->count_props; i++) {
		drmModePropertyPtr prop = drmModeGetProperty(fd, koutput->props[i]);

		if (!prop)
			continue;

		if (drm_property_type_is(prop, type) && !strcmp(prop->name, name))
			idx = i;

		drmModeFreeProperty(prop);

		if (idx > -1)
			break;
	}

	return idx;
}

static drmModePropertyBlobPtr
koutput_get_prop_blob(int fd, drmModeConnectorPtr koutput, const char *name)
{
	drmModePropertyBlobPtr blob = NULL;
	int idx = koutput_get_prop_idx(fd, koutput, DRM_MODE_PROP_BLOB, name);

	if (idx > -1)
		blob = drmModeGetPropertyBlob(fd, koutput->prop_values[idx]);

	return blob;
}

static void
drmmode_output_attach_tile(xf86OutputPtr output)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmModeConnectorPtr koutput = drmmode_output->mode_output;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	struct xf86CrtcTileInfo tile_info, *set = NULL;

	if (!koutput) {
		xf86OutputSetTile(output, NULL);
		return;
	}

	drmModeFreePropertyBlob(drmmode_output->tile_blob);

	/* look for a TILE property */
	drmmode_output->tile_blob =
		koutput_get_prop_blob(drmmode->fd, koutput, "TILE");

	if (drmmode_output->tile_blob) {
		if (xf86OutputParseKMSTile(drmmode_output->tile_blob->data, drmmode_output->tile_blob->length, &tile_info) == TRUE)
			set = &tile_info;
	}
	xf86OutputSetTile(output, set);
}


static DisplayModePtr
drmmode_output_get_modes(xf86OutputPtr output)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmModeConnectorPtr koutput = drmmode_output->mode_output;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	int i;
	DisplayModePtr Modes = NULL, Mode;
	xf86MonPtr ddc_mon = NULL;

	if (!koutput)
		return NULL;

	/* look for an EDID property */
	drmmode_output->edid_blob =
		koutput_get_prop_blob(drmmode->fd, koutput, "EDID");

	if (drmmode_output->edid_blob) {
		ddc_mon = xf86InterpretEDID(output->scrn->scrnIndex,
					    drmmode_output->edid_blob->data);
		if (ddc_mon && drmmode_output->edid_blob->length > 128)
			ddc_mon->flags |= MONITOR_EDID_COMPLETE_RAWDATA;
	}
	xf86OutputSetEDID(output, ddc_mon);

	drmmode_output_attach_tile(output);

	/* modes should already be available */
	for (i = 0; i < koutput->count_modes; i++) {
		Mode = XNFalloc(sizeof(DisplayModeRec));

		drmmode_ConvertFromKMode(output->scrn, &koutput->modes[i],
					 Mode);
		Modes = xf86ModesAdd(Modes, Mode);

	}
	return Modes;
}

static void
drmmode_output_destroy(xf86OutputPtr output)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	int i;

	if (drmmode_output->edid_blob)
		drmModeFreePropertyBlob(drmmode_output->edid_blob);
	if (drmmode_output->tile_blob)
		drmModeFreePropertyBlob(drmmode_output->tile_blob);
	for (i = 0; i < drmmode_output->num_props; i++) {
		drmModeFreeProperty(drmmode_output->props[i].mode_prop);
		free(drmmode_output->props[i].atoms);
	}
	drmModeFreeConnector(drmmode_output->mode_output);
	free(drmmode_output);
	output->driver_private = NULL;
}

static void
drmmode_output_dpms(xf86OutputPtr output, int mode)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmModeConnectorPtr koutput = drmmode_output->mode_output;
	drmModePropertyPtr props;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	int mode_id = -1, i;

	if (!koutput)
		return;

	for (i = 0; i < koutput->count_props; i++) {
		props = drmModeGetProperty(drmmode->fd, koutput->props[i]);
		if (props && (props->flags & DRM_MODE_PROP_ENUM)) {
			if (!strcmp(props->name, "DPMS")) {
				mode_id = koutput->props[i];
				drmModeFreeProperty(props);
				break;
			}
			drmModeFreeProperty(props);
		}
	}

	if (mode_id < 0)
		return;

	drmModeConnectorSetProperty(drmmode->fd, koutput->connector_id,
				    mode_id, mode);
}

static Bool
drmmode_property_ignore(drmModePropertyPtr prop)
{
	if (!prop)
	    return TRUE;
	/* ignore blob prop */
	if (prop->flags & DRM_MODE_PROP_BLOB)
		return TRUE;
	/* ignore standard property */
	if (!strcmp(prop->name, "EDID") ||
	    !strcmp(prop->name, "DPMS"))
		return TRUE;

	return FALSE;
}

static void
drmmode_output_create_resources(xf86OutputPtr output)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmModeConnectorPtr mode_output = drmmode_output->mode_output;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	drmModePropertyPtr drmmode_prop;
	uint32_t value;
	int i, j, err;

	drmmode_output->props = calloc(mode_output->count_props, sizeof(drmmode_prop_rec));
	if (!drmmode_output->props)
		return;

	drmmode_output->num_props = 0;
	for (i = 0, j = 0; i < mode_output->count_props; i++) {
		drmmode_prop = drmModeGetProperty(drmmode->fd, mode_output->props[i]);
		if (drmmode_property_ignore(drmmode_prop)) {
			drmModeFreeProperty(drmmode_prop);
			continue;
		}
		drmmode_output->props[j].mode_prop = drmmode_prop;
		drmmode_output->props[j].index = i;
		drmmode_output->num_props++;
		j++;
	}

	for (i = 0; i < drmmode_output->num_props; i++) {
		drmmode_prop_ptr p = &drmmode_output->props[i];
		drmmode_prop = p->mode_prop;

		value = drmmode_output->mode_output->prop_values[p->index];

		if (drmmode_prop->flags & DRM_MODE_PROP_RANGE) {
			INT32 range[2];

			p->num_atoms = 1;
			p->atoms = calloc(p->num_atoms, sizeof(Atom));
			if (!p->atoms)
				continue;
			p->atoms[0] = MakeAtom(drmmode_prop->name, strlen(drmmode_prop->name), TRUE);
			range[0] = drmmode_prop->values[0];
			range[1] = drmmode_prop->values[1];
			err = RRConfigureOutputProperty(output->randr_output, p->atoms[0],
							FALSE, TRUE,
							drmmode_prop->flags & DRM_MODE_PROP_IMMUTABLE ? TRUE : FALSE,
							2, range);
			if (err != 0) {
				xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
					   "RRConfigureOutputProperty error, %d\n", err);
			}
			err = RRChangeOutputProperty(output->randr_output, p->atoms[0],
						     XA_INTEGER, 32, PropModeReplace, 1,
						     &value, FALSE, FALSE);
			if (err != 0) {
				xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
					   "RRChangeOutputProperty error, %d\n", err);
			}
		} else if (drmmode_prop->flags & DRM_MODE_PROP_ENUM) {
			p->num_atoms = drmmode_prop->count_enums + 1;
			p->atoms = calloc(p->num_atoms, sizeof(Atom));
			if (!p->atoms)
				continue;
			p->atoms[0] = MakeAtom(drmmode_prop->name, strlen(drmmode_prop->name), TRUE);
			for (j = 1; j <= drmmode_prop->count_enums; j++) {
				struct drm_mode_property_enum *e = &drmmode_prop->enums[j-1];
				p->atoms[j] = MakeAtom(e->name, strlen(e->name), TRUE);
			}
			err = RRConfigureOutputProperty(output->randr_output, p->atoms[0],
							FALSE, FALSE,
							drmmode_prop->flags & DRM_MODE_PROP_IMMUTABLE ? TRUE : FALSE,
							p->num_atoms - 1, (INT32 *)&p->atoms[1]);
			if (err != 0) {
				xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
					   "RRConfigureOutputProperty error, %d\n", err);
			}
			for (j = 0; j < drmmode_prop->count_enums; j++)
				if (drmmode_prop->enums[j].value == value)
					break;
			/* there's always a matching value */
			err = RRChangeOutputProperty(output->randr_output, p->atoms[0],
						     XA_ATOM, 32, PropModeReplace, 1, &p->atoms[j+1], FALSE, FALSE);
			if (err != 0) {
				xf86DrvMsg(output->scrn->scrnIndex, X_ERROR,
					   "RRChangeOutputProperty error, %d\n", err);
			}
		}
	}
}

static Bool
drmmode_output_set_property(xf86OutputPtr output, Atom property,
			    RRPropertyValuePtr value)
{
	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	int i, ret;

	for (i = 0; i < drmmode_output->num_props; i++) {
		drmmode_prop_ptr p = &drmmode_output->props[i];

		if (p->atoms[0] != property)
			continue;

		if (p->mode_prop->flags & DRM_MODE_PROP_RANGE) {
			uint32_t val;

			if (value->type != XA_INTEGER || value->format != 32 ||
			    value->size != 1)
				return FALSE;
			val = *(uint32_t *)value->data;

			ret = drmModeConnectorSetProperty(drmmode->fd, drmmode_output->output_id,
							  p->mode_prop->prop_id, (uint64_t)val);

			if (ret)
				return FALSE;

			return TRUE;

		} else if (p->mode_prop->flags & DRM_MODE_PROP_ENUM) {
			Atom	atom;
			const char	*name;
			int		j;

			if (value->type != XA_ATOM || value->format != 32 || value->size != 1)
				return FALSE;
			memcpy(&atom, value->data, 4);
			if (!(name = NameForAtom(atom)))
				return FALSE;

			/* search for matching name string, then set its value down */
			for (j = 0; j < p->mode_prop->count_enums; j++) {
				if (!strcmp(p->mode_prop->enums[j].name, name)) {
					ret = drmModeConnectorSetProperty(drmmode->fd,
									  drmmode_output->output_id,
									  p->mode_prop->prop_id,
									  p->mode_prop->enums[j].value);

					if (ret)
						return FALSE;

					return TRUE;
				}
			}

			return FALSE;
		}
	}

	return TRUE;
}

static Bool
drmmode_output_get_property(xf86OutputPtr output, Atom property)
{

	drmmode_output_private_ptr drmmode_output = output->driver_private;
	drmmode_ptr drmmode = drmmode_output->drmmode;
	uint32_t value;
	int err, i;

	if (output->scrn->vtSema) {
		drmModeFreeConnector(drmmode_output->mode_output);
		drmmode_output->mode_output =
			drmModeGetConnector(drmmode->fd, drmmode_output->output_id);
	}

	if (!drmmode_output->mode_output)
		return FALSE;

	for (i = 0; i < drmmode_output->num_props; i++) {
		drmmode_prop_ptr p = &drmmode_output->props[i];
		if (p->atoms[0] != property)
			continue;

		value = drmmode_output->mode_output->prop_values[p->index];

		if (p->mode_prop->flags & DRM_MODE_PROP_RANGE) {
			err = RRChangeOutputProperty(output->randr_output,
						     property, XA_INTEGER, 32,
						     PropModeReplace, 1, &value,
						     FALSE, FALSE);

			return !err;
		} else if (p->mode_prop->flags & DRM_MODE_PROP_ENUM) {
			int		j;

			/* search for matching name string, then set its value down */
			for (j = 0; j < p->mode_prop->count_enums; j++) {
				if (p->mode_prop->enums[j].value == value)
					break;
			}

			err = RRChangeOutputProperty(output->randr_output, property,
						     XA_ATOM, 32, PropModeReplace, 1,
						     &p->atoms[j+1], FALSE, FALSE);

			return !err;
		}
	}

	return FALSE;
}

static const xf86OutputFuncsRec drmmode_output_funcs = {
	.create_resources = drmmode_output_create_resources,
	.dpms = drmmode_output_dpms,
	.detect = drmmode_output_detect,
	.mode_valid = drmmode_output_mode_valid,
	.get_modes = drmmode_output_get_modes,
	.set_property = drmmode_output_set_property,
	.get_property = drmmode_output_get_property,
	.destroy = drmmode_output_destroy
};

static int subpixel_conv_table[7] = { 0, SubPixelUnknown,
				      SubPixelHorizontalRGB,
				      SubPixelHorizontalBGR,
				      SubPixelVerticalRGB,
				      SubPixelVerticalBGR,
				      SubPixelNone };

const char *output_names[] = { "None",
			       "VGA",
			       "DVI-I",
			       "DVI-D",
			       "DVI-A",
			       "Composite",
			       "SVIDEO",
			       "LVDS",
			       "CTV",
			       "DIN",
			       "DP",
			       "HDMI",
			       "HDMI",
			       "TV",
			       "eDP",
};
#define NUM_OUTPUT_NAMES (sizeof(output_names) / sizeof(output_names[0]))

static Bool
drmmode_zaphod_match(ScrnInfoPtr pScrn, const char *s, char *output_name)
{
    int i = 0;
    char s1[20];

    do {
	switch(*s) {
	case ',':
	    s1[i] = '\0';
	    i = 0;
	    if (strcmp(s1, output_name) == 0)
		return TRUE;
	    break;
	case ' ':
	case '\t':
	case '\n':
	case '\r':
	    break;
	default:
	    s1[i] = *s;
	    i++;
	    break;
	}
    } while(*s++);

    s1[i] = '\0';
    if (strcmp(s1, output_name) == 0)
	return TRUE;

    return FALSE;
}

static xf86OutputPtr find_output(ScrnInfoPtr pScrn, int id)
{
	xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	int i;
	for (i = 0; i < xf86_config->num_output; i++) {
		xf86OutputPtr output = xf86_config->output[i];
		drmmode_output_private_ptr drmmode_output;

		drmmode_output = output->driver_private;
		if (drmmode_output->output_id == id)
			return output;
	}
	return NULL;
}

static int parse_path_blob(drmModePropertyBlobPtr path_blob, int *conn_base_id, char **path)
{
	char *conn;
	char conn_id[5];
	int id, len;
	char *blob_data;

	if (!path_blob)
		return -1;

	blob_data = path_blob->data;
	/* we only handle MST paths for now */
	if (strncmp(blob_data, "mst:", 4))
		return -1;

	conn = strchr(blob_data + 4, '-');
	if (!conn)
		return -1;
	len = conn - (blob_data + 4);
	if (len + 1 > 5)
		return -1;
	memcpy(conn_id, blob_data + 4, len);
	conn_id[len] = '\0';
	id = strtoul(conn_id, NULL, 10);

	*conn_base_id = id;

	*path = conn + 1;
	return 0;
}

static void
drmmode_create_name(ScrnInfoPtr pScrn, drmModeConnectorPtr koutput, char *name,
                    drmModePropertyBlobPtr path_blob)
{
	int ret;
	char *extra_path;
	int conn_id;
	xf86OutputPtr output;

	ret = parse_path_blob(path_blob, &conn_id, &extra_path);
	if (ret == -1)
		goto fallback;

	output = find_output(pScrn, conn_id);
	if (!output)
		goto fallback;

	snprintf(name, 32, "%s-%s", output->name, extra_path);
	return;

fallback:
	if (koutput->connector_type >= ARRAY_SIZE(output_names))
		snprintf(name, 32, "Unknown%d-%d", koutput->connector_type, koutput->connector_type_id);
	else if (pScrn->is_gpu)
		snprintf(name, 32, "%s-%d-%d", output_names[koutput->connector_type], pScrn->scrnIndex - GPU_SCREEN_OFFSET + 1, koutput->connector_type_id);
	else
		snprintf(name, 32, "%s-%d", output_names[koutput->connector_type], koutput->connector_type_id);
}

static unsigned int
drmmode_output_init(ScrnInfoPtr pScrn, drmmode_ptr drmmode, drmModeResPtr mode_res, int num, Bool dynamic, int crtcshift)
{
	NVPtr pNv = NVPTR(pScrn);
	xf86OutputPtr output;
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	drmModeConnectorPtr koutput;
	drmModeEncoderPtr kencoder;
	drmmode_output_private_ptr drmmode_output;
	const char *s;
	char name[32];
	drmModePropertyBlobPtr path_blob = NULL;
	int i;

	koutput = drmModeGetConnector(drmmode->fd,
				      mode_res->connectors[num]);
	if (!koutput)
		return 0;

	path_blob = koutput_get_prop_blob(drmmode->fd, koutput, "PATH");

	drmmode_create_name(pScrn, koutput, name, path_blob);

	if (path_blob)
		drmModeFreePropertyBlob(path_blob);

	if (path_blob && dynamic) {
		/* see if we have an output with this name already
		   and hook stuff up */
		for (i = 0; i < xf86_config->num_output; i++) {
			output = xf86_config->output[i];

			if (strncmp(output->name, name, 32))
				continue;

			drmmode_output = output->driver_private;
			drmmode_output->output_id = mode_res->connectors[num];
			drmmode_output->mode_output = koutput;
			return 1;
		}
	}


	kencoder = drmModeGetEncoder(drmmode->fd, koutput->encoders[0]);
	if (!kencoder) {
		drmModeFreeConnector(koutput);
		return 0;
	}

	if (xf86IsEntityShared(pScrn->entityList[0])) {
		s = xf86GetOptValString(pNv->Options, OPTION_ZAPHOD_HEADS);
		if (s) {
			if (!drmmode_zaphod_match(pScrn, s, name)) {
				drmModeFreeEncoder(kencoder);
				drmModeFreeConnector(koutput);
				return 0;
			}
		} else {
			if (pNv->Primary && (num != 0)) {
				drmModeFreeEncoder(kencoder);
				drmModeFreeConnector(koutput);
				return 0;
			} else
			if (pNv->Secondary && (num != 1)) {
				drmModeFreeEncoder(kencoder);
				drmModeFreeConnector(koutput);
				return 0;
			}
		}
	}

	output = xf86OutputCreate (pScrn, &drmmode_output_funcs, name);
	if (!output) {
		drmModeFreeEncoder(kencoder);
		drmModeFreeConnector(koutput);
		return 0;
	}

	drmmode_output = calloc(sizeof(drmmode_output_private_rec), 1);
	if (!drmmode_output) {
		xf86OutputDestroy(output);
		drmModeFreeConnector(koutput);
		drmModeFreeEncoder(kencoder);
		return 0;
	}

	drmmode_output->output_id = mode_res->connectors[num];
	drmmode_output->mode_output = koutput;
	drmmode_output->mode_encoder = kencoder;
	drmmode_output->drmmode = drmmode;
	output->mm_width = koutput->mmWidth;
	output->mm_height = koutput->mmHeight;

	output->subpixel_order = subpixel_conv_table[koutput->subpixel];
	output->driver_private = drmmode_output;

	output->possible_crtcs = kencoder->possible_crtcs >> crtcshift;
	output->possible_clones = kencoder->possible_clones >> crtcshift;

	output->interlaceAllowed = true;
	output->doubleScanAllowed = true;

	if (dynamic)
		output->randr_output = RROutputCreate(xf86ScrnToScreen(pScrn), output->name, strlen(output->name), output);

	return 1;
}

static Bool
drmmode_xf86crtc_resize(ScrnInfoPtr scrn, int width, int height)
{
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
	ScreenPtr screen = xf86ScrnToScreen(scrn);
	NVPtr pNv = NVPTR(scrn);
	drmmode_crtc_private_ptr drmmode_crtc = NULL;
	drmmode_ptr drmmode = NULL;
	uint32_t old_width, old_height, old_pitch, old_fb_id = 0;
	struct nouveau_bo *old_bo = NULL;
	int ret, i, pitch;
	PixmapPtr ppix;

	if (xf86_config->num_crtc) {
		drmmode_crtc = xf86_config->crtc[0]->driver_private;
		drmmode = drmmode_crtc->drmmode;
	}
	ErrorF("resize called %d %d\n", width, height);

	if (scrn->virtualX == width && scrn->virtualY == height)
		return TRUE;

	old_width = scrn->virtualX;
	old_height = scrn->virtualY;
	old_pitch = scrn->displayWidth;
	if (drmmode)
		old_fb_id = drmmode->fb_id;
	nouveau_bo_ref(pNv->scanout, &old_bo);
	nouveau_bo_ref(NULL, &pNv->scanout);

	ret = nouveau_allocate_surface(scrn, width, height,
				       scrn->bitsPerPixel,
				       NOUVEAU_CREATE_PIXMAP_SCANOUT,
				       &pitch, &pNv->scanout);
	if (!ret)
		goto fail;

	scrn->virtualX = width;
	scrn->virtualY = height;
	scrn->displayWidth = pitch / (scrn->bitsPerPixel >> 3);

	nouveau_bo_map(pNv->scanout, NOUVEAU_BO_RDWR, pNv->client);

	if (drmmode) {
		ret = drmModeAddFB(drmmode->fd, width, height, scrn->depth,
				  scrn->bitsPerPixel, pitch, pNv->scanout->handle,
				  &drmmode->fb_id);
		if (ret)
			goto fail;
	}

	if (pNv->ShadowPtr) {
		free(pNv->ShadowPtr);
		pNv->ShadowPitch = pitch;
		pNv->ShadowPtr = malloc(pNv->ShadowPitch * height);
	}

	ppix = screen->GetScreenPixmap(screen);
	if (pNv->AccelMethod > NONE)
		nouveau_bo_ref(pNv->scanout, &drmmode_pixmap(ppix)->bo);
	screen->ModifyPixmapHeader(ppix, width, height, -1, -1, pitch,
				   (pNv->AccelMethod > NONE || pNv->ShadowPtr) ?
				   pNv->ShadowPtr : pNv->scanout->map);

	if (pNv->AccelMethod == EXA) {
		pNv->EXADriverPtr->PrepareSolid(ppix, GXcopy, ~0, 0);
		pNv->EXADriverPtr->Solid(ppix, 0, 0, width, height);
		pNv->EXADriverPtr->DoneSolid(ppix);
		nouveau_bo_map(pNv->scanout, NOUVEAU_BO_RDWR, pNv->client);
	} else {
		memset(pNv->scanout->map, 0x00, pNv->scanout->size);
	}

	for (i = 0; i < xf86_config->num_crtc; i++) {
		xf86CrtcPtr crtc = xf86_config->crtc[i];

		if (!crtc->enabled)
			continue;

		drmmode_set_mode_major(crtc, &crtc->mode,
				       crtc->rotation, crtc->x, crtc->y);
	}

	if (old_fb_id)
		drmModeRmFB(drmmode->fd, old_fb_id);
	nouveau_bo_ref(NULL, &old_bo);

	return TRUE;

 fail:
	nouveau_bo_ref(old_bo, &pNv->scanout);
	scrn->virtualX = old_width;
	scrn->virtualY = old_height;
	scrn->displayWidth = old_pitch;
	if (drmmode)
		drmmode->fb_id = old_fb_id;

	return FALSE;
}

static const xf86CrtcConfigFuncsRec drmmode_xf86crtc_config_funcs = {
	drmmode_xf86crtc_resize
};

Bool drmmode_pre_init(ScrnInfoPtr pScrn, int fd, int cpp)
{
	drmmode_ptr drmmode;
	drmModeResPtr mode_res;
	NVEntPtr pNVEnt = NVEntPriv(pScrn);
	int i;
	unsigned int crtcs_needed = 0;
	int crtcshift;

	drmmode = XNFcallocarray(sizeof(*drmmode), 1);
	drmmode->fd = fd;
	drmmode->fb_id = 0;

	xf86CrtcConfigInit(pScrn, &drmmode_xf86crtc_config_funcs);

	drmmode->cpp = cpp;
	mode_res = drmModeGetResources(drmmode->fd);
	if (!mode_res)
		return FALSE;

	xf86CrtcSetSizeRange(pScrn, 320, 200, mode_res->max_width,
			     mode_res->max_height);

	if (!mode_res->count_connectors ||
	    !mode_res->count_crtcs) {
		free(drmmode);
		goto done;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Initializing outputs ...\n");
	crtcshift = ffs(pNVEnt->assigned_crtcs ^ 0xffffffff) - 1;
	for (i = 0; i < mode_res->count_connectors; i++)
		crtcs_needed += drmmode_output_init(pScrn, drmmode, mode_res, i, FALSE, crtcshift);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "%d crtcs needed for screen.\n", crtcs_needed);

	for (i = 0; i < mode_res->count_crtcs; i++) {
		if (!xf86IsEntityShared(pScrn->entityList[0]) ||
		    (crtcs_needed && !(pNVEnt->assigned_crtcs & (1 << i))))
			crtcs_needed -= drmmode_crtc_init(pScrn, drmmode, mode_res, i);
	}

	/* All ZaphodHeads outputs provided with matching crtcs? */
	if (xf86IsEntityShared(pScrn->entityList[0]) && (crtcs_needed > 0))
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "%d ZaphodHeads crtcs unavailable. Trouble!\n",
			   crtcs_needed);

done:
	drmModeFreeResources(mode_res);

#ifdef NOUVEAU_PIXMAP_SHARING
	xf86ProviderSetup(pScrn, NULL, "nouveau");
#endif

	xf86InitialConfiguration(pScrn, TRUE);

	return TRUE;
}

void
drmmode_adjust_frame(ScrnInfoPtr scrn, int x, int y)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(scrn);
	xf86OutputPtr output = config->output[config->compat_output];
	xf86CrtcPtr crtc = output->crtc;

	if (!crtc || !crtc->enabled)
		return;

	drmmode_set_mode_major(crtc, &crtc->mode, crtc->rotation, x, y);
}

void
drmmode_remove_fb(ScrnInfoPtr pScrn)
{
	xf86CrtcConfigPtr config = XF86_CRTC_CONFIG_PTR(pScrn);
	xf86CrtcPtr crtc = NULL;
	drmmode_crtc_private_ptr drmmode_crtc;
	drmmode_ptr drmmode;

	if (config && config->num_crtc)
		crtc = config->crtc[0];
	if (!crtc)
		return;

	drmmode_crtc = crtc->driver_private;
	drmmode = drmmode_crtc->drmmode;

	if (drmmode->fb_id)
		drmModeRmFB(drmmode->fd, drmmode->fb_id);
	drmmode->fb_id = 0;
}

int
drmmode_cursor_init(ScreenPtr pScreen)
{
	NVPtr pNv = NVPTR(xf86ScreenToScrn(pScreen));
	int size = nv_cursor_width(pNv);
	int flags = HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
		    HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_32 |
		    (pNv->dev->chipset >= 0x11 ? HARDWARE_CURSOR_ARGB : 0) |
		    HARDWARE_CURSOR_UPDATE_UNHIDDEN;

	return xf86_cursors_init(pScreen, size, size, flags);
}

#ifdef HAVE_LIBUDEV

#define DRM_MODE_LINK_STATUS_GOOD       0
#define DRM_MODE_LINK_STATUS_BAD        1

static void
drmmode_handle_uevents(ScrnInfoPtr scrn)
{
    struct udev_device *dev;
    drmmode_ptr drmmode = drmmode_from_scrn(scrn);
    drmModeResPtr mode_res;
    xf86CrtcConfigPtr  config = XF86_CRTC_CONFIG_PTR(scrn);
    int i, j;
    Bool found = FALSE;
    Bool changed = FALSE;

    while ((dev = udev_monitor_receive_device(drmmode->uevent_monitor))) {
        udev_device_unref(dev);
        found = TRUE;
    }
    if (!found)
        return;

    /* Try to re-set the mode on all the connectors with a BAD link-state:
     * This may happen if a link degrades and a new modeset is necessary, using
     * different link-training parameters. If the kernel found that the current
     * mode is not achievable anymore, it should have pruned the mode before
     * sending the hotplug event. Try to re-set the currently-set mode to keep
     * the display alive, this will fail if the mode has been pruned.
     * In any case, we will send randr events for the Desktop Environment to
     * deal with it, if it wants to.
     */
    for (i = 0; i < config->num_output; i++) {
        xf86OutputPtr output = config->output[i];
        xf86CrtcPtr crtc = output->crtc;
        drmmode_output_private_ptr drmmode_output = output->driver_private;
        uint32_t con_id, idx;
        drmModeConnectorPtr koutput;

        if (crtc == NULL || drmmode_output->mode_output == NULL)
            continue;

        con_id = drmmode_output->mode_output->connector_id;
        /* Get an updated view of the properties for the current connector and
         * look for the link-status property
         */
        koutput = drmModeGetConnectorCurrent(drmmode->fd, con_id);
        if (!koutput)
            continue;

        idx = koutput_get_prop_idx(drmmode->fd, koutput,
                DRM_MODE_PROP_ENUM, "link-status");

        if ((idx > -1) &&
                (koutput->prop_values[idx] == DRM_MODE_LINK_STATUS_BAD)) {
            /* the connector got a link failure, re-set the current mode */
            drmmode_set_mode_major(crtc, &crtc->mode, crtc->rotation,
                                   crtc->x, crtc->y);

            xf86DrvMsg(scrn->scrnIndex, X_WARNING,
                       "hotplug event: connector %u's link-state is BAD, "
                       "tried resetting the current mode. You may be left"
                       "with a black screen if this fails...\n", con_id);
        }

        drmModeFreeConnector(koutput);
    }

    mode_res = drmModeGetResources(drmmode->fd);
    if (!mode_res)
        goto out;

    if (mode_res->count_crtcs != config->num_crtc) {
        ErrorF("number of CRTCs changed - failed to handle, %d vs %d\n", mode_res->count_crtcs, config->num_crtc);
        goto out_free_res;
    }

    /* figure out if we have gotten rid of any connectors
       traverse old output list looking for outputs */
    for (i = 0; i < config->num_output; i++) {
        xf86OutputPtr output = config->output[i];
        drmmode_output_private_ptr drmmode_output;

        drmmode_output = output->driver_private;
        found = FALSE;
        for (j = 0; j < mode_res->count_connectors; j++) {
            if (mode_res->connectors[j] == drmmode_output->output_id) {
                found = TRUE;
                break;
            }
        }
        if (found)
            continue;

        drmModeFreeConnector(drmmode_output->mode_output);
        drmmode_output->mode_output = NULL;
        drmmode_output->output_id = -1;

        changed = TRUE;
    }

    /* find new output ids we don't have outputs for */
    for (i = 0; i < mode_res->count_connectors; i++) {
        found = FALSE;

        for (j = 0; j < config->num_output; j++) {
            xf86OutputPtr output = config->output[j];
            drmmode_output_private_ptr drmmode_output;

            drmmode_output = output->driver_private;
            if (mode_res->connectors[i] == drmmode_output->output_id) {
                found = TRUE;
                break;
            }
        }
        if (found)
            continue;

        changed = TRUE;
        drmmode_output_init(scrn, drmmode, mode_res, i, TRUE, 0);
    }

    if (changed) {
        RRSetChanged(xf86ScrnToScreen(scrn));
        RRTellChanged(xf86ScrnToScreen(scrn));
    }

out_free_res:
    drmModeFreeResources(mode_res);
out:
    RRGetInfo(xf86ScrnToScreen(scrn), TRUE);
}

#undef DRM_MODE_LINK_STATUS_BAD
#undef DRM_MODE_LINK_STATUS_GOOD

#endif

#if HAVE_NOTIFY_FD
static void
drmmode_udev_notify(int fd, int notify, void *data)
{
	ScrnInfoPtr scrn = data;
	drmmode_handle_uevents(scrn);
}
#endif

static bool has_randr(void)
{
	return dixPrivateKeyRegistered(rrPrivKey);
}

static void
drmmode_uevent_init(ScrnInfoPtr scrn)
{
#ifdef HAVE_LIBUDEV
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	struct udev *u;
	struct udev_monitor *mon;

	/* RandR will be disabled if Xinerama is active, and so generating
	 * RR hotplug events is then forbidden.
	 */
	if (!has_randr())
		return;

	u = udev_new();
	if (!u)
		return;
	mon = udev_monitor_new_from_netlink(u, "udev");
	if (!mon) {
		udev_unref(u);
		return;
	}

	if (udev_monitor_filter_add_match_subsystem_devtype(mon,
							    "drm",
							    "drm_minor") < 0 ||
	    udev_monitor_enable_receiving(mon) < 0) {
		udev_monitor_unref(mon);
		udev_unref(u);
		return;
	}

#if HAVE_NOTIFY_FD
	SetNotifyFd(udev_monitor_get_fd(mon), drmmode_udev_notify, X_NOTIFY_READ, scrn);
#else
	AddGeneralSocket(udev_monitor_get_fd(mon));
#endif
	drmmode->uevent_monitor = mon;
#endif
}

static void
drmmode_uevent_fini(ScrnInfoPtr scrn)
{
#ifdef HAVE_LIBUDEV
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);

	if (drmmode->uevent_monitor) {
		struct udev *u = udev_monitor_get_udev(drmmode->uevent_monitor);

#if HAVE_NOTIFY_FD
		RemoveNotifyFd(udev_monitor_get_fd(drmmode->uevent_monitor));
#else
		RemoveGeneralSocket(udev_monitor_get_fd(drmmode->uevent_monitor));
#endif
		udev_monitor_unref(drmmode->uevent_monitor);
		udev_unref(u);
	}
#endif
}

#if HAVE_NOTIFY_FD
static void
drmmode_notify_fd(int fd, int notify, void *data)
{
	ScrnInfoPtr scrn = data;
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	drmHandleEvent(drmmode->fd, &drmmode->event_context);
}
#else

static void
drmmode_wakeup_handler(pointer data, int err, pointer p)
{
	ScrnInfoPtr scrn = data;
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	fd_set *read_mask = p;

	if (scrn == NULL || err < 0)
		return;

	if (FD_ISSET(drmmode->fd, read_mask))
		drmHandleEvent(drmmode->fd, &drmmode->event_context);

#ifdef HAVE_LIBUDEV
	if (FD_ISSET(udev_monitor_get_fd(drmmode->uevent_monitor), read_mask))
		drmmode_handle_uevents(scrn);
#endif
}
#endif

void
drmmode_screen_init(ScreenPtr pScreen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(pScreen);
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	NVEntPtr pNVEnt = NVEntPriv(scrn);

	/* Setup handler for DRM events */
	drmmode_event_init(scrn);

	/* Setup handler for udevevents */
	drmmode_uevent_init(scrn);

	/* Register wakeup handler only once per servergen, so ZaphodHeads work */
	if (pNVEnt->fd_wakeup_registered != serverGeneration) {
		/* Register a wakeup handler to get informed on DRM events */
#if HAVE_NOTIFY_FD
		SetNotifyFd(drmmode->fd, drmmode_notify_fd, X_NOTIFY_READ, scrn);
#else
		AddGeneralSocket(drmmode->fd);
		RegisterBlockAndWakeupHandlers((BlockHandlerProcPtr)NoopDDA,
		                               drmmode_wakeup_handler, scrn);
#endif
		pNVEnt->fd_wakeup_registered = serverGeneration;
		pNVEnt->fd_wakeup_ref = 1;
	}
	else
		pNVEnt->fd_wakeup_ref++;
}

void
drmmode_screen_fini(ScreenPtr pScreen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(pScreen);
	drmmode_ptr drmmode = drmmode_from_scrn(scrn);
	NVEntPtr pNVEnt = NVEntPriv(scrn);

	/* Unregister wakeup handler after last x-screen for this servergen dies. */
	if (pNVEnt->fd_wakeup_registered == serverGeneration &&
		!--pNVEnt->fd_wakeup_ref) {

#if HAVE_NOTIFY_FD
		RemoveNotifyFd(drmmode->fd);
#else
		/* Unregister wakeup handler */
		RemoveBlockAndWakeupHandlers((BlockHandlerProcPtr)NoopDDA,
		                             drmmode_wakeup_handler, scrn);
		RemoveGeneralSocket(drmmode->fd);
#endif
	}

	/* Tear down udev event handler */
	drmmode_uevent_fini(scrn);

	/* Tear down DRM event handler */
	drmmode_event_fini(scrn);
}
